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
#include "item.h"
#include "item_menu.h"
#include "main.h"
#include "shop.h"
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
void ChangePokemonNickname(void);
void ChangeBoxPokemonNickname(void);
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
u16 QzBackupPartyForOptionBattle(void) { ScriptContext_Enable(); return 0; }   // a script waits on it
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

// ---------------------------------------------------------------------------------------------------- gifts, nicknames
// After a gift (givemon's VAR_RESULT: 0 party, 1 PC): VAR_0x8004 = its party slot, or 6 in the PC; its name in STR_VAR_1
static u16 QzBufferLastReceivedMonForNickname_(void)
{
    u16 species;
    if (gSpecialVar_Result == MON_GIVEN_TO_PC)
    {
        gSpecialVar_0x8004 = PARTY_SIZE;
        species = GetBoxMonDataAt(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos, MON_DATA_SPECIES);
    }
    else
    {
        gSpecialVar_0x8004 = CalculatePlayerPartyCount() - 1;
        species = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES);
    }
    GetSpeciesName(gStringVar1, species);
    return TRUE;
}

u16 QzNicknameLastReceivedMon(void)
{
    if (gSpecialVar_0x8004 < PARTY_SIZE)
        ChangePokemonNickname();
    else
        ChangeBoxPokemonNickname();
    return 0;
}

// ---------------------------------------------------------------------------------------------------- fossils
static const u16 sFossils[][2] =
{
    {ITEM_HELIX_FOSSIL, SPECIES_OMANYTE}, {ITEM_DOME_FOSSIL, SPECIES_KABUTO}, {ITEM_OLD_AMBER, SPECIES_AERODACTYL},
    {ITEM_ROOT_FOSSIL, SPECIES_LILEEP}, {ITEM_CLAW_FOSSIL, SPECIES_ANORITH}, {ITEM_ARMOR_FOSSIL, SPECIES_SHIELDON},
    {ITEM_SKULL_FOSSIL, SPECIES_CRANIDOS},
};

static u16 FossilSpecies(u16 item)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sFossils); i++)
        if (sFossils[i][0] == item)
            return sFossils[i][1];
    return SPECIES_NONE;
}

static u16 QzBagHasFossil_(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sFossils); i++)
        if (CheckBagHasItem(sFossils[i][0], 1))
            return TRUE;
    return FALSE;
}

static void CB2_ChooseFossil(void)
{
    GoToBagMenu(ITEMMENULOCATION_BERRY_TREE, KEYITEMS_POCKET, CB2_ReturnToFieldContinueScript);
}

// The bag's KEY ITEMS to pick a fossil from (VAR_ITEM_ID, 0 when backed out); waitstate
u16 QzChooseItemFromBagForFossil(void)
{
    SetMainCallback2(CB2_ChooseFossil);
    return 0;
}

static u16 QzIsSelectedItemFossil_(void) { return FossilSpecies(gSpecialVar_ItemId) != SPECIES_NONE; }
static u16 QzIsSelectedItemFossilHalf_(void) { return FALSE; }       // Galar's fossil halves are past the cap
static u16 QzAreFossilHalvesCompatible_(void) { return FALSE; }

// The Pokémon of the fossil VAR_ITEM_ID (Lv 20 in Kanto, 10 in Johto) into VAR_0x800A; VAR_RESULT = where it went
static u16 QzGiveRevivedFossilMon_(void)
{
    u16 species = FossilSpecies(gSpecialVar_ItemId);
    if (species == SPECIES_NONE)
        return MON_CANT_GIVE;
    gSpecialVar_0x800A = species;
    return ScriptGiveMon(species, MP_Region_Current() == REGION_JOHTO ? 10 : 20, ITEM_NONE, 0, 0, 0);
}

// ---------------------------------------------------------------------------------------------------- marts
// The Celadon / Goldenrod counters for items of legend; Mega Stones and Z-Crystals are past the cap
static const u16 sLegendaryItems[] = {ITEM_SOUL_DEW, ITEM_ADAMANT_ORB, ITEM_LUSTROUS_ORB, ITEM_GRISEOUS_ORB, ITEM_NONE};
static const u16 sNoItems[] = {ITEM_NONE};

u16 QzShowLegendaryItemShop(void)
{
    CreatePokemartMenu(sLegendaryItems);
    return 0;
}

u16 QzShowMegaStoneShop(void)
{
    CreatePokemartMenu(sNoItems);
    return 0;
}

