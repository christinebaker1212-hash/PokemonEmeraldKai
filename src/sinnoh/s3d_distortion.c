// Moemon Kai: SINNOH's DISTORTION WORLD as Platinum's (src/overlay009/ov9_02249960.c and src/player_move.c): each
// floor's floating platforms (floors at other heights, west / east walls, ceilings), each with its own 32 x 32 tile
// attributes, walked on Platinum's 3D tile grid with its step tables (sDistortionStepDirection*); the jump points
// that carry the player from one surface to another (the displacement, the sprite turned by the jump's angle, the hop);
// the camera's angle zones turning the view around the player (DistWorldCameraAngleTemplate, over transitionSteps).
// The data is Platinum's, converted when the game is built (tools/sinnoh/gen_distortion.py -> sinnoh/platinum_dw.h).
// Coordinates are Platinum's: tiles of the stacked world (the floor's own tile + its offset; heights in tiles).
// On a platform the player's own map position follows along x / z (so the camera and coord events follow), up a
// wall it stays; the picture and the player's sprite come from the 3D position (s3d.c SetupView, S3D_PlaceSprite).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "script.h"
#include "overworld.h"
#include "fieldmap.h"
#include "sound.h"
#include "sprite.h"
#include "constants/event_object_movement.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/maps.h"
#include "constants/map_groups.h"
#include "s3d.h"
#include "mp.h"
#include "palette.h"
#include "metatile_behavior.h"
#include "constants/rgb.h"

void MoveCameraAndRedrawMap(int deltaX, int deltaY);   // (field_camera.c)

struct DwBounds { s16 x, y, z, sx, sy, sz; };
struct DwFloor { u16 map, hid; s16 off[3]; u8 plat, nplat, jump, njump, cam, ncam; };
struct DwPlatform { s16 kind; u16 attr; struct DwBounds b; u16 vert, horiz; };
struct DwJump { s16 dir; struct DwBounds b; s16 d[3]; s16 rot, steps; u16 axis, inv; s16 face, kind, idx; };
struct DwCam { struct DwBounds b; u16 ang[3]; s16 dir; s32 steps; };
// overlay009's moving platforms (sMovingPlatformMapTemplate*), elevator paths and prop kinds
struct DwMoving { u16 hid; u8 index; s16 t[3]; u8 path, dir, dest, kind, flag; };
struct DwPath { u8 next; s16 fin[3], chg[3]; s32 delta[3]; u8 set, clr; };
struct DwProp { u16 model; float off[3]; float scale; };
// overlay009's events (sMapEvents) and their commands, each with its params struct's fields in order
struct DwEvent { u16 hid; s16 t[3]; u8 cond; u16 val; u16 cmd, ncmd; };
struct DwCmd { u8 kind; s32 p[14]; };
// overlay009's sMapObjectEvents: who stands where, shown by a condition (or added by a script), their script
struct DwTeleport { u16 hid; s16 x, y, z, dz; u8 dir; u16 minProgress; const u8 *script; };
struct DwPerson { u16 hid; u8 localId; u16 gfx; s16 pos[3]; u8 dir, cond; u16 val, hide; u8 invisible; const u8 *script; };

#include "../../sinnoh/platinum_dw.h"

// Platinum's FaceDirection
enum { PD_UP, PD_DOWN, PD_LEFT, PD_RIGHT, PD_NONE };
// Platinum's floating platform kinds (FLOATING_PLATFORM_KIND_*); KIND_LAND: on the map's own ground
enum { KIND_FLOOR, KIND_WEST, KIND_EAST, KIND_CEILING, KIND_LAND };

// the moving platforms' saved state (Platinum's persisted movingPlatformFlags: which elevator is on which floor),
// in SINNOH's own vars (bit 15 of the high word: set up)
#define VAR_DW_PLATFORMS_LO 0x52C0
#define VAR_DW_PLATFORMS_HI 0x52C1
#define DW_FLAGS_SET_UP 0x8000
enum { ELEV_UP, ELEV_DOWN, ELEV_NONE };
// Platinum's FlagCondition and EventCmdKind
enum { COND_NONE, COND_PUZZLE_FALSE, COND_PUZZLE_TRUE, COND_PROGRESS_EQ, COND_PROGRESS_LEQ, COND_PROGRESS_GEQ,
       COND_MANUAL_ADD_ONLY, COND_GIRATINA_SHADOW, COND_CYRUS_APPEARANCE };
enum { CMD_SET_OBJECT_ANIM, CMD_MOVE_PLATFORM, CMD_ADD_OBJECT, CMD_DELETE_OBJECT, CMD_CASCADE_UP, CMD_START_SCRIPT,
       CMD_SET_PROGRESS, CMD_SHOW_GIRATINA_SHADOW, CMD_SET_GIRATINA_ANIM, CMD_SET_PUZZLE_FLAG, CMD_CASCADE_DOWN,
       CMD_GIRATINA_ARRIVAL, CMD_UXIE_TUTO, CMD_AZELF_TUTO, CMD_MESPRIT_TUTO, CMD_SHOW_ROOM_PLATFORMS,
       CMD_HIDE_ROOM_PLATFORMS, CMD_CLEAR_PUZZLE_FLAG };
// Platinum's other DISTORTION WORLD state, in SINNOH's vars: the CYRUS appearance state, GIRATINA's shadow
// flags (bits) and the boulder puzzle's persisted flags (bits)
#define VAR_DW_CYRUS 0x52C2
#define VAR_DW_SHADOWS 0x52C3
#define VAR_DW_PUZZLE 0x52C4      // (flags 0-15; 16 in VAR_DW_PUZZLE_HI's bit 0, its bit 15: set up)
#define VAR_DW_PUZZLE_HI 0x52C5
#define DW_PERSON_FLAGS 0x3C10   // a person's hide flag: this + its place in sDwPeople (set as its condition says on each load)
#define DW_PROP_SLOT 8
#define SHADOW_SLOT (S3D_DYN_PROPS - 1)   // (GIRATINA's shadow: the last)   // (the first of the field's dynamic prop slots the DISTORTION WORLD uses)

#define BASE_PITCH (-10750)   // DISTORTION_WORLD_CAMERA_BASE_ANGLE_X (the field camera's)
#define ATTR_COLLISION 0x8000
#define ATTR_WATER(a) (((a) & 0xFF) == 0x10 || ((a) & 0xFF) == 0x15)   // (Platinum's water behaviors: surfed)

static const s8 sStep[4][4][3] = {   // [kind][Platinum dir] -> dx, dy, dz (sDistortionStepDirection*)
    {{0, 0, -1}, {0, 0, 1}, {-1, 0, 0}, {1, 0, 0}},   // floor
    {{0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}},   // west wall
    {{0, 1, 0}, {0, -1, 0}, {0, 0, -1}, {0, 0, 1}},   // east wall
    {{0, 0, 1}, {0, 0, -1}, {-1, 0, 0}, {1, 0, 0}},   // ceiling
};
static const s16 sKindRot[5] = {0, 90, 270, 180, 0};   // the sprite's turn on each kind (InitPlayer rotAngles)
static const u8 sJumpHop[16] = {4, 6, 8, 10, 11, 12, 12, 12, 11, 10, 9, 8, 6, 4, 0, 0};   // sFloatingPlatformJumpOffsets

static struct
{
    const struct DwFloor *floor;   // NULL: not in the DISTORTION WORLD
    u16 map;
    u8 kind;                       // the surface walked on (KIND_*)
    s8 plat;                       // its platform (index in the floor's list), -1: none
    s16 x, y, z;                   // the tile (Platinum's stacked world)
    u8 dir;                        // facing (Platinum's)
    // a step or a jump under way
    u8 moving, t, dur;
    float from[3], to[3], pos[3];  // world units (Platinum's: 16 a tile)
    const struct DwJump *jump;
    float rot, rotFrom, rotTo;     // the sprite's turn (degrees)
    // the camera's angles (Platinum's u16 units) and a transition under way
    s32 cam[3], camTo[3];
    float camStep[3], camF[3];
    s32 camLeft;
    u8 matrix;                     // the player's sprite's affine matrix (0xFF: none)
    s16 last[3];                   // the tile last frame (a platform is ridden when the player steps onto it)
    // an elevator ride (Platinum's DistWorldElevatorPlatform): the platform, its path, how far along (world units)
    u8 riding, ridePhase, rideT;
    const struct DwMoving *ride;
    s32 rideOff[3];                // the platform's and the player's offset from their tiles (units x 4096 / 4096)
    float vib, rideBase[3];
    s16 rideTile[3];               // the tile the current path starts from (a chained path goes on from the last's end)
    u8 ridePath;
} sDw = {.matrix = 0xFF};

// An event's commands running (Platinum's DistWorldLoadedEvent), and the moving platforms' offsets on this floor
static struct
{
    const struct DwCmd *cmd;
    u8 n, i, state;
    u16 t;
    float vib, done[3];
    u8 carry;                      // the player rides the moving platform
    u8 cascade;                    // a waterfall carries the player (drawn from sDw.pos)
    s32 off;                       // how far the waterfall has carried them (units)
    s16 start[3];                  // the tile it started from
} sEv;
static float sPlatOff[64][3];      // (each moving platform of this floor moved from its tile, world units)

// A ride carried over the change of floors (the warp): the path and where along it
static struct { bool8 on; u16 destHid; u8 destIndex, path, dir; s32 off[3]; } sRideNext;
// CYNTHIA riding an elevator along (Platinum's passengerMapObj: 1F's down to B1F, B6F's down to B7F): who rides on
// this floor, and on the floor reached, where she stands as that floor's local id 128 while its arrival scene plays
// SURF on a floating platform's water (B4F's channel on its ceiling), and a waterfall carried across floors
static bool8 sSurfing, sSurfStart;
static struct { bool8 on; u8 up; s32 off; s16 x, y, z; } sCascadeNext;
static u8 sRider;
static float sRiderBase;
static bool8 sRiderBaseSet;
static struct { bool8 on; u16 hid, progress; s16 x, y; } sArrival;

static const struct DwFloor *FloorFor(u16 map)
{
    u32 i;
    for (i = 0; i < DW_FLOOR_COUNT; i++)
        if (sDwFloors[i].map == map)
            return &sDwFloors[i];
    return NULL;
}

static bool8 InBounds(const struct DwBounds *b, s32 x, s32 y, s32 z)
{
    return y >= b->y && y <= b->y + b->sy && z >= b->z && z <= b->z + b->sz && x >= b->x && x <= b->x + b->sx;
}

static const struct DwPlatform *Plat(s32 i)
{
    return sDw.floor && i >= 0 && i < sDw.floor->nplat ? &sDwPlatforms[sDw.floor->plat + i] : NULL;
}

// The tile's attributes on platform P: 0xFFFF outside it (GetCurrentFloatingPlatformTileAttributes)
static u16 PlatAttr(const struct DwPlatform *p, s32 x, s32 y, s32 z)
{
    s32 v, h;
    if (p == NULL || !InBounds(&p->b, x, y, z))
        return 0xFFFF;
    switch (p->kind)
    {
    case KIND_FLOOR: v = x - p->b.x, h = z - p->b.z; break;
    case KIND_WEST: v = p->b.sy - (y - p->b.y), h = z - p->b.z; break;
    case KIND_EAST: v = y - p->b.y, h = z - p->b.z; break;
    default: v = p->b.sx - (x - p->b.x), h = z - p->b.z; break;
    }
    if (v < 0 || h < 0 || v + h * p->vert >= 1024)
        return 0xFFFF;
    return sDwAttrs[p->attr][v + h * p->vert];
}

