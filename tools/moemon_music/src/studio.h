// Moemon Music Studio: the open project, its converted song and the engine song built from them.
#ifndef STUDIO_H
#define STUDIO_H

#include "project.h"
#include "convert.h"
#include "engine.h"

typedef struct {
    Project p;
    Conv c;
    int haveConv;
    EInst voices[128];
    float gain[128];          // each slot's volume factor (1 = as the game plays it)
    ESong song;
    // from a quiet run through the song
    int totalFrames;          // to the end, or to the loop end
    int loopStartFrame;       // -1 without a loop
    int peakVoices;           // most DirectSound channels busy at once
    char msg[512];
} Studio;

extern char g_biosDir[1024];     // where PlayStation rips look for a BIOS (settings / MMS_BIOS)

void studio_init(Studio *s);
// Converts the project's MIDI again (after settings or track mutes change).
int studio_rebuild(Studio *s);
// Rebuilds only the engine instruments (after an instrument change).
void studio_refresh_voices(Studio *s);
void studio_measure(Studio *s);
// Rip projects: sets the loudness so the song plays as loud as the original did (a quiet run of the
// first minute through the engine against the emulator's sound). Rebuilds. Returns the gain.
float studio_match_rip(Studio *s);
// Loudness of the engine's song and of the rip's own sound over the same stretch (RMS, full scale = 1).
void studio_rip_levels(Studio *s, int frames, double *kai, double *orig);

#endif
