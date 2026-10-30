// Moemon Co-op: a Pokémon that walks behind you (and behind other players).
//
// Press SELECT on a Pokémon in the party menu to have it follow you; press
// SELECT on it again to call it back. Other players see your follower too.
//
// Followers are plain sprites, not object events: they never block anyone,
// use no object slots and nothing about them is saved. Each follower keeps
// to the tile its owner just left and moves with the owner's own step
// progress, so it keeps pace at any speed (walking, running, bikes, jumps).
// Frames come from graphics/moemon/followers/followers.bin, cut from the
// overworld sheets by tools/moemon_followers/slice.py.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "map_name_popup.h"
#include "main.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_message_box.h"
#include "event_object_lock.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/species.h"
#include "constants/songs.h"
#include "mp_life_emote.h"
#include "mp_life_field.h"
#include "mp_garden.h"
#include "mp_pmd.h"

struct MpFollowerGfx
{
    u16 dex;
    u8 variant;   // 0 normal, 1 female, 2 alternate form
    u8 size;      // 32 or 64
    u32 offset;   // into gMpFollowerData
};

#include "../data/mp_followers.h"

static const u8 sFollowerData[] __attribute__((aligned(4))) = INCBIN_U8("graphics/moemon/followers/followers.bin");

#define NUM_SLOTS        (1 + MP_MAX_PLAYERS)   // 0 = local player, 1 + id = remote players
#define TAG_FOLLOWER     0x1150
#define WALK_FRAME_TIME  8
#define IDLE_FRAME_TIME  24

enum { FDIR_DOWN, FDIR_UP, FDIR_LEFT, FDIR_RIGHT };

struct FollowerSlot
{
    u16 dex;                  // 0 = none
    u8 female;
    bool8 shiny;
    s16 hue;                  // its Stadium hue (mp_hue.c; 0 = its own colors)
    const struct MpFollowerGfx *gfx;
    u8 spriteId;              // SPRITE_NONE when not created
    struct Coords16 prev;     // tile the follower is leaving
    struct Coords16 cur;      // tile it is walking to (the owner's last tile)
    struct Coords16 ownerLast;
    s16 ownerLastX, ownerLastY;   // owner sprite position last frame
    u16 moved;                // pixels the owner moved in its current step
    u8 dir;
    u8 anim;
    u16 animTimer;
    bool8 placed;
    u8 ownerObj;
    // Ball effects (mp_ballfx.c): it pops out of its ball / goes back with its owner's effect
    bool8 seqKnown;
    u8 lastSeq;
    bool8 pendingIn;      // pop out when it shows up
    u8 popTimer;          // the ball's flight, then the entrance
    u8 pendingTimeout;
    bool8 fxHide;         // hidden by the effect (the ball still flying, or called back)
    bool8 recalling;
    bool8 biking;         // in its ball while its owner rides a bike (BikeBall)
    u8 bikeFrames;
    u32 fx;               // MP_BallFx_Pack
    u8 fxSound;
    struct SpriteFrameImage frames[8];
    struct SpriteTemplate tmpl;
    // MOéMON GARDEN (mp_life_field.c): the follower going about its own business while its
    // Trainer stands still. Its own tiles and step; it hurries back when the Trainer moves on.
    bool8 autoOn;
    bool8 autoStuck;
    bool8 autoReturning;     // on its way back to its Trainer's side
    struct Coords16 aPrev, aCur, aTarget;
    u8 aMoved;               // pixels into its step (16 = standing on aCur)
    u8 aSpeed;               // pixels a frame
    u8 aDir;
    u8 aPose;                // LIFE_POSE_*
    u16 aTimer;
    struct Coords16 trail;   // returning: the tile its Trainer just left (where it falls in behind them)
    u8 ownerSpeed;           // pixels a frame its Trainer last moved at (it runs to keep up)
    u8 aStuckFrames;         // returning with no way through
    // a friend's follower on its own: the tiles their game says it stepped on, replayed in order (the same
    // path and facing on every screen)
    bool8 remoteDriven;
    u8 qLen;
    struct Coords16 q[8];
    struct Coords16 qLast;   // the last tile queued (or where it started)
    // PMD Collab sprite (mp_pmd.c): the whole picture as a big sprite, 8 directions, turning smoothly
    bool8 pmdOn;
    s8 pushX, pushY;        // a big PMD picture drawn this far back from its Trainer
    u8 pdir;                 // PMD_DIR_* shown
    u8 pturn;                // frames until its next 45-degree step toward the facing it wants
    struct MpPmdView pmd;
};

static EWRAM_DATA struct FollowerSlot sSlots[NUM_SLOTS];
static bool8 sTrace;   // MOEMON_FOLLOWTRACE: the local follower's position in pixels every frame (tests)
static bool8 sMoveTrace;   // MOEMON_MOVETRACE: every player's and follower's tile and facing as they change (tests)
static bool8 sInit;
static u32 sLocalPersonality;   // the party Pokémon that follows (0 = none)
static bool8 sLocalLoaded;
static u8 sLocalToggleSeq;      // counts sending it out (the others play the entrance on a change)
static u32 sLeavingPersonality; // called back: shown until its recall has played (0 = none)
static u8 sLeavingState;        // 0 none, 1 waiting to be on the field, 2 its recall is playing

static const union AnimCmd sAnim0[] = {ANIMCMD_FRAME(0, 1), ANIMCMD_END};
static const union AnimCmd sAnim1[] = {ANIMCMD_FRAME(1, 1), ANIMCMD_END};
static const union AnimCmd sAnim2[] = {ANIMCMD_FRAME(2, 1), ANIMCMD_END};
static const union AnimCmd sAnim3[] = {ANIMCMD_FRAME(3, 1), ANIMCMD_END};
static const union AnimCmd sAnim4[] = {ANIMCMD_FRAME(4, 1), ANIMCMD_END};
static const union AnimCmd sAnim5[] = {ANIMCMD_FRAME(5, 1), ANIMCMD_END};
static const union AnimCmd sAnim6[] = {ANIMCMD_FRAME(6, 1), ANIMCMD_END};
static const union AnimCmd sAnim7[] = {ANIMCMD_FRAME(7, 1), ANIMCMD_END};
static const union AnimCmd *const sAnims[] = {sAnim0, sAnim1, sAnim2, sAnim3, sAnim4, sAnim5, sAnim6, sAnim7};

static const struct OamData sOam32 = {.shape = SPRITE_SHAPE(32x32), .size = SPRITE_SIZE(32x32), .priority = 2};
static const struct OamData sOam64 = {.shape = SPRITE_SHAPE(64x64), .size = SPRITE_SIZE(64x64), .priority = 2};

static void SpriteCB_Follower(struct Sprite *sprite)
{
    // Moved every frame by MP_Follower_Update
}

static void InitIfNeeded(void)
{
    u8 i;
    if (sInit)
        return;
    memset(sSlots, 0, sizeof(sSlots));
    for (i = 0; i < NUM_SLOTS; i++)
        sSlots[i].spriteId = SPRITE_NONE;
    sTrace = getenv("MOEMON_FOLLOWTRACE") != NULL;
    sMoveTrace = getenv("MOEMON_MOVETRACE") != NULL;
    sInit = TRUE;
}

static const struct MpFollowerGfx *FindGfx(u16 dex, bool8 female)
{
    const struct MpFollowerGfx *normal = NULL;
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sMpFollowerGfx); i++)
    {
        const struct MpFollowerGfx *g = &sMpFollowerGfx[i];
        if (g->dex != dex)
            continue;
        if (g->variant == 1 && female)
            return g;
        if (g->variant == 0 && !normal)
            normal = g;
    }
    return normal;
}

static bool8 SpriteIsOurs(struct FollowerSlot *s)
{
    return s->spriteId != SPRITE_NONE && s->spriteId < MAX_SPRITES
        && gSprites[s->spriteId].inUse && gSprites[s->spriteId].callback == SpriteCB_Follower
        && gSprites[s->spriteId].data[7] == (s16)(s - sSlots);
}

bool8 MP_Follower_IsFollowerSprite(struct Sprite *sprite)
{
    return sprite->inUse && sprite->callback == SpriteCB_Follower;
}

static void DetachPmd(struct FollowerSlot *s)
{
    if (s->pmdOn)
        MP_Pmd_Detach(&s->pmd, SpriteIsOurs(s) ? &gSprites[s->spriteId] : NULL);
    s->pmdOn = FALSE;
}

static void DestroySlotSprite(struct FollowerSlot *s)
{
    MP_FollowFx_Remove(s - sSlots);   // its grass, splashes and reflection go with it
    DetachPmd(s);
    if (SpriteIsOurs(s))
    {
        u16 tag = TAG_FOLLOWER + (s - sSlots);
        DestroySprite(&gSprites[s->spriteId]);
        FreeSpritePaletteByTag(tag);
    }
    s->spriteId = SPRITE_NONE;
}

