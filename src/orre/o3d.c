// Moemon Kai: ORRE's 3D engine (docs/ORRE_DECISIONS.md). Its own engine, cloned from SINNOH's (src/sinnoh/s3d.c:
// the camera, the rasterizer, sprites standing in the scene, fades) and kept apart from it, so either region can be
// changed without touching the other. An ORRE map is a map of the field engine like any other (grid movement,
// scripts, warps, co-op), but what is shown of it is Pokemon Colosseum's own 3D world: its map model and textures,
// drawn by this software renderer into the game's 240x160 picture each frame. The renderer (gba_easy_draw.c) hands
// each line of the field's BG layers to O3dLine, which puts the 3D picture in the bottom layer and clears the map's
// other layers; the text windows (BG0) and the sprites go on top. Field sprites are moved to where their spot on the
// ground is in the 3D view (sprite.c UpdateOamCoords -> O3D_PlaceSprite).
//
// The world comes from ORRE's pack, orre/orre3d.pak next to the game (MoemonOrre.zip; tools/orre/make_pack.sh rips it
// from the disc, tools/orre/build_pack.py has the format): its textures in GameCube GX encoding, then each map's model
// sets and tile grid, deflated. A map is read the first time it's visited and kept: its textures decoded
// (src/orre/gx_tex.c) and halved until they're at most TEX_MAX wide (Colosseum drew them at 640 x 480; here the view
// is 240 x 160), its meshes in this engine's form, and its grid: each tile's collision and ground heights, which
// O3D_LayoutBlocks gives the map loader in place of the layout's placeholders. Colosseum's units become 16 / tile here
// (a tile is 16 units as on the DS, and its grid's `tile`, 8 Colosseum units, there).
//
// The camera is Platinum's default field camera (666.92 units away, 59.05 degrees down) at the DS's own pixel scale,
// as SINNOH's: one unit a pixel at the target, 16 pixels a tile. It follows Emerald's own camera.
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "gpu_regs.h"
#include "main.h"
#include "overworld.h"
#include "o3d.h"
#include "orre.h"
#include "gx_tex.h"
#include "palette.h"
#include "sprite.h"
#include "constants/event_objects.h"
#include "constants/maps.h"
bool8 FieldEffect_IsReflectionSprite(const struct Sprite *sprite);   // (field_effect_helpers.c)

extern void (*gMpO3dLine)(int line, u16 layers[4][DISPLAY_WIDTH]);
extern bool (*gMpO3dOamHidden)(int oamIndex, int x, int line);
const char *Updater_OrreRoot(void);
unsigned long long SDL_GetPerformanceCounter(void);
unsigned long long SDL_GetPerformanceFrequency(void);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);   // (zlib)

#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define CAM_DIST 666.922119140625f
#define CAM_PITCH 59.051513671875f
#define HALF_FOV 8.0914306640625f
#define TEX_MAX 256
#define BLOCKED (0x0001 | (1 << 10) | (3 << 12))            // metatile 1, collision, elevation 3
#define WALK ((O3D_METATILE_BEHAVIOR + 0) | (3 << 12))      // a plain tile, elevation 3 (+ its behavior)

#include "orre_maps.h"   // sOrreMaps: each ORRE map, its file in the pack, size in blocks, start tile (tools/orre/gen_maps.py)

// ------------------------------------------------------------------------------------------------ the pack
typedef struct { float x, y, z, u, v; u8 r, g, b, pad; } OVtx;
typedef struct
{
    s16 tex;
    u8 rs, rt, fs, ft, alpha, pid;
    u32 count;     // vertices (3 a triangle)
    OVtx *v;
    float c[3], r; // the mesh's centre and radius (skipped off screen)
} OMesh;
typedef struct { u16 w, h; u32 *px; } OTex;   // r | g << 8 | b << 16 | a << 24

typedef struct
{
    bool8 tried, ok;
    u16 w, h;                 // tiles
    u16 *blocks;              // the field engine's map blocks (metatile | collision << 10 | elevation << 12)
    u8 *hn;                   // the ground's heights at each tile's centre: how many (more than one: a bridge)
    float (*hgt)[3];          // ... lowest first
    u32 nmesh;
    OMesh *mesh;
    float tile, x0, z0;       // Colosseum's units: a tile's size, the grid's corner (O3D_TileToColo)
} OMap;

typedef struct { u8 fmt, tfmt; u16 w, h, n; u32 off, size; } OTexEntry;
static struct
{
    bool8 tried, ok;
    char path[1200];
    u32 ntex, nmaps, base;
    OTexEntry *entry;
    struct { char name[32]; u32 off, zsize, size; } *maps;
    OTex *tex;                // decoded (w == 0: not yet)
    OMap map[ARRAY_COUNT(sOrreMaps) ? ARRAY_COUNT(sOrreMaps) : 1];
} sPack;

