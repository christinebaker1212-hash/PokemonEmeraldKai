// Moemon Co-op: the TRAINER CARD, one design for every region (trainer_card.c opens it over the game's own card, for
// the player's card and a friend's with the co-op add-on). The card is one of Pokemon Battle Revolution's Battle Pass
// designs (graphics/moemon/cards/passes.bin, tools/moemon_cards/build_passes.py), chosen on MY CARD with a hue for the
// recolorable ones, an accent color, a frame, the panels behind the text and a holo shine; it never changes color by
// itself. Its pages: the PROFILE (picture, showcase POKéMON, title, catchphrase, every region's stars), BADGES & STARS
// (each region's 8 badges as the BADGE CASE shows them and its 3 stars), RECORDS, TEAM & SIGNATURE, and on a friend's
// card YOU AND <NAME>. Drawn at 480x320 (display.c shows it as the game picture; spectators get it halved), the card
// tilting with the right stick or the mouse. COOP_NOTES "The TRAINER CARD".
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "sound.h"
#include "decompress.h"
#include "data.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "palette.h"
#include "trainer_card.h"   // (MON_ICON_TINT_*: the GAME CORNER photo)
#include "graphics.h"
#include "constants/songs.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_tcard.h"
#include "../tcg/tcg_gfx.h"

#define CS_W 480
#define CS_H 320
#define CARD_W 456                      // the card as drawn (the designs are 340x214 or 255x161)
#define CARD_H 288
#define FLIP_FRAMES 14
#define FADE_FRAMES 8
#define MAX_PASSES 64

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
void stbi_image_free(void *);
extern const u8 gMpPassData[], gMpPassDataEnd[];
void Gamepad_Sticks(float *lx, float *ly, float *rx, float *ry);
void Gamepad_Rumble(int strength, int ms);
bool8 Platform_PoketchMouse(s32 *x, s32 *y, bool8 *down);
void MP_BadgeCase_DrawBadgeFor(SDL_Renderer *r, const struct MpCardView *v, u8 region, u8 badge, int x, int y, int size);
void MP_BadgeCase_Tick(void);
void MP_BadgeCase_HollowShade(u8 shade);
void MP_BadgeCase_OpenView(const struct MpCardView *view, u8 region);
void MP_BadgeCase_Open(u8 region);
bool8 MP_BadgeCase_IsActive(void);
const u16 *MP_CardScreen_Halve(SDL_Renderer *r, SDL_Texture *canvas, u16 *out);

static const char *const sRegionNames[REGION_COUNT] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
// the regions in the order they're travelled (the BADGE CASE's tabs)
static const u8 sRegionOrder[REGION_COUNT] = {REGION_HOENN, REGION_KANTO, REGION_JOHTO, REGION_SINNOH};

static const u32 sAccents[TCARD_ACCENTS] = {
    0xe04848, 0xf08830, 0xe8c030, 0x80c838, 0x30a860, 0x30b0b0,
    0x3878e0, 0x5850c8, 0x9850c8, 0xe868a8, 0xb8c0c8, 0x383840,
};
static const char *const sAccentNames[TCARD_ACCENTS] = {
    "RED", "ORANGE", "GOLD", "LIME", "GREEN", "TEAL", "BLUE", "INDIGO", "PURPLE", "PINK", "SILVER", "BLACK",
};
static const char *const sFrameNames[TCARD_FRAME_COUNT] = {"NONE", "THIN", "THICK", "GLOW", "DOUBLE"};
static const char *const sPanelNames[TCARD_PANEL_COUNT] = {"LIGHT", "DARK", "GLASS", "NONE"};

enum { PAGE_PROFILE, PAGE_BADGES, PAGE_RECORDS, PAGE_TEAM, PAGE_TOGETHER, PAGE_COUNT };
static const char *const sPageNames[PAGE_COUNT] = {"PROFILE", "BADGES & STARS", "RECORDS", "TEAM & SIGNATURE", "TOGETHER"};

enum { MODE_CARD, MODE_MYCARD, MODE_ALBUM, MODE_SIGN };
enum {
    ROW_DESIGN, ROW_HUE, ROW_ACCENT, ROW_FRAME, ROW_PANEL, ROW_HOLO, ROW_SHOWCASE, ROW_TITLE, ROW_SHOWTITLE,
    ROW_TEAM, ROW_SIGN, ROW_ALBUM, ROW_COUNT
};
static const char *const sRowNames[ROW_COUNT] = {
    "DESIGN", "HUE", "ACCENT", "FRAME", "PANELS", "HOLO", "SHOWCASE", "TITLE", "SHOW TITLE", "CARD TEAM",
    "SIGNATURE", "FRIENDS' CARDS",
};

struct Pass
{
    char name[32];
    u8 flags;
    const u8 *png;
    u32 size;
};

static struct
{
    bool8 active, linger, draining, white;   // white: it fades to white (back to the FRONTIER PASS)
    u8 result;
    u8 mode;
    u8 page, prevPage;
    s8 flipDir;
    u8 flip, fadeIn, closing;
    u8 regionRow;                        // BADGES & STARS: the region the BADGE CASE opens on
    u8 row, albumAt;
    bool8 fromAlbum, styleDirty;
    u16 prevKeys, heldFrames;
    u32 age;
    float tiltX, tiltY;                  // -1..1, eased toward the stick / the mouse
    u32 mouseAt;                         // when the mouse last moved over the card (age)
    bool8 mouseDown;
    struct MpCardView view, ownBackup;
    struct MpCardStyle style;            // MY CARD's edits (saved when it closes)
    // the signature pad
    u8 sig[TCARD_SIG_BYTES];
    float penX, penY, lastX, lastY;
    bool8 penDown, eraser, sigChanged;
    // textures (made again when the renderer changes)
    SDL_Renderer *owner;
    SDL_Texture *canvas, *card, *text, *art, *frame, *holo, *streak, *star, *pic, *mon, *sigTex;
    SDL_Texture *icon[PARTY_SIZE];
    u16 iconSpecies[PARTY_SIZE];
    u8 iconTint[PARTY_SIZE];
    SDL_Texture *sticker[TCARD_STICKERS][4];
    int artDesign, artHue, frameDesign, frameStyle;
    u32 monKey;
    u8 sigShown[TCARD_SIG_BYTES];
    bool8 sigKnown;
} C;

static bool8 sStill;                     // tests: the card held flat
static struct Pass sPasses[MAX_PASSES];
static u8 sPassCount;
static bool8 sPassesRead;

// ----------------------------------------------------------------------------------------------------- the designs

static void ReadPasses(void)
{
    const u8 *p = gMpPassData, *end = gMpPassDataEnd;
    u16 count, i;
    if (sPassesRead)
        return;
    sPassesRead = TRUE;
    if (end - p < 8 || memcmp(p, "MPAS", 4) != 0)
        return;
    count = p[6] | (p[7] << 8);
    p += 8;
    for (i = 0; i < count && sPassCount < MAX_PASSES; i++)
    {
        struct Pass *pass = &sPasses[sPassCount];
        if (end - p < 40)
            break;
        memcpy(pass->name, p, 32);
        pass->name[31] = 0;
        pass->flags = p[32];
        pass->size = p[36] | (p[37] << 8) | (p[38] << 16) | ((u32)p[39] << 24);
        p += 40;
        if ((u32)(end - p) < pass->size)
            break;
        pass->png = p;
        p += pass->size;
        sPassCount++;
    }
}

u8 MP_CardScreen_DesignCount(void) { ReadPasses(); return sPassCount; }
static bool8 Recolorable(u8 design) { ReadPasses(); return design < sPassCount && (sPasses[design].flags & 1); }
static const char *DesignName(u8 design) { ReadPasses(); return design < sPassCount ? sPasses[design].name : "-"; }

static void HueRotate(u8 *rgb, float shift)
{
    float r = rgb[0] / 255.0f, g = rgb[1] / 255.0f, b = rgb[2] / 255.0f;
    float mx = G_MaxF(r, G_MaxF(g, b)), mn = G_MinF(r, G_MinF(g, b)), d = mx - mn, h, s, v = mx, f, p, q, t;
    int i;
    if (d <= 0.0001f)
        return;
    s = d / mx;
    if (mx == r)
        h = (g - b) / d + (g < b ? 6 : 0);
    else if (mx == g)
        h = (b - r) / d + 2;
    else
        h = (r - g) / d + 4;
    h += shift / 60.0f;
    while (h >= 6)
        h -= 6;
    while (h < 0)
        h += 6;
    i = (int)h;
    f = h - i;
    p = v * (1 - s), q = v * (1 - s * f), t = v * (1 - s * (1 - f));
    switch (i)
    {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    default: r = v, g = p, b = q; break;
    }
    rgb[0] = (u8)(r * 255 + 0.5f), rgb[1] = (u8)(g * 255 + 0.5f), rgb[2] = (u8)(b * 255 + 0.5f);
}

// the design at the card's size (bilinear, premultiplied so the edges don't darken), turned to its hue
static u32 *sArtPx;
static void MakeArt(SDL_Renderer *r, u8 design, u16 hue)
{
    int w, h, comp, x, y;
    u8 *rgba;
    float shift = Recolorable(design) ? (float)(hue % 360) : 0;
    if (C.art && C.artDesign == design && C.artHue == (int)shift)
        return;
    if (C.art)
        SDL_DestroyTexture(C.art);
    C.art = NULL;
    C.artDesign = design, C.artHue = (int)shift;
    if (design >= sPassCount)
        return;
    rgba = stbi_load_from_memory(sPasses[design].png, (int)sPasses[design].size, &w, &h, &comp, 4);
    if (rgba == NULL)
        return;
    if (sArtPx == NULL)
        sArtPx = malloc(CARD_W * CARD_H * 4);
    if (sArtPx == NULL)
    {
        stbi_image_free(rgba);
        return;
    }
    for (y = 0; y < CARD_H; y++)
        for (x = 0; x < CARD_W; x++)
        {
            float sx = (x + 0.5f) * w / CARD_W - 0.5f, sy = (y + 0.5f) * h / CARD_H - 0.5f;
            int x0 = (int)sx, y0 = (int)sy, k;
            float fx = sx - x0, fy = sy - y0, acc[4] = {0, 0, 0, 0};
            u8 out[4];
            if (sx < 0) x0 = 0, fx = 0;
            if (sy < 0) y0 = 0, fy = 0;
            for (k = 0; k < 4; k++)
            {
                int px = x0 + (k & 1), py = y0 + (k >> 1);
                float wgt = ((k & 1) ? fx : 1 - fx) * ((k >> 1) ? fy : 1 - fy), a;
                const u8 *s;
                if (px >= w) px = w - 1;
                if (py >= h) py = h - 1;
                s = rgba + (py * w + px) * 4;
                a = s[3] / 255.0f;
                acc[0] += s[0] * a * wgt, acc[1] += s[1] * a * wgt, acc[2] += s[2] * a * wgt, acc[3] += a * wgt;
            }
            if (acc[3] > 0.001f)
            {
                out[0] = (u8)G_MinF(255, acc[0] / acc[3] + 0.5f);
                out[1] = (u8)G_MinF(255, acc[1] / acc[3] + 0.5f);
                out[2] = (u8)G_MinF(255, acc[2] / acc[3] + 0.5f);
            }
            else
                out[0] = out[1] = out[2] = 0;
            out[3] = (u8)G_MinF(255, acc[3] * 255 + 0.5f);
            if (shift != 0 && out[3])
                HueRotate(out, shift);
            sArtPx[y * CARD_W + x] = ((u32)out[3] << 24) | (out[0] << 16) | (out[1] << 8) | out[2];
        }
    stbi_image_free(rgba);
    C.art = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, CARD_W, CARD_H);
    if (C.art)
    {
        SDL_UpdateTexture(C.art, NULL, sArtPx, CARD_W * 4);
        SDL_SetTextureBlendMode(C.art, SDL_BLENDMODE_NONE);
    }
    C.frameDesign = -1;   // (the frame follows the design's edge)
}

