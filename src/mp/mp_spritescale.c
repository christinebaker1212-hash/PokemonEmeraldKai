// Moemon Co-op: followers' and the GARDEN's PMD pictures toward their POKeDEX height next to a trainer
// (include/mp_spritescale.h; OPTIONS > DISPLAY > SPRITE SCALING turns each on and off). Only ever smaller: a POKeMON as
// tall as a trainer or taller keeps its picture (ALAKAZAM, ONIX); a small one drawn big (PIKACHU, RAICHU, AZUMARILL)
// comes down, to just shorter than the player at the least.
#include <stdio.h>
#include "global.h"
#include "pokedex.h"
#include "mp_spritescale.h"
#include "mp_pmd.h"

u16 MP_SpriteScale_ForMon(u16 dexNum, u16 artHeightPx)
{
    u32 dm = GetPokedexHeightWeight(dexNum, 0), target, scale;
    if (dm == 0 || artHeightPx == 0 || dm >= MP_TRAINER_HEIGHT_DM)
        return 0;
    target = dm * MP_TRAINER_HEIGHT_PX / MP_TRAINER_HEIGHT_DM;
    if (target < MP_SCALE_MIN_PX)
        target = MP_SCALE_MIN_PX;
    if (target >= artHeightPx)
        return 0;   // (already drawn this small)
    scale = target * 256 / artHeightPx;
    return scale < 32 ? 32 : scale;
}

u16 MP_SpriteScale_TiledHeight(const u8 *tiles, u8 size)
{
    s32 x, y, top = -1, bottom = -1, tilesWide = size / 8;
    for (y = 0; y < size; y++)
        for (x = 0; x < size; x++)
        {
            u8 c = tiles[((y / 8) * tilesWide + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            if ((x & 1) ? c >> 4 : c & 15)
            {
                if (top < 0)
                    top = y;
                bottom = y;
                break;
            }
        }
    return top < 0 ? 0 : bottom - top + 1;
}

u16 MP_SpriteScale_BigHeight(const u8 *frame, u8 bpp)
{
    s32 w, h, x, y, top = -1, bottom = -1, stride;
    const u8 *px;
    if (frame == NULL)
        return 0;
    w = frame[4] | frame[5] << 8;
    h = frame[6] | frame[7] << 8;
    px = frame + 8;
    stride = bpp == 4 ? (w + 1) / 2 : w;
    for (y = 0; y < h; y++)
        for (x = 0; x < w; x++)
        {
            u8 c = bpp == 4 ? ((x & 1) ? px[y * stride + x / 2] >> 4 : px[y * stride + x / 2] & 15) : px[y * stride + x];
            if (c)
            {
                if (top < 0)
                    top = y;
                bottom = y;
                break;
            }
        }
    return top < 0 ? 0 : bottom - top + 1;
}

u16 MP_SpriteScale_Mon(bool8 on, u16 dex, const struct MpPmdView *pmd, const u8 *tiles, u8 size, u32 *key, u16 *value)
{
    u32 k = (u32)dex << 8 | (pmd != NULL) << 7 | size | 1u << 31;
    if (!on || dex == 0 || pmd == NULL)
        return 0;   // (only the PMD pictures: the game's own follower sprites keep their size)
    if (*key != k)
    {
        *key = k;
        *value = MP_SpriteScale_ForMon(dex, pmd != NULL ? MP_SpriteScale_BigHeight(pmd->big.frame, pmd->big.bpp)
                                                        : MP_SpriteScale_TiledHeight(tiles, size));
        printf("[mp] sprite scale: #%d %s height %d dm -> %d/256\n", dex, pmd != NULL ? "PMD" : "sprite",
               GetPokedexHeightWeight(dex, 0), *value);
    }
    return *value;
}
