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
#include "orre_data.h"
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
#define FEET_Z 6.0f   // where a person's card stands, from its tile's middle toward the field's camera (Platinum's)
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
    u8 sky;        // the map's sky: drawn only while a shot of Colosseum's plays (the field camera looks down)
    u8 solid;      // a prop's model (DrawProps): texels drawn whole or not at all, and outlined (sPropMask)
    u32 count;     // vertices (3 a triangle)
    OVtx *v;
    float c[3], r; // the mesh's centre and radius (skipped off screen)
    // What moves (tools/orre/build_pack.py set_anims, played in a loop at 60 a second): 1 the texture slides
    // (anim: du dv a sample, in its coordinates; su sv: those to texels), 2 the mesh moves with its joint (anim: a 3x4
    // matrix a sample, Colosseum's units; v0 the mesh at rest)
    u8 animKind, animDoor;   // (kind 3: a door's leaf, animDoor its number in Colosseum's door table)
    u32 animPeriod, animCount;
    float *anim;
    OVtx *v0;
    float c0[3], su, sv, du, dv;
} OMesh;

// Colosseum's doors (src/orre/orre_vm.c CtrlDoor), by their number in its door table: open or shut, how far along
// the opening animation (frames), and that animation's length
static u8 sDoorOpen[256];
static u16 sDoorPos[256], sDoorLen[256];

// floorEventCtrlDoor's modes: 0 it opens, 1 it's open at once, 2 it closes, 3 it's shut at once
void O3D_DoorMode(u8 door, u8 mode)
{
    sDoorOpen[door] = mode < 2;
    if (mode == 1)
        sDoorPos[door] = sDoorLen[door] ? sDoorLen[door] : 0xFFFF;
    else if (mode == 3)
        sDoorPos[door] = 0;
    else if (sDoorPos[door] > sDoorLen[door])
        sDoorPos[door] = sDoorLen[door];
}

// r | g << 8 | b << 16 | a << 24. mip[0] is px; each level after it half the one before (alpha-weighted), down to 2
// texels: a surface far off or seen at a slant is drawn from the level whose texels are about a pixel wide, so it no
// longer speckles, and one nearer than a texel a pixel is smoothed between its four nearest texels (Colosseum's own
// look: the GameCube filters its textures; Platinum's DS picks single texels. The user, 2026-11-11)
#define MIP_MAX 8
typedef struct { u16 w, h; u32 *px; u32 *mip[MIP_MAX]; u8 nmip; } OTex;

typedef struct
{
    bool8 tried, ok;
    u16 w, h;                 // tiles
    u16 *blocks;              // the field engine's map blocks (metatile | collision << 10 | elevation << 12)
    u8 *hn;                   // the ground's heights at each tile's centre: how many (more than one: a bridge)
    u8 *edge;                 // each tile's sides a step can't cross (1 N, 2 E, 4 S, 8 W: tools/orre/grid.py EDGE_*)
    s8 *trig;                 // each tile's trigger: Colosseum's collision id (-1 none: a door's, a way out's...)
    float (*hgt)[3];          // ... lowest first
    u32 nmesh;
    OMesh *mesh;
    float tile, x0, z0;       // Colosseum's units: a tile's size, the grid's corner (O3D_TileToColo)
} OMap;

