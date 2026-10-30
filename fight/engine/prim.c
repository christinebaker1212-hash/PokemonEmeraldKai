/*
 * Drawing primitives (original module 0x485780-0x4886DF): clearing the
 * render target, pixels, lines, rectangle outlines and filled / blended
 * rectangles, for the software renderer (16-bit RGB565 targets) and for the
 * OpenGL renderer. Gfx_SelectRenderer points the g_gfx* entries at the
 * versions of the renderer in use (tables g_gfxClearFns..g_gfxPresentFns).
 *
 * Software versions: colours are RGB565; the plain ones go through the global
 * PalFX (PalFx_Apply565), the additive / subtractive ones add the raw colour
 * with saturation per component. Coordinates are inclusive and clipped to
 * g_clipX1..g_clipX2-1, g_clipY1..g_clipY2-1. Pixels are addressed through
 * the row offset table of the target sprite (Sprite.rows), whose stride is
 * w * bytes per pixel, not the SDL surface pitch.
 *
 * Floating point follows the x87 code of the original: values the original
 * keeps on the FPU stack are doubles here (the FPU runs with 53-bit
 * precision), values it stores are floats; float to int conversions are those
 * of __ftol2_sse (Ftol).
 *
 * Quirks kept from the original:
 * - Gfx_SoftClear on a raw 16 or 32-bit target writes pixels 1..w*h (one past
 *   the end) instead of 0..w*h-1;
 * - the additive / subtractive lines and rectangles ignore the PalFX (they
 *   call PalFx_Apply565 and drop the result), the plain rectangle outline
 *   applies it twice;
 * - vertical lines step by g_targetWidth * 2 bytes, not by the row table;
 * - Gfx_SoftAddPixel and Gfx_SoftInvertPixel assume a 16-bit target;
 * - Gfx_SoftDrawLineRaster draws nothing for vertical lines drawn upwards and
 *   horizontal lines drawn leftwards (they cannot happen through
 *   Gfx_SoftDrawLine, which only sends diagonal lines there).
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "mugen.h"

/* ---- helpers ---- */

/* The render target sprite (g_renderTarget must be set). */
static Sprite *Prim_Target(void)
{
    const KeyedArray *a = g_renderTarget->sprites;

    return (Sprite *)(a->data + a->elem_size * g_renderTargetSprite);
}

/* Pixels of the render target sprite: those of its SDL surface, or its raw data. */
static unsigned char *Prim_TargetPixels(const Sprite *spr)
{
    if (spr->format == SPRITE_FORMAT_SURFACE) {
        return ((SDL_Surface *)spr->pixels)->pixels;
    }
    return spr->pixels;
}

/* RGB565 components of a colour, for the additive and subtractive versions. */
#define PRIM_R(c)   ((c) & 0xF800)
#define PRIM_G(c)   ((c) & 0x07E0)
#define PRIM_B(c)   ((c) & 0x001F)

/* d + (ar, ag, ab) per component, each capped at its maximum (unsigned compare). */
static unsigned short Prim_Add565(unsigned int d, unsigned int ar, unsigned int ag, unsigned int ab)
{
    unsigned int r = PRIM_R(d) + ar;
    unsigned int g = PRIM_G(d) + ag;
    unsigned int b = PRIM_B(d) + ab;

    if (r > 0xF800) {
        r = 0xF800;
    }
    if (g > 0x07E0) {
        g = 0x07E0;
    }
    if (b > 0x001F) {
        b = 0x001F;
    }
    return (unsigned short)(r | g | b);
}

/* d + (sr, sg, sb) per component (negated components), negative results become 0. */
static unsigned short Prim_Sub565(unsigned int d, int sr, int sg, int sb)
{
    int r = (int)PRIM_R(d) + sr;
    int g = (int)PRIM_G(d) + sg;
    int b = (int)PRIM_B(d) + sb;

    if (r < 0) {
        r = 0;
    }
    if (g < 0) {
        g = 0;
    }
    if (b < 0) {
        b = 0;
    }
    return (unsigned short)(r | g | b);
}

/* Half of d (each component shifted right), the base of the 50% blends. */
static unsigned int Prim_Half565(unsigned int d)
{
    return (d >> 1) & 0x7BEF;
}

/* Sorts lo <= hi. */
static void Prim_Order(int *lo, int *hi)
{
    if (*hi < *lo) {
        int t = *lo;
        *lo = *hi;
        *hi = t;
    }
}

/* ---- software renderer ---- */

/*
 * 0x00485780: g_gfxClear of the software renderers: fills the render target
 * with an RGB565 colour (through the global PalFX).
 */
void Gfx_SoftClear(unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned int c = PalFx_Apply565(color);
    int bytes;
    int n;

    if (spr->format == SPRITE_FORMAT_SURFACE) {
        SDL_FillRect(spr->pixels, NULL, c);
        return;
    }
    bytes = (spr->bpp + 7) >> 3;
    if (bytes == 1) {
        memset(spr->pixels, (unsigned char)c, spr->pixels_size);
    } else if (bytes == 2) {
        /* Quirk: indices w*h down to 1. */
        for (n = spr->h * spr->w; n != 0; n--) {
            ((unsigned short *)spr->pixels)[n] = (unsigned short)c;
        }
    } else if (bytes == 4) {
        for (n = spr->h * spr->w; n != 0; n--) {
            ((unsigned int *)spr->pixels)[n] = c;
        }
    }
}

