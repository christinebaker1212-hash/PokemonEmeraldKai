#ifndef MUGEN_UTIL_H
#define MUGEN_UTIL_H

/* File kinds recognised by extension (File_GetType). */
enum {
    FILETYPE_UNKNOWN = 0,
    FILETYPE_PCX = 1,
    FILETYPE_SFF = 2,
    FILETYPE_WAV = 3,
    FILETYPE_SND = 4,
    FILETYPE_MP3 = 5,
    FILETYPE_MIDI = 6,
    FILETYPE_MOD = 7,   /* mod, s3m, xm */
    FILETYPE_DA = 8,
    FILETYPE_ZIP = 9,
    FILETYPE_OGG = 10,
    FILETYPE_PNG = 11
};

#endif /* MUGEN_UTIL_H */