// Another platform of the same kind holding the tile (FindAndPrepareNewCurrentFloatingPlatform), -1: none
static s32 PlatAt(s32 x, s32 y, s32 z, s32 kind)
{
    s32 i;
    for (i = 0; sDw.floor && i < sDw.floor->nplat; i++)
        if (InBounds(&Plat(i)->b, x, y, z) && (kind < 0 || Plat(i)->kind == kind))
            return i;
    return -1;
}

static u8 ToPlatDir(u8 dir)
{
    switch (dir)
    {
    case DIR_NORTH: return PD_UP;
    case DIR_SOUTH: return PD_DOWN;
    case DIR_WEST: return PD_LEFT;
    case DIR_EAST: return PD_RIGHT;
    }
    return PD_NONE;
}

static u8 FromPlatDir(u8 dir)
{
    static const u8 d[] = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_SOUTH};
    return d[dir < 5 ? dir : 4];
}

// A tile's point in this map's world (Platinum's units; the floor's own model is at its own origin)
static void TileWorld(s32 x, s32 y, s32 z, float *out)
{
    out[0] = (x - sDw.floor->off[0]) * 16 + 8;
    out[1] = (y - sDw.floor->off[1]) * 16;
    out[2] = (z - sDw.floor->off[2]) * 16 + 8;
}

// The player's tile on the ground (on the map's own land: their map position, the ground's height)
static void LandTile(s32 *x, s32 *y, s32 *z)
{
    struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    *x = obj->currentCoords.x - MAP_OFFSET + sDw.floor->off[0];
    *z = obj->currentCoords.y - MAP_OFFSET + sDw.floor->off[2];
    *y = sDw.floor->off[1] + (s32)floorf(S3D_PlayerGround() / 16.0f + 0.5f);
}

bool8 S3D_Dw_Active(void)
{
    return sDw.floor != NULL;
}

// The floor's place in the DISTORTION WORLD's stacked world: Platinum's scripts there give and compare tiles with it
// added (GetPlayerMapPos, SetObjectEventPos); FALSE off its floors
bool8 S3D_Dw_TileOffset(s32 *x, s32 *z)
{
    if (sDw.floor == NULL)
        return FALSE;
    *x = sDw.floor->off[0];
    *z = sDw.floor->off[2];
    return TRUE;
}

// On the map's own ground or up on a platform / mid-jump: the 3D position drives the view and the player's sprite
static bool8 Off(void)
{
    return sDw.floor != NULL && (sDw.kind != KIND_LAND || sDw.jump != NULL || sDw.riding || sEv.carry || sEv.cascade);
}

// --------------------------------------------------------------------------------------------- the moving platforms
static u32 PlatformFlags(void)
{
    u32 f = VarGet(VAR_DW_PLATFORMS_LO) | (u32)VarGet(VAR_DW_PLATFORMS_HI) << 16;
    if (!(VarGet(VAR_DW_PLATFORMS_HI) & DW_FLAGS_SET_UP))
    {
        f = DW_PLATFORM_FLAGS_START;   // (InitPersistedData: a first visit)
        VarSet(VAR_DW_PLATFORMS_LO, f & 0xFFFF);
        VarSet(VAR_DW_PLATFORMS_HI, (f >> 16) | DW_FLAGS_SET_UP);
    }
    return f & 0x7FFFFFFF;
}

static void SetPlatformFlag(u8 flag, bool8 on)
{
    u32 f = PlatformFlags();
    if (flag >= DW_PLATFORM_FLAG_INVALID)
        return;
    f = on ? f | (1u << flag) : f & ~(1u << flag);
    VarSet(VAR_DW_PLATFORMS_LO, f & 0xFFFF);
    VarSet(VAR_DW_PLATFORMS_HI, (f >> 16) | DW_FLAGS_SET_UP);
}

// Platinum's persisted boulder puzzle flags (DIST_WORLD_PUZZLE_FLAG_*: where each boulder is, the tutorials seen, the
// trio in B6F, GIRATINA's room's platforms)
enum { PUZZLE_UXIE_TUTO_SEEN = 10, PUZZLE_COUNT = 17 };
static void InitPuzzle(void);
static u32 PuzzleFlags(void)
{
    if (!(VarGet(VAR_DW_PUZZLE_HI) & DW_FLAGS_SET_UP))
        InitPuzzle();
    return VarGet(VAR_DW_PUZZLE) | (u32)(VarGet(VAR_DW_PUZZLE_HI) & 1) << 16;
}

static void SetPuzzleFlags(u32 f)
{
    VarSet(VAR_DW_PUZZLE, f & 0xFFFF);
    VarSet(VAR_DW_PUZZLE_HI, ((f >> 16) & 1) | DW_FLAGS_SET_UP);
}

static void SetPuzzleFlag(u8 flag, bool8 on)
{
    u32 f = PuzzleFlags();
    if (flag >= PUZZLE_COUNT)
        return;
    SetPuzzleFlags(on ? f | (1u << flag) : f & ~(1u << flag));
}

// InitPersistedData's puzzle: the three boulders on B5F, or (the puzzle done once) in B6F's pits, tutorials seen
static void InitPuzzle(void)
{
    if (!FlagGet(DW_FLAG_PUZZLE_FINISHED))
        SetPuzzleFlags(1 << 0 | 1 << 1 | 1 << 2);
    else
        SetPuzzleFlags(1 << PUZZLE_UXIE_TUTO_SEEN | 1 << 11 | 1 << 12 | 1 << 6 | 1 << 7 | 1 << 8);
}

// Platinum's InitPersistedMapFeaturesForDistortionWorld (each floor's transition script): entered from outside, the
// world starts over (its persisted data cleared, InitPersistedData then sets it up: the B7F start has every elevator
// down); a floor reached by an elevator here is a warp, which keeps it (Platinum's floors change without one)
static bool8 sFromOutside;
void S3D_Dw_InitPersisted(void)
{
    u32 f = DW_PLATFORM_FLAGS_START;
    if (!sFromOutside || sDw.floor == NULL)
        return;
    sFromOutside = FALSE;
    if (sDw.floor->hid == 581)   // MAP_HEADER_DISTORTION_WORLD_B7F
        f = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 7 | 1 << 9 | 1 << 10;
    VarSet(VAR_DW_PLATFORMS_LO, f & 0xFFFF);
    VarSet(VAR_DW_PLATFORMS_HI, (f >> 16) | DW_FLAGS_SET_UP);
    InitPuzzle();
    MPNet_Log("sinnoh dw: entered, the world set up (platforms %x, puzzle %x)", f, PuzzleFlags());
}

// Platinum's GetPlayer3DPos: the player's tile in its stacked world (y in tiles)
void S3D_Dw_Player3DPos(u16 *x, u16 *y, u16 *z)
{
    struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (sDw.floor == NULL)
    {
        *x = pl->currentCoords.x - MAP_OFFSET, *y = 0, *z = pl->currentCoords.y - MAP_OFFSET;
        return;
    }
    if (sDw.kind == KIND_LAND && !sDw.riding)
    {
        s32 lx, ly, lz;
        LandTile(&lx, &ly, &lz);
        *x = lx, *y = ly, *z = lz;
        return;
    }
    *x = sDw.x, *y = sDw.y, *z = sDw.z;
}

// A platform on this floor is there (its flag set, or none: always)
static bool8 Present(const struct DwMoving *m)
{
    if (m->hid == 580 && m->index == 1 && !FlagGet(DW_FLAG_STEPPING_STONES))
        return FALSE;   // (B6F's platform to B7F: there once the puzzle is done, isGiratinaRoomElevator)
    return m->flag >= DW_PLATFORM_FLAG_INVALID || (PlatformFlags() & (1u << m->flag));
}

static const struct DwMoving *MovingOf(u16 hid, u8 index)
{
    u32 i;
    for (i = 0; i < DW_MOVING_COUNT; i++)
        if (sDwMoving[i].hid == hid && sDwMoving[i].index == index)
            return &sDwMoving[i];
    return NULL;
}

static const struct DwMoving *ElevatorAt(s32 x, s32 y, s32 z)
{
    u32 i;
    for (i = 0; sDw.floor && i < DW_MOVING_COUNT; i++)
    {
        const struct DwMoving *m = &sDwMoving[i];
        if (m->hid == sDw.floor->hid && m->t[0] == x && abs(m->t[1] - y) <= 1 && m->t[2] == z && m->dir != ELEV_NONE && Present(m))   // (heights here within a tile of Platinum's)
            return m;
    }
    return NULL;
}

static u16 ConnectedFloor(u16 hid, bool8 down)
{
    u32 i;
    for (i = 0; i < DW_CONN_COUNT; i++)
        if (sDwConns[i][0] == hid)
            return sDwConns[i][down ? 2 : 1];
    return 0xFFFF;
}

static const struct DwFloor *FloorOfHid(u16 hid)
{
    u32 i;
    for (i = 0; i < DW_FLOOR_COUNT; i++)
        if (sDwFloors[i].hid == hid)
            return &sDwFloors[i];
    return NULL;
}

// The platforms shown on this floor: each a prop at its tile (Platinum's sPropInitialPosOffsetByKind, its scale),
// the one being ridden moved along its path
static void DrawPlatforms(void)
{
    u32 i, slot = DW_PROP_SLOT;
    for (i = 0; i < DW_MOVING_COUNT && slot < SHADOW_SLOT; i++)
    {
        const struct DwMoving *m = &sDwMoving[i];
        float w[3];
        const struct DwProp *p;
        if (m->hid != sDw.floor->hid || !Present(m) || m->kind >= ARRAY_COUNT(sDwProps))
            continue;
        p = &sDwProps[m->kind];
        TileWorld(m->t[0], m->t[1], m->t[2], w);
        if (i < ARRAY_COUNT(sPlatOff))
            w[0] += sPlatOff[i][0], w[1] += sPlatOff[i][1], w[2] += sPlatOff[i][2];
        if (sDw.riding && (sDw.ride == m || (sDw.ride->hid == m->hid && sDw.ride->index == m->index)))
            TileWorld(sDw.rideTile[0], sDw.rideTile[1], sDw.rideTile[2], w);
        if (sDw.riding && (sDw.ride == m || (sDw.ride->hid == m->hid && sDw.ride->index == m->index)))
        {
            w[0] += sDw.rideOff[0] / 4096.0f;
            w[1] += sDw.rideOff[1] / 4096.0f + sDw.vib;
            w[2] += sDw.rideOff[2] / 4096.0f;
        }
        S3D_DynPropScaled(slot++, p->model, w[0] + p->off[0], w[1] + p->off[1], w[2] + p->off[2], 0, p->scale);
    }
    for (; slot < SHADOW_SLOT; slot++)
        S3D_DynPropHide(slot);
}

// ---- GIRATINA coming down into its room (EventCmdPlayGiratinaArrival_*): from 10 tiles up, dark, brightening once down
static struct { bool8 on; u8 state, wait; float off, dark; } sArrive;

