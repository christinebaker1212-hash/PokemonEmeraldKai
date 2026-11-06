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
#include "constants/event_objects.h"
#include "event_data.h"
#include "mp_region.h"
#include "gpu_regs.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
bool8 FieldEffect_IsReflectionSprite(const struct Sprite *sprite);   // (field_effect_helpers.c)
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "s3d.h"
#include "sprite.h"
#include "task.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "../data/sinnoh_night_songs.h"

extern void (*gMpS3dLine)(int line, u16 layers[4][DISPLAY_WIDTH]);
const char *Updater_ExeFile(const char *name);
unsigned long long SDL_GetPerformanceCounter(void);
unsigned long long SDL_GetPerformanceFrequency(void);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);   // (zlib)
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
typedef struct { u16 w, h; u32 *px; bool8 soft; } S3dTex;   // r | g << 8 | b << 16 | a << 24; soft: texels part see-through
typedef struct { u8 on, r, g, b; float d[3]; } S3dLight;
typedef struct { u32 start; S3dLight l[4]; u8 col[4][3]; } S3dLightBlock;   // start: its END time (2-second units); diffuse, ambient, specular, emission
typedef struct { u32 count; S3dLightBlock *b; } S3dLightSet;
typedef struct { s16 mesh; float m[12]; } S3dProp;
// hn / h: the ground's heights at each tile's centre (BDHC; more than one: a bridge over the ground), lowest first
typedef struct { s16 mesh, alt; u16 light, nprops; S3dProp *props; u16 blocks[32 * 32]; u8 hn[32 * 32]; float h[32 * 32][3]; } S3dCell;
// A Platinum matrix: 0 the region, others the buildings (each kept once; maps name theirs in sMaps); the cells kept
// start at (x0, z0)
typedef struct S3dMatrix { u16 number, width, height, x0, z0; S3dCell *cells; } S3dMatrix;

static struct
{
    bool8 tried, ok;
    u32 ntex, nmesh, nlight;
    S3dTex *tex;
    S3dMesh *mesh;
    S3dLightSet *light;
    u32 nmat;
    struct S3dMatrix *mat;
    u32 nnamed;
    struct { u16 model; s16 mesh; } *named;   // (version 4) the models the gyms load at run time
    // (version 5) Platinum's animated map textures (water, shores, lamps: fldtanime): each texture's frames in turn,
    // for so many of Platinum's field frames (30 a second); shown[t]: the texture drawn for t this frame
    u32 nanim;
    struct { u16 tex, nsteps, cycle; u16 (*steps)[2]; } *anim;
    u16 *shown;
    // (version 6) props that move all the time (Platinum's looping joint animations: windmills, a ship's screw...): a
    // mesh per frame, each for `step` of Platinum's frames; meshShown[m]: the mesh drawn for prop mesh m this frame
    u32 nmoving;
    struct { u16 mesh, nframes, step; u16 *frames; } *moving;
    u16 *meshShown;
    // (version 7) Platinum's door props: each one's animations (open, close; four: walking in, then walking out), a
    // mesh per frame; doorOf[m]: the door of prop mesh m + 1 (0: not a door)
    u32 ndoor;
    struct { u16 mesh; u8 sound, nanim; struct { u16 nframes, step; u16 *frames; } a[4]; } *door;
    u16 *doorOf;
} sPack;

// Props this game places at run time (SINNOH's gyms that move: src/sinnoh/s3d_gyms.c), in the current map's world
typedef struct { bool8 on; s16 mesh; float m[12]; } S3dDynProp;
static S3dDynProp sDyn[S3D_DYN_PROPS];
float (*gS3dHeightHook)(s32 mapX, s32 mapY, float h);
float (*gS3dObjectHeightHook)(u8 localId, float h);

