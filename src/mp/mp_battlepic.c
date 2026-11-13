// Moemon Kai: a battle's background in full colour, past the GBA's tiles and palettes (the user, 2026-11-13: "this
// isn't a GBA game, this is a PC port"): UNOVA's Black scenes and SINNOH's Platinum ones (u3d_battlebg.c,
// s3d_battlebg.c) hand their picture here after loading their Emerald-format background into BG3 as usual. The
// renderer's picture hook (gba_easy_draw.c gMpBgPictureLine) then draws each line of BG3 from the picture at BG3's own
// scroll (the intro's slide, a move's shake), so everything else about the background is the game's own: its layer,
// priority, windows, blending, the move backgrounds that replace it (fadetobg loads its own BG3: the signature below
// goes and the picture stops) and Kai's battles, Doubles and Triples untouched.
// Fades follow BG3's palettes: each frame the shown palettes 2-4 against the unfaded ones give, per colour channel,
// the line c -> a + b * c that a fade or a move's tint is (as mp_mappic.c does), applied to the picture's colours.
// The picture is drawn only while the battle's BG3 is set up (char base 2, map 26) and palette 2 / 3's unused colour 0
// hold this picture's signature (another screen's palettes there turn it off).
#include <string.h>
#include "global.h"
#include "main.h"
#include "palette.h"
#include "mp.h"

extern void (*gMpBgPictureLine)(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy);
extern int gMpBgPictureBg;

#define PIC_W 512
#define PIC_H 128
#define SIG_A 0x5A3C
#define SIG_B 0x2DC3

static u16 sPic[PIC_W * PIC_H];
static bool8 sOn;
static s32 sFadeA[3], sFadeB[3];   // c -> (a + b * c) / 256, per channel
static u32 sFadeFrame;

static bool8 Showing(void)
{
    u16 cnt = REG_BG3CNT;
    return sOn && gMain.inBattle && ((cnt >> 2) & 3) == 2 && ((cnt >> 8) & 0x1F) == 26
        && gPlttBufferUnfaded[BG_PLTT_ID(2)] == SIG_A && gPlttBufferUnfaded[BG_PLTT_ID(3)] == SIG_B;
}

// This frame's fade, from BG3's palettes as shown against as they are
static void FindFade(void)
{
    const u16 *shown = (const u16 *)PLTT;
    s32 ch, i;
    for (ch = 0; ch < 3; ch++)
    {
        s32 lo = 32, hi = -1, loShown = 0, hiShown = 0;
        for (i = BG_PLTT_ID(2); i < BG_PLTT_ID(5); i++)
        {
            s32 u, s;
            if ((i & 15) == 0)
                continue;   // (the signature)
            u = (gPlttBufferUnfaded[i] >> (5 * ch)) & 31, s = (shown[i] >> (5 * ch)) & 31;
            if (u < lo)
                lo = u, loShown = s;
            if (u > hi)
                hi = u, hiShown = s;
        }
        if (hi > lo)
        {
            sFadeB[ch] = (hiShown - loShown) * 256 / (hi - lo);
            sFadeA[ch] = loShown * 256 - sFadeB[ch] * lo;
        }
        else
        {
            sFadeB[ch] = hi > 0 ? hiShown * 256 / hi : 256;
            sFadeA[ch] = 0;
        }
    }
}

static inline u16 Faded(u16 c)
{
    s32 ch, out = 0x8000;
    for (ch = 0; ch < 3; ch++)
    {
        s32 v = (sFadeA[ch] + sFadeB[ch] * ((c >> (5 * ch)) & 31) + 128) >> 8;
        out |= (v < 0 ? 0 : v > 31 ? 31 : v) << (5 * ch);
    }
    return out;
}

static void Line(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy)
{
    s32 x, py;
    const u16 *row;
    (void)dy;
    if (!Showing())
        return;
    if (line == 0 || gMain.vblankCounter1 != sFadeFrame)
    {
        sFadeFrame = gMain.vblankCounter1;
        FindFade();
    }
    py = (y0 >> 8) & 0xFF;
    row = sPic + (py < PIC_H ? py : PIC_H - 1) * PIC_W;   // (below the picture: its last line, under the text box)
    for (x = 0; x < DISPLAY_WIDTH; x++, x0 += dx)
        if (layer[x] & 0x8000)
            layer[x] = Faded(row[(x0 >> 8) & (PIC_W - 1)]);
}

// Draw PIXELS (512 x 128 BGR555, BG3's map space) as the battle's background from now on, BG3's palettes 2-4 loaded
void MP_BattlePic_Show(const u16 *pixels)
{
    memcpy(sPic, pixels, sizeof(sPic));
    sOn = TRUE;
    gPlttBufferUnfaded[BG_PLTT_ID(2)] = gPlttBufferFaded[BG_PLTT_ID(2)] = SIG_A;
    gPlttBufferUnfaded[BG_PLTT_ID(3)] = gPlttBufferFaded[BG_PLTT_ID(3)] = SIG_B;
    sFadeFrame = ~0u;
    gMpBgPictureBg = 3;
    gMpBgPictureLine = Line;
}

// Stop drawing it (Kai's own background, or a region without a picture for this battle)
void MP_BattlePic_Hide(void)
{
    sOn = FALSE;
    if (gMpBgPictureLine == Line)
    {
        gMpBgPictureLine = NULL;
        gMpBgPictureBg = -1;
    }
}