static bool8 PackOpen(void)
{
    FILE *f;
    u8 head[16];
    u32 i;
    if (sPack.tried)
        return sPack.ok;
    sPack.tried = TRUE;
    if (getenv("MOEMON_ORRE_PAK"))
        snprintf(sPack.path, sizeof(sPack.path), "%s", getenv("MOEMON_ORRE_PAK"));
    else
        snprintf(sPack.path, sizeof(sPack.path), "%s/orre3d.pak", Updater_OrreRoot());
    if ((f = fopen(sPack.path, "rb")) == NULL)
    {
        printf("[o3d] no ORRE pack at %s\n", sPack.path);
        return FALSE;
    }
    if (fread(head, 1, 16, f) != 16 || memcmp(head, "ORZ3", 4) != 0 || head[4] != 2)
    {
        printf("[o3d] %s isn't an ORRE pack of version 2\n", sPack.path);
        fclose(f);
        return FALSE;
    }
    memcpy(&sPack.ntex, head + 8, 4);
    memcpy(&sPack.nmaps, head + 12, 4);
    sPack.entry = calloc(sPack.ntex ? sPack.ntex : 1, sizeof(OTexEntry));
    sPack.maps = calloc(sPack.nmaps ? sPack.nmaps : 1, sizeof(*sPack.maps));
    sPack.tex = calloc(sPack.ntex ? sPack.ntex : 1, sizeof(OTex));
    for (i = 0; i < sPack.ntex; i++)
    {
        u8 b[16];
        if (fread(b, 1, 16, f) != 16)
            break;
        sPack.entry[i].fmt = b[0], sPack.entry[i].tfmt = b[1];
        memcpy(&sPack.entry[i].w, b + 2, 2);
        memcpy(&sPack.entry[i].h, b + 4, 2);
        memcpy(&sPack.entry[i].n, b + 6, 2);
        memcpy(&sPack.entry[i].off, b + 8, 4);
        memcpy(&sPack.entry[i].size, b + 12, 4);
    }
    for (i = 0; i < sPack.nmaps; i++)
    {
        u8 b[44];
        if (fread(b, 1, 44, f) != 44)
            break;
        memcpy(sPack.maps[i].name, b, 32);
        sPack.maps[i].name[31] = 0;
        memcpy(&sPack.maps[i].off, b + 32, 4);
        memcpy(&sPack.maps[i].zsize, b + 36, 4);
        memcpy(&sPack.maps[i].size, b + 40, 4);
    }
    fclose(f);
    if (i < sPack.nmaps)
    {
        printf("[o3d] %s is cut short\n", sPack.path);
        return FALSE;
    }
    sPack.base = 16 + sPack.ntex * 16 + sPack.nmaps * 44;
    sPack.ok = TRUE;
    printf("[o3d] %s: %u textures, %u maps\n", sPack.path, sPack.ntex, sPack.nmaps);
    return TRUE;
}

static u8 *PackRead(u32 off, u32 size)
{
    FILE *f = fopen(sPack.path, "rb");
    u8 *d = malloc(size ? size : 1);
    if (f == NULL || d == NULL || fseek(f, (long)(sPack.base + off), SEEK_SET) != 0 || fread(d, 1, size, f) != size)
    {
        if (f)
            fclose(f);
        free(d);
        return NULL;
    }
    fclose(f);
    return d;
}

// A texture, decoded the first time a map uses it, then halved (alpha-weighted) until it fits TEX_MAX
static s16 Texture(s32 t)
{
    const OTexEntry *e;
    u8 *raw;
    u32 *px, w, h;
    if (t < 0 || (u32)t >= sPack.ntex)
        return -1;
    if (sPack.tex[t].w)
        return (s16)t;
    e = &sPack.entry[t];
    raw = PackRead(e->off, e->size);
    w = e->w, h = e->h;
    px = malloc(4 * w * h);
    if (raw == NULL || px == NULL
        || !GxTex_Decode(e->fmt, e->w, e->h, raw, e->size - e->n * 2, e->tfmt, raw + e->size - e->n * 2, e->n, px))
    {
        free(raw);
        free(px);
        return -1;
    }
    free(raw);
    while ((w > TEX_MAX || h > TEX_MAX) && w >= 2 && h >= 2)
    {
        u32 nw = w / 2, nh = h / 2, x, y, k;
        u32 *half = malloc(4 * nw * nh);
        for (y = 0; y < nh; y++)
            for (x = 0; x < nw; x++)
            {
                u32 sum[3] = {0, 0, 0}, a = 0;
                for (k = 0; k < 4; k++)
                {
                    u32 c = px[(2 * y + (k >> 1)) * w + 2 * x + (k & 1)], ca = c >> 24;
                    sum[0] += (c & 255) * ca, sum[1] += ((c >> 8) & 255) * ca, sum[2] += ((c >> 16) & 255) * ca;
                    a += ca;
                }
                half[y * nw + x] = a ? (sum[0] / a) | (sum[1] / a) << 8 | (sum[2] / a) << 16 | (a / 4) << 24 : 0;
            }
        free(px);
        px = half, w = nw, h = nh;
    }
    sPack.tex[t].w = (u16)w, sPack.tex[t].h = (u16)h, sPack.tex[t].px = px;
    return (s16)t;
}

typedef struct { const u8 *p; u32 left; bool8 bad; } Reader;

static void Take(Reader *r, void *dst, u32 n)
{
    if (r->bad || r->left < n)
    {
        r->bad = TRUE;
        memset(dst, 0, n);
        return;
    }
    memcpy(dst, r->p, n);
    r->p += n;
    r->left -= n;
}

static const u8 *Skip(Reader *r, u32 n)
{
    const u8 *at = r->p;
    if (r->bad || r->left < n)
    {
        r->bad = TRUE;
        return NULL;
    }
    r->p += n;
    r->left -= n;
    return at;
}

