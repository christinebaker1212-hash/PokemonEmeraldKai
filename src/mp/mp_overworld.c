// Moemon Co-op: remote players in the overworld (object events, movement replay, nametags)
#include "global.h"
#include "mp.h"
#include "main.h"
#include "overworld.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "field_player_avatar.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "palette.h"
#include "sprite.h"
#include "text.h"
#include "string_util.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/characters.h"

u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
void MP_RemoveObjectEvent(struct ObjectEvent *objectEvent);
u8 MP_FindSpritePaletteTag(u16 tag, u8 minSlot);
u8 MP_AllocSpritePalette(u16 tag, u8 minSlot);
void MPNet_Log(const char *fmt, ...);

#define QUEUE_SIZE 24
#define NAMETAG_W 64
#define NAMETAG_H 32
#define NAMETAG_TILES ((NAMETAG_W / 8) * (NAMETAG_H / 8))
#define TAG_NAMETAG_BASE 0x5380
#define TAG_REMOTE_PAL_BASE 0x5390

struct QueuedPos
{
    u8 mapGroup;
    u8 mapNum;
    s16 x;
    s16 y;
    u8 speed;
};

struct RemoteObj
{
    bool8 spawned;
    u8 objId;
    u8 builtMode;
    u32 builtVersion;
    u8 palSlot;
    struct QueuedPos queue[QUEUE_SIZE];
    u8 qHead;
    u8 qCount;
    u8 lastEmote;
    u8 tagSpriteId;
    u8 tagLevel;
    bool8 tagBattle;
    u32 tagVersion;
    u8 blobSpriteId;
    s16 lastX, lastY;   // last position applied (current map frame)
};

static struct RemoteObj sRemote[MP_MAX_PLAYERS];
static u8 sNametagPixels[NAMETAG_W * NAMETAG_H];
static u8 sNametagTiles[NAMETAG_TILES * 32];
static bool8 sInit;

static const struct OamData sOam_Nametag = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
};

static void SpriteCB_Nametag(struct Sprite *sprite);

static void InitIfNeeded(void)
{
    u8 i;
    if (sInit)
        return;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        memset(&sRemote[i], 0, sizeof(sRemote[i]));
        sRemote[i].objId = OBJECT_EVENTS_COUNT;
        sRemote[i].tagSpriteId = MAX_SPRITES;
        sRemote[i].blobSpriteId = MAX_SPRITES;
        sRemote[i].palSlot = 0xFF;
    }
    sInit = TRUE;
}

bool8 MP_IsMpObjectEvent(const struct ObjectEvent *objectEvent)
{
    return objectEvent->localId >= MP_LOCALID_BASE && objectEvent->localId < MP_LOCALID_BASE + MP_MAX_PLAYERS
        && MP_IsMpGraphicsId(objectEvent->graphicsId);
}

u8 MP_GetRemoteObjectEventId(u8 id)
{
    InitIfNeeded();
    if (id >= MP_MAX_PLAYERS || !sRemote[id].spawned)
        return OBJECT_EVENTS_COUNT;
    return sRemote[id].objId;
}

// ---------------------------------------------------------------------------
// Coordinates

// Converts a position on (mapGroup, mapNum) into the current map's coordinate
// frame. Works for the current map and directly connected maps.
static bool8 ConvertToCurrentMap(u8 mapGroup, u8 mapNum, s16 x, s16 y, s16 *outX, s16 *outY)
{
    const struct MapConnections *connections;
    s32 i;

    if (mapGroup == gSaveBlock1Ptr->location.mapGroup && mapNum == gSaveBlock1Ptr->location.mapNum)
    {
        *outX = x;
        *outY = y;
        return TRUE;
    }
    connections = gMapHeader.connections;
    if (connections == NULL)
        return FALSE;
    for (i = 0; i < connections->count; i++)
    {
        const struct MapConnection *c = &connections->connections[i];
        const struct MapHeader *theirs;
        s32 rx = x - MAP_OFFSET, ry = y - MAP_OFFSET;
        s32 offset = (s32)(s16)c->offset;
        if (c->mapGroup != mapGroup || c->mapNum != mapNum)
            continue;
        theirs = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
        switch (c->direction)
        {
        case CONNECTION_NORTH:
            rx += offset;
            ry -= theirs->mapLayout->height;
            break;
        case CONNECTION_SOUTH:
            rx += offset;
            ry += gMapHeader.mapLayout->height;
            break;
        case CONNECTION_WEST:
            rx -= theirs->mapLayout->width;
            ry += offset;
            break;
        case CONNECTION_EAST:
            rx += gMapHeader.mapLayout->width;
            ry += offset;
            break;
        default:
            return FALSE;
        }
        *outX = rx + MAP_OFFSET;
        *outY = ry + MAP_OFFSET;
        return TRUE;
    }
    return FALSE;
}

