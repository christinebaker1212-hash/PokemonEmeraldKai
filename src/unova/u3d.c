// Moemon Kai: UNOVA's 3D engine (docs/UNOVA_HANDOFF.md). An UNOVA map is a map of the field engine like any other (grid
// movement, scripts, warps, co-op), but what is shown of it is Black's own 3D world: its map blocks, their buildings
// and textures (each season's), drawn by this software renderer into the game's 240x160 picture each frame. The
// renderer (gba_easy_draw.c) hands each line of the field's BG layers to U3dLine, which puts the 3D picture in the
// bottom layer and clears the map's other layers; text windows and sprites go on top as on any map. Field sprites are
// moved to where their spot on the ground is in the 3D view (sprite.c -> U3D_PlaceSprite).
//
// Its own engine, never SINNOH's (src/sinnoh/s3d.c) or ORRE's (src/orre/o3d.c): its code started as a copy of
// SINNOH's renderer (the user allowed it, docs/UNOVA_DECISIONS.md) and goes its own way: Black's camera, Black's
// seasons, Black's collision grids.
//
// The world comes from the pack (unova/black3d.pak beside the game) the release builds from the user's Black ROM
// (tools/unova/build_pack.py; nothing of it is in the repository): meshes in Black's units (16 a tile, a block of
// 32x32 tiles is 512 wide, centred on its middle), textures as RGBA, and per matrix cell its block (a mesh a season),
// buildings and the field engine's blocks (collision and behaviors) that U3D_LayoutBlocks gives the map loader.
//
// The camera is Black's own field camera (its setting 0: aletheia's field_camera.c, a/0/6/0): 237 units from a point
// 4 above the player, 53.2 degrees down, at one unit a pixel there (16 pixels a tile, as Emerald's maps): 240x160
// shows about what the DS's 256x192 does (its field of view, 20 degrees each way up and down, is 18.6 here).
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "metatile_behavior.h"
#include "mp.h"
#include "overworld.h"
#include "palette.h"
#include "rtc.h"
#include "sprite.h"
#include "u3d.h"
#include "constants/event_objects.h"
#include "constants/maps.h"

bool8 FieldEffect_IsReflectionSprite(const struct Sprite *sprite);   // (field_effect_helpers.c)
const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
extern void (*gMpU3dLine)(int line, u16 layers[4][DISPLAY_WIDTH]);
extern bool (*gMpU3dOamHidden)(int oamIndex, int x, int line);

#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define BLOCK 512.0f
#define CAM_DIST 237.0f
#define CAM_PITCH 53.2177734375f        // (9688 of 65536 turns)
#define CAM_LIFT 4.0f                   // (the setting's target shift: 4 units up)
#define FEET_TOWARD 0.0f                // (Black stands a person's billboard on its tile's middle)
#define EMERALD_DROP 8.0f               // (the player's view a half tile lower: the player's feet where Emerald draws
                                        //  them, the tile's foot rather than its middle; scenes' cameras ease off it)
#define SEASONS 4

// ------------------------------------------------------------------------------------------------ the pack
typedef struct { float x, y, z, u, v; u8 r, g, b, pad; float nx, ny, nz; } U3dVtx;
typedef struct
{
    s16 tex;
    u8 rs, rt, fs, ft, alpha, pid, cull, lights, mode;
    u32 count;
    U3dVtx *v;
    float *lit;
    s32 litFor;
} U3dGroup;
typedef struct { float c[3], r; u32 ngroups; U3dGroup *g; } U3dMesh;
typedef struct { u16 w, h; u32 *px; bool8 soft; } U3dTex;
typedef struct { u8 on, r, g, b; float d[3]; } U3dLight;
typedef struct { u32 start; U3dLight l[4]; u8 col[4][3]; u8 clear[4]; } U3dLightBlock;
typedef struct { u32 count; U3dLightBlock *b; } U3dLightSet;
typedef struct { s16 mesh[SEASONS]; float m[12]; } U3dProp;
typedef struct
{
    u16 zone;
    s16 mesh[SEASONS];
    u16 light, nprops;
    U3dProp *props;
    u16 blocks[32 * 32];
    u8 hn[32 * 32];
    float h[32 * 32][2];
    u16 (*cam)[3];   // (a rail map's cells: Black's camera of each tile's rail, pitch, yaw, distance; NULL none)
} U3dCell;
typedef struct { u16 number, width, height, x0, z0, scale; U3dCell *cells; } U3dMatrix;   // (scale: a block is scale x scale
                                                                                              // cells, its meshes in the first)

static struct
{
    bool8 tried, ok;
    u32 ntex, nmesh, nlight, nmat;
    U3dTex *tex;
    U3dMesh *mesh;
    U3dLightSet *light;
    U3dMatrix *mat;
} sPack;

