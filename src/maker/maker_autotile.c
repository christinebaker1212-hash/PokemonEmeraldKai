// Moemon Maker: AUTOTILE (the TILES panel's switch, maker/maker_mapview.cpp; the TILESETS window's TERRAINS,
// maker/maker_tilesets_ui.cpp). Water, trees, paths, cliffs... painted freely: the tiles round them (edges, corners, a
// pond's lip) are chosen as the game's own maps choose them. Nothing is written by hand: each tileset's terrains are
// learned from the maps that use it, the first time a map of that tileset is painted:
//
//   terrains: a centre tile the maps surround with itself (water, sand), or a repeating 2x2 / 2x1 / 1x2 pattern
//     (trees). The primary tileset's are learned from every map that uses it, the secondary's from its own maps.
//   a terrain's tiles: its centre, and the pieces beside it on the tileset's sheet (a path's nine tiles are a 3x3 block,
//     the trees a 2x2 one) that lie beside it on the maps in a good part of their uses. A tile that sits between two
//     kinds of terrain (water above, grass below: a pond's lip) belongs to the side it walks like.
//   what each terrain draws: by its shape (which of the 8 neighbours are the same terrain, as RPG Maker's 47 shapes),
//     where it is in its pattern (a tree's left or right half), and what lies around it (land, water, a wall, other
//     tiles): the block the maps put there most often, its collision and elevation with it.
//
// Painting a terrain over some tiles makes them that terrain; they and the ring round them are drawn again (the ring
// only where its own shape changed, so the map's own art stays as it was). A shape the maps never show against what's
// around it draws the nearest shape they do, or (too far from any) the plain centre: never a wrong edge.
//
// The user's changes to the learned rules, per pair of tilesets, in MoemonMaker/autotile/PRIMARY+SECONDARY.txt:
//   name CENTRE NEW NAME            a terrain renamed (CENTRE: its centre tile(s), e.g. 0x0A1 or 0x1D4+0x1D5+0x1DC+0x1DD)
//   hide CENTRE                     not offered in the TILES panel
//   fix CENTRE SHAPE PHASE TILE     what it draws for a shape (0-255, as its neighbours' bits) at a phase (0-3); TILE -1:
//                                   the learned one again
//   terrain CENTRE [WxH]            a terrain of the user's own (new tiles: its pieces are its fix lines)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "global.h"
#include "global.fieldmap.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "maker.h"
#include "maker_api.h"

int Tilesets_MakerMetatileCount(const struct Tileset *tileset, const char **name);
const u16 *Maker_TilesetAttributes(const struct Tileset *ts);

#define AT_TERRAINS 48
#define AT_POOL 24          // terrains of the primary tileset (learned from all its maps)
#define AT_PAIR 14          // ... and of the pair's own (its secondary's tiles)
#define AT_MIN_CORE 6       // a centre surrounded by itself this often at least
#define AT_MANY 40          // samples of a kind of surroundings before its nearest shape is trusted

enum { KIND_INSIDE, KIND_OTHER, KIND_LAND, KIND_WATER, KIND_WALL, KIND_COUNT };

