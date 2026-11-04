// Moemon Kai: Magic: The Gathering in the game: the save's collection ("<save>.mtg"), the CARD CASE's MAGIC side
// (src/mtg/mtg_ui.c, hidden until the player has a deck), the duel table (src/mtg/mtg_board.c), MOSSDEEP's CARD
// SHOP (data/scripts/mtg_shop.inc). Drawn through the card screens' canvas (src/tcg/tcg_ui.c Tcg_Canvas).
#ifndef GUARD_MTG_INTEGRATION_H
#define GUARD_MTG_INTEGRATION_H

#include "mtg/collection.h"

int Mtg_EnsureDb(void);                       // the built-in pack loaded (once); 0 if it can't be
MtgCollection *Mtg_GetCollection(void);
void Mtg_WriteSidecar(void);                  // right after the game was saved (from Tcg_WriteSidecar)
void Mtg_ForgetSidecar(void);                 // another save file now
int Mtg_Unlocked(void);                       // has a deck: the MAGIC side and NPCs' Magic show
void Mtg_Debug(const char *command);          // "mp mtg ..." (src/mp_debug.c)
void Mtg_PlainText(char *out, unsigned cap, const char *in);   // card words in the font's letters (mtg_text.c)

int MtgUi_Active(void);
int MtgUi_Step(unsigned keys);
void MtgUi_Draw(void);
void MtgUi_OpenCase(void (*returnToGame)(void), int fromScript);
void MtgUi_OpenStarter(void);                 // the CARD SHOP's clerk: choose a starter deck
void MtgUi_OpenChallenge(unsigned duelist);   // gMtgDuelists[duelist] asks for a duel
unsigned MtgUi_DuelingNow(void);

// the map scripts' specials (data/specials.inc)
void Mtg_ShopState(void);                     // VAR_RESULT: has a deck
void Mtg_ShopStarter(void);                   // (waitstate) the starter screen
void Mtg_DuelistState(void);                  // VAR_0x8004 the duelist: VAR_RESULT has a deck; their name in STR_VAR_1
void Mtg_DuelistChallenge(void);              // (waitstate) their challenge
#endif
