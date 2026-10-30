/*
 * Moemon Fight: resampled drawing of scaled sprites, for the software drawers (softrender.c, softblend.c).
 *
 * MUGEN draws a sprite at a size other than its own by stepping through it in 16.16 fixed point, one source
 * pixel per screen pixel (nearest). The cabinet's picture is 480x320, so 320x240 characters and stages are
 * drawn at 4/3 and the 640x480 screenpacks at 2/3: nearest made every third column and row two pixels wide
 * (4/3) or dropped every third (2/3, letters lost strokes), and which ones moved with the sprite's position,
 * so pixels crawled as the camera moved.
 *
 * Here each screen pixel is the area-weighted average of the source pixels under it (a box filter, exact for
 * the sprite's float position and size), so every source pixel covers the same 4/3 or 2/3 of the screen. A
 * screen pixel belongs to a sprite when its centre is inside the sprite's rectangle, as with nearest: sprites
 * that meet (a stage's tiles) still meet without a seam, and the rectangle's own edges stay hard. Inside it,
 * transparent source pixels (colour 0) lower the coverage, and the colour is blended in by it, so outlines
 * are as even as the rest. The drawer's own pixel operation (copy, subtract, add, alpha) is applied to the
 * averaged colour and the result mixed in by the coverage, so the intentional effects stay what they were
 * wherever a sprite fully covers a pixel. The same goes for rotated sprites (MfSoft_RotateResampled), sampled
 * 4x4 per screen pixel.
 *
 * Not fight state: the buffers here are the drawing's scratch, rebuilt for each sprite (this file isn't in
 * the Makefile's FIGHT_STATE_OBJS).
 */
#define MF_SYSTEM_HEAP  /* the cache below is the C library's memory, not the fight's heap (mf_mem.h) */
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mugen.h"
#include "mf_softscale.h"

#define MAX_AXIS 1024           /* screen pixels along one axis of one draw (the picture is 480x320) */
#define MAX_TAPS 24             /* source pixels averaged along one axis for one screen pixel */
#define RING 64                 /* source rows kept resampled across (more than MAX_TAPS) */

typedef struct Axis {
    short first[MAX_AXIS];      /* first source pixel (unflipped) of each screen pixel */
    unsigned char n[MAX_AXIS];  /* how many */
    signed char step[MAX_AXIS];     /* +-1 (flipped: -1), more when a big shrink samples only some of them */
    unsigned short w[MAX_AXIS][MAX_TAPS];   /* their weights in 256ths, summing to 256 */
} Axis;

/* A source row resampled across: per screen column, the RGB565 components times coverage and the coverage, in
 * 256ths, as four 16-bit lanes (r, g, b, coverage) of one 64-bit word: one multiply-add per source pixel */
typedef unsigned long long Px;

#define LANE(c, n) ((Px)(c) << (16 * (n)))

static Axis sCols, sRows;
static Px sRing[RING][MAX_AXIS];
static int sRingRow[RING];
static Px sPal[256];            /* the palette as lanes; colour 0 is nothing unless the draw is opaque */

int g_mfNearest = -1;

int MfSoft_Nearest(void)
{
    if (g_mfNearest < 0) {
        const char *e = getenv("MOEMON_FIGHT_NEAREST");
        g_mfNearest = e != NULL && e[0] == '1';
    }
    return g_mfNearest;
}

/* The weights of screen pixels p0..p0+count-1 along an axis where the sprite (size source pixels) spans
 * [pos, pos + scaled) on screen; flipped, the source pixels count from the other end (first and step say
 * where to read). */