/* Pixel address of (x, y) in the target, 16-bit. */
static unsigned short *Prim_Pixel16(const Sprite *spr, unsigned char *pixels, int x, int y)
{
    return (unsigned short *)(pixels + spr->rows[y] + x * 2);
}

/* 1 when (x, y) is inside the clip rectangle. */
static int Prim_InClip(int x, int y)
{
    return x >= g_clipX1 && x < g_clipX2 && y >= g_clipY1 && y < g_clipY2;
}

/* 0x00485860: g_gfxPutPixel (software): sets pixel (x, y) to an RGB565 colour (PalFX applied). */
void Gfx_SoftPutPixel(int x, int y, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned int c = PalFx_Apply565(color);
    SDL_Surface *surface = NULL;
    unsigned char *pixels;

    if (spr->format == SPRITE_FORMAT_SURFACE) {
        surface = spr->pixels;
        pixels = surface->pixels;
    } else {
        pixels = spr->pixels;
    }
    if (pixels == NULL || !Prim_InClip(x, y)) {
        return;
    }
    switch ((spr->bpp + 7) >> 3) {
    case 1:
        pixels[spr->rows[y] + x] = (unsigned char)c;
        break;
    case 2:
        *Prim_Pixel16(spr, pixels, x, y) = (unsigned short)c;
        break;
    case 4:
        *(unsigned int *)(pixels + spr->rows[y] + x * 4) = c;
        break;
    }
    if (surface != NULL) {
        SDL_UpdateRect(surface, x, y, 1, 1);
    }
}

/* 0x00485960: g_gfxAddPixel (software): adds an RGB565 colour (PalFX applied) to pixel (x, y). */
void Gfx_SoftAddPixel(int x, int y, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned int c = PalFx_Apply565(color);
    SDL_Surface *surface = NULL;
    unsigned char *pixels;
    unsigned short *p;

    if (spr->format == SPRITE_FORMAT_SURFACE) {
        surface = spr->pixels;
        pixels = surface->pixels;
    } else {
        pixels = spr->pixels;
    }
    if (pixels == NULL || !Prim_InClip(x, y)) {
        return;
    }
    p = Prim_Pixel16(spr, pixels, x, y);
    *p = Prim_Add565(*p, PRIM_R(c), PRIM_G(c), PRIM_B(c));
    if (surface != NULL) {
        SDL_UpdateRect(surface, x, y, 1, 1);
    }
}

/* 0x00485AA0: g_gfxInvertPixel (software): inverts pixel (x, y). */
void Gfx_SoftInvertPixel(int x, int y)
{
    Sprite *spr = Prim_Target();
    SDL_Surface *surface = NULL;
    unsigned char *pixels;
    unsigned short *p;

    if (spr->format == SPRITE_FORMAT_SURFACE) {
        surface = spr->pixels;
        pixels = surface->pixels;
    } else {
        pixels = spr->pixels;
    }
    if (pixels == NULL || !Prim_InClip(x, y)) {
        return;
    }
    p = Prim_Pixel16(spr, pixels, x, y);
    *p = (unsigned short)~*p;
    if (surface != NULL) {
        SDL_UpdateRect(surface, x, y, 1, 1);
    }
}

/*
 * 0x00485B20: g_gfxDrawLine (software): draws a line in an RGB565 colour (PalFX
 * applied). Lines on one row or column of pixels go to the horizontal /
 * vertical line code, the others are clipped and rasterized.
 */
void Gfx_SoftDrawLine(float x1, float y1, float x2, float y2, unsigned int color)
{
    Sprite *spr = Prim_Target();
    int iy1 = Ftol(y1);
    int iy2 = Ftol(y2);
    int ix1, ix2;
    unsigned int c;
    unsigned char *pixels;

    if (iy1 == iy2) {
        Gfx_SoftHLine(Ftol(x2), Ftol(x1), iy1, color);
        return;
    }
    ix1 = Ftol(x1);
    ix2 = Ftol(x2);
    if (ix1 == ix2) {
        Gfx_SoftVLine(iy2, iy1, ix1, color);
        return;
    }
    c = PalFx_Apply565(color);
    pixels = Prim_TargetPixels(spr);
    if (pixels != NULL) {
        Gfx_SoftDrawLineClipped(pixels, spr->rows, x1, y1, x2, y2, c);
    }
}

/* Step of the rasterizer's error term (the original's x87 sequences). */
static int Prim_LineStep(int err, double inc)
{
    return Ftol((double)err + inc);
}

/*
 * 0x00485C10: draws a clipped line from (x1, y1) to (x2, y2) in colour c
 * (RGB565, no PalFX) on 16-bit pixels with row offsets rows. Diagonal lines
 * use a Bresenham error term seeded with the sub-pixel position.
 */
