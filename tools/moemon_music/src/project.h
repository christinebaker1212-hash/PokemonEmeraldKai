// Moemon Music Studio: a project (the MIDI, its settings and every instrument choice) and its file.
#ifndef PROJECT_H
#define PROJECT_H

#include <stddef.h>
#include <stdint.h>
#include "midi.h"
#include "convert.h"
#include "sf.h"
#include "inst.h"

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
    // music ripped from another console (PSF, USF, GBS...): the MIDI above was made from it
    int isRip;
    int ripKind;
    char ripPath[1024];       // the rip, for the original-instruments player (played again on open)
    int ripTrack;             // GBS subsong
    char ripInfo[256];        // game / artist / console, for display
    SfVoice ripOrig[128];     // each slot as the original instrument
    SfVoice ripGm[128];       // each slot as its closest General MIDI instrument (type 0xFF = none)
    int ripUseGm;             // the instruments in use: 0 original, 1 General MIDI
    struct Rip *rip;          // the recording (its sound plays as "original"); NULL when the file is gone
    float ripGain;            // the loudness match (studio_match_rip)
    // samples the project brings (rips' original instruments); voices refer to them as INST_CUSTOM + i
    CustomSample *custom;
    int ncustom;
} Project;

void project_init(Project *p);
// Sets the track and voice limits from gbaAccurate.
void project_apply_mode(Project *p);
void project_free(Project *p);
// Loads a MIDI into a fresh project. A game MIDI (file named like a game song) starts with that
// song's instruments and settings.
int project_load_midi(Project *p, const char *path, char *err, int errlen);
int project_load(Project *p, const char *path, char *err, int errlen);
// Plays a rip through its console's emulator and makes a project from it. biosDir: PlayStation BIOS
// folder. gbsTrack: -1 = the .m3u's or the first.
int project_load_rip(Project *p, const char *path, const char *biosDir, int gbsTrack, char *err, int errlen);
// Rip projects: switch every slot between the original instruments and General MIDI ones.
void project_rip_instruments(Project *p, int useGm);
// Makes the MIDI again from the recording with a new loudness (instrument choices stay).
int project_rip_regain(Project *p, float gain, char *err, int errlen);
// Points the instrument layer at this project's custom samples (before converting or playing it).
void project_bind(Project *p);
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
