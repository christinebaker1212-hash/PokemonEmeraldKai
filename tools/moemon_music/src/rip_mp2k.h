// Moemon Music Studio: reading an MP2K song (the game's own engine) out of a GBA ROM.
#ifndef RIP_MP2K_H
#define RIP_MP2K_H

#include <stddef.h>
#include <stdint.h>
#include "sf.h"
#include "inst.h"

typedef struct {
    uint8_t *mid;                // the tracks as a MIDI mid2agb converts back to the same commands
    size_t midLen;
    int ntracks, hasLoop;
    int priority, reverb;        // reverb -1 = the song doesn't set it
    SfVoice voice[128];          // the voicegroup
    CustomSample *custom;        // samples / waves the game doesn't have
    int ncustom;
    CustomGroup *groups;         // kits / key-split groups the game doesn't have
    int ngroups;
    CustomSplit *splits;
    int nsplits;
} Mp2kSong;

int rip_mp2k_song(const uint8_t *rom, size_t romLen, uint32_t header, const char *prefix, Mp2kSong *out, char *err, int errlen);
void rip_mp2k_free(Mp2kSong *s);

#endif