#define TV_SIZE 64   // a TV's picture (tools/orre/tv.py SIZE)
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
// A texture at half its size, each texel its four's average weighed by their alpha
static u32 *HalfTexture(const u32 *px, u32 w, u32 h)
{
    u32 nw = w / 2, nh = h / 2, x, y, k;
    u32 *half = malloc(4 * nw * nh);
    if (half == NULL)
        return NULL;
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
    return half;
}

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
        u32 *half = HalfTexture(px, w, h);
        free(px);
        px = half, w /= 2, h /= 2;
    }
    sPack.tex[t].w = (u16)w, sPack.tex[t].h = (u16)h, sPack.tex[t].px = px;
    {
        OTex *o = &sPack.tex[t];
        o->mip[0] = px, o->nmip = 1;
        while (o->nmip < MIP_MAX && (w >> o->nmip) >= 2 && (h >> o->nmip) >= 2)
        {
            u32 *half = HalfTexture(o->mip[o->nmip - 1], w >> (o->nmip - 1), h >> (o->nmip - 1));
            if (half == NULL)
                break;
            o->mip[o->nmip++] = half;
        }
    }
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
// A room's model from the pack: map I's (its grid too: the field's blocks), or, I < 0, a scene room's (a room only shots
// look at: the opening's; its grid if it has one, else Colosseum's units as they are at the tile scale)
static bool8 RoomLoad(OMap *om, const char *file, s32 i)
{
    u32 m, set, ns, nm, mi, has, x, y, fileMesh = 0;
    s32 pm = -1, *fileToMesh = NULL;
    Reader afterHas, anims;
    unsigned long rawSize;
    u8 *z, *b;
    Reader r, meshes;
    u16 gw, gh;
    float tile, x0, z0, S;
    const u8 *cells;
    if (om->tried)
        return om->ok;
    om->tried = TRUE;
    if (!PackOpen())
        return FALSE;
    for (m = 0; m < sPack.nmaps; m++)
        if (strcmp(sPack.maps[m].name, file) == 0)
            pm = (s32)m;
    if (pm < 0 || (z = PackRead(sPack.maps[pm].off, sPack.maps[pm].zsize)) == NULL)
    {
        printf("[o3d] ORRE's pack has no %s\n", file);
        return FALSE;
    }
    rawSize = sPack.maps[pm].size;
    b = malloc(rawSize ? rawSize : 1);
    if (b == NULL || uncompress(b, &rawSize, z, sPack.maps[pm].zsize) != 0 || rawSize != sPack.maps[pm].size)
    {
        free(z);
        free(b);
        printf("[o3d] ORRE's %s won't inflate\n", file);
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
    afterHas = r;
    Take(&r, &gw, 2);
    Take(&r, &gh, 2);
    Take(&r, &tile, 4);
    Take(&r, &x0, 4);
    Take(&r, &z0, 4);
    cells = has == 1 ? Skip(&r, (u32)gw * gh * 16) : NULL;
    anims = has == 1 ? r : afterHas;   // (what moves: after the grid, or after a scene room's "no grid")
    if (i < 0 && (r.bad || has != 1 || tile <= 0.0f))
    {
        has = 0, gw = gh = 0, tile = 8.0f, x0 = z0 = 0.0f;   // (a scene room without a grid)
        r.bad = FALSE;
    }
    if (r.bad || (i >= 0 && (has != 1 || tile <= 0.0f)))
    {
        free(b);
        printf("[o3d] ORRE's %s has no grid\n", file);
        return FALSE;
    }
    if (i >= 0 && (gw != sOrreMaps[i].bw * 32 || gh != sOrreMaps[i].bh * 32))
    {
        free(b);
        printf("[o3d] ORRE's %s isn't the size of its map (tools/orre/gen_maps.py)\n", file);
        return FALSE;
    }
    S = 16.0f / tile;
    om->w = gw, om->h = gh;
    om->tile = tile, om->x0 = x0, om->z0 = z0;
    om->blocks = malloc(sizeof(u16) * ((u32)gw * gh + 1));
    om->hn = calloc((u32)gw * gh, 1);
    om->edge = calloc((u32)gw * gh, 1);
    om->hgt = calloc((u32)gw * gh, sizeof(*om->hgt));
    om->trig = malloc((u32)gw * gh + 1);
    if (om->trig)
        memset(om->trig, -1, (u32)gw * gh + 1);
    for (y = 0; y < gh && cells; y++)
        for (x = 0; x < gw; x++)
        {
            const u8 *t = cells + ((u32)y * gw + x) * 16;
            u32 k = (u32)y * gw + x, n = t[2] > 3 ? 3 : t[2], j;
            om->blocks[k] = (t[0] & 1) ? (WALK + t[3]) : BLOCKED;   // (t[3]: the tile's behavior: a warp's)
            om->hn[k] = (u8)n;
            if (om->edge)
                om->edge[k] = t[0] >> 4;
            if (om->trig)
                om->trig[k] = (s8)t[1];
            for (j = 0; j < n; j++)
            {
                float hh;
                memcpy(&hh, t + 4 + 4 * j, 4);
                om->hgt[k][j] = hh * S;
            }
        }
    // the meshes (the sky's marked: drawn only under a shot)
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
            u32 thisFile = fileMesh++;
            fileToMesh = realloc(fileToMesh, sizeof(s32) * fileMesh);
            fileToMesh[thisFile] = -1;
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
            if (meshes.bad || ni < 3)
                continue;
            om->mesh = realloc(om->mesh, sizeof(OMesh) * (om->nmesh + 1));
            fileToMesh[thisFile] = (s32)om->nmesh;
            me = &om->mesh[om->nmesh++];
            memset(me, 0, sizeof(*me));
            me->sky = kind[0] == 2;
            // (-(2 + set): a TV's screen, no texture of its own: its set kept for the picture, O3D_TvSet)
            me->tex = tex >= 0 ? Texture(tex) : tex <= -2 && tex > -0x1000 ? tex : -1;
            me->su = me->tex >= 0 ? us * sPack.tex[me->tex].w : me->tex <= -2 ? us * TV_SIZE : 0.0f;
            me->sv = me->tex >= 0 ? vs * sPack.tex[me->tex].h : me->tex <= -2 ? vs * TV_SIZE : 0.0f;
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
                d->u = me->tex >= 0 ? p[3] * us * sPack.tex[me->tex].w : me->tex <= -2 ? p[3] * us * TV_SIZE : 0.0f;
                d->v = me->tex >= 0 ? p[4] * vs * sPack.tex[me->tex].h : me->tex <= -2 ? p[4] * vs * TV_SIZE : 0.0f;
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
    // what moves (a pack from before has none: the map stands still)
    if (!anims.bad && anims.left >= 4 && !meshes.bad)
    {
        u32 n, k, moving = 0;
        Take(&anims, &n, 4);
        for (k = 0; k < n && !anims.bad; k++)
        {
            u32 mesh, period, count, per;
            u8 kind[4];
            const u8 *data;
            Take(&anims, &mesh, 4);
            Take(&anims, kind, 4);
            Take(&anims, &period, 4);
            Take(&anims, &count, 4);
            per = kind[0] == 1 ? 2 : 12;
            data = Skip(&anims, count * per * 4);
            if (data && mesh < fileMesh && fileToMesh[mesh] >= 0 && kind[0] >= 1 && kind[0] <= 3 && count && period)
            {
                OMesh *me = &om->mesh[fileToMesh[mesh]];
                me->anim = malloc(count * per * 4);
                memcpy(me->anim, data, count * per * 4);
                me->animKind = kind[0], me->animDoor = kind[1], me->animPeriod = period, me->animCount = count;
                if (kind[0] == 3 && period > sDoorLen[kind[1]])
                    sDoorLen[kind[1]] = period > 0xFFFF ? 0xFFFF : period;
                if (kind[0] >= 2)
                {
                    me->v0 = malloc(sizeof(OVtx) * me->count);
                    memcpy(me->v0, me->v, sizeof(OVtx) * me->count);
                    memcpy(me->c0, me->c, sizeof(me->c0));
                    me->r *= 1.5f;   // (room to move: culled by its rest centre's moved place)
                }
                moving++;
            }
        }
        if (moving)
            printf("[o3d] ORRE's %s: %u meshes move\n", file, moving);
    }
    free(fileToMesh);
    free(b);
    if (meshes.bad)
    {
        printf("[o3d] ORRE's %s is cut short\n", file);
        return FALSE;
    }
    om->ok = TRUE;
    printf("[o3d] ORRE's %s: %u meshes\n", file, om->nmesh);
    return TRUE;
}

static bool8 MapLoad(u32 i)
{
    return i < ARRAY_COUNT(sOrreMaps) && RoomLoad(&sPack.map[i], sOrreMaps[i].file, (s32)i);
}

// ---- a scene room: drawn instead of the field's map while a shot plays (the opening's rooms aren't maps)
static const OMap *sCur;   // the map the player is on
static OMap sScene;
static char sSceneFile[32];
static bool8 sSceneOn;

static void SceneFree(void)
{
    u32 k;
    for (k = 0; k < sScene.nmesh; k++)
    {
        free(sScene.mesh[k].v);
        free(sScene.mesh[k].v0);
        free(sScene.mesh[k].anim);
    }
    free(sScene.mesh);
    free(sScene.blocks);
    free(sScene.hn);
    free(sScene.edge);
    free(sScene.hgt);
    free(sScene.trig);
    memset(&sScene, 0, sizeof(sScene));
}

// The room FILE (its pack name) drawn under the shots from now on; NULL: the field's map again
bool8 O3D_SceneRoom(const char *file)
{
    if (file == NULL)
    {
        sSceneOn = FALSE;
        return TRUE;
    }
    if (strcmp(file, sSceneFile) != 0)
    {
        SceneFree();
        snprintf(sSceneFile, sizeof(sSceneFile), "%s", file);
    }
    sSceneOn = RoomLoad(&sScene, file, -1);
    return sSceneOn;
}

// The room the picture shows: a scene room while one is set, else the field's map
static const OMap *Drawn(void)
{
    return sSceneOn && sScene.ok ? &sScene : sCur;
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

// Colosseum's room ROOM, entry ENTRY (floorLink's): its map and the tile it lands on (sOrreArrivals lists each map's
// entries in Colosseum's order); FALSE: the room isn't a map
bool8 O3D_RoomEntry(u16 room, u32 entry, u16 *map, s16 *x, s16 *y)
{
    u32 i, k = 0, first = ARRAY_COUNT(sOrreArrivals);
    *map = 0xFFFF;
    for (i = 0; i < ARRAY_COUNT(sOrreMaps); i++)
        if (sOrreMaps[i].room == room)
            *map = sOrreMaps[i].map;
    if (*map == 0xFFFF)
        return FALSE;
    for (i = 0; i < ARRAY_COUNT(sOrreArrivals); i++)
        if (sOrreArrivals[i].map == *map)
        {
            if (first == ARRAY_COUNT(sOrreArrivals))
                first = i;
            if (k++ == entry)
            {
                *x = sOrreArrivals[i].x, *y = sOrreArrivals[i].y;
                return TRUE;
            }
        }
    if (first == ARRAY_COUNT(sOrreArrivals))
        return FALSE;
    *x = sOrreArrivals[first].x, *y = sOrreArrivals[first].y;   // (no such entry: the first)
    return TRUE;
}

// The entry of the current map the player stands on (floorGetNextPosIndex after a door); 0 when none
u32 O3D_EntryAt(s16 x, s16 y)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i, k = 0;
    for (i = 0; i < ARRAY_COUNT(sOrreArrivals); i++)
        if (sOrreArrivals[i].map == map)
        {
            if (sOrreArrivals[i].x == x && sOrreArrivals[i].y == y)
                return k;
            k++;
        }
    return 0;
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
static u8 sPropMask[H][W];   // the props' pixels this frame: their soft outline (OutlineProps)
static s32 sFadeA[3], sFadeB[3];

static float sFieldEye[3], sFieldFwd[3];   // the field camera this frame (O3D_FieldView)

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
#define SHOT_LINES 114   // (160 less the message box and its frame)
static void ShotView(void)
{
    float eye[3], tgt[3], roll, fov, S, f[3], r[3], u[3], len, cr, sr;
    u32 i;
    const OMap *g = Drawn();
    if (!g || !O3D_CamGet(eye, tgt, &roll, &fov))
        return;
    S = 16.0f / g->tile;
    eye[0] = (eye[0] - g->x0) * S, eye[1] *= S, eye[2] = (eye[2] - g->z0) * S;
    tgt[0] = (tgt[0] - g->x0) * S, tgt[1] *= S, tgt[2] = (tgt[2] - g->z0) * S;
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
    // (the shot's picture is the screen above Kai's message box, SHOT_LINES of the 160: Colosseum frames its people
    // over the whole picture and speaks in bubbles; under Kai's box their bodies, and often all of them, were out of
    // sight: the user, 2026-11-10. What the shot sees below its own picture lies under the box)
    sView.ax = DISPLAY_WIDTH / 2, sView.ay = SHOT_LINES / 2;
    sView.focal = (SHOT_LINES / 2) / tanf(fov * 0.5f * 3.14159265f / 180.0f);
}

// A player just arrived on a map: the ground of the entry they land on, where its tile has more than one (PHENAC's
// PRE GYM door is over the level under the plaza: the lower one hid the player in it, the user 2026-11-08); 0 else
static float ArrivalHeight(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    u16 id = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    s16 x = gSaveBlock1Ptr->pos.x, y = gSaveBlock1Ptr->pos.y;
    u32 i;
    (void)map;
    for (i = 0; i < ARRAY_COUNT(sOrreArrivals); i++)
        if (sOrreArrivals[i].map == id && sOrreArrivals[i].x == x && sOrreArrivals[i].y == y)
            return sOrreArrivals[i].h;
    return 0.0f;
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
    sView.ty = GroundAt(sView.tx, sView.tz, sView.havePlayerH ? sView.playerH : ArrivalHeight());
    sView.playerH = sView.ty;
    sView.havePlayerH = TRUE;
    sView.eye[0] = sView.tx;
    sView.eye[1] = sView.ty + CAM_DIST * sp;
    sView.eye[2] = sView.tz + CAM_DIST * cp;
    sView.fwd[0] = 0.0f, sView.fwd[1] = -sp, sView.fwd[2] = -cp;
    sView.right[0] = 1.0f, sView.right[1] = 0.0f, sView.right[2] = 0.0f;
    sView.up[0] = 0.0f, sView.up[1] = cp, sView.up[2] = -sp;
    sView.focal = 96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f);
    memcpy(sFieldEye, sView.eye, sizeof(sFieldEye));
    memcpy(sFieldFwd, sView.fwd, sizeof(sFieldFwd));
    ShotView();
}

// The field camera (as SetupView made it this frame, before a shot's) in Colosseum's units: the scene's own camera
// starts from it and goes back to it (o3d_cam.c sFree)
void O3D_FieldView(float eye[3], float rot[3])
{
    const OMap *g = Drawn();
    float S = g ? 16.0f / g->tile : 1.0f;
    eye[0] = sFieldEye[0] / S + (g ? g->x0 : 0), eye[1] = sFieldEye[1] / S, eye[2] = sFieldEye[2] / S + (g ? g->z0 : 0);
    rot[0] = asinf(sFieldFwd[1]);
    rot[1] = atan2f(-sFieldFwd[0], -sFieldFwd[2]);
    rot[2] = 0.0f;
}

float O3D_FieldFov(void)
{
    // (SetupView's focal: 96 pixels at HALF_FOV, over the 80 lines above the centre)
    return 2.0f * atanf(80.0f * tanf(HALF_FOV * 3.14159265f / 180.0f) / 96.0f) * 180.0f / 3.14159265f;
}

// A step from one tile to the one beside it that a wall of Colosseum's stands across (map tiles, the field's own:
// with MAP_OFFSET). Colosseum's player walks at any angle wherever a person fits, so a tile is walkable for the floor
// reached inside it and its walls are its sides' (tools/orre/grid.py _tiles_from_flood)
bool8 O3D_StepBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY)
{
    const OMap *g;
    s32 x = fromX - MAP_OFFSET, y = fromY - MAP_OFFSET, dx = toX - fromX, dy = toY - fromY;
    u8 bit;
    if (gSaveBlock1Ptr == NULL || !O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup))
        return FALSE;
    g = sCur;
    if (g == NULL || g->edge == NULL || x < 0 || y < 0 || x >= g->w || y >= g->h)
        return FALSE;
    bit = dy < 0 ? 1 : dx > 0 ? 2 : dy > 0 ? 4 : dx < 0 ? 8 : 0;
    return (g->edge[(u32)y * g->w + x] & bit) != 0;
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

