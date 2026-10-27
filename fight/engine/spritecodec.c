/*
 * SFF v2 sprite codecs (original module 0x48C3E0-0x48D9AF): the growable
 * output buffer of the encoders, the RLE8 (format 2), LZ5 (format 4) and
 * RLE5 (format 3) encoders and decoders (g_spriteEncoders,
 * g_spriteDecoders), and the sprite decompression buffer in which the
 * sprite cache (0x477EC0) keeps decoded sprites.
 *
 * The encoders are not used by the game (nothing references
 * g_spriteEncoders); the decoders run when sprites are loaded or drawn.
 */
#include "mugen.h"

/* ------------------------------------------------------------------ GrowBuf */

/* 0x0048C3E0: a buffer of size zeroed bytes (none when size <= 0) growing by 1 KB. */
GrowBuf *GrowBuf_Create(int size)
{
    GrowBuf *buf = calloc(1, sizeof(GrowBuf));

    if (buf == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    if (size > 0) {
        unsigned char *data = calloc(1, size);
        if (data == NULL) {
            g_outOfMemory = 1;
        }
        buf->data = data;
        if (data == NULL) {
            free(buf);
            return NULL;
        }
    }
    buf->size = size;
    buf->grow_step = 0x400;
    return buf;
}

/* 0x0048C450: frees buf and its data. */
void GrowBuf_Free(GrowBuf *buf)
{
    if (buf == NULL) {
        return;
    }
    if (buf->data != NULL) {
        free(buf->data);
        buf->data = NULL;
    }
    free(buf);
}

/*
 * 0x0048C480: resizes buf to size bytes; growing rounds size up to a
 * multiple of grow_step and zeroes the new bytes. Shrinking reallocates to
 * exactly size. Returns 0 when the memory cannot be allocated (buf is kept).
 */
int GrowBuf_Resize(GrowBuf *buf, int size)
{
    int old = buf->size;
    unsigned char *data;

    if (size > old) {
        size = (buf->grow_step + size - 1) / buf->grow_step * buf->grow_step;
    }
    if (buf->data == NULL) {
        data = calloc(1, size);
        if (data == NULL) {
            g_outOfMemory = 1;
        }
        buf->data = data;
        if (data == NULL) {
            return 0;
        }
        buf->size = size;
        return 1;
    }
    data = realloc(buf->data, size);
    if (data == NULL) {
        return 0;
    }
    buf->data = data;
    if (size > old) {
        memset(data + old, 0, size - old);
    }
    buf->size = size;
    return 1;
}

/* Moves the write position by n, unless that leaves the buffer (the last byte is then rewritten). */
static void Writer_Advance(GrowBufWriter *w, int n)
{
    int pos = w->pos + n;

    if (pos >= 0 && pos < w->buf->size) {
        w->pos = pos;
    }
}

/* GrowBufWriter_PutByte, inlined by the original everywhere else. */
static int Writer_Put(GrowBufWriter *w, unsigned char value)
{
    if (w->pos + 2 > w->buf->size && !GrowBuf_Resize(w->buf, w->pos + 2)) {
        return 0;
    }
    w->buf->data[w->pos] = value;
    Writer_Advance(w, 1);
    return 1;
}

/* 0x0048C510: appends a byte (keeping one spare byte). Returns 0 when the buffer cannot grow. */
int GrowBufWriter_PutByte(GrowBufWriter *w, int value)
{
    return Writer_Put(w, (unsigned char)value);
}

/*
 * The encoders' output buffer for w x h pixels: n / 3 bytes rounded down to
 * 1 KB, at least SPRITE_CODEC_MIN_BUFFER, growing by SPRITE_CODEC_GROW_STEP,
 * starting with the decoded size. The writer starts after it (at 0 if the
 * buffer could not hold it).
 */
static GrowBuf *Codec_CreateOutput(GrowBufWriter *w, int n)
{
    int size = n / 3 / 1024 * 1024;
    GrowBuf *buf;

    if (size < SPRITE_CODEC_MIN_BUFFER) {
        size = SPRITE_CODEC_MIN_BUFFER;
    }
    buf = GrowBuf_Create(size);
    if (buf == NULL) {
        return NULL;
    }
    buf->grow_step = SPRITE_CODEC_GROW_STEP;
    w->buf = buf;
    w->pos = 0;
    if (buf->size >= SPRITE_CODEC_PREFIX + 1 || GrowBuf_Resize(buf, SPRITE_CODEC_PREFIX + 1)) {
        *(int *)buf->data = n;
        if (buf->size > SPRITE_CODEC_PREFIX) {
            w->pos = SPRITE_CODEC_PREFIX;
        }
    }
    return buf;
}

/* The encoder's result: buf's data cut to size bytes; buf itself is freed. */
static unsigned char *Codec_Finish(GrowBuf *buf, int size)
{
    unsigned char *data;

    GrowBuf_Resize(buf, size);
    data = buf->data;
    buf->data = NULL;
    free(buf);
    return data;
}

/* Frees buf and its data (an encoder failed). */
static void Codec_Discard(GrowBuf *buf)
{
    if (buf->data != NULL) {
        free(buf->data);
        buf->data = NULL;
    }
    free(buf);
}

/* --------------------------------------------------------------------- RLE8 */

/*
 * 0x0048C730: writes count (0..63) pixels of value: a single pixel that
 * does not look like a run header as itself, else a run header
 * (0x40 | count) and the value. Returns the bytes written, 0 on failure
 * (and for count 0).
 */
int Rle8_PutRun(int count, GrowBufWriter *w, int value)
{
    unsigned char n = (unsigned char)count;
    unsigned char v = (unsigned char)value;

    if (n == 0) {
        return 0;
    }
    if (n == 1 && (v & 0xC0) != 0x40) {
        return GrowBufWriter_PutByte(w, value) != 0;
    }
    if (!Writer_Put(w, n | 0x40)) {
        return 0;
    }
    return GrowBufWriter_PutByte(w, value) ? 2 : 0;
}

/*
 * 0x0048C550: RLE8-encodes w x h 8-bit pixels: runs of up to 63 pixels.
 * Returns the malloc'ed data (decoded size first), its size in *size_out,
 * or NULL. w * h == 0 still encodes the first pixel.
 */
unsigned char *Rle8_Encode(const void *pixels, int w, int h, int *size_out)
{
    const unsigned char *src = pixels;
    int n = w * h;
    const unsigned char *last = src + n - 1;
    const unsigned char *p;
    GrowBufWriter wr;
    GrowBuf *buf;
    unsigned char prev;
    unsigned char count;
    int total;
    int r;

    buf = Codec_CreateOutput(&wr, n);
    if (buf == NULL) {
        return NULL;
    }
    prev = src[0];
    p = src + 1;
    total = SPRITE_CODEC_PREFIX;
    count = 1;
    if (p <= last) {
        do {
            unsigned char c = *p++;
            if (c == prev) {
                count++;
                if (count == 0x3F) {
                    r = Rle8_PutRun(0x3F, &wr, prev);
                    if (r == 0) {
                        Codec_Discard(buf);
                        return NULL;
                    }
                    total += r;
                    count = 0;
                }
            } else {
                if (count != 0) {
                    r = Rle8_PutRun(count, &wr, prev);
                    if (r == 0) {
                        Codec_Discard(buf);
                        return NULL;
                    }
                    total += r;
                }
                prev = c;
                count = 1;
            }
        } while (p <= last);
        if (count == 0) {
            *size_out = total;
            return Codec_Finish(buf, total);
        }
    }
    r = Rle8_PutRun(count, &wr, prev);
    if (r == 0) {
        GrowBuf_Free(buf);
        return NULL;
    }
    total += r;
    *size_out = total;
    return Codec_Finish(buf, total);
}

/*
 * 0x0048C6D0: decodes size bytes of RLE8 data (after the decoded size) into
 * out: 0x40 | n followed by a value is a run of n pixels, any other byte is
 * a pixel. Nothing is checked against the output size. The original returns
 * nothing meaningful (its callers ignore the result); this returns 0.
 */
int Rle8_Decode(const void *data, size_t size, void *out)
{
    const unsigned char *p = (const unsigned char *)data + SPRITE_CODEC_PREFIX;
    const unsigned char *end = (const unsigned char *)data + size - 1;
    unsigned char *dst = out;

    while (p <= end) {
        unsigned char b = *p++;
        if ((b & 0xC0) == 0x40) {
            unsigned char v = *p++;
            int n = b & 0x3F;
            if (n != 0) {
                memset(dst, v, n);
                dst += n;
            }
        } else {
            *dst++ = b;
        }
    }
    return 0;
}

/* ---------------------------------------------------------------------- LZ5 */

/*
 * 0x0048D420: the length of the run of equal bytes starting at each of the n
 * positions of src (malloc'ed, n + 1 entries, the last 0). Lengths are 16-bit
 * and wrap. The allocation is not checked.
 */
short *Lz5_RunsForward(int n, const unsigned char *src)
{
    short *runs = malloc(n * 2 + 2);
    int i;

    runs[n - 1] = 1;
    runs[n] = 0;
    for (i = n - 2; i >= 0; i--) {
        runs[i] = 1;
        if (src[i] == src[i + 1]) {
            runs[i] = (short)(runs[i + 1] + 1);
        }
    }
    return runs;
}

/* 0x0048D480: the length of the run of equal bytes ending at each position (malloc'ed, n entries). */
short *Lz5_RunsBackward(const unsigned char *src, int n)
{
    short *runs = malloc(n * 2);
    int i;

    runs[0] = 1;
    for (i = 1; i < n; i++) {
        runs[i] = 1;
        if (src[i] == src[i - 1]) {
            runs[i] = (short)(runs[i - 1] + 1);
        }
    }
    return runs;
}

/*
 * 0x0048CF90: optimal parse of n bytes of 5-bit pixels for the LZ5 encoder:
 * from the end, the cheapest coding (in bits) of the data from each position
 * as a literal, a run or a copy of an earlier match (found through a hash
 * chain of byte pairs, at most LZ5_MAX_OFFSET back). Returns the calloc'ed
 * table of n + 1 nodes.
 *
 * Quirks kept: pixel values must be below 32, the pair hash indexes a
 * 1024-entry table on the stack; allocations are not checked; run lengths
 * are 16-bit; a candidate's position is moved back over a shared run and the
 * chain then continues from the moved position.
 */
Lz5Node *Lz5_Parse(int n, const unsigned char *src)
{
    int head[1024];
    short *runs_fwd;
    short *runs_back;
    Lz5Node *table;
    int *chain;
    int last = n - 1;
    int i, j;

    memset(head, 0, sizeof(head));
    runs_fwd = Lz5_RunsForward(n, src);
    runs_back = Lz5_RunsBackward(src, n);
    table = calloc(n + 1, sizeof(Lz5Node));
    chain = malloc(n * 4);
    for (j = 0; j < last; j++) {
        int key = src[j] * 32 + src[j + 1];
        chain[j] = head[key] - 1;
        head[key] = j + 1;
    }
    table[n - 1].cost = 9;
    table[n - 1].mode = LZ5_MODE_RLE_SHORT;
    table[n - 1].len = 1;

    for (i = n - 2; i >= 0; i--) {
        const unsigned char *cur = src + i;
        Lz5Node *e = &table[i];
        short run = runs_fwd[i];
        int maxlen = 1;
        int best_cost = 0x7FFFFFFF;
        int best_off = 0;
        int best_len = 0;
        int k;

        /* a literal, or a run */
        e->cost = table[i + 1].cost + 9;
        e->len = 1;
        k = run;
        if (k > LZ5_MAX_RUN) {
            k = LZ5_MAX_RUN;
        }
        for (; k >= 2; k--) {
            int cost = table[i + k].cost + 9;
            if (k > 7) {
                cost += 8;
            }
            if (cost < e->cost) {
                e->cost = cost;
                e->len = (short)k;
                e->mode = (short)(k > 7);
            }
        }

        /* copies of earlier data */
        for (j = chain[i]; j >= 0; j = chain[j]) {
            int dist = i - j;
            short back_j, back_i;
            int len;

            if (dist >= LZ5_MAX_OFFSET + 1) {
                break;
            }
            if (src[j + maxlen] != cur[maxlen]) {
                continue;
            }
            back_j = runs_back[j + maxlen];
            back_i = runs_back[i + maxlen];
            if (back_j != back_i && (back_j <= maxlen || back_i <= maxlen)) {
                continue;
            }
            if (run > 1) {
                /* start the match earlier, inside the run both positions are in */
                int shift = run - runs_fwd[j];
                if (shift < 0) {
                    shift = 0;
                }
                if (shift > runs_back[j] - 1) {
                    shift = runs_back[j] - 1;
                }
                if (dist <= 0x100 && shift - j + i > 0x100) {
                    shift = j - i + 0x100;
                }
                j -= shift;
                if (i - j > LZ5_MAX_OFFSET) {
                    j = i - LZ5_MAX_OFFSET;
                }
            }
            len = 2;
            if (cur[2] == src[j + 2]) {
                for (;;) {
                    short a, b;
                    if (i + len > last || len >= LZ5_MAX_COPY) {
                        break;
                    }
                    a = runs_fwd[i + len];
                    b = runs_fwd[j + len];
                    len += a > b ? b : a;
                    if (cur[len] != src[j + len]) {
                        break;
                    }
                }
                if (len > LZ5_MAX_COPY) {
                    len = LZ5_MAX_COPY;
                }
            }
            dist = i - j;
            if (dist > 0x100) {
                /* long copies only */
                if (len <= maxlen) {
                    continue;
                }
                for (k = len; k > maxlen; k--) {
                    int cost;
                    if (k <= run) {
                        break;
                    }
                    cost = table[i + k].cost + 0x19;
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_len = k;
                        best_off = dist;
                    }
                }
            } else {
                int short_len = len > 0x40 ? 0x40 : len;
                int seen = short_len;
                if (short_len > maxlen) {
                    k = short_len;
                    do {
                        int cost;
                        if (k < 8 && k <= run) {
                            break;
                        }
                        cost = table[i + k].cost + 0xF;
                        if (k > 0x40) {
                            seen = 1;
                            cost += 0xA;
                        } else {
                            seen = 0;
                        }
                        if (cost < best_cost) {
                            best_cost = cost;
                            best_len = k;
                            best_off = dist;
                        }
                        k--;
                    } while (k > maxlen);
                    if (maxlen < seen) {
                        maxlen = seen;
                    }
                }
                if (len <= maxlen) {
                    continue;
                }
                for (k = len; k > maxlen; k--) {
                    int cost;
                    if (k <= run) {
                        break;
                    }
                    cost = table[i + k].cost + 0xF;
                    if (k > 0x40) {
                        cost += 0xA;
                    }
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_len = k;
                        best_off = dist;
                    }
                }
            }
            if (maxlen < len) {
                maxlen = len;
            }
        }
        if (best_cost < e->cost) {
            e->cost = best_cost;
            e->mode = (short)(((best_len > 0x40 || best_off > 0x100) ? 1 : 0) + LZ5_MODE_COPY_SHORT);
            e->len = (short)best_len;
            e->offset = (short)best_off;
        }
    }
    free(chain);
    free(runs_fwd);
    free(runs_back);
    return table;
}