#include "u3d_maps.h"   // sUnovaMaps: each UNOVA map, its zone, where it lies on its matrix (tools/unova/gen_maps.py)

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
    const char *path = getenv("MOEMON_U3D_PAK");
    FILE *f;
    long size;
    u8 *data;
    Reader r;
    u32 i, j, k, version;
    sPack.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black3d.pak");   // (beside the game, as music/)
    f = fopen(path, "rb");
    if (f == NULL)
    {
        printf("[u3d] no UNOVA pack (%s): tools/unova/build_pack.py makes it from Black's ROM\n", path);
        return;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size)
    {
        fclose(f);
        free(data);
        return;
    }
    fclose(f);
    if (size >= 8 && memcmp(data, "U3DZ", 4) == 0)
    {
        unsigned long rawSize = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
        u8 *raw = malloc(rawSize ? rawSize : 1);
        if (raw == NULL || uncompress(raw, &rawSize, data + 8, (unsigned long)(size - 8)) != 0)
        {
            printf("[u3d] %s: can't inflate the pack\n", path);
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
    if (memcmp(data, "U3DP", 4) != 0)
    {
        printf("[u3d] %s isn't an UNOVA pack\n", path);
        free(data);
        return;
    }
    r.p += 4;
    version = TakeU32(&r);
    if (version < 1 || version > 3)
    {
        printf("[u3d] %s: another pack version (%u)\n", path, (unsigned)version);
        free(data);
        return;
    }
    sPack.ntex = TakeU32(&r);
    sPack.tex = calloc(sPack.ntex ? sPack.ntex : 1, sizeof(U3dTex));
    for (i = 0; i < sPack.ntex && !r.bad; i++)
    {
        u32 n;
        sPack.tex[i].w = TakeU16(&r);
        sPack.tex[i].h = TakeU16(&r);
        n = (u32)sPack.tex[i].w * sPack.tex[i].h;
        sPack.tex[i].px = malloc(4 * (n ? n : 1));
        Take(&r, sPack.tex[i].px, 4 * n);
        for (k = 0; k < n && !sPack.tex[i].soft; k++)
            if ((sPack.tex[i].px[k] >> 24) != 0 && (sPack.tex[i].px[k] >> 24) != 0xFF)
                sPack.tex[i].soft = TRUE;
    }
    sPack.nmesh = TakeU32(&r);
    sPack.mesh = calloc(sPack.nmesh ? sPack.nmesh : 1, sizeof(U3dMesh));
    for (i = 0; i < sPack.nmesh && !r.bad; i++)
    {
        U3dMesh *m = &sPack.mesh[i];
        Take(&r, m->c, 12);
        Take(&r, &m->r, 4);
        m->ngroups = TakeU32(&r);
        m->g = calloc(m->ngroups ? m->ngroups : 1, sizeof(U3dGroup));
        for (j = 0; j < m->ngroups && !r.bad; j++)
        {
            U3dGroup *g = &m->g[j];
            u8 b[10];
            Take(&r, &g->tex, 2);
            Take(&r, b, 10);
            g->rs = b[0], g->rt = b[1], g->fs = b[2], g->ft = b[3], g->alpha = b[4], g->pid = b[5], g->cull = b[6];
            g->lights = b[7], g->mode = b[8];
            g->count = TakeU32(&r) * 3;
            g->v = malloc(sizeof(U3dVtx) * (g->count ? g->count : 1));
            for (k = 0; k < g->count; k++)
                Take(&r, &g->v[k], 36);
            g->lit = malloc(sizeof(float) * 3 * (g->count ? g->count : 1));
            g->litFor = -1;
        }
    }
    sPack.nlight = TakeU32(&r);
    sPack.light = calloc(sPack.nlight ? sPack.nlight : 1, sizeof(U3dLightSet));
    for (i = 0; i < sPack.nlight && !r.bad; i++)
    {
        U3dLightSet *s = &sPack.light[i];
        s->count = TakeU32(&r);
        s->b = calloc(s->count ? s->count : 1, sizeof(U3dLightBlock));
        for (j = 0; j < s->count && !r.bad; j++)
        {
            U3dLightBlock *b = &s->b[j];
            b->start = TakeU32(&r);
            for (k = 0; k < 4; k++)
            {
                Take(&r, &b->l[k].on, 4);
                Take(&r, b->l[k].d, 12);
            }
            Take(&r, b->col, 12);
            Take(&r, b->clear, 4);
        }
    }
    sPack.nmat = TakeU32(&r);
    sPack.mat = calloc(sPack.nmat ? sPack.nmat : 1, sizeof(U3dMatrix));
    for (k = 0; k < sPack.nmat && !r.bad; k++)
    {
        U3dMatrix *mt = &sPack.mat[k];
        mt->number = TakeU16(&r);
        mt->width = TakeU16(&r);
        mt->height = TakeU16(&r);
        mt->x0 = TakeU16(&r);
        mt->z0 = TakeU16(&r);
        mt->scale = version >= 2 ? TakeU16(&r) : 1;
        if (mt->scale == 0)
            mt->scale = 1;
        mt->cells = calloc((u32)mt->width * mt->height ? (u32)mt->width * mt->height : 1, sizeof(U3dCell));
        for (i = 0; i < (u32)mt->width * mt->height && !r.bad; i++)
        {
            U3dCell *c = &mt->cells[i];
            c->zone = TakeU16(&r);
            Take(&r, c->mesh, 8);
            c->light = TakeU16(&r);
            c->nprops = TakeU16(&r);
            c->props = calloc(c->nprops ? c->nprops : 1, sizeof(U3dProp));
            for (j = 0; j < c->nprops; j++)
            {
                Take(&r, c->props[j].mesh, 8);
                Take(&r, c->props[j].m, 48);
            }
            Take(&r, c->blocks, sizeof(c->blocks));
            Take(&r, c->hn, sizeof(c->hn));
            Take(&r, c->h, sizeof(c->h));
            if (version >= 3)
            {
                u8 hasCam = 0;
                Take(&r, &hasCam, 1);
                if (hasCam && (c->cam = malloc(sizeof(u16) * 3 * 1024)) != NULL)
                    Take(&r, c->cam, sizeof(u16) * 3 * 1024);
                else if (hasCam)
                    r.bad = TRUE;
            }
        }
    }
    if (r.bad)
    {
        printf("[u3d] %s is cut short\n", path);
        return;
    }
    sPack.ok = TRUE;
    printf("[u3d] UNOVA's pack: %u textures, %u meshes, %u matrices\n", (unsigned)sPack.ntex, (unsigned)sPack.nmesh,
           (unsigned)sPack.nmat);
}

// Black's map replacements a story flag switches on (src/unova/u3d_replace.h: the Champion's room once N's Castle has
// risen...): the matrix a map shows now
#include "u3d_replace.h"
static u16 Replaced(u16 mat)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUnovaReplace); i++)
        if (sUnovaReplace[i].mat == mat && U3D_Vm_Work(sUnovaReplace[i].work) == sUnovaReplace[i].value)
            return sUnovaReplace[i].alt;
    return mat;
}

