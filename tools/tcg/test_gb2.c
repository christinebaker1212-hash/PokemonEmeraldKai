/* Pokemon Card GB2's cards (ids past 228): every attack used on a staged board, many times with different
 * coins, with the choices the AI makes; then the duel goes on with the AI for a while. Each must be legal when
 * its block says so, keep every card where it can be counted, and never crash. Then a few by their numbers. */
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
                                               TCG_CARD_WATER_ENERGY};
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
int main(void)
{
    struct TcgDuel g;
    unsigned id, attack, used = 0, ok = 0, steps = 0;
    uint32_t seed;
    for (id = 229; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = &gTcgCards[id];
        if (c->kind != TCG_POKEMON)
            continue;
        for (attack = 0; attack < 2; attack++)
        {
            const struct TcgAttack *at = &c->attacks[attack];
            if (!at->name[0] || at->category == TCG_POKEMON_POWER)
                continue;
            if (!TcgDuel_EffectSupported(at->effect))
            {
                printf("FAIL %s's %s: %s is not supported\n", c->name, at->name, at->effect);
                return 1;
            }
            used++;
            for (seed = 1; seed <= 40; seed++)
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
                    printf("FAIL %s's %s (seed %u): %d %s\n", c->name, at->name, seed, result, g.error);
                    return 1;
                }
                ok++;
                assert(TcgDuel_Conserved(&g));
                /* the duel goes on */
                for (k = 0; k < 24 && g.phase != TCG_FINISHED; k++)
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
    printf("PASS %u Pokemon Card GB2 attacks used %u times, then %u AI actions after them\n", used, ok, steps);
    return 0;
}
