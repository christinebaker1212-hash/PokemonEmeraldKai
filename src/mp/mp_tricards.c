// Moemon Co-op: the compact see-through cards of a Triple Battle.
//
// Six vanilla healthboxes don't fit the GBA's sprite memory next to six Moemon,
// and a GBA can't blend a sprite over another sprite. So in a Triple Battle the
// game's own healthbox objects still exist (every battle routine keeps using
// them), parked off-screen (InitBattlerHealthboxCoords), and these cards are
// drawn by the port over the finished frame (MP_TriCards_Composite, from VDraw).
//
// A card is 72x20: row 1 the full name, row 2 the level, a status tag and the HP
// bar. Foes are stacked top-left, the team right above the text box; the team's
// cards ramp from 38% to 0% see-through (the top one sits over the right foe).
// Only the cream fill and the border are blended: text, bar, status tag and the
// owner strip stay solid. Colors come from a sprite palette loaded for the battle,
// so they fade with the rest of the screen.
#include <string.h>
#include "global.h"
void MPNet_Log(const char *fmt, ...);
#include "battle.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "battle_anim.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "constants/rgb.h"
#include "pokemon.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "mp.h"

#define CARD_W 72
#define CARD_H 20
#define CARD_STEP 21
#define FOE_X 2
#define FOE_Y 2
#define TEAM_X 166
#define TEAM_Y 49

#define TAG_TRICARD_PAL 0x53C8

enum {
    C_NONE, C_FILL, C_EDGE, C_TEXT, C_SHADOW, C_BAR_EDGE, C_BAR_EMPTY, C_GREEN, C_YELLOW, C_RED,
    C_OWN1, C_OWN2, C_OWN3, C_WHITE, C_PSN, C_BRN,
};

static const u16 sCardPal[16] = {
    [C_NONE]     = RGB(0, 0, 0),
    [C_FILL]     = RGB(31, 31, 28),
    [C_EDGE]     = RGB(8, 10, 9),
    [C_TEXT]     = RGB(9, 9, 10),
    [C_SHADOW]   = RGB(27, 26, 22),
    [C_BAR_EDGE] = RGB(10, 13, 11),
    [C_BAR_EMPTY]= RGB(9, 9, 9),
    [C_GREEN]    = RGB(14, 31, 21),
    [C_YELLOW]   = RGB(31, 28, 0),
    [C_RED]      = RGB(31, 11, 7),
    [C_OWN1]     = RGB(29, 10, 8),
    [C_OWN2]     = RGB(8, 17, 29),
    [C_OWN3]     = RGB(9, 23, 9),
    [C_WHITE]    = RGB(31, 31, 31),
    [C_PSN]      = RGB(24, 12, 25),
    [C_BRN]      = RGB(29, 13, 8),
};

static const struct SpritePalette sCardSpritePal = {sCardPal, TAG_TRICARD_PAL};

struct Card
{
    bool8 used;
    bool8 dirty;
    u8 owner;           // 0-2: owner strip color, 0xFF: none (foes)
    u8 name[POKEMON_NAME_LENGTH + 1];
    u8 level;
    u8 status;          // 0 none, 1 PSN, 2 PAR, 3 SLP, 4 FRZ, 5 BRN
    s32 hp, maxHp;
    u8 pix[CARD_H][CARD_W];
};

static EWRAM_DATA struct Card sCards[MAX_BATTLERS_COUNT];
static bool8 sActive;

// ---------------------------------------------------------------------------
// Small-font text into a pixel buffer (the game's own font via a window)

#define TEXT_WIN_W 10
#define TEXT_WIN_H 2
// rows of the 16-row window that hold the glyphs (with their shadow)
#define GLYPH_TOP 4
#define GLYPH_ROWS 8

static const struct WindowTemplate sTextWindow = {
    .bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = TEXT_WIN_W, .height = TEXT_WIN_H, .paletteNum = 0, .baseBlock = 0
};

