// Moemon Kai: SINNOH's 3D engine (docs/SINNOH_HANDOFF.md). A SINNOH map is a map of the field engine like any other
// (grid movement, scripts, warps, co-op), but what is shown of it is Platinum's own 3D world: its map blocks, their
// props (houses, fences, signs) and textures, lit by the area's lights for the time of day, drawn by this software
// renderer into the game's 240x160 picture each frame. The renderer (gba_easy_draw.c) hands each line of the field's BG
// layers to S3dLine, which puts the 3D picture in the bottom layer and clears the map's other layers; the text
// windows (BG0) and the sprites go on top as on any map. Field sprites (the ones that follow the camera) are moved to
// where their spot on the ground is in the 3D view (sprite.c UpdateOamCoords -> S3D_PlaceSprite).
//
// The world comes from the 3D pack (sinnoh/platinum3d.pak beside the game) the release builds from pret/pokeplatinum
// (tools/sinnoh/make_pack.sh; nothing of it is in the repository): meshes in Platinum's units (16 a tile, a block of 32x32 tiles is 512 wide, centred on
// its middle), textures as RGBA, the area lights, and per matrix cell its block, props and the field engine's map
// blocks (collision and behaviors) that S3D_LayoutBlocks gives the map loader in place of the layout's placeholders.
//
// The camera is Platinum's default field camera (666.92 units away, 59.05 degrees down) at the DS's own pixel scale:
// one unit a pixel at the target, 16 pixels a tile as Emerald's maps, so 240x160 is a narrower slice of the DS view.
// It follows Emerald's own camera: the player's spot on Emerald's screen is the 3D view's target.
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "gpu_regs.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "s3d.h"
#include "sprite.h"
#include "constants/maps.h"

extern void (*gMpS3dLine)(int line, u16 layers[4][DISPLAY_WIDTH]);
const char *Updater_ExeFile(const char *name);
extern bool (*gMpS3dOamHidden)(int oamIndex, int x, int line);

#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define BLOCK 512.0f
#define CAM_DIST 666.922119140625f
#define CAM_PITCH 59.051513671875f
#define HALF_FOV 8.0914306640625f

// ------------------------------------------------------------------------------------------------ the pack
typedef struct { float x, y, z, u, v; u8 r, g, b, pad; float nx, ny, nz; } S3dVtx;
typedef struct
{
    s16 tex;
    u8 rs, rt, fs, ft, alpha, pid, cull, lights, mode;
    u32 count;     // vertices (3 a triangle)
    S3dVtx *v;
    float *lit;    // r g b a vertex, for the light block litFor
    s32 litFor;
} S3dGroup;
typedef struct { float c[3], r; u32 ngroups; S3dGroup *g; } S3dMesh;
typedef struct { u16 w, h; u32 *px; } S3dTex;   // r | g << 8 | b << 16 | a << 24
typedef struct { u8 on, r, g, b; float d[3]; } S3dLight;
typedef struct { u32 start; S3dLight l[4]; u8 col[4][3]; } S3dLightBlock;   // start: its END time (2-second units); diffuse, ambient, specular, emission
typedef struct { u32 count; S3dLightBlock *b; } S3dLightSet;
typedef struct { s16 mesh; float m[12]; } S3dProp;
typedef struct { s16 mesh, alt; u16 light, nprops; S3dProp *props; u16 blocks[32 * 32]; } S3dCell;

static struct
{
    bool8 tried, ok;
    u32 ntex, nmesh, nlight;
    S3dTex *tex;
    S3dMesh *mesh;
    S3dLightSet *light;
    u16 width, height, x0, z0;   // the cells kept: matrix cells (x0, z0) on
    S3dCell *cells;
} sPack;

// SINNOH's maps: the matrix cell of each one's top-left block and its size in blocks
static const struct { u16 map; u8 cx, cz, bw, bh; } sMaps[] = {
    {MAP_SINNOH_TWINLEAF_TOWN, 3, 27, 1, 1},
    {MAP_SINNOH_ROUTE201, 3, 26, 2, 1},
};

typedef struct { const u8 *p, *end; bool8 bad; } Reader;

