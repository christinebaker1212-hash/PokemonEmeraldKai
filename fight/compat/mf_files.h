/* Moemon Fight: minizip opens its zips under the mugen/ folder, as the engine does (mf_port.h). */
#ifndef MF_FILES_H
#define MF_FILES_H
#include <stdio.h>
FILE *MfFile_Open(const char *path, const char *mode);
#define fopen(path, mode) MfFile_Open(path, mode)
#endif
