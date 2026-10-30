#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <stdio.h>
#include <string.h>
enum
{
    FX_PARALYZE_COIN,
    FX_POISON,
    FX_POISON_COIN,
    FX_CONFUSE_COIN,
    FX_SLEEP,
    FX_SLEEP_COIN,
    FX_DRAIN,
    FX_BARRIER_COIN,
    FX_BONUS_COIN,
    FX_RAGE,
    FX_MEDITATE,
    FX_PSYWAVE,
    FX_PSYCHIC,
    FX_KARATE_CHOP,
    FX_SUPER_FANG,
    FX_MULTI_COIN,
    FX_DO_THE_WAVE,
    FX_WATER_EXTRA,
    /* Whole command lists (effect_functions.asm), see generate_effects.py. */
    FX_DISCARD_TYPED,
    FX_FLAMES_OF_RAGE,
    FX_RECOVER,
    FX_BARRIER_ALL,
    FX_RECOIL,
    FX_RECOIL_COIN,
    FX_THRASH,
    FX_FAIL_COIN,
    FX_CLAMP,
    FX_FLY,
    FX_DRAW,
    FX_HEAL_SELF,
    FX_DRAIN_DONE,
    FX_CONFUSE_BOTH,
    FX_PETAL_DANCE,
    FX_FOUL_GAS,
    FX_VENOM_POWDER,
    FX_TANTRUM,
    FX_RAMPAGE,
    FX_FLAIL,
    FX_EGGSPLOSION,
    FX_UNTIL_TAILS,
    FX_BOYFRIENDS,
    FX_NO_WR,
    FX_DREAM_EATER,
    FX_SELFDESTRUCT,
    FX_EARTHQUAKE,
    FX_BLIZZARD,
    FX_THUNDERSTORM,
    FX_CHAIN_LIGHTNING,
    FX_RANDOM_OPP,
    FX_BIG_THUNDER,
    FX_CHOSEN_BENCH,
    FX_GIGASHOCK,
    FX_LURE,
    FX_TELEPORT,
    FX_OPP_SWITCH,
    FX_GUARD,
    FX_WEAKEN,
    FX_NO_ATTACK_COIN,
    FX_NO_RETREAT_COIN,
    FX_HEADACHE,
    FX_CHARGE,
    FX_CALL_FAMILY,
    FX_FRIENDSHIP_SONG,
    FX_ENERGY_CONVERSION,
    FX_ENERGY_ABSORPTION,
    FX_ENERGY_SPIKE,
    FX_HYPER_BEAM,
    /* (1.46.0: the last twenty attacks, from effect_functions.asm) */
    FX_MIX_UP,
    FX_WILDFIRE,
    FX_AMNESIA,
    FX_MAGNETIC_STORM,
    FX_WAIL,
    FX_SCAVENGE,
    FX_DESTINY_BOND,
    FX_PROPHECY,
    FX_MYSTERY_ATTACK,
    FX_DEVOLUTION_BEAM,
    FX_MIRROR_MOVE,
    FX_GALE,
    FX_HURRICANE,
    FX_METRONOME,
    FX_MORPH,
    FX_CONVERSION_1,
    FX_CONVERSION_2,
    /* Pokemon Card GB2's attacks (tools/tcg/gb2_effects.py: amount, extra as noted there) */
    FX_STATUS_HT,         /* a coin: amount on heads, extra on tails */
    FX_STATUS_OR_RECOIL,  /* a coin: extra on heads, amount to itself on tails */
    FX_RANDOM_HITS,       /* extra times: amount to one of their Pokemon at random (no W/R) */
    FX_HORRID_POLLEN,     /* Poisoned and, at random, Confused / Asleep / Paralyzed */
    FX_DEADLY_POISON,     /* Poisoned; amount more if it already was */
    FX_CHOSEN_ANY,        /* amount to the chosen one of their Pokemon (target 0-5); extra CA_* */
    FX_POISON_BENCH,      /* Poisoned, amount to each of their Benched Pokemon */
    FX_NAME_COUNT,        /* amount more per Pokemon of the group extra (NG_*) on its Bench (or in play: NG_IN_PLAY) */
    FX_FAIL_RECOIL,       /* tails: nothing, and amount to itself */
    FX_PETAL_WHIRL,       /* amount coins x extra; 2 or more heads: Confused after */
    FX_DISCARD_FOE_ENERGY,/* (extra 1: on heads) an Energy card of the Defending Pokemon discarded */
    FX_REGENERATION,      /* every damage counter off; its Evolution and Energy cards discarded */
    FX_BONUS_STATUS,      /* heads: amount more and extra (ST_*) */
    FX_GAS_EXPLOSION,     /* amount to itself; a Knock Out: their next Active is Poisoned */
    FX_MASS_EXPLOSION,    /* amount x the Koffing / Weezing in play, then amount to each of them */
    FX_LOCK_SELF,         /* its next turn: extra 0 this attack, 3 no attack or retreat */
    FX_FIREBALL,          /* needs Fire Energy; heads: one discarded (and amount more); tails: nothing (amount 0) */
    FX_CONT_FIREBALL,     /* a coin per Fire Energy: amount each heads, as many Fire Energy discarded */
    FX_SWITCH_HIT,        /* before the damage: target comes out (extra 1: must, and needs a Bench) */
    FX_LURE_COIN,         /* heads: target comes out (needs a Bench) */
    FX_SKIP_DRAW,         /* they flip: tails, no draw next turn */
    FX_SEARCH_TRAINER,    /* heads: selections[0], a Trainer from the deck, to the hand */
    FX_TAILS_DISCARD_SELF,/* tails: one of its Energy cards discarded */
    FX_FLAME_INFERNO,     /* option Fire Energy discarded (up to extra): amount more each */
    FX_FLAME_PILLAR,      /* option 1: a Fire Energy discarded, amount to target on their Bench */
    FX_BURNING_FIRE,      /* option Fire Energy discarded from its side: amount more each */
    FX_DRY_UP,            /* target: until tails, a Water Energy discarded per heads */
    FX_MIRROR_SHELL,      /* damaged next turn: the same back at the attacker */
    FX_WATER_EXTRA2,      /* amount more per Energy past the cost (extra: up to n; + 16: any type, else Water) */
    FX_RECOIL_BARRIER,    /* amount to itself; heads: no damage to it next turn */
    FX_THIRD_EYE,         /* an Energy card of its discarded: draw amount */
    FX_REST,              /* healed, cured, Asleep */
    FX_ENERGY_SEARCH,     /* (amount + 128: on heads) up to amount Energy of type extra from the deck onto it */
    FX_WATER_BOMB,        /* amount per Water Energy past the cost (up to 2) to target on their Bench */
    FX_STRANGE_BEAM,      /* their next turn: its damaged Water Pokemon have no Weakness */
    FX_SPIN_SHOWER,       /* heads: 1 or 2 of theirs at random, amount each and an Energy card discarded */
    FX_RAPID_EVOLUTION,   /* the card from the deck that evolves from it, onto it */
    FX_FAIL_COINS,        /* amount coins: any tails, nothing */
    FX_TENTACLE_GRIP,     /* a coin per Water Energy: draw amount per heads */
    FX_ACID_LOCK,         /* heads: Paralyzed; tails: this attack not next turn */
    FX_SHORT_CIRCUIT,     /* target: amount per Water Energy on it */
    FX_SURPRISE_THUNDER,  /* heads, then heads 20 / tails 10 to each of their Benched Pokemon */
    FX_MICROWAVE,         /* target: amount, heads one of its Energy cards discarded */
    FX_MAGNETIC_LINES,    /* a basic Energy of the Defending Pokemon onto target on their Bench (extra 1: on heads) */
    FX_ENERGY_BOMB,       /* its Energy cards onto target on its Bench (none: discarded) */
    FX_RAGING_THUNDER,    /* tails: amount to target, one of its own */
    FX_SWIFT,             /* no Weakness, Resistance, powers or effects on the Defending Pokemon */
    FX_ROLLING_NEEDLE,    /* amount coins: extra more each heads, and extra to itself */
    FX_THEIR_COIN_BONUS,  /* they flip: tails, amount more */
    FX_SHUFFLE_THEIRS,    /* their deck shuffled */
    FX_CONFUSE_SELF,      /* Confused after */
    FX_FOCUSED_SHOT,      /* heads: next turn the other attack doubled; tails: not next turn */
    FX_FOCUS_BLAST,       /* heads: amount to target (W/R apply) */
    FX_SEETHING,          /* a coin per damage counter: amount more each heads */
    FX_FLING_DECK,        /* the Defending Pokemon and its cards shuffled into their deck (needs a Bench) */
    FX_BOULDER,           /* amount coins: a damage counter per heads on target on their Bench */
    FX_ROCK_BLAST,        /* option Fighting Energy discarded (up to 5): amount each to target */
    FX_BONE_TOSS,         /* heads: amount to it; tails: extra to target on their Bench */
    FX_BONE_HEADBUTT,     /* 0 / 10 / 20 to one on their Bench at random, then one of them in at random */
    FX_COIN_BENCH,        /* heads: amount to target on their Bench */
    FX_MOUNTAIN_BREAK,    /* the top 5 of its deck discarded; Fighting Energy among them to the hand */
    FX_VANISH,            /* it and its cards shuffled into its deck (needs a Bench) */
    FX_TYPE_DOUBLE,       /* base damage doubled against type extra */
    FX_TRANS_DAMAGE,      /* its Energy discarded; its damage counters onto the Defending Pokemon */
    FX_BOTH_ASLEEP,       /* both Asleep */
    FX_ONLY_ASLEEP,       /* only while it's Asleep */
    FX_GRUDGE,            /* amount more per Prize they've taken */
    FX_POLTERGEIST,       /* amount per Trainer in their hand */
    FX_BOTHER,            /* heads: a Trainer from their hand at random into their deck */
    FX_PUSH_ASIDE,        /* heads: one of theirs at random and its cards into their deck */
    FX_BENCH_MANIP,       /* they flip a coin per Benched Pokemon: amount per tails, no W/R */
    FX_PSYCRUSH,          /* amount per Psychic Energy card on their Pokemon */
    FX_COMPLETE_RECOVERY, /* its Energy discarded, healed, no longer Poisoned or Confused */
    FX_DARK_AMP,          /* a coin per Dark Pokemon on its Bench: amount more each heads */
    FX_TWISTER,           /* unless Knocked Out, heads: Trainer and Energy cards on it back to their hand */
    FX_DARK_SONG,         /* heads: Asleep, amount to each of their Benched non-Dark Pokemon */
    FX_KICK_STAMP,        /* heads: amount more; tails: they switch */
    FX_ROLL_OVER,         /* it's Asleep; heads: the Defending Pokemon too */
    FX_EAT,               /* a Food counter (up to 2) */
    FX_ROLLOUT,           /* option Food counters off: amount more each */
    FX_COIN_PLUS,         /* amount coins: extra more each heads */
    FX_STIR_UP,           /* they switch (they choose), then it may switch with target on its Bench */
    FX_NOTHING            /* (an attack without an effect of its own: Corkscrew Punch) */
};
/* GB2's Special Conditions (FX_STATUS_HT ...) */
enum { ST_NONE, ST_PAR, ST_PSN, ST_CNF, ST_SLP };
/* FX_CHOSEN_ANY's extra: CA_POWER_OFF its Pokemon Power off until their turn ends (Stare), CA_COIN heads only */
enum { CA_POWER_OFF = 1, CA_COIN = 2 };
/* FX_NAME_COUNT's groups */
enum { NG_NIDORAN = 1, NG_MAGNEMITE = 2, NG_VOLTORB = 3, NG_IN_PLAY = 16 };
#define BOTH_PHASES 2
#define ANY_TYPE 7
struct Effect
{
    const char *name;
    unsigned kind, amount, after, extra;
};
static const struct Effect sEffects[] = {
#include "tcg_effect_table.inc"
};
static const struct Effect *Effect(const char *name)
{
    unsigned i;
    for (i = 0; i < sizeof(sEffects) / sizeof(sEffects[0]); i++)
        if (!strcmp(name, sEffects[i].name))
            return &sEffects[i];
    return NULL;
}
int TcgDuel_EffectSupported(const char *name) { return !strcmp(name, "NONE") || Effect(name) != NULL; }

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */
static const struct TcgCard *CardAt(const struct TcgDuel *g, unsigned who, unsigned physical)
{
    return Tcg_Card(g->players[who].cards[physical]);
}
static unsigned BenchCount(const struct TcgDuel *g, unsigned who)
{
    unsigned s, n = 0;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        n += g->players[who].field[s].stackSize != 0;
    return n;
}
static unsigned HpLeft(const struct TcgDuel *g, unsigned who, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, who, slot);
    unsigned hp = !c ? 0 : c->kind == TCG_TRAINER ? 10 : c->hp, damage = g->players[who].field[slot].damage;
    return hp > damage ? hp - damage : 0;
}
/* The original lets the player pick which cards pay these costs; every
 * card that qualifies is an identical basic Energy except for "any Energy"
 * costs, where basic Energy goes before Double Colorless. */
