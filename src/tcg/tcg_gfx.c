#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include "mp_version.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "fonts.h"
#include "text_window.h"
#include "constants/characters.h"
#include "tcg/card_ids.h"
#include "tcg_gfx.h"

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
extern const unsigned char gTcgOriginalArt[], gTcgOriginalArtEnd[], gTcgPrintedArt[], gTcgPrintedArtEnd[];
void stbi_image_free(void *);

static SDL_Renderer *sR;
static SDL_Texture *sFontFace[3], *sFontShadow[3], *sWindowFrame;
static unsigned sWindowFrameId = 0xffff;
static int sFontReady;

/* ------------------------------------------------------------------ */
/* Software surfaces                                                   */
/* ------------------------------------------------------------------ */
struct GSurface G_SurfaceNew(int w, int h)
{
    struct GSurface s;
    s.w = w;
    s.h = h;
    s.px = calloc((size_t)w * h, sizeof(uint32_t));
    return s;
}
void G_SurfaceFree(struct GSurface *s)
{
    free(s->px);
    s->px = NULL;
}
SDL_Texture *G_SurfaceTexture(const struct GSurface *s)
{
    SDL_Texture *t;
    if (!sR || !s->px)
        return NULL;
    t = SDL_CreateTexture(sR, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, s->w, s->h);
    if (!t)
        return NULL;
    SDL_UpdateTexture(t, NULL, s->px, s->w * 4);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(t, SDL_ScaleModeNearest);
    return t;
}
void S_Px(struct GSurface *s, int x, int y, uint32_t argb)
{
    if (x >= 0 && y >= 0 && x < s->w && y < s->h)
        s->px[y * s->w + x] = argb;
}
void S_Blend(struct GSurface *s, int x, int y, unsigned rgb, unsigned alpha)
{
    uint32_t *p, d;
    unsigned da, r, g, b, a;
    if (x < 0 || y < 0 || x >= s->w || y >= s->h || !alpha)
        return;
    p = &s->px[y * s->w + x];
    d = *p;
    da = d >> 24;
    if (!da)
    {
        *p = ARGB(rgb, alpha);
        return;
    }
    a = alpha + da * (255 - alpha) / 255;
    r = (((rgb >> 16) & 255) * alpha + ((d >> 16) & 255) * (255 - alpha)) / 255;
    g = (((rgb >> 8) & 255) * alpha + ((d >> 8) & 255) * (255 - alpha)) / 255;
    b = ((rgb & 255) * alpha + (d & 255) * (255 - alpha)) / 255;
    *p = a << 24 | r << 16 | g << 8 | b;
}
void S_Fill(struct GSurface *s, int x, int y, int w, int h, uint32_t argb)
{
    int i, j;
    for (j = y; j < y + h; j++)
        for (i = x; i < x + w; i++)
            S_Px(s, i, j, argb);
}
void S_Frame(struct GSurface *s, int x, int y, int w, int h, uint32_t argb)
{
    S_Fill(s, x, y, w, 1, argb);
    S_Fill(s, x, y + h - 1, w, 1, argb);
    S_Fill(s, x, y, 1, h, argb);
    S_Fill(s, x + w - 1, y, 1, h, argb);
}
static int Corner(int i, int j, int w, int h, int radius, int *edge)
{
    /* Distance of (i,j) inside a rounded rectangle's corner region. */
    int cx = i < radius ? radius - 1 : i >= w - radius ? w - radius : i;
    int cy = j < radius ? radius - 1 : j >= h - radius ? h - radius : j;
    float dx = (float)(i - cx), dy = (float)(j - cy), d = sqrtf(dx * dx + dy * dy);
    if (radius <= 0)
    {
        *edge = i == 0 || j == 0 || i == w - 1 || j == h - 1;
        return 1;
    }
    if ((i < radius || i >= w - radius) && (j < radius || j >= h - radius))
    {
        if (d > radius - 0.5f)
            return 0;
        *edge = d > radius - 1.5f;
        return 1;
    }
    *edge = i == 0 || j == 0 || i == w - 1 || j == h - 1;
    return 1;
}
void S_RoundFrame(struct GSurface *s, int x, int y, int w, int h, int radius, uint32_t argb)
{
    int i, j, edge;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (Corner(i, j, w, h, radius, &edge) && edge)
                S_Blend(s, x + i, y + j, argb & 0xffffff, argb >> 24);   /* (blended: a see-through line never
                                                                            punches a hole in what's under it) */
}
void S_RoundFill(struct GSurface *s, int x, int y, int w, int h, int radius, uint32_t argb)
{
    int i, j, edge;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            if (Corner(i, j, w, h, radius, &edge))
                S_Blend(s, x + i, y + j, argb & 0xffffff, argb >> 24);
}
void S_Ring(struct GSurface *s, float cx, float cy, float radius, float thickness, uint32_t argb)
{
    int x, y;
    for (y = (int)(cy - radius - 2); y <= (int)(cy + radius + 2); y++)
        for (x = (int)(cx - radius - 2); x <= (int)(cx + radius + 2); x++)
        {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy, d = sqrtf(dx * dx + dy * dy);
            if (d <= radius && d > radius - thickness)
                S_Blend(s, x, y, argb & 0xffffff, argb >> 24);
        }
}
void S_Disc(struct GSurface *s, float cx, float cy, float radius, uint32_t argb)
{
    int x, y;
    for (y = (int)(cy - radius - 1); y <= (int)(cy + radius + 1); y++)
        for (x = (int)(cx - radius - 1); x <= (int)(cx + radius + 1); x++)
        {
            float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
            if (dx * dx + dy * dy <= radius * radius)
                S_Blend(s, x, y, argb & 0xffffff, argb >> 24);
        }
}

/* ------------------------------------------------------------------ */
/* Renderer primitives                                                 */
/* ------------------------------------------------------------------ */
SDL_Renderer *G_R(void) { return sR; }
unsigned G_Rgb15(unsigned c)
{
    return ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}
