#include "tcg/duel.h"
#include "tcg/card_ids.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void Deck(uint16_t *cards)
{
    unsigned i;for(i=0;i<60;i++)cards[i]=i<4?TCG_CARD_CHARMANDER:i<8?TCG_CARD_BILL:TCG_CARD_FIRE_ENERGY;
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
    puts("PASS 20 complete AI duels, action legality, immutable decisions and hidden-information isolation");
    return 0;
}
