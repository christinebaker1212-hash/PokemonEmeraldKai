// Moemon Co-op: emotes and quick phrases in battle.
//
// Holding LT / RT (Q / E on the keyboard) in a battle opens a small pick at the
// top of the screen with the same pages as the overworld quick pick (EMOTES,
// MINE, the ready-made groups). The D-Pad picks, L / R change the page, letting
// go of the trigger (or A) sends, B closes. Phrases can't be edited here.
//
// What you send pops up as a bubble over your Moemon, on your screen and on
// your co-op partner's or PvP opponent's (mp_emote.c MP_Emote_BattleSay). While
// the pick is open the battle doesn't see the buttons (main.c ReadKeys).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "battle_anim.h"
#include "link.h"
#include "main.h"
#include "palette.h"
#include "pokemon.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "platform/gamepad.h"
#include "constants/songs.h"
#include "constants/characters.h"

u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define PICK_W 128
#define PICK_H 64
#define TAG_PICK 0x53F0   // + half
#define LINE_H 12

// Colors of the shared overlay palette (mp_emote.c sBubblePal)
enum { C_CLEAR, C_WHITE, C_DARK, C_GREY, C_EDGE, C_PANEL, C_TEXT, C_SHADOW, C_BORDER, C_BAND };

static struct
{
    bool8 open;
    u8 page;
    s8 sel;        // -1 nothing yet, else 0 up, 1 right, 2 down, 3 left
    s8 hold;       // the trigger holding it open (0 LT/Q, 1 RT/E)
    u8 spriteId[2];
    bool8 dirty;
    u16 newKeys, heldKeys;
} sPick = {.spriteId = {MAX_SPRITES, MAX_SPRITES}};

static u8 sPixels[PICK_W * PICK_H];
static u8 sTiles[2][64 * 64 / 2];

static const struct OamData sOam_Pick = {
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 0,
};

static void SpriteCB_Pick(struct Sprite *sprite)
{
}

// ---------------------------------------------------------------------------
// Who's who in this battle

// The battler of this game's player
u8 MP_BattleChat_LocalBattler(void)
{
    u8 b;
    if ((gBattleTypeFlags & BATTLE_TYPE_MULTI) && (gBattleTypeFlags & BATTLE_TYPE_LINK))
    {
        u8 half = GetMultiplayerId() == 0 ? 0 : 1;
        for (b = 0; b < gBattlersCount; b++)
            if (GetBattlerSide(b) == B_SIDE_PLAYER && gBattlerPartyIndexes[b] / MULTI_PARTY_SIZE == half)
                return b;
    }
    return GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
}

// The battler of another player who's in this battle with us (MAX_BATTLERS_COUNT: not in it)
u8 MP_BattleChat_BattlerOf(u8 playerId)
{
    u8 local, b;
    if (!gMain.inBattle)
        return MAX_BATTLERS_COUNT;
    if (gMpCoopBattle && MP_Coop_PartnerId() == playerId)
    {
        local = MP_BattleChat_LocalBattler();
        for (b = 0; b < gBattlersCount; b++)
            if (b != local && GetBattlerSide(b) == B_SIDE_PLAYER)
                return b;
        return MAX_BATTLERS_COUNT;
    }
    if (!gMpCoopBattle && MP_Battle_PeerId() == playerId)
        return GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
    return MAX_BATTLERS_COUNT;
}

// ---------------------------------------------------------------------------
// Drawing

static void Put(int x, int y, u8 c)
{
    if (x >= 0 && x < PICK_W && y >= 0 && y < PICK_H)
        sPixels[y * PICK_W + x] = c;
}

static int GlyphW(u16 glyph)
{
    if (glyph == CHAR_SPACE)
        return 3;
    return MP_RenderSmallGlyph(glyph, C_TEXT, C_CLEAR, C_SHADOW);
}

static int Glyph(int x, int y, u16 glyph, u8 fg, u8 shadow)
{
    int w, gx, gy;
    if (glyph == CHAR_SPACE)
        return 3;
    w = MP_RenderSmallGlyph(glyph, fg, C_CLEAR, shadow);
    for (gy = 0; gy < 16; gy++)
    {
        const u32 *rows = (gy < 8) ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
        for (gx = 0; gx < w && gx < 16; gx++)
        {
            u8 c = (rows[(gx < 8 ? 0 : 8) + (gy % 8)] >> ((gx % 8) * 4)) & 0xF;
            if (c)
                Put(x + gx, y + gy, c);
        }
    }
    return w;
}

