// Moemon Co-op: full-color pictures pinned to maps, past the GBA's tiles and palettes (the GAME CENTER: its tower
// in RUSTBORO CITY and its four floors). The renderer (gba_easy_draw.c) hands each line of the field's BG layers to
// MapPicLine, which puts the picture's pixels in the bottom layer (BG3) and clears the two above it where the
// picture is drawn, so sprites go on top as on any map. A picture's "over" part (a counter's back edge, in the row
// of whoever stands behind it) goes in the top layer (BG1), over the sprites, as the GBA's counters do with their
// top layer. The map's own tiles only carry collision, warps and
// behaviors (tools/moemon_gamecenter/gen_gamecenter.py). A room's picture also fills the rest of the screen with
// its color (black around a floor, the sky around the rooftop).
//
// Fades follow the BG palettes: each frame the shown palette (PLTT) against the unfaded one gives, per color
// channel, the line c -> a + b * c that every fade (to black, to white, a weather tint) is.
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "field_camera.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "constants/maps.h"
#include "constants/rgb.h"

extern void (*gMpMapPicLine)(int line, u16 layers[4][DISPLAY_WIDTH]);

struct MapPic
{
    u16 map;           // MAP_* (group << 8 | number)
    s16 x, y;          // its top-left, in pixels from the map's (0, 0)
    u16 w, h;
    const u16 *pixels; // RGB555, bit 15 = drawn
    u16 fill;          // the rest of the screen (bit 15 set), or 0 for the map's own tiles
    const u16 *over;   // the same size: what is drawn over the sprites (a counter's back edge), or NULL
};

static const u16 sGameCenterBuilding[] = INCBIN_U16("graphics/moemon/gamecenter/building.bin");
static const u16 sGameCenter1F[] = INCBIN_U16("graphics/moemon/gamecenter/1f.bin");
static const u16 sGameCenter2F[] = INCBIN_U16("graphics/moemon/gamecenter/2f.bin");
static const u16 sGameCenter3F[] = INCBIN_U16("graphics/moemon/gamecenter/3f.bin");
static const u16 sGameCenterRooftop[] = INCBIN_U16("graphics/moemon/gamecenter/rooftop.bin");
static const u16 sGameCenter1FOver[] = INCBIN_U16("graphics/moemon/gamecenter/1f_over.bin");
static const u16 sGameCenter2FOver[] = INCBIN_U16("graphics/moemon/gamecenter/2f_over.bin");
static const u16 sGameCenter3FOver[] = INCBIN_U16("graphics/moemon/gamecenter/3f_over.bin");

#define BLACK 0x8000
#define SKY (0x8000 | RGB(19, 26, 31))

// The tower stands in the trees north of RUSTBORO's road east, its two doors on cells (28, 7) and (32, 7) with the
// tunnel sign between them; its top rises past the map's edge (seen from the road north, ROUTE 115)
static const struct MapPic sPics[] = {
    {MAP_RUSTBORO_CITY, 407, -47, 155, 179, sGameCenterBuilding, 0, NULL},
    {MAP_RUSTBORO_CITY_GAME_CENTER_1F, 0, 0, 208, 272, sGameCenter1F, BLACK, sGameCenter1FOver},
    {MAP_RUSTBORO_CITY_GAME_CENTER_2F, 0, 0, 208, 272, sGameCenter2F, BLACK, sGameCenter2FOver},
    {MAP_RUSTBORO_CITY_GAME_CENTER_3F, 0, 0, 208, 272, sGameCenter3F, BLACK, sGameCenter3FOver},
    {MAP_RUSTBORO_CITY_GAME_CENTER_ROOFTOP, 0, 0, 288, 256, sGameCenterRooftop, SKY, NULL},
};

#define MAX_SHOWN 4
static struct
{
    const struct MapPic *pic;
    s32 x, y;          // its top-left from the current map's (0, 0)
} sShown[MAX_SHOWN];
static u8 sShownCount;
static s32 sFadeA[3], sFadeB[3];   // c -> (a + b * c) / 256, per channel