// A map's world: its grid, then its meshes in the grid's world ((x - x0, y, z - z0) * 16 / tile)
static bool8 MapLoad(u32 i)
{
    OMap *om = &sPack.map[i];
    u32 m, set, ns, nm, mi, has, x, y;
    s32 pm = -1;
    unsigned long rawSize;
    u8 *z, *b;
    Reader r, meshes;
    u16 gw, gh;
    float tile, x0, z0, S;
    const u8 *cells;
    if (i >= ARRAY_COUNT(sOrreMaps))
        return FALSE;
    if (om->tried)
        return om->ok;
    om->tried = TRUE;
    if (!PackOpen())
        return FALSE;
    for (m = 0; m < sPack.nmaps; m++)
        if (strcmp(sPack.maps[m].name, sOrreMaps[i].file) == 0)
            pm = (s32)m;
    if (pm < 0 || (z = PackRead(sPack.maps[pm].off, sPack.maps[pm].zsize)) == NULL)
    {
        printf("[o3d] ORRE's pack has no %s\n", sOrreMaps[i].file);
        return FALSE;
    }
    rawSize = sPack.maps[pm].size;
    b = malloc(rawSize ? rawSize : 1);
    if (b == NULL || uncompress(b, &rawSize, z, sPack.maps[pm].zsize) != 0 || rawSize != sPack.maps[pm].size)
    {
        free(z);
        free(b);
        printf("[o3d] ORRE's %s won't inflate\n", sOrreMaps[i].file);
        return FALSE;
    }
    free(z);
    // past the meshes to the grid (its tile size scales the meshes)
    r.p = b, r.left = (u32)rawSize, r.bad = FALSE;
    Take(&r, &ns, 4);
    meshes = r;
    for (set = 0; set < ns && !r.bad; set++)
    {
        Skip(&r, 4);
        Take(&r, &nm, 4);
        for (mi = 0; mi < nm && !r.bad; mi++)
        {
            u8 head[36];
            u32 nv, ni;
            Take(&r, head, 36);
            memcpy(&nv, head + 28, 4);
            memcpy(&ni, head + 32, 4);
            Skip(&r, nv * 28 + ni * (nv > 65535 ? 4 : 2));
        }
    }
    Take(&r, &has, 4);
    Take(&r, &gw, 2);
    Take(&r, &gh, 2);
    Take(&r, &tile, 4);
    Take(&r, &x0, 4);
    Take(&r, &z0, 4);
    cells = Skip(&r, (u32)gw * gh * 16);
    if (r.bad || has != 1 || tile <= 0.0f)
    {
        free(b);
        printf("[o3d] ORRE's %s has no grid\n", sOrreMaps[i].file);
        return FALSE;
    }
    if (gw != sOrreMaps[i].bw * 32 || gh != sOrreMaps[i].bh * 32)
    {
        free(b);
        printf("[o3d] ORRE's %s isn't the size of its map (tools/orre/gen_maps.py)\n", sOrreMaps[i].file);
        return FALSE;
    }
    S = 16.0f / tile;
    om->w = gw, om->h = gh;
    om->tile = tile, om->x0 = x0, om->z0 = z0;
    om->blocks = malloc(sizeof(u16) * gw * gh);
    om->hn = calloc((u32)gw * gh, 1);
    om->hgt = calloc((u32)gw * gh, sizeof(*om->hgt));
    for (y = 0; y < gh; y++)
        for (x = 0; x < gw; x++)
        {
            const u8 *t = cells + ((u32)y * gw + x) * 16;
            u32 k = (u32)y * gw + x, n = t[2] > 3 ? 3 : t[2], j;
            om->blocks[k] = (t[0] & 1) ? (WALK + t[3]) : BLOCKED;   // (t[3]: the tile's behavior: a warp's)
            om->hn[k] = (u8)n;
            for (j = 0; j < n; j++)
            {
                float hh;
                memcpy(&hh, t + 4 + 4 * j, 4);
                om->hgt[k][j] = hh * S;
            }
        }
    // the meshes (the sky left out: Platinum's camera looks down)
    for (set = 0; set < ns && !meshes.bad; set++)
    {
        u8 kind[4];
        Take(&meshes, kind, 4);
        Take(&meshes, &nm, 4);
        for (mi = 0; mi < nm && !meshes.bad; mi++)
        {
            s32 tex;
            u32 rm, nv, ni, j, asum = 0, ch;
            u8 flags, wrap[3], diff[4];
            float us, vs, alpha, lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
            const u8 *vtx, *idx;
            OMesh *me;
            Take(&meshes, &tex, 4);
            Take(&meshes, &rm, 4);   // (HSD's render mode; see-through is in flags)
            Take(&meshes, &flags, 1);
            Take(&meshes, wrap, 3);
            Take(&meshes, &us, 4);
            Take(&meshes, &vs, 4);
            Take(&meshes, diff, 4);
            Take(&meshes, &alpha, 4);
            Take(&meshes, &nv, 4);
            Take(&meshes, &ni, 4);
            vtx = Skip(&meshes, nv * 28);
            idx = Skip(&meshes, ni * (nv > 65535 ? 4 : 2));
            (void)rm;
            if (meshes.bad || kind[0] == 2 || ni < 3)
                continue;
            om->mesh = realloc(om->mesh, sizeof(OMesh) * (om->nmesh + 1));
            me = &om->mesh[om->nmesh++];
            memset(me, 0, sizeof(*me));
            me->tex = tex >= 0 ? Texture(tex) : -1;
            me->rs = wrap[0] != 0, me->rt = wrap[1] != 0, me->fs = wrap[0] == 2, me->ft = wrap[1] == 2;
            me->count = ni - ni % 3;
            me->v = malloc(sizeof(OVtx) * me->count);
            for (j = 0; j < me->count; j++)
            {
                u32 vi = 0;
                const u8 *v;
                float p[5];
                OVtx *d = &me->v[j];
                if (nv > 65535)
                    memcpy(&vi, idx + 4 * j, 4);
                else
                {
                    u16 v16;
                    memcpy(&v16, idx + 2 * j, 2);
                    vi = v16;
                }
                if (vi >= nv)
                    vi = 0;
                v = vtx + vi * 28;
                memcpy(p, v, 20);
                d->x = (p[0] - x0) * S, d->y = p[1] * S, d->z = (p[2] - z0) * S;
                d->u = me->tex >= 0 ? p[3] * us * sPack.tex[me->tex].w : 0.0f;
                d->v = me->tex >= 0 ? p[4] * vs * sPack.tex[me->tex].h : 0.0f;
                // Colosseum's colours are lit (baked) at half strength: 128 is white (GX's TEV doubles them)
                for (ch = 0; ch < 3; ch++)
                {
                    u32 c = (flags & 8) ? v[20 + ch] : diff[ch], c5 = c * 2 * 31 / 255;
                    (&d->r)[ch] = (u8)(c5 > 31 ? 31 : c5);
                }
                asum += (flags & 8) ? v[23] : diff[3];
                for (ch = 0; ch < 3; ch++)
                {
                    float c = ch == 0 ? d->x : ch == 1 ? d->y : d->z;
                    lo[ch] = c < lo[ch] ? c : lo[ch];
                    hi[ch] = c > hi[ch] ? c : hi[ch];
                }
            }
            // see-through: the mesh's own alpha (its baked shadows are black at about half)
            me->alpha = 31;
            if (flags & 1)
            {
                float a = (me->count ? (float)asum / me->count / 255.0f : 1.0f) * (alpha < 1.0f ? alpha : 1.0f);
                s32 a31 = (s32)(a * 31.0f + 0.5f);
                me->alpha = (u8)(a31 >= 31 ? 30 : a31 < 1 ? 1 : a31);
                me->pid = (u8)(1 + (om->nmesh & 31));
            }
            for (ch = 0; ch < 3; ch++)
                me->c[ch] = (lo[ch] + hi[ch]) / 2;
            me->r = sqrtf((hi[0] - lo[0]) * (hi[0] - lo[0]) + (hi[1] - lo[1]) * (hi[1] - lo[1])
                          + (hi[2] - lo[2]) * (hi[2] - lo[2])) / 2;
        }
    }
    free(b);
    if (meshes.bad)
    {
        printf("[o3d] ORRE's %s is cut short\n", sOrreMaps[i].file);
        return FALSE;
    }
    om->ok = TRUE;
    printf("[o3d] ORRE's %s: %u meshes\n", sOrreMaps[i].file, om->nmesh);
    return TRUE;
}

