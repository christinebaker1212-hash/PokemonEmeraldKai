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
void TtUi_OpenOnlineTrade(void);
#endif
