// Moemon Rhythm: drawing the DANCE machine's screen. One 240x160 picture in 24-bit color (pictures blend over it
// with their alpha), turned into the GBA's 15-bit colors once a frame and put where the game's picture goes, so the
// window's scaling, its filters and SPECTATE treat it as the game (docs/RHYTHM_DECISIONS.md #6, #17).
// Text is the game's own fonts' glyphs (text.c MP_RenderFontGlyph).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "constants/characters.h"
#include "text.h"
#include "rh.h"

unsigned char *stbi_load(const char *filename, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *retval_from_stbi_load);
u8 MP_RenderFontGlyph(u8 fontId, u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);

static uint32_t sPix[RH_W * RH_H];

void RhGfx_Clear(uint32_t rgb)
{
    int i;
    for (i = 0; i < RH_W * RH_H; i++)
        sPix[i] = rgb;
}

static inline void Blend(uint32_t *d, int r, int g, int b, int a)
{
    uint32_t c = *d;
    int dr = c >> 16 & 255, dg = c >> 8 & 255, db = c & 255;
    dr += (r - dr) * a >> 8;
    dg += (g - dg) * a >> 8;
    db += (b - db) * a >> 8;
    *d = (uint32_t)dr << 16 | (uint32_t)dg << 8 | (uint32_t)db;
}

void RhGfx_Fill(int x, int y, int w, int h, uint32_t rgb, int alpha)
{
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y, x1 = x + w > RH_W ? RH_W : x + w, y1 = y + h > RH_H ? RH_H : y + h;
    int r = rgb >> 16 & 255, g = rgb >> 8 & 255, b = rgb & 255, i, j;
    if (alpha <= 0)
        return;
    for (j = y0; j < y1; j++)
        for (i = x0; i < x1; i++)
        {
            if (alpha >= 256)
                sPix[j * RH_W + i] = rgb;
            else
                Blend(&sPix[j * RH_W + i], r, g, b, alpha);
        }
}

// The source pixel for screen offset (i, j) of a w x h picture turned by rot
static inline const uint8_t *Src(const uint8_t *f, int w, int h, int i, int j, int rot)
{
    int sx, sy;
    switch (rot)
    {
    case RH_ROT_90:     // turned clockwise: the screen's (i, j) is the picture's (j, h - 1 - i)
        sx = j;
        sy = h - 1 - i;
        break;
    case RH_ROT_180:
        sx = w - 1 - i;
        sy = h - 1 - j;
        break;
    case RH_ROT_270:
        sx = w - 1 - j;
        sy = i;
        break;
    default:
        sx = i;
        sy = j;
        break;
    }
    return f + (sy * w + sx) * 4;
}

