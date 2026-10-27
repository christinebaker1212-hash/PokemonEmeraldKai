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
    FX_HYPER_BEAM
};
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
/* Recoil: the attacker's own Weakness and Resistance to its own type apply
 * (ApplyDamageModifiers_DamageToSelf). */
void TcgDuel_SelfDamage(struct TcgDuel *g, int amount)
{
    const struct TcgCard *c = TcgDuel_Field(g, g->current, 0);
    struct TcgFieldCard *f = &g->players[g->current].field[0];
    unsigned type = TcgDuel_Type(g, g->current, 0);
    if (!c || amount <= 0)
        return;
    if (c->weakness == type)
        amount *= 2;
    if (c->resistance == type)
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
            *required = 1;
        return 2;
    }
    return 0;
}
static const char *const sFamily[][3] = {
    /* Call for Family: the names searched for, by command list. */
    {"NidoranFCallForFamilyEffectCommands", "Nidoran F", "Nidoran M"},
    {"BellsproutCallForFamilyEffectCommands", "Bellsprout", NULL},
    {"KrabbyCallForFamilyEffectCommands", "Krabby", NULL},
    {"OddishSproutEffectCommands", "Oddish", NULL},
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
        return BenchCount(g, player) ? NULL : "You have no Benched Pokemon.";
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
    }
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
        if (after)
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
            self->charged = 1;
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
    default:
        return TCG_UNSUPPORTED;
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
    case FX_HYPER_BEAM: case FX_PSYWAVE: case FX_SUPER_FANG:
        return 1;
    }
    return 0;
}