// One line of game text, cut with ".." to fit maxW
static void Text(int x, int y, const u8 *s, int maxW, u8 fg, u8 shadow)
{
    int i, w = 0, dots = 2 * GlyphW(CHAR_PERIOD);
    for (i = 0; s[i] != EOS; i++)
    {
        u8 c = s[i] == CHAR_NEWLINE ? CHAR_SPACE : s[i];
        int gw = GlyphW(c), j, rest = 0;
        for (j = i + 1; s[j] != EOS; j++)
            rest += GlyphW(s[j] == CHAR_NEWLINE ? CHAR_SPACE : s[j]);
        if (w + gw + (rest ? dots : 0) > maxW && w + gw + rest > maxW)
        {
            Glyph(x + w, y, CHAR_PERIOD, fg, shadow);
            Glyph(x + w + dots / 2, y, CHAR_PERIOD, fg, shadow);
            return;
        }
        w += Glyph(x + w, y, c, fg, shadow);
    }
}

static int TextW(const u8 *s)
{
    int i, w = 0;
    for (i = 0; s[i] != EOS; i++)
        w += GlyphW(s[i] == CHAR_NEWLINE ? CHAR_SPACE : s[i]);
    return w;
}

static void Draw(void)
{
    static const u8 sArrows[4] = {CHAR_UP_ARROW, CHAR_RIGHT_ARROW, CHAR_DOWN_ARROW, CHAR_LEFT_ARROW};
    char name[16];
    u8 buf[100];
    int x, y, d, i;

    memset(sPixels, 0, sizeof(sPixels));
    // Panel with a band along the top for the page name
    for (y = 0; y < PICK_H; y++)
        for (x = 0; x < PICK_W; x++)
        {
            bool8 edge = (x == 0 || y == 0 || x == PICK_W - 1 || y == PICK_H - 1);
            bool8 corner = (x == 0 || x == PICK_W - 1) && (y == 0 || y == PICK_H - 1);
            if (!corner)
                Put(x, y, edge ? C_EDGE : (y <= 13 ? C_DARK : C_WHITE));
        }
    MP_Quick_PageName(sPick.page, name, sizeof(name));
    MP_AsciiToGame(buf, name, 20);
    Text(4, 0, buf, 80, C_WHITE, C_GREY);
    MP_AsciiToGame(buf, "L/R", 8);
    Text(PICK_W - 4 - TextW(buf), 0, buf, 30, C_WHITE, C_GREY);

    for (d = 0; d < 4; d++)
    {
        u8 emote, kind;
        bool8 sel = sPick.sel == d;
        u8 fg = sel ? C_WHITE : C_DARK, sh = C_GREY;
        y = 14 + d * LINE_H + 1;
        if (sel)
            for (i = y; i < y + LINE_H && i < PICK_H - 1; i++)
                for (x = 1; x < PICK_W - 1; x++)
                    Put(x, i, C_EDGE);
        Glyph(4, y - 2, sArrows[d], fg, sh);
        kind = MP_Quick_Choice(sPick.page, d, &emote, buf, sizeof(buf), TRUE);
        if (kind == MP_QUICK_NONE)
            MP_AsciiToGame(buf, "- empty -", 12);
        Text(14, y - 2, buf, PICK_W - 18, kind == MP_QUICK_NONE ? C_GREY : fg, kind == MP_QUICK_NONE ? C_CLEAR : sh);
    }

    memset(sTiles, 0, sizeof(sTiles));
    for (i = 0; i < 2; i++)
    {
        int tx, ty, px, py;
        for (ty = 0; ty < 8; ty++)
            for (tx = 0; tx < 8; tx++)
            {
                u8 *tile = sTiles[i] + (ty * 8 + tx) * 32;
                for (py = 0; py < 8; py++)
                    for (px = 0; px < 8; px++)
                    {
                        u8 c = sPixels[(ty * 8 + py) * PICK_W + i * 64 + tx * 8 + px];
                        int k = py * 8 + px;
                        tile[k / 2] |= (k & 1) ? (c << 4) : c;
                    }
            }
    }
    sPick.dirty = FALSE;
}

static void DestroySprites(void)
{
    u8 i;
    for (i = 0; i < 2; i++)
    {
        u8 s = sPick.spriteId[i];
        if (s < MAX_SPRITES && gSprites[s].inUse && gSprites[s].callback == SpriteCB_Pick)
            DestroySprite(&gSprites[s]);
        sPick.spriteId[i] = MAX_SPRITES;
        FreeSpriteTilesByTag(TAG_PICK + i);
    }
}

