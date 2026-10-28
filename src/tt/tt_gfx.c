// Moemon Kai: Triple Triad card faces. Final Fantasy XIV's pictures (graphics/tt/cards.bin, tools/tt/build_art.py)
// decoded when first drawn and kept a while: the face at the table's size and at full size, the list icon, the
// back, the plate behind the picture (tinted here once per color), the star and the type icons. The numbers sit in
// their diamond at the bottom as in XIV, in a pixel font of our own with a dark edge.
#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include "tt/tt.h"
#include "tt_gfx.h"
#include "../tcg/tcg_gfx.h"

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
void stbi_image_free(void *);
extern const unsigned char gTtCardArt[], gTtCardArtEnd[];

enum { PIC_BIG, PIC_BOARD, PIC_ICON };
enum { EX_BACK_BIG, EX_BACK_BOARD, EX_PLATE_BIG, EX_PLATE_BOARD, EX_STAR, EX_TYPES };

static uint32_t Rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

// The PNG for a card picture (kind PIC_*) or an extra (id 0, kind EX_*)
static const unsigned char *Blob(unsigned id, unsigned kind, unsigned *size)
{
    const unsigned char *b = gTtCardArt;
    unsigned total = (unsigned)(gTtCardArtEnd - gTtCardArt), top, extras, at, offset;
    if (total < 12 || memcmp(b, "TTCA", 4))
        return NULL;
    top = Rd32(b + 4);
    extras = Rd32(b + 8);
    if (id)
    {
        if (id > top || kind > PIC_ICON)
            return NULL;
        at = 12 + (id - 1) * 24 + kind * 8;
    }
    else
    {
        if (kind >= extras)
            return NULL;
        at = 12 + top * 24 + kind * 8;
    }
    offset = Rd32(b + at);
    *size = Rd32(b + at + 4);
    if (!*size || offset + *size > total)
        return NULL;
    return b + offset;
}

// A texture from a PNG; tint (0xRRGGBB, 0 none): the grey plate takes the color, keeping its light and shade
static SDL_Texture *Decode(const unsigned char *png, unsigned size, unsigned tint, int smooth, int *w, int *h)
{
    SDL_Renderer *r = G_R();
    SDL_Texture *t;
    unsigned char *rgba;
    int channels, i;
    if (!r || !png)
        return NULL;
    rgba = stbi_load_from_memory(png, (int)size, w, h, &channels, 4);
    if (!rgba)
    {
        printf("[tt] a card picture didn't decode (%u bytes)\n", size);
        return NULL;
    }
    if (tint)
        for (i = 0; i < *w * *h; i++)
        {
            unsigned char *p = rgba + i * 4;
            // the plate's grey around 80: that is the color; lighter and darker pixels lighten and darken it
            float l = (p[0] * 0.3f + p[1] * 0.59f + p[2] * 0.11f) / 80.0f;
            int k;
            for (k = 0; k < 3; k++)
            {
                float c = (float)((tint >> (16 - 8 * k)) & 255) * l;
                p[k] = (unsigned char)(c > 255 ? 255 : c);
            }
        }
    t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, *w, *h);
    if (t)
    {
        SDL_UpdateTexture(t, NULL, rgba, *w * 4);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(t, smooth ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
    }
    else
        printf("[tt] no texture for a %dx%d card picture: %s\n", *w, *h, SDL_GetError());
    stbi_image_free(rgba);
    return t;
}

// ---- the cache: card pictures by (id, kind), least recently used out
struct Pic
{
    unsigned id, kind, age;
    SDL_Texture *t;
};
#define PICS 96
static struct Pic sPics[PICS];
static unsigned sAge;
static SDL_Texture *sExtra[EX_TYPES + 4];
static SDL_Texture *sPlate[2][3];   // [big][seat]
static SDL_Texture *sGlyph[2];      // the numbers: fill (white) and edge
static SDL_Renderer *sOwner;

void Tt_ForgetTextures(void)
{
    unsigned i, k;
    for (i = 0; i < PICS; i++)
        if (sPics[i].t)
            SDL_DestroyTexture(sPics[i].t);
    memset(sPics, 0, sizeof(sPics));
    for (i = 0; i < sizeof(sExtra) / sizeof(sExtra[0]); i++)
        if (sExtra[i])
            SDL_DestroyTexture(sExtra[i]);
    memset(sExtra, 0, sizeof(sExtra));
    for (i = 0; i < 2; i++)
        for (k = 0; k < 3; k++)
            if (sPlate[i][k])
                SDL_DestroyTexture(sPlate[i][k]);
    memset(sPlate, 0, sizeof(sPlate));
    for (i = 0; i < 2; i++)
        if (sGlyph[i])
            SDL_DestroyTexture(sGlyph[i]);
    memset(sGlyph, 0, sizeof(sGlyph));
    sOwner = NULL;
}
static void Check(void)
{
    if (sOwner && sOwner != G_R())
        Tt_ForgetTextures();
    sOwner = G_R();
}

