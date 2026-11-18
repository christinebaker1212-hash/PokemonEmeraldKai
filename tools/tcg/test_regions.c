/* The regional sets' finished cards (docs/TCG_REGIONS.md): every attack used on a staged board with different
 * coins and the AI's choices, the duel going on after; then decks of them in AI duels to the end. Every action legal,
 * every card kept where it can be counted. */
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void Deck(uint16_t *cards, unsigned id)
{
    unsigned i;
    for (i = 0; i < 60; i++)
        cards[i] = i % 2 ? TCG_CARD_FIRE_ENERGY : TCG_CARD_WATER_ENERGY;
    for (i = 30; i < 42; i++)
        cards[i] = i % 3 == 0 ? TCG_CARD_LIGHTNING_ENERGY : i % 3 == 1 ? TCG_CARD_PSYCHIC_ENERGY : TCG_CARD_GRASS_ENERGY;
    for (i = 42; i < 50; i++)
        cards[i] = TCG_CARD_FIGHTING_ENERGY;
    for (i = 50; i < 52; i++)
        cards[i] = TCG_CARD_MACHOP;
    cards[54] = TCG_CARD_DOUBLE_COLORLESS_ENERGY;
    cards[55] = TCG_CARD_POTION;
    cards[56] = TCG_CARD_BILL;
    cards[57] = TCG_CARD_DIGLETT;
    cards[58] = id;
    cards[59] = id;
}
static int Take(struct TcgDuel *g, unsigned who, unsigned id)
{
    struct TcgPlayer *p = &g->players[who];
    unsigned i;
    for (i = 0; i < p->deckSize; i++)
        if (p->cards[p->deck[i]] == id)
            return TcgPile_Remove(p->deck, &p->deckSize, i);
    return -1;
}
static void Put(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    int c = Take(g, who, id);
    assert(c >= 0);
    f->stack[f->stackSize++] = (uint8_t)c;
}
static void Attach(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id, unsigned count)
{
    struct TcgFieldCard *f = &g->players[who].field[slot];
    while (count--)
    {
        int c = Take(g, who, id);
        if (c >= 0)
            f->attached[f->attachedSize++] = (uint8_t)c;
    }
}
static void Stage(struct TcgDuel *g, unsigned id, unsigned attack, uint32_t seed)
{
    static const unsigned energy[TCG_TYPES] = {TCG_CARD_FIRE_ENERGY, TCG_CARD_GRASS_ENERGY, TCG_CARD_LIGHTNING_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_FIGHTING_ENERGY, TCG_CARD_PSYCHIC_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_WATER_ENERGY, TCG_CARD_WATER_ENERGY};
    uint16_t a[60], b[60];
    unsigned t, who;
    Deck(a, id);
    Deck(b, TCG_CARD_DIGLETT);
    assert(TcgDuel_Init(g, a, b, seed, 6));
    for (who = 0; who < 2; who++)
    {
        while (g->players[who].handSize)
            g->players[who].deck[g->players[who].deckSize++] = g->players[who].hand[--g->players[who].handSize];
        while (g->players[who].prizeSize)
            g->players[who].deck[g->players[who].deckSize++] = g->players[who].prizes[--g->players[who].prizeSize];
    }
    g->phase = TCG_TURN;
    g->current = 0;
    g->turn = 3;
    g->players[0].turnsTaken = g->players[1].turnsTaken = 2;
    g->players[0].setupDone = g->players[1].setupDone = 1;
    Put(g, 0, 0, id);
    for (t = 0; t < TCG_TYPES; t++)
        Attach(g, 0, 0, energy[t], gTcgCards[id].attacks[attack].cost[t]);
    /* (more than it needs: Water Gun and the like, Fireball) */
    Attach(g, 0, 0, TCG_CARD_WATER_ENERGY, 2);
    Attach(g, 0, 0, TCG_CARD_FIRE_ENERGY, 1);
    Attach(g, 0, 0, TCG_CARD_FIGHTING_ENERGY, 2);
    Put(g, 0, 1, TCG_CARD_MACHOP);
    Put(g, 0, 2, TCG_CARD_DIGLETT);
    Put(g, 1, 0, TCG_CARD_MACHOP);
    Attach(g, 1, 0, TCG_CARD_WATER_ENERGY, 2);
    Attach(g, 1, 0, TCG_CARD_PSYCHIC_ENERGY, 1);
    Put(g, 1, 1, TCG_CARD_MACHOP);
    Put(g, 1, 2, TCG_CARD_DIGLETT);
    Attach(g, 1, 2, TCG_CARD_WATER_ENERGY, 1);
    /* a few cards in both hands */
    for (who = 0; who < 2; who++)
    {
        int c;
        struct TcgPlayer *p = &g->players[who];
        if ((c = Take(g, who, TCG_CARD_POTION)) >= 0)
            p->hand[p->handSize++] = (uint8_t)c;
        if ((c = Take(g, who, TCG_CARD_GRASS_ENERGY)) >= 0)
            p->hand[p->handSize++] = (uint8_t)c;
    }
    for (who = 0; who < 2; who++)
        while (g->players[who].prizeSize < 6)
            g->players[who].prizes[g->players[who].prizeSize++] = g->players[who].deck[--g->players[who].deckSize];
    g->players[0].field[0].damage = 20;
    g->players[0].field[0].food = (uint8_t)(seed % 3);
    if (gTcgCards[id].attacks[attack].effect && TcgDuel_SleepAttack(&gTcgCards[id].attacks[attack]))
        g->players[0].field[0].condition = TCG_ASLEEP;
    if (seed % 4 == 0)
        g->players[1].field[0].poison = 10;
}
/* A deck of finished regional cards of one type (and colorless ones): 4 of each Basic and its line, Energy after */
static int TypeDeck(uint16_t out[60], unsigned type, unsigned skip)
{
    static const unsigned energy[TCG_TYPES] = {TCG_CARD_FIRE_ENERGY, TCG_CARD_GRASS_ENERGY, TCG_CARD_LIGHTNING_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_FIGHTING_ENERGY, TCG_CARD_PSYCHIC_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_WATER_ENERGY, TCG_CARD_WATER_ENERGY};
    unsigned id, n = 0, basics = 0, seen = 0;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT && n < 32; id++)
    {
        const struct TcgCard *c = Tcg_Card(id), *from;
        unsigned k;
        if (!c || c->kind != TCG_POKEMON || (c->type != type && c->type != TCG_COLORLESS))
            continue;
        if (c->stage)
        {
            /* (only with its Basic: the one before in the deck) */
            for (k = 0, from = NULL; k < n && !from; k++)
                if (!strcmp(Tcg_Card(out[k])->name, c->evolvesFrom))
                    from = Tcg_Card(out[k]);
            if (!from)
                continue;
        }
        if (seen++ < skip)
            continue;
        for (k = 0; k < (c->stage ? 2u : 3u) && n < 32; k++)
            out[n++] = (uint16_t)id;
        basics += !c->stage;
    }
    if (!basics)
        return 0;
    while (n < 60)
        out[n++] = (uint16_t)energy[type < 6 ? type : 3];
    return 1;
}
int main(void)
{
    struct TcgDuel g;
    unsigned id, attack, used = 0, ok = 0, steps = 0, ready = 0;
    uint32_t seed;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        if (!c)
            continue;
        ready++;
        if (c->kind != TCG_POKEMON)
            continue;
        for (attack = 0; attack < TCG_ATTACKS; attack++)
        {
            const struct TcgAttack *at = &c->attacks[attack];
            if (!at->name[0] || at->category == TCG_POKEMON_POWER)
                continue;
            if (!TcgDuel_EffectSupported(at->effect))
            {
                printf("FAIL %s's %s: %s is not supported (a finished card)\n", c->name, at->name, at->effect);
                return 1;
            }
            used++;
            for (seed = 1; seed <= 12; seed++)
            {
                struct TcgAction a;
                int required, result, k;
                unsigned choice;
                Stage(&g, id, attack, seed);
                memset(&a, 0, sizeof(a));
                a.type = TCG_ATTACK;
                a.attack = (uint8_t)attack;
                choice = TcgDuel_AttackChoice(at, &required);
                if (choice == 1)
                    a.target = 1;
                if (choice == 2)
                    a.target = 2;
                TcgDuel_AiAttackChoices(&g, &a);
                if (TcgDuel_IsMetronome(at) && !a.copy)
                    a.copy = 1;
                if (!TcgDuel_CanAttack(&g, 0, attack) || !TcgDuel_CanAttackNow(&g, 0, attack))
                    continue;
                result = TcgDuel_Apply(&g, &a);
                if (result != TCG_OK)
                {
                    printf("FAIL %s's %s (%s, seed %u): %d %s\n", c->name, at->name, at->effect, seed, result, g.error);
                    return 1;
                }
                ok++;
                assert(TcgDuel_Conserved(&g));
                for (k = 0; k < 16 && g.phase != TCG_FINISHED; k++)
                {
                    struct TcgAction next;
                    memset(&next, 0, sizeof(next));
                    if (!TcgDuel_AiAction(&g, &next))
                        break;
                    result = TcgDuel_Apply(&g, &next);
                    if (result != TCG_OK)
                    {
                        printf("FAIL after %s's %s (seed %u, step %d, action %u): %s\n", c->name, at->name, seed, k,
                               next.type, g.error);
                        return 1;
                    }
                    assert(TcgDuel_Conserved(&g));
                    steps++;
                }
            }
        }
    }
    printf("PASS %u finished regional cards: %u attacks used %u times, then %u AI actions after them\n", ready, used, ok,
           steps);
    /* Decks of them, each type against the others, to the end */
    {
        unsigned t, u, games = 0, finished = 0, actions = 0, skip;
        for (skip = 0; skip < 3; skip++)
            for (t = 0; t < 6; t++)
                for (u = 0; u < 6; u++)
                {
                    uint16_t a[60], b[60];
                    unsigned k;
                    if (t == u || !TypeDeck(a, t, skip) || !TypeDeck(b, u, skip * 2))
                        continue;
                    if (!TcgDuel_Init(&g, a, b, t * 7 + u * 3 + skip + 1, 4))
                        continue; /* (too few Basic Pokemon of a type yet) */
                    games++;
                    for (k = 0; k < 3000 && g.phase != TCG_FINISHED; k++)
                    {
                        struct TcgAction next;
                        int result;
                        memset(&next, 0, sizeof(next));
                        if (!TcgDuel_AiAction(&g, &next))
                        {
                            printf("FAIL type %u vs %u: the AI has no action (phase %u)\n", t, u, g.phase);
                            return 1;
                        }
                        result = TcgDuel_Apply(&g, &next);
                        if (result != TCG_OK)
                        {
                            printf("FAIL type %u vs %u: action %u refused: %s\n", t, u, next.type, g.error);
                            return 1;
                        }
                        assert(TcgDuel_Conserved(&g));
                        actions++;
                    }
                    finished += g.phase == TCG_FINISHED;
                }
        printf("PASS %u AI duels with finished regional cards, %u finished, %u actions\n", games, finished, actions);
        if (finished * 10 < games * 9)
            return 1;
    }
    return 0;
}
