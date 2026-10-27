/* Asking friends for card duels and trades (src/tcg/tcg_online.c). */
#ifndef MOEMON_TCG_ONLINE_H
#define MOEMON_TCG_ONLINE_H
#include <stdint.h>
#include "tcg/link.h"

enum
{
    TCG_SOCIAL_NONE,
    TCG_SOCIAL_ASKING,   /* we asked, waiting for their answer */
    TCG_SOCIAL_INCOMING, /* they asked: shown when we're free */
    TCG_SOCIAL_ASKED,    /* their question is on screen */
    TCG_SOCIAL_DECK,     /* a yes: both pick a deck */
    TCG_SOCIAL_TRADE     /* a yes: both offer a card */
};
enum
{
    TCG_ASK_DUEL,
    TCG_ASK_TRADE
};
enum
{
    TCG_ANSWER_NO,
    TCG_ANSWER_YES,
    TCG_ANSWER_BUSY,
    TCG_ANSWER_NOCASE,
    TCG_ANSWER_WAITING = 0xFF
};
struct TcgSocial
{
    uint8_t role, kind, peer, seat, answer, peerLeft;
    uint8_t myConfirm, theirConfirm;
    uint16_t myOffer, theirOffer;
    uint32_t session, deadline;
};
const struct TcgSocial *Tcg_Social(void);
void Tcg_SocialTick(void);
void Tcg_SocialReceive(unsigned from, const unsigned char *data, unsigned len);
int Tcg_SocialExpectsInvite(unsigned from);
void Tcg_SocialOffer(unsigned card);
void Tcg_SocialConfirm(void);
void Tcg_SocialLeave(void);
void Tcg_SocialInit(void);
/* The card screens' side (tcg_ui.c). */
void Tcg_OpenOnlineDeck(void);
void Tcg_OpenOnlineTrade(void);
#endif
