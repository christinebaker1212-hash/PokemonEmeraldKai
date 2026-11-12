// Moemon Maker: a map's tiles for the editor (include/maker_api.h). While the editor is open the game's screen is off,
// so a map's tilesets are loaded into BG VRAM exactly where the field puts them (tiles 0-1023, the primary first) and
// the game's own tile animations run on them (tileset_anims.c: same frames, same tiles, same timing). Metatiles are
// then drawn from those tiles and the map's 13 palettes as the field draws them (field_camera.c DrawMetatile: the
// bottom half under the top half; NORMAL metatiles over the field's filler tile 0x3014).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "fieldmap.h"
#include "overworld.h"
#include "decompress.h"
#include "tileset_anims.h"
#include "s3d.h"
#include "o3d.h"   // Moemon Kai: ORRE
#include "u3d.h"   // Moemon Kai: UNOVA
#include "metatile_behavior.h"
#include "maker.h"
#include "maker_api.h"
#include "maker.h"

int Tilesets_MakerMetatileCount(const struct Tileset *tileset, const char **name);   // tilesets.c
void TilesetAnims_InitForLayout(const struct MapLayout *layout);                    // tileset_anims.c
u8 TilesetAnims_Queued(void);

const u16 *Maker_TilesetMetatiles(const struct Tileset *ts);
const u16 *Maker_TilesetAttributes(const struct Tileset *ts);
static const struct MapLayout *sLayout;   // the editor's map's layout (its tiles are in BG VRAM)
static u8 *sTileBase;                     // where its tiles are: BG VRAM, or while the game plays (F5) a copy of our own
static u8 *sOwnTiles;
static u16 sPalettes[16][16];
static u8 sWorld3d;   // the map's 3D region (MAKER_REGION_SINNOH / UNOVA / ORRE), 0 otherwise

// The 3D region a map group belongs to (its blocks come from that engine's pack, behavior in the id), 0 for none
u8 Maker_World3dOf(u8 group)
{
    if (S3D_IsSinnohGroup(group))
        return MAKER_REGION_SINNOH;
    if (U3D_IsUnovaGroup(group))
        return MAKER_REGION_UNOVA;
    if (O3D_IsOrreGroup(group))
        return MAKER_REGION_ORRE;
    return 0;
}

// A map's blocks as the field has them: a 3D region's from its own engine's pack (each engine reads its own)
static const u16 *RegionBlocks(u8 group, const struct MapLayout *l)
{
    switch (Maker_World3dOf(group))
    {
    case MAKER_REGION_SINNOH: return S3D_LayoutBlocks(l);
    case MAKER_REGION_UNOVA:  return U3D_LayoutBlocks(l);
    case MAKER_REGION_ORRE:   return O3D_LayoutBlocks(l);
    }
    return l->map;
}
static bool8 sAnimating = TRUE;
static bool8 sTilesChanged;

static const struct MapHeader *Header(int index)
{
    if (index < 0 || index >= Maker_MapCount())
        return NULL;
    return Maker_BaseMapHeader(Maker_Map(index)->mapGroup, Maker_Map(index)->mapNum);
}

// A tileset's tiles into BG VRAM at tile `offset` (as fieldmap.c CopyTilesetToVram), never past `numTiles`
static void LoadTiles(const struct Tileset *t, u16 numTiles, u16 offset)
{
    u8 *dst = sTileBase + offset * TILE_SIZE_4BPP;
    u32 max = numTiles * TILE_SIZE_4BPP;
    if (t == NULL || t->tiles == NULL)
        return;
    if (!t->isCompressed)
        memcpy(dst, t->tiles, max);
    else
    {
        u32 size = ((const u32 *)t->tiles)[0] >> 8;
        u8 *buf = malloc(size + 64);
        if (!buf)
            return;
        LZ77UnCompWram(t->tiles, buf);
        memcpy(dst, buf, size < max ? size : max);
        free(buf);
    }
}

// The map's palettes as fieldmap.c loads them: the primary's first (color 0 black), the secondary's after
static void LoadPalettes(const struct MapLayout *l)
{
    u16 split = Fieldmap_PrimaryPals(l);
    u32 i;
    memset(sPalettes, 0, sizeof(sPalettes));
    for (i = 0; i < split && l->primaryTileset; i++)
        memcpy(sPalettes[i], l->primaryTileset->palettes[i], 32);
    sPalettes[0][0] = 0;
    for (i = split; i < NUM_PALS_TOTAL && l->secondaryTileset; i++)
        memcpy(sPalettes[i], l->secondaryTileset->palettes[i], 32);
}

