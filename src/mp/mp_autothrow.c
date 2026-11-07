// Moemon Kai: AUTO THROW (the user, 2026-11-07). In a battle where L throws the last kind of ball again, holding L
// asks "Would you like to turn on Auto Throw?"; with YES, each turn throws that ball by itself until the Pokémon is
// caught, the battle ends or that ball runs out (then the menu is the player's again). L at any time turns it off.
// "AUTO THROW ON" shows at the bottom left while it's on. In co-op battles it's the same: each turn of yours throws
// the ball (at the wild Pokémon in front of you when there are two). The action menu's side is in
// battle_controller_player.c (MpAutoThrow*); this file keeps the state, L to cancel and the label.
#include "global.h"
#include "battle.h"
#include "gba/defines.h"
#include "palette.h"
#include "main.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mp.h"

u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);

static u16 sBall;                      // the ball thrown each turn, ITEM_NONE: off
static u8 sLabel[12][96];              // "AUTO THROW ON" in the small font: 1 text, 2 shadow
static u8 sLabelW;

bool8 MP_AutoThrow_Active(void)
{
    return sBall != 0;
}

u16 MP_AutoThrow_Ball(void)
{
    return sBall;
}

void MP_AutoThrow_On(u16 ball)
{
    sBall = ball;
    MPNet_Log("autothrow: on (ball %u)", ball);
}

void MP_AutoThrow_Off(const char *why)
{
    if (sBall)
        MPNet_Log("autothrow: off (%s)", why);
    sBall = 0;
}

// (each battle frame) L turns it off, whatever the battle is doing
void MP_AutoThrow_Frame(void)
{
    if (sBall && JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        MP_AutoThrow_Off("L");
    }
}

static void BuildLabel(void)
{
    static const u8 sText[] = _("AUTO THROW ON");
    int i, x = 0, gx, gy;
    if (sLabelW)
        return;
    for (i = 0; sText[i] != EOS; i++)
    {
        int w;
        if (sText[i] == CHAR_SPACE)
        {
            x += 3;
            continue;
        }
        w = MP_RenderSmallGlyph(sText[i], 1, 0, 2);
        for (gy = 0; gy < 16; gy++)
        {
            const u32 *rows = gy < 8 ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
            for (gx = 0; gx < w && gx < 16; gx++)
            {
                u8 c = (rows[(gx < 8 ? 0 : 8) + gy % 8] >> ((gx % 8) * 4)) & 0xF;
                int yy = gy - 1;   // (the small font's glyphs sit low in their 16)
                if ((c == 1 || c == 2) && yy >= 0 && yy < 12 && x + gx < 96 && (c == 1 || !sLabel[yy][x + gx]))
                    sLabel[yy][x + gx] = c;
            }
        }
        x += w;
    }
    sLabelW = (u8)(x > 96 ? 96 : x);
}

static u16 Mix(u16 a, u16 b, int t)    // t of 16 toward b
{
    int r = (a & 31) + (((b & 31) - (a & 31)) * t >> 4);
    int g = ((a >> 5) & 31) + ((((b >> 5) & 31) - ((a >> 5) & 31)) * t >> 4);
    int bl = ((a >> 10) & 31) + ((((b >> 10) & 31) - ((a >> 10) & 31)) * t >> 4);
    return (u16)(r | g << 5 | bl << 10);
}

// (over the battle's picture) the label at the bottom left, just above the message box
void MP_AutoThrow_Composite(u16 *frame)
{
    int x, y, lx = 3, ly = 99, w, h = 11;
    if (!sBall || !gMain.inBattle)
        return;
    BuildLabel();
    w = sLabelW + 6;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            u16 *dst = &frame[(ly + y) * DISPLAY_WIDTH + lx + x];
            int tx = x - 3, ty = y - 1;
            if ((x == 0 || x == w - 1) && (y == 0 || y == h - 1))
                continue;   // (rounded corners)
            *dst = Mix(*dst, RGB(3, 3, 8), 12);
            if (x == 0 || x == w - 1 || y == 0 || y == h - 1)
                *dst = Mix(*dst, RGB(31, 24, 6), 10);   // a gold rim
            if (tx >= 0 && tx < sLabelW && ty >= 0 && ty < 12 && sLabel[ty][tx])
                *dst = sLabel[ty][tx] == 1 ? RGB(31, 31, 31) : RGB(4, 4, 6);
        }
}