static void Take(Reader *r, void *dst, size_t n)
{
    if (r->bad || (size_t)(r->end - r->p) < n)
    {
        r->bad = TRUE;
        memset(dst, 0, n);
        return;
    }
    memcpy(dst, r->p, n);
    r->p += n;
}

static u32 TakeU32(Reader *r) { u32 v; Take(r, &v, 4); return v; }
static u16 TakeU16(Reader *r) { u16 v; Take(r, &v, 2); return v; }

static void LoadPack(void)
{
    const char *path = getenv("MOEMON_S3D_PAK");
    FILE *f;
    long size;
    u8 *data;
    Reader r;
    u32 i, j, k;
    sPack.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum3d.pak");   // (beside the game, as music/)
    f = fopen(path, "rb");
    if (f == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size)
    {
        fclose(f);
        free(data);
        return;
    }
    fclose(f);
    r.p = data;
    r.end = data + size;
    r.bad = FALSE;
    if (memcmp(data, "S3DP", 4) != 0)
    {
        printf("[s3d] %s isn't a SINNOH pack\n", path);
        free(data);
        return;
    }
    r.p += 4;
    if (TakeU32(&r) != 1)
    {
        printf("[s3d] %s: another pack version\n", path);
        free(data);
        return;
    }
    sPack.ntex = TakeU32(&r);
    sPack.tex = calloc(sPack.ntex, sizeof(S3dTex));
    for (i = 0; i < sPack.ntex && !r.bad; i++)
    {
        sPack.tex[i].w = TakeU16(&r);
        sPack.tex[i].h = TakeU16(&r);
        sPack.tex[i].px = malloc(4 * sPack.tex[i].w * sPack.tex[i].h);
        Take(&r, sPack.tex[i].px, 4 * sPack.tex[i].w * sPack.tex[i].h);
    }
    sPack.nmesh = TakeU32(&r);
    sPack.mesh = calloc(sPack.nmesh, sizeof(S3dMesh));
    for (i = 0; i < sPack.nmesh && !r.bad; i++)
    {
        S3dMesh *m = &sPack.mesh[i];
        Take(&r, m->c, 12);
        Take(&r, &m->r, 4);
        m->ngroups = TakeU32(&r);
        m->g = calloc(m->ngroups, sizeof(S3dGroup));
        for (j = 0; j < m->ngroups && !r.bad; j++)
        {
            S3dGroup *g = &m->g[j];
            u8 b[10];
            Take(&r, &g->tex, 2);
            Take(&r, b, 10);
            g->rs = b[0], g->rt = b[1], g->fs = b[2], g->ft = b[3], g->alpha = b[4], g->pid = b[5], g->cull = b[6];
            g->lights = b[7], g->mode = b[8];
            g->count = TakeU32(&r) * 3;
            g->v = malloc(sizeof(S3dVtx) * g->count);
            for (k = 0; k < g->count; k++)
                Take(&r, &g->v[k], 36);
            g->lit = malloc(sizeof(float) * 3 * g->count);
            g->litFor = -1;
        }
    }
    sPack.nlight = TakeU32(&r);
    sPack.light = calloc(sPack.nlight, sizeof(S3dLightSet));
    for (i = 0; i < sPack.nlight && !r.bad; i++)
    {
        S3dLightSet *s = &sPack.light[i];
        s->count = TakeU32(&r);
        s->b = calloc(s->count, sizeof(S3dLightBlock));
        for (j = 0; j < s->count && !r.bad; j++)
        {
            S3dLightBlock *b = &s->b[j];
            u8 pad[4];
            b->start = TakeU32(&r);
            for (k = 0; k < 4; k++)
            {
                Take(&r, &b->l[k].on, 4);
                Take(&r, b->l[k].d, 12);
            }
            Take(&r, b->col, 12);
            Take(&r, pad, 4);
        }
    }
    sPack.width = TakeU16(&r);
    sPack.height = TakeU16(&r);
    sPack.x0 = TakeU16(&r);
    sPack.z0 = TakeU16(&r);
    sPack.cells = calloc((u32)sPack.width * sPack.height, sizeof(S3dCell));
    for (i = 0; i < (u32)sPack.width * sPack.height && !r.bad; i++)
    {
        S3dCell *c = &sPack.cells[i];
        Take(&r, &c->mesh, 2);
        Take(&r, &c->alt, 2);
        c->light = TakeU16(&r);
        c->nprops = TakeU16(&r);
        c->props = calloc(c->nprops ? c->nprops : 1, sizeof(S3dProp));
        for (j = 0; j < c->nprops; j++)
        {
            Take(&r, &c->props[j].mesh, 2);
            Take(&r, c->props[j].m, 48);
        }
        Take(&r, c->blocks, sizeof(c->blocks));
    }
    free(data);
    if (r.bad)
    {
        printf("[s3d] %s is cut short\n", path);
        return;
    }
    sPack.ok = TRUE;
    printf("[s3d] %s: %u textures, %u meshes, %ux%u cells from (%u, %u)\n", path, sPack.ntex, sPack.nmesh, sPack.width,
           sPack.height, sPack.x0, sPack.z0);
}

