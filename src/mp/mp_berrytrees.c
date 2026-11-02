// Moemon Kai: BERRY trees past the save's own 128 (its berryTrees[] is full: HOENN's 1-89, JOHTO's and the GARDEN
// farm's above): numbers 128-255, kept in "<save>.region" (chunk BTRE, mp_region.c). SINNOH's are Platinum's 118
// soil patches (tools/sinnoh/gen_maps.py: tree 128 + Platinum's patch number), each the game's own BERRY tree
// (berry.c GetBerryTreeInfo, BerryTreeScript), starting with the BERRY Platinum's new game puts there, ripe
// (Platinum's include/data/berry_init.h: gSinnohBerryInit, tools/sinnoh/contest_data.py berry_lines).
#include <string.h>
#include "global.h"
#include "berry.h"
#include "constants/berry.h"
#include "constants/items.h"

#define FIRST_TREE 128
#define TREES (256 - FIRST_TREE)
#define MAGIC 0x45525442   // "BTRE"

static struct
{
    u32 magic;
    struct BerryTree trees[TREES];
} sB;

extern const u16 gSinnohBerryInit[];   // (count, then item and yield per patch)

struct BerryTree *MP_RegionBerryTree(u8 id)
{
    if (id < FIRST_TREE)
        return NULL;
    return &sB.trees[id - FIRST_TREE];
}

struct BerryTree *MP_RegionBerryTrees(u32 *count)
{
    *count = TREES;
    return sB.trees;
}

// a new game: SINNOH's patches with Platinum's first BERRIES, ripe (BerryPatches_Init)
void MP_RegionBerryTrees_Reset(void)
{
    u32 i, n = gSinnohBerryInit[0];
    memset(&sB, 0, sizeof(sB));
    sB.magic = MAGIC;
    for (i = 0; i < n && i < TREES; i++)
    {
        u16 item = gSinnohBerryInit[1 + i * 2];
        if (item < FIRST_BERRY_INDEX)
            continue;
        PlantBerryTree(FIRST_TREE + i, ITEM_TO_BERRY(item), BERRY_STAGE_BERRIES, TRUE);
        sB.trees[i].berryYield = gSinnohBerryInit[2 + i * 2];
    }
}

const void *MP_RegionBerryTrees_Data(u32 *size)
{
    sB.magic = MAGIC;
    *size = sizeof(sB);
    return &sB;
}

void MP_RegionBerryTrees_Load(const void *data, u32 size)
{
    if (size == sizeof(sB) && ((const u32 *)data)[0] == MAGIC)
        memcpy(&sB, data, size);
}
