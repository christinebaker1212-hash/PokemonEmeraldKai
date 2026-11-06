/* OPTIONS > DISPLAY > BATTLE ANIMS > MODERN: every battle animation (moves, status conditions, stat changes, weather,
 * held items...) in battles and contests, drawn over the Classic one in full color.
 *
 * The Classic animation script runs exactly as it always does: the same sprites, tasks, sounds and the same number of
 * frames, so a game on MODERN and a friend's on CLASSIC stay in step (this layer only reads the battle and draws over
 * the finished picture, never changing anything the battle or the link sees). What it adds:
 *  - the effect sprites glow in their own colors, tinted by the move's type, and leave fading trails behind them
 *    (the renderer marks their pixels: gMpOamFx / gMpFxLayer, gba_easy_draw.c);
 *  - at each hit (a hit splat, the target being shaken, a sound aimed at the target) a burst: a soft core of light, a
 *    shockwave ring, sparks and particles of the move's type (embers rise, droplets and rocks fall, leaves sway...),
 *    and for strong hits a small shake of the picture;
 *  - while a move charges, motes of its type drawn into the user;
 *  - stat changes: chevrons rising or falling through the Pokemon.
 * Gentle on the eyes: nothing flashes the whole screen, light stays local and fades over several frames, at most a
 * few bursts a second, and the shake never moves the picture more than 2 pixels.
 * Everything stops when the Classic animation ends. Tests: anim_modern (every move with MOEMON_BATTLE_ANIMS=modern).
 */
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_interface.h"
#include "contest.h"
#include "main.h"
#include "mp.h"
#include "pokemon.h"
#include "sprite.h"
#include "task.h"
#include "constants/battle_anim.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

extern const u8 *const gBattleAnims_Moves[];
extern const u8 *const gBattleAnims_General[];
extern const u8 *const gBattleAnims_StatusConditions[];

void AnimTask_ShakeMon(u8 taskId);
void AnimTask_ShakeMon2(u8 taskId);
void AnimTask_ShakeMonInPlace(u8 taskId);
void AnimTask_StatsChange(u8 taskId);
void AnimTask_ShakeTargetBasedOnMovePowerOrDmg(u8 taskId);
void AnimTask_ShakeTargetInPattern(u8 taskId);
void AnimTask_ShakeAndSinkMon(u8 taskId);
void AnimTask_HorizontalShake(u8 taskId);

#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define HW (W / 2)
#define HH (H / 2)

u8 gMpOamFx[256];          // per OAM entry: drawn by a battle animation's effect sprite (sprite.c fills it)
u16 *gMpFxLayer;           // while MODERN draws: the effect sprites' pixels (bit 15 = drawn), else NULL
static u16 sFxLayer[W * H];
extern u8 *gMpUiLayer;     // gba_easy_draw.c: the battle UI's pixels (BG0's text box and menus, the HP boxes)
static u8 sUiLayer[W * H];
static u16 sSnap[W * H];   // the picture before MODERN drew on it (the UI's pixels come back from it)

enum { KIND_NONE, KIND_MOVE, KIND_GENERAL, KIND_STATUS };
enum { HIT_SPLAT, HIT_SHAKE, HIT_SOUND, HIT_QUAKE };

#define MAX_BURSTS 10
struct Burst
{
    s16 x, y;
    u16 start;     // frame
    u8 strength;   // 0-255
    u8 kind;       // HIT_*
    u32 seed;
};

static struct
{
    u8 kind;
    u16 id;
    u8 type;
    u8 power;
    bool8 physical;
    bool8 statDown;
    u8 attacker, target;
    u16 frame;               // frames since it started
    u16 lastBurst, firstBurst;
    bool8 hadBurst;
    bool8 quaked;
    u16 nBurst[4];           // the audit (MOEMON_MODERN_AUDIT): hits by kind
    u32 fxPixels, fxPeak;    // effect-sprite pixels in all, and the most in one frame
    struct Burst bursts[MAX_BURSTS];
    u8 shake;                // frames of shake left
    u8 shakeAmp;
    u8 trail[HH][HW][3];     // fading afterimages of the effect sprites (0-255 a channel)
    u16 glow[HH][HW][3];     // scratch: the blurred glow
    u32 seed;
} sM;

struct Rgb { u8 r, g, b; };

// the move's own effect (below: "The move's own effect")
#include "../data/mp_modernanim_moves.h"

static struct
{
    u8 family, variant;
    struct Rgb c;
    u16 len;          // the Classic animation's length (frames; the audit's, or a guess)
    s16 hit;          // the (first) hit's frame: the audit's, the live one when it comes sooner
    s16 lastHit;
    bool8 synth;      // no hit in the Classic script: MODERN makes its own at H
    bool8 synthDone;
    u8 synthCount;    // MULTI: blows made so far
    s16 ax, ay, tx, ty;
} sF;
static void FamilyBegin(void);

// ---------------------------------------------------------------------------
// Colors


// A bright color per type (and the light at its core): what the move's energy looks like
static const struct Rgb sTypeColor[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL]   = {255, 208, 104},   // golden: white vanishes on the pale battle fields
    [TYPE_FIGHTING] = {255, 132,  64},
    [TYPE_FLYING]   = {128, 184, 255},
    [TYPE_POISON]   = {200,  96, 230},
    [TYPE_GROUND]   = {228, 184, 104},
    [TYPE_ROCK]     = {200, 168, 104},
    [TYPE_BUG]      = {176, 228,  64},
    [TYPE_GHOST]    = {150, 110, 240},
    [TYPE_STEEL]    = {160, 184, 220},
    [TYPE_MYSTERY]  = {255, 255, 255},
    [TYPE_FIRE]     = {255, 128,  32},
    [TYPE_WATER]    = { 72, 160, 255},
    [TYPE_GRASS]    = { 96, 224,  96},
    [TYPE_ELECTRIC] = {255, 232,  64},
    [TYPE_PSYCHIC]  = {255, 104, 184},
    [TYPE_ICE]      = {150, 236, 255},
    [TYPE_DRAGON]   = {128, 112, 255},
    [TYPE_DARK]     = {170,  70, 110},
    [TYPE_FAIRY]    = {255, 150, 220},   // Moemon Kai: FAIRY
};

static u8 Clamp8(int v) { return v < 0 ? 0 : (v > 255 ? 255 : v); }

static u32 Rand(u32 *s)
{
    *s = *s * 1103515245u + 12345u;
    return (*s >> 8) & 0xFFFF;
}

// The frame is the GBA picture: 15-bit BGR, 5 bits a channel
static inline void AddPx(u16 *frame, int x, int y, int r, int g, int b)
{
    u16 c;
    int cr, cg, cb;
    if ((unsigned)x >= W || (unsigned)y >= H)
        return;
    c = frame[y * W + x];
    cr = (c & 31) + (r >> 3);
    cg = ((c >> 5) & 31) + (g >> 3);
    cb = ((c >> 10) & 31) + (b >> 3);
    frame[y * W + x] = (c & 0x8000) | (cr > 31 ? 31 : cr) | (cg > 31 ? 31 : cg) << 5 | (cb > 31 ? 31 : cb) << 10;
}

// alpha 0-256 toward (r, g, b)
static inline void MixPx(u16 *frame, int x, int y, int r, int g, int b, int alpha)
{
    u16 c;
    int cr, cg, cb;
    if ((unsigned)x >= W || (unsigned)y >= H || alpha <= 0)
        return;
    if (alpha > 256)
        alpha = 256;
    c = frame[y * W + x];
    cr = (c & 31) * 8;
    cg = ((c >> 5) & 31) * 8;
    cb = ((c >> 10) & 31) * 8;
    cr += ((r - cr) * alpha) >> 8;
    cg += ((g - cg) * alpha) >> 8;
    cb += ((b - cb) * alpha) >> 8;
    frame[y * W + x] = (c & 0x8000) | (cr >> 3) | (cg >> 3) << 5 | (cb >> 3) << 10;
}

// A soft round light: full at the center, nothing past the radius (additive)
static void Light(u16 *frame, int cx, int cy, int radius, struct Rgb c, int strength)
{
    int x, y, r2 = radius * radius;
    if (radius <= 0 || strength <= 0)
        return;
    for (y = cy - radius; y <= cy + radius; y++)
        for (x = cx - radius; x <= cx + radius; x++)
        {
            int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy), k;
            if (d2 >= r2)
                continue;
            k = strength * (r2 - d2) / r2;             // a smooth falloff
            k = k * (r2 - d2) / r2;
            AddPx(frame, x, y, c.r * k >> 8, c.g * k >> 8, c.b * k >> 8);
            MixPx(frame, x, y, c.r, c.g, c.b, k / 4);   // (a touch of the color itself: light vanishes on pale fields)
        }
}

// A ring of `thickness` around (cx, cy), anti-aliased by distance; squash > 1: flattened (a wave along the ground)
static void RingSquashed(u16 *frame, int cx, int cy, int radius, int thickness, struct Rgb c, int alpha, int squash)
{
    int x, y, outer = radius + thickness + 1;
    if (alpha <= 0)
        return;
    for (y = cy - outer / squash - 1; y <= cy + outer / squash + 1; y++)
        for (x = cx - outer; x <= cx + outer; x++)
        {
            int d = (x - cx) * (x - cx) + (y - cy) * (y - cy) * squash * squash, dist16, edge;
            // distance in sixteenths of a pixel (integer square root)
            int lo = 0, hi = outer * 16 + 16;
            d *= 256;
            while (lo < hi)
            {
                int mid = (lo + hi + 1) / 2;
                if (mid * mid <= d)
                    lo = mid;
                else
                    hi = mid - 1;
            }
            dist16 = lo - radius * 16;
            if (dist16 < -8 || dist16 > thickness * 16 + 8)
                continue;
            edge = dist16 < 8 ? dist16 + 8 : (dist16 > thickness * 16 - 8 ? thickness * 16 + 8 - dist16 : 16);
            MixPx(frame, x, y, c.r, c.g, c.b, alpha * edge / 16);
        }
}

static void Ring(u16 *frame, int cx, int cy, int radius, int thickness, struct Rgb c, int alpha)
{
    RingSquashed(frame, cx, cy, radius, thickness, c, alpha, 1);
}

// A line from (x0, y0) to (x1, y1), its light fading toward the end
static void Spark(u16 *frame, int x0, int y0, int x1, int y1, struct Rgb c, int alpha)
{
    int dx = x1 - x0, dy = y1 - y0, steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy), i;
    if (steps == 0)
        steps = 1;
    for (i = 0; i <= steps; i++)
    {
        int x = x0 + dx * i / steps, y = y0 + dy * i / steps, a = alpha * (steps - i / 2) / steps;
        MixPx(frame, x, y, c.r, c.g, c.b, a);
        if (i < steps / 2)
            MixPx(frame, x, y, 255, 255, 255, a / 2);
    }
}

// ---------------------------------------------------------------------------
// The particles of each type

