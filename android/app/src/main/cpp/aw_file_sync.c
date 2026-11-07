// The game's background file writer (include/aw_file.h, src/platform/aw_file.c needs SDL's threads) as plain stdio for
// the app: the card games' collection code (src/tt, src/tcg, src/ygo *_collection.c) writes through it. The file is built
// in memory and swapped in whole on Close, as the game's is, only on the calling thread.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aw_file.h"

struct AwFile { char *path, *data; size_t len, cap; };

AwFile *AwFile_Open(const char *path)
{
    AwFile *f = calloc(1, sizeof(*f));
    f->path = strdup(path);
    return f;
}

static void Grow(AwFile *f, size_t n)
{
    if (f->len + n <= f->cap)
        return;
    f->cap = (f->len + n) * 2 + 256;
    f->data = realloc(f->data, f->cap);
}

size_t AwFile_Write(const void *data, size_t size, size_t count, AwFile *f)
{
    size_t n = size * count;
    if (!f || !n)
        return 0;
    Grow(f, n);
    memcpy(f->data + f->len, data, n);
    f->len += n;
    return count;
}

int AwFile_Printf(AwFile *f, const char *fmt, ...)
{
    char buf[1024], *p = buf;
    int n;
    va_list ap;
    va_start(ap, fmt);
    n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0)
        return n;
    if ((size_t)n >= sizeof(buf)) {
        p = malloc(n + 1);
        va_start(ap, fmt);
        vsnprintf(p, n + 1, fmt, ap);
        va_end(ap);
    }
    AwFile_Write(p, 1, n, f);
    if (p != buf)
        free(p);
    return n;
}

int AwFile_Putc(int c, AwFile *f) { unsigned char b = (unsigned char)c; AwFile_Write(&b, 1, 1, f); return c; }
int AwFile_Puts(const char *s, AwFile *f) { AwFile_Write(s, 1, strlen(s), f); return 0; }

static void Free(AwFile *f)
{
    free(f->path);
    free(f->data);
    free(f);
}

void AwFile_WriteNow(const char *path, const void *data, size_t size)
{
    char tmp[1024];
    FILE *o;
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    o = fopen(tmp, "wb");
    if (!o)
        return;
    if (fwrite(data, 1, size, o) != size) {
        fclose(o);
        remove(tmp);
        return;
    }
    fclose(o);
    rename(tmp, path);
}

int AwFile_Close(AwFile *f)
{
    if (!f)
        return 0;
    AwFile_WriteNow(f->path, f->data ? f->data : "", f->len);
    Free(f);
    return 0;
}

void AwFile_Discard(AwFile *f) { if (f) Free(f); }
void AwFile_Flush(void) {}
void AwFile_Remove(const char *path) { remove(path); }