unsigned G_Mix(unsigned a, unsigned b, float t)
{
    unsigned out = 0, shift;
    if (t < 0)
        t = 0;
    if (t > 1)
        t = 1;
    for (shift = 0; shift < 24; shift += 8)
    {
        float ca = (float)((a >> shift) & 255), cb = (float)((b >> shift) & 255);
        out |= (unsigned)(ca + (cb - ca) * t + 0.5f) << shift;
    }
    return out;
}
static void SetColor(unsigned rgb, unsigned alpha)
{
    SDL_SetRenderDrawBlendMode(sR, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sR, (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha > 255 ? 255 : alpha);
}
void G_FillA(int x, int y, int w, int h, unsigned rgb, unsigned alpha)
{
    SDL_Rect r = {x, y, w, h};
    if (w <= 0 || h <= 0 || !alpha)
        return;
    SetColor(rgb, alpha);
    SDL_RenderFillRect(sR, &r);
}
void G_Fill(int x, int y, int w, int h, unsigned rgb) { G_FillA(x, y, w, h, rgb, 255); }
/* Light added over what is there (a foil's shine: it brightens the picture without covering it) */
void G_FillAdd(int x, int y, int w, int h, unsigned rgb, unsigned alpha)
{
    SDL_Rect r = {x, y, w, h};
    if (w <= 0 || h <= 0 || !alpha)
        return;
    SDL_SetRenderDrawBlendMode(sR, SDL_BLENDMODE_ADD);
    SDL_SetRenderDrawColor(sR, (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha > 255 ? 255 : alpha);
    SDL_RenderFillRect(sR, &r);
    SDL_SetRenderDrawBlendMode(sR, SDL_BLENDMODE_BLEND);
}
void G_Frame(int x, int y, int w, int h, unsigned rgb, unsigned alpha)
{
    G_FillA(x, y, w, 1, rgb, alpha);
    G_FillA(x, y + h - 1, w, 1, rgb, alpha);
    G_FillA(x, y + 1, 1, h - 2, rgb, alpha);
    G_FillA(x + w - 1, y + 1, 1, h - 2, rgb, alpha);
}
void G_RoundFill(int x, int y, int w, int h, unsigned rgb, unsigned alpha)
{
    G_FillA(x + 1, y, w - 2, 1, rgb, alpha);
    G_FillA(x, y + 1, w, h - 2, rgb, alpha);
    G_FillA(x + 1, y + h - 1, w - 2, 1, rgb, alpha);
}
void G_RoundFrame(int x, int y, int w, int h, unsigned rgb, unsigned alpha)
{
    G_FillA(x + 1, y, w - 2, 1, rgb, alpha);
    G_FillA(x + 1, y + h - 1, w - 2, 1, rgb, alpha);
    G_FillA(x, y + 1, 1, h - 2, rgb, alpha);
    G_FillA(x + w - 1, y + 1, 1, h - 2, rgb, alpha);
}
void G_Line(int x0, int y0, int x1, int y1, unsigned rgb, unsigned alpha)
{
    SetColor(rgb, alpha);
    SDL_RenderDrawLine(sR, x0, y0, x1, y1);
}

/* ------------------------------------------------------------------ */
/* Emerald fonts                                                        */
/* ------------------------------------------------------------------ */
static unsigned GameChar(const unsigned char **s)
{
    unsigned char c = **s;
    (*s)++;
    if (c == 0xC3 && **s)
    {
        unsigned char d = **s;
        (*s)++;
        if (d == 0xA9)
            return CHAR_e_ACUTE;
        if (d == 0x97)
            return CHAR_MULT_SIGN;
        return CHAR_QUESTION_MARK;
    }
    if (c >= 'A' && c <= 'Z')
        return CHAR_A + c - 'A';
    if (c >= 'a' && c <= 'z')
        return CHAR_a + c - 'a';
    if (c >= '0' && c <= '9')
        return CHAR_0 + c - '0';
    switch (c)
    {
    case ' ':
        return CHAR_SPACE;
    case '!':
        return CHAR_EXCL_MARK;
    case '?':
        return CHAR_QUESTION_MARK;
    case '.':
        return CHAR_PERIOD;
    case '-':
        return CHAR_HYPHEN;
    case ',':
        return CHAR_COMMA;
    case '/':
        return CHAR_SLASH;
    case ':':
        return CHAR_COLON;
    case '\'':
        return CHAR_SGL_QUOTE_RIGHT;
    case '"':
        return CHAR_DBL_QUOTE_RIGHT;
    case '(':
        return CHAR_LEFT_PAREN;
    case ')':
        return CHAR_RIGHT_PAREN;
    case '+':
        return CHAR_PLUS;
    case '%':
        return CHAR_PERCENT;
    case 0xA5:   /* Latin-1 yen: the game's money sign (the GAME CENTER CARD SHOP) */
        return CHAR_CURRENCY;
    case '&':
        return CHAR_AMPERSAND;
    case '=':
        return CHAR_EQUALS;
    case ';':
        return CHAR_SEMICOLON;
    case '*':
        return CHAR_MULT_SIGN;
    case '<':
        return CHAR_LEFT_ARROW;
    case '>':
        return CHAR_RIGHT_ARROW;
    case '^':
        return CHAR_UP_ARROW;
    case '_':
        return CHAR_DOWN_ARROW;
    default:
        return CHAR_QUESTION_MARK;
    }
}
static const u16 *FontGlyphs(int font)
{
    return font == TF_NORMAL ? gFontNormalLatinGlyphs : font == TF_SMALL ? gFontSmallLatinGlyphs
                                                                         : gFontSmallNarrowLatinGlyphs;
}
static int FontWidth(unsigned ch, int font)
{
    return font == TF_NORMAL  ? gFontNormalLatinGlyphWidths[ch]
           : font == TF_SMALL ? gFontSmallLatinGlyphWidths[ch]
                              : gFontSmallNarrowLatinGlyphWidths[ch];
}
int G_LineHeight(int font) { return font == TF_NORMAL ? 16 : font == TF_SMALL ? 12 : 11; }
static void LoadFonts(void)
{
    unsigned font, code, x, y;
    uint32_t *face, *shadow;
    if (sFontReady)
        return;
    sFontReady = 1;
    face = calloc(256 * 256, sizeof(*face));
    shadow = calloc(256 * 256, sizeof(*shadow));
    if (!face || !shadow)
    {
        free(face);
        free(shadow);
        return;
    }
    for (font = 0; font < 3; font++)
    {
        const u16 *glyphs = FontGlyphs(font);
        memset(face, 0, 256 * 256 * 4);
        memset(shadow, 0, 256 * 256 * 4);
        for (code = 0; code < 256; code++)
            for (y = 0; y < 16; y++)
                for (x = 0; x < 16; x++)
                {
                    unsigned tile = (y / 8) * 2 + x / 8;
                    unsigned value = (glyphs[code * 32 + tile * 8 + y % 8] >> (14 - 2 * (x % 8))) & 3;
                    unsigned dst = ((code / 16) * 16 + y) * 256 + (code % 16) * 16 + x;
                    if (value == 1)
                        face[dst] = 0xffffffffu;
                    if (value == 2)
                        shadow[dst] = 0xffffffffu;
                }
        sFontFace[font] = SDL_CreateTexture(sR, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 256);
        sFontShadow[font] = SDL_CreateTexture(sR, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 256);
        if (sFontFace[font])
        {
            SDL_UpdateTexture(sFontFace[font], NULL, face, 256 * 4);
            SDL_SetTextureBlendMode(sFontFace[font], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sFontFace[font], SDL_ScaleModeNearest);
        }
        if (sFontShadow[font])
        {
            SDL_UpdateTexture(sFontShadow[font], NULL, shadow, 256 * 4);
            SDL_SetTextureBlendMode(sFontShadow[font], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sFontShadow[font], SDL_ScaleModeNearest);
        }
    }
    free(face);
    free(shadow);
}
int G_Measure(const char *s, int font)
{
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;
    while (*p)
    {
        if (*p == '\n')
        {
            p++;
            continue;
        }
        w += FontWidth(GameChar(&p), font);
    }
    return w;
}
static unsigned sTextAlpha = 255;   /* (G_TextAlpha) */
void G_TextAlpha(unsigned alpha) { sTextAlpha = alpha > 255 ? 255 : alpha; }
static void Glyph(unsigned code, int x, int y, int font, unsigned color, unsigned shadow)
{
    int w = FontWidth(code, font), h = font == TF_NORMAL ? 15 : 12;
    SDL_Rect src = {(int)(code & 15) * 16, (int)(code >> 4) * 16, w, h}, dst = {x, y, w, h};
    SDL_Texture *sh = sFontShadow[font], *face = sFontFace[font];
    if (!sh || !face)
        return;
    if (shadow != G_NO_SHADOW)
    {
        SDL_SetTextureColorMod(sh, (shadow >> 16) & 255, (shadow >> 8) & 255, shadow & 255);
        SDL_SetTextureAlphaMod(sh, (Uint8)sTextAlpha);
        SDL_RenderCopy(sR, sh, &src, &dst);
    }
    SDL_SetTextureColorMod(face, (color >> 16) & 255, (color >> 8) & 255, color & 255);
    SDL_SetTextureAlphaMod(face, (Uint8)sTextAlpha);
    SDL_RenderCopy(sR, face, &src, &dst);
}
static int sMeasureOnly;   /* (G_TextHeight: the same wrapping, nothing drawn) */
int G_TextHeight(const char *text, int width, int font)
{
    int h;
    sMeasureOnly = 1;
    h = G_Text(0, 0, 0, 0, text, width, font);
    sMeasureOnly = 0;
    return text && *text ? h : 0;
}
int G_Text(int x, int y, unsigned color, unsigned shadow, const char *text, int width, int font)
{
    int px = x, py = y, start = 1, line = G_LineHeight(font);
    const unsigned char *p = (const unsigned char *)text;
    while (*p)
    {
        unsigned char ch = *p;
        unsigned code;
        int w;
        if (start && width > 0 && ch != ' ' && ch != '\n')
        {
            const unsigned char *next = p;
            int word = 0;
            while (*next && *next != ' ' && *next != '\n')
                word += FontWidth(GameChar(&next), font);
            if (px > x && px + word > x + width)
            {
                px = x;
                py += line;
            }
        }
        start = ch == ' ' || ch == '\n';
        if (ch == '\n')
        {
            p++;
            px = x;
            py += line;
            continue;
        }
        code = GameChar(&p);
        w = FontWidth(code, font);
        if (width > 0 && px + w > x + width && ch != ' ')
        {
            px = x;
            py += line;
        }
        if (!sMeasureOnly)
            Glyph(code, px, py, font, color, shadow);
        px += w;
    }
    return py + line;
}
void G_TextAlign(int x, int y, int align, unsigned color, unsigned shadow, const char *text, int font)
{
    int w = G_Measure(text, font);
    if (align == TA_CENTER)
        x -= w / 2;
    else if (align == TA_RIGHT)
        x -= w;
    G_Text(x, y, color, shadow, text, 0, font);
}
void G_Window(int x, int y, int w, int h)
{
    unsigned frame = gSaveBlock2Ptr ? gSaveBlock2Ptr->optionsWindowFrameType : 0;
    const struct TilesPal *assets;
    unsigned t, px, py;
    if (frame >= WINDOW_FRAMES_COUNT)
        frame = 0;
    if (!sWindowFrame || sWindowFrameId != frame)
    {
        uint32_t pixels[24 * 24];
        assets = GetWindowFrameTilesPal(frame);
        if (sWindowFrame)
            SDL_DestroyTexture(sWindowFrame);
        sWindowFrameId = frame;
        for (t = 0; t < 9; t++)
            for (py = 0; py < 8; py++)
                for (px = 0; px < 8; px++)
                {
                    unsigned index = (assets->tiles[t * 32 + py * 4 + px / 2] >> ((px & 1) * 4)) & 15;
                    unsigned dst = ((t / 3) * 8 + py) * 24 + (t % 3) * 8 + px;
                    pixels[dst] = index ? ARGB(G_Rgb15(assets->pal[index]), 255) : 0;
                }
        sWindowFrame = SDL_CreateTexture(sR, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 24, 24);
        if (sWindowFrame)
        {
            SDL_UpdateTexture(sWindowFrame, NULL, pixels, 24 * 4);
            SDL_SetTextureBlendMode(sWindowFrame, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(sWindowFrame, SDL_ScaleModeNearest);
        }
    }
    G_Fill(x + 4, y + 4, w - 8, h - 8, 0xf8f8f0);
    if (!sWindowFrame)
        return;
    for (py = 0; py < (unsigned)h; py += 8)
        for (px = 0; px < (unsigned)w; px += 8)
        {
            unsigned tx = px == 0 ? 0 : px + 8 >= (unsigned)w ? 2 : 1, ty = py == 0 ? 0 : py + 8 >= (unsigned)h ? 2 : 1;
            SDL_Rect src = {(int)tx * 8, (int)ty * 8, 8, 8},
                     dst = {x + (int)px, y + (int)py, w - (int)px < 8 ? w - (int)px : 8,
                            h - (int)py < 8 ? h - (int)py : 8};
            if (tx == 2 && px + 8 > (unsigned)w)
                dst.x = x + w - 8, dst.w = 8;
            if (ty == 2 && py + 8 > (unsigned)h)
                dst.y = y + h - 8, dst.h = 8;
            src.w = dst.w;
            src.h = dst.h;
            SDL_RenderCopy(sR, sWindowFrame, &src, &dst);
        }
}
/* The card tables' cursor, shared by the three games: corner brackets with a shadow that step out and back
   every 8 frames. White, or yellow (G_FOCUS_TARGET) while picking a target. */
void G_Focus(int x, int y, int w, int h, unsigned color, unsigned frame)
{
    int o = (frame / 8) % 2 + 1, len = 4, x0 = x - o - 1, y0 = y - o - 1, x1 = x + w + o, y1 = y + h + o;
    unsigned shadow = 0x102010;
    G_Fill(x0 + 1, y0 + 1, len, 1, shadow);
    G_Fill(x0 + 1, y0 + 1, 1, len, shadow);
    G_Fill(x1 - len + 1, y0 + 1, len, 1, shadow);
    G_Fill(x1 + 1, y0 + 1, 1, len, shadow);
    G_Fill(x0 + 1, y1 + 1, len, 1, shadow);
    G_Fill(x0 + 1, y1 - len + 2, 1, len, shadow);
    G_Fill(x1 - len + 1, y1 + 1, len + 1, 1, shadow);
    G_Fill(x1 + 1, y1 - len + 2, 1, len, shadow);
    G_Fill(x0, y0, len, 1, color);
    G_Fill(x0, y0, 1, len, color);
    G_Fill(x1 - len + 1, y0, len, 1, color);
    G_Fill(x1, y0, 1, len, color);
    G_Fill(x0, y1, len, 1, color);
    G_Fill(x0, y1 - len + 1, 1, len, color);
    G_Fill(x1 - len + 1, y1, len, 1, color);
    G_Fill(x1, y1 - len + 1, 1, len, color);
}
/* A 64x64 battle picture cut to a window: the face and shoulders, centred on the figure's top */
void G_PicCut(SDL_Texture *tex, const uint32_t *px, int x, int y, int w, int h)
{
    SDL_Rect src, dst;
    int top = 64, left = 64, right = -1, i, j;
    for (j = 0; j < 64 && top == 64; j++)
        for (i = 0; i < 64; i++)
            if (px[j * 64 + i] >> 24)
            {
                top = j;
                break;
            }
    if (top == 64)
        return;
    for (j = top; j < top + 28 && j < 64; j++)
        for (i = 0; i < 64; i++)
            if (px[j * 64 + i] >> 24)
            {
                if (i < left)
                    left = i;
                if (i > right)
                    right = i;
            }
    if (w > 64)
        w = 64;
    if (h > 64)
        h = 64;
    src.w = dst.w = w;
    src.h = dst.h = h;
    src.x = (left + right + 1) / 2 - w / 2;
    src.y = top - 3;
    if (src.x < 0)
        src.x = 0;
    if (src.x > 64 - w)
        src.x = 64 - w;
    if (src.y < 0)
        src.y = 0;
    if (src.y > 64 - h)
        src.y = 64 - h;
    dst.x = x;
    dst.y = y;
    SDL_RenderCopy(sR, tex, &src, &dst);
}
void G_Circle(int cx, int cy, float r, unsigned color, unsigned alpha)
{
    int i;
    for (i = 0; i < 20; i++)
    {
        float a0 = i * 0.31416f, a1 = (i + 1) * 0.31416f;
        G_Line(cx + (int)(SDL_cosf(a0) * r), cy + (int)(SDL_sinf(a0) * r), cx + (int)(SDL_cosf(a1) * r),
               cy + (int)(SDL_sinf(a1) * r), color, alpha);
    }
}
void G_Cursor(int x, int y)
{
    int row;
    for (row = 0; row < 7; row++)
    {
        int w = row < 4 ? row + 1 : 7 - row;
        G_Fill(x, y + row, w, 1, 0x383838);
    }
}
void G_Arrow(int x, int y, int dir, unsigned rgb)
{
    int i;
    for (i = 0; i < 4; i++)
    {
        int len = 7 - i * 2;
        if (dir == 0)
            G_Fill(x + i, y + 3 - i, len, 1, rgb);
        else if (dir == 1)
            G_Fill(x + i, y + i, len, 1, rgb);
        else if (dir == 2)
            G_Fill(x + 3 - i, y + i, 1, len, rgb);
        else
            G_Fill(x + i, y + i, 1, len, rgb);
    }
}

/* ------------------------------------------------------------------ */
/* Pixel digits                                                         */
/* ------------------------------------------------------------------ */
static const char sGlyphChars[] = "0123456789HPx+-/%";
static const uint8_t sTiny[][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 3, 1, 7}, {5, 5, 7, 1, 1}, {7, 4, 7, 1, 7},
    {7, 4, 7, 5, 7}, {7, 1, 1, 2, 2}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7}, {5, 5, 7, 5, 5}, {6, 5, 6, 4, 4},
    {0, 5, 2, 5, 0}, {0, 2, 7, 2, 0}, {0, 0, 7, 0, 0}, {1, 1, 2, 4, 4}, {5, 1, 2, 4, 5},
};
/* Squared LCD-style 5x7 digits, like the DS duel plates. */
static const uint8_t sBig[][7] = {
    {0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F}, {0x06, 0x0E, 0x06, 0x06, 0x06, 0x06, 0x06},
    {0x1F, 0x01, 0x01, 0x1F, 0x10, 0x10, 0x1F}, {0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F},
    {0x11, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x01}, {0x1F, 0x10, 0x10, 0x1F, 0x01, 0x01, 0x1F},
    {0x1F, 0x10, 0x10, 0x1F, 0x11, 0x11, 0x1F}, {0x1F, 0x11, 0x01, 0x01, 0x01, 0x01, 0x01},
    {0x1F, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x1F}, {0x1F, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x1F},
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10},
    {0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11}, {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10},
    {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13},
};
static int GlyphIndex(char c)
{
    const char *p = strchr(sGlyphChars, c);
    return c && p ? (int)(p - sGlyphChars) : -1;
}
int G_DigitsWidth(const char *s, int size, int scale)
{
    int n = (int)strlen(s);
    if (!n)
        return 0;
    return (n * ((size ? 5 : 3) + 1) - 1) * scale;
}
static void DigitPixels(int x, int y, const char *s, int size, int scale, unsigned rgb, unsigned alpha, int dx,
                        int dy)
{
    SDL_Rect rects[64];
    int n = 0, gw = size ? 5 : 3, gh = size ? 7 : 5;
    SetColor(rgb, alpha);
    for (; *s; s++, x += (gw + 1) * scale)
    {
        int g = GlyphIndex(*s), row, col;
        if (g < 0)
            continue;
        for (row = 0; row < gh; row++)
            for (col = 0; col < gw; col++)
            {
                unsigned bits = size ? sBig[g][row] : sTiny[g][row];
                if (bits >> (gw - 1 - col) & 1)
                {
                    SDL_Rect r = {x + col * scale + dx, y + row * scale + dy, scale, scale};
                    rects[n++] = r;
                    if (n == 64)
                    {
                        SDL_RenderFillRects(sR, rects, n);
                        n = 0;
                    }
                }
            }
    }
    if (n)
        SDL_RenderFillRects(sR, rects, n);
}
void G_Digits(int x, int y, const char *s, int size, int scale, unsigned rgb, unsigned alpha)
{
    DigitPixels(x, y, s, size, scale, rgb, alpha, 0, 0);
}
void G_DigitsOutlined(int x, int y, const char *s, int size, int scale, unsigned fill, unsigned outline,
                      unsigned alpha)
{
    int dx, dy;
    for (dy = -1; dy <= 1; dy++)
        for (dx = -1; dx <= 1; dx++)
            if (dx || dy)
                DigitPixels(x, y, s, size, scale, outline, alpha, dx, dy);
    DigitPixels(x, y + 1, s, size, scale, G_Mix(outline, fill, 0.5f), alpha, 0, 0);
    DigitPixels(x, y, s, size, scale, fill, alpha, 0, 0);
}
int G_HpPlateWidth(int small) { return small ? 26 : 37; }
void G_HpPlate(int x, int y, int hp, unsigned maxHp, int small, unsigned alpha)
{
    char digits[8];
    int w = G_HpPlateWidth(small), h = small ? 9 : 11;
    unsigned bg = 0x14203a, label = 0x5aa8ff, text = 0xc8ecff, ghost = 0x223456;
    if (hp < 0)
        hp = 0;
    if (hp == 0)
        bg = 0x3a0c0c, label = 0xff5858, text = 0xff3c3c, ghost = 0x561c1c;
    else if (maxHp && (unsigned)hp * 4 <= maxHp)
        text = 0xffd860, ghost = 0x3a3820;
    G_RoundFill(x, y, w, h, 0x282828, alpha);
    G_FillA(x + 1, y + 1, w - 2, 1, 0x9a9a9a, alpha);
    G_FillA(x + 1, y + h - 2, w - 2, 1, 0x585858, alpha);
    G_FillA(x + 2, y + 2, w - 4, h - 4, bg, alpha);
    snprintf(digits, sizeof(digits), "%03d", hp > 999 ? 999 : hp);
    if (small)
    {
        G_Digits(x + 3, y + 2, "HP", 0, 1, label, alpha);
        G_Digits(x + w - 3 - G_DigitsWidth("888", 0, 1), y + 2, "888", 0, 1, ghost, alpha);
        G_Digits(x + w - 3 - G_DigitsWidth(digits, 0, 1), y + 2, digits, 0, 1, text, alpha);
    }
    else
    {
        G_Digits(x + 3, y + 3, "HP", 0, 1, label, alpha);
        G_Digits(x + w - 3 - G_DigitsWidth("888", 1, 1), y + 2, "888", 1, 1, ghost, alpha);
        G_Digits(x + w - 3 - G_DigitsWidth(digits, 1, 1), y + 2, digits, 1, 1, text, alpha);
    }
}