// While a shot plays, a field sprite stands in the scene as a card where its spot on the ground is, as big as the shot
// sees it there (Q8, 0: as drawn: the field's own camera); one behind the shot's camera isn't drawn
static u16 sSpriteScale[MAX_SPRITES];

static u8 sSpriteScaleFrame[MAX_SPRITES];

// A sprite out of the picture this frame (behind the camera, past its near edge, or wholly beside the picture): its OAM
// entries are switched off (O3D_NoteOam). Never a far-off position: OAM keeps 9 bits of x and 8 of y, so -327 came
// back as 185 and people behind a shot's camera stood on the diner's TV (the user, 2026-11-08)
static u8 sSpriteOffFrame[MAX_SPRITES];
static bool8 sSpriteOff[MAX_SPRITES];

static s16 sSpriteLift[MAX_SPRITES];
static u8 sSpriteLiftFrame[MAX_SPRITES];

void O3D_LiftSprite(u16 spriteId, s32 lift)
{
    if (spriteId < MAX_SPRITES)
        sSpriteLift[spriteId] = (s16)lift, sSpriteLiftFrame[spriteId] = sView.frame;
}

static void SpriteOff(u16 spriteId, bool8 off)
{
    if (spriteId < MAX_SPRITES)
        sSpriteOff[spriteId] = off, sSpriteOffFrame[spriteId] = sView.frame;
}

