// Moemon Music Studio: writing songs out for the game.
//  - .kaisong: one file holding the song, its voicegroup and the sample names it plays
//    (layout in docs/KAISONG_FORMAT.md)
//  - decomp sources: <song>.s + voicegroup .inc in the form the game's sound folder uses
#ifndef EXPORT_H
#define EXPORT_H

#include "project.h"
#include "convert.h"

#define KAISONG_VERSION 1           // songs with only the game's samples
#define KAISONG_VERSION_CUSTOM 2    // songs that bring their own samples (SMPL / WAVE chunks)

int export_kaisong(const Project *p, const Conv *c, const char *path, char *err, int errlen);
int export_decomp(const Project *p, const Conv *c, const char *dir, char *err, int errlen);
// The file name a .kaisong export gets: <song>@<place>.kaisong, <song>.kaisong, or the title's.
void export_default_name(const Project *p, char *buf, int n, const char *ext);
// Writes DIR/SONG_TABLE.md: every song and where it plays, with the .kaisong files in DIR (what the game's own page
// docs/SONG_TABLE.md is, made by tools/kaisong/gen.py for its music folder: the same text).
int export_song_table(const char *dir, char *err, int errlen);
// What the next export will leave out or warn about ("" if nothing).
void export_warnings(const Project *p, const Conv *c, char *buf, int n);

#endif
