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

// Defaults: settings every MIDI and rip opened starts with (projects keep their own). -1 = the song's own.
typedef struct {
    int gbaAccurate;          // -1, 0, 1
    int volume;               // -1 or 0-127
    int reverb;               // -2 the song's, -1 off, 0-127
    int balance, drumsFirst;  // -1, 0, 1 (GM fixes)
    int ripGm;                // rips: 0 the original instruments, 1 their closest GM instruments
    int gbToGba;              // Game Boy rips: 1 = Convert GB > GBA
    int attack, decay, sustain, release;   // -1 or 0-255, for sample instruments (Game Boy ones keep theirs)
    int octave;               // every instrument but drum kits moves this many octaves (-4..4)
} StudioDefaults;

extern StudioDefaults g_defaults;
void defaults_reset(StudioDefaults *d);
int defaults_load(StudioDefaults *d, const char *path);    // 0 when there's no file (d stays as it was)
int defaults_save(const StudioDefaults *d, const char *path);
// Opening a song: the project settings (before the first conversion), then the instruments (after it,
// when the slots are known). Each returns whether anything changed.
int studio_defaults_settings(Studio *s, const StudioDefaults *d);
int studio_defaults_voices(Studio *s, const StudioDefaults *d);

#endif