static bool8 CreateSlotSprite(struct FollowerSlot *s)
{
    u16 tag = TAG_FOLLOWER + (s - sSlots);
    u32 frameBytes;
    struct SpritePalette pal;
    u16 colors[16];
    u8 i, id;

    if (!s->gfx)
        return FALSE;
    frameBytes = (u32)s->gfx->size * s->gfx->size / 2;
    for (i = 0; i < 8; i++)
    {
        s->frames[i].data = sFollowerData + s->gfx->offset + i * frameBytes;
        s->frames[i].size = frameBytes;
    }
    pal.data = (const u16 *)(sFollowerData + s->gfx->offset + 8 * frameBytes);
    pal.tag = tag;
    memcpy(colors, pal.data, sizeof(colors));
    MP_Hue_ShiftPalette(colors, 0, s->hue);   // a nicknamed one in its Stadium hue (the whole picture: creature art)
    if (MP_AllocCoopPalette(tag, colors) == 0xFF)
        return FALSE; // no palette slot free right now; try again later
    s->tmpl.tileTag = TAG_NONE;
    s->tmpl.paletteTag = tag;
    s->tmpl.oam = s->gfx->size == 64 ? &sOam64 : &sOam32;
    s->tmpl.anims = sAnims;
    s->tmpl.images = s->frames;
    s->tmpl.affineAnims = gDummySpriteAffineAnimTable;
    s->tmpl.callback = SpriteCB_Follower;
    id = CreateSprite(&s->tmpl, 0, 0, 0);
    if (id == MAX_SPRITES)
        return FALSE;
    s->spriteId = id;
    gSprites[id].data[7] = s - sSlots;
    gSprites[id].coordOffsetEnabled = TRUE;
    gSprites[id].invisible = TRUE;
    s->anim = 0xFF;
    // The PMD Collab picture when there's one (the Moemon frames above stay as the fallback)
    s->pmdOn = FALSE;
    if (MP_PmdWanted(FALSE) && MP_Pmd_Attach(&s->pmd, &gSprites[id], s->dex, s->female, s->shiny, s->hue, colors))
    {
        u8 slot = IndexOfSpritePaletteTag(tag);
        if (slot != 0xFF)
            LoadPalette(colors, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);
        s->pmdOn = TRUE;
        s->pdir = MP_Pmd_DirFrom4(s->dir);
        s->pturn = 0;
    }
    return TRUE;
}

static void SetSlotPokemon(struct FollowerSlot *s, u16 dex, bool8 female, bool8 shiny, s16 hue)
{
    const struct MpFollowerGfx *gfx = dex ? FindGfx(dex, female) : NULL;
    if (gfx == s->gfx && s->dex == dex && s->hue == hue && s->shiny == shiny)
        return;
    if (gfx)
        MPNet_Log("follower: slot %d dex %d hue %d%s%s", (int)(s - sSlots), dex, hue, shiny ? " shiny" : "",
                  MP_Pmd_Has(dex) ? " (PMD)" : "");
    if (gfx == s->gfx && s->dex == dex && SpriteIsOurs(s))
    {
        // only its colors changed (renamed, or shiny now): the same sprite with the new palette
        u16 colors[16];
        u32 frameBytes = (u32)s->gfx->size * s->gfx->size / 2;
        u8 slot = IndexOfSpritePaletteTag(TAG_FOLLOWER + (s - sSlots));
        s->hue = hue;
        s->shiny = shiny;
        memcpy(colors, sFollowerData + s->gfx->offset + 8 * frameBytes, sizeof(colors));
        MP_Hue_ShiftPalette(colors, 0, hue);
        if (s->pmdOn)
        {
            u8 anim = s->pmd.anim, dir = s->pmd.dir;
            bool8 loop = s->pmd.loop;
            s->pmdOn = MP_Pmd_Attach(&s->pmd, &gSprites[s->spriteId], dex, s->female, shiny, hue, colors);
            if (s->pmdOn)
                MP_Pmd_SetAnim(&s->pmd, anim, dir, loop);
        }
        if (slot != 0xFF)
            LoadPalette(colors, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);
        return;
    }
    DestroySlotSprite(s);
    s->hue = hue;
    s->shiny = shiny;
    s->dex = gfx ? dex : 0;
    s->female = female;
    s->gfx = gfx;
    s->placed = FALSE;
    s->fxHide = FALSE;
    s->recalling = FALSE;
    s->popTimer = 0;
}

// Is the tile behind the owner somewhere the follower can stand?
static bool8 CanStandBehind(struct ObjectEvent *owner, struct Coords16 *out)
{
    s16 x = owner->currentCoords.x, y = owner->currentCoords.y;
    u8 behavior;
    MoveCoords(GetOppositeDirection(owner->facingDirection), &x, &y);
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsDoor(behavior) || MetatileBehavior_IsWarpDoor(behavior) || MetatileBehavior_IsNonAnimDoor(behavior))
        return FALSE;
    out->x = x;
    out->y = y;
    return TRUE;
}

static void PlaceAtOwner(struct FollowerSlot *s, struct ObjectEvent *owner)
{
    struct Coords16 behind;
    if (s->autoOn && s == &sSlots[0])
        MPNet_Log("follower: put back beside its Trainer");
    s->autoOn = FALSE;   // MOéMON GARDEN: whatever it was doing on its own ends (a warp, a new map)
    s->autoReturning = FALSE;
    s->remoteDriven = FALSE;
    s->qLen = 0;
    s->aPose = LIFE_POSE_NONE;
    s->prev = s->cur = s->ownerLast = owner->currentCoords;
    if (CanStandBehind(owner, &behind))
        s->prev = s->cur = behind; // appear right away instead of after the first step
    s->dir = FDIR_DOWN;
    switch (owner->facingDirection)
    {
    case DIR_NORTH: s->dir = FDIR_UP; break;
    case DIR_WEST:  s->dir = FDIR_LEFT; break;
    case DIR_EAST:  s->dir = FDIR_RIGHT; break;
    }
    s->pdir = MP_Pmd_DirFrom4(s->dir);   // (PMD picture) facing that way right away
    s->pturn = 0;
    s->moved = 16;
    s->placed = TRUE;
    if (owner->spriteId < MAX_SPRITES)
    {
        s->ownerLastX = gSprites[owner->spriteId].x;
        s->ownerLastY = gSprites[owner->spriteId].y;
    }
}

static u8 DirBetween(struct Coords16 from, struct Coords16 to, u8 fallback)
{
    s16 dx = to.x - from.x, dy = to.y - from.y;
    if (dx == 0 && dy == 0)
        return fallback;
    if (abs(dx) >= abs(dy))
        return dx < 0 ? FDIR_LEFT : FDIR_RIGHT;
    return dy < 0 ? FDIR_UP : FDIR_DOWN;
}

// MOéMON GARDEN: can the follower stand on this tile on its own? (never where anyone needs to be)
static bool8 AutoCanStand(struct ObjectEvent *owner, s16 x, s16 y)
{
    u8 behavior, elevation;
    if (x == owner->currentCoords.x && y == owner->currentCoords.y)
        return FALSE;
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsDoor(behavior) || MetatileBehavior_IsWarpDoor(behavior) || MetatileBehavior_IsNonAnimDoor(behavior)
     || MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsWaterfall(behavior)
     || MetatileBehavior_IsJumpEast(behavior) || MetatileBehavior_IsJumpWest(behavior)
     || MetatileBehavior_IsJumpNorth(behavior) || MetatileBehavior_IsJumpSouth(behavior)
     || MetatileBehavior_IsDeepSand(behavior) || MetatileBehavior_IsIce(behavior) || MetatileBehavior_IsEastArrowWarp(behavior) || MetatileBehavior_IsWestArrowWarp(behavior)
     || MetatileBehavior_IsNorthArrowWarp(behavior) || MetatileBehavior_IsSouthArrowWarp(behavior)
     || MetatileBehavior_IsDeepSouthWarp(behavior) || MetatileBehavior_IsEscalator(behavior))
        return FALSE;
    elevation = MapGridGetElevationAt(x, y);
    if (elevation != 0 && elevation != 15 && owner->currentElevation != 0 && owner->currentElevation != 15
     && elevation != owner->currentElevation)
        return FALSE;
    // near its Trainer and on screen
    if (abs(x - owner->currentCoords.x) > 6 || abs(y - owner->currentCoords.y) > 4)
        return FALSE;
    return TRUE;
}

bool8 MP_Follower_CanStandAt(s16 x, s16 y)
{
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return FALSE;
    return AutoCanStand(&gObjectEvents[gPlayerAvatar.objectEventId], x, y);
}

static u8 DirToward(struct Coords16 from, struct Coords16 to, u8 fallback);