static s32 MapIndex(u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreMaps); i++)
        if (sOrreMaps[i].map == ((group << 8) | num))
            return (s32)i;
    return -1;
}

// ORRE's pack is here and readable (looked for again while it isn't: the boat's download may have just put it there)
bool8 O3D_PackPresent(void)
{
    if (!sPack.ok)
        sPack.tried = FALSE;
    return PackOpen();
}

bool8 O3D_IsOrreGroup(u8 mapGroup)
{
    return ARRAY_COUNT(sOrreMaps) > 0 && mapGroup == MAP_GROUP(MAP_ORRE_OUTSKIRT_STAND);
}

// the way a player arriving at (x, y) of an ORRE map faces: the angle of the room's entry that lands there
// (sOrreArrivals: Colosseum's entry positions, tools/orre/gen_maps.py); DIR_NONE where no entry lands
u8 O3D_ArrivalDir(u16 map, s16 x, s16 y)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreArrivals); i++)
        if (sOrreArrivals[i].map == map && sOrreArrivals[i].x == x && sOrreArrivals[i].y == y)
            return sOrreArrivals[i].dir;
    return DIR_NONE;
}

// ------------------------------------------------------------------------------------------------ map blocks
const u16 *O3D_LayoutBlocks(const struct MapLayout *layout)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOrreMaps); i++)
    {
        const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(sOrreMaps[i].map >> 8, sOrreMaps[i].map & 0xFF);
        if (h->mapLayout != layout)
            continue;
        if (!MapLoad(i) || layout->width != sPack.map[i].w || layout->height != sPack.map[i].h)
            return layout->map;
        return sPack.map[i].blocks;
    }
    return layout->map;
}

// ------------------------------------------------------------------------------------------------ the ground's height
static const OMap *sCur;   // the map the player is on

// A tile's ground height (16 units a tile): of its heights, the one nearest `near` (an object on a bridge stays on
// it); `near` itself where it has none
static float TileHeight(s32 tx, s32 tz, float near)
{
    u32 t, i, best = 0;
    if (sCur == NULL || tx < 0 || tz < 0 || tx >= sCur->w || tz >= sCur->h)
        return near;
    t = (u32)tz * sCur->w + tx;
    if (sCur->hn[t] == 0)
        return near;
    for (i = 1; i < sCur->hn[t] && i < 3; i++)
        if (fabsf(sCur->hgt[t][i] - near) < fabsf(sCur->hgt[t][best] - near))
            best = i;
    return sCur->hgt[t][best];
}

// The ground's height under a point: the tile centres around it, blended (slopes as smooth as a walk shows them)
static float GroundAt(float wx, float wz, float near)
{
    float fx = (wx - 8.0f) / 16.0f, fz = (wz - 8.0f) / 16.0f;
    s32 tx = (s32)floorf(fx), tz = (s32)floorf(fz);
    float ax = fx - tx, az = fz - tz;
    float h00 = TileHeight(tx, tz, near), h10 = TileHeight(tx + 1, tz, near);
    float h01 = TileHeight(tx, tz + 1, near), h11 = TileHeight(tx + 1, tz + 1, near);
    return (h00 * (1 - ax) + h10 * ax) * (1 - az) + (h01 * (1 - ax) + h11 * ax) * az;
}

// ------------------------------------------------------------------------------------------------ camera
static struct
{
    bool8 active;
    s32 map;
    s32 camX, camY;          // Emerald's screen (0, 0) in the map's pixels
    float tx, ty, tz;        // the target (the player's tile centre, on the ground)
    float playerH;           // the player's ground height last frame (which layer of a bridge they're on)
    bool8 havePlayerH;
    u8 frame;
    float ax, ay;            // where the target is on the screen
    float focal;
    float right[3], up[3], fwd[3], eye[3];
} sView;

