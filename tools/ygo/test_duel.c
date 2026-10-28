/* Yu-Gi-Oh! rules engine tests (tools/ygo/test.sh): random duels must end, keep every card, replay the same. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ygo/ygo_internal.h"

#define CHECK(x)                                                                 \
    do                                                                           \
    {                                                                            \
        if (!(x))                                                                \
        {                                                                        \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x);         \
            exit(1);                                                             \
        }                                                                        \
    } while (0)

static uint32_t sRng = 12345;
static unsigned Rnd(unsigned n)
{
    sRng = sRng * 1103515245u + 12345u;
    return n ? (sRng >> 8) % n : 0;
}

/* A random but sensible answer: summons and attacks more often than ending the turn */
static void RandomAnswer(const struct YgoDuel *d, struct YgoAnswer *a)
{
    const struct YgoQuery *q = &d->query;
    unsigned i;
    memset(a, 0, sizeof(*a));
    if (q->type == YGO_Q_IDLE || q->type == YGO_Q_BATTLE)
    {
        unsigned tries;
        for (tries = 0; tries < 6; tries++)
        {
            unsigned k = Rnd(q->count);
            unsigned kind = q->items[k].kind;
            if ((kind == YGO_ACT_END_TURN || kind == YGO_ACT_TO_MAIN2) && tries < 5)
                continue;
            a->count = 1;
            a->picks[0] = k;
            return;
        }
        a->count = 1;
        a->picks[0] = q->count - 1;
        return;
    }
    if (q->type == YGO_Q_CHAIN)
    {
        a->count = Rnd(3) == 0 && q->count;
        a->picks[0] = Rnd(q->count);
        return;
    }
    a->count = q->min + (q->max > q->min ? Rnd(q->max - q->min + 1) : 0);
    if (a->count > q->count)
        a->count = q->count;
    for (i = 0; i < a->count; i++)
    {
        unsigned k;
        int dup;
        do
        {
            unsigned j;
            k = Rnd(q->count);
            dup = 0;
            for (j = 0; j < i; j++)
                dup |= a->picks[j] == k;
        } while (dup);
        a->picks[i] = k;
    }
}

static void Deck(uint16_t *out, unsigned set)
{
    const struct YgoSet *s = &gYgoSets[set];
    unsigned i, n = 0;
    memset(out, 0, YGO_DECK_MAX * sizeof(uint16_t));
    for (i = 0; i < s->count && n < YGO_DECK_MAX; i++)
    {
        const struct YgoCard *c = Ygo_Card(s->cards[i].card);
        unsigned k;
        if (!Ygo_CardReady(c->id) || c->frame == YGO_FUSION)
            continue;
        for (k = 0; k < s->cards[i].copies && n < YGO_DECK_MAX; k++)
            out[n++] = c->id;
    }
    /* pad with Normal Monsters from the set to 40 */
    for (i = 0; n < YGO_DECK_MIN; i = (i + 1) % s->count)
        if (Ygo_Card(s->cards[i].card)->frame == YGO_NORMAL)
            out[n++] = s->cards[i].card;
}

static int RunDuel(uint32_t seed, unsigned set0, unsigned set1, uint32_t *hash, unsigned *steps, int verbose)
{
    static struct YgoDuel d;
    uint16_t a[YGO_DECK_MAX], b[YGO_DECK_MAX];
    unsigned n = 0;
    Deck(a, set0);
    Deck(b, set1);
    sRng = seed;
    CHECK(YgoDuel_Init(&d, a, NULL, b, NULL, seed, seed & 1));
    while (d.query.type != YGO_Q_NONE && n < 20000)
    {
        struct YgoAnswer ans;
        RandomAnswer(&d, &ans);
        if (!YgoDuel_Answer(&d, &ans))
        {
            fprintf(stderr, "refused: %s (query %u)\n", d.error, d.query.type);
            return 0;
        }
        CHECK(YgoDuel_Conserved(&d));
        n++;
    }
    if (verbose)
        printf("  seed %u: %u answers, turn %u, winner %u by %u, LP %d / %d\n", seed, n, d.s.turn, d.s.winner,
               d.s.winReason, d.s.players[0].lp, d.s.players[1].lp);
    CHECK(d.s.over);
    *hash = YgoDuel_Hash(&d);
    *steps = n;
    return 1;
}

int main(void)
{
    unsigned i, total = 0;
    for (i = 1; i <= 40; i++)
    {
        uint32_t h1, h2;
        unsigned n1, n2;
        CHECK(RunDuel(i * 7919, YGO_SET_SDY, YGO_SET_SDK, &h1, &n1, i <= 3));
        CHECK(RunDuel(i * 7919, YGO_SET_SDY, YGO_SET_SDK, &h2, &n2, 0));
        CHECK(h1 == h2 && n1 == n2);
        total += n1;
    }
    printf("PASS 40 random duels: every card kept, every duel ended, the same again from the same seed (%u answers)\n",
           total);
    return 0;
}
