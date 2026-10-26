#include "mp_version.h"
#if MOEMON_DEV_BUILD || defined(TCG_TEST)
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
    p->turnsTaken++;
    g->turn++;
    g->phase = TCG_TURN;
    g->needsTurnStart = 0;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        p->field[s].powerUsed = 0;
        p->field[s].barrier = 0;
    }
    if (!TcgDuel_Draw(g, g->current, 1))
    {
        g->winner = g->current ^ 1;
        g->phase = TCG_FINISHED;
        TcgDuel_Log(g, "Player %u cannot draw a card.", g->current + 1);
        return;
    }
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
        if (c->kind == TCG_ENERGY && (type == TCG_NONE || type == c->type))
            n += c->id == TCG_CARD_DOUBLE_COLORLESS_ENERGY ? 2 : 1;
    }
    return n;
}
int TcgDuel_CanAttack(const struct TcgDuel *g, unsigned player, unsigned attack)
{
    const struct TcgCard *c = TcgDuel_Field(g, player, 0);
    const struct TcgAttack *a;
    unsigned t, total = 0, typed = 0;
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
            if (TcgDuel_Energy(g, player, 0, t) < a->cost[t])
                return 0;
            typed += a->cost[t];
        }
    }
    (void)typed;
    return TcgDuel_Energy(g, player, 0, TCG_NONE) >= total;
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
                TcgDuel_DiscardField(g, who, s);
                if (prize && winner->prizeSize)
                    winner->hand[winner->handSize++] = winner->prizes[--winner->prizeSize];
            }
        }
    for (who = 0; who < 2; who++)
        win[who] = !g->players[who].prizeSize || !PokemonCount(&g->players[who ^ 1]);
    if (win[0] && win[1])
    {
        uint16_t decks[2][60];
        uint32_t rng = g->rng;
        memcpy(decks[0], g->players[0].cards, sizeof(decks[0]));
        memcpy(decks[1], g->players[1].cards, sizeof(decks[1]));
        TcgDuel_Init(g, decks[0], decks[1], rng, 1);
        TcgDuel_Log(g, "Sudden death: play again with one Prize card.");
        return;
    }
    if (win[0] || win[1])
    {
        g->winner = win[0] ? 0 : 1;
        g->phase = TCG_FINISHED;
        TcgDuel_Log(g, "Player %u wins the duel.", g->winner + 1);
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
    if (p->field[0].condition == TCG_PARALYZED)
        p->field[0].condition = TCG_HEALTHY;
    for (s = 0; s < TCG_FIELD_SIZE; s++)
    {
        p->field[s].plusPower = 0;
        p->field[s].noAttack = 0;
        p->field[s].noRetreat = 0;
    }
    for (who = 0; who < 2; who++)
    {
        struct TcgFieldCard *f = &g->players[who].field[0];
        if (!f->stackSize)
            continue;
        if (f->poison)
            f->damage += f->poison;
        if (f->condition == TCG_ASLEEP && Tcg_RandomBelow(&g->rng, 2))
            f->condition = TCG_HEALTHY;
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
        return TCG_OK;
    }
    if (g->phase == TCG_PROMOTION)
    {
        unsigned who = g->promotePlayer;
        if (a->type != TCG_PROMOTE || s < 1 || s >= TCG_FIELD_SIZE || !g->players[who].field[s].stackSize)
            return Error(g, "Choose a Benched Pokemon to promote.");
        g->players[who].field[0] = g->players[who].field[s];
        memset(&g->players[who].field[s], 0, sizeof(struct TcgFieldCard));
        ClearCondition(&g->players[who].field[0]);
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
        const struct TcgCard *attacker = TcgDuel_Field(g, g->current, 0), *target = TcgDuel_Field(g, g->current ^ 1, 0);
        const struct TcgAttack *attack;
        unsigned damage;
        int result;
        f = &p->field[0];
        if (!attacker || !target || !TcgDuel_CanAttack(g, g->current, a->attack))
            return Error(g, "That attack cannot be used with the attached Energy.");
        if (f->condition == TCG_ASLEEP || f->condition == TCG_PARALYZED || f->noAttack)
            return Error(g, "This Pokemon cannot attack now.");
        attack = &attacker->attacks[a->attack];
        if (!TcgDuel_EffectSupported(attack->effect))
        {
            snprintf(g->error, sizeof(g->error), "Effect not yet implemented: %s", attack->effect);
            return TCG_UNSUPPORTED;
        }
        if (f->condition == TCG_CONFUSED && !Tcg_RandomBelow(&g->rng, 2))
        {
            f->damage += 20;
            TcgDuel_Log(g, "Confusion: %s does 20 damage to itself.", attacker->name);
            EndTurn(g);
            return TCG_OK;
        }
        damage = attack->damage;
        result = TcgDuel_AttackEffect(g, attack, a, &damage, 0);
        if (result != TCG_OK)
            return result;
        if (target->weakness == attacker->type)
            damage *= 2;
        if (target->resistance == attacker->type)
            damage = damage > 30 ? damage - 30 : 0;
        if (damage)
            damage += 10 * AttachedTrainerCount(g, g->current, 0, TCG_CARD_PLUSPOWER);
        {
            unsigned reduction = 20 * AttachedTrainerCount(g, g->current ^ 1, 0, TCG_CARD_DEFENDER);
            damage = damage > reduction ? damage - reduction : 0;
        }
        if (g->players[g->current ^ 1].field[0].barrier)
            damage = 0;
        TcgDuel_Damage(g, g->current ^ 1, 0, damage);
        TcgDuel_Log(g, "%s uses %s for %u damage.", attacker->name, attack->name, damage);
        result = TcgDuel_AttackEffect(g, attack, a, &damage, 1);
        if (result != TCG_OK)
            return result;
        g->animationCard = attacker->id;
        g->animationKind = a->attack + 1;
        TcgDuel_ResolveKnockouts(g);
        if (g->phase != TCG_FINISHED && g->phase != TCG_SETUP)
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
        if (c->kind == TCG_TRAINER || f->condition == TCG_ASLEEP || f->condition == TCG_PARALYZED || f->noRetreat ||
            p->confusionRetreatFailed)
            return Error(g, "This Pokemon cannot retreat.");
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
        if (n < c->retreat)
            return Error(g, "Not enough Energy selected for retreat.");
        if (f->condition == TCG_CONFUSED && !Tcg_RandomBelow(&g->rng, 2))
        {
            p->confusionRetreatFailed = 1;
            TcgDuel_Log(g, "Confusion: retreat failed after discarding Energy.");
            return TCG_OK;
        }
        temp = *f;
        *f = p->field[s];
        p->field[s] = temp;
        ClearCondition(&p->field[s]);
        p->confusionRetreatFailed = 0;
        return TCG_OK;
    }
    if (a->type == TCG_USE_POWER)
        return TcgDuel_Power(g, a);
    if (a->hand >= p->handSize)
        return Error(g, "Choose a card in your hand.");
    c = Tcg_Card(p->cards[p->hand[a->hand]]);
    if (a->type == TCG_PLAY_TRAINER)
        return TcgDuel_Trainer(g, a);
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
        TcgDuel_Log(g, "Player %u plays %s.", g->current + 1, c->name);
        return TCG_OK;
    case TCG_EVOLVE:
        if (c->kind != TCG_POKEMON || !c->stage || !f->stackSize || f->stackSize == 4 ||
            strcmp(c->evolvesFrom, TcgDuel_Field(g, g->current, s)->name))
            return Error(g, "This Pokemon does not evolve from that card.");
        if (p->turnsTaken < 2 || f->enteredTurn == g->turn)
            return Error(g, "A Pokemon cannot evolve on its first turn in play.");
        f->stack[f->stackSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        f->enteredTurn = g->turn;
        ClearCondition(f);
        TcgDuel_Log(g, "Evolved into %s.", c->name);
        return TCG_OK;
    case TCG_ATTACH:
        if (c->kind != TCG_ENERGY || !f->stackSize || p->energyPlayed)
            return Error(g, "Only one Energy card may be attached from your hand each turn.");
        f->attached[f->attachedSize++] = TcgPile_Remove(p->hand, &p->handSize, a->hand);
        p->energyPlayed = 1;
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
#endif