/* ------------------------------------------------------------------ */
/* Type colors, energy, status, buttons, banners, coins                 */
/* ------------------------------------------------------------------ */
unsigned G_TypeColor(unsigned type, int shade)
{
    static const unsigned colors[8][3] = {
        {0xe85830, 0xf8a878, 0x982010}, /* fire */
        {0x58b038, 0xb0e088, 0x2c6c18}, /* grass */
        {0xf0c820, 0xf8f090, 0x987800}, /* lightning */
        {0x3888e8, 0x98c8f8, 0x184ca8}, /* water */
        {0xb86c38, 0xe0b080, 0x703810}, /* fighting */
        {0x9858c0, 0xd0a8e8, 0x582880}, /* psychic */
        {0xc8c8c0, 0xf4f4ec, 0x7c7c74}, /* colorless */
        {0xd8d8e0, 0xf8f8f8, 0x707080}, /* trainer */
    };
    if (type > 7)
        type = 6;
    return colors[type][shade < 0 || shade > 2 ? 0 : shade];
}
const char *G_TypeName(unsigned type)
{
    static const char *const names[] = {"FIRE", "GRASS", "LIGHTNING", "WATER", "FIGHTING", "PSYCHIC", "COLORLESS"};
    return type < 7 ? names[type] : "";
}
void G_EnergyOrb(int x, int y, unsigned type, int big)
{
    unsigned base = G_TypeColor(type, 0), light = G_TypeColor(type, 1), dark = G_TypeColor(type, 2);
    if (big)
    {
        /* 7x7 */
        G_Fill(x + 2, y, 3, 1, 0x202020);
        G_Fill(x + 1, y + 1, 5, 5, base);
        G_Fill(x, y + 2, 7, 3, base);
        G_Fill(x + 1, y + 1, 1, 1, 0x202020);
        G_Fill(x + 5, y + 1, 1, 1, 0x202020);
        G_Fill(x, y + 2, 1, 3, 0x202020);
        G_Fill(x + 6, y + 2, 1, 3, 0x202020);
        G_Fill(x + 1, y + 5, 1, 1, 0x202020);
        G_Fill(x + 5, y + 5, 1, 1, 0x202020);
        G_Fill(x + 2, y + 6, 3, 1, 0x202020);
        G_Fill(x + 4, y + 4, 2, 1, dark);
        G_Fill(x + 2, y + 2, 2, 1, light);
        G_Fill(x + 2, y + 3, 1, 1, light);
    }
    else
    {
        /* 5x5 */
        G_Fill(x + 1, y, 3, 1, 0x202020);
        G_Fill(x, y + 1, 1, 3, 0x202020);
        G_Fill(x + 4, y + 1, 1, 3, 0x202020);
        G_Fill(x + 1, y + 4, 3, 1, 0x202020);
        G_Fill(x + 1, y + 1, 3, 3, base);
        G_Fill(x + 1, y + 1, 1, 1, light);
        G_Fill(x + 3, y + 3, 1, 1, dark);
    }
}
int G_StatusBadges(int x, int y, unsigned condition, unsigned poison)
{
    static const char *const names[] = {"", "SLP", "PAR", "CNF"};
    static const unsigned colors[] = {0, 0x4870d0, 0xd0a010, 0xd04890};
    int start = x;
    if (poison)
    {
        int w = G_Measure("PSN", TF_NARROW) + 4;
        G_RoundFill(x, y, w, 9, 0x8038b0, 255);
        G_Text(x + 2, y - 1, G_WHITE, 0x401860, "PSN", 0, TF_NARROW);
        x += w + 1;
    }
    if (condition && condition < 4)
    {
        int w = G_Measure(names[condition], TF_NARROW) + 4;
        G_RoundFill(x, y, w, 9, colors[condition], 255);
        G_Text(x + 2, y - 1, G_WHITE, G_Mix(colors[condition], 0, 0.5f), names[condition], 0, TF_NARROW);
        x += w + 1;
    }
    return x - start;
}
void G_StatusBadge(int x, int y, unsigned condition, unsigned poison) { G_StatusBadges(x, y, condition, poison); }
void G_Button(int x, int y, int w, int h, const char *label, int state)
{
    unsigned body = state == G_BUTTON_FOCUS ? 0xa8a8b0 : state == G_BUTTON_DISABLED ? 0x6c7070 : 0x8c8c90;
    unsigned top = state == G_BUTTON_DISABLED ? 0x8c9090 : 0xd8d8dc, bottom = 0x4c4c50;
    unsigned text = state == G_BUTTON_DISABLED ? 0x9ca0a0 : 0x404044;
    int press = state == G_BUTTON_PRESSED;
    G_RoundFill(x, y + 1, w, h, 0x0c3018, 160);
    G_RoundFill(x, y + press, w, h - 1, 0x303034, 255);
    G_FillA(x + 1, y + 1 + press, w - 2, h - 3, body, 255);
    G_FillA(x + 1, y + 1 + press, w - 2, 1, top, 255);
    G_FillA(x + 1, y + h - 3 + press, w - 2, 1, bottom, 255);
    if (state == G_BUTTON_FOCUS)
        G_FillA(x + 1, y + 2 + press, w - 2, (h - 4) / 2, 0xffffff, 40);
    G_TextAlign(x + w / 2, y + (h - 12) / 2 + press, TA_CENTER, text, state == G_BUTTON_DISABLED ? G_NO_SHADOW : 0xe8e8ec,
                label, TF_SMALL);
}
void G_Banner(int cx, int cy, const char *text, unsigned color, float scale, unsigned alpha)
{
    int tw = G_Measure(text, TF_NORMAL), w = (int)((tw + 20) * (scale > 1 ? 1 : scale)), h = 20;
    int x = cx - w / 2, y = cy - h / 2;
    if (w < 4)
        return;
    G_RoundFill(x - 1, y - 1, w + 2, h + 2, 0x10506a, alpha * 3 / 5);
    G_RoundFill(x, y, w, h, 0x48c8e0, alpha * 13 / 16);
    G_FillA(x + 1, y + 1, w - 2, h / 2 - 1, 0xb8f4ff, alpha / 2);
    G_RoundFrame(x, y, w, h, 0xe8ffff, alpha);
    if (scale >= 0.98f)
        G_Text(cx - tw / 2, cy - 7, color, 0xf8f8f8, text, 0, TF_NORMAL);
}
void G_Coin(int cx, int cy, float spin, int heads, unsigned alpha)
{
    /* spin: 0..1 = one turn; the visible width follows |cos|. */
    const int radius = 10;
    float c = cosf(spin * 6.2831853f), xs = fabsf(c);
    int showHeads = c >= 0 ? heads : !heads, row;
    if (xs < 0.12f)
        xs = 0.12f;
    for (row = -radius; row <= radius; row++)
    {
        float half = sqrtf((float)(radius * radius - row * row)) * xs;
        int hw = (int)(half + 0.5f), y = cy + row;
        if (hw <= 0)
            continue;
        G_FillA(cx - hw, y, hw * 2, 1, 0x7a4c08, alpha);
        if (hw > 1)
        {
            float inner = sqrtf((float)((radius - 1) * (radius - 1) - row * row)) * xs;
            int iw = (int)(inner + 0.5f);
            if (abs(row) < radius)
            {
                unsigned fill = row < -4 ? 0xf8e070 : row > 4 ? 0xd09820 : 0xf0c038;
                if (showHeads && abs(row) <= 5)
                {
                    float ball = sqrtf((float)(25 - row * row)) * xs;
                    int bw = (int)(ball + 0.5f);
                    G_FillA(cx - iw, y, iw * 2, 1, fill, alpha);
                    if (bw > 0)
                        G_FillA(cx - bw, y, bw * 2, 1, row < 0 ? 0xe83828 : row == 0 ? 0x302020 : 0xf8f8f8, alpha);
                    if (row == 0 && bw > 1)
                        G_FillA(cx - 1, y, 2, 1, 0xf8f8f8, alpha);
                }
                else
                {
                    G_FillA(cx - iw, y, iw * 2, 1, fill, alpha);
                    if (!showHeads && abs(row) <= 3)
                    {
                        float star = (float)(3 - abs(row)) * xs + 1;
                        G_FillA(cx - (int)star, y, (int)star * 2, 1, 0xb07818, alpha);
                    }
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Controller buttons: the icons the rest of the game shows (mp_buttons.c), */
/* for the pad used last, or the GBA keys for a keyboard.                   */
/* ------------------------------------------------------------------ */
u8 MP_ButtonIcon_ToPixels(u8 id, u8 *buf, int stride, int x, int y, const u16 *pal, u8 bodyIdx, u8 contrastIdx);
int Gamepad_IconFamily(void);
int Gamepad_EffectiveNintendoLayout(void);
/* mp_buttons_data.h color classes: passing them as the palette keeps each class. */
static const u16 sButtonClass[16] = {0, 0x0000, 0x5EF7, 0x14DF, 0x7DE0, 0x7F2F, 0x7DFF, 0x73A0, 0x2F8D, 0x27FF};
static const u8 sKeypadTiles[] = INCGFX_U8("graphics/fonts/keypad_icons.png", ".4bpp");
static const struct
{
    u8 tile, width;
} sKeypad[] = {
    [CHAR_A_BUTTON] = {0x00, 8},      [CHAR_B_BUTTON] = {0x01, 8},       [CHAR_L_BUTTON] = {0x02, 16},
    [CHAR_R_BUTTON] = {0x04, 16},     [CHAR_START_BUTTON] = {0x06, 24},  [CHAR_SELECT_BUTTON] = {0x09, 24},
    [CHAR_DPAD_UP] = {0x0C, 8},       [CHAR_DPAD_DOWN] = {0x0D, 8},      [CHAR_DPAD_LEFT] = {0x0E, 8},
    [CHAR_DPAD_RIGHT] = {0x0F, 8},    [CHAR_DPAD_UPDOWN] = {0x20, 8},    [CHAR_DPAD_LEFTRIGHT] = {0x21, 8},
    [CHAR_DPAD_NONE] = {0x22, 8},
};
struct ButtonTex
{
    int key, w, h, top;
    SDL_Texture *texture;
};
static struct ButtonTex sButtons[24];
/* {INSPECT}: the card games' look-closer button. A controller's right trigger (RT / R2 / ZR); on the keyboard, D. */
#define G_BUTTON_INSPECT 0xFE
/* {SEARCH}: SEARCH & SORT in the card lists. A controller's left trigger (LT / L2 / ZL); on the keyboard, F. */
#define G_BUTTON_SEARCH 0xFD
static const u8 sKeyD[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E};   /* 5x7 "D", top row first, bit 4 = left */
static const u8 sKeyF[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};   /* 5x7 "F" */
static int ButtonKey(unsigned id, int light)
{
    return (int)id | Gamepad_IconFamily() << 8 | Gamepad_EffectiveNintendoLayout() << 12 | light << 13;
}
/* light: a pale button for dark backgrounds (its marks turn dark). */
static struct ButtonTex *Button(unsigned id, int light)
{
    int key = ButtonKey(id, light), i, x, y, w, top = 99, bottom = -1;
    unsigned body = light ? 0xe0e0e4 : 0x303034, mark = light ? 0x303034 : G_Rgb15(sButtonClass[2]);
    struct ButtonTex *t = NULL;
    u8 buf[16 * 32];
    struct GSurface s;
    for (i = 0; i < 24; i++)
        if (sButtons[i].texture && sButtons[i].key == key)
            return &sButtons[i];
    for (i = 0; i < 24 && !t; i++)
        if (!sButtons[i].texture)
            t = &sButtons[i];
    if (!t)
    {
        /* The pad changed: start over. */
        for (i = 0; i < 24; i++)
            if (sButtons[i].texture)
                SDL_DestroyTexture(sButtons[i].texture);
        memset(sButtons, 0, sizeof(sButtons));
        t = &sButtons[0];
    }
    memset(buf, 0, sizeof(buf));
    s = G_SurfaceNew(32, 16);
    if (!s.px)
        return NULL;
    if ((id == G_BUTTON_INSPECT || id == G_BUTTON_SEARCH) && Gamepad_IconFamily() == 0 /* GAMEPAD_ICONS_KEYBOARD */)
    {
        /* a key cap with a D on it, the size of the GBA's A / B buttons' neighbours */
        w = 11;
        for (y = 0; y < 11; y++)
            for (x = 0; x < w; x++)
            {
                int corner = (x == 0 || x == w - 1) && (y == 0 || y == 10);
                int edge = x == 0 || x == w - 1 || y == 0 || y == 10;
                const u8 *glyph = id == G_BUTTON_SEARCH ? sKeyF : sKeyD;
                int letter = x >= 3 && x < 8 && y >= 2 && y < 9 && (glyph[y - 2] >> (7 - x) & 1);
                if (corner)
                    continue;
                S_Px(&s, x, y + 1, ARGB(letter ? mark : edge ? (light ? 0x9090a0 : 0x505058) : body, 255));
            }
        top = 1;
        bottom = 11;
    }
    else if ((w = MP_ButtonIcon_ToPixels((u8)(id == G_BUTTON_INSPECT ? CHAR_MP_RT : id == G_BUTTON_SEARCH ? CHAR_MP_LT : id), buf, 32, 0, 0, sButtonClass, 1, 2)) != 0)
    {
        for (y = 0; y < 16; y++)
            for (x = 0; x < 32; x++)
            {
                u8 v = buf[y * 32 + x];
                if (!v)
                    continue;
                S_Px(&s, x, y, ARGB(v == 1 ? body : v == 2 ? mark : G_Rgb15(sButtonClass[v]), 255));
                if (y < top)
                    top = y;
                if (y > bottom)
                    bottom = y;
            }
    }
    else if (id < sizeof(sKeypad) / sizeof(sKeypad[0]))
    {
        /* GBA keypad icons: 4bpp tiles in rows of sixteen, 12 pixels tall. */
        uint32_t colors[] = {0, 0xf8f8f8, 0x505058, 0xd0d0c8, 0x303034};
        if (light)
            colors[1] = 0x303034, colors[2] = 0xe0e0e4, colors[3] = 0x9090a0;
        w = sKeypad[id].width;
        for (y = 0; y < 12; y++)
            for (x = 0; x < w; x++)
            {
                unsigned tile = sKeypad[id].tile + (y / 8) * 16 + x / 8;
                unsigned byte = sKeypadTiles[tile * 32 + (y % 8) * 4 + (x % 8) / 2];
                unsigned v = (x & 1) ? byte >> 4 : byte & 15;
                if (!v || v > 4)
                    continue;
                S_Px(&s, x, y, ARGB(colors[v], 255));
                if (y < top)
                    top = y;
                if (y > bottom)
                    bottom = y;
            }
    }
    if (bottom < 0)
        top = bottom = 0;
    t->texture = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
    t->key = key;
    t->w = w;
    t->top = top;
    t->h = bottom - top + 1;
    return t->texture ? t : NULL;
}
static int ButtonIcon(int x, int y, unsigned id, int light)
{
    struct ButtonTex *t = Button(id, light);
    SDL_Rect src, dst;
    if (!t)
        return 0;
    src.x = 0;
    src.y = t->top;
    src.w = t->w;
    src.h = t->h;
    dst.x = x;
    dst.y = y - t->h / 2;
    dst.w = t->w;
    dst.h = t->h;
    SDL_RenderCopy(sR, t->texture, &src, &dst);
    return t->w;
}
int G_ButtonIcon(int x, int y, unsigned id) { return ButtonIcon(x, y, id, 0); }
int G_ButtonIconWidth(unsigned id)
{
    struct ButtonTex *t = Button(id, 0);
    return t ? t->w : 0;
}
static int HintToken(const char *s, unsigned *id)
{
    static const struct
    {
        const char *name;
        unsigned id;
    } tokens[] = {{"{A}", CHAR_A_BUTTON},           {"{B}", CHAR_B_BUTTON},
                  {"{L}", CHAR_L_BUTTON},           {"{R}", CHAR_R_BUTTON},
                  {"{START}", CHAR_START_BUTTON},   {"{SELECT}", CHAR_SELECT_BUTTON},
                  {"{UP}", CHAR_DPAD_UP},           {"{DOWN}", CHAR_DPAD_DOWN},
                  {"{LEFT}", CHAR_DPAD_LEFT},       {"{RIGHT}", CHAR_DPAD_RIGHT},
                  {"{UPDOWN}", CHAR_DPAD_UPDOWN},   {"{LEFTRIGHT}", CHAR_DPAD_LEFTRIGHT},
                  {"{DPAD}", CHAR_DPAD_NONE},       {"{INSPECT}", G_BUTTON_INSPECT},
                  {"{SEARCH}", G_BUTTON_SEARCH}};
    unsigned i;
    for (i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++)
        if (!strncmp(s, tokens[i].name, strlen(tokens[i].name)))
        {
            *id = tokens[i].id;
            return (int)strlen(tokens[i].name);
        }
    return 0;
}
/* One line with {A}-style buttons drawn as the controller's icons. */
int G_Hint(int x, int y, unsigned color, unsigned shadow, const char *text, int font, int draw)
{
    char word[96];
    int start = x, n = 0;
    while (*text)
    {
        unsigned id;
        int len = *text == '{' ? HintToken(text, &id) : 0;
        if (len || n == (int)sizeof(word) - 1)
        {
            word[n] = 0;
            if (draw && n)
                G_Text(x, y, color, shadow, word, 0, font);
            x += G_Measure(word, font);
            n = 0;
        }
        if (len)
        {
            int light = ((color >> 16 & 255) + (color >> 8 & 255) + (color & 255)) > 3 * 128;
            x += draw ? ButtonIcon(x, y + (font == TF_NORMAL ? 8 : 6), id, light) : G_ButtonIconWidth(id);
            x += 1;
            text += len;
            continue;
        }
        word[n++] = *text++;
    }
    word[n] = 0;
    if (draw && n)
        G_Text(x, y, color, shadow, word, 0, font);
    x += G_Measure(word, font);
    return x - start;
}

/* ------------------------------------------------------------------ */
/* Card art                                                             */
/* ------------------------------------------------------------------ */
struct Source
{
    unsigned id, kind, age;
    int w, h;
    uint32_t *px;
    int artX, artY, artW, artH; /* where the illustration is (the whole picture for the Game Boy's) */
};
#define SOURCES 48
static struct Source sSource[SOURCES];
static unsigned sSourceAge;
static unsigned sArtMode; /* G_ART_* */
static uint32_t *ToArgb(unsigned char *pixels, int *w, int *h)
{
    int i;
    uint32_t *out;
    if (!pixels)
        return NULL;
    out = malloc((size_t)*w * *h * 4);
    if (out)
        for (i = 0; i < *w * *h; i++)
            out[i] = (uint32_t)pixels[i * 4 + 3] << 24 | pixels[i * 4] << 16 | pixels[i * 4 + 1] << 8 | pixels[i * 4 + 2];
    stbi_image_free(pixels);
    return out;
}
static unsigned U32(const unsigned char *b) { return b[0] | b[1] << 8 | b[2] << 16 | (unsigned)b[3] << 24; }
/* A card's entry in a built-in archive: graphics/tcg/original.bin ("TCGA": offset, size) or
 * graphics/tcg/printed.bin ("TCGP": offset, size, then the illustration's x, y, w, h as u16;
 * then, for id 0, the card back: hasBack). */
static const unsigned char *Entry(const unsigned char *start, const unsigned char *end, const char *magic,
                                  unsigned stride, int hasBack, unsigned id, unsigned *offset, unsigned *size)
{
    unsigned total = (unsigned)(end - start), count, index;
    const unsigned char *b;
    if (total < 8 || memcmp(start, magic, 4))
        return NULL;
    count = U32(start + 4);
    index = id ? id - 1 : count;
    if (id > count || (!id && !hasBack) || 8 + stride * (index + 1) > total)
        return NULL;
    b = start + 8 + stride * index;
    *offset = U32(b);
    *size = U32(b + 4);
    if (!*size || *offset > total || *size > total - *offset)
        return NULL;
    return b;
}
static const unsigned char *PrintedEntry(unsigned id, unsigned *offset, unsigned *size)
{
    return Entry(gTcgPrintedArt, gTcgPrintedArtEnd, "TCGP", 16, 1, id, offset, size);
}
/* Decodes the Game Boy illustration (kind 0) or the printed card (kind 1; id 0 = the back). */
static int LoadSource(struct Source *s, unsigned id, unsigned kind)
{
    const unsigned char *start = kind ? gTcgPrintedArt : gTcgOriginalArt, *b;
    unsigned offset, size;
    int channels;
    b = kind ? PrintedEntry(id, &offset, &size)
             : Entry(gTcgOriginalArt, gTcgOriginalArtEnd, "TCGA", 8, 0, id, &offset, &size);
    if (!b)
        return 0;
    s->px = ToArgb(stbi_load_from_memory(start + offset, (int)size, &s->w, &s->h, &channels, 4), &s->w, &s->h);
    if (!s->px)
        return 0;
    s->artX = 0, s->artY = 0, s->artW = s->w, s->artH = s->h;
    if (kind)
    {
        s->artX = b[8] | b[9] << 8;
        s->artY = b[10] | b[11] << 8;
        s->artW = b[12] | b[13] << 8;
        s->artH = b[14] | b[15] << 8;
        if (!s->artW || !s->artH || s->artX + s->artW > s->w || s->artY + s->artH > s->h)
            s->artX = 0, s->artY = 0, s->artW = s->w, s->artH = s->h;
    }
    return 1;
}
/* kind: 0 = Game Boy illustration, 1 = printed card. */
static struct Source *Source(unsigned id, unsigned kind)
{
    unsigned i, best = 0;
    struct Source *s;
    for (i = 0; i < SOURCES; i++)
        if (sSource[i].px && sSource[i].id == id && sSource[i].kind == kind)
        {
            sSource[i].age = ++sSourceAge;
            return &sSource[i];
        }
    for (i = 0; i < SOURCES; i++)
        if (!sSource[i].px || sSource[i].age < sSource[best].age)
        {
            best = i;
            if (!sSource[i].px)
                break;
        }
    s = &sSource[best];
    free(s->px);
    memset(s, 0, sizeof(*s));
    if (!LoadSource(s, id, kind))
    {
        free(s->px);
        memset(s, 0, sizeof(*s));
        return NULL;
    }
    s->id = id;
    s->kind = kind;
    s->age = ++sSourceAge;
    return s;
}
static void ForgetFaces(void);
/* OPTIONS > DISPLAY > CARD ART. */
void G_SetArtMode(unsigned mode)
{
    if (mode >= G_ART_COUNT)
        mode = G_ART_PRINTED_ART;
    if (mode == sArtMode)
        return;
    sArtMode = mode;
    ForgetFaces();
}
static unsigned sBackMode; /* G_BACK_* */
void G_SetBackMode(unsigned mode)
{
    if (mode >= G_BACK_COUNT)
        mode = G_BACK_PRINTED;
    if (mode == sBackMode)
        return;
    sBackMode = mode;
    ForgetFaces();
}
/* The printed card, unless CARD ART is GAME BOY or the card was only ever on the Game Boy. */
static struct Source *Scan(unsigned id)
{
    if (sArtMode == G_ART_GAME_BOY || !G_HasPrinted(id))
        return NULL;
    return Source(id, 1);
}
/* Where a card's illustration is on its face of w x h (as G_CardTexture draws it), for the foil over it */
void G_CardArtRect(unsigned id, int w, int h, int *ax, int *ay, int *aw, int *ah)
{
    const struct TcgCard *c = id ? Tcg_Card(id) : NULL;
    struct Source *scan = c && sArtMode == G_ART_FULL_CARD ? Scan(id) : NULL;
    int b = w >= 40 ? 3 : w >= 26 ? 2 : 1;
    if (scan && scan->w > 0 && scan->h > 0)
    {
        *ax = scan->artX * w / scan->w;
        *ay = scan->artY * h / scan->h;
        *aw = scan->artW * w / scan->w;
        *ah = scan->artH * h / scan->h;
        return;
    }
    *ax = b + 1;
    *aw = w - 2 * b - 2;
    if (c && c->kind == TCG_ENERGY)
    {
        *ah = *aw * 3 / 4;
        *ay = (h - *ah) / 2;
        return;
    }
    *ah = c && c->kind == TCG_TRAINER && sArtMode == G_ART_PRINTED_ART && Scan(id) ? (*aw * 5 + 4) / 9 : (*aw * 3 + 2) / 4;
    *ay = b + (w >= 40 ? 6 : w >= 26 ? 3 : 2) + 1;
}
int G_HasPrinted(unsigned id)
{
    unsigned offset, size;
    return PrintedEntry(id, &offset, &size) != NULL;
}
/* Area-averaging resample of a source rectangle (premultiplied by alpha). */
static void Resample(const struct Source *src, float sx, float sy, float sw, float sh, struct GSurface *dst, int dx,
                     int dy, int dw, int dh)
{
    int x, y;
    for (y = 0; y < dh; y++)
        for (x = 0; x < dw; x++)
        {
            float x0 = sx + sw * x / dw, x1 = sx + sw * (x + 1) / dw, y0 = sy + sh * y / dh,
                  y1 = sy + sh * (y + 1) / dh;
            float acc[4] = {0}, total = 0;
            int ix, iy;
            for (iy = (int)y0; iy < (int)ceilf(y1) && iy < src->h; iy++)
                for (ix = (int)x0; ix < (int)ceilf(x1) && ix < src->w; ix++)
                {
                    float wx = G_MinF(x1, ix + 1.0f) - G_MaxF(x0, (float)ix), wy = G_MinF(y1, iy + 1.0f) - G_MaxF(y0, (float)iy);
                    float wgt = wx * wy, a;
                    uint32_t p;
                    if (wgt <= 0 || ix < 0 || iy < 0)
                        continue;
                    p = src->px[iy * src->w + ix];
                    a = (float)(p >> 24) / 255.0f;
                    acc[0] += wgt * a * ((p >> 16) & 255);
                    acc[1] += wgt * a * ((p >> 8) & 255);
                    acc[2] += wgt * a * (p & 255);
                    acc[3] += wgt * a;
                    total += wgt;
                }
            if (total > 0 && acc[3] > 0)
            {
                unsigned r = (unsigned)(acc[0] / acc[3] + 0.5f), g = (unsigned)(acc[1] / acc[3] + 0.5f),
                         b = (unsigned)(acc[2] / acc[3] + 0.5f), a = (unsigned)(acc[3] / total * 255 + 0.5f);
                S_Px(dst, dx + x, dy + y, a << 24 | r << 16 | g << 8 | b);
            }
        }
}

/* A card's illustration into a box, cut to the box's shape around its middle. */
static void ArtInto(const struct Source *art, struct GSurface *s, int x, int y, int w, int h)
{
    float sx = (float)art->artX, sy = (float)art->artY, sw = (float)art->artW, sh = (float)art->artH;
    if (sw * h > sh * w)
    {
        float cut = sw - sh * w / h;
        sx += cut / 2;
        sw -= cut;
    }
    else
    {
        float cut = sh - sw * h / w;
        sy += cut / 2;
        sh -= cut;
    }
    Resample(art, sx, sy, sw, sh, s, x, y, w, h);
}
static void Back(struct GSurface *s)
{
    int w = s->w, h = s->h, x, y;
    float cx = w / 2.0f, cy = h / 2.0f, r = w * 0.26f;
    S_RoundFill(s, 0, 0, w, h, w >= 20 ? 2 : 1, ARGB(0x183070, 255));
    S_RoundFill(s, 1, 1, w - 2, h - 2, w >= 20 ? 2 : 1, ARGB(0x2858b8, 255));
    /* A soft swirl behind the emblem. */
    for (y = 2; y < h - 2; y++)
        for (x = 2; x < w - 2; x++)
        {
            float dx = (x + 0.5f - cx) / w, dy = (y + 0.5f - cy) / h, d = sqrtf(dx * dx + dy * dy);
            float swirl = sinf(d * 22.0f - atan2f(dy, dx) * 2.0f);
            if (swirl > 0.55f)
                S_Blend(s, x, y, 0x4880e0, 120);
        }
    if (w >= 10)
    {
        S_Disc(s, cx, cy, r + 1, ARGB(0x101010, 255));
        S_Disc(s, cx, cy, r, ARGB(0xf8f8f8, 255));
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
            {
                float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
                if (dx * dx + dy * dy <= r * r && dy < -0.5f)
                    S_Px(s, x, y, ARGB(dy < -r * 0.6f ? 0xf05848 : 0xd83020, 255));
            }
        S_Fill(s, (int)(cx - r), (int)cy - (w >= 20 ? 1 : 0), (int)(r * 2 + 1), w >= 20 ? 2 : 1, ARGB(0x101010, 255));
        S_Disc(s, cx, cy, r * 0.38f + 0.6f, ARGB(0x101010, 255));
        S_Disc(s, cx, cy, r * 0.38f - 0.4f, ARGB(0xf8f8f8, 255));
    }
    S_RoundFrame(s, 0, 0, w, h, w >= 20 ? 2 : 1, ARGB(0x0c1838, 255));
}
/* The type-colored Game Boy frame, around the printed illustration (CARD ART: PRINTED ART) or the
 * Game Boy picture. */
static void Face(struct GSurface *s, const struct TcgCard *c)
{
    int w = s->w, h = s->h, b = w >= 40 ? 3 : w >= 26 ? 2 : 1, i, artW, artH, artX, artY, textY;
    unsigned type = c->kind == TCG_TRAINER ? 7 : c->type, base = G_TypeColor(type, 0), light = G_TypeColor(type, 1),
             dark = G_TypeColor(type, 2);
    struct Source *printed = sArtMode == G_ART_PRINTED_ART ? Scan(c->id) : NULL;
    struct Source *art = printed ? printed : Source(c->id, 0);
    int radius = w >= 26 ? 2 : 1;
    /* Yellow border, as printed. */
    S_RoundFill(s, 0, 0, w, h, radius, ARGB(0xf0c838, 255));
    S_Fill(s, 1, 1, w - 2, 1, ARGB(0xf8e888, 255));
    S_RoundFrame(s, 0, 0, w, h, radius, ARGB(0x403010, 255));
    /* Inner panel with a gentle vertical gradient. */
    for (i = b; i < h - b; i++)
    {
        float t = (float)(i - b) / (h - 2 * b);
        S_Fill(s, b, i, w - 2 * b, 1, ARGB(G_Mix(light, base, t * 1.3f), 255));
    }
    if (c->kind == TCG_ENERGY)
    {
        /* The energy illustration is the symbol: let it fill the card. */
        artW = w - 2 * b - 2;
        artH = artW * 3 / 4;
        artX = b + 1;
        artY = (h - artH) / 2;
        if (art)
            ArtInto(art, s, artX, artY, artW, artH);
        S_Frame(s, artX - 1, artY - 1, artW + 2, artH + 2, ARGB(dark, 255));
        S_Fill(s, b + 1, h - b - 3, w - 2 * b - 2, 1, ARGB(dark, 160));
        return;
    }
    /* Name strip (Trainer: a dark TRAINER band). */
    textY = b + (w >= 40 ? 6 : w >= 26 ? 3 : 2);
    if (c->kind == TCG_TRAINER)
        S_Fill(s, b, b, w - 2 * b, textY - b, ARGB(0x404858, 255));
    else
    {
        S_Fill(s, b + 1, b + 1, (w - 2 * b) / 2, 1, ARGB(dark, 200));
        S_Fill(s, w - b - 1 - (w >= 26 ? 5 : 3), b + 1, w >= 26 ? 4 : 2, 1, ARGB(0xd02020, 220));
    }
    artW = w - 2 * b - 2;
    /* The Game Boy pictures are 4:3. A printed Trainer's is much wider (about 1.85:1):
     * its box is too, so the picture keeps its sides. */
    artH = printed && c->kind == TCG_TRAINER ? (artW * 5 + 4) / 9 : (artW * 3 + 2) / 4;
    artX = b + 1;
    artY = textY + 1;
    S_Fill(s, artX - 1, artY - 1, artW + 2, artH + 2, ARGB(c->kind == TCG_TRAINER ? 0x606878 : 0xc0b070, 255));
    if (w >= 26)
        S_Frame(s, artX - 1, artY - 1, artW + 2, artH + 2, ARGB(0x585040, 255));
    if (art)
        ArtInto(art, s, artX, artY, artW, artH);
    /* Text box: cost dots and lines that read as attack text from afar. */
    textY = artY + artH + 2;
    if (textY < h - b - 2)
    {
        int lines = (h - b - 1 - textY) / (w >= 40 ? 6 : w >= 26 ? 5 : 4), l;
        S_Fill(s, b + 1, textY - 1, w - 2 * b - 2, h - b - textY, ARGB(G_Mix(light, 0xffffff, 0.45f), 255));
        for (l = 0; l < lines && l < 3; l++)
        {
            int y = textY + l * (w >= 40 ? 6 : w >= 26 ? 5 : 4) + 1, x = b + 2, k, cost = 0;
            if (c->kind == TCG_POKEMON && l < 2 && c->attacks[l].name[0])
            {
                for (k = 0; k < TCG_TYPES; k++)
                    cost += c->attacks[l].cost[k];
                for (k = 0; k < TCG_TYPES && cost; k++)
                {
                    unsigned n = c->attacks[l].cost[k];
                    while (n-- && x < w / 2)
                    {
                        if (w >= 26)
                            S_Fill(s, x, y, 2, 2, ARGB(G_TypeColor(k, 2), 255));
                        else
                            S_Px(s, x, y, ARGB(G_TypeColor(k, 2), 255));
                        x += w >= 26 ? 3 : 2;
                    }
                }
                x += 1;
            }
            else if (c->kind == TCG_POKEMON)
                break;
            if (x < w - b - 3)
                S_Fill(s, x, y, w - b - 2 - x - (c->kind == TCG_POKEMON ? 3 : 0), 1, ARGB(0x707070, 200));
        }
        /* The bottom row as printed, in the attacks' dots: weakness (left), resistance (middle), retreat cost
         * (right, a dot per Energy); a dash where there's none. Readable without zooming in. */
        if (c->kind == TCG_POKEMON && w >= 18)
        {
            int big = w >= 26, dot = w >= 40 ? 3 : big ? 2 : 1, gap = dot + 1, y = h - b - (w >= 40 ? 5 : big ? 3 : 2), k;
            int slots[3] = {b + 2, w / 2 - dot, 0};
            unsigned types[2] = {c->weakness, c->resistance};
            S_Fill(s, b + 1, y - (big ? 2 : 1), w - 2 * b - 2, 1, ARGB(dark, 90));
            for (k = 0; k < 2; k++)
            {
                if (types[k] < TCG_TYPES)
                    S_Fill(s, slots[k], y, dot, dot, ARGB(G_TypeColor(types[k], 2), 255));
                else
                    S_Fill(s, slots[k], y + dot / 2, big ? 3 : 2, 1, ARGB(0x707070, 160));
            }
            if (!c->retreat)
                S_Fill(s, w - b - 2 - (big ? 3 : 2), y + dot / 2, big ? 3 : 2, 1, ARGB(0x707070, 160));
            for (k = 0; k < c->retreat && k < 4; k++)
                S_Fill(s, w - b - 2 - dot - k * gap, y, dot, dot, ARGB(G_TypeColor(TCG_COLORLESS, 2), 255));
        }
    }
    (void)i;
}
static void ScanFace(struct GSurface *s, const struct Source *scan)
{
    int w = s->w, h = s->h;
    Resample(scan, 0, 0, (float)scan->w, (float)scan->h, s, 0, 0, w, h);
    /* Printed cards have round corners: clear them. */
    if (w >= 12)
    {
        S_Px(s, 0, 0, 0);
        S_Px(s, w - 1, 0, 0);
        S_Px(s, 0, h - 1, 0);
        S_Px(s, w - 1, h - 1, 0);
    }
}

struct CardTex
{
    unsigned id, age;
    int w, h;
    SDL_Texture *texture;
};
#define CARD_TEXTURES 192
static struct CardTex sCards[CARD_TEXTURES];
static unsigned sCardAge;
SDL_Texture *G_CardTexture(unsigned id, int w, int h)
{
    unsigned i, best = 0;
    struct CardTex *t;
    struct GSurface s;
    const struct TcgCard *c = id ? Tcg_Card(id) : NULL;
    if (!sR || w <= 0 || h <= 0)
        return NULL;
    if (id && !c)
        id = 0;
    for (i = 0; i < CARD_TEXTURES; i++)
        if (sCards[i].texture && sCards[i].id == id && sCards[i].w == w && sCards[i].h == h)
        {
            sCards[i].age = ++sCardAge;
            return sCards[i].texture;
        }
    for (i = 0; i < CARD_TEXTURES; i++)
        if (!sCards[i].texture || sCards[i].age < sCards[best].age)
        {
            best = i;
            if (!sCards[i].texture)
                break;
        }
    t = &sCards[best];
    if (t->texture)
        SDL_DestroyTexture(t->texture);
    memset(t, 0, sizeof(*t));
    s = G_SurfaceNew(w, h);
    if (!s.px)
        return NULL;
    if (!id)
    {
        struct Source *back = sBackMode == G_BACK_PRINTED ? Source(0, 1) : NULL;
        if (back)
            ScanFace(&s, back);
        else
            Back(&s);
    }
    else
    {
        struct Source *scan = sArtMode == G_ART_FULL_CARD ? Scan(id) : NULL;
        if (scan)
            ScanFace(&s, scan);
        else
            Face(&s, c);
    }
    t->texture = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
    if (!t->texture)
        return NULL;
    if (w < 40)
        SDL_SetTextureScaleMode(t->texture, SDL_ScaleModeLinear);
    t->id = id;
    t->w = w;
    t->h = h;
    t->age = ++sCardAge;
    return t->texture;
}
/* Sizes cards are built at; anything in between scales the next size up. */
static const int sSizes[][2] = {{12, 16}, {18, 25}, {22, 30}, {30, 42}, {44, 61}, {72, 100}};
void G_Card(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
{
    SDL_Texture *t;
    SDL_FRect dst = {x, y, w, h};
    unsigned i, pick = 5;
    if (w < 1 || h < 1 || !alpha)
        return;
    for (i = 0; i < 6; i++)
        if ((float)sSizes[i][0] >= w - 0.5f)
        {
            pick = i;
            break;
        }
    t = G_CardTexture(id, sSizes[pick][0], sSizes[pick][1]);
    if (!t)
        return;
    if ((int)w == sSizes[pick][0] && (int)h == sSizes[pick][1] && angle == 0)
    {
        dst.x = (float)(int)(x + 0.5f);
        dst.y = (float)(int)(y + 0.5f);
    }
    SDL_SetTextureAlphaMod(t, alpha > 255 ? 255 : alpha);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyExF(sR, t, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
    if (flash)
    {
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(t, flash > 255 ? 255 : flash);
        SDL_RenderCopyExF(sR, t, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    }
    SDL_SetTextureAlphaMod(t, 255);
}
static struct
{
    unsigned id;
    int w, h, scan;
    SDL_Texture *texture;
} sArt;
/* The big picture: the whole printed card (FULL CARD), its illustration (PRINTED ART) or the
 * Game Boy picture. */
SDL_Texture *G_ArtTexture(unsigned id, int *w, int *h, int *scan)
{
    struct Source *s;
    struct GSurface surface;
    if (sArt.texture && sArt.id == id)
        goto done;
    if (sArt.texture)
        SDL_DestroyTexture(sArt.texture);
    memset(&sArt, 0, sizeof(sArt));
    s = Scan(id);
    sArt.scan = s != NULL;
    if (!s)
        s = Source(id, 0);
    if (!s)
        return NULL;
    if (sArt.scan && sArtMode == G_ART_PRINTED_ART)
    {
        int y;
        surface = G_SurfaceNew(s->artW, s->artH);
        if (!surface.px)
            return NULL;
        for (y = 0; y < s->artH; y++)
            memcpy(surface.px + (size_t)y * s->artW, s->px + (size_t)(s->artY + y) * s->w + s->artX,
                   (size_t)s->artW * 4);
        sArt.texture = G_SurfaceTexture(&surface);
        G_SurfaceFree(&surface);
    }
    else
    {
        surface.w = s->w;
        surface.h = s->h;
        surface.px = s->px;
        sArt.texture = G_SurfaceTexture(&surface);
    }
    if (!sArt.texture)
        return NULL;
    if (sArt.scan)
        SDL_SetTextureScaleMode(sArt.texture, SDL_ScaleModeLinear);
    sArt.id = id;
    SDL_QueryTexture(sArt.texture, NULL, NULL, &sArt.w, &sArt.h);
done:
    if (w)
        *w = sArt.w;
    if (h)
        *h = sArt.h;
    if (scan)
        *scan = sArt.scan;
    return sArt.texture;
}

static void ForgetFaces(void)
{
    unsigned i;
    for (i = 0; i < CARD_TEXTURES; i++)
        if (sCards[i].texture)
            SDL_DestroyTexture(sCards[i].texture);
    memset(sCards, 0, sizeof(sCards));
    if (sArt.texture)
        SDL_DestroyTexture(sArt.texture);
    memset(&sArt, 0, sizeof(sArt));
}
void G_Begin(SDL_Renderer *r)
{
    if (sR && sR != r)
        G_Destroy();
    sR = r;
    LoadFonts();
}
void G_Destroy(void)
{
    unsigned i;
    for (i = 0; i < CARD_TEXTURES; i++)
        if (sCards[i].texture)
            SDL_DestroyTexture(sCards[i].texture);
    memset(sCards, 0, sizeof(sCards));
    for (i = 0; i < 3; i++)
    {
        if (sFontFace[i])
            SDL_DestroyTexture(sFontFace[i]);
        if (sFontShadow[i])
            SDL_DestroyTexture(sFontShadow[i]);
        sFontFace[i] = sFontShadow[i] = NULL;
    }
    if (sWindowFrame)
        SDL_DestroyTexture(sWindowFrame);
    sWindowFrame = NULL;
    sWindowFrameId = 0xffff;
    if (sArt.texture)
        SDL_DestroyTexture(sArt.texture);
    memset(&sArt, 0, sizeof(sArt));
    for (i = 0; i < 24; i++)
        if (sButtons[i].texture)
            SDL_DestroyTexture(sButtons[i].texture);
    memset(sButtons, 0, sizeof(sButtons));
    sFontReady = 0;
    sR = NULL;
}

/* The end of a duel, the same in every card game: the screen dims, YOU WIN! / YOU LOSE... on a band with "vs. NAME",
 * sparks for a win, then a window as tall as its text: how it ended (reason) and what it brought (result, in blue).
 * t: frames since the duel ended. */
static unsigned ResultHash(unsigned a)
{
    a ^= a >> 16;
    a *= 0x7feb352du;
    a ^= a >> 15;
    a *= 0x846ca68bu;
    return a ^ (a >> 16);
}
void G_ResultScreen(int win, int t, unsigned frame, const char *opponent, const char *reason, const char *result)
{
    float open = t / 20.0f;
    const char *title = win ? "YOU WIN!" : "YOU LOSE...";
    int k;
    (void)frame;
    open = open > 1 ? 1 : open < 0 ? 0 : open;
    open = 1 - (1 - open) * (1 - open);
    G_FillA(0, 0, 240, 160, 0x000000, (unsigned)(110 * open));
    /* sparks: a burst every few frames for the first seconds, each flying out and fading */
    for (k = 0; win && k < 27; k++)
    {
        int born = k * 3, age = t - born, n;
        unsigned h = ResultHash((unsigned)k * 2654435761u + 17u);
        int cx = 20 + (int)(h % 200), cy = 40 + (int)((h >> 8) % 40);
        unsigned color = (h >> 20) & 1 ? 0xf8e060 : 0x60c8f8;
        if (age < 0 || age >= 24)
            continue;
        for (n = 0; n < 3; n++)
        {
            float a = n * 2.0944f + (float)((h >> (4 + n)) % 100) / 200.0f, v = 1.5f * (0.6f + (float)((h >> (8 + n)) % 50) / 100.0f);
            float x = cx + cosf(a) * v * age, y = cy + (sinf(a) * v - 0.4f) * age + 0.05f * age * age;
            G_FillA((int)x, (int)y, 2, 2, n & 1 ? color : G_Mix(color, 0xffffff, 0.6f), (unsigned)(255 * (24 - age) / 24));
        }
    }
    {
        int band = (int)(34 * open);
        G_FillA(0, 48 - band / 2, 240, band, win ? 0x283868 : 0x402828, 230);
        G_FillA(0, 48 - band / 2, 240, 1, win ? 0xf8d850 : 0xa06060, 255);
        G_FillA(0, 48 + band / 2 - 1, 240, 1, win ? 0xf8d850 : 0xa06060, 255);
        if (open > 0.9f)
        {
            char vs[64];
            G_TextAlign(120, 36, TA_CENTER, win ? 0xf8e060 : 0xe8c8c8, 0x302810, title, TF_NORMAL);
            snprintf(vs, sizeof(vs), "vs. %s", opponent ? opponent : "");
            G_TextAlign(120, 51, TA_CENTER, 0xc8d0e0, G_NO_SHADOW, vs, TF_NARROW);
        }
    }
    if (t >= G_RESULT_BOX)
    {
        /* (as tall as its text: a card player's quote can run to three lines; under the banner, never off screen) */
        int lines = G_TextHeight(reason, 192, TF_SMALL), extra = result && result[0] ? G_TextHeight(result, 192, TF_SMALL) : 0;
        int h = 5 + lines + (extra ? 2 + extra : 0) + 12, y = 74;
        if (h < 56)
            h = 56;
        if (h > 160 - 66 - 2)
            h = 160 - 66 - 2;
        if (y + h > 158)
            y = 158 - h;   /* (moves up, at most to just under the banner) */
        G_Window(16, y, 208, h);
        if (t < G_RESULT_TEXT)
            return;   /* (the box waits a moment empty, then the quote and the prizes) */
        G_Text(24, y + 5, G_INK, G_INK_SHADOW, reason ? reason : "", 192, TF_SMALL);
        if (extra)
            G_Text(24, y + 5 + lines + 2, 0x3858a0, G_INK_SHADOW, result, 192, TF_SMALL);
        if (t >= G_RESULT_READY && (frame / 16) % 2)
            G_ButtonIcon(206, y + h - 8, 0);
    }
}