bool8 MP_GetRemoteMapPos(u8 id, s16 *x, s16 *y)
{
    struct MpPlayer *p = &gMpPlayers[id];
    if (!p->present || !p->hasState)
        return FALSE;
    return ConvertToCurrentMap(p->state.mapGroup, p->state.mapNum, p->state.x, p->state.y, x, y);
}

// ---------------------------------------------------------------------------
// Nametags

static void PutPixel(int x, int y, u8 c)
{
    if (x >= 0 && x < NAMETAG_W && y >= 0 && y < NAMETAG_H)
        sNametagPixels[y * NAMETAG_W + x] = c;
}

static int DrawGlyph(int x, int y, u16 glyph, u8 fg, u8 shadow)
{
    int w = MP_RenderSmallGlyph(glyph, fg, 0, shadow);
    int gx, gy;
    for (gy = 0; gy < 16; gy++)
    {
        const u32 *rows = (gy < 8) ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
        for (gx = 0; gx < w && gx < 16; gx++)
        {
            u32 row = rows[(gx < 8 ? 0 : 8) + (gy % 8)];
            u8 c = (row >> ((gx % 8) * 4)) & 0xF;
            if (c)
                PutPixel(x + gx, y + gy, c);
        }
    }
    return w;
}

static int MeasureGlyph(u16 glyph)
{
    return MP_RenderSmallGlyph(glyph, 1, 0, 2);
}

// Finds palette indices close to white and black for the nametag text
static void PickTextColors(const u16 *pal, u8 *fg, u8 *outline)
{
    int i, bestW = -1, bestB = 1 << 30;
    *fg = 14;
    *outline = 15;
    for (i = 1; i < 16; i++)
    {
        int r = pal[i] & 0x1F, g = (pal[i] >> 5) & 0x1F, b = (pal[i] >> 10) & 0x1F;
        int lum = r * 3 + g * 6 + b;
        if (lum > bestW) { bestW = lum; *fg = i; }
        if (lum < bestB) { bestB = lum; *outline = i; }
    }
}

static bool8 IsRemoteInBattle(u8 id)
{
    return gMpPlayers[id].hasState && (gMpPlayers[id].state.flags & MPF_IN_BATTLE);
}

// Draws one centered, outlined line of small-font text at row y
static void DrawTextLine(const u8 *text, int n, int y, u8 fg, u8 outline)
{
    static const s8 offs[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}};
    int i, o, width = 0, x;
    for (i = 0; i < n; i++)
        width += (text[i] == CHAR_SPACE) ? 3 : MeasureGlyph(text[i]);
    if (width > NAMETAG_W - 2)
        width = NAMETAG_W - 2;
    x = (NAMETAG_W - width) / 2;
    for (o = 0; o < 8; o++)
    {
        int cx = x;
        for (i = 0; i < n; i++)
            cx += (text[i] == CHAR_SPACE) ? 3 : DrawGlyph(cx + offs[o][0], y + offs[o][1], text[i], outline, 0);
    }
    {
        int cx = x;
        for (i = 0; i < n; i++)
            cx += (text[i] == CHAR_SPACE) ? 3 : DrawGlyph(cx, y, text[i], fg, 0);
    }
}

