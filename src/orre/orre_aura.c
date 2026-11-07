// Moemon Kai: the Shadow aura (docs/SHADOW_POKEMON.md "S3c"), drawn over the finished frame (sdl2.c VDraw) like the
// other effects layers (mp_astralfx.c): no sprite or palette is used and the Pokémon's picture is never changed.
//  - Around a Shadow Pokémon in battle: a dark halo hugging its outline and tongues of dark smoke licking up from
//    its tops (their shapes moving up), violet at their edges with bright flickers inside, and wisps torn from them.
//    Nothing is drawn over the Pokémon itself: the aura comes from behind it. Its pixels are found by its sprite
//    palette's colours inside its box, so the send-out's growing, a bob or a move's shake are followed.
//  - Faint while it's out; a burst when Colosseum plays its aura (fight_out_pokemon_suffix_80207C6C.c
//    fightOutPokemonDarkPokemonEffect: on being sent out, entering HYPER MODE, ignoring orders, the HYPER MODE
//    reminder at the end of a turn); in HYPER MODE it burns crimson and stays stronger.
// Its own random numbers: the game's are never touched.
#include <string.h>
#include "global.h"
#include "battle.h"
#include "main.h"
#include "mp.h"
#include "orre_shadow.h"
#include "palette.h"
#include "sprite.h"
#include "constants/rgb.h"

#define MAX_WISPS 64
#define FX 16           // wisp positions in 1/16 pixels
#define PAD 6           // the halo reaches this far past the outline
#define TOP 28          // the flames rise this far above it (HYPER MODE's; 20 otherwise)
#define BOX_W (128 + 2 * PAD)
#define BOX_H (128 + TOP + PAD)
#define TEXT_TOP 112    // the battle text box stays clear

struct Wisp { s32 x, y; s16 vx; u8 age, life, size, battler; bool8 on, hyper; };

static struct Wisp sWisps[MAX_WISPS];
static u16 sBurst[MAX_BATTLERS_COUNT];      // a burst's strength (0..256), fading
static bool8 sBurstHyper[MAX_BATTLERS_COUNT];
static u32 sSpawnAcc[MAX_BATTLERS_COUNT];   // spawn rate's carry (1/256 wisps)
static u32 sSeen[MAX_BATTLERS_COUNT];       // the Shadow Pokémon each battler showed last (its personality): a new one
                                            // sent out flares (Colosseum plays its aura then)
static u16 sTime;
static u32 sRng = 0x5AD0A0A1;
static u8 sDist[BOX_W * BOX_H];             // each pixel's distance from the Pokémon (0 = the Pokémon itself)
static u8 sBelow[BOX_W * BOX_H];            // how far above the Pokémon it is, in its column (255: nothing below)
static u16 sEdge[512][2];
static u8 sUi[TEXT_TOP][DISPLAY_WIDTH / 8];  // the UI on screen (the healthboxes): never drawn over (the user, 2026-11-07)
static s8 sShakeX, sShakeY;                 // HYPER MODE's shake this frame

static u32 Rand(void)
{
    sRng = sRng * 1103515245 + 12345;
    return sRng >> 16;
}

static const u8 sDims[3][4][2] = {
    {{8, 8}, {16, 16}, {32, 32}, {64, 64}},
    {{16, 8}, {32, 8}, {32, 16}, {64, 32}},
    {{8, 16}, {8, 32}, {16, 32}, {32, 64}},
};

static bool8 SpriteBox(u8 id, s16 *x, s16 *y, s16 *w, s16 *h)
{
    struct Sprite *s = &gSprites[id];
    if (id >= MAX_SPRITES || !s->inUse || s->invisible || s->oam.shape > 2)
        return FALSE;
    *w = sDims[s->oam.shape][s->oam.size][0];
    *h = sDims[s->oam.shape][s->oam.size][1];
    if ((s->oam.affineMode & ST_OAM_AFFINE_DOUBLE_MASK) == ST_OAM_AFFINE_DOUBLE_MASK)
        *w *= 2, *h *= 2;
    *x = s->oam.x;
    *y = s->oam.y;
    if (*x >= 256)
        *x -= 512;
    if (*y + *h > 256)
        *y -= 256;
    return *w + 2 * PAD <= BOX_W && *h + TOP + PAD <= BOX_H
        && *x + *w > 0 && *x < DISPLAY_WIDTH && *y + *h > 0 && *y < TEXT_TOP;
}

static u16 Mix(u16 dst, u16 src, u8 a)   // a: 0..16
{
    u16 r = ((dst & 31) * (16 - a) + (src & 31) * a) / 16;
    u16 g = (((dst >> 5) & 31) * (16 - a) + ((src >> 5) & 31) * a) / 16;
    u16 b = (((dst >> 10) & 31) * (16 - a) + ((src >> 10) & 31) * a) / 16;
    return r | (g << 5) | (b << 10);
}

