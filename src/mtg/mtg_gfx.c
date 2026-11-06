/* Moemon Kai: Magic card faces. The game's own frame (an original design, never the printed one) colored by the
 * card's color around the printed illustration, or the whole printed card (CARD ART: FULL CARD), built once into
 * textures at the sizes the screens use, like ygo_gfx.c. The printed cards come from graphics/mtg/printed.bin
 * (tools/mtg/build_printed.py), looked up by name. */
#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtg/mtg.h"
#include "../tcg/tcg_gfx.h"
#include "mtg_gfx.h"

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
void stbi_image_free(void *);
extern const unsigned char gMtgPrintedArt[], gMtgPrintedArtEnd[];

static unsigned sArtMode, sBackMode;

/* ---- the printed cards, by name */
static uint32_t Rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static unsigned Rd16(const unsigned char *p) { return p[0] | p[1] << 8; }
static unsigned Total(void) { return (unsigned)(gMtgPrintedArtEnd - gMtgPrintedArt); }
static int Valid(void) { return Total() >= 16 && !memcmp(gMtgPrintedArt, "MTGP", 4); }

static int16_t *sEntry;   /* per definition: its entry in printed.bin, -1 none */
static int sEntryCount;

static const unsigned char *EntryOf(int card)
{
    int n = Mtg_DbCount();
    if (!Valid() || card < 0 || card >= n)
        return NULL;
    if (!sEntry || sEntryCount != n)
    {
        unsigned count = Rd32(gMtgPrintedArt + 4), i;
        int k;
        SDL_free(sEntry);
        sEntry = SDL_malloc(sizeof(*sEntry) * (size_t)n);
        if (!sEntry)
            return NULL;
        sEntryCount = n;
        for (k = 0; k < n; k++)
            sEntry[k] = -1;
        if (16 + 24 * (size_t)count > Total())
            return NULL;
        for (k = 0; k < n; k++)
        {
            const char *name = Mtg_Card(k)->name;
            unsigned lo = 0, hi = count;
            while (lo < hi)
            {
                unsigned mid = (lo + hi) / 2, off = Rd32(gMtgPrintedArt + 16 + 24 * mid);
                int cmp = off < Total() ? strcmp((const char *)gMtgPrintedArt + off, name) : 1;
                if (!cmp)
                {
                    sEntry[k] = (int16_t)mid;
                    break;
                }
                if (cmp < 0)
                    lo = mid + 1;
                else
                    hi = mid;
            }
        }
        (void)i;
    }
    return sEntry[card] >= 0 ? gMtgPrintedArt + 16 + 24 * sEntry[card] : NULL;
}

int MtgGfx_HasPrinted(int card) { return EntryOf(card) != NULL; }

const char *MtgGfx_Artist(int card)
{
    const unsigned char *e = EntryOf(card);
    unsigned off = e ? Rd32(e + 20) : 0;
    return e && off < Total() ? (const char *)gMtgPrintedArt + off : NULL;
}

/* ---- pictures, decoded on demand and kept a while */
struct Pic
{
    int card;
    unsigned age;
    int w, h, artX, artY, artW, artH;
    uint32_t *px;
};
#define PICS 40
static struct Pic sPics[PICS];
static unsigned sAge;

static int Decode(unsigned offset, unsigned size, struct Pic *p)
{
    int w, h, channels, i;
    unsigned char *rgba;
    if (!size || offset + size > Total())
        return 0;
    rgba = stbi_load_from_memory(gMtgPrintedArt + offset, (int)size, &w, &h, &channels, 4);
    if (!rgba)
        return 0;
    p->px = SDL_malloc((size_t)w * h * 4);
    if (!p->px)
    {
        stbi_image_free(rgba);
        return 0;
    }
    for (i = 0; i < w * h; i++)
        p->px[i] = 0xff000000u | rgba[i * 4] << 16 | rgba[i * 4 + 1] << 8 | rgba[i * 4 + 2];
    stbi_image_free(rgba);
    p->w = w;
    p->h = h;
    p->artX = p->artY = 0;
    p->artW = w;
    p->artH = h;
    return 1;
}

