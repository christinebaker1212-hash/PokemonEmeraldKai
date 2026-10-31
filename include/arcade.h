// Moemon Kai: the GAME CENTER's arcade games (CPS-2 boards), run from each player's own ROM sets in the "arcade"
// folder beside the game (arcade/arcade_cps2.cpp: cps2recomp's board, its 68000 interpreted; nothing of the games is
// in this repository or the game's downloads). COOP_NOTES "The GAME CENTER's arcade cabinets".
#ifndef GUARD_ARCADE_H
#define GUARD_ARCADE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    ARCADE_VAMPIRE_SAVIOR,
    ARCADE_CYBERBOTS,
    ARCADE_GAME_COUNT
};

// A player's controls (the board's): 1 while pressed
#define ARCADE_RIGHT  (1 << 0)
#define ARCADE_LEFT   (1 << 1)
#define ARCADE_DOWN   (1 << 2)
#define ARCADE_UP     (1 << 3)
#define ARCADE_LP     (1 << 4)
#define ARCADE_MP     (1 << 5)
#define ARCADE_HP     (1 << 6)
#define ARCADE_LK     (1 << 7)
#define ARCADE_MK     (1 << 8)
#define ARCADE_HK     (1 << 9)
#define ARCADE_START  (1 << 10)
#define ARCADE_COIN   (1 << 11)

#define ARCADE_WIDTH  384
#define ARCADE_HEIGHT 224

const char *Arcade_Title(int game);        // "VAMPIRE SAVIOR"
const char *Arcade_SetFiles(int game);     // the zips it needs, for the player: "vsav.zip and vsavj.zip"
// Whether the player's set loads (false: why, in a line for the log; Arcade_SetFiles for the player)
bool Arcade_Check(int game, char *why, size_t whySize);

bool Arcade_Open(int game);                 // the board of that game, reset (false: its set doesn't load)
void Arcade_Close(void);
bool Arcade_IsOpen(void);
// One frame of the board (59.6 a second) with both players' controls
void Arcade_Frame(uint16_t p1, uint16_t p2);
const uint32_t *Arcade_Picture(void);       // ARCADE_WIDTH x ARCADE_HEIGHT, 0x00RRGGBB
// The frame's sound: stereo pairs at Arcade_SampleRate(); takes up to maxPairs, returns how many
int Arcade_TakeAudio(int16_t *out, int maxPairs);
int Arcade_SampleRate(void);
// Save states (online rollback): the board's whole state into buf (returns its size, 0 if it didn't fit) / back
size_t Arcade_SaveState(uint8_t *buf, size_t cap);
bool Arcade_LoadState(const uint8_t *buf, size_t size);
uint32_t Arcade_Hash(void);                 // of the board's memory, for desync checks
const char *Arcade_Folder(void);            // where the sets go: the "arcade" folder beside the game

#ifdef __cplusplus
}
#endif

#endif