void Gfx_SoftDrawLineRaster(const int *rows, unsigned char *pixels, float x1, float y1, float x2, float y2,
                            unsigned int color)
{
    unsigned short c = (unsigned short)color;
    float dx, dy, ady;
    double ddx, ddy, dady, a, e;
    int ix1, ix2, iy1, iy2, err, n, x, y;
    const int *row;

    if (x2 < x1) {
        float t = x1;
        x1 = x2;
        x2 = t;
        t = y1;
        y1 = y2;
        y2 = t;
    }
    dx = x2 - x1;
    dy = y2 - y1;
    ix1 = Ftol(x1);
    ix2 = Ftol(x2);
    iy1 = Ftol(y1);
    iy2 = Ftol(y2);
    if (ix1 == ix2) {
        for (y = iy1; y <= iy2; y++) {
            *(unsigned short *)(pixels + rows[y] + ix1 * 2) = c;
        }
        return;
    }
    if (iy1 == iy2) {
        for (x = ix1; x <= ix2; x++) {
            *(unsigned short *)(pixels + rows[iy1] + x * 2) = c;
        }
        return;
    }
    row = &rows[iy1];
    x = ix1 * 2;
    *(unsigned short *)(pixels + *row + x) = c;
    if (!(dx > 0.0f)) {
        return;
    }
    ddx = dx;
    ddy = dy;
    if (dy > 0.0f) {
        if (dy <= dx) {
            /* shallow, downwards */
            a = (ddy + ddy) - ddx;
            e = ((double)y1 - iy1) * ddx - ddy * ((double)x1 - ix1);
            err = Ftol(e + e + a);
            n = Ftol(ddx);
            for (; n > 0; n--) {
                x += 2;
                if (err < 0) {
                    err = Prim_LineStep(err, ddy + ddy);
                } else {
                    err = Ftol((a - ddx) + err);
                    row++;
                }
                *(unsigned short *)(pixels + *row + x) = c;
            }
            return;
        }
        if (dy > dx) {
            /* steep, downwards */
            a = (ddx + ddx) - ddy;
            e = ((double)x1 - ix1) * ddy - ddx * ((double)y1 - iy1);
            err = Ftol(e + e + a);
            n = Ftol(ddy);
            for (; n > 0; n--) {
                row++;
                if (err < 0) {
                    err = Prim_LineStep(err, ddx + ddx);
                } else {
                    err = Ftol((a - ddy) + err);
                    x += 2;
                }
                *(unsigned short *)(pixels + *row + x) = c;
            }
            return;
        }
    }
    if (!(dy < 0.0f)) {
        return;
    }
    ady = -dy;
    if (ady <= dx) {
        /* shallow, upwards */
        ady = (float)(ddy * -1.0);
        dady = ady;
        a = (dady + dady) - ddx;
        e = ((double)x1 - ix1) * -dady - ((double)y1 - iy1) * ddx;
        err = Ftol(e + e + a);
        n = Ftol(ddx);
        for (; n > 0; n--) {
            x += 2;
            if (err < 0) {
                err = Prim_LineStep(err, dady + dady);
            } else {
                err = Ftol((a - ddx) + err);
                row--;
            }
            *(unsigned short *)(pixels + *row + x) = c;
        }
        return;
    }
    if (!(ady > dx)) {
        return;
    }
    /* steep, upwards */
    ady = (float)(ddy * -1.0);
    dady = ady;
    a = (ddx + ddx) - dady;
    e = ((double)x1 - ix1) * dady + ddx * ((double)y1 - iy1);
    err = Ftol(e + e + a);
    n = Ftol(dady);
    for (; n > 0; n--) {
        row--;
        if (err < 0) {
            err = Prim_LineStep(err, ddx + ddx);
        } else {
            err = Ftol((a - dady) + err);
            x += 2;
        }
        *(unsigned short *)(pixels + *row + x) = c;
    }
}

/*
 * 0x00486000: parameter t at which the line (x, y) + t * (dx, dy) crosses the
 * clip edge through (px, py) with outward normal (nx, ny).
 */
float Gfx_LineEdgeParam(float x, float y, float dx, float dy, float nx, float ny, float px, float py)
{
    float rx = x - px;
    float ry = y - py;
    float num = rx * nx + ry * ny;
    float den = -(nx * dx + ny * dy);
    float t = num / den;

    return t;
}

/* Bit of the Cohen-Sutherland outcode of an integer point. */
static int Prim_Outcode(int x, int y)
{
    int code = 0;

    if (x < g_clipX1) {
        code = 1;
    } else if (x >= g_clipX2) {
        code = 2;
    }
    if (y < g_clipY1) {
        code |= 4;
    } else if (y >= g_clipY2) {
        code |= 8;
    }
    return code;
}

/*
 * Direction of the line against a clip edge (Cyrus-Beck): 1 when it enters
 * through the edge (v < 0), -1 otherwise, 0 when the crossing t is outside
 * [0, 1] (or NaN).
 */
static int Prim_EdgeDir(float t, double v)
{
    if (!(t >= 0.0f) || t > 1.0f) {
        return 0;
    }
    return v < 0.0 ? 1 : -1;
}

