// Moemon Co-op: the MOéMON GARDEN's decorations (docs/POKEMON_GARDEN.md 11.3).
//
// Players place earned SECRET BASE decorations: the ones that are figures (dolls, big dolls, cushions:
// DECORPERM_SPRITE, drawn from their object graphics). They come from the placer's own collection and go back
// any time; a decoration out here isn't in the Secret Base or the bedroom at the same time (decoration.c
// counts these). Pokémon use them their own way: dolls and balls for play, cushions for naps; one may grow a
// favorite. No stats, no Friendship: chances for behavior, not a layout to optimize. Friends' decorations
// show while they're here (mp_garden_net.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "mp_farm.h"
#include "mp_life.h"
#include "decoration.h"
#include "decoration_inventory.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "field_message_box.h"
#include "text.h"
#include "main.h"
#include "metatile_behavior.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "constants/decorations.h"
#include "constants/event_objects.h"
#include "constants/songs.h"
#include "mp_garden_map.h"

#define REMOTE_DECOR (MP_MAX_PLAYERS * GARDEN_DECOR_MAX)
#define DECOR_MAGIC  0x4744   // "GD"
#define GX(x) ((x) + MAP_OFFSET)
#define GY(y) ((y) + MAP_OFFSET)

bool8 MP_GardenLife_TileToSprite(s16 tx, s16 ty, s16 *x, s16 *y);
bool8 MP_Garden_StoneShown(void);

struct DecorSprite
{
    u8 decor;       // DECOR_* (0 none)
    s16 x, y;       // map tile with MAP_OFFSET
    u8 spriteId;
    u8 gfx;
};

static struct
{
    struct DecorSprite local[GARDEN_DECOR_MAX + 1];   // + the storm stone
    struct DecorSprite remote[REMOTE_DECOR];          // friends' (player id * 16 + index)
    u16 seq;                                          // our list changed (mp_garden_net.c sends it)
} sD;

static void SpriteCB_Decor(struct Sprite *sprite)
{
}

static bool8 IsSpriteDecor(u8 decor)
{
    return decor != DECOR_NONE && decor < NUM_DECORATIONS && gDecorations[decor].permission == DECORPERM_SPRITE;
}

static bool8 SpriteIsOurs(struct DecorSprite *d, u16 tag)
{
    return d->spriteId < MAX_SPRITES && gSprites[d->spriteId].inUse && gSprites[d->spriteId].callback == SpriteCB_Decor
        && gSprites[d->spriteId].data[6] == DECOR_MAGIC && gSprites[d->spriteId].data[7] == (s16)tag;
}

static void Remove(struct DecorSprite *d, u16 tag)
{
    if (SpriteIsOurs(d, tag))
        DestroySprite(&gSprites[d->spriteId]);
    d->spriteId = MAX_SPRITES;
}

static void DrawOne(struct DecorSprite *d, u16 tag)
{
    s16 x, y;
    struct Sprite *sp;
    bool8 big;
    if (d->decor == DECOR_NONE)
    {
        Remove(d, tag);
        return;
    }
    if (!MP_GardenLife_TileToSprite(d->x, d->y, &x, &y))
        return;
    if (!SpriteIsOurs(d, tag))
    {
        u16 gfx = d->decor == 0xFF ? OBJ_EVENT_GFX_PUSHABLE_BOULDER : gDecorations[d->decor].tiles[0];
        d->spriteId = CreateObjectGraphicsSprite(gfx, SpriteCB_Decor, 0, 0, 0);
        if (d->spriteId >= MAX_SPRITES)
        {
            d->spriteId = MAX_SPRITES;
            return;
        }
        gSprites[d->spriteId].data[6] = DECOR_MAGIC;
        gSprites[d->spriteId].data[7] = tag;
        gSprites[d->spriteId].coordOffsetEnabled = TRUE;
    }
    sp = &gSprites[d->spriteId];
    big = d->decor != 0xFF && gDecorations[d->decor].shape != DECORSHAPE_1x1;
    // the tile's bottom edge, like a person standing there (a big doll covers the tile and the ones above / right)
    sp->x = x + (big ? 8 : 0);
    sp->y = y + (big ? 0 : 8);
    sp->oam.priority = gSprites[gObjectEvents[gPlayerAvatar.objectEventId].spriteId].oam.priority;
    SetObjectSubpriorityByElevation(3, sp, 1);
    // hidden off the screen (a sprite's place wraps round: far away it would show up again in view)
    x = sp->x + gSpriteCoordOffsetX;
    y = sp->y + gSpriteCoordOffsetY;
    sp->invisible = !(x > -32 && x < DISPLAY_WIDTH + 32 && y > -32 && y < DISPLAY_HEIGHT + 48);
}

