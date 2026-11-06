// Moemon Maker, M5: SINNOH's 3D mode, new models (docs/MOEMON_MAKER.md "3D"). A model made in Blender and exported as
// glTF binary (.glb: File > Export > glTF 2.0, "glTF Binary") goes in MoemonMaker/models/; a map's placements in
// MoemonMaker/models/MAP_X.txt, one a line:
//
//   place FILE.glb X Y [HEIGHT] [TURN] [SCALE]     (the tile; HEIGHT in tiles over the ground there; TURN degrees about
//                                                   the up axis; SCALE in percent. One Blender unit = one tile)
//
// Read here into the 3D engine's own meshes (S3D_AddMesh: each primitive a mesh with its base color texture, or its
// base color), every mesh node's translation / rotation / scale applied, and placed as run-time props when the map
// loads (S3D_MAKER_PROPS_FIRST..: up to 16 meshes a map). The .glb reader is small: positions, normals, the first
// texture coordinates, indices (8 / 16 / 32-bit), materials' base color and PNG / JPEG textures.
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "s3d.h"
#include "maker.h"
#include "maker_api.h"

unsigned char *stbi_load_from_memory(const unsigned char *, int, int *, int *, int *, int);
void stbi_image_free(void *);
const char *Maker_ReadDir(void);
void MPNet_Log(const char *fmt, ...);

// ---- a little JSON
enum { J_NULL, J_NUM, J_STR, J_ARR, J_OBJ, J_BOOL };
struct J { u8 type; double num; char *str; int n; struct J *kids; char **keys; };

static const char *Ws(const char *p) { while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++; return p; }

static const char *Parse(const char *p, struct J *j)
{
    p = Ws(p);
    memset(j, 0, sizeof(*j));
    if (*p == '{' || *p == '[')
    {
        bool8 obj = *p == '{';
        j->type = obj ? J_OBJ : J_ARR;
        p = Ws(p + 1);
        while (*p && *p != (obj ? '}' : ']'))
        {
            j->kids = realloc(j->kids, sizeof(struct J) * (j->n + 1));
            if (obj)
            {
                struct J key;
                j->keys = realloc(j->keys, sizeof(char *) * (j->n + 1));
                p = Parse(p, &key);
                j->keys[j->n] = key.str ? key.str : strdup("");
                p = Ws(p);
                if (*p == ':')
                    p++;
            }
            p = Parse(p, &j->kids[j->n]);
            j->n++;
            p = Ws(p);
            if (*p == ',')
                p = Ws(p + 1);
            else if (*p != (obj ? '}' : ']'))
                return p;   // (broken: stop here)
        }
        return *p ? p + 1 : p;
    }
    if (*p == '"')
    {
        const char *e = ++p;
        int n = 0;
        j->type = J_STR;
        while (*e && *e != '"')
            e += (*e == '\\' && e[1]) ? 2 : 1;
        j->str = malloc(e - p + 1);
        while (p < e)
        {
            if (*p == '\\' && p + 1 < e)
                p++;
            j->str[n++] = *p++;
        }
        j->str[n] = 0;
        return *e ? e + 1 : e;
    }
    if (!strncmp(p, "true", 4) || !strncmp(p, "false", 5))
    {
        j->type = J_BOOL, j->num = *p == 't';
        return p + (*p == 't' ? 4 : 5);
    }
    if (!strncmp(p, "null", 4))
        return p + 4;
    {
        char *e;
        j->type = J_NUM;
        j->num = strtod(p, &e);
        return e > p ? e : p + 1;
    }
}

static void Free(struct J *j)
{
    int i;
    for (i = 0; i < j->n; i++)
    {
        Free(&j->kids[i]);
        if (j->keys)
            free(j->keys[i]);
    }
    free(j->kids), free(j->keys), free(j->str);
}

static struct J *Get(struct J *j, const char *key)
{
    int i;
    for (i = 0; j && j->type == J_OBJ && i < j->n; i++)
        if (!strcmp(j->keys[i], key))
            return &j->kids[i];
    return NULL;
}
static struct J *At(struct J *j, int i) { return j && j->type == J_ARR && i >= 0 && i < j->n ? &j->kids[i] : NULL; }
static double Num(struct J *j, double d) { return j && (j->type == J_NUM || j->type == J_BOOL) ? j->num : d; }

// ---- a model: its meshes
#define MODEL_MESHES 16
struct Model { char name[64]; s16 mesh[MODEL_MESHES]; int nmesh; };
static struct Model *sModels;
static int sModelCount;

struct Glb { struct J json; const u8 *bin; u32 binLen; };

