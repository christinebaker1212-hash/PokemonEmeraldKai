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
    u32 fx;               // MP_BallFx_Pack
    u8 fxSound;
    struct SpriteFrameImage frames[8];
    struct SpriteTemplate tmpl;
};

static EWRAM_DATA struct FollowerSlot sSlots[NUM_SLOTS];
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

static void DestroySlotSprite(struct FollowerSlot *s)
{
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
    return TRUE;
}

static void SetSlotPokemon(struct FollowerSlot *s, u16 dex, bool8 female, s16 hue)
{
    const struct MpFollowerGfx *gfx = dex ? FindGfx(dex, female) : NULL;
    if (gfx == s->gfx && s->dex == dex && s->hue == hue)
        return;
    if (gfx)
        MPNet_Log("follower: slot %d dex %d hue %d", (int)(s - sSlots), dex, hue);
    if (gfx == s->gfx && s->dex == dex && SpriteIsOurs(s))
    {
        // only its colors changed (renamed): the same sprite with the new palette
        u16 colors[16];
        u32 frameBytes = (u32)s->gfx->size * s->gfx->size / 2;
        u8 slot = IndexOfSpritePaletteTag(TAG_FOLLOWER + (s - sSlots));
        s->hue = hue;
        memcpy(colors, sFollowerData + s->gfx->offset + 8 * frameBytes, sizeof(colors));
        MP_Hue_ShiftPalette(colors, 0, hue);
        if (slot != 0xFF)
            LoadPalette(colors, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);
        return;
    }
    DestroySlotSprite(s);
    s->hue = hue;
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

// Moves one follower with its owner for this frame. hidden: owner surfing etc.
static void UpdateSlot(struct FollowerSlot *s, u8 ownerObj, bool8 hidden)
{
    struct ObjectEvent *owner;
    struct Sprite *os, *fs;
    s16 dxPix, dyPix;
    s32 fx, fy;
    u8 frame;

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
        if (jump > 3)
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
        fvx = s->prev.x * 256 + (s->cur.x - s->prev.x) * frac;
        fvy = s->prev.y * 256 + (s->cur.y - s->prev.y) * frac;
        dxPix = (s16)((fvx - ovx) * 16 / 256);
        dyPix = (s16)((fvy - ovy) * 16 / 256);
        fx = os->x + dxPix;
        fy = os->y + dyPix;
        if (s->gfx->size == 64)
            fy -= 16;
        fs->x = fx;
        fs->y = fy;
        fs->x2 = 0;
        // Hop along when the owner jumps (ledges)
        fs->y2 = (abs(s->cur.x - s->prev.x) + abs(s->cur.y - s->prev.y) >= 2) ? os->y2 : 0;

        // Walking frames while moving, a slow idle bob while standing
        s->animTimer++;
        if (s->moved < total && (s->prev.x != s->cur.x || s->prev.y != s->cur.y))
            frame = s->dir * 2 + ((s->animTimer / WALK_FRAME_TIME) & 1);
        else
            frame = s->dir * 2 + ((s->animTimer / IDLE_FRAME_TIME) & 1);
    }
    if (frame != s->anim)
    {
        StartSpriteAnim(fs, frame);
        s->anim = frame;
    }
    // Same layer as the owner; behind them when level with them
    fs->oam.priority = os->oam.priority;
    SetObjectSubpriorityByElevation(owner->previousElevation, fs, 2);
    fs->invisible = hidden || os->invisible || s->fxHide
        || (s->cur.x == owner->currentCoords.x && s->cur.y == owner->currentCoords.y && s->moved >= 16);
    if (s->recalling && !MP_BallFx_BodyRunning(s->spriteId))
        fs->invisible = TRUE;   // gone back into its ball: stays hidden until it's really gone
    // Someone else stepped onto its tile (a friend hopping off a piggyback
    // ride, or walking up behind): tuck it away instead of covering them
    if (!fs->invisible && s->moved >= 16)
    {
        u8 other = GetObjectEventIdByXY(s->cur.x, s->cur.y);
        if (other != OBJECT_EVENTS_COUNT && other != ownerObj)
            fs->invisible = TRUE;
    }
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
    *x = s->cur.x;
    *y = s->cur.y;
    return TRUE;
}

static bool8 LocalHidden(void)
{
    return (gPlayerAvatar.flags & (PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_UNDERWATER)) != 0 || MP_Piggy_IsRiding();
}

static bool8 RemoteHidden(u8 id)
{
    u8 mode = gMpPlayers[id].state.avatarMode;
    // Holding the BATTLE FLAG: no follower either, so they blend in
    return mode == MP_AVATAR_SURF || mode == MP_AVATAR_UNDERWATER || (gMpPlayers[id].state.flags & (MPF_TRAINER_MODE | MPF_RIDING));
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
void MP_Follower_Update(void)
{
    u8 i;
    bool8 female;
    u16 dex;

    InitIfNeeded();
    if (!MP_IsInOverworld())
        return;

    dex = MP_Follower_LocalDex(&female);
    SetSlotPokemon(&sSlots[0], dex, female, dex ? MP_Follower_LocalHue() : 0);
    UpdateSlot(&sSlots[0], gPlayerAvatar.objectEventId, LocalHidden());
    {
        u8 sound;
        u32 fx = MP_Follower_LocalFx(&sound);
        SlotEffects(&sSlots[0], sLocalToggleSeq, sLeavingState == 2, fx, sound & 7, gPlayerAvatar.objectEventId);
        UpdateLeaving();
    }

    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct FollowerSlot *s = &sSlots[1 + i];
        u8 obj = OBJECT_EVENTS_COUNT;
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState)
        {
            SetSlotPokemon(s, gMpPlayers[i].state.followerDex, (gMpPlayers[i].state.followerFlags & 1) != 0,
                           gMpPlayers[i].state.followerHue);
            obj = MP_GetRemoteObjectEventId(i);
        }
        else
        {
            SetSlotPokemon(s, 0, FALSE, 0);
            s->seqKnown = FALSE;
        }
        UpdateSlot(s, obj, i < MP_MAX_PLAYERS && gMpPlayers[i].present ? RemoteHidden(i) : TRUE);
        if (obj != OBJECT_EVENTS_COUNT)
        {
            u8 fl = gMpPlayers[i].state.followerFlags;
            SlotEffects(s, (fl >> 4) & 3, (fl & 0x40) != 0, gMpPlayers[i].state.followerFx, (fl >> 1) & 7, obj);
        }
    }
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