enum { P_DOT, P_EMBER, P_DROP, P_LEAF, P_BOLT, P_SHARD, P_ROCK, P_RINGLET, P_WISP, P_STAR, P_FEATHER, P_BUBBLE };

static const struct
{
    u8 shape;
    s8 gravity;   // sixteenths of a pixel per frame per frame (negative: rises)
    u8 count;
    u8 life;
} sTypeParticles[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL]   = {P_STAR,     2, 10, 18},
    [TYPE_FIGHTING] = {P_STAR,     3, 12, 16},
    [TYPE_FLYING]   = {P_FEATHER,  1,  9, 26},
    [TYPE_POISON]   = {P_BUBBLE,  -2, 10, 24},
    [TYPE_GROUND]   = {P_ROCK,     6, 12, 22},
    [TYPE_ROCK]     = {P_ROCK,     7, 12, 22},
    [TYPE_BUG]      = {P_DOT,      1, 14, 20},
    [TYPE_GHOST]    = {P_WISP,    -2,  9, 28},
    [TYPE_STEEL]    = {P_SHARD,    4, 12, 18},
    [TYPE_MYSTERY]  = {P_DOT,      0,  8, 16},
    [TYPE_FIRE]     = {P_EMBER,   -4, 14, 22},
    [TYPE_WATER]    = {P_DROP,     6, 14, 22},
    [TYPE_GRASS]    = {P_LEAF,     1, 10, 28},
    [TYPE_ELECTRIC] = {P_BOLT,     0, 10, 14},
    [TYPE_PSYCHIC]  = {P_RINGLET, -1,  8, 24},
    [TYPE_ICE]      = {P_SHARD,    3, 12, 24},
    [TYPE_DRAGON]   = {P_EMBER,   -2, 12, 24},
    [TYPE_DARK]     = {P_WISP,     0, 10, 22},
    [TYPE_FAIRY]    = {P_STAR,    -1, 11, 24},   // Moemon Kai: FAIRY (an empty entry divided by a zero life)
};

static void Particle(u16 *frame, int shape, int x, int y, int age, int life, struct Rgb c, u32 seed)
{
    int a = 230 * (life - age) / life, i;
    if (a <= 0)
        return;
    if (shape != P_RINGLET && shape != P_BUBBLE && shape != P_BOLT && shape != P_WISP)
    {
        // a darker rim so the particle reads on the pale battle fields as well as in caves
        MixPx(frame, x - 1, y - 1, c.r / 3, c.g / 3, c.b / 3, a / 3);
        MixPx(frame, x + 1, y + 1, c.r / 3, c.g / 3, c.b / 3, a / 3);
        MixPx(frame, x + 1, y - 1, c.r / 3, c.g / 3, c.b / 3, a / 4);
        MixPx(frame, x - 1, y + 1, c.r / 3, c.g / 3, c.b / 3, a / 4);
    }
    switch (shape)
    {
    case P_EMBER:   // a flame lick: bright core, warm halo, turning to smoke-dark red as it fades
        MixPx(frame, x, y, 255, 240, 160, a);
        MixPx(frame, x, y - 1, c.r, c.g, c.b, a * 3 / 4);
        MixPx(frame, x - 1, y, c.r, c.g / 2, 0, a / 2);
        MixPx(frame, x + 1, y, c.r, c.g / 2, 0, a / 2);
        MixPx(frame, x, y + 1, c.r, c.g / 3, 0, a / 3);
        break;
    case P_DROP:    // a droplet with a highlight
        MixPx(frame, x, y, c.r, c.g, c.b, a);
        MixPx(frame, x, y - 1, c.r, c.g, c.b, a * 2 / 3);
        MixPx(frame, x - 1, y + 1, c.r / 2, c.g / 2, c.b, a / 2);
        MixPx(frame, x + 1, y + 1, c.r / 2, c.g / 2, c.b, a / 2);
        MixPx(frame, x - 1, y, 255, 255, 255, a / 2);
        break;
    case P_LEAF:    // a small leaf, turning as it falls
        for (i = -2; i <= 2; i++)
        {
            int t = ((age + (seed & 15)) / 4) & 1;
            MixPx(frame, x + i, y + (t ? i / 2 : -i / 2), c.r / 2, c.g, c.b / 2, a * (3 - abs(i)) / 3);
        }
        MixPx(frame, x, y, 200, 255, 160, a / 2);
        break;
    case P_BOLT:    // a jagged spark
    {
        u32 s = seed + age;
        int px = x, py = y;
        for (i = 0; i < 5; i++)
        {
            int nx = px + (int)(Rand(&s) % 5) - 2, ny = py + 2;
            Spark(frame, px, py, nx, ny, c, a);
            px = nx, py = ny;
        }
        break;
    }
    case P_SHARD:   // a glinting diamond
        MixPx(frame, x, y, 255, 255, 255, a);
        MixPx(frame, x - 1, y, c.r, c.g, c.b, a * 3 / 4);
        MixPx(frame, x + 1, y, c.r, c.g, c.b, a * 3 / 4);
        MixPx(frame, x, y - 1, c.r, c.g, c.b, a * 3 / 4);
        MixPx(frame, x, y + 1, c.r, c.g, c.b, a * 3 / 4);
        MixPx(frame, x, y - 2, c.r, c.g, c.b, a / 3);
        break;
    case P_ROCK:    // a tumbling stone with a lit edge
        MixPx(frame, x, y, c.r * 3 / 4, c.g * 3 / 4, c.b * 3 / 4, a);
        MixPx(frame, x + 1, y, c.r / 2, c.g / 2, c.b / 2, a);
        MixPx(frame, x, y + 1, c.r / 2, c.g / 2, c.b / 2, a);
        MixPx(frame, x + 1, y + 1, c.r / 3, c.g / 3, c.b / 3, a);
        MixPx(frame, x, y - 1, c.r, c.g, c.b, a / 2);
        break;
    case P_RINGLET: // a small expanding ring
        Ring(frame, x, y, 1 + age / 4, 1, c, a * 2 / 3);
        break;
    case P_WISP:    // a drifting wisp
        Light(frame, x, y, 4, c, a * 3 / 4);
        MixPx(frame, x, y, c.r / 2, c.g / 3, c.b / 2, a / 2);
        break;
    case P_STAR:    // a four-point glint
        MixPx(frame, x, y, 255, 255, 255, a);
        MixPx(frame, x - 1, y, c.r, c.g, c.b, a * 2 / 3);
        MixPx(frame, x + 1, y, c.r, c.g, c.b, a * 2 / 3);
        MixPx(frame, x, y - 1, c.r, c.g, c.b, a * 2 / 3);
        MixPx(frame, x, y + 1, c.r, c.g, c.b, a * 2 / 3);
        MixPx(frame, x - 2, y, c.r, c.g, c.b, a / 4);
        MixPx(frame, x + 2, y, c.r, c.g, c.b, a / 4);
        break;
    case P_FEATHER: // a pale streak tumbling down
        for (i = -2; i <= 2; i++)
            MixPx(frame, x + i, y + ((age / 5 + (int)(seed & 1)) & 1 ? i / 2 : 0), 255, 255, 255, a * (3 - abs(i)) / 4);
        break;
    case P_BUBBLE:  // a bubble: a ring with a highlight
        Ring(frame, x, y, 2, 1, c, a * 3 / 4);
        MixPx(frame, x - 1, y - 1, 255, 255, 255, a / 2);
        break;
    default:
        MixPx(frame, x, y, c.r, c.g, c.b, a);
        MixPx(frame, x + 1, y, c.r, c.g, c.b, a / 3);
        MixPx(frame, x, y + 1, c.r, c.g, c.b, a / 3);
        break;
    }
}

// ---------------------------------------------------------------------------
// Bursts

static void DrawBurst(u16 *frame, const struct Burst *b, struct Rgb c)
{
    int f = sM.frame - b->start, i, s = b->strength;
    u32 seed = b->seed;
    const int ty = sM.type < NUMBER_OF_MON_TYPES ? sM.type : TYPE_NORMAL;
    int count = sTypeParticles[ty].count * (128 + s) / 256 + 4, life = sTypeParticles[ty].life;
    bool8 soft = b->kind == HIT_SOUND;

    if (f < 0)
        return;
    if (b->kind == HIT_QUAKE)
    {
        // the ground heaving: flattened waves rolling out from under the target, rocks and dust thrown up
        static const struct Rgb sDust = {196, 160, 104};
        struct Rgb deep = {120, 88, 48};
        for (i = 0; i < 6; i++)   // a wave every 7 frames while the ground shakes (about 50 frames)
        {
            int t = f - i * 7;
            if (t >= 0 && t < 22)
            {
                RingSquashed(frame, b->x, b->y, 8 + t * 4, t < 8 ? 3 : 2, deep, 200 * (22 - t) / 22, 4);
                RingSquashed(frame, b->x, b->y, 7 + t * 4, 1, sDust, 150 * (22 - t) / 22, 4);
            }
        }
        for (i = 0; i < 30; i++)
        {
            int vx = (int)(Rand(&seed) % 96) - 48, vy = -(int)(Rand(&seed) % 40) - 12, l = 22 + (int)(Rand(&seed) % 10);
            int delay = (int)(Rand(&seed) % 36), t = f - delay;
            if (t < 0 || t >= l)
                continue;
            Particle(frame, P_ROCK, b->x + vx * t / 16, b->y + (vy * t + 6 * t * t / 2) / 16, t, l, sDust, Rand(&seed));
        }
        return;
    }
    // the core of light: grows, then fades over 7 frames
    if (f < 7)
    {
        struct Rgb white = {255, 250, 235};
        int rad = (soft ? 5 : 7) + f * 3 / 2 + s / 64;
        Light(frame, b->x, b->y, rad, c, (soft ? 60 : 110) * (7 - f) / 7);
        Light(frame, b->x, b->y, rad / 2, white, (soft ? 40 : 90) * (7 - f) / 7);
    }
    // the shockwave
    if (!soft && f < 16)
    {
        struct Rgb white = {255, 255, 255}, deep = {c.r * 3 / 4, c.g * 3 / 4, c.b * 3 / 4};
        int r = 6 + f * (3 + s / 128), th = f < 5 ? 3 : (f < 10 ? 2 : 1);
        Ring(frame, b->x, b->y, r, th, deep, 245 * (16 - f) / 16);
        Ring(frame, b->x, b->y, r - 1, 1, white, 200 * (16 - f) / 16);
        if (s > 150 && f >= 3)
            Ring(frame, b->x, b->y, 3 + (f - 3) * 2, 1, c, 170 * (16 - f) / 16);
    }
    // sparks flying out
    if (!soft && f < 10)
    {
        int n = 8 + s / 24;
        for (i = 0; i < n; i++)
        {
            int ang = (int)(Rand(&seed) % 360), len = 8 + (int)(Rand(&seed) % 8) + s / 32;
            int r0 = 3 + f * 4, r1 = r0 + len * (10 - f) / 10;
            // a cheap direction: a point on a circle of radius 64 (table-free)
            int dx = 0, dy = 0, k, a2 = ang;
            static const s8 sCos16[16] = {64, 59, 45, 24, 0, -24, -45, -59, -64, -59, -45, -24, 0, 24, 45, 59};
            k = (a2 * 16 / 360) & 15;
            dx = sCos16[k];
            dy = sCos16[(k + 12) & 15];
            Spark(frame, b->x + dx * r0 / 64, b->y + dy * r0 / 64, b->x + dx * r1 / 64, b->y + dy * r1 / 64, c,
                  240 * (10 - f) / 10);
            if (f < 5)   // two pixels thick while they're fresh
                Spark(frame, b->x + dx * r0 / 64 + 1, b->y + dy * r0 / 64, b->x + dx * r1 / 64 + 1, b->y + dy * r1 / 64, c,
                      160 * (10 - f) / 10);
        }
    }
    // the type's own particles
    for (i = 0; i < count; i++)
    {
        int vx = (int)(Rand(&seed) % 64) - 32, vy = (int)(Rand(&seed) % 56) - 36;   // sixteenths of a pixel a frame
        int l = life - (int)(Rand(&seed) % 6), g = sTypeParticles[ty].gravity, x, y;
        u32 ps = Rand(&seed);
        if (f >= l)
            continue;
        if (sTypeParticles[ty].shape == P_BOLT || sTypeParticles[ty].shape == P_RINGLET)
            vx /= 2, vy /= 2;
        x = b->x + (vx * f) / 16;
        y = b->y + (vy * f + g * f * f / 2) / 16;
        if (sTypeParticles[ty].shape == P_LEAF || sTypeParticles[ty].shape == P_FEATHER)
            x += ((f + (int)(ps & 7)) / 6 & 1) ? 1 : -1;   // swaying
        Particle(frame, sTypeParticles[ty].shape, x, y, f, l, c, ps);
    }
}

