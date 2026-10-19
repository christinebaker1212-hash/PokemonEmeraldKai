// Moemon Co-op: player appearance (base sprite, NPC sprites, skin tone, outfit
// colors, sprite packs) for the local player and the remote players.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO_WRAPPERS
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "third_party/stb_image.h"

#include "global.h"
#include "mp.h"
#include "mp_pack.h"
#include "mp_gfx_externs.h"
#include "data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "palette.h"
#include "sprite.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/rgb.h"
#include "constants/trainers.h"

#define OBJ_EVENT_PAL_TAG_BRENDAN           0x1100
#define OBJ_EVENT_PAL_TAG_MAY               0x1110
#define OBJ_EVENT_PAL_TAG_PLAYER_UNDERWATER 0x1115
#define OBJ_EVENT_PAL_TAG_RS_BRENDAN        0x1122
#define OBJ_EVENT_PAL_TAG_RS_MAY            0x1123
#define OBJ_EVENT_PAL_TAG_MP_LOCAL          0x1130
#define OBJ_EVENT_PAL_TAG_MP_PACK           0x1131
#define OBJ_EVENT_PAL_TAG_MP_PACK_DIVE      0x1132

#define PACK_DIR "sprites"
#define MAX_PACK_FILES 32

const u16 *MP_GetObjectEventPaletteData(u16 tag);
void MP_SetObjectEventPaletteData(u16 tag, const u16 *data);

extern const struct ObjectEventGraphicsInfo *const gObjectEventGraphicsInfoPointers[];

// Runtime trainer pictures from sprite packs (see include/data.h)
u32 gMpTrainerFrontPicLz[2][MP_PIC_LZ_WORDS];
u32 gMpTrainerFrontPalLz[2][MP_PAL_LZ_WORDS];
u32 gMpTrainerBackPalLz[MP_PAL_LZ_WORDS];
u8 gMpTrainerBackTiles[4 * TRAINER_PIC_SIZE];
u32 gMpPartnerBackPalLz[MP_PAL_LZ_WORDS];
u8 gMpPartnerBackTiles[4 * TRAINER_PIC_SIZE];
const struct SpriteFrameImage gMpPartnerBackFrames[4] = {
    {gMpPartnerBackTiles + 0 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpPartnerBackTiles + 1 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpPartnerBackTiles + 2 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpPartnerBackTiles + 3 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
};
const struct SpriteFrameImage gMpTrainerBackFrames[4] = {
    {gMpTrainerBackTiles + 0 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpTrainerBackTiles + 1 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpTrainerBackTiles + 2 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
    {gMpTrainerBackTiles + 3 * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE},
};

// ---------------------------------------------------------------------------
// Presets. For Brendan/May, skin replaces palette indices 1-3, outfit 12-13,
// accent 10-11. Other sprites are recolored automatically (see AutoRecolor).

static const u16 sSkinPresets[MP_SKIN_COUNT][3] = {
    {0, 0, 0},                                                  // default
    {RGB(31, 29, 27), RGB(30, 26, 23), RGB(27, 21, 18)},     // porcelain
    {RGB(31, 26, 22), RGB(31, 24, 18), RGB(27, 18, 14)},     // light
    {RGB(29, 22, 16), RGB(26, 19, 13), RGB(22, 14, 10)},     // tan
    {RGB(25, 17, 12), RGB(22, 14, 9),  RGB(17, 10, 7)},      // brown
    {RGB(19, 13, 9),  RGB(16, 10, 7),  RGB(12, 7, 5)},       // dark
    {RGB(14, 9, 7),   RGB(12, 7, 5),   RGB(9, 5, 4)},        // deep
};

static const u16 sColorPresets[MP_COLOR_COUNT][2] = {
    {0, 0},                                  // default
    {RGB(31, 12, 11), RGB(24, 8, 8)},      // red
    {RGB(11, 18, 31), RGB(6, 10, 25)},     // blue
    {RGB(13, 26, 13), RGB(6, 18, 7)},      // green
    {RGB(31, 27, 10), RGB(26, 20, 5)},     // yellow
    {RGB(23, 13, 31), RGB(16, 7, 24)},     // purple
    {RGB(31, 18, 25), RGB(26, 11, 18)},    // pink
    {RGB(31, 20, 8),  RGB(26, 13, 3)},     // orange
    {RGB(11, 11, 13), RGB(6, 6, 8)},       // black
    {RGB(30, 30, 30), RGB(23, 23, 25)},    // white
    {RGB(10, 26, 25), RGB(5, 18, 18)},     // teal
};

static const char *const sBaseNames[MP_BASE_COUNT] = {
    "DEFAULT", "BRENDAN", "MAY", "RS BRENDAN", "RS MAY", "CUSTOM", "NPC",
};
static const char *const sSkinNames[MP_SKIN_COUNT] = {
    "DEFAULT", "PORCELAIN", "LIGHT", "TAN", "BROWN", "DARK", "DEEP",
};
static const char *const sColorNames[MP_COLOR_COUNT] = {
    "DEFAULT", "RED", "BLUE", "GREEN", "YELLOW", "PURPLE", "PINK", "ORANGE", "BLACK", "WHITE", "TEAL",
};

#include "mp_sprite_catalog.h"

const char *MP_Avatar_BaseName(u8 base) { return base < MP_BASE_COUNT ? sBaseNames[base] : "?"; }
const char *MP_Avatar_SkinName(u8 skin) { return skin < MP_SKIN_COUNT ? sSkinNames[skin] : "?"; }
const char *MP_Avatar_ColorName(u8 c) { return c < MP_COLOR_COUNT ? sColorNames[c] : "?"; }

const struct MpSpriteEntry *MP_Avatar_FindCatalog(u16 gfxId)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sSpriteCatalog); i++)
        if (sSpriteCatalog[i].gfxId == gfxId)
            return &sSpriteCatalog[i];
    return NULL;
}

static const struct ObjectEventGraphicsInfo *NpcInfo(u16 gfxId)
{
    if (MP_Avatar_FindCatalog(gfxId) == NULL)
        return NULL;
    return gObjectEventGraphicsInfoPointers[gfxId];
}

// ---------------------------------------------------------------------------
// Local sprite pack and the pack files in the "sprites" folder

static u8 *sLocalPackBlob;
static u32 sLocalPackSize;
static struct MpPack sLocalPack;
static bool8 sLocalHasPack;
static char sPackFiles[MAX_PACK_FILES][160];
static char sPackNames[MAX_PACK_FILES][24];
static u8 sPackFileCount;
static u32 sLocalVersion;      // bumped when the local look changes
static bool8 sAvatarInit;

static void RebuildLocal(void);

static bool8 HasPngExtension(const char *name)
{
    size_t n = strlen(name);
    return n > 4 && (strcmp(name + n - 4, ".png") == 0 || strcmp(name + n - 4, ".PNG") == 0);
}