static bool8 ShowSprites(void)
{
    u8 i;
    Draw();
    for (i = 0; i < 2; i++)
    {
        u8 s = sPick.spriteId[i];
        if (s < MAX_SPRITES && gSprites[s].inUse && gSprites[s].callback == SpriteCB_Pick)
        {
            u16 start = GetSpriteTileStartByTag(TAG_PICK + i);
            if (start != 0xFFFF)
                memcpy((void *)(OBJ_VRAM0 + start * TILE_SIZE_4BPP), sTiles[i], sizeof(sTiles[i]));
            continue;
        }
        {
            struct SpriteSheet sheet = {sTiles[i], sizeof(sTiles[i]), TAG_PICK + i};
            struct SpriteTemplate tmpl = {
                .tileTag = TAG_PICK + i, .paletteTag = MP_TAG_UI_PAL, .oam = &sOam_Pick,
                .anims = gDummySpriteAnimTable, .images = NULL,
                .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCB_Pick,
            };
            if (!MP_Emote_LoadUiPalette())
                return FALSE;
            FreeSpriteTilesByTag(TAG_PICK + i);
            if (LoadSpriteSheet(&sheet) == 0)
                return FALSE;
            s = CreateSprite(&tmpl, DISPLAY_WIDTH / 2 - 32 + i * 64, 38, 0);
            if (s == MAX_SPRITES)
                return FALSE;
            gSprites[s].coordOffsetEnabled = FALSE;
            sPick.spriteId[i] = s;
        }
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
// Opening, input, sending

bool8 MP_BattleChat_IsOpen(void)
{
    return sPick.open;
}

void MP_BattleChat_Open(u8 page, s8 hold)
{
    if (sPick.open || !gMain.inBattle)
        return;
    sPick.page = MP_Quick_StartPage(page);
    sPick.sel = -1;
    sPick.hold = hold;
    sPick.open = TRUE;
    if (!ShowSprites())
    {
        DestroySprites();
        sPick.open = FALSE;
        return;
    }
    PlaySE(SE_WIN_OPEN);
}

static void Close(bool8 send)
{
    u8 emote, kind, text[100];
    DestroySprites();
    sPick.open = FALSE;
    if (!send || sPick.sel < 0)
        return;
    kind = MP_Quick_Choice(sPick.page, sPick.sel, &emote, text, sizeof(text), FALSE);
    if (kind == MP_QUICK_EMOTE)
    {
        static const char *const sEmoteText[] = {"!", "?"};
        // ! and ? as text, the heart and the music note as pictures
        switch (emote)
        {
        case 0:
        case 1:
            MP_AsciiToGame(text, sEmoteText[emote], 4);
            MP_Emote_BattleSay(text, MP_BUBBLE_ICON_NONE);
            break;
        case 2:
            MP_Emote_BattleSay(NULL, MP_BUBBLE_ICON_HEART);
            break;
        default:
            MP_Emote_BattleSay(NULL, MP_BUBBLE_ICON_NOTE);
            break;
        }
    }
    else if (kind == MP_QUICK_TEXT)
    {
        MP_Emote_BattleSay(text, MP_BUBBLE_ICON_NONE);
    }
}

static void Update(void)
{
    u16 newKeys = sPick.newKeys, held = sPick.heldKeys;
    s8 sel = sPick.sel;
    if (!gMain.inBattle)
    {
        Close(FALSE);
        return;
    }
    if (held & DPAD_UP)
        sel = 0;
    else if (held & DPAD_RIGHT)
        sel = 1;
    else if (held & DPAD_DOWN)
        sel = 2;
    else if (held & DPAD_LEFT)
        sel = 3;
    if (sel != sPick.sel)
    {
        sPick.sel = sel;
        PlaySE(SE_SELECT);
        sPick.dirty = TRUE;
    }
    if (newKeys & B_BUTTON)
    {
        PlaySE(SE_SELECT);
        Close(FALSE);
        return;
    }
    if ((newKeys & A_BUTTON) || (sPick.hold >= 0 && !Gamepad_TriggerHeld(sPick.hold)))
    {
        u8 emote, buf[4];
        if (sPick.sel >= 0 && MP_Quick_Choice(sPick.page, sPick.sel, &emote, buf, 0, FALSE) == MP_QUICK_NONE)
        {
            PlaySE(SE_FAILURE);
            if (sPick.hold >= 0 && !Gamepad_TriggerHeld(sPick.hold))
                Close(FALSE);
            return;
        }
        Close(TRUE);
        return;
    }
    if (newKeys & (L_BUTTON | R_BUTTON))
    {
        u8 n = MP_Quick_PageCount();
        do
            sPick.page = (sPick.page + ((newKeys & R_BUTTON) ? 1 : n - 1)) % n;
        while (!MP_Quick_PageShown(sPick.page));
        MP_Quick_SetPhrasePage(sPick.page);
        PlaySE(SE_SELECT);
        sPick.dirty = TRUE;
    }
    if (sPick.dirty && !ShowSprites())
        Close(FALSE);
}

// main.c ReadKeys: while the pick is open it takes the buttons, the battle sees none
void MP_BattleChat_FilterKeys(void)
{
    if (!sPick.open)
        return;
    sPick.newKeys = gMain.newKeys;
    sPick.heldKeys = gMain.heldKeys;
    gMain.newKeys = 0;
    gMain.heldKeys = 0;
    gMain.newAndRepeatedKeys = 0;
    Update();
}