// Returning: the next tile on a shortest way to aTarget, within the area around its Trainer where it may
// stand (a breadth-first search back from the target), so a fence or a tree in between doesn't stop it.
// FALSE: no way there.
#define PATH_W 13   // AutoCanStand's area: 6 tiles either side of the Trainer, 4 above and below
#define PATH_H 9
static bool8 PathStep(struct FollowerSlot *s, struct ObjectEvent *owner, struct Coords16 *next)
{
    static const s8 d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    u8 dist[PATH_W * PATH_H];
    u8 queue[PATH_W * PATH_H];
    u16 head = 0, tail = 0;
    s16 ox = owner->currentCoords.x - PATH_W / 2, oy = owner->currentCoords.y - PATH_H / 2;
    s16 tx = s->aTarget.x - ox, ty = s->aTarget.y - oy;
    u8 i, best = 0xFF;
    if (tx < 0 || tx >= PATH_W || ty < 0 || ty >= PATH_H)
        return FALSE;
    memset(dist, 0xFF, sizeof(dist));
    dist[ty * PATH_W + tx] = 0;
    queue[tail++] = ty * PATH_W + tx;
    while (head < tail)
    {
        u8 c = queue[head++];
        s16 cx = c % PATH_W, cy = c / PATH_W;
        for (i = 0; i < 4; i++)
        {
            s16 nx = cx + d[i][0], ny = cy + d[i][1];
            u8 n;
            if (nx < 0 || nx >= PATH_W || ny < 0 || ny >= PATH_H)
                continue;
            n = ny * PATH_W + nx;
            if (dist[n] != 0xFF)
                continue;
            // (its own tile counts as open, whatever is there: it's where the way starts)
            if (!(nx + ox == s->aCur.x && ny + oy == s->aCur.y) && !AutoCanStand(owner, nx + ox, ny + oy))
                continue;
            dist[n] = dist[c] + 1;
            queue[tail++] = n;
        }
    }
    for (i = 0; i < 4; i++)
    {
        s16 nx = s->aCur.x + d[i][0] - ox, ny = s->aCur.y + d[i][1] - oy;
        u8 n;
        if (nx < 0 || nx >= PATH_W || ny < 0 || ny >= PATH_H)
            continue;
        n = ny * PATH_W + nx;
        if (dist[n] == 0xFF || !AutoCanStand(owner, nx + ox, ny + oy))
            continue;
        if (best == 0xFF || dist[n] < dist[best])
        {
            best = n;
            next->x = nx + ox;
            next->y = ny + oy;
        }
    }
    return best != 0xFF;
}

// One step of its own walk toward aTarget (greedy, around small obstacles; returning: a real way there)
static void AutoStep(struct FollowerSlot *s, struct ObjectEvent *owner)
{
    s16 dx, dy, i;
    struct Coords16 tries[4];
    u8 n = 0;
    if (s->aMoved < 16)
    {
        // (a friend's follower behind on its steps: a little quicker until it's caught up)
        s->aMoved += s->remoteDriven && s->qLen >= 2 ? s->aSpeed * 2 : s->aSpeed;
        if (s->aMoved > 16)
            s->aMoved = 16;
        return;
    }
    s->aPrev = s->aCur;
    if (s->remoteDriven)
    {
        // its game's steps, in order: the same tiles, so the same facings
        while (s->qLen && s->q[0].x == s->aCur.x && s->q[0].y == s->aCur.y)
        {
            memmove(&s->q[0], &s->q[1], (s->qLen - 1) * sizeof(s->q[0]));
            s->qLen--;
        }
        if (s->qLen == 0)
            return;
        if (abs(s->q[0].x - s->aCur.x) + abs(s->q[0].y - s->aCur.y) == 1)
        {
            s->aCur = s->q[0];
            memmove(&s->q[0], &s->q[1], (s->qLen - 1) * sizeof(s->q[0]));
            s->qLen--;
            s->aMoved = 0;
            s->aDir = DirToward(s->aPrev, s->aCur, s->aDir);
            s->autoStuck = FALSE;
            return;
        }
        s->aTarget = s->q[0];   // (a gap: steps lost to a lag spike) straight there, then on along its path
    }
    dx = s->aTarget.x - s->aCur.x;
    dy = s->aTarget.y - s->aCur.y;
    if (dx == 0 && dy == 0)
        return;
    if (s->autoReturning)
    {
        struct Coords16 next;
        if (PathStep(s, owner, &next))
        {
            s->aCur = next;
            s->aMoved = 0;
            s->aDir = DirToward(s->aPrev, s->aCur, s->aDir);
            s->autoStuck = FALSE;
            return;
        }
        // (no way within the area: the greedy steps below, then the fallback in FinishReturn)
    }
    // the longer way first, then the shorter, then sideways around something in the way
    if (abs(dx) >= abs(dy))
    {
        tries[n].x = s->aCur.x + (dx > 0 ? 1 : -1), tries[n++].y = s->aCur.y;
        if (dy) tries[n].x = s->aCur.x, tries[n++].y = s->aCur.y + (dy > 0 ? 1 : -1);
        else    tries[n].x = s->aCur.x, tries[n++].y = s->aCur.y + 1, tries[n].x = s->aCur.x, tries[n++].y = s->aCur.y - 1;
    }
    else
    {
        tries[n].x = s->aCur.x, tries[n++].y = s->aCur.y + (dy > 0 ? 1 : -1);
        if (dx) tries[n].x = s->aCur.x + (dx > 0 ? 1 : -1), tries[n++].y = s->aCur.y;
        else    tries[n].x = s->aCur.x + 1, tries[n++].y = s->aCur.y, tries[n].x = s->aCur.x - 1, tries[n++].y = s->aCur.y;
    }
    for (i = 0; i < n; i++)
        if (AutoCanStand(owner, tries[i].x, tries[i].y))
        {
            s->aCur = tries[i];
            s->aMoved = 0;
            s->aDir = DirToward(s->aPrev, s->aCur, s->aDir);
            s->autoStuck = FALSE;
            return;
        }
    s->autoStuck = TRUE;
}

static u8 DirToward(struct Coords16 from, struct Coords16 to, u8 fallback)
{
    s16 dx = to.x - from.x, dy = to.y - from.y;
    if (dx == 0 && dy == 0)
        return fallback;
    if (abs(dx) >= abs(dy))
        return dx < 0 ? FDIR_LEFT : FDIR_RIGHT;
    return dy < 0 ? FDIR_UP : FDIR_DOWN;
}

// The frame (and little offsets) for the follower on its own: walking, or its pose
static u8 AutoFrame(struct FollowerSlot *s, struct Sprite *fs)
{
    u16 t = ++s->aTimer;
    fs->x2 = 0;
    if (s->aMoved < 16 && (s->aPrev.x != s->aCur.x || s->aPrev.y != s->aCur.y))
        return s->aDir * 2 + ((s->animTimer / (s->aSpeed > 1 ? 4 : WALK_FRAME_TIME)) & 1);
    switch (MP_LifePose_Base(s->aPose))
    {
    case LIFE_POSE_SIT:
        return s->aDir * 2;
    case LIFE_POSE_NAP:
        fs->y2 = 2;
        return FDIR_DOWN * 2;
    case LIFE_POSE_HOP:
    {
        static const s8 sHop[16] = {0, -2, -4, -5, -6, -6, -5, -4, -2, 0, 0, 0, 0, 0, 0, 0};
        fs->y2 = sHop[(t / 2) % 16];
        return s->aDir * 2 + ((t / 8) & 1);
    }
    case LIFE_POSE_SHAKE:
        fs->x2 = ((t / 3) & 1) ? 1 : -1;
        return s->aDir * 2 + ((t / 6) & 1);
    case LIFE_POSE_SPIN:
    {
        static const u8 sSpin[4] = {FDIR_DOWN, FDIR_RIGHT, FDIR_UP, FDIR_LEFT};
        return sSpin[(t / 8) & 3] * 2;
    }
    case LIFE_POSE_LOOKUP:
        return FDIR_UP * 2;
    }
    return s->aDir * 2 + ((s->animTimer / IDLE_FRAME_TIME) & 1);
}

static bool8 FinishReturn(struct FollowerSlot *s, struct ObjectEvent *owner);

// MOéMON GARDEN: back to its Trainer. It heads for the tile behind them (then for each tile they leave, if
// they walk on) and falls in behind them there (FinishReturn).
static void StartReturn(struct FollowerSlot *s, struct ObjectEvent *owner)
{
    struct Coords16 behind;
    if (s->autoReturning)
        return;
    s->trail = CanStandBehind(owner, &behind) ? behind : s->aCur;
    s->autoReturning = TRUE;
    s->autoStuck = FALSE;
    s->aStuckFrames = 0;
    s->aPose = LIFE_POSE_NONE;
}

