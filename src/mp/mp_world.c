// Moemon Co-op: the world friends share on a map.
//
// Field moves that change the map change it for everyone on that map: a tree one of
// you CUT or a rock one of you smashed is gone for all, a STRENGTH boulder moves for
// all, CUT grass is cut for all, and ashy grass on Route 113 a friend walked through is
// clear for everyone (the soot is theirs). As in the game itself, the changes
// last until the map is loaded again: everyone on the map keeps the list, and a friend
// who walks in while someone is still there is sent it (WS_LIST). When the last of you
// leaves, the map is as new.
//
// Dark caves: each friend on the map carries their own light (their flash level, sent
// in their state), drawn by the renderer as extra "inside window 0" areas next to the
// game's own circle around you (MP_Light_IsLit). A friend who used FLASH lights up the
// cave around them for everyone; your own light stays yours.
#include <string.h>
#include "global.h"
#include "mp.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "sound.h"
#include "sprite.h"
#include "constants/event_object_movement.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/vars.h"

enum {
    WS_CHANGE = MPR_WORLD_FIRST,   // -> all: u8 mapGroup, mapNum, count, count * change (as it happens)
    WS_LIST,                       // -> a friend who walked in: the same, every change so far (no animations)
};

enum {
    WC_OBJ_GONE = 1,   // localId (anim: 1 cut tree, 2 smashed rock)
    WC_TILE,           // x, y, tile (anim: 1 cut grass, 2 ash)
    WC_BOULDER,        // localId moved to x, y, pushed in dir
};

#define CHANGE_BYTES 10
#define MAX_CHANGES 192
#define CHANGES_PER_MSG 48

enum { NOT_APPLIED, APPLIED, ANIMATING };

struct WorldChange
{
    u8 type;
    u8 anim;
    u8 localId;
    u8 dir;
    s16 x, y;
    u16 tile;
    u8 state;     // NOT_APPLIED / APPLIED / ANIMATING
    bool8 live;   // it just happened (animate it), or it came with a list
};

static struct
{
    u8 mapGroup, mapNum;
    bool8 valid;
    u16 count;
    struct WorldChange c[MAX_CHANGES];
} sWorld;

static bool8 sFriendHere[MP_MAX_PLAYERS];

static bool8 OnMap(u8 mapGroup, u8 mapNum)
{
    return gSaveBlock1Ptr->location.mapGroup == mapGroup && gSaveBlock1Ptr->location.mapNum == mapNum;
}

// The list belongs to the map we are on: a new map starts a new one
static void CheckMap(void)
{
    if (!sWorld.valid || !OnMap(sWorld.mapGroup, sWorld.mapNum))
    {
        sWorld.valid = TRUE;
        sWorld.mapGroup = gSaveBlock1Ptr->location.mapGroup;
        sWorld.mapNum = gSaveBlock1Ptr->location.mapNum;
        sWorld.count = 0;
    }
}

// Adds (or updates) a change; returns it, or NULL when it adds nothing new
static struct WorldChange *Store(const struct WorldChange *n)
{
    u16 i;
    for (i = 0; i < sWorld.count; i++)
    {
        struct WorldChange *c = &sWorld.c[i];
        if (c->type != n->type)
            continue;
        if (n->type == WC_OBJ_GONE && c->localId == n->localId)
            return NULL;
        if (n->type == WC_TILE && c->x == n->x && c->y == n->y)
        {
            if (c->tile == n->tile)
                return NULL;
            *c = *n;
            return c;
        }
        if (n->type == WC_BOULDER && c->localId == n->localId)
        {
            if (c->x == n->x && c->y == n->y)
                return NULL;
            *c = *n;
            return c;
        }
    }
    if (sWorld.count >= MAX_CHANGES)
        return NULL;
    sWorld.c[sWorld.count] = *n;
    return &sWorld.c[sWorld.count++];
}

