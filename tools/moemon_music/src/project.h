// Moemon Music Studio: a project (the MIDI, its settings and every instrument choice) and its file.
#ifndef PROJECT_H
#define PROJECT_H

#include <stddef.h>
#include <stdint.h>
#include "midi.h"
#include "convert.h"
#include "sf.h"

#define STUDIO_VERSION "1.0"

typedef struct {
    char path[1024];          // project file ("" until saved)
    char midiName[256];       // the MIDI's file name (kept for display / exports)
    uint8_t *midiData;        // the MIDI itself (projects carry it, so they open anywhere)
    size_t midiLen;
    Midi midi;
    char title[64];
    char target[64];          // game song it replaces, e.g. "mus_route101" ("" = not chosen)
    int masterVol;            // 0-127 (mid2agb -V)
    int reverb;               // -1 = off, else 0-127 (mid2agb -R)
    int priority;             // 0-255 (mid2agb -P)
    int exactGate;
    int gmFixes;
    int loopMode;
    int gbaAccurate;          // off by default; on: play and export like the game (10 tracks, 5 sample voices), off: 64 / 64
    int moreTracks;           // derived from gbaAccurate (project_apply_mode)
    int voices;               // derived: sample voices mixed at once
    int drumsFirst;           // GM fixes: drum tracks win voices over melodic ones
    int balance;              // GM fixes: even out instrument volumes (the game's piano as the reference)
    uint32_t loopStart, loopEnd;   // MIDI ticks (LOOP_CUSTOM)
    uint64_t muted;           // candidate tracks left out
    uint64_t solo;            // candidate tracks soloed (listening only, not saved)
    SfVoice voice[128];
    uint8_t voiceSet[128];    // slot has a choice (otherwise the GM match is used)
    int dirty;
} Project;

void project_init(Project *p);
// Sets the track and voice limits from gbaAccurate.
void project_apply_mode(Project *p);
void project_free(Project *p);
// Loads a MIDI into a fresh project. A game MIDI (file named like a game song) starts with that
// song's instruments and settings.
int project_load_midi(Project *p, const char *path, char *err, int errlen);
int project_load(Project *p, const char *path, char *err, int errlen);
int project_save(Project *p, const char *path, char *err, int errlen);
// Fills every used slot that has no choice yet with its closest General MIDI match.
void project_default_voices(Project *p, const Conv *c, int force);
// Sets every used slot to what a game song's voicegroup has there.
void project_use_song_group(Project *p, const Conv *c, int group);
void project_conv_opts(const Project *p, ConvOpts *o);
uint8_t project_reverb_byte(const Project *p);

int read_file(const char *path, uint8_t **data, size_t *len);
const char *path_base(const char *path);

#endif