// Moves one follower with its owner for this frame. hidden: owner surfing etc.
static void UpdateSlot(struct FollowerSlot *s, u8 ownerObj, bool8 hidden)
{
    struct ObjectEvent *owner;
    struct Sprite *os, *fs;
    s16 dxPix, dyPix;
    s32 fx, fy;
    u8 frame;
    struct Coords16 fxPrev, fxCur;   // the tiles of its step, and how far into it (for its ground effects)
    u8 fxMoved, fxDir;

    if (!s->gfx || ownerObj >= OBJECT_EVENTS_COUNT || !gObjectEvents[ownerObj].active)
    {
        DestroySlotSprite(s);
        s->placed = FALSE;
        return;
    }
    owner = &gObjectEvents[ownerObj];
    if (owner->spriteId >= MAX_SPRITES)
        return;
    os = &gSprites[owner->spriteId];
    if (!SpriteIsOurs(s))
    {
        s->spriteId = SPRITE_NONE;
        if (!CreateSlotSprite(s))
            return;
        s->placed = FALSE;
    }
    fs = &gSprites[s->spriteId];
    {
        // The game can take the palette slot back (sprite.c MP_MakeRoomForPalette):
        // wait for a new one rather than show someone else's colors
        u8 palSlot = IndexOfSpritePaletteTag(TAG_FOLLOWER + (s - sSlots));
        if (palSlot == 0xFF)
        {
            MP_FollowFx_Remove(s - sSlots);
            DetachPmd(s);
            DestroySprite(fs);
            s->spriteId = SPRITE_NONE;
            return;
        }
        fs->oam.paletteNum = palSlot;
    }
    if (!s->placed || s->ownerObj != ownerObj)
    {
        s->ownerObj = ownerObj;
        PlaceAtOwner(s, owner);
    }

    // The owner reached a new tile: follow into the one it left
    if (owner->currentCoords.x != s->ownerLast.x || owner->currentCoords.y != s->ownerLast.y)
    {
        s16 jump = abs(owner->currentCoords.x - s->ownerLast.x) + abs(owner->currentCoords.y - s->ownerLast.y);
        if (s->autoOn && jump > 3)
        {
            PlaceAtOwner(s, owner);   // warped or teleported
        }
        else if (s->autoOn)
        {
            // MOéMON GARDEN: its Trainer moves on. It runs after them along the tiles they leave and falls in
            // behind them (FinishReturn), never popping over to them. (A friend's follower runs as their game
            // says: DriveRemoteAuto.)
            if (!s->autoReturning && !s->remoteDriven)
            {
                struct Coords16 at = s->aMoved >= 8 ? s->aCur : s->aPrev;
                if (s == &sSlots[0])
                    MP_LifeField_OnHurryBack(abs(at.x - s->ownerLast.x) + abs(at.y - s->ownerLast.y));
                StartReturn(s, owner);
            }
            s->trail = s->ownerLast;
            s->ownerLast = owner->currentCoords;
            s->moved = 0;   // (its Trainer's step, counted as when following: falling in mid-step is seamless)
        }
        else if (jump > 3)
        {
            PlaceAtOwner(s, owner); // warped or teleported
        }
        else
        {
            s->prev = s->cur;
            s->cur = s->ownerLast;
            s->ownerLast = owner->currentCoords;
            s->moved = 0;
            s->dir = DirBetween(s->prev, s->cur, s->dir);
        }
    }
    // How far into its step the owner is (pixels moved since the step began)
    {
        s16 ddx = os->x - s->ownerLastX, ddy = os->y - s->ownerLastY;
        s16 d = abs(ddx) + abs(ddy);
        if (d <= 8) // larger jumps are the camera re-centering, not movement
            s->moved += d;
        if (d > 0 && d <= 8)
            s->ownerSpeed = d;
        s->ownerLastX = os->x;
        s->ownerLastY = os->y;
    }
    {
        s16 ownerStep = abs(owner->currentCoords.x - owner->previousCoords.x) + abs(owner->currentCoords.y - owner->previousCoords.y);
        s16 total = 16 * (ownerStep ? ownerStep : 1);
        s32 frac; // 0..256
        s32 ovx, ovy, fvx, fvy;
        if (s->moved > total)
            s->moved = total;
        frac = s->moved * 256 / total;
        // Where the owner and the follower are along their steps (in 1/256 tiles)
        ovx = owner->previousCoords.x * 256 + (owner->currentCoords.x - owner->previousCoords.x) * frac;
        ovy = owner->previousCoords.y * 256 + (owner->currentCoords.y - owner->previousCoords.y) * frac;
        if (owner->currentCoords.x == s->ownerLast.x && owner->currentCoords.y == s->ownerLast.y && s->moved >= total)
        {
            ovx = owner->currentCoords.x * 256;
            ovy = owner->currentCoords.y * 256;
        }
        if (s->autoOn && s->autoReturning && !s->remoteDriven && !FinishReturn(s, owner))
        {
            // on its way back: to the tile its Trainer left, fast enough to catch up (twice their pace)
            bool8 ownerStill = owner->currentCoords.x == s->ownerLast.x && owner->currentCoords.y == s->ownerLast.y
                            && s->moved >= total;
            u8 speed = ownerStill ? 2 : s->ownerSpeed * 2;
            s->aTarget = s->trail;
            s->aSpeed = speed < 2 ? 2 : speed > 6 ? 6 : speed;
        }
        fvx = s->prev.x * 256 + (s->cur.x - s->prev.x) * frac;
        fvy = s->prev.y * 256 + (s->cur.y - s->prev.y) * frac;
        fxPrev = s->prev;
        fxCur = s->cur;
        fxMoved = frac >= 256 ? 16 : frac * 16 / 256;
        fxDir = s->dir;
        if (s->autoOn)
        {
            // its own walk (MOéMON GARDEN)
            AutoStep(s, owner);
            fvx = s->aPrev.x * 256 + (s->aCur.x - s->aPrev.x) * s->aMoved * 16;
            fvy = s->aPrev.y * 256 + (s->aCur.y - s->aPrev.y) * s->aMoved * 16;
            fxPrev = s->aPrev;
            fxCur = s->aCur;
            fxMoved = s->aMoved >= 16 ? 16 : s->aMoved;
            fxDir = s->aDir;
        }
        dxPix = (s16)((fvx - ovx) * 16 / 256);
        dyPix = (s16)((fvy - ovy) * 16 / 256);
        if (s == &sSlots[0] && sTrace)
            printf("[test] followtrace %ld %ld auto %d ret %d\n", (long)(fvx * 16 / 256), (long)(fvy * 16 / 256), s->autoOn, s->autoReturning);
        fx = os->x + dxPix;
        fy = os->y + dyPix;
        if (s->pmdOn)
        {
            // a PMD picture hangs from its ground point (its feet). A big one stands a little farther back from its
            // Trainer, so its body doesn't spread over them (up to a tile more, easing in and out)
            // (away from its Trainer: the way from their tile to its tile)
            u16 reach = MP_Pmd_Reach(&s->pmd);
            s16 want = s->autoOn || reach <= 18 ? 0 : (reach - 16 > 16 ? 16 : reach - 16);
            s16 ax = s->cur.x - owner->currentCoords.x, ay = s->cur.y - owner->currentCoords.y;
            s16 tx = (ax > 0) - (ax < 0), ty = (ay > 0) - (ay < 0);
            tx *= want;
            ty *= want;
            s->pushX += tx > s->pushX ? 1 : tx < s->pushX ? -1 : 0;
            s->pushY += ty > s->pushY ? 1 : ty < s->pushY ? -1 : 0;
            fx += s->pushX;
            fy += PMD_GROUND_DY + s->pushY;
        }
        else if (s->gfx->size == 64)
            fy -= 16;
        fs->x = fx;
        fs->y = fy;
        fs->x2 = 0;
        // Hop along when the owner jumps (ledges)
        fs->y2 = (abs(s->cur.x - s->prev.x) + abs(s->cur.y - s->prev.y) >= 2) ? os->y2 : 0;

        // Walking frames while moving, a slow idle bob while standing
        s->animTimer++;
        if (s->autoOn)
        {
            s16 hop = fs->y2;
            frame = AutoFrame(s, fs);
            if (s->pmdOn)   // the PMD picture has its own motion: no bobbing offsets
            {
                fs->x2 = 0;
                fs->y2 = hop;
                MP_Pmd_Animate(&s->pmd, &s->pdir, &s->pturn,
                               s->aMoved < 16 && (s->aPrev.x != s->aCur.x || s->aPrev.y != s->aCur.y), s->aDir, s->aPose,
                               s->aSpeed > 1);
            }
        }
        else
        {
            bool8 walking = s->moved < total && (s->prev.x != s->cur.x || s->prev.y != s->cur.y);
            fs->x2 = 0;
            if (walking)
                frame = s->dir * 2 + ((s->animTimer / WALK_FRAME_TIME) & 1);
            else
                frame = s->dir * 2 + ((s->animTimer / IDLE_FRAME_TIME) & 1);
            if (s->pmdOn)
                MP_Pmd_Animate(&s->pmd, &s->pdir, &s->pturn, walking, s->dir, LIFE_POSE_NONE, s->ownerSpeed > 1);
        }
    }
    if (s->pmdOn)
        MP_Pmd_Tick(&s->pmd, fs);
    if (frame != s->anim)
    {
        StartSpriteAnim(fs, frame);
        s->anim = frame;
    }
    // Same layer as the owner; behind them when level with them
    fs->oam.priority = os->oam.priority;
    if (s->pmdOn)
    {
        fs->y -= s->pushY;   // (the depth of where it stands, not of the picture drawn back)
        MP_Pmd_SetSubpriority(fs, owner->previousElevation, 2);
        fs->y += s->pushY;
    }
    else
        SetObjectSubpriorityByElevation(owner->previousElevation, fs, 2);
    fs->invisible = hidden || os->invisible || s->fxHide
        || (!s->autoOn && s->cur.x == owner->currentCoords.x && s->cur.y == owner->currentCoords.y && s->moved >= 16);
    if (s->recalling && !MP_BallFx_BodyRunning(s->spriteId))
        fs->invisible = TRUE;   // gone back into its ball: stays hidden until it's really gone
    // Someone else stepped onto its tile (a friend hopping off a piggyback
    // ride, or walking up behind): tuck it away instead of covering them
    if (!fs->invisible && s->moved >= 16 && !s->autoOn)
    {
        u8 other = GetObjectEventIdByXY(s->cur.x, s->cur.y);
        if (other != OBJECT_EVENTS_COUNT && other != ownerObj)
            fs->invisible = TRUE;
    }
    // Grass, sand, water and reflections under it, as under its Trainer (mp_followfx.c)
    MP_FollowFx_Update(s - sSlots, s->spriteId, fxPrev.x, fxPrev.y, fxCur.x, fxCur.y, fxMoved, fxDir, s->gfx->size,
                       owner->previousElevation, fs->invisible);
}