static void MakePackName(const char *path, char *out, int len)
{
    const char *base = path, *p;
    int i = 0;
    for (p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            base = p + 1;
    out[i++] = '*';
    out[i++] = ' ';
    for (p = base; *p && *p != '.' && i < len - 1; p++)
        out[i++] = (*p == '_') ? ' ' : toupper((unsigned char)*p);
    out[i] = 0;
}

void MP_Avatar_RefreshPackList(void)
{
    DIR *d = opendir(PACK_DIR);
    struct dirent *e;
    sPackFileCount = 0;
    if (d)
    {
        while ((e = readdir(d)) != NULL && sPackFileCount < MAX_PACK_FILES)
        {
            if (e->d_name[0] == '.' || !HasPngExtension(e->d_name))
                continue;
            snprintf(sPackFiles[sPackFileCount], sizeof(sPackFiles[0]), "%s/%s", PACK_DIR, e->d_name);
            MakePackName(e->d_name, sPackNames[sPackFileCount], sizeof(sPackNames[0]));
            sPackFileCount++;
        }
        closedir(d);
    }
    // A pack set in the ini outside the folder still shows up
    if (gMpConfig.customSprite[0] && sPackFileCount < MAX_PACK_FILES)
    {
        u8 i;
        for (i = 0; i < sPackFileCount; i++)
            if (strcmp(sPackFiles[i], gMpConfig.customSprite) == 0)
                break;
        if (i == sPackFileCount)
        {
            snprintf(sPackFiles[i], sizeof(sPackFiles[0]), "%s", gMpConfig.customSprite);
            MakePackName(gMpConfig.customSprite, sPackNames[i], sizeof(sPackNames[0]));
            sPackFileCount++;
        }
    }
}

static void UnloadLocalPack(void)
{
    free(sLocalPackBlob);
    sLocalPackBlob = NULL;
    sLocalPackSize = 0;
    sLocalHasPack = FALSE;
    memset(&sLocalPack, 0, sizeof(sLocalPack));
}

static bool8 LoadLocalPack(const char *path, char *err, int errLen)
{
    u32 size;
    u8 *blob = MP_Pack_LoadFile(path, &size, err, errLen);
    struct MpPack pack;
    if (!blob)
        return FALSE;
    if (!MP_Pack_Parse(blob, size, &pack))
    {
        free(blob);
        snprintf(err, errLen, "%s could not be read", path);
        return FALSE;
    }
    UnloadLocalPack();
    sLocalPackBlob = blob;
    sLocalPackSize = size;
    sLocalPack = pack;
    sLocalHasPack = TRUE;
    return TRUE;
}

static bool8 LocalPackActive(void)
{
    return sLocalHasPack && gMpConfig.app.hasCustom;
}

const u8 *MP_Avatar_LocalCustomBlob(void)
{
    return LocalPackActive() ? sLocalPackBlob : NULL;
}

u32 MP_Avatar_LocalCustomBlobSize(void)
{
    return LocalPackActive() ? sLocalPackSize : 0;
}

bool8 MP_Avatar_IsValidBlob(const u8 *data, u32 size)
{
    struct MpPack pack;
    return MP_Pack_Parse(data, size, &pack);
}

static const struct MpPack *LocalPack(void)
{
    return LocalPackActive() ? &sLocalPack : NULL;
}

// Parsed pack of a remote player, or NULL
static const struct MpPack *RemotePack(u8 id, struct MpPack *buf)
{
    struct MpPlayer *p = &gMpPlayers[id];
    if (!p->app.hasCustom || !p->customGfx || !MP_Pack_Parse(p->customGfx, p->customGfxSize, buf))
        return NULL;
    return buf;
}

// ---------------------------------------------------------------------------
// Sprite choices (CO-OP menu > SPRITE): bases, catalog sprites, then packs

#define NUM_BASE_CHOICES MP_BASE_CUSTOM   // DEFAULT, BRENDAN, MAY, RS BRENDAN, RS MAY

u16 MP_Avatar_ChoiceCount(void)
{
    return NUM_BASE_CHOICES + ARRAY_COUNT(sSpriteCatalog) + sPackFileCount;
}

u16 MP_Avatar_GetChoice(const struct MpAppearance *app)
{
    u16 i;
    if (app->hasCustom && sLocalHasPack)
    {
        for (i = 0; i < sPackFileCount; i++)
            if (strcmp(sPackFiles[i], gMpConfig.customSprite) == 0)
                return NUM_BASE_CHOICES + ARRAY_COUNT(sSpriteCatalog) + i;
    }
    if (app->base == MP_BASE_NPC)
    {
        for (i = 0; i < ARRAY_COUNT(sSpriteCatalog); i++)
            if (sSpriteCatalog[i].gfxId == app->gfxId)
                return NUM_BASE_CHOICES + i;
        return 0;
    }
    return app->base < NUM_BASE_CHOICES ? app->base : 0;
}

void MP_Avatar_SetChoice(struct MpAppearance *app, u16 choice)
{
    if (choice < NUM_BASE_CHOICES)
    {
        app->base = (u8)choice;
        app->gfxId = 0;
        app->hasCustom = FALSE;
    }
    else if (choice < NUM_BASE_CHOICES + ARRAY_COUNT(sSpriteCatalog))
    {
        app->base = MP_BASE_NPC;
        app->gfxId = sSpriteCatalog[choice - NUM_BASE_CHOICES].gfxId;
        app->hasCustom = FALSE;
    }
    else if (choice - NUM_BASE_CHOICES - ARRAY_COUNT(sSpriteCatalog) < sPackFileCount)
    {
        const char *path = sPackFiles[choice - NUM_BASE_CHOICES - ARRAY_COUNT(sSpriteCatalog)];
        char err[160];
        if (LoadLocalPack(path, err, sizeof(err)))
        {
            snprintf(gMpConfig.customSprite, sizeof(gMpConfig.customSprite), "%s", path);
            app->base = sLocalPack.base == MP_BASE_CUSTOM ? MP_BASE_DEFAULT : sLocalPack.base;
            app->gfxId = sLocalPack.gfxId;
            app->hasCustom = TRUE;
        }
        else
        {
            printf("[mp] sprite pack not loaded: %s\n", err);
        }
    }
    if (!app->hasCustom)
        gMpConfig.customSprite[0] = 0;
    sLocalVersion++;
}

const char *MP_Avatar_ChoiceName(u16 choice)
{
    if (choice < NUM_BASE_CHOICES)
        return sBaseNames[choice];
    if (choice < NUM_BASE_CHOICES + ARRAY_COUNT(sSpriteCatalog))
        return sSpriteCatalog[choice - NUM_BASE_CHOICES].name;
    if (choice - NUM_BASE_CHOICES - ARRAY_COUNT(sSpriteCatalog) < sPackFileCount)
        return sPackNames[choice - NUM_BASE_CHOICES - ARRAY_COUNT(sSpriteCatalog)];
    return "?";
}

static void ToConfigName(char *dst, const char *src, int len)
{
    int i;
    for (i = 0; src[i] && i < len - 1; i++)
    {
        char c = src[i];
        dst[i] = (c == ' ') ? '_' : (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
    }
    dst[i] = 0;
}

void MP_Avatar_SpriteConfigName(const struct MpAppearance *app, char *out, int len)
{
    if (app->base == MP_BASE_NPC && MP_Avatar_FindCatalog(app->gfxId))
        ToConfigName(out, MP_Avatar_FindCatalog(app->gfxId)->name, len);
    else
        ToConfigName(out, MP_Avatar_BaseName(app->base >= MP_BASE_CUSTOM ? MP_BASE_DEFAULT : app->base), len);
}

int MP_Avatar_ParseSpriteName(const char *name, struct MpAppearance *app)
{
    char buf[40];
    u16 i;
    for (i = 0; i < MP_BASE_NPC; i++)
    {
        ToConfigName(buf, sBaseNames[i], sizeof(buf));
        if (strcmp(buf, name) == 0)
        {
            app->base = (i == MP_BASE_CUSTOM) ? MP_BASE_DEFAULT : (u8)i;
            app->gfxId = 0;
            return TRUE;
        }
    }
    for (i = 0; i < ARRAY_COUNT(sSpriteCatalog); i++)
    {
        ToConfigName(buf, sSpriteCatalog[i].name, sizeof(buf));
        if (strcmp(buf, name) == 0)
        {
            app->base = MP_BASE_NPC;
            app->gfxId = sSpriteCatalog[i].gfxId;
            return TRUE;
        }
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
// Automatic recolor for sprites whose palettes don't follow the Brendan/May
// layout (NPCs, custom sprites). Skin tones are found by hue/saturation and
// replaced shade by shade; the most used clothing hue becomes the OUTFIT color
// and the second one the ACCENT color, keeping each color's shading.

static void RgbToHsv(u16 c, int *h, int *s, int *v)
{
    int r = (c & 0x1F) * 255 / 31, g = ((c >> 5) & 0x1F) * 255 / 31, b = ((c >> 10) & 0x1F) * 255 / 31;
    int max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    int min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    int d = max - min;
    *v = max;
    *s = max ? d * 255 / max : 0;
    if (d == 0)
        *h = 0;
    else if (max == r)
        *h = (60 * (g - b) / d + 360) % 360;
    else if (max == g)
        *h = 60 * (b - r) / d + 120;
    else
        *h = 60 * (r - g) / d + 240;
}

static u16 HsvToRgb(int h, int s, int v)
{
    int r, g, b, f, p, q, t;
    if (s < 0) s = 0; if (s > 255) s = 255;
    if (v < 0) v = 0; if (v > 255) v = 255;
    h = ((h % 360) + 360) % 360;
    f = (h % 60) * 255 / 60;
    p = v * (255 - s) / 255;
    q = v * (255 - s * f / 255) / 255;
    t = v * (255 - s * (255 - f) / 255) / 255;
    switch (h / 60)
    {
    case 0:  r = v; g = t; b = p; break;
    case 1:  r = q; g = v; b = p; break;
    case 2:  r = p; g = v; b = t; break;
    case 3:  r = p; g = q; b = v; break;
    case 4:  r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
    }
    return RGB(r * 31 / 255, g * 31 / 255, b * 31 / 255);
}

static void CountTileUsage(const u8 *tiles, u32 bytes, u32 counts[16])
{
    u32 i;
    for (i = 0; i < bytes; i++)
    {
        counts[tiles[i] & 0xF]++;
        counts[tiles[i] >> 4]++;
    }
}

// Counts colors in rows [y0, y1) of one 4bpp frame laid out as wTiles x hTiles tiles
static void CountRegionUsage(const u8 *tiles, int wTiles, int hTiles, int y0, int y1, u32 counts[16])
{
    int ty, tx, py, px;
    for (ty = 0; ty < hTiles; ty++)
        for (tx = 0; tx < wTiles; tx++)
        {
            const u8 *tile = tiles + (ty * wTiles + tx) * 32;
            for (py = 0; py < 8; py++)
            {
                int y = ty * 8 + py;
                if (y < y0 || y >= y1)
                    continue;
                for (px = 0; px < 8; px++)
                {
                    u8 b = tile[(py * 8 + px) / 2];
                    counts[(px & 1) ? (b >> 4) : (b & 0xF)]++;
                }
            }
        }
}

// Finds the first and last rows with visible pixels in one frame
static void VisibleRows(const u8 *tiles, int wTiles, int hTiles, int *top, int *bottom)
{
    int y;
    *top = -1;
    *bottom = -1;
    for (y = 0; y < hTiles * 8; y++)
    {
        u32 row[16] = {0}, i, any = 0;
        CountRegionUsage(tiles, wTiles, hTiles, y, y + 1, row);
        for (i = 1; i < 16; i++)
            any |= row[i];
        if (any)
        {
            if (*top < 0)
                *top = y;
            *bottom = y;
        }
    }
    if (*top < 0)
    {
        *top = 0;
        *bottom = hTiles * 8 - 1;
    }
}

// Head (hair/hat) and body (clothes) color usage of a front-facing frame
static void CountHeadAndBody(const u8 *tiles, int wTiles, int hTiles, u32 head[16], u32 body[16])
{
    int top, bottom, h;
    VisibleRows(tiles, wTiles, hTiles, &top, &bottom);
    h = bottom - top + 1;
    CountRegionUsage(tiles, wTiles, hTiles, top, top + h * 3 / 10, head);
    CountRegionUsage(tiles, wTiles, hTiles, top + h * 4 / 10, top + h * 9 / 10, body);
}

static bool8 IsSkinTone(int h, int s, int v)
{
    return h >= 5 && h <= 45 && s >= 25 && s <= 175 && v >= 110;
}

static void RecolorCluster(u16 *pal, const u8 *members, int n, u16 target)
{
    int th, ts, tv, i, maxV = 1, maxS = 1;
    RgbToHsv(target, &th, &ts, &tv);
    for (i = 0; i < n; i++)
    {
        int h, s, v;
        RgbToHsv(pal[members[i]], &h, &s, &v);
        if (v > maxV) maxV = v;
        if (s > maxS) maxS = s;
    }
    for (i = 0; i < n; i++)
    {
        int h, s, v;
        RgbToHsv(pal[members[i]], &h, &s, &v);
        pal[members[i]] = HsvToRgb(th, ts * s / maxS, v * tv / maxV);
    }
}

// counts: whole sprite; body/head: the clothes and hair/hat parts of the front frame
static void AutoRecolor(const struct MpAppearance *app, u16 *pal, const u32 counts[16], const u32 body[16], const u32 head[16])
{
    u8 skin[16], nSkin = 0, i, j;
    u32 binWeight[12], bodyWeight[12], headWeight[12];
    s8 bin[16];
    int best = -1, second = -1;

    if (app->skin == 0 && app->outfit == 0 && app->accent == 0)
        return;
    memset(binWeight, 0, sizeof(binWeight));
    memset(bodyWeight, 0, sizeof(bodyWeight));
    memset(headWeight, 0, sizeof(headWeight));
    for (i = 1; i < 16; i++)
    {
        int h, s, v;
        bin[i] = -1;
        if (counts[i] == 0)
            continue;
        RgbToHsv(pal[i], &h, &s, &v);
        if (IsSkinTone(h, s, v))
            skin[nSkin++] = i;
        else if (s >= 60 && v >= 40)
        {
            bin[i] = ((h + 15) / 30) % 12;
            binWeight[bin[i]] += counts[i];
            bodyWeight[bin[i]] += body[i];
            headWeight[bin[i]] += head[i];
        }
    }

    if (app->skin > 0 && app->skin < MP_SKIN_COUNT && nSkin > 0)
    {
        // Lightest skin shade first
        for (i = 0; i < nSkin; i++)
            for (j = i + 1; j < nSkin; j++)
            {
                int h1, s1, v1, h2, s2, v2;
                RgbToHsv(pal[skin[i]], &h1, &s1, &v1);
                RgbToHsv(pal[skin[j]], &h2, &s2, &v2);
                if (v2 > v1)
                {
                    u8 t = skin[i];
                    skin[i] = skin[j];
                    skin[j] = t;
                }
            }
        for (i = 0; i < nSkin; i++)
            pal[skin[i]] = sSkinPresets[app->skin][nSkin == 1 ? 0 : (i * 2 + (nSkin - 1) / 2) / (nSkin - 1)];
    }

    if (getenv("MOEMON_DEBUG_RECOLOR"))
        for (i = 1; i < 16; i++)
        {
            int h, s2, v;
            RgbToHsv(pal[i], &h, &s2, &v);
            printf("[recolor] %2d rgb=%04x h=%3d s=%3d v=%3d all=%4u body=%4u head=%4u bin=%d\n", i, pal[i], h, s2, v, counts[i], body[i], head[i], bin[i]);
        }
    // OUTFIT: the main clothing hue (body rows); ACCENT: hair/hat (head rows).
    // Fall back to overall usage when a region has no clear color.
    for (i = 0; i < 12; i++)
        if (bodyWeight[i] && (best < 0 || bodyWeight[i] > bodyWeight[best]))
            best = i;
    for (i = 0; i < 12; i++)
        if (i != best && headWeight[i] && (second < 0 || headWeight[i] > headWeight[second]))
            second = i;
    for (i = 0; i < 12 && best < 0; i++)
        if (binWeight[i] && (best < 0 || binWeight[i] > binWeight[best]))
            best = i;
    for (i = 0; i < 12 && second < 0; i++)
        if (i != best && binWeight[i] && (second < 0 || binWeight[i] > binWeight[second]))
            second = i;
    if (best >= 0 && app->outfit > 0 && app->outfit < MP_COLOR_COUNT)
    {
        u8 members[16], n = 0;
        for (i = 1; i < 16; i++)
            if (bin[i] == best)
                members[n++] = i;
        RecolorCluster(pal, members, n, sColorPresets[app->outfit][0]);
    }
    if (second >= 0 && app->accent > 0 && app->accent < MP_COLOR_COUNT)
    {
        u8 members[16], n = 0;
        for (i = 1; i < 16; i++)
            if (bin[i] == second)
                members[n++] = i;
        RecolorCluster(pal, members, n, sColorPresets[app->accent][0]);
    }
}

// ---------------------------------------------------------------------------
// Avatar states. The remote avatar modes (MP_AVATAR_*) are the first five.

enum {
    ST_FOOT,
    ST_MACH_BIKE,
    ST_ACRO_BIKE,
    ST_SURF,
    ST_UNDERWATER,
    ST_FIELD_MOVE,
    ST_FISHING,
    ST_WATERING,
    ST_DECORATING,
    ST_COUNT
};

static const u8 sStatePackSlot[ST_COUNT] = {
    PACK_WALK, PACK_MACH_BIKE, PACK_ACRO_BIKE, PACK_SURF, PACK_DIVE,
    PACK_FIELD_MOVE, PACK_FISHING, PACK_WATERING, PACK_DECORATE,
};

static const struct ObjectEventGraphicsInfo *const sDefaultStateInfo[ST_COUNT][GENDER_COUNT] = {
    [ST_FOOT]       = {&gObjectEventGraphicsInfo_BrendanNormal, &gObjectEventGraphicsInfo_MayNormal},
    [ST_MACH_BIKE]  = {&gObjectEventGraphicsInfo_BrendanMachBike, &gObjectEventGraphicsInfo_MayMachBike},
    [ST_ACRO_BIKE]  = {&gObjectEventGraphicsInfo_BrendanAcroBike, &gObjectEventGraphicsInfo_MayAcroBike},
    [ST_SURF]       = {&gObjectEventGraphicsInfo_BrendanSurfing, &gObjectEventGraphicsInfo_MaySurfing},
    [ST_UNDERWATER] = {&gObjectEventGraphicsInfo_BrendanUnderwater, &gObjectEventGraphicsInfo_MayUnderwater},
    [ST_FIELD_MOVE] = {&gObjectEventGraphicsInfo_BrendanFieldMove, &gObjectEventGraphicsInfo_MayFieldMove},
    [ST_FISHING]    = {&gObjectEventGraphicsInfo_BrendanFishing, &gObjectEventGraphicsInfo_MayFishing},
    [ST_WATERING]   = {&gObjectEventGraphicsInfo_BrendanWatering, &gObjectEventGraphicsInfo_MayWatering},
    [ST_DECORATING] = {&gObjectEventGraphicsInfo_BrendanDecorating, &gObjectEventGraphicsInfo_MayDecorating},
};

// Player graphics ids that sprite packs can replace, per state
static const u8 sStateGfxIds[ST_COUNT][GENDER_COUNT] = {
    [ST_FOOT]       = {OBJ_EVENT_GFX_BRENDAN_NORMAL, OBJ_EVENT_GFX_MAY_NORMAL},
    [ST_MACH_BIKE]  = {OBJ_EVENT_GFX_BRENDAN_MACH_BIKE, OBJ_EVENT_GFX_MAY_MACH_BIKE},
    [ST_ACRO_BIKE]  = {OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE, OBJ_EVENT_GFX_MAY_ACRO_BIKE},
    [ST_SURF]       = {OBJ_EVENT_GFX_BRENDAN_SURFING, OBJ_EVENT_GFX_MAY_SURFING},
    [ST_UNDERWATER] = {OBJ_EVENT_GFX_BRENDAN_UNDERWATER, OBJ_EVENT_GFX_MAY_UNDERWATER},
    [ST_FIELD_MOVE] = {OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE, OBJ_EVENT_GFX_MAY_FIELD_MOVE},
    [ST_FISHING]    = {OBJ_EVENT_GFX_BRENDAN_FISHING, OBJ_EVENT_GFX_MAY_FISHING},
    [ST_WATERING]   = {OBJ_EVENT_GFX_BRENDAN_WATERING, OBJ_EVENT_GFX_MAY_WATERING},
    [ST_DECORATING] = {OBJ_EVENT_GFX_BRENDAN_DECORATING, OBJ_EVENT_GFX_MAY_DECORATING},
};

static const struct ObjectEventGraphicsInfo *DefaultInfo(u8 state, u8 gender)
{
    return sDefaultStateInfo[state][gender == FEMALE ? FEMALE : MALE];
}

static bool8 PackHasState(const struct MpPack *pack, u8 state)
{
    return pack && pack->slot[sStatePackSlot[state]] != NULL;
}

// Graphics info storage:
//   [0]                  local player on foot (OBJ_EVENT_GFX_MP_LOCAL)
//   [1 + id]             remote players
//   [LOCAL_STATE(st)]    local player's bike/surf/... from a sprite pack
#define LOCAL_STATE(st) (1 + MP_MAX_PLAYERS + (st) - 1)
#define NUM_MP_GFX (1 + MP_MAX_PLAYERS + ST_COUNT - 1)
#define MAX_FRAMES 27

static struct ObjectEventGraphicsInfo sMpGfxInfo[NUM_MP_GFX];
static struct SpriteFrameImage sMpFrames[NUM_MP_GFX][MAX_FRAMES];
static u32 sLocalStateBuilt[ST_COUNT];
static u16 sOrigPalBrendan[16], sOrigPalMay[16], sOrigPalRSBrendan[16], sOrigPalRSMay[16];
static u16 sLocalPalBrendan[16], sLocalPalMay[16], sLocalPalCustom[16], sLocalPalPack[16], sLocalPalPackDive[16];

// Fills an info for one state. Palette fields are set by the callers.
static void BuildStateInfo(int gfxSlot, const struct MpAppearance *app, const struct MpPack *pack, u8 state)
{
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[gfxSlot];
    const struct ObjectEventGraphicsInfo *def = DefaultInfo(state, app->gender);
    int i;

    if (PackHasState(pack, state))
    {
        u8 slot = sStatePackSlot[state];
        u32 frameBytes = MP_Pack_FrameBytes(slot);
        *info = *def;
        if (state == ST_FOOT)
        {
            // 9 walking + 9 running frames (running reuses walking if missing)
            const u8 *run = pack->slot[PACK_RUN] ? pack->slot[PACK_RUN] : pack->slot[PACK_WALK];
            for (i = 0; i < 9; i++)
            {
                sMpFrames[gfxSlot][i].data = pack->slot[PACK_WALK] + i * frameBytes;
                sMpFrames[gfxSlot][i].size = frameBytes;
                sMpFrames[gfxSlot][9 + i].data = run + i * frameBytes;
                sMpFrames[gfxSlot][9 + i].size = frameBytes;
            }
        }
        else
        {
            for (i = 0; i < gMpPackSlots[slot].frames && i < MAX_FRAMES; i++)
            {
                sMpFrames[gfxSlot][i].data = pack->slot[slot] + i * frameBytes;
                sMpFrames[gfxSlot][i].size = frameBytes;
            }
        }
        info->images = sMpFrames[gfxSlot];
        info->size = frameBytes;
        return;
    }

    if (state != ST_FOOT)
    {
        *info = *def;
        return;
    }

    // On foot without a pack: the chosen character
    {
        const struct ObjectEventGraphicsInfo *base;
        const struct ObjectEventGraphicsInfo *anims = DefaultInfo(ST_FOOT, app->gender);
        bool8 hasRunFrames;
        switch (app->base)
        {
        case MP_BASE_NPC:        base = NpcInfo(app->gfxId) ? NpcInfo(app->gfxId) : anims; break;
        case MP_BASE_BRENDAN:    base = &gObjectEventGraphicsInfo_BrendanNormal; break;
        case MP_BASE_MAY:        base = &gObjectEventGraphicsInfo_MayNormal; break;
        case MP_BASE_RS_BRENDAN: base = &gObjectEventGraphicsInfo_RubySapphireBrendan; break;
        case MP_BASE_RS_MAY:     base = &gObjectEventGraphicsInfo_RubySapphireMay; break;
        default:                 base = anims; break;
        }
        hasRunFrames = (base == &gObjectEventGraphicsInfo_BrendanNormal || base == &gObjectEventGraphicsInfo_MayNormal);
        *info = *base;
        for (i = 0; i < 18; i++)
            sMpFrames[gfxSlot][i] = base->images[hasRunFrames ? i : (i % 9)];
        info->images = sMpFrames[gfxSlot];
        info->anims = anims->anims;   // player animations (walk + run)
        info->inanimate = FALSE;
        if (app->base != MP_BASE_NPC)
        {
            info->size = 256;
            info->width = 16;
            info->height = 32;
            info->oam = anims->oam;
            info->subspriteTables = anims->subspriteTables;
        }
    }
}

// ---------------------------------------------------------------------------
// Palettes

static void ApplyRecolor(const struct MpAppearance *app, u16 *pal)
{
    if (app->skin > 0 && app->skin < MP_SKIN_COUNT)
    {
        pal[1] = sSkinPresets[app->skin][0];
        pal[2] = sSkinPresets[app->skin][1];
        pal[3] = sSkinPresets[app->skin][2];
    }
    if (app->outfit > 0 && app->outfit < MP_COLOR_COUNT)
    {
        pal[12] = sColorPresets[app->outfit][0];
        pal[13] = sColorPresets[app->outfit][1];
    }
    if (app->accent > 0 && app->accent < MP_COLOR_COUNT)
    {
        pal[10] = sColorPresets[app->accent][0];
        pal[11] = sColorPresets[app->accent][1];
    }
}

// Pack overworld palette, recolored using the walking frames (or any slot).
// Diving has its own palette and is never recolored (like the default one).
static void PackPalette(const struct MpAppearance *app, const struct MpPack *pack, u8 state, u16 *dst)
{
    u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
    u8 slot, front = PACK_SLOT_COUNT;
    if (state == ST_UNDERWATER)
    {
        memcpy(dst, pack->pal[PACK_PAL_DIVE], 32);
        return;
    }
    memcpy(dst, pack->pal[PACK_PAL_OVERWORLD], 32);
    for (slot = 0; slot < PACK_FRONT; slot++)
        if (pack->slot[slot] && gMpPackSlots[slot].palette == PACK_PAL_OVERWORLD)
        {
            CountTileUsage(pack->slot[slot], MP_Pack_SlotBytes(slot), counts);
            if (front == PACK_SLOT_COUNT)
                front = slot;
        }
    if (front == PACK_SLOT_COUNT)
        return;
    CountHeadAndBody(pack->slot[front], gMpPackSlots[front].w / 8, gMpPackSlots[front].h / 8, head, body);
    AutoRecolor(app, dst, counts, body, head);
}

// Palette of a character in a state (no pack)
static void StatePalette(const struct MpAppearance *app, u8 state, u16 *dst)
{
    bool8 female = (app->gender == FEMALE);
    if (state == ST_UNDERWATER)
    {
        memcpy(dst, MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_PLAYER_UNDERWATER), 32);
        return;
    }
    if (state == ST_FOOT && app->base == MP_BASE_NPC && NpcInfo(app->gfxId))
    {
        const struct ObjectEventGraphicsInfo *npc = NpcInfo(app->gfxId);
        const u16 *pal = MP_GetObjectEventPaletteData(npc->paletteTag);
        if (pal)
        {
            u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
            u8 f;
            memcpy(dst, pal, 32);
            for (f = 0; f < 9; f++)
                CountTileUsage(npc->images[f].data, npc->images[f].size, counts);
            CountHeadAndBody(npc->images[0].data, npc->width / 8, npc->height / 8, head, body);
            AutoRecolor(app, dst, counts, body, head);
            return;
        }
    }
    if (state == ST_FOOT && app->base == MP_BASE_RS_BRENDAN)
        memcpy(dst, sOrigPalRSBrendan, 32);
    else if (state == ST_FOOT && app->base == MP_BASE_RS_MAY)
        memcpy(dst, sOrigPalRSMay, 32);
    else if (state == ST_FOOT && app->base == MP_BASE_BRENDAN)
        memcpy(dst, sOrigPalBrendan, 32);
    else if (state == ST_FOOT && app->base == MP_BASE_MAY)
        memcpy(dst, sOrigPalMay, 32);
    else
        memcpy(dst, female ? sOrigPalMay : sOrigPalBrendan, 32);
    ApplyRecolor(app, dst);
}

// Unrecolored palette for a tag (Brendan/May palettes are recolored in place)
const u16 *MP_Avatar_OriginalPalette(u16 tag)
{
    switch (tag)
    {
    case OBJ_EVENT_PAL_TAG_BRENDAN: return sOrigPalBrendan;
    case OBJ_EVENT_PAL_TAG_MAY:     return sOrigPalMay;
    }
    return MP_GetObjectEventPaletteData(tag);
}

void MP_Avatar_GetRemotePalette(u8 id, u8 avatarMode, u16 *dst)
{
    struct MpPack buf;
    const struct MpPack *pack = RemotePack(id, &buf);
    struct MpAppearance app = gMpPlayers[id].app;
    if (avatarMode >= ST_COUNT)
        avatarMode = ST_FOOT;
    if (PackHasState(pack, avatarMode))
        PackPalette(&app, pack, avatarMode, dst);
    else
        StatePalette(&app, avatarMode, dst);
}

// ---------------------------------------------------------------------------
// Local player

void MP_Avatar_Init(void)
{
    if (sAvatarInit)
        return;
    memcpy(sOrigPalBrendan, MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_BRENDAN), 32);
    memcpy(sOrigPalMay, MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MAY), 32);
    memcpy(sOrigPalRSBrendan, MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_RS_BRENDAN), 32);
    memcpy(sOrigPalRSMay, MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_RS_MAY), 32);
    memcpy(sLocalPalBrendan, sOrigPalBrendan, 32);
    memcpy(sLocalPalMay, sOrigPalMay, 32);
    memcpy(sLocalPalCustom, sOrigPalBrendan, 32);
    memcpy(sLocalPalPack, sOrigPalBrendan, 32);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_BRENDAN, sLocalPalBrendan);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MAY, sLocalPalMay);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MP_LOCAL, sLocalPalCustom);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MP_PACK, sLocalPalPack);
    memcpy(sLocalPalPackDive, sOrigPalBrendan, 32);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MP_PACK_DIVE, sLocalPalPackDive);

    if (gMpConfig.app.base == MP_BASE_CUSTOM)
        gMpConfig.app.base = MP_BASE_DEFAULT;   // old ini files
    gMpConfig.app.hasCustom = FALSE;
    if (gMpConfig.customSprite[0])
    {
        char err[160];
        if (LoadLocalPack(gMpConfig.customSprite, err, sizeof(err)))
            gMpConfig.app.hasCustom = TRUE;
        else
            printf("[mp] sprite pack not loaded: %s\n", err);
    }
    MP_Avatar_RefreshPackList();
    sAvatarInit = TRUE;
}

