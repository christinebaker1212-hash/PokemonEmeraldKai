/* Triple Triad's power ratings (tools/tt/rate_cards.sh -> src/tt/tt_ratings.c): every card's Elo rating from games the
 * computer plays against itself with random hands (both players the best level, fair: neither sees hidden cards),
 * fitted as "a hand is as strong as its cards' average" and pulled toward its stars (XIV's own grading of the cards);
 * then how much each computer level gives away, and the first move's worth. `audit` plays every region's rules and
 * prints how a fair match goes for each level. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt/duel.h"
#include "tt/collection.h"
#include "tt/npc.h"
#include "tt/rating.h"

static uint32_t sRng = 12345;
static unsigned R(unsigned n) { return Tt_Rand(&sRng) % n; }

static uint16_t sByStars[TT_MAX_STARS + 1][TT_CARD_MAX + 1];
static unsigned sStarCount[TT_MAX_STARS + 1];
static void Index(void)
{
    unsigned id;
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id))
            sByStars[Tt_Card(id)->stars][sStarCount[Tt_Card(id)->stars]++] = (uint16_t)id;
}
static int In(const uint16_t *h, unsigned n, unsigned id)
{
    unsigned i;
    for (i = 0; i < n; i++)
        if (h[i] == id)
            return 1;
    return 0;
}
/* five different cards around `band` stars */
static void Hand(unsigned band, uint16_t *h)
{
    unsigned n = 0;
    while (n < TT_HAND)
    {
        int s = (int)band + (int)R(3) - 1;
        unsigned id;
        if (s < 1)
            s = 1;
        if (s > TT_MAX_STARS)
            s = TT_MAX_STARS;
        id = sByStars[s][R(sStarCount[s])];
        if (!In(h, n, id))
            h[n++] = (uint16_t)id;
    }
}
static const uint32_t sRuleSets[] = {
    0,
    TT_RULE_BIT(TT_RULE_OPEN),
    TT_RULE_BIT(TT_RULE_SAME),
    TT_RULE_BIT(TT_RULE_PLUS),
    TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_PLUS),
    TT_RULE_BIT(TT_RULE_ELEMENTAL),
    TT_RULE_BIT(TT_RULE_OPEN) | TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_SAME_WALL),
    TT_RULE_BIT(TT_RULE_THREE_OPEN) | TT_RULE_BIT(TT_RULE_PLUS),
};
#define RULE_SETS (sizeof(sRuleSets) / sizeof(sRuleSets[0]))

/* One game: the result for seat 0 (1 a win, 0.5 a draw, 0 a loss) */
static double Play(const uint16_t *a, const uint16_t *b, uint32_t rules, unsigned first, unsigned la, unsigned lb,
                   uint32_t seed)
{
    static struct TtDuel d;
    unsigned guard = 0;
    TtDuel_Start(&d, rules & ~TT_RULE_BIT(TT_RULE_SUDDEN_DEATH), TT_TRADE_ONE, seed, a, b, first);
    while (!d.over && guard++ < 40)
    {
        unsigned s, c;
        TtAi_Choose(&d, d.toMove ? lb : la, seed + guard * 977, &s, &c);
        if (s == 0xFF || !TtDuel_Play(&d, s, c))
            break;
    }
    return d.winner == 2 ? 0.5 : d.winner == 0 ? 1 : 0;
}

struct Game
{
    uint16_t a[TT_HAND], b[TT_HAND];
    uint8_t first;
    float score;
};

static double sRating[TT_CARD_MAX + 1], sPrior[TT_CARD_MAX + 1];
static double Mean(const uint16_t *h)
{
    double t = 0;
    unsigned i;
    for (i = 0; i < TT_HAND; i++)
        t += sRating[h[i]];
    return t / TT_HAND;
}
static double Expect(double d) { return 1 / (1 + pow(10, -d / 400)); }

