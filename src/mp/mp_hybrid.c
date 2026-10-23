// Moemon Co-op: action sprites (bikes, surfing, field moves, fishing, watering, decorating) for
// looks that have none of their own: NPC sprites, the Ruby/Sapphire pair and LOOK > CREATE trainers.
// The body is Brendan's (May's for a girl) for that action, its clothes taking the look's colors, and
// the head is the look's own, from its standing frame facing the same way. Where Brendan's head sits
// in each action frame, and which pixels are his head, comes from tools/moemon_hybrid/build.py.
#include <string.h>
#include "global.h"
#include "mp_hybrid.h"

struct MpHyFrame
{
    u8 dir;       // 0 S, 1 N, 2 W, 3 E
    s8 dx, dy;    // where a 16x32 walking frame's head goes in the 32x32 canvas
    u32 mask[32]; // Brendan's head pixels in this frame (bit x of row y)
};

struct MpHyState
{
    const struct MpHyFrame *frames;
    u8 count;
    u8 width;     // 32, or 16 (decorating: centered in the 32-wide canvas)
};

struct MpHyBody
{
    u8 group[4][4];   // clothing colors, light to dark, biggest group first
    u8 size[4];
    u8 base[4];       // the shade most of each group is drawn in
    u8 count;
};

#include "mp_hybrid_data.h"

#define TEMPLATE_NECK 23   // first body row of a 16x32 walking frame
#define CANVAS 32
#define HEAD_X 8           // head canvas column of a walking frame's column 0

static const struct MpHyState *StateData(bool8 female, u8 state)
{
    const struct MpHyState *s;
    if (state >= ARRAY_COUNT(sHyStates_brendan))
        return NULL;
    s = female ? &sHyStates_may[state] : &sHyStates_brendan[state];
    return s->frames ? s : NULL;
}

u8 MP_Hybrid_FrameCount(bool8 female, u8 state)
{
    const struct MpHyState *s = StateData(female, state);
    return s ? s->count : 0;
}

u8 MP_Hybrid_FrameDir(bool8 female, u8 state, u8 frame)
{
    const struct MpHyState *s = StateData(female, state);
    return (s && frame < s->count) ? s->frames[frame].dir : 0xFF;
}

