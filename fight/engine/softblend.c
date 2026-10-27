/*
 * Software sprite drawing with additive blending (original module
 * 0x48EE70-0x49AA4F): the drawers Sprite_DrawSoft8 uses for BLEND_ADD on
 * 16-bit RGB565 targets. They come from the same template as the drawers of
 * softrender.c (unscaled, scaled and rotated, every flip a separate
 * function), with two pixel operations:
 * - full additive blending (the "Add" drawers): the palette colour is added
 *   to the target per component, saturating;
 * - blending by alpha_src / alpha_dst (256 = 1.0, the "Alpha" drawers and
 *   SoftSprite_RotateAdd): the palette is first scaled by alpha_src into a
 *   copy on the stack (halved for 128, zeroed for 0), then per pixel the
 *   target is replaced (alpha_dst 0), or scaled by alpha_dst (halved for
 *   128) and added, saturating only when alpha_src + alpha_dst > 256 (the
 *   components can carry into each other otherwise).
 * The alpha drawers hand 256/0 to the masked drawers of softrender.c and
 * 256/256 to the full additive ones, and draw nothing for 0/256.
 *
 * As in softrender.c, values the original keeps on the x87 stack are
 * doubles here, the ones it stores are floats, and each original entry
 * point is a thin wrapper around one implementation per kind.
 *
 * Quirks kept (see softrender.c for the template's): colour 0 is always
 * transparent; a NaN width or height passes the "nothing to draw" checks;
 * the scaled drawers do not clamp the source row to the sprite; the
 * palette results are used without a NULL check.
 */
#include "mugen.h"

/* ---- helpers (as softrender.c) ---- */

static Sprite *Blend_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *a = set->sprites;

    return (Sprite *)(a->data + a->elem_size * index);
}

/* 32-bit wrapping int arithmetic of the original (imul / add). */
static int Blend_MulAdd(int a, int b, int c)
{
    return (int)((unsigned int)a * (unsigned int)b + (unsigned int)c);
}

/* Flips (bits of the drawer variants). */
#define BLEND_FLIP_H    1
#define BLEND_FLIP_V    2

/* Pixel operations: full addition, or blending by alphas (Blend_Mode picks one of the others). */
enum {
    BLEND_OP_ADD = 0,           /* add, saturating */
    BLEND_OP_ALPHA = 1,         /* placeholder for the drawers: one of the following */
    BLEND_OP_COPY = 2,          /* alpha_dst 0: the (scaled) colour replaces the target */
    BLEND_OP_HALF = 3,          /* alpha_dst 128, alpha_src + alpha_dst <= 256: half the target plus the colour */
    BLEND_OP_SCALE = 4,         /* alpha_src + alpha_dst <= 256: target * alpha_dst plus the colour */
    BLEND_OP_HALF_SAT = 5,      /* as BLEND_OP_HALF, saturating */
    BLEND_OP_SCALE_SAT = 6      /* as BLEND_OP_SCALE, saturating */
};

static int Blend_Mode(int alpha_src, int alpha_dst)
{
    if (alpha_dst == 0) {
        return BLEND_OP_COPY;
    }
    if (alpha_src + alpha_dst <= 0x100) {
        return alpha_dst == 0x80 ? BLEND_OP_HALF : BLEND_OP_SCALE;
    }
    return alpha_dst == 0x80 ? BLEND_OP_HALF_SAT : BLEND_OP_SCALE_SAT;
}

/* RGB565 component c * alpha / 256, kept in the component's bits (unsigned 32-bit product). */
static unsigned int Blend_Scale565(unsigned int c, unsigned int mask, int alpha)
{
    return (c & mask) * (unsigned int)alpha >> 8 & mask;
}

/*
 * The palette scaled by alpha_src: pal itself for 256, else buf (entries 1
 * to ncolors - 1; all zeroed for 0, halved for 128).
 */
