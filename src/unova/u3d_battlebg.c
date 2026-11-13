// Moemon Kai: UNOVA's battle backgrounds, Black's own (docs/UNOVA_HANDOFF.md): each of Black's battle scenes (its field
// and the two stages the POKeMON stand on) drawn through Black's battle camera when the release is built
// (tools/unova/battle_bg.py, from the user's Black ROM: unova/black_battle.pak beside the game, nothing of it in this
// repository) and fitted to Kai's battle screen as Emerald's own backgrounds are: BG3's tiles at BG_CHAR_ADDR(2), its
// map at BG_SCREEN_ADDR(26), three palettes at BG_PLTT_ID(2) (src/battle_bg.c calls U3D_BattleBg first).
// Black picks the scene as FIELD_ENCOUNT_SetFieldSituation does: the zone's field effect set is the stage table's
// row, the ground under the player its column (the pack's tiles: u3d.c U3D_BattleColumnAt), the season its variant
// where the row has variants; a row with a light tints it by the light of the time (ZONEDATA_GetTimeLightColors: the
// area's light now, u3d.c U3D_LightColor). Without the pack, or outside UNOVA, Kai's own backgrounds.
// Its own code, never SINNOH's or ORRE's.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "gpu_regs.h"
#include "mp_region.h"
#include "overworld.h"
#include "palette.h"
#include "u3d.h"

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);

#define COLUMNS 17
#define MAP_W 64
#define MAP_H 16

static struct
{
    bool8 tried, ok;
    u8 *raw;
    u32 size;
    u16 zones;
    const u8 *zoneSet;
    u8 rows;
    const u8 *row;       // per row: u8 has light, u8 has variant, u16 background [17][4]
    u16 count;
    u32 *at;             // each background's record
} sBb;

static void Load(void)
{
    const char *path = getenv("MOEMON_U3D_BATTLE");
    FILE *f;
    long size;
    u8 *data;
    unsigned long rawSize;
    u32 o, i;
    sBb.tried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black_battle.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || size < 12 || fread(data, 1, size, f) != (size_t)size || memcmp(data, "U3DB", 4) != 0
     || (data[4] | data[5] << 8) != 3)
    {
        fclose(f);
        free(data);
        printf("[u3d] %s isn't UNOVA's battle backgrounds\n", path);
        return;
    }
    fclose(f);
    rawSize = data[8] | data[9] << 8 | data[10] << 16 | (u32)data[11] << 24;
    sBb.raw = malloc(rawSize ? rawSize : 1);
    if (sBb.raw == NULL || uncompress(sBb.raw, &rawSize, data + 12, (unsigned long)(size - 12)) != 0)
    {
        free(data);
        free(sBb.raw);
        sBb.raw = NULL;
        return;
    }
    free(data);
    sBb.size = rawSize;
    o = 0;
    sBb.zones = sBb.raw[0] | sBb.raw[1] << 8;
    sBb.zoneSet = sBb.raw + 2;
    o = 2 + sBb.zones;
    if (o + 1 > sBb.size)
        return;
    sBb.rows = sBb.raw[o++];
    sBb.row = sBb.raw + o;
    o += sBb.rows * (2 + COLUMNS * 4 * 2);
    if (o + 2 > sBb.size)
        return;
    sBb.count = sBb.raw[o] | sBb.raw[o + 1] << 8;
    o += 2;
    sBb.at = calloc(sBb.count ? sBb.count : 1, sizeof(u32));
    for (i = 0; i < sBb.count && o + 2 <= sBb.size; i++)
    {
        u16 tiles = sBb.raw[o] | sBb.raw[o + 1] << 8;
        sBb.at[i] = o;
        o += 2 + tiles * 32 + MAP_W * MAP_H * 2 + 48 * 2;
    }
    sBb.ok = i == sBb.count && o <= sBb.size;
    if (!sBb.ok)
        printf("[u3d] %s: cut short\n", path);
}

