// Moemon Kai: Moemon Quetzal's specials, ported for the Johto / Kanto scripts (docs/quetzal/SPECIALS.md says what each
// did in Quetzal). The ones not here yet are stubs in src/qz_specials_stub.c (tools/quetzal_port/import.py leaves out
// every name this file defines).
#include <stdio.h>
#include "global.h"
#include "constants/moves.h"
#include "credits.h"
#include "event_data.h"
#include "load_save.h"
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
#include "strings.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "qz_specials.h"
#include "random.h"
#include "save.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "script_movement.h"
#include "task.h"
#include "palette.h"
#include "naming_screen.h"
#include "data.h"
#include "constants/characters.h"
#include "constants/trainers.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "sound.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/qz_port.h"
#include "constants/species.h"
#include "constants/rgb.h"

u8 GetDirectionToFace(s16 x, s16 y, s16 targetX, s16 targetY);
void ChangePokemonNickname(void);
void ChangeBoxPokemonNickname(void);
void CloseDeptStoreElevatorWindow(void);
bool8 PlayerNotAtTrainerHillEntrance(void);

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
// The ROCKET HIDEOUT's multi battle beside LANCE: pick three POKeMON, as for STEVEN's (the script waits; VAR_RESULT 0 =
// backed out). It was a stub, and the script asked again forever behind a black screen
u16 QzBackupPartyForOptionBattle(void)
{
    void ChooseHalfPartyForBattle(void);
    ChooseHalfPartyForBattle();
    return 0;
}
u16 QzQzEnemyMonEventFixup(void) { return 0; }
u16 QzHoennPostHealEventCheck(void) { return 0; }
u16 QzResetFollowerObjects(void) { return 0; }   // Kai's followers aren't objects
u16 QzSetEnemyMonOverrides(void) { return 0; }

// Gym leaders' and the ELITE FOUR's legendary teams: Quetzal's GYM TEAMS setting (off by default; Kai has no such option yet)
static u16 QzUseAltGymLeaderTeam_(void) { return FALSE; }

// HMs work without badges in Kai (mp_heldhm.c)
bool8 MP_PartyCanUseFieldMove(u16 move);   // (mp_region.c)
bool8 MP_Region_HasFieldMoveBadge(u8 hm);

// Moemon Kai: the field move in VAR_0x8004 (CUT, ROCK SMASH, STRENGTH, WHIRLPOOL): the region's badge for it
// (mp_region.c MP_Region_FieldMoveBadge: FireRed's in KANTO, Crystal's in JOHTO) and someone to use it (a party
// Pokémon that knows it or holds its HM, or a friend close by: the game's checkpartymove). Quetzal's said yes to all.
static u16 QzHasBadgeForFieldMove_(void)
{
    u16 move = gSpecialVar_0x8004;
    u8 hm;
    switch (move)
    {
    case MOVE_CUT:        hm = 0; break;
    case MOVE_STRENGTH:   hm = 3; break;
    case MOVE_ROCK_SMASH: hm = 5; break;
    case MOVE_WHIRLPOOL:  hm = 8; break;   // (JOHTO's GLACIERBADGE: MP_Region_FieldMoveBadge)
    default:              return TRUE;
    }
    if (!MP_Region_HasFieldMoveBadge(hm))
        return FALSE;
    if (move == MOVE_WHIRLPOOL)   // (no HM for it yet: SURF does, until one is added; docs/REGIONS_TODO.md)
        return MP_PartyCanUseFieldMove(MOVE_WHIRLPOOL) || MP_PartyCanUseFieldMove(MOVE_SURF);
    return MP_PartyCanUseFieldMove(move);
}

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

// Quetzal's Giovanni mode (the Celebi past): TEAM ROCKET in CHERRYGROVE, AZALEA, ROUTE 35 and GOLDENROD (flag 0x14A4
// hides them) only in the past (1.63.11: this cleared it and nothing set it, so they stood there from a new game on)
static void QzGiovanniRockets(void);
u16 QzSetGiovanniModeFlag14A4(void)
{
    QzGiovanniRockets();
    return 0;
}

u16 QzClearRadioTowerFlag14A3(void)
{
    FlagClear(FLAG_QZ_0x14A3);
    return 0;
}

// Quetzal's C cleared its own flags 0x2108 / 0x2109 here; in this port they're KANTO's FLAG_SYS_GAME_CLEAR and
// FLAG_DEFEATED_CHAMP, so CELIO taking the SAPPHIRE undid the LEAGUE (ferry to ISLANDS 4-7, MT. EMBER, the ELITE
// FOUR's rematch teams...). FireRed's SetPostgameFlags only sets flags, which the script does itself: nothing to do.
// (Saves it already hit get them back on load: mp_region.c KantoLeagueFlags.)
u16 QzClearSeagallopRestrictions(void)
{
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

// KANTO's ARCEUS trials (Quetzal): 17 ARCEUS of the types on routes and islands, shown once ARCEUS has come to the
// player's bedroom in PALLET (FLAG_QZ_0x2E13); the Normal-type one in ALTERING CAVE once the 17 are done
#define QZ_ARCEUS_FINAL_FLAG 0x2E15
static bool8 ArceusTrialWaits(const struct QzStatic *e)
{
    u32 i;
    if (e->region != 2 || e->species != SPECIES_ARCEUS)
        return FALSE;
    if (!FlagGet(FLAG_QZ_0x2E13))
        return TRUE;
    if (e->objFlag != QZ_ARCEUS_FINAL_FLAG)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
        if (sQzStatics[i].region == 2 && sQzStatics[i].species == SPECIES_ARCEUS
         && sQzStatics[i].objFlag != QZ_ARCEUS_FINAL_FLAG && !FlagGet(sQzStatics[i].doneFlag))
            return TRUE;
    return FALSE;
}

// Before a region's LEAGUE is beaten only the legendaries its own games let you catch then are out: JOHTO's RAIKOU,
// ENTEI and SUICUNE (Crystal), KANTO's ARTICUNO, ZAPDOS and MOLTRES (FireRed / LeafGreen; not their GALARIAN forms).
// Every other one waits for the LEAGUE (Quetzal shows them all from the start)
static bool8 PreLeagueLegend(const struct QzStatic *e)
{
    if (e->region == REGION_JOHTO)
        return e->species == SPECIES_RAIKOU || e->species == SPECIES_ENTEI || e->species == SPECIES_SUICUNE;
    if (e->region == REGION_KANTO)
        return e->gen == 0 && (e->species == SPECIES_ARTICUNO || e->species == SPECIES_ZAPDOS || e->species == SPECIES_MOLTRES);
    return FALSE;
}

// Moemon Kai: each region's legendaries only in their own region (the user, 2026-11-02: Quetzal put every
// generation's in JOHTO and KANTO, FireRed's Sevii islands and Emerald's event islands borrow other regions' too).
// A legendary that has no encounter in its home region (JIRACHI, MANAPHY, PHIONE) keeps the places it has.
static const u16 sLegendHome[][2] = {
    {SPECIES_ARTICUNO, REGION_KANTO}, {SPECIES_ZAPDOS, REGION_KANTO}, {SPECIES_MOLTRES, REGION_KANTO},
    {SPECIES_MEWTWO, REGION_KANTO}, {SPECIES_MEW, REGION_KANTO},
    {SPECIES_RAIKOU, REGION_JOHTO}, {SPECIES_ENTEI, REGION_JOHTO}, {SPECIES_SUICUNE, REGION_JOHTO},
    {SPECIES_LUGIA, REGION_JOHTO}, {SPECIES_HO_OH, REGION_JOHTO}, {SPECIES_CELEBI, REGION_JOHTO},
    {SPECIES_REGIROCK, REGION_HOENN}, {SPECIES_REGICE, REGION_HOENN}, {SPECIES_REGISTEEL, REGION_HOENN},
    {SPECIES_LATIAS, REGION_HOENN}, {SPECIES_LATIOS, REGION_HOENN}, {SPECIES_KYOGRE, REGION_HOENN},
    {SPECIES_GROUDON, REGION_HOENN}, {SPECIES_RAYQUAZA, REGION_HOENN}, {SPECIES_DEOXYS, REGION_HOENN},
    {SPECIES_UXIE, REGION_SINNOH}, {SPECIES_MESPRIT, REGION_SINNOH}, {SPECIES_AZELF, REGION_SINNOH},
    {SPECIES_DIALGA, REGION_SINNOH}, {SPECIES_PALKIA, REGION_SINNOH}, {SPECIES_HEATRAN, REGION_SINNOH},
    {SPECIES_REGIGIGAS, REGION_SINNOH}, {SPECIES_GIRATINA, REGION_SINNOH}, {SPECIES_CRESSELIA, REGION_SINNOH},
    {SPECIES_DARKRAI, REGION_SINNOH}, {SPECIES_SHAYMIN, REGION_SINNOH}, {SPECIES_ARCEUS, REGION_SINNOH},
};

bool8 MP_LegendForeign(u16 species, u8 region)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sLegendHome); i++)
        if (sLegendHome[i][0] == species)
            return sLegendHome[i][1] != region;
    return FALSE;
}