// Draws str at (x, y) of the card: font pixel 1 = text, 3 = shadow. Returns the x after it.
static u8 DrawText(struct Card *c, const u8 *str, u8 x, u8 y, u8 maxX)
{
    static const u8 sColors[3] = {0, 1, 3};
    u8 winId = AddWindow(&sTextWindow);
    u8 *tiles, right = x;
    int px, py;

    if (winId == WINDOW_NONE)
        return x;
    FillWindowPixelBuffer(winId, PIXEL_FILL(0));
    AddTextPrinterParameterized4(winId, FONT_SMALL, 0, 0, 0, 0, sColors, TEXT_SKIP_DRAW, str);
    tiles = GetWindowTileData(winId);
    for (py = 0; py < GLYPH_ROWS; py++)
    {
        int wy = GLYPH_TOP + py;
        for (px = 0; px < TEXT_WIN_W * 8 && x + px < maxX; px++)
        {
            u8 *t = tiles + ((wy / 8) * TEXT_WIN_W + px / 8) * TILE_SIZE_4BPP + (wy % 8) * 4 + (px % 8) / 2;
            u8 v = (px & 1) ? (*t >> 4) : (*t & 0xF);
            if (v == 1 || v == 3)
            {
                c->pix[y + py][x + px] = (v == 1) ? C_TEXT : C_SHADOW;
                if (x + px + 1 > right)
                    right = x + px + 1;
            }
        }
    }
    RemoveWindow(winId);
    return right;
}

static void FillRect(struct Card *c, int x0, int y0, int x1, int y1, u8 color)
{
    int x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
            if (x >= 0 && x < CARD_W && y >= 0 && y < CARD_H)
                c->pix[y][x] = color;
}

// A 20x8 status tag: rounded colored box with the 3 letters in white
static u8 DrawStatusTag(struct Card *c, u8 x, u8 y, u8 status)
{
    static const u8 sBg[] = {0, C_PSN, C_YELLOW, C_BAR_EMPTY, C_OWN2, C_BRN};
    static const u8 sText[][4] = {
        {EOS}, {CHAR_P, CHAR_S, CHAR_N, EOS}, {CHAR_P, CHAR_A, CHAR_R, EOS}, {CHAR_S, CHAR_L, CHAR_P, EOS},
        {CHAR_F, CHAR_R, CHAR_Z, EOS}, {CHAR_B, CHAR_R, CHAR_N, EOS},
    };
    int px, py;
    struct Card tmp;
    u8 end;

    FillRect(c, x + 1, y, x + 18, y + 7, sBg[status]);
    FillRect(c, x, y + 1, x + 19, y + 6, sBg[status]);
    memset(tmp.pix, 0, sizeof(tmp.pix));
    end = DrawText(&tmp, sText[status], 0, 0, CARD_W);
    // text only (no shadow), centered, in white
    for (py = 0; py < GLYPH_ROWS - 1; py++)
        for (px = 0; px < end && px < 18; px++)
            if (tmp.pix[py][px] == C_TEXT)
                c->pix[y + py][x + 1 + (18 - end) / 2 + px] = C_WHITE;
    return x + 20;
}

static void Render(struct Card *c)
{
    u8 lx, x, lv[8];
    int bx, bw, filled;
    u8 barColor;

    memset(c->pix, C_NONE, sizeof(c->pix));
    // body with cut corners, border
    FillRect(c, 1, 0, CARD_W - 2, CARD_H - 1, C_FILL);
    FillRect(c, 0, 1, CARD_W - 1, CARD_H - 2, C_FILL);
    FillRect(c, 1, 0, CARD_W - 2, 0, C_EDGE);
    FillRect(c, 1, CARD_H - 1, CARD_W - 2, CARD_H - 1, C_EDGE);
    FillRect(c, 0, 1, 0, CARD_H - 2, C_EDGE);
    FillRect(c, CARD_W - 1, 1, CARD_W - 1, CARD_H - 2, C_EDGE);
    lx = 3;
    if (c->owner < 3)
    {
        FillRect(c, 1, 1, 2, CARD_H - 2, C_OWN1 + c->owner);
        lx = 5;
    }
    // row 1: the name
    DrawText(c, c->name, lx, 2, CARD_W - 2);
    // row 2: level, status, HP bar
    lv[0] = CHAR_EXTRA_SYMBOL;
    lv[1] = CHAR_LV_2;
    ConvertIntToDecimalStringN(lv + 2, c->level, STR_CONV_MODE_LEFT_ALIGN, 3);
    x = DrawText(c, lv, lx, 10, CARD_W - 2);
    if (c->status)
        x = DrawStatusTag(c, x + 1, 10, c->status);
    bx = x + 2;
    bw = CARD_W - bx - 3;
    if (bw < 8)
        bw = 8;
    FillRect(c, bx - 1, 12, bx + bw, 15, C_BAR_EDGE);
    FillRect(c, bx, 13, bx + bw - 1, 14, C_BAR_EMPTY);
    if (c->maxHp > 0 && c->hp > 0)
    {
        filled = (c->hp * bw) / c->maxHp;
        if (filled < 1)
            filled = 1;
        barColor = (c->hp * 2 > c->maxHp) ? C_GREEN : (c->hp * 5 > c->maxHp) ? C_YELLOW : C_RED;
        FillRect(c, bx, 13, bx + filled - 1, 14, barColor);
    }
    c->dirty = FALSE;
}