// An accessor's element I, component K, as a float (normalized integers kept as their value)
static bool8 Accessor(struct Glb *g, int acc, const u8 **base, int *count, int *comps, int *ctype, int *stride)
{
    struct J *a = At(Get(&g->json, "accessors"), acc), *bv;
    const char *type;
    int view, off, csize;
    if (!a)
        return FALSE;
    view = (int)Num(Get(a, "bufferView"), -1);
    bv = At(Get(&g->json, "bufferViews"), view);
    if (!bv)
        return FALSE;
    type = Get(a, "type") ? Get(a, "type")->str : "";
    *comps = !strcmp(type, "SCALAR") ? 1 : !strcmp(type, "VEC2") ? 2 : !strcmp(type, "VEC3") ? 3 : !strcmp(type, "VEC4") ? 4 : 0;
    *ctype = (int)Num(Get(a, "componentType"), 0);
    *count = (int)Num(Get(a, "count"), 0);
    csize = *ctype == 5126 || *ctype == 5125 ? 4 : *ctype == 5123 || *ctype == 5122 ? 2 : 1;
    *stride = (int)Num(Get(bv, "byteStride"), 0);
    if (!*stride)
        *stride = csize * *comps;
    off = (int)Num(Get(bv, "byteOffset"), 0) + (int)Num(Get(a, "byteOffset"), 0);
    if (!*comps || off < 0 || (u32)off + (u32)(*stride) * (*count > 0 ? *count - 1 : 0) + csize * *comps > g->binLen)
        return FALSE;
    *base = g->bin + off;
    return TRUE;
}

static float Comp(const u8 *p, int ctype)
{
    switch (ctype)
    {
    case 5126: { float f; memcpy(&f, p, 4); return f; }
    case 5125: { u32 v; memcpy(&v, p, 4); return (float)v; }
    case 5123: { u16 v; memcpy(&v, p, 2); return (float)v; }
    case 5122: { s16 v; memcpy(&v, p, 2); return (float)v; }
    case 5121: return (float)*p;
    default: return (float)*(const s8 *)p;
    }
}

// A material's base color texture (RGBA) or its base color as a 1x1 one
static u32 *Texture(struct Glb *g, int material, int *w, int *h)
{
    struct J *m = At(Get(&g->json, "materials"), material), *pbr = Get(m, "pbrMetallicRoughness");
    struct J *bct = Get(pbr, "baseColorTexture"), *factor = Get(pbr, "baseColorFactor");
    u32 *px;
    if (bct)
    {
        struct J *tex = At(Get(&g->json, "textures"), (int)Num(Get(bct, "index"), -1));
        struct J *img = At(Get(&g->json, "images"), (int)Num(Get(tex, "source"), -1));
        struct J *bv = At(Get(&g->json, "bufferViews"), (int)Num(Get(img, "bufferView"), -1));
        if (bv)
        {
            int off = (int)Num(Get(bv, "byteOffset"), 0), len = (int)Num(Get(bv, "byteLength"), 0), n;
            if (off >= 0 && len > 0 && (u32)(off + len) <= g->binLen)
            {
                u8 *rgba = stbi_load_from_memory(g->bin + off, len, w, h, &n, 4);
                if (rgba && *w > 0 && *h > 0 && *w <= 1024 && *h <= 1024)
                {
                    px = malloc(sizeof(u32) * *w * *h);
                    memcpy(px, rgba, sizeof(u32) * *w * *h);   // (R, G, B, A bytes: the engine's r | g << 8 ...)
                    stbi_image_free(rgba);
                    return px;
                }
                if (rgba)
                    stbi_image_free(rgba);
            }
        }
    }
    px = malloc(sizeof(u32));
    *w = *h = 1;
    {
        int r = (int)(255 * Num(At(factor, 0), 1)), gg = (int)(255 * Num(At(factor, 1), 1)), b = (int)(255 * Num(At(factor, 2), 1));
        px[0] = 0xFF000000u | (b << 16) | (gg << 8) | r;
    }
    return px;
}

// A node's matrix (column-major 4x4) from its TRS
static void NodeMatrix(struct J *node, float m[16])
{
    struct J *t = Get(node, "translation"), *r = Get(node, "rotation"), *s = Get(node, "scale"), *mat = Get(node, "matrix");
    float qx = (float)Num(At(r, 0), 0), qy = (float)Num(At(r, 1), 0), qz = (float)Num(At(r, 2), 0), qw = (float)Num(At(r, 3), 1);
    float sx = (float)Num(At(s, 0), 1), sy = (float)Num(At(s, 1), 1), sz = (float)Num(At(s, 2), 1);
    int i;
    if (mat && mat->n == 16)
    {
        for (i = 0; i < 16; i++)
            m[i] = (float)Num(At(mat, i), 0);
        return;
    }
    m[0] = (1 - 2 * (qy * qy + qz * qz)) * sx, m[1] = (2 * (qx * qy + qz * qw)) * sx, m[2] = (2 * (qx * qz - qy * qw)) * sx, m[3] = 0;
    m[4] = (2 * (qx * qy - qz * qw)) * sy, m[5] = (1 - 2 * (qx * qx + qz * qz)) * sy, m[6] = (2 * (qy * qz + qx * qw)) * sy, m[7] = 0;
    m[8] = (2 * (qx * qz + qy * qw)) * sz, m[9] = (2 * (qy * qz - qx * qw)) * sz, m[10] = (1 - 2 * (qx * qx + qy * qy)) * sz, m[11] = 0;
    m[12] = (float)Num(At(t, 0), 0), m[13] = (float)Num(At(t, 1), 0), m[14] = (float)Num(At(t, 2), 0), m[15] = 1;
}

