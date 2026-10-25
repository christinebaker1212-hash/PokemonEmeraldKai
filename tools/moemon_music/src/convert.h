// Moemon Music Studio: MIDI -> the game's song bytecode, the way tools/mid2agb converts the
// game's own songs (-E exact gate times, no pattern compression), plus optional fixes for
// ordinary General MIDI files.
#ifndef CONVERT_H
#define CONVERT_H

#include <stdint.h>
#include "midi.h"

#define CONV_MAX_CAND 64     // MIDI track x channel pairs that have notes
#define CONV_MAX_OUT 16

enum { LOOP_MARKERS, LOOP_NONE, LOOP_WHOLE, LOOP_CUSTOM };

typedef struct {
    int masterVol;           // mid2agb -V (0-127)
    int exactGate;           // mid2agb -E
    int gmFixes;             // expression (CC11), RPN bend range, missing program changes, drum channel slot,
                             // tempo/markers from every MIDI track, notes clipped at the loop end, state re-sent at the loop start
    int loopMode;
    uint32_t loopStart, loopEnd;  // MIDI ticks, LOOP_CUSTOM
    uint64_t exclude;        // candidate tracks left out (bit per candidate)
} ConvOpts;

typedef struct {
    int midiTrack, chan;
    char name[64];
    int notes;
    int drum;                // MIDI channel 10
    int program;             // first program (-1 = none)
    int slot;                // first voice slot it selects (-1 = none / not converted)
} ConvCand;

typedef struct {
    int ncand;
    ConvCand cand[CONV_MAX_CAND];
    int ntracks;                       // tracks made (candidates not excluded), at most CONV_MAX_OUT
    int candOf[CONV_MAX_OUT];          // candidate each track came from
    uint8_t *data[CONV_MAX_OUT];
    int len[CONV_MAX_OUT];
    int nreloc[CONV_MAX_OUT];
    int *reloc[CONV_MAX_OUT];          // offsets of 4-byte GOTO targets in data
    uint8_t slotUsed[128];             // voice slots the tracks select
    uint8_t slotDrum[128];             // slot holds a drum kit (MIDI channel 10)
    int8_t slotGm[128];                // GM program the slot came from (-1 none)
    int dropped;                       // candidates over the 16-track limit
    uint32_t endTick;                  // last note off (MIDI ticks)
    uint32_t loopStartTick, loopEndTick;
    int hasLoop;
    uint32_t loopStartClock, loopEndClock, endClock;   // in song clocks (24 per beat)
    int division;
} Conv;

int conv_build(Conv *c, const Midi *m, const ConvOpts *o, char *err, int errlen);
void conv_free(Conv *c);

#endif
