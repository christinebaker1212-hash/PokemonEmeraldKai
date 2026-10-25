// Moemon Co-op: TMs are reusable, as in later games (party_menu.c Task_LearnedMove keeps the TM; shop.c sells a TM
// once). A save from before that (1.18.1) may have used some up, so the LILYCOVE LOTTERY CORNER's clerk brings them
// back as "lottery winnings" nobody collected: once per save, the first time the player is free on the map, she walks
// up (as PROF. BIRCH does with the CANDY JAR) and hands over every TM the save got and has in neither the BAG nor the
// PC. What it got: the event flags of the TMs the game hands out once (mp_tm_sources.h, read from the maps by
// tools/moemon_tms/gen_tm_sources.py) and the TRICK HOUSE's puzzle 5 prize (its variables). Shop and prize TMs leave no
// trace, so they can't come back. A new game never sees her (new_game.c marks it done: nothing was used up, and the
// harness's made-up saves with story flags set count as new games too).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
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
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "mp_tm_sources.h"

#define REFUND_KEY "tm:refund"
#define LOCALID_CLERK 0xDB
#define MAX_REFUND NUM_TECHNICAL_MACHINES

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

static EWRAM_DATA struct
{
    u8 state;
    u8 objId;
    u8 steps, walked;
    u8 dir;          // her way toward the player
    u16 timer;
    u8 count;
    u8 tms[MAX_REFUND];   // TM numbers (1-50)
} sClerk = {0};

static bool8 HasItemAnywhere(u16 item)
{
    return CheckBagHasItem(item, 1) || CheckPCHasItem(item, 1);
}

// Did this save get TM n (1-50)? (its flags; the TRICK HOUSE's puzzle 5 prize by its variables: past puzzle 5, or
// at it with the prize picked up)
static bool8 GotTm(u8 n)
{
    u16 i;
    for (i = 0; i < ARRAY_COUNT(sTmSources); i++)
        if (sTmSources[i].tm == n && FlagGet(sTmSources[i].flag))
            return TRUE;
    if (n == 12)
    {
        u16 level = VarGet(VAR_TRICK_HOUSE_LEVEL);
        return level > 5 || (level == 5 && VarGet(VAR_TRICK_HOUSE_PRIZE_PICKUP) == 0);
    }
    return FALSE;
}

// The TMs to bring back: got, and gone
static u8 FindUsedUp(u8 *out)
{
    u8 n, count = 0;
    for (n = 1; n <= NUM_TECHNICAL_MACHINES; n++)
        if (GotTm(n) && !HasItemAnywhere(ITEM_TM01 + n - 1))
            out[count++] = n;
    return count;
}

static bool8 TileFree(s16 x, s16 y, u8 elevation)
{
    if (MapGridGetCollisionAt(x, y) != 0 || GetObjectEventIdByXY(x, y) != OBJECT_EVENTS_COUNT)
        return FALSE;
    if (MapGridGetElevationAt(x, y) != elevation && MapGridGetElevationAt(x, y) != 0 && elevation != 0)
        return FALSE;
    return TRUE;
}

static void ClerkText(const char *ascii)
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

// "TM03, TM08 and TM27", a line of at most ~30 characters each
static void ListTms(char *out, u32 size)
{
    u32 len = 0, line = 0;
    u8 i;
    out[0] = 0;
    for (i = 0; i < sClerk.count && len + 12 < size; i++)
    {
        const char *sep = i == 0 ? "" : i == sClerk.count - 1 ? " and " : ", ";
        if (line + strlen(sep) + 4 > 30)
        {
            sep = i == sClerk.count - 1 ? "\nand " : ",\n";
            line = 0;
        }
        len += snprintf(out + len, size - len, "%sTM%02d", sep, sClerk.tms[i]);
        line += strlen(sep) + 4;
    }
}

static void GiveTms(void)
{
    u8 i;
    for (i = 0; i < sClerk.count; i++)
        AddBagItem(ITEM_TM01 + sClerk.tms[i] - 1, 1);
    MP_SaveData_Set(REFUND_KEY, 1);
    MPNet_Log("tms: the lottery clerk gave back %d used-up TMs", sClerk.count);
}