static struct MpAppearance GetLocalAppearance(void)
{
    struct MpAppearance app = gMpConfig.app;
    app.gender = gSaveBlock2Ptr->playerGender;
    if (app.base == MP_BASE_DEFAULT || app.base == MP_BASE_CUSTOM)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    return app;
}

bool8 MP_LocalUsesCustomAvatar(void)
{
    struct MpAppearance app;
    if (!sAvatarInit)
        return FALSE;
    app = GetLocalAppearance();
    if (PackHasState(LocalPack(), ST_FOOT))
        return TRUE;
    return !((app.base == MP_BASE_BRENDAN && app.gender == MALE) || (app.base == MP_BASE_MAY && app.gender == FEMALE));
}

bool8 MP_IsMpGraphicsId(u8 graphicsId)
{
    return graphicsId >= OBJ_EVENT_GFX_MP_FIRST && graphicsId <= OBJ_EVENT_GFX_MP_LAST;
}

const struct ObjectEventGraphicsInfo *MP_GetGraphicsInfo(u8 graphicsId)
{
    int slot = graphicsId - OBJ_EVENT_GFX_MP_FIRST;
    if (slot == 0 && sMpGfxInfo[0].images == NULL)
        RebuildLocal();
    if (sMpGfxInfo[slot].images == NULL)
        return &gObjectEventGraphicsInfo_BrendanNormal;
    return &sMpGfxInfo[slot];
}

