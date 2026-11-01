// Moemon Kai: SINNOH's TOWN MAP picture (tools/sinnoh/gen_townmap.py picture(), in sinnoh/platinum_gfx.pak): Platinum's
// own town map art on region_map.c's SINNOH pages, drawn in full color over the map's background by the renderer's
// picture hook (gba_easy_draw.c gMpBgPictureLine), following that BG's own scroll and zoom. Its palette
// (gS3dTownMapPal) is the BG's: a white and a black the fades change, so the picture fades with them, and a signature
// that says the picture's palette is still the one loaded (another screen's palette turns the picture off).
// Also the FLY spots' flags, set on arriving (Sinnoh_TownMapArrive).
#include <stdio.h>
#include "global.h"
#include "event_data.h"
#include "palette.h"
#include "constants/region_map_sections.h"
#include "constants/heal_locations.h"

bool8 S3D_Gfx_TownMap(u8 page, const u16 **pixels, u16 *w, u16 *h);   // (s3d_gfx.c)
extern void (*gMpBgPictureLine)(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy);
extern int gMpBgPictureBg;

#define SIG_A 0x1234
#define SIG_B 0x0421
const u16 gS3dTownMapPal[48] = {0x7FFF, 0x0000, SIG_A, SIG_B};

// The picture's point for the BG's texture point (0, 0), in texels: the full view shows texture (OFF_X, OFF_Y) at the
// top-left of the screen (region_map.c's BG2 in its full view; the POKéDEX's AREA page shows its map unscrolled)
static s32 sOffX, sOffY;
static const u16 *sPixels;
static u16 sW, sH;

static void Line(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy)
{
    const u16 *pal = (const u16 *)PLTT + BG_PLTT_ID(7);
    u16 white, black;
    s32 x, wr, wg, wb, br, bg, bb;
    if (gPlttBufferUnfaded[BG_PLTT_ID(7) + 2] != SIG_A || gPlttBufferUnfaded[BG_PLTT_ID(7) + 3] != SIG_B || sPixels == NULL)
        return;
    white = pal[0], black = pal[1];
    br = black & 31, bg = black >> 5 & 31, bb = black >> 10 & 31;
    wr = (white & 31) - br, wg = (white >> 5 & 31) - bg, wb = (white >> 10 & 31) - bb;
    x0 -= sOffX << 8;
    y0 -= sOffY << 8;
    for (x = 0; x < DISPLAY_WIDTH; x++, x0 += dx, y0 += dy)
    {
        s32 px = x0 >> 7, py = y0 >> 7;   // (the picture is at twice the full view's resolution)
        u16 c;
        if (px < 0) px = 0;
        if (py < 0) py = 0;
        if (px >= sW) px = sW - 1;
        if (py >= sH) py = sH - 1;
        c = sPixels[py * sW + px];
        layer[x] = 0x8000 | ((c & 31) * wr / 31 + br) | ((c >> 5 & 31) * wg / 31 + bg) << 5 | ((c >> 10 & 31) * wb / 31 + bb) << 10;
    }
}

// Page PAGE (0 NORTH, 1 SOUTH; 0xFF: none) on BG BG, its texture (OFFX, OFFY) at the screen's top-left in the full view
void S3D_TownMapShowOn(u8 page, int bg, s32 offX, s32 offY)
{
    if (page == 0xFF || !S3D_Gfx_TownMap(page, &sPixels, &sW, &sH))
    {
        sPixels = NULL;
        gMpBgPictureLine = NULL;
        return;
    }
    sOffX = offX;
    sOffY = offY;
    gMpBgPictureBg = bg;
    gMpBgPictureLine = Line;
}

// FLY: the SINNOH towns visited (each FLY spot's flag, region_map.c sSinnohFlyDests), on entering one of their maps
#define QZMAP_SINNOH_NORTH 6
struct QzMapPlace { mapsec_u16_t mapsec; u8 map, x, y, width, height; };
struct QzFlyDest { mapsec_u16_t mapsec; u8 map; u8 heal; u16 flag; };
#define MAP_HEIGHT 15
#define MAP_WIDTH 28
#define gSinnohMapSectionNames sUnusedNames
#define sSinnohMapLayouts sUnusedLayouts
#define sSinnohMapPlaces sUnusedPlaces
#include "../data/region_map/sinnoh_region_map.h"

void Sinnoh_TownMapArrive(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohFlyDests); i++)
        if (sSinnohFlyDests[i].mapsec == gMapHeader.regionMapSectionId && !FlagGet(sSinnohFlyDests[i].flag))
            FlagSet(sSinnohFlyDests[i].flag);
}
