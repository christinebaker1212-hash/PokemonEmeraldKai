#include "tcg/card_ids.h"
#include "tcg/duel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void Deck(uint16_t *cards)
{
    unsigned i;
    for (i = 0; i < 60; i++)
        cards[i] = i < 4 ? TCG_CARD_CHARMANDER : i < 8 ? TCG_CARD_BILL : TCG_CARD_FIRE_ENERGY;
}
static int FindHand(struct TcgDuel *g, unsigned who, unsigned kind)
{
    unsigned i;
    struct TcgPlayer *p = &g->players[who];
    for (i = 0; i < p->handSize; i++)
        if (Tcg_Card(p->cards[p->hand[i]])->kind == kind)
            return i;
    return -1;
}
static void Setup(struct TcgDuel *g)
{
    unsigned who;
    struct TcgAction a = {0};
    for (who = 0; who < 2; who++)
    {
        assert(g->current == who);
        a.type = TCG_PLAY_BASIC;
        a.hand = FindHand(g, who, TCG_POKEMON);
        a.slot = 0;
        assert(TcgDuel_Apply(g, &a) == TCG_OK);
        a.type = TCG_FINISH_SETUP;
        assert(TcgDuel_Apply(g, &a) == TCG_OK);
    }
    assert(g->phase == TCG_TURN && g->turn == 1);
    assert(TcgDuel_Conserved(g));
}
static unsigned GiveFromDeck(struct TcgDuel *g, unsigned who, unsigned id)
{
    unsigned i;
    struct TcgPlayer *p = &g->players[who];
    for (i = 0; i < p->deckSize; i++)
        if (p->cards[p->deck[i]] == id)
        {
            p->hand[p->handSize++] = TcgPile_Remove(p->deck, &p->deckSize, i);
            assert(TcgDuel_Conserved(g));
            return p->handSize - 1;
        }
    assert(!"Test fixture card not available in deck");
    return 0;
}
int main(void)
{
    struct TcgDuel g, before;
    struct TcgAction a = {0};
    uint16_t cards[60];
    unsigned player, n, rounds;
    Deck(cards);
    assert(TcgDuel_Init(&g, cards, cards, 123, 6));
    assert(TcgDuel_Conserved(&g));
    Setup(&g);
    player = g.current;
    a.type = TCG_ATTACH;
    a.slot = 0;
    a.hand = GiveFromDeck(&g, player, TCG_CARD_FIRE_ENERGY);
    assert(TcgDuel_Apply(&g, &a) == TCG_OK);
    assert(TcgDuel_Energy(&g, player, 0, TCG_FIRE) == 1);
    a.hand = GiveFromDeck(&g, player, TCG_CARD_FIRE_ENERGY);
    before = g;
    assert(TcgDuel_Apply(&g, &a) == TCG_INVALID);
    assert(!memcmp(g.players, before.players, sizeof(g.players)));
    assert(g.rng == before.rng);
    a.type = TCG_PLAY_TRAINER;
    a.hand = GiveFromDeck(&g, player, TCG_CARD_BILL);
    n = g.players[player].handSize;
    assert(TcgDuel_Apply(&g, &a) == TCG_OK);
    assert(g.players[player].handSize == n + 1);
    assert(TcgDuel_Conserved(&g));
    a.type = TCG_ATTACK;
    a.attack = 0;
    n = g.players[player ^ 1].field[0].damage;
    assert(TcgDuel_Apply(&g, &a) == TCG_OK);
    assert(g.players[player ^ 1].field[0].damage == n + 10);
    assert(g.current == (player ^ 1));
    puts("PASS setup, energy limits, transactional invalid actions, Bill and Scratch");
    /* Play a real deterministic rules-only duel to completion; every action
     * must preserve all sixty physical cards for both players. */
    for (rounds = 0; rounds < 200 && g.phase != TCG_FINISHED; rounds++)
    {
        int hand;
        memset(&a, 0, sizeof(a));
        if (g.phase == TCG_PROMOTION)
        {
            for (n = 1; n < TCG_FIELD_SIZE && !g.players[g.promotePlayer].field[n].stackSize; n++)
                ;
            a.type = TCG_PROMOTE;
            a.slot = n;
            assert(TcgDuel_Apply(&g, &a) == TCG_OK);
            continue;
        }
        for (n = 1; n < TCG_FIELD_SIZE; n++)
        {
            hand = FindHand(&g, g.current, TCG_POKEMON);
            if (hand < 0)
                break;
            if (!g.players[g.current].field[n].stackSize)
            {
                a.type = TCG_PLAY_BASIC;
                a.slot = n;
                a.hand = hand;
                assert(TcgDuel_Apply(&g, &a) == TCG_OK);
            }
        }
        hand = FindHand(&g, g.current, TCG_ENERGY);
        if (hand >= 0 && !g.players[g.current].energyPlayed)
        {
            a.type = TCG_ATTACH;
            a.slot = 0;
            a.hand = hand;
            assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        }
        a.type = TcgDuel_CanAttack(&g, g.current, 0) ? TCG_ATTACK : TCG_PASS;
        a.attack = 0;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        assert(TcgDuel_Conserved(&g));
    }
    assert(g.phase == TCG_FINISHED && g.winner < 2);
    puts("PASS complete duel, Knock Outs, prizes, promotion and card conservation");
    /* Pure rule state and RNG are separate from Emerald's global random state. */
    assert(TcgDuel_Init(&g, cards, cards, 456, 6));
    assert(TcgDuel_Init(&before, cards, cards, 456, 6));
    assert(!memcmp(&g, &before, sizeof(g)));
    puts("PASS deterministic independent rules state");
    /* Retreat is once per turn, and every action leaves presentation events. */
    {
        uint32_t seq;
        unsigned found = 0, i;
        assert(TcgDuel_Init(&g, cards, cards, 789, 6));
        Setup(&g);
        player = g.current;
        memset(&a, 0, sizeof(a));
        a.type = TCG_PLAY_BASIC;
        a.slot = 1;
        a.hand = GiveFromDeck(&g, player, TCG_CARD_CHARMANDER);
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        for (i = 0; i < 2; i++)
        {
            a.type = TCG_ATTACH;
            a.slot = 0;
            a.hand = GiveFromDeck(&g, player, TCG_CARD_FIRE_ENERGY);
            g.players[player].energyPlayed = 0;
            assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        }
        seq = g.eventSeq;
        memset(&a, 0, sizeof(a));
        a.type = TCG_RETREAT;
        a.slot = 1;
        a.selections[0] = g.players[player].field[0].attached[0];
        a.selectionCount = 1;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        assert(TcgDuel_GetEvent(&g, seq)->type == TCG_EV_RETREAT);
        a.selections[0] = g.players[player].field[1].attached[0];
        assert(TcgDuel_Apply(&g, &a) == TCG_INVALID);
        assert(strstr(g.error, "once"));
        memset(&a, 0, sizeof(a));
        a.type = TCG_PASS;
        seq = g.eventSeq;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
        for (; seq < g.eventSeq; seq++)
            found |= 1u << TcgDuel_GetEvent(&g, seq)->type;
        assert(found & (1u << TCG_EV_TURN) && found & (1u << TCG_EV_DRAW));
        assert(!g.players[g.current].retreated);
    }
    puts("PASS retreat once per turn and presentation events");
    return 0;
}
