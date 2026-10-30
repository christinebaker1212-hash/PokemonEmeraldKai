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
    u16 lastBurst;
    bool8 hadBurst;
    bool8 quaked;
    struct Burst bursts[MAX_BURSTS];
    u8 shake;                // frames of shake left
    u8 shakeAmp;
    u8 trail[HH][HW][3];     // fading afterimages of the effect sprites (0-255 a channel)
    u16 glow[HH][HW][3];     // scratch: the blurred glow
    u32 seed;
} sM;

// ---------------------------------------------------------------------------
// Colors

struct Rgb { u8 r, g, b; };

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

// battle_anim.c LaunchBattleAnimation
void MP_ModernAnim_Begin(const u8 *const table[], u16 id)
{
    memset(&sM.bursts, 0, sizeof(sM.bursts));
    sM.kind = KIND_NONE;
    if (!MP_ModernAnim_On())
        return;
    sM.id = id;
    sM.frame = 0;
    sM.hadBurst = FALSE;
    sM.quaked = FALSE;
    sM.shake = 0;
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
        {
            u16 arg = gBattleSpritesDataPtr != NULL && gBattleSpritesDataPtr->animationData != NULL
                    ? gBattleSpritesDataPtr->animationData->animArg & 0xFF : 0;
            sM.statDown = (arg >= STAT_ANIM_MINUS1 && arg < STAT_ANIM_MULTIPLE_PLUS1)
                       || arg == STAT_ANIM_MULTIPLE_MINUS1 || arg == STAT_ANIM_MULTIPLE_MINUS2;
            sM.type = sM.statDown ? TYPE_WATER : TYPE_FIRE;   // cool blue going down, warm orange going up
            break;
        }
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
    sM.lastBurst = sM.frame;
    sM.hadBurst = TRUE;
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
            sM.kind = KIND_NONE;   // over: nothing outlives the Classic animation
        if (gMpFxLayer != NULL)
        {
            gMpFxLayer = NULL;
            memset(sM.trail, 0, sizeof(sM.trail));
        }
        return;
    }
    if (gMpFxLayer == NULL)
    {
        memset(sFxLayer, 0, sizeof(sFxLayer));
        memset(sM.trail, 0, sizeof(sM.trail));
        gMpFxLayer = sFxLayer;   // from the next frame the renderer marks the effect sprites
    }
    c = sTypeColor[sM.type < NUMBER_OF_MON_TYPES ? sM.type : TYPE_NORMAL];

    GlowAndTrails(frame, c);
    if (sM.kind == KIND_MOVE && sM.attacker != sM.target)
        DrawCharge(frame, c);
    if (sM.kind == KIND_GENERAL && sM.id == B_ANIM_STATS_CHANGE)
        DrawStatChevrons(frame, sM.attacker, sM.statDown, c);
    for (i = 0; i < MAX_BURSTS; i++)
        if (sM.bursts[i].strength)
            DrawBurst(frame, &sM.bursts[i], c);

    // the shake: the whole picture a pixel or two aside, for a few frames
    if (sM.shake)
    {
        int dx = (sM.shake & 1) ? sM.shakeAmp : -sM.shakeAmp, y;
        if (sM.shake <= 2)
            dx = dx > 0 ? 1 : -1;
        for (y = 0; y < H; y++)
        {
            u16 *row = frame + y * W;
            if (dx > 0)
            {
                memmove(row + dx, row, (W - dx) * sizeof(u16));
            }
            else
            {
                memmove(row, row - dx, (W + dx) * sizeof(u16));
            }
        }
        sM.shake--;
    }
    memset(sFxLayer, 0, sizeof(sFxLayer));   // the renderer fills it again next frame
    sM.frame++;
}
