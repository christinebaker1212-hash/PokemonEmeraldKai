// Moemon Kai: SINNOH's legendaries standing on the map (tools/sinnoh/legends.py, src/data/sinnoh_legends.h): on entering
// their map (special Sinnoh_LegendsRefresh) one is shown unless it was caught or beaten, or its time hasn't come:
// UXIE, AZELF, MESPRIT and GIRATINA from the 7th BADGE (when Platinum's story gives them), the others once SINNOH's
// LEAGUE is beaten
#include "global.h"
#include "event_data.h"
#include "mp_region.h"
#include "constants/species.h"
#include "constants/map_groups.h"
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