static struct Pic *Picture(int card)
{
    const unsigned char *e = EntryOf(card);
    unsigned i, best = 0;
    struct Pic *p;
    if (!e)
        return NULL;
    for (i = 0; i < PICS; i++)
        if (sPics[i].px && sPics[i].card == card)
        {
            sPics[i].age = ++sAge;
            return &sPics[i];
        }
    for (i = 0; i < PICS; i++)
        if (!sPics[i].px || sPics[i].age < sPics[best].age)
        {
            best = i;
            if (!sPics[i].px)
                break;
        }
    p = &sPics[best];
    SDL_free(p->px);
    memset(p, 0, sizeof(*p));
    if (!Decode(Rd32(e + 4), Rd32(e + 8), p))
    {
        SDL_free(p->px);
        memset(p, 0, sizeof(*p));
        return NULL;
    }
    p->artX = (int)Rd16(e + 12);
    p->artY = (int)Rd16(e + 14);
    p->artW = (int)Rd16(e + 16);
    p->artH = (int)Rd16(e + 18);
    if (p->artW <= 0 || p->artH <= 0 || p->artX + p->artW > p->w || p->artY + p->artH > p->h)
        p->artX = p->artY = 0, p->artW = p->w, p->artH = p->h;
    p->card = card;
    p->age = ++sAge;
    return p;
}

static struct Pic *PrintedBack(void)
{
    static struct Pic back;
    static int tried;
    if (!tried)
    {
        tried = 1;
        if (!Valid() || !Decode(Rd32(gMtgPrintedArt + 8), Rd32(gMtgPrintedArt + 12), &back))
            memset(&back, 0, sizeof(back));
    }
    return back.px ? &back : NULL;
}

/* Box-filtered copy of a source rectangle into the surface */
static void Resample(const struct Pic *src, float sx, float sy, float sw, float sh, struct GSurface *dst, int dx,
                     int dy, int dw, int dh)
{
    int x, y;
    for (y = 0; y < dh; y++)
        for (x = 0; x < dw; x++)
        {
            float x0 = sx + sw * x / dw, x1 = sx + sw * (x + 1) / dw, y0 = sy + sh * y / dh, y1 = sy + sh * (y + 1) / dh;
            float acc[3] = {0, 0, 0}, total = 0;
            int ix, iy;
            for (iy = (int)y0; iy < (int)SDL_ceilf(y1) && iy < src->h; iy++)
                for (ix = (int)x0; ix < (int)SDL_ceilf(x1) && ix < src->w; ix++)
                {
                    float wx = G_MinF(x1, ix + 1.0f) - G_MaxF(x0, (float)ix);
                    float wy = G_MinF(y1, iy + 1.0f) - G_MaxF(y0, (float)iy), wgt = wx * wy;
                    uint32_t p;
                    if (wgt <= 0 || ix < 0 || iy < 0)
                        continue;
                    p = src->px[iy * src->w + ix];
                    acc[0] += wgt * ((p >> 16) & 255);
                    acc[1] += wgt * ((p >> 8) & 255);
                    acc[2] += wgt * (p & 255);
                    total += wgt;
                }
            if (total > 0)
                S_Px(dst, dx + x, dy + y,
                     0xff000000u | (unsigned)(acc[0] / total + 0.5f) << 16 | (unsigned)(acc[1] / total + 0.5f) << 8 |
                         (unsigned)(acc[2] / total + 0.5f));
        }
}

/* The picture in a box, cut to the box's shape around its middle */
static void ArtInto(const struct Pic *p, struct GSurface *s, int x, int y, int w, int h)
{
    float sx = (float)p->artX, sy = (float)p->artY, sw = (float)p->artW, sh = (float)p->artH;
    if (sw * h > sh * w)
    {
        float nw = sh * w / h;
        sx += (sw - nw) / 2;
        sw = nw;
    }
    else
    {
        float nh = sw * h / w;
        sy += (sh - nh) / 2;
        sh = nh;
    }
    Resample(p, sx, sy, sw, sh, s, x, y, w, h);
}

/* ---- colors */
unsigned MtgGfx_ManaColor(int kind)
{
    static const unsigned colors[] = {0xf4ecc8, 0x4a90d8, 0x4c4450, 0xe05430, 0x3e9a50, 0xb8b4ac};
    return colors[kind >= 0 && kind < 6 ? kind : 5];
}