static void Axis_Build(Axis *a, int p0, int count, double pos, double scaled, int size, int flip)
{
    double k = (double)size / scaled;       /* source pixels per screen pixel */
    double wt[MAX_TAPS];
    int i;

    for (i = 0; i < count; i++) {
        double s0 = ((double)(p0 + i) - pos) * k, s1 = s0 + k, sum = 0.0;
        int t0, t1, t, n = 0, step = 1, left = 256, big = 0;
        if (s0 < 0.0) {
            s0 = 0.0;
        }
        if (s1 > (double)size) {
            s1 = (double)size;
        }
        if (s1 <= s0) {
            /* a sliver past the sprite's end by rounding: its nearest source pixel */
            t = (int)floor(s0 < (double)size ? s0 : (double)size - 1);
            t = t < 0 ? 0 : t;
            a->first[i] = (short)(flip ? size - 1 - t : t);
            a->n[i] = 1;
            a->step[i] = 1;
            a->w[i][0] = 256;
            continue;
        }
        t0 = (int)floor(s0);
        t1 = (int)ceil(s1);
        if (t1 > size) {
            t1 = size;
        }
        while ((t1 - t0 + step - 1) / step > MAX_TAPS) {
            step++;
        }
        for (t = t0; t < t1 && n < MAX_TAPS; t += step) {
            double lo = t > s0 ? (double)t : s0, hi = t + step < s1 ? (double)(t + step) : s1;
            wt[n] = hi > lo ? hi - lo : 0.0;
            sum += wt[n++];
        }
        if (sum <= 0.0) {
            wt[0] = sum = 1.0;
        }
        /* in 256ths summing to exactly 256: the rounding left over goes to the biggest */
        for (t = 0; t < n; t++) {
            int q = (int)floor(wt[t] / sum * 256.0 + 0.5);
            a->w[i][t] = (unsigned short)q;
            left -= q;
            if (wt[t] > wt[big]) {
                big = t;
            }
        }
        a->w[i][big] = (unsigned short)(a->w[i][big] + left);
        a->first[i] = (short)(flip ? size - 1 - t0 : t0);
        a->n[i] = (unsigned char)n;
        a->step[i] = (signed char)(flip ? -step : step);
    }
}

/* d moved toward full by coverage a256 (0..256), per RGB565 component */
static unsigned short Mix565(unsigned int d, unsigned int full, int a256)
{
    int r = (int)(d >> 11), g = (int)(d >> 5 & 63), b = (int)(d & 31);
    int dr = (int)(full >> 11) - r, dg = (int)(full >> 5 & 63) - g, db = (int)(full & 31) - b;

    r += (dr * a256 + (dr >= 0 ? 128 : -128)) / 256;
    g += (dg * a256 + (dg >= 0 ? 128 : -128)) / 256;
    b += (db * a256 + (db >= 0 ? 128 : -128)) / 256;
    return (unsigned short)(r << 11 | g << 5 | b);
}

/* Applies the drawer's operation to the colour (sums of components times coverage, coverage cov out of
 * 65536) at *d, mixed in by the coverage. */
static void Put(unsigned short *d, unsigned int r, unsigned int g, unsigned int b, unsigned int cov, MfSoftPut put,
                void *ctx)
{
    unsigned int c;
    unsigned short full = *d;

    if (cov < 1024) {
        return;     /* under 1/64 */
    }
    if (cov >= 65536) {
        c = (r + 32768) >> 16 << 11 | (g + 32768) >> 16 << 5 | (b + 32768) >> 16;
    } else {
        c = (r + cov / 2) / cov << 11 | (g + cov / 2) / cov << 5 | (b + cov / 2) / cov;
    }
    put(&full, c, ctx);
    *d = cov >= 65280 ? full : Mix565(*d, full, (int)(cov >> 8));
}

/* Source row `row` (unflipped) resampled across the screen columns */
static const Px *Row(const Sprite *spr, const unsigned char *src, int row, int ncols, int flip_v)
{
    Px *out = sRing[row & (RING - 1)];
    const unsigned char *s;
    int ix;

    if (sRingRow[row & (RING - 1)] == row) {
        return out;
    }
    sRingRow[row & (RING - 1)] = row;
    s = src + spr->rows[flip_v ? spr->h - 1 - row : row];
    for (ix = 0; ix < ncols; ix++) {
        const unsigned char *p = s + sCols.first[ix];
        const unsigned short *wt = sCols.w[ix];
        int n = sCols.n[ix], st = (signed char)sCols.step[ix], i;
        Px acc = sPal[*p] * wt[0];
        for (i = 1; i < n; i++) {
            p += st;
            acc += sPal[*p] * wt[i];
        }
        out[ix] = acc;
    }
    return out;
}