static u16 sColor[H][W];
static float sDepth[H][W];
static s8 sIds[H][W];
static s32 sFadeA[3], sFadeB[3];

static void FindCamera(void)
{
    u8 tileX, tileY;
    s32 hoffs = GetGpuReg(REG_OFFSET_BG3HOFS), voffs = GetGpuReg(REG_OFFSET_BG3VOFS);   // (this frame's, not yet shown)
    GetFieldCameraTileOffset(&tileX, &tileY);
    hoffs = (s8)((hoffs - tileX * 8) & 255);
    voffs = (s8)((voffs - tileY * 8) & 255);
    sView.camX = (gSaveBlock1Ptr->pos.x - 7) * 16 + hoffs;
    sView.camY = (gSaveBlock1Ptr->pos.y - 7) * 16 + voffs;
}

// A shot of Colosseum's playing (o3d_cam.c): the view looks through its camera, in the map's world ((x - x0, y,
// z - z0) * 16 / tile), the picture's centre the screen's, its vertical field of view over Kai's 160 lines
static void ShotView(void)
{
    float eye[3], tgt[3], roll, fov, S, f[3], r[3], u[3], len, cr, sr;
    u32 i;
    if (!sCur || !O3D_CamGet(eye, tgt, &roll, &fov))
        return;
    S = 16.0f / sCur->tile;
    eye[0] = (eye[0] - sCur->x0) * S, eye[1] *= S, eye[2] = (eye[2] - sCur->z0) * S;
    tgt[0] = (tgt[0] - sCur->x0) * S, tgt[1] *= S, tgt[2] = (tgt[2] - sCur->z0) * S;
    for (i = 0; i < 3; i++)
        f[i] = tgt[i] - eye[i];
    len = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    if (len < 1e-4f)
        return;
    for (i = 0; i < 3; i++)
        f[i] /= len;
    // right = forward x world up, up = right x forward (the field camera's own axes, as SetupView makes them)
    r[0] = -f[2], r[1] = 0.0f, r[2] = f[0];
    len = sqrtf(r[0] * r[0] + r[2] * r[2]);
    if (len < 1e-4f)
        r[0] = 1.0f, r[2] = 0.0f, len = 1.0f;
    r[0] /= len, r[2] /= len;
    u[0] = r[1] * f[2] - r[2] * f[1], u[1] = r[2] * f[0] - r[0] * f[2], u[2] = r[0] * f[1] - r[1] * f[0];
    cr = cosf(roll), sr = sinf(roll);   // (HSD's roll: radians about the line of sight)
    for (i = 0; i < 3; i++)
    {
        float ri = r[i] * cr + u[i] * sr, ui = u[i] * cr - r[i] * sr;
        sView.right[i] = ri, sView.up[i] = ui, sView.fwd[i] = f[i], sView.eye[i] = eye[i];
    }
    sView.ax = DISPLAY_WIDTH / 2, sView.ay = DISPLAY_HEIGHT / 2;
    sView.focal = (DISPLAY_HEIGHT / 2) / tanf(fov * 0.5f * 3.14159265f / 180.0f);
}

static void SetupView(void)
{
    const struct Sprite *s = &gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId];
    u8 cam;
    float p = CAM_PITCH * 3.14159265f / 180.0f;
    float sp = sinf(p), cp = cosf(p);
    // (a scene's camera object: the view follows it until it's removed)
    for (cam = 0; cam < OBJECT_EVENTS_COUNT; cam++)
        if (gObjectEvents[cam].active && gObjectEvents[cam].localId == OBJ_EVENT_ID_CAMERA)
        {
            s = &gSprites[gObjectEvents[cam].spriteId];
            break;
        }
    sView.ax = s->x + s->x2 + gSpriteCoordOffsetX;
    sView.ay = s->y + s->y2 + gSpriteCoordOffsetY - s->centerToCornerVecY - 8;
    sView.tx = sView.camX + sView.ax;
    sView.tz = sView.camY + sView.ay;
    sView.ty = GroundAt(sView.tx, sView.tz, sView.havePlayerH ? sView.playerH : 0.0f);
    sView.playerH = sView.ty;
    sView.havePlayerH = TRUE;
    sView.eye[0] = sView.tx;
    sView.eye[1] = sView.ty + CAM_DIST * sp;
    sView.eye[2] = sView.tz + CAM_DIST * cp;
    sView.fwd[0] = 0.0f, sView.fwd[1] = -sp, sView.fwd[2] = -cp;
    sView.right[0] = 1.0f, sView.right[1] = 0.0f, sView.right[2] = 0.0f;
    sView.up[0] = 0.0f, sView.up[1] = cp, sView.up[2] = -sp;
    sView.focal = 96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f);
    ShotView();
}

// World -> screen x, y and depth (FALSE: behind the camera)
static bool8 Project(float x, float y, float z, float *sx, float *sy, float *depth)
{
    float dx = x - sView.eye[0], dy = y - sView.eye[1], dz = z - sView.eye[2];
    float cx = dx * sView.right[0] + dy * sView.right[1] + dz * sView.right[2];
    float cy = dx * sView.up[0] + dy * sView.up[1] + dz * sView.up[2];
    float cz = dx * sView.fwd[0] + dy * sView.fwd[1] + dz * sView.fwd[2];
    if (cz < 1.0f)
        return FALSE;
    *sx = sView.ax + sView.focal * cx / cz;
    *sy = sView.ay - sView.focal * cy / cz;
    *depth = cz;
    return TRUE;
}

