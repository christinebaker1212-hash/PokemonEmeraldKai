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

// Quetzal's specials write VAR_RESULT themselves (its scripts use "special", not "specialvar"): the ones that answer
// something are wrappers at the end of this file around their _ function.

// ---------------------------------------------------------------------------------------------------- nothing to do
// Quetzal's options Kai doesn't have (Nuzlocke, randomizers, its cheat mode, link checks) answer "off"
u16 QzQuestLogNop(void) { return 0; }
static u16 QzReturnZero_(void) { return 0; }
static u16 QzIsNuzlockeActiveInRegion_(void) { return FALSE; }
static u16 QzIsNuzlockeOption2D4Bit6_(void) { return FALSE; }
static u16 QzIsItemRandomizerOn_(void) { return FALSE; }
u16 QzRandomizeItemInVar8000(void) { return 0; }
static u16 QzIsSaveFlag2F4Bit4Set_(void) { return FALSE; }
u16 QzIncrementChallengeCounterVar(void) { return 0; }
u16 QzBackupPartyForOptionBattle(void) { return 0; }
u16 QzQzEnemyMonEventFixup(void) { return 0; }
u16 QzHoennPostHealEventCheck(void) { return 0; }
u16 QzResetFollowerObjects(void) { return 0; }   // Kai's followers aren't objects
u16 QzSetEnemyMonOverrides(void) { return 0; }

// Gym leaders' rematch teams: Kai's level scaling and difficulty do that job
static u16 QzUseAltGymLeaderTeam_(void) { return FALSE; }

// HMs work without badges in Kai (mp_heldhm.c)
static u16 QzHasBadgeForFieldMove_(void) { return TRUE; }

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

static u16 QzIsNationalPokedexEnabled_(void)
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
static u16 QzGetPokedexCount_(void)
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
static u16 QzIsRegionalDexComplete_(void)
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

static u16 QzPartyHasSpecies_(void)
{
    return PartyHas(Species());
}

static u16 QzPartyHasTogepiLineOrTogekiss_(void)
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
static u16 QzIsLeadMonHoOh_(void) { return LeadIs(SPECIES_HO_OH); }
static u16 QzIsLeadMonAerodactyl_(void) { return LeadIs(SPECIES_AERODACTYL); }
static u16 QzIsLeadMonKabuto_(void) { return LeadIs(SPECIES_KABUTO); }
static u16 QzIsLeadMonOmanyte_(void) { return LeadIs(SPECIES_OMANYTE); }

static u16 QzIsCurrentPcBoxFull_(void)
{
    u8 i, box = StorageGetCurrentBox();
    for (i = 0; i < IN_BOX_COUNT; i++)
        if (GetBoxMonDataAt(box, i, MON_DATA_SPECIES) == SPECIES_NONE)
            return FALSE;
    return TRUE;
}

// The rival's starter: the one strong against yours (Kanto: var 0x5143 is the player's pick 1-3)
static u16 QzGetRivalStarterSpecies_(void)
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
static u16 QzGetRegionBadgeCount_(void)
{
    return MP_Region_BadgeCount(MP_Region_Current());
}

// The gate between Johto's and Kanto's Victory Roads: champions of both leagues only
static u16 QzVictoryRoadGateCheck_(void)
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

// ---------------------------------------------------------------------------------------------------- static encounters
#include "battle.h"
#include "battle_setup.h"
#include "constants/battle.h"
#include "data/qz_static_encounters.h"

// Quetzal's own numbers (its C code, not its scripts, uses them)
#define QZ_VAR_STATIC_ENTRY       0x506F   // the Johto static encounter being fought (entry + 1)
#define QZ_VAR_KANTO_STATIC_ENTRY 0x5148   // the same for Kanto's lookup by species
#define QZ_FLAG_UNOWN_STATIC      0x1313


static bool8 HereIs(const struct QzStatic *e)
{
    return e->group == gSaveBlock1Ptr->location.mapGroup && e->num == gSaveBlock1Ptr->location.mapNum;
}

