// Moemon Kai: UNOVA's scripts (docs/UNOVA_HANDOFF.md "Black's words and scripts"). Black's own script bytecode, from
// unova/black_scripts.pak beside the game (tools/unova/build_scripts.py, made from the user's Black ROM when the
// release is built; nothing of it is in this repository), run as Black's field VM runs it (aletheia's vm.c,
// script_work_cmd.c): a call stack that's also the comparisons' stack, Black's own event work (flags and works, kept
// in the region side file as Black lays them out: mp_region.c chunk UNEW), its messages from UNOVA's text pack, its
// people moved by its movement lists. Every command's arguments are known (src/unova/u3d_cmds.h from
// tools/unova/scrcmd.py), so one this file doesn't play yet is read past and the script goes on.
//
// The field: an UNOVA map's people, signs and trigger spots run Unova_EventScript_Talk / _Sign / _Pos
// (data/scripts/unova.inc), which start the script Black's event data gives them here and wait (waitstate) until it
// ends. A map's own scripts (Black's init file: the field-init script that places people by the story, the scene
// table run when a work holds a value) run from U3D_Vm_MapLoad and U3D_Vm_SceneCheck.
//
// Its own code, never SINNOH's (s3d_scrcmd.c) or ORRE's (orre_vm.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "script_movement.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "u3d.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "u3d_cmds.h"
#include "u3d_maps.h"

const char *Updater_ExeFile(const char *name);
int uncompress(unsigned char *dest, unsigned long *destLen, const unsigned char *source, unsigned long sourceLen);
void MPNet_Log(const char *fmt, ...);

#define SYSTEM_BANKS 288        // (tools/unova/build_text.py: the system banks, then the story banks)
#define MSG_OWN 0x400            // (a message command's file: the script's own)
#define OBJ_PLAYER 0xFF

// ------------------------------------------------------------------------------------------------ the pack
static u8 *sPack;
static u32 sPackSize;
static bool8 sTried;
static u32 sFilesAt, sRangesAt, sZonesAt, sEventsAt;

static u32 Get32(u32 at) { return at + 4 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 | sPack[at + 2] << 16 | (u32)sPack[at + 3] << 24 : 0; }
static u16 Get16(u32 at) { return at + 2 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 : 0; }

static u32 SkipFiles(u32 at) { return at + 4 + 8 * Get32(at); }

static void Load(void)
{
    const char *path = getenv("MOEMON_U3D_SCRIPTS");
    FILE *f;
    long size;
    u8 *data;
    unsigned long raw;
    sTried = TRUE;
    if (path == NULL)
        path = Updater_ExeFile("unova/black_scripts.pak");
    if ((f = fopen(path, "rb")) == NULL)
        return;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(size > 0 ? size : 1);
    if (data == NULL || fread(data, 1, size, f) != (size_t)size || size < 8 || memcmp(data, "U3SZ", 4) != 0)
    {
        fclose(f);
        free(data);
        printf("[u3d] %s isn't UNOVA's scripts\n", path);
        return;
    }
    fclose(f);
    raw = data[4] | data[5] << 8 | data[6] << 16 | (unsigned long)data[7] << 24;
    sPack = malloc(raw ? raw : 1);
    sPackSize = raw;
    if (sPack == NULL || uncompress(sPack, &raw, data + 8, (unsigned long)(size - 8)) != 0 || memcmp(sPack, "U3DS", 4) != 0
     || Get32(4) != 1)
    {
        printf("[u3d] %s: can't read UNOVA's scripts\n", path);
        free(sPack);
        sPack = NULL;
        sPackSize = 0;
    }
    free(data);
    if (sPack == NULL)
        return;
    sFilesAt = 8;
    // (the files' bodies follow their table: the next block starts after the last one)
    {
        u32 n = Get32(sFilesAt), last = n ? Get32(sFilesAt + 4 + 8 * (n - 1)) + Get32(sFilesAt + 8 + 8 * (n - 1)) : SkipFiles(sFilesAt);
        sRangesAt = (last + 3) & ~3;
    }
    sZonesAt = sRangesAt + 4 + 10 * Get32(sRangesAt);
    sEventsAt = sZonesAt + 4 + 16 * Get32(sZonesAt);
    printf("[u3d] UNOVA's scripts: %u files, %u zones\n", Get32(sFilesAt), Get32(sZonesAt));
}

static bool8 Ready(void)
{
    if (!sTried)
        Load();
    return sPack != NULL;
}

static const u8 *FileOf(u32 block, u32 index, u32 *size)
{
    if (!Ready() || index >= Get32(block))
    {
        *size = 0;
        return NULL;
    }
    *size = Get32(block + 8 + 8 * index);
    return sPack + Get32(block + 4 + 8 * index);
}

enum { Z_SCRIPT, Z_INIT, Z_MSG, Z_EVENT, Z_MATRIX, Z_PARENT, Z_TYPE, Z_PLACE };
static u16 ZoneField(u16 zone, u32 field)
{
    if (!Ready() || zone >= Get32(sZonesAt))
        return 0xFFFF;
    return Get16(sZonesAt + 4 + 16 * zone + 2 * field);
}

// ------------------------------------------------------------------------------------------------ Black's event work
// (zonedata.c: works 0x4000 on, then the flags 1-0xB5F; 0x4000+ flags are temporary, 0x8000+ works the script's own)
#define WORKS 0x140
#define FLAG_BYTES 0x16C
static struct { u16 work[WORKS]; u8 flags[FLAG_BYTES]; } sWork;
static u8 sTempFlags[8];
static u16 sLocal[0x40];

