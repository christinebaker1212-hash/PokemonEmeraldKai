#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/flow.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void TcgDuel_Log(struct TcgDuel *g, const char *format, ...)
{
    va_list ap;
    unsigned i;
    if (g->logCount == TCG_LOG_LINES)
    {
        for (i = 1; i < TCG_LOG_LINES; i++)
            memcpy(g->log[i - 1], g->log[i], TCG_LOG_WIDTH);
        g->logCount--;
    }
    va_start(ap, format);
    vsnprintf(g->log[g->logCount++], TCG_LOG_WIDTH, format, ap);
    va_end(ap);
}
void TcgDuel_Event(struct TcgDuel *g, unsigned type, unsigned player, unsigned slot, unsigned value, unsigned card,
                   unsigned flags)
{
    struct TcgEvent *e = &g->events[g->eventSeq++ % TCG_EVENT_RING];
    e->type = type;
    e->player = player;
    e->slot = slot;
    e->value = value;
    e->card = card;
    e->flags = flags;
    if ((type == TCG_EV_RETREAT || type == TCG_EV_SWITCH || type == TCG_EV_PROMOTE) && player < 2 && slot < TCG_FIELD_SIZE)
    {
        /* The one that went to the Bench loses what lasts only while it's Active (Giant Growth, Shadow Images) */
        if (type != TCG_EV_PROMOTE)
        {
            g->players[player].field[slot].hpSet = g->players[player].field[slot].guardKeep = 0;
            g->players[player].field[slot].lockOnUntil = 0;
            g->players[player].turnBuffs &= (uint8_t)~TCG_BUFF_ION;   /* (Ion Coating ends with that Active) */
            TcgPower_ToBench(g, player, slot); /* (Magma Pool) */
        }
        if (player == g->current && g->phase != TCG_SETUP)
            TcgPower_NewActive(g, player); /* (Spikes) */
        if (TcgPower_Working(g, player, 0) == TCG_POWER_R_LEGENDARY_BODY)
        {
            /* (Legendary Body: the Trainer cards attached to it discarded once it's the Active Pokemon) */
            unsigned tool;
            while (TcgDuel_Tool(g, player, 0, &tool))
                TcgDuel_DiscardAttached(g, player, 0, tool);
        }
    }
    if (type == TCG_EV_EVOLVE && player < 2 && player == g->current && g->phase == TCG_TURN)
    {
        /* Eevee's Chain Reaction: when a Pokemon evolves, a card from the deck that evolves from Eevee onto it */
        unsigned s;
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (s != slot && TcgPower_Working(g, player, s) == TCG_POWER_R_CHAIN_REACTION)
            {
                g->choiceData = (uint8_t)s;
                TcgDuel_AskChoice(g, player, TCG_CHOICE_CHAIN_REACTION, TcgDuel_Field(g, player, s)->id, 1);
                break;
            }
    }
}
const struct TcgEvent *TcgDuel_GetEvent(const struct TcgDuel *g, uint32_t seq)
{
    if (seq >= g->eventSeq || g->eventSeq - seq > TCG_EVENT_RING)
        return NULL;
    return &g->events[seq % TCG_EVENT_RING];
}
unsigned TcgDuel_Coin(struct TcgDuel *g, unsigned player)
{
    unsigned heads = Tcg_RandomBelow(&g->rng, 2);
    if (g->forceHeads && player == g->current)
        heads = 1; /* (Lock-on: tails are heads) */
    TcgDuel_Event(g, TCG_EV_COIN, player, 0, heads, 0, 0);
    if (g->coinCount < 32)
        g->coinBits |= (uint32_t)heads << g->coinCount;
    g->coinCount++;
    return heads;
}
uint8_t TcgPile_Remove(uint8_t *pile, uint8_t *count, unsigned index)
{
    uint8_t card = pile[index];
    memmove(pile + index, pile + index + 1, *count - index - 1);
    --*count;
    return card;
}
const struct TcgCard *TcgDuel_Field(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f;
    if (player > 1 || slot >= TCG_FIELD_SIZE)
        return NULL;
    f = &g->players[player].field[slot];
    return f->stackSize ? Tcg_Card(g->players[player].cards[f->stack[f->stackSize - 1]]) : NULL;
}
void TcgDuel_Shuffle(struct TcgDuel *g, unsigned player)
{
    struct TcgPlayer *p = &g->players[player];
    unsigned i;
    for (i = p->deckSize; i > 1; i--)
    {
        unsigned j = Tcg_RandomBelow(&g->rng, i);
        uint8_t c = p->deck[i - 1];
        p->deck[i - 1] = p->deck[j];
        p->deck[j] = c;
    }
}
unsigned TcgDuel_Draw(struct TcgDuel *g, unsigned player, unsigned count)
{
    struct TcgPlayer *p = &g->players[player];
    unsigned drawn = 0;
    while (count-- && p->deckSize)
    {
        p->hand[p->handSize++] = p->deck[--p->deckSize];
        drawn++;
    }
    return drawn;
}
static int HasBasic(const struct TcgPlayer *p)
{
    unsigned i;
    for (i = 0; i < p->handSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->hand[i]]);
        if (c->kind == TCG_POKEMON && !c->stage)
            return 1;
    }
    return 0;
}
static void ToolsBetweenTurns(struct TcgDuel *g, int start);
static void StartTurn(struct TcgDuel *g)
{
    struct TcgPlayer *p = &g->players[g->current];
    unsigned s;
    p->energyPlayed = 0;
    p->supporterPlayed = 0;
    p->confusionRetreatFailed = 0;
    p->retreated = 0;
    p->turnsTaken++;
    g->turn++;
    g->phase = TCG_TURN;
    g->needsTurnStart = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        /* Protection from the last turn ends; Swords Dance takes effect. */
        p->field[s].powerUsed = 0;
        p->field[s].barrier = 0;
        p->field[s].guard = 0;
        p->field[s].charged = p->field[s].charged == 1 ? 2 : 0;
        if (!p->field[s].charged)
            p->field[s].charge3 = 0;
        p->field[s].destinyBond = 0; /* (Destiny Bond lasted the opponent's turn) */
        p->field[s].mirrorShell = 0; /* (Mirror Shell too) */
    }
    TcgDuel_Event(g, TCG_EV_TURN, g->current, 0, g->turn, 0, 0);
    ToolsBetweenTurns(g, 1);
    /* (Rocket's Secret Experiment's tails: no Trainers this turn either) */
    if (p->noTrainersTurn == g->turn)
        p->noTrainers = 1;
    if (p->skipDrawTurn == g->turn)
    {
        /* Perplex: no card this turn */
        TcgDuel_Log(g, "Player %u doesn't draw a card this turn.", g->current + 1);
        return;
    }
    if (!TcgDuel_Draw(g, g->current, 1))
    {
        g->winner = g->current ^ 1;
        g->phase = TCG_FINISHED;
        TcgDuel_Log(g, "Player %u cannot draw a card.", g->current + 1);
        TcgDuel_Event(g, TCG_EV_WIN, g->winner, 0, 2, 0, 0);
        return;
    }
    TcgDuel_Event(g, TCG_EV_DRAW, g->current, 0, 1, 0, 0);
    TcgDuel_Log(g, "Player %u draws a card.", g->current + 1);
}
int TcgDuel_Init(struct TcgDuel *g, const uint16_t a[60], const uint16_t b[60], uint32_t seed, unsigned prizes)
{
    unsigned who, i, attempt;
    if (prizes < 1 || prizes > 6 || !Tcg_DeckValidate(a, NULL, NULL, 0) || !Tcg_DeckValidate(b, NULL, NULL, 0))
        return 0;
    memset(g, 0, sizeof(*g));
    g->rng = seed ? seed : 1;
    g->prizeCount = prizes;
    g->winner = TCG_NONE;
    g->first = g->current = Tcg_RandomBelow(&g->rng, 2);
    for (who = 0; who < 2; who++)
    {
        struct TcgPlayer *p = &g->players[who];
        memcpy(p->cards, who ? b : a, sizeof(p->cards));
        for (attempt = 0; attempt < 4096; attempt++)
        {
            p->deckSize = 60;
            p->handSize = 0;
            for (i = 0; i < 60; i++)
                p->deck[i] = i;
            TcgDuel_Shuffle(g, who);
            TcgDuel_Draw(g, who, 7);
            if (HasBasic(p))
                break;
            TcgDuel_Log(g, "Player %u has no Basic Pokemon and redraws.", who + 1);
        }
        if (attempt == 4096)
            return 0;
        for (i = 0; i < prizes; i++)
            p->prizes[p->prizeSize++] = p->deck[--p->deckSize];
    }
    g->phase = TCG_SETUP;
    g->current = 0;
    TcgDuel_Log(g, "Choose your Active Pokemon and up to five Benched Pokemon.");
    return 1;
}
unsigned TcgDuel_Energy(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned type)
{
    const struct TcgFieldCard *f;
    const struct TcgPlayer *p;
    unsigned i, n = 0;
    if (player > 1 || slot >= TCG_FIELD_SIZE)
        return 0;
    p = &g->players[player];
    f = &p->field[slot];
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[f->attached[i]]);
        /* (Charizard's Energy Burn: all of it is Fire Energy this turn; the regional bodies: Dark Aura...) */
        unsigned amount = c->kind == TCG_ENERGY ? Tcg_EnergyProvides(c->id) : 0;
        unsigned kind = f->burn ? TCG_FIRE : TcgPower_EnergyType(g, player, slot, c->type, &amount);
        if (c->kind == TCG_ENERGY && c->id > 6 && TcgPower_WindBlows(g))
            kind = TCG_COLORLESS;   /* (Miraculous Wind) */
        else if (c->kind == TCG_ENERGY && p->convCard == f->attached[i] + 1 && player == g->current &&
                 TcgPower_InPlay(g, player, TCG_POWER_R_ENERGY_CONVERTER))
            kind = p->convType;     /* (Energy Converter, while a Porygon2's power still works) */
        else if (kind == TCG_LIGHTNING && !slot && player == g->current && (p->turnBuffs & TCG_BUFF_ION))
            kind = TCG_WATER;       /* (Ion Coating) */
        if (c->kind == TCG_ENERGY && (type == TCG_NONE || type == kind))
            n += amount;
    }
    return n;
}
int TcgDuel_BoundEffect(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgFieldCard *f = &g->players[player].field[slot], *them = &g->players[player ^ 1].field[0];
    return f->effectBy && them->stackSize && them->stack[them->stackSize - 1] == f->effectBy - 1;
}
int TcgDuel_NoAttack(const struct TcgDuel *g, unsigned player)
{
    const struct TcgFieldCard *f = &g->players[player].field[0];
    const struct TcgCard *c = TcgDuel_Field(g, player, 0);
    return f->noAttack == 1 || (f->noAttack == 2 && TcgDuel_BoundEffect(g, player, 0)) || f->noAttackAll ||
           (c && (c->flags & TCG_CF_BABY) && TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_R_SCARE);
}
unsigned TcgDuel_MaxHp(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, slot);
    const struct TcgFieldCard *f;
    unsigned hp, k;
    if (!c)
        return 0;
    if (c->kind != TCG_POKEMON)
        return 10;
    f = &g->players[player].field[slot];
    hp = f->hpSet ? f->hpSet : c->hp;
    if (TcgDuel_Stadium(g, NULL) == TCG_CARD_NEO3_63 && (c->dark || !strncmp(c->name, "Dark ", 5)))
        hp += 20; /* (Rocket's Hideout) */
    if (TcgPower_Working(g, player, slot) == TCG_POWER_R_PSYCHIC_HP)
        for (k = 0; k < f->attachedSize; k++)
            hp += g->players[player].cards[f->attached[k]] == TCG_CARD_PSYCHIC_ENERGY ? 10 : 0;
    return hp;
}
int TcgDuel_Anchored(const struct TcgDuel *g, unsigned player)
{
    const struct TcgFieldCard *f = &g->players[player].field[0], *them = &g->players[player ^ 1].field[0];
    return f->stackSize && f->anchorUntil >= g->turn &&
           (!f->anchorBy || (them->stackSize && them->stack[them->stackSize - 1] == f->anchorBy - 1));
}
int TcgDuel_Stuck(const struct TcgDuel *g, unsigned player)
{
    const struct TcgFieldCard *f = &g->players[player].field[0], *them = &g->players[player ^ 1].field[0];
    return f->stuck == 1 || TcgDuel_Anchored(g, player) || TcgPower_Working(g, player ^ 1, 0) == TCG_POWER_R_GUARD ||
           (f->stuck == 2 && them->stackSize && them->stack[them->stackSize - 1] == f->stuckBy - 1);
}
static int EnergyPaid(const struct TcgDuel *g, unsigned player, unsigned attack);
/* (Hypno's Puppet Master: a Clefairy Doll uses Hypno's attacks, paid with Hypno's Energy) */
static unsigned EnergySlot(const struct TcgDuel *g, unsigned player)
{
    unsigned s = TcgPower_PuppetSlot(g, player);
    return s == TCG_NONE ? 0 : s;
}
const struct TcgCard *TcgDuel_Attacker(const struct TcgDuel *g, unsigned player)
{
    return TcgDuel_Field(g, player, EnergySlot(g, player));
}
/* Slashing Strike, Drill Dive and the like: that attack (lockAttack TCG_LOCK_ALL: any) can't be used this turn */
static int Locked(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgFieldCard *f = &g->players[player].field[0];
    return f->lockTurn == g->turn && (f->lockAttack == TCG_LOCK_ALL || f->lockAttack == TCG_LOCK_ATTACKS || f->lockAttack == attack + 1);
}
unsigned TcgDuel_Tm(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned *physical)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[f->attached[i]]);
        if (c && c->kind == TCG_TRAINER && (c->flags & TCG_CF_TM))
        {
            if (physical)
                *physical = f->attached[i];
            return c->id;
        }
    }
    return 0;
}
int TcgDuel_TmFits(const struct TcgDuel *g, unsigned who, unsigned slot, const struct TcgCard *tm)
{
    static const char *const types[TCG_TYPES] = {"Fire", "Grass", "Lightning", "Water", "Fighting", "Psychic",
                                                 "Colorless", "Darkness", "Metal"};
    const struct TcgCard *c = slot < TCG_FIELD_SIZE ? TcgDuel_Field(g, who, slot) : NULL;
    const char *words = tm && tm->description ? tm->description : "", *of = strstr(words, "Attach this card to 1 of your ");
    unsigned t, named = 0, fits = 0;
    if (!c || c->kind != TCG_POKEMON)
        return 0;
    if (!of || TcgPower_Working(g, who, slot) == TCG_POWER_R_SYNCHRONICITY) /* (Xatu's Synchronicity: any) */
        return 1;
    of += strlen("Attach this card to 1 of your ");
    if (!strncmp(of, "Evolved ", 8))
    {
        if (!c->stage && g->players[who].field[slot].stackSize < 2)
            return 0;
        of += 8;
    }
    /* (the types named before "Pokemon in play") */
    for (t = 0; t < TCG_TYPES; t++)
    {
        const char *at = strstr(of, types[t]), *end = strstr(of, "Pokemon");
        if (at && end && at < end)
            named++, fits |= TcgDuel_Type(g, who, slot) == t;
    }
    return !named || fits;
}
/* The want-th borrowed attack of that player's Active Pokemon (TCG_ATTACK_BORROW + want) */
static const struct TcgAttack *Borrowed(const struct TcgDuel *g, unsigned player, unsigned want, unsigned *card)
{
    const struct TcgPlayer *p = &g->players[player];
    const struct TcgFieldCard *f = &p->field[0];
    const struct TcgCard *top = TcgDuel_Field(g, player, 0);
    unsigned n = 0, k, i, s, power;
    if (!top || top->kind != TCG_POKEMON || EnergySlot(g, player) || want >= TCG_BORROW_MAX)
        return NULL;
    power = TcgPower_Working(g, player, 0);
    /* Recall (this turn), Memory Berry, Prehistoric Memory (either side's): the attacks of the cards under it */
    if (f->stackSize > 1 && ((p->turnBuffs & TCG_BUFF_RECALL) || TcgDuel_AttachedCard(g, player, 0, TCG_CARD_ECARD2_128) >= 0 ||
                             TcgPower_InPlay(g, 2, TCG_POWER_R_PREHISTORIC_MEMORY)))
        for (k = f->stackSize - 1; k-- > 0;)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[f->stack[k]]);
            for (i = 0; c && c->kind == TCG_POKEMON && i < TCG_ATTACKS; i++)
                if (c->attacks[i].name[0] && c->attacks[i].category != TCG_POKEMON_POWER && n++ == want)
                {
                    if (card)
                        *card = c->id | i << 16; /* (whose attack: for the board's banner) */
                    return &c->attacks[i];
                }
        }
    /* Psylink: your other Psychic Pokemon's attacks */
    if (power == TCG_POWER_R_PSYLINK)
        for (s = 1; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, player, s);
            if (!c || c->kind != TCG_POKEMON || TcgDuel_Type(g, player, s) != TCG_PSYCHIC)
                continue;
            for (i = 0; i < TCG_ATTACKS; i++)
                if (c->attacks[i].name[0] && c->attacks[i].category != TCG_POKEMON_POWER && n++ == want)
                {
                    if (card)
                        *card = c->id | i << 16; /* (whose attack: for the board's banner) */
                    return &c->attacks[i];
                }
        }
    /* Psymimic (not with a Special Condition): any of their Pokemon's attacks */
    if (power == TCG_POWER_R_PSYMIMIC && !f->condition && !f->poison && !f->burned)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, player ^ 1, s);
            for (i = 0; c && c->kind == TCG_POKEMON && i < TCG_ATTACKS; i++)
                if (c->attacks[i].name[0] && c->attacks[i].category != TCG_POKEMON_POWER && n++ == want)
                {
                    if (card)
                        *card = c->id | i << 16;
                    return &c->attacks[i];
                }
        }
    /* Mimic: the Defending Pokemon's attacks */
    if (power == TCG_POWER_R_MIMIC)
    {
        const struct TcgCard *c = TcgDuel_Field(g, player ^ 1, 0);
        for (i = 0; c && c->kind == TCG_POKEMON && i < TCG_ATTACKS; i++)
            if (c->attacks[i].name[0] && c->attacks[i].category != TCG_POKEMON_POWER && n++ == want)
            {
                if (card)
                    *card = c->id | i << 16;
                return &c->attacks[i];
            }
    }
    return NULL;
}
const struct TcgAttack *TcgDuel_AttackOf(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, player), *tm;
    if (!c || c->kind != TCG_POKEMON)
        return NULL;
    if (attack < TCG_ATTACKS)
        return &c->attacks[attack];
    if (attack >= TCG_ATTACK_BORROW && attack < TCG_ATTACK_SLOTS)
        return Borrowed(g, player, attack - TCG_ATTACK_BORROW, NULL);
    if (attack != TCG_ATTACK_TM || !(tm = Tcg_Card(TcgDuel_Tm(g, player, 0, NULL))) || !tm->attacks[0].name ||
        !tm->attacks[0].name[0])
        return NULL;
    return &tm->attacks[0];
}
int TcgDuel_CanAttack(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgAttack *a = TcgDuel_AttackOf(g, player, attack);
    /* (Amnesia: not the attack chosen, this turn) */
    return a && EnergyPaid(g, player, attack) && g->players[player].field[0].disabled != attack + 1 &&
           !Locked(g, player, attack) && !TcgDuel_EffectBlock(g, player, a);
}
int TcgDuel_CanAttackNow(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgAttack *a = TcgDuel_AttackOf(g, player, attack);
    const struct TcgFieldCard *f = &g->players[player].field[0];
    if (!a || f->condition == TCG_PARALYZED || TcgDuel_NoAttack(g, player) || Locked(g, player, attack))
        return 0;
    /* (Big Snore: only while Asleep) */
    return f->condition != TCG_ASLEEP || TcgDuel_SleepAttack(a);
}
static int EnergyPaidFor(const struct TcgDuel *g, unsigned player, const struct TcgAttack *a);
static int EnergyPaid(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, player);
    if (!c || c->kind != TCG_POKEMON)
        return 0;
    return EnergyPaidFor(g, player, TcgDuel_AttackOf(g, player, attack));
}
/* Does that player's Active's Energy pay for this attack's cost? */
static int EnergyPaidFor(const struct TcgDuel *g, unsigned player, const struct TcgAttack *a)
{
    unsigned t, total = 0, typed = 0, from = EnergySlot(g, player), rainbow = 0, short_ = 0, i, crystal = 0, shortBare = 0;
    const struct TcgFieldCard *f = &g->players[player].field[from];
    /* (Rainbow Energy: any one type each; Crystal Energy: one of the types of the basic Energy cards on it) */
    for (i = 0; i < f->attachedSize && !TcgPower_WindBlows(g); i++)
    {
        unsigned rule = Tcg_EnergyRule(g->players[player].cards[f->attached[i]]);
        rainbow += rule == TCG_ER_RAINBOW ? 1 : rule == TCG_ER_MIRACLE ? 2 : 0;   /* (Miracle Energy: every type, 2) */
        crystal += rule == TCG_ER_CRYSTAL;
    }
    if (!a || !a->name[0] || a->category == TCG_POKEMON_POWER)
        return 0;
    for (t = 0; t < TCG_TYPES; t++)
    {
        total += a->cost[t];
        if (t != TCG_COLORLESS)
        {
            unsigned have = TcgDuel_Energy(g, player, from, t), k, basic = 0;
            for (k = 0; k < f->attachedSize; k++)
                basic |= g->players[player].cards[f->attached[k]] <= 6 &&
                         Tcg_Card(g->players[player].cards[f->attached[k]])->type == t;
            if (have < a->cost[t])
            {
                short_ += a->cost[t] - have;
                if (!basic)
                    shortBare += a->cost[t] - have; /* (Crystal Energy can't be this type) */
            }
            typed += a->cost[t];
        }
    }
    (void)typed;
    if (TcgPower_Working(g, player, from) == TCG_POWER_R_PRISMATIC_BODY)
    {
        /* (Ditto's Prismatic Body: each basic Energy card any one type; the other Energy pay their own) */
        unsigned basics = 0, need = 0, k;
        for (k = 0; k < f->attachedSize; k++)
            basics += g->players[player].cards[f->attached[k]] <= 6;
        for (t = 0; t < TCG_TYPES; t++)
            if (t != TCG_COLORLESS && a->cost[t])
            {
                unsigned special = 0;
                for (k = 0; k < f->attachedSize; k++)
                {
                    const struct TcgCard *e = Tcg_Card(g->players[player].cards[f->attached[k]]);
                    special += e->kind == TCG_ENERGY && e->id > 6 && e->type == t ? Tcg_EnergyProvides(e->id) : 0;
                }
                need += a->cost[t] > special ? a->cost[t] - special : 0;
            }
        return need <= basics + rainbow && TcgDuel_Energy(g, player, from, TCG_NONE) >= total;
    }
    return short_ <= rainbow + crystal && shortBare <= rainbow && TcgDuel_Energy(g, player, from, TCG_NONE) >= total;
}
void TcgDuel_DiscardField(struct TcgDuel *g, unsigned player, unsigned slot)
{
    struct TcgPlayer *p = &g->players[player];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->stackSize; i++)
        p->discard[p->discardSize++] = f->stack[i];
    for (i = 0; i < f->attachedSize; i++)
        p->discard[p->discardSize++] = f->attached[i];
    memset(f, 0, sizeof(*f));
}
void TcgDuel_Damage(struct TcgDuel *g, unsigned player, unsigned slot, unsigned amount)
{
    if (TcgDuel_Field(g, player, slot))
        g->players[player].field[slot].damage += amount;
}
static unsigned PokemonCount(const struct TcgPlayer *p)
{
    unsigned i, n = 0;
    for (i = 0; i < TCG_FIELD_SIZE; i++)
        n += p->field[i].stackSize != 0;
    return n;
}
/* A Pokemon of who's Knocked Out by the other player's attack, before its cards are discarded: Time Shard (up to 2
 * basic Energy cards of it into the hand), EXP.ALL on another of theirs (a basic Energy of the Active onto that one; the
 * EXP.ALL discarded). Both "may": always taken, the basic Energy cards first in the order attached. */
