// Moemon Maker: the names the editor shows and edits are saved by (docs/MOEMON_MAKER.md "Names, not numbers"): every
// map, each map's people / signs / triggers by name, and the named values (songs, looks, movement types, trainer types,
// items, species, flags) worked out by the compiler from the game's own constants. The tables are made when the game is
// built (tools/maker/gen_names.py), so they're always this build's.
#include <string.h>
#include "global.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/trainer_types.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/flags.h"
#include "constants/songs.h"
#include "constants/qz_port.h"
#include "constants/sinnoh_gfx.h"
#include "mp.h"
#include "maker.h"
#include "maker_api.h"
#include "constants/maker_names.h"

extern const struct MapHeader *const *const gMapGroups[];

int Maker_MapCount(void)
{
    return MAKER_MAP_COUNT;
}

const struct MakerMapName *Maker_Map(int index)
{
    return index >= 0 && index < MAKER_MAP_COUNT ? &sMakerMapNames[index] : NULL;
}

int Maker_MapIndex(u16 mapGroup, u16 mapNum)
{
    static s16 sIndex[64][256];
    static bool8 sBuilt;
    if (!sBuilt)
    {
        int i;
        memset(sIndex, 0xFF, sizeof(sIndex));
        for (i = 0; i < MAKER_MAP_COUNT; i++)
            if (sMakerMapNames[i].mapGroup < 64)
                sIndex[sMakerMapNames[i].mapGroup][sMakerMapNames[i].mapNum] = i;
        sBuilt = TRUE;
    }
    return mapGroup < 64 && mapNum < 256 ? sIndex[mapGroup][mapNum] : -1;
}

const struct MakerMapEventNames *Maker_MapEventNames(int index)
{
    int i;
    if (index < 0 || index >= MAKER_MAP_COUNT)
        return NULL;
    // (both tables in map.json order are sorted differently: find by name, the first time only)
    {
        static s16 sEvents[MAKER_MAP_COUNT];
        static bool8 sBuilt;
        if (!sBuilt)
        {
            int m;
            for (m = 0; m < MAKER_MAP_COUNT; m++)
            {
                sEvents[m] = -1;
                for (i = 0; i < MAKER_MAP_EVENTS_COUNT; i++)
                    if (strcmp(sMakerMapEventNames[i].map, sMakerMapNames[m].name) == 0)
                    {
                        sEvents[m] = i;
                        break;
                    }
            }
            sBuilt = TRUE;
        }
        return sEvents[index] >= 0 ? &sMakerMapEventNames[sEvents[index]] : NULL;
    }
}

const struct MakerObjName *Maker_ObjName(int i)
{
    return &sMakerObjNames[i];
}

const struct MakerBgName *Maker_BgName(int i)
{
    return &sMakerBgNames[i];
}

const struct MakerCoordName *Maker_CoordName(int i)
{
    return &sMakerCoordNames[i];
}

const struct MapHeader *Maker_BaseMapHeader(u16 mapGroup, u16 mapNum)
{
    return gMapGroups[mapGroup][mapNum];
}

static const struct MakerNamedValue *Table(int table, int *count)
{
    switch (table)
    {
    case MAKER_NAMES_SONG: *count = MAKER_SONG_COUNT; return sSongNames;
    case MAKER_NAMES_GFX: *count = MAKER_GFX_COUNT; return sGfxNames;
    case MAKER_NAMES_MOVEMENT: *count = MAKER_MOVEMENT_COUNT; return sMovementNames;
    case MAKER_NAMES_TRAINERTYPE: *count = MAKER_TRAINERTYPE_COUNT; return sTrainertypeNames;
    case MAKER_NAMES_ITEM: *count = MAKER_ITEM_COUNT; return sItemNames;
    case MAKER_NAMES_SPECIES: *count = MAKER_SPECIES_COUNT; return sSpeciesNames;
    case MAKER_NAMES_FLAG: *count = MAKER_FLAG_COUNT; return sFlagNames;
    }
    *count = 0;
    return NULL;
}

int MakerApi_NameCount(int table)
{
    int n;
    Table(table, &n);
    return n;
}

const char *MakerApi_NameAt(int table, int i, unsigned int *value)
{
    int n;
    const struct MakerNamedValue *t = Table(table, &n);
    if (i < 0 || i >= n)
        return NULL;
    if (value)
        *value = t[i].value;
    return t[i].name;
}

const char *MakerApi_NameOf(int table, unsigned int value)
{
    int n, i;
    const struct MakerNamedValue *t = Table(table, &n);
    for (i = 0; i < n; i++)
        if (t[i].value == value)
            return t[i].name;
    return "";
}

int MakerApi_ValueOf(int table, const char *name, unsigned int *value)
{
    int n, i;
    const struct MakerNamedValue *t = Table(table, &n);
    for (i = 0; i < n; i++)
        if (strcmp(t[i].name, name) == 0)
        {
            *value = t[i].value;
            return 1;
        }
    return 0;
}