// SINNOH's maps: the matrix cell of each one's top-left block and its size in blocks
#include "s3d_maps.h"   // sMaps: each SINNOH map and where it lies on the matrix (tools/sinnoh/gen_maps.py)

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
    u32 i, j, k, version;
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
    if (size >= 8 && memcmp(data, "S3DZ", 4) == 0)
    {
        // the pack is stored deflated (build_pack.py): inflated once here (about a quarter of a second)
        unsigned long rawSize = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
        u8 *raw = malloc(rawSize ? rawSize : 1);
        if (raw == NULL || uncompress(raw, &rawSize, data + 8, (unsigned long)(size - 8)) != 0)
        {
            printf("[s3d] %s: can't inflate the pack\n", path);
            free(raw);
            free(data);
            return;
        }
        free(data);
        data = raw;
        size = (long)rawSize;
    }
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
    version = TakeU32(&r);
    if (version < 3 || version > 7)
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
        {
            // (A3I5 / A5I3 textures: shadows, glass, soft leaf edges; the DS draws them after the solid ones)
            u32 k, n = sPack.tex[i].w * sPack.tex[i].h;
            for (k = 0; k < n && !sPack.tex[i].soft; k++)
                if ((sPack.tex[i].px[k] >> 24) != 0 && (sPack.tex[i].px[k] >> 24) != 0xFF)
                    sPack.tex[i].soft = TRUE;
        }
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
    sPack.nmat = TakeU32(&r);
    sPack.mat = calloc(sPack.nmat ? sPack.nmat : 1, sizeof(S3dMatrix));
    for (k = 0; k < sPack.nmat && !r.bad; k++)
    {
    S3dMatrix *mt = &sPack.mat[k];
    mt->number = TakeU16(&r);
    mt->width = TakeU16(&r);
    mt->height = TakeU16(&r);
    mt->x0 = TakeU16(&r);
    mt->z0 = TakeU16(&r);
    mt->cells = calloc((u32)mt->width * mt->height, sizeof(S3dCell));
    for (i = 0; i < (u32)mt->width * mt->height && !r.bad; i++)
    {
        S3dCell *c = &mt->cells[i];
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
        Take(&r, c->hn, sizeof(c->hn));
        Take(&r, c->h, sizeof(c->h));
    }
    }
    if (version >= 4)
    {
        sPack.nnamed = TakeU32(&r);
        sPack.named = calloc(sPack.nnamed ? sPack.nnamed : 1, sizeof(*sPack.named));
        for (i = 0; i < sPack.nnamed && !r.bad; i++)
        {
            sPack.named[i].model = TakeU16(&r);
            Take(&r, &sPack.named[i].mesh, 2);
        }
    }
    sPack.shown = malloc(sizeof(u16) * (sPack.ntex ? sPack.ntex : 1));
    for (i = 0; i < sPack.ntex; i++)
        sPack.shown[i] = i;
    if (version >= 5)
    {
        sPack.nanim = TakeU32(&r);
        sPack.anim = calloc(sPack.nanim ? sPack.nanim : 1, sizeof(*sPack.anim));
        for (i = 0; i < sPack.nanim && !r.bad; i++)
        {
            sPack.anim[i].tex = TakeU16(&r);
            sPack.anim[i].nsteps = TakeU16(&r);
            sPack.anim[i].steps = malloc(4 * (sPack.anim[i].nsteps ? sPack.anim[i].nsteps : 1));
            for (j = 0; j < sPack.anim[i].nsteps && !r.bad; j++)
            {
                sPack.anim[i].steps[j][0] = TakeU16(&r);
                sPack.anim[i].steps[j][1] = TakeU16(&r);
                if (sPack.anim[i].steps[j][0] >= sPack.ntex)
                    sPack.anim[i].steps[j][0] = sPack.anim[i].tex;
                sPack.anim[i].cycle += sPack.anim[i].steps[j][1];
            }
            if (sPack.anim[i].tex >= sPack.ntex)
                sPack.anim[i].nsteps = 0;
        }
    }
    sPack.meshShown = malloc(sizeof(u16) * (sPack.nmesh ? sPack.nmesh : 1));
    for (i = 0; i < sPack.nmesh; i++)
        sPack.meshShown[i] = i;
    if (version >= 6)
    {
        sPack.nmoving = TakeU32(&r);
        sPack.moving = calloc(sPack.nmoving ? sPack.nmoving : 1, sizeof(*sPack.moving));
        for (i = 0; i < sPack.nmoving && !r.bad; i++)
        {
            sPack.moving[i].mesh = TakeU16(&r);
            sPack.moving[i].nframes = TakeU16(&r);
            sPack.moving[i].step = TakeU16(&r);
            sPack.moving[i].frames = malloc(2 * (sPack.moving[i].nframes ? sPack.moving[i].nframes : 1));
            for (j = 0; j < sPack.moving[i].nframes && !r.bad; j++)
            {
                sPack.moving[i].frames[j] = TakeU16(&r);
                if (sPack.moving[i].frames[j] >= sPack.nmesh)
                    sPack.moving[i].frames[j] = sPack.moving[i].mesh;
            }
            if (sPack.moving[i].mesh >= sPack.nmesh || sPack.moving[i].step == 0)
                sPack.moving[i].nframes = 0;
        }
    }
    sPack.doorOf = calloc(sPack.nmesh ? sPack.nmesh : 1, sizeof(u16));
    if (version >= 7)
    {
        sPack.ndoor = TakeU32(&r);
        sPack.door = calloc(sPack.ndoor ? sPack.ndoor : 1, sizeof(*sPack.door));
        for (i = 0; i < sPack.ndoor && !r.bad; i++)
        {
            sPack.door[i].mesh = TakeU16(&r);
            Take(&r, &sPack.door[i].sound, 1);
            Take(&r, &sPack.door[i].nanim, 1);
            for (k = 0; k < sPack.door[i].nanim && !r.bad; k++)
            {
                u16 n = TakeU16(&r), step = TakeU16(&r);
                u16 *frames = malloc(2 * (n ? n : 1));
                for (j = 0; j < n && !r.bad; j++)
                {
                    frames[j] = TakeU16(&r);
                    if (frames[j] >= sPack.nmesh)
                        frames[j] = sPack.door[i].mesh;
                }
                if (k < 4)
                {
                    sPack.door[i].a[k].nframes = step ? n : 0;
                    sPack.door[i].a[k].step = step;
                    sPack.door[i].a[k].frames = frames;
                }
                else
                    free(frames);
            }
            if (sPack.door[i].nanim > 4)
                sPack.door[i].nanim = 4;
            if (sPack.door[i].mesh < sPack.nmesh && (sPack.door[i].nanim == 2 || sPack.door[i].nanim == 4))
                sPack.doorOf[sPack.door[i].mesh] = i + 1;
        }
    }
    free(data);
    if (r.bad)
    {
        printf("[s3d] %s is cut short\n", path);
        return;
    }
    sPack.ok = TRUE;
    printf("[s3d] %s: %u textures, %u meshes, %u matrices\n", path, sPack.ntex, sPack.nmesh, sPack.nmat);
}

static const S3dMatrix *MatrixNumbered(u16 number)
{
    u32 i;
    for (i = 0; sPack.ok && i < sPack.nmat; i++)
        if (sPack.mat[i].number == number)
            return &sPack.mat[i];
    return NULL;
}

static const S3dCell *CellIn(const S3dMatrix *mt, s32 cx, s32 cz)
{
    if (mt == NULL)
        return NULL;
    cx -= mt->x0;
    cz -= mt->z0;
    if (cx < 0 || cz < 0 || cx >= mt->width || cz >= mt->height)
        return NULL;
    return &mt->cells[cz * mt->width + cx];
}

static const S3dMatrix *sCurMat;   // the matrix of the map the player is on
static s32 sCurMap;                // (its sMaps index)

static const S3dCell *Cell(s32 cx, s32 cz)
{
    return CellIn(sCurMat, cx, cz);
}