static int DiscardEnergy(struct TcgDuel *g, unsigned who, unsigned slot, unsigned type, unsigned count)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned pass, i, done = 0, all = !count;
    for (pass = 0; pass < 2; pass++)
        for (i = f->attachedSize; i > 0 && (all || done < count); i--)
        {
            const struct TcgCard *c = CardAt(g, who, f->attached[i - 1]);
            int basic = c->id <= 6;
            if (c->kind != TCG_ENERGY || (pass == 0 && !basic) || (pass == 1 && basic))
                continue;
            if (type != ANY_TYPE && (c->type != type || !basic))
                continue;
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
            done++;
        }
    return all || done == count;
}
static void Heal(struct TcgDuel *g, unsigned who, unsigned slot, unsigned amount)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    if (amount > f->damage)
        amount = f->damage;
    if (!amount)
        return;
    f->damage -= amount;
    TcgDuel_Event(g, TCG_EV_HEAL, who, slot, amount, 0, 0);
}
static void Switch(struct TcgDuel *g, unsigned who, unsigned slot)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard temp = p->field[0];
    p->field[0] = p->field[slot];
    p->field[slot] = temp;
    /* A Pokemon moved to the Bench loses its conditions and substatus. */
    p->field[slot].condition = TCG_HEALTHY;
    p->field[slot].poison = 0;
    p->field[slot].barrier = 0;
    p->field[slot].noRetreat = 0;
    p->field[slot].noAttack = 0;
    p->field[slot].guard = 0;
    p->field[slot].weaken = 0;
    p->field[slot].charged = 0;
    p->field[slot].effectBy = 0;
    p->field[slot].burn = p->field[slot].shift = 0;
    p->field[slot].disabled = p->field[slot].destinyBond = p->field[slot].weakAs = p->field[slot].resistAs = 0;
    p->field[slot].mirrorDamage = 0;
    p->field[slot].mirrorCondition = p->field[slot].mirrorPoison = p->field[slot].mirrorWeaken = 0;
    p->field[slot].mirrorNoAttack = p->field[slot].mirrorNoRetreat = p->field[slot].mirrorEffect = 0;
    p->field[slot].mirrorArg = p->field[slot].mirrorWeak = 0;
    p->field[slot].lockTurn = p->field[slot].lockAttack = p->field[slot].mirrorShell = p->field[slot].charge3 = 0;
    TcgDuel_Event(g, TCG_EV_SWITCH, who, slot, 0, 0, 0);
}
static void ToBench(struct TcgDuel *g, unsigned who, unsigned physical)
{
    struct TcgPlayer *p = &g->players[who];
    unsigned s;
    int at = -1;
    for (s = 0; s < p->deckSize; s++)
        if (p->deck[s] == physical)
            at = (int)s;
    for (s = 1; s < TCG_FIELD_SIZE && at >= 0; s++)
        if (!p->field[s].stackSize)
        {
            struct TcgFieldCard *f = &p->field[s];
            memset(f, 0, sizeof(*f));
            f->stack[f->stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
            f->enteredTurn = g->turn;
            TcgDuel_Event(g, TCG_EV_PLAY, who, s, 0, p->cards[physical], 0);
            TcgPower_OnPlay(g, who, s);
            return;
        }
}
static int SameName(const struct TcgCard *c, const char *name) { return c && !strcmp(c->name, name); }
static unsigned EnergyCards(const struct TcgDuel *g, unsigned who, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
        n += CardAt(g, who, f->attached[i])->kind == TCG_ENERGY;
    return n;
}
static void SetStatus(struct TcgFieldCard *f, unsigned condition) { f->condition = condition; }

/* Damage from an effect: no Weakness/Resistance; Defender everywhere; for an
 * Active Pokemon, the attacker's PlusPower and the Active's own protection
 * (duel.asm DealDamageToPlayAreaPokemon). */
void TcgDuel_EffectDamage(struct TcgDuel *g, unsigned player, unsigned slot, int amount, unsigned flags)
{
    struct TcgPlayer *p = &g->players[player];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    if (!f->stackSize || amount <= 0)
        return;
    /* (the Defending Pokemon's protection was settled as the attack began; Mew's
     * Neutralizing Shield and Haunter's Transparency cover the Bench too) */
    if (!slot && player != g->current)
    {
        if (g->foeImmune == TCG_IMMUNE_UNDECIDED)
            g->foeImmune = TcgPower_Immune(g, player, slot) ? TCG_IMMUNE_YES : TCG_IMMUNE_NO;
        if (g->foeImmune == TCG_IMMUNE_YES)
            return;
    }
    else if (TcgPower_Immune(g, player, slot))
        return;
    if (!slot && player != g->current)
    {
        if (f->barrier)
            amount = 0;
        for (i = 0; i < g->players[g->current].field[0].attachedSize; i++)
            amount += 10 * (g->players[g->current].cards[g->players[g->current].field[0].attached[i]] ==
                            TCG_CARD_PLUSPOWER);
    }
    for (i = 0; i < f->attachedSize; i++)
        amount -= 20 * (p->cards[f->attached[i]] == TCG_CARD_DEFENDER);
    amount = TcgPower_AttackDamage(g, player, slot, amount, 0);
    if (amount <= 0)
        return;
    f->damage += (unsigned)amount;
    TcgDuel_Event(g, TCG_EV_DAMAGE, player, slot, (unsigned)amount, 0, flags);
}

/* ------------------------------------------------------------------ */
/* Pokemon Card GB2's helpers                                           */
/* ------------------------------------------------------------------ */
static void SetSt(struct TcgFieldCard *f, unsigned st)
{
    if (st == ST_PAR)
        f->condition = TCG_PARALYZED;
    else if (st == ST_CNF)
        f->condition = TCG_CONFUSED;
    else if (st == ST_SLP)
        f->condition = TCG_ASLEEP;
    else if (st == ST_PSN && f->poison < 10)
        f->poison = 10;
}
/* amount to one Pokemon from an attack's effect: no Weakness / Resistance unless wr (then the attacker's type's) */
static void HitSlot(struct TcgDuel *g, unsigned who, unsigned slot, int amount, int wr)
{
    if (!TcgDuel_Field(g, who, slot) || amount <= 0)
        return;
    if (wr)
    {
        unsigned type = TcgDuel_Type(g, who ^ 1, 0);
        if (TcgDuel_Weakness(g, who, slot) == type)
            amount *= 2;
        if (TcgDuel_Resistance(g, who, slot) == type)
            amount -= 30;
        if (amount <= 0)
            return;
    }
    TcgDuel_EffectDamage(g, who, slot, amount, 0);
}
/* An Energy card attached to that Pokemon discarded: a basic one of type (ANY_TYPE: any card, Double Colorless last
 * unless pickBest: then first, as the attacker would) */
static int DiscardOne(struct TcgDuel *g, unsigned who, unsigned slot, unsigned type, int pickBest)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    int pick = -1;
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = CardAt(g, who, f->attached[i]);
        if (c->kind != TCG_ENERGY || (type != ANY_TYPE && (c->id > 6 || c->type != type)))
            continue;
        if (pick < 0 || (pickBest ? c->id > 6 : CardAt(g, who, f->attached[pick])->id > 6))
            pick = (int)i;
    }
    if (pick < 0)
        return 0;
    p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, (unsigned)pick);
    return 1;
}
static unsigned EnergyOfType(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned type)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = CardAt(g, who, f->attached[i]);
        n += c->kind == TCG_ENERGY && c->id <= 6 && c->type == type;
    }
    return n;
}
static int IsDark(const struct TcgCard *c) { return c && c->kind == TCG_POKEMON && (c->dark || !strncmp(c->name, "Dark ", 5)); }
/* Every card of that Pokemon into its owner's deck, shuffled (Fling, Vanish, Push Aside) */
static void SlotToDeck(struct TcgDuel *g, unsigned who, unsigned slot)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->stackSize; i++)
        p->deck[p->deckSize++] = f->stack[i];
    for (i = 0; i < f->attachedSize; i++)
        p->deck[p->deckSize++] = f->attached[i];
    memset(f, 0, sizeof(*f));
    TcgDuel_Shuffle(g, who);
    TcgDuel_Event(g, TCG_EV_DECK_CHANGED, who, 0, 0, 0, 0);
}
/* The target an attack names, if it's one of that side's Pokemon (bench: 1-5 only) */
static int ValidTarget(const struct TcgDuel *g, unsigned who, unsigned target, int bench)
{
    return target < TCG_FIELD_SIZE && (!bench || target >= 1) && g->players[who].field[target].stackSize;
}
static unsigned Coins(struct TcgDuel *g, unsigned who, unsigned n)
{
    unsigned i, heads = 0;
    for (i = 0; i < n && i < 60; i++)
        heads += TcgDuel_Coin(g, who);
    return heads;
}
/* Recoil: the attacker's own Weakness and Resistance to its own type apply
 * (ApplyDamageModifiers_DamageToSelf). */
void TcgDuel_SelfDamage(struct TcgDuel *g, int amount)
{
    const struct TcgCard *c = TcgDuel_Field(g, g->current, 0);
    struct TcgFieldCard *f = &g->players[g->current].field[0];
    unsigned type = TcgDuel_Type(g, g->current, 0);
    if (!c || amount <= 0)
        return;
    if (TcgDuel_Weakness(g, g->current, 0) == type)
        amount *= 2;
    if (TcgDuel_Resistance(g, g->current, 0) == type)
        amount -= 30;
    amount = TcgPower_AttackDamage(g, g->current, 0, amount, 0);
    if (amount <= 0)
        return;
    f->damage += (unsigned)amount;
    TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, (unsigned)amount, 0, TCG_DMG_SELF);
}

/* ------------------------------------------------------------------ */
/* Can the attack be used, and does it need a choice?                   */
/* ------------------------------------------------------------------ */
unsigned TcgDuel_AttackChoice(const struct TcgAttack *attack, int *required)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    if (required)
        *required = 0;
    if (!e)
        return 0;
    switch (e->kind)
    {
    case FX_CHOSEN_BENCH:
        if (required)
            *required = e->extra != 0;
        return 1;
    case FX_LURE:
        if (required)
            *required = 1;
        return 1;
    case FX_TELEPORT:
        if (required)
            *required = !e->extra;
        return 2;
    /* GB2's */
    case FX_SWITCH_HIT:
        if (required)
            *required = e->extra != 0;
        return 1;
    case FX_LURE_COIN:
        if (required)
            *required = 1;
        return 1;
    case FX_FLAME_PILLAR: case FX_WATER_BOMB: case FX_MAGNETIC_LINES: case FX_BOULDER: case FX_BONE_TOSS:
    case FX_COIN_BENCH:
        return 1;
    case FX_ENERGY_BOMB: case FX_STIR_UP:
        return 2;
    case FX_CHOSEN_ANY: case FX_DRY_UP: case FX_SHORT_CIRCUIT: case FX_MICROWAVE: case FX_FOCUS_BLAST:
    case FX_ROCK_BLAST:
        if (required)
            *required = 1;
        return 3;
    case FX_RAGING_THUNDER:
        if (required)
            *required = 1;
        return 4;
    }
    return 0;
}
const char *TcgDuel_AttackTargetPrompt(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    if (!e)
        return NULL;
    switch (e->kind)
    {
    case FX_SWITCH_HIT: return e->extra ? "Drag out which of their Benched Pokemon?" : "Switch in which of theirs? (B: none)";
    case FX_LURE_COIN: return "If heads, bring out which of theirs?";
    case FX_FLAME_PILLAR: return "Burn which of their Benched Pokemon?";
    case FX_WATER_BOMB: return "Splash which of their Benched Pokemon?";
    case FX_MAGNETIC_LINES: return "Move their Energy to which Benched Pokemon?";
    case FX_BOULDER: return "If heads, which Benched Pokemon gets the counters?";
    case FX_BONE_TOSS: return "If tails, hit which Benched Pokemon?";
    case FX_COIN_BENCH: return "If heads, hit which Benched Pokemon?";
    case FX_ENERGY_BOMB: return "Move its Energy to which Benched Pokemon?";
    case FX_STIR_UP: return "Then switch with which of yours? (B: stay)";
    case FX_TELEPORT: return e->extra ? "Switch with which of yours? (B: stay)" : NULL;
    case FX_RAGING_THUNDER: return "If tails, which of yours takes 30?";
    case FX_DRY_UP: return "Dry up which of their Pokemon?";
    case FX_FOCUS_BLAST: return "If heads, hit which of their Pokemon?";
    case FX_ROCK_BLAST: return "Throw the rocks at which Pokemon?";
    case FX_CHOSEN_ANY: case FX_SHORT_CIRCUIT: case FX_MICROWAVE: return "Aim at which of their Pokemon?";
    }
    return NULL;
}
int TcgDuel_SleepAttack(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    return e && e->kind == FX_ONLY_ASLEEP;
}
int TcgDuel_MetronomeHeads(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    return e && e->kind == FX_METRONOME && e->extra;
}
int TcgDuel_IsMetronome(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    return e && e->kind == FX_METRONOME;
}
unsigned TcgDuel_AttackAsk(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    if (!e)
        return TCG_ASK_NONE;
    switch (e->kind)
    {
    case FX_SCAVENGE:
        return TCG_ASK_DISCARD_TRAINER;
    case FX_PROPHECY:
        return TCG_ASK_ORDER_TOP3;
    case FX_DEVOLUTION_BEAM:
        return TCG_ASK_EVOLVED;
    case FX_SEARCH_TRAINER:
        return TCG_ASK_DECK_TRAINER;
    }
    return TCG_ASK_NONE;
}
static int RealAttack(const struct TcgAttack *a) { return a && a->name[0] && a->category != TCG_POKEMON_POWER; }
const struct TcgAttack *TcgDuel_MetronomeAttack(const struct TcgDuel *g, unsigned player, unsigned copy)
{
    const struct TcgCard *foe = TcgDuel_Field(g, player ^ 1, 0);
    const struct TcgAttack *a;
    if (!foe || foe->kind != TCG_POKEMON || copy < 1 || copy > 2)
        return NULL;
    a = &foe->attacks[copy - 1];
    /* (never Metronome with Metronome) */
    if (!RealAttack(a) || TcgDuel_IsMetronome(a) || !TcgDuel_EffectSupported(a->effect))
        return NULL;
    return a;
}
static unsigned FireEnergyCards(const struct TcgDuel *g, unsigned who, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
        n += g->players[who].cards[f->attached[i]] == TCG_CARD_FIRE_ENERGY;
    return n;
}
static int HasEvolved(const struct TcgDuel *g, unsigned who)
{
    unsigned s;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (g->players[who].field[s].stackSize > 1)
            return 1;
    return 0;
}
static const char *const sTypeLabels[TCG_COLORLESS] = {"FIRE", "GRASS", "LIGHTNING", "WATER", "FIGHTING", "PSYCHIC"};
unsigned TcgDuel_AttackOptions(const struct TcgDuel *g, unsigned player, const struct TcgAttack *attack,
                               const char *labels[TCG_MAX_OPTIONS], uint8_t ok[TCG_MAX_OPTIONS])
{
    static const char *const sCounts[TCG_MAX_OPTIONS] = {"NONE", "1", "2", "3", "4", "5", "6", "7"};
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    const struct TcgCard *foe = TcgDuel_Field(g, player ^ 1, 0);
    unsigned i, n = 0;
    if (!e)
        return 0;
    switch (e->kind)
    {
    case FX_AMNESIA:
    case FX_METRONOME:
        /* the Defending Pokemon's attacks */
        if (!foe || foe->kind != TCG_POKEMON)
            return 0;
        for (i = 0; i < 2; i++)
        {
            labels[i] = foe->attacks[i].name[0] ? foe->attacks[i].name : "---";
            ok[i] = e->kind == FX_AMNESIA ? RealAttack(&foe->attacks[i])
                                         : TcgDuel_MetronomeAttack(g, player, i + 1) != NULL;
        }
        return 2;
    case FX_MIRROR_MOVE:
        /* Amnesia used on it: which of the Defending Pokemon's attacks to give it back to */
        if (g->players[player].field[0].mirrorEffect != TCG_MIRROR_AMNESIA || !foe || foe->kind != TCG_POKEMON)
            return 0;
        for (i = 0; i < 2; i++)
        {
            labels[i] = foe->attacks[i].name[0] ? foe->attacks[i].name : "---";
            ok[i] = RealAttack(&foe->attacks[i]);
        }
        return 2;
    case FX_WILDFIRE:
        n = FireEnergyCards(g, player, 0);
        for (i = 0; i <= n && i < TCG_MAX_OPTIONS; i++)
            labels[i] = sCounts[i], ok[i] = 1;
        return i;
    case FX_CONVERSION_1:
    case FX_CONVERSION_2:
        for (i = 0; i < TCG_COLORLESS; i++)
            labels[i] = sTypeLabels[i], ok[i] = 1;
        return TCG_COLORLESS;
    case FX_PROPHECY:
        labels[0] = "YOUR DECK", ok[0] = g->players[player].deckSize != 0;
        labels[1] = "THEIR DECK", ok[1] = g->players[player ^ 1].deckSize != 0;
        return 2;
    case FX_DEVOLUTION_BEAM:
        labels[0] = "YOUR POKEMON", ok[0] = (uint8_t)HasEvolved(g, player);
        labels[1] = "THEIR POKEMON", ok[1] = (uint8_t)HasEvolved(g, player ^ 1);
        return 2;
    /* GB2's: how many to discard / remove */
    case FX_FLAME_INFERNO: case FX_FLAME_PILLAR: case FX_BURNING_FIRE: case FX_ROCK_BLAST: case FX_ROLLOUT:
        if (e->kind == FX_FLAME_INFERNO)
            n = FireEnergyCards(g, player, 0), n = n > e->extra ? e->extra : n;
        else if (e->kind == FX_FLAME_PILLAR)
            n = FireEnergyCards(g, player, 0) ? 1 : 0;
        else if (e->kind == FX_BURNING_FIRE)
            for (i = 0; i < TCG_FIELD_SIZE; i++)
                n += FireEnergyCards(g, player, i);
        else if (e->kind == FX_ROCK_BLAST)
            n = EnergyOfType(g, player, 0, TCG_FIGHTING), n = n > 5 ? 5 : n;
        else
            n = g->players[player].field[0].food;
        for (i = 0; i <= n && i < TCG_MAX_OPTIONS; i++)
            labels[i] = sCounts[i], ok[i] = 1;
        return i;
    }
    return 0;
}
unsigned TcgDuel_Weakness(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    if (!c)
        return TCG_NONE;
    /* Staryu's Strange Beam: that side's damaged Water Pokemon have no Weakness this turn */
    if (g->players[player].strangeBeamUntil == g->turn && f->damage && c->type == TCG_WATER)
        return TCG_NONE;
    return f->weakAs && c->weakness != TCG_NONE ? f->weakAs - 1u : c->weakness;
}
unsigned TcgDuel_Resistance(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    const struct TcgFieldCard *f = &g->players[player].field[slot];
    if (!c)
        return TCG_NONE;
    return f->resistAs && c->resistance != TCG_NONE ? f->resistAs - 1u : c->resistance;
}
/* The AI's choices. The original's AI never picks Prophecy or Metronome's attack ("does nothing for AI"); it
 * is given sensible ones here so its Clefairy can still attack. */
