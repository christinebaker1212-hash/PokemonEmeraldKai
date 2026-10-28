/* Yu-Gi-Oh! duels between two games over the link (src/ygo/ygo_net.c): the AI plays both seats, every message
 * goes through a queue (in order, as the relay keeps it), and both games must end on the same duel. Then answers
 * crossing a concession, and a tampered answer. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo/collection.h"
#include "ygo/net.h"

#define CHECK(x)                                                                                                      \
    do                                                                                                                \
    {                                                                                                                 \
        if (!(x))                                                                                                     \
        {                                                                                                             \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #x);                                                     \
            exit(1);                                                                                                  \
        }                                                                                                             \
    } while (0)

struct Msg
{
    unsigned to, len;
    uint8_t data[256];
};
static struct Msg sQueue[4096];
static unsigned sHead, sTail, sTamper;
static void Send(void *context, unsigned to, const uint8_t *data, unsigned len)
{
    struct Msg *m = &sQueue[sTail++ % 4096];
    (void)context;
    m->to = to;
    m->len = len;
    memcpy(m->data, data, len);
    if (sTamper && data[2] == 11 && len > 21)
    {
        m->data[21] ^= 1;   /* a pick changed on the way */
        sTamper = 0;
    }
}
static struct YgoNet sLinks[2];
/* Delivers what's waiting (up to max messages) */
static void Deliver(unsigned max)
{
    while (sHead != sTail && max--)
    {
        struct Msg *m = &sQueue[sHead++ % 4096];
        YgoNet_Receive(&sLinks[m->to], m->to ^ 1, m->data, m->len, 1, 0);
    }
}
static void Decks(uint16_t main[2][YGO_DECK_MAX], uint16_t extra[2][YGO_FUSION_MAX])
{
    struct YgoCollection c;
    Ygo_CollectionInit(&c, 3);
    memset(main, 0, 2 * YGO_DECK_MAX * sizeof(uint16_t));
    memset(extra, 0, 2 * YGO_FUSION_MAX * sizeof(uint16_t));
    CHECK(Ygo_ChooseStarter(&c, YGO_SET_SDJ));
    memcpy(main[0], c.decks[c.activeDeck].main, sizeof(main[0]));
    memcpy(extra[0], c.decks[c.activeDeck].extra, sizeof(extra[0]));
    Ygo_CollectionInit(&c, 4);
    CHECK(Ygo_ChooseStarter(&c, YGO_SET_SDK));
    memcpy(main[1], c.decks[c.activeDeck].main, sizeof(main[1]));
    memcpy(extra[1], c.decks[c.activeDeck].extra, sizeof(extra[1]));
}
static void Connect(uint32_t session)
{
    uint16_t main[2][YGO_DECK_MAX], extra[2][YGO_FUSION_MAX];
    sHead = sTail = 0;
    YgoNet_Init(&sLinks[0], Send, NULL);
    YgoNet_Init(&sLinks[1], Send, NULL);
    Decks(main, extra);
    CHECK(YgoNet_Invite(&sLinks[0], 1, session, main[0], extra[0], 0));
    Deliver(100);
    CHECK(sLinks[1].state == YGO_NET_INVITED);
    CHECK(YgoNet_Accept(&sLinks[1], main[1], extra[1], 0));
    Deliver(100);
    CHECK(sLinks[0].state == YGO_NET_PLAYING && sLinks[1].state == YGO_NET_PLAYING);
    CHECK(YgoNet_Hash(&sLinks[0].game) == YgoNet_Hash(&sLinks[1].game));
}
/* One step: whoever's question it is answers (the AI), then up to `deliver` messages go through */
static int Step(unsigned deliver)
{
    unsigned k;
    for (k = 0; k < 2; k++)
    {
        struct YgoNet *l = &sLinks[k];
        struct YgoAnswer a;
        if (l->state != YGO_NET_PLAYING || l->pending || l->game.s.over || l->game.query.player != l->seat ||
            l->game.query.type == YGO_Q_NONE)
            continue;
        YgoDuel_AiAnswer(&l->game, &a);
        if (!YgoNet_Submit(l, &a, 0))
        {
            memset(&a, 0, sizeof(a));
            if (!YgoNet_Submit(l, &a, 0))
                YgoNet_Concede(l, 0);
        }
    }
    Deliver(deliver);
    return !(sLinks[0].game.s.over && sLinks[1].game.s.over && sHead == sTail);
}

int main(void)
{
    unsigned seed, answers = 0, steps;
    for (seed = 1; seed <= 12; seed++)
    {
        Connect(0x1234567u * seed | 1);
        for (steps = 0; steps < 200000 && Step(seed % 3 + 1); steps++)
            if (sLinks[0].state != YGO_NET_PLAYING || sLinks[1].state != YGO_NET_PLAYING)
                break;
        CHECK(sLinks[0].state == YGO_NET_PLAYING && sLinks[1].state == YGO_NET_PLAYING);
        CHECK(sLinks[0].game.s.over && sLinks[1].game.s.over);
        CHECK(sLinks[0].game.s.winner == sLinks[1].game.s.winner);
        CHECK(YgoNet_Hash(&sLinks[0].game) == YgoNet_Hash(&sLinks[1].game));
        answers += sLinks[0].sequence;
    }
    printf("PASS 12 duels between two games over the link: the same duel to the end on both (%u answers)\n", answers);

    /* A concession crossing an answer: both end on the concession, the answer rolled back */
    Connect(0xBEEF1);
    for (steps = 0; steps < 40; steps++)
        Step(1000);
    {
        unsigned k, asker = sLinks[0].game.query.player;
        struct YgoAnswer a;
        CHECK(!sLinks[0].game.s.over && asker < 2);
        YgoDuel_AiAnswer(&sLinks[asker].game, &a);
        CHECK(YgoNet_Submit(&sLinks[asker], &a, 0));
        CHECK(YgoNet_Concede(&sLinks[asker ^ 1], 0));   /* (before the answer arrives) */
        Deliver(1000);
        for (k = 0; k < 2; k++)
        {
            CHECK(sLinks[k].state == YGO_NET_PLAYING && sLinks[k].game.s.over);
            CHECK(sLinks[k].game.s.winner == asker);
        }
        CHECK(YgoNet_Hash(&sLinks[0].game) == YgoNet_Hash(&sLinks[1].game));
    }
    printf("PASS a concession crossing an answer: both games end on the concession\n");

    /* A changed answer is caught */
    Connect(0xC0FFEE1);
    for (steps = 0; steps < 400 && sLinks[0].state == YGO_NET_PLAYING && sLinks[1].state == YGO_NET_PLAYING; steps++)
    {
        if (steps == 30)
            sTamper = 1;
        Step(1000);
    }
    CHECK(sLinks[0].state == YGO_NET_ENDED || sLinks[1].state == YGO_NET_ENDED);
    printf("PASS an answer changed on the way ends the duel (\"%s\")\n",
           sLinks[0].error[0] ? sLinks[0].error : sLinks[1].error);
    return 0;
}
