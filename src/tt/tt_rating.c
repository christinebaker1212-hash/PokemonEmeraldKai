// Moemon Kai: what Triple Triad's power ratings (tt_ratings.c) say about a game (include/tt/rating.h). No libm in
// the game: powers of ten are multiplied out.
#include "tt/rating.h"

int Tt_HandPower(const uint16_t *hand)
{
    int total = 0;
    unsigned i, n = 0;
    for (i = 0; i < TT_HAND; i++)
        if (Tt_Card(hand[i]))
            total += gTtCardRating[hand[i]], n++;
    return n ? total / (int)n : 1500;
}

int Tt_LevelRating(unsigned level, uint32_t rules)
{
    level = level > 3 ? 3 : level;
    return gTtLevelRating[((rules & TT_RULE_BIT(TT_RULE_OPEN)) ? 1 : 0) + (Tt_SamePlus(rules) ? 2 : 0)][level];
}

/* 10^(d/400) */
static double Pow10Over400(int d)
{
    double base = 1.0057730762, out = 1;
    unsigned n = (unsigned)(d < 0 ? -d : d);
    while (n)
    {
        if (n & 1)
            out *= base;
        base *= base;
        n >>= 1;
    }
    return d < 0 ? 1 / out : out;
}
unsigned Tt_Chance(int mine, int theirs)
{
    return (unsigned)(1000 / (1 + Pow10Over400(theirs - mine)) + 0.5);
}