// The move charging: motes of its type drawn into the user over its first frames
static void DrawCharge(u16 *frame, struct Rgb c)
{
    int f = sM.frame, i, cx, cy;
    u32 seed = sM.seed;
    if (f >= 18 || sM.hadBurst)
        return;
    cx = GetBattlerSpriteCoord(sM.attacker, BATTLER_COORD_X_2);
    cy = GetBattlerSpriteCoord(sM.attacker, BATTLER_COORD_Y_PIC_OFFSET);
    for (i = 0; i < 10; i++)
    {
        static const s8 sDir[10][2] = {{24, 0}, {17, 17}, {0, 24}, {-17, 17}, {-24, 0}, {-17, -17}, {0, -24}, {17, -17},
                                       {22, -9}, {-22, 9}};
        int delay = (int)(Rand(&seed) % 6), t = f - delay;
        if (t < 0 || t >= 12)
            continue;
        Particle(frame, P_STAR, cx + sDir[i][0] * (12 - t) / 12, cy + sDir[i][1] * (12 - t) / 12, t, 14, c, Rand(&seed));
    }
    if (f < 12)
        Ring(frame, cx, cy, 20 - f * 3 / 2, 1, c, 150 * (f < 6 ? f + 2 : 12 - f) / 8);
    if (f < 14)
        Light(frame, cx, cy, 10, c, 28 * (f < 7 ? f : 14 - f) / 7);
}

// Stat changes: chevrons rising (or falling) through the Pokemon
static void DrawStatChevrons(u16 *frame, u8 battler, bool8 down, struct Rgb c)
{
    int f = sM.frame, i, cx = GetBattlerSpriteCoord(battler, BATTLER_COORD_X_2),
        cy = GetBattlerSpriteCoord(battler, BATTLER_COORD_Y_PIC_OFFSET);
    for (i = 0; i < 4; i++)
    {
        int t = (f + i * 9) % 36, y = down ? cy - 22 + t : cy + 18 - t, a = 160 * (t < 8 ? t : (36 - t > 8 ? 8 : 36 - t)) / 8;
        int k, x0 = cx - 14 + (i & 1) * 10;
        for (k = -5; k <= 5; k++)
        {
            int yy = y + (down ? -abs(k) / 2 : abs(k) / 2);
            MixPx(frame, x0 + k, yy, c.r, c.g, c.b, a);
            MixPx(frame, x0 + k, yy + 1, 255, 255, 255, a / 3);
        }
    }
}

// ---------------------------------------------------------------------------
// Glow and trails from the effect sprites' own pixels

static void GlowAndTrails(u16 *frame, struct Rgb tint)
{
    static u16 sTmp[HH][HW][3];
    int x, y, ch;
    bool8 any = FALSE;

    // this frame's effect sprites at half size, folded into the fading trail
    for (y = 0; y < HH; y++)
        for (x = 0; x < HW; x++)
        {
            int r = 0, g = 0, b = 0, n = 0, dx, dy;
            for (dy = 0; dy < 2; dy++)
                for (dx = 0; dx < 2; dx++)
                {
                    u16 c = sFxLayer[(y * 2 + dy) * W + x * 2 + dx];
                    if (c & 0x8000)
                    {
                        r += (c & 31) * 8, g += ((c >> 5) & 31) * 8, b += ((c >> 10) & 31) * 8;
                        n++;
                    }
                }
            for (ch = 0; ch < 3; ch++)
                sM.trail[y][x][ch] = sM.trail[y][x][ch] * 11 / 16;   // the trail fades
            if (n)
            {
                // the sprite's own color pulled toward the move's: energy rather than paint
                int v[3] = {(r / n) * 5 / 8 + tint.r * 3 / 8, (g / n) * 5 / 8 + tint.g * 3 / 8, (b / n) * 5 / 8 + tint.b * 3 / 8};
                v[0] = v[0] * n / 4, v[1] = v[1] * n / 4, v[2] = v[2] * n / 4;
                for (ch = 0; ch < 3; ch++)
                    if (v[ch] > sM.trail[y][x][ch])
                        sM.trail[y][x][ch] = Clamp8(v[ch]);
            }
            if (sM.trail[y][x][0] | sM.trail[y][x][1] | sM.trail[y][x][2])
                any = TRUE;
        }
    if (!any)
        return;

    // a soft blur (two box passes each way)
    for (y = 0; y < HH; y++)
        for (x = 0; x < HW; x++)
            for (ch = 0; ch < 3; ch++)
            {
                int s = 0, k;
                for (k = -3; k <= 3; k++)
                {
                    int xx = x + k < 0 ? 0 : (x + k >= HW ? HW - 1 : x + k);
                    s += sM.trail[y][xx][ch];
                }
                sTmp[y][x][ch] = s / 7;
            }
    for (y = 0; y < HH; y++)
        for (x = 0; x < HW; x++)
            for (ch = 0; ch < 3; ch++)
            {
                int s = 0, k;
                for (k = -3; k <= 3; k++)
                {
                    int yy = y + k < 0 ? 0 : (y + k >= HH ? HH - 1 : y + k);
                    s += sTmp[yy][x][ch];
                }
                sM.glow[y][x][ch] = s / 7;
            }

    // the glow over the picture, and the trail where the sprites were a moment ago
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
        {
            const u16 *g = sM.glow[y / 2][x / 2];
            const u8 *t = sM.trail[y / 2][x / 2];
            bool8 sprite = (sFxLayer[y * W + x] & 0x8000) != 0;
            int r = g[0], gg = g[1], b = g[2], m, a;
            if (!sprite)
                r += t[0] / 2, gg += t[1] / 2, b += t[2] / 2;
            m = r > gg ? (r > b ? r : b) : (gg > b ? gg : b);
            if (m < 8 || sprite)
                continue;   // (the sprite itself stays as drawn)
            // the halo's own hue at full brightness, mixed in by how strong it is here
            a = m * 3 / 4;
            if (a > 150)
                a = 150;
            MixPx(frame, x, y, r * 255 / m, gg * 255 / m, b * 255 / m, a);
        }
}

// ---------------------------------------------------------------------------
// The battle's side

bool8 MP_ModernAnim_On(void)
{
    static s8 sEnv = -1;
    if (sEnv < 0)
    {
        const char *e = getenv("MOEMON_BATTLE_ANIMS");   // tests
        sEnv = e == NULL ? 2 : (strcmp(e, "modern") == 0);
    }
    return sEnv == 2 ? gMpConfig.battleAnims != 0 : sEnv;
}

static u8 BattlerFor(u8 which)
{
    switch (which)
    {
    case ANIM_ATTACKER:    return gBattleAnimAttacker;
    case ANIM_ATK_PARTNER: return IsContest() ? gBattleAnimAttacker : BATTLE_PARTNER(gBattleAnimAttacker);
    case ANIM_DEF_PARTNER: return IsContest() ? gBattleAnimTarget : BATTLE_PARTNER(gBattleAnimTarget);
    default:               return gBattleAnimTarget;
    }
}

// MOEMON_MODERN_AUDIT: one line per animation (docs/MODERN_ANIMS.md, tools/moemon_modernanim)
static void AuditFlush(void)
{
    static s8 sAudit = -1;
    if (sAudit < 0)
        sAudit = getenv("MOEMON_MODERN_AUDIT") != NULL;
    if (sAudit && sM.kind != KIND_NONE)
        MPNet_Log("modern audit: kind %d id %d type %d power %d frames %d hits splat %d shake %d sound %d quake %d"
                  " fx %u peak %u first %d last %d", sM.kind, sM.id, sM.type, sM.power, sM.frame, sM.nBurst[HIT_SPLAT],
                  sM.nBurst[HIT_SHAKE], sM.nBurst[HIT_SOUND], sM.nBurst[HIT_QUAKE], (unsigned)sM.fxPixels,
                  (unsigned)sM.fxPeak, sM.hadBurst ? sM.firstBurst : -1, sM.hadBurst ? sM.lastBurst : -1);
    sM.kind = KIND_NONE;
}

