/*
 * Read-only streams over files, memory buffers and zip entries
 * (original module 0x466A30-0x467500). Errors are reported in g_streamError.
 */
#include "mugen.h"

static int Stream_IsMemory(const Stream *s)
{
    return (unsigned int)(s->type - STREAM_MEMORY) < 2;
}

/* 0x00466A30 */
int Stream_GetByte(Stream *s)
{
    if (s->type == STREAM_FILE) {
        return fgetc(s->fp);
    }
    if (Stream_IsMemory(s)) {
        if (s->pos < s->end) {
            return *s->pos++;
        }
        s->eof = 1;
    }
    return -1;
}

/* 0x00466A70: fgets() semantics; memory streams also stop after '\r'. */
char *Stream_Gets(Stream *s, char *buf, int size)
{
    char *dst = buf;
    int left;

    if (s->type == STREAM_FILE) {
        char *r = fgets(buf, size, s->fp);
        if (r == NULL) {
            g_streamError = ferror(s->fp) ? STREAM_ERR_READ : STREAM_ERR_EOF;
        }
        return r;
    }
    if (!Stream_IsMemory(s)) {
        g_streamError = STREAM_ERR_TYPE;
        return NULL;
    }
    if (s->pos >= s->end) {
        s->eof = 1;
        g_streamError = STREAM_ERR_EOF;
        return NULL;
    }
    for (left = size - 1; left != 0; left--) {
        char c;
        if (s->pos == s->end) {
            s->eof = 1;
            break;
        }
        c = (char)*s->pos++;
        *dst++ = c;
        if (c == '\n' || c == '\r') {
            break;
        }
    }
    *dst = '\0';
    return buf;
}

/* 0x00466B20: fread() semantics. */
size_t Stream_Read(void *buf, size_t size, size_t count, Stream *s)
{
    if (s->type == STREAM_FILE) {
        size_t n = fread(buf, size, count, s->fp);
        if (n < count) {
            g_streamError = ferror(s->fp) ? STREAM_ERR_READ : STREAM_ERR_EOF;
        }
        return n;
    }
    if (Stream_IsMemory(s)) {
        size_t total = size * count;
        if (s->pos + total > s->end) {
            size_t avail = (size_t)(s->end - s->pos);
            memcpy(buf, s->pos, avail);
            s->pos += avail;
            s->eof = 1;
            g_streamError = STREAM_ERR_EOF;
            return avail / size;
        }
        memcpy(buf, s->pos, total);
        s->pos += total;
        return count;
    }
    g_streamError = STREAM_ERR_TYPE;
    return 0;
}

/* 0x00466BF0: feof() semantics; streams of an unknown type are always at the end. */
int Stream_Eof(Stream *s)
{
    if (s->type == STREAM_FILE) {
        return feof(s->fp);
    }
    if (Stream_IsMemory(s)) {
        return s->eof;
    }
    return 1;
}

/* 0x00466C20 */
Stream *Stream_OpenFile(const char *path, const char *mode)
{
    Stream *s = Mem_Calloc(1, sizeof(Stream));

    if (s == NULL) {
        g_streamError = STREAM_ERR_NOMEM;
        return NULL;
    }
    s->type = STREAM_FILE;
    s->fp = fopen(path, mode);
    if (s->fp == NULL) {
        g_streamError = STREAM_ERR_OPEN;
        free(s);
        return NULL;
    }
    return s;
}

/* 0x00466C90: the stream does not own the buffer. */
Stream *Stream_OpenMemory(void *data, int size)
{
    Stream *s = Mem_Calloc(1, sizeof(Stream));

    if (s == NULL) {
        g_streamError = STREAM_ERR_NOMEM;
        return NULL;
    }
    s->pos = data;
    s->begin = data;
    s->end = (unsigned char *)data + size;
    s->type = STREAM_MEMORY;
    return s;
}

