// Moemon Co-op: followers on the ground (task 196). A follower (mp_follower.c) is a plain sprite, not a map
// object, so the game's ground effects never see it. This gives it the player's: tall and long grass it walks
// through (shaking as it steps in, covering its feet), short grass, sand and deep-sand footprints, the sand
// heap, puddle splashes, flowing water at its feet, hot springs water, ripples, and its reflection in water
// and ice. The art, sounds and timing are the game's own field effects (the same templates, palettes and
// rules as event_object_movement.c's GetAllGroundEffectFlags_*), driven from the follower's tiles.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "palette.h"
#include "sound.h"
#include "sprite.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "mp.h"

extern const struct SpritePalette gSpritePalette_GeneralFieldEffect0;
extern const struct SpritePalette gSpritePalette_GeneralFieldEffect1;

#define FX_SLOTS (1 + MP_MAX_PLAYERS)

enum { FX_TALL_GRASS, FX_LONG_GRASS, FX_SPLASH, FX_FLOW, FX_SPRINGS, FX_PILE, FX_SHORT_GRASS };

struct FollowFx
{
    bool8 active;          // the follower was on the field this frame
    bool8 known;           // its tiles below are from last frame
    u8 spriteId;
    struct Coords16 cur, prev;
    u8 moved;              // pixels into its step (16 = standing on cur)
    bool8 stepDone;
    u8 size;               // 32 or 64
    u8 elevation;
    u8 dir;                // DIR_*
    bool8 hidden;
    bool8 inShortGrass, inFlow, inSprings, inPile;
    u8 reflection;         // its reflection sprite (SPRITE_NONE)
};

static struct FollowFx sFx[FX_SLOTS];
static bool8 sInit;
static bool8 sTrace;   // MOEMON_FOLLOWFXTRACE: log each effect a follower starts (tests)

static void Trace(u8 slot, const char *what)
{
    if (sTrace)
        printf("[test] followfx slot %d: %s at %d,%d\n", slot, what, sFx[slot].cur.x - MAP_OFFSET, sFx[slot].cur.y - MAP_OFFSET);
}

// Sprite data of the effect sprites
#define sSlot     data[0]
#define sX        data[1]
#define sY        data[2]
#define sMoved    data[3]
#define sMap      data[4]
#define sKind     data[5]
#define sPrevX    data[6]
#define sPrevY    data[7]

static void SpriteCB_Effect(struct Sprite *sprite);
static void SpriteCB_Reflection(struct Sprite *sprite);

static void InitIfNeeded(void)
{
    u8 i;
    if (sInit)
        return;
    sInit = TRUE;
    sTrace = getenv("MOEMON_FOLLOWFXTRACE") != NULL;
    for (i = 0; i < FX_SLOTS; i++)
    {
        memset(&sFx[i], 0, sizeof(sFx[i]));
        sFx[i].reflection = SPRITE_NONE;
    }
}

static u16 CurrentMap(void)
{
    return ((u8)gSaveBlock1Ptr->location.mapNum << 8) | (u8)gSaveBlock1Ptr->location.mapGroup;
}

static struct Sprite *FollowerSprite(struct FollowFx *f)
{
    if (!f->active || f->spriteId >= MAX_SPRITES || !MP_Follower_IsFollowerSprite(&gSprites[f->spriteId]))
        return NULL;
    return &gSprites[f->spriteId];
}

static bool8 OnTile(struct FollowFx *f, s16 x, s16 y)
{
    return (f->cur.x == x && f->cur.y == y) || (f->prev.x == x && f->prev.y == y);
}

void FieldEffectFreeGraphicsResources(struct Sprite *sprite);   // field_effect.c

static void StopEffect(struct Sprite *sprite)
{
    FieldEffectFreeGraphicsResources(sprite);
}

