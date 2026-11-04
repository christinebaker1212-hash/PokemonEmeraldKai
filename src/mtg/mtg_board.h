// Moemon Kai: the Magic duel table (mtg_board.c), on the card screens' 240x160 canvas. The player is always
// player 0; player 1 is the computer (mtg_ai.c until forge-ai's translation).
#ifndef GUARD_MTG_BOARD_H
#define GUARD_MTG_BOARD_H

#include "mtg/mtg.h"

void MtgBoard_Start(const MtgDeck *mine, const MtgDeck *theirs, uint32_t seed, const char *opponent);
// a frame of the duel: 0 while it goes on, then 1 won / 2 lost / 3 a draw once the result screen is left
int MtgBoard_Step(unsigned pressed, unsigned keys);
void MtgBoard_Draw(void);
void MtgBoard_End(void);               // the map's song back
void MtgBoard_EndHoldMusic(void);      // ... but the victory song plays on (the spoils screen)
void MtgBoard_ReleaseMusic(void);
int MtgBoard_Active(void);
void MtgBoard_Autoplay(int on);        // the test harness: the computer plays the player's side too
void MtgBoard_ForceEnd(int win);       // the test harness
void MtgBoard_DebugFocus(const char *name);

#endif