enum { F_WHITE, F_BLUE, F_BLACK, F_RED, F_GREEN, F_GOLD, F_ARTIFACT, F_LAND, F_COLORLESS };
static const struct
{
    unsigned base, light, dark, box;   /* box: the text box's paper */
} sFrames[] = {
    [F_WHITE] = {0xe0d8bc, 0xfaf6e6, 0x8a8068, 0xfbf8ee},
    [F_BLUE] = {0x3c74b8, 0x8cc0f0, 0x1a3a68, 0xe0ecf8},
    [F_BLACK] = {0x4a4450, 0x8c8494, 0x18141c, 0xdcd8de},
    [F_RED] = {0xc8503a, 0xf09878, 0x6a2010, 0xf8e4da},
    [F_GREEN] = {0x3c8a4c, 0x90d09a, 0x184a24, 0xe2f0de},
    [F_GOLD] = {0xc8a040, 0xf0d890, 0x6a5018, 0xf8f0d8},
    [F_ARTIFACT] = {0x9aa4ac, 0xd8e0e6, 0x4a545c, 0xeef2f4},
    [F_LAND] = {0x9a7a58, 0xd4b896, 0x4a3420, 0xf2e8da},
    [F_COLORLESS] = {0xb0aca4, 0xe0dcd4, 0x5a5650, 0xf2f0ec},
};
static int FrameOf(const MtgCardDef *c)
{
    uint8_t colors = c->colors;
    if (c->types & MTG_T_LAND)
        return F_LAND;
    if (colors & (colors - 1))
        return F_GOLD;
    if (colors)
        return __builtin_ctz(colors);
    return c->types & MTG_T_ARTIFACT ? F_ARTIFACT : F_COLORLESS;
}
unsigned MtgGfx_FrameColor(int card, int shade)
{
    const MtgCardDef *c = card >= 0 ? Mtg_Card(card) : NULL;
    int f = c ? FrameOf(c) : F_COLORLESS;
    return shade == 1 ? sFrames[f].light : shade == 2 ? sFrames[f].dark : shade == 3 ? sFrames[f].box : sFrames[f].base;
}

/* ---- the back: the game's own (a deep brown field, a five-colored ring of dots round a gold star) */
static void FullCard(struct GSurface *s, const struct Pic *p);
static void Back(struct GSurface *s)
{
    struct Pic *printed = sBackMode == G_BACK_PRINTED ? PrintedBack() : NULL;
    int w = s->w, h = s->h, x, y, k;
    float cx = w / 2.0f, cy = h / 2.0f, r = (w < h ? w : h) * 0.30f;
    if (printed)
    {
        FullCard(s, printed);
        return;
    }
    S_RoundFill(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(0x1c0e06, 255));
    for (y = 1; y < h - 1; y++)
        for (x = 1; x < w - 1; x++)
        {
            float dx = (x + 0.5f - cx) / (w * 0.45f), dy = (y + 0.5f - cy) / (h * 0.45f);
            float d = SDL_sqrtf(dx * dx + dy * dy);
            S_Px(s, x, y, ARGB(G_Mix(0x7a4a24, 0x2e180a, G_MinF(1, d)), 255));
        }
    if (w >= 18)
    {
        S_Ring(s, cx, cy, r * 1.25f, 1.0f, ARGB(0xd8a850, 220));
        for (k = 0; k < 5; k++)
        {
            float a = -1.5708f + k * 1.2566f;
            S_Disc(s, cx + SDL_cosf(a) * r * 1.25f, cy + SDL_sinf(a) * r * 1.25f, w >= 40 ? 3.0f : 1.6f,
                   ARGB(MtgGfx_ManaColor(k), 255));
        }
        S_Disc(s, cx, cy, r * 0.42f, ARGB(0xe8c060, 255));
        S_Disc(s, cx - r * 0.12f, cy - r * 0.12f, r * 0.16f, ARGB(0xfff0c0, 255));
    }
    S_RoundFrame(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(0x0a0402, 255));
    if (w >= 26)
        S_RoundFrame(s, 1, 1, w - 2, h - 2, 1, ARGB(0x9a6a34, 255));
}

static void FullCard(struct GSurface *s, const struct Pic *p)
{
    Resample(p, 0, 0, (float)p->w, (float)p->h, s, 0, 0, s->w, s->h);
    if (s->w >= 12)
    {
        S_Px(s, 0, 0, 0);
        S_Px(s, s->w - 1, 0, 0);
        S_Px(s, 0, s->h - 1, 0);
        S_Px(s, s->w - 1, s->h - 1, 0);
    }
}

