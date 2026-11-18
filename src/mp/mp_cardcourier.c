// Moemon Kai: the CARD CLUB's courier (docs/TCG_REGIONS.md step 6). JOHTO's, KANTO's and SINNOH's special card trainers
// played HOENN's decks before they had decks and promo cards of their own; a save that beat one then is owed that
// trainer's promo (tcg_field.c Tcg_CourierOwed, from the card record), once. A CARD CLUB member walks up with them
// wherever the player is free (SINNOH: a parcel, nobody walks up in its 3D maps), as BATTLE ISLAND's ticket courier does
// (mp_battleisland.c).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "o3d.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_message_box.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "tcg/integration.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/songs.h"

#define LOCALID_CARD_COURIER 0xDC

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
bool8 MP_BattleIsland_IsIsland(u8 group);

static EWRAM_DATA struct
{
    u8 state, objId, steps, walked, dir, slow;
    u16 timer, check;
} sC = {0};

static void Say(const char *ascii)
{
    static u8 sText[640];
    char buf[640], name[16];
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

static const char *Hello(void)
{
    if (MP_Region_Current() == REGION_SINNOH)
        return "A parcel from the CARD CLUB found @!\f"
               "\"The TRAINERS you beat at cards now\nhand out promo cards of their own.\nThese are yours.\"";
    return "@! I'm from the CARD CLUB.\f"
           "The TRAINERS you beat at cards now\nhand out promo cards of their own.\f"
           "They asked me to bring you yours!";
}

static bool8 TileFree(s16 x, s16 y, u8 elevation)
{
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (MapGridGetElevationAt(x, y) != elevation && MapGridGetElevationAt(x, y) != 0 && elevation != 0)
        return FALSE;
    return TRUE;
}

static void Task_CardCourier(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *courier = sC.objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[sC.objId] : NULL;
    static const s8 sDx[] = {0, 0, 0, -1, 1}, sDy[] = {0, 1, -1, 0, 0};

    switch (sC.state)
    {
    case 0:
    {
        static const u8 sOrder[] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
        s16 px = player->currentCoords.x, py = player->currentCoords.y;
        u8 i, best = 0, bestLen = 0;
        sC.objId = OBJECT_EVENTS_COUNT;
        if (MP_Region_Current() == REGION_SINNOH)
        {
            Say(Hello());
            sC.state = 3;
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
            sC.objId = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_MANIAC, MOVEMENT_TYPE_NONE, LOCALID_CARD_COURIER,
                                                            px + sDx[best] * bestLen, py + sDy[best] * bestLen,
                                                            player->currentElevation);
        if (sC.objId >= OBJECT_EVENTS_COUNT)
        {
            UnlockPlayerFieldControls();   // (nowhere to stand: next time, somewhere roomier)
            DestroyTask(taskId);
            break;
        }
        sC.dir = (best == DIR_SOUTH) ? DIR_NORTH : (best == DIR_NORTH) ? DIR_SOUTH : (best == DIR_WEST) ? DIR_EAST : DIR_WEST;
        sC.steps = bestLen - 1;
        sC.walked = 0;
        SetObjectEventDirection(&gObjectEvents[sC.objId], sC.dir);
        ObjectEventTurn(player, best);
        PlaySE(SE_PIN);
        sC.timer = 0;
        sC.state++;
        break;
    }
    case 1:
        if (++sC.timer < 20)
            break;
        sC.state++;
        // fallthrough
    case 2:
        if (ObjectEventIsHeldMovementActive(courier) && !ObjectEventClearHeldMovementIfFinished(courier))
            break;
        if (sC.walked < sC.steps)
        {
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(sC.dir));
            sC.walked++;
            break;
        }
        Say(Hello());
        sC.state = 3;
        break;
    case 3:
    {
        char list[480], msg[560];
        unsigned n;
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        n = Tcg_CourierDeliver(list, sizeof(list));
        MPNet_Log("card courier: %u promo card(s) handed over", n);
        PlayFanfare(MUS_OBTAIN_ITEM);
        if (n == 1)
            snprintf(msg, sizeof(msg), "@ received a promo card!\f%s", list);
        else
            snprintf(msg, sizeof(msg), "@ received %u promo cards!\f%s", n, list);
        Say(msg);
        sC.state = 4;
        break;
    }
    case 4:
        if (IsTextPrinterActive(0) || !IsFanfareTaskInactive() || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        sC.walked = 0;
        sC.state = 5;
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
        if (sC.walked < sC.steps + 1)
        {
            u8 away = (sC.dir == DIR_NORTH) ? DIR_SOUTH : (sC.dir == DIR_SOUTH) ? DIR_NORTH
                    : (sC.dir == DIR_WEST) ? DIR_EAST : DIR_WEST;
            ObjectEventSetHeldMovement(courier, GetWalkNormalMovementAction(away));
            sC.walked++;
            break;
        }
        RemoveObjectEventByLocalIdAndMap(LOCALID_CARD_COURIER, gSaveBlock1Ptr->location.mapNum,
                                         gSaveBlock1Ptr->location.mapGroup);
        UnlockPlayerFieldControls();
        DestroyTask(taskId);
        break;
    }
}

// Every frame (MP_FrameUpdate): every few seconds, free on a map, not on BATTLE ISLAND or in ORRE
void MP_CardCourier_Update(void)
{
    if (gMain.inBattle || FuncIsActiveTask(Task_CardCourier) || !MP_IsInOverworld() || !MP_IsLocalFree())
        return;
    if (++sC.check < 30)
        return;
    sC.check = 0;
    if (++sC.slow < 6)
        return;
    sC.slow = 0;
    if (MP_BattleIsland_IsIsland(gSaveBlock1Ptr->location.mapGroup) || O3D_IsOrreGroup(gSaveBlock1Ptr->location.mapGroup)
     || !Tcg_CourierOwed())
        return;
    // (headless tests: only the ones about the courier)
    if (getenv("MOEMON_TEST") && !getenv("MOEMON_TEST_CARD_COURIER"))
        return;
    memset(&sC, 0, sizeof(sC));
    MPNet_Log("card courier: on the way, %u owed", Tcg_CourierOwed());
    LockPlayerFieldControls();
    CreateTask(Task_CardCourier, 80);
}