// ---------------------------------------------------------------------------
// The local player's follower

static struct Pokemon *FindPartyMon(u32 personality)
{
    u8 i;
    if (personality == 0)
        return NULL;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_PERSONALITY) == personality)
            return mon;
    }
    return NULL;
}

// Moemon Kai: the party Pokémon following you, NULL when none (Johto / Kanto scripts: qz_bufferfollowerspecies)
struct Pokemon *MP_Follower_LocalMon(void)
{
    return sLocalPersonality ? FindPartyMon(sLocalPersonality) : NULL;
}

// The Pokémon shown behind you: the follower, or the one going back into its ball
static struct Pokemon *FindLocalFollower(void)
{
    if (sLeavingState && sLeavingPersonality)
        return FindPartyMon(sLeavingPersonality);
    return FindPartyMon(sLocalPersonality);
}

u32 MP_Follower_LocalFx(u8 *sound)
{
    struct Pokemon *mon = FindLocalFollower();
    struct MpBallFx fx;
    *sound = 0;
    if (!mon)
        return 0;
    MP_BallFx_OfMon(mon, &fx);
    *sound = fx.sound | (sLeavingState == 2 ? 0x8 : 0);   // bit 3: its recall is playing
    return MP_BallFx_Pack(&fx);
}

u8 MP_Follower_LocalToggleSeq(void)
{
    return sLocalToggleSeq;
}

static void LoadLocalChoice(void)
{
    if (sLocalLoaded)
        return;
    sLocalLoaded = TRUE;
    sLocalPersonality = gMpConfig.followerPersonality;
}

// The follower's national dex number and gender, for the network (0 = none)
u16 MP_Follower_LocalDex(bool8 *female)
{
    struct Pokemon *mon;
    LoadLocalChoice();
    mon = FindLocalFollower();
    *female = FALSE;
    if (MP_Prank_MagikarpActive())
        return NATIONAL_DEX_MAGIKARP;   // Moemon Co-op prank: everyone sees a Magikarp
    if (!mon || GetMonData(mon, MON_DATA_HP) == 0)
        return 0;
    *female = GetMonGender(mon) == MON_FEMALE;
    return SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
}

// The follower's Stadium hue, for the network (mp_hue.c; 0 = its own colors)
bool8 MP_Follower_LocalShiny(void)
{
    struct Pokemon *mon;
    LoadLocalChoice();
    mon = FindLocalFollower();
    return mon && !MP_Prank_MagikarpActive() && IsMonShiny(mon);
}

s16 MP_Follower_LocalHue(void)
{
    struct Pokemon *mon;
    LoadLocalChoice();
    mon = FindLocalFollower();
    if (MP_Prank_MagikarpActive() || !mon)
        return 0;
    return MP_Hue_OfBoxMon(&mon->box);
}

bool8 MP_Follower_IsFollowing(struct Pokemon *mon)
{
    LoadLocalChoice();
    return sLocalPersonality != 0 && GetMonData(mon, MON_DATA_PERSONALITY) == sLocalPersonality;
}

// Walking with you makes a Moemon happier: +FOLLOW_JOY friendship when it's chosen
// (once a day per Moemon, so toggling it back and forth gains nothing) and +1 every
// 128 steps on top of the party's own walking friendship (MP_Follower_OnWalkSteps)
#define FOLLOW_JOY 5

static void AddFriendship(struct Pokemon *mon, u8 amount)
{
    s32 f = GetMonData(mon, MON_DATA_FRIENDSHIP);
    u8 v;
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return;
    f += amount;
    v = f > MAX_FRIENDSHIP ? MAX_FRIENDSHIP : f;
    SetMonData(mon, MON_DATA_FRIENDSHIP, &v);
}

static void JoyOfBeingChosen(struct Pokemon *mon)
{
    char key[40];
    snprintf(key, sizeof(key), "followjoy:%d:%08x", (int)MP_Today(), (unsigned)GetMonData(mon, MON_DATA_PERSONALITY));
    if (MP_SaveData_Get(key))
        return;
    MP_SaveData_Set(key, 1);
    AddFriendship(mon, FOLLOW_JOY);
    MPNet_Log("follower: friendship +%d -> %d", FOLLOW_JOY, (int)GetMonData(mon, MON_DATA_FRIENDSHIP));
}

// Every 128 steps (field_control_avatar.c UpdateFriendshipStepCounter)
void MP_Follower_OnWalkSteps(void)
{
    u8 i;
    LoadLocalChoice();
    if (sLocalPersonality == 0)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == sLocalPersonality)
        {
            AddFriendship(&gPlayerParty[i], 1);
            return;
        }
}

// SELECT in the party menu. Returns TRUE when the Pokémon now follows.
bool8 MP_Follower_Toggle(struct Pokemon *mon)
{
    LoadLocalChoice();
    if (MP_Follower_IsFollowing(mon))
    {
        // called back: it stays out until its recall has played on the field
        sLeavingPersonality = sLocalPersonality;
        sLeavingState = 1;
        sLocalPersonality = 0;
    }
    else
    {
        sLeavingState = 0;
        sLeavingPersonality = 0;
        sLocalPersonality = GetMonData(mon, MON_DATA_PERSONALITY);
        sLocalToggleSeq = (sLocalToggleSeq + 1) & 3;   // counts sending out only
        JoyOfBeingChosen(mon);
        MP_Achieve_Unlock("follower");
    }
    gMpConfig.followerPersonality = sLocalPersonality;
    MP_SaveConfig();
    return sLocalPersonality != 0;
}

bool8 MP_Follower_HasGraphics(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    return FindGfx(SpeciesToNationalPokedexNum(species), FALSE) != NULL;
}

// The tile the local follower is on, for talking to it
bool8 MP_Follower_LocalTile(s16 *x, s16 *y)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!sInit || !s->gfx || !SpriteIsOurs(s) || gSprites[s->spriteId].invisible)
        return FALSE;
    *x = s->autoOn ? s->aCur.x : s->cur.x;
    *y = s->autoOn ? s->aCur.y : s->cur.y;
    return TRUE;
}

// Moemon Co-op: on a bike the follower rides in its ball. Called back (its recall, toward its
// owner) as they get on, sent out again (the ball tossed, its entrance) as they get off. Returns
// whether it's hidden for it; *leaving: play its recall now.
static bool8 BikeBall(struct FollowerSlot *s, bool8 bike, bool8 *leaving)
{
    if (bike && !s->biking)
    {
        s->biking = TRUE;
        s->bikeFrames = 0;
    }
    else if (!bike && s->biking)
    {
        s->biking = FALSE;
        s->recalling = FALSE;
        s->fxHide = TRUE;            // (hidden until its ball has landed)
        s->pendingIn = TRUE;
        s->pendingTimeout = 120;
        s->popTimer = 0;
    }
    if (!s->biking)
        return FALSE;
    if (s->bikeFrames < 255)
        s->bikeFrames++;
    *leaving = TRUE;
    // shown while its recall plays (or for the frame it starts in), hidden after
    return s->recalling ? FALSE : s->bikeFrames > 2;
}

static bool8 LocalBiking(void)
{
    return (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE)) != 0;
}

static bool8 RemoteBiking(u8 id)
{
    u8 mode = gMpPlayers[id].state.avatarMode;
    return mode == MP_AVATAR_MACH_BIKE || mode == MP_AVATAR_ACRO_BIKE;
}

static bool8 LocalHidden(void)
{
    return (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER)) != 0 || MP_Piggy_IsRiding()
        || MP_Garden_HidesFollowers();   // MOéMON GARDEN: it lives freely there (mp_garden_life.c)
}

static bool8 RemoteHidden(u8 id)
{
    u8 mode = gMpPlayers[id].state.avatarMode;
    // Holding the BATTLE FLAG: no follower either, so they blend in
    return mode == MP_AVATAR_SURF || mode == MP_AVATAR_UNDERWATER || (gMpPlayers[id].state.flags & (MPF_TRAINER_MODE | MPF_RIDING))
        || MP_Garden_HidesFollowers();   // MOéMON GARDEN: friends' followers live there as Garden Pokémon too
}

static s16 ScreenX(const struct Sprite *sp) { return sp->x + sp->x2 + (sp->coordOffsetEnabled ? gSpriteCoordOffsetX : 0); }
static s16 ScreenY(const struct Sprite *sp) { return sp->y + sp->y2 + (sp->coordOffsetEnabled ? gSpriteCoordOffsetY : 0); }