// FireRed's own island events keep their legendaries whatever region they're from (the user, 2026-11-02: they were
// in the original game): NAVEL ROCK's LUGIA and HO-OH, BIRTH ISLAND's DEOXYS. (Emerald's NAVEL ROCK and FARAWAY
// ISLAND aren't in this table: their own scripts, untouched.)
static bool8 OriginalGameEvent(const struct QzStatic *e)
{
    u16 map = (e->group << 8) | e->num;
    return map == MAP_KANTO_NAVEL_ROCK_BASE || map == MAP_KANTO_NAVEL_ROCK_SUMMIT || map == MAP_KANTO_BIRTH_ISLAND_EXTERIOR;
}

static bool8 HiddenForeignLegend(const struct QzStatic *e)
{
    return MP_LegendForeign(e->species, e->region) && !OriginalGameEvent(e);
}

// After a map's transition scripts (overworld.c): a legendary from another region that Quetzal placed stays hidden,
// also where the map's own scripts would show it
void MP_HideForeignLegends(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
        if (HereIs(&sQzStatics[i]) && HiddenForeignLegend(&sQzStatics[i]))
            FlagSet(sQzStatics[i].objFlag);
}

// On entering a map: a done Pokémon stays hidden; one of a later generation than the region shows waits (MP_Gen_Cap);
// one its region's own games give only after the LEAGUE waits for it (PreLeagueLegend; none is part of the story
// before it: only its own map's scripts and the postgame's read their flags)
static u16 RefreshStatics(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sQzStatics); i++)
    {
        const struct QzStatic *e = &sQzStatics[i];
        if (!HereIs(e))
            continue;
        if (FlagGet(e->doneFlag) || !MP_Gen_Allowed(e->species) || MP_Gen_Cap(e->region) < e->gen   // (a form: GALARIAN ARTICUNO...)
         || HiddenForeignLegend(e)
         || (e->region == REGION_JOHTO && (e->species == SPECIES_RAIKOU || e->species == SPECIES_ENTEI))   // (they roam: mp_johto_roamer.c)
         || (!MP_Region_Cleared(e->region) && !PreLeagueLegend(e))
         || (e->region == 2 && e->species == SPECIES_MELOETTA && !FlagGet(FLAG_QZ_0x2924))   // after the CHANSEY Dance
         || (e->species == SPECIES_DARKRAI && !FlagGet(e->region == 1 ? FLAG_QZ_0x1425 : FLAG_QZ_0x292B))   // in a dream
         || (e->region == 2 && e->species == SPECIES_MEW && e->group == 36 && !FlagGet(FLAG_QZ_0x2931))   // under the truck
         || ArceusTrialWaits(e))
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

// The walk between JOHTO's VICTORY ROAD gate and KANTO's ROUTE 22 gate (docs/REGIONS_AUDIT.md cases 43 and 70: Kai's
// customs on every crossing): for champions of both LEAGUES; VAR_0x8004 the region, VAR_0x8005 the one POKéMON who
// comes (EventScript_RegionCustoms), the others into the GLOBAL PC. RESULT 0: not allowed, or the GLOBAL PC is full
u16 MP_RegionGateCross(void)
{
    u8 to = gSpecialVar_0x8004, from = MP_Region_Current(), keep = gSpecialVar_0x8005, i;
    if (to >= REGION_COUNT || to == from || !QzVictoryRoadGateCheck_())
        return gSpecialVar_Result = 0;
    if (keep >= PARTY_SIZE || !GetMonData(&gPlayerParty[keep], MON_DATA_SANITY_HAS_SPECIES))
        keep = 0;
    if (MP_Region_GlobalFree() < CalculatePlayerPartyCount() - 1)
        return gSpecialVar_Result = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i == keep || !GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
            continue;
        MP_Region_DepositGlobal(&gPlayerParty[i].box);
        ZeroMonData(&gPlayerParty[i]);
    }
    CompactPartySlots();
    MP_Region_Travel(from, to);
    return gSpecialVar_Result = 1;
}
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
    // where the last gift really went (GiveMonToPlayer): scripts call this long after it, VAR_RESULT is long gone
    extern u8 gMpLastGivenMonPlace, gMpLastGivenMonSlot;
    u16 species;
    if (gMpLastGivenMonPlace == MON_GIVEN_TO_PC)
    {
        gSpecialVar_0x8004 = PARTY_SIZE;
        species = GetBoxMonDataAt(gSpecialVar_MonBoxId, gSpecialVar_MonBoxPos, MON_DATA_SPECIES);
    }
    else
    {
        gSpecialVar_0x8004 = gMpLastGivenMonPlace == MON_GIVEN_TO_PARTY ? gMpLastGivenMonSlot : CalculatePlayerPartyCount() - 1;
        species = GetMonData(&gPlayerParty[gSpecialVar_0x8004], MON_DATA_SPECIES);
    }
    if (species == SPECIES_NONE)
        return FALSE;   // (nothing to name: the script skips the question)
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

// GOLDENROD's department store elevator (Quetzal's ran in its own code: the rip has no panel script). The panel
// (data/scripts/mp_region.inc EventScript_GoldenrodElevator) lists B1F-6F; the floor picked becomes the elevator's
// way out (the dynamic warp: each floor's door at (9, 4), B1F's at (10, 6)). Floors in Emerald's numbering (B1F 3)
static const struct { u16 map; u8 x, y; const u8 *name; } sGoldenrodFloors[] = {
    {MAP_JOHTO_GOLDENROD_DEPT_STORE_B1F, 10, 6, gText_B1F}, {MAP_JOHTO_GOLDENROD_DEPT_STORE1F, 9, 4, gText_1F},
    {MAP_JOHTO_GOLDENROD_DEPT_STORE2F, 9, 4, gText_2F}, {MAP_JOHTO_GOLDENROD_DEPT_STORE3F, 9, 4, gText_3F},
    {MAP_JOHTO_GOLDENROD_DEPT_STORE4F, 9, 4, gText_4F}, {MAP_JOHTO_GOLDENROD_DEPT_STORE5F, 9, 4, gText_5F},
    {MAP_JOHTO_GOLDENROD_DEPT_STORE6F, 9, 4, gText_6F},
};
static u8 sGoldenrodPick;
extern u16 (*gQzScrollResult)(u16 choice);

static u8 GoldenrodFloorNow(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sGoldenrodFloors); i++)
        if (MAP_GROUP(sGoldenrodFloors[i].map) == gSaveBlock1Ptr->dynamicWarp.mapGroup
         && MAP_NUM(sGoldenrodFloors[i].map) == gSaveBlock1Ptr->dynamicWarp.mapNum)
            return i;
    return 1;
}

static u16 GoldenrodPicked(u16 row)
{
    sGoldenrodPick = row;
    return row < ARRAY_COUNT(sGoldenrodFloors) ? row : MULTI_B_PRESSED;
}

u16 QzGoldenrodElevatorMenu(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sGoldenrodFloors); i++)
        gQzScrollOptions[i] = sGoldenrodFloors[i].name;
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = GoldenrodPicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}

