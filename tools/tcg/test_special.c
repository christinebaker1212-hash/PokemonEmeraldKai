/* Every special trainer's deck (src/tcg/tcg_decks.c) is legal and plays a whole
 * AI duel against each starter deck without stalling or losing a card. */
#include "mp_version.h"
#include "tcg/duel.h"
#include "tcg/npc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    unsigned d, s, promos = 0;
    char reason[128];
    assert(gTcgSpecialDeckCount == TCG_SPECIAL_COUNT);
    for (d = 0; d < gTcgSpecialDeckCount; d++)
    {
        assert(Tcg_DeckValidate(gTcgSpecialDecks[d].cards, NULL, reason, sizeof(reason)));
        for (s = 0; s < 3; s++)
        {
            static const char *starters[] = {"CharmanderAndFriends", "SquirtleAndFriends", "BulbasaurAndFriends"};
            const uint16_t *mine = NULL;
            struct TcgDuel g;
            struct TcgAction a;
            unsigned i, steps = 0;
            for (i = 0; i < gTcgDeckDefinitionCount; i++)
                if (!strcmp(gTcgDeckDefinitions[i].name, starters[s]))
                    mine = gTcgDeckDefinitions[i].cards;
            assert(mine && TcgDuel_Init(&g, mine, gTcgSpecialDecks[d].cards, 1000 + d * 7 + s, 6));
            while (g.phase != TCG_FINISHED && steps++ < 3000)
            {
                if (!TcgDuel_AiAction(&g, &a) || TcgDuel_Apply(&g, &a) != TCG_OK)
                {
                    memset(&a, 0, sizeof(a));
                    a.type = g.phase == TCG_SETUP ? TCG_FINISH_SETUP : TCG_PASS;
                    if (TcgDuel_Apply(&g, &a) != TCG_OK)
                        break;
                }
                assert(TcgDuel_Conserved(&g));
            }
            assert(g.phase == TCG_FINISHED);
        }
    }
    for (d = 0; d < TCG_SPECIAL_COUNT; d++)
        if (gTcgSpecialPromos[d])
        {
            unsigned k, copies = 0, e;
            const struct TcgCard *c = Tcg_Card(gTcgSpecialPromos[d]);
            assert(c && c->set == 4);
            for (k = 0; k < TCG_DECK_SIZE; k++)
                copies += gTcgSpecialDecks[d].cards[k] == gTcgSpecialPromos[d];
            assert(copies == 1);
            for (e = 0; e < d; e++)
                assert(gTcgSpecialPromos[e] != gTcgSpecialPromos[d]); /* each promo has one holder */
            promos++;
        }
    assert(promos == 15);
    printf("PASS %u special decks: legal, %u promo holders, every one plays to the end against each starter\n",
           gTcgSpecialDeckCount, promos);
    return 0;
}
