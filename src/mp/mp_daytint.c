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
#include "main.h"
#include "overworld.h"
#include "rtc.h"
#include "sprite.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"

const char *Updater_ExeFile(const char *name);

static u8 sTable[48][3];
static bool8 sLoaded, sTried;
static u16 sMul[3];   // this frame's share of each channel, 256 = all

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

static u16 DayTintPixel(u16 c, int x, int y, bool8 sprite)
{
    u32 r = c & 0x1F, g = (c >> 5) & 0x1F, b = (c >> 10) & 0x1F;
    r = r * sMul[0] >> 8;
    g = g * sMul[1] >> 8;
    b = b * sMul[2] >> 8;
    return (c & 0x8000) | (r > 31 ? 31 : r) | ((g > 31 ? 31 : g) << 5) | ((b > 31 ? 31 : b) << 10);
}

static bool8 Wanted(void)
{
    if (gMain.callback2 != CB2_Overworld || gMain.inBattle)
        return FALSE;
    return IsMapTypeOutdoors(gMapHeader.mapType);
}

// sdl2.c VDraw, after the battle weather's (which holds the hook in battle): the tint for the next frame
void MP_DayTint_Update(void)
{
    u32 half, next, ch, mins, i;
    if (!sTried)
        Load();
    if (!sLoaded || !Wanted())
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
        sMul[ch] = (sTable[half][ch] * (30 - mins % 30) + sTable[next][ch] * (mins % 30)) * 2 / 30;
    memset(gMpSceneObjPal, 0, sizeof(gMpSceneObjPal));
    for (i = 0; i < MAX_SPRITES; i++)
        if (gSprites[i].inUse && !gSprites[i].invisible && gSprites[i].oam.priority >= 1)
            gMpSceneObjPal[gSprites[i].oam.paletteNum & 63] = 1;
    // SINNOH's 3D world is lit by the area lights itself (s3d.c): there only its sprites take the tint (the user,
    // 2026-11-02: the light should tint the sprites as well)
    gMpSceneLayers = S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) ? 0 : 0xE;
    gMpScenePixel = DayTintPixel;
}

// Tests (mp daytint): the tint now
void MP_DayTint_Debug(void)
{
    if (!sTried)
        Load();
    printf("[test] daytint: %s, on %d, mul %u %u %u\n", sLoaded ? "loaded" : "none", gMpScenePixel == DayTintPixel,
           sMul[0], sMul[1], sMul[2]);
}
