// Moemon Kai: KANTO's doors animate as FireRed's (pokefirered src/field_door.c): KANTO's tilesets are FireRed's, its
// door metatiles FireRed's numbers, its door pictures FireRed's (src/data/frlg_doors.h from
// tools/quetzal_port/gen_frlg_doors.py). FireRed's doors are one metatile (1x1) or one wide and two tall (1x2), their
// frames 4 tiles a metatile, unlike Emerald's (field_door.c, which hands KANTO's maps over here).
// JOHTO's doors too: Quetzal's door pictures are only in its ROM (its door table wasn't ripped), so a JOHTO door's
// frames are made from the closed door itself, as a GBA door opens: its panel rises in three steps behind the wall
// above and the doorway under it goes dark (its palette's darkest color); its frame (the two columns each side) stays.
#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "task.h"
#include "constants/songs.h"
#include "mp_region.h"
#include "mp.h"
#include "metatile_behavior.h"
#include "palette.h"

enum { FRLG_DOOR_SOUND_NORMAL, FRLG_DOOR_SOUND_SLIDING };
enum { FRLG_DOOR_SIZE_1x1, FRLG_DOOR_SIZE_1x2 };

struct FrlgDoorGraphics
{
    u16 metatileId;
    u8 sound;
    u8 size;
    const u8 *tiles;
    const u8 *paletteNums;
};

struct FrlgDoorFrame
{
    u8 duration;
    u16 tileOffset;
};

#include "../data/frlg_doors.h"

#define CLOSED 0xFFFF
static const struct FrlgDoorFrame sOpenSmall[] = {{4, CLOSED}, {4, 0 * TILE_SIZE_4BPP}, {4, 4 * TILE_SIZE_4BPP}, {4, 8 * TILE_SIZE_4BPP}, {0}};
static const struct FrlgDoorFrame sOpenLarge[] = {{4, CLOSED}, {4, 0 * TILE_SIZE_4BPP}, {4, 8 * TILE_SIZE_4BPP}, {4, 16 * TILE_SIZE_4BPP}, {0}};
static const struct FrlgDoorFrame sCloseSmall[] = {{4, 8 * TILE_SIZE_4BPP}, {4, 4 * TILE_SIZE_4BPP}, {4, 0 * TILE_SIZE_4BPP}, {4, CLOSED}, {0}};
static const struct FrlgDoorFrame sCloseLarge[] = {{4, 16 * TILE_SIZE_4BPP}, {4, 8 * TILE_SIZE_4BPP}, {4, 0 * TILE_SIZE_4BPP}, {4, CLOSED}, {0}};

#define DOOR_TILE_START (NUM_TILES_TOTAL - 8)

// a KANTO map (its tilesets FireRed's) or a JOHTO one (its doors made from the closed door)
bool8 MP_FrlgDoor_Here(void)
{
    u8 region = MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup);
    return region == REGION_KANTO || region == REGION_JOHTO;
}

// ---- JOHTO: the frames made from the closed door metatile
static u8 sJohtoTiles[4 * 4 * TILE_SIZE_4BPP];   // three frames, four tiles each (+ room for the 8-tile copy)
static u8 sJohtoPals[8];
static struct FrlgDoorGraphics sJohtoDoor;

// a field tile's pixel (its 4bpp data in the map's character block, flipped as the entry says)
static u8 TilePixel(u16 entry, u32 px, u32 py)
{
    const u8 *tile = (const u8 *)(VRAM + TILE_OFFSET_4BPP(entry & 0x3FF));
    u8 b;
    if (entry & (1 << 10))
        px = 7 - px;
    if (entry & (1 << 11))
        py = 7 - py;
    b = tile[py * 4 + px / 2];
    return px & 1 ? b >> 4 : b & 0xF;
}

static void PutPixel(u8 *tile, u32 px, u32 py, u8 c)
{
    u8 *b = &tile[py * 4 + px / 2];
    *b = px & 1 ? (*b & 0x0F) | (c << 4) : (*b & 0xF0) | c;
}

