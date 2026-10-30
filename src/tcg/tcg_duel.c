#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <stdarg.h>
#include <stdio.h>
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
    TcgDuel_Event(g, TCG_EV_COIN, player, 0, heads, 0, 0);
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
static void StartTurn(struct TcgDuel *g)
{
    struct TcgPlayer *p = &g->players[g->current];
    unsigned s;
    p->energyPlayed = 0;
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
        /* (Charizard's Energy Burn: all of it is Fire Energy this turn) */
        unsigned kind = f->burn ? TCG_FIRE : c->type;
        if (c->kind == TCG_ENERGY && (type == TCG_NONE || type == kind))
            n += c->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1;
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
    return f->noAttack == 1 || (f->noAttack == 2 && TcgDuel_BoundEffect(g, player, 0));
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
/* Slashing Strike, Drill Dive and the like: that attack (lockAttack 3: any) can't be used this turn */
static int Locked(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgFieldCard *f = &g->players[player].field[0];
    return f->lockTurn == g->turn && (f->lockAttack == 3 || f->lockAttack == attack + 1);
}
int TcgDuel_CanAttack(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, player);
    /* (Amnesia: not the attack chosen, this turn) */
    return EnergyPaid(g, player, attack) && g->players[player].field[0].disabled != attack + 1 &&
           !Locked(g, player, attack) && !TcgDuel_EffectBlock(g, player, &c->attacks[attack]);
}
int TcgDuel_CanAttackNow(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, player);
    const struct TcgFieldCard *f = &g->players[player].field[0];
    if (!c || attack > 1 || c->kind != TCG_POKEMON || f->condition == TCG_PARALYZED || TcgDuel_NoAttack(g, player) ||
        Locked(g, player, attack))
        return 0;
    /* (Big Snore: only while Asleep) */
    return f->condition != TCG_ASLEEP || TcgDuel_SleepAttack(&c->attacks[attack]);
}
static int EnergyPaid(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Attacker(g, player);
    const struct TcgAttack *a;
    unsigned t, total = 0, typed = 0, from = EnergySlot(g, player);
    if (!c || attack > 1 || c->kind != TCG_POKEMON)
        return 0;
    a = &c->attacks[attack];
    if (!a->name[0] || a->category == TCG_POKEMON_POWER)
        return 0;
    for (t = 0; t < TCG_TYPES; t++)
    {
        total += a->cost[t];
        if (t != TCG_COLORLESS)
        {
            if (TcgDuel_Energy(g, player, from, t) < a->cost[t])
                return 0;
            typed += a->cost[t];
        }
    }
    (void)typed;
    return TcgDuel_Energy(g, player, from, TCG_NONE) >= total;
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
void TcgDuel_ResolveKnockouts(struct TcgDuel *g)
{
    unsigned who, s, win[2] = {0};
    /* Resolve both boards before checking victory; simultaneous KOs must not
     * award victory to whichever side happened to be scanned first. */
    for (who = 0; who < 2; who++)
        for (s = 0; s < TCG_FIELD_SIZE; s++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, who, s);
            struct TcgFieldCard *f = &g->players[who].field[s];
            unsigned hp = c ? (c->kind == TCG_TRAINER ? 10 : c->hp) : 0;
            if (c && f->damage >= hp)
            {
                struct TcgPlayer *winner = &g->players[who ^ 1];
                unsigned prize = !f->noPrize;
                TcgDuel_Log(g, "%s is Knocked Out.", c->name);
                TcgDuel_Event(g, TCG_EV_KO, who, s, 0, c->id, 0);
                TcgDuel_DiscardField(g, who, s);
                if (prize && winner->prizeSize)
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
        if (p->cards[f->attached[i]] == id)
            count++;
    return count;
}

static void EndTurn(struct TcgDuel *g)
{
    unsigned who, s;
    struct TcgPlayer *p = &g->players[g->current];
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
    }
    p->noTrainers = 0;
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
        if (f->condition == TCG_ASLEEP && TcgDuel_Coin(g, who))
        {
            f->condition = TCG_HEALTHY;
            TcgDuel_Event(g, TCG_EV_STATUS, who, 0, TCG_HEALTHY, 0, f->poison);
        }
        if (f->defender)
            f->defender--;
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
        unsigned damage, flags = 0, selfCondition, type, k;
        uint16_t struck[TCG_FIELD_SIZE];
        const char *blocked;
        int result, total;
        f = &p->field[0];
        if (!attacker || !target || attacker->kind != TCG_POKEMON || a->attack > 1 ||
            !EnergyPaid(g, g->current, a->attack))
            return Error(g, "That attack cannot be used with the attached Energy.");
        attack = &attacker->attacks[a->attack];
        if (f->disabled == a->attack + 1)
            return Error(g, "Amnesia: that attack can't be used this turn.");
        if ((blocked = TcgDuel_EffectBlock(g, g->current, attack)))
            return Error(g, blocked);
        if (TcgDuel_IsMetronome(attack))
        {
            /* Metronome: one of the Defending Pokemon's attacks, without its Energy cost (its type stays its own) */
            if (!(attack = TcgDuel_MetronomeAttack(g, g->current, a->copy)))
                return Error(g, "Choose one of the Defending Pokemon's attacks to copy.");
            if ((blocked = TcgDuel_EffectBlock(g, g->current, attack)))
                return Error(g, blocked);
            TcgDuel_Log(g, "Metronome copies %s.", attack->name);
        }
        if (!TcgDuel_CanAttackNow(g, g->current, a->attack))
            return Error(g, "This Pokemon cannot attack now.");
        if (!TcgDuel_EffectSupported(attack->effect))
        {
            snprintf(g->error, sizeof(g->error), "Effect not yet implemented: %s", attack->effect);
            return TCG_UNSUPPORTED;
        }
        g->noWeakRes = g->attackFailed = g->coinResult = g->swift = g->bonus = 0;
        TcgDuel_Event(g, TCG_EV_ATTACK, g->current, a->attack, 0, attacker->id, 0);
        if (f->condition == TCG_CONFUSED && !TcgDuel_Coin(g, g->current))
        {
            TcgDuel_SelfDamage(g, TcgPower_Frenzy(g, g->current) ? 50 : 20); /* (Dark Primeape's Frenzy: 30 more) */
            TcgDuel_Log(g, "Confusion: %s does damage to itself.", attacker->name);
            TcgDuel_ResolveKnockouts(g);
            if (g->phase != TCG_FINISHED && g->phase != TCG_SETUP)
                EndTurn(g);
            return TCG_OK;
        }
        if (f->weaken == TCG_WEAKEN_SMOKESCREEN && !TcgDuel_Coin(g, g->current))
        {
            /* Sand-attack / Smokescreen: tails, and the attack does nothing. */
            TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current, 0, 1, 0, 0);
            TcgDuel_Log(g, "%s's attack missed.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        if (TcgDuel_MetronomeHeads(&attacker->attacks[a->attack]) && !TcgDuel_Coin(g, g->current))
        {
            /* Mini-Metronome: tails, nothing */
            TcgDuel_Log(g, "%s's attack failed.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        /* Before the attack: Fox Fire, Drag Off bring out another Defending Pokemon */
        damage = attack->damage;
        if ((result = TcgDuel_AttackEffect(g, attack, a, &damage, 3)) != TCG_OK)
            return result;
        if (!(target = TcgDuel_Field(g, g->current ^ 1, 0)))
            return Error(g, "There's no Defending Pokemon.");
        foeBefore = *foe;
        selfCondition = f->condition;
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
            total += g->bonus + TcgPower_AttackBonus(g, total); /* (Frenzy, Dark Wave, Prehistoric Dream) */
            if (!g->noWeakRes && TcgDuel_Weakness(g, g->current ^ 1, 0) == type)
            {
                total *= 2;
                flags |= TCG_DMG_WEAK;
            }
            if (!g->noWeakRes && TcgDuel_Resistance(g, g->current ^ 1, 0) == type)
            {
                total -= 30;
                flags |= TCG_DMG_RESIST;
            }
            total += 10 * (int)AttachedTrainerCount(g, g->current, 0, TCG_CARD_PLUSPOWER);
            if (!g->swift)
                total -= 20 * (int)AttachedTrainerCount(g, g->current ^ 1, 0, TCG_CARD_DEFENDER);
            if (g->swift)
                ;
            else if (foe->barrier || g->foeImmune == TCG_IMMUNE_YES)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (foe->guard == TCG_GUARD_REDUCE_10)
                total -= 10;
            else if (foe->guard == TCG_GUARD_REDUCE_20)
                total -= 20;
            else if (foe->guard == TCG_GUARD_UNDER_40 && total < 40)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (foe->guard == TCG_GUARD_HALVE)
                total = total / 2 / 10 * 10;
            /* GB2's: Harden, Double-Edged Pincers, Blink, Expand */
            else if (foe->guard == TCG_GUARD_UNDER_30 && total <= 20)
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (foe->guard == TCG_GUARD_DOUBLE)
                total *= 2;
            else if (foe->guard == TCG_GUARD_BLINK && TcgDuel_Coin(g, g->current ^ 1))
            {
                TcgDuel_Event(g, TCG_EV_NO_DAMAGE, g->current ^ 1, 0, 0, 0, 0);
                total = 0;
            }
            else if (foe->guard == TCG_GUARD_REDUCE_10_LATE)
                total -= 10;
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
        if (damage && foe->destinyBond && foe->damage >= (target->kind == TCG_TRAINER ? 10u : target->hp) &&
            f->damage < attacker->hp)
        {
            /* Destiny Bond: the Pokemon that Knocks it Out is Knocked Out too */
            TcgDuel_Event(g, TCG_EV_DAMAGE, g->current, 0, attacker->hp - f->damage, 0, TCG_DMG_SELF);
            f->damage = attacker->hp;
            TcgDuel_Log(g, "Destiny Bond: %s is Knocked Out too.", attacker->name);
        }
        if (damage && foe->damage >= (target->kind == TCG_TRAINER ? 10u : target->hp))
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
        if (foeBefore.barrier == 2 || g->foeImmune == TCG_IMMUNE_YES)
        {
            /* Agility, Fly, Barrier (and the powers above): no effect of the attack reaches it. */
            foe->condition = foeBefore.condition;
            foe->poison = foeBefore.poison;
            foe->noAttack = foeBefore.noAttack;
            foe->noRetreat = foeBefore.noRetreat;
            foe->weaken = foeBefore.weaken;
            foe->effectBy = foeBefore.effectBy;
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
        if (f->stackSize && f->condition != selfCondition)
            TcgDuel_Event(g, TCG_EV_STATUS, g->current, 0, f->condition, 0, f->poison);
        g->animationCard = attacker->id;
        g->animationKind = a->attack + 1;
        g->noWeakRes = g->attackFailed = g->coinResult = g->foeImmune = g->swift = g->bonus = 0;
        if (g->switchPending)
        {
            /* Only if the Defending Pokemon is still there to be switched. It comes before any
             * Knock Out: Ram switches "even if Rhydon is Knocked Out" (the end of the turn,
             * after the switch, takes care of that). */
            unsigned who = g->current ^ 1, bench = 0, k, hp = target->kind == TCG_TRAINER ? 10 : target->hp;
            for (k = 1; k < TCG_FIELD_SIZE; k++)
                bench += g->players[who].field[k].stackSize != 0;
            if (g->players[who].field[0].stackSize && g->players[who].field[0].damage < hp && bench)
            {
                g->phase = TCG_PROMOTION;
                g->promotePlayer = who;
                g->endAfterSwitch = 1;
                return TCG_OK;
            }
            g->switchPending = 0;
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
            p->confusionRetreatFailed || (f->lockTurn == g->turn && f->lockAttack == 3))
            return Error(g, "This Pokemon cannot retreat.");
        {
            const char *why = TcgPower_RetreatBlock(g, g->current);
            if (why)
                return Error(g, why);
        }
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
            n += c->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1;
            p->discard[p->discardSize++] = TcgPile_Remove(f->attached, &f->attachedSize, j);
        }
        c = TcgDuel_Field(g, g->current, 0);
        /* (each Dodrio on the Bench: Retreat Aid, one Colorless less; Dark Muk's Sticky Goo, 2 more) */
        if (n < TcgDuel_RetreatCost(g, g->current))
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
        return TcgDuel_Power(g, a);
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
        if (TcgPower_NoTrainers(g))
            return Error(g, "Dark Vileplume's Hay Fever: no Trainer cards can be played.");
        return TcgDuel_Trainer(g, a);
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
            g->fromHand = 1;
            TcgPower_OnPlay(g, g->current, s);
            g->fromHand = 0;
            TcgPower_Sanitize(g);
            TcgDuel_ResolveKnockouts(g);
        }
        return TCG_OK;
    case TCG_EVOLVE:
        if (c->kind != TCG_POKEMON || !c->stage || !f->stackSize || f->stackSize == 4 ||
            strcmp(c->evolvesFrom, TcgDuel_Field(g, g->current, s)->name))
            return Error(g, "This Pokemon does not evolve from that card.");
        if (p->turnsTaken < 2 || f->enteredTurn == g->turn)
            return Error(g, "A Pokemon cannot evolve on its first turn in play.");
        if (TcgPower_InPlay(g, 2, TCG_POWER_PREHISTORIC))
            return Error(g, "Aerodactyl's Prehistoric Power: no Evolution cards can be played.");
        f->stack[f->stackSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        f->enteredTurn = g->turn;
        ClearCondition(f);
        TcgDuel_Event(g, TCG_EV_EVOLVE, g->current, s, 0, c->id, 0);
        TcgDuel_Log(g, "Evolved into %s.", c->name);
        g->fromHand = 1;
        TcgPower_OnPlay(g, g->current, s); /* (Healing Wind) */
        g->fromHand = 0;
        TcgPower_Sanitize(g);
        TcgDuel_ResolveKnockouts(g);
        return TCG_OK;
    case TCG_ATTACH:
        if (c->kind != TCG_ENERGY || !f->stackSize || p->energyPlayed)
            return Error(g, "Only one Energy card may be attached from your hand each turn.");
        f->attached[f->attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        p->energyPlayed = 1;
        TcgDuel_Event(g, TCG_EV_ATTACH, g->current, s, 0, c->id, 0);
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
int TcgDuel_Apply(struct TcgDuel *g, const struct TcgAction *action)
{
    struct TcgDuel next = *g;
    int result;
    if (action->selectionCount > 60)
        return TCG_INVALID;
    next.error[0] = 0;
    result = Apply(&next, action);
    if (result == TCG_OK)
    {
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