static u8 StartEffect(u8 slot, u8 kind)
{
    static const u8 sTemplates[] = {
        [FX_TALL_GRASS]  = FLDEFFOBJ_TALL_GRASS,
        [FX_LONG_GRASS]  = FLDEFFOBJ_LONG_GRASS,
        [FX_SPLASH]      = FLDEFFOBJ_SPLASH,
        [FX_FLOW]        = FLDEFFOBJ_SPLASH,
        [FX_SPRINGS]     = FLDEFFOBJ_HOT_SPRINGS_WATER,
        [FX_PILE]        = FLDEFFOBJ_SAND_PILE,
        [FX_SHORT_GRASS] = FLDEFFOBJ_SHORT_GRASS,
    };
    // the palettes the game's field effect scripts load for them
    const struct SpritePalette *pal = (kind == FX_SPLASH || kind == FX_FLOW || kind == FX_PILE)
                                    ? &gSpritePalette_GeneralFieldEffect0 : &gSpritePalette_GeneralFieldEffect1;
    struct FollowFx *f = &sFx[slot];
    struct Sprite *fs = FollowerSprite(f), *sprite;
    s16 x = f->cur.x, y = f->cur.y;
    u8 id;

    if (fs == NULL || f->hidden)
        return MAX_SPRITES;
    LoadSpritePalette(pal);
    UpdateSpritePaletteWithWeather(IndexOfSpritePaletteTag(pal->tag));
    if (kind == FX_TALL_GRASS || kind == FX_LONG_GRASS)
        SetSpritePosToOffsetMapCoords(&x, &y, 8, 8);
    else
        x = fs->x, y = fs->y;
    id = CreateSpriteAtEnd(gFieldEffectObjectTemplatePointers[sTemplates[kind]], x, y, 0);
    if (id == MAX_SPRITES)
        return MAX_SPRITES;
    {
        static const char *const sNames[] = {"tall grass", "long grass", "splash", "flowing water", "hot springs",
                                             "sand heap", "short grass"};
        Trace(slot, sNames[kind]);
    }
    sprite = &gSprites[id];
    sprite->callback = SpriteCB_Effect;
    sprite->coordOffsetEnabled = TRUE;
    sprite->sSlot = slot;
    sprite->sKind = kind;
    sprite->sX = f->cur.x;
    sprite->sY = f->cur.y;
    sprite->sMoved = FALSE;
    sprite->sMap = CurrentMap();
    sprite->sPrevX = fs->x;
    sprite->sPrevY = fs->y;
    switch (kind)
    {
    case FX_TALL_GRASS:
        sprite->oam.priority = 2;   // as GroundEffect_StepOnTallGrass
        break;
    case FX_LONG_GRASS:
        sprite->oam.priority = ElevationToPriority(f->elevation);
        break;
    case FX_SPLASH:
        sprite->oam.priority = fs->oam.priority;
        sprite->y2 = (f->size >> 1) - 4;
        PlaySE(SE_PUDDLE);
        break;
    case FX_FLOW:
        sprite->oam.priority = fs->oam.priority;
        sprite->y2 = (f->size >> 1) - 4;
        sprite->sPrevX = -1;   // the tile it last splashed on
        sprite->sPrevY = -1;
        StartSpriteAnim(sprite, 1);
        break;
    case FX_SPRINGS:
    case FX_SHORT_GRASS:
        sprite->oam.priority = fs->oam.priority;
        break;
    case FX_PILE:
        sprite->oam.priority = fs->oam.priority;
        sprite->y2 = (f->size >> 1) - 2;
        SeekSpriteAnim(sprite, 2);
        break;
    }
    return id;
}

// The grass at its feet: drawn over it like over the player (the game's grass rule, with the follower counted)
static void GrassSubpriority(struct Sprite *sprite, u8 elevation, u8 subpriority)
{
    MP_UpdateGrassFieldEffectSubpriority(sprite, elevation, subpriority);
}

