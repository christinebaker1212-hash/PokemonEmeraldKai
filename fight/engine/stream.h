#ifndef MUGEN_STREAM_H
#define MUGEN_STREAM_H

/* Stream kinds. */
enum {
    STREAM_FILE = 1,    /* stdio FILE */
    STREAM_MEMORY = 2,  /* caller-provided buffer */
    STREAM_ZIP = 3      /* file inside a .zip, inflated into memory */
};

/* Buffer allocators for Stream.buf_alloc. */
enum {
    STREAM_ALLOC_MALLOC = 0,
    STREAM_ALLOC_VIRTUAL = 1
};

/* g_streamError codes. */
enum {
    STREAM_ERR_NOMEM = -1,
    STREAM_ERR_OPEN = -2,
    STREAM_ERR_ZIP = -3,
    STREAM_ERR_EOF = -4,
    STREAM_ERR_READ = -5,
    STREAM_ERR_SEEK = -6,
    STREAM_ERR_TYPE = -10
};

/* Read-only file abstraction ("MFstream") over files, memory and zip entries. */
typedef struct Stream {
    int type;               /* 0x00 STREAM_* */
    FILE *fp;               /* 0x04 STREAM_FILE */
    void *zip;              /* 0x08 STREAM_ZIP: unzFile */
    unsigned char *begin;   /* 0x0C memory streams: data */
    unsigned char *end;     /* 0x10 */
    unsigned char *pos;     /* 0x14 read position */
    int owns_buf;           /* 0x18 begin is freed on close */
    int buf_alloc;          /* 0x1C STREAM_ALLOC_* */
    int eof;                /* 0x20 */
} Stream;
MUGEN_ASSERT_SIZE(Stream, 0x24);

/* SDL_RWops wrapper state (Stream_ToRWops). */
typedef struct StreamRW {
    Stream *stream;         /* 0x00 */
    int base;               /* 0x04 memory streams: offset of the RWops origin */
    int size;               /* 0x08 memory streams: bytes after base */
    int close_stream;       /* 0x0C close the stream with the RWops */
} StreamRW;
MUGEN_ASSERT_SIZE(StreamRW, 0x10);

/* SDL_RWops.type of RWops created by Stream_ToRWops. */
#define STREAM_RW_TYPE 0xE1ECB47Eu

#endif /* MUGEN_STREAM_H */