// ------------------------------------------------------------------------------------------------ the ground's height
// A tile's ground height (world tile coordinates, 16 units a tile): of its heights, the one nearest `near` (an object
// on a bridge stays on it, one under it stays under); `near` itself where the block has none
static float TileHeight(s32 gtx, s32 gtz, float near)
{
    s32 cx = gtx >= 0 ? gtx / 32 : -((31 - gtx) / 32), cz = gtz >= 0 ? gtz / 32 : -((31 - gtz) / 32);
    const S3dCell *c = Cell(cx, cz);
    u32 t, i, best = 0;
    if (c == NULL)
        return near;
    t = (gtz - cz * 32) * 32 + (gtx - cx * 32);
    if (gS3dHeightHook != NULL)
    {
        // (a gym that moves: its own heights where it has them; NaN: the block's)
        float h = gS3dHeightHook(gtx - sMaps[sCurMap].cx * 32, gtz - sMaps[sCurMap].cz * 32, near);
        if (h == h)
            return h;
    }
    if (c->hn[t] == 0)
        return near;
    for (i = 1; i < c->hn[t] && i < 3; i++)
        if (fabsf(c->h[t][i] - near) < fabsf(c->h[t][best] - near))
            best = i;
    return c->h[t][best];
}

// The ground's height under a world point: the tile centres around it, blended (slopes and stairs as smooth as the
// DS shows them while someone walks)
static float GroundAt(float wx, float wz, float near)
{
    float fx = (wx - 8.0f) / 16.0f, fz = (wz - 8.0f) / 16.0f;
    s32 tx = (s32)floorf(fx), tz = (s32)floorf(fz);
    float ax = fx - tx, az = fz - tz;
    float h00 = TileHeight(tx, tz, near), h10 = TileHeight(tx + 1, tz, near);
    float h01 = TileHeight(tx, tz + 1, near), h11 = TileHeight(tx + 1, tz + 1, near);
    return (h00 * (1 - ax) + h10 * ax) * (1 - az) + (h01 * (1 - ax) + h11 * ax) * az;
}

// Tests (mp s3dground): the ground's height at a point of the current SINNOH map, in its pixels (16 a tile); NaN when
// it isn't a 3D map
float S3D_TestGround(s32 mapX, s32 mapY, float near);

static s32 MapIndex(u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMaps); i++)
        if (sMaps[i].map == ((group << 8) | num))
            return i;
    return -1;
}

bool8 S3D_IsSinnohGroup(u8 mapGroup)
{
    // group 40 (outdoors) and the buildings' groups after it (tools/sinnoh/gen_maps.py)
    return mapGroup >= MAP_GROUP(MAP_SINNOH_TWINLEAF_TOWN) && mapGroup < MAP_GROUP(MAP_SINNOH_TWINLEAF_TOWN) + SINNOH_GROUPS;
}

// Platinum's towns and routes have night versions of their songs (20:00 to 4:00), picked as the map's music starts
u16 S3D_MapMusic(u16 song)
{
    u32 i;
    s32 hour;
    RtcCalcLocalTime();
    hour = getenv("MOEMON_S3D_HOUR") ? atoi(getenv("MOEMON_S3D_HOUR")) : gLocalTime.hours;   // (tests: a fixed hour)
    if (hour >= 4 && hour < 20)
        return song;
    for (i = 0; i < ARRAY_COUNT(sSinnohNightSongs); i++)
        if (sSinnohNightSongs[i][0] == song)
            return sSinnohNightSongs[i][1];
    return song;
}