static void PutChange(u8 *p, const struct WorldChange *c)
{
    p[0] = c->type;
    p[1] = c->anim;
    p[2] = c->localId;
    p[3] = c->dir;
    p[4] = c->x & 0xFF; p[5] = (u16)c->x >> 8;
    p[6] = c->y & 0xFF; p[7] = (u16)c->y >> 8;
    p[8] = c->tile & 0xFF; p[9] = c->tile >> 8;
}

static void GetChange(const u8 *p, struct WorldChange *c)
{
    c->type = p[0];
    c->anim = p[1];
    c->localId = p[2];
    c->dir = p[3];
    c->x = (s16)(p[4] | (p[5] << 8));
    c->y = (s16)(p[6] | (p[7] << 8));
    c->tile = p[8] | (p[9] << 8);
}

// Sends changes [first, first + n) to target (0xFF: everyone)
static void Send(u8 kind, u8 target, u16 first, u16 n)
{
    static u8 buf[4 + CHANGES_PER_MSG * CHANGE_BYTES];
    while (n > 0)
    {
        u16 k = n > CHANGES_PER_MSG ? CHANGES_PER_MSG : n, i;
        buf[0] = kind;
        buf[1] = sWorld.mapGroup;
        buf[2] = sWorld.mapNum;
        buf[3] = k;
        for (i = 0; i < k; i++)
            PutChange(buf + 4 + i * CHANGE_BYTES, &sWorld.c[first + i]);
        MP_SendRelay(target, buf, 4 + k * CHANGE_BYTES);
        first += k;
        n -= k;
    }
}

// Something we did changed the map: keep it and tell everyone
static void Local(const struct WorldChange *n)
{
    struct WorldChange *c;
    if (!MP_IsConnected())
        return;
    CheckMap();
    if ((c = Store(n)) == NULL)
        return;
    c->state = APPLIED;
    c->live = FALSE;
    Send(WS_CHANGE, 0xFF, c - sWorld.c, 1);
}

// ---------------------------------------------------------------------------
// What the game tells us (hooks in the field code)

// Specials, right after the scripts' removeobject (field_move_scripts.inc)
void MP_World_CutTreeGone(void)
{
    struct WorldChange c = {WC_OBJ_GONE, 1, gSpecialVar_LastTalked};
    MPNet_Log("world: tree %d cut", gSpecialVar_LastTalked);
    Local(&c);
}

void MP_World_RockGone(void)
{
    struct WorldChange c = {WC_OBJ_GONE, 2, gSpecialVar_LastTalked};
    Local(&c);
}

// field_player_avatar.c TryPushBoulder: the boulder goes to x, y
void MP_World_BoulderPushed(u8 objectEventId, u8 direction, s16 x, s16 y)
{
    struct WorldChange c = {WC_BOULDER, 0, gObjectEvents[objectEventId].localId, direction, x, y};
    Local(&c);
}

// fldeff_cut.c FldEff_CutGrass: the tiles around the cut, before and after
#define SNAP_R 5
#define SNAP_W (SNAP_R * 2 + 1)
static u16 sSnap[SNAP_W][SNAP_W];
static s16 sSnapX, sSnapY;

void MP_World_CutGrassBegin(s16 x, s16 y)
{
    s16 i, j;
    sSnapX = x;
    sSnapY = y;
    for (j = 0; j < SNAP_W; j++)
        for (i = 0; i < SNAP_W; i++)
            sSnap[j][i] = MapGridGetMetatileIdAt(x - SNAP_R + i, y - SNAP_R + j);
}

void MP_World_CutGrassEnd(void)
{
    s16 i, j;
    for (j = 0; j < SNAP_W; j++)
    {
        for (i = 0; i < SNAP_W; i++)
        {
            s16 x = sSnapX - SNAP_R + i, y = sSnapY - SNAP_R + j;
            u16 now = MapGridGetMetatileIdAt(x, y);
            if (now != sSnap[j][i])
            {
                struct WorldChange c = {WC_TILE, 1, 0, 0, x, y, now};
                Local(&c);
            }
        }
    }
}

// field_tasks.c AshGrassPerStepCallback: we walked through ashy grass (it becomes tile)
void MP_World_AshGathered(s16 x, s16 y, u16 tile)
{
    struct WorldChange c = {WC_TILE, 2, 0, 0, x, y, tile};
    Local(&c);
}