void U3D_Vm_ResetWork(void)
{
    memset(&sWork, 0, sizeof(sWork));
    memset(sTempFlags, 0, sizeof(sTempFlags));
}

const void *U3D_Vm_WorkData(u32 *size)
{
    *size = sizeof(sWork);
    return &sWork;
}

void U3D_Vm_WorkLoad(const void *data, u32 size)
{
    U3D_Vm_ResetWork();
    memcpy(&sWork, data, size < sizeof(sWork) ? size : sizeof(sWork));
}

static u16 sDummyWork;
static u16 *WorkPtr(u16 id)
{
    if (id >= 0x4000 && id < 0x4000 + WORKS)
        return &sWork.work[id - 0x4000];
    if (id >= 0x8000 && id < 0xC000)
        return &sLocal[(id - 0x8000) & 0x3F];
    sDummyWork = 0;
    return &sDummyWork;
}

static u16 WorkValue(u16 id)
{
    return id < 0x4000 ? id : *WorkPtr(id);
}

// Emerald's vars U3D_VAR_BASE + n are Black's works 0x4000 + n (event_data.c GetVarPointer: an UNOVA map's trigger
// spots compare them as Black's do)
u16 *U3D_Vm_VarPointer(u16 id)
{
    return WorkPtr(id - U3D_VAR_BASE + 0x4000);
}

static u8 *FlagByte(u16 id, u8 *bit)
{
    *bit = 1 << (id % 8);
    if (id == 0)
        return NULL;
    if (id < 0x4000)
        return (id >> 3) < FLAG_BYTES ? &sWork.flags[id >> 3] : NULL;
    return ((id - 0x4000) >> 3) < sizeof(sTempFlags) ? &sTempFlags[(id - 0x4000) >> 3] : NULL;
}

static bool8 FlagOn(u16 id)
{
    u8 bit, *p = FlagByte(id, &bit);
    return p && (*p & bit);
}

static void FlagTo(u16 id, bool8 on)
{
    u8 bit, *p = FlagByte(id, &bit);
    if (p)
        *p = on ? (*p | bit) : (*p & ~bit);
}

bool8 U3D_Vm_Flag(u16 id) { return FlagOn(id); }

// ------------------------------------------------------------------------------------------------ the zone's events
static u16 CurrentZone(void)
{
    return U3D_CurrentZone();
}

// Black's event file of a zone: the counts, then the tables (tools/unova/events.py)
static const u8 *Events(u16 zone, u32 *size)
{
    u16 ev = ZoneField(zone, Z_EVENT);
    return ev == 0xFFFF ? (*size = 0, NULL) : FileOf(sEventsAt, ev, size);
}

#define EV_BG 0x14
#define EV_PERSON 0x24
#define EV_CONNECT 0x14
#define EV_POS 0x16

static const u8 *Person(u16 zone, u8 index)
{
    u32 size;
    const u8 *e = Events(zone, &size);
    if (e == NULL || size < 8 || index >= e[5])
        return NULL;
    return e + 8 + EV_BG * e[4] + EV_PERSON * index;
}

static u16 Le16(const u8 *p) { return p[0] | p[1] << 8; }

// ------------------------------------------------------------------------------------------------ the map's people
// The map's object event of Black's object id OBJ: its person's place in the zone's event data + 1
// (tools/unova/gen_maps.py writes them in that order); 0xFF is the player
static u8 LocalId(u16 obj)
{
    u32 size, i;
    const u8 *e;
    if (obj == OBJ_PLAYER)
        return OBJ_EVENT_ID_PLAYER;
    e = Events(U3D_CurrentZone(), &size);
    for (i = 0; e && size >= 8 && i < e[5]; i++)
        if (Le16(e + 8 + EV_BG * e[4] + EV_PERSON * i) == obj)
            return i + 1;
    return (u8)(obj + 1);
}

static struct ObjectEvent *ObjectOf(u16 obj)
{
    u8 id;
    if (TryGetObjectEventIdByLocalIdAndMap(LocalId(obj), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
        return NULL;
    return &gObjectEvents[id];
}

// event_object_movement.c: a person of the map hidden by Black's flag (its event data's vanish flag)
bool8 U3D_Vm_ObjectHidden(u8 localId)
{
    const u8 *p;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || localId == 0 || localId >= OBJ_EVENT_ID_PLAYER)
        return FALSE;
    p = Person(CurrentZone(), localId - 1);
    if (p == NULL)
        return FALSE;
    if (Le16(p + 8) != 0 && FlagOn(Le16(p + 8)))   // (MmdlHeader: id, code, move, type, flag)
        return TRUE;
    {
        // (a person whose spot is on another of the zone's maps: tools/unova/gen_maps.py keeps its place in the list)
        s32 m = U3D_MapIndex(), x = Le16(p + 28), z = Le16(p + 30);
        if (m >= 0 && Le16(p + 24) == 0)
        {
            x -= 32 * U3D_MapCellX(m), z -= 32 * U3D_MapCellZ(m);
            if (x < 0 || z < 0 || x >= gMapHeader.mapLayout->width || z >= gMapHeader.mapLayout->height)
                return TRUE;
        }
    }
    return FALSE;
}

static u8 BlackToDir(u16 d)   // (Black: up, down, left, right)
{
    static const u8 sDirs[4] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST};
    return sDirs[d & 3];
}

