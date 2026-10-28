// Triple Triad rules tests (tools/tt/test.sh): one scenario per rule, built from real cards found in the catalog,
// then thousands of random games between the computer players with random rules (every game ends, the ten cards
// stay ten, the same seed and moves play out the same).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt/duel.h"

static int sFails, sChecks;
#define CHECK(c, ...) do { sChecks++; if (!(c)) { sFails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static int S(unsigned id, unsigned side) { return Tt_Card(id)->side[side]; }
// Puts a card down directly (a scenario's board)
static void Put(struct TtDuel *d, unsigned cell, unsigned id, unsigned seat)
{
    d->cell[cell].card = (uint16_t)id;
    d->cell[cell].owner = d->cell[cell].origin = (uint8_t)seat;
    d->turn++;
}
static void Fresh(struct TtDuel *d, uint32_t rules, unsigned mine)
{
    uint16_t a[5] = {mine, 2, 3, 4, 5}, b[5] = {6, 7, 8, 9, 10};
    TtDuel_Start(d, rules, TT_TRADE_ONE, 1234, a, b, 0);
}

static void TestPlain(void)
{
    unsigned a, b;
    struct TtDuel d;
    // a card above ours: ours takes it when our top beats its bottom
    for (a = 1; a <= TT_CARD_MAX; a++)
        for (b = 1; b <= TT_CARD_MAX; b++)
        {
            if (!Tt_Card(a) || !Tt_Card(b) || a == b)
                continue;
            Fresh(&d, 0, b);
            Put(&d, 1, a, 1);
            TtDuel_Play(&d, 0, 4);
            CHECK((d.cell[1].owner == 0) == (S(b, TT_UP) > S(a, TT_DOWN)), "plain %u over %u", b, a);
            if (b > 40)
                break;
        }
    // REVERSE: the lower number takes; FALLEN ACE: a 1 takes an A
    for (a = 1; a <= TT_CARD_MAX; a++)
        for (b = 1; b <= TT_CARD_MAX; b++)
        {
            int fallen;
            if (!Tt_Card(a) || !Tt_Card(b) || a == b || b > 60)
                continue;
            Fresh(&d, TT_RULE_BIT(TT_RULE_REVERSE), b);
            Put(&d, 1, a, 1);
            TtDuel_Play(&d, 0, 4);
            CHECK((d.cell[1].owner == 0) == (S(b, TT_UP) < S(a, TT_DOWN)), "reverse %u over %u", b, a);
            Fresh(&d, TT_RULE_BIT(TT_RULE_FALLEN_ACE), b);
            Put(&d, 1, a, 1);
            TtDuel_Play(&d, 0, 4);
            fallen = S(b, TT_UP) == 1 && S(a, TT_DOWN) == 10;
            CHECK((d.cell[1].owner == 0) == (S(b, TT_UP) > S(a, TT_DOWN) || fallen), "fallen ace %u over %u", b, a);
        }
    printf("plain, reverse, fallen ace: ok\n");
}