static const S3dCell *Cell(s32 cx, s32 cz)
{
    cx -= sPack.x0;
    cz -= sPack.z0;
    if (!sPack.ok || cx < 0 || cz < 0 || cx >= sPack.width || cz >= sPack.height)
        return NULL;
    return &sPack.cells[cz * sPack.width + cx];
}

static s32 MapIndex(u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
        if (sMaps[i].map == ((group << 8) | num))
            return i;
    return -1;
}

// ------------------------------------------------------------------------------------------------ map blocks
const u16 *S3D_LayoutBlocks(const struct MapLayout *layout)
{
    static u16 *sBlocks[ARRAY_COUNT(sMaps)];
    u32 i, bx, bz, x, y;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
    {
        const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(sMaps[i].map >> 8, sMaps[i].map & 0xFF);
        if (h->mapLayout != layout)
            continue;
        if (!sPack.tried)
            LoadPack();
        if (!sPack.ok)
            return layout->map;
        if (sBlocks[i] == NULL)
        {
            u32 w = 32 * sMaps[i].bw;
            sBlocks[i] = malloc(sizeof(u16) * w * 32 * sMaps[i].bh);
            for (bz = 0; bz < sMaps[i].bh; bz++)
                for (bx = 0; bx < sMaps[i].bw; bx++)
                {
                    const S3dCell *c = Cell(sMaps[i].cx + bx, sMaps[i].cz + bz);
                    for (y = 0; y < 32; y++)
                        for (x = 0; x < 32; x++)
                            sBlocks[i][(bz * 32 + y) * w + bx * 32 + x] = c ? c->blocks[y * 32 + x] : layout->map[0];
                }
        }
        return sBlocks[i];
    }
    return layout->map;
}

// ------------------------------------------------------------------------------------------------ camera
static struct
{
    bool8 active;
    s32 map;                 // sMaps index
    s32 camX, camY;          // Emerald's screen (0, 0) in the map's pixels
    float originX, originZ;  // the map's (0, 0) in Platinum's world
    float tx, ty, tz;        // the target (the player's tile centre)
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
    // (as mp_mappic.c: the BG tilemap holds the map from the camera's tile (pos - 7) on at its tile offset)
    hoffs = (s8)((hoffs - tileX * 8) & 255);
    voffs = (s8)((voffs - tileY * 8) & 255);
    sView.camX = (gSaveBlock1Ptr->pos.x - 7) * 16 + hoffs;
    sView.camY = (gSaveBlock1Ptr->pos.y - 7) * 16 + voffs;
}

static void SetupView(void)
{
    const struct Sprite *s = &gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId];
    float p = CAM_PITCH * 3.14159265f / 180.0f;
    float sp = sinf(p), cp = cosf(p);
    // The player's tile centre: their sprite's middle, 8 pixels above its feet (the tile's bottom edge)
    sView.ax = s->x + s->x2 + gSpriteCoordOffsetX;
    sView.ay = s->y + s->y2 + gSpriteCoordOffsetY - s->centerToCornerVecY - 8;
    sView.originX = sMaps[sView.map].cx * BLOCK;
    sView.originZ = sMaps[sView.map].cz * BLOCK;
    sView.tx = sView.originX + sView.camX + sView.ax;
    sView.tz = sView.originZ + sView.camY + sView.ay;
    sView.ty = 0.0f;
    sView.eye[0] = sView.tx;
    sView.eye[1] = sView.ty + CAM_DIST * sp;
    sView.eye[2] = sView.tz + CAM_DIST * cp;
    sView.fwd[0] = 0.0f, sView.fwd[1] = -sp, sView.fwd[2] = -cp;
    sView.right[0] = 1.0f, sView.right[1] = 0.0f, sView.right[2] = 0.0f;
    sView.up[0] = 0.0f, sView.up[1] = cp, sView.up[2] = -sp;
    sView.focal = 96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f);
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

