// Moemon Kai: the boat to ORRE (docs/ORRE_DECISIONS.md "A post-game place", the user 2026-11-07: "Orre will not be
// selectable as a region to start in, rather, it is always a post-game region, and a special one at that - it goes on
// the BATTLE FRONTIER, BATTLE ISLAND, FARADAY ISLAND, etc menu"). The ORRE TICKET comes after any region's ELITE FOUR
// (src/mp/mp_battleisland.c's courier, old saves too); HOENN's ferries list ORRE, every other port offers it
// (data/scripts/orre.inc). The boat lands at the OUTSKIRT STAND, Colosseum's own start, in front of its sailor (Kai's
// one person in ORRE: the user's choice), who sails back to the port the player came from (ORRE's maps are shared,
// mp_region.c MP_Region_IsSharedMap: the player stays in their region). A Shadow Pokémon can't leave ORRE
// (docs/SHADOW_POKEMON.md decision 2).
#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "mp.h"
#include "mp_region.h"
#include "orre.h"
#include "orre_shadow.h"
#include "o3d.h"
#include "constants/maps.h"

void MP_Voyage_Begin(bool8 eastbound, bool8 faded);   // mp_voyage.c
bool8 MP_Region_PortOf(u8 region, u16 *map, s16 *x, s16 *y);   // qz_specials.c

void Orre_Sail(void)
{
    s16 x = 33, y = 40;
    Orre_MapStart(MAP_ORRE_OUTSKIRT_STAND, &x, &y);
    SetWarpDestination(MAP_GROUP(MAP_ORRE_OUTSKIRT_STAND), MAP_NUM(MAP_ORRE_OUTSKIRT_STAND), WARP_ID_NONE, x, y);
    MPNet_Log("orre: sailing from map %d.%d", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    MP_Voyage_Begin(FALSE, FALSE);   // (westward)
}

void Orre_SailBack(void)
{
    if (gRegionState.originValid && !O3D_IsOrreGroup(gRegionState.originGroup)
     && gRegionState.originGroup != MAP_GROUP(MAP_BATTLE_ISLAND))
        SetWarpDestination(gRegionState.originGroup, gRegionState.originNum, WARP_ID_NONE, gRegionState.originX,
                           gRegionState.originY);
    else
    {
        u16 map;
        s16 x, y;
        if (!MP_Region_PortOf(MP_Region_Current(), &map, &x, &y))
            map = MAP_LILYCOVE_CITY_HARBOR, x = 8, y = 11;
        SetWarpDestination(map >> 8, map & 0xFF, WARP_ID_NONE, x, y);
    }
    MPNet_Log("orre: sailing back");
    MP_Voyage_Begin(TRUE, FALSE);
}

void Orre_ShadowAboard(void)
{
    gSpecialVar_Result = Orre_Shadow_PartyHasShadow();
}
