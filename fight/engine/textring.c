/*
 * Ring buffer of text lines (original module around 0x4661E0).
 */
#include "mugen.h"

/* 0x004661E0 */
TextRing *TextRing_Create(int width, int count)
{
    TextRing *ring;
    char **line;
    char *p;

    ring = calloc(1, sizeof(TextRing));
    if (ring == NULL) {
        return NULL;
    }
    ring->width = width;
    ring->count = count;
    ring->lines = calloc(count, sizeof(char *));
    if (ring->lines == NULL) {
        if (ring->buf != NULL) {
            free(ring->buf);
        }
        free(ring);
        return NULL;
    }
    ring->buf = calloc(width * count, 1);
    if (ring->buf == NULL) {
        TextRing_Free(ring);
        return NULL;
    }
    p = ring->buf;
    for (line = ring->lines; count > 0; count--) {
        *line++ = p;
        p += width;
    }
    return ring;
}

/* 0x00466280 */
void TextRing_Free(TextRing *ring)
{
    if (ring == NULL) {
        return;
    }
    if (ring->lines != NULL) {
        free(ring->lines);
    }
    if (ring->buf != NULL) {
        free(ring->buf);
    }
    free(ring);
}

/*
 * 0x00466480: appends the ring's lines (the last max_lines, or all for -1)
 * to out, one per line, within size bytes of out's capacity.
 *
 * As in the original, left is not checked against the line length, so a
 * line longer than the remaining room wraps the counter around.
 */
void TextRing_Dump(TextRing *ring, char *out, int max_lines, unsigned int size)
{
    unsigned int left = size;
    int i;

    if (ring == NULL || --left == 0) {
        return;
    }
    i = ring->head;
    if (max_lines != -1) {
        int last = ring->cur;
        if (last < i) {
            last += ring->count;
        }
        if (last - i + 1 > max_lines) {
            i = ring->cur - max_lines;
            if (i < 0) {
                i += ring->count;
            }
        }
    }
    while (i != ring->cur) {
        const char *line = ring->lines[i];
        unsigned int len = (unsigned int)strlen(line);
        strncat(out, line, left);
        left -= len;
        if (left == 0) {
            goto done;
        }
        strncat(out, "\n", left);
        if (--left == 0) {
            goto done;
        }
        if (++i >= ring->count) {
            i = 0;
        }
    }
    if (ring->lines[i][0] != '\0') {
        strncat(out, ring->lines[i], left);
    }
done:
    out[size - 1] = '\0';
}

/* 0x004662B0: clears the ring, then prints into it. */
void TextRing_SetPrintf(TextRing *ring, const char *fmt, ...)
{
    char buf[LOG_LINE_SIZE];
    va_list ap;

    ring->cur = 0;
    ring->head = 0;
    ring->lines[0][0] = '\0';
    va_start(ap, fmt);
    _vsnprintf(buf, LOG_LINE_SIZE, fmt, ap);
    va_end(ap);
    TextRing_Append(ring, buf);
}

/* 0x00466320 */
void TextRing_Printf(TextRing *ring, const char *fmt, ...)
{
    char buf[LOG_LINE_SIZE];
    va_list ap;

    va_start(ap, fmt);
    _vsnprintf(buf, LOG_LINE_SIZE, fmt, ap);
    va_end(ap);
    TextRing_Append(ring, buf);
}

/* 0x00466380 */
void TextRing_VPrintf(TextRing *ring, va_list ap, const char *fmt)
{
    char buf[LOG_LINE_SIZE];

    _vsnprintf(buf, LOG_LINE_SIZE, fmt, ap);
    TextRing_Append(ring, buf);
}

/*
 * 0x004663E0: appends text to the current line. A newline, or a full line,
 * moves on to the next line, overwriting the oldest one when the ring is full.
 */
void TextRing_Append(TextRing *ring, const char *text)
{
    int width = ring->width;
    int count = ring->count;
    char *line = ring->lines[ring->cur];
    int len = (int)strlen(line);

    for (; *text != '\0'; text++) {
        if (len < width - 1 && *text != '\n') {
            line[len++] = *text;
            continue;
        }
        line[len] = '\0';
        if (++ring->cur >= count) {
            ring->cur = 0;
        }
        if (ring->cur == ring->head && ++ring->head >= count) {
            ring->head = 0;
        }
        line = ring->lines[ring->cur];
        len = 0;
        if (*text != '\n') {
            line[len++] = *text;
        }
    }
    line[len] = '\0';
}