static const unsigned short *Blend_Palette(const unsigned short *pal, int ncolors, int alpha_src,
                                           unsigned short *buf)
{
    int i;

    if (alpha_src == 0x100) {
        return pal;
    }
    if (alpha_src == 0) {
        memset(buf, 0, ncolors * 2);
    } else if (alpha_src == 0x80) {
        for (i = 1; i < ncolors; i++) {
            buf[i] = (unsigned short)((pal[i] >> 1) & 0x7BEF);
        }
    } else {
        for (i = 1; i < ncolors; i++) {
            unsigned int c = pal[i];
            buf[i] = (unsigned short)(Blend_Scale565(c, 0x7E0, alpha_src) | Blend_Scale565(c, 0x1F, alpha_src) |
                                      Blend_Scale565(c, 0xF800, alpha_src));
        }
    }
    return buf;
}

static unsigned int Blend_Sat(unsigned int r, unsigned int g, unsigned int b)
{
    if (r > 0xF800) {
        r = 0xF800;
    }
    if (g > 0x7E0) {
        g = 0x7E0;
    }
    if (b > 0x1F) {
        b = 0x1F;
    }
    return r | g | b;
}

/* Blends colour c (palette entry, nonzero index) into *d. */
static void Blend_Put(unsigned short *d, unsigned int c, int op, int alpha_dst)
{
    unsigned int dv = *d;
    unsigned int r;

    switch (op) {
    case BLEND_OP_ADD:
        r = Blend_Sat((dv & 0xF800) + (c & 0xF800), (dv & 0x7E0) + (c & 0x7E0), (dv & 0x1F) + (c & 0x1F));
        break;
    case BLEND_OP_COPY:
        r = c;
        break;
    case BLEND_OP_HALF:
        dv = dv >> 1 & 0x7BEF;
        r = ((dv & 0x7E0) + (c & 0x7E0)) | ((dv & 0x1F) + (c & 0x1F)) | ((dv & 0xF800) + (c & 0xF800));
        break;
    case BLEND_OP_SCALE:
        r = (Blend_Scale565(dv, 0x7E0, alpha_dst) + (c & 0x7E0)) | (Blend_Scale565(dv, 0x1F, alpha_dst) + (c & 0x1F)) |
            (Blend_Scale565(dv, 0xF800, alpha_dst) + (c & 0xF800));
        break;
    case BLEND_OP_HALF_SAT:
        dv = dv >> 1 & 0x7BEF;
        r = Blend_Sat((dv & 0xF800) + (c & 0xF800), (dv & 0x7E0) + (c & 0x7E0), (dv & 0x1F) + (c & 0x1F));
        break;
    default:
        r = Blend_Sat(Blend_Scale565(dv, 0xF800, alpha_dst) + (c & 0xF800),
                      Blend_Scale565(dv, 0x7E0, alpha_dst) + (c & 0x7E0),
                      Blend_Scale565(dv, 0x1F, alpha_dst) + (c & 0x1F));
        break;
    }
    *d = (unsigned short)r;
}

/*
 * The colours a drawer uses and the pixel operation: the converted palette
 * of the sprite, scaled into buf for the alpha operation.
 */
static const unsigned short *Blend_Colors(SpriteSet *set, const Sprite *spr, int *op, int alpha_src, int alpha_dst,
                                          unsigned short *buf)
{
    const unsigned short *pal = SpriteSet_GetPaletteConverted(set, spr->pal_index)->rgb565;

    if (*op == BLEND_OP_ADD) {
        return pal;
    }
    *op = Blend_Mode(alpha_src, alpha_dst);
    return Blend_Palette(pal, spr->ncolors, alpha_src, buf);
}

/* ---- unscaled drawing ---- */

/*
 * Draws sprite `sprite` of set with its axis at (x, y), unscaled, flipped by
 * flip (BLEND_FLIP_*), with pixel operation op (BLEND_OP_ADD or _ALPHA).
 */