static bool8 Covers(struct DecorSprite *d, s16 x, s16 y)
{
    if (d->decor == DECOR_NONE)
        return FALSE;
    if (d->x == x && d->y == y)
        return TRUE;
    // a big doll (2x2) stands on its tile and the ones above / to the right
    return d->decor != 0xFF && gDecorations[d->decor].shape != DECORSHAPE_1x1 && x >= d->x && x <= d->x + 1 && y >= d->y - 1 && y <= d->y;
}

bool8 MP_GardenDecor_At(s16 x, s16 y)
{
    u16 i;
    if (!MP_Garden_IsHere())
        return FALSE;
    for (i = 0; i <= GARDEN_DECOR_MAX; i++)
        if (Covers(&sD.local[i], x, y))
            return TRUE;
    for (i = 0; i < REMOTE_DECOR; i++)
        if (Covers(&sD.remote[i], x, y))
            return TRUE;
    return FALSE;
}

// The nearest decoration of a kind (0: to play with: dolls, big dolls; 1: a cushion), and its tile
s8 MP_GardenDecor_Near(s16 x, s16 y, u8 radius, u8 kind, s16 *dx, s16 *dy)
{
    u16 i;
    int best = radius + 1;
    s8 found = -1;
    for (i = 0; i < GARDEN_DECOR_MAX + REMOTE_DECOR; i++)
    {
        struct DecorSprite *d = i < GARDEN_DECOR_MAX ? &sD.local[i] : &sD.remote[i - GARDEN_DECOR_MAX];
        int dist;
        bool8 cushion;
        if (d->decor == DECOR_NONE || d->decor == 0xFF)
            continue;
        cushion = gDecorations[d->decor].category == DECORCAT_CUSHION;
        if (cushion != (kind == 1))
            continue;
        dist = abs(d->x - x) + abs(d->y - y);
        if (dist < best)
        {
            best = dist;
            found = i < 127 ? (s8)i : -1;
            *dx = d->x;
            *dy = d->y;
        }
    }
    return found;
}

static u8 DecorTypeOf(s8 index)
{
    if (index < 0)
        return DECOR_NONE;
    if (index < GARDEN_DECOR_MAX)
        return sD.local[index].decor;
    if (index - GARDEN_DECOR_MAX < REMOTE_DECOR)
        return sD.remote[index - GARDEN_DECOR_MAX].decor;
    return DECOR_NONE;
}

// It used one: a favorite grows from playing with the same one again and again (docs 11.3)
void MP_GardenDecor_Used(s8 index, struct MpLifeRec *rec)
{
    u8 type = DecorTypeOf(index), count;
    if (type == DECOR_NONE || type == 0xFF || rec == NULL)
        return;
    count = rec->favDecor >> 8;
    if ((rec->favDecor & 0xFF) == type)
    {
        if (count < 255)
            count++;
        if (count == 5)
            MPNet_Log("garden: a favorite decoration (%u)", type);
    }
    else if (count <= 1)
    {
        rec->favDecor = type;
        count = 1;
    }
    else
    {
        count--;
    }
    rec->favDecor = (rec->favDecor & 0xFF) | (count << 8);
}

bool8 MP_GardenDecor_IsFavorite(s8 index, struct MpLifeRec *rec)
{
    u8 type = DecorTypeOf(index);
    return rec && type != DECOR_NONE && (rec->favDecor & 0xFF) == type && (rec->favDecor >> 8) >= 5;
}

void MP_GardenDecor_Update(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u16 i;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        struct DecorSprite *d = &sD.local[i];
        u8 want = IsSpriteDecor(gs->decor[i].decor) ? gs->decor[i].decor : DECOR_NONE;
        if (d->decor != want || d->x != GX(gs->decor[i].x) || d->y != GY(gs->decor[i].y))
        {
            Remove(d, i);
            d->decor = want;
            d->x = GX(gs->decor[i].x);
            d->y = GY(gs->decor[i].y);
        }
        DrawOne(d, i);
    }
    // the storm stone (a growth mark)
    {
        struct DecorSprite *s = &sD.local[GARDEN_DECOR_MAX];
        u8 want = MP_Garden_StoneShown() ? 0xFF : DECOR_NONE;
        if (s->decor != want)
        {
            Remove(s, GARDEN_DECOR_MAX);
            s->decor = want;
            s->x = GX(GARDEN_STONE_X);
            s->y = GY(GARDEN_STONE_Y);
        }
        DrawOne(s, GARDEN_DECOR_MAX);
    }
    for (i = 0; i < REMOTE_DECOR; i++)
    {
        u8 owner = i / GARDEN_DECOR_MAX;
        if (sD.remote[i].decor != DECOR_NONE && !(gMpPlayers[owner].present && gMpPlayers[owner].hasState
             && gMpPlayers[owner].state.mapGroup == gSaveBlock1Ptr->location.mapGroup && gMpPlayers[owner].state.mapNum == gSaveBlock1Ptr->location.mapNum))
            sD.remote[i].decor = DECOR_NONE;   // they left: theirs go with them
        DrawOne(&sD.remote[i], 100 + i);
    }
}