static void BuildNametag(u8 id, const u16 *pal)
{
    struct MpPlayer *p = &gMpPlayers[id];
    u8 text[24];
    int i, n = 0, tx, ty;
    u8 fg, outline;

    for (i = 0; i < MP_NAME_LEN && p->name[i] != EOS; i++)
        text[n++] = p->name[i];
    if (p->hasState && p->state.topLevel > 0)
    {
        u8 lvl = p->state.topLevel;
        text[n++] = CHAR_SPACE;
        text[n++] = CHAR_LV;
        if (lvl >= 100) text[n++] = CHAR_0 + lvl / 100;
        if (lvl >= 10) text[n++] = CHAR_0 + (lvl / 10) % 10;
        text[n++] = CHAR_0 + lvl % 10;
    }

    PickTextColors(pal, &fg, &outline);
    memset(sNametagPixels, 0, sizeof(sNametagPixels));
    if (IsRemoteInBattle(id))
    {
        // Two lines: "IN BATTLE" above the name (the tag is drawn 12px higher)
        u8 battle[16];
        MP_AsciiToGame(battle, "IN BATTLE", 12);
        DrawTextLine(battle, StringLength(battle), 1, fg, outline);
        DrawTextLine(text, n, 13, fg, outline);
    }
    else
        DrawTextLine(text, n, 1, fg, outline);

    // Pack into 4bpp 1D-mapped tiles (8x4 tiles)
    memset(sNametagTiles, 0, sizeof(sNametagTiles));
    for (ty = 0; ty < NAMETAG_H / 8; ty++)
    {
        for (tx = 0; tx < NAMETAG_W / 8; tx++)
        {
            u8 *tile = sNametagTiles + (ty * (NAMETAG_W / 8) + tx) * 32;
            int px, py;
            for (py = 0; py < 8; py++)
                for (px = 0; px < 8; px++)
                {
                    u8 c = sNametagPixels[(ty * 8 + py) * NAMETAG_W + tx * 8 + px];
                    int k = py * 8 + px;
                    tile[k / 2] |= (k & 1) ? (c << 4) : c;
                }
        }
    }
}

static void SpriteCB_Nametag(struct Sprite *sprite)
{
    // Positioned by MP_Overworld_Update
}

static bool8 IsNametagSpriteValid(u8 id)
{
    u8 s = sRemote[id].tagSpriteId;
    return s < MAX_SPRITES && gSprites[s].inUse && gSprites[s].callback == SpriteCB_Nametag && gSprites[s].data[0] == id + 1;
}

static void DestroyNametag(u8 id)
{
    if (IsNametagSpriteValid(id))
        DestroySprite(&gSprites[sRemote[id].tagSpriteId]);
    sRemote[id].tagSpriteId = MAX_SPRITES;
    FreeSpriteTilesByTag(TAG_NAMETAG_BASE + id);
}

static void UpdateNametag(u8 id, struct ObjectEvent *obj, const u16 *pal)
{
    struct RemoteObj *r = &sRemote[id];
    struct MpPlayer *p = &gMpPlayers[id];
    struct Sprite *objSprite = &gSprites[obj->spriteId];
    struct Sprite *tag;
    u16 tileStart = GetSpriteTileStartByTag(TAG_NAMETAG_BASE + id);
    bool8 rebuild = FALSE;

    if (!IsNametagSpriteValid(id) || tileStart == 0xFFFF)
    {
        struct SpriteSheet sheet;
        struct SpriteTemplate tmpl;
        DestroyNametag(id);
        BuildNametag(id, pal);
        sheet.data = sNametagTiles;
        sheet.size = sizeof(sNametagTiles);
        sheet.tag = TAG_NAMETAG_BASE + id;
        if (LoadSpriteSheet(&sheet) == 0)
            return;
        tmpl.tileTag = TAG_NAMETAG_BASE + id;
        tmpl.paletteTag = TAG_NONE;
        tmpl.oam = &sOam_Nametag;
        tmpl.anims = gDummySpriteAnimTable;
        tmpl.images = NULL;
        tmpl.affineAnims = gDummySpriteAffineAnimTable;
        tmpl.callback = SpriteCB_Nametag;
        r->tagSpriteId = CreateSprite(&tmpl, 0, 0, 0);
        if (r->tagSpriteId == MAX_SPRITES)
            return;
        gSprites[r->tagSpriteId].data[0] = id + 1;
        gSprites[r->tagSpriteId].coordOffsetEnabled = TRUE;
        r->tagLevel = p->hasState ? p->state.topLevel : 0;
        r->tagBattle = IsRemoteInBattle(id);
        r->tagVersion = p->appearanceVersion;
        tileStart = GetSpriteTileStartByTag(TAG_NAMETAG_BASE + id);
    }
    else if (r->tagLevel != (p->hasState ? p->state.topLevel : 0) || r->tagVersion != p->appearanceVersion
          || r->tagBattle != IsRemoteInBattle(id))
    {
        rebuild = TRUE;
    }
    if (rebuild && tileStart != 0xFFFF)
    {
        BuildNametag(id, pal);
        memcpy((void *)(OBJ_VRAM0 + tileStart * TILE_SIZE_4BPP), sNametagTiles, sizeof(sNametagTiles));
        r->tagLevel = p->hasState ? p->state.topLevel : 0;
        r->tagBattle = IsRemoteInBattle(id);
        r->tagVersion = p->appearanceVersion;
    }

    tag = &gSprites[r->tagSpriteId];
    tag->x = objSprite->x + objSprite->x2;
    tag->y = objSprite->y + objSprite->y2 + objSprite->centerToCornerVecY + 10 - (r->tagBattle ? 12 : 0);
    tag->oam.paletteNum = r->palSlot;
    // Holding the BATTLE FLAG: no name tag, so they blend in like an NPC trainer
    tag->invisible = objSprite->invisible || MP_Emote_RemoteBubbleShown(id)
                  || (p->hasState && (p->state.flags & MPF_TRAINER_MODE) && !r->tagBattle);
}