static void SpriteCB_Effect(struct Sprite *sprite)
{
    struct FollowFx *f = &sFx[sprite->sSlot];
    struct Sprite *fs = FollowerSprite(f);
    u8 behavior;

    switch (sprite->sKind)
    {
    case FX_TALL_GRASS:
    case FX_LONG_GRASS:
        if (gCamera.active && sprite->sMap != CurrentMap())
        {
            sprite->sX -= gCamera.x;
            sprite->sY -= gCamera.y;
            sprite->sMap = CurrentMap();
        }
        behavior = MapGridGetMetatileBehaviorAt(sprite->sX, sprite->sY);
        if (fs == NULL
         || !(sprite->sKind == FX_TALL_GRASS ? MetatileBehavior_IsTallGrass(behavior) : MetatileBehavior_IsLongGrass(behavior))
         || (sprite->sMoved && sprite->animEnded))
        {
            StopEffect(sprite);
            return;
        }
        if (!OnTile(f, sprite->sX, sprite->sY))
            sprite->sMoved = TRUE;
        UpdateObjectEventSpriteInvisibility(sprite, FALSE);
        GrassSubpriority(sprite, f->elevation, (sprite->sKind == FX_TALL_GRASS && sprite->animCmdIndex == 0) ? 4 : 0);
        break;
    case FX_SPLASH:
        if (fs == NULL || sprite->animEnded)
        {
            StopEffect(sprite);
            return;
        }
        sprite->x = fs->x;
        sprite->y = fs->y;
        UpdateObjectEventSpriteInvisibility(sprite, f->hidden || fs->invisible);
        break;
    case FX_FLOW:
        if (fs == NULL || !f->inFlow)
        {
            StopEffect(sprite);
            return;
        }
        sprite->x = fs->x;
        sprite->y = fs->y;
        sprite->subpriority = fs->subpriority;
        UpdateObjectEventSpriteInvisibility(sprite, f->hidden || fs->invisible);
        if (f->cur.x != sprite->sPrevX || f->cur.y != sprite->sPrevY)
        {
            sprite->sPrevX = f->cur.x;
            sprite->sPrevY = f->cur.y;
            if (!sprite->invisible)
                PlaySE(SE_PUDDLE);
        }
        break;
    case FX_SPRINGS:
        if (fs == NULL || !f->inSprings)
        {
            StopEffect(sprite);
            return;
        }
        sprite->x = fs->x;
        sprite->y = fs->y + (f->size >> 1) - 8;
        sprite->subpriority = fs->subpriority - 1;
        UpdateObjectEventSpriteInvisibility(sprite, f->hidden || fs->invisible);
        break;
    case FX_PILE:
    case FX_SHORT_GRASS:
        if (fs == NULL || !(sprite->sKind == FX_PILE ? f->inPile : f->inShortGrass))
        {
            StopEffect(sprite);
            return;
        }
        if (fs->x != sprite->sPrevX || fs->y != sprite->sPrevY)
        {
            // it moved: the sand flies / the grass rustles again
            sprite->sPrevX = fs->x;
            sprite->sPrevY = fs->y;
            if (sprite->animEnded)
                StartSpriteAnim(sprite, 0);
        }
        sprite->x = fs->x;
        sprite->y = fs->y;
        if (sprite->sKind == FX_SHORT_GRASS)
        {
            sprite->y2 = (f->size >> 1) - 8;
            sprite->subpriority = fs->subpriority - 1;
            sprite->oam.priority = fs->oam.priority;
        }
        else
        {
            sprite->subpriority = fs->subpriority;
        }
        UpdateObjectEventSpriteInvisibility(sprite, f->hidden || fs->invisible);
        break;
    default:
        StopEffect(sprite);
        break;
    }
}

// ---------------------------------------------------------------------------
// Reflections (field_effect_helpers.c SetUpReflection / UpdateObjectReflectionSprite, for a follower)

static u8 ReflectionTypeAt(s16 x, s16 y)
{
    u8 b = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsIce(b))
        return REFL_TYPE_ICE;
    if (MetatileBehavior_IsReflective(b))
        return REFL_TYPE_WATER;
    return REFL_TYPE_NONE;
}