static void Task_LotteryClerk(u8 taskId)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    struct ObjectEvent *clerk = sClerk.objId < OBJECT_EVENTS_COUNT ? &gObjectEvents[sClerk.objId] : NULL;
    static const s8 sDx[] = {0, 0, 0, -1, 1}, sDy[] = {0, 1, -1, 0, 0};   // DIR_SOUTH, NORTH, WEST, EAST
    char list[160], text[400];

    switch (sClerk.state)
    {
    case 0:
    {
        // where she comes from: the side with the longest free line (up to 4 tiles), south first
        static const u8 sOrder[] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH};
        s16 px = player->currentCoords.x, py = player->currentCoords.y;
        u8 i, best = 0, bestLen = 0;
        for (i = 0; i < ARRAY_COUNT(sOrder); i++)
        {
            u8 d = sOrder[i], len = 0;
            while (len < 4 && TileFree(px + sDx[d] * (len + 1), py + sDy[d] * (len + 1), player->currentElevation))
                len++;
            if (len > bestLen)
            {
                bestLen = len;
                best = d;
            }
        }
        sClerk.objId = OBJECT_EVENTS_COUNT;
        if (bestLen > 0)
            sClerk.objId = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_BEAUTY, MOVEMENT_TYPE_NONE, LOCALID_CLERK,
                                                                px + sDx[best] * bestLen, py + sDy[best] * bestLen,
                                                                player->currentElevation);
        if (sClerk.objId >= OBJECT_EVENTS_COUNT)
        {
            // nowhere to stand: they come by delivery
            MPNet_Log("tms: the lottery clerk can't stand anywhere, the TMs come by delivery");
            sClerk.state = 10;
            break;
        }
        sClerk.dir = (best == DIR_SOUTH) ? DIR_NORTH : (best == DIR_NORTH) ? DIR_SOUTH : (best == DIR_WEST) ? DIR_EAST : DIR_WEST;
        sClerk.steps = bestLen - 1;
        sClerk.walked = 0;
        SetObjectEventDirection(&gObjectEvents[sClerk.objId], sClerk.dir);
        ObjectEventTurn(player, best);
        MPNet_Log("tms: the lottery clerk comes from %d tiles away (dir %d) with %d TMs", bestLen, best, sClerk.count);
        PlaySE(SE_PIN);
        sClerk.timer = 0;
        sClerk.state++;
        break;
    }
    case 1:
        if (++sClerk.timer < 20)
            break;
        sClerk.state++;
        // fallthrough
    case 2:
        if (ObjectEventIsHeldMovementActive(clerk) && !ObjectEventClearHeldMovementIfFinished(clerk))
            break;
        if (sClerk.walked < sClerk.steps)
        {
            ObjectEventSetHeldMovement(clerk, GetWalkNormalMovementAction(sClerk.dir));
            sClerk.walked++;
            break;
        }
        sClerk.state = 3;
        break;
    case 3:
        ClerkText("Oh, there you are, @! I'm from the\nLOTTERY CORNER at the LILYCOVE\nDEPARTMENT STORE.\f"
                  "You've got lottery winnings nobody\never came to collect!\f"
                  "Well, that's what my manager calls\nthem. Between you and me...\f"
                  "TMs don't wear out anymore! These\nare the ones you used up before,\nback for good.");
        sClerk.state++;
        break;
    case 4:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        // fallthrough
    case 10:
        GiveTms();
        ListTms(list, sizeof(list));
        PlayFanfare(MUS_OBTAIN_TMHM);
        snprintf(text, sizeof(text), "@ received %s!", list);
        ClerkText(text);
        sClerk.state = (sClerk.state == 10) ? 11 : 5;
        break;
    case 5:
    case 11:
        if (IsTextPrinterActive(0) || !IsFanfareTaskInactive() || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        ClerkText(sClerk.state == 5
                  ? "A TM can teach its move as often as\nyou like now. Teach away!\f"
                    "Well, the LOTTERY CORNER awaits.\nGood luck, @!"
                  : "(A note from the LILYCOVE LOTTERY\nCORNER came with them: TMs don't\n"
                    "wear out anymore. Teach away!)");
        sClerk.state = (sClerk.state == 5) ? 6 : 12;
        break;
    case 6:
    case 12:
        if (IsTextPrinterActive(0) || !JOY_NEW(A_BUTTON | B_BUTTON))
            break;
        HideFieldMessageBox();
        if (sClerk.state == 12)
        {
            sClerk.state = 9;
            break;
        }
        sClerk.walked = 0;
        sClerk.state = 7;
        break;
    case 7:
        // back the way she came, then gone
        if (ObjectEventIsHeldMovementActive(clerk) && !ObjectEventClearHeldMovementIfFinished(clerk))
            break;
        if (sClerk.walked < sClerk.steps + 1)
        {
            u8 away = (sClerk.dir == DIR_NORTH) ? DIR_SOUTH : (sClerk.dir == DIR_SOUTH) ? DIR_NORTH
                    : (sClerk.dir == DIR_WEST) ? DIR_EAST : DIR_WEST;
            ObjectEventSetHeldMovement(clerk, GetWalkNormalMovementAction(away));
            sClerk.walked++;
            break;
        }
        RemoveObjectEventByLocalIdAndMap(LOCALID_CLERK, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        sClerk.state = 9;
        break;
    case 9:
        UnlockPlayerFieldControls();
        MPNet_Log("tms: the lottery clerk's visit is over");
        DestroyTask(taskId);
        break;
    }
}

// Every frame (MP_FrameUpdate): once per save, the first time the player is free on the map
void MP_TmRefund_Update(void)
{
    if (gMain.inBattle || FuncIsActiveTask(Task_LotteryClerk) || !MP_IsInOverworld() || !MP_IsLocalFree())
        return;
    if (MP_SaveData_Get(REFUND_KEY))
        return;
    memset(&sClerk, 0, sizeof(sClerk));
    sClerk.count = FindUsedUp(sClerk.tms);
    if (sClerk.count == 0)
    {
        MP_SaveData_Set(REFUND_KEY, 1);   // nothing was used up (or a new game): she never needs to come
        return;
    }
    LockPlayerFieldControls();
    CreateTask(Task_LotteryClerk, 80);
}

// The harness: "mp tmrefund" lists what she'd bring, "mp tmrefund reset" lets her come again
void MP_TmRefund_Debug(const char *args)
{
    u8 tms[MAX_REFUND], n, i;
    if (strncmp(args, "reset", 5) == 0)
        MP_SaveData_Set(REFUND_KEY, 0);
    n = FindUsedUp(tms);
    printf("[test] tmrefund done %d, %d used up:", (int)MP_SaveData_Get(REFUND_KEY), n);
    for (i = 0; i < n; i++)
        printf(" %d", tms[i]);
    printf("\n");
}
