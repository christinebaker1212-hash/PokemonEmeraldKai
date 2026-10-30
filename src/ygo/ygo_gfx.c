/* Yu-Gi-Oh! card faces: frames colored by kind around the printed illustration or Stairway's picture, or the whole
 * printed card, built once into textures at the sizes the screens use. */
#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include <string.h>
#include "../tcg/tcg_gfx.h"
#include "ygo_gfx.h"

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
void stbi_image_free(void *);
extern const unsigned char gYgoPrintedArt[], gYgoPrintedArtEnd[];
extern const unsigned char gYgoSddArt[], gYgoSddArtEnd[];

#define SDD_ART_W 72
#define SDD_ART_H 80
#define SDD_ART_BYTES 0x10E0
#define SDD_PAL_BYTES 0x80

static unsigned sMode, sBackMode;

/* ---- names and colors */
static const struct
{
    unsigned base, light, dark;
} sFrames[] = {
    [YGO_NORMAL] = {0xd8b070, 0xf0d8a8, 0x7a5a28},
    [YGO_EFFECT] = {0xd88040, 0xf0b888, 0x7c3a10},
    [YGO_FUSION] = {0x9060c0, 0xc8a8e8, 0x4a2a70},
    [YGO_RITUAL] = {0x5880d0, 0xa8c0f0, 0x264882},
    [YGO_SPELL] = {0x30a890, 0x98dcc8, 0x0e5848},
    [YGO_TRAP] = {0xc04888, 0xeaa4cc, 0x6c1846},
    [YGO_TOKEN] = {0xa0a0a0, 0xd8d8d8, 0x505050},
};
unsigned Ygo_FrameColor(unsigned id, int shade)
{
    const struct YgoCard *c = Ygo_Card(id);
    unsigned f = c ? c->frame : YGO_NORMAL;
    return shade == 1 ? sFrames[f].light : shade == 2 ? sFrames[f].dark : sFrames[f].base;
}
unsigned Ygo_AttributeColor(unsigned attribute)
{
    static const unsigned colors[] = {0x909090, 0xf0d040, 0x8040a0, 0x3878e0, 0xe03828, 0x986830, 0x40b048, 0xe8c060};
    return colors[attribute < 8 ? attribute : 0];
}
const char *Ygo_AttributeName(unsigned attribute)
{
    static const char *const names[] = {"", "LIGHT", "DARK", "WATER", "FIRE", "EARTH", "WIND", "DIVINE"};
    return names[attribute < 8 ? attribute : 0];
}
const char *Ygo_RaceName(unsigned race)
{
    static const char *const names[] = {"", "Aqua", "Beast", "Beast-Warrior", "Dinosaur", "Dragon", "Fairy", "Fiend",
                                        "Fish", "Insect", "Machine", "Plant", "Pyro", "Reptile", "Rock",
                                        "Sea Serpent", "Spellcaster", "Thunder", "Warrior", "Winged Beast", "Zombie"};
    return race < YGO_RACE_COUNT ? names[race] : "";
}
const char *Ygo_RarityName(unsigned rarity)
{
    static const char *const names[] = {"Common", "Short Print", "Super Short Print", "Rare", "Super Rare",
                                        "Ultra Rare", "Secret Rare"};
    return rarity < 7 ? names[rarity] : "";
}

/* ---- pictures, decoded on demand and kept a while */
struct Pic
{
    unsigned id, kind, age; /* kind 0: printed card, 1: Stairway picture */
    int w, h, artX, artY, artW, artH;
    uint32_t *px;
};
#define PICS 40
static struct Pic sPics[PICS];
static unsigned sAge;

static uint32_t Rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static unsigned Rd16(const unsigned char *p) { return p[0] | p[1] << 8; }

static int DecodePrinted(unsigned id, struct Pic *p)
{
    const unsigned char *b = gYgoPrintedArt, *e;
    unsigned count, offset, size, total = (unsigned)(gYgoPrintedArtEnd - gYgoPrintedArt);
    int w, h, channels, i;
    unsigned char *rgba;
    if (total < 8 || memcmp(b, "YGOP", 4))
        return 0;
    count = Rd32(b + 4);
    /* id count + 1: the card back, stored after the cards */
    if (!id || id > count + 1 || 8 + 16 * id > total)
        return 0;
    e = b + 8 + 16 * (id - 1);
    offset = Rd32(e);
    size = Rd32(e + 4);
    if (!size || offset + size > total || offset < 8 + 16 * id)
        return 0;
    rgba = stbi_load_from_memory(b + offset, (int)size, &w, &h, &channels, 4);
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
    p->artX = (int)Rd16(e + 8);
    p->artY = (int)Rd16(e + 10);
    p->artW = (int)Rd16(e + 12);
    p->artH = (int)Rd16(e + 14);
    return 1;
}

