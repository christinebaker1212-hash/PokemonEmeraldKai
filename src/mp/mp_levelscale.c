// Moemon Co-op: FF8-style level scaling (task 198). Wild Pokémon and trainers keep up with the team, so Double and
// Triple Battles don't leave everything behind. OPTIONS > GAME > LEVEL SCALING (on by default; online the room host's
// setting counts, as TEAM PADDING: MP_LevelScaling in mp_main.c).
//
// - The team's level: the average of its 3 highest-level Pokémon (MP_LevelScale_Top3). In a co-op battle, the average
//   of everyone's (each player sends theirs in the wire state: MpState.top3).
// - Regular trainers' strongest Pokémon match it (the rest keep their gap below); bosses (Gym Leaders, team leaders and
//   admins, the rival, the Elite Four and the Champion) are 2-5 levels above, more with more badges; wild Pokémon 3-6
//   below.
// - Never below the normal level (the area's, the trainer's), never above a cap that rises with the badges (bosses
//   their margin above it); after the League, 100.
// - A raised Pokémon evolves if it would have by that level (level evolutions) and knows that level's moves; trainers
//   with set moves keep them.
// - EXP is normal for the level they are.
// The Battle Frontier, the Battle Pike and Pyramid, mass outbreaks and roaming Pokémon keep their own levels.
#include <stdio.h>
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

static u8 Cap(void)
{
    u8 b = MP_Badges();
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return MAX_LEVEL;
    return sCaps[b < ARRAY_COUNT(sCaps) ? b : ARRAY_COUNT(sCaps) - 1];
}

static u8 BossMargin(u16 trainerId)
{
    u8 cls = gTrainers[trainerId].trainerClass;
    if (cls == TRAINER_CLASS_ELITE_FOUR || cls == TRAINER_CLASS_CHAMPION)
        return 5;
    if (cls == TRAINER_CLASS_RIVAL || MP_Coop_IsBossTrainer(trainerId))
        return 2 + (MP_Badges() * 3 + 4) / 8;   // +2 with no badges ... +5 with eight
    return 0;
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
    if (!Enabled() || (ref = Reference()) == 0)
        return normal;
    margin = BossMargin(trainerId);
    target = ref + margin;
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

// A wild Pokémon's level (ChooseWildMonLevel's, then this): 3-6 below the team, never below its own
u8 MP_LevelScale_Wild(u8 normal)
{
    u8 ref;
    s32 target;
    sPendingNormal = normal;
    sPendingValid = TRUE;
    if (!Enabled() || (ref = Reference()) == 0)
        return normal;
    target = (s32)ref - 3 - (Random() % 4);
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
        if (to == SPECIES_NONE)
            break;
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

// The wild Pokémon just made (CreateWildMon): evolved if it was raised past an evolution, and remembered with its own
// level (a co-op battle may raise it for the whole team)
void MP_LevelScale_WildMade(struct Pokemon *mon)
{
    u8 level = GetMonData(mon, MON_DATA_LEVEL);
    if (!sPendingValid)
        return;
    if (level > sPendingNormal)
        MP_LevelScale_EvolveMon(mon);
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
    target = (s32)ref - 3 - (personality % 4);   // the same spread every game would pick (no Random: only the master runs)
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
    MP_LevelScale_EvolveMon(mon);
    MPNet_Log("level scale: co-op wild %d Lv%d -> Lv%d (team %d)", GetMonData(mon, MON_DATA_SPECIES), level, target, ref);
}
