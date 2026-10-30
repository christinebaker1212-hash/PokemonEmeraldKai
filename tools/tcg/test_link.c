#include "tcg/link.h"
#include "tcg/card_ids.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
struct Packet
{
    unsigned to, len;
    uint8_t data[256];
};
static struct Packet q[8192];
static unsigned head, tail;
static void Send(void *ctx, unsigned peer, const uint8_t *p, unsigned len)
{
    (void)ctx;
    assert(tail < 8192 && len <= 256);
    q[tail].to = peer;
    q[tail].len = len;
    memcpy(q[tail++].data, p, len);
}
static void Flush(struct TcgLink *l)
{
    while (head < tail)
    {
        struct Packet p = q[head++];
        TcgLink_Receive(&l[p.to], p.to ^ 1, p.data, p.len, 1, 0);
    }
}
int main(void)
{
    struct TcgLink l[2];
    uint16_t deck[60];
    struct TcgAction a;
    unsigned i, seed, steps;
    for (i = 0; i < 60; i++)
        deck[i] = i < 4 ? TCG_CARD_CHARMANDER : i < 8 ? TCG_CARD_BILL : TCG_CARD_FIRE_ENERGY;
    for (seed = 1; seed <= 20; seed++)
    {
        head = tail = 0;
        for (i = 0; i < 2; i++)
            TcgLink_Init(&l[i], Send, NULL);
        assert(TcgLink_Invite(&l[0], 1, seed, deck, 0));
        Flush(l);
        assert(l[1].state == TCG_LINK_INVITED);
        assert(TcgLink_Accept(&l[1], deck, 0));
        Flush(l);
        assert(l[0].state == TCG_LINK_PLAYING && l[1].state == TCG_LINK_PLAYING);
        for (steps = 0; steps < 2000 && l[0].game.phase != TCG_FINISHED; steps++)
        {
            unsigned who = l[0].game.phase == TCG_PROMOTION ? l[0].game.promotePlayer : l[0].game.current;
            assert(TcgDuel_AiAction(&l[who].game, &a));
            assert(!TcgLink_Submit(&l[who ^ 1], &a, 0));
            assert(TcgLink_Submit(&l[who], &a, 0));
            assert(!TcgLink_Submit(&l[who], &a, 0));
            Flush(l);
            assert(!l[who].pending);
            assert(TcgLink_Hash(&l[0].game) == TcgLink_Hash(&l[1].game));
            assert(TcgDuel_Conserved(&l[0].game) && TcgDuel_Conserved(&l[1].game));
        }
        assert(l[0].game.phase == TCG_FINISHED && l[0].game.winner == l[1].game.winner);
        /* A concession crossing the other player's move on the wire. */
        head = tail = 0;
        for (i = 0; i < 2; i++)
            TcgLink_Init(&l[i], Send, NULL);
        assert(TcgLink_Invite(&l[0], 1, seed + 100, deck, 0));
        Flush(l);
        assert(TcgLink_Accept(&l[1], deck, 0));
        Flush(l);
        for (steps = 0; steps < seed * 3 && l[0].game.phase != TCG_FINISHED; steps++)
        {
            unsigned who = l[0].game.phase == TCG_PROMOTION ? l[0].game.promotePlayer : l[0].game.current;
            assert(TcgDuel_AiAction(&l[who].game, &a));
            assert(TcgLink_Submit(&l[who], &a, 0));
            Flush(l);
        }
        if (l[0].game.phase != TCG_FINISHED)
        {
            unsigned who = l[0].game.phase == TCG_PROMOTION ? l[0].game.promotePlayer : l[0].game.current;
            struct TcgAction concede = {0};
            concede.type = TCG_CONCEDE;
            uint32_t shown;
            assert(TcgDuel_AiAction(&l[who].game, &a));
            assert(TcgLink_Submit(&l[who], &a, 0));
            shown = l[who].game.eventSeq; /* what the mover's duel screen has seen */
            assert(TcgLink_Submit(&l[who ^ 1], &concede, 0));
            Flush(l);
            assert(l[0].state == TCG_LINK_PLAYING && l[1].state == TCG_LINK_PLAYING);
            assert(!l[0].pending && !l[1].pending);
            assert(l[0].game.phase == TCG_FINISHED && l[0].game.winner == who);
            assert(TcgLink_Hash(&l[0].game) == TcgLink_Hash(&l[1].game));
            /* Both screens get the win event, the mover's after everything it had shown */
            for (i = 0; i < 2; i++)
            {
                const struct TcgEvent *e = TcgDuel_GetEvent(&l[i].game, l[i].game.eventSeq - 1);
                assert(e && e->type == TCG_EV_WIN && e->value == 3 && e->player == who);
            }
            assert(l[who].game.eventSeq > shown);
        }
    }
    {
        uint8_t action[27] = {TCG_LINK_RELAY, TCG_LINK_VERSION, 4};
        struct TcgLink before = l[1];
        unsigned len;
        for (len = 0; len < 7; len++)
            TcgLink_Receive(&l[1], 0, action, len, 1, 0);
        assert(!memcmp(&l[1], &before, sizeof(before)));
        /* Foreign peer and stale session messages cannot change a duel. */
        action[3] = l[1].session;
        TcgLink_Receive(&l[1], 3, action, sizeof(action), 1, 0);
        assert(!memcmp(&l[1], &before, sizeof(before)));
        action[3] ^= 128;
        TcgLink_Receive(&l[1], 0, action, sizeof(action), 1, 0);
        assert(!memcmp(&l[1], &before, sizeof(before)));
        action[3] ^= 128;
        TcgLink_Receive(&l[1], 0, action, sizeof(action), 1, 0);
        assert(l[1].state == TCG_LINK_ENDED);
        assert(TcgLink_Hash(&l[1].game) == TcgLink_Hash(&before.game));
    }
    TcgLink_Tick(&l[0], 0, 0);
    assert(l[0].state == TCG_LINK_ENDED);
    head = tail = 0;
    TcgLink_Init(&l[0], Send, NULL);
    assert(TcgLink_Invite(&l[0], 1, 99, deck, 0));
    TcgLink_Tick(&l[0], 1, 120001);
    assert(l[0].state == TCG_LINK_ENDED);
    /* Consent is required even for a valid invitation packet. */
    head = tail = 0;
    for (i = 0; i < 2; i++)
        TcgLink_Init(&l[i], Send, NULL);
    assert(TcgLink_Invite(&l[0], 1, 123, deck, 0));
    TcgLink_Receive(&l[1], 0, q[0].data, q[0].len, 0, 0);
    assert(l[1].state == TCG_LINK_IDLE);
    Flush(l);
    assert(TcgLink_Accept(&l[1], deck, 0));
    Flush(l);
    /* Either person may concede, including while it is the other person's turn. */
    i = l[0].game.current ^ 1;
    memset(&a, 0, sizeof(a));
    a.type = TCG_CONCEDE;
    assert(TcgLink_Submit(&l[i], &a, 0));
    Flush(l);
    assert(l[0].game.winner == (i ^ 1) && l[1].game.winner == (i ^ 1));
    assert(TcgLink_Hash(&l[0].game) == TcgLink_Hash(&l[1].game));
    puts("PASS 20 synchronized online duels, crossing concessions, turn ownership, acknowledgement gating, conservation, disconnect and "
         "timeout");
    return 0;
}