/* Stairway's picture: 72x80, 9x10 tiles of 8x8, 6 bits per pixel, 64 BGR555 colors (tools/ygo/import_sdd.py) */
static int DecodeSdd(unsigned sdd, struct Pic *p)
{
    const unsigned char *b = gYgoSddArt, *art, *pal;
    unsigned count, total = (unsigned)(gYgoSddArtEnd - gYgoSddArt), g, t, x, y;
    uint32_t colors[64];
    uint8_t pixels[SDD_ART_W * SDD_ART_H / 64][64];
    if (total < 8 || memcmp(b, "YGOA", 4))
        return 0;
    count = Rd32(b + 4);
    if (!sdd || sdd > count || 8 + (size_t)sdd * (SDD_ART_BYTES + SDD_PAL_BYTES) > total)
        return 0;
    art = b + 8 + (size_t)(sdd - 1) * (SDD_ART_BYTES + SDD_PAL_BYTES);
    pal = art + SDD_ART_BYTES;
    for (g = 0; g < 64; g++)
    {
        unsigned c = Rd16(pal + 2 * g);
        colors[g] = 0xff000000u | (((c & 31) * 255 / 31) << 16) | ((((c >> 5) & 31) * 255 / 31) << 8) |
                    (((c >> 10) & 31) * 255 / 31);
    }
    for (g = 0; g < SDD_ART_BYTES / 6; g++)
    {
        uint64_t v = 0;
        unsigned k;
        for (k = 0; k < 6; k++)
            v |= (uint64_t)art[g * 6 + k] << (8 * k);
        for (k = 0; k < 8; k++)
            pixels[(g * 8 + k) / 64][(g * 8 + k) % 64] = (v >> (6 * k)) & 63;
    }
    p->px = SDL_malloc(SDD_ART_W * SDD_ART_H * 4);
    if (!p->px)
        return 0;
    for (t = 0; t < 90; t++)
        for (y = 0; y < 8; y++)
            for (x = 0; x < 8; x++)
                p->px[((t / 9) * 8 + y) * SDD_ART_W + (t % 9) * 8 + x] = colors[pixels[t][y * 8 + x]];
    p->w = SDD_ART_W;
    p->h = SDD_ART_H;
    p->artX = p->artY = 0;
    p->artW = SDD_ART_W;
    p->artH = SDD_ART_H;
    return 1;
}

