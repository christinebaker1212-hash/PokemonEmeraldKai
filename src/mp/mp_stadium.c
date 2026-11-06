// POKéMON STADIUM on BATTLE ISLAND (docs/STADIUM.md, docs/stadium/RULES.md).
// The data: tools/stadium/gen_data.py -> src/data/mp_stadium_data.h (from the user's ROM, as STADIUM has it).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"
#include "battle.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/moves.h"
#include "constants/characters.h"
#include "constants/pokemon.h"

#include "../data/mp_stadium_data.h"

// The game's stat order (HP, ATK, DEF, SPE, SPA, SPD); a nature's (ATK, DEF, SPE, SPA, SPD) is it minus one
enum { S_HP, S_ATK, S_DEF, S_SPE, S_SPA, S_SPD };

static u32 CeilSqrt(u32 v)
{
    u32 r = 0;
    while (r * r < v)
        r++;
    return r;
}

// The stat that suits it besides its attack: the highest of HP, DEF, SPE and SPD (SPEED for PERSIAN, HP for SNORLAX)
static u8 BenefitStat(u16 species)
{
    const struct SpeciesInfo *b = &gSpeciesInfo[species];
    u8 best = S_SPE, v = b->baseSpeed;
    if (b->baseHP > v)
        best = S_HP, v = b->baseHP;
    if (b->baseDefense > v)
        best = S_DEF, v = b->baseDefense;
    if (b->baseSpDefense > v)
        best = S_SPD;
    return best;
}

void Stadium_Convert(const struct StadiumMon *sm, u8 ivs[6], u8 evs[6], u8 *nature)
{
    u8 atk = (sm->dvs >> 12) & 15, def = (sm->dvs >> 8) & 15, spd = (sm->dvs >> 4) & 15, spc = sm->dvs & 15;
    u8 hp = ((atk & 1) << 3) | ((def & 1) << 2) | ((spd & 1) << 1) | (spc & 1);
    u8 want[6], order[6], n = 0, i, j, up, down, benefit = BenefitStat(sm->species);
    u32 phys = 0, spec = 0, total = 0, left = MAX_TOTAL_EVS;
    const u16 statExpOf[6] = {sm->statExp[0], sm->statExp[1], sm->statExp[2], sm->statExp[3], sm->statExp[4], sm->statExp[4]};

    // DVs to their closest IVs: a DV covers two IVs (15 -> 31, 0 -> 1); SPECIAL is both SP. ATK and SP. DEF
    ivs[S_HP] = hp * 2 + 1, ivs[S_ATK] = atk * 2 + 1, ivs[S_DEF] = def * 2 + 1;
    ivs[S_SPE] = spd * 2 + 1, ivs[S_SPA] = ivs[S_SPD] = spc * 2 + 1;

    // which attacking stat its moves use (the game's split: gMoveCategories)
    for (i = 0; i < 4; i++)
    {
        u16 m = sm->moves[i];
        if (m == MOVE_NONE || gBattleMoves[m].power == 0)
            continue;
        if (gMoveCategories[m] == MOVE_CATEGORY_PHYSICAL)
            phys += gBattleMoves[m].power;
        else if (gMoveCategories[m] == MOVE_CATEGORY_SPECIAL)
            spec += gBattleMoves[m].power;
    }
    if (phys == 0 && spec == 0)
        up = benefit == S_HP ? S_DEF : benefit;   // all status moves: the stat that keeps it standing
    else
        up = phys >= spec ? S_ATK : S_SPA;
    if (phys != 0 && spec != 0)   // both: lower the weaker defense it doesn't lean on
        down = gSpeciesInfo[sm->species].baseDefense < gSpeciesInfo[sm->species].baseSpDefense ? S_DEF : S_SPD;
    else
        down = up == S_ATK ? S_SPA : S_ATK;
    if (down == benefit || down == up)
        down = up == S_ATK ? S_SPA : S_ATK;
    *nature = (up - 1) * 5 + (down - 1);

    // stat EXP to EVs: Generation I adds ceil(sqrt(stat EXP)) / 4 points at level 100 (63 at most), Generation III
    // EV / 4, so the same points are 4 x as many EVs. Over 510 in all, the attacking stat and the benefit stat first
    for (i = 0; i < 6; i++)
    {
        u32 pts = CeilSqrt(statExpOf[i]) / 4;
        want[i] = (pts > 63 ? 63 : pts) * 4;
        total += want[i];
        evs[i] = 0;
    }
    if (total <= MAX_TOTAL_EVS)
    {
        memcpy(evs, want, 6);
        return;
    }
    order[n++] = up;
    if (benefit != up)
        order[n++] = benefit;
    {
        // the rest by base stat, highest first
        const struct SpeciesInfo *bs = &gSpeciesInfo[sm->species];
        const u8 base[6] = {bs->baseHP, bs->baseAttack, bs->baseDefense, bs->baseSpeed, bs->baseSpAttack, bs->baseSpDefense};
        u8 first = n;
        for (i = 0; i < 6; i++)
        {
            bool8 seen = FALSE;
            for (j = 0; j < n; j++)
                seen |= order[j] == i;
            if (!seen && i != down)
                order[n++] = i;
        }
        for (i = first; i < n; i++)
            for (j = i + 1; j < n; j++)
                if (base[order[j]] > base[order[i]])
                {
                    u8 tmp = order[i];
                    order[i] = order[j], order[j] = tmp;
                }
    }
    order[n++] = down;
    for (i = 0; i < n && left > 0; i++)
    {
        u8 take = want[order[i]] < left ? want[order[i]] : left;
        evs[order[i]] = take;
        left -= take;
    }
}

