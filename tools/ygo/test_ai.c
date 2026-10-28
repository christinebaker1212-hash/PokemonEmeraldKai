/* Yu-Gi-Oh! AI tests (tools/ygo/test.sh): it beats random play with the same deck, and AI-vs-AI duels end. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../src/ygo/ygo_internal.h"

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
static void RandomAnswer(const struct YgoDuel *d, struct YgoAnswer *a)
{
    const struct YgoQuery *q = &d->query;
    unsigned i;
    memset(a, 0, sizeof(*a));
    if (q->type == YGO_Q_CHAIN)
        return;
    a->count = q->min ? q->min : 1;
    if (a->count > q->count)
        a->count = q->count;
    for (i = 0; i < a->count; i++)
        a->picks[i] = i;
    if (q->type == YGO_Q_IDLE || q->type == YGO_Q_BATTLE || (q->min == 1 && q->max == 1))
        a->picks[0] = Rnd(q->count);
}

static void Starter(uint16_t *out, unsigned set)
{
    const struct YgoSet *s = &gYgoSets[set];
    unsigned i, n = 0;
    memset(out, 0, YGO_DECK_MAX * 2);
    for (i = 0; i < s->count && n < YGO_DECK_MAX; i++)
        if (Ygo_CardReady(s->cards[i].card) && Ygo_Card(s->cards[i].card)->frame != YGO_FUSION)
            out[n++] = s->cards[i].card;
}

/* ai: bit per player played by the AI; returns the winner */
static unsigned Play(unsigned ai, unsigned seed, unsigned set0, unsigned set1, unsigned *answers)
{
    static struct YgoDuel d;
    uint16_t a[YGO_DECK_MAX], b[YGO_DECK_MAX];
    unsigned n = 0;
    Starter(a, set0);
    Starter(b, set1);
    sRng = seed;
    CHECK(YgoDuel_Init(&d, a, NULL, b, NULL, seed, seed & 1));
    while (d.query.type != YGO_Q_NONE && n < 5000)
    {
        struct YgoAnswer ans;
        if (ai & (1u << d.query.player))
            YgoDuel_AiAnswer(&d, &ans);
        else
            RandomAnswer(&d, &ans);
        if (!YgoDuel_Answer(&d, &ans))
        {
            fprintf(stderr, "refused (%s): %s\n", ai & (1u << d.query.player) ? "AI" : "random", d.error);
            exit(1);
        }
        CHECK(YgoDuel_Conserved(&d));
        n++;
    }
    CHECK(d.s.over);
    *answers += n;
    return d.s.winner;
}

int main(void)
{
    unsigned i, wins = 0, answers = 0;
    clock_t t0 = clock();
    for (i = 0; i < 20; i++)
        wins += Play(1, 100 + i, YGO_SET_SDY, YGO_SET_SDY, &answers) == 0;
    printf("  the AI beat random play %u of 20 times (%.1f ms an answer)\n", wins,
           1000.0 * (clock() - t0) / CLOCKS_PER_SEC / answers);
    CHECK(wins >= 17);
    answers = 0;
    t0 = clock();
    for (i = 0; i < 6; i++)
        Play(3, 300 + i, YGO_SET_SDY, YGO_SET_SDK, &answers);
    printf("PASS the AI beats random play, AI-vs-AI duels end (%.1f ms an answer)\n",
           1000.0 * (clock() - t0) / CLOCKS_PER_SEC / answers);
    return 0;
}
