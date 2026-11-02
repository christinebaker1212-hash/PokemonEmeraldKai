// Moemon Kai: SINNOH's roaming POKéMON, as Platinum has them (pret/pokeplatinum src/roaming_pokemon.c, overlay006
// wild_encounters.c TryEncounterRoamer, roamer_after_battle.c): six slots (MESPRIT, CRESSELIA, DARKRAI, MOLTRES,
// ZAPDOS, ARTICUNO), each set going by Platinum's own scripts (ActivateRoamingPokemon: VERITY CAVERN, FULLMOON ISLAND,
// ETERNA's house of the birds) with its level and IVs, its HP and status kept between battles. As the player changes
// maps each moves on: one time in 16 anywhere on the mainland's routes, else to a route near its own (never onto the
// map the player just left); flying puts them anywhere. A wild encounter on a roamer's route is it half of the time
// (repel by its level). After its battle: beaten, it's gone (Platinum's state DEFEATED: VAR_ROAMING_*_STATE),
// caught, CAPTURED; otherwise it keeps its HP and status, and every roamer on that route leaves it; a battle with
// anything else sends them off 30% of the time. Emerald's own roamer (LATIOS / LATIAS) stays HOENN's (roamer.c);
// the battle is the game's roamer battle (it flees). Kept in "<save>.region" chunk ROAM (mp_region.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "mp.h"
#include "s3d.h"

#define SLOTS 6
#define ROUTES 29
#define ROAM_MAGIC 0x4D414F52   // "ROAM"

enum { STATE_ROAMING, STATE_CAPTURED, STATE_DEFEATED, STATE_RESET };

static const u16 sRoutes[ROUTES] = {
    MAP_SINNOH_ROUTE201, MAP_SINNOH_ROUTE202, MAP_SINNOH_ROUTE203, MAP_SINNOH_ROUTE204_SOUTH, MAP_SINNOH_ROUTE204_NORTH,
    MAP_SINNOH_ROUTE205_SOUTH, MAP_SINNOH_ROUTE205_NORTH, MAP_SINNOH_ROUTE206, MAP_SINNOH_ROUTE207, MAP_SINNOH_ROUTE208,
    MAP_SINNOH_ROUTE209, MAP_SINNOH_ROUTE210_SOUTH, MAP_SINNOH_ROUTE210_NORTH, MAP_SINNOH_ROUTE211_WEST,
    MAP_SINNOH_ROUTE211_EAST, MAP_SINNOH_ROUTE212_NORTH, MAP_SINNOH_ROUTE212_SOUTH, MAP_SINNOH_ROUTE213,
    MAP_SINNOH_ROUTE214, MAP_SINNOH_ROUTE215, MAP_SINNOH_ROUTE216, MAP_SINNOH_ROUTE217, MAP_SINNOH_ROUTE218,
    MAP_SINNOH_ROUTE219, MAP_SINNOH_ROUTE220, MAP_SINNOH_ROUTE221, MAP_SINNOH_ROUTE222,
    MAP_SINNOH_VALLEY_WINDWORKS_OUTSIDE, MAP_SINNOH_FUEGO_IRONWORKS_OUTSIDE,
};

// (Platinum's sNearbyRoutes: the routes next to each and those by the towns next to it; its own 9 for ROUTE 205 north
// kept)
static const u8 sNearby[ROUTES][6] = {
    {2, 1, 23}, {5, 0, 2, 3, 22, 23}, {4, 1, 3, 8, 22}, {4, 1, 2, 4, 22}, {2, 3, 5}, {4, 4, 6, 27, 28}, {3, 5, 7, 9},
    {3, 6, 8, 13}, {3, 2, 7, 9}, {3, 8, 10, 15}, {3, 9, 11, 15}, {3, 10, 12, 19}, {2, 11, 14}, {4, 6, 7, 14, 20},
    {3, 12, 13, 20}, {3, 9, 10, 16}, {2, 15, 17}, {3, 16, 18, 26}, {3, 17, 19, 26}, {2, 11, 18}, {3, 13, 14, 21},
    {1, 20}, {3, 1, 2, 3}, {3, 0, 1, 24}, {2, 23, 25}, {1, 24}, {2, 17, 18}, {1, 5}, {1, 5},
};

static const struct { u16 species; u8 level; } sKinds[SLOTS] = {
    {SPECIES_MESPRIT, 50}, {SPECIES_CRESSELIA, 50}, {SPECIES_DARKRAI, 40},
    {SPECIES_MOLTRES, 60}, {SPECIES_ZAPDOS, 60}, {SPECIES_ARTICUNO, 60},
};

