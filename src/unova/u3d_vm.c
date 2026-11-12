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
#include "constants/songs.h"
#include "u3d_cmds.h"
#include "u3d_maps.h"
#include "script_pokemon_util.h"
#include "random.h"
#include "money.h"
#include "mp_region.h"
#include "rtc.h"
#include "load_save.h"
#include "data.h"
#include "party_menu.h"
#include "mp.h"
#include "battle.h"
#include "constants/battle.h"
#include "pokemon_storage_system.h"
#include "menu.h"
#include "script_menu.h"
#include "platform.h"
#include "save.h"
#include "field_screen_effect.h"
#include "constants/flags.h"
#include "item.h"
#include "battle_setup.h"
#include "constants/items.h"
#include "constants/species.h"
void MP_StarterTable_Open(const u16 *species, MainCallback back);

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
static u32 sFilesAt, sRangesAt, sZonesAt, sEventsAt, sTrainersAt, sTeamsAt, sSongKindsAt, sEncountAt, sTrMsgAt, sEyeSongsAt, sWarpsAt, sCallsAt;

static u32 Get32(u32 at) { return at + 4 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 | sPack[at + 2] << 16 | (u32)sPack[at + 3] << 24 : 0; }
static u16 Get16(u32 at) { return at + 2 <= sPackSize ? sPack[at] | sPack[at + 1] << 8 : 0; }

static u32 ZoneSize(void) { return Get32(4) >= 4 ? 18 : 16; }   // (a zone's record: 9 u16 from version 4)

// (a block's files follow their table, each padded to 4 bytes: the next block starts after the last one)
static u32 BlockEnd(u32 at)
{
    u32 n = Get32(at), end = at + 4 + 8 * n;
    if (n)
        end = Get32(at + 4 + 8 * (n - 1)) + Get32(at + 8 + 8 * (n - 1));
    return (end + 3) & ~3;
}

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
     || Get32(4) < 1 || Get32(4) > 7)
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
    sRangesAt = BlockEnd(sFilesAt);
    sZonesAt = sRangesAt + 4 + 10 * Get32(sRangesAt);
    sEventsAt = (sZonesAt + 4 + ZoneSize() * Get32(sZonesAt) + 3) & ~3;
    if (Get32(4) >= 2)   // (version 2: Black's trainers and their teams)
    {
        sTrainersAt = BlockEnd(sEventsAt);
        sTeamsAt = BlockEnd(sTrainersAt);
        if (Get32(4) >= 3)   // (version 3: each trainer's battle song kind)
            sSongKindsAt = BlockEnd(sTeamsAt);
        if (Get32(4) >= 4)   // (version 4: the wild POKéMON)
            sEncountAt = (sSongKindsAt + 4 + Get32(sSongKindsAt) + 3) & ~3;
        if (Get32(4) >= 5)   // (version 5: the trainers' words, the classes' encounter songs)
        {
            sTrMsgAt = BlockEnd(sEncountAt);
            sEyeSongsAt = BlockEnd(sTrMsgAt);
        }
        if (Get32(4) >= 6)   // (version 6: Black's warp points)
            sWarpsAt = sEyeSongsAt + 4 + 4 * Get32(sEyeSongsAt);
        if (Get32(4) >= 7)   // (version 7: the Xtransceiver's calls)
            sCallsAt = (sWarpsAt + 4 + 20 * Get32(sWarpsAt) + 3) & ~3;
    }
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

// Black's trainer ID's record (20 bytes) and team (u3d_battle.c); NULL without them
const u8 *U3D_Vm_Trainer(u16 id, u32 *size)
{
    return Ready() && sTrainersAt ? FileOf(sTrainersAt, id, size) : (*size = 0, NULL);
}

const u8 *U3D_Vm_TrainerTeam(u16 id, u32 *size)
{
    return Ready() && sTeamsAt ? FileOf(sTeamsAt, id, size) : (*size = 0, NULL);
}

// Black's trainer ID's battle song kind (tools/unova/build_scripts.py song_kinds: 0 a trainer .. 8 a Subway Boss)
u8 U3D_Vm_TrainerSongKind(u16 id)
{
    return Ready() && sSongKindsAt && id < Get32(sSongKindsAt) ? sPack[sSongKindsAt + 4 + id] : 0;
}

enum { Z_SCRIPT, Z_INIT, Z_MSG, Z_EVENT, Z_MATRIX, Z_PARENT, Z_TYPE, Z_PLACE, Z_ENCOUNT };
static u16 ZoneField(u16 zone, u32 field)
{
    if (!Ready() || zone >= Get32(sZonesAt))
        return 0xFFFF;
    if (field == Z_ENCOUNT && Get32(4) < 4)
        return 0xFFFF;
    return Get16(sZonesAt + 4 + ZoneSize() * zone + 2 * field);
}

// The zone's wild POKéMON row for SEASON (0xE8 bytes; tools/unova/build_scripts.py), NULL none
const u8 *U3D_Vm_WildRow(u16 zone, u8 season)
{
    u32 size;
    u16 id;
    const u8 *d;
    if (!Ready() || !sEncountAt)
        return NULL;
    id = ZoneField(zone, Z_ENCOUNT);
    if (id == 0xFFFF)
        return NULL;
    d = FileOf(sEncountAt, id, &size);
    if (d == NULL || (size != 0xE8 && size != 4 * 0xE8))
        return NULL;
    return size == 0xE8 ? d : d + 0xE8 * (season & 3);
}


// ------------------------------------------------------------------------------------------------ Black's event work
// (zonedata.c: works 0x4000 on, then the flags 1-0xB5F; 0x4000+ flags are temporary, 0x8000+ works the script's own)
#define WORKS 0x140
#define FLAG_BYTES 0x16C
static struct { u16 work[WORKS]; u8 flags[FLAG_BYTES]; u8 began, badges, cgear; } sWork;   // (began: Black's new-game
                                                // script has run; badges: UNOVA's, a bit each; cgear: the C-GEAR got)
u8 *U3D_Vm_BadgeFlags(void)
{
    return &sWork.badges;
}
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