// SAME: our card at the center, matching two neighbors (above and left) takes them; then a COMBO from them
static void TestSame(void)
{
    unsigned a, b, c, found = 0, wall = 0;
    struct TtDuel d;
    for (a = 1; a <= TT_CARD_MAX && found < 20; a++)
        for (b = 1; b <= TT_CARD_MAX && found < 20; b++)
            for (c = 1; c <= TT_CARD_MAX && found < 20; c++)
            {
                if (!Tt_Card(a) || !Tt_Card(b) || !Tt_Card(c) || a == b || b == c || a == c)
                    continue;
                if (S(c, TT_UP) != S(a, TT_DOWN) || S(c, TT_LEFT) != S(b, TT_RIGHT))
                    continue;
                Fresh(&d, TT_RULE_BIT(TT_RULE_SAME), c);
                Put(&d, 1, a, 1);
                Put(&d, 3, b, 1);
                TtDuel_Play(&d, 0, 4);
                CHECK(d.cell[1].owner == 0 && d.cell[3].owner == 0, "same %u: %u %u", c, a, b);
                CHECK(d.evCount >= 4 && d.ev[1].kind == TTE_RULE && d.ev[1].by == TT_BY_SAME, "same banner");
                // without SAME, nothing taken (equal numbers)
                Fresh(&d, 0, c);
                Put(&d, 1, a, 1);
                Put(&d, 3, b, 1);
                TtDuel_Play(&d, 0, 4);
                CHECK(d.cell[1].owner == 1 && d.cell[3].owner == 1, "no same %u", c);
                found++;
            }
    CHECK(found > 0, "no SAME scenario found");
    // SAME WALL: a card with an A on top at the top middle, matching its left neighbor, takes it
    for (a = 1; a <= TT_CARD_MAX && wall < 10; a++)
        for (c = 1; c <= TT_CARD_MAX && wall < 10; c++)
        {
            if (!Tt_Card(a) || !Tt_Card(c) || a == c || S(c, TT_UP) != 10 || S(c, TT_LEFT) != S(a, TT_RIGHT))
                continue;
            Fresh(&d, TT_RULE_BIT(TT_RULE_SAME) | TT_RULE_BIT(TT_RULE_SAME_WALL), c);
            Put(&d, 0, a, 1);
            TtDuel_Play(&d, 0, 1);
            CHECK(d.cell[0].owner == 0, "same wall %u %u", c, a);
            Fresh(&d, TT_RULE_BIT(TT_RULE_SAME), c);
            Put(&d, 0, a, 1);
            TtDuel_Play(&d, 0, 1);
            CHECK(d.cell[0].owner == 1, "same without the wall %u %u", c, a);
            wall++;
        }
    CHECK(wall > 0, "no SAME WALL scenario found");
    printf("same (%u), same wall (%u): ok\n", found, wall);
}

// PLUS: two sums alike take both; the COMBO goes on from a taken card
static void TestPlus(void)
{
    unsigned a, b, c, e, found = 0, combos = 0;
    struct TtDuel d;
    for (a = 1; a <= TT_CARD_MAX && found < 20; a++)
        for (b = 1; b <= TT_CARD_MAX && found < 20; b++)
            for (c = 1; c <= TT_CARD_MAX && found < 20; c++)
            {
                if (!Tt_Card(a) || !Tt_Card(b) || !Tt_Card(c) || a == b || b == c || a == c)
                    continue;
                if (S(c, TT_UP) + S(a, TT_DOWN) != S(c, TT_LEFT) + S(b, TT_RIGHT))
                    continue;
                Fresh(&d, TT_RULE_BIT(TT_RULE_PLUS), c);
                Put(&d, 1, a, 1);
                Put(&d, 3, b, 1);
                TtDuel_Play(&d, 0, 4);
                CHECK(d.cell[1].owner == 0 && d.cell[3].owner == 0, "plus %u", c);
                found++;
                // a COMBO: a card of theirs at the corner (0) beaten by the taken left card's top or the taken
                // top card's left
                for (e = 1; e <= TT_CARD_MAX && combos < 5; e++)
                {
                    if (!Tt_Card(e) || e == a || e == b || e == c)
                        continue;
                    if (!(S(b, TT_UP) > S(e, TT_DOWN) || S(a, TT_LEFT) > S(e, TT_RIGHT)))
                        continue;
                    Fresh(&d, TT_RULE_BIT(TT_RULE_PLUS), c);
                    Put(&d, 1, a, 1);
                    Put(&d, 3, b, 1);
                    Put(&d, 0, e, 1);
                    TtDuel_Play(&d, 0, 4);
                    CHECK(d.cell[0].owner == 0, "combo %u from %u/%u to %u", c, a, b, e);
                    {
                        unsigned i, banner = 0;
                        for (i = 0; i < d.evCount; i++)
                            banner |= d.ev[i].kind == TTE_RULE && d.ev[i].by == TT_BY_COMBO;
                        CHECK(banner, "combo banner");
                    }
                    combos++;
                }
            }
    CHECK(found > 0 && combos > 0, "plus %u combos %u", found, combos);
    printf("plus (%u), combo (%u): ok\n", found, combos);
}

