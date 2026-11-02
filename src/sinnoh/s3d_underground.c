// Moemon Kai: SINNOH's Underground, for its digging and SPIRITOMB only (the user, 2026-11-02: "The Underground will ONLY
// be implemented for its Fossil-hunting features and Spiritomb prerequisites only - the Wireless feature is superceded
// by CO-OP"). As Platinum has it (pret/pokeplatinum):
// - the EXPLORER KIT used outdoors takes the player down (field_map_change.c MapChangeUndergroundContext_New: the
//   region's cell to the Underground's, two cells to one, tile 8 or 23 by the cell's parity); used below, up again to
//   where they went down (Platinum's GO UP);
// - dig spots: once a day, 24 clusters of 6-11 spots on walls beside open floor (underground/mining.c
//   Mining_SpawnMiningSpotsAndTraps; Platinum's 25 less a sphere-dependent count: no spheres here), up to 250, each
//   gone once dug; the DS showed them on its radar: here a sparkle on the wall near the player;
// - A facing a spot: the digging screen (s3d_mining.c);
// - SPIRITOMB's counter (Platinum's, read by the HALLOWED TOWER's script: VAR_SINNOH_SPIRITOMB, GetSpiritombCounter in
//   tools/sinnoh/gen_scripts.py): one for each friend talked to in the Underground on a visit (Platinum's first talk
//   with each), friends being co-op's.
// Kept in "<save>.region" chunk UNDG (mp_region.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "sound.h"
#include "constants/field_effects.h"
#include "constants/map_groups.h"
#include "constants/songs.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"
#include "s3d_underground.h"

#define UNDG_MAGIC 0x47444E55   // "UNDG"
#define SPOTS 250
#define MAIN_X0 32              // (Platinum's UNDERGROUND_MAIN_AREA_START_X / _Z: the secret bases' strip left out)
#define MAIN_Z0 64

static struct
{
    u32 magic;
    s8 retGroup, retNum;
    u8 inside, everDug;
    s16 retX, retY;
    u32 day;                   // (the spots' day: new ones each day)
    u8 dug[(SPOTS + 7) / 8];
    u8 pad;
    u16 plates;                // (the plates dug: Platinum digs each but once)
} sU;

extern const u16 gSinnohUndergroundFlags[];   // (Platinum's FLAG_ENTERED_UNDERGROUND, FLAG_DIGGING_FOR_FOSSILS: the
                                              // UNDERGROUND MAN's script reads them; tools/sinnoh/contest_data.py)

static struct { u16 x, y; } sSpot[SPOTS];
static u16 sSpots;
static u32 sSpotsDay = 0xFFFFFFFF;
static u8 sTalked;             // (the friends talked to on this visit)
static u16 sSparkleTimer;


const void *Sinnoh_UndergroundData(u32 *size)
{
    sU.magic = UNDG_MAGIC;
    *size = sizeof(sU);
    return &sU;
}

void Sinnoh_UndergroundLoad(const void *data, u32 size)
{
    memset(&sU, 0, sizeof(sU));
    if (size == sizeof(sU) && ((const u32 *)data)[0] == UNDG_MAGIC)
        memcpy(&sU, data, size);
    sSpotsDay = 0xFFFFFFFF;
}

void Sinnoh_UndergroundReset(void)
{
    memset(&sU, 0, sizeof(sU));
    sSpotsDay = 0xFFFFFFFF;
}

bool8 Sinnoh_InUnderground(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_SINNOH_UNDERGROUND)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_SINNOH_UNDERGROUND);
}

bool8 Sinnoh_UndergroundEverDug(void)
{
    return sU.everDug;
}

void Sinnoh_UndergroundDugOnce(void)
{
    sU.everDug = TRUE;
}

bool8 Sinnoh_UndergroundPlateDug(u16 plate)
{
    return plate < 16 && (sU.plates >> plate) & 1;
}

void Sinnoh_UndergroundSetPlateDug(u16 plate)
{
    if (plate < 16)
        sU.plates |= 1 << plate;
}

// ---- in and out

// the EXPLORER KIT works outdoors in SINNOH (Platinum: on the region's matrix) and in the Underground
const u8 *Sinnoh_ExplorerKitCantUse(void)
{
    u16 mat, cx, cz;
    static const u8 sCant[] = _("This can't be used here.");
    if (Sinnoh_InUnderground())
        return NULL;
    if (!S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup) || !S3D_MapCell(&mat, &cx, &cz) || mat != 0)
        return sCant;
    return NULL;
}