// The entry for the object the player talked to (by the flag it hides with)
static const struct QzStatic *TalkedStatic(void)
{
    u32 i;
    u8 id = GetObjectEventIdByLocalIdAndMap(gSpecialVar_LastTalked, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    u16 flag = 0;
    if (id < OBJECT_EVENTS_COUNT)
    {
        const struct ObjectEventTemplate *t = GetObjectEventTemplateByLocalIdAndMap(gSpecialVar_LastTalked,
            gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        if (t)
            flag = t->flagId;
    }
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
        if (HereIs(&sQzStatics[i]) && (flag == 0 || sQzStatics[i].objFlag == flag))
            return &sQzStatics[i];
    return NULL;
}

static u16 SetUpStatic(const struct QzStatic *e)
{
    if (e == NULL)
        return FALSE;
    gSpecialVar_0x8004 = e->species;
    gSpecialVar_0x8005 = e->level;
    gSpecialVar_0x8006 = e->item;
    VarSet(QZ_VAR_STATIC_ENTRY, (e - sQzStatics) + 1);
    PlayCry_Normal(e->species, 0);
    return TRUE;
}

// Johto: the Pokémon standing there (VAR_0x8004-0x8006 for CreateEnemyEventMon)
static u16 QzJohtoStaticEncounterSetup_(void) { return SetUpStatic(TalkedStatic()); }
static u16 QzJohtoStaticEncounterSetup2_(void) { return SetUpStatic(TalkedStatic()); }
static u16 QzJohtoStaticEncounterSetupTower_(void) { return SetUpStatic(TalkedStatic()); }

static u16 QzKantoStaticEncounterSetupByLocalId_(void)
{
    u16 ok = SetUpStatic(TalkedStatic());
    gSpecialVar_0x8006 = 0;
    return ok;
}

// Kanto, by species (VAR_0x8004) on this map: var 0x5148 = the entry
static u16 QzKantoStaticEncounterLookupBySpecies_(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
        if (HereIs(&sQzStatics[i]) && sQzStatics[i].species == gSpecialVar_0x8004)
        {
            VarSet(QZ_VAR_KANTO_STATIC_ENTRY, i + 1);
            printf("[qz] static encounter %u: species %u\n", (unsigned)i, sQzStatics[i].species);
            return TRUE;
        }
    printf("[qz] no static encounter for species %u on map %u.%u\n", gSpecialVar_0x8004, gSaveBlock1Ptr->location.mapGroup,
           gSaveBlock1Ptr->location.mapNum);
    return FALSE;
}

u16 QzKantoStaticEncounterPrepareMon(void)
{
    CreateScriptedWildMon(gSpecialVar_0x8004, gSpecialVar_0x8005, gSpecialVar_0x8006);
    return 0;
}

// After the battle: caught or beaten, the Pokémon stays gone (its object's flag) and its entry is done
static u16 AfterStatic(u16 entryVar)
{
    u16 n = VarGet(entryVar);
    const struct QzStatic *e;
    if (n == 0 || n > ARRAY_COUNT(sQzStatics))
        return FALSE;
    e = &sQzStatics[n - 1];
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_CAUGHT)
        return FALSE;
    FlagSet(e->objFlag);
    FlagSet(e->doneFlag);
    return TRUE;
}

static u16 QzJohtoStaticEncounterAfterBattle_(void) { return AfterStatic(QZ_VAR_STATIC_ENTRY); }

static u16 QzKantoStaticEncounterAfterBattle_(void)
{
    u16 n = VarGet(QZ_VAR_KANTO_STATIC_ENTRY) ? QZ_VAR_KANTO_STATIC_ENTRY : QZ_VAR_STATIC_ENTRY;
    u16 done = AfterStatic(n);
    gSpecialVar_0x8004 = gSpecialVar_LastTalked;
    VarSet(QZ_VAR_KANTO_STATIC_ENTRY, 0);
    return done;
}

static u16 QzClearStaticFlagAfterWildBattle_(void)
{
    if (gBattleOutcome != B_OUTCOME_WON && gBattleOutcome != B_OUTCOME_CAUGHT)
        return FALSE;
    FlagClear(QZ_FLAG_UNOWN_STATIC);
    return TRUE;
}

// On entering a map: a done Pokémon stays hidden; a Gen 4 one waits until an Elite Four is beaten somewhere
static u16 RefreshStatics(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
    {
        const struct QzStatic *e = &sQzStatics[i];
        if (!HereIs(e))
            continue;
        if (FlagGet(e->doneFlag) || (IS_GEN4_SPECIES(e->species) && !MP_Region_AnyCleared()))
            FlagSet(e->objFlag);
        else
            FlagClear(e->objFlag);
    }
    return 0;
}

u16 QzJohtoStaticEncountersRefresh(void) { return RefreshStatics(); }
u16 QzKantoStaticEncountersRefresh(void) { return RefreshStatics(); }

// Ruins of Alph: the UNOWN behind the last wall
u16 QzSetupUnownStaticEncounter(void)
{
    gSpecialVar_0x8004 = SPECIES_UNOWN;
    gSpecialVar_0x8005 = 40;
    gSpecialVar_0x8006 = ITEM_NONE;
    PlayCry_Normal(SPECIES_UNOWN, 0);
    return 0;
}

// Ice Path's legendary: its IVs perfect
u16 QzSetupIcePathLegendary(void)
{
    u8 i, iv = MAX_PER_STAT_IVS;
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(&gEnemyParty[0], MON_DATA_HP_IV + i, &iv);
    CalculateMonStats(&gEnemyParty[0]);
    return 0;
}

// Pokémon Tower's ghost: MAROWAK's spirit
static u16 QzStartMarowakGhostBattle_(void)
{
    CreateScriptedWildMon(SPECIES_MAROWAK, 30, ITEM_NONE);
    BattleSetup_StartScriptedWildBattle();
    return 0;
}
u16 QzReturnZero(void) { return gSpecialVar_Result = QzReturnZero_(); }
u16 QzIsNuzlockeActiveInRegion(void) { return gSpecialVar_Result = QzIsNuzlockeActiveInRegion_(); }
u16 QzIsNuzlockeOption2D4Bit6(void) { return gSpecialVar_Result = QzIsNuzlockeOption2D4Bit6_(); }
u16 QzIsItemRandomizerOn(void) { return gSpecialVar_Result = QzIsItemRandomizerOn_(); }
u16 QzIsSaveFlag2F4Bit4Set(void) { return gSpecialVar_Result = QzIsSaveFlag2F4Bit4Set_(); }
u16 QzUseAltGymLeaderTeam(void) { return gSpecialVar_Result = QzUseAltGymLeaderTeam_(); }
u16 QzHasBadgeForFieldMove(void) { return gSpecialVar_Result = QzHasBadgeForFieldMove_(); }
u16 QzIsNationalPokedexEnabled(void) { return gSpecialVar_Result = QzIsNationalPokedexEnabled_(); }
u16 QzGetPokedexCount(void) { return gSpecialVar_Result = QzGetPokedexCount_(); }
u16 QzIsRegionalDexComplete(void) { return gSpecialVar_Result = QzIsRegionalDexComplete_(); }
u16 QzPartyHasSpecies(void) { return gSpecialVar_Result = QzPartyHasSpecies_(); }
u16 QzPartyHasTogepiLineOrTogekiss(void) { return gSpecialVar_Result = QzPartyHasTogepiLineOrTogekiss_(); }
u16 QzIsLeadMonHoOh(void) { return gSpecialVar_Result = QzIsLeadMonHoOh_(); }
u16 QzIsLeadMonAerodactyl(void) { return gSpecialVar_Result = QzIsLeadMonAerodactyl_(); }
u16 QzIsLeadMonKabuto(void) { return gSpecialVar_Result = QzIsLeadMonKabuto_(); }
u16 QzIsLeadMonOmanyte(void) { return gSpecialVar_Result = QzIsLeadMonOmanyte_(); }
u16 QzIsCurrentPcBoxFull(void) { return gSpecialVar_Result = QzIsCurrentPcBoxFull_(); }
u16 QzGetRivalStarterSpecies(void) { return gSpecialVar_Result = QzGetRivalStarterSpecies_(); }
u16 QzGetRegionBadgeCount(void) { return gSpecialVar_Result = QzGetRegionBadgeCount_(); }
u16 QzVictoryRoadGateCheck(void) { return gSpecialVar_Result = QzVictoryRoadGateCheck_(); }
u16 QzJohtoStaticEncounterSetup(void) { return gSpecialVar_Result = QzJohtoStaticEncounterSetup_(); }
u16 QzJohtoStaticEncounterSetup2(void) { return gSpecialVar_Result = QzJohtoStaticEncounterSetup2_(); }
u16 QzJohtoStaticEncounterSetupTower(void) { return gSpecialVar_Result = QzJohtoStaticEncounterSetupTower_(); }
u16 QzKantoStaticEncounterSetupByLocalId(void) { return gSpecialVar_Result = QzKantoStaticEncounterSetupByLocalId_(); }
u16 QzKantoStaticEncounterLookupBySpecies(void) { return gSpecialVar_Result = QzKantoStaticEncounterLookupBySpecies_(); }
u16 QzJohtoStaticEncounterAfterBattle(void) { return gSpecialVar_Result = QzJohtoStaticEncounterAfterBattle_(); }
u16 QzKantoStaticEncounterAfterBattle(void) { return gSpecialVar_Result = QzKantoStaticEncounterAfterBattle_(); }
u16 QzClearStaticFlagAfterWildBattle(void) { return gSpecialVar_Result = QzClearStaticFlagAfterWildBattle_(); }
u16 QzStartMarowakGhostBattle(void) { return gSpecialVar_Result = QzStartMarowakGhostBattle_(); }
