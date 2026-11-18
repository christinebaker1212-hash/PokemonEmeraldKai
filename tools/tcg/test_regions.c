/* The regional sets' finished cards (docs/TCG_REGIONS.md): every attack used on a staged board with different
 * coins and the AI's choices, the duel going on after; then decks of them in AI duels to the end. Every action legal,
 * every card kept where it can be counted. */
#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include "tcg/flow.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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
    /* (the only Darkness / Metal Energy: special) */
    for (i = 0; i < 8; i++)
        cards[i] = i < 4 ? TCG_CARD_NEO1_104 : TCG_CARD_NEO1_19;
    for (i = 50; i < 52; i++)
        cards[i] = TCG_CARD_MACHOP;
    cards[54] = TCG_CARD_DOUBLE_COLORLESS_ENERGY;
    cards[55] = TCG_CARD_POTION;
    cards[56] = TCG_CARD_BILL;
    cards[57] = TCG_CARD_DIGLETT;
    cards[58] = id;
    cards[59] = id;
    if (Tcg_Card(id) && (Tcg_Card(id)->flags & (TCG_CF_SHINING | TCG_CF_CRYSTAL)))
        cards[59] = TCG_CARD_POTION; /* (one Shining / Crystal Pokemon a deck) */
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
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_NEO1_104, TCG_CARD_NEO1_19};
    uint16_t a[60], b[60];
    unsigned t, who;
    Deck(a, id);
    Deck(b, TCG_CARD_DIGLETT);
    if (!TcgDuel_Init(g, a, b, seed, 6))
    {
        printf("FAIL a deck with two of %s (%u) was refused\n", Tcg_Card(id) ? Tcg_Card(id)->name : "?", id);
        exit(1);
    }
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
    unsigned id, n = 0, basics = 0, seen = 0, k;
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
        {
            /* (4 of a name at most: several sets print the same Pokemon) */
            unsigned same = 0, j;
            for (j = 0; j < n; j++)
                same += !strcmp(Tcg_Card(out[j])->name, c->name);
            for (k = 0; k < (c->stage ? 2u : 3u) && n < 32 && same < 4; k++, same++)
                out[n++] = (uint16_t)id;
        }
        basics += !c->stage;
    }
    if (!basics)
        return 0;
    /* a dozen finished regional Trainers, a different run for each deck */
    {
        unsigned trainers[256], nt = 0, t;
        for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT && nt < 256; id++)
            if (Tcg_Card(id) && Tcg_Card(id)->kind == TCG_TRAINER)
                trainers[nt++] = id;
        for (t = 0; nt && t < 12 && n < 44; t++)
            out[n++] = (uint16_t)trainers[(type * 13 + skip * 5 + t * 7) % nt];
    }
    /* (Darkness / Metal: four of the special Energy, the rest Colorless-paying basic Energy) */
    if (type == TCG_DARKNESS || type == TCG_METAL)
        for (k = 0; k < 4 && n < 60; k++, n++)
            out[n] = (uint16_t)(type == TCG_DARKNESS ? (k % 2 ? TCG_CARD_NEO1_104 : TCG_CARD_ECARD1_158)
                                                     : (k % 2 ? TCG_CARD_NEO1_19 : TCG_CARD_ECARD2_143));
    while (n < 60)
        out[n++] = (uint16_t)energy[type < 6 ? type : 3];
    return 1;
}
/* Darkness Energy (10 more, a counter at the end of the turn on a Pokemon that isn't Dark), Metal Energy (10 less to
 * it), Boost Energy (only on an Evolved Pokemon, then no retreat) */
static int LowKick(unsigned mine, unsigned theirs, unsigned *dealt, unsigned *took)
{
    struct TcgDuel g;
    struct TcgAction a;
    Stage(&g, TCG_CARD_MACHOP, 0, 7);
    g.players[1].field[0].poison = 0;
    if (mine)
        Attach(&g, 0, 0, mine, 1);
    if (theirs)
        Attach(&g, 1, 0, theirs, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Low Kick: %s\n", g.error), 1;
    *dealt = g.players[1].field[0].damage;
    *took = g.players[0].field[0].damage;
    return 0;
}
/* The Baby rules: a Basic Pikachu goes on a Pichu (any printing of either); attacking a Baby, tails ends the turn */
static int Babies(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, attacked = 0, stopped = 0, baby = 0, id;
    /* (a finished Baby: only those play) */
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT && !baby; id++)
        if (Tcg_Card(id) && (Tcg_Card(id)->flags & TCG_CF_BABY))
            baby = id;
    if (!TcgDuel_EvolvesFrom(Tcg_CardAny(TCG_CARD_PIKACHU_LV12), Tcg_CardAny(TCG_CARD_NEO1_12)) ||
        !TcgDuel_EvolvesFrom(Tcg_CardAny(TCG_CARD_PIKACHU_LV12), Tcg_CardAny(TCG_CARD_BASEP_35)) ||
        TcgDuel_EvolvesFrom(Tcg_CardAny(TCG_CARD_PIKACHU_LV12), Tcg_CardAny(TCG_CARD_NEO1_20)))
        return printf("FAIL Pikachu evolves from Pichu (only)\n"), 1;
    if (!baby)
        return printf("PASS Baby Pokemon: Pikachu on Pichu (no finished Baby to attack yet)\n"), 0;
    for (seed = 1; seed <= 20; seed++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, seed);
        {
            /* their Active becomes a Pichu (the card in its place) */
            struct TcgPlayer *op = &g.players[1];
            op->cards[op->field[0].stack[0]] = (uint16_t)baby;
            op->field[0].damage = 0;
        }
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL attacking a Baby: %s\n", g.error), 1;
        attacked++;
        stopped += g.players[1].field[0].damage == 0;
    }
    if (!stopped || stopped == attacked)
        return printf("FAIL the Baby's coin: %u of %u attacks stopped\n", stopped, attacked), 1;
    printf("PASS Baby Pokemon: Pikachu on Pichu, %u of %u attacks on a Baby stopped by tails\n", stopped, attacked);
    return 0;
}
/* Effect scripts' turn-spanning mechanics: Focus Energy's boost next turn (Machoke's Mega Punch 80), "can't become
 * affected by a Special Condition", "pays Colorless more to retreat" */
/* Jaw Clamp: the Defending Pokemon can't retreat and isn't switched out while Croconaw stays Active, through their
 * turn; Earthdrill only after Lie Low the turn before */
static int Anchors(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    if (!Tcg_Card(TCG_CARD_NEO1_31) || !Tcg_Card(TCG_CARD_GYM2_22))
        return printf("PASS anchors (Croconaw / Brock's Dugtrio not finished)\n"), 0;
    Stage(&g, TCG_CARD_NEO1_31, 1, 3);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Jaw Clamp: %s\n", g.error), 1;
    if (g.players[1].field[0].stackSize && (!TcgDuel_Anchored(&g, 1) || !TcgDuel_Stuck(&g, 1)))
        return printf("FAIL Jaw Clamp: the Defending Pokemon isn't held\n"), 1;
    g.players[0].field[0].stack[g.players[0].field[0].stackSize - 1] ^= 1; /* (another card Active: it lets go) */
    if (TcgDuel_Anchored(&g, 1))
        return printf("FAIL Jaw Clamp holds with Croconaw gone\n"), 1;
    Stage(&g, TCG_CARD_GYM2_22, 1, 3);
    if (TcgDuel_CanAttack(&g, 0, 1) && TcgDuel_CanAttackNow(&g, 0, 1))
        return printf("FAIL Earthdrill usable without Lie Low\n"), 1;
    g.players[0].field[0].lastAttack = 1;
    g.players[0].field[0].lastAttackTurn = (uint16_t)(g.turn - 2);
    if (!TcgDuel_CanAttack(&g, 0, 1) || !TcgDuel_CanAttackNow(&g, 0, 1))
        return printf("FAIL Earthdrill after Lie Low\n"), 1;
    /* Fire Wall: an attack that damages Rocket's Moltres next turn takes 10 back; Endure leaves 10 HP */
    if (Tcg_Card(TCG_CARD_GYM1_12))
    {
        unsigned before;
        Stage(&g, TCG_CARD_GYM1_12, 0, 3);
        g.players[0].field[0].reactKind = TCG_REACT_FIRE_WALL;
        g.players[0].field[0].reactUntil = (uint16_t)(g.turn + 1);
        g.current = 1;
        g.turn++;
        before = g.players[1].field[0].damage;
        TcgPower_Damaged(&g, 0, 0, 20);
        if (g.players[1].field[0].damage < before + 10)
            return printf("FAIL Fire Wall: the attacker took %u\n", g.players[1].field[0].damage - before), 1;
        g.players[0].field[0].reactKind = TCG_REACT_ENDURE;
        g.players[0].field[0].damage = 200;
        TcgPower_Damaged(&g, 0, 0, 200);
        if (g.players[0].field[0].damage != Tcg_Card(TCG_CARD_GYM1_12)->hp - 10u)
            return printf("FAIL Endure: %u damage\n", g.players[0].field[0].damage), 1;
    }
    /* maximum HP: Rocket's Hideout gives a Dark Pokemon 20 more; Gaseous Form 10 more a Psychic Energy card */
    if (Tcg_Card(TCG_CARD_NEO3_63) && Tcg_Card(TCG_CARD_NEO4_2) && Tcg_Card(TCG_CARD_GYM2_97))
    {
        int c;
        Stage(&g, TCG_CARD_NEO4_2, 0, 5);
        c = Take(&g, 0, TCG_CARD_GRASS_ENERGY);
        g.players[0].cards[c] = TCG_CARD_NEO3_63;
        g.players[0].stadium = (uint8_t)(c + 1);
        if (TcgDuel_MaxHp(&g, 0, 0) != Tcg_Card(TCG_CARD_NEO4_2)->hp + 20u)
            return printf("FAIL Rocket's Hideout: %u HP\n", TcgDuel_MaxHp(&g, 0, 0)), 1;
        Stage(&g, TCG_CARD_GYM2_97, 0, 5);
        Attach(&g, 0, 0, TCG_CARD_PSYCHIC_ENERGY, 2);
        if (TcgDuel_MaxHp(&g, 0, 0) < Tcg_Card(TCG_CARD_GYM2_97)->hp + 20u)
            return printf("FAIL Gaseous Form: %u HP\n", TcgDuel_MaxHp(&g, 0, 0)), 1;
    }
    printf("PASS anchors: Jaw Clamp holds while Croconaw is Active, Earthdrill only after Lie Low, Fire Wall, Endure, maximum HP\n");
    return 0;
}
/* Burst's two coins, Terrorize, Reflect Shield, Shadow Images, Spikes, Crystal Body, Mirror Coat */
static int Guards(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, seen = 0;
    if (!Tcg_Card(TCG_CARD_NEO2_61) || !Tcg_Card(TCG_CARD_NEO3_38) || !Tcg_Card(TCG_CARD_NEO4_109) ||
        !Tcg_Card(TCG_CARD_GYM1_13) || !Tcg_Card(TCG_CARD_NEO2_2) || !Tcg_Card(TCG_CARD_NEO3_14) ||
        !Tcg_Card(TCG_CARD_ECARD3_45))
        return printf("FAIL guards: a card isn't finished\n"), 1;
    /* Burst: (heads, heads) 40 to itself and 10 to each Bench; (tails, heads) the damage only; second tails nothing */
    for (seed = 1; seed < 60; seed++)
    {
        unsigned foe, bench, prizes;
        Stage(&g, TCG_CARD_NEO2_61, 0, seed);
        g.players[0].field[0].damage = 0;
        g.players[1].field[0].damage = 0;
        g.players[1].field[0].poison = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Burst: %s\n", g.error), 1;
        foe = g.players[1].field[0].damage, bench = g.players[1].field[1].damage, prizes = g.players[1].prizeSize;
        if (!foe && !bench && prizes == 6)
            seen |= 1;
        else if (foe && bench == 10 && prizes < 6)
            seen |= 2;
        else if (foe && !bench && prizes == 6)
            seen |= 4;
        else
            return printf("FAIL Burst (seed %u): %u to the Defending, %u to the Bench, %u Prizes\n", seed, foe, bench, prizes), 1;
    }
    if (seen != 7)
        return printf("FAIL Burst: outcomes seen %u\n", seen), 1;
    /* Terrorize: the Defending Basic's chosen attack can't be used next turn */
    Stage(&g, TCG_CARD_NEO3_38, 0, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].disabled != 1)
        return printf("FAIL Terrorize: %s (disabled %u)\n", g.error, g.players[1].field[0].disabled), 1;
    /* Reflect Shield (heads: no damage, 20 back) and Shadow Images (tails: no damage; kept until it's damaged) */
    for (seen = 0, seed = 1; seed < 40; seed++)
    {
        unsigned k;
        for (k = 0; k < 2; k++)
        {
            unsigned mine, theirs;
            Stage(&g, k ? TCG_CARD_GYM1_13 : TCG_CARD_NEO4_109, 0, seed);
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            if (TcgDuel_Apply(&g, &a) != TCG_OK)
                return printf("FAIL %s: %s\n", k ? "Shadow Images" : "Reflect Shield", g.error), 1;
            if (k ? g.players[0].field[0].guardKeep != TCG_GUARD_TAILS : g.players[0].field[0].guard != TCG_GUARD_BLINK_HIT)
                return printf("FAIL %s: no guard\n", k ? "Shadow Images" : "Reflect Shield"), 1;
            if (k) /* (Rocket's Scyther resists Low Kick: the guard on a Pokemon that doesn't) */
                g.players[0].cards[g.players[0].field[0].stack[0]] = TCG_CARD_NEO4_109;
            /* their Machop's Low Kick */
            g.current = 1, g.phase = TCG_TURN, g.needsTurnStart = 0;
            g.players[1].field[0].condition = TCG_HEALTHY;
            g.players[1].field[0].poison = g.players[1].field[0].burned = 0;
            Attach(&g, 1, 0, TCG_CARD_FIGHTING_ENERGY, 1);
            mine = g.players[0].field[0].damage, theirs = g.players[1].field[0].damage;
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            if (TcgDuel_Apply(&g, &a) != TCG_OK)
                return printf("FAIL Low Kick into %s: %s\n", k ? "Shadow Images" : "Reflect Shield", g.error), 1;
            if (g.players[0].field[0].damage == mine)
            {
                if (!k && g.players[1].field[0].damage != theirs + 20)
                    return printf("FAIL Reflect Shield: no 20 back (%u, %u)\n", theirs, g.players[1].field[0].damage), 1;
                if (k && g.players[0].field[0].guardKeep != TCG_GUARD_TAILS)
                    return printf("FAIL Shadow Images ended without damage\n"), 1;
                seen |= 1u << (k * 2);
            }
            else
            {
                if (k && g.players[0].field[0].guardKeep)
                    return printf("FAIL Shadow Images lasts after damage\n"), 1;
                seen |= 2u << (k * 2);
            }
        }
    }
    if (seen != 15)
        return printf("FAIL Reflect Shield / Shadow Images: outcomes seen %u\n", seen), 1;
    /* Spikes: their Benched Pokemon that becomes Active on their turn takes 10 */
    Stage(&g, TCG_CARD_NEO2_2, 0, 1);
    g.current = 1, g.phase = TCG_TURN, g.needsTurnStart = 0;
    g.players[1].field[1].damage = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_RETREAT;
    a.slot = 1;
    a.selections[0] = g.players[1].field[0].attached[0];
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].damage != 10)
        return printf("FAIL Spikes: %s (%u damage)\n", g.error, g.players[1].field[0].damage), 1;
    /* Crystal Body: Poison Fang's damage but not its Poison; Mirror Coat: Poisoned, and the attacker too */
    for (seed = 0; seed < 3; seed++)
    {
        static const unsigned ids[3] = {TCG_CARD_MACHOP, TCG_CARD_NEO3_14, TCG_CARD_ECARD3_45};
        Stage(&g, TCG_CARD_ARBOK, 1, 1);
        g.players[1].cards[g.players[1].field[0].stack[0]] = (uint16_t)ids[seed];
        g.players[1].field[0].poison = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Poison Fang: %s\n", g.error), 1;
        if (!g.players[1].field[0].damage || !g.players[1].field[0].poison != (seed == 1) ||
            !g.players[0].field[0].poison != (seed != 2))
            return printf("FAIL %s: damage %u, Poisoned %u, the attacker %u\n", Tcg_Card(ids[seed])->name,
                          g.players[1].field[0].damage, g.players[1].field[0].poison, g.players[0].field[0].poison), 1;
    }
    printf("PASS Burst, Terrorize, Reflect Shield, Shadow Images, Spikes, Crystal Body, Mirror Coat\n");
    return 0;
}
static int Relabel(struct TcgDuel *g, unsigned who, unsigned id);
/* Soak Up, Electromagnetic Power, [Bear], Magma Pool, Ancient Wind, [Vanish] */
static int MovePowers(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned before, k, seed;
    int c;
    if (!Tcg_Card(TCG_CARD_GYM2_38) || !Tcg_Card(TCG_CARD_NEO3_10) || !Tcg_Card(TCG_CARD_NEO3_39) ||
        !Tcg_Card(TCG_CARD_NEO3_33) || !Tcg_Card(TCG_CARD_ECARD3_1) || !Tcg_Card(TCG_CARD_NEO4_89))
        return printf("FAIL move powers: a card isn't finished\n"), 1;
    /* Soak Up: 2 Grass Energy from the Bench onto it */
    Stage(&g, TCG_CARD_GYM2_38, 0, 1);
    Attach(&g, 0, 1, TCG_CARD_GRASS_ENERGY, 2);
    before = g.players[0].field[0].attachedSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.selections[0] = g.players[0].field[1].attached[0], a.selections[1] = g.players[0].field[1].attached[1];
    a.selectionCount = 2;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].attachedSize != before + 2 || g.players[0].field[1].attachedSize)
        return printf("FAIL Soak Up: %s\n", g.error), 1;
    /* Electromagnetic Power: to another Magneton, not to Diglett; as often as you like */
    Stage(&g, TCG_CARD_NEO3_10, 0, 1);
    g.players[0].cards[g.players[0].field[1].stack[0]] = TCG_CARD_NEO3_10;
    before = g.players[0].field[0].attachedSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.selections[0] = g.players[0].field[0].attached[0];
    a.selectionCount = 1;
    a.target = 2;
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Electromagnetic Power onto Diglett\n"), 1;
    for (k = 0; k < 2; k++)
    {
        a.selections[0] = g.players[0].field[0].attached[0];
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Electromagnetic Power (%u): %s\n", k, g.error), 1;
    }
    if (g.players[0].field[0].attachedSize != before - 2 || g.players[0].field[1].attachedSize != 2)
        return printf("FAIL Electromagnetic Power: %u / %u\n", g.players[0].field[0].attachedSize, g.players[0].field[1].attachedSize), 1;
    /* [Bear]: a counter from another Unown onto it */
    Stage(&g, TCG_CARD_NEO3_39, 0, 1);
    g.players[0].cards[g.players[0].field[1].stack[0]] = TCG_CARD_NEO3_39;
    g.players[0].field[0].damage = 0, g.players[0].field[1].damage = 20;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.target = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].damage != 10 || g.players[0].field[1].damage != 10)
        return printf("FAIL [Bear]: %s\n", g.error), 1;
    /* Magma Pool: retreating, a Fire Energy follows onto the new Active */
    Stage(&g, TCG_CARD_NEO3_33, 0, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_RETREAT;
    a.slot = 1;
    for (k = 0; k < g.players[0].field[0].attachedSize && a.selectionCount < Tcg_Card(TCG_CARD_NEO3_33)->retreat; k++)
        if (g.players[0].cards[g.players[0].field[0].attached[k]] != TCG_CARD_FIRE_ENERGY)
            a.selections[a.selectionCount++] = g.players[0].field[0].attached[k];
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].attachedSize != 1 ||
        g.players[0].cards[g.players[0].field[0].attached[0]] != TCG_CARD_FIRE_ENERGY)
        return printf("FAIL Magma Pool: %s (%u)\n", g.error, g.players[0].field[0].attachedSize), 1;
    /* Ancient Wind: their Wobbuffet's Mirror Coat ignored this turn */
    Stage(&g, TCG_CARD_ECARD3_1, 0, 1);
    g.players[1].cards[g.players[1].field[0].stack[0]] = TCG_CARD_ECARD3_45;
    if (TcgPower_Working(&g, 1, 0) != TCG_POWER_R_MIRROR_COAT)
        return printf("FAIL Mirror Coat isn't working\n"), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || TcgPower_Working(&g, 1, 0))
        return printf("FAIL Ancient Wind: %s\n", g.error), 1;
    /* [Vanish]: heads, the Unown chosen back to the hand */
    for (seed = 1, k = 0; seed < 20 && !k; seed++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, seed);
        g.players[0].cards[g.players[0].field[1].stack[0]] = TCG_CARD_NEO3_39;
        Attach(&g, 0, 1, TCG_CARD_GRASS_ENERGY, 1);
        c = Relabel(&g, 0, TCG_CARD_NEO4_89);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
        before = g.players[0].handSize;
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_BASIC;
        a.hand = (uint8_t)(before - 1);
        a.slot = 3;
        a.target = 1;
        a.option = TCG_PLAY_CHOSEN;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL [Vanish]: %s\n", g.error), 1;
        if (!g.players[0].field[1].stackSize)
        {
            if (g.players[0].handSize != before)
                return printf("FAIL [Vanish]: hand %u\n", g.players[0].handSize), 1;
            k = 1;
        }
    }
    if (!k)
        return printf("FAIL [Vanish] never returned the Unown\n"), 1;
    printf("PASS Soak Up, Electromagnetic Power, [Bear], Magma Pool, Ancient Wind, [Vanish]\n");
    return 0;
}
static void Give(struct TcgDuel *g, unsigned who, unsigned id, int toDiscard);
/* Terraforming, Strange Tentacles, Dark Moon, Aqua Trick */
static int LookPowers(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned k, seed, seen = 0;
    if (!Tcg_Card(TCG_CARD_ECARD1_16) || !Tcg_Card(TCG_CARD_ECARD2_38) || !Tcg_Card(TCG_CARD_ECARD2_41) ||
        !Tcg_Card(TCG_CARD_ECARD3_33))
        return printf("FAIL look powers: a card isn't finished\n"), 1;
    /* Terraforming: the top 4 reversed */
    Stage(&g, TCG_CARD_ECARD1_16, 0, 1);
    g.players[0].field[0].poison = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    for (k = 0; k < 4; k++)
        a.selections[k] = g.players[0].deck[g.players[0].deckSize - 4 + k];
    a.selectionCount = 4;
    {
        uint8_t was[4];
        for (k = 0; k < 4; k++)
            was[k] = g.players[0].deck[g.players[0].deckSize - 1 - k];
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Terraforming: %s\n", g.error), 1;
        for (k = 0; k < 4; k++)
            if (g.players[0].deck[g.players[0].deckSize - 4 + k] != was[k])
                return printf("FAIL Terraforming: card %u out of order\n", k), 1;
    }
    /* Strange Tentacles: an Energy from their discard pile onto the Defending Pokemon (it has fewer) */
    Stage(&g, TCG_CARD_ECARD2_38, 0, 1);
    Give(&g, 1, TCG_CARD_GRASS_ENERGY, 1);
    k = g.players[1].field[0].attachedSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.selections[0] = g.players[1].discard[g.players[1].discardSize - 1];
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].attachedSize != k + 1)
        return printf("FAIL Strange Tentacles: %s\n", g.error), 1;
    /* Dark Moon: one card from their hand per Darkness Energy, they draw as many */
    Stage(&g, TCG_CARD_ECARD2_41, 0, 1);
    Attach(&g, 0, 0, TCG_CARD_NEO1_104, 1);
    k = g.players[1].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.selections[0] = g.players[1].hand[0], a.selections[1] = g.players[1].hand[1];
    a.selectionCount = 2;
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Dark Moon: 2 cards for 1 Darkness Energy\n"), 1;
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].handSize != k)
        return printf("FAIL Dark Moon: %s (%u cards)\n", g.error, g.players[1].handSize), 1;
    /* Aqua Trick: heads, the Defending's Energy card picked to their Benched one chosen */
    for (seed = 1; seed < 30; seed++)
    {
        unsigned before;
        Stage(&g, TCG_CARD_ECARD3_33, 2, seed); /* (attack 0 is its Poke-Body) */
        g.players[1].field[0].poison = 0;
        before = g.players[1].field[1].attachedSize;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 2;
        a.target = 1;
        a.selections[0] = g.players[1].field[0].attached[2];
        a.selectionCount = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Aqua Trick: %s\n", g.error), 1;
        seen |= g.players[1].field[1].attachedSize == before + 1 ? 1 : 2;
    }
    if (seen != 3)
        return printf("FAIL Aqua Trick: outcomes %u\n", seen), 1;
    printf("PASS Terraforming, Strange Tentacles, Dark Moon, Aqua Trick\n");
    return 0;
}
/* Drive Off, Major Tsunami, Lightning Burst (their choice), Baton Pass, Bonfire, Roaring Flames, Messenger, Sleight of
 * Hand, Destructive Fire */