// (only a sprite O3D_PlaceSprite placed this frame on ORRE's field: never a battle's, whose sprites reuse the slots)
// While a scene's camera looks from another side than the field's (which looks north), a person's sprite shows the
// side of them that camera sees: facing east under a camera looking south is facing its left (the field's west). Their
// facing itself stays as the scripts set it; only the picture turns, by quarter turns (the sprites' four sides), and
// turns back when the field's camera returns. The player and ORRE's people (Kai's standard picture sets: face, walk,
// run in fours, south / north / west / east)
static bool8 sTurned[OBJECT_EVENTS_COUNT];
bool8 MP_SpriteHasFourSides(const struct Sprite *sprite, bool8 *runs);   // event_object_movement.c

static void SceneFacing(void)
{
    static const u8 sYawOfDir[] = {[DIR_NORTH] = 0, [DIR_WEST] = 1, [DIR_SOUTH] = 2, [DIR_EAST] = 3};
    static const u8 sAnimOfYaw[4] = {1, 2, 0, 3};   // (north, west, south, east as the anims' S N W E order)
    s32 q = 0;
    u32 i;
    if (O3D_CamActive())
    {
        float yaw = atan2f(-sView.fwd[0], -sView.fwd[2]);
        q = ((s32)floorf(yaw / (3.14159265f / 2) + 0.5f) % 4 + 4) % 4;
    }
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *o = &gObjectEvents[i];
        struct Sprite *sp;
        u8 dir, group, target, limit;
        bool8 runs;
        if (!o->active || o->spriteId >= MAX_SPRITES || (!o->isPlayer && o->localId < 0x40))
            continue;
        if (q == 0 && !sTurned[i])
            continue;
        sp = &gSprites[o->spriteId];
        if (!MP_SpriteHasFourSides(sp, &runs))
            continue;
        dir = o->facingDirection;
        limit = runs ? ANIM_STD_COUNT + 4 : ANIM_STD_COUNT;   // (the player's runs too)
        if (dir < DIR_SOUTH || dir > DIR_EAST || sp->animNum >= limit)
            continue;
        group = sp->animNum / 4;
        target = group * 4 + sAnimOfYaw[(sYawOfDir[dir] - q + 4) % 4];
        sTurned[i] = q != 0;
        if (sp->animNum != target)
            StartSpriteAnim(sp, target);
    }
}

u16 O3D_SpriteScale(u16 spriteId)
{
    return O3D_Active() && O3D_CamActive() && spriteId < MAX_SPRITES && sSpriteScaleFrame[spriteId] == sView.frame
         ? sSpriteScale[spriteId] : 0;
}

