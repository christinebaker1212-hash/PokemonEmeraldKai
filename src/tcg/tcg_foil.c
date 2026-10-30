/* Foil for rare cards, shared by the three card games: see tcg_foil.h. Everything here is added light
 * (G_FillAdd), so dark parts of the picture take a soft glow and light parts are left nearly as printed. */
#include <SDL2/SDL.h> // before global.h: its macros break the Windows intrinsics headers
#include <math.h>
#include "tcg_gfx.h"
#include "tcg_foil.h"

static unsigned Hash(unsigned a, unsigned b)
{
    unsigned h = a * 0x9e3779b1u ^ (b + 0x7f4a7c15u) * 0x85ebca6bu;
    h ^= h >> 15;
    h *= 0x2c1b3c6du;
    h ^= h >> 12;
    return h;
}

unsigned G_FoilHue(int degrees)
{
    /* a pastel wheel: never a full primary, so the prism tints the picture instead of painting it */
    float h = (float)(((degrees % 360) + 360) % 360) / 60.0f, f = h - (int)h;
    float r, g, b;
    switch ((int)h)
    {
    case 0: r = 1, g = f, b = 0; break;
    case 1: r = 1 - f, g = 1, b = 0; break;
    case 2: r = 0, g = 1, b = f; break;
    case 3: r = 0, g = 1 - f, b = 1; break;
    case 4: r = f, g = 0, b = 1; break;
    default: r = 1, g = 0, b = 1 - f; break;
    }
    r = 0.45f + 0.55f * r, g = 0.45f + 0.55f * g, b = 0.45f + 0.55f * b;
    return (unsigned)(r * 255) << 16 | (unsigned)(g * 255) << 8 | (unsigned)(b * 255);
}

/* The sheen: a soft band leaning to the right crosses the picture in a gentle ease, brightest midway, then rests */
static void Sheen(unsigned kind, int ax, int ay, int aw, int ah, unsigned seed, unsigned frame, float peak)
{
    int period = kind == FOIL_STAR ? 260 : 200, sweep = kind == FOIL_STAR ? 110 : 80;
    int t = (int)((frame + seed * 37u) % (unsigned)period), r, layer;
    float slope = 0.55f, bw, travel, u, p, env;
    static const float widths[3] = {1.0f, 0.55f, 0.22f}, alphas[3] = {0.30f, 0.38f, 0.45f};
    if (t >= sweep || aw < 4 || ah < 4)
        return;
    bw = aw / 5.0f;
    bw = bw < 3 ? 3 : bw > 11 ? 11 : bw;
    travel = aw + ah * slope + 2 * bw;
    u = (float)t / sweep;
    env = sinf(3.14159f * u);
    u = u * u * (3 - 2 * u);
    p = -bw + u * travel;
    for (r = 0; r < ah; r++)
    {
        float c = p - (ah - 1 - r) * slope;   /* (the lower rows trail: a band leaning right) */
        unsigned color;
        switch (kind)
        {
        case FOIL_GOLD: color = 0xffe0a0; break;
        case FOIL_PRISM: color = G_FoilHue(r * 6 + (int)(frame * 2) + (int)seed); break;
        case FOIL_COSMOS: color = G_Mix(G_FoilHue(r * 5 + (int)frame + (int)seed), 0xffffff, 0.45f); break;
        case FOIL_STAR: color = 0xfff0c8; break;
        default: color = 0xffffff; break;
        }
        for (layer = 0; layer < 3; layer++)
        {
            float half = bw * widths[layer];
            int x0 = (int)floorf(c - half + 0.5f), x1 = (int)floorf(c + half + 0.5f);
            if (x0 < 0)
                x0 = 0;
            if (x1 > aw)
                x1 = aw;
            if (x1 > x0)
                G_FillAdd(ax + x0, ay + r, x1 - x0, 1, color, (unsigned)(peak * env * alphas[layer]));
        }
    }
}

/* Twinkles: tiny four-point stars, each living a moment somewhere new in the picture */
static void Twinkles(int n, int ax, int ay, int aw, int ah, unsigned seed, unsigned frame, unsigned color, int big)
{
    int i;
    if (aw < 6 || ah < 6)
        return;
    for (i = 0; i < n; i++)
    {
        unsigned life = 44, cycle = 110 + Hash(seed, (unsigned)i) % 60, off = Hash(seed + 1, (unsigned)i) % cycle;
        unsigned tt = (frame + off) % cycle, k, h;
        float e;
        int px, py, arm;
        if (tt >= life)
            continue;
        e = sinf(3.14159f * tt / life);
        h = Hash(seed * 7u + (unsigned)i, (frame + off) / cycle);
        px = ax + 1 + (int)(h % (unsigned)(aw - 2));
        py = ay + 1 + (int)((h >> 12) % (unsigned)(ah - 2));
        G_FillAdd(px, py, 1, 1, color, (unsigned)(230 * e));
        arm = big && e > 0.55f ? 2 : 1;
        for (k = 1; k <= (unsigned)arm; k++)
        {
            unsigned a = (unsigned)(120 * e / k);
            if (px - (int)k >= ax)
                G_FillAdd(px - (int)k, py, 1, 1, color, a);
            if (px + (int)k < ax + aw)
                G_FillAdd(px + (int)k, py, 1, 1, color, a);
            if (py - (int)k >= ay)
                G_FillAdd(px, py - (int)k, 1, 1, color, a);
            if (py + (int)k < ay + ah)
                G_FillAdd(px, py + (int)k, 1, 1, color, a);
        }
    }
}

