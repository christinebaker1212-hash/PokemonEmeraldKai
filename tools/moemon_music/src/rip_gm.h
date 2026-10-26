// Moemon Music Studio: the game's closest General MIDI instrument for each original instrument of a rip.
#ifndef RIP_GM_H
#define RIP_GM_H

#include "rip.h"
#include "rip_song.h"

#define RIP_GM_MAX_DIST 14.0f    // further than this from every GM sound: the original sample stays

typedef struct {
    SfVoice voice[128];          // type 0xFF = no match (keep the original)
    int prog[128];               // GM program, 128 + key for a drum, -1 none
    float dist[128];
    float gain[128];             // brings the game sample to the original's loudness
} RipGm;

void rip_gm_match(const Rip *r, const RipSong *song, RipGm *out);

#endif