void O3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth)
{
    float sx, sy, d, h, fy, near = sView.ty;
    float wx = sView.camX + x, wz = sView.camY + feetY - 8;
    if (spriteId < MAX_SPRITES)
        sSpriteScale[spriteId] = 0;
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
    // (a scene's camera: one nearer than a tile and a half is past its near edge, as Colosseum's models get cut off,
    // never a giant card over the picture)
    // (a person's card stands 6 units toward the field's camera from its tile's middle, its feet on the ground there:
    // Platinum's own rule for its people, which ORRE's camera is. The GBA's rule, the feet at the tile's bottom edge
    // 8 lines under its middle, put them 3 lines low (a body out of the middle of a counter) and, under a shot's
    // camera along the ground, 8 lines into it: a small follower to its middle: the user, 2026-11-11)
    if (!Project(wx, h, wz + FEET_Z, &sx, &sy, &d) || (O3D_CamActive() && d < 24.0f))
    {
        if (O3D_CamActive())
        {
            // (behind a shot's camera: not in the picture at all, never left floating where the field had it)
            SpriteOff(spriteId, TRUE);
            *outX = DISPLAY_WIDTH + 64;
            *outY = DISPLAY_HEIGHT + 32;
            *depth = 0.0f;
            return;
        }
        SpriteOff(spriteId, FALSE);
        *outX = x;
        *outY = feetY;
        *depth = 0.0f;
        return;
    }
    fy = sy;
    if (spriteId < MAX_SPRITES)
    {
        // (wholly beside the picture, as big as it's drawn: anchored at its feet, scaled about them; one held to 3x
        // keeps its head where the head is and gives up its feet: a close shot frames a face, and RUI's, from a camera
        // at her knees, sank under the message box with her feet: the user, 2026-11-08)
        const struct Sprite *sp = &gSprites[spriteId];
        float sc = 1.0f, hw = -sp->centerToCornerVecX, ht = -2 * sp->centerToCornerVecY;
        bool8 lifted = (u8)(sView.frame - sSpriteLiftFrame[spriteId]) <= 1;
        if (O3D_CamActive())
        {
            float field = (96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f)) / CAM_DIST, s = (sView.focal / d) / field;
            sc = s < 1.0f / 16 ? 1.0f / 16 : s > 3.0f ? 3.0f : s;
            // (a lifted one keeps to its owner's head: the owner's height, its lift and the 8 its bottom overlaps)
            if (s > 3.0f)
                fy -= (lifted ? sSpriteLift[spriteId] + 8 : ht) * (s - 3.0f);
        }
        if (lifted)
            fy -= sSpriteLift[spriteId] * sc;   // (over its owner's head, as big as the spot is drawn)
        SpriteOff(spriteId, sx + hw * sc < -8.0f || sx - hw * sc > DISPLAY_WIDTH + 8.0f || fy < -8.0f
                         || fy - ht * sc > DISPLAY_HEIGHT + 8.0f);
    }
    if (O3D_CamActive() && spriteId < MAX_SPRITES)
    {
        // (the field's camera draws a sprite 1:1 at its own distance: a shot as much bigger or smaller as it's nearer
        // or farther, against the field camera's focal length)
        float field = (96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f)) / CAM_DIST;
        float s = (sView.focal / d) / field * 256.0f;
        // (at most 3x: the field's 16 x 32 people never fill the picture as a wall of pixels, the user 2026-11-08)
        sSpriteScale[spriteId] = (u16)(s < 16.0f ? 16 : s > 768.0f ? 768 : s);
        sSpriteScaleFrame[spriteId] = sView.frame;
    }
    *outX = (s32)floorf(sx + 0.5f);
    *outY = (s32)floorf(fy + 0.5f);
    *depth = d;
}

// A field sprite's subpriority from how far its spot is from the camera (the view as last drawn: a frame behind at
// most): 20 + 2 a quarter tile, to 60 quarters, then 2 a tile, so the field's own +-1 offsets (a shadow under its
// owner, grass over feet) stay between neighbours; one behind the camera last. The same spot gives the same depth,
// so an effect at its owner's feet keeps its place against them
s32 O3D_DepthSubpriority(const struct Sprite *sprite)
{
    u16 id = sprite - gSprites;
    float sx, sy, d, h, wx, wz;
    s32 q;
    if (!O3D_Active() || !sprite->coordOffsetEnabled || id >= MAX_SPRITES)
        return -1;
    wx = sView.camX + sprite->x + sprite->x2 + gSpriteCoordOffsetX;
    wz = sView.camY + sprite->y + sprite->y2 - sprite->centerToCornerVecY + gSpriteCoordOffsetY - 8;
    h = (u8)(sView.frame - sSpriteHFrame[id]) <= 2 ? sSpriteH[id] : sView.ty;
    if (!Project(wx, h, wz, &sx, &sy, &d))
        return 230;
    q = d <= 0.0f ? 0 : (s32)(d / 4.0f);
    if (q > 60)
        q = 60 + (q - 60) / 4;
    if (q > 104)
        q = 104;
    return 20 + 2 * q;
}

// A sprite is an upright card standing at its spot: a pixel dy above its feet is dy * tan(pitch) nearer the camera
float gO3dSpriteDepth[MAX_SPRITES];
s16 gO3dSpriteFeet[MAX_SPRITES];
// slope: how much nearer the camera the card is a screen line above its feet: the field's camera, 1:1, tan(59.05);
// a scene's camera from its own pitch and the size it draws the sprite at (a level camera: about none, so its upper
// body never counts as in front of a handrail it stands behind: the user, 2026-11-08)
typedef struct { float d0, slope; s16 feet; u8 flat, refl; } OamDepth;   // flat: a shadow on the ground; refl: a reflection
static OamDepth sPendingOam[OAM_ENTRY_COUNT], sLiveOam[OAM_ENTRY_COUNT];
static bool8 sLiveAny;

