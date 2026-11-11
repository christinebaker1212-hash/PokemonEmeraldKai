// Moemon Kai: ORRE's rooms' people (docs/ORRE_HANDOFF.md step 5, "People on the maps"). Each room's characters are
// Colosseum's own (src/data/orre_room_people.h from tools/orre/gen_people.py: where they stand, their model's ORRE
// person, their name, their talk script), put on the map as Kai object events (their LOOK sprites: orre_people.c) and
// driven by Colosseum's scripts through orre_vm.c's natives: shown and hidden (peopleSetVisible,
// scriptFloorCharSetDisp; the record's 0x80 at the start), placed, turned, walked (Kai's scripted movement, which
// carries on while the field is locked), and talked to (A on one runs its talk script: Orre_EventScript_Talk).
// A script names a person by its index in the room; (0, 100) is the hero, the player.
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_object_movement.h"
#include "event_data.h"
#include "fieldmap.h"
#include "field_control_avatar.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "o3d.h"
#include "overworld.h"
void MoveCameraAndRedrawMap(int deltaX, int deltaY);   // (field_camera.c)
#include "orre.h"
#include "orre_people.h"
#include "script.h"
#include "script_movement.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"

void MPNet_Log(const char *fmt, ...);
extern const u8 Orre_EventScript_Talk[];
extern const u8 Orre_EventScript_Check[];
extern const u8 Orre_EventScript_Touch[];

struct OrreRoomPerson
{
    u16 room;
    u8 index, flags, talkTypes;
    s16 angle;
    u16 person, name;
    u32 move, talk;
    float x, z;
};
#include "../data/orre_room_people.h"

struct OrreRoomCheck
{
    u16 room, col;
    u32 script, args[4];
    float x0, z0, x1, z1;
};
#include "../data/orre_room_checks.h"

// The props (src/data/orre_props.h: people with no ORRE person, drawn as Colosseum's own model: src/orre/o3d.c)
struct OrreProp { u16 row; const char *model; };
#include "../data/orre_props.h"

#define NPC_MAX 48
#define NPC_LOCAL_FIRST 0x40   // (a person's object: local id 0x40 + its index; the maps' own objects are below)
#define NPC_HERO 100
#define NPC_PATH_MAX 64

struct Npc
{
    const struct OrreRoomPerson *p;
    bool8 shown;
    s16 x, y;            // its tile (the map's, without the border)
    u8 dir;
    u8 path[NPC_PATH_MAX];   // the walk under way (Kai's movement actions)
    bool8 walking;
    bool8 talkTurned;    // turned to the player for its words (std054), facing talkDir before
    u8 talkDir;
    u8 walkCount;        // its walk list (peopleReserveWalkList / AddWalkList: the tiles it walks through)
    bool8 walkLoop;      // ... gone round and round (peopleStartWalkList's loop)
    s16 walkX[8], walkY[8];
    u8 speed;            // peopleSetWalkSpeed's, as Kai's steps (SPEED_*)
    u8 pending, pendWait;   // a walk of PENDING steps (in path) waiting for its object (shown this frame: no slot yet)
    s16 pendX, pendY;    // ... where it ends
    const char *model;   // a prop's model (src/data/orre_props.h), drawn by o3d.c; NULL: a sprite
    s16 angle;           // its angle (degrees, as Colosseum's: 0 facing +Z)
};

// peopleSetWalkSpeed (Colosseum's 1.0 a walk, 2.0 a run) as Kai's step speeds (Step)
enum { SPEED_NORMAL, SPEED_SLOW, SPEED_FAST, SPEED_FASTER };
static u8 sHeroSpeed;

static struct Npc sNpc[NPC_MAX];
static u8 sCount;
static u16 sRoom;
static u8 sHeroPath[NPC_PATH_MAX];
static bool8 sHeroWalking;

static u8 DirOfAngle(s32 angle)
{
    static const u8 sDirs[4] = {DIR_SOUTH, DIR_EAST, DIR_NORTH, DIR_WEST};
    angle %= 360;
    if (angle < 0)
        angle += 360;
    return sDirs[((angle + 45) / 90) % 4];
}

// RUI as she follows the player: Colosseum's character 101 (the hero's "member": heroMoveDismissMember), a walking
// partner (MOVEMENT_TYPE_FOLLOW_PLAYER: the player's POKéMON follower walks behind her, mp_follower.c) while
// ORRE_FLAG_RUI_FOLLOWS is set; the scenes move and turn her as character 101
#define NPC_PARTNER 101
#define ORRE_PERSON_RUI 1   // (src/data/orre_people.h: hizuki_b1)
static const struct OrreRoomPerson sRuiPerson = {0, NPC_PARTNER, 0x80, 0, 0, ORRE_PERSON_RUI, 0, 0, 0, 0.0f, 0.0f};
static struct Npc sPartner;

static struct Npc *Of(u32 index)
{
    u32 i;
    if (index == NPC_PARTNER)
    {
        // (before she follows, a scene shows her itself: PHENAC's, out of the sack, places character 101 and has her
        // shown: she was no one, so not there until she joined: the user, 2026-11-11)
        if (sPartner.p == NULL && sRoom != 0)
        {
            memset(&sPartner, 0, sizeof(sPartner));
            sPartner.p = &sRuiPerson;
            sPartner.dir = DIR_SOUTH;
            if (gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT)
                sPartner.x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET,
                sPartner.y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET;
        }
        return sPartner.p != NULL ? &sPartner : NULL;
    }
    for (i = 0; i < sCount; i++)
        if (sNpc[i].p->index == index)
            return &sNpc[i];
    return NULL;
}

