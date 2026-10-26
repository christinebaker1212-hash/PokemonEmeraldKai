#include "mp_version.h"
#if MOEMON_DEV_BUILD || defined(TCG_TEST)
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <string.h>
/* Decisions use public opposing cards/counts and the AI's own cards only.
 * Never score the outcome of a simulated coin flip or inspect the opponent's
 * hidden hand/deck/prizes when choosing an action. */
static int Legal(const struct TcgDuel *g, const struct TcgAction *a)
{
    struct TcgDuel next = *g;
    return TcgDuel_Apply(&next, a) == TCG_OK;
}
static unsigned Score(const struct TcgCard *c)
{
    if (c->kind == TCG_ENERGY)
        return 25;
    if (c->kind == TCG_TRAINER)
        return c->id == TCG_CARD_BILL || c->id == TCG_CARD_PROFESSOR_OAK ? 45 : 15;
    return c->hp + c->attacks[0].damage + c->attacks[1].damage / 2;
}
static int SelectCost(const struct TcgPlayer *p, unsigned played, uint8_t *out, unsigned count)
{
    unsigned i, j;
    int scores[60];
    for (i = 0; i < p->handSize; i++)
        scores[i] = p->hand[i] == played ? 100000 : (int)Score(Tcg_Card(p->cards[p->hand[i]]));
    for (j = 0; j < count; j++)
    {
        unsigned best = TCG_NONE;
        for (i = 0; i < p->handSize; i++)
            if (best == TCG_NONE || scores[i] < scores[best])
                best = i;
        if (best == TCG_NONE || scores[best] >= 100000)
            return 0;
        out[j] = p->hand[best];
        scores[best] = 100000;
    }
    return 1;
}
static unsigned BestPokemon(const struct TcgDuel *g, unsigned who)
{
    unsigned i, best = TCG_NONE, score = 0;
    for (i = 1; i < TCG_FIELD_SIZE; i++)
    {
        const struct TcgCard *c = TcgDuel_Field(g, who, i);
        unsigned value;
        if (!c)
            continue;
        value = Score(c) + 20 * TcgDuel_Energy(g, who, i, TCG_NONE);
        if (best == TCG_NONE || value > score)
        {
            best = i;
            score = value;
        }
    }
    return best;
}
static int Search(const struct TcgPlayer *p, const uint8_t *pile, unsigned count, int kind, int basicEnergy)
{
    unsigned i;
    int best = -1;
    unsigned score = 0;
    for (i = 0; i < count; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[pile[i]]);
        unsigned value;
        if ((kind >= 0 && c->kind != (unsigned)kind) || (basicEnergy && c->id > 6))
            continue;
        value = Score(c);
        if (best < 0 || value > score)
        {
            best = pile[i];
            score = value;
        }
    }
    return best;
}
static int TrainerAction(const struct TcgDuel *g, unsigned hand, struct TcgAction *a)
{
    const struct TcgPlayer *p = &g->players[g->current], *op = &g->players[g->current ^ 1];
    unsigned id = p->cards[p->hand[hand]], i, j;
    int selected;
    unsigned best = 0;
    memset(a, 0, sizeof(*a));
    a->type = TCG_PLAY_TRAINER;
    a->hand = hand;
    switch (id)
    {
    case TCG_CARD_BILL:
        break;
    case TCG_CARD_PROFESSOR_OAK:
        if (p->handSize > 3 || p->deckSize < 7)
            return 0;
        break;
    case TCG_CARD_IMPOSTER_PROFESSOR_OAK:
        if (op->handSize <= 7)
            return 0;
        break;
    case TCG_CARD_LASS:
        if (op->handSize < p->handSize + 4)
            return 0;
        break;
    case TCG_CARD_POTION:
    case TCG_CARD_SUPER_POTION:
    case TCG_CARD_DEFENDER:
        for (i = 1; i < TCG_FIELD_SIZE; i++)
            if (p->field[i].stackSize && p->field[i].damage > p->field[best].damage)
                best = i;
        a->slot = best;
        if (id == TCG_CARD_SUPER_POTION)
        {
            for (i = 0; i < p->field[best].attachedSize; i++)
                if (Tcg_Card(p->cards[p->field[best].attached[i]])->kind == TCG_ENERGY)
                {
                    a->selections[a->selectionCount++] = p->field[best].attached[i];
                    break;
                }
        }
        break;
    case TCG_CARD_PLUSPOWER:
        if (!TcgDuel_CanAttack(g, g->current, 0) && !TcgDuel_CanAttack(g, g->current, 1))
            return 0;
        break;
    case TCG_CARD_FULL_HEAL:
    case TCG_CARD_POKEMON_CENTER:
        break;
    case TCG_CARD_CLEFAIRY_DOLL:
    case TCG_CARD_MYSTERIOUS_FOSSIL:
        return 0; /* Avoid filling the bench without a deck-specific purpose. */
    case TCG_CARD_ENERGY_SEARCH:
    case TCG_CARD_POKE_BALL:
        selected = Search(p, p->deck, p->deckSize, id == TCG_CARD_POKE_BALL ? TCG_POKEMON : TCG_ENERGY,
                          id == TCG_CARD_ENERGY_SEARCH);
        if (selected < 0)
            return 0;
        a->selections[a->selectionCount++] = selected;
        break;
    case TCG_CARD_COMPUTER_SEARCH:
    case TCG_CARD_ITEM_FINDER:
        if (!SelectCost(p, p->hand[hand], a->selections, 2))
            return 0;
        selected = Search(p, id == TCG_CARD_ITEM_FINDER ? p->discard : p->deck,
                          id == TCG_CARD_ITEM_FINDER ? p->discardSize : p->deckSize,
                          id == TCG_CARD_ITEM_FINDER ? TCG_TRAINER : -1, 0);
        if (selected < 0)
            return 0;
        a->selections[2] = selected;
        a->selectionCount = 3;
        break;
    case TCG_CARD_ENERGY_RETRIEVAL:
    case TCG_CARD_SUPER_ENERGY_RETRIEVAL:
        j = id == TCG_CARD_ENERGY_RETRIEVAL ? 1 : 2;
        if (!SelectCost(p, p->hand[hand], a->selections, j))
            return 0;
        a->selectionCount = j;
        for (i = 0; i < p->discardSize && a->selectionCount < j + (j == 1 ? 2 : 4); i++)
            if (p->cards[p->discard[i]] <= 6)
                a->selections[a->selectionCount++] = p->discard[i];
        if (a->selectionCount == j)
            return 0;
        break;
    case TCG_CARD_ENERGY_REMOVAL:
    case TCG_CARD_SUPER_ENERGY_REMOVAL:
        if (id == TCG_CARD_SUPER_ENERGY_REMOVAL)
        {
            for (i = 0; i < TCG_FIELD_SIZE; i++)
            {
                for (j = 0; j < p->field[i].attachedSize; j++)
                    if (Tcg_Card(p->cards[p->field[i].attached[j]])->kind == TCG_ENERGY)
                    {
                        a->slot = i;
                        a->selections[a->selectionCount++] = p->field[i].attached[j];
                        break;
                    }
                if (a->selectionCount)
                    break;
            }
            if (!a->selectionCount)
                return 0;
        }
        for (i = 0; i < TCG_FIELD_SIZE; i++)
        {
            unsigned initial = a->selectionCount;
            for (j = 0; j < op->field[i].attachedSize; j++)
                if (Tcg_Card(op->cards[op->field[i].attached[j]])->kind == TCG_ENERGY)
                {
                    a->target = i;
                    a->selections[a->selectionCount++] = op->field[i].attached[j];
                    if (id == TCG_CARD_ENERGY_REMOVAL || a->selectionCount == 3)
                        break;
                }
            if (a->selectionCount > initial)
                break;
        }
        break;
    case TCG_CARD_POKEDEX:
        a->selectionCount = p->deckSize < 5 ? p->deckSize : 5;
        for (i = 0; i < a->selectionCount; i++)
            a->selections[i] = p->deck[p->deckSize - 1 - i];
        for (i = 0; i < a->selectionCount; i++)
            for (j = i + 1; j < a->selectionCount; j++)
                if (Score(Tcg_Card(p->cards[a->selections[j]])) > Score(Tcg_Card(p->cards[a->selections[i]])))
                {
                    uint8_t t = a->selections[i];
                    a->selections[i] = a->selections[j];
                    a->selections[j] = t;
                }
        break;
    case TCG_CARD_GUST_OF_WIND:
        for (i = 1; i < TCG_FIELD_SIZE; i++)
        {
            const struct TcgCard *c = TcgDuel_Field(g, g->current ^ 1, i),
                                 *active = TcgDuel_Field(g, g->current ^ 1, 0);
            if (c && active && c->hp - op->field[i].damage < active->hp - op->field[0].damage)
            {
                a->target = i;
                break;
            }
        }
        if (!a->target)
            return 0;
        break;
    case TCG_CARD_REVIVE:
        for (i = 1; i < TCG_FIELD_SIZE; i++)
            if (!p->field[i].stackSize)
            {
                a->slot = i;
                break;
            }
        for (i = 0; i < p->discardSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->discard[i]]);
            if (c->kind == TCG_POKEMON && !c->stage)
            {
                a->selections[0] = p->discard[i];
                a->selectionCount = 1;
                break;
            }
        }
        break;
    case TCG_CARD_RECYCLE:
        selected = Search(p, p->discard, p->discardSize, -1, 0);
        if (selected < 0)
            return 0;
        a->selections[0] = selected;
        a->selectionCount = 1;
        break;
    default:
        return 0;
    }
    return Legal(g, a);
}
int TcgDuel_AiAction(const struct TcgDuel *g, struct TcgAction *a)
{
    const struct TcgPlayer *p;
    unsigned i, s, attack, bestAttack = TCG_NONE, bestScore = 0;
    memset(a, 0, sizeof(*a));
    if (g->phase == TCG_FINISHED)
        return 0;
    if (g->phase == TCG_PROMOTION)
    {
        a->type = TCG_PROMOTE;
        a->slot = BestPokemon(g, g->promotePlayer);
        return a->slot != TCG_NONE;
    }
    p = &g->players[g->current];
    if (g->phase == TCG_SETUP)
    {
        if (!p->field[0].stackSize)
        {
            int best = -1;
            for (i = 0; i < p->handSize; i++)
            {
                const struct TcgCard *c = Tcg_Card(p->cards[p->hand[i]]);
                if (c->kind == TCG_POKEMON && !c->stage && (best < 0 || Score(c) > bestScore))
                {
                    best = i;
                    bestScore = Score(c);
                }
            }
            if (best < 0)
                return 0;
            a->type = TCG_PLAY_BASIC;
            a->hand = best;
            a->slot = 0;
            return 1;
        }
    }
    /* Evolution precedes Energy attachment, so attachment choices use the final
     * attack costs. No random outcomes are examined when testing legality. */
    if (g->phase == TCG_TURN)
        for (i = 0; i < p->handSize; i++)
        {
            const struct TcgCard *c = Tcg_Card(p->cards[p->hand[i]]);
            if (c->kind != TCG_POKEMON || !c->stage)
                continue;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                a->type = TCG_EVOLVE;
                a->hand = i;
                a->slot = s;
                if (Legal(g, a))
                    return 1;
            }
        }
    for (i = 0; i < p->handSize; i++)
    {
        const struct TcgCard *c = Tcg_Card(p->cards[p->hand[i]]);
        if (c->kind != TCG_POKEMON || c->stage)
            continue;
        for (s = 1; s < TCG_FIELD_SIZE; s++)
            if (!p->field[s].stackSize)
            {
                a->type = TCG_PLAY_BASIC;
                a->hand = i;
                a->slot = s;
                return 1;
            }
    }
    if (g->phase == TCG_SETUP)
    {
        a->type = TCG_FINISH_SETUP;
        return 1;
    }
    if (!p->energyPlayed)
    {
        int bestHand = -1;
        unsigned bestSlot = 0;
        int energyScore = -100000;
        for (i = 0; i < p->handSize; i++)
        {
            const struct TcgCard *energy = Tcg_Card(p->cards[p->hand[i]]);
            if (energy->kind != TCG_ENERGY)
                continue;
            for (s = 0; s < TCG_FIELD_SIZE; s++)
            {
                const struct TcgCard *c = TcgDuel_Field(g, g->current, s);
                int score;
                if (!c || c->kind != TCG_POKEMON)
                    continue;
                score = s == 0 ? 40 : 0;
                for (attack = 0; attack < 2; attack++)
                {
                    const struct TcgAttack *move = &c->attacks[attack];
                    if (move->name[0] && move->category != TCG_POKEMON_POWER)
                    {
                        unsigned have = TcgDuel_Energy(g, g->current, s, energy->type);
                        if (have < move->cost[energy->type])
                            score += 50;
                        else if (TcgDuel_Energy(g, g->current, s, TCG_NONE) < move->cost[TCG_COLORLESS])
                            score += 20;
                    }
                }
                score -= 10 * TcgDuel_Energy(g, g->current, s, TCG_NONE);
                if (bestHand < 0 || score > energyScore)
                {
                    bestHand = i;
                    bestSlot = s;
                    energyScore = score;
                }
            }
        }
        if (bestHand >= 0)
        {
            a->type = TCG_ATTACH;
            a->hand = bestHand;
            a->slot = bestSlot;
            return 1;
        }
    }
    for (i = 0; i < p->handSize; i++)
        if (Tcg_Card(p->cards[p->hand[i]])->kind == TCG_TRAINER && TrainerAction(g, i, a))
            return 1;
    for (attack = 0; attack < 2; attack++)
    {
        const struct TcgCard *c = TcgDuel_Field(g, g->current, 0);
        unsigned score;
        if (!c || !TcgDuel_CanAttack(g, g->current, attack) || !TcgDuel_EffectSupported(c->attacks[attack].effect))
            continue;
        score = c->attacks[attack].damage;
        if (bestAttack == TCG_NONE || score > bestScore)
        {
            bestAttack = attack;
            bestScore = score;
        }
    }
    if (bestAttack != TCG_NONE && p->field[0].condition != TCG_ASLEEP && p->field[0].condition != TCG_PARALYZED &&
        !p->field[0].noAttack)
    {
        a->type = TCG_ATTACK;
        a->attack = bestAttack;
        return 1;
    }
    a->type = TCG_PASS;
    return 1;
}
#endif