// VAR_RESULT 1: another floor (MoveElevator's VAR_0x8005 / 0x8006 are set: from, to)
u16 QzGoldenrodElevatorGo(void)
{
    u8 now = GoldenrodFloorNow(), to = sGoldenrodPick;
    if (to >= ARRAY_COUNT(sGoldenrodFloors) || to == now)
        return gSpecialVar_Result = 0;
    SetDynamicWarpWithCoords(0, MAP_GROUP(sGoldenrodFloors[to].map), MAP_NUM(sGoldenrodFloors[to].map), WARP_ID_NONE,
                             sGoldenrodFloors[to].x, sGoldenrodFloors[to].y);
    gSpecialVar_0x8005 = now + 3;
    gSpecialVar_0x8006 = to + 3;
    return gSpecialVar_Result = 1;
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

// FireRed's ReturnToListMenu: the list kept open after a choice (CERULEAN's badges) is drawn again and takes input
// again (it waited for ScrollableMultichoice_TryReturnToList: without it B and A did nothing and the player was stuck)
u16 QzReturnToListMenu(void)
{
    ScrollableMultichoice_RedrawPersistentMenu();
    ScrollableMultichoice_TryReturnToList();
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
        VarSet(VAR_KANTO_RESORT_STEPS, 0);
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
static const u8 sSeagallopNames[][15] = {
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

void MP_Voyage_Begin(bool8 eastbound, bool8 faded);   // src/mp/mp_voyage.c
// Sail from VAR_0x8004 to VAR_0x8006: FireRed's ferry scene (mp_voyage.c), the way FireRed's direction matrix says
// (pokefirered src/seagallop.c sTravelDirectionMatrix: bit DEST of ORIGIN's entry set = eastbound)
u16 QzDoSeagallopFerryScene(void)
{
    static const u16 sTravelDirectionMatrix[] = {0x6fe, 0x6fc, 0x6f8, 0x6f0, 0x6e0, 0x4c0, 0x400, 0x440, 0x7ff, 0x6e0, 0x000};
    u16 dest = gSpecialVar_0x8006 < ARRAY_COUNT(sSeagallopSpots) ? gSpecialVar_0x8006 : 0;
    bool8 east = gSpecialVar_0x8004 >= ARRAY_COUNT(sTravelDirectionMatrix) ? TRUE
               : (sTravelDirectionMatrix[gSpecialVar_0x8004] >> dest) & 1;
    SetWarpDestination(sSeagallopSpots[dest].map >> 8, sSeagallopSpots[dest].map & 0xFF, WARP_ID_NONE,
                       sSeagallopSpots[dest].x, sSeagallopSpots[dest].y);
    MP_Voyage_Begin(east, TRUE);   // (the script faded the screen already)
    return 0;
}

// The S.S. ANNE leaves, as FireRed's (pokefirered src/ss_anne.c): its horn, 50 frames, then the ship glides left a
// pixel every 5 frames until it's out of view, its horn again, 40 frames (the picture: mp_mappic.c gMpSSAnneShift)
extern s16 gMpSSAnneShift;
static void Task_SSAnneDeparts(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s32 screenX;
    switch (data[0])
    {
    case 0:
        if (++data[1] >= 50)
            data[0] = 1, data[1] = 0;
        break;
    case 1:
        data[1]++;
        gMpSSAnneShift = data[1] / 5;
        // (the ship's left edge on screen: FireRed stops when its sprite's middle is 120 px past the left edge)
        screenX = 30 * 16 + 8 - gMpSSAnneShift - (gSaveBlock1Ptr->pos.x * 16 + 8 - DISPLAY_WIDTH / 2);
        if (screenX < -120)
        {
            PlaySE(SE_RG_SS_ANNE_HORN);
            data[0] = 2, data[1] = 0;
        }
        break;
    case 2:
        if (++data[1] == 40)
        {
            DestroyTask(taskId);
            ScriptContext_Enable();
        }
        break;
    }
}

u16 QzDoSSAnneDepartureCutscene(void)
{
    gMpSSAnneShift = 0;
    PlaySE(SE_RG_SS_ANNE_HORN);
    CreateTask(Task_SSAnneDeparts, 8);
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

// ---------------------------------------------------------------------------------------------------- travelling events
#include "rtc.h"
#include "constants/maps.h"

// Quetzal's travelling visitors (Team Rocket's JESSIE and JAMES...) are in one town a day: its object shows there
// (flag 0x1490 Johto, 0x2EAB Kanto, clear = shown); VAR_RESULT = they are here
static const u16 sJohtoTowns[] = {MAP_JOHTO_NEW_BARK_TOWN, MAP_JOHTO_CHERRYGROVE_CITY, MAP_JOHTO_VIOLET_CITY, MAP_JOHTO_AZALEA_TOWN,
    MAP_JOHTO_GOLDENROD_CITY, MAP_JOHTO_ECRUTEAK_CITY, MAP_JOHTO_CIANWOOD_CITY, MAP_JOHTO_MAHOGANY_TOWN,
    MAP_JOHTO_BLACKTHORN_CITY, MAP_JOHTO_INDIGO_PLATEAU};
static const u16 sKantoTowns[] = {MAP_KANTO_PALLET_TOWN, MAP_KANTO_VIRIDIAN_CITY, MAP_KANTO_PEWTER_CITY, MAP_KANTO_CERULEAN_CITY,
    MAP_KANTO_VERMILION_CITY, MAP_KANTO_LAVENDER_TOWN, MAP_KANTO_CELADON_CITY, MAP_KANTO_FUCHSIA_CITY, MAP_KANTO_SAFFRON_CITY,
    MAP_KANTO_CINNABAR_ISLAND, MAP_KANTO_ONE_ISLAND, MAP_KANTO_TWO_ISLAND, MAP_KANTO_THREE_ISLAND, MAP_KANTO_FOUR_ISLAND,
    MAP_KANTO_FIVE_ISLAND, MAP_KANTO_SIX_ISLAND, MAP_KANTO_SEVEN_ISLAND};

// Quetzal's JESSIE & JAMES travelling battle (a town a day after the LEAGUE) is gone (docs/REGIONS_AUDIT.md case 16:
// FireRed / Crystal have no such event, and its PIKACHU reward was an empty POKéMON): their objects stay hidden
static u16 TravelingEvent(const u16 *towns, u8 count, u16 flag, u16 salt)
{
    (void)towns, (void)count, (void)salt;
    FlagSet(flag);
    return FALSE;
}

static u16 QzJohtoTravelingEventCheck_(void) { return TravelingEvent(sJohtoTowns, ARRAY_COUNT(sJohtoTowns), 0x1490, 1); }
static u16 QzKantoTravelingEventCheck_(void) { return TravelingEvent(sKantoTowns, ARRAY_COUNT(sKantoTowns), 0x2EAB, 2); }

// ---------------------------------------------------------------------------------------------------- small ones
static u16 QzPlayerNotAtTrainerHillEntrance_(void) { return PlayerNotAtTrainerHillEntrance(); }

// An NPC's line out of its set: by where it stands, so the same one each time
static u16 QzPickNpcFlavourLine_(void)
{
    return (gSpecialVar_LastTalked + gSaveBlock1Ptr->pos.x + gSaveBlock1Ptr->pos.y) & 7;
}

// Cinnabar's checklist of Quetzal's own events: nothing to list in Kai
static u16 QzBufferLabQuestChecklist_(void)
{
    gSpecialVar_0x8006 = TRUE;
    return 0;
}

// FireRed's SeafoamIslandsB4F_CurrentDumpsPlayerOnLand: the current sweeps the player off their POKéMON onto land, north
u16 QzSeafoamCurrentEffect(void)
{
    void MP_StopSurfingNorth(void);   // (field_player_avatar.c)
    MP_StopSurfingNorth();
    return 0;
}
u16 QzSetDeoxysRockPaletteVar5071(void) { return 0; }
u16 QzDoDeoxysRockInteractionAlt(void) { ScriptContext_Enable(); return 0; }

// Icefall Cave's cracked ice: the tiles of floor already cracked (var 0x4001's bits), as FireRed
u16 QzSetIcefallCaveCrackedIceMetatiles(void) { return 0; }
u16 QzJohtoTravelingEventCheck(void) { return gSpecialVar_Result = QzJohtoTravelingEventCheck_(); }
u16 QzKantoTravelingEventCheck(void) { return gSpecialVar_Result = QzKantoTravelingEventCheck_(); }
u16 QzPlayerNotAtTrainerHillEntrance(void) { return gSpecialVar_Result = QzPlayerNotAtTrainerHillEntrance_(); }
u16 QzPickNpcFlavourLine(void) { return gSpecialVar_Result = QzPickNpcFlavourLine_(); }
u16 QzBufferLabQuestChecklist(void) { return gSpecialVar_Result = QzBufferLabQuestChecklist_(); }

// ---------------------------------------------------------------------------------------------------- region travel
// The boats between regions (docs/REGIONS_PLAN.md "Travel"): a region's port takes the player to the other regions
// once its Elite Four is beaten and the S.S. TICKET is in the bag. RESULT = bit per region (HOENN 1, JOHTO 2,
// KANTO 4), the ports' menus (MULTI_QZ_0x8E-0x93) list them.
static u16 QzGetReachableRegionsMask_(void)
{
    u8 cur = MP_Region_Current();
    if (!MP_Region_Cleared(cur) || !CheckBagHasItem(ITEM_SS_TICKET, 1))
        return 0;
    return ((1 << REGION_COUNT) - 1) & ~(1 << cur);
}

// The boat's list with SINNOH on it (EventScript_RegionPickAll asks about each region in turn): RESULT 1 if the boat
// goes to region VAR_0x8001 from here, its name in STR_VAR_1
static u16 MP_RegionBoatAsk_(void)
{
    static const u8 sHoenn[] = _("HOENN"), sJohto[] = _("JOHTO"), sKanto[] = _("KANTO"), sSinnoh[] = _("SINNOH");
    static const u8 *const sNames[REGION_COUNT] = {sHoenn, sJohto, sKanto, sSinnoh};
    u16 r = gSpecialVar_0x8001;
    if (r == 0xFF)
        return MP_Region_Current();   // (EventScript_RegionPickAll: which menu)
    if (r == 0xFE)   // (Quetzal_EventScript_840F954: the menus with SINNOH, when it's on the list or the boat leaves it)
        return MP_Region_Current() == REGION_SINNOH || (QzGetReachableRegionsMask_() & (1 << REGION_SINNOH));
    if (r >= REGION_COUNT || !(QzGetReachableRegionsMask_() & (1 << r)))
        return 0;
    StringCopy(gStringVar1, sNames[r]);
    return 1;
}

// Where each region's boat comes in: HOENN at SLATEPORT's harbor, JOHTO at OLIVINE's port, KANTO at VERMILION, SINNOH
// at CANALAVE
static const struct {u16 map; s8 x, y;} sRegionPorts[REGION_COUNT] = {
    {MAP_SLATEPORT_CITY_HARBOR, 8, 11},
    {MAP_JOHTO_OLIVINE_PORT, 8, 16},
    {MAP_KANTO_VERMILION_CITY, 23, 32},
    {MAP_SINNOH_CANALAVE_CITY, 11, 22},   // (CANALAVE's dock, beside the harbor sailor: tools/sinnoh/words.py)
};

// VAR_0x8004 = the region, VAR_0x8005 = the party slot of the one Pokémon who comes (customs: the others go to the
// GLOBAL PC; EventScript_RegionCustoms picked it). RESULT 1: the boat leaves (the script waits), 0: not allowed.
#define TRIP_TRAIN 0x7A   // VAR_0x8007 for the MAGNET TRAIN (EventScript_MagnetTrain*): GOLDENROD <-> SAFFRON, no ticket

static u16 TravelNow(u8 to, u8 keep, bool8 train);

static u16 QzTravelToRegion_(void)
{
    u8 to = gSpecialVar_0x8004, from = MP_Region_Current(), keep = gSpecialVar_0x8005;
    bool8 train = gSpecialVar_0x8007 == TRIP_TRAIN;
    gSpecialVar_0x8007 = 0;
    if (to >= REGION_COUNT || to == from)
        return 0;
    if (train ? !MP_MagnetTrainOpen() : !(QzGetReachableRegionsMask_() & (1 << to)))
        return 0;
    return TravelNow(to, keep, train);
}

// Online, a friend in the room follows the host's trip (mp_main.c): customs keeps their first POKeMON, the rest go to
// the GLOBAL PC; they come in at the region's port (or its first town the first time). FALSE: no room in the GLOBAL PC
bool8 MP_Region_FollowHost(u8 to)
{
    if (to >= REGION_COUNT || to == MP_Region_Current())
        return FALSE;
    return TravelNow(to, 0, FALSE);
}

static u16 TravelNow(u8 to, u8 keep, bool8 train)
{
    u8 from = MP_Region_Current(), i;
    bool8 first;
    if (keep >= PARTY_SIZE || !GetMonData(&gPlayerParty[keep], MON_DATA_SANITY_HAS_SPECIES))
        keep = 0;
    if (MP_Region_GlobalFree() < CalculatePlayerPartyCount() - 1)
        return 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (i == keep || !GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES))
            continue;
        MP_Region_DepositGlobal(&gPlayerParty[i].box);
        ZeroMonData(&gPlayerParty[i]);
    }
    CompactPartySlots();
    first = !(gRegionState.started & (1 << to)) && to != REGION_HOENN;
    MP_Region_Travel(from, to);
    HealPlayerParty();
    if (first && !train)
        MP_Region_FirstBoatIn(to);   // the first visit: in at the port, where a guide takes the player to the first town
    else if (first)
        MP_Region_GoToStartHouse(to);
    else if (train)   // GOLDENROD <-> SAFFRON
    {
        bool8 MP_MagnetTrain_Ride(bool8 toSaffron);
        SetWarpDestinationToHealLocation(to == REGION_KANTO ? HEAL_LOCATION_KANTO_SAFFRON_CITY : HEAL_LOCATION_JOHTO_GOLDENROD_CITY);
        if (MP_MagnetTrain_Ride(to == REGION_KANTO))   // (Crystal's ride, then the warp: src/mp/mp_magnettrain.c)
        {
            ResetInitialPlayerAvatarState();
            return 1;
        }
    }
    else
        SetWarpDestination(sRegionPorts[to].map >> 8, sRegionPorts[to].map & 0xFF, WARP_ID_NONE, sRegionPorts[to].x, sRegionPorts[to].y);
    if (!train)
    {
        // the voyage (FireRed's ferry scene, mp_voyage.c), east for the regions in order HOENN, JOHTO, KANTO, SINNOH
        MP_Voyage_Begin(to > from, FALSE);
        return 1;
    }
    DoWarp();
    ResetInitialPlayerAvatarState();
    return 1;
}

// After the Hall of Fame: the region counts as cleared (its boat, the GLOBAL PC) and the game is
// saved. RESULT 1 = saved.
static u16 QzSaveLeagueVictoryRecord_(void)
{
    MP_Region_SetCleared(MP_Region_Current());
    return TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
}

u16 QzGetReachableRegionsMask(void) { return gSpecialVar_Result = QzGetReachableRegionsMask_(); }

// The boat can sail from here (the region's ELITE FOUR beaten, the S.S. TICKET): CANALAVE's ferry sailor shows
bool8 MP_Region_CanSail(void) { return QzGetReachableRegionsMask_() != 0; }
u16 MP_RegionBoatAsk(void) { return gSpecialVar_Result = MP_RegionBoatAsk_(); }

// EventScript_RegionPickAll: the menu's choice (RESULT) -> the region (src/data/script_menu.h MULTI_REGIONS_FROM_*)
u16 MP_RegionBoatPick(void)
{
    static const u8 sTo[REGION_COUNT][3] = {
        [REGION_HOENN] = {REGION_KANTO, REGION_JOHTO, REGION_SINNOH},
        [REGION_JOHTO] = {REGION_KANTO, REGION_HOENN, REGION_SINNOH},
        [REGION_KANTO] = {REGION_JOHTO, REGION_HOENN, REGION_SINNOH},
        [REGION_SINNOH] = {REGION_HOENN, REGION_JOHTO, REGION_KANTO},
    };
    u16 choice = gSpecialVar_Result;
    u8 cur = MP_Region_Current();
    return gSpecialVar_Result = (choice < 3 && cur < REGION_COUNT) ? sTo[cur][choice] : 0x7F;
}
u16 QzTravelToRegion(void) { return gSpecialVar_Result = QzTravelToRegion_(); }
u16 QzSaveLeagueVictoryRecord(void) { return gSpecialVar_Result = QzSaveLeagueVictoryRecord_(); }

// ---------------------------------------------------------------------------------------------------- starters
// Moemon Kai: the lab's table (the user's design): the region's three starters on Emerald's choosing screen in table
// mode (starter_choose.c MP_StarterTable_Open). The balls are the map's objects at (8, 4), (9, 4), (10, 4): left to
// right as on the screen. Afterwards RESULT = the species (0 for B), VAR_TEMP_1 = the ball (0-2, OAK's script picks
// BLUE's ball from it) and LAST_TALKED = that ball's object, so `removeobject VAR_LAST_TALKED` takes the right one.
#define QZ_TABLE_X 8
#define QZ_TABLE_Y 4
static const u16 sQzTableStarters[][3] = {
    {SPECIES_CYNDAQUIL, SPECIES_TOTODILE, SPECIES_CHIKORITA},   // JOHTO (ELM), as on the original games' desk
    {SPECIES_BULBASAUR, SPECIES_SQUIRTLE, SPECIES_CHARMANDER},  // KANTO (OAK), FireRed's order: BLUE's walk to the
                                                                // ball strong against yours counts on it
};

// BLUE's battles in KANTO as FireRed has them: each in three versions by the starter BLUE took (SQUIRTLE, BULBASAUR,
// CHARMANDER: the one strong against the player's), FireRed's teams (tools/moemon_starters/frlg_rival.py). The scripts
// name the SQUIRTLE one; MP_Region_TrainerData (mp_region.c) reads the version for the player's starter, at every
// GENERATION. A save whose starter isn't one of the three (from before the table) keeps the SQUIRTLE one.
static const u16 sQzBlueBattles[][3] = {
    {TRAINER_QZ_8518, TRAINER_QZ_8519, TRAINER_QZ_8520},   // OAK's lab
    {TRAINER_QZ_8521, TRAINER_QZ_8522, TRAINER_QZ_8523},   // ROUTE 22
    {TRAINER_QZ_8524, TRAINER_QZ_8525, TRAINER_QZ_8526},   // CERULEAN
    {TRAINER_QZ_8618, TRAINER_QZ_8619, TRAINER_QZ_8620},   // S.S. ANNE
    {TRAINER_QZ_8621, TRAINER_QZ_8622, TRAINER_QZ_8623},   // POKéMON TOWER
    {TRAINER_QZ_8624, TRAINER_QZ_8625, TRAINER_QZ_8626},   // SILPH CO.
    {TRAINER_QZ_8627, TRAINER_QZ_8628, TRAINER_QZ_8629},   // ROUTE 22, before the LEAGUE
    {TRAINER_QZ_8630, TRAINER_QZ_8631, TRAINER_QZ_8632},   // the CHAMPION
};

// The version BLUE has (0 SQUIRTLE, 1 BULBASAUR, 2 CHARMANDER) from the player's starter (VAR_QZ_0x5146)
static u8 QzBlueVersion(void)
{
    switch (VarGet(VAR_QZ_0x5146))
    {
    case SPECIES_BULBASAUR:  return 2;
    case SPECIES_SQUIRTLE:   return 1;
    }
    return 0;
}

// The trainer whose team BLUE brings (trainerNum itself if it isn't one of BLUE's battles); *isBlue: it is
u16 QzKantoRivalTrainer(u16 trainerNum, bool8 *isBlue)
{
    u32 i, j;
    for (i = 0; i < ARRAY_COUNT(sQzBlueBattles); i++)
        for (j = 0; j < 3; j++)
            if (sQzBlueBattles[i][j] == trainerNum)
            {
                *isBlue = TRUE;
                return sQzBlueBattles[i][QzBlueVersion()];
            }
    *isBlue = FALSE;
    return trainerNum;
}
static u8 sQzTableRegion;

static void CB2_QzTableDone(void)
{
    u16 ball = gSpecialVar_Result;
    if (ball < 3)
    {
        u8 i;
        const struct ObjectEventTemplate *t = gMapHeader.events->objectEvents;
        gSpecialVar_Result = sQzTableStarters[sQzTableRegion][ball];
        VarSet(VAR_TEMP_1, ball);
        for (i = 0; i < gMapHeader.events->objectEventCount; i++)
            if (t[i].x == QZ_TABLE_X + ball && t[i].y == QZ_TABLE_Y)
                gSpecialVar_LastTalked = t[i].localId;
    }
    else
    {
        gSpecialVar_Result = 0;
    }
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_QzOpenTable(u8 taskId)
{
    void MP_StarterTable_Open(const u16 *species, MainCallback back);
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    MP_StarterTable_Open(sQzTableStarters[sQzTableRegion], CB2_QzTableDone);
}

static u16 QzChooseStarter_(u8 region)
{
    sQzTableRegion = region;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_QzOpenTable, 10);
    return 0;
}

// The player walks by themselves to the table's middle and faces it, when the professor says to choose: to the first
// tile below the middle ball they can stand on (9, 5 in both labs),
// by the shortest way around the room's furniture and people (a search over the map's collision). The script waits
// with `waitmovement LOCALID_PLAYER` (`waitmovement 0` would wait on the last applymovement's object).
#define QZ_WALK_W 24
#define QZ_WALK_H 20
static u8 sQzWalk[48];

static bool8 QzWalkFree(s16 x, s16 y)
{
    if (x < 0 || y < 0 || x >= QZ_WALK_W || y >= QZ_WALK_H || x >= gMapHeader.mapLayout->width || y >= gMapHeader.mapLayout->height)
        return FALSE;
    if (MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET))
        return FALSE;
    return GetObjectEventIdByXY(x + MAP_OFFSET, y + MAP_OFFSET) == OBJECT_EVENTS_COUNT;
}

u16 QzWalkToStarterTable(void)
{
    static const s8 sDx[4] = {0, 0, -1, 1}, sDy[4] = {1, -1, 0, 0};
    static const u8 sStep[4] = {MOVEMENT_ACTION_WALK_NORMAL_DOWN, MOVEMENT_ACTION_WALK_NORMAL_UP,
                                MOVEMENT_ACTION_WALK_NORMAL_LEFT, MOVEMENT_ACTION_WALK_NORMAL_RIGHT};
    static u8 sFrom[QZ_WALK_H][QZ_WALK_W];   // the direction taken to reach a tile + 1, 0: not reached
    static u16 sQueue[QZ_WALK_W * QZ_WALK_H];
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 px = player->currentCoords.x - MAP_OFFSET, py = player->currentCoords.y - MAP_OFFSET;
    s16 tx = QZ_TABLE_X + 1, ty, x, y;
    u16 head = 0, tail = 0;
    u8 n = 0, path[40], len = 0, d;

    for (ty = QZ_TABLE_Y + 1; ty < QZ_TABLE_Y + 4 && !QzWalkFree(tx, ty) && !(tx == px && ty == py); ty++)
        ;
    memset(sFrom, 0, sizeof(sFrom));
    if (px >= 0 && py >= 0 && px < QZ_WALK_W && py < QZ_WALK_H)
    {
        sFrom[py][px] = 5;   // (the start)
        sQueue[tail++] = py * QZ_WALK_W + px;
    }
    while (head < tail && !sFrom[ty][tx])
    {
        u16 at = sQueue[head++];
        for (d = 0; d < 4; d++)
        {
            x = at % QZ_WALK_W + sDx[d], y = at / QZ_WALK_W + sDy[d];
            if (QzWalkFree(x, y) && !sFrom[y][x])
            {
                sFrom[y][x] = d + 1;
                sQueue[tail++] = y * QZ_WALK_W + x;
            }
        }
    }
    if (sFrom[ty][tx])   // back from the table to the start
        for (x = tx, y = ty; sFrom[y][x] != 5 && len < ARRAY_COUNT(path); len++)
        {
            d = sFrom[y][x] - 1;
            path[len] = d;
            x -= sDx[d], y -= sDy[d];
        }
    while (len > 0 && n < ARRAY_COUNT(sQzWalk) - 2)
        sQzWalk[n++] = sStep[path[--len]];
    sQzWalk[n++] = MOVEMENT_ACTION_FACE_UP;
    sQzWalk[n] = MOVEMENT_ACTION_STEP_END;
    printf("[qz] walk to the table: from %d,%d to %d,%d in %d steps\n", px, py, tx, ty, n - 1);
    ScriptMovement_StartObjectMovementScript(LOCALID_PLAYER, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, sQzWalk);
    return 0;
}

// The chosen starter (VAR_TEMP_2) at Lv 5. RESULT: 0 party, 1 PC, 2 no room
static u16 QzGiveStarter_(void)
{
    u16 species = VarGet(VAR_TEMP_2);
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return 2;
    return ScriptGiveMon(species, 5, ITEM_NONE, 0, 0, 0);
}

u16 QzChooseStarterJohto(void) { return QzChooseStarter_(0); }
u16 QzChooseStarterKanto(void) { return QzChooseStarter_(1); }
u16 QzGiveStarterJohto(void)
{
    gSpecialVar_Result = QzGiveStarter_();
    if (gSpecialVar_Result != 2)
        VarSet(VAR_JOHTO_STARTER, VarGet(VAR_TEMP_2));   // (SILVER's line: MP_JohtoRivalSpecies)
    return gSpecialVar_Result;
}

// Moemon Kai: JOHTO's rival's name, as Crystal has the player give it (the officer at ELM's: QzNameJohtoRival); kept
// in JOHTO's vars 0x52FC-0x52FF (8 bytes: 7 letters and EOS); none given: SILVER. {RIVAL} in JOHTO's words and his
// battles' name read it.
#define VAR_JOHTO_RIVAL_NAME 0x52FC
static const u8 sSilverName[] = _("SILVER");

static void RivalNameRead(u8 *out)
{
    u32 i;
    for (i = 0; i < 4; i++)
    {
        u16 v = VarGet(VAR_JOHTO_RIVAL_NAME + i);
        out[2 * i] = v & 0xFF;
        out[2 * i + 1] = v >> 8;
    }
    out[PLAYER_NAME_LENGTH] = EOS;
    if (out[0] == 0 || out[0] == EOS)
        StringCopy(out, sSilverName);
}

static void RivalNameWrite(const u8 *name)
{
    u8 buf[8];
    u32 i;
    memset(buf, EOS, sizeof(buf));
    for (i = 0; i < PLAYER_NAME_LENGTH && name[i] != EOS; i++)
        buf[i] = name[i];
    for (i = 0; i < 4; i++)
        VarSet(VAR_JOHTO_RIVAL_NAME + i, buf[2 * i] | (buf[2 * i + 1] << 8));
}

// The name this player gave (sent to friends in a room this player hosts: mp_main.c MPR_RULES)
const u8 *MP_JohtoRivalOwnName(void)
{
    static u8 sName[PLAYER_NAME_LENGTH + 1];
    RivalNameRead(sName);
    return sName;
}

// The name he goes by: online in a friend's room, the name the host gave (mp_main.c MP_RoomRivalName)
const u8 *MP_JohtoRivalName(void)
{
    const u8 *MP_RoomRivalName(void);
    const u8 *host = MP_RoomRivalName();
    return host != NULL ? host : MP_JohtoRivalOwnName();
}

// {RIVAL} reads it in JOHTO (else NULL: HOENN's MAY / BRENDAN)
const u8 *MP_JohtoRivalNameHere(void)
{
    return MP_Region_Current() == REGION_JOHTO ? MP_JohtoRivalName() : NULL;
}

// A battle's trainer name: SILVER's teams say his name
const u8 *MP_JohtoRivalTrainerName(u16 trainerNum)
{
    if (trainerNum >= FIRST_QZ_TRAINER && trainerNum < FIRST_QZ_TRAINER + QZ_TRAINERS_COUNT
     && StringCompare(gTrainers[trainerNum].trainerName, sSilverName) == 0)
        return MP_JohtoRivalName();
    return gTrainers[trainerNum].trainerName;
}

// The officer at ELM's asks the thief's name: Crystal's naming screen (special NameRival), SILVER to start
static u8 sRivalNameBuf[PLAYER_NAME_LENGTH + 1];
static void CB2_RivalNamed(void)
{
    void MP_SendRoomRules(void);
    if (sRivalNameBuf[0] == EOS)
        StringCopy(sRivalNameBuf, sSilverName);
    RivalNameWrite(sRivalNameBuf);
    MP_SendRoomRules();   // (hosting: friends hear it)
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// In a friend's room the host names him: no naming screen here (the officer says the host's name)
u16 QzNameJohtoRival(void)
{
    bool8 MP_IsRoomGuest(void);
    if (MP_IsRoomGuest())
        return gSpecialVar_Result = FALSE;   // (VAR_RESULT: no naming screen to wait for)
    RivalNameRead(sRivalNameBuf);
    DoNamingScreen(NAMING_SCREEN_RIVAL, sRivalNameBuf, 0, 0, 0, CB2_RivalNamed);
    return gSpecialVar_Result = TRUE;
}

// Moemon Kai: SILVER's starter is the one strong against the player's, as Crystal's (Quetzal gave him TOTODILE's line
// always): his teams' TOTODILE line becomes CHIKORITA's against CYNDAQUIL... (mp_region.c MP_Region_TrainerData).
// A save from before (VAR_JOHTO_STARTER 0) keeps TOTODILE's.
u16 MP_JohtoRivalSpecies(u16 species)
{
    static const u16 sLines[3][3] = {
        {SPECIES_CHIKORITA, SPECIES_BAYLEEF, SPECIES_MEGANIUM},
        {SPECIES_CYNDAQUIL, SPECIES_QUILAVA, SPECIES_TYPHLOSION},
        {SPECIES_TOTODILE, SPECIES_CROCONAW, SPECIES_FERALIGATR},
    };
    u16 player = VarGet(VAR_JOHTO_STARTER);
    u8 line, stage;
    switch (player)
    {
    case SPECIES_CHIKORITA: line = 1; break;   // CYNDAQUIL's line against CHIKORITA
    case SPECIES_CYNDAQUIL: line = 2; break;   // TOTODILE's against CYNDAQUIL
    case SPECIES_TOTODILE:  line = 0; break;   // CHIKORITA's against TOTODILE
    default: return species;
    }
    for (stage = 0; stage < 3; stage++)
        if (species == sLines[2][stage])
            return sLines[line][stage];
    return species;
}
u16 QzGiveStarterKanto(void) { return gSpecialVar_Result = QzGiveStarter_(); }

// ---------------------------------------------------------------------------------------------- credits
// Moemon Kai: Quetzal's credits (Emerald's here). After RED on MT. SILVER (QzAfterCreditsReset) the party is healed and
// the game saved at home before they roll, as the games did; the credits end at the title screen.
u16 QzDoCredits(void)
{
    SetMainCallback2(CB2_StartCreditsSequence);
    return 0;
}

u16 QzAfterCreditsReset(void)
{
    HealPlayerParty();
    SetContinueGameWarpStatus();
    SetContinueGameWarpToHealLocation(MP_Region_Current() == REGION_KANTO ? HEAL_LOCATION_KANTO_PALLET_TOWN
                                                                         : HEAL_LOCATION_JOHTO_NEW_BARK_TOWN);
    TrySavingData(SAVE_NORMAL);
    SetMainCallback2(CB2_StartCreditsSequence);
    return 0;
}

#include "constants/moves.h"
extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

// SEVAULT CANYON's house: MELOETTA joins the CHANSEY Dance when a full team dances along: a CHANSEY leads, holding a
// LUCKY PUNCH and knowing SING, and five others of different families each know a dance (Quetzal's rule)
static u16 FamilyBase(u16 species)
{
    u32 depth, i, j;
    for (depth = 0; depth < 4; depth++)
    {
        u16 from = SPECIES_NONE;
        for (i = 1; i < NUM_SPECIES && from == SPECIES_NONE; i++)
            for (j = 0; j < EVOS_PER_MON; j++)
                if (gEvolutionTable[i][j].targetSpecies == species && gEvolutionTable[i][j].method != 0)
                {
                    from = i;
                    break;
                }
        if (from == SPECIES_NONE)
            break;
        species = from;
    }
    return species;
}

static bool8 KnowsDance(struct Pokemon *mon)
{
    static const u16 sDanceMoves[] = {MOVE_SWORDS_DANCE, MOVE_PETAL_DANCE, MOVE_FEATHER_DANCE, MOVE_TEETER_DANCE,
        MOVE_DRAGON_DANCE, MOVE_LUNAR_DANCE, MOVE_QUIVER_DANCE, MOVE_FIERY_DANCE, MOVE_REVELATION_DANCE,
        MOVE_CLANGOROUS_SOUL, MOVE_VICTORY_DANCE, MOVE_AQUA_STEP};
    u32 i, k;
    for (i = 0; i < MAX_MON_MOVES; i++)
        for (k = 0; k < ARRAY_COUNT(sDanceMoves); k++)
            if (GetMonData(mon, MON_DATA_MOVE1 + i) == sDanceMoves[k])
                return TRUE;
    return FALSE;
}

u16 QzSevaultHouseEventEligibility(void)
{
    u16 base[PARTY_SIZE];
    u32 i, j, k;
    bool8 sing = FALSE;
    gSpecialVar_Result = FALSE;
    if (FlagGet(FLAG_QZ_0x2924) || !MP_Gen_Allowed(SPECIES_MELOETTA) || !MP_Region_Cleared(REGION_KANTO)
     || GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_MELOETTA), FLAG_GET_CAUGHT))
        return 0;
    if (GetMonData(&gPlayerParty[0], MON_DATA_SPECIES_OR_EGG) != SPECIES_CHANSEY
     || GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM) != ITEM_LUCKY_PUNCH)
        return 0;
    for (k = 0; k < MAX_MON_MOVES; k++)
        if (GetMonData(&gPlayerParty[0], MON_DATA_MOVE1 + k) == MOVE_SING)
            sing = TRUE;
    if (!sing)
        return 0;
    for (i = 1; i < PARTY_SIZE; i++)
    {
        u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (sp == SPECIES_NONE || sp == SPECIES_EGG || !KnowsDance(&gPlayerParty[i]))
            return 0;
        base[i] = FamilyBase(sp);
        for (j = 1; j < i; j++)
            if (base[j] == base[i])
                return 0;
    }
    gSpecialVar_Result = TRUE;
    return 0;
}