static u8 LocalId(const struct Npc *n)
{
    return NPC_LOCAL_FIRST + n->p->index;
}

static struct ObjectEvent *Object(const struct Npc *n)
{
    u8 id;
    if (!n->shown || TryGetObjectEventIdByLocalIdAndMap(LocalId(n), gSaveBlock1Ptr->location.mapNum,
                                                        gSaveBlock1Ptr->location.mapGroup, &id))
        return NULL;   // (TRUE: not there)
    return &gObjectEvents[id];
}

// The field engine has 16 object slots (the save's layout keeps them), fewer than a busy room's people (PHENAC shows 19):
// a person who shows has an object only while near the player, the nearest first (Orre_Npcs_Frame); far off they
// keep their place and facing in their record, as the engine's own people do off screen
#define NPC_RANGE_X 10
#define NPC_RANGE_Y 8
#define NPC_SCENE_RANGE 32

static u32 Dist(const struct Npc *n)
{
    s16 px = gSaveBlock1Ptr->pos.x, py = gSaveBlock1Ptr->pos.y;   // (the camera's corner: the player at + 7, + 7)
    if (gPlayerAvatar.objectEventId < OBJECT_EVENTS_COUNT)
        px = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET,
        py = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET;
    return (u32)(abs(n->x - px) + abs(n->y - py));
}

static bool8 InRange(const struct Npc *n)
{
    s16 px = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET;
    s16 py = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET;
    // (a scene's camera looks anywhere in the room: everyone it shows is there, as far as the slots go)
    if (O3D_CamActive())
        return abs(n->x - px) <= NPC_SCENE_RANGE && abs(n->y - py) <= NPC_SCENE_RANGE;
    return abs(n->x - px) <= NPC_RANGE_X && abs(n->y - py) <= NPC_RANGE_Y;
}

// Its object made (FALSE: no slot free)
static bool8 Spawn(struct Npc *n)
{
    u8 id;
    if (n->p->person == ORRE_PERSON_NONE)
    {
        // (a prop or a POKéMON with no sprite yet: RUI's sack in PHENAC, the hunters' truck...) there all the same,
        // unseen, so it stands in the way and can be looked at: its talk script is the story's
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_ITEM_BALL, MOVEMENT_TYPE_NONE, LocalId(n),
                                                  n->x + MAP_OFFSET, n->y + MAP_OFFSET, 3);
        if (id >= OBJECT_EVENTS_COUNT)
            return FALSE;
        gObjectEvents[id].invisible = TRUE;
        return TRUE;
    }
    id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_ORRE_FIRST + n->p->person,
                                              n == &sPartner && Orre_Flag(ORRE_FLAG_RUI_FOLLOWS) ? MOVEMENT_TYPE_FOLLOW_PLAYER : MOVEMENT_TYPE_NONE,
                                              LocalId(n), n->x + MAP_OFFSET, n->y + MAP_OFFSET, 3);
    if (id >= OBJECT_EVENTS_COUNT)
        return FALSE;
    ObjectEventTurn(&gObjectEvents[id], n->dir);
    return TRUE;
}

