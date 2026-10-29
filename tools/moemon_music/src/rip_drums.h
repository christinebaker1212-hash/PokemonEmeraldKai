// Moemon Music Studio: Game Boy drums as the game's drums (Convert GB > GBA).
//
// A Game Boy driver plays each drum as a fixed little program on the noise channel: the same noise settings
// and volumes frame after frame, every hit. Hits are grouped by how they start (their first frames); each
// group's sound is made again from its longest hit (the noise generator at those settings and volumes), and
// compared with every drum sample in the game's kits as the engine plays it: its spectrum over time (40 bands,
// 6 ms steps, the first 400 ms), level matched. The closest is the group's drum.
#ifndef RIP_DRUMS_H
#define RIP_DRUMS_H

#include "inst.h"

#define RIP_DRUM_FRAMES 24

typedef struct {
    int len;                          // frames it sounds (to the next hit or its end)
    int shortMode;                    // the 7-bit noise
    float hz[RIP_DRUM_FRAMES];        // the noise clock each frame
    float vol[RIP_DRUM_FRAMES];       // its volume each frame (0-1, 1 = the channel's 15)
} RipHit;

typedef struct {
    int ngroups;
    int *groupOf;                     // per hit
    int key[64];                      // the kit key each group plays on
    int drum[64];                     // the game kit's key of the drum chosen (for the debug output)
    float gain[64];                   // the drum's level against the Game Boy hit's (engine level)
    float dist[64];
    int proto[64];                    // the hit it was heard from
    float protoVol[64];               // that hit's first-frame volume
    int hold[64];                     // frames its drum rings for (kicks, snares, toms, cymbals ring out; 0 = as the hit)
} RipDrums;

// Matches the hits; fills kit (its 128 voices: group g's drum on key[g]). Returns the number of groups (0 = none).
int rip_drums_match(const RipHit *hits, int n, CustomGroup *kit, RipDrums *out);
void rip_drums_free(RipDrums *d);

#endif