/* A land without a printed picture: its mana's color in a soft field, its symbol in the middle */
static void LandArt(struct GSurface *s, const MtgCardDef *c, int x, int y, int w, int h)
{
    int kind = c->intrinsicMana ? __builtin_ctz(c->intrinsicMana) : MTG_C, j;
    unsigned col = MtgGfx_ManaColor(kind);
    for (j = 0; j < h; j++)
        S_Fill(s, x, y + j, w, 1, ARGB(G_Mix(G_Mix(col, 0xffffff, 0.35f), G_Mix(col, 0x000000, 0.3f),
                                             (float)j / (h > 1 ? h - 1 : 1)), 255));
    S_Disc(s, x + w / 2.0f, y + h / 2.0f, (w < h ? w : h) * 0.3f, ARGB(G_Mix(col, 0xffffff, 0.5f), 255));
    S_Ring(s, x + w / 2.0f, y + h / 2.0f, (w < h ? w : h) * 0.3f, 1.0f, ARGB(G_Mix(col, 0x000000, 0.5f), 255));
}

/* The face's layout at w x h: the border, the name bar, the illustration, the type bar, the text box */
struct Layout
{
    int b, nameH, artX, artY, artW, artH, typeY, typeH, boxY, boxH;
};
static void LayoutOf(int w, int h, struct Layout *l)
{
    if (w < 22)   /* a small card (a land on the table, a pile): the picture fills it, a thin name bar above */
    {
        l->b = 1;
        l->nameH = 2;
        l->artX = 1;
        l->artY = 4;
        l->artW = w - 2;
        l->artH = h - 4 - 3;
        l->typeH = 0;
        l->typeY = l->artY + l->artH + 1;
        l->boxY = h;
        l->boxH = 0;
        return;
    }
    l->b = w >= 40 ? 3 : w >= 22 ? 2 : 1;
    l->nameH = w >= 60 ? 8 : w >= 40 ? 6 : w >= 22 ? 4 : 2;
    l->artX = l->b + 1;
    l->artY = l->b + l->nameH + 1;
    l->artW = w - 2 * l->b - 2;
    l->artH = l->artW * 4 / 5;   /* (the printed illustrations are about 5:4) */
    l->typeH = w >= 60 ? 6 : w >= 40 ? 4 : w >= 22 ? 3 : 2;
    l->typeY = l->artY + l->artH + 1;
    l->boxY = l->typeY + l->typeH + 1;
    l->boxH = h - l->b - l->boxY;
    if (l->boxH < 1)
    {
        l->artH += l->boxH - 1;
        l->typeY = l->artY + l->artH + 1;
        l->boxY = l->typeY + l->typeH + 1;
        l->boxH = h - l->b - l->boxY;
    }
}

/* a token's illustration (tokens have no printed card): the card's color in soft diagonal bands behind a silver
 * token coin with a T, so a token never looks like a card whose art is missing */
static void TokenArt(struct GSurface *s, unsigned base, int x, int y, int w, int h)
{
    int i, j;
    float cx = x + w / 2.0f, cy = y + h / 2.0f, r = G_MinF(w, h) * 0.34f;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
            S_Fill(s, x + i, y + j, 1, 1, ARGB(G_Mix(base, ((i + j) / 3) & 1 ? 0xffffff : 0x000000, 0.18f), 255));
    if (r < 2)
        return;
    S_Disc(s, cx, cy, r + 0.8f, ARGB(0x303438, 255));
    S_Disc(s, cx, cy, r, ARGB(0xd8dee6, 255));
    S_Ring(s, cx, cy, r - 1.0f, 0.7f, ARGB(0x8a96a4, 255));
    if (r >= 4)
    {
        int tw = (int)(r * 1.1f), th = (int)(r * 1.1f), sw = tw / 4 > 0 ? tw / 4 : 1;
        S_Fill(s, (int)(cx - tw / 2.0f), (int)(cy - th / 2.0f), tw, sw, ARGB(0x3a4250, 255));
        S_Fill(s, (int)(cx - sw / 2.0f), (int)(cy - th / 2.0f), sw, th, ARGB(0x3a4250, 255));
    }
}