// ------------------------------------------------------------------------------------------------ map blocks
const u16 *S3D_LayoutBlocks(const struct MapLayout *layout)
{
    FlagSet(FLAG_SINNOH_HIDDEN);   // (Platinum's objects that aren't people here stay hidden: tools/sinnoh/gen_maps.py)
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
                    const S3dCell *c = CellIn(MatrixNumbered(sMaps[i].mat), sMaps[i].cx + bx, sMaps[i].cz + bz);
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
    float tx, ty, tz;        // the target (the player's tile centre, on the ground)
    float playerH;           // the player's ground height last frame (which layer of a bridge they're on)
    bool8 havePlayerH;
    u8 frame;                // counts S3D_Update calls
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
    u8 cam;
    float p = CAM_PITCH * 3.14159265f / 180.0f;
    float sp = sinf(p), cp = cosf(p);
    // (a scene's camera, the game's own camera object (special SpawnCameraObject: Platinum's AddFreeCamera): the
    // view follows it until RemoveCameraObject)
    for (cam = 0; cam < OBJECT_EVENTS_COUNT; cam++)
        if (gObjectEvents[cam].active && gObjectEvents[cam].localId == OBJ_EVENT_ID_CAMERA)
        {
            s = &gSprites[gObjectEvents[cam].spriteId];
            break;
        }
    // The player's tile centre: their sprite's middle, 8 pixels above its feet (the tile's bottom edge)
    sView.ax = s->x + s->x2 + gSpriteCoordOffsetX;
    sView.ay = s->y + s->y2 + gSpriteCoordOffsetY - s->centerToCornerVecY - 8;
    sView.originX = sMaps[sView.map].cx * BLOCK;
    sView.originZ = sMaps[sView.map].cz * BLOCK;
    sView.tx = sView.originX + sView.camX + sView.ax;
    sView.tz = sView.originZ + sView.camY + sView.ay;
    // the target is on the ground (BDHC), on the layer the player was on (a bridge, or under it)
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
    {
        // the DISTORTION WORLD: Platinum's camera angles there (pitch and yaw around the target: Camera_
        // AdjustPositionAroundTarget), and up on a wall or a ceiling the player's 3D position as the target
        float pitch, yaw, t[3];
        u8 dw = S3D_Dw_View(&pitch, &yaw, t);
        if (dw)
        {
            float pr = pitch * 3.14159265f / 180.0f, yr = yaw * 3.14159265f / 180.0f, len;
            if (dw == 2)
                sView.tx = t[0], sView.ty = t[1], sView.tz = t[2];
            sView.eye[0] = sView.tx + CAM_DIST * sinf(yr) * cosf(pr);
            sView.eye[1] = sView.ty + CAM_DIST * sinf(-pr);
            sView.eye[2] = sView.tz + CAM_DIST * cosf(yr) * cosf(pr);
            sView.fwd[0] = sView.tx - sView.eye[0], sView.fwd[1] = sView.ty - sView.eye[1], sView.fwd[2] = sView.tz - sView.eye[2];
            len = sqrtf(sView.fwd[0] * sView.fwd[0] + sView.fwd[1] * sView.fwd[1] + sView.fwd[2] * sView.fwd[2]);
            sView.fwd[0] /= len, sView.fwd[1] /= len, sView.fwd[2] /= len;
            // right = fwd x world up, up = right x fwd
            sView.right[0] = -sView.fwd[2], sView.right[1] = 0.0f, sView.right[2] = sView.fwd[0];
            len = sqrtf(sView.right[0] * sView.right[0] + sView.right[2] * sView.right[2]);
            sView.right[0] /= len, sView.right[2] /= len;
            sView.up[0] = sView.right[1] * sView.fwd[2] - sView.right[2] * sView.fwd[1];
            sView.up[1] = sView.right[2] * sView.fwd[0] - sView.right[0] * sView.fwd[2];
            sView.up[2] = sView.right[0] * sView.fwd[1] - sView.right[1] * sView.fwd[0];
        }
    }
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

// Each sprite's ground height last frame (the bridge layer it's on), NaN when it has none yet
static float sSpriteH[MAX_SPRITES];
static u8 sSpriteHFrame[MAX_SPRITES];

bool8 S3D_ProjectWorld(float x, float y, float z, float *sx, float *sy, float *depth)
{
    return sView.active && Project(x, y, z, sx, sy, depth);
}

float S3D_PlayerGround(void)
{
    return sView.ty;
}

void S3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth)
{
    float sx, sy, d, h, near = sView.ty;
    float wx = sView.originX + sView.camX + x, wz = sView.originZ + sView.camY + feetY - 8;
    if (!sView.active)
    {
        *outX = x;
        *outY = feetY;
        *depth = 0.0f;
        return;
    }
    if (S3D_Dw_PlayerSprite(spriteId, outX, outY, depth))
        return;   // (up on a DISTORTION WORLD wall, a ceiling, or mid-jump: src/sinnoh/s3d_distortion.c)
    // (a sprite not placed in the last few frames starts from the player's layer)
    if (spriteId < MAX_SPRITES && (u8)(sView.frame - sSpriteHFrame[spriteId]) <= 2)
        near = sSpriteH[spriteId];
    h = GroundAt(wx, wz, near);
    if (gS3dObjectHeightHook != NULL && spriteId < MAX_SPRITES)
    {
        // (a gym's people on its own floors: by who they are)
        u32 i;
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
            if (gObjectEvents[i].active && gObjectEvents[i].spriteId == spriteId && !gObjectEvents[i].isPlayer)
            {
                float oh = gS3dObjectHeightHook(gObjectEvents[i].localId, h);
                if (oh == oh)
                    h = oh;
                break;
            }
    }
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

// ------------------------------------------------------------------------------------------------ sprites in the scene
// A sprite is an upright card standing at its spot: a pixel dy above its feet is dy / cos(pitch) units up the card,
// so dy * tan(pitch) nearer the camera (about one unit a pixel at the target)
float gS3dSpriteDepth[MAX_SPRITES];
s16 gS3dSpriteFeet[MAX_SPRITES];
typedef struct { float d0; s16 feet; u8 flat, refl; } OamDepth;   // flat: a shadow lying on the ground (mp_shadow.c);
                                                                // refl: a reflection, seen only over water
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
        sPendingOam[i].flat = on && gSprites[spriteId].mpShadow;
        sPendingOam[i].refl = on && FieldEffect_IsReflectionSprite(&gSprites[spriteId]);
    }
}

void S3D_CommitOam(void)
{
    memcpy(sLiveOam, sPendingOam, sizeof(sLiveOam));
    sLiveAny = sView.active;
    if (sView.active)
        memset(sPendingOam, 0, sizeof(sPendingOam));
}