// ObjectEventGetNearbyReflectionType for the follower's size
static u8 NearbyReflectionType(struct FollowFx *f)
{
    s16 width = (f->size + 8) >> 4, height = (f->size + 8) >> 4;
    s16 i, j;
    u8 t;

    for (i = 0; i < height; i++)
    {
        if ((t = ReflectionTypeAt(f->cur.x, f->cur.y + 1 + i)) || (t = ReflectionTypeAt(f->prev.x, f->prev.y + 1 + i)))
            return t;
        for (j = 1; j < width; j++)
        {
            if ((t = ReflectionTypeAt(f->cur.x + j, f->cur.y + 1 + i)) || (t = ReflectionTypeAt(f->cur.x - j, f->cur.y + 1 + i))
             || (t = ReflectionTypeAt(f->prev.x + j, f->prev.y + 1 + i)) || (t = ReflectionTypeAt(f->prev.x - j, f->prev.y + 1 + i)))
                return t;
        }
    }
    return REFL_TYPE_NONE;
}

// Reflection palettes for co-op colors. The game's own palettes reflect in their hand-made reflection palettes
// (paler, toward the water's blue: brendan_reflection.pal...); a co-op palette (a follower, a friend, a custom look)
// gets the same treatment here: a slot of its own (tag MP_TAG_REFL_PAL + the palette's slot) holding its colors washed
// 42% toward white, the tint the local player's custom looks get (LoadPlayerObjectReflectionPalette). It follows the
// palette it copies, fades included, and goes when no sprite uses it (MP_ReflectionPalettes_Sweep).
#define MP_TAG_REFL_PAL 0x5B00

u16 MP_ReflectionTint(u16 c)
{
    u16 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
    r += (31 - r) * 42 / 100;
    g += (31 - g) * 42 / 100;
    b += (31 - b) * 42 / 100;
    return r | (g << 5) | (b << 10);
}

// Does this sprite palette slot hold co-op colors (no reflection palette of the game's)?
bool8 MP_ReflectionNeedsOwnPalette(u8 slot)
{
    return slot >= 16 || MP_IsCoopPaletteTag(GetSpritePaletteTagByPaletteNum(slot));
}

// The slot of the reflection palette for the colors in `src` (`src` itself when there's no room)
u8 MP_ReflectionPalette(u8 src)
{
    u16 tag = MP_TAG_REFL_PAL + src, tinted[16], shown[16];
    const u16 *srcOwn = &gPlttBufferUnfaded[OBJ_PLTT_ID(src)], *srcShown = &gPlttBufferFaded[OBJ_PLTT_ID(src)];
    struct MpFadeFit fit;
    u8 slot, i;

    if (src >= 64)
        return src;
    slot = IndexOfSpritePaletteTag(tag);
    if (slot == 0xFF)
    {
        if ((slot = MP_AllocCoopPalette(tag, NULL)) == 0xFF)
            return src;
        MPNet_Log("reflection palette: slot %d for the colors in slot %d (tag %04X)", slot, src,
                  GetSpritePaletteTagByPaletteNum(src));
    }
    for (i = 0; i < 16; i++)
        tinted[i] = MP_ReflectionTint(srcOwn[i]);
    MP_FadeFit_FromColors(&fit, srcOwn, srcShown);   // a fade or blend on the colors it copies: the same on the copy
    for (i = 0; i < 16; i++)
        shown[i] = MP_FadeFit_Apply(&fit, tinted[i]);
    if (memcmp(&gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], tinted, sizeof(tinted)) != 0)
        memcpy(&gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], tinted, sizeof(tinted));
    if (memcmp(&gPlttBufferFaded[OBJ_PLTT_ID(slot)], shown, sizeof(shown)) != 0)
        memcpy(&gPlttBufferFaded[OBJ_PLTT_ID(slot)], shown, sizeof(shown));
    return slot;
}

