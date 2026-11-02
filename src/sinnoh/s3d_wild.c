// Moemon Kai: SINNOH's wild POKéMON as Platinum changes them (src/data/sinnoh_wild_extra.h, tools/sinnoh/gen_wild.py):
// - the time of day: grass slots 2 and 3 hold the table's POKéMON in the morning (4:00-9:59), others by day
//   (10:00-19:59) and by night (20:00-3:59)
// - swarms: once SINNOH's LEAGUE is beaten, the twin in SANDGEM's house by the LAB (data/scripts/sinnoh_wild.inc)
//   tells of the day's swarm: one of 22 routes, its grass slots 0 and 1 its swarm's POKéMON all day
// - HONEY trees: 21 trees, one a place (gen_maps.py honey_tree). HONEY slathered on one brings a POKéMON in 6 to 24
//   hours: group A (common), B (uncommon) or C (MUNCHLAX, on four trees picked by the Trainer ID), a slot in it at
//   40/20/20/10/5/5 percent; the same tree again keeps its group 9 times in 10. The trees count on the player's own
//   clock (as BERRY trees: MP_ClockShift) and are kept in "<save>.region" (chunk HONY, mp_region.c)
// - the TROPHY GARDEN: MR. BACKLOT's daily POKéMON (after the LEAGUE) in its grass slots 6-7, kept with the trees
#include "global.h"
#include "event_data.h"
#include "mp_region.h"
#include "mp.h"
#include "constants/species.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "fieldmap.h"
#include "overworld.h"
#include "region_map.h"
#include "rtc.h"
#include "random.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "battle.h"
#include "constants/battle.h"
#include "constants/game_stat.h"
#include "../data/sinnoh_wild_extra.h"

#define FLAG_SINNOH_SWARMS 0x3D30   // the twin told of swarms (tools/sinnoh: free between the story's and FLY's flags)
#define HONEY_TREES        ARRAY_COUNT(sHoneyMaps)

u16 Sinnoh_GreatMarshDaily(u16 species, u8 index);
bool8 Sinnoh_GreatMarshWildMap(u8 *group, u8 *num);

// The HONEY trees and the TROPHY GARDEN, kept in "<save>.region"
struct HoneyTree
{
    u32 stamp;   // the player's own clock in minutes when the HONEY went on (0: bare)
    u8 group;    // 0 nothing comes, 1 A, 2 B, 3 C (MUNCHLAX)
    u8 slot;
    u8 pad[2];
};
static struct
{
    struct HoneyTree trees[21];
    u8 last;     // the tree slathered last (+1; 0 none)
    u8 pad[3];
    // (then the TROPHY GARDEN: MR. BACKLOT's last two POKéMON, sTrophyMons + 1 (0 none), newest first, and the day he
    // last told of one; a shorter chunk from before leaves them 0)
    u8 trophy[2];
    u8 pad2[2];
    u32 trophyDay;
} sHoney;
STATIC_ASSERT(ARRAY_COUNT(sHoney.trees) == ARRAY_COUNT(sHoneyMaps), honeyTreeCount);

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static u32 TrainerId(void)
{
    return gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8
         | gSaveBlock2Ptr->playerTrainerId[2] << 16 | (u32)gSaveBlock2Ptr->playerTrainerId[3] << 24;
}

// ------------------------------------------------------------------------------------------------------------ swarms
static bool8 SwarmsOn(void)
{
    return FlagGet(FLAG_SINNOH_SWARMS) && MP_Region_Cleared(REGION_SINNOH);
}

static const struct SinnohTimedSlots *Timed(u16 map)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sTimed); i++)
        if (sTimed[i].map == map)
            return &sTimed[i];
    return NULL;
}

// Today's swarm route (its place in sSwarmMaps): the day and the Trainer ID, only routes whose swarm is allowed by the
// GENERATION setting (0xFF: none)
static u8 sDebugSwarm = 0xFF;   // (tests: `mp swarm N`)

void Sinnoh_DebugSwarm(u8 r)
{
    sDebugSwarm = r;
}

static u8 SwarmToday(void)
{
    u32 seed = RtcGetLocalDayCount() * 0x9E3779B1u ^ TrainerId() ^ 0x5A4D;
    u32 i, n = ARRAY_COUNT(sSwarmMaps);
    seed ^= seed >> 16;
    seed *= 0x85EBCA6Bu;
    seed ^= seed >> 13;
    if (sDebugSwarm < n)
        seed = sDebugSwarm;
    for (i = 0; i < n; i++)
    {
        u8 r = (seed + i) % n;
        const struct SinnohTimedSlots *t = Timed(sSwarmMaps[r]);
        if (t != NULL && MP_Gen_Allowed(t->swarm[0]))
            return r;
    }
    return 0xFF;
}

