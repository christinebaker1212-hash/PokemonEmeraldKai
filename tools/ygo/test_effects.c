/* Yu-Gi-Oh! effects tests (tools/ygo/test.sh): random duels with random decks of every card that works, under the
 * sanitizers: every duel ends, every card is kept, no answer is refused. Prints the cards whose effects never
 * activated. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static uint32_t sRng = 99;
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
    if (q->type == YGO_Q_IDLE || q->type == YGO_Q_BATTLE)
    {
        unsigned tries;
        for (tries = 0; tries < 8; tries++)
        {
            unsigned k = Rnd(q->count), kind = q->items[k].kind;
            if ((kind == YGO_ACT_END_TURN || kind == YGO_ACT_TO_MAIN2) && tries < 7)
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
        a->count = q->count && Rnd(2) == 0;
        a->picks[0] = Rnd(q->count);
        return;
    }
    a->count = q->min + (q->max > q->min ? Rnd(q->max - q->min + 1) : 0);
    if (a->count > q->count)
        a->count = q->count;
    for (i = 0; i < a->count; i++)
    {
        unsigned k, j;
        int dup;
        do
        {
            k = Rnd(q->count);
            dup = 0;
            for (j = 0; j < i; j++)
                dup |= a->picks[j] == k;
        } while (dup);
        a->picks[i] = k;
    }
}

static uint16_t sPool[YGO_CARD_COUNT], sFusions[YGO_CARD_COUNT];
static unsigned sPoolCount, sFusionCount;
static unsigned sActivated[YGO_CARD_COUNT + 1], sSeen[YGO_CARD_COUNT + 1];

static void Pool(void)
{
    static const unsigned sets[] = {YGO_SET_SDY, YGO_SET_SDK, YGO_SET_SDJ, YGO_SET_SDP, YGO_SET_LOB, YGO_SET_MRD};
    unsigned i, k;
    for (k = 0; k < sizeof(sets) / sizeof(sets[0]); k++)
        for (i = 0; i < gYgoSets[sets[k]].count; i++)
        {
            unsigned id = gYgoSets[sets[k]].cards[i].card, j;
            const struct YgoCard *c = Ygo_Card(id);
            int have = 0;
            if (!Ygo_CardReady(id))
                continue;
            for (j = 0; j < sPoolCount; j++)
                have |= sPool[j] == id;
            for (j = 0; j < sFusionCount; j++)
                have |= sFusions[j] == id;
            if (have)
                continue;
            if (c->frame == YGO_FUSION)
                sFusions[sFusionCount++] = id;
            else
                sPool[sPoolCount++] = id;
        }
}

static void RandomDeck(uint16_t *main, uint16_t *extra)
{
    unsigned n = 0, i;
    memset(main, 0, YGO_DECK_MAX * 2);
    memset(extra, 0, YGO_FUSION_MAX * 2);
    while (n < 40)
    {
        uint16_t id = sPool[Rnd(sPoolCount)];
        unsigned copies = 0;
        for (i = 0; i < n; i++)
            copies += main[i] == id;
        if (copies < Ygo_Card(id)->limit && copies < 3)
            main[n++] = id;
    }
    for (i = 0; i < 8 && i < sFusionCount; i++)
        extra[i] = sFusions[Rnd(sFusionCount)];
}

int main(void)
{
    static struct YgoDuel d;
    unsigned duel, total = 0, never = 0, i;
    Pool();
    printf("  %u cards and %u Fusion Monsters work in stage 1\n", sPoolCount, sFusionCount);
    for (duel = 0; duel < 400; duel++)
    {
        uint16_t a[YGO_DECK_MAX], b[YGO_DECK_MAX], ea[YGO_FUSION_MAX], eb[YGO_FUSION_MAX];
        unsigned n = 0;
        uint32_t seq = 0;
        sRng = 1000 + duel * 31;
        RandomDeck(a, ea);
        RandomDeck(b, eb);
        CHECK(YgoDuel_Init(&d, a, ea, b, eb, 5 + duel, duel & 1));
        for (i = 0; i < 40; i++)
            sSeen[a[i]]++, sSeen[b[i]]++;
        while (d.query.type != YGO_Q_NONE && n < 30000)
        {
            struct YgoAnswer ans;
            RandomAnswer(&d, &ans);
            if (!YgoDuel_Answer(&d, &ans))
            {
                fprintf(stderr, "duel %u: refused: %s (query %u)\n", duel, d.error, d.query.type);
                exit(1);
            }
            CHECK(YgoDuel_Conserved(&d));
            for (; seq < d.s.eventSeq; seq++)
            {
                const struct YgoEvent *e = YgoDuel_GetEvent(&d.s, seq);
                if (e && e->type == YGO_EV_ACTIVATE && e->card <= YGO_CARD_COUNT)
                    sActivated[e->card]++;
                if (e && e->type == YGO_EV_SUMMON && e->value == YGO_SUMMON_FUSION)
                    sActivated[e->card]++;
            }
            n++;
        }
        CHECK(d.s.over);
        total += n;
    }
    printf("PASS 400 random duels with random stage-1 decks: every card kept, every duel ended (%u answers)\n", total);
    printf("  never activated (of the cards with effects that were in decks):");
    for (i = 1; i <= YGO_CARD_COUNT; i++)
    {
        const struct YgoCard *c = Ygo_Card(i);
        unsigned n;
        const struct YgoEffect *e = Ygo_Effects(i, &n);
        unsigned k, active = 0;
        for (k = 0; k < n; k++)
            active |= e[k].type != YE_CONTINUOUS;
        if ((sSeen[i] && active && !sActivated[i]) || (c->frame == YGO_FUSION && Ygo_CardReady(i) && !sActivated[i]))
        {
            printf(" %s;", c->name);
            never++;
        }
    }
    printf(" (%u)\n", never);
    return 0;
}
