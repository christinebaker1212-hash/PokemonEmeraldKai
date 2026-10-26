// Moemon Music Studio: a rip's recording -> a MIDI the studio converts like any other, plus the
// original instruments as custom samples (8-bit, at about the GBA's 13379 Hz as heard).
#ifndef RIP_SONG_H
#define RIP_SONG_H

#include <stddef.h>
#include <stdint.h>
#include "rip.h"
#include "sf.h"
#include "inst.h"

#define RIP_GBA_RATE 13379

typedef struct {
    uint8_t *mid;                // Standard MIDI File: 24 ticks a beat at 150 BPM = one tick a game frame
    size_t midLen;
    int hasLoop;
    uint32_t loopStart, loopEnd; // ticks (= frames)
    int slotSample[128];         // the rip sample each voice slot plays (-1 = none)
    SfVoice orig[128];           // the slot as the original instrument (a custom sample)
    int gm[128];                 // closest General MIDI program (-1 = none close enough); drums: 128 + GM drum key
    float gmDist[128];           // how far that match is (0 = the same sound)
    int nslots;
    int reverb;                  // -1 or 0-127 for the project
    int voicesUsed;              // chip voices that sound
    float slotKey[128];          // the key the slot's notes are around (median), 0 = none
    float slotSustain[128];      // what's left of a note 0.5 s after its peak (median), -1 = notes too short
    float slotDur[128];          // median note length (s)
    uint8_t slotNoise[128];      // played at one pitch all the time (a drum)
    float *keyDur;               // noise slots: [slot * 128 + key] median note length (s) at each key, -1 none
    // Game Boy noise slots: the drum kit made for each (rip_drums.c), and how loud its loudest drum has to be
    CustomGroup *drumKit[128];   // NULL = none
    float drumGain[128];
    int gbaDrums;                // the MIDI plays the noise slots as those kits (Convert GB > GBA)
    int hardwareLevels;          // only Game Boy channels: their volume is the hardware's (no loudness match)
    char prefix[24];             // custom sample names start with this
    CustomSample *custom;        // the original instruments (voices' refs are INST_CUSTOM + index)
    int ncustom;
} RipSong;

// Builds the MIDI and the instruments. gain scales every note (1 = the chip's own level, as the
// translation measures it; studio_match_rip finds the gain that matches the original's loudness).
// gbaDrums: Game Boy noise notes go to the kits made for them (Convert GB > GBA), as drum hits.
int rip_song_build(Rip *r, RipSong *out, float gain, int gbaDrums, char *err, int errlen);
void rip_song_free(RipSong *s);

// Analysis used by the build (and tests)
float rip_detect_root(const int16_t *pcm, int len, int rate, int loopStart, int loops, int *pitched);
// Resamples to `toRate` (windowed sinc), keeping a loop seamless. Returns new length; *loopStart updated.
int rip_resample(const int16_t *in, int len, int fromRate, int loopStart, int loops,
                 double toRate, int16_t **out, int *outLoopStart);

#endif