// The matrix each map shows, fixed as its blocks load (Black swaps a map only when it's entered or reloaded, or at
// once by WORK_CmdSetMapReplaceFlag's "now": U3D_RefreshBlocks): a swap a script sets before a battle (the player's
// room left in a mess) shows once the field comes back, not as the battle's transition starts
static u16 sShown[ARRAY_COUNT(sUnovaMaps)];

static u16 ShownMat(s32 map)
{
    if (!sShown[map])
        sShown[map] = Replaced(sUnovaMaps[map].mat) + 1;
    return sShown[map] - 1;
}

static const U3dMatrix *MatrixNumbered(u16 number)
{
    u32 i;
    for (i = 0; sPack.ok && i < sPack.nmat; i++)
        if (sPack.mat[i].number == number)
            return &sPack.mat[i];
    return NULL;
}

static const U3dCell *CellIn(const U3dMatrix *mt, s32 cx, s32 cz)
{
    if (mt == NULL)
        return NULL;
    cx -= mt->x0;
    cz -= mt->z0;
    if (cx < 0 || cz < 0 || cx >= mt->width || cz >= mt->height)
        return NULL;
    return &mt->cells[cz * mt->width + cx];
}

static const U3dMatrix *sCurMat;
static s32 sCurMap;

static const U3dCell *Cell(s32 cx, s32 cz)
{
    return CellIn(sCurMat, cx, cz);
}

// ------------------------------------------------------------------------------------------------ seasons
// Black's season by the month (January spring, February summer, March autumn, April winter, then round again: its
// field's season), from the game clock (online the room host's: MP_ClockShift)
u8 U3D_Season(void)
{
    const char *test = getenv("MOEMON_U3D_SEASON");
    time_t t;
    struct tm *tm;
    if (test)
        return (u8)(atoi(test) & 3);
    t = time(NULL) + (time_t)MP_ClockShift();
    tm = localtime(&t);
    return tm ? (u8)(tm->tm_mon % 4) : 0;
}

static u8 sSeason;

// ------------------------------------------------------------------------------------------------ the ground's height
// A tile's ground height (world tile coordinates): of its heights, the one nearest NEAR (a bridge keeps its walker)
static float TileHeight(s32 gtx, s32 gtz, float near)
{
    s32 cx = gtx >= 0 ? gtx / 32 : -((31 - gtx) / 32), cz = gtz >= 0 ? gtz / 32 : -((31 - gtz) / 32);
    const U3dCell *c = Cell(cx, cz);
    u32 t, i, best = 0;
    if (c == NULL)
        return near;
    t = (gtz - cz * 32) * 32 + (gtx - cx * 32);
    if (c->hn[t] == 0)
        return near;
    for (i = 1; i < c->hn[t] && i < 2; i++)
        if (fabsf(c->h[t][i] - near) < fabsf(c->h[t][best] - near))
            best = i;
    return c->h[t][best];
}

static float GroundAt(float wx, float wz, float near)
{
    float fx = (wx - 8.0f) / 16.0f, fz = (wz - 8.0f) / 16.0f;
    s32 tx = (s32)floorf(fx), tz = (s32)floorf(fz);
    float ax = fx - tx, az = fz - tz;
    float h00 = TileHeight(tx, tz, near), h10 = TileHeight(tx + 1, tz, near);
    float h01 = TileHeight(tx, tz + 1, near), h11 = TileHeight(tx + 1, tz + 1, near);
    return (h00 * (1 - ax) + h10 * ax) * (1 - az) + (h01 * (1 - ax) + h11 * ax) * az;
}

