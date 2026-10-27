/*
 * Software sprite rasterizer (original module 0x47CC10-0x484BB0): draws
 * paletted (8-bit) sprites onto 16-bit RGB565 render targets for
 * Sprite_DrawSoft. Sprite_DrawSoft8 picks one of the drawers by blending and
 * rotation:
 * - no blending: opaque or colour-0-transparent (DrawParams.masked);
 * - subtractive blending (BLEND_SUB): the palette colour is subtracted from
 *   the target per component;
 * - additive blending: drawers outside this module (0x493170, 0x497C40,
 *   0x499350);
 * and, without rotation, by the signs of the scales (horizontal / vertical
 * flips). Every combination is a separate function in the original (the code
 * was generated from one template); here they share one implementation per
 * kind, with each original entry point kept as a thin wrapper.
 *
 * Scaled drawing steps through the source in 16.16 fixed point; sprites drawn
 * at their size (within 0.001) go to the blitters instead.
 *
 * Floating point follows the x87 code of the original: values the original
 * keeps on the FPU stack are doubles here (the FPU runs with 53-bit
 * precision), values it stores are floats. Conversions to int are those of
 * the original's __ftol2_sse (Ftol), except where the original inlines a
 * 64-bit fistp (Trunc64).
 *
 * Quirks kept from the original:
 * - a NaN width or height passes the "nothing to draw" checks;
 * - only the plain drawers clamp the source row to the sprite (the
 *   subtractive ones can read one row past the end when stepping rounds up);
 * - flipped and rotated sprites are always drawn masked, whatever
 *   DrawParams.masked says;
 * - SpriteSet_GetPaletteConverted's result is used without a NULL check;
 * - a NaN y (scaled drawing), or a NaN / infinite position (rotated drawing)
 *   overflows the pixel counts and writes outside the target;
 * - the rotated drawers test one sample past the right end of each row when
 *   trimming it to the sprite.
 */
#include "mugen.h"

/* ---- helpers ---- */

static Sprite *Soft_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *a = set->sprites;

    return (Sprite *)(a->data + a->elem_size * index);
}

/* 32-bit wrapping int arithmetic of the original (imul / add). */
static int Soft_MulAdd(int a, int b, int c)
{
    return (int)((unsigned int)a * (unsigned int)b + (unsigned int)c);
}

/* Pixel operations. */
#define SOFT_OP_OPAQUE  0       /* every pixel */
#define SOFT_OP_MASKED  1       /* colour 0 is transparent */
#define SOFT_OP_SUB     2       /* masked; the colour is subtracted from the target */

/* Flips (bits of the drawer variants). */
#define SOFT_FLIP_H     1
#define SOFT_FLIP_V     2

/* Writes palette entry index (already known nonzero for the masked ops) to d. */
static void Soft_Put(unsigned short *d, unsigned int index, const unsigned short *pal, int op)
{
    if (op == SOFT_OP_SUB) {
        int c = (short)pal[index];
        unsigned int dv = *d;
        int r = (int)(dv & 0xF800) - (c & 0xF800);
        int g = (int)(dv & 0x07E0) - (c & 0x07E0);
        int b = (int)(dv & 0x001F) - (c & 0x001F);
        if (r < 0) {
            r = 0;
        }
        if (g < 0) {
            g = 0;
        }
        if (b < 0) {
            b = 0;
        }
        *d = (unsigned short)(r | g | b);
    } else {
        *d = pal[index];
    }
}

/* ---- unscaled drawing ---- */

/*
 * Draws sprite `sprite` of set with its axis at (x, y), unscaled, flipped by
 * flip (SOFT_FLIP_*), with pixel operation op.
 */