u16 QzShowZCrystalShop(void)
{
    CreatePokemartMenu(sNoItems);
    return 0;
}
u16 QzBufferLastReceivedMonForNickname(void) { return gSpecialVar_Result = QzBufferLastReceivedMonForNickname_(); }
u16 QzBagHasFossil(void) { return gSpecialVar_Result = QzBagHasFossil_(); }
u16 QzIsSelectedItemFossil(void) { return gSpecialVar_Result = QzIsSelectedItemFossil_(); }
u16 QzIsSelectedItemFossilHalf(void) { return gSpecialVar_Result = QzIsSelectedItemFossilHalf_(); }
u16 QzAreFossilHalvesCompatible(void) { return gSpecialVar_Result = QzAreFossilHalvesCompatible_(); }
u16 QzGiveRevivedFossilMon(void) { return gSpecialVar_Result = QzGiveRevivedFossilMon_(); }

// ---------------------------------------------------------------------------------------------------- Kanto (FireRed's)
#include "constants/field_specials.h"
#include "constants/game_stat.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "script_menu.h"

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;

// Vermilion's gym: the first switch's can at random, the second next to it
u16 QzSetVermilionTrashCans(void)
{
    u16 idx = (Random() % 15) + 1;
    s16 second = idx;
    static const s8 sNear[16][4] = {
        {0}, {1, 5}, {1, 5, -1}, {1, 5, -1}, {1, 5, -1}, {5, -1}, {-5, 1, 5}, {-5, 1, 5, -1}, {-5, 1, 5, -1},
        {-5, 1, 5, -1}, {-5, 5, -1}, {-5, 1}, {-5, 1, -1}, {-5, 1, -1}, {-5, 1, -1}, {-5, -1},
    };
    u8 n = 0;
    while (n < 4 && sNear[idx][n] != 0)
        n++;
    second += sNear[idx][Random() % n];
    if (second < 1 || second > 15)
        second = (idx % 5 == 0) ? idx - 1 : idx + 1;
    gSpecialVar_0x8004 = idx;
    gSpecialVar_0x8005 = second;
    return 0;
}

static u16 QzIsPlayerLeftOfVermilionSailor_(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_KANTO_VERMILION_CITY)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_KANTO_VERMILION_CITY) && gSaveBlock1Ptr->pos.x < 24;
}

// Elevators: var 0x5104 = the floor of the map the elevator was entered from (4 = 1F, as Emerald's floor names)
static const struct { u16 map; u8 floor; } sElevatorFloors[] =
{
    {MAP_KANTO_SILPH_CO_1F, 4}, {MAP_KANTO_SILPH_CO_2F, 5}, {MAP_KANTO_SILPH_CO_3F, 6}, {MAP_KANTO_SILPH_CO_4F, 7},
    {MAP_KANTO_SILPH_CO_5F, 8}, {MAP_KANTO_SILPH_CO_6F, 9}, {MAP_KANTO_SILPH_CO_7F, 10}, {MAP_KANTO_SILPH_CO_8F, 11},
    {MAP_KANTO_SILPH_CO_9F, 12}, {MAP_KANTO_SILPH_CO_10F, 13}, {MAP_KANTO_SILPH_CO_11F, 14},
    {MAP_KANTO_ROCKET_HIDEOUT_B1F, 3}, {MAP_KANTO_ROCKET_HIDEOUT_B2F, 2}, {MAP_KANTO_ROCKET_HIDEOUT_B4F, 0},
    {MAP_KANTO_CELADON_CITY_DEPARTMENT_STORE_1F, 4}, {MAP_KANTO_CELADON_CITY_DEPARTMENT_STORE_2F, 5},
    {MAP_KANTO_CELADON_CITY_DEPARTMENT_STORE_3F, 6}, {MAP_KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, 7},
    {MAP_KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, 8}, {MAP_KANTO_TRAINER_TOWER_LOBBY, 4}, {MAP_KANTO_TRAINER_TOWER_ROOF, 15},
};

static u8 ElevatorFloor(void)
{
    u8 i;
    u16 map = (gSaveBlock1Ptr->dynamicWarp.mapGroup << 8) | gSaveBlock1Ptr->dynamicWarp.mapNum;
    for (i = 0; i < ARRAY_COUNT(sElevatorFloors); i++)
        if (sElevatorFloors[i].map == map)
            return sElevatorFloors[i].floor;
    return 4;
}

u16 QzGetElevatorFloor(void)
{
    VarSet(0x5104, ElevatorFloor());
    return 0;
}

// The floor list's row for the floor you're on (Silph: 11F first)
static u16 QzGetElevatorFloorCursor_(void)
{
    u8 floor = ElevatorFloor();
    return floor >= 4 && floor <= 14 ? 14 - floor : 0;
}

