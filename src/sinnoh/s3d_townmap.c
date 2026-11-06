// Moemon Kai: SINNOH's TOWN MAP picture (tools/sinnoh/gen_townmap.py picture(), in sinnoh/platinum_gfx.pak): Platinum's
// own town map art on region_map.c's SINNOH pages, drawn in full color over the map's background by the renderer's
// picture hook (gba_easy_draw.c gMpBgPictureLine), following that BG's own scroll and zoom. Its palette
// (gS3dTownMapPal) is the BG's: a white and a black the fades change, so the picture fades with them, and a signature
// that says the picture's palette is still the one loaded (another screen's palette turns the picture off).
// Also the FLY spots' flags, set on arriving (Sinnoh_TownMapArrive).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "palette.h"
#include "constants/region_map_sections.h"
#include "constants/heal_locations.h"

bool8 S3D_Gfx_TownMap(u8 page, const u16 **pixels, u16 *w, u16 *h);   // (s3d_gfx.c)
const u8 *S3D_Gfx_TownBlock(u32 n);
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

static bool8 SpotUnlocked(u8 spot);
static u16 sWork[480 * 320];   // the page with its FLY spots' blocks

// The page with Platinum's FLY spot blocks on it (gen_townmap.py blocks): gray while the town is unvisited, blue
// (towns) or red (cities) once its FLY spot is open; VICTORY ROAD's only once the LEAGUE's is open
static const u16 *WithBlocks(u8 page, const u16 *pixels, u16 w, u16 h)
{
    const u8 *b;
    u32 n;
    if (w * h > ARRAY_COUNT(sWork))
        return pixels;
    memcpy(sWork, pixels, w * h * 2);
    for (n = 0; (b = S3D_Gfx_TownBlock(n)) != NULL; n++)
    {
        u32 x0 = b[4] | b[5] << 8, y0 = b[6] | b[7] << 8, bw = b[8] | b[9] << 8, bh = b[10] | b[11] << 8, x, y;
        bool8 open = SpotUnlocked(b[1]);
        const u8 *px = b + 12 + (open ? bw * bh * 2 : 0);
        if (b[0] != page || (b[2] && !open) || b[1] == 0xFF)
            continue;
        for (y = 0; y < bh; y++)
            for (x = 0; x < bw; x++, px += 2)
            {
                u16 c = px[0] | px[1] << 8;
                if (c && x0 + x < w && y0 + y < h)
                    sWork[(y0 + y) * w + x0 + x] = c & 0x7FFF;
            }
    }
    return sWork;
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
    sPixels = WithBlocks(page, sPixels, sW, sH);
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

// A SINNOH town visited (its FLY spot's flag: Platinum's first-arrival flags, for the TV: s3d_tv.c)
bool8 Sinnoh_TownVisited(u16 mapsec)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohFlyDests); i++)
        if (sSinnohFlyDests[i].mapsec == mapsec)
            return FlagGet(sSinnohFlyDests[i].flag);
    return FALSE;
}

static bool8 SpotUnlocked(u8 spot)
{
    return spot < ARRAY_COUNT(sSinnohFlyDests) && FlagGet(sSinnohFlyDests[spot].flag);
}

void Sinnoh_TownMapArrive(void)
{
    void Sinnoh_JournalArrived(u16 mapsec);
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohFlyDests); i++)
        if (sSinnohFlyDests[i].mapsec == gMapHeader.regionMapSectionId && !FlagGet(sSinnohFlyDests[i].flag))
        {
            FlagSet(sSinnohFlyDests[i].flag);
            Sinnoh_JournalArrived(sSinnohFlyDests[i].mapsec);   // (the JOURNAL: "Arrived in ...": s3d_journal.c)
        }
}