// battle_anim.c LaunchBattleAnimation
void MP_ModernAnim_Begin(const u8 *const table[], u16 id)
{
    AuditFlush();
    memset(&sM.bursts, 0, sizeof(sM.bursts));
    sM.kind = KIND_NONE;
    if (!MP_ModernAnim_On())
        return;
    sM.id = id;
    sM.frame = 0;
    sM.hadBurst = FALSE;
    sM.quaked = FALSE;
    sM.shake = 0;
    memset(sM.nBurst, 0, sizeof(sM.nBurst));
    sM.fxPixels = sM.fxPeak = 0;
    sM.attacker = gBattleAnimAttacker;
    sM.target = gBattleAnimTarget;
    sM.seed = 0x9E3779B9u * (id + 1) + gBattleAnimAttacker * 7919;
    sM.statDown = FALSE;
    if (table == gBattleAnims_Moves && id < MOVES_COUNT)
    {
        sM.kind = KIND_MOVE;
        sM.type = gBattleMoves[id].type;
        sM.power = gBattleMoves[id].power;
        sM.physical = sM.power != 0 && IS_TYPE_PHYSICAL(sM.type);
    }
    else if (table == gBattleAnims_General)
    {
        sM.kind = KIND_GENERAL;
        sM.power = 0;
        sM.physical = FALSE;
        switch (id)
        {
        case B_ANIM_RAIN_CONTINUES:     sM.type = TYPE_WATER; break;
        case B_ANIM_SUN_CONTINUES:      sM.type = TYPE_FIRE; break;
        case B_ANIM_SANDSTORM_CONTINUES: sM.type = TYPE_GROUND; break;
        case B_ANIM_HAIL_CONTINUES:     sM.type = TYPE_ICE; break;
        case B_ANIM_LEECH_SEED_DRAIN:
        case B_ANIM_INGRAIN_HEAL:       sM.type = TYPE_GRASS; break;
        case B_ANIM_FOCUS_BAND:
        case B_ANIM_FOCUS_PUNCH_SETUP:  sM.type = TYPE_FIGHTING; break;
        case B_ANIM_FUTURE_SIGHT_HIT:   sM.type = TYPE_PSYCHIC; sM.power = 80; break;
        case B_ANIM_DOOM_DESIRE_HIT:    sM.type = TYPE_STEEL; sM.power = 120; break;
        case B_ANIM_TURN_TRAP:          sM.type = TYPE_NORMAL; sM.power = 15; break;
        case B_ANIM_MON_HIT:            sM.type = TYPE_NORMAL; sM.power = 40; sM.physical = TRUE; break;
        case B_ANIM_STATS_CHANGE:
            // the stats going up or down: the game's own animation, as it is (the user, 2026-11-06)
            sM.kind = KIND_NONE;
            return;
        default:                        sM.type = TYPE_NORMAL; break;
        }
    }
    else if (table == gBattleAnims_StatusConditions)
    {
        static const u8 sStatusType[] = {
            [B_ANIM_STATUS_PSN] = TYPE_POISON, [B_ANIM_STATUS_CONFUSION] = TYPE_ELECTRIC, [B_ANIM_STATUS_BRN] = TYPE_FIRE,
            [B_ANIM_STATUS_INFATUATION] = TYPE_PSYCHIC, [B_ANIM_STATUS_SLP] = TYPE_FLYING, [B_ANIM_STATUS_PRZ] = TYPE_ELECTRIC,
            [B_ANIM_STATUS_FRZ] = TYPE_ICE, [B_ANIM_STATUS_CURSED] = TYPE_GHOST, [B_ANIM_STATUS_NIGHTMARE] = TYPE_DARK,
        };
        sM.kind = KIND_STATUS;
        sM.type = id < ARRAY_COUNT(sStatusType) ? sStatusType[id] : TYPE_NORMAL;
        sM.power = 0;
        sM.physical = FALSE;
    }
    // (special animations: ball throws, switches and level ups have their own effects: mp_ballfx.c)
    FamilyBegin();
}

// A hit: a burst where it lands (battle_anim.c: hit splats, shake tasks, sounds aimed at the target)
static void Burst(u8 kind, s16 x, s16 y)
{
    int i, slot = -1, strength;
    if (sM.kind == KIND_NONE)
        return;
    // hits a moment apart are one hit (a hit splat, the shake and the sound of the same blow)
    if (kind != HIT_QUAKE && sM.hadBurst && sM.frame - sM.lastBurst < 4)
    {
        for (i = 0; i < MAX_BURSTS; i++)
            if (sM.bursts[i].strength && sM.bursts[i].start == sM.lastBurst && sM.bursts[i].kind == HIT_SOUND && kind != HIT_SOUND)
                sM.bursts[i].kind = kind;   // the sound came first: it's a real hit
        return;
    }
    for (i = 0; i < MAX_BURSTS; i++)
        if (sM.bursts[i].strength == 0 || sM.frame - sM.bursts[i].start > (sM.bursts[i].kind == HIT_QUAKE ? 80 : 40))
        {
            slot = i;
            break;
        }
    if (slot < 0)
        return;
    strength = 70 + (sM.power > 150 ? 150 : sM.power);
    if (kind == HIT_SOUND)
        strength = strength * 2 / 3;
    sM.bursts[slot].x = x;
    sM.bursts[slot].y = y;
    sM.bursts[slot].start = sM.frame;
    sM.bursts[slot].strength = strength > 255 ? 255 : strength;
    sM.bursts[slot].kind = kind;
    sM.bursts[slot].seed = sM.seed + sM.frame * 2654435761u;
    if (!sM.hadBurst)
    {
        sM.firstBurst = sM.frame;
        if (sF.hit > (s16)sM.frame && !sF.synth)
            sF.hit = sM.frame;   // (the Classic blow came sooner than the audit's: follow it)
    }
    if ((s16)sM.frame > sF.lastHit && !sF.synth)
        sF.lastHit = sM.frame;
    sM.lastBurst = sM.frame;
    sM.hadBurst = TRUE;
    sM.nBurst[kind]++;
    if (kind != HIT_SOUND && sM.power >= 60 && (sM.physical || sM.power >= 90 || kind == HIT_QUAKE))
    {
        sM.shake = sM.power >= 100 ? 8 : 6;
        sM.shakeAmp = sM.power >= 100 ? 2 : 1;
    }
}

static void BurstOn(u8 kind, u8 battler, s16 dx, s16 dy)
{
    if (sM.kind == KIND_NONE)
        return;
    Burst(kind, GetBattlerSpriteCoord(battler, BATTLER_COORD_X_2) + dx,
          GetBattlerSpriteCoord(battler, BATTLER_COORD_Y_PIC_OFFSET) + dy);
}

// ---------------------------------------------------------------------------
// The move's own effect (1.120.0): each move has a family (src/data/mp_modernanim_moves.h from
// tools/moemon_modernanim/gen_recipes.py) drawn along the Classic animation's own timing: the audit's length and hit
// frames for the move, corrected by the hits seen live. docs/MODERN_ANIMS.md.


static int ISqrt(int v)
{
    int r = 0, b = 1 << 14;
    if (v <= 0)
        return 0;
    while (b > v)
        b >>= 2;
    while (b)
    {
        if (v >= r + b)
            v -= r + b, r = (r >> 1) + b;
        else
            r >>= 1;
        b >>= 2;
    }
    return r;
}

static const s8 sSin64[64] = {   // sin(i * 2pi / 64) * 64
      0,   6,  12,  19,  24,  30,  36,  41,  45,  49,  53,  56,  59,  61,  63,  64,
     64,  64,  63,  61,  59,  56,  53,  49,  45,  41,  36,  30,  24,  19,  12,   6,
      0,  -6, -12, -19, -24, -30, -36, -41, -45, -49, -53, -56, -59, -61, -63, -64,
    -64, -64, -63, -61, -59, -56, -53, -49, -45, -41, -36, -30, -24, -19, -12,  -6,
};
#define SIN64(a) (sSin64[(a) & 63])
#define COS64(a) (sSin64[((a) + 16) & 63])

static int Ease(int t, int len)   // 0..256, slow-fast-slow
{
    int x;
    if (len <= 0 || t >= len)
        return 256;
    if (t <= 0)
        return 0;
    x = t * 256 / len;
    return x * x * (768 - 2 * x) / 65536;
}

static int Fade(int t, int in, int hold, int out)   // 0..256 over in / hold / out frames
{
    if (t < 0)
        return 0;
    if (t < in)
        return 256 * t / in;
    t -= in;
    if (t < hold)
        return 256;
    t -= hold;
    if (t < out)
        return 256 * (out - t) / out;
    return 0;
}

// A thick soft line: white core, the color around it (alpha 0-256)
static void ThickLine(u16 *frame, int x0, int y0, int x1, int y1, int width, struct Rgb c, int alpha, bool8 core)
{
    int dx = x1 - x0, dy = y1 - y0, len = ISqrt(dx * dx + dy * dy), minx, maxx, miny, maxy, x, y;
    if (len == 0 || alpha <= 0 || width <= 0)
        return;
    minx = (x0 < x1 ? x0 : x1) - width - 1, maxx = (x0 > x1 ? x0 : x1) + width + 1;
    miny = (y0 < y1 ? y0 : y1) - width - 1, maxy = (y0 > y1 ? y0 : y1) + width + 1;
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= W) maxx = W - 1;
    if (maxy >= H) maxy = H - 1;
    for (y = miny; y <= maxy; y++)
        for (x = minx; x <= maxx; x++)
        {
            int px = x - x0, py = y - y0, along = (px * dx + py * dy), d, k;
            if (along < 0 || along > len * len)
                continue;
            d = abs(px * dy - py * dx) * 16 / len;   // distance from the line, sixteenths of a pixel
            if (d > width * 16)
                continue;
            k = alpha * (width * 16 - d) / (width * 16);
            MixPx(frame, x, y, c.r, c.g, c.b, k);
            if (core && d < width * 6)
                MixPx(frame, x, y, 255, 255, 255, k * (width * 6 - d) / (width * 6));
        }
}

// An arc of a circle (angles in 64ths of a turn, a0 to a1), thick, anti-aliased by distance
static void Arc(u16 *frame, int cx, int cy, int r, int a0, int a1, int thick, struct Rgb c, int alpha)
{
    int a, steps;
    if (r <= 0 || alpha <= 0)
        return;
    steps = (a1 - a0) * r / 8 + 2;
    for (a = 0; a <= steps; a++)
    {
        int ang16 = a0 * 16 + (a1 - a0) * 16 * a / steps, i0 = ang16 >> 4, fr = ang16 & 15, t;
        int cs = (COS64(i0) * (16 - fr) + COS64(i0 + 1) * fr) / 16, sn = (SIN64(i0) * (16 - fr) + SIN64(i0 + 1) * fr) / 16;
        for (t = -thick; t <= thick; t++)
        {
            int rr = r + t, k = alpha * (thick + 1 - abs(t)) / (thick + 1);
            MixPx(frame, cx + cs * rr / 64, cy + sn * rr / 64, c.r, c.g, c.b, k);
        }
    }
}

// Darken around a point (shadows, miasma): alpha 0-256
static void Shade(u16 *frame, int cx, int cy, int radius, struct Rgb tint, int strength)
{
    int x, y, r2 = radius * radius;
    if (radius <= 0 || strength <= 0)
        return;
    for (y = cy - radius; y <= cy + radius; y++)
        for (x = cx - radius; x <= cx + radius; x++)
        {
            int d2 = (x - cx) * (x - cx) + (y - cy) * (y - cy);
            if (d2 >= r2)
                continue;
            MixPx(frame, x, y, tint.r / 4, tint.g / 5, tint.b / 4, strength * (r2 - d2) / r2);
        }
}