static void Soft_Blit(SpriteSet *set, int sprite, int x, int y, int flip, int op)
{
    const Sprite *spr = Soft_SpriteAt(set, sprite);
    const Sprite *tspr = Soft_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int w = spr->w, h = spr->h;
    int sx = 0, sy = 0;
    int x0, y0, x1, y1, xend, yend, ncols, nrows, r;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;

    if (flip & SOFT_FLIP_H) {
        x = x + spr->x - w + 1;
    } else {
        x -= spr->x;
    }
    if (flip & SOFT_FLIP_V) {
        y = y + spr->y - h + 1;
    } else {
        y -= spr->y;
    }
    if (y >= g_clipY2) {
        return;
    }
    yend = h + y;
    if (yend <= g_clipY1 || x >= g_clipX2) {
        return;
    }
    xend = w + x;
    if (xend <= g_clipX1) {
        return;
    }
    y0 = y;
    if (y < g_clipY1) {
        sy = g_clipY1 - y;
        y0 = g_clipY1;
    }
    y1 = yend - 1;
    if (y1 >= g_clipY2) {
        y1 = g_clipY2 - 1;
    }
    x0 = x;
    if (x < g_clipX1) {
        sx = g_clipX1 - x;
        x0 = g_clipX1;
    }
    x1 = xend - 1;
    if (x1 >= g_clipX2) {
        x1 = g_clipX2 - 1;
    }
    ncols = x1 - x0 + 1;
    nrows = y1 - y0 + 1;
    if (ncols <= 0 || nrows <= 0) {
        return;
    }
    src = Sprite_GetPixels(set, sprite);
    if (src == NULL) {
        return;
    }
    dst = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = SpriteSet_GetPaletteConverted(set, spr->pal_index)->rgb565;
    for (r = 0; r < nrows; r++) {
        int row = (flip & SOFT_FLIP_V) ? h - sy - 1 - r : sy + r;
        const unsigned char *s = src + spr->rows[row];
        unsigned short *d = (unsigned short *)(dst + tspr->rows[y0 + r] + x0 * 2);
        int step = (flip & SOFT_FLIP_H) ? -1 : 1;
        int n;

        s += (flip & SOFT_FLIP_H) ? w - 1 - sx : sx;
        for (n = ncols; n != 0; n--) {
            unsigned int c = *s;
            if (c != 0 || op == SOFT_OP_OPAQUE) {
                Soft_Put(d, c, pal, op);
            }
            d++;
            s += step;
        }
    }
}

/* 0x0047CC10: subtractive blit with the axis at (x, y). */
void SoftSprite_BlitSub(int y, int x, SpriteSet *set, int sprite)
{
    Soft_Blit(set, sprite, x, y, 0, SOFT_OP_SUB);
}

/* 0x0047CE50: SoftSprite_BlitSub, flipped horizontally. */
void SoftSprite_BlitSubH(int y, SpriteSet *set, int sprite, int x)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_H, SOFT_OP_SUB);
}

/* 0x0047D090: SoftSprite_BlitSub, flipped vertically. */
void SoftSprite_BlitSubV(int x, SpriteSet *set, int sprite, int y)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_V, SOFT_OP_SUB);
}

/* 0x0047D2D0: SoftSprite_BlitSub, flipped both ways. */
void SoftSprite_BlitSubHV(SpriteSet *set, int sprite, int x, int y)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_H | SOFT_FLIP_V, SOFT_OP_SUB);
}

/* 0x00480A00: masked blit (colour 0 transparent) with the axis at (x, y). */
void SoftSprite_Blit(int y, int x, SpriteSet *set, int sprite)
{
    Soft_Blit(set, sprite, x, y, 0, SOFT_OP_MASKED);
}

/* 0x00480BD0: opaque blit with the axis at (x, y). */
void SoftSprite_BlitOpaque(int y, int x, SpriteSet *set, int sprite)
{
    Soft_Blit(set, sprite, x, y, 0, SOFT_OP_OPAQUE);
}

/* 0x00480DA0: masked blit, flipped horizontally. */
void SoftSprite_BlitH(int y, SpriteSet *set, int sprite, int x)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_H, SOFT_OP_MASKED);
}

/* 0x00480F80: masked blit, flipped vertically. */
void SoftSprite_BlitV(int x, SpriteSet *set, int sprite, int y)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_V, SOFT_OP_MASKED);
}

/* 0x00481160: masked blit, flipped both ways. */
void SoftSprite_BlitHV(SpriteSet *set, int sprite, int x, int y)
{
    Soft_Blit(set, sprite, x, y, SOFT_FLIP_H | SOFT_FLIP_V, SOFT_OP_MASKED);
}