static s32 MapIndex(u8 group, u8 num)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUnovaMaps); i++)
        if (sUnovaMaps[i].map == ((group << 8) | num))
            return i;
    return -1;
}

// UNOVA's packs beside the game (the release builds them from the user's ROM): without them the region isn't offered
bool8 U3D_Available(void)
{
    static s8 sHave = -1;
    if (sHave < 0)
    {
        static const char *const sPacks[] = {"unova/black3d.pak", "unova/black_scripts.pak", "unova/black_text.pak"};
        u32 i;
        sHave = 1;
        for (i = 0; i < ARRAY_COUNT(sPacks); i++)
        {
            FILE *f = fopen(Updater_ExeFile(sPacks[i]), "rb");
            if (f == NULL)
                sHave = 0;
            else
                fclose(f);
        }
    }
    return sHave;
}

bool8 U3D_IsUnovaGroup(u8 mapGroup)
{
    return mapGroup >= MAP_GROUP(MAP_UNOVA_BLACKCITY) && mapGroup < MAP_GROUP(MAP_UNOVA_BLACKCITY) + UNOVA_GROUPS;
}

// The map of Black's ZONE holding its tile (X, Z: the zone's matrix's tiles), and the tile on that map; FALSE none
bool8 U3D_MapOfZoneTile(u16 zone, u16 x, u16 z, u8 *group, u8 *num, s16 *mapX, s16 *mapY)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUnovaMaps); i++)
    {
        u32 cx = x / 32, cz = z / 32;
        if (sUnovaMaps[i].zone != zone)
            continue;
        if (sUnovaMaps[i].bw > 1 || sUnovaMaps[i].bh > 1 || cx >= sUnovaMaps[i].cx)
        {
            if (cx < sUnovaMaps[i].cx || cz < sUnovaMaps[i].cz || cx >= sUnovaMaps[i].cx + sUnovaMaps[i].bw
             || cz >= sUnovaMaps[i].cz + sUnovaMaps[i].bh)
                continue;
        }
        *group = sUnovaMaps[i].map >> 8;
        *num = sUnovaMaps[i].map & 0xFF;
        *mapX = x - 32 * sUnovaMaps[i].cx;
        *mapY = z - 32 * sUnovaMaps[i].cz;
        return TRUE;
    }
    return FALSE;
}

s32 U3D_MapIndex(void)
{
    return gSaveBlock1Ptr ? MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) : -1;
}

u8 U3D_MapCellX(s32 index) { return index >= 0 && index < (s32)ARRAY_COUNT(sUnovaMaps) ? sUnovaMaps[index].cx : 0; }
u8 U3D_MapCellZ(s32 index) { return index >= 0 && index < (s32)ARRAY_COUNT(sUnovaMaps) ? sUnovaMaps[index].cz : 0; }

u16 U3D_CurrentZone(void)
{
    s32 i;
    if (gSaveBlock1Ptr == NULL)
        return 0xFFFF;
    i = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    return i < 0 ? 0xFFFF : sUnovaMaps[i].zone;
}