// ---------------------------------------------------------------------------
// Spawning

static u8 AvatarModeOf(u8 id)
{
    return gMpPlayers[id].hasState ? gMpPlayers[id].state.avatarMode : MP_AVATAR_FOOT;
}

static void DestroyBlob(u8 id)
{
    struct RemoteObj *r = &sRemote[id];
    if (r->blobSpriteId < MAX_SPRITES && gSprites[r->blobSpriteId].inUse)
        DestroySprite(&gSprites[r->blobSpriteId]);
    r->blobSpriteId = MAX_SPRITES;
}

static void Despawn(u8 id)
{
    struct RemoteObj *r = &sRemote[id];
    if (r->spawned && r->objId < OBJECT_EVENTS_COUNT)
    {
        struct ObjectEvent *obj = &gObjectEvents[r->objId];
        if (obj->active && MP_IsMpObjectEvent(obj) && obj->localId == MP_LOCALID_BASE + id)
            MP_RemoveObjectEvent(obj);
    }
    DestroyBlob(id);
    DestroyNametag(id);
    {
        u8 s = MP_FindSpritePaletteTag(TAG_REMOTE_PAL_BASE + id, OBJ_PALSLOT_COUNT);
        if (s != 0xFF)
            FreeSpritePaletteByTag(TAG_REMOTE_PAL_BASE + id);
    }
    r->spawned = FALSE;
    r->objId = OBJECT_EVENTS_COUNT;
    r->palSlot = 0xFF;
}

static bool8 IsSpawnValid(u8 id)
{
    struct RemoteObj *r = &sRemote[id];
    struct ObjectEvent *obj;
    if (!r->spawned || r->objId >= OBJECT_EVENTS_COUNT)
        return FALSE;
    obj = &gObjectEvents[r->objId];
    return obj->active && obj->localId == MP_LOCALID_BASE + id && obj->graphicsId == OBJ_EVENT_GFX_MP_REMOTE(id);
}

// Makes sure the remote player's palette is loaded into a sprite palette slot.
// Uses the free dynamic sprite palette slots first. When those run out (weather
// and field effects can use some), it borrows the NPC reflection slots, which
// are only needed for NPC reflections on water.
static const u8 sBorrowableSlots[] = {
    PALSLOT_NPC_4_REFLECTION, PALSLOT_NPC_3_REFLECTION, PALSLOT_NPC_2_REFLECTION, PALSLOT_NPC_1_REFLECTION,
};

static bool8 IsSlotUsedByOtherRemote(u8 id, u8 slot)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != id && sRemote[i].spawned && sRemote[i].palSlot == slot)
            return TRUE;
    return FALSE;
}