static void KnockedOutByAttack(struct TcgDuel *g, unsigned who, unsigned s)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[s];
    unsigned tool, k, n = 0, t;
    if (TcgDuel_Tool(g, who, s, &tool) == TCG_CARD_ECARD2_135)
    {
        for (k = 0; k < f->attachedSize && n < 2;)
        {
            if (p->cards[f->attached[k]] >= 1 && p->cards[f->attached[k]] <= 6)
            {
                p->hand[p->handSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
                n++;
            }
            else
                k++;
        }
    }
    if (s)
        return;
    for (t = 1; t < TCG_FIELD_SIZE; t++)
        if (TcgDuel_Tool(g, who, t, &tool) == TCG_CARD_NEO4_93)
        {
            for (k = 0; k < f->attachedSize && !(p->cards[f->attached[k]] >= 1 && p->cards[f->attached[k]] <= 6); k++)
                ;
            if (k < f->attachedSize)
            {
                struct TcgFieldCard *to = &p->field[t];
                to->attached[to->attachedSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
                TcgDuel_Event(g, TCG_EV_ATTACH, who, t, 0, p->cards[to->attached[to->attachedSize - 1]], 0);
                TcgDuel_DiscardAttached(g, who, t, tool);
            }
            break;
        }
}
void TcgDuel_ResolveKnockouts(struct TcgDuel *g)
{
    unsigned who, s, win[2] = {0};
    if (g->phase == TCG_CHOICE)
        return; /* (after the choice: Apply's TCG_CHOICE branch) */
    /* Resolve both boards before checking victory; simultaneous KOs must not
     * award victory to whichever side happened to be scanned first. */
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, who, s);
            struct TcgFieldCard *f = &g->players[who].field[s];
            unsigned hp = TcgDuel_MaxHp(g, who, s);
            if (c && f->damage >= hp)
            {
                struct TcgPlayer *winner = &g->players[who ^ 1];
                /* (a Pokemon-ex: 2 Prizes) */
                unsigned prize = f->noPrize ? 0 : c->kind == TCG_POKEMON && (c->flags & TCG_CF_EX) ? 2 : 1;
                int rebirth = TcgPower_Working(g, who, s) == TCG_POWER_R_REBIRTH;
                uint8_t top = f->stack[f->stackSize - 1];
                TcgDuel_Log(g, "%s is Knocked Out.", c->name);
                if (who != g->current && g->attackTurn == g->turn)
                    KnockedOutByAttack(g, who, s);   /* (Time Shard, EXP.ALL) */
                TcgDuel_Event(g, TCG_EV_KO, who, s, 0, c->id, 0);
                TcgDuel_DiscardField(g, who, s);
                if (rebirth)
                {
                    /* (Rocket's Moltres's Rebirth: back to the hand after it's discarded) */
                    struct TcgPlayer *q = &g->players[who];
                    unsigned k;
                    for (k = q->discardSize; k > 0; k--)
                        if (q->discard[k - 1] == top)
                        {
                            q->hand[q->handSize++] = TcgPile_Remove(q->discard, &q->discardSize, k - 1);
                            TcgDuel_Log(g, "Rebirth: %s returns to the hand.", c->name);
                            break;
                        }
                }
                while (prize-- && winner->prizeSize)
                {
                    winner->hand[winner->handSize++] = winner->prizes[--winner->prizeSize];
                    TcgDuel_Event(g, TCG_EV_PRIZE, who ^ 1, 0, winner->prizeSize, 0, 0);
                }
            }
        }
    for (who = 0; who < 2; who++)
        win[who] = !g->players[who].prizeSize || !PokemonCount(&g->players[who ^ 1]);
    if (win[0] && win[1])
    {
        uint16_t decks[2][60];
        uint32_t rng = g->rng, seq = g->eventSeq;
        struct TcgEvent events[TCG_EVENT_RING];
        memcpy(decks[0], g->players[0].cards, sizeof(decks[0]));
        memcpy(decks[1], g->players[1].cards, sizeof(decks[1]));
        memcpy(events, g->events, sizeof(events));
        TcgDuel_Init(g, decks[0], decks[1], rng, 1);
        memcpy(g->events, events, sizeof(events));
        g->eventSeq = seq;
        TcgDuel_Event(g, TCG_EV_SUDDEN_DEATH, 0, 0, 0, 0, 0);
        TcgDuel_Log(g, "Sudden death: play again with one Prize card.");
        return;
    }
    if (win[0] || win[1])
    {
        g->winner = win[0] ? 0 : 1;
        g->phase = TCG_FINISHED;
        TcgDuel_Log(g, "Player %u wins the duel.", g->winner + 1);
        TcgDuel_Event(g, TCG_EV_WIN, g->winner, 0, g->players[g->winner].prizeSize ? 1 : 0, 0, 0);
        return;
    }
    for (who = 0; who < 2; who++)
        if (!g->players[who].field[0].stackSize)
        {
            g->phase = TCG_PROMOTION;
            g->promotePlayer = who;
            return;
        }
}
static void ClearCondition(struct TcgFieldCard *f)
{
    f->condition = TCG_HEALTHY;
    f->poison = 0;
    f->burned = 0;
    f->barrier = 0;
    f->noRetreat = 0;
    f->noAttack = 0;
    f->guard = 0;
    f->weaken = 0;
    f->charged = 0;
    f->effectBy = 0;
    f->burn = f->shift = 0;
    f->disabled = f->destinyBond = f->weakAs = f->resistAs = 0;
    f->lockTurn = f->lockAttack = f->mirrorShell = f->charge3 = f->fossilBoost = 0;
    f->stuck = f->stuckBy = f->blindAll = f->noAttackAll = 0;
    f->noEvolveUntil = 0;
    f->condImmune = 0;
    f->anchorUntil = f->anchorBy = 0;
    f->reactUntil = f->reactKind = 0;
    f->hpSet = f->guardKeep = 0;
    f->lockOnUntil = 0;
    if (f->boostTurn == 0xFFFF)
        f->boostTurn = f->boostAttack = f->boostValue = 0;
}
/* What the opponent's last attack did to it (Mirror Move) */
static void ClearMirror(struct TcgFieldCard *f)
{
    f->mirrorDamage = 0;
    f->mirrorCondition = f->mirrorPoison = f->mirrorWeaken = f->mirrorNoAttack = f->mirrorNoRetreat = 0;
    f->mirrorEffect = f->mirrorArg = f->mirrorWeak = 0;
}
static unsigned AttachedTrainerCount(const struct TcgDuel *g, unsigned player, unsigned slot, unsigned id)
{
    const struct TcgPlayer *p = &g->players[player];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i, count = 0;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Canon(p->cards[f->attached[i]]) == id)
            count++;
    return count;
}
int TcgDuel_AttachedCard(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned canon)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (Tcg_Canon(p->cards[f->attached[i]]) == canon)
            return f->attached[i];
    return -1;
}
void TcgDuel_DiscardAttached(struct TcgDuel *g, unsigned who, unsigned slot, unsigned physical)
{
    struct TcgPlayer *p = &g->players[who];
    struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
        if (f->attached[i] == physical)
        {
            p->attachmentExpires[physical] = 0;
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i);
            return;
        }
}
static int Named(const struct TcgCard *c, const char *word) { return c && strstr(c->name, word) != NULL; }
/* The regional Pokemon Tools that act between turns: Berry, Gold Berry, Miracle Berry at the start of each turn;
 * Healing Berry at the end of any */