// Sprite pack replacements for the player's bike/surf/... graphics ids
const struct ObjectEventGraphicsInfo *MP_Avatar_LocalStateInfo(u8 graphicsId)
{
    u8 st, g;
    const struct MpPack *pack = LocalPack();
    if (!pack || !sAvatarInit)
        return NULL;
    for (st = ST_MACH_BIKE; st < ST_COUNT; st++)
    {
        for (g = 0; g < GENDER_COUNT; g++)
        {
            if (sStateGfxIds[st][g] != graphicsId)
                continue;
            if (!PackHasState(pack, st))
                return NULL;
            if (sLocalStateBuilt[st] != sLocalVersion + 1)
            {
                struct MpAppearance app = GetLocalAppearance();
                struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[LOCAL_STATE(st)];
                BuildStateInfo(LOCAL_STATE(st), &app, pack, st);
                info->paletteSlot = PALSLOT_PLAYER;
                info->paletteTag = (st == ST_UNDERWATER) ? OBJ_EVENT_PAL_TAG_MP_PACK_DIVE : OBJ_EVENT_PAL_TAG_MP_PACK;
                sLocalStateBuilt[st] = sLocalVersion + 1;
            }
            return &sMpGfxInfo[LOCAL_STATE(st)];
        }
    }
    return NULL;
}