/*
 * 0x0048CC20: LZ5-encodes the pixels from src to last into w, following
 * Lz5_Parse: groups of 8 codes after a flag byte (bit set: copy), the low
 * offset bits of 4 short copies packed into the top bits of their length
 * bytes. Returns the bytes written, -1 on failure (the table leaks when
 * the buffer cannot grow).
 */
int Lz5_EncodeBlock(const unsigned char *last, GrowBufWriter *w, const unsigned char *src)
{
    int n = (int)(last - src) + 1;
    int start = w->pos;
    int i = 0;
    int bits = 0;
    int nrec = 0;
    int flag_pos = 0;
    unsigned char flags = 0;
    int rec[4];
    Lz5Node *table = Lz5_Parse(n, src);

    if (n > 0) {
        do {
            const Lz5Node *e;
            short len;
            unsigned char b;

            if (bits == 0) {
                flag_pos = w->pos;
                if (!Writer_Put(w, 0)) {
                    return -1;
                }
            }
            flags >>= 1;
            bits++;
            e = &table[i];
            len = e->len;
            if (len == 0) {
                fprintf(stderr, "Fatal error: table length 0 (mode %d) at offset %d (of %d)\n", e->mode, i, n);
                free(table);
                return -1;
            }
            switch (e->mode) {
            case LZ5_MODE_RLE_SHORT:
                b = (unsigned char)((unsigned char)len << 5 | src[i]);
                break;
            case LZ5_MODE_RLE_LONG:
                if (!Writer_Put(w, src[i])) {
                    return -1;
                }
                b = (unsigned char)((unsigned char)len - 8);
                break;
            case LZ5_MODE_COPY_SHORT:
                flags |= 0x80;
                rec[nrec++] = w->pos;
                if (!Writer_Put(w, (unsigned char)((unsigned char)len - 1))) {
                    return -1;
                }
                b = (unsigned char)((unsigned char)e->offset - 1);
                if (nrec == 4) {
                    unsigned char *data = w->buf->data;
                    data[rec[0]] |= b & 0xC0;
                    data[rec[1]] |= (unsigned char)(b << 2) & 0xC0;
                    data[rec[2]] |= (unsigned char)(b << 4) & 0xC0;
                    data[rec[3]] |= (unsigned char)(b << 6);
                    nrec = 0;
                    goto next;
                }
                break;
            case LZ5_MODE_COPY_LONG:
                flags |= 0x80;
                if (!Writer_Put(w, (unsigned char)(((e->offset - 1) >> 2) & 0xC0))) {
                    return -1;
                }
                if (!Writer_Put(w, (unsigned char)((unsigned char)e->offset - 1))) {
                    return -1;
                }
                b = (unsigned char)((unsigned char)len - 3);
                break;
            default:
                fprintf(stderr, "Got bad mode number %d: shouldn't get here!\n", e->mode);
                free(table);
                return -1;
            }
            if (!Writer_Put(w, b)) {
                return -1;
            }
        next:
            i += len;
            if (bits == 8) {
                w->buf->data[flag_pos] = flags;
                bits = 0;
            }
        } while (i < n);
        if (bits != 0) {
            w->buf->data[flag_pos] = (unsigned char)(flags >> (8 - bits));
        }
    }
    free(table);
    return w->pos - start;
}