static void ToolsBetweenTurns(struct TcgDuel *g, int start)
{
    unsigned who, s;
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            struct TcgFieldCard *f = &g->players[who].field[s];
            const struct TcgCard *c = TcgDuel_Field(g, who, s);
            unsigned physical, tool = c ? TcgDuel_Tool(g, who, s, &physical) : 0, heal = 0;
            if (!tool)
                continue;
            if (start && (tool == TCG_CARD_NEO1_99 || tool == TCG_CARD_NEO1_93))
            {
                unsigned need = tool == TCG_CARD_NEO1_99 ? 20 : 40;
                if (f->damage < need)
                    continue;
                heal = need;
            }
            else if (start && tool == TCG_CARD_NEO1_94)
            {
                if (!f->condition && !f->poison && !f->burned)
                    continue;
                f->condition = TCG_HEALTHY;
                f->poison = f->burned = 0;
                TcgDuel_Event(g, TCG_EV_STATUS, who, s, TCG_HEALTHY, 0, 0);
            }
            else if (!start && tool == TCG_CARD_ECARD2_125 && c->kind == TCG_POKEMON && f->damage &&
                     f->damage < TcgDuel_MaxHp(g, who, s) && TcgDuel_MaxHp(g, who, s) - f->damage <= 20)
                heal = 30;
            else
                continue;
            if (heal)
            {
                heal = heal < f->damage ? heal : f->damage;
                f->damage -= heal;
                TcgDuel_Event(g, TCG_EV_HEAL, who, s, heal, 0, 0);
            }
            TcgDuel_Log(g, "%s's %s.", c->name, Tcg_Card(tool)->name);
            TcgDuel_DiscardAttached(g, who, s, physical);
        }
}

int TcgDuel_CanEvolveNow(const struct TcgDuel *g, unsigned who, unsigned slot)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    return f->stackSize && f->noEvolveUntil < g->turn && !TcgPower_EvolveBlocked(g, who, slot) &&
           ((p->turnsTaken >= 2 && f->enteredTurn != g->turn) || f->freeEvolve == g->turn + 1u);
}
int TcgDuel_LaterStage(const struct TcgCard *evolution, const struct TcgCard *from)
{
    unsigned i;
    if (!evolution || !from || evolution->kind != TCG_POKEMON || !evolution->stage)
        return 0;
    if (TcgDuel_EvolvesFrom(evolution, from))
        return 1;
    if (evolution->stage != 2)
        return 0;
    for (i = 1; i <= TCG_CARD_COUNT; i++)
    {
        const struct TcgCard *mid = Tcg_CardAny(i);
        if (mid && mid->kind == TCG_POKEMON && mid->stage == 1 && !strcmp(mid->name, evolution->evolvesFrom) &&
            TcgDuel_EvolvesFrom(mid, from))
            return 1;
    }
    return 0;
}

unsigned TcgDuel_Stadium(const struct TcgDuel *g, unsigned *owner)
{
    unsigned who;
    for (who = 0; who < 2; who++)
        if (g->players[who].stadium)
        {
            if (owner)
                *owner = who;
            return Tcg_Canon(g->players[who].cards[g->players[who].stadium - 1]);
        }
    return 0;
}
unsigned TcgDuel_Tool(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned *physical)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[f->attached[i]]);
        if (c && c->kind == TCG_TRAINER && (c->flags & TCG_CF_TOOL))
        {
            if (physical)
                *physical = f->attached[i];
            return Tcg_Canon(c->id);
        }
    }
    return 0;
}
/* How many Energy cards attached to that Pokemon have this rule (Tcg_EnergyRule) */
unsigned TcgDuel_AttachedRule(const struct TcgDuel *g, unsigned who, unsigned slot, unsigned rule)
{
    const struct TcgPlayer *p = &g->players[who];
    const struct TcgFieldCard *f = &p->field[slot];
    unsigned i, n = 0;
    for (i = 0; i < f->attachedSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[f->attached[i]]);
        n += c && c->kind == TCG_ENERGY && Tcg_EnergyRule(c->id) == rule;
    }
    /* (Miraculous Wind: Special Energy only provides; Double Colorless's two is what it provides) */
    if (n && rule != TCG_ER_BASIC && rule != TCG_ER_DOUBLE_COLORLESS && TcgPower_WindBlows(g))
        return 0;
    return n;
}
int TcgDuel_EvolvesFrom(const struct TcgCard *evolution, const struct TcgCard *from)
{
    /* the Babies and what they evolve into (by species: Pichu, Cleffa, Igglybuff, Tyrogue, Smoochum, Elekid, Magby) */
    static const uint16_t babies[][4] = {{172, 25}, {173, 35}, {174, 39}, {236, 106, 107, 237}, {238, 124}, {239, 125},
                                         {240, 126}};
    unsigned i, k;
    if (!evolution || !from || evolution->kind != TCG_POKEMON || from->kind != TCG_POKEMON)
        return 0;
    if (evolution->stage)
        return !strcmp(evolution->evolvesFrom, from->name);
    if (!(from->flags & TCG_CF_BABY))
        return 0;
    for (i = 0; i < sizeof(babies) / sizeof(babies[0]); i++)
        if (babies[i][0] == from->species)
            for (k = 1; k < 4 && babies[i][k]; k++)
                if (babies[i][k] == evolution->species && !(evolution->flags & TCG_CF_BABY))
                    return 1;
    return 0;
}
static int DarkPokemon(const struct TcgCard *c)
{
    return c->type == TCG_DARKNESS || c->dark || strstr(c->name, "Dark") != NULL;
}
#include "tcg_heights.inc"
unsigned TcgDuel_TopCount(const struct TcgDuel *g, unsigned deckSize)
{
    unsigned n = g->phase == TCG_CHOICE && g->choiceKind == TCG_CHOICE_ORDER_THEIRS ? g->choiceData : 3u;
    return deckSize < n ? deckSize : n;
}
unsigned TcgDuel_QuizOptions(const struct TcgDuel *g, uint16_t out[4])
{
    /* (the quiz's player is the one not answering) */
    unsigned owner = g->choicePlayer ^ 1, k = 0, tries, truth, id;
    const struct TcgCard *c = Tcg_Card(g->players[owner].cards[g->choiceData]);
    uint32_t seed = (uint32_t)(c ? c->id : 0) * 2654435761u + g->turn * 40503u + 7u;
    if (!c)
        return 0;
    truth = seed % 4;
    if (g->choiceKind == TCG_CHOICE_QUIZ1)
    {
        unsigned h = c->species < TCG_HEIGHT_SPECIES ? TCG_HEIGHTS[c->species] : 0;
        out[truth] = (uint16_t)(h ? h : 1);
        for (tries = 0; k < 4 && tries < 400; tries++)
        {
            unsigned other, j, same = 0;
            seed = seed * 1103515245u + 12345u;
            other = TCG_HEIGHTS[1 + (seed >> 8) % (TCG_HEIGHT_SPECIES - 1)];
            if (k == truth)
                k++;
            for (j = 0; j < 4; j++)
                same |= (j < k || j == truth) && out[j] == other;
            if (!other || same || k >= 4)
                continue;
            out[k++] = (uint16_t)other;
        }
        return truth;
    }
    out[truth] = c->id;
    for (tries = 0; k < 4 && tries < 4000; tries++)
    {
        const struct TcgCard *o;
        unsigned j, same = 0;
        seed = seed * 1103515245u + 12345u;
        id = 1 + (seed >> 8) % TCG_CARD_COUNT;
        o = Tcg_Card(id);
        if (k == truth)
            k++;
        if (k >= 4)
            break;
        if (!o || o->kind != TCG_POKEMON || o->type != c->type)
            continue;
        for (j = 0; j < 4; j++)
            same |= (j < k || j == truth) && !strcmp(Tcg_Card(out[j])->name, o->name);
        if (!same)
            out[k++] = (uint16_t)id;
    }
    while (k < 4)
    {
        if (k != truth)
            out[k] = c->id;
        k++;
    }
    return truth;
}
static void EndTurn(struct TcgDuel *g)
{
    unsigned who, s;
    struct TcgPlayer *p = &g->players[g->current];
    g->attackTurn = 0;
    g->heldAsked = g->heldPending = g->vermilion = 0;
    for (s = 0; s < TCG_FIELD_SIZE && g->phase != TCG_FINISHED && g->phase != TCG_SETUP; s++)
    {
        /* Charity back to its player's hand, Sabrina's ESP discarded, as their turn ends */
        int at;
        struct TcgFieldCard *f = &p->field[s];
        if ((at = TcgDuel_AttachedCard(g, g->current, s, TCG_CARD_GYM1_99)) >= 0)
        {
            unsigned k;
            for (k = 0; k < f->attachedSize; k++)
                if (f->attached[k] == (uint8_t)at)
                {
                    p->hand[p->handSize++] = TcgPile_Remove(f->attached, &f->attachedSize, k);
                    TcgDuel_Log(g, "Charity goes back to the hand.");
                    break;
                }
        }
        if ((at = TcgDuel_AttachedCard(g, g->current, s, TCG_CARD_GYM1_117)) >= 0)
            TcgDuel_DiscardAttached(g, g->current, s, (unsigned)at);
    }
    if (g->phase == TCG_TURN)
    {
        /* Star Piece (between turns): on a Benched Pokemon with 2 or more damage counters, an evolution of it from the
         * deck, its player's pick (tcg_choice.inc); the Star Piece discarded */
        unsigned w, s2;
        for (w = 0; w < 2; w++)
            for (s2 = 1; s2 < TCG_FIELD_SIZE; s2++)
            {
                unsigned piece;
                if (g->players[w].field[s2].damage < 20 || !TcgDuel_Tool(g, w, s2, &piece) ||
                    Tcg_Canon(g->players[w].cards[piece]) != TCG_CARD_ECARD3_139)
                    continue;
                TcgDuel_DiscardAttached(g, w, s2, piece);
                g->choiceData = (uint8_t)s2;
                TcgDuel_AskChoice(g, w, TCG_CHOICE_STAR_PIECE, TCG_CARD_ECARD3_139, 1);
                if (g->phase == TCG_CHOICE)
                {
                    g->choiceEndTurn = 1;
                    return;
                }
            }
    }
    if (g->phase == TCG_TURN && p->handSize > 7 && TcgDuel_Stadium(g, NULL) == TCG_CARD_NP_28)
    {
        /* Championship Arena: down to 7 in the hand first, the player's pick; then the turn ends (TcgDuel_Apply) */
        TcgDuel_AskChoice(g, g->current, TCG_CHOICE_ARENA, TCG_CARD_NP_28, 1);
        if (g->phase == TCG_CHOICE)
        {
            g->choiceEndTurn = 1;
            return;
        }
    }
    if (p->field[0].stackSize && p->field[0].condition == TCG_PARALYZED)
    {
        p->field[0].condition = TCG_HEALTHY;
        TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, TCG_HEALTHY, 0, p->field[0].poison);
    }
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        /* What the opponent's last attack did to this side lasts this turn. */
        p->field[s].plusPower = 0;
        p->field[s].noAttack = 0;
        p->field[s].noRetreat = 0;
        p->field[s].weaken = 0;
        p->field[s].effectBy = 0;
        /* Energy Burn and Shift last the rest of the turn */
        p->field[s].burn = p->field[s].shift = 0;
        /* Amnesia lasted this turn; what was done to it last turn can't be Mirror Moved any more */
        p->field[s].disabled = 0;
        ClearMirror(&p->field[s]);
        if (p->field[s].charged == 2)
            p->field[s].charged = p->field[s].charge3 = 0;
        p->field[s].fossilBoost = 0;
        /* (Technical Machines: "at the end of your turn, discard" them) */
        {
            unsigned tm;
            while (TcgDuel_Tm(g, g->current, s, &tm))
            {
                unsigned k;
                for (k = 0; k < p->field[s].attachedSize && p->field[s].attached[k] != tm; k++)
                    ;
                p->discard[p->discardSize++] = TcgPile_Remove(p->field[s].attached, &p->field[s].attachedSize, k);
            }
        }
    }
    p->noTrainers = p->noSupporters = 0;
    p->turnBuffs = p->stadiumUsed = 0;
    p->convCard = 0;
    if (p->asideSize && p->asideUntil <= g->turn)
    {
        /* (Tickling Machine: the hand set aside comes back at the end of this player's turn) */
        while (p->asideSize)
            p->hand[p->handSize++] = p->aside[--p->asideSize];
        p->asideUntil = 0;
    }   /* (Energy Converter: until the end of the turn) */
    /* (the turn's Supporter goes to the discard pile as the turn ends) */
    if (p->supporterCard)
    {
        p->discard[p->discardSize++] = (uint8_t)(p->supporterCard - 1);
        p->supporterCard = 0;
    }
    for (who = 0; who < 2; who++)
    {
        struct TcgFieldCard *f = &g->players[who].field[0];
        if (!f->stackSize)
            continue;
        if (f->poison)
        {
            /* (Weezing's Poison Mist: 20 instead of 10) */
            unsigned amount = f->poison == 10 && (g->players[0].poisonMistUntil >= g->turn ||
                                                  g->players[1].poisonMistUntil >= g->turn) ? 20u : f->poison;
            f->damage += amount;
            TcgDuel_Event(g, TCG_EV_DAMAGE, who, 0, amount, 0, TCG_DMG_POISON);
        }
        if (f->burned && !TcgDuel_Coin(g, who))
        {
            /* Burned (the regional sets): tails, 2 damage counters */
            f->damage += 20;
            TcgDuel_Event(g, TCG_EV_DAMAGE, who, 0, 20, 0, TCG_DMG_POISON);
        }
        if (f->condition == TCG_ASLEEP && TcgDuel_Coin(g, who) &&
            (!TcgPower_InPlay(g, 2, TCG_POWER_R_DEEP_SLEEP) || TcgDuel_Coin(g, who))) /* (Deep Sleep: 2 coins) */
        {
            f->condition = TCG_HEALTHY;
            TcgDuel_Event(g, TCG_EV_STATUS, who, 0, TCG_HEALTHY, 0, f->poison);
        }
        if (f->defender)
            f->defender--;
    }
    ToolsBetweenTurns(g, 0);
    TcgPower_BetweenTurns(g); /* (Rain Dish, Burning Aura) */
    /* Darkness Energy: at the end of every turn, a damage counter on a Pokemon that isn't Darkness or Dark */
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, who, s);
            unsigned n = c ? TcgDuel_AttachedRule(g, who, s, TCG_ER_DARKNESS) : 0;
            if (n && c->kind == TCG_POKEMON && !DarkPokemon(c))
            {
                g->players[who].field[s].damage += 10 * n;
                TcgDuel_Event(g, TCG_EV_DAMAGE, who, s, 10 * n, 0, 0);
            }
        }
    /* Char counters (Quilava): a coin after every turn, tails 2 damage counters */
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
            if (g->players[who].field[s].stackSize && (g->players[who].field[s].markers & TCG_MARK_CHAR) &&
                !TcgDuel_Coin(g, who))
            {
                g->players[who].field[s].damage += 20;
                TcgDuel_Event(g, TCG_EV_DAMAGE, who, s, 20, 0, TCG_DMG_POISON);
            }
    for (who = 0; who < 2; who++)
    {
        struct TcgPlayer *q = &g->players[who];
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            struct TcgFieldCard *f = &q->field[s];
            unsigned i;
            for (i = f->attachedSize; i > 0; i--)
            {
                unsigned physical = f->attached[i - 1];
                if (q->attachmentExpires[physical] && q->attachmentExpires[physical] <= g->turn)
                {
                    q->discard[q->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, i - 1);
                    q->attachmentExpires[physical] = 0;
                }
            }
        }
    }
    g->current ^= 1;
    g->needsTurnStart = 1;
    g->phase = TCG_TURN;
    TcgDuel_ResolveKnockouts(g);
    if (g->phase == TCG_FINISHED || g->phase == TCG_SETUP)
        return;
    if (g->phase != TCG_PROMOTION)
        StartTurn(g);
}
static int Error(struct TcgDuel *g, const char *s)
{
    snprintf(g->error, sizeof(g->error), "%s", s);
    return TCG_INVALID;
}
static int Apply(struct TcgDuel *g, const struct TcgAction *a);
/* Before an attack resolves, a player may answer (each asked once an attack): Koga's Ninja Trick (the defender's player:
 * switch it out first), Vermilion City Gym (a Lt. Surge Pokemon's player: flip for it), Sabrina's ESP (the attack is
 * tried on a copy to see its coins; keep them or re-flip them all). The attack waits in heldAttack (TcgDuel_Apply
 * resumes it once answered); 1: it waits. */
