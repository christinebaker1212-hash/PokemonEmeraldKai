#include "global.h"
#include "main.h"
#include "menu.h"
#include "bg.h"
#include "malloc.h"
#include "palette.h"
#include "pokedex_area_region_map.h"
#include "region_map.h"
#include "decompress.h"

static EWRAM_DATA u8 *sPokedexAreaMapBgNum = NULL;

static const u16 ALIGNED(4) sPokedexAreaMap_Pal[] = INCGFX_U16("graphics/pokedex/region_map.pal", ".gbapal");
static const u32 sPokedexAreaMap_Gfx[] = INCGFX_U32("graphics/pokedex/region_map.png", ".8bpp.lz", "-num_tiles 232 -Wnum_tiles");
static const u32 sPokedexAreaMap_Tilemap[] = INCGFX_U32("graphics/pokedex/region_map.bin", ".lz");
static const u32 sPokedexAreaMapAffine_Gfx[] = INCGFX_U32("graphics/pokedex/region_map_affine.png", ".8bpp.lz", "-num_tiles 233 -Wnum_tiles");
static const u32 sPokedexAreaMapAffine_Tilemap[] = INCGFX_U32("graphics/pokedex/region_map_affine.bin", ".lz");

void LoadPokedexAreaMapGfx(const struct PokedexAreaMapTemplate *template)
{
    u8 mode;
    void *tilemap;
    const u32 *qzGfx, *qzTilemap;
    const u16 *qzPal;
    sPokedexAreaMapBgNum = Alloc(sizeof(sPokedexAreaMapBgNum));
    mode = template->mode;

    if (mode == 0 && MP_RegionMap_AreaGfx(&qzGfx, &qzTilemap, &qzPal))
    {
        // Moemon Kai: JOHTO's / KANTO's town map art (its 64x64 affine map: the top-left 32x32 as a text map)
        static EWRAM_DATA u16 sTilemap[32 * 32];
        u8 *bytes = Alloc(64 * 64);
        u32 i;
        SetBgAttribute(template->bg, BG_ATTR_METRIC, 0);
        DecompressAndCopyTileDataToVram(template->bg, qzGfx, 0, template->offset, 0);
        LZ77UnCompWram(qzTilemap, bytes);
        for (i = 0; i < 32 * 32; i++)
            sTilemap[i] = bytes[(i / 32) * 64 + (i % 32)];
        Free(bytes);
        LoadBgTilemap(template->bg, sTilemap, sizeof(sTilemap), 0);
        ChangeBgX(template->bg, 0, BG_COORD_SET);
        ChangeBgY(template->bg, 0, BG_COORD_SET);
        SetBgAttribute(template->bg, BG_ATTR_PALETTEMODE, 1);
        CpuCopy32(qzPal, &gPlttBufferUnfaded[BG_PLTT_ID(7)], 3 * PLTT_SIZE_4BPP);
        *sPokedexAreaMapBgNum = template->bg;
        MP_RegionMap_AreaPicture(template->bg);   // (SINNOH's: its picture over this BG)
        return;
    }
    if (mode == 0)
    {
        SetBgAttribute(template->bg, BG_ATTR_METRIC, 0);
        DecompressAndCopyTileDataToVram(template->bg, sPokedexAreaMap_Gfx, 0, template->offset, 0);
        tilemap = DecompressAndCopyTileDataToVram(template->bg, sPokedexAreaMap_Tilemap, 0, 0, 1);
        AddValToTilemapBuffer(tilemap, template->offset, 32, 32, FALSE); // template->offset is always 0, so this does nothing.
    }
    else
    {
        // This is never reached, only a mode of 0 is given
        SetBgAttribute(template->bg, BG_ATTR_METRIC, 2);
        SetBgAttribute(template->bg, BG_ATTR_TYPE, BG_TYPE_AFFINE); // This does nothing. BG_ATTR_TYPE can't be set with this function
        DecompressAndCopyTileDataToVram(template->bg, sPokedexAreaMapAffine_Gfx, 0, template->offset, 0);
        tilemap = DecompressAndCopyTileDataToVram(template->bg, sPokedexAreaMapAffine_Tilemap, 0, 0, 1);
        AddValToTilemapBuffer(tilemap, template->offset, 64, 64, TRUE); // template->offset is always 0, so this does nothing.
    }

    ChangeBgX(template->bg, 0, BG_COORD_SET);
    ChangeBgY(template->bg, 0, BG_COORD_SET);
    SetBgAttribute(template->bg, BG_ATTR_PALETTEMODE, 1);
    CpuCopy32(sPokedexAreaMap_Pal, &gPlttBufferUnfaded[BG_PLTT_ID(7)], sizeof(sPokedexAreaMap_Pal));
    *sPokedexAreaMapBgNum = template->bg;
}

bool32 TryShowPokedexAreaMap(void)
{
    if (!FreeTempTileDataBuffersIfPossible())
    {
        ShowBg(*sPokedexAreaMapBgNum);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void FreePokedexAreaMapBgNum(void)
{
    TRY_FREE_AND_SET_NULL(sPokedexAreaMapBgNum);
}

void PokedexAreaMapChangeBgY(u32 move)
{
    ChangeBgY(*sPokedexAreaMapBgNum, move * 0x100, BG_COORD_SET);
}