// ------------------------------------------------------------------------------------------------ map blocks
const u16 *U3D_LayoutBlocks(const struct MapLayout *layout)
{
    static u16 *sBlocks[ARRAY_COUNT(sUnovaMaps)];
    static u16 sBlocksMat[ARRAY_COUNT(sUnovaMaps)];   // (the matrix they came from: a story's swap rebuilds them)
    u32 i, bx, bz, x, y;
    for (i = 0; i < ARRAY_COUNT(sUnovaMaps); i++)
    {
        const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(sUnovaMaps[i].map >> 8, sUnovaMaps[i].map & 0xFF);
        if (h->mapLayout != layout)
            continue;
        if (!sPack.tried)
            LoadPack();
        if (!sPack.ok)
            return layout->map;
        sShown[i] = Replaced(sUnovaMaps[i].mat) + 1;   // (loading: the map as the story has it now)
        if (sBlocks[i] == NULL || sBlocksMat[i] != ShownMat(i))
        {
            u32 w = 32 * sUnovaMaps[i].bw;
            const U3dMatrix *mt = MatrixNumbered(ShownMat(i));
            sBlocksMat[i] = ShownMat(i);
            if (sBlocks[i] == NULL)
                sBlocks[i] = malloc(sizeof(u16) * w * 32 * sUnovaMaps[i].bh);
            for (bz = 0; bz < sUnovaMaps[i].bh; bz++)
                for (bx = 0; bx < sUnovaMaps[i].bw; bx++)
                {
                    const U3dCell *c = CellIn(mt, sUnovaMaps[i].cx + bx, sUnovaMaps[i].cz + bz);
                    // (a cell of another zone inside an indoor map's rectangle: blocked)
                    bool8 mine = c != NULL && (c->zone == sUnovaMaps[i].zone || c->zone == 0xFFFF
                                               || (sUnovaMaps[i].bw == 1 && sUnovaMaps[i].bh == 1));
                    for (y = 0; y < 32; y++)
                        for (x = 0; x < 32; x++)
                            sBlocks[i][(bz * 32 + y) * w + bx * 32 + x] = mine ? c->blocks[y * 32 + x] : layout->map[0];
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
    s32 map;
    s32 camX, camY;          // Emerald's screen (0, 0) in the map's pixels
    float originX, originZ;  // the map's (0, 0) in Black's world
    float tx, ty, tz;        // the target (the player's tile centre, on the ground)
    float playerH;
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
    s32 hoffs = GetGpuReg(REG_OFFSET_BG3HOFS), voffs = GetGpuReg(REG_OFFSET_BG3VOFS);
    GetFieldCameraTileOffset(&tileX, &tileY);
    hoffs = (s8)((hoffs - tileX * 8) & 255);
    voffs = (s8)((voffs - tileY * 8) & 255);
    sView.camX = (gSaveBlock1Ptr->pos.x - 7) * 16 + hoffs;
    sView.camY = (gSaveBlock1Ptr->pos.y - 7) * 16 + voffs;
}

// Black's scene cameras (its scripts' PROC_FLD_StartCameraMove*: u3d_vm.c): the view moves to a target, pitch, yaw
// and distance over some frames, holds there, and comes back to the player's view the same way
static struct
{
    u8 mode;                          // CAM_FOLLOW, CAM_TO (to the target, then holding), CAM_BACK (to the player's)
    u16 t, frames;
    float from[7], to[7];             // target x, y, z, pitch, yaw (degrees), distance, the screen's shift (EMERALD_DROP)
    float cur[7];
} sCam;
enum { CAM_FOLLOW, CAM_TO, CAM_BACK };

static void CamStart(u8 mode, const float *to, u16 frames)
{
    memcpy(sCam.from, sCam.cur, sizeof(sCam.from));
    if (to)
        memcpy(sCam.to, to, sizeof(sCam.to));
    sCam.mode = mode, sCam.t = 0, sCam.frames = frames ? frames : 1;
}

// target in Black's world units (16 a tile, the zone's matrix), angles in 65536ths of a turn
void U3D_CameraMoveTo(u16 pitch, u16 yaw, s32 dist, s32 x, s32 y, s32 z, u16 frames)
{
    float to[7];
    to[0] = x / 4096.0f, to[1] = y / 4096.0f, to[2] = z / 4096.0f;
    to[3] = pitch * 360.0f / 65536.0f, to[4] = yaw * 360.0f / 65536.0f, to[5] = dist / 4096.0f;
    to[6] = 0.0f;   // (a scene frames its target as Black does)
    CamStart(CAM_TO, to, frames * 2);   // (Black's frames are its field's 30 a second)
}

void U3D_CameraMoveBack(u16 frames)
{
    if (sCam.mode != CAM_FOLLOW)
        CamStart(CAM_BACK, NULL, frames * 2);
}

void U3D_CameraReset(void)
{
    sCam.mode = CAM_FOLLOW;
}

bool8 U3D_CameraMoving(void)
{
    return sCam.mode != CAM_FOLLOW && sCam.t < sCam.frames;
}

static void SetupView(void)
{
    const struct Sprite *s = &gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId];
    u8 cam;
    float p = CAM_PITCH * 3.14159265f / 180.0f;
    float sp = sinf(p), cp = cosf(p), aim;
    for (cam = 0; cam < OBJECT_EVENTS_COUNT; cam++)
        if (gObjectEvents[cam].active && gObjectEvents[cam].localId == OBJ_EVENT_ID_CAMERA)
        {
            s = &gSprites[gObjectEvents[cam].spriteId];
            break;
        }
    sView.ax = s->x + s->x2 + gSpriteCoordOffsetX;
    sView.ay = s->y + s->y2 + gSpriteCoordOffsetY - s->centerToCornerVecY - 8;
    sView.originX = sUnovaMaps[sView.map].cx * BLOCK;
    sView.originZ = sUnovaMaps[sView.map].cz * BLOCK;
    sView.tx = sView.originX + sView.camX + sView.ax;
    sView.tz = sView.originZ + sView.camY + sView.ay;
    sView.ty = GroundAt(sView.tx, sView.tz, sView.havePlayerH ? sView.playerH : 0.0f);
    sView.playerH = sView.ty;
    sView.havePlayerH = TRUE;
    {
        // the player's view, or a scene's camera on the way to (or back from) its own
        float follow[7], *v = sCam.cur, f, sy, cy;
        u32 i;
        follow[0] = sView.tx, follow[1] = sView.ty, follow[2] = sView.tz;
        follow[3] = CAM_PITCH, follow[4] = 0.0f, follow[5] = CAM_DIST, follow[6] = EMERALD_DROP;
        {
            // a rail map's tile: Black's camera of that rail, its pitch and distance eased to (the view stays north up:
            // a turned view would want the d-pad and the sprites' facings turned with it)
            static float sRailPitch = CAM_PITCH, sRailDist = CAM_DIST;
            float wantP = CAM_PITCH, wantD = CAM_DIST;
            s32 gtx = (s32)floorf(sView.tx / 16.0f), gtz = (s32)floorf(sView.tz / 16.0f);
            const U3dCell *rc = Cell(gtx >= 0 ? gtx / 32 : -1, gtz >= 0 ? gtz / 32 : -1);
            if (rc && rc->cam && gtx >= 0 && gtz >= 0)
            {
                const u16 *rcam = rc->cam[(gtz % 32) * 32 + gtx % 32];
                if (rcam[2] != 0)
                    wantP = rcam[0] * 360.0f / 65536.0f, wantD = rcam[2];
            }
            sRailPitch += (wantP - sRailPitch) * 0.08f;
            sRailDist += (wantD - sRailDist) * 0.08f;
            follow[3] = sRailPitch, follow[5] = sRailDist;
        }
        if (sCam.mode == CAM_FOLLOW)
            memcpy(v, follow, sizeof(follow));
        else
        {
            const float *to = sCam.mode == CAM_TO ? sCam.to : follow;
            if (sCam.t < sCam.frames)
                sCam.t++;
            f = (float)sCam.t / sCam.frames;
            f = f * f * (3.0f - 2.0f * f);   // (eased at both ends, as Black's)
            for (i = 0; i < 7; i++)
                v[i] = sCam.from[i] + (to[i] - sCam.from[i]) * f;
            if (sCam.mode == CAM_BACK && sCam.t >= sCam.frames)
                sCam.mode = CAM_FOLLOW;
        }
        p = v[3] * 3.14159265f / 180.0f;
        sp = sinf(p), cp = cosf(p);
        sy = sinf(v[4] * 3.14159265f / 180.0f), cy = cosf(v[4] * 3.14159265f / 180.0f);
        // (Black aims a little above the ground; the player's tile stays where Emerald's camera puts it on the screen,
        // and a scene's target there too)
        aim = v[1] + CAM_LIFT;
        sView.eye[0] = v[0] + v[5] * cp * sy;
        sView.eye[1] = aim + v[5] * sp;
        sView.eye[2] = v[2] + v[5] * cp * cy;
        sView.fwd[0] = -cp * sy, sView.fwd[1] = -sp, sView.fwd[2] = -cp * cy;
        sView.right[0] = cy, sView.right[1] = 0.0f, sView.right[2] = -sy;
        sView.up[0] = -sp * sy, sView.up[1] = cp, sView.up[2] = -sp * cy;
        sView.focal = CAM_DIST;   // (one unit a pixel at the player's distance)
    }
    sView.ay -= CAM_LIFT * cp;   // (the aim's point 4 up shows a little above the player's feet on the screen)
    sView.ay += sCam.cur[6];
}

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

static float sSpriteH[MAX_SPRITES];
static u8 sSpriteHFrame[MAX_SPRITES];

void U3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth)
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
    if (spriteId < MAX_SPRITES && (u8)(sView.frame - sSpriteHFrame[spriteId]) <= 2)
        near = sSpriteH[spriteId];
    h = GroundAt(wx, wz, near);
    if (spriteId < MAX_SPRITES)
    {
        sSpriteH[spriteId] = h;
        sSpriteHFrame[spriteId] = sView.frame;
    }
    if (!Project(wx, h, wz + FEET_TOWARD, &sx, &sy, &d))
    {
        *outX = x;
        *outY = feetY;
        *depth = 0.0f;
        return;
    }
    *outX = (s32)floorf(sx + 0.5f);
    *outY = (s32)floorf(sy + 0.5f);
    *depth = d;
}

