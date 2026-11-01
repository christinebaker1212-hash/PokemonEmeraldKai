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

static void Drop(u8 owner)
{
    if (IsOurs(sShadowOf[owner]))
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

static void Cast(u8 owner, bool8 person)
{
    struct Sprite *o = &gSprites[owner], *s;
    s32 x, y, w;
    u8 size, id = sShadowOf[owner];
    if (!IsOurs(id))
    {
        id = CreateSprite(&sTemplate, 0, 0, 255);
        if (id == MAX_SPRITES)
            return;
        gSprites[id].data[7] = SHADOW_MAGIC;
        gSprites[id].mpShadow = TRUE;
        sShadowOf[owner] = id;
    }
    s = &gSprites[id];
    Feet(o, &x, &y, &w);
    // people (the player in any LOOK, every NPC, GEN 4 or not): all the same, the user's mock-up oval; POKeMON by their
    // width (1.63.9)
    size = person ? 0 : w <= 16 ? 0 : w <= 32 ? 1 : w <= 48 ? 2 : 3;
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
    if (IsOurs(id) && !gSprites[id].invisible)
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
            Cast(e->spriteId, TRUE);
        }
        for (i = 0; i < MAX_SPRITES; i++)   // followers and Garden POKeMON (mp_follower.c marks them)
        {
            struct Sprite *o = &gSprites[i];
            if (o->inUse && o->mpCastShadow && !o->invisible && !o->mpClipOn && !sSeen[i])
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