static void ArriveTint(void)
{
    u8 obj;
    if (TryGetObjectEventIdByLocalIdAndMap(128, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &obj))
        return;
    if (gObjectEvents[obj].spriteId < MAX_SPRITES)
        MP_BlendSpritePalette(gSprites[gObjectEvents[obj].spriteId].oam.paletteNum, (u8)(sArrive.dark + 0.5f), RGB_BLACK);
}

// ---- GIRATINA's shadow flying by (Platinum's DistWorldGiratinaShadowProp: from a tile, a step each frame, its cry or
// its flight's sound first; drawn until its steps are done)
static const s16 sShadowTurn[5][2] = {{0, 0}, {0, 180}, {0, 90}, {0, 270}, {90, 0}};   // (tilt, turn) by rotAnglesIndex
static struct { bool8 on; u16 t, steps; float pos[3], step[3], scale; u8 rot; } sShadow;

static void StartShadow(const s32 *p)
{
    // p: tile x y z, turn, sound (1 cry, 2 the flight), scale x y z, step x y z (x 4096), steps
    memset(&sShadow, 0, sizeof(sShadow));
    if (sDw.floor == NULL)
        return;
    sShadow.on = TRUE;
    sShadow.rot = p[3] < 5 && p[3] >= 0 ? p[3] : 0;
    sShadow.pos[0] = (p[0] - sDw.floor->off[0]) * 16 + (sShadow.rot == 4 ? 8 : 0);
    sShadow.pos[1] = (p[1] - sDw.floor->off[1]) * 16;
    sShadow.pos[2] = (p[2] - sDw.floor->off[2]) * 16;
    // (Platinum's field runs at 30 frames a second: its step in two of ours)
    sShadow.step[0] = p[8] / 8192.0f, sShadow.step[1] = p[9] / 8192.0f, sShadow.step[2] = p[10] / 8192.0f;
    sShadow.scale = p[5] / 4096.0f;
    sShadow.steps = p[11] * 2;
    if (p[4] == 1)
        PlayCry_Normal(SPECIES_GIRATINA, 0);
    else if (p[4] == 2)
        PlaySE(SE_M_FLY);
}

static void ShadowUpdate(void)
{
    const struct DwProp *pr = &sDwProps[DW_PROP_GIRATINA_SHADOW < ARRAY_COUNT(sDwProps) ? DW_PROP_GIRATINA_SHADOW : 0];
    if (!sShadow.on || sShadow.t >= sShadow.steps)
    {
        S3D_DynPropHide(SHADOW_SLOT);
        return;
    }
    sShadow.pos[0] += sShadow.step[0], sShadow.pos[1] += sShadow.step[1], sShadow.pos[2] += sShadow.step[2];
    sShadow.t++;
    S3D_DynPropTurnedScaled(SHADOW_SLOT, pr->model, sShadow.pos[0] + pr->off[0], sShadow.pos[1] + pr->off[1],
                            sShadow.pos[2] + pr->off[2], sShadowTurn[sShadow.rot][0], sShadowTurn[sShadow.rot][1],
                            pr->scale * sShadow.scale);
    S3D_DynPropShade(SHADOW_SLOT, 16);
}

void S3D_Dw_Shadow(u16 n)
{
    if (n == 0xFFFF)
        sShadow.on = FALSE;
    else if (n < DW_SHADOW_COUNT)
        StartShadow(sDwShadows[n].p);
}

// The platforms' tiles walkable while a platform is there, void without (the map's own tiles there are void)
static void PlatformTiles(void)
{
    u32 i;
    for (i = 0; i < DW_MOVING_COUNT; i++)
    {
        const struct DwMoving *m = &sDwMoving[i];
        if (m->hid == sDw.floor->hid && m->dir != ELEV_NONE)
            MapGridSetMetatileImpassabilityAt(m->t[0] - sDw.floor->off[0] + MAP_OFFSET, m->t[2] - sDw.floor->off[2] + MAP_OFFSET, !Present(m));
    }
}

static void StartRide(const struct DwMoving *m, s32 startOff[3], bool8 vibrate)
{
    sDw.riding = TRUE;
    sDw.ride = m;
    sDw.ridePhase = vibrate ? 0 : 1;
    sDw.rideT = 0;
    sDw.vib = vibrate ? 6 : 0;
    memcpy(sDw.rideOff, startOff, sizeof(sDw.rideOff));
    sDw.x = m->t[0], sDw.y = m->t[1], sDw.z = m->t[2];
    sDw.rideTile[0] = m->t[0], sDw.rideTile[1] = m->t[1], sDw.rideTile[2] = m->t[2];
    sDw.ridePath = m->path;
    LockPlayerFieldControls();
    sRider = 0;
    sRiderBaseSet = FALSE;
    if (vibrate && sDw.floor != NULL)
    {
        // (DistWorldElevatorPlatform_Init: CYNTHIA rides along at these points of the story)
        u8 id = sDw.floor->hid == 573 && VarGet(DW_VAR_PROGRESS) == 2 ? 129
              : sDw.floor->hid == 580 && VarGet(DW_VAR_PROGRESS) == 7 ? 134 : 0;
        u8 obj;
        if (id && !TryGetObjectEventIdByLocalIdAndMap(id, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &obj))
            sRider = id;
    }
    if (vibrate)
        PlaySE(SE_ELEVATOR);
    if (getenv("MOEMON_TEST") && vibrate)
        printf("[dw] elevator %d on floor %d (path %d, %s)\n", m->index, m->hid, m->path, m->dir == ELEV_DOWN ? "down" : "up");
}

// Platinum's DistWorldElevatorPlatform_*: shake, move to the change of floors, the next floor, move on to the end
static void RideUpdate(void)
{
    const struct DwPath *p = &sDwPaths[sDw.ridePath];
    u32 i;
    float w[3];
    if (sDw.ridePhase == 0)
    {
        // the shake: up and down, smaller each time (Vibrate)
        sDw.vib = -sDw.vib;
        if (sDw.vib >= 0)
        {
            if (sDw.vib >= 4) sDw.vib -= 2;
            else if (sDw.vib > 1) sDw.vib -= 1;
            else if (++sDw.rideT >= 8) sDw.vib -= 1;
            if (sDw.vib <= 0)
                sDw.vib = 0, sDw.ridePhase = 1;
        }
    }
    else
    {
        bool8 done = TRUE;
        for (i = 0; i < 3; i++)
            if (sDw.rideOff[i] != (sDw.ridePhase == 3 ? 0 : p->fin[i] * 16 * 4096))
                sDw.rideOff[i] += p->delta[i], done = FALSE;
        if (sDw.ridePhase == 1 && sDw.rideOff[0] == p->chg[0] * 16 * 4096 && sDw.rideOff[1] == p->chg[1] * 16 * 4096
         && sDw.rideOff[2] == p->chg[2] * 16 * 4096 && p->next >= ARRAY_COUNT(sDwPaths))   // (a chained path changes floors on its last leg)
        {
            // the change of floors: the platform's flags, then the next floor with the platform it becomes there
            u16 dest = ConnectedFloor(sDw.floor->hid, sDw.ride->dir == ELEV_DOWN);
            const struct DwFloor *f = FloorOfHid(dest);
            const struct DwMoving *to = MovingOf(dest, sDw.ride->dest);
            SetPlatformFlag(p->set, TRUE);
            SetPlatformFlag(p->clr, FALSE);
            if (f != NULL && to != NULL)
            {
                s32 fx = sDw.rideTile[0] + p->fin[0], fz = sDw.rideTile[2] + p->fin[2];
                sRideNext.on = TRUE;
                sRideNext.destHid = dest;
                sRideNext.destIndex = sDw.ride->dest;
                sRideNext.path = sDw.ridePath;
                sRideNext.dir = sDw.ride->dir;
                for (i = 0; i < 3; i++)
                    sRideNext.off[i] = sDw.rideOff[i] - p->fin[i] * 16 * 4096;
                sArrival.on = FALSE;
                if (sRider)
                {
                    u8 obj;
                    if (!TryGetObjectEventIdByLocalIdAndMap(sRider, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &obj))
                    {
                        sArrival.on = TRUE;
                        sArrival.hid = dest;
                        sArrival.progress = VarGet(DW_VAR_PROGRESS);
                        sArrival.x = gObjectEvents[obj].currentCoords.x - MAP_OFFSET + sDw.floor->off[0] + p->fin[0] - f->off[0];
                        sArrival.y = gObjectEvents[obj].currentCoords.y - MAP_OFFSET + sDw.floor->off[2] + p->fin[2] - f->off[2];
                    }
                }
                SetWarpDestination(f->map >> 8, f->map & 0xFF, WARP_ID_NONE, fx - f->off[0], fz - f->off[2]);
                DoQuietWarp();
                sDw.ridePhase = 2;   // (waiting for the warp)
                return;
            }
        }
        if (sDw.ridePhase == 2)
            return;
        if (done && sDw.ridePhase == 1 && p->next < ARRAY_COUNT(sDwPaths))
        {
            // a chained path: the next leg from where this one ended (MoveSecondHalf -> INIT)
            for (i = 0; i < 3; i++)
                sDw.rideTile[i] += p->fin[i], sDw.rideOff[i] = 0;
            sDw.ridePath = p->next;
            done = FALSE;
        }
        if (done)
        {
            // the end of the ride: the player on the platform's tile, free to go
            sDw.riding = FALSE;
            PlatformTiles();
            UnlockPlayerFieldControls();
            if (getenv("MOEMON_TEST"))
                printf("[dw] elevator stopped on floor %d at %d,%d,%d\n", sDw.floor->hid, sDw.ride->t[0], sDw.ride->t[1], sDw.ride->t[2]);
            sDw.last[0] = sDw.ride->t[0], sDw.last[1] = sDw.ride->t[1], sDw.last[2] = sDw.ride->t[2];
            return;
        }
    }
    TileWorld(sDw.rideTile[0], sDw.rideTile[1], sDw.rideTile[2], w);
    sDw.pos[0] = w[0] + sDw.rideOff[0] / 4096.0f;
    sDw.pos[1] = w[1] + sDw.rideOff[1] / 4096.0f + sDw.vib;
    sDw.pos[2] = w[2] + sDw.rideOff[2] / 4096.0f;
}

static void StartCamera(const struct DwCam *c)
{
    u32 i;
    s32 to[3] = {c->ang[0] * 256, c->ang[1] * 256, c->ang[2] * 256};
    if (to[0] == sDw.camTo[0] && to[1] == sDw.camTo[1] && to[2] == sDw.camTo[2])
        return;
    for (i = 0; i < 3; i++)
    {
        s32 delta = (s16)to[i] - (s16)sDw.cam[i];   // (CalculateCameraAngleDelta: the short way round)
        if (delta > 0x8000) delta -= 0x10000;
        if (delta < -0x8000) delta += 0x10000;
        sDw.camTo[i] = to[i];
        sDw.camF[i] = sDw.cam[i];
        sDw.camStep[i] = (float)delta / (c->steps > 0 ? c->steps : 1);
    }
    sDw.camLeft = c->steps > 0 ? c->steps : 1;
    if (getenv("MOEMON_TEST"))
        printf("[dw] camera to %d %d %d over %d\n", c->ang[0], c->ang[1], c->ang[2], (int)c->steps);
}