// how far each pixel is inside the design's own edge (its alpha: the round corners, the notch), by a two-pass chamfer
static float *sInside;
static void InsideDistances(void)
{
    int x, y;
    if (sInside == NULL)
        sInside = malloc(sizeof(float) * CARD_W * CARD_H);
    if (sInside == NULL)
        return;
    for (y = 0; y < CARD_H; y++)
        for (x = 0; x < CARD_W; x++)
        {
            float a = (sArtPx[y * CARD_W + x] >> 24) / 255.0f, d = a < 0.5f ? 0 : 1e9f;
            float edge = (float)G_MinF(G_MinF(x + 1, y + 1), G_MinF(CARD_W - x, CARD_H - y));
            sInside[y * CARD_W + x] = G_MinF(d, edge);
        }
    for (y = 0; y < CARD_H; y++)
        for (x = 0; x < CARD_W; x++)
        {
            float *d = &sInside[y * CARD_W + x];
            if (x > 0) *d = G_MinF(*d, d[-1] + 1);
            if (y > 0) *d = G_MinF(*d, d[-CARD_W] + 1);
            if (x > 0 && y > 0) *d = G_MinF(*d, d[-CARD_W - 1] + 1.4142f);
            if (x < CARD_W - 1 && y > 0) *d = G_MinF(*d, d[-CARD_W + 1] + 1.4142f);
        }
    for (y = CARD_H - 1; y >= 0; y--)
        for (x = CARD_W - 1; x >= 0; x--)
        {
            float *d = &sInside[y * CARD_W + x];
            if (x < CARD_W - 1) *d = G_MinF(*d, d[1] + 1);
            if (y < CARD_H - 1) *d = G_MinF(*d, d[CARD_W] + 1);
            if (x < CARD_W - 1 && y < CARD_H - 1) *d = G_MinF(*d, d[CARD_W + 1] + 1.4142f);
            if (x > 0 && y < CARD_H - 1) *d = G_MinF(*d, d[CARD_W - 1] + 1.4142f);
        }
}

static float Band(float d, float from, float to)
{
    // 1 between from and to (distances inside: negative), soft at its edges
    float a = G_MinF(d - from + 0.5f, to - d + 0.5f);
    return a <= 0 ? 0 : a >= 1 ? 1 : a;
}

// the frame: white, tinted by the accent when drawn, inside the design's own edge
static void MakeFrame(SDL_Renderer *r, u8 style)
{
    static u32 px[CARD_W * CARD_H];
    int x, y;
    if (C.frame && C.frameDesign == C.artDesign && C.frameStyle == style)
        return;
    if (C.frame)
        SDL_DestroyTexture(C.frame);
    C.frame = NULL;
    C.frameDesign = C.artDesign, C.frameStyle = style;
    if (style == TCARD_FRAME_NONE || sArtPx == NULL)
        return;
    InsideDistances();
    for (y = 0; y < CARD_H; y++)
        for (x = 0; x < CARD_W; x++)
        {
            float d = sInside ? -sInside[y * CARD_W + x] + 0.5f : 0, a = 0;
            switch (style)
            {
            case TCARD_FRAME_THIN: a = Band(d, -8, -5); break;
            case TCARD_FRAME_THICK: a = Band(d, -13, -5); break;
            case TCARD_FRAME_DOUBLE: a = G_MaxF(Band(d, -7, -5), Band(d, -12, -10)); break;
            case TCARD_FRAME_GLOW:
                a = Band(d, -8, -5);   // (its light dies away before the pass's label and emblem)
                if (d < -8 && d > -16)
                    a = G_MaxF(a, (1 - (-8 - d) / 8.0f) * 0.45f);
                break;
            }
            a *= (sArtPx[y * CARD_W + x] >> 24) / 255.0f;
            px[y * CARD_W + x] = ((u32)(a * 255 + 0.5f) << 24) | 0xFFFFFF;
        }
    C.frame = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, CARD_W, CARD_H);
    if (C.frame)
    {
        SDL_UpdateTexture(C.frame, NULL, px, CARD_W * 4);
        SDL_SetTextureBlendMode(C.frame, SDL_BLENDMODE_BLEND);
    }
}

static u32 HsvRgb(float h, float s, float v)
{
    u8 rgb[3];
    int k;
    float c = v * s, x, m = v - c, rr = 0, gg = 0, bb = 0;
    h = h - 360.0f * (int)(h / 360.0f);
    x = c * (1 - SDL_fabsf((h / 60.0f) - 2 * (int)(h / 120.0f) - 1));
    k = (int)(h / 60.0f);
    if (k == 0) rr = c, gg = x;
    else if (k == 1) rr = x, gg = c;
    else if (k == 2) gg = c, bb = x;
    else if (k == 3) gg = x, bb = c;
    else if (k == 4) rr = x, bb = c;
    else rr = c, bb = x;
    rgb[0] = (u8)((rr + m) * 255), rgb[1] = (u8)((gg + m) * 255), rgb[2] = (u8)((bb + m) * 255);
    return (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
}

// the holo's rainbow foil, its moving streak of light, a star
static void MakeLooks(SDL_Renderer *r)
{
    static u32 px[256 * 256];
    int x, y;
    if (!C.holo)
    {
        for (y = 0; y < 256; y++)
            for (x = 0; x < 256; x++)
            {
                float d = (x + y * 0.7f) / 256.0f;
                float band = 0.5f + 0.5f * SDL_sinf(d * 6.2831853f * 3 + SDL_sinf(y * 0.05f) * 0.6f);
                u32 a = (u32)(band * band * 255);
                px[y * 256 + x] = (a << 24) | HsvRgb(d * 720.0f + x * 0.3f, 0.55f, 1.0f);
            }
        C.holo = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 256, 256);
        if (C.holo)
        {
            SDL_UpdateTexture(C.holo, NULL, px, 256 * 4);
            SDL_SetTextureBlendMode(C.holo, SDL_BLENDMODE_ADD);
            SDL_SetTextureScaleMode(C.holo, SDL_ScaleModeLinear);
        }
    }
    if (!C.streak)
    {
        for (x = 0; x < 64; x++)
        {
            float t = (x - 31.5f) / 32.0f;
            px[x] = ((u32)((1 - t * t) * (1 - t * t) * 255) << 24) | 0xFFFFFF;
        }
        C.streak = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 1);
        if (C.streak)
        {
            SDL_UpdateTexture(C.streak, NULL, px, 64 * 4);
            SDL_SetTextureBlendMode(C.streak, SDL_BLENDMODE_ADD);
            SDL_SetTextureScaleMode(C.streak, SDL_ScaleModeLinear);
        }
    }
    if (!C.star)
    {
        // a five-pointed star, 32x32, anti-aliased by 4x4 samples
        float vx[10], vy[10];
        int k;
        for (k = 0; k < 10; k++)
        {
            float a = -1.5707963f + k * 0.6283185f, rad = (k & 1) ? 6.4f : 15.5f;
            vx[k] = 16 + SDL_cosf(a) * rad, vy[k] = 17 + SDL_sinf(a) * rad;
        }
        for (y = 0; y < 32; y++)
            for (x = 0; x < 32; x++)
            {
                int hits = 0, sx, sy;
                for (sy = 0; sy < 4; sy++)
                    for (sx = 0; sx < 4; sx++)
                    {
                        float fx = x + (sx + 0.5f) / 4, fy = y + (sy + 0.5f) / 4;
                        int i, j, in = 0;
                        for (i = 0, j = 9; i < 10; j = i++)
                            if ((vy[i] > fy) != (vy[j] > fy) && fx < (vx[j] - vx[i]) * (fy - vy[i]) / (vy[j] - vy[i]) + vx[i])
                                in = !in;
                        hits += in;
                    }
                px[y * 32 + x] = ((u32)(hits * 255 / 16) << 24) | 0xFFFFFF;
            }
        C.star = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 32);
        if (C.star)
        {
            SDL_UpdateTexture(C.star, NULL, px, 32 * 4);
            SDL_SetTextureBlendMode(C.star, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(C.star, SDL_ScaleModeLinear);
        }
    }
}

static u32 Rgb555(u16 c)
{
    return 0xFF000000 | ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}

// a POKéMON's menu icon (its first frame), in the card's print (the GAME CORNER photo: trainer_card.c LoadMonIconGfx)
static SDL_Texture *IconTexture(SDL_Renderer *r, u8 slot, u16 species, u8 tint)
{
    static u32 px[32 * 32];
    u16 pal[16];
    const u8 *tiles;
    const u16 *src;
    int t, i;
    if (C.icon[slot] && C.iconSpecies[slot] == species && C.iconTint[slot] == tint)
        return C.icon[slot];
    if (C.icon[slot])
        SDL_DestroyTexture(C.icon[slot]);
    C.icon[slot] = NULL;
    tiles = GetMonIconTiles(species, 0);
    src = gMonIconPalettes[GetMonIconPaletteIndexFromSpecies(species)];
    if (tiles == NULL || src == NULL)
        return NULL;
    memcpy(pal, src, sizeof(pal));
    switch (tint)
    {
    case MON_ICON_TINT_BLACK: TintPalette_CustomTone(pal, 16, 0, 0, 0); break;
    case MON_ICON_TINT_PINK: TintPalette_CustomTone(pal, 16, 500, 330, 310); break;
    case MON_ICON_TINT_SEPIA: TintPalette_SepiaTone(pal, 16); break;
    }
    for (t = 0; t < 16; t++)
        for (i = 0; i < 64; i++)
        {
            u8 b = tiles[t * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
            px[((t / 4) * 8 + i / 8) * 32 + (t % 4) * 8 + i % 8] = v ? Rgb555(pal[v]) : 0;
        }
    C.icon[slot] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 32, 32);
    if (C.icon[slot])
    {
        SDL_UpdateTexture(C.icon[slot], NULL, px, 32 * 4);
        SDL_SetTextureBlendMode(C.icon[slot], SDL_BLENDMODE_BLEND);
    }
    C.iconSpecies[slot] = species;
    C.iconTint[slot] = tint;
    return C.icon[slot];
}

// FireRed's card stickers (graphics/trainer_card/frlg/stickers.png: HALL OF FAME, EGGS, VICTORIES, 16x16 each, in the
// palette of their level 1-4)
extern const u32 gMpCardStickerTiles[];   // (mp_cardstickers.c: the asset macros need a file without SDL's headers)
extern const u16 gMpCardStickerPal1[], gMpCardStickerPal2[], gMpCardStickerPal3[], gMpCardStickerPal4[];
static SDL_Texture *StickerTexture(SDL_Renderer *r, u8 kind, u8 level)
{
    static const u16 *const sPals[] = {gMpCardStickerPal1, gMpCardStickerPal2, gMpCardStickerPal3, gMpCardStickerPal4};
    u32 px[16 * 16];
    const u8 *tiles = (const u8 *)gMpCardStickerTiles;
    int y, x;
    if (kind >= TCARD_STICKERS || level < 1 || level > 4)
        return NULL;
    if (C.sticker[kind][level - 1])
        return C.sticker[kind][level - 1];
    for (y = 0; y < 16; y++)
        for (x = 0; x < 16; x++)
        {
            // (the sheet is 2 tiles wide: a sticker is rows 2 * kind and 2 * kind + 1 of 8x8 tiles)
            int tile = (kind * 2 + y / 8) * 2 + x / 8;
            u8 b = tiles[tile * 32 + (y % 8) * 4 + (x % 8) / 2], v = (x & 1) ? b >> 4 : b & 15;
            px[y * 16 + x] = v ? Rgb555(sPals[level - 1][v]) : 0;
        }
    C.sticker[kind][level - 1] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 16, 16);
    if (C.sticker[kind][level - 1])
    {
        SDL_UpdateTexture(C.sticker[kind][level - 1], NULL, px, 16 * 4);
        SDL_SetTextureBlendMode(C.sticker[kind][level - 1], SDL_BLENDMODE_BLEND);
    }
    return C.sticker[kind][level - 1];
}

