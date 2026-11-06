#include <string.h>
#include "global.h"
#include "tilesets.h"
#include "tileset_anims.h"

#include "data/tilesets/graphics.h"
#include "data/tilesets/metatiles.h"
#include "data/tilesets/headers.h"
#include "data/tilesets/qz_graphics.h"   // Moemon Kai: Johto / Kanto (tools/quetzal_port/import.py)
#include "data/tilesets/qz_metatiles.h"
#include "data/tilesets/qz_headers.h"

// Moemon Maker: every tileset's metatile count (the editor's TILES panel; src/maker/maker_tiles.c), counted here where
// the arrays are defined. The table is made when the game is built (tools/maker/gen_names.py --tilesets).
#include "constants/maker_tilesets.h"
int Tilesets_MakerMetatileCount(const struct Tileset *tileset, const char **name)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMakerTilesetSizes); i++)
        if (sMakerTilesetSizes[i].tileset == tileset)
        {
            if (name)
                *name = sMakerTilesetSizes[i].name;
            return sMakerTilesetSizes[i].metatiles;
        }
    if (name)
        *name = "";
    return 0;
}

// Moemon Maker (M2b): a tileset by the name the TILES panel shows ("General"), for the user's new maps
const struct Tileset *Tilesets_MakerByName(const char *name)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMakerTilesetSizes); i++)
        if (strcmp(sMakerTilesetSizes[i].name, name) == 0)
            return sMakerTilesetSizes[i].tileset;
    return NULL;
}

int MakerApi_TilesetCount(void)
{
    return ARRAY_COUNT(sMakerTilesetSizes);
}

const char *MakerApi_TilesetName(int i, int *secondary)
{
    if (i < 0 || i >= (int)ARRAY_COUNT(sMakerTilesetSizes))
        return NULL;
    if (secondary)
        *secondary = sMakerTilesetSizes[i].tileset->isSecondary;
    return sMakerTilesetSizes[i].name;
}