// bits of a shape: the 8 neighbours, NW N NE W E SW S SE
static const s8 sOffs[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

struct AtEntry { u16 key; u16 block[4]; u32 count[4]; };
struct AtFix { u16 key; s16 tile; };   // key: shape | phase << 8

struct AtTerrain
{
    char name[32];
    char centre[48];          // its identity in the user's file
    u8 px, py;                // the pattern's size (1 or 2)
    u16 q[4];                 // the centre tiles, row by row, at phase 0
    u8 off;                   // the most common phase offset in the maps (x + y * 2)
    u8 kind;                  // KIND_LAND / WATER / WALL
    bool8 pair;               // learned from the pair's own maps (has its secondary's tiles)
    bool8 user, hidden;
    u32 freq;                 // how often its centre is in the maps (the rarer owns the tiles both claim)
    u8 area[NUM_METATILES_TOTAL / 8];
    struct AtEntry *tab;
    u32 ntab, captab;
    struct AtFix fix[256];
    u16 nfix;
};

struct AtSet
{
    const struct Tileset *p, *s;
    u16 split;                // the secondary's first metatile
    char file[96];            // PRIMARY+SECONDARY
    int n;
    struct AtTerrain t[AT_TERRAINS];
    u8 order[AT_TERRAINS];    // rarest first
    u16 bits[NUM_METATILES_TOTAL];   // the collision / elevation the maps give each metatile most often
    u8 beh[NUM_METATILES_TOTAL];
};

static struct AtSet *sSets[8];
static int sNumSets;
static struct AtSet *sCur;    // the open map's

// ------------------------------------------------------------------------------------------------ the maps
struct Grid { const u16 *b; int w, h; bool8 samePair; };
static struct Grid *sGrids;
static int sNumGrids;

static void Corpus(const struct Tileset *p, const struct Tileset *s)
{
    int i, j, n = Maker_MapCount() - Maker_NewMapCount();
    free(sGrids);
    sGrids = calloc(n > 0 ? n : 1, sizeof(*sGrids));
    sNumGrids = 0;
    for (i = 0; i < n; i++)
    {
        const struct MakerMapName *m = Maker_Map(i);
        const struct MapHeader *h = m ? Maker_BaseMapHeader(m->mapGroup, m->mapNum) : NULL;
        const struct MapLayout *l = h ? h->mapLayout : NULL;
        if (l == NULL || l->map == NULL || l->primaryTileset != p || Maker_World3dOf(m->mapGroup) || l->width < 3 || l->height < 3)
            continue;
        for (j = 0; j < sNumGrids && sGrids[j].b != l->map; j++)
            ;
        if (j < sNumGrids)
            continue;   // (one layout, several maps)
        sGrids[sNumGrids].b = l->map;
        sGrids[sNumGrids].w = l->width;
        sGrids[sNumGrids].h = l->height;
        sGrids[sNumGrids].samePair = l->secondaryTileset == s;
        sNumGrids++;
    }
}

static inline u16 Id(const struct Grid *g, int x, int y)
{
    return g->b[y * g->w + x] & MAPGRID_METATILE_ID_MASK;
}

// ------------------------------------------------------------------------------------------------ shapes
static u8 Canon(u8 m)
{
    // a corner counts only where both sides beside it are the same terrain (RPG Maker's 47 shapes)
    if (!((m & 2) && (m & 8))) m &= ~1;
    if (!((m & 2) && (m & 16))) m &= ~4;
    if (!((m & 64) && (m & 8))) m &= ~32;
    if (!((m & 64) && (m & 16))) m &= ~128;
    return m;
}

static int ShapeDistance(u8 a, u8 b)
{
    u8 d = a ^ b;
    int i, n = 0;
    for (i = 0; i < 8; i++)
        if (d & (1 << i))
            n += (i == 1 || i == 3 || i == 4 || i == 6) ? 4 : 1;
    return n;
}

static bool8 InArea(const struct AtTerrain *t, u16 v)
{
    return v < NUM_METATILES_TOTAL && (t->area[v >> 3] & (1 << (v & 7)));
}

static void AddArea(struct AtTerrain *t, u16 v)
{
    if (v < NUM_METATILES_TOTAL)
        t->area[v >> 3] |= 1 << (v & 7);
}

static bool8 IsCentre(const struct AtTerrain *t, u16 v)
{
    int i;
    for (i = 0; i < t->px * t->py; i++)
        if (t->q[i] == v)
            return TRUE;
    return FALSE;
}

// where a tile is on its tileset's sheet: 8 a row (the secondary's from its own top)
static bool8 SheetNear(u16 a, u16 b, u16 split, int r)
{
    bool8 sa = a >= split, sb = b >= split;
    int wa = sa ? a - split : a, wb = sb ? b - split : b;
    return sa == sb && abs(wa / 8 - wb / 8) <= r && abs(wa % 8 - wb % 8) <= r;
}

// ------------------------------------------------------------------------------------------------ learning
struct Count { u32 *c; };

static int CmpPattern(const u16 *a, const u16 *b, int n)
{
    int i;
    for (i = 0; i < n; i++)
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    return 0;
}

struct Cand { u8 px, py; u16 q[4]; u32 n; };

static void AddCand(struct Cand **c, int *n, int *cap, u8 px, u8 py, const u16 *q, u32 k)
{
    int i, len = px * py;
    for (i = 0; i < *n; i++)
        if ((*c)[i].px == px && (*c)[i].py == py && CmpPattern((*c)[i].q, q, len) == 0)
        {
            (*c)[i].n += k;
            return;
        }
    if (*n == *cap)
    {
        *cap = *cap ? *cap * 2 : 256;
        *c = realloc(*c, *cap * sizeof(**c));
    }
    memset(&(*c)[*n], 0, sizeof(**c));
    (*c)[*n].px = px, (*c)[*n].py = py, (*c)[*n].n = k;
    memcpy((*c)[*n].q, q, len * sizeof(u16));
    (*n)++;
}

static int CmpCand(const void *a, const void *b)
{
    const struct Cand *x = a, *y = b;
    return x->n < y->n ? 1 : x->n > y->n ? -1 : 0;
}

// The centres of the maps: tiles surrounded by themselves, and 2-repeating patterns (trees). ONLY: ids below `lo`
// (the primary's) or (lo = 0) any with at least one at or past `need` (the pair's own)
static int FindCentres(struct Cand **out, u16 idmax, u16 need, bool8 pairOnly)
{
    struct Cand *c = NULL;
    int n = 0, cap = 0, gi, x, y, i, j;
    for (gi = 0; gi < sNumGrids; gi++)
    {
        const struct Grid *g = &sGrids[gi];
        if (pairOnly && !g->samePair)
            continue;
        for (y = 1; y < g->h - 1; y++)
            for (x = 1; x < g->w - 1; x++)
            {
                u16 v = Id(g, x, y);
                bool8 same = TRUE;
                for (i = 0; i < 8 && same; i++)
                    same = Id(g, x + sOffs[i][0], y + sOffs[i][1]) == v;
                if (same && v < idmax && (!pairOnly || v >= need))
                    AddCand(&c, &n, &cap, 1, 1, &v, 1);
            }
        for (y = 0; y + 3 < g->h; y++)
            for (x = 0; x + 3 < g->w; x++)
            {
                u16 q[4], best[4], t[4];
                u8 px, py;
                bool8 ok = TRUE, any = FALSE;
                int ox, oy;
                for (j = 0; j < 4 && ok; j++)
                    for (i = 0; i < 4 && ok; i++)
                        ok = Id(g, x + i, y + j) == Id(g, x + (i & 1), y + (j & 1));
                if (!ok)
                    continue;
                q[0] = Id(g, x, y), q[1] = Id(g, x + 1, y), q[2] = Id(g, x, y + 1), q[3] = Id(g, x + 1, y + 1);
                if (q[0] == q[1] && q[1] == q[2] && q[2] == q[3])
                    continue;
                for (i = 0; i < 4; i++)
                {
                    if (q[i] >= idmax)
                        ok = FALSE;
                    if (q[i] >= need)
                        any = TRUE;
                }
                if (!ok || (pairOnly && !any))
                    continue;
                if (q[0] == q[1] && q[2] == q[3])
                    px = 1, py = 2;
                else if (q[0] == q[2] && q[1] == q[3])
                    px = 2, py = 1;
                else
                    px = 2, py = 2;
                // (one pattern whatever phase it's met at: the smallest)
                for (oy = 0; oy < py; oy++)
                    for (ox = 0; ox < px; ox++)
                    {
                        int k = 0, ii, jj;
                        for (jj = 0; jj < py; jj++)
                            for (ii = 0; ii < px; ii++)
                                t[k++] = Id(g, x + ox + ii, y + oy + jj);
                        if ((ox == 0 && oy == 0) || CmpPattern(t, best, px * py) < 0)
                            memcpy(best, t, sizeof(t));
                    }
                AddCand(&c, &n, &cap, px, py, best, 1);
            }
    }
    j = 0;
    for (i = 0; i < n; i++)
        if (c[i].n >= AT_MIN_CORE)
            c[j++] = c[i];
    qsort(c, j, sizeof(*c), CmpCand);
    *out = c;
    return j;
}

static u8 PhaseOf(const struct AtTerrain *t, const u16 *blocks, int w, int h, int x, int y, u8 *offOut)
{
    u32 votes[4] = {0};
    int xx, yy, ox, oy, best = -1;
    u8 off;
    if (t->px == 1 && t->py == 1)
    {
        if (offOut)
            *offOut = 0;
        return 0;
    }
    for (yy = y - 2; yy <= y + 2; yy++)
        for (xx = x - 2; xx <= x + 2; xx++)
        {
            u16 v;
            if (xx < 0 || yy < 0 || xx >= w || yy >= h)
                continue;
            v = blocks[yy * w + xx] & MAPGRID_METATILE_ID_MASK;
            for (oy = 0; oy < t->py; oy++)
                for (ox = 0; ox < t->px; ox++)
                    if (t->q[(((yy - oy) % t->py + t->py) % t->py) * t->px + ((xx - ox) % t->px + t->px) % t->px] == v)
                        votes[ox + oy * 2]++;
        }
    for (ox = 0; ox < 4; ox++)
        if (votes[ox] && (best < 0 || votes[ox] > votes[best]))
            best = ox;
    off = best >= 0 ? best : t->off;
    if (offOut)
        *offOut = off;
    return (((x - (off & 1)) % t->px + t->px) % t->px) + ((((y - (off >> 1)) % t->py + t->py) % t->py) << 1);
}

static void Learn(struct AtSet *set);

// ------------------------------------------------------------------------------------------------ terrain of a cell
// A cell's terrain (the rarest whose tiles it is), -1 none. PAIR: the map uses the set's secondary (its own tiles count)
static int TerrainOf(const struct AtSet *set, u16 v, bool8 pair)
{
    int k;
    for (k = 0; k < set->n; k++)
    {
        const struct AtTerrain *t = &set->t[set->order[k]];
        if (t->hidden && !t->user)
            continue;
        if (!pair && v >= set->split)
            continue;   // (another secondary's tile: the same number is another picture)
        if (InArea(t, v))
            return set->order[k];
    }
    return -1;
}

static void Assign(const struct AtSet *set, const u16 *b, int w, int h, bool8 pair, s8 *ter)
{
    int i;
    for (i = 0; i < w * h; i++)
        ter[i] = TerrainOf(set, b[i] & MAPGRID_METATILE_ID_MASK, pair);
}

// Whether a neighbour of terrain U counts as terrain TI's own for TI's shape: the same terrain, or a rarer land on
// TI's land (a pale patch in the grass draws its own edges: to the grass it's no edge at all)
static bool8 SameFor(const struct AtSet *set, int ti, int u)
{
    return u == ti || (u >= 0 && ti >= 0 && set->t[u].kind == KIND_LAND && set->t[ti].kind == KIND_LAND
                       && set->t[u].freq < set->t[ti].freq);
}

static u8 ShapeOf(const struct AtSet *set, const s8 *ter, int w, int h, int x, int y, int ti)
{
    u8 m = 0;
    int i;
    for (i = 0; i < 8; i++)
    {
        int nx = x + sOffs[i][0], ny = y + sOffs[i][1];
        if (nx < 0 || ny < 0 || nx >= w || ny >= h || SameFor(set, ti, ter[ny * w + nx]))
            m |= 1 << i;   // (past the map's edge counts as the same: the map goes on)
    }
    return Canon(m);
}

static u8 KindAround(const struct AtSet *set, const s8 *ter, int w, int h, int x, int y, int ti)
{
    u8 c[KIND_COUNT] = {0}, best = KIND_INSIDE;
    int i, other = 0, most = 0;
    static const u8 sOrder[] = {KIND_WATER, KIND_WALL, KIND_LAND};   // (a tie: the more telling first)
    for (i = 0; i < 8; i++)
    {
        int nx = x + sOffs[i][0], ny = y + sOffs[i][1], u;
        if (nx < 0 || ny < 0 || nx >= w || ny >= h)
            continue;
        u = ter[ny * w + nx];
        if (SameFor(set, ti, u))
            continue;
        // (the four beside it count twice the corners: a pond's lip is under its water, whatever's at its sides)
        if (u < 0)
            other++;
        else
            c[set->t[u].kind] += (sOffs[i][0] == 0 || sOffs[i][1] == 0) ? 2 : 1;
    }
    for (i = 0; i < 3; i++)
        if (c[sOrder[i]] > most)
            most = c[sOrder[i]], best = sOrder[i];
    if (most == 0)
        best = other ? KIND_OTHER : KIND_INSIDE;
    return best;
}

static u16 Key(u8 shape, u8 phase, u8 kind)
{
    return shape | (phase << 8) | (kind << 10);
}

static struct AtEntry *Entry(struct AtTerrain *t, u16 key, bool8 add)
{
    u32 i;
    for (i = 0; i < t->ntab; i++)
        if (t->tab[i].key == key)
            return &t->tab[i];
    if (!add)
        return NULL;
    if (t->ntab == t->captab)
    {
        t->captab = t->captab ? t->captab * 2 : 64;
        t->tab = realloc(t->tab, t->captab * sizeof(*t->tab));
    }
    memset(&t->tab[t->ntab], 0, sizeof(*t->tab));
    t->tab[t->ntab].key = key;
    return &t->tab[t->ntab++];
}

static void Note(struct AtTerrain *t, u16 key, u16 block)
{
    struct AtEntry *e = Entry(t, key, TRUE);
    int i, low = 0;
    for (i = 0; i < 4; i++)
        if (e->count[i] && e->block[i] == block)
        {
            e->count[i]++;
            return;
        }
    for (i = 0; i < 4; i++)
        if (e->count[i] < e->count[low])
            low = i;
    if (e->count[low] == 0 || e->count[low] == 1)
        e->block[low] = block, e->count[low] = 1;
}

static u16 Best(const struct AtEntry *e, u32 *total)
{
    int i, b = 0;
    u32 sum = 0;
    for (i = 0; i < 4; i++)
    {
        sum += e->count[i];
        if (e->count[i] > e->count[b])
            b = i;
    }
    if (total)
        *total = sum;
    return e->block[b];
}

static void Learn(struct AtSet *set)
{
    struct Cand *cands = NULL;
    u32 *total = calloc(NUM_METATILES_TOTAL, sizeof(u32));
    u32 (*bits)[16] = calloc(NUM_METATILES_TOTAL, sizeof(*bits));
    int nc, i, k, gi, x, y, ring;
    const u16 *pa = Maker_TilesetAttributes(set->p), *sa = set->s ? Maker_TilesetAttributes(set->s) : NULL;
    for (i = 0; i < NUM_METATILES_TOTAL; i++)
    {
        u16 a = i < set->split ? (pa ? pa[i] : 0) : (sa ? sa[i - set->split] : 0);
        set->beh[i] = UNPACK_BEHAVIOR(a);
    }
    Corpus(set->p, set->s);
    for (gi = 0; gi < sNumGrids; gi++)
        for (i = 0; i < sGrids[gi].w * sGrids[gi].h; i++)
        {
            u16 b = sGrids[gi].b[i], v = b & MAPGRID_METATILE_ID_MASK;
            if (v >= set->split && !sGrids[gi].samePair)
                continue;
            total[v]++;
            bits[v][b >> 12]++;   // (collision 2 bits + elevation 4: the top 6 bits; elevation's 16 kept per collision 0)
        }
    for (i = 0; i < NUM_METATILES_TOTAL; i++)
    {
        int bestb = 0;
        for (k = 1; k < 16; k++)
            if (bits[i][k] > bits[i][bestb])
                bestb = k;
        set->bits[i] = bestb << 12;
    }
    // (the collision bits, separately: the most common for each tile)
    {
        u32 (*coll)[4] = calloc(NUM_METATILES_TOTAL, sizeof(*coll));
        for (gi = 0; gi < sNumGrids; gi++)
            for (i = 0; i < sGrids[gi].w * sGrids[gi].h; i++)
            {
                u16 b = sGrids[gi].b[i], v = b & MAPGRID_METATILE_ID_MASK;
                if (v < set->split || sGrids[gi].samePair)
                    coll[v][(b & MAPGRID_COLLISION_MASK) >> MAPGRID_COLLISION_SHIFT]++;
            }
        for (i = 0; i < NUM_METATILES_TOTAL; i++)
        {
            int bc = 0;
            for (k = 1; k < 4; k++)
                if (coll[i][k] > coll[i][bc])
                    bc = k;
            set->bits[i] = (set->bits[i] & ~MAPGRID_COLLISION_MASK) | (bc << MAPGRID_COLLISION_SHIFT);
        }
        free(coll);
    }
    // the terrains: the primary's from all its maps, then the pair's own
    set->n = 0;
    for (k = 0; k < 2; k++)
    {
        int limit = k == 0 ? AT_POOL : AT_PAIR;
        nc = FindCentres(&cands, k == 0 ? set->split : NUM_METATILES_TOTAL, set->split, k == 1);
        for (i = 0; i < nc && limit > 0 && set->n < AT_TERRAINS; i++)
        {
            struct AtTerrain *t;
            int j, u;
            bool8 taken = FALSE;
            // (a pattern made of another terrain's centre isn't a terrain: grass with flowers here and there)
            for (j = 0; j < cands[i].px * cands[i].py && cands[i].px * cands[i].py > 1; j++)
                for (u = 0; u < set->n; u++)
                    taken |= IsCentre(&set->t[u], cands[i].q[j]);
            if (taken)
                continue;
            limit--;
            t = &set->t[set->n++];
            memset(t, 0, sizeof(*t));
            t->px = cands[i].px, t->py = cands[i].py;
            memcpy(t->q, cands[i].q, sizeof(t->q));
            t->pair = k == 1;
            for (j = 0; j < t->px * t->py; j++)
            {
                AddArea(t, t->q[j]);
                t->freq += total[t->q[j]];
                snprintf(t->centre + strlen(t->centre), sizeof(t->centre) - strlen(t->centre), "%s0x%03X", j ? "+" : "", t->q[j]);
            }
            if (t->px * t->py == 2)
                snprintf(t->centre + strlen(t->centre), sizeof(t->centre) - strlen(t->centre), "/%dx%d", t->px, t->py);
        }
        free(cands);
        cands = NULL;
    }
    // each one's kind: water by its centre's behavior, a wall by the collision the maps give it
    for (i = 0; i < set->n; i++)
    {
        struct AtTerrain *t = &set->t[i];
        u16 c = t->q[0];
        t->kind = MetatileBehavior_IsSurfableWaterOrUnderwater(set->beh[c]) ? KIND_WATER
                : (set->bits[c] & MAPGRID_COLLISION_MASK) ? KIND_WALL : KIND_LAND;
    }
    // each one's pieces: beside it on the sheet, and beside it on the maps in a good part of their uses
    for (i = 0; i < set->n; i++)
    {
        struct AtTerrain *t = &set->t[i];
        u32 *near = calloc(NUM_METATILES_TOTAL, sizeof(u32));
        u16 idmax = t->pair ? NUM_METATILES_TOTAL : set->split;
        for (ring = 0; ring < 4; ring++)
        {
            bool8 grew = FALSE;
            u16 v;
            memset(near, 0, NUM_METATILES_TOTAL * sizeof(u32));
            for (gi = 0; gi < sNumGrids; gi++)
            {
                const struct Grid *g = &sGrids[gi];
                if (t->pair && !g->samePair)
                    continue;
                for (y = 0; y < g->h; y++)
                    for (x = 0; x < g->w; x++)
                    {
                        u16 c = Id(g, x, y);
                        int d;
                        if (InArea(t, c) || (c >= set->split && !g->samePair))
                            continue;
                        for (d = 0; d < 8; d++)
                        {
                            int nx = x + sOffs[d][0], ny = y + sOffs[d][1];
                            if (nx >= 0 && ny >= 0 && nx < g->w && ny < g->h && InArea(t, Id(g, nx, ny)))
                            {
                                near[c]++;
                                break;
                            }
                        }
                    }
            }
            {
                u16 *list = malloc(NUM_METATILES_TOTAL * sizeof(u16));
                int nl = 0, j2;
                for (v = 0; v < NUM_METATILES_TOTAL; v++)
                    if (InArea(t, v))
                        list[nl++] = v;
            for (v = 0; v < idmax; v++)
            {
                bool8 other = FALSE, sheet = FALSE, centreNear = FALSE;
                int j, u;
                if (near[v] < 2 || near[v] * 10 < total[v] * 4 || InArea(t, v))
                    continue;
                for (u = 0; u < set->n && !other; u++)
                    other = u != i && IsCentre(&set->t[u], v);
                if (other)
                    continue;
                for (j = 0; j < t->px * t->py; j++)
                    centreNear |= SheetNear(v, t->q[j], set->split, 3);
                for (j2 = 0; j2 < nl && !sheet && centreNear; j2++)
                    sheet = SheetNear(v, list[j2], set->split, 2);
                if (sheet)
                    AddArea(t, v), grew = TRUE;
            }
                free(list);
            }
            if (!grew)
                break;
        }
        free(near);
    }
    // the rarest first (a tile two claim is the rarer's: water owns its shore)
    for (i = 0; i < set->n; i++)
        set->order[i] = i;
    for (i = 1; i < set->n; i++)
        for (k = i; k > 0 && set->t[set->order[k]].freq < set->t[set->order[k - 1]].freq; k--)
        {
            u8 tmp = set->order[k];
            set->order[k] = set->order[k - 1], set->order[k - 1] = tmp;
        }
    // a tile in none that lies between two kinds (water above, grass below: a pond's lip) is the side it walks like
    {
        u32 (*between)[AT_TERRAINS] = calloc(NUM_METATILES_TOTAL, sizeof(*between));
        u32 *bn = calloc(NUM_METATILES_TOTAL, sizeof(u32));
        s8 *ter = NULL;
        int cap = 0;
        for (gi = 0; gi < sNumGrids; gi++)
        {
            const struct Grid *g = &sGrids[gi];
            if (g->w * g->h > cap)
                cap = g->w * g->h, ter = realloc(ter, cap);
            Assign(set, g->b, g->w, g->h, g->samePair, ter);
            for (y = 1; y < g->h - 1; y++)
                for (x = 1; x < g->w - 1; x++)
                {
                    int d;
                    u16 v = Id(g, x, y);
                    if (ter[y * g->w + x] >= 0 || (v >= set->split && !g->samePair))
                        continue;
                    for (d = 0; d < 2; d++)
                    {
                        int dx = d, dy = !d, a = ter[(y - dy) * g->w + x - dx], b = ter[(y + dy) * g->w + x + dx];
                        if (a >= 0 && b >= 0 && a != b && set->t[a].kind != set->t[b].kind)
                        {
                            between[v][a]++, between[v][b]++, bn[v]++;
                            break;
                        }
                    }
                }
        }
        for (i = 0; i < NUM_METATILES_TOTAL; i++)
        {
            int u, best = -1;
            bool8 bestSame = FALSE;
            if (bn[i] < 3 || bn[i] * 10 < total[i] * 3)
                continue;
            for (u = 0; u < set->n; u++)
            {
                bool8 same;
                if (!between[i][u])
                    continue;
                same = set->beh[set->t[u].q[0]] == set->beh[i];
                if (best < 0 || (same && !bestSame) || (same == bestSame && between[i][u] > between[i][best]))
                    best = u, bestSame = same;
            }
            if (best >= 0)
                AddArea(&set->t[best], i);
        }
        free(between);
        free(bn);
        // what each draws: by shape, phase and surroundings, from every map (and each pattern's usual phase)
        for (gi = 0; gi < sNumGrids; gi++)
        {
            const struct Grid *g = &sGrids[gi];
            if (g->w * g->h > cap)
                cap = g->w * g->h, ter = realloc(ter, cap);
            Assign(set, g->b, g->w, g->h, g->samePair, ter);
            for (y = 0; y < g->h; y++)
                for (x = 0; x < g->w; x++)
                {
                    int ti = ter[y * g->w + x];
                    struct AtTerrain *t;
                    u8 off;
                    if (ti < 0)
                        continue;
                    t = &set->t[ti];
                    Note(t, Key(ShapeOf(set, ter, g->w, g->h, x, y, ti), PhaseOf(t, g->b, g->w, g->h, x, y, &off),
                                KindAround(set, ter, g->w, g->h, x, y, ti)), g->b[y * g->w + x]);
                }
        }
        free(ter);
    }
    // each repeating pattern's usual phase (where the maps put a tree's left half)
    for (i = 0; i < set->n; i++)
    {
        struct AtTerrain *t = &set->t[i];
        u32 offs[4] = {0};
        int o, best = 0;
        if (t->px * t->py == 1)
            continue;
        for (gi = 0; gi < sNumGrids; gi++)
        {
            const struct Grid *g = &sGrids[gi];
            if (t->pair && !g->samePair)
                continue;
            for (y = 0; y < g->h; y++)
                for (x = 0; x < g->w; x++)
                    for (o = 0; o < 4; o++)
                    {
                        int ox = o & 1, oy = o >> 1;
                        if (ox >= t->px || oy >= t->py)
                            continue;
                        if (t->q[(((y - oy) % t->py + t->py) % t->py) * t->px + ((x - ox) % t->px + t->px) % t->px] == Id(g, x, y))
                            offs[o]++;
                    }
        }
        for (o = 1; o < 4; o++)
            if (offs[o] > offs[best])
                best = o;
        t->off = best;
    }
    // names: what its centre is in the game
    for (i = 0; i < set->n; i++)
    {
        struct AtTerrain *t = &set->t[i];
        u8 b = set->beh[t->q[0]];
        const char *n = t->px * t->py > 1 ? (t->kind == KIND_WALL ? "TREES" : "PATTERN")
                      : MetatileBehavior_IsTallGrass(b) || MetatileBehavior_IsLongGrass(b) ? "TALL GRASS"
                      : MetatileBehavior_IsDeepOrOceanWater(b) ? "SEA"
                      : t->kind == KIND_WATER ? "WATER"
                      : MetatileBehavior_IsSandOrDeepSand(b) ? "SAND"
                      : MetatileBehavior_IsIce(b) ? "ICE"
                      : MetatileBehavior_IsPuddle(b) ? "PUDDLE"
                      : t->kind == KIND_WALL ? "ROCK" : "GROUND";
        int same = 0, j;
        for (j = 0; j < i; j++)
            if (strncmp(set->t[j].name, n, strlen(n)) == 0)
                same++;
        if (same)
            snprintf(t->name, sizeof(t->name), "%s %d", n, same + 1);
        else
            snprintf(t->name, sizeof(t->name), "%s", n);
    }
    free(total);
    free(bits);
    free(sGrids);
    sGrids = NULL;
    sNumGrids = 0;
}

// ------------------------------------------------------------------------------------------------ the user's rules
static void FilePath(const struct AtSet *set, char *out, int size)
{
    snprintf(out, size, "%s/autotile/%s.txt", MakerApi_EditsDir(), set->file);
}

static struct AtTerrain *ByCentre(struct AtSet *set, const char *centre)
{
    int i;
    for (i = 0; i < set->n; i++)
        if (strcmp(set->t[i].centre, centre) == 0)
            return &set->t[i];
    return NULL;
}

static void SetFix(struct AtTerrain *t, u16 key, s16 tile)
{
    int i;
    for (i = 0; i < t->nfix && t->fix[i].key != key; i++)
        ;
    if (tile < 0)
    {
        if (i < t->nfix)
            t->fix[i] = t->fix[--t->nfix];
        return;
    }
    if (i == t->nfix)
    {
        if (t->nfix >= ARRAY_COUNT(t->fix))
            return;
        t->nfix++;
    }
    t->fix[i].key = key, t->fix[i].tile = tile;
    AddArea(t, tile);
}

static struct AtTerrain *NewTerrain(struct AtSet *set, const char *centre)
{
    struct AtTerrain *t;
    char buf[48], *p, *tok;
    int k = 0;
    if (set->n >= AT_TERRAINS)
        return NULL;
    t = &set->t[set->n];
    memset(t, 0, sizeof(*t));
    snprintf(t->centre, sizeof(t->centre), "%s", centre);
    snprintf(buf, sizeof(buf), "%s", centre);
    t->px = t->py = 1;
    if ((p = strchr(buf, '/')) != NULL)
    {
        sscanf(p + 1, "%hhux%hhu", &t->px, &t->py);
        *p = 0;
    }
    for (tok = strtok(buf, "+"); tok && k < 4; tok = strtok(NULL, "+"))
        t->q[k++] = strtol(tok, NULL, 0) & MAPGRID_METATILE_ID_MASK;
    if (k == 4)
        t->px = t->py = 2;
    else if (k < 1 || t->px * t->py != k)
        return NULL;
    for (k = 0; k < t->px * t->py; k++)
        AddArea(t, t->q[k]);
    t->user = TRUE;
    t->kind = (sCur && (sCur->bits[t->q[0]] & MAPGRID_COLLISION_MASK)) ? KIND_WALL : KIND_LAND;
    if (sCur && MetatileBehavior_IsSurfableWaterOrUnderwater(sCur->beh[t->q[0]]))
        t->kind = KIND_WATER;
    snprintf(t->name, sizeof(t->name), "MY TERRAIN %d", set->n + 1);
    memmove(set->order + 1, set->order, set->n);   // (the user's own first: a tile it shares is its)
    set->order[0] = set->n;
    set->n++;
    return t;
}

static void ReadRules(struct AtSet *set)
{
    char path[700], line[256];
    FILE *f;
    FilePath(set, path, sizeof(path));
    f = fopen(path, "r");
    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f))
    {
        char word[16] = "", centre[48] = "", rest[64] = "";
        struct AtTerrain *t;
        if (line[0] == '#' || sscanf(line, "%15s %47s %63[^\n]", word, centre, rest) < 2)
            continue;
        t = ByCentre(set, centre);
        if (strcmp(word, "terrain") == 0)
        {
            if (t == NULL)
                NewTerrain(set, centre);
            continue;
        }
        if (t == NULL)
            continue;
        if (strcmp(word, "name") == 0 && rest[0])
            snprintf(t->name, sizeof(t->name), "%s", rest);
        else if (strcmp(word, "hide") == 0)
            t->hidden = TRUE;
        else if (strcmp(word, "fix") == 0)
        {
            int shape = 0, phase = 0, tile = -1;
            if (sscanf(rest, "%i %i %i", &shape, &phase, &tile) == 3)
                SetFix(t, Canon(shape) | ((phase & 3) << 8), tile);
        }
    }
    fclose(f);
}