static u8 DirToBlack(u8 d)
{
    switch (d)
    {
    case DIR_NORTH: return 0;
    case DIR_WEST:  return 2;
    case DIR_EAST:  return 3;
    default:        return 1;
    }
}

// Black's tile coordinates (the matrix's) -> the map's own
static s32 MapX(u16 x) { s32 i = U3D_MapIndex(); return i < 0 ? x : x - 32 * U3D_MapCellX(i); }
static s32 MapZ(u16 z) { s32 i = U3D_MapIndex(); return i < 0 ? z : z - 32 * U3D_MapCellZ(i); }

// ------------------------------------------------------------------------------------------------ movement lists
// Black's movement codes (the ACMD table, ov021 0x021D5F58) as Emerald's movement actions. Black's frames are its
// field's 30 a second: 8 frames a tile is Emerald's normal walk
#define MAX_MOVE 96
static u8 sMoves[OBJECT_EVENTS_COUNT][MAX_MOVE];
static u8 sMoving[OBJECT_EVENTS_COUNT];   // local ids moving for the script

static u8 Dir4(u8 code, u8 base)   // Black's U D L R at code - base -> Emerald's D U L R offset
{
    static const u8 sOff[4] = {1, 0, 2, 3};
    return sOff[(code - base) & 3];
}

static u32 AddMoves(u8 *out, u32 n, u16 code)
{
#define PUT(a) do { if (n < MAX_MOVE - 2) out[n++] = (a); } while (0)
    static const u8 sStepMove[12] = {0, 1, 2, 3, 0, 1, 2, 2, 0, 1, 2, 3};   // (Step* / Hop* / DashStep*: the way walked)
    static const u8 sStepFace[12] = {2, 3, 1, 0, 3, 2, 0, 1, 1, 0, 3, 2};   // (... and the way faced after, Black's U D L R)
    if (code <= 0x03)
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(code, 0x00));
    else if (code <= 0x0B)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0x04));
    else if (code <= 0x0F)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x0C));
    else if (code <= 0x13)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x10));
    else if (code <= 0x17)
        PUT(MOVEMENT_ACTION_WALK_FASTER_DOWN + Dir4(code, 0x14));
    else if (code <= 0x1F)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_SLOW_DOWN + Dir4(code, 0x18));
    else if (code <= 0x23)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_NORMAL_DOWN + Dir4(code, 0x20));
    else if (code <= 0x27)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_FAST_DOWN + Dir4(code, 0x24));
    else if (code <= 0x2B)
        PUT(MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN + Dir4(code, 0x28));
    else if (code <= 0x33)
        PUT(MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN + Dir4(code, 0x2C));
    else if (code <= 0x37)
        PUT(MOVEMENT_ACTION_JUMP_DOWN + Dir4(code, 0x34));
    else if (code <= 0x3B)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + Dir4(code, 0x38));
    else if (code <= 0x42)
    {
        static const u8 sWait[7] = {2, 4, 8, 16, 30, 32, 64};   // (Wait 1, 2, 4, 8, 15, 16, 32 of Black's frames)
        u32 f = sWait[code - 0x3C];
        while (f >= 16) { PUT(MOVEMENT_ACTION_DELAY_16); f -= 16; }
        if (f >= 8) { PUT(MOVEMENT_ACTION_DELAY_8); f -= 8; }
        if (f >= 4) { PUT(MOVEMENT_ACTION_DELAY_4); f -= 4; }
        if (f >= 2) { PUT(MOVEMENT_ACTION_DELAY_2); f -= 2; }
    }
    else if (code == 0x45)
        PUT(MOVEMENT_ACTION_SET_INVISIBLE);
    else if (code == 0x46)
        PUT(MOVEMENT_ACTION_SET_VISIBLE);
    else if (code == 0x47)
        PUT(MOVEMENT_ACTION_LOCK_FACING_DIRECTION);
    else if (code == 0x48)
        PUT(MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION);
    else if (code == 0x4B || code == 0x67 || code == 0x99)
        PUT(MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK);
    else if (code >= 0x4C && code <= 0x4F)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x4C));
    else if (code >= 0x50 && code <= 0x53)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x50));
    else if (code >= 0x54 && code <= 0x57)
        PUT(MOVEMENT_ACTION_WALK_FASTER_DOWN + Dir4(code, 0x54));
    else if (code >= 0x58 && code <= 0x5B)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0x58));
    else if (code == 0x5C || code == 0x5D)
        PUT(MOVEMENT_ACTION_JUMP_DOWN + (code == 0x5C ? 2 : 3));
    else if (code == 0x5E || code == 0x5F)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + (code == 0x5E ? 2 : 3));
    else if (code >= 0x60 && code <= 0x63)
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(code, 0x60));
    else if (code >= 0x69 && code <= 0x74)   // Step... Face... 8F
    {
        PUT(MOVEMENT_ACTION_WALK_NORMAL_DOWN + Dir4(sStepMove[code - 0x69], 0));
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(sStepFace[code - 0x69], 0));
    }
    else if (code >= 0x75 && code <= 0x78)
        PUT(MOVEMENT_ACTION_JUMP_2_DOWN + Dir4(code, 0x75));
    else if (code >= 0x79 && code <= 0x84)   // DashStep... 4F
    {
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(sStepMove[code - 0x79], 0));
        PUT(MOVEMENT_ACTION_FACE_DOWN + Dir4(sStepFace[code - 0x79], 0));
    }
    else if (code >= 0x85 && code <= 0x90)   // Hop... 8F: a hop on the spot, turning
    {
        PUT(MOVEMENT_ACTION_JUMP_IN_PLACE_DOWN + Dir4(sStepFace[code - 0x85], 0));
    }
    else if (code >= 0x91 && code <= 0x98)   // Step(dir)Face(opposite) 4F / 2F: a step back, still facing
    {
        static const u8 sBack[4] = {0, 1, 2, 3};
        u8 d = sBack[(code - 0x91) & 3];
        PUT(MOVEMENT_ACTION_LOCK_FACING_DIRECTION);
        PUT((code < 0x95 ? MOVEMENT_ACTION_WALK_FAST_DOWN : MOVEMENT_ACTION_WALK_FASTER_DOWN) + Dir4(d, 0));
        PUT(MOVEMENT_ACTION_UNLOCK_FACING_DIRECTION);
    }
    else if (code >= 0x9B && code <= 0x9E)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0x9B));
    else if (code == 0x9F)
        PUT(MOVEMENT_ACTION_EMOTE_QUESTION_MARK);
    else if (code == 0xA0)
        PUT(MOVEMENT_ACTION_EMOTE_HEART);
    else if (code >= 0xA3 && code <= 0xA6)
        PUT(MOVEMENT_ACTION_WALK_FAST_DOWN + Dir4(code, 0xA3));
    else if (code >= 0xA7 && code <= 0xAA)
        PUT(MOVEMENT_ACTION_WALK_SLOW_DOWN + Dir4(code, 0xA7));
    // (the rest: draw-state switches and effects Kai's people don't have)
    return n;