static int HoldAttack(struct TcgDuel *g, const struct TcgAction *a)
{
    unsigned me = g->current, foe = me ^ 1, s, bench = 0;
    const struct TcgCard *attacker = TcgDuel_Field(g, me, 0);
    for (s = 1; s < TCG_FIELD_SIZE; s++)
        bench += g->players[foe].field[s].stackSize != 0;
    if (!(g->heldAsked & TCG_HELD_NINJA))
    {
        g->heldAsked |= TCG_HELD_NINJA;
        if (bench && TcgDuel_AttachedCard(g, foe, 0, TCG_CARD_GYM2_115) >= 0)
        {
            g->heldAttack = *a;
            TcgDuel_AskChoice(g, foe, TCG_CHOICE_NINJA, TCG_CARD_GYM2_115, 0);
            if (g->phase == TCG_CHOICE)
                return g->heldPending = 1;
        }
    }
    if (!(g->heldAsked & TCG_HELD_VERMILION))
    {
        g->heldAsked |= TCG_HELD_VERMILION;
        if (TcgDuel_Stadium(g, NULL) == TCG_CARD_GYM1_120 && Named(attacker, "Lt. Surge"))
        {
            g->heldAttack = *a;
            TcgDuel_AskChoice(g, me, TCG_CHOICE_VERMILION, TCG_CARD_GYM1_120, 0);
            if (g->phase == TCG_CHOICE)
                return g->heldPending = 1;
        }
    }
    if (!(g->heldAsked & TCG_HELD_ESP))
    {
        g->heldAsked |= TCG_HELD_ESP;
        if (TcgDuel_AttachedCard(g, me, 0, TCG_CARD_GYM1_117) >= 0)
        {
            struct TcgDuel *look = (struct TcgDuel *)malloc(sizeof(*look));
            if (look)
            {
                *look = *g;
                look->heldAsked = TCG_HELD_NINJA | TCG_HELD_VERMILION | TCG_HELD_ESP;
                look->heldResuming = 1;
                look->coinCount = 0, look->coinBits = 0;
                Apply(look, a);
                g->espCoins = (uint8_t)(look->coinCount > 32 ? 32 : look->coinCount);
                g->espBits = look->coinBits;
                free(look);
                if (g->espCoins)
                {
                    g->heldAttack = *a;
                    TcgDuel_AskChoice(g, me, TCG_CHOICE_ESP, TCG_CARD_GYM1_117, 0);
                    if (g->phase == TCG_CHOICE)
                        return g->heldPending = 1;
                }
            }
        }
    }
    return 0;
}
static int Apply(struct TcgDuel *g, const struct TcgAction *a)
{
    struct TcgPlayer *p = &g->players[g->current];
    struct TcgFieldCard *f;
    const struct TcgCard *c;
    unsigned s = a->slot;
    if (g->phase == TCG_FINISHED)
        return Error(g, "This duel has ended.");
    if (a->type == TCG_CONCEDE)
    {
        g->winner = g->current ^ 1;
        g->phase = TCG_FINISHED;
        TcgDuel_Event(g, TCG_EV_WIN, g->winner, 0, 3, 0, 0);
        return TCG_OK;
    }
    if (g->phase == TCG_CHOICE)
    {
        /* A card's effect waits for choicePlayer's answer; then what the action was doing finishes */
        int result;
        if (a->type != TCG_CHOOSE)
            return Error(g, "Make the choice first.");
        if ((result = TcgDuel_Choose(g, a)) != TCG_OK)
            return result;
        if (g->phase == TCG_CHOICE)
            return TCG_OK; /* (the next stage of it) */
        if (g->heldPending && g->phase == TCG_TURN)
        {
            /* (an attack held for an answer: now it resolves) */
            struct TcgAction held = g->heldAttack;
            g->heldPending = 0;
            g->heldResuming = 1;
            result = Apply(g, &held);
            g->heldResuming = 0;
            return result;
        }
        g->heldPending = 0;
        TcgDuel_ResolveKnockouts(g);
        if (g->choiceEndTurn && (g->phase == TCG_TURN || g->phase == TCG_PROMOTION))
        {
            g->choiceEndTurn = 0;
            EndTurn(g);
        }
        g->choiceEndTurn = 0;
        return TCG_OK;
    }
    if (a->type == TCG_CHOOSE)
        return Error(g, "There is nothing to choose.");
    if (g->phase == TCG_PROMOTION)
    {
        unsigned who = g->promotePlayer;
        if (a->type != TCG_PROMOTE || s < 1 || s >= TCG_FIELD_SIZE || !g->players[who].field[s].stackSize)
            return Error(g, "Choose a Benched Pokemon to promote.");
        if (g->switchPending)
        {
            /* Whirlwind and the like: the defender's owner picks who comes in. */
            struct TcgFieldCard temp = g->players[who].field[0];
            g->players[who].field[0] = g->players[who].field[s];
            g->players[who].field[s] = temp;
            ClearCondition(&g->players[who].field[s]);
            TcgDuel_Event(g, TCG_EV_SWITCH, who, s, 0, 0, 0);
            g->switchPending = 0;
            g->phase = TCG_TURN;
            if (g->endAfterSwitch)
            {
                g->endAfterSwitch = 0;
                EndTurn(g);
            }
            return TCG_OK;
        }
        g->players[who].field[0] = g->players[who].field[s];
        memset(&g->players[who].field[s], 0, sizeof(struct TcgFieldCard));
        ClearCondition(&g->players[who].field[0]);
        TcgDuel_Event(g, TCG_EV_PROMOTE, who, s, 0, TcgDuel_Field(g, who, 0)->id, 0);
        if (g->players[who].poisonNext)
        {
            /* Gas Explosion: the new Active Pokemon is Poisoned */
            g->players[who].poisonNext = 0;
            g->players[who].field[0].poison = 10;
            TcgPower_Sanitize(g);
            TcgDuel_Event(g, TCG_EV_STATUS, who, 0, g->players[who].field[0].condition, 0,
                          g->players[who].field[0].poison);
        }
        g->phase = TCG_TURN;
        TcgDuel_ResolveKnockouts(g);
        if (g->phase == TCG_TURN && g->needsTurnStart)
            StartTurn(g);
        return TCG_OK;
    }
    if (a->type == TCG_FINISH_SETUP)
    {
        if (g->phase != TCG_SETUP || !p->field[0].stackSize)
            return Error(g, "Choose an Active Pokemon first.");
        p->setupDone = 1;
        TcgDuel_Event(g, TCG_EV_SETUP_DONE, g->current, 0, 0, 0, 0);
        if (g->players[g->current ^ 1].setupDone)
        {
            g->current = g->first;
            StartTurn(g);
        }
        else
            g->current ^= 1;
        return TCG_OK;
    }
    if (g->phase == TCG_SETUP && a->type != TCG_PLAY_BASIC)
        return Error(g, "Finish setting up your Pokemon first.");
    if (a->type == TCG_PASS)
    {
        EndTurn(g);
        return TCG_OK;
    }
    if (a->type == TCG_ATTACK)
    {
        const struct TcgCard *attacker = TcgDuel_Attacker(g, g->current), *target = TcgDuel_Field(g, g->current ^ 1, 0);
        const struct TcgAttack *attack;
        struct TcgFieldCard *foe = &g->players[g->current ^ 1].field[0], foeBefore;
        unsigned damage, flags = 0, selfCondition, type, k, guard = 0, crystal, selfPoison, selfBurned;
        uint16_t struck[TCG_FIELD_SIZE];
        const char *blocked;
        int result, total;
        /* (Misty's Tentacruel's Flee: it must be able to use it as the attack begins) */
        unsigned fleeTop = foe->stackSize ? foe->stack[foe->stackSize - 1] : 255u, fleeDamage = foe->damage;
        int flee = TcgPower_Working(g, g->current ^ 1, 0) == TCG_POWER_R_FLEE;
        f = &p->field[0];
        g->attackTurn = g->turn;
        if (!attacker || !target || attacker->kind != TCG_POKEMON || a->attack >= TCG_ATTACK_SLOTS ||
            !EnergyPaid(g, g->current, a->attack))
            return Error(g, "That attack cannot be used with the attached Energy.");
        attack = TcgDuel_AttackOf(g, g->current, a->attack); /* (TCG_ATTACK_TM: its Technical Machine's) */
        if (f->disabled == a->attack + 1)
            return Error(g, "Amnesia: that attack can't be used this turn.");
        if ((blocked = TcgDuel_EffectBlock(g, g->current, attack)))
            return Error(g, blocked);
        f->lastAttack = (uint8_t)(a->attack + 1);
        f->lastAttackTurn = g->turn;
        if (TcgDuel_IsMetronome(attack))
        {
            /* Metronome: one of the Defending Pokemon's attacks, without its Energy cost (its type stays its own) */
            if (!(attack = TcgDuel_MetronomeAttack(g, g->current, a->copy)))
                return Error(g, "Choose one of the Defending Pokemon's attacks to copy.");
            if ((blocked = TcgDuel_EffectBlock(g, g->current, attack)))
                return Error(g, blocked);
            TcgDuel_Log(g, "Metronome copies %s.", attack->name);
            if (TcgDuel_MetronomeNeedsEnergy(&attacker->attacks[a->attack < TCG_ATTACKS ? a->attack : 0]) &&
                !EnergyPaidFor(g, g->current, attack))
            {
                /* (Copy: without the Energy for it, the attack does nothing) */
                TcgDuel_Log(g, "Not enough Energy for %s: nothing happens.", attack->name);
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current, 0, 1, 0, 0);
                EndTurn(g);
                return TCG_OK;
            }
        }
        if (!TcgDuel_CanAttackNow(g, g->current, a->attack))
            return Error(g, "This Pokemon cannot attack now.");
        if (!TcgDuel_EffectSupported(attack->effect))
        {
            snprintf(g->error, sizeof(g->error), "Effect not yet implemented: %s", attack->effect);
            return TCG_UNSUPPORTED;
        }
        if (!g->heldResuming)
            g->heldAsked = 0, g->vermilion = 0; /* (a new attack: everything asked again) */
        g->coinCount = 0, g->coinBits = 0;
        if (HoldAttack(g, a))
            return TCG_OK; /* (it waits for an answer: TCG_CHOICE) */
        g->noWeakRes = g->attackFailed = g->coinResult = g->swift = g->bonus = g->scriptCoins = g->scriptHeads = 0, g->scriptBits = 0, g->forceHeads = 0;
        /* A Baby Pokemon as the Defending Pokemon: a coin before anything; tails, the turn ends without an attack */
        if ((target->flags & TCG_CF_BABY) && !TcgDuel_Coin(g, g->current))
        {
            TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 1, 0, 0);
            TcgDuel_Log(g, "Baby Pokemon: %s couldn't bring itself to attack.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        {
            /* (a borrowed attack: value its card's id, flags its index there) */
            unsigned whose = 0;
            if (a->attack >= TCG_ATTACK_BORROW)
                Borrowed(g, g->current, a->attack - TCG_ATTACK_BORROW, &whose);
            TcgDuel_Event(g, TCG_EV_ATTACK, g->current, a->attack, whose & 0xFFFF, attacker->id, whose >> 16);
        }
        if (f->condition == TCG_CONFUSED && !TcgDuel_Coin(g, g->current))
        {
            TcgDuel_SelfDamage(g, TcgPower_Frenzy(g, g->current) ? 50 : 20); /* (Dark Primeape's Frenzy: 30 more) */
            TcgDuel_Log(g, "Confusion: %s does damage to itself.", attacker->name);
            TcgDuel_ResolveKnockouts(g);
            if (g->phase != TCG_FINISHED && g->phase != TCG_SETUP)
                EndTurn(g);
            return TCG_OK;
        }
        if ((f->weaken == TCG_WEAKEN_SMOKESCREEN || f->blindAll) && !TcgDuel_Coin(g, g->current))
        {
            /* Sand-attack / Smokescreen: tails, and the attack does nothing. */
            TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current, 0, 1, 0, 0);
            TcgDuel_Log(g, "%s's attack missed.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        if (TcgDuel_MetronomeHeads(TcgDuel_AttackOf(g, g->current, a->attack)) && !TcgDuel_Coin(g, g->current))
        {
            /* Mini-Metronome: tails, nothing */
            TcgDuel_Log(g, "%s's attack failed.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        if (TcgPower_BeforeAttack(g))
        {
            /* (Rebellion: the attack does nothing, the attacker went into the deck) */
            TcgDuel_ResolveKnockouts(g);
            if (g->phase == TCG_TURN)
                EndTurn(g);
            return TCG_OK;
        }
        /* Before the attack: Fox Fire, Drag Off bring out another Defending Pokemon */
        damage = attack->damage;
        if ((result = TcgDuel_AttackEffect(g, attack, a, &damage, 3)) != TCG_OK)
            return result;
        if (!(target = TcgDuel_Field(g, g->current ^ 1, 0)))
            return Error(g, "There's no Defending Pokemon.");
        /* Crystal Body: nothing but the damage reaches it, as barrier 3 (the effects undone after the attack) */
        if ((crystal = !foe->barrier && TcgPower_Working(g, g->current ^ 1, 0) == TCG_POWER_R_CRYSTAL_BODY) != 0)
            foe->barrier = 3;
        foeBefore = *foe;
        selfCondition = f->condition;
        selfPoison = f->poison, selfBurned = f->burned;
        type = TcgDuel_Type(g, g->current, 0); /* (Venomoth's Shift) */
        for (k = 0; k < TCG_FIELD_SIZE; k++)
            struck[k] = g->players[g->current ^ 1].field[k].damage; /* (Machamp's Strikes Back) */
        /* Nothing it does reaches the Defending Pokemon: Agility and the like, Mew's
         * Neutralizing Shield, heads for Haunter's Transparency (flipped when the attack does
         * something to it; one that may or may not, like Ice Breath, decides as it hits) */
        g->foeImmune = TCG_IMMUNE_UNDECIDED;
        if (foe->barrier == 2)
            g->foeImmune = TCG_IMMUNE_YES;
        else if (attack->damage || TcgDuel_EffectTouchesDefender(attack))
            g->foeImmune = TcgPower_Immune(g, g->current ^ 1, 0) ? TCG_IMMUNE_YES : TCG_IMMUNE_NO;
        damage = attack->damage;
        if ((f->boostTurn == g->turn || f->boostTurn == 0xFFFF) && f->boostAttack == a->attack + 1)   /* (an effect script's boost: "during your next turn"; 0xFFFF until Benched) */
            damage = f->boostValue ? f->boostValue : damage * 2;
        /* Lock-on: its Electric Bolt's coins against the Pokemon it locked on */
        if (f->lockOnUntil >= g->turn && !strcmp(attack->name, "Electric Bolt") && foe->stackSize &&
            foe->stack[foe->stackSize - 1] == f->lockOnFoe)
            g->forceHeads = 1;
        result = TcgDuel_AttackEffect(g, attack, a, &damage, 0);
        if (result != TCG_OK)
            return result;
        /* ApplyDamageModifiers_DamageToTarget: Swords Dance, Weakness,
         * Resistance, PlusPower, Defender, then the substatus that reduce
         * damage; nothing is added to an attack that does no damage. */
        total = (int)damage;
        if (g->swift)
            g->noWeakRes = 1; /* (Swift: Weakness, Resistance, powers and every effect on it don't count) */
        if (total > 0)
        {
            if (f->charged == 2)
                total *= f->charge3 ? 3 : 2;
            total += g->bonus + TcgPower_AttackBonus(g, total, attack); /* (Frenzy, Dark Wave, Prehistoric Dream) */
            /* Metal Energy on a Pokemon that isn't Metal: its damage 10 less each (before Weakness, Resistance) */
            if (type != TCG_METAL)
                total -= 10 * (int)TcgDuel_AttachedRule(g, g->current, 0, TCG_ER_METAL);
            /* (noWeakRes 1: neither, 2: no Resistance, 3: no Weakness; the regional Stadiums: Cinnabar City Gym, no
             * Weakness when Water hits a Blaine Pokemon; Pewter City Gym, a Brock Pokemon ignores Resistance;
             * Magnifier, the same; Resistance Gym, every Resistance 20 less) */
            unsigned stadium = TcgDuel_Stadium(g, NULL);
            int weakOk = !(stadium == TCG_CARD_GYM2_113 && type == TCG_WATER && Named(target, "Blaine"));
            int resistOk = !(stadium == TCG_CARD_GYM1_115 && Named(attacker, "Brock")) &&
                           TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_NEO4_101) < 0;
            if ((!g->noWeakRes || g->noWeakRes == 2) && weakOk && TcgDuel_Weakness(g, g->current ^ 1, 0) == type)
            {
                total *= 2;
                flags |= TCG_DMG_WEAK;
            }
            if ((!g->noWeakRes || g->noWeakRes == 3) && resistOk && TcgDuel_Resistance(g, g->current ^ 1, 0) == type)
            {
                total -= stadium == TCG_CARD_GYM2_109 || TcgPower_InPlay(g, 2, TCG_POWER_R_ENERVATING_POLLEN) ? 10 : 30;
                flags |= TCG_DMG_RESIST;
            }
            if (g->vermilion == 1 && total > 0)
                total += 10; /* (Vermilion City Gym, heads: 10 more after Weakness and Resistance) */
            total += 10 * (int)AttachedTrainerCount(g, g->current, 0, TCG_CARD_PLUSPOWER);
            /* Darkness Energy: 10 more each, if it does damage; Expedition's Metal Energy on a non-Metal attacker 10
             * less each, and any Metal Energy on the Defending Pokemon 10 less each (after Weakness, Resistance) */
            if (total > 0)
                total += 10 * (int)TcgDuel_AttachedRule(g, g->current, 0, TCG_ER_DARKNESS);
            /* Misty (the Trainer): 20 more from a Misty Pokemon; Strength Charm 10 more (then discarded as the turn
             * ends); Sprout Tower: a Colorless Pokemon's attacks 30 less */
            if (total > 0 && (g->players[g->current].turnBuffs & TCG_BUFF_MISTY) && Named(attacker, "Misty"))
                total += 20;
            if (total > 0)
            {
                int charm = TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_ECARD1_150);
                if (charm >= 0)
                {
                    total += 10;
                    g->players[g->current].attachmentExpires[charm] = g->turn;
                }
            }
            if (stadium == TCG_CARD_NEO1_97 && type == TCG_COLORLESS)
                total -= 30;
            if (type != TCG_METAL)
                total -= 10 * (int)TcgDuel_AttachedRule(g, g->current, 0, TCG_ER_METAL_LATE);
            if (!g->swift)
                total -= 10 * (int)(TcgDuel_AttachedRule(g, g->current ^ 1, 0, TCG_ER_METAL) +
                                    TcgDuel_AttachedRule(g, g->current ^ 1, 0, TCG_ER_METAL_LATE));
            if (foe->vulnUntil >= g->turn)
                total += foe->vulnBonus; /* (the regional sets: "that attack does 20 more damage") */
            if (total > 0 && TcgPower_Working(g, g->current, 0) == TCG_POWER_R_FROG_SONG)
            {
                /* Frog Song: more than 3 Poliwag, Poliwhirl, Poliwrath and Politoed in play (both sides), 40 more */
                static const char *const poli[4] = {"Poliwag", "Poliwhirl", "Poliwrath", "Politoed"};
                unsigned who, s2, n = 0, i;
                for (who = 0; who < 2; who++)
                    for (s2 = 0; s2 < TCG_FIELD_SIZE; s2++)
                        for (i = 0; i < 4; i++)
                            n += TcgDuel_Field(g, who, s2) && !strcmp(TcgDuel_Field(g, who, s2)->name, poli[i]);
                if (n > 3)
                {
                    total += 40;
                    TcgDuel_Event(g, TCG_EV_POWER, g->current, 0, TCG_POWER_R_FROG_SONG, attacker->id, 0);
                    TcgDuel_Log(g, "Frog Song: 40 more.");
                }
            }
            if (!g->swift)
                total -= 20 * (int)AttachedTrainerCount(g, g->current ^ 1, 0, TCG_CARD_DEFENDER);
            guard = foe->guard ? foe->guard : foe->guardKeep;
            if (g->swift)
                ;
            else if ((foe->barrier && foe->barrier != 3) || g->foeImmune == TCG_IMMUNE_YES)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (guard == TCG_GUARD_REDUCE_10)
                total -= 10;
            else if (guard == TCG_GUARD_REDUCE_20)
                total -= 20;
            else if (guard == TCG_GUARD_UNDER_40 && total < 40)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (guard == TCG_GUARD_HALVE)
                total = total / 2 / 10 * 10;
            /* GB2's: Harden, Double-Edged Pincers, Blink, Expand */
            else if (guard == TCG_GUARD_UNDER_30 && total <= 20)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (guard == TCG_GUARD_DOUBLE)
                total *= 2;
            else if (guard == TCG_GUARD_BLINK && TcgDuel_Coin(g, g->current ^ 1))
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (guard == TCG_GUARD_REDUCE_10_LATE)
                total -= 10;
            else if (guard == TCG_GUARD_30_OR_MORE && total >= 30)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (guard == TCG_GUARD_BLINK_HIT && total > 0 && TcgDuel_Coin(g, g->current ^ 1))
            {
                /* Reflect Shield: heads, no damage, and 20 to the attacking Pokemon */
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
                f->damage += 20;
                TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, 20, 0, 0);
                TcgDuel_Log(g, "Reflect Shield: no damage, and 20 back.");
            }
            else if (guard == TCG_GUARD_TAILS && total > 0 && !TcgDuel_Coin(g, g->current))
            {
                /* Shadow Images: the attacker's coin, tails no damage */
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            /* (Growl and the like: only against the Pokemon that used it) */
            if (f->weaken == TCG_WEAKEN_10 && TcgDuel_BoundEffect(g, g->current, 0))
                total -= 10;
            else if (f->weaken == TCG_WEAKEN_20 && TcgDuel_BoundEffect(g, g->current, 0))
                total -= 20;
            /* Mr. Mime's Invisible Wall, Kabuto Armor */
            if (!g->swift)
                total = TcgPower_AttackDamage(g, g->current ^ 1, 0, total, 1);
            if (total < 0)
                total = 0;
        }
        damage = (unsigned)total;
        TcgDuel_Damage(g, g->current ^ 1, 0, damage);
        if (damage)
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current ^ 1, 0, damage, 0, flags);
        if (damage && foe->destinyBond && foe->damage >= TcgDuel_MaxHp(g, g->current ^ 1, 0) &&
            f->damage < TcgDuel_MaxHp(g, g->current, 0))
        {
            /* Destiny Bond: the Pokemon that Knocks it Out is Knocked Out too */
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, TcgDuel_MaxHp(g, g->current, 0) - f->damage, 0, TCG_DMG_SELF);
            f->damage = (uint16_t)TcgDuel_MaxHp(g, g->current, 0);
            TcgDuel_Log(g, "Destiny Bond: %s is Knocked Out too.", attacker->name);
        }
        if (damage)
        {
            foe->guardKeep = 0; /* (Shadow Images lasts until it takes damage) */
            TcgPower_Damaged(g, g->current ^ 1, 0, damage); /* (the regional powers: Fortitude, Restless Sleep...) */
        }
        /* Koga (the Trainer): a Koga Pokemon's damage Poisons */
        if (damage && (g->players[g->current].turnBuffs & TCG_BUFF_KOGA) && Named(attacker, "Koga") && !foe->poison)
        {
            foe->poison = 10;
            TcgDuel_Event(g, TCG_EV_STATUS, g->current ^ 1, 0, foe->condition, 0, foe->poison);
        }
        /* Focus Band: a Knock Out by this attack, heads, leaves it at 10 HP (then discarded) */
        if (damage && target->kind == TCG_POKEMON && foe->damage >= TcgDuel_MaxHp(g, g->current ^ 1, 0))
        {
            int band = TcgDuel_AttachedCard(g, g->current ^ 1, 0, TCG_CARD_NEO1_86);
            if (band >= 0 && TcgDuel_Coin(g, g->current ^ 1))
            {
                foe->damage = (uint16_t)(TcgDuel_MaxHp(g, g->current ^ 1, 0) - 10u);
                TcgDuel_Log(g, "Focus Band: %s hangs on.", target->name);
                TcgDuel_DiscardAttached(g, g->current ^ 1, 0, (unsigned)band);
            }
        }
        /* Counterattack Claws: damaged, a coin, heads 2 counters on the attacker (then discarded) */
        if (damage)
        {
            int claws = TcgDuel_AttachedCard(g, g->current ^ 1, 0, TCG_CARD_NEO4_97);
            if (claws >= 0)
            {
                if (TcgDuel_Coin(g, g->current ^ 1))
                {
                    f->damage += 20;
                    TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, 20, 0, TCG_DMG_SELF);
                }
                TcgDuel_DiscardAttached(g, g->current ^ 1, 0, (unsigned)claws);
            }
        }
        /* Crystal Shard, Memory Berry: discarded at the end of a turn its Pokemon attacked */
        {
            int shard = TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_ECARD3_122);
            if (shard >= 0)
                g->players[g->current].attachmentExpires[shard] = g->turn;
            shard = TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_ECARD2_128);
            if (shard >= 0)
                g->players[g->current].attachmentExpires[shard] = g->turn;
        }
        if (damage && foe->damage >= TcgDuel_MaxHp(g, g->current ^ 1, 0))
            TcgPower_KnockedOut(g, g->current ^ 1, 0); /* (Dark Gyarados's Final Beam) */
        if (damage && foe->mirrorShell && f->stackSize)
        {
            /* Dark Wartortle's Mirror Shell: as much back at the attacker */
            f->damage += damage;
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, damage, 0, 0);
            TcgDuel_Log(g, "Mirror Shell: %u damage back.", damage);
        }
        TcgDuel_Log(g, "%s uses %s for %u damage.", attacker->name, attack->name, damage);
        result = TcgDuel_AttackEffect(g, attack, a, &damage, 1);
        if (result != TCG_OK)
            return result;
        if (foeBefore.barrier >= 2 || g->foeImmune == TCG_IMMUNE_YES)
        {
            /* Agility, Fly, Barrier (and the powers above): no effect of the attack reaches it (barrier 3: but its
             * damage does). */
            foe->condition = foeBefore.condition;
            foe->poison = foeBefore.poison;
            foe->burned = foeBefore.burned;
            foe->stuck = foeBefore.stuck, foe->stuckBy = foeBefore.stuckBy;
            foe->blindAll = foeBefore.blindAll, foe->noAttackAll = foeBefore.noAttackAll;
            foe->noAttack = foeBefore.noAttack;
            foe->noRetreat = foeBefore.noRetreat;
            foe->weaken = foeBefore.weaken;
            foe->effectBy = foeBefore.effectBy;
        }
        else if (foeBefore.condImmune >= g->turn && foe->stackSize && foe->stack[0] == foeBefore.stack[0])
        {
            /* (an effect script's noconds: it can't become affected by a Special Condition this turn) */
            foe->condition = foeBefore.condition;
            foe->poison = foeBefore.poison;
            foe->burned = foeBefore.burned;
        }
        if (crystal && foe->barrier == 3)
            foe->barrier = 0;
        if (foe->stackSize && foe->stack[0] == foeBefore.stack[0] && f->stackSize &&
            TcgPower_Working(g, g->current ^ 1, 0) == TCG_POWER_R_MIRROR_COAT && foe->damage < TcgDuel_MaxHp(g, g->current ^ 1, 0))
        {
            /* Mirror Coat: Poisoned or Burned by this attack, and the attacker is too */
            if (foe->poison && !foeBefore.poison)
                f->poison = foe->poison;
            if (foe->burned && !foeBefore.burned)
                f->burned = foe->burned;
        }
        TcgPower_Sanitize(g); /* (Snorlax, Clefairy Doll, Mysterious Fossil) */
        if (foe->stackSize && foe->stack[0] == foeBefore.stack[0])
        {
            /* What this attack did to the Defending Pokemon, for its Mirror Move next turn */
            ClearMirror(foe);
            foe->mirrorDamage = (uint16_t)damage;
            foe->mirrorCondition = foe->condition != foeBefore.condition ? foe->condition : 0;
            foe->mirrorPoison = foe->poison != foeBefore.poison ? foe->poison : 0;
            foe->mirrorWeaken = foe->weaken != foeBefore.weaken ? foe->weaken : 0;
            foe->mirrorNoAttack = foe->noAttack != foeBefore.noAttack ? foe->noAttack : 0;
            foe->mirrorNoRetreat = foe->noRetreat != foeBefore.noRetreat ? foe->noRetreat : 0;
            foe->mirrorWeak = foe->weakAs != foeBefore.weakAs ? foe->weakAs : 0;
            if (foe->disabled && foe->disabled != foeBefore.disabled)
                foe->mirrorEffect = TCG_MIRROR_AMNESIA, foe->mirrorArg = foe->disabled - 1;
            else if (foe->attachedSize < foeBefore.attachedSize)
                foe->mirrorEffect = TCG_MIRROR_DISCARD_ENERGY;
        }
        TcgPower_StrikesBack(g, struck, foeBefore.condition);
        /* Effects change conditions as they resolve; show them after the hit. */
        if (foe->stackSize && (foe->condition != foeBefore.condition || foe->poison != foeBefore.poison))
            TcgDuel_Event(g, TCG_EV_STATUS, g->current ^ 1, 0, foe->condition, 0, foe->poison);
        if (f->stackSize && (f->condition != selfCondition || f->poison != selfPoison || f->burned != selfBurned))
            TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, f->condition, 0, f->poison);
        g->animationCard = attacker->id;
        g->animationKind = a->attack + 1;
        if (g->phase == TCG_CHOICE)
        {
            /* (Light Venomoth and the like: the turn ends once the choices are made) */
            g->noWeakRes = g->attackFailed = g->coinResult = g->foeImmune = g->swift = g->bonus = 0;
            g->scriptCoins = g->scriptHeads = 0, g->scriptBits = 0, g->forceHeads = 0;
            g->choiceEndTurn = 1;
            return TCG_OK;
        }
        g->noWeakRes = g->attackFailed = g->coinResult = g->foeImmune = g->swift = g->bonus = g->scriptCoins = g->scriptHeads = 0, g->scriptBits = 0, g->forceHeads = 0;
        if (g->switchPending)
        {
            /* Only if the Defending Pokemon is still there to be switched. It comes before any
             * Knock Out: Ram switches "even if Rhydon is Knocked Out" (the end of the turn,
             * after the switch, takes care of that). */
            unsigned who = g->current ^ 1, bench = 0, k, hp = TcgDuel_MaxHp(g, g->current ^ 1, 0);
            for (k = 1; k < TCG_FIELD_SIZE; k++)
                bench += g->players[who].field[k].stackSize != 0;
            if (g->players[who].field[0].stackSize && g->players[who].field[0].damage < hp && bench &&
                !TcgDuel_Anchored(g, who))
            {
                g->phase = TCG_PROMOTION;
                g->promotePlayer = who;
                g->endAfterSwitch = 1;
                return TCG_OK;
            }
            g->switchPending = 0;
        }
        if (g->vermilion == 2)
        {
            /* (Vermilion City Gym, tails: 10 damage to the attacker too; no Weakness or Resistance on its own) */
            f->damage += 10;
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, 10, 0, TCG_DMG_SELF);
            g->vermilion = 0;
        }
        if (TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_GYM1_99) >= 0 && foe->stackSize &&
            foe->stack[foe->stackSize - 1] == fleeTop && foe->damage > fleeDamage)
        {
            /* Charity: its player may reduce the damage just done to the Defending Pokemon (tcg_choice.inc) */
            g->choiceData = (uint8_t)((foe->damage - fleeDamage) / 10);
            TcgDuel_AskChoice(g, g->current, TCG_CHOICE_CHARITY, TCG_CARD_GYM1_99, 0);
            if (g->phase == TCG_CHOICE)
            {
                g->choiceEndTurn = 1;
                return TCG_OK;
            }
        }
        if (flee && foe->stackSize && foe->stack[foe->stackSize - 1] == fleeTop && foe->damage > fleeDamage &&
            foe->damage < TcgDuel_MaxHp(g, g->current ^ 1, 0))
        {
            /* Flee: the attack damaged it; its player may switch it with a Benched one (tcg_choice.inc) */
            TcgDuel_AskChoice(g, g->current ^ 1, TCG_CHOICE_FLEE, TcgDuel_Field(g, g->current ^ 1, 0)->id, 0);
            if (g->phase == TCG_CHOICE)
            {
                g->choiceEndTurn = 1;
                return TCG_OK;
            }
        }
        TcgDuel_ResolveKnockouts(g);
        if (g->phase == TCG_FINISHED || g->phase == TCG_SETUP)
            return TCG_OK;
        EndTurn(g);
        return TCG_OK;
    }
    if (a->type == TCG_RETREAT)
    {
        unsigned i, n = 0;
        struct TcgFieldCard temp;
        c = TcgDuel_Field(g, g->current, 0);
        f = &p->field[0];
        if (!c || s < 1 || s >= TCG_FIELD_SIZE || !p->field[s].stackSize)
            return Error(g, "Choose a Benched Pokemon.");
        if (p->retreated)
            return Error(g, "You can only retreat once each turn.");
        if (c->kind == TCG_TRAINER || f->condition == TCG_ASLEEP || f->condition == TCG_PARALYZED || f->noRetreat ||
            TcgDuel_Stuck(g, g->current) ||
            p->confusionRetreatFailed || (f->lockTurn == g->turn && f->lockAttack == TCG_LOCK_ALL) ||
            TcgDuel_AttachedRule(g, g->current, 0, TCG_ER_BOOST))
            return Error(g, "This Pokemon cannot retreat.");
        {
            const char *why = TcgPower_RetreatBlock(g, g->current);
            if (why)
                return Error(g, why);
        }
        /* Mirage Stadium: a coin first; tails, it can't retreat this turn (nothing discarded) */
        if (TcgDuel_Stadium(g, NULL) == TCG_CARD_ECARD3_132 && !TcgDuel_Coin(g, g->current))
        {
            f->noRetreat = 1;
            p->retreated = 1;
            TcgDuel_Event(g, TCG_EV_RETREAT_FAIL, g->current, 0, 0, 0, 0);
            TcgDuel_Log(g, "Mirage Stadium: it can't retreat this turn.");
            return TCG_OK;
        }
        /* Balloon Berry: discarded instead of the Energy (TcgDuel_RetreatCost is then 0) */
        int berry = TcgDuel_AttachedCard(g, g->current, 0, TCG_CARD_NEO3_60);
        if (berry >= 0)
            TcgDuel_DiscardAttached(g, g->current, 0, (unsigned)berry);
        for (i = 0; i < a->selectionCount; i++)
        {
            unsigned j;
            for (j = 0; j < f->attachedSize && f->attached[j] != a->selections[i]; j++)
                ;
            if (j == f->attachedSize)
                return Error(g, "Select attached Energy to pay the retreat cost.");
            c = Tcg_Card(p->cards[f->attached[j]]);
            if (c->kind != TCG_ENERGY)
                return TCG_INVALID;
            n += Tcg_EnergyProvides(c->id);
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, j);
        }
        c = TcgDuel_Field(g, g->current, 0);
        /* (each Dodrio on the Bench: Retreat Aid, one Colorless less; Dark Muk's Sticky Goo, 2 more) */
        if (berry < 0 && n < TcgDuel_RetreatCost(g, g->current))
            return Error(g, "Not enough Energy selected for retreat.");
        p->retreated = 1;
        if (f->condition == TCG_CONFUSED && !TcgDuel_Coin(g, g->current))
        {
            p->confusionRetreatFailed = 1;
            TcgDuel_Event(g, TCG_EV_RETREAT_FAIL, g->current, 0, 0, 0, 0);
            TcgDuel_Log(g, "Confusion: retreat failed after discarding Energy.");
            return TCG_OK;
        }
        temp = *f;
        *f = p->field[s];
        p->field[s] = temp;
        ClearCondition(&p->field[s]);
        ClearMirror(&p->field[s]);
        p->confusionRetreatFailed = 0;
        TcgDuel_Event(g, TCG_EV_RETREAT, g->current, s, 0, 0, 0);
        TcgPower_AfterRetreat(g, g->current, s); /* (Sinkhole, Vine Pull) */
        TcgDuel_ResolveKnockouts(g);
        return TCG_OK;
    }
    if (a->type == TCG_USE_POWER)
    {
        int result = TcgDuel_Power(g, a);
        if (result == TCG_OK && g->turnOver)
        {
            /* (Playful Punch: the turn ends) */
            g->turnOver = 0;
            if (g->phase == TCG_TURN)
                EndTurn(g);
        }
        return result;
    }
    if (a->type == TCG_USE_STADIUM)
    {
        int result = TcgDuel_UseStadium(g, a);
        if (result == TCG_OK)
            TcgPower_Sanitize(g);
        return result;
    }
    if (a->type == TCG_DISCARD_DOLL)
    {
        /* Clefairy Doll / Mysterious Fossil: "At any time during your turn before your attack,
         * you may discard it from play" (it isn't a Knock Out: no Prize) */
        unsigned k, others = 0;
        c = s < TCG_FIELD_SIZE ? TcgDuel_Field(g, g->current, s) : NULL;
        if (g->phase != TCG_TURN || !c || c->kind != TCG_TRAINER)
            return Error(g, "Choose a Clefairy Doll or Mysterious Fossil in play.");
        for (k = 0; k < TCG_FIELD_SIZE; k++)
            others += k != s && p->field[k].stackSize;
        if (!others)
            return Error(g, "It's your only Pokemon in play.");
        TcgDuel_Event(g, TCG_EV_TRAINER, g->current, s, 0, c->id, 0);
        TcgDuel_DiscardField(g, g->current, s);
        TcgDuel_ResolveKnockouts(g); /* (from the Active spot: promote a Benched Pokemon) */
        return TCG_OK;
    }
    if (a->hand >= p->handSize)
        return Error(g, "Choose a card in your hand.");
    c = Tcg_Card(p->cards[p->hand[a->hand]]);
    if (a->type == TCG_PLAY_TRAINER)
    {
        if (p->noTrainers)
            return Error(g, "Headache: you can't play Trainer cards this turn.");
        if (p->noSupporters && (c->flags & TCG_CF_SUPPORTER))
            return Error(g, "You can't play Supporter cards this turn.");
        if (TcgPower_NoTrainers(g))
            return Error(g, "Dark Vileplume's Hay Fever: no Trainer cards can be played.");
        if (!(c->flags & TCG_CF_STADIUM) && (TcgPower_Working(g, 0, 0) == TCG_POWER_R_LEGENDARY_BODY ||
                                              TcgPower_Working(g, 1, 0) == TCG_POWER_R_LEGENDARY_BODY))
        {
            /* Entei's Legendary Body: no Trainer but a Stadium affects the Active Entei or its Energy (a Trainer that
             * would is refused) */
            struct TcgDuel *test = malloc(sizeof(*test));
            unsigned w;
            int touched = 0;
            if (test)
            {
                *test = *g;
                if (TcgDuel_Trainer(test, a) == TCG_OK)
                    for (w = 0; w < 2; w++)
                        touched |= TcgPower_Working(g, w, 0) == TCG_POWER_R_LEGENDARY_BODY &&
                                   memcmp(&test->players[w].field[0], &g->players[w].field[0], sizeof(g->players[w].field[0]));
                free(test);
            }
            if (touched)
                return Error(g, "Legendary Body: Trainer cards don't affect that Entei.");
        }
        if (TcgDuel_Stadium(g, NULL) == TCG_CARD_GYM1_103 &&
            (Tcg_Canon(c->id) == TCG_CARD_ENERGY_REMOVAL || Tcg_Canon(c->id) == TCG_CARD_SUPER_ENERGY_REMOVAL) &&
            p->handSize < 3)
            return Error(g, "No Removal Gym: discard 2 other cards from your hand to play it.");
        {
            unsigned s2;
            for (s2 = 0; s2 < TCG_FIELD_SIZE; s2++)
                if (TcgPower_Working(g, g->current ^ 1, s2) == TCG_POWER_R_MIND_GAMES)
                    break;
            if (s2 < TCG_FIELD_SIZE)
            {
                /* Slowking's Mind Games: a Trainer that would play: a coin; heads, it does nothing and goes on top
                 * of the deck */
                struct TcgDuel *test = malloc(sizeof(*test));
                int ok = 0;
                if (test)
                {
                    *test = *g;
                    ok = TcgDuel_Trainer(test, a) == TCG_OK;
                    free(test);
                }
                if (ok)
                {
                    TcgDuel_Event(g, TCG_EV_POWER, g->current ^ 1, s2, TCG_POWER_R_MIND_GAMES,
                                  TcgDuel_Field(g, g->current ^ 1, s2)->id, 0);
                    if (TcgDuel_Coin(g, g->current ^ 1))
                    {
                        if (c->flags & TCG_CF_SUPPORTER)
                            p->supporterPlayed = 1;
                        TcgDuel_Event(g, TCG_EV_TRAINER, g->current, 0, 0, c->id, 0);
                        p->deck[p->deckSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
                        TcgDuel_Log(g, "Mind Games: %s does nothing and goes on top of the deck.", c->name);
                        TcgDuel_Event(g, TCG_EV_DECK_CHANGED, g->current, 0, 0, 0, 0);
                        return TCG_OK;
                    }
                }
            }
        }
        {
            int noRemoval = TcgDuel_Stadium(g, NULL) == TCG_CARD_GYM1_103 &&
                            (Tcg_Canon(c->id) == TCG_CARD_ENERGY_REMOVAL || Tcg_Canon(c->id) == TCG_CARD_SUPER_ENERGY_REMOVAL);
            int result = TcgDuel_Trainer(g, a);
            if (result == TCG_OK && noRemoval && g->phase == TCG_TURN)
                TcgDuel_AskChoice(g, g->current, TCG_CHOICE_DISCARD2, TCG_CARD_GYM1_103, 1);
            if (result == TCG_OK && g->turnOver)
            {
                /* Computer Error: the turn is over */
                g->turnOver = 0;
                if (g->phase == TCG_TURN)
                    EndTurn(g);
            }
            return result;
        }
    }
    if (s >= TCG_FIELD_SIZE)
        return TCG_INVALID;
    f = &p->field[s];
    switch (a->type)
    {
    case TCG_PLAY_BASIC:
        if (c->kind != TCG_POKEMON || c->stage || f->stackSize || (g->phase != TCG_SETUP && !s))
            return Error(g, "Choose a Basic Pokemon and an empty Bench slot.");
        f->stack[f->stackSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        f->enteredTurn = g->turn;
        TcgDuel_Event(g, TCG_EV_PLAY, g->current, s, 0, c->id, 0);
        TcgDuel_Log(g, "Player %u plays %s.", g->current + 1, c->name);
        /* (Firegiver, Quickfreeze, Peal of Thunder: not during set-up) */
        if (g->phase != TCG_SETUP)
        {
            TcgPower_BenchedFromHand(g, g->current, s); /* (Hot Plate) */
            g->fromHand = 1;
            TcgPower_OnPlayFrom(g, g->current, s, a);
            g->fromHand = 0;
            TcgPower_Sanitize(g);
            TcgDuel_ResolveKnockouts(g);
        }
        return TCG_OK;
    case TCG_EVOLVE:
        if (c->kind != TCG_POKEMON || !f->stackSize || f->stackSize == 4 ||
            !TcgDuel_EvolvesFrom(c, TcgDuel_Field(g, g->current, s)))
            return Error(g, "This Pokemon does not evolve from that card.");
        if (!TcgDuel_CanEvolveNow(g, g->current, s))
            return Error(g, "A Pokemon cannot evolve on its first turn in play.");
        if (TcgPower_InPlay(g, 2, TCG_POWER_PREHISTORIC))
            return Error(g, "Aerodactyl's Prehistoric Power: no Evolution cards can be played.");
        f->stack[f->stackSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        f->enteredTurn = g->turn;
        ClearCondition(f);
        TcgDuel_Event(g, TCG_EV_EVOLVE, g->current, s, 0, c->id, 0);
        TcgDuel_Log(g, "Evolved into %s.", c->name);
        /* Viridian City Gym: a Giovanni Pokemon evolving loses 2 counters */
        if (TcgDuel_Stadium(g, NULL) == TCG_CARD_GYM2_123 && Named(c, "Giovanni") && f->damage)
        {
            unsigned heal = f->damage < 20 ? f->damage : 20;
            f->damage -= heal;
            TcgDuel_Event(g, TCG_EV_HEAL, g->current, s, heal, 0, 0);
        }
        g->fromHand = 1;
        TcgPower_OnPlayFrom(g, g->current, s, a); /* (Healing Wind) */
        g->fromHand = 0;
        TcgPower_Sanitize(g);
        TcgDuel_ResolveKnockouts(g);
        return TCG_OK;
    case TCG_ATTACH:
        if (c->kind != TCG_ENERGY || !f->stackSize || (p->energyPlayed && !TcgDuel_HarvestOk(g, g->current, s)))
            return Error(g, "Only one Energy card may be attached from your hand each turn.");
        if (Tcg_EnergyRule(c->id) == TCG_ER_BOOST && f->stackSize < 2)
            return Error(g, "Boost Energy can be attached only to an Evolved Pokemon.");
        if (Tcg_EnergyRule(c->id) == TCG_ER_BOUNCE)
        {
            unsigned k;
            for (k = 0; k < f->attachedSize && p->cards[f->attached[k]] > 6; k++)
                ;
            if (k == f->attachedSize)
                return Error(g, "Bounce Energy goes only on a Pokemon with a basic Energy card.");
        }
        if (Tcg_EnergyRule(c->id) == TCG_ER_MIRACLE &&
            !(TcgDuel_Field(g, g->current, s)->flags & (TCG_CF_SHINING | TCG_CF_LIGHT)))
            return Error(g, "Miracle Energy goes only on a Shining or Light Pokemon.");
        if (TcgPower_AttachBlock(g, g->current, s, c->id))
            return Error(g, "That Energy can't be attached to this Pokemon (its power).");
        if (!s && p->noAttachActive >= g->turn)
            return Error(g, "No Energy can be attached to your Active Pokemon this turn.");
        /* (Boost Energy: discarded at the end of the turn) */
        if (Tcg_EnergyRule(c->id) == TCG_ER_BOOST || Tcg_EnergyRule(c->id) == TCG_ER_MIRACLE)
            p->attachmentExpires[p->hand[a->hand]] = g->turn;   /* (Miracle Energy too) */
        f->attached[f->attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        /* (2: the first went to the Active Pokemon, Harvest Bounty may add one; 3: it did) */
        p->energyPlayed = p->energyPlayed ? 3 : s ? 1 : 2;
        TcgDuel_Event(g, TCG_EV_ATTACH, g->current, s, 0, c->id, 0);
        if (f->attachPoisonUntil >= g->turn)
        {
            /* (Triggered Poison) */
            f->poison = 10;
            f->attachPoisonUntil = 0;
            TcgDuel_Event(g, TCG_EV_STATUS, g->current, s, f->condition, 0, f->poison);
        }
        TcgPower_AfterAttach(g, g->current, s, f->attached[f->attachedSize - 1]);
        /* Pokemon Park: once a turn, Energy onto a Benched Pokemon takes a counter off it */
        if (TcgDuel_Stadium(g, NULL) == TCG_CARD_ECARD2_131 && s && f->damage && !(p->turnBuffs & TCG_BUFF_PARK))
        {
            p->turnBuffs |= TCG_BUFF_PARK;
            f->damage -= 10;
            TcgDuel_Event(g, TCG_EV_HEAL, g->current, s, 10, 0, 0);
        }
        /* GB2's special Energy, attached from the hand */
        if (c->id == TCG_CARD_POTION_ENERGY && f->damage)
        {
            f->damage -= 10;
            TcgDuel_Event(g, TCG_EV_HEAL, g->current, s, 10, 0, 0);
        }
        else if (c->id == TCG_CARD_FULLHEAL_ENERGY && (f->condition || f->poison))
        {
            f->condition = TCG_HEALTHY;
            f->poison = 0;
            TcgDuel_Event(g, TCG_EV_STATUS, g->current, s, TCG_HEALTHY, 0, 0);
        }
        else if (Tcg_EnergyRule(c->id) == TCG_ER_RAINBOW)
        {
            f->damage += 10;
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, s, 10, 0, 0);
            TcgDuel_ResolveKnockouts(g);
        }
        else if (Tcg_EnergyRule(c->id) == TCG_ER_BOUNCE)
        {
            /* a basic Energy on it back to the hand: the one picked (a->selections[0]), else the first */
            unsigned k, at = f->attachedSize;
            for (k = 0; k < f->attachedSize; k++)
                if (p->cards[f->attached[k]] <= 6 && (at == f->attachedSize || (a->selectionCount && f->attached[k] == a->selections[0])))
                    at = k;
            if (at < f->attachedSize)
                p->hand[p->handSize++] = TcgPile_Remove(f->attached, &f->attachedSize, at);
        }
        else if (Tcg_EnergyRule(c->id) == TCG_ER_WARP && !s && !TcgDuel_Anchored(g, g->current))
        {
            /* its Active switches with the Benched Pokemon chosen (a->target), else the first */
            unsigned t = a->target, k;
            if (t < 1 || t >= TCG_FIELD_SIZE || !p->field[t].stackSize)
                for (t = 0, k = 1; k < TCG_FIELD_SIZE && !t; k++)
                    if (p->field[k].stackSize)
                        t = k;
            if (t)
            {
                struct TcgFieldCard temp = p->field[0];
                p->field[0] = p->field[t];
                p->field[t] = temp;
                ClearCondition(&p->field[t]);
                ClearMirror(&p->field[t]);
                TcgDuel_Event(g, TCG_EV_SWITCH, g->current, t, 0, 0, 0);
            }
        }
        else if (Tcg_EnergyRule(c->id) == TCG_ER_CYCLONE && !s && !TcgDuel_Anchored(g, g->current ^ 1))
        {
            /* their Active switches with one of their Benched Pokemon: they choose */
            unsigned k;
            for (k = 1; k < TCG_FIELD_SIZE && !g->players[g->current ^ 1].field[k].stackSize; k++)
                ;
            if (k < TCG_FIELD_SIZE)
            {
                g->phase = TCG_PROMOTION;
                g->promotePlayer = (uint8_t)(g->current ^ 1);
                g->switchPending = 1;
            }
        }
        else if (Tcg_EnergyRule(c->id) == TCG_ER_RETRO && f->stackSize >= 2 && a->target == 1)
        {
            /* (they chose YES) up to 2 counters off it, its top card discarded: devolved */
            unsigned heal = f->damage < 20 ? f->damage : 20;
            f->damage -= heal;
            if (heal)
                TcgDuel_Event(g, TCG_EV_HEAL, g->current, s, heal, 0, 0);
            p->discard[p->discardSize++] = f->stack[--f->stackSize];
            ClearCondition(f);
            TcgDuel_Event(g, TCG_EV_DEVOLVE, g->current, s, 0, p->cards[f->stack[f->stackSize - 1]], 0);
        }
        return TCG_OK;
    default:
        return Error(g, "That action is not available.");
    }
}
int TcgDuel_Conserved(const struct TcgDuel *g)
{
    unsigned who, i, j;
    for (who = 0; who < 2; who++)
    {
        const struct TcgPlayer *p = &g->players[who];
        unsigned seen[60] = {0}, n = 0;
#define CHECK_CARD(x)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        unsigned idx = (x);                                                                                            \
        if (idx >= 60 || seen[idx]++)                                                                                  \
            return 0;                                                                                                  \
        n++;                                                                                                           \
    } while (0)
        if (p->deckSize > 60 || p->handSize > 60 || p->discardSize > 60 || p->prizeSize > 6)
            return 0;
        for (i = 0; i < p->deckSize; i++)
            CHECK_CARD(p->deck[i]);
        for (i = 0; i < p->handSize; i++)
            CHECK_CARD(p->hand[i]);
        for (i = 0; i < p->discardSize; i++)
            CHECK_CARD(p->discard[i]);
        for (i = 0; i < p->prizeSize; i++)
            CHECK_CARD(p->prizes[i]);
        for (i = 0; i < p->asideSize && i < 60; i++)
            CHECK_CARD(p->aside[i]);
        if (p->stadium)
            CHECK_CARD(p->stadium - 1u);
        if (p->supporterCard)
            CHECK_CARD(p->supporterCard - 1u);
        for (i = 0; i < TCG_FIELD_SIZE; i++)
        {
            const struct TcgFieldCard *f = &p->field[i];
            if (f->stackSize > 4 || f->attachedSize > 60)
                return 0;
            for (j = 0; j < f->stackSize; j++)
                CHECK_CARD(f->stack[j]);
            for (j = 0; j < f->attachedSize; j++)
                CHECK_CARD(f->attached[j]);
        }
        if (n != 60)
            return 0;
#undef CHECK_CARD
    }
    return 1;
}
/* ------------------------------------------------------------------ */
/* The rules that undo part of what an effect did, after the action      */
/* ------------------------------------------------------------------ */
static int PileAt(const uint8_t *pile, unsigned n, unsigned card)
{
    unsigned i;
    for (i = 0; i < n; i++)
        if (pile[i] == card)
            return (int)i;
    return -1;
}
/* card out of wherever it is now (the hand, the deck, the Prizes, attached, a lone Pokemon or the top of one) */
static int TakeBack(struct TcgPlayer *p, unsigned card)
{
    int at;
    unsigned s;
    if ((at = PileAt(p->hand, p->handSize, card)) >= 0)
        return TcgPile_Remove(p->hand, &p->handSize, (unsigned)at), 1;
    if ((at = PileAt(p->deck, p->deckSize, card)) >= 0)
        return TcgPile_Remove(p->deck, &p->deckSize, (unsigned)at), 1;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        struct TcgFieldCard *f = &p->field[s];
        if ((at = PileAt(f->attached, f->attachedSize, card)) >= 0)
            return TcgPile_Remove(f->attached, &f->attachedSize, (unsigned)at), 1;
        if (f->stackSize && f->stack[f->stackSize - 1] == card && (f->stackSize > 1 || !f->attachedSize))
        {
            if (--f->stackSize == 0)
                memset(f, 0, sizeof(*f));
            return 1;
        }
    }
    return 0;
}
static void Undo(const struct TcgDuel *before, struct TcgDuel *g)
{
    unsigned actor = before->current, v = actor ^ 1, from = g->effectFrom, s, s2, k, q;
    int effect = from == TCG_ATTACK || from == TCG_USE_POWER || from == TCG_PLAY_TRAINER;
    unsigned stadium = TcgDuel_Stadium(before, NULL);
    if (effect && before->phase != TCG_SETUP)
    {
        /* Unown [K] (the Unown), Brock's Protection (attacks and Trainers): the Energy cards the other player's effect
         * discarded from them go back on; Ecogym: a non-Colorless one goes to its owner's hand */
        int keep = TcgPower_InPlay(before, v, TCG_POWER_R_KEEP), eco = stadium == TCG_CARD_NEO1_84 ||
                                                                        TcgDuel_Stadium(g, NULL) == TCG_CARD_NEO1_84;
        const struct TcgPlayer *was = &before->players[v];
        struct TcgPlayer *now = &g->players[v];
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgFieldCard *f = &was->field[s];
            const struct TcgCard *c = f->stackSize ? Tcg_Card(was->cards[f->stack[f->stackSize - 1]]) : NULL;
            int guard;
            if (!c)
                continue;
            for (s2 = 0; s2 < TCG_FIELD_SIZE && !(now->field[s2].stackSize &&
                                                   now->field[s2].stack[now->field[s2].stackSize - 1] == f->stack[f->stackSize - 1]); s2++)
                ;
            if (s2 == TCG_FIELD_SIZE)
                continue; /* (Knocked Out: those don't count) */
            guard = (keep && strstr(c->name, "Unown")) ||
                    (from != TCG_USE_POWER && TcgDuel_AttachedCard(before, v, s, TCG_CARD_GYM2_101) >= 0);
            for (k = 0; k < f->attachedSize; k++)
            {
                unsigned e = f->attached[k];
                int at = PileAt(now->discard, now->discardSize, e);
                const struct TcgCard *ec = Tcg_Card(now->cards[e]);
                if (at < 0 || !ec || ec->kind != TCG_ENERGY)
                    continue;
                if (guard)
                {
                    TcgPile_Remove(now->discard, &now->discardSize, (unsigned)at);
                    now->field[s2].attached[now->field[s2].attachedSize++] = (uint8_t)e;
                }
                else if (eco && ec->type != TCG_COLORLESS)
                {
                    TcgPile_Remove(now->discard, &now->discardSize, (unsigned)at);
                    now->hand[now->handSize++] = (uint8_t)e;
                }
            }
        }
    }
    for (q = 0; q < 2 && before->phase != TCG_SETUP; q++)
    {
        const struct TcgPlayer *was = &before->players[q];
        struct TcgPlayer *now = &g->players[q];
        int pollen = (from == TCG_ATTACK || from == TCG_USE_POWER) && TcgPower_InPlay(before, 2, TCG_POWER_R_ALLERGIC_POLLEN);
        for (k = 0; k < was->discardSize; k++)
        {
            unsigned card = was->discard[k];
            if (PileAt(now->discard, now->discardSize, card) >= 0)
                continue;
            /* Parasect's Allergic Pollen: attacks and powers leave the discard piles alone; Pokemon Tower: nothing
             * goes from a discard pile into the hand */
            if ((pollen || (stadium == TCG_CARD_BASEP_42 && PileAt(now->hand, now->handSize, card) >= 0)) &&
                TakeBack(now, card))
                now->discard[now->discardSize++] = (uint8_t)card;
        }
    }
}
int TcgDuel_Apply(struct TcgDuel *g, const struct TcgAction *action)
{
    struct TcgDuel next = *g;
    int result;
    if (action->selectionCount > 60)
        return TCG_INVALID;
    next.error[0] = 0;
    if (action->type != TCG_CHOOSE)
        next.effectFrom = action->type;
    result = Apply(&next, action);
    if (result == TCG_OK)
    {
        /* GB2's Recycle Energy: put into the discard pile from play, it goes back to the hand */
        unsigned who, s, i;
        for (who = 0; who < 2; who++)
            for (s = 0; s < TCG_FIELD_SIZE; s++)
                for (i = 0; i < g->players[who].field[s].attachedSize; i++)
                {
                    uint8_t card = g->players[who].field[s].attached[i];
                    struct TcgPlayer *q = &next.players[who];
                    unsigned k;
                    if (Tcg_EnergyRule(g->players[who].cards[card]) != TCG_ER_RECYCLE)
                        continue;
                    for (k = 0; k < q->discardSize; k++)
                        if (q->discard[k] == card)
                        {
                            q->hand[q->handSize++] = TcgPile_Remove(q->discard, &q->discardSize, k);
                            TcgDuel_Log(&next, "Recycle Energy went back to the hand.");
                            break;
                        }
                }
        Undo(g, &next);
        if (!TcgDuel_Conserved(&next))
        {
            snprintf(g->error, sizeof(g->error), "Internal error: card conservation failed.");
            return TCG_INVALID;
        }
        *g = next;
    }
    else
        snprintf(g->error, sizeof(g->error), "%s", next.error);
    return result;
}

