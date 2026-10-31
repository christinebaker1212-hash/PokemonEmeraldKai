// Moemon Co-op: FF8-style level scaling (task 198). Wild Pokémon and trainers keep up with the team, so Double and
// Triple Battles don't leave everything behind. LEVEL SCALING and its settings (MP_SCALE_*: OPTIONS > GAME > LEVEL
// SCALING playing alone, HOST A GAME > GAME RULES for a room; online the host's count: MP_LevelScaleRule).
//
// - The team's level: the average of its 3 highest-level Pokémon (MP_LevelScale_Top3). In a co-op battle, the average
//   of everyone's (each player sends theirs in the wire state: MpState.top3).
// - TRAINERS: regular trainers' strongest Pokémon BELOW (4 under) / EVEN with / ABOVE (3 over) it (the rest keep their
//   gap below); bosses (Gym Leaders, team leaders and admins, the rival, the Elite Four and the Champion) BOSS MARGIN
//   more (AUTO: 2-5 by badges, the Elite Four 5).
// - WILD: LOW a quarter of the way from the area's level to it, MEDIUM halfway (the default), HIGH 3-6 below it.
// - Never below the normal level (the area's, the trainer's), never above a cap that rises with the badges (bosses
//   their margin above it) when BADGE CAPS is on; with eight badges PAST 8 BADGES: capped until the League then 100,
//   free, or always capped (80 after the League).
// - A raised trainer's Pokémon evolves if it would have by that level (level evolutions) and knows that level's moves;
//   trainers with set moves keep them. A raised wild one stays the area's Pokémon; 1 in 10 is the evolved one, a rare
//   encounter (RareEvolvedForm).
// - EXP is normal for the level they are.
// The Battle Frontier, the Battle Pike and Pyramid, mass outbreaks and roaming Pokémon keep their own levels.
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "pokemon.h"
#include "data.h"
#include "random.h"
#include "constants/flags.h"
#include "constants/trainers.h"
#include "mp.h"
#include "mp_region.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

static const u8 sCaps[] = {18, 24, 30, 36, 42, 48, 54, 60, 65};   // by badges; 100 after the League

static u8 sPendingNormal;      // ChooseWildMonLevel's own level for the wild Pokémon about to be made
static bool8 sPendingValid;
static struct { u32 personality; u8 normal; } sNoted[8];   // wild Pokémon made lately and their own levels
static u8 sNotedNext;

static bool8 Enabled(void)
{
    return MP_LevelScaling() && !InBattlePike() && !InBattlePyramid_();
}

u8 MP_LevelScale_Top3(void)
{
    u8 lv[PARTY_SIZE], n = 0, i, j, sum = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        lv[n++] = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
    }
    for (i = 0; i < n; i++)   // highest first
        for (j = i + 1; j < n; j++)
            if (lv[j] > lv[i])
            {
                u8 t = lv[i];
                lv[i] = lv[j];
                lv[j] = t;
            }
    if (n > 3)
        n = 3;
    if (n == 0)
        return 0;
    {
        u16 s = 0;
        for (i = 0; i < n; i++)
            s += lv[i];
        sum = (s + n / 2) / n;
    }
    return sum;
}

// The team's level for this battle: ours, or the average of everyone's in a co-op battle
static u8 Reference(void)
{
    u8 seats = MP_Coop_SeatCount(), own = MP_LevelScale_Top3(), s, n = 0;
    u16 sum = 0;
    if (!gMpCoopBattle || seats < 2)
        return own;
    for (s = 0; s < seats; s++)
    {
        u8 id = MP_Coop_SeatPlayer(s), lv = own;
        if (s != MP_Coop_Seat() && id < MP_MAX_PLAYERS && gMpPlayers[id].present && gMpPlayers[id].hasState)
            lv = gMpPlayers[id].state.top3 ? gMpPlayers[id].state.top3 : gMpPlayers[id].state.topLevel;
        if (lv == 0)
            continue;
        sum += lv;
        n++;
    }
    return n ? (sum + n / 2) / n : own;
}

// The level nothing scales past: by badges (BADGE CAPS), and with all eight as PAST 8 BADGES says
static u8 Cap(void)
{
    u8 b = MP_Badges();
    if (!MP_LevelScaleRule(MP_SCALE_CAPS))
        return MAX_LEVEL;
    if (b >= 8)
    {
        switch (MP_LevelScaleRule(MP_SCALE_PAST8))
        {
        case 1: return MAX_LEVEL;                                           // free
        case 2: return FlagGet(FLAG_SYS_GAME_CLEAR) ? 80 : sCaps[8];        // always capped
        }
    }
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return MAX_LEVEL;
    return sCaps[b < ARRAY_COUNT(sCaps) ? b : ARRAY_COUNT(sCaps) - 1];
}