static void TestElementalAndAscension(void)
{
    unsigned a, b, n = 0;
    struct TtDuel d;
    for (b = 1; b <= TT_CARD_MAX && n < 30; b++)
        for (a = 1; a <= TT_CARD_MAX && n < 30; a++)
        {
            int up;
            if (!Tt_Card(a) || !Tt_Card(b) || a == b)
                continue;
            Fresh(&d, TT_RULE_BIT(TT_RULE_ELEMENTAL), b);
            memset(d.cell, 0, sizeof(d.cell));
            d.cell[4].tile = TT_TYPE_PRIMAL;
            Put(&d, 1, a, 1);
            TtDuel_Play(&d, 0, 4);
            up = S(b, TT_UP) + (Tt_Card(b)->type == TT_TYPE_PRIMAL ? 1 : -1);
            CHECK((d.cell[1].owner == 0) == (up > S(a, TT_DOWN)), "elemental %u over %u", b, a);
            n++;
        }
    // ASCENSION: two typed cards of one type: the second sees +2
    for (a = 1, n = 0; a <= TT_CARD_MAX; a++)
        for (b = a + 1; b <= TT_CARD_MAX && Tt_Card(a) && Tt_Card(a)->type; b++)
            if (Tt_Card(b) && Tt_Card(b)->type == Tt_Card(a)->type && n < 5)
            {
                Fresh(&d, TT_RULE_BIT(TT_RULE_ASCENSION), b);
                memset(d.placedTypes, 0, sizeof(d.placedTypes));
                Put(&d, 8, a, 1);
                d.placedTypes[Tt_Card(a)->type]++;
                TtDuel_Play(&d, 0, 4);
                CHECK(TtDuel_TypeShift(&d, Tt_Card(a)->type) == 2, "ascension count");
                CHECK(TtDuel_Side(&d, 4, TT_UP) == (S(b, TT_UP) + 2 > 10 ? 10 : S(b, TT_UP) + 2), "ascension side");
                n++;
            }
    CHECK(n > 0, "ascension pairs");
    printf("elemental, ascension: ok\n");
}

static void TestOrderChaosSwap(void)
{
    struct TtDuel d;
    uint16_t a[5] = {1, 2, 3, 4, 5}, b[5] = {6, 7, 8, 9, 10};
    unsigned i;
    TtDuel_Start(&d, TT_RULE_BIT(TT_RULE_ORDER), 0, 5, a, b, 0);
    CHECK(TtDuel_Playable(&d) == 1, "order first slot");
    TtDuel_Play(&d, 0, 0);
    CHECK(TtDuel_Playable(&d) == 1, "order theirs");
    TtDuel_Play(&d, 0, 1);
    CHECK(TtDuel_Playable(&d) == 2, "order next");
    TtDuel_Start(&d, TT_RULE_BIT(TT_RULE_CHAOS), 0, 5, a, b, 0);
    CHECK(__builtin_popcount(TtDuel_Playable(&d)) == 1, "chaos one");
    TtDuel_Start(&d, TT_RULE_BIT(TT_RULE_SWAP), 0, 5, a, b, 0);
    CHECK(d.swapped[0] < 5 && d.origin[0][d.swapped[0]] == 1 && d.hand[0][d.swapped[0]] >= 6, "swap");
    TtDuel_Start(&d, TT_RULE_BIT(TT_RULE_THREE_OPEN), 0, 5, a, b, 0);
    for (i = 0; i < 2; i++)
        CHECK(__builtin_popcount(d.shown[i]) == 3, "three open");
    printf("order, chaos, swap, three open: ok\n");
}

static void RandomHand(uint32_t *rng, uint16_t *h)
{
    unsigned n = 0, k;
    while (n < TT_HAND)
    {
        unsigned id = 1 + Tt_Rand(rng) % TT_CARD_MAX;
        if (!Tt_Card(id))
            continue;
        for (k = 0; k < n && h[k] != id; k++)
            ;
        if (k == n)
            h[n++] = (uint16_t)id;
    }
}