// Platinum's ResetDistortionWorldPersistedCameraAngles (s3d_misc.c op 39): the camera's usual angles again
void S3D_Dw_ResetCamera(void)
{
    memset(sDw.cam, 0, sizeof(sDw.cam));
    memset(sDw.camTo, 0, sizeof(sDw.camTo));
    memset(sDw.camF, 0, sizeof(sDw.camF));
    sDw.camLeft = 0;
}

// ApplyCameraAngleForPlayerPosition: the zone the player is in, facing the way they go
static void CameraFor(s32 x, s32 y, s32 z, u8 pdir)
{
    s32 i;
    for (i = 0; i < sDw.floor->ncam; i++)
    {
        const struct DwCam *c = &sDwCams[sDw.floor->cam + i];
        if (c->dir == pdir && InBounds(&c->b, x, y, z))
        {
            StartCamera(c);
            return;
        }
    }
}

static const struct DwJump *JumpAt(s32 x, s32 y, s32 z, u8 pdir)
{
    s32 i;
    for (i = 0; i < sDw.floor->njump; i++)
    {
        const struct DwJump *j = &sDwJumps[sDw.floor->jump + i];
        if (j->dir == pdir && InBounds(&j->b, x, y, z))
            return j;
    }
    return NULL;
}

// The player's map position moved by DX, DZ tiles at once (a jump's landing): the camera with them
static void PlaceOnMap(s32 dx, s32 dz)
{
    struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (dx == 0 && dz == 0)
        return;
    MoveCameraAndRedrawMap(dx, dz);
    MoveObjectEventToMapCoords(obj, obj->currentCoords.x + dx, obj->currentCoords.y + dz);
}

static void StartJump(const struct DwJump *j)
{
    float a[3];
    sDw.jump = j;
    sDw.t = 0;
    sDw.dur = j->steps > 0 ? j->steps : 16;
    TileWorld(sDw.x, sDw.y, sDw.z, a);
    memcpy(sDw.from, a, sizeof(a));
    sDw.x += j->d[0], sDw.y += j->d[1], sDw.z += j->d[2];
    TileWorld(sDw.x, sDw.y, sDw.z, sDw.to);
    sDw.rotFrom = sDw.rot;
    sDw.rotTo = sDw.rot + j->rot;
    sDw.moving = TRUE;
    PlaySE(SE_LEDGE);
    if (getenv("MOEMON_TEST"))
        printf("[dw] jump to %d,%d,%d kind %d platform %d\n", sDw.x, sDw.y, sDw.z, j->kind, j->idx);
}

static void Settle(s32 x, s32 y, s32 z);
static bool8 sJumpSettles;   // (an event's jump: the surface found where it lands, not the jump point's)

