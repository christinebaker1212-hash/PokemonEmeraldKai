// Moemon Kai: SINNOH's legendaries standing on the map (tools/sinnoh/legends.py, src/data/sinnoh_legends.h): on entering
// their map (special Sinnoh_LegendsRefresh) one is shown unless it was caught or beaten, or its time hasn't come:
// UXIE, AZELF, MESPRIT and GIRATINA from the 7th BADGE (when Platinum's story gives them), the others once SINNOH's
// LEAGUE is beaten
#include "global.h"
#include "event_data.h"
#include "mp_region.h"
#include "constants/species.h"
#include "constants/map_groups.h"
#include "fieldmap.h"
#include "rtc.h"
#include "../data/sinnoh_legends.h"

u16 Sinnoh_LegendsRefresh(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSinnohLegends); i++)
    {
        bool8 shown;
        if (sSinnohLegends[i].group != gSaveBlock1Ptr->location.mapGroup || sSinnohLegends[i].num != gSaveBlock1Ptr->location.mapNum)
            continue;
        shown = !FlagGet(sSinnohLegends[i].doneFlag) && MP_Gen_Allowed(sSinnohLegends[i].species)
             && (sSinnohLegends[i].badges == 0xFF ? MP_Region_Cleared(REGION_SINNOH)
                                                    : MP_Region_BadgeCount(REGION_SINNOH) >= sSinnohLegends[i].badges);
        if (shown)
            FlagClear(sSinnohLegends[i].objFlag);
        else
            FlagSet(sSinnohLegends[i].objFlag);
    }
    return 0;
}

// The GREAT MARSH (tools/sinnoh/gen_maps.py marsh_merge): the player stays on the 6th area's map for the whole marsh,
// so the wild Pokemon are the area's under the player's feet (Platinum's blocks: areas 1-2 / 3-4 / 5-6 from the
// top, west then east; the border's blocks count as the nearest area)
static bool8 InGreatMarsh(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return map >= MAP_SINNOH_GREAT_MARSH1 && map <= MAP_SINNOH_GREAT_MARSH6;
}

u16 Sinnoh_InGreatMarsh(void)
{
    return InGreatMarsh();
}

// The map whose wild Pokemon are met where the player stands (FALSE: not in the GREAT MARSH)
bool8 Sinnoh_GreatMarshWildMap(u8 *group, u8 *num)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 bx, bz;
    u16 map;
    if (!InGreatMarsh())
        return FALSE;
    bx = (player->currentCoords.x - MAP_OFFSET) / 32;
    bz = (player->currentCoords.y - MAP_OFFSET) / 32;
    bx = bx <= 1 ? 0 : 1;
    bz = bz <= 1 ? 0 : bz >= 3 ? 2 : 1;
    map = MAP_SINNOH_GREAT_MARSH1 + bz * 2 + bx;
    *group = map >> 8;
    *num = map & 0xFF;
    return TRUE;
}

// Platinum's daily Pokemon of the GREAT MARSH (its SAFARI GAME only): each area has one a day in its 5 percent slots
// (6 and 7), one of 32 picks; after SINNOH's LEAGUE (Platinum: the NATIONAL POKeDEX) the second list
static const u16 sMarshDaily[2][32] = {
    {
        SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_CARNIVINE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_WOOPER,
        SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_SKORUPI, SPECIES_CARNIVINE,
        SPECIES_YANMA, SPECIES_WOOPER, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_WOOPER, SPECIES_TROPIUS, SPECIES_WOOPER, SPECIES_WOOPER,
        SPECIES_SKORUPI, SPECIES_CARNIVINE, SPECIES_CROAGUNK, SPECIES_TANGELA,
    },
    {
        SPECIES_TOXICROAK, SPECIES_KECLEON, SPECIES_KECLEON, SPECIES_CARNIVINE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_DRAPION,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_QUAGSIRE, SPECIES_KANGASKHAN,
        SPECIES_PARAS, SPECIES_DRAPION, SPECIES_EXEGGCUTE, SPECIES_EXEGGCUTE,
        SPECIES_SKORUPI, SPECIES_CROAGUNK, SPECIES_SKORUPI, SPECIES_CARNIVINE,
        SPECIES_YANMA, SPECIES_SHROOMISH, SPECIES_PARAS, SPECIES_KANGASKHAN,
        SPECIES_GULPIN, SPECIES_TROPIUS, SPECIES_GULPIN, SPECIES_SHROOMISH,
        SPECIES_SKORUPI, SPECIES_CARNIVINE, SPECIES_CROAGUNK, SPECIES_TANGELA,
    },
};

// Today's pick for area AREA (0-5): the day and the player's Trainer ID, so each save has its own days
u16 Sinnoh_GreatMarshDailySpecies(u8 area)
{
    u32 seed = RtcGetLocalDayCount() * 2654435761u ^ (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8
             | gSaveBlock2Ptr->playerTrainerId[2] << 16 | (u32)gSaveBlock2Ptr->playerTrainerId[3] << 24);
    seed ^= seed >> 15;
    seed *= 0x2C1B3C6Du;
    seed ^= seed >> 12;
    return sMarshDaily[MP_Region_Cleared(REGION_SINNOH) ? 1 : 0][(seed >> (5 * area)) & 31];
}

// The SAFARI GAME's land Pokemon for slot INDEX (SPECIES: the table's): today's pick in the 5 percent slots
u16 Sinnoh_GreatMarshDaily(u16 species, u8 index)
{
    u8 group, num;
    u16 daily;
    if ((index != 6 && index != 7) || !FlagGet(FLAG_SYS_SAFARI_MODE) || !Sinnoh_GreatMarshWildMap(&group, &num))
        return species;
    daily = Sinnoh_GreatMarshDailySpecies(num - MAP_NUM(MAP_SINNOH_GREAT_MARSH1));
    return MP_Gen_Allowed(daily) ? daily : species;
}
