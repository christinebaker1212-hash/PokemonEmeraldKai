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
#include "fieldmap.h"
#include "sound.h"
#include "sprite.h"
#include "constants/event_object_movement.h"
#include "constants/songs.h"
#include "constants/maps.h"
#include "constants/map_groups.h"
#include "s3d.h"

void MoveCameraAndRedrawMap(int deltaX, int deltaY);   // (field_camera.c)

struct DwBounds { s16 x, y, z, sx, sy, sz; };
struct DwFloor { u16 map, hid; s16 off[3]; u8 plat, nplat, jump, njump, cam, ncam; };
struct DwPlatform { s16 kind; u16 attr; struct DwBounds b; u16 vert, horiz; };
struct DwJump { s16 dir; struct DwBounds b; s16 d[3]; s16 rot, steps; u16 axis, inv; s16 face, kind, idx; };
struct DwCam { struct DwBounds b; u16 ang[3]; s16 dir; s32 steps; };

#include "../../sinnoh/platinum_dw.h"

// Platinum's FaceDirection
enum { PD_UP, PD_DOWN, PD_LEFT, PD_RIGHT, PD_NONE };
// Platinum's floating platform kinds (FLOATING_PLATFORM_KIND_*); KIND_LAND: on the map's own ground
enum { KIND_FLOOR, KIND_WEST, KIND_EAST, KIND_CEILING, KIND_LAND };

#define BASE_PITCH (-10750)   // DISTORTION_WORLD_CAMERA_BASE_ANGLE_X (the field camera's)
#define ATTR_COLLISION 0x8000

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
} sDw = {.matrix = 0xFF};

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

// On the map's own ground or up on a platform / mid-jump: the 3D position drives the view and the player's sprite
static bool8 Off(void)
{
    return sDw.floor != NULL && (sDw.kind != KIND_LAND || sDw.jump != NULL);
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

static void Landed(void)
{
    const struct DwJump *j = sDw.jump;
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
    PlaySE(SE_M_SAND_ATTACK);   // (Platinum's SEQ_SE_PL_SUTYA2: the dust of a landing)
    if (getenv("MOEMON_TEST"))
        printf("[dw] landed at %d,%d,%d on kind %d platform %d\n", sDw.x, sDw.y, sDw.z, sDw.kind, sDw.plat);
}

void S3D_Dw_OnMapLoad(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    const struct DwFloor *f = FloorFor(map);
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
    if (sDw.matrix != 0xFF)
    {
        FreeOamMatrix(sDw.matrix);
        sDw.matrix = 0xFF;
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
    if (sDw.camLeft > 0)
    {
        if (--sDw.camLeft <= 0)
            memcpy(sDw.cam, sDw.camTo, sizeof(sDw.cam));
        else
            for (i = 0; i < 3; i++)
                sDw.camF[i] += sDw.camStep[i], sDw.cam[i] = (s32)sDw.camF[i];
    }
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
            sDw.moving = FALSE;
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
