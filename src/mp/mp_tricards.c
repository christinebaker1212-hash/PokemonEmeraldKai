// Moemon Co-op: the compact see-through cards of a Triple Battle.
//
// Six vanilla healthboxes don't fit the GBA's sprite memory next to six Moemon,
// and a GBA can't blend a sprite over another sprite. So in a Triple Battle the
// game's own healthbox objects still exist (every battle routine keeps using
// them), parked off-screen (InitBattlerHealthboxCoords), and these cards are
// drawn by the port over the finished frame (MP_TriCards_Composite, from VDraw).
//
// The cards are the user's pixel art (tools/moemon_tricards: mp_tricard_gfx.h): a
// 74x22 card with a tail toward the type icons, name and gender on row 1, the
// level and the original game's HP bar (with a small HP label) on row 2. A status
// tag covers the level, as in the original healthbox, and the bar gets shorter to
// fit it. Foes are stacked top-left, the team right above the text box; the
// team's cards ramp from 38% to 0% see-through (the top one sits over the right
// foe). Only the fill and the frame are blended: text, gender, bar and status tag
// stay solid. The colors fade with the screen like the battle's own sprites (a
// palette loaded for the battle is the reference: MP_FadeFit).
#include <string.h>
#include "global.h"
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
#include "constants/species.h"
#include "data.h"
#include "mp.h"

#include "mp_tricard_gfx.h"

#define CARD_W TRICARD_FRAME_W
#define CARD_H TRICARD_FRAME_H
#define CARD_STEP 21
// where the frames go: the foes' card body at x 1, the team's at x 165 (the mock-up)
#define FOE_X (1 - TRICARD_FOE_BODY)
#define FOE_Y 1
#define TEAM_X (165 - TRICARD_TEAM_BODY)
#define TEAM_Y 48

// card rows (from the card's top)
#define ROW_NAME 3
#define ROW_LEVEL 11
#define ROW_BAR 12
#define INSIDE_RIGHT 70   // the last column inside the ring, from the body's left

#define TAG_TRICARD_PAL MP_TRICARD_PAL_TAG

// The fade reference: a sprite palette of the card's own colors, loaded for the battle
static const u16 sCardPal[16] = {
    RGB(0, 0, 0), RGB(31, 31, 28), RGB(10, 13, 12), RGB(15, 18, 16), RGB(7, 10, 8), RGB(4, 7, 0), RGB(27, 26, 22), RGB(24, 23, 14),
    RGB(9, 9, 10), RGB(31, 19, 18), RGB(8, 25, 31), RGB(10, 13, 11), RGB(26, 25, 23), RGB(31, 31, 31), RGB(11, 26, 16), RGB(14, 31, 21),
};

static const struct SpritePalette sCardSpritePal = {sCardPal, TAG_TRICARD_PAL};

enum { GENDER_NONE, GENDER_MALE, GENDER_FEMALE };

struct Card
{
    bool8 used;
    bool8 dirty;
    u8 name[POKEMON_NAME_LENGTH + 1];
    u8 letter;          // 0, or A/B/C when a card on the same side has the same name
    u8 gender;          // GENDER_*
    u8 level;
    u8 status;          // 0 none, 1 PSN, 2 PAR, 3 SLP, 4 FRZ, 5 BRN
    bool8 team;         // the team's card (tail on the left)
    s32 hp, maxHp;
    u8 pix[CARD_H][CARD_W];   // TC_* roles
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

// Draws str at (x, y) of the card: font pixel 1 = color, 3 = shadow (up to maxX, not
// included). Returns the x after it.
static u8 DrawText(struct Card *c, const u8 *str, u8 x, u8 y, u8 maxX, u8 color)
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
                c->pix[y + py][x + px] = (v == 1) ? color : TC_SHADOW;
                if (x + px + 1 > right)
                    right = x + px + 1;
            }
        }
    }
    RemoveWindow(winId);
    return right;
}

// The original game's 20x8 status tag at (x, y). Returns the x after it.
static u8 DrawStatusTag(struct Card *c, u8 x, u8 y, u8 status)
{
    int px, py;
    for (py = 0; py < 8; py++)
        for (px = 0; px < 20; px++)
            c->pix[y + py][x + px] = sTriCardStatus[status - 1][py][px];
    return x + 20;
}

