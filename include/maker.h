#ifndef GUARD_MAKER_H
#define GUARD_MAKER_H

// Moemon Maker: the game's side (docs/MOEMON_MAKER.md). The editor itself is C++ in maker/ and only talks to the game
// through include/maker_api.h.

// The way in (src/maker/maker_gate.c)
void Maker_Typed(void);
bool8 Maker_Unlocked(void);
bool8 Maker_TakeMenuRefresh(void);
void Maker_FrameUpdate(void);
void Maker_InitFromEnv(void);

// The editor (src/maker/maker_main.c)
void Maker_Open(void);          // from the main menu: the editor window opens, the game waits on a black screen
bool8 Maker_IsOpen(void);

#endif // GUARD_MAKER_H