/*
 * A big draw repeated as it was (a menu's background layers, a still camera) is kept: its colours and
 * coverages, keyed by everything they come from (the sprite, its pixels, the palette's colours, the exact
 * position and size, the flips and the clip), the least recently used of CACHE_SLOTS going first.
 */
#define CACHE_MIN_PIXELS 8192
#define CACHE_SLOTS 12

typedef struct Cached {
    const Sprite *spr;
    const unsigned char *src;
    double x0, y0, sw, sh;
    int flip_h, flip_v, opaque, ix0, iy0, ncols, nrows;
    unsigned int pal_hash, used;
    unsigned short *c;
    unsigned char *a;
    int cap;
} Cached;

static Cached sCache[CACHE_SLOTS];
static unsigned int sCacheClock;
static unsigned short sOutC[MAX_AXIS * 4];
static unsigned char sOutA[MAX_AXIS * 4];

/* Resamples the draw into colours c (RGB565) and coverages a (255: full; 0: nothing), ncols x nrows */
static void Compute(const Sprite *spr, const unsigned char *src, int ncols, int nrows, int flip_v,
                    unsigned short *c, unsigned char *a)
{
    int iy, ix, k;

    for (k = 0; k < RING; k++) {
        sRingRow[k] = -1;
    }
    for (iy = 0; iy < nrows; iy++) {
        int rn = sRows.n[iy], rs = sRows.step[iy], rf = sRows.first[iy], j;
        const Px *rows[MAX_TAPS];
        unsigned int wr[MAX_TAPS];
        for (j = 0; j < rn; j++) {
            rows[j] = Row(spr, src, rf + j * rs, ncols, flip_v);
            wr[j] = sRows.w[iy][j];
        }
        for (ix = 0; ix < ncols; ix++, c++, a++) {
            /* two 32-bit lanes per word now: (r, g) and (b, coverage) */
            unsigned long long rg = 0, ba = 0;
            unsigned int cov, r, g, b;
            for (j = 0; j < rn; j++) {
                Px p = rows[j][ix];
                rg += ((p & 0xFFFF) | (p & 0xFFFF0000ull) << 16) * wr[j];
                ba += ((p >> 32 & 0xFFFF) | (p >> 16 & 0xFFFF00000000ull)) * wr[j];
            }
            cov = (unsigned int)(ba >> 32);
            if (cov < 1024) {
                *a = 0;     /* under 1/64 */
                continue;
            }
            r = (unsigned int)rg;
            g = (unsigned int)(rg >> 32);
            b = (unsigned int)ba;
            if (cov >= 65280) {
                *c = (unsigned short)((r + cov / 2) / cov << 11 | (g + cov / 2) / cov << 5 | (b + cov / 2) / cov);
                *a = 255;
            } else {
                *c = (unsigned short)((r + cov / 2) / cov << 11 | (g + cov / 2) / cov << 5 | (b + cov / 2) / cov);
                *a = (unsigned char)(cov >> 8);
            }
        }
    }
}

/* Puts the resampled draw on the target through the drawer's operation (copy: a plain copy) */
static void Composite(const Sprite *tspr, unsigned char *dst, int ix0, int iy0, int ncols, int nrows,
                      const unsigned short *c, const unsigned char *a, int copy, MfSoftPut put, void *ctx)
{
    int iy, ix;

    for (iy = 0; iy < nrows; iy++) {
        unsigned short *d = (unsigned short *)(dst + tspr->rows[iy0 + iy]) + ix0;
        for (ix = 0; ix < ncols; ix++, d++, c++, a++) {
            unsigned short full;
            if (*a == 0) {
                continue;
            }
            if (*a == 255 && copy) {
                *d = *c;
                continue;
            }
            full = *d;
            put(&full, *c, ctx);
            *d = *a == 255 ? full : Mix565(*d, full, *a);
        }
    }
}