void Stadium_BuildMon(struct Pokemon *mon, const struct StadiumMon *sm)
{
    u8 ivs[6], evs[6], nature, i, name[POKEMON_NAME_LENGTH + 1], ot[PLAYER_NAME_LENGTH + 1];
    u32 pid;
    Stadium_Convert(sm, ivs, evs, &nature);
    do
        pid = Random32();
    while (pid % NUM_NATURES != nature || IsShinyOtIdPersonality(sm->otId, pid));
    CreateMon(mon, sm->species, sm->level, 0, TRUE, pid, OT_ID_PRESET, sm->otId);
    for (i = 0; i < 6; i++)
    {
        SetMonData(mon, MON_DATA_HP_IV + i, &ivs[i]);
        SetMonData(mon, MON_DATA_HP_EV + i, &evs[i]);
    }
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, sm->moves[i], i);
    memset(name, EOS, sizeof(name));
    memcpy(name, sm->nickname, sizeof(sm->nickname) < POKEMON_NAME_LENGTH ? sizeof(sm->nickname) : POKEMON_NAME_LENGTH);
    SetMonData(mon, MON_DATA_NICKNAME, name);
    memset(ot, EOS, sizeof(ot));
    memcpy(ot, sm->otName, PLAYER_NAME_LENGTH);
    SetMonData(mon, MON_DATA_OT_NAME, ot);
    CalculateMonStats(mon);
}

const struct StadiumTrainer *Stadium_Trainer(u8 round, u8 division, u8 index)
{
    if (round >= STADIUM_ROUNDS || division >= STADIUM_DIVISION_COUNT || index >= sStadiumDivisions[round][division].count)
        return NULL;
    return &sStadiumTrainers[sStadiumDivisions[round][division].first + index];
}

u8 Stadium_TrainerCount(u8 round, u8 division)
{
    return round < STADIUM_ROUNDS && division < STADIUM_DIVISION_COUNT ? sStadiumDivisions[round][division].count : 0;
}

const struct StadiumMon *Stadium_TrainerMon(const struct StadiumTrainer *t, u8 i)
{
    return t != NULL && i < t->count ? &sStadiumMons[t->firstMon + i] : NULL;
}

u16 Stadium_RentalCount(u8 round, u8 list)
{
    return round < STADIUM_ROUNDS && list < STADIUM_RENTAL_LIST_COUNT ? sStadiumRentals[round][list].count : 0;
}

const struct StadiumMon *Stadium_Rental(u8 round, u8 list, u16 i)
{
    return i < Stadium_RentalCount(round, list) ? &sStadiumMons[sStadiumRentals[round][list].first + i] : NULL;
}

