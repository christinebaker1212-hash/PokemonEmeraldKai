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

#define STBI_ONLY_JPEG   // the card game's printed scans (src/tcg/tcg_gfx.c)
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO_WRAPPERS
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "third_party/stb_image.h"

#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "mp_pack.h"
#include "mp_hybrid.h"
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
#define OBJ_EVENT_PAL_TAG_MP_BODY           0x1133

#define PACK_DIR "sprites"
#define MAX_PACK_FILES 32

const u16 *MP_GetObjectEventPaletteData(u16 tag);
void MP_SetObjectEventPaletteData(u16 tag, const u16 *data);

extern const struct ObjectEventGraphicsInfo *const gObjectEventGraphicsInfoPointers[];

// Runtime trainer pictures from sprite packs (see include/data.h)
u32 gMpTrainerFrontPicLz[2][MP_PIC_LZ_WORDS];
u32 gMpTrainerFrontPalLz[2][MP_PAL_LZ_WORDS];
u32 gMpTrainerBackPalLz[MP_PAL_LZ_WORDS];
u8 gMpTrainerBackTiles[4 * MP_BACK_PIC_SIZE];
u32 gMpPartnerBackPalLz[MP_PAL_LZ_WORDS];
u8 gMpPartnerBackTiles[4 * MP_BACK_PIC_SIZE];
// a Triple Battle's second ally
u32 gMpPartner2BackPalLz[MP_PAL_LZ_WORDS];
u8 gMpPartner2BackTiles[4 * MP_BACK_PIC_SIZE];
const struct SpriteFrameImage gMpPartner2BackFrames[4] = {
    {gMpPartner2BackTiles + 0 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartner2BackTiles + 1 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartner2BackTiles + 2 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartner2BackTiles + 3 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
};
const struct SpriteFrameImage gMpPartnerBackFrames[4] = {
    {gMpPartnerBackTiles + 0 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartnerBackTiles + 1 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartnerBackTiles + 2 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpPartnerBackTiles + 3 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
};
const struct SpriteFrameImage gMpTrainerBackFrames[4] = {
    {gMpTrainerBackTiles + 0 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpTrainerBackTiles + 1 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpTrainerBackTiles + 2 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
    {gMpTrainerBackTiles + 3 * MP_BACK_PIC_SIZE, MP_BACK_PIC_SIZE},
};

// The 96x64 back pictures are drawn as two pieces: 64x64 (tiles 0-63) and 32x64 (tiles 64-95), placed so a
// 64x64 picture MP_BACK_PIC_LEFT pixels in shows where the game's 64x64 sprite would
static const struct Subsprite sWideBackSubsprites[] = {
    {.x = -32 - MP_BACK_PIC_LEFT, .y = -32, .shape = SPRITE_SHAPE(64x64), .size = SPRITE_SIZE(64x64), .tileOffset = 0, .priority = 2},
    {.x = 32 - MP_BACK_PIC_LEFT, .y = -32, .shape = SPRITE_SHAPE(32x64), .size = SPRITE_SIZE(32x64), .tileOffset = 64, .priority = 2},
};
static const struct SubspriteTable sWideBackSubspriteTables[] = {{ARRAY_COUNT(sWideBackSubsprites), sWideBackSubsprites}};

// Called by CreateSpriteAt for every new sprite: one made from a player's back picture gets its two pieces.
// Affine is turned off: the pieces' places would read the matrix number as flip bits (and a back picture
// is never scaled).
void MP_Avatar_OnSpriteCreated(struct Sprite *sprite)
{
    if (sprite->images != gMpTrainerBackFrames && sprite->images != gMpPartnerBackFrames
     && sprite->images != gMpPartner2BackFrames)
        return;
    FreeSpriteOamMatrix(sprite);
    sprite->oam.matrixNum = 0;
    SetSubspriteTables(sprite, sWideBackSubspriteTables);
    sprite->subspriteMode = SUBSPRITES_IGNORE_PRIORITY;
}

// 64x64 frames <-> the 96x64 layout (tile column c of a 64x64 frame is the wide picture's column c + 1)
static u8 *WideTile(u8 *frame, int col, int row)
{
    return col < 8 ? frame + (row * 8 + col) * TILE_SIZE_4BPP : frame + (64 + row * 4 + col - 8) * TILE_SIZE_4BPP;
}

static void BackFramesToWide(const u8 *std, u8 *wide)
{
    int f, row, col;
    memset(wide, 0, 4 * MP_BACK_PIC_SIZE);
    for (f = 0; f < 4; f++)
        for (row = 0; row < 8; row++)
            for (col = 0; col < 8; col++)
                memcpy(WideTile(wide + f * MP_BACK_PIC_SIZE, col + MP_BACK_PIC_LEFT / 8, row),
                       std + f * TRAINER_PIC_SIZE + (row * 8 + col) * TILE_SIZE_4BPP, TILE_SIZE_4BPP);
}

static void BackFramesToStd(const u8 *wide, u8 *std)
{
    int f, row, col;
    for (f = 0; f < 4; f++)
        for (row = 0; row < 8; row++)
            for (col = 0; col < 8; col++)
                memcpy(std + f * TRAINER_PIC_SIZE + (row * 8 + col) * TILE_SIZE_4BPP,
                       WideTile((u8 *)wide + f * MP_BACK_PIC_SIZE, col + MP_BACK_PIC_LEFT / 8, row), TILE_SIZE_4BPP);
}

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
    "DEFAULT", "BRENDAN", "MAY", "RS BRENDAN", "RS MAY", "CUSTOM", "NPC", "CREATED",
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

// ---------------------------------------------------------------------------
// Player characters from Moemon's other branch (tools/moemon_chars/import.py): their own walking and
// running frames, Mach / Acro Bike frames (a bicycle, or a ridden Pokemon or vehicle) and diving frames.
// Surfing, field moves, fishing, watering and decorating are built (mp_hybrid.c).

#define MP_CHAR_FRONT_EMERALD(n) (n)
#define MP_CHAR_FRONT_NEW(n) (0x1000 + (n))
#define MP_CHAR_BACK_EMERALD(n) (n)
#define MP_CHAR_BACK_NEW(n) (0x100 + (n))
#define MP_CHARF_MACH_RIDE 1   // the Mach Bike sheet shows a ridden Pokemon or a vehicle (LOOK > BIKE)
#define MP_CHARF_ACRO_RIDE 2
#define MP_CHARF_GENERIC   4   // a class or a group's member: LOOK > GENERICS

struct MpCharDef
{
    const char *name;
    const char *cfg;      // ini name
    u8 gender;
    u16 front, back;      // MP_CHAR_FRONT_* / MP_CHAR_BACK_*
    u8 flags;
    u32 offset;           // in sCharOwTiles
    u8 bikeSize;          // the bikes' frames: 32x32, or 64x64 (PROF. BIRCH and the ZIGZAGOON chasing him)
};

#include "mp_chars_gfx.h"

static const u8 sCharOwTiles[] = INCBIN_U8("graphics/moemon/chars/ow_tiles.bin");
static const u16 sCharOwPals[] = INCBIN_U16("graphics/moemon/chars/ow_pals.bin");   // field, dive
static const u8 sCharFrontTiles[] = INCBIN_U8("graphics/moemon/chars/front_tiles.bin");
static const u16 sCharFrontPals[] = INCBIN_U16("graphics/moemon/chars/front_pals.bin");
static const u8 sCharBackTiles[] = INCBIN_U8("graphics/moemon/chars/back_tiles.bin");
static const u16 sCharBackPals[] = INCBIN_U16("graphics/moemon/chars/back_pals.bin");

#define EMERALD_FRONT_COUNT TRAINER_PIC_MP_LOCAL          // Emerald's own trainer pics come first
#define CHAR_FRONT_FIRST (EMERALD_FRONT_COUNT + MP_EXT_FRONT_COUNT)
#define FRONT_CHOICES (CHAR_FRONT_FIRST + MP_CHAR_FRONT_COUNT)
#define EMERALD_BACK_COUNT TRAINER_BACK_PIC_MP_LOCAL
#define CHAR_BACK_FIRST (EMERALD_BACK_COUNT + MP_EXT_BACK_COUNT)
#define BACK_CHOICES (CHAR_BACK_FIRST + MP_CHAR_BACK_COUNT)

// A character's frames: 9 walking + 9 running (16x32), 9 Mach Bike + 27 Acro Bike (bikeSize square), 9 diving (32x32)
#define CHAR_WALK 0
#define CHAR_RUN 9
#define CHAR_MACH 18
#define CHAR_ACRO 27
#define CHAR_DIVE 54

static const u8 *CharFrame(const struct MpCharDef *c, u8 n)
{
    u32 bike = c->bikeSize * c->bikeSize / 2, off = c->offset;
    if (n < CHAR_MACH)
        return sCharOwTiles + off + n * 256;
    off += CHAR_MACH * 256;
    if (n < CHAR_DIVE)
        return sCharOwTiles + off + (n - CHAR_MACH) * bike;
    return sCharOwTiles + off + (CHAR_DIVE - CHAR_MACH) * bike + (n - CHAR_DIVE) * 512;
}

static int CharIndex(u16 gfxId)
{
    return (gfxId >= MP_GFX_CHAR_FIRST && gfxId - MP_GFX_CHAR_FIRST < MP_CHAR_COUNT) ? gfxId - MP_GFX_CHAR_FIRST : -1;
}

// The character a look wears (MP_BASE_NPC with a character's sprite), or NULL
static const struct MpCharDef *CharOf(const struct MpAppearance *app)
{
    int i = app->base == MP_BASE_NPC ? CharIndex(app->gfxId) : -1;
    return i >= 0 ? &sMpChars[i] : NULL;
}

static u16 CharFrontChoice(u16 v)
{
    return v >= MP_CHAR_FRONT_NEW(0) ? CHAR_FRONT_FIRST + v - MP_CHAR_FRONT_NEW(0) : v;
}

static u16 CharBackChoice(u16 v)
{
    return v >= MP_CHAR_BACK_NEW(0) ? CHAR_BACK_FIRST + v - MP_CHAR_BACK_NEW(0) : v;
}

#define CATALOG_COUNT (ARRAY_COUNT(sSpriteCatalog) + MP_EXT_OW_COUNT + MP_CHAR_COUNT)
// Selections: 0 = MATCH, 1..CHOICES = a picture, then DEFAULT (Brendan/May by
// the save's gender) for ID PIC and NONE (hidden in battles) for BK PIC
#define FRONT_DEFAULT (FRONT_CHOICES + 1)
#define BACK_NONE (BACK_CHOICES + 1)

static struct MpSpriteEntry sExtCatalog[MP_EXT_OW_COUNT];
static struct ObjectEventGraphicsInfo sExtInfo[MP_EXT_OW_COUNT];
static struct SpriteFrameImage sExtFrames[MP_EXT_OW_COUNT][9];
static struct MpSpriteEntry sCharCatalog[MP_CHAR_COUNT];
static struct ObjectEventGraphicsInfo sCharInfo[MP_CHAR_COUNT];   // walking (the catalog's NPC info)
static struct SpriteFrameImage sCharFrames[MP_CHAR_COUNT][9];
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
    for (i = 0; i < MP_CHAR_COUNT; i++)
    {
        const struct MpCharDef *c = &sMpChars[i];
        sCharCatalog[i].gfxId = MP_GFX_CHAR_FIRST + i;
        sCharCatalog[i].name = c->name;
        sCharCatalog[i].frontPic = CharFrontChoice(c->front);
        sCharCatalog[i].backPic = CharBackChoice(c->back);
        sCharCatalog[i].gender = c->gender;
        sCharInfo[i] = gObjectEventGraphicsInfo_Boy1;
        for (f = 0; f < 9; f++)
        {
            sCharFrames[i][f].data = CharFrame(c, CHAR_WALK + f);
            sCharFrames[i][f].size = 256;
        }
        sCharInfo[i].images = sCharFrames[i];
        sCharInfo[i].size = 256;
        sCharInfo[i].inanimate = FALSE;
        sCharInfo[i].tracks = TRACKS_FOOT;
    }
    sExtReady = TRUE;
}

static const struct MpSpriteEntry *CatalogAt(u16 i)
{
    InitExtCatalog();
    if (i < ARRAY_COUNT(sSpriteCatalog))
        return &sSpriteCatalog[i];
    if (i < ARRAY_COUNT(sSpriteCatalog) + MP_EXT_OW_COUNT)
        return &sExtCatalog[i - ARRAY_COUNT(sSpriteCatalog)];
    return &sCharCatalog[i - ARRAY_COUNT(sSpriteCatalog) - MP_EXT_OW_COUNT];
}

const struct MpSpriteEntry *MP_Avatar_FindCatalog(u16 gfxId)
{
    u16 i;
    if (gfxId >= MP_GFX_CHAR_FIRST)
    {
        InitExtCatalog();
        return CharIndex(gfxId) >= 0 ? &sCharCatalog[CharIndex(gfxId)] : NULL;
    }
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
    if (CharIndex(gfxId) >= 0)
        return &sCharInfo[CharIndex(gfxId)];
    if (gfxId >= MP_GFX_EXT_FIRST)
        return &sExtInfo[gfxId - MP_GFX_EXT_FIRST];
    return gObjectEventGraphicsInfoPointers[gfxId];
}

const u16 *MP_Avatar_NpcPalette(u16 gfxId)
{
    const struct ObjectEventGraphicsInfo *info = MP_Avatar_NpcInfo(gfxId);
    if (!info)
        return NULL;
    if (CharIndex(gfxId) >= 0)
        return sCharOwPals + CharIndex(gfxId) * 32;   // the field palette
    if (gfxId >= MP_GFX_EXT_FIRST)
        return sExtOwPals + (gfxId - MP_GFX_EXT_FIRST) * 16;
    return MP_GetObjectEventPaletteData(info->paletteTag);
}

static const struct ObjectEventGraphicsInfo *NpcInfo(u16 gfxId)
{
    return MP_Avatar_NpcInfo(gfxId);
}

// A character's frame for the sprite editor (frame numbers: CHAR_WALK...), its size (16 = 16x32) and its
// palette as drawn (the diving one for diving frames). NULL when gfxId isn't a character.
const u8 *MP_Avatar_CharFrame(u16 gfxId, u8 frame, u8 *size, const u16 **pal)
{
    int i = CharIndex(gfxId);
    if (i < 0 || frame >= CHAR_DIVE + 9)
        return NULL;
    *size = frame < CHAR_MACH ? 16 : frame < CHAR_DIVE ? sMpChars[i].bikeSize : 32;
    *pal = sCharOwPals + i * 32 + (frame >= CHAR_DIVE ? 16 : 0);
    return CharFrame(&sMpChars[i], frame);
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
static bool8 sChoicesBuilt;   // the LOOK > SPRITE list (BuildChoices): rebuilt when the packs change

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
    sChoicesBuilt = FALSE;
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
// Sprite choices (CO-OP menu > LOOK > SPRITE), in three groups (LOOK > GROUP):
//   NAMED NPC  DEFAULT, BRENDAN, MAY, the characters with a name (in the sprite pack's order), then the
//              game's own named people (Norman, Anabel, Bill...) and the Ruby/Sapphire pair
//   GENERICS   classes and townsfolk: the characters that are a class (grunts...), then the game's own
//   CUSTOM     CUSTOM 1-3 (LOOK > CREATE), then the sprite packs in the sprites folder
// Looks the characters replaced (the old STEVEN, RED, PROF. OAK...) aren't offered; saves that wear
// them get the character (sMpCharAliases).

#define NUM_BASE_CHOICES MP_BASE_CUSTOM   // DEFAULT, BRENDAN, MAY, RS BRENDAN, RS MAY

enum { CH_BASE, CH_CREATED, CH_NPC, CH_PACK };

struct MpChoice
{
    u8 kind;
    u16 value;   // base, CUSTOM slot (1-3), graphics id or pack index
};

#define MAX_CHOICES (NUM_BASE_CHOICES + MP_CREATED_SLOTS + CATALOG_COUNT + MAX_PACK_FILES)
static struct MpChoice sChoices[MAX_CHOICES];
static u16 sGroupStart[MP_LOOK_GROUP_COUNT + 1];

// The game's own sprites of people with a name (the rest are classes and townsfolk)
static const char *const sNamedCatalog[] = {
    "norman", "juan", "anabel", "tucker", "spenser", "greta", "noland", "brandon", "scott", "mom", "teala",
    "frlg_celio", "frlg_bill", "frlg_daisy", "frlg_lorelei", "frlg_mr._fuji",
};

static void ToConfigName(char *dst, const char *src, int len);

// The ini name of a catalog sprite (frlg_ for FireRed/LeafGreen ones, kai_ for the characters)
static void CatalogConfigName(const struct MpSpriteEntry *e, char *out, int len)
{
    char buf[40];
    int c = CharIndex(e->gfxId);
    if (c >= 0)
    {
        snprintf(out, len, "%s", sMpChars[c].cfg);
        return;
    }
    ToConfigName(buf, e->name, sizeof(buf));
    snprintf(out, len, "%s%s", e->gfxId >= MP_GFX_EXT_FIRST ? "frlg_" : "", buf);
}

static bool8 IsReplacedCatalog(const char *cfg)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sMpCharAliases); i++)
        if (strcmp(sMpCharAliases[i][0], cfg) == 0)
            return TRUE;
    return FALSE;
}

