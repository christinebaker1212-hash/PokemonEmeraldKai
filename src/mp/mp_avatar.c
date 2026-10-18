// Moemon Co-op: player appearance (base sprite, skin tone, outfit colors, custom PNG sprites)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO_WRAPPERS
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "third_party/stb_image.h"

#include "global.h"
#include "mp.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "palette.h"
#include "sprite.h"
#include "constants/event_object_movement.h"
#include "constants/rgb.h"

#define OBJ_EVENT_PAL_TAG_BRENDAN           0x1100
#define OBJ_EVENT_PAL_TAG_MAY               0x1110
#define OBJ_EVENT_PAL_TAG_PLAYER_UNDERWATER 0x1115
#define OBJ_EVENT_PAL_TAG_RS_BRENDAN        0x1122
#define OBJ_EVENT_PAL_TAG_RS_MAY            0x1123
#define OBJ_EVENT_PAL_TAG_MP_LOCAL          0x1130

const u16 *MP_GetObjectEventPaletteData(u16 tag);
void MP_SetObjectEventPaletteData(u16 tag, const u16 *data);

extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_BrendanNormal;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_BrendanMachBike;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_BrendanAcroBike;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_BrendanSurfing;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_BrendanUnderwater;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_MayNormal;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_MayMachBike;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_MayAcroBike;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_MaySurfing;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_MayUnderwater;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_RubySapphireBrendan;
extern const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_RubySapphireMay;

// ---------------------------------------------------------------------------
// Presets. Skin replaces palette indices 1-3, outfit 12-13, accent 10-11.

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
    "DEFAULT", "BRENDAN", "MAY", "RS BRENDAN", "RS MAY", "CUSTOM",
};
static const char *const sSkinNames[MP_SKIN_COUNT] = {
    "DEFAULT", "PORCELAIN", "LIGHT", "TAN", "BROWN", "DARK", "DEEP",
};
static const char *const sColorNames[MP_COLOR_COUNT] = {
    "DEFAULT", "RED", "BLUE", "GREEN", "YELLOW", "PURPLE", "PINK", "ORANGE", "BLACK", "WHITE", "TEAL",
};

const char *MP_Avatar_BaseName(u8 base) { return base < MP_BASE_COUNT ? sBaseNames[base] : "?"; }
const char *MP_Avatar_SkinName(u8 skin) { return skin < MP_SKIN_COUNT ? sSkinNames[skin] : "?"; }
const char *MP_Avatar_ColorName(u8 c) { return c < MP_COLOR_COUNT ? sColorNames[c] : "?"; }

// ---------------------------------------------------------------------------
// Graphics info storage: [0] = local custom avatar, [1 + id] = remote players

#define NUM_MP_GFX (1 + MP_MAX_PLAYERS)
static struct ObjectEventGraphicsInfo sMpGfxInfo[NUM_MP_GFX];
static struct SpriteFrameImage sMpFrames[NUM_MP_GFX][MP_CUSTOM_FRAMES];
static u16 sOrigPalBrendan[16], sOrigPalMay[16], sOrigPalRSBrendan[16], sOrigPalRSMay[16];
static u16 sLocalPalBrendan[16], sLocalPalMay[16], sLocalPalCustom[16];
static u8 sLocalCustomBlob[MP_CUSTOM_BLOB_SIZE];
static bool8 sLocalHasCustomBlob;
static bool8 sAvatarInit;
static void RebuildLocal(void);