void S3D_PlaceSprite(s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth)
{
    float sx, sy, d;
    float wx = sView.originX + sView.camX + x, wz = sView.originZ + sView.camY + feetY - 8;
    if (!sView.active || !Project(wx, 0.0f, wz, &sx, &sy, &d))
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

// ------------------------------------------------------------------------------------------------ sprites in the scene
// A sprite is an upright card standing at its spot: a pixel dy above its feet is dy / cos(pitch) units up the card,
// so dy * tan(pitch) nearer the camera (about one unit a pixel at the target)
float gS3dSpriteDepth[MAX_SPRITES];
s16 gS3dSpriteFeet[MAX_SPRITES];
typedef struct { float d0; s16 feet; } OamDepth;
static OamDepth sPendingOam[OAM_ENTRY_COUNT], sLiveOam[OAM_ENTRY_COUNT];
static bool8 sLiveAny;

void S3D_NoteOam(u16 spriteId, u16 first, u16 end)
{
    u16 i;
    bool8 on = sView.active && spriteId < MAX_SPRITES && gSprites[spriteId].coordOffsetEnabled;
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
    {
        sPendingOam[i].d0 = on ? gS3dSpriteDepth[spriteId] : 0.0f;
        sPendingOam[i].feet = on ? gS3dSpriteFeet[spriteId] : 0;
    }
}

void S3D_CommitOam(void)
{
    memcpy(sLiveOam, sPendingOam, sizeof(sLiveOam));
    sLiveAny = sView.active;
    if (sView.active)
        memset(sPendingOam, 0, sizeof(sPendingOam));
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
    d = o->d0 - (o->feet - line) * 1.6652f;   // tan(59.05 degrees)
    return sDepth[line][x] < d - 6.0f;
}

// ------------------------------------------------------------------------------------------------ lights
static s32 sLightBlock = -1;
static const S3dLightBlock *sLight;

static void PickLight(u16 set)
{
    const S3dLightSet *s;
    u32 now, i, pick = 0;
    if (set >= sPack.nlight || sPack.light[set].count == 0)
    {
        sLight = NULL;
        return;
    }
    s = &sPack.light[set];
    now = (gLocalTime.hours * 3600 + gLocalTime.minutes * 60) / 2;
    if (getenv("MOEMON_S3D_HOUR"))   // (tests: a fixed hour)
        now = atoi(getenv("MOEMON_S3D_HOUR")) * 1800;
    // (Platinum's area_light.c: the first block that ends after now, in 2-second units of the day)
    pick = s->count - 1;
    for (i = 0; i < s->count; i++)
        if (s->b[i].start > now)
        {
            pick = i;
            break;
        }
    sLight = &s->b[pick];
    sLightBlock = set * 256 + pick;
}

static void Light(S3dGroup *g)
{
    u32 i, k, ch;
    if (g->litFor == sLightBlock || sLight == NULL)
        return;
    g->litFor = sLightBlock;
    for (i = 0; i < g->count; i++)
    {
        const S3dVtx *v = &g->v[i];
        float n[3] = {v->nx, v->ny, v->nz};
        float len = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        float acc[3];
        if (len > 0.0f)
            n[0] /= len, n[1] /= len, n[2] /= len;
        for (ch = 0; ch < 3; ch++)
            acc[ch] = sLight->col[3][ch];
        for (k = 0; k < 4; k++)
        {
            const S3dLight *l = &sLight->l[k];
            float level, lc[3] = {l->r, l->g, l->b};
            if (!l->on || !(g->lights & (1 << k)))
                continue;
            level = -(l->d[0] * n[0] + l->d[1] * n[1] + l->d[2] * n[2]);
            if (level < 0.0f)
                level = 0.0f;
            for (ch = 0; ch < 3; ch++)
                acc[ch] += (sLight->col[0][ch] * lc[ch] * level + sLight->col[1][ch] * lc[ch]) / 31.0f;
        }
        for (ch = 0; ch < 3; ch++)
            g->lit[3 * i + ch] = acc[ch] > 31.0f ? 31.0f : acc[ch];
    }
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

static void DrawTriangle(const SVtx *a, const SVtx *b, const SVtx *c, const S3dGroup *g)
{
    float area = (b->x - a->x) * (c->y - a->y) - (c->x - a->x) * (b->y - a->y);
    s32 minx, maxx, miny, maxy, x, y;
    const S3dTex *t = g->tex >= 0 && (u32)g->tex < sPack.ntex ? &sPack.tex[g->tex] : NULL;
    bool8 clear = g->alpha < 31;
    if (area == 0.0f || g->cull == 3 || (g->cull == 1 && area > 0.0f) || (g->cull == 2 && area < 0.0f))
        return;
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
                    out[ch] = g->mode == 0 ? (tc + 1.0f) * (col[ch] + 1.0f) / 32.0f - 1.0f : tc;
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
                u16 px = 0;
                for (ch = 0; ch < 3; ch++)
                {
                    float o = (old >> (5 * ch)) & 31;
                    float v = out[ch] * al + o * (1.0f - al);
                    s32 iv = (s32)(v + 0.5f);
                    px |= (iv < 0 ? 0 : iv > 31 ? 31 : iv) << (5 * ch);
                }
                sColor[y][x] = px;
            }
            if (clear)
            {
                sIds[y][x] = g->pid;
            }
            else
            {
                sDepth[y][x] = z;
                sIds[y][x] = -1;
            }
        }
    }
}