// The pictures of this map and the maps around it
static void FindShown(void)
{
    const struct MapConnections *conns = gMapHeader.connections;
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i, j;
    sShownCount = 0;
    for (i = 0; i < ARRAY_COUNT(sPics) && sShownCount < MAX_SHOWN; i++)
    {
        if (sPics[i].map == here)
        {
            sShown[sShownCount].pic = &sPics[i];
            sShown[sShownCount].x = sPics[i].x;
            sShown[sShownCount].y = sPics[i].y;
            sShownCount++;
            continue;
        }
        if (!conns || sPics[i].fill)
            continue;
        for (j = 0; j < (u32)conns->count && sShownCount < MAX_SHOWN; j++)
        {
            const struct MapConnection *c = &conns->connections[j];
            const struct MapHeader *other;
            s32 ox = 0, oy = 0, off = (s32)(intptr_t)c->offset;
            if (((c->mapGroup << 8) | c->mapNum) != sPics[i].map)
                continue;
            other = Overworld_GetMapHeaderByGroupAndId(c->mapGroup, c->mapNum);
            switch (c->direction)
            {
            case CONNECTION_SOUTH: ox = off, oy = gMapHeader.mapLayout->height; break;
            case CONNECTION_NORTH: ox = off, oy = -other->mapLayout->height; break;
            case CONNECTION_EAST: ox = gMapHeader.mapLayout->width, oy = off; break;
            case CONNECTION_WEST: ox = -other->mapLayout->width, oy = off; break;
            default: continue;
            }
            sShown[sShownCount].pic = &sPics[i];
            sShown[sShownCount].x = ox * 16 + sPics[i].x;
            sShown[sShownCount].y = oy * 16 + sPics[i].y;
            sShownCount++;
        }
    }
}

// This frame's fade, from the BG palettes as shown against as they are
static void FindFade(void)
{
    const u16 *shown = (const u16 *)PLTT;
    s32 ch, i;
    for (ch = 0; ch < 3; ch++)
    {
        s32 lo = 32, hi = -1, loShown = 0, hiShown = 0;
        for (i = 0; i < 13 * 16; i++)
        {
            s32 u = (gPlttBufferUnfaded[i] >> (5 * ch)) & 31, s = (shown[i] >> (5 * ch)) & 31;
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
            sFadeB[ch] = hi >= 0 && hi ? hiShown * 256 / hi : 256;
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

static void MapPicLine(int line, u16 layers[4][DISPLAY_WIDTH])
{
    u8 tileX, tileY;
    s32 hoffs, voffs, camX, camY, mapY, x, i;
    if (!sShownCount || gMain.callback2 != CB2_Overworld)
        return;
    if (line == 0)
        FindFade();
    GetFieldCameraTileOffset(&tileX, &tileY);
    hoffs = REG_BG3HOFS;
    voffs = REG_BG3VOFS;
    // A screen pixel's map pixel: the BG tilemap (256 pixels, wrapping) holds the map from the camera's tile
    // (pos - 7) on at its tile offset
    camX = (gSaveBlock1Ptr->pos.x - 7) * 16;
    camY = (gSaveBlock1Ptr->pos.y - 7) * 16;
    mapY = camY + ((line + voffs - tileY * 8) & 255);
    for (i = 0; i < sShownCount; i++)
    {
        const struct MapPic *p = sShown[i].pic;
        s32 py = mapY - sShown[i].y;
        bool8 rowIn = py >= 0 && py < p->h;
        u16 fill = p->fill ? Faded(p->fill) : 0;
        if (!rowIn && !fill)
            continue;
        for (x = 0; x < DISPLAY_WIDTH; x++)
        {
            s32 px = camX + ((x + hoffs - tileX * 8) & 255) - sShown[i].x;
            bool8 in = rowIn && px >= 0 && px < p->w;
            u16 c = in ? p->pixels[py * p->w + px] : 0, o = in && p->over ? p->over[py * p->w + px] : 0;
            if (c & 0x8000)
                c = Faded(c);
            else if (fill)
                c = fill;
            else
                continue;
            layers[3][x] = c;
            layers[2][x] = 0;
            // (BG1, the field's top layer, is drawn over the sprites)
            layers[1][x] = o & 0x8000 ? Faded(o) : 0;
        }
    }
}

// Every overworld frame: which pictures are in view (none: the renderer skips the hook)
void MP_MapPic_Update(void)
{
    FindShown();
    gMpMapPicLine = sShownCount ? MapPicLine : NULL;
}

// A map with a picture of its own (the GAME CENTER's floors: their tiles never show)
bool8 MP_MapPic_IsPictureMap(u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPics); i++)
        if (sPics[i].map == ((group << 8) | num) && sPics[i].fill)
            return TRUE;
    return FALSE;
}
