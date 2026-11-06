#ifndef GUARD_AW_FILE_H
#define GUARD_AW_FILE_H
// Moemon Co-op: files written in the background (src/platform/aw_file.c). A sidecar (".coop", ".life", ".cards"...)
// is built in memory with the same calls stdio has, and a worker thread writes it and swaps it in for the old one,
// so the game never waits on the disk ("Moemon is not a demanding game, and it should never be hitching": the user,
// 2026-11-06). A newer copy of a file still waiting replaces the older one. AwFile_Flush waits until every queued file
// is on disk (a save, a SAVE FILE switch, quitting).
#include <stddef.h>

typedef struct AwFile AwFile;

AwFile *AwFile_Open(const char *path);   // (never NULL)
size_t AwFile_Write(const void *data, size_t size, size_t count, AwFile *f);   // as fwrite
int AwFile_Printf(AwFile *f, const char *fmt, ...);
int AwFile_Putc(int c, AwFile *f);
int AwFile_Puts(const char *s, AwFile *f);
int AwFile_Close(AwFile *f);   // queues it (0)
void AwFile_Discard(AwFile *f);   // (nothing written)
void AwFile_Flush(void);
void AwFile_WriteNow(const char *path, const void *data, size_t size);   // a whole buffer, queued the same way
void AwFile_Remove(const char *path);   // the file deleted now (a write of it still queued is dropped)
#endif