static bool8 IsBoss(u16 trainerId)
{
    u8 cls = gTrainers[trainerId].trainerClass;
    return cls == TRAINER_CLASS_ELITE_FOUR || cls == TRAINER_CLASS_CHAMPION || cls == TRAINER_CLASS_RIVAL
        || MP_Coop_IsBossTrainer(trainerId);
}

static u8 BossMargin(u16 trainerId)
{
    u8 cls = gTrainers[trainerId].trainerClass, rule = MP_LevelScaleRule(MP_SCALE_BOSS);
    if (!IsBoss(trainerId))
        return 0;
    if (rule > 0)
        return rule - 1;   // BOSS MARGIN +0 .. +8
    if (cls == TRAINER_CLASS_ELITE_FOUR || cls == TRAINER_CLASS_CHAMPION)
        return 5;
    return 2 + (MP_Badges() * 3 + 4) / 8;   // AUTO: +2 with no badges ... +5 with eight
}

// TRAINERS: where a trainer's strongest Pokémon stands against the team
static s8 TrainerOffset(void)
{
    static const s8 sOffsets[] = {0, -4, 0, 3};   // (OFF never scales)
    u8 rule = MP_LevelScaleRule(MP_SCALE_TRAINERS);
    return sOffsets[rule < ARRAY_COUNT(sOffsets) ? rule : 2];
}

// WILD: how far toward the team a wild Pokémon comes (spread 0-3: Random alone, the personality in a co-op battle)
static s32 WildTarget(u8 ref, u8 normal, u8 spread)
{
    switch (MP_LevelScaleRule(MP_SCALE_WILD))
    {
    case 0:  return normal;
    case 1:  return normal + ((s32)ref - normal) / 4 - (spread & 1);   // LOW: a quarter of the way
    case 3:  return (s32)ref - 3 - spread;                            // HIGH: 3-6 below the team
    default: return normal + ((s32)ref - normal) / 2 - (spread % 3);  // MEDIUM: halfway
    }
}

static u8 ClampLevel(s32 v)
{
    return v < 1 ? 1 : v > MAX_LEVEL ? MAX_LEVEL : v;
}

// A trainer's Pokémon: its normal level, and the normal level of the trainer's strongest one
u8 MP_LevelScale_TrainerMon(u16 trainerId, u8 normal, u8 partyMax)
{
    u8 ref, margin;
    s32 target;
    if (!Enabled() || (ref = Reference()) == 0 || MP_LevelScaleRule(MP_SCALE_TRAINERS) == 0)
        return normal;
    margin = BossMargin(trainerId);
    target = ref + TrainerOffset() + margin;
    if (target > Cap() + margin)
        target = Cap() + margin;
    if (target <= partyMax)
        return normal;
    return ClampLevel(normal + (target - partyMax));
}

void MP_LevelScale_LogTrainer(u16 trainerId, u8 partyMax)
{
    if (Enabled())
        MPNet_Log("level scale: trainer %d ace %d -> %d (team %d, cap %d)", trainerId, partyMax,
                  MP_LevelScale_TrainerMon(trainerId, partyMax, partyMax), Reference(), Cap() + BossMargin(trainerId));
}

// A wild Pokémon's level (ChooseWildMonLevel's, then this): toward the team as WILD says, never below its own
u8 MP_LevelScale_Wild(u8 normal)
{
    u8 ref;
    s32 target;
    sPendingNormal = normal;
    sPendingValid = TRUE;
    if (!Enabled() || (ref = Reference()) == 0 || ref <= normal)
        return normal;
    target = WildTarget(ref, normal, Random() % 4);
    if (target > Cap())
        target = Cap();
    return target > normal ? ClampLevel(target) : normal;
}

void MP_LevelScale_ForgetPending(void)
{
    sPendingValid = FALSE;
}