// A reflection pixel shows only where the ground under it is water (or ice): the pixel's ray from the eye meets the
// ground's level (the player's) on a map cell whose behavior reflects. Emerald's land tiles hide the rest with their top
// layer; here, without it, a reflection showed on the grass about a tile before the water
static bool8 OverReflectiveCell(int x, int line)
{
    float rx = (x - sView.ax) / sView.focal, ry = (sView.ay - line) / sView.focal;
    float d[3], t, wx, wz;
    s32 cx, cy;
    u32 i, b;
    for (i = 0; i < 3; i++)
        d[i] = sView.fwd[i] + sView.right[i] * rx + sView.up[i] * ry;
    if (d[1] > -1e-4f)
        return TRUE;
    t = (sView.ty - sView.eye[1]) / d[1];
    wx = sView.eye[0] + d[0] * t - sView.originX;
    wz = sView.eye[2] + d[2] * t - sView.originZ;
    cx = (s32)floorf(wx / 16.0f);
    cy = (s32)floorf(wz / 16.0f);
    b = MapGridGetMetatileBehaviorAt(cx + MAP_OFFSET, cy + MAP_OFFSET);
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
    if (o->refl && !OverReflectiveCell(x, line))
        return TRUE;
    if (o->flat)   // (a shadow lies on the ground: all of it as far as its middle, and only a wall in front hides it;
        return sDepth[line][x] < o->d0 - 16.0f;   //  1.63.9: the ground around hid parts of it as an upright picture's)
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
    const S3dTex *t = g->tex >= 0 && (u32)g->tex < sPack.ntex ? &sPack.tex[sPack.shown[g->tex]] : NULL;
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
            u32 ch, ta = 0xFF;
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
                ta = texel >> 24;
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
            else if (ta != 0xFF)
            {
                // a see-through texel of a solid polygon (a shadow's): what's behind it stays drawable
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
// translucent ones (see-through polygons, or textures with part see-through texels, as the DS sorts them)
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
        bool8 soft = g->alpha < 31 || (g->tex >= 0 && (u32)g->tex < sPack.ntex && sPack.tex[sPack.shown[g->tex]].soft);
        if (soft != pass)
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

// Platinum's animated map textures: the frame each one shows now (its field runs at 30 frames a second: one of its
// frames is two of ours)
static u32 sTexClock;
static void AnimateTextures(void)
{
    u32 i, j, t;
    sTexClock++;
    for (i = 0; i < sPack.nanim; i++)
    {
        if (sPack.anim[i].nsteps == 0 || sPack.anim[i].cycle == 0)
            continue;
        t = (sTexClock / 2) % sPack.anim[i].cycle;
        for (j = 0; j + 1 < sPack.anim[i].nsteps && t >= sPack.anim[i].steps[j][1]; j++)
            t -= sPack.anim[i].steps[j][1];
        sPack.shown[sPack.anim[i].tex] = sPack.anim[i].steps[j][0];
    }
    for (i = 0; i < sPack.nmoving; i++)   // (the props that move: their frame now)
        if (sPack.moving[i].nframes)
            sPack.meshShown[sPack.moving[i].mesh]
                = sPack.moving[i].frames[(sTexClock / 2 / sPack.moving[i].step) % sPack.moving[i].nframes];
}

// `mp s3dtex`: how many textures are animated, and what the first shows (tests)
void S3D_TexAnimInfo(u32 *count, u32 *firstShown)
{
    printf("[test] s3d moving props %u\n", (unsigned)sPack.nmoving);
    *count = sPack.nanim;
    *firstShown = sPack.nanim ? sPack.shown[sPack.anim[0].tex] : 0;
}

// A platform lift (s3d_lift.c): the map's props standing in a rectangle of the matrix world drawn at height Y instead
static struct { bool8 on; float x0, z0, x1, z1, y; } sLift;

void S3D_SetLift(bool8 on, float x0, float z0, float x1, float z1, float y)
{
    sLift.on = on, sLift.x0 = x0, sLift.z0 = z0, sLift.x1 = x1, sLift.z1 = z1, sLift.y = y;
}

// ------------------------------------------------------------------------------------------------ Platinum's doors
// A door prop's animation as the player walks through (ov5_021D431C.c): the field's door calls (field_door.c, as
// HOENN's door tiles) find the door prop on or beside the warp tile and play one of its animations; the last frame
// stays (the door left open) until the next one, and a closing one ends with the door as it stands
static struct { const S3dProp *prop; u16 door; u8 anim; bool8 hold; u16 t; s32 map; } sDoorAnim;

static u16 DoorAnimLength(u16 door, u8 anim)
{
    return sPack.door[door].a[anim].nframes * sPack.door[door].a[anim].step * 2;   // (Platinum's 30 a second)
}

static u16 DoorMesh(const S3dProp *pr)
{
    u16 n, i;
    if (sDoorAnim.prop != pr || sDoorAnim.map != sCurMap)
        return pr->mesh;
    n = sPack.door[sDoorAnim.door].a[sDoorAnim.anim].nframes;
    if (n == 0)
        return pr->mesh;
    i = sDoorAnim.t / (sPack.door[sDoorAnim.door].a[sDoorAnim.anim].step * 2);
    return sPack.door[sDoorAnim.door].a[sDoorAnim.anim].frames[i < n ? i : n - 1];
}

static void Task_S3dDoor(u8 taskId)
{
    if (sDoorAnim.prop == NULL)
    {
        DestroyTask(taskId);
        return;
    }
    if (++sDoorAnim.t >= DoorAnimLength(sDoorAnim.door, sDoorAnim.anim))
    {
        sDoorAnim.t = DoorAnimLength(sDoorAnim.door, sDoorAnim.anim);
        if (!sDoorAnim.hold)
            sDoorAnim.prop = NULL;   // (closed: the prop as it stands)
        DestroyTask(taskId);
    }
}

// The door prop on map tile (X, Y) of the current SINNOH map (the field's coordinates, MAP_OFFSET in), or beside it
static const S3dProp *DoorAt(s32 x, s32 y, u16 *door)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum), gtx, gtz, cx, cz, dx;
    const S3dProp *best = NULL;
    float bestD = 99;
    if (!sPack.tried)
        LoadPack();
    if (map < 0 || !sPack.ok || sPack.ndoor == 0)
        return NULL;
    gtx = sMaps[map].cx * 32 + x - MAP_OFFSET, gtz = sMaps[map].cz * 32 + y - MAP_OFFSET;
    for (dx = -1; dx <= 1; dx++)
    {
        const S3dCell *c = CellIn(MatrixNumbered(sMaps[map].mat), (gtx + dx) / 32, gtz / 32);
        u32 i;
        if (dx != 0 && (gtx + dx) / 32 == gtx / 32)
            continue;   // (each block once)
        cx = (gtx + dx) / 32, cz = gtz / 32;
        for (i = 0; c != NULL && i < c->nprops; i++)
        {
            const S3dProp *pr = &c->props[i];
            float px, pz, d;
            if (pr->mesh < 0 || (u32)pr->mesh >= sPack.nmesh || sPack.doorOf[pr->mesh] == 0)
                continue;
            px = (cx * BLOCK + BLOCK / 2 + pr->m[3]) / 16.0f, pz = (cz * BLOCK + BLOCK / 2 + pr->m[11]) / 16.0f;
            if ((s32)floorf(pz) != gtz || abs((s32)floorf(px) - gtx) > 1)
                continue;
            d = fabsf(px - (gtx + 0.5f));
            if (d < bestD)
                bestD = d, best = pr, *door = sPack.doorOf[pr->mesh] - 1;
        }
    }
    return best;
}

