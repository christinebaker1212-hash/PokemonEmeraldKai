// Moemon Kai: UNOVA's TOWN MAP picture (tools/unova/gen_townmap.py picture(), in unova/black_gfx.pak): Black's own
// town map on region_map.c's UNOVA page, drawn in full color over the map's background by the renderer's picture hook
// (gba_easy_draw.c gMpBgPictureLine), following that BG's own scroll and zoom. Its palette (gU3dTownMapPal) is the
// BG's: a white and a black the fades change, so the picture fades with them, and a signature that says the picture's
// palette is still the one loaded (another screen's palette turns the picture off). UNOVA's own, never SINNOH's.
// Also FLY's towns: Black's own arrival flags (its town map's field 15), set on entering the town's zone as Black's
// field sets them (U3D_TownMapArrive), and the town open to FLY once its flag is (U3D_TownMapFlyOpen).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "palette.h"
#include "constants/region_map_sections.h"
#include "constants/heal_locations.h"

bool8 U3D_Gfx_TownMap(const u16 **pixels, u16 *w, u16 *h);   // (u3d_gfx.c)
bool8 U3D_Vm_Flag(u16 id);
void U3D_Vm_SetFlag(u16 id);
bool8 U3D_IsUnovaGroup(u8 group);
extern void (*gMpBgPictureLine)(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy);
extern int gMpBgPictureBg;

#define SIG_A 0x1235
#define SIG_B 0x0422
const u16 gU3dTownMapPal[48] = {0x7FFF, 0x0000, SIG_A, SIG_B};

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

// The picture on BG BG (SHOW FALSE: none), its texture (OFFX, OFFY) at the screen's top-left in the full view
void U3D_TownMapShowOn(bool8 show, int bg, s32 offX, s32 offY)
{
    if (!show || !U3D_Gfx_TownMap(&sPixels, &sW, &sH))
    {
        if (gMpBgPictureLine == Line)
            gMpBgPictureLine = NULL;
        sPixels = NULL;
        return;
    }
    sOffX = offX;
    sOffY = offY;
    gMpBgPictureBg = bg;
    gMpBgPictureLine = Line;
}

#define QZMAP_UNOVA 8
#define MAP_HEIGHT 15
#define MAP_WIDTH 28
struct QzMapPlace { mapsec_u16_t mapsec; u8 map, x, y, width, height; };
struct QzFlyDest { mapsec_u16_t mapsec; u8 map; u8 heal; u16 flag; };
#define sUnovaMapLayout sUnusedLayout
#define sUnovaMapPlaces sUnusedPlaces
#define sUnovaFlyDests sUnusedFlyDests
#include "../data/region_map/unova_townmap.h"

// Black's arrival flag of the town whose zone the player entered; and the towns the story has passed by the UNOVA
// badges held (NUVEMA, ACCUMULA, then each badge's city: STRIATON 0x9B2 ... OPELUCID 0x9B9), for games played before
// the arrival flags were kept
void U3D_TownMapArrive(u16 zone)
{
    u8 MP_Region_BadgeCount(u8 region);
    u32 i, badges = MP_Region_BadgeCount(4);   // (REGION_UNOVA)
    for (i = 0x9B0; i <= 0x9B1 + badges && i <= 0x9B9; i++)
        if (badges && !U3D_Vm_Flag(i))
            U3D_Vm_SetFlag(i);
    for (i = 0; i < ARRAY_COUNT(sUnovaArrive); i++)
        if (sUnovaArrive[i][0] == zone && !U3D_Vm_Flag(sUnovaArrive[i][1]))
            U3D_Vm_SetFlag(sUnovaArrive[i][1]);
}

// FLY can go to a town: its arrival flag set, and the flag it waits for (LIBERTY GARDEN's) too
bool8 U3D_TownMapFlyOpen(u16 flag, u32 index)
{
    return U3D_Vm_Flag(flag) && (index >= ARRAY_COUNT(sUnovaFlyLocks) || sUnovaFlyLocks[index] == 0xFFFF
                                 || U3D_Vm_Flag(sUnovaFlyLocks[index]));
}