/* Damped Newton steps on the log-likelihood, each card pulled toward its prior (sigma: how far it may stray) */
static void Fit(const struct Game *g, unsigned n, double firstEdge, double sigma, unsigned steps)
{
    static double grad[TT_CARD_MAX + 1], hess[TT_CARD_MAX + 1];
    const double k = log(10) / 400;
    unsigned it, i, j, id;
    for (it = 0; it < steps; it++)
    {
        memset(grad, 0, sizeof(grad));
        memset(hess, 0, sizeof(hess));
        for (i = 0; i < n; i++)
        {
            double diff = Mean(g[i].a) - Mean(g[i].b) + (g[i].first == 0 ? firstEdge : -firstEdge);
            double e = Expect(diff), err = g[i].score - e, w = e * (1 - e) / (TT_HAND * TT_HAND);
            for (j = 0; j < TT_HAND; j++)
            {
                grad[g[i].a[j]] += k * err / TT_HAND;
                grad[g[i].b[j]] -= k * err / TT_HAND;
                hess[g[i].a[j]] += k * k * w;
                hess[g[i].b[j]] += k * k * w;
            }
        }
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (Tt_Card(id))
            {
                double gr = grad[id] - (sRating[id] - sPrior[id]) / (sigma * sigma);
                double h = hess[id] + 1 / (sigma * sigma);
                sRating[id] += 0.7 * gr / h;
            }
    }
}

static double Elo(double score)
{
    if (score < 0.01)
        score = 0.01;
    if (score > 0.99)
        score = 0.99;
    return -400 * log10(1 / score - 1);
}