// ------------------------------------------------------------------------------------------------ sprites in the scene
static float sSpriteH[MAX_SPRITES];
static u8 sSpriteHFrame[MAX_SPRITES];

void O3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth)
{
    float sx, sy, d, h, near = sView.ty;
    float wx = sView.camX + x, wz = sView.camY + feetY - 8;
    if (!sView.active)
    {
        *outX = x;
        *outY = feetY;
        *depth = 0.0f;
        return;
    }
    if (spriteId < MAX_SPRITES && (u8)(sView.frame - sSpriteHFrame[spriteId]) <= 2)
        near = sSpriteH[spriteId];
    h = GroundAt(wx, wz, near);
    if (spriteId < MAX_SPRITES)
    {
        sSpriteH[spriteId] = h;
        sSpriteHFrame[spriteId] = sView.frame;
    }
    if (!Project(wx, h, wz, &sx, &sy, &d))
    {
        *outX = x;
        *outY = feetY;
        *depth = 0.0f;
        return;
    }
    *outX = (s32)floorf(sx + 0.5f);
    *outY = (s32)floorf(sy + 0.5f) + 8;
    *depth = d;
}

// A sprite is an upright card standing at its spot: a pixel dy above its feet is dy * tan(pitch) nearer the camera
float gO3dSpriteDepth[MAX_SPRITES];
s16 gO3dSpriteFeet[MAX_SPRITES];
typedef struct { float d0; s16 feet; u8 flat, refl; } OamDepth;   // flat: a shadow on the ground; refl: a reflection
static OamDepth sPendingOam[OAM_ENTRY_COUNT], sLiveOam[OAM_ENTRY_COUNT];
static bool8 sLiveAny;

void O3D_NoteOam(u16 spriteId, u16 first, u16 end)
{
    u16 i;
    bool8 on = sView.active && spriteId < MAX_SPRITES && gSprites[spriteId].coordOffsetEnabled;
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
    {
        sPendingOam[i].d0 = on ? gO3dSpriteDepth[spriteId] : 0.0f;
        sPendingOam[i].feet = on ? gO3dSpriteFeet[spriteId] : 0;
        sPendingOam[i].flat = on && gSprites[spriteId].mpShadow;
        sPendingOam[i].refl = on && FieldEffect_IsReflectionSprite(&gSprites[spriteId]);
    }
}

void O3D_CommitOam(void)
{
    memcpy(sLiveOam, sPendingOam, sizeof(sLiveOam));
    sLiveAny = sView.active;
    if (sView.active)
        memset(sPendingOam, 0, sizeof(sPendingOam));
}

// A reflection pixel shows only where the ground under it reflects (the ray from the eye meets the player's level
// on a tile whose behavior reflects)
static bool8 OverReflectiveTile(int x, int line)
{
    float rx = (x - sView.ax) / sView.focal, ry = (sView.ay - line) / sView.focal;
    float d[3], t, wx, wz;
    u32 i, b;
    for (i = 0; i < 3; i++)
        d[i] = sView.fwd[i] + sView.right[i] * rx + sView.up[i] * ry;
    if (d[1] > -1e-4f)
        return TRUE;
    t = (sView.ty - sView.eye[1]) / d[1];
    wx = sView.eye[0] + d[0] * t;
    wz = sView.eye[2] + d[2] * t;
    b = MapGridGetMetatileBehaviorAt((s32)floorf(wx / 16.0f) + MAP_OFFSET, (s32)floorf(wz / 16.0f) + MAP_OFFSET);
    return MetatileBehavior_IsReflective(b) || MetatileBehavior_IsIce(b);
}

static bool OamHidden(int oamIndex, int x, int line)
{
    const OamDepth *o;
    float d;
    if (!sLiveAny || oamIndex < 0 || oamIndex >= OAM_ENTRY_COUNT || x < 0 || x >= W || line < 0 || line >= H)
        return FALSE;
    o = &sLiveOam[oamIndex];
    if (o->d0 <= 0.0f)
        return FALSE;
    if (o->refl && !OverReflectiveTile(x, line))
        return TRUE;
    if (o->flat)   // (a shadow lies on the ground: only a wall in front hides it)
        return sDepth[line][x] < o->d0 - 16.0f;
    d = o->d0 - (o->feet - line) * 1.6652f;   // tan(59.05 degrees)
    return sDepth[line][x] < d - 6.0f;
}

// ------------------------------------------------------------------------------------------------ rasterizer
typedef struct { float x, y, iz, u, v, c[3]; } SVtx;   // u, v and c over z (perspective)

static s32 Wrap(s32 i, s32 size, u8 repeat, u8 flip)
{
    if (!repeat)
        return i < 0 ? 0 : i >= size ? size - 1 : i;
    if (flip)
    {
        s32 m = ((i % (2 * size)) + 2 * size) % (2 * size);
        return m >= size ? 2 * size - 1 - m : m;
    }
    return ((i % size) + size) % size;
}

