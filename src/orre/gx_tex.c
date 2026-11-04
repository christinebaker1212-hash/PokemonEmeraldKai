// Moemon Kai: GameCube GX textures (I4, I8, IA4, IA8, RGB565, RGB5A3, RGBA8, C4, C8, C14X2, CMPR) for ORRE's maps,
// which keep the disc's own encoding in ORRE's pack (tools/orre/build_pack.py; the same decoding as tools/orre/gxtex.py,
// checked against it by tools/orre/test_gxtex.sh). GX stores a texture in tiles (blocks) of 8x8, 8x4 or 4x4 pixels.
#include <string.h>
#include "gx_tex.h"

enum { GX_I4 = 0, GX_I8, GX_IA4, GX_IA8, GX_RGB565, GX_RGB5A3, GX_RGBA8, GX_C4 = 8, GX_C8, GX_C14X2, GX_CMPR = 14 };

static int Block(uint8_t f, int *bpp, int *bw, int *bh)
{
    switch (f)
    {
    case GX_I4: case GX_C4: case GX_CMPR: *bpp = 4, *bw = 8, *bh = 8; return 1;
    case GX_I8: case GX_IA4: case GX_C8: *bpp = 8, *bw = 8, *bh = 4; return 1;
    case GX_IA8: case GX_RGB565: case GX_RGB5A3: case GX_C14X2: *bpp = 16, *bw = 4, *bh = 4; return 1;
    case GX_RGBA8: *bpp = 32, *bw = 4, *bh = 4; return 1;
    }
    return 0;
}

uint32_t GxTex_Size(uint8_t format, uint16_t w, uint16_t h)
{
    int bpp, bw, bh;
    if (!Block(format, &bpp, &bw, &bh))
        return 0;
    return (uint32_t)((w + bw - 1) / bw) * ((h + bh - 1) / bh) * bw * bh * bpp / 8;
}

static uint32_t Rgba(uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    return r | g << 8 | b << 16 | a << 24;
}

static uint32_t C565(uint16_t v)
{
    return Rgba(((v >> 11) & 31) * 255 / 31, ((v >> 5) & 63) * 255 / 63, (v & 31) * 255 / 31, 255);
}

static uint32_t C5A3(uint16_t v)
{
    if (v & 0x8000)
        return Rgba(((v >> 10) & 31) * 255 / 31, ((v >> 5) & 31) * 255 / 31, (v & 31) * 255 / 31, 255);
    return Rgba(((v >> 8) & 15) * 17, ((v >> 4) & 15) * 17, (v & 15) * 17, ((v >> 12) & 7) * 255 / 7);
}

static uint32_t Pal(uint8_t tf, const uint8_t *tlut, uint16_t n, uint32_t i)
{
    uint16_t v;
    if (tlut == NULL || n == 0)
        return Rgba(i & 255, i & 255, i & 255, 255);
    if (i >= n)
        i = n - 1;
    v = (uint16_t)(tlut[2 * i] << 8 | tlut[2 * i + 1]);
    if (tf == 0)
        return Rgba(v & 255, v & 255, v & 255, v >> 8);
    return tf == 1 ? C565(v) : C5A3(v);
}

int GxTex_Decode(uint8_t format, uint16_t w, uint16_t h, const uint8_t *d, uint32_t size, uint8_t tf,
                 const uint8_t *tlut, uint16_t entries, uint32_t *out)
{
    int bpp, bw, bh, bx, by, nbx = 0, nby = 0, x, y;
    uint32_t o = 0;
    if (!Block(format, &bpp, &bw, &bh) || size < GxTex_Size(format, w, h))
        return 0;
    nbx = (w + bw - 1) / bw;
    nby = (h + bh - 1) / bh;
    for (by = 0; by < nby; by++)
        for (bx = 0; bx < nbx; bx++)
        {
            if (format == GX_CMPR)
            {
                // four 4x4 S3TC blocks (DXT1, big endian) in a 2x2 square
                int sub;
                for (sub = 0; sub < 4; sub++, o += 8)
                {
                    uint16_t c0 = (uint16_t)(d[o] << 8 | d[o + 1]), c1 = (uint16_t)(d[o + 2] << 8 | d[o + 3]);
                    uint32_t p[4], a = C565(c0), b = C565(c1);
                    int ch;
                    p[0] = a, p[1] = b, p[2] = 0, p[3] = 0;
                    for (ch = 0; ch < 3; ch++)
                    {
                        uint32_t ca = (a >> (8 * ch)) & 255, cb = (b >> (8 * ch)) & 255;
                        if (c0 > c1)
                        {
                            p[2] |= ((2 * ca + cb) / 3) << (8 * ch);
                            p[3] |= ((ca + 2 * cb) / 3) << (8 * ch);
                        }
                        else
                            p[2] |= ((ca + cb) / 2) << (8 * ch);
                    }
                    p[2] |= 255u << 24;
                    if (c0 > c1)
                        p[3] |= 255u << 24;
                    for (y = 0; y < 4; y++)
                        for (x = 0; x < 4; x++)
                        {
                            int px = bx * 8 + (sub & 1) * 4 + x, py = by * 8 + (sub >> 1) * 4 + y;
                            if (px < w && py < h)
                                out[py * w + px] = p[(d[o + 4 + y] >> (6 - 2 * x)) & 3];
                        }
                }
                continue;
            }
            if (format == GX_RGBA8)
            {
                // a 4x4 block: 16 AR pairs, then 16 GB pairs
                for (y = 0; y < 4; y++)
                    for (x = 0; x < 4; x++)
                    {
                        int k = y * 4 + x, px = bx * 4 + x, py = by * 4 + y;
                        if (px < w && py < h)
                            out[py * w + px] = Rgba(d[o + 2 * k + 1], d[o + 32 + 2 * k], d[o + 32 + 2 * k + 1], d[o + 2 * k]);
                    }
                o += 64;
                continue;
            }
            for (y = 0; y < bh; y++)
                for (x = 0; x < bw; x++)
                {
                    int px = bx * bw + x, py = by * bh + y;
                    uint32_t c = 0, v;
                    switch (format)
                    {
                    case GX_I4: case GX_C4:
                        v = (d[o >> 1] >> ((o & 1) ? 0 : 4)) & 15;
                        o++;
                        c = format == GX_I4 ? Rgba(v * 17, v * 17, v * 17, v * 17) : Pal(tf, tlut, entries, v);   // (intensity is alpha too)
                        break;
                    case GX_I8: case GX_C8:
                        v = d[o++];
                        c = format == GX_I8 ? Rgba(v, v, v, v) : Pal(tf, tlut, entries, v);
                        break;
                    case GX_IA4:
                        v = d[o++];
                        c = Rgba((v & 15) * 17, (v & 15) * 17, (v & 15) * 17, (v >> 4) * 17);
                        break;
                    case GX_IA8:
                        c = Rgba(d[2 * o + 1], d[2 * o + 1], d[2 * o + 1], d[2 * o]);
                        o++;
                        break;
                    default:   // 16-bit texels
                        v = (uint32_t)(d[2 * o] << 8 | d[2 * o + 1]);
                        o++;
                        c = format == GX_RGB565 ? C565((uint16_t)v) : format == GX_RGB5A3 ? C5A3((uint16_t)v)
                                                                       : Pal(tf, tlut, entries, v & 0x3FFF);
                        break;
                    }
                    if (px < w && py < h)
                        out[py * w + px] = c;
                }
        }
    return 1;
}