static void Show(struct Npc *n, bool8 show)
{
    if (show == n->shown)
        return;
    if (!show)
    {
        RemoveObjectEventByLocalIdAndMap(LocalId(n), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        n->shown = FALSE;
        return;
    }
    n->shown = TRUE;
    MPNet_Log("orre npcs: %u shown at %d,%d%s", n->p->index, n->x, n->y,
              n->p->person == ORRE_PERSON_NONE ? " (a prop: unseen)" : "");
    if (InRange(n))
        Spawn(n);   // (no slot: Orre_Npcs_Frame gives it the farthest one's)
}

// Who a room shows when the player arrives: the record's 0x80, which the rooms' first set-up scripts change
// (scriptFloorCharSetDisp writes it into Colosseum's table in memory, so it holds for the rest of the session: here too)
static u8 sDisp[(ORRE_ROOM_PEOPLE_COUNT + 7) / 8];
static bool8 sDispRead;

static bool8 Disp(u32 row)
{
    if (!sDispRead)
    {
        u32 i;
        for (i = 0; i < ORRE_ROOM_PEOPLE_COUNT; i++)
            if (sOrreRoomPeople[i].flags & 0x80)
                sDisp[i / 8] |= 1 << (i % 8);
        sDispRead = TRUE;
    }
    return (sDisp[row / 8] >> (row % 8)) & 1;
}

// The player arrived in ORRE room ROOM: its people (placed by Orre_Npcs_Place once the room's set-up has run)
void Orre_Npcs_RoomEntered(u16 room)
{
    u32 i;
    float cx, cz;
    sCount = 0;
    sRoom = room;
    sHeroWalking = FALSE;
    sHeroSpeed = SPEED_NORMAL;
    sPartner.p = NULL;
    for (i = 0; i < ORRE_ROOM_PEOPLE_COUNT && sCount < NPC_MAX; i++)
    {
        struct Npc *n;
        if (sOrreRoomPeople[i].room != room)
            continue;
        n = &sNpc[sCount++];
        memset(n, 0, sizeof(*n));
        n->p = &sOrreRoomPeople[i];
        cx = n->p->x, cz = n->p->z;
        if (!O3D_ColoToTile(cx, cz, &n->x, &n->y))
            n->x = n->y = 0;
        n->dir = DirOfAngle(n->p->angle);
        n->angle = n->p->angle;
        {
            u32 k;
            for (k = 0; k < ORRE_PROP_COUNT; k++)
                if (sOrreProps[k].row == (u32)(n->p - sOrreRoomPeople))
                    n->model = sOrreProps[k].model;
        }
    }
}

// Colosseum's _floorInitCharacters: after the room's first set-up script (its floor record's + 0x2C), each person is
// put where their record says and shown as their 0x80 says
static void PartnerAppear(void);

void Orre_Npcs_Place(void)
{
    u32 i, shown = 0;
    if (Orre_Flag(ORRE_FLAG_RUI_FOLLOWS))
        PartnerAppear();   // (first: her object before the room's people take the slots)
    for (i = 0; i < sCount; i++)
        if (Disp(sNpc[i].p - sOrreRoomPeople))
        {
            Show(&sNpc[i], TRUE);
            shown += sNpc[i].shown;
        }
    MPNet_Log("orre npcs: room %u, %u people, %u shown", sRoom, sCount, shown);
}

// scriptFloorCharSetDisp: whether person INDEX shows the next time the room is set up
void Orre_Npc_SetDisp(u32 index, bool8 show)
{
    struct Npc *n = Of(index);
    u32 row;
    if (n == NULL)
        return;
    row = n->p - sOrreRoomPeople;
    Disp(row);
    if (show)
        sDisp[row / 8] |= 1 << (row % 8);
    else
        sDisp[row / 8] &= ~(1 << (row % 8));
}

void Orre_Npcs_Clear(void)
{
    sPartner.p = NULL;
    sCount = 0;
    sRoom = 0;
    sHeroWalking = FALSE;
    sHeroSpeed = SPEED_NORMAL;
}

// ---- what the scripts do with them (orre_vm.c)

void Orre_Npc_SetShown(u32 index, bool8 show)
{
    struct Npc *n = Of(index);
    if (n)
        Show(n, show);
}

// Colosseum's place (x, z): the tile it's on
void Orre_Npc_SetPosition(u32 group, u32 index, float x, float z)
{
    struct Npc *n;
    s16 tx, ty;
    if (!O3D_ColoToTile(x, z, &tx, &ty))
        return;
    if (group == 0 && index == NPC_HERO)
    {
        // (the scene puts the player there: a battle's stance, a talk's place; the camera with them, as a jump's
        // landing moves it: never onto a wall)
        struct ObjectEvent *o = &gObjectEvents[gPlayerAvatar.objectEventId];
        s32 dx = tx + MAP_OFFSET - o->currentCoords.x, dy = ty + MAP_OFFSET - o->currentCoords.y;
        if ((dx || dy) && !MapGridGetCollisionAt(tx + MAP_OFFSET, ty + MAP_OFFSET))
        {
            MoveCameraAndRedrawMap(dx, dy);
            MoveObjectEventToMapCoords(o, tx + MAP_OFFSET, ty + MAP_OFFSET);
            MPNet_Log("orre npcs: the player put at %d,%d", tx, ty);
        }
        return;
    }
    if ((n = Of(index)) == NULL)
        return;
    n->x = tx, n->y = ty;
    if (n->shown)
    {
        struct ObjectEvent *o = Object(n);
        if (o)
            MoveObjectEventToMapCoords(o, tx + MAP_OFFSET, ty + MAP_OFFSET);
    }
}

static void Face(u32 group, u32 index, u8 dir)
{
    struct Npc *n;
    if (group == 0 && index == NPC_HERO)
    {
        ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], dir);
        return;
    }
    if ((n = Of(index)) == NULL)
        return;
    n->dir = dir;
    if (n->shown && Object(n))
        ObjectEventTurn(Object(n), dir);
}

void Orre_Npc_SetAngle(u32 group, u32 index, float degrees)
{
    struct Npc *n;
    if (group != 0 || index != NPC_HERO)
        if ((n = Of(index)) != NULL)
            n->angle = (s16)degrees;
    Face(group, index, DirOfAngle((s32)degrees));
}

// The props shown now (o3d.c draws their models): each one's model, where it stands (Colosseum's x, z: its own spot
// while it's on its starting tile, else its tile's middle) and its angle; at most MAX
u32 Orre_Npc_Props(const char **model, float *x, float *z, float *angle, u32 max)
{
    u32 i, k = 0;
    for (i = 0; i < sCount && k < max; i++)
    {
        struct Npc *n = &sNpc[i];
        s16 tx, ty;
        if (!n->shown || n->model == NULL)
            continue;
        model[k] = n->model;
        angle[k] = n->angle;
        if (O3D_ColoToTile(n->p->x, n->p->z, &tx, &ty) && tx == n->x && ty == n->y)
            x[k] = n->p->x, z[k] = n->p->z;
        else if (!O3D_TileToColo(n->x, n->y, &x[k], &z[k]))
            continue;
        k++;
    }
    return k;
}

static bool8 Where(u32 group, u32 index, s16 *x, s16 *y)
{
    struct Npc *n;
    if (group == 0 && index == NPC_HERO)
    {
        *x = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET;
        *y = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET;
        return TRUE;
    }
    if ((n = Of(index)) == NULL)
        return FALSE;
    if (n->shown && Object(n))
    {
        *x = Object(n)->currentCoords.x - MAP_OFFSET;
        *y = Object(n)->currentCoords.y - MAP_OFFSET;
    }
    else
        *x = n->x, *y = n->y;
    return TRUE;
}

// (Colosseum's speeds in its scripts: 0.1-6.0)

static u8 Step(u8 speed, u32 dir)
{
    switch (speed)
    {
    case SPEED_SLOW: return GetWalkSlowMovementAction(dir);
    case SPEED_FAST: return GetWalkFastMovementAction(dir);
    case SPEED_FASTER: return GetWalkFasterMovementAction(dir);
    default: return GetWalkNormalMovementAction(dir);
    }
}