void TcgDuel_AiAttackChoices(const struct TcgDuel *g, struct TcgAction *a)
{
    const struct TcgCard *self = TcgDuel_Field(g, g->current, 0), *foe = TcgDuel_Field(g, g->current ^ 1, 0);
    const struct TcgAttack *attack = self ? &self->attacks[a->attack & 1] : NULL;
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    const struct TcgPlayer *p = &g->players[g->current];
    unsigned i, s;
    if (!e)
        return;
    switch (e->kind)
    {
    case FX_METRONOME:
    {
        unsigned best = 0, bestDamage = 0;
        for (i = 1; i <= 2; i++)
        {
            const struct TcgAttack *c = TcgDuel_MetronomeAttack(g, g->current, i);
            if (c && (!best || c->damage > bestDamage))
                best = i, bestDamage = c->damage;
        }
        a->copy = (uint8_t)best;
        break;
    }
    case FX_AMNESIA:
    case FX_MIRROR_MOVE:
        /* AIPickAttackForAmnesia: the second attack if it has the Energy for it, else the first real one */
        if (foe)
            a->option = (uint8_t)(RealAttack(&foe->attacks[1]) && TcgDuel_CanAttack(g, g->current ^ 1, 1) ? 1
                                  : RealAttack(&foe->attacks[0])                                       ? 0
                                                                                                        : 1);
        break;
    case FX_WILDFIRE:
        a->option = 0; /* (the original's AI always discards none) */
        break;
    case FX_CONVERSION_1:
        /* its own Pokemon's type, so the Defending Pokemon becomes weak to them */
        a->option = TCG_FIRE;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            unsigned t = TcgDuel_Field(g, g->current, s) ? TcgDuel_Type(g, g->current, s) : TCG_COLORLESS;
            if (t < TCG_COLORLESS)
            {
                a->option = (uint8_t)t;
                break;
            }
        }
        break;
    case FX_CONVERSION_2:
        /* the Defending Pokemon's type unless it's Colorless */
        a->option = foe && TcgDuel_Type(g, g->current ^ 1, 0) < TCG_COLORLESS ? (uint8_t)TcgDuel_Type(g, g->current ^ 1, 0)
                                                                              : TCG_FIRE;
        break;
    case FX_PROPHECY:
        a->option = 0;
        a->selectionCount = 0; /* (looks, changes nothing) */
        break;
    case FX_DEVOLUTION_BEAM:
        /* theirs first, else its own */
        for (i = 0; i < 2; i++)
        {
            unsigned who = g->current ^ (i == 0);
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (g->players[who].field[s].stackSize > 1)
                {
                    a->option = (uint8_t)(i == 0);
                    a->target = (uint8_t)s;
                    return;
                }
        }
        break;
    case FX_SCAVENGE:
        for (i = 0; i < p->discardSize; i++)
            if (CardAt(g, g->current, p->discard[i])->kind == TCG_TRAINER)
            {
                a->selections[0] = p->discard[i];
                a->selectionCount = 1;
                break;
            }
        break;
    /* GB2's */
    case FX_SEARCH_TRAINER:
        for (i = 0; i < p->deckSize; i++)
            if (CardAt(g, g->current, p->deck[i])->kind == TCG_TRAINER)
            {
                a->selections[0] = p->deck[i];
                a->selectionCount = 1;
                break;
            }
        break;
    case FX_FLAME_INFERNO: case FX_ROCK_BLAST: case FX_ROLLOUT:
    {
        const char *labels[TCG_MAX_OPTIONS];
        uint8_t ok[TCG_MAX_OPTIONS];
        unsigned n = TcgDuel_AttackOptions(g, g->current, attack, labels, ok);
        a->option = (uint8_t)(n ? n - 1 : 0); /* (all it can) */
        break;
    }
    case FX_FLAME_PILLAR:
        a->option = (uint8_t)(BenchCount(g, g->current ^ 1) && FireEnergyCards(g, g->current, 0) > attack->cost[TCG_FIRE]);
        break;
    case FX_BURNING_FIRE:
    {
        /* only as many as a Knock Out needs */
        unsigned need = HpLeft(g, g->current ^ 1, 0), have = 0, base = attack->damage;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            have += FireEnergyCards(g, g->current, s);
        a->option = 0;
        for (i = 0; i <= have && i < TCG_MAX_OPTIONS; i++)
            if (base + e->amount * i >= need)
            {
                a->option = (uint8_t)i;
                break;
            }
        break;
    }
    }
    /* Their Pokemon or its own, for the GB2 attacks that aim anywhere */
    {
        unsigned choice = TcgDuel_AttackChoice(attack, NULL), who = g->current ^ (choice == 3), best = TCG_NONE;
        if (choice == 3 || choice == 4)
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (TcgDuel_Field(g, who, s) &&
                    (best == TCG_NONE || (choice == 3 ? HpLeft(g, who, s) < HpLeft(g, who, best)
                                                      : HpLeft(g, who, s) > HpLeft(g, who, best))))
                    best = s;
            /* (Dry Up and Short Circuit go where the Water Energy is) */
            if (choice == 3 && (e->kind == FX_DRY_UP || e->kind == FX_SHORT_CIRCUIT))
                for (s = 0, best = 0; s < TCG_FIELD_SIZE; s++)
                    if (EnergyOfType(g, who, s, TCG_WATER) > EnergyOfType(g, who, best, TCG_WATER))
                        best = s;
            a->target = (uint8_t)(best == TCG_NONE ? 0 : best);
        }
    }
}
static const char *const sFamily[][3] = {
    /* Call for Family: the names searched for, by command list. */
    {"NidoranFCallForFamilyEffectCommands", "Nidoran F", "Nidoran M"},
    {"BellsproutCallForFamilyEffectCommands", "Bellsprout", NULL},
    {"KrabbyCallForFamilyEffectCommands", "Krabby", NULL},
    {"OddishSproutEffectCommands", "Oddish", NULL},
    {"ParasScatterSporesEffectCommands", "Paras", NULL},
};
static int FamilyMatch(const struct Effect *e, const struct TcgCard *c)
{
    unsigned i;
    if (!c || c->kind != TCG_POKEMON || c->stage)
        return 0;
    if (e->extra)
        return c->type == TCG_FIGHTING;
    for (i = 0; i < sizeof(sFamily) / sizeof(sFamily[0]); i++)
        if (!strcmp(e->name, sFamily[i][0]))
            return SameName(c, sFamily[i][1]) || (sFamily[i][2] && SameName(c, sFamily[i][2]));
    return 0;
}
const char *TcgDuel_EffectBlock(const struct TcgDuel *g, unsigned player, const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    const struct TcgPlayer *p = &g->players[player];
    const struct TcgFieldCard *self = &p->field[0], *foe = &g->players[player ^ 1].field[0];
    unsigned i;
    if (!e)
        return NULL;
    switch (e->kind)
    {
    case FX_DREAM_EATER:
        return foe->condition == TCG_ASLEEP ? NULL : "The Defending Pokemon isn't Asleep.";
    case FX_RECOVER:
        return self->damage ? NULL : "There are no damage counters to remove.";
    case FX_HEAL_SELF:
        return self->damage ? NULL : "There are no damage counters to remove.";
    case FX_FAIL_COIN:
        return e->extra && self->usedOnce ? "This attack can't be used again." : NULL;
    case FX_CHOSEN_BENCH:
        return e->extra && !BenchCount(g, player ^ 1) ? "Your opponent has no Benched Pokemon." : NULL;
    case FX_LURE:
        return BenchCount(g, player ^ 1) ? NULL : "Your opponent has no Benched Pokemon.";
    case FX_TELEPORT:
        return e->extra || BenchCount(g, player) ? NULL : "You have no Benched Pokemon.";
    /* GB2's */
    case FX_LURE_COIN:
    case FX_FLING_DECK:
        return BenchCount(g, player ^ 1) ? NULL : "Your opponent has no Benched Pokemon.";
    case FX_SWITCH_HIT:
        return !e->extra || BenchCount(g, player ^ 1) ? NULL : "Your opponent has no Benched Pokemon.";
    case FX_VANISH:
        return BenchCount(g, player) ? NULL : "You have no Benched Pokemon.";
    case FX_FIREBALL:
        return FireEnergyCards(g, player, 0) ? NULL : "There's no Fire Energy card attached to it.";
    case FX_EAT:
        return self->food < 2 ? NULL : "It already has 2 Food counters.";
    case FX_ONLY_ASLEEP:
        return self->condition == TCG_ASLEEP ? NULL : "It can only use this attack while it's Asleep.";
    case FX_CALL_FAMILY:
    case FX_FRIENDSHIP_SONG:
        if (BenchCount(g, player) == TCG_FIELD_SIZE - 1)
            return "Your Bench is full.";
        return p->deckSize ? NULL : "Your deck is empty.";
    case FX_ENERGY_CONVERSION:
    case FX_ENERGY_ABSORPTION:
        for (i = 0; i < p->discardSize; i++)
            if (CardAt(g, player, p->discard[i])->kind == TCG_ENERGY)
                return NULL;
        return "There are no Energy cards in your discard pile.";
    case FX_ENERGY_SPIKE:
        return p->deckSize ? NULL : "Your deck is empty.";
    case FX_AMNESIA:
    {
        const struct TcgCard *c = TcgDuel_Field(g, player ^ 1, 0);
        return c && c->kind == TCG_POKEMON && (RealAttack(&c->attacks[0]) || RealAttack(&c->attacks[1]))
                   ? NULL
                   : "The Defending Pokemon has no attacks to choose.";
    }
    case FX_METRONOME:
        return TcgDuel_MetronomeAttack(g, player, 1) || TcgDuel_MetronomeAttack(g, player, 2)
                   ? NULL
                   : "There's no attack Metronome can copy.";
    case FX_WILDFIRE:
        return FireEnergyCards(g, player, 0) ? NULL : "Not enough Fire Energy.";
    case FX_WAIL:
        return BenchCount(g, player) < TCG_FIELD_SIZE - 1 || BenchCount(g, player ^ 1) < TCG_FIELD_SIZE - 1
                   ? NULL
                   : "There's no space on either Bench.";
    case FX_SCAVENGE:
        if (!TcgDuel_Energy(g, player, 0, TCG_PSYCHIC))
            return "Not enough Psychic Energy.";
        for (i = 0; i < p->discardSize; i++)
            if (CardAt(g, player, p->discard[i])->kind == TCG_TRAINER)
                return NULL;
        return "There are no Trainer cards in your discard pile.";
    case FX_DESTINY_BOND:
        return TcgDuel_Energy(g, player, 0, TCG_PSYCHIC) ? NULL : "Not enough Psychic Energy.";
    case FX_PROPHECY:
        return p->deckSize || g->players[player ^ 1].deckSize ? NULL : "There are no cards left in either deck.";
    case FX_DEVOLUTION_BEAM:
        return HasEvolved(g, player) || HasEvolved(g, player ^ 1) ? NULL : "There are no evolved Pokemon in play.";
    case FX_MIRROR_MOVE:
        return self->mirrorDamage || self->mirrorCondition || self->mirrorPoison || self->mirrorWeaken ||
                       self->mirrorNoAttack || self->mirrorNoRetreat || self->mirrorEffect || self->mirrorWeak
                   ? NULL
                   : "It wasn't attacked last turn.";
    case FX_CONVERSION_1:
    {
        const struct TcgCard *c = TcgDuel_Field(g, player ^ 1, 0);
        return c && c->weakness != TCG_NONE ? NULL : "The Defending Pokemon has no Weakness.";
    }
    case FX_CONVERSION_2:
    {
        const struct TcgCard *c = TcgDuel_Field(g, player, 0);
        return c && c->resistance != TCG_NONE ? NULL : "It has no Resistance.";
    }
    }
    (void)foe;
    return NULL;
}