// Ball effects for one follower (mp_ballfx.c). seq: how many times its owner sent one
// out (a change while it's around = it was just sent out); leaving: its recall is playing.
static void SlotEffects(struct FollowerSlot *s, u8 seq, bool8 leaving, u32 fxPacked, u8 sound, u8 ownerObj)
{
    struct MpBallFx fx;
    struct Sprite *fs, *os;

    s->fx = fxPacked;
    s->fxSound = sound;
    if (!s->seqKnown)
    {
        s->seqKnown = TRUE;
        s->lastSeq = seq;
    }
    else if (seq != s->lastSeq)
    {
        s->lastSeq = seq;
        if (!leaving)
        {
            s->pendingIn = TRUE;
            s->pendingTimeout = 120;
        }
    }
    if (!SpriteIsOurs(s) || ownerObj >= OBJECT_EVENTS_COUNT || !s->placed)
    {
        if (s->pendingIn && s->pendingTimeout && --s->pendingTimeout == 0)
            s->pendingIn = FALSE;
        return;
    }
    fs = &gSprites[s->spriteId];
    os = &gSprites[gObjectEvents[ownerObj].spriteId];
    MP_BallFx_Unpack(fxPacked, sound, &fx);

    // sent out: its ball is tossed to its spot and it comes out with its effect
    if (s->pendingIn && !s->recalling)
    {
        if (s->popTimer == 0)
        {
            if (fs->invisible && !s->fxHide)
            {
                if (s->pendingTimeout && --s->pendingTimeout == 0)
                    s->pendingIn = FALSE;
                return;   // waits until it can be seen
            }
            s->fxHide = TRUE;
            fs->invisible = TRUE;
            MP_BallFx_MiniBall(ScreenX(os), ScreenY(os) - 6, ScreenX(fs), ScreenY(fs) + 6, fx.color, 12);
            s->popTimer = 1;
        }
        else if (++s->popTimer >= 13)
        {
            if (fx.form == MPFX_FORM_DEFAULT)
                fx.form = MPFX_FORM_VANILLA;
            if (fx.part == MPFX_PART_DEFAULT)
                fx.part = MPFX_PART_NONE;
            s->fxHide = FALSE;
            fs->invisible = FALSE;
            PlaySE(SE_BALL_OPEN);
            MP_BallFx_StartEntrance(s->spriteId, ScreenX(fs), ScreenY(fs) + 6, MPFX_FIELD, &fx);
            s->pendingIn = FALSE;
            s->popTimer = 0;
        }
        return;
    }
    // called back: its recall, toward its owner
    if (leaving && !s->recalling && !fs->invisible)
    {
        if (fx.recall == MPFX_RECALL_DEFAULT)
            fx.recall = MPFX_RECALL_VANILLA;
        PlaySE(SE_BALL_OPEN);
        if (MP_BallFx_StartRecall(s->spriteId, ScreenX(os), ScreenY(os), MPFX_FIELD, &fx))
            s->recalling = TRUE;
    }
}

// The local follower being called back: shown until its recall has played
static void UpdateLeaving(void)
{
    static u8 sWait;
    struct FollowerSlot *s = &sSlots[0];
    if (sLeavingState == 0)
        return;
    if (sLeavingState == 1)
    {
        // waits to be seen on the field (not after a while: hidden, surfing...)
        if (SpriteIsOurs(s) && s->placed && !gSprites[s->spriteId].invisible)
        {
            sLeavingState = 2;
            sWait = 0;
        }
        else if (++sWait > 90 || FindPartyMon(sLeavingPersonality) == NULL)
        {
            sLeavingState = 0;
            sLeavingPersonality = 0;
            sWait = 0;
        }
        return;
    }
    // its recall is over (or never started)
    if ((s->recalling && !MP_BallFx_BodyRunning(s->spriteId)) || !SpriteIsOurs(s) || ++sWait > 60)
    {
        sLeavingState = 0;
        sLeavingPersonality = 0;
        sWait = 0;
    }
}

// Every overworld frame (src/overworld.c, before sprites are drawn)
static void DriveRemoteAuto(struct FollowerSlot *s, u8 obj, u8 id, const struct MpState *st);

// Tests (MOEMON_MOVETRACE, tools/test/check_movetrace.py): each player's and follower's tile and facing, logged as
// they change, under the player's id, so two games' logs can be compared step by step
static void MoveTrace(void)
{
    static s16 last[2 * MP_MAX_PLAYERS][3];
    static bool8 lastValid[2 * MP_MAX_PLAYERS];
    u8 i;
    if (gMpLocalId < 0)
        return;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        u8 obj = i == gMpLocalId ? gPlayerAvatar.objectEventId : MP_GetRemoteObjectEventId(i);
        struct FollowerSlot *s = i == gMpLocalId ? &sSlots[0] : &sSlots[1 + i];
        s16 v[2][3];
        bool8 ok[2] = {FALSE, FALSE};
        u8 k;
        if (obj < OBJECT_EVENTS_COUNT && gObjectEvents[obj].active)
        {
            v[0][0] = gObjectEvents[obj].currentCoords.x;
            v[0][1] = gObjectEvents[obj].currentCoords.y;
            v[0][2] = gObjectEvents[obj].facingDirection;
            ok[0] = TRUE;
        }
        if (s->gfx && s->placed && SpriteIsOurs(s) && !gSprites[s->spriteId].invisible)
        {
            v[1][0] = s->autoOn ? s->aCur.x : s->cur.x;
            v[1][1] = s->autoOn ? s->aCur.y : s->cur.y;
            v[1][2] = s->autoOn ? s->aDir : s->dir;
            ok[1] = TRUE;
        }
        for (k = 0; k < 2; k++)
        {
            u8 n = i * 2 + k;
            if (!ok[k])
            {
                lastValid[n] = FALSE;
                continue;
            }
            if (lastValid[n] && last[n][0] == v[k][0] && last[n][1] == v[k][1] && last[n][2] == v[k][2])
                continue;
            memcpy(last[n], v[k], sizeof(last[n]));
            lastValid[n] = TRUE;
            printf("[test] mv %c%d %d %d %d\n", k ? 'F' : 'P', i, v[k][0] - MAP_OFFSET, v[k][1] - MAP_OFFSET, v[k][2]);
        }
    }
}

void MP_Follower_Update(void)
{
    u8 i;
    bool8 female;
    u16 dex;

    InitIfNeeded();
    if (!MP_IsInOverworld())
        return;
    // OPTIONS > FOLLOWERS changed: every follower's sprite again (the other kind)
    for (i = 0; i < ARRAY_COUNT(sSlots); i++)
        if (SpriteIsOurs(&sSlots[i]) && sSlots[i].pmdOn != (MP_PmdWanted(FALSE) && MP_Pmd_Has(sSlots[i].dex)))
        {
            DestroySlotSprite(&sSlots[i]);
            sSlots[i].placed = FALSE;
        }

    dex = MP_Follower_LocalDex(&female);
    SetSlotPokemon(&sSlots[0], dex, female, dex && MP_Follower_LocalShiny(), dex ? MP_Follower_LocalHue() : 0);
    {
        bool8 bikeLeaving = FALSE;
        bool8 bikeHidden = BikeBall(&sSlots[0], LocalBiking(), &bikeLeaving);
        u8 sound;
        u32 fx;
        UpdateSlot(&sSlots[0], gPlayerAvatar.objectEventId, LocalHidden() || bikeHidden);
        fx = MP_Follower_LocalFx(&sound);
        SlotEffects(&sSlots[0], sLocalToggleSeq, sLeavingState == 2 || bikeLeaving, fx, sound & 7, gPlayerAvatar.objectEventId);
        UpdateLeaving();
    }

    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct FollowerSlot *s = &sSlots[1 + i];
        u8 obj = OBJECT_EVENTS_COUNT;
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState)
        {
            SetSlotPokemon(s, gMpPlayers[i].state.followerDex, (gMpPlayers[i].state.followerFlags & 1) != 0,
                           (gMpPlayers[i].state.followerFlags & 0x80) != 0, gMpPlayers[i].state.followerHue);
            obj = MP_GetRemoteObjectEventId(i);
        }
        else
        {
            SetSlotPokemon(s, 0, FALSE, FALSE, 0);
            s->seqKnown = FALSE;
        }
        if (obj != OBJECT_EVENTS_COUNT)
            DriveRemoteAuto(s, obj, i, &gMpPlayers[i].state);   // MOéMON GARDEN: on its own, as its Trainer's game says
        {
            bool8 bikeLeaving = FALSE;
            bool8 bikeHidden = obj != OBJECT_EVENTS_COUNT && BikeBall(s, RemoteBiking(i), &bikeLeaving);
            UpdateSlot(s, obj, i < MP_MAX_PLAYERS && gMpPlayers[i].present ? RemoteHidden(i) || bikeHidden : TRUE);
            if (obj != OBJECT_EVENTS_COUNT)
            {
                u8 fl = gMpPlayers[i].state.followerFlags;
                SlotEffects(s, (fl >> 4) & 3, (fl & 0x40) != 0 || bikeLeaving, gMpPlayers[i].state.followerFx, (fl >> 1) & 7, obj);
            }
        }
    }
    MP_LifeInteract_Update();
    MP_LifeField_Update();   // MOéMON GARDEN: the follower's own life (mp_life_field.c)
    if (sMoveTrace)
        MoveTrace();
}