/* ---- scaled drawing ---- */

/* The unscaled drawer of a scaled variant. */
static void Soft_BlitVariant(SpriteSet *set, int sprite, int x, int y, int flip, int op)
{
    switch (flip | op << 2) {
    case SOFT_OP_OPAQUE << 2:
        SoftSprite_BlitOpaque(y, x, set, sprite);
        break;
    case SOFT_OP_MASKED << 2:
        SoftSprite_Blit(y, x, set, sprite);
        break;
    case SOFT_FLIP_H | SOFT_OP_MASKED << 2:
        SoftSprite_BlitH(y, set, sprite, x);
        break;
    case SOFT_FLIP_V | SOFT_OP_MASKED << 2:
        SoftSprite_BlitV(x, set, sprite, y);
        break;
    case SOFT_FLIP_H | SOFT_FLIP_V | SOFT_OP_MASKED << 2:
        SoftSprite_BlitHV(set, sprite, x, y);
        break;
    case SOFT_OP_SUB << 2:
        SoftSprite_BlitSub(y, x, set, sprite);
        break;
    case SOFT_FLIP_H | SOFT_OP_SUB << 2:
        SoftSprite_BlitSubH(y, set, sprite, x);
        break;
    case SOFT_FLIP_V | SOFT_OP_SUB << 2:
        SoftSprite_BlitSubV(x, set, sprite, y);
        break;
    default:
        SoftSprite_BlitSubHV(set, sprite, x, y);
        break;
    }
}

/*
 * First destination pixel and source offset (16.16) of one axis of a scaled
 * draw: pos is rounded to the nearest pixel (halves down), the source starts
 * size / scaled pixels per destination pixel before or after.
 */
static int Soft_StartPixel(float pos, float size, float scaled, int *frac)
{
    int p = Ftol(floor(pos));

    *frac = 0;
    if (pos > (double)p) {
        float t;
        if ((double)pos - 0.5 > (double)p) {
            p++;
        }
        t = (float)(((double)p - pos) * size / scaled * 65536.0);
        *frac = Ftol(floor(t));
    }
    return p;
}

/* Source offset (16.16) of n destination pixels: n * size / scaled. */
static int Soft_SkipFrac(int n, int size, float scaled)
{
    float t = (float)((double)Soft_MulAdd(n, size, 0) / scaled * 65536.0);

    return Trunc64(floor(t));
}

/*
 * Draws sprite `sprite` of set scaled to sw x sh pixels, with its axis
 * (scaled) at (x, y), flipped by flip, with pixel operation op. row_clamp:
 * the source row is kept inside the sprite (the plain drawers only).
 */
