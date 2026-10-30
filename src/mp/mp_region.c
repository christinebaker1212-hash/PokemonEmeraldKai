// Moemon Kai: HOENN, JOHTO and KANTO (docs/REGIONS_PLAN.md).
// - Which region a map is in (by its map group: Kai's groups are Hoenn, Johto 34-35, Kanto 36-39).
// - The flags and vars past the save block's: Johto / Kanto content uses flags 0x1000-0x3FFF and vars 0x5000-0x52FF
//   (tools/quetzal_port/import.py). They are kept in "<save>.region", written with the save and only read back
//   when it was written with this very save (like the bag's extra slots, mp_bag.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "mp.h"
#include "mp_region.h"
#include "save.h"
#include "constants/map_groups.h"
#include "constants/flags.h"
#include "constants/qz_port.h"

extern char gPlatformSavePath[512];

#define REGION_MAGIC 0x4E474552   // "REGN"
#define REGION_FORMAT 2   // 2: + struct RegionState

static u8 sFlags[REGION_FLAG_BYTES];
static u16 sVars[REGION_VARS_COUNT];
struct RegionState gRegionState;   // per-region progress (mp_region.h)

u8 MP_Region_OfGroup(u8 group)
{
    if (group == MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) || group == MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 1)
        return REGION_JOHTO;
    if (group >= MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 2 && group <= MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN) + 5)
        return REGION_KANTO;
    return REGION_HOENN;
}

u8 MP_Region_Current(void)
{
    return MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup);
}

u8 *MP_Region_FlagPointer(u16 id)
{
    if (id < REGION_FLAGS_START || id >= REGION_FLAGS_END)
        return NULL;
    return &sFlags[(id - REGION_FLAGS_START) / 8];
}

u16 *MP_Region_VarPointer(u16 id)
{
    if (id < REGION_VARS_START || id >= REGION_VARS_START + REGION_VARS_COUNT)
        return NULL;
    return &sVars[id - REGION_VARS_START];
}

void MP_Region_NewGame(void)
{
    memset(sFlags, 0, sizeof(sFlags));
    memset(sVars, 0, sizeof(sVars));
    memset(&gRegionState, 0, sizeof(gRegionState));
}

// What ties the side file to the save it was written with
static u32 Fingerprint(void)
{
    u32 sum = gSaveCounter * 2654435761u;
    sum = sum * 31 + (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8
                      | gSaveBlock2Ptr->playerTrainerId[2] << 16 | (u32)gSaveBlock2Ptr->playerTrainerId[3] << 24);
    return sum;
}

void MP_Region_WriteSidecar(void)
{
    char path[600], tmp[610];
    FILE *f;
    u32 header[4];
    if (gSaveBlock2Ptr == NULL)
        return;
    snprintf(path, sizeof(path), "%s.region", gPlatformSavePath);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return;
    header[0] = REGION_MAGIC;
    header[1] = REGION_FORMAT;
    header[2] = Fingerprint();
    header[3] = sizeof(sFlags) + sizeof(sVars) + sizeof(gRegionState);
    fwrite(header, sizeof(header), 1, f);
    fwrite(sFlags, sizeof(sFlags), 1, f);
    fwrite(sVars, sizeof(sVars), 1, f);
    fwrite(&gRegionState, sizeof(gRegionState), 1, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

static bool8 LoadFrom(const char *path)
{
    FILE *f = fopen(path, "rb");
    u32 header[4];
    bool8 ok = FALSE;
    if (!f)
        return FALSE;
    memset(&gRegionState, 0, sizeof(gRegionState));
    if (fread(header, sizeof(header), 1, f) == 1 && header[0] == REGION_MAGIC && header[2] == Fingerprint()
     && header[3] >= sizeof(sFlags) + sizeof(sVars))
    {
        ok = fread(sFlags, sizeof(sFlags), 1, f) == 1 && fread(sVars, sizeof(sVars), 1, f) == 1;
        if (ok && header[3] > sizeof(sFlags) + sizeof(sVars))   // format 2+: the region state (a later build's longer one: what fits)
        {
            u32 n = header[3] - sizeof(sFlags) - sizeof(sVars);
            fread(&gRegionState, n < sizeof(gRegionState) ? n : sizeof(gRegionState), 1, f);
        }
    }
    fclose(f);
    return ok;
}

void MP_Region_LoadFromSave(void)
{
    char path[600];
    int i;
    for (i = 0; i <= 3; i++)
    {
        if (i == 0)
            snprintf(path, sizeof(path), "%s.region", gPlatformSavePath);
        else
            snprintf(path, sizeof(path), "%s.bak%d.region", gPlatformSavePath, i);
        if (LoadFrom(path))
        {
            MPNet_Log("region: loaded%s", i ? " (from a backup)" : "");
            return;
        }
    }
    MP_Region_NewGame();
    MPNet_Log("region: no side file for this save (Johto / Kanto progress starts empty)");
}

// ---------------------------------------------------------------------------------------------------- progress
static const u16 sBadgeFlags[REGION_COUNT] = {FLAG_BADGE01_GET, FLAG_QZ_0x11B2, FLAG_QZ_0x2100};

u16 MP_Region_BadgeFlag(u8 region, u8 badge)
{
    if (region >= REGION_COUNT || badge >= 8)
        return 0;
    return sBadgeFlags[region] + badge;
}

u8 MP_Region_BadgeCount(u8 region)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        n += FlagGet(MP_Region_BadgeFlag(region, i)) != 0;
    return n;
}

bool8 MP_Region_Cleared(u8 region)
{
    if (region == REGION_HOENN && FlagGet(FLAG_SYS_GAME_CLEAR))
        return TRUE;
    return region < REGION_COUNT && (gRegionState.cleared & (1 << region));
}

void MP_Region_SetCleared(u8 region)
{
    if (region < REGION_COUNT)
        gRegionState.cleared |= 1 << region;
}