// While decorating (the player's sprite is the decorating cursor's: nothing can be placed from it), a decoration
// put away loses its sprite at once; the rest stay where they are
void MP_GardenDecor_Prune(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u16 i;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
        if (sD.local[i].decor != DECOR_NONE && sD.local[i].decor != gs->decor[i].decor)
        {
            Remove(&sD.local[i], i);
            sD.local[i].decor = DECOR_NONE;
        }
}

// mp_garden_net.c: a friend's list (u8 decor, x, y each)
void MP_GardenDecor_SetRemote(u8 owner, const u8 *list, u8 count)
{
    u8 i;
    if (owner >= MP_MAX_PLAYERS)
        return;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        struct DecorSprite *d = &sD.remote[owner * GARDEN_DECOR_MAX + i];
        u8 want = i < count && IsSpriteDecor(list[i * 3]) ? list[i * 3] : DECOR_NONE;
        s16 x = i < count ? GX(list[i * 3 + 1]) : 0, y = i < count ? GY(list[i * 3 + 2]) : 0;
        if (d->decor != want || d->x != x || d->y != y)
        {
            if (want != DECOR_NONE)
                MPNet_Log("garden: player %d's decoration %u at %d,%d", owner + 1, want, x - MAP_OFFSET, y - MAP_OFFSET);
            Remove(d, 100 + owner * GARDEN_DECOR_MAX + i);
            d->decor = want;
            d->x = x;
            d->y = y;
        }
    }
}

u8 MP_GardenDecor_PackLocal(u8 *out)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i, n = 0;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        if (!IsSpriteDecor(gs->decor[i].decor))
            continue;
        out[n * 3] = gs->decor[i].decor;
        out[n * 3 + 1] = gs->decor[i].x;
        out[n * 3 + 2] = gs->decor[i].y;
        n++;
    }
    return n;
}

u16 MP_GardenDecor_Seq(void)
{
    return sD.seq;
}

// ---------------------------------------------------------------------------
// DECORATE (the GARDEN BOARD): the SECRET BASE's decorating does it (decoration.c, its MOéMON GARDEN mode: the
// same menus, the decoration carried over the map by the decorating cursor, "Place it here?"); these keep the
// Garden's own list (a big doll's place is its bottom left tile, as the Secret Base's)

// Open ground of the meadow (map coords with MAP_OFFSET): not a tree, the pond, the escalator's landing
static bool8 GroundFree(s16 x, s16 y)
{
    s16 mx = x - MAP_OFFSET, my = y - MAP_OFFSET;
    return mx >= GARDEN_X0 && mx <= GARDEN_X1 && my >= GARDEN_Y0 && my <= GARDEN_Y1
        && !(mx <= GARDEN_PAD_X1 && my >= GARDEN_PAD_Y0 && my <= GARDEN_PAD_Y1)
        && MapGridGetCollisionAt(x, y) == 0 && !MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y))
        && !MP_Farm_IsFarmGround(mx, my);   // (the BERRY farm and the ways to its plots, open or not yet)
}

// On load (and when the Garden grows): a decoration whose place isn't open ground any more (a save from an older
// Garden, a grove grown there) goes back to the collection
void MP_GardenDecor_CheckPlaces(void)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i;
    if (!(gs->flags & MP_GARDEN_HABITATS))
    {
        // a save from before 1.17: its decorations stood in the old, smaller Garden, so their places mean nothing
        // in this one (they could stand anywhere, even across a way through): back to the collection, once
        for (i = 0; i < GARDEN_DECOR_MAX; i++)
            if (gs->decor[i].decor != DECOR_NONE)
            {
                MPNet_Log("garden: decoration %u put away (the Garden has changed)", gs->decor[i].decor);
                gs->decor[i].decor = DECOR_NONE;
                sD.seq++;
            }
        gs->flags |= MP_GARDEN_HABITATS;
    }
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        u8 decor = gs->decor[i].decor;
        s16 x = GX(gs->decor[i].x), y = GY(gs->decor[i].y);
        if (decor == DECOR_NONE)
            continue;
        if (GroundFree(x, y) && (gDecorations[decor].shape == DECORSHAPE_1x1
             || (MapGridGetCollisionAt(x + 1, y) == 0 && MapGridGetCollisionAt(x, y - 1) == 0 && MapGridGetCollisionAt(x + 1, y - 1) == 0)))
            continue;
        gs->decor[i].decor = DECOR_NONE;
        sD.seq++;
        MPNet_Log("garden: decoration %u put away (its place is gone)", decor);
    }
}