static void DrawPic(const struct RhPic *pic, int frame, int x, int y, int sy0, int sh, int rot, int alpha, int glow)
{
    const uint8_t *f;
    int w, h, i, j;
    if (!pic || alpha <= 0)
        return;
    if (frame < 0)
        frame = 0;
    frame %= pic->frames;
    f = pic->rgba + (size_t)frame * pic->w * pic->h * 4;
    w = pic->w;
    h = pic->h;
    if (sh <= 0)
        sh = h;
    for (j = 0; j < sh; j++)
    {
        int yy = y + j, sj = sy0 + j;
        if (yy < 0 || yy >= RH_H || sj < 0 || sj >= h)
            continue;
        for (i = 0; i < w; i++)
        {
            int xx = x + i, a, r, g, b;
            const uint8_t *s;
            if (xx < 0 || xx >= RH_W)
                continue;
            s = (rot == RH_ROT_0) ? f + (sj * w + i) * 4 : Src(f, w, h, i, sj, rot);
            a = s[3] * alpha >> 8;
            if (a == 0)
                continue;
            r = s[0];
            g = s[1];
            b = s[2];
            if (glow > 0)
            {
                r += (255 - r) * glow >> 8;
                g += (255 - g) * glow >> 8;
                b += (255 - b) * glow >> 8;
            }
            if (a >= 255)
                sPix[yy * RH_W + xx] = (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
            else
                Blend(&sPix[yy * RH_W + xx], r, g, b, a + 1);
        }
    }
}

void RhGfx_Pic(const struct RhPic *pic, int frame, int x, int y, int rot, int alpha)
{
    DrawPic(pic, frame, x, y, 0, 0, rot, alpha, 0);
}

void RhGfx_PicPart(const struct RhPic *pic, int frame, int x, int y, int sy, int sh, int rot, int alpha)
{
    DrawPic(pic, frame, x, y, sy, sh, rot, alpha, 0);
}

void RhGfx_PicGlow(const struct RhPic *pic, int frame, int x, int y, int rot, int alpha, int glow)
{
    DrawPic(pic, frame, x, y, 0, 0, rot, alpha, glow);
}

void RhGfx_PicScaled(const struct RhPic *pic, int frame, int cx, int cy, int w, int h, int alpha)
{
    const uint8_t *f;
    int x0, y0, i, j;
    if (!pic || alpha <= 0 || w <= 0 || h <= 0)
        return;
    if (w == pic->w && h == pic->h)
    {
        DrawPic(pic, frame, cx - w / 2, cy - h / 2, 0, 0, 0, alpha, 0);
        return;
    }
    frame = (frame < 0 ? 0 : frame) % pic->frames;
    f = pic->rgba + (size_t)frame * pic->w * pic->h * 4;
    x0 = cx - w / 2;
    y0 = cy - h / 2;
    for (j = 0; j < h; j++)
    {
        int yy = y0 + j;
        // bilinear, from the middle of each screen pixel
        int sy = (int)(((j + 0.5) * pic->h / h - 0.5) * 256);
        if (yy < 0 || yy >= RH_H)
            continue;
        for (i = 0; i < w; i++)
        {
            int xx = x0 + i, sx, ix, iy, fx, fy, k, c[4], a;
            if (xx < 0 || xx >= RH_W)
                continue;
            sx = (int)(((i + 0.5) * pic->w / w - 0.5) * 256);
            ix = sx >> 8;
            iy = sy >> 8;
            fx = sx & 255;
            fy = sy & 255;
            if (sx < 0)
                ix = 0, fx = 0;
            if (sy < 0)
                iy = 0, fy = 0;
            for (k = 0; k < 4; k++)
            {
                int x1 = ix + 1 < pic->w ? ix + 1 : ix, y1 = iy + 1 < pic->h ? iy + 1 : iy;
                const uint8_t *p00 = f + (iy * pic->w + ix) * 4, *p10 = f + (iy * pic->w + x1) * 4;
                const uint8_t *p01 = f + (y1 * pic->w + ix) * 4, *p11 = f + (y1 * pic->w + x1) * 4;
                // (colors weighted by alpha, so transparent pixels don't darken the edges)
                int top, bot;
                if (k < 3)
                {
                    top = (p00[k] * p00[3] * (256 - fx) + p10[k] * p10[3] * fx) >> 8;
                    bot = (p01[k] * p01[3] * (256 - fx) + p11[k] * p11[3] * fx) >> 8;
                }
                else
                {
                    top = (p00[3] * (256 - fx) + p10[3] * fx) >> 8;
                    bot = (p01[3] * (256 - fx) + p11[3] * fx) >> 8;
                }
                c[k] = (top * (256 - fy) + bot * fy) >> 8;
            }
            if (c[3] == 0)
                continue;
            a = c[3] * alpha >> 8;
            Blend(&sPix[yy * RH_W + xx], c[0] / c[3] > 255 ? 255 : c[0] / c[3], c[1] / c[3] > 255 ? 255 : c[1] / c[3],
                  c[2] / c[3] > 255 ? 255 : c[2] / c[3], a + 1 > 256 ? 256 : a + 1);
        }
    }
}

void RhGfx_Image(const struct RhImage *img, int x, int y, int alpha)
{
    struct RhPic pic;
    if (!img || !img->rgba)
        return;
    pic.rgba = img->rgba;
    pic.w = (uint16_t)img->w;
    pic.h = (uint16_t)img->h;
    pic.frames = 1;
    DrawPic(&pic, 0, x, y, 0, 0, 0, alpha, 0);
}

// ---------------------------------------------------------------------------
// Pictures from files: an area filter down to the size (cover: fill it, cutting the sides off; else fit inside)

int RhImage_Load(struct RhImage *img, const char *path, int w, int h, int cover)
{
    int sw, sh, comp, x, y;
    unsigned char *src;
    double scale, ox = 0, oy = 0;
    memset(img, 0, sizeof(*img));
    src = path && path[0] ? stbi_load(path, &sw, &sh, &comp, 4) : NULL;
    if (!src)
        return 0;
    if (w <= 0 || h <= 0)
    {
        w = sw;
        h = sh;
    }
    scale = cover ? (double)sw / w < (double)sh / h ? (double)sw / w : (double)sh / h
                  : (double)sw / w > (double)sh / h ? (double)sw / w : (double)sh / h;
    if (!cover)
    {
        w = (int)(sw / scale + 0.5);
        h = (int)(sh / scale + 0.5);
        if (w < 1)
            w = 1;
        if (h < 1)
            h = 1;
    }
    else
    {
        ox = (sw - w * scale) / 2;
        oy = (sh - h * scale) / 2;
    }
    img->rgba = malloc((size_t)w * h * 4);
    img->w = w;
    img->h = h;
    for (y = 0; y < h; y++)
    {
        int y0 = (int)(oy + y * scale), y1 = (int)(oy + (y + 1) * scale);
        if (y1 <= y0)
            y1 = y0 + 1;
        if (y1 > sh)
            y1 = sh;
        for (x = 0; x < w; x++)
        {
            int x0 = (int)(ox + x * scale), x1 = (int)(ox + (x + 1) * scale), i, j;
            uint32_t r = 0, g = 0, b = 0, a = 0, n = 0;
            uint8_t *d = img->rgba + (y * w + x) * 4;
            if (x1 <= x0)
                x1 = x0 + 1;
            if (x1 > sw)
                x1 = sw;
            for (j = y0; j < y1; j++)
                for (i = x0; i < x1; i++)
                {
                    const unsigned char *s = src + (j * sw + i) * 4;
                    r += s[0] * s[3];
                    g += s[1] * s[3];
                    b += s[2] * s[3];
                    a += s[3];
                    n++;
                }
            d[0] = (uint8_t)(a ? r / a : 0);
            d[1] = (uint8_t)(a ? g / a : 0);
            d[2] = (uint8_t)(a ? b / a : 0);
            d[3] = (uint8_t)(n ? a / n : 0);
        }
    }
    stbi_image_free(src);
    return 1;
}

void RhImage_Free(struct RhImage *img)
{
    free(img->rgba);
    memset(img, 0, sizeof(*img));
}

// ---------------------------------------------------------------------------
// Text: the game's glyphs, cached as 16x16 masks per font (1 = the letter, 2 = its shadow)

struct Glyph
{
    uint8_t ready, w;
    uint8_t mask[16][16];
};

static struct Glyph sGlyphs[3][256];

static const u8 sFontIds[3] = {FONT_SMALL, FONT_NORMAL, FONT_NARROW};

static u16 GameChar(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return CHAR_A + (c - 'A');
    if (c >= 'a' && c <= 'z')
        return CHAR_a + (c - 'a');
    if (c >= '0' && c <= '9')
        return CHAR_0 + (c - '0');
    switch (c)
    {
    case ' ': return CHAR_SPACE;
    case '!': return CHAR_EXCL_MARK;
    case '?': return CHAR_QUESTION_MARK;
    case '.': return CHAR_PERIOD;
    case ',': return CHAR_COMMA;
    case '-': return CHAR_HYPHEN;
    case '~': return CHAR_HYPHEN;
    case '\'': return CHAR_SGL_QUOTE_RIGHT;
    case '`': return CHAR_SGL_QUOTE_LEFT;
    case '"': return CHAR_DBL_QUOTE_LEFT;
    case '/': return CHAR_SLASH;
    case ':': return CHAR_COLON;
    case '&': return CHAR_AMPERSAND;
    case '(': case '[': case '{': return CHAR_LEFT_PAREN;
    case ')': case ']': case '}': return CHAR_RIGHT_PAREN;
    case '+': return CHAR_PLUS;
    case '=': return CHAR_EQUALS;
    case '%': return CHAR_PERCENT;
    case ';': return CHAR_SEMICOLON;
    case '<': return CHAR_LESS_THAN;
    case '>': return CHAR_GREATER_THAN;
    case '*': return CHAR_MULT_SIGN;
    case 0xE9: return CHAR_e_ACUTE;
    }
    return CHAR_QUESTION_MARK;
}

static const struct Glyph *GetGlyph(int font, unsigned char c)
{
    struct Glyph *g;
    int x, y, w;
    if (font < 0 || font > 2)
        font = 0;
    g = &sGlyphs[font][c];
    if (g->ready)
        return g;
    g->ready = 1;
    w = MP_RenderFontGlyph(sFontIds[font], GameChar(c), 1, 0, 2);
    g->w = (uint8_t)(w > 16 ? 16 : w);
    for (y = 0; y < 16; y++)
    {
        const u32 *rows = y < 8 ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
        for (x = 0; x < 16; x++)
        {
            u8 v = (rows[(x < 8 ? 0 : 8) + (y % 8)] >> ((x % 8) * 4)) & 0xF;
            g->mask[y][x] = v == 1 ? 1 : v == 2 ? 2 : 0;
        }
    }
    return g;
}

static int CharAdvance(int font, unsigned char c)
{
    if (c == ' ')
        return font == RH_FONT_NORMAL ? 4 : 3;
    return GetGlyph(font, c)->w;
}

int RhGfx_TextWidth(int font, const char *text)
{
    int w = 0;
    for (; *text; text++)
        w += CharAdvance(font, (unsigned char)*text);
    return w;
}

static void DrawText(int font, int x, int y, const char *text, uint32_t rgb, int maxW)
{
    int r = rgb >> 16 & 255, g = rgb >> 8 & 255, b = rgb & 255, start = x;
    // the small fonts sit two rows down in their 16
    int dy = font == RH_FONT_NORMAL ? -1 : -2;
    for (; *text; text++)
    {
        unsigned char c = (unsigned char)*text;
        const struct Glyph *gl;
        int i, j;
        if (maxW > 0 && x - start + CharAdvance(font, c) > maxW)
            break;
        if (c == ' ')
        {
            x += CharAdvance(font, c);
            continue;
        }
        gl = GetGlyph(font, c);
        for (j = 0; j < 16; j++)
        {
            int yy = y + j + dy;
            if (yy < 0 || yy >= RH_H)
                continue;
            for (i = 0; i < 16; i++)
            {
                int xx = x + i;
                if (xx < 0 || xx >= RH_W || !gl->mask[j][i])
                    continue;
                if (gl->mask[j][i] == 1)
                    sPix[yy * RH_W + xx] = (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
                else
                    Blend(&sPix[yy * RH_W + xx], 0, 0, 0, 160);
            }
        }
        x += gl->w;
    }
}

int RhGfx_Text(int font, int x, int y, const char *text, uint32_t rgb, int align)
{
    int w = RhGfx_TextWidth(font, text);
    if (align == RH_CENTER)
        x -= w / 2;
    else if (align == RH_RIGHT)
        x -= w;
    DrawText(font, x, y, text, rgb, 0);
    return w;
}

int RhGfx_TextFit(int font, int x, int y, const char *text, uint32_t rgb, int align, int maxW)
{
    char buf[256];
    int w = RhGfx_TextWidth(font, text), n;
    if (w <= maxW)
        return RhGfx_Text(font, x, y, text, rgb, align);
    // the narrow font first, then cut with "..."
    if (font != RH_FONT_NARROW && RhGfx_TextWidth(RH_FONT_NARROW, text) <= maxW)
        return RhGfx_Text(RH_FONT_NARROW, x, y, text, rgb, align);
    font = RH_FONT_NARROW;
    snprintf(buf, sizeof(buf), "%s", text);
    n = (int)strlen(buf);
    while (n > 0)
    {
        buf[n - 1] = 0;
        n--;
        while (n > 0 && buf[n - 1] == ' ')
            buf[--n] = 0;
        {
            char tmp[260];
            snprintf(tmp, sizeof(tmp), "%s...", buf);
            if (RhGfx_TextWidth(font, tmp) <= maxW)
                return RhGfx_Text(font, x, y, tmp, rgb, align);
        }
    }
    return 0;
}

int RhGfx_Number(const struct RhPic *digits, int x, int y, uint32_t value, int align, int zeros, int alpha, int spacing)
{
    char buf[16];
    int n, w, i;
    if (!digits)
        return 0;
    n = snprintf(buf, sizeof(buf), "%0*u", zeros > 0 ? zeros : 1, value);
    w = n * (digits->w + spacing) - spacing;
    if (align == RH_CENTER)
        x -= w / 2;
    else if (align == RH_RIGHT)
        x -= w;
    for (i = 0; i < n; i++)
        RhGfx_Pic(digits, buf[i] - '0', x + i * (digits->w + spacing), y, 0, alpha);
    return w;
}

void RhGfx_ToGba(uint16_t *out)
{
    int i;
    for (i = 0; i < RH_W * RH_H; i++)
    {
        uint32_t c = sPix[i];
        out[i] = (uint16_t)((c >> 19 & 31) | (c >> 11 & 31) << 5 | (c >> 3 & 31) << 10 | 0x8000);   // (bit 15: drawn)
    }
}