static void Landed(void)
{
    const struct DwJump *j = sDw.jump;
    if (sJumpSettles)
    {
        sJumpSettles = FALSE;
        sDw.jump = NULL;
        sDw.moving = FALSE;
        Settle(sDw.x, sDw.y, sDw.z);
        return;
    }
    struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 mx = obj->currentCoords.x - MAP_OFFSET + sDw.floor->off[0], mz = obj->currentCoords.y - MAP_OFFSET + sDw.floor->off[2];
    sDw.jump = NULL;
    sDw.moving = FALSE;
    sDw.kind = j->kind < KIND_LAND ? j->kind : KIND_LAND;
    sDw.plat = j->idx >= 0 && j->idx < sDw.floor->nplat ? j->idx : -1;
    if (sDw.plat < 0)
        sDw.kind = KIND_LAND;
    sDw.rot = sKindRot[sDw.kind];
    PlaceOnMap(sDw.x - mx, sDw.z - mz);   // (the map position follows to the landing tile)
    sDw.dir = j->face >= 0 && j->face < 4 ? j->face : sDw.dir;
    ObjectEventClearHeldMovementIfFinished(obj);
    ObjectEventTurn(obj, FromPlatDir(sDw.dir));
    if (sSurfStart)
    {
        // (the hop onto the water: surfing from here, the player sitting on the POKéMON)
        sSurfStart = FALSE;
        sSurfing = TRUE;
        ObjectEventSetGraphicsId(obj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
        ObjectEventTurn(obj, FromPlatDir(sDw.dir));
        ScriptContext_Enable();
    }
    else
        PlaySE(SE_M_SAND_ATTACK);   // (Platinum's SEQ_SE_PL_SUTYA2: the dust of a landing)
    if (getenv("MOEMON_TEST"))
        printf("[dw] landed at %d,%d,%d on kind %d platform %d\n", sDw.x, sDw.y, sDw.z, sDw.kind, sDw.plat);
}

// ------------------------------------------------------------------------------------------------------ events
static bool8 Condition(u8 cond, u16 val)
{
    switch (cond)
    {
    case COND_NONE: return TRUE;
    case COND_PUZZLE_FALSE: return !(PuzzleFlags() & (1u << val));
    case COND_PUZZLE_TRUE: return (PuzzleFlags() & (1u << val)) != 0;
    case COND_PROGRESS_EQ: return VarGet(DW_VAR_PROGRESS) == val;
    case COND_PROGRESS_LEQ: return VarGet(DW_VAR_PROGRESS) <= val;
    case COND_PROGRESS_GEQ: return VarGet(DW_VAR_PROGRESS) >= val;
    case COND_GIRATINA_SHADOW: return !(VarGet(VAR_DW_SHADOWS) & (1 << val));
    case COND_CYRUS_APPEARANCE: return VarGet(VAR_DW_CYRUS) == val;
    }
    return FALSE;
}

// The player comes to rest at a tile: the surface there (a floating platform holding it, or the map's ground), the
// map position with them (EventCmdMovePlatform_EndMovement: FindAndPrepareNewCurrentFloatingPlatform)
static void Settle(s32 x, s32 y, s32 z)
{
    struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 mx = obj->currentCoords.x - MAP_OFFSET + sDw.floor->off[0], mz = obj->currentCoords.y - MAP_OFFSET + sDw.floor->off[2];
    s32 pi = PlatAt(x, y, z, -1);
    sDw.x = x, sDw.y = y, sDw.z = z;
    sDw.plat = pi;
    sDw.kind = pi >= 0 ? Plat(pi)->kind : KIND_LAND;
    sDw.rot = sKindRot[sDw.kind];
    PlaceOnMap(x - mx, z - mz);
    sDw.last[0] = x, sDw.last[1] = y, sDw.last[2] = z;
    TileWorld(x, y, z, sDw.pos);
    if (getenv("MOEMON_TEST"))
        printf("[dw] settled at %d,%d,%d on kind %d platform %d\n", x, y, z, sDw.kind, sDw.plat);
}

static const struct DwEvent *EventAt(s32 x, s32 y, s32 z)
{
    u32 i;
    for (i = 0; sDw.floor && i < DW_EVENT_COUNT; i++)
    {
        const struct DwEvent *e = &sDwEvents[i];
        if (e->hid == sDw.floor->hid && e->t[0] == x && e->t[2] == z && abs(e->t[1] - y) <= 1 && Condition(e->cond, e->val))
            return e;
    }
    return NULL;
}

static void StartEvent(const struct DwCmd *cmd, u8 n)
{
    memset(&sEv, 0, sizeof(sEv));
    sEv.cmd = cmd;
    sEv.n = n;
    LockPlayerFieldControls();
}

static void LandToState(void)
{
    s32 x, y, z;
    LandTile(&x, &y, &z);
    sDw.x = x, sDw.y = y, sDw.z = z;
}

static void PlayerWorldNow(float *w)
{
    if (Off())
        memcpy(w, sDw.pos, sizeof(sDw.pos));
    else
    {
        s32 x, y, z;
        LandTile(&x, &y, &z);
        TileWorld(x, y, z, w);
    }
}

static const struct DwMoving *FloorMoving(u8 index, u32 *gi)
{
    u32 i;
    for (i = 0; i < DW_MOVING_COUNT; i++)
        if (sDwMoving[i].hid == sDw.floor->hid && sDwMoving[i].index == index)
        {
            *gi = i;
            return &sDwMoving[i];
        }
    return NULL;
}

// One command a frame (overlay009's sEventCmdHandlers, state by state); TRUE: the event is over
static bool8 EventUpdate(void)
{
    const struct DwCmd *c;
    u32 i, gi = 0;
    if (sEv.i >= sEv.n)
        return TRUE;
    c = &sEv.cmd[sEv.i];
    switch (c->kind)
    {
    case CMD_MOVE_PLATFORM:
    {
        // p: platform index, move the player, (dummy), final offset x y z (tiles), the step a frame x y z (x 4096)
        const struct DwMoving *m = FloorMoving(c->p[0], &gi);
        if (m == NULL || gi >= ARRAY_COUNT(sPlatOff))
            break;
        if (sEv.state == 0)
        {
            sEv.carry = c->p[1];
            if (sEv.carry)
            {
                float w[3];
                PlayerWorldNow(w);
                memcpy(sDw.from, w, sizeof(w));
                if (sDw.kind == KIND_LAND)
                    LandToState();
            }
            memset(sEv.done, 0, sizeof(sEv.done));
            sEv.vib = 2;
            sEv.t = 0;
            sEv.state = 1;
            PlaySE(SE_ELEVATOR);
        }
        if (sEv.state == 1)
        {
            // the shake (EventCmdMovePlatform_Vibrate)
            sEv.vib = -sEv.vib;
            if (sEv.vib >= 0)
            {
                if (sEv.vib > 1)
                    sEv.vib -= 1;
                else if (++sEv.t >= 4)
                    sEv.vib = 0;
                if (sEv.vib == 0)
                    sEv.state = 2;
            }
        }
        else if (sEv.state == 2)
        {
            bool8 moving = FALSE;
            for (i = 0; i < 3; i++)
                if ((s32)sEv.done[i] != c->p[3 + i] * 16)
                {
                    float d = c->p[6 + i] / 4096.0f;
                    sEv.done[i] += d;
                    sPlatOff[gi][i] += d;
                    moving = TRUE;
                }
            if (!moving)
                sEv.state = 3;
        }
        if (sEv.carry)
            for (i = 0; i < 3; i++)
                sDw.pos[i] = sDw.from[i] + sEv.done[i] + (i == 1 ? sEv.vib : 0);
        if (sEv.state == 3)
        {
            if (sEv.carry)
            {
                sEv.carry = FALSE;
                Settle(sDw.x + c->p[3], sDw.y + c->p[4], sDw.z + c->p[5]);
            }
            break;   // (the next command)
        }
        return FALSE;
    }
    case CMD_SET_OBJECT_ANIM:
        // p: local id, movement action; the player's jump across (MOVEMENT_ACTION_JUMP_DISTORTION_WORLD_*: 3 tiles
        // over 24 frames, high)
        if (c->p[0] == 0xFF && c->p[1] >= 0x1000 && c->p[1] < 0x1004)
        {
            static struct DwJump sHop;
            static const s8 sDirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
            u8 d = c->p[1] - 0x1000;
            if (sEv.state == 0)
            {
                memset(&sHop, 0, sizeof(sHop));
                sHop.dir = d;
                sHop.d[0] = sDirs[d][0] * 3, sHop.d[2] = sDirs[d][1] * 3;
                sHop.steps = 24;
                sHop.axis = 1;
                sHop.face = d;
                if (sDw.kind == KIND_LAND)
                    LandToState();
                sJumpSettles = TRUE;
                ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], FromPlatDir(d));
                StartJump(&sHop);
                sEv.state = 1;
                return FALSE;
            }
            if (sDw.jump != NULL)
                return FALSE;
        }
        break;
    case CMD_SET_PROGRESS:
        VarSet(DW_VAR_PROGRESS, c->p[0]);
        break;
    case CMD_SET_PUZZLE_FLAG:
        SetPuzzleFlag(c->p[0], TRUE);
        break;
    case CMD_CLEAR_PUZZLE_FLAG:
        SetPuzzleFlag(c->p[0], FALSE);
        break;
    case CMD_CASCADE_DOWN:
    case CMD_CASCADE_UP:
    {
        // p: axis, direction, the last tile's fix x y z (tiles), the final offset x y z (units), the change of floors
        // x y z (tiles), the step x y z (x 4096: Platinum's 30 a second, so half in each of ours); Platinum's
        // EventCmdCascadeDown / Up: the sprite turned onto the falls, carried along to the next floor, set on the
        // tile below / above, a slow walk away
        bool8 up = c->kind == CMD_CASCADE_UP;
        float w[3];
        s32 fin = c->p[6], chg = c->p[9] * 16, step = c->p[12] / 8192;
        struct ObjectEvent *pl = &gObjectEvents[gPlayerAvatar.objectEventId];
        if (step == 0)
            step = up ? 4 : -1;
        if (sEv.state == 0)
        {
            s32 lx, ly, lz;
            if (sDw.kind == KIND_LAND)
            {
                LandTile(&lx, &ly, &lz);
                sDw.x = lx, sDw.y = ly, sDw.z = lz;
                if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) && pl->fieldEffectSpriteId < MAX_SPRITES)
                    gSprites[pl->fieldEffectSpriteId].invisible = TRUE;   // (Kai's SURF blob: the falls carry them)
            }
            sEv.start[0] = sDw.x, sEv.start[1] = sDw.y, sEv.start[2] = sDw.z;
            sEv.off = 0;
            sEv.cascade = TRUE;
            sEv.t = 0;
            sDw.rotFrom = sDw.rot;
            sEv.state = 1;
            PlaySE(SE_FALL);
        }
        if (sEv.state == 1 || sEv.state == 2)
        {
            if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) && pl->fieldEffectSpriteId < MAX_SPRITES)
                gSprites[pl->fieldEffectSpriteId].invisible = TRUE;   // (the blob below waits for them)
            if (sEv.t < 16)
                sDw.rot = sDw.rotFrom + 90.0f * (sEv.t + 1) / 16, sEv.t++;   // (RotateMapObject 90)
            if (sEv.off != fin)
                sEv.off += step;
            if ((up ? sEv.off > fin : sEv.off < fin))
                sEv.off = fin;
            TileWorld(sEv.start[0], sEv.start[1], sEv.start[2], w);
            sDw.pos[0] = w[0], sDw.pos[1] = w[1] + sEv.off, sDw.pos[2] = w[2];
            if (sEv.state == 1 && (up ? sEv.off >= chg : sEv.off <= chg))
            {
                // the change of floors (LoadFloor): carried on there (S3D_Dw_OnMapLoad resumes this)
                u16 dest = ConnectedFloor(sDw.floor->hid, !up);
                const struct DwFloor *f = FloorOfHid(dest);
                if (f != NULL)
                {
                    s32 ty = sEv.start[1] + c->p[3];
                    sCascadeNext.on = TRUE;
                    sCascadeNext.up = up;
                    sCascadeNext.off = sEv.off;
                    sCascadeNext.x = sEv.start[0], sCascadeNext.y = sEv.start[1], sCascadeNext.z = sEv.start[2];
                    (void)ty;
                    SetWarpDestination(f->map >> 8, f->map & 0xFF, WARP_ID_NONE, sEv.start[0] - f->off[0], sEv.start[2] - f->off[2]);
                    DoQuietWarp();
                    sEv.state = 9;   // (waiting for the warp)
                    return FALSE;
                }
                sEv.state = 2;
            }
            if (sEv.off == fin)
            {
                // the tile below / above (FinishingPosFix): on foot or surfing there, facing left
                sEv.cascade = FALSE;
                Settle(sEv.start[0], sEv.start[1] + c->p[3], sEv.start[2]);
                if (sDw.kind != KIND_LAND)
                {
                    sSurfing = TRUE;
                    ObjectEventSetGraphicsId(pl, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_SURFING));
                }
                else if (pl->fieldEffectSpriteId < MAX_SPRITES)
                    gSprites[pl->fieldEffectSpriteId].invisible = FALSE;
                sDw.dir = PD_LEFT;
                ObjectEventTurn(pl, DIR_WEST);
                sEv.state = 3;
                sEv.t = 0;
            }
            return FALSE;
        }
        if (sEv.state == 9)
            return FALSE;
        if (sEv.state == 3 || sEv.state == 4)
        {
            // the walk away: two slow steps west (Platinum's WALK_SLOW / WALK_SLOWER_WEST)
            if (sDw.kind == KIND_LAND)
            {
                if (ObjectEventIsMovementOverridden(pl) && !ObjectEventClearHeldMovementIfFinished(pl))
                    return FALSE;
                ObjectEventSetHeldMovement(pl, MOVEMENT_ACTION_WALK_SLOW_LEFT);
            }
            else
            {
                if (sDw.moving)
                    return FALSE;
                S3D_Dw_PlayerStep(DIR_WEST, 0);
            }
            sEv.state++;
            return FALSE;
        }
        if (sDw.kind == KIND_LAND && ObjectEventIsMovementOverridden(pl) && !ObjectEventClearHeldMovementIfFinished(pl))
            return FALSE;
        if (sDw.moving)
            return FALSE;
        break;
    }
    case CMD_UXIE_TUTO:
    case CMD_AZELF_TUTO:
    case CMD_MESPRIT_TUTO:
    {
        // B5F's lake guardian over its boulder (EventCmdShow*BoulderTuto: up, hovering, off to B6F): its cry, then
        // the tutorial seen and the guardian waiting in B6F (puzzle flags 10-12, 13-15)
        static const u16 sSpecies[3] = {SPECIES_UXIE, SPECIES_AZELF, SPECIES_MESPRIT};
        u8 k = c->kind - CMD_UXIE_TUTO;
        if (sEv.state == 0)
        {
            S3D_Dw_AddObject(131 + k);
            PlayCry_Normal(sSpecies[k], 0);
            sEv.t = 0;
            sEv.state = 1;
        }
        if (++sEv.t < 90)
            return FALSE;
        S3D_Dw_DeleteObject(131 + k);
        SetPuzzleFlag(10 + k, TRUE);
        SetPuzzleFlag(13 + k, TRUE);
        break;
    }
    case CMD_START_SCRIPT:
        // p: the floor's script number (Platinum's ScriptManager_Start: the event waits for it to end)
        if (sEv.state == 0)
        {
            for (i = 0; i < DW_EVENT_SCRIPT_COUNT; i++)
                if (sDwEventScripts[i].hid == sDw.floor->hid && sDwEventScripts[i].n == c->p[0])
                    break;
            if (i == DW_EVENT_SCRIPT_COUNT || sDwEventScripts[i].script == NULL)
                break;
            ScriptContext_SetupScript(sDwEventScripts[i].script);
            sEv.state = 1;
            return FALSE;
        }
        if (ScriptContext_IsEnabled())
            return FALSE;
        LockPlayerFieldControls();   // (the script's own release ends its lock; the event goes on)
        break;
    case CMD_ADD_OBJECT:
    case CMD_DELETE_OBJECT:
        // p: the floor, the local id (another floor's people show by their condition when it loads)
        if (c->p[0] == sDw.floor->hid)
        {
            if (c->kind == CMD_ADD_OBJECT)
                S3D_Dw_AddObject(c->p[1]);
            else
                S3D_Dw_DeleteObject(c->p[1]);
        }
        break;
    case CMD_SHOW_GIRATINA_SHADOW:
        if (sEv.state == 0)
        {
            StartShadow(c->p);
            sEv.state = 1;
        }
        if (sShadow.on && sShadow.t < sShadow.steps)
            return FALSE;
        sShadow.on = FALSE;
        break;
    case CMD_GIRATINA_ARRIVAL:
        // (Platinum's steps at 30 frames a second: halved in ours)
        if (sEv.state == 0)
        {
            S3D_Dw_AddObject(128);
            sArrive.on = TRUE, sArrive.off = 160.0f, sArrive.dark = 16.0f, sArrive.wait = 0;
            PlayCry_Normal(SPECIES_GIRATINA, 0);
            sEv.state = 1;
        }
        if (sEv.state == 1)
        {
            sArrive.off -= sArrive.off >= 16.0f ? 0.5f : 0.25f;
            if (sArrive.off <= 0)
                sArrive.off = 0, sEv.state = 2;
            ArriveTint();
            return FALSE;
        }
        if (sEv.state == 2)
        {
            sArrive.dark -= 16.0f / 180.0f;
            if (sArrive.dark <= 0)
                sArrive.dark = 0, sEv.state = 3;
            ArriveTint();
            return FALSE;
        }
        if (++sArrive.wait < 60)
            return FALSE;
        sArrive.on = FALSE;
        break;
    case CMD_SHOW_ROOM_PLATFORMS:
    case CMD_HIDE_ROOM_PLATFORMS:
        // (the room's stepping stones, ghost props shown group by group: their timing; the props are drawn with the
        // other ghost props)
        if (sEv.state == 0)
            sEv.t = c->kind == CMD_SHOW_ROOM_PLATFORMS ? 36 + 48 * 2 : 16 + 48 * 2 + 8, sEv.state = 1;
        if (sEv.t-- > 0)
            return FALSE;
        break;
    case CMD_SET_GIRATINA_ANIM:
        VarSet(VAR_DW_SHADOWS, VarGet(VAR_DW_SHADOWS) | (1 << c->p[0]));
        break;
    default:
        if (getenv("MOEMON_TEST"))
            printf("[dw] event command %d not played yet\n", c->kind);
        break;
    }
    sEv.i++;
    sEv.state = 0;
    return sEv.i >= sEv.n;
}

// A tile stepped onto: an event there (HandleEventAt)
static bool8 TileEvents(s32 x, s32 y, s32 z)
{
    const struct DwEvent *e = EventAt(x, y, z);
    if (e == NULL)
        return FALSE;
    if (getenv("MOEMON_TEST"))
        printf("[dw] event at %d,%d,%d (%d commands)\n", x, y, z, e->ncmd);
    StartEvent(&sDwCmds[e->cmd], e->ncmd);
    return TRUE;
}