int MakerApi_LoadMapTiles(int index, struct MakerTilesInfo *info)
{
    const struct MapHeader *h = Header(index);
    const struct MapLayout *l = h ? h->mapLayout : NULL;
    const char *name;
    memset(info, 0, sizeof(*info));
    if (l == NULL)
        return 0;
    sLayout = l;
    sWorld3d = Maker_World3dOf(Maker_Map(index)->mapGroup);
    // (while the game plays, its VRAM is its own: the tiles go into a copy, and don't animate)
    if (Maker_IsPlaying())
    {
        if (!sOwnTiles)
            sOwnTiles = malloc(NUM_TILES_TOTAL * TILE_SIZE_4BPP);
        sTileBase = sOwnTiles;
    }
    else
        sTileBase = (u8 *)BG_VRAM;
    LoadTiles(l->primaryTileset, Fieldmap_PrimaryTiles(l), 0);
    LoadTiles(l->secondaryTileset, NUM_TILES_TOTAL - Fieldmap_PrimaryTiles(l), Fieldmap_PrimaryTiles(l));
    LoadPalettes(l);
    if (!Maker_IsPlaying())
        TilesetAnims_InitForLayout(l);
    info->primaryMetatiles = Tilesets_MakerMetatileCount(l->primaryTileset, &name);
    snprintf(info->primaryName, sizeof(info->primaryName), "%s", name);
    info->secondaryMetatiles = Tilesets_MakerMetatileCount(l->secondaryTileset, &name);
    snprintf(info->secondaryName, sizeof(info->secondaryName), "%s", name);
    info->secondaryStart = Fieldmap_PrimaryMetatiles(l);
    if (info->primaryMetatiles > info->secondaryStart)
        info->primaryMetatiles = info->secondaryStart;
    if (info->secondaryMetatiles > NUM_METATILES_TOTAL - info->secondaryStart)
        info->secondaryMetatiles = NUM_METATILES_TOTAL - info->secondaryStart;
    info->world3d = sWorld3d;
    info->width = l->width;
    info->height = l->height;
    sTilesChanged = TRUE;
    return 1;
}

static const u16 *MetatileEntries(u16 id)
{
    u16 split;
    if (sLayout == NULL)
        return NULL;
    split = Fieldmap_PrimaryMetatiles(sLayout);
    if (id < split)
        return sLayout->primaryTileset ? &Maker_TilesetMetatiles(sLayout->primaryTileset)[id * 8] : NULL;
    if (id < NUM_METATILES_TOTAL && sLayout->secondaryTileset)
        return &Maker_TilesetMetatiles(sLayout->secondaryTileset)[(id - split) * 8];
    return NULL;
}

static u32 Rgba(u16 c)
{
    u32 r = (c & 31) * 255 / 31, g = ((c >> 5) & 31) * 255 / 31, b = ((c >> 10) & 31) * 255 / 31;
    return 0xFF000000u | (b << 16) | (g << 8) | r;   // (R, G, B, A bytes in memory)
}

// One 8x8 tile entry (tile | hflip | vflip | palette) at (ox, oy) of a 16x16 picture; color 0 left as it was
static void DrawTile(u32 *out, u16 entry, int ox, int oy, bool8 opaque)
{
    const u8 *tile = sTileBase + (entry & 0x3FF) * TILE_SIZE_4BPP;
    const u16 *pal = sPalettes[(entry >> 12) & 15];
    int x, y;
    for (y = 0; y < 8; y++)
        for (x = 0; x < 8; x++)
        {
            int sx = (entry & 0x400) ? 7 - x : x, sy = (entry & 0x800) ? 7 - y : y;
            u8 b = tile[sy * 4 + sx / 2];
            u8 c = (sx & 1) ? b >> 4 : b & 15;
            if (c || opaque)
                out[(oy + y) * 16 + ox + x] = Rgba(c ? pal[c] : sPalettes[0][0]);
        }
}

// (M5) one 8x8 tile entry as the loaded map's tiles show it (the TILESETS window's picker); 64 RGBA
void MakerApi_RenderTile(int entry, unsigned int *rgba)
{
    u32 buf[256];
    int y;
    if (sTileBase == NULL)
    {
        memset(rgba, 0, 64 * 4);
        return;
    }
    DrawTile(buf, (u16)entry, 0, 0, TRUE);
    for (y = 0; y < 8; y++)
        memcpy(rgba + y * 8, buf + y * 16, 8 * 4);
}

// (M5) the tileset edits changed: the editor's pictures drawn again
void Maker_TilesChanged(void)
{
    sTilesChanged = TRUE;
}