/*
 * 0x0048CB10: LZ5-encodes w x h 5-bit pixels. Returns the malloc'ed data
 * (decoded size first), its size in *size_out, or NULL.
 */
unsigned char *Lz5_Encode(const void *pixels, int w, int h, int *size_out)
{
    const unsigned char *src = pixels;
    int n = w * h;
    GrowBufWriter wr;
    GrowBuf *buf;
    int size;

    buf = Codec_CreateOutput(&wr, n);
    if (buf == NULL) {
        return NULL;
    }
    size = Lz5_EncodeBlock(src + n - 1, &wr, src);
    if (size == -1) {
        Codec_Discard(buf);
        return NULL;
    }
    *size_out = size + SPRITE_CODEC_PREFIX;
    return Codec_Finish(buf, size + SPRITE_CODEC_PREFIX);
}

/*
 * 0x0048D4D0: decodes the LZ5 data from src to end into out. A flag byte
 * precedes each group of 8 codes; a set bit is a copy from earlier output
 * (the top 2 bits of 4 short copies form the next offset byte), a clear
 * bit a run of one color. Returns the bytes written. Nothing is checked:
 * copies may read before out.
 */
int Lz5_DecodeBlock(const unsigned char *end, const unsigned char *src, unsigned char *out)
{
    int n = (int)(end - src) + 1;
    int i = 0;
    int pos = 0;
    int bits = 0;
    int rec_bits = 0;
    unsigned char flags = 0;
    unsigned char rec = 0;

    if (n <= 0) {
        return 0;
    }
    do {
        unsigned char c;
        if (bits == 0) {
            flags = src[i++];
            bits = 8;
        }
        bits--;
        c = src[i];
        if (flags & 1) {
            int len = c & 0x3F;
            int offset;
            const unsigned char *from;
            unsigned char *to;
            int k;

            if (len != 0) {
                rec_bits += 2;
                rec = (unsigned char)(c >> 6 | rec << 2);
                len++;
                if (rec_bits == 8) {
                    offset = rec;
                    rec_bits = 0;
                } else {
                    offset = src[++i];
                }
            } else {
                offset = src[i + 1] | (c & 0xC0) << 2;
                i += 2;
                len = src[i] + 3;
            }
            from = out + pos - offset - 1;
            to = out + pos;
            for (k = len; k != 0; k--) {
                *to++ = *from++;
            }
            pos += len;
        } else {
            int len = c >> 5;
            if (len == 0) {
                len = src[++i] + 8;
            }
            memset(out + pos, c & 0x1F, len);
            pos += len;
        }
        flags >>= 1;
        i++;
    } while (i < n);
    return pos;
}