/* A glint along the name strip above the picture */
static void Glint(int x, int y, int w, int top, int bottom, unsigned seed, unsigned frame, unsigned color)
{
    int period = 220, sweep = 50, t = (int)((frame + seed * 53u) % (unsigned)period), r;
    float u, c;
    if (t >= sweep || bottom - top < 2)
        return;
    u = (float)t / sweep;
    c = -4 + u * (w + 8 + (bottom - top));
    for (r = top; r < bottom; r++)
    {
        float cr = c - (bottom - 1 - r) * 0.6f;
        int x0 = (int)cr - 1, x1 = (int)cr + 2;
        unsigned a = (unsigned)(150 * sinf(3.14159f * u));
        if (x0 < 1)
            x0 = 1;
        if (x1 > w - 1)
            x1 = w - 1;
        if (x1 > x0)
            G_FillAdd(x + x0, y + r, x1 - x0, 1, color, a);
    }
}

/* A light running round the edge of the card */
static void EdgePoint(int i, int w, int h, int *px, int *py)
{
    int n = 2 * (w + h) - 4;
    i = ((i % n) + n) % n;
    if (i < w)
        *px = i, *py = 0;
    else if (i < w + h - 1)
        *px = w - 1, *py = i - w + 1;
    else if (i < 2 * w + h - 2)
        *px = w - 1 - (i - (w + h - 1)) - 1, *py = h - 1;
    else
        *px = 0, *py = h - 1 - (i - (2 * w + h - 2)) - 1;
}
static void Edge(int x, int y, int w, int h, unsigned seed, unsigned frame, int prism, unsigned color)
{
    int head = (int)((frame * 3u) / 2u + seed * 29u), k, len = w >= 40 ? 10 : 6;
    for (k = 0; k < len; k++)
    {
        int px, py;
        EdgePoint(head - k, w, h, &px, &py);
        G_FillAdd(x + px, y + py, 1, 1, prism ? G_FoilHue(head * 4 - k * 12) : color, (unsigned)(170 * (len - k) / len));
    }
}

void G_Foil(unsigned kind, int x, int y, int w, int h, int ax, int ay, int aw, int ah, unsigned seed, unsigned frame)
{
    float small = w < 40 ? 0.8f : 1.0f;
    int big = w >= 40;
    switch (kind)
    {
    case FOIL_GLINT:
        Glint(x, y, w, 1, ay - y, seed, frame, 0xfff8e0);
        break;
    case FOIL_SILVER:
        Sheen(kind, ax, ay, aw, ah, seed, frame, 90 * small);
        Twinkles(2, ax, ay, aw, ah, seed, frame, 0xffffff, big);
        break;
    case FOIL_GOLD:
        Sheen(kind, ax, ay, aw, ah, seed, frame, 90 * small);
        Twinkles(2, ax, ay, aw, ah, seed, frame, 0xfff0c0, big);
        Glint(x, y, w, 1, ay - y, seed + 3, frame, 0xffe8a0);
        Edge(x, y, w, h, seed, frame, 0, 0xffe090);
        break;
    case FOIL_PRISM:
        Sheen(kind, ax, ay, aw, ah, seed, frame, 100 * small);
        Twinkles(3, ax, ay, aw, ah, seed, frame, 0xffffff, big);
        Glint(x, y, w, 1, ay - y, seed + 3, frame, 0xfff8ff);
        Edge(x, y, w, h, seed, frame, 1, 0);
        break;
    case FOIL_COSMOS:
        Sheen(kind, ax, ay, aw, ah, seed, frame, 85 * small);
        Twinkles(big ? 6 : 3, ax, ay, aw, ah, seed, frame, 0xffffff, big);
        break;
    case FOIL_STAR:
        Sheen(kind, ax, ay, aw, ah, seed, frame, 70 * small);
        Twinkles(big ? 4 : 2, ax, ay, aw, ah, seed, frame, 0xfff4d0, big);
        break;
    default:
        break;
    }
}

/* A soft glow around a card just revealed: rings of light fading outward, breathing slowly */
void G_FoilHalo(int x, int y, int w, int h, unsigned color, unsigned frame, int prism)
{
    float breath = 0.75f + 0.25f * sinf(frame * 0.08f);
    int k;
    for (k = 1; k <= 4; k++)
    {
        unsigned c = prism ? G_FoilHue((int)(frame * 3) + k * 25) : color;
        unsigned a = (unsigned)(breath * (150 - k * 32));
        /* a rounded ring k pixels out */
        G_FillAdd(x - k + 1, y - k, w + 2 * k - 2, 1, c, a);
        G_FillAdd(x - k + 1, y + h + k - 1, w + 2 * k - 2, 1, c, a);
        G_FillAdd(x - k, y - k + 1, 1, h + 2 * k - 2, c, a);
        G_FillAdd(x + w + k - 1, y - k + 1, 1, h + 2 * k - 2, c, a);
    }
}
