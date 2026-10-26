// Moemon Music Studio: the game's closest General MIDI instrument for each original instrument of a rip.
#ifndef RIP_GM_H
#define RIP_GM_H

#include "rip.h"
#include "rip_song.h"
#include "inst.h"

#define RIP_GM_MAX_DIST 14.0f    // further than this from every GM sound: the original sample stays

typedef struct {
    SfVoice voice[128];          // type 0xFF = no match (keep the original)
    int prog[128];               // GM program, 128 + key for a drum, -1 none
    float dist[128];
    float gain[128];             // brings the game sample to the original's loudness
    // Game Boy noise channels become drum kits made for them (each noise setting = the GM drum it
    // sounds like); the voices' sub is INST_CUSTOM + index. The caller takes the list.
    CustomGroup *groups;
    int ngroups;
    // GBS: every gain is scaled so none is over 1 (a volume can't go past 127); a slot left on its
    // Game Boy channel plays at this factor, so the mix stays as balanced as the original
    float cgbNorm;
} RipGm;

// GBS / Game Boy voices (squares, waves, noise) are matched by the sound they make, always (Convert GB > GBA).
// The song's custom samples must be bound (inst_custom_bind): the original voices' loudness is measured.
void rip_gm_match(const Rip *r, const RipSong *song, RipGm *out);

#endif