static void TestRandomGames(unsigned games)
{
    uint32_t rng = 99;
    unsigned g, wins[3] = {0, 0, 0}, rounds = 0, levelWins[2] = {0, 0};
    for (g = 0; g < games; g++)
    {
        struct TtDuel d, replay;
        uint16_t a[5], b[5];
        uint8_t moves[TT_MAX_ROUNDS * 9][2];
        unsigned m = 0, i, total, lv[2];
        uint32_t rules = Tt_Rand(&rng) & ((1u << TT_RULE_COUNT) - 1);
        if ((rules & TT_RULE_BIT(TT_RULE_ORDER)) && (rules & TT_RULE_BIT(TT_RULE_CHAOS)))
            rules &= ~TT_RULE_BIT(TT_RULE_CHAOS);
        if ((rules & TT_RULE_BIT(TT_RULE_ASCENSION)) && (rules & TT_RULE_BIT(TT_RULE_DESCENSION)))
            rules &= ~TT_RULE_BIT(TT_RULE_DESCENSION);
        RandomHand(&rng, a);
        RandomHand(&rng, b);
        lv[0] = g % 4;
        lv[1] = (g / 4) % 4;
        TtDuel_Start(&d, rules, g % 4, rng, a, b, g & 1);
        while (!d.over)
        {
            unsigned s, c;
            CHECK(m < TT_MAX_ROUNDS * 9, "game too long");
            if (m >= TT_MAX_ROUNDS * 9)
                break;
            TtAi_Choose(&d, lv[d.toMove], g, &s, &c);
            CHECK(TtDuel_CanPlay(&d, s, c), "ai move %u %u", s, c);
            moves[m][0] = (uint8_t)s;
            moves[m++][1] = (uint8_t)c;
            TtDuel_Play(&d, s, c);
            for (total = 0, i = 0; i < 2; i++)
                total += TtDuel_Score(&d, i);
            CHECK(total == 10 || d.over, "ten cards (%u)", total);
        }
        CHECK(TtDuel_Score(&d, 0) + TtDuel_Score(&d, 1) == 10, "ten at the end");
        wins[d.winner]++;
        rounds += d.round;
        if (lv[0] != lv[1] && d.winner < 2)
            levelWins[(lv[d.winner] > lv[d.winner ^ 1])]++;
        // the same seed and moves: the same game
        TtDuel_Start(&replay, rules, g % 4, d.seed, a, b, g & 1);
        for (i = 0; i < m; i++)
            TtDuel_Play(&replay, moves[i][0], moves[i][1]);
        CHECK(TtDuel_Hash(&replay) == TtDuel_Hash(&d), "replay %u", g);
    }
    printf("%u random games: %u / %u / %u draws, %u rounds; the better level won %u of %u\n", games, wins[0],
           wins[1], wins[2], rounds, levelWins[1], levelWins[0] + levelWins[1]);
    CHECK(levelWins[1] > levelWins[0], "the better computer player should win more");
}

static void TestDecks(void)
{
    char why[96];
    uint16_t deck[5] = {0};
    unsigned id, n = 0, five = 0;
    for (id = 1; id <= TT_CARD_MAX && n < 5; id++)
        if (Tt_Card(id) && Tt_Card(id)->stars <= 3)
            deck[n++] = (uint16_t)id;
    CHECK(Tt_DeckValid(deck, why, sizeof(why)), "low deck: %s", why);
    deck[1] = deck[0];
    CHECK(!Tt_DeckValid(deck, why, sizeof(why)), "copies");
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id) && Tt_Card(id)->stars == 5)
        {
            if (!five)
                deck[1] = (uint16_t)id, five = id;
            else
            {
                CHECK(!Tt_DeckFits(deck, 2, id, why, sizeof(why)), "two fives");
                break;
            }
        }
    printf("decks: ok\n");
}

int main(int argc, char **argv)
{
    unsigned games = argc > 1 ? (unsigned)atoi(argv[1]) : 2000;
    TestPlain();
    TestSame();
    TestPlus();
    TestElementalAndAscension();
    TestOrderChaosSwap();
    TestDecks();
    TestRandomGames(games);
    printf("%d checks, %d failed\n", sChecks, sFails);
    return sFails != 0;
}
