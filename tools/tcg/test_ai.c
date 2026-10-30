#include "tcg/duel.h"
#include "tcg/card_ids.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void Deck(uint16_t *cards)
{
    unsigned i;for(i=0;i<60;i++)cards[i]=i<4?TCG_CARD_CHARMANDER:i<8?TCG_CARD_BILL:TCG_CARD_FIRE_ENERGY;
}
static unsigned Take(struct TcgPlayer *p, unsigned physical)
{
    unsigned i;
    for (i = 0; i < p->deckSize; i++)
        if (p->deck[i] == physical)
            return TcgPile_Remove(p->deck, &p->deckSize, i);
    assert(0);
    return 0;
}
/* A lone Active Charmander with Energy enough for both its attacks, a Fire Energy in the hand (and Charmeleon) */
static void LoneCharmander(struct TcgDuel *g, int evolution)
{
    unsigned who, i;
    memset(g, 0, sizeof(*g));
    g->rng = 99;
    g->phase = TCG_TURN;
    g->turn = 5;
    g->winner = TCG_NONE;
    g->prizeCount = 6;
    for (who = 0; who < 2; who++)
    {
        struct TcgPlayer *p = &g->players[who];
        p->turnsTaken = 3;
        for (i = 0; i < 60; i++)
        {
            p->deck[p->deckSize++] = i;
            p->cards[i] = TCG_CARD_FIRE_ENERGY;
        }
        p->cards[0] = TCG_CARD_CHARMANDER;
        p->cards[1] = TCG_CARD_CHARMELEON;
        p->field[0].stack[p->field[0].stackSize++] = Take(p, 0);
        p->field[0].enteredTurn = 1;
        for (i = 50; i < 56; i++)
            p->prizes[p->prizeSize++] = Take(p, i);
        if (who == 0)
        {
            p->field[0].attached[p->field[0].attachedSize++] = Take(p, 10);
            p->field[0].attached[p->field[0].attachedSize++] = Take(p, 11);
            p->hand[p->handSize++] = Take(p, 12);
            if (evolution)
                p->hand[p->handSize++] = Take(p, 1);
        }
    }
    assert(TcgDuel_Conserved(g));
}
int main(void)
{
    struct TcgDuel g,hidden,unchanged;struct TcgAction a,b;uint16_t cards[60];unsigned seed,steps;
    Deck(cards);
    for(seed=1;seed<=20;seed++){
        assert(TcgDuel_Init(&g,cards,cards,seed,6));
        for(steps=0;steps<2000 && g.phase!=TCG_FINISHED;steps++){
            unsigned other=g.current^1;
            unchanged=g;
            assert(TcgDuel_AiAction(&g,&a));
            assert(!memcmp(&g,&unchanged,sizeof(g)));
            hidden=g;
            if(hidden.players[other].handSize && hidden.players[other].deckSize){
                unsigned tmp=hidden.players[other].hand[0];
                hidden.players[other].hand[0]=hidden.players[other].deck[0];hidden.players[other].deck[0]=tmp;
            }
            assert(TcgDuel_AiAction(&hidden,&b));
            assert(!memcmp(&a,&b,sizeof(a)));
            assert(TcgDuel_Apply(&g,&a)==TCG_OK);
            assert(TcgDuel_Conserved(&g));
        }
        assert(g.phase==TCG_FINISHED && g.winner<2);
    }
    /* Energy that brings no attack closer stays in the hand; for the Evolution in the hand it's attached */
    LoneCharmander(&g, 0);
    assert(TcgDuel_AiAction(&g, &a));
    assert(a.type != TCG_ATTACH);
    LoneCharmander(&g, 1);
    for (steps = 0; steps < 4; steps++)
    {
        assert(TcgDuel_AiAction(&g, &a));
        if (a.type == TCG_ATTACH || a.type == TCG_ATTACK || a.type == TCG_PASS)
            break;
        assert(TcgDuel_Apply(&g, &a) == TCG_OK);
    }
    assert(a.type == TCG_ATTACH);
    puts("PASS energy kept back when no attack needs it, attached for the Evolution in the hand");
    puts("PASS 20 complete AI duels, action legality, immutable decisions and hidden-information isolation");
    return 0;
}