// Talking to your follower
static const u8 sText_Happy[] = _("{STR_VAR_1} is happy to be with you!");

static void Task_FollowerTalk(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (data[0])
    {
    case 0:
        HideMapNamePopUpWindow(); // the route sign scrolls the text box layer
        MP_ShowFieldMessageFitted(gStringVar4);
        data[0]++;
        break;
    case 1:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            data[0]++;
        }
        break;
    case 2:
        if (IsFieldMessageBoxHidden())
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
        }
        break;
    }
}

bool8 MP_Follower_TryInteract(void)
{
    struct ObjectEvent *me;
    struct Pokemon *mon;
    s16 x, y, fx, fy;
    if (!JOY_NEW(A_BUTTON) || !MP_IsLocalFree() || FuncIsActiveTask(Task_FollowerTalk))
        return FALSE;
    if (!MP_Follower_LocalTile(&fx, &fy))
        return FALSE;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    if (x != fx || y != fy || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    mon = FindLocalFollower();
    if (!mon)
        return FALSE;
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    StringExpandPlaceholders(gStringVar4, sText_Happy);
    PlayCry_Normal(GetMonData(mon, MON_DATA_SPECIES), 0);
    LockPlayerFieldControls();
    CreateTask(Task_FollowerTalk, 80);
    return TRUE;
}

// ---------------------------------------------------------------------------
// MOéMON GARDEN (mp_life_field.c): the local follower on its own

static struct ObjectEvent *LocalOwner(void)
{
    if (gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT || !gObjectEvents[gPlayerAvatar.objectEventId].active)
        return NULL;
    return &gObjectEvents[gPlayerAvatar.objectEventId];
}

// It may wander off only while standing (not mid-step) and seen
bool8 MP_Follower_AutoStart(void)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!sInit || !s->gfx || !SpriteIsOurs(s) || !s->placed || s->moved < 16 || s->autoOn
     || gSprites[s->spriteId].invisible || LocalOwner() == NULL)
        return FALSE;
    s->autoOn = TRUE;
    s->autoStuck = FALSE;
    s->autoReturning = FALSE;
    s->aPrev = s->aCur = s->aTarget = s->cur;
    s->aMoved = 16;
    s->aSpeed = 1;
    s->aDir = s->dir;
    s->aPose = LIFE_POSE_NONE;
    s->aTimer = 0;
    return TRUE;
}

bool8 MP_Follower_AutoActive(void)
{
    return sInit && sSlots[0].autoOn;
}

void MP_Follower_AutoTarget(s16 x, s16 y, u8 speed)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!s->autoOn)
        return;
    s->aTarget.x = x;
    s->aTarget.y = y;
    s->aSpeed = speed ? speed : 1;
    s->autoStuck = FALSE;
    s->autoReturning = FALSE;   // (a new errand: it isn't heading back any more)
    if (s->aPose != LIFE_POSE_NONE)
        s->aPose = LIFE_POSE_NONE;
}

bool8 MP_Follower_AutoArrived(void)
{
    struct FollowerSlot *s = &sSlots[0];
    return s->autoOn && s->aMoved >= 16 && s->aCur.x == s->aTarget.x && s->aCur.y == s->aTarget.y;
}

bool8 MP_Follower_AutoStuck(void)
{
    return sSlots[0].autoOn && sSlots[0].autoStuck;
}

void MP_Follower_AutoPose(u8 pose)
{
    if (sSlots[0].autoOn)
    {
        sSlots[0].aPose = pose;
        sSlots[0].aTimer = 0;
    }
}

void MP_Follower_AutoFaceTile(s16 x, s16 y)
{
    struct FollowerSlot *s = &sSlots[0];
    struct Coords16 to;
    if (!s->autoOn)
        return;
    to.x = x;
    to.y = y;
    s->aDir = DirToward(s->aCur, to, s->aDir);
}

void MP_Follower_AutoFace(u8 dir)
{
    if (sSlots[0].autoOn)
        sSlots[0].aDir = dir;
}

// Back to following: it walks to its Trainer's side, then follows as usual
void MP_Follower_AutoReturn(void)
{
    struct FollowerSlot *s = &sSlots[0];
    struct ObjectEvent *owner = LocalOwner();
    if (!s->autoOn || owner == NULL)
        return;
    StartReturn(s, owner);
}

// Called every frame by mp_life_field.c while it's returning: once beside its Trainer, it follows again
bool8 MP_Follower_AutoFinishReturn(void)
{
    return FinishReturn(&sSlots[0], LocalOwner());
}

// Falls in behind its Trainer once it's caught up (TRUE: following again). On the tile they left, it takes up
// following right where their step is (no jump); beside a Trainer who stands still, it just follows from there.
// Boxed in for a moment with no way through: close by it follows from where it is (it slides over on their
// next step); farther, it reappears behind them (the last resort).
static bool8 FinishReturn(struct FollowerSlot *s, struct ObjectEvent *owner)
{
    s16 d;
    bool8 ownerStill;
    if (!s->autoOn || owner == NULL)
        return TRUE;
    if (s->aMoved < 16)
        return FALSE;
    d = abs(s->aCur.x - owner->currentCoords.x) + abs(s->aCur.y - owner->currentCoords.y);
    ownerStill = owner->currentCoords.x == s->ownerLast.x && owner->currentCoords.y == s->ownerLast.y && s->moved >= 16;
    if (s->autoStuck)
        s->aStuckFrames++;
    else
        s->aStuckFrames = 0;
    if (s->aCur.x == s->trail.x && s->aCur.y == s->trail.y && d <= 2)
    {
        // on the tile they left (or behind them): following from here, their step carries on as it is
        s->prev = s->cur = s->aCur;
        if (ownerStill)
            s->ownerLast = owner->currentCoords;
    }
    else if (d <= 1 && ownerStill)
    {
        s->prev = s->cur = s->aCur;
        s->ownerLast = owner->currentCoords;
        s->moved = 16;
    }
    else if (s->aStuckFrames >= 20)
    {
        if (d > 3)
        {
            PlaceAtOwner(s, owner);
            s->autoReturning = FALSE;
            return TRUE;
        }
        s->prev = s->cur = s->aCur;
        s->ownerLast = owner->currentCoords;
    }
    else
    {
        return FALSE;
    }
    s->dir = s->aDir;
    s->autoOn = FALSE;
    s->autoReturning = FALSE;
    s->aPose = LIFE_POSE_NONE;
    if (s == &sSlots[0])
        MPNet_Log("follower trail: fell in behind (%d from its Trainer)", d);
    return TRUE;
}

// Where the local follower stands (its own tile while on its own)
bool8 MP_Follower_LocalPos(s16 *x, s16 *y, u8 *dir)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!sInit || !s->gfx || !SpriteIsOurs(s) || !s->placed)
        return FALSE;
    if (s->autoOn)
    {
        *x = s->aCur.x;
        *y = s->aCur.y;
        *dir = s->aDir;
    }
    else
    {
        *x = s->cur.x;
        *y = s->cur.y;
        *dir = s->dir;
    }
    return TRUE;
}

// For the emote bubble over a follower: its sprite's screen position (FALSE = not shown)
bool8 MP_Follower_SlotScreenTop(u8 slot, s16 *x, s16 *top, u8 *subpriority)
{
    struct FollowerSlot *s;
    struct Sprite *fs;
    if (!sInit || slot >= NUM_SLOTS)
        return FALSE;
    s = &sSlots[slot];
    if (!s->gfx || !SpriteIsOurs(s))
        return FALSE;
    fs = &gSprites[s->spriteId];
    if (fs->invisible)
        return FALSE;
    *x = ScreenX(fs);
    if (s->pmdOn)   // the PMD picture's top, above its feet
        *top = ScreenY(fs) - MP_Pmd_Height(&s->pmd);
    else
        *top = ScreenY(fs) - s->gfx->size / 2 + (s->gfx->size == 64 ? 16 : 4);
    *subpriority = fs->subpriority > 0 ? fs->subpriority - 1 : 0;
    return TRUE;
}

u16 MP_Follower_SlotDex(u8 slot)
{
    return slot < NUM_SLOTS && sInit ? sSlots[slot].dex : 0;
}