// The twin's news (data/scripts/sinnoh_wild.inc): turns swarms on; gStringVar1 the route, gStringVar2 the POKéMON
// (VAR_RESULT FALSE: no swarm the GENERATION setting allows)
u16 Sinnoh_SwarmNews(void)
{
    u8 r;
    const struct MapHeader *h;
    FlagSet(FLAG_SINNOH_SWARMS);
    r = SwarmToday();
    if (r == 0xFF)
        return gSpecialVar_Result = FALSE;
    h = Overworld_GetMapHeaderByGroupAndId(sSwarmMaps[r] >> 8, sSwarmMaps[r] & 0xFF);
    GetMapName(gStringVar1, h->regionMapSectionId, 0);
    GetSpeciesName(gStringVar2, Timed(sSwarmMaps[r])->swarm[0]);
    MPNet_Log("sinnoh: swarm news: species %d on map %d.%d", Timed(sSwarmMaps[r])->swarm[0], sSwarmMaps[r] >> 8,
              sSwarmMaps[r] & 0xFF);
    return gSpecialVar_Result = TRUE;
}

// Today's swarm, once the twin has told of them (Platinum's SpecialEncounter_IsSwarmEnabled): its place and POKéMON
// (the TV's SWARM NEWS FLASH: s3d_tv.c); FALSE: none
bool8 Sinnoh_SwarmToday(u16 *mapsec, u16 *species)
{
    u8 r;
    if (!FlagGet(FLAG_SINNOH_SWARMS) || (r = SwarmToday()) == 0xFF)
        return FALSE;
    *mapsec = Overworld_GetMapHeaderByGroupAndId(sSwarmMaps[r] >> 8, sSwarmMaps[r] & 0xFF)->regionMapSectionId;
    *species = Timed(sSwarmMaps[r])->swarm[0];
    return TRUE;
}

// ---------------------------------------------------------------------------------------------- the grass's slots
// Grass slot INDEX's POKéMON (SPECIES: the table's) where the player is: the GREAT MARSH's daily ones, the swarm, the
// time of day. A slot already changed from Platinum's table (the GENERATION's own POKéMON) stays as it is.
u16 Sinnoh_LandSlot(u16 species, u8 index)
{
    const struct SinnohTimedSlots *t;
    u8 group, num;
    u16 map, pick = SPECIES_NONE;
    species = Sinnoh_GreatMarshDaily(species, index);
    if (MP_Region_Current() != REGION_SINNOH)
        return species;
    if (index == 6 || index == 7)
    {
        u8 n = sHoney.trophy[index - 6];
        if (CurrentMap() == MAP_SINNOH_TROPHY_GARDEN && MP_Region_Cleared(REGION_SINNOH) && species == sTrophyBase[index - 6]
         && n != 0 && n <= ARRAY_COUNT(sTrophyMons) && MP_Gen_Allowed(sTrophyMons[n - 1]))
            return sTrophyMons[n - 1];
        return species;
    }
    if (index > 3)
        return species;
    map = Sinnoh_GreatMarshWildMap(&group, &num) ? (group << 8 | num) : CurrentMap();
    if ((t = Timed(map)) == NULL || species != t->base[index])
        return species;
    if (index < 2)
    {
        u8 r;
        if (SwarmsOn() && (r = SwarmToday()) != 0xFF && sSwarmMaps[r] == map)
            pick = t->swarm[index];
    }
    else
    {
        RtcCalcLocalTime();
        if (gLocalTime.hours >= 10 && gLocalTime.hours < 20)
            pick = t->day[index - 2];
        else if (gLocalTime.hours >= 20 || gLocalTime.hours < 4)
            pick = t->night[index - 2];
    }
    return pick != SPECIES_NONE && MP_Gen_Allowed(pick) ? pick : species;
}

// ------------------------------------------------------------------------------------------------------- HONEY trees

void Sinnoh_HoneyReset(void)
{
    memset(&sHoney, 0, sizeof(sHoney));
}

const void *Sinnoh_HoneyData(u32 *size)
{
    *size = sizeof(sHoney);
    return &sHoney;
}