static void EnsurePalette(u8 id, u16 *palOut)
{
    struct RemoteObj *r = &sRemote[id];
    
    u8 slot = MP_FindSpritePaletteTag(TAG_REMOTE_PAL_BASE + id, OBJ_PALSLOT_COUNT);

    MP_Avatar_GetRemotePalette(id, AvatarModeOf(id), palOut);

    if (slot == 0xFF && r->palSlot < OBJ_PALSLOT_COUNT && r->palSlot != 0 && !IsSlotUsedByOtherRemote(id, r->palSlot))
        slot = r->palSlot; // keep the borrowed slot
    if (slot == 0xFF)
        slot = MP_AllocSpritePalette(TAG_REMOTE_PAL_BASE + id, OBJ_PALSLOT_COUNT);
    if (slot == 0xFF)
    {
        u8 i;
        for (i = 0; i < ARRAY_COUNT(sBorrowableSlots); i++)
        {
            if (!IsSlotUsedByOtherRemote(id, sBorrowableSlots[i]))
            {
                slot = sBorrowableSlots[i];
                break;
            }
        }
    }
    if (slot == 0xFF)
    {
        r->palSlot = 0; // last resort: the local player's palette
        return;
    }
    r->palSlot = slot;
    // (Re)load when our colors aren't there (new slot, map reload, appearance change)
    if (memcmp(&gPlttBufferUnfaded[OBJ_PLTT_ID(slot)], palOut, PLTT_SIZE_4BPP) != 0)
    {
        LoadPalette(palOut, OBJ_PLTT_ID(slot), PLTT_SIZE_4BPP);
        UpdateSpritePaletteWithWeather(slot);
    }
}

static void RefreshGraphics(u8 id, struct ObjectEvent *obj)
{
    struct RemoteObj *r = &sRemote[id];
    u8 mode = AvatarModeOf(id);
    MP_Avatar_BuildRemote(id);
    ObjectEventSetGraphicsId(obj, OBJ_EVENT_GFX_MP_REMOTE(id));
    r->builtVersion = gMpPlayers[id].appearanceVersion;
    r->builtMode = mode;
}

static void Spawn(u8 id, s16 x, s16 y)
{
    struct RemoteObj *r = &sRemote[id];
    struct MpPlayer *p = &gMpPlayers[id];
    u8 objId;
    u16 pal[16];

    MP_Avatar_BuildRemote(id);
    objId = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_MP_REMOTE(id), MOVEMENT_TYPE_NONE,
                                                 MP_LOCALID_BASE + id, x, y, p->state.elevation);
    if (objId >= OBJECT_EVENTS_COUNT)
        return;
    r->spawned = TRUE;
    r->objId = objId;
    MPNet_Log("spawned remote %d as object %d (sprite %d), player object %d (sprite %d) at %d,%d",
              id, objId, gObjectEvents[objId].spriteId, gPlayerAvatar.objectEventId, gPlayerAvatar.spriteId, x, y);
    r->builtVersion = 0xFFFFFFFF;
    r->palSlot = 0xFF;
    r->qCount = 0;
    r->lastX = x;
    r->lastY = y;
    r->lastEmote = p->state.emote;
    gObjectEvents[objId].disableAnim = FALSE;
    SetObjectEventDirection(&gObjectEvents[objId], p->state.facing ? p->state.facing : DIR_SOUTH);
    EnsurePalette(id, pal);
    RefreshGraphics(id, &gObjectEvents[objId]);
}

// ---------------------------------------------------------------------------
// Movement replay

void MP_Overworld_OnRemoteState(u8 id, const struct MpState *old)
{
    struct RemoteObj *r;
    struct MpState *s = &gMpPlayers[id].state;
    InitIfNeeded();
    r = &sRemote[id];
    if (old->mapGroup == s->mapGroup && old->mapNum == s->mapNum && old->x == s->x && old->y == s->y)
        return;
    if (r->qCount == QUEUE_SIZE)
    {
        // Too far behind: drop the oldest entry
        r->qHead = (r->qHead + 1) % QUEUE_SIZE;
        r->qCount--;
    }
    {
        struct QueuedPos *q = &r->queue[(r->qHead + r->qCount) % QUEUE_SIZE];
        q->mapGroup = s->mapGroup;
        q->mapNum = s->mapNum;
        q->x = s->x;
        q->y = s->y;
        q->speed = s->speed;
        r->qCount++;
    }
}

static u8 DirectionFromDelta(s16 dx, s16 dy)
{
    if (dx > 0) return DIR_EAST;
    if (dx < 0) return DIR_WEST;
    if (dy > 0) return DIR_SOUTH;
    return DIR_NORTH;
}

