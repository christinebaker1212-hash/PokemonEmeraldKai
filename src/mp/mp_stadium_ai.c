// POKéMON STADIUM's trainers switch like players (the user, 2026-11-07: "allow STADIUM trainers to switch out
// Pokemon intelligently"). The game's own switching (battle_ai_switch_items.c ShouldSwitch) only switches in a few
// narrow cases. In a STADIUM battle it asks here instead: every POKéMON the trainer could have out is weighed against
// the player's active one by how much each can hurt the other (the best move's damage as a share of the target's HP:
// power, STAB, type matchup, the stats, immunities from abilities), and the trainer switches when another one would
// clearly do better, with limits a person would keep to: not twice in a row, not out of a winning or boosted matchup,
// not when the one out can still finish the foe first, and no switch while it is trapped (ShouldSwitch's own checks
// run before this). The same weighing picks who comes in, after a switch or a faint.
#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "battle_anim.h"
#include "util.h"
#include "battle_main.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/battle_move_effects.h"
#include "constants/moves.h"
#include "mp.h"
#include "malloc.h"
#include "constants/species.h"

extern bool8 gMpStadiumBattle;

static u8 sLastSwitchTurn[MAX_BATTLERS_COUNT];
static bool8 sSwitched[MAX_BATTLERS_COUNT];

// one side of a matchup: what the damage needs from a battler or a party POKéMON
struct Side
{
    u16 species, ability, hp, maxHp, atk, def, spa, spd, spe;
    u8 level, types[2];
    u16 moves[MAX_MON_MOVES];
};

static void FromBattler(struct Side *s, u8 b)
{
    struct BattlePokemon *m = &gBattleMons[b];
    u32 i;
    s->species = m->species;
    s->ability = m->ability;
    s->hp = m->hp;
    s->maxHp = m->maxHP ? m->maxHP : 1;
    s->atk = m->attack;
    s->def = m->defense;
    s->spa = m->spAttack;
    s->spd = m->spDefense;
    s->spe = m->speed;
    s->level = m->level;
    s->types[0] = m->types[0];
    s->types[1] = m->types[1];
    for (i = 0; i < MAX_MON_MOVES; i++)
        s->moves[i] = m->moves[i];
}

static void FromMon(struct Side *s, struct Pokemon *mon)
{
    u32 i;
    s->species = GetMonData(mon, MON_DATA_SPECIES);
    s->ability = GetMonAbility(mon);
    s->hp = GetMonData(mon, MON_DATA_HP);
    s->maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    if (s->maxHp == 0)
        s->maxHp = 1;
    s->atk = GetMonData(mon, MON_DATA_ATK);
    s->def = GetMonData(mon, MON_DATA_DEF);
    s->spa = GetMonData(mon, MON_DATA_SPATK);
    s->spd = GetMonData(mon, MON_DATA_SPDEF);
    s->spe = GetMonData(mon, MON_DATA_SPEED);
    s->level = GetMonData(mon, MON_DATA_LEVEL);
    s->types[0] = gSpeciesInfo[s->species].types[0];
    s->types[1] = gSpeciesInfo[s->species].types[1];
    for (i = 0; i < MAX_MON_MOVES; i++)
        s->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
}

// the type chart's multiplier for a move type against a defender, x10 per type (10 normal, 0 none)
static u32 TypeMul(u8 moveType, const struct Side *d)
{
    u32 mul = 100, i;
    bool8 secondDiffers = d->types[1] != d->types[0];
    for (i = 0; TYPE_EFFECT_ATK_TYPE(i) != TYPE_ENDTABLE; i += 3)
    {
        if (TYPE_EFFECT_ATK_TYPE(i) == TYPE_FORESIGHT)
            continue;
        if (TYPE_EFFECT_ATK_TYPE(i) != moveType)
            continue;
        if (TYPE_EFFECT_DEF_TYPE(i) == d->types[0])
            mul = mul * TYPE_EFFECT_MULTIPLIER(i) / 10;
        if (secondDiffers && TYPE_EFFECT_DEF_TYPE(i) == d->types[1])
            mul = mul * TYPE_EFFECT_MULTIPLIER(i) / 10;
    }
    return mul;   // (100 = normal, 400 = four times, 0 = no effect)
}