void O3D_NoteOam(u16 spriteId, u16 first, u16 end)
{
    u16 i;
    bool8 on = sView.active && spriteId < MAX_SPRITES && gSprites[spriteId].coordOffsetEnabled;
    float slope = 1.6652f;   // (the field's camera: tan(59.05 degrees))
    if (on && O3D_CamActive())
    {
        // (a world unit up the card is fwd.y nearer and up.y lines higher at the sprite's own drawn size)
        float sc = O3D_SpriteScale(spriteId) / 256.0f;
        if (sc <= 0.0f)
            sc = 1.0f;
        slope = sView.up[1] > 0.05f ? -sView.fwd[1] / (sView.up[1] * sc) : 0.0f;
    }
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
    {
        sPendingOam[i].d0 = on ? gO3dSpriteDepth[spriteId] : 0.0f;
        sPendingOam[i].slope = on ? slope : 0.0f;
        sPendingOam[i].feet = on ? gO3dSpriteFeet[spriteId] : 0;
        sPendingOam[i].flat = on && gSprites[spriteId].mpShadow;
        sPendingOam[i].refl = on && FieldEffect_IsReflectionSprite(&gSprites[spriteId]);
        if (on && sSpriteOffFrame[spriteId] == (u8)sView.frame && sSpriteOff[spriteId])
            gMain.oamBuffer[i].affineMode = ST_OAM_AFFINE_ERASE;   // (out of the picture: switched off)
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
    if (sSceneOn)
        return TRUE;   // (a scene room under a shot: the field's sprites aren't in it)
    if (!sLiveAny || oamIndex < 0 || oamIndex >= OAM_ENTRY_COUNT || x < 0 || x >= W || line < 0 || line >= H)
        return FALSE;
    o = &sLiveOam[oamIndex];
    if (o->d0 <= 0.0f)
        return FALSE;
    if (o->refl && !OverReflectiveTile(x, line))
        return TRUE;
    if (o->flat)   // (a shadow lies on the ground: only a wall in front hides it)
        return sDepth[line][x] < o->d0 - 16.0f;
    // (lines under its feet line: a follower's picture, a PMD one's or its own, is drawn a few lines lower than the
    // sprite's own bottom edge. The card's slope carried on under the feet put those lines under the ground, and the
    // ground in front of them cut the follower off at the legs (the user, 2026-11-11). They lie at the feet's depth,
    // as a shadow does: only something well in front, a wall or a counter, hides them)
    if (line > o->feet)
        return sDepth[line][x] < o->d0 - 16.0f - (line - o->feet) * 1.5f;
    d = o->d0 - (o->feet - line) * o->slope;
    return sDepth[line][x] < d - 6.0f;
}

// ------------------------------------------------------------------------------------------------ the TVs
// Colosseum's TVs (std171_mapSetTextureChange: the decomp's field_camera_exact_80117AE4.c): a room's channel is a
// programme's model drawn through its own camera onto the screen's texture, every frame. Here a still of each
// programme, drawn when the pack is built (tools/orre/tv.py, "tv/<name>.bin" in the data pack, 64 x 64 RGBA), put on
// the screen's meshes (the room model's set the table names: their texture number is -(2 + set), build_pack.py)
struct OrreTv { u16 room; u8 id, set; const char *pic; };
#include "../data/orre_tv.h"

static struct
{
    bool8 on;
    s16 mark;                 // the screen meshes' texture number
    u16 room;
    s16 id;
    OTex tex;
    u32 px[TV_SIZE * TV_SIZE];
} sTv;

// The room's TV shows channel ID (a negative one, or one the room hasn't: off, the dark screen)
void O3D_TvSet(u16 room, s32 id)
{
    u32 i, size = 0;
    char name[64];
    u8 *d;
    if (sTv.on && sTv.room == room && sTv.id == id)
        return;
    sTv.on = FALSE;
    sTv.room = room, sTv.id = (s16)id;
    for (i = 0; i < ARRAY_COUNT(sOrreTv); i++)
        if (sOrreTv[i].room == room && sOrreTv[i].id == id)
            break;
    if (id < 0 || i == ARRAY_COUNT(sOrreTv))
        return;
    snprintf(name, sizeof(name), "tv/%s.bin", sOrreTv[i].pic);
    if ((d = OrreData_Read(name, &size)) == NULL || size < sizeof(sTv.px))
    {
        free(d);
        return;   // (a pack from before the TVs' pictures: the dark screen, as it was)
    }
    memcpy(sTv.px, d, sizeof(sTv.px));
    free(d);
    sTv.tex.w = sTv.tex.h = TV_SIZE;
    sTv.tex.px = sTv.px;
    sTv.tex.mip[0] = sTv.px;
    sTv.tex.nmip = 1;
    sTv.mark = -(2 + sOrreTv[i].set);
    sTv.on = TRUE;
    printf("[o3d] room %u's TV shows %s\n", room, sOrreTv[i].pic);
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
    const OTex *t = g->tex >= 0 ? &sPack.tex[g->tex] : sTv.on && g->tex == sTv.mark ? &sTv.tex : NULL;
    bool8 clear = g->alpha < 31, smooth = FALSE;
    s32 lod = 0, tw = 0, th = 0;
    const u32 *tex = NULL;
    float tscale = 1.0f;
    if (area == 0.0f)
        return;
    if (t && t->w == 0)
        t = NULL;
    if (t)
    {
        // (the triangle's texels a pixel: its area on the texture over its area on the screen)
        float ua = a->u / a->iz, va = a->v / a->iz, ub = b->u / b->iz, vb = b->v / b->iz, uc = c->u / c->iz, vc = c->v / c->iz;
        float ta = fabsf((ub - ua) * (vc - va) - (uc - ua) * (vb - va)), ratio = ta / fabsf(area);
        if (ratio > 1.0f)
        {
            lod = (s32)(0.5f * log2f(ratio) + 0.5f);
            if (lod > t->nmip - 1)
                lod = t->nmip - 1;
            if (lod < 0)
                lod = 0;
        }
        smooth = ratio < 0.7f;
        tex = t->mip[lod], tw = t->w >> lod, th = t->h >> lod, tscale = 1.0f / (float)(1 << lod);
    }
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
                float fu = (w0 * a->u + w1 * b->u + w2 * c->u) * z * tscale, fv = (w0 * a->v + w1 * b->v + w2 * c->v) * z * tscale;
                u32 texel, ta;
                if (smooth)
                {
                    // (between the four nearest texels, weighed by their alpha)
                    float bu = fu - 0.5f, bv = fv - 0.5f, fx = bu - floorf(bu), fy = bv - floorf(bv);
                    s32 u0 = Wrap((s32)floorf(bu), tw, g->rs, g->fs), u1 = Wrap((s32)floorf(bu) + 1, tw, g->rs, g->fs);
                    s32 v0 = Wrap((s32)floorf(bv), th, g->rt, g->ft), v1 = Wrap((s32)floorf(bv) + 1, th, g->rt, g->ft);
                    u32 q[4] = {tex[v0 * tw + u0], tex[v0 * tw + u1], tex[v1 * tw + u0], tex[v1 * tw + u1]};
                    float wq[4] = {(1 - fx) * (1 - fy), fx * (1 - fy), (1 - fx) * fy, fx * fy}, sa = 0, sc[3] = {0, 0, 0};
                    u32 k;
                    for (k = 0; k < 4; k++)
                    {
                        float qa = (q[k] >> 24) * wq[k];
                        sa += qa;
                        sc[0] += (q[k] & 255) * qa, sc[1] += ((q[k] >> 8) & 255) * qa, sc[2] += ((q[k] >> 16) & 255) * qa;
                    }
                    ta = (u32)(sa + 0.5f);
                    texel = sa > 0.0f ? ((u32)(sc[0] / sa) | (u32)(sc[1] / sa) << 8 | (u32)(sc[2] / sa) << 16 | ta << 24) : 0;
                }
                else
                {
                    s32 tu = Wrap((s32)floorf(fu), tw, g->rs, g->fs);
                    s32 tv = Wrap((s32)floorf(fv), th, g->rt, g->ft);
                    texel = tex[tv * tw + tu];
                    ta = texel >> 24;
                }
                if (ta == 0 || (g->solid && ta < 128))
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
            if (g->solid)
                al = 1.0f;
            sPropMask[y][x] = g->solid;
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
// A triangle partly behind the camera, cut at the near plane (depth NEAR_Z): the part in front drawn as a fan
#define NEAR_Z 1.5f
static void DrawClipped(const OVtx *tri, const OMesh *g)
{
    OVtx in[3], out[4];
    float d[3];
    u32 i, n = 0;
    SVtx sv[4];
    for (i = 0; i < 3; i++)
    {
        float dx = tri[i].x - sView.eye[0], dy = tri[i].y - sView.eye[1], dz = tri[i].z - sView.eye[2];
        in[i] = tri[i];
        d[i] = dx * sView.fwd[0] + dy * sView.fwd[1] + dz * sView.fwd[2];
    }
    for (i = 0; i < 3 && n < 4; i++)
    {
        u32 j = (i + 1) % 3;
        if (d[i] >= NEAR_Z)
            out[n++] = in[i];
        if ((d[i] >= NEAR_Z) != (d[j] >= NEAR_Z) && n < 4)
        {
            float t = (NEAR_Z - d[i]) / (d[j] - d[i]);
            OVtx *o = &out[n++];
            o->x = in[i].x + (in[j].x - in[i].x) * t, o->y = in[i].y + (in[j].y - in[i].y) * t;
            o->z = in[i].z + (in[j].z - in[i].z) * t, o->u = in[i].u + (in[j].u - in[i].u) * t;
            o->v = in[i].v + (in[j].v - in[i].v) * t;
            o->r = (u8)(in[i].r + (in[j].r - in[i].r) * t), o->g = (u8)(in[i].g + (in[j].g - in[i].g) * t);
            o->b = (u8)(in[i].b + (in[j].b - in[i].b) * t);
        }
    }
    if (n < 3)
        return;
    for (i = 0; i < n; i++)
    {
        float depth;
        if (!Project(out[i].x, out[i].y, out[i].z, &sv[i].x, &sv[i].y, &depth))
            depth = NEAR_Z, sv[i].x = sView.ax, sv[i].y = sView.ay;   // (on the plane: as near as it gets)
        sv[i].iz = 1.0f / depth;
        sv[i].u = (out[i].u + g->du) * sv[i].iz, sv[i].v = (out[i].v + g->dv) * sv[i].iz;
        sv[i].c[0] = out[i].r * sv[i].iz, sv[i].c[1] = out[i].g * sv[i].iz, sv[i].c[2] = out[i].b * sv[i].iz;
    }
    DrawTriangle(&sv[0], &sv[1], &sv[2], g);
    if (n == 4)
        DrawTriangle(&sv[0], &sv[2], &sv[3], g);
}

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
        s->u = (v->u + g->du) * s->iz;
        s->v = (v->v + g->dv) * s->iz;
        s->c[0] = v->r * s->iz, s->c[1] = v->g * s->iz, s->c[2] = v->b * s->iz;
        if (sTv.on && g->tex == sTv.mark)   // (a lit screen: the picture as it is, not under the dark glass's colour)
            s->c[0] = s->c[1] = s->c[2] = 31.0f * s->iz;
    }
    for (i = 0; i + 2 < g->count; i += 3)
    {
        if (sScratchOk[i] && sScratchOk[i + 1] && sScratchOk[i + 2])
            DrawTriangle(&sScratch[i], &sScratch[i + 1], &sScratch[i + 2], g);
        else if (sScratchOk[i] || sScratchOk[i + 1] || sScratchOk[i + 2])
            DrawClipped(&g->v[i], g);   // (partly behind the camera: a shot's ground passing under it)
    }
}