static SDL_Texture *Picture(unsigned id, unsigned kind)
{
    unsigned i, oldest = 0, size;
    int w, h;
    Check();
    for (i = 0; i < PICS; i++)
        if (sPics[i].t && sPics[i].id == id && sPics[i].kind == kind)
        {
            sPics[i].age = ++sAge;
            return sPics[i].t;
        }
    for (i = 0; i < PICS; i++)
    {
        if (!sPics[i].t)
        {
            oldest = i;
            break;
        }
        if (sPics[i].age < sPics[oldest].age)
            oldest = i;
    }
    if (sPics[oldest].t)
        SDL_DestroyTexture(sPics[oldest].t);
    {
        const unsigned char *png = Blob(id, kind, &size);   /* (before size is read) */
        sPics[oldest].t = png ? Decode(png, size, 0, kind != PIC_BOARD, &w, &h) : NULL;
    }
    sPics[oldest].id = id;
    sPics[oldest].kind = kind;
    sPics[oldest].age = ++sAge;
    return sPics[oldest].t;
}
static SDL_Texture *Extra(unsigned which)
{
    unsigned size;
    int w, h;
    Check();
    if (which >= sizeof(sExtra) / sizeof(sExtra[0]))
        return NULL;
    if (!sExtra[which])
    {
        const unsigned char *png = Blob(0, which, &size);
        sExtra[which] = png ? Decode(png, size, 0, 1, &w, &h) : NULL;
    }
    return sExtra[which];
}

unsigned Tt_SeatColor(unsigned seat, int shade)
{
    static const unsigned sColors[3][3] = {
        {0x3a78e8, 0x8cc0ff, 0x183c88},   // blue: the player (Final Fantasy XIV's own colors)
        {0xe0484c, 0xff9ca0, 0x801820},   // red: the opponent
        {0x808890, 0xc8ccd4, 0x404448},
    };
    return sColors[seat < 3 ? seat : 2][shade < 3 ? shade : 0];
}
static SDL_Texture *Plate(int big, unsigned seat)
{
    unsigned size;
    int w, h;
    Check();
    if (seat > 2)
        seat = 2;
    if (!sPlate[big][seat])
    {
        const unsigned char *png = Blob(0, big ? EX_PLATE_BIG : EX_PLATE_BOARD, &size);
        sPlate[big][seat] = png ? Decode(png, size, G_Mix(Tt_SeatColor(seat, 0), 0xffffff, 0.08f), 1, &w, &h) : NULL;
    }
    return sPlate[big][seat];
}

