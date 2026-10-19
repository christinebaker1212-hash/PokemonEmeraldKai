// Moemon Co-op: trainers made in LOOK > CREATE from the trainer customization
// kit's parts (body, hair, top, bottom, hat, bag). A look is a small recipe
// (struct MpCreated); every player's game builds the pictures from it:
//   overworld  32x32 walk, run and Mach Bike frames (one 15-color palette)
//   ID picture 64x64 (the kit's 80x80 front, fitted without cutting feet or head)
//   back       64x64, Emerald's 4 throw frames
// Parts data: graphics/moemon/creator/parts.bin (tools/moemon_creator/build.py).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"

enum { CR_VIEW_FRONT, CR_VIEW_BACK, CR_VIEW_WALK, CR_VIEW_RUN, CR_VIEW_BIKE, CR_VIEW_COUNT };
#define CR_CAT_COUNT MP_CR_CAT_COUNT

struct CrStyle
{
    const char *name;
    u16 firstColor;
    u8 nColors;
};

struct CrColor
{
    const char *name;
    u16 img[CR_VIEW_COUNT];   // 0xFFFF: not drawn in that view (bags on the pictures)
};

#include "mp_creator_data.h"

static const u8 sCrParts[] = INCBIN_U8("graphics/moemon/creator/parts.bin");

#define FRONT_W 80
#define FRONT_H 80
#define BACK_FW 87     // one back frame (4 side by side)
#define BACK_H 98
#define OW_W (9 * 32)
#define OW_H 32
#define FRONT_X0 8     // 64x64 window in the 80x80 front
#define BACK_X0 4      // and in an 87-wide back frame

// ---------------------------------------------------------------------------
// Recipes

void MP_Created_Pack(const struct MpCreated *r, u8 *p)
{
    p[0] = r->made; p[1] = r->girl; p[2] = r->skin;
    p[3] = r->hair; p[4] = r->hairColor;
    p[5] = r->top; p[6] = r->topColor;
    p[7] = r->bottom; p[8] = r->bottomColor;
    p[9] = r->hat; p[10] = r->hatColor;
    p[11] = r->bag; p[12] = r->bagColor;
}

void MP_Created_Unpack(const u8 *p, struct MpCreated *r)
{
    r->made = p[0]; r->girl = p[1] != 0; r->skin = p[2];
    r->hair = p[3]; r->hairColor = p[4];
    r->top = p[5]; r->topColor = p[6];
    r->bottom = p[7]; r->bottomColor = p[8];
    r->hat = p[9]; r->hatColor = p[10];
    r->bag = p[11]; r->bagColor = p[12];
    MP_Created_Clamp(r);
}

void MP_Created_ToHex(const struct MpCreated *r, char *out)
{
    u8 p[MP_CREATED_BYTES];
    int i;
    MP_Created_Pack(r, p);
    for (i = 0; i < MP_CREATED_BYTES; i++)
        sprintf(out + i * 2, "%02x", p[i]);
}

bool8 MP_Created_FromHex(const char *s, struct MpCreated *r)
{
    u8 p[MP_CREATED_BYTES];
    int i;
    unsigned v;
    for (i = 0; i < MP_CREATED_BYTES; i++)
    {
        if (sscanf(s + i * 2, "%2x", &v) != 1)
            return FALSE;
        p[i] = v;
    }
    MP_Created_Unpack(p, r);
    return TRUE;
}

static const struct CrStyle *Style(bool8 girl, u8 cat, u8 style)
{
    return &sCrStyles[sCrCatStyles[girl ? 1 : 0][cat][0] + style];
}

u8 MP_Created_StyleCount(bool8 girl, u8 cat)
{
    return sCrCatStyles[girl ? 1 : 0][cat][1];
}

const char *MP_Created_StyleName(bool8 girl, u8 cat, u8 style)
{
    // The kit calls them Hair 1-4; its clothes list says what they are
    static const char *const sHairNames[2][4] = {
        {"SPIKY", "SHORT", "MEDIUM", "SLICKED BACK"},
        {"LONG", "SHORT", "MEDIUM", "PONYTAIL"},
    };
    if (style >= MP_Created_StyleCount(girl, cat))
        return "?";
    if (cat == MP_CR_HAIR && style < 4)
        return sHairNames[girl ? 1 : 0][style];
    return Style(girl, cat, style)->name;
}

u8 MP_Created_ColorCount(bool8 girl, u8 cat, u8 style)
{
    return style < MP_Created_StyleCount(girl, cat) ? Style(girl, cat, style)->nColors : 0;
}

const char *MP_Created_ColorName(bool8 girl, u8 cat, u8 style, u8 color)
{
    const struct CrStyle *st;
    if (style >= MP_Created_StyleCount(girl, cat))
        return "";
    st = Style(girl, cat, style);
    return color < st->nColors ? sCrColors[st->firstColor + color].name : "";
}