/*
 * 0x00486070: clips a line to the clip rectangle (Cohen-Sutherland trivial
 * reject, then Cyrus-Beck against the four edges) and rasterizes it.
 */
void Gfx_SoftDrawLineClipped(unsigned char *pixels, const int *rows, float x1, float y1, float x2, float y2,
                             unsigned int color)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float xmin, xmax, ymin, ymax, fx2, fy2;
    float t_left, t_right, t_top, t_bottom, t_in, t_out;
    float nx1, ny1, nx2, ny2;
    int left, right, top, bottom;
    double ddx, ddy;

    if (Prim_Outcode(Ftol(x1), Ftol(y1)) & Prim_Outcode(Ftol(x2), Ftol(y2))) {
        return;
    }
    ddx = dx;
    ddy = dy;
    ymax = (float)(g_clipY2 - 1);
    xmin = (float)g_clipX1;
    t_left = Gfx_LineEdgeParam(x1, y1, dx, dy, -1.0f, 0.0f, xmin, ymax);
    left = Prim_EdgeDir(t_left, ddy * 0.0 - ddx * 1.0);
    ymin = (float)g_clipY1;
    xmax = (float)(g_clipX2 - 1);
    t_right = Gfx_LineEdgeParam(x1, y1, dx, dy, 1.0f, 0.0f, xmax, ymin);
    right = Prim_EdgeDir(t_right, ddy * 0.0 + ddx);
    t_top = Gfx_LineEdgeParam(x1, y1, dx, dy, 0.0f, -1.0f, xmin, ymin);
    top = Prim_EdgeDir(t_top, ddx * 0.0 - ddy * 1.0);
    t_bottom = Gfx_LineEdgeParam(x1, y1, dx, dy, 0.0f, 1.0f, xmax, ymax);
    bottom = Prim_EdgeDir(t_bottom, ddx * 0.0 + ddy);

    t_out = 1.0f;
    if (left == -1 && t_left < t_out) {
        t_out = t_left;
    }
    if (right == -1 && t_out > t_right) {
        t_out = t_right;
    }
    if (top == -1 && t_top < t_out) {
        t_out = t_top;
    }
    if (bottom == -1 && t_bottom < t_out) {
        t_out = t_bottom;
    }
    t_in = 0.0f;
    if (left == 1 && t_left > t_in) {
        t_in = t_left;
    }
    if (right == 1 && t_in < t_right) {
        t_in = t_right;
    }
    if (top == 1 && t_top > t_in) {
        t_in = t_top;
    }
    if (bottom == 1 && t_in < t_bottom) {
        t_in = t_bottom;
    }
    if (t_out < t_in || t_out != t_out || t_in != t_in) {
        return;
    }
    nx1 = (float)((double)t_in * ddx + x1);
    ny1 = (float)((double)t_in * ddy + y1);
    nx2 = (float)(ddx * t_out + x1);
    ny2 = (float)(ddy * t_out + y1);
    if (xmin > nx1) {
        nx1 = xmin;
    }
    fx2 = (float)g_clipX2;
    if (nx1 >= fx2) {
        nx1 = xmax;
    }
    if (nx2 < xmin) {
        nx2 = xmin;
    }
    if (nx2 >= fx2) {
        nx2 = xmax;
    }
    if (ymin > ny1) {
        ny1 = ymin;
    }
    fy2 = (float)g_clipY2;
    if (ny1 >= fy2) {
        ny1 = ymax;
    }
    if (ny2 < ymin) {
        ny2 = ymin;
    }
    if (ny2 >= fy2) {
        ny2 = ymax;
    }
    Gfx_SoftDrawLineRaster(rows, pixels, nx1, ny1, nx2, ny2, color);
}

/*
 * Clips the span lo..hi (sorted first) on one axis and checks the other
 * coordinate; returns 0 when nothing is left. lo/hi are clamped to [min, max).
 */
static int Prim_ClipSpan(int *lo, int *hi, int min, int max)
{
    Prim_Order(lo, hi);
    if (*hi < min || *lo >= max) {
        return 0;
    }
    if (*lo < min) {
        *lo = min;
    }
    if (*hi >= max) {
        *hi = max - 1;
    }
    return 1;
}

/*
 * Clipping of the horizontal (vertical = 0) and vertical lines: the fixed
 * coordinate first, then the span; returns 0 when nothing is left.
 */
static int Prim_ClipLine(int vertical, int *lo, int *hi, int at)
{
    Prim_Order(lo, hi);
    if (vertical) {
        if (at < g_clipX1 || at >= g_clipX2) {
            return 0;
        }
        return Prim_ClipSpan(lo, hi, g_clipY1, g_clipY2);
    }
    if (at < g_clipY1 || at >= g_clipY2) {
        return 0;
    }
    return Prim_ClipSpan(lo, hi, g_clipX1, g_clipX2);
}

