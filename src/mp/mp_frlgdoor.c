// Moemon Kai: KANTO's doors animate as FireRed's (pokefirered src/field_door.c): KANTO's tilesets are FireRed's, its
// door metatiles FireRed's numbers, its door pictures FireRed's (src/data/frlg_doors.h from
// tools/quetzal_port/gen_frlg_doors.py). FireRed's doors are one metatile (1x1) or one wide and two tall (1x2), their
// frames 4 tiles a metatile, unlike Emerald's (field_door.c, which hands KANTO's maps over here).
#include "global.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "task.h"
#include "constants/songs.h"
#include "mp_region.h"
#include "mp.h"

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

// a KANTO map (its tilesets FireRed's)
bool8 MP_FrlgDoor_Here(void)
{
    return MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup) == REGION_KANTO;
}

static const struct FrlgDoorGraphics *Find(u32 x, u32 y)
{
    const struct FrlgDoorGraphics *gfx;
    u16 id = MapGridGetMetatileIdAt(x, y);
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