void MfSoft_ScaleResampled(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst,
                           double x0, double y0, double sw, double sh, int flip_h, int flip_v, int opaque,
                           const unsigned short *pal, MfSoftPut put, void *ctx, int copy)
{
    int w = spr->w, h = spr->h, ix0, ix1, iy0, iy1, ncols, nrows, k, ncol_pal;
    unsigned int hash = 2166136261u;
    Cached *slot = NULL;

    if (!(sw > 0.0) || !(sh > 0.0) || w <= 0 || h <= 0 || src == NULL || dst == NULL) {
        return;
    }
    /* the screen pixels whose centres are inside the rectangle, inside the clip */
    ix0 = (int)ceil(x0 - 0.5);
    ix1 = (int)ceil(x0 + sw - 0.5) - 1;
    iy0 = (int)ceil(y0 - 0.5);
    iy1 = (int)ceil(y0 + sh - 0.5) - 1;
    if (ix0 < g_clipX1) {
        ix0 = g_clipX1;
    }
    if (ix1 >= g_clipX2) {
        ix1 = g_clipX2 - 1;
    }
    if (iy0 < g_clipY1) {
        iy0 = g_clipY1;
    }
    if (iy1 >= g_clipY2) {
        iy1 = g_clipY2 - 1;
    }
    ncols = ix1 - ix0 + 1;
    nrows = iy1 - iy0 + 1;
    if (ncols <= 0 || nrows <= 0 || ncols > MAX_AXIS || nrows > MAX_AXIS) {
        return;
    }
    ncol_pal = spr->ncolors > 0 && spr->ncolors <= 256 ? spr->ncolors : 256;
    if (ncols * nrows >= CACHE_MIN_PIXELS) {
        Cached *oldest = &sCache[0];
        for (k = 0; k < ncol_pal; k++) {
            hash = (hash ^ (k == 0 && !opaque ? 0u : pal[k])) * 16777619u;
        }
        for (k = 0; k < CACHE_SLOTS; k++) {
            Cached *e = &sCache[k];
            if (e->spr == spr && e->src == src && e->x0 == x0 && e->y0 == y0 && e->sw == sw && e->sh == sh &&
                e->flip_h == flip_h && e->flip_v == flip_v && e->opaque == opaque && e->ix0 == ix0 &&
                e->iy0 == iy0 && e->ncols == ncols && e->nrows == nrows && e->pal_hash == hash && e->c != NULL) {
                e->used = ++sCacheClock;
                Composite(tspr, dst, ix0, iy0, ncols, nrows, e->c, e->a, copy, put, ctx);
                return;
            }
            if (e->used < oldest->used) {
                oldest = e;
            }
        }
        slot = oldest;
        if (slot->cap < ncols * nrows) {
            free(slot->c);
            free(slot->a);
            slot->c = malloc(sizeof(unsigned short) * (size_t)ncols * (size_t)nrows);
            slot->a = malloc((size_t)ncols * (size_t)nrows);
            slot->cap = slot->c != NULL && slot->a != NULL ? ncols * nrows : 0;
            if (slot->cap == 0) {
                free(slot->c);
                free(slot->a);
                slot->c = NULL;
                slot->a = NULL;
                slot = NULL;
            }
        }
    }
    Axis_Build(&sCols, ix0, ncols, x0, sw, w, flip_h);
    Axis_Build(&sRows, iy0, nrows, y0, sh, h, 0);
    for (k = 0; k < ncol_pal; k++) {
        unsigned int c = pal[k];
        sPal[k] = LANE(c >> 11, 0) | LANE(c >> 5 & 63, 1) | LANE(c & 31, 2) | LANE(1, 3);
    }
    for (; k < 256; k++) {
        sPal[k] = 0;
    }
    if (!opaque) {
        sPal[0] = 0;
    }
    if (slot != NULL) {
        Compute(spr, src, ncols, nrows, flip_v, slot->c, slot->a);
        slot->spr = spr;
        slot->src = src;
        slot->x0 = x0;
        slot->y0 = y0;
        slot->sw = sw;
        slot->sh = sh;
        slot->flip_h = flip_h;
        slot->flip_v = flip_v;
        slot->opaque = opaque;
        slot->ix0 = ix0;
        slot->iy0 = iy0;
        slot->ncols = ncols;
        slot->nrows = nrows;
        slot->pal_hash = hash;
        slot->used = ++sCacheClock;
        Composite(tspr, dst, ix0, iy0, ncols, nrows, slot->c, slot->a, copy, put, ctx);
        return;
    }
    /* small: a band of rows at a time through the scratch */
    {
        int band = (int)(sizeof(sOutA) / (size_t)ncols), y;
        for (k = 0; k < RING; k++) {
            sRingRow[k] = -1;
        }
        for (y = 0; y < nrows; y += band) {
            int n = nrows - y < band ? nrows - y : band;
            /* Compute walks sRows from its first entry: shift the band's rows to the front */
            if (y > 0) {
                memmove(sRows.first, sRows.first + band, sizeof(sRows.first[0]) * (size_t)(nrows - y));
                memmove(sRows.n, sRows.n + band, sizeof(sRows.n[0]) * (size_t)(nrows - y));
                memmove(sRows.step, sRows.step + band, sizeof(sRows.step[0]) * (size_t)(nrows - y));
                memmove(sRows.w, sRows.w + band, sizeof(sRows.w[0]) * (size_t)(nrows - y));
            }
            Compute(spr, src, ncols, n, flip_v, sOutC, sOutA);
            Composite(tspr, dst, ix0, iy0 + y, ncols, n, sOutC, sOutA, copy, put, ctx);
        }
    }
}