static void Face(struct GSurface *s, int card)
{
    const MtgCardDef *c = Mtg_Card(card);
    int w = s->w, h = s->h, f = FrameOf(c), i, k;
    unsigned base = sFrames[f].base, light = sFrames[f].light, dark = sFrames[f].dark, box = sFrames[f].box;
    struct Layout l;
    struct Pic *art;
    LayoutOf(w, h, &l);
    /* the frame: a gradient from light at the top to the base color, a dark rim */
    S_RoundFill(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(base, 255));
    for (i = 1; i < h - 1; i++)
        S_Fill(s, 1, i, w - 2, 1, ARGB(G_Mix(G_Mix(base, light, 0.18f), G_Mix(base, dark, 0.3f), (float)i / h), 255));
    if (f == F_GOLD && w >= 22)   /* gold: a thin stripe of each of its colors down the sides */
    {
        int n = 0;
        for (k = 0; k < 5; k++)
            if (c->colors & MTG_COLOR_BIT(k))
            {
                S_Fill(s, 1, l.artY + n * 3, 1, 3, ARGB(MtgGfx_ManaColor(k), 255));
                S_Fill(s, w - 2, l.artY + n * 3, 1, 3, ARGB(MtgGfx_ManaColor(k), 255));
                n++;
            }
    }
    S_RoundFrame(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(G_Mix(dark, 0x000000, 0.3f), 255));
    /* the name bar and the mana cost as dots on its right */
    S_RoundFill(s, l.b, l.b, w - 2 * l.b, l.nameH, w >= 40 ? 1 : 0, ARGB(G_Mix(light, 0xffffff, 0.35f), 255));
    if (w >= 22)
        S_Fill(s, l.b + 2, l.b + l.nameH / 2, (w - 2 * l.b) / 2, 1, ARGB(G_Mix(dark, light, 0.4f), 200));
    if (!(c->types & MTG_T_LAND) && w >= 18)
    {
        float r = l.nameH / 2.0f - (w >= 40 ? 1.0f : 0.5f), cx = w - l.b - r - 1.5f, cy = l.b + l.nameH / 2.0f;
        int dots = 0;
        for (k = MTG_C; k >= 0 && dots < 5; k--)
            for (i = 0; i < c->cost.pip[k] && dots < 5; i++, dots++, cx -= 2 * r + 1)
                S_Disc(s, cx, cy, r, ARGB(MtgGfx_ManaColor(k), 255)), S_Ring(s, cx, cy, r, 0.8f, ARGB(0x202020, 200));
        if ((c->cost.generic || c->cost.x) && dots < 5)
            S_Disc(s, cx, cy, r, ARGB(0xc8c4bc, 255)), S_Ring(s, cx, cy, r, 0.8f, ARGB(0x202020, 200));
    }
    else if (c->types & MTG_T_LAND && w >= 18 && c->intrinsicMana)   /* a land: the mana it makes */
    {
        float r = l.nameH / 2.0f - (w >= 40 ? 1.0f : 0.5f), cx = w - l.b - r - 1.5f, cy = l.b + l.nameH / 2.0f;
        for (k = 0; k < 5; k++)
            if (c->intrinsicMana & MTG_COLOR_BIT(k))
            {
                S_Disc(s, cx, cy, r, ARGB(MtgGfx_ManaColor(k), 255)), S_Ring(s, cx, cy, r, 0.8f, ARGB(0x202020, 200));
                cx -= 2 * r + 1;
            }
    }
    /* the illustration */
    S_Fill(s, l.artX - 1, l.artY - 1, l.artW + 2, l.artH + 2, ARGB(G_Mix(dark, 0x000000, 0.4f), 255));
    art = Picture(card);
    if (art)
        ArtInto(art, s, l.artX, l.artY, l.artW, l.artH);
    else if (c->types & MTG_T_LAND)
        LandArt(s, c, l.artX, l.artY, l.artW, l.artH);
    else if (card >= MTG_TOKEN_BASE)
        TokenArt(s, base, l.artX, l.artY, l.artW, l.artH);
    else
        S_Fill(s, l.artX, l.artY, l.artW, l.artH, ARGB(G_Mix(base, 0x000000, 0.2f), 255));
    /* the type bar */
    if (l.typeH && l.typeY + l.typeH < h - l.b)
    {
        S_RoundFill(s, l.b, l.typeY, w - 2 * l.b, l.typeH, w >= 40 ? 1 : 0, ARGB(G_Mix(light, 0xffffff, 0.35f), 255));
        if (w >= 30)
            S_Fill(s, l.b + 2, l.typeY + l.typeH / 2, (w - 2 * l.b) / 3, 1, ARGB(G_Mix(dark, light, 0.4f), 200));
        if (w >= 30)   /* the rarity's mark at its right: black, silver, gold */
        {
            unsigned mark = c->rarity == MTG_RARITY_RARE ? 0xe0b030 : c->rarity == MTG_RARITY_UNCOMMON ? 0xb8c4d0 : 0x202020;
            S_Disc(s, w - l.b - l.typeH / 2.0f - 1, l.typeY + l.typeH / 2.0f, l.typeH / 2.0f - 0.5f, ARGB(mark, 255));
        }
    }
    /* the text box: lines that read as text from afar; a big mana symbol on a basic land */
    if (l.boxH >= 3)
    {
        int lineH = w >= 60 ? 5 : w >= 40 ? 4 : 3, lines = (l.boxH - 2) / lineH, plate = 0;
        S_Fill(s, l.b + 1, l.boxY, w - 2 * l.b - 2, l.boxH - 1, ARGB(box, 255));
        if (c->types & MTG_T_CREATURE && w >= 22)
            plate = 1;
        if ((c->supertypes & MTG_ST_BASIC) && c->intrinsicMana)
        {
            float r = G_MinF((w - 2 * l.b) * 0.22f, (l.boxH - 2) * 0.42f);
            int kind = __builtin_ctz(c->intrinsicMana);
            S_Disc(s, w / 2.0f, l.boxY + l.boxH / 2.0f - 0.5f, r, ARGB(MtgGfx_ManaColor(kind), 255));
            S_Ring(s, w / 2.0f, l.boxY + l.boxH / 2.0f - 0.5f, r, 1.0f, ARGB(0x303030, 255));
        }
        else
            for (k = 0; k < lines && k < 4; k++)
                S_Fill(s, l.b + 2, l.boxY + 1 + k * lineH + lineH / 2, w - 2 * l.b - 4 - (k == lines - 1 ? w / 3 : 0), 1,
                       ARGB(0x8a8478, 150));
        if (plate)   /* the power / toughness plate over the box's corner */
        {
            int pw = w >= 60 ? 16 : w >= 40 ? 11 : 7, ph = w >= 60 ? 8 : w >= 40 ? 6 : 4;
            S_RoundFill(s, w - l.b - pw, h - l.b - ph, pw, ph, 1, ARGB(G_Mix(light, 0xffffff, 0.3f), 255));
            S_RoundFrame(s, w - l.b - pw, h - l.b - ph, pw, ph, 1, ARGB(dark, 255));
        }
    }
}