void Orre_Npc_SetSpeed(u32 group, u32 index, float speed)
{
    struct Npc *n;
    u8 s = speed < 0.75f ? SPEED_SLOW : speed < 1.5f ? SPEED_NORMAL : speed < 2.75f ? SPEED_FAST : SPEED_FASTER;
    if (group == 0 && index == NPC_HERO)
        sHeroSpeed = s;
    else if ((n = Of(index)) != NULL)
        n->speed = s;
}

static u8 DirTo(s16 fx, s16 fy, s16 tx, s16 ty)
{
    s16 dx = tx - fx, dy = ty - fy;
    if (abs(dx) >= abs(dy))
        return dx >= 0 ? DIR_EAST : DIR_WEST;
    return dy >= 0 ? DIR_SOUTH : DIR_NORTH;
}

// PERSON (group, index) faces the other one
void Orre_Npc_FaceOther(u32 group, u32 index, u32 tgroup, u32 tindex)
{
    s16 x, y, tx, ty;
    if (Where(group, index, &x, &y) && Where(tgroup, tindex, &tx, &ty) && (x != tx || y != ty))
        Face(group, index, DirTo(x, y, tx, ty));
}

// peopleTurnToPoint(group, index, x, y, z, seconds): faces Colosseum's point (x, z)
void Orre_Npc_FacePoint(u32 group, u32 index, float px, float pz)
{
    s16 x, y, tx, ty;
    if (Where(group, index, &x, &y) && O3D_ColoToTile(px, pz, &tx, &ty) && (x != tx || y != ty))
        Face(group, index, DirTo(x, y, tx, ty));
}

// Someone's scripted walk is under way (the player's too)
static bool8 AnyWalking(void)
{
    u32 i;
    if (sHeroWalking)
        return TRUE;
    for (i = 0; i < sCount; i++)
        if (sNpc[i].walking)
            return TRUE;
    return FALSE;
}

// Walk to tile (TX, TY): along x, then along y, stopping a tile short when STOPSHORT (walking up to someone)
static void StartPath(struct Npc *n, u32 index, u8 localId, u8 *path, u8 k, s16 tx, s16 ty);

static void WalkTo(u32 group, u32 index, s16 tx, s16 ty, bool8 stopShort)
{
    s16 x, y;
    u8 *path, k = 0, localId;
    struct Npc *n = NULL;
    if (!Where(group, index, &x, &y))
        return;
    if (group == 0 && index == NPC_HERO)
        path = sHeroPath, localId = LOCALID_PLAYER;
    else
    {
        n = Of(index);
        if (n == NULL || !n->shown)
        {
            if (n)
                n->x = tx, n->y = ty;   // (hidden: it's simply there)
            return;
        }
        path = n->path, localId = LocalId(n);
    }
    if (stopShort)
    {
        if (abs(tx - x) >= abs(ty - y))
            tx += tx > x ? -1 : 1;
        else
            ty += ty > y ? -1 : 1;
    }
    while (x != tx && k < NPC_PATH_MAX - 1)
    {
        path[k++] = Step(n ? n->speed : sHeroSpeed, tx > x ? DIR_EAST : DIR_WEST);
        x += tx > x ? 1 : -1;
    }
    while (y != ty && k < NPC_PATH_MAX - 1)
    {
        path[k++] = Step(n ? n->speed : sHeroSpeed, ty > y ? DIR_SOUTH : DIR_NORTH);
        y += ty > y ? 1 : -1;
    }
    path[k] = MOVEMENT_ACTION_STEP_END;
    MPNet_Log("orre npcs: %u walks %u steps to %d,%d", index, k, tx, ty);
    StartPath(n, index, localId, path, k, tx, ty);
}

// Kai's scripted movement of PATH (K steps, ending at TX, TY) for N (NULL: the player)
static void StartPath(struct Npc *n, u32 index, u8 localId, u8 *path, u8 k, s16 tx, s16 ty)
{
    if (k == 0)
        return;
    if (n && n->shown && Object(n) == NULL && path == n->path)
    {
        // (just shown, its object not made yet: the walk starts with it, from where it stands: Orre_Npcs_Frame)
        n->pending = k, n->pendWait = 0, n->pendX = tx, n->pendY = ty;
        n->walking = TRUE;
        return;
    }
    if (ScriptMovement_StartObjectMovementScript(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup,
                                                 path)
     && !(!AnyWalking() && (ScriptMovement_UnfreezeObjectEvents(),   // (stale walks of people since parked: cleared)
                            !ScriptMovement_StartObjectMovementScript(localId, gSaveBlock1Ptr->location.mapNum,
                                                                      gSaveBlock1Ptr->location.mapGroup, path))))
    {
        // (its movement couldn't start: one cut off before, a scene stopped mid-walk, still holds its slot) it's
        // simply there, so the scene waiting for it goes on
        struct ObjectEvent *o = n ? Object(n) : &gObjectEvents[gPlayerAvatar.objectEventId];
        MPNet_Log("orre npcs: %u can't walk: put at %d,%d", index, tx, ty);
        if (o)
            MoveObjectEventToMapCoords(o, tx + MAP_OFFSET, ty + MAP_OFFSET);
        if (n)
            n->x = tx, n->y = ty;
        return;
    }
    if (n)
    {
        n->walking = TRUE;
        n->x = tx, n->y = ty;
    }
    else
        sHeroWalking = TRUE;
}

void Orre_Npc_MoveTo(u32 group, u32 index, float x, float z)
{
    s16 tx, ty;
    if (O3D_ColoToTile(x, z, &tx, &ty))
        WalkTo(group, index, tx, ty, FALSE);
}