/* 0x00486630: horizontal line from x1 to x2 on row y, RGB565 colour (PalFX applied). */
void Gfx_SoftHLine(int x1, int x2, int y, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned short c;
    unsigned short *p;
    unsigned char *pixels;
    int n;

    if (!Prim_ClipLine(0, &x2, &x1, y)) {
        return;
    }
    c = (unsigned short)PalFx_Apply565(color);
    pixels = Prim_TargetPixels(spr);
    if (pixels == NULL) {
        return;
    }
    p = (unsigned short *)(pixels + spr->rows[y] + x2 * 2);
    for (n = x1 - x2 + 1; n != 0; n--) {
        *p++ = c;
    }
}

/* 0x004866D0: vertical line from y1 to y2 in column x, RGB565 colour (PalFX applied). */
void Gfx_SoftVLine(int y1, int y2, int x, unsigned int color)
{
    Sprite *spr = Prim_Target();
    int stride = g_targetWidth * 2;
    unsigned short c;
    unsigned char *p;
    unsigned char *pixels;
    int n;

    if (!Prim_ClipLine(1, &y2, &y1, x)) {
        return;
    }
    c = (unsigned short)PalFx_Apply565(color);
    pixels = Prim_TargetPixels(spr);
    if (pixels == NULL) {
        return;
    }
    p = pixels + spr->rows[y2] + x * 2;
    for (n = y1 - y2 + 1; n != 0; n--) {
        *(unsigned short *)p = c;
        p += stride;
    }
}

/*
 * 0x00486780: g_gfxDrawLineAdd (software): additive line (raw RGB565 colour, no
 * PalFX). Only horizontal and vertical lines are drawn (compared as floats).
 */
void Gfx_SoftDrawLineAdd(float x1, float y1, float x2, float y2, unsigned int color)
{
    if (y2 == y1) {
        Gfx_SoftHLineAdd(color, Ftol(x1), Ftol(x2), Ftol(y1));
    } else if (x2 == x1) {
        Gfx_SoftVLineAdd(color, Ftol(y1), Ftol(y2), Ftol(x1));
    }
}

/* Span operations of the additive / 50% / subtractive lines and rectangles. */
#define PRIM_OP_ADD     0       /* d + c */
#define PRIM_OP_HALF    1       /* d / 2 + c */
#define PRIM_OP_SUB     2       /* d - c */

/* Applies op with colour c to n pixels from p, step bytes apart. */
static void Prim_Span(unsigned char *p, int n, int step, int op, unsigned int c)
{
    unsigned int ar = PRIM_R(c), ag = PRIM_G(c), ab = PRIM_B(c);

    for (; n != 0; n--) {
        unsigned short *d = (unsigned short *)p;
        switch (op) {
        case PRIM_OP_ADD:
            *d = Prim_Add565(*d, ar, ag, ab);
            break;
        case PRIM_OP_HALF:
            *d = Prim_Add565(Prim_Half565(*d), ar, ag, ab);
            break;
        default:
            *d = Prim_Sub565(*d, -(int)ar, -(int)ag, -(int)ab);
            break;
        }
        p += step;
    }
}

/*
 * The additive / 50% / subtractive lines: horizontal (vertical = 0) from a to
 * b on row at, or vertical from a to b in column at. The colour is raw; quirk:
 * PalFx_Apply565 is called and its result dropped.
 */
static void Prim_LineOp(int vertical, int op, unsigned int color, int a, int b, int at)
{
    Sprite *spr = Prim_Target();
    unsigned char *pixels;
    unsigned char *p;

    if (!Prim_ClipLine(vertical, &a, &b, at)) {
        return;
    }
    PalFx_Apply565(color);
    pixels = Prim_TargetPixels(spr);
    if (pixels == NULL) {
        return;
    }
    if (vertical) {
        p = pixels + spr->rows[a] + at * 2;
        Prim_Span(p, b - a + 1, g_targetWidth * 2, op, color);
    } else {
        p = pixels + spr->rows[at] + a * 2;
        Prim_Span(p, b - a + 1, 2, op, color);
    }
}

/* 0x00486810: additive horizontal line from x1 to x2 on row y. */
void Gfx_SoftHLineAdd(unsigned int color, int x1, int x2, int y)
{
    Prim_LineOp(0, PRIM_OP_ADD, color, x1, x2, y);
}

/* 0x00486930: horizontal line from x1 to x2 on row y: half of the target plus the colour. */
void Gfx_SoftHLineHalfAdd(unsigned int color, int x1, int x2, int y)
{
    Prim_LineOp(0, PRIM_OP_HALF, color, x1, x2, y);
}

/* 0x00486A60: additive vertical line from y1 to y2 in column x. */
void Gfx_SoftVLineAdd(unsigned int color, int y1, int y2, int x)
{
    Prim_LineOp(1, PRIM_OP_ADD, color, y1, y2, x);
}

/* 0x00486B90: vertical line from y1 to y2 in column x: half of the target plus the colour. */
void Gfx_SoftVLineHalfAdd(unsigned int color, int y1, int y2, int x)
{
    Prim_LineOp(1, PRIM_OP_HALF, color, y1, y2, x);
}

/* 0x00486CD0: subtractive horizontal line from x1 to x2 on row y. */
void Gfx_SoftHLineSub(unsigned int color, int x1, int x2, int y)
{
    Prim_LineOp(0, PRIM_OP_SUB, color, x1, x2, y);
}