void MfSoft_RotateResampled(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst,
                            int ix0, int iy0, int ix1, int iy1, const double m[6], int opaque,
                            const unsigned short *pal, MfSoftPut put, void *ctx)
{
    int w = spr->w, h = spr->h, ix, iy;

    if (src == NULL || dst == NULL || w <= 0 || h <= 0) {
        return;
    }
    if (ix0 < g_clipX1) {
        ix0 = g_clipX1;
    }
    if (ix1 >= g_clipX2) {
        ix1 = g_clipX2 - 1;
    }
    if (iy0 < g_clipY1) {
        iy0 = g_clipY1;
    }
    if (iy1 >= g_clipY2) {
        iy1 = g_clipY2 - 1;
    }
    for (iy = iy0; iy <= iy1; iy++) {
        unsigned short *d = (unsigned short *)(dst + tspr->rows[iy]) + ix0;
        for (ix = ix0; ix <= ix1; ix++, d++) {
            /* is the pixel's centre on the sprite? (the hard edge, as with the scaled ones) */
            double cx = ix + 0.5, cy = iy + 0.5;
            double u = m[0] * cx + m[1] * cy + m[2], v = m[3] * cx + m[4] * cy + m[5];
            unsigned int r = 0, g = 0, b = 0, cov = 0;
            int sx, sy;
            if (!(u >= 0.0 && v >= 0.0 && u < (double)w && v < (double)h)) {
                continue;
            }
            /* 4x4 samples, each clamped onto the sprite */
            for (sy = 0; sy < 4; sy++) {
                for (sx = 0; sx < 4; sx++) {
                    double px = ix + (sx + 0.5) / 4.0, py = iy + (sy + 0.5) / 4.0;
                    int tu = (int)floor(m[0] * px + m[1] * py + m[2]);
                    int tv = (int)floor(m[3] * px + m[4] * py + m[5]);
                    unsigned int idx;
                    tu = tu < 0 ? 0 : tu >= w ? w - 1 : tu;
                    tv = tv < 0 ? 0 : tv >= h ? h - 1 : tv;
                    idx = src[spr->rows[tv] + tu];
                    if (idx != 0 || opaque) {
                        unsigned int c = pal[idx];
                        r += c >> 11;
                        g += c >> 5 & 63;
                        b += c & 31;
                        cov++;
                    }
                }
            }
            Put(d, r << 12, g << 12, b << 12, cov << 12, put, ctx);     /* 16 samples: 65536ths */
        }
    }
}