// FireRed's script lists: 0 the badges (Cerulean's house), 1 Silph Co.'s floors; waitstate, VAR_RESULT = the row
u16 QzListMenu(void)
{
    u16 list = gSpecialVar_0x8004;
    if (list == 0)
        gSpecialVar_0x8004 = SCROLL_MULTI_QZ_KANTO_BADGES;
    else if (list == 1)
        gSpecialVar_0x8004 = SCROLL_MULTI_QZ_SILPH_FLOORS;
    else
    {
        gSpecialVar_Result = MULTI_B_PRESSED;
        ScriptContext_Enable();
        return 0;
    }
    ShowScrollableMultichoice();
    return 0;
}

u16 QzReturnToListMenu(void)
{
    ScrollableMultichoice_RedrawPersistentMenu();
    return 0;
}

u16 QzForcePlayerOntoBike(void)
{
    if (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ON_FOOT)
        SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_MACH_BIKE);
    Overworld_SetSavedMusic(MUS_CYCLING);
    Overworld_ChangeMusicTo(MUS_CYCLING);
    return 0;
}

u16 QzForcePlayerToStartSurfing(void)
{
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_SURFING);
    return 0;
}

// Lorelei's dolls: one more for every 25 entries in the HALL OF FAME
u16 QzUpdateLoreleiDollCollection(void)
{
    static const u16 sDolls[] = {FLAG_QZ_0x2197, FLAG_QZ_0x2195, FLAG_QZ_0x2192, FLAG_QZ_0x2196, FLAG_QZ_0x2193,
                                 FLAG_QZ_0x2190, FLAG_QZ_0x2194, FLAG_QZ_0x2191};
    u32 clears = GetGameStat(GAME_STAT_ENTERED_HOF), i;
    for (i = 0; i < ARRAY_COUNT(sDolls); i++)
        if (clears >= 25 * (i + 1))
            FlagClear(sDolls[i]);
    return 0;
}

// Pewter's museum: the fossils' Pokémon in a picture box
u16 QzOpenMuseumFossilPicKabutops(void) { ScriptMenu_ShowPokemonPic(SPECIES_KABUTOPS, 10, 3); return 0; }
u16 QzOpenMuseumFossilPicAerodactyl(void) { ScriptMenu_ShowPokemonPic(SPECIES_AERODACTYL, 10, 3); return 0; }
u16 QzCloseMuseumFossilPic(void) { ScriptMenu_HidePokemonPic(); return 0; }

// Bill's teleporter: the machine's own animation is FireRed's art; nothing waits on it
u16 QzAnimateTeleporterHousing(void) { return 0; }
u16 QzAnimateTeleporterCable(void) { return 0; }