/* 0x0048CC00: decodes size bytes of LZ5 data (after the decoded size) into out; returns the bytes written. */
int Lz5_Decode(const void *data, size_t size, void *out)
{
    const unsigned char *d = data;

    return Lz5_DecodeBlock(d + size - 1, d + SPRITE_CODEC_PREFIX, out);
}

/* --------------------------------------------------------------------- RLE5 */

/*
 * 0x0048D920: writes the pixels from src to last as RLE5 bytes: up to 7
 * more repeats << 5 | color. Returns the bytes written, -1 on failure.
 */
int Rle5_PutPixels(const unsigned char *src, GrowBufWriter *w, const unsigned char *last)
{
    int count = 0;

    while (src <= last) {
        unsigned char c = *src++;
        int run = 0;
        while (*src == c && src <= last) {
            run++;
            src++;
            if (run >= 7) {
                break;
            }
        }
        if (!Writer_Put(w, (unsigned char)(run << 5 | c))) {
            return -1;
        }
        count++;
    }
    return count;
}

/*
 * 0x0048D720: RLE5-encodes the pixels from src to last into w: packets of
 * a run of one color (2-byte header: run length - 1, then 0x80 | the number
 * of RLE5 bytes, with the color byte after it unless it is 0) followed by
 * RLE5 bytes up to the next long run. Returns the bytes written, -1 on
 * failure.
 *
 * Quirks kept: the packet header pointer is taken before the buffer grows,
 * so when the buffer moves the header goes to the old block; a failed
 * header write is ignored; a failure of Rle5_PutPixels ors 0xFF into the
 * header and goes on.
 */