// ---------------------------------------------------------------------------
// Friends' changes

static void SetTemplateFlag(u8 localId)
{
    u8 i;
    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        const struct ObjectEventTemplate *t = &gSaveBlock1Ptr->objectEventTemplates[i];
        if (t->localId == localId && t->flagId != 0)
        {
            FlagSet(t->flagId);
            return;
        }
    }
}

static bool8 FindObject(u8 localId, u8 *objId)
{
    return !TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, objId);
}

static void Apply(struct WorldChange *c)
{
    u8 objId;
    switch (c->type)
    {
    case WC_OBJ_GONE:
        SetTemplateFlag(c->localId);   // stays gone when it scrolls back into view
        if (!FindObject(c->localId, &objId))
        {
            c->state = APPLIED;
            break;
        }
        if (c->live && c->anim && !ObjectEventIsMovementOverridden(&gObjectEvents[objId]))
        {
            ObjectEventClearHeldMovementIfActive(&gObjectEvents[objId]);
            ObjectEventSetHeldMovement(&gObjectEvents[objId], c->anim == 1 ? MOVEMENT_ACTION_CUT_TREE : MOVEMENT_ACTION_ROCK_SMASH_BREAK);
            PlaySE(c->anim == 1 ? SE_M_CUT : SE_M_ROCK_THROW);
            c->state = ANIMATING;
        }
        else
        {
            RemoveObjectEventByLocalIdAndMap(c->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            c->state = APPLIED;
        }
        break;
    case WC_TILE:
        if (MapGridGetMetatileIdAt(c->x, c->y) != c->tile)
        {
            if (c->live && c->anim == 2)
            {
                // the ash puff (the soot went in the SOOT SACK of the friend who walked through)
                StartAshFieldEffect(c->x, c->y, c->tile, 4);
            }
            else
            {
                MapGridSetMetatileIdAt(c->x, c->y, c->tile);
                CurrentMapDrawMetatileAt(c->x, c->y);
            }
        }
        c->state = APPLIED;
        break;
    case WC_BOULDER:
    {
        struct ObjectEvent *obj;
        s16 fromX = c->x, fromY = c->y;
        if (!FindObject(c->localId, &objId))
        {
            c->state = APPLIED;
            break;
        }
        obj = &gObjectEvents[objId];
        if (obj->currentCoords.x == c->x && obj->currentCoords.y == c->y)
        {
            c->state = APPLIED;
            break;
        }
        MoveCoords(GetOppositeDirection(c->dir), &fromX, &fromY);
        if (c->live && obj->currentCoords.x == fromX && obj->currentCoords.y == fromY
         && !ObjectEventIsMovementOverridden(obj))
        {
            ObjectEventClearHeldMovementIfActive(obj);
            ObjectEventSetHeldMovement(obj, GetWalkSlowMovementAction(c->dir));
            gFieldEffectArguments[0] = obj->currentCoords.x;
            gFieldEffectArguments[1] = obj->currentCoords.y;
            gFieldEffectArguments[2] = obj->previousElevation;
            gFieldEffectArguments[3] = gSprites[obj->spriteId].oam.priority;
            FieldEffectStart(FLDEFF_DUST);
            PlaySE(SE_M_STRENGTH);
            c->state = ANIMATING;
        }
        else
        {
            MoveObjectEventToMapCoords(obj, c->x, c->y);
            c->state = APPLIED;
        }
        break;
    }
    }
}

// A cut tree / smashed rock / pushed boulder finishing its animation
static void Animate(struct WorldChange *c)
{
    u8 objId;
    if (!FindObject(c->localId, &objId))
    {
        c->state = APPLIED;
        return;
    }
    if (ObjectEventClearHeldMovementIfFinished(&gObjectEvents[objId]) == 0)
        return;   // still going
    if (c->type == WC_OBJ_GONE)
        RemoveObjectEventByLocalIdAndMap(c->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
    c->state = APPLIED;
}

void MP_World_OnRelay(u8 from, const u8 *data, u16 len)
{
    u16 n, i;
    if (len < 4 || from >= MP_MAX_PLAYERS)
        return;
    n = data[3];
    if (len < 4 + n * CHANGE_BYTES)
        return;
    CheckMap();
    if (!OnMap(data[1], data[2]))
        return;   // a map we aren't on (any more)
    for (i = 0; i < n; i++)
    {
        struct WorldChange c, *s;
        GetChange(data + 4 + i * CHANGE_BYTES, &c);
        if (c.type < WC_OBJ_GONE || c.type > WC_BOULDER)
            continue;
        if ((s = Store(&c)) == NULL)
            continue;
        s->state = NOT_APPLIED;
        s->live = data[0] == WS_CHANGE;
    }
    MPNet_Log("world: %d change(s) from player %d (%s)", n, from + 1, data[0] == WS_CHANGE ? "live" : "list");
}

// Every frame
void MP_World_Update(void)
{
    u16 i;
    u8 id;
    bool8 overworld;

    if (!MP_IsConnected())
    {
        sWorld.valid = FALSE;
        memset(sFriendHere, 0, sizeof(sFriendHere));
        return;
    }
    overworld = MP_IsInOverworld();
    if (overworld)
        CheckMap();

    // A friend who just walked onto our map gets what has changed on it so far
    for (id = 0; id < MP_MAX_PLAYERS; id++)
    {
        struct MpPlayer *p = &gMpPlayers[id];
        bool8 here = id != gMpLocalId && p->present && p->hasState && (p->state.flags & MPF_IN_OVERWORLD)
                  && overworld && OnMap(p->state.mapGroup, p->state.mapNum);
        if (here && !sFriendHere[id] && sWorld.valid && sWorld.count > 0 && OnMap(sWorld.mapGroup, sWorld.mapNum))
        {
            Send(WS_LIST, id, 0, sWorld.count);
            MPNet_Log("world: sent %d change(s) to player %d", sWorld.count, id + 1);
        }
        sFriendHere[id] = here;
    }

    if (!overworld || !sWorld.valid || !OnMap(sWorld.mapGroup, sWorld.mapNum))
        return;
    for (i = 0; i < sWorld.count; i++)
    {
        struct WorldChange *c = &sWorld.c[i];
        if (c->state == ANIMATING)
            Animate(c);
        else if (c->state == NOT_APPLIED && MP_IsLocalFree())
            Apply(c);
    }
}

// ---------------------------------------------------------------------------
// Light in dark caves

static const u16 sLevelToRadius[] = {200, 72, 64, 56, 48, 40, 32, 24, 0};   // field_screen_effect.c

struct Light
{
    s16 cx, cy;
    u16 r;
};
static struct Light sLights[MP_MAX_PLAYERS];
static u8 sLightCount;
static u16 sShownRadius[MP_MAX_PLAYERS];
static s16 sSpanL[DISPLAY_HEIGHT][MP_MAX_PLAYERS], sSpanR[DISPLAY_HEIGHT][MP_MAX_PLAYERS];
static u8 sSpanCount[DISPLAY_HEIGHT];
bool8 gMpLightActive;
u8 gMpDarkUiPal = 0xFF;   // in a dark cave: the co-op UI palette's slot (toasts, bubbles...) shown in the dark

static u16 ISqrt(u32 v)
{
    u32 r = 0, b = 1u << 30;
    while (b > v)
        b >>= 2;
    while (b)
    {
        if (v >= r + b)
        {
            v -= r + b;
            r = (r >> 1) + b;
        }
        else
        {
            r >>= 1;
        }
        b >>= 2;
    }
    return r;
}

// Before the frame is drawn (sdl2.c VDraw): where friends' lights are on screen
void MP_World_PrepareLights(void)
{
    bool8 MP_IsBattleTransitionRunning(void);
    struct Sprite *me;
    u8 id, i;
    s16 y;

    gMpLightActive = FALSE;
    gMpDarkUiPal = 0xFF;
    sLightCount = 0;
    if (!MP_IsInOverworld() || GetFlashLevel() == 0 || InBattlePyramid_()
     || MP_IsBattleTransitionRunning() || !(REG_DISPCNT & DISPCNT_WIN0_ON)
     || gPlayerAvatar.spriteId >= MAX_SPRITES)
    {
        memset(sShownRadius, 0, sizeof(sShownRadius));
        return;
    }
    // notes, speech bubbles and the like stay readable in the dark (the renderers let their
    // pixels through the window; the cave behind them stays dark)
    gMpDarkUiPal = IndexOfSpritePaletteTag(MP_TAG_UI_PAL);
    if (!MP_IsConnected())
    {
        memset(sShownRadius, 0, sizeof(sShownRadius));
        return;
    }
    me = &gSprites[gPlayerAvatar.spriteId];
    for (id = 0; id < MP_MAX_PLAYERS; id++)
    {
        struct MpPlayer *p = &gMpPlayers[id];
        u8 objId = MP_GetRemoteObjectEventId(id);
        struct Sprite *s;
        u16 want;
        if (id == gMpLocalId || !p->present || !p->hasState || p->state.light == 0 || p->state.light >= ARRAY_COUNT(sLevelToRadius)
         || !OnMap(p->state.mapGroup, p->state.mapNum) || objId >= OBJECT_EVENTS_COUNT || !gObjectEvents[objId].active
         || gObjectEvents[objId].invisible || gObjectEvents[objId].spriteId >= MAX_SPRITES)
        {
            sShownRadius[id] = 0;
            continue;
        }
        // grows and shrinks like the game's own FLASH circle
        want = sLevelToRadius[p->state.light];
        if (sShownRadius[id] < want)
            sShownRadius[id] = sShownRadius[id] + 2 > want ? want : sShownRadius[id] + 2;
        else if (sShownRadius[id] > want)
            sShownRadius[id] = sShownRadius[id] < want + 2 ? want : sShownRadius[id] - 2;
        if (sShownRadius[id] == 0)
            continue;
        s = &gSprites[gObjectEvents[objId].spriteId];
        // the game's circle is around the screen's middle (us): theirs is where they stand
        sLights[sLightCount].cx = DISPLAY_WIDTH / 2 + (s->x + s->x2) - (me->x + me->x2);
        sLights[sLightCount].cy = DISPLAY_HEIGHT / 2 + (s->y + s->y2) - (me->y + me->y2);
        sLights[sLightCount].r = sShownRadius[id];
        sLightCount++;
    }
    if (sLightCount == 0)
        return;
    for (y = 0; y < DISPLAY_HEIGHT; y++)
    {
        sSpanCount[y] = 0;
        for (i = 0; i < sLightCount; i++)
        {
            s32 dy = y - sLights[i].cy, r = sLights[i].r, w;
            if (dy < -r || dy > r)
                continue;
            w = ISqrt(r * r - dy * dy);
            sSpanL[y][sSpanCount[y]] = sLights[i].cx - w;
            sSpanR[y][sSpanCount[y]] = sLights[i].cx + w;
            sSpanCount[y]++;
        }
    }
    gMpLightActive = TRUE;
}

// The renderers' window 0 test: is (x, y) inside a friend's light?
bool8 MP_Light_IsLit(int x, int y)
{
    u8 i;
    if (y < 0 || y >= DISPLAY_HEIGHT)
        return FALSE;
    for (i = 0; i < sSpanCount[y]; i++)
        if (x >= sSpanL[y][i] && x < sSpanR[y][i])
            return TRUE;
    return FALSE;
}

// Test builds: the lights on screen (mp lights)
void MP_World_DebugLights(void)
{
    u8 i;
    MPNet_Log("lights: %d friend light(s)%s", sLightCount, gMpLightActive ? "" : " (off)");
    for (i = 0; i < sLightCount; i++)
        MPNet_Log("light %d: center %d,%d radius %d", i, sLights[i].cx, sLights[i].cy, sLights[i].r);
}
