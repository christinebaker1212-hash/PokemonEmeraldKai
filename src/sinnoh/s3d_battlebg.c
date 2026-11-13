// Moemon Kai: SINNOH's battle backgrounds, Platinum's own (docs/SINNOH_HANDOFF.md): the background picture the map's
// Platinum header names (BACKGROUND_WATER while surfing), its time of day, and the two platforms the ground under the
// player gives (CalcTerrain: the tile's behaviour, else the background's own), drawn when the packs are built
// (tools/sinnoh/battle_bg.py, from pret/pokeplatinum: sinnoh/platinum_battle.pak beside the game, nothing of it in this
// repository) and fitted to Kai's battle screen as Emerald's own backgrounds are: BG3's tiles at BG_CHAR_ADDR(2), its
// map at BG_SCREEN_ADDR(26), three palettes at BG_PLTT_ID(2) (src/battle_bg.c calls S3D_BattleBg first). Visual only:
// Kai's battles run as they always do. Without the pack, or outside SINNOH, Kai's own backgrounds.
// Its own code, never UNOVA's or ORRE's.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "mp.h"
#include "mp_region.h"
#include "palette.h"
#include "rtc.h"
#include "s3d.h"
#include "constants/event_object_movement.h"

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

#define MAP_W 64
#define MAP_H 16
#define NO_TERRAIN 0xFE
#define PER_TILE 0xFF
#define BACKGROUND_WATER 1   // (Platinum's BattleBackground order: generated/battle_backgrounds.txt)

static struct
{
    bool8 tried, ok;
    u8 *raw;
    u32 size;
    u16 maps;
    u32 *mapAt;          // each map's record: u8 w, h, then its blocks
    u8 backgrounds;
    const u8 *bgInfo;    // per background: u8 its own terrain, u8 outdoor
    u16 scenes;
    u32 *sceneAt;
} sPb;

static u32 BlockBytes(const u8 *p)
{
    return p[1] == PER_TILE ? 2 + 1024 : 2;
}

static void Load(void)
{
    const char *path = getenv("MOEMON_S3D_BATTLE");
    FILE *f;
    long size;
    u8 *data;
    unsigned long rawSize;
    u32 o, i, n;
    sPb.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("sinnoh/platinum_battle.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || size < 12 || fread(data, 1, size, f) != (size_t)size || memcmp(data, "S3DB", 4) != 0
     || (data[4] | data[5] << 8) != 2)
    {
        fclose(f);
        free(data);
        printf("[s3d] %s isn't SINNOH's battle backgrounds\n", path);
        return;
    }
    fclose(f);
    rawSize = data[8] | data[9] << 8 | data[10] << 16 | (u32)data[11] << 24;
    sPb.raw = malloc(rawSize ? rawSize : 1);
    if (sPb.raw == NULL || uncompress(sPb.raw, &rawSize, data + 12, (unsigned long)(size - 12)) != 0)
    {
        free(data);
        free(sPb.raw);
        sPb.raw = NULL;
        return;
    }
    free(data);
    sPb.size = rawSize;
    sPb.maps = sPb.raw[0] | sPb.raw[1] << 8;
    sPb.mapAt = calloc(sPb.maps ? sPb.maps : 1, sizeof(u32));
    for (i = 0, o = 2; i < sPb.maps && o + 2 <= sPb.size; i++)
    {
        sPb.mapAt[i] = o;
        n = sPb.raw[o] * sPb.raw[o + 1];
        o += 2;
        while (n-- > 0 && o + 2 <= sPb.size)
            o += BlockBytes(sPb.raw + o);
    }
    if (i < sPb.maps || o + 1 > sPb.size)
        return;
    sPb.backgrounds = sPb.raw[o++];
    sPb.bgInfo = sPb.raw + o;
    o += sPb.backgrounds * 2;
    if (o + 2 > sPb.size)
        return;
    sPb.scenes = sPb.raw[o] | sPb.raw[o + 1] << 8;
    o += 2;
    sPb.sceneAt = calloc(sPb.scenes ? sPb.scenes : 1, sizeof(u32));
    for (i = 0; i < sPb.scenes && o + 4 <= sPb.size; i++)
    {
        u16 tiles = sPb.raw[o + 2] | sPb.raw[o + 3] << 8;
        sPb.sceneAt[i] = o;
        o += 4 + tiles * 32 + MAP_W * MAP_H * 2;
        if (o >= sPb.size)
            break;
        n = sPb.raw[o];
        o += 1 + n * 48 * 2;
        while (n-- > 0 && o + 4 <= sPb.size)   // (2: each set's picture in full colour)
            o += 4 + (sPb.raw[o] | sPb.raw[o + 1] << 8 | sPb.raw[o + 2] << 16 | (u32)sPb.raw[o + 3] << 24);
    }
    sPb.ok = i == sPb.scenes && o <= sPb.size;
    if (!sPb.ok)
        printf("[s3d] %s: cut short\n", path);
}

