/* Triple Triad in the game: the collection of this save ("<save>.tt"), the card screens (src/tt/tt_ui.c) opened
 * from the CARD CASE's TRIPLE TRIAD side and drawn through the card screens' canvas (src/tcg/tcg_ui.c Tcg_Canvas). */
#ifndef MOEMON_TT_INTEGRATION_H
#define MOEMON_TT_INTEGRATION_H
#include "tt/collection.h"
struct TtNet;

struct TtCollection *Tt_GetCollection(void);
void Tt_WriteSidecar(void);   /* right after the game was saved (from Tcg_WriteSidecar) */

int TtUi_Active(void);
int TtUi_Step(unsigned keys);
void TtUi_Draw(void);
void TtUi_OpenCase(void (*returnToGame)(void), int fromScript);
unsigned TtUi_DuelingNow(void);
unsigned TtUi_HiddenFrom(void);
void Tt_Debug(const char *command);
/* Games and trades with friends (tt_net.c over the card games' relay, asked through tcg_online.c) */
struct TtNet *Tt_Online(void);
void Tt_OnlineReceive(unsigned from, const unsigned char *data, unsigned len, int willing);
void Tt_OnlineTick(void);
void TtUi_OpenOnlineDeck(void);
/* HOENN's rules online (tt_integration.c): in a room someone else made, every region's rules and TRADE RULE and where
 * the CARD QUEEN is are the room host's (the collection holds them while there; the save keeps the player's own). */
#define TT_WORLD_TYPE 15   /* on the card games' relay: kind 0 the host's HOENN, 1 a friend's change to it */
void Tt_WorldReceive(unsigned from, const unsigned char *data, unsigned len);
int Tt_WorldFromHost(void);
void TtUi_OpenOnlineTrade(void);
#endif