static int SwitchesAndFire(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned k, before, seed, seen = 0;
    int c;
    static const unsigned ids[] = {TCG_CARD_NEO4_12, TCG_CARD_NEO3_13, TCG_CARD_ECARD1_12, TCG_CARD_NEO1_39, TCG_CARD_GYM2_31,
                                   TCG_CARD_GYM2_2, TCG_CARD_GYM2_49, TCG_CARD_GYM1_94, TCG_CARD_NEO4_113};
    for (k = 0; k < sizeof(ids) / sizeof(ids[0]); k++)
        if (!Tcg_Card(ids[k]))
            return printf("FAIL switches and fire: card %u isn't finished\n", ids[k]), 1;
    /* Drive Off: they choose who comes in (a promotion of theirs), then the turn goes on */
    Stage(&g, TCG_CARD_NEO4_12, 1, 1);
    g.players[0].field[0].poison = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_PROMOTION || g.promotePlayer != 1)
        return printf("FAIL Drive Off: %s (phase %u)\n", g.error, g.phase), 1;
    before = g.players[1].field[2].stack[0];
    memset(&a, 0, sizeof(a));
    a.type = TCG_PROMOTE;
    a.slot = 2;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_TURN || g.current != 0 || g.players[1].field[0].stack[0] != before)
        return printf("FAIL Drive Off's promotion: %s\n", g.error), 1;
    /* Major Tsunami: theirs, and Feraligatr with the Benched one chosen */
    Stage(&g, TCG_CARD_ECARD1_12, 1, 1);
    before = g.players[0].field[2].stack[0];
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.target = 2;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_PROMOTION || g.players[0].field[0].stack[0] != before)
        return printf("FAIL Major Tsunami: %s\n", g.error), 1;
    /* Lightning Burst: a Lightning Energy from the hand onto Raikou */
    Stage(&g, TCG_CARD_NEO3_13, 1, 1);
    c = Relabel(&g, 0, TCG_CARD_LIGHTNING_ENERGY); /* (the deck's Lightning Energy are all on it) */
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACH;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.slot = 0;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_PROMOTION || g.promotePlayer != 1)
        return printf("FAIL Lightning Burst: %s\n", g.error), 1;
    /* Baton Pass: its Grass Energy to the Benched Grass Pokemon, which comes in */
    Stage(&g, TCG_CARD_NEO1_39, 0, 1);
    g.players[0].cards[g.players[0].field[2].stack[0]] = TCG_CARD_NEO1_39;
    before = g.players[0].field[2].stack[0];
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = 0;
    a.target = 1; /* (Machop: not Grass) */
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Baton Pass to a Machop\n"), 1;
    a.target = 2;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].stack[0] != before ||
        TcgDuel_Energy(&g, 0, 0, TCG_GRASS) < 2)
        return printf("FAIL Baton Pass: %s\n", g.error), 1;
    /* Bonfire: 10 a heads to each of theirs, a Fire Energy discarded a heads */
    for (seed = 1; seed < 12; seed++)
    {
        unsigned fire, heads;
        Stage(&g, TCG_CARD_GYM2_31, 1, seed);
        g.players[1].field[0].poison = 0;
        g.players[1].field[0].damage = g.players[1].field[1].damage = 0;
        fire = TcgDuel_Energy(&g, 0, 0, TCG_FIRE);
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Bonfire: %s\n", g.error), 1;
        heads = g.players[1].field[1].damage / 10;
        if (g.players[1].field[0].damage != heads * 10 || TcgDuel_Energy(&g, 0, 0, TCG_FIRE) != fire - heads)
            return printf("FAIL Bonfire (seed %u): %u / %u, Fire %u -> %u\n", seed, g.players[1].field[0].damage,
                          g.players[1].field[1].damage, fire, TcgDuel_Energy(&g, 0, 0, TCG_FIRE)), 1;
    }
    /* Roaring Flames: every Fire Energy discarded, 20 more for each */
    Stage(&g, TCG_CARD_GYM2_2, 0, 1);
    g.players[1].field[0].damage = 0;
    k = TcgDuel_Energy(&g, 0, 0, TCG_FIRE);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || TcgDuel_Energy(&g, 0, 0, TCG_FIRE))
        return printf("FAIL Roaring Flames: %s\n", g.error), 1;
    if (g.players[1].field[0].stackSize && g.players[1].field[0].damage && g.players[1].field[0].damage < 20 + 20 * k)
        return printf("FAIL Roaring Flames: %u damage for %u Fire\n", g.players[1].field[0].damage, k), 1;
    /* Messenger: a Pokemon card not named Koga's Pidgey, and Pidgey into the deck */
    Stage(&g, TCG_CARD_GYM2_49, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_MACHOP);
    g.players[0].deck[g.players[0].deckSize++] = (uint8_t)c; /* (back into the deck) */
    before = g.players[0].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.selections[0] = (uint8_t)c;
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].handSize != before + 1)
        return printf("FAIL Messenger: %s\n", g.error), 1;
    /* Sleight of Hand: 2 cards on top, 2 basic Energy into the hand (3 refused) */
    Stage(&g, TCG_CARD_GYM1_94, 0, 1);
    before = g.players[0].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.selections[0] = g.players[0].hand[0], a.selections[1] = g.players[0].hand[1];
    for (k = 0, a.selectionCount = 2; k < g.players[0].deckSize && a.selectionCount < 5; k++)
        if (g.players[0].cards[g.players[0].deck[k]] >= 1 && g.players[0].cards[g.players[0].deck[k]] <= 6)
            a.selections[a.selectionCount++] = g.players[0].deck[k];
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Sleight of Hand took 3 for 2\n"), 1;
    a.selectionCount = 4;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].handSize != before)
        return printf("FAIL Sleight of Hand: %s (%u)\n", g.error, g.players[0].handSize), 1;
    /* Destructive Fire: a Fire Energy of its and one of the Defending's a heads */
    for (seed = 1; seed < 20; seed++)
    {
        unsigned fire, theirs, lost;
        Stage(&g, TCG_CARD_NEO4_113, 1, seed);
        fire = TcgDuel_Energy(&g, 0, 0, TCG_FIRE), theirs = g.players[1].field[0].attachedSize;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Destructive Fire: %s\n", g.error), 1;
        lost = fire - TcgDuel_Energy(&g, 0, 0, TCG_FIRE);
        if (g.players[1].field[0].stackSize && theirs - g.players[1].field[0].attachedSize != (lost < theirs ? lost : theirs))
            return printf("FAIL Destructive Fire: %u heads, %u of theirs gone\n", lost, theirs - g.players[1].field[0].attachedSize), 1;
        seen |= lost ? 1 : 2;
    }
    if (seen != 3)
        return printf("FAIL Destructive Fire: outcomes %u\n", seen), 1;
    printf("PASS Drive Off, Major Tsunami, Lightning Burst, Baton Pass, Bonfire, Roaring Flames, Messenger, Sleight of Hand, Destructive Fire\n");
    return 0;
}
/* Borrowed attacks: Recall and Memory Berry (Machop's Low Kick from Machoke), Prehistoric Memory, Psylink, Mimic */
static int BorrowedAttacks(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    const struct TcgAttack *at;
    int c;
    unsigned k;
    if (!Tcg_Card(TCG_CARD_GYM1_116) || !Tcg_Card(TCG_CARD_ECARD2_128) || !Tcg_Card(TCG_CARD_GYM2_16) ||
        !Tcg_Card(TCG_CARD_NEO3_15) || !Tcg_Card(TCG_CARD_NEO3_26))
        return printf("FAIL borrowed attacks: a card isn't finished\n"), 1;
    /* (Machoke over a Machop) */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_MACHOKE);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    if (TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW))
        return printf("FAIL borrowed: Machoke has Low Kick without Recall\n"), 1;
    c = Relabel(&g, 0, TCG_CARD_GYM1_116);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Recall: %s\n", g.error), 1;
    at = TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_MACHOP)->attacks[0].name))
        return printf("FAIL Recall: borrowed %s\n", at ? at->name : "nothing"), 1;
    k = g.players[1].field[0].damage;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = TCG_ATTACK_BORROW;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].damage <= k)
        return printf("FAIL Recall's Low Kick: %s\n", g.error), 1;
    /* Memory Berry: while attached, and discarded after the attack */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_MACHOKE);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    c = Relabel(&g, 0, TCG_CARD_ECARD2_128);
    g.players[0].field[0].attached[g.players[0].field[0].attachedSize++] = (uint8_t)c;
    if (!TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW))
        return printf("FAIL Memory Berry: no borrowed attack\n"), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = TCG_ATTACK_BORROW;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || TcgDuel_AttachedCard(&g, 0, 0, TCG_CARD_ECARD2_128) >= 0)
        return printf("FAIL Memory Berry: %s (still attached after the attack)\n", g.error), 1;
    /* Prehistoric Memory: their Aerodactyl lets your Machoke use Low Kick too */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_MACHOKE);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    g.players[1].cards[g.players[1].field[1].stack[0]] = TCG_CARD_NEO3_15;
    if (!TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW))
        return printf("FAIL Prehistoric Memory\n"), 1;
    /* Psylink: a Benched Psychic Pokemon's attacks; Mimic: the Defending Pokemon's */
    Stage(&g, TCG_CARD_GYM2_16, 1, 1);
    g.players[0].field[0].condition = TCG_HEALTHY;
    if (TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW))
        return printf("FAIL Psylink with no Psychic Pokemon\n"), 1;
    g.players[0].cards[g.players[0].field[1].stack[0]] = TCG_CARD_KADABRA;
    at = TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_KADABRA)->attacks[at == &Tcg_Card(TCG_CARD_KADABRA)->attacks[0] ? 0 : 1].name))
        return printf("FAIL Psylink: %s\n", at ? at->name : "nothing"), 1;
    Stage(&g, TCG_CARD_NEO3_26, 1, 1);
    g.players[0].field[0].condition = TCG_HEALTHY;
    at = TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_MACHOP)->attacks[0].name))
        return printf("FAIL Mimic: %s\n", at ? at->name : "nothing"), 1;
    printf("PASS borrowed attacks: Recall, Memory Berry, Prehistoric Memory, Psylink, Mimic\n");
    return 0;
}
/* Genetic Memory (Seadra's attacks under Kingdra), Sketch (Low Kick last turn), Rocket's Minefield Gym */
static int Copies(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    const struct TcgAttack *at;
    unsigned seed, seen = 0;
    int c;
    if (!Tcg_Card(TCG_CARD_NEO3_19) || !Tcg_Card(TCG_CARD_NEO2_11) || !Tcg_Card(TCG_CARD_GYM2_119))
        return printf("FAIL copies: a card isn't finished\n"), 1;
    Stage(&g, TCG_CARD_NEO3_19, 0, 1);
    g.players[0].deck[g.players[0].deckSize++] = g.players[0].field[0].stack[0]; /* (rebuilt as Horsea, Seadra, Kingdra) */
    g.players[0].field[0].stackSize = 0;
    c = Relabel(&g, 0, TCG_CARD_HORSEA);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    c = Relabel(&g, 0, TCG_CARD_SEADRA);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    c = Relabel(&g, 0, TCG_CARD_NEO3_19);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    at = TcgDuel_MetronomeAttack(&g, 0, 1);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_SEADRA)->attacks[0].name))
        return printf("FAIL Genetic Memory: %s\n", at ? at->name : "nothing"), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.copy = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Genetic Memory's attack: %s\n", g.error), 1;
    /* Sketch: nothing to copy unless the Defending Pokemon attacked last turn */
    Stage(&g, TCG_CARD_NEO2_11, 0, 1);
    if (TcgDuel_MetronomeAttack(&g, 0, 1))
        return printf("FAIL Sketch copies with no attack last turn\n"), 1;
    g.players[1].field[0].lastAttack = 1;
    g.players[1].field[0].lastAttackTurn = (uint16_t)(g.turn - 1);
    at = TcgDuel_MetronomeAttack(&g, 0, 1);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_MACHOP)->attacks[0].name))
        return printf("FAIL Sketch: %s\n", at ? at->name : "nothing"), 1;
    /* Rocket's Minefield Gym: tails, 2 counters on the Basic benched from the hand */
    for (seed = 1; seed < 16; seed++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, seed);
        c = Relabel(&g, 0, TCG_CARD_GYM2_119);
        g.players[0].stadium = (uint8_t)(c + 1);
        c = Relabel(&g, 0, TCG_CARD_MACHOP);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_BASIC;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        a.slot = 3;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Minefield: %s\n", g.error), 1;
        seen |= g.players[0].field[3].damage == 20 ? 1 : !g.players[0].field[3].damage ? 2 : 4;
    }
    if (seen != 3)
        return printf("FAIL Rocket's Minefield Gym: outcomes %u\n", seen), 1;
    printf("PASS Genetic Memory, Sketch, Rocket's Minefield Gym\n");
    return 0;
}
/* Lock-on, Frog Song, Psymimic, Harvest Bounty */
static int PowersOfFour(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, k;
    int c;
    if (!Tcg_Card(TCG_CARD_NEO2_7) || !Tcg_Card(TCG_CARD_NEO2_8) || !Tcg_Card(TCG_CARD_ECARD1_1) || !Tcg_Card(TCG_CARD_ECARD1_30))
        return printf("FAIL lock-on and the rest: a card isn't finished\n"), 1;
    /* Lock-on, then Electric Bolt next turn: always Paralyzed */
    for (seed = 1; seed < 8; seed++)
    {
        unsigned turn;
        Stage(&g, TCG_CARD_NEO2_7, 0, seed);
        c = Relabel(&g, 0, TCG_CARD_LIGHTNING_ENERGY);
        g.players[0].field[0].attached[g.players[0].field[0].attachedSize++] = (uint8_t)c;
        g.players[1].field[0].poison = 0;
        turn = g.turn;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Lock-on: %s\n", g.error), 1;
        g.current = 0, g.phase = TCG_TURN, g.needsTurnStart = 0, g.turn = (uint16_t)(turn + 2);
        g.players[1].field[0].damage = 0, g.players[1].field[0].condition = TCG_HEALTHY;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Electric Bolt: %s\n", g.error), 1;
        if (g.players[1].field[0].stackSize && g.players[1].field[0].condition != TCG_PARALYZED)
            return printf("FAIL Lock-on (seed %u): not Paralyzed (%u damage)\n", seed, g.players[1].field[0].damage), 1;
    }
    /* Frog Song: Politoed and 3 more Poliwag in play, 40 more on a damaging Doubleslap */
    for (seed = 1; seed < 10; seed++)
    {
        unsigned d;
        Stage(&g, TCG_CARD_NEO2_8, 1, seed);
        g.players[0].cards[g.players[0].field[1].stack[0]] = TCG_CARD_POLIWAG;
        g.players[0].cards[g.players[0].field[2].stack[0]] = TCG_CARD_POLIWAG;
        g.players[1].cards[g.players[1].field[1].stack[0]] = TCG_CARD_POLIWAG;
        g.players[1].field[0].poison = 0, g.players[1].field[0].damage = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Frog Song: %s\n", g.error), 1;
        d = g.players[1].field[0].damage;
        if (g.players[1].field[0].stackSize && d != 0 && d != 80 && d != 120)
            return printf("FAIL Frog Song: %u damage\n", d), 1;
    }
    /* Psymimic: their Machop's Low Kick, paid with its own Energy */
    Stage(&g, TCG_CARD_ECARD1_1, 1, 1);
    {
        const struct TcgAttack *at = TcgDuel_AttackOf(&g, 0, TCG_ATTACK_BORROW);
        if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_MACHOP)->attacks[0].name))
            return printf("FAIL Psymimic: %s\n", at ? at->name : "nothing"), 1;
    }
    k = g.players[1].field[0].damage;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = TCG_ATTACK_BORROW;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].damage <= k)
        return printf("FAIL Psymimic's Low Kick: %s\n", g.error), 1;
    /* Harvest Bounty: a second Energy only onto the Active, after the first went there */
    Stage(&g, TCG_CARD_ECARD1_30, 1, 1);
    g.players[0].field[0].poison = 0;
    for (k = 0; k < 3; k++)
    {
        c = Relabel(&g, 0, TCG_CARD_GRASS_ENERGY);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    }
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACH;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Harvest Bounty's first: %s\n", g.error), 1;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.slot = 1;
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Harvest Bounty onto the Bench\n"), 1;
    a.slot = 0;
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Harvest Bounty's second: %s\n", g.error), 1;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Harvest Bounty: a third Energy\n"), 1;
    printf("PASS Lock-on, Frog Song, Psymimic, Harvest Bounty\n");
    return 0;
}
/* A choice in the middle of an effect: Light Venomoth's (their pick, then yours, then the turn ends), through the
 * flow as the board and the phone ask it; Special Delivery's */