// a cheap hash for the flames' shapes and flicker
static u8 Noise(s16 x, s16 y, u16 t)
{
    u32 h = (u32)(x * 374761393) ^ (u32)(y * 668265263) ^ (u32)(t * 2246822519u);
    h = (h ^ (h >> 13)) * 1274126177u;
    return (h >> 24) & 0xFF;
}

// a smooth noise (0..255) between the hash's lattice points SX x SY apart
static u8 Smooth(s16 x, s16 y, u8 sx, u8 sy)
{
    s16 gx = x >= 0 ? x / sx : (x - sx + 1) / sx, gy = y >= 0 ? y / sy : (y - sy + 1) / sy;
    s16 fx = x - gx * sx, fy = y - gy * sy;
    u32 a = Noise(gx, gy, 7), b = Noise(gx + 1, gy, 7), c = Noise(gx, gy + 1, 7), d = Noise(gx + 1, gy + 1, 7);
    u32 top = a * (sx - fx) + b * fx, bot = c * (sx - fx) + d * fx;
    return (top * (sy - fy) + bot * fy) / (sx * sy);
}

static bool8 BattlerShadow(u8 battler)
{
    return battler < gBattlersCount && gBattleMons[battler].hp != 0
        && Orre_Shadow_IsKey(gBattleMons[battler].otId, gBattleMons[battler].personality);
}

void Orre_Aura_Burst(u8 battler, u8 kind)
{
    if (battler >= MAX_BATTLERS_COUNT)
        return;
    sBurst[battler] = 256;
    sBurstHyper[battler] = kind == ORRE_AURA_HYPER || Orre_Shadow_InHyper(battler);
}

// the aura's colours: the dark smoke (near the Pokémon), its violet edge and the bright flickers in the flames;
// crimson in HYPER MODE
static void Colours(bool8 hyper, u16 *core, u16 *rim, u16 *spark)
{
    *core = hyper ? RGB(10, 0, 6) : RGB(4, 1, 9);
    *rim = hyper ? RGB(28, 2, 12) : RGB(13, 4, 22);
    *spark = hyper ? RGB(31, 10, 16) : RGB(22, 10, 31);
}

// the healthboxes (both halves and the HP bar) of every battler, marked so the aura never covers them
static void MarkUi(void)
{
    u8 b, k;
    memset(sUi, 0, sizeof(sUi));
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        u8 ids[3];
        ids[0] = gHealthboxSpriteIds[b];
        if (ids[0] >= MAX_SPRITES || !gSprites[ids[0]].inUse)
            continue;
        ids[1] = gSprites[ids[0]].oam.affineParam;
        ids[2] = gSprites[ids[0]].data[5];
        for (k = 0; k < 3; k++)
        {
            s16 x, y, w, h, i, j;
            if (ids[k] >= MAX_SPRITES || !SpriteBox(ids[k], &x, &y, &w, &h))
                continue;
            for (j = y < 0 ? 0 : y; j < y + h && j < TEXT_TOP; j++)
                for (i = x < 0 ? 0 : x; i < x + w && i < DISPLAY_WIDTH; i++)
                    sUi[j][i >> 3] |= 1 << (i & 7);
        }
    }
}

static void Put(u16 *frame, s16 x, s16 y, u16 colour, u8 a)
{
    u16 *dst;
    if (x < 0 || x >= DISPLAY_WIDTH || y < 0 || y >= TEXT_TOP || a == 0 || (sUi[y][x >> 3] >> (x & 7) & 1))
        return;
    dst = &frame[y * DISPLAY_WIDTH + x];
    *dst = Mix(*dst, colour, a > 14 ? 14 : a) | 0x8000;
}