// ------------------------------------------------------------------------------------------------ sprites in the scene
float gU3dSpriteDepth[MAX_SPRITES];
s16 gU3dSpriteFeet[MAX_SPRITES];
typedef struct { float d0; s16 feet; u8 flat, refl; } OamDepth;
static OamDepth sPendingOam[OAM_ENTRY_COUNT], sLiveOam[OAM_ENTRY_COUNT];
static bool8 sLiveAny;

void U3D_NoteOam(u16 spriteId, u16 first, u16 end)
{
    u16 i;
    bool8 on = sView.active && spriteId < MAX_SPRITES && gSprites[spriteId].coordOffsetEnabled;
    for (i = first; i < end && i < OAM_ENTRY_COUNT; i++)
    {
        sPendingOam[i].d0 = on ? gU3dSpriteDepth[spriteId] : 0.0f;
        sPendingOam[i].feet = on ? gU3dSpriteFeet[spriteId] : 0;
        sPendingOam[i].flat = on && gSprites[spriteId].mpShadow;
        sPendingOam[i].refl = on && FieldEffect_IsReflectionSprite(&gSprites[spriteId]);
    }
}

void U3D_CommitOam(void)
{
    memcpy(sLiveOam, sPendingOam, sizeof(sLiveOam));
    sLiveAny = sView.active;
    if (sView.active)
        memset(sPendingOam, 0, sizeof(sPendingOam));
}