static void Blend_Blit(SpriteSet *set, int sprite, int x, int y, int flip, int op, int alpha_src, int alpha_dst)
{
    const Sprite *spr = Blend_SpriteAt(set, sprite);
    const Sprite *tspr = Blend_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int w = spr->w, h = spr->h;
    int sx = 0, sy = 0;
    int x0, y0, x1, y1, xend, yend, ncols, nrows, r;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;
    unsigned short buf[256];

    if (flip & BLEND_FLIP_H) {
        x = x + spr->x - w + 1;
    } else {
        x -= spr->x;
    }
    if (flip & BLEND_FLIP_V) {
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
    pal = Blend_Colors(set, spr, &op, alpha_src, alpha_dst, buf);
    for (r = 0; r < nrows; r++) {
        int row = (flip & BLEND_FLIP_V) ? h - sy - 1 - r : sy + r;
        const unsigned char *s = src + spr->rows[row];
        unsigned short *d = (unsigned short *)(dst + tspr->rows[y0 + r] + x0 * 2);
        int step = (flip & BLEND_FLIP_H) ? -1 : 1;
        int n;

        s += (flip & BLEND_FLIP_H) ? w - 1 - sx : sx;
        for (n = ncols; n != 0; n--) {
            unsigned int c = *s;
            if (c != 0) {
                Blend_Put(d, pal[c], op, alpha_dst);
            }
            d++;
            s += step;
        }
    }
}

/* 0x0048EE70: additive blit with the axis at (x, y). */
void SoftSprite_BlitAdd(int y, int x, SpriteSet *set, int sprite)
{
    Blend_Blit(set, sprite, x, y, 0, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x0048F0B0: SoftSprite_BlitAdd, flipped horizontally. */
void SoftSprite_BlitAddH(int y, SpriteSet *set, int sprite, int x)
{
    Blend_Blit(set, sprite, x, y, BLEND_FLIP_H, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x0048F310: SoftSprite_BlitAdd, flipped vertically. */
void SoftSprite_BlitAddV(int x, SpriteSet *set, int sprite, int y)
{
    Blend_Blit(set, sprite, x, y, BLEND_FLIP_V, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x0048F560: SoftSprite_BlitAdd, flipped both ways. */
void SoftSprite_BlitAddHV(SpriteSet *set, int sprite, int x, int y)
{
    Blend_Blit(set, sprite, x, y, BLEND_FLIP_H | BLEND_FLIP_V, BLEND_OP_ADD, 0x100, 0x100);
}

/*
 * The alpha blits: 256/0 is a masked blit, 256/256 an additive one, 0/256
 * draws nothing.
 */
static void Blend_BlitAlpha(SpriteSet *set, int sprite, int x, int y, int flip, int alpha_src, int alpha_dst)
{
    if (alpha_src == 0x100) {
        if (alpha_dst == 0) {
            switch (flip) {
            case 0: SoftSprite_Blit(y, x, set, sprite); break;
            case BLEND_FLIP_H: SoftSprite_BlitH(y, set, sprite, x); break;
            case BLEND_FLIP_V: SoftSprite_BlitV(x, set, sprite, y); break;
            default: SoftSprite_BlitHV(set, sprite, x, y); break;
            }
            return;
        }
        if (alpha_dst == 0x100) {
            switch (flip) {
            case 0: SoftSprite_BlitAdd(y, x, set, sprite); break;
            case BLEND_FLIP_H: SoftSprite_BlitAddH(y, set, sprite, x); break;
            case BLEND_FLIP_V: SoftSprite_BlitAddV(x, set, sprite, y); break;
            default: SoftSprite_BlitAddHV(set, sprite, x, y); break;
            }
            return;
        }
    } else if (alpha_src == 0 && alpha_dst == 0x100) {
        return;
    }
    Blend_Blit(set, sprite, x, y, flip, BLEND_OP_ALPHA, alpha_src, alpha_dst);
}

/* 0x0048F7D0: blit with the axis at (x, y), blended by alpha_src / alpha_dst. */
void SoftSprite_BlitAlpha(SpriteSet *set, int x, int sprite, int y, int alpha_src, int alpha_dst)
{
    Blend_BlitAlpha(set, sprite, x, y, 0, alpha_src, alpha_dst);
}

/* 0x0048FF90: SoftSprite_BlitAlpha, flipped horizontally. */
void SoftSprite_BlitAlphaH(int alpha_src, int y, SpriteSet *set, int sprite, int x, int alpha_dst)
{
    Blend_BlitAlpha(set, sprite, x, y, BLEND_FLIP_H, alpha_src, alpha_dst);
}

/* 0x00490790: SoftSprite_BlitAlpha, flipped vertically. */
void SoftSprite_BlitAlphaV(int x, SpriteSet *set, int sprite, int y, int alpha_src, int alpha_dst)
{
    Blend_BlitAlpha(set, sprite, x, y, BLEND_FLIP_V, alpha_src, alpha_dst);
}

/* 0x00490F60: SoftSprite_BlitAlpha, flipped both ways. */
void SoftSprite_BlitAlphaHV(SpriteSet *set, int sprite, int x, int y, int alpha_src, int alpha_dst)
{
    Blend_BlitAlpha(set, sprite, x, y, BLEND_FLIP_H | BLEND_FLIP_V, alpha_src, alpha_dst);
}

/* ---- scaled drawing ---- */

/* The unscaled drawer of a scaled variant. */
static void Blend_BlitVariant(SpriteSet *set, int sprite, int x, int y, int flip, int op, int alpha_src,
                              int alpha_dst)
{
    if (op == BLEND_OP_ADD) {
        switch (flip) {
        case 0: SoftSprite_BlitAdd(y, x, set, sprite); break;
        case BLEND_FLIP_H: SoftSprite_BlitAddH(y, set, sprite, x); break;
        case BLEND_FLIP_V: SoftSprite_BlitAddV(x, set, sprite, y); break;
        default: SoftSprite_BlitAddHV(set, sprite, x, y); break;
        }
    } else {
        switch (flip) {
        case 0: SoftSprite_BlitAlpha(set, x, sprite, y, alpha_src, alpha_dst); break;
        case BLEND_FLIP_H: SoftSprite_BlitAlphaH(alpha_src, y, set, sprite, x, alpha_dst); break;
        case BLEND_FLIP_V: SoftSprite_BlitAlphaV(x, set, sprite, y, alpha_src, alpha_dst); break;
        default: SoftSprite_BlitAlphaHV(set, sprite, x, y, alpha_src, alpha_dst); break;
        }
    }
}

/*
 * First destination pixel and source offset (16.16) of one axis of a scaled
 * draw: pos is rounded to the nearest pixel (halves down), the source starts
 * size / scaled pixels per destination pixel before or after.
 */
static int Blend_StartPixel(float pos, float size, float scaled, int *frac)
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
static int Blend_SkipFrac(int n, int size, float scaled)
{
    float t = (float)((double)Blend_MulAdd(n, size, 0) / scaled * 65536.0);

    return Trunc64(floor(t));
}

/*
 * Draws sprite `sprite` of set scaled to sw x sh pixels, with its axis
 * (scaled) at (x, y), flipped by flip, with pixel operation op.
 */
static void Blend_Scale(SpriteSet *set, int sprite, float x, float y, float sw, float sh, int flip, int op,
                        int alpha_src, int alpha_dst)
{
    const Sprite *spr = Blend_SpriteAt(set, sprite);
    const Sprite *tspr = Blend_SpriteAt(g_renderTarget, g_renderTargetSprite);
    int w, h, ix0, iy0, ix1, iy1, u0, v0, du, dv, sx_int = 0, sy_int = 0, ncols, nrows, iy;
    float fw, fh, d1, d2, x0, y0;
    double xend, yend;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;
    unsigned short buf[256];

    w = spr->w;
    h = spr->h;
    fw = (float)w;
    d1 = (float)fabs((double)fw - sw);
    if (0.001f > d1) {
        d2 = (float)fabs((double)h - sh);
        if (0.001f > d2) {
            Blend_BlitVariant(set, sprite, Ftol(x), Ftol(y), flip, op, alpha_src, alpha_dst);
            return;
        }
    }
    if (flip & BLEND_FLIP_H) {
        x0 = (float)(x - ((double)(w - spr->x) * sw / fw - 1.0));
    } else {
        x0 = (float)(x - (double)spr->x * sw / fw);
    }
    fh = (float)h;
    if (flip & BLEND_FLIP_V) {
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

    iy0 = Blend_StartPixel(y0, fh, sh, &v0);
    if (iy0 < g_clipY1) {
        v0 += Blend_SkipFrac(g_clipY1 - iy0, h, sh);
        iy0 = g_clipY1;
    }
    iy1 = Gfx_SoftCeil((float)(yend - 1.0));
    if (iy1 >= g_clipY2) {
        iy1 = g_clipY2 - 1;
    }
    if (iy1 - iy0 + 1 <= 0) {
        return;
    }
    ix0 = Blend_StartPixel(x0, fw, sw, &u0);
    if ((double)g_clipX1 > x0) {
        u0 += Blend_SkipFrac(g_clipX1 - ix0, w, sw);
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
    if ((Blend_MulAdd(ncols - 1, du, u0) >> 16) + sx_int >= w) {
        du--;
    }
    if ((Blend_MulAdd(nrows - 1, dv, v0) >> 16) + sy_int >= h) {
        dv--;
    }
    src = Sprite_GetPixels(set, sprite);
    if (src == NULL) {
        return;
    }
    dst = Sprite_GetPixels(g_renderTarget, g_renderTargetSprite);
    pal = Blend_Colors(set, spr, &op, alpha_src, alpha_dst, buf);
    for (iy = iy0; nrows > 0; nrows--, iy++) {
        const unsigned char *s;
        unsigned short *d = (unsigned short *)(dst + tspr->rows[iy] + ix0 * 2);
        int u = u0;
        int row, n;

        row = (v0 >> 16) + sy_int;
        if (flip & BLEND_FLIP_V) {
            row = h - row - 1;
        }
        s = src + spr->rows[row];
        if (flip & BLEND_FLIP_H) {
            s += w - sx_int - 1;
        } else {
            s += sx_int;
        }
        for (n = ncols; n != 0; n--) {
            unsigned int c = *s;
            if (c != 0) {
                Blend_Put(d, pal[c], op, alpha_dst);
            }
            d++;
            u += du;
            if (flip & BLEND_FLIP_H) {
                s -= u >> 16;
            } else {
                s += u >> 16;
            }
            u &= 0xFFFF;
        }
        v0 += dv;
    }
}

/* 0x00491770: additive drawing scaled to w x h pixels, axis at (x, y). */
void SoftSprite_ScaleAdd(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    Blend_Scale(set, sprite, x, y, w, h, 0, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x00491DE0: SoftSprite_ScaleAdd, flipped horizontally. */
void SoftSprite_ScaleAddH(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    Blend_Scale(set, sprite, x, y, w, h, BLEND_FLIP_H, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x00492450: SoftSprite_ScaleAdd, flipped vertically. */
void SoftSprite_ScaleAddV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    Blend_Scale(set, sprite, x, y, w, h, BLEND_FLIP_V, BLEND_OP_ADD, 0x100, 0x100);
}

/* 0x00492AD0: SoftSprite_ScaleAdd, flipped both ways. */
void SoftSprite_ScaleAddHV(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    Blend_Scale(set, sprite, x, y, w, h, BLEND_FLIP_H | BLEND_FLIP_V, BLEND_OP_ADD, 0x100, 0x100);
}

typedef void (*BlendScaleFn)(SpriteSet *set, int sprite, float x, float y, float w, float h);

/*
 * The alpha scales: 256/0 is the masked scale of softrender.c, 256/256 the
 * additive one, 0/256 draws nothing.
 */
static void Blend_ScaleAlpha(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src,
                             int alpha_dst, int flip, BlendScaleFn masked, BlendScaleFn add)
{
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    if (alpha_src == 0x100) {
        if (alpha_dst == 0) {
            masked(set, sprite, x, y, w, h);
            return;
        }
        if (alpha_dst == 0x100) {
            add(set, sprite, x, y, w, h);
            return;
        }
    } else if (alpha_src == 0 && alpha_dst == 0x100) {
        return;
    }
    Blend_Scale(set, sprite, x, y, w, h, flip, BLEND_OP_ALPHA, alpha_src, alpha_dst);
}

/* 0x004932E0: drawing scaled to w x h pixels, axis at (x, y), blended by alpha_src / alpha_dst. */
void SoftSprite_ScaleAlpha(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src,
                           int alpha_dst)
{
    Blend_ScaleAlpha(set, sprite, x, y, w, h, alpha_src, alpha_dst, 0, SoftSprite_Scale, SoftSprite_ScaleAdd);
}

/* 0x00493F60: SoftSprite_ScaleAlpha, flipped horizontally. */
void SoftSprite_ScaleAlphaH(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src,
                            int alpha_dst)
{
    Blend_ScaleAlpha(set, sprite, x, y, w, h, alpha_src, alpha_dst, BLEND_FLIP_H, SoftSprite_ScaleH,
                     SoftSprite_ScaleAddH);
}

/* 0x00494C20: SoftSprite_ScaleAlpha, flipped vertically. */
void SoftSprite_ScaleAlphaV(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src,
                            int alpha_dst)
{
    Blend_ScaleAlpha(set, sprite, x, y, w, h, alpha_src, alpha_dst, BLEND_FLIP_V, SoftSprite_ScaleV,
                     SoftSprite_ScaleAddV);
}

/* 0x004958E0: SoftSprite_ScaleAlpha, flipped both ways. */
void SoftSprite_ScaleAlphaHV(SpriteSet *set, int sprite, float x, float y, float w, float h, int alpha_src,
                             int alpha_dst)
{
    Blend_ScaleAlpha(set, sprite, x, y, w, h, alpha_src, alpha_dst, BLEND_FLIP_H | BLEND_FLIP_V, SoftSprite_ScaleHV,
                     SoftSprite_ScaleAddHV);
}

/*
 * 0x00493170: drawing at (x, y) scaled by (sx, sy) (negative scales flip),
 * blended by alpha_src / alpha_dst. The drawers get the absolute scaled
 * size.
 */
void SoftSprite_DrawScaledAdd(SpriteSet *set, int sprite, int alpha_dst, float x, float y, float sx, float sy,
                              int alpha_src)
{
    const Sprite *spr = Blend_SpriteAt(set, sprite);
    float sw = (float)((double)spr->w * sx);
    float sh = (float)((double)spr->h * sy);

    if (0.0f < sy) {
        if (0.0f < sx) {
            SoftSprite_ScaleAlpha(set, sprite, x, y, sw, sh, alpha_src, alpha_dst);
        }
        if (0.0f > sx) {
            SoftSprite_ScaleAlphaH(set, sprite, x, y, -sw, sh, alpha_src, alpha_dst);
        }
    }
    if (0.0f > sy) {
        if (0.0f < sx) {
            SoftSprite_ScaleAlphaV(set, sprite, x, y, sw, -sh, alpha_src, alpha_dst);
        }
        if (0.0f > sx) {
            SoftSprite_ScaleAlphaHV(set, sprite, x, y, -sw, -sh, alpha_src, alpha_dst);
        }
    }
}

/* ---- rotated drawing ---- */

/* Corner order of a rotated sprite by quadrant: top vertex first (P1), then clockwise. */
static const unsigned char blend_corners[4][4] = {
    /* corner kinds: 0 (-ax, -ay), 1 (w-ax-1, -ay), 2 (w-ax-1, h-ay-1), 3 (-ax, h-ay-1) */
    { 0, 1, 2, 3 }, { 1, 2, 3, 0 }, { 2, 3, 0, 1 }, { 3, 0, 1, 2 },
};

/* Sample of the rotated sprite inside the source? */
static int Blend_InSprite(int u, int v, int w, int h)
{
    return (v >> 16) < h && (u >> 16) < w && (v >> 16) >= 0 && (u >> 16) >= 0;
}

/*
 * Draws sprite `sprite` of set rotated by angle degrees around its axis at
 * (x, y), scaled by scale_x * scale2_x, scale_y * scale2_y, with pixel
 * operation op; mirror: mirrored horizontally around x (see softrender.c's
 * Soft_Rotate, the same template). Multiples of 180 degrees go to
 * Sprite_DrawAdd / Sprite_DrawAlpha instead.
 */
static void Blend_Rotate(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                         float scale2_x, float scale2_y, int mirror, int op, int alpha_src, int alpha_dst, int ix,
                         int iy)
{
    const Sprite *spr = Blend_SpriteAt(set, sprite);
    const Sprite *tspr = Blend_SpriteAt(g_renderTarget, g_renderTargetSprite);
    const int *srows = spr->rows;
    float fcos, fsin, c, s, fw_scale, fh_scale;
    float px[4], py[4];
    int sw, sh, idx, cosv, sinv, dux, dvx, ax, ay, w, h, k, quad;
    int y0r, y1r, y2r, y3r, dy01, dy21, e01, e21, xl, xr, row, lim, ybot;
    const unsigned char *src;
    unsigned char *dst;
    const unsigned short *pal;
    unsigned short buf[256];

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
        if (op == BLEND_OP_ADD) {
            Sprite_DrawAdd(set, sprite, x, y, sx2, sy2);
        } else {
            Sprite_DrawAlpha(set, sprite, x, y, sx2, sy2, alpha_src, alpha_dst);
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
        int kind = blend_corners[quad][k];
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
    pal = Blend_Colors(set, spr, &op, alpha_src, alpha_dst, buf);
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
        v_end = Blend_MulAdd(n, dvx, v);
        u_end = Blend_MulAdd(n, dux, u);
        if (xb >= xa) {
            while (!Blend_InSprite(u_end, v_end, w, h)) {
                v_end -= dvx;
                u_end -= dux;
                xb--;
                if (xb < xa) {
                    break;
                }
            }
            if (xb >= xa) {
                while (xa <= xb && !Blend_InSprite(u, v, w, h)) {
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
                Blend_Put(d, pal[col], op, alpha_dst);
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

/* The rotated drawers' position: (x, y) rounded to the nearest pixel. */
static int Blend_RoundPos(float v)
{
    float t = (float)floor((double)v + 0.5);

    return Ftol(t);
}

/* 0x004965B0: additive rotated drawing (see Blend_Rotate). */
void SoftSprite_RotateAddFull(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x,
                              float scale_y, float scale2_x, float scale2_y)
{
    int ix = Blend_RoundPos(x);
    int iy = Blend_RoundPos(y);

    Blend_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 0, BLEND_OP_ADD, 0x100, 0x100, ix,
                 iy);
}

/* 0x00497100: additive rotated drawing, mirrored horizontally. */
void SoftSprite_RotateAddFullFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x,
                                  float scale_y, float scale2_x, float scale2_y)
{
    int ix = Blend_RoundPos(x);
    int iy = Blend_RoundPos(y);

    Blend_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 1, BLEND_OP_ADD, 0x100, 0x100, ix,
                 iy);
}

/*
 * 0x00497C40: rotated drawing blended by alpha_src / alpha_dst: 256/0 is
 * the masked rotation of softrender.c, 256/256 the additive one, 0/256
 * draws nothing.
 */
void SoftSprite_RotateAdd(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x, float scale_y,
                          float scale2_x, float scale2_y, int alpha_src, int alpha_dst)
{
    int ix = Blend_RoundPos(x);
    int iy = Blend_RoundPos(y);

    if (alpha_src == 0x100) {
        if (alpha_dst == 0) {
            SoftSprite_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y);
            return;
        }
        if (alpha_dst == 0x100) {
            SoftSprite_RotateAddFull(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y);
            return;
        }
    } else if (alpha_src == 0 && alpha_dst == 0x100) {
        return;
    }
    Blend_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 0, BLEND_OP_ALPHA, alpha_src,
                 alpha_dst, ix, iy);
}

/* 0x00499350: SoftSprite_RotateAdd, mirrored horizontally. */
void SoftSprite_RotateAddFlip(SpriteSet *set, int sprite, float x, float y, float angle, float scale_x,
                              float scale_y, float scale2_x, float scale2_y, int alpha_src, int alpha_dst)
{
    int ix = Blend_RoundPos(x);
    int iy = Blend_RoundPos(y);

    if (alpha_src == 0x100) {
        if (alpha_dst == 0) {
            SoftSprite_RotateFlip(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y);
            return;
        }
        if (alpha_dst == 0x100) {
            SoftSprite_RotateAddFullFlip(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y);
            return;
        }
    } else if (alpha_src == 0 && alpha_dst == 0x100) {
        return;
    }
    Blend_Rotate(set, sprite, x, y, angle, scale_x, scale_y, scale2_x, scale2_y, 1, BLEND_OP_ALPHA, alpha_src,
                 alpha_dst, ix, iy);
}