// The picture rippled inside a box (the air warped by psychic power): rows pushed aside along a sine
static void Warp(u16 *frame, int cx, int cy, int hw, int hh, int amp, int phase)
{
    static u16 sRow[W];
    int y;
    if (amp <= 0)
        return;
    for (y = cy - hh; y <= cy + hh; y++)
    {
        int x0 = cx - hw, x1 = cx + hw, shift, x;
        if ((unsigned)y >= H)
            continue;
        shift = SIN64(y * 3 + phase) * amp / 64 * (hh - abs(y - cy)) / hh;
        if (shift == 0)
            continue;
        if (x0 < 0) x0 = 0;
        if (x1 >= W) x1 = W - 1;
        for (x = x0; x <= x1; x++)
            sRow[x] = frame[y * W + x];
        for (x = x0; x <= x1; x++)
        {
            int sx = x - shift;
            if (sx >= x0 && sx <= x1)
                frame[y * W + x] = sRow[sx];
        }
    }
}

// A chevron (^ or v), 11 pixels wide
static void Chevron(u16 *frame, int x0, int y, bool8 down, struct Rgb c, int a)
{
    int k;
    for (k = -5; k <= 5; k++)
    {
        int yy = y + (down ? -abs(k) / 2 : abs(k) / 2);
        MixPx(frame, x0 + k, yy, c.r, c.g, c.b, a);
        MixPx(frame, x0 + k, yy + 1, 255, 255, 255, a / 3);
    }
}

// A hexagon outline (a shield) centered on (cx, cy)
static void Hexagon(u16 *frame, int cx, int cy, int rx, int ry, struct Rgb c, int alpha)
{
    static const s8 sHex[7][2] = {{0, -64}, {55, -32}, {55, 32}, {0, 64}, {-55, 32}, {-55, -32}, {0, -64}};
    int i;
    for (i = 0; i < 6; i++)
        ThickLine(frame, cx + sHex[i][0] * rx / 64, cy + sHex[i][1] * ry / 64, cx + sHex[i + 1][0] * rx / 64,
                  cy + sHex[i + 1][1] * ry / 64, 2, c, alpha, TRUE);
}

static int Lerp(int a, int b, int t256)
{
    return a + (b - a) * t256 / 256;
}

static struct Rgb Brighter(struct Rgb c)
{
    struct Rgb b = {(c.r + 255) / 2, (c.g + 255) / 2, (c.b + 255) / 2};
    return b;
}

// The particle shape of the move's type
static int TypeShape(void)
{
    return sTypeParticles[sM.type < NUMBER_OF_MON_TYPES ? sM.type : TYPE_NORMAL].shape;
}

// The move's own impact where MODERN makes one (a Classic script with no hit)
static void SynthHit(int x, int y)
{
    Burst(HIT_SPLAT, x, y);
}

// ---- the families --------------------------------------------------------------------------------------------

// speed lines streaming back from the user, toward the blow
static void FxSpeedLines(u16 *frame, int f, int until, struct Rgb c)
{
    int i, dx = sF.tx - sF.ax, dy = sF.ty - sF.ay, len = ISqrt(dx * dx + dy * dy);
    u32 seed = sM.seed;
    if (len == 0 || f >= until)
        return;
    for (i = 0; i < 7; i++)
    {
        int off = (int)(Rand(&seed) % 33) - 16, t = (f * 3 + (int)(Rand(&seed) % 20)) % 20, l = 10 + (int)(Rand(&seed) % 10);
        int px = -dy * off / len, py = dx * off / len;
        int sx = sF.ax + px - dx * (t + l) / len, sy = sF.ay + py - dy * (t + l) / len;
        Spark(frame, sx, sy, sx + dx * l / len, sy + dy * l / len, c, 150 * (until - f) / until);
    }
}

// an impact star: eight points, long ones and short ones
static void FxStar(u16 *frame, int x, int y, int age, int size, struct Rgb c)
{
    int i, a;
    if (age < 0 || age >= 12)
        return;
    a = 240 * (12 - age) / 12;
    for (i = 0; i < 8; i++)
    {
        int ang = i * 8 + 4, l = (i & 1 ? size * 2 / 3 : size) * (age < 3 ? age + 1 : 3) / 3;
        ThickLine(frame, x, y, x + COS64(ang) * l / 64, y + SIN64(ang) * l / 64, i & 1 ? 1 : 2, c, a, TRUE);
    }
    Light(frame, x, y, size / 2 + 2, Brighter(c), a / 2);
}

static void FxStrike(u16 *frame, int f, struct Rgb c, bool8 ram)
{
    int h = sF.hit;
    FxSpeedLines(frame, f, h + 2, c);
    if (ram && f < h && f >= h - 10)
    {
        // a comet tail from the user to the target
        int t = Ease(f - (h - 10), 10), x = Lerp(sF.ax, sF.tx, t), y = Lerp(sF.ay, sF.ty, t);
        ThickLine(frame, Lerp(sF.ax, sF.tx, t / 3), Lerp(sF.ay, sF.ty, t / 3), x, y, 4 + (sF.variant == MAV_BIG ? 2 : 0), c,
                  150, TRUE);
        Light(frame, x, y, 8, c, 90);
    }
    FxStar(frame, sF.tx, sF.ty, f - h, sF.variant == MAV_BIG ? 26 : (ram ? 20 : 16), c);
    if (sF.variant == MAV_BIG && f >= h && f < h + 18)
        Ring(frame, sF.tx, sF.ty, 10 + (f - h) * 4, 2, Brighter(c), 200 * (h + 18 - f) / 18);
}

static void FxPunch(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit, age = f - h;
    if (f >= h - 6 && f < h)
    {
        int t = (f - (h - 6)) * 256 / 6;
        ThickLine(frame, Lerp(sF.ax, sF.tx, t / 2), Lerp(sF.ay, sF.ty, t / 2), Lerp(sF.ax, sF.tx, t), Lerp(sF.ay, sF.ty, t), 3, c,
                  170, TRUE);
    }
    if (age >= 0 && age < 14)
    {
        int i;
        // the blow's flash: a tight ring and four heavy rays
        Ring(frame, sF.tx, sF.ty, 4 + age * 3, age < 4 ? 3 : 2, c, 230 * (14 - age) / 14);
        for (i = 0; i < 4; i++)
        {
            int ang = i * 16 + 8, l0 = 6 + age * 2, l1 = l0 + 10 - age / 2;
            ThickLine(frame, sF.tx + COS64(ang) * l0 / 64, sF.ty + SIN64(ang) * l0 / 64, sF.tx + COS64(ang) * l1 / 64,
                      sF.ty + SIN64(ang) * l1 / 64, 2, c, 220 * (14 - age) / 14, TRUE);
        }
        Light(frame, sF.tx, sF.ty, 9, Brighter(c), 140 * (14 - age) / 14);
    }
}

static void FxKick(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit, age = f - h;
    if (f >= h - 8 && f < h + 6)
    {
        // a sweeping arc into the target, from below
        int t = f - (h - 8), a1 = 40 + t * 3;
        Arc(frame, sF.tx, sF.ty + 4, 20, a1 - 14, a1, 2, c, 220 * (14 - t) / 14);
        Arc(frame, sF.tx, sF.ty + 4, 17, a1 - 10, a1, 1, Brighter(c), 200 * (14 - t) / 14);
    }
    FxStar(frame, sF.tx, sF.ty, age, 18, c);
}

static void FxSlash(u16 *frame, int f, struct Rgb c)
{
    int i, n = sF.variant == MAV_BIG ? 2 : 3;
    for (i = 0; i < n; i++)
    {
        int start = sF.hit - 4 + i * 3, age = f - start, len = n == 2 ? 36 : 26, off = (i - (n - 1) / 2) * 9, prog, a;
        if (age < 0 || age >= 14)
            continue;
        prog = age < 4 ? (age + 1) * 64 : 256;
        a = 255 * (14 - age) / 14;
        // top right to bottom left, offset sideways
        {
            int x0 = sF.tx + len / 2 + off, y0 = sF.ty - len / 2, x1 = sF.tx - len / 2 + off, y1 = sF.ty + len / 2;
            ThickLine(frame, x0, y0, Lerp(x0, x1, prog), Lerp(y0, y1, prog), age < 6 ? 2 : 1, c, a, TRUE);
        }
    }
}

static void FxBite(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit;
    if (f >= h - 10 && f < h + 8)
    {
        int t = f - (h - 10), gap = t < 10 ? 18 - t * 18 / 10 : 0, a = 230 * (18 - t) / 18;
        Arc(frame, sF.tx, sF.ty - gap, 16, 36, 60, 2, c, a);    // the upper jaw
        Arc(frame, sF.tx, sF.ty + gap, 16, 4, 28, 2, c, a);     // the lower jaw
        Arc(frame, sF.tx, sF.ty - gap, 13, 38, 58, 1, Brighter(c), a);
        Arc(frame, sF.tx, sF.ty + gap, 13, 6, 26, 1, Brighter(c), a);
    }
    FxStar(frame, sF.tx, sF.ty, f - h, 14, c);
}

static void FxPierce(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit;
    if (f >= h - 8 && f < h + 4)
    {
        int t = Ease(f - (h - 8), 8), x = Lerp(sF.ax, sF.tx, t), y = Lerp(sF.ay, sF.ty, t);
        ThickLine(frame, Lerp(sF.ax, sF.tx, t * 2 / 3), Lerp(sF.ay, sF.ty, t * 2 / 3), x, y, 2, c, 230, TRUE);
    }
    if (f >= h && f < h + 10)
    {
        int i, age = f - h;
        for (i = 0; i < 6; i++)
        {
            int ang = i * 11 + 3, l = 6 + age * 2;
            Spark(frame, sF.tx, sF.ty, sF.tx + COS64(ang) * l / 64, sF.ty + SIN64(ang) * l / 64, c, 240 * (10 - age) / 10);
        }
    }
}

// the beam's window: from a little before the hit to a little after (or the middle of a hitless animation)
static void BeamWindow(int *s, int *e)
{
    if (sF.hit > 0)
    {
        *s = sF.hit - 22 > 4 ? sF.hit - 22 : 4;
        *e = (sF.lastHit > sF.hit ? sF.lastHit : sF.hit) + 12;
    }
    else
    {
        *s = sF.len / 4;
        *e = sF.len * 3 / 4;
    }
    if (*e > sF.len - 2 && sF.len > 8)
        *e = sF.len - 2;
    if (*e <= *s)
        *e = *s + 16;
}

static void FxBeam(u16 *frame, int f, struct Rgb c)
{
    int s, e, w, a, k;
    BeamWindow(&s, &e);
    if (f < s || f > e + 6)
        return;
    w = (sF.variant == MAV_BIG ? 6 : 4) * Fade(f - s, 5, e - s - 5, 11) / 256;
    a = Fade(f - s, 3, e - s, 9);
    if (w > 0)
    {
        ThickLine(frame, sF.ax, sF.ay, sF.tx, sF.ty, w + 2, c, a * 3 / 5, FALSE);
        ThickLine(frame, sF.ax, sF.ay, sF.tx, sF.ty, w, Brighter(c), a, TRUE);
    }
    // pulses running down the beam
    for (k = 0; k < 3; k++)
    {
        int t = ((f - s) * 24 + k * 85) % 256;
        Light(frame, Lerp(sF.ax, sF.tx, t), Lerp(sF.ay, sF.ty, t), 4 + w / 2, Brighter(c), a / 2);
    }
    Light(frame, sF.ax, sF.ay, 6 + w, c, a / 3);            // at the mouth
    Light(frame, sF.tx, sF.ty, 10 + w * 2, c, a * 2 / 5);   // where it lands
    if (sF.variant == MAV_TWIN)
        ThickLine(frame, sF.ax, sF.ay + 6, sF.tx, sF.ty + 6, w / 2 + 1, c, a / 2, TRUE);
}