// MP_FrameUpdate: reflection palettes no sprite shows any more are freed
void MP_ReflectionPalettes_Sweep(void)
{
    u64 used = 0, ours = 0;
    u8 slot;
    u32 i;

    for (slot = 0; slot < 64; slot++)
    {
        u16 tag = GetSpritePaletteTagByPaletteNum(slot);
        if (tag >= MP_TAG_REFL_PAL && tag < MP_TAG_REFL_PAL + 64)
            ours |= (u64)1 << slot;
    }
    if (!ours)
        return;
    for (i = 0; i < MAX_SPRITES; i++)
        if (gSprites[i].inUse && gSprites[i].oam.paletteNum < 64)
            used |= (u64)1 << gSprites[i].oam.paletteNum;
    for (slot = 0; slot < 64; slot++)
        if ((ours & ((u64)1 << slot)) && !(used & ((u64)1 << slot)))
            FreeSpritePaletteByTag(GetSpritePaletteTagByPaletteNum(slot));
}

#define sReflStill data[1]

static bool8 ReflectionIsOurs(struct FollowFx *f)
{
    return f->reflection < MAX_SPRITES && gSprites[f->reflection].inUse
        && gSprites[f->reflection].callback == SpriteCB_Reflection && gSprites[f->reflection].sSlot == f - sFx;
}

static void DestroyReflection(struct FollowFx *f)
{
    if (ReflectionIsOurs(f))
        DestroySprite(&gSprites[f->reflection]);
    f->reflection = SPRITE_NONE;
}

static void CreateReflection(struct FollowFx *f, bool8 still)
{
    struct Sprite *fs = FollowerSprite(f), *r;
    u8 id;

    if (fs == NULL || ReflectionIsOurs(f))
        return;
    id = CreateCopySpriteAt(fs, fs->x, fs->y, 152);
    if (id == MAX_SPRITES)
        return;
    r = &gSprites[id];
    r->callback = SpriteCB_Reflection;
    r->oam.priority = 3;          // under the land tiles, so it only shows on the water / ice
    r->usingSheet = TRUE;         // shares the follower's tiles
    r->anims = gDummySpriteAnimTable;
    StartSpriteAnim(r, 0);
    r->affineAnims = gDummySpriteAffineAnimTable;
    r->affineAnimBeginning = TRUE;
    r->subspriteMode = SUBSPRITES_OFF;
    r->sSlot = f - sFx;
    r->sReflStill = still;
    if (!still)
        r->oam.affineMode = ST_OAM_AFFINE_NORMAL;   // the water's ripple (affine matrix 0, as the player's)
    f->reflection = id;
    SpriteCB_Reflection(r);
    Trace(f - sFx, still ? "ice reflection" : "water reflection");
}

static void SpriteCB_Reflection(struct Sprite *r)
{
    struct FollowFx *f = &sFx[r->sSlot];
    struct Sprite *fs = FollowerSprite(f);

    if (fs == NULL || f->reflection != r - gSprites)
    {
        if (f->reflection == r - gSprites)
            f->reflection = SPRITE_NONE;
        DestroySprite(r);
        return;
    }
    // the follower's colors washed out like the player's reflection (its own reflection palette)
    r->oam.paletteNum = MP_ReflectionPalette(fs->oam.paletteNum);
    r->oam.shape = fs->oam.shape;
    r->oam.size = fs->oam.size;
    r->oam.tileNum = fs->oam.tileNum;
    r->oam.matrixNum = r->sReflStill ? ST_OAM_VFLIP : 0;
    r->invisible = fs->invisible || f->hidden;
    r->x = fs->x;
    r->y = fs->mpBig ? fs->y + 6 : fs->y + f->size - 2;   // a PMD picture mirrors at the bottom of its tile, as the player does
    r->centerToCornerVecX = fs->centerToCornerVecX;
    r->centerToCornerVecY = fs->centerToCornerVecY;
    r->x2 = fs->x2;
    r->y2 = -fs->y2;
    r->coordOffsetEnabled = fs->coordOffsetEnabled;
}

#undef sReflStill

bool8 MP_FollowFx_IsReflection(struct Sprite *sprite)
{
    return sprite->callback == SpriteCB_Reflection;
}

// ---------------------------------------------------------------------------

