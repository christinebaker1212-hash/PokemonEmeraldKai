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
#include "gba/syscall.h"

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
// Presets. Which palette colors each sprite recolors is mapped by hand
// (tools/moemon_frlg/colormaps.txt, see RecolorByMap).

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

// Hair: target color (shades keep their lightness)
static const u16 sHairPresets[MP_HAIR_COUNT] = {
    0,                  // default
    RGB(7, 7, 9),       // black
    RGB(17, 11, 6),     // brown
    RGB(22, 10, 5),     // chestnut
    RGB(31, 27, 13),    // blonde
    RGB(30, 15, 5),     // ginger
    RGB(28, 6, 7),      // red
    RGB(24, 24, 27),    // silver
    RGB(31, 31, 31),    // white
    RGB(8, 15, 31),     // blue
    RGB(31, 16, 24),    // pink
    RGB(20, 10, 29),    // purple
    RGB(9, 23, 10),     // green
    RGB(7, 24, 23),     // teal
    RGB(31, 22, 4),     // gold
};
static const char *const sHairNames[MP_HAIR_COUNT] = {
    "DEFAULT", "BLACK", "BROWN", "CHESTNUT", "BLONDE", "GINGER", "RED", "SILVER", "WHITE",
    "BLUE", "PINK", "PURPLE", "GREEN", "TEAL", "GOLD",
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
const char *MP_Avatar_HairName(u8 c) { return c < MP_HAIR_COUNT ? sHairNames[c] : "?"; }

// ---------------------------------------------------------------------------
// FireRed/LeafGreen overworld sprites, extra trainer pictures (FRLG and
// Pokemon Showdown) and back pictures, from tools/moemon_frlg/import.py

struct MpExtOverworld
{
    const char *name;
    u8 w, h;
    u32 offset;     // in sExtOwTiles (9 frames)
    s16 front;      // front choice, -1 = none
    u8 gender;
};

struct MpExtFront
{
    const char *name;
    const char *cfg;      // ini name
    const char *artist;   // credited fan art: used as drawn, never recolored
    u8 recolor;
};

#include "mp_ext_gfx.h"

static const u8 sExtOwTiles[] = INCBIN_U8("graphics/moemon/ext/ow_tiles.bin");
static const u16 sExtOwPals[] = INCBIN_U16("graphics/moemon/ext/ow_pals.bin");
static const u8 sExtFrontTiles[] = INCBIN_U8("graphics/moemon/ext/front_tiles.bin");
static const u16 sExtFrontPals[] = INCBIN_U16("graphics/moemon/ext/front_pals.bin");
static const u8 sExtBackTiles[] = INCBIN_U8("graphics/moemon/ext/back_tiles.bin");
static const u16 sExtBackPals[] = INCBIN_U16("graphics/moemon/ext/back_pals.bin");

#define EMERALD_FRONT_COUNT TRAINER_PIC_MP_LOCAL          // Emerald's own trainer pics come first
#define FRONT_CHOICES (EMERALD_FRONT_COUNT + MP_EXT_FRONT_COUNT)
#define EMERALD_BACK_COUNT TRAINER_BACK_PIC_MP_LOCAL
#define BACK_CHOICES (EMERALD_BACK_COUNT + MP_EXT_BACK_COUNT)

#define CATALOG_COUNT (ARRAY_COUNT(sSpriteCatalog) + MP_EXT_OW_COUNT)
// Selections: 0 = MATCH, 1..CHOICES = a picture, then DEFAULT (Brendan/May by
// the save's gender) for ID PIC and NONE (hidden in battles) for BK PIC
#define FRONT_DEFAULT (FRONT_CHOICES + 1)
#define BACK_NONE (BACK_CHOICES + 1)

static struct MpSpriteEntry sExtCatalog[MP_EXT_OW_COUNT];
static struct ObjectEventGraphicsInfo sExtInfo[MP_EXT_OW_COUNT];
static struct SpriteFrameImage sExtFrames[MP_EXT_OW_COUNT][9];
static bool8 sExtReady;

static void InitExtCatalog(void)
{
    int i, f;
    if (sExtReady)
        return;
    for (i = 0; i < MP_EXT_OW_COUNT; i++)
    {
        const struct MpExtOverworld *e = &sMpExtOverworld[i];
        const struct ObjectEventGraphicsInfo *tmpl;
        u32 frameBytes = e->w * e->h / 2;
        sExtCatalog[i].gfxId = MP_GFX_EXT_FIRST + i;
        sExtCatalog[i].name = e->name;
        sExtCatalog[i].frontPic = e->front >= 0 ? (u16)e->front : MP_PIC_NONE;
        sExtCatalog[i].backPic = MP_PIC_NONE;
        sExtCatalog[i].gender = e->gender;
        // An Emerald sprite of the same size lends its OAM shapes and animations
        if (e->w == 32)
            tmpl = &gObjectEventGraphicsInfo_CyclingTriathleteM;
        else if (e->h == 16)
            tmpl = &gObjectEventGraphicsInfo_LittleBoy;
        else
            tmpl = &gObjectEventGraphicsInfo_Boy1;
        sExtInfo[i] = *tmpl;
        for (f = 0; f < 9; f++)
        {
            sExtFrames[i][f].data = sExtOwTiles + e->offset + f * frameBytes;
            sExtFrames[i][f].size = frameBytes;
        }
        sExtInfo[i].images = sExtFrames[i];
        sExtInfo[i].size = frameBytes;
        sExtInfo[i].width = e->w;
        sExtInfo[i].height = e->h;
        sExtInfo[i].inanimate = FALSE;
        sExtInfo[i].tracks = TRACKS_FOOT;
        sExtInfo[i].anims = gObjectEventGraphicsInfo_Boy1.anims;
    }
    sExtReady = TRUE;
}

static const struct MpSpriteEntry *CatalogAt(u16 i)
{
    InitExtCatalog();
    if (i < ARRAY_COUNT(sSpriteCatalog))
        return &sSpriteCatalog[i];
    return &sExtCatalog[i - ARRAY_COUNT(sSpriteCatalog)];
}

const struct MpSpriteEntry *MP_Avatar_FindCatalog(u16 gfxId)
{
    u16 i;
    if (gfxId >= MP_GFX_EXT_FIRST)
        return gfxId - MP_GFX_EXT_FIRST < MP_EXT_OW_COUNT ? CatalogAt(ARRAY_COUNT(sSpriteCatalog) + gfxId - MP_GFX_EXT_FIRST) : NULL;
    for (i = 0; i < ARRAY_COUNT(sSpriteCatalog); i++)
        if (sSpriteCatalog[i].gfxId == gfxId)
            return &sSpriteCatalog[i];
    return NULL;
}

const struct ObjectEventGraphicsInfo *MP_Avatar_NpcInfo(u16 gfxId)
{
    if (MP_Avatar_FindCatalog(gfxId) == NULL)
        return NULL;
    if (gfxId >= MP_GFX_EXT_FIRST)
        return &sExtInfo[gfxId - MP_GFX_EXT_FIRST];
    return gObjectEventGraphicsInfoPointers[gfxId];
}

const u16 *MP_Avatar_NpcPalette(u16 gfxId)
{
    const struct ObjectEventGraphicsInfo *info = MP_Avatar_NpcInfo(gfxId);
    if (!info)
        return NULL;
    if (gfxId >= MP_GFX_EXT_FIRST)
        return sExtOwPals + (gfxId - MP_GFX_EXT_FIRST) * 16;
    return MP_GetObjectEventPaletteData(info->paletteTag);
}

static const struct ObjectEventGraphicsInfo *NpcInfo(u16 gfxId)
{
    return MP_Avatar_NpcInfo(gfxId);
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
    return NUM_BASE_CHOICES + CATALOG_COUNT + sPackFileCount;
}

u16 MP_Avatar_GetChoice(const struct MpAppearance *app)
{
    u16 i;
    if (app->hasCustom && sLocalHasPack)
    {
        for (i = 0; i < sPackFileCount; i++)
            if (strcmp(sPackFiles[i], gMpConfig.customSprite) == 0)
                return NUM_BASE_CHOICES + CATALOG_COUNT + i;
    }
    if (app->base == MP_BASE_NPC)
    {
        for (i = 0; i < CATALOG_COUNT; i++)
            if (CatalogAt(i)->gfxId == app->gfxId)
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
    else if (choice < NUM_BASE_CHOICES + CATALOG_COUNT)
    {
        app->base = MP_BASE_NPC;
        app->gfxId = CatalogAt(choice - NUM_BASE_CHOICES)->gfxId;
        app->hasCustom = FALSE;
    }
    else if (choice - NUM_BASE_CHOICES - CATALOG_COUNT < sPackFileCount)
    {
        const char *path = sPackFiles[choice - NUM_BASE_CHOICES - CATALOG_COUNT];
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

// Puts back the sprite (and sprite pack) of a saved look (LOOK > REVERT, SELECT)
void MP_Avatar_RestoreSprite(const struct MpAppearance *saved, const char *savedPack)
{
    struct MpAppearance tmp = *saved;
    u16 i;
    if (saved->hasCustom && savedPack[0])
    {
        for (i = 0; i < sPackFileCount; i++)
            if (strcmp(sPackFiles[i], savedPack) == 0)
            {
                MP_Avatar_SetChoice(&gMpConfig.app, NUM_BASE_CHOICES + CATALOG_COUNT + i);
                return;
            }
    }
    tmp.hasCustom = FALSE;
    MP_Avatar_SetChoice(&gMpConfig.app, MP_Avatar_GetChoice(&tmp));
}

const char *MP_Avatar_ChoiceName(u16 choice)
{
    if (choice < NUM_BASE_CHOICES)
        return sBaseNames[choice];
    if (choice < NUM_BASE_CHOICES + CATALOG_COUNT)
        return CatalogAt(choice - NUM_BASE_CHOICES)->name;
    if (choice - NUM_BASE_CHOICES - CATALOG_COUNT < sPackFileCount)
        return sPackNames[choice - NUM_BASE_CHOICES - CATALOG_COUNT];
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
    {
        char buf[40];
        ToConfigName(buf, MP_Avatar_FindCatalog(app->gfxId)->name, sizeof(buf));
        snprintf(out, len, "%s%s", app->gfxId >= MP_GFX_EXT_FIRST ? "frlg_" : "", buf);
    }
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
    for (i = 0; i < CATALOG_COUNT; i++)
    {
        ToConfigName(buf, CatalogAt(i)->name, sizeof(buf));
        if (i >= ARRAY_COUNT(sSpriteCatalog))
        {
            // FireRed/LeafGreen sprites are saved as frlg_<name>
            char ext[48];
            snprintf(ext, sizeof(ext), "frlg_%s", buf);
            snprintf(buf, sizeof(buf), "%s", ext);
        }
        if (strcmp(buf, name) == 0)
        {
            app->base = MP_BASE_NPC;
            app->gfxId = CatalogAt(i)->gfxId;
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

static bool8 WantsRecolor(const struct MpAppearance *app)
{
    return app->skin || app->outfit || app->accent || app->hair;
}

// fullSat: every shade takes the target's saturation (hair, gray clothes)
static void RecolorClusterEx(u16 *pal, const u8 *members, int n, u16 target, bool8 fullSat)
{
    int th, ts, tv, i, maxV = 1, maxS = 1, lightS = 0;
    RgbToHsv(target, &th, &ts, &tv);
    for (i = 0; i < n; i++)
    {
        int h, s, v;
        RgbToHsv(pal[members[i]], &h, &s, &v);
        if (v > maxV)
        {
            maxV = v;
            lightS = s;
        }
        if (s > maxS) maxS = s;
    }
    // A white or gray piece (a white hat) takes the target's saturation in every shade
    if (lightS < 80)
        fullSat = TRUE;
    for (i = 0; i < n; i++)
    {
        int h, s, v;
        RgbToHsv(pal[members[i]], &h, &s, &v);
        pal[members[i]] = HsvToRgb(th, fullSat ? ts : ts * s / maxS, v * tv / maxV);
    }
}

// ---------------------------------------------------------------------------
// Hand-mapped colors (tools/moemon_frlg/colormaps.txt): for each sprite and
// picture, which palette colors are skin (S), hair (H), outfit (O) and
// accent (A). Sprites without a map (Showdown pictures, sprite packs) keep
// their colors.

enum { CMAP_OW, CMAP_FRONT, CMAP_BACK };

struct MpColorMap
{
    u8 kind;
    const char *cfg;
    const char *map;   // 15 letters for palette colors 1-15
};

#include "mp_color_maps.h"

static const char *FindColorMap(u8 kind, const char *cfg)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sMpColorMaps); i++)
        if (sMpColorMaps[i].kind == kind && strcmp(sMpColorMaps[i].cfg, cfg) == 0)
            return sMpColorMaps[i].map;
    return NULL;
}

static u8 MapMembers(const char *map, char role, u8 *members)
{
    u8 i, n = 0;
    for (i = 0; i < 15 && map[i]; i++)
        if (map[i] == role)
            members[n++] = i + 1;
    return n;
}

static void RecolorByMap(const struct MpAppearance *app, u16 *pal, const char *map)
{
    u8 members[16], n, i, j;
    if (map == NULL || !WantsRecolor(app))
        return;
    if (app->skin > 0 && app->skin < MP_SKIN_COUNT && (n = MapMembers(map, 'S', members)) > 0)
    {
        // Lightest shade first, then the preset's three shades spread over them
        for (i = 0; i < n; i++)
            for (j = i + 1; j < n; j++)
            {
                int h1, s1, v1, h2, s2, v2;
                RgbToHsv(pal[members[i]], &h1, &s1, &v1);
                RgbToHsv(pal[members[j]], &h2, &s2, &v2);
                if (v2 > v1)
                {
                    u8 t = members[i];
                    members[i] = members[j];
                    members[j] = t;
                }
            }
        for (i = 0; i < n; i++)
            pal[members[i]] = sSkinPresets[app->skin][n == 1 ? 0 : (i * 2 + (n - 1) / 2) / (n - 1)];
    }
    if (app->hair > 0 && app->hair < MP_HAIR_COUNT && (n = MapMembers(map, 'H', members)) > 0)
        RecolorClusterEx(pal, members, n, sHairPresets[app->hair], FALSE);
    if (app->outfit > 0 && app->outfit < MP_COLOR_COUNT && (n = MapMembers(map, 'O', members)) > 0)
        RecolorClusterEx(pal, members, n, sColorPresets[app->outfit][0], FALSE);
    if (app->accent > 0 && app->accent < MP_COLOR_COUNT && (n = MapMembers(map, 'A', members)) > 0)
        RecolorClusterEx(pal, members, n, sColorPresets[app->accent][0], FALSE);
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

// Brendan/May (and their bike, surf... sprites, which share the palette) and the RS ones
static void ApplyRecolor(const struct MpAppearance *app, u16 *pal)
{
    switch (app->base)
    {
    case MP_BASE_BRENDAN:    RecolorByMap(app, pal, FindColorMap(CMAP_OW, "brendan")); break;
    case MP_BASE_MAY:        RecolorByMap(app, pal, FindColorMap(CMAP_OW, "may")); break;
    case MP_BASE_RS_BRENDAN: RecolorByMap(app, pal, FindColorMap(CMAP_OW, "rs_brendan")); break;
    case MP_BASE_RS_MAY:     RecolorByMap(app, pal, FindColorMap(CMAP_OW, "rs_may")); break;
    }
}

// Pack overworld palette (drawn by the player: kept as drawn)
static void PackPalette(const struct MpAppearance *app, const struct MpPack *pack, u8 state, u16 *dst)
{
    memcpy(dst, pack->pal[state == ST_UNDERWATER ? PACK_PAL_DIVE : PACK_PAL_OVERWORLD], 32);
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
        const u16 *pal = MP_Avatar_NpcPalette(app->gfxId);
        if (pal)
        {
            char cfg[48];
            memcpy(dst, pal, 32);
            MP_Avatar_SpriteConfigName(app, cfg, sizeof(cfg));
            RecolorByMap(app, dst, FindColorMap(CMAP_OW, cfg));
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
    {
        // Bike, surf...: Brendan's or May's sprites, so their hair layout
        struct MpAppearance g = *app;
        memcpy(dst, female ? sOrigPalMay : sOrigPalBrendan, 32);
        g.base = female ? MP_BASE_MAY : MP_BASE_BRENDAN;
        ApplyRecolor(&g, dst);
        return;
    }
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
// Trainer pictures (trainer card, battles). They match the overworld sprite
// unless LOOK > TRAINER PIC / BACK PIC picks another one, and they take the
// same SKIN / HAIR / OUTFIT / ACCENT colors (except credited fan art).
// Front choices: Emerald's trainer pics, then FRLG and Showdown ones.
// Back choices: Emerald's back pics, then FRLG ones.

static const char *const sEmeraldBackNames[EMERALD_BACK_COUNT][2] = {
    {"BRENDAN", "brendan"}, {"MAY", "may"}, {"RED", "red"}, {"LEAF", "leaf"},
    {"RS BRENDAN", "rs_brendan"}, {"RS MAY", "rs_may"}, {"WALLY", "wally"}, {"STEVEN", "steven"},
};

u16 MP_Avatar_FrontChoiceCount(void) { return FRONT_DEFAULT + 1; }
u8 MP_Avatar_BackChoiceCount(void) { return BACK_NONE + 1; }

const char *MP_Avatar_FrontSelName(u16 sel)
{
    if (sel == 0)
        return "MATCH";
    if (sel == FRONT_DEFAULT)
        return "DEFAULT";
    sel--;
    if (sel < EMERALD_FRONT_COUNT)
        return sMpEmeraldFrontNames[sel][0];
    if (sel < FRONT_CHOICES)
        return sMpExtFronts[sel - EMERALD_FRONT_COUNT].name;
    return "?";
}

const char *MP_Avatar_FrontSelArtist(u16 sel)
{
    if (sel > EMERALD_FRONT_COUNT && sel <= FRONT_CHOICES)
        return sMpExtFronts[sel - 1 - EMERALD_FRONT_COUNT].artist;
    return "";
}

bool8 MP_Avatar_FrontSelRecolors(u16 sel)
{
    if (sel == FRONT_DEFAULT)
        return TRUE;
    return sel == 0 || FindColorMap(CMAP_FRONT, MP_Avatar_FrontSelConfigName(sel)) != NULL;
}

const char *MP_Avatar_FrontSelConfigName(u16 sel)
{
    if (sel == FRONT_DEFAULT)
        return "default";
    if (sel == 0 || sel > FRONT_CHOICES)
        return "match";
    sel--;
    if (sel < EMERALD_FRONT_COUNT)
        return sMpEmeraldFrontNames[sel][1];
    return sMpExtFronts[sel - EMERALD_FRONT_COUNT].cfg;
}

u16 MP_Avatar_ParseFrontSel(const char *name)
{
    u16 i;
    for (i = 1; i <= FRONT_DEFAULT; i++)
        if (strcmp(MP_Avatar_FrontSelConfigName(i), name) == 0)
            return i;
    return 0;
}

const char *MP_Avatar_BackSelName(u8 sel)
{
    if (sel == 0)
        return "MATCH";
    if (sel == BACK_NONE)
        return "NONE";
    sel--;
    if (sel < EMERALD_BACK_COUNT)
        return sEmeraldBackNames[sel][0];
    if (sel < BACK_CHOICES)
        return sMpExtBackNames[sel - EMERALD_BACK_COUNT][0];
    return "?";
}

const char *MP_Avatar_BackSelConfigName(u8 sel)
{
    static char buf[40];
    if (sel == BACK_NONE)
        return "none";
    if (sel == 0 || sel > BACK_CHOICES)
        return "match";
    sel--;
    if (sel < EMERALD_BACK_COUNT)
        return sEmeraldBackNames[sel][1];
    (void)buf;
    return sMpExtBackNames[sel - EMERALD_BACK_COUNT][1];
}

u8 MP_Avatar_ParseBackSel(const char *name)
{
    u8 i;
    for (i = 1; i <= BACK_NONE; i++)
        if (strcmp(MP_Avatar_BackSelConfigName(i), name) == 0)
            return i;
    return 0;
}

// Default/custom bases become Brendan or May
static struct MpAppearance Resolved(const struct MpAppearance *a)
{
    struct MpAppearance app = *a;
    if (app.base == MP_BASE_DEFAULT || app.base == MP_BASE_CUSTOM || app.base >= MP_BASE_COUNT)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    return app;
}

// Front choice of a look, -1 = none (the game's default)
static int FrontChoice(const struct MpAppearance *app)
{
    const struct MpSpriteEntry *e;
    if (app->frontSel == FRONT_DEFAULT)
        return app->gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN;
    if (app->frontSel)
        return app->frontSel - 1 < FRONT_CHOICES ? app->frontSel - 1 : -1;
    switch (app->base)
    {
    case MP_BASE_BRENDAN:    return TRAINER_PIC_BRENDAN;
    case MP_BASE_MAY:        return TRAINER_PIC_MAY;
    case MP_BASE_RS_BRENDAN: return TRAINER_PIC_RS_BRENDAN;
    case MP_BASE_RS_MAY:     return TRAINER_PIC_RS_MAY;
    case MP_BASE_NPC:
        e = MP_Avatar_FindCatalog(app->gfxId);
        if (e && e->frontPic != MP_PIC_NONE && e->frontPic < FRONT_CHOICES)
            return e->frontPic;
        break;
    }
    return -1;
}

static int BackChoice(const struct MpAppearance *app)
{
    const struct MpSpriteEntry *e;
    if (app->backSel)
        return app->backSel - 1 < BACK_CHOICES ? app->backSel - 1 : -1;
    switch (app->base)
    {
    case MP_BASE_BRENDAN:    return TRAINER_BACK_PIC_BRENDAN;
    case MP_BASE_MAY:        return TRAINER_BACK_PIC_MAY;
    case MP_BASE_RS_BRENDAN: return TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN;
    case MP_BASE_RS_MAY:     return TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY;
    case MP_BASE_NPC:
        e = MP_Avatar_FindCatalog(app->gfxId);
        if (e && e->backPic != MP_PIC_NONE && e->backPic < BACK_CHOICES)
            return e->backPic;
        break;
    }
    return -1;
}

static bool8 FrontChoiceTiles(int c, u8 *tiles, u16 *pal, bool8 *canRecolor)
{
    *canRecolor = TRUE;
    if (c < 0 || c >= FRONT_CHOICES)
        return FALSE;
    if (c < EMERALD_FRONT_COUNT)
    {
        LZ77UnCompWram(gTrainerFrontPicTable[c].data, tiles);
        LZ77UnCompWram(gTrainerFrontPicPaletteTable[c].data, pal);
        return TRUE;
    }
    c -= EMERALD_FRONT_COUNT;
    memcpy(tiles, sExtFrontTiles + c * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE);
    memcpy(pal, sExtFrontPals + c * 16, 32);
    *canRecolor = sMpExtFronts[c].recolor;
    return TRUE;
}

// 4 frames in Brendan's order (stand, throw..., after the throw)
static bool8 BackChoiceTiles(int b, u8 *tiles, u16 *pal)
{
    static const u8 sFiveFrameOrder[4] = {0, 1, 3, 4};   // Red and Leaf
    int i;
    if (b < 0 || b >= BACK_CHOICES)
        return FALSE;
    if (b < EMERALD_BACK_COUNT)
    {
        const u8 *src = (const u8 *)gTrainerBackPicTable[b].data;
        u32 frames = gTrainerBackPicTable[b].size / TRAINER_PIC_SIZE;
        for (i = 0; i < 4; i++)
        {
            u32 f = frames >= 5 ? sFiveFrameOrder[i] : ((u32)i < frames ? (u32)i : 0);
            memcpy(tiles + i * TRAINER_PIC_SIZE, src + f * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE);
        }
        LZ77UnCompWram(gTrainerBackPicPaletteTable[b].data, pal);
        return TRUE;
    }
    b -= EMERALD_BACK_COUNT;
    memcpy(tiles, sExtBackTiles + b * 4 * TRAINER_PIC_SIZE, 4 * TRAINER_PIC_SIZE);
    memcpy(pal, sExtBackPals + b * 16, 32);
    return TRUE;
}

// The look's front picture. FALSE = nothing picked (the game's default)
static bool8 BuildFront(const struct MpAppearance *appIn, const struct MpPack *pack, u8 *tiles, u16 *pal, bool8 recolor)
{
    struct MpAppearance app = Resolved(appIn);
    bool8 canRecolor = TRUE;
    int c = FrontChoice(&app);
    if (!app.frontSel && pack && pack->slot[PACK_FRONT])
    {
        memcpy(tiles, pack->slot[PACK_FRONT], TRAINER_PIC_SIZE);
        memcpy(pal, pack->pal[PACK_PAL_FRONT], 32);
        return TRUE;
    }
    if (!FrontChoiceTiles(c, tiles, pal, &canRecolor))
    {
        // Characters without a picture show Brendan/May, in the look's colors
        if (!recolor || !WantsRecolor(&app))
            return FALSE;
        c = app.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN;
        FrontChoiceTiles(c, tiles, pal, &canRecolor);
    }
    if (recolor && canRecolor)
        RecolorByMap(&app, pal, FindColorMap(CMAP_FRONT, MP_Avatar_FrontSelConfigName(c + 1)));
    return TRUE;
}

static bool8 BuildBack(const struct MpAppearance *appIn, const struct MpPack *pack, u8 *tiles, u16 *pal, bool8 recolor)
{
    struct MpAppearance app = Resolved(appIn);
    int b = BackChoice(&app);
    if (!app.backSel && pack && pack->slot[PACK_BACK])
    {
        memcpy(tiles, pack->slot[PACK_BACK], 4 * TRAINER_PIC_SIZE);
        memcpy(pal, pack->pal[PACK_PAL_BACK], 32);
        return TRUE;
    }
    if (!BackChoiceTiles(b, tiles, pal))
        return FALSE;
    if (recolor)
        RecolorByMap(&app, pal, FindColorMap(CMAP_BACK, MP_Avatar_BackSelConfigName(b + 1)));
    return TRUE;
}

static u8 sPicBuf[4 * TRAINER_PIC_SIZE];

// Front pic id for a look: an Emerald pic as is, or the runtime slot 0 (local) / 1 (others)
static u16 FrontPicId(const struct MpAppearance *appIn, const struct MpPack *pack, int which)
{
    struct MpAppearance app = Resolved(appIn);
    int c = FrontChoice(&app);
    bool8 usePack = !app.frontSel && pack && pack->slot[PACK_FRONT];
    u16 pal[16];
    if (!usePack && c < EMERALD_FRONT_COUNT && !WantsRecolor(&app))
        return c < 0 ? 0xFFFF : (u16)c;
    if (!BuildFront(&app, pack, sPicBuf, pal, TRUE))
        return 0xFFFF;
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPicLz[which], sPicBuf, TRAINER_PIC_SIZE);
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPalLz[which], pal, 32);
    return which == 0 ? TRAINER_PIC_MP_LOCAL : TRAINER_PIC_MP_OPPONENT;
}

static u16 BackPicId(const struct MpAppearance *appIn, const struct MpPack *pack, u8 *tiles, u32 *palLz, u16 slotPic)
{
    struct MpAppearance app = Resolved(appIn);
    int b = BackChoice(&app);
    bool8 usePack = !app.backSel && pack && pack->slot[PACK_BACK];
    u16 pal[16];
    if (!usePack && (b < 0 || (b < EMERALD_BACK_COUNT && !WantsRecolor(&app))))
        return b < 0 ? 0xFFFF : (u16)b;
    if (!BuildBack(&app, pack, tiles, pal, TRUE))
        return 0xFFFF;
    MP_Pack_WriteLz77Literal((u8 *)palLz, pal, 32);
    return slotPic;
}

// The Emerald front pic that matches a look (no runtime pictures), 0xFFFF = none
u16 MP_Avatar_FrontPicFor(const struct MpAppearance *app)
{
    struct MpAppearance r = Resolved(app);
    int c = FrontChoice(&r);
    return (c >= 0 && c < EMERALD_FRONT_COUNT) ? (u16)c : 0xFFFF;
}

bool8 MP_Avatar_GetFrontTiles(const struct MpAppearance *app, u8 *tiles, u16 *pal)
{
    return BuildFront(app, NULL, tiles, pal, FALSE);
}

bool8 MP_Avatar_GetBackTiles(const struct MpAppearance *app, u8 *tiles, u16 *pal)
{
    return BuildBack(app, NULL, tiles, pal, FALSE);
}

bool8 MP_Avatar_PreviewFront(u8 *tiles, u16 *pal)
{
    struct MpAppearance app = GetLocalAppearance();
    bool8 canRecolor;
    if (BuildFront(&app, LocalPack(), tiles, pal, TRUE))
        return TRUE;
    // What the game shows by default
    FrontChoiceTiles(app.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN, tiles, pal, &canRecolor);
    return TRUE;
}

bool8 MP_Avatar_PreviewBack(u8 *tiles, u16 *pal)
{
    struct MpAppearance app = GetLocalAppearance();
    if (!BuildBack(&app, LocalPack(), sPicBuf, pal, TRUE))
        return FALSE;
    memcpy(tiles, sPicBuf, TRAINER_PIC_SIZE);
    return TRUE;
}

u16 MP_Avatar_LocalFrontPic(void)
{
    struct MpAppearance app = GetLocalAppearance();
    return FrontPicId(&app, LocalPack(), 0);
}

u16 MP_Avatar_RemoteFrontPic(u8 id)
{
    struct MpPack buf;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    return FrontPicId(&gMpPlayers[id].app, RemotePack(id, &buf), 1);
}

u16 MP_LocalFrontPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalFrontPic();
    return pic == 0xFFFF ? defaultPic : pic;
}

u16 MP_Avatar_LocalBackPic(void)
{
    struct MpAppearance app = GetLocalAppearance();
    return BackPicId(&app, LocalPack(), gMpTrainerBackTiles, gMpTrainerBackPalLz, TRAINER_BACK_PIC_MP_LOCAL);
}

// Another player's battle back pic (co-op partner), 0xFFFF = game default
u16 MP_Avatar_RemoteBackPic(u8 id)
{
    struct MpPack buf;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    return BackPicId(&gMpPlayers[id].app, RemotePack(id, &buf), gMpPartnerBackTiles, gMpPartnerBackPalLz, TRAINER_BACK_PIC_MP_PARTNER);
}

bool8 MP_Avatar_RemoteHidesBack(u8 id)
{
    return id < MP_MAX_PLAYERS && gMpPlayers[id].present
        && (gMpPlayers[id].app.base == MP_BASE_NPC || gMpPlayers[id].app.backSel == BACK_NONE)
        && MP_Avatar_RemoteBackPic(id) == 0xFFFF;
}

u16 MP_LocalBackPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalBackPic();
    return pic == 0xFFFF ? defaultPic : pic;
}

bool8 MP_LocalHidesTrainerBack(void)
{
    return (gMpConfig.app.base == MP_BASE_NPC || gMpConfig.app.backSel == BACK_NONE) && MP_Avatar_LocalBackPic() == 0xFFFF;
}

// LOOK > START: the pictures that go with the sprite; when it has none, ID PIC
// becomes DEFAULT (Brendan/May) and BK PIC becomes NONE
void MP_Avatar_MatchPictures(struct MpAppearance *appIn, bool8 front, bool8 back)
{
    struct MpAppearance app;
    const struct MpPack *pack = (appIn == &gMpConfig.app) ? LocalPack() : NULL;
    if (front)
    {
        appIn->frontSel = 0;
        app = Resolved(appIn);
        if (FrontChoice(&app) < 0 && !(pack && pack->slot[PACK_FRONT]))
            appIn->frontSel = FRONT_DEFAULT;
    }
    if (back)
    {
        appIn->backSel = 0;
        app = Resolved(appIn);
        if (BackChoice(&app) < 0 && !(pack && pack->slot[PACK_BACK]))
            appIn->backSel = BACK_NONE;
    }
}

// ---------------------------------------------------------------------------
// Test harness: writes every recolorable sprite and picture with its palette
// (for tools/moemon_frlg/colormaps.py). index.txt: kind cfg width height frames file

static void DumpAsset(FILE *idx, const char *dir, const char *kind, const char *cfg, int w, int h,
                      const u8 *const *frames, int nFrames, const u16 *pal)
{
    char path[300];
    FILE *f;
    int i;
    snprintf(path, sizeof(path), "%s/%s_%s.bin", dir, kind, cfg);
    f = fopen(path, "wb");
    if (!f)
        return;
    for (i = 0; i < nFrames; i++)
        fwrite(frames[i], 1, w * h / 2, f);
    fwrite(pal, 1, 32, f);
    fclose(f);
    fprintf(idx, "%s %s %d %d %d %s_%s.bin\n", kind, cfg, w, h, nFrames, kind, cfg);
}

void MP_Avatar_DumpColorAssets(const char *dir)
{
    char path[300], cfg[48];
    FILE *idx;
    u16 i, pal[16];
    const u8 *fr[3];
    static const struct { const char *cfg; const struct ObjectEventGraphicsInfo *info; u16 tag; } sBases[] = {
        {"brendan", &gObjectEventGraphicsInfo_BrendanNormal, OBJ_EVENT_PAL_TAG_BRENDAN},
        {"may", &gObjectEventGraphicsInfo_MayNormal, OBJ_EVENT_PAL_TAG_MAY},
        {"rs_brendan", &gObjectEventGraphicsInfo_RubySapphireBrendan, OBJ_EVENT_PAL_TAG_RS_BRENDAN},
        {"rs_may", &gObjectEventGraphicsInfo_RubySapphireMay, OBJ_EVENT_PAL_TAG_RS_MAY},
    };

    MP_Avatar_Init();
    snprintf(path, sizeof(path), "%s/index.txt", dir);
    idx = fopen(path, "w");
    if (!idx)
    {
        printf("[test] dumpcolors: can't write %s\n", path);
        return;
    }
    for (i = 0; i < ARRAY_COUNT(sBases); i++)
    {
        const struct ObjectEventGraphicsInfo *info = sBases[i].info;
        fr[0] = info->images[0].data; fr[1] = info->images[1].data; fr[2] = info->images[2].data;
        DumpAsset(idx, dir, "ow", sBases[i].cfg, info->width, info->height, fr, 3, MP_Avatar_OriginalPalette(sBases[i].tag));
    }
    for (i = 0; i < CATALOG_COUNT; i++)
    {
        const struct MpSpriteEntry *e = CatalogAt(i);
        const struct ObjectEventGraphicsInfo *info = MP_Avatar_NpcInfo(e->gfxId);
        struct MpAppearance app;
        if (!info || !MP_Avatar_NpcPalette(e->gfxId))
            continue;
        memset(&app, 0, sizeof(app));
        app.base = MP_BASE_NPC;
        app.gfxId = e->gfxId;
        MP_Avatar_SpriteConfigName(&app, cfg, sizeof(cfg));
        fr[0] = info->images[0].data; fr[1] = info->images[1].data; fr[2] = info->images[2].data;
        DumpAsset(idx, dir, "ow", cfg, info->width, info->height, fr, 3, MP_Avatar_NpcPalette(e->gfxId));
    }
    for (i = 1; i <= FRONT_CHOICES; i++)
    {
        bool8 canRecolor;
        if (!FrontChoiceTiles(i - 1, sPicBuf, pal, &canRecolor) || !canRecolor)
            continue;
        fr[0] = sPicBuf;
        DumpAsset(idx, dir, "front", MP_Avatar_FrontSelConfigName(i), 64, 64, fr, 1, pal);
    }
    for (i = 1; i <= BACK_CHOICES; i++)
    {
        if (!BackChoiceTiles(i - 1, sPicBuf, pal))
            continue;
        fr[0] = sPicBuf;
        DumpAsset(idx, dir, "back", MP_Avatar_BackSelConfigName(i), 64, 64, fr, 1, pal);
    }
    fclose(idx);
    printf("[test] dumpcolors: wrote %s\n", path);
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
    return MP_Pack_Export(pathOut, &app, LocalPack());
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
    choice = NUM_BASE_CHOICES + CATALOG_COUNT + i;
    MP_Avatar_SetChoice(&gMpConfig.app, choice);
    if (!gMpConfig.app.hasCustom)
    {
        snprintf(err, errLen, "couldn't load %s", dst);
        return FALSE;
    }
    return TRUE;
}