/* ---- textures */
struct Tex
{
    int card, w, h;
    unsigned age, mode;
    SDL_Texture *texture;
};
#define TEXTURES 160
static struct Tex sTex[TEXTURES];
static unsigned sTexAge;

void MtgGfx_Forget(void)
{
    unsigned i;
    for (i = 0; i < TEXTURES; i++)
        if (sTex[i].texture)
            SDL_DestroyTexture(sTex[i].texture);
    memset(sTex, 0, sizeof(sTex));
}

void MtgGfx_SetModes(unsigned art, unsigned back)
{
    if (back >= G_BACK_COUNT)
        back = G_BACK_PRINTED;
    if (art == sArtMode && back == sBackMode)
        return;
    sArtMode = art;
    sBackMode = back;
    MtgGfx_Forget();
}

SDL_Texture *MtgGfx_CardTexture(int card, int w, int h)
{
    unsigned i, best = 0;
    struct Tex *t;
    struct GSurface s;
    if (!G_R() || w <= 0 || h <= 0)
        return NULL;
    if (card < 0 || !Mtg_Card(card))
        card = MTG_GFX_BACK;
    for (i = 0; i < TEXTURES; i++)
        if (sTex[i].texture && sTex[i].card == card && sTex[i].w == w && sTex[i].h == h && sTex[i].mode == sArtMode)
        {
            sTex[i].age = ++sTexAge;
            return sTex[i].texture;
        }
    for (i = 0; i < TEXTURES; i++)
        if (!sTex[i].texture || sTex[i].age < sTex[best].age)
        {
            best = i;
            if (!sTex[i].texture)
                break;
        }
    t = &sTex[best];
    if (t->texture)
        SDL_DestroyTexture(t->texture);
    memset(t, 0, sizeof(*t));
    s = G_SurfaceNew(w, h);
    if (!s.px)
        return NULL;
    if (card == MTG_GFX_BACK)
        Back(&s);
    else
    {
        struct Pic *p = sArtMode == G_ART_FULL_CARD ? Picture(card) : NULL;
        if (p)
            FullCard(&s, p);
        else
            Face(&s, card);
    }
    t->texture = G_SurfaceTexture(&s);
    G_SurfaceFree(&s);
    if (!t->texture)
        return NULL;
    if (w < 40)
        SDL_SetTextureScaleMode(t->texture, SDL_ScaleModeLinear);
    t->card = card;
    t->w = w;
    t->h = h;
    t->mode = sArtMode;
    t->age = ++sTexAge;
    return t->texture;
}