#undef PUT
}

static void StartMoves(u16 obj, const u8 *list, const u8 *end)
{
    u8 slot, localId = LocalId(obj);
    u32 n = 0;
    u8 *out;
    for (slot = 0; slot < OBJECT_EVENTS_COUNT; slot++)
        if (sMoving[slot] == localId || sMoving[slot] == 0)
            break;
    if (slot == OBJECT_EVENTS_COUNT || ObjectOf(obj) == NULL)
        return;
    out = sMoves[slot];
    while (list + 4 <= end)
    {
        u16 code = Le16(list), count = Le16(list + 2), k;
        list += 4;
        if (code == 0xFE)
            break;
        for (k = 0; k < count && k < 32; k++)
            n = AddMoves(out, n, code);
    }
    out[n] = MOVEMENT_ACTION_STEP_END;
    sMoving[slot] = localId;
    ScriptMovement_StartObjectMovementScript(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, out);
}

static bool8 MovesDone(void)
{
    u8 slot;
    bool8 done = TRUE;
    for (slot = 0; slot < OBJECT_EVENTS_COUNT; slot++)
    {
        if (sMoving[slot] == 0)
            continue;
        if (ScriptMovement_IsObjectMovementFinished(sMoving[slot], gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup))
            sMoving[slot] = 0;
        else
            done = FALSE;
    }
    return done;
}

// ------------------------------------------------------------------------------------------------ the VM
enum { W_NONE, W_FRAMES, W_MESSAGE, W_MESSAGE_KEY, W_KEY, W_MOVES, W_FADE, W_YESNO, W_CLOSE };
#define STACK 48

struct Thread
{
    const u8 *code;
    u32 size, pc;
    u32 stack[STACK];
    u8 sp, cmp, wait;
    bool8 alive, box;
    u16 zone, bank, waitFrames, target;
    u16 *result;
};

static struct Thread sThread;
static u8 sPlaceholder[8];
static bool8 sHolding;   // Unova_EventScript_* waits (waitstate) until the script ends
static u16 sLogged[U3D_CMD_COUNT / 16 + 1];

static void Push(struct Thread *t, u32 v)
{
    if (t->sp < STACK)
        t->stack[t->sp++] = v;
}

static u32 Pop(struct Thread *t)
{
    return t->sp ? t->stack[--t->sp] : 0;
}

static u8 Code8(struct Thread *t) { return t->pc < t->size ? t->code[t->pc++] : 0; }
static u16 Code16(struct Thread *t) { u16 v = t->pc + 2 <= t->size ? Le16(t->code + t->pc) : 0; t->pc += 2; return v; }
static s32 Code32(struct Thread *t) { s32 v = t->pc + 4 <= t->size ? (s32)(Le16(t->code + t->pc) | (u32)Le16(t->code + t->pc + 2) << 16) : 0; t->pc += 4; return v; }

