/* Yu-Gi-Oh! on EDOPro's engine (tools/ygo/test.sh): duels between the AI and random play end, every card is always
 * in one place, the objects follow the engine without being rebuilt, the same seed and answers give the same duel,
 * and a duel rebuilt from its answers (YgoDuel_Copy / Rewind) is the same duel. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"
#include "../../src/ygo/ygo_edo.h"

extern unsigned gYgoEdoRebuilds, gYgoEdoRetries;
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                     \
        }                                                                \
    } while (0)

static uint32_t sRng = 5;
static unsigned Rnd(unsigned n)
{
    sRng = sRng * 1103515245u + 12345u;
    return n ? (sRng >> 8) % n : 0;
}
/* A random answer the engine takes */
static void RandomAnswer(const struct YgoDuel *d, struct YgoAnswer *a)
{
    const struct YgoQuery *q = &d->query;
    unsigned tries;
    for (tries = 0; tries < 200; tries++)
    {
        unsigned i, n;
        memset(a, 0, sizeof(*a));
        if (q->type == YGO_Q_NAME)
        {
            unsigned k = Rnd(d->nameCount);
            a->count = 2;
            a->picks[0] = (uint8_t)k;
            a->picks[1] = (uint8_t)(k >> 8);
        }
        else if (q->type == YGO_Q_COUNTER)
        {
            unsigned left = q->sum;
            a->count = q->count;
            for (i = 0; i < q->count; i++)
            {
                unsigned t = q->items[i].param < left ? q->items[i].param : left;
                a->picks[i] = (uint8_t)t;
                left -= t;
            }
        }
        else if (q->type == YGO_Q_SORT)
        {
            a->count = q->count;
            for (i = 0; i < q->count; i++)
                a->picks[i] = (uint8_t)i;
        }
        else
        {
            n = q->min + Rnd((unsigned)(q->max - q->min) + 1);
            if (q->type == YGO_Q_CHAIN)
                n = Rnd(3) ? 0 : 1;
            if (n > q->count)
                n = q->count;
            while (a->count < n)
            {
                unsigned p = Rnd(q->count), k, dup = 0;
                for (k = 0; k < a->count; k++)
                    dup |= a->picks[k] == p;
                if (!dup)
                    a->picks[a->count++] = (uint8_t)p;
            }
        }
        if (YgoDuel_Check(d, a))
            return;
    }
    YgoDuel_AiAnswer(d, a);
}
static unsigned StarterDeck(uint16_t *out, unsigned set)
{
    const struct YgoSet *s = &gYgoSets[set];
    unsigned i, k, n = 0;
    memset(out, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    for (i = 0; i < s->count; i++)
        for (k = 0; k < s->cards[i].copies && n < YGO_DECK_MAX; k++)
            out[n++] = s->cards[i].card;
    return n;
}
/* One duel: each side AI or random; returns the turns it took */
static unsigned Duel(uint32_t seed, unsigned first, int random0, int random1, uint32_t *hashes, unsigned *answers)
{
    static struct YgoDuel d;
    uint16_t m0[YGO_DECK_MAX], m1[YGO_DECK_MAX], x[YGO_FUSION_MAX];
    unsigned steps = 0;
    sRng = seed;
    memset(x, 0, sizeof(x));
    StarterDeck(m0, YGO_SET_SDY);
    StarterDeck(m1, YGO_SET_SDK);
    CHECK(YgoDuel_Init(&d, m0, x, m1, x, seed, first));
    CHECK(YgoDuel_Conserved(&d));
    while (d.query.type != YGO_Q_NONE && steps < 3000)
    {
        struct YgoAnswer a;
        if ((d.query.player ? random1 : random0))
            RandomAnswer(&d, &a);
        else
            YgoDuel_AiAnswer(&d, &a);
        if (!YgoDuel_Answer(&d, &a))
        {
            fprintf(stderr, "answer refused: %s (question %u)\n", d.error, d.query.type);
            CHECK(0);
        }
        CHECK(YgoDuel_Conserved(&d));
        if (hashes)
            hashes[steps] = YgoDuel_Hash(&d);
        steps++;
    }
    if (answers)
        *answers = steps;
    CHECK(d.s.over || steps >= 3000);
    return d.s.turn;
}

int main(void)
{
    static struct YgoDuel a, b;
    static uint32_t h1[3000], h2[3000];
    unsigned i, n1, n2, wins[2] = {0, 0}, turns = 0;
    uint16_t m0[YGO_DECK_MAX], m1[YGO_DECK_MAX], x[YGO_FUSION_MAX];
    CHECK(YgoEdo_DataInit());
    CHECK(YgoEdo_ListCount() >= 5);
    /* the Decks are shuffled: different seeds deal different opening hands, the same seed the same */
    {
        uint16_t m[YGO_DECK_MAX], xx[YGO_FUSION_MAX];
        unsigned s, o, distinct = 0, k;
        uint32_t hands[8];
        memset(m, 0, sizeof(m));
        memset(xx, 0, sizeof(xx));
        for (i = 0; i < 40; i++)
            m[i] = (uint16_t)(100 + i);
        for (s = 0; s < 8; s++)
        {
            uint32_t h = 0;
            CHECK(YgoDuel_Init(&a, m, xx, m, xx, 1000 + s * 7919u, 0));
            for (o = 0; o < a.s.objCount; o++)
                if (a.s.objs[o].location == YGO_LOC_HAND && a.s.objs[o].controller == 0)
                    h = h * 131 + a.s.objs[o].card;
            hands[s] = h;
            for (k = 0; k < s && hands[k] != h; k++)
                ;
            distinct += k == s;
        }
        CHECK(distinct >= 7);
        CHECK(YgoDuel_Init(&b, m, xx, m, xx, 1000, 0));
        {
            uint32_t h = 0;
            for (o = 0; o < b.s.objCount; o++)
                if (b.s.objs[o].location == YGO_LOC_HAND && b.s.objs[o].controller == 0)
                    h = h * 131 + b.s.objs[o].card;
            CHECK(h == hands[0]);
        }
        printf("edo: 8 seeds, %u different opening hands\n", distinct);
    }
    /* the catalog's cards are all in EDOPro, and its Normal Monsters and scripted cards are playable */
    {
        unsigned ready = 0;
        for (i = 1; i <= YGO_CARD_COUNT; i++)
        {
            CHECK(YgoEdo_CodeOfId(i));
            CHECK(YgoEdo_IdOfCode(YgoEdo_CodeOfId(i)) == i || Ygo_Card(YgoEdo_IdOfCode(YgoEdo_CodeOfId(i)))->password ==
                                                                 Ygo_Card(i)->password);
            ready += Ygo_CardReady(i);
        }
        printf("edo: %u of %u catalog cards playable\n", ready, YGO_CARD_COUNT);
        CHECK(ready > YGO_CARD_COUNT * 9 / 10);
    }
    /* determinism */
    Duel(77, 0, 0, 1, h1, &n1);
    Duel(77, 0, 0, 1, h2, &n2);
    CHECK(n1 == n2 && !memcmp(h1, h2, n1 * sizeof(h1[0])));
    /* AI against AI, AI against random, random against random */
    for (i = 0; i < 24; i++)
    {
        static struct YgoDuel d;
        unsigned steps = 0;
        memset(x, 0, sizeof(x));
        StarterDeck(m0, i % 2 ? YGO_SET_SDY : YGO_SET_SDJ);
        StarterDeck(m1, i % 3 ? YGO_SET_SDK : YGO_SET_SDP);
        CHECK(YgoDuel_Init(&d, m0, x, m1, x, 1000 + i, i & 1));
        while (d.query.type != YGO_Q_NONE && steps++ < 3000)
        {
            struct YgoAnswer ans;
            int random = i >= 8 || (i >= 4 && d.query.player == 1);
            if (random)
                RandomAnswer(&d, &ans);
            else
                YgoDuel_AiAnswer(&d, &ans);
            if (steps > 2980)
            {
                unsigned k;
                printf("  turn %u phase %u q%u p%u prompt %u min %u max %u:", d.s.turn, d.s.phase, d.query.type,
                       d.query.player, d.query.prompt, d.query.min, d.query.max);
                for (k = 0; k < d.query.count && k < 12; k++)
                    printf(" [%u o%u %s]", d.query.items[k].kind, d.query.items[k].obj,
                           Ygo_Card(d.query.items[k].card) ? Ygo_Card(d.query.items[k].card)->name : "-");
                printf(" -> %u picks %u\n", ans.count, ans.picks[0]);
            }
            if (!YgoDuel_Answer(&d, &ans))
            {
                fprintf(stderr, "duel %u: answer refused: %s (question %u prompt %u)\n", i, d.error, d.query.type,
                        d.query.prompt);
                CHECK(0);
            }
            CHECK(YgoDuel_Conserved(&d));
        }
        CHECK(d.s.over);
        if (d.s.winner <= 1)
            wins[d.s.winner] += i < 4;
        turns += d.s.turn;
        printf("edo: duel %u over on turn %u, winner %d, %u answers\n", i, d.s.turn, d.s.winner == 0xFF ? -1 : d.s.winner,
               YgoDuel_AnswerCount(&d));
    }
    printf("edo: AI vs AI wins %u / %u, %u turns in all\n", wins[0], wins[1], turns);
    /* rebuilt from the answers */
    memset(x, 0, sizeof(x));
    StarterDeck(m0, YGO_SET_SDY);
    StarterDeck(m1, YGO_SET_SDK);
    CHECK(YgoDuel_Init(&a, m0, x, m1, x, 9, 1));
    for (i = 0; i < 120 && a.query.type != YGO_Q_NONE; i++)
    {
        struct YgoAnswer ans;
        YgoDuel_AiAnswer(&a, &ans);
        CHECK(YgoDuel_Answer(&a, &ans));
    }
    CHECK(YgoDuel_Copy(&b, &a));
    CHECK(YgoDuel_Hash(&a) == YgoDuel_Hash(&b));
    n1 = YgoDuel_AnswerCount(&a);
    {
        uint32_t want;
        static struct YgoDuel c;
        CHECK(YgoDuel_Rewind(&b, n1 - 5));
        CHECK(YgoDuel_AnswerCount(&b) == n1 - 5);
        CHECK(YgoDuel_Copy(&c, &b));
        want = YgoDuel_Hash(&c);
        CHECK(YgoDuel_Hash(&b) == want);
        YgoDuel_Free(&c);
    }
    YgoDuel_Free(&a);
    YgoDuel_Free(&b);
    printf("edo: %u rebuilds\n", gYgoEdoRebuilds);
    CHECK(gYgoEdoRebuilds == 0);
    printf("edo: %u answers refused by the engine\n", gYgoEdoRetries);
    CHECK(gYgoEdoRetries == 0);
    puts("ygo edo tests passed");
    return 0;
}