// Play the door's animation at (X, Y): KIND 0 opening and 1 closing as the player walks in, 2 / 3 walking out. A task
// that ends with it (-1: no door prop there, or not on a SINNOH map)
s8 S3D_DoorAnimate(s32 x, s32 y, u8 kind)
{
    u16 door = 0;
    const S3dProp *pr = DoorAt(x, y, &door);
    u8 taskId;
    if (pr == NULL)
        return -1;
    if (sPack.door[door].nanim == 2)
        kind &= 1;   // (two: the same open and close both ways)
    if (DoorAnimLength(door, kind) == 0)
        return -1;
    sDoorAnim.prop = pr, sDoorAnim.door = door, sDoorAnim.anim = kind, sDoorAnim.t = 0, sDoorAnim.map = sCurMap;
    sDoorAnim.hold = !(kind & 1);
    while ((taskId = FindTaskIdByFunc(Task_S3dDoor)) != TASK_NONE)
        DestroyTask(taskId);
    printf("[s3d] door %u at %d,%d: animation %u\n", door, (int)x - MAP_OFFSET, (int)y - MAP_OFFSET, kind);
    return CreateTask(Task_S3dDoor, 80);
}

// The door's sound (field_door.c GetDoorSoundEffect): 0 hinged, 1 sliding, 2 the DEPT. STORE's chime; -1 none there
s8 S3D_DoorSound(s32 x, s32 y)
{
    u16 door = 0;
    return DoorAt(x, y, &door) != NULL ? sPack.door[door].sound : -1;
}

// Closing: after walking out (the door was opened for that: its own closing of four), else after walking in
s8 S3D_DoorClose(s32 x, s32 y)
{
    u16 door = 0;
    const S3dProp *pr = DoorAt(x, y, &door);
    return S3D_DoorAnimate(x, y, pr != NULL && sDoorAnim.prop == pr && sDoorAnim.anim == 2 ? 3 : 1);
}

bool8 S3D_DoorRunning(void)
{
    return FindTaskIdByFunc(Task_S3dDoor) != TASK_NONE;
}

// A locked door: a door tile with no warp on it (Platinum's scripts move a door's warp away until it opens, as at the
// VALLEY WINDWORKS before the WORKS KEY) is a wall, so the player stands in front of it and its sign is the tile they
// face; walked onto, the sign was under their feet and A did nothing. (X, Y: the field's coordinates, MAP_OFFSET in)
bool8 S3D_DoorLocked(s16 x, s16 y)
{
    const struct MapEvents *ev = gMapHeader.events;
    u8 i;
    if (!S3D_Active() || ev == NULL || !MetatileBehavior_IsWarpDoor(MapGridGetMetatileBehaviorAt(x, y)))
        return FALSE;
    for (i = 0; i < ev->warpCount; i++)
        if (ev->warps[i].x == x - MAP_OFFSET && ev->warps[i].y == y - MAP_OFFSET)
            return FALSE;
    return TRUE;
}

static void DrawProp(const S3dProp *pr, float ox, float oy, float oz, u32 pass)
{
    const float *m = pr->m;
    float moved[12];
    if (sLift.on && ox + m[3] >= sLift.x0 && ox + m[3] < sLift.x1 && oz + m[11] >= sLift.z0 && oz + m[11] < sLift.z1)
    {
        memcpy(moved, m, sizeof(moved));
        moved[7] = sLift.y - oy;
        m = moved;
    }
    DrawMesh(&sPack.mesh[sDoorAnim.prop == pr ? DoorMesh(pr) : sPack.meshShown[pr->mesh]], m, ox, oy, oz, pass);
}

static void Render(void)
{
    AnimateTextures();
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
                float ox = cx * BLOCK + BLOCK / 2, oy = c ? c->alt * 8.0f : 0.0f, oz = cz * BLOCK + BLOCK / 2;
                if (c == NULL || c->mesh < 0 || (u32)c->mesh >= sPack.nmesh)
                    continue;
                DrawMesh(&sPack.mesh[c->mesh], NULL, ox, oy, oz, pass);
                for (i = 0; i < c->nprops; i++)
                    if (c->props[i].mesh >= 0 && (u32)c->props[i].mesh < sPack.nmesh)
                        DrawProp(&c->props[i], ox, oy, oz, pass);
            }
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < S3D_DYN_PROPS; i++)
            if (sDyn[i].on)
                DrawMesh(&sPack.mesh[sDyn[i].mesh], sDyn[i].m, 0, 0, 0, pass);
}

// Moemon Maker: the 3D world of MAP around tile (TILEX, TILEY) as the game's camera sees it with the player there, into
// OUT (240 x 160, the game's colors), for the editor's 3D PREVIEW (src/maker/maker_tiles.c). Only while the editor
// is open and the game isn't playing: the view's own state is put back after. FALSE without the pack or the map.
bool8 S3D_RenderPreview(u8 group, u8 num, s32 tileX, s32 tileY, u16 *out)
{
    static u8 saved[sizeof(sView)];
    const S3dMatrix *savedMat = sCurMat;
    s32 savedMap = sCurMap, map = MapIndex(group, num);
    float p = CAM_PITCH * 3.14159265f / 180.0f, sp = sinf(p), cp = cosf(p);
    if (map < 0)
        return FALSE;
    if (!sPack.tried)
        LoadPack();
    if (!sPack.ok)
        return FALSE;
    memcpy(saved, &sView, sizeof(sView));
    sView.map = map;
    sCurMat = MatrixNumbered(sMaps[map].mat);
    sCurMap = map;
    // (as SetupView, the target on the tile's middle instead of the player's)
    sView.camX = (tileX - 7) * 16;
    sView.camY = (tileY - 4) * 16 - 8;
    sView.ax = 7 * 16 + 8;
    sView.ay = 4 * 16 + 16;
    sView.originX = sMaps[map].cx * BLOCK;
    sView.originZ = sMaps[map].cz * BLOCK;
    sView.tx = sView.originX + tileX * 16 + 8;
    sView.tz = sView.originZ + tileY * 16 + 8;
    sView.ty = GroundAt(sView.tx, sView.tz, 0.0f);
    sView.eye[0] = sView.tx;
    sView.eye[1] = sView.ty + CAM_DIST * sp;
    sView.eye[2] = sView.tz + CAM_DIST * cp;
    sView.fwd[0] = 0.0f, sView.fwd[1] = -sp, sView.fwd[2] = -cp;
    sView.right[0] = 1.0f, sView.right[1] = 0.0f, sView.right[2] = 0.0f;
    sView.up[0] = 0.0f, sView.up[1] = cp, sView.up[2] = -sp;
    sView.focal = 96.0f / tanf(HALF_FOV * 3.14159265f / 180.0f);
    Render();
    memcpy(out, sColor, sizeof(sColor));
    memcpy(&sView, saved, sizeof(sView));
    sCurMat = savedMat;
    sCurMap = savedMap;
    return TRUE;
}