// the showcase POKéMON's battle picture (its own palette: shiny, the Stadium hue of a nickname)
static SDL_Texture *ShowcaseTexture(SDL_Renderer *r, const struct MpCardShowcase *s)
{
    static u8 tiles[MON_PIC_SIZE * 4];
    static u16 pal[64];
    static u32 px[64 * 64];
    u32 key = s->species * 2654435761u ^ s->personality ^ (s->otId * 40503u) ^ (MP_MonArtPokemon() ? 0x80000000u : 0);
    int x, y;
    if (s->species == SPECIES_NONE || s->species >= NUM_SPECIES)
        return NULL;
    if (C.mon && C.monKey == key)
        return C.mon;
    if (C.mon)
        SDL_DestroyTexture(C.mon);
    C.mon = NULL;
    C.monKey = key;
    HandleLoadSpecialPokePic(&gMonFrontPicTable[s->species], tiles, s->species, s->personality);
    LZ77UnCompWram(GetMonSpritePalFromSpeciesAndPersonality(s->species, s->otId, s->personality), pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 b = tiles[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 v = (x & 1) ? b >> 4 : b & 15;
            px[y * 64 + x] = v ? Rgb555(pal[v]) : 0;
        }
    C.mon = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 64);
    if (C.mon)
    {
        SDL_UpdateTexture(C.mon, NULL, px, 64 * 4);
        SDL_SetTextureBlendMode(C.mon, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(C.mon, SDL_ScaleModeNearest);
    }
    return C.mon;
}

static SDL_Texture *SignatureTexture(SDL_Renderer *r, const u8 *sig)
{
    static u32 px[TCARD_SIG_W * TCARD_SIG_H];
    int i;
    if (C.sigTex && C.sigKnown && memcmp(C.sigShown, sig, TCARD_SIG_BYTES) == 0)
        return C.sigTex;
    if (!C.sigTex)
    {
        C.sigTex = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TCARD_SIG_W, TCARD_SIG_H);
        if (!C.sigTex)
            return NULL;
        SDL_SetTextureBlendMode(C.sigTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(C.sigTex, SDL_ScaleModeNearest);
    }
    for (i = 0; i < TCARD_SIG_W * TCARD_SIG_H; i++)
        px[i] = ((sig[i / 8] >> (i % 8)) & 1) ? 0xFFFFFFFF : 0;
    SDL_UpdateTexture(C.sigTex, NULL, px, TCARD_SIG_W * 4);
    memcpy(C.sigShown, sig, TCARD_SIG_BYTES);
    C.sigKnown = TRUE;
    return C.sigTex;
}

static void ForgetTextures(void)
{
    // (the old renderer's textures went with it)
    C.canvas = C.card = C.text = C.art = C.frame = C.holo = C.streak = C.star = C.pic = C.mon = C.sigTex = NULL;
    memset(C.icon, 0, sizeof(C.icon));
    memset(C.iconSpecies, 0, sizeof(C.iconSpecies));
    C.artDesign = C.frameDesign = -1;
    C.sigKnown = FALSE;
    C.monKey = 0;
}

// ----------------------------------------------------------------------------------------------------- the card's look

// The card's grid, taken from the Battle Pass art itself (GBA pixels on the 228x144 card; the designs share it): the
// face inside the pass's bevel, the BATTLE PASS label's line (each page's heading sits on it, in the label's lettering,
// and the label itself is never covered: it ends at x 218, the content's right edge), the P emblem in the bottom-left
// corner (x 5-48, y 114-140: the row beside it starts at x 52) and the notch on the right edge (clear of x 218).
#define GRID_L 10                       // the content's left and right edges (the label's right margin, both sides)
#define GRID_R 218
#define GRID_HEAD_Y 2                   // the headings' line: their letters from y 6.5 to 13.5, as BATTLE PASS's
#define GRID_TOP 19                     // the content area
#define GRID_BOTTOM 111
#define GRID_ROW_X 52                   // the row beside the P emblem
#define GRID_ROW_Y 116
#define GRID_ROW_H 21
#define PANEL_RADIUS 5                  // the panels' corners (picture pixels), as round as the pass's own

struct Ink
{
    unsigned text, shadow, label;
};

static const struct MpCardStyle *Style(void) { return C.mode == MODE_MYCARD || C.mode == MODE_ALBUM ? &C.style : &C.view.style; }
static u32 Accent(void) { return sAccents[Style()->accent % TCARD_ACCENTS]; }

static bool8 IsLight(u32 rgb)
{
    return ((rgb >> 16) & 255) * 299 + ((rgb >> 8) & 255) * 587 + (rgb & 255) * 114 > 150000;
}

static struct Ink InkOf(void)
{
    struct Ink ink;
    switch (Style()->panel)
    {
    case TCARD_PANEL_LIGHT:
        ink.text = 0x303038, ink.shadow = 0xd0d0d8, ink.label = 0x5c6070;
        break;
    case TCARD_PANEL_GLASS:
        ink.text = 0x202028, ink.shadow = 0xf0f4f8, ink.label = 0x404858;
        break;
    default:
        ink.text = 0xf8f8f8, ink.shadow = 0x101018, ink.label = 0xc8ccd8;
        break;
    }
    return ink;
}

// a rounded rectangle at the picture's own pixels (GBA coordinates in; the card is drawn at twice them)
static void RoundRect(int gx, int gy, int gw, int gh, int rad, u32 rgb, u8 alpha)
{
    SDL_Renderer *r = G_R();
    int x = gx * 2, y = gy * 2, w = gw * 2, h = gh * 2, i;
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha);
    for (i = 0; i < rad; i++)
    {
        float dy = rad - i - 0.5f;
        int inset = rad - (int)(SDL_sqrtf((float)(rad * rad) - dy * dy) + 0.5f);
        SDL_Rect top = {x + inset, y + i, w - 2 * inset, 1}, bottom = {x + inset, y + h - 1 - i, w - 2 * inset, 1};
        SDL_RenderFillRect(r, &top);
        SDL_RenderFillRect(r, &bottom);
    }
    {
        SDL_Rect mid = {x, y + rad, w, h - 2 * rad};
        SDL_RenderFillRect(r, &mid);
    }
    SDL_RenderSetScale(r, 2, 2);
}

// a line along a rounded rectangle's top or bottom edge (inside its corners)
static void EdgeLine(int gx, int gy, int gw, int gh, bool8 bottom, u32 rgb, u8 alpha)
{
    SDL_Renderer *r = G_R();
    SDL_Rect line = {gx * 2 + PANEL_RADIUS, bottom ? (gy + gh) * 2 - 1 : gy * 2, gw * 2 - 2 * PANEL_RADIUS, 1};
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, (rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255, alpha);
    SDL_RenderFillRect(r, &line);
    SDL_RenderSetScale(r, 2, 2);
}

// what the words sit on: bevelled like the pass (a light top edge, a dark bottom one)
static void Panel(int x, int y, int w, int h)
{
    switch (Style()->panel)
    {
    case TCARD_PANEL_LIGHT:
        RoundRect(x, y, w, h, PANEL_RADIUS, 0xf8f8fc, 222);
        EdgeLine(x, y, w, h, FALSE, 0xffffff, 255);
        EdgeLine(x, y, w, h, TRUE, 0x000820, 60);
        break;
    case TCARD_PANEL_DARK:
        RoundRect(x, y, w, h, PANEL_RADIUS, 0x0a0e1c, 178);
        EdgeLine(x, y, w, h, FALSE, 0xffffff, 60);
        EdgeLine(x, y, w, h, TRUE, 0x000000, 110);
        break;
    case TCARD_PANEL_GLASS:
        RoundRect(x, y, w, h, PANEL_RADIUS, 0xffffff, 104);
        EdgeLine(x, y, w, h, FALSE, 0xffffff, 230);
        EdgeLine(x, y, w, h, TRUE, 0xffffff, 120);
        break;
    }
}