static bool8 IsNamedCatalog(const char *cfg)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sNamedCatalog); i++)
        if (strcmp(sNamedCatalog[i], cfg) == 0)
            return TRUE;
    return FALSE;
}

static void AddChoice(u16 *n, u8 kind, u16 value)
{
    if (*n < MAX_CHOICES)
    {
        sChoices[*n].kind = kind;
        sChoices[*n].value = value;
        (*n)++;
    }
}

static void BuildChoices(void)
{
    u16 n = 0, i;
    char cfg[48];
    // NAMED NPC
    sGroupStart[MP_LOOK_NAMED] = n;
    AddChoice(&n, CH_BASE, MP_BASE_DEFAULT);
    AddChoice(&n, CH_BASE, MP_BASE_BRENDAN);
    AddChoice(&n, CH_BASE, MP_BASE_MAY);
    for (i = 0; i < MP_CHAR_COUNT; i++)
        if (!(sMpChars[i].flags & MP_CHARF_GENERIC))
            AddChoice(&n, CH_NPC, MP_GFX_CHAR_FIRST + i);
    for (i = 0; i < ARRAY_COUNT(sSpriteCatalog) + MP_EXT_OW_COUNT; i++)
    {
        CatalogConfigName(CatalogAt(i), cfg, sizeof(cfg));
        if (IsNamedCatalog(cfg) && !IsReplacedCatalog(cfg))
            AddChoice(&n, CH_NPC, CatalogAt(i)->gfxId);
    }
    AddChoice(&n, CH_BASE, MP_BASE_RS_BRENDAN);
    AddChoice(&n, CH_BASE, MP_BASE_RS_MAY);
    // GENERICS
    sGroupStart[MP_LOOK_GENERIC] = n;
    for (i = 0; i < MP_CHAR_COUNT; i++)
        if (sMpChars[i].flags & MP_CHARF_GENERIC)
            AddChoice(&n, CH_NPC, MP_GFX_CHAR_FIRST + i);
    for (i = 0; i < ARRAY_COUNT(sSpriteCatalog) + MP_EXT_OW_COUNT; i++)
    {
        CatalogConfigName(CatalogAt(i), cfg, sizeof(cfg));
        if (!IsNamedCatalog(cfg) && !IsReplacedCatalog(cfg))
            AddChoice(&n, CH_NPC, CatalogAt(i)->gfxId);
    }
    // CUSTOM
    sGroupStart[MP_LOOK_CUSTOM] = n;
    for (i = 1; i <= MP_CREATED_SLOTS; i++)
        AddChoice(&n, CH_CREATED, i);
    for (i = 0; i < sPackFileCount; i++)
        AddChoice(&n, CH_PACK, i);
    sGroupStart[MP_LOOK_GROUP_COUNT] = n;
    sChoicesBuilt = TRUE;
}