// ------------------------------------------------------------------------------------------------ run-time props
// A prop of Platinum's model MODEL in slot SLOT at (X, Y, Z) of the current map's matrix world (Platinum's units: 16 a
// tile, a block 512 from its matrix's corner), turned TURN degrees about the up axis (as Platinum's): FALSE when the
// pack hasn't the model (an older pack: the gym stays as it was)
bool8 S3D_DynProp(u8 slot, u16 model, float x, float y, float z, float turn)
{
    return S3D_DynPropTurned(slot, model, x, y, z, 0, turn);
}

// The same turned TILT degrees about the east-west axis first (an upright gear's), then TURN about the up axis
bool8 S3D_DynPropTurned(u8 slot, u16 model, float x, float y, float z, float tilt, float turn)
{
    u32 i;
    float a = turn * 3.14159265f / 180.0f, cs = cosf(a), sn = sinf(a);
    float b = tilt * 3.14159265f / 180.0f, cx = cosf(b), sx = sinf(b);
    if (slot >= S3D_DYN_PROPS || !sPack.ok)
        return FALSE;
    for (i = 0; i < sPack.nnamed && sPack.named[i].model != model; i++)
        ;
    if (i == sPack.nnamed || sPack.named[i].mesh < 0 || (u32)sPack.named[i].mesh >= sPack.nmesh)
        return FALSE;
    sDyn[slot].on = TRUE;
    sDyn[slot].mesh = sPack.named[i].mesh;
    // (rows of Ry * Rx: a point is turned about x, then about y; Platinum's angles)
    sDyn[slot].m[0] = cs, sDyn[slot].m[1] = sn * sx, sDyn[slot].m[2] = sn * cx, sDyn[slot].m[3] = x;
    sDyn[slot].m[4] = 0, sDyn[slot].m[5] = cx, sDyn[slot].m[6] = -sx, sDyn[slot].m[7] = y;
    sDyn[slot].m[8] = -sn, sDyn[slot].m[9] = cs * sx, sDyn[slot].m[10] = cs * cx, sDyn[slot].m[11] = z;
    return TRUE;
}

void S3D_DynPropHide(u8 slot)
{
    if (slot < S3D_DYN_PROPS)
        sDyn[slot].on = FALSE;
}

void S3D_DynPropsClear(void)
{
    memset(sDyn, 0, sizeof(sDyn));
}

// Where the player is in the map's matrix world (the camera's target: smooth while walking)
void S3D_PlayerWorld(float *x, float *z)
{
    *x = sView.tx, *z = sView.tz;
}

// The ground height the player stands at (the last frame's; NaN: not known yet)
float S3D_PlayerHeight(void)
{
    return sView.havePlayerH ? sView.playerH : NAN;
}

// The block's ground height on a tile of the current map nearest NEAR (a bridge's two layers: the one an object at that
// height stands on; NaN: none there)
float S3D_TileFloor(s32 mapX, s32 mapY, float near)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum), gtx, gtz, cx, cz;
    const S3dCell *c;
    u32 t;
    if (map < 0 || !sPack.ok)
        return NAN;
    gtx = sMaps[map].cx * 32 + mapX, gtz = sMaps[map].cz * 32 + mapY;
    cx = gtx / 32, cz = gtz / 32;
    c = CellIn(MatrixNumbered(sMaps[map].mat), cx, cz);
    if (c == NULL || mapX < 0 || mapY < 0)
        return NAN;
    t = (gtz - cz * 32) * 32 + (gtx - cx * 32);
    if (c->hn[t] == 0)
        return NAN;
    {
        u32 i, best = 0;
        for (i = 1; i < c->hn[t] && i < 3; i++)
            if (fabsf(c->h[t][i] - near) < fabsf(c->h[t][best] - near))
                best = i;
        return c->h[t][best];
    }
}

// The current map's block (0, 0) corner in its matrix's world (for a gym's run-time props); FALSE: not a SINNOH map
// the current map's top-left block in Platinum's matrix (SINNOH's converted scripts: Platinum's map coordinates)
bool8 S3D_MapCells(s32 *cx, s32 *cz)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return FALSE;
    *cx = sMaps[map].cx, *cz = sMaps[map].cz;
    return TRUE;
}

// the current SINNOH map's matrix (0: the region) and the cell of its top-left block there (s3d_underground.c)
bool8 S3D_MapCell(u16 *mat, u16 *cx, u16 *cz)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return FALSE;
    *mat = sMaps[map].mat;
    *cx = sMaps[map].cx;
    *cz = sMaps[map].cz;
    return TRUE;
}

bool8 S3D_MapOrigin(float *x, float *z)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return FALSE;
    if (!sPack.tried)
        LoadPack();
    *x = sMaps[map].cx * BLOCK, *z = sMaps[map].cz * BLOCK;
    return sPack.ok;
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

// A dark room (HEARTHOME's gym): the 3D picture LEVEL as bright as it is, but for circles of light {x, y, radius} on
// the screen (the people's lamps: Platinum's fog and flashlights; s3d_gyms.c sets them every frame)
static struct { float level; u8 n; s16 lights[S3D_DARK_LIGHTS][3]; } sDark;

void S3D_SetDarkness(float level, const s16 (*lights)[3], u8 n)
{
    sDark.level = level;
    sDark.n = n > S3D_DARK_LIGHTS ? S3D_DARK_LIGHTS : n;
    if (n)
        memcpy(sDark.lights, lights, sDark.n * sizeof(sDark.lights[0]));
}