// ---------------------------------------------------------------------------

void MP_TriCards_Begin(void)
{
    u8 i;
    memset(sCards, 0, sizeof(sCards));
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        sCards[i].owner = 0xFF;
    if (IndexOfSpritePaletteTag(TAG_TRICARD_PAL) == 0xFF)
        LoadSpritePalette(&sCardSpritePal);
    sActive = TRUE;
}

// The battle screen came back (bag, party...): FreeAllSpritePalettes dropped ours
void MP_TriCards_Reload(void)
{
    if (sActive && IndexOfSpritePaletteTag(TAG_TRICARD_PAL) == 0xFF)
        LoadSpritePalette(&sCardSpritePal);
}

void MP_TriCards_End(void)
{
    if (sActive)
        FreeSpritePaletteByTag(TAG_TRICARD_PAL);
    sActive = FALSE;
}

bool8 MP_TriCards_Active(void)
{
    return sActive && IS_TRIPLE_BATTLE();
}

void MP_TriCards_SetOwner(u8 battler, u8 seat)
{
    if (battler < MAX_BATTLERS_COUNT)
    {
        sCards[battler].owner = seat;
        sCards[battler].dirty = TRUE;
    }
}

static u8 StatusOf(u32 status1)
{
    if (status1 & STATUS1_SLEEP)
        return 3;
    if (status1 & STATUS1_PSN_ANY)
        return 1;
    if (status1 & STATUS1_BURN)
        return 5;
    if (status1 & STATUS1_FREEZE)
        return 4;
    if (status1 & STATUS1_PARALYSIS)
        return 2;
    return 0;
}

// UpdateHealthboxAttribute: name, level, status and HP from the Pokémon
void MP_TriCards_Update(u8 battler, struct Pokemon *mon, bool8 hpToo)
{
    struct Card *c;
    if (!sActive || battler >= MAX_BATTLERS_COUNT)
        return;
    c = &sCards[battler];
    c->used = TRUE;
    GetMonData(mon, MON_DATA_NICKNAME, c->name);
    c->name[POKEMON_NAME_LENGTH] = EOS;
    StringGet_Nickname(c->name);
    c->level = GetMonData(mon, MON_DATA_LEVEL);
    c->status = StatusOf(GetMonData(mon, MON_DATA_STATUS));
    if (hpToo)
    {
        c->hp = GetMonData(mon, MON_DATA_HP);
        c->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    }
    c->dirty = TRUE;
}

// MoveBattleBar: the HP the bar shows right now
void MP_TriCards_SetBarHp(u8 battler, s32 hp, s32 maxHp)
{
    struct Card *c;
    if (!sActive || battler >= MAX_BATTLERS_COUNT)
        return;
    c = &sCards[battler];
    if (hp < 0)
        hp = 0;
    if (c->hp != hp || c->maxHp != maxHp)
    {
        c->hp = hp;
        c->maxHp = maxHp;
        c->dirty = TRUE;
    }
}

// Where battler's card goes and how see-through its fill is (0-256 opacity)
static bool8 CardPlace(u8 battler, s16 *x, s16 *y, u16 *opacity)
{
    static const u8 sFoeRow[MAX_POSITION_COUNT] = {
        [B_POSITION_OPPONENT_RIGHT] = 0, [B_POSITION_OPPONENT_CENTER] = 1, [B_POSITION_OPPONENT_LEFT] = 2,
    };
    static const u8 sTeamRow[MAX_POSITION_COUNT] = {
        [B_POSITION_PLAYER_LEFT] = 0, [B_POSITION_PLAYER_CENTER] = 1, [B_POSITION_PLAYER_RIGHT] = 2,
    };
    static const u16 sTeamOpacity[3] = {159, 218, 256};   // 38%, 15%, 0% see-through
    u8 pos = GetBattlerPosition(battler);
    if (pos >= MAX_POSITION_COUNT)
        return FALSE;
    if (GetBattlerSide(battler) == B_SIDE_OPPONENT)
    {
        *x = FOE_X;
        *y = FOE_Y + sFoeRow[pos] * CARD_STEP;
        *opacity = 159;
    }
    else
    {
        *x = TEAM_X;
        *y = TEAM_Y + sTeamRow[pos] * CARD_STEP;
        *opacity = sTeamOpacity[sTeamRow[pos]];
    }
    return TRUE;
}

