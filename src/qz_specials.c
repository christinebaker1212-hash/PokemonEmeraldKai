// Moemon Kai: Moemon Quetzal's specials, ported for the Johto / Kanto scripts (docs/quetzal/SPECIALS.md says what each
// did in Quetzal). The ones not here yet are stubs in src/qz_specials_stub.c (tools/quetzal_port/import.py leaves out
// every name this file defines).
#include <stdio.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "field_specials.h"
#include "mp.h"
#include "mp_region.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "qz_specials.h"
#include "random.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/qz_port.h"
#include "constants/species.h"

u8 GetDirectionToFace(s16 x, s16 y, s16 targetX, s16 targetY);
void CloseDeptStoreElevatorWindow(void);

// ---------------------------------------------------------------------------------------------------- nothing to do
// Quetzal's options Kai doesn't have (Nuzlocke, randomizers, its cheat mode, link checks) answer "off"
u16 QzQuestLogNop(void) { return 0; }
u16 QzReturnZero(void) { return 0; }
u16 QzIsNuzlockeActiveInRegion(void) { return FALSE; }
u16 QzIsNuzlockeOption2D4Bit6(void) { return FALSE; }
u16 QzIsItemRandomizerOn(void) { return FALSE; }
u16 QzRandomizeItemInVar8000(void) { return 0; }
u16 QzIsSaveFlag2F4Bit4Set(void) { return FALSE; }
u16 QzIncrementChallengeCounterVar(void) { return 0; }
u16 QzBackupPartyForOptionBattle(void) { return 0; }
u16 QzQzEnemyMonEventFixup(void) { return 0; }
u16 QzHoennPostHealEventCheck(void) { return 0; }
u16 QzResetFollowerObjects(void) { return 0; }   // Kai's followers aren't objects
u16 QzSetEnemyMonOverrides(void) { return 0; }

// Gym leaders' rematch teams: Kai's level scaling and difficulty do that job
u16 QzUseAltGymLeaderTeam(void) { return FALSE; }

// HMs work without badges in Kai (mp_heldhm.c)
u16 QzHasBadgeForFieldMove(void) { return TRUE; }

// ---------------------------------------------------------------------------------------------------- party, dex
u16 QzHealPlayerPartyEx(void)
{
    HealPlayerParty();
    return 0;
}

static u16 Species(void)
{
    return gSpecialVar_0x8004;
}

u16 QzSetSeenMon(void)
{
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(Species()), FLAG_SET_SEEN);
    return 0;
}

u16 QzIsNationalPokedexEnabled(void)
{
    return IsNationalPokedexEnabled();
}

static u16 KantoCount(u8 caseId)
{
    u16 i, n = 0;
    for (i = 1; i <= 151; i++)
        n += GetSetPokedexFlag(i, caseId) != 0;
    return n;
}

// VAR_0x8004 0: the Kanto dex (1-151), else the National: seen in 0x8005, caught in 0x8006
u16 QzGetPokedexCount(void)
{
    if (gSpecialVar_0x8004 == 0)
    {
        gSpecialVar_0x8005 = KantoCount(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = KantoCount(FLAG_GET_CAUGHT);
    }
    else
    {
        gSpecialVar_0x8005 = GetNationalPokedexCount(FLAG_GET_SEEN);
        gSpecialVar_0x8006 = GetNationalPokedexCount(FLAG_GET_CAUGHT);
    }
    return IsNationalPokedexEnabled();
}

// The regional dex of the region you're in, all caught (Kanto 1-151, Johto Crystal's 251)
u16 QzIsRegionalDexComplete(void)
{
    u16 i, last = MP_Region_Current() == REGION_JOHTO ? 251 : 151;
    for (i = 1; i <= last; i++)
        if (i != 151 && i != 251 && !GetSetPokedexFlag(i, FLAG_GET_CAUGHT))   // the mythicals don't count
            return FALSE;
    return TRUE;
}

static bool8 PartyHas(u16 species)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            return TRUE;
    return FALSE;
}

u16 QzPartyHasSpecies(void)
{
    return PartyHas(Species());
}