static inline u16 Darkened(u16 c, int x, int line)
{
    float lit = sDark.level;
    u32 i, ch, out = 0x8000;
    for (i = 0; i < sDark.n && lit < 1.0f; i++)
    {
        float dx = x - sDark.lights[i][0], dy = line - sDark.lights[i][1], r = sDark.lights[i][2];
        float d2 = dx * dx + dy * dy, f;
        if (d2 >= r * r)
            continue;
        f = 1.0f - sqrtf(d2) / r;   // (soft to the edge)
        f = f > 0.4f ? 1.0f : f / 0.4f;
        f = sDark.level + (1.0f - sDark.level) * f;
        lit = f > lit ? f : lit;
    }
    if (lit >= 1.0f)
        return c;
    for (ch = 0; ch < 3; ch++)
        out |= ((u32)(((c >> (5 * ch)) & 31) * lit + 0.5f)) << (5 * ch);
    return out;
}

// The field, also while a battle's opening plays (battle_transition.c runs on CB2_OverworldBasic: 1.63.8, the map
// was gone behind it)
static bool8 OnField(void)
{
    return gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic;
}

static void S3dLine(int line, u16 layers[4][DISPLAY_WIDTH]);

// A POKé MART's BUY screen (shop.c): the field's last 3D picture behind it, moved by (dx, dy) as the screen moves
// the map (1.63.8: Emerald's own map tiles showed there, garbled)
static struct { bool8 on; s16 dx, dy; } sShopPic;
void S3D_ShopPicture(bool8 on, s16 dx, s16 dy)
{
    sShopPic.on = on && sView.active;
    sShopPic.dx = dx, sShopPic.dy = dy;
    if (sShopPic.on)
        gMpS3dLine = S3dLine;
}

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

static void S3dLine(int line, u16 layers[4][DISPLAY_WIDTH])
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
        // MOEMON_S3D_PERF: the time a frame's 3D picture takes, averaged over 120 frames
        static int sPerf = -1, sCount;
        static unsigned long long sSum, sMax;
        unsigned long long t0 = 0;
        if (sPerf < 0)
            sPerf = getenv("MOEMON_S3D_PERF") != NULL;
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
                printf("[s3d] perf: %.2f ms a frame (worst %.2f)\n", sSum / f / sCount, sMax / f);
                sCount = 0;
                sSum = sMax = 0;
            }
        }
    }
    for (x = 0; x < W; x++)
    {
        layers[3][x] = Faded(sDark.level > 0 ? Darkened(sColor[line][x], x, line) : sColor[line][x]);
        layers[2][x] = 0;
        layers[1][x] = 0;
    }
}

// ------------------------------------------------------------------------------------------------ every frame
float S3D_TestGround(s32 mapX, s32 mapY, float near)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return NAN;
    if (!sPack.tried)
        LoadPack();
    if (!sPack.ok)
        return NAN;
    sCurMat = MatrixNumbered(sMaps[map].mat);
    sCurMap = map;
    return GroundAt(sMaps[map].cx * BLOCK + mapX, sMaps[map].cz * BLOCK + mapY, near);
}

bool8 S3D_Active(void)
{
    return sView.active && OnField();
}

void S3D_Update(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map >= 0 && !sPack.tried)
        LoadPack();
    sView.active = map >= 0 && sPack.ok;
    sShopPic.on = FALSE;   // (back on the field)
    gMpS3dLine = sView.active ? S3dLine : NULL;
    gMpS3dOamHidden = sView.active && !getenv("MOEMON_S3D_NOHIDE") ? OamHidden : NULL;
    if (!sView.active)
        return;
    if (sView.map != map)
        sView.havePlayerH = FALSE;   // (a new map: its own ground)
    sView.map = map;
    sCurMat = MatrixNumbered(sMaps[map].mat);
    sCurMap = map;
    sView.frame++;
    S3D_Gyms_Update();
    S3D_Dw_Update();
    FindCamera();
    SetupView();
    if (getenv("MOEMON_S3D_TRACE"))
    {
        const struct Sprite *sp = &gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId];
        s32 px, py;
        float pd;
        S3D_PlaceSprite(0xFFFF, sp->x + sp->x2 + gSpriteCoordOffsetX, sp->y + sp->y2 + gSpriteCoordOffsetY - sp->centerToCornerVecY, &px, &py, &pd);
        printf("[s3d] ground %.1f cam %d,%d anchor %.1f,%.1f target %.1f,%.1f sprite %d+%d+%d,%d+%d+%d c2c %d -> %d,%d\n", sView.ty, sView.camX,
               sView.camY, sView.ax, sView.ay, sView.tx, sView.tz, sp->x, sp->x2, gSpriteCoordOffsetX, sp->y, sp->y2,
               gSpriteCoordOffsetY, sp->centerToCornerVecY, px, py);
    }
}

// The player's cell on SINNOH's world matrix (the POKéTCH's MARKING MAP): outdoors where they stand, indoors the place
// they came in from (the escape warp's map); FALSE: neither is on the matrix
bool8 S3D_PlayerMatrixCell(s32 *cx, s32 *cz)
{
    s32 i = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (i >= 0 && sMaps[i].mat == 0)
    {
        *cx = sMaps[i].cx + (gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET) / 32;
        *cz = sMaps[i].cz + (gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET) / 32;
        return TRUE;
    }
    i = MapIndex(gSaveBlock1Ptr->escapeWarp.mapGroup, gSaveBlock1Ptr->escapeWarp.mapNum);
    if (i >= 0 && sMaps[i].mat == 0)
    {
        *cx = sMaps[i].cx + (gSaveBlock1Ptr->escapeWarp.x >= 0 ? gSaveBlock1Ptr->escapeWarp.x / 32 : 0);
        *cz = sMaps[i].cz + (gSaveBlock1Ptr->escapeWarp.y >= 0 ? gSaveBlock1Ptr->escapeWarp.y / 32 : 0);
        return TRUE;
    }
    return FALSE;
}