/* 0x00486DE0: subtractive vertical line from y1 to y2 in column x. */
void Gfx_SoftVLineSub(unsigned int color, int y1, int y2, int x)
{
    Prim_LineOp(1, PRIM_OP_SUB, color, y1, y2, x);
}

/*
 * 0x00486F00: g_gfxDrawRect (software): rectangle outline in an RGB565 colour,
 * drawn as four lines. Quirk: the PalFX is applied here and again by the
 * lines.
 */
void Gfx_SoftDrawRect(int x1, int y1, int x2, int y2, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned int c = PalFx_Apply565(color);
    float fx1, fy1, fx2, fy2;

    if (Prim_TargetPixels(spr) == NULL) {
        return;
    }
    fy2 = (float)y2;
    fx1 = (float)x1;
    fy1 = (float)y1;
    Gfx_SoftDrawLine(fx1, fy1, fx1, fy2, c);
    fx2 = (float)x2;
    Gfx_SoftDrawLine(fx1, fy2, fx2, fy2, c);
    Gfx_SoftDrawLine(fx2, fy2, fx2, fy1, c);
    Gfx_SoftDrawLine(fx2, fy1, fx1, fy1, c);
}

/* 0x00487040: g_gfxDrawRectAdd (software): additive rectangle outline (raw RGB565 colour). */
void Gfx_SoftDrawRectAdd(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_SoftHLineAdd(color, x1, x2, y1);
    Gfx_SoftHLineAdd(color, x1, x2, y2);
    Gfx_SoftVLineAdd(color, y1, y2, x1);
    Gfx_SoftVLineAdd(color, y1, y2, x2);
}

/*
 * 0x004870A0: g_gfxDrawRectBlend (software): blended rectangle outline: additive
 * when alpha_dst > 128, else half of the target plus the colour. alpha_src is
 * not used.
 */
void Gfx_SoftDrawRectBlend(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst)
{
    if (alpha_dst > 0x80) {
        Gfx_SoftDrawRectAdd(x1, y1, x2, y2, color);
        return;
    }
    Gfx_SoftHLineHalfAdd(color, x1, x2, y1);
    Gfx_SoftHLineHalfAdd(color, x1, x2, y2);
    Gfx_SoftVLineHalfAdd(color, y1, y2, x1);
    Gfx_SoftVLineHalfAdd(color, y1, y2, x2);
}

/* 0x00487120: g_gfxDrawRectSub (software): subtractive rectangle outline (raw RGB565 colour). */
void Gfx_SoftDrawRectSub(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_SoftHLineSub(color, x1, x2, y1);
    Gfx_SoftHLineSub(color, x1, x2, y2);
    Gfx_SoftVLineSub(color, y1, y2, x1);
    Gfx_SoftVLineSub(color, y1, y2, x2);
}

/* Sorts and clips a rectangle (inclusive corners); returns 0 when nothing is left. */
static int Prim_ClipRect(int *x1, int *y1, int *x2, int *y2)
{
    Prim_Order(x1, x2);
    Prim_Order(y1, y2);
    if (*x2 < g_clipX1 || *x1 >= g_clipX2 || *y2 < g_clipY1 || *y1 >= g_clipY2) {
        return 0;
    }
    if (*x1 < g_clipX1) {
        *x1 = g_clipX1;
    }
    if (*x2 >= g_clipX2) {
        *x2 = g_clipX2 - 1;
    }
    if (*y1 < g_clipY1) {
        *y1 = g_clipY1;
    }
    if (*y2 >= g_clipY2) {
        *y2 = g_clipY2 - 1;
    }
    return 1;
}

/* 0x00487180: g_gfxFillRect (software): fills a rectangle with an RGB565 colour (PalFX applied). */
void Gfx_SoftFillRect(int x1, int y1, int x2, int y2, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned short c = (unsigned short)PalFx_Apply565(color);
    unsigned char *pixels = Prim_TargetPixels(spr);
    int y;

    if (pixels == NULL || !Prim_ClipRect(&x1, &y1, &x2, &y2)) {
        return;
    }
    for (y = y1; y <= y2; y++) {
        unsigned short *p = (unsigned short *)(pixels + spr->rows[y] + x1 * 2);
        int n;
        for (n = x2 - x1 + 1; n != 0; n--) {
            *p++ = c;
        }
    }
}

/* The additive / 50% / subtractive filled rectangles (raw colour; PalFx_Apply565's result dropped). */
static void Prim_RectOp(int op, int x1, int y1, int x2, int y2, unsigned int color)
{
    Sprite *spr = Prim_Target();
    unsigned char *pixels;
    int y;

    if (!Prim_ClipRect(&x1, &y1, &x2, &y2)) {
        return;
    }
    PalFx_Apply565(color);
    pixels = Prim_TargetPixels(spr);
    if (pixels == NULL) {
        return;
    }
    for (y = y1; y <= y2; y++) {
        Prim_Span(pixels + spr->rows[y] + x1 * 2, x2 - x1 + 1, 2, op, color);
    }
}