/* ------------------------------------------------------------------ */
/* The effects                                                          */
/* ------------------------------------------------------------------ */
static void BenchDamage(struct TcgDuel *g, unsigned who, int amount)
{
    unsigned s;
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        TcgDuel_EffectDamage(g, who, s, amount, 0);
}
/* One of a player's Pokemon in play at random (PickRandomPlayAreaCard). */
static unsigned RandomSlot(struct TcgDuel *g, unsigned who, int skipActive)
{
    unsigned slots[TCG_FIELD_SIZE], n = 0, s;
    for (s = skipActive ? 1 : 0; s < TCG_FIELD_SIZE; s++)
        if (g->players[who].field[s].stackSize)
            slots[n++] = s;
    return n ? slots[Tcg_RandomBelow(&g->rng, n)] : TCG_NONE;
}
static int TakeEnergyFromDiscard(struct TcgDuel *g, unsigned max, int attach)
{
    struct TcgPlayer *p = &g->players[g->current];
    const struct TcgCard *self = TcgDuel_Field(g, g->current, 0);
    unsigned pass, i, n = 0;
    /* The original asks which cards; take the attacker's own type first. */
    for (pass = 0; pass < 2 && n < max; pass++)
        for (i = p->discardSize; i > 0 && n < max; i--)
        {
            const struct TcgCard *c = CardAt(g, g->current, p->discard[i - 1]);
            uint8_t card;
            if (c->kind != TCG_ENERGY || (pass == 0) != (self && c->type == self->type))
                continue;
            card = TcgPile_Remove(p->discard, &p->discardSize, i - 1);
            if (attach)
                p->field[0].attached[p->field[0].attachedSize++] = card;
            else
                p->hand[p->handSize++] = card;
            n++;
        }
    return (int)n;
}
static void ShuffleList(struct TcgDuel *g, uint8_t *list, unsigned n)
{
    unsigned i;
    for (i = n; i > 1; i--)
    {
        unsigned j = Tcg_RandomBelow(&g->rng, i);
        uint8_t t = list[i - 1];
        list[i - 1] = list[j];
        list[j] = t;
    }
}
/* Wail: Basic Pokemon at random from the deck until the Bench is full (Wail_FillBenchEffect .FillBench) */
static void FillBench(struct TcgDuel *g, unsigned who)
{
    struct TcgPlayer *p = &g->players[who];
    uint8_t list[TCG_DECK_SIZE];
    unsigned n = p->deckSize, i, s;
    memcpy(list, p->deck, n);
    ShuffleList(g, list, n);
    for (i = 0; i < n && BenchCount(g, who) < TCG_FIELD_SIZE - 1; i++)
    {
        const struct TcgCard *c = CardAt(g, who, list[i]);
        int at = -1;
        if (c->kind != TCG_POKEMON || c->stage)
            continue;
        for (s = 0; s < p->deckSize; s++)
            if (p->deck[s] == list[i])
                at = (int)s;
        for (s = 1; s < TCG_FIELD_SIZE && at >= 0; s++)
            if (!p->field[s].stackSize)
            {
                struct TcgFieldCard *f = &p->field[s];
                memset(f, 0, sizeof(*f));
                f->stack[f->stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
                f->enteredTurn = g->turn;
                TcgDuel_Event(g, TCG_EV_PLAY, who, s, 0, c->id, 0);
                /* (put into play during its owner's turn: Firegiver and the like) */
                if (who == g->current)
                    TcgPower_OnPlay(g, who, s);
                break;
            }
    }
    TcgDuel_Shuffle(g, who);
}
/* Devolution Beam: the top card back to the hand; its damage stays (a Knock Out if it's too much now) */
static void Devolve(struct TcgDuel *g, unsigned who, unsigned slot)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    if (f->stackSize < 2)
        return;
    p->hand[p->handSize++] = f->stack[--f->stackSize];
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    f->noAttack = f->noRetreat = f->weaken = f->guard = f->charged = f->effectBy = 0;
    f->disabled = f->weakAs = f->resistAs = 0;
    f->burn = f->shift = 0;
    TcgDuel_Event(g, TCG_EV_DEVOLVE, who, slot, 0, p->cards[f->stack[f->stackSize - 1]], 0);
    TcgDuel_Log(g, "%s devolved.", CardAt(g, who, f->stack[f->stackSize - 1])->name);
}
/* Amnesia on the Defending Pokemon (ApplyAmnesiaToAttack) */
static void Amnesia(struct TcgDuel *g, unsigned attack)
{
    struct TcgFieldCard *foe = &g->players[g->current ^ 1].field[0];
    const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
    if (!c || attack > 1 || g->foeImmune == TCG_IMMUNE_YES)
        return;
    foe->disabled = (uint8_t)(attack + 1);
    TcgDuel_Log(g, "%s can't use %s next turn.", c->name, c->attacks[attack].name);
}
/* ------------------------------------------------------------------ */
/* Pokemon Card GB2's attacks (tools/tcg/gb2_effects.py)                */
/* ------------------------------------------------------------------ */
/* The Energy an attack's cost doesn't use: Water (or any type: any) past the cost, like FX_WATER_EXTRA */
static unsigned EnergyPastCost(const struct TcgDuel *g, const struct TcgAttack *a, int any)
{
    unsigned total = TcgDuel_Energy(g, g->current, 0, TCG_NONE), cost = 0, t, water, other, need;
    for (t = 0; t < TCG_TYPES; t++)
        cost += a->cost[t];
    if (any)
        return total > cost ? total - cost : 0;
    water = TcgDuel_Energy(g, g->current, 0, TCG_WATER);
    other = total - water;
    need = a->cost[TCG_WATER] + (a->cost[TCG_COLORLESS] > other ? a->cost[TCG_COLORLESS] - other : 0);
    return water > need ? water - need : 0;
}
static unsigned FieldFireCards(const struct TcgDuel *g, unsigned who)
{
    unsigned s, n = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        n += FireEnergyCards(g, who, s);
    return n;
}
static unsigned NamedInPlay(const struct TcgDuel *g, unsigned who, int bench, const char *const *names)
{
    unsigned s, n = 0, k;
    for (s = bench ? 1 : 0; s < TCG_FIELD_SIZE; s++)
        for (k = 0; names[k]; k++)
            if (SameName(TcgDuel_Field(g, who, s), names[k]))
            {
                n++;
                break;
            }
    return n;
}
/* That Pokemon's attached Energy (or Trainer: kind) cards into a list: their indexes in attached[] */
static unsigned Attached(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned kind, uint8_t *out)
{
    const struct TcgFieldCard *f = &g->players[who].field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
        if (CardAt(g, who, f->attached[i])->kind == kind)
            out[n++] = (uint8_t)i;
    return n;
}
static void DiscardFoeEnergy(struct TcgDuel *g, unsigned who, unsigned slot, int random)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    uint8_t list[TCG_DECK_SIZE];
    unsigned n = Attached(g, who, slot, TCG_ENERGY, list), i, pick;
    if (!n)
        return;
    pick = list[0];
    if (random)
        pick = list[Tcg_RandomBelow(&g->rng, n)];
    else
        for (i = 0; i < n; i++)   /* (the attacker picks: Double Colorless and special Energy cost the most) */
            if (CardAt(g, who, f->attached[list[i]])->id > 6)
                pick = list[i];
    g->players[who].discard[g->players[who].discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, pick);
}
static void Evolve(struct TcgDuel *g, unsigned who, unsigned slot, unsigned deckIndex)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    if (f->stackSize >= 4)
        return;
    f->stack[f->stackSize++] = TcgPile_Remove(p->deck, &p->deckSize, deckIndex);
    f->enteredTurn = g->turn;
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    f->noAttack = f->noRetreat = f->weaken = f->guard = f->charged = f->effectBy = f->charge3 = 0;
    f->disabled = f->weakAs = f->resistAs = f->burn = f->shift = 0;
    f->lockTurn = f->lockAttack = 0;
    TcgDuel_Event(g, TCG_EV_EVOLVE, who, slot, 0, p->cards[f->stack[f->stackSize - 1]], 0);
    TcgDuel_Log(g, "Evolved into %s.", CardAt(g, who, f->stack[f->stackSize - 1])->name);
    TcgPower_OnPlay(g, who, slot);
}
static int Gb2Effect(struct TcgDuel *g, const struct Effect *e, const struct TcgAttack *a, const struct TcgAction *action,
                     unsigned *damage, int after)
{
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    struct TcgFieldCard *self = &p->field[0], *foe = &op->field[0];
    unsigned me = g->current, them = g->current ^ 1, n = 0, i, s;
    int blocked = g->foeImmune == TCG_IMMUNE_YES;
    switch (e->kind)
    {
    case FX_STATUS_HT:
        if (!after)
            SetSt(foe, TcgDuel_Coin(g, me) ? e->amount : e->extra);
        break;
    case FX_STATUS_OR_RECOIL:
        if (!after)
        {
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
            if (g->coinResult)
                SetSt(foe, e->extra);
        }
        else if (!g->coinResult)
            TcgDuel_SelfDamage(g, (int)e->amount);
        break;
    case FX_RANDOM_HITS:
        if (!after)
            *damage = 0;
        else
        {
            unsigned hits[TCG_FIELD_SIZE] = {0};
            for (i = 0; i < e->extra; i++)
                if ((s = RandomSlot(g, them, 0)) != TCG_NONE)
                    hits[s]++;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (hits[s] && !(s == 0 && blocked))
                    TcgDuel_EffectDamage(g, them, s, (int)(e->amount * hits[s]), 0);
        }
        break;
    case FX_HORRID_POLLEN:
        if (!after)
        {
            static const uint8_t sThree[3] = {ST_CNF, ST_SLP, ST_PAR};
            SetSt(foe, ST_PSN);
            SetSt(foe, sThree[Tcg_RandomBelow(&g->rng, 3)]);
        }
        break;
    case FX_DEADLY_POISON:
        if (!after)
        {
            if (foe->poison && !blocked)
                *damage += e->amount;
            SetSt(foe, ST_PSN);
        }
        break;
    case FX_CHOSEN_ANY:
        if (!after)
        {
            *damage = 0;
            g->coinResult = (uint8_t)(!(e->extra & CA_COIN) || TcgDuel_Coin(g, me));
        }
        else
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (!g->coinResult || (action->target == 0 && blocked))
                break;
            HitSlot(g, them, action->target, (int)e->amount, 0);
            if ((e->extra & CA_POWER_OFF) && TcgPower_Id(TcgDuel_Field(g, them, action->target)))
            {
                op->field[action->target].powerOffUntil = (uint16_t)(g->turn + 1);
                TcgDuel_Log(g, "%s's Pokemon Power stops working.", TcgDuel_Field(g, them, action->target)->name);
            }
        }
        break;
    case FX_POISON_BENCH:
        if (!after)
            SetSt(foe, ST_PSN);
        else
            BenchDamage(g, them, (int)e->amount);
        break;
    case FX_NAME_COUNT:
        if (!after)
        {
            static const char *const sNido[] = {"Nidoran M", "Nidoran F", "Nidorino", "Nidorina", "Nidoking", "Nidoqueen", NULL};
            static const char *const sMagnet[] = {"Magnemite", "Magneton", "Dark Magneton", NULL};
            static const char *const sVoltorb[] = {"Voltorb", NULL};
            unsigned group = e->extra & 15;
            const char *const *names = group == NG_NIDORAN ? sNido : group == NG_MAGNEMITE ? sMagnet : sVoltorb;
            if (e->extra & NG_IN_PLAY)
                n = NamedInPlay(g, me, 0, names) + NamedInPlay(g, them, 0, names);
            else
                n = NamedInPlay(g, me, 1, names);
            *damage += e->amount * n;
        }
        break;
    case FX_FAIL_RECOIL:
        if (!after && !TcgDuel_Coin(g, me))
        {
            *damage = 0;
            g->attackFailed = 1;
            TcgDuel_SelfDamage(g, (int)e->amount);
        }
        break;
    case FX_PETAL_WHIRL:
        if (!after)
        {
            n = Coins(g, me, e->amount);
            *damage = e->extra * n;
            g->coinResult = n >= 2;
        }
        else if (g->coinResult)
            SetStatus(self, TCG_CONFUSED);
        break;
    case FX_DISCARD_FOE_ENERGY:
        if (!after)
            g->coinResult = (uint8_t)(!e->extra || TcgDuel_Coin(g, me));
        else if (g->coinResult && foe->stackSize && !blocked)
            DiscardFoeEnergy(g, them, 0, 0);
        break;
    case FX_REGENERATION:
        if (after)
        {
            Heal(g, me, 0, self->damage);
            while (self->stackSize > 1)
                p->discard[p->discardSize++] = self->stack[--self->stackSize];
            for (i = self->attachedSize; i > 0; i--)
                if (CardAt(g, me, self->attached[i - 1])->kind == TCG_ENERGY)
                    p->discard[p->discardSize++] = TcgPile_Remove(self->attached, &self->attachedSize, i - 1);
            self->condition = TCG_HEALTHY;
            self->poison = 0;
            TcgDuel_Event(g, TCG_EV_DEVOLVE, me, 0, 0, p->cards[self->stack[0]], 0);
        }
        break;
    case FX_BONUS_STATUS:
        if (!after && TcgDuel_Coin(g, me))
        {
            *damage += e->amount;
            SetSt(foe, e->extra);
        }
        break;
    case FX_GAS_EXPLOSION:
        if (after)
        {
            TcgDuel_SelfDamage(g, (int)e->amount);
            if (foe->stackSize && !HpLeft(g, them, 0))
                op->poisonNext = 1;
        }
        break;
    case FX_MASS_EXPLOSION:
    {
        static const char *const sGas[] = {"Koffing", "Weezing", "Dark Weezing", NULL};
        unsigned who;
        if (!after)
            *damage = e->amount * (NamedInPlay(g, me, 0, sGas) + NamedInPlay(g, them, 0, sGas));
        else
            for (who = 0; who < 2; who++)
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                {
                    static const char *const *names = sGas;
                    unsigned k;
                    for (k = 0; names[k]; k++)
                        if (SameName(TcgDuel_Field(g, who, s), names[k]) && !(who == them && s == 0 && blocked))
                        {
                            TcgDuel_EffectDamage(g, who, s, (int)e->amount, who == me ? TCG_DMG_SELF : 0);
                            break;
                        }
                }
        break;
    }
    case FX_LOCK_SELF:
        if (after)
        {
            self->lockTurn = (uint16_t)(g->turn + 2);
            self->lockAttack = (uint8_t)(e->extra == 3 ? 3 : action->attack + 1);
        }
        break;
    case FX_FIREBALL:
        if (!after)
        {
            if (TcgDuel_Coin(g, me))
            {
                DiscardEnergy(g, me, 0, TCG_FIRE, 1);
                *damage += e->amount;
            }
            else if (!e->extra)
            {
                *damage = 0;
                g->attackFailed = 1;
            }
        }
        break;
    case FX_CONT_FIREBALL:
        if (!after)
        {
            n = Coins(g, me, FireEnergyCards(g, me, 0));
            *damage = e->amount * n;
            if (n)
                DiscardEnergy(g, me, 0, TCG_FIRE, n);
        }
        break;
    case FX_SWITCH_HIT:
        break; /* (before the attack: TcgDuel_AttackEffect's phase 3) */
    case FX_LURE_COIN:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
        else if (g->coinResult && !blocked && BenchCount(g, them))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            Switch(g, them, action->target);
        }
        break;
    case FX_SKIP_DRAW:
        if (after && !TcgDuel_Coin(g, them))
        {
            op->skipDrawTurn = (uint16_t)(g->turn + 1);
            TcgDuel_Log(g, "They won't draw a card next turn.");
        }
        break;
    case FX_SEARCH_TRAINER:
        if (after)
        {
            int at = -1;
            if (TcgDuel_Coin(g, me))
            {
                for (i = 0; i < p->deckSize; i++)
                    if (CardAt(g, me, p->deck[i])->kind == TCG_TRAINER &&
                        (action->selectionCount ? p->deck[i] == action->selections[0] : at < 0))
                        at = (int)i;
                if (at >= 0)
                {
                    p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at);
                    TcgDuel_Event(g, TCG_EV_DRAW, me, 0, 1, 0, 0);
                    TcgDuel_Log(g, "%s to the hand.", CardAt(g, me, p->hand[p->handSize - 1])->name);
                }
            }
            TcgDuel_Shuffle(g, me);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, 0, 0, 0);
        }
        break;
    case FX_TAILS_DISCARD_SELF:
        if (after && !TcgDuel_Coin(g, me))
            DiscardOne(g, me, 0, ANY_TYPE, 0);
        break;
    case FX_FLAME_INFERNO:
        if (!after)
        {
            if (action->option > e->extra || action->option > FireEnergyCards(g, me, 0))
                return TCG_INVALID;
            if (action->option)
                DiscardEnergy(g, me, 0, TCG_FIRE, action->option);
            *damage += e->amount * action->option;
        }
        break;
    case FX_FLAME_PILLAR:
        if (!after)
        {
            if (action->option > 1 || action->option > FireEnergyCards(g, me, 0))
                return TCG_INVALID;
            if (action->option)
                DiscardEnergy(g, me, 0, TCG_FIRE, 1);
        }
        else if (action->option && BenchCount(g, them) && ValidTarget(g, them, action->target, 1))
            TcgDuel_EffectDamage(g, them, action->target, (int)e->amount, 0);
        break;
    case FX_BURNING_FIRE:
        if (!after)
        {
            unsigned left = action->option;
            if (left > FieldFireCards(g, me))
                return TCG_INVALID;
            /* (from this Pokemon first, then the Bench) */
            for (s = 0; s < TCG_FIELD_SIZE && left; s++)
            {
                unsigned k = FireEnergyCards(g, me, s);
                if (k > left)
                    k = left;
                if (k)
                    DiscardEnergy(g, me, s, TCG_FIRE, k);
                left -= k;
            }
            *damage += e->amount * action->option;
        }
        break;
    case FX_DRY_UP:
        if (!after)
            *damage = 0;
        else
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            while (n < 60 && TcgDuel_Coin(g, me))
                n++;
            if (action->target == 0 && blocked)
                break;
            for (i = 0; i < n; i++)
                DiscardOne(g, them, action->target, TCG_WATER, 0);
        }
        break;
    case FX_MIRROR_SHELL:
        if (after)
            self->mirrorShell = 1;
        break;
    case FX_WATER_EXTRA2:
        if (!after)
        {
            n = EnergyPastCost(g, a, (e->extra & 16) != 0);
            if (n > (e->extra & 15))
                n = e->extra & 15;
            *damage += e->amount * n;
        }
        break;
    case FX_RECOIL_BARRIER:
        if (after)
        {
            TcgDuel_SelfDamage(g, (int)e->amount);
            if (TcgDuel_Coin(g, me))
                self->barrier = 1;
        }
        break;
    case FX_THIRD_EYE:
        if (!after)
        {
            if (!DiscardOne(g, me, 0, ANY_TYPE, 0))
                return TCG_INVALID;
        }
        else
            TcgDuel_Event(g, TCG_EV_DRAW, me, 0, TcgDuel_Draw(g, me, e->amount), 0, 0);
        break;
    case FX_REST:
        if (after)
        {
            Heal(g, me, 0, self->damage);
            self->poison = 0;
            self->condition = TCG_ASLEEP;
        }
        break;
    case FX_ENERGY_SEARCH:
        if (after)
        {
            unsigned max = e->amount & 127;
            if ((e->amount & 128) && !TcgDuel_Coin(g, me))
                break;
            for (i = p->deckSize; i > 0 && n < max; i--)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[i - 1]);
                if (c->kind == TCG_ENERGY && c->id <= 6 && c->type == e->extra)
                {
                    self->attached[self->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, i - 1);
                    TcgDuel_Event(g, TCG_EV_ATTACH, me, 0, 0, c->id, 0);
                    n++;
                }
            }
            TcgDuel_Shuffle(g, me);
        }
        break;
    case FX_WATER_BOMB:
        if (!after)
        {
            n = EnergyPastCost(g, a, 0);
            g->coinResult = (uint8_t)(n > 2 ? 2 : n);
        }
        else if (g->coinResult && BenchCount(g, them))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            TcgDuel_EffectDamage(g, them, action->target, (int)(e->amount * g->coinResult), 0);
        }
        break;
    case FX_STRANGE_BEAM:
        if (after)
            p->strangeBeamUntil = (uint16_t)(g->turn + 1);
        break;
    case FX_SPIN_SHOWER:
        if (!after)
            *damage = 0;
        else if (TcgDuel_Coin(g, me))
        {
            unsigned count = 1 + Tcg_RandomBelow(&g->rng, 2), first = TCG_NONE;
            for (i = 0; i < count; i++)
            {
                unsigned slots[TCG_FIELD_SIZE], k = 0;
                for (s = 0; s < TCG_FIELD_SIZE; s++)
                    if (op->field[s].stackSize && s != first)
                        slots[k++] = s;
                if (!k)
                    break;
                s = slots[Tcg_RandomBelow(&g->rng, k)];
                first = s;
                if (s == 0 && blocked)
                    continue;
                TcgDuel_EffectDamage(g, them, s, (int)e->amount, 0);
                DiscardFoeEnergy(g, them, s, 1);
            }
        }
        break;
    case FX_RAPID_EVOLUTION:
        if (after)
        {
            const struct TcgCard *mine = TcgDuel_Field(g, me, 0);
            int at = -1;
            for (i = 0; i < p->deckSize && mine; i++)
            {
                const struct TcgCard *c = CardAt(g, me, p->deck[i]);
                if (c->kind == TCG_POKEMON && c->stage && !strcmp(c->evolvesFrom, mine->name) &&
                    (at < 0 || c->hp > CardAt(g, me, p->deck[at])->hp))
                    at = (int)i;
            }
            if (at >= 0)
                Evolve(g, me, 0, (unsigned)at);
            TcgDuel_Shuffle(g, me);
        }
        break;
    case FX_FAIL_COINS:
        if (!after)
        {
            if (Coins(g, me, e->amount) != e->amount)
            {
                *damage = 0;
                g->attackFailed = 1;
            }
        }
        break;
    case FX_TENTACLE_GRIP:
        if (!after)
            *damage = 0;
        else if ((n = Coins(g, me, TcgDuel_Energy(g, me, 0, TCG_WATER))))
            TcgDuel_Event(g, TCG_EV_DRAW, me, 0, TcgDuel_Draw(g, me, e->amount * n), 0, 0);
        break;
    case FX_ACID_LOCK:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
        else if (g->coinResult)
            SetSt(foe, ST_PAR);
        else
        {
            self->lockTurn = (uint16_t)(g->turn + 2);
            self->lockAttack = (uint8_t)(action->attack + 1);
        }
        break;
    case FX_SHORT_CIRCUIT:
        if (!after)
            *damage = 0;
        else
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (!(action->target == 0 && blocked))
                HitSlot(g, them, action->target, (int)(e->amount * EnergyOfType(g, them, action->target, TCG_WATER)), 0);
        }
        break;
    case FX_SURPRISE_THUNDER:
        if (!after)
            g->coinResult = (uint8_t)(TcgDuel_Coin(g, me) ? 1 + TcgDuel_Coin(g, me) : 0);
        else if (g->coinResult)
            BenchDamage(g, them, g->coinResult == 2 ? 20 : 10);
        break;
    case FX_MICROWAVE:
        if (!after)
            *damage = 0;
        else
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (action->target == 0 && blocked)
                break;
            HitSlot(g, them, action->target, (int)e->amount, 0);
            if (TcgDuel_Coin(g, me))
                DiscardFoeEnergy(g, them, action->target, 0);
        }
        break;
    case FX_MAGNETIC_LINES:
        if (!after)
            g->coinResult = (uint8_t)(!e->extra || TcgDuel_Coin(g, me));
        else if (g->coinResult && foe->stackSize && !blocked && BenchCount(g, them))
        {
            int pick = -1;
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            for (i = 0; i < foe->attachedSize && pick < 0; i++)
            {
                const struct TcgCard *c = CardAt(g, them, foe->attached[i]);
                if (c->kind == TCG_ENERGY && c->id <= 6)
                    pick = (int)i;
            }
            if (pick >= 0)
            {
                struct TcgFieldCard *to = &op->field[action->target];
                to->attached[to->attachedSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, (unsigned)pick);
                TcgDuel_Event(g, TCG_EV_ATTACH, them, action->target, 0, op->cards[to->attached[to->attachedSize - 1]], 0);
            }
        }
        break;
    case FX_ENERGY_BOMB:
        if (after)
        {
            uint8_t list[TCG_DECK_SIZE];
            unsigned k = Attached(g, me, 0, TCG_ENERGY, list);
            int to = BenchCount(g, me) ? (ValidTarget(g, me, action->target, 1) ? action->target : -1) : 0;
            if (to < 0)
                return TCG_INVALID;
            for (i = k; i > 0; i--)
            {
                uint8_t card = TcgPile_Remove(self->attached, &self->attachedSize, list[i - 1]);
                if (to)
                    p->field[to].attached[p->field[to].attachedSize++] = card;
                else
                    p->discard[p->discardSize++] = card;
            }
            if (to && k)
                TcgDuel_Event(g, TCG_EV_ATTACH, me, (unsigned)to, 0, 0, 0);
        }
        break;
    case FX_RAGING_THUNDER:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
        else if (!g->coinResult)
        {
            if (!ValidTarget(g, me, action->target, 0))
                return TCG_INVALID;
            TcgDuel_EffectDamage(g, me, action->target, (int)e->amount, TCG_DMG_SELF);
        }
        break;
    case FX_SWIFT:
        if (!after)
            g->swift = 1;
        break;
    case FX_ROLLING_NEEDLE:
        if (!after)
        {
            g->coinResult = (uint8_t)Coins(g, me, e->amount);
            *damage += e->extra * g->coinResult;
        }
        else if (g->coinResult)
            TcgDuel_SelfDamage(g, (int)(e->extra * g->coinResult));
        break;
    case FX_THEIR_COIN_BONUS:
        if (!after && !TcgDuel_Coin(g, them))
            *damage += e->amount;
        break;
    case FX_SHUFFLE_THEIRS:
        if (after)
        {
            TcgDuel_Shuffle(g, them);
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
        }
        break;
    case FX_CONFUSE_SELF:
        if (after)
            SetStatus(self, TCG_CONFUSED);
        break;
    case FX_FOCUSED_SHOT:
        if (after)
        {
            if (TcgDuel_Coin(g, me))
                self->charged = 1;
            else
            {
                self->lockTurn = (uint16_t)(g->turn + 2);
                self->lockAttack = (uint8_t)((action->attack ^ 1) + 1);
            }
        }
        break;
    case FX_FOCUS_BLAST:
        if (!after)
            *damage = 0;
        else
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (TcgDuel_Coin(g, me) && !(action->target == 0 && blocked))
                HitSlot(g, them, action->target, (int)e->amount, 1);
        }
        break;
    case FX_SEETHING:
        if (!after)
            *damage += e->amount * Coins(g, me, self->damage / 10);
        break;
    case FX_FLING_DECK:
        if (after && foe->stackSize && !blocked && BenchCount(g, them))
        {
            TcgDuel_Log(g, "%s was shuffled into the deck.", TcgDuel_Field(g, them, 0)->name);
            SlotToDeck(g, them, 0);
        }
        break;
    case FX_BOULDER:
        if (!after)
            g->coinResult = (uint8_t)Coins(g, me, e->amount);
        else if (g->coinResult && BenchCount(g, them))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            /* damage counters, not damage: nothing reduces them */
            op->field[action->target].damage += 10u * g->coinResult;
            TcgDuel_Event(g, TCG_EV_DAMAGE, them, action->target, 10u * g->coinResult, 0, 0);
        }
        break;
    case FX_ROCK_BLAST:
        if (!after)
        {
            unsigned fighting = EnergyOfType(g, me, 0, TCG_FIGHTING);
            if (action->option > 5 || action->option > fighting)
                return TCG_INVALID;
            if (action->option)
                DiscardEnergy(g, me, 0, TCG_FIGHTING, action->option);
            *damage = 0;
        }
        else if (action->option)
        {
            if (!ValidTarget(g, them, action->target, 0))
                return TCG_INVALID;
            if (!(action->target == 0 && blocked))
                HitSlot(g, them, action->target, (int)(e->amount * action->option), 0);
        }
        break;
    case FX_BONE_TOSS:
        if (!after)
        {
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
            *damage = g->coinResult ? e->amount : 0;
        }
        else if (!g->coinResult && BenchCount(g, them))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            TcgDuel_EffectDamage(g, them, action->target, (int)e->extra, 0);
        }
        break;
    case FX_BONE_HEADBUTT:
        if (after)
        {
            if ((s = RandomSlot(g, them, 1)) != TCG_NONE)
                TcgDuel_EffectDamage(g, them, s, (int)(10 * Tcg_RandomBelow(&g->rng, 3)), 0);
            if (foe->stackSize && !blocked && HpLeft(g, them, 0) && (s = RandomSlot(g, them, 1)) != TCG_NONE)
                Switch(g, them, s);
        }
        break;
    case FX_COIN_BENCH:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
        else if (g->coinResult && BenchCount(g, them))
        {
            if (!ValidTarget(g, them, action->target, 1))
                return TCG_INVALID;
            TcgDuel_EffectDamage(g, them, action->target, (int)e->amount, 0);
        }
        break;
    case FX_MOUNTAIN_BREAK:
        if (after)
        {
            unsigned kept = 0;
            for (i = 0; i < 5 && p->deckSize; i++)
            {
                uint8_t card = p->deck[--p->deckSize];
                const struct TcgCard *c = CardAt(g, me, card);
                if (c->kind == TCG_ENERGY && c->id <= 6 && c->type == TCG_FIGHTING)
                    p->hand[p->handSize++] = card, kept++;
                else
                    p->discard[p->discardSize++] = card;
            }
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, me, 0, i, 0, 0);
            if (kept)
                TcgDuel_Event(g, TCG_EV_DRAW, me, 0, kept, 0, 0);
            TcgDuel_Log(g, "Mountain Break: %u cards discarded, %u Fighting Energy to the hand.", i - kept, kept);
        }
        break;
    case FX_VANISH:
        if (after && self->stackSize)
        {
            TcgDuel_Log(g, "%s vanished into the deck.", TcgDuel_Field(g, me, 0)->name);
            /* (its attached cards go to the discard pile) */
            for (i = 0; i < self->attachedSize; i++)
                p->discard[p->discardSize++] = self->attached[i];
            self->attachedSize = 0;
            SlotToDeck(g, me, 0);
        }
        break;
    case FX_TYPE_DOUBLE:
        if (!after && TcgDuel_Field(g, them, 0) && TcgDuel_Type(g, them, 0) == e->extra)
            *damage *= 2;
        break;
    case FX_TRANS_DAMAGE:
        if (after)
        {
            unsigned moved = self->damage;
            DiscardEnergy(g, me, 0, ANY_TYPE, 0);
            if (!moved || !foe->stackSize || blocked)
                break;
            Heal(g, me, 0, moved);
            foe->damage += moved;
            TcgDuel_Event(g, TCG_EV_DAMAGE, them, 0, moved, 0, 0);
        }
        break;
    case FX_BOTH_ASLEEP:
        if (after)
        {
            SetSt(foe, ST_SLP);
            SetStatus(self, TCG_ASLEEP);
        }
        break;
    case FX_ONLY_ASLEEP:
        break; /* (TcgDuel_EffectBlock, TcgDuel_SleepAttack) */
    case FX_GRUDGE:
        if (!after)
            *damage += e->amount * (g->prizeCount > op->prizeSize ? g->prizeCount - op->prizeSize : 0);
        break;
    case FX_POLTERGEIST:
        if (!after)
        {
            for (i = 0; i < op->handSize; i++)
                n += CardAt(g, them, op->hand[i])->kind == TCG_TRAINER;
            *damage = e->amount * n;
            TcgDuel_Log(g, "Poltergeist: %u Trainer cards in their hand.", n);
        }
        break;
    case FX_BOTHER:
        if (after && TcgDuel_Coin(g, me))
        {
            uint8_t list[TCG_DECK_SIZE];
            for (i = 0; i < op->handSize; i++)
                if (CardAt(g, them, op->hand[i])->kind == TCG_TRAINER)
                    list[n++] = (uint8_t)i;
            if (n)
            {
                op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, list[Tcg_RandomBelow(&g->rng, n)]);
                TcgDuel_Shuffle(g, them);
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, them, 0, 0, 0, 0);
                TcgDuel_Log(g, "Bother: a Trainer card from their hand went into their deck.");
            }
        }
        break;
    case FX_PUSH_ASIDE:
        if (after && TcgDuel_Coin(g, me) && BenchCount(g, them))
        {
            s = RandomSlot(g, them, 0);
            if (s == TCG_NONE || (s == 0 && blocked) || (s && TcgPower_Immune(g, them, s)))
                break;
            TcgDuel_Log(g, "%s was shuffled into the deck.", TcgDuel_Field(g, them, s)->name);
            SlotToDeck(g, them, s);
        }
        break;
    case FX_BENCH_MANIP:
        if (!after)
        {
            unsigned coins = BenchCount(g, them);
            n = coins - Coins(g, them, coins);
            *damage = e->amount * n;
            g->noWeakRes = 1;
        }
        break;
    case FX_PSYCRUSH:
        if (!after)
        {
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                n += EnergyOfType(g, them, s, TCG_PSYCHIC);
            *damage = e->amount * n;
        }
        break;
    case FX_COMPLETE_RECOVERY:
        if (after)
        {
            DiscardEnergy(g, me, 0, ANY_TYPE, 0);
            Heal(g, me, 0, self->damage);
            self->poison = 0;
            if (self->condition == TCG_CONFUSED)
                self->condition = TCG_HEALTHY;
            self->food = 0;
        }
        break;
    case FX_DARK_AMP:
        if (!after)
        {
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                n += IsDark(TcgDuel_Field(g, me, s));
            *damage += e->amount * Coins(g, me, n);
        }
        break;
    case FX_TWISTER:
        if (after && foe->stackSize && !blocked && HpLeft(g, them, 0) && TcgDuel_Coin(g, me))
        {
            for (i = foe->attachedSize; i > 0; i--)
                op->hand[op->handSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, i - 1);
            TcgDuel_Event(g, TCG_EV_DRAW, them, 0, 0, 0, 0);
            TcgDuel_Log(g, "Twister: the cards on %s blew back to the hand.", TcgDuel_Field(g, them, 0)->name);
        }
        break;
    case FX_DARK_SONG:
        if (!after)
        {
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
            if (g->coinResult)
                SetSt(foe, ST_SLP);
        }
        else if (g->coinResult)
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (TcgDuel_Field(g, them, s) && !IsDark(TcgDuel_Field(g, them, s)))
                    TcgDuel_EffectDamage(g, them, s, (int)e->amount, 0);
        break;
    case FX_KICK_STAMP:
        if (!after)
        {
            g->coinResult = (uint8_t)TcgDuel_Coin(g, me);
            if (g->coinResult)
                *damage += e->amount;
        }
        else if (!g->coinResult && BenchCount(g, them) && !blocked)
            g->switchPending = 1;
        break;
    case FX_ROLL_OVER:
        if (after)
        {
            SetStatus(self, TCG_ASLEEP);
            if (TcgDuel_Coin(g, me))
                SetSt(foe, ST_SLP);
        }
        break;
    case FX_EAT:
        if (after && self->food < 2)
            self->food++;
        break;
    case FX_ROLLOUT:
        if (!after)
        {
            if (action->option > self->food)
                return TCG_INVALID;
            self->food -= action->option;
            *damage += e->amount * action->option;
        }
        break;
    case FX_COIN_PLUS:
        if (!after)
            *damage += e->extra * Coins(g, me, e->amount);
        break;
    case FX_STIR_UP:
        if (after)
        {
            if (BenchCount(g, me) && ValidTarget(g, me, action->target, 1))
                Switch(g, me, action->target);
            if (foe->stackSize && BenchCount(g, them) && !blocked)
                g->switchPending = 1;
        }
        break;
    case FX_NOTHING:
        break;
    default:
        return TCG_UNSUPPORTED;
    }
    (void)n;
    return TCG_OK;
}
int TcgDuel_AttackEffect(struct TcgDuel *g, const struct TcgAttack *a, const struct TcgAction *action, unsigned *damage,
                         int after)
{
    const struct Effect *e = Effect(a->effect);
    struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    struct TcgFieldCard *self = &p->field[0], *foe = &op->field[0];
    unsigned n = 0, i, s;
    if (!strcmp(a->effect, "NONE"))
        return TCG_OK;
    if (!e)
        return TCG_UNSUPPORTED;
    if (after == 3)
    {
        /* Before the attack (and before the Defending Pokemon's protection is settled): Fox Fire, Drag Off */
        if (e->kind == FX_SWITCH_HIT && BenchCount(g, g->current ^ 1))
        {
            if (ValidTarget(g, g->current ^ 1, action->target, 1))
                Switch(g, g->current ^ 1, action->target);
            else if (e->extra)
                return TCG_INVALID;
        }
        return TCG_OK;
    }
    if (e->after != BOTH_PHASES && e->after != (unsigned)after)
        return TCG_OK;
    if (g->attackFailed)
        return TCG_OK;
    switch (e->kind)
    {
    case FX_PARALYZE_COIN:
        if (TcgDuel_Coin(g, g->current))
            foe->condition = TCG_PARALYZED;
        break;
    case FX_POISON:
        foe->poison = e->amount;
        break;
    case FX_POISON_COIN:
        if (TcgDuel_Coin(g, g->current))
            foe->poison = e->amount;
        break;
    case FX_CONFUSE_COIN:
        if (TcgDuel_Coin(g, g->current))
            foe->condition = TCG_CONFUSED;
        break;
    case FX_SLEEP:
        foe->condition = TCG_ASLEEP;
        break;
    case FX_SLEEP_COIN:
        if (TcgDuel_Coin(g, g->current))
            foe->condition = TCG_ASLEEP;
        break;
    case FX_DRAIN:
        if (*damage)
            Heal(g, g->current, 0, e->amount);
        break;
    case FX_BARRIER_COIN:
        if (TcgDuel_Coin(g, g->current))
            self->barrier = e->amount ? (uint8_t)e->amount : 1;
        break;
    case FX_BONUS_COIN:
        if (TcgDuel_Coin(g, g->current))
            *damage += e->amount;
        break;
    case FX_RAGE:
        *damage += self->damage;
        break;
    case FX_MEDITATE:
        *damage += foe->damage;
        break;
    /* "for each Energy card attached": a Double Colorless is one card */
    case FX_PSYWAVE:
        *damage = 10 * EnergyCards(g, g->current ^ 1, 0);
        break;
    case FX_PSYCHIC:
        *damage += 10 * EnergyCards(g, g->current ^ 1, 0);
        break;
    case FX_KARATE_CHOP:
        *damage = *damage > self->damage ? *damage - self->damage : 0;
        break;
    case FX_SUPER_FANG:
    {
        unsigned left = HpLeft(g, g->current ^ 1, 0);
        *damage = ((left + 19) / 20) * 10;
        break;
    }
    case FX_MULTI_COIN:
        for (i = 0; i < e->amount; i++)
            n += TcgDuel_Coin(g, g->current);
        *damage *= n;
        break;
    case FX_DO_THE_WAVE:
        for (i = 1; i < TCG_FIELD_SIZE; i++)
            if (g->players[g->current].field[i].stackSize)
                *damage += 10;
        break;
    case FX_WATER_EXTRA:
    {
        /* Water Energy not used to pay the cost: the Water part takes Water, the
         * Colorless part takes the other Energy first. At most 20 more. */
        unsigned water = TcgDuel_Energy(g, g->current, 0, TCG_WATER), other, need;
        other = TcgDuel_Energy(g, g->current, 0, TCG_NONE) - water;
        need = a->cost[TCG_WATER] + (a->cost[TCG_COLORLESS] > other ? a->cost[TCG_COLORLESS] - other : 0);
        if (water > need)
        {
            n = water - need;
            *damage += 10 * (n > 2 ? 2 : n);
        }
        break;
    }

    /* Costs paid in Energy cards, before the damage. */
    case FX_DISCARD_TYPED:
        if (!after && !DiscardEnergy(g, g->current, 0, e->extra, e->amount))
            return TCG_INVALID;
        break;
    case FX_FLAMES_OF_RAGE:
        if (!after)
        {
            if (!DiscardEnergy(g, g->current, 0, e->extra, e->amount))
                return TCG_INVALID;
            *damage += self->damage;
        }
        break;
    case FX_RECOVER:
        if (!after && !DiscardEnergy(g, g->current, 0, e->extra, e->amount))
            return TCG_INVALID;
        if (after)
            Heal(g, g->current, 0, self->damage);
        break;
    case FX_BARRIER_ALL:
        if (!after)
        {
            if (!DiscardEnergy(g, g->current, 0, e->extra, e->amount))
                return TCG_INVALID;
            self->barrier = 2;
        }
        break;

    /* Recoil and coin results. */
    case FX_RECOIL:
        /* (extra: no Weakness and Resistance, Tailspin Attack) */
        if (after && e->extra && self->stackSize)
        {
            self->damage += e->amount;
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, e->amount, 0, TCG_DMG_SELF);
        }
        else if (after)
            TcgDuel_SelfDamage(g, (int)e->amount);
        break;
    case FX_RECOIL_COIN:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, g->current);
        else if (!g->coinResult)
            TcgDuel_SelfDamage(g, (int)e->amount);
        break;
    case FX_THRASH:
        if (!after)
        {
            g->coinResult = (uint8_t)TcgDuel_Coin(g, g->current);
            if (g->coinResult)
                *damage += e->amount;
        }
        else if (!g->coinResult)
            TcgDuel_SelfDamage(g, (int)e->amount);
        break;
    case FX_FAIL_COIN:
        if (!after)
        {
            if (e->extra)
                self->usedOnce = 1;
            if (!TcgDuel_Coin(g, g->current))
            {
                *damage = 0;
                g->attackFailed = 1;
            }
        }
        break;
    case FX_CLAMP:
        if (!after)
        {
            if (TcgDuel_Coin(g, g->current))
                foe->condition = TCG_PARALYZED;
            else
            {
                *damage = 0;
                g->attackFailed = 1;
            }
        }
        break;
    case FX_FLY:
        if (!after)
        {
            if (TcgDuel_Coin(g, g->current))
                self->barrier = 2;
            else
            {
                *damage = 0;
                g->attackFailed = 1;
            }
        }
        break;
    case FX_DRAW:
        if (after && (!e->extra || TcgDuel_Coin(g, g->current)))
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, TcgDuel_Draw(g, g->current, e->amount), 0, 0);
        break;
    case FX_HEAL_SELF:
        if (!after && e->extra)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, g->current);
        if (after && (!e->extra || g->coinResult))
            Heal(g, g->current, 0, e->amount);
        break;
    case FX_DRAIN_DONE:
        /* Damage done after Weakness and Resistance; half rounds up to 10. */
        if (after && *damage)
            Heal(g, g->current, 0, e->amount == 2 ? (*damage / 2 + 9) / 10 * 10 : *damage);
        break;

    /* Special conditions. */
    case FX_CONFUSE_BOTH:
        if (after)
        {
            foe->condition = TCG_CONFUSED;
            SetStatus(self, TCG_CONFUSED);
        }
        break;
    case FX_PETAL_DANCE:
        if (!after)
        {
            for (i = 0; i < e->amount; i++)
                n += TcgDuel_Coin(g, g->current);
            *damage = e->extra * n;
            SetStatus(self, TCG_CONFUSED);
        }
        break;
    case FX_FOUL_GAS:
        if (!after)
        {
            if (TcgDuel_Coin(g, g->current))
                foe->poison = 10;
            else
                foe->condition = TCG_CONFUSED;
        }
        break;
    case FX_VENOM_POWDER:
        if (!after && TcgDuel_Coin(g, g->current))
        {
            foe->condition = TCG_CONFUSED;
            foe->poison = 10;
        }
        break;
    case FX_TANTRUM:
        if (!after && !TcgDuel_Coin(g, g->current))
            SetStatus(self, TCG_CONFUSED);
        break;
    case FX_RAMPAGE:
        if (!after)
        {
            *damage += self->damage;
            if (!TcgDuel_Coin(g, g->current))
                SetStatus(self, TCG_CONFUSED);
        }
        break;

    /* Damage formulas. */
    case FX_FLAIL:
        if (!after)
            *damage = self->damage;
        break;
    case FX_EGGSPLOSION:
        if (!after)
        {
            unsigned coins = TcgDuel_Energy(g, g->current, 0, TCG_NONE);
            for (i = 0; i < coins; i++)
                n += TcgDuel_Coin(g, g->current);
            *damage = e->amount * n;
        }
        break;
    case FX_UNTIL_TAILS:
        if (!after)
        {
            while (n < 60 && TcgDuel_Coin(g, g->current))
                n++;
            *damage = e->amount * n;
        }
        break;
    case FX_BOYFRIENDS:
        if (!after)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                if (SameName(TcgDuel_Field(g, g->current, s), "Nidoking"))
                    *damage += e->amount;
        break;
    case FX_NO_WR:
        if (!after)
            g->noWeakRes = 1;
        break;
    case FX_DREAM_EATER:
        break;

    /* Damage to the Bench and to Pokemon chosen at random. */
    case FX_SELFDESTRUCT:
        if (after)
        {
            TcgDuel_SelfDamage(g, (int)e->extra);
            BenchDamage(g, g->current, (int)e->amount);
            BenchDamage(g, g->current ^ 1, (int)e->amount);
        }
        break;
    case FX_EARTHQUAKE:
        if (after)
            BenchDamage(g, g->current, (int)e->amount);
        break;
    case FX_BLIZZARD:
        if (!after)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, g->current);
        else
            BenchDamage(g, g->current ^ (g->coinResult ? 1 : 0), (int)e->amount);
        break;
    case FX_THUNDERSTORM:
        if (after)
        {
            unsigned tails = 0, heads[TCG_FIELD_SIZE] = {0};
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (op->field[s].stackSize)
                {
                    heads[s] = TcgDuel_Coin(g, g->current);
                    tails += !heads[s];
                }
            TcgDuel_SelfDamage(g, (int)(tails * e->extra));
            for (s = 1; s < TCG_FIELD_SIZE; s++)
                if (heads[s])
                    TcgDuel_EffectDamage(g, g->current ^ 1, s, (int)e->amount, 0);
        }
        break;
    case FX_CHAIN_LIGHTNING:
        if (after)
        {
            const struct TcgCard *target = TcgDuel_Field(g, g->current ^ 1, 0);
            unsigned who, type = TcgDuel_Type(g, g->current ^ 1, 0);
            if (!target || type == TCG_COLORLESS)
                break;
            for (who = 0; who < 2; who++)
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (TcgDuel_Field(g, who, s) && TcgDuel_Type(g, who, s) == type)
                        TcgDuel_EffectDamage(g, who, s, (int)e->amount, 0);
        }
        break;
    case FX_RANDOM_OPP:
        if (!after)
            *damage = 0;
        else if ((s = RandomSlot(g, g->current ^ 1, 0)) != TCG_NONE)
            TcgDuel_EffectDamage(g, g->current ^ 1, s, (int)e->amount, 0);
        break;
    case FX_BIG_THUNDER:
        if (after)
        {
            /* Any Pokemon in play but Zapdos itself, each as likely. */
            unsigned who, pool[2 * TCG_FIELD_SIZE];
            for (who = 0; who < 2; who++)
                for (s = who == g->current ? 1 : 0; s < TCG_FIELD_SIZE; s++)
                    if (g->players[who].field[s].stackSize)
                        pool[n++] = who * TCG_FIELD_SIZE + s;
            if (n)
            {
                s = pool[Tcg_RandomBelow(&g->rng, n)];
                TcgDuel_EffectDamage(g, s / TCG_FIELD_SIZE, s % TCG_FIELD_SIZE, (int)e->amount, 0);
            }
        }
        break;
    case FX_CHOSEN_BENCH:
        if (after && BenchCount(g, g->current ^ 1))
        {
            if (action->target < 1 || action->target >= TCG_FIELD_SIZE || !op->field[action->target].stackSize)
                return TCG_INVALID;
            TcgDuel_EffectDamage(g, g->current ^ 1, action->target, (int)e->amount, 0);
        }
        break;
    case FX_GIGASHOCK:
        if (after)
        {
            /* Up to three: the original lets the attacker pick; the ones
             * with the least HP left are taken here. */
            unsigned hit = 0, used[TCG_FIELD_SIZE] = {0};
            while (hit < e->extra)
            {
                unsigned best = TCG_NONE;
                for (s = 1; s < TCG_FIELD_SIZE; s++)
                    if (op->field[s].stackSize && !used[s] &&
                        (best == TCG_NONE || HpLeft(g, g->current ^ 1, s) < HpLeft(g, g->current ^ 1, best)))
                        best = s;
                if (best == TCG_NONE)
                    break;
                used[best] = 1;
                hit++;
                TcgDuel_EffectDamage(g, g->current ^ 1, best, (int)e->amount, 0);
            }
        }
        break;
    case FX_LURE:
        if (after && g->foeImmune != TCG_IMMUNE_YES)
        {
            if (action->target < 1 || action->target >= TCG_FIELD_SIZE || !op->field[action->target].stackSize)
                return TCG_INVALID;
            Switch(g, g->current ^ 1, action->target);
        }
        break;
    case FX_TELEPORT:
        /* (extra: it may, Teleport Blast) */
        if (after && e->extra && (!BenchCount(g, g->current) || !action->target))
            break;
        if (after)
        {
            if (action->target < 1 || action->target >= TCG_FIELD_SIZE || !p->field[action->target].stackSize)
                return TCG_INVALID;
            Switch(g, g->current, action->target);
        }
        break;
    case FX_OPP_SWITCH:
        if (!after && e->extra)
            g->coinResult = (uint8_t)TcgDuel_Coin(g, g->current);
        if (after)
        {
            TcgDuel_SelfDamage(g, (int)e->amount);
            if ((!e->extra || g->coinResult) && BenchCount(g, g->current ^ 1) && g->foeImmune != TCG_IMMUNE_YES)
                g->switchPending = 1;
        }
        break;

    /* Substatus. */
    case FX_GUARD:
        if (after)
            self->guard = (uint8_t)e->amount;
        break;
    case FX_WEAKEN:
        if (after)
        {
            foe->weaken = (uint8_t)e->amount;
            /* Growl, Pounce, Snivel: only attacks on this Pokemon (TcgDuel_BoundEffect) */
            if (e->amount != TCG_WEAKEN_SMOKESCREEN)
                foe->effectBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
        }
        break;
    case FX_NO_ATTACK_COIN:
        if (!after && TcgDuel_Coin(g, g->current))
        {
            /* Leer, Tail Wag: it can't attack this Pokemon; Bone Attack: it can't attack */
            foe->noAttack = e->extra ? 2 : 1;
            if (e->extra)
                foe->effectBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
        }
        break;
    case FX_NO_RETREAT_COIN:
        if (!after && TcgDuel_Coin(g, g->current))
            foe->noRetreat = 1;
        break;
    case FX_HEADACHE:
        if (after)
            op->noTrainers = 1;
        break;
    case FX_CHARGE:
        if (after)
        {
            self->charged = 1;
            self->charge3 = e->extra == 3; /* (Water Power: tripled) */
        }
        break;

    /* Cards from the deck and the discard pile. */
    case FX_CALL_FAMILY:
        if (after)
        {
            /* The original asks which; any copy of the card is the same. */
            for (i = p->deckSize; i > 0; i--)
                if (FamilyMatch(e, CardAt(g, g->current, p->deck[i - 1])))
                {
                    ToBench(g, g->current, p->deck[i - 1]);
                    break;
                }
            TcgDuel_Shuffle(g, g->current);
        }
        break;
    case FX_FRIENDSHIP_SONG:
        if (after && TcgDuel_Coin(g, g->current))
        {
            unsigned picks[TCG_DECK_SIZE];
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                if (c->kind == TCG_POKEMON && !c->stage)
                    picks[n++] = p->deck[i];
            }
            if (n)
                ToBench(g, g->current, picks[Tcg_RandomBelow(&g->rng, n)]);
            TcgDuel_Shuffle(g, g->current);
        }
        break;
    case FX_ENERGY_CONVERSION:
        if (after)
        {
            TakeEnergyFromDiscard(g, e->amount, 0);
            TcgDuel_SelfDamage(g, (int)e->extra);
        }
        break;
    case FX_ENERGY_ABSORPTION:
        if (after)
            TakeEnergyFromDiscard(g, e->amount, 1);
        break;
    case FX_ENERGY_SPIKE:
        if (after)
        {
            /* The original asks which Energy and where: a basic Energy of the
             * Active Pokemon's type goes to it, else any basic Energy. */
            const struct TcgCard *active = TcgDuel_Field(g, g->current, 0);
            int pick = -1;
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                if (c->kind == TCG_ENERGY && c->id <= 6 && (pick < 0 || (active && c->type == active->type)))
                    pick = (int)i;
            }
            if (pick >= 0)
            {
                self->attached[self->attachedSize++] = TcgPile_Remove(p->deck, &p->deckSize, (unsigned)pick);
                TcgDuel_Event(g, TCG_EV_ATTACH, g->current, 0, 0, p->cards[self->attached[self->attachedSize - 1]], 0);
            }
            TcgDuel_Shuffle(g, g->current);
        }
        break;
    case FX_HYPER_BEAM:
        if (after && foe->stackSize && g->foeImmune != TCG_IMMUNE_YES)
        {
            /* The attacker picks; Double Colorless costs the most to lose. */
            int pick = -1;
            for (i = 0; i < foe->attachedSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current ^ 1, foe->attached[i]);
                if (c->kind == TCG_ENERGY && (pick < 0 || c->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY))
                    pick = (int)i;
            }
            if (pick >= 0)
                op->discard[op->discardSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, (unsigned)pick);
        }
        break;

    /* (1.46.0) */
    case FX_MIX_UP:
        if (after)
        {
            /* Their Pokemon cards in hand go into their deck; as many Pokemon cards from the top of it go back */
            unsigned back = 0;
            for (i = op->handSize; i > 0; i--)
                if (CardAt(g, g->current ^ 1, op->hand[i - 1])->kind == TCG_POKEMON)
                {
                    op->deck[op->deckSize++] = TcgPile_Remove(op->hand, &op->handSize, i - 1);
                    back++;
                }
            TcgDuel_Shuffle(g, g->current ^ 1);
            for (i = op->deckSize; i > 0 && n < back; i--)
                if (CardAt(g, g->current ^ 1, op->deck[i - 1])->kind == TCG_POKEMON)
                {
                    op->hand[op->handSize++] = TcgPile_Remove(op->deck, &op->deckSize, i - 1);
                    n++;
                }
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ 1, 0, 0, 0, 0);
            TcgDuel_Log(g, "Mix-Up: %u Pokemon cards shuffled away and drawn again.", back);
        }
        break;
    case FX_WILDFIRE:
        if (!after)
        {
            /* Discard that many Fire Energy cards from it first */
            if (action->option > FireEnergyCards(g, g->current, 0))
                return TCG_INVALID;
            if (action->option && !DiscardEnergy(g, g->current, 0, TCG_FIRE, action->option))
                return TCG_INVALID;
        }
        else
        {
            for (i = 0; i < action->option && op->deckSize; i++)
                op->discard[op->discardSize++] = op->deck[--op->deckSize];
            if (i)
            {
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ 1, 0, i, 0, 0);
                TcgDuel_Log(g, "Wildfire: %u cards from the top of their deck discarded.", i);
            }
        }
        break;
    case FX_AMNESIA:
        if (!after)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
            if (!c || action->option > 1 || !RealAttack(&c->attacks[action->option]))
                return TCG_INVALID;
            /* (extra: on heads, Disable) */
            if (!e->extra || TcgDuel_Coin(g, g->current))
                Amnesia(g, action->option);
        }
        break;
    case FX_MAGNETIC_STORM:
        if (after)
        {
            /* Every Energy card on its side, shuffled, dealt out evenly; the rest to Pokemon chosen at random */
            uint8_t energy[TCG_DECK_SIZE], slots[TCG_FIELD_SIZE];
            unsigned count = 0, mons = 0, each, k;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                struct TcgFieldCard *f = &p->field[s];
                if (!f->stackSize)
                    continue;
                slots[mons++] = (uint8_t)s;
                for (i = f->attachedSize; i > 0; i--)
                    if (CardAt(g, g->current, f->attached[i - 1])->kind == TCG_ENERGY)
                        energy[count++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
            }
            if (!mons)
                break;
            ShuffleList(g, energy, count);
            each = count / mons;
            for (k = 0, i = 0; k < mons; k++)
                for (s = 0; s < each; s++, i++)
                    p->field[slots[k]].attached[p->field[slots[k]].attachedSize++] = energy[i];
            ShuffleList(g, slots, mons);
            for (k = 0; i < count; i++, k++)
                p->field[slots[k]].attached[p->field[slots[k]].attachedSize++] = energy[i];
            for (k = 0; k < mons; k++)
                TcgDuel_Event(g, TCG_EV_ATTACH, g->current, slots[k], 0, 0, 0);
            TcgDuel_Log(g, "Magnetic Storm: %u Energy cards dealt out again.", count);
        }
        break;
    case FX_WAIL:
        if (after)
        {
            FillBench(g, g->current ^ 1);
            FillBench(g, g->current);
        }
        break;
    case FX_SCAVENGE:
        if (!after)
        {
            if (!DiscardEnergy(g, g->current, 0, TCG_PSYCHIC, 1))
                return TCG_INVALID;
        }
        else
        {
            int at = -1;
            for (i = 0; i < p->discardSize; i++)
                if (CardAt(g, g->current, p->discard[i])->kind == TCG_TRAINER &&
                    (action->selectionCount ? p->discard[i] == action->selections[0] : at < 0))
                    at = (int)i;
            if (at < 0)
                return TCG_INVALID;
            p->hand[p->handSize++] = TcgPile_Remove(p->discard, &p->discardSize, (unsigned)at);
            TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, 1, 0, 0);
            TcgDuel_Log(g, "Scavenge: %s back to the hand.", CardAt(g, g->current, p->hand[p->handSize - 1])->name);
        }
        break;
    case FX_DESTINY_BOND:
        if (!after)
        {
            if (!DiscardEnergy(g, g->current, 0, TCG_PSYCHIC, 1))
                return TCG_INVALID;
            self->destinyBond = 1;
        }
        break;
    case FX_PROPHECY:
        if (after)
        {
            /* The top cards of that deck (up to 3) in the order chosen, the first on top */
            struct TcgPlayer *d = &g->players[g->current ^ (action->option ? 1 : 0)];
            unsigned top = d->deckSize < 3 ? d->deckSize : 3, k;
            if (action->option > 1)
                return TCG_INVALID;
            if (!action->selectionCount)
            {
                TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ (action->option ? 1 : 0), 0, 0, 0, 0);
                break;
            }
            if (action->selectionCount != top)
                return TCG_INVALID;
            for (i = 0; i < top; i++)
            {
                for (k = 0; k < top && d->deck[d->deckSize - 1 - k] != action->selections[i]; k++)
                    ;
                if (k == top)
                    return TCG_INVALID;
                for (k = 0; k < i; k++)
                    if (action->selections[k] == action->selections[i])
                        return TCG_INVALID;
            }
            for (i = 0; i < top; i++)
                d->deck[d->deckSize - 1 - i] = action->selections[i];
            TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current ^ (action->option ? 1 : 0), 0, 0, 0, 0);
            TcgDuel_Log(g, "Prophecy: the top of a deck rearranged.");
        }
        break;
    case FX_MYSTERY_ATTACK:
        if (!after)
        {
            /* 10 damage and one of eight things at random (MysteryAttack_RandomEffect) */
            g->coinResult = (uint8_t)Tcg_RandomBelow(&g->rng, 8);
            *damage = 10;
            switch (g->coinResult)
            {
            case 0: foe->condition = TCG_PARALYZED; break;
            case 1: foe->poison = 10; break;
            case 2: foe->condition = TCG_ASLEEP; break;
            case 3: foe->condition = TCG_CONFUSED; break;
            case 6: *damage = 20; break;
            case 7: *damage = 0; break;
            }
        }
        else if (g->coinResult == 4)
            Heal(g, g->current, 0, 10);
        break;
    case FX_DEVOLUTION_BEAM:
        if (after)
        {
            unsigned who = g->current ^ (action->option ? 1 : 0);
            if (action->option > 1 || action->target >= TCG_FIELD_SIZE || g->players[who].field[action->target].stackSize < 2)
                return TCG_INVALID;
            /* (the opponent's: not through Neutralizing Shield, Transparency, Agility...) */
            if (who != g->current &&
                (action->target ? TcgPower_Immune(g, who, action->target) : g->foeImmune == TCG_IMMUNE_YES))
                break;
            Devolve(g, who, action->target);
        }
        break;
    case FX_MIRROR_MOVE:
        if (!after)
        {
            /* What their last attack did to it, done back (MirrorMove_BeforeDamage) */
            if (self->mirrorEffect == TCG_MIRROR_AMNESIA)
            {
                const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
                if (!c || action->option > 1 || !RealAttack(&c->attacks[action->option]))
                    return TCG_INVALID;
                *damage = 0;
                Amnesia(g, action->option);
                break;
            }
            *damage = self->mirrorDamage;
            if (self->mirrorPoison)
                foe->poison = self->mirrorPoison;
            if (self->mirrorCondition)
                foe->condition = self->mirrorCondition;
            if (self->mirrorWeaken)
            {
                foe->weaken = self->mirrorWeaken;
                if (self->mirrorWeaken != TCG_WEAKEN_SMOKESCREEN)
                    foe->effectBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
            }
            if (self->mirrorNoAttack)
            {
                foe->noAttack = self->mirrorNoAttack;
                if (self->mirrorNoAttack == 2)
                    foe->effectBy = (uint8_t)(self->stack[self->stackSize - 1] + 1);
            }
            if (self->mirrorNoRetreat)
                foe->noRetreat = self->mirrorNoRetreat;
        }
        else if (foe->stackSize && g->foeImmune != TCG_IMMUNE_YES)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
            if (self->mirrorEffect == TCG_MIRROR_DISCARD_ENERGY)
            {
                int pick = -1;
                for (i = 0; i < foe->attachedSize; i++)
                {
                    const struct TcgCard *en = CardAt(g, g->current ^ 1, foe->attached[i]);
                    if (en->kind == TCG_ENERGY && (pick < 0 || en->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY))
                        pick = (int)i;
                }
                if (pick >= 0)
                    op->discard[op->discardSize++] = TcgPile_Remove(foe->attached, &foe->attachedSize, (unsigned)pick);
            }
            if (self->mirrorWeak && c && c->weakness != TCG_NONE)
                foe->weakAs = self->mirrorWeak;
        }
        break;
    case FX_GALE:
        if (after)
        {
            /* The Defending Pokemon with one of their Benched at random, then it with one of its own */
            if (g->foeImmune != TCG_IMMUNE_YES && foe->stackSize && BenchCount(g, g->current ^ 1) &&
                (s = RandomSlot(g, g->current ^ 1, 1)) != TCG_NONE)
                Switch(g, g->current ^ 1, s);
            if (BenchCount(g, g->current) && (s = RandomSlot(g, g->current, 1)) != TCG_NONE)
                Switch(g, g->current, s);
        }
        break;
    case FX_HURRICANE:
        if (after && foe->stackSize && g->foeImmune != TCG_IMMUNE_YES && HpLeft(g, g->current ^ 1, 0))
        {
            /* Unless it's Knocked Out: it and every card on it back to their hand */
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
            TcgDuel_Log(g, "Hurricane: %s returned to the hand.", c->name);
            for (i = 0; i < foe->stackSize; i++)
                op->hand[op->handSize++] = foe->stack[i];
            for (i = 0; i < foe->attachedSize; i++)
                op->hand[op->handSize++] = foe->attached[i];
            memset(foe, 0, sizeof(*foe));
            TcgDuel_Event(g, TCG_EV_TO_HAND, g->current ^ 1, 0, 0, c->id, 0);
        }
        break;
    case FX_METRONOME:
        break; /* (tcg_duel.c uses the copied attack in its place) */
    case FX_MORPH:
        if (after)
        {
            /* A copy of a Basic Pokemon (not Ditto) at random from the deck; the card itself stays there */
            uint8_t picks[TCG_DECK_SIZE];
            unsigned top;
            for (i = 0; i < p->deckSize; i++)
            {
                const struct TcgCard *c = CardAt(g, g->current, p->deck[i]);
                if (c->kind == TCG_POKEMON && !c->stage && c->id != TCG_CARD_DITTO)
                    picks[n++] = p->deck[i];
            }
            if (!n || !self->stackSize)
            {
                TcgDuel_Log(g, "Morph: there's no Basic Pokemon in the deck.");
                break;
            }
            /* (Clefable's Metronome copying it: the cards under it go to the discard pile) */
            while (self->stackSize > 1)
            {
                p->discard[p->discardSize++] = self->stack[0];
                memmove(self->stack, self->stack + 1, --self->stackSize);
            }
            top = self->stack[0];
            p->cards[top] = p->cards[picks[Tcg_RandomBelow(&g->rng, n)]];
            self->damage = 0;
            self->condition = TCG_HEALTHY;
            self->poison = 0;
            self->noAttack = self->noRetreat = self->weaken = self->guard = self->charged = self->effectBy = 0;
            self->disabled = self->weakAs = self->resistAs = self->burn = self->shift = 0;
            TcgDuel_Event(g, TCG_EV_MORPH, g->current, 0, 0, p->cards[top], 0);
            TcgDuel_Log(g, "Ditto morphed into %s.", CardAt(g, g->current, top)->name);
        }
        break;
    case FX_CONVERSION_1:
        if (after)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, 0);
            if (action->option >= TCG_COLORLESS)
                return TCG_INVALID;
            if (c && c->weakness != TCG_NONE && g->foeImmune != TCG_IMMUNE_YES)
            {
                foe->weakAs = (uint8_t)(action->option + 1);
                TcgDuel_Log(g, "%s's Weakness is now %s.", c->name, sTypeLabels[action->option]);
            }
        }
        break;
    case FX_CONVERSION_2:
        if (after)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current, 0);
            if (action->option >= TCG_COLORLESS)
                return TCG_INVALID;
            if (c && c->resistance != TCG_NONE)
            {
                self->resistAs = (uint8_t)(action->option + 1);
                TcgDuel_Log(g, "%s's Resistance is now %s.", c->name, sTypeLabels[action->option]);
            }
        }
        break;
    default:
    {
        int result = Gb2Effect(g, e, a, action, damage, after);
        if (result != TCG_OK)
            return result;
    }
    }
    /* (what can't be Asleep, Confused, Paralyzed or Poisoned) */
    TcgPower_Sanitize(g);
    return TCG_OK;
}
/* Does the attack do anything to the Defending Pokemon beyond its damage? (Haunter's
 * Transparency flips for every attack that does something to it.) */