// Platinum's time for the background (BattleSystem_GetBackgroundTimeOffset, rtc.c's hours): 0 morning and day (4-16),
// 1 twilight (17-19), 2 night and late night
static u8 TimeOffset(void)
{
    u8 h;
    RtcCalcLocalTime();
    h = gLocalTime.hours;
    if (h >= 4 && h < 17)
        return 0;
    if (h >= 17 && h < 20)
        return 1;
    return 2;
}

// The scene now (*palette: its palette set), -1 for Kai's own
static s32 Choose(u8 *palette)
{
    s32 map = S3D_CurrentMapIndex(), i;
    const u8 *p;
    u8 bw, bh, bg, terrain, x, y;
    u32 block;
    if (map < 0 || MP_Region_Current() != REGION_SINNOH)
        return -1;
    if (!sPb.tried)
        Load();
    if (!sPb.ok || map >= sPb.maps)
        return -1;
    p = sPb.raw + sPb.mapAt[map];
    bw = p[0], bh = p[1];
    x = (u8)((gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET) / 32);
    y = (u8)((gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET) / 32);
    if (x >= bw)
        x = bw - 1;
    if (y >= bh)
        y = bh - 1;
    p += 2;
    for (block = 0; block < (u32)(y * bw + x); block++)
        p += BlockBytes(p);
    bg = p[0];
    terrain = p[1];
    if (terrain == PER_TILE)
    {
        s32 tx = (gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET) % 32;
        s32 ty = (gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET) % 32;
        terrain = p[2 + (ty < 0 ? 0 : ty) * 32 + (tx < 0 ? 0 : tx)];
    }
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
        bg = BACKGROUND_WATER;
    if (bg >= sPb.backgrounds)
        return -1;
    if (terrain == NO_TERRAIN)
        terrain = sPb.bgInfo[bg * 2];
    *palette = sPb.bgInfo[bg * 2 + 1] ? TimeOffset() : 0;
    for (i = 0; i < sPb.scenes; i++)
    {
        const u8 *s = sPb.raw + sPb.sceneAt[i];
        if (s[0] == bg && s[1] == terrain)
        {
            if (getenv("MOEMON_S3D_BATTLE_LOG"))
                printf("s3d battle bg: map %d background %u terrain %u time %u -> scene %d\n", map, bg, terrain,
                       *palette, i);
            return i;
        }
    }
    return -1;
}

// Battle background PART (S3D_BBG_*) drawn as Platinum's: TRUE when it was (src/battle_bg.c then skips Kai's own)
bool8 S3D_BattleBg(u8 part)
{
    u8 palette = 0;
    s32 sc = Choose(&palette);
    const u8 *rec, *map, *pals;
    u16 tiles;
    u32 x, y, i;
    if (sc < 0)
        return FALSE;
    rec = sPb.raw + sPb.sceneAt[sc];
    tiles = rec[2] | rec[3] << 8;
    map = rec + 4 + tiles * 32;
    pals = map + MAP_W * MAP_H * 2;
    if (palette >= pals[0])
        palette = 0;
    if (part == S3D_BBG_ALL || part == S3D_BBG_TILES)
        memcpy((void *)BG_CHAR_ADDR(2), rec + 4, tiles * 32);
    if (part == S3D_BBG_ALL || part == S3D_BBG_MAP)
    {
        // (64 x 32: the scene and the background going on, the last row again down to the bottom)
        u16 *dst = (u16 *)BG_SCREEN_ADDR(26);
        for (y = 0; y < 32; y++)
            for (x = 0; x < 64; x++)
            {
                u32 sy = y < MAP_H ? y : MAP_H - 1;
                u16 e = map[(sy * MAP_W + x) * 2] | map[(sy * MAP_W + x) * 2 + 1] << 8;
                dst[(x / 32) * 32 * 32 + y * 32 + x % 32] = (e & 0x0FFF) | ((((e >> 12) & 3) + 2) << 12);
            }
    }
    if (part == S3D_BBG_ALL || part == S3D_BBG_PALETTE)
    {
        static u16 sPix[512 * 128];
        u16 colors[48];
        const u8 *p = pals + 1 + palette * 48 * 2, *pic = pals + 1 + pals[0] * 48 * 2;
        unsigned long size = sizeof(sPix);
        for (i = 0; i < 48; i++)
            colors[i] = p[i * 2] | p[i * 2 + 1] << 8;
        LoadPalette(colors, BG_PLTT_ID(2), sizeof(colors));
        // (2: the picture in full colour, past the GBA's tiles and palettes: mp_battlepic.c)
        for (i = 0; i < palette; i++)
            pic += 4 + (pic[0] | pic[1] << 8 | pic[2] << 16 | (u32)pic[3] << 24);
        if (uncompress((unsigned char *)sPix, &size, pic + 4, pic[0] | pic[1] << 8 | pic[2] << 16 | (u32)pic[3] << 24) == 0
         && size == sizeof(sPix))
            MP_BattlePic_Show(sPix);
        else
            MP_BattlePic_Hide();
    }
    if (part == S3D_BBG_ENTRY)
    {
        // (the scene itself opens the battle: Kai's entry layer left empty, as UNOVA's)
        memset((void *)BG_CHAR_ADDR(1), 0, 32);
        memset((void *)BG_SCREEN_ADDR(28), 0, 0x1000);
    }
    return TRUE;
}
