// Moemon Kai: drop shadows (OPTIONS > DISPLAY > SHADOWS; the user's mock-up: a soft dark oval under the feet). Every
// person on the field (the player, every NPC in every region, friends), every follower and Garden POKeMON gets a
// shadow sprite at its feet: an oval in a 32x16 frame whose pixels darken what the backgrounds show there
// (sprite->mpShadow: gba_easy_draw.c ShadowPixel), drawn under every other sprite (subpriority 255) at its owner's
// priority, so roofs and tree tops cover it as they cover its owner. Not drawn in water (surfing, diving, swimming
// Garden POKeMON cut off by mpClipOn), for things (inanimate objects) or hidden owners. On SINNOH's 3D maps the shadow
// stands where its owner does (sprite.c UpdateOamCoords places every field sprite by its feet).
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "metatile_behavior.h"
#include "sprite.h"
#include "mp.h"
#include "mp_bigsprite.h"
#include "mp_pmd.h"
#include "field_effect_helpers.h"
#include "constants/event_objects.h"
#include "s3d.h"
#include "o3d.h"
#include "u3d.h"

#define TAG_SHADOW 0x5AD0
#define SHADOW_MAGIC 0x5AD0
#define SIZES 4

static const u8 sOvalW[SIZES] = {16, 20, 26, 32};
static const u8 sOvalH[SIZES] = {6, 6, 8, 10};
static u8 sTiles[SIZES][8 * 32];   // each size: a 32x16 frame, 4bpp, 1D tiles (four wide, two high)
static bool8 sTilesMade;
static u8 sShadowOf[MAX_SPRITES];  // the owner sprite's shadow sprite (MAX_SPRITES: none)
static u8 sSeen[MAX_SPRITES];

static const struct OamData sOam = {
    .shape = SPRITE_SHAPE(32x16),
    .size = SPRITE_SIZE(32x16),
    .priority = 2,
};

static const union AnimCmd sAnim0[] = {ANIMCMD_FRAME(0, 1), ANIMCMD_END};
static const union AnimCmd sAnim1[] = {ANIMCMD_FRAME(8, 1), ANIMCMD_END};
static const union AnimCmd sAnim2[] = {ANIMCMD_FRAME(16, 1), ANIMCMD_END};
static const union AnimCmd sAnim3[] = {ANIMCMD_FRAME(24, 1), ANIMCMD_END};
static const union AnimCmd *const sAnims[SIZES] = {sAnim0, sAnim1, sAnim2, sAnim3};

static void SpriteCB_Shadow(struct Sprite *sprite)
{
}

static const struct SpriteTemplate sTemplate = {
    .tileTag = TAG_SHADOW,
    .paletteTag = TAG_NONE,
    .oam = &sOam,
    .anims = sAnims,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Shadow,
};