// A script id as the zone runs it (scripts.py script_file): its file, its start, its message bank
static bool8 Locate(u16 zone, u16 scriptId, const u8 **code, u32 *size, u32 *pc, u16 *bank)
{
    u32 i, n = Ready() ? Get32(sRangesAt) : 0, index = 0xFFFFFFFF, file = 0xFFFF, msg = 0xFFFF;
    for (i = 0; i < n; i++)
    {
        u32 at = sRangesAt + 4 + 10 * i;
        if (scriptId >= Get16(at) && scriptId <= Get16(at + 2))
        {
            index = scriptId - Get16(at);
            file = Get16(at + 4);
            msg = Get16(at + 8);
            break;
        }
    }
    if (index == 0xFFFFFFFF)
    {
        if (scriptId == 0)
            return FALSE;
        index = scriptId - 1;
        file = ZoneField(zone, Z_SCRIPT);
        msg = ZoneField(zone, Z_MSG);
    }
    *code = FileOf(sFilesAt, file, size);
    if (*code == NULL || 4 * index + 4 > *size || Le16(*code + 4 * index) == 0xFD13)
        return FALSE;
    *pc = 4 * index + 4 + (s32)(Le16(*code + 4 * index) | (u32)Le16(*code + 4 * index + 2) << 16);
    *bank = SYSTEM_BANKS + msg;
    return *pc < *size;
}

static void Finish(struct Thread *t)
{
    t->alive = FALSE;
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    if (sHolding)
    {
        sHolding = FALSE;
        ScriptContext_Enable();
    }
}

static void Message(struct Thread *t, u16 file, u16 msg)
{
    u16 bank = file == MSG_OWN ? t->bank : SYSTEM_BANKS + file;
    sPlaceholder[0] = PLACEHOLDER_BEGIN;
    sPlaceholder[1] = PLACEHOLDER_ID_UNOVA_TEXT;
    sPlaceholder[2] = (bank >> 7) & 0x7F;
    sPlaceholder[3] = bank & 0x7F;
    sPlaceholder[4] = (msg >> 7) & 0x7F;
    sPlaceholder[5] = msg & 0x7F;
    sPlaceholder[6] = EOS;
    if (!IsFieldMessageBoxHidden())
        HideFieldMessageBox();
    ShowFieldMessage(sPlaceholder);
    t->wait = W_MESSAGE;
}

static u8 *Slot(u8 slot)
{
    static u8 *const sVars[3] = {gStringVar1, gStringVar2, gStringVar3};
    return slot < 3 ? sVars[slot] : gUnovaStrVar[slot % UNOVA_STR_VARS];
}

static u16 Species(u16 national)
{
    return national ? NationalPokedexNumToSpecies(national) : SPECIES_NONE;
}

// v (work or constant), p (a work's id), h, w, b: as u16_cmds.h says, read into a[]
static u32 ReadArgs(struct Thread *t, u16 op, u32 *a)
{
    const char *s = op < U3D_CMD_COUNT ? sU3dCmdArgs[op] : "";
    u32 n = 0;
    for (; *s && n < 16; s++, n++)
    {
        switch (*s)
        {
        case 'b': a[n] = Code8(t); break;
        case 'w': a[n] = (u32)Code32(t); break;
        default:  a[n] = Code16(t); break;
        }
    }
    return n;
}

#define V(i) WorkValue((u16)a[i])