static void Soft_Scale(SpriteSet *set, int sprite, float x, float y, float sw, float sh, int flip, int op,
                       int row_clamp)
{
    const Sprite *spr = Soft_SpriteAt(set, sprite);
    const Sprite *tspr = Soft_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int w, h, ix0, iy0, ix1, iy1, u0, v0, du, dv, sx_int = 0, sy_int = 0, ncols, nrows, iy;
    float fw, fh, d1, d2, x0, y0;
    double xend, yend;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;

    if (sw <= 0.0f || sh <= 0.0f) {
        return;
    }
    w = spr->w;
    h = spr->h;
    fw = (float)w;
    d1 = (float)fabs((double)fw - sw);
    if (0.001f > d1) {
        d2 = (float)fabs((double)h - sh);
        if (0.001f > d2) {
            Soft_BlitVariant(set, sprite, Ftol(x), Ftol(y), flip, op);
            return;
        }
    }
    if (flip & SOFT_FLIP_H) {
        x0 = (float)(x - ((double)(w - spr->x) * sw / fw - 1.0));
    } else {
        x0 = (float)(x - (double)spr->x * sw / fw);
    }
    fh = (float)h;
    if (flip & SOFT_FLIP_V) {
        y0 = (float)(y - ((double)(h - spr->y) * sh / fh - 1.0));
    } else {
        y0 = (float)(y - (double)spr->y * sh / fh);
    }
    if ((double)g_clipY2 <= y0) {
        return;
    }
    yend = (double)y0 + sh;
    if ((double)g_clipY1 >= yend - 0.5) {
        return;
    }
    if ((double)g_clipX2 <= x0) {
        return;
    }
    xend = (double)x0 + sw;
    if ((double)g_clipX1 >= xend - 0.5) {
        return;
    }

    iy0 = Soft_StartPixel(y0, fh, sh, &v0);
    if (iy0 < g_clipY1) {
        v0 += Soft_SkipFrac(g_clipY1 - iy0, h, sh);
        iy0 = g_clipY1;
    }
    iy1 = Gfx_SoftCeil((float)(yend - 1.0));
    if (iy1 >= g_clipY2) {
        iy1 = g_clipY2 - 1;
    }
    if (iy1 - iy0 + 1 <= 0) {
        return;
    }
    ix0 = Soft_StartPixel(x0, fw, sw, &u0);
    if ((double)g_clipX1 > x0) {
        u0 += Soft_SkipFrac(g_clipX1 - ix0, w, sw);
        ix0 = g_clipX1;
    }
    ix1 = Gfx_SoftCeil((float)(xend - 1.0));
    if (ix1 >= g_clipX2) {
        ix1 = g_clipX2 - 1;
    }
    du = Ftol((double)(w << 16) / sw);
    dv = Ftol((double)(h << 16) / sh);
    while (u0 < 0 && ix0 <= ix1) {
        u0 += du;
        ix0++;
    }
    sx_int = u0 >> 16;
    u0 &= 0xFFFF;
    while (v0 < 0 && iy0 <= iy1) {
        iy0++;
        v0 += dv;
    }
    sy_int = v0 >> 16;
    v0 &= 0xFFFF;
    ncols = ix1 - ix0 + 1;
    nrows = iy1 - iy0 + 1;
    if (ncols <= 0 || nrows <= 0) {
        return;
    }
    if ((Soft_MulAdd(ncols - 1, du, u0) >> 16) + sx_int >= w) {
        du--;
    }
    if ((Soft_MulAdd(nrows - 1, dv, v0) >> 16) + sy_int >= h) {
        dv--;
    }
    src = Sprite_GetPixels(set, sprite);
    if (src == NULL) {
        return;
    }
    dst = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = SpriteSet_GetPaletteConverted(set, spr->pal_index)->rgb565;
    for (iy = iy0; nrows > 0; nrows--, iy++) {
        const unsigned char *s;
        unsigned short *d = (unsigned short *)(dst + tspr->rows[iy] + ix0 * 2);
        int u = u0;
        int row, n;

        if (row_clamp && (v0 >> 16) + sy_int >= h) {
            v0 -= 0x10000;
        }
        row = (v0 >> 16) + sy_int;
        if (flip & SOFT_FLIP_V) {
            row = h - row - 1;
        }
        s = src + spr->rows[row];
        if (flip & SOFT_FLIP_H) {
            s += w - sx_int - 1;
        } else {
            s += sx_int;
        }
        for (n = ncols; n != 0; n--) {
            unsigned int c = *s;
            if (c != 0 || op == SOFT_OP_OPAQUE) {
                Soft_Put(d, c, pal, op);
            }
            d++;
            u += du;
            if (flip & SOFT_FLIP_H) {
                s -= u >> 16;
            } else {
                s += u >> 16;
            }
            u &= 0xFFFF;
        }
        v0 += dv;
    }
}

/* 0x0047D690: subtractive drawing scaled to w x h pixels, axis at (x, y). */
void SoftSprite_ScaleSub(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, 0, SOFT_OP_SUB, 0);
}

/* 0x0047DCE0: SoftSprite_ScaleSub, flipped horizontally. */
void SoftSprite_ScaleSubH(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_H, SOFT_OP_SUB, 0);
}

/* 0x0047E330: SoftSprite_ScaleSub, flipped vertically. */
void SoftSprite_ScaleSubV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_V, SOFT_OP_SUB, 0);
}

/* 0x0047E990: SoftSprite_ScaleSub, flipped both ways. */
void SoftSprite_ScaleSubHV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_H | SOFT_FLIP_V, SOFT_OP_SUB, 0);
}