// (special, waitstate: Sinnoh_EventScript_ExplorerKit)
void Sinnoh_ExplorerKitUse(void)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 px = me->currentCoords.x - MAP_OFFSET, py = me->currentCoords.y - MAP_OFFSET;
    if (Sinnoh_InUnderground())
    {
        // (GO UP: back where the player went down)
        sU.inside = FALSE;
        if (S3D_IsSinnohGroup(sU.retGroup))
            SetWarpDestination(sU.retGroup, sU.retNum, WARP_ID_NONE, sU.retX, sU.retY);
        else
            SetWarpDestinationToLastHealLocation();   // (never went down with the kit: the last POKéMON CENTER)
        MPNet_Log("sinnoh underground: up to %d.%d %d,%d", sU.retGroup, sU.retNum, sU.retX, sU.retY);
    }
    else
    {
        u16 mat, cx, cz;
        s32 wx, wz, mx, mz, x, z;
        S3D_MapCell(&mat, &cx, &cz);
        wx = cx * 32 + px;
        wz = cz * 32 + py;
        mx = wx / 32 - 1;
        mz = wz / 32 - 6;
        if (mx < 0)
            mx = 0;
        if (mz < 0)
            mz = 0;
        x = (mx / 2 + 1) * 32 + (mx % 2 == 0 ? 8 : 23);
        z = (mz / 2 + 3) * 32 + (mz % 2 == 0 ? 8 : 23);
        sU.retGroup = gSaveBlock1Ptr->location.mapGroup;
        sU.retNum = gSaveBlock1Ptr->location.mapNum;
        sU.retX = px;
        sU.retY = py;
        sU.inside = TRUE;
        SetWarpDestination(MAP_GROUP(MAP_SINNOH_UNDERGROUND), MAP_NUM(MAP_SINNOH_UNDERGROUND), WARP_ID_NONE, x, z);
        MPNet_Log("sinnoh underground: down from %d,%d (region %d,%d) to %d,%d", px, py, wx, wz, x, z);
    }
    DoWarp();
    ResetInitialPlayerAvatarState();
}

// ---- the dig spots

static bool8 Wall(s32 x, s32 y)
{
    return MapGridGetCollisionAt(x + MAP_OFFSET, y + MAP_OFFSET) != 0;
}

static bool8 CanSpot(s32 x, s32 z)
{
    if (x < MAIN_X0 || z < MAIN_Z0 || x > 478 || z > 478)
        return FALSE;
    if (!Wall(x, z))
        return FALSE;
    return !Wall(x, z + 1) || !Wall(x, z - 1) || !Wall(x + 1, z) || !Wall(x - 1, z);
}

static u32 sRng;
static u16 Rand16(u16 n)
{
    sRng = sRng * 1103515245 + 24691;
    return n ? (sRng >> 16) % n : 0;
}

static void SpawnSpots(u32 day)
{
    u32 cluster;
    sRng = day * 2654435761u ^ gSaveBlock2Ptr->playerTrainerId[0] ^ gSaveBlock2Ptr->playerTrainerId[1] << 8;
    sSpots = 0;
    for (cluster = 0; cluster < 24 && sSpots < SPOTS; cluster++)
    {
        u16 cx, cz, n, i, j, tries = 0;
        do
        {
            cx = Rand16(13 * 32) + MAIN_X0;
            cz = Rand16(13 * 32) + MAIN_Z0;
        } while (!CanSpot(cx, cz) && ++tries < 2000);
        if (tries >= 2000)
            continue;
        n = Rand16(6) + 6;
        for (i = 0; i < n && sSpots < SPOTS; i++)
            for (j = 0; j < 100; j++)
            {
                s32 x = Rand16(20) + cx - 10, z = Rand16(20) + cz - 10;
                if (CanSpot(x, z))
                {
                    sSpot[sSpots].x = x;
                    sSpot[sSpots].y = z;
                    sSpots++;
                    break;
                }
            }
    }
    MPNet_Log("sinnoh underground: %d dig spots for day %u", sSpots, day);
}

static u32 Today(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days;
}

void Sinnoh_Underground_MapLoaded(void)
{
    u32 day;
    if (!Sinnoh_InUnderground())
    {
        sTalked = 0;   // (out of the Underground: the next visit's talks count again)
        return;
    }
    if (gSinnohUndergroundFlags[0])
        FlagSet(gSinnohUndergroundFlags[0]);
    day = Today();
    if (sU.day != day)
    {
        sU.day = day;
        memset(sU.dug, 0, sizeof(sU.dug));
    }
    if (sSpotsDay != day)
    {
        SpawnSpots(day);
        sSpotsDay = day;
    }
}

s32 Sinnoh_UndergroundSpotAt(s16 x, s16 y)
{
    u16 i;
    if (!Sinnoh_InUnderground())
        return -1;
    for (i = 0; i < sSpots; i++)
        if (sSpot[i].x == x && sSpot[i].y == y && !((sU.dug[i / 8] >> (i % 8)) & 1))
            return i;
    return -1;
}