// Rebuilds the local palettes and graphics
static void RebuildLocal(void)
{
    struct MpAppearance app, genderApp;
    const struct MpPack *pack;

    MP_Avatar_Init();
    app = GetLocalAppearance();
    pack = LocalPack();
    sLocalVersion++;

    // Default bike/surf/etc sprites use the save file gender's palette, recolored
    genderApp = app;
    genderApp.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    memcpy(sLocalPalBrendan, sOrigPalBrendan, 32);
    memcpy(sLocalPalMay, sOrigPalMay, 32);
    if (app.gender == FEMALE)
        ApplyRecolor(&genderApp, sLocalPalMay);
    else
        ApplyRecolor(&genderApp, sLocalPalBrendan);

    StatePalette(&app, ST_FOOT, sLocalPalCustom);
    if (pack)
    {
        PackPalette(&app, pack, ST_FOOT, sLocalPalPack);
        PackPalette(&app, pack, ST_UNDERWATER, sLocalPalPackDive);
    }

    BuildStateInfo(0, &app, pack, ST_FOOT);
    sMpGfxInfo[0].paletteSlot = PALSLOT_PLAYER;
    sMpGfxInfo[0].paletteTag = PackHasState(pack, ST_FOOT) ? OBJ_EVENT_PAL_TAG_MP_PACK : OBJ_EVENT_PAL_TAG_MP_LOCAL;
    memset(sLocalStateBuilt, 0, sizeof(sLocalStateBuilt));
}