static const struct ObjectEventGraphicsInfo *GetBaseInfo(u8 base, u8 gender, u8 avatarMode)
{
    if (avatarMode != MP_AVATAR_FOOT)
    {
        bool8 female = (gender == FEMALE);
        switch (avatarMode)
        {
        case MP_AVATAR_MACH_BIKE: return female ? &gObjectEventGraphicsInfo_MayMachBike : &gObjectEventGraphicsInfo_BrendanMachBike;
        case MP_AVATAR_ACRO_BIKE: return female ? &gObjectEventGraphicsInfo_MayAcroBike : &gObjectEventGraphicsInfo_BrendanAcroBike;
        case MP_AVATAR_SURF:      return female ? &gObjectEventGraphicsInfo_MaySurfing : &gObjectEventGraphicsInfo_BrendanSurfing;
        default:                  return female ? &gObjectEventGraphicsInfo_MayUnderwater : &gObjectEventGraphicsInfo_BrendanUnderwater;
        }
    }
    switch (base)
    {
    case MP_BASE_BRENDAN:    return &gObjectEventGraphicsInfo_BrendanNormal;
    case MP_BASE_MAY:        return &gObjectEventGraphicsInfo_MayNormal;
    case MP_BASE_RS_BRENDAN: return &gObjectEventGraphicsInfo_RubySapphireBrendan;
    case MP_BASE_RS_MAY:     return &gObjectEventGraphicsInfo_RubySapphireMay;
    default:                 return gender == FEMALE ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal;
    }
}

static const u16 *GetOrigBasePalette(u8 base, u8 gender, u8 avatarMode)
{
    if (avatarMode == MP_AVATAR_UNDERWATER)
        return MP_GetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_PLAYER_UNDERWATER);
    if (avatarMode != MP_AVATAR_FOOT)
        return gender == FEMALE ? sOrigPalMay : sOrigPalBrendan;
    switch (base)
    {
    case MP_BASE_BRENDAN:    return sOrigPalBrendan;
    case MP_BASE_MAY:        return sOrigPalMay;
    case MP_BASE_RS_BRENDAN: return sOrigPalRSBrendan;
    case MP_BASE_RS_MAY:     return sOrigPalRSMay;
    default:                 return gender == FEMALE ? sOrigPalMay : sOrigPalBrendan;
    }
}

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

void MP_Avatar_GetPalette(const struct MpAppearance *app, const u8 *customGfx, u8 avatarMode, u16 *dst)
{
    if (avatarMode == MP_AVATAR_FOOT && app->base == MP_BASE_CUSTOM && customGfx)
    {
        memcpy(dst, customGfx + MP_CUSTOM_TILES_SIZE, 32);
        return;
    }
    memcpy(dst, GetOrigBasePalette(app->base, app->gender, avatarMode), 32);
    if (avatarMode != MP_AVATAR_UNDERWATER)
        ApplyRecolor(app, dst);
}

// Builds a graphics info for the given appearance into sMpGfxInfo[slot]
static void BuildGfxInfo(int slot, const struct MpAppearance *app, const u8 *customGfx, u8 avatarMode)
{
    const struct ObjectEventGraphicsInfo *base = GetBaseInfo(app->base, app->gender, avatarMode);
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[slot];
    int i;

    *info = *base;
    if (avatarMode == MP_AVATAR_FOOT)
    {
        // Every on-foot avatar gets 18 frames (9 walk + 9 run) and the player anim table
        const struct ObjectEventGraphicsInfo *anims = (app->gender == FEMALE) ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal;
        bool8 hasRunFrames = (base == &gObjectEventGraphicsInfo_BrendanNormal || base == &gObjectEventGraphicsInfo_MayNormal);

        for (i = 0; i < MP_CUSTOM_FRAMES; i++)
        {
            if (app->base == MP_BASE_CUSTOM && customGfx)
            {
                sMpFrames[slot][i].data = customGfx + i * 256;
                sMpFrames[slot][i].size = 256;
            }
            else
            {
                sMpFrames[slot][i] = base->images[hasRunFrames ? i : (i % 9)];
            }
        }
        info->images = sMpFrames[slot];
        info->anims = anims->anims;
        info->size = 256;
        info->width = 16;
        info->height = 32;
        info->oam = anims->oam;
        info->subspriteTables = anims->subspriteTables;
    }

    if (slot == 0)
    {
        info->paletteSlot = PALSLOT_PLAYER;
        if (avatarMode == MP_AVATAR_FOOT)
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_LOCAL;
    }
    else
    {
        // The overworld code assigns the real palette slot after spawning
        info->paletteSlot = PALSLOT_NPC_1;
        info->paletteTag = 0x1380 + slot;
        info->disableReflectionPaletteLoad = TRUE;
    }
}

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
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_BRENDAN, sLocalPalBrendan);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MAY, sLocalPalMay);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MP_LOCAL, sLocalPalCustom);

    sLocalHasCustomBlob = FALSE;
    if (gMpConfig.customSprite[0])
    {
        char err[160];
        if (MP_Avatar_LoadCustomPng(gMpConfig.customSprite, sLocalCustomBlob, err, sizeof(err)))
            sLocalHasCustomBlob = TRUE;
        else
            printf("[mp] custom sprite not loaded: %s\n", err);
    }
    if (gMpConfig.app.base == MP_BASE_CUSTOM && !sLocalHasCustomBlob)
        gMpConfig.app.base = MP_BASE_DEFAULT;
    gMpConfig.app.hasCustom = sLocalHasCustomBlob;
    sAvatarInit = TRUE;
}