// The object event of a local id: on the player's map, else on any map (Black's people live by zone, and a scene's
// people walk with the player over a map's edge, or were added on the map before: AddMmdl)
static u8 ObjEventId(u8 localId)
{
    u8 id;
    // (while a map loads, its people aren't set out yet: those in the table are the last map's, sprites gone; a
    // script moves their templates then: InitMmdlGridPosition)
    if (gMain.callback2 != CB2_Overworld && gMain.callback2 != CB2_OverworldBasic)
        return OBJECT_EVENTS_COUNT;
    if (!TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
        return id;
    // (only a scene's own people, Black's object ids from 0x80: AddMmdl's; never a person of the map left behind,
    // whose sprite may be gone as the next map loads)
    for (id = 0; localId > 0x80 && id < OBJECT_EVENTS_COUNT; id++)
        if (gObjectEvents[id].active && !gObjectEvents[id].isPlayer && gObjectEvents[id].localId == localId
         && U3D_IsUnovaGroup(gObjectEvents[id].mapGroup) && gSprites[gObjectEvents[id].spriteId].inUse)
            return id;
    return OBJECT_EVENTS_COUNT;
}

static struct ObjectEvent *ObjectOf(u16 obj)
{
    u8 id = ObjEventId(LocalId(obj));
    return id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
}

// event_object_movement.c: a person of the map hidden by Black's flag (its event data's vanish flag)
// People a zone's script set somewhere else (Black's SetNpcGridPos: EVENTDATA_SYSTEM_SetNpcGridPos, CHEREN waiting at
// NACRENE's gate): Black's tile and direction, kept while the player is in the zone and put on each of its maps
#define NPC_AT_MAX 16
static struct { u8 local, dir; u16 x, z; } sNpcAt[NPC_AT_MAX];
static u8 sNpcAtCount;

static bool8 NpcAt(u8 local, u16 *x, u16 *z)
{
    u32 i;
    for (i = 0; i < sNpcAtCount; i++)
        if (sNpcAt[i].local == local)
        {
            *x = sNpcAt[i].x, *z = sNpcAt[i].z;
            return TRUE;
        }
    return FALSE;
}

// People a script deleted that have no flag of their own (CUT's trees, a boulder...): gone until the map is loaded
// again, as Black's come back on re-entering (the game would put them back the moment they come into view)
static u8 sGone[16], sGoneCount;

static void Gone(u8 local, bool8 gone)
{
    u32 i;
    for (i = 0; i < sGoneCount; i++)
        if (sGone[i] == local)
        {
            if (!gone)
                sGone[i] = sGone[--sGoneCount];
            return;
        }
    if (gone && sGoneCount < ARRAY_COUNT(sGone))
        sGone[sGoneCount++] = local;
}

bool8 U3D_Vm_ObjectHidden(u8 localId)
{
    const u8 *p;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || localId == 0 || localId >= OBJ_EVENT_ID_PLAYER)
        return FALSE;
    {
        u32 i;
        for (i = 0; i < sGoneCount; i++)
            if (sGone[i] == localId)
                return TRUE;
    }
    p = Person(CurrentZone(), localId - 1);
    if (p == NULL)
        return FALSE;
    if (Le16(p + 8) != 0 && FlagOn(Le16(p + 8)))   // (MmdlHeader: id, code, move, type, flag)
        return TRUE;
    {
        // (a person whose spot is on another of the zone's maps: tools/unova/gen_maps.py keeps its place in the list)
        s32 m = U3D_MapIndex(), x = Le16(p + 28), z = Le16(p + 30);
        u16 ax, az;
        if (NpcAt(localId, &ax, &az))
            x = ax, z = az;
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
static u16 sPendingScript;                 // Black's SetProcAction: a script to run next
static u8 sSubText[400] = {EOS}, sSubCount;   // Black's side windows' words (AddSubWinMsg; empty: EOS, not 0, a space)

// ---- people a script added (Black's AddMmdl): kept until the map changes or the script deletes them, and put on the
// field once it is up (a zone's field-init script adds them while the map loads, before the map's people are set out)
static struct { u8 local, dir; u16 code, x, z; } sAdded[8];
static u8 sAddedCount;

static u8 BlackToDir(u16 d);
static s32 MapX(u16 x);
static s32 MapZ(u16 z);

static void SpawnAdded(void)
{
    u32 i;
    if (gMain.callback2 != CB2_Overworld)
        return;
    for (i = 0; i < sAddedCount; i++)
    {
        u8 id;
        if (ObjEventId(sAdded[i].local) < OBJECT_EVENTS_COUNT)
            continue;
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_UNOVA_FIRST + sAdded[i].code, MOVEMENT_TYPE_FACE_DOWN,
                                                  sAdded[i].local, MapX(sAdded[i].x) + MAP_OFFSET,
                                                  MapZ(sAdded[i].z) + MAP_OFFSET, 3);
        if (id < OBJECT_EVENTS_COUNT)
            ObjectEventTurn(&gObjectEvents[id], BlackToDir(sAdded[i].dir));
    }
}

static void AddPerson(u8 local, u16 code, u16 x, u16 z, u16 dir)
{
    u32 i;
    for (i = 0; i < sAddedCount && sAdded[i].local != local; i++)
        ;
    if (i == sAddedCount && sAddedCount < ARRAY_COUNT(sAdded))
        sAddedCount++;
    if (i < ARRAY_COUNT(sAdded))
        sAdded[i].local = local, sAdded[i].code = code, sAdded[i].x = x, sAdded[i].z = z, sAdded[i].dir = dir;
    SpawnAdded();
}

static void ForgetPerson(u8 local)
{
    u32 i;
    for (i = 0; i < sAddedCount; i++)
        if (sAdded[i].local == local)
        {
            sAdded[i] = sAdded[--sAddedCount];
            return;
        }
}

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
    // (a buffer per object event, never shared: the game's movement task keeps pointing into a finished list, so a
    // list written over another's buffer would walk that person too)
    u8 slot, localId = LocalId(obj);
    u32 n = 0;
    u8 *out;
    if ((slot = ObjEventId(localId)) >= OBJECT_EVENTS_COUNT)
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
    ScriptMovement_StartObjectMovementScript(localId, gObjectEvents[slot].mapNum, gObjectEvents[slot].mapGroup, out);
}

static bool8 MovesDone(void)
{
    u8 slot;
    bool8 done = TRUE;
    for (slot = 0; slot < OBJECT_EVENTS_COUNT; slot++)
    {
        if (sMoving[slot] == 0)
            continue;
        if (!gObjectEvents[slot].active || gObjectEvents[slot].localId != sMoving[slot]
         || ScriptMovement_IsObjectMovementFinished(sMoving[slot], gObjectEvents[slot].mapNum, gObjectEvents[slot].mapGroup))
            sMoving[slot] = 0;
        else
            done = FALSE;
    }
    return done;
}

// trainers' eyes (the end of this file)
static struct EyeHit { u16 obj, trainer; u8 local, distance, dir; } sEye[2];
static u8 sEyeKind;   // (0 one trainer, 1 two who each saw the player, 2 a pair: GetTrainerHitKind)
static u16 TrainerOfScript(u16 script);
static void StartEye(u8 slot);
static u16 TrainerWords(u16 trainer, u16 kind);
static u16 EyeSong(u16 trainer);

// ------------------------------------------------------------------------------------------------ the VM
enum { W_NONE, W_FRAMES, W_MESSAGE, W_MESSAGE_KEY, W_KEY, W_MOVES, W_FADE, W_YESNO, W_CLOSE, W_SCREEN, W_SONG_THEN_POP, W_SE, W_CAMERA, W_CRY, W_MENU };

// ---- Black's songs: its sound archive's sequence numbers as the game's song slots (tools/kaisong/unova_songs.py);
// a pushed song comes back when the one played over it ends (a fanfare: Black's PushAndPlayBgm, StartWaitBgmThenPop)
#include "../data/unova_songs.h"
static u16 sSongStack[4];
static u8 sSongDepth;

static u16 SongOf(u16 seq)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUnovaSongs); i++)
        if (sUnovaSongs[i].seq == seq)
            return sUnovaSongs[i].song;
    return MUS_NONE;
}

static void PushSong(void)
{
    if (sSongDepth < ARRAY_COUNT(sSongStack))
        sSongStack[sSongDepth++] = GetCurrentMapMusic();
}

static void PopSong(void)
{
    if (sSongDepth)
        PlayNewMapMusic(sSongStack[--sSongDepth]);
}
// The screens Black's scripts open, from Unova_EventScript_Screen (data/scripts/unova.inc)
enum { SCREEN_NONE, SCREEN_STARTER, SCREEN_BATTLE, SCREEN_CATCH_DEMO, SCREEN_WILD, SCREEN_PARTY };
static u8 sScreen;
static u16 sScreenVar, sPartySlotWork;
static u8 sWildResult = 2;
#define STACK 48

struct Frame { const u8 *code; u32 size, pc; u16 bank; u8 sp; };   // (a script that called another: CallScript)
#define FRAMES 6