// Rebuilds the local avatar and refreshes the player sprite
void MP_Avatar_ApplyLocal(void)
{
    void MP_SpriteRefreshImages(struct Sprite *sprite, u32 oldFrameBytes);
    bool8 onField = MP_IsInOverworld() && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT
                 && gObjectEvents[gPlayerAvatar.objectEventId].active;
    u32 oldBytes = 0;

    // The sprite's tile block was sized for the old look
    if (onField && gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].images)
        oldBytes = gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].images->size;
    RebuildLocal();
    if (onField)
    {
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        u8 gfx = GetPlayerAvatarGraphicsIdByCurrentState();
        if (gfx != 0 || (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ON_FOOT))
        {
            ObjectEventSetGraphicsId(player, gfx);
            StartSpriteAnim(&gSprites[player->spriteId], GetFaceDirectionAnimNum(player->facingDirection));
            // Show it now, even with the menu open (the player is frozen)
            MP_SpriteRefreshImages(&gSprites[player->spriteId], oldBytes);
        }
        UpdateSpritePaletteWithWeather(PALSLOT_PLAYER);
    }
}

// ---------------------------------------------------------------------------
// Remote players

void MP_Avatar_BuildRemote(u8 id)
{
    struct MpPlayer *p = &gMpPlayers[id];
    struct MpAppearance app = p->app;
    struct MpPack buf;
    const struct MpPack *pack = RemotePack(id, &buf);
    u8 mode = p->hasState ? p->state.avatarMode : MP_AVATAR_FOOT;
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[1 + id];

    if (mode >= ST_COUNT)
        mode = ST_FOOT;
    if (app.base == MP_BASE_DEFAULT || app.base >= MP_BASE_COUNT || app.base == MP_BASE_CUSTOM)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    BuildStateInfo(1 + id, &app, pack, mode);
    // The overworld code assigns the real palette slot after spawning
    info->paletteSlot = PALSLOT_NPC_1;
    info->paletteTag = 0x1380 + 1 + id;
    info->disableReflectionPaletteLoad = TRUE;
}

