// Moemon Music Studio: CHIPTUNE. Any song the studio plays (a MIDI, a project, every kind of rip) made
// into music for the NES's 2A03 or the Game Boy's sound chip, and written as a file of that console's
// own (.nsf / .gbs) with a small driver that plays it, so it plays in any NSF / GBS player and opens in
// the studio again like any other NES / Game Boy rip.
//
// How (chip.c): the song is played through the game's engine once, every voice read each frame (which
// part it belongs to, its pitch with bends and vibrato, how loud it is as heard, which instrument);
// then each frame's notes go onto the chip's channels:
//   - drums onto the noise channel, following each hit's own loudness and brightness frame by frame;
//   - the bass onto the triangle (NES) / wave channel (Game Boy);
//   - the lead onto square 1;
//   - everything else onto square 2, taking turns a frame at a time (the chip's arpeggio) when more
//     than one part plays, part by part, so every part is heard; a note's first frame always sounds;
//   - a note too low or too high for its channel moves by octaves; two parts on the same note are one;
//   - each instrument's tone picks its square's duty (12.5 / 25 / 50%: the one whose harmonics are
//     closest) and, on the Game Boy, the wave channel's 32 steps are drawn from the instrument itself.
#ifndef CHIP_H
#define CHIP_H

#include "studio.h"

enum { CHIP_NES, CHIP_GB };

// what a part of the song plays as (the arrangement table): AUTO = as the studio works it out
enum { CHIP_ROLE_AUTO, CHIP_ROLE_LEAD, CHIP_ROLE_HARMONY, CHIP_ROLE_ACCOMP, CHIP_ROLE_ECHO, CHIP_ROLE_BASS, CHIP_ROLE_OFF,
       CHIP_ROLE_DRUMS, CHIP_ROLE_COUNT };
enum { CHIP_CHORDS_AUTO, CHIP_CHORDS_HOLD, CHIP_CHORDS_ARP };
#define CHIP_MAX_PARTS 65            // the song's tracks (up to 64) and one for notes whose track isn't known

typedef struct {
    int target;              // CHIP_NES / CHIP_GB
    int chords;              // CHIP_CHORDS_*: a chord under the melody as an arpeggio (AUTO: when no harmony line plays)
    int arpFrames;           // frames each arpeggio note lasts (1-4)
    int dmc;                 // NES: kicks and snares as DMC samples (the noise keeps the hi-hats)
    int vibrato;             // a delayed vibrato on the melody's long notes that have none of their own
    // per part (the engine's track): CHIP_ROLE_*, duty (-1 auto, 0-3: 12.5 / 25 / 50 / 75%), octaves to move (-2..2)
    int role[CHIP_MAX_PARTS], duty[CHIP_MAX_PARTS], octave[CHIP_MAX_PARTS];
    char title[64], artist[64], copyright[64];   // the file's tags ("" = the song's)
} ChipOpts;

// The song's parts as the studio reads them (for the arrangement table)
typedef struct {
    int n;
    struct {
        int part;            // index into ChipOpts.role[] (the engine's track)
        int role;            // CHIP_ROLE_* worked out (never AUTO)
        int notes;
        int keyLo, keyHi;    // its range (MIDI keys)
        int duty;            // the duty it gets by itself (0-2)
        char what[48];       // a word on what it is ("melody in 3rds", "ostinato", "echo of track 3"...)
    } p[CHIP_MAX_PARTS];
} ChipPlan;

void chip_opts_default(ChipOpts *o, int target);
// Translates the open song and writes `path` (.nsf for the NES, .gbs for the Game Boy). msg: what was done
// (or why it failed). Returns 1 on success.
int chip_export(Studio *s, const ChipOpts *o, const char *path, char *msg, int msglen);
// Reads the song's parts as the export would (the options' overrides applied) without writing anything.
int chip_plan(Studio *s, const ChipOpts *o, ChipPlan *plan, char *msg, int msglen);
const char *chip_role_name(int role);
const char *chip_ext(int target);      // ".nsf" / ".gbs"
const char *chip_name(int target);     // "NES" / "Game Boy"

// A file this studio wrote: its length and loop (seconds; loop -1 = none). 0 when it isn't one.
int chip_file_info(const char *path, double *length, double *loopStart);

#endif