static void MakeDir(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static void WriteRules(struct AtSet *set)
{
    char path[700];
    FILE *f;
    int i, j;
    snprintf(path, sizeof(path), "%s", MakerApi_EditsDir());
    MakeDir(path);
    snprintf(path + strlen(path), sizeof(path) - strlen(path), "/autotile");
    MakeDir(path);
    FilePath(set, path, sizeof(path));
    f = fopen(path, "w");
    if (f == NULL)
        return;
    fprintf(f, "# Moemon Maker: autotile rules of %s (written by the editor; format at the top of src/maker/maker_autotile.c)\n", set->file);
    for (i = 0; i < set->n; i++)
    {
        struct AtTerrain *t = &set->t[i];
        if (t->user)
            fprintf(f, "terrain %s\n", t->centre);
        if (t->name[0] && (t->user || strncmp(t->name, "MY TERRAIN", 10) != 0))
            fprintf(f, "name %s %s\n", t->centre, t->name);
        if (t->hidden)
            fprintf(f, "hide %s\n", t->centre);
        for (j = 0; j < t->nfix; j++)
            fprintf(f, "fix %s %d %d 0x%03X\n", t->centre, t->fix[j].key & 0xFF, (t->fix[j].key >> 8) & 3, t->fix[j].tile);
    }
    fclose(f);
}

// ------------------------------------------------------------------------------------------------ the open map's set
static struct AtSet *SetFor(int map)
{
    const struct MakerMapName *m = Maker_Map(map);
    const struct MapHeader *h = m ? Maker_BaseMapHeader(m->mapGroup, m->mapNum) : NULL;
    const struct MapLayout *l = h ? h->mapLayout : NULL;
    const char *pn = "", *sn = "";
    int i;
    if (l == NULL || l->primaryTileset == NULL || Maker_World3dOf(m->mapGroup))
        return NULL;
    for (i = 0; i < sNumSets; i++)
        if (sSets[i]->p == l->primaryTileset && sSets[i]->s == l->secondaryTileset)
            return sSets[i];
    if (sNumSets == ARRAY_COUNT(sSets))
    {
        // (the oldest forgotten)
        int k;
        for (k = 0; k < sSets[0]->n; k++)
            free(sSets[0]->t[k].tab);
        free(sSets[0]);
        memmove(sSets, sSets + 1, sizeof(sSets[0]) * (--sNumSets));
    }
    sSets[sNumSets] = calloc(1, sizeof(struct AtSet));
    sSets[sNumSets]->p = l->primaryTileset;
    sSets[sNumSets]->s = l->secondaryTileset;
    sSets[sNumSets]->split = Fieldmap_PrimaryMetatiles(l);
    Tilesets_MakerMetatileCount(l->primaryTileset, &pn);
    if (l->secondaryTileset)
        Tilesets_MakerMetatileCount(l->secondaryTileset, &sn);
    snprintf(sSets[sNumSets]->file, sizeof(sSets[sNumSets]->file), "%s+%s", pn, sn);
    sCur = sSets[sNumSets];
    Learn(sSets[sNumSets]);
    ReadRules(sSets[sNumSets]);
    printf("[maker] autotile: %s, %d terrains learned\n", sSets[sNumSets]->file, sSets[sNumSets]->n);
    if (getenv("MOEMON_MAKER_ATDEBUG"))
        for (i = 0; i < sSets[sNumSets]->n; i++)
        {
            int k;
            printf("[maker] autotile %s (%s) %u shapes:", sSets[sNumSets]->t[i].name, sSets[sNumSets]->t[i].centre, sSets[sNumSets]->t[i].ntab);
            for (k = 0; k < NUM_METATILES_TOTAL; k++)
                if (InArea(&sSets[sNumSets]->t[i], k))
                    printf(" %03X", k);
            printf("\n");
            for (k = 0; k < (int)sSets[sNumSets]->t[i].ntab; k++)
            {
                struct AtEntry *e = &sSets[sNumSets]->t[i].tab[k];
                if ((e->key & 0xFF) == (getenv("MOEMON_MAKER_ATSHAPE") ? (u8)strtol(getenv("MOEMON_MAKER_ATSHAPE"), NULL, 0) : 0xF8))
                    printf("[maker]   shape phase %d kind %d: %04X x%u, %04X x%u, %04X x%u, %04X x%u\n", (e->key >> 8) & 3,
                           e->key >> 10, e->block[0], e->count[0], e->block[1], e->count[1], e->block[2], e->count[2], e->block[3], e->count[3]);
            }
        }
    return sSets[sNumSets++];
}

int MakerApi_AutotileLoad(int map)
{
    sCur = SetFor(map);
    return sCur ? sCur->n : 0;
}

static struct AtTerrain *Ter(int i)
{
    return sCur && i >= 0 && i < sCur->n ? &sCur->t[i] : NULL;
}

int MakerApi_AutotileTerrain(int i, struct MakerTerrain *out)
{
    struct AtTerrain *t = Ter(i);
    int k, n = 0;
    if (t == NULL)
        return 0;
    memset(out, 0, sizeof(*out));
    snprintf(out->name, sizeof(out->name), "%s", t->name);
    snprintf(out->centre, sizeof(out->centre), "%s", t->centre);
    out->centreTile = t->q[0];
    out->px = t->px, out->py = t->py;
    for (k = 0; k < 4; k++)
        out->q[k] = t->q[k];
    out->kind = t->kind == KIND_WATER ? 1 : t->kind == KIND_WALL ? 2 : 0;
    out->user = t->user, out->hidden = t->hidden;
    for (k = 0; k < NUM_METATILES_TOTAL; k++)
        n += InArea(t, k);
    out->tiles = n;
    out->fixes = t->nfix;
    return 1;
}

int MakerApi_AutotileArea(int i, int *ids, int max)
{
    struct AtTerrain *t = Ter(i);
    int k, n = 0;
    if (t == NULL)
        return 0;
    for (k = 0; k < NUM_METATILES_TOTAL; k++)
        if (InArea(t, k))
        {
            if (n < max)
                ids[n] = k;
            n++;
        }
    return n;
}

// An entry's block; a tie (or near one) goes to the block the terrain puts at that shape most against anything
static u16 BestOf(const struct AtTerrain *t, const struct AtEntry *e)
{
    u32 i, k, top = 0, bestScore = 0;
    u16 best = e->block[0];
    for (k = 0; k < 4; k++)
        if (e->count[k] > top)
            top = e->count[k];
    for (k = 0; k < 4; k++)
    {
        u32 score = 0;
        if (e->count[k] == 0 || e->count[k] * 4 < top * 3)
            continue;   // (only those near the most common)
        for (i = 0; i < t->ntab; i++)
            if ((t->tab[i].key & 0x3FF) == (e->key & 0x3FF))
            {
                int j;
                for (j = 0; j < 4; j++)
                    if (t->tab[i].count[j] && (t->tab[i].block[j] & MAPGRID_METATILE_ID_MASK) == (e->block[k] & MAPGRID_METATILE_ID_MASK))
                        score += t->tab[i].count[j];
            }
        score = score * 4 + e->count[k] * 8;
        if (score > bestScore)
            bestScore = score, best = e->block[k];
    }
    return best;
}

// What a terrain draws: a fix of the user's; the maps' block for that shape, phase and surroundings; the nearest
// shape the maps show against the same (when they show it often); else the centre at that phase. BLOCK: with the
// collision and elevation the maps give it
static u16 Pick(struct AtTerrain *t, u8 shape, u8 phase, u8 kind)
{
    u32 i, n, sum = 0;
    int bestd = 1 << 30;
    u16 bestb = 0xFFFF;
    struct AtEntry *e;
    shape = Canon(shape);
    for (i = 0; i < t->nfix; i++)
        if (t->fix[i].key == (shape | (phase << 8)))
            return t->fix[i].tile | (sCur ? (sCur->bits[t->fix[i].tile] & ~MAPGRID_METATILE_ID_MASK) : 0);
    if ((e = Entry(t, Key(shape, phase, kind), FALSE)) != NULL)
        return BestOf(t, e);
    if (kind == KIND_OTHER && (e = Entry(t, Key(shape, phase, KIND_LAND), FALSE)) != NULL)
        return BestOf(t, e);   // (only tiles of no terrain round it, a house: as if grass)
    if (kind == KIND_INSIDE || shape == 0xFF)
        if ((e = Entry(t, Key(0xFF, phase, KIND_INSIDE), FALSE)) != NULL)
            return BestOf(t, e);
    for (i = 0; i < t->ntab; i++)
        if ((t->tab[i].key >> 8 & 3) == phase && (t->tab[i].key >> 10) == kind)
        {
            Best(&t->tab[i], &n);
            sum += n;
        }
    if (sum >= AT_MANY)
        for (i = 0; i < t->ntab; i++)
            if ((t->tab[i].key >> 8 & 3) == phase && (t->tab[i].key >> 10) == kind)
            {
                int d = ShapeDistance(t->tab[i].key & 0xFF, shape);
                if (d < bestd)
                    bestd = d, bestb = Best(&t->tab[i], NULL);
            }
    // (a terrain of the user's: its nearest fix)
    if (t->user && bestb == 0xFFFF)
        for (i = 0; i < t->nfix; i++)
            if ((t->fix[i].key >> 8) == phase)
            {
                int d = ShapeDistance(t->fix[i].key & 0xFF, shape);
                if (d < bestd)
                    bestd = d, bestb = t->fix[i].tile | (sCur ? (sCur->bits[t->fix[i].tile] & ~MAPGRID_METATILE_ID_MASK) : 0);
            }
    if (bestb != 0xFFFF && (bestd <= 4 || t->user))
        return bestb;
    {
        u16 c = t->q[(phase & 1) + (phase >> 1) * t->px];
        return c | (sCur ? (sCur->bits[c] & ~MAPGRID_METATILE_ID_MASK) : 0);
    }
}

// The TERRAINS window: what terrain I draws for SHAPE at PHASE against land (its usual surroundings); FIXED: the
// user's fix there, or -1
int MakerApi_AutotilePiece(int i, int shape, int phase, int *fixed)
{
    struct AtTerrain *t = Ter(i);
    u32 k;
    if (fixed)
        *fixed = -1;
    if (t == NULL)
        return -1;
    for (k = 0; k < t->nfix; k++)
        if (t->fix[k].key == (Canon(shape) | ((phase & 3) << 8)) && fixed)
            *fixed = t->fix[k].tile;
    return Pick(t, shape, phase & 3, Canon(shape) == 0xFF ? KIND_INSIDE : KIND_LAND) & MAPGRID_METATILE_ID_MASK;
}

void MakerApi_AutotileSetFix(int i, int shape, int phase, int tile)
{
    struct AtTerrain *t = Ter(i);
    if (t == NULL)
        return;
    SetFix(t, Canon(shape) | ((phase & 3) << 8), tile);
    WriteRules(sCur);
}

void MakerApi_AutotileSetHidden(int i, int hidden)
{
    struct AtTerrain *t = Ter(i);
    if (t == NULL)
        return;
    t->hidden = hidden;
    WriteRules(sCur);
}

void MakerApi_AutotileRename(int i, const char *name)
{
    struct AtTerrain *t = Ter(i);
    if (t == NULL || name == NULL || !name[0])
        return;
    snprintf(t->name, sizeof(t->name), "%s", name);
    WriteRules(sCur);
}

// A terrain of the user's own: CENTRE's tiles (1, or 4 for a 2x2 pattern: row by row); its index, -1
int MakerApi_AutotileNew(const int *centre, int count)
{
    char key[48] = "";
    int k;
    struct AtTerrain *t;
    if (sCur == NULL || (count != 1 && count != 4))
        return -1;
    for (k = 0; k < count; k++)
        snprintf(key + strlen(key), sizeof(key) - strlen(key), "%s0x%03X", k ? "+" : "", centre[k] & MAPGRID_METATILE_ID_MASK);
    if ((t = ByCentre(sCur, key)) == NULL)
        t = NewTerrain(sCur, key);
    if (t == NULL)
        return -1;
    WriteRules(sCur);
    return (int)(t - sCur->t);
}

// RPG Maker's way: a terrain as a 3x3 block (corners, edges, centre) and four inner corners. A slot stands for every
// shape that draws it; setting a slot sets them all
// slots: 0 top-left 1 top 2 top-right 3 left 4 centre 5 right 6 bottom-left 7 bottom 8 bottom-right,
//        9 inner corner NW (grass in the NW corner only) 10 NE 11 SW 12 SE
static const u8 sSlotShape[13] = {0xD0, 0xF8, 0x68, 0xD6, 0xFF, 0x6B, 0x16, 0x1F, 0x0B, 0xFE, 0xFB, 0xDF, 0x7F};

static int SlotOf(u8 shape)
{
    bool8 n = !(shape & 2), w = !(shape & 8), e = !(shape & 16), s = !(shape & 64);
    if (n && w) return 0;
    if (n && e) return 2;
    if (s && w) return 6;
    if (s && e) return 8;
    if (n) return 1;
    if (s) return 7;
    if (w) return 3;
    if (e) return 5;
    if (!(shape & 1)) return 9;
    if (!(shape & 4)) return 10;
    if (!(shape & 32)) return 11;
    if (!(shape & 128)) return 12;
    return 4;
}

int MakerApi_AutotileSlotPiece(int i, int slot, int phase, int *fixed)
{
    return slot >= 0 && slot < 13 ? MakerApi_AutotilePiece(i, sSlotShape[slot], phase, fixed) : -1;
}

// Every shape the slot stands for drawn as TILE (-1: as learned again)
static void SetSlot(struct AtTerrain *t, int slot, int phase, int tile)
{
    int m;
    bool8 done[256] = {0};
    for (m = 0; m < 256; m++)
    {
        u8 c = Canon(m);
        if (!done[c] && SlotOf(c) == slot)
            SetFix(t, c | ((phase & 3) << 8), tile), done[c] = TRUE;
    }
}

void MakerApi_AutotileSetSlot(int i, int slot, int phase, int tile)
{
    struct AtTerrain *t = Ter(i);
    if (t == NULL || slot < 0 || slot >= 13)
        return;
    SetSlot(t, slot, phase, tile);
    WriteRules(sCur);
}

// A terrain of the user's own from its block on the tileset's sheet (8 a row): the 3x3 from TOPLEFT, and (INNER >= 0)
// the four inner corners as a 2x2 from INNER. Its index, -1
int MakerApi_AutotileNewBlock(int topLeft, int inner)
{
    int c = topLeft + 9, i, k;
    struct AtTerrain *t;
    if (sCur == NULL || topLeft < 0 || topLeft + 18 >= NUM_METATILES_TOTAL)
        return -1;
    i = MakerApi_AutotileNew(&c, 1);
    if ((t = Ter(i)) == NULL)
        return -1;
    for (k = 0; k < 9; k++)
        SetSlot(t, k, 0, topLeft + (k / 3) * 8 + k % 3);
    if (inner >= 0 && inner + 9 < NUM_METATILES_TOTAL)
        for (k = 0; k < 4; k++)
            SetSlot(t, 9 + k, 0, inner + (k / 2) * 8 + k % 2);
    WriteRules(sCur);
    return i;
}

// The inner corners of a terrain (the user's own, or a fix) as a 2x2 block from INNER on the sheet
void MakerApi_AutotileSetInner(int i, int inner)
{
    struct AtTerrain *t = Ter(i);
    int k;
    if (t == NULL || inner < 0 || inner + 9 >= NUM_METATILES_TOTAL)
        return;
    for (k = 0; k < 4; k++)
        SetSlot(t, 9 + k, 0, inner + (k / 2) * 8 + k % 2);
    WriteRules(sCur);
}

int MakerApi_AutotileShapes(int *shapes)
{
    int m, n = 0, k;
    for (m = 0; m < 256; m++)
    {
        u8 c = Canon(m);
        for (k = 0; k < n && shapes[k] != c; k++)
            ;
        if (k == n)
            shapes[n++] = c;
    }
    return n;
}

// Painting: terrain TERRAIN over CELLS (x, y pairs) of BLOCKS (W x H, the map as it is now). Every changed cell goes
// to OUT as x, y, block; the count (at most MAX)
int MakerApi_AutotilePaint(const int *blocks, int w, int h, const int *cells, int ncells, int terrain, int *out, int max)
{
    struct AtTerrain *t = Ter(terrain);
    u16 *b;
    s8 *ter, *before;
    u8 *painted, *dirty;
    u16 (*mapBits)[2];
    int i, x, y, n = 0;
    if (t == NULL || w <= 0 || h <= 0)
        return 0;
    b = malloc(w * h * sizeof(u16));
    ter = malloc(w * h);
    before = malloc(w * h);
    painted = calloc(w * h, 1);
    dirty = calloc(w * h, 1);
    mapBits = calloc(NUM_METATILES_TOTAL, sizeof(*mapBits));   // [0] the bits, [1] (non-zero: the map has the tile)
    for (i = 0; i < w * h; i++)
        b[i] = blocks[i] < 0 ? 0 : blocks[i];
    Assign(sCur, b, w, h, TRUE, ter);
    memcpy(before, ter, w * h);
    // (the map's own collision / elevation for a tile it already has: the most common)
    {
        u16 *cnt = calloc(NUM_METATILES_TOTAL * 64, sizeof(u16));
        for (i = 0; i < w * h; i++)
        {
            u16 v = b[i] & MAPGRID_METATILE_ID_MASK, k = b[i] >> 10;
            if (cnt[v * 64 + k] < 0xFFFF)
                cnt[v * 64 + k]++;
            if (cnt[v * 64 + k] > mapBits[v][1])
                mapBits[v][0] = b[i] & ~MAPGRID_METATILE_ID_MASK, mapBits[v][1] = cnt[v * 64 + k];
        }
        free(cnt);
    }
    for (i = 0; i < ncells; i++)
    {
        int cx = cells[i * 2], cy = cells[i * 2 + 1];
        if (cx < 0 || cy < 0 || cx >= w || cy >= h)
            continue;
        // (a repeating terrain is painted in whole units: whole trees)
        if (t->px > 1 || t->py > 1)
        {
            u8 ph = PhaseOf(t, b, w, h, cx, cy, NULL);
            int x0 = cx - (t->px > 1 ? (ph & 1) : 0), y0 = cy - (t->py > 1 ? (ph >> 1) : 0);
            for (y = y0; y < y0 + t->py; y++)
                for (x = x0; x < x0 + t->px; x++)
                    if (x >= 0 && y >= 0 && x < w && y < h)
                        painted[y * w + x] = 1;
        }
        else
            painted[cy * w + cx] = 1;
    }
    for (i = 0; i < w * h; i++)
        if (painted[i])
        {
            int k;
            ter[i] = terrain;
            x = i % w, y = i / w;
            for (k = -1; k < 8; k++)
            {
                int nx = x + (k < 0 ? 0 : sOffs[k][0]), ny = y + (k < 0 ? 0 : sOffs[k][1]);
                if (nx >= 0 && ny >= 0 && nx < w && ny < h)
                    dirty[ny * w + nx] = 1;
            }
        }
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            int u = ter[y * w + x], k = y * w + x;
            u16 v;
            if (!dirty[k] || u < 0)
                continue;
            if (!painted[k] && ShapeOf(sCur, ter, w, h, x, y, u) == ShapeOf(sCur, before, w, h, x, y, u)
             && KindAround(sCur, ter, w, h, x, y, u) == KindAround(sCur, before, w, h, x, y, u))
                continue;   // (the ring: only where its own shape changed)
            v = Pick(&sCur->t[u], ShapeOf(sCur, ter, w, h, x, y, u), PhaseOf(&sCur->t[u], b, w, h, x, y, NULL),
                     KindAround(sCur, ter, w, h, x, y, u));
            if (mapBits[v & MAPGRID_METATILE_ID_MASK][1])
                v = (v & MAPGRID_METATILE_ID_MASK) | mapBits[v & MAPGRID_METATILE_ID_MASK][0];
            if (v != b[k])
            {
                if (n < max)
                    out[n * 3] = x, out[n * 3 + 1] = y, out[n * 3 + 2] = v;
                n++;
            }
        }
    free(b);
    free(ter);
    free(before);
    free(painted);
    free(dirty);
    free(mapBits);
    return n < max ? n : max;
}