static struct Pic *Picture(unsigned id, unsigned kind)
{
    const struct YgoCard *c = Ygo_Card(id);
    unsigned i, best = 0;
    struct Pic *p;
    if (!c || (kind == 1 && !c->sdd))
        return NULL;
    for (i = 0; i < PICS; i++)
        if (sPics[i].px && sPics[i].id == id && sPics[i].kind == kind)
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
    if (!(kind ? DecodeSdd(c->sdd, p) : DecodePrinted(id, p)))
    {
        SDL_free(p->px);
        memset(p, 0, sizeof(*p));
        return NULL;
    }
    p->id = id;
    p->kind = kind;
    p->age = ++sAge;
    return p;
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

/* The printed card back (CARD BACKS: PRINTED), decoded once */
static struct Pic *PrintedBack(void)
{
    static struct Pic back;
    static int tried;
    if (!tried)
    {
        tried = 1;
        if (gYgoPrintedArtEnd - gYgoPrintedArt >= 8 && !memcmp(gYgoPrintedArt, "YGOP", 4) &&
            !DecodePrinted(Rd32(gYgoPrintedArt + 4) + 1, &back))
            memset(&back, 0, sizeof(back));
    }
    return back.px ? &back : NULL;
}

/* ---- the face in the game's own frame */
static void FullCard(struct GSurface *s, const struct Pic *p);
static void Back(struct GSurface *s)
{
    struct Pic *printed = sBackMode == G_BACK_PRINTED ? PrintedBack() : NULL;
    if (printed)
    {
        FullCard(s, printed);
        return;
    }
    int w = s->w, h = s->h, x, y;
    float cx = w / 2.0f, cy = h / 2.0f;
    S_RoundFill(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(0x1a1008, 255));
    for (y = 1; y < h - 1; y++)
        for (x = 1; x < w - 1; x++)
        {
            float dx = (x + 0.5f - cx) / (w * 0.42f), dy = (y + 0.5f - cy) / (h * 0.40f);
            float r = SDL_sqrtf(dx * dx + dy * dy), a = SDL_atan2f(dy, dx);
            float swirl = SDL_sinf(r * 9.0f - a * 2.0f);
            unsigned c = r < 1.0f ? G_Mix(0x6a3a14, 0xb86a24, 0.5f + 0.5f * swirl * (1 - r))
                                  : G_Mix(0x4a2410, 0x2c160a, G_MinF(1, r - 1));
            S_Px(s, x, y, ARGB(c, 255));
        }
    S_RoundFrame(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(0x0a0604, 255));
    if (w >= 26)
        S_RoundFrame(s, 1, 1, w - 2, h - 2, 1, ARGB(0x8a5a2a, 255));
}

static void Face(struct GSurface *s, const struct YgoCard *c)
{
    int w = s->w, h = s->h, b = w >= 40 ? 3 : w >= 26 ? 2 : 1, i;
    unsigned base = sFrames[c->frame].base, light = sFrames[c->frame].light, dark = sFrames[c->frame].dark;
    int nameH = w >= 40 ? 6 : w >= 26 ? 4 : 2, artX, artY, artW, artH, y;
    struct Pic *art = NULL;
    S_RoundFill(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(base, 255));
    for (i = b; i < h - b; i++)
        S_Fill(s, b, i, w - 2 * b, 1, ARGB(G_Mix(light, base, (float)(i - b) / (h - 2 * b) * 1.2f), 255));
    S_RoundFrame(s, 0, 0, w, h, w >= 26 ? 2 : 1, ARGB(dark, 255));
    /* the name bar and the attribute */
    S_Fill(s, b, b, w - 2 * b, nameH, ARGB(G_Mix(dark, base, 0.25f), 255));
    if (w >= 26)
        S_Fill(s, b + 2, b + nameH / 2, (w - 2 * b) / 2, 1, ARGB(0xf0f0f0, 200));
    {
        unsigned dot = c->frame == YGO_SPELL ? 0x18a070 : c->frame == YGO_TRAP ? 0xc02878 : Ygo_AttributeColor(c->attribute);
        float r = nameH / 2.0f;
        S_Disc(s, w - b - r - 0.5f, b + r, r, ARGB(dot, 255));
    }
    y = b + nameH + 1;
    /* Level stars */
    if (c->frame <= YGO_RITUAL && w >= 26)
    {
        int star = w >= 40 ? 3 : 2, n = c->level, x = w - b - 2 - n * (star + 1);
        for (i = 0; i < n && x + i * (star + 1) > b; i++)
            S_Fill(s, x + i * (star + 1), y, star, star, ARGB(0xe85818, 255));
        y += star + 1;
    }
    /* the illustration: square, like the printed one */
    artW = w - 2 * b - 2;
    artH = artW;
    if (y + artH > h - b - (w >= 26 ? 6 : 2))
        artH = h - b - (w >= 26 ? 6 : 2) - y;
    artX = b + 1;
    artY = y;
    if (sMode == G_ART_GAME_BOY)
        art = Picture(c->id, 1);
    if (!art)
        art = Picture(c->id, 0);
    S_Fill(s, artX - 1, artY - 1, artW + 2, artH + 2, ARGB(dark, 255));
    if (art)
        ArtInto(art, s, artX, artY, artW, artH);
    /* the text box: lines that read as text from afar; monsters keep a line for ATK / DEF */
    y = artY + artH + 2;
    if (y < h - b - 1)
    {
        int lines = (h - b - y) / (w >= 40 ? 4 : 3), l;
        S_Fill(s, b + 1, y - 1, w - 2 * b - 2, h - b - y, ARGB(G_Mix(light, 0xffffff, 0.5f), 255));
        for (l = 0; l < lines && l < 3; l++)
            S_Fill(s, b + 2, y + l * (w >= 40 ? 4 : 3) + 1, w - 2 * b - 4 - (l == lines - 1 ? w / 4 : 0), 1,
                   ARGB(0x808078, 160));
    }
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

/* ---- textures */
struct Tex
{
    unsigned id, age, mode;
    int w, h;
    SDL_Texture *texture;
};
#define TEXTURES 160
static struct Tex sTex[TEXTURES];
static unsigned sTexAge;

void Ygo_ForgetTextures(void)
{
    unsigned i;
    for (i = 0; i < TEXTURES; i++)
        if (sTex[i].texture)
            SDL_DestroyTexture(sTex[i].texture);
    memset(sTex, 0, sizeof(sTex));
}

void Ygo_SetArtMode(unsigned mode)
{
    if (mode == sMode)
        return;
    sMode = mode;
    Ygo_ForgetTextures();
}

void Ygo_SetBackMode(unsigned mode)
{
    if (mode >= G_BACK_COUNT)
        mode = G_BACK_PRINTED;
    if (mode == sBackMode)
        return;
    sBackMode = mode;
    Ygo_ForgetTextures();
}

SDL_Texture *Ygo_CardTexture(unsigned id, int w, int h)
{
    unsigned i, best = 0;
    struct Tex *t;
    struct GSurface s;
    const struct YgoCard *c = id ? Ygo_Card(id) : NULL;
    if (!G_R() || w <= 0 || h <= 0)
        return NULL;
    if (id && !c)
        id = 0;
    for (i = 0; i < TEXTURES; i++)
        if (sTex[i].texture && sTex[i].id == id && sTex[i].w == w && sTex[i].h == h && sTex[i].mode == sMode)
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
    if (!id)
        Back(&s);
    else
    {
        struct Pic *p = sMode == G_ART_FULL_CARD ? Picture(id, 0) : NULL;
        if (p)
            FullCard(&s, p);
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
    t->mode = sMode;
    t->age = ++sTexAge;
    return t->texture;
}

/* The sizes faces are built at (the Pokemon cards' sizes); anything in between scales the next size up */
static const int sSizes[][2] = {{12, 16}, {18, 25}, {22, 30}, {30, 42}, {44, 61}, {72, 100}};
/* Where the illustration is on a face of w x h as Ygo_DrawCard draws it (the foil over it): the printed card's own
 * picture rectangle when the whole printed card shows, else the frame's square box (Face) */
void Ygo_CardArtRect(unsigned id, int w, int h, int *ax, int *ay, int *aw, int *ah)
{
    const struct YgoCard *c = Ygo_Card(id);
    struct Pic *p = sMode == G_ART_FULL_CARD && id ? Picture(id, 0) : NULL;
    unsigned i, pick = 5;
    int tw, th, b, nameH, y;
    if (p && p->w > 0 && p->h > 0)
    {
        *ax = (p->artX * w + p->w / 2) / p->w;
        *ay = (p->artY * h + p->h / 2) / p->h;
        *aw = (p->artW * w + p->w / 2) / p->w;
        *ah = (p->artH * h + p->h / 2) / p->h;
        return;
    }
    /* (the face is built at the next size up and scaled to w x h) */
    for (i = 0; i < 6; i++)
        if ((float)sSizes[i][0] >= w - 0.5f)
        {
            pick = i;
            break;
        }
    tw = sSizes[pick][0], th = sSizes[pick][1];
    b = tw >= 40 ? 3 : tw >= 26 ? 2 : 1;
    nameH = tw >= 40 ? 6 : tw >= 26 ? 4 : 2;
    y = b + nameH + 1;
    if (c && c->frame <= YGO_RITUAL && tw >= 26)
        y += (tw >= 40 ? 3 : 2) + 1;
    *aw = tw - 2 * b - 2;
    *ah = *aw;
    if (y + *ah > th - b - (tw >= 26 ? 6 : 2))
        *ah = th - b - (tw >= 26 ? 6 : 2) - y;
    *ax = (b + 1) * w / tw;
    *ay = y * h / th;
    *aw = (*aw * w + tw / 2) / tw;
    *ah = (*ah * h + th / 2) / th;
}
void Ygo_DrawCard(unsigned id, float x, float y, float w, float h, double angle, unsigned alpha, unsigned flash)
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
    t = Ygo_CardTexture(id, sSizes[pick][0], sSizes[pick][1]);
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
    unsigned id, mode;
    int w, h, smooth;
    SDL_Texture *texture;
} sArt;
SDL_Texture *Ygo_ArtTexture(unsigned id, int *w, int *h, int *smooth)
{
    struct Pic *p = NULL;
    struct GSurface s;
    if (sArt.texture && sArt.id == id && sArt.mode == sMode)
        goto done;
    if (sArt.texture)
        SDL_DestroyTexture(sArt.texture);
    memset(&sArt, 0, sizeof(sArt));
    if (sMode == G_ART_GAME_BOY)
        p = Picture(id, 1);
    if (p)
    {
        s.w = p->w;
        s.h = p->h;
        s.px = p->px;
        sArt.texture = G_SurfaceTexture(&s);
        sArt.w = p->w;
        sArt.h = p->h;
    }
    else if ((p = Picture(id, 0)) != NULL)
    {
        int full = sMode == G_ART_FULL_CARD;
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
        sArt.smooth = 1;
        G_SurfaceFree(&s);
        if (sArt.texture)
            SDL_SetTextureScaleMode(sArt.texture, SDL_ScaleModeLinear);
    }
    if (!sArt.texture)
        return NULL;
    sArt.id = id;
    sArt.mode = sMode;
done:
    if (w)
        *w = sArt.w;
    if (h)
        *h = sArt.h;
    if (smooth)
        *smooth = sArt.smooth;
    return sArt.texture;
}