static inline u16 Blend(u16 fg, u16 bg, u16 a)
{
    u16 r = ((fg & 31) * a + (bg & 31) * (256 - a)) >> 8;
    u16 g = (((fg >> 5) & 31) * a + ((bg >> 5) & 31) * (256 - a)) >> 8;
    u16 b = (((fg >> 10) & 31) * a + ((bg >> 10) & 31) * (256 - a)) >> 8;
    return r | (g << 5) | (b << 10) | 0x8000;   // bit 15: opaque (ABGR1555 frame)
}

// After the GBA frame is drawn (platform VDraw): the cards over it
void MP_TriCards_Composite(u16 *frame)
{
    const u16 *pal;
    u8 slot, b;

    if (!sActive || !IS_TRIPLE_BATTLE() || !gMain.inBattle || gMain.callback2 != BattleMainCB2)
        return;
    slot = IndexOfSpritePaletteTag(TAG_TRICARD_PAL);
    if (slot == 0xFF)
        return;
    pal = (const u16 *)(OBJ_PLTT) + slot * 16;
    for (b = 0; b < gBattlersCount; b++)
    {
        struct Card *c = &sCards[b];
        struct Sprite *box;
        s16 x0, y0;
        u16 opacity, edgeOpacity;
        int px, py;

        if (!c->used || gHealthboxSpriteIds[b] >= MAX_SPRITES)
            continue;
        box = &gSprites[gHealthboxSpriteIds[b]];
        if (gMpDebugTriple)
        {
            static u8 sLastState[MAX_BATTLERS_COUNT];
            u8 st = (!box->inUse ? 1 : 0) | (box->invisible ? 2 : 0);
            if (st != sLastState[b])
            {
                MPNet_Log("tricards: b%d box %d state %d -> %d (cmds %d %d %d %d %d %d, main %p)", b, gHealthboxSpriteIds[b], sLastState[b], st,
                          gBattleBufferA[0][0], gBattleBufferA[1][0], gBattleBufferA[2][0], gBattleBufferA[3][0], gBattleBufferA[4][0], gBattleBufferA[5][0],
                          (void *)gBattleMainFunc);
                MPNet_Log("tricards: sprites %d %d %d %d %d %d, boxes %d %d %d %d %d %d, atk %d tgt %d", gBattlerSpriteIds[0], gBattlerSpriteIds[1],
                          gBattlerSpriteIds[2], gBattlerSpriteIds[3], gBattlerSpriteIds[4], gBattlerSpriteIds[5], gHealthboxSpriteIds[0], gHealthboxSpriteIds[1],
                          gHealthboxSpriteIds[2], gHealthboxSpriteIds[3], gHealthboxSpriteIds[4], gHealthboxSpriteIds[5], gBattleAnimAttacker, gBattleAnimTarget);
                sLastState[b] = st;
            }
        }
        if (!box->inUse || box->invisible)
            continue;
        if (!CardPlace(b, &x0, &y0, &opacity))
            continue;
        x0 += box->x2;
        y0 += box->y2;
        if (c->dirty)
            Render(c);
        edgeOpacity = opacity + 40 > 256 ? 256 : opacity + 40;
        for (py = 0; py < CARD_H; py++)
        {
            int sy = y0 + py;
            if (sy < 0 || sy >= DISPLAY_HEIGHT)
                continue;
            for (px = 0; px < CARD_W; px++)
            {
                int sx = x0 + px;
                u8 v = c->pix[py][px];
                u16 *dst;
                if (v == C_NONE || sx < 0 || sx >= DISPLAY_WIDTH)
                    continue;
                dst = &frame[sy * DISPLAY_WIDTH + sx];
                if (v == C_FILL)
                    *dst = Blend(pal[v], *dst, opacity);
                else if (v == C_EDGE)
                    *dst = Blend(pal[v], *dst, edgeOpacity);
                else
                    *dst = pal[v] | 0x8000;
            }
        }
    }
}