/* 0x00481620: masked drawing scaled to w x h pixels, axis at (x, y). */
void SoftSprite_Scale(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, 0, SOFT_OP_MASKED, 1);
}

/* 0x00481C10: opaque drawing scaled to w x h pixels, axis at (x, y). */
void SoftSprite_ScaleOpaque(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, 0, SOFT_OP_OPAQUE, 1);
}

/* 0x004821E0: masked scaled drawing, flipped horizontally. */
void SoftSprite_ScaleH(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_H, SOFT_OP_MASKED, 1);
}

/* 0x004827E0: masked scaled drawing, flipped vertically. */
void SoftSprite_ScaleV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_V, SOFT_OP_MASKED, 1);
}

/* 0x00482DD0: masked scaled drawing, flipped both ways. */
void SoftSprite_ScaleHV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    Soft_Scale(set, sprite, x, y, w, h, SOFT_FLIP_H | SOFT_FLIP_V, SOFT_OP_MASKED, 1);
}

/*
 * The scale dispatchers: sx, sy are the signed scales; negative ones flip.
 * The drawers get the absolute scaled size.
 */
typedef void (*SoftScaleFn)(SpriteSet *set, int sprite, float x, float y, float w, float h);

static void Soft_DispatchScale(SpriteSet *set, int sprite, float x, float y, float sx, float sy, SoftScaleFn normal,
                               SoftScaleFn flip_h, SoftScaleFn flip_v, SoftScaleFn flip_hv)
{
    const Sprite *spr = Soft_SpriteAt(set, sprite);
    float sw = (float)((double)spr->w * sx);
    float sh = (float)((double)spr->h * sy);

    if (0.0f < sy) {
        if (0.0f < sx) {
            normal(set, sprite, x, y, sw, sh);
        }
        if (0.0f > sx) {
            flip_h(set, sprite, x, y, -sw, sh);
        }
    }
    if (0.0f > sy) {
        if (0.0f < sx) {
            flip_v(set, sprite, x, y, sw, -sh);
        }
        if (0.0f > sx) {
            flip_hv(set, sprite, x, y, -sw, -sh);
        }
    }
}

/* 0x0047D530: subtractive drawing at (x, y) scaled by (sx, sy) (negative scales flip). */
void SoftSprite_DrawScaledSub(SpriteSet *set, int sprite, float x, float y, float sx, float sy)
{
    Soft_DispatchScale(set, sprite, x, y, sx, sy, SoftSprite_ScaleSub, SoftSprite_ScaleSubH, SoftSprite_ScaleSubV,
                       SoftSprite_ScaleSubHV);
}

/* 0x00481360: masked drawing at (x, y) scaled by (sx, sy) (negative scales flip). */
void SoftSprite_DrawScaled(SpriteSet *set, int sprite, float x, float y, float sx, float sy)
{
    Soft_DispatchScale(set, sprite, x, y, sx, sy, SoftSprite_Scale, SoftSprite_ScaleH, SoftSprite_ScaleV,
                       SoftSprite_ScaleHV);
}

/* 0x004814C0: opaque drawing at (x, y) scaled by (sx, sy); flipped sprites are drawn masked. */
void SoftSprite_DrawScaledOpaque(SpriteSet *set, int sprite, float x, float y, float sx, float sy)
{
    Soft_DispatchScale(set, sprite, x, y, sx, sy, SoftSprite_ScaleOpaque, SoftSprite_ScaleH, SoftSprite_ScaleV,
                       SoftSprite_ScaleHV);
}

/* ---- rotated drawing ---- */

/* Corner order of a rotated sprite by quadrant: top vertex first (P1), then clockwise. */
static const unsigned char soft_corners[4][4] = {
    /* corner kinds: 0 (-ax, -ay), 1 (w-ax-1, -ay), 2 (w-ax-1, h-ay-1), 3 (-ax, h-ay-1) */
    { 0, 1, 2, 3 }, { 1, 2, 3, 0 }, { 2, 3, 0, 1 }, { 3, 0, 1, 2 },
};