void MakerApi_RenderMetatile(int id, unsigned int *rgba, int layers)
{
    const u16 *e = MetatileEntries(id);
    u32 *out = (u32 *)rgba;
    int i, layerType = MakerApi_MetatileLayerType(id);
    for (i = 0; i < 256; i++)
        out[i] = 0xFF000000u;
    if (e == NULL)
        return;
    if (layers & 1)
    {
        // (the field's bottom layer: the filler tile under NORMAL metatiles, otherwise the backdrop)
        if (layerType == METATILE_LAYER_TYPE_NORMAL)
            for (i = 0; i < 4; i++)
                DrawTile(out, 0x3014, (i & 1) * 8, (i >> 1) * 8, TRUE);
        for (i = 0; i < 4; i++)
            DrawTile(out, e[i], (i & 1) * 8, (i >> 1) * 8, layerType != METATILE_LAYER_TYPE_NORMAL);
    }
    if (layers & 2)
        for (i = 0; i < 4; i++)
            DrawTile(out, e[4 + i], (i & 1) * 8, (i >> 1) * 8, FALSE);
}

static u16 Attributes(int id)
{
    u16 split;
    if (sLayout == NULL || id < 0)
        return 0;
    split = Fieldmap_PrimaryMetatiles(sLayout);
    if (id < split)
        return sLayout->primaryTileset ? Maker_TilesetAttributes(sLayout->primaryTileset)[id] : 0;
    if (id < NUM_METATILES_TOTAL && sLayout->secondaryTileset)
        return Maker_TilesetAttributes(sLayout->secondaryTileset)[id - split];
    return 0;
}

int MakerApi_MetatileBehavior(int id)
{
    // (a 3D region's blocks carry their behavior: fieldmap.c; all three engines from 0x300)
    if (sWorld3d == MAKER_REGION_SINNOH && id >= S3D_METATILE_BEHAVIOR)
        return id - S3D_METATILE_BEHAVIOR;
    if (sWorld3d == MAKER_REGION_UNOVA && id >= U3D_METATILE_BEHAVIOR)
        return id - U3D_METATILE_BEHAVIOR;
    if (sWorld3d == MAKER_REGION_ORRE && id >= O3D_METATILE_BEHAVIOR)
        return id - O3D_METATILE_BEHAVIOR;
    return UNPACK_BEHAVIOR(Attributes(id));
}

int MakerApi_MetatileLayerType(int id)
{
    return UNPACK_LAYER_TYPE(Attributes(id));
}

// The map's own block at x, y (metatile | collision << 10 | elevation << 12): a 3D region's from its engine's pack
int MakerApi_MapBlock(int index, int x, int y)
{
    const struct MapHeader *h = Header(index);
    const struct MapLayout *l = h ? h->mapLayout : NULL;
    const u16 *blocks;
    if (l == NULL || x < 0 || y < 0 || x >= l->width || y >= l->height)
        return -1;
    blocks = RegionBlocks(Maker_Map(index)->mapGroup, l);
    return blocks ? blocks[y * l->width + x] : -1;
}

// The game's tile animations, one step a game frame while the editor is open (maker_main.c CB2_MakerWaiting)
void Maker_TilesFrame(void)
{
    if (sLayout == NULL || !sAnimating || sTileBase != (u8 *)BG_VRAM)
        return;
    UpdateTilesetAnimations();
    if (TilesetAnims_Queued())
        sTilesChanged = TRUE;
    TransferTilesetAnimsBuffer();
}

void MakerApi_SetAnimating(int on)
{
    sAnimating = on != 0;
}

int MakerApi_TakeTilesChanged(void)
{
    int r = sTilesChanged;
    sTilesChanged = FALSE;
    return r;
}

void MakerApi_UnloadMapTiles(void)
{
    sLayout = NULL;
}

// What the MAP panel marks a tile as, by the game's own behavior checks (metatile_behavior.c), so the marks are always
// what the game does
int MakerApi_BehaviorClass(int b)
{
    if (MetatileBehavior_IsDoor(b) || MetatileBehavior_IsWarpDoor(b) || MetatileBehavior_IsNonAnimDoor(b))
        return MAKER_BEHAVIOR_DOOR;
    if (MetatileBehavior_IsEscalator(b) || MetatileBehavior_IsLadder(b) || MetatileBehavior_IsEastArrowWarp(b)
     || MetatileBehavior_IsWestArrowWarp(b) || MetatileBehavior_IsNorthArrowWarp(b) || MetatileBehavior_IsSouthArrowWarp(b)
     || MetatileBehavior_IsDeepSouthWarp(b) || MetatileBehavior_IsAquaHideoutWarp(b) || MetatileBehavior_IsLavaridgeB1FWarp(b)
     || MetatileBehavior_IsLavaridge1FWarp(b) || MetatileBehavior_IsMossdeepGymWarp(b) || MetatileBehavior_IsMtPyreHole(b))
        return MAKER_BEHAVIOR_WARP;
    if (MetatileBehavior_IsJumpEast(b))
        return MAKER_BEHAVIOR_LEDGE_E;
    if (MetatileBehavior_IsJumpWest(b))
        return MAKER_BEHAVIOR_LEDGE_W;
    if (MetatileBehavior_IsJumpNorth(b))
        return MAKER_BEHAVIOR_LEDGE_N;
    if (MetatileBehavior_IsJumpSouth(b))
        return MAKER_BEHAVIOR_LEDGE_S;
    if (MetatileBehavior_IsWaterfall(b))
        return MAKER_BEHAVIOR_WATERFALL;
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(b))
        return MAKER_BEHAVIOR_WATER;
    if (MetatileBehavior_IsLandWildEncounter(b))
        return MAKER_BEHAVIOR_GRASS;
    if (MetatileBehavior_IsIndoorEncounter(b) || MetatileBehavior_IsMountain(b))
        return MAKER_BEHAVIOR_CAVE;
    if (MetatileBehavior_IsIce(b) || MetatileBehavior_IsThinIce(b) || MetatileBehavior_IsCrackedIce(b))
        return MAKER_BEHAVIOR_ICE;
    if (MetatileBehavior_IsCounter(b))
        return MAKER_BEHAVIOR_COUNTER;
    return MAKER_BEHAVIOR_NONE;
}