// MOéMON GARDEN (mp_garden_life.c): a Pokémon's walking sprite for the Garden. 8 frames (down, up, left, right;
// two steps each), its own palette (Stadium hue). Returns the sprite id or MAX_SPRITES.
u8 MP_Follower_CreateMonSprite(u16 dex, bool8 female, s16 hue, u16 palTag, struct SpriteFrameImage *frames,
                               struct SpriteTemplate *tmpl, u8 *size, void (*callback)(struct Sprite *))
{
    const struct MpFollowerGfx *gfx = dex ? FindGfx(dex, female) : NULL;
    u32 frameBytes;
    u16 colors[16];
    u8 i, id;
    if (gfx == NULL)
        return MAX_SPRITES;
    frameBytes = (u32)gfx->size * gfx->size / 2;
    for (i = 0; i < 8; i++)
    {
        frames[i].data = sFollowerData + gfx->offset + i * frameBytes;
        frames[i].size = frameBytes;
    }
    memcpy(colors, sFollowerData + gfx->offset + 8 * frameBytes, sizeof(colors));
    MP_Hue_ShiftPalette(colors, 0, hue);
    if (IndexOfSpritePaletteTag(palTag) == 0xFF && MP_AllocCoopPalette(palTag, colors) == 0xFF)
        return MAX_SPRITES;
    tmpl->tileTag = TAG_NONE;
    tmpl->paletteTag = palTag;
    tmpl->oam = gfx->size == 64 ? &sOam64 : &sOam32;
    tmpl->anims = sAnims;
    tmpl->images = frames;
    tmpl->affineAnims = gDummySpriteAffineAnimTable;
    tmpl->callback = callback;
    id = CreateSprite(tmpl, 0, 0, 0);
    if (id == MAX_SPRITES)
        return MAX_SPRITES;
    gSprites[id].coordOffsetEnabled = TRUE;
    gSprites[id].invisible = TRUE;
    *size = gfx->size;
    return id;
}

bool8 MP_Follower_DexHasGraphics(u16 dex)
{
    return dex && FindGfx(dex, FALSE) != NULL;
}

// MOéMON GARDEN: a friend's follower on its own, as their game says: the tiles it steps on (queued and replayed
// in order, so its path and facings are the same as on their screen), its pace, its facing and pose once there.
// When their game has it following again, it finishes those steps and falls in behind them here.
static void QueueTile(struct FollowerSlot *s, struct Coords16 t)
{
    if (t.x == s->qLast.x && t.y == s->qLast.y)
        return;
    if (s->qLen == ARRAY_COUNT(s->q))
    {
        memmove(&s->q[0], &s->q[1], (s->qLen - 1) * sizeof(s->q[0]));   // far behind: the oldest step goes
        s->qLen--;
    }
    s->q[s->qLen++] = t;
    s->qLast = t;
}

static void DriveRemoteAuto(struct FollowerSlot *s, u8 obj, u8 id, const struct MpState *st)
{
    struct ObjectEvent *owner;
    s16 ox, oy;
    if (obj >= OBJECT_EVENTS_COUNT || !s->gfx || !s->placed || !SpriteIsOurs(s))
        return;
    owner = &gObjectEvents[obj];
    if (st->lifeDx != MP_LIFE_NO_AUTO && st->lifePx != MP_LIFE_NO_AUTO && MP_GetRemoteMapPos(id, &ox, &oy))
    {
        struct Coords16 t, p;
        t.x = ox + st->lifeDx;
        t.y = oy + st->lifeDy;
        p.x = ox + st->lifePx;
        p.y = oy + st->lifePy;
        if (!s->autoOn)
        {
            if (s->moved < 16)
                return;   // its step first
            s->autoOn = TRUE;
            s->autoStuck = FALSE;
            s->aPrev = s->aCur = s->aTarget = s->cur;
            s->aMoved = 16;
            s->aDir = s->dir;
            s->aTimer = 0;
            s->qLen = 0;
            s->qLast = s->cur;
        }
        if (!s->remoteDriven)
        {
            s->remoteDriven = TRUE;
            s->qLen = 0;
            s->qLast = s->aCur;
        }
        s->autoReturning = FALSE;
        if (abs(t.x - s->qLast.x) + abs(t.y - s->qLast.y) > 6)
        {
            // far from where it was (it was put back with them, a lag spike): it simply goes on from there
            s->qLen = 0;
            s->aPrev = s->aCur = s->aTarget = s->qLast = t;
            s->aMoved = 16;
        }
        else if (!(t.x == s->qLast.x && t.y == s->qLast.y))
        {
            // a new step (the same state again changes nothing): the step before first, if that state was
            // merged into this one
            if (!(p.x == t.x && p.y == t.y) && abs(p.x - s->qLast.x) + abs(p.y - s->qLast.y) == 1)
                QueueTile(s, p);
            QueueTile(s, t);
        }
        s->aSpeed = (st->lifeFace >> 2) & 7;
        if (s->aSpeed == 0)
            s->aSpeed = 1;
        if (s->qLen == 0 && s->aMoved >= 16 && s->aCur.x == t.x && s->aCur.y == t.y)
        {
            s->aDir = st->lifeFace & 3;
            if (s->aPose != st->lifePose)
            {
                s->aPose = st->lifePose;
                s->aTimer = 0;
            }
        }
        else
        {
            s->aPose = LIFE_POSE_NONE;
        }
    }
    else if (s->autoOn)
    {
        // following again in their game: the last steps first, then it follows from right there, as it does
        // in their game (its Trainer's step so far is already counted: UpdateSlot)
        if (s->remoteDriven && (s->qLen || s->aMoved < 16))
            return;
        if (s->remoteDriven)
        {
            s->remoteDriven = FALSE;
            s->prev = s->cur = s->aCur;
            s->dir = s->aDir;
            s->autoOn = FALSE;
            s->aPose = LIFE_POSE_NONE;
            return;
        }
        StartReturn(s, owner);
    }
}

// Where a follower stands now (its own tile while on its own)
bool8 MP_Follower_SlotTile(u8 slot, s16 *x, s16 *y)
{
    struct FollowerSlot *s;
    if (!sInit || slot >= NUM_SLOTS)
        return FALSE;
    s = &sSlots[slot];
    if (!s->gfx || !SpriteIsOurs(s) || !s->placed || gSprites[s->spriteId].invisible)
        return FALSE;
    *x = s->autoOn ? s->aCur.x : s->cur.x;
    *y = s->autoOn ? s->aCur.y : s->cur.y;
    return TRUE;
}

// The local follower on its own (an errand, or on its way back): the tile it's on or stepping onto, the one it
// stepped from, its facing and pace (mp_life_social.c sends them; friends' games replay its steps)
bool8 MP_Follower_AutoState(s16 *x, s16 *y, s16 *px, s16 *py, u8 *dir, u8 *speed)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!sInit || !s->autoOn)
        return FALSE;
    *x = s->aCur.x;
    *y = s->aCur.y;
    *px = s->aPrev.x;
    *py = s->aPrev.y;
    *dir = s->aDir;
    *speed = s->aSpeed;
    return TRUE;
}

// The local follower's destination while on its own (FALSE: following)
bool8 MP_Follower_AutoTargetTile(s16 *x, s16 *y)
{
    struct FollowerSlot *s = &sSlots[0];
    if (!sInit || !s->autoOn || s->autoReturning)
        return FALSE;
    *x = s->aTarget.x;
    *y = s->aTarget.y;
    return TRUE;
}

u8 MP_Follower_SlotPose(u8 slot)
{
    return sInit && slot < NUM_SLOTS && sSlots[slot].autoOn ? sSlots[slot].aPose : LIFE_POSE_NONE;
}

void MP_Follower_DebugDump(void)
{
    u8 i;
    for (i = 0; i < NUM_SLOTS; i++)
    {
        struct FollowerSlot *s = &sSlots[i];
        if (!s->gfx)
            continue;
        printf("[test] follower %d cur %d,%d prev %d,%d moved %d sprite %d inv %d other %d owner %d\n", i, s->cur.x, s->cur.y,
               s->prev.x, s->prev.y, s->moved, s->spriteId, SpriteIsOurs(s) ? gSprites[s->spriteId].invisible : -1,
               GetObjectEventIdByXY(s->cur.x, s->cur.y), s->ownerObj);
    }
}

// Tests (mp pmd): a follower's PMD picture: its Pokémon, colors, animation and facing
void MP_Follower_PmdDebug(u8 slot)
{
    struct FollowerSlot *s;
    if (!sInit || slot >= NUM_SLOTS)
        return;
    s = &sSlots[slot];
    printf("[test] pmd slot %d: dex %d on %d shiny %d anim %d shown %d dir %d frame %d height %d\n", slot, s->dex, s->pmdOn,
           s->shiny, s->pmd.anim, s->pmd.shown, s->pmd.dir, s->pmd.frame, s->pmdOn ? MP_Pmd_Height(&s->pmd) : 0);
    if (slot == 0 && SpriteIsOurs(s))
    {
        // depth: is it drawn in front of its Trainer? (lower subpriority: in front), and where it is beside them
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        struct Sprite *fs = &gSprites[s->spriteId], *os = &gSprites[me->spriteId];
        printf("[test] pmd depth: follower %d,%d trainer %d,%d front %d push %d,%d%s lift %d\n", s->cur.x - MAP_OFFSET,
               s->cur.y - MAP_OFFSET, me->currentCoords.x - MAP_OFFSET, me->currentCoords.y - MAP_OFFSET,
               fs->oam.priority < os->oam.priority || (fs->oam.priority == os->oam.priority && fs->subpriority < os->subpriority),
               s->pushX, s->pushY, s->pushX || s->pushY ? " (back)" : "", s->pmd.big.lift);
    }
}