static void UpdateReflection(struct FollowFx *f)
{
    u8 type = NearbyReflectionType(f);
    if (type == REFL_TYPE_NONE)
        DestroyReflection(f);
    else if (!ReflectionIsOurs(f))
        CreateReflection(f, type == REFL_TYPE_ICE);
}

// The lasting states (GetGroundEffectFlags_ShortGrass, _HotSprings, _ShallowFlowingWater, _SandHeap): on while
// both tiles of its step are of the kind; the effect starts when one comes on
static void UpdateStates(u8 slot)
{
    struct FollowFx *f = &sFx[slot];
    u8 c = MapGridGetMetatileBehaviorAt(f->cur.x, f->cur.y), p = MapGridGetMetatileBehaviorAt(f->prev.x, f->prev.y);
    bool8 shortGrass = MetatileBehavior_IsShortGrass(c) && MetatileBehavior_IsShortGrass(p);
    bool8 springs = MetatileBehavior_IsHotSprings(c) && MetatileBehavior_IsHotSprings(p);
    bool8 flow = (MetatileBehavior_IsShallowFlowingWater(c) && MetatileBehavior_IsShallowFlowingWater(p))
              || (MetatileBehavior_IsPacifidlogLog(c) && MetatileBehavior_IsPacifidlogLog(p));
    bool8 pile = MetatileBehavior_IsDeepSand(c) && MetatileBehavior_IsDeepSand(p);

    if (shortGrass && !f->inShortGrass)
        StartEffect(slot, FX_SHORT_GRASS);
    if (springs && !f->inSprings)
        StartEffect(slot, FX_SPRINGS);
    if (flow && !f->inFlow)
        StartEffect(slot, FX_FLOW);
    if (pile && !f->inPile)
        StartEffect(slot, FX_PILE);
    f->inShortGrass = shortGrass;
    f->inSprings = springs;
    f->inFlow = flow;
    f->inPile = pile;
}

static void OnSpawn(u8 slot)
{
    struct FollowFx *f = &sFx[slot];
    u8 c = MapGridGetMetatileBehaviorAt(f->cur.x, f->cur.y), id;

    f->inShortGrass = f->inSprings = f->inFlow = f->inPile = FALSE;
    DestroyReflection(f);
    UpdateReflection(f);
    if (MetatileBehavior_IsTallGrass(c) && (id = StartEffect(slot, FX_TALL_GRASS)) != MAX_SPRITES)
        SeekSpriteAnim(&gSprites[id], 4);   // already standing in it: the grass at rest
    if (MetatileBehavior_IsLongGrass(c) && (id = StartEffect(slot, FX_LONG_GRASS)) != MAX_SPRITES)
        SeekSpriteAnim(&gSprites[id], 6);
    UpdateStates(slot);
}

static void OnBeginStep(u8 slot)
{
    struct FollowFx *f = &sFx[slot];
    u8 c = MapGridGetMetatileBehaviorAt(f->cur.x, f->cur.y), p = MapGridGetMetatileBehaviorAt(f->prev.x, f->prev.y);

    UpdateReflection(f);
    if (MetatileBehavior_IsTallGrass(c))
        StartEffect(slot, FX_TALL_GRASS);
    if (MetatileBehavior_IsLongGrass(c))
        StartEffect(slot, FX_LONG_GRASS);
    // footprints on the sand it leaves (DoTracksGroundEffect_Footprints)
    if (!f->hidden && (MetatileBehavior_IsDeepSand(p) || MetatileBehavior_IsSandOrDeepSand(p) || MetatileBehavior_IsFootprints(p)))
    {
        gFieldEffectArguments[0] = f->prev.x;
        gFieldEffectArguments[1] = f->prev.y;
        gFieldEffectArguments[2] = 149;
        gFieldEffectArguments[3] = 2;
        gFieldEffectArguments[4] = f->dir;
        FieldEffectStart(MetatileBehavior_IsDeepSand(p) ? FLDEFF_DEEP_SAND_FOOTPRINTS : FLDEFF_SAND_FOOTPRINTS);
        Trace(slot, "footprints");
    }
    if (MetatileBehavior_IsPuddle(c) && MetatileBehavior_IsPuddle(p))
        StartEffect(slot, FX_SPLASH);
    UpdateStates(slot);
}