// ---- the lettering: a 5x7 pixel font of our own (the numbers 1-9 and A, and the banners' capitals), each glyph with
// a one-pixel dark edge, in a strip of 7x9 cells: the fill (white, colored when drawn) and the edge
static const char sChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!?.-+':/ ";
static const char *const sGlyphs[] = {
    ".###.#...##...#######...##...##...#", "####.#...##...#####.#...##...#####.", ".###.#...##....#....#....#...#.###.",
    "####.#...##...##...##...##...#####.", "######....#....####.#....#....#####", "######....#....####.#....#....#....",
    ".###.#...##....#.####...##...#.####", "#...##...##...#######...##...##...#", ".###...#....#....#....#....#...###.",
    "..###...#....#....#....#.#..#..##..", "#...##..#.#.#..##...#.#..#..#.#...#", "#....#....#....#....#....#....#####",
    "#...###.###.#.##.#.##...##...##...#", "#...##...###..##.#.##..###...##...#", ".###.#...##...##...##...##...#.###.",
    "####.#...##...#####.#....#....#....", ".###.#...##...##...##.#.##..#..##.#", "####.#...##...#####.#.#..#..#.#...#",
    ".#####....#.....###.....#....#####.", "#####..#....#....#....#....#....#..", "#...##...##...##...##...##...#.###.",
    "#...##...##...##...##...#.#.#...#..", "#...##...##...##.#.##.#.##.#.#.#.#.", "#...##...#.#.#...#...#.#.#...##...#",
    "#...##...#.#.#...#....#....#....#..", "#####....#...#...#...#...#....#####",
    ".###.#...##..###.#.###..##...#.###.", "..#...##....#....#....#....#...###.", ".###.#...#....#..##..#...#....#####",
    "####.....#....#.###.....#....#####.", "...#...##..#.#.#..#.#####...#....#.", "######....####.....#....##...#.###.",
    ".###.#....#....####.#...##...#.###.", "#####....#...#...#...#....#....#...", ".###.#...##...#.###.#...##...#.###.",
    ".###.#...##...#.####....#....#.###.",
    "..#....#....#....#....#.........#..", ".###.#...#....#...#...#.........#..", "................................#..",
    "...............#####...............", ".......#....#..#####..#....#.......", "..#....#...#.......................",
    ".......#..............#............", "....#...#...#...#...#...#..........", "...................................",
};
#define GLYPHS ((int)sizeof(sChars) - 1)
#define GW 7
#define GH 9
static void BuildGlyphs(void)
{
    static uint32_t px[2][GH][GW * GLYPHS];
    int g, x, y, k;
    SDL_Renderer *r = G_R();
    memset(px, 0, sizeof(px));
    for (g = 0; g < GLYPHS; g++)
        for (y = 0; y < 7; y++)
            for (x = 0; x < 5; x++)
            {
                int dx, dy;
                if (sGlyphs[g][y * 5 + x] != '#')
                    continue;
                px[0][y + 1][g * GW + x + 1] = 0xffffffffu;
                for (dy = -1; dy <= 1; dy++)
                    for (dx = -1; dx <= 1; dx++)
                        px[1][y + 1 + dy][g * GW + x + 1 + dx] = 0xff000000u;
            }
    for (k = 0; k < 2; k++)
    {
        sGlyph[k] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GW * GLYPHS, GH);
        if (!sGlyph[k])
            continue;
        SDL_UpdateTexture(sGlyph[k], NULL, px[k], GW * GLYPHS * 4);
        SDL_SetTextureBlendMode(sGlyph[k], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(sGlyph[k], SDL_ScaleModeNearest);
    }
}
// A character with its top-left corner at x, y; sx / sy the size of a pixel
void Tt_Char(float x, float y, char ch, float sx, float sy, unsigned fill, unsigned edge, unsigned alpha)
{
    const char *at;
    SDL_Rect src;
    SDL_FRect dst;
    Check();
    if (!sGlyph[0])
        BuildGlyphs();
    if (ch >= 'a' && ch <= 'z')
        ch = (char)(ch - 'a' + 'A');
    at = strchr(sChars, ch);
    if (!sGlyph[0] || !ch || !at || sx <= 0.05f || sy <= 0.05f || ch == ' ')
        return;
    src.x = (int)(at - sChars) * GW;
    src.y = 0;
    src.w = GW;
    src.h = GH;
    dst.x = x - sx;
    dst.y = y - sy;
    dst.w = GW * sx;
    dst.h = GH * sy;
    if (edge != G_NO_SHADOW)
    {
        SDL_SetTextureAlphaMod(sGlyph[1], (Uint8)(alpha * 3 / 4));
        SDL_SetTextureColorMod(sGlyph[1], (edge >> 16) & 255, (edge >> 8) & 255, edge & 255);
        SDL_RenderCopyF(G_R(), sGlyph[1], &src, &dst);
    }
    SDL_SetTextureAlphaMod(sGlyph[0], (Uint8)alpha);
    SDL_SetTextureColorMod(sGlyph[0], (fill >> 16) & 255, (fill >> 8) & 255, fill & 255);
    SDL_RenderCopyF(G_R(), sGlyph[0], &src, &dst);
}
float Tt_TextWidth(const char *text, float scale) { return (float)strlen(text) * 6 * scale - scale; }
void Tt_Text(float x, float y, const char *text, float scale, unsigned fill, unsigned edge, unsigned alpha)
{
    for (; *text; text++, x += 6 * scale)
        Tt_Char(x, y, *text, scale, scale, fill, edge, alpha);
}
static void Glyph(float x, float y, int value, float sx, float sy, unsigned fill, unsigned edge, unsigned alpha)
{
    if (value >= 1 && value <= 10)
        Tt_Char(x, y, value == 10 ? 'A' : (char)('0' + value), sx, sy, fill, edge, alpha);
}
void Tt_Number(int x, int y, int value, int scale, unsigned fill, unsigned alpha)
{
    Glyph((float)x + scale, (float)y + scale, value, (float)scale, (float)scale, fill, 0x101010, alpha);
}

static void Copy(SDL_Texture *t, float x, float y, float w, float h, unsigned alpha)
{
    SDL_FRect dst = {x, y, w, h};
    if (!t || w <= 0 || h <= 0)
        return;
    SDL_SetTextureAlphaMod(t, (Uint8)(alpha > 255 ? 255 : alpha));
    SDL_RenderCopyF(G_R(), t, NULL, &dst);
}