// mp stadium ROUND DIVISION TRAINER: the trainer's team as the game builds it (tests: stadium_data)
void MP_Stadium_Debug(u8 round, u8 division, u8 index)
{
    const struct StadiumTrainer *t = Stadium_Trainer(round, division, index);
    u8 i;
    if (t == NULL)
    {
        printf("[test] stadium: no trainer %d %d %d\n", round, division, index);
        return;
    }
    for (i = 0; i < t->count; i++)
    {
        const struct StadiumMon *sm = Stadium_TrainerMon(t, i);
        u8 ivs[6], evs[6], nature;
        Stadium_Convert(sm, ivs, evs, &nature);
        printf("[test] stadium: %d/%d/%d mon %d species %d lv %d nature %d ivs %d %d %d %d %d %d evs %d %d %d %d %d %d\n",
               round, division, index, i, sm->species, sm->level, nature, ivs[0], ivs[1], ivs[2], ivs[3], ivs[4], ivs[5],
               evs[0], evs[1], evs[2], evs[3], evs[4], evs[5]);
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The CHALLENGE CUP: six random POKéMON, STADIUM 2's way (docs/stadium/RULES.md "The CHALLENGE CUP")

#include "data.h"
#include "constants/items.h"
#include "constants/battle_move_effects.h"
#include "constants/daycare.h"
#include "constants/trainers.h"
#include "party_menu.h"
#include "../data/mp_stadium_cc.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
u8 MP_SpeciesEggMoves(u16 species, u16 *out);   // daycare.c

#define CC_MAX_MOVES 160
#define GEN2_LAST_MOVE 251

static bool8 CcNeverMove(u16 m)
{
    return m == MOVE_RETURN || m == MOVE_FRUSTRATION || m == MOVE_HIDDEN_POWER || m == MOVE_REST || m == MOVE_RECOVER
        || m == MOVE_SLEEP_TALK || m == MOVE_RAPID_SPIN || m == MOVE_STRUGGLE || m == MOVE_NONE || m > GEN2_LAST_MOVE;
}

static void CcAdd(u16 *list, u8 *n, u16 m)
{
    u8 i;
    if (CcNeverMove(m) || *n >= CC_MAX_MOVES)
        return;
    for (i = 0; i < *n; i++)
        if (list[i] == m)
            return;
    list[(*n)++] = m;
}

static u16 CcPreEvolution(u16 species, bool8 *byLevel)
{
    u16 s;
    u8 i;
    for (s = 1; s <= 251; s++)
        for (i = 0; i < EVOS_PER_MON; i++)
            if (gEvolutionTable[s][i].method != 0 && gEvolutionTable[s][i].targetSpecies == species)
            {
                *byLevel = gEvolutionTable[s][i].method == EVO_LEVEL;
                return s;
            }
    return SPECIES_NONE;
}

// Every move it may get: level-up to the class's level (its earlier stages too, a level less for a level evolution),
// the TMs; Round 2 also its egg moves (Generation I and Crystal's tutors come in with the TMs here)
static u8 CcMovePool(u16 species, u8 level, u8 round, u16 *list)
{
    u8 n = 0, i, stageLevel = level;
    u16 s = species;
    while (s != SPECIES_NONE)
    {
        bool8 byLevel = FALSE;
        u16 pre;
        for (i = 0; gLevelUpLearnsets[s][i] != LEVEL_UP_END; i++)
            if ((gLevelUpLearnsets[s][i] >> 16) <= stageLevel)
                CcAdd(list, &n, gLevelUpLearnsets[s][i] & LEVEL_UP_MOVE_ID);
        for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
            if (CanSpeciesLearnTMHM(s, i))
                CcAdd(list, &n, ItemIdToBattleMoveId(ITEM_TM01 + i));
        if (round == 1)
        {
            u16 egg[EGG_MOVES_ARRAY_COUNT];
            u8 k, ne = MP_SpeciesEggMoves(s, egg);
            for (k = 0; k < ne; k++)
                CcAdd(list, &n, egg[k]);
        }
        pre = CcPreEvolution(s, &byLevel);
        if (byLevel && stageLevel > 1)
            stageLevel--;
        s = pre;
    }
    return n;
}

static bool8 CcDamaging(u16 m) { return gBattleMoves[m].power > 1; }
static bool8 CcGoodAttack(u16 m) { return gBattleMoves[m].power >= 70; }
static bool8 CcBadStatus(u16 m)
{
    u16 e = gBattleMoves[m].effect;
    return e == EFFECT_ATTACK_DOWN || e == EFFECT_DEFENSE_DOWN || e == EFFECT_SPEED_DOWN || e == EFFECT_ACCURACY_DOWN
        || e == EFFECT_EVASION_DOWN || e == EFFECT_EVASION_UP;
}

static u16 CcPick(const u16 *list, u8 n, const u16 *taken, bool8 (*ok)(u16, const void *), const void *ctx)
{
    u16 cand[CC_MAX_MOVES];
    u8 c = 0, i, j;
    for (i = 0; i < n; i++)
    {
        bool8 used = FALSE;
        for (j = 0; j < 4; j++)
            used |= taken[j] == list[i];
        if (!used && ok(list[i], ctx))
            cand[c++] = list[i];
    }
    return c ? cand[Random() % c] : MOVE_NONE;
}

struct CcCtx { const u8 *types; u8 type; bool8 good; u8 avoid1, avoid2; bool8 physical; };
static bool8 CcIsStabOf(u16 m, const void *p) { const struct CcCtx *c = p; return CcDamaging(m) && gBattleMoves[m].type == c->type && CcGoodAttack(m) == c->good; }
static bool8 CcIsStabAny(u16 m, const void *p) { const struct CcCtx *c = p; return CcDamaging(m) && gBattleMoves[m].type == c->type; }
static bool8 CcIsCoverage(u16 m, const void *p)
{
    const struct CcCtx *c = p;
    return CcDamaging(m) && gBattleMoves[m].type != c->avoid1 && gBattleMoves[m].type != c->avoid2
        && (gMoveCategories[m] == MOVE_CATEGORY_PHYSICAL) == c->physical;
}
static bool8 CcIsAnyAttack(u16 m, const void *p) { (void)p; return CcDamaging(m); }
static bool8 CcIsGoodStatus(u16 m, const void *p) { (void)p; return !CcDamaging(m) && !CcBadStatus(m); }
static bool8 CcIsBadStatus(u16 m, const void *p) { (void)p; return !CcDamaging(m) && CcBadStatus(m); }

static void CcMoves(u16 species, u8 level, u8 round, const u16 *favorites, u16 out[4])
{
    u16 list[CC_MAX_MOVES];
    u8 n, i;
    const struct SpeciesInfo *b = &gSpeciesInfo[species];
    struct CcCtx c = {b->types, b->types[0], TRUE, 0xFF, 0xFF, b->baseAttack >= b->baseSpAttack};
    for (i = 0; i < 4; i++)
        out[i] = MOVE_NONE;
    if (species == SPECIES_DITTO)
    {
        out[0] = MOVE_TRANSFORM;
        return;
    }
    if (species == SPECIES_WOBBUFFET)
    {
        out[0] = MOVE_COUNTER, out[1] = MOVE_MIRROR_COAT, out[2] = MOVE_DESTINY_BOND, out[3] = MOVE_SAFEGUARD;
        return;
    }
    n = CcMovePool(species, level, round, list);
    // 1: a damaging move, STAB and good first
    out[0] = CcPick(list, n, out, CcIsStabOf, &c);
    if (out[0] == MOVE_NONE && b->types[1] != b->types[0])
        c.type = b->types[1], out[0] = CcPick(list, n, out, CcIsStabOf, &c);
    if (out[0] == MOVE_NONE)
        c.type = b->types[0], out[0] = CcPick(list, n, out, CcIsStabAny, &c);
    if (out[0] == MOVE_NONE)
        out[0] = CcPick(list, n, out, CcIsAnyAttack, &c);
    // 2: a STAB of its second type, or an attack of another type
    if (b->types[1] != b->types[0])
        c.type = b->types[1], out[1] = CcPick(list, n, out, CcIsStabAny, &c);
    if (out[1] == MOVE_NONE)
    {
        c.avoid1 = out[0] ? gBattleMoves[out[0]].type : 0xFF;
        c.physical = !c.physical;
        out[1] = CcPick(list, n, out, CcIsCoverage, &c);
        c.physical = !c.physical;
        if (out[1] == MOVE_NONE)
            out[1] = CcPick(list, n, out, CcIsCoverage, &c);
    }
    // 3: the trainer's favorite if it can learn one, else a good status move (a bad one if none)
    for (i = 0; i < 7 && favorites != NULL && out[2] == MOVE_NONE; i++)
    {
        u8 k;
        for (k = 0; k < n && favorites[i] != MOVE_NONE; k++)
            if (list[k] == favorites[i] && list[k] != out[0] && list[k] != out[1])
                out[2] = list[k];
    }
    if (out[2] == MOVE_NONE)
        out[2] = CcPick(list, n, out, CcIsGoodStatus, &c);
    if (out[2] == MOVE_NONE)
        out[2] = CcPick(list, n, out, CcIsBadStatus, &c);
    // 4: an attack of its higher attacking stat in a type not used yet, or a status move
    c.avoid1 = out[0] ? gBattleMoves[out[0]].type : 0xFF;
    c.avoid2 = out[1] ? gBattleMoves[out[1]].type : 0xFF;
    out[3] = CcPick(list, n, out, CcIsCoverage, &c);
    if (out[3] == MOVE_NONE)
        out[3] = CcPick(list, n, out, CcIsBadStatus, &c);
    if (out[3] == MOVE_NONE)
        out[3] = CcPick(list, n, out, CcIsAnyAttack, &c);
    // the moves shuffled, empty ones last
    for (i = 3; i > 0; i--)
    {
        u8 j = Random() % (i + 1);
        u16 t = out[i];
        out[i] = out[j], out[j] = t;
    }
    for (i = 0; i < 3; i++)
        if (out[i] == MOVE_NONE)
        {
            u8 j;
            for (j = i + 1; j < 4; j++)
                if (out[j] != MOVE_NONE)
                {
                    out[i] = out[j], out[j] = MOVE_NONE;
                    break;
                }
        }
}

static u16 CcTypeItem(u8 type)
{
    switch (type)
    {
    case TYPE_FIRE: return ITEM_CHARCOAL;
    case TYPE_WATER: return ITEM_MYSTIC_WATER;
    case TYPE_GRASS: return ITEM_MIRACLE_SEED;
    case TYPE_ELECTRIC: return ITEM_MAGNET;
    case TYPE_PSYCHIC: return ITEM_TWISTED_SPOON;
    case TYPE_ICE: return ITEM_NEVER_MELT_ICE;
    case TYPE_FIGHTING: return ITEM_BLACK_BELT;
    case TYPE_POISON: return ITEM_POISON_BARB;
    case TYPE_GROUND: return ITEM_SOFT_SAND;
    case TYPE_FLYING: return ITEM_SHARP_BEAK;
    case TYPE_BUG: return ITEM_SILVER_POWDER;
    case TYPE_ROCK: return ITEM_HARD_STONE;
    case TYPE_GHOST: return ITEM_SPELL_TAG;
    case TYPE_DRAGON: return Random() & 1 ? ITEM_DRAGON_FANG : ITEM_DRAGON_SCALE;
    case TYPE_DARK: return ITEM_BLACK_GLASSES;
    case TYPE_STEEL: return ITEM_METAL_COAT;
    default: return ITEM_SILK_SCARF;   // NORMAL (STADIUM 2's PINK BOW / POLKADOT BOW)
    }
}

static u16 CcSpeciesItem(u16 species)
{
    switch (species)
    {
    case SPECIES_PIKACHU: return ITEM_LIGHT_BALL;
    case SPECIES_CUBONE: case SPECIES_MAROWAK: return ITEM_THICK_CLUB;
    case SPECIES_DITTO: return ITEM_METAL_POWDER;
    case SPECIES_FARFETCHD: return ITEM_STICK;
    case SPECIES_CHANSEY: return ITEM_LUCKY_PUNCH;
    default: return ITEM_NONE;
    }
}

// 116/256 a BERRY, 89/256 one for the first move, 38/256 a hax item, 13/256 its own (STADIUM 2's odds; its BERRIES
// as their Generation III namesakes)
static u16 CcItem(u16 species, u16 firstMove)
{
    static const u16 sBerries[] = {ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_CHERI_BERRY, ITEM_CHESTO_BERRY,
                                   ITEM_RAWST_BERRY, ITEM_ASPEAR_BERRY, ITEM_PECHA_BERRY, ITEM_PERSIM_BERRY,
                                   ITEM_LUM_BERRY, ITEM_LEPPA_BERRY, ITEM_BERRY_JUICE};
    static const u16 sHax[] = {ITEM_KINGS_ROCK, ITEM_SCOPE_LENS, ITEM_BRIGHT_POWDER, ITEM_QUICK_CLAW};
    u16 r = Random() % 256, own = CcSpeciesItem(species);
    if (r < 116)
        return sBerries[Random() % ARRAY_COUNT(sBerries)];
    if (r < 205)
        return firstMove != MOVE_NONE && CcDamaging(firstMove) ? CcTypeItem(gBattleMoves[firstMove].type)
                                                                : sBerries[Random() % ARRAY_COUNT(sBerries)];
    if (r < 243 || own == ITEM_NONE)
        return sHax[Random() % ARRAY_COUNT(sHax)];
    return own;
}

static bool8 CcHasType(const struct StadiumCcMon *m, u8 t) { return m->types[0] == t || m->types[1] == t; }

// The nickname: the player's "BULBAsaur", the opponents' "bulbaSAUR"
static void CcNickname(u8 *out, u16 species, u8 split, bool8 player)
{
    u8 i;
    StringCopy(out, gSpeciesNames[species]);
    for (i = 0; out[i] != EOS && i < POKEMON_NAME_LENGTH; i++)
    {
        bool8 lower = player ? i >= split : i < split;
        if (lower && out[i] >= CHAR_A && out[i] <= CHAR_Z)
            out[i] += CHAR_a - CHAR_A;
    }
}

void StadiumCc_MakeTeam(struct Pokemon *party, u8 cls, u8 round, u8 trainer)
{
    const struct StadiumCcClass *c = &sStadiumCcClasses[cls % STADIUM_CC_CLASSES];
    const struct StadiumCcTrainer *t = trainer == STADIUM_CC_PLAYER ? NULL : &sStadiumCcTrainers[cls % 4][trainer % 8];
    u8 tmpl[6], i, k, chosen[6];
    u16 items[6];
    u32 bstSum = 0;
    round = round ? 1 : 0;
    if (t != NULL)
        memcpy(tmpl, t->tmpl, 6);
    else
    {
        const u8 *pt = sStadiumCcPlayerTemplates[Random() % 6];
        tmpl[0] = pt[0], tmpl[1] = pt[1], tmpl[2] = pt[2], tmpl[3] = tmpl[4] = tmpl[5] = TYPE_NONE;
    }
    for (i = 0; i < 6; i++)
    {
        u16 cand[64], weight[64], total = 0, r;
        u8 n = 0;
        // the half of the pool that brings the team's average toward the class's BST
        bool8 wantHigh = i > 0 && bstSum / i < c->bstTarget;
        int pass;
        for (pass = 0; pass < 2 && n == 0; pass++)
            for (k = 0; k < c->count && n < 64; k++)
            {
                const struct StadiumCcMon *m = &sStadiumCcPool[c->first + k];
                u8 j;
                bool8 ok = TRUE;
                for (j = 0; j < i; j++)
                    ok &= chosen[j] != k;
                if (tmpl[i] != TYPE_NONE)
                    ok &= CcHasType(m, tmpl[i]);
                else   // a type not on the team yet
                    for (j = 0; j < i; j++)
                        ok &= !CcHasType(m, sStadiumCcPool[c->first + chosen[j]].types[0])
                           && !CcHasType(m, sStadiumCcPool[c->first + chosen[j]].types[1]);
                if (pass == 0 && i > 0)
                    ok &= wantHigh ? m->bst >= c->bstTarget : m->bst < c->bstTarget;
                if (ok)
                    cand[n] = k, weight[n] = m->rate + 1, total += m->rate + 1, n++;
            }
        if (n == 0)   // nothing fits the template: anything not taken
            for (k = 0; k < c->count && n < 64; k++)
            {
                u8 j;
                bool8 ok = TRUE;
                for (j = 0; j < i; j++)
                    ok &= chosen[j] != k;
                if (ok)
                    cand[n] = k, weight[n] = 1, total += 1, n++;
            }
        r = Random() % total;
        for (k = 0; k < n && r >= weight[k]; k++)
            r -= weight[k];
        chosen[i] = cand[k < n ? k : n - 1];
        bstSum += sStadiumCcPool[c->first + chosen[i]].bst;
    }

    for (i = 0; i < 6; i++)
    {
        const struct StadiumCcMon *m = &sStadiumCcPool[c->first + chosen[i]];
        u16 moves[4], item, se, spe;
        u8 name[POKEMON_NAME_LENGTH + 1], j, ev, iv;
        struct Pokemon *mon = &party[i];
        CcMoves(m->species, c->level, round, t ? t->moves : NULL, moves);
        do   // no two the same item
        {
            item = CcItem(m->species, moves[0]);
            for (j = 0; j < i && items[j] != item; j++)
                ;
        } while (j < i);
        items[i] = item;
        CreateMon(mon, m->species, c->level, 0, FALSE, 0, OT_ID_PRESET, 0);
        for (j = 0; j < 4; j++)
            SetMonMoveSlot(mon, moves[j], j);
        SetMonData(mon, MON_DATA_HELD_ITEM, &item);
        // random DVs as their IVs; stat EXP: the player's about 131 a BST point below 680, the trainers' their own
        se = t ? t->statExp[round] : (m->bst < 680 ? (680 - m->bst) * 131 : 0);
        spe = t && t->speedExp[round] ? t->speedExp[round] : se;
        for (j = 0; j < 6; j++)
        {
            u32 pts = CeilSqrt(j == S_SPE ? spe : se) / 4;
            iv = (Random() % 16) * 2 + 1;
            ev = (pts > 63 ? 63 : pts) * 4;
            SetMonData(mon, MON_DATA_HP_IV + j, &iv);
            SetMonData(mon, MON_DATA_HP_EV + j, &ev);
        }
        {   // over 510 EVs: the same share of each
            u32 sum = 0;
            for (j = 0; j < 6; j++)
                sum += GetMonData(mon, MON_DATA_HP_EV + j);
            if (sum > MAX_TOTAL_EVS)
                for (j = 0; j < 6; j++)
                {
                    ev = GetMonData(mon, MON_DATA_HP_EV + j) * MAX_TOTAL_EVS / sum;
                    SetMonData(mon, MON_DATA_HP_EV + j, &ev);
                }
        }
        CcNickname(name, m->species, m->split, t == NULL);
        SetMonData(mon, MON_DATA_NICKNAME, name);
        CalculateMonStats(mon);
    }
}

// mp ccteam CLASS ROUND TRAINER (255: the player's): a CHALLENGE CUP team as rolled (tests: stadium_data)
void MP_StadiumCc_Debug(u8 cls, u8 round, u8 trainer)
{
    static struct Pokemon party[6];
    u8 i, j;
    StadiumCc_MakeTeam(party, cls, round, trainer);
    for (i = 0; i < 6; i++)
    {
        printf("[test] ccteam: %d species %d lv %d item %d moves", i, (int)GetMonData(&party[i], MON_DATA_SPECIES),
               (int)GetMonData(&party[i], MON_DATA_LEVEL), (int)GetMonData(&party[i], MON_DATA_HELD_ITEM));
        for (j = 0; j < 4; j++)
            printf(" %d", (int)GetMonData(&party[i], MON_DATA_MOVE1 + j));
        printf("\n");
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The lobby (data/maps/BattleIsland_Stadium/scripts.inc)

#include "script.h"
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// The NURSE's news: the player's own results and the cups' champions (gStringVar4)
void MP_Stadium_BufferNews(void)
{
    char buf[400];
    StadiumRecords_News(buf, sizeof(buf));
    MP_AsciiToGame(gStringVar4, buf, sizeof(gStringVar4) - 1);
}

// The attendant: into POKéMON STADIUM (its screens: mp_stadium_ui.c), the script waits until they close
void MP_Stadium_Open(void)
{
    void MP_StadiumUi_Open(void);
    MP_StadiumUi_Open();
}

const struct StadiumCcTrainer *StadiumCc_Trainer(u8 cls, u8 i)
{
    return &sStadiumCcTrainers[cls % 4][i % 8];
}