// ---- the props: the rooms' people with no ORRE person, drawn as Colosseum's own model (tools/orre/build_pack.py packs
// each as "*<model>", posed, about its feet) where its person stands on the ground, turned as it's turned
#define PROP_MODELS 32
#define PROPS_SHOWN 24
static struct { char name[24]; OMap m; } sPropModel[PROP_MODELS];
static OMesh sPropMesh;
static u32 sPropMeshCap;

static const OMap *PropModel(const char *model)
{
    u32 i;
    char file[32];
    for (i = 0; i < PROP_MODELS; i++)
    {
        if (sPropModel[i].name[0] == 0)
        {
            snprintf(sPropModel[i].name, sizeof(sPropModel[i].name), "%s", model);
            snprintf(file, sizeof(file), "*%s", model);
            if (RoomLoad(&sPropModel[i].m, file, -1))
            {
                // (a character's materials are marked see-through for their edges: whole, as the game draws them)
                u32 k;
                for (k = 0; k < sPropModel[i].m.nmesh; k++)
                    sPropModel[i].m.mesh[k].alpha = 31, sPropModel[i].m.mesh[k].solid = TRUE;
            }
        }
        if (strcmp(sPropModel[i].name, model) == 0)
            return sPropModel[i].m.ok ? &sPropModel[i].m : NULL;
    }
    return NULL;
}

