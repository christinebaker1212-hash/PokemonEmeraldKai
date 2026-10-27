/* CARD SCALING: deck strength as Elo ratings measured by AI duels (include/tcg/scale.h).
 * No libm: the game links without it, so powers of ten are multiplied out. */
#include "mp_version.h"
#include <string.h>
#include "tcg/duel.h"
#include "tcg/scale.h"

int TcgScale_Play(const uint16_t a[TCG_DECK_SIZE], const uint16_t b[TCG_DECK_SIZE], uint32_t seed)
{
    static struct TcgDuel g;
    struct TcgAction act;
    unsigned steps = 0;
    if (!TcgDuel_Init(&g, a, b, seed, 6))
        return -1;
    while (g.phase != TCG_FINISHED && steps++ < 3000)
        if (!TcgDuel_AiAction(&g, &act) || TcgDuel_Apply(&g, &act) != TCG_OK)
        {
            memset(&act, 0, sizeof(act));
            act.type = g.phase == TCG_SETUP ? TCG_FINISH_SETUP : TCG_PASS;
            if (TcgDuel_Apply(&g, &act) != TCG_OK)
                break;
        }
    return g.phase == TCG_FINISHED ? (int)g.winner : -1;
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
static double Expected(int rating, int other)
{
    return 1 / (1 + Pow10Over400(other - rating));
}
unsigned TcgScale_Expected(int rating, int other)
{
    return (unsigned)(Expected(rating, other) * 1000 + 0.5);
}

int TcgScale_Fit(const int16_t *ratings, const unsigned *games, const unsigned *halfPoints, unsigned count)
{
    int lo = 700, hi = 2300;
    double score = 0;
    unsigned i;
    for (i = 0; i < count; i++)
        score += halfPoints[i] / 2.0;
    while (hi - lo > 1)
    {
        int mid = (lo + hi) / 2;
        double e = 0;
        for (i = 0; i < count; i++)
            e += games[i] * Expected(mid, ratings[i]);
        if (e > score)
            hi = mid;
        else
            lo = mid;
    }
    return lo;
}

static uint32_t Hash(const uint16_t cards[TCG_DECK_SIZE])
{
    uint32_t h = 2166136261u;
    unsigned i;
    for (i = 0; i < TCG_DECK_SIZE; i++)
        h = (h ^ cards[i]) * 16777619u;
    return h;
}
int TcgScale_RateDeck(const uint16_t cards[TCG_DECK_SIZE])
{
    /* A few decks, remembered: rating one takes a few hundred quick duels' worth of time */
    static struct
    {
        uint32_t hash;
        int16_t rating;
    } sCache[8];
    static unsigned sNext;
    int16_t ratings[64];
    unsigned games[64], half[64], i, g, n = gTcgRatingReferenceCount < 64 ? gTcgRatingReferenceCount : 64;
    uint32_t h = Hash(cards);
    for (i = 0; i < 8; i++)
        if (sCache[i].rating && sCache[i].hash == h)
            return sCache[i].rating;
    for (i = 0; i < n; i++)
    {
        const uint16_t *ref = gTcgDeckDefinitions[gTcgRatingReferences[i]].cards;
        ratings[i] = gTcgDeckRating[gTcgRatingReferences[i]];
        games[i] = TCG_SCALE_REF_GAMES;
        half[i] = 0;
        for (g = 0; g < TCG_SCALE_REF_GAMES; g++)
        {
            int w = TcgScale_Play(cards, ref, h + i * 7919 + g * 104729);
            half[i] += w < 0 ? 1 : w == 0 ? 2 : 0;
        }
    }
    sCache[sNext].hash = h;
    sCache[sNext].rating = (int16_t)TcgScale_Fit(ratings, games, half, n);
    i = sNext;
    sNext = (sNext + 1) % 8;
    return sCache[i].rating;
}

void TcgScale_Result(struct TcgCollection *c, int deck, int opponent, int won)
{
    /* Faster at first, while it knows little about the player */
    int k = c->rated < 20 ? 48 : 24, skill;
    double e = Expected(deck + c->skill, opponent);
    skill = c->skill + (int)(k * ((won ? 1 : 0) - e) + (won ? 0.5 : -0.5));
    if (skill > TCG_SCALE_SKILL_MAX)
        skill = TCG_SCALE_SKILL_MAX;
    if (skill < -TCG_SCALE_SKILL_MAX)
        skill = -TCG_SCALE_SKILL_MAX;
    c->skill = (int16_t)skill;
    if (c->rated < 0xFFFF)
        c->rated++;
}