static bool8 AbilityBlocks(u8 moveType, u16 ability)
{
    return (moveType == TYPE_GROUND && ability == ABILITY_LEVITATE)
        || (moveType == TYPE_ELECTRIC && ability == ABILITY_VOLT_ABSORB)
        || (moveType == TYPE_WATER && ability == ABILITY_WATER_ABSORB)
        || (moveType == TYPE_FIRE && ability == ABILITY_FLASH_FIRE);
}

// the best damage A's moves do to D, as a share of D's HP now, in tenths of a percent (1000 = all of it)
static u32 BestDamage(const struct Side *a, const struct Side *d)
{
    u32 best = 0, i;
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 move = a->moves[i];
        u32 power, mul, atk, def, dmg;
        u8 type;
        if (move == MOVE_NONE || move >= MOVES_COUNT)
            continue;
        power = gBattleMoves[move].power;
        type = gBattleMoves[move].type;
        if (power == 0)
            continue;   // (status moves: the matchup is about damage)
        if (power == 1)
            power = 60;   // (variable power: about this much)
        if (gBattleMoves[move].effect == EFFECT_EXPLOSION)
            continue;   // (never counted on: it ends the user)
        mul = TypeMul(type, d);
        if (mul == 0 || AbilityBlocks(type, d->ability))
            continue;
        if (d->ability == ABILITY_WONDER_GUARD && mul <= 100)
            continue;
        atk = IS_TYPE_PHYSICAL(type) ? a->atk : a->spa;
        def = IS_TYPE_PHYSICAL(type) ? d->def : d->spd;
        if (def == 0)
            def = 1;
        dmg = ((2 * a->level / 5 + 2) * power * atk / def) / 50 + 2;
        if (type == a->types[0] || type == a->types[1])
            dmg = dmg * 3 / 2;
        dmg = dmg * mul / 100;
        dmg = dmg * 9 / 10;   // (the damage roll's middle)
        dmg = dmg * 1000 / (d->hp ? d->hp : 1);
        if (dmg > best)
            best = dmg;
    }
    return best > 2000 ? 2000 : best;
}

// a matchup's worth for A against the foe F: what A does to F against what F does to A, the faster one's hit first
static s32 Matchup(const struct Side *a, const struct Side *f, s32 *outHurt, s32 *outThreat)
{
    s32 hurt = BestDamage(a, f), threat = BestDamage(f, a), score;
    score = hurt - threat;
    if (a->spe > f->spe && hurt >= 1000)
        score += 600;   // (it knocks the foe out before the foe moves)
    if (f->spe > a->spe && threat >= 1000)
        score -= 600;   // (the foe knocks it out first)
    if (outHurt)
        *outHurt = hurt;
    if (outThreat)
        *outThreat = threat;
    return score;
}

static u8 FoeOf(u8 battler)
{
    u8 foe = GetBattlerAtPosition(BATTLE_OPPOSITE(GetBattlerPosition(battler)));
    if (gAbsentBattlerFlags & gBitTable[foe])
        foe = GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(foe)));
    return foe;
}

static bool8 Usable(struct Pokemon *party, s32 i, u8 battler)
{
    u16 species = GetMonData(&party[i], MON_DATA_SPECIES_OR_EGG);
    return species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(&party[i], MON_DATA_HP) != 0
        && i != gBattlerPartyIndexes[battler] && i != gBattleStruct->monToSwitchIntoId[battler];
}

// the best one to bring in against the foe, or PARTY_SIZE; its matchup's score in *outScore
static u8 BestBench(u8 battler, s32 firstId, s32 lastId, const struct Side *foe, bool8 takesAHit, s32 *outScore)
{
    struct Pokemon *party = GetBattlerSide(battler) == B_SIDE_PLAYER ? gPlayerParty : gEnemyParty;
    s32 i, best = -100000;
    u8 bestId = PARTY_SIZE;
    for (i = firstId; i < lastId; i++)
    {
        struct Side s;
        s32 score, threat;
        if (!Usable(party, i, battler))
            continue;
        FromMon(&s, &party[i]);
        score = Matchup(&s, foe, NULL, &threat);
        if (takesAHit)
            score -= threat / 2;   // (coming in by a switch, it takes the foe's hit first)
        if (score > best)
            best = score, bestId = i;
    }
    if (outScore)
        *outScore = best;
    return bestId;
}

