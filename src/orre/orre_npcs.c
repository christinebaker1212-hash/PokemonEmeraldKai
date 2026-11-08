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
#include "field_player_avatar.h"
#include "o3d.h"
#include "orre.h"
#include "orre_people.h"
#include "script_movement.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"

void MPNet_Log(const char *fmt, ...);
extern const u8 Orre_EventScript_Talk[];
extern const u8 Orre_EventScript_Check[];

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
};

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

static struct Npc *Of(u32 index)
{
    u32 i;
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
    if (n->p->person == ORRE_PERSON_NONE)
        return;   // (a POKéMON or a prop: not a person yet)
    SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_ORRE_FIRST + n->p->person, MOVEMENT_TYPE_NONE, LocalId(n),
                                         n->x + MAP_OFFSET, n->y + MAP_OFFSET, 3);
    n->shown = TRUE;
    MPNet_Log("orre npcs: %u shown at %d,%d", n->p->index, n->x, n->y);
    {
        struct ObjectEvent *o = Object(n);
        if (o)
            ObjectEventTurn(o, n->dir);
    }
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
    }
}

// Colosseum's _floorInitCharacters: after the room's first set-up script (its floor record's + 0x2C), each person is
// put where their record says and shown as their 0x80 says
void Orre_Npcs_Place(void)
{
    u32 i, shown = 0;
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
    sCount = 0;
    sRoom = 0;
    sHeroWalking = FALSE;
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
        return;   // (the player stays where they stand: Kai's camera follows them)
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
    Face(group, index, DirOfAngle((s32)degrees));
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

// Walk to tile (TX, TY): along x, then along y, stopping a tile short when STOPSHORT (walking up to someone)
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
        path[k++] = GetWalkNormalMovementAction(tx > x ? DIR_EAST : DIR_WEST);
        x += tx > x ? 1 : -1;
    }
    while (y != ty && k < NPC_PATH_MAX - 1)
    {
        path[k++] = GetWalkNormalMovementAction(ty > y ? DIR_SOUTH : DIR_NORTH);
        y += ty > y ? 1 : -1;
    }
    path[k] = MOVEMENT_ACTION_STEP_END;
    MPNet_Log("orre npcs: %u walks %u steps to %d,%d", index, k, tx, ty);
    if (k == 0)
        return;
    ScriptMovement_StartObjectMovementScript(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup,
                                             path);
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

// The check spot in front of the player when A is pressed: Orre_EventScript_Check with VAR_0x8004 its row, else NULL
const u8 *Orre_Check_InFront(void)
{
    struct ObjectEvent *p = &gObjectEvents[gPlayerAvatar.objectEventId];
    float cx, cz;
    u32 i;
    if (!Orre_Vm_Running() || !O3D_TileToColo(p->currentCoords.x - MAP_OFFSET, p->currentCoords.y - MAP_OFFSET, &cx, &cz))
        return NULL;
    cx += p->facingDirection == DIR_EAST ? CHECK_REACH : p->facingDirection == DIR_WEST ? -CHECK_REACH : 0;
    cz += p->facingDirection == DIR_SOUTH ? CHECK_REACH : p->facingDirection == DIR_NORTH ? -CHECK_REACH : 0;
    for (i = 0; i < ORRE_ROOM_CHECK_COUNT; i++)
    {
        const struct OrreRoomCheck *c = &sOrreRoomChecks[i];
        if (c->room == sRoom && cx >= c->x0 - CHECK_SLACK && cx <= c->x1 + CHECK_SLACK && cz >= c->z0 - CHECK_SLACK
         && cz <= c->z1 + CHECK_SLACK)
        {
            gSpecialVar_0x8004 = i;
            return Orre_EventScript_Check;
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