// PALLET TOWN's bed after the LEAGUE: ARCEUS appears to a full team at Lv 100 (Quetzal's rule; once, FLAG_QZ_0x2E13)
u16 QzPalletPostgameEventEligibility(void)
{
    u32 i;
    gSpecialVar_Result = FALSE;
    if (FlagGet(FLAG_QZ_0x2E13) || !MP_Region_Cleared(REGION_KANTO) || !MP_Gen_Allowed(SPECIES_ARCEUS))
        return 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (sp == SPECIES_NONE || sp == SPECIES_EGG || GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) != MAX_LEVEL)
            return 0;
    }
    gSpecialVar_Result = TRUE;
    return 0;
}

// The DARKRAI dreams. JOHTO: the bed at home, once CRESSELIA is caught: "A shadow smothers your dreams", the world
// as it was (FLAG_QZ_0x1425 on), DARKRAI in DARK CAVE's BLACKTHORN side; beating or catching it wakes you
// (QzEndDreamEvent), and sleeping at home again during the dream wakes you too. A beaten DARKRAI comes back in the
// next dream ("The memory of that nightmare still lingers"). KANTO: the S.S. ANNE's cabin, the same with
// FLAG_QZ_0x292B and DARKRAI in the room.
#define QZ_FLAG_JOHTO_DARKRAI_DONE 0x1239
#define QZ_FLAG_KANTO_DARKRAI_DONE 0x2929
#define QZ_FLAG_KANTO_WOKE         0x292C