static bool8 StatsRaised(u8 battler)
{
    u32 i;
    for (i = STAT_ATK; i < NUM_BATTLE_STATS; i++)
        if (gBattleMons[battler].statStages[i] > DEFAULT_STAT_STAGE + 1)
            return TRUE;
    return FALSE;
}

// ShouldSwitch's question in a STADIUM battle: switch, and to whom (gBattleStruct->AI_monToSwitchIntoId)
bool8 MP_StadiumAi_ShouldSwitch(u8 battler, s32 firstId, s32 lastId)
{
    struct Side me, foe;
    s32 now, hurt, threat, bench;
    u8 pick, turn = gBattleResults.battleTurnCounter;
    if (!gMpStadiumBattle)
        return FALSE;
    FromBattler(&me, battler);
    FromBattler(&foe, FoeOf(battler));
    now = Matchup(&me, &foe, &hurt, &threat);
    if (sSwitched[battler] && (u8)(turn - sLastSwitchTurn[battler]) < 2)
        return FALSE;   // (not twice in a row: it just came in)
    if (StatsRaised(battler) && threat < 700)
        return FALSE;   // (its boosts would be lost)
    if (me.spe > foe.spe && hurt >= 1000)
        return FALSE;   // (it finishes the foe first)
    if (me.hp * 4 < me.maxHp && threat >= 1000)
        return FALSE;   // (nearly gone either way: it hits back instead of wasting a turn)
    pick = BestBench(battler, firstId, lastId, &foe, TRUE, &bench);
    if (pick == PARTY_SIZE)
        return FALSE;
    // a clearly better matchup waiting: the one out is in danger, or can't touch the foe
    if (bench > now + 350 && (threat >= 600 || hurt < 150))
    {
        gBattleStruct->AI_monToSwitchIntoId[battler] = pick;
        sSwitched[battler] = TRUE;
        sLastSwitchTurn[battler] = turn;
        MPNet_Log("stadium ai: battler %d switches to party %d (matchup %d -> %d; dealt %d, taken %d per mille)",
                  battler, pick, now, bench, hurt, threat);
        return TRUE;
    }
    return FALSE;
}

// who comes in after a faint: the best matchup against the foe that's out (PARTY_SIZE: not a STADIUM battle)
u8 MP_StadiumAi_Replacement(u8 battler, s32 firstId, s32 lastId)
{
    struct Side foe;
    u8 pick;
    if (!gMpStadiumBattle)
        return PARTY_SIZE;
    FromBattler(&foe, FoeOf(battler));
    pick = BestBench(battler, firstId, lastId, &foe, FALSE, NULL);
    if (pick != PARTY_SIZE)
        MPNet_Log("stadium ai: battler %d sends out party %d", battler, pick);
    return pick;
}

void MP_StadiumAi_BattleStart(void)
{
    memset(sSwitched, 0, sizeof(sSwitched));
    memset(sLastSwitchTurn, 0, sizeof(sLastSwitchTurn));
}

// ---- `mp stadiumai`: the weighing on fixed matchups, outside a battle (test stadium_ai) ----

static void SetBattleMon(u8 b, struct Pokemon *mon)
{
    struct BattlePokemon *m = &gBattleMons[b];
    struct Side s;
    u32 i;
    FromMon(&s, mon);
    memset(m, 0, sizeof(*m));
    m->species = s.species;
    m->ability = s.ability;
    m->hp = s.hp;
    m->maxHP = s.maxHp;
    m->attack = s.atk;
    m->defense = s.def;
    m->spAttack = s.spa;
    m->spDefense = s.spd;
    m->speed = s.spe;
    m->level = s.level;
    m->types[0] = s.types[0];
    m->types[1] = s.types[1];
    for (i = 0; i < MAX_MON_MOVES; i++)
        m->moves[i] = s.moves[i];
    for (i = 0; i < NUM_BATTLE_STATS; i++)
        m->statStages[i] = DEFAULT_STAT_STAGE;
}

static void TestMon(struct Pokemon *mon, u16 species, u16 move1, u16 move2)
{
    CreateMon(mon, species, 50, 31, TRUE, 0, OT_ID_PLAYER_ID, 0);
    SetMonMoveSlot(mon, move1, 0);
    SetMonMoveSlot(mon, move2, 1);
    SetMonMoveSlot(mon, MOVE_NONE, 2);
    SetMonMoveSlot(mon, MOVE_NONE, 3);
}