static void StepObject(u8 id, struct ObjectEvent *obj)
{
    struct RemoteObj *r = &sRemote[id];
    struct MpPlayer *p = &gMpPlayers[id];

    if (ObjectEventIsHeldMovementActive(obj) && !ObjectEventClearHeldMovementIfFinished(obj))
        return;

    while (r->qCount > 0)
    {
        struct QueuedPos q = r->queue[r->qHead];
        s16 tx, ty, dx, dy;
        u8 dir, speed;
        r->qHead = (r->qHead + 1) % QUEUE_SIZE;
        r->qCount--;
        if (!ConvertToCurrentMap(q.mapGroup, q.mapNum, q.x, q.y, &tx, &ty))
            continue;
        dx = tx - obj->currentCoords.x;
        dy = ty - obj->currentCoords.y;
        if (dx == 0 && dy == 0)
            continue;
        dir = DirectionFromDelta(dx, dy);
        speed = q.speed;
        if (r->qCount >= 6 && speed < MP_SPEED_FASTER)
            speed++; // catch up when lagging behind
        if ((dx == 0 || dy == 0) && (abs(dx) + abs(dy)) == 1)
        {
            u8 action;
            switch (speed)
            {
            case MP_SPEED_RUN:    action = (AvatarModeOf(id) == MP_AVATAR_FOOT) ? GetPlayerRunMovementAction(dir) : GetWalkFastMovementAction(dir); break;
            case MP_SPEED_FAST:   action = GetWalkFastMovementAction(dir); break;
            case MP_SPEED_FASTER: action = GetWalkFasterMovementAction(dir); break;
            default:              action = GetWalkNormalMovementAction(dir); break;
            }
            ObjectEventSetHeldMovement(obj, action);
        }
        else if ((dx == 0 || dy == 0) && (abs(dx) + abs(dy)) == 2)
        {
            ObjectEventSetHeldMovement(obj, GetJump2MovementAction(dir));
        }
        else
        {
            MoveObjectEventToMapCoords(obj, tx, ty);
        }
        r->lastX = tx;
        r->lastY = ty;
        return;
    }

    // Idle: keep facing and elevation in sync
    if (p->state.facing >= DIR_SOUTH && p->state.facing <= DIR_EAST && obj->facingDirection != p->state.facing)
        ObjectEventSetHeldMovement(obj, GetFaceDirectionMovementAction(p->state.facing));
    if (obj->currentElevation != p->state.elevation)
    {
        obj->currentElevation = p->state.elevation;
        obj->previousElevation = p->state.elevation;
    }
}

static void UpdateBlob(u8 id, struct ObjectEvent *obj)
{
    struct RemoteObj *r = &sRemote[id];
    bool8 surfing = (AvatarModeOf(id) == MP_AVATAR_SURF);
    bool8 blobValid = r->blobSpriteId < MAX_SPRITES && gSprites[r->blobSpriteId].inUse;

    if (surfing && !blobValid)
    {
        gFieldEffectArguments[0] = obj->currentCoords.x;
        gFieldEffectArguments[1] = obj->currentCoords.y;
        gFieldEffectArguments[2] = r->objId;
        r->blobSpriteId = FieldEffectStart(FLDEFF_SURF_BLOB);
        if (r->blobSpriteId < MAX_SPRITES)
            SetSurfBlob_BobState(r->blobSpriteId, BOB_PLAYER_AND_MON);
    }
    else if (!surfing && blobValid)
    {
        DestroyBlob(id);
    }
    if (surfing && r->blobSpriteId < MAX_SPRITES)
        gSprites[r->blobSpriteId].oam.paletteNum = r->palSlot;
}

