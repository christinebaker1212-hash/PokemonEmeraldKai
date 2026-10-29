#ifndef TCG_LINK_H
#define TCG_LINK_H
#include "tcg/duel.h"
enum
{
    TCG_LINK_IDLE,
    TCG_LINK_SENT,
    TCG_LINK_INVITED,
    TCG_LINK_STARTING,
    TCG_LINK_PLAYING,
    TCG_LINK_ENDED
};
/* Versioned extension relayed by the existing server; no native structs on wire. */
#define TCG_LINK_RELAY 255
#define TCG_LINK_VERSION 14  /* 6: Yu-Gi-Oh! duels and trades (types 8-13: src/ygo/ygo_net.c; request kinds 2-3);
                               * 7: a Yu-Gi-Oh! answer carries the zone a card goes to (YgoAnswer.zone);
                               * 8: Triple Triad games and trades (type 14: src/tt/tt_net.c; request kinds 4-5);
                               * 9: an action carries an attack's option and Metronome's copy (the last 20 attacks);
                               * 10: Yu-Gi-Oh!'s Battle Phase offers END TURN;
                               * 11: Yu-Gi-Oh! duels on EDOPro's engine (its questions, up to 64 picks an answer);
                               * 12: Yu-Gi-Oh! Decks shuffled from the duel's seed;
                               * 13: Triple Triad's COMBO only from what SAME / PLUS took;
                               * 14: Triple Triad's HOENN rules from the room host (type 15: tt_integration.c) */
struct TcgLink
{
    struct TcgDuel game;
    /* The state before our unacknowledged action: a concession from the
     * other player that crossed it on the wire is applied to this instead. */
    struct TcgDuel beforePending;
    uint32_t sequenceBefore;
    uint16_t decks[2][60];
    uint32_t session, sequence, deadline;
    unsigned state, peer, seat, pending;
    void (*send)(void *, unsigned, const uint8_t *, unsigned);
    void *context;
    char error[160];
};
void TcgLink_Init(struct TcgLink *, void (*send)(void *, unsigned, const uint8_t *, unsigned), void *);
int TcgLink_Invite(struct TcgLink *, unsigned, uint32_t, const uint16_t *, uint32_t);
int TcgLink_Accept(struct TcgLink *, const uint16_t *, uint32_t);
void TcgLink_Receive(struct TcgLink *, unsigned, const uint8_t *, unsigned, int, uint32_t);
int TcgLink_Submit(struct TcgLink *, const struct TcgAction *, uint32_t);
void TcgLink_Cancel(struct TcgLink *, const char *);
void TcgLink_Tick(struct TcgLink *, int, uint32_t);
uint32_t TcgLink_Hash(const struct TcgDuel *);
#endif
