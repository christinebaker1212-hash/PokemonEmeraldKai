// Moemon Music Studio: writing songs out for the game.
//  - .kaisong: one file holding the song, its voicegroup and the sample names it plays
//    (layout in docs/KAISONG_FORMAT.md)
//  - decomp sources: <song>.s + voicegroup .inc in the form the game's sound folder uses
#ifndef EXPORT_H
#define EXPORT_H

#include "project.h"
#include "convert.h"

#define KAISONG_VERSION 1

int export_kaisong(const Project *p, const Conv *c, const char *path, char *err, int errlen);
int export_decomp(const Project *p, const Conv *c, const char *dir, char *err, int errlen);
// What the next export will leave out or warn about ("" if nothing).
void export_warnings(const Project *p, const Conv *c, char *buf, int n);

#endif