static bool8 DrawBattler(u16 *frame, u8 battler, u16 strength, bool8 hyper)
{
    u8 id = gBattlerSpriteIds[battler], i, n, halo, flame;
    s16 bx, by, bw, bh, x, y, W, H, x0, y0;
    const u16 *pal;
    u16 colours[15], core, rim, spark, edges = 0;
    u32 count = 0;

    if (!SpriteBox(id, &bx, &by, &bw, &bh) || gSprites[id].data[2] != gBattleMons[battler].species)
        return FALSE;   // (not out yet: the slot's sprite is the trainer's until the send-out sets its species)
    pal = (const u16 *)(PLTT + 0x200) + gSprites[id].oam.paletteNum * 16;
    for (i = 0; i < 15; i++)
        colours[i] = pal[i + 1] & 0x7FFF;
    x0 = bx - PAD, y0 = by - TOP, W = bw + 2 * PAD, H = bh + TOP + PAD;

    // the Pokémon's own pixels (distance 0), then each pixel's distance from them (two-pass chamfer) and, in its
    // column, how far above them it is
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
        {
            s16 fx = x0 + x, fy = y0 + y;
            u8 d = 255;
            if (x >= PAD && y >= TOP && x < PAD + bw && y < TOP + bh && fx >= 0 && fy >= 0 && fx < DISPLAY_WIDTH
             && fy < DISPLAY_HEIGHT)
            {
                u16 c = frame[fy * DISPLAY_WIDTH + fx] & 0x7FFF;
                for (i = 0; i < 15; i++)
                    if (colours[i] == c)
                    {
                        d = 0;
                        count++;
                        break;
                    }
            }
            sDist[y * W + x] = d;
        }
    if (count < 24)
        return FALSE;   // (not on screen yet, or faded out)
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
        {
            u8 *d = &sDist[y * W + x];
            if (x > 0 && d[-1] + 1 < *d)
                *d = d[-1] + 1;
            if (y > 0 && d[-W] + 1 < *d)
                *d = d[-W] + 1;
        }
    for (y = H - 1; y >= 0; y--)
        for (x = W - 1; x >= 0; x--)
        {
            u8 *d = &sDist[y * W + x], *up = &sBelow[y * W + x];
            if (x < W - 1 && d[1] + 1 < *d)
                *d = d[1] + 1;
            if (y < H - 1 && d[W] + 1 < *d)
                *d = d[W] + 1;
            if (*d == 0)
                *up = 0;
            else if (y == H - 1 || up[W] == 255)
                *up = 255;
            else
                *up = up[W] + 1;
            if (*up == 1 && edges < ARRAY_COUNT(sEdge))
                sEdge[edges][0] = x, sEdge[edges++][1] = y;   // (the outline's tops: where the flames rise from)
        }

    // the halo hugging the outline and the flames licking up from its tops: tongues of dark smoke, their shapes
    // moving up (a smooth noise scrolled with time), violet at their edges, bright flickers inside
    Colours(hyper, &core, &rim, &spark);
    halo = 2 + (strength * 3) / 256;
    flame = 4 + (strength * ((hyper ? TOP : 20) - 4)) / 256;
    for (y = 0; y < H; y++)
    {
        s16 fy = y0 + y;
        if (fy < 0 || fy >= TEXT_TOP)
            continue;
        for (x = 0; x < W; x++)
        {
            s16 fx = x0 + x, heat = 0, lx = x, ly = y;
            u8 d, up;
            // (HYPER MODE: the whole aura shakes)
            if (hyper)
            {
                lx = x - sShakeX, ly = y - sShakeY;
                if (lx < 0 || ly < 0 || lx >= W || ly >= H || sDist[y * W + x] == 0)
                    continue;
            }
            d = sDist[ly * W + lx], up = sBelow[ly * W + lx];
            if (d == 0 || fx < 0 || fx >= DISPLAY_WIDTH)
                continue;
            if (d <= halo)
                heat = 256 * (halo + 1 - d) / (halo + 1);
            if (up != 255 && up <= flame)
            {
                // a tongue's height here: a smooth noise in bands of columns, moving up with time (in HYPER MODE
                // thinner, jagged and racing up)
                s16 tongue = hyper ? flame * (24 + Smooth(fx, fy + sTime * 2, 2, 4)) / 280
                                   : flame * (40 + Smooth(fx, fy + sTime / 2, 3, 6)) / 296, h;
                if (up <= tongue)
                {
                    h = 256 * (tongue + 1 - up) / (tongue + 1);
                    if (h > heat)
                        heat = h;
                }
            }
            if (heat <= 0)
                continue;
            heat = heat * (strength + 128) / 384;   // (faint while it's out)
            if (heat > 150 && Noise(fx, fy, hyper ? sTime : sTime / 3) > (hyper ? 225 : 240))
                Put(frame, fx, fy, spark, 12);
            else
                Put(frame, fx, fy, heat > 110 ? core : rim, 2 + heat / 18);
        }
    }

    // wisps of smoke torn from the flames' tops
    sSpawnAcc[battler] += 10 + strength / 2 + (hyper ? 160 : 0);
    n = 0;
    while (sSpawnAcc[battler] >= 256 && edges)
    {
        sSpawnAcc[battler] -= 256;
        for (i = 0; i < MAX_WISPS && sWisps[i].on; i++)
            ;
        if (i == MAX_WISPS || ++n > 3)
            break;
        {
            u16 e = Rand() % edges;
            struct Wisp *w = &sWisps[i];
            w->on = TRUE;
            w->battler = battler;
            w->hyper = hyper;
            w->x = (x0 + sEdge[e][0]) * FX;
            w->y = (y0 + sEdge[e][1] - flame / 2) * FX;
            w->vx = hyper ? (s16)(Rand() % 13) - 6 : (s16)(Rand() % 7) - 3;
            w->age = 0;
            w->life = hyper ? 10 + Rand() % 10 : 16 + Rand() % 14;
            w->size = 2 + (strength > 160);
        }
    }
    for (i = 0; i < MAX_WISPS; i++)
    {
        struct Wisp *w = &sWisps[i];
        s16 cx, cy, dx, dy, r;
        u8 fade;
        if (!w->on || w->battler != battler)
            continue;
        if (++w->age >= w->life)
        {
            w->on = FALSE;
            continue;
        }
        w->y -= w->hyper ? 22 : 10;   // (HYPER MODE's thrown up hard)
        w->x += w->vx + ((Noise(i, w->age >> 2, 0) & 7) - 3);
        cx = w->x / FX, cy = w->y / FX;
        fade = 16 - (w->age * 16) / w->life;
        r = (w->size * (fade + 8) + 23) / 24;   // (a puff of 2 x 2 pixels, or a rounded 3 x 3 / 5 x 5)
        Colours(w->hyper, &core, &rim, &spark);
        for (dy = -r; dy <= r; dy++)
            for (dx = -r; dx <= r; dx++)
            {
                s16 px = cx + dx, py = cy + dy, lx = px - x0, ly = py - y0, dd = dx * dx + dy * dy;
                if (r <= 1 ? dx < 0 || dy < 0 : dd > r * r)
                    continue;
                if (lx >= 0 && ly >= 0 && lx < W && ly < H && sDist[ly * W + lx] == 0)
                    continue;   // (behind the Pokémon)
                if (px < x0 || px >= x0 + W || py < y0 - 8)
                    continue;   // (kept near it: the healthboxes stay clear)
                if (w->hyper && dx == 0 && dy == 0 && (i & 3) == 0)
                    Put(frame, px, py, spark, (fade * 14) / 16);   // (a spark among them)
                else
                    Put(frame, px, py, dd * 2 < r * r ? core : rim, (fade * (dd * 2 < r * r ? 9 : 5)) / 16);
            }
    }
    return TRUE;
}