const char *MP_Created_SkinName(u8 skin)
{
    return skin < MP_CR_SKIN_COUNT ? sCrSkinNames[skin] : "?";
}

static void ClampPart(bool8 girl, u8 cat, u8 *style, u8 *color, bool8 optional)
{
    u8 n = MP_Created_StyleCount(girl, cat);
    if (optional)
    {
        if (*style > n)
            *style = 0;
        if (*style == 0)
        {
            *color = 0;
            return;
        }
        if (*color >= MP_Created_ColorCount(girl, cat, *style - 1))
            *color = 0;
        return;
    }
    if (*style >= n)
        *style = 0;
    if (*color >= MP_Created_ColorCount(girl, cat, *style))
        *color = 0;
}

void MP_Created_Clamp(struct MpCreated *r)
{
    r->girl = r->girl != 0;
    if (r->skin >= MP_CR_SKIN_COUNT)
        r->skin = 1;
    ClampPart(r->girl, MP_CR_HAIR, &r->hair, &r->hairColor, FALSE);
    ClampPart(r->girl, MP_CR_TOP, &r->top, &r->topColor, FALSE);
    ClampPart(r->girl, MP_CR_BOTTOM, &r->bottom, &r->bottomColor, FALSE);
    ClampPart(r->girl, MP_CR_HAT, &r->hat, &r->hatColor, TRUE);
    ClampPart(r->girl, MP_CR_BAG, &r->bag, &r->bagColor, TRUE);
}

void MP_Created_Default(struct MpCreated *r, bool8 girl)
{
    memset(r, 0, sizeof(*r));
    r->girl = girl;
    r->skin = 1;              // light
    r->hair = girl ? 3 : 1;   // ponytail / short
    r->hairColor = 1;         // brown
    r->top = 0;               // t-shirt
    r->topColor = 3;          // red
    r->bottom = 0;            // jeans
    r->bottomColor = 3;       // light blue
    MP_Created_Clamp(r);
}

// ---------------------------------------------------------------------------
// Drawing: canvases are RGB555 with bit 15 set where something is drawn

static void DrawImage(u16 *canvas, int cw, int ch, u16 img)
{
    const u8 *p;
    int w, h, n, i, pos = 0, total;
    const u8 *pal;
    if (img == 0xFFFF || img >= CR_IMG_COUNT)
        return;
    p = sCrParts + sCrImgOffset[img];
    w = p[0] | (p[1] << 8);
    h = p[2] | (p[3] << 8);
    n = p[4];
    pal = p + 6;
    p += 6 + n * 2;
    total = w * h;
    if (w != cw || h != ch)
        return;
    while (pos < total)
    {
        int count = p[0], c = p[1];
        p += 2;
        if (c == 0)
        {
            pos += count;
            continue;
        }
        {
            u16 color = (pal[(c - 1) * 2] | (pal[(c - 1) * 2 + 1] << 8)) | 0x8000;
            for (i = 0; i < count && pos < total; i++)
                canvas[pos++] = color;
        }
    }
}

static u16 PartImage(bool8 girl, u8 cat, u8 style, u8 color, u8 view)
{
    const struct CrStyle *st;
    if (style >= MP_Created_StyleCount(girl, cat))
        return 0xFFFF;
    st = Style(girl, cat, style);
    if (color >= st->nColors)
        color = 0;
    return sCrColors[st->firstColor + color].img[view];
}

static void Compose(const struct MpCreated *r, u8 view, u16 *canvas, int w, int h)
{
    bool8 girl = r->girl;
    memset(canvas, 0, w * h * 2);
    DrawImage(canvas, w, h, sCrBase[girl][r->skin][view]);
    DrawImage(canvas, w, h, PartImage(girl, MP_CR_BOTTOM, r->bottom, r->bottomColor, view));
    DrawImage(canvas, w, h, PartImage(girl, MP_CR_TOP, r->top, r->topColor, view));
    if (r->bag)
        DrawImage(canvas, w, h, PartImage(girl, MP_CR_BAG, r->bag - 1, r->bagColor, view));
    DrawImage(canvas, w, h, PartImage(girl, MP_CR_HAIR, r->hair, r->hairColor, view));
    if (r->hat)
        DrawImage(canvas, w, h, PartImage(girl, MP_CR_HAT, r->hat - 1, r->hatColor, view));
}

// ---------------------------------------------------------------------------
// 15 colors: merges the pair that costs least (fewest pixels times distance)
// until 15 are left; the more used color of a pair wins. Same as
// tools/moemon_creator/reduce.py.

#define MAX_COLORS 256

struct ColorSet
{
    u16 color[MAX_COLORS];
    u32 count[MAX_COLORS];
    u16 n;
};