void Sinnoh_UndergroundSpotDug(s32 spot)
{
    if (spot >= 0 && spot < SPOTS)
        sU.dug[spot / 8] |= 1 << (spot % 8);
}

// the spots near the player sparkle now and then (the DS's radar on its second screen)
void Sinnoh_Underground_Update(void)
{
    struct ObjectEvent *me;
    u16 i;
    if (!Sinnoh_InUnderground() || ++sSparkleTimer < 40)
        return;
    sSparkleTimer = 0;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < sSpots; i++)
    {
        s32 dx = sSpot[i].x - (me->currentCoords.x - MAP_OFFSET), dy = sSpot[i].y - (me->currentCoords.y - MAP_OFFSET);
        if ((sU.dug[i / 8] >> (i % 8)) & 1)
            continue;
        if (dx >= -7 && dx <= 7 && dy >= -5 && dy <= 5)
        {
            gFieldEffectArguments[0] = sSpot[i].x;
            gFieldEffectArguments[1] = sSpot[i].y;
            gFieldEffectArguments[2] = 2;
            FieldEffectStart(FLDEFF_SPARKLE);
        }
    }
}

// ---- SPIRITOMB

static void SpiritombAdd(void)
{
    if (VarGet(VAR_SINNOH_SPIRITOMB) < 0xFFFF)
        VarSet(VAR_SINNOH_SPIRITOMB, VarGet(VAR_SINNOH_SPIRITOMB) + 1);
    MPNet_Log("sinnoh underground: SPIRITOMB's counter %d", VarGet(VAR_SINNOH_SPIRITOMB));
}

// A facing a co-op friend in the Underground: Platinum's first talk with each on a visit (field_control_avatar.c; not
// the A press itself: whatever co-op does with it goes on)
void MP_Underground_TryTalk(void)
{
    struct ObjectEvent *me;
    s16 x, y;
    u8 i;
    if (!Sinnoh_InUnderground() || !MP_IsConnected())
        return;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    x = me->currentCoords.x;
    y = me->currentCoords.y;
    MoveCoords(me->facingDirection, &x, &y);
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 rx, ry;
        if (i == gMpLocalId || !gMpPlayers[i].present || (sTalked >> i) & 1)
            continue;
        if (!MP_GetRemoteMapPos(i, &rx, &ry) || rx != x || ry != y)
            continue;
        sTalked |= 1 << i;
        SpiritombAdd();
    }
}

// A facing a dig spot: Platinum's "The wall is bulging slightly." and the digging (data/scripts/sinnoh_places.inc)
static s32 sDigSpot = -1;

const u8 *Sinnoh_UndergroundDigScript(s16 x, s16 y)
{
    extern const u8 Sinnoh_EventScript_UndergroundDig[];
    s32 spot = Sinnoh_UndergroundSpotAt(x, y);
    if (spot < 0)
        return NULL;
    sDigSpot = spot;
    return Sinnoh_EventScript_UndergroundDig;
}

s32 Sinnoh_UndergroundDigSpot(void)
{
    return sDigSpot;
}

void Sinnoh_UndergroundDebug(void)
{
    printf("[test] underground: inside %d spots %d dug-ever %d return %d.%d %d,%d spiritomb %d\n", Sinnoh_InUnderground(), sSpots,
           sU.everDug, sU.retGroup, sU.retNum, sU.retX, sU.retY,
           VarGet(VAR_SINNOH_SPIRITOMB));
}

// (tests) the nearest spot to the player, and the tile in front of it to stand on facing it
bool8 Sinnoh_UndergroundNearestSpot(s16 *x, s16 *y, u8 *dir)
{
    struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
    s32 best = 0x7FFFFFFF;
    u16 i, k;
    static const s8 sD[4][3] = {{0, 1, DIR_NORTH}, {0, -1, DIR_SOUTH}, {1, 0, DIR_WEST}, {-1, 0, DIR_EAST}};
    for (i = 0; i < sSpots; i++)
        for (k = 0; k < 4; k++)
        {
            s32 fx = sSpot[i].x + sD[k][0], fy = sSpot[i].y + sD[k][1];
            s32 d = (fx - (me->currentCoords.x - MAP_OFFSET)) * (fx - (me->currentCoords.x - MAP_OFFSET))
                  + (fy - (me->currentCoords.y - MAP_OFFSET)) * (fy - (me->currentCoords.y - MAP_OFFSET));
            if (!Wall(fx, fy) && d < best && !((sU.dug[i / 8] >> (i % 8)) & 1))
            {
                best = d;
                *x = fx;
                *y = fy;
                *dir = sD[k][2];
            }
        }
    return best != 0x7FFFFFFF;
}