static struct
{
    u32 magic;
    struct { u8 active, route, level, status; u16 species, hp; u32 ivs, personality; } r[SLOTS];
    u16 previousMap;    // (the map the player left last: no roamer moves there)
    u16 lastMap;
} sR;

static s8 sBattled = -1;   // (the slot of the roamer in the battle now)

extern const u16 gSinnohRoamerStateVars[];   // (Platinum's VAR_ROAMING_*_STATE, this game's: contest_data.py)
void Sinnoh_LegendDone(u16 species, bool8 done);   // (s3d_legends.c: MESPRIT's and CRESSELIA's objects)

const void *Sinnoh_RoamerData(u32 *size)
{
    sR.magic = ROAM_MAGIC;
    *size = sizeof(sR);
    return &sR;
}

void Sinnoh_RoamerLoad(const void *data, u32 size)
{
    memset(&sR, 0, sizeof(sR));
    if (size == sizeof(sR) && ((const u32 *)data)[0] == ROAM_MAGIC)
        memcpy(&sR, data, size);
}

void Sinnoh_RoamerReset(void)
{
    memset(&sR, 0, sizeof(sR));
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static void SetState(u8 slot, u16 state)
{
    u16 var = gSinnohRoamerStateVars[slot];
    if (var >= VARS_START)
        VarSet(var, state);
}

static void MoveRandom(u8 slot)
{
    u16 cur = sRoutes[sR.r[slot].route % ROUTES], tries;
    for (tries = 0; tries < 200; tries++)
    {
        u8 i = Random() % ROUTES;
        if (sRoutes[i] != sR.previousMap && sRoutes[i] != cur)
        {
            sR.r[slot].route = i;
            return;
        }
    }
}

static void MoveNearby(u8 slot)
{
    const u8 *n = sNearby[sR.r[slot].route % ROUTES];
    u16 tries;
    if (n[0] == 1)
    {
        if (sRoutes[n[1]] == sR.previousMap)
            MoveRandom(slot);
        else
            sR.r[slot].route = n[1];
        return;
    }
    for (tries = 0; tries < 200; tries++)
    {
        u8 i = n[1 + Random() % n[0]];
        if (sRoutes[i] != sR.previousMap)
        {
            sR.r[slot].route = i;
            return;
        }
    }
}

// (ActivateRoamingPokemon) a slot set going: its POKéMON made fresh, somewhere on the routes
void Sinnoh_RoamerActivate(u8 slot)
{
    struct Pokemon mon;
    if (slot >= SLOTS)
        return;
    CreateMon(&mon, sKinds[slot].species, sKinds[slot].level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    sR.r[slot].species = sKinds[slot].species;
    sR.r[slot].level = sKinds[slot].level;
    sR.r[slot].ivs = GetMonData(&mon, MON_DATA_IVS);
    sR.r[slot].personality = GetMonData(&mon, MON_DATA_PERSONALITY);
    sR.r[slot].hp = GetMonData(&mon, MON_DATA_MAX_HP);
    sR.r[slot].status = 0;
    sR.r[slot].active = TRUE;
    MoveRandom(slot);
    SetState(slot, STATE_ROAMING);
    Sinnoh_LegendDone(sKinds[slot].species, TRUE);   // (its object in the cave / the forest: gone while it roams)
    MPNet_Log("sinnoh roamer: %u set going on route %u", sKinds[slot].species, sR.r[slot].route);
}

// a new map (overworld.c's map change): every roamer moves on (Platinum's RoamingPokemon_MoveAllLocations)
void Sinnoh_RoamerMapChanged(void)
{
    u16 map = CurrentMap();
    u8 i;
    if (map == sR.lastMap || !S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup))
        return;
    sR.previousMap = sR.lastMap;
    sR.lastMap = map;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active)
        {
            if (Random() % 16 == 0)
                MoveRandom(i);
            else
                MoveNearby(i);
        }
}

// flying: anywhere (Platinum's RoamingPokemon_RandomizeAllLocations)
void Sinnoh_RoamerFlew(void)
{
    u8 i;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active)
            MoveRandom(i);
}