/* The sizes faces are built at (the other card games' sizes); anything in between scales the next size up */
static const int sSizes[][2] = {{12, 17}, {18, 25}, {22, 31}, {30, 42}, {44, 61}, {72, 100}};
static int SizeFor(float w)
{
    int i;
    for (i = 0; i < 6; i++)
        if ((float)sSizes[i][0] >= w - 0.5f)
            return i;
    return 5;
}

void MtgGfx_ArtRect(int card, int w, int h, int *ax, int *ay, int *aw, int *ah)
{
    struct Pic *p = sArtMode == G_ART_FULL_CARD && card >= 0 ? Picture(card) : NULL;
    struct Layout l;
    int pick = SizeFor((float)w), tw = sSizes[pick][0], th = sSizes[pick][1];
    if (p && p->w > 0 && p->h > 0)
    {
        *ax = (p->artX * w + p->w / 2) / p->w;
        *ay = (p->artY * h + p->h / 2) / p->h;
        *aw = (p->artW * w + p->w / 2) / p->w;
        *ah = (p->artH * h + p->h / 2) / p->h;
        return;
    }
    LayoutOf(tw, th, &l);
    *ax = l.artX * w / tw;
    *ay = l.artY * h / th;
    *aw = (l.artW * w + tw / 2) / tw;
    *ah = (l.artH * h + th / 2) / th;
}