void Orre_Npc_MoveToOther(u32 group, u32 index, u32 tgroup, u32 tindex)
{
    s16 tx, ty;
    if (Where(tgroup, tindex, &tx, &ty))
        WalkTo(group, index, tx, ty, TRUE);
}

// Its walk is over (peopleMoveCheck)
bool8 Orre_Npc_Done(u32 group, u32 index)
{
    struct Npc *n;
    u8 localId;
    if (group == 0 && index == NPC_HERO)
    {
        if (!sHeroWalking)
            return TRUE;
        localId = LOCALID_PLAYER;
    }
    else
    {
        if ((n = Of(index)) == NULL || !n->walking)
            return TRUE;
        if (n->pending)
            return FALSE;   // (its walk hasn't started yet)
        localId = LocalId(n);
    }
    if (!ScriptMovement_IsObjectMovementFinished(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup))
        return FALSE;
    if (group == 0 && index == NPC_HERO)
        sHeroWalking = FALSE;
    else
        Of(index)->walking = FALSE;
    {
        s16 x, y;
        Where(group, index, &x, &y);
        MPNet_Log("orre npcs: %u walked to %d,%d", index, x, y);
    }
    return TRUE;
}

// std054_characterTalk: a person whose talk start type is 1 (its head) or 2 (itself) turns to the player while it
// talks; std055_characterEndSpeech turns it back (its head always; itself when its talk end type is 1)
void Orre_Npc_TalkStart(u32 group, u32 index)
{
    struct Npc *n;
    u32 start;
    if ((group == 0 && index == NPC_HERO) || (n = Of(index)) == NULL || !n->shown)
        return;
    start = n->p->talkTypes >> 6;
    if (start != 1 && start != 2)
        return;
    if (!n->talkTurned)
        n->talkTurned = TRUE, n->talkDir = n->dir;
    Orre_Npc_FaceOther(group, index, 0, NPC_HERO);
}

void Orre_Npc_TalkEnd(u32 group, u32 index)
{
    struct Npc *n;
    u32 start;
    if ((group == 0 && index == NPC_HERO) || (n = Of(index)) == NULL || !n->talkTurned)
        return;
    n->talkTurned = FALSE;
    start = n->p->talkTypes >> 6;
    if (start == 1 || ((n->p->talkTypes >> 4) & 3) == 1)
        Face(group, index, n->talkDir);
}

// Its name's message (Colosseum's name table row, std054_characterTalk: "???" until the player has heard it)
u16 Orre_Npc_Name(u32 index)
{
    struct Npc *n = Of(index);
    return n ? n->p->name : 0xFFFF;
}

// ---- talking to them

// The script of the object the player talks to on an ORRE map (field_control_avatar.c), NULL if it isn't a person
// with a talk script
const u8 *Orre_Npc_TalkScript(u8 localId)
{
    struct Npc *n;
    if (localId < NPC_LOCAL_FIRST || (n = Of(localId - NPC_LOCAL_FIRST)) == NULL || n->p->talk == 0 || !n->shown)
        return NULL;
    gSpecialVar_0x8004 = n->p->index;
    return Orre_EventScript_Talk;
}

u32 Orre_Npc_TalkScriptId(u32 index)
{
    struct Npc *n = Of(index);
    return n ? n->p->talk : 0;
}

// ---- the rooms' check spots (Colosseum's floor events of kind 3: tools/orre/gen_checks.py)

#define CHECK_REACH 6.0f   // (Colosseum's units: how far in front of the player's tile centre a spot is checked)
#define CHECK_SLACK 4.0f   // (half a tile: a spot's box is often a flat face on a counter or a wall)

// The check spot in front of someone on tile X, Y facing DIR: its row + 1, 0 none
static u32 CheckSpotFrom(s16 x, s16 y, u8 dir)
{
    float cx, cz;
    u32 i;
    if (!Orre_Vm_Running() || !O3D_TileToColo(x, y, &cx, &cz))
        return 0;
    cx += dir == DIR_EAST ? CHECK_REACH : dir == DIR_WEST ? -CHECK_REACH : 0;
    cz += dir == DIR_SOUTH ? CHECK_REACH : dir == DIR_NORTH ? -CHECK_REACH : 0;
    for (i = 0; i < ORRE_ROOM_CHECK_COUNT; i++)
    {
        const struct OrreRoomCheck *c = &sOrreRoomChecks[i];
        if (c->room == sRoom && cx >= c->x0 - CHECK_SLACK && cx <= c->x1 + CHECK_SLACK && cz >= c->z0 - CHECK_SLACK
         && cz <= c->z1 + CHECK_SLACK)
            return i + 1;
    }
    return 0;
}

static u32 CheckSpotInFront(void)
{
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    return CheckSpotFrom(p->currentCoords.x - MAP_OFFSET, p->currentCoords.y - MAP_OFFSET, p->facingDirection);
}

bool8 Orre_Check_SpotInFront(void)
{
    return CheckSpotInFront() != 0;
}

// Tests' walker: the step from FROMX, FROMY towards DIR is shut by a door or gate that a check spot there with the
// room's own script (its button: PYRITE's hideout bookshelf) may open: A facing it
bool8 Orre_TestButtonDoor(s16 fromX, s16 fromY, u8 dir, s16 toX, s16 toY)
{
    u32 row = CheckSpotFrom(fromX, fromY, dir);
    return row && (sOrreRoomChecks[row - 1].script >> 16) == 0x0100 && Orre_Vm_GateShutAt(toX, toY);
}