/* Sample of the rotated sprite inside the source? */
static int Soft_InSprite(int u, int v, int w, int h)
{
    return (v >> 16) < h && (u >> 16) < w && (v >> 16) >= 0 && (u >> 16) >= 0;
}

/*
 * Draws sprite `sprite` of set rotated by angle degrees (counter-clockwise,
 * 1/8 degree steps through g_sinTable) around its axis at (x, y), scaled by
 * scale_x * scale2_x, scale_y * scale2_y (negative scale_x / scale_y turn by
 * 180 degrees), always masked, with pixel operation op. mirror: the result is
 * mirrored horizontally around x. Multiples of 180 degrees go to
 * Sprite_DrawAt / Sprite_DrawSub (op SOFT_OP_SUB) instead.
 *
 * The rotated rectangle is scanned row by row between two edges stepped in
 * 16.16; each row's source start is computed in floating point, then its
 * ends are trimmed to the samples inside the sprite. Quirk: the right end is
 * tested one sample past the last pixel.
 */
static void Soft_Rotate(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                        float scale2_x, float scale2_y, int mirror, int op)
{
    const Sprite *spr = Soft_SpriteAt(set, sprite);
    const Sprite *tspr = Soft_SpriteAt(g_renderTarget, g_renderTargetSprite);
    const int *srows = spr->rows;
    float t, fcos, fsin, c, s, fw_scale, fh_scale;
    float px[4], py[4];
    int ix, iy, sw, sh, idx, cosv, sinv, dux, dvx, ax, ay, w, h, k, quad;
    int y0r, y1r, y2r, y3r, dy01, dy21, e01, e21, xl, xr, row, lim, ybot;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;

    t = (float)floor((double)x + 0.5);
    ix = Ftol(t);
    t = (float)floor((double)y + 0.5);
    iy = Ftol(t);
    sw = Ftol((double)scale_x * 1024.0);
    if (sw < 0) {
        sw = -sw;
        angle = (float)((double)angle + 180.0);
    }
    sh = Ftol((double)scale_y * 1024.0);
    if (sh < 0) {
        sh = -sh;
        angle = (float)(180.0 + angle);
    }
    if (sw == 0 || sh == 0) {
        return;
    }
    if (angle < 0.0f) {
        int turns = Ftol(angle / -360.0);
        angle = (float)((double)angle + (double)(360 - turns * -360));
    }
    idx = Ftol((double)angle * 8.0) % 2880;
    if (idx == 0 || idx == 1440) {
        float sy2 = idx == 0 ? (float)((double)scale_y * scale2_y) : (float)(-(double)scale_y * scale2_y);
        float sx2 = (idx == 0) != (mirror != 0) ? (float)((double)scale_x * scale2_x)
                                                : (float)(-(double)scale_x * scale2_x);
        if (op == SOFT_OP_SUB) {
            Sprite_DrawSub(set, sprite, x, y, sx2, sy2);
        } else {
            Sprite_DrawAt(set, sprite, x, y, sx2, sy2);
        }
        return;
    }
    ax = spr->x;
    h = spr->h;
    ay = spr->y;
    w = spr->w;
    cosv = g_sinTable[(idx + 720) % 2880];
    fcos = (float)cosv;
    c = (float)(fcos * (1.0 / 1024));
    sinv = g_sinTable[idx % 2880];
    fsin = (float)sinv;
    s = (float)((1.0 / 1024) * fsin);
    dux = Ftol((double)((cosv << 16) / sw) / scale2_x);
    dvx = Ftol((double)((sinv << 16) / sh) / scale2_x);
    if (idx < 720) {
        quad = 0;
    } else if (idx < 1440) {
        quad = 1;
    } else if (idx < 2160) {
        quad = 2;
    } else if (idx < 2880) {
        quad = 3;
    } else {
        return;
    }
    for (k = 0; k < 4; k++) {
        int kind = soft_corners[quad][k];
        float cx = (float)(kind == 1 || kind == 2 ? w - ax - 1 : -ax);
        float cy = (float)(kind >= 2 ? h - ay - 1 : -ay);
        px[k] = (float)((double)cx * scale_x);
        py[k] = (float)((double)cy * scale_y);
    }
    y0r = Math_Round((float)(((double)py[0] * c - (double)px[0] * s) * scale2_y));
    y2r = Math_Round((float)(((double)py[2] * c - (double)px[2] * s) * scale2_y));
    y1r = Math_Round((float)(((double)py[1] * c - (double)px[1] * s) * scale2_y));
    y3r = Math_Round((float)(((double)py[3] * c - (double)px[3] * s) * scale2_y));
    dy01 = y1r - y0r;
    dy21 = y1r - y2r;
    if (dy01 != 0 && dy21 != 0) {
        double x1v = (double)px[1] * c + (double)py[1] * s;
        double x0v = (double)px[0] * c + (double)py[0] * s;
        double x2v = (double)px[2] * c + (double)py[2] * s;
        e01 = Ftol((x1v - x0v) * 65536.0 * scale2_x / dy01);
        if (e01 < 0) {
            e01 = -e01;
        }
        e21 = Ftol((x2v - x1v) * 65536.0 * scale2_x / dy21);
        if (e21 < 0) {
            e21 = -e21;
        }
        xl = xr = Ftol(x1v * 65536.0 * scale2_x);
    } else {
        double x0v = (double)px[0] * c + (double)py[0] * s;
        double x2v = (double)px[2] * c + (double)py[2] * s;
        e01 = 0;
        e21 = 0;
        xl = Ftol(x0v * 65536.0 * scale2_x);
        xr = Ftol(x2v * 65536.0 * scale2_x);
    }
    row = y1r;
    lim = g_clipY1 - iy;
    for (; row < lim; row++) {
        if (row < y0r) {
            xl -= e01;
        } else {
            xl += e21;
        }
        if (row < y2r) {
            xr += e21;
        } else {
            xr -= e01;
        }
    }
    ybot = y3r;
    if (ybot >= g_clipY2 - iy) {
        ybot = g_clipY2 - iy - 1;
    }
    src = Sprite_GetPixels(set, sprite);
    if (src == NULL) {
        return;
    }
    dst = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = SpriteSet_GetPaletteConverted(set, spr->pal_index)->rgb565;
    fw_scale = (float)sw;
    fh_scale = (float)sh;
    for (; row <= ybot; row++) {
        int xa = xl >> 16, xb = xr >> 16;
        int u, v, u_end, v_end, n, step = mirror ? -1 : 1;
        double xs, ys;
        unsigned short *d;

        if (mirror) {
            if (ix - xa >= g_clipX2) {
                xa = ix - g_clipX2 + 1;
            }
            if (ix - xb < g_clipX1) {
                xb = ix - g_clipX1;
            }
        } else {
            if (xa < g_clipX1 - ix) {
                xa = g_clipX1 - ix;
            }
            if (xb >= g_clipX2 - ix) {
                xb = g_clipX2 - ix - 1;
            }
        }
        xs = (double)xa / scale2_x;
        ys = (double)row / scale2_y;
        u = (ax << 16) - Ftol(((double)fcos * xs - (double)fsin * ys) * -65536.0 / fw_scale);
        v = (ay << 16) - Ftol((xs * fsin + ys * fcos) * -65536.0 / fh_scale);
        d = (unsigned short *)(dst + tspr->rows[row + iy] + (mirror ? ix - xa : xa + ix) * 2);
        n = xb - xa + 1;
        v_end = Soft_MulAdd(n, dvx, v);
        u_end = Soft_MulAdd(n, dux, u);
        if (xb >= xa) {
            while (!Soft_InSprite(u_end, v_end, w, h)) {
                v_end -= dvx;
                u_end -= dux;
                xb--;
                if (xb < xa) {
                    break;
                }
            }
            if (xb >= xa) {
                while (xa <= xb && !Soft_InSprite(u, v, w, h)) {
                    u += dux;
                    d += step;
                    xa++;
                    v += dvx;
                }
            }
        }
        for (n = xb - xa + 1; n > 0; n--) {
            unsigned int col = src[srows[v >> 16] + (u >> 16)];
            if (col != 0) {
                Soft_Put(d, col, pal, op);
            }
            u += dux;
            d += step;
            v += dvx;
        }
        if (row < y0r) {
            xl -= e01;
        } else {
            xl += e21;
        }
        if (row < y2r) {
            xr += e21;
        } else {
            xr -= e01;
        }
    }
}