static void OnFinishStep(u8 slot)
{
    struct FollowFx *f = &sFx[slot];
    struct Sprite *fs = FollowerSprite(f);

    UpdateStates(slot);
    if (fs != NULL && !f->hidden && MetatileBehavior_HasRipples(MapGridGetMetatileBehaviorAt(f->cur.x, f->cur.y)))
    {
        gFieldEffectArguments[0] = fs->x;
        gFieldEffectArguments[1] = fs->y + (f->size >> 1) - 2;
        gFieldEffectArguments[2] = 151;
        gFieldEffectArguments[3] = 3;
        FieldEffectStart(FLDEFF_RIPPLE);
        Trace(slot, "ripple");
    }
}

void MP_FollowFx_Update(u8 slot, u8 spriteId, s16 prevX, s16 prevY, s16 curX, s16 curY, u8 moved, u8 fdir, u8 size,
                        u8 elevation, bool8 hidden)
{
    static const u8 sDirs[] = {DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST};   // mp_follower.c's FDIR_* order
    struct FollowFx *f;
    bool8 spawn, newStep;

    InitIfNeeded();
    if (slot >= FX_SLOTS)
        return;
    f = &sFx[slot];
    spawn = !f->known || !f->active || f->spriteId != spriteId
         || abs(curX - f->cur.x) + abs(curY - f->cur.y) > 2;
    newStep = !spawn && (curX != f->cur.x || curY != f->cur.y);
    f->active = TRUE;
    f->known = TRUE;
    f->spriteId = spriteId;
    f->prev.x = prevX;
    f->prev.y = prevY;
    f->cur.x = curX;
    f->cur.y = curY;
    f->size = size;
    f->elevation = elevation;
    f->dir = sDirs[fdir & 3];
    f->hidden = hidden;
    if (spawn)
    {
        f->moved = moved;
        f->stepDone = moved >= 16;
        OnSpawn(slot);
        return;
    }
    if (newStep)
    {
        f->stepDone = FALSE;
        OnBeginStep(slot);
    }
    f->moved = moved;
    if (moved >= 16 && !f->stepDone)
    {
        f->stepDone = TRUE;
        OnFinishStep(slot);
    }
}

// The follower left the field (called back, its Trainer warped, or gone): its effects end on their next frame
void MP_FollowFx_Remove(u8 slot)
{
    InitIfNeeded();
    if (slot >= FX_SLOTS)
        return;
    DestroyReflection(&sFx[slot]);
    sFx[slot].active = FALSE;
    sFx[slot].known = FALSE;
    sFx[slot].inShortGrass = sFx[slot].inSprings = sFx[slot].inFlow = sFx[slot].inPile = FALSE;
}

// Grass in front of a follower standing in it, behind one standing just north of it (the game's rule for map
// objects in UpdateGrassFieldEffectSubpriority, for followers too)
void MP_FollowFx_GrassSubpriority(struct Sprite *grass)
{
    u8 i;
    InitIfNeeded();
    for (i = 0; i < FX_SLOTS; i++)
    {
        struct Sprite *fs = FollowerSprite(&sFx[i]);
        s16 xlo, xhi, lyhi, ylo, yhi;
        if (fs == NULL || fs->invisible)
            continue;
        xlo = grass->x + grass->centerToCornerVecX;
        xhi = grass->x - grass->centerToCornerVecX;
        if (xlo < fs->x && xhi > fs->x)
        {
            lyhi = fs->y + fs->centerToCornerVecY;
            ylo = grass->y - grass->centerToCornerVecY;
            yhi = ylo + fs->centerToCornerVecY;
            if ((lyhi < yhi || lyhi < ylo) && fs->y > yhi && grass->subpriority <= fs->subpriority)
            {
                grass->subpriority = fs->subpriority + 2;
                return;
            }
        }
    }
}