static void FxBreath(u16 *frame, int f, struct Rgb c)
{
    int s, e, i, dx = sF.tx - sF.ax, dy = sF.ty - sF.ay, len = ISqrt(dx * dx + dy * dy), shape = TypeShape();
    BeamWindow(&s, &e);
    if (len == 0 || f < s || f > e + 18)
        return;
    // particles leaving the mouth along the line, a cone widening toward the target
    for (i = 0; i < 26; i++)
    {
        u32 ps = sM.seed + i * 7919u;
        int born = s + (int)(Rand(&ps) % (e - s + 1)), age = f - born, life = 18, t, spread, x, y;
        if (age < 0 || age >= life || born > e)
            continue;
        t = age * 256 / life;
        spread = ((int)(Rand(&ps) % 21) - 10) * t / 256;
        x = Lerp(sF.ax, sF.tx, t) - dy * spread / len;
        y = Lerp(sF.ay, sF.ty, t) + dx * spread / len;
        Particle(frame, shape == P_RINGLET ? P_DOT : shape, x, y, age / 2, life, c, ps);
        if ((i & 3) == 0)
            Light(frame, x, y, 4, c, 70 * (life - age) / life);
    }
    if (f >= s && f <= e)
        Light(frame, sF.tx, sF.ty, 12, c, 60);
}

static void FxOrb(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit > 0 ? sF.hit : sF.len / 2, s = h - 14 > 2 ? h - 14 : 2, r = sF.variant == MAV_BIG ? 8 : 6;
    if (f >= s && f < h)
    {
        int t = Ease(f - s, h - s), x = Lerp(sF.ax, sF.tx, t), y = Lerp(sF.ay, sF.ty, t) - SIN64(t / 8) * 10 / 64, k;
        for (k = 1; k <= 4; k++)   // the trail
        {
            int tt = Ease(f - s - k * 2, h - s);
            Light(frame, Lerp(sF.ax, sF.tx, tt), Lerp(sF.ay, sF.ty, tt), r - k, c, 90 - k * 18);
        }
        Light(frame, x, y, r + 3, c, 170);
        Ring(frame, x, y, r - 2, 1, Brighter(c), 200);
        Light(frame, x, y, r / 2, Brighter(c), 200);
    }
    if (f >= h && f < h + 16)
    {
        int age = f - h;
        Ring(frame, sF.tx, sF.ty, 6 + age * 3, 2, c, 220 * (16 - age) / 16);
        Light(frame, sF.tx, sF.ty, 14 - age / 2, Brighter(c), 160 * (16 - age) / 16);
    }
}

static void FxFromSky(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit > 0 ? sF.hit : sF.len / 2, i, shape = TypeShape();
    for (i = 0; i < 6; i++)
    {
        u32 ps = sM.seed + i * 104729u;
        int land = h - 6 + (int)(Rand(&ps) % 14), dur = 10, age = f - (land - dur), x, y, ox = (int)(Rand(&ps) % 41) - 20;
        if (age < 0 || age > dur + 8)
            continue;
        x = sF.tx + ox;
        if (age <= dur)
        {
            y = Lerp(-8, sF.ty + 8, age * 256 / dur);
            if (sM.type == TYPE_ELECTRIC)
            {
                // a jagged bolt from the top
                u32 bs = ps + f;
                int px = x, py = -4, k;
                for (k = 0; k < 8 && py < y; k++)
                {
                    int nx = px + (int)(Rand(&bs) % 9) - 4, ny = py + (y + 4) / 8;
                    ThickLine(frame, px, py, nx, ny, 1, c, 240, TRUE);
                    px = nx, py = ny;
                }
            }
            else
            {
                ThickLine(frame, x, y - 12, x, y, 2, c, 190, TRUE);
                Particle(frame, shape, x, y, 0, 20, c, ps);
            }
        }
        else
        {
            Light(frame, x, sF.ty + 8, 7, c, 130 * (dur + 8 - age) / 8);
        }
    }
}

static void FxQuake(u16 *frame, int f, struct Rgb c)
{
    int h = sF.hit > 0 ? sF.hit : sF.len / 3, age = f - h, i;
    if (age < 0 || age > 40)
        return;
    // cracks running out along the ground under the target
    for (i = 0; i < 5; i++)
    {
        int ang = 28 + i * 2 - 4, l = age < 12 ? age * 4 : 48;
        ThickLine(frame, sF.tx, sF.ty + 26, sF.tx + COS64(ang) * l / 64 * (i & 1 ? -1 : 1), sF.ty + 26 + SIN64(ang) * l / 256, 1,
                  c, 180 * (40 - age) / 40, FALSE);
    }
    if (!sM.quaked && age == 0)
    {
        sM.quaked = TRUE;
        Burst(HIT_QUAKE, sF.tx, sF.ty + 24);
    }
}

static void FxBlast(u16 *frame, int f, struct Rgb c)
{
    // an explosion: on the user for EXPLOSION / SELF-DESTRUCT / MIND BLOWN, otherwise where the blow lands
    bool8 self = sM.id == MOVE_EXPLOSION || sM.id == MOVE_SELF_DESTRUCT || sM.id == MOVE_MIND_BLOWN
              || sM.id == MOVE_MISTY_EXPLOSION || sM.id == MOVE_FINAL_GAMBIT;
    int h = sF.hit > 0 ? sF.hit : sF.len / 2, age = f - h, cx = self ? sF.ax : sF.tx, cy = self ? sF.ay : sF.ty;
    if (age < -6 || age > 30)
        return;
    if (age < 0)
    {
        Light(frame, cx, cy, 6 - age, c, 40 * (6 + age));   // gathering
        return;
    }
    Light(frame, cx, cy, 18 + age, c, 200 * (30 - age) / 30);
    Light(frame, cx, cy, 10 + age / 2, Brighter(c), 160 * (30 - age) / 30);
    Ring(frame, cx, cy, 8 + age * 4, 3, c, 220 * (30 - age) / 30);
    Ring(frame, cx, cy, 4 + age * 3, 1, Brighter(c), 180 * (30 - age) / 30);
}

static void FxWave(u16 *frame, int f, struct Rgb c)
{
    int s, e, t, x, y, side = GetBattlerSide(sM.attacker) == B_SIDE_PLAYER ? 1 : -1;
    BeamWindow(&s, &e);
    if (f < s || f > e)
        return;
    // a band sweeping from the user's side across the field
    t = (f - s) * 256 / (e - s + 1);
    for (y = 40; y < 120; y += 2)
    {
        int crest = Lerp(side > 0 ? -20 : W + 20, side > 0 ? W + 20 : -20, t) + SIN64(y * 2 + f * 3) * 6 / 64;
        for (x = crest - 10; x <= crest + 10; x++)
        {
            int d = abs(x - crest), a = 150 * (10 - d) / 10 * Fade(f - s, 4, e - s - 8, 4) / 256;
            MixPx(frame, x, y, c.r, c.g, c.b, a);
            MixPx(frame, x, y + 1, c.r, c.g, c.b, a / 2);
            if (d < 2)
                MixPx(frame, x, y, 255, 255, 255, a);
        }
    }
}

static void FxSound(u16 *frame, int f, struct Rgb c)
{
    int s = 2, e = sF.len > 20 ? sF.len - 8 : 24, k, dx = sF.tx - sF.ax, dy = sF.ty - sF.ay, ang;
    if (f < s || f > e)
        return;
    // the direction to the target, in 64ths of a turn
    ang = abs(dx) >= abs(dy) ? (dx > 0 ? 0 : 32) : (dy > 0 ? 16 : 48);
    if (dx != 0 && dy != 0)
        ang = dx > 0 ? (dy > 0 ? 6 : 58) : (dy > 0 ? 26 : 38);
    for (k = 0; k < 5; k++)
    {
        int age = (f - s) - k * 7, r;
        if (age < 0 || age >= 30)
            continue;
        r = 8 + age * 3;
        Arc(frame, sF.ax, sF.ay, r, ang - 7, ang + 7, age < 10 ? 2 : 1, c, 210 * (30 - age) / 30);
    }
}

static void FxWind(u16 *frame, int f, struct Rgb c)
{
    int s, e, k;
    BeamWindow(&s, &e);
    if (f < s - 6 || f > e + 6)
        return;
    for (k = 0; k < 4; k++)
    {
        int r = 10 + k * 6, a0 = (f * 3 + k * 16) & 63, a = 170 * Fade(f - s + 6, 6, e - s, 6) / 256;
        Arc(frame, sF.tx, sF.ty - k * 4 + 8, r, a0, a0 + 18, 1, k & 1 ? Brighter(c) : c, a);
    }
}

static void FxMind(u16 *frame, int f, struct Rgb c)
{
    int s = 4, e = sF.len > 24 ? sF.len - 6 : 30, k, a = Fade(f - s, 6, e - s - 12, 6);
    if (f < s || f > e)
        return;
    for (k = 0; k < 3; k++)
    {
        int age = ((f - s) + k * 8) % 24;
        Ring(frame, sF.tx, sF.ty, 30 - age, 1, c, a * (age < 4 ? age : 4) / 4 * 200 / 256);
    }
    Warp(frame, sF.tx, sF.ty, 30, 26, 2 * a / 256, f * 4);
    Light(frame, sF.tx, sF.ty, 18, c, a / 5);
}

static void FxShadow(u16 *frame, int f, struct Rgb c)
{
    int s = 2, e = sF.len > 20 ? sF.len - 4 : 24, a = Fade(f - s, 8, e - s - 16, 8), k;
    if (f < s || f > e)
        return;
    Shade(frame, sF.tx, sF.ty + 4, 30, c, a * 3 / 5);
    for (k = 0; k < 6; k++)
    {
        int ang = (f + k * 11) & 63, r = 16 + (k * 5) % 12;
        Particle(frame, P_WISP, sF.tx + COS64(ang) * r / 64, sF.ty + SIN64(ang) * r / 96 + 4, 4, 28, c, k);
    }
}