static void UpdateRemote(u8 id)
{
    struct RemoteObj *r = &sRemote[id];
    struct MpPlayer *p = &gMpPlayers[id];
    struct ObjectEvent *obj;
    s16 x, y;
    u16 pal[16];

    if (!p->present || !p->hasState || !p->hasInfo || !MP_GetRemoteMapPos(id, &x, &y))
    {
        if (r->spawned)
        {
            MPNet_Log("[mp] despawn remote %d (present %d state %d info %d map %d.%d)", id, p->present, p->hasState,
                      p->hasInfo, p->state.mapGroup, p->state.mapNum);
            Despawn(id);
        }
        return;
    }

    if (!IsSpawnValid(id))
    {
        r->spawned = FALSE;
        DestroyNametag(id);
        r->blobSpriteId = MAX_SPRITES;
        Spawn(id, x, y);
        if (!IsSpawnValid(id))
            return;
    }
    obj = &gObjectEvents[r->objId];

    EnsurePalette(id, pal);
    if (r->builtVersion != p->appearanceVersion || r->builtMode != AvatarModeOf(id))
        RefreshGraphics(id, obj);
    gSprites[obj->spriteId].oam.paletteNum = r->palSlot;

    // Resync if our idea of the position drifted (e.g. after a map transition)
    if (r->qCount == 0 && !ObjectEventIsHeldMovementActive(obj)
     && (obj->currentCoords.x != x || obj->currentCoords.y != y))
    {
        s16 dx = x - obj->currentCoords.x, dy = y - obj->currentCoords.y;
        if (abs(dx) + abs(dy) > 2)
            MoveObjectEventToMapCoords(obj, x, y);
        else
            MP_Overworld_OnRemoteState(id, &(struct MpState){0xFF});
    }

    StepObject(id, obj);
    UpdateBlob(id, obj);

    if (p->state.emote != r->lastEmote)
    {
        r->lastEmote = p->state.emote;
        MP_Emote_OnRemoteIcon(id, obj, p->state.emote & 0xF);
    }

    UpdateNametag(id, obj, pal);
}

void MP_Overworld_Update(void)
{
    static u8 sAppliedGender = 0xFF;
    u8 i;
    InitIfNeeded();
    if (!MP_IsInOverworld())
        return;
    MP_Notice_Update();
    if (!MP_Spectate_TryInteract() && !MP_Piggy_TryInteract())
        MP_Follower_TryInteract();
    if (sAppliedGender != gSaveBlock2Ptr->playerGender)
    {
        sAppliedGender = gSaveBlock2Ptr->playerGender;
        MP_Avatar_ApplyLocal();
    }
    // Remove stale remote player objects (e.g. restored from a save file)
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && MP_IsMpObjectEvent(obj))
        {
            u8 id = obj->localId - MP_LOCALID_BASE;
            if (!sRemote[id].spawned || sRemote[id].objId != i)
                MP_RemoveObjectEvent(obj);
        }
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId)
            continue;
        UpdateRemote(i);
    }
}

void MP_Overworld_OnPlayerLeft(u8 id)
{
    InitIfNeeded();
    if (MP_IsInOverworld())
        Despawn(id);
    else
    {
        sRemote[id].spawned = FALSE;
        sRemote[id].tagSpriteId = MAX_SPRITES;
        sRemote[id].blobSpriteId = MAX_SPRITES;
    }
    sRemote[id].qCount = 0;
}

void MP_Overworld_OnAppearanceChanged(u8 id)
{
    // Picked up on the next update through appearanceVersion
}

void MP_Overworld_Reset(void)
{
    u8 i;
    InitIfNeeded();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        MP_Overworld_OnPlayerLeft(i);
}

// Test harness: "mp objs" prints the remote player objects
void MP_Overworld_DebugDump(void)
{
    u8 i;
    InitIfNeeded();
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct RemoteObj *r = &sRemote[i];
        struct ObjectEvent *o;
        printf("[test] slot %d present %d state %d info %d map %d.%d xy %d,%d spawned %d obj %d inOverworld %d local %d\n", i,
               gMpPlayers[i].present, gMpPlayers[i].hasState, gMpPlayers[i].hasInfo, gMpPlayers[i].state.mapGroup,
               gMpPlayers[i].state.mapNum, gMpPlayers[i].state.x, gMpPlayers[i].state.y, r->spawned, r->objId,
               MP_IsInOverworld(), gMpLocalId);
        if (!r->spawned || r->objId >= OBJECT_EVENTS_COUNT)
            continue;
        o = &gObjectEvents[r->objId];
        printf("[test] remote %d obj %d active %d invisible %d coords %d,%d gfx %d sprite %d", i, r->objId, o->active,
               o->invisible, o->currentCoords.x, o->currentCoords.y, o->graphicsId, o->spriteId);
        if (o->spriteId < MAX_SPRITES)
        {
            struct Sprite *s = &gSprites[o->spriteId];
            printf(" inUse %d sinv %d xy %d,%d pal %d prio %d", s->inUse, s->invisible, s->x, s->y, s->oam.paletteNum, s->oam.priority);
        }
        printf("\n");
    }
}
