#ifndef GUARD_ORRE_SOUND_H
#define GUARD_ORRE_SOUND_H

// ORRE's sound: Colosseum's own songs and sound effects, played from the disc's MusyX data in orre/orre_data.pak by the
// MusyX runtime (musyx/, AxioDL's decompilation with its PC renderer, CC0), through a GBA-style filter (the mix at
// the GBA's 13379 Hz, 8 bits). The bridge is musyx/orre_musyx.c (no game headers: MusyX has its own types).
// docs/ORRE_HANDOFF.md "Sound".
#include <stdint.h>

// Colosseum's soundPlay(id, volume, bgm): an id of its sound table (common_rel .data 0x13FE08): a song (the one
// playing fades out) or an effect. volume 0-127. 1 if it started
int OrreSound_Play(uint32_t id, uint32_t volume, int bgm);
void OrreSound_Stop(uint32_t id, uint32_t fadeMs);       // (that sound; a song fades)
void OrreSound_StopBgm(uint32_t fadeMs);                 // (the song playing, whichever it is)
void OrreSound_StopAmbient(uint32_t fadeMs);             // (the second song: the wind, started as an effect)
void OrreSound_Volume(uint32_t id, uint32_t volume, uint32_t fadeMs);
void OrreSound_MasterVolume(uint32_t volume, uint32_t fadeMs);
uint32_t OrreSound_Bgm(void);                            // (the song playing: its id, 0 none)
void OrreSound_Silence(void);                            // (leaving ORRE: everything off)
// the game's audio frame (stereo float, COUNT floats at RATE Hz): ORRE's sound added while ON, else held as it is
void OrreSound_Mix(float *buffer, int32_t count, int32_t rate, int on);

#endif