// ---------------------------------------------------------------------------
// Trainer pictures (trainer card, battles)

u16 MP_Avatar_FrontPicFor(const struct MpAppearance *app)
{
    const struct MpSpriteEntry *e;
    switch (app->base)
    {
    case MP_BASE_BRENDAN:    return TRAINER_PIC_BRENDAN;
    case MP_BASE_MAY:        return TRAINER_PIC_MAY;
    case MP_BASE_RS_BRENDAN: return TRAINER_PIC_RS_BRENDAN;
    case MP_BASE_RS_MAY:     return TRAINER_PIC_RS_MAY;
    case MP_BASE_NPC:
        e = MP_Avatar_FindCatalog(app->gfxId);
        if (e && e->frontPic != MP_PIC_NONE)
            return e->frontPic;
        break;
    }
    return 0xFFFF;
}

// Loads a pack's FRONT picture into runtime slot 0 (local) or 1 (opponent)
static void LoadPackFront(const struct MpAppearance *app, const struct MpPack *pack, int which)
{
    u16 pal[16];
    u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
    memcpy(pal, pack->pal[PACK_PAL_FRONT], 32);
    CountTileUsage(pack->slot[PACK_FRONT], TRAINER_PIC_SIZE, counts);
    CountHeadAndBody(pack->slot[PACK_FRONT], 8, 8, head, body);
    AutoRecolor(app, pal, counts, body, head);
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPicLz[which], pack->slot[PACK_FRONT], TRAINER_PIC_SIZE);
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPalLz[which], pal, 32);
}

u16 MP_Avatar_LocalFrontPic(void)
{
    const struct MpPack *pack = LocalPack();
    struct MpAppearance app = GetLocalAppearance();
    if (pack && pack->slot[PACK_FRONT])
    {
        LoadPackFront(&app, pack, 0);
        return TRAINER_PIC_MP_LOCAL;
    }
    return MP_Avatar_FrontPicFor(&gMpConfig.app);
}

u16 MP_Avatar_RemoteFrontPic(u8 id)
{
    struct MpPack buf;
    const struct MpPack *pack;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    pack = RemotePack(id, &buf);
    if (pack && pack->slot[PACK_FRONT])
    {
        LoadPackFront(&gMpPlayers[id].app, pack, 1);
        return TRAINER_PIC_MP_OPPONENT;
    }
    return MP_Avatar_FrontPicFor(&gMpPlayers[id].app);
}