// Both faces (Colosseum's walls were made to be seen from its own cameras); textures modulated by the colours
static void DrawTriangle(const SVtx *a, const SVtx *b, const SVtx *c, const OMesh *g)
{
    float area = (b->x - a->x) * (c->y - a->y) - (c->x - a->x) * (b->y - a->y);
    s32 minx, maxx, miny, maxy, x, y;
    const OTex *t = g->tex >= 0 ? &sPack.tex[g->tex] : NULL;
    bool8 clear = g->alpha < 31;
    if (area == 0.0f)
        return;
    if (t && t->w == 0)
        t = NULL;
    minx = (s32)floorf(fminf(a->x, fminf(b->x, c->x)));
    maxx = (s32)ceilf(fmaxf(a->x, fmaxf(b->x, c->x)));
    miny = (s32)floorf(fminf(a->y, fminf(b->y, c->y)));
    maxy = (s32)ceilf(fmaxf(a->y, fmaxf(b->y, c->y)));
    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx > W - 1) maxx = W - 1;
    if (maxy > H - 1) maxy = H - 1;
    for (y = miny; y <= maxy; y++)
    {
        float py = y + 0.5f;
        for (x = minx; x <= maxx; x++)
        {
            float px = x + 0.5f;
            float w0 = ((b->x - px) * (c->y - py) - (c->x - px) * (b->y - py)) / area;
            float w1 = ((c->x - px) * (a->y - py) - (a->x - px) * (c->y - py)) / area;
            float w2 = 1.0f - w0 - w1, iz, z, col[3], out[3], al = 1.0f;
            u32 ch;
            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
                continue;
            iz = w0 * a->iz + w1 * b->iz + w2 * c->iz;
            z = 1.0f / iz;
            if (z >= sDepth[y][x] || (clear && sIds[y][x] == g->pid))
                continue;
            for (ch = 0; ch < 3; ch++)
                col[ch] = (w0 * a->c[ch] + w1 * b->c[ch] + w2 * c->c[ch]) * z;
            if (t)
            {
                s32 tu = Wrap((s32)floorf((w0 * a->u + w1 * b->u + w2 * c->u) * z), t->w, g->rs, g->fs);
                s32 tv = Wrap((s32)floorf((w0 * a->v + w1 * b->v + w2 * c->v) * z), t->h, g->rt, g->ft);
                u32 texel = t->px[tv * t->w + tu];
                u32 ta = texel >> 24;
                if (ta == 0)
                    continue;
                for (ch = 0; ch < 3; ch++)
                {
                    float tc = ((texel >> (8 * ch)) & 0xFF) * 31.0f / 255.0f;
                    out[ch] = (tc + 1.0f) * (col[ch] + 1.0f) / 32.0f - 1.0f;
                }
                al = ta / 255.0f;
            }
            else
            {
                for (ch = 0; ch < 3; ch++)
                    out[ch] = col[ch];
            }
            al *= g->alpha / 31.0f;
            {
                u16 old = sColor[y][x];
                u16 pxl = 0;
                for (ch = 0; ch < 3; ch++)
                {
                    float o = (old >> (5 * ch)) & 31;
                    float v = out[ch] * al + o * (1.0f - al);
                    s32 iv = (s32)(v + 0.5f);
                    pxl |= (iv < 0 ? 0 : iv > 31 ? 31 : iv) << (5 * ch);
                }
                sColor[y][x] = pxl;
            }
            if (clear)
                sIds[y][x] = g->pid;
            else
            {
                sDepth[y][x] = z;
                sIds[y][x] = -1;
            }
        }
    }
}

static SVtx *sScratch;
static u8 *sScratchOk;
static u32 sScratchSize;

// pass 0 draws the opaque meshes, 1 the see-through ones
static void DrawMesh(const OMesh *g, int pass)
{
    u32 i;
    float sx, sy, d;
    if ((g->alpha < 31) != pass)
        return;
    if (Project(g->c[0], g->c[1], g->c[2], &sx, &sy, &d))
    {
        float rad = g->r * sView.focal / d * 1.5f + 2.0f;
        if (sx + rad < 0 || sx - rad > W || sy + rad < 0 || sy - rad > H)
            return;
    }
    if (g->count > sScratchSize)
    {
        sScratchSize = g->count + 1024;
        sScratch = realloc(sScratch, sizeof(SVtx) * sScratchSize);
        sScratchOk = realloc(sScratchOk, sScratchSize);
    }
    for (i = 0; i < g->count; i++)
    {
        const OVtx *v = &g->v[i];
        SVtx *s = &sScratch[i];
        float depth;
        sScratchOk[i] = Project(v->x, v->y, v->z, &s->x, &s->y, &depth);
        if (!sScratchOk[i])
            continue;
        s->iz = 1.0f / depth;
        s->u = v->u * s->iz;
        s->v = v->v * s->iz;
        s->c[0] = v->r * s->iz, s->c[1] = v->g * s->iz, s->c[2] = v->b * s->iz;
    }
    for (i = 0; i + 2 < g->count; i += 3)
        if (sScratchOk[i] && sScratchOk[i + 1] && sScratchOk[i + 2])
            DrawTriangle(&sScratch[i], &sScratch[i + 1], &sScratch[i + 2], g);
}

static void Render(void)
{
    u32 i, pass;
    for (i = 0; i < W * H; i++)
    {
        (&sColor[0][0])[i] = 0;
        (&sDepth[0][0])[i] = 1e30f;
        (&sIds[0][0])[i] = -1;
    }
    if (sCur == NULL)
        return;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < sCur->nmesh; i++)
            DrawMesh(&sCur->mesh[i], pass);
}

// ------------------------------------------------------------------------------------------------ the picture
// The palette's fade (a screen fading to black or white, a battle's opening) applied to the 3D picture too
static void FindFade(void)
{
    const u16 *shown = (const u16 *)PLTT;
    s32 ch, i;
    for (ch = 0; ch < 3; ch++)
    {
        s32 lo = 32, hi = -1, loShown = 0, hiShown = 0;
        for (i = 0; i < 13 * 16; i++)
        {
            s32 u = (gPlttBufferUnfaded[i] >> (5 * ch)) & 31, s = (shown[i] >> (5 * ch)) & 31;
            if (u < lo)
                lo = u, loShown = s;
            if (u > hi)
                hi = u, hiShown = s;
        }
        if (hi > lo)
        {
            sFadeB[ch] = (hiShown - loShown) * 256 / (hi - lo);
            sFadeA[ch] = loShown * 256 - sFadeB[ch] * lo;
        }
        else
        {
            sFadeB[ch] = hi > 0 ? hiShown * 256 / hi : 256;
            sFadeA[ch] = 0;
        }
    }
}