int Rle5_EncodeBlock(const unsigned char *src, GrowBufWriter *w, const unsigned char *last)
{
    int start = w->pos;

    while (src <= last) {
        unsigned char *hdr = w->buf->data + w->pos;
        const unsigned char *q;
        const unsigned char *lim;
        unsigned int first;
        int run;

        if (w->pos + 3 <= w->buf->size || GrowBuf_Resize(w->buf, w->pos + 3)) {
            *(unsigned short *)(w->buf->data + w->pos) = 0;
            Writer_Advance(w, 2);
        }
        first = *src++;
        run = 0;
        while (*src == first && src <= last) {
            run++;
            src++;
            if (run >= 0xFF) {
                break;
            }
        }
        hdr[0] = (unsigned char)run;
        hdr[1] = 0;
        if (first != 0) {
            if (!Writer_Put(w, (unsigned char)first)) {
                return -1;
            }
            hdr[1] |= 0x80;
        }
        if (src > last) {
            break;
        }
        lim = src + 0x7F;
        if (lim > last) {
            lim = last;
        }
        q = src;
        while (q <= lim) {
            const unsigned char *at = q;
            unsigned int c = *q++;
            run = 0;
            while (*q == c && q <= lim) {
                run++;
                q++;
                if (run >= 0xFF) {
                    break;
                }
            }
            if (run > (c != 0 ? 16 : 8)) {
                q = at;
                break;
            }
        }
        hdr[1] |= (unsigned char)Rle5_PutPixels(src, w, q - 1);
        src = q;
    }
    return w->pos - start;
}