// the pass's own lettering (BATTLE PASS): light metal, darker below its middle, a dark edge one picture pixel wide and a
// soft shadow under it. The words are drawn once into C.text (white), then laid on the card in those colors.
static void LabelText(int x, int y, int align, const char *text, int font)
{
    static const s8 sAround[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
    SDL_Renderer *r = G_R();
    SDL_Texture *target = SDL_GetRenderTarget(r);
    int w = G_Measure(text, font), X, Y, k;
    SDL_Rect src, dst, upper;
    if (!C.text)
        return;
    if (align == TA_CENTER)
        x -= w / 2;
    else if (align == TA_RIGHT)
        x -= w;
    SDL_SetRenderTarget(r, C.text);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderSetScale(r, 2, 2);
    G_Text(1, 0, 0xffffff, G_NO_SHADOW, text, 0, font);
    SDL_SetRenderTarget(r, target);
    SDL_RenderSetScale(r, 1, 1);
    // (the game's letters made as tall and as bold as the pass's: 6/5 their size from their foot, a picture pixel wider)
    src.x = src.y = 0, src.w = (w + 2) * 2, src.h = G_LineHeight(font) * 2;
    dst.w = src.w * 6 / 5, dst.h = src.h * 6 / 5;
    X = x * 2 - 2, Y = y * 2 + 1 + (font == TF_NORMAL ? 28 : 23) - (font == TF_NORMAL ? 28 : 23) * 6 / 5;
    dst.x = X + 1, dst.y = Y + 2;
    SDL_SetTextureColorMod(C.text, 0, 0, 0);
    SDL_SetTextureAlphaMod(C.text, 90);
    SDL_RenderCopy(r, C.text, &src, &dst);
    dst.x = X + 2;
    SDL_RenderCopy(r, C.text, &src, &dst);
    SDL_SetTextureColorMod(C.text, 0x26, 0x28, 0x30);
    SDL_SetTextureAlphaMod(C.text, 255);
    for (k = 0; k < 8; k++)
    {
        int bold;
        for (bold = 0; bold < 2; bold++)
        {
            dst.x = X + bold + sAround[k][0], dst.y = Y + sAround[k][1];
            SDL_RenderCopy(r, C.text, &src, &dst);
        }
    }
    upper.x = X - 2, upper.y = 0, upper.w = dst.w + 4, upper.h = Y + (font == TF_NORMAL ? 28 : 23) - 8;
    for (k = 0; k < 2; k++)
    {
        dst.x = X + k, dst.y = Y;
        SDL_SetTextureColorMod(C.text, 0x9c, 0xa0, 0xae);
        SDL_RenderCopy(r, C.text, &src, &dst);
        SDL_RenderSetClipRect(r, &upper);
        SDL_SetTextureColorMod(C.text, 0xf8, 0xf8, 0xfc);
        SDL_RenderCopy(r, C.text, &src, &dst);
        SDL_RenderSetClipRect(r, NULL);
    }
    SDL_RenderSetScale(r, 2, 2);
}

// a page's heading, on the label's line at the left (as BATTLE PASS is at the right)
static void Heading(const char *text)
{
    LabelText(GRID_L, GRID_HEAD_Y, TA_LEFT, text, TF_SMALL);
}

// a strip in the accent color with its words, lit from above like the pass
static void Ribbon(int x, int y, int w, const char *text, int align)
{
    u32 a = Accent();
    unsigned ink = IsLight(a) ? 0x202024 : 0xf8f8f8, shadow = IsLight(a) ? G_Mix(a, 0xffffff, 0.5f) : G_Mix(a, 0, 0.5f);
    RoundRect(x, y, w, 12, PANEL_RADIUS, a, 240);
    RoundRect(x, y, w, 6, 3, 0xffffff, 46);
    EdgeLine(x, y, w, 12, TRUE, 0x000000, 90);
    G_TextAlign(align == TA_LEFT ? x + 5 : align == TA_RIGHT ? x + w - 5 : x + w / 2, y, align, ink, shadow, text, TF_SMALL);
}

static void Row(int x, int y, int w, const char *label, const char *value, int font)
{
    struct Ink ink = InkOf();
    G_Text(x, y, ink.label, ink.shadow, label, 0, font);
    if (value)
        G_TextAlign(x + w, y, TA_RIGHT, ink.text, ink.shadow, value, font);
}

// a star at GBA coordinates (size in GBA pixels): gold with a dark edge, or an empty one
static void Star(float x, float y, float size, bool8 lit)
{
    SDL_Renderer *r = G_R();
    SDL_FRect dst = {x, y, size, size};
    if (!C.star)
        return;
    if (lit)
    {
        SDL_FRect edge = {x - 0.5f, y - 0.5f, size + 1, size + 1};
        SDL_Rect upper = {(int)x - 1, (int)y - 1, (int)size + 2, (int)(size / 2) + 1};
        SDL_SetTextureColorMod(C.star, 120, 64, 8);
        SDL_SetTextureAlphaMod(C.star, 255);
        SDL_RenderCopyF(r, C.star, NULL, &edge);
        SDL_SetTextureColorMod(C.star, 248, 200, 48);
        SDL_RenderCopyF(r, C.star, NULL, &dst);
        SDL_RenderSetClipRect(r, &upper);   // (lit from above, like the pass's own metal)
        SDL_SetTextureColorMod(C.star, 255, 248, 200);
        SDL_SetTextureAlphaMod(C.star, 110);
        SDL_RenderCopyF(r, C.star, NULL, &dst);
        SDL_RenderSetClipRect(r, NULL);
    }
    else
    {
        bool8 dark = InkOf().text != 0xf8f8f8;
        SDL_SetTextureColorMod(C.star, dark ? 40 : 230, dark ? 44 : 232, dark ? 56 : 240);
        SDL_SetTextureAlphaMod(C.star, 64);
        SDL_RenderCopyF(r, C.star, NULL, &dst);
    }
    SDL_SetTextureColorMod(C.star, 255, 255, 255);
    SDL_SetTextureAlphaMod(C.star, 255);
}

static void Stars(float x, float y, float size, u8 count)
{
    u8 k;
    for (k = 0; k < 3; k++)
        Star(x + k * (size + 1), y, size, k < count);
}

static u8 BadgeCount(const struct MpCardView *v, u8 region)
{
    u8 b, n = 0;
    for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
        n += MP_CardView_HasBadge(v, region, b);
    return n;
}

static void Picture(SDL_Texture *t, int x, int y, int w, int h)
{
    SDL_Rect dst = {x, y, w, h};
    if (t)
        SDL_RenderCopy(G_R(), t, NULL, &dst);
}

static void FormatSeconds(char *buf, size_t size, u32 s)
{
    snprintf(buf, size, "%u:%02u", (unsigned)(s / 3600), (unsigned)(s / 60 % 60));
}

// the row beside the P emblem: a panel of its own
static void EmblemRow(void)
{
    Panel(GRID_ROW_X, GRID_ROW_Y, GRID_R - GRID_ROW_X, GRID_ROW_H);
}

// ----------------------------------------------------------------------------------------------------- the pages

static void DrawProfile(SDL_Renderer *r)
{
    const struct MpCardView *v = &C.view;
    struct Ink ink = InkOf();
    char buf[64];
    u8 k, total = 0, stars = 0;
    // the name on the label's line
    LabelText(GRID_L, GRID_HEAD_Y, TA_LEFT, v->name, TF_SMALL);
    // their picture, the numbers, the showcase POKéMON: three panels across
    Panel(GRID_L, GRID_TOP, 64, 77);
    if (v->hasPic)
    {
        if (!C.pic)
        {
            C.pic = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 64);
            if (C.pic)
            {
                SDL_UpdateTexture(C.pic, NULL, v->pic, 64 * 4);
                SDL_SetTextureBlendMode(C.pic, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(C.pic, SDL_ScaleModeNearest);
            }
        }
        Picture(C.pic, GRID_L, GRID_TOP + 2, 64, 64);
    }
    snprintf(buf, sizeof(buf), "IDNo.%05u", v->trainerId);
    G_TextAlign(GRID_L + 32, GRID_TOP + 64, TA_CENTER, ink.label, ink.shadow, buf, TF_NARROW);
    for (k = 0; k < REGION_COUNT; k++)
        total += BadgeCount(v, k), stars += MP_CardView_Stars(v, k);
    Panel(78, GRID_TOP, 72, 77);
    snprintf(buf, sizeof(buf), "\xA5%u", (unsigned)v->money);
    Row(82, GRID_TOP + 3, 64, "MONEY", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u", v->dex);
    Row(82, GRID_TOP + 17, 64, "MO\xc3\xa9" "DEX", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u:%02u", v->hours, v->minutes);
    Row(82, GRID_TOP + 31, 64, "TIME", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u", total);
    Row(82, GRID_TOP + 45, 64, "BADGES", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u/%u", stars, REGION_COUNT * 3);
    Row(82, GRID_TOP + 59, 64, "STARS", buf, TF_SMALL);
    Panel(154, GRID_TOP, 64, 77);
    if (v->showcase.species)
    {
        char nick[24];
        Picture(ShowcaseTexture(r, &v->showcase), 154, GRID_TOP + 1, 64, 64);
        MP_TCard_GameToText(nick, v->showcase.nickname, sizeof(nick));
        G_TextAlign(186, GRID_TOP + 64, TA_CENTER, ink.text, ink.shadow, nick, TF_NARROW);
        if (v->showcase.shiny)
            Star(206, GRID_TOP + 4, 8, TRUE);
    }
    else
        G_TextAlign(186, GRID_TOP + 33, TA_CENTER, ink.label, ink.shadow, "-", TF_SMALL);
    // the title and the catchphrase (the profile's words) under them
    {
        int x = GRID_L, w = GRID_R - GRID_L;
        if (v->title[0])
        {
            int tw = G_Measure(v->title, TF_SMALL) + 12;
            if (tw > w / 2)
                tw = w / 2;
            Ribbon(x, 99, tw, v->title, TA_CENTER);
            x += tw + 4, w -= tw + 4;
        }
        if (v->profile[0][0] || v->profile[1][0] || v->profile[2][0] || v->profile[3][0])
        {
            int font = TF_SMALL;
            snprintf(buf, sizeof(buf), "\"%s %s %s %s\"", (char *)v->profile[0], (char *)v->profile[1],
                     (char *)v->profile[2], (char *)v->profile[3]);
            if (G_Measure(buf, font) > w - 8)
                font = TF_NARROW;
            Panel(x, 99, w, 12);
            G_TextAlign(x + w / 2, 99, TA_CENTER, ink.text, ink.shadow, buf, font);
        }
    }
    // every region's stars, beside the emblem
    for (k = 0; k < REGION_COUNT; k++)
    {
        u8 region = sRegionOrder[k];
        int w = (GRID_R - GRID_ROW_X - 3 * 2) / REGION_COUNT, x = GRID_ROW_X + k * (w + 2);
        Panel(x, GRID_ROW_Y, w, GRID_ROW_H);
        G_TextAlign(x + w / 2, GRID_ROW_Y, TA_CENTER, ink.label, ink.shadow, sRegionNames[region], TF_NARROW);
        Stars(x + w / 2 - 11, GRID_ROW_Y + 11, 7, MP_CardView_Stars(v, region));
    }
}

static void DrawBadges(SDL_Renderer *r)
{
    const struct MpCardView *v = &C.view;
    struct Ink ink = InkOf();
    char buf[64], time[16];
    u8 k, b, region;
    Heading(sPageNames[PAGE_BADGES]);
    for (k = 0; k < REGION_COUNT; k++)
    {
        int y = GRID_TOP + k * 24;
        region = sRegionOrder[k];
        Panel(GRID_L, y, GRID_R - GRID_L, 22);
        if (k == C.regionRow)
        {
            u32 a = Accent();
            RoundRect(GRID_L, y, 4, 22, 2, a, 255);
            EdgeLine(GRID_L, y, GRID_R - GRID_L, 22, TRUE, a, 255);
        }
        G_Text(GRID_L + 6, y + 1, ink.text, ink.shadow, sRegionNames[region], 0, TF_NARROW);
        Stars(GRID_L + 6, y + 12, 7, MP_CardView_Stars(v, region));
        SDL_RenderSetScale(r, 1, 1);
        MP_BadgeCase_HollowShade(ink.text == 0xf8f8f8 ? 255 : 0);   // (badges not won yet: seen on dark panels too)
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
            MP_BadgeCase_DrawBadgeFor(r, v, region, b, (60 + b * 20) * 2, (y + 2) * 2, 36);
        MP_BadgeCase_HollowShade(0);
        SDL_RenderSetScale(r, 2, 2);
    }
    // the region chosen, beside the emblem
    region = sRegionOrder[C.regionRow];
    EmblemRow();
    FormatSeconds(time, sizeof(time), v->regionSeconds[region]);
    snprintf(buf, sizeof(buf), "%s  %u/8  %s", sRegionNames[region], BadgeCount(v, region),
             (v->cleared >> region) & 1 ? "CHAMPION" : "");
    G_Text(GRID_ROW_X + 5, GRID_ROW_Y + 4, ink.text, ink.shadow, buf, 0, TF_SMALL);
    G_TextAlign(GRID_R - 5, GRID_ROW_Y + 4, TA_RIGHT, ink.label, ink.shadow, time, TF_SMALL);
}

static void DrawRecords(void)
{
    static const char *const sLeft[] = {"CAUGHT", "HATCHED", "EVOLVED", "BATTLES", "TRAINERS", "WILD", "FISHED", "STEPS"};
    static const u8 sLeftStat[] = {TCARD_STAT_CATCHES, TCARD_STAT_HATCHED, TCARD_STAT_EVOLVED, TCARD_STAT_BATTLES,
                                   TCARD_STAT_TRAINERS, TCARD_STAT_WILD, TCARD_STAT_FISHED, TCARD_STAT_STEPS};
    static const char *const sRight[] = {"CONTESTS WON", "HALL OF FAME", "ACHIEVEMENTS", "WON WITH FRIENDS"};
    static const u8 sRightStat[] = {TCARD_STAT_CONTESTS, TCARD_STAT_HOF, TCARD_STAT_ACHIEVEMENTS, TCARD_STAT_TOGETHER};
    const struct MpCardView *v = &C.view;
    char buf[32], label[24];
    int half = (GRID_R - GRID_L - 4) / 2;
    u8 k;
    Heading(sPageNames[PAGE_RECORDS]);
    Panel(GRID_L, GRID_TOP, half, GRID_BOTTOM - GRID_TOP);
    Panel(GRID_L + half + 4, GRID_TOP, half, GRID_BOTTOM - GRID_TOP);
    for (k = 0; k < 8; k++)
    {
        snprintf(buf, sizeof(buf), "%u", (unsigned)v->stats[sLeftStat[k]]);
        Row(GRID_L + 5, GRID_TOP + 2 + k * 11, half - 10, sLeft[k], buf, TF_NARROW);
    }
    for (k = 0; k < 4; k++)
    {
        snprintf(buf, sizeof(buf), "%u", (unsigned)v->stats[sRightStat[k]]);
        Row(GRID_L + half + 9, GRID_TOP + 2 + k * 11, half - 10, sRight[k], buf, TF_NARROW);
    }
    for (k = 0; k < REGION_COUNT; k++)
    {
        u8 region = sRegionOrder[k];
        snprintf(label, sizeof(label), "IN %s", sRegionNames[region]);
        FormatSeconds(buf, sizeof(buf), v->regionSeconds[region]);
        Row(GRID_L + half + 9, GRID_TOP + 2 + (k + 4) * 11, half - 10, label, buf, TF_NARROW);
    }
    // the link records, beside the emblem
    EmblemRow();
    snprintf(buf, sizeof(buf), "%u-%u", v->linkWins, v->linkLosses);
    Row(GRID_ROW_X + 5, GRID_ROW_Y + 4, 76, "LINK", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u", v->trades);
    Row(GRID_ROW_X + 90, GRID_ROW_Y + 4, GRID_R - GRID_ROW_X - 95, "TRADES", buf, TF_SMALL);
}

static void DrawTeam(SDL_Renderer *r)
{
    const struct MpCardView *v = &C.view;
    struct Ink ink = InkOf();
    int step = (GRID_R - GRID_L) / PARTY_SIZE;
    u8 i;
    Heading(sPageNames[PAGE_TEAM]);
    Panel(GRID_L, GRID_TOP, GRID_R - GRID_L, 36);
    for (i = 0; i < v->teamCount && i < PARTY_SIZE; i++)
    {
        int x = GRID_L + i * step + (step - 32) / 2;
        Picture(IconTexture(r, i, v->team[i], v->style.tint), x, GRID_TOP + 1, 32, 32);
        if ((v->teamShiny >> i) & 1)
            Star(x + 25, GRID_TOP + 3, 7, TRUE);
    }
    if (v->teamCount == 0)
        G_TextAlign(114, GRID_TOP + 12, TA_CENTER, ink.label, ink.shadow, "-", TF_SMALL);
    // the signature, on a line like a real card's
    Panel(GRID_L, GRID_TOP + 39, GRID_R - GRID_L, GRID_BOTTOM - GRID_TOP - 39);
    G_FillA(GRID_L + 12, GRID_BOTTOM - 9, GRID_R - GRID_L - 24, 1, ink.label, 150);
    if (v->hasSig)
    {
        SDL_Texture *t = SignatureTexture(r, v->sig);
        if (t)
        {
            unsigned c = ink.text;
            SDL_FRect dst = {114 - TCARD_SIG_W * 0.375f, GRID_TOP + 40, TCARD_SIG_W * 0.75f, TCARD_SIG_H * 0.75f};
            SDL_SetTextureColorMod(t, (c >> 16) & 255, (c >> 8) & 255, c & 255);
            SDL_SetTextureScaleMode(t, SDL_ScaleModeLinear);
            SDL_RenderCopyF(r, t, NULL, &dst);
        }
    }
    else
        G_TextAlign(114, GRID_TOP + 56, TA_CENTER, ink.label, ink.shadow, v->own ? "MY CARD > SIGNATURE" : "-", TF_SMALL);
    // what the team is, beside the emblem
    EmblemRow();
    {
        char buf[32];
        u8 k, n = 0;
        for (k = 0; k < TCARD_STICKERS; k++)
            n += TCARD_STICKER(v->stickers, k) != 0;
        snprintf(buf, sizeof(buf), "%u", v->teamCount);
        Row(GRID_ROW_X + 5, GRID_ROW_Y + 4, GRID_R - GRID_ROW_X - 10 - n * 18, "POKéMON ON THE CARD", buf, TF_SMALL);
        // FOUR ISLAND's stickers at the row's end
        for (k = 0, n = 0; k < TCARD_STICKERS; k++)
            if (TCARD_STICKER(v->stickers, k))
                Picture(StickerTexture(r, k, TCARD_STICKER(v->stickers, k)), GRID_R - 5 - 16 - 18 * n++, GRID_ROW_Y + 2, 16, 16);
    }
}

static void DrawTogether(void)
{
    const struct MpCardView *v = &C.view;
    const struct MpTCardFriend *f = MP_TCard_FriendByName(v->name);
    struct Ink ink = InkOf();
    char buf[48];
    snprintf(buf, sizeof(buf), "YOU AND %s", v->name);
    Heading(buf);
    Panel(GRID_L, GRID_TOP, GRID_R - GRID_L, 58);
    snprintf(buf, sizeof(buf), "%u", f ? f->wonTogether : 0);
    Row(GRID_L + 6, GRID_TOP + 4, GRID_R - GRID_L - 12, "WON TOGETHER", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u - %u", f ? f->vsWins : 0, f ? f->vsLosses : 0);
    Row(GRID_L + 6, GRID_TOP + 17, GRID_R - GRID_L - 12, "YOU VS THEM", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u", f ? f->trades : 0);
    Row(GRID_L + 6, GRID_TOP + 30, GRID_R - GRID_L - 12, "TRADED", buf, TF_SMALL);
    snprintf(buf, sizeof(buf), "%u-%u", v->linkWins, v->linkLosses);
    Row(GRID_L + 6, GRID_TOP + 43, GRID_R - GRID_L - 12, "THEIR LINK BATTLES", buf, TF_SMALL);
    EmblemRow();
    if (f && f->firstMet)
    {
        time_t t = (time_t)f->firstMet;
        struct tm *tm = localtime(&t);
        if (tm)
            snprintf(buf, sizeof(buf), "%04d/%02d/%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        else
            snprintf(buf, sizeof(buf), "-");
    }
    else
        snprintf(buf, sizeof(buf), "-");
    Row(GRID_ROW_X + 5, GRID_ROW_Y + 4, GRID_R - GRID_ROW_X - 10, "FIRST MET", buf, TF_SMALL);
    (void)ink;
}

static u8 PageCount(void) { return C.view.own ? PAGE_TOGETHER : PAGE_COUNT; }

// the card itself, whole and flat, into C.card
static void DrawCard(SDL_Renderer *r, u8 page)
{
    const struct MpCardStyle *s = Style();
    u32 a = Accent();
    SDL_SetRenderTarget(r, C.card);
    SDL_RenderSetScale(r, 1, 1);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    MakeArt(r, s->design, s->hue);
    MakeFrame(r, s->frame);
    if (C.art)
        SDL_RenderCopy(r, C.art, NULL, NULL);
    else
    {
        SDL_RenderSetScale(r, 2, 2);
        G_RoundFill(0, 0, CARD_W / 2, CARD_H / 2, 0xd8d8e0, 255);
        SDL_RenderSetScale(r, 1, 1);
    }
    if (C.frame)
    {
        SDL_SetTextureColorMod(C.frame, (a >> 16) & 255, (a >> 8) & 255, a & 255);
        SDL_RenderCopy(r, C.frame, NULL, NULL);
    }
    SDL_RenderSetScale(r, 2, 2);
    switch (page)
    {
    case PAGE_PROFILE: DrawProfile(r); break;
    case PAGE_BADGES: DrawBadges(r); break;
    case PAGE_RECORDS: DrawRecords(); break;
    case PAGE_TEAM: DrawTeam(r); break;
    default: DrawTogether(); break;
    }
    SDL_RenderSetScale(r, 1, 1);
    if (s->holo && C.holo && C.streak)
    {
        // the holo: rainbow foil and a streak of light that move as the card tilts, a few glints
        SDL_Rect dst = {-120 + (int)(C.tiltX * 90), -100 + (int)(C.tiltY * 70),
                        CARD_W + 240, CARD_H + 200};
        SDL_FRect streak;
        int k;
        SDL_SetTextureAlphaMod(C.holo, 34);
        SDL_RenderCopy(r, C.holo, NULL, &dst);
        streak.w = 150, streak.h = 900;
        streak.x = CARD_W * (0.5f + C.tiltX * 0.9f) - 75;
        streak.y = (CARD_H - 900) / 2.0f;
        SDL_SetTextureAlphaMod(C.streak, 54);
        SDL_RenderCopyExF(r, C.streak, NULL, &streak, 24, NULL, SDL_FLIP_NONE);
        for (k = 0; k < 9; k++)
        {
            u32 h = (k + 1) * 2654435761u;
            int x = 20 + (int)(h % (CARD_W - 40)), y = 16 + (int)((h >> 12) % (CARD_H - 32));
            // (glints only as the card turns: still, it shows none)
            float turn = G_MinF(1, (SDL_fabsf(C.tiltX) + SDL_fabsf(C.tiltY)) * 4);
            float tw = 0.55f + (SDL_sinf(k * 1.7f + C.tiltX * 5 + C.tiltY * 3) - 0.55f) * turn;
            if (tw > 0.55f)
            {
                int len = (int)((tw - 0.55f) * 12);
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
                SDL_SetRenderDrawColor(r, 255, 255, 255, 150);
                SDL_RenderDrawLine(r, x - len, y, x + len, y);
                SDL_RenderDrawLine(r, x, y - len, x, y + len);
                SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            }
        }
    }
}

// the card tilted (and turned while it flips) in perspective, as a grid of triangles
#define GRID_X 16
#define GRID_Y 10
static void DrawCardQuad(SDL_Renderer *r, float cx, float cy, float scale, float ax, float ay, SDL_Color color)
{
    static SDL_Vertex v[(GRID_X + 1) * (GRID_Y + 1)];
    static int idx[GRID_X * GRID_Y * 6];
    float cax = SDL_cosf(ax), sax = SDL_sinf(ax), cay = SDL_cosf(ay), say = SDL_sinf(ay);
    int i, j, n = 0;
    for (j = 0; j <= GRID_Y; j++)
        for (i = 0; i <= GRID_X; i++)
        {
            float u = (float)i / GRID_X, w = (float)j / GRID_Y;
            float x = (u - 0.5f) * CARD_W * scale, y = (w - 0.5f) * CARD_H * scale, z = 0, x2, z2, y2, k;
            x2 = x * cay + z * say;
            z2 = -x * say + z * cay;
            y2 = y * cax - z2 * sax;
            z = y * sax + z2 * cax;
            k = 900.0f / (900.0f + z);
            v[j * (GRID_X + 1) + i].position.x = cx + x2 * k;
            v[j * (GRID_X + 1) + i].position.y = cy + y2 * k;
            v[j * (GRID_X + 1) + i].tex_coord.x = u;
            v[j * (GRID_X + 1) + i].tex_coord.y = w;
            v[j * (GRID_X + 1) + i].color = color;
        }
    for (j = 0; j < GRID_Y; j++)
        for (i = 0; i < GRID_X; i++)
        {
            int a = j * (GRID_X + 1) + i;
            idx[n++] = a, idx[n++] = a + 1, idx[n++] = a + GRID_X + 1;
            idx[n++] = a + 1, idx[n++] = a + GRID_X + 2, idx[n++] = a + GRID_X + 1;
        }
    SDL_RenderGeometry(r, C.card, v, (GRID_X + 1) * (GRID_Y + 1), idx, n);
}

static void Background(void)
{
    u32 a = Accent();
    int y;
    G_Fill(0, 0, 240, 160, 0x14141c);
    for (y = 0; y < 160; y += 4)
        G_FillA(0, y, 240, 4, a, 6 + y / 8);
}

// the hints under the card
static void HintsAt(int cx, int y, const char *text)
{
    int font = TF_SMALL, w = G_Hint(0, 0, G_WHITE, 0x202020, text, font, 0);
    if (w > 232)
        font = TF_NARROW, w = G_Hint(0, 0, G_WHITE, 0x202020, text, font, 0);
    G_Hint(cx - w / 2, y, G_WHITE, 0x202020, text, font, 1);
}

static void Hints(const char *text)
{
    HintsAt(120, 147, text);
}

static void DrawCardView(SDL_Renderer *r)
{
    float ax, ay, t;
    u8 page = C.page, k, n = PageCount();
    SDL_Color shade = {255, 255, 255, 255}, shadow = {0, 0, 0, 110};
    ay = C.tiltX * 0.30f;
    ax = -C.tiltY * 0.22f;
    if (C.flip)
    {
        // turned over to the next page: the old one away to the edge, the new one in from the other side
        t = 1.0f - (float)C.flip / FLIP_FRAMES;
        if (t < 0.5f)
            page = C.prevPage, ay += t * 2 * 1.5707963f * C.flipDir;
        else
            ay -= (1 - t) * 2 * 1.5707963f * C.flipDir;
        shade.r = shade.g = shade.b = (u8)(255 - 80 * SDL_fabsf(SDL_sinf(ay)));
    }
    DrawCard(r, page);
    SDL_SetRenderTarget(r, C.canvas);
    SDL_RenderSetScale(r, 2, 2);
    Background();
    SDL_RenderSetScale(r, 1, 1);
    DrawCardQuad(r, 240 + 6, 142 + 9, 0.94f, ax, ay, shadow);
    DrawCardQuad(r, 240, 142, 0.94f, ax, ay, shade);
    SDL_RenderSetScale(r, 2, 2);
    for (k = 0; k < n; k++)
        G_Circle(120 - (n - 1) * 4 + k * 8, 142, k == C.page ? 2.5f : 1.8f, k == C.page ? Accent() : 0x808088, 255);
    if (C.page == PAGE_BADGES)
        Hints(C.view.own ? "{UP}{DOWN} REGION {SELECT} POLISH {L}{R} PAGE {B} CLOSE"
                         : "{UP}{DOWN} REGION {SELECT} BADGE CASE {L}{R} PAGE {B} CLOSE");
    else if (C.view.own)
        Hints("{L}{R} PAGE {SELECT} BADGE CASE {START} MY CARD {B} CLOSE");
    else
        Hints(C.fromAlbum ? "{L}{R} PAGE {SELECT} BADGE CASE {B} BACK" : "{L}{R} PAGE {SELECT} BADGE CASE {B} CLOSE");
}

// ----------------------------------------------------------------------------------------------------- MY CARD

static void RowValue(u8 row, char *out, size_t size)
{
    const struct MpCardStyle *s = &C.style;
    switch (row)
    {
    case ROW_DESIGN: snprintf(out, size, "%s", DesignName(s->design)); break;
    case ROW_HUE:
        if (Recolorable(s->design))
            snprintf(out, size, "%u", s->hue);
        else
            snprintf(out, size, "-");
        break;
    case ROW_ACCENT: snprintf(out, size, "%s", sAccentNames[s->accent % TCARD_ACCENTS]); break;
    case ROW_FRAME: snprintf(out, size, "%s", sFrameNames[s->frame % TCARD_FRAME_COUNT]); break;
    case ROW_PANEL: snprintf(out, size, "%s", sPanelNames[s->panel % TCARD_PANEL_COUNT]); break;
    case ROW_HOLO: snprintf(out, size, "%s", s->holo ? "ON" : "OFF"); break;
    case ROW_SHOWCASE:
        if (C.view.showcase.species)
            MP_TCard_GameToText(out, C.view.showcase.nickname, (int)size);
        else
            snprintf(out, size, "NONE");
        break;
    case ROW_TITLE:
    {
        u16 t = MP_TCard_Title();
        const char *name, *desc;
        char status[8], reward[8];
        if (t == 0 || t > MP_Achieve_Total() || !MP_Achieve_IsDone(t - 1))
            snprintf(out, size, "NONE");
        else
        {
            MP_Achieve_Info(t - 1, &name, &desc, status, sizeof(status), reward, sizeof(reward));
            snprintf(out, size, "%s", name);
        }
        break;
    }
    case ROW_SHOWTITLE: snprintf(out, size, "%s", MP_TCard_ShowTitle() ? "ON" : "OFF"); break;
    case ROW_TEAM: snprintf(out, size, "%s", MP_TCard_TeamCount() ? "KEPT AS SET" : "MY PARTY"); break;
    case ROW_SIGN: snprintf(out, size, "%s", C.view.hasSig ? "SIGNED" : "NONE"); break;
    default: snprintf(out, size, "%u", MP_TCard_AlbumCount()); break;
    }
}

static const char *RowHelp(u8 row)
{
    switch (row)
    {
    case ROW_DESIGN: return "Battle Revolution's Battle Pass designs.";
    case ROW_HUE: return Recolorable(C.style.design) ? "Turn this design to any color." : "This design keeps its own colors.";
    case ROW_ACCENT: return "The color of the frame, the ribbons and the cursor.";
    case ROW_FRAME: return "The line around the card's edge.";
    case ROW_PANEL: return "What the words sit on.";
    case ROW_HOLO: return "Rainbow foil and light that follow the card's tilt.";
    case ROW_SHOWCASE: return "A POKéMON of your party, shown big on the front.";
    case ROW_TITLE: return "An achievement's name under yours.";
    case ROW_SHOWTITLE: return "Whether the title shows on the card.";
    case ROW_TEAM: return "The party as it is, or this team kept.";
    case ROW_SIGN: return "Sign the card with the stick, the D-Pad or the mouse.";
    default: return "The cards of friends you've seen.";
    }
}

static void DrawMyCard(SDL_Renderer *r)
{
    char value[48];
    u8 i;
    float ax = -C.tiltY * 0.22f, ay = C.tiltX * 0.30f;
    SDL_Color shade = {255, 255, 255, 255}, shadow = {0, 0, 0, 110};
    DrawCard(r, PAGE_PROFILE);
    SDL_SetRenderTarget(r, C.canvas);
    SDL_RenderSetScale(r, 2, 2);
    Background();
    SDL_RenderSetScale(r, 1, 1);
    DrawCardQuad(r, 352 + 4, 86 + 6, 0.5f, ax, ay, shadow);
    DrawCardQuad(r, 352, 86, 0.5f, ax, ay, shade);
    SDL_RenderSetScale(r, 2, 2);
    // the list
    G_RoundFill(4, 2, 108, 156, 0x101018, 200);
    G_TextAlign(58, 4, TA_CENTER, G_WHITE, 0x202020, C.mode == MODE_ALBUM ? "FRIENDS' CARDS" : "MY CARD", TF_NORMAL);
    if (C.mode == MODE_ALBUM)
    {
        u8 n = MP_TCard_AlbumCount(), k, first = C.albumAt > 7 ? C.albumAt - 7 : 0;
        if (n == 0)
            G_Text(10, 28, 0xb0b0b8, 0x202020, "Look at a friend's card to keep it here.", 96, TF_SMALL);
        for (k = first; k < n && k < first + 8; k++)
        {
            const struct MpTCardFriend *f = MP_TCard_AlbumFriend(k);
            int y = 24 + (k - first) * 14;
            char nm[24];
            if (k == C.albumAt)
                G_RoundFill(8, y - 1, 100, 13, Accent(), 230);
            MP_GameToAscii(nm, f->name, sizeof(nm));
            G_Text(12, y, k == C.albumAt && IsLight(Accent()) ? 0x282828 : G_WHITE, G_NO_SHADOW, nm, 0, TF_SMALL);
        }
        HintsAt(177, 100, "{A} THEIR CARD {B} BACK");
        return;
    }
    for (i = 0; i < ROW_COUNT; i++)
    {
        int y = 22 + i * 11;
        bool8 on = i == C.row, off = i == ROW_HUE && !Recolorable(C.style.design);
        unsigned ink = off ? 0x707078 : on && IsLight(Accent()) ? 0x282828 : G_WHITE;
        if (on)
            G_RoundFill(7, y - 1, 102, 12, Accent(), 230);
        G_Text(10, y, ink, G_NO_SHADOW, sRowNames[i], 0, TF_NARROW);
        if (i == ROW_ACCENT)
            G_RoundFill(96, y + 1, 9, 8, sAccents[C.style.accent % TCARD_ACCENTS], 255), G_RoundFrame(96, y + 1, 9, 8, G_WHITE, 200);
        else if (i == ROW_HUE && !off)
            G_RoundFill(96, y + 1, 9, 8, HsvRgb((float)C.style.hue, 0.8f, 0.95f), 255), G_RoundFrame(96, y + 1, 9, 8, G_WHITE, 200);
        else
        {
            const char *shortValue = NULL;
            char n[12];
            if (i == ROW_HOLO || i == ROW_SHOWTITLE)
                shortValue = (i == ROW_HOLO ? C.style.holo : MP_TCard_ShowTitle()) ? "ON" : "OFF";
            else if (i == ROW_DESIGN)
                snprintf(n, sizeof(n), "%u/%u", C.style.design + 1, sPassCount), shortValue = n;
            else if (i == ROW_FRAME)
                shortValue = sFrameNames[C.style.frame % TCARD_FRAME_COUNT];
            else if (i == ROW_PANEL)
                shortValue = sPanelNames[C.style.panel % TCARD_PANEL_COUNT];
            else if (i == ROW_TEAM)
                shortValue = MP_TCard_TeamCount() ? "KEPT" : "PARTY";
            else if (i == ROW_SIGN)
                shortValue = C.view.hasSig ? "SIGNED" : "-";
            else if (i == ROW_ALBUM)
                snprintf(n, sizeof(n), "%u", MP_TCard_AlbumCount()), shortValue = n;
            if (shortValue)
                G_TextAlign(106, y, TA_RIGHT, ink, G_NO_SHADOW, shortValue, TF_NARROW);
        }
    }
    // what the row is set to
    G_RoundFill(118, 92, 118, 66, 0x101018, 200);
    G_Text(122, 94, Accent() == 0x383840 ? 0xb8c0c8 : Accent(), G_NO_SHADOW, sRowNames[C.row], 0, TF_SMALL);
    if (C.row == ROW_DESIGN)
    {
        char n[12];
        snprintf(n, sizeof(n), "%u/%u", C.style.design + 1, sPassCount);
        G_TextAlign(232, 94, TA_RIGHT, 0xb0b0b8, G_NO_SHADOW, n, TF_SMALL);
    }
    RowValue(C.row, value, sizeof(value));
    G_Text(122, 106, G_WHITE, 0x202020, value, 0, G_Measure(value, TF_NORMAL) > 110 ? TF_SMALL : TF_NORMAL);
    G_Text(122, 122, 0xb0b0b8, G_NO_SHADOW, RowHelp(C.row), 112, TF_NARROW);
    if (C.row == ROW_SIGN || C.row == ROW_ALBUM)
        HintsAt(177, 145, "{A} OPEN {B} DONE");
    else
        HintsAt(177, 145, "{LEFT}{RIGHT} CHANGE {B} DONE");
}

// ----------------------------------------------------------------------------------------------------- the signature

static void SigPixel(int x, int y, bool8 ink)
{
    int i;
    if (x < 0 || y < 0 || x >= TCARD_SIG_W || y >= TCARD_SIG_H)
        return;
    i = y * TCARD_SIG_W + x;
    if (ink)
        C.sig[i / 8] |= 1 << (i % 8);
    else
        C.sig[i / 8] &= ~(1 << (i % 8));
}

static void SigLine(float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0, dy = y1 - y0, len = SDL_sqrtf(dx * dx + dy * dy);
    int steps = (int)(len * 2) + 1, k, ox, oy, rad = C.eraser ? 3 : 1;
    for (k = 0; k <= steps; k++)
    {
        float t = (float)k / steps;
        int x = (int)(x0 + dx * t), y = (int)(y0 + dy * t);
        for (oy = -rad; oy <= rad - (C.eraser ? 0 : 1); oy++)
            for (ox = -rad; ox <= rad - (C.eraser ? 0 : 1); ox++)
                SigPixel(x + ox + (C.eraser ? 0 : 1), y + oy + (C.eraser ? 0 : 1), !C.eraser);
    }
    C.sigChanged = TRUE;
}

#define SIG_X 48                        // the pad on the canvas: the signature at twice its pixels
#define SIG_Y 84

static void DrawSign(SDL_Renderer *r)
{
    SDL_Texture *t;
    SDL_Rect dst = {SIG_X, SIG_Y, TCARD_SIG_W * 2, TCARD_SIG_H * 2};
    SDL_SetRenderTarget(r, C.canvas);
    SDL_RenderSetScale(r, 2, 2);
    Background();
    G_TextAlign(120, 8, TA_CENTER, G_WHITE, 0x202020, "SIGNATURE", TF_NORMAL);
    G_TextAlign(120, 25, TA_CENTER, 0xb0b0b8, G_NO_SHADOW, C.eraser ? "ERASER" : "PEN", TF_SMALL);
    G_RoundFill(SIG_X / 2 - 4, SIG_Y / 2 - 4, TCARD_SIG_W + 8, TCARD_SIG_H + 8, 0xf8f8f0, 255);
    G_RoundFrame(SIG_X / 2 - 4, SIG_Y / 2 - 4, TCARD_SIG_W + 8, TCARD_SIG_H + 8, Accent(), 255);
    G_FillA(SIG_X / 2 + 8, SIG_Y / 2 + TCARD_SIG_H - 8, TCARD_SIG_W - 16, 1, 0xa0a098, 255);
    SDL_RenderSetScale(r, 1, 1);
    C.sigKnown = FALSE;   // (the pad changes as it's drawn on)
    t = SignatureTexture(r, C.sig);
    if (t)
    {
        SDL_SetTextureColorMod(t, 0x28, 0x28, 0x30);
        SDL_RenderCopy(r, t, NULL, &dst);
    }
    // the pen
    {
        int px = SIG_X + (int)(C.penX * 2), py = SIG_Y + (int)(C.penY * 2), rad = C.eraser ? 7 : 3;
        SDL_SetRenderDrawColor(r, 0xe0, 0x40, 0x40, 255);
        SDL_RenderDrawLine(r, px - rad - 4, py, px - rad, py);
        SDL_RenderDrawLine(r, px + rad, py, px + rad + 4, py);
        SDL_RenderDrawLine(r, px, py - rad - 4, px, py - rad);
        SDL_RenderDrawLine(r, px, py + rad, px, py + rad + 4);
    }
    SDL_RenderSetScale(r, 2, 2);
    Hints("{A} DRAW  {L}{R} PEN/ERASER  {SELECT} CLEAR  {B} DONE");
}

static void RefreshOwn(void);

static void LeaveSign(void)
{
    u32 i, any = 0;
    for (i = 0; i < TCARD_SIG_BYTES; i++)
        any |= C.sig[i];
    if (C.sigChanged)
    {
        MP_TCard_SetSignature(C.sig, any != 0);
        MPNet_Log("card: signature %s", any ? "saved" : "cleared");
    }
    RefreshOwn();
    C.mode = MODE_MYCARD;
}

static void SignStep(u16 keys, u16 pressed)
{
    float lx, ly, rx, ry, speed;
    s32 mx, my;
    bool8 down, moved = Platform_PoketchMouse(&mx, &my, &down);
    bool8 drawing = (keys & A_BUTTON) != 0;
    Gamepad_Sticks(&lx, &ly, &rx, &ry);
    if (pressed & (B_BUTTON | START_BUTTON))
    {
        LeaveSign();
        PlaySE(SE_SELECT);
        return;
    }
    if (pressed & SELECT_BUTTON)
    {
        memset(C.sig, 0, sizeof(C.sig));
        C.sigChanged = TRUE;
        PlaySE(SE_BALL_TRAY_EXIT);
    }
    if (pressed & (L_BUTTON | R_BUTTON))
    {
        C.eraser = !C.eraser;
        PlaySE(SE_SELECT);
    }
    C.lastX = C.penX, C.lastY = C.penY;
    if (moved || down)
    {
        float sx = (mx * 2 - SIG_X) / 2.0f, sy = (my * 2 - SIG_Y) / 2.0f;
        if (sx >= -8 && sy >= -8 && sx < TCARD_SIG_W + 8 && sy < TCARD_SIG_H + 8)
        {
            C.penX = G_MaxF(0, G_MinF(TCARD_SIG_W - 1, sx));
            C.penY = G_MaxF(0, G_MinF(TCARD_SIG_H - 1, sy));
            if (down && !C.mouseDown)
                C.lastX = C.penX, C.lastY = C.penY;
            drawing = drawing || down;
        }
    }
    C.mouseDown = down;
    // the stick (the D-Pad moves it too: the stick is the D-Pad's keys as well, so its own reading wins)
    if (lx * lx + ly * ly > 0.04f)
    {
        speed = 3.0f;
        C.penX += lx * speed, C.penY += ly * speed;
    }
    else
    {
        speed = C.heldFrames > 12 ? 2.0f : 1.0f;
        if (keys & DPAD_LEFT) C.penX -= speed;
        if (keys & DPAD_RIGHT) C.penX += speed;
        if (keys & DPAD_UP) C.penY -= speed;
        if (keys & DPAD_DOWN) C.penY += speed;
    }
    C.penX = G_MaxF(0, G_MinF(TCARD_SIG_W - 1, C.penX));
    C.penY = G_MaxF(0, G_MinF(TCARD_SIG_H - 1, C.penY));
    if (drawing)
    {
        if (!C.penDown)
            C.lastX = C.penX, C.lastY = C.penY;
        SigLine(C.lastX, C.lastY, C.penX, C.penY);
    }
    C.penDown = drawing;
}

// ----------------------------------------------------------------------------------------------------- MY CARD's input

static void RefreshOwn(void)
{
    bool8 hasPic = C.view.hasPic;
    static u32 pic[64 * 64];
    memcpy(pic, C.view.pic, sizeof(pic));
    MP_CardView_FillOwn(&C.view);
    C.view.hasPic = hasPic;
    memcpy(C.view.pic, pic, sizeof(pic));
    if (C.styleDirty)
        C.view.style = C.style;   // (MY CARD's edits, saved when it closes)
}

static s8 ShowcaseSlot(void)
{
    u8 i;
    const struct MpCardShowcase *s = &C.view.showcase;
    if (s->species == SPECIES_NONE)
        return -1;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == s->species
         && GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == s->personality)
            return i;
    return -1;
}

static void LeaveMyCard(void)
{
    if (C.styleDirty)
    {
        MP_TCard_SetStyle(&C.style);
        MPNet_Log("card: style design %u (%s) hue %u accent %u frame %u panel %u holo %u", C.style.design,
                  DesignName(C.style.design), C.style.hue, C.style.accent, C.style.frame, C.style.panel, C.style.holo);
    }
    C.styleDirty = FALSE;
    RefreshOwn();
    C.mode = MODE_CARD;
}

static void Change(s8 dir, bool8 repeat)
{
    struct MpCardStyle *s = &C.style;
    u8 n;
    switch (C.row)
    {
    case ROW_DESIGN:
        n = sPassCount ? sPassCount : 1;
        s->design = (s->design + n + dir) % n;
        break;
    case ROW_HUE:
        if (!Recolorable(s->design))
        {
            if (!repeat)
                PlaySE(SE_FAILURE);
            return;
        }
        s->hue = (s->hue + 360 + dir * (repeat ? 6 : 15)) % 360;
        break;
    case ROW_ACCENT: s->accent = (s->accent + TCARD_ACCENTS + dir) % TCARD_ACCENTS; break;
    case ROW_FRAME: s->frame = (s->frame + TCARD_FRAME_COUNT + dir) % TCARD_FRAME_COUNT; break;
    case ROW_PANEL: s->panel = (s->panel + TCARD_PANEL_COUNT + dir) % TCARD_PANEL_COUNT; break;
    case ROW_HOLO: s->holo = !s->holo; break;
    case ROW_SHOWCASE:
    {
        // the next POKéMON of the party (or none)
        s8 at = ShowcaseSlot(), k;
        for (k = 0; k < PARTY_SIZE + 1; k++)
        {
            at += dir;
            if (at >= PARTY_SIZE)
                at = -1;
            if (at < -1)
                at = PARTY_SIZE - 1;
            if (at == -1 || (GetMonData(&gPlayerParty[at], MON_DATA_SPECIES) != SPECIES_NONE
                          && !GetMonData(&gPlayerParty[at], MON_DATA_IS_EGG)))
                break;
        }
        MP_TCard_SetShowcaseFromParty(at);
        RefreshOwn();
        break;
    }
    case ROW_TITLE:
    {
        u16 total = MP_Achieve_Total(), t = MP_TCard_Title(), k;
        for (k = 0; k <= total; k++)
        {
            t = (t + total + 1 + dir) % (total + 1);
            if (t == 0 || MP_Achieve_IsDone(t - 1))
                break;
        }
        MP_TCard_SetTitle(t);
        RefreshOwn();
        break;
    }
    case ROW_SHOWTITLE:
        MP_TCard_SetShowTitle(!MP_TCard_ShowTitle());
        RefreshOwn();
        break;
    case ROW_TEAM:
        if (MP_TCard_TeamCount())
            MP_TCard_SetTeamFromParty(NULL, 0);
        else
        {
            static const u8 sAll[PARTY_SIZE] = {0, 1, 2, 3, 4, 5};
            MP_TCard_SetTeamFromParty(sAll, PARTY_SIZE);
        }
        RefreshOwn();
        break;
    default:
        return;
    }
    if (C.row <= ROW_HOLO)
    {
        C.styleDirty = TRUE;
        C.view.style = C.style;
    }
    PlaySE(SE_SELECT);
}

static void MyCardStep(u16 keys, u16 pressed)
{
    u16 dirs = keys & (DPAD_LEFT | DPAD_RIGHT | DPAD_UP | DPAD_DOWN);
    bool8 repeat = dirs && C.heldFrames >= 18 && (C.heldFrames % 3) == 0;
    if (C.mode == MODE_ALBUM)
    {
        u8 n = MP_TCard_AlbumCount();
        if (pressed & B_BUTTON)
            C.mode = MODE_MYCARD;
        else if (((pressed | (repeat ? dirs : 0)) & DPAD_UP) && n)
            C.albumAt = (C.albumAt + n - 1) % n;
        else if (((pressed | (repeat ? dirs : 0)) & DPAD_DOWN) && n)
            C.albumAt = (C.albumAt + 1) % n;
        else if ((pressed & A_BUTTON) && n)
        {
            const struct MpTCardFriend *f = MP_TCard_AlbumFriend(C.albumAt);
            C.ownBackup = C.view;
            C.view = f->card;
            C.view.own = FALSE;
            if (C.pic)
                SDL_DestroyTexture(C.pic);
            C.pic = NULL;
            C.page = PAGE_PROFILE;
            C.mode = MODE_CARD;
            C.fromAlbum = TRUE;
            MPNet_Log("card: a friend's card from the album");
        }
        else
            return;
        PlaySE(SE_SELECT);
        return;
    }
    if (pressed & (B_BUTTON | START_BUTTON))
    {
        LeaveMyCard();
        PlaySE(SE_SELECT);
        return;
    }
    if ((pressed | (repeat ? dirs : 0)) & DPAD_UP)
    {
        C.row = (C.row + ROW_COUNT - 1) % ROW_COUNT;
        PlaySE(SE_SELECT);
    }
    else if ((pressed | (repeat ? dirs : 0)) & DPAD_DOWN)
    {
        C.row = (C.row + 1) % ROW_COUNT;
        PlaySE(SE_SELECT);
    }
    else if (C.row == ROW_SIGN)
    {
        if (pressed & A_BUTTON)
        {
            bool8 has;
            memcpy(C.sig, MP_TCard_Signature(&has), sizeof(C.sig));
            if (!has)
                memset(C.sig, 0, sizeof(C.sig));
            C.penX = TCARD_SIG_W / 2, C.penY = TCARD_SIG_H / 2;
            C.penDown = C.eraser = C.sigChanged = FALSE;
            C.mode = MODE_SIGN;
            PlaySE(SE_SELECT);
        }
    }
    else if (C.row == ROW_ALBUM)
    {
        if (pressed & A_BUTTON)
        {
            C.mode = MODE_ALBUM;
            C.albumAt = 0;
            PlaySE(SE_SELECT);
        }
    }
    else if ((pressed | (repeat ? dirs : 0)) & (DPAD_LEFT | DPAD_RIGHT))
        Change(((pressed | dirs) & DPAD_LEFT) ? -1 : 1, !(pressed & (DPAD_LEFT | DPAD_RIGHT)));
    else if (pressed & A_BUTTON)
        Change(1, FALSE);
}

// ----------------------------------------------------------------------------------------------------- the screen

SDL_Texture *MP_CardScreen_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!C.active && !C.linger)
        return NULL;
    G_Begin(r);
    if (C.owner != r)
    {
        ForgetTextures();
        C.owner = r;
    }
    ReadPasses();
    if (!C.canvas)
    {
        C.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CS_W, CS_H);
        C.card = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CARD_W, CARD_H);
        C.text = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CARD_W, 48);
        if (!C.canvas || !C.card || !C.text)
            return NULL;
        SDL_SetTextureBlendMode(C.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(C.card, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(C.card, SDL_ScaleModeLinear);
        SDL_SetTextureBlendMode(C.text, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(C.text, SDL_ScaleModeLinear);
    }
    MakeLooks(r);
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    if (!C.active)
    {
        // closed: black until the game's card has gone (it fades out underneath)
        u8 c = C.white ? 255 : 0;
        SDL_SetRenderTarget(r, C.canvas);
        SDL_SetRenderDrawColor(r, c, c, c, 255);
        SDL_RenderClear(r);
    }
    else
    {
        MP_BadgeCase_Tick();
        if (C.mode == MODE_SIGN)
            DrawSign(r);
        else if (C.mode == MODE_MYCARD || C.mode == MODE_ALBUM)
            DrawMyCard(r);
        else
            DrawCardView(r);
        if (C.fadeIn || C.closing)
        {
            float k = C.closing ? 1.0f - (float)C.closing / FADE_FRAMES : (float)C.fadeIn / FADE_FRAMES;
            SDL_RenderSetScale(r, 1, 1);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            u8 c = C.white && C.closing ? 255 : 0;
            SDL_SetRenderDrawColor(r, c, c, c, (u8)(G_MinF(1, k) * 255));
            SDL_RenderFillRect(r, NULL);
        }
    }
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    {
        void MP_CardScreen_FrameDrawn(void);
        MP_CardScreen_FrameDrawn();
    }
    return C.canvas;
}

bool8 MP_CardScreen_IsActive(void) { return C.active; }

// The TRAINER CARD over the game's: the player's own (friendView NULL) or a friend's; pic: the game card's picture
// of them (NULL: none)
void MP_CardScreen_Open(const struct MpCardView *friendView, const u32 *pic, bool8 fadeWhite)
{
    C.white = fadeWhite;
    ReadPasses();
    if (friendView)
        C.view = *friendView;
    else
        MP_CardView_FillOwn(&C.view);
    if (pic)
    {
        memcpy(C.view.pic, pic, sizeof(C.view.pic));
        C.view.hasPic = TRUE;
    }
    if (C.pic)
        SDL_DestroyTexture(C.pic);
    C.pic = NULL;
    C.active = TRUE;
    C.linger = FALSE;
    C.mode = MODE_CARD;
    C.page = C.prevPage = PAGE_PROFILE;
    C.flip = C.closing = 0;
    C.fadeIn = FADE_FRAMES;
    C.fromAlbum = FALSE;
    C.styleDirty = FALSE;
    C.result = 0xFF;
    C.prevKeys = 0xFFFF;
    C.regionRow = 0;
    {
        u8 k, here = MP_Region_Current();
        for (k = 0; k < REGION_COUNT; k++)
            if (sRegionOrder[k] == here)
                C.regionRow = k;
    }
    C.tiltX = C.tiltY = 0;
    PlaySE(SE_RG_CARD_OPEN);
    MPNet_Log("card: %s card shown (design %u %s)", friendView ? "a friend's" : "own", C.view.style.design,
              DesignName(C.view.style.design));
}

// the game's card has closed: the screen goes back to the game
void MP_CardScreen_Hide(void)
{
    C.linger = FALSE;
}

u8 MP_CardScreen_TakeResult(void)
{
    u8 r = C.result;
    C.result = 0xFF;
    return r;
}

static void FlipTo(u8 page, s8 dir)
{
    if (page == C.page)
        return;
    C.prevPage = C.page;
    C.page = page;
    C.flipDir = dir;
    C.flip = FLIP_FRAMES;
    PlaySE(SE_RG_CARD_FLIP);
    Gamepad_Rumble(25, 45);
    MPNet_Log("card: page %s", sPageNames[page]);
}

// The card turns only when the player turns it: the right stick, or the mouse pointing at it (once it has moved);
// let go (the stick back, the pointer off the card) and it settles flat
static void Tilt(void)
{
    float lx, ly, rx, ry, tx = 0, ty = 0;
    s32 mx = -1, my = -1;
    bool8 down;
    bool8 moved = C.mode != MODE_SIGN && Platform_PoketchMouse(&mx, &my, &down);
    Gamepad_Sticks(&lx, &ly, &rx, &ry);
    if (moved)
        C.mouseAt = 1;
    if (sStill)
        ;
    else if (rx * rx + ry * ry > 0.02f)
        tx = rx, ty = ry;
    else if (C.mouseAt && C.mode == MODE_CARD)
    {
        s32 x = -1, y = -1;   // (left as they are when the pointer is off the window)
        if (!moved)
            Platform_PoketchMouse(&x, &y, &down), mx = x, my = y;
        if (mx >= 6 && mx < 234 && my >= 4 && my < 140)   // (on the card)
        {
            tx = G_MaxF(-1, G_MinF(1, (mx - 120) / 114.0f));
            ty = G_MaxF(-1, G_MinF(1, (my - 71) / 67.0f));
        }
    }
    C.tiltX += (tx - C.tiltX) * 0.15f;
    C.tiltY += (ty - C.tiltY) * 0.15f;
    if (SDL_fabsf(C.tiltX) < 0.001f && tx == 0) C.tiltX = 0;
    if (SDL_fabsf(C.tiltY) < 0.001f && ty == 0) C.tiltY = 0;
}

bool8 MP_CardScreen_Step(u16 keys)
{
    u16 pressed;
    if (C.draining)
    {
        if (keys & (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON | L_BUTTON | R_BUTTON))
            return TRUE;
        C.draining = FALSE;
    }
    if (!C.active)
        return FALSE;
    if (MP_BadgeCase_IsActive())
    {
        C.prevKeys = 0xFFFF;   // (the case's buttons aren't the card's when it closes)
        return FALSE;
    }
    C.age++;
    if (C.fadeIn)
        C.fadeIn--;
    if (C.flip)
        C.flip--;
    if (keys & (DPAD_LEFT | DPAD_RIGHT | DPAD_UP | DPAD_DOWN))
        C.heldFrames++;
    else
        C.heldFrames = 0;
    pressed = keys & ~C.prevKeys;
    C.prevKeys = keys;
    if (C.mode != MODE_SIGN)
        Tilt();
    if (C.closing)
    {
        if (--C.closing == 0)
        {
            C.active = FALSE;
            C.linger = TRUE;
            C.draining = TRUE;
            C.result = 0xFE;
        }
        return TRUE;
    }
    if (C.mode == MODE_SIGN)
    {
        SignStep(keys, pressed);
        return TRUE;
    }
    if (C.mode != MODE_CARD)
    {
        MyCardStep(keys, pressed);
        return TRUE;
    }
    if (C.flip > FLIP_FRAMES / 2)
        return TRUE;
    if (pressed & (A_BUTTON | R_BUTTON | DPAD_RIGHT))
        FlipTo((C.page + 1) % PageCount(), 1);
    else if (pressed & (L_BUTTON | DPAD_LEFT))
        FlipTo((C.page + PageCount() - 1) % PageCount(), -1);
    else if ((pressed & (DPAD_UP | DPAD_DOWN)) && C.page == PAGE_BADGES)
    {
        C.regionRow = (C.regionRow + ((pressed & DPAD_UP) ? REGION_COUNT - 1 : 1)) % REGION_COUNT;
        PlaySE(SE_SELECT);
    }
    else if (pressed & SELECT_BUTTON)
    {
        u8 region = sRegionOrder[C.regionRow];
        if (C.view.own)
            MP_BadgeCase_Open(region);
        else
            MP_BadgeCase_OpenView(&C.view, region);
    }
    else if ((pressed & START_BUTTON) && C.view.own)
    {
        C.style = C.view.style;
        C.styleDirty = FALSE;
        C.mode = MODE_MYCARD;
        C.row = 0;
        PlaySE(SE_WIN_OPEN);
    }
    else if ((pressed & B_BUTTON) && C.fromAlbum)
    {
        C.fromAlbum = FALSE;
        C.view = C.ownBackup;
        if (C.pic)
            SDL_DestroyTexture(C.pic);
        C.pic = NULL;
        C.page = PAGE_PROFILE;
        C.mode = MODE_ALBUM;
        PlaySE(SE_SELECT);
    }
    else if (pressed & B_BUTTON)
    {
        C.closing = FADE_FRAMES;
        PlaySE(SE_RG_CARD_FLIPPING);
    }
    return TRUE;
}

// ----------------------------------------------------------------------------------------------------- spectating

static u16 sFrame[240 * 160];
static bool8 sFrameRead;

void MP_CardScreen_FrameDrawn(void)
{
    sFrameRead = FALSE;
}

const u16 *MP_CardScreen_Frame(void)
{
    if ((!C.active && !C.linger) || !C.canvas || !C.owner)
        return NULL;
    if (!sFrameRead)
        sFrameRead = MP_CardScreen_Halve(C.owner, C.canvas, sFrame) != NULL;
    return sFrameRead ? sFrame : NULL;
}

// tests: the card's state in the log
void MP_CardScreen_Debug(void)
{
    MPNet_Log("card: screen active %u mode %u page %u/%u design %u hue %u accent %u frame %u panel %u holo %u stars %u%u%u%u",
              C.active, C.mode, C.page, PageCount(), C.view.style.design, C.view.style.hue, C.view.style.accent,
              C.view.style.frame, C.view.style.panel, C.view.style.holo, MP_CardView_Stars(&C.view, 0),
              MP_CardView_Stars(&C.view, 1), MP_CardView_Stars(&C.view, 2), MP_CardView_Stars(&C.view, 3));
}

// tests: cardscreen DESIGN [PAGE [PANEL [ACCENT [FRAME]]]]: the card shown as if styled so (not saved)
void MP_CardScreen_DebugSet(const char *arg)
{
    int design = -1, page = -1, panel = -1, accent = -1, frame = -1;
    sscanf(arg, "%d %d %d %d %d", &design, &page, &panel, &accent, &frame);
    if (design >= 0 && design < MP_CardScreen_DesignCount())
        C.view.style.design = design;
    if (page >= 0 && page < PageCount())
        C.page = C.prevPage = page, C.flip = 0;
    if (panel >= 0 && panel < TCARD_PANEL_COUNT)
        C.view.style.panel = panel;
    if (accent >= 0 && accent < TCARD_ACCENTS)
        C.view.style.accent = accent;
    if (frame >= 0 && frame < TCARD_FRAME_COUNT)
        C.view.style.frame = frame;
    C.tiltX = C.tiltY = 0;
    sStill = TRUE;
}
