/* Yu-Gi-Oh! in the game: the collection of this save, the card screens (src/ygo/ygo_ui.c) opened from the CARD
 * CASE's YU-GI-OH! side and drawn through the card screens' canvas (src/tcg/tcg_ui.c Tcg_Canvas). */
#ifndef MOEMON_YGO_INTEGRATION_H
#define MOEMON_YGO_INTEGRATION_H
#include "ygo/collection.h"

struct YgoCollection *Ygo_GetCollection(void);
/* Right after the game was saved: the collection goes in "<save>.ygo" (from Tcg_WriteSidecar). */
void Ygo_WriteSidecar(void);

/* The screens */
int YgoUi_Active(void);
int YgoUi_Step(unsigned keys);
void YgoUi_Draw(void);
/* The CARD CASE on its YU-GI-OH! side (returnToGame: as Tcg_OpenCase's) */
void YgoUi_OpenCase(void (*returnToGame)(void), int fromScript);
/* Shown over this player's name to friends: a Yu-Gi-Oh! duel now */
unsigned YgoUi_DuelingNow(void);
void Ygo_Debug(const char *command);
#endif