// Resort Gorgeous: Selphy asks for a Pokémon you have seen, with a reward
static u16 QzSampleResortGorgeousMonAndReward_(void)
{
    static const u16 sRewards[] = {ITEM_BIG_PEARL, ITEM_PEARL, ITEM_STARDUST, ITEM_STAR_PIECE, ITEM_NUGGET, ITEM_RARE_CANDY};
    u16 species = VarGet(VAR_QZ_0x5141), i;
    if (species == SPECIES_NONE || species == 0xFFFF)
    {
        for (i = 0; i < 100; i++)
        {
            species = (Random() % (FIRST_GEN4_SPECIES - 1)) + 1;
            if (GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
                break;
        }
        if (i == 100)
            species = SPECIES_PIKACHU;
        VarSet(VAR_QZ_0x5141, species);
        VarSet(0x5142, sRewards[Random() % ARRAY_COUNT(sRewards)]);
    }
    GetSpeciesName(gStringVar1, VarGet(VAR_QZ_0x5141));
    return TRUE;
}

static u16 QzPartyHasRequestedResortMon_(void)
{
    return PartyHas(VarGet(VAR_QZ_0x5141));
}

// ---------------------------------------------------------------------------------------------------- the Seagallop
static const struct { u16 map; u8 x, y; } sSeagallopSpots[] =
{
    {MAP_KANTO_VERMILION_CITY, 0x17, 0x20}, {MAP_KANTO_ONE_ISLAND_HARBOR, 8, 5}, {MAP_KANTO_TWO_ISLAND_HARBOR, 8, 5},
    {MAP_KANTO_THREE_ISLAND_HARBOR, 8, 5}, {MAP_KANTO_FOUR_ISLAND_HARBOR, 8, 5}, {MAP_KANTO_FIVE_ISLAND_HARBOR, 8, 5},
    {MAP_KANTO_SIX_ISLAND_HARBOR, 8, 5}, {MAP_KANTO_SEVEN_ISLAND_HARBOR, 8, 5}, {MAP_KANTO_CINNABAR_ISLAND, 0x15, 7},
    {MAP_KANTO_NAVEL_ROCK_HARBOR, 8, 5}, {MAP_KANTO_BIRTH_ISLAND_HARBOR, 8, 5},
};
static const u8 sSeagallopNames[][14] = {
    _("VERMILION CITY"), _("ONE ISLAND"), _("TWO ISLAND"), _("THREE ISLAND"), _("FOUR ISLAND"), _("FIVE ISLAND"),
    _("SIX ISLAND"), _("SEVEN ISLAND"),
};
static const u8 sText_Exit[] = _("EXIT");
static u8 sSeagallopChoices[ARRAY_COUNT(sSeagallopNames) + 1];

// The ports you can sail to from VAR_0x8004 (the Sevii maps' flags open them); waitstate; FALSE when there is none
static u16 QzShowSeagallopDestinationMenu_(void)
{
    u8 i, n = 0;
    for (i = 0; i < ARRAY_COUNT(sSeagallopNames); i++)
    {
        if (i == gSpecialVar_0x8004)
            continue;
        if (i >= 1 && i <= 3 && !FlagGet(FLAG_QZ_0x2213))
            continue;
        if (i >= 4 && !FlagGet(FLAG_QZ_0x2214))
            continue;
        sSeagallopChoices[n] = i;
        gQzScrollOptions[n++] = sSeagallopNames[i];
    }
    if (n == 0)
        return FALSE;
    sSeagallopChoices[n] = 127;
    gQzScrollOptions[n++] = sText_Exit;
    gQzScrollCount = n;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return TRUE;
}

static u16 QzGetSeagallopMenuSelection_(void)
{
    if (gSpecialVar_Result >= gQzScrollCount)
        return 127;
    return sSeagallopChoices[gSpecialVar_Result];
}

// The ship's number (the scripts say it): FireRed's
static u16 QzGetSeagallopNumber_(void)
{
    u16 a = gSpecialVar_0x8004, b = gSpecialVar_0x8006;
    if (a == 8 || b == 8) return 1;
    if (a == 0 || b == 0) return 7;
    if (a == 9 || b == 9) return 10;
    if (a == 10 || b == 10) return 12;
    if (a >= 1 && a <= 3 && b >= 1 && b <= 3) return 2;
    if (a >= 4 && a <= 5 && b >= 4 && b <= 5) return 3;
    if (a >= 6 && a <= 7 && b >= 6 && b <= 7) return 5;
    return 6;
}

// Sail to VAR_0x8006 (FireRed's ferry scene is its own art: the trip is the screen's fade)
u16 QzDoSeagallopFerryScene(void)
{
    u16 dest = gSpecialVar_0x8006 < ARRAY_COUNT(sSeagallopSpots) ? gSpecialVar_0x8006 : 0;
    SetWarpDestination(sSeagallopSpots[dest].map >> 8, sSeagallopSpots[dest].map & 0xFF, WARP_ID_NONE,
                       sSeagallopSpots[dest].x, sSeagallopSpots[dest].y);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return 0;
}

// The S.S. ANNE leaves: FireRed's scene is its own art; the script goes on
u16 QzDoSSAnneDepartureCutscene(void)
{
    ScriptContext_Enable();
    return 0;
}

// The League's lights (FireRed's palette effect): the rooms show as they are
u16 QzDoPokemonLeagueLightingEffect(void) { return 0; }
u16 QzIsPlayerLeftOfVermilionSailor(void) { return gSpecialVar_Result = QzIsPlayerLeftOfVermilionSailor_(); }
u16 QzGetElevatorFloorCursor(void) { return gSpecialVar_Result = QzGetElevatorFloorCursor_(); }
u16 QzSampleResortGorgeousMonAndReward(void) { return gSpecialVar_Result = QzSampleResortGorgeousMonAndReward_(); }
u16 QzPartyHasRequestedResortMon(void) { return gSpecialVar_Result = QzPartyHasRequestedResortMon_(); }
u16 QzShowSeagallopDestinationMenu(void) { return gSpecialVar_Result = QzShowSeagallopDestinationMenu_(); }
u16 QzGetSeagallopMenuSelection(void) { return gSpecialVar_Result = QzGetSeagallopMenuSelection_(); }
u16 QzGetSeagallopNumber(void) { return gSpecialVar_Result = QzGetSeagallopNumber_(); }