static void AddPrimitive(struct Glb *g, struct Model *mo, struct J *prim, const float nm[16])
{
    struct J *attr = Get(prim, "attributes");
    const u8 *pb, *nb = NULL, *ub = NULL, *ib = NULL;
    int pc, pcomp, ptype, pstride, nc = 0, ncomp, ntype, nstride, uc = 0, ucomp, utype, ustride, ic = 0, icomp, itype, istride;
    int i, n, tw, th;
    float *v;
    u32 *tex;
    if (mo->nmesh >= MODEL_MESHES || (int)Num(Get(prim, "mode"), 4) != 4
     || !Accessor(g, (int)Num(Get(attr, "POSITION"), -1), &pb, &pc, &pcomp, &ptype, &pstride) || pcomp < 3)
        return;
    if (Get(attr, "NORMAL") && !Accessor(g, (int)Num(Get(attr, "NORMAL"), -1), &nb, &nc, &ncomp, &ntype, &nstride))
        nb = NULL;
    if (Get(attr, "TEXCOORD_0") && !Accessor(g, (int)Num(Get(attr, "TEXCOORD_0"), -1), &ub, &uc, &ucomp, &utype, &ustride))
        ub = NULL;
    if (Get(prim, "indices") && !Accessor(g, (int)Num(Get(prim, "indices"), -1), &ib, &ic, &icomp, &itype, &istride))
        return;
    n = ib ? ic : pc;
    if (n < 3 || n > 300000)
        return;
    v = malloc(sizeof(float) * 8 * n);
    for (i = 0; i < n; i++)
    {
        int k = ib ? (int)Comp(ib + i * istride, itype) : i, c;
        float p[3], q[3] = {0, 1, 0};
        if (k < 0 || k >= pc)
            k = 0;
        for (c = 0; c < 3; c++)
            p[c] = Comp(pb + k * pstride + c * 4, ptype);
        if (nb && k < nc)
            for (c = 0; c < 3; c++)
                q[c] = Comp(nb + k * nstride + c * 4, ntype);
        // (the node's matrix; one Blender unit is a tile: 16 of the engine's units; glTF's +Y is up as the engine's)
        for (c = 0; c < 3; c++)
            v[8 * i + c] = (nm[c] * p[0] + nm[4 + c] * p[1] + nm[8 + c] * p[2] + nm[12 + c]) * 16.0f;
        for (c = 0; c < 3; c++)
            v[8 * i + 5 + c] = nm[c] * q[0] + nm[4 + c] * q[1] + nm[8 + c] * q[2];
        {
            float l = sqrtf(v[8 * i + 5] * v[8 * i + 5] + v[8 * i + 6] * v[8 * i + 6] + v[8 * i + 7] * v[8 * i + 7]);
            if (l > 0)
                v[8 * i + 5] /= l, v[8 * i + 6] /= l, v[8 * i + 7] /= l;
        }
        v[8 * i + 3] = ub && k < uc ? Comp(ub + k * ustride, utype) : 0;
        v[8 * i + 4] = ub && k < uc ? Comp(ub + k * ustride + 4, utype) : 0;
    }
    tex = Texture(g, (int)Num(Get(prim, "material"), -1), &tw, &th);
    mo->mesh[mo->nmesh] = S3D_AddMesh(v, n, tex, tw, th);
    if (mo->mesh[mo->nmesh] >= 0)
        mo->nmesh++;
    free(v);
    free(tex);
}