static unsigned PeekCount(const struct TcgDuel *g, unsigned who)
{
    unsigned k, n = 0;
    for (k = 0; k < TCG_EVENT_RING; k++)
        n += g->events[k].type == TCG_EV_PEEK && g->events[k].player == who;
    return n;
}
static int Peeks(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned hand, prizes;
    int c;
    if (!Tcg_Card(TCG_CARD_NEO2_69) || !Tcg_Card(TCG_CARD_ECARD3_40) || !Tcg_Card(TCG_CARD_GYM1_112))
        return printf("FAIL peeks: a card isn't finished\n"), 1;
    /* [Observe]: their top 5 shown, in order, nothing moved */
    Stage(&g, TCG_CARD_NEO2_69, 0, 1);
    g.players[0].field[0].poison = 0;
    memset(g.events, 0, sizeof(g.events));
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    {
        uint8_t top = g.players[1].deck[g.players[1].deckSize - 1];
        if (TcgDuel_Apply(&g, &a) != TCG_OK || PeekCount(&g, 0) != 5 || g.players[1].deck[g.players[1].deckSize - 1] != top)
            return printf("FAIL [Observe]: %s (%u looked at)\n", g.error, PeekCount(&g, 0)), 1;
    }
    /* Investigate: a choice of what to look at, 2 of their Prizes */
    Stage(&g, TCG_CARD_ECARD3_40, 0, 1);
    g.players[0].field[0].poison = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 0 || g.choiceKind != TCG_CHOICE_PEEK_ZONE)
        return printf("FAIL Investigate: %s\n", g.error), 1;
    memset(g.events, 0, sizeof(g.events));
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = TCG_PEEK_THEIR_PRIZE;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || PeekCount(&g, 0) != 2 || g.phase != TCG_TURN)
        return printf("FAIL Investigate's look: %s (%u)\n", g.error, PeekCount(&g, 0)), 1;
    /* Lt. Surge's Treaty: they choose; everyone takes a Prize, or you draw */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_GYM1_112);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 1)
        return printf("FAIL Lt. Surge's Treaty: %s\n", g.error), 1;
    hand = g.players[1].handSize, prizes = g.players[0].prizeSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = 0;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].handSize != hand + 1 || g.players[0].prizeSize != prizes - 1 ||
        g.phase != TCG_TURN)
        return printf("FAIL Lt. Surge's Treaty's Prizes: %s\n", g.error), 1;
    /* Lightning Rod: the marker on the chosen one of theirs; Lightning Bolt: 20 to it besides */
    if (Tcg_Card(TCG_CARD_BASEP_46))
    {
        Stage(&g, TCG_CARD_BASEP_46, 0, 1);
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || !(g.players[1].field[1].markers & TCG_MARK_ROD))
            return printf("FAIL Lightning Rod: %s\n", g.error), 1;
        Stage(&g, TCG_CARD_BASEP_46, 1, 1);
        g.players[1].field[1].markers = TCG_MARK_ROD;
        hand = g.players[1].field[1].damage;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[1].damage != hand + 20)
            return printf("FAIL Lightning Bolt: %s (%u -> %u)\n", g.error, hand, g.players[1].field[1].damage), 1;
    }
    /* Sweet Scent: heads one of yours, tails one of theirs, 2 counters off */
    if (Tcg_Card(TCG_CARD_NEO1_29))
    {
        unsigned q;
        Stage(&g, TCG_CARD_NEO1_29, 0, 1);
        g.players[0].field[1].damage = 30;
        g.players[1].field[1].damage = 30;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 0 ||
            (g.choiceKind != TCG_CHOICE_HEAL_MINE && g.choiceKind != TCG_CHOICE_HEAL_THEIRS))
            return printf("FAIL Sweet Scent: %s (phase %u)\n", g.error, g.phase), 1;
        q = g.choiceKind == TCG_CHOICE_HEAL_MINE ? 0 : 1;
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[q].field[1].damage != 10)
            return printf("FAIL Sweet Scent's heal: %s\n", g.error), 1;
    }
    printf("PASS peeks: [Observe], Investigate's choice, Lt. Surge's Treaty, Lightning Rod, Sweet Scent\n");
    return 0;
}
/* Energy Control, Energy Splash, Electric Blast, Attract Current: choices asked again and again */
static int MoreChoices(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, k, n;
    if (!Tcg_Card(TCG_CARD_BASEP_12) || !Tcg_Card(TCG_CARD_ECARD3_25) || !Tcg_Card(TCG_CARD_ECARD3_19) ||
        !Tcg_Card(TCG_CARD_NEO3_1))
        return printf("FAIL more choices: a card isn't finished\n"), 1;
    /* Energy Control (heads): their Active's Water Energy onto their Benched Machop */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_BASEP_12, 0, seed);
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Energy Control: %s\n", g.error), 1;
        if (g.phase == TCG_CHOICE)
            break;
    }
    if (g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_MOVE_THEIRS_FROM)
        return printf("FAIL Energy Control: no choice in 40 coins\n"), 1;
    n = g.players[1].field[0].attachedSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = 0;
    a.selections[0] = g.players[1].field[0].attached[0];
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_MOVE_THEIRS_TO)
        return printf("FAIL Energy Control's card: %s\n", g.error), 1;
    k = g.players[1].field[1].attachedSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].attachedSize != n - 1 ||
        g.players[1].field[1].attachedSize != k + 1 || g.phase == TCG_CHOICE)
        return printf("FAIL Energy Control's move: %s\n", g.error), 1;
    /* Energy Splash: 2 Water Energy to the Bench, one and then the other */
    Stage(&g, TCG_CARD_ECARD3_25, 2, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = 2;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_WATER_TO_BENCH)
        return printf("FAIL Energy Splash: %s (phase %u)\n", g.error, g.phase), 1;
    k = g.players[0].field[1].attachedSize;
    for (n = 0; n < 2; n++)
    {
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Energy Splash's move %u: %s\n", n, g.error), 1;
    }
    if (g.players[0].field[1].attachedSize != k + 2 || g.phase == TCG_CHOICE)
        return printf("FAIL Energy Splash: %u moved\n", g.players[0].field[1].attachedSize - k), 1;
    /* Electric Blast (YES): its Lightning Energy discarded, a counter for each on their Bench */
    Stage(&g, TCG_CARD_ECARD3_19, 1, 1);
    Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 1);
    for (k = n = 0; k < g.players[0].field[0].attachedSize; k++)
        n += Tcg_Card(g.players[0].cards[g.players[0].field[0].attached[k]])->type == TCG_LIGHTNING;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = 1;
    a.option = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_COUNTER_BENCH ||
        g.choiceData != n)
        return printf("FAIL Electric Blast: %s (%u Lightning, %u asked)\n", g.error, n, g.choiceData), 1;
    for (k = 0; k < n; k++)
    {
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Electric Blast's counter %u: %s\n", k, g.error), 1;
    }
    if (g.players[1].field[1].stackSize && g.players[1].field[1].damage != 10 * n)
        return printf("FAIL Electric Blast: %u damage\n", g.players[1].field[1].damage), 1;
    /* Attract Current: a Lightning Energy for each heads onto a Lightning Pokemon (itself) */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_NEO3_1, 0, seed);
        {
            int c = Relabel(&g, 0, TCG_CARD_LIGHTNING_ENERGY);   /* (back into the deck) */
            if (c >= 0)
                g.players[0].deck[g.players[0].deckSize++] = (uint8_t)c;
        }
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Attract Current: %s\n", g.error), 1;
        if (g.phase == TCG_CHOICE)
            break;
    }
    if (g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_ATTACH_LIGHTNING)
        return printf("FAIL Attract Current: no choice\n"), 1;
    n = g.players[0].field[0].attachedSize;
    TcgDuel_AiChoose(&g, &a);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].attachedSize != n + 1)
        return printf("FAIL Attract Current's attach: %s\n", g.error), 1;
    /* Ion Coating: the Active's Lightning Energy is Water this turn, not once it retreats */
    if (Tcg_Card(TCG_CARD_ECARD2_21) && Tcg_Card(TCG_CARD_NEO3_12) && Tcg_Card(TCG_CARD_NEO4_16))
    {
        Stage(&g, TCG_CARD_ECARD2_21, 1, 1);
        g.players[0].field[0].poison = 0;
        Attach(&g, 0, 0, TCG_CARD_LIGHTNING_ENERGY, 1);
        k = TcgDuel_Energy(&g, 0, 0, TCG_WATER), n = TcgDuel_Energy(&g, 0, 0, TCG_LIGHTNING);
        memset(&a, 0, sizeof(a));
        a.type = TCG_USE_POWER;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || TcgDuel_Energy(&g, 0, 0, TCG_WATER) != k + n ||
            TcgDuel_Energy(&g, 0, 0, TCG_LIGHTNING))
            return printf("FAIL Ion Coating: %s\n", g.error), 1;
        /* Energy Converter: a Water Energy of its is Metal this turn */
        Stage(&g, TCG_CARD_NEO3_12, 1, 1);
        g.players[0].field[0].poison = 0;
        for (k = 0; k < g.players[0].field[0].attachedSize && g.players[0].cards[g.players[0].field[0].attached[k]] != TCG_CARD_WATER_ENERGY; k++)
            ;
        memset(&a, 0, sizeof(a));
        a.type = TCG_USE_POWER;
        a.selections[0] = g.players[0].field[0].attached[k];
        a.selectionCount = 1;
        a.target = TCG_METAL;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || TcgDuel_Energy(&g, 0, 0, TCG_METAL) != 1)
            return printf("FAIL Energy Converter: %s (%u Metal)\n", g.error, TcgDuel_Energy(&g, 0, 0, TCG_METAL)), 1;
        /* Miracle Energy: only on a Shining or Light Pokemon */
        Stage(&g, TCG_CARD_MACHOP, 0, 1);
        {
            int c = Relabel(&g, 0, TCG_CARD_NEO4_16);
            g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
        }
        g.players[0].energyPlayed = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACH;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        if (TcgDuel_Apply(&g, &a) == TCG_OK)
            return printf("FAIL Miracle Energy went on a Machop\n"), 1;
    }
    printf("PASS more choices: Energy Control, Energy Splash, Electric Blast, Attract Current, Ion Coating, Energy Converter, Miracle Energy\n");
    return 0;
}
/* Texture Magic, Card-Flip Game, Championship Arena, Pure Body, Scavenger Hunt */
static int MoreRules(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned k, n;
    int c;
    if (!Tcg_Card(TCG_CARD_BASEP_15) || !Tcg_Card(TCG_CARD_NEO1_92) || !Tcg_Card(TCG_CARD_NP_28) ||
        !Tcg_Card(TCG_CARD_NP_30) || !Tcg_Card(TCG_CARD_ECARD2_48))
        return printf("FAIL more rules: a card isn't finished\n"), 1;
    /* Texture Magic: its Resistance to Fighting, the Defending Machop's Weakness to Colorless? no: to Psychic */
    Stage(&g, TCG_CARD_BASEP_15, 0, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_TEXTURE_RES)
        return printf("FAIL Texture Magic: %s\n", g.error), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = TCG_FIGHTING;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_TEXTURE_WEAK)
        return printf("FAIL Texture Magic's Resistance: %s\n", g.error), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = TCG_PSYCHIC;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase == TCG_CHOICE || TcgDuel_Weakness(&g, 1, 0) != TCG_PSYCHIC)
        return printf("FAIL Texture Magic's Weakness: %s (%u)\n", g.error, TcgDuel_Weakness(&g, 1, 0)), 1;
    /* Card-Flip Game: a guess at one of their Prizes; right, draw 2 */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    while (g.players[1].prizeSize < 3)
        g.players[1].prizes[g.players[1].prizeSize++] = g.players[1].deck[--g.players[1].deckSize];
    c = Relabel(&g, 0, TCG_CARD_NEO1_92);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_CARD_FLIP)
        return printf("FAIL Card-Flip Game: %s\n", g.error), 1;
    {
        const struct TcgCard *pz = Tcg_Card(g.players[1].cards[g.choiceData]);
        n = g.players[0].handSize;
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = pz->kind == TCG_ENERGY ? 0 : pz->kind == TCG_TRAINER ? 1 : 2;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].handSize != n + 2)
            return printf("FAIL Card-Flip Game's right guess: %s\n", g.error), 1;
    }
    /* Championship Arena: 9 cards at the end of the turn, 2 discarded first */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_NP_28);
    g.players[0].stadium = (uint8_t)(c + 1);
    while (g.players[0].handSize < 9)
        g.players[0].hand[g.players[0].handSize++] = g.players[0].deck[--g.players[0].deckSize];
    memset(&a, 0, sizeof(a));
    a.type = TCG_PASS;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_ARENA || g.current != 0)
        return printf("FAIL Championship Arena: %s (phase %u)\n", g.error, g.phase), 1;
    TcgDuel_AiChoose(&g, &a);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].handSize != 7 || g.current != 1)
        return printf("FAIL Championship Arena's discard: %s (hand %u, current %u)\n", g.error, g.players[0].handSize, g.current), 1;
    /* Pure Body: a Water Energy onto Suicune, one of its Energy discarded */
    Stage(&g, TCG_CARD_NP_30, 0, 1);
    n = g.players[0].field[0].attachedSize;
    c = Take(&g, 0, TCG_CARD_WATER_ENERGY);
    if (c < 0)
        c = Relabel(&g, 0, TCG_CARD_WATER_ENERGY);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    g.players[0].energyPlayed = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACH;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_PURE_BODY)
        return printf("FAIL Pure Body: %s\n", g.error), 1;
    TcgDuel_AiChoose(&g, &a);
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].attachedSize != n)
        return printf("FAIL Pure Body's discard: %s\n", g.error), 1;
    /* Scavenger Hunt: 2 hand cards into the deck, an Energy from the deck into the hand */
    Stage(&g, TCG_CARD_ECARD2_48, 1, 1);
    g.players[0].field[0].poison = 0;
    for (k = 0; k < 2; k++)
        g.players[0].hand[g.players[0].handSize++] = g.players[0].deck[--g.players[0].deckSize];
    n = g.players[0].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    a.selections[0] = g.players[0].hand[0], a.selections[1] = g.players[0].hand[1];
    for (k = 0; k < g.players[0].deckSize && Tcg_Card(g.players[0].cards[g.players[0].deck[k]])->kind != TCG_ENERGY; k++)
        ;
    a.selections[2] = g.players[0].deck[k];
    a.selectionCount = 3;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].handSize != n - 1)
        return printf("FAIL Scavenger Hunt: %s (hand %u -> %u)\n", g.error, n, g.players[0].handSize), 1;
    printf("PASS more rules: Texture Magic, Card-Flip Game, Championship Arena, Pure Body, Scavenger Hunt\n");
    return 0;
}
static void Choose(struct TcgDuel *g, unsigned target, int pick)
{
    struct TcgAction a;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.target = (uint8_t)target;
    if (pick >= 0)
        a.selections[0] = (uint8_t)pick, a.selectionCount = 1;
    if (TcgDuel_Apply(g, &a) != TCG_OK)
        printf("(choose %u: %s)\n", target, g->error);
}
static int PlayHeld(struct TcgDuel *g, unsigned id)
{
    struct TcgAction a;
    int c = Relabel(g, 0, id);
    g->players[0].hand[g->players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g->players[0].handSize - 1);
    return TcgDuel_Apply(g, &a);
}
/* Chaos Move, Magnetic Flow, Invigorate, Misty's Wish, Mind Games, Tickling Machine, Fairy Power */
static int EvenMoreRules(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, k, n;
    int c;
    if (!Tcg_Card(TCG_CARD_ECARD1_13) || !Tcg_Card(TCG_CARD_ECARD2_22) || !Tcg_Card(TCG_CARD_GYM2_56) ||
        !Tcg_Card(TCG_CARD_GYM2_108) || !Tcg_Card(TCG_CARD_NEO1_14) || !Tcg_Card(TCG_CARD_GYM1_119) ||
        !Tcg_Card(TCG_CARD_GYM1_3))
        return printf("FAIL even more rules: a card isn't finished\n"), 1;
    /* Chaos Move: a counter off their Benched Machop onto their Active */
    Stage(&g, TCG_CARD_ECARD1_13, 1, 1);
    g.players[0].field[0].poison = 0;
    while (g.players[1].prizeSize > 3)
        g.players[1].deck[g.players[1].deckSize++] = g.players[1].prizes[--g.players[1].prizeSize];
    while (g.players[1].prizeSize < 3)
        g.players[1].prizes[g.players[1].prizeSize++] = g.players[1].deck[--g.players[1].deckSize];
    g.players[1].field[1].damage = 20;
    k = g.players[1].field[0].damage;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_CHAOS_FROM_SIDE)
        return printf("FAIL Chaos Move: %s\n", g.error), 1;
    Choose(&g, 1, -1), Choose(&g, 1, -1), Choose(&g, 1, -1), Choose(&g, 0, -1);
    if (g.players[1].field[1].damage != 10 || g.players[1].field[0].damage != k + 10 || g.phase == TCG_CHOICE)
        return printf("FAIL Chaos Move's counter (%u, %u)\n", g.players[1].field[1].damage, g.players[1].field[0].damage), 1;
    /* Magnetic Flow (heads): an Energy of their Active and of their Diglett switched */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_ECARD2_22, 1, seed);
        g.players[0].field[0].poison = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_USE_POWER;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Magnetic Flow: %s\n", g.error), 1;
        if (g.phase == TCG_CHOICE)
            break;
    }
    if (g.choiceKind != TCG_CHOICE_FLOW_A)
        return printf("FAIL Magnetic Flow: no choice\n"), 1;
    {
        uint8_t first = g.players[1].field[0].attached[g.players[1].field[0].attachedSize - 1], second = g.players[1].field[2].attached[0];
        Choose(&g, 0, first);
        Choose(&g, 2, second);
        if (g.players[1].field[2].attached[0] != first || g.phase == TCG_CHOICE)
            return printf("FAIL Magnetic Flow's switch\n"), 1;
    }
    /* Invigorate: a Diglett from their discard pile onto their Bench, half its HP in counters */
    Stage(&g, TCG_CARD_GYM2_56, 0, 1);
    for (k = 0; k < g.players[1].deckSize && g.players[1].cards[g.players[1].deck[k]] != TCG_CARD_DIGLETT; k++)
        ;
    g.players[1].discard[g.players[1].discardSize++] = TcgPile_Remove(g.players[1].deck, &g.players[1].deckSize, k);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_INVIGORATE_SIDE)
        return printf("FAIL Invigorate: %s\n", g.error), 1;
    Choose(&g, 1, -1);
    Choose(&g, 0, g.players[1].discard[g.players[1].discardSize - 1]);
    if (!g.players[1].field[3].stackSize || g.players[1].field[3].damage != Tcg_Card(TCG_CARD_DIGLETT)->hp / 2 / 10 * 10)
        return printf("FAIL Invigorate's Diglett\n"), 1;
    /* Misty's Wish: they accept; a hand card switched with the Prize looked at */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    while (g.players[0].prizeSize < 2)
        g.players[0].prizes[g.players[0].prizeSize++] = g.players[0].deck[--g.players[0].deckSize];
    g.players[0].hand[g.players[0].handSize++] = g.players[0].deck[--g.players[0].deckSize];
    if (PlayHeld(&g, TCG_CARD_GYM2_108) != TCG_OK || g.choiceKind != TCG_CHOICE_WISH_ASK || g.choicePlayer != 1)
        return printf("FAIL Misty's Wish: %s\n", g.error), 1;
    Choose(&g, 1, -1);
    if (g.choiceKind != TCG_CHOICE_WISH_SWAP || g.choicePlayer != 0)
        return printf("FAIL Misty's Wish: not asked to switch\n"), 1;
    n = g.players[0].hand[0];
    Choose(&g, 0, (int)n);
    for (k = 0; k < g.players[0].prizeSize && g.players[0].prizes[k] != n; k++)
        ;
    if (k == g.players[0].prizeSize)
        return printf("FAIL Misty's Wish's switch\n"), 1;
    /* Mind Games: their Slowking; heads, the Trainer does nothing and goes on top of the deck */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, seed);
        c = Relabel(&g, 1, TCG_CARD_NEO1_14);
        if (c < 0)
            return printf("FAIL Mind Games: no card to make a Slowking of\n"), 1;
        g.players[1].field[3].stack[g.players[1].field[3].stackSize++] = (uint8_t)c;
        if (PlayHeld(&g, TCG_CARD_GYM1_112) != TCG_OK)
            return printf("FAIL Mind Games: %s\n", g.error), 1;
        if (g.phase != TCG_CHOICE)
            break;
    }
    if (Tcg_Card(g.players[0].cards[g.players[0].deck[g.players[0].deckSize - 1]])->id != TCG_CARD_GYM1_112)
        return printf("FAIL Mind Games: the Trainer isn't on top of the deck\n"), 1;
    /* Tickling Machine (heads): their hand set aside until the end of their next turn */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, seed);
        for (k = 0; k < 3; k++)
            g.players[1].hand[g.players[1].handSize++] = g.players[1].deck[--g.players[1].deckSize];
        n = g.players[1].handSize;
        if (PlayHeld(&g, TCG_CARD_GYM1_119) != TCG_OK)
            return printf("FAIL Tickling Machine: %s\n", g.error), 1;
        if (g.players[1].asideSize)
            break;
    }
    if (g.players[1].asideSize != n || g.players[1].handSize)
        return printf("FAIL Tickling Machine: %u set aside\n", g.players[1].asideSize), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PASS;
    TcgDuel_Apply(&g, &a);
    TcgDuel_Apply(&g, &a);
    if (g.players[1].asideSize || g.players[1].handSize < n)
        return printf("FAIL Tickling Machine: the hand didn't come back (%u, %u)\n", g.players[1].asideSize, g.players[1].handSize), 1;
    /* Fairy Power (heads): the Benched Machop back to the hand, then DONE */
    for (seed = 1; seed < 40; seed++)
    {
        Stage(&g, TCG_CARD_GYM1_3, 0, seed);
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Fairy Power: %s\n", g.error), 1;
        if (g.phase == TCG_CHOICE)
            break;
    }
    n = g.players[0].handSize;
    Choose(&g, 1, -1);
    if (g.players[0].field[1].stackSize || g.players[0].handSize != n + 1 || g.choiceKind != TCG_CHOICE_RETURN_MON)
        return printf("FAIL Fairy Power's return\n"), 1;
    Choose(&g, 7, -1);
    if (g.phase == TCG_CHOICE)
        return printf("FAIL Fairy Power: DONE didn't end it\n"), 1;
    printf("PASS even more rules: Chaos Move, Magnetic Flow, Invigorate, Misty's Wish, Mind Games, Tickling Machine, Fairy Power\n");
    return 0;
}
/* Blaine's Quiz #1 / #3, Shuffle Attack, Howl, Manipulate */
static int QuizzesAndPlays(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned k, n, q;
    int c;
    uint16_t opts[4];
    if (!Tcg_Card(TCG_CARD_GYM1_97) || !Tcg_Card(TCG_CARD_GYM2_112) || !Tcg_Card(TCG_CARD_ECARD2_34) ||
        !Tcg_Card(TCG_CARD_NEO3_6) || !Tcg_Card(TCG_CARD_ECARD3_10))
        return printf("FAIL quizzes and plays: a card isn't finished\n"), 1;
    /* the Quizzes: a right guess draws the guesser 2 / 3 */
    for (q = 0; q < 2; q++)
    {
        Stage(&g, TCG_CARD_MACHOP, 0, 1);
        c = Take(&g, 0, TCG_CARD_MACHOP);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
        k = Relabel(&g, 0, q ? TCG_CARD_GYM2_112 : TCG_CARD_GYM1_97);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)k;
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_TRAINER;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        a.selections[0] = (uint8_t)c;
        a.selectionCount = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choicePlayer != 1 ||
            g.choiceKind != (q ? TCG_CHOICE_QUIZ3 : TCG_CHOICE_QUIZ1))
            return printf("FAIL Blaine's Quiz #%u: %s\n", q ? 3 : 1, g.error), 1;
        n = g.players[1].handSize;
        Choose(&g, TcgDuel_QuizOptions(&g, opts), -1);
        if (g.players[1].handSize != n + (q ? 3 : 2) || g.phase == TCG_CHOICE)
            return printf("FAIL Blaine's Quiz #%u's right guess\n", q ? 3 : 1), 1;
    }
    /* Shuffle Attack: their top cards (3 Energy on the Defending Machop) in the order picked */
    Stage(&g, TCG_CARD_ECARD2_34, 1, 1);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    a.attack = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_ORDER_THEIRS || g.choiceData != 3)
        return printf("FAIL Shuffle Attack: %s (%u)\n", g.error, g.choiceData), 1;
    {
        struct TcgPlayer *d = &g.players[1];
        uint8_t was[3] = {d->deck[d->deckSize - 1], d->deck[d->deckSize - 2], d->deck[d->deckSize - 3]};
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.selections[0] = was[2], a.selections[1] = was[1], a.selections[2] = was[0];
        a.selectionCount = 3;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || d->deck[d->deckSize - 1] != was[2] || d->deck[d->deckSize - 3] != was[0])
            return printf("FAIL Shuffle Attack's order: %s\n", g.error), 1;
    }
    /* Howl: YES discards the top 5 and ends the turn */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_NEO3_6);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    n = g.players[0].deckSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_BASIC;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.slot = 3;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_HOWL_YES)
        return printf("FAIL Howl: %s\n", g.error), 1;
    Choose(&g, 1, -1);
    while (g.phase == TCG_CHOICE && g.choiceKind == TCG_CHOICE_HOWL_ATTACH)
        Choose(&g, 3, -1);
    if (g.players[0].deckSize + 5u != n || g.current != 1u)
        return printf("FAIL Howl: deck %u -> %u, current %u\n", n, g.players[0].deckSize, g.current), 1;
    /* Manipulate: Gengar evolving the Active, a Basic from the discard pile onto the Bench */
    Stage(&g, TCG_CARD_GASTLY_LV8, 0, 1);
    g.players[0].field[0].enteredTurn = 0;
    c = Relabel(&g, 0, TCG_CARD_HAUNTER_LV17);
    g.players[0].field[0].stack[g.players[0].field[0].stackSize++] = (uint8_t)c;
    c = Relabel(&g, 0, TCG_CARD_ECARD3_10);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    c = Take(&g, 0, TCG_CARD_GASTLY_LV8);
    if (c < 0)
        c = Relabel(&g, 0, TCG_CARD_GASTLY_LV8);
    g.players[0].discard[g.players[0].discardSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_EVOLVE;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.slot = 0;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_MANIPULATE)
        return printf("FAIL Manipulate: %s (phase %u kind %u)\n", g.error, g.phase, g.choiceKind), 1;
    Choose(&g, 0, c);
    while (g.phase == TCG_CHOICE)
    {
        TcgDuel_AiChoose(&g, &a);
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Manipulate's Energy: %s\n", g.error), 1;
    }
    if (!g.players[0].field[3].stackSize)
        return printf("FAIL Manipulate: nothing on the Bench\n"), 1;
    printf("PASS quizzes and plays: Blaine's Quiz #1 and #3, Shuffle Attack, Howl, Manipulate\n");
    return 0;
}
/* Play the Trainer id from player 0's hand at target, with one selection (or none: sel < 0) */
static int PlayAt(struct TcgDuel *g, unsigned id, unsigned target, int sel)
{
    struct TcgAction a;
    int c = Relabel(g, 0, id);
    g->players[0].hand[g->players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g->players[0].handSize - 1);
    a.target = (uint8_t)target;
    if (sel >= 0)
        a.selections[0] = (uint8_t)sel, a.selectionCount = 1;
    return TcgDuel_Apply(g, &a);
}
static void Prizes3(struct TcgDuel *g)
{
    unsigned who;
    for (who = 0; who < 2; who++)
        while (g->players[who].prizeSize < 3)
            g->players[who].prizes[g->players[who].prizeSize++] = g->players[who].deck[--g->players[who].deckSize];
}
static void AttachTool(struct TcgDuel *g, unsigned who, unsigned slot, unsigned id)
{
    int c = Relabel(g, who, id);
    g->players[who].field[slot].attached[g->players[who].field[slot].attachedSize++] = (uint8_t)c;
}
/* Flee, Bench Guard, Legendary Body, Unown [K], Ecogym, Brock's Protection, Pokemon Tower, Allergic Pollen, Time Shard,
 * EXP.ALL, Super Metronome */
