// Moemon Kai: drop shadows (OPTIONS > DISPLAY > SHADOWS; the user's mock-up: a soft dark oval under the feet). Every
// person on the field (the player, every NPC in every region, friends), every follower and Garden POKeMON gets a
// shadow sprite at its feet: an oval in a 32x16 frame whose pixels darken what the backgrounds show there
// (sprite->mpShadow: gba_easy_draw.c ShadowPixel), drawn under every other sprite (subpriority 255) at its owner's
// priority, so roofs and tree tops cover it as they cover its owner. Not drawn in water (surfing, diving, swimming
// Garden POKeMON cut off by mpClipOn), for things (inanimate objects) or hidden owners. On SINNOH's 3D maps the shadow
// stands where its owner does (sprite.c UpdateOamCoords places every field sprite by its feet).
#include <stdio.h>
#include "global.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "field_player_avatar.h"
#include "metatile_behavior.h"
#include "sprite.h"
#include "mp.h"
#include "mp_bigsprite.h"
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

// Where a sprite's feet are, the same whatever frame it shows (1.63.8: running up or down lifts the feet in some
// frames, and a shadow following each frame's lowest row hopped): the lowest row of every frame of its current
// animation, and of any frame seen before with the same pictures
static const void *sFeetImages[MAX_SPRITES];
static u8 sFeetBottom[MAX_SPRITES];
static bool8 SteadyBottom(const struct Sprite *sp, u8 *bottom)
{
    u32 id = sp - gSprites, i;
    u8 top, b, best = 0;
    bool8 any = FALSE;
    if (!Readable(sp) || id >= MAX_SPRITES)
        return FALSE;
    for (i = 0; i < 32; i++)
    {
        const union AnimCmd *cmd = &sp->anims[sp->animNum][i];
        if (cmd->type == -1 || cmd->type == -2)
            break;
        if (cmd->type < 0)
            continue;
        if (FrameRows(sp, cmd->frame.imageValue, &top, &b))
        {
            if (!any || b > best)
                best = b;
            any = TRUE;
        }
    }
    if (!any)
        return FALSE;
    if (sFeetImages[id] == sp->images && sFeetBottom[id] > best)
        best = sFeetBottom[id];
    sFeetImages[id] = sp->images;
    sFeetBottom[id] = best;
    *bottom = best;
    return TRUE;
}

// The owner's feet (sprite coordinates, without its hop) and how wide it is
static void Feet(const struct Sprite *o, s32 *x, s32 *y, s32 *w)
{
    *x = o->x + o->x2;
    if (o->mpBig != NULL && o->mpBig->frame != NULL)
    {
        const u8 *f = o->mpBig->frame;
        s16 dy = (s16)(f[2] | f[3] << 8);
        u16 fw = f[4] | f[5] << 8, fh = f[6] | f[7] << 8;
        *y = o->y + dy + fh;
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

static void Cast(u8 owner)
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
    size = w <= 16 ? 0 : w <= 32 ? 1 : w <= 48 ? 2 : 3;
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

// `mp shadowwatch N` (test): the player's shadow against the player's sprite for N frames, its lowest and highest gap
static u16 sWatch;
static s16 sWatchMin, sWatchMax;
void MP_Shadow_Watch(u16 frames)
{
    sWatch = frames;
    sWatchMin = 0x7FFF, sWatchMax = -0x7FFF;
}

static void Watch(void)
{
    u8 owner = gObjectEvents[gPlayerAvatar.objectEventId].spriteId, id;
    if (sWatch == 0 || owner >= MAX_SPRITES)
        return;
    id = sShadowOf[owner];
    if (IsOurs(id))
    {
        s16 gap = (gSprites[id].y + gSprites[id].y2) - (gSprites[owner].y + gSprites[owner].y2);
        if (gap < sWatchMin)
            sWatchMin = gap;
        if (gap > sWatchMax)
            sWatchMax = gap;
    }
    if (--sWatch == 0)
        printf("[test] shadowwatch gap %d..%d (moved %d)\n", sWatchMin, sWatchMax, sWatchMax - sWatchMin);
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
            Cast(e->spriteId);
        }
        for (i = 0; i < MAX_SPRITES; i++)   // followers and Garden POKeMON (mp_follower.c marks them)
        {
            struct Sprite *o = &gSprites[i];
            if (o->inUse && o->mpCastShadow && !o->invisible && !o->mpClipOn && !sSeen[i])
                Cast(i);
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