// The species a Pokémon would be by this level (level evolutions only: stones, trades and friendship need more
// than a level)
u16 MP_LevelScale_Evolve(u16 species, u8 level, u32 personality)
{
    u8 stage, i;
    for (stage = 0; stage < 3; stage++)
    {
        u16 to = SPECIES_NONE;
        for (i = 0; i < EVOS_PER_MON && to == SPECIES_NONE; i++)
        {
            const struct Evolution *e = &gEvolutionTable[species][i];
            if (e->targetSpecies == SPECIES_NONE || level < e->param)
                continue;
            switch (e->method)
            {
            case EVO_LEVEL:
            case EVO_LEVEL_NINJASK:
                to = e->targetSpecies;
                break;
            case EVO_LEVEL_ATK_GT_DEF:   // Tyrogue: one of the three, by its personality
            case EVO_LEVEL_ATK_EQ_DEF:
            case EVO_LEVEL_ATK_LT_DEF:
                if (e->method - EVO_LEVEL_ATK_GT_DEF == personality % 3)
                    to = e->targetSpecies;
                break;
            case EVO_LEVEL_SILCOON:      // Wurmple, as the game decides it
                if ((personality >> 16) % 10 <= 4)
                    to = e->targetSpecies;
                break;
            case EVO_LEVEL_CASCOON:
                if ((personality >> 16) % 10 > 4)
                    to = e->targetSpecies;
                break;
            }
        }
        if (to == SPECIES_NONE || (IS_GEN4_SPECIES(to) && !IS_GEN4_SPECIES(species)) || MP_Gen_SealedSpecies(to))
            break;   // (Moemon Kai: never into a Gen 4 Moemon on the way, nor past the GENERATION)
        species = to;
    }
    return species;
}

// A raised Pokémon becomes what it would be by its level: evolved (its name too), with that level's moves
void MP_LevelScale_EvolveMon(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES), evo;
    if (species == SPECIES_NONE)
        return;
    evo = MP_LevelScale_Evolve(species, GetMonData(mon, MON_DATA_LEVEL), GetMonData(mon, MON_DATA_PERSONALITY));
    if (evo == species)
        return;
    SetMonData(mon, MON_DATA_SPECIES, &evo);
    SetMonData(mon, MON_DATA_NICKNAME, gSpeciesNames[evo]);
    CalculateMonStats(mon);
    GiveMonInitialMoveset(mon);
    MPNet_Log("level scale: evolved %d -> %d at Lv%d", species, evo, GetMonData(mon, MON_DATA_LEVEL));
}

// A wild Pokémon raised past an evolution stays what the area has (the normal encounter); now and then (1 in 10) the
// evolved one turns up instead, a rare encounter. Read from its personality, so every game in a co-op battle agrees.
static bool8 RareEvolvedForm(struct Pokemon *mon)
{
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    if (MP_LevelScale_Evolve(species, GetMonData(mon, MON_DATA_LEVEL), personality) == species)
        return FALSE;
    if (getenv("MOEMON_SCALE_RARE"))   // tests: 1 always the evolved one, 0 never
        return atoi(getenv("MOEMON_SCALE_RARE")) != 0;
    return ((personality >> 8) % 10) == 0;
}

// The wild Pokémon just made (CreateWildMon): raised past an evolution, the rare evolved form now and then; remembered
// with its own level (a co-op battle may raise it for the whole team)
void MP_LevelScale_WildMade(struct Pokemon *mon)
{
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    if (!sPendingValid)
        return;
    if (level > sPendingNormal && RareEvolvedForm(mon))
    {
        MPNet_Log("level scale: a rare evolved form");
        MP_LevelScale_EvolveMon(mon);
    }
    sNoted[sNotedNext].personality = GetMonData(mon, MON_DATA_PERSONALITY);
    sNoted[sNotedNext].normal = sPendingNormal;
    sNotedNext = (sNotedNext + 1) % ARRAY_COUNT(sNoted);
    if (level != sPendingNormal)
        MPNet_Log("level scale: wild %d Lv%d -> Lv%d (team %d, cap %d)", GetMonData(mon, MON_DATA_SPECIES), sPendingNormal,
                  level, Reference(), Cap());
    sPendingValid = FALSE;
}

// The co-op master: a wild Pokémon raised to the whole team's level (it was made for the one who found it)
void MP_LevelScale_CoopWild(struct Pokemon *mon)
{
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY), exp;
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 level = GetMonData(mon, MON_DATA_LEVEL), normal = level, i, ref;
    s32 target;

    if (species == SPECIES_NONE || !Enabled() || (ref = Reference()) == 0)
        return;
    for (i = 0; i < ARRAY_COUNT(sNoted); i++)
        if (sNoted[i].personality == personality && sNoted[i].normal != 0)
            normal = sNoted[i].normal;
    target = ref > normal ? WildTarget(ref, normal, personality % 4) : normal;   // the same spread every game would pick (no Random: only the master runs)
    if (target > Cap())
        target = Cap();
    if (target < normal)
        target = normal;
    if (target == level)
        return;
    exp = gExperienceTables[gSpeciesInfo[species].growthRate][target];
    SetMonData(mon, MON_DATA_EXP, &exp);
    CalculateMonStats(mon);
    GiveMonInitialMoveset(mon);
    if (RareEvolvedForm(mon))
        MP_LevelScale_EvolveMon(mon);
    MPNet_Log("level scale: co-op wild %d Lv%d -> Lv%d (team %d)", GetMonData(mon, MON_DATA_SPECIES), level, target, ref);
}
