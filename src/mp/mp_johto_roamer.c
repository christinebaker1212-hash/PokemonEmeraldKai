// Moemon Kai: JOHTO's roaming beasts, as Crystal has them (pret/pokecrystal engine/overworld/wildmons.asm InitRoamMons,
// CheckEncounterRoamMon, UpdateRoamMons / JumpRoamMon; data/wild/roammon_maps.asm). The BURNED TOWER's scene sets them
// going (Crystal's EVENT_RELEASED_THE_BEASTS + special InitRoamMons; this port's script had Emerald's InitRoamer, a
// LATIOS loose in HOENN): RAIKOU from ROUTE 42, ENTEI from ROUTE 37, Lv 40. As the player changes maps on JOHTO's
// ground each moves on, one time in 32 anywhere on Crystal's sixteen roaming routes, else to one of its route's
// neighbours (never the map the player just left); flying sends them anywhere. A wild encounter (not on water) on a
// beast's route is it a little under one time in ten each (Crystal's 100 / 256, then one of three, a third of which is
// the old third roamer slot that Crystal leaves empty). The battle is the game's roamer battle (it flees); beaten or
// caught it's gone for good, else it keeps its HP and status. Kept in "<save>.region" chunk JRMS (mp_region.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle.h"
#include "constants/map_groups.h"
#include "constants/species.h"
#include "constants/qz_port.h"
#include "mp.h"
#include "mp_region.h"

#define SLOTS 2
#define ROUTES 16
#define JRMS_MAGIC 0x534D524A   // "JRMS"

static const u16 sRoutes[ROUTES] = {
    MAP_JOHTO_ROUTE29, MAP_JOHTO_ROUTE30, MAP_JOHTO_ROUTE31, MAP_JOHTO_ROUTE32, MAP_JOHTO_ROUTE33, MAP_JOHTO_ROUTE34,
    MAP_JOHTO_ROUTE35, MAP_JOHTO_ROUTE36, MAP_JOHTO_ROUTE37, MAP_JOHTO_ROUTE38, MAP_JOHTO_ROUTE39, MAP_JOHTO_ROUTE42,
    MAP_JOHTO_ROUTE43, MAP_JOHTO_ROUTE44, MAP_JOHTO_ROUTE45, MAP_JOHTO_ROUTE46,
};
enum { R29, R30, R31, R32, R33, R34, R35, R36, R37, R38, R39, R42, R43, R44, R45, R46 };

// (RoamMaps: each route's neighbours, count first)
static const u8 sNext[ROUTES][5] = {
    [R29] = {2, R30, R46},            [R30] = {2, R29, R31},           [R31] = {3, R30, R32, R36},
    [R32] = {3, R36, R31, R33},       [R33] = {2, R32, R34},           [R34] = {2, R33, R35},
    [R35] = {2, R34, R36},            [R36] = {4, R35, R31, R32, R37}, [R37] = {3, R36, R38, R42},
    [R38] = {3, R37, R39, R42},       [R39] = {1, R38},                [R42] = {4, R43, R44, R37, R38},
    [R43] = {2, R42, R44},            [R44] = {3, R42, R43, R45},      [R45] = {2, R44, R46},
    [R46] = {2, R45, R29},
};

static const struct { u16 species; u8 level, start; } sKinds[SLOTS] = {
    {SPECIES_RAIKOU, 40, R42}, {SPECIES_ENTEI, 40, R37},
};

static struct
{
    u32 magic;
    struct { u8 active, route, level, status; u16 species, hp; u32 ivs, personality; } r[SLOTS];
    u16 lastMap, previousMap;
    u8 inited;
} sJ;

static s8 sBattled = -1;

const void *MP_JohtoRoamer_Data(u32 *size)
{
    sJ.magic = JRMS_MAGIC;
    *size = sizeof(sJ);
    return &sJ;
}

void MP_JohtoRoamer_Load(const void *data, u32 size)
{
    memset(&sJ, 0, sizeof(sJ));
    if (size == sizeof(sJ) && ((const u32 *)data)[0] == JRMS_MAGIC)
        memcpy(&sJ, data, size);
}