static void Step(struct Thread *t)
{
    u32 guard;
    for (guard = 0; guard < 4096 && t->alive && t->wait == W_NONE; guard++)
    {
        u32 a[16], here = t->pc;
        u16 op;
        if (t->pc + 2 > t->size)
        {
            Finish(t);
            return;
        }
        op = Code16(t);
        if (op >= U3D_CMD_COUNT)
        {
            MPNet_Log("u3d vm: op %04X at %X, script ends", op, here);
            Finish(t);
            return;
        }
        // (IfJump / IfCall read their own: b then w)
        ReadArgs(t, op, a);
        switch (op)
        {
        case U3C_Nop:
        case U3C_Dummy:
            break;
        case U3C_Halt:
        case U3C_End:
            Finish(t);
            return;
        case U3C_TimeWait:
            t->waitFrames = 2 * a[0];
            t->wait = W_FRAMES;
            break;
        case U3C_Call:
            Push(t, t->pc);
            t->pc += (s32)a[0];
            break;
        case U3C_Return:
            t->pc = Pop(t);
            break;
        case U3C_Jump:
            t->pc += (s32)a[0];
            break;
        case U3C_IfJump:
        case U3C_IfCall:
        {
            bool8 go;
            static const u8 sCond[6][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 1, 1}, {1, 0, 1}};
            if (a[0] == 0xFF)
                go = Pop(t) != 1;   // (after CompareStack: the "else" taken when it didn't hold)
            else
                go = a[0] < 6 && sCond[a[0]][t->cmp];
            if (go)
            {
                if (op == U3C_IfCall)
                    Push(t, t->pc);
                t->pc += (s32)a[1];
            }
            break;
        }
        case U3C_PushConst:
            Push(t, a[0]);
            break;
        case U3C_PushWorkValue:
            Push(t, V(0));
            break;
        case U3C_PopToWork:
            *WorkPtr(a[0]) = Pop(t);
            break;
        case U3C_PopDiscard:
            Pop(t);
            break;
        case U3C_Add: { u32 x = Pop(t), y = Pop(t); Push(t, y + x); break; }
        case U3C_Sub: { u32 x = Pop(t), y = Pop(t); Push(t, y - x); break; }
        case U3C_Mul: { u32 x = Pop(t), y = Pop(t); Push(t, y * x); break; }
        case U3C_Div: { u32 x = Pop(t), y = Pop(t); Push(t, x ? y / x : 0); break; }
        case U3C_PushEventFlag:
            Push(t, FlagOn(V(0)));
            break;
        case U3C_CompareStack:
        {
            u32 top = Pop(t), below = Pop(t), r;
            switch (a[0])
            {
            case 0: r = below < top; break;
            case 1: r = below == top; break;
            case 2: r = below > top; break;
            case 3: r = below <= top; break;
            case 4: r = below >= top; break;
            case 5: r = below != top; break;
            case 6: r = below == 1 || top == 1; break;
            case 7: r = below == 1 && top == 1; break;
            default: r = 0; break;
            }
            Push(t, r);
            break;
        }
        case U3C_CompareWorkConst:
        {
            u16 x = *WorkPtr(a[0]), y = a[1];
            t->cmp = x < y ? 0 : x == y ? 1 : 2;
            break;
        }
        case U3C_CompareWorks:
        {
            u16 x = *WorkPtr(a[0]), y = *WorkPtr(a[1]);
            t->cmp = x < y ? 0 : x == y ? 1 : 2;
            break;
        }
        case U3C_SetEventFlag:
            FlagTo(V(0), TRUE);
            break;
        case U3C_ResetEventFlag:
            FlagTo(V(0), FALSE);
            break;
        case U3C_CheckEventFlag:
            *WorkPtr(a[1]) = FlagOn(V(0));
            break;
        case U3C_AddWork: *WorkPtr(a[0]) += V(1); break;
        case U3C_SubWork: *WorkPtr(a[0]) -= V(1); break;
        case U3C_MulWork: *WorkPtr(a[0]) *= V(1); break;
        case U3C_DivWork: { u16 d = V(1); if (d) *WorkPtr(a[0]) /= d; break; }
        case U3C_ModWork: { u16 d = V(1); if (d) *WorkPtr(a[0]) %= d; break; }
        case U3C_AndWork: *WorkPtr(a[0]) &= V(1); break;
        case U3C_OrWork: *WorkPtr(a[0]) |= V(1); break;
        case U3C_LoadWorkConst: *WorkPtr(a[0]) = a[1]; break;
        case U3C_LoadWorkValue: *WorkPtr(a[0]) = V(1); break;
        case U3C_CopyWork: *WorkPtr(a[0]) = *WorkPtr(a[1]); break;
        case U3C_ChangeScript:
        case U3C_CallScript:
        {
            const u8 *code;
            u32 size, pc;
            u16 bank;
            if (op == U3C_CallScript)
            {
                // (the script runs to its end and this one goes on: kept on the stack as a return into this file)
                MPNet_Log("u3d vm: CallScript %u (run in place)", a[0]);
            }
            if (Locate(t->zone, a[0], &code, &size, &pc, &bank))
            {
                if (op == U3C_CallScript && code == t->code)
                {
                    Push(t, t->pc);   // (the same file: a call)
                    t->pc = pc;
                }
                else if (op == U3C_ChangeScript || code != t->code)
                {
                    t->code = code, t->size = size, t->pc = pc, t->bank = bank, t->sp = 0;
                }
            }
            break;
        }
        case U3C_CallFieldCommand:   // (the people stop: Emerald's lock did it)
        case U3C_CallScriptEvent:
            break;
        case U3C_PopAllBgm:
            break;
        case U3C_WaitKeyDecide:
        case U3C_WaitKeyAny:
            t->wait = W_KEY;
            break;
        // ---- messages
        case U3C_BalloonWinMsg:          // file, message, person, kind, window
        case U3C_BalloonWinMsgTarget:    // file, message, kind, window
            Message(t, V(0), V(1));
            break;
        case U3C_BalloonWinMsgGender:    // file, (he, she), person...
            Message(t, V(0), gSaveBlock2Ptr->playerGender == FEMALE ? V(2) : V(1));
            break;
        case U3C_BalloonWinMsgSkipArg:   // file, (unused), message, ...
            Message(t, V(0), V(2));
            break;
        case U3C_InfoSysWinMsg:
        case U3C_InfoSysWinMsgNow:
        case U3C_PlainWinMsg:
        case U3C_PlainWinMsgFrame1:
            Message(t, MSG_OWN, V(0));
            break;
        case U3C_BgWinMsg:   // (a sign's board, a special window: it waits for a key itself, WaitBgWin)
        case U3C_SpWinMsg:
            Message(t, MSG_OWN, a[0]);
            t->wait = W_MESSAGE_KEY;
            break;
        case U3C_CloseInfoSysWin:
        case U3C_ClosePlainWin:
        case U3C_CloseTalkWin:
        case U3C_CloseAllWins:
        case U3C_CloseBgWin:
        case U3C_CloseSpWin:
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            break;
        case U3C_YesNoWin:
            DisplayYesNoMenuDefaultYes();
            t->result = WorkPtr(a[0]);
            t->wait = W_YESNO;
            break;
        case U3C_BufferPlayerName:
            StringCopy(Slot(a[0]), gSaveBlock2Ptr->playerName);
            break;
        case U3C_BufferSpeciesName:
            GetSpeciesName(Slot(a[0]), Species(V(1)));
            break;
        case U3C_BufferNumber:
            ConvertIntToDecimalStringN(Slot(a[0]), V(1), STR_CONV_MODE_LEFT_ALIGN, 6);
            break;
        // ---- people
        case U3C_TurnTargetMmdlToPlayer:
        {
            struct ObjectEvent *o = ObjectOf(t->target);
            if (t->target != 0xFFFF && o)
                ObjectEventTurn(o, GetOppositeDirection(gObjectEvents[gPlayerAvatar.objectEventId].facingDirection));
            break;
        }
        case U3C_StartMmdlAnime:
            StartMoves(V(0), t->code + t->pc + (s32)a[1], t->code + t->size);
            break;
        case U3C_WaitAnimeTasks:
            t->wait = W_MOVES;
            break;
        case U3C_DeleteMmdl:
            RemoveObjectEventByLocalIdAndMap(LocalId(V(0)), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            break;
        case U3C_InitMmdlGridPosition:   // person, x, y, z, direction
        {
            u16 obj = V(0);
            struct ObjectEvent *o = ObjectOf(obj);
            s32 x = MapX(V(1)), z = MapZ(V(3));
            if (obj != OBJ_PLAYER)
                SetObjEventTemplateCoords(LocalId(obj), x, z);
            if (o)
            {
                TryMoveObjectEventToMapCoords(LocalId(obj), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, x, z);
                ObjectEventTurn(o, BlackToDir(V(4)));
            }
            break;
        }
        case U3C_GetPlayerDir:
            *WorkPtr(a[0]) = DirToBlack(gObjectEvents[gPlayerAvatar.objectEventId].facingDirection);
            break;
        case U3C_GetPlayerGridPos:
        {
            s32 i = U3D_MapIndex();
            *WorkPtr(a[0]) = gSaveBlock1Ptr->pos.x + (i < 0 ? 0 : 32 * U3D_MapCellX(i));
            *WorkPtr(a[1]) = gSaveBlock1Ptr->pos.y + (i < 0 ? 0 : 32 * U3D_MapCellZ(i));
            break;
        }
        // ---- sound
        case U3C_PlaySe:
            if (V(0) == 547)   // (the talk sound)
                PlaySE(SE_SELECT);
            break;
        // ---- screen
        case U3C_SCREENEFFECTS_FadeOutSlow:
        case U3C_SCREENEFFECTS_FadeOutSlowAltScreens:
            BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeOutFast:
        case U3C_SCREENEFFECTS_FadeOutFastAltScreens:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInSlow:
        case U3C_SCREENEFFECTS_FadeInSlowAgain:
        case U3C_SCREENEFFECTS_FadeInSlowAltScreens:
            BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInFast:
        case U3C_SCREENEFFECTS_FadeInFastAltScreens:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_WaitBrightness:
            t->wait = W_FADE;
            break;
        default:
            if (!(sLogged[op / 16] & (1 << (op % 16))))
            {
                sLogged[op / 16] |= 1 << (op % 16);
                MPNet_Log("u3d vm: %03X not played yet (read past)", op);
                printf("[u3d] command %03X not played yet\n", op);
            }
            break;
        }
    }
}