/* 0x0047F020: subtractive rotated drawing (see Soft_Rotate). */
void SoftSprite_RotateSub(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                          float scale2_x, float scale2_y)
{
    Soft_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 0, SOFT_OP_SUB);
}

/* 0x0047FB50: subtractive rotated drawing, mirrored horizontally. */
void SoftSprite_RotateSubFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x,
                              float scale_y, float scale2_x, float scale2_y)
{
    Soft_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 1, SOFT_OP_SUB);
}

/* 0x004833E0: masked rotated drawing (see Soft_Rotate). */
void SoftSprite_Rotate(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                       float scale2_x, float scale2_y)
{
    Soft_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 0, SOFT_OP_MASKED);
}

/* 0x00483ED0: masked rotated drawing, mirrored horizontally. */
void SoftSprite_RotateFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                           float scale2_x, float scale2_y)
{
    Soft_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 1, SOFT_OP_MASKED);
}

/* 0x00480680: ceil(x) as an int. */
int Gfx_SoftCeil(float x)
{
    return Ftol(ceil(x));
}

/*
 * 0x004806A0: Sprite_DrawSoft's drawer for paletted sprites. Rotated sprites
 * (flips and negative scales folded into the angle) go to the rotating
 * drawers, the others to the scaled ones with the combined signed scales.
 */