static struct Model *LoadModel(const char *name)
{
    char path[1300];
    FILE *f;
    long size;
    u8 *data;
    struct Glb g;
    struct Model *mo;
    u32 jlen;
    int i;
    for (i = 0; i < sModelCount; i++)
        if (!strcmp(sModels[i].name, name))
            return sModels[i].nmesh ? &sModels[i] : NULL;
    sModels = realloc(sModels, sizeof(*sModels) * (sModelCount + 1));
    mo = &sModels[sModelCount++];
    memset(mo, 0, sizeof(*mo));
    snprintf(mo->name, sizeof(mo->name), "%s", name);
    if (strchr(name, '/') || strchr(name, '\\') || strstr(name, ".."))
        return NULL;
    snprintf(path, sizeof(path), "%s/models/%s", Maker_ReadDir(), name);
    if ((f = fopen(path, "rb")) == NULL)
    {
        MPNet_Log("maker model: no %s", name);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size + 1);
    if (size < 28 || fread(data, 1, size, f) != (size_t)size || memcmp(data, "glTF", 4) != 0)
    {
        fclose(f);
        free(data);
        MPNet_Log("maker model: %s isn't a .glb (glTF binary)", name);
        return NULL;
    }
    fclose(f);
    memcpy(&jlen, data + 12, 4);
    if (20 + jlen > (u32)size || memcmp(data + 16, "JSON", 4) != 0)
    {
        free(data);
        return NULL;
    }
    {
        char *js = malloc(jlen + 1);
        memcpy(js, data + 20, jlen);
        js[jlen] = 0;
        Parse(js, &g.json);
        free(js);
    }
    g.bin = NULL, g.binLen = 0;
    if (20 + jlen + 8 <= (u32)size && memcmp(data + 20 + jlen + 4, "BIN", 3) == 0)
    {
        memcpy(&g.binLen, data + 20 + jlen, 4);
        g.bin = data + 28 + jlen;
        if (28 + jlen + g.binLen > (u32)size)
            g.binLen = size - 28 - jlen;
    }
    // every node with a mesh (its own matrix), else every mesh as it is
    {
        struct J *nodes = Get(&g.json, "nodes"), *meshes = Get(&g.json, "meshes");
        bool8 any = FALSE;
        for (i = 0; nodes && i < nodes->n; i++)
        {
            struct J *node = At(nodes, i), *mesh = At(meshes, (int)Num(Get(node, "mesh"), -1)), *prims;
            float m[16];
            int k;
            if (!mesh)
                continue;
            NodeMatrix(node, m);
            prims = Get(mesh, "primitives");
            for (k = 0; prims && k < prims->n; k++)
                AddPrimitive(&g, mo, At(prims, k), m);
            any = TRUE;
        }
        for (i = 0; !any && meshes && i < meshes->n; i++)
        {
            static const float id[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            struct J *prims = Get(At(meshes, i), "primitives");
            int k;
            for (k = 0; prims && k < prims->n; k++)
                AddPrimitive(&g, mo, At(prims, k), id);
        }
    }
    Free(&g.json);
    free(data);
    MPNet_Log("maker model: %s read (%d meshes)", name, mo->nmesh);
    return mo->nmesh ? mo : NULL;
}

// overworld.c, as a map's grid is built (after SINNOH's gyms place theirs): this map's models placed
void Maker_ModelsMapLoaded(void)
{
    char path[1300], line[200];
    FILE *f;
    int slot = S3D_MAKER_PROPS_FIRST, map;
    float ox, oz;
    for (map = S3D_MAKER_PROPS_FIRST; map < S3D_DYN_PROPS; map++)
        S3D_DynPropHide(map);
    if (!S3D_MapOrigin(&ox, &oz))   // (a SINNOH map, the 3D pack there: as the gyms ask)
        return;
    map = Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map < 0)
        return;
    snprintf(path, sizeof(path), "%s/models/%s.txt", Maker_ReadDir(), Maker_Map(map)->name);
    if ((f = fopen(path, "rb")) == NULL)
        return;
    while (fgets(line, sizeof(line), f))
    {
        char file[64];
        int x, y, k, n;
        float height = 0, turn = 0, scale = 100, ground;
        struct Model *mo;
        n = sscanf(line, "place %63s %d %d %f %f %f", file, &x, &y, &height, &turn, &scale);
        if (n < 3 || (mo = LoadModel(file)) == NULL)
            continue;
        ground = S3D_TestGround(x * 16 + 8, y * 16 + 8, 0);
        if (ground != ground)   // (NaN: no ground there)
            ground = 0;
        for (k = 0; k < mo->nmesh && slot < S3D_DYN_PROPS; k++)
            S3D_DynPropMesh(slot++, mo->mesh[k], ox + x * 16 + 8, ground + height * 16, oz + y * 16 + 8, turn, scale / 100.0f);
        MPNet_Log("maker model: %s placed at %d,%d", file, x, y);
    }
    fclose(f);
}

// The editor: the map's placements changed (its file written): placed again if the game is on that map
void MakerApi_ModelsChanged(const char *mapName)
{
    int map = Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    if (map >= 0 && !strcmp(Maker_Map(map)->name, mapName))
        Maker_ModelsMapLoaded();
}