static u8 GetPixel(const u8 *tiles, int w, int x, int y)
{
    u8 b = tiles[((y / 8) * (w / 8) + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
    return (x & 1) ? b >> 4 : b & 0xF;
}

static void PutTiles(u8 *dst, const u8 *canvas, int w, int x0)
{
    int x, y;
    memset(dst, 0, w * CANVAS / 2);
    for (y = 0; y < CANVAS; y++)
        for (x = 0; x < w; x++)
        {
            u8 c = canvas[y * CANVAS + x0 + x];
            u8 *b = &dst[((y / 8) * (w / 8) + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            *b |= (x & 1) ? c << 4 : c;
        }
}

static int Lum(u16 c)
{
    // 0..255
    int r = (c & 31) * 255 / 31, g = ((c >> 5) & 31) * 255 / 31, b = ((c >> 10) & 31) * 255 / 31;
    return (r * 30 + g * 59 + b * 11) / 100;
}

// Hue 0..359 (-1 = gray), saturation 0..255
static void HueSat(u16 c, int *h, int *s)
{
    int r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    int max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    int min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    int d = max - min;
    *s = max ? d * 255 / max : 0;
    if (d == 0)
        *h = -1;
    else if (max == r)
        *h = (60 * (g - b) / d + 360) % 360;
    else if (max == g)
        *h = 60 * (b - r) / d + 120;
    else
        *h = 60 * (r - g) / d + 240;
}

static u32 ColorDist(u16 a, u16 b)
{
    int dr = (a & 31) - (b & 31), dg = ((a >> 5) & 31) - ((b >> 5) & 31), db = ((a >> 10) & 31) - ((b >> 10) & 31);
    return dr * dr * 3 + dg * dg * 4 + db * db * 2;
}

static bool8 IsSkin(const struct MpHybridLook *look, u8 c)
{
    u8 i;
    for (i = 0; i < look->nSkin; i++)
        if (look->skin[i] == c)
            return TRUE;
    return FALSE;
}

// The look's clothing colors (below the neck), grouped by hue, biggest group first
struct Groups
{
    u8 group[8][15];
    u8 size[8];
    u8 base[8];
    u8 count;
};

static void FindGroups(const struct MpHybridLook *look, struct Groups *out)
{
    u32 counts[16];
    u8 order[15], n = 0, i, j, d;
    memset(counts, 0, sizeof(counts));
    memset(out, 0, sizeof(*out));
    for (d = 0; d < 3; d++)
    {
        int x, y;
        for (y = look->neck[d]; y < look->h; y++)
            for (x = 0; x < look->w; x++)
                counts[GetPixel(look->frame[d], look->w, x, y)]++;
    }
    for (i = 1; i < 16; i++)
        if (counts[i] && !IsSkin(look, i) && Lum(look->pal[i]) > 30)
            order[n++] = i;
    // most used first (ties: lower index)
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
            if (counts[order[j]] > counts[order[i]])
            {
                u8 t = order[i];
                order[i] = order[j];
                order[j] = t;
            }
    for (i = 0; i < n; i++)
    {
        int h, s;
        u8 g;
        HueSat(look->pal[order[i]], &h, &s);
        for (g = 0; g < out->count; g++)
        {
            int h2, s2, dh;
            HueSat(look->pal[out->group[g][0]], &h2, &s2);
            dh = (h < 0 || h2 < 0) ? 360 : (h > h2 ? h - h2 : h2 - h);
            if (dh > 180)
                dh = 360 - dh;
            if ((s < 46 && s2 < 46) || (s >= 46 && s2 >= 46 && dh < 29))
                break;
        }
        if (g == out->count)
        {
            if (out->count == ARRAY_COUNT(out->group))
                continue;
            out->count++;
        }
        out->group[g][out->size[g]++] = order[i];
    }
    // Groups by size; shades light to dark, the base the most used one
    for (i = 0; i < out->count; i++)
        for (j = i + 1; j < out->count; j++)
        {
            u32 ci = 0, cj = 0;
            u8 k;
            for (k = 0; k < out->size[i]; k++) ci += counts[out->group[i][k]];
            for (k = 0; k < out->size[j]; k++) cj += counts[out->group[j][k]];
            if (cj > ci)
            {
                u8 tmp[15], ts = out->size[i];
                memcpy(tmp, out->group[i], 15);
                memcpy(out->group[i], out->group[j], 15);
                out->size[i] = out->size[j];
                memcpy(out->group[j], tmp, 15);
                out->size[j] = ts;
            }
        }
    for (i = 0; i < out->count; i++)
    {
        u8 *g = out->group[i], k, best = 0;
        for (j = 0; j < out->size[i]; j++)
            for (k = j + 1; k < out->size[i]; k++)
                if (Lum(look->pal[g[k]]) > Lum(look->pal[g[j]]))
                {
                    u8 t = g[j];
                    g[j] = g[k];
                    g[k] = t;
                }
        for (j = 1; j < out->size[i]; j++)
            if (counts[g[j]] > counts[g[best]])
                best = j;
        out->base[i] = best;
    }
}

// Brendan's / May's palette index -> the look's
static void BodyLut(const struct MpHybridLook *look, bool8 female, const u16 *bodyPal, u8 *lut)
{
    const struct MpHyBody *body = female ? &sHyBody_may : &sHyBody_brendan;
    struct Groups g;
    u8 i, j;
    lut[0] = 0;
    for (i = 1; i < 16; i++)
    {
        u32 best = 0xFFFFFFFF;
        lut[i] = 1;
        for (j = 1; j < 16; j++)
        {
            u32 d = ColorDist(bodyPal[i], look->pal[j]);
            if (d < best)
            {
                best = d;
                lut[i] = j;
            }
        }
    }
    // Skin: colors 1-4 of both, lightest first
    if (look->nSkin)
        for (i = 0; i < 4; i++)
            lut[1 + i] = look->skin[look->nSkin == 1 ? 0 : (i * (look->nSkin - 1) * 2 + 3) / 6];
    FindGroups(look, &g);
    if (g.count == 0)
        return;
    for (i = 0; i < body->count; i++)
    {
        u8 k = i < g.count ? i : 0;
        for (j = 0; j < body->size[i]; j++)
        {
            int at = g.base[k] + j - body->base[i];
            if (at < 0)
                at = 0;
            if (at >= g.size[k])
                at = g.size[k] - 1;
            lut[body->group[i][j]] = g.group[k][at];
        }
    }
}

// The look's heads in walking-frame coordinates (column + HEAD_X), facing S, N, W, E
static u8 sHead[4][CANVAS][CANVAS];

static void CutHeads(const struct MpHybridLook *look)
{
    int d, x, y, ox = (look->w - 16) / 2;
    memset(sHead, 0, sizeof(sHead));
    for (d = 0; d < 3; d++)
        for (y = 0; y < look->neck[d] && y < look->h; y++)
            for (x = 0; x < look->w; x++)
            {
                u8 c = GetPixel(look->frame[d], look->w, x, y);
                int tx = x - ox, ty = y + TEMPLATE_NECK - look->neck[d];
                if (c == 0 || ty < 0 || ty >= CANVAS || tx + HEAD_X < 0 || tx + HEAD_X >= CANVAS)
                    continue;
                if (look->kaiArms && d < 2 && ty >= 16 && ty <= 21 && (tx <= 2 || tx >= 13) && IsSkin(look, c))
                    continue;
                sHead[d][ty][tx + HEAD_X] = c;
            }
    // E: the W head mirrored around the walking frame's middle
    for (y = 0; y < CANVAS; y++)
        for (x = 0; x < CANVAS; x++)
        {
            int mx = 15 - (x - HEAD_X) + HEAD_X;
            if (mx >= 0 && mx < CANVAS)
                sHead[3][y][mx] = sHead[2][y][x];
        }
}

u8 MP_Hybrid_Compose(const struct MpHybridLook *look, bool8 female, u8 state, const u8 *bodyArt,
                     const u16 *bodyPal, u8 *out, u8 maxFrames, u32 *frameBytes)
{
    static u8 sCanvas[CANVAS * CANVAS];
    const struct MpHyState *s = StateData(female, state);
    u8 lut[16], f;
    int pad;

    if (!s || !look || !bodyArt || !bodyPal || s->count > maxFrames)
        return 0;
    BodyLut(look, female, bodyPal, lut);
    CutHeads(look);
    pad = (CANVAS - s->width) / 2;
    *frameBytes = s->width * CANVAS / 2;
    for (f = 0; f < s->count; f++)
    {
        const struct MpHyFrame *fr = &s->frames[f];
        const u8 *art = bodyArt + f * *frameBytes;
        int x, y;
        memset(sCanvas, 0, sizeof(sCanvas));
        for (y = 0; y < CANVAS; y++)
            for (x = 0; x < s->width; x++)
            {
                u8 c = GetPixel(art, s->width, x, y);
                if (c && !(fr->mask[y] & (1u << (x + pad))))
                    sCanvas[y * CANVAS + x + pad] = lut[c];
            }
        for (y = 0; y < CANVAS; y++)
            for (x = 0; x < CANVAS; x++)
            {
                u8 c = sHead[fr->dir][y][x];
                int X = x - HEAD_X + fr->dx, Y = y + fr->dy;
                if (c && X >= 0 && X < CANVAS && Y >= 0 && Y < CANVAS)
                    sCanvas[Y * CANVAS + X] = c;
            }
        PutTiles(out + f * *frameBytes, sCanvas, s->width, pad);
    }
    return s->count;
}
