// Moemon Co-op: where your friends are, and START > CO-OP > GO TO FRIEND.
//
// GO TO FRIEND flies you to the friend's town, or to the visited town nearest
// to them, like Fly does (only towns you've been to).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "overworld.h"
#include "region_map.h"
#include "sound.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"

mapsec_u16_t MP_RegionMap_NearestVisitedTown(mapsec_u16_t mapsec);
void MP_RegionMap_SetFlyWarp(mapsec_u16_t town);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// Maps per group (include/constants/map_groups.h), to check map numbers from the network
static const u8 sMapGroupSizes[MAP_GROUPS_COUNT] = {57, 5, 5, 6, 7, 8, 9, 7, 7, 14, 8, 17, 10, 23, 13, 15, 15, 2, 2, 2, 3, 1, 1, 1, 108, 61, 89, 2, 1, 13, 1, 1, 3, 1};

bool8 MP_IsValidMap(u8 mapGroup, u8 mapNum)
{
    return mapGroup < MAP_GROUPS_COUNT && mapNum < sMapGroupSizes[mapGroup];
}

// Other players in the room whose position is known; returns the count
u8 MP_Friends_List(u8 *ids)
{
    u8 i, n = 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasInfo && gMpPlayers[i].hasState)
            ids[n++] = i;
    return n;
}

// The region map section a player is in (MAPSEC_NONE if unknown)
mapsec_u16_t MP_Friends_Mapsec(u8 id)
{
    const struct MpState *s;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].present || !gMpPlayers[id].hasState)
        return MAPSEC_NONE;
    s = &gMpPlayers[id].state;
    if (!MP_IsValidMap(s->mapGroup, s->mapNum))
        return MAPSEC_NONE;
    return Overworld_GetMapHeaderByGroupAndId(s->mapGroup, s->mapNum)->regionMapSectionId;
}

// "ROUTE 104", "IN BATTLE" or "?" (game text)
void MP_Friends_LocationName(u8 id, u8 *dst)
{
    mapsec_u16_t sec;
    if (gMpPlayers[id].hasState && (gMpPlayers[id].state.flags & MPF_IN_BATTLE))
    {
        MP_AsciiToGame(dst, "IN BATTLE", 16);
        return;
    }
    sec = MP_Friends_Mapsec(id);
    if (sec >= MAPSEC_NONE)
    {
        MP_AsciiToGame(dst, "?", 4);
        return;
    }
    GetMapName(dst, sec, 0);
}

static bool8 CanFlyFromHere(void)
{
    if (FlagGet(FLAG_SYS_SAFARI_MODE) || gMapHeader.mapType == MAP_TYPE_UNDERWATER)
        return FALSE;
    if (gMapHeader.regionMapSectionId == MAPSEC_BATTLE_FRONTIER || gMapHeader.regionMapSectionId == MAPSEC_SECRET_BASE)
        return FALSE;
    return TRUE;
}

// The town GO TO FRIEND would take you to (MAPSEC_NONE: nowhere)
mapsec_u16_t MP_Friends_Destination(u8 id)
{
    return MP_RegionMap_NearestVisitedTown(MP_Friends_Mapsec(id));
}

// Starts the warp. Returns FALSE (and plays the "no" sound) when it can't.
bool8 MP_Friends_GoTo(u8 id)
{
    mapsec_u16_t town = MP_Friends_Destination(id);
    if (town >= MAPSEC_NONE || !CanFlyFromHere())
    {
        PlaySE(SE_FAILURE);
        return FALSE;
    }
    MP_RegionMap_SetFlyWarp(town);
    DoTeleportTileWarp();
    return TRUE;
}