// Tests: how many of the map's terrain cells its terrains draw again the same (per mille), each drawn from its own
// shape on the map as it is
int MakerApi_AutotileRedraw(int map)
{
    const struct MakerMapName *m = Maker_Map(map);
    const struct MapHeader *h = m ? Maker_BaseMapHeader(m->mapGroup, m->mapNum) : NULL;
    const struct MapLayout *l = h ? h->mapLayout : NULL;
    s8 *ter;
    int x, y, same = 0, all = 0;
    if (l == NULL || (sCur = SetFor(map)) == NULL)
        return -1;
    ter = malloc(l->width * l->height);
    Assign(sCur, l->map, l->width, l->height, TRUE, ter);
    for (y = 0; y < l->height; y++)
        for (x = 0; x < l->width; x++)
        {
            int u = ter[y * l->width + x];
            u16 v;
            if (u < 0)
                continue;
            v = Pick(&sCur->t[u], ShapeOf(sCur, ter, l->width, l->height, x, y, u),
                     PhaseOf(&sCur->t[u], l->map, l->width, l->height, x, y, NULL),
                     KindAround(sCur, ter, l->width, l->height, x, y, u));
            all++;
            same += (v & MAPGRID_METATILE_ID_MASK) == (l->map[y * l->width + x] & MAPGRID_METATILE_ID_MASK);
        }
    free(ter);
    return all ? same * 1000 / all : -1;
}

// The user's rules were changed outside (the editor reloaded the edits): read again
void Maker_AutotileReload(void)
{
    int i, k;
    for (i = 0; i < sNumSets; i++)
    {
        for (k = 0; k < sSets[i]->n; k++)
            free(sSets[i]->t[k].tab);
        free(sSets[i]);
    }
    sNumSets = 0;
    sCur = NULL;
}