static void FxPowder(u16 *frame, int f, struct Rgb c)
{
    int s = 4, e = sF.len > 24 ? sF.len - 6 : 30, i;
    if (f < s || f > e + 12)
        return;
    for (i = 0; i < 22; i++)
    {
        u32 ps = sM.seed + i * 31337u;
        int born = s + (int)(Rand(&ps) % (e - s + 1)), age = f - born, life = 26, x, y;
        if (age < 0 || age >= life)
            continue;
        x = sF.tx + (int)(Rand(&ps) % 49) - 24 + SIN64(age * 3 + i * 9) * 3 / 64;
        y = sF.ty - 34 + age * 2;
        Particle(frame, P_DOT, x, y, age, life, c, ps);
        if ((i & 3) == 0)
            Light(frame, x, y, 3, Brighter(c), 80 * (life - age) / life);
    }
}

static void FxStatus(u16 *frame, int f, struct Rgb c)
{
    int s, e, k;
    BeamWindow(&s, &e);
    if (sM.attacker == sM.target || sF.ax == sF.tx)
    {
        // on the user (or the user's side): a ring of light settling on it
        if (f >= s && f <= e)
            Ring(frame, sF.ax, sF.ay, 20 - (f - s) % 16, 1, c, 180);
        return;
    }
    for (k = 0; k < 6; k++)
    {
        int t = ((f - s) * 12 + k * 43) % 256;
        if (f < s || f > e)
            break;
        Ring(frame, Lerp(sF.ax, sF.tx, t), Lerp(sF.ay, sF.ty, t), 3 + t / 64, 1, c, 200);
    }
    if (f >= e - 10 && f <= e + 12)
    {
        int age = f - (e - 10);
        Ring(frame, sF.tx, sF.ty, 18, 2, c, 200 * (age < 6 ? age : (22 - age > 0 ? 22 - age : 0)) / 16);
    }
}

static void FxDrain(u16 *frame, int f, struct Rgb c)
{
    int s, e, k;
    BeamWindow(&s, &e);
    s = sF.hit > 0 ? sF.hit : s;
    e = sF.len > s + 30 ? sF.len - 4 : s + 30;
    for (k = 0; k < 7; k++)
    {
        int born = s + k * 4, age = f - born, life = 22, t, x, y;
        if (age < 0 || age >= life)
            continue;
        t = Ease(age, life);
        x = Lerp(sF.tx, sF.ax, t);
        y = Lerp(sF.ty, sF.ay, t) - SIN64(t / 8) * 16 / 64;
        Light(frame, x, y, 5, c, 170);
        Light(frame, x, y, 2, Brighter(c), 200);
    }
    if (f >= s + 20 && f < e)
        Light(frame, sF.ax, sF.ay, 16, c, 70 * Fade(f - s - 20, 6, e - s - 32, 6) / 256);
}

static void FxHeal(u16 *frame, int f, struct Rgb c, int x0, int y0)
{
    int s = 4, e = sF.len > 24 ? sF.len - 4 : 30, i, a = Fade(f - s, 8, e - s - 16, 8);
    struct Rgb soft = {(c.r + 255 * 2) / 3, (c.g + 255 * 2) / 3, (c.b + 255 * 2) / 3};
    if (f < s || f > e)
        return;
    Light(frame, x0, y0, 22, soft, a / 4);
    for (i = 0; i < 12; i++)
    {
        u32 ps = sM.seed + i * 7907u;
        int born = s + (int)(Rand(&ps) % (e - s + 1)), age = f - born, life = 24, x, y;
        if (age < 0 || age >= life)
            continue;
        x = x0 + (int)(Rand(&ps) % 37) - 18;
        y = y0 + 20 - age * 2;
        if (i & 1)
        {
            // a small plus
            MixPx(frame, x, y, 255, 255, 255, 200 * (life - age) / life);
            MixPx(frame, x - 1, y, soft.r, soft.g, soft.b, 200 * (life - age) / life);
            MixPx(frame, x + 1, y, soft.r, soft.g, soft.b, 200 * (life - age) / life);
            MixPx(frame, x, y - 1, soft.r, soft.g, soft.b, 200 * (life - age) / life);
            MixPx(frame, x, y + 1, soft.r, soft.g, soft.b, 200 * (life - age) / life);
        }
        else
            Particle(frame, P_STAR, x, y, age, life, soft, ps);
    }
}

static void FxAura(u16 *frame, int f, struct Rgb c, int x0, int y0, bool8 down)
{
    int s = 2, e = sF.len > 20 ? sF.len - 4 : 24, a = Fade(f - s, 8, e - s - 16, 8), y, i;
    if (f < s || f > e)
        return;
    // a column of light (rising for stats up; a dark one sinking for stats down)
    for (y = y0 - 34; y <= y0 + 26; y += 2)
    {
        int band = SIN64((y - (down ? -f : f) * 2) * 2) * 24 / 64 + 40;
        int w = 16 + (y0 + 26 - y) / 8;
        if (down)
            Shade(frame, x0, y, w / 2, c, a * band / 512);
        else
            Light(frame, x0, y, w / 2, c, a * band / 640);
    }
    // streaks of the color rising (or sinking) around it
    for (i = 0; i < 8; i++)
    {
        u32 ps = sM.seed + i * 40503u;
        int sx = x0 + (int)(Rand(&ps) % 41) - 20, t = (f * 2 + (int)(Rand(&ps) % 40)) % 40;
        int sy = down ? y0 - 30 + t * 3 / 2 : y0 + 26 - t * 3 / 2, l = 6 + (int)(Rand(&ps) % 6);
        int fa = a * (t < 8 ? t : (40 - t > 8 ? 8 : 40 - t)) / 8 * 250 / 256;
        ThickLine(frame, sx, sy, sx, down ? sy - l : sy + l, 2, c, fa, !down);
    }
}

static void FxShield(u16 *frame, int f, struct Rgb c)
{
    int s = 4, e = sF.len > 24 ? sF.len - 4 : 30, a = Fade(f - s, 6, e - s - 14, 8), grow, side;
    if (f < s || f > e)
        return;
    side = GetBattlerSide(sM.attacker) == B_SIDE_PLAYER ? 1 : -1;
    grow = Ease(f - s, 8);
    Hexagon(frame, sF.ax + side * 18, sF.ay, 14 * grow / 256 + 4, 30 * grow / 256 + 6, c, a * 3 / 4);
    Light(frame, sF.ax + side * 18, sF.ay - 10 + ((f * 2) % 24), 6, Brighter(c), a / 4);   // a glint running down it
}

static void FxField(u16 *frame, int f, struct Rgb c)
{
    int s = 2, e = sF.len > 20 ? sF.len - 4 : 24, a = Fade(f - s, 12, e - s - 24, 12), x, y, i;
    if (f < s || f > e)
        return;
    // the field washed gently in the move's color, a band of light rolling across it
    for (y = 0; y < H; y += 1)
    {
        int band = 30 + SIN64(y / 2 + f * 2) * 14 / 64;
        for (x = 0; x < W; x += 1)
            if (((x + y) & 1) == 0)
                MixPx(frame, x, y, c.r, c.g, c.b, a * band / 1024);
    }
    for (i = 0; i < 16; i++)
    {
        u32 ps = sM.seed + i * 2654435761u;
        int px = (int)(Rand(&ps) % W), py = ((int)(Rand(&ps) % H) + f * 2) % H;
        Particle(frame, TypeShape() == P_BOLT ? P_STAR : TypeShape(), px, py, 4, 24, c, ps);
    }
}

static void FxSwap(u16 *frame, int f, struct Rgb c)
{
    int s, e, t;
    BeamWindow(&s, &e);
    if (f < s || f > e || sF.ax == sF.tx)
        return;
    t = Ease(f - s, e - s);
    // two lights trading places along an arc
    Light(frame, Lerp(sF.ax, sF.tx, t), Lerp(sF.ay, sF.ty, t) - SIN64(t / 8) * 18 / 64, 7, c, 170);
    Light(frame, Lerp(sF.tx, sF.ax, t), Lerp(sF.ty, sF.ay, t) + SIN64(t / 8) * 18 / 64, 7, Brighter(c), 170);
}

static void FxCharge(u16 *frame, int f, struct Rgb c)
{
    int e = sF.len > 20 ? sF.len - 6 : 24, i;
    if (f > e)
        return;
    for (i = 0; i < 10; i++)
    {
        int t = (f * 2 + i * 26) % 50, ang = i * 6 + 3, r = 34 - t * 34 / 50;
        Particle(frame, P_STAR, sF.ax + COS64(ang) * r / 64, sF.ay + SIN64(ang) * r / 64, t / 3, 20, c, i);
    }
    Light(frame, sF.ax, sF.ay, 8 + f * 10 / (e + 1), c, 30 + f * 70 / (e + 1));
}

static void FxDance(u16 *frame, int f, struct Rgb c)
{
    int s = 2, e = sF.len > 20 ? sF.len - 4 : 24, a = Fade(f - s, 6, e - s - 12, 6), k;
    if (f < s || f > e)
        return;
    for (k = 0; k < 2; k++)
        Arc(frame, sF.ax, sF.ay + 6 - k * 10, 24 - k * 4, (f * 2 + k * 32) & 63, ((f * 2 + k * 32) & 63) + 22, 1,
            k ? Brighter(c) : c, a * 3 / 4);
    if (sF.hit > 0)
        FxStar(frame, sF.tx, sF.ty, f - sF.hit, 16, c);
}

static void DrawFamily(u16 *frame)
{
    int f = sM.frame;
    struct Rgb c = sF.c;
    switch (sF.family)
    {
    case MA_STRIKE:  FxStrike(frame, f, c, FALSE); break;
    case MA_RAM:     FxStrike(frame, f, c, TRUE); break;
    case MA_PUNCH:   FxPunch(frame, f, c); break;
    case MA_KICK:    FxKick(frame, f, c); break;
    case MA_SLASH:   FxSlash(frame, f, c); break;
    case MA_BITE:    FxBite(frame, f, c); break;
    case MA_PIERCE:  FxPierce(frame, f, c); break;
    case MA_BEAM:    FxBeam(frame, f, c); break;
    case MA_BREATH:  FxBreath(frame, f, c); break;
    case MA_ORB:     FxOrb(frame, f, c); break;
    case MA_FROMSKY: FxFromSky(frame, f, c); break;
    case MA_QUAKE:   FxQuake(frame, f, c); break;
    case MA_BLAST:   FxBlast(frame, f, c); break;
    case MA_WAVE:    FxWave(frame, f, c); break;
    case MA_SOUND:   FxSound(frame, f, c); break;
    case MA_WIND:    FxWind(frame, f, c); break;
    case MA_MIND:    FxMind(frame, f, c); break;
    case MA_SHADOW:  FxShadow(frame, f, c); break;
    case MA_POWDER:  FxPowder(frame, f, c); break;
    case MA_STATUS:  FxStatus(frame, f, c); break;
    case MA_DRAIN:   FxDrain(frame, f, c); break;
    case MA_HEAL:
        // HEAL PULSE / FLORAL HEALING / POLLEN PUFF on another: the light on the target
        if (sM.attacker != sM.target && (sM.id == MOVE_HEAL_PULSE || sM.id == MOVE_FLORAL_HEALING))
            FxHeal(frame, f, c, sF.tx, sF.ty);
        else
            FxHeal(frame, f, c, sF.ax, sF.ay);
        break;
    case MA_BUFF:    FxAura(frame, f, c, sF.ax, sF.ay, FALSE); break;
    case MA_DEBUFF:  FxAura(frame, f, c, sF.tx, sF.ty, TRUE); break;
    case MA_SHIELD:  FxShield(frame, f, c); break;
    case MA_FIELD:   FxField(frame, f, c); break;
    case MA_SWAP:    FxSwap(frame, f, c); break;
    case MA_CHARGE:  FxCharge(frame, f, c); break;
    case MA_DANCE:   FxDance(frame, f, c); break;
    case MA_MULTI:
        // a small star at each blow (the Classic hits, or MODERN's own spread over the animation)
        if (sM.hadBurst)
            FxStar(frame, sF.tx, sF.ty, f - sM.lastBurst, 12, c);
        break;
    }
}