// The tiles a decoration covers, its place being its bottom left one: 1x1, or 2x2 for a big doll
static u8 DecorSize(u8 decor)
{
    return gDecorations[decor].shape == DECORSHAPE_1x1 ? 1 : 2;
}

// Our decoration covering this tile (map coords with MAP_OFFSET), or -1
s8 MP_GardenDecor_OursAt(s16 x, s16 y)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
    {
        u8 decor = gs->decor[i].decor, n;
        s16 dx, dy;
        if (!IsSpriteDecor(decor))
            continue;
        n = DecorSize(decor);
        dx = x - GX(gs->decor[i].x);
        dy = GY(gs->decor[i].y) - y;
        if (dx >= 0 && dx < n && dy >= 0 && dy < n)
            return i;
    }
    return -1;
}

// Can it stand with its bottom left at x, y: its place open ground, the rest of a big doll clear, nobody there, no other
// decoration there
bool8 MP_GardenDecor_CanPlaceAt(u8 decor, s16 x, s16 y)
{
    u8 n, i, j;
    if (!IsSpriteDecor(decor) || !GroundFree(x, y))
        return FALSE;
    n = DecorSize(decor);
    for (j = 0; j < n; j++)
        for (i = 0; i < n; i++)
        {
            s16 tx = x + i, ty = y - j;
            if (MapGridGetCollisionAt(tx, ty) || MP_Garden_ActorAt(tx, ty) || GetObjectEventIdByXY(tx, ty) != OBJECT_EVENTS_COUNT
             || MP_GardenDecor_At(tx, ty)   // (ours, friends', the storm stone)
             || MP_Farm_IsFarmGround(tx - MAP_OFFSET, ty - MAP_OFFSET))
                return FALSE;
        }
    return TRUE;
}

bool8 MP_GardenDecor_PlaceAt(u8 decor, s16 x, s16 y)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    u8 i;
    if (!MP_GardenDecor_CanPlaceAt(decor, x, y))
        return FALSE;
    for (i = 0; i < GARDEN_DECOR_MAX; i++)
        if (gs->decor[i].decor == DECOR_NONE)
        {
            gs->decor[i].decor = decor;
            gs->decor[i].x = x - MAP_OFFSET;
            gs->decor[i].y = y - MAP_OFFSET;
            sD.seq++;
            MPNet_Log("garden: placed decoration %u at %d,%d", decor, x - MAP_OFFSET, y - MAP_OFFSET);
            return TRUE;
        }
    return FALSE;
}

void MP_GardenDecor_PutAway(u8 i)
{
    struct MpGardenSave *gs = MP_Life_GardenSave();
    if (i >= GARDEN_DECOR_MAX || gs->decor[i].decor == DECOR_NONE)
        return;
    MPNet_Log("garden: put away decoration %u", gs->decor[i].decor);
    gs->decor[i].decor = DECOR_NONE;
    sD.seq++;
}

// (the harness) in front of the player
bool8 MP_GardenDecor_Place(u8 decor)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = me->currentCoords.x, y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    return MP_GardenDecor_PlaceAt(decor, x, y);
}

void MP_GardenDecor_Debug(const char *args)
{
    char cmd[32] = "";
    int a = 0;
    sscanf(args, "%31s %d", cmd, &a);
    if (strcmp(cmd, "give") == 0)
    {
        // give DECOR: one more in the collection
        DecorationAdd(a);
        printf("[test] garden decor give %d\n", a);
    }
    else if (strcmp(cmd, "place") == 0)
    {
        printf("[test] garden decor place %d: %d\n", a, MP_GardenDecor_Place(a));
    }
    else if (strcmp(cmd, "list") == 0)
    {
        u8 i;
        for (i = 0; i < GARDEN_DECOR_MAX; i++)
            if (MP_Life_GardenSave()->decor[i].decor)
                printf("[test] garden decor %u: %u at %u,%u\n", i, MP_Life_GardenSave()->decor[i].decor,
                       MP_Life_GardenSave()->decor[i].x, MP_Life_GardenSave()->decor[i].y);
    }
}