static bool8 OverReflectiveCell(int x, int line)
{
    float rx = (x - sView.ax) / sView.focal, ry = (sView.ay - line) / sView.focal;
    float d[3], t, wx, wz;
    u32 i, b;
    for (i = 0; i < 3; i++)
        d[i] = sView.fwd[i] + sView.right[i] * rx + sView.up[i] * ry;
    if (d[1] > -1e-4f)
        return TRUE;
    t = (sView.ty - sView.eye[1]) / d[1];
    wx = sView.eye[0] + d[0] * t - sView.originX;
    wz = sView.eye[2] + d[2] * t - sView.originZ;
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
    if (o->refl && !OverReflectiveCell(x, line))
        return TRUE;
    if (o->flat)
        return sDepth[line][x] < o->d0 - 16.0f;
    d = o->d0 - (o->feet - line) * 1.3375f;   // tan(53.22 degrees): a card's pixel above its feet is that much nearer
    return sDepth[line][x] < d - 6.0f;
}

// ------------------------------------------------------------------------------------------------ lights
static s32 sLightBlock = -1;
static const U3dLightBlock *sLight;

// Black's light for a cell (its table: the area's own a season when 0x8000) now: the set of that table in the season
static void PickLight(u16 light)
{
    const U3dLightSet *s;
    u32 now, i, pick, set = ((light & 0x8000) ? (light & 0x7FFF) + sSeason : light) * 4 + sSeason;
    if (set >= sPack.nlight || sPack.light[set].count == 0)
    {
        sLight = NULL;
        return;
    }
    s = &sPack.light[set];
    now = (gLocalTime.hours * 3600 + gLocalTime.minutes * 60) / 2;
    if (getenv("MOEMON_U3D_HOUR"))   // (tests: a fixed hour)
        now = atoi(getenv("MOEMON_U3D_HOUR")) * 1800;
    pick = s->count - 1;
    for (i = 0; i < s->count; i++)
        if (s->b[i].start > now)
        {
            pick = i;
            break;
        }
    sLight = &s->b[pick];
    sLightBlock = set * 256 + pick;   // (the vertices' light is worked out again when this changes)
}

