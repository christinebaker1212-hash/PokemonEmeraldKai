// Moemon Kai: Platinum's time of day in the 2D regions (HOENN, JOHTO, KANTO; the user, 2026-11-02: it must look like
// Platinum's tinting), and on SINNOH's sprites (its 3D world is lit by s3d.c itself). SINNOH's 3D maps are lit by Platinum's area lights for the hour (src/sinnoh/s3d.c); here the
// same lights give a color per half hour (sinnoh/platinum_daylight.bin, tools/sinnoh/build_daylight.py: flat ground
// under Platinum's outdoor light set, over noon's), and on an outdoor map the map's layers (BG1-3) and the sprites in
// the world (OAM priority 1-3: people, POKéMON, field effects; not the text boxes, menus and co-op tags) are drawn in
// that color through the renderer's scene hook (gba_easy_draw.c gMpScenePixel), eased from one half hour to the next.
// Without the file (no SINNOH pack built) nothing is tinted.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "mp_weather.h"
#include "main.h"
#include "overworld.h"
#include "rtc.h"
#include "sprite.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"
#include "mp_uistyle.h"

const char *Updater_ExeFile(const char *name);

static u8 sTable[48][3];
static bool8 sLoaded, sTried;
static u16 sMul[3];   // this frame's share of each channel, 256 = all
static u16 sLayerMul[3];   // ... for the map's layers (SINNOH's 3D world: only the sun's, it lights the hour itself)
static u16 sSunAmount;   // the hourly weather's bright sun, 0-256 (mp_weather.c)
static const u16 sSun[3] = {292, 276, 246};   // ... its warm light, at its brightest

static void Load(void)
{
    FILE *f;
    u8 head[4];
    const char *path = getenv("MOEMON_DAYLIGHT");
    sTried = TRUE;
    if (getenv("MOEMON_TEST") && !getenv("MOEMON_DAYTINT"))
        return;   // (tests: off unless asked, so pictures compared pixel for pixel stay as they were)
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum_daylight.bin");
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    if (fread(head, 1, 4, f) == 4 && head[0] == 'P' && head[1] == 'L' && head[2] == 'D' && head[3] == 'L'
     && fread(sTable, 1, sizeof(sTable), f) == sizeof(sTable))
        sLoaded = TRUE;
    fclose(f);
}

static u8 sRedStep;   // RED's DAY / NIGHT: 0 day, 1 the lightest shade a step darker (dawn, dusk), 2 every shade (night)

static u16 DayTintPixel(u16 c, int x, int y, bool8 sprite)
{
    u16 out;
    u32 r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    if (sRedStep != 0xFF && MP_GbPal_RedNight(c, sRedStep, &out))
        return (c & 0x8000) | out;   // (RED's own shades: darkened by whole shades, as Red's fades)
    {
        const u16 *mul = sprite ? sMul : sLayerMul;
        r = r * mul[0] >> 8;
        g = g * mul[1] >> 8;
        b = b * mul[2] >> 8;
    }
    return (c & 0x8000) | (r > 31 ? 31 : r) | ((g > 31 ? 31 : g) << 5) | ((b > 31 ? 31 : b) << 10);
}

static bool8 Wanted(void)
{
    // (CB2_OverworldBasic: the field drawn under a battle's opening flash and transition, the tint stays on it)
    if ((gMain.callback2 != CB2_Overworld && gMain.callback2 != CB2_OverworldBasic) || gMain.inBattle)
        return FALSE;
    if (MP_UiStyle_Active() == UISTYLE_RED && !MP_Gb_On(MP_GB_DAYNIGHT))
        return FALSE;   // (Red had no time of day: none is drawn in its style, the user 2026-11-18, unless GB OPTIONS > DAY / NIGHT)
    return IsMapTypeOutdoors(gMapHeader.mapType);
}

// sdl2.c VDraw, after the battle weather's (which holds the hook in battle): the tint for the next frame
void MP_DayTint_Update(void)
{
    u32 half, next, ch, mins, i;
    if (!sTried)
        Load();
    // Moemon Co-op: the hourly weather's bright sun, a warm light eased in and out over a second or two
    // (mp_weather.c), with or without the day's tint
    if (MP_Weather_GentleSun() && Wanted())
        sSunAmount = sSunAmount + 4 > 256 ? 256 : sSunAmount + 4;
    else
        sSunAmount = sSunAmount < 4 ? 0 : sSunAmount - 4;
    if ((!sLoaded && !sSunAmount) || !Wanted())
    {
        if (gMpScenePixel == DayTintPixel)
            gMpScenePixel = NULL;
        return;
    }
    if (gMpScenePixel != NULL && gMpScenePixel != DayTintPixel)
        return;   // (another scene effect holds it)
    RtcCalcLocalTime();
    mins = gLocalTime.hours * 60 + gLocalTime.minutes;
    if (S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) && getenv("MOEMON_S3D_HOUR"))
        mins = atoi(getenv("MOEMON_S3D_HOUR")) * 60;   // (tests: SINNOH's lighting hour, as its 3D world has it)
    half = (mins / 30) % 48;
    next = (half + 1) % 48;
    for (ch = 0; ch < 3; ch++)   // (eased between half hours; 128 in the table = all of it)
        sMul[ch] = sLoaded ? (sTable[half][ch] * (30 - mins % 30) + sTable[next][ch] * (mins % 30)) * 2 / 30 : 256;
    for (ch = 0; ch < 3; ch++)
    {
        u32 sun = (256 * 256 + (sSun[ch] - 256) * sSunAmount) >> 8;
        sLayerMul[ch] = S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) ? sun : sMul[ch] * sun >> 8;
        sMul[ch] = sMul[ch] * sun >> 8;
    }
    sRedStep = 0xFF;
    if (MP_UiStyle_Active() == UISTYLE_RED)
    {
        u32 dark = 256 - ((sMul[0] * 77 + sMul[1] * 151 + sMul[2] * 28) >> 8);   // (Platinum's night ~117, dusk ~80)
        sRedStep = dark >= 96 ? 2 : dark >= 56 ? 1 : 0;
    }
    memset(gMpSceneObjPal, 0, sizeof(gMpSceneObjPal));
    for (i = 0; i < MAX_SPRITES; i++)
        if (gSprites[i].inUse && !gSprites[i].invisible && gSprites[i].oam.priority >= 1)
            gMpSceneObjPal[gSprites[i].oam.paletteNum & 63] = 1;
    // SINNOH's 3D world is lit by the area lights itself (s3d.c): there only its sprites take the tint (the user,
    // 2026-11-02: the light should tint the sprites as well)
    gMpSceneLayers = S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) && !sSunAmount ? 0 : 0xE;
    gMpScenePixel = DayTintPixel;
}

// Tests (mp daytint): the tint now
void MP_DayTint_Debug(void)
{
    if (!sTried)
        Load();
    printf("[test] daytint: %s, on %d, mul %u %u %u, red step %d\n", sLoaded ? "loaded" : "none", gMpScenePixel == DayTintPixel,
           sMul[0], sMul[1], sMul[2], sRedStep == 0xFF ? -1 : sRedStep);
}