void MP_StadiumAi_SelfTest(void)
{
    bool8 hadStruct = gBattleStruct != NULL, wasStadium = gMpStadiumBattle;
    u8 savedCount = gBattlersCount, savedPos[MAX_BATTLERS_COUNT];
    u16 savedIdx[MAX_BATTLERS_COUNT];
    bool8 sw;
    memcpy(savedPos, gBattlerPositions, sizeof(savedPos));
    memcpy(savedIdx, gBattlerPartyIndexes, sizeof(savedIdx));
    if (!hadStruct)
        gBattleStruct = AllocZeroed(sizeof(*gBattleStruct));
    gMpStadiumBattle = TRUE;
    gBattlersCount = 2;
    gBattlerPositions[0] = B_POSITION_PLAYER_LEFT;
    gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
    gAbsentBattlerFlags = 0;
    // the player's PIKACHU (THUNDERBOLT) against the trainer's GYARADOS (four times weak), GOLEM (immune, EARTHQUAKE)
    // and STARMIE in the back
    TestMon(&gPlayerParty[0], SPECIES_PIKACHU, MOVE_THUNDERBOLT, MOVE_QUICK_ATTACK);
    TestMon(&gEnemyParty[0], SPECIES_GYARADOS, MOVE_SURF, MOVE_BITE);
    TestMon(&gEnemyParty[1], SPECIES_STARMIE, MOVE_SURF, MOVE_PSYCHIC);
    TestMon(&gEnemyParty[2], SPECIES_GOLEM, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE);
    SetBattleMon(0, &gPlayerParty[0]);
    gBattlerPartyIndexes[0] = 0;
    // 1. GYARADOS out: switches, to GOLEM
    MP_StadiumAi_BattleStart();
    SetBattleMon(1, &gEnemyParty[0]);
    gBattlerPartyIndexes[1] = 0;
    gBattleStruct->monToSwitchIntoId[1] = PARTY_SIZE;
    gBattleStruct->AI_monToSwitchIntoId[1] = PARTY_SIZE;
    gBattleResults.battleTurnCounter = 3;
    sw = MP_StadiumAi_ShouldSwitch(1, 0, 3);
    MPNet_Log("stadium ai test: GYARADOS vs PIKACHU: %s %d", sw ? "switch to" : "stays", sw ? gBattleStruct->AI_monToSwitchIntoId[1] : -1);
    // 2. the next turn, GOLEM out: stays (it just came in, and it's the right one anyway)
    SetBattleMon(1, &gEnemyParty[2]);
    gBattlerPartyIndexes[1] = 2;
    gBattleStruct->AI_monToSwitchIntoId[1] = PARTY_SIZE;
    gBattleResults.battleTurnCounter = 4;
    sw = MP_StadiumAi_ShouldSwitch(1, 0, 3);
    MPNet_Log("stadium ai test: GOLEM vs PIKACHU: %s", sw ? "switches" : "stays");
    // 3. a later turn, STARMIE out against PIKACHU: faster and hitting hard, it stays and fights
    SetBattleMon(1, &gEnemyParty[1]);
    gBattlerPartyIndexes[1] = 1;
    gBattleStruct->AI_monToSwitchIntoId[1] = PARTY_SIZE;
    gBattleResults.battleTurnCounter = 9;
    sw = MP_StadiumAi_ShouldSwitch(1, 0, 3);
    MPNet_Log("stadium ai test: STARMIE vs PIKACHU: %s %d", sw ? "switch to" : "stays", sw ? gBattleStruct->AI_monToSwitchIntoId[1] : -1);
    // 4. GYARADOS fainted: GOLEM comes in, not STARMIE
    SetMonData(&gEnemyParty[0], MON_DATA_HP, &(u16){0});
    gBattlerPartyIndexes[1] = 0;
    MPNet_Log("stadium ai test: after a faint, sends %d", MP_StadiumAi_Replacement(1, 0, 3));
    if (!hadStruct)
    {
        Free(gBattleStruct);
        gBattleStruct = NULL;
    }
    gMpStadiumBattle = wasStadium;
    gBattlersCount = savedCount;
    memcpy(gBattlerPositions, savedPos, sizeof(savedPos));
    memcpy(gBattlerPartyIndexes, savedIdx, sizeof(savedIdx));
}