struct Thread
{
    struct Frame frames[FRAMES];
    u8 depth;
    const u8 *code;
    u32 size, pc;
    u32 stack[STACK];
    u8 sp, cmp, wait;
    bool8 alive, box;
    u16 zone, bank, waitFrames, target;
    u16 *result;
    u16 script;   // (the script it was started as: a trainer's is 3000 + its number, GetScriptTrainerId)
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

static void Task_Vm(u8 taskId);

// Black's script wants a screen of the game's: the field script takes it from here (U3D_VmOpenScreen) and the VM
// goes on when it's back (U3D_VmResume)
static void OpenScreen(struct Thread *t, u8 screen, u16 var)
{
    sScreen = screen;
    sScreenVar = var;
    t->wait = W_SCREEN;
    if (sHolding)
        ScriptContext_Enable();
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

static void MessageInBank(struct Thread *t, u16 bank, u16 msg);

static void Message(struct Thread *t, u16 file, u16 msg)
{
    MessageInBank(t, file == MSG_OWN ? t->bank : SYSTEM_BANKS + file, msg);
}

static void MessageInBank(struct Thread *t, u16 bank, u16 msg)
{
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

static u8 *Slot(u8 slot)   // (Black's word slots, UNOVA's own: the game's battles rewrite gStringVar1-3)
{
    return gUnovaStrVar[slot % UNOVA_STR_VARS];
}

// Black's item numbers as the game's (tools/unova/gen_items.py: by name; ITEM_NONE where the game has no such item yet)
#include "../data/unova_items.h"
static u16 Item(u16 black)
{
    return black < ARRAY_COUNT(sUnovaItems) ? sUnovaItems[black] : ITEM_NONE;
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

// A POKéMON knows a move, Black's numbers being the game's (CUT 15...): Kai's held HMs count (scrcmd.c checkpartymove)
static bool8 PartyMonHasMove(struct Pokemon *mon, u16 move)
{
    return GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
        && (MonKnowsMove(mon, move) || MP_HeldHmMove(mon) == move);
}

// Black's script menus (MSGWIN_CmdInitMenu..., AddMenuItem, StartMenu): items from the script's own words (InitMenu:
// story bank 262, Black's shared menu words), a value each; the choice's value into the result work, B 0xFFFE where
// the menu may be cancelled. Drawn as the game's multichoice windows, its right and bottom edges the YES / NO window's
#define MENU_MAX 16
static struct
{
    u16 bank, values[MENU_MAX], *result;
    u8 count, cursor, cancel, window;
    u8 text[MENU_MAX][48];
    struct MenuAction actions[MENU_MAX];
} sMenu;

static void MenuInit(u16 bank, u16 cursor, u8 cancel, u16 *result)
{
    sMenu.bank = bank, sMenu.count = 0, sMenu.cursor = cursor, sMenu.cancel = cancel, sMenu.result = result;
}

static void MenuAdd(u16 msg, u16 value)
{
    if (sMenu.count >= MENU_MAX)
        return;
    StringExpandPlaceholders(sMenu.text[sMenu.count], U3D_Text(sMenu.bank, msg));
    sMenu.actions[sMenu.count].text = sMenu.text[sMenu.count];
    sMenu.actions[sMenu.count].func.void_u8 = NULL;
    sMenu.values[sMenu.count++] = value;
}

static void MenuStart(struct Thread *t)
{
    s32 i, width = 0;
    u8 w, top;
    if (sMenu.count == 0)
        return;
    for (i = 0; i < sMenu.count; i++)
        width = max(width, GetStringWidth(FONT_NORMAL, sMenu.text[i], 0));
    w = (width + 9 + 7) / 8 + 1;   // (the cursor's room)
    top = sMenu.count * 2 + 1 < 13 ? 13 - sMenu.count * 2 : 1;
    sMenu.window = CreateWindowFromRect(w < 25 ? 26 - w : 1, top, w, sMenu.count * 2);
    SetStandardWindowBorderStyle(sMenu.window, FALSE);
    PrintMenuTable(sMenu.window, sMenu.count, sMenu.actions);
    InitMenuInUpperLeftCornerNormal(sMenu.window, sMenu.count, sMenu.cursor < sMenu.count ? sMenu.cursor : 0);
    ScheduleBgCopyTilemapToVram(0);
    t->wait = W_MENU;
}

static bool8 MenuInput(void)
{
    s8 r = Menu_ProcessInput();
    if (r == MENU_NOTHING_CHOSEN || (r == MENU_B_PRESSED && !sMenu.cancel))
        return FALSE;
    if (sMenu.result)
        *sMenu.result = r == MENU_B_PRESSED ? 0xFFFE : sMenu.values[r];
    ClearStdWindowAndFrameToTransparent(sMenu.window, TRUE);
    RemoveWindow(sMenu.window);
    return TRUE;
}

static void SetWhiteoutPoint(u16 warpId);

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
            if (t->depth)   // (a called script's end: back to the one that called it)
            {
                struct Frame *f = &t->frames[--t->depth];
                t->code = f->code, t->size = f->size, t->pc = f->pc, t->bank = f->bank, t->sp = f->sp;
                break;
            }
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
        case U3C_ChangeScript:   // (another script in this one's place)
        case U3C_CallScript:     // (another script run to its end, then this one goes on: SCRIPT_WORK_WaitChildScript)
        {
            const u8 *code;
            u32 size, pc;
            u16 bank;
            if (!Locate(t->zone, a[0], &code, &size, &pc, &bank))
            {
                MPNet_Log("u3d vm: no script %u", a[0]);
                break;
            }
            if (op == U3C_CallScript)
            {
                struct Frame *f;
                if (t->depth >= FRAMES)
                    break;
                f = &t->frames[t->depth++];
                f->code = t->code, f->size = t->size, f->pc = t->pc, f->bank = t->bank, f->sp = t->sp;
            }
            t->code = code, t->size = size, t->pc = pc, t->bank = bank;
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
        case U3C_PROC_CallValueScreen:   // the Xtransceiver's call N: each step's words, a key between (ov174)
        {
            static s16 sCallStep = -1;
            u32 size, k;
            const u8 *c = sCallsAt ? FileOf(sCallsAt, V(0), &size) : NULL;
            if (gPaletteFade.active)   // (the scene's fade out first)
            {
                t->pc = here, t->wait = W_FADE;
                return;
            }
            if (sCallStep < 0)
            {
                // (the call over the field, in place of Black's call screen: the field faded back in for it)
                sCallStep = 0;
                if (gPaletteFade.y != 0)
                {
                    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
                    t->pc = here, t->wait = W_FADE;
                    return;
                }
            }
            for (k = 8 + 4 * sCallStep; c && k + 4 <= size && c[k] != 0; k += 4, sCallStep++)
                if (c[k] == 1)
                {
                    MPNet_Log("u3d vm: call %u, step %d", V(0), sCallStep);
                    MessageInBank(t, 0xBF + V(0), Le16(c + k + 2));   // (system bank 0xBF + N)
                    t->wait = W_MESSAGE_KEY;
                    sCallStep++;
                    t->pc = here;   // (back here for the next step)
                    return;
                }
            sCallStep = -1;
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);   // (dark again: the scene fades in after)
            t->wait = W_FADE;
            break;
        }
        // ---- warps (more of Black's: the 3D one's tile is x, z; y the height)
        case U3C_WarpToPosition3D:   // zone, x, y, z, direction
        case U3C_WarpToPositionEvent:   // zone, x, z
        {
            u8 group, num;
            s16 x, y;
            u16 wz = a[0], wx = a[1], wzz = op == U3C_WarpToPosition3D ? a[3] : a[2];
            if (U3D_MapOfZoneTile(wz, wx, wzz, &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: warp to zone %u (%d.%d at %d,%d)", wz, group, num, x, y);
                sHolding = FALSE;
                t->alive = FALSE;
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            break;
        }
        // ---- battles
        case U3C_PROC_FLD_StartScriptedWildBattle:   // species, level, flags (a legend's, a static POKeMON's)
            CreateScriptedWildMon(Species(V(0)), V(1), ITEM_NONE);
            OpenScreen(t, SCREEN_WILD, 0);
            break;
        case U3C_WORK_CmdCheckLastBattleResultB:
            *WorkPtr(a[0]) = U3D_BattleResult() != 2;
            break;
        case U3C_WORK_CmdEndScriptWhiteOutB:
            sHolding = FALSE;
            t->alive = FALSE;
            return;
        case U3C_PROC_FLD_StoreBattleResult:
            *WorkPtr(a[0]) = sWildResult;
            break;
        // ---- the party
        case U3C_POKEMON_FLD_SelectPartyMon:   // slot, result, mode
            sPartySlotWork = a[0];
            OpenScreen(t, SCREEN_PARTY, a[1]);
            break;
        case U3C_PROC_CheckMonIsEgg:   // result, slot
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE && GetMonData(&gPlayerParty[V(1)], MON_DATA_IS_EGG);
            break;
        case U3C_PROC_GetMonSpeciesOrEgg:   // result, slot (national; an egg 0x28F, Black's EGG)
        {
            u16 sp = V(1) < PARTY_SIZE ? GetMonData(&gPlayerParty[V(1)], MON_DATA_SPECIES_OR_EGG) : SPECIES_NONE;
            *WorkPtr(a[0]) = sp == SPECIES_EGG ? 0x28F : sp ? SpeciesToNationalPokedexNum(sp) : 0;
            break;
        }
        case U3C_PROC_GetMonFriendship:   // result, slot
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE ? GetMonData(&gPlayerParty[V(1)], MON_DATA_FRIENDSHIP) : 0;
            break;
        case U3C_PROC_GetMonMove:   // result, slot, index
            *WorkPtr(a[0]) = V(1) < PARTY_SIZE && V(2) < MAX_MON_MOVES ? GetMonData(&gPlayerParty[V(1)], MON_DATA_MOVE1 + V(2)) : 0;
            break;
        case U3C_POKEMON_FLD_GetMonIvStat:   // slot, stat, result
            *WorkPtr(a[2]) = V(0) < PARTY_SIZE && V(1) < NUM_STATS ? GetMonData(&gPlayerParty[V(0)], MON_DATA_HP_IV + V(1)) : 0;
            break;
        case U3C_PROC_FindFirstPartyIndex:   // result, mode (1 not an egg, 2 able to battle)
        {
            u32 i;
            *WorkPtr(a[0]) = 0;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                struct Pokemon *mon = &gPlayerParty[i];
                if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                    continue;
                if (V(1) == 2 && GetMonData(mon, MON_DATA_HP) == 0)
                    continue;
                *WorkPtr(a[0]) = i;
                break;
            }
            break;
        }
        case U3C_PROC_FindPartyMonBySpecies:   // species, found, slot
        {
            u32 i;
            u16 want = Species(V(0));
            *WorkPtr(a[1]) = 0;
            for (i = 0; i < PARTY_SIZE; i++)
                if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == want)
                {
                    *WorkPtr(a[1]) = 1, *WorkPtr(a[2]) = i;
                    break;
                }
            break;
        }
        case U3C_PROC_GetBoxMonCount:   // result, kind
            *WorkPtr(a[0]) = CountAllStorageMons();   // (the PC's POKeMON)
            break;
        // ---- money (the game's money box)
        case U3C_OpenMoneyWin:   // x, y
            DrawMoneyBox(GetMoney(&gSaveBlock1Ptr->money), 0, 0);
            break;
        case U3C_UpdateMoneyWin:
            ChangeAmountInMoneyBox(GetMoney(&gSaveBlock1Ptr->money));
            break;
        case U3C_CloseMoneyWin:
            HideMoneyBox();
            break;
        case U3C_PROC_CheckMoneyAtLeast:   // result, amount
            *WorkPtr(a[0]) = IsEnoughMoney(&gSaveBlock1Ptr->money, V(1));
            break;
        case U3C_PROC_SubMoney:
            RemoveMoney(&gSaveBlock1Ptr->money, V(0));
            break;
        // ---- the time, the season, the player
        case U3C_WORK_CmdGetHourAndMinute:
        {
            struct SiiRtcInfo dt;
            Platform_GetDateTime(&dt);
            *WorkPtr(a[0]) = ConvertBcdToBinary(dt.hour), *WorkPtr(a[1]) = ConvertBcdToBinary(dt.minute);
            break;
        }
        case U3C_WORK_CmdGetSeason:
            *WorkPtr(a[0]) = U3D_Season();
            break;
        case U3C_GetPlayerForm:   // (0 walking, 1 a bike, 2 surfing)
            *WorkPtr(a[0]) = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) ? 2
                           : TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE) ? 1 : 0;
            break;
        case U3C_GetMmdlGridPos:   // person, x, z (Black's tiles)
        {
            struct ObjectEvent *o = ObjectOf(V(0));
            s32 m = U3D_MapIndex();
            if (o)
            {
                *WorkPtr(a[1]) = o->currentCoords.x - MAP_OFFSET + (m >= 0 ? 32 * U3D_MapCellX(m) : 0);
                *WorkPtr(a[2]) = o->currentCoords.y - MAP_OFFSET + (m >= 0 ? 32 * U3D_MapCellZ(m) : 0);
            }
            break;
        }
        case U3C_WORK_CmdGetMsgDataId:   // (the script's own words' file: a shared script prints from it)
            *WorkPtr(a[0]) = t->bank >= SYSTEM_BANKS ? t->bank - SYSTEM_BANKS : t->bank;
            break;
        case U3C_GetContinueFlag:
            *WorkPtr(a[0]) = 0;
            break;
        // ---- what has no screen of its own here yet (read past quietly)
        case U3C_StopSe:
        case U3C_SetAutoMode:
        case U3C_SAVEANIME_Start:
        case U3C_PROC_FLD_StartCameraShake:
        case U3C_TurnPlayerToFrontEvent:
        case U3C_WORK_CmdSetSpecialLocation:
        case U3C_SCREENEFFECTS_CreateBuildModelNode:   // (build models' animations: doors, lifts)
        case U3C_SCREENEFFECTS_DeleteBuildModelNode:
        case U3C_SCREENEFFECTS_PlayBuildModelNodeSlotWithSe:
        case U3C_SCREENEFFECTS_WaitBuildModelNodeAnim:
        case U3C_SCREENEFFECTS_PlayBuildModelAnimPair:
        case U3C_SCREENEFFECTS_PlayBuildModelAnimTwo:
        case U3C_SCREENEFFECTS_SwapBuildModelId:
        case U3C_SCREENEFFECTS_PlayBuildModelNodeCurrentMode3:
            break;
        case U3C_NETWORK_CmdGetCommValue:   // (wireless on: Kai's co-op stands in for it, docs/UNOVA_DECISIONS.md)
            *WorkPtr(a[0]) = 1;
            break;
        case U3C_PROC_FLD_ObtainCGearAndCallCommResetEvent:   // (the C-GEAR got: CGEAR_SAVE_SetObtained)
            sWork.cgear = TRUE;
            break;
        case U3C_ShowOrClearInfoWindow:   // (Black's touch screen's info window: nothing to show on one screen)
            break;
        case U3C_InitMenu:   // x, y, cursor, cancellable, result
            MenuInit(262, V(2), a[3], WorkPtr(a[4]));
            break;
        case U3C_InitMenuMsgData:
        case U3C_InitMenuMsgDataMode1:
            MenuInit(t->bank, V(2), a[3], WorkPtr(a[4]));
            break;
        case U3C_AddMenuItem:   // message, help message, value
            MenuAdd(a[0], V(2));
            break;
        case U3C_StartMenu:
        case U3C_StartMenuEx:
            MenuStart(t);
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
        // ---- Black's scene cameras (u3d.c): to a place and back; the player's view is the one "saved"
        case U3C_PROC_FLD_SaveCamera:
        case U3C_PROC_FLD_StopCameraTargetTrace:
        case U3C_PROC_FLD_ResumeCameraTargetTrace:
            break;
        case U3C_PROC_FLD_StartCameraMoveFromStream:   // pitch, yaw, distance, x, y, z, frames
            U3D_CameraMoveTo(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
            break;
        case U3C_PROC_FLD_StartCameraMoveToSaved:
        case U3C_PROC_FLD_StartCameraMoveToSetting:
            U3D_CameraMoveBack(a[0]);
            break;
        case U3C_PROC_FLD_RestoreCameraCmd:
            if (!U3D_CameraMoving())
                U3D_CameraReset();
            break;
        case U3C_PROC_FLD_WaitCameraIdle:
            t->wait = W_CAMERA;
            break;
        // ---- the save's misc bytes (Black's Wi-Fi and event slots: none here)
        case U3C_WORK_CmdSetMiscByte:
            break;
        case U3C_WORK_CmdGetMiscByte:
        case U3C_WORK_CmdCheckMiscByte:
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_PROC_FLD_CallCatchTutorial:   // JUNIPER's catching demonstration (u3d_battle.c)
            OpenScreen(t, SCREEN_CATCH_DEMO, 0);
            break;
        case U3C_StartFld3dCi:   // (Black's 3D cut-ins: the POKéDEX's, a Gym Leader's... not drawn yet)
        case U3C_StartFld3dCiByTable:
        case U3C_StartFld3dCiByVar:   // (a field move's: its POKéMON over the field)
        case U3C_IAIGIRI_StartOnPlayer:   // (CUT's slash on the player: the tree goes with its own sound after)
        case U3C_PALPARK_SetBeacon:   // (a link beacon of what the player did: no wireless here)
        case U3C_DebugWork:   // (Black's debug prints)
        case U3C_DebugStack:
        case U3C_PROC_Ov037InitFromVars:   // (a gym's switches' gimmick state: the gyms' own puzzles aren't kept yet)
        case U3C_PROC_Ov037SetValueA:
        case U3C_PROC_Ov037SetValueB:
        case U3C_PROC_Ov037ReadEightVars:
        case U3C_PROC_Ov037Finish:
            break;
        case U3C_PROC_CheckPartyMonKnowsMove:   // result, move, party slot
            *WorkPtr(a[0]) = V(2) < PARTY_SIZE && PartyMonHasMove(&gPlayerParty[V(2)], V(1));
            break;
        case U3C_PROC_FindPartyMonWithMove:   // result (6: nobody), move: a field move's POKéMON
        {
            u32 i;
            *WorkPtr(a[0]) = PARTY_SIZE;
            for (i = 0; i < PARTY_SIZE; i++)
                if (PartyMonHasMove(&gPlayerParty[i], V(1)))
                {
                    *WorkPtr(a[0]) = i;
                    break;
                }
            break;
        }
        case U3C_PROC_GetMonParamIfListed:   // result, party slot, Black's param (POKE_PARA: its 12 a script may read)
        {
            struct Pokemon *mon = V(1) < PARTY_SIZE ? &gPlayerParty[V(1)] : NULL;
            u32 v = 0;
            if (mon && GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES))
            {
                switch (V(2))
                {
                case 5:    // species (National Dex number)
                    v = SpeciesToNationalPokedexNum(GetMonData(mon, MON_DATA_SPECIES));
                    break;
                case 111:  // gender: 0 male, 1 female, 2 none
                {
                    u8 g = GetMonGender(mon);
                    v = g == MON_MALE ? 0 : g == MON_FEMALE ? 1 : 2;
                    break;
                }
                case 117:  // nicknamed
                {
                    u8 nick[POKEMON_NAME_LENGTH + 1], name[POKEMON_NAME_LENGTH + 1];
                    GetMonData(mon, MON_DATA_NICKNAME, nick);
                    GetSpeciesName(name, GetMonData(mon, MON_DATA_SPECIES));
                    v = StringCompare(nick, name) != 0;
                    break;
                }
                case 154:  // POKéRUS
                    v = GetMonData(mon, MON_DATA_POKERUS);
                    break;
                }
            }
            *WorkPtr(a[0]) = v;
            break;
        }
        case U3C_BufferCategoryEntryName:   // (bank 0xC7: Black's type names)
            StringCopy(Slot(a[0]), U3D_Text(199, V(1)));
            break;
        case U3C_BufferConvertedEntryName:   // slot, a TM / HM (Black's number): the move it teaches (the game's TM's)
        {
            u16 item = Item(V(1));
            if (IS_ITEM_TMHM(item))
                StringCopy(Slot(a[0]), gMoveNames[ItemIdToBattleMoveId(item)]);
            else
                *Slot(a[0]) = EOS;
            break;
        }
        case U3C_POKEMON_FLD_StorePartySpeciesRecord:   // (Black's record of the team a Gym Leader was beaten with)
            break;
        case U3C_BufferShortEntryName:   // (bank 0x37: Black's bag pockets)
            StringCopy(Slot(a[0]), U3D_Text(55, V(1)));
            break;
        case U3C_BufferNumber:
            ConvertIntToDecimalStringN(Slot(a[0]), V(1), STR_CONV_MODE_LEFT_ALIGN, 6);
            break;
        // ---- the game's screens
        case U3C_PROC_CallWordScreen:   // (ov223: the gift box's three POKé BALLS; the ball picked 0-2 in the work)
            OpenScreen(t, SCREEN_STARTER, a[0]);
            break;
        // ---- trainers (their eyes: U3D_Vm_TrainerSight)
        case U3C_GetTrainerHitKind:
            *WorkPtr(a[0]) = sEyeKind;
            break;
        case U3C_GetTrainerEyeTrainerId:   // which hit, result
            *WorkPtr(a[1]) = sEye[V(0) & 1].trainer;
            break;
        case U3C_GetTrainerEyeObjId:
            *WorkPtr(a[1]) = sEye[V(0) & 1].obj;
            break;
        case U3C_CreateTrainerEye:
            StartEye(V(0));
            break;
        case U3C_CallTrainerEyePairEvent:   // (the walks up end; the player turns to the trainer)
            ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], GetOppositeDirection(sEye[0].dir));
            t->wait = W_MOVES;
            break;
        case U3C_GetScriptTrainerId:
            *WorkPtr(a[0]) = TrainerOfScript(t->script);
            break;
        case U3C_GetTrainerBattleKind:   // trainer, result (0 single, 1 double, 2 triple, 3 rotation)
        case U3C_ClassifyTrainer:        // trainer, result (1 heals the party after, 2 gives an item, 0 neither)
        {
            u32 size;
            const u8 *tr = U3D_Vm_Trainer(V(0), &size);
            u16 r = 0;
            if (tr && size >= 20)
                r = op == U3C_GetTrainerBattleKind ? tr[2] : tr[16] ? 1 : (tr[18] | tr[19] << 8) ? 2 : 0;
            *WorkPtr(a[1]) = r;
            break;
        }
        case U3C_GetTrainerMessageIds:   // (the words a trainer says: before, after, after a loss; a pair's own)
        {
            u32 size;
            const u8 *tr = U3D_Vm_Trainer(TrainerOfScript(t->script), &size);
            bool8 doubles = tr && size >= 3 && tr[2] == 1;
            *WorkPtr(a[0]) = !doubles ? 0 : t->script >= 5000 ? 7 : 3;
            *WorkPtr(a[1]) = !doubles ? 2 : t->script >= 5000 ? 9 : 5;
            *WorkPtr(a[2]) = !doubles ? 24 : t->script >= 5000 ? 10 : 6;
            break;
        }
        case U3C_PlayTrainerEncounterBgm:
            PushSong();
            if (EyeSong(V(0)) != MUS_NONE)
                PlayNewMapMusic(EyeSong(V(0)));
            break;
        case U3C_BalloonWinSavedText:   // trainer, kind, person
        {
            u16 m = TrainerWords(V(0), V(1));
            if (m != 0xFFFF)
                MessageInBank(t, 189, m);
            break;
        }
        case U3C_StartTrainerBattle:   // trainer, a second, a third (u3d_battle.c: Black's trainers)
            if (U3D_PrepareBattle(V(0), V(1)))
                OpenScreen(t, SCREEN_BATTLE, 0);
            else
                MPNet_Log("u3d vm: no trainer %u", V(0));
            break;
        case U3C_StartMultiTrainerBattle:   // partner, the first, the second, (a fourth: unused)
            if (U3D_PrepareTagBattle(V(0), V(1), V(2)))
                OpenScreen(t, SCREEN_BATTLE, 0);
            else
                MPNet_Log("u3d vm: no tag battle %u %u %u", V(0), V(1), V(2));
            break;
        // ---- POKéMON
        case U3C_PROC_HealParty:
            HealPlayerParty();
            break;
        case U3C_PROC_GetPartyCount:   // (result, kind: 0 all, else those that can battle)
        {
            u32 i, n = 0;
            for (i = 0; i < PARTY_SIZE; i++)
            {
                u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
                if (sp != SPECIES_NONE && (V(1) == 0 || (sp != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HP))))
                    n++;
            }
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_PROC_AddMonToPartySimple:   // result, species, form, level
            *WorkPtr(a[0]) = ScriptGiveMon(Species(V(1)), V(3), ITEM_NONE, 0, 0, 0) != MON_CANT_GIVE;
            break;
        case U3C_BufferPartyMonNickname:
            if (V(1) < PARTY_SIZE)
            {
                GetMonData(&gPlayerParty[V(1)], MON_DATA_NICKNAME, Slot(a[0]));
                StringGet_Nickname(Slot(a[0]));
            }
            break;
        case U3C_BufferPartyMonSpeciesName:
            if (V(1) < PARTY_SIZE)
                GetSpeciesName(Slot(a[0]), GetMonData(&gPlayerParty[V(1)], MON_DATA_SPECIES));
            break;
        case U3C_PlayPokeVoice:   // species, form
            PlayCry_Normal(Species(V(0)), 0);
            break;
        case U3C_WORK_CmdGetPlayerSex:
            *WorkPtr(a[0]) = gSaveBlock2Ptr->playerGender;
            break;
        case U3C_WORK_CmdCheckLastBattleResultA:
            *WorkPtr(a[0]) = U3D_BattleResult() != 2;   // (Black: 1 won, 0 the player lost)
            break;
        case U3C_WORK_CmdGetRandom:
            *WorkPtr(a[0]) = V(1) ? Random() % V(1) : 0;
            break;
        case U3C_PROC_AddMoney:
            AddMoney(&gSaveBlock1Ptr->money, V(0));
            break;
        // ---- items (Black's numbers: Item())
        case U3C_AddItem:   // item, count, result (1: it fit)
            if (Item(V(0)) == ITEM_NONE)
                MPNet_Log("u3d vm: Black's item %u isn't in the game yet", V(0));
            *WorkPtr(a[2]) = Item(V(0)) == ITEM_NONE || AddBagItem(Item(V(0)), V(1));
            break;
        case U3C_RemoveItem:
            *WorkPtr(a[2]) = Item(V(0)) != ITEM_NONE && RemoveBagItem(Item(V(0)), V(1));
            break;
        case U3C_CanAddItem:
            *WorkPtr(a[2]) = Item(V(0)) == ITEM_NONE || CheckBagHasSpace(Item(V(0)), V(1));
            break;
        case U3C_HasItem:
            *WorkPtr(a[2]) = Item(V(0)) != ITEM_NONE && CheckBagHasItem(Item(V(0)), V(1));
            break;
        case U3C_GetItemPocket:   // (Black's pocket: 0 ITEMS, 1 MEDICINE, 2 TMs & HMs, 3 BERRIES, 4 KEY ITEMS)
            *WorkPtr(a[1]) = V(0) < ARRAY_COUNT(sUnovaItemPockets) ? sUnovaItemPockets[V(0)] : 0;
            break;
        case U3C_GetItemQuantity:
            *WorkPtr(a[1]) = Item(V(0)) == ITEM_NONE ? 0 : CountTotalItemQuantityInBag(Item(V(0)));
            break;
        case U3C_BufferItemName:
        case U3C_BufferItemNameAlt:
        case U3C_BufferItemNameWithCount:
            if (Item(V(1)) != ITEM_NONE)
                CopyItemName(Item(V(1)), Slot(a[0]));
            else
                StringCopy(Slot(a[0]), U3D_Text(54, V(1)));   // (Black's own name for an item the game lacks)
            break;
        // ---- warps (Black's zone and tile: the map holding it, u3d.c)
        case U3C_WarpToPosition:
        case U3C_WarpToPositionPlain:
        case U3C_WarpToPositionFixedExit:
        {
            u8 group, num;
            s16 x, y;
            if (U3D_MapOfZoneTile(a[0], a[1], a[2], &group, &num, &x, &y))
            {
                MPNet_Log("u3d vm: warp to zone %u (%d.%d at %d,%d)", a[0], group, num, x, y);
                sHolding = FALSE;   // (the map's load starts the field's scripts afresh)
                t->alive = FALSE;
                SetWarpDestination(group, num, WARP_ID_NONE, x, y);
                DoWarp();
                return;
            }
            MPNet_Log("u3d vm: no map for zone %u at %u,%u", a[0], a[1], a[2]);
            break;
        }
        case U3C_WORK_CmdSetWarpDestination:   // Black's warp point (a POKéMON CENTER, home): where a whiteout returns
            SetWhiteoutPoint(V(0));
            break;
        case U3C_WORK_CmdEndScriptWhiteOutA:   // (a loss the story doesn't go on from)
            sHolding = FALSE;
            t->alive = FALSE;
            SetMainCallback2(CB2_WhiteOut);
            return;
        // ---- the game's state
        case U3C_WORK_CmdGetRomVersion:
            *WorkPtr(a[0]) = 21;   // (Black: PM_VERSION_ID; White's branches aren't taken)
            break;
        case U3C_WORK_CmdCheckBadge:
            *WorkPtr(a[0]) = V(1) < 8 && FlagGet(FLAG_UNOVA_BADGE01 + V(1));
            break;
        case U3C_WORK_CmdSetBadge:
            if (V(0) < 8)
                FlagSet(FLAG_UNOVA_BADGE01 + V(0));
            break;
        case U3C_WORK_CmdGetBadgeCount:
        {
            u32 i, n = 0;
            for (i = 0; i < 8; i++)
                n += FlagGet(FLAG_UNOVA_BADGE01 + i);
            *WorkPtr(a[0]) = n;
            break;
        }
        case U3C_WORK_CmdGetZukanNationalFlag:
            *WorkPtr(a[0]) = FlagGet(FLAG_SYS_NATIONAL_DEX);
            break;
        case U3C_WORK_CmdSetZukanObtained:   // (JUNIPER's POKéDEX: the user's decision, the NATIONAL DEX)
            FlagSet(FLAG_SYS_POKEDEX_GET);
            EnableNationalPokedex();
            break;
        // ---- Black's map replacements (ov010 d_02170FB0: flag, work, the work's value that means "replaced"): the
        // works the scripts read; the replaced blocks themselves aren't redrawn yet
        case U3C_WORK_CmdSetMapReplaceFlag:   // flag, on, now
        case U3C_WORK_CmdCheckMapReplaceFlag:   // flag, result
        {
            static const u16 sReplace[10][2] = {{0x4031, 4640}, {0x4032, 548}, {0x4033, 518}, {0x4034, 4617},
                                                {0x4035, 790}, {0x4036, 4647}, {0x4037, 2096}, {0x4038, 1032},
                                                {0x4039, 288}, {0x403A, 4136}};
            u16 f = V(0);
            if (op == U3C_WORK_CmdSetMapReplaceFlag)
            {
                if (f < 10)
                    *WorkPtr(sReplace[f][0]) = V(1) ? sReplace[f][1] : 0;
            }
            else
                *WorkPtr(a[1]) = f < 10 && *WorkPtr(sReplace[f][0]) == sReplace[f][1];
            break;
        }
        // ---- a trainer beaten: Black's event flag 1420 + the trainer's number (EVENT_TRAINER_*DefeatedFlag)
        case U3C_WORK_CmdSetTrainerDefeatedFlag:
            FlagTo(1420 + V(0), TRUE);
            break;
        case U3C_WORK_CmdClearTrainerDefeatedFlag:
            FlagTo(1420 + V(0), FALSE);
            break;
        case U3C_WORK_CmdCheckTrainerDefeatedFlag:
            *WorkPtr(a[1]) = FlagOn(1420 + V(0));
            break;
        case U3C_WORK_CmdSetFixedEventFlag:
            FlagTo(0x963, TRUE);
            break;
        case U3C_WORK_CmdGetTimeZone:   // Black's time of day: 0 morning, 1 day, 2 evening, 3 night, 4 late night
        {
            u8 h;
            RtcCalcLocalTime();
            h = gLocalTime.hours;
            *WorkPtr(a[0]) = h < 5 ? 4 : h < 10 ? 0 : h < 17 ? 1 : h < 20 ? 2 : 3;
            break;
        }
        case U3C_WORK_CmdGetTrainerCardStars:   // (Black's card's stars: the game's TRAINER CARD keeps its own)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_WORK_CmdGetMonthAndDay:   // month, day (the PC's clock)
        {
            struct SiiRtcInfo r;
            Platform_GetDateTime(&r);
            *WorkPtr(a[0]) = ConvertBcdToBinary(r.month);
            *WorkPtr(a[1]) = ConvertBcdToBinary(r.day);
            break;
        }
        case U3C_WORK_CmdGetBirthday:   // (the DS's owner's birthday: none here, so the nurse never says it)
            *WorkPtr(a[0]) = 0;
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_POKEMON_FLD_CheckPartyFlagged:   // a POKeMON of the party has POKeRUS
            *WorkPtr(a[0]) = CheckPartyPokerus(gPlayerParty, 0x3F) != 0;
            break;
        case U3C_POKEMON_FLD_HandleMysteryGift:   // (Black's Mystery Gift deliveryman: none to give)
            *WorkPtr(a[1]) = 0;
            break;
        case U3C_StartPlayerFormChangeAndWait:   // (Black turns the player to the counter and back)
        case U3C_AddTrainerRecordValue:
            break;
        case U3C_SCREENEFFECTS_StartRecoveryBallsEvent:   // the healing machine: its tune, the song after
            PushSong();
            PlayNewMapMusic(MUS_UNOVA_ME_ASA);
            t->waitFrames = 60 * 6;
            t->wait = W_SONG_THEN_POP;
            break;
        case U3C_WORK_CmdGetWeek:
            RtcCalcLocalTime();
            *WorkPtr(a[0]) = gLocalTime.days % 7;
            break;
        case U3C_PROC_CheckMonHpNotFullOrEgg:   // (result 1: full HP or an EGG)
        {
            u16 i = V(1);
            *WorkPtr(a[0]) = i < PARTY_SIZE && GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES)
                          && (GetMonData(&gPlayerParty[i], MON_DATA_HP) == GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP)
                              || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG));
            break;
        }
        case U3C_StartWaitSeStopped:
            t->wait = W_SE;
            break;
        case U3C_StartWaitPokeVoice:   // (a POKéMON's cry played to its end)
            t->wait = W_CRY;
            break;
        case U3C_FadeInField:
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            t->wait = W_FADE;
            break;
        case U3C_ReportSave:   // (result: 1 saved)
            *WorkPtr(a[0]) = TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
            break;
        case U3C_POKEMON_FLD_StartSelectEvent:   // (nickname the POKéMON? Black asks; the game's own naming comes later)
            *WorkPtr(a[0]) = 0;
            break;
        case U3C_WORK_CmdEnableNationalDex:
            EnableNationalPokedex();
            break;
        case U3C_WaitMsgKey:   // (once the message has printed)
            t->wait = W_MESSAGE_KEY;
            break;
        // ---- Black's small side windows (a crowd's murmurs) shown together in the message box, the script going on;
        // its WaitMsgKey / close end them
        case U3C_AddSubWinMsg:   // message, x, y, window
        {
            static u8 sLine[160];
            u16 bank = t->bank;
            const u8 *text = U3D_Text(bank, a[0]);
            StringExpandPlaceholders(sLine, text);
            if (sSubCount && sSubCount % 2)
                StringAppend(sSubText, (const u8[]){CHAR_NEWLINE, EOS});
            else if (sSubCount)
                StringAppend(sSubText, (const u8[]){CHAR_PROMPT_SCROLL, EOS});
            if (StringLength(sSubText) + StringLength(sLine) < sizeof(sSubText) - 4)
                StringAppend(sSubText, sLine);
            sSubCount++;
            if (!IsFieldMessageBoxHidden())
                HideFieldMessageBox();
            ShowFieldMessage(sSubText);
            break;
        }
        case U3C_DeleteSubWinMsg:
            sSubCount = 0;
            sSubText[0] = EOS;
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
        case U3C_AddMmdl:   // x, z (Black's tiles), direction, object id, Black's object code, move code
            Gone(LocalId(V(3)), FALSE);
            AddPerson(LocalId(V(3)), V(4), V(0), V(1), V(2));
            break;
        case U3C_AddPartyFollowerMmdl:   // a person of the zone's event data brought out (its flag cleared before)
        {
            u8 local = LocalId(V(0));
            Gone(local, FALSE);
            if (ObjEventId(local) >= OBJECT_EVENTS_COUNT)
                TrySpawnObjectEvent(local, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            break;
        }
        case U3C_SetMmdlZoneDeleteOff:      // (a person who stays as the player crosses zones: ObjEventId finds it)
        case U3C_SCREENEFFECTS_ShowPlaceName:   // (the game shows a map's name as it is entered)
            break;
        case U3C_SetProcAction:   // a script to run once this one has ended, by its id in the zone the player is then in
            sPendingScript = a[0];
            break;
        case U3C_DeleteMmdl:
        {
            u8 id = ObjEventId(LocalId(V(0)));
            ForgetPerson(LocalId(V(0)));
            Gone(LocalId(V(0)), TRUE);
            if (id < OBJECT_EVENTS_COUNT && !gObjectEvents[id].isPlayer)
                RemoveObjectEventByLocalIdAndMap(gObjectEvents[id].localId, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup);
        }
            break;
        case U3C_WORK_CmdSetNpcGridPos:   // person (its event id), direction, x, y, z: its place while in the zone
        {
            u8 local = LocalId(V(0));
            u32 i;
            for (i = 0; i < sNpcAtCount && sNpcAt[i].local != local; i++)
                ;
            if (i < NPC_AT_MAX)
            {
                sNpcAt[i].local = local, sNpcAt[i].dir = V(1), sNpcAt[i].x = V(2), sNpcAt[i].z = V(4);
                if (i == sNpcAtCount)
                    sNpcAtCount++;
                SetObjEventTemplateCoords(local, MapX(V(2)), MapZ(V(4)));
            }
            break;
        }
        case U3C_InitMmdlGridPosition:   // person, x, y, z, direction
        {
            u16 obj = V(0);
            struct ObjectEvent *o = ObjectOf(obj);
            s32 x = MapX(V(1)), z = MapZ(V(3));
            if (obj != OBJ_PLAYER)
                SetObjEventTemplateCoords(LocalId(obj), x, z);
            if (o)
            {
                TryMoveObjectEventToMapCoords(o->localId, o->mapNum, o->mapGroup, x, z);
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
        // ---- music
        case U3C_FadeOutPlayBgm:
        case U3C_FadeOutPlayBgmWithFade:
        case U3C_FadeOutPlayFixedBgm:
            if (SongOf(a[0]) != MUS_NONE)
                FadeOutAndPlayNewMapMusic(SongOf(a[0]), 4);
            break;
        case U3C_PushAndPlayBgm:
        case U3C_PushBgmAndPlayWait:   // (the same, the field's song held under it: a scene's own song)
            PushSong();
            if (SongOf(a[0]) != MUS_NONE)
                PlayNewMapMusic(SongOf(a[0]));
            break;
        case U3C_StartWaitBgmThenPop:   // (the fanfare to its end, then the song it played over)
            t->waitFrames = 60 * 20;
            t->wait = W_SONG_THEN_POP;
            break;
        case U3C_PushBgm:
            PushSong();
            break;
        case U3C_PopBgm:
            PopSong();
            break;
        case U3C_ResumeZoneBgm:
        case U3C_ResumeZoneBgmWithFade:
        case U3C_ResumeBgmSlots:
            sSongDepth = 0;
            if (GetCurrentMapMusic() != GetCurrLocationDefaultMusic())
                FadeOutAndPlayNewMapMusic(GetCurrLocationDefaultMusic(), 4);
            break;
        // ---- sound
        case U3C_PlaySe:
            if (V(0) == 547)   // (the talk sound)
                PlaySE(SE_SELECT);
            break;
        // ---- screen (a fade waits for the one before it to end: the game ignores one started over another)
        case U3C_SCREENEFFECTS_WaitScriptEffect:
            t->wait = W_FADE;
            break;
        case U3C_PROC_FLD_StartFieldMapFadeIn:   // (after a screen Black opens over the field)
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            if (gPaletteFade.y != 0)
                BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeOutSlow:
        case U3C_SCREENEFFECTS_FadeOutSlowAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 2, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeOutFast:
        case U3C_SCREENEFFECTS_FadeOutFastAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInSlow:
        case U3C_SCREENEFFECTS_FadeInSlowAgain:
        case U3C_SCREENEFFECTS_FadeInSlowAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 2, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_FadeInFast:
        case U3C_SCREENEFFECTS_FadeInFastAltScreens:
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            break;
        case U3C_SCREENEFFECTS_WaitBrightness:
            t->wait = W_FADE;
            break;
        case U3C_SCREENEFFECTS_StartBrightnessChange:   // screens, from, to (Black's brightness, 16 = gone), speed
        {
            s16 from = (s16)a[1], to = (s16)a[2];
            if (gPaletteFade.active)
            {
                t->pc = here, t->wait = W_FADE;
                break;
            }
            BeginNormalPaletteFade(PALETTES_ALL, a[3] > 1 ? a[3] - 1 : 0, from < 0 ? -from : from, to < 0 ? -to : to, RGB_BLACK);
            break;
        }
        case U3C_PROC_CallFieldOpenEvent:    // (Black reopens its field after a screen: the game's own does that)
        case U3C_PROC_CallFieldCloseEvent:
        case U3C_RESEARCH_CmdSetSurveyAnswer:   // (the Xtransceiver survey: Wi-Fi's)
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
    case W_SE:
        if (!IsSEPlaying())
            t->wait = W_NONE;
        break;
    case W_CRY:
        if (!IsCryPlaying())
            t->wait = W_NONE;
        break;
    case W_CAMERA:
        if (!U3D_CameraMoving())
            t->wait = W_NONE;
        break;
    case W_SONG_THEN_POP:
        if (IsBGMStopped() || t->waitFrames == 0 || --t->waitFrames == 0)
        {
            PopSong();
            t->wait = W_NONE;
        }
        break;
    case W_MENU:
        if (MenuInput())
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
    if (sThread.wait == W_SCREEN)
        return;
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
    t->depth = 0;
    t->script = scriptId;
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
        if (Le16(p + 8) == 0 && x >= px && x < px + (w ? w : 1) && y >= pz && y < pz + (h ? h : 1)
         && WorkValue(Le16(p + 4)) == Le16(p + 2) && Run(Le16(p), 0xFFFF, TRUE))
            return;
    }
    // (a trigger on a rail: tools/unova/gen_maps.py laid its tiles out by the rail; the one whose work holds its value)
    for (i = 0; e && size >= 8 && i < e[7]; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * e[5] + EV_CONNECT * e[6] + EV_POS * i;
        if (Le16(p + 8) != 0 && WorkValue(Le16(p + 4)) == Le16(p + 2) && Run(Le16(p), 0xFFFF, TRUE))
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

// ON_TRANSITION of every UNOVA map (data/scripts/unova.inc): the zone-change and field-init scripts; the first time,
// Black's new-game script before them (SCRIPT_SYS_RunInitScript: script 9600, the flags that hide what isn't there yet)
#define SCRIPT_NEW_GAME 9600
void U3D_VmMapLoad(void)
{
    u16 zone = CurrentZone();
    static u16 sLastZone = 0xFFFF;
    if (zone == 0xFFFF || !Ready())
        return;
    sAddedCount = 0;   // (a new map: its field-init script adds its own)
    sGoneCount = 0;
    if (!sWork.began)
    {
        sWork.began = TRUE;
        RunNow(SCRIPT_NEW_GAME);
    }
    if (zone != sLastZone)
    {
        sNpcAtCount = 0;   // (a new zone: its people where its data puts them, till its scripts say)
        RunNow(InitScript(zone, 2));
    }
    else
    {
        u32 i;
        for (i = 0; i < sNpcAtCount; i++)   // (another of the zone's maps: its people set as before)
            SetObjEventTemplateCoords(sNpcAt[i].local, MapX(sNpcAt[i].x), MapZ(sNpcAt[i].z));
    }
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
    SpawnAdded();
    if (sPendingScript)   // (a script's SetProcAction: FIELD_EVENT_CHECK_StartPendingScript)
    {
        u16 id = sPendingScript;
        sPendingScript = 0;
        if (Run(id, 0xFFFF, TRUE))
        {
            ScriptContext_SetupScript(Unova_EventScript_Scene);
            return TRUE;
        }
    }
    d = InitFile(zone, &size);
    for (o = 0; d && o + 6 <= size && Le16(d + o) != 0; o += 6)
    {
        if (Le16(d + o) != 1)
            continue;
        at = o + 6 + (Le16(d + o + 2) | Le16(d + o + 4) << 16);   // (from the entry after its own: FindSceneScript)
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

// specialvar U3D_VmScreen: the screen Black's script waits for (0: it ended)
u16 U3D_VmScreen(void)
{
    return sThread.alive && sThread.wait == W_SCREEN ? sScreen : SCREEN_NONE;
}

static void CB2_ScreenDone(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

// special U3D_VmOpenScreen (waitstate: the script goes on when the screen closes)
void U3D_VmOpenScreen(void)
{
    static const u16 sGiftBox[3] = {495, 498, 501};   // (Black's three, national numbers: the script's 0-2)
    static u16 sSpecies[3];
    u32 i;
    switch (sScreen)
    {
    case SCREEN_STARTER:
        for (i = 0; i < 3; i++)
            sSpecies[i] = Species(sGiftBox[i]);
        MP_StarterTable_Open(sSpecies, CB2_ScreenDone);
        break;
    case SCREEN_BATTLE:
        BattleSetup_StartUnovaBattle();   // (back to the field and this script when it ends)
        break;
    case SCREEN_CATCH_DEMO:
        U3D_CatchTutorial();   // (the party saved; back to the field and this script when it ends)
        break;
    case SCREEN_WILD:   // (Black's scripted wild battle: the POKeMON set up by StartScriptedWildBattle)
        BattleSetup_StartScriptedWildBattle();
        ScriptContext_Stop();
        break;
    case SCREEN_PARTY:   // (Black's SelectPartyMon: the game's party screen, the slot in VAR_0x8004)
        ChoosePartyMon();
        break;
    default:
        NothingToRun();
        break;
    }
}

// special U3D_VmResume: back from the screen, the script goes on
void U3D_VmResume(void)
{
    switch (sScreen)
    {
    case SCREEN_STARTER:
        *WorkPtr(sScreenVar) = gSpecialVar_Result < 3 ? gSpecialVar_Result : 0;
        break;
    case SCREEN_CATCH_DEMO:
        LoadPlayerParty();   // (the player's own party back)
        break;
    case SCREEN_WILD:
        // (Black's GetResultFlag: 0 caught, 1 ran, 2 knocked out; CheckLastBattleResult: the player not beaten)
        sWildResult = gBattleOutcome == B_OUTCOME_CAUGHT ? 0 : gBattleOutcome == B_OUTCOME_WON ? 2 : 1;
        U3D_BattleEnded(!(gBattleOutcome == B_OUTCOME_LOST || gBattleOutcome == B_OUTCOME_DREW));
        break;
    case SCREEN_PARTY:
        if (sPartySlotWork)
            *WorkPtr(sPartySlotWork) = gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0xFF;
        if (sScreenVar)
            *WorkPtr(sScreenVar) = gSpecialVar_0x8004 < PARTY_SIZE;
        break;
    }
    sScreen = SCREEN_NONE;
    if (sThread.alive)
    {
        sThread.wait = W_NONE;
        if (!FuncIsActiveTask(Task_Vm))
            CreateTask(Task_Vm, 80);
    }
    else
        NothingToRun();
}

bool8 U3D_VmRunning(void)
{
    return sThread.alive;
}

// tests (mp_debug.c `mp u3dwork WORK VALUE` / `mp u3dflag FLAG 0|1`, Black's numbers in hex): a scene set up without
// playing the story to it
void U3D_Vm_DebugSet(bool8 flag, u16 id, u16 value)
{
    if (!flag && id == 0xFFFF)   // (`mp u3dwork FFFF 0`: the running script ends where it is: the opening's room)
    {
        if (sThread.alive)
            Finish(&sThread);
        return;
    }
    if (flag)
        FlagTo(id, value != 0);
    else
        *WorkPtr(id) = value;
}

// ------------------------------------------------------------------------------------------------ trainers' eyes
// Black's trainers who see the player (event_trainer_eye.c): a person of the zone's event data whose kind is 1 (sees
// the way it faces) or 2 (every way) and whose script is a trainer's (3000 + its number; 5000 + for a pair's second),
// not beaten (event flag 1420 + its number), the player in a straight line within its range (its first parameter),
// nothing blocking. The hit starts Black's common script 3616, which asks for these as it goes
#define SCRIPT_TRAINER_EYE 3616
#define TRAINER_FLAG_BASE 1420

static u16 TrainerOfScript(u16 script)
{
    return script >= 5000 ? script - 5000 : script >= 3000 ? script - 3000 : 0xFFFF;
}

static bool8 SeesPlayer(const u8 *p, u8 local, struct EyeHit *hit)
{
    struct ObjectEvent *o, *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 id = ObjEventId(local), d, dir, k;
    u16 trainer = TrainerOfScript(Le16(p + 10)), range = Le16(p + 14);
    if (trainer == 0xFFFF || id >= OBJECT_EVENTS_COUNT || FlagOn(TRAINER_FLAG_BASE + trainer) || range == 0)
        return FALSE;
    o = &gObjectEvents[id];
    if (o->invisible)
        return FALSE;
    for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
    {
        s16 dx = gDirectionToVectors[dir].x, dy = gDirectionToVectors[dir].y;
        if (Le16(p + 6) == 1 && dir != o->facingDirection)
            continue;
        for (d = 1; d <= range; d++)
        {
            s16 x = o->currentCoords.x + dx * d, y = o->currentCoords.y + dy * d;
            if (x == pl->currentCoords.x && y == pl->currentCoords.y)
            {
                for (k = 1; k < d; k++)   // (nothing in the way)
                    if (MapGridGetCollisionAt(o->currentCoords.x + dx * k, o->currentCoords.y + dy * k)
                     || GetObjectEventIdByXY(o->currentCoords.x + dx * k, o->currentCoords.y + dy * k) != OBJECT_EVENTS_COUNT)
                        return FALSE;
                hit->obj = Le16(p), hit->trainer = trainer, hit->local = local, hit->distance = d, hit->dir = dir;
                return TRUE;
            }
        }
    }
    return FALSE;
}

// field_control_avatar.c ProcessPlayerFieldInput, on an UNOVA map: a trainer has seen the player (their script runs)
bool8 U3D_Vm_TrainerSight(void)
{
    u32 size, i, n = 0;
    const u8 *e;
    if (!U3D_IsUnovaGroup(gSaveBlock1Ptr->location.mapGroup) || sThread.alive || !Ready())
        return FALSE;
    e = Events(CurrentZone(), &size);
    for (i = 0; e && size >= 8 && i < e[5] && n < 2; i++)
    {
        const u8 *p = e + 8 + EV_BG * e[4] + EV_PERSON * i;
        u16 kind = Le16(p + 6);
        if ((kind == 1 || kind == 2) && SeesPlayer(p, i + 1, &sEye[n]))
            n++;
    }
    if (n == 0)
        return FALSE;
    sEyeKind = 0;
    if (n == 2 && sEye[0].trainer == sEye[1].trainer)
        sEyeKind = 2;   // (a pair: one trainer of two)
    else if (n == 2 && CalculatePlayerPartyCount() >= 2)
        sEyeKind = 1;   // (two trainers at once)
    if (!Run(SCRIPT_TRAINER_EYE, sEye[0].obj, TRUE))
        return FALSE;
    ScriptContext_SetupScript(Unova_EventScript_Scene);
    return TRUE;
}

// CreateTrainerEye: the trainer's "!" and walk up to the player
static void StartEye(u8 slot)
{
    u8 id, n = 0, k;
    u8 *out;
    if (slot > 1 || (id = ObjEventId(sEye[slot].local)) >= OBJECT_EVENTS_COUNT)
        return;
    out = sMoves[id];
    out[n++] = MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK;
    for (k = 1; k < sEye[slot].distance && n < MAX_MOVE - 2; k++)
        out[n++] = MOVEMENT_ACTION_WALK_NORMAL_DOWN + (sEye[slot].dir - DIR_SOUTH);
    out[n] = MOVEMENT_ACTION_STEP_END;
    sMoving[id] = sEye[slot].local;
    ScriptMovement_StartObjectMovementScript(sEye[slot].local, gObjectEvents[id].mapNum, gObjectEvents[id].mapGroup, out);
}

// A trainer's words of KIND (a/0/9/0's records: their place is the message in system bank 189); 0xFFFF none
static u16 TrainerWords(u16 trainer, u16 kind)
{
    u32 size, i;
    const u8 *d = sTrMsgAt ? FileOf(sTrMsgAt, 0, &size) : NULL;
    for (i = 0; d && i + 4 <= size; i += 4)
        if (Le16(d + i) == trainer && Le16(d + i + 2) == kind)
            return i / 4;
    return 0xFFFF;
}

// The encounter song of a trainer's class (arm9's table; 1114 for one not in it)
static u16 EyeSong(u16 trainer)
{
    u32 size, i, n;
    const u8 *t = U3D_Vm_Trainer(trainer, &size);
    if (t == NULL || size < 2 || !sEyeSongsAt)
        return SongOf(1114);
    n = Get32(sEyeSongsAt);
    for (i = 0; i < n; i++)
        if (Le16(sPack + sEyeSongsAt + 4 + 4 * i) == t[1])
            return SongOf(Le16(sPack + sEyeSongsAt + 6 + 4 * i));
    return SongOf(1114);
}

// Black's warp points (ov010's 17: MAP_REPLACE / GAME_DATA_SetWarpId): u16 arrival flag, kind, the zone inside (a
// POKéMON CENTER, the player's home), its tile (x low byte, z high), the zone outside and its tile. Kai's whiteout
// returns to the save's last heal place (overworld.c SetWarpDestinationToLastHealLocation): the one inside, in front
// of the nurse, as Black's whiteout (or the outside one, for a point with no inside tile)
static void SetWhiteoutPoint(u16 warpId)
{
    const u8 *w;
    u8 group, num;
    s16 x, y;
    u16 zone, tx, tz;
    if (!Ready() || !sWarpsAt || warpId == 0 || warpId > Get32(sWarpsAt))
        return;
    w = sPack + sWarpsAt + 4 + 20 * (warpId - 1);
    zone = Le16(w + 4), tx = w[6], tz = w[7];
    if (Le16(w + 6) == 0)
        zone = Le16(w + 8), tx = Le16(w + 10), tz = Le16(w + 12);
    if (U3D_MapOfZoneTile(zone, tx, tz, &group, &num, &x, &y))
    {
        struct WarpData *h = &gSaveBlock1Ptr->lastHealLocation;
        h->mapGroup = group, h->mapNum = num, h->warpId = WARP_ID_NONE, h->x = x, h->y = y;
        MPNet_Log("u3d vm: whiteout point %u: zone %u (%d.%d at %d,%d)", warpId, zone, group, num, x, y);
    }
}

// A new game in UNOVA: Black's first warp point (the player's home in NUVEMA: MAP_REPLACE_GetDefaultWarpId)
void U3D_Vm_WhiteoutHome(void)
{
    SetWhiteoutPoint(1);
}

// The C-GEAR (FENNEL's gift in STRIATON): UNOVA's device once it's got
bool8 U3D_Vm_HasCGear(void)
{
    return sWork.cgear;
}
