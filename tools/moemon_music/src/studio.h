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
    ESong song;
    // from a quiet run through the song
    int totalFrames;          // to the end, or to the loop end
    int loopStartFrame;       // -1 without a loop
    int peakVoices;           // most DirectSound channels busy at once
    char msg[512];
} Studio;

void studio_init(Studio *s);
// Converts the project's MIDI again (after settings or track mutes change).
int studio_rebuild(Studio *s);
// Rebuilds only the engine instruments (after an instrument change).
void studio_refresh_voices(Studio *s);
void studio_measure(Studio *s);

#endif