static int LastRules(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    const struct TcgAttack *at;
    unsigned k, n, d;
    int c;
    static const unsigned need[] = {TCG_CARD_GYM1_10, TCG_CARD_GYM1_2, TCG_CARD_NEO3_17, TCG_CARD_NEO3_58, TCG_CARD_NEO1_84,
                                    TCG_CARD_GYM2_101, TCG_CARD_BASEP_42, TCG_CARD_NEO3_35, TCG_CARD_ECARD2_135,
                                    TCG_CARD_NEO4_93, TCG_CARD_NEO1_16, TCG_CARD_GYM2_56};
    for (k = 0; k < sizeof(need) / sizeof(need[0]); k++)
        if (!Tcg_Card(need[k]))
            return printf("FAIL last rules: card %u isn't finished\n", need[k]), 1;
    /* Flee: their Tentacruel, damaged and not Knocked Out, switched with a Benched one; then their turn */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    Prizes3(&g);
    g.players[0].field[0].poison = 0;
    g.players[1].cards[g.players[1].field[0].stack[0]] = TCG_CARD_GYM1_10;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_FLEE || g.choicePlayer != 1)
        return printf("FAIL Flee: not asked (%s)\n", g.error), 1;
    Choose(&g, 1, -1);
    if (g.players[1].cards[g.players[1].field[1].stack[0]] != TCG_CARD_GYM1_10 || !g.players[1].field[1].damage ||
        g.phase != TCG_TURN || g.current != 1)
        return printf("FAIL Flee's switch\n"), 1;
    /* Bench Guard: 10 of a Benched one's 20 to the Benched Rhydon */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    g.players[1].cards[g.players[1].field[1].stack[0]] = TCG_CARD_GYM1_2;
    TcgDuel_EffectDamage(&g, 1, 2, 20, 0);
    if (g.players[1].field[1].damage != 10 || g.players[1].field[2].damage != 10)
        return printf("FAIL Bench Guard (%u, %u)\n", g.players[1].field[1].damage, g.players[1].field[2].damage), 1;
    /* Legendary Body: no Potion on the Active Entei */
    Stage(&g, TCG_CARD_NEO3_17, 0, 1);
    g.players[0].field[0].damage = 20;
    if (PlayAt(&g, TCG_CARD_POTION, 0, -1) == TCG_OK || g.players[0].field[0].damage != 20)
        return printf("FAIL Legendary Body let a Potion through\n"), 1;
    /* Unown [K]: Energy Removal leaves its Energy on */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    g.players[1].cards[g.players[1].field[0].stack[0]] = TCG_CARD_NEO3_58;
    n = g.players[1].field[0].attachedSize;
    if (PlayAt(&g, TCG_CARD_ENERGY_REMOVAL, 0, g.players[1].field[0].attached[0]) != TCG_OK || g.players[1].field[0].attachedSize != n)
        return printf("FAIL [Keep] (%s)\n", g.error), 1;
    /* Brock's Protection, the same */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    g.players[1].cards[g.players[1].field[0].stack[0]] = TCG_CARD_GYM1_2;
    AttachTool(&g, 1, 0, TCG_CARD_GYM2_101);
    n = g.players[1].field[0].attachedSize;
    if (PlayAt(&g, TCG_CARD_ENERGY_REMOVAL, 0, g.players[1].field[0].attached[0]) != TCG_OK || g.players[1].field[0].attachedSize != n)
        return printf("FAIL Brock's Protection (%s)\n", g.error), 1;
    /* Ecogym: the Water Energy removed goes to their hand */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_NEO1_84);
    g.players[0].stadium = (uint8_t)(c + 1);
    n = g.players[1].handSize, d = g.players[1].discardSize;
    if (PlayAt(&g, TCG_CARD_ENERGY_REMOVAL, 0, g.players[1].field[0].attached[0]) != TCG_OK ||
        g.players[1].handSize != n + 1 || g.players[1].discardSize != d)
        return printf("FAIL Ecogym (%s)\n", g.error), 1;
    /* Pokemon Tower: Item Finder's Potion stays in the discard pile */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    c = Relabel(&g, 0, TCG_CARD_BASEP_42);
    g.players[0].stadium = (uint8_t)(c + 1);
    c = Relabel(&g, 0, TCG_CARD_POTION);
    g.players[0].discard[g.players[0].discardSize++] = (uint8_t)c;
    for (k = 0; k < 2; k++)
        g.players[0].hand[g.players[0].handSize++] = g.players[0].deck[--g.players[0].deckSize];
    c = Relabel(&g, 0, TCG_CARD_ITEM_FINDER);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_TRAINER;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.selections[0] = g.players[0].hand[g.players[0].handSize - 2];
    a.selections[1] = g.players[0].hand[g.players[0].handSize - 3];
    a.selections[2] = g.players[0].discard[g.players[0].discardSize - 1];
    a.selectionCount = 3;
    k = a.selections[2];
    if (TcgDuel_Apply(&g, &a) != TCG_OK)
        return printf("FAIL Pokemon Tower: %s\n", g.error), 1;
    for (n = 0; n < g.players[0].discardSize && g.players[0].discard[n] != k; n++)
        ;
    if (n == g.players[0].discardSize)
        return printf("FAIL Pokemon Tower: the Potion came back\n"), 1;
    /* Allergic Pollen: Invigorate can't take their Diglett from the discard pile */
    Stage(&g, TCG_CARD_GYM2_56, 0, 1);
    g.players[1].cards[g.players[1].field[1].stack[0]] = TCG_CARD_NEO3_35;
    for (k = 0; k < g.players[1].deckSize && g.players[1].cards[g.players[1].deck[k]] != TCG_CARD_DIGLETT; k++)
        ;
    g.players[1].discard[g.players[1].discardSize++] = TcgPile_Remove(g.players[1].deck, &g.players[1].deckSize, k);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) == TCG_OK && g.phase == TCG_CHOICE)
    {
        Choose(&g, 1, -1);
        if (g.phase == TCG_CHOICE)
            Choose(&g, 0, g.players[1].discard[g.players[1].discardSize - 1]);
    }
    if (g.players[1].field[3].stackSize || g.players[1].cards[g.players[1].discard[g.players[1].discardSize - 1]] != TCG_CARD_DIGLETT)
        return printf("FAIL Allergic Pollen\n"), 1;
    /* Time Shard: their Machop Knocked Out, 2 basic Energy cards to their hand */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    Prizes3(&g);
    g.players[0].field[0].poison = 0;
    AttachTool(&g, 1, 0, TCG_CARD_ECARD2_135);
    g.players[1].field[0].damage = (uint16_t)(TcgDuel_MaxHp(&g, 1, 0) - 10);
    n = g.players[1].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].handSize != n + 2)
        return printf("FAIL Time Shard (%u to the hand; %s)\n", g.players[1].handSize - n, g.error), 1;
    /* EXP.ALL: a basic Energy of the Knocked Out Machop onto the Benched one with it, EXP.ALL discarded */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    Prizes3(&g);
    g.players[0].field[0].poison = 0;
    AttachTool(&g, 1, 1, TCG_CARD_NEO4_93);
    g.players[1].field[0].damage = (uint16_t)(TcgDuel_MaxHp(&g, 1, 0) - 10);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[1].attachedSize != 1 ||
        Tcg_Card(g.players[1].cards[g.players[1].field[1].attached[0]])->kind != TCG_ENERGY)
        return printf("FAIL EXP.ALL (%s)\n", g.error), 1;
    /* Super Metronome: any attack of their Pokemon in play */
    Stage(&g, TCG_CARD_NEO1_16, 0, 1);
    at = TcgDuel_MetronomeAttack(&g, 0, 3);
    if (!at || strcmp(at->name, Tcg_Card(TCG_CARD_DIGLETT)->attacks[0].name))
        return printf("FAIL Super Metronome: %s\n", at ? at->name : "nothing"), 1;
    printf("PASS last rules: Flee, Bench Guard, Legendary Body, [Keep], Brock's Protection, Ecogym, Pokemon Tower, Allergic Pollen, Time Shard, EXP.ALL, Super Metronome\n");
    return 0;
}
/* Koga's Ninja Trick, Vermilion City Gym, Sabrina's ESP, Charity: answered before / after an attack (the attack held) */
static int HeldAttacks(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned seed, base = 0, baseSelf = 0, seen = 0;
    int c;
    /* Ninja Trick: their Koga's Grimer switches out first; the attack hits the Machop that came in */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    Prizes3(&g);
    g.players[0].field[0].poison = 0;
    g.players[1].cards[g.players[1].field[0].stack[0]] = TCG_CARD_GYM2_78;
    AttachTool(&g, 1, 0, TCG_CARD_GYM2_115);
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choiceKind != TCG_CHOICE_NINJA || g.choicePlayer != 1)
        return printf("FAIL Ninja Trick: not asked (%s)\n", g.error), 1;
    Choose(&g, 1, -1);
    if (g.players[1].cards[g.players[1].field[1].stack[0]] != TCG_CARD_GYM2_78 || g.players[1].field[1].damage ||
        !g.players[1].field[0].damage || TcgDuel_AttachedCard(&g, 1, 1, TCG_CARD_GYM2_115) >= 0 || g.current != 1)
        return printf("FAIL Ninja Trick: the switch or the attack\n"), 1;
    /* Vermilion City Gym: NO, the attack as it is; YES, heads 10 more or tails 10 to the attacker */
    for (seed = 1; seed < 40 && seen != 3; seed++)
    {
        unsigned yes;
        for (yes = 0; yes < 2; yes++)
        {
            Stage(&g, TCG_CARD_GYM1_52, 0, seed);
            Prizes3(&g);
            for (c = 0; c < 2; c++)
                g.players[c].field[0].poison = g.players[c].field[0].burned = 0, g.players[c].field[0].condition = TCG_HEALTHY,
                g.players[c].field[0].damage = 0;
            c = Relabel(&g, 0, TCG_CARD_GYM1_120);
            g.players[0].stadium = (uint8_t)(c + 1);
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_VERMILION || g.phase != TCG_CHOICE)
                return printf("FAIL Vermilion City Gym: not asked (%s)\n", g.error), 1;
            Choose(&g, yes, -1);
            if (!yes)
                base = g.players[1].field[0].damage, baseSelf = g.players[0].field[0].damage;
            else if (g.players[1].field[0].damage == base + 10 && g.players[0].field[0].damage == baseSelf)
                seen |= 1;
            else if (g.players[1].field[0].damage == base && g.players[0].field[0].damage == baseSelf + 10)
                seen |= 2;
            else
                return printf("FAIL Vermilion City Gym: %u / %u (base %u, seed %u, last log %s)\n", g.players[1].field[0].damage,
                              g.players[0].field[0].damage, base, seed, g.error), 1;
        }
    }
    if (seen != 3)
        return printf("FAIL Vermilion City Gym: outcomes %u\n", seen), 1;
    /* Sabrina's ESP: KEEP gives the coins it showed; RE-FLIP flips them anew */
    for (seed = 1, seen = 0; seed < 60 && seen != 3; seed++)
    {
        unsigned again;
        for (again = 0; again < 2; again++)
        {
            uint32_t shown;
            Stage(&g, TCG_CARD_GYM2_94, 1, seed);
            Prizes3(&g);
            g.players[0].field[0].poison = 0;
            AttachTool(&g, 0, 0, TCG_CARD_GYM1_117);
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            a.attack = 1;
            if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_ESP || g.phase != TCG_CHOICE || !g.espCoins)
                return printf("FAIL Sabrina's ESP: not asked (%s)\n", g.error), 1;
            shown = g.espBits;
            Choose(&g, again, -1);
            if (g.phase == TCG_CHOICE)
                return printf("FAIL Sabrina's ESP: still asking\n"), 1;
            if (!again && g.coinBits != shown)
                return printf("FAIL Sabrina's ESP: KEEP changed the coins\n"), 1;
            if (again)
                seen |= g.coinBits != shown ? 1 : 2;
        }
    }
    if (!(seen & 1))
        return printf("FAIL Sabrina's ESP: RE-FLIP never changed the coins\n"), 1;
    if (TcgDuel_AttachedCard(&g, 0, 0, TCG_CARD_GYM1_117) >= 0)
        return printf("FAIL Sabrina's ESP: not discarded as the turn ended\n"), 1;
    /* Charity: 10 less of the damage done; back to the hand as the turn ends */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    Prizes3(&g);
    g.players[0].field[0].poison = 0;
    AttachTool(&g, 0, 0, TCG_CARD_GYM1_99);
    base = g.players[0].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.choiceKind != TCG_CHOICE_CHARITY || g.phase != TCG_CHOICE || !g.choiceData)
        return printf("FAIL Charity: not asked (%s)\n", g.error), 1;
    {
        unsigned dealt = g.players[1].field[0].damage;
        Choose(&g, 1, -1);
        if (g.players[1].field[0].damage != dealt - 10 || g.players[0].handSize != base + 1 || g.current != 1)
            return printf("FAIL Charity: %u of %u, hand %u\n", g.players[1].field[0].damage, dealt, g.players[0].handSize), 1;
    }
    /* Narrow Gym: both Benches of 5 give one back (theirs first), then no fifth */
    Stage(&g, TCG_CARD_MACHOP, 0, 1);
    for (c = 0; c < 2; c++)
    {
        unsigned s;
        for (s = 3; s < TCG_FIELD_SIZE; s++)
        {
            struct TcgPlayer *q = &g.players[c];
            uint8_t card = q->deck[--q->deckSize];
            q->cards[card] = TCG_CARD_DIGLETT;
            q->field[s].stack[q->field[s].stackSize++] = card;
        }
    }
    if (PlayHeld(&g, TCG_CARD_GYM1_124) != TCG_OK || g.choiceKind != TCG_CHOICE_NARROW || g.choicePlayer != 1)
        return printf("FAIL Narrow Gym: not asked (%s)\n", g.error), 1;
    Choose(&g, 3, -1);
    if (g.choiceKind != TCG_CHOICE_NARROW || g.choicePlayer != 0 || g.players[1].field[3].stackSize)
        return printf("FAIL Narrow Gym: their Bench or our turn to choose\n"), 1;
    Choose(&g, 4, -1);
    if (g.phase != TCG_TURN || g.players[0].field[4].stackSize)
        return printf("FAIL Narrow Gym: our Bench\n"), 1;
    c = Relabel(&g, 0, TCG_CARD_DIGLETT);
    g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PLAY_BASIC;
    a.hand = (uint8_t)(g.players[0].handSize - 1);
    a.slot = 4;
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL Narrow Gym: a fifth Benched Pokemon\n"), 1;
    /* Misty's Psyduck's ESP: 1 heads draws, 2 heads 20 damage, 3 heads copies the Defending Pokemon's attack */
    for (seed = 1, seen = 0; seed < 200 && seen != 7; seed++)
    {
        unsigned hand, dmg;
        Stage(&g, TCG_CARD_GYM2_90, 0, seed);
        Prizes3(&g);
        for (c = 0; c < 2; c++)
            g.players[c].field[0].poison = g.players[c].field[0].burned = 0, g.players[c].field[0].condition = TCG_HEALTHY;
        hand = g.players[0].handSize, dmg = g.players[1].field[0].damage;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Misty's Psyduck: %s\n", g.error), 1;
        if (g.phase == TCG_CHOICE)
        {
            if (g.choiceKind != TCG_CHOICE_PSY_COPY)
                return printf("FAIL Misty's Psyduck: the wrong question\n"), 1;
            Choose(&g, 0, -1);
            if (g.players[1].field[0].damage <= dmg)
                return printf("FAIL Misty's Psyduck: the copied attack did nothing\n"), 1;
            seen |= 4;
        }
        else if (g.players[1].field[0].damage == dmg + 20 || (g.players[1].field[0].damage > dmg && g.psyHeads == 2))
            seen |= 2;
        else if (g.players[0].handSize == hand + 1 && g.players[1].field[0].damage == dmg)
            seen |= 1;
    }
    if (seen != 7)
        return printf("FAIL Misty's Psyduck: outcomes %u\n", seen), 1;
    printf("PASS held attacks: Koga's Ninja Trick, Vermilion City Gym (both coins), Sabrina's ESP (KEEP, RE-FLIP), Charity; Narrow Gym; Misty's Psyduck\n");
    return 0;
}
static int Choices(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    struct TcgFlow f;
    unsigned q, k, hand;
    int c, mine;
    if (!Tcg_Card(TCG_CARD_NEO4_53) || !Tcg_Card(TCG_CARD_BASEP_5))
        return printf("FAIL choices: a card isn't finished\n"), 1;
    Stage(&g, TCG_CARD_NEO4_53, 0, 1);
    g.players[1].field[0].poison = 0;
    c = Relabel(&g, 1, TCG_CARD_MACHOP);
    g.players[1].discard[g.players[1].discardSize++] = (uint8_t)c;
    mine = Relabel(&g, 0, TCG_CARD_DIGLETT);
    g.players[0].discard[g.players[0].discardSize++] = (uint8_t)mine;
    hand = g.players[1].handSize;
    memset(&a, 0, sizeof(a));
    a.type = TCG_ATTACK;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 1 || g.current != 0)
        return printf("FAIL Mysterious Wing: %s (phase %u, chooser %u)\n", g.error, g.phase, g.choicePlayer), 1;
    if (TcgFlow_Actor(&g) != 1)
        return printf("FAIL choices: the actor isn't the chooser\n"), 1;
    q = TcgFlow_Begin(&f, &g, 1, TCG_FLOW_CHOICE, 0);
    if (q != TCG_Q_PICK)
        return printf("FAIL choices: the flow asks %u (%s)\n", q, f.error), 1;
    for (k = 0; k < f.pickCount && f.pick[k] != (uint8_t)c; k++)
        ;
    if (k == f.pickCount || !f.pickOk[k])
        return printf("FAIL choices: their Machop isn't offered\n"), 1;
    {
        uint8_t chosen[1] = {(uint8_t)c};
        q = TcgFlow_Pick(&f, &g, chosen, 1);
    }
    if (q != TCG_Q_DONE || TcgDuel_Apply(&g, &f.action) != TCG_OK)
        return printf("FAIL choices: their pick: %s %s\n", f.error, g.error), 1;
    if (g.phase != TCG_CHOICE || g.choicePlayer != 0 || g.players[1].handSize != hand + 1)
        return printf("FAIL choices: then yours (phase %u, chooser %u)\n", g.phase, g.choicePlayer), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_PASS;
    if (TcgDuel_Apply(&g, &a) == TCG_OK)
        return printf("FAIL choices: another action while it waits\n"), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE; /* (none) */
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase == TCG_CHOICE || g.current != 1)
        return printf("FAIL choices: the turn didn't end (%s)\n", g.error), 1;
    /* Special Delivery: draw, then a card of the hand on top */
    Stage(&g, TCG_CARD_BASEP_5, 1, 1);
    g.players[0].field[0].poison = 0;
    memset(&a, 0, sizeof(a));
    a.type = TCG_USE_POWER;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 0)
        return printf("FAIL Special Delivery: %s\n", g.error), 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CHOOSE;
    a.selections[0] = g.players[0].hand[0];
    a.selectionCount = 1;
    if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_TURN || g.players[0].deck[g.players[0].deckSize - 1] != a.selections[0])
        return printf("FAIL Special Delivery's card: %s\n", g.error), 1;
    /* Blaine's Quiz #2: their right guess (a Pokemon) draws them 2 */
    if (Tcg_Card(TCG_CARD_GYM2_111) && Tcg_Card(TCG_CARD_NEO1_100))
    {
        int quiz, mon;
        Stage(&g, TCG_CARD_MACHOP, 0, 1);
        quiz = Relabel(&g, 0, TCG_CARD_GYM2_111);
        mon = Relabel(&g, 0, TCG_CARD_MACHOP);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)mon;
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)quiz;
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_TRAINER;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        a.selections[0] = (uint8_t)mon;
        a.selectionCount = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.phase != TCG_CHOICE || g.choicePlayer != 1)
            return printf("FAIL Blaine's Quiz #2: %s\n", g.error), 1;
        hand = g.players[1].handSize;
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = 2;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].handSize != hand + 2 || g.phase != TCG_TURN)
            return printf("FAIL Blaine's Quiz #2's guess: %s (hand %u -> %u, phase %u, mine %u)\n", g.error, hand, g.players[1].handSize, g.phase, g.players[0].handSize), 1;
        /* Double Gust: their Benched one you chose, then they choose yours */
        Stage(&g, TCG_CARD_MACHOP, 0, 1);
        c = Relabel(&g, 0, TCG_CARD_NEO1_100);
        g.players[0].hand[g.players[0].handSize++] = (uint8_t)c;
        k = g.players[1].field[2].stack[0], hand = g.players[0].field[1].stack[0];
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_TRAINER;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        a.target = 2;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[1].field[0].stack[0] != k || g.phase != TCG_CHOICE)
            return printf("FAIL Double Gust: %s\n", g.error), 1;
        memset(&a, 0, sizeof(a));
        a.type = TCG_CHOOSE;
        a.target = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK || g.players[0].field[0].stack[0] != hand)
            return printf("FAIL Double Gust's choice: %s\n", g.error), 1;
    }
    printf("PASS choices: Mysterious Wing through the flow, their pick then yours, Special Delivery, Quiz #2, Double Gust\n");
    return 0;
}
static int ScriptMechanics(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned id, big = 0, run;
    for (id = 1; id <= TCG_CARD_GB_COUNT && !big; id++)
        if (Tcg_Card(id) && Tcg_Card(id)->kind == TCG_POKEMON && Tcg_Card(id)->hp >= 100 &&
            Tcg_Card(id)->weakness != TCG_FIGHTING && Tcg_Card(id)->resistance != TCG_FIGHTING)
            big = id;
    if (!Tcg_Card(TCG_CARD_ECARD3_38) || !big)
        return printf("PASS script mechanics (Machoke not finished)\n"), 0;
    for (run = 0; run < 2; run++)
    {
        Stage(&g, TCG_CARD_ECARD3_38, 0, 11);
        g.players[1].cards[g.players[1].field[0].stack[0]] = (uint16_t)big;
        if (run == 0)
        {
            memset(&a, 0, sizeof(a));
            a.type = TCG_ATTACK;
            if (TcgDuel_Apply(&g, &a) != TCG_OK)
                return printf("FAIL Focus Energy: %s\n", g.error), 1;
            if (g.players[0].field[0].boostTurn != 5)
                return printf("FAIL Focus Energy's boost: turn %u (5)\n", g.players[0].field[0].boostTurn), 1;
            g.current = 0;
            g.turn = 5;
        }
        g.players[1].field[0].damage = 0;
        g.players[1].field[0].condition = TCG_HEALTHY;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACK;
        a.attack = 1;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL Mega Punch: %s\n", g.error), 1;
        if (g.players[1].field[0].damage != (run == 0 ? 80u : 40u))
            return printf("FAIL Mega Punch %s Focus Energy: %u\n", run ? "without" : "after", g.players[1].field[0].damage), 1;
    }
    /* noconds: an attack's Special Condition doesn't take; rt1: one more to retreat */
    Stage(&g, TCG_CARD_MACHOP, 0, 5);
    g.players[1].field[0].condImmune = (uint16_t)g.turn;
    g.players[0].field[0].retreatPlus = 1;
    g.players[0].field[0].retreatUntil = (uint16_t)g.turn;
    if (TcgDuel_RetreatCost(&g, 0) != (unsigned)Tcg_Card(TCG_CARD_MACHOP)->retreat + 1)
        return printf("FAIL rt1: Retreat Cost %u\n", TcgDuel_RetreatCost(&g, 0)), 1;
    /* Crystal Type: Celebi (Skyridge) turns Water with a Water Energy from the hand, for the turn */
    if (Tcg_Card(TCG_CARD_ECARD3_145))
    {
        struct TcgPlayer *p;
        uint8_t c;
        Stage(&g, TCG_CARD_ECARD3_145, 0, 9);
        p = &g.players[0];
        c = p->deck[--p->deckSize];
        p->cards[c] = TCG_CARD_WATER_ENERGY;
        p->hand[p->handSize++] = c;
        p->energyPlayed = 0;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACH;
        a.hand = (uint8_t)(p->handSize - 1);
        a.slot = 0;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL attaching to Crystal Celebi: %s\n", g.error), 1;
        if (TcgDuel_Type(&g, 0, 0) != TCG_WATER)
            return printf("FAIL Crystal Type: Celebi is type %u after a Water Energy\n", TcgDuel_Type(&g, 0, 0)), 1;
    }
    /* Unown [F]'s [Find]: only with Unown [F], [I], [N] and [D] on the Bench */
    {
        static const char *const letters[4] = {"Unown [F]", "Unown [I]", "Unown [N]", "Unown [D]"};
        unsigned ids[4] = {0}, k, id;
        for (k = 0; k < 4; k++)
            for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT && !ids[k]; id++)
                if (Tcg_Card(id) && !strcmp(Tcg_Card(id)->name, letters[k]) && (k || TcgPower_Id(Tcg_Card(id))))
                    ids[k] = id;
        if (ids[0] && ids[1] && ids[2] && ids[3])
        {
            struct TcgPlayer *p;
            Stage(&g, TCG_CARD_MACHOP, 0, 13);
            p = &g.players[0];
            /* their Bench: the four letters in slots 1-4 (the cards in place of whatever was there) */
            for (k = 0; k < 4; k++)
            {
                struct TcgFieldCard *f = &p->field[1 + k];
                if (!f->stackSize)
                {
                    uint8_t c = p->deck[--p->deckSize];
                    f->stack[f->stackSize++] = c;
                }
                p->cards[f->stack[0]] = (uint16_t)ids[k];
            }
            if (TcgPower_Block(&g, 0, 1))
                return printf("FAIL [Find] with the four Unown on the Bench: %s\n", TcgPower_Block(&g, 0, 1)), 1;
            p->cards[p->field[4].stack[0]] = TCG_CARD_MACHOP;
            if (!TcgPower_Block(&g, 0, 1))
                return printf("FAIL [Find] usable without Unown [D] on the Bench\n"), 1;
        }
    }
    printf("PASS script mechanics: Focus Energy's boost next turn (and only then), Retreat Cost + 1, Crystal Type, Unown letters\n");
    return 0;
}
/* Technical Machines: each finished one attached to a Pokemon it names, its attack used (TCG_ATTACK_TM), then
 * discarded as the turn ends */