static bool8 Caught(u16 species)
{
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

// (DARKRAI is SINNOH's: MP_LegendForeign, so neither dream starts in JOHTO or KANTO any more; one an older save is in
// ends: sleeping again at home, or coming back into the cabin)
static bool8 DreamAllowed(void)
{
    return !MP_LegendForeign(SPECIES_DARKRAI, MP_Region_Current()) && MP_Gen_Allowed(SPECIES_DARKRAI) && Caught(SPECIES_CRESSELIA) && !Caught(SPECIES_DARKRAI)
        && !FlagGet(FLAG_QZ_0x1318)   // (not in GIOVANNI's past)
        && MP_Region_Cleared(MP_Region_Current());   // (DARKRAI waits for the LEAGUE: RefreshStatics)
}

u16 QzEndDreamEvent(void)
{
    FlagClear(FLAG_QZ_0x1425);
    FlagSet(FLAG_QZ_0x1321);
    return 0;
}

u16 QzDreamEventEligibility(void)
{
    gSpecialVar_Result = 0;
    if (FlagGet(FLAG_QZ_0x1425))
    {
        QzEndDreamEvent();   // sleeping during the dream: you wake up
        return 0;
    }
    if (DreamAllowed())
        gSpecialVar_Result = FlagGet(QZ_FLAG_JOHTO_DARKRAI_DONE) ? 2 : 1;
    return 0;
}

u16 QzStartDreamEvent(void)
{
    HealPlayerParty();
    FlagClear(QZ_FLAG_JOHTO_DARKRAI_DONE);   // a DARKRAI beaten in an earlier dream waits again
    FlagSet(FLAG_QZ_0x1425);
    return 0;
}

static void SSAnneWake(void)
{
    FlagClear(FLAG_QZ_0x292B);
    FlagSet(FLAG_QZ_0x2928);
    FlagClear(FLAG_QZ_0x292D);
    FlagSet(QZ_FLAG_KANTO_WOKE);
}

u16 QzSSAnneDreamEventEligibility(void)
{
    gSpecialVar_Result = DreamAllowed() && !FlagGet(FLAG_QZ_0x1425) && MP_Region_Cleared(REGION_KANTO);
    return 0;
}

u16 QzSSAnneDreamEventStart(void)
{
    HealPlayerParty();
    FlagClear(QZ_FLAG_KANTO_WOKE);
    FlagClear(QZ_FLAG_KANTO_DARKRAI_DONE);
    FlagSet(FLAG_QZ_0x292B);
    return 0;
}

u16 QzSSAnneDreamEventEnd(void)
{
    SSAnneWake();
    return 0;
}

// The cabin on entering: the woman is away during the dream; a dream whose DARKRAI is caught or beaten ends
u16 QzSSAnneRoom6OnResume(void)
{
    if (FlagGet(FLAG_QZ_0x292B) && (FlagGet(QZ_FLAG_KANTO_DARKRAI_DONE) || MP_LegendForeign(SPECIES_DARKRAI, REGION_KANTO)))
        SSAnneWake();
    if (FlagGet(FLAG_QZ_0x292B))
        FlagSet(FLAG_QZ_0x292D);
    else
        FlagClear(FLAG_QZ_0x292D);
    RefreshStatics();
    return 0;
}

// The truck by the S.S. ANNE (KANTO, once the ship has sailed): a Lv 100 NIDOKING leading that knows STRENGTH pushes it
// after KANTO's LEAGUE, while MEW isn't caught (Quetzal's rule); MEW waits under it. The truck's metatiles slide left
// two steps (QzSSAnneExteriorEventStep 1, 2) and stay there (FLAG_QZ_0x2931).
#include "field_camera.h"
static bool8 OnSSAnneExterior(void)
{
    return gSaveBlock1Ptr->location.mapGroup == 36 && gSaveBlock1Ptr->location.mapNum == 4;
}

static void ShiftTruck(u8 n)
{
    const struct MapLayout *l = gMapHeader.mapLayout;
    s32 x, y;
    for (y = 1; y <= 3; y++)
    {
        for (x = 53; x <= 57; x++)
            MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, l->map[y * l->width + 52]);
        for (x = 55; x <= 57; x++)
            MapGridSetMetatileIdAt(x - n + MAP_OFFSET, y + MAP_OFFSET, l->map[y * l->width + x]);
    }
}

