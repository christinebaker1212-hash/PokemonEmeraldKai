// Moemon Kai: UNOVA's face-ups (Black's ov082 fld_faceup.c, script commands FLD_FACEUP 0x1D3-0x1D5; the user's approved
// mock-up 2026-11-15, docs/unova/mockups/faceup_*.png): N's big portrait over one of three backgrounds (NIMBASA's
// Ferris wheel, his castle's throne room, a white light) while he speaks, the words in the game's own message box over
// him. The DS's 256 x 192 picture is cropped to the screen's middle, never scaled. Black's order: the field fades to
// black, the picture is set and fades back in, N blends in over 30 of Black's frames; his mouth moves while his words
// print (FLD_FACEUP_UpdateTaskMain's lip sync: pictures 1, 2, 1, 0, at the text speed's pace) and he blinks every 80
// to 99 frames (0, 1, 2, 1, 0); closing fades to black, the field back, and in again. Drawn as the field's lowest
// layer (u3d.c U3dLine) with the field's people hidden, so the message box and its windows stay over him.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "palette.h"
#include "constants/rgb.h"
#include "random.h"
#include "text.h"
#include "mp.h"
#include "u3d.h"

bool8 U3D_Gfx_Faceup(u8 type, const u16 **back, const u16 **n, const u16 **mouth, const u16 **blink);

enum { FU_OFF, FU_OUT, FU_IN, FU_BLEND, FU_SHOWN, FU_CLOSE_OUT, FU_CLOSE_IN };
static struct
{
    u8 phase, type;
    u16 t;
    const u16 *back, *n, *mouth, *blink;
    struct { s8 image; u8 frame, delay, idle; } anim[2];   // (0 the mouth, 1 the blink: FaceupAnim)
    u16 blinkTimer;
    u8 tick;   // (Black's frames: one of ours in two)
} sFu;

#define CROP_X 8
#define CROP_Y 16

static void FadeTo(bool8 black)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, black ? 0 : 16, black ? 16 : 0, RGB_BLACK);
}

// FLD_FACEUP_Create + OpenEventMain: TYPE's face-up begins (FALSE without its pictures)
bool8 U3D_FaceupStart(u8 type)
{
    if (!U3D_Gfx_Faceup(type, &sFu.back, &sFu.n, &sFu.mouth, &sFu.blink))
    {
        MPNet_Log("u3d: no face-up %u in the people's pack", type);
        return FALSE;
    }
    sFu.type = type, sFu.phase = FU_OUT, sFu.t = 0;
    memset(sFu.anim, 0, sizeof(sFu.anim));
    sFu.anim[0].image = sFu.anim[1].image = -1, sFu.anim[0].idle = sFu.anim[1].idle = TRUE;
    sFu.blinkTimer = 0;
    FadeTo(TRUE);
    MPNet_Log("u3d: N's face-up %u", type);
    return TRUE;
}

// FLD_FACEUP_StartClose
void U3D_FaceupClose(void)
{
    if (sFu.phase == FU_OFF)
        return;
    sFu.phase = FU_CLOSE_OUT, sFu.t = 0;
    FadeTo(TRUE);
}

// opening: TRUE until N has blended in; closing: TRUE until the field is back
bool8 U3D_FaceupBusy(void)
{
    return sFu.phase != FU_OFF && sFu.phase != FU_SHOWN;
}

bool8 U3D_FaceupShown(void)
{
    return sFu.phase == FU_IN || sFu.phase == FU_BLEND || sFu.phase == FU_SHOWN || sFu.phase == FU_CLOSE_OUT;
}