static void EnsureChoices(void)
{
    if (!sChoicesBuilt)
        BuildChoices();
}

static u16 PackChoice(u16 pack)
{
    u16 i;
    EnsureChoices();
    for (i = sGroupStart[MP_LOOK_CUSTOM]; i < sGroupStart[MP_LOOK_GROUP_COUNT]; i++)
        if (sChoices[i].kind == CH_PACK && sChoices[i].value == pack)
            return i;
    return 0;
}

static u16 FindChoice(u8 kind, u16 value)
{
    u16 i;
    EnsureChoices();
    for (i = 0; i < sGroupStart[MP_LOOK_GROUP_COUNT]; i++)
        if (sChoices[i].kind == kind && sChoices[i].value == value)
            return i;
    return 0;
}

// A CUSTOM slot that was never made looks like DEFAULT
static bool8 IsCreated(const struct MpAppearance *app)
{
    return app->base == MP_BASE_CREATED && app->recipe.made;
}

u16 MP_Avatar_ChoiceCount(void)
{
    EnsureChoices();
    return sGroupStart[MP_LOOK_GROUP_COUNT];
}

u8 MP_Avatar_ChoiceGroup(u16 choice)
{
    u8 g;
    EnsureChoices();
    for (g = MP_LOOK_GROUP_COUNT - 1; g > 0; g--)
        if (choice >= sGroupStart[g])
            break;
    return g;
}

// First choice of a group and how many it has
u16 MP_Avatar_GroupFirst(u8 group)
{
    EnsureChoices();
    return group < MP_LOOK_GROUP_COUNT ? sGroupStart[group] : 0;
}

u16 MP_Avatar_GroupSize(u8 group)
{
    EnsureChoices();
    return group < MP_LOOK_GROUP_COUNT ? sGroupStart[group + 1] - sGroupStart[group] : 0;
}

const char *MP_Avatar_GroupName(u8 group)
{
    static const char *const sNames[MP_LOOK_GROUP_COUNT] = {"NAMED NPC", "GENERICS", "CUSTOM"};
    return group < MP_LOOK_GROUP_COUNT ? sNames[group] : "?";
}

u16 MP_Avatar_GetChoice(const struct MpAppearance *app)
{
    u16 i;
    if (app->hasCustom && sLocalHasPack)
    {
        for (i = 0; i < sPackFileCount; i++)
            if (strcmp(sPackFiles[i], gMpConfig.customSprite) == 0)
                return PackChoice(i);
    }
    if (app->base == MP_BASE_CREATED)
        return (app->created >= 1 && app->created <= MP_CREATED_SLOTS) ? FindChoice(CH_CREATED, app->created) : 0;
    if (app->base == MP_BASE_NPC)
        return FindChoice(CH_NPC, app->gfxId);
    return app->base < NUM_BASE_CHOICES ? FindChoice(CH_BASE, app->base) : 0;
}

void MP_Avatar_SetChoice(struct MpAppearance *app, u16 choice)
{
    const struct MpChoice *c;
    EnsureChoices();
    if (choice >= sGroupStart[MP_LOOK_GROUP_COUNT])
        choice = 0;
    c = &sChoices[choice];
    switch (c->kind)
    {
    case CH_BASE:
        app->base = (u8)c->value;
        app->gfxId = 0;
        app->hasCustom = FALSE;
        break;
    case CH_CREATED:
        // CUSTOM 1-3: the trainer made in LOOK > CREATE (DEFAULT until it's made)
        app->base = MP_BASE_CREATED;
        app->created = c->value;
        app->recipe = gMpConfig.custom[app->created - 1];
        app->gfxId = 0;
        app->hasCustom = FALSE;
        break;
    case CH_NPC:
        app->base = MP_BASE_NPC;
        app->gfxId = c->value;
        app->hasCustom = FALSE;
        break;
    case CH_PACK:
        if (c->value < sPackFileCount)
        {
            const char *path = sPackFiles[c->value];
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
        break;
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
                MP_Avatar_SetChoice(&gMpConfig.app, PackChoice(i));
                return;
            }
    }
    tmp.hasCustom = FALSE;
    MP_Avatar_SetChoice(&gMpConfig.app, MP_Avatar_GetChoice(&tmp));
}

const char *MP_Avatar_ChoiceName(u16 choice)
{
    static const char *const sCustomNames[MP_CREATED_SLOTS] = {"CUSTOM 1", "CUSTOM 2", "CUSTOM 3"};
    const struct MpChoice *c;
    const struct MpSpriteEntry *e;
    EnsureChoices();
    if (choice >= sGroupStart[MP_LOOK_GROUP_COUNT])
        return "?";
    c = &sChoices[choice];
    switch (c->kind)
    {
    case CH_BASE:    return sBaseNames[c->value];
    case CH_CREATED: return sCustomNames[c->value - 1];
    case CH_NPC:     return (e = MP_Avatar_FindCatalog(c->value)) != NULL ? e->name : "?";
    default:         return c->value < sPackFileCount ? sPackNames[c->value] : "?";
    }
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
    if (app->base == MP_BASE_CREATED)
    {
        snprintf(out, len, "custom_%d", app->created ? app->created : 1);
        return;
    }
    if (app->base == MP_BASE_NPC && MP_Avatar_FindCatalog(app->gfxId))
        CatalogConfigName(MP_Avatar_FindCatalog(app->gfxId), out, len);
    else
        ToConfigName(out, MP_Avatar_BaseName(app->base >= MP_BASE_CUSTOM ? MP_BASE_DEFAULT : app->base), len);
}