static u8 Darkest(u8 pal)
{
    u8 i, best = 1;
    u32 bestLum = 0xFFFF;
    for (i = 1; i < 16; i++)
    {
        u16 c = gPlttBufferUnfaded[BG_PLTT_ID(pal) + i];
        u32 lum = (c & 0x1F) * 3 + ((c >> 5) & 0x1F) * 6 + ((c >> 10) & 0x1F);
        if (lum < bestLum)
            bestLum = lum, best = i;
    }
    return best;
}

static const struct FrlgDoorGraphics *JohtoDoor(u32 x, u32 y)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    const u16 *entries;
    u16 id = MapGridGetMetatileIdAt(x, y);
    u8 img[16][16], pal[4], dark[4];
    u32 q, f, px, py;
    if (!MetatileBehavior_IsDoor(MapGridGetMetatileBehaviorAt(x, y)) || layout == NULL)
        return NULL;
    if (id < Fieldmap_PrimaryMetatiles(layout))
        entries = layout->primaryTileset->metatiles + id * NUM_TILES_PER_METATILE;
    else
        entries = layout->secondaryTileset->metatiles + (id - Fieldmap_PrimaryMetatiles(layout)) * NUM_TILES_PER_METATILE;
    for (q = 0; q < 4; q++)   // the closed door: the top layer over the bottom, a quadrant's palette the one drawing it
    {
        u16 bottom = entries[q], top = entries[4 + q];
        bool8 topDraws = FALSE;
        for (py = 0; py < 8; py++)
            for (px = 0; px < 8; px++)
            {
                u8 t = TilePixel(top, px, py), c = t ? t : TilePixel(bottom, px, py);
                topDraws |= t != 0;
                img[(q / 2) * 8 + py][(q % 2) * 8 + px] = c;
            }
        pal[q] = (topDraws ? top : bottom) >> 12;
        dark[q] = Darkest(pal[q]);
        sJohtoPals[q] = pal[q];
        sJohtoPals[4 + q] = 0;
    }
    for (f = 0; f < 3; f++)   // the panel risen 5, 10 and 15 pixels
    {
        u32 rise = (f + 1) * 5;
        u8 *frame = &sJohtoTiles[f * 4 * TILE_SIZE_4BPP];
        for (py = 0; py < 16; py++)
            for (px = 0; px < 16; px++)
            {
                u32 tq = (py / 8) * 2 + px / 8;
                u8 c;
                if (px < 2 || px > 13)
                    c = img[py][px];
                else if (py + rise < 16)
                    c = img[py + rise][px];
                else
                    c = dark[tq];
                PutPixel(&frame[tq * TILE_SIZE_4BPP], px % 8, py % 8, c);
            }
    }
    sJohtoDoor.metatileId = id;
    sJohtoDoor.sound = FRLG_DOOR_SOUND_NORMAL;
    sJohtoDoor.size = FRLG_DOOR_SIZE_1x1;
    sJohtoDoor.tiles = sJohtoTiles;
    sJohtoDoor.paletteNums = sJohtoPals;
    return &sJohtoDoor;
}

static const struct FrlgDoorGraphics *Find(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx;
    u16 id = MapGridGetMetatileIdAt(x, y);
    if (MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup) == REGION_JOHTO)
        return JohtoDoor(x, y);
    for (gfx = sFrlgDoors; gfx->tiles != NULL; gfx++)
        if (gfx->metatileId == id)
            return gfx;
    return NULL;
}

static void BuildDoorTiles(u16 *tiles, u16 tileNum, const u8 *paletteNums)
{
    int i;
    for (i = 0; i < 4; i++)
        tiles[i] = (*(paletteNums++) << 12) | (tileNum + i);
    for (; i < 8; i++)
        tiles[i] = *(paletteNums++) << 12;
}