static SVtx *sScratch;
static u32 sScratchSize;
static u8 *sScratchOk;

// One mesh placed at (ox, oy, oz) with a 3x3 (rows) transform m (NULL: none); pass 0 draws the opaque groups, 1 the
// translucent ones
static void DrawMesh(const S3dMesh *mesh, const float *m, float ox, float oy, float oz, int pass)
{
    u32 gi, i;
    float cx = mesh->c[0], cy = mesh->c[1], cz = mesh->c[2], sx, sy, d;
    if (m)
    {
        float x = cx, y = cy, z = cz;
        cx = m[0] * x + m[1] * y + m[2] * z + m[3];
        cy = m[4] * x + m[5] * y + m[6] * z + m[7];
        cz = m[8] * x + m[9] * y + m[10] * z + m[11];
    }
    // (off screen by more than its radius: skipped)
    if (Project(cx + ox, cy + oy, cz + oz, &sx, &sy, &d))
    {
        float rad = mesh->r * sView.focal / d * 1.5f + 2.0f;
        if (sx + rad < 0 || sx - rad > W || sy + rad < 0 || sy - rad > H)
            return;
    }
    for (gi = 0; gi < mesh->ngroups; gi++)
    {
        S3dGroup *g = &mesh->g[gi];
        if ((g->alpha < 31) != pass)
            continue;
        if (g->count > sScratchSize)
        {
            sScratchSize = g->count + 1024;
            sScratch = realloc(sScratch, sizeof(SVtx) * sScratchSize);
            sScratchOk = realloc(sScratchOk, sScratchSize);
        }
        if (g->lights)
            Light(g);
        for (i = 0; i < g->count; i++)
        {
            const S3dVtx *v = &g->v[i];
            float x = v->x, y = v->y, z = v->z, depth;
            SVtx *s = &sScratch[i];
            if (m)
            {
                x = m[0] * v->x + m[1] * v->y + m[2] * v->z + m[3];
                y = m[4] * v->x + m[5] * v->y + m[6] * v->z + m[7];
                z = m[8] * v->x + m[9] * v->y + m[10] * v->z + m[11];
            }
            sScratchOk[i] = Project(x + ox, y + oy, z + oz, &s->x, &s->y, &depth);
            if (!sScratchOk[i])
                continue;
            s->iz = 1.0f / depth;
            s->u = v->u * s->iz;
            s->v = v->v * s->iz;
            if (g->lights && sLight)
            {
                s->c[0] = g->lit[3 * i] * s->iz, s->c[1] = g->lit[3 * i + 1] * s->iz, s->c[2] = g->lit[3 * i + 2] * s->iz;
            }
            else
            {
                s->c[0] = v->r * s->iz, s->c[1] = v->g * s->iz, s->c[2] = v->b * s->iz;
            }
        }
        for (i = 0; i + 2 < g->count; i += 3)
            if (sScratchOk[i] && sScratchOk[i + 1] && sScratchOk[i + 2])
                DrawTriangle(&sScratch[i], &sScratch[i + 1], &sScratch[i + 2], g);
    }
}