int MP_Avatar_ParseSpriteName(const char *name, struct MpAppearance *app)
{
    char buf[40];
    u16 i;
    if (strncmp(name, "custom_", 7) == 0 && name[7] >= '1' && name[7] < '1' + MP_CREATED_SLOTS && !name[8])
    {
        // The recipe comes from the customN lines (MP_Avatar_Init)
        app->base = MP_BASE_CREATED;
        app->created = name[7] - '0';
        app->gfxId = 0;
        return TRUE;
    }
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
    // Looks a character replaced: the character
    for (i = 0; i < ARRAY_COUNT(sMpCharAliases); i++)
        if (strcmp(sMpCharAliases[i][0], name) == 0)
        {
            name = sMpCharAliases[i][1];
            break;
        }
    for (i = 0; i < CATALOG_COUNT; i++)
    {
        CatalogConfigName(CatalogAt(i), buf, sizeof(buf));
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

// Pokemon among the NPC sprites: no Brendan / May body for them
static bool8 IsPokemonNpc(u16 gfxId)
{
    return gfxId == OBJ_EVENT_GFX_WINGULL || gfxId == OBJ_EVENT_GFX_KIRLIA || gfxId == OBJ_EVENT_GFX_DUSCLOPS
        || gfxId == OBJ_EVENT_GFX_MEW;
}

// How a look shows an action (any state but walking). The look's base has been resolved
// (DEFAULT is BRENDAN or MAY by the save's gender).
enum {
    ACT_DEFAULT,   // Brendan's / May's own sprites (ActionGender), recolored
    ACT_PACK,      // the sprite pack's
    ACT_KIT,       // CREATE's own (the Mach Bike)
    ACT_HYBRID,    // Brendan's / May's body with the look's head (mp_hybrid.c)
    ACT_OWN,       // a Pokemon's own standing / walking frames
    ACT_STANDIN,   // the look's standing frame (StandInInfo)
    ACT_CHAR,      // a character's own bike / diving frames (mp_chars_gfx.h)
};

// Whose body an action is drawn on: Brendan's or May's
static u8 ActionGender(const struct MpAppearance *app)
{
    const struct MpSpriteEntry *e;
    switch (app->base)
    {
    case MP_BASE_BRENDAN:
    case MP_BASE_RS_BRENDAN:
        return MALE;
    case MP_BASE_MAY:
    case MP_BASE_RS_MAY:
        return FEMALE;
    case MP_BASE_CREATED:
        if (app->recipe.made)
            return app->recipe.girl ? FEMALE : MALE;
        break;
    case MP_BASE_NPC:
        if ((e = MP_Avatar_FindCatalog(app->gfxId)) != NULL)
            return e->gender == FEMALE ? FEMALE : MALE;
        break;
    }
    return app->gender == FEMALE ? FEMALE : MALE;
}

// A character who rides a Pokemon or a vehicle shows a bicycle instead (LOOK > BIKE: BIKE)
static bool8 CharBikeIsHybrid(const struct MpCharDef *c, const struct MpAppearance *app, u8 state)
{
    u8 flag = (state == ST_MACH_BIKE) ? MP_CHARF_MACH_RIDE : MP_CHARF_ACRO_RIDE;
    return (c->flags & flag) && app->bike;
}

bool8 MP_Avatar_HasRide(const struct MpAppearance *app)
{
    const struct MpCharDef *c = CharOf(app);
    return c && (c->flags & (MP_CHARF_MACH_RIDE | MP_CHARF_ACRO_RIDE));
}

static u8 ActionKind(const struct MpAppearance *app, const struct MpPack *pack, u8 state)
{
    const struct ObjectEventGraphicsInfo *npc;
    const struct MpCharDef *c;
    if (PackHasState(pack, state))
        return ACT_PACK;
    if (PackHasState(pack, ST_FOOT))
        return state >= ST_FIELD_MOVE ? ACT_STANDIN : ACT_DEFAULT;   // a drawn look: as before
    if ((c = CharOf(app)) != NULL)
    {
        // Their own bikes and diving; the rest (surfing, field moves...) built on Brendan's / May's body
        if (state == ST_UNDERWATER)
            return ACT_CHAR;
        if (state == ST_MACH_BIKE || state == ST_ACRO_BIKE)
            return CharBikeIsHybrid(c, app, state) ? ACT_HYBRID : ACT_CHAR;
        return ACT_HYBRID;
    }
    if (state == ST_UNDERWATER)
        return ACT_DEFAULT;   // a dark shape, the same for everyone
    switch (app->base)
    {
    case MP_BASE_CREATED:
        if (!app->recipe.made)
            break;
        return state == ST_MACH_BIKE ? ACT_KIT : ACT_HYBRID;
    case MP_BASE_RS_BRENDAN:
    case MP_BASE_RS_MAY:
        return ACT_HYBRID;
    case MP_BASE_NPC:
        if ((npc = NpcInfo(app->gfxId)) == NULL)
            break;
        if (npc->width == 16 && (npc->height == 32 || npc->height == 16) && !IsPokemonNpc(app->gfxId))
            return ACT_HYBRID;
        return state >= ST_FIELD_MOVE ? ACT_STANDIN : ACT_OWN;
    }
    return ACT_DEFAULT;
}

// Graphics info storage:
//   [0]                  local player on foot (OBJ_EVENT_GFX_MP_LOCAL)
//   [1 + id]             remote players
//   [LOCAL_STATE(st)]    local player's bike/surf/... from a sprite pack
#define LOCAL_STATE(st) (1 + MP_MAX_PLAYERS + (st) - 1)
#define NUM_MP_GFX (1 + MP_MAX_PLAYERS + ST_COUNT - 1)
#define MAX_FRAMES 27
#define LOCAL_TILE_BYTES (32 * 32 / 2)   // the local player's sprite: its biggest frames

static struct ObjectEventGraphicsInfo sMpGfxInfo[NUM_MP_GFX];
static struct SpriteFrameImage sMpFrames[NUM_MP_GFX][MAX_FRAMES];
static u8 sHyTiles[NUM_MP_GFX][MP_HY_MAX_FRAMES * 32 * 32 / 2];   // ACT_HYBRID frames
// Entries in the player's picture tables for each state (several can show the same frame)
static const u8 sStatePicCount[ST_COUNT] = {18, 9, 27, 12, 9, 5, 12, 9, 1};
static u32 sLocalStateBuilt[ST_COUNT];
static u16 sOrigPalBrendan[16], sOrigPalMay[16], sOrigPalRSBrendan[16], sOrigPalRSMay[16];
static u16 sLocalPalBrendan[16], sLocalPalMay[16], sLocalPalCustom[16], sLocalPalPack[16], sLocalPalPackDive[16];
static u16 sLocalPalBody[16];   // Brendan's / May's sprites for a look of the other gender's body

static void StatePalette(const struct MpAppearance *app, const struct MpPack *pack, u8 cacheSlot, u8 state, u16 *dst);
static struct MpAppearance RemoteAppearance(u8 id);

// The local player's sprite keeps one tile block whatever it's doing (on foot, on a bike, surfing...):
// big enough for every state's frames. 32x32, or 64x64 for PROF. BIRCH's bikes.
static u32 LocalTileBytes(const struct MpAppearance *app, const struct MpPack *pack)
{
    const struct MpCharDef *c = CharOf(app);
    if (c && c->bikeSize == 64 && !PackHasState(pack, ST_FOOT))
        return 64 * 64 / 2;
    return LOCAL_TILE_BYTES;
}

// A character's skin shades (its color map's S colors, lightest first; without a map the skin-like
// colors of its standing frames)
static void CharSkin(const struct MpCharDef *c, struct MpHybridLook *look)
{
    const u16 *pal = sCharOwPals + (c - sMpChars) * 32;
    const char *map = FindColorMap(CMAP_OW, c->cfg);
    u8 i, j, n = 0, idx[15];
    for (i = 1; i < 16; i++)
    {
        int h, sat, v;
        RgbToHsv(pal[i], &h, &sat, &v);
        if (map ? map[i - 1] == 'S' : (pal[i] && h <= 45 && sat >= 40 && sat <= 170 && v >= 150))
            idx[n++] = i;
    }
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
        {
            int h1, s1, v1, h2, s2, v2;
            RgbToHsv(pal[idx[i]], &h1, &s1, &v1);
            RgbToHsv(pal[idx[j]], &h2, &s2, &v2);
            if (v2 > v1)
            {
                u8 t = idx[i];
                idx[i] = idx[j];
                idx[j] = t;
            }
        }
    look->nSkin = n > 4 ? 4 : n;
    for (i = 0; i < look->nSkin; i++)
        look->skin[i] = idx[i];
}

// The look's head, standing frames and palette for mp_hybrid.c
static bool8 HybridLook(const struct MpAppearance *app, u8 cacheSlot, struct MpHybridLook *look, u16 *pal)
{
    const struct ObjectEventGraphicsInfo *info = NULL;
    u8 i;
    memset(look, 0, sizeof(*look));
    StatePalette(app, NULL, cacheSlot, ST_FOOT, pal);
    look->pal = pal;
    switch (app->base)
    {
    case MP_BASE_NPC:        info = NpcInfo(app->gfxId); break;
    case MP_BASE_RS_BRENDAN: info = &gObjectEventGraphicsInfo_RubySapphireBrendan; break;
    case MP_BASE_RS_MAY:     info = &gObjectEventGraphicsInfo_RubySapphireMay; break;
    case MP_BASE_CREATED:
    {
        const struct MpCreatedGfx *g;
        u16 skin[4];
        if (!app->recipe.made)
            return FALSE;
        g = MP_Created_Gfx(cacheSlot, &app->recipe);
        MP_Created_FieldHeadInfo(&app->recipe, look->neck, skin);
        for (i = 0; i < 3; i++)
            look->frame[i] = g->ow[i];
        look->w = look->h = 32;
        // The palette colors nearest to the skin's shades (lightest first)
        for (i = 0; i < 4; i++)
        {
            u8 j, best = 1, k;
            u32 bestD = 0xFFFFFFFF;
            for (j = 1; j < 16; j++)
            {
                int dr = (pal[j] & 31) - (skin[i] & 31), dg = ((pal[j] >> 5) & 31) - ((skin[i] >> 5) & 31);
                int db = ((pal[j] >> 10) & 31) - ((skin[i] >> 10) & 31);
                u32 d = dr * dr + dg * dg + db * db;
                if (d < bestD)
                {
                    bestD = d;
                    best = j;
                }
            }
            for (k = 0; k < look->nSkin && look->skin[k] != best; k++)
                ;
            if (k == look->nSkin)
                look->skin[look->nSkin++] = best;
        }
        look->kaiArms = FALSE;
        return TRUE;
    }
    default:
        return FALSE;
    }
    if (info == NULL || info->width != 16 || (info->height != 32 && info->height != 16))
        return FALSE;
    for (i = 0; i < 3; i++)
        look->frame[i] = info->images[i].data;
    look->w = 16;
    look->h = info->height;
    look->neck[0] = look->neck[1] = look->neck[2] = (info->height == 32) ? 23 : 9;
    for (i = 0; i < 4; i++)
        look->skin[i] = 1 + i;   // every Kai sprite's skin shades
    look->nSkin = 4;
    look->kaiArms = (info->height == 32);
    if (CharOf(app))
        CharSkin(CharOf(app), look);
    return TRUE;
}

// ACT_HYBRID frames for a state into an info. FALSE (info untouched) when they can't be made.
static bool8 BuildHybridInfo(int gfxSlot, const struct MpAppearance *app, u8 state, u8 cacheSlot)
{
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[gfxSlot];
    u8 gender = ActionGender(app);
    const struct ObjectEventGraphicsInfo *def = DefaultInfo(state, gender);
    struct MpHybridLook look;
    u16 pal[16];
    u32 frameBytes;
    u8 n, i;

    if (!HybridLook(app, cacheSlot, &look, pal))
        return FALSE;
    n = MP_Hybrid_Compose(&look, gender == FEMALE, state, def->images[0].data,
                          gender == FEMALE ? sOrigPalMay : sOrigPalBrendan,
                          sHyTiles[gfxSlot], MP_HY_MAX_FRAMES, &frameBytes);
    if (n == 0 || frameBytes != def->size || sStatePicCount[state] > MAX_FRAMES)
        return FALSE;
    for (i = 0; i < sStatePicCount[state]; i++)
    {
        // The picture table's entry -> the frame it shows
        u32 off = (u32)((const u8 *)def->images[i].data - (const u8 *)def->images[0].data);
        if (off % frameBytes != 0 || off / frameBytes >= n)
            return FALSE;
        sMpFrames[gfxSlot][i].data = sHyTiles[gfxSlot] + off;
        sMpFrames[gfxSlot][i].size = frameBytes;
    }
    *info = *def;
    info->images = sMpFrames[gfxSlot];
    return TRUE;
}

// ACT_OWN: a Pokemon look's own frames for a bike or surfing (walking frames for the bikes' riding)
static void BuildOwnInfo(int gfxSlot, const struct MpAppearance *app, u8 state)
{
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[gfxSlot];
    const struct ObjectEventGraphicsInfo *foot = NpcInfo(app->gfxId);
    u8 gender = ActionGender(app), i;
    const struct ObjectEventGraphicsInfo *def = DefaultInfo(state, gender);
    for (i = 0; i < sStatePicCount[state] && i < MAX_FRAMES; i++)
    {
        u8 f;
        u32 off = (u32)((const u8 *)def->images[i].data - (const u8 *)def->images[0].data);
        u8 src = off / def->size, dir = MP_Hybrid_FrameDir(gender == FEMALE, state, src);
        if ((state == ST_MACH_BIKE || state == ST_ACRO_BIKE) && src < 9)
            f = src;                                 // the same order as walking
        else
            f = (dir == 1) ? 1 : (dir >= 2) ? 2 : 0;   // standing (E is W turned by the animation)
        sMpFrames[gfxSlot][i] = foot->images[f];
    }
    *info = *foot;
    info->images = sMpFrames[gfxSlot];
    info->anims = def->anims;
    info->inanimate = FALSE;
}

// ACT_CHAR: a character's own Mach / Acro Bike or diving frames (one per picture-table entry, as Brendan's)
static void BuildCharInfo(int gfxSlot, const struct MpCharDef *c, u8 state, u8 gender)
{
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[gfxSlot];
    u8 first = (state == ST_MACH_BIKE) ? CHAR_MACH : (state == ST_ACRO_BIKE) ? CHAR_ACRO : CHAR_DIVE;
    u8 n = (state == ST_ACRO_BIKE) ? 27 : 9, i;
    u8 size = (state == ST_UNDERWATER) ? 32 : c->bikeSize;
    *info = *DefaultInfo(state, gender);
    for (i = 0; i < n && i < MAX_FRAMES; i++)
    {
        sMpFrames[gfxSlot][i].data = CharFrame(c, first + i);
        sMpFrames[gfxSlot][i].size = size * size / 2;
    }
    info->images = sMpFrames[gfxSlot];
    info->size = size * size / 2;
    info->width = size;
    info->height = size;
    if (size == 64)
    {
        info->oam = gObjectEventGraphicsInfo_RayquazaStill.oam;
        info->subspriteTables = gObjectEventGraphicsInfo_RayquazaStill.subspriteTables;
    }
    info->inanimate = FALSE;
}

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

    if (IsCreated(app) && (state == ST_FOOT || state == ST_MACH_BIKE))
    {
        // LOOK > CREATE: 32x32 frames built from the recipe
        u8 cacheSlot = (gfxSlot >= 1 && gfxSlot <= MP_MAX_PLAYERS) ? gfxSlot - 1 : MP_CR_CACHE_LOCAL;
        const struct MpCreatedGfx *g = MP_Created_Gfx(cacheSlot, &app->recipe);
        int first = (state == ST_FOOT) ? 0 : 18, n = (state == ST_FOOT) ? 18 : 9;
        *info = *def;
        for (i = 0; i < n; i++)
        {
            sMpFrames[gfxSlot][i].data = g->ow[first + i];
            sMpFrames[gfxSlot][i].size = sizeof(g->ow[0]);
        }
        info->images = sMpFrames[gfxSlot];
        info->size = sizeof(g->ow[0]);
        info->width = 32;
        info->height = 32;
        info->oam = gObjectEventGraphicsInfo_CyclingTriathleteM.oam;
        info->subspriteTables = gObjectEventGraphicsInfo_CyclingTriathleteM.subspriteTables;
        info->inanimate = FALSE;
        return;
    }

    if (state != ST_FOOT)
    {
        u8 cacheSlot = (gfxSlot >= 1 && gfxSlot <= MP_MAX_PLAYERS) ? gfxSlot - 1 : MP_CR_CACHE_LOCAL;
        switch (ActionKind(app, pack, state))
        {
        case ACT_HYBRID:
            if (BuildHybridInfo(gfxSlot, app, state, cacheSlot))
                return;
            MPNet_Log("avatar: no action frames for state %d (base %d, sprite %d)", state, app->base, app->gfxId);
            break;
        case ACT_OWN:
            BuildOwnInfo(gfxSlot, app, state);
            return;
        case ACT_CHAR:
            BuildCharInfo(gfxSlot, CharOf(app), state, ActionGender(app));
            return;
        }
        *info = *DefaultInfo(state, ActionGender(app));
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
        if (CharOf(app))
        {
            // The characters run in their own running frames (other NPC looks run in their walking ones)
            for (i = 0; i < 9; i++)
            {
                sMpFrames[gfxSlot][9 + i].data = CharFrame(CharOf(app), CHAR_RUN + i);
                sMpFrames[gfxSlot][9 + i].size = 256;
            }
        }
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
static void StatePalette(const struct MpAppearance *app, const struct MpPack *pack, u8 cacheSlot, u8 state, u16 *dst)
{
    bool8 female = (ActionGender(app) == FEMALE);
    if (state != ST_FOOT)
    {
        // Actions drawn with the look's own colors use its walking palette (a character's diving: its own)
        u8 kind = ActionKind(app, pack, state);
        if (kind == ACT_CHAR && state == ST_UNDERWATER)
        {
            const struct MpCharDef *c = CharOf(app);
            char cfg[48];
            memcpy(dst, sCharOwPals + (c - sMpChars) * 32 + 16, 32);
            snprintf(cfg, sizeof(cfg), "%s_dive", c->cfg);
            RecolorByMap(app, dst, FindColorMap(CMAP_OW, cfg));
            return;
        }
        if (kind == ACT_HYBRID || kind == ACT_OWN || kind == ACT_STANDIN || kind == ACT_CHAR)
        {
            StatePalette(app, pack, cacheSlot, ST_FOOT, dst);
            return;
        }
    }
    if (IsCreated(app) && (state == ST_FOOT || state == ST_MACH_BIKE))
    {
        memcpy(dst, MP_Created_Gfx(cacheSlot, &app->recipe)->owPal, 32);
        return;
    }
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
    struct MpAppearance app = RemoteAppearance(id);
    if (avatarMode >= ST_COUNT)
        avatarMode = ST_FOOT;
    if (PackHasState(pack, avatarMode))
        PackPalette(&app, pack, avatarMode, dst);
    else
        StatePalette(&app, pack, id, avatarMode, dst);
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
    memcpy(sLocalPalBody, sOrigPalBrendan, 32);
    MP_SetObjectEventPaletteData(OBJ_EVENT_PAL_TAG_MP_BODY, sLocalPalBody);

    if (gMpConfig.app.base == MP_BASE_CUSTOM)
        gMpConfig.app.base = MP_BASE_DEFAULT;   // old ini files
    if (gMpConfig.app.base == MP_BASE_CREATED)
    {
        if (gMpConfig.app.created < 1 || gMpConfig.app.created > MP_CREATED_SLOTS)
            gMpConfig.app.created = 1;
        gMpConfig.app.recipe = gMpConfig.custom[gMpConfig.app.created - 1];
    }
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
    if (app.base == MP_BASE_DEFAULT || app.base == MP_BASE_CUSTOM || (app.base == MP_BASE_CREATED && !app.recipe.made))
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    return app;
}

// Scenes that show the player as a separate object (a Battle Tent / Frontier room's walk-in,
// the Contest Hall, an interview's word screen...) take Brendan / May by gender: with a look of
// your own, that object wears it instead
u16 MP_PlayerStandInGfxId(u16 vanillaGfx)
{
    return MP_LocalUsesCustomAvatar() ? OBJ_EVENT_GFX_MP_LOCAL : vanillaGfx;
}

// Special, after a script set its player stand-in: every VAR_OBJ_GFX_ID_n holding the player's own
// Brendan / May graphics takes the player's look
void MP_SetPlayerStandInGfx(void)
{
    u16 own = gSaveBlock2Ptr->playerGender == MALE ? OBJ_EVENT_GFX_RIVAL_BRENDAN_NORMAL : OBJ_EVENT_GFX_RIVAL_MAY_NORMAL;
    u16 own2 = gSaveBlock2Ptr->playerGender == MALE ? OBJ_EVENT_GFX_BRENDAN_NORMAL : OBJ_EVENT_GFX_MAY_NORMAL;
    u16 v;
    if (!MP_LocalUsesCustomAvatar())
        return;
    for (v = VAR_OBJ_GFX_ID_0; v <= VAR_OBJ_GFX_ID_F; v++)
        if (VarGet(v) == own || VarGet(v) == own2)
            VarSet(v, OBJ_EVENT_GFX_MP_LOCAL);
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

// A look with no frames for an action (a field move, fishing, watering, decorating) keeps
// its own standing frame for it instead of turning into Brendan / May. Every animation of
// the action shows the standing frame of its direction (the tables' anims go S, N, W, E)
// and ends, so code that waits for the action's animation goes on. The frames sit at
// 0 / 4 / 6: AlignFishingAnimationFrames shifts the sprite for frames 1-3, 5, 10 and 11.
#define STANDIN_S 0
#define STANDIN_N 4
#define STANDIN_W 6
static const union AnimCmd sStandIn_S[] = {ANIMCMD_FRAME(STANDIN_S, 8), ANIMCMD_END};
static const union AnimCmd sStandIn_N[] = {ANIMCMD_FRAME(STANDIN_N, 8), ANIMCMD_END};
static const union AnimCmd sStandIn_W[] = {ANIMCMD_FRAME(STANDIN_W, 8), ANIMCMD_END};
static const union AnimCmd sStandIn_E[] = {ANIMCMD_FRAME(STANDIN_W, 8, .hFlip = TRUE), ANIMCMD_END};
static const union AnimCmd *const sStandInAnims[] = {
    sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E, sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E,
    sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E, sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E,
    sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E, sStandIn_S, sStandIn_N, sStandIn_W, sStandIn_E,
};
static struct ObjectEventGraphicsInfo sStandInInfo;
static struct SpriteFrameImage sStandInFrames[STANDIN_W + 1];

static const struct ObjectEventGraphicsInfo *StandInInfo(void)
{
    const struct ObjectEventGraphicsInfo *foot = MP_GetGraphicsInfo(OBJ_EVENT_GFX_MP_LOCAL);
    u8 i;
    sStandInInfo = *foot;
    for (i = 0; i < ARRAY_COUNT(sStandInFrames); i++)
        sStandInFrames[i] = foot->images[0];
    sStandInFrames[STANDIN_N] = foot->images[1];
    sStandInFrames[STANDIN_W] = foot->images[2];
    sStandInInfo.images = sStandInFrames;
    sStandInInfo.anims = sStandInAnims;
    sStandInInfo.inanimate = FALSE;
    return &sStandInInfo;
}

// The player's graphics id for an avatar state (the save's gender's), or FALSE
static bool8 PlayerStateOfGfxId(u8 graphicsId, u8 *state)
{
    u8 st, g = gSaveBlock2Ptr->playerGender == FEMALE ? FEMALE : MALE;
    for (st = ST_MACH_BIKE; st < ST_COUNT; st++)
        if (sStateGfxIds[st][g] == graphicsId)
        {
            *state = st;
            return TRUE;
        }
    return FALSE;
}

// The local player's bike / surf / field move / fishing / watering / decorating graphics: the
// sprite pack's, CREATE's Mach Bike, Brendan's / May's body with the look's head... NULL = the game's own.
const struct ObjectEventGraphicsInfo *MP_Avatar_LocalStateInfo(u8 graphicsId)
{
    u8 st, kind;
    const struct MpPack *pack;
    struct MpAppearance app;
    struct ObjectEventGraphicsInfo *info;
    if (!sAvatarInit || !PlayerStateOfGfxId(graphicsId, &st))
        return NULL;
    pack = LocalPack();
    app = GetLocalAppearance();
    kind = ActionKind(&app, pack, st);
    if (kind == ACT_STANDIN)
        return StandInInfo();
    if (kind == ACT_DEFAULT && ActionGender(&app) == (gSaveBlock2Ptr->playerGender == FEMALE ? FEMALE : MALE))
        return NULL;   // the save's own Brendan / May (their palette is recolored in RebuildLocal)
    info = &sMpGfxInfo[LOCAL_STATE(st)];
    if (sLocalStateBuilt[st] == sLocalVersion + 1)
        return info;
    if (kind == ACT_DEFAULT)
    {
        // The other one's sprites (a BRENDAN look on a girl's save...), with a palette of their own
        *info = *DefaultInfo(st, ActionGender(&app));
        if (info->paletteTag == OBJ_EVENT_PAL_TAG_BRENDAN || info->paletteTag == OBJ_EVENT_PAL_TAG_MAY)
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_BODY;
    }
    else
    {
        BuildStateInfo(LOCAL_STATE(st), &app, pack, st);
        info->paletteSlot = PALSLOT_PLAYER;
        if (kind == ACT_PACK)
            info->paletteTag = (st == ST_UNDERWATER) ? OBJ_EVENT_PAL_TAG_MP_PACK_DIVE : OBJ_EVENT_PAL_TAG_MP_PACK;
        else if (kind == ACT_KIT)
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_PACK;   // RebuildLocal: CREATE's palette
        else if (kind == ACT_CHAR && st == ST_UNDERWATER)
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_PACK_DIVE;   // RebuildLocal: the character's diving palette
        else if (info->images == sMpFrames[LOCAL_STATE(st)])
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_LOCAL;  // HYBRID / OWN: the look's walking palette
        else if (info->paletteTag == OBJ_EVENT_PAL_TAG_BRENDAN || info->paletteTag == OBJ_EVENT_PAL_TAG_MAY)
            info->paletteTag = OBJ_EVENT_PAL_TAG_MP_BODY;   // couldn't be built: Brendan / May
    }
    if (info->size < LocalTileBytes(&app, pack))
        info->size = LocalTileBytes(&app, pack);   // see RebuildLocal
    sLocalStateBuilt[st] = sLocalVersion + 1;
    return info;
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
    // ...and a look on the other body (a BRENDAN look on a girl's save) its own copy
    genderApp.base = (ActionGender(&app) == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    memcpy(sLocalPalBody, ActionGender(&app) == FEMALE ? sOrigPalMay : sOrigPalBrendan, 32);
    ApplyRecolor(&genderApp, sLocalPalBody);

    StatePalette(&app, pack, MP_CR_CACHE_LOCAL, ST_FOOT, sLocalPalCustom);
    if (IsCreated(&app))
        StatePalette(&app, pack, MP_CR_CACHE_LOCAL, ST_MACH_BIKE, sLocalPalPack);   // same palette
    if (pack)
    {
        PackPalette(&app, pack, ST_FOOT, sLocalPalPack);
        PackPalette(&app, pack, ST_UNDERWATER, sLocalPalPackDive);
    }
    else if (CharOf(&app))
        StatePalette(&app, NULL, MP_CR_CACHE_LOCAL, ST_UNDERWATER, sLocalPalPackDive);

    BuildStateInfo(0, &app, pack, ST_FOOT);
    // The player's sprite keeps the tile block it was made with when it gets on a bike, goes
    // surfing...: room for those 32x32 frames whatever the look (as Brendan's / May's own)
    if (sMpGfxInfo[0].size < LocalTileBytes(&app, pack))
        sMpGfxInfo[0].size = LocalTileBytes(&app, pack);
    sMpGfxInfo[0].paletteSlot = PALSLOT_PLAYER;
    sMpGfxInfo[0].paletteTag = PackHasState(pack, ST_FOOT) ? OBJ_EVENT_PAL_TAG_MP_PACK : OBJ_EVENT_PAL_TAG_MP_LOCAL;
    memset(sLocalStateBuilt, 0, sizeof(sLocalStateBuilt));
}

// Debug (mp actiondump): the local look's frames for every state, as the game shows them.
// File: per state "state w h n tag", then n frames and the 16 colors; tools render it.
void MP_Avatar_DumpActions(const char *path)
{
    FILE *f;
    u8 st, g = gSaveBlock2Ptr->playerGender == FEMALE ? FEMALE : MALE;
    f = fopen(path, "wb");
    if (!f)
        return;
    RebuildLocal();
    for (st = 0; st < ST_COUNT; st++)
    {
        const struct ObjectEventGraphicsInfo *info;
        const u16 *pal;
        u8 i, n = sStatePicCount[st];
        if (st == ST_FOOT)
            info = MP_LocalUsesCustomAvatar() ? MP_GetGraphicsInfo(OBJ_EVENT_GFX_MP_LOCAL) : DefaultInfo(ST_FOOT, g);
        else
        {
            info = MP_Avatar_LocalStateInfo(sStateGfxIds[st][g]);
            if (info == NULL)
                info = DefaultInfo(st, g);
        }
        if (info == &sStandInInfo)
            n = STANDIN_W + 1;
        else if (st == ST_FOOT)
            n = 9;
        pal = MP_GetObjectEventPaletteData(info->paletteTag);
        fprintf(f, "%d %d %d %d %d\n", st, info->width, info->height, n, info->paletteTag);
        for (i = 0; i < n; i++)
            fwrite(info->images[i].data, 1, info->width * info->height / 2, f);
        fwrite(pal ? pal : sOrigPalBrendan, 1, 32, f);
    }
    fclose(f);
    printf("[test] actiondump: wrote %s\n", path);
}

// Rebuilds the local avatar and refreshes the player sprite
void MP_Avatar_ApplyLocal(void)
{
    void MP_SpriteRefreshImages(struct Sprite *sprite, u32 oldBytes, u32 newBytes);
    bool8 onField = MP_IsInOverworld() && gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT
                 && gObjectEvents[gPlayerAvatar.objectEventId].active;
    u32 oldBytes = 0;

    // The sprite's tile block was sized for the old look's graphics
    if (onField && gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].images)
        oldBytes = GetObjectEventGraphicsInfo(gObjectEvents[gPlayerAvatar.objectEventId].graphicsId)->size;
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
            if (oldBytes)
                MP_SpriteRefreshImages(&gSprites[player->spriteId], oldBytes, GetObjectEventGraphicsInfo(gfx)->size);
            // and its reflection in water the new colors
            LoadPlayerObjectReflectionPalette(GetObjectEventGraphicsInfo(gfx)->paletteTag, PALSLOT_PLAYER);
            UpdateSpritePaletteWithWeather(gReflectionEffectPaletteMap[PALSLOT_PLAYER]);
        }
        UpdateSpritePaletteWithWeather(PALSLOT_PLAYER);
    }
}

// ---------------------------------------------------------------------------
// Remote players

// A remote player's look with its base resolved (DEFAULT -> BRENDAN / MAY...)
static struct MpAppearance RemoteAppearance(u8 id)
{
    struct MpAppearance app = gMpPlayers[id].app;
    if (app.base == MP_BASE_DEFAULT || app.base >= MP_BASE_COUNT || app.base == MP_BASE_CUSTOM
     || (app.base == MP_BASE_CREATED && !app.recipe.made))
        app.base = (app.gender == FEMALE) ? MP_BASE_MAY : MP_BASE_BRENDAN;
    return app;
}

// The surf blob draws with colors 5, 6 and 15 of the surfer's palette: Brendan's / May's have
// them, other looks' palettes don't, so their blob gets a palette of its own (Brendan's or May's)
u8 MP_Avatar_SurfBlobPalette(u8 id)
{
    struct MpAppearance app;
    struct MpPack buf;
    const struct MpPack *pack;
    bool8 female, loaded;
    u16 tag;
    u8 slot;
    if (!sAvatarInit)
        return 0xFF;
    if (id == 0xFF)
    {
        app = GetLocalAppearance();
        pack = LocalPack();
    }
    else if (id < MP_MAX_PLAYERS)
    {
        app = RemoteAppearance(id);
        pack = RemotePack(id, &buf);
    }
    else
        return 0xFF;
    if (ActionKind(&app, pack, ST_SURF) == ACT_DEFAULT)
        return 0xFF;
    female = ActionGender(&app) == FEMALE;
    tag = MP_TAG_BLOB_PAL + (female ? 1 : 0);
    loaded = IndexOfSpritePaletteTag(tag) != 0xFF;
    slot = MP_AllocCoopPalette(tag, female ? sOrigPalMay : sOrigPalBrendan);
    if (slot != 0xFF && !loaded)
        UpdateSpritePaletteWithWeather(slot);
    return slot;
}

void MP_Avatar_BuildRemote(u8 id)
{
    struct MpPlayer *p = &gMpPlayers[id];
    struct MpAppearance app = RemoteAppearance(id);
    struct MpPack buf;
    const struct MpPack *pack = RemotePack(id, &buf);
    u8 mode = p->hasState ? p->state.avatarMode : MP_AVATAR_FOOT;
    struct ObjectEventGraphicsInfo *info = &sMpGfxInfo[1 + id];

    if (mode >= ST_COUNT)
        mode = ST_FOOT;
    BuildStateInfo(1 + id, &app, pack, mode);
    // The overworld code assigns the real palette slot after spawning
    info->paletteSlot = PALSLOT_NPC_1;
    info->paletteTag = 0x1380 + 1 + id;
    info->disableReflectionPaletteLoad = TRUE;
}

// A 16x16 head for the Town Map (region_map.c): the top of the look's standing front frame,
// centered on what's drawn there, with a 1-pixel outline in `outline` (a palette index no one
// in the head uses). id -1 is the local player. tiles: 4 tiles (2x2, 128 bytes); pal: 16 colors.
bool8 MP_Avatar_HeadIcon(s8 id, u8 *tiles, u16 *pal, u16 outline)
{
    const struct ObjectEventGraphicsInfo *info;
    const u8 *src;
    u8 head[16][16], used[16] = {0}, edge = 0;
    int w, h, x, y, top = -1, left = 999, right = -1, cx, i;

    if (id < 0)
    {
        u8 gfx = gObjectEvents[gPlayerAvatar.objectEventId].graphicsId;
        const u16 *p;
        info = MP_Avatar_LocalStateInfo(gfx);
        if (info == NULL)
            info = GetObjectEventGraphicsInfo(gfx);
        p = info ? MP_GetObjectEventPaletteData(info->paletteTag) : NULL;
        if (p == NULL)
            return FALSE;
        memcpy(pal, p, 32);
    }
    else
    {
        if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
            return FALSE;
        info = &sMpGfxInfo[1 + id];
        if (info->images == NULL)
            MP_Avatar_BuildRemote(id);
        MP_Avatar_GetRemotePalette(id, gMpPlayers[id].hasState ? gMpPlayers[id].state.avatarMode : MP_AVATAR_FOOT, pal);
    }
    if (info == NULL || info->images == NULL || info->images[0].data == NULL)
        return FALSE;
    src = info->images[0].data;
    w = info->width;
    h = info->height;
#define PIX(px, py) ((src[(((py) / 8) * (w / 8) + (px) / 8) * 32 + ((py) % 8) * 4 + ((px) % 8) / 2] >> (((px) & 1) * 4)) & 0xF)
    for (y = 0; y < h && top < 0; y++)
        for (x = 0; x < w; x++)
            if (PIX(x, y))
            {
                top = y;
                break;
            }
    if (top < 0)
        return FALSE;
    for (y = top; y < top + 14 && y < h; y++)
        for (x = 0; x < w; x++)
            if (PIX(x, y))
            {
                if (x < left)
                    left = x;
                if (x > right)
                    right = x;
            }
    cx = (left + right + 1) / 2;
    memset(head, 0, sizeof(head));
    // (a row of room above for the outline)
    for (y = 0; y < 15; y++)
        for (x = 0; x < 16; x++)
        {
            int sx = cx - 8 + x, sy = top + y;
            if (sx >= 0 && sx < w && sy < h && x > 0 && x < 15)
                head[y + 1][x] = PIX(sx, sy);
            used[head[y + 1][x]] = 1;
        }
#undef PIX
    for (i = 15; i > 0 && !edge; i--)
        if (!used[i])
            edge = i;
    if (edge)
    {
        u8 copy[16][16];
        memcpy(copy, head, sizeof(copy));
        pal[edge] = outline;
        for (y = 0; y < 16; y++)
            for (x = 0; x < 16; x++)
                if (!copy[y][x] && ((x > 0 && copy[y][x - 1]) || (x < 15 && copy[y][x + 1]) || (y > 0 && copy[y - 1][x])
                                    || (y < 15 && copy[y + 1][x])))
                    head[y][x] = edge;
    }
    memset(tiles, 0, 128);
    for (y = 0; y < 16; y++)
        for (x = 0; x < 16; x++)
        {
            u8 *b = &tiles[((y / 8) * 2 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            *b |= head[y][x] << ((x & 1) * 4);
        }
    return TRUE;
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
    if (sel < CHAR_FRONT_FIRST)
        return sMpExtFronts[sel - EMERALD_FRONT_COUNT].name;
    if (sel < FRONT_CHOICES)
        return sMpCharFrontNames[sel - CHAR_FRONT_FIRST][0];
    return "?";
}

const char *MP_Avatar_FrontSelArtist(u16 sel)
{
    if (sel > EMERALD_FRONT_COUNT && sel <= CHAR_FRONT_FIRST)
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
    if (sel < CHAR_FRONT_FIRST)
        return sMpExtFronts[sel - EMERALD_FRONT_COUNT].cfg;
    return sMpCharFrontNames[sel - CHAR_FRONT_FIRST][1];
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
    if (sel < CHAR_BACK_FIRST)
        return sMpExtBackNames[sel - EMERALD_BACK_COUNT][0];
    if (sel < BACK_CHOICES)
        return sMpCharBackNames[sel - CHAR_BACK_FIRST][0];
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
    if (sel < CHAR_BACK_FIRST)
        return sMpExtBackNames[sel - EMERALD_BACK_COUNT][1];
    return sMpCharBackNames[sel - CHAR_BACK_FIRST][1];
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
    if (app.base == MP_BASE_DEFAULT || app.base == MP_BASE_CUSTOM || app.base >= MP_BASE_COUNT
     || (app.base == MP_BASE_CREATED && !app.recipe.made))
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
    if (c >= CHAR_FRONT_FIRST)
    {
        c -= CHAR_FRONT_FIRST;
        memcpy(tiles, sCharFrontTiles + c * TRAINER_PIC_SIZE, TRAINER_PIC_SIZE);
        memcpy(pal, sCharFrontPals + c * 16, 32);
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
    if (b >= CHAR_BACK_FIRST)
    {
        b -= CHAR_BACK_FIRST;
        memcpy(tiles, sCharBackTiles + b * 4 * TRAINER_PIC_SIZE, 4 * TRAINER_PIC_SIZE);
        memcpy(pal, sCharBackPals + b * 16, 32);
        return TRUE;
    }
    b -= EMERALD_BACK_COUNT;
    memcpy(tiles, sExtBackTiles + b * 4 * TRAINER_PIC_SIZE, 4 * TRAINER_PIC_SIZE);
    memcpy(pal, sExtBackPals + b * 16, 32);
    return TRUE;
}

// The look's front picture. FALSE = nothing picked (the game's default)
static bool8 BuildFront(const struct MpAppearance *appIn, u8 cacheSlot, const struct MpPack *pack, u8 *tiles, u16 *pal, bool8 recolor)
{
    struct MpAppearance app = Resolved(appIn);
    bool8 canRecolor = TRUE;
    int c = FrontChoice(&app);
    if (!app.frontSel && IsCreated(&app))
    {
        const struct MpCreatedGfx *g = MP_Created_Gfx(cacheSlot, &app.recipe);
        memcpy(tiles, g->front, TRAINER_PIC_SIZE);
        memcpy(pal, g->frontPal, 32);
        return TRUE;
    }
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

static u8 sPicBuf[4 * MP_BACK_PIC_SIZE];

// The look's 4 back frames: 96x64 (MP_BACK_PIC_WIDTH) when wide, else 64x64 (a CREATE look's wider
// picture cut to the 64x64 the game shows of a normal one)
static bool8 BuildBack(const struct MpAppearance *appIn, u8 cacheSlot, const struct MpPack *pack, u8 *tiles, u16 *pal, bool8 recolor, bool8 wide)
{
    static u8 sStdBuf[4 * TRAINER_PIC_SIZE];
    struct MpAppearance app = Resolved(appIn);
    int b = BackChoice(&app);
    if (!app.backSel && IsCreated(&app))
    {
        const struct MpCreatedGfx *g = MP_Created_Gfx(cacheSlot, &app.recipe);
        if (wide)
            memcpy(tiles, g->back, 4 * MP_BACK_PIC_SIZE);
        else
            BackFramesToStd(g->back[0], tiles);
        memcpy(pal, g->backPal, 32);
        return TRUE;
    }
    if (!app.backSel && pack && pack->slot[PACK_BACK])
    {
        memcpy(sStdBuf, pack->slot[PACK_BACK], 4 * TRAINER_PIC_SIZE);
        memcpy(pal, pack->pal[PACK_PAL_BACK], 32);
    }
    else
    {
        if (!BackChoiceTiles(b, sStdBuf, pal))
            return FALSE;
        if (recolor)
            RecolorByMap(&app, pal, FindColorMap(CMAP_BACK, MP_Avatar_BackSelConfigName(b + 1)));
    }
    if (wide)
        BackFramesToWide(sStdBuf, tiles);
    else
        memcpy(tiles, sStdBuf, 4 * TRAINER_PIC_SIZE);
    return TRUE;
}

// Front pic id for a look: an Emerald pic as is, or the runtime slot 0 (local) / 1 (others)
static u16 FrontPicId(const struct MpAppearance *appIn, u8 cacheSlot, const struct MpPack *pack, int which)
{
    struct MpAppearance app = Resolved(appIn);
    int c = FrontChoice(&app);
    bool8 usePack = !app.frontSel && ((pack && pack->slot[PACK_FRONT]) || IsCreated(&app));
    u16 pal[16];
    if (!usePack && c < EMERALD_FRONT_COUNT && !WantsRecolor(&app))
        return c < 0 ? 0xFFFF : (u16)c;
    if (!BuildFront(&app, cacheSlot, pack, sPicBuf, pal, TRUE))
        return 0xFFFF;
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPicLz[which], sPicBuf, TRAINER_PIC_SIZE);
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPalLz[which], pal, 32);
    return which == 0 ? TRAINER_PIC_MP_LOCAL : TRAINER_PIC_MP_OPPONENT;
}

static u16 BackPicId(const struct MpAppearance *appIn, u8 cacheSlot, const struct MpPack *pack, u8 *tiles, u32 *palLz, u16 slotPic)
{
    struct MpAppearance app = Resolved(appIn);
    int b = BackChoice(&app);
    bool8 usePack = !app.backSel && ((pack && pack->slot[PACK_BACK]) || IsCreated(&app));
    u16 pal[16];
    if (!usePack && (b < 0 || (b < EMERALD_BACK_COUNT && !WantsRecolor(&app))))
        return b < 0 ? 0xFFFF : (u16)b;
    if (!BuildBack(&app, cacheSlot, pack, tiles, pal, TRUE, TRUE))
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
    return BuildFront(app, MP_CR_CACHE_LOCAL, NULL, tiles, pal, FALSE);
}

bool8 MP_Avatar_GetBackTiles(const struct MpAppearance *app, u8 *tiles, u16 *pal)
{
    return BuildBack(app, MP_CR_CACHE_LOCAL, NULL, tiles, pal, FALSE, FALSE);
}

bool8 MP_Avatar_PreviewFront(u8 *tiles, u16 *pal)
{
    struct MpAppearance app = GetLocalAppearance();
    bool8 canRecolor;
    if (BuildFront(&app, MP_CR_CACHE_LOCAL, LocalPack(), tiles, pal, TRUE))
        return TRUE;
    // What the game shows by default
    FrontChoiceTiles(app.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN, tiles, pal, &canRecolor);
    return TRUE;
}

// A player's front picture as others see it (-1 = me), for the TEAM PHOTO
bool8 MP_Avatar_PlayerFront(s8 id, u8 *tiles, u16 *pal)
{
    struct MpPack buf;
    bool8 canRecolor;
    if (id < 0 || id == gMpLocalId || id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return MP_Avatar_PreviewFront(tiles, pal);
    if (BuildFront(&gMpPlayers[id].app, id, RemotePack(id, &buf), tiles, pal, TRUE))
        return TRUE;
    FrontChoiceTiles(gMpPlayers[id].app.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN, tiles, pal, &canRecolor);
    return TRUE;
}

// The first back frame as 64x64 (a CREATE look's: the wide picture's left 64 columns, where that frame is)
bool8 MP_Avatar_PreviewBack(u8 *tiles, u16 *pal)
{
    struct MpAppearance app = GetLocalAppearance();
    struct MpAppearance r = Resolved(&app);
    bool8 created = !r.backSel && IsCreated(&r);
    if (!BuildBack(&app, MP_CR_CACHE_LOCAL, LocalPack(), sPicBuf, pal, TRUE, created))
        return FALSE;
    memcpy(tiles, sPicBuf, TRAINER_PIC_SIZE);
    return TRUE;
}

u16 MP_Avatar_LocalFrontPic(void)
{
    struct MpAppearance app = GetLocalAppearance();
    return FrontPicId(&app, MP_CR_CACHE_LOCAL, LocalPack(), 0);
}

// This game's look as others see it (the BATTLE SIMULATOR's record, mp_sim.c)
struct MpAppearance MP_Avatar_LocalAppearance(void)
{
    return GetLocalAppearance();
}

// The BATTLE SIMULATOR (mp_sim.c): a stored look as the opponent's front picture, as a hologram: its shades kept, in
// the simulator's cyan light
u16 MP_Avatar_DigitalFrontPic(const struct MpAppearance *appIn)
{
    struct MpAppearance app = Resolved(appIn);
    u16 pal[16];
    bool8 canRecolor;
    int i;
    if (!BuildFront(&app, MP_CR_CACHE_LOCAL, NULL, sPicBuf, pal, TRUE))
    {
        int c = FrontChoice(&app);
        if (c < 0 || !FrontChoiceTiles(c, sPicBuf, pal, &canRecolor))
            FrontChoiceTiles(app.gender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN, sPicBuf, pal, &canRecolor);
    }
    for (i = 1; i < 16; i++)
    {
        int r = (pal[i] & 31), g = (pal[i] >> 5) & 31, b = (pal[i] >> 10) & 31;
        int l = (r * 3 + g * 6 + b) / 10;              // its brightness
        int nr = l * 5 / 8 + r / 8, ng = l * 7 / 8 + 4 + g / 8, nb = l * 7 / 8 + 7 + b / 8;
        pal[i] = (nr > 31 ? 31 : nr) | (ng > 31 ? 31 : ng) << 5 | (nb > 31 ? 31 : nb) << 10;
    }
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPicLz[1], sPicBuf, TRAINER_PIC_SIZE);
    MP_Pack_WriteLz77Literal((u8 *)gMpTrainerFrontPalLz[1], pal, 32);
    return TRAINER_PIC_MP_OPPONENT;
}

u16 MP_Avatar_RemoteFrontPic(u8 id)
{
    struct MpPack buf;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    return FrontPicId(&gMpPlayers[id].app, id, RemotePack(id, &buf), 1);
}

u16 MP_LocalFrontPicOr(u16 defaultPic)
{
    u16 pic = MP_Avatar_LocalFrontPic();
    return pic == 0xFFFF ? defaultPic : pic;
}

u16 MP_Avatar_LocalBackPic(void)
{
    struct MpAppearance app = GetLocalAppearance();
    return BackPicId(&app, MP_CR_CACHE_LOCAL, LocalPack(), gMpTrainerBackTiles, gMpTrainerBackPalLz, TRAINER_BACK_PIC_MP_LOCAL);
}

// Another player's battle back pic (co-op partner), 0xFFFF = game default
u16 MP_Avatar_RemoteBackPic(u8 id)
{
    return MP_Avatar_RemoteBackPicSlot(id, 0);
}

// slot 1: a Triple Battle's second ally (its own tiles and palette)
u16 MP_Avatar_RemoteBackPicSlot(u8 id, u8 slot)
{
    struct MpPack buf;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present)
        return 0xFFFF;
    if (slot == 1)
        return BackPicId(&gMpPlayers[id].app, id, RemotePack(id, &buf), gMpPartner2BackTiles, gMpPartner2BackPalLz, TRAINER_BACK_PIC_MP_PARTNER2);
    return BackPicId(&gMpPlayers[id].app, id, RemotePack(id, &buf), gMpPartnerBackTiles, gMpPartnerBackPalLz, TRAINER_BACK_PIC_MP_PARTNER);
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
        if (FrontChoice(&app) < 0 && !(pack && pack->slot[PACK_FRONT]) && !IsCreated(&app))
            appIn->frontSel = FRONT_DEFAULT;
    }
    if (back)
    {
        appIn->backSel = 0;
        app = Resolved(appIn);
        if (BackChoice(&app) < 0 && !(pack && pack->slot[PACK_BACK]) && !IsCreated(&app))
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
    choice = PackChoice(i);
    MP_Avatar_SetChoice(&gMpConfig.app, choice);
    if (!gMpConfig.app.hasCustom)
    {
        snprintf(err, errLen, "couldn't load %s", dst);
        return FALSE;
    }
    return TRUE;
}
