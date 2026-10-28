// Triple Triad games between friends (tools/tt/test.sh): two link engines (src/tt/tt_net.c) joined by a queue play
// thousands of games with random rules and hands, the computer moving for both; both must end on the same game.
// Then the awkward cases: a concession crossing a move on the wire, two concessions crossing, a tampered move.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt/net.h"

static int sFails, sChecks;
#define CHECK(c, ...) do { sChecks++; if (!(c)) { sFails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

// The wire: messages in order, each to the other engine
#define WIRE 64
static struct { unsigned to, len; uint8_t p[64]; } sWire[WIRE];
static unsigned sQueued, sTamper;
static struct TtNet sNet[2];
static void Send(void *context, unsigned to, const uint8_t *p, unsigned len)
{
    unsigned from = (unsigned)(size_t)context;
    (void)to;
    if (sQueued >= WIRE || len > 64)
        abort();
    sWire[sQueued].to = from ^ 1;
    sWire[sQueued].len = len;
    memcpy(sWire[sQueued].p, p, len);
    if (sTamper && len > 20)
        sWire[sQueued].p[len - 5] ^= 0x40, sTamper = 0;   // the "after" hash of a move
    sQueued++;
}
static void Deliver(void)
{
    while (sQueued)
    {
        unsigned to = sWire[0].to, len = sWire[0].len;
        uint8_t p[64];
        memcpy(p, sWire[0].p, len);
        memmove(sWire, sWire + 1, (sQueued - 1) * sizeof(sWire[0]));
        sQueued--;
        TtNet_Receive(&sNet[to], to ^ 1, p, len, 1, 1000);
    }
}
static uint32_t sRng = 12345;
static uint32_t Rand(void)
{
    sRng = sRng * 1103515245u + 12345u;
    return sRng >> 8;
}
static void RandomHand(uint16_t *hand)
{
    unsigned i, k;
    for (i = 0; i < TT_HAND;)
    {
        unsigned id = 1 + Rand() % TT_CARD_MAX;
        if (!Tt_Card(id))
            continue;
        for (k = 0; k < i && hand[k] != id; k++)
            ;
        if (k == i)
            hand[i++] = (uint16_t)id;
    }
}
static void Begin(uint32_t rules)
{
    uint16_t a[TT_HAND], b[TT_HAND];
    unsigned i;
    for (i = 0; i < 2; i++)
        TtNet_Init(&sNet[i], Send, (void *)(size_t)i);
    sQueued = 0;
    RandomHand(a);
    RandomHand(b);
    CHECK(TtNet_Invite(&sNet[0], 1, Rand() | 1, rules, a, 0), "invite");
    Deliver();
    CHECK(sNet[1].state == TT_NET_INVITED, "invited (%u)", sNet[1].state);
    CHECK(sNet[1].rules == (rules & ((1u << TT_RULE_COUNT) - 1)), "the rules came with the invite");
    CHECK(TtNet_Accept(&sNet[1], b, 0), "accept");
    Deliver();
    CHECK(sNet[0].state == TT_NET_PLAYING && sNet[1].state == TT_NET_PLAYING, "both playing (%u %u)", sNet[0].state,
          sNet[1].state);
    CHECK(TtDuel_Hash(&sNet[0].game) == TtDuel_Hash(&sNet[1].game), "the same game to start");
}
// One move by whoever is to move; the other takes it off the queue
static int Step(unsigned level)
{
    unsigned seat = sNet[0].game.toMove, slot, cell, s2, c2;
    TtAi_Choose(&sNet[seat].game, level, Rand(), &slot, &cell);
    if (!TtNet_Move(&sNet[seat], slot, cell, 0))
    {
        CHECK(0, "move refused (seat %u, %u to %u)", seat, slot, cell);
        return 0;
    }
    CHECK(!TtNet_Move(&sNet[seat], slot, cell, 0), "no second move before the first is acknowledged");
    Deliver();
    CHECK(TtNet_Take(&sNet[seat ^ 1], &s2, &c2) && s2 == slot && c2 == cell, "the other side gets the move");
    CHECK(!sNet[seat].pending, "acknowledged");
    return 1;
}
static void TestGames(void)
{
    unsigned g, draws = 0, wins[2] = {0, 0};
    for (g = 0; g < 2000; g++)
    {
        uint32_t rules = Rand() & ((1u << TT_RULE_COUNT) - 1);
        int moves = 0;
        Begin(rules);
        while (!sNet[0].game.over && moves < 200 && Step(Rand() % 4))
            moves++;
        CHECK(sNet[0].game.over && sNet[1].game.over, "game %u over (%d moves)", g, moves);
        CHECK(TtDuel_Hash(&sNet[0].game) == TtDuel_Hash(&sNet[1].game), "game %u: the same game at the end", g);
        CHECK(sNet[0].game.winner == sNet[1].game.winner, "game %u: the same winner", g);
        CHECK(sNet[0].state == TT_NET_PLAYING && sNet[1].state == TT_NET_PLAYING, "game %u: no error (%s / %s)", g,
              sNet[0].error, sNet[1].error);
        if (sNet[0].game.winner < 2)
            wins[sNet[0].game.winner]++;
        else
            draws++;
    }
    printf("net: 2000 games, wins %u / %u, draws %u\n", wins[0], wins[1], draws);
}
static void TestConcedeCrossesMove(void)
{
    unsigned mover, other, slot, cell, s, c;
    Begin(0);
    Step(1);
    Step(1);
    mover = sNet[0].game.toMove;
    other = mover ^ 1;
    TtAi_Choose(&sNet[mover].game, 1, 7, &slot, &cell);
    CHECK(TtNet_Move(&sNet[mover], slot, cell, 0), "move");
    CHECK(TtNet_Concede(&sNet[other], 0), "the other concedes before it arrives");
    Deliver();
    CHECK(sNet[0].game.over && sNet[1].game.over, "both over");
    CHECK(sNet[mover].game.winner == mover && sNet[other].game.winner == mover, "the concession stands");
    CHECK(TtDuel_Hash(&sNet[0].game) == TtDuel_Hash(&sNet[1].game), "the same game (the move taken back)");
    CHECK(TtNet_Take(&sNet[mover], &s, &c) && s == 254, "the table takes the mover's card back");
    CHECK(TtNet_Take(&sNet[mover], &s, &c) && s == 255, "then the concession");
    CHECK(!TtNet_Take(&sNet[other], &s, &c), "the conceder plays nothing");
    CHECK(!sNet[0].pending && !sNet[1].pending, "nothing left waiting");
    CHECK(sNet[0].state == TT_NET_PLAYING && sNet[1].state == TT_NET_PLAYING, "no error (%s / %s)", sNet[0].error,
          sNet[1].error);
}
static void TestConcessionsCross(void)
{
    Begin(0);
    Step(1);
    CHECK(TtNet_Concede(&sNet[0], 0) && TtNet_Concede(&sNet[1], 0), "both concede");
    Deliver();
    CHECK(sNet[0].game.winner == 1 && sNet[1].game.winner == 1, "the asker's concession stands (%u %u)",
          sNet[0].game.winner, sNet[1].game.winner);
    CHECK(TtDuel_Hash(&sNet[0].game) == TtDuel_Hash(&sNet[1].game), "the same game");
    CHECK(sNet[0].state == TT_NET_PLAYING && sNet[1].state == TT_NET_PLAYING, "no error (%s / %s)", sNet[0].error,
          sNet[1].error);
}
static void TestTamper(void)
{
    unsigned seat, slot, cell;
    Begin(0);
    seat = sNet[0].game.toMove;
    TtAi_Choose(&sNet[seat].game, 1, 3, &slot, &cell);
    sTamper = 1;
    TtNet_Move(&sNet[seat], slot, cell, 0);
    Deliver();
    CHECK(sNet[0].state == TT_NET_ENDED && sNet[1].state == TT_NET_ENDED, "a game out of step ends (%u %u)",
          sNet[0].state, sNet[1].state);
    CHECK(sNet[seat ^ 1].error[0] && sNet[seat].error[0], "both say why");
}
static void TestRefusals(void)
{
    uint16_t bad[TT_HAND] = {1, 1, 2, 3, 4};
    TtNet_Init(&sNet[0], Send, (void *)0);
    CHECK(!TtNet_Invite(&sNet[0], 1, 5, 0, bad, 0), "a hand with a card twice is refused");
    Begin(0);
    {
        unsigned seat = sNet[0].game.toMove ^ 1;
        CHECK(!TtNet_Move(&sNet[seat], 0, 0, 0), "not your turn");
    }
    // the friend's game times out when nothing comes back
    {
        unsigned seat = sNet[0].game.toMove, slot, cell;
        TtAi_Choose(&sNet[seat].game, 0, 1, &slot, &cell);
        TtNet_Move(&sNet[seat], slot, cell, 0);
        sQueued = 0;   // lost
        TtNet_Tick(&sNet[seat], 1, 29000);
        CHECK(sNet[seat].state == TT_NET_PLAYING, "still waiting at 29 seconds");
        TtNet_Tick(&sNet[seat], 1, 31000);
        CHECK(sNet[seat].state == TT_NET_ENDED, "timed out at 30");
    }
}
int main(void)
{
    TestRefusals();
    TestTamper();
    TestConcedeCrossesMove();
    TestConcessionsCross();
    TestGames();
    printf("net: %d checks, %d failed\n", sChecks, sFails);
    return sFails != 0;
}
