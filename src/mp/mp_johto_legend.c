// Moemon Kai: helpers for Crystal's legendary story in JOHTO (audit cases 65 / 139; the scripts in
// data/maps/Johto_TinTower1F, Johto_TinTowerRoof, Johto_WhirlIslandLugiaChamber).
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "mp.h"
#include "mp_region.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "constants/species.h"

void MP_Legend_Beaten(u16 flag);

static bool8 Owns(u16 species)
{
    struct PokemonStorage *pc = MP_PersonalStorage();
    u32 b, i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == species && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG, NULL))
            return TRUE;
    if (pc != NULL)
        for (b = 0; b < TOTAL_BOXES_COUNT; b++)
            for (i = 0; i < IN_BOX_COUNT; i++)
                if (GetBoxMonData(&pc->boxes[b][i], MON_DATA_SPECIES, NULL) == species
                 && !GetBoxMonData(&pc->boxes[b][i], MON_DATA_IS_EGG, NULL))
                    return TRUE;
    return FALSE;
}

// Crystal's MonCheck: VAR_RESULT 1 when the player has VAR_0x8004 in the party or the PC
u16 MP_JohtoMonCheck(void)
{
    return gSpecialVar_Result = Owns(gSpecialVar_0x8004);
}

// Crystal's BeastsCheck: RAIKOU, ENTEI and SUICUNE all the player's (TIN TOWER's sage gives the RAINBOW WING)
u16 MP_JohtoBeastsCheck(void)
{
    return gSpecialVar_Result = Owns(SPECIES_RAIKOU) && Owns(SPECIES_ENTEI) && Owns(SPECIES_SUICUNE);
}

// After a scripted legendary battle: VAR_0x8004 the flag that hides it. Caught or beaten, it's gone; beaten, it comes
// back in a day (mp_legends.c, as every one-time legendary). VAR_RESULT 1 when it's gone
u16 MP_JohtoLegendAfterBattle(void)
{
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_CAUGHT)
        return gSpecialVar_Result = 0;
    FlagSet(gSpecialVar_0x8004);
    if (gBattleOutcome == B_OUTCOME_WON)
        MP_Legend_Beaten(gSpecialVar_0x8004);
    return gSpecialVar_Result = 1;
}

// VAR_RESULT 1 once JOHTO's ELITE FOUR and CHAMPION are beaten (Crystal's EVENT_BEAT_ELITE_FOUR)
u16 MP_JohtoLeagueBeaten(void)
{
    bool8 MP_Region_Cleared(u8 region);
    return gSpecialVar_Result = MP_Region_Cleared(REGION_JOHTO);
}