/*
 * 0x0048D610: RLE5-encodes w x h 5-bit pixels. Returns the malloc'ed data
 * (decoded size first), its size in *size_out, or NULL.
 */
unsigned char *Rle5_Encode(const void *pixels, int w, int h, int *size_out)
{
    const unsigned char *src = pixels;
    int n = w * h;
    GrowBufWriter wr;
    GrowBuf *buf;
    int size;

    buf = Codec_CreateOutput(&wr, n);
    if (buf == NULL) {
        return NULL;
    }
    size = Rle5_EncodeBlock(src, &wr, src + n - 1);
    if (size == -1) {
        Codec_Discard(buf);
        return NULL;
    }
    *size_out = size + SPRITE_CODEC_PREFIX;
    return Codec_Finish(buf, size + SPRITE_CODEC_PREFIX);
}

/*
 * 0x0048D880: decodes the RLE5 packets from src to end into out. Returns
 * the bytes written. Nothing is checked against the output size.
 */
int Rle5_DecodeBlock(const unsigned char *src, const unsigned char *end, unsigned char *out)
{
    unsigned char *dst = out;

    while (src <= end) {
        const unsigned char *hdr = src;
        unsigned char ctrl = src[1];
        int count = ctrl & 0x7F;
        const unsigned char *p;
        int color = 0;

        src += 2;
        p = src;
        if (ctrl & 0x80) {
            color = *p++;
            src++;
        }
        memset(dst, color, hdr[0] + 1);
        dst += hdr[0] + 1;
        if (count > 0) {
            const unsigned char *p_last = p + count - 1;
            while (p <= p_last) {
                unsigned char b = *p++;
                int k = b >> 5;
                do {
                    *dst++ = b & 0x1F;
                } while (--k >= 0);
            }
        }
        src += count;
    }
    return (int)(dst - out);
}