/* 0x004872D0: g_gfxFillRectColor (software): additive filled rectangle (raw RGB565 colour). */
void Gfx_SoftFillRectAdd(int x1, int y1, int x2, int y2, unsigned int color)
{
    Prim_RectOp(PRIM_OP_ADD, x1, y1, x2, y2, color);
}

/*
 * 0x00487470: g_gfxBlendRect (software): blends a colour over a rectangle:
 * additive when alpha_dst > 128, else half of the target plus the colour.
 * alpha_src is not used.
 */
void Gfx_SoftBlendRect(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst)
{
    if (alpha_dst > 0x80) {
        Gfx_SoftFillRectAdd(x1, y1, x2, y2, color);
    } else {
        Gfx_SoftFillRectHalfAdd(color, x1, x2, y1, y2);
    }
}

/* 0x004874C0: filled rectangle: half of the target plus the colour (raw RGB565). */
void Gfx_SoftFillRectHalfAdd(unsigned int color, int x1, int x2, int y1, int y2)
{
    Prim_RectOp(PRIM_OP_HALF, x1, y1, x2, y2, color);
}

/* 0x00487660: g_gfxFillRectSub (software): subtractive filled rectangle (raw RGB565 colour). */
void Gfx_SoftFillRectSub(int x1, int y1, int x2, int y2, unsigned int color)
{
    Prim_RectOp(PRIM_OP_SUB, x1, y1, x2, y2, color);
}

/* ---- OpenGL renderer ---- */

#define PRIM_GL_CONSTANT_COLOR          0x8001
#define PRIM_GL_CONSTANT_ALPHA          0x8003
#define PRIM_GL_FUNC_ADD                0x8006
#define PRIM_GL_FUNC_REVERSE_SUBTRACT   0x800B

/* glColor3f of a 0xBBGGRR colour. */
static void Prim_GlColor(unsigned int color)
{
    float b = (float)((color >> 16 & 0xFF) / 255.0);
    float g = (float)((color >> 8 & 0xFF) / 255.0);
    float r = (float)((color & 0xFF) / 255.0);

    glColor3f(r, g, b);
}

/* Additive blending by constant factors (the colour times 1, plus the target times alpha_dst). */
static void Prim_GlBlend(float src, float dst, unsigned int equation)
{
    glEnable(GL_BLEND);
    glBlendFunc(PRIM_GL_CONSTANT_COLOR, PRIM_GL_CONSTANT_ALPHA);
    g_glewBlendColor(src, src, src, dst);
    g_glewBlendEquation(equation);
}

/* Sorts a <= b (as floats). */
static void Prim_GlOrder(float *a, float *b)
{
    if (*b < *a) {
        float t = *a;
        *a = *b;
        *b = t;
    }
}

/*
 * A rectangle's corners as floats, sorted; the far edges are moved by
 * 0.9999 to include the last pixel.
 */
static void Prim_GlRect(int x1, int y1, int x2, int y2, float *fx1, float *fy1, float *fx2, float *fy2)
{
    *fx1 = (float)x1;
    *fy1 = (float)y1;
    *fx2 = (float)x2;
    *fy2 = (float)y2;
    Prim_GlOrder(fx1, fx2);
    Prim_GlOrder(fy1, fy2);
    *fx2 = (float)((double)*fx2 + 0.9999);
    *fy2 = (float)((double)*fy2 + 0.9999);
}

/*
 * 0x004877E0: g_gfxClear of the OpenGL renderer: clears the target to a
 * 0xBBGGRR colour (through the global PalFX), ignoring the clip rectangle.
 */