void MfSoft_RotateAt(const Sprite *spr, const unsigned char *src, const Sprite *tspr, unsigned char *dst, int ix,
                     int iy, int cosv, int sinv, int sw, int sh, double scale2_x, double scale2_y, int mirror,
                     int opaque, const unsigned short *pal, MfSoftPut put, void *ctx)
{
    /* the drawers' mapping (softrender.c Soft_Rotate): a screen pixel's offset (dx, dy) from (ix, iy), counted
     * from its centre, lands on u = ax + (cos dx / s2x - sin dy / s2y) / sx, v = ay + (sin dx / s2x + cos dy / s2y)
     * / sy, with cos, sin, sx, sy in 1024ths; mirrored, dx counts the other way */
    double cs = cosv / 1024.0, sn = sinv / 1024.0, sx = sw / 1024.0, sy = sh / 1024.0;
    double A = cs / (scale2_x * sx), B = -sn / (scale2_y * sx), C = sn / (scale2_x * sy), D = cs / (scale2_y * sy);
    double mx = mirror ? -1.0 : 1.0, cx0 = mirror ? ix + 0.5 : -(ix + 0.5), cy0 = -(iy + 0.5);
    double m[6], det, inv[4], xs[4], ys[4], minx, maxx, miny, maxy;
    int k;

    if (!(scale2_x != 0.0) || !(scale2_y != 0.0) || sw == 0 || sh == 0) {
        return;
    }
    m[0] = A * mx;
    m[1] = B;
    m[2] = spr->x + A * cx0 + B * cy0;
    m[3] = C * mx;
    m[4] = D;
    m[5] = spr->y + C * cx0 + D * cy0;
    det = m[0] * m[4] - m[1] * m[3];
    if (!(fabs(det) > 1e-12)) {
        return;
    }
    inv[0] = m[4] / det;
    inv[1] = -m[1] / det;
    inv[2] = -m[3] / det;
    inv[3] = m[0] / det;
    for (k = 0; k < 4; k++) {
        double u = (k & 1) ? spr->w : 0, v = (k & 2) ? spr->h : 0;
        xs[k] = inv[0] * (u - m[2]) + inv[1] * (v - m[5]);
        ys[k] = inv[2] * (u - m[2]) + inv[3] * (v - m[5]);
    }
    minx = maxx = xs[0];
    miny = maxy = ys[0];
    for (k = 1; k < 4; k++) {
        minx = xs[k] < minx ? xs[k] : minx;
        maxx = xs[k] > maxx ? xs[k] : maxx;
        miny = ys[k] < miny ? ys[k] : miny;
        maxy = ys[k] > maxy ? ys[k] : maxy;
    }
    if (maxx < g_clipX1 || minx > g_clipX2 || maxy < g_clipY1 || miny > g_clipY2) {
        return;
    }
    MfSoft_RotateResampled(spr, src, tspr, dst, (int)floor(minx) - 1, (int)floor(miny) - 1, (int)ceil(maxx) + 1,
                           (int)ceil(maxy) + 1, m, opaque, pal, put, ctx);
}

/*
 * A stage's parallax floor (bg.c BgElem_DrawParallaxSoft*): each source row is its own span on screen, further
 * out the lower it is (the shear MUGEN gives "xscale" floors, and the stretch of "width" ones). MUGEN put each
 * row at a whole pixel and its rows nearest, so the floor's rows stepped against each other as the camera
 * moved; here each row keeps its exact place, resampled across, and rows are mixed by how much of each a
 * screen row covers.
 */
static const MfParallaxRow *sParRows;
static int sParTile, sParX0, sParCols;

static const Px *ParRow(int row)
{
    Px *out = sRing[row & (RING - 1)];
    const MfParallaxRow *pr = &sParRows[row];
    double k, start = pr->start, width = pr->width;
    int x, srcw = pr->srcw;

    if (sRingRow[row & (RING - 1)] == row) {
        return out;
    }
    sRingRow[row & (RING - 1)] = row;
    if (!(width > 0.0) || srcw <= 0 || pr->src == NULL) {
        memset(out, 0, sizeof(Px) * (size_t)sParCols);
        return out;
    }
    k = srcw / width;
    for (x = 0; x < sParCols; x++) {
        double s0, s1, sum = 0.0, cx = sParX0 + x + 0.5;
        unsigned int q[MAX_TAPS];
        int idx[MAX_TAPS], t, n = 0, left = 256, big = 0;
        double wt[MAX_TAPS];
        Px acc = 0;
        if (!sParTile && !(cx >= start && cx < start + width)) {
            out[x] = 0;
            continue;
        }
        s0 = (sParX0 + x - start) * k;
        s1 = s0 + k;
        if (sParTile) {
            double f = floor(s0 / srcw) * srcw;
            s0 -= f;
            s1 -= f;
        } else {
            s0 = s0 < 0.0 ? 0.0 : s0;
            s1 = s1 > srcw ? srcw : s1;
            if (s1 <= s0) {
                s0 = s1 - 1e-6;
            }
        }
        for (t = (int)floor(s0); t < s1 && n < MAX_TAPS; t++) {
            double lo = t > s0 ? t : s0, hi = t + 1 < s1 ? t + 1 : s1;
            wt[n] = hi > lo ? hi - lo : 0.0;
            idx[n] = pr->src[((t % srcw) + srcw) % srcw];
            sum += wt[n++];
        }
        if (n == 0 || sum <= 0.0) {
            out[x] = 0;
            continue;
        }
        for (t = 0; t < n; t++) {
            q[t] = (unsigned int)floor(wt[t] / sum * 256.0 + 0.5);
            left -= (int)q[t];
            if (wt[t] > wt[big]) {
                big = t;
            }
        }
        q[big] = (unsigned int)((int)q[big] + left);
        for (t = 0; t < n; t++) {
            acc += sPal[idx[t]] * q[t];
        }
        out[x] = acc;
    }
    return out;
}