u16 QzSSAnneExteriorEventCheck(void)
{
    struct Pokemon *lead = &gPlayerParty[0];
    u32 k;
    gSpecialVar_Result = FALSE;
    if (!OnSSAnneExterior() || FlagGet(FLAG_QZ_0x2931) || FlagGet(FLAG_QZ_0x1425) || FlagGet(FLAG_QZ_0x1318)
     || Caught(SPECIES_MEW) || !MP_Region_Cleared(REGION_KANTO)
     || GetMonData(lead, MON_DATA_SPECIES_OR_EGG) != SPECIES_NIDOKING || GetMonData(lead, MON_DATA_LEVEL) != MAX_LEVEL)
        return 0;
    for (k = 0; k < MAX_MON_MOVES; k++)
        if (GetMonData(lead, MON_DATA_MOVE1 + k) == MOVE_STRENGTH)
            gSpecialVar_Result = TRUE;
    return 0;
}

u16 QzSSAnneExteriorEventInit(void)
{
    if (OnSSAnneExterior())
        ShiftTruck(FlagGet(FLAG_QZ_0x2931) ? 2 : 0);
    return 0;
}

u16 QzSSAnneExteriorEventStep(void)
{
    if (OnSSAnneExterior() && gSpecialVar_0x8004 >= 1 && gSpecialVar_0x8004 <= 2)
    {
        ShiftTruck(gSpecialVar_0x8004);
        DrawWholeMapView();
    }
    return 0;
}