void Sprite_DrawSoft8(const DrawParams *dp, SpriteSet *set, int sprite)
{
    int alpha_src = Ftol(dp->alpha_src * 256.0);
    int alpha_dst = Ftol(256.0 * dp->alpha_dst);
    float sx, sy;

    if (!(0.0f == dp->angle)) {
        DrawParams r = *dp;
        DrawParams_FlipToAngle(&r);
        switch (r.blend) {
        case BLEND_NONE:
            if (r.flip2 & DRAW_FLIP_H) {
                SoftSprite_RotateFlip(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x, r.scale2_y);
            } else {
                SoftSprite_Rotate(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x, r.scale2_y);
            }
            break;
        case BLEND_ADD:
            if (r.flip2 & DRAW_FLIP_H) {
                SoftSprite_RotateAddFlip(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x,
                                         r.scale2_y, alpha_src, alpha_dst);
            } else {
                SoftSprite_RotateAdd(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x, r.scale2_y,
                                     alpha_src, alpha_dst);
            }
            break;
        case BLEND_SUB:
            if (r.flip2 & DRAW_FLIP_H) {
                SoftSprite_RotateSubFlip(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x,
                                         r.scale2_y);
            } else {
                SoftSprite_RotateSub(set, sprite, r.x, r.y, r.angle, r.scale_x, r.scale_y, r.scale2_x, r.scale2_y);
            }
            break;
        }
        return;
    }
    sx = (float)((double)dp->scale2_x * dp->scale_x * ((dp->flip & DRAW_FLIP_H) ? -1 : 1) *
                 ((dp->flip2 & DRAW_FLIP_H) ? -1 : 1));
    sy = (float)((double)dp->scale2_y * dp->scale_y * ((dp->flip & DRAW_FLIP_V) ? -1 : 1) *
                 ((dp->flip2 & DRAW_FLIP_V) ? -1 : 1));
    switch (dp->blend) {
    case BLEND_NONE:
        if (dp->masked != 0) {
            SoftSprite_DrawScaled(set, sprite, dp->x, dp->y, sx, sy);
        } else {
            SoftSprite_DrawScaledOpaque(set, sprite, dp->x, dp->y, sx, sy);
        }
        break;
    case BLEND_ADD:
        SoftSprite_DrawScaledAdd(set, sprite, alpha_dst, dp->x, dp->y, sx, sy, alpha_src);
        break;
    case BLEND_SUB:
        SoftSprite_DrawScaledSub(set, sprite, dp->x, dp->y, sx, sy);
        break;
    }
}