u16 QzPartyHasTogepiLineOrTogekiss(void)
{
    return PartyHas(SPECIES_TOGEPI) || PartyHas(SPECIES_TOGETIC) || PartyHas(SPECIES_TOGEKISS);
}

static bool8 LeadIs(u16 species)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            return GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == species;
    return FALSE;
}

// Ruins of Alph: the chambers' walls open for the Pokémon in front
u16 QzIsLeadMonHoOh(void) { return LeadIs(SPECIES_HO_OH); }
u16 QzIsLeadMonAerodactyl(void) { return LeadIs(SPECIES_AERODACTYL); }
u16 QzIsLeadMonKabuto(void) { return LeadIs(SPECIES_KABUTO); }
u16 QzIsLeadMonOmanyte(void) { return LeadIs(SPECIES_OMANYTE); }

u16 QzIsCurrentPcBoxFull(void)
{
    u8 i, box = StorageGetCurrentBox();
    for (i = 0; i < IN_BOX_COUNT; i++)
        if (GetBoxMonDataAt(box, i, MON_DATA_SPECIES) == SPECIES_NONE)
            return FALSE;
    return TRUE;
}

// The rival's starter: the one strong against yours (Kanto: var 0x5143 is the player's pick 1-3)
u16 QzGetRivalStarterSpecies(void)
{
    if (VarGet(VAR_QZ_0x5146))
        return VarGet(VAR_QZ_0x5146);
    switch (VarGet(VAR_QZ_0x5143))
    {
    case 1: return SPECIES_SQUIRTLE;
    case 2: return SPECIES_CHARMANDER;
    default: return SPECIES_BULBASAUR;
    }
}

// ---------------------------------------------------------------------------------------------------- objects, map
// Object VAR_0x8004 and object VAR_0x8005 (0xFF = the player) face each other
u16 QzObjectFaceObject(void)
{
    u8 a, b;
    u8 la = gSpecialVar_0x8004, lb = gSpecialVar_0x8005;
    struct ObjectEvent *oa, *ob;
    a = la == 0xFF ? gPlayerAvatar.objectEventId : GetObjectEventIdByLocalIdAndMap(la, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    b = lb == 0xFF ? gPlayerAvatar.objectEventId : GetObjectEventIdByLocalIdAndMap(lb, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    if (a >= OBJECT_EVENTS_COUNT || b >= OBJECT_EVENTS_COUNT)
        return 0;
    oa = &gObjectEvents[a];
    ob = &gObjectEvents[b];
    ObjectEventTurn(oa, GetDirectionToFace(oa->currentCoords.x, oa->currentCoords.y, ob->currentCoords.x, ob->currentCoords.y));
    ObjectEventTurn(ob, GetDirectionToFace(ob->currentCoords.x, ob->currentCoords.y, oa->currentCoords.x, oa->currentCoords.y));
    return 0;
}

// Re-enter the map where you stand (scripts change what it shows)
u16 QzReloadMapAtPlayerPos(void)
{
    s16 x, y;
    PlayerGetDestCoords(&x, &y);
    SetWarpDestination(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE, x - MAP_OFFSET, y - MAP_OFFSET);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return 0;
}

u16 QzCloseDeptStoreElevatorWindow(void)
{
    CloseDeptStoreElevatorWindow();
    return 0;
}

// ---------------------------------------------------------------------------------------------------- regions
u16 QzGetRegionBadgeCount(void)
{
    return MP_Region_BadgeCount(MP_Region_Current());
}

// The gate between Johto's and Kanto's Victory Roads: champions of both leagues only
u16 QzVictoryRoadGateCheck(void)
{
    return MP_Region_Cleared(REGION_JOHTO) && MP_Region_Cleared(REGION_KANTO);
}

// Quetzal's Giovanni mode (the Celebi past) and its flags: never on in Kai
u16 QzSetGiovanniModeFlag14A4(void)
{
    FlagClear(FLAG_QZ_0x14A4);
    return 0;
}

u16 QzClearRadioTowerFlag14A3(void)
{
    FlagClear(FLAG_QZ_0x14A3);
    return 0;
}

u16 QzClearSeagallopRestrictions(void)
{
    FlagClear(FLAG_QZ_0x2108);
    FlagClear(FLAG_QZ_0x2109);
    return 0;
}