void MtgGfx_DrawCard(int card, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
{
    SDL_Texture *t;
    SDL_FRect dst = {x, y, w, h};
    int pick;
    if (w < 1 || h < 1 || !alpha)
        return;
    pick = SizeFor(w);
    t = MtgGfx_CardTexture(card, sSizes[pick][0], sSizes[pick][1]);
    if (!t)
        return;
    if ((int)w == sSizes[pick][0] && (int)h == sSizes[pick][1] && angle == 0)
    {
        dst.x = (float)(int)(x + 0.5f);
        dst.y = (float)(int)(y + 0.5f);
    }
    SDL_SetTextureAlphaMod(t, alpha > 255 ? 255 : alpha);
    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyExF(G_R(), t, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
    if (flash)
    {
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_ADD);
        SDL_SetTextureAlphaMod(t, flash > 255 ? 255 : flash);
        SDL_RenderCopyExF(G_R(), t, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
    }
    SDL_SetTextureAlphaMod(t, 255);
}

static struct
{
    int card, w, h;
    unsigned mode;
    SDL_Texture *texture;
} sArt = {-2, 0, 0, 0, NULL};
SDL_Texture *MtgGfx_ArtTexture(int card, int *w, int *h)
{
    struct Pic *p;
    struct GSurface s;
    if (sArt.texture && sArt.card == card && sArt.mode == sArtMode)
        goto done;
    if (sArt.texture)
        SDL_DestroyTexture(sArt.texture);
    sArt.texture = NULL;
    sArt.card = -2;
    if ((p = Picture(card)) == NULL)
        return NULL;
    {
        int full = sArtMode == G_ART_FULL_CARD;
        s = G_SurfaceNew(full ? p->w : p->artW, full ? p->h : p->artH);
        if (!s.px)
            return NULL;
        if (full)
            Resample(p, 0, 0, (float)p->w, (float)p->h, &s, 0, 0, s.w, s.h);
        else
            Resample(p, (float)p->artX, (float)p->artY, (float)p->artW, (float)p->artH, &s, 0, 0, s.w, s.h);
        sArt.texture = G_SurfaceTexture(&s);
        sArt.w = s.w;
        sArt.h = s.h;
        G_SurfaceFree(&s);
        if (!sArt.texture)
            return NULL;
        SDL_SetTextureScaleMode(sArt.texture, SDL_ScaleModeLinear);
    }
    sArt.card = card;
    sArt.mode = sArtMode;
done:
    if (w)
        *w = sArt.w;
    if (h)
        *h = sArt.h;
    return sArt.texture;
}

/* ---- mana symbols on the screen: 7x7 dots (5x5 small), the generic amount's digits in a grey one */
static void Symbol(int x, int y, unsigned col, const char *digits, int big)
{
    int d = big ? 7 : 5;
    G_Fill(x + 1, y, d - 2, d, 0x202020);
    G_Fill(x, y + 1, d, d - 2, 0x202020);
    G_Fill(x + 1, y + 1, d - 2, d - 2, col);
    if (big)
    {
        G_Fill(x + 2, y + 1, d - 4, 1, col), G_Fill(x + 1, y + 2, 1, d - 4, col);
        G_Fill(x + 2, y + 2, 1, 1, G_Mix(col, 0xffffff, 0.6f));
    }
    if (digits && big)
        G_Digits(x + (d - G_DigitsWidth(digits, 0, 1)) / 2 + 0, y + 1, digits, 0, 1, 0x202020, 255);
}

int MtgGfx_Cost(int card, int x, int y, int right, int big)
{
    const MtgCardDef *c = Mtg_Card(card);
    int step = big ? 8 : 6, n = 0, k, i, w;
    char g[8];
    if (c->types & MTG_T_LAND)
        return 0;
    n += c->cost.x;
    n += c->cost.generic ? 1 : 0;
    for (k = 0; k < 6; k++)
        n += c->cost.pip[k];
    w = n ? n * step - 1 : 0;
    if (right)
        x -= w;
    for (i = 0; i < c->cost.x; i++, x += step)
        Symbol(x, y, 0xc8c4bc, "X", big);
    if (c->cost.generic)
    {
        snprintf(g, sizeof(g), "%u", c->cost.generic);
        Symbol(x, y, 0xc8c4bc, g, big);
        x += step;
    }
    for (k = 0; k < 6; k++)
        for (i = 0; i < c->cost.pip[k]; i++, x += step)
            Symbol(x, y, MtgGfx_ManaColor(k), NULL, big);
    return w;
}

void MtgGfx_KeywordLine(int card, uint32_t kw, char *out, unsigned cap)
{
    static const char *const words[MTG_KW_COUNT] = {"FLYING", "REACH", "HASTE", "VIGILANCE", "DEFENDER", "FIRST STRIKE",
                                                    "DOUBLE STRIKE", "TRAMPLE", "SHROUD", "FEAR", "FLASH", "LIFELINK"};
    static const char *const walks[5] = {"PLAINSWALK", "ISLANDWALK", "SWAMPWALK", "MOUNTAINWALK", "FORESTWALK"};
    static const char *const colors[5] = {"WHITE", "BLUE", "BLACK", "RED", "GREEN"};
    const MtgCardDef *c = card >= 0 ? Mtg_Card(card) : NULL;
    unsigned n = 0;
    int k;
    if (!cap)
        return;
    out[0] = 0;
    if (!c)
        return;
    for (k = 0; k < MTG_KW_COUNT; k++)
        if (kw & MTG_KW_BIT(k) && n + 1 < cap)
            n += (unsigned)snprintf(out + n, cap - n, "%s%s", n ? "  " : "", words[k]);
    for (k = 0; k < 5; k++)
        if (c->landwalk & (1u << k) && n + 1 < cap)
            n += (unsigned)snprintf(out + n, cap - n, "%s%s", n ? "  " : "", walks[k]);
    for (k = 0; k < 5; k++)
        if (c->protection & MTG_COLOR_BIT(k) && n + 1 < cap)
            n += (unsigned)snprintf(out + n, cap - n, "%sPROTECTION FROM %s", n ? "  " : "", colors[k]);
}