static int TechnicalMachines(void)
{
    static const unsigned energy[TCG_TYPES] = {TCG_CARD_FIRE_ENERGY, TCG_CARD_GRASS_ENERGY, TCG_CARD_LIGHTNING_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_FIGHTING_ENERGY, TCG_CARD_PSYCHIC_ENERGY,
                                               TCG_CARD_WATER_ENERGY, TCG_CARD_NEO1_104, TCG_CARD_NEO1_19};
    unsigned id, used = 0, tms = 0;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *tm = Tcg_Card(id);
        unsigned holder, t, k;
        struct TcgDuel g;
        struct TcgAction a;
        int ok = 0;
        if (!tm || !(tm->flags & TCG_CF_TM))
            continue;
        tms++;
        /* (a Stage 1 Pokemon it may go on: every one names a type or none, some only an Evolved one) */
        for (holder = 1; holder <= TCG_CARD_COUNT && !ok; holder++)
        {
            const struct TcgCard *c = Tcg_Card(holder);
            if (!c || c->kind != TCG_POKEMON || c->stage != 1)
                continue;
            Stage(&g, holder, 0, 3 + holder);
            ok = TcgDuel_TmFits(&g, 0, 0, tm);
        }
        if (!ok)
            return printf("FAIL no Pokemon for %s\n", tm->name), 1;
        for (t = 0; t < TCG_TYPES; t++)
            Attach(&g, 0, 0, energy[t], 2);
        {
            /* (a card of the deck becomes the Technical Machine, in the hand) */
            struct TcgPlayer *p = &g.players[0];
            uint8_t c = p->deck[--p->deckSize];
            p->cards[c] = (uint16_t)id;
            p->hand[p->handSize++] = c;
        }
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_TRAINER;
        a.hand = (uint8_t)(g.players[0].handSize - 1);
        a.slot = 0;
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL attaching %s: %s\n", tm->name, g.error), 1;
        if (TcgDuel_Tm(&g, 0, 0, NULL) != id || !TcgDuel_CanAttack(&g, 0, TCG_ATTACK_TM))
            return printf("FAIL %s's attack can't be used once attached\n", tm->name), 1;
        /* (the attack built as a player builds it: the flow, its first choices) */
        {
            struct TcgFlow f;
            unsigned q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, TCG_ATTACK_TM), guard = 0;
            while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 16)
            {
                if (q == TCG_Q_SLOT)
                {
                    for (k = 0; k < TCG_FIELD_SIZE && !(f.slotMask >> k & 1); k++)
                        ;
                    q = TcgFlow_Slot(&f, &g, k);
                }
                else if (q == TCG_Q_PICK)
                {
                    uint8_t chosen[TCG_DECK_SIZE];
                    unsigned n = 0;
                    for (k = 0; k < f.pickCount && n < f.pickMin; k++)
                        if (f.pickOk[k])
                            chosen[n++] = f.pick[k];
                    q = TcgFlow_Pick(&f, &g, chosen, n);
                }
                else if (q == TCG_Q_MENU)
                {
                    for (k = 0; k < f.menu.count && !f.menu.ok[k]; k++)
                        ;
                    if (k == f.menu.count)
                        break;
                    q = TcgFlow_Menu(&f, &g, f.menu.value[k]);
                }
            }
            if (q != TCG_Q_DONE)
                return printf("FAIL %s's %s: the flow didn't build it (%u)\n", tm->name, tm->attacks[0].name, q), 1;
            a = f.action;
        }
        if (TcgDuel_Apply(&g, &a) != TCG_OK)
            return printf("FAIL %s's %s: %s\n", tm->name, tm->attacks[0].name, g.error), 1;
        used++;
        if (TcgDuel_Tm(&g, 0, 0, NULL))
            return printf("FAIL %s stayed attached after the turn\n", tm->name), 1;
        for (k = 0; k < g.players[0].discardSize && g.players[0].cards[g.players[0].discard[k]] != id; k++)
            ;
        if (k == g.players[0].discardSize && g.phase != TCG_FINISHED)
            return printf("FAIL %s isn't in the discard pile after the turn\n", tm->name), 1;
    }
    printf("PASS %u finished Technical Machines: attached, their attacks built through the flow and used, discarded at the end of the turn\n", used);
    (void)tms;
    return 0;
}
static int SpecialEnergy(void)
{
    struct TcgDuel g;
    struct TcgAction a;
    unsigned dealt, took;
    if (LowKick(0, 0, &dealt, &took) || dealt != 20 || took != 20)
        return printf("FAIL plain Low Kick: %u / %u\n", dealt, took), 1;
    if (LowKick(TCG_CARD_NEO1_104, 0, &dealt, &took) || dealt != 30 || took != 30)
        return printf("FAIL Darkness Energy: dealt %u (30), took %u (30)\n", dealt, took), 1;
    if (LowKick(0, TCG_CARD_NEO1_19, &dealt, &took) || dealt != 10)
        return printf("FAIL Metal Energy on the Defending Pokemon: %u (10)\n", dealt), 1;
    if (LowKick(TCG_CARD_NEO1_19, 0, &dealt, &took) || dealt != 10)
        return printf("FAIL Metal Energy on a Fighting attacker: %u (10)\n", dealt), 1;
    /* Boost Energy: not on a Basic Pokemon */
    Stage(&g, TCG_CARD_MACHOP, 0, 3);
    {
        struct TcgPlayer *p = &g.players[0];
        int c = Take(&g, 0, TCG_CARD_GRASS_ENERGY);
        uint16_t keep = p->cards[c];
        p->cards[c] = TCG_CARD_ECARD2_145;
        p->hand[p->handSize++] = (uint8_t)c;
        memset(&a, 0, sizeof(a));
        a.type = TCG_ATTACH;
        a.hand = (uint8_t)(p->handSize - 1);
        a.slot = 0;
        if (TcgDuel_Apply(&g, &a) == TCG_OK)
            return printf("FAIL Boost Energy went on a Basic Pokemon\n"), 1;
        p->cards[c] = keep;
    }
    printf("PASS special Energy: Darkness Energy's 10 more and its damage counter, Metal Energy both ways, Boost Energy\n");
    return 0;
}
/* The e-Card sets' special Energy attached through the flow: Warp Energy switches the Active with the Benched one
 * chosen, Cyclone Energy has them switch, Bounce Energy returns the basic Energy picked, Retro Energy devolves on YES;
 * Crystal Energy pays a type of the basic Energy beside it */