void Sinnoh_HoneyLoad(const void *data, u32 size)
{
    Sinnoh_HoneyReset();
    memcpy(&sHoney, data, size < sizeof(sHoney) ? size : sizeof(sHoney));
}

static u32 OwnMinutes(void)
{
    s64 m;
    RtcCalcLocalTime();
    m = (s64)gLocalTime.days * 1440 + gLocalTime.hours * 60 + gLocalTime.minutes - MP_ClockShift() / 60;
    return m < 1 ? 1 : (u32)m;
}

static u8 TreeHere(void)
{
    u32 i;
    for (i = 0; i < HONEY_TREES; i++)
        if (sHoneyMaps[i] == CurrentMap())
            return i;
    return 0xFF;
}

// Platinum's four MUNCHLAX trees: the Trainer ID's four bytes, each a tree, none twice
static bool8 MunchlaxTree(u8 tree)
{
    u32 id = TrainerId();
    u8 t[4], i, j;
    for (i = 0; i < 4; i++)
        t[i] = ((id >> (24 - 8 * i)) & 0xFF) % HONEY_TREES;
    for (i = 1; i < 4; i++)
        for (j = 0; j < i; j++)
            if (t[j] == t[i] && ++t[i] >= HONEY_TREES)
                t[i] = 0;
    for (i = 0; i < 4; i++)
        if (t[i] == tree)
            return TRUE;
    return FALSE;
}

static u8 RollSlot(void)
{
    u32 r = Random() % 100;
    return r < 5 ? 5 : r < 10 ? 4 : r < 20 ? 3 : r < 40 ? 2 : r < 60 ? 1 : 0;
}

// VAR_RESULT: 0 bare, 1 HONEY on it (too soon), 2 a POKéMON came, 3 the HONEY's gone and nothing came, 0xFF no tree
static u16 HoneyTreeState(u8 tree);

u16 Sinnoh_HoneyTreeState(void)
{
    u8 tree = TreeHere();
    u16 state = HoneyTreeState(tree);
    MPNet_Log("sinnoh: honey tree %d: state %d", tree, state);
    return gSpecialVar_Result = state;
}

static u16 HoneyTreeState(u8 tree)
{
    u32 age, now = OwnMinutes();
    if (tree == 0xFF)
        return 0xFF;
    if (sHoney.trees[tree].stamp == 0)
        return 0;
    if (now < sHoney.trees[tree].stamp)   // (a clock turned back: from now)
        sHoney.trees[tree].stamp = now;
    age = now - sHoney.trees[tree].stamp;
    if (age > 24 * 60)   // (a day on: the HONEY's eaten or dried; what came has gone)
    {
        sHoney.trees[tree].stamp = 0;
        return 0;
    }
    if (age < 6 * 60)
        return 1;
    if (sHoney.trees[tree].group == 0)
    {
        sHoney.trees[tree].stamp = 0;
        return 3;
    }
    return 2;
}

u16 Sinnoh_HoneySlather(void)
{
    u8 tree = TreeHere();
    struct HoneyTree *t;
    u32 r;
    if (tree == 0xFF)
        return 0;
    t = &sHoney.trees[tree];
    t->stamp = OwnMinutes();
    if (sHoney.last == tree + 1 && t->group != 0 && Random() % 100 < 90)
    {
        t->slot = RollSlot();
        return 0;
    }
    r = Random() % 100;
    if (MunchlaxTree(tree))
        t->group = r < 1 ? 3 : r < 10 ? 0 : r < 30 ? 1 : 2;
    else
        t->group = r < 10 ? 0 : r < 30 ? 2 : 1;
    t->slot = RollSlot();
    sHoney.last = tree + 1;
    return 0;
}

// The POKéMON in the tree for the battle (setwildbattle's part: the script's dowildbattle follows), levels 5-15; the
// tree is bare again. One the GENERATION setting doesn't allow is swapped for one it does (HERACROSS, AIPOM...).
u16 Sinnoh_HoneyTreeMon(void)
{
    static const u16 sFallback[] = {SPECIES_HERACROSS, SPECIES_AIPOM, SPECIES_WURMPLE};
    u8 tree = TreeHere(), level;
    u16 ability;
    u16 species;
    u32 i;
    if (tree == 0xFF || sHoney.trees[tree].group == 0)
        return 0;
    species = sHoneyMons[sHoney.trees[tree].group - 1][sHoney.trees[tree].slot % 6];
    for (i = 0; !MP_Gen_Allowed(species) && i < ARRAY_COUNT(sFallback); i++)
        species = sFallback[i];
    level = 5 + Random() % 11;
    ability = GetMonAbility(&gPlayerParty[0]);
    if (!GetMonData(&gPlayerParty[0], MON_DATA_IS_EGG)
     && (ability == ABILITY_HUSTLE || ability == ABILITY_VITAL_SPIRIT || ability == ABILITY_PRESSURE) && (Random() & 1))
        level = 15;
    sHoney.trees[tree].stamp = 0;
    MPNet_Log("sinnoh: honey tree %d: species %d level %d", tree, species, level);
    CreateScriptedWildMon(species, level, ITEM_NONE);
    return 0;
}

