// Moemon Kai: BATTLE ISLAND, POKéMON STADIUM's home (docs/STADIUM.md). The BATTLE TICKET comes once any region's
// ELITE FOUR is beaten, brought by someone who fits the region the player is in (HOENN: SCOTT; JOHTO, KANTO: a
// sailor from the port; SINNOH: a letter); old saves get it the same way. Every region's port then offers the boat
// (data/scripts/battle_island.inc EventScript_BattleIslandOffer). The island's maps are shared (mp_region.c
// MP_Region_IsSharedMap): the player stays in their region, and the landing's sailor sails back to the port they
// left from.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/songs.h"

#define LOCALID_TICKET_COURIER 0xDD
#define ISLAND_LANDING_X 24
#define ISLAND_LANDING_Y 30

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_Voyage_Begin(bool8 eastbound, bool8 faded);   // mp_voyage.c
bool8 MP_Region_PortOf(u8 region, u16 *map, s16 *x, s16 *y);   // qz_specials.c

bool8 MP_BattleIsland_IsIsland(u8 group)
{
    return group == MAP_GROUP(MAP_BATTLE_ISLAND);
}

// EventScript_BattleIslandOffer: the boat from a port (the origin noted by the warp: mp_region.c MP_Region_NoteWarp)
void MP_BattleIsland_Sail(void)
{
    SetWarpDestination(MAP_GROUP(MAP_BATTLE_ISLAND), MAP_NUM(MAP_BATTLE_ISLAND), WARP_ID_NONE, ISLAND_LANDING_X,
                       ISLAND_LANDING_Y);
    MPNet_Log("battle island: sailing from map %d.%d", gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    MP_Voyage_Begin(TRUE, FALSE);
}

// BattleIsland_EventScript_Sailor: back to the port the player sailed from (the region's own port if that's lost)
void MP_BattleIsland_Return(void)
{
    if (gRegionState.originValid && !MP_BattleIsland_IsIsland(gRegionState.originGroup))
        SetWarpDestination(gRegionState.originGroup, gRegionState.originNum, WARP_ID_NONE, gRegionState.originX,
                           gRegionState.originY);
    else
    {
        u16 map;
        s16 x, y;
        if (!MP_Region_PortOf(MP_Region_Current(), &map, &x, &y))
            map = MAP_SLATEPORT_CITY_HARBOR, x = 8, y = 11;
        SetWarpDestination(map >> 8, map & 0xFF, WARP_ID_NONE, x, y);
    }
    MPNet_Log("battle island: sailing back");
    MP_Voyage_Begin(FALSE, FALSE);
}

// ---- the BATTLE TICKET's courier ----------------------------------------------------------------------------------

static EWRAM_DATA struct
{
    u8 state, objId, steps, walked, dir, slow;
    u16 timer, check;
} sT = {0};

static bool8 TicketOwed(void)
{
    return MP_Region_AnyCleared() && !CheckBagHasItem(ITEM_BATTLE_TICKET, 1);
}

static const char *Hello(void)
{
    switch (MP_Region_Current())
    {
    case REGION_JOHTO:
        return "Ahoy, @! I've come from OLIVINE's\nport with word for the CHAMPION.\f"
               "Far out at sea lies BATTLE ISLAND,\nhome of POKeMON STADIUM, where the\nstrongest TRAINERS of every region meet.\f"
               "Here's your ticket! Show it to the\nboat's crew at any port.";
    case REGION_KANTO:
        return "Ahoy, @! I sail out of VERMILION.\f"
               "Word is you beat the ELITE FOUR!\nThen BATTLE ISLAND's POKeMON STADIUM\nwants you.\f"
               "This ticket's yours. Any port's boat\nwill take you there!";
    case REGION_SINNOH:
        return "A letter from CANALAVE's harbor\nfound @!\f"
               "\"To the CHAMPION: the boat to BATTLE\nISLAND and its POKeMON STADIUM awaits\nyou at any port.\"\f"
               "A ticket was inside!";
    default:
        return "@! It's me, SCOTT!\f"
               "I heard you beat the ELITE FOUR! Have\nyou heard of BATTLE ISLAND?\f"
               "Its POKeMON STADIUM gathers the best\nTRAINERS of every region. I got this\nfor you!\f"
               "Show it at any port, and the boat will\ntake you there!";
    }
}

static void Say(const char *ascii)
{
    static u8 sText[480];
    char buf[480], name[16];
    const char *p;
    int n = 0;
    MP_GameToAscii(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    for (p = ascii; *p && n < (int)sizeof(buf) - 16; p++)   // "@": the player's name
    {
        if (*p == '@')
            n += snprintf(buf + n, sizeof(buf) - n, "%s", name);
        else
            buf[n++] = *p;
    }
    buf[n] = 0;
    MP_AsciiToGame(sText, buf, sizeof(sText) - 1);
    MP_ShowFieldMessageFitted(sText);
}

static bool8 TileFree(s16 x, s16 y, u8 elevation)
{
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (MapGridGetElevationAt(x, y) != elevation && MapGridGetElevationAt(x, y) != 0 && elevation != 0)
        return FALSE;
    return TRUE;
}

static u16 CourierGfx(void)
{
    return MP_Region_Current() == REGION_HOENN ? OBJ_EVENT_GFX_SCOTT : OBJ_EVENT_GFX_SAILOR;
}

static void Task_TicketCourier(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *courier = sT.objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[sT.objId] : NULL;
    static const s8 sDx[] = {0, 0, 0, -1, 1}, sDy[] = {0, 1, -1, 0, 0};

    switch (sT.state)
    {
    case 0:
    {
        static const u8 sOrder[] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
        s16 px = player->currentCoords.x, py = player->currentCoords.y;
        u8 i, best = 0, bestLen = 0;
        sT.objId = OBJECT_EVENTS_COUNT;
        if (MP_Region_Current() == REGION_SINNOH)   // (a letter: nobody walks up in SINNOH's 3D maps)
        {
            Say(Hello());
            sT.state = 3;
            break;
        }
        for (i = 0; i < ARRAY_COUNT(sOrder); i++)
        {
            u8 d = sOrder[i], len = 0;
            while (len < 4 && TileFree(px + sDx[d] * (len + 1), py + sDy[d] * (len + 1), player->currentElevation))
                len++;
            if (len > bestLen)
                bestLen = len, best = d;
        }
        if (bestLen > 0)
            sT.objId = SpawnSpecialObjectEventParameterized(CourierGfx(), MOVEMENT_TYPE_NONE, LOCALID_TICKET_COURIER,
                                                            px + sDx[best] * bestLen, py + sDy[best] * bestLen,
                                                            player->currentElevation);
        if (sT.objId >= OBJECT_EVENTS_COUNT)
        {
            UnlockPlayerFieldControls();   // (nowhere to stand: next time, somewhere roomier)
            DestroyTask(taskId);
            break;
        }
        sT.dir = (best == DIR_SOUTH) ? DIR_NORTH : (best == DIR_NORTH) ? DIR_SOUTH : (best == DIR_WEST) ? DIR_EAST : DIR_WEST;
        sT.steps = bestLen - 1;
        sT.walked = 0;
        SetObjectEventDirection(&gObjectEvents[sT.objId], sT.dir);
        ObjectEventTurn(player, best);
        PlaySE(SE_PIN);
        sT.timer = 0;
        sT.state++;
        break;
    }
    case 1:
        if (++sT.timer < 20)
            break;
        sT.state++;
        // fallthrough
    case 2:
        if (ObjectEventIsHeldMovementActive(courier) && !ObjectEventClearHeldMovementIfFinished(courier))
            break;
        if (sT.walked < sT.steps)
        {
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(sT.dir));
            sT.walked++;
            break;
        }
        Say(Hello());
        sT.state = 3;
        break;
    case 3:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        AddBagItem(ITEM_BATTLE_TICKET, 1);
        MPNet_Log("battle island: BATTLE TICKET handed over (region %d)", MP_Region_Current());
        PlayFanfare(MUS_OBTAIN_ITEM);
        Say("@ received the BATTLE TICKET!");
        sT.state = 4;
        break;
    case 4:
        if (IsTextPrinterActive(0) || !IsFanfareTaskInactive() || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        sT.walked = 0;
        sT.state = 5;
        break;
    case 5:
        if (courier == NULL)
        {
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            break;
        }
        if (ObjectEventIsHeldMovementActive(courier) && !ObjectEventClearHeldMovementIfFinished(courier))
            break;
        if (sT.walked < sT.steps + 1)
        {
            u8 away = (sT.dir == DIR_NORTH) ? DIR_SOUTH : (sT.dir == DIR_SOUTH) ? DIR_NORTH
                    : (sT.dir == DIR_WEST) ? DIR_EAST : DIR_WEST;
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(away));
            sT.walked++;
            break;
        }
        RemoveObjectEventByLocalIdAndMap(LOCALID_TICKET_COURIER, gSaveBlock1Ptr->location.mapNum,
                                         gSaveBlock1Ptr->location.mapGroup);
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        break;
    }
}

// Every frame (MP_FrameUpdate): every couple of seconds, free on a map outdoors or in, not on the island
void MP_BattleIsland_Update(void)
{
    if (gMain.inBattle || FuncIsActiveTask(Task_TicketCourier) || !MP_IsInOverworld() || !MP_IsLocalFree())
        return;
    if (++sT.check < 30)
        return;
    sT.check = 0;
    if (++sT.slow < 4)
        return;
    sT.slow = 0;
    if (!TicketOwed() || MP_BattleIsland_IsIsland(gSaveBlock1Ptr->location.mapGroup))
        return;
    memset(&sT, 0, sizeof(sT));
    LockPlayerFieldControls();
    CreateTask(Task_TicketCourier, 80);
}
