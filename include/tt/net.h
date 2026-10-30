/* Triple Triad games with a friend (src/tt/tt_net.c). The game is deterministic from the seed and the moves, so the
 * link carries the asker's rules and both hands once, then each move with the game's hash before and after; the
 * next move waits for the other game's acknowledgement. Rides the card games' relay extension (TCG_LINK_RELAY /
 * TCG_LINK_VERSION, include/tcg/link.h) as type 14, the kind of message in byte 7. */
#ifndef MOEMON_TT_NET_H
#define MOEMON_TT_NET_H
#include <stdint.h>
#include "tt/duel.h"

#define TT_NET_TYPE 14
enum
{
    TT_NET_IDLE,
    TT_NET_SENT,      /* invited with our rules and hand, waiting for theirs */
    TT_NET_INVITED,   /* their invite is in: accept with a hand */
    TT_NET_STARTING,  /* accepted, waiting for the start */
    TT_NET_PLAYING,
    TT_NET_ENDED
};
#define TT_NET_QUEUE 16
struct TtNet
{
    struct TtDuel game;          /* the game as both sides agree it is (moves applied as they cross the wire) */
    struct TtDuel beforePending; /* before our unacknowledged move (a concession crossing it is applied to this) */
    uint32_t sequenceBefore;
    uint16_t hand[2][TT_HAND];
    uint32_t rules, session, sequence, deadline;
    unsigned state, peer, seat, pending;
    /* the other side's moves for the table to play (255 255: they conceded) */
    uint8_t queue[TT_NET_QUEUE][2];
    unsigned queued;
    void (*send)(void *, unsigned, const uint8_t *, unsigned);
    void *context;
    char error[160];
};
void TtNet_Init(struct TtNet *l, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *context);
/* Seat 0 (the asker) invites with its rules and hand; seat 1 accepts with its hand once the invite is in */
int TtNet_Invite(struct TtNet *l, unsigned peer, uint32_t session, uint32_t rules, const uint16_t *hand, uint32_t now);
int TtNet_Accept(struct TtNet *l, const uint16_t *hand, uint32_t now);
void TtNet_Receive(struct TtNet *l, unsigned from, const uint8_t *p, unsigned len, int willing, uint32_t now);
/* Our move (it must be our turn); 0 refused */
int TtNet_Move(struct TtNet *l, unsigned slot, unsigned cell, uint32_t now);
int TtNet_Concede(struct TtNet *l, uint32_t now);
/* The other side's next move for the table: 1 and slot / cell (255 / 255 when they conceded), 0 none */
int TtNet_Take(struct TtNet *l, unsigned *slot, unsigned *cell);
void TtNet_Cancel(struct TtNet *l, const char *reason);
void TtNet_Tick(struct TtNet *l, int connected, uint32_t now);
/* Who goes first between friends: from the session (both games agree) */
unsigned TtNet_First(uint32_t session);
/* A side gives up: the other wins */
void TtDuel_Concede(struct TtDuel *d, unsigned seat);
#endif