static void Wait(struct Thread *t)
{
    switch (t->wait)
    {
    case W_FRAMES:
        if (t->waitFrames == 0 || --t->waitFrames == 0)
            t->wait = W_NONE;
        break;
    case W_MESSAGE:
        if (!IsTextPrinterActive(0))
            t->wait = W_NONE;
        break;
    case W_MESSAGE_KEY:
        if (!IsTextPrinterActive(0))
            t->wait = W_KEY;
        break;
    case W_KEY:
        if (JOY_NEW(A_BUTTON | B_BUTTON))
            t->wait = W_NONE;
        break;
    case W_MOVES:
        if (MovesDone())
            t->wait = W_NONE;
        break;
    case W_FADE:
        if (!gPaletteFade.active)
            t->wait = W_NONE;
        break;
    case W_YESNO:
    {
        s8 r = Menu_ProcessInputNoWrapClearOnChoose();
        if (r != MENU_NOTHING_CHOSEN)
        {
            if (t->result)
                *t->result = r == 0 ? 0 : 1;   // (Black: 0 yes, 1 no)
            t->wait = W_NONE;
        }
        break;
    }
    }
}

static void Task_Vm(u8 taskId)
{
    struct Thread *t = &sThread;
    if (!t->alive)
    {
        DestroyTask(taskId);
        return;
    }
    if (t->wait != W_NONE)
        Wait(t);
    if (t->wait == W_NONE)
        Step(t);
}

// Start Black's script SCRIPT_ID of the player's zone (TARGET: the person talked to, Black's id; 0xFFFF none)
static bool8 Run(u16 scriptId, u16 target, bool8 hold)
{
    struct Thread *t = &sThread;
    u16 zone = CurrentZone();
    if (t->alive || zone == 0xFFFF || !Locate(zone, scriptId, &t->code, &t->size, &t->pc, &t->bank))
        return FALSE;
    t->sp = 0, t->cmp = 0, t->wait = W_NONE, t->alive = TRUE, t->zone = zone, t->target = target, t->result = NULL;
    memset(sLocal, 0, sizeof(sLocal));
    sLocal[0x11] = target;   // (0x8011: the person talked to, SCRIPT_WORK_SetTargetObj)
    memset(sMoving, 0, sizeof(sMoving));
    sHolding = hold;
    MPNet_Log("u3d vm: zone %u script %u", zone, scriptId);
    CreateTask(Task_Vm, 80);
    return TRUE;
}

// ------------------------------------------------------------------------------------------------ the field's hooks
// (a special that ends at once still lets its waitstate stop the script first: the enable comes a frame later)
static void Task_EnableNext(u8 taskId)
{
    ScriptContext_Enable();
    DestroyTask(taskId);
}

static void NothingToRun(void)
{
    CreateTask(Task_EnableNext, 80);
}

