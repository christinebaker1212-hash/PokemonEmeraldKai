// Moemon Maker: a map's wild Pokémon for the editor (include/maker_api.h): its own tables as built (gWildMonHeaders),
// and the user's slot edits applied to whatever the game picks (src/maker/maker_overrides.c Maker_WildInfo: after the
// GENERATION's pools, mp_region.c MP_Region_WildInfo, so the user's slot always wins).
#include <string.h>
#include "global.h"
#include "wild_encounter.h"
#include "maker.h"
#include "maker_api.h"

static const struct WildPokemonHeader *HeaderOf(int map)
{
    const struct MakerMapName *m = Maker_Map(map);
    u32 i;
    if (m == NULL)
        return NULL;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].mapGroup == m->mapGroup && gWildMonHeaders[i].mapNum == m->mapNum)
            return &gWildMonHeaders[i];
    return NULL;
}

static const struct WildPokemonInfo *InfoOf(const struct WildPokemonHeader *h, int area)
{
    if (h == NULL)
        return NULL;
    switch (area)
    {
    case MAKER_WILD_LAND: return h->landMonsInfo;
    case MAKER_WILD_WATER: return h->waterMonsInfo;
    case MAKER_WILD_ROCK: return h->rockSmashMonsInfo;
    case MAKER_WILD_FISH: return h->fishingMonsInfo;
    }
    return NULL;
}

int MakerApi_WildSlotCount(int area)
{
    static const u8 sCounts[MAKER_WILD_AREAS] = {12, 5, 5, 10};
    return area >= 0 && area < MAKER_WILD_AREAS ? sCounts[area] : 0;
}

// A map's table for an area as built: its slots, and its encounter rate; 0 if the map has none there
int MakerApi_WildArea(int map, int area, struct MakerWildSlot *slots, int *rate)
{
    const struct WildPokemonInfo *info = InfoOf(HeaderOf(map), area);
    int i, n = MakerApi_WildSlotCount(area);
    if (info == NULL || info->wildPokemon == NULL)
        return 0;
    for (i = 0; i < n; i++)
    {
        slots[i].species = info->wildPokemon[i].species;
        slots[i].minLevel = info->wildPokemon[i].minLevel;
        slots[i].maxLevel = info->wildPokemon[i].maxLevel;
    }
    *rate = info->encounterRate;
    return n;
}

// (maker_overrides.c) which map and area a table as built belongs to; FALSE if none
bool8 Maker_WildInfoOwner(const struct WildPokemonInfo *info, int *map, int *area)
{
    u32 i;
    int a;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        for (a = 0; a < MAKER_WILD_AREAS; a++)
            if (InfoOf(&gWildMonHeaders[i], a) == info)
            {
                *map = Maker_MapIndex(gWildMonHeaders[i].mapGroup, gWildMonHeaders[i].mapNum);
                *area = a;
                return *map >= 0;
            }
    return FALSE;
}
