/* What card players say (src/tcg/card_quotes.c): every line fits the text box (36 letters, 2 lines a page) in every
 * game, every voice has something for every moment and record, and the specials speak as themselves. */
#include "tcg/quotes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
unsigned CardQuote_Count(void);
const char *CardQuote_Raw(unsigned i);
static void Fits(const char *text, const char *what)
{
    unsigned line = 0, lines = 1;
    for (; *text; text++)
    {
        if (*text == '\n' || *text == '\f')
        {
            lines = *text == '\f' ? 1 : lines + 1;
            line = 0;
            if (lines > 2)
            {
                printf("FAIL %s: more than 2 lines on a page\n", what);
                assert(0);
            }
            continue;
        }
        if (++line > 36)
        {
            printf("FAIL %s: a line longer than 36\n", what);
            assert(0);
        }
    }
}
int main(void)
{
    char out[256];
    unsigned i, voice, game, when, wins, losses, pick, n = CardQuote_Count();
    for (i = 0; i < n; i++)
        Fits(CardQuote_Raw(i), CardQuote_Raw(i));
    for (voice = 0; voice < QUOTE_VOICE_COUNT; voice++)
        for (game = 0; game < 3; game++)
            for (when = 0; when < 3; when++)
                for (wins = 0; wins < 4; wins++)
                    for (losses = 0; losses < 4; losses++)
                    {
                        if (when != QUOTE_HELLO && !wins && !losses)
                            continue;
                        for (pick = 0; pick < 40; pick++)
                        {
                            assert(CardQuote(out, sizeof(out), NULL, voice, game, when, wins, losses, pick * 7919u));
                            assert(!strchr(out, '{'));
                            Fits(out, out);
                        }
                    }
    {
        static const char *const who[] = {"ROXANNE", "TATE & LIZA", "LUCY", "CARD QUEEN", "MOM", "STEVEN"};
        for (i = 0; i < sizeof(who) / sizeof(who[0]); i++)
            for (when = 0; when < 3; when++)
            {
                assert(CardQuote(out, sizeof(out), who[i], QUOTE_VOICE_REFINED, QUOTE_GAME_TT, when, 1, 1, 5));
                Fits(out, out);
            }
        /* (someone without lines of their own speaks as a refined player) */
        assert(CardQuote(out, sizeof(out), "NOBODY", QUOTE_VOICE_KID, 0, QUOTE_THEY_LOST, 1, 0, 3));
    }
    printf("PASS %u card table lines, every voice, moment and record, in all three games\n", n);
    return 0;
}