// ------------------------------------------------------------------------------------------------ the TROPHY GARDEN
// MR. BACKLOT in the POKéMON MANSION's office (data/scripts/sinnoh_wild.inc): once SINNOH's LEAGUE is beaten he boasts
// of a POKéMON in his TROPHY GARDEN once a day, and it lives there (grass slot 6), the one before it in slot 7.
// VAR_RESULT: 0 not yet (the LEAGUE), 1 he told today (gStringVar1 its name), 2 he has a new one to tell
u16 Sinnoh_TrophyGardenToday(void)
{
    if (!MP_Region_Cleared(REGION_SINNOH))
        return gSpecialVar_Result = 0;
    if (sHoney.trophy[0] != 0 && sHoney.trophyDay == RtcGetLocalDayCount() + 1)
    {
        GetSpeciesName(gStringVar1, sTrophyMons[(sHoney.trophy[0] - 1) % ARRAY_COUNT(sTrophyMons)]);
        return gSpecialVar_Result = 1;
    }
    return gSpecialVar_Result = 2;
}

// Today's new one (gStringVar1 its name): not one of the two there now, and allowed by the GENERATION setting
u16 Sinnoh_TrophyGardenAdd(void)
{
    u32 i;
    u8 pick = 0;
    for (i = 0; i < 64 && pick == 0; i++)
    {
        u8 n = 1 + Random() % ARRAY_COUNT(sTrophyMons);
        if (n != sHoney.trophy[0] && n != sHoney.trophy[1] && MP_Gen_Allowed(sTrophyMons[n - 1]))
            pick = n;
    }
    if (pick == 0)
        pick = 1;
    sHoney.trophy[1] = sHoney.trophy[0];
    sHoney.trophy[0] = pick;
    sHoney.trophyDay = RtcGetLocalDayCount() + 1;
    GetSpeciesName(gStringVar1, sTrophyMons[pick - 1]);
    MPNet_Log("sinnoh: trophy garden: species %d", sTrophyMons[pick - 1]);
    return 0;
}

// (tests: `mp honeyage N[,G]` makes this place's tree's HONEY N minutes old, what comes from group G 1-3)
void Sinnoh_HoneyDebugAge(u32 minutes, s32 group)
{
    u8 tree = TreeHere();
    if (tree != 0xFF && sHoney.trees[tree].stamp != 0)
    {
        sHoney.trees[tree].stamp = OwnMinutes() - minutes;
        if (group >= 0 && group <= 3)
            sHoney.trees[tree].group = group;
    }
}

// (data/specials.inc: the place of three specials gone, so the numbers after them stay: the HALLOWED TOWER's stand-in,
// before Platinum's own script ran it)
u16 Sinnoh_SpecialRetired(void)
{
    return 0;
}

// ------------------------------------------------------------------------------------------------- the POKé RADAR
// Grass slot INDEX's POKéMON in a POKé RADAR encounter (src/mp/mp_pokeradar.c): Platinum's radar ones in slots 4, 5,
// 10 and 11 where the slot still holds the table's (SPECIES: the slot's as Sinnoh_LandSlot gives it)
u16 Sinnoh_RadarSlot(u16 species, u8 index)
{
    static const u8 sSlots[] = {4, 5, 10, 11};
    u32 i, k;
    if (MP_Region_Current() != REGION_SINNOH)
        return species;
    for (k = 0; k < ARRAY_COUNT(sSlots) && sSlots[k] != index; k++)
        ;
    if (k == ARRAY_COUNT(sSlots))
        return species;
    for (i = 0; i < ARRAY_COUNT(sRadar); i++)
        if (sRadar[i].map == CurrentMap() && sRadar[i].base[k] == species && MP_Gen_Allowed(sRadar[i].radar[k]))
            return sRadar[i].radar[k];
    return species;
}
