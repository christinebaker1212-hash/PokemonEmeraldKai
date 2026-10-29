/* What Yu-Gi-Oh!'s power ratings (ygo_card_ratings.c, tools/ygo/rate_cards.sh) say: a deck's rating from its cards,
 * the chance one side beats another, the skill correction (src/ygo/ygo_internal.h). No libm in the game. */
#include "ygo_internal.h"
#include "ygo/collection.h"

int Ygo_DeckRating(const uint16_t *main)
{
    long total = 0;
    unsigned i, n = 0;
    for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
        if (main[i] <= YGO_CARD_COUNT && gYgoCardRating[main[i]])
            total += gYgoCardRating[main[i]], n++;
    if (!n)
        return 1500;
    return (int)(gYgoDeckFitA + (long long)gYgoDeckFitB * (total / (long)n) / 1024);
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
unsigned Ygo_Chance(int mine, int theirs) { return (unsigned)(1000 / (1 + Pow10Over400(theirs - mine)) + 0.5); }

void Ygo_SkillResult(struct YgoCollection *c, int mine, int theirs, int won)
{
    int k = c->rated < 20 ? 48 : 24, e = (int)Ygo_Chance(mine + c->skill, theirs), s = won ? 1000 : 0, skill;
    skill = c->skill + (k * (s - e) + (s >= e ? 500 : -500)) / 1000;
    c->skill = (int16_t)(skill > YGO_SKILL_MAX ? YGO_SKILL_MAX : skill < -YGO_SKILL_MAX ? -YGO_SKILL_MAX : skill);
    if (c->rated < 0xFFFF)
        c->rated++;
}
