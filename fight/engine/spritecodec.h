#ifndef MUGEN_SPRITECODEC_H
#define MUGEN_SPRITECODEC_H

/*
 * SFF v2 sprite codecs (spritecodec.c): RLE8 (format 2), RLE5 (format 3)
 * and LZ5 (format 4), their encoders, the growable output buffer they write
 * into (GrowBuf, pngio.h), and the sprite decompression buffer: a fixed
 * block of memory in which the sprite cache keeps decoded sprites, evicting
 * the cheapest ones when it is full (DecompBuffer, game.h).
 *
 * Compressed sprite data starts with the decoded size (int); the decoders
 * skip it and do not check their output against it.
 */

/* Encoder of a compressed sprite format (g_spriteEncoders[format]): malloc'ed data, or NULL. */
typedef unsigned char *(__cdecl *SpriteEncodeFn)(const void *pixels, int w, int h, int *size_out);

/* Size prefix of compressed sprite data (the decoded size). */
#define SPRITE_CODEC_PREFIX 4

/* Initial size and growth step of the encoders' output buffers. */
#define SPRITE_CODEC_MIN_BUFFER 0x1000
#define SPRITE_CODEC_GROW_STEP  0x2000

/* Write position in a GrowBuf, passed by pointer to the encoders' helpers. */
typedef struct GrowBufWriter {
    struct GrowBuf *buf;    /* 0x00 */
    int pos;                /* 0x04 next byte written; only advanced while it stays inside the buffer */
} GrowBufWriter;
MUGEN_ASSERT_SIZE(GrowBufWriter, 0x08);

/* ---- LZ5 ---- */

/* Lz5Node.mode: how the encoder codes the bytes from a position. */
enum {
    LZ5_MODE_RLE_SHORT = 0,     /* 1 byte: length (2..7) << 5 | color; also single literals (length 1) */
    LZ5_MODE_RLE_LONG = 1,      /* 2 bytes: color, length - 8 (8..263) */
    LZ5_MODE_COPY_SHORT = 2,    /* 1 byte: length - 1 (1..64); the 8-bit offset - 1 is spread over 4 copies */
    LZ5_MODE_COPY_LONG = 3      /* 3 bytes: ((offset - 1) >> 2) & 0xC0, offset - 1, length - 3 (3..258) */
};

/* Longest copy distance and run lengths of the LZ5 parser. */
#define LZ5_MAX_OFFSET      0x400
#define LZ5_MAX_RUN         0x107
#define LZ5_MAX_COPY        0x102

/* Entries of Lz5_Parse's table: the cheapest way to code the data from each position. */
typedef struct Lz5Node {
    short len;              /* 0x00 bytes covered (0: never set) */
    short offset;           /* 0x02 copy distance (copy modes) */
    short mode;             /* 0x04 LZ5_MODE_* */
    short pad_06;           /* 0x06 never accessed by the game */
    int cost;               /* 0x08 bits needed for the data from here to the end */
    int pad_0c;             /* 0x0C never accessed by the game */
} Lz5Node;
MUGEN_ASSERT_SIZE(Lz5Node, 0x10);

/* ---- sprite decompression buffer ---- */

/*
 * Block of a DecompBuffer, followed by its payload. Blocks tile the buffer;
 * a block of size 0 ends the list (the last 16 bytes of the buffer).
 */
typedef struct DecompBlock {
    void *key;              /* 0x00 owner (the compressed sprite data), NULL for a free block */
    unsigned int size;      /* 0x04 bytes including this header, 0 for the end marker */
    int expire;             /* 0x08 DecompBuffer.clock + cost when last used: eviction priority */
    int cost;               /* 0x0C cost of decoding the sprite again (its compressed size) */
} DecompBlock;
MUGEN_ASSERT_SIZE(DecompBlock, 0x10);

/* Request to DecompBuffer_Get / DecompBuffer_Alloc (built on the stack by the sprite cache). */
typedef struct DecompRequest {
    struct DecompBuffer *pool;  /* 0x00 */
    unsigned int size;          /* 0x04 payload bytes */
    void *key;                  /* 0x08 */
    int cost;                   /* 0x0C */
} DecompRequest;
MUGEN_ASSERT_SIZE(DecompRequest, 0x10);

#endif /* MUGEN_SPRITECODEC_H */