/* ------------------------------------------------------------------ */
/* audit: the game's own situations, a stand-in player (the SKILLED level: it slips now and then, as people do)  */
/* ------------------------------------------------------------------ */
/* the player's deck at a badge tier: 1-2 stars early, a four-star later, a five-star at the end */
static void PlayerDeck(unsigned tier, uint16_t *h)
{
    static const uint8_t sShape[4][TT_HAND] = {{1, 1, 2, 2, 2}, {2, 2, 3, 3, 3}, {3, 3, 3, 3, 4}, {3, 3, 3, 4, 5}};
    unsigned n = 0;
    while (n < TT_HAND)
    {
        unsigned s = sShape[tier][n], id = sByStars[s][R(sStarCount[s])];
        if (!In(h, n, id))
            h[n++] = (uint16_t)id;
    }
}
/* the cards a player owns at a tier: the deck's stars and below, about 40 */
static void Owned(unsigned tier, uint8_t *owned)
{
    unsigned i, top = tier + 2 > TT_MAX_STARS ? TT_MAX_STARS : tier + 2;
    memset(owned, 0, TT_CARD_MAX + 1);
    for (i = 0; i < 40; i++)
    {
        unsigned s = 1 + R(top);
        if (s > 3 && R(3))
            s = 3;   /* (rarely the high ones) */
        owned[sByStars[s][R(sStarCount[s])]] = 1;
    }
}
static double Mix(const uint16_t *a, const uint16_t *b, uint32_t rules, unsigned la, unsigned lb, unsigned i)
{
    /* (the stand-in plays as the game counts a player: CASUAL under SAME / PLUS, else SKILLED) */
    if (la == 2 && Tt_SamePlus(rules))
        la = 1;
    return Play(a, b, rules, i & 1, la, lb, 90000 + i);
}
static void Audit(unsigned games)
{
    static const char *const sTier[4] = {"0-1 badges", "2-3 badges", "4-6 badges", "7-8 badges"};
    unsigned region, tier, i, k;
    struct TtCollection coll;
    Tt_CollectionInit(&coll, 1);
    printf("Triple Triad audit: %u games a line, the player a stand-in as the game counts one (SKILLED; CASUAL under SAME / PLUS), every region's first rules\n",
           games);
    printf("%-12s %-11s %8s %8s %8s %8s\n", "region", "tier", "before", "now", "RANDOM<", "RANDOM");
    for (region = 0; region < TT_REGIONS; region++)
        for (tier = 0; tier < 4; tier++)
        {
            double old = 0, now = 0, rOld = 0, rNow = 0;
            uint32_t rules = Tt_RegionStartRules(region) & ~TT_RULE_BIT(TT_RULE_RANDOM);
            for (i = 0; i < games; i++)
            {
                struct TtOpponent o;
                uint16_t me[TT_HAND], theirs[TT_HAND], dealt[TT_HAND], deal2[TT_HAND];
                static uint8_t owned[TT_CARD_MAX + 1];
                unsigned level;
                memset(&o, 0, sizeof(o));
                o.region = (uint8_t)region;
                o.hash = Tt_Rand(&sRng);
                o.tier = (uint8_t)tier;
                o.special = TT_SPECIAL_NONE;
                level = tier >= 2 ? 2 : 1;
                if ((o.hash >> 13) % 4 == 0)
                    level--;
                o.level = (uint8_t)level;
                PlayerDeck(tier, me);
                /* before: their hand by the badges, RANDOM the player's five from the collection at random */
                Tt_RegionHand(region, o.hash, tier, i, theirs);
                old += Mix(me, theirs, rules, 2, level, i);
                Owned(tier, owned);
                Tt_DealRandom(owned, o.hash, dealt);
                rOld += Mix(dealt, theirs, rules | TT_RULE_BIT(TT_RULE_RANDOM), 2, level, i);
                /* now: fitted to the deck; RANDOM dealt fair to both */
                Tt_FitOpponentHand(&o, Tt_HandPower(me), rules, i, theirs);
                level = o.level;
                now += Mix(me, theirs, rules, 2, level, i);
                if (Tt_DealFair(owned, o.hash, Tt_HandPower(theirs) + Tt_LevelRating(level, rules) - Tt_PlayerRating(rules) + Tt_OpponentFavor(&o), dealt))
                {
                    Tt_DealRegion(region, o.hash ^ 0xdea1,
                                  Tt_HandPower(dealt) + Tt_PlayerRating(rules) - Tt_OpponentFavor(&o) - Tt_LevelRating(level, rules), deal2);
                    rNow += Mix(dealt, deal2, rules | TT_RULE_BIT(TT_RULE_RANDOM), 2, level, i);
                }
            }
            printf("%-12s %-11s %7.0f%% %7.0f%% %7.0f%% %7.0f%%\n", Tt_RegionName(region), sTier[tier],
                   100 * old / games, 100 * now / games, 100 * rOld / games, 100 * rNow / games);
        }
    /* the special players and the CARD QUEEN, by tier */
    printf("\n%-12s %-11s %8s %8s %8s %8s\n", "special", "tier", "before", "now", "RANDOM<", "RANDOM");
    for (k = 0; k < 2; k++)
        for (tier = 0; tier < 4; tier++)
        {
            double old = 0, now = 0, rOld = 0, rNow = 0;
            unsigned special = k ? TT_SPECIAL_QUEEN : TT_SPECIAL_STEVEN;
            uint32_t rules = TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_PLUS);
            for (i = 0; i < games; i++)
            {
                struct TtOpponent o;
                uint16_t me[TT_HAND], theirs[TT_HAND], dealt[TT_HAND], deal2[TT_HAND];
                static uint8_t owned[TT_CARD_MAX + 1];
                memset(&o, 0, sizeof(o));
                o.special = (uint8_t)special;
                o.level = 3;
                o.region = TT_REGION_SOOTOPOLIS;
                PlayerDeck(tier, me);
                Tt_SpecialHand(special, i, theirs);
                old += Mix(me, theirs, rules, 2, 3, i);
                Owned(tier, owned);
                Tt_DealRandom(owned, i * 77 + 5, dealt);
                rOld += Mix(dealt, theirs, rules | TT_RULE_BIT(TT_RULE_RANDOM), 2, 3, i);
                Tt_FitOpponentHand(&o, Tt_HandPower(me), rules, i, theirs);
                now += Mix(me, theirs, rules, 2, 3, i);
                if (Tt_DealFair(owned, i * 77 + 5, Tt_HandPower(theirs) - Tt_PlayerRating(rules) + Tt_OpponentFavor(&o), dealt))
                {
                    Tt_DealRegion(o.region, i * 31 + 9, Tt_HandPower(dealt) + Tt_PlayerRating(rules) - Tt_OpponentFavor(&o), deal2);
                    rNow += Mix(dealt, deal2, rules | TT_RULE_BIT(TT_RULE_RANDOM), 2, 3, i);
                }
            }
            printf("%-12s %-11s %7.0f%% %7.0f%% %7.0f%% %7.0f%%\n", k ? "CARD QUEEN" : "STEVEN", sTier[tier],
                   100 * old / games, 100 * now / games, 100 * rOld / games, 100 * rNow / games);
        }
    /* every level against the best, the same hands: the better plays better */
    printf("\nlevels against the best (same hands, both seats):");
    for (k = 0; k < 3; k++)
    {
        double t = 0;
        for (i = 0; i < games; i++)
        {
            uint16_t a[TT_HAND], b[TT_HAND];
            Hand(3, a);
            Hand(3, b);
            t += Mix(a, b, TT_RULE_BIT(TT_RULE_SAME), k, 3, i) + 1 - Mix(b, a, TT_RULE_BIT(TT_RULE_SAME), 3, k, i);
        }
        printf(" level %u %.0f%%", k, 100 * t / (2.0 * games));
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    if (argc > 2 && !strcmp(argv[1], "audit"))
    {
        Index();
        Audit((unsigned)atoi(argv[2]));
        return 0;
    }

    unsigned games = argc > 1 ? (unsigned)atoi(argv[1]) : 20000, i, id, s, lv;
    struct Game *g = calloc(games, sizeof(*g));
    double firstScore = 0, starMean[TT_MAX_STARS + 1], starN[TT_MAX_STARS + 1], level[4][4], first;
    unsigned open;
    Index();
    for (i = 0; i < games; i++)
    {
        unsigned band = 1 + R(TT_MAX_STARS), other = band + R(3) - 1;   /* (a star apart now and then: the scale) */
        Hand(band, g[i].a);
        Hand(other < 1 ? 1 : other > TT_MAX_STARS ? TT_MAX_STARS : other, g[i].b);
        g[i].first = (uint8_t)R(2);
        g[i].score = (float)Play(g[i].a, g[i].b, sRuleSets[R(RULE_SETS)], g[i].first, 3, 3, 1000 + i);
        firstScore += g[i].first == 0 ? g[i].score : 1 - g[i].score;
        if (i % 2000 == 1999)
            fprintf(stderr, "cards: %u games\n", i + 1);
    }
    first = Elo(firstScore / games) / 2;
    /* first: around 1500 with a weak pull; then the pull toward its stars' average */
    for (id = 0; id <= TT_CARD_MAX; id++)
        sRating[id] = sPrior[id] = 1500;
    Fit(g, games, first, 400, 60);
    memset(starMean, 0, sizeof(starMean));
    memset(starN, 0, sizeof(starN));
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id))
            starMean[Tt_Card(id)->stars] += sRating[id], starN[Tt_Card(id)->stars]++;
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id))
            sPrior[id] = starMean[Tt_Card(id)->stars] / starN[Tt_Card(id)->stars];
    Fit(g, games, first, 120, 60);
    /* the levels: the same hands, level lv against the best, each side once; by the table: hands hidden / OPEN,
     * without / with SAME or PLUS */
    for (open = 0; open < 4; open++)
        for (lv = 0; lv < 4; lv++)
        {
            double t = 0;
            unsigned n = games / 20;
            static const uint32_t sSp[3] = {TT_RULE_BIT(TT_RULE_SAME), TT_RULE_BIT(TT_RULE_PLUS),
                                            TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_PLUS)};
            for (i = 0; i < n && lv < 3; i++)
            {
                uint16_t a[TT_HAND], b[TT_HAND];
                unsigned band = 1 + R(TT_MAX_STARS);
                uint32_t rules = (open & 2) ? sSp[R(3)] | (R(2) ? TT_RULE_BIT(TT_RULE_SAME_WALL) : 0)
                                            : (R(2) ? TT_RULE_BIT(TT_RULE_ELEMENTAL) : 0);
                if (open & 1)
                    rules |= TT_RULE_BIT(TT_RULE_OPEN);
                Hand(band, a);
                Hand(band, b);
                t += Play(a, b, rules, i & 1, lv, 3, 50000 + i) + 1 - Play(b, a, rules, i & 1, 3, lv, 50000 + i);
            }
            level[open][lv] = lv == 3 ? 0 : Elo(t / (2.0 * n));
            fprintf(stderr, "level %u (%s%s): %+.0f\n", lv, open & 1 ? "OPEN" : "hidden", open & 2 ? ", SAME/PLUS" : "",
                    level[open][lv]);
        }
    printf("/* Generated by tools/tt/rate_cards.sh (tools/tt/rate_cards.c, %u games): every card's power rating, what\n"
           " * each computer level gives away, the first move's worth. Never shown as numbers. Do not edit. */\n",
           games);
    printf("#include \"tt/rating.h\"\n\n");
    printf("const int16_t gTtLevelRating[4][4] = {\n");
    for (open = 0; open < 4; open++)
        printf("    {%.0f, %.0f, %.0f, 0},%s\n", level[open][0], level[open][1], level[open][2],
               open == 0 ? " /* hands hidden */" : open == 1 ? " /* OPEN */" : open == 2 ? " /* hidden, SAME / PLUS */"
                                                                         : " /* OPEN, SAME / PLUS */");
    printf("};\n");
    printf("const int16_t gTtFirstMove = %.0f;\n", first);
    printf("const int16_t gTtCardRating[TT_CARD_MAX + 1] = {\n");
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id))
            printf("    [%u] = %.0f,\n", id, sRating[id]);
    printf("};\n");
    for (s = 1; s <= TT_MAX_STARS; s++)
        fprintf(stderr, "%u stars: %.0f on average (%.0f cards)\n", s, starMean[s] / starN[s], starN[s]);
    return 0;
}