// special U3D_VmTalk: the person the player talked to (gSpecialVar_LastTalked) runs their script (Black's event data)
void U3D_VmTalk(void)
{
    u16 local = gSpecialVar_LastTalked;
    const u8 *p = local ? Person(CurrentZone(), local - 1) : NULL;
    if (p == NULL || !Run(Le16(p + 10), Le16(p), TRUE))
        NothingToRun();
}

// special U3D_VmSign: the sign / spot the player faces
void U3D_VmSign(void)
{
    u32 size, i;
    const u8 *e = Events(CurrentZone(), &size);
    s16 x, y;
    s32 m = U3D_MapIndex();
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    x -= MAP_OFFSET, y -= MAP_OFFSET;
    for (i = 0; e && size >= 8 && i < e[4]; i++)
    {
        const u8 *b = e + 8 + EV_BG * i;
        s32 bx = (s32)(Le16(b + 8) | Le16(b + 10) << 16), bz = (s32)(Le16(b + 12) | Le16(b + 14) << 16);
        if (m >= 0)
            bx -= 32 * U3D_MapCellX(m), bz -= 32 * U3D_MapCellZ(m);
        if (bx == x && bz == y && Run(Le16(b), 0xFFFF, TRUE))
            return;
    }
    NothingToRun();
}

// special U3D_VmPos: the trigger spot the player stepped on (its work held its value: the coord event's var)
void U3D_VmPos(void)
{
    u32 size, i;
    const u8 *e = Events(CurrentZone(), &size);
    s32 m = U3D_MapIndex();
    s32 x = gSaveBlock1Ptr->pos.x, y = gSaveBlock1Ptr->pos.y;
    if (m >= 0)
        x += 32 * U3D_MapCellX(m), y += 32 * U3D_MapCellZ(m);
    for (i = 0; e && size >= 8 && i < e[7]; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * e[5] + EV_CONNECT * e[6] + EV_POS * i;
        u16 px = Le16(p + 10), pz = Le16(p + 12), w = Le16(p + 14), h = Le16(p + 16);
        if (x >= px && x < px + (w ? w : 1) && y >= pz && y < pz + (h ? h : 1)
         && WorkValue(Le16(p + 4)) == Le16(p + 2) && Run(Le16(p), 0xFFFF, TRUE))
            return;
    }
    NothingToRun();
}

// A map's own scripts (Black's init file): on loading it the field-init script (kind 4) runs to its end at once
static const u8 *InitFile(u16 zone, u32 *size)
{
    u16 f = ZoneField(zone, Z_INIT);
    return f == 0xFFFF ? (*size = 0, NULL) : FileOf(sFilesAt, f, size);
}

static u16 InitScript(u16 zone, u16 kind)
{
    u32 size, o;
    const u8 *d = InitFile(zone, &size);
    for (o = 0; d && o + 6 <= size && Le16(d + o) != 0; o += 6)
        if (Le16(d + o) == kind)
            return Le16(d + o + 2);
    return 0xFFFF;
}

static void RunNow(u16 scriptId)
{
    struct Thread *t = &sThread;
    u32 guard;
    if (scriptId == 0xFFFF || !Run(scriptId, 0xFFFF, FALSE))
        return;
    for (guard = 0; guard < 64 && t->alive; guard++)
    {
        t->wait = W_NONE;   // (no waits on loading: it places people and sets works)
        Step(t);
    }
    t->alive = FALSE;
}

// ON_TRANSITION of every UNOVA map (data/scripts/unova.inc): the zone-change and field-init scripts
void U3D_VmMapLoad(void)
{
    u16 zone = CurrentZone();
    static u16 sLastZone = 0xFFFF;
    if (zone == 0xFFFF || !Ready())
        return;
    if (zone != sLastZone)
        RunNow(InitScript(zone, 2));
    sLastZone = zone;
    RunNow(InitScript(zone, 4));
}

// script.c TryRunOnFrameMapScript, on an UNOVA map: a scene the zone's table starts (its work holds its value), held by
// Unova_EventScript_Scene until it ends
extern const u8 Unova_EventScript_Scene[];
bool8 U3D_Vm_SceneStart(void)
{
    u32 size, o, at;
    u16 zone = CurrentZone();
    const u8 *d;
    if (zone == 0xFFFF || sThread.alive || !Ready())
        return FALSE;
    d = InitFile(zone, &size);
    for (o = 0; d && o + 6 <= size && Le16(d + o) != 0; o += 6)
    {
        if (Le16(d + o) != 1)
            continue;
        at = o;
        while (at + 2 <= size && Le16(d + at) != 0)
            at += 6;
        at += 2 + (Le16(d + o + 2) | Le16(d + o + 4) << 16);
        for (; at + 6 <= size && Le16(d + at) != 0; at += 6)
            if (WorkValue(Le16(d + at)) == Le16(d + at + 2))
            {
                if (!Run(Le16(d + at + 4), 0xFFFF, TRUE))
                    return FALSE;
                ScriptContext_SetupScript(Unova_EventScript_Scene);
                return TRUE;
            }
    }
    return FALSE;
}

// special U3D_VmHold (Unova_EventScript_Scene): its waitstate lasts until the scene's script ends
void U3D_VmHold(void)
{
    if (!sThread.alive)
        NothingToRun();
}

bool8 U3D_VmRunning(void)
{
    return sThread.alive;
}
