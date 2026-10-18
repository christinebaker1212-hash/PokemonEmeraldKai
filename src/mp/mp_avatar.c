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
    "DEFAULT", "BRENDAN", "MAY", "RS BRENDAN", "RS MAY", "CUSTOM", "NPC",
};

#include "constants/trainers.h"
#include "mp_sprite_catalog.h"

extern const struct ObjectEventGraphicsInfo *const gObjectEventGraphicsInfoPointers[];
static const char *const sSkinNames[MP_SKIN_COUNT] = {
    "DEFAULT", "PORCELAIN", "LIGHT", "TAN", "BROWN", "DARK", "DEEP",
};
static const char *const sColorNames[MP_COLOR_COUNT] = {
    "DEFAULT", "RED", "BLUE", "GREEN", "YELLOW", "PURPLE", "PINK", "ORANGE", "BLACK", "WHITE", "TEAL",
};

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

// Choices: DEFAULT, BRENDAN, MAY, RS BRENDAN, RS MAY, [CUSTOM], catalog...
static u16 NumBaseChoices(void)
{
    return MP_Avatar_LocalCustomBlob() ? MP_BASE_CUSTOM + 1 : MP_BASE_CUSTOM;
}

u16 MP_Avatar_ChoiceCount(void)
{
    return NumBaseChoices() + ARRAY_COUNT(sSpriteCatalog);
}

u16 MP_Avatar_GetChoice(const struct MpAppearance *app)
{
    u16 i;
    if (app->base == MP_BASE_NPC)
    {
        for (i = 0; i < ARRAY_COUNT(sSpriteCatalog); i++)
            if (sSpriteCatalog[i].gfxId == app->gfxId)
                return NumBaseChoices() + i;
        return 0;
    }
    if (app->base < NumBaseChoices())
        return app->base;
    return 0;
}

void MP_Avatar_SetChoice(struct MpAppearance *app, u16 choice)
{
    if (choice < NumBaseChoices())
    {
        app->base = (u8)choice;
        app->gfxId = 0;
    }
    else if (choice - NumBaseChoices() < ARRAY_COUNT(sSpriteCatalog))
    {
        app->base = MP_BASE_NPC;
        app->gfxId = sSpriteCatalog[choice - NumBaseChoices()].gfxId;
    }
}

const char *MP_Avatar_ChoiceName(u16 choice)
{
    if (choice < NumBaseChoices())
        return sBaseNames[choice];
    if (choice - NumBaseChoices() < ARRAY_COUNT(sSpriteCatalog))
        return sSpriteCatalog[choice - NumBaseChoices()].name;
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
        ToConfigName(out, MP_Avatar_BaseName(app->base == MP_BASE_NPC ? MP_BASE_DEFAULT : app->base), len);
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
            app->base = (u8)i;
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

static const struct ObjectEventGraphicsInfo *NpcInfo(u16 gfxId)
{
    if (MP_Avatar_FindCatalog(gfxId) == NULL)
        return NULL;
    return gObjectEventGraphicsInfoPointers[gfxId];
}

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

u16 MP_Avatar_LocalFrontPic(void)
{
    return MP_Avatar_FrontPicFor(&gMpConfig.app);
}

u16 MP_LocalFrontPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalFrontPic();
    return pic == 0xFFFF ? defaultPic : pic;
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

u16 MP_Avatar_LocalBackPic(void)
{
    const struct MpSpriteEntry *e;
    switch (gMpConfig.app.base)
    {
    case MP_BASE_BRENDAN:    return TRAINER_BACK_PIC_BRENDAN;
    case MP_BASE_MAY:        return TRAINER_BACK_PIC_MAY;
    case MP_BASE_RS_BRENDAN: return TRAINER_BACK_PIC_RUBY_SAPPHIRE_BRENDAN;
    case MP_BASE_RS_MAY:     return TRAINER_BACK_PIC_RUBY_SAPPHIRE_MAY;
    case MP_BASE_NPC:
        e = MP_Avatar_FindCatalog(gMpConfig.app.gfxId);
        if (e && e->backPic != MP_PIC_NONE)
            return e->backPic;
        break;
    }
    return 0xFFFF;
}

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

static const struct ObjectEventGraphicsInfo *GetBaseInfo(u8 base, u16 gfxId, u8 gender, u8 avatarMode)
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
    case MP_BASE_NPC:        return NpcInfo(gfxId) ? NpcInfo(gfxId) : (gender == FEMALE ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal);
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

void MP_Avatar_GetPalette(const struct MpAppearance *app, const u8 *customGfx, u8 avatarMode, u16 *dst)
{
    if (avatarMode == MP_AVATAR_FOOT && app->base == MP_BASE_CUSTOM && customGfx)
    {
        u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
        memcpy(dst, customGfx + MP_CUSTOM_TILES_SIZE, 32);
        CountTileUsage(customGfx, 9 * 256, counts);
        CountHeadAndBody(customGfx, 2, 4, head, body);
        AutoRecolor(app, dst, counts, body, head);
        return;
    }
    if (avatarMode == MP_AVATAR_FOOT && app->base == MP_BASE_NPC && NpcInfo(app->gfxId))
    {
        const struct ObjectEventGraphicsInfo *npc = NpcInfo(app->gfxId);
        const u16 *pal = MP_GetObjectEventPaletteData(npc->paletteTag);
        if (pal)
        {
            u32 counts[16] = {0}, body[16] = {0}, head[16] = {0};
            int h = npc->height;
            u8 f;
            memcpy(dst, pal, 32);
            for (f = 0; f < 9; f++)
                CountTileUsage(npc->images[f].data, npc->images[f].size, counts);
            CountHeadAndBody(npc->images[0].data, npc->width / 8, h / 8, head, body);
            AutoRecolor(app, dst, counts, body, head);
            return;
        }
    }
    memcpy(dst, GetOrigBasePalette(app->base, app->gender, avatarMode), 32);
    if (avatarMode != MP_AVATAR_UNDERWATER)
        ApplyRecolor(app, dst);
}

// Builds a graphics info for the given appearance into sMpGfxInfo[slot]
static void BuildGfxInfo(int slot, const struct MpAppearance *app, const u8 *customGfx, u8 avatarMode)
{
    const struct ObjectEventGraphicsInfo *base = GetBaseInfo(app->base, app->gfxId, app->gender, avatarMode);
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[slot];
    int i;

    *info = *base;
    if (avatarMode == MP_AVATAR_FOOT && app->base == MP_BASE_NPC && NpcInfo(app->gfxId))
    {
        // NPC sheets have 9 walking frames: running reuses them with the
        // player's animation table. Size, shape and shadow stay the NPC's own.
        const struct ObjectEventGraphicsInfo *anims = (app->gender == FEMALE) ? &gObjectEventGraphicsInfo_MayNormal : &gObjectEventGraphicsInfo_BrendanNormal;
        for (i = 0; i < MP_CUSTOM_FRAMES; i++)
            sMpFrames[slot][i] = base->images[i % 9];
        info->images = sMpFrames[slot];
        info->anims = anims->anims;
        info->inanimate = FALSE;
    }
    else if (avatarMode == MP_AVATAR_FOOT)
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