static void CountColors(struct ColorSet *set, const u16 *canvas, int pixels)
{
    int i, j;
    for (i = 0; i < pixels; i++)
    {
        u16 c = canvas[i];
        if (!(c & 0x8000))
            continue;
        c &= 0x7FFF;
        for (j = 0; j < set->n; j++)
            if (set->color[j] == c)
                break;
        if (j == set->n)
        {
            if (set->n == MAX_COLORS)
                continue;
            set->color[j] = c;
            set->count[j] = 0;
            set->n++;
        }
        set->count[j]++;
    }
}

static u32 ColorDist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31), dg = ((a >> 5) & 31) - ((b >> 5) & 31), db = ((a >> 10) & 31) - ((b >> 10) & 31);
    return 3 * dr * dr + 4 * dg * dg + 2 * db * db;
}

// Fills map[i] (index into the final palette, 1-15) for every color of the set
static void ReduceColors(struct ColorSet *set, u16 *pal, u8 *map)
{
    u16 alive[MAX_COLORS], nAlive = set->n, i, j;
    u16 parent[MAX_COLORS];
    u32 count[MAX_COLORS];
    for (i = 0; i < set->n; i++)
    {
        alive[i] = i;
        parent[i] = i;
        count[i] = set->count[i];
    }
    while (nAlive > 15)
    {
        u32 best = 0xFFFFFFFF;
        u16 bi = 0, bj = 1, keep, drop;
        for (i = 0; i < nAlive; i++)
            for (j = i + 1; j < nAlive; j++)
            {
                u16 a = alive[i], b = alive[j];
                u32 c = (count[a] < count[b] ? count[a] : count[b]) * ColorDist(set->color[a], set->color[b]);
                if (c < best)
                {
                    best = c;
                    bi = i;
                    bj = j;
                }
            }
        keep = alive[bi];
        drop = alive[bj];
        if (count[drop] > count[keep])
        {
            u16 t = keep;
            keep = drop;
            drop = t;
            alive[bi] = keep;
        }
        count[keep] += count[drop];
        for (i = 0; i < set->n; i++)
            if (parent[i] == drop)
                parent[i] = keep;
        alive[bj] = alive[--nAlive];
    }
    memset(pal, 0, 32);
    for (i = 0; i < nAlive; i++)
        pal[i + 1] = set->color[alive[i]];
    for (i = 0; i < set->n; i++)
        for (j = 0; j < nAlive; j++)
            if (alive[j] == parent[i])
                map[i] = j + 1;
}

static u8 PaletteIndex(const struct ColorSet *set, const u8 *map, u16 c)
{
    u16 i;
    if (!(c & 0x8000))
        return 0;
    c &= 0x7FFF;
    for (i = 0; i < set->n; i++)
        if (set->color[i] == c)
            return map[i];
    return 0;
}

// One frame (w x h pixels at x0, y0 of a canvas) as GBA 4bpp tiles
static void ToTiles(u8 *dst, const u16 *canvas, int stride, int x0, int y0, int w, int h,
                    const struct ColorSet *set, const u8 *map)
{
    int tx, ty, y, x;
    for (ty = 0; ty < h / 8; ty++)
        for (tx = 0; tx < w / 8; tx++)
            for (y = 0; y < 8; y++)
                for (x = 0; x < 8; x += 2)
                {
                    const u16 *row = canvas + (y0 + ty * 8 + y) * stride + x0 + tx * 8 + x;
                    *dst++ = PaletteIndex(set, map, row[0]) | (PaletteIndex(set, map, row[1]) << 4);
                }
}

// ---------------------------------------------------------------------------
// Fitting the 80x80 front into 64x64: rows that look like the row above them
// are taken out of the legs (pants repeat a lot) until it's 64 tall, so the
// head and the feet both stay.

static u16 sFront[FRONT_W * FRONT_H];
static u16 sFrontFit[64 * 64];

static void FitFront(void)
{
    u8 keep[FRONT_H];
    int top = -1, bottom = -1, y, x, height, out;
    for (y = 0; y < FRONT_H; y++)
        for (x = 0; x < FRONT_W; x++)
            if (sFront[y * FRONT_W + x] & 0x8000)
            {
                if (top < 0)
                    top = y;
                bottom = y;
                break;
            }
    memset(sFrontFit, 0, sizeof(sFrontFit));
    if (top < 0)
        return;
    memset(keep, 0, sizeof(keep));
    for (y = top; y <= bottom; y++)
        keep[y] = TRUE;
    height = bottom - top + 1;
    while (height > 64)
    {
        int best = -1, bestDiff = 0x7FFFFFFF;
        int from = top + (bottom - top) * 55 / 100, to = bottom - 3;
        for (y = from; y <= to; y++)
        {
            int prev = y - 1, diff = 0;
            if (!keep[y])
                continue;
            while (prev > top && !keep[prev])
                prev--;
            for (x = 0; x < FRONT_W; x++)
                diff += sFront[y * FRONT_W + x] != sFront[prev * FRONT_W + x];
            if (diff < bestDiff)
            {
                bestDiff = diff;
                best = y;
            }
        }
        if (best < 0)
            best = top;   // nothing to take from the legs: the top goes
        keep[best] = FALSE;
        height--;
    }
    // Feet on the bottom row
    out = 63;
    for (y = bottom; y >= top && out >= 0; y--)
    {
        if (!keep[y])
            continue;
        memcpy(&sFrontFit[out * 64], &sFront[y * FRONT_W + FRONT_X0], 64 * 2);
        out--;
    }
}