int TcgDuel_HarvestOk(const struct TcgDuel *g, unsigned player, unsigned slot)
{
    const struct TcgPlayer *p = &g->players[player];
    unsigned s;
    if (slot || p->energyPlayed != 2)
        return 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
        if (TcgPower_Working(g, player, s) == TCG_POWER_R_HARVEST_BOUNTY && !p->field[s].condition && !p->field[s].poison &&
            !p->field[s].burned)
            return 1;
    return 0;
}
int TcgDuel_OpponentSwitch(struct TcgDuel *g, unsigned who)
{
    unsigned k, bench = 0;
    for (k = 1; k < TCG_FIELD_SIZE; k++)
        bench += g->players[who].field[k].stackSize != 0;
    if (!bench || !g->players[who].field[0].stackSize || TcgDuel_Anchored(g, who) || g->phase != TCG_TURN)
        return 0;
    g->switchPending = 1;
    g->endAfterSwitch = 0;
    g->phase = TCG_PROMOTION;
    g->promotePlayer = (uint8_t)who;
    return 1;
}
/* (flow.h's: the names cards are searched by; here, as every build of the rules links this file) */
const char *TcgFlow_OwnerWord(unsigned owner)
{
    static const char *const words[TCG_OWNERS] = {"Brock", "Misty", "Erika", "Lt. Surge", "Sabrina", "Blaine",
                                                  "Giovanni", "Koga"};
    return owner < TCG_OWNERS ? words[owner] : "";
}
int TcgFlow_InNameGroup(const char *name, unsigned group)
{
    static const char *const groups[TCG_NAME_GROUPS][5] = {
        {"Gyarados", "Dark Gyarados", "Shining Gyarados"},
        {"Tentacool", "Tentacruel", "Misty's Tentacool", "Misty's Tentacruel"},
        {"Sabrina's Gastly", "Sabrina's Haunter", "Sabrina's Gengar"},
        {"Omanyte", "Omastar", "Kabuto", "Kabutops"},
        {"Giovanni's Nidoking"},
        {"Murkrow"},
        {"Magnemite", "Magneton", "Dark Magneton"},
    };
    unsigned i;
    for (i = 0; group < TCG_NAME_GROUPS && i < 5 && groups[group][i]; i++)
        if (!strcmp(name, groups[group][i]))
            return 1;
    return 0;
}