u16 MP_LocalFrontPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalFrontPic();
    return pic == 0xFFFF ? defaultPic : pic;
}

// Copies a pack's BACK frames (recolored for the appearance) into a back pic slot
static void LoadPackBack(const struct MpAppearance *appIn, const struct MpPack *pack, u8 *tiles, u32 *palLz)
{
    struct MpAppearance app = *appIn;
    u16 pal[16];
    u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
    memcpy(tiles, pack->slot[PACK_BACK], 4 * TRAINER_PIC_SIZE);
    memcpy(pal, pack->pal[PACK_PAL_BACK], 32);
    CountTileUsage(pack->slot[PACK_BACK], TRAINER_PIC_SIZE, counts);
    CountHeadAndBody(pack->slot[PACK_BACK], 8, 8, head, body);
    AutoRecolor(&app, pal, counts, body, head);
    MP_Pack_WriteLz77Literal((u8 *)palLz, pal, 32);
}

static u16 BackPicForBase(const struct MpAppearance *app)
{
    const struct MpSpriteEntry *e;
    switch (app->base)
    {
    case MP_BASE_BRENDAN:    return TRAINER_BACK_PIC_BRENDAN;
    case MP_BASE_MAY:        return TRAINER_BACK_PIC_MAY;
    case MP_BASE_RS_BRENDAN: return TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN;
    case MP_BASE_RS_MAY:     return TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY;
    case MP_BASE_NPC:
        e = MP_Avatar_FindCatalog(app->gfxId);
        if (e && e->backPic != MP_PIC_NONE)
            return e->backPic;
        break;
    }
    return 0xFFFF;
}

u16 MP_Avatar_LocalBackPic(void)
{
    const struct MpPack *pack = LocalPack();

    if (pack && pack->slot[PACK_BACK])
    {
        struct MpAppearance app = GetLocalAppearance();
        LoadPackBack(&app, pack, gMpTrainerBackTiles, gMpTrainerBackPalLz);
        return TRAINER_BACK_PIC_MP_LOCAL;
    }
    return BackPicForBase(&gMpConfig.app);
}

// Another player's battle back pic (co-op partner), 0xFFFF = game default
u16 MP_Avatar_RemoteBackPic(u8 id)
{
    struct MpPack buf;
    const struct MpPack *pack;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    pack = RemotePack(id, &buf);
    if (pack && pack->slot[PACK_BACK])
    {
        LoadPackBack(&gMpPlayers[id].app, pack, gMpPartnerBackTiles, gMpPartnerBackPalLz);
        return TRAINER_BACK_PIC_MP_PARTNER;
    }
    return BackPicForBase(&gMpPlayers[id].app);
}

bool8 MP_Avatar_RemoteHidesBack(u8 id)
{
    return id < MP_MAX_PLAYERS && gMpPlayers[id].present && gMpPlayers[id].app.base == MP_BASE_NPC
        && MP_Avatar_RemoteBackPic(id) == 0xFFFF;
}

u16 MP_LocalBackPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalBackPic();
    return pic == 0xFFFF ? defaultPic : pic;
}

bool8 MP_LocalHidesTrainerBack(void)
{
    return gMpConfig.app.base == MP_BASE_NPC && MP_Avatar_LocalBackPic() == 0xFFFF;
}

// ---------------------------------------------------------------------------
// Sprite packs: exporting for the editor, installing dropped files

static void EnsurePackDir(void)
{
#ifdef _WIN32
    _mkdir(PACK_DIR);
#else
    mkdir(PACK_DIR, 0755);
#endif
}

// Writes the current character (with the active pack) as an editable PNG
bool8 MP_Avatar_ExportLocal(char *pathOut, int len)
{
    struct MpAppearance app = GetLocalAppearance();
    char name[40];
    EnsurePackDir();
    MP_Avatar_SpriteConfigName(&app, name, sizeof(name));
    if (LocalPack() && gMpConfig.customSprite[0])
        snprintf(pathOut, len, "%s", gMpConfig.customSprite); // edit the pack in place
    else
        snprintf(pathOut, len, "%s/my_%s.png", PACK_DIR, name);
    return MP_Pack_Export(pathOut, app.base, app.gfxId, app.gender, LocalPack());
}

static bool8 FileExists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (f)
        fclose(f);
    return f != NULL;
}

// Copies a dropped PNG into the sprites folder and wears it.
// Returns FALSE (with a reason) when the file isn't a sprite sheet.
bool8 MP_Avatar_InstallPack(const char *src, char *err, int errLen)
{
    u32 size;
    u8 *blob = MP_Pack_LoadFile(src, &size, err, errLen);
    const char *base = src, *p;
    char dst[200], stem[96];
    int i, n = 0;
    FILE *in, *out;
    u16 choice;

    if (!blob)
        return FALSE;
    free(blob);
    for (p = src; *p; p++)
        if (*p == '/' || *p == '\\')
            base = p + 1;
    for (p = base; *p && *p != '.' && n < 60; p++)
        stem[n++] = (isalnum((unsigned char)*p) || *p == '_' || *p == '-') ? *p : '_';
    stem[n] = 0;
    if (n == 0)
        snprintf(stem, sizeof(stem), "sprite");
    EnsurePackDir();
    snprintf(dst, sizeof(dst), "%s/%s.png", PACK_DIR, stem);
    // Same file dropped again (e.g. re-saved from the editor): replace it
    in = fopen(src, "rb");
    if (!in)
    {
        snprintf(err, errLen, "can't read %s", src);
        return FALSE;
    }
    if (FileExists(dst) && strcmp(dst, src) != 0)
        remove(dst);
    out = fopen(dst, "wb");
    if (out)
    {
        char buf[4096];
        size_t got;
        while ((got = fread(buf, 1, sizeof(buf), in)) > 0)
            fwrite(buf, 1, got, out);
        fclose(out);
    }
    fclose(in);
    if (!out)
    {
        snprintf(err, errLen, "can't write %s", dst);
        return FALSE;
    }
    MP_Avatar_RefreshPackList();
    for (i = 0; i < sPackFileCount; i++)
        if (strcmp(sPackFiles[i], dst) == 0)
            break;
    if (i == sPackFileCount)
    {
        snprintf(err, errLen, "sprites folder is full");
        return FALSE;
    }
    choice = NUM_BASE_CHOICES + ARRAY_COUNT(sSpriteCatalog) + i;
    MP_Avatar_SetChoice(&gMpConfig.app, choice);
    if (!gMpConfig.app.hasCustom)
    {
        snprintf(err, errLen, "couldn't load %s", dst);
        return FALSE;
    }
    return TRUE;
}