// The ovals: each centered across, its middle 3 rows below the frame's (the frame's middle is the sprite's y)
static void MakeTiles(void)
{
    s32 k, x, y;
    for (k = 0; k < SIZES; k++)
    {
        float a = sOvalW[k] / 2.0f, b = sOvalH[k] / 2.0f;
        for (y = 0; y < 16; y++)
            for (x = 0; x < 32; x++)
            {
                float dx = (x + 0.5f - 16.0f) / a, dy = (y + 0.5f - 11.0f) / b;
                if (dx * dx + dy * dy <= 1.0f)
                {
                    u8 *t = &sTiles[k][((y / 8) * 4 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                    *t |= (x & 1) ? 0x10 : 0x01;
                }
            }
    }
    sTilesMade = TRUE;
}

static bool8 IsOurs(u8 id)
{
    return id < MAX_SPRITES && gSprites[id].inUse && gSprites[id].callback == SpriteCB_Shadow
        && gSprites[id].data[7] == SHADOW_MAGIC;
}

// ... and this owner's: a shadow knows its owner (data[6]), so an owner whose shadow went with the sprites (a sprite
// reset) never takes over the shadow that sprite id became for someone else (two owners moved one shadow each frame,
// the last one's place winning: the player's shadow under an NPC off screen)
static bool8 IsOwnShadow(u8 id, u8 owner)
{
    return IsOurs(id) && gSprites[id].data[6] == owner;
}

static void Drop(u8 owner)
{
    if (IsOwnShadow(sShadowOf[owner], owner))
        DestroySprite(&gSprites[sShadowOf[owner]]);
    sShadowOf[owner] = MAX_SPRITES;
}

// The rows one frame of a sprite draws (its picture's first and last rows with a pixel, 0 = the frame's top);
// FALSE: no picture to read (a sheet sprite, a big one). 4bpp frames from its images, 1D tiles; cached by picture.
static const u8 sShapeW[3][4] = {{8, 16, 32, 64}, {16, 32, 32, 64}, {8, 8, 16, 32}};
static const u8 sShapeH[3][4] = {{8, 16, 32, 64}, {8, 8, 16, 32}, {16, 32, 32, 64}};
static bool8 FrameRows(const struct Sprite *sp, u16 imageValue, u8 *top, u8 *bottom)
{
    static struct { const void *data; u8 top, bottom; } sCache[64];
    const struct SpriteFrameImage *img = &sp->images[imageValue];
    const u8 *d;
    u32 w, h, x, y, k;
    if (img->data == NULL)
        return FALSE;
    k = ((uintptr_t)img->data >> 4) & 63;
    if (sCache[k].data != img->data)
    {
        w = sShapeW[sp->oam.shape][sp->oam.size];
        h = sShapeH[sp->oam.shape][sp->oam.size];
        if (img->size < w * h / 2)
            return FALSE;
        d = img->data;
        sCache[k].top = 0xFF, sCache[k].bottom = 0;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x += 2)
            {
                u8 b = d[((y / 8) * (w / 8) + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                if (b)
                {
                    if (sCache[k].top == 0xFF)
                        sCache[k].top = y;
                    sCache[k].bottom = y;
                    break;
                }
            }
        if (sCache[k].top == 0xFF)
            sCache[k].top = 0;
        sCache[k].data = img->data;
    }
    *top = sCache[k].top;
    *bottom = sCache[k].bottom;
    return TRUE;
}

static bool8 Readable(const struct Sprite *sp)
{
    return sp->images != NULL && sp->mpBig == NULL && sp->anims != NULL && sp->oam.shape <= 2 && !sp->oam.bpp;
}

// The rows a sprite's current frame draws
bool8 MP_SpriteDrawnRows(const struct Sprite *sp, u8 *top, u8 *bottom)
{
    const union AnimCmd *cmd;
    if (!Readable(sp))
        return FALSE;
    cmd = &sp->anims[sp->animNum][sp->animCmdIndex];
    if (cmd->type < 0)
        return FALSE;
    return FrameRows(sp, cmd->frame.imageValue, top, bottom);
}

// The middle column of a frame's pixels (x2: twice it, so halves count; cached by picture) for MP_SpriteStandShift
static bool8 FrameMiddle2(const struct Sprite *sp, u16 imageValue, s16 *mid2)
{
    static struct { const void *data; s16 mid2; } sCache[64];
    const struct SpriteFrameImage *img = &sp->images[imageValue];
    const u8 *d;
    u32 w, h, x, y, k, n = 0, sum = 0;
    if (img->data == NULL)
        return FALSE;
    k = ((uintptr_t)img->data >> 4) & 63;
    if (sCache[k].data != img->data)
    {
        w = sShapeW[sp->oam.shape][sp->oam.size];
        h = sShapeH[sp->oam.shape][sp->oam.size];
        if (img->size < w * h / 2)
            return FALSE;
        d = img->data;
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++)
            {
                u8 b = d[((y / 8) * (w / 8) + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
                if ((x & 1) ? b >> 4 : b & 0xF)
                    n++, sum += x;
            }
        sCache[k].mid2 = n ? (s16)((sum * 2 + n / 2) / n) : (s16)(w - 1);
        sCache[k].data = img->data;
    }
    *mid2 = sCache[k].mid2;
    return TRUE;
}

// Moemon Kai (1.63.13): a GBA-size POKeMON sprite (a follower's or a Garden POKeMON's original sprite) placed as a PMD
// picture is (mp_pmd.c UpdateLift): Y, added to a sprite standing at its tile (the old 32x32 frames' center), puts
// its standing picture's lowest row (its first frame's) on the player's line, PMD_GROUND_DY + PMD_FEET_DY below that;
// X, added across, puts the middle of the pixels of the frame it stands in for the facing (ANIM) on its point, as a
// PMD picture's MpBigSprite.shift does. FALSE: a sprite whose pictures can't be read (they stay as they were).
bool8 MP_SpriteStandShift(const struct Sprite *sp, u8 anim, s16 *dx, s16 *dy)
{
    const union AnimCmd *cmd;
    u8 top, bottom;
    s16 mid2;
    u32 w;
    if (!Readable(sp) || !FrameRows(sp, 0, &top, &bottom))
        return FALSE;
    *dy = PMD_GROUND_DY + PMD_FEET_DY - (sp->centerToCornerVecY + bottom);
    *dx = 0;
    cmd = &sp->anims[anim][0];
    if (cmd->type >= 0 && FrameMiddle2(sp, cmd->frame.imageValue, &mid2))
    {
        w = sShapeW[sp->oam.shape][sp->oam.size];
        if (cmd->frame.hFlip)
            mid2 = (s16)(2 * (w - 1)) - mid2;
        *dx = -(mid2 - (s16)(w - 1) + (mid2 > (s16)(w - 1) ? 1 : 0)) / 2;   // (to the nearest pixel, either way)
    }
    return TRUE;
}

// Below a sprite's y, the row under its standing picture's feet (its first frame's lowest row + 1): what a scaled
// POKeMON sprite shrinks toward (mp_bigsprite.c), as a PMD picture shrinks toward its feet
bool8 MP_SpriteFeetRow(const struct Sprite *sp, s16 *row)
{
    u8 top, bottom;
    if (!Readable(sp) || !FrameRows(sp, 0, &top, &bottom))
        return FALSE;
    *row = sp->centerToCornerVecY + bottom + 1;
    return TRUE;
}

// Where a sprite's feet are, the same whatever frame it shows: its first picture's lowest row (an object's first
// picture is it standing, facing down). 1.63.8 took the lowest row of the animation's frames, and the walking frames,
// a row or two lower, moved the shadow when a follower set off; each frame's own row hopped it while running.
static const void *sFeetImages[MAX_SPRITES];   // (big sprites: their picture set, and half their widest frame)
static u8 sFeetBottom[MAX_SPRITES];
static bool8 SteadyBottom(const struct Sprite *sp, u8 *bottom)
{
    u8 top;
    if (!Readable(sp))
        return FALSE;
    return FrameRows(sp, 0, &top, bottom);
}

// The owner's feet (sprite coordinates, without its hop) and how wide it is
static void Feet(const struct Sprite *o, s32 *x, s32 *y, s32 *w)
{
    *x = o->x + o->x2;
    if (o->mpBig != NULL && o->mpBig->frame != NULL)
    {
        // a big (PMD) picture's point is its feet on the ground (mp_pmd.c): the shadow stays there whatever the frame's
        // picture does (1.63.9: its bottom bobbed with the walk, and so did the shadow); as wide as the widest frame
        const u8 *f = o->mpBig->frame;
        u16 fw = f[4] | f[5] << 8;
        u32 id = o - gSprites;
        *y = o->y + PMD_FEET_DY + 1;   // (its standing feet: mp_pmd.c UpdateLift puts them on the player's line)
        if (id < MAX_SPRITES)
        {
            if (sFeetImages[id] != (const void *)o->mpBig || fw > sFeetBottom[id] * 2)
                sFeetBottom[id] = fw / 2;   // (half the width: it fits a u8)
            sFeetImages[id] = o->mpBig;
            fw = sFeetBottom[id] * 2;
        }
        *w = fw;
    }
    else
    {
        u8 bottom;
        *y = o->y - o->centerToCornerVecY;
        *w = -2 * o->centerToCornerVecX;
        if (SteadyBottom(o, &bottom))   // (where its picture's feet are: a big POKeMON's sit higher)
            *y = o->y + o->centerToCornerVecY + bottom + 1;
    }
    if (o->mpScale != 0)   // (drawn smaller: mp_spritescale.h)
        *w = *w * o->mpScale / 256;
}

// The row under a sprite's feet and the x they're at, as its shadow finds them (followers' ground effects:
// mp_followfx.c)
void MP_SpriteFeet(const struct Sprite *o, s16 *x, s16 *y)
{
    s32 fx, fy, w;
    Feet(o, &fx, &fy, &w);
    *x = fx;
    *y = fy;
}

static void Cast(u8 owner, bool8 person)
{
    struct Sprite *o = &gSprites[owner], *s;
    s32 x, y, w;
    u8 size, id = sShadowOf[owner];
    if (!IsOwnShadow(id, owner))
    {
        id = CreateSprite(&sTemplate, 0, 0, 255);
        if (id == MAX_SPRITES)
        {
            sShadowOf[owner] = MAX_SPRITES;
            return;
        }
        gSprites[id].data[7] = SHADOW_MAGIC;
        gSprites[id].data[6] = owner;
        gSprites[id].mpShadow = TRUE;
        sShadowOf[owner] = id;
    }
    s = &gSprites[id];
    Feet(o, &x, &y, &w);
    // people (the player in any LOOK, every NPC, GEN 4 or not): all the same, the user's mock-up oval; POKeMON too
    // (1.63.13, the user: every POKeMON's shadow the player's size, whatever its size or style)
    size = 0;
    (void)person;
    StartSpriteAnimIfDifferent(s, size);
    s->x = x;
    s->y = y - 1 - 3;   // (the oval's middle on the feet's last row; it sits 3 below the frame's middle)
    s->x2 = s->y2 = 0;
    s->coordOffsetEnabled = o->coordOffsetEnabled;
    s->oam.priority = o->oam.priority;
    s->subpriority = 255;
    s->invisible = FALSE;
    sSeen[owner] = TRUE;
}

void MP_Shadow_Reset(void);
bool8 MP_IsReflectionSprite(struct Sprite *sprite);   // field_effect_helpers.c

static bool8 InGrass(s16 x, s16 y)
{
    u32 b = MapGridGetMetatileBehaviorAt(x, y);
    return MetatileBehavior_IsTallGrass(b) || MetatileBehavior_IsLongGrass(b);
}

// The tile's top layer over the feet (a potted plant, a tree top, a roof: NORMAL / SPLIT metatiles draw it on BG1,
// over sprites at priority 2): the owner is hidden there, and its oval, a few pixels lower, peeked out under the tile
// (the user's picture: a line under the second plant's pot). True when the top layer's bottom two pixel rows are
// mostly drawn
static bool8 FeetCovered(s16 x, s16 y, const struct Sprite *o)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    const u16 *tiles;
    u32 id, k, row, opaque = 0;
    if (o->oam.priority <= 1 || layout == NULL || S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup)
     || O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
     || U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup))   // (3D maps: their stand-in tiles cover nothing)
        return FALSE;
    if (MP_MapPic_CoversFeet(x, y))   // (the GAME CENTER's counters: its pictures' over layer, not tiles)
        return TRUE;
    if (MapGridGetMetatileLayerTypeAt(x, y) == METATILE_LAYER_TYPE_COVERED)
        return FALSE;
    id = MapGridGetMetatileIdAt(x, y);
    if (id >= NUM_METATILES_TOTAL)
        return FALSE;
    if (id < Fieldmap_PrimaryMetatiles(layout))
    {
        if (layout->primaryTileset == NULL)
            return FALSE;
        tiles = layout->primaryTileset->metatiles;
    }
    else
    {
        if (layout->secondaryTileset == NULL)
            return FALSE;
        tiles = layout->secondaryTileset->metatiles;
        id -= Fieldmap_PrimaryMetatiles(layout);
    }
    tiles += id * NUM_TILES_PER_METATILE;
    for (k = 6; k < 8; k++)   // (the top layer's bottom row: entries 6 and 7)
    {
        u16 e = tiles[k];
        const u8 *px = (const u8 *)(BG_VRAM + TILE_OFFSET_4BPP(e & 0x3FF));
        for (row = 6; row < 8; row++)
        {
            u32 r = (e & 0x800) ? 7 - row : row, b;
            for (b = 0; b < 4; b++)
                opaque += ((px[r * 4 + b] & 0xF) != 0) + ((px[r * 4 + b] >> 4) != 0);
        }
    }
    return opaque >= 8;   // (of the 32 pixels under the feet: leaves have gaps)
}

// A follower or Garden POKeMON has no object event: its tile from where its feet stand against the player's (both
// sprites in the same screen space; a tile is 16 pixels)
static bool8 SpriteFeetCovered(const struct Sprite *o)
{
    struct ObjectEvent *pe = &gObjectEvents[gPlayerAvatar.objectEventId];
    const struct Sprite *p;
    s32 fx, fy, px, py, w;
    if (pe->spriteId >= MAX_SPRITES || o->coordOffsetEnabled != gSprites[pe->spriteId].coordOffsetEnabled)
        return FALSE;
    p = &gSprites[pe->spriteId];
    Feet(o, &fx, &fy, &w);
    Feet(p, &px, &py, &w);
    return FeetCovered(pe->currentCoords.x + (fx - px + 8 + 1600) / 16 - 100,
                       pe->currentCoords.y + (fy - py + 8 + 1600) / 16 - 100, o);
}

static bool8 InWater(s16 x, s16 y)
{
    u8 b = MapGridGetMetatileBehaviorAt(x, y);
    return MetatileBehavior_IsSurfableWaterOrUnderwater(b) || MetatileBehavior_IsDeepOrOceanWater(b);
}

// `mp shadowwatch N` (test): the player's and the follower's shadows against where they stand (their sprite's y, not
// a hop's y2) for N frames: the lowest and highest gap, and the frames they had none
static u16 sWatch;
static struct { s16 min, max; u16 missing; } sW[2];
static u8 sWatchJump;   // the most of the game's own jump shadows (FLDEFF_SHADOW) seen at once
void MP_Shadow_Watch(u16 frames)
{
    u32 k;
    sWatch = frames;
    sWatchJump = 0;
    for (k = 0; k < 2; k++)
        sW[k].min = 0x7FFF, sW[k].max = -0x7FFF, sW[k].missing = 0;
}

static void WatchOne(u32 k, u8 owner)
{
    u8 id;
    if (owner >= MAX_SPRITES)
        return;
    id = sShadowOf[owner];
    if (IsOwnShadow(id, owner) && !gSprites[id].invisible)
    {
        s16 gap = gSprites[id].y - gSprites[owner].y;
        if (getenv("MOEMON_SHADOW_TRACE"))
            printf("[shadow] %s owner %u y %d shadow %u y %d anim %u\n", k ? "follower" : "player", owner, gSprites[owner].y,
                   id, gSprites[id].y, gSprites[owner].animNum);
        if (gap < sW[k].min)
            sW[k].min = gap;
        if (gap > sW[k].max)
            sW[k].max = gap;
    }
    else
        sW[k].missing++;
}

static void Watch(void)
{
    u32 i, k;
    u8 follower = MAX_SPRITES;
    if (sWatch == 0)
        return;
    for (i = 0; i < MAX_SPRITES && follower == MAX_SPRITES; i++)
        if (gSprites[i].inUse && gSprites[i].mpCastShadow && !gSprites[i].invisible)
            follower = i;
    WatchOne(0, gObjectEvents[gPlayerAvatar.objectEventId].spriteId);
    WatchOne(1, follower);
    for (i = 0, k = 0; i < MAX_SPRITES; i++)
        if (gSprites[i].inUse && gSprites[i].callback == UpdateShadowFieldEffect)
            k++;
    if (k > sWatchJump)
        sWatchJump = k;
    if (--sWatch == 0)
        printf("[test] shadowwatch jump shadows %u\n", (unsigned)sWatchJump);
    if (sWatch == 0)
        for (k = 0; k < 2; k++)
            if (sW[k].max >= sW[k].min || sW[k].missing)
                printf("[test] shadowwatch %s gap %d..%d (moved %d) missing %u\n", k ? "follower" : "player", sW[k].min,
                       sW[k].max, sW[k].max >= sW[k].min ? sW[k].max - sW[k].min : 0, (unsigned)sW[k].missing);
}

// Every overworld frame, last thing before the OAM is built (overworld.c OverworldBasic)
void MP_Shadow_Update(void)
{
    u32 i;
    if (!sTilesMade)
    {
        MakeTiles();
        MP_Shadow_Reset();
    }
    if (GetSpriteTileStartByTag(TAG_SHADOW) == 0xFFFF)
    {
        struct SpriteSheet sheet = {sTiles[0], sizeof(sTiles), TAG_SHADOW};
        LoadSpriteSheet(&sheet);
    }
    memset(sSeen, 0, sizeof(sSeen));
    if (!gMpConfig.noShadows)
    {
        for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        {
            struct ObjectEvent *e = &gObjectEvents[i];
            struct Sprite *o;
            if (!e->active || e->invisible || e->inanimate || e->spriteId >= MAX_SPRITES)
                continue;
            o = &gSprites[e->spriteId];
            if (!o->inUse || o->invisible || o->mpShadow || o->mpClipOn)
                continue;
            if (e->isPlayer && TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER))
                continue;
            if (InWater(e->currentCoords.x, e->currentCoords.y))
                continue;
            if (InGrass(e->currentCoords.x, e->currentCoords.y))
                continue;   // (the grass covers the feet: the oval showed a pixel below its tile)
            if (FeetCovered(e->currentCoords.x, e->currentCoords.y, o))
                continue;   // (a top layer over the feet: the oval showed under the tile)
            Cast(e->spriteId, TRUE);
        }
        for (i = 0; i < MAX_SPRITES; i++)   // followers and Garden POKeMON (mp_follower.c marks them)
        {
            struct Sprite *o = &gSprites[i];
            if (o->inUse && o->mpCastShadow && !o->invisible && !o->mpClipOn && !sSeen[i] && !MP_IsReflectionSprite(o)
             && !SpriteFeetCovered(o))
                Cast(i, FALSE);
        }
    }
    for (i = 0; i < MAX_SPRITES; i++)
        if (!sSeen[i] && sShadowOf[i] != MAX_SPRITES)
            Drop(i);    Watch();
}