void Orre_Aura_Composite(u16 *frame)
{
    u8 b;
    bool8 any = FALSE;

    if (!gMain.inBattle || !MP_OBJ_LAYER_SHOWN())
    {
        if (!gMain.inBattle)
        {
            memset(sWisps, 0, sizeof(sWisps));
            memset(sBurst, 0, sizeof(sBurst));
            memset(sSeen, 0, sizeof(sSeen));
        }
        return;
    }
    sTime++;
    MarkUi();
    sShakeX = (s8)(Rand() % 3) - 1;
    sShakeY = (s8)(Rand() % 3) - 1;
    for (b = 0; b < gBattlersCount && b < MAX_BATTLERS_COUNT; b++)
    {
        bool8 hyper;
        u16 strength;
        if (!BattlerShadow(b))
        {
            u8 i;
            sBurst[b] = 0;
            sSeen[b] = 0;
            for (i = 0; i < MAX_WISPS; i++)
                if (sWisps[i].battler == b)
                    sWisps[i].on = FALSE;
            continue;
        }
        any = TRUE;
        hyper = Orre_Shadow_InHyper(b);
        strength = 90;   // (faint while it's out)
        if (hyper)
        {
            // HYPER MODE: violent: throbbing, and sudden flare-ups
            u8 t = sTime % 24, throb = t < 12 ? t : 24 - t;
            strength = 150 + throb * 6;
            if (Rand() % 14 == 0 && sBurst[b] < 210)
                sBurst[b] = 210, sBurstHyper[b] = TRUE;
        }
        if (sBurst[b] > strength)
        {
            strength = sBurst[b];
            hyper |= sBurstHyper[b];
        }
        if (sBurst[b] >= 2)
            sBurst[b] -= 2;
        else
            sBurst[b] = 0;
        // a new Shadow Pokémon flares the first frame it's seen (sent out: its picture on screen)
        if (DrawBattler(frame, b, strength, hyper) && sSeen[b] != gBattleMons[b].personality)
        {
            sSeen[b] = gBattleMons[b].personality;
            Orre_Aura_Burst(b, ORRE_AURA_APPEAR);
        }
    }
    if (!any)
        memset(sWisps, 0, sizeof(sWisps));
}