static int EcardEnergy(void)
{
    static const unsigned ids[4] = {TCG_CARD_ECARD2_147, TCG_CARD_ECARD3_143, TCG_CARD_ECARD3_142, TCG_CARD_ECARD3_144};
    unsigned e;
    for (e = 0; e < 4; e++)
    {
        struct TcgDuel g;
        struct TcgFlow f;
        struct TcgPlayer *p;
        unsigned q, guard = 0, hand, before, machop;
        int c;
        if (!Tcg_Card(ids[e]))
            continue;
        Stage(&g, TCG_CARD_MACHOP, 0, 7);
        p = &g.players[0];
        if (e == 3)
        {
            /* (Retro Energy: a Machoke on Machop, damaged) */
            int m = Take(&g, 0, TCG_CARD_MACHOKE);
            if (m < 0)
                continue;
            p->field[0].stack[p->field[0].stackSize++] = (uint8_t)m;
            p->field[0].damage = 30;
        }
        c = Take(&g, 0, TCG_CARD_GRASS_ENERGY);
        if (c < 0)
            return printf("FAIL e-Card Energy: no card to stand in\n"), 1;
        p->cards[c] = (uint16_t)ids[e];
        p->hand[p->handSize++] = (uint8_t)c;
        hand = p->handSize;
        machop = p->field[0].stack[0];
        before = p->field[0].attachedSize;
        q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, p->handSize - 1u);
        while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 8)
        {
            if (q == TCG_Q_SLOT)
                q = TcgFlow_Slot(&f, &g, guard == 1 ? 0u : 2u); /* (the Active, then Diglett on the Bench) */
            else if (q == TCG_Q_PICK)
            {
                uint8_t chosen[1];
                unsigned k;
                for (k = 0; k < f.pickCount && !f.pickOk[k]; k++)
                    ;
                chosen[0] = f.pick[k];
                q = TcgFlow_Pick(&f, &g, chosen, 1);
            }
            else if (q == TCG_Q_MENU)
                q = TcgFlow_Menu(&f, &g, 1);
        }
        if (q != TCG_Q_DONE || TcgDuel_Apply(&g, &f.action) != TCG_OK)
            return printf("FAIL %s through the flow: %s %s\n", Tcg_Card(ids[e])->name, f.error, g.error), 1;
        assert(TcgDuel_Conserved(&g));
        if (e == 0 && p->field[0].stack[0] == machop)
            return printf("FAIL Warp Energy: the Active didn't switch\n"), 1;
        if (e == 1 && (g.phase != TCG_PROMOTION || g.promotePlayer != 1))
            return printf("FAIL Cyclone Energy: they weren't asked to switch\n"), 1;
        if (e == 2 && (p->handSize != hand || p->field[0].attachedSize != before))
            return printf("FAIL Bounce Energy: hand %u (%u), attached %u (%u)\n", p->handSize, hand,
                          p->field[0].attachedSize, before), 1;
        if (e == 3 && (p->field[0].stackSize != 1 || p->field[0].damage != 10))
            return printf("FAIL Retro Energy: stack %u, damage %u\n", p->field[0].stackSize, p->field[0].damage), 1;
    }
    /* Crystal Energy: with a Fighting Energy beside it, it pays a Fighting cost; alone, only Colorless */
    if (Tcg_Card(TCG_CARD_ECARD2_146))
    {
        struct TcgDuel g;
        struct TcgFieldCard *f;
        int c;
        Stage(&g, TCG_CARD_MACHOP, 0, 9);
        f = &g.players[0].field[0];
        while (f->attachedSize)
            g.players[0].discard[g.players[0].discardSize++] = f->attached[--f->attachedSize];
        c = Take(&g, 0, TCG_CARD_GRASS_ENERGY);
        g.players[0].cards[c] = TCG_CARD_ECARD2_146;
        f->attached[f->attachedSize++] = (uint8_t)c;
        if (TcgDuel_CanAttack(&g, 0, 0) && Tcg_Card(TCG_CARD_MACHOP)->attacks[0].cost[TCG_FIGHTING])
            return printf("FAIL Crystal Energy alone paid a Fighting cost\n"), 1;
        c = Take(&g, 0, TCG_CARD_FIGHTING_ENERGY);
        f->attached[f->attachedSize++] = (uint8_t)c;
        if (Tcg_Card(TCG_CARD_MACHOP)->attacks[0].cost[TCG_FIGHTING] == 1 &&
            Tcg_Card(TCG_CARD_MACHOP)->attacks[0].cost[TCG_COLORLESS] <= 1 && !TcgDuel_CanAttack(&g, 0, 0))
            return printf("FAIL Crystal Energy with a Fighting Energy\n"), 1;
    }
    printf("PASS e-Card Energy through the flow: Warp, Cyclone, Bounce, Retro; Crystal Energy's types\n");
    return 0;
}
/* Every finished regional Trainer played through the flow (the board's and the phone's questions, answered with the
 * first choices; with the most cards picked on even seeds) on staged boards: the rules must take what the flow built;
 * then the duel goes on with the AI. */