static void DrawClosed(const struct FrlgDoorGraphics *gfx, u32 x, u32 y)
{
    CurrentMapDrawMetatileAt(x, y);
    if (gfx->size == FRLG_DOOR_SIZE_1x2)
        CurrentMapDrawMetatileAt(x, y - 1);
}

static void DrawFrame(const struct FrlgDoorGraphics *gfx, const struct FrlgDoorFrame *frame, u32 x, u32 y)
{
    u16 tiles[8];
    if (frame->tileOffset == CLOSED)
    {
        DrawClosed(gfx, x, y);
        return;
    }
    CpuFastCopy(&gfx->tiles[frame->tileOffset], (void *)(VRAM + TILE_OFFSET_4BPP(DOOR_TILE_START)), 8 * TILE_SIZE_4BPP);
    if (gfx->size == FRLG_DOOR_SIZE_1x2)
    {
        BuildDoorTiles(tiles, DOOR_TILE_START, gfx->paletteNums);
        DrawDoorMetatileAt(x, y - 1, tiles);
        BuildDoorTiles(tiles, DOOR_TILE_START + 4, &gfx->paletteNums[4]);
    }
    else
        BuildDoorTiles(tiles, DOOR_TILE_START, gfx->paletteNums);
    DrawDoorMetatileAt(x, y, tiles);
}

#define tFrameId data[4]
#define tCounter data[5]
#define tX data[6]
#define tY data[7]

static void Task_FrlgDoor(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    const struct FrlgDoorFrame *frames = gTasks[taskId].ptr.genericPtr[0];
    const struct FrlgDoorGraphics *gfx = gTasks[taskId].ptr.genericPtr[1];
    if (tCounter == 0)
        DrawFrame(gfx, &frames[tFrameId], tX, tY);
    if (tCounter == frames[tFrameId].duration)
    {
        tCounter = 0;
        tFrameId++;
        if (frames[tFrameId].duration == 0)
            DestroyTask(taskId);
        return;
    }
    tCounter++;
}

static s8 Start(const struct FrlgDoorFrame *frames, const struct FrlgDoorGraphics *gfx, u32 x, u32 y)
{
    u8 taskId;
    if (FuncIsActiveTask(Task_FrlgDoor))
        return -1;
    taskId = CreateTask(Task_FrlgDoor, 0x50);
    gTasks[taskId].data[6] = x;
    gTasks[taskId].data[7] = y;
    gTasks[taskId].ptr.genericPtr[0] = (void *)frames;
    gTasks[taskId].ptr.genericPtr[1] = (void *)gfx;
    return taskId;
}

s8 MP_FrlgDoor_Open(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx = Find(x, y);
    MPNet_Log("frlg door: metatile 0x%X %s", MapGridGetMetatileIdAt(x, y), gfx ? "opens" : "isn't FireRed's");
    if (gfx == NULL)
        return -1;
    return Start(gfx->size == FRLG_DOOR_SIZE_1x1 ? sOpenSmall : sOpenLarge, gfx, x, y);
}

s8 MP_FrlgDoor_Close(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx = Find(x, y);
    if (gfx == NULL)
        return -1;
    return Start(gfx->size == FRLG_DOOR_SIZE_1x1 ? sCloseSmall : sCloseLarge, gfx, x, y);
}

void MP_FrlgDoor_SetOpened(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx = Find(x, y);
    if (gfx != NULL)
        DrawFrame(gfx, gfx->size == FRLG_DOOR_SIZE_1x1 ? &sOpenSmall[3] : &sOpenLarge[3], x, y);
}

void MP_FrlgDoor_SetClosed(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx = Find(x, y);
    if (gfx != NULL)
        DrawClosed(gfx, x, y);
}

bool8 MP_FrlgDoor_Running(void)
{
    return FuncIsActiveTask(Task_FrlgDoor);
}

u32 MP_FrlgDoor_Sound(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx = Find(x, y);
    return gfx != NULL && gfx->sound == FRLG_DOOR_SOUND_SLIDING ? SE_SLIDING_DOOR : SE_DOOR;
}