// A new field (the sprites were all reset): no shadows remembered
void MP_Shadow_Reset(void)
{
    memset(sShadowOf, MAX_SPRITES, sizeof(sShadowOf));
    memset(sFeetImages, 0, sizeof(sFeetImages));
}

// Tests (mp shadows): each shadow, its owner and where both are on screen
void MP_Shadow_Debug(void)
{
    u32 i, j;
    for (i = 0; i < MAX_SPRITES; i++)
        for (j = i + 1; j < MAX_SPRITES; j++)
            if (sShadowOf[i] != MAX_SPRITES && sShadowOf[i] == sShadowOf[j] && IsOurs(sShadowOf[i]))
                printf("[test] shadow shared: sprites %u and %u both hold shadow %u\n", (unsigned)i, (unsigned)j, sShadowOf[i]);
    {
        struct ObjectEvent *e = &gObjectEvents[gPlayerAvatar.objectEventId];
        struct Sprite *o = &gSprites[e->spriteId];
        u32 used = 0;
        for (j = 0; j < MAX_SPRITES; j++)
            used += gSprites[j].inUse;
        printf("[test] shadow player: %u sprites used, its shadow %u (ours %d); sprite %u inUse %d inv %d mpShadow %d clip %d evInv %d inanimate %d water %d under %u cast %d\n",
               (unsigned)used, sShadowOf[e->spriteId], IsOurs(sShadowOf[e->spriteId]), e->spriteId, o->inUse, o->invisible, o->mpShadow, o->mpClipOn, e->invisible, e->inanimate,
               InWater(e->currentCoords.x, e->currentCoords.y), o->mpUnder, o->mpCastShadow);
    }
    for (i = 0; i < MAX_SPRITES; i++)
        if (sShadowOf[i] != MAX_SPRITES && IsOwnShadow(sShadowOf[i], i))
        {
            const struct Sprite *o = &gSprites[i], *s = &gSprites[sShadowOf[i]];
            printf("[test] shadow of sprite %u (%s): owner at %d,%d pri %d sub %d, shadow at %d,%d pri %d inv %d\n",
                   (unsigned)i, MP_IsReflectionSprite((struct Sprite *)o) ? "reflection" : o->mpCastShadow ? "follower" : "object",
                   o->x + o->x2 + (o->coordOffsetEnabled ? gSpriteCoordOffsetX : 0),
                   o->y + o->y2 + (o->coordOffsetEnabled ? gSpriteCoordOffsetY : 0), o->oam.priority, o->subpriority,
                   s->x + s->x2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetX : 0),
                   s->y + s->y2 + (s->coordOffsetEnabled ? gSpriteCoordOffsetY : 0), s->oam.priority, s->invisible);
        }
}