static void Give(struct TcgDuel *g, unsigned who, unsigned id, int toDiscard)
{
    struct TcgPlayer *p = &g->players[who];
    int c = Take(g, who, id);
    if (c < 0)
        return;
    if (toDiscard)
        p->discard[p->discardSize++] = (uint8_t)c;
    else
        p->hand[p->handSize++] = (uint8_t)c;
}
static int PlayTrainers(void)
{
    unsigned id, played = 0, refused = 0, tried = 0;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        uint32_t seed;
        if (!c || c->kind != TCG_TRAINER)
            continue;
        tried++;
        for (seed = 1; seed <= 8; seed++)
        {
            struct TcgDuel g;
            struct TcgFlow f;
            unsigned q, k, guard = 0;
            int at = -1;
            Stage(&g, TCG_CARD_MACHOP, 0, seed);
            {
                struct TcgPlayer *p = &g.players[0];
                /* the card itself (in place of a Potion), cards to pick in the hand, the discard pile and the deck */
                int pot = Take(&g, 0, TCG_CARD_BILL);
                if (pot < 0)
                    continue;
                p->cards[pot] = (uint16_t)id;
                p->hand[p->handSize++] = (uint8_t)pot;
                at = (int)p->handSize - 1;
                Give(&g, 0, TCG_CARD_DIGLETT, seed & 1);
                Give(&g, 0, TCG_CARD_FIRE_ENERGY, 0);
                Give(&g, 0, TCG_CARD_FIRE_ENERGY, 0);
                Give(&g, 0, TCG_CARD_WATER_ENERGY, 1);
                Give(&g, 0, TCG_CARD_PSYCHIC_ENERGY, 1);
                Give(&g, 0, TCG_CARD_POTION, 1);
                if (seed % 3 == 0)
                    p->field[1].damage = 30;
                if (seed == 5)
                    g.players[0].energyPlayed = 1;
            }
            if (!TcgFlow_TrainerHasUse(&g, 0, id))
            {
                refused++;
                continue;
            }
            q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_HAND, (unsigned)at);
            while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 16)
            {
                if (q == TCG_Q_SLOT)
                {
                    for (k = 0; k < TCG_FIELD_SIZE && !(f.slotMask >> k & 1); k++)
                        ;
                    q = TcgFlow_Slot(&f, &g, k);
                }
                else if (q == TCG_Q_PICK)
                {
                    uint8_t chosen[TCG_DECK_SIZE];
                    unsigned n = 0, want = seed % 2 ? f.pickMin : f.pickMax;
                    for (k = 0; k < f.pickCount && n < want; k++)
                        if (f.pickOk[k])
                            chosen[n++] = f.pick[k];
                    if (n < f.pickMin)
                        break;
                    q = TcgFlow_Pick(&f, &g, chosen, n);
                }
                else if (q == TCG_Q_MENU)
                {
                    for (k = 0; k < f.menu.count && !f.menu.ok[k]; k++)
                        ;
                    if (k == f.menu.count)
                        break;
                    q = TcgFlow_Menu(&f, &g, f.menu.value[k]);
                }
            }
            if (q != TCG_Q_DONE)
            {
                refused++;
                continue;
            }
            {
                int result = TcgDuel_Apply(&g, &f.action);
                if (result != TCG_OK)
                {
                    printf("FAIL %s (seed %u): the flow's action was refused: %d %s\n", c->name, seed, result, g.error);
                    return 1;
                }
                assert(TcgDuel_Conserved(&g));
                played++;
                for (k = 0; k < 24 && g.phase != TCG_FINISHED; k++)
                {
                    struct TcgAction next;
                    memset(&next, 0, sizeof(next));
                    if (!TcgDuel_AiAction(&g, &next))
                        break;
                    if ((result = TcgDuel_Apply(&g, &next)) != TCG_OK)
                    {
                        printf("FAIL after %s (seed %u): AI action %u refused: %s\n", c->name, seed, next.type, g.error);
                        return 1;
                    }
                    assert(TcgDuel_Conserved(&g));
                }
            }
        }
    }
    printf("PASS %u finished regional Trainers through the flow: %u played, %u not playable on that board\n", tried,
           played, refused);
    return 0;
}
/* Every finished regional used power through the flow (as the Active, then on the Bench), first choices answered */
static int UsePowers(void)
{
    unsigned id, cards = 0, used = 0, blocked = 0;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        unsigned power = TcgPower_Id(c), where;
        uint32_t seed;
        if (!c || power < TCG_POWER_REGION_FIRST || !TcgPower_IsUsed(power))
            continue;
        cards++;
        for (where = 0; where < 2; where++)
            for (seed = 1; seed <= 6; seed++)
            {
                struct TcgDuel g;
                struct TcgFlow f;
                unsigned q, k, slot = where ? 3 : 0, guard = 0;
                Stage(&g, where ? TCG_CARD_MACHOP : id, 0, seed);
                if (where)
                {
                    /* (a card of the deck becomes it, on the Bench) */
                    int card = Take(&g, 0, TCG_CARD_WATER_ENERGY);
                    assert(card >= 0);
                    g.players[0].cards[card] = (uint16_t)id;
                    g.players[0].field[3].stack[g.players[0].field[3].stackSize++] = (uint8_t)card;
                }
                /* (cards for its choices: Energy in the hand and the discard pile, damage about) */
                Give(&g, 0, TCG_CARD_LIGHTNING_ENERGY, 1);
                Give(&g, 0, TCG_CARD_FIRE_ENERGY, 0);
                Attach(&g, 0, 1, TCG_CARD_LIGHTNING_ENERGY, 1);
                g.players[0].field[1].damage = 20;
                if (!g.players[0].field[slot].stackSize || TcgPower_Block(&g, 0, slot))
                {
                    blocked++;
                    continue;
                }
                q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_POWER, slot);
                while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 16)
                {
                    if (q == TCG_Q_SLOT)
                    {
                        for (k = 0; k < TCG_FIELD_SIZE && !(f.slotMask >> k & 1); k++)
                            ;
                        q = TcgFlow_Slot(&f, &g, k);
                    }
                    else if (q == TCG_Q_PICK)
                    {
                        uint8_t chosen[TCG_DECK_SIZE];
                        unsigned n = 0;
                        for (k = 0; k < f.pickCount && n < f.pickMax && (n < f.pickMin || n < 1); k++)
                            if (f.pickOk[k])
                                chosen[n++] = f.pick[k];
                        q = TcgFlow_Pick(&f, &g, chosen, n);
                    }
                    else
                        break;
                }
                if (q != TCG_Q_DONE)
                {
                    blocked++;
                    continue;
                }
                {
                    int result = TcgDuel_Apply(&g, &f.action);
                    if (result != TCG_OK)
                        return printf("FAIL %s's %s (seed %u, slot %u): %s\n", c->name, TcgPower_Name(power), seed,
                                      slot, g.error), 1;
                    assert(TcgDuel_Conserved(&g));
                    used++;
                    for (k = 0; k < 16 && g.phase != TCG_FINISHED; k++)
                    {
                        struct TcgAction next;
                        memset(&next, 0, sizeof(next));
                        if (!TcgDuel_AiAction(&g, &next))
                            break;
                        if (TcgDuel_Apply(&g, &next) != TCG_OK)
                            return printf("FAIL after %s's power: AI action %u refused: %s\n", c->name, next.type,
                                          g.error), 1;
                        assert(TcgDuel_Conserved(&g));
                    }
                }
            }
    }
    printf("PASS %u finished regional cards with used powers: %u uses through the flow, %u not usable there\n", cards,
           used, blocked);
    return 0;
}
/* Every finished regional attack built through the flow (the board's and the phone's questions: slots, picks and menus
 * answered with the first choices, the most cards on even seeds, a menu's choices in turn): the rules must take it */