static void Render(void)
{
    s32 bx = (s32)floorf(sView.tx / BLOCK), bz = (s32)floorf(sView.tz / BLOCK), cx, cz;
    u32 i, pass;
    for (i = 0; i < W * H; i++)
    {
        (&sColor[0][0])[i] = 0;
        (&sDepth[0][0])[i] = 1e30f;
        (&sIds[0][0])[i] = -1;
    }
    {
        const S3dCell *here = Cell(bx, bz);
        PickLight(here ? here->light : 0);
    }
    for (pass = 0; pass < 2; pass++)
        for (cz = bz - 1; cz <= bz + 1; cz++)
            for (cx = bx - 1; cx <= bx + 1; cx++)
            {
                const S3dCell *c = Cell(cx, cz);
                float ox = cx * BLOCK + BLOCK / 2, oy = c ? c->alt * 16.0f : 0.0f, oz = cz * BLOCK + BLOCK / 2;
                if (c == NULL || c->mesh < 0 || (u32)c->mesh >= sPack.nmesh)
                    continue;
                DrawMesh(&sPack.mesh[c->mesh], NULL, ox, oy, oz, pass);
                for (i = 0; i < c->nprops; i++)
                    if (c->props[i].mesh >= 0 && (u32)c->props[i].mesh < sPack.nmesh)
                        DrawMesh(&sPack.mesh[c->props[i].mesh], c->props[i].m, ox, oy, oz, pass);
            }
}

// ------------------------------------------------------------------------------------------------ fades, lines
// (as mp_mappic.c: the shown BG palettes against the unfaded ones give each channel's c -> a + b * c)
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

static void S3dLine(int line, u16 layers[4][DISPLAY_WIDTH])
{
    s32 x;
    if (!sView.active || gMain.callback2 != CB2_Overworld)
        return;
    if (line == 0)
    {
        FindFade();
        Render();
    }
    for (x = 0; x < W; x++)
    {
        layers[3][x] = Faded(sColor[line][x]);
        layers[2][x] = 0;
        layers[1][x] = 0;
    }
}

// ------------------------------------------------------------------------------------------------ every frame
bool8 S3D_Active(void)
{
    return sView.active && gMain.callback2 == CB2_Overworld;
}

void S3D_Update(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map >= 0 && !sPack.tried)
        LoadPack();
    sView.active = map >= 0 && sPack.ok;
    gMpS3dLine = sView.active ? S3dLine : NULL;
    gMpS3dOamHidden = sView.active && !getenv("MOEMON_S3D_NOHIDE") ? OamHidden : NULL;
    if (!sView.active)
        return;
    sView.map = map;
    FindCamera();
    SetupView();
    if (getenv("MOEMON_S3D_TRACE"))
    {
        const struct Sprite *sp = &gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId];
        s32 px, py;
        float pd;
        S3D_PlaceSprite(sp->x + sp->x2 + gSpriteCoordOffsetX, sp->y + sp->y2 + gSpriteCoordOffsetY - sp->centerToCornerVecY, &px, &py, &pd);
        printf("[s3d] cam %d,%d anchor %.1f,%.1f target %.1f,%.1f sprite %d+%d+%d,%d+%d+%d c2c %d -> %d,%d\n", sView.camX,
               sView.camY, sView.ax, sView.ay, sView.tx, sView.tz, sp->x, sp->x2, gSpriteCoordOffsetX, sp->y, sp->y2,
               gSpriteCoordOffsetY, sp->centerToCornerVecY, px, py);
    }
}
