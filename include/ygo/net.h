/* Yu-Gi-Oh! duels with a friend (src/ygo/ygo_net.c). The engine is deterministic from the seed and the answers,
 * so the link carries only the decks once and then each answer, with the duel's hash before and after so both
 * games know they hold the same duel. One answer at a time: the next waits for the other game's acknowledgement.
 * Rides the card games' relay extension (TCG_LINK_RELAY / TCG_LINK_VERSION, include/tcg/link.h), types 8-13. */
#ifndef MOEMON_YGO_NET_H
#define MOEMON_YGO_NET_H
#include <stdint.h>
#include "ygo/duel.h"

enum
{
    YGO_NET_IDLE,
    YGO_NET_SENT,     /* invited with our deck, waiting for theirs */
    YGO_NET_INVITED,  /* their invite is in: accept with a deck */
    YGO_NET_STARTING, /* accepted, waiting for the start */
    YGO_NET_PLAYING,
    YGO_NET_ENDED
};
/* The message types (byte 2): below 0x10 like the Pokemon duel's, above its 1-6 */
#define YGO_NET_FIRST_TYPE 8
#define YGO_NET_LAST_TYPE 13

struct YgoNet
{
    struct YgoDuel game;
    /* The duel before our unacknowledged answer: a concession from the other player that crossed it on the wire is
     * applied to this instead */
    struct YgoDuel beforePending;
    uint32_t sequenceBefore;
    uint16_t main[2][YGO_DECK_MAX], extra[2][YGO_FUSION_MAX];
    uint32_t session, sequence, deadline;
    unsigned state, peer, seat, pending;
    void (*send)(void *, unsigned, const uint8_t *, unsigned);
    void *context;
    char error[160];
};
void YgoNet_Init(struct YgoNet *l, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *context);
/* Seat 0 (the asker) invites with its deck; seat 1 accepts the invite with its own */
int YgoNet_Invite(struct YgoNet *l, unsigned peer, uint32_t session, const uint16_t *main, const uint16_t *extra,
                   uint32_t now);
int YgoNet_Accept(struct YgoNet *l, const uint16_t *main, const uint16_t *extra, uint32_t now);
/* willing: an invite from this player is expected */
void YgoNet_Receive(struct YgoNet *l, unsigned from, const uint8_t *p, unsigned len, int willing, uint32_t now);
/* Our answer to the duel's question (it must be ours), applied here and sent; 0 refused (l->error) */
int YgoNet_Submit(struct YgoNet *l, const struct YgoAnswer *a, uint32_t now);
int YgoNet_Concede(struct YgoNet *l, uint32_t now);
void YgoNet_Cancel(struct YgoNet *l, const char *reason);
void YgoNet_Tick(struct YgoNet *l, int connected, uint32_t now);
/* The duel's hash with the answers given inside the current step */
uint32_t YgoNet_Hash(const struct YgoDuel *d);
/* Who goes first in a duel between friends: from the session (both games agree) */
unsigned YgoNet_First(uint32_t session);
#endif
