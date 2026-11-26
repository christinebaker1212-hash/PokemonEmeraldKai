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

typedef struct {
    int target;              // CHIP_NES / CHIP_GB
    int arpFrames;           // chords: 0 = the note that matters most; 1-4 = a part's chord goes round its notes on
                             // square 2, this many frames each (1 = the classic arpeggio)
    char title[64], artist[64], copyright[64];   // the file's tags ("" = the song's)
} ChipOpts;

void chip_opts_default(ChipOpts *o, int target);
// Translates the open song and writes `path` (.nsf for the NES, .gbs for the Game Boy). msg: what was done
// (or why it failed). Returns 1 on success.
int chip_export(Studio *s, const ChipOpts *o, const char *path, char *msg, int msglen);
const char *chip_ext(int target);      // ".nsf" / ".gbs"
const char *chip_name(int target);     // "NES" / "Game Boy"

// A file this studio wrote: its length and loop (seconds; loop -1 = none). 0 when it isn't one.
int chip_file_info(const char *path, double *length, double *loopStart);

#endif