// A soft outline round the props' models (the user, 2026-11-08: "to differentiate them better"): each pixel just
// outside one, where nothing nearer covers the edge, darkened halfway
static void OutlineProps(void)
{
    static const s8 sNear[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    static u8 sEdge[H][W];
    u32 x, y, k;
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
        {
            sEdge[y][x] = 0;
            if (sPropMask[y][x])
                continue;
            for (k = 0; k < 4; k++)
            {
                s32 nx = (s32)x + sNear[k][0], ny = (s32)y + sNear[k][1];
                if (nx >= 0 && ny >= 0 && nx < W && ny < H && sPropMask[ny][nx] && sDepth[ny][nx] <= sDepth[y][x])
                {
                    sEdge[y][x] = 1;
                    break;
                }
            }
        }
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
            if (sEdge[y][x])
            {
                u16 c = sColor[y][x];
                sColor[y][x] = (((c & 31) / 2) | (((c >> 5) & 31) / 2) << 5 | (((c >> 10) & 31) / 2) << 10);
            }
}

static void DrawProps(const OMap *g)
{
    const char *model[PROPS_SHOWN];
    float x[PROPS_SHOWN], z[PROPS_SHOWN], angle[PROPS_SHOWN];
    u32 n = Orre_Npc_Props(model, x, z, angle, PROPS_SHOWN), i, k, j, pass;
    float S = 16.0f / g->tile;
    for (i = 0; i < n; i++)
    {
        const OMap *pm = PropModel(model[i]);
        float a = angle[i] * 3.14159265f / 180.0f, c = cosf(a), sn = sinf(a);
        float wx = (x[i] - g->x0) * S, wz = (z[i] - g->z0) * S, wy;
        if (pm == NULL)
            continue;
        wy = GroundAt(wx, wz, sView.ty);
        for (pass = 0; pass < 2; pass++)
            for (k = 0; k < pm->nmesh; k++)
            {
                const OMesh *src = &pm->mesh[k];
                float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
                if (src->sky)
                    continue;
                if (src->count > sPropMeshCap)
                {
                    sPropMeshCap = src->count + 256;
                    sPropMesh.v = realloc(sPropMesh.v, sizeof(OVtx) * sPropMeshCap);
                }
                {
                    OVtx *keep = sPropMesh.v;
                    sPropMesh = *src;
                    sPropMesh.v = keep;
                }
                for (j = 0; j < src->count; j++)
                {
                    const OVtx *v = &src->v[j];
                    OVtx *d = &sPropMesh.v[j];
                    u32 ch;
                    *d = *v;
                    // (turned about Y: 0 faces +Z, 90 +X, as Colosseum's angles)
                    d->x = wx + v->x * c + v->z * sn;
                    d->z = wz - v->x * sn + v->z * c;
                    d->y = wy + v->y;
                    for (ch = 0; ch < 3; ch++)
                    {
                        float q = ch == 0 ? d->x : ch == 1 ? d->y : d->z;
                        lo[ch] = q < lo[ch] ? q : lo[ch];
                        hi[ch] = q > hi[ch] ? q : hi[ch];
                    }
                }
                for (j = 0; j < 3; j++)
                    sPropMesh.c[j] = (lo[j] + hi[j]) / 2;
                sPropMesh.r = sqrtf((hi[0] - lo[0]) * (hi[0] - lo[0]) + (hi[1] - lo[1]) * (hi[1] - lo[1])
                                    + (hi[2] - lo[2]) * (hi[2] - lo[2])) / 2;
                DrawMesh(&sPropMesh, pass);
            }
    }
}

// This frame of what moves in map G (60 a second, each animation in its own loop; a door's leaf by its door)
static u32 sAnimTick;

static void Animate(OMap *g)
{
    u32 i, k;
    float S = 16.0f / g->tile;
    for (i = 0; i < g->nmesh; i++)
    {
        OMesh *me = &g->mesh[i];
        const float *a;
        if (!me->animKind)
            continue;
        if (me->animKind == 3)
        {
            // (a door's leaf: shut at the opening animation's first frame, open at its last)
            u32 at = (u64)sDoorPos[me->animDoor] * me->animCount / me->animPeriod;
            a = me->anim + (at < me->animCount ? at : me->animCount - 1) * 12;
        }
        else
        a = me->anim + (u64)(sAnimTick % me->animPeriod) * me->animCount / me->animPeriod * (me->animKind == 1 ? 2 : 12);
        if (me->animKind == 1)
        {
            me->du = a[0] * me->su;
            me->dv = a[1] * me->sv;
            continue;
        }
        for (k = 0; k <= me->count; k++)
        {
            // (the map's world back to Colosseum's, the joint's move, and back)
            const float *src = k < me->count ? &me->v0[k].x : me->c0;
            float *dst = k < me->count ? &me->v[k].x : me->c;
            float px = src[0] / S + g->x0, py = src[1] / S, pz = src[2] / S + g->z0;
            dst[0] = (a[0] * px + a[1] * py + a[2] * pz + a[3] - g->x0) * S;
            dst[1] = (a[4] * px + a[5] * py + a[6] * pz + a[7]) * S;
            dst[2] = (a[8] * px + a[9] * py + a[10] * pz + a[11] - g->z0) * S;
        }
    }
}

static void Render(void)
{
    u32 i, pass;
    for (i = 0; i < W * H; i++)
    {
        (&sColor[0][0])[i] = 0;
        (&sDepth[0][0])[i] = 1e30f;
        (&sIds[0][0])[i] = -1;
        (&sPropMask[0][0])[i] = 0;
    }
    const OMap *g = Drawn();
    if (g == NULL)
        return;
    sAnimTick++;
    {
        u32 d;
        for (d = 0; d < 256; d++)
        {
            if (sDoorOpen[d] && sDoorPos[d] < sDoorLen[d])
                sDoorPos[d]++;
            else if (!sDoorOpen[d] && sDoorPos[d])
                sDoorPos[d]--;
        }
    }
    Animate((OMap *)g);   // (the water sliding, the wheels turning: Colosseum's own animations)
    if (O3D_CamActive())
    {
        // (a shot of Colosseum's: its sky first, behind everything: the depth cleared after it)
        for (pass = 0; pass < 2; pass++)
            for (i = 0; i < g->nmesh; i++)
                if (g->mesh[i].sky)
                    DrawMesh(&g->mesh[i], pass);
        for (i = 0; i < W * H; i++)
            (&sDepth[0][0])[i] = 1e30f;
    }
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < g->nmesh; i++)
            if (!g->mesh[i].sky)
                DrawMesh(&g->mesh[i], pass);
    if (g == sCur)
    {
        DrawProps(g);   // (the room's props: not in a scene room of the opening's)
        OutlineProps();
    }
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

// The trigger (Colosseum's collision id) on tile X, Y of the current ORRE map (map coordinates, no MAP_OFFSET); -1 none
s32 O3D_TriggerAt(s32 x, s32 y)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    const OMap *g;
    if (map < 0 || !MapLoad((u32)map))
        return -1;
    g = &sPack.map[map];
    if (g->trig == NULL || x < 0 || y < 0 || x >= g->w || y >= g->h)
        return -1;
    return g->trig[(u32)y * g->w + x];
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
    SceneFacing();     // (people's sides as a scene's camera sees them)
    if (!sView.active)
    {
        sCur = NULL;
        if (O3D_CamActive())
            O3D_CamStop();   // (off ORRE: no shot)
        return;
    }
    if (sView.map != map)
    {
        u32 i;
        sView.havePlayerH = FALSE;   // (a new map: its own ground)
        // (and every sprite's last height forgotten: the PRE GYM's floor, 0, put the player on PHENAC's lower level
        // under the plaza by its door, hidden in it until they walked off the two-level tiles: the user, 2026-11-08)
        for (i = 0; i < MAX_SPRITES; i++)
            sSpriteHFrame[i] = (u8)(sView.frame - 100);
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