// The check spot in front of the player when A is pressed: Orre_EventScript_Check with VAR_0x8004 its row, else NULL
const u8 *Orre_Check_InFront(void)
{
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    u32 i;
    if (!Orre_Vm_Running())
        return NULL;
    if ((i = CheckSpotInFront()) != 0)
    {
        gSpecialVar_0x8004 = i - 1;
        return Orre_EventScript_Check;
    }
    {
        // (a prop with something to say, two tiles ahead: its collision, a story gate, may cover the tile between, as
        // RUI's sack in PHENAC does; Colosseum checks the model, not a tile)
        s16 x = p->currentCoords.x - MAP_OFFSET, y = p->currentCoords.y - MAP_OFFSET;
        s16 dx = p->facingDirection == DIR_EAST ? 1 : p->facingDirection == DIR_WEST ? -1 : 0;
        s16 dy = p->facingDirection == DIR_SOUTH ? 1 : p->facingDirection == DIR_NORTH ? -1 : 0;
        for (i = 0; i < sCount; i++)
        {
            struct Npc *n = &sNpc[i];
            if (n->shown && n->p->person == ORRE_PERSON_NONE && n->p->talk && n->x == x + 2 * dx && n->y == y + 2 * dy
             && MapGridGetCollisionAt(x + dx + MAP_OFFSET, y + dy + MAP_OFFSET))
            {
                gSpecialVar_0x8004 = n->p->index;
                return Orre_EventScript_Talk;
            }
        }
    }
    return NULL;
}

// Check spot ROW's script and its four arguments
u32 Orre_Check_Script(u32 row, u32 *args)
{
    if (row >= ORRE_ROOM_CHECK_COUNT)
        return 0;
    memcpy(args, sOrreRoomChecks[row].args, sizeof(sOrreRoomChecks[row].args));
    return sOrreRoomChecks[row].script;
}

// ---- walk lists (Colosseum's peopleReserveWalkList, peopleAddWalkList, peopleStartWalkList: a person walks through
// points; looped, round and round)

void Orre_Npc_WalkListReset(u32 index)
{
    struct Npc *n = Of(index);
    if (n)
        n->walkCount = 0, n->walkLoop = FALSE;
}

void Orre_Npc_WalkListAdd(u32 index, float x, float z)
{
    struct Npc *n = Of(index);
    s16 tx, ty;
    if (n && n->walkCount < ARRAY_COUNT(n->walkX) && O3D_ColoToTile(x, z, &tx, &ty))
        n->walkX[n->walkCount] = tx, n->walkY[n->walkCount] = ty, n->walkCount++;
}

// Its walk through the list's points as one path (each leg along x, then along y); hidden, or not yet on the map (its
// slot comes as the player nears it), it's simply at the list's end (as WalkTo)
static void WalkList(struct Npc *n, u32 index)
{
    s16 x, y;
    u32 i, k = 0;
    if (n->walkCount == 0 || !Where(1, index, &x, &y))
        return;
    if (!n->shown)
    {
        n->x = n->walkX[n->walkCount - 1], n->y = n->walkY[n->walkCount - 1];
        return;
    }
    for (i = 0; i < n->walkCount && k < NPC_PATH_MAX - 1; i++)
    {
        s16 tx = n->walkX[i], ty = n->walkY[i];
        while (x != tx && k < NPC_PATH_MAX - 1)
            n->path[k++] = Step(n->speed, tx > x ? DIR_EAST : DIR_WEST), x += tx > x ? 1 : -1;
        while (y != ty && k < NPC_PATH_MAX - 1)
            n->path[k++] = Step(n->speed, ty > y ? DIR_SOUTH : DIR_NORTH), y += ty > y ? 1 : -1;
    }
    n->path[k] = MOVEMENT_ACTION_STEP_END;
    MPNet_Log("orre npcs: %u walks %u steps through %u points to %d,%d", index, k, n->walkCount, x, y);
    StartPath(n, index, LocalId(n), n->path, k, x, y);
}

void Orre_Npc_WalkListStart(u32 index, bool8 loop)
{
    struct Npc *n = Of(index);
    if (n == NULL)
        return;
    n->walkLoop = loop;
    WalkList(n, index);
}

