// Moemon Co-op: where your friends are, and CO-OP menu > GO TO FRIEND.
//
// GO TO FRIEND flies you to the friend's town, or to the visited town nearest
// to them, like Fly does (only towns you've been to).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_garden.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "overworld.h"
#include "region_map.h"
#include "mp_region.h"
#include "o3d.h"
#include "sound.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/maps.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"

mapsec_u16_t MP_RegionMap_NearestVisitedTown(mapsec_u16_t mapsec);
void MP_RegionMap_SetFlyWarp(mapsec_u16_t town);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// Maps per group, to check map numbers from the network: generated from data/maps/map_groups.json (sMapGroupSizes,
// tools/phone/gen_map_sizes.py). It was written by hand and knew only HOENN's 34 groups: every friend in KANTO, JOHTO,
// SINNOH or ORRE was "?" and GO TO FRIEND couldn't go (the user, 2026-11-11)
#include "../data/mp_map_group_sizes.h"

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

// In the MOéMON GARDEN (every POKéMON CENTER's second floor: its map section is LITTLEROOT's, but it's no town)
bool8 MP_Friends_InGarden(u8 id)
{
    return id < MP_MAX_PLAYERS && gMpPlayers[id].present && gMpPlayers[id].hasState
        && gMpPlayers[id].state.mapGroup == MAP_GROUP(MAP_MOEMON_GARDEN) && gMpPlayers[id].state.mapNum == MAP_NUM(MAP_MOEMON_GARDEN);
}

// "ROUTE 104", "MOéMON GARDEN", "IN BATTLE" or "?" (game text)
void MP_Friends_LocationName(u8 id, u8 *dst)
{
    mapsec_u16_t sec;
    if (gMpPlayers[id].hasState && (gMpPlayers[id].state.flags & MPF_IN_BATTLE))
    {
        MP_AsciiToGame(dst, "IN BATTLE", 16);
        return;
    }
    if (MP_Friends_InGarden(id))
    {
        static const u8 sGarden[] = _("MOéMON GARDEN");
        StringCopy(dst, sGarden);
        return;
    }
    sec = MP_Friends_Mapsec(id);
    if (sec == MAPSEC_NONE)
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

// The town GO TO FRIEND would take you to (MAPSEC_NONE: nowhere). Never into or out of ORRE (the user, 2026-11-11:
// it's reached by boat only), and only a friend in the region you're in (another region is a voyage away)
bool8 MP_RegionMap_TownIsQz(mapsec_u16_t town);
bool8 MP_RegionMap_IsQzPlace(mapsec_u16_t mapsec);
// The place on KANTO's / JOHTO's Town Map a map is: its own, or the one its doors lead out to (the fewest warps)
static mapsec_u16_t OutdoorPlace(u8 group, u8 num)
{
    u16 queue[48], head = 0, tail = 0, k;
    queue[tail++] = (group << 8) | num;
    while (head < tail)
    {
        u8 g = queue[head] >> 8, n = queue[head] & 0xFF, i;
        const struct MapHeader *h = MP_IsValidMap(g, n) ? Overworld_GetMapHeaderByGroupAndId(g, n) : NULL;
        head++;
        if (h == NULL)
            continue;
        if (MP_RegionMap_IsQzPlace(h->regionMapSectionId))
            return h->regionMapSectionId;
        for (i = 0; h->events && i < h->events->warpCount; i++)
        {
            const struct WarpEvent *e = &h->events->warps[i];
            u16 key = (e->mapGroup << 8) | e->mapNum;
            if (e->mapNum == MAP_NUM(MAP_DYNAMIC) || !MP_IsValidMap(e->mapGroup, e->mapNum))
                continue;
            for (k = 0; k < tail && queue[k] != key; k++)
                ;
            if (k == tail && tail < ARRAY_COUNT(queue))
                queue[tail++] = key;
        }
    }
    return MAPSEC_NONE;
}

// GO TO's town for someone on that map. KANTO's / JOHTO's: a place their Town Map doesn't list (a building, a cave,
// the ROCKET HIDEOUT) is where its doors lead out to, and the town is always theirs: HOENN's Town Map read with their
// numbers flew the player to FALLARBOR (the user, 2026-11-23: "when we selected GO TO, it took us to Fallarbor, in
// Hoenn"; `mp nearestaudit h` checks every map)
mapsec_u16_t MP_Friends_DestinationOfMap(u8 group, u8 num)
{
    if (MP_Region_OfGroup(group) != REGION_HOENN)
    {
        mapsec_u16_t town = MP_RegionMap_NearestVisitedTown(OutdoorPlace(group, num));
        return town != MAPSEC_NONE && MP_RegionMap_TownIsQz(town) ? town : MAPSEC_NONE;
    }
    return MP_RegionMap_NearestVisitedTown(Overworld_GetMapHeaderByGroupAndId(group, num)->regionMapSectionId);
}

mapsec_u16_t MP_Friends_Destination(u8 id)
{
    const struct MpState *st;
    if (id >= MP_MAX_PLAYERS || !gMpPlayers[id].hasState)
        return MAPSEC_NONE;
    st = &gMpPlayers[id].state;
    if (!MP_IsValidMap(st->mapGroup, st->mapNum) || O3D_IsOrreGroup(st->mapGroup)
     || O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup))
        return MAPSEC_NONE;
    if (!MP_Region_IsSharedMap(st->mapGroup, st->mapNum) && MP_Region_OfGroup(st->mapGroup) != MP_Region_Current())
        return MAPSEC_NONE;
    return MP_Friends_DestinationOfMap(st->mapGroup, st->mapNum);
}

// Starts the warp. Returns FALSE (and plays the "no" sound) when it can't.
bool8 MP_Friends_GoTo(u8 id)
{
    mapsec_u16_t town = MP_Friends_Destination(id);
    if (town == MAPSEC_NONE || !CanFlyFromHere())
    {
        PlaySE(SE_FAILURE);
        return FALSE;
    }
    MP_RegionMap_SetFlyWarp(town);
    DoTeleportTileWarp();
    return TRUE;
}

// Buddy bonus: extra EXP and prize money for playing together.
// +5% for every friend online, +10% more for every friend on your map (or a
// map right next to it). With 3 friends right there it's +45%.
u16 MP_BuddyBonusPercent(void)
{
    u8 i;
    u16 pct = 0;
    if (!MP_IsConnected() || gMpLocalId < 0)
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        s16 x, y;
        if (i == gMpLocalId || !gMpPlayers[i].present || !gMpPlayers[i].hasInfo)
            continue;
        pct += 5;
        if (gMpPlayers[i].hasState && MP_GetRemoteMapPos(i, &x, &y))
            pct += 10;
    }
    return pct;
}