// Platinum's ledges jumped twice (GIRATINA's room: stones three tiles apart, the two tiles between them its JUMP_*_TWICE
// pair, built as two ledges facing each other): the game's ledge hop lands on the second of the pair, which then hops
// one tile on to the next stone
static bool8 TwiceLedge(struct ObjectEvent *obj)
{
    u8 b = MapGridGetMetatileBehaviorAt(obj->currentCoords.x, obj->currentCoords.y);
    u8 d = obj->facingDirection;
    if (!((d == DIR_NORTH && MetatileBehavior_IsJumpSouth(b)) || (d == DIR_SOUTH && MetatileBehavior_IsJumpNorth(b))
       || (d == DIR_WEST && MetatileBehavior_IsJumpEast(b)) || (d == DIR_EAST && MetatileBehavior_IsJumpWest(b))))
        return FALSE;
    ObjectEventClearHeldMovementIfFinished(obj);
    if (ObjectEventSetHeldMovement(obj, GetJumpMovementAction(d)))
        return FALSE;
    PlaySE(SE_LEDGE);
    return TRUE;
}

// B7F's north end into GIRATINA's room (once CYRUS is beaten) and the room's south end back: no events, overlay009's
// own tile checks (DistWorld_HandlePlayerPositionChanged)
static bool8 Teleport(s32 x, s32 y, s32 z, u8 dir)
{
    u32 i;
    for (i = 0; i < DW_TELEPORT_COUNT; i++)
    {
        const struct DwTeleport *t = &sDwTeleports[i];
        if (t->hid == sDw.floor->hid && dir == t->dir && x == t->x && abs(y - t->y) <= 1
         && (z == t->z || z == t->z + t->dz) && VarGet(DW_VAR_PROGRESS) >= t->minProgress)
        {
            if (getenv("MOEMON_TEST"))
                printf("[dw] teleport at %d,%d,%d\n", x, y, z);
            ScriptContext_SetupScript(t->script);
            return TRUE;
        }
    }
    return FALSE;
}

// ---- the boulder puzzle (sBoulderFallLocations, DistWorldFallingBoulder_*): STRENGTH's boulders pushed onto B5F's
// holes fall to B6F; there, into the right pit they stay (the lake guardian there goes home), into a wrong one they
// fall back to B5F
static struct { u8 on, localId, dest, t; s16 flag; float off; } sFall;
enum { FALL_B6F, FALL_PIT, FALL_WRONG };

static s32 FallAt(s16 x, s16 y)
{
    u32 i;
    if (sDw.floor == NULL)
        return -1;
    for (i = 0; i < DW_FALL_COUNT; i++)
        if (sDwFalls[i].hid == sDw.floor->hid && sDwFalls[i].x == x - MAP_OFFSET + sDw.floor->off[0]
         && sDwFalls[i].z == y - MAP_OFFSET + sDw.floor->off[2])
            return i;
    return -1;
}

bool8 S3D_Dw_BoulderFallAt(s16 x, s16 y)
{
    return FallAt(x, y) >= 0;
}

// (field_player_avatar.c PushBoulder_End) TRUE: the boulder falls
bool8 S3D_Dw_BoulderPushed(struct ObjectEvent *boulder)
{
    s32 i = FallAt(boulder->currentCoords.x, boulder->currentCoords.y);
    if (i < 0 || sFall.on || boulder->localId < 128 || boulder->localId > 130)
        return FALSE;
    sFall.on = TRUE;
    sFall.localId = boulder->localId;
    sFall.flag = sDwFalls[i].flag;
    sFall.dest = sFall.flag >= 17 ? FALL_B6F : (sFall.flag >= 6 && sFall.flag <= 8 && !(PuzzleFlags() & (1u << sFall.flag))) ? FALL_PIT : FALL_WRONG;
    sFall.t = 0;
    sFall.off = 0;
    LockPlayerFieldControls();
    PlaySE(SE_FALL);
    MPNet_Log("sinnoh dw: boulder %u falls (%s)", sFall.localId, sFall.dest == FALL_B6F ? "to B6F" : sFall.dest == FALL_PIT ? "into its pit" : "back to B5F");
    return TRUE;
}

static void FallUpdate(void)
{
    u8 k = sFall.localId - 128;   // (0 MESPRIT's, 1 AZELF's, 2 UXIE's boulder)
    if (!sFall.on)
        return;
    sFall.off -= sFall.dest == FALL_PIT ? (sFall.t < 8 ? 1.0f : 2.0f) : 4.0f;
    if (++sFall.t < (sFall.dest == FALL_PIT ? 12 : 30))
        return;
    sFall.on = FALSE;
    S3D_Dw_DeleteObject(sFall.localId);
    if (sFall.dest == FALL_B6F)
    {
        SetPuzzleFlag(3 + k, TRUE);   // (*_BOULDER_IN_B6F_OUTSIDE)
        SetPuzzleFlag(0 + k, FALSE);  // (*_BOULDER_IN_B5F)
    }
    else if (sFall.dest == FALL_WRONG)
    {
        SetPuzzleFlag(0 + k, TRUE);
        SetPuzzleFlag(3 + k, FALSE);
    }
    else
    {
        // in its pit: the boulder there for good, the guardian's flag cleared and its scene (B6F's scripts 5 / 7 / 6)
        static const u8 sGuardian[3] = {15, 14, 13}, sScript[3] = {5, 7, 6};
        u8 p = sFall.flag - 6;   // (0 MESPRIT's pit, 1 AZELF's, 2 UXIE's)
        u32 i;
        SetPuzzleFlag(sFall.flag, TRUE);
        SetPuzzleFlag(3 + k, FALSE);
        S3D_Dw_AddObject(144 + p);
        PlaySE(SE_M_STRENGTH);
        SetPuzzleFlag(sGuardian[p], FALSE);
        if ((PuzzleFlags() & (7u << 6)) == (7u << 6))
            FlagSet(DW_FLAG_PUZZLE_FINISHED);
        for (i = 0; i < DW_EVENT_SCRIPT_COUNT; i++)
            if (sDwEventScripts[i].hid == 580 && sDwEventScripts[i].n == sScript[p] && sDwEventScripts[i].script)
            {
                ScriptContext_SetupScript(sDwEventScripts[i].script);
                return;
            }
    }
    UnlockPlayerFieldControls();
}

// ---- SURF on the platforms (Platinum's FieldMoves_CanUseSurfDistortionWorld: the tile ahead on the platform walked)
bool8 S3D_Dw_FacingWater(void)
{
    const s8 *d;
    u16 attr;
    if (sDw.floor == NULL || !DW_HAVE_DATA || sDw.kind == KIND_LAND || sDw.plat < 0 || sSurfing || sDw.moving || sDw.jump)
        return FALSE;
    d = sStep[sDw.kind][sDw.dir & 3];
    attr = PlatAttr(Plat(sDw.plat), sDw.x + d[0], sDw.y + d[1], sDw.z + d[2]);
    return attr != 0xFFFF && !(attr & ATTR_COLLISION) && ATTR_WATER(attr) && PartyHasMonWithSurf();
}

// special: the hop onto the water (Sinnoh_EventScript_DwSurf, after the game's own "use SURF?"); the script waits
void S3D_Dw_Surf(void)
{
    static struct DwJump sHop;
    if (!S3D_Dw_FacingWater())
    {
        ScriptContext_Enable();
        return;
    }
    memset(&sHop, 0, sizeof(sHop));
    sHop.d[0] = sStep[sDw.kind][sDw.dir & 3][0];
    sHop.d[1] = sStep[sDw.kind][sDw.dir & 3][1];
    sHop.d[2] = sStep[sDw.kind][sDw.dir & 3][2];
    sHop.steps = 16;
    sHop.axis = 1;
    sHop.kind = sDw.kind;
    sHop.idx = sDw.plat;
    sHop.face = sDw.dir;
    sSurfStart = TRUE;
    StartJump(&sHop);
}

// ---- the floor's people (Platinum's AddMapObjectsForMap / AddMapObjectWithLocalID): templates the map's events gain
// as it loads (s3d_scrcmd.c Sinnoh_PlatinumMapScripts), each shown or hidden by its own flag as its condition says, so
// the game's own object code spawns them, finds their scripts and keeps them across battles