void Gfx_GlClear(unsigned int color)
{
    unsigned char src[4], dst[4];
    unsigned int mapped;

    glDisable(GL_SCISSOR_TEST);
    src[0] = (unsigned char)color;
    src[1] = (unsigned char)(color >> 8);
    src[2] = (unsigned char)(color >> 16);
    src[3] = 0xFF;
    PalFx_ApplyColor(src, dst);
    mapped = Video_MapRGB(dst[0], dst[1], dst[2]);
    {
        float b = (float)((mapped >> 16 & 0xFF) / 255.0);
        float g = (float)((mapped >> 8 & 0xFF) / 255.0);
        float r = (float)((mapped & 0xFF) / 255.0);
        glClearColor(r, g, b, 0.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    Gl_ApplyClipRect();
}

/* 0x004878C0: g_gfxPutPixel (OpenGL): sets pixel (x, y) to a 0xBBGGRR colour. */
void Gfx_GlPutPixel(int x, int y, unsigned int color)
{
    Gl_ResetModelview();
    Prim_GlColor(color);
    glBegin(GL_POINTS);
    glVertex2f((float)x, (float)y);
    glEnd();
}

/* 0x004879B0: g_gfxAddPixel (OpenGL): adds a 0xBBGGRR colour to pixel (x, y). */
void Gfx_GlAddPixel(int x, int y, unsigned int color)
{
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(1.0f, 1.0f, PRIM_GL_FUNC_ADD);
    glBegin(GL_POINTS);
    glVertex2f((float)x, (float)y);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x00487AE0: g_gfxInvertPixel (OpenGL): inverts pixel (x, y) (XOR logic op; nothing without EXT_blend_logic_op). */
void Gfx_GlInvertPixel(int x, int y)
{
    if (!g_glewExtBlendLogicOp) {
        return;
    }
    Gl_ResetModelview();
    glColor3f(1.0f, 1.0f, 1.0f);
    Prim_GlBlend(1.0f, 1.0f, GL_LOGIC_OP);
    glLogicOp(GL_XOR);
    glBegin(GL_POINTS);
    glVertex2f((float)x, (float)y);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x00487B90: g_gfxDrawLine (OpenGL): line in a 0xBBGGRR colour. */
void Gfx_GlDrawLine(float x1, float y1, float x2, float y2, unsigned int color)
{
    Gl_ResetModelview();
    Prim_GlColor(color);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

/* 0x00487C90: g_gfxDrawLineAdd (OpenGL): additive line. */
void Gfx_GlDrawLineAdd(float x1, float y1, float x2, float y2, unsigned int color)
{
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(1.0f, 1.0f, PRIM_GL_FUNC_ADD);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x00487DE0: g_gfxDrawRect (OpenGL): rectangle outline. */
void Gfx_GlDrawRect(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_GlDrawRectBlend(x1, y1, x2, y2, color, 0x100, 0);
}

/* 0x00487E10: g_gfxDrawRectAdd (OpenGL): additive rectangle outline. */
void Gfx_GlDrawRectAdd(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_GlDrawRectBlend(x1, y1, x2, y2, color, 0x100, 0x100);
}

/*
 * 0x00487E40: g_gfxDrawRectBlend (OpenGL): rectangle outline blended over the
 * target with alphas out of 256.
 */
void Gfx_GlDrawRectBlend(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst)
{
    float src = (float)(alpha_src * (1.0 / 256));
    float dst = (float)((1.0 / 256) * alpha_dst);
    float fx1, fy1, fx2, fy2;

    Prim_GlRect(x1, y1, x2, y2, &fx1, &fy1, &fx2, &fy2);
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(src, dst, PRIM_GL_FUNC_ADD);
    glBegin(GL_LINE_LOOP);
    glVertex2f(fx1, fy1);
    glVertex2f(fx2, fy1);
    glVertex2f(fx2, fy2);
    glVertex2f(fx1, fy2);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x00488060: g_gfxDrawRectSub (OpenGL): subtractive rectangle outline. */
void Gfx_GlDrawRectSub(int x1, int y1, int x2, int y2, unsigned int color)
{
    float fx1, fy1, fx2, fy2;

    Prim_GlRect(x1, y1, x2, y2, &fx1, &fy1, &fx2, &fy2);
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(1.0f, 1.0f, PRIM_GL_FUNC_REVERSE_SUBTRACT);
    glBegin(GL_LINE_LOOP);
    glVertex2f(fx1, fy1);
    glVertex2f(fx2, fy1);
    glVertex2f(fx2, fy2);
    glVertex2f(fx1, fy2);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x00488260: g_gfxFillRect (OpenGL): fills a rectangle. */
void Gfx_GlFillRect(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_GlBlendRect(x1, y1, x2, y2, color, 0x100, 0);
}

/* 0x00488290: g_gfxFillRectColor (OpenGL): additive filled rectangle. */
void Gfx_GlFillRectAdd(int x1, int y1, int x2, int y2, unsigned int color)
{
    Gfx_GlBlendRect(x1, y1, x2, y2, color, 0x100, 0x100);
}

/* 0x004882C0: g_gfxBlendRect (OpenGL): blends a colour over a rectangle with alphas out of 256. */
void Gfx_GlBlendRect(int x1, int y1, int x2, int y2, unsigned int color, int alpha_src, int alpha_dst)
{
    float src = (float)(alpha_src * (1.0 / 256));
    float dst = (float)((1.0 / 256) * alpha_dst);
    float fx1, fy1, fx2, fy2;

    Prim_GlRect(x1, y1, x2, y2, &fx1, &fy1, &fx2, &fy2);
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(src, dst, PRIM_GL_FUNC_ADD);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(fx1, fy1);
    glVertex2f(fx2, fy1);
    glVertex2f(fx1, fy2);
    glVertex2f(fx2, fy2);
    glEnd();
    glDisable(GL_BLEND);
}

/* 0x004884E0: g_gfxFillRectSub (OpenGL): subtractive filled rectangle. */
void Gfx_GlFillRectSub(int x1, int y1, int x2, int y2, unsigned int color)
{
    float fx1, fy1, fx2, fy2;

    Prim_GlRect(x1, y1, x2, y2, &fx1, &fy1, &fx2, &fy2);
    Gl_ResetModelview();
    Prim_GlColor(color);
    Prim_GlBlend(1.0f, 1.0f, PRIM_GL_FUNC_REVERSE_SUBTRACT);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(fx1, fy1);
    glVertex2f(fx2, fy1);
    glVertex2f(fx1, fy2);
    glVertex2f(fx2, fy2);
    glEnd();
    glDisable(GL_BLEND);
}