/* 0x00466CE0: inflates the zip entry name into a new buffer. */
int Stream_ZipLoad(Stream *s, const char *name)
{
    unz_file_info info;

    if (s->type != STREAM_ZIP) {
        g_streamError = STREAM_ERR_TYPE;
        return 0;
    }
    if (unzLocateFile(s->zip, name, 0) == UNZ_OK &&
        unzOpenCurrentFile(s->zip) == UNZ_OK &&
        unzGetCurrentFileInfo(s->zip, &info, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK) {
        unsigned char *buf = Mem_Calloc(1, info.uncompressed_size);
        s->pos = buf;
        s->begin = buf;
        s->owns_buf = 1;
        s->end = buf + info.uncompressed_size;
        while (!unzeof(s->zip)) {
            if (unzReadCurrentFile(s->zip, s->begin, info.uncompressed_size) < 0) {
                unzCloseCurrentFile(s->zip);
                g_streamError = STREAM_ERR_ZIP;
                return 0;
            }
        }
        s->eof = 0;
        return 1;
    }
    g_streamError = STREAM_ERR_ZIP;
    return 0;
}

/* 0x00466E90 */
void Stream_FreeBuffer(Stream *s)
{
    if (!s->owns_buf || s->begin == NULL) {
        return;
    }
    if (s->buf_alloc == STREAM_ALLOC_MALLOC) {
        free(s->begin);
    } else if (s->buf_alloc == STREAM_ALLOC_VIRTUAL) {
        VirtualFree(s->begin, 0, MEM_DECOMMIT);
    }
    s->end = NULL;
    s->begin = NULL;
    s->owns_buf = 0;
    s->buf_alloc = 0;
}

/* 0x00466DB0: returns 1 when the zip entry closed cleanly (CRC ok). */
int Stream_ZipUnload(Stream *s)
{
    int ok = unzCloseCurrentFile(s->zip) == UNZ_OK;

    Stream_FreeBuffer(s);
    return ok;
}

/* 0x00466E20 */
Stream *Stream_OpenZip(const char *zip_path, const char *name)
{
    Stream *s = Mem_Calloc(1, sizeof(Stream));

    if (s == NULL) {
        g_streamError = STREAM_ERR_NOMEM;
        return NULL;
    }
    s->type = STREAM_ZIP;
    s->zip = unzOpen(zip_path);
    if (s->zip == NULL) {
        g_streamError = STREAM_ERR_OPEN;
    } else if (Stream_ZipLoad(s, name)) {
        return s;
    }
    free(s);
    return NULL;
}

/* 0x00466EE0 */
int Stream_Close(Stream *s)
{
    if (s == NULL) {
        return -1;
    }
    switch (s->type) {
    case STREAM_FILE:
        fclose(s->fp);
        break;
    case STREAM_MEMORY:
        Stream_FreeBuffer(s);
        s->begin = NULL;
        break;
    case STREAM_ZIP:
        Stream_ZipUnload(s);
        unzClose(s->zip);
        break;
    }
    free(s);
    return 0;
}

/* 0x00466F50: fseek() semantics; returns 0 on success. */
int Stream_Seek(Stream *s, long offset, int origin)
{
    unsigned char *base;
    unsigned char *target;

    if (s->type == STREAM_FILE) {
        int r = fseek(s->fp, offset, origin);
        if (r != 0) {
            g_streamError = STREAM_ERR_SEEK;
        }
        return r;
    }
    if (!Stream_IsMemory(s)) {
        g_streamError = STREAM_ERR_TYPE;
        return 1;
    }
    s->eof = 0;
    if (origin == SEEK_SET) {
        base = s->begin;
    } else if (origin == SEEK_CUR) {
        base = s->pos;
    } else if (origin == SEEK_END) {
        base = s->end;
    } else {
        g_streamError = STREAM_ERR_TYPE;
        return 1;
    }
    target = base + offset;
    if (target >= s->begin && target <= s->end) {
        s->pos = target;
        return 0;
    }
    g_streamError = STREAM_ERR_SEEK;
    return 1;
}

/* 0x00466FF0 */
long Stream_Tell(Stream *s)
{
    if (s->type == STREAM_FILE) {
        return ftell(s->fp);
    }
    if (Stream_IsMemory(s)) {
        return (long)(s->pos - s->begin);
    }
    return -1;
}

/* 0x00467020 */
long Stream_GetSize(Stream *s)
{
    long cur = Stream_Tell(s);
    long size;

    Stream_Seek(s, 0, SEEK_END);
    size = Stream_Tell(s);
    Stream_Seek(s, cur, SEEK_SET);
    return size;
}

/* 0x00467130: returns 0, or -1 at end of stream. */
int Stream_ReadU8(Stream *s, unsigned char *out)
{
    int c = Stream_GetByte(s);

    if (c == -1) {
        return -1;
    }
    *out = (unsigned char)c;
    return 0;
}

/* 0x00467180: little endian. */
int Stream_ReadU16(Stream *s, unsigned short *out)
{
    int lo, hi;

    if ((lo = Stream_GetByte(s)) == -1 || (hi = Stream_GetByte(s)) == -1) {
        return -1;
    }
    *out = (unsigned short)(hi << 8 | lo);
    return 0;
}

/* 0x00467210: little endian. */
int Stream_ReadU32(Stream *s, unsigned int *out)
{
    int b0, b1, b2, b3;

    if ((b0 = Stream_GetByte(s)) == -1 || (b1 = Stream_GetByte(s)) == -1 ||
        (b2 = Stream_GetByte(s)) == -1 || (b3 = Stream_GetByte(s)) == -1) {
        return -1;
    }
    *out = (unsigned int)b3 << 24 | (unsigned int)b2 << 16 | (unsigned int)b1 << 8 | (unsigned int)b0;
    return 0;
}

/* ---- SDL_RWops over a stream ---- */

/*
 * 0x00467300. For memory streams SEEK_CUR computes an absolute position but
 * still passes SEEK_CUR to Stream_Seek, and SEEK_END is refused; kept as is.
 */
static int SDLCALL StreamRW_Seek(SDL_RWops *rw, int offset, int whence)
{
    StreamRW *ctx = rw->hidden.unknown.data1;
    Stream *s = ctx->stream;
    long pos;

    switch (s->type) {
    case STREAM_FILE:
        pos = offset;
        break;
    case STREAM_MEMORY:
        if (whence == SEEK_SET) {
            if (offset < 0) {
                return -1;
            }
            pos = offset + ctx->base;
        } else if (whence == SEEK_CUR) {
            pos = Stream_Tell(s) + offset;
            if (pos < ctx->base) {
                return -1;
            }
        } else {
            return -1;
        }
        break;
    default:
        return 1;
    }
    Stream_Seek(s, pos, whence);
    return Stream_Tell(ctx->stream);
}

/* 0x004673B0 */
static int SDLCALL StreamRW_Read(SDL_RWops *rw, void *ptr, int size, int maxnum)
{
    StreamRW *ctx = rw->hidden.unknown.data1;
    int n = (int)Stream_Read(ptr, size, maxnum, ctx->stream);

    return n < 0 ? -1 : n;
}

/* 0x004673E0 */
static int SDLCALL StreamRW_Write(SDL_RWops *rw, const void *ptr, int size, int num)
{
    return -1;
}

/* 0x004673F0 */
static int SDLCALL StreamRW_Close(SDL_RWops *rw)
{
    StreamRW *ctx = rw->hidden.unknown.data1;

    if (ctx->close_stream) {
        Stream_Close(ctx->stream);
    }
    free(ctx);
    SDL_FreeRW(rw);
    return 0;
}

/* 0x00467420: RWops that reads through the stream (file and memory streams only). */
SDL_RWops *Stream_ToRWops(Stream *s)
{
    SDL_RWops *rw;
    StreamRW *ctx;

    if ((unsigned int)(s->type - STREAM_FILE) > 1) {
        return NULL;
    }
    rw = SDL_AllocRW();
    if (rw == NULL) {
        return NULL;
    }
    ctx = Mem_Calloc(1, sizeof(StreamRW));
    if (ctx == NULL) {
        SDL_FreeRW(rw);
        return NULL;
    }
    rw->type = STREAM_RW_TYPE;
    rw->close = StreamRW_Close;
    rw->seek = StreamRW_Seek;
    rw->write = StreamRW_Write;
    rw->read = StreamRW_Read;
    ctx->stream = s;
    ctx->close_stream = 0;
    if (s->type == STREAM_MEMORY) {
        ctx->base = Stream_Tell(s);
        ctx->size = Stream_GetSize(s) - ctx->base;
    }
    rw->hidden.unknown.data1 = ctx;
    return rw;
}

/* 0x004674C0: plain SDL RWops over the remaining data (file and memory streams only). */
SDL_RWops *Stream_ToSDLRWops(Stream *s)
{
    unsigned char *pos, *begin;

    if (s->type == STREAM_FILE) {
        return SDL_RWFromFP(s->fp, 0);
    }
    if (s->type != STREAM_MEMORY) {
        return NULL;
    }
    pos = s->pos;
    begin = s->begin;
    return SDL_RWFromMem(s->pos, Stream_GetSize(s) - (int)(pos - begin));
}