// The 3D PREVIEW (a 3D region's map): its 3D world around a tile as the game shows it, as RGBA (240 x 160); 0 without
// the region's pack, or while the game plays (the 3D engine is the game's then). SINNOH's and ORRE's engines draw it at
// once; UNOVA's is the running game's own picture (maker_play.c Maker_PeekUnova: 2 while the game gets there)
// The helper Brendan (the user, 2026-11-12: "so passability in the 3D view can be a little more accurate to the
// cursor"): a small Brendan standing on the previewed tile, his feet on its middle, where the player would stand: the
// tile the moves and the walls go by, whatever the 3D picture of the place suggests. The editor's 3D PREVIEW turns him
// on and off (maker_mapview.cpp).
static bool8 sHelper3d = TRUE;
void MakerApi_Set3DHelper(int on)
{
    sHelper3d = on != 0;
}

#define HELPER_W 12   // (Brendan's 16 x 32 standing frame at three quarters: smaller than the player)
#define HELPER_H 24
static void DrawHelper(u16 *px, s32 cx, s32 cy)
{
    extern const u32 gObjectEventPic_BrendanNormal[];
    extern const u16 gObjectEventPal_Brendan[];
    const u8 *tiles = (const u8 *)gObjectEventPic_BrendanNormal;   // (frame 0: standing, facing down)
    s32 x, y, x0 = cx - HELPER_W / 2, y0 = cy + 3 - HELPER_H;
    for (y = 0; y < HELPER_H; y++)
        for (x = 0; x < HELPER_W; x++)
        {
            u32 bx = x * 16 / HELPER_W, by = y * 32 / HELPER_H;
            u8 b = tiles[((by / 8) * 2 + bx / 8) * 32 + (by % 8) * 4 + (bx % 8) / 2], c = bx & 1 ? b >> 4 : b & 15;
            if (c != 0 && x0 + x >= 0 && x0 + x < DISPLAY_WIDTH && y0 + y >= 0 && y0 + y < DISPLAY_HEIGHT)
                px[(y0 + y) * DISPLAY_WIDTH + x0 + x] = gObjectEventPal_Brendan[c];
        }
}

int MakerApi_Render3D(int map, int tileX, int tileY, unsigned int *rgba)
{
    void Maker_PeekTileSpot(s32 *x, s32 *y);
    s32 hx = DISPLAY_WIDTH / 2, hy = DISPLAY_HEIGHT / 2;   // (SINNOH's and ORRE's previews aim at the tile's middle)
    bool8 S3D_RenderPreview(u8 group, u8 num, s32 tileX, s32 tileY, u16 *out);
    int Maker_PeekUnova(int map, int x, int y, u16 *out);
    static u16 px[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    const struct MakerMapName *m = Maker_Map(map);
    int i, got;
    if (m == NULL)
        return 0;
    switch (Maker_World3dOf(m->mapGroup))
    {
    case MAKER_REGION_SINNOH:
        got = !Maker_IsPlaying() && S3D_RenderPreview(m->mapGroup, m->mapNum, tileX, tileY, px);
        break;
    case MAKER_REGION_ORRE:
        got = !Maker_IsPlaying() && O3D_RenderPreview(m->mapGroup, m->mapNum, tileX, tileY, px);
        break;
    case MAKER_REGION_UNOVA:
        got = Maker_PeekUnova(map, tileX, tileY, px);   // (the running game's own picture: 2 while on its way)
        break;
    default:
        got = 0;
    }
    if (got != 1)
        return got;
    if (sHelper3d)
    {
        if (Maker_World3dOf(m->mapGroup) == MAKER_REGION_UNOVA)
            Maker_PeekTileSpot(&hx, &hy);
        DrawHelper(px, hx, hy);
    }
    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
        rgba[i] = Rgba(px[i]);
    return 1;
}