// HP the way the original bar counts it: 48 pixels, at least 1 while any is left
static u8 HpPixels48(s32 hp, s32 maxHp)
{
    s32 px;
    if (maxHp <= 0 || hp <= 0)
        return 0;
    px = hp * 48 / maxHp;
    return px == 0 ? 1 : px;
}

// The HP bar from x to the inside of the ring (right): the left end with its HP label,
// the fill, the right end. Green, yellow and red at the original game's thresholds.
static void DrawBar(struct Card *c, int x, int right)
{
    int width = right - 1 - (x + 12);   // fill columns x+12 .. right-2
    int filled = 0, i, py;
    u8 top = TC_GREEN_TOP, bottom = TC_GREEN_BOTTOM, px48;

    if (width < 1)
        return;
    px48 = HpPixels48(c->hp, c->maxHp);
    if (px48)
    {
        filled = c->hp * width / c->maxHp;
        if (filled < 1)
            filled = 1;
        if (filled > width)
            filled = width;
        if (px48 <= 9)
            top = TC_RED_TOP, bottom = TC_RED_BOTTOM;
        else if (px48 <= 24)
            top = TC_YELLOW_TOP, bottom = TC_YELLOW_BOTTOM;
    }
    for (py = 0; py < 6; py++)
    {
        for (i = 0; i < 12; i++)
            c->pix[ROW_BAR + py][x + i] = sTriCardBarLeft[py][i];
        for (i = 0; i < 2; i++)
            c->pix[ROW_BAR + py][right - 1 + i] = sTriCardBarRight[py][i];
    }
    for (i = 0; i < width; i++)
    {
        u8 *col = &c->pix[ROW_BAR][x + 12 + i];
        bool8 full = i < filled;
        col[0 * CARD_W] = TC_BAR_EDGE;
        col[1 * CARD_W] = TC_BAR_WHITE;
        col[2 * CARD_W] = full ? top : TC_EMPTY_TOP;
        col[3 * CARD_W] = full ? bottom : TC_EMPTY_BOTTOM;
        col[4 * CARD_W] = TC_BAR_WHITE;
        col[5 * CARD_W] = TC_BAR_EDGE;
    }
}

static void Render(struct Card *c)
{
    u8 body = c->team ? TRICARD_TEAM_BODY : TRICARD_FOE_BODY;
    u8 lx = body + 4, right = body + INSIDE_RIGHT, nameMax = right + 1, x, lv[8];

    memcpy(c->pix, c->team ? sTriCardTeamFrame : sTriCardFoeFrame, sizeof(c->pix));
    // row 1: the name and the gender, and the letter at the right when names repeat
    if (c->letter)
    {
        u8 tag[2] = {c->letter, EOS};
        DrawText(c, tag, right - 6, ROW_NAME, right + 1, TC_TEXT);
        nameMax = right - 7;
    }
    x = DrawText(c, c->name, lx, ROW_NAME, nameMax, TC_TEXT);
    if (c->gender != GENDER_NONE)
    {
        static const u8 sMale[] = {CHAR_MALE, EOS}, sFemale[] = {CHAR_FEMALE, EOS};
        x = lx + GetStringWidth(FONT_SMALL, c->name, 0) + 1;   // one pixel apart (the mock-up)
        DrawText(c, c->gender == GENDER_MALE ? sMale : sFemale, x, ROW_NAME, nameMax,
                 c->gender == GENDER_MALE ? TC_MALE : TC_FEMALE);
    }
    // row 2: the level, or the status tag over it; then the HP bar to the right end
    if (c->status)
    {
        x = DrawStatusTag(c, lx, ROW_LEVEL, c->status);
    }
    else
    {
        lv[0] = CHAR_EXTRA_SYMBOL;
        lv[1] = CHAR_LV_2;
        ConvertIntToDecimalStringN(lv + 2, c->level, STR_CONV_MODE_LEFT_ALIGN, 3);
        x = DrawText(c, lv, lx, ROW_LEVEL, right + 1, TC_TEXT);
    }
    DrawBar(c, x + 1, right);
    c->dirty = FALSE;
}