static void Light(U3dGroup *g)
{
    u32 i, k, ch;
    if (g->litFor == sLightBlock || sLight == NULL)
        return;
    g->litFor = sLightBlock;
    for (i = 0; i < g->count; i++)
    {
        const U3dVtx *v = &g->v[i];
        float n[3] = {v->nx, v->ny, v->nz};
        float len = sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        float acc[3];
        if (len > 0.0f)
            n[0] /= len, n[1] /= len, n[2] /= len;
        for (ch = 0; ch < 3; ch++)
            acc[ch] = sLight->col[3][ch];
        for (k = 0; k < 4; k++)
        {
            const U3dLight *l = &sLight->l[k];
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
typedef struct { float x, y, iz, u, v, c[3]; } SVtx;

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

static void DrawTriangle(const SVtx *a, const SVtx *b, const SVtx *c, const U3dGroup *g)
{
    float area = (b->x - a->x) * (c->y - a->y) - (c->x - a->x) * (b->y - a->y);
    s32 minx, maxx, miny, maxy, x, y;
    const U3dTex *t = g->tex >= 0 && (u32)g->tex < sPack.ntex ? &sPack.tex[g->tex] : NULL;
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
                u16 pxc = 0;
                for (ch = 0; ch < 3; ch++)
                {
                    float o = (old >> (5 * ch)) & 31;
                    float v = out[ch] * al + o * (1.0f - al);
                    s32 iv = (s32)(v + 0.5f);
                    pxc |= (iv < 0 ? 0 : iv > 31 ? 31 : iv) << (5 * ch);
                }
                sColor[y][x] = pxc;
            }
            if (clear)
                sIds[y][x] = g->pid;
            else if (ta == 0xFF)
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

static void DrawMesh(const U3dMesh *mesh, const float *m, float ox, float oy, float oz, int pass)
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
    if (Project(cx + ox, cy + oy, cz + oz, &sx, &sy, &d))
    {
        float rad = mesh->r * sView.focal / d * 1.5f + 2.0f;
        if (sx + rad < 0 || sx - rad > W || sy + rad < 0 || sy - rad > H)
            return;
    }
    for (gi = 0; gi < mesh->ngroups; gi++)
    {
        U3dGroup *g = &mesh->g[gi];
        bool8 soft = g->alpha < 31 || (g->tex >= 0 && (u32)g->tex < sPack.ntex && sPack.tex[g->tex].soft);
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
            const U3dVtx *v = &g->v[i];
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
                s->c[0] = g->lit[3 * i] * s->iz, s->c[1] = g->lit[3 * i + 1] * s->iz, s->c[2] = g->lit[3 * i + 2] * s->iz;
            else
                s->c[0] = v->r * s->iz, s->c[1] = v->g * s->iz, s->c[2] = v->b * s->iz;
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
    u16 clear = 0;
    {
        const U3dCell *here = Cell(bx, bz);
        PickLight(here ? here->light : 0);
    }
    if (sLight && sLight->clear[3])   // (Black's clear colour: what shows where nothing is drawn)
        clear = sLight->clear[0] | sLight->clear[1] << 5 | sLight->clear[2] << 10;
    for (i = 0; i < W * H; i++)
    {
        (&sColor[0][0])[i] = clear;
        (&sDepth[0][0])[i] = 1e30f;
        (&sIds[0][0])[i] = -1;
    }
    // (a matrix of bigger blocks, SKYARROW BRIDGE's: the blocks round the view, each drawn from its first cell at its
    // middle)
    s32 k = sCurMat && sCurMat->scale > 1 ? sCurMat->scale : 1;
    bx = (s32)floorf(sView.tx / (BLOCK * k)), bz = (s32)floorf(sView.tz / (BLOCK * k));
    for (pass = 0; pass < 2; pass++)
        for (cz = bz - 1; cz <= bz + 1; cz++)
            for (cx = bx - 1; cx <= bx + 1; cx++)
            {
                const U3dCell *c = Cell(cx * k, cz * k);
                float ox = (cx + 0.5f) * BLOCK * k, oz = (cz + 0.5f) * BLOCK * k;
                s16 mesh;
                if (c == NULL)
                    continue;
                mesh = c->mesh[sSeason];
                if (mesh >= 0 && (u32)mesh < sPack.nmesh)
                    DrawMesh(&sPack.mesh[mesh], NULL, ox, 0.0f, oz, pass);
                for (i = 0; i < c->nprops; i++)
                {
                    s16 pm = c->props[i].mesh[sSeason];
                    if (pm >= 0 && (u32)pm < sPack.nmesh)
                        DrawMesh(&sPack.mesh[pm], c->props[i].m, ox, 0.0f, oz, pass);
                }
            }
}

// ------------------------------------------------------------------------------------------------ fades, lines
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

static bool8 OnField(void)
{
    return gMain.callback2 == CB2_Overworld || gMain.callback2 == CB2_OverworldBasic;
}

static void U3dLine(int line, u16 layers[4][DISPLAY_WIDTH]);

static struct { bool8 on; s16 dx, dy; } sShopPic;
void U3D_ShopPicture(bool8 on, s16 dx, s16 dy)
{
    sShopPic.on = on && sView.active;
    sShopPic.dx = dx, sShopPic.dy = dy;
    if (sShopPic.on)
        gMpU3dLine = U3dLine;
}

static void U3dLine(int line, u16 layers[4][DISPLAY_WIDTH])
{
    s32 x;
    if (sShopPic.on && !OnField())
    {
        s32 sy = line - sShopPic.dy;
        if (line == 0)
            FindFade();
        for (x = 0; x < W; x++)
        {
            s32 sx = x - sShopPic.dx;
            layers[3][x] = (sy >= 0 && sy < H && sx >= 0 && sx < W) ? Faded(sColor[sy][sx]) : Faded(0);
            layers[2][x] = 0;
        }
        return;
    }
    if (!sView.active || !OnField())
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
float U3D_TestGround(s32 mapX, s32 mapY, float near)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return NAN;
    if (!sPack.tried)
        LoadPack();
    if (!sPack.ok)
        return NAN;
    sCurMat = MatrixNumbered(ShownMat(map));
    sCurMap = map;
    return GroundAt(sUnovaMaps[map].cx * BLOCK + mapX, sUnovaMaps[map].cz * BLOCK + mapY, near);
}

// Black's map replaced while the player stands on it (WORK_CmdSetMapReplaceFlag ..., now): the field's blocks again
void U3D_RefreshBlocks(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    const struct MapLayout *layout = gMapHeader.mapLayout;
    const u16 *b;
    s32 x, y;
    if (map < 0 || layout == NULL)
        return;
    b = U3D_LayoutBlocks(layout);
    for (y = 0; y < layout->height; y++)
        for (x = 0; x < layout->width; x++)
            MapGridSetMetatileEntryAt(x + MAP_OFFSET, y + MAP_OFFSET, b[y * layout->width + x]);
}

bool8 U3D_Active(void)
{
    return sView.active && OnField();
}

void U3D_Update(void)
{
    s32 map = MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map >= 0 && !sPack.tried)
        LoadPack();
    sView.active = map >= 0 && sPack.ok;
    sShopPic.on = FALSE;
    gMpU3dLine = sView.active ? U3dLine : NULL;
    gMpU3dOamHidden = sView.active ? OamHidden : NULL;
    if (!sView.active)
        return;
    if (sView.map != map)
    {
        sView.havePlayerH = FALSE;
        sCam.mode = CAM_FOLLOW;   // (a scene's camera stays on its map)
        sSeason = U3D_Season();   // (Black changes the season as a map is entered)
    }
    sView.map = map;
    sCurMat = MatrixNumbered(ShownMat(map));
    sCurMap = map;
    sView.frame++;
    FindCamera();
    SetupView();
}