// ---------------------------------------------------------------------------
// Cache: one built look per player (and the local player)

static struct MpCreatedGfx sGfx[MP_MAX_PLAYERS + 1];
static struct MpCreated sGfxFor[MP_MAX_PLAYERS + 1];
static bool8 sGfxValid[MP_MAX_PLAYERS + 1];
static u16 sOw[3][OW_W * OW_H];
static u16 sBack[BACK_FW * 4 * BACK_H];
static u16 sBackFit[4][64 * 64];

static void BuildOverworld(const struct MpCreated *r, struct MpCreatedGfx *g)
{
    struct ColorSet set;
    u8 map[MAX_COLORS];
    int v, f;
    memset(&set, 0, sizeof(set));
    for (v = 0; v < 3; v++)
    {
        Compose(r, CR_VIEW_WALK + v, sOw[v], OW_W, OW_H);
        CountColors(&set, sOw[v], OW_W * OW_H);
    }
    ReduceColors(&set, g->owPal, map);
    for (v = 0; v < 3; v++)
        for (f = 0; f < 9; f++)
            ToTiles(g->ow[v * 9 + f], sOw[v], OW_W, f * 32, 0, 32, 32, &set, map);
}

static void BuildFront(const struct MpCreated *r, struct MpCreatedGfx *g)
{
    struct ColorSet set;
    u8 map[MAX_COLORS];
    memset(&set, 0, sizeof(set));
    Compose(r, CR_VIEW_FRONT, sFront, FRONT_W, FRONT_H);
    FitFront();
    CountColors(&set, sFrontFit, 64 * 64);
    ReduceColors(&set, g->frontPal, map);
    ToTiles(g->front, sFrontFit, 64, 0, 0, 64, 64, &set, map);
}

static void BuildBack(const struct MpCreated *r, struct MpCreatedGfx *g)
{
    struct ColorSet set;
    u8 map[MAX_COLORS];
    int f, y, x, top = BACK_H, y0;
    memset(&set, 0, sizeof(set));
    Compose(r, CR_VIEW_BACK, sBack, BACK_FW * 4, BACK_H);
    // The window starts just above the hair or hat; the waist is cut, like the kit's art
    for (y = 0; y < BACK_H && top == BACK_H; y++)
        for (x = 0; x < BACK_FW * 4; x++)
            if (sBack[y * BACK_FW * 4 + x] & 0x8000)
            {
                top = y;
                break;
            }
    y0 = top > 0 ? top - 1 : 0;
    if (y0 + 64 > BACK_H)
        y0 = BACK_H - 64;
    for (f = 0; f < 4; f++)
    {
        for (y = 0; y < 64; y++)
            memcpy(&sBackFit[f][y * 64], &sBack[(y0 + y) * BACK_FW * 4 + f * BACK_FW + BACK_X0], 64 * 2);
        CountColors(&set, sBackFit[f], 64 * 64);
    }
    ReduceColors(&set, g->backPal, map);
    for (f = 0; f < 4; f++)
        ToTiles(g->back[f], sBackFit[f], 64, 0, 0, 64, 64, &set, map);
}

const struct MpCreatedGfx *MP_Created_Gfx(u8 cacheSlot, const struct MpCreated *r)
{
    struct MpCreated clean = *r;
    if (cacheSlot > MP_MAX_PLAYERS)
        cacheSlot = MP_MAX_PLAYERS;
    MP_Created_Clamp(&clean);
    clean.made = 1;
    if (sGfxValid[cacheSlot] && memcmp(&sGfxFor[cacheSlot], &clean, sizeof(clean)) == 0)
        return &sGfx[cacheSlot];
    BuildOverworld(&clean, &sGfx[cacheSlot]);
    BuildFront(&clean, &sGfx[cacheSlot]);
    BuildBack(&clean, &sGfx[cacheSlot]);
    sGfxFor[cacheSlot] = clean;
    sGfxValid[cacheSlot] = TRUE;
    return &sGfx[cacheSlot];
}