void Tt_DrawTypeIcon(unsigned type, float x, float y, float size, unsigned alpha)
{
    if (type && type < TT_TYPE_COUNT)
        Copy(Extra(EX_TYPES + type - 1), x, y, size, size, alpha);
}
void Tt_DrawStar(float x, float y, float size, unsigned alpha) { Copy(Extra(EX_STAR), x, y, size, size, alpha); }
void Tt_DrawStars(unsigned stars, int x, int y, int size, unsigned alpha)
{
    unsigned i;
    for (i = 0; i < stars; i++)
        Tt_DrawStar((float)(x + (int)i * (size - 2)), (float)y, (float)size, alpha);
}
void Tt_DrawIcon(unsigned id, float x, float y, float size, unsigned alpha, int dim)
{
    SDL_Texture *t = Picture(id, PIC_ICON);
    if (!t)
        return;
    SDL_SetTextureColorMod(t, dim ? 70 : 255, dim ? 70 : 255, dim ? 80 : 255);
    Copy(t, x, y, size, size, alpha);
    SDL_SetTextureColorMod(t, 255, 255, 255);
}

void Tt_DrawCard(unsigned id, float x, float y, float w, float h, float scaleX, float scaleY, unsigned alpha,
                 const struct TtFace *face)
{
    static const struct TtFace sPlain = {{0, 0, 0, 0}, TT_SEAT_NONE, 0, 0, 0};
    const struct TtCard *c = Tt_Card(id);
    int big = w > TT_BOARD_W + 8;
    float cx = x + w / 2, cy = y + h / 2, dw = w * scaleX, dh = h * scaleY, px, py, k;
    SDL_Texture *art;
    if (!face)
        face = &sPlain;
    if (dw < 0)
        dw = -dw;
    if (dw < 0.5f || dh < 0.5f || !alpha)
        return;
    px = cx - dw / 2;
    py = cy - dh / 2;
    if (face->back || !c)
    {
        art = Extra(big ? EX_BACK_BIG : EX_BACK_BOARD);
        Copy(art, px, py, dw, dh, alpha);
        if (face->glow && art)
        {
            SDL_SetTextureBlendMode(art, SDL_BLENDMODE_ADD);
            Copy(art, px, py, dw, dh, face->glow * alpha / 255);
            SDL_SetTextureBlendMode(art, SDL_BLENDMODE_BLEND);
        }
        return;
    }
    Copy(Plate(big, face->seat), px, py, dw, dh, alpha);
    art = Picture(id, big ? PIC_BIG : PIC_BOARD);
    Copy(art, px, py, dw, dh, alpha);
    if (face->glow && art)
    {
        SDL_SetTextureBlendMode(art, SDL_BLENDMODE_ADD);
        Copy(art, px, py, dw, dh, face->glow * alpha / 255);
        SDL_SetTextureBlendMode(art, SDL_BLENDMODE_BLEND);
    }
    // The type at the top right, the stars along the top left: a fraction of the card's size
    k = dw / w;   // (squashed: the details squash too)
    if (c->type)
        Tt_DrawTypeIcon(c->type, px + dw - (w * 0.30f) * k - w * 0.02f * k, py + h * 0.03f * (dh / h), w * 0.28f * k,
                        alpha);
    if (!face->noNumbers)
    {
        // the numbers in their diamond, centered near the bottom
        float s = w / TT_BOARD_W, gx = s * scaleX, gy = s * scaleY;
        float ncx = cx, nby = cy + dh / 2 - 3 * gy;
        float colW = 5 * gx, rowH = 7 * gy;
        int side;
        unsigned starSize = (unsigned)(w * 0.2f);
        if (c->stars && starSize >= 5 && k > 0.6f)
            for (side = 0; side < c->stars; side++)
                Tt_DrawStar(px + w * 0.04f * k + side * (starSize - 2) * k, py + h * 0.03f, starSize * k, alpha);
        for (side = 0; side < 4; side++)
        {
            int v = c->side[side] + face->mod[side];
            unsigned fill = face->mod[side] > 0 ? 0x90ff70 : face->mod[side] < 0 ? 0xff8070 : 0xffffff;
            float gxp, gyp;
            if (v < 1)
                v = 1;
            if (v > TT_RANK_A)
                v = TT_RANK_A;
            switch (side)
            {
            case TT_UP: gxp = ncx - colW / 2; gyp = nby - rowH * 2 - 2 * gy; break;
            case TT_DOWN: gxp = ncx - colW / 2; gyp = nby - rowH; break;
            case TT_LEFT: gxp = ncx - colW / 2 - colW - 1 * gx; gyp = nby - rowH * 1.5f - 1 * gy; break;
            default: gxp = ncx + colW / 2 + 1 * gx; gyp = nby - rowH * 1.5f - 1 * gy; break;
            }
            Glyph(gxp, gyp, v, gx, gy, fill, 0x101010, alpha);
        }
    }
}