// The battle's background now (Black's scene for the zone, the ground under the player and the season), its row's
// light (*lit), or -1 for Kai's own
static s32 Choose(bool8 *lit)
{
    u16 zone;
    u8 set, column, variant;
    const u8 *row;
    s16 x, y;
    if (MP_Region_Current() != REGION_UNOVA || !U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup))
        return -1;
    if (!sBb.tried)
        Load();
    if (!sBb.ok)
        return -1;
    zone = U3D_CurrentZone();
    if (zone >= sBb.zones)
        return -1;
    set = sBb.zoneSet[zone];
    if (set >= sBb.rows)
        return -1;
    x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x;
    y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y;
    column = U3D_BattleColumnAt(x, y);
    if (column >= COLUMNS)
        column = 0;
    row = sBb.row + set * (2 + COLUMNS * 4 * 2);
    variant = row[1] ? (U3D_Season() & 3) : 0;
    *lit = row[0] != 0;
    {
        const u8 *p = row + 2 + (column * 4 + variant) * 2;
        u16 bg = p[0] | p[1] << 8;
        if (getenv("MOEMON_U3D_BATTLE_LOG"))
            printf("u3d battle bg: zone %u set %u column %u variant %u -> scene %u\n", zone, set, column, variant, bg);
        return bg < sBb.count ? bg : -1;
    }
}

static void Tiles(const u8 *rec)
{
    u16 tiles = rec[0] | rec[1] << 8;
    memcpy((void *)BG_CHAR_ADDR(2), rec + 2, tiles * 32);
}

static void Map(const u8 *rec)
{
    u16 tiles = rec[0] | rec[1] << 8;
    const u8 *map = rec + 2 + tiles * 32;
    u16 *dst = (u16 *)BG_SCREEN_ADDR(26);
    u32 x, y;
    // (64 x 32: the picture's 64 x 16 tiles, the scene and Black's field beyond it, as Kai's backgrounds keep their
    // second half free of platforms for the intro's slide; the last row again down to the bottom)
    for (y = 0; y < 32; y++)
        for (x = 0; x < 64; x++)
        {
            u32 sy = y < MAP_H ? y : MAP_H - 1;
            u16 e = map[(sy * MAP_W + x) * 2] | map[(sy * MAP_W + x) * 2 + 1] << 8;
            e = (e & 0x0FFF) | ((((e >> 12) & 3) + 2) << 12);   // (palettes 2-4)
            dst[(x / 32) * 32 * 32 + y * 32 + x % 32] = e;
        }
}

static void Palette(const u8 *rec, bool8 lit)
{
    u16 tiles = rec[0] | rec[1] << 8;
    const u8 *pal = rec + 2 + tiles * 32 + MAP_W * MAP_H * 2;
    u16 colors[48];
    u8 lc[3] = {31, 31, 31};
    u32 i;
    if (lit)
        U3D_LightColor(lc);
    for (i = 0; i < 48; i++)
    {
        u16 c = pal[i * 2] | pal[i * 2 + 1] << 8;
        u32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        r = r * (lc[0] + 1) / 32, g = g * (lc[1] + 1) / 32, b = b * (lc[2] + 1) / 32;
        colors[i] = r | g << 5 | b << 10;
    }
    LoadPalette(colors, BG_PLTT_ID(2), sizeof(colors));
}

// Battle background PART (U3D_BBG_*) drawn as Black's: TRUE when it was (src/battle_bg.c then skips Kai's own)
bool8 U3D_BattleBg(u8 part)
{
    bool8 lit = FALSE;
    s32 bg = Choose(&lit);
    const u8 *rec;
    if (bg < 0)
        return FALSE;
    rec = sBb.raw + sBb.at[bg];
    switch (part)
    {
    case U3D_BBG_ALL:
        Tiles(rec);
        Map(rec);
        Palette(rec, lit);
        break;
    case U3D_BBG_TILES:
        Tiles(rec);
        break;
    case U3D_BBG_MAP:
        Map(rec);
        break;
    case U3D_BBG_PALETTE:
        Palette(rec, lit);
        break;
    case U3D_BBG_ENTRY:
        // (Black's battles open on the scene itself: nothing slides over it, Kai's entry layer left empty)
        memset((void *)BG_CHAR_ADDR(1), 0, 32);
        memset((void *)BG_SCREEN_ADDR(28), 0, 0x1000);
        break;
    }
    return TRUE;
}