// Every frame (orre_vm.c): looped walks go round again; a walk of Kai's scripted movement ends when it's over
void Orre_Npcs_Frame(void)
{
    u32 i;
    if (sPartner.p != NULL && sPartner.shown && Object(&sPartner) == NULL)
    {
        struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
        if (Orre_Flag(ORRE_FLAG_RUI_FOLLOWS))   // (one a scene placed stays where it put her)
            sPartner.x = pl->currentCoords.x - MAP_OFFSET, sPartner.y = pl->currentCoords.y - MAP_OFFSET;
        sPartner.walking = FALSE;
        Spawn(&sPartner);   // (after a battle or a screen: there again, at the player's side as they go on)
    }
    for (i = 0; i < sCount; i++)
    {
        struct Npc *n = &sNpc[i];
        struct ObjectEvent *o;
        if (!n->shown)
            continue;
        o = Object(n);
        if (n->pending && o != NULL)
        {
            u8 k = n->pending;   // (its object is here: the walk it was given starts)
            n->pending = 0;
            StartPath(n, n->p->index, LocalId(n), n->path, k, n->pendX, n->pendY);
            continue;
        }
        if (n->pending && ++n->pendWait > 30)
        {
            n->pending = 0, n->walking = FALSE;   // (no object for it: it's simply there)
            n->x = n->pendX, n->y = n->pendY;
            MPNet_Log("orre npcs: %u put at %d,%d (no slot to walk in)", n->p->index, n->x, n->y);
        }
        if (o != NULL && !n->walking && !n->talkTurned && !InRange(n))
        {
            // (far off: its slot for someone nearer; it keeps its place and facing)
            n->x = o->currentCoords.x - MAP_OFFSET, n->y = o->currentCoords.y - MAP_OFFSET, n->dir = o->facingDirection;
            RemoveObjectEventByLocalIdAndMap(LocalId(n), gSaveBlock1Ptr->location.mapNum,
                                             gSaveBlock1Ptr->location.mapGroup);
            continue;
        }
        if (o == NULL && InRange(n))
        {
            // (near, or the field came back from a battle or a screen without it: there again; no slot free, the
            // farthest one with one gives up its own)
            if (!n->pending)
                n->walking = FALSE;
            if (!Spawn(n))
            {
                u32 j, far = sCount;
                for (j = 0; j < sCount; j++)
                    if (sNpc[j].shown && !sNpc[j].walking && !sNpc[j].talkTurned && Dist(&sNpc[j]) > Dist(n)
                     && Object(&sNpc[j]) != NULL && (far == sCount || Dist(&sNpc[j]) > Dist(&sNpc[far])))
                        far = j;
                if (far < sCount)
                {
                    struct ObjectEvent *fo = Object(&sNpc[far]);
                    sNpc[far].x = fo->currentCoords.x - MAP_OFFSET, sNpc[far].y = fo->currentCoords.y - MAP_OFFSET;
                    sNpc[far].dir = fo->facingDirection;
                    RemoveObjectEventByLocalIdAndMap(LocalId(&sNpc[far]), gSaveBlock1Ptr->location.mapNum,
                                                     gSaveBlock1Ptr->location.mapGroup);
                    Spawn(n);
                }
            }
            continue;
        }
        if (n->walkLoop && n->walking
         && ScriptMovement_IsObjectMovementFinished(LocalId(n), gSaveBlock1Ptr->location.mapNum,
                                                    gSaveBlock1Ptr->location.mapGroup))
        {
            n->walking = FALSE;
            WalkList(n, n->p->index);
        }
    }
}

// std131_characterSurpriseAnimation: the "!" over them
void Orre_Npc_Surprise(u32 group, u32 index)
{
    struct Npc *n;
    if (group == 0 && index == NPC_HERO)
        gFieldEffectArguments[0] = LOCALID_PLAYER;
    else if ((n = Of(index)) != NULL && n->shown)
        gFieldEffectArguments[0] = LocalId(n);
    else
        return;
    gFieldEffectArguments[1] = gSaveBlock1Ptr->location.mapNum;
    gFieldEffectArguments[2] = gSaveBlock1Ptr->location.mapGroup;
    FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);
    MPNet_Log("orre npcs: %u surprised", index);
}

// std172_characterGetPosition: where they stand, in Colosseum's units (their tile's centre); FALSE: no such person
bool8 Orre_Npc_GetColo(u32 group, u32 index, float *x, float *z)
{
    s16 tx, ty;
    return Where(group, index, &tx, &ty) && O3D_TileToColo(tx, ty, x, z);
}

// peopleGazeHeroCheck: the player stands right in front of them, where they look (Colosseum: within 3 units, 30
// degrees either side of their facing)
bool8 Orre_Npc_SeesHero(u32 index)
{
    struct Npc *n = Of(index);
    s16 x, y, hx, hy;
    if (n == NULL || !n->shown || !Object(n) || !Where(0, NPC_HERO, &hx, &hy))
        return FALSE;
    x = Object(n)->currentCoords.x - MAP_OFFSET, y = Object(n)->currentCoords.y - MAP_OFFSET;
    switch (Object(n)->facingDirection)
    {
    case DIR_NORTH: return hx == x && hy == y - 1;
    case DIR_SOUTH: return hx == x && hy == y + 1;
    case DIR_WEST: return hy == y && hx == x - 1;
    case DIR_EAST: return hy == y && hx == x + 1;
    }
    return FALSE;
}

// ---- touch spots (Colosseum's floor events of kind 2: stepping into the trigger runs its script)

// Every frame (orre_vm.c, nothing running): the player stepped onto a new tile inside a touch spot's box: its script,
// once until they've left it
void Orre_Touch_Frame(void)
{
    static s16 sX = -1, sY = -1;
    static s16 sInside = -1;
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = p->currentCoords.x - MAP_OFFSET, y = p->currentCoords.y - MAP_OFFSET;
    float cx, cz;
    s32 i, at = -1;
    if (x == sX && y == sY)
        return;
    // (a way out's tile: Kai's trigger there, the world map, goes first: it starts as the step ends, later in the frame
    // than this. Colosseum's exits are the floor's own, and the spot's script only speaks when the story keeps the
    // player in: starting first, it had the trigger never fire. PHENAC's west and east ways out never opened the map)
    // (the object's tile changes as its step begins: the trigger only fires once the step is done, so the spot waits
    // for the player to have stood there two frames)
    {
        static u8 sStood;
        struct MapPosition pos = {p->currentCoords.x, p->currentCoords.y, 0};
        if (GetCoordEventScriptAtMapPosition(&pos) != NULL)
        {
            sStood = gPlayerAvatar.tileTransitionState == T_TILE_TRANSITION ? 0 : sStood + 1;
            if (sStood < 3)
                return;
        }
        sStood = 0;
    }
    sX = x, sY = y;
    if (!O3D_TileToColo(x, y, &cx, &cz))
        return;
    for (i = 0; i < ORRE_ROOM_TOUCH_COUNT; i++)
    {
        const struct OrreRoomCheck *c = &sOrreRoomTouches[i];
        if (c->room == sRoom && cx >= c->x0 - CHECK_SLACK && cx <= c->x1 + CHECK_SLACK && cz >= c->z0 - CHECK_SLACK
         && cz <= c->z1 + CHECK_SLACK)
        {
            at = i;
            break;
        }
    }
    if (at >= 0 && at != sInside && !ScriptContext_IsEnabled())
    {
        gSpecialVar_0x8004 = at;
        ScriptContext_SetupScript(Orre_EventScript_Touch);
        MPNet_Log("orre npcs: touch %d (room %u, collision %u)", at, sRoom, sOrreRoomTouches[at].col);
    }
    sInside = at;
}