// ---------------------------------------------------------------------------------------- GIOVANNI's alternate past
// Quetzal's specials 693-703 and 792 (read from its ROM: Emerald's FlagSet / FlagClear order, 0x80E4788 / 0x80E47D8).
// GIOVANNI at TOHJO FALLS asks a player dressed in Team Rocket's colors, with CELEBI leading their party, to answer his
// men's call at the RADIO TOWER in an alternate past: FLAG_QZ_0x1318 = in that past ("GIOVANNI mode"), 0x1319 = the
// story is over (its end, 698), VAR 0x506C = its step (5 at the start, 1 after the battle beside GIOVANNI, 3 for the
// last battle beside SILVER, 4 when done), 0x5076 = the RADIO TOWER / NEW BARK scene to play, 0x5075 / 0x506D the
// player's look in / before the past (Quetzal's character ids; Kai keeps the player's LOOK as it is: the story asks for
// a Team Rocket LOOK to begin, as Quetzal asks for its Rocket characters).
#define GIOVANNI_PAST FLAG_QZ_0x1318
#define GIOVANNI_DONE 0x1319

static bool8 InGiovanniPast(void) { return FlagGet(GIOVANNI_PAST); }

// Moemon Kai: Quetzal's own addition, not Crystal's or HeartGold / SoulSilver's (the user, 2026-11-02: Quetzal's fan
// additions, GIOVANNI's quest first, aren't wanted): it never begins any more; a save already in it (begun, not over)
// can still see it through
static bool8 GiovanniStoryBegun(void)
{
    return VarGet(VAR_QZ_0x506C) != 0 && !FlagGet(GIOVANNI_DONE);
}

static void QzGiovanniRockets(void)
{
    if (InGiovanniPast())
        FlagClear(FLAG_QZ_0x14A4);
    else
        FlagSet(FLAG_QZ_0x14A4);
}
u16 QzGiovanniStoryItemAvailable(void);

static bool8 OnMap(u8 group, u8 num)
{
    return gSaveBlock1Ptr->location.mapGroup == group && gSaveBlock1Ptr->location.mapNum == num;
}

// Quetzal's 0x82A2DD8 (its Rocket characters): Kai's Team Rocket LOOKs
static bool8 RocketLook(void)
{
    bool8 MP_Avatar_LocalIsTeamRocket(void);
    return MP_Avatar_LocalIsTeamRocket();
}