static u16 MapNow(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static bool8 PersonShown(const struct DwPerson *p)
{
    if (p->cond == COND_MANUAL_ADD_ONLY)
        return FALSE;
    return Condition(p->cond, p->val) && (p->hide == 0 || !FlagGet(p->hide));
}

static u16 PersonFlag(u32 i)
{
    return sDwPeople[i].hide ? sDwPeople[i].hide : DW_PERSON_FLAGS + i;
}

// The person of this floor with that local id (Platinum lists some twice, in two places: the one its condition shows)
static s32 PersonOf(u16 hid, u8 localId)
{
    s32 i, first = -1;
    for (i = 0; i < DW_PEOPLE_COUNT; i++)
        if (sDwPeople[i].hid == hid && sDwPeople[i].localId == localId)
        {
            if (PersonShown(&sDwPeople[i]))
                return i;
            if (first < 0)
                first = i;
        }
    return first;
}

static void HidePeople(void);

// After a battle (s3d_misc.c op 38, GetBattleResult): Platinum loads the map again, so the people the story shows now
// come out (GIRATINA's room's portal once GIRATINA is gone); this game comes back without a reload
void S3D_Dw_RefreshPeople(void)
{
    const struct DwFloor *f = sDw.floor;
    u32 i;
    if (f == NULL)
        return;
    for (i = 0; i < DW_PEOPLE_COUNT; i++)
    {
        const struct DwPerson *p = &sDwPeople[i];
        if (p->hid != f->hid || p->hide != 0 || p->cond == COND_MANUAL_ADD_ONLY || PersonOf(p->hid, p->localId) != (s32)i)
            continue;
        if (PersonShown(p) && FlagGet(DW_PERSON_FLAGS + i))
        {
            FlagClear(DW_PERSON_FLAGS + i);
            TrySpawnObjectEvent(p->localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
            HidePeople();
        }
    }
}

void S3D_Dw_AddPeople(struct MapEvents *ev, struct ObjectEventTemplate *buf, u32 cap)
{
    static const u8 sFace[4] = {MOVEMENT_TYPE_FACE_UP, MOVEMENT_TYPE_FACE_DOWN, MOVEMENT_TYPE_FACE_LEFT, MOVEMENT_TYPE_FACE_RIGHT};
    const struct DwFloor *f = FloorFor(MapNow());
    u32 i, n = 0;
    if (f == NULL || DW_PEOPLE_COUNT == 0 || ev == NULL)
        return;
    if (ev->objectEvents != buf)
    {
        if (ev->objectEventCount > cap)
            return;
        memcpy(buf, ev->objectEvents, ev->objectEventCount * sizeof(*buf));
        ev->objectEvents = buf;
    }
    if (sArrival.on && sArrival.hid == f->hid && VarGet(DW_VAR_PROGRESS) == sArrival.progress && ev->objectEventCount < cap)
    {
        // CYNTHIA off the elevator, for the floor's arrival scene (B7F's keeps her talk script, Platinum's 6)
        struct ObjectEventTemplate *t = &buf[ev->objectEventCount++];
        s32 k = PersonOf(f->hid, 128);
        memset(t, 0, sizeof(*t));
        t->localId = 128;
        t->graphicsIdLo = OBJ_EVENT_GFX_SINNOH_CYNTHIA & 0xFF;
        t->graphicsIdHi = OBJ_EVENT_GFX_SINNOH_CYNTHIA >> 8;
        t->x = sArrival.x;
        t->y = sArrival.y;
        t->elevation = 3;
        t->movementType = MOVEMENT_TYPE_FACE_DOWN;
        t->script = f->hid == 581 && k >= 0 ? sDwPeople[k].script : NULL;
        n++;
    }
    for (i = 0; i < DW_PEOPLE_COUNT; i++)
    {
        const struct DwPerson *p = &sDwPeople[i];
        struct ObjectEventTemplate *t;
        if (p->hid != f->hid || PersonOf(p->hid, p->localId) != (s32)i || ev->objectEventCount >= cap)
            continue;
        if (p->hide == 0)
        {
            if (PersonShown(p))
                FlagClear(DW_PERSON_FLAGS + i);
            else
                FlagSet(DW_PERSON_FLAGS + i);
        }
        t = &buf[ev->objectEventCount++];
        memset(t, 0, sizeof(*t));
        t->localId = p->localId;
        t->graphicsIdLo = p->gfx & 0xFF;
        t->graphicsIdHi = p->gfx >> 8;
        t->x = p->pos[0] - f->off[0];
        t->y = p->pos[2] - f->off[2];
        t->elevation = 3;
        t->movementType = sFace[p->dir & 3];
        t->script = p->script;
        t->flagId = PersonFlag(i);
        n++;
    }
    MPNet_Log("sinnoh dw: %u people on floor %u", n, f->hid);
}

// Platinum's AddDistortionWorldMapObject / DeleteDistortionWorldMapObject (s3d_misc.c ops 30, 31)
void S3D_Dw_AddObject(u8 localId)
{
    const struct DwFloor *f = FloorFor(MapNow());
    s32 i = f ? PersonOf(f->hid, localId) : -1;
    if (i < 0)
        return;
    FlagClear(PersonFlag(i));
    TrySpawnObjectEvent(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

void S3D_Dw_DeleteObject(u8 localId)
{
    const struct DwFloor *f = FloorFor(MapNow());
    s32 i = f ? PersonOf(f->hid, localId) : -1;
    if (i >= 0)
        FlagSet(PersonFlag(i));
    RemoveObjectEventByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

float S3D_TestGround(s32 mapX, s32 mapY, float near);

// Someone standing on an elevator platform (no ground of the floor's own there: the platform is a prop): its top
static float PlatformUnder(u8 localId)
{
    u8 obj;
    u32 i;
    s32 x, z, mx, my;
    if (sDw.floor == NULL || TryGetObjectEventIdByLocalIdAndMap(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &obj))
        return NAN;
    mx = gObjectEvents[obj].currentCoords.x - MAP_OFFSET;
    my = gObjectEvents[obj].currentCoords.y - MAP_OFFSET;
    if (S3D_TestGround(mx * 16 + 8, my * 16 + 8, -4096.0f) != -4096.0f)
        return NAN;   // (the floor's own ground)
    x = mx + sDw.floor->off[0];
    z = my + sDw.floor->off[2];
    for (i = 0; i < DW_MOVING_COUNT; i++)
    {
        const struct DwMoving *m = &sDwMoving[i];
        if (m->hid == sDw.floor->hid && Present(m) && abs(m->t[0] - x) <= 2 && abs(m->t[2] - z) <= 2)
            return (m->t[1] - sDw.floor->off[1]) * 16.0f;
    }
    return NAN;
}

// CYNTHIA riding along: up and down with the elevator (s3d.c's people heights); people on a platform stand on it
static float PersonHeight(u8 localId, float h)
{
    float top;
    if (sFall.on && localId == sFall.localId)
        return h + sFall.off;
    if (sArrive.on && localId == 128 && sDw.floor != NULL && sDw.floor->hid == 582)
        return h + sArrive.off;
    top = PlatformUnder(localId);
    if (sDw.floor != NULL && sDw.riding && sRider && localId == sRider)
    {
        // (her ground when the ride starts: the height given back is kept as the next frame's search height)
        if (!sRiderBaseSet)
            sRiderBase = top == top ? top : h, sRiderBaseSet = TRUE;
        return sRiderBase + sDw.rideOff[1] / 4096.0f + sDw.vib;
    }
    return top;
}

// Platinum's INVISIBLE people (the places scripts walk others to) stay unseen
static void HidePeople(void)
{
    u32 i, j;
    if (sDw.floor == NULL)
        return;
    for (i = 0; i < DW_PEOPLE_COUNT; i++)
        if (sDwPeople[i].invisible && sDwPeople[i].hid == sDw.floor->hid)
            for (j = 0; j < OBJECT_EVENTS_COUNT; j++)
                if (gObjectEvents[j].active && gObjectEvents[j].localId == sDwPeople[i].localId
                 && gObjectEvents[j].mapNum == gSaveBlock1Ptr->location.mapNum
                 && gObjectEvents[j].mapGroup == gSaveBlock1Ptr->location.mapGroup)
                    gObjectEvents[j].invisible = TRUE;
}

void S3D_Dw_OnMapLoad(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    const struct DwFloor *f = FloorFor(map);
    sFromOutside = f != NULL && sDw.floor == NULL;
    if (f == NULL || f->hid != sArrival.hid)
        sArrival.on = FALSE;
    if (f != sDw.floor || map != sDw.map)
    {
        memset(sDw.cam, 0, sizeof(sDw.cam));
        memset(sDw.camTo, 0, sizeof(sDw.camTo));
        sDw.camLeft = 0;
    }
    sDw.floor = f;
    sDw.map = map;
    sDw.kind = KIND_LAND;
    sDw.plat = -1;
    sDw.jump = NULL;
    sDw.moving = FALSE;
    sDw.rot = 0;
    sDw.riding = FALSE;
    sDw.last[0] = sDw.last[1] = sDw.last[2] = -1;
    memset(sPlatOff, 0, sizeof(sPlatOff));
    memset(&sEv, 0, sizeof(sEv));
    if (sDw.matrix != 0xFF)
    {
        FreeOamMatrix(sDw.matrix);
        sDw.matrix = 0xFF;
    }
    if (f != NULL && DW_HAVE_DATA)
        PlatformTiles();
    if (sCascadeNext.on)
    {
        // a waterfall's second half on the floor it reaches (CmdRunDataCascadeBase_Update after LoadFloor)
        u16 cmd = sCascadeNext.up ? DW_CMDS_B5F_WATERFALL : DW_CMDS_B4F_WATERFALL;
        sCascadeNext.on = FALSE;
        if (f != NULL && cmd != 0xFFFF)
        {
            StartEvent(&sDwCmds[cmd], 1);
            sEv.state = 2;
            sEv.cascade = TRUE;
            sEv.off = sCascadeNext.off;
            sEv.t = 16;
            sEv.start[0] = sCascadeNext.x, sEv.start[1] = sCascadeNext.y, sEv.start[2] = sCascadeNext.z;
            sDw.rot = sKindRot[sCascadeNext.up ? KIND_LAND : KIND_CEILING] + 90;
        }
    }
    if (sRideNext.on)
    {
        // the second half of an elevator ride, on the floor it goes to (MoveSecondHalf)
        const struct DwMoving *m = MovingOf(sRideNext.destHid, sRideNext.destIndex);
        sRideNext.on = FALSE;
        if (f != NULL && m != NULL && f->hid == sRideNext.destHid)
        {
            static struct DwMoving sRidden;
            sRidden = *m;
            sRidden.path = sRideNext.path;   // (the path keeps going: its end is this platform's tile)
            sRidden.dir = sRideNext.dir;
            StartRide(MovingOf(m->hid, m->index), sRideNext.off, FALSE);
            sDw.ride = &sRidden;
            sDw.ridePath = sRideNext.path;
            sDw.ridePhase = 3;
            sRider = sArrival.on && sArrival.hid == f->hid ? 128 : 0;
            sRiderBaseSet = FALSE;
        }
    }
}

// The player's input on a DISTORTION WORLD floor (field_player_avatar.c PlayerStep): TRUE when handled here
bool8 S3D_Dw_PlayerStep(u8 dir, u16 heldKeys)
{
    struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 pdir = ToPlatDir(dir);
    s32 x, y, z;
    const struct DwJump *j;
    if (sDw.floor == NULL || !DW_HAVE_DATA)
        return FALSE;
    if (sDw.jump != NULL || (sDw.kind != KIND_LAND && sDw.moving))
        return TRUE;   // (a jump or a step under way)
    if (sDw.kind == KIND_LAND)
    {
        if (pdir == PD_NONE || (ObjectEventIsMovementOverridden(obj) && !ObjectEventClearHeldMovementIfFinished(obj)))
            return FALSE;
        LandTile(&x, &y, &z);
        sDw.x = x, sDw.y = y, sDw.z = z;
        if (sDw.floor->hid == 579 && pdir == PD_RIGHT && x == 104 && z >= 76 && z <= 79 && abs(y - 128) <= 1
         && TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) && DW_CMDS_B5F_WATERFALL != 0xFFFF)
        {
            // B5F's waterfall, surfed into heading right: up to B4F's ceiling (DistWorld_HandlePlayerMoved)
            StartEvent(&sDwCmds[DW_CMDS_B5F_WATERFALL], 1);
            return TRUE;
        }
        // (a tap only turns, as on the ground anywhere: a step that way is what takes the jump or turns the camera)
        if (dir != obj->facingDirection && gPlayerAvatar.runningState != MOVING)
            return FALSE;
        CameraFor(x, y, z, pdir);
        j = JumpAt(x, y, z, pdir);
        if (j == NULL)
            return FALSE;
        sDw.dir = pdir;
        sDw.rot = 0;
        ObjectEventClearHeldMovementIfFinished(obj);
        ObjectEventTurn(obj, dir);
        StartJump(j);
        return TRUE;
    }
    // up on a platform: Platinum's own walking (PlayerAvatar_SetMovement_Distortion*)
    if (ObjectEventIsMovementOverridden(obj) && !ObjectEventClearHeldMovementIfFinished(obj))
        return TRUE;
    if (pdir == PD_NONE)
        return TRUE;
    CameraFor(sDw.x, sDw.y, sDw.z, pdir);
    j = JumpAt(sDw.x, sDw.y, sDw.z, pdir);
    if (j != NULL)
    {
        sDw.dir = pdir;
        StartJump(j);
        return TRUE;
    }
    {
        const s8 *d = sStep[sDw.kind][pdir];
        s32 nx = sDw.x + d[0], ny = sDw.y + d[1], nz = sDw.z + d[2];
        u16 attr = PlatAttr(Plat(sDw.plat), nx, ny, nz);
        u8 mdir;
        if (attr == 0xFFFF)
        {
            s32 other = PlatAt(nx, ny, nz, sDw.kind);
            if (other >= 0)
                sDw.plat = other, attr = PlatAttr(Plat(other), nx, ny, nz);
        }
        sDw.dir = pdir;
        mdir = d[0] < 0 ? DIR_WEST : d[0] > 0 ? DIR_EAST : d[2] < 0 ? DIR_NORTH : d[2] > 0 ? DIR_SOUTH : dir;
        if (sSurfing && sDw.floor->hid == 577 && pdir == PD_RIGHT && sDw.x == 104 && sDw.y == 170 && sDw.z >= 76 && sDw.z <= 79
         && DW_CMDS_B4F_WATERFALL != 0xFFFF)
        {
            // B4F's waterfall, surfed into heading right (DistWorld_HandlePlayerMoved): down to B5F
            StartEvent(&sDwCmds[DW_CMDS_B4F_WATERFALL], 1);
            return TRUE;
        }
        if (attr != 0xFFFF && !(attr & ATTR_COLLISION) && ATTR_WATER(attr) != sSurfing)
        {
            if (sSurfing)
            {
                // (out of the water onto the platform: on foot again)
                sSurfing = FALSE;
                ObjectEventSetGraphicsId(obj, GetPlayerAvatarGraphicsIdByStateId(PLAYER_AVATAR_STATE_NORMAL));
            }
            else
                attr = ATTR_COLLISION;   // (water without SURF: A in front of it asks: S3D_Dw_FacingWater)
        }
        if (attr == 0xFFFF || (attr & ATTR_COLLISION))
        {
            // a bump: walking on the spot, slowly, with the sound (as the game's own collisions)
            TileWorld(sDw.x, sDw.y, sDw.z, sDw.from);
            memcpy(sDw.to, sDw.from, sizeof(sDw.to));
            sDw.t = 0;
            sDw.dur = 32;
            sDw.moving = TRUE;
            ObjectEventSetHeldMovement(obj, GetWalkInPlaceSlowMovementAction(dir));
            PlaySE(SE_WALL_HIT);
            return TRUE;
        }
        TileWorld(sDw.x, sDw.y, sDw.z, sDw.from);
        sDw.x = nx, sDw.y = ny, sDw.z = nz;
        TileWorld(nx, ny, nz, sDw.to);
        sDw.t = 0;
        sDw.dur = (heldKeys & B_BUTTON) && FlagGet(FLAG_SYS_B_DASH) ? 8 : 16;
        sDw.moving = TRUE;
        // the map position follows along x / z; up a wall the player walks on the spot
        if (d[0] != 0 || d[2] != 0)
            ObjectEventSetHeldMovement(obj, sDw.dur == 8 ? GetWalkFastMovementAction(mdir) : GetWalkNormalMovementAction(mdir));
        else
            ObjectEventSetHeldMovement(obj, sDw.dur == 8 ? GetWalkInPlaceFastMovementAction(dir) : GetWalkInPlaceNormalMovementAction(dir));
    }
    return TRUE;
}

static void Lerp(float t)
{
    u32 i;
    for (i = 0; i < 3; i++)
        sDw.pos[i] = sDw.from[i] + (sDw.to[i] - sDw.from[i]) * t;
}

void S3D_Dw_Update(void)
{
    u32 i;
    if (sDw.floor == NULL)
        return;
    HidePeople();
    FallUpdate();
    if (gS3dObjectHeightHook == NULL)
        gS3dObjectHeightHook = PersonHeight;
    if (DW_HAVE_DATA)
        DrawPlatforms(), ShadowUpdate();
    if (sDw.riding)
    {
        RideUpdate();
        return;
    }
    if (sEv.cmd != NULL)
    {
        if (sDw.jump != NULL)
            goto jumping;   // (an event's jump plays below)
        if (EventUpdate())
        {
            sEv.cmd = NULL;
            UnlockPlayerFieldControls();
        }
        if (sEv.cmd != NULL && sDw.jump == NULL)
            return;
    }
    if (sDw.kind == KIND_LAND && sDw.jump == NULL && DW_HAVE_DATA)
    {
        // a step onto an elevator platform rides it (HandleElevatorPlatformPropAnimatorAt)
        struct ObjectEvent *obj = &gObjectEvents[gPlayerAvatar.objectEventId];
        s32 x, y, z;
        LandTile(&x, &y, &z);
        if (!obj->singleMovementActive && !(obj->heldMovementActive && !obj->heldMovementFinished)   // (a step finished)
         && (x != sDw.last[0] || z != sDw.last[2]))   // (a new tile: by x / z, the ground's height read can differ)
        {
            const struct DwMoving *m = ElevatorAt(x, y, z);
            bool8 first = sDw.last[0] < 0;
            sDw.last[0] = x, sDw.last[1] = y, sDw.last[2] = z;
            if (!first && TwiceLedge(obj))
                return;
            if (!first && TileEvents(x, y, z))
                return;
            if (!first && Teleport(x, y, z, obj->facingDirection))
                return;
            if (m != NULL && !first)
            {
                s32 zero[3] = {0, 0, 0};
                StartRide(m, zero, TRUE);
                return;
            }
        }
    }
    if (sDw.camLeft > 0)
    {
        if (--sDw.camLeft <= 0)
            memcpy(sDw.cam, sDw.camTo, sizeof(sDw.cam));
        else
            for (i = 0; i < 3; i++)
                sDw.camF[i] += sDw.camStep[i], sDw.cam[i] = (s32)sDw.camF[i];
    }
jumping:
    if (sDw.jump != NULL)
    {
        float t = (float)(sDw.t + 1) / sDw.dur, hop[3] = {0}, a[3];
        u8 k = sDw.t * 16 / sDw.dur;
        Lerp(t);
        // the hop: away from the surface left (its "up"), Platinum's offsets along the jump's axis
        hop[sDw.jump->axis < 3 ? sDw.jump->axis : 1] = (sDw.jump->inv ? -1.0f : 1.0f) * sJumpHop[k < 16 ? k : 15];
        if (sDw.kind == KIND_WEST) a[0] = hop[1], a[1] = -hop[0], a[2] = hop[2];
        else if (sDw.kind == KIND_EAST) a[0] = -hop[1], a[1] = hop[0], a[2] = hop[2];
        else if (sDw.kind == KIND_CEILING) a[0] = hop[0], a[1] = -hop[1], a[2] = hop[2];
        else memcpy(a, hop, sizeof(a));
        for (i = 0; i < 3; i++)
            sDw.pos[i] += a[i];
        sDw.rot = sDw.rotFrom + (sDw.rotTo - sDw.rotFrom) * t;
        if (++sDw.t >= sDw.dur)
        {
            memcpy(sDw.pos, sDw.to, sizeof(sDw.pos));
            Landed();
        }
        return;
    }
    if (sDw.kind == KIND_LAND)
        return;
    if (sDw.moving)
    {
        Lerp((float)(sDw.t + 1) / sDw.dur);
        if (++sDw.t >= sDw.dur)
        {
            sDw.moving = FALSE;
            if (sDw.x != sDw.last[0] || sDw.y != sDw.last[1] || sDw.z != sDw.last[2])
            {
                sDw.last[0] = sDw.x, sDw.last[1] = sDw.y, sDw.last[2] = sDw.z;
                TileEvents(sDw.x, sDw.y, sDw.z);
            }
        }
    }
    else
        TileWorld(sDw.x, sDw.y, sDw.z, sDw.pos);
}

// The camera (s3d.c SetupView): Platinum's base angles plus the zone's; TARGET: the 3D position when off the ground
bool8 S3D_Dw_View(float *pitch, float *yaw, float *target)
{
    if (sDw.floor == NULL)
        return FALSE;
    *pitch = (s16)(BASE_PITCH + sDw.cam[0]) * 360.0f / 65536.0f;
    *yaw = (s16)sDw.cam[1] * 360.0f / 65536.0f;
    if (Off())
    {
        target[0] = sDw.pos[0];
        target[1] = sDw.pos[1];
        target[2] = sDw.pos[2];
        return 2;
    }
    return TRUE;
}

// The player's sprite on a wall, a ceiling or mid-jump (s3d.c S3D_PlaceSprite): stands at the 3D position, turned to
// its surface (an affine matrix on the sprite); FALSE: the ground's usual placement
bool8 S3D_Dw_PlayerSprite(u16 spriteId, s32 *outX, s32 *outY, float *depth)
{
    struct Sprite *s;
    float fx, fy, fd, ux, uy, a, len, n[3] = {0, 1, 0}, head[3];
    s32 i;
    if (!Off() || spriteId >= MAX_SPRITES || spriteId != gObjectEvents[gPlayerAvatar.objectEventId].spriteId)
    {
        if (spriteId < MAX_SPRITES && spriteId == gObjectEvents[gPlayerAvatar.objectEventId].spriteId && sDw.matrix != 0xFF)
        {
            gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_OFF;
            FreeOamMatrix(sDw.matrix);
            sDw.matrix = 0xFF;
        }
        return FALSE;
    }
    s = &gSprites[spriteId];
    if (!S3D_ProjectWorld(sDw.pos[0], sDw.pos[1], sDw.pos[2], &fx, &fy, &fd))
        return FALSE;
    // the sprite's "up": out of its surface, turned by the jump's angle on the way (a rotation about the z axis)
    a = sDw.rot * 3.14159265f / 180.0f;
    n[0] = sinf(a), n[1] = cosf(a), n[2] = 0;
    for (i = 0; i < 3; i++)
        head[i] = sDw.pos[i] + n[i] * 16.0f;
    if (!S3D_ProjectWorld(head[0], head[1], head[2], &ux, &uy, &len))
        ux = fx, uy = fy - 16;
    ux -= fx, uy -= fy;
    len = sqrtf(ux * ux + uy * uy);
    if (len < 0.01f)
        ux = 0, uy = -1, len = 1;
    ux /= len, uy /= len;
    if (sDw.matrix == 0xFF)
        sDw.matrix = AllocOamMatrix();
    if (sDw.matrix != 0xFF)
    {
        // screen up (0, -1) turned to (ux, uy): the angle, and the matrix that maps the screen back to the sprite
        float ang = atan2f(ux, -uy), c = cosf(ang), sn = sinf(ang);
        SetOamMatrix(sDw.matrix, (s16)(c * 256), (s16)(sn * 256), (s16)(-sn * 256), (s16)(c * 256));
        s->oam.affineMode = ST_OAM_AFFINE_DOUBLE;
        s->oam.matrixNum = sDw.matrix;
        // (double size: the box's corner is its centre less the sprite's size; the centre half a body up the turned
        // sprite from its feet)
        *outX = (s32)floorf(fx + ux * 16 + 0.5f) + s->centerToCornerVecX;
        *outY = (s32)floorf(fy + uy * 16 + 0.5f);
    }
    else
    {
        *outX = (s32)floorf(fx + 0.5f);
        *outY = (s32)floorf(fy + 0.5f);
    }
    *depth = fd;
    return TRUE;
}

// For tests: where the player is (mp dw)
void S3D_Dw_Put(s32 x, s32 y, s32 z, u8 dir)
{
    if (sDw.floor == NULL)
        return;
    Settle(x, y, z);
    sDw.dir = dir & 3;
    ObjectEventTurn(&gObjectEvents[gPlayerAvatar.objectEventId], FromPlatDir(sDw.dir));
    S3D_Dw_Print();
}

void S3D_Dw_Print(void)
{
    s32 x, y, z;
    if (sDw.floor == NULL)
    {
        printf("[dw] not in the DISTORTION WORLD\n");
        return;
    }
    if (sDw.kind == KIND_LAND)
        LandTile(&x, &y, &z);
    else
        x = sDw.x, y = sDw.y, z = sDw.z;
    printf("[dw] floor %d at %d,%d,%d kind %d platform %d camera %d %d %d\n", sDw.floor->hid, x, y, z, sDw.kind, sDw.plat,
           (int)(sDw.cam[0] / 256), (int)(sDw.cam[1] / 256), (int)(sDw.cam[2] / 256));
}