void MfSoft_Parallax(const MfParallaxRow *rows, int nrows_src, double top, double scale_y, int tile_x, int opaque,
                     const unsigned short *pal, int ncolors, const Sprite *tspr, unsigned char *dst)
{
    int iy0, iy1, nrows, iy, ix, k;

    if (rows == NULL || nrows_src <= 0 || !(scale_y > 0.0) || dst == NULL || g_clipX2 <= g_clipX1) {
        return;
    }
    iy0 = (int)ceil(top - 0.5);
    iy1 = (int)ceil(top + nrows_src * scale_y - 0.5) - 1;
    if (iy0 < g_clipY1) {
        iy0 = g_clipY1;
    }
    if (iy1 >= g_clipY2) {
        iy1 = g_clipY2 - 1;
    }
    nrows = iy1 - iy0 + 1;
    sParCols = g_clipX2 - g_clipX1;
    if (nrows <= 0 || nrows > MAX_AXIS || sParCols > MAX_AXIS) {
        return;
    }
    sParRows = rows;
    sParTile = tile_x;
    sParX0 = g_clipX1;
    Axis_Build(&sRows, iy0, nrows, top, nrows_src * scale_y, nrows_src, 0);
    for (k = 0; k < RING; k++) {
        sRingRow[k] = -1;
    }
    for (k = 0; k < (ncolors > 0 && ncolors <= 256 ? ncolors : 256); k++) {
        unsigned int c = pal[k];
        sPal[k] = LANE(c >> 11, 0) | LANE(c >> 5 & 63, 1) | LANE(c & 31, 2) | LANE(1, 3);
    }
    for (; k < 256; k++) {
        sPal[k] = 0;
    }
    if (!opaque) {
        sPal[0] = 0;
    }
    for (iy = 0; iy < nrows; iy++) {
        unsigned short *d = (unsigned short *)(dst + tspr->rows[iy0 + iy]) + sParX0;
        int rn = sRows.n[iy], rs = sRows.step[iy], rf = sRows.first[iy], j;
        const Px *rp[MAX_TAPS];
        unsigned int wr[MAX_TAPS];
        for (j = 0; j < rn; j++) {
            rp[j] = ParRow(rf + j * rs);
            wr[j] = sRows.w[iy][j];
        }
        for (ix = 0; ix < sParCols; ix++, d++) {
            unsigned long long rg = 0, ba = 0;
            unsigned int cov;
            for (j = 0; j < rn; j++) {
                Px p = rp[j][ix];
                rg += ((p & 0xFFFF) | (p & 0xFFFF0000ull) << 16) * wr[j];
                ba += ((p >> 32 & 0xFFFF) | (p >> 16 & 0xFFFF00000000ull)) * wr[j];
            }
            cov = (unsigned int)(ba >> 32);
            if (cov >= 1024) {
                unsigned int r = (unsigned int)rg, g = (unsigned int)(rg >> 32), b = (unsigned int)ba;
                unsigned short c = (unsigned short)((r + cov / 2) / cov << 11 | (g + cov / 2) / cov << 5 |
                                                    (b + cov / 2) / cov);
                *d = cov >= 65280 ? c : Mix565(*d, c, (int)(cov >> 8));
            }
        }
    }
}