// Quetzal's 0x82A2E54: CELEBI can take the player through time - not in a link or with friends in the room, not in a
// dream (0x1425), not already in the past, this region's League beaten, CELEBI leading the party (or anywhere in it)
// and its own encounter done (0x11FA: caught or met in ILEX FOREST)
static bool8 CelebiCanTravel(bool8 leadOnly)
{
    u8 i, n = leadOnly ? 1 : PARTY_SIZE;
    if (IsOverworldLinkActive() || FlagGet(FLAG_QZ_0x1425) || InGiovanniPast())
        return FALSE;
    if (MP_IsConnected())
        for (i = 0; i < MP_MAX_PLAYERS; i++)
            if (gMpPlayers[i].present)
                return FALSE;   // (Quetzal: "disconnect from multiplayer" first)
    if (!MP_Region_Cleared(MP_Region_Current()))
        return FALSE;
    for (i = 0; i < n; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) == SPECIES_CELEBI)
            return FlagGet(0x11FA);
    return FALSE;
}

// Quetzal's per-map refresh (693, on every JOHTO map load; mp_region.c MP_Region_OnMapLoaded): who stands where
u16 QzGiovanniStoryUpdateFlags(void)
{
    QzGiovanniRockets();
    if (OnMap(35, 94))   // TOHJO FALLS: GIOVANNI waits only for a save already in his story (GiovanniStoryBegun)
    {
        if (!InGiovanniPast() && GiovanniStoryBegun())
            FlagClear(FLAG_QZ_0x10D4);
        else
            FlagSet(FLAG_QZ_0x10D4);
    }
    if (InGiovanniPast() && !FlagGet(GIOVANNI_DONE))   // the RADIO TOWER's GIOVANNI, in the past
        FlagClear(FLAG_QZ_0x131E);
    else
        FlagSet(FLAG_QZ_0x131E);
    if (OnMap(34, 83))   // RADIO TOWER 5F: the first battle's opponent
    {
        if (InGiovanniPast() && (VarGet(VAR_QZ_0x506C) == 5 || FlagGet(GIOVANNI_DONE)))
            FlagClear(0x14A9);
        else
            FlagSet(0x14A9);
    }
    if (OnMap(35, 116))   // the RUINS OF ALPH's ARCEUS chamber: its item, in the past
    {
        QzGiovanniStoryItemAvailable();
        if (gSpecialVar_Result)
            FlagClear(0x131D);
        else
            FlagSet(0x131D);
    }
    if ((InGiovanniPast() || FlagGet(FLAG_QZ_0x1425)) && (OnMap(34, 96) || OnMap(35, 20)))
        FlagClear(FLAG_QZ_0x1321);   // (the INDIGO PLATEAU / VICTORY ROAD gate people of the past and the dream)
    else
        FlagSet(FLAG_QZ_0x1321);
    FlagSet(FLAG_QZ_0x131F);   // ELM's lab's ARCEUS: its own scene shows it
    {
        // The gym leaders out and about once JOHTO's League is beaten: MORTY in ECRUTEAK, WHITNEY at the DEPT. STORE's
        // roof, PRYCE at the LAKE OF RAGE, JASMINE at OLIVINE's cafe. JOHTO's setup hides them and Quetzal's code that
        // shows them wasn't carried over: no script clears these flags
        static const u16 sLeaders[] = {FLAG_QZ_0x1017, FLAG_QZ_0x1019, FLAG_QZ_0x101C, FLAG_QZ_0x101E};
        u32 i;
        bool8 shown = MP_Region_Cleared(REGION_JOHTO) && !InGiovanniPast();
        for (i = 0; i < ARRAY_COUNT(sLeaders); i++)
        {
            if (shown)
                FlagClear(sLeaders[i]);
            else
                FlagSet(sLeaders[i]);
        }
    }
    return 0;
}

// 697: NEW BARK's house on arriving: GIOVANNI's-past people (0x14A1, 0x14A2) stand there only in the past, the first
// once the RADIO TOWER battle is won (and the house's scene, VAR 0x5076 = 2, plays)
u16 QzGiovanniStoryNewBarkHouseFlags(void)
{
    FlagSet(0x14A1);
    FlagSet(0x14A2);
    if (InGiovanniPast())
    {
        FlagClear(0x14A2);
        if (!FlagGet(GIOVANNI_DONE) && VarGet(VAR_QZ_0x506C) == 1)
        {
            FlagClear(0x14A1);
            if (VarGet(VAR_QZ_0x5076) != 4)
                VarSet(VAR_QZ_0x5076, 2);
        }
    }
    return 0;
}

// 694: GIOVANNI asks the player to come along
u16 QzGiovanniStoryCanStart(void)
{
    gSpecialVar_Result = FALSE;   // (never begins: GiovanniStoryBegun)
    return gSpecialVar_Result;
}

// 792: TOHJO FALLS' radio: CELEBI can go back to the past once the story has begun
u16 QzGiovanniStoryTohjoSignCheck(void)
{
    gSpecialVar_Result = GiovanniStoryBegun() && CelebiCanTravel(FALSE);   // (a save already in the story finishes it)
    return gSpecialVar_Result;
}

// 695: into the past (the script then warps to the RADIO TOWER 5F, or NEW BARK for a later step)
u16 QzGiovanniStoryStart(void)
{
    if (InGiovanniPast())
        return 0;
    VarSet(VAR_QZ_0x5076, 0);
    FlagSet(GIOVANNI_PAST);
    if (FlagGet(GIOVANNI_DONE))
    {
        VarSet(VAR_QZ_0x506C, 4);
        return 0;
    }
    if (VarGet(VAR_QZ_0x506C) == 0)
        VarSet(VAR_QZ_0x506C, 5);
    if (VarGet(VAR_QZ_0x506C) == 5)
        VarSet(VAR_QZ_0x5076, 1);   // the RADIO TOWER's first scene
    return 0;
}

// 699: back to the present (the script warps to TOHJO FALLS or home)
u16 QzGiovanniStoryEnterPast(void)
{
    FlagClear(GIOVANNI_PAST);
    VarSet(VAR_QZ_0x5076, 0);
    return 0;
}

// 698: the story's end (after the battle beside SILVER): done, and back to the present
u16 QzGiovanniStoryEnd(void)
{
    FlagSet(0x131C);
    FlagSet(GIOVANNI_DONE);
    VarSet(VAR_QZ_0x506C, 4);
    QzGiovanniStoryEnterPast();
    VarSet(VAR_QZ_0x5076, 3);
    return 0;
}

// 700: the ARCEUS chamber's item, in the past, for a team of six level 100 legendaries once every other legendary of
// JOHTO and KANTO (at this GENERATION) is caught or beaten (Quetzal's 0x82A2FC0, 0x82A2F6C, 0x82AA84C)
u16 QzGiovanniStoryItemAvailable(void)
{
    u32 i, k;
    gSpecialVar_Result = FALSE;
    if (!InGiovanniPast() || FlagGet(0x131A))
        return 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        bool8 legendary = FALSE;
        if (!species || GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) != MAX_LEVEL)
            return 0;
        for (k = 0; k < ARRAY_COUNT(sQzStatics) && !legendary; k++)
            legendary = sQzStatics[k].species == species;
        if (!legendary)
            return 0;
    }
    for (k = 0; k < ARRAY_COUNT(sQzStatics); k++)
        if (sQzStatics[k].species != SPECIES_ARCEUS && sQzStatics[k].gen <= MP_Gen_Cap(sQzStatics[k].region)
         && !FlagGet(sQzStatics[k].doneFlag))
            return 0;
    gSpecialVar_Result = TRUE;
    return 0;
}

// 701: the item taken
u16 QzGiovanniStoryItemTaken(void)
{
    FlagSet(0x131A);
    FlagSet(0x131D);
    return 0;
}

// 702 / 703: back in the present, ELM's scene about it (once)
u16 QzGiovanniStoryElmCheck(void)
{
    gSpecialVar_Result = FlagGet(0x131A) && !FlagGet(0x131B) && !InGiovanniPast();
    return gSpecialVar_Result;
}

u16 QzGiovanniStoryElmDone(void)
{
    FlagSet(0x131B);
    FlagSet(FLAG_QZ_0x131F);
    return 0;
}

// FireRed's IncrementResortGorgeousStepCounter (field_control_avatar.c, every step): SELPHY's wish lasts 250 steps,
// then she's tired of waiting (0xFFFF) and wishes for another
void MP_ResortGorgeousStep(void)
{
    u16 steps = VarGet(VAR_KANTO_RESORT_STEPS);
    if (VarGet(VAR_QZ_0x5141) == SPECIES_NONE)
        return;
    if (++steps >= 250)
    {
        VarSet(VAR_QZ_0x5141, 0xFFFF);
        steps = 0;
    }
    VarSet(VAR_KANTO_RESORT_STEPS, steps);
}