static inline u16 Faded(u16 c)
{
    s32 ch, out = 0x8000;
    for (ch = 0; ch < 3; ch++)
    {
        s32 v = (sFadeA[ch] + sFadeB[ch] * ((c >> (5 * ch)) & 31) + 128) >> 8;
        out |= (v < 0 ? 0 : v > 31 ? 31 : v) << (5 * ch);
    }
    return out;
}

// The field, also while a battle's opening plays
static bool8 OnField(void)
{
    return gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic;
}

// The BUY screen of a mart on an ORRE map (shop.c): the field's last 3D picture, moved by DX DY as Emerald moves its map
// so the player stands where the screen puts them (SINNOH's engine does the same for its own maps)
static struct { bool8 on; s16 dx, dy; } sShopPic;

static void ShopLine(int line, u16 layers[4][DISPLAY_WIDTH])
{
    s32 x, sy = line - sShopPic.dy;
    if (line == 0)
        FindFade();
    for (x = 0; x < W; x++)
    {
        s32 sx = x - sShopPic.dx;
        layers[3][x] = (sy >= 0 && sy < H && sx >= 0 && sx < W) ? Faded(sColor[sy][sx]) : Faded(0);
        layers[2][x] = 0;
    }
}

static void O3dLine(int line, u16 layers[4][DISPLAY_WIDTH])
{
    s32 x;
    if (sShopPic.on && !OnField())
    {
        ShopLine(line, layers);
        return;
    }
    if (!sView.active || !OnField())
        return;
    if (line == 0)
    {
        // MOEMON_O3D_PERF: the time a frame's 3D picture takes, averaged over 120 frames
        static int sPerf = -1, sCount;
        static unsigned long long sSum, sMax;
        unsigned long long t0 = 0;
        if (sPerf < 0)
            sPerf = getenv("MOEMON_O3D_PERF") != NULL;
        if (sPerf)
            t0 = SDL_GetPerformanceCounter();
        FindFade();
        Render();
        if (sPerf)
        {
            unsigned long long dt = SDL_GetPerformanceCounter() - t0;
            sSum += dt;
            sMax = dt > sMax ? dt : sMax;
            if (++sCount == 120)
            {
                double f = (double)SDL_GetPerformanceFrequency() / 1000.0;
                printf("[o3d] perf: %.2f ms a frame (worst %.2f)\n", sSum / f / sCount, sMax / f);
                sCount = 0;
                sSum = sMax = 0;
            }
        }
    }
    for (x = 0; x < W; x++)
    {
        layers[3][x] = Faded(sColor[line][x]);
        layers[2][x] = 0;
        layers[1][x] = 0;
    }
}

// ------------------------------------------------------------------------------------------------ every frame
float O3D_TestGround(s32 mapX, s32 mapY, float near)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0 || !MapLoad((u32)map))
        return NAN;
    sCur = &sPack.map[map];
    return GroundAt((float)mapX, (float)mapY, near);
}

bool8 O3D_Active(void)
{
    return sView.active && OnField();
}

void O3D_ShopPicture(bool8 on, s16 dx, s16 dy)
{
    sShopPic.on = on && sView.active;
    sShopPic.dx = dx, sShopPic.dy = dy;
    if (sShopPic.on)
        gMpO3dLine = O3dLine;
}

void O3D_Update(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    sView.active = map >= 0 && MapLoad((u32)map);
    sShopPic.on = FALSE;   // (back on the field)
    gMpO3dLine = sView.active ? O3dLine : NULL;
    gMpO3dOamHidden = sView.active && !getenv("MOEMON_O3D_NOHIDE") ? OamHidden : NULL;
    Orre_Vm_Frame();   // (Colosseum's scripts: src/orre/orre_vm.c; they stop off ORRE)
    O3D_CamTick();     // (a shot of Colosseum's playing: o3d_cam.c)
    if (!sView.active)
    {
        sCur = NULL;
        if (O3D_CamActive())
            O3D_CamStop();   // (off ORRE: no shot)
        return;
    }
    if (sView.map != map)
    {
        sView.havePlayerH = FALSE;   // (a new map: its own ground)
        if (O3D_CamActive() && !O3D_CamKeep())
            O3D_CamStop();   // (a shot belongs to its room)
    }
    sView.map = map;
    sCur = &sPack.map[map];
    sView.frame++;
    FindCamera();
    SetupView();
}

// Colosseum's units on the current ORRE map (its scripts' positions: src/orre/orre_vm.c): a tile's centre, and the tile
// a point is on. FALSE: not on a loaded ORRE map
bool8 O3D_TileToColo(s16 x, s16 y, float *cx, float *cz)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    const OMap *om;
    if (map < 0 || !MapLoad(map))
        return FALSE;
    om = &sPack.map[map];
    *cx = om->x0 + (x + 0.5f) * om->tile;
    *cz = om->z0 + (y + 0.5f) * om->tile;
    return TRUE;
}

bool8 O3D_ColoToTile(float cx, float cz, s16 *x, s16 *y)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    const OMap *om;
    if (map < 0 || !MapLoad(map))
        return FALSE;
    om = &sPack.map[map];
    *x = (s16)floorf((cx - om->x0) / om->tile);
    *y = (s16)floorf((cz - om->z0) / om->tile);
    return TRUE;
}

// The current ORRE map's file ("S1_out") and Colosseum room (sOrreMaps), NULL / 0 off ORRE
const char *O3D_RoomFile(u16 *room)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return NULL;
    if (room)
        *room = sOrreMaps[map].room;
    return sOrreMaps[map].file;
}