// (wild_encounter.c / roamer.c TryStartRoamerEncounter in SINNOH) half of the time on a roamer's route: it, in
// gEnemyParty[0]; *level its level (repel)
bool8 Sinnoh_TryRoamerEncounter(u8 *level)
{
    u8 here[SLOTS], n = 0, i, s;
    u16 map = CurrentMap();
    struct Pokemon *mon = &gEnemyParty[0];
    u32 status;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active && sRoutes[sR.r[i].route % ROUTES] == map)
            here[n++] = i;
    if (n == 0 || Random() % 2 == 0)
        return FALSE;
    s = here[n > 1 ? Random() % n : 0];
    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, sR.r[s].species, sR.r[s].level, sR.r[s].ivs, sR.r[s].personality);
    status = sR.r[s].status;
    SetMonData(mon, MON_DATA_STATUS, &status);
    SetMonData(mon, MON_DATA_HP, &sR.r[s].hp);
    sBattled = s;
    *level = sR.r[s].level;
    MPNet_Log("sinnoh roamer: %u met (hp %u)", sR.r[s].species, sR.r[s].hp);
    return TRUE;
}

static void OffThisMap(void)
{
    u16 map = CurrentMap();
    u8 i;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active && sRoutes[sR.r[i].route % ROUTES] == map)
            MoveRandom(i);
}

// (battle_main.c, a roamer battle's end) TRUE: it was SINNOH's
bool8 Sinnoh_RoamerAfterBattle(struct Pokemon *mon, u8 outcome)
{
    s8 s = sBattled;
    sBattled = -1;
    if (s < 0)
        return FALSE;
    if (outcome == B_OUTCOME_CAUGHT)
    {
        sR.r[s].active = FALSE;
        SetState(s, STATE_CAPTURED);
    }
    else if (outcome == B_OUTCOME_WON && GetMonData(mon, MON_DATA_HP) == 0)
    {
        sR.r[s].active = FALSE;
        SetState(s, STATE_DEFEATED);
    }
    else
    {
        sR.r[s].hp = GetMonData(mon, MON_DATA_HP);
        sR.r[s].status = GetMonData(mon, MON_DATA_STATUS);
    }
    MPNet_Log("sinnoh roamer: battle over (outcome %u, %s)", outcome, sR.r[s].active ? "roaming on" : "gone");
    OffThisMap();
    return TRUE;
}

// (battle_setup.c, any other wild battle's end in SINNOH) the roamers there leave 30% of the time
void Sinnoh_RoamerOtherBattle(void)
{
    if (S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) && Random() % 100 < 30)
        OffThisMap();
}

// (GameClear in SINNOH: Platinum's PokemonLeagueHallOfFame_ResetRoaming*) a beaten roamer can be met again:
// MESPRIT and CRESSELIA back in their places, the birds from PROF. OAK in ETERNA (their state RESET)
void Sinnoh_RoamerHallOfFame(void)
{
    u8 i;
    for (i = 0; i < SLOTS; i++)
    {
        u16 var = gSinnohRoamerStateVars[i];
        if (var >= VARS_START && VarGet(var) == STATE_DEFEATED)
        {
            VarSet(var, STATE_RESET);
            Sinnoh_LegendDone(sKinds[i].species, FALSE);
        }
    }
}

// (the TV's SINNOH NOW: Platinum's ROAMER NEWS FLASH) one of the roamers and where it is now: FALSE, none roams
bool8 Sinnoh_RoamerNews(u16 *species, u16 *mapsec)
{
    u8 here[SLOTS], n = 0, i, s;
    u16 map;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active)
            here[n++] = i;
    if (n == 0)
        return FALSE;
    s = here[Random() % n];
    map = sRoutes[sR.r[s].route % ROUTES];
    *species = sR.r[s].species;
    *mapsec = Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF)->regionMapSectionId;
    return TRUE;
}

bool8 Sinnoh_RoamerAt(u16 map, u16 *species)
{
    u8 i;
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active && sRoutes[sR.r[i].route % ROUTES] == map)
        {
            *species = sR.r[i].species;
            return TRUE;
        }
    return FALSE;
}

void Sinnoh_RoamerDebug(void)
{
    u8 i;
    printf("[test] roamers:");
    for (i = 0; i < SLOTS; i++)
        if (sR.r[i].active)
            printf(" %u@%d.%d(hp %u)", sR.r[i].species, sRoutes[sR.r[i].route % ROUTES] >> 8,
                   sRoutes[sR.r[i].route % ROUTES] & 0xFF, sR.r[i].hp);
    printf("\n");
}

// (tests) the roamer of SLOT to the map the player is on
void Sinnoh_RoamerHere(u8 slot)
{
    u8 i;
    for (i = 0; i < ROUTES; i++)
        if (slot < SLOTS && sRoutes[i] == CurrentMap())
            sR.r[slot].route = i;
}