void MP_JohtoRoamer_Reset(void)
{
    memset(&sJ, 0, sizeof(sJ));
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static bool8 InJohto(void)
{
    return MP_Region_OfGroup(gSaveBlock1Ptr->location.mapGroup) == REGION_JOHTO;
}

// (JumpRoamMon) anywhere on the routes but the map the player is on
static void Jump(u8 s)
{
    u16 tries;
    for (tries = 0; tries < 200; tries++)
    {
        u8 i = Random() % ROUTES;
        if (sRoutes[i] != CurrentMap() && i != sJ.r[s].route)
        {
            sJ.r[s].route = i;
            return;
        }
    }
}

// (UpdateRoamMons' .Update) one time in 32 anywhere, else a neighbour that isn't the map the player just left
static void Move(u8 s)
{
    const u8 *n = sNext[sJ.r[s].route % ROUTES];
    u16 tries;
    if ((Random() & 31) == 0)
    {
        Jump(s);
        return;
    }
    for (tries = 0; tries < 200; tries++)
    {
        u8 i = n[1 + Random() % n[0]];
        if (sRoutes[i] != sJ.previousMap)
        {
            sJ.r[s].route = i;
            return;
        }
    }
}

// (special InitRoamMons, the BURNED TOWER's scene) both beasts set going, made fresh
void MP_JohtoRoamer_Init(void)
{
    u8 s;
    for (s = 0; s < SLOTS; s++)
    {
        struct Pokemon mon;
        CreateMon(&mon, sKinds[s].species, sKinds[s].level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        sJ.r[s].species = sKinds[s].species;
        sJ.r[s].level = sKinds[s].level;
        sJ.r[s].ivs = GetMonData(&mon, MON_DATA_IVS);
        sJ.r[s].personality = GetMonData(&mon, MON_DATA_PERSONALITY);
        sJ.r[s].hp = GetMonData(&mon, MON_DATA_MAX_HP);
        sJ.r[s].status = 0;
        sJ.r[s].route = sKinds[s].start;
        sJ.r[s].active = TRUE;
    }
    sJ.inited = TRUE;
    MPNet_Log("johto roamers: RAIKOU and ENTEI set going");
}

// (mp_region.c, a save loaded) the BURNED TOWER's scene played before 1.88.0 (FLAG_QZ_0x10AE, set just before its
// InitRoamer): the beasts set going now
void MP_JohtoRoamer_CatchUp(void)
{
    if (!sJ.inited && FlagGet(FLAG_QZ_0x10AE))
        MP_JohtoRoamer_Init();
}

// (overworld.c, a new map) on JOHTO's ground they move on
void MP_JohtoRoamer_MapChanged(void)
{
    u16 map = CurrentMap();
    u8 s;
    if (map == sJ.lastMap || !InJohto())
        return;
    sJ.previousMap = sJ.lastMap;
    sJ.lastMap = map;
    for (s = 0; s < SLOTS; s++)
        if (sJ.r[s].active)
            Move(s);
}

// flying in JOHTO: anywhere
void MP_JohtoRoamer_Flew(void)
{
    u8 s;
    if (!InJohto())
        return;
    for (s = 0; s < SLOTS; s++)
        if (sJ.r[s].active)
            Jump(s);
}

// (roamer.c TryStartRoamerEncounter in JOHTO: CheckEncounterRoamMon) it, in gEnemyParty[0]; *level its level
bool8 MP_JohtoRoamer_TryEncounter(u8 *level)
{
    struct Pokemon *mon = &gEnemyParty[0];
    u8 r = Random(), s;
    u32 status;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) || r >= 100 || (r & 3) == 0)
        return FALSE;
    s = (r & 3) - 1;   // (2: Crystal's empty third slot)
    if (s >= SLOTS || !sJ.r[s].active || sRoutes[sJ.r[s].route % ROUTES] != CurrentMap())
        return FALSE;
    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, sJ.r[s].species, sJ.r[s].level, sJ.r[s].ivs, sJ.r[s].personality);
    status = sJ.r[s].status;
    SetMonData(mon, MON_DATA_STATUS, &status);
    SetMonData(mon, MON_DATA_HP, &sJ.r[s].hp);
    sBattled = s;
    *level = sJ.r[s].level;
    MPNet_Log("johto roamer: %u met (hp %u)", sJ.r[s].species, sJ.r[s].hp);
    return TRUE;
}

// (battle_main.c, a roamer battle's end) TRUE: it was JOHTO's
bool8 MP_JohtoRoamer_AfterBattle(struct Pokemon *mon, u8 outcome)
{
    s8 s = sBattled;
    sBattled = -1;
    if (s < 0)
        return FALSE;
    if (outcome == B_OUTCOME_CAUGHT || (outcome == B_OUTCOME_WON && GetMonData(mon, MON_DATA_HP) == 0))
    {
        sJ.r[s].active = FALSE;
    }
    else
    {
        sJ.r[s].hp = GetMonData(mon, MON_DATA_HP);
        sJ.r[s].status = GetMonData(mon, MON_DATA_STATUS);
        Jump(s);   // (it fled)
    }
    MPNet_Log("johto roamer: battle over (outcome %u, %s)", outcome, sJ.r[s].active ? "roaming on" : "gone");
    return TRUE;
}

bool8 MP_JohtoRoamer_At(u16 map, u16 *species)
{
    u8 s;
    for (s = 0; s < SLOTS; s++)
        if (sJ.r[s].active && sRoutes[sJ.r[s].route % ROUTES] == map)
        {
            *species = sJ.r[s].species;
            return TRUE;
        }
    return FALSE;
}

void MP_JohtoRoamer_Debug(void)
{
    u8 s;
    printf("[test] johto roamers:");
    for (s = 0; s < SLOTS; s++)
        if (sJ.r[s].active)
            printf(" %u@%d.%d(hp %u)", sJ.r[s].species, sRoutes[sJ.r[s].route % ROUTES] >> 8,
                   sRoutes[sJ.r[s].route % ROUTES] & 0xFF, sJ.r[s].hp);
    printf("\n");
}

// (tests) SLOT's beast to the map the player is on
void MP_JohtoRoamer_Here(u8 slot)
{
    u8 i;
    for (i = 0; i < ROUTES; i++)
        if (slot < SLOTS && sRoutes[i] == CurrentMap())
            sJ.r[slot].route = i;
}