int TcgDuel_EffectTouchesDefender(const struct TcgAttack *attack)
{
    const struct Effect *e = attack ? Effect(attack->effect) : NULL;
    if (!e)
        return 0;
    switch (e->kind)
    {
    case FX_PARALYZE_COIN: case FX_POISON: case FX_POISON_COIN: case FX_CONFUSE_COIN: case FX_SLEEP:
    case FX_SLEEP_COIN: case FX_CONFUSE_BOTH: case FX_FOUL_GAS: case FX_VENOM_POWDER: case FX_CLAMP:
    case FX_LURE: case FX_OPP_SWITCH: case FX_WEAKEN: case FX_NO_ATTACK_COIN: case FX_NO_RETREAT_COIN:
    case FX_HYPER_BEAM: case FX_PSYWAVE: case FX_SUPER_FANG: case FX_AMNESIA: case FX_MYSTERY_ATTACK:
    case FX_MIRROR_MOVE: case FX_GALE: case FX_HURRICANE: case FX_CONVERSION_1: case FX_DEVOLUTION_BEAM:
    case FX_STATUS_HT: case FX_STATUS_OR_RECOIL: case FX_HORRID_POLLEN: case FX_DEADLY_POISON: case FX_POISON_BENCH:
    case FX_DISCARD_FOE_ENERGY: case FX_BONUS_STATUS: case FX_LURE_COIN: case FX_MAGNETIC_LINES: case FX_TWISTER:
    case FX_DARK_SONG: case FX_KICK_STAMP: case FX_ROLL_OVER: case FX_BOTH_ASLEEP: case FX_TRANS_DAMAGE:
    case FX_FLING_DECK: case FX_BONE_HEADBUTT: case FX_STIR_UP: case FX_RANDOM_HITS: case FX_SPIN_SHOWER:
        return 1;
    }
    return 0;
}