// ---------------------------------------------------------------------------

void MP_TriCards_Begin(void)
{
    memset(sCards, 0, sizeof(sCards));
    if (IndexOfSpritePaletteTag(TAG_TRICARD_PAL) == 0xFF)
        LoadSpritePalette(&sCardSpritePal);
    sActive = TRUE;
}

// The battle screen came back (bag, party...): FreeAllSpritePalettes dropped ours. A battle that became a Triple
// Battle while it ran (friends joined, mp_join.c) starts its cards here: the healthboxes being made fill them in.
void MP_TriCards_Reload(void)
{
    if (!sActive)
    {
        MP_TriCards_Begin();
        return;
    }
    if (IndexOfSpritePaletteTag(TAG_TRICARD_PAL) == 0xFF)
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
    c->team = GetBattlerSide(battler) == B_SIDE_PLAYER;
    // the gender as on the original healthbox (a NIDORAN♀/♂ under its own name already shows it)
    switch (GetMonGender(mon))
    {
    case MON_MALE:   c->gender = GENDER_MALE; break;
    case MON_FEMALE: c->gender = GENDER_FEMALE; break;
    default:         c->gender = GENDER_NONE; break;
    }
    {
        u16 species = GetMonData(mon, MON_DATA_SPECIES);
        if ((species == SPECIES_NIDORAN_F || species == SPECIES_NIDORAN_M) && StringCompare(c->name, gSpeciesNames[species]) == 0)
            c->gender = GENDER_NONE;
    }
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

// Same-named Pokémon on one side get the letters A, B, C in screen order (left
// to right), so the target box and the cards tell them apart. A fainted one keeps
// its card name until it is replaced, so the letters don't shift mid-battle.
static const u8 sLetterOrder[2][3] = {
    {B_POSITION_PLAYER_LEFT, B_POSITION_PLAYER_CENTER, B_POSITION_PLAYER_RIGHT},
    {B_POSITION_OPPONENT_RIGHT, B_POSITION_OPPONENT_CENTER, B_POSITION_OPPONENT_LEFT},
};

u8 MP_TriCards_Letter(u8 battler)
{
    u8 side, i, n = 0, mine = 0xFF;

    if (!sActive || !IS_TRIPLE_BATTLE() || battler >= gBattlersCount || !sCards[battler].used)
        return 0;
    side = GetBattlerSide(battler);
    for (i = 0; i < 3; i++)
    {
        u8 b = GetBattlerAtPosition(sLetterOrder[side][i]);
        if (b >= gBattlersCount || !sCards[b].used || StringCompare(sCards[b].name, sCards[battler].name) != 0)
            continue;
        if (b == battler)
            mine = n;
        n++;
    }
    if (n < 2 || mine == 0xFF)
        return 0;
    return CHAR_A + mine;
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

// A sprite drawn again over the frame (the emote pick and bubbles go over the cards)
static void RedrawSprite(u16 *frame, const struct Sprite *s)
{
    static const u8 sDims[3][4][2] = {
        {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
        {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
        {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
    };
    const u8 *tiles;
    const u16 *pal;
    int w, h, x0, y0, px, py;

    if (!s->inUse || s->invisible || s->oam.affineMode != ST_OAM_AFFINE_OFF || s->oam.shape > 2
     || s->oam.bpp != ST_OAM_4BPP || s->oam.objMode == ST_OAM_OBJ_WINDOW)
        return;
    w = sDims[s->oam.shape][s->oam.size][0];
    h = sDims[s->oam.shape][s->oam.size][1];
    x0 = s->oam.x >= 256 ? s->oam.x - 512 : s->oam.x;
    y0 = s->oam.y >= 160 ? s->oam.y - 256 : s->oam.y;
    tiles = (const u8 *)(OBJ_VRAM0) + s->oam.tileNum * TILE_SIZE_4BPP;
    pal = (const u16 *)(OBJ_PLTT) + s->oam.paletteNum * 16;
    for (py = 0; py < h; py++)
    {
        int sy = y0 + py, ty = s->oam.matrixNum & ST_OAM_VFLIP ? h - 1 - py : py;
        if (sy < 0 || sy >= DISPLAY_HEIGHT)
            continue;
        for (px = 0; px < w; px++)
        {
            int sx = x0 + px, tx = s->oam.matrixNum & ST_OAM_HFLIP ? w - 1 - px : px;
            u8 v;
            if (sx < 0 || sx >= DISPLAY_WIDTH)
                continue;
            v = tiles[((ty / 8) * (w / 8) + tx / 8) * TILE_SIZE_4BPP + (ty % 8) * 4 + (tx % 8) / 2];
            v = (tx & 1) ? (v >> 4) : (v & 0xF);
            if (v)
                frame[sy * DISPLAY_WIDTH + sx] = pal[v] | 0x8000;
        }
    }
}

// For the type icons (mp_typeicons.c): where battler's card body is drawn this frame (its
// outer ring's left and right columns and top row), FALSE when it isn't
bool8 MP_TriCards_Place(u8 battler, s16 *left, s16 *right, s16 *top)
{
    struct Sprite *box;
    s16 x0, y0;
    u16 opacity;
    if (!sActive || battler >= MAX_BATTLERS_COUNT || !sCards[battler].used || gHealthboxSpriteIds[battler] >= MAX_SPRITES)
        return FALSE;
    box = &gSprites[gHealthboxSpriteIds[battler]];
    if (!box->inUse || box->invisible || !CardPlace(battler, &x0, &y0, &opacity))
        return FALSE;
    *left = x0 + box->x2 + (GetBattlerSide(battler) == B_SIDE_PLAYER ? TRICARD_TEAM_BODY : TRICARD_FOE_BODY);
    *right = *left + TRICARD_BODY_W - 1;
    *top = y0 + box->y2;
    return TRUE;
}

// After the GBA frame is drawn (platform VDraw): the cards over it
void MP_TriCards_Composite(u16 *frame)
{
    struct MpFadeFit fit;
    u16 colors[TC_COUNT];
    u8 slot, b, i;

    if (!sActive || !IS_TRIPLE_BATTLE() || !gMain.inBattle || gMain.callback2 != BattleMainCB2 || !MP_OBJ_LAYER_SHOWN())
        return;
    slot = IndexOfSpritePaletteTag(TAG_TRICARD_PAL);
    if (slot == 0xFF)
        return;
    // the card's colors under the same fade as the battle's sprites
    MP_FadeFit_FromPalette(&fit, OBJ_PLTT_ID(slot));
    for (i = 0; i < TC_COUNT; i++)
        colors[i] = MP_FadeFit_Apply(&fit, sTriCardColors[i]);
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
                sLastState[b] = st;
            }
        }
        if (!box->inUse || box->invisible)
            continue;
        {
            u8 letter = MP_TriCards_Letter(b);
            if (letter != c->letter)
            {
                c->letter = letter;
                c->dirty = TRUE;
            }
        }
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
                if (v == TC_NONE || sx < 0 || sx >= DISPLAY_WIDTH)
                    continue;
                dst = &frame[sy * DISPLAY_WIDTH + sx];
                switch (v)
                {
                case TC_FILL:
                    *dst = Blend(colors[v], *dst, opacity);
                    break;
                case TC_OUTER:
                case TC_OUTER_AA:
                case TC_CORNER:
                case TC_DARK:
                case TC_RING:
                case TC_RING_CORNER:
                    *dst = Blend(colors[v], *dst, edgeOpacity);
                    break;
                default:
                    *dst = colors[v] | 0x8000;
                    break;
                }
            }
        }
    }
}

// Any battle (Singles and Doubles too): the emote pick and battle bubbles painted
// again over the finished frame, so cards, HP boxes, text boxes and animation
// backgrounds never cover them (platform VDraw, after MP_TriCards_Composite)
void MP_BattleOverlay_Composite(u16 *frame)
{
    u8 ids[16], n, i;
    if (!gMain.inBattle || gMain.callback2 != BattleMainCB2)
        return;
    n = MP_Emote_BattleOverlaySprites(ids, 12);
    n += MP_BattleChat_OverlaySprites(ids + n, 16 - n);
    for (i = 0; i < n; i++)
        RedrawSprite(frame, &gSprites[ids[i]]);
}