// A move with damage but no hit in its Classic script: MODERN's own blow(s) at the recipe's frame
static bool8 DamagingFamily(void)
{
    return sM.power > 0 && sF.family != MA_HEAL && sF.family != MA_BUFF && sF.family != MA_SHIELD
        && sF.family != MA_FIELD && sF.family != MA_CHARGE;
}

static void SynthHits(void)
{
    int f = sM.frame;
    if (!sF.synth || (sM.hadBurst && sF.family != MA_MULTI))
        return;
    if (sF.family == MA_MULTI)
    {
        // two to five blows spread over the middle of the animation (none once the Classic ones started)
        int n = 3, k;
        if (sM.nBurst[HIT_SPLAT] + sM.nBurst[HIT_SHAKE] > sF.synthCount)
            return;
        for (k = sF.synthCount; k < n; k++)
            if (f == sF.len / 4 + k * (sF.len / 2) / n)
            {
                SynthHit(sF.tx + (k - 1) * 6, sF.ty + ((k & 1) ? -4 : 4));
                sF.synthCount++;
                break;
            }
        return;
    }
    if (!sF.synthDone && f >= sF.hit)
    {
        sF.synthDone = TRUE;
        if (sF.family == MA_BLAST && (sM.id == MOVE_EXPLOSION || sM.id == MOVE_SELF_DESTRUCT))
            return;   // (the blast itself)
        if (sF.family == MA_QUAKE)
            return;   // (FxQuake heaves the ground)
        SynthHit(sF.tx, sF.ty);
    }
}

static void FamilyBegin(void)
{
    memset(&sF, 0, sizeof(sF));
    if (sM.kind != KIND_MOVE || sM.id >= MOVES_COUNT)
        return;
    {
        const struct MaRecipe *r = &sMaRecipes[sM.id];
        struct Rgb c = {r->r, r->g, r->b};
        sF.family = r->family;
        sF.variant = r->variant;
        sF.c = c;
        sF.len = r->frames ? r->frames : 90;
        sF.hit = r->firstHit;
        sF.lastHit = r->lastHit;
        if (sF.hit < 0)
        {
            // no hit in the Classic script: MODERN's own, where the energy would land
            sF.synth = DamagingFamily();
            sF.hit = sF.family == MA_BEAM || sF.family == MA_BREATH || sF.family == MA_WAVE ? sF.len * 3 / 5 : sF.len / 2;
            sF.lastHit = sF.hit;
        }
    }
}

static void FamilyPositions(void)
{
    sF.ax = GetBattlerSpriteCoord(sM.attacker, BATTLER_COORD_X_2);
    sF.ay = GetBattlerSpriteCoord(sM.attacker, BATTLER_COORD_Y_PIC_OFFSET);
    sF.tx = GetBattlerSpriteCoord(sM.target, BATTLER_COORD_X_2);
    sF.ty = GetBattlerSpriteCoord(sM.target, BATTLER_COORD_Y_PIC_OFFSET);
}

// battle_anim.c Cmd_createsprite: a hit splat (or another sprite that marks a blow landing)
void MP_ModernAnim_OnSprite(const struct SpriteTemplate *template)
{
    u16 tag;
    if (sM.kind == KIND_NONE || template == NULL)
        return;
    tag = template->tileTag;
    switch (tag)
    {
    case ANIM_TAG_IMPACT:
    case ANIM_TAG_CROSS_IMPACT:
    case ANIM_TAG_CIRCLE_IMPACT:
    case ANIM_TAG_WATER_IMPACT:
    case ANIM_TAG_HIT:
    case ANIM_TAG_HIT_2:
    case ANIM_TAG_HORN_HIT:
    case ANIM_TAG_HORN_HIT_2:
    case ANIM_TAG_SLAM_HIT:
    case ANIM_TAG_SLAM_HIT_2:
    case ANIM_TAG_WHIP_HIT:
        // (the hit splat's own offset from its battler: arguments 0 and 1)
        BurstOn(HIT_SPLAT, gBattleAnimArgs[2] == ANIM_ATTACKER ? sM.attacker : sM.target,
                gBattleAnimArgs[0] > -48 && gBattleAnimArgs[0] < 48 ? gBattleAnimArgs[0] : 0,
                gBattleAnimArgs[1] > -48 && gBattleAnimArgs[1] < 48 ? gBattleAnimArgs[1] : 0);
        break;
    }
}

// battle_anim.c Cmd_createvisualtask: the target (or its partner) shaken by a blow
void MP_ModernAnim_OnTask(TaskFunc func)
{
    if (sM.kind == KIND_NONE)
        return;
    if ((func == AnimTask_ShakeMon || func == AnimTask_ShakeMon2 || func == AnimTask_ShakeMonInPlace
      || func == AnimTask_ShakeAndSinkMon)
     && gBattleAnimArgs[0] != ANIM_ATTACKER && gBattleAnimArgs[0] != ANIM_ATK_PARTNER)
        BurstOn(HIT_SHAKE, BattlerFor(gBattleAnimArgs[0]), 0, 0);
    else if (func == AnimTask_ShakeTargetBasedOnMovePowerOrDmg || func == AnimTask_ShakeTargetInPattern)
        BurstOn(HIT_SHAKE, sM.target, 0, 0);
    else if (func == AnimTask_HorizontalShake && !sM.quaked)
    {
        // the ground shaking under the target's side: one heave, at the target's feet
        sM.quaked = TRUE;
        Burst(HIT_QUAKE, GetBattlerSpriteCoord(sM.target, BATTLER_COORD_X_2),
              GetBattlerSpriteCoord(sM.target, BATTLER_COORD_Y_PIC_OFFSET) + 24);
    }
}

// battle_anim.c Cmd_playsewithpan: a sound aimed at the target (a softer burst: it may be a hit or just arriving)
void MP_ModernAnim_OnSound(s8 pan)
{
    if (sM.kind == KIND_MOVE && pan == SOUND_PAN_TARGET && sM.attacker != sM.target)
        BurstOn(HIT_SOUND, sM.target, 0, 0);
}

// sprite.c: an effect sprite of the running animation (its pixels glow and trail)
bool8 MP_ModernAnim_IsFxSprite(const struct Sprite *sprite)
{
    return gMpFxLayer != NULL && sprite->template != NULL && sprite->template->tileTag >= ANIM_SPRITES_START
        && sprite->template->tileTag < ANIM_SPRITES_START + 300;
}

// sprite.c: a sprite of the battle's UI (MODERN never draws over it)
bool8 MP_ModernAnim_IsUiSprite(const struct Sprite *sprite)
{
    u16 tag;
    if (gMpFxLayer == NULL || sprite->template == NULL)
        return FALSE;
    tag = sprite->template->tileTag;
    return tag >= TAG_HEALTHBOX_PLAYER1_TILE && tag <= TAG_STATUS_SUMMARY_BALLS_TILE + 4;
}

// sdl2.c VDraw: over the finished picture
void MP_ModernAnim_Composite(u16 *frame)
{
    struct Rgb c;
    int i;
    bool8 running = sM.kind != KIND_NONE && gAnimScriptActive && MP_ModernAnim_On()
                 && (gMain.inBattle || IsContest());
    if (!running)
    {
        if (sM.kind != KIND_NONE && !gAnimScriptActive)
        {
            AuditFlush();
            sM.kind = KIND_NONE;   // over: nothing outlives the Classic animation
        }
        if (gMpFxLayer != NULL)
        {
            gMpFxLayer = NULL;
            gMpUiLayer = NULL;
            memset(sM.trail, 0, sizeof(sM.trail));
        }
        return;
    }
    if (gMpFxLayer == NULL)
    {
        memset(sFxLayer, 0, sizeof(sFxLayer));
        memset(sM.trail, 0, sizeof(sM.trail));
        gMpFxLayer = sFxLayer;   // from the next frame the renderer marks the effect sprites
        gMpUiLayer = sUiLayer;   // ... and the UI's pixels
        sM.frame++;              // (nothing drawn until the UI is known)
        return;
    }
    memcpy(sSnap, frame, sizeof(sSnap));
    c = sTypeColor[sM.type < NUMBER_OF_MON_TYPES ? sM.type : TYPE_NORMAL];

    GlowAndTrails(frame, c);
    if (sM.kind == KIND_MOVE)
    {
        FamilyPositions();
        SynthHits();
        DrawFamily(frame);
        if (sM.attacker != sM.target && (sF.family == MA_BEAM || sF.family == MA_BREATH || sF.family == MA_ORB
                                      || sF.family == MA_FROMSKY || sF.family == MA_WAVE))
            DrawCharge(frame, sF.c);
    }
    for (i = 0; i < MAX_BURSTS; i++)
        if (sM.bursts[i].strength)
            DrawBurst(frame, &sM.bursts[i], c);

    // the shake: the scene a pixel or two aside for a few frames (the UI stays still: below)
    if (sM.shake)
    {
        static u16 sRowCopy[W];
        int dx = (sM.shake & 1) ? sM.shakeAmp : -sM.shakeAmp, y, x;
        if (sM.shake <= 2)
            dx = dx > 0 ? 1 : -1;
        for (y = 0; y < H; y++)
        {
            u16 *row = frame + y * W;
            const u8 *ui = sUiLayer + y * W;
            memcpy(sRowCopy, row, sizeof(sRowCopy));
            for (x = 0; x < W; x++)
            {
                int sx = x - dx;
                if (!ui[x] && sx >= 0 && sx < W && !ui[sx])
                    row[x] = sRowCopy[sx];
            }
        }
        sM.shake--;
    }
    // nothing of MODERN over the battle's UI: its pixels as the game drew them
    for (i = 0; i < W * H; i++)
        if (sUiLayer[i])
            frame[i] = sSnap[i];
    {
        u32 n = 0, k;
        for (k = 0; k < W * H; k++)
            n += sFxLayer[k] >> 15;
        sM.fxPixels += n;
        if (n > sM.fxPeak)
            sM.fxPeak = n;
    }
    memset(sFxLayer, 0, sizeof(sFxLayer));   // the renderer fills it again next frame
    sM.frame++;
}