u32 Orre_Touch_Script(u32 row, u32 *args)
{
    if (row >= ORRE_ROOM_TOUCH_COUNT)
        return 0;
    memcpy(args, sOrreRoomTouches[row].args, sizeof(sOrreRoomTouches[row].args));
    return sOrreRoomTouches[row].script;
}

// RUI behind the player: on the tile behind them (the way they face), else on theirs (the player walks through her)
static void PartnerAppear(void)
{
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 x = pl->currentCoords.x, y = pl->currentCoords.y;
    s16 bx = x - gDirectionToVectors[pl->facingDirection].x, by = y - gDirectionToVectors[pl->facingDirection].y;
    if (sPartner.p != NULL && sPartner.shown && Object(&sPartner) != NULL)
        return;
    memset(&sPartner, 0, sizeof(sPartner));
    sPartner.p = &sRuiPerson;
    if (!MapGridGetCollisionAt(bx, by))
        x = bx, y = by;
    sPartner.x = x - MAP_OFFSET, sPartner.y = y - MAP_OFFSET;
    sPartner.dir = pl->facingDirection;
    sPartner.shown = TRUE;
    Spawn(&sPartner);
    MPNet_Log("orre npcs: RUI follows (at %d,%d)", sPartner.x, sPartner.y);
}

// std035_setPartnerFollowing: RUI joins the player (or leaves: the scene shows her where it wants her)
void Orre_Npc_PartnerFollow(bool8 on)
{
    if (on && !Orre_Flag(ORRE_FLAG_RUI_FOLLOWS) && sPartner.p != NULL)
    {
        // (the scene had her standing about as one of its people: she starts over as the one who follows)
        if (sPartner.shown)
            RemoveObjectEventByLocalIdAndMap(LocalId(&sPartner), gSaveBlock1Ptr->location.mapNum,
                                             gSaveBlock1Ptr->location.mapGroup);
        sPartner.p = NULL;
    }
    Orre_SetFlag(ORRE_FLAG_RUI_FOLLOWS, on);
    if (on)
        PartnerAppear();
    else if (sPartner.p != NULL)
    {
        RemoveObjectEventByLocalIdAndMap(LocalId(&sPartner), gSaveBlock1Ptr->location.mapNum,
                                         gSaveBlock1Ptr->location.mapGroup);
        sPartner.p = NULL;
        MPNet_Log("orre npcs: RUI no longer follows");
    }
}

// ---- tests (the harness's approach: src/platform/sdl2.c, mp_debug.c MP_TestApproachKey)

// Where person INDEX of the current room stands, FALSE when not shown
bool8 Orre_TestPersonTile(u32 index, s16 *x, s16 *y)
{
    struct Npc *n = Of(index);
    if (n == NULL || !n->shown)
        return FALSE;
    if (Object(n) == NULL)
    {
        *x = n->x, *y = n->y;   // (too far off for Kai to have made their object yet: where they stand)
        return TRUE;
    }
    *x = Object(n)->currentCoords.x - MAP_OFFSET;
    *y = Object(n)->currentCoords.y - MAP_OFFSET;
    return TRUE;
}

// The tiles under the current room's touch (TOUCH) or check spot with collision id COL: its box's corners' tiles
// (tests) standing on this tile sets off the room's touch spot COL: Orre_Touch_Frame's own rule
bool8 Orre_TestTouchTile(u32 col, s16 x, s16 y)
{
    float cx, cz;
    u32 i;
    if (!O3D_TileToColo(x, y, &cx, &cz))
        return FALSE;
    for (i = 0; i < ORRE_ROOM_TOUCH_COUNT; i++)
    {
        const struct OrreRoomCheck *c = &sOrreRoomTouches[i];
        if (c->room == sRoom && cx >= c->x0 - CHECK_SLACK && cx <= c->x1 + CHECK_SLACK && cz >= c->z0 - CHECK_SLACK
         && cz <= c->z1 + CHECK_SLACK)
            return c->col == col;   // (the first spot the tile is in, as the game picks it)
    }
    return FALSE;
}

bool8 Orre_TestSpotTiles(bool8 touch, u32 col, s16 *x0, s16 *y0, s16 *x1, s16 *y1)
{
    const struct OrreRoomCheck *t = touch ? sOrreRoomTouches : sOrreRoomChecks;
    u32 i, n = touch ? ORRE_ROOM_TOUCH_COUNT : ORRE_ROOM_CHECK_COUNT;
    for (i = 0; i < n; i++)
        if (t[i].room == sRoom && t[i].col == col)
        {
            s16 ax, ay, bx, by;
            if (!O3D_ColoToTile(t[i].x0, t[i].z0, &ax, &ay) || !O3D_ColoToTile(t[i].x1, t[i].z1, &bx, &by))
                return FALSE;
            *x0 = ax < bx ? ax : bx, *x1 = ax < bx ? bx : ax;
            *y0 = ay < by ? ay : by, *y1 = ay < by ? by : ay;
            return TRUE;
        }
    return FALSE;
}