const u8 *MP_Avatar_LocalCustomBlob(void)
{
    return sLocalHasCustomBlob ? sLocalCustomBlob : NULL;
}

u32 MP_Avatar_LocalCustomBlobSize(void)
{
    return sLocalHasCustomBlob ? MP_CUSTOM_BLOB_SIZE : 0;
}

bool8 MP_Avatar_IsValidBlob(const u8 *data, u32 size)
{
    return data != NULL && size == MP_CUSTOM_BLOB_SIZE;
}

static struct MpAppearance GetLocalAppearance(void)
{
    struct MpAppearance app = gMpConfig.app;
    app.gender = gSaveBlock2Ptr->playerGender;
    if (app.base == MP_BASE_DEFAULT)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    return app;
}

bool8 MP_LocalUsesCustomAvatar(void)
{
    struct MpAppearance app;
    if (!sAvatarInit)
        return FALSE;
    app = GetLocalAppearance();
    if (app.base == MP_BASE_CUSTOM)
        return sLocalHasCustomBlob;
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

// Rebuilds the local palettes and custom avatar graphics info
static void RebuildLocal(void)
{
    struct MpAppearance app;
    struct MpAppearance genderApp;

    MP_Avatar_Init();
    app = GetLocalAppearance();

    // Bike/surf/etc sprites use the save file gender's palette, recolored
    genderApp = app;
    genderApp.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    memcpy(sLocalPalBrendan, sOrigPalBrendan, 32);
    memcpy(sLocalPalMay, sOrigPalMay, 32);
    if (app.gender == FEMALE)
        ApplyRecolor(&genderApp, sLocalPalMay);
    else
        ApplyRecolor(&genderApp, sLocalPalBrendan);

    MP_Avatar_GetPalette(&app, MP_Avatar_LocalCustomBlob(), MP_AVATAR_FOOT, sLocalPalCustom);
    BuildGfxInfo(0, &app, MP_Avatar_LocalCustomBlob(), MP_AVATAR_FOOT);
}

// Rebuilds the local avatar and refreshes the player sprite
void MP_Avatar_ApplyLocal(void)
{
    RebuildLocal();
    if (MP_IsInOverworld() && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT
     && gObjectEvents[gPlayerAvatar.objectEventId].active)
    {
        struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
        u8 gfx = GetPlayerAvatarGraphicsIdByCurrentState();
        if (gfx != 0 || (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ON_FOOT))
            ObjectEventSetGraphicsId(player, gfx);
        UpdateSpritePaletteWithWeather(PALSLOT_PLAYER);
    }
}

void MP_Avatar_BuildRemote(u8 id)
{
    struct MpPlayer *p = &gMpPlayers[id];
    struct MpAppearance app = p->app;
    if (app.base == MP_BASE_DEFAULT || app.base >= MP_BASE_COUNT)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    if (app.base == MP_BASE_CUSTOM && !p->customGfx)
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    BuildGfxInfo(1 + id, &app, p->customGfx, p->hasState ? p->state.avatarMode : MP_AVATAR_FOOT);
}

// ---------------------------------------------------------------------------
// Custom PNG sprite sheets.
// Layout: 16x32 frames side by side. 9 frames (144x32, walking only; running
// reuses them) or 18 frames (288x32, 9 walking + 9 running), in the same order
// as graphics/object_events/pics/people/brendan/walking.png + running.png.
// Up to 15 colors plus transparency. Transparent pixels (alpha < 128), or the
// color of the top-left pixel when the image has no alpha, become color 0.

bool8 MP_Avatar_LoadCustomPng(const char *path, u8 *blob, char *err, int errLen)
{
    int w, h, comp, frames, x, y, i;
    unsigned char *px;
    u32 colors[16];
    int numColors = 1;
    bool8 hasAlpha = FALSE;
    u32 keyColor;
    u16 *pal = (u16 *)(blob + MP_CUSTOM_TILES_SIZE);
    FILE *f = fopen(path, "rb");
    unsigned char *fileData;
    long fileSize;

    if (!f)
    {
        snprintf(err, errLen, "cannot open %s", path);
        return FALSE;
    }
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    fileData = malloc(fileSize);
    if (fread(fileData, 1, fileSize, f) != (size_t)fileSize)
    {
        fclose(f);
        free(fileData);
        snprintf(err, errLen, "cannot read %s", path);
        return FALSE;
    }
    fclose(f);
    px = stbi_load_from_memory(fileData, (int)fileSize, &w, &h, &comp, 4);
    free(fileData);
    if (!px)
    {
        snprintf(err, errLen, "%s is not a valid PNG", path);
        return FALSE;
    }
    if (h != 32 || (w != 144 && w != 288))
    {
        snprintf(err, errLen, "sprite sheet must be 144x32 or 288x32 (got %dx%d)", w, h);
        stbi_image_free(px);
        return FALSE;
    }
    frames = w / 16;
    for (i = 0; i < w * h; i++)
        if (px[i * 4 + 3] < 255)
            hasAlpha = TRUE;
    keyColor = px[0] | (px[1] << 8) | (px[2] << 16);
    memset(blob, 0, MP_CUSTOM_BLOB_SIZE);
    colors[0] = 0xFFFFFFFF;

    for (y = 0; y < 32; y++)
    {
        for (x = 0; x < w; x++)
        {
            unsigned char *p = px + (y * w + x) * 4;
            u32 rgb = p[0] | (p[1] << 8) | (p[2] << 16);
            int index = 0, frame, fx, tile, tx, ty;
            u8 *tileData;
            bool8 transparent = hasAlpha ? (p[3] < 128) : (rgb == keyColor);

            if (!transparent)
            {
                // Quantize to 5 bits per channel to merge near-identical colors
                u32 q = (p[0] >> 3) | ((p[1] >> 3) << 5) | ((p[2] >> 3) << 10);
                for (index = 1; index < numColors; index++)
                    if (colors[index] == q)
                        break;
                if (index == numColors)
                {
                    if (numColors == 16)
                    {
                        snprintf(err, errLen, "sprite sheet has more than 15 colors");
                        stbi_image_free(px);
                        return FALSE;
                    }
                    colors[numColors++] = q;
                }
            }
            frame = x / 16;
            fx = x % 16;
            tx = fx / 8;
            ty = y / 8;
            tile = ty * 2 + tx;
            tileData = blob + frame * 256 + tile * 32;
            {
                int pxIndex = (y % 8) * 8 + (fx % 8);
                if (pxIndex & 1)
                    tileData[pxIndex / 2] |= index << 4;
                else
                    tileData[pxIndex / 2] |= index;
            }
        }
    }
    stbi_image_free(px);

    if (frames == 9)
        memcpy(blob + 9 * 256, blob, 9 * 256);

    pal[0] = RGB(14, 24, 20);
    for (i = 1; i < 16; i++)
        pal[i] = (i < numColors) ? (u16)colors[i] : 0;
    // Nametags draw with colors 14 (fill) and 15 (outline); keep them readable
    // only if the sheet doesn't use them.
    if (numColors <= 14)
    {
        pal[14] = RGB(31, 31, 31);
        pal[15] = RGB(0, 0, 0);
    }
    return TRUE;
}