/* 0x0048D700: decodes size bytes of RLE5 data (after the decoded size) into out; returns the bytes written. */
int Rle5_Decode(const void *data, size_t size, void *out)
{
    const unsigned char *d = data;

    return Rle5_DecodeBlock(d + SPRITE_CODEC_PREFIX, d + size - 1, out);
}

/* ------------------------------------------------- sprite decompression buffer */

static DecompBlock *DecompBlock_Next(const DecompBlock *blk)
{
    return (DecompBlock *)((unsigned char *)blk + blk->size);
}

/* Bytes a block needs for size payload bytes (the original's rounding). */
static unsigned int DecompBlock_Need(unsigned int size)
{
    return ((size + 0xF) & ~3u) + 4;
}

/*
 * 0x0048C830: empties buf: one free block covering it, then the end marker
 * in its last 16 bytes. Returns 1.
 */
int DecompBuffer_Reset(DecompBuffer *buf)
{
    DecompBlock *first = buf->data;
    DecompBlock *end = (DecompBlock *)((unsigned char *)buf->data + buf->size - sizeof(DecompBlock));

    first->size = buf->size - sizeof(DecompBlock);
    first->expire = 0;
    first->key = NULL;
    first->cost = 0;
    end->key = NULL;
    end->size = 0;
    end->expire = 0;
    end->cost = 0;
    buf->used_count = 0;
    buf->used_bytes = 0;
    buf->free_count = 1;
    buf->block_count = 1;
    return 1;
}