static int AttackFlows(void)
{
    unsigned id, attack, built = 0, skipped = 0;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
    {
        const struct TcgCard *c = Tcg_Card(id);
        if (!c || c->kind != TCG_POKEMON)
            continue;
        for (attack = 0; attack < TCG_ATTACKS; attack++)
        {
            uint32_t seed;
            if (!c->attacks[attack].name[0] || c->attacks[attack].category == TCG_POKEMON_POWER)
                continue;
            for (seed = 1; seed <= 4; seed++)
            {
                struct TcgDuel g;
                struct TcgFlow f;
                unsigned q, k, guard = 0;
                int result;
                Stage(&g, id, attack, seed);
                Give(&g, 0, TCG_CARD_GRASS_ENERGY, 0);
                Give(&g, 0, TCG_CARD_FIRE_ENERGY, 1);
                Give(&g, 0, TCG_CARD_DIGLETT, 1);
                if (seed & 1)
                    g.players[1].field[1].damage = 10;
                q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_ATTACK, attack);
                while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 16)
                {
                    if (q == TCG_Q_SLOT)
                    {
                        for (k = 0; k < TCG_FIELD_SIZE && !(f.slotMask >> k & 1); k++)
                            ;
                        q = TcgFlow_Slot(&f, &g, k);
                    }
                    else if (q == TCG_Q_PICK)
                    {
                        uint8_t chosen[TCG_DECK_SIZE];
                        unsigned n = 0, want = seed % 2 ? f.pickMin : f.pickMax;
                        for (k = 0; k < f.pickCount && n < want; k++)
                            if (f.pickOk[k])
                                chosen[n++] = f.pick[k];
                        if (n < f.pickMin)
                            break;
                        q = TcgFlow_Pick(&f, &g, chosen, n);
                    }
                    else if (q == TCG_Q_MENU)
                    {
                        unsigned tries;
                        for (tries = 0, k = seed % f.menu.count; tries < f.menu.count && !f.menu.ok[k]; tries++)
                            k = (k + 1) % f.menu.count;
                        if (!f.menu.ok[k])
                            break;
                        q = TcgFlow_Menu(&f, &g, f.menu.value[k]);
                    }
                }
                if (q != TCG_Q_DONE)
                {
                    skipped++;
                    continue;
                }
                result = TcgDuel_Apply(&g, &f.action);
                if (result != TCG_OK)
                {
                    printf("FAIL %s's %s (seed %u): the flow's attack was refused: %d %s\n", c->name,
                           c->attacks[attack].name, seed, result, g.error);
                    return 1;
                }
                assert(TcgDuel_Conserved(&g));
                built++;
            }
        }
    }
    printf("PASS finished regional attacks through the flow: %u built and taken, %u not usable there\n", built, skipped);
    return 0;
}
/* Every Stadium a player uses during their turn, used through the flow on a staged board (first choices), twice: once a
 * turn but Celadon / Saffron City Gym; then the duel goes on with the AI */
static int Relabel(struct TcgDuel *g, unsigned who, unsigned id)
{
    int c = Take(g, who, TCG_CARD_FIRE_ENERGY);
    if (c >= 0)
        g->players[who].cards[c] = (uint16_t)id;
    return c;
}
static unsigned Named(const char *word)
{
    unsigned id;
    for (id = TCG_CARD_GB_COUNT + 1; id <= TCG_CARD_COUNT; id++)
        if (Tcg_Card(id) && Tcg_Card(id)->kind == TCG_POKEMON && !Tcg_Card(id)->stage && strstr(Tcg_Card(id)->name, word))
            return id;
    return 0;
}
static int UseStadiums(void)
{
    static const unsigned stadiums[] = {TCG_CARD_GYM1_107, TCG_CARD_GYM2_114, TCG_CARD_GYM2_122, TCG_CARD_NEO3_61,
                                        TCG_CARD_NEO4_95, TCG_CARD_NEO4_99, TCG_CARD_NEO4_100, TCG_CARD_ECARD2_118,
                                        TCG_CARD_ECARD2_138, TCG_CARD_ECARD2_139, TCG_CARD_ECARD3_119,
                                        TCG_CARD_ECARD3_137, TCG_CARD_ECARD3_141};
    unsigned i, used = 0, again = 0;
    for (i = 0; i < sizeof(stadiums) / sizeof(stadiums[0]); i++)
    {
        unsigned id = stadiums[i], active = TCG_CARD_MACHOP;
        uint32_t seed;
        if (!Tcg_Card(id))
            continue;
        if (id == TCG_CARD_GYM1_107)
            active = Named("Erika");
        if (id == TCG_CARD_GYM2_114)
            active = Named("Koga");
        if (id == TCG_CARD_GYM2_122)
            active = Named("Sabrina");
        if (!active)
            continue;
        for (seed = 1; seed <= 6; seed++)
        {
            struct TcgDuel g;
            struct TcgFlow f;
            struct TcgPlayer *p;
            unsigned q, k, guard = 0, round;
            int c;
            Stage(&g, active, 0, seed);
            p = &g.players[0];
            if ((c = Relabel(&g, seed & 1, id)) >= 0)
                g.players[seed & 1].stadium = (uint8_t)(c + 1);
            if ((c = Relabel(&g, 0, TCG_CARD_MACHOKE)) >= 0)
                p->field[1].stack[p->field[1].stackSize++] = (uint8_t)c;
            if ((c = Relabel(&g, 0, TCG_CARD_OMANYTE)) >= 0)
                p->discard[p->discardSize++] = (uint8_t)c;
            if ((c = Relabel(&g, 0, TCG_CARD_MACHOKE)) >= 0)
                p->hand[p->handSize++] = (uint8_t)c;
            Give(&g, 0, TCG_CARD_FIGHTING_ENERGY, 0);
            Give(&g, 0, TCG_CARD_FIGHTING_ENERGY, 1);
            p->field[0].condition = TCG_ASLEEP;
            for (round = 0; round < 2; round++)
            {
                q = TcgFlow_Begin(&f, &g, 0, TCG_FLOW_STADIUM, 0);
                guard = 0;
                while (q != TCG_Q_DONE && q != TCG_Q_FAIL && guard++ < 16)
                {
                    if (q == TCG_Q_SLOT)
                    {
                        for (k = 0; k < TCG_FIELD_SIZE && !(f.slotMask >> k & 1); k++)
                            ;
                        q = TcgFlow_Slot(&f, &g, k);
                    }
                    else if (q == TCG_Q_PICK)
                    {
                        uint8_t chosen[TCG_DECK_SIZE];
                        unsigned n = 0, want = seed % 2 ? f.pickMin : f.pickMax;
                        for (k = 0; k < f.pickCount && n < want; k++)
                            if (f.pickOk[k])
                                chosen[n++] = f.pick[k];
                        if (n < f.pickMin)
                            break;
                        q = TcgFlow_Pick(&f, &g, chosen, n);
                    }
                    else
                        break;
                }
                if (q != TCG_Q_DONE)
                {
                    if (!round)
                    {
                        printf("FAIL %s (seed %u): the flow didn't finish: %s\n", Tcg_Card(id)->name, seed, f.error);
                        return 1;
                    }
                    break;
                }
                {
                    int result = TcgDuel_Apply(&g, &f.action);
                    if (result != TCG_OK)
                    {
                        printf("FAIL %s (seed %u): the flow's action was refused: %d %s\n", Tcg_Card(id)->name, seed,
                               result, g.error);
                        return 1;
                    }
                    assert(TcgDuel_Conserved(&g));
                    if (round)
                        again++;
                    else
                        used++;
                    if (g.phase != TCG_TURN)
                        break;
                }
            }
            if (again && id != TCG_CARD_GYM1_107 && id != TCG_CARD_GYM2_122 && !TcgDuel_StadiumBlock(&g, 0))
            {
                printf("FAIL %s: used twice in a turn\n", Tcg_Card(id)->name);
                return 1;
            }
            for (k = 0; k < 24 && g.phase != TCG_FINISHED; k++)
            {
                struct TcgAction next;
                int result;
                memset(&next, 0, sizeof(next));
                if (!TcgDuel_AiAction(&g, &next))
                    break;
                if ((result = TcgDuel_Apply(&g, &next)) != TCG_OK)
                {
                    printf("FAIL after %s (seed %u): AI action %u refused: %s\n", Tcg_Card(id)->name, seed, next.type,
                           g.error);
                    return 1;
                }
                assert(TcgDuel_Conserved(&g));
            }
        }
    }
    printf("PASS Stadiums used through the flow: %u uses, %u second uses (Celadon / Saffron City Gym)\n", used, again);
    return 0;
}
int main(void)
{
    struct TcgDuel g;
    unsigned id, attack, used = 0, ok = 0, steps = 0, ready = 0;
    uint32_t seed;
    if (getenv("TCG_ONLY_LAST"))
        return HeldAttacks() || LastRules(); /* (the newest rules alone, while translating) */
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
                {
                    /* (every option in turn, not only the AI's) */
                    const char *labels[TCG_MAX_OPTIONS];
                    uint8_t okay[TCG_MAX_OPTIONS];
                    unsigned n = TcgDuel_AttackOptions(&g, 0, at, labels, okay);
                    if (n && okay[seed % n])
                    {
                        a.option = (uint8_t)(seed % n);
                        if (TcgDuel_AttackAsk(at) == TCG_ASK_ORDER_TOP3)
                            a.selectionCount = 0;
                    }
                }
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
    if (SpecialEnergy() || EcardEnergy() || ScriptMechanics() || Anchors() || Guards() || MovePowers() || LookPowers() || SwitchesAndFire() || BorrowedAttacks() || Copies() || PowersOfFour() || Choices() || Peeks() || MoreChoices() || MoreRules() || EvenMoreRules() || QuizzesAndPlays() || LastRules() || HeldAttacks() || TechnicalMachines() || Babies())
        return 1;
    if (PlayTrainers() || UsePowers() || UseStadiums() || AttackFlows())
        return 1;
    /* Decks of them, each type against the others, to the end */
    {
        static const unsigned types[] = {TCG_FIRE, TCG_GRASS, TCG_LIGHTNING, TCG_WATER, TCG_FIGHTING, TCG_PSYCHIC,
                                         TCG_DARKNESS, TCG_METAL};
        unsigned ti, ui, t, u, games = 0, finished = 0, actions = 0, skip;
        for (skip = 0; skip < 3; skip++)
            for (ti = 0; ti < 8; ti++)
                for (ui = 0; ui < 8; ui++)
                {
                    t = types[ti], u = types[ui];
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