// FLD_FACEUP_StepAnim: a part's next picture of FRAMES (COUNT of them), DELAY frames apart; idle once round
static void StepAnim(u8 k, const u8 *frames, u8 count, u8 delay)
{
    if (sFu.anim[k].delay != 0)
    {
        sFu.anim[k].delay--;
        return;
    }
    if (sFu.anim[k].frame == 0 && sFu.anim[k].idle)
        return;
    sFu.anim[k].image = frames[sFu.anim[k].frame];
    sFu.anim[k].delay = delay;
    if (++sFu.anim[k].frame >= count)
        sFu.anim[k].frame = 0, sFu.anim[k].idle = TRUE;
}

void U3D_FaceupFrame(void)
{
    static const u8 sMouth[4] = {1, 2, 1, 0}, sBlink[5] = {0, 1, 2, 1, 0};   // (gFldFaceupMouthFrames / BlinkFrames)
    if (sFu.phase == FU_OFF)
        return;
    sFu.t++;
    switch (sFu.phase)
    {
    case FU_OUT:
        if (!gPaletteFade.active)
            sFu.phase = FU_IN, sFu.t = 0, FadeTo(FALSE);
        break;
    case FU_IN:
        if (!gPaletteFade.active)
            sFu.phase = FU_BLEND, sFu.t = 0;
        break;
    case FU_BLEND:   // (FLD_FACEUP_StepFadeIn: 30 of Black's frames)
        if (sFu.t >= 60)
            sFu.phase = FU_SHOWN;
        break;
    case FU_CLOSE_OUT:
        if (!gPaletteFade.active)
            sFu.phase = FU_CLOSE_IN, sFu.t = 0, FadeTo(FALSE);
        break;
    case FU_CLOSE_IN:
        if (!gPaletteFade.active)
            sFu.phase = FU_OFF;
        return;
    }
    if (++sFu.tick & 1)
        return;
    if (IsTextPrinterActive(0))   // (his words printing: the mouth goes round again)
        sFu.anim[0].idle = FALSE;
    StepAnim(0, sMouth, 4, gSaveBlock2Ptr->optionsTextSpeed == 0 ? 3 : gSaveBlock2Ptr->optionsTextSpeed == 1 ? 2 : 1);
    if (sFu.blinkTimer == 0)
        sFu.anim[1].idle = FALSE, sFu.blinkTimer = Random() % 20 + 80;
    else
        sFu.blinkTimer--;
    StepAnim(1, sBlink, 5, 2);
}

// the face-up's line LINE into OUT (BGR555, the field's colours before its fade); FALSE when none is shown
bool8 U3D_FaceupLine(int line, u16 *out)
{
    s32 x, y = line + CROP_Y;
    u32 level = sFu.phase == FU_BLEND ? min(sFu.t * 16 / 60, 16) : sFu.phase == FU_IN ? 0 : 16;
    if (!U3D_FaceupShown())
        return FALSE;
    for (x = 0; x < DISPLAY_WIDTH; x++)
    {
        s32 dx = x + CROP_X;
        u16 c = sFu.back[y * 256 + dx], n = sFu.n[y * 256 + dx];
        // (the parts over N: the mouth at screen tiles 14, 11, the blink at 13, 8)
        if (sFu.anim[0].image >= 0 && dx >= 112 && dx < 144 && y >= 88 && y < 104)
            n = sFu.mouth[sFu.anim[0].image * 512 + (y - 88) * 32 + dx - 112];
        else if (sFu.anim[1].image >= 0 && dx >= 104 && dx < 152 && y >= 64 && y < 80)
            n = sFu.blink[sFu.anim[1].image * 768 + (y - 64) * 48 + dx - 104];
        if (n & 0x8000)
        {
            if (level >= 16)
                c = n;
            else   // (BLDCNT: N at LEVEL sixteenths over the background)
            {
                u32 ch;
                u16 m = 0;
                for (ch = 0; ch < 3; ch++)
                    m |= ((((n >> (5 * ch)) & 31) * level + ((c >> (5 * ch)) & 31) * (16 - level)) / 16) << (5 * ch);
                c = m | 0x8000;
            }
        }
        out[x] = c & 0x7FFF;
    }
    return TRUE;
}