/* 0x0048C7B0: a decompression buffer of size bytes rounded up to 1 KB, or NULL. */
DecompBuffer *DecompBuffer_Create(unsigned int size)
{
    DecompBuffer *buf = calloc(1, sizeof(DecompBuffer));
    unsigned int bytes;

    if (buf == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    buf->size = 0;
    buf->data = NULL;
    buf->clock = 0;
    buf->hits = 0;
    buf->misses = 0;
    buf->block_count = 0;
    buf->used_count = 0;
    buf->free_count = 0;
    bytes = (((size - 1) >> 10) + 1) << 10;
    buf->evictions = 0;
    buf->used_bytes = 0;
    buf->data = malloc(bytes);
    if (buf->data == NULL) {
        free(buf);
        return NULL;
    }
    buf->size = bytes;
    DecompBuffer_Reset(buf);
    return buf;
}

/*
 * 0x0048CAA0: frees the used block blk (prev is the block before it, or
 * NULL) of buf and merges it with free neighbours. Returns the free block
 * holding it (prev when merged into it); free blocks are returned as is.
 */
DecompBlock *DecompBuffer_FreeBlock(DecompBlock *blk, DecompBuffer *buf, DecompBlock *prev)
{
    DecompBlock *next;

    if (blk->key == NULL) {
        return blk;
    }
    blk->key = NULL;
    blk->expire = 0;
    blk->cost = 0;
    buf->free_count++;
    buf->used_count--;
    buf->used_bytes -= blk->size;
    next = DecompBlock_Next(blk);
    if (next->size != 0 && next->key == NULL) {
        blk->size += next->size;
        next->size = 0;
        buf->block_count--;
        buf->free_count--;
    }
    if (prev != NULL && prev->key == NULL) {
        prev->size += blk->size;
        blk->size = 0;
        buf->block_count--;
        buf->free_count--;
        return prev;
    }
    return blk;
}

/*
 * 0x0048C910: makes room for req and gives it a block: the run of
 * consecutive blocks large enough with the lowest total expire value is
 * freed (used blocks in it are evicted), and split when the rest is worth
 * a block. buf->clock becomes that total, the block's expire clock + cost.
 * Returns the payload, or NULL when req does not fit.
 */
void *DecompBuffer_Alloc(const DecompRequest *req)
{
    DecompBuffer *buf = req->pool;
    unsigned int need = DecompBlock_Need(req->size);
    DecompBlock *start, *end, *cur;
    DecompBlock *before = NULL;
    DecompBlock *best_start = NULL;
    DecompBlock *best_before = NULL;
    DecompBlock *best_end = NULL;
    unsigned int step, span, rest;
    int best_sum = 0x7FFFFFFF;
    int sum;

    if (need > buf->size) {
        return NULL;
    }
    cur = buf->data;
    sum = cur->expire;
    step = cur->size;
    span = cur->size;
    start = cur;
    end = cur;
    if (step == 0) {
        return NULL;
    }
    for (;;) {
        /* drop the blocks before cur from the window */
        while (start != cur) {
            unsigned int size = start->size;
            sum -= start->expire;
            before = start;
            start = (DecompBlock *)((unsigned char *)start + size);
            span -= size;
        }
        /* extend it until it holds need bytes */
        if (span < need) {
            for (;;) {
                end = DecompBlock_Next(end);
                if (end->size == 0) {
                    break;
                }
                sum += end->expire;
                span += end->size;
                if (span >= need) {
                    break;
                }
            }
            if (span < need) {
                break;
            }
        }
        if (sum < best_sum) {
            best_start = start;
            best_before = before;
            best_end = end;
            best_sum = sum;
        }
        if (sum == 0) {
            break;
        }
        cur = (DecompBlock *)((unsigned char *)cur + step);
        step = cur->size;
        if (step == 0) {
            break;
        }
    }
    if (best_start == NULL) {
        return NULL;
    }
    {
        DecompBlock *blk = best_start;
        DecompBlock *prev = best_before;
        while (blk <= best_end) {
            if (blk->key != NULL) {
                blk = DecompBuffer_FreeBlock(blk, buf, prev);
                buf->evictions++;
            }
            prev = blk;
            blk = DecompBlock_Next(blk);
        }
    }
    buf->clock = best_sum;
    rest = best_start->size - need;
    if (rest >= 0x14) {
        DecompBlock *split = (DecompBlock *)((unsigned char *)best_start + need);
        best_start->size = need;
        split->size = rest;
        split->expire = 0;
        split->key = NULL;
        split->cost = 0;
        buf->block_count++;
        buf->free_count++;
    }
    best_start->cost = req->cost;
    best_start->key = req->key;
    best_start->expire = buf->clock + req->cost;
    buf->free_count--;
    buf->used_count++;
    buf->used_bytes += best_start->size;
    return best_start + 1;
}

/*
 * 0x0048C890: the block of req->key in its buffer: the payload, high dword
 * 1 when the block was there (its expire value is renewed), 0 when a new
 * block was allocated (a block too small for req is freed first; payload
 * NULL when nothing fits). Moemon Fight: the high dword is *found (a 64-bit
 * pointer doesn't fit beside it).
 */
void *DecompBuffer_Get(const DecompRequest *req, int *found)
{
    DecompBuffer *buf = req->pool;
    unsigned int need = DecompBlock_Need(req->size);
    DecompBlock *prev = NULL;
    DecompBlock *blk;

    *found = 0;
    if (need > buf->size) {
        return NULL;
    }
    blk = buf->data;
    if (blk->size != 0) {
        while (blk->key != req->key) {
            prev = blk;
            blk = DecompBlock_Next(blk);
            if (blk->size == 0) {
                buf->misses++;
                return DecompBuffer_Alloc(req);
            }
        }
        if (blk->size >= need) {
            blk->cost = req->cost;
            blk->expire = buf->clock + req->cost;
            buf->hits++;
            *found = 1;
            return blk + 1;
        }
        DecompBuffer_FreeBlock(blk, buf, prev);
    }
    buf->misses++;
    return DecompBuffer_Alloc(req);
}
