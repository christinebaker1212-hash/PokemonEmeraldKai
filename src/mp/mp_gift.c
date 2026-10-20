// Moemon Co-op: CO-OP menu > DAILY GIFT. Once a day you can send each
// friend online today's gift. They get the gift; you get a thank-you bonus
// that grows with the number of players online. The gift changes every day.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/characters.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);

enum {
    MPR_GIFT_SEND = MPR_GIFT_FIRST,   // u16 item, u16 qty
    MPR_GIFT_THANKS,                  // u8 receivedOk
};

struct Gift
{
    u16 item;
    u16 qty;
};

// One per day, in turn
static const struct Gift sGifts[] = {
    {ITEM_POKE_BALL, 5},
    {ITEM_SUPER_POTION, 2},
    {ITEM_GREAT_BALL, 3},
    {ITEM_ORAN_BERRY, 3},
    {ITEM_FULL_HEAL, 2},
    {ITEM_REVIVE, 1},
    {ITEM_ULTRA_BALL, 2},
    {ITEM_HYPER_POTION, 1},
    {ITEM_SITRUS_BERRY, 2},
    {ITEM_ETHER, 2},
    {ITEM_RARE_CANDY, 1},
    {ITEM_TIMER_BALL, 3},
    {ITEM_PP_UP, 1},
    {ITEM_NUGGET, 1},
};

#define THANKS_MONEY_PER_PLAYER 200

static EWRAM_DATA struct
{
    u8 taskId;
    u16 cursor, scroll;
    u8 friends[MP_MAX_PLAYERS];
    u8 friendCount;
    char note[72];
    bool8 dirty;
} sGiftUi = {0};

static const struct Gift *TodaysGift(void)
{
    return &sGifts[MP_DayNumber(MP_Today()) % ARRAY_COUNT(sGifts)];
}

static void SentKey(u8 id, char *out, size_t size)
{
    char name[16];
    MP_GameToAscii(name, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    snprintf(out, size, "gift:%d:%s", (int)MP_Today(), name);
}

static bool8 SentToday(u8 id)
{
    char key[48];
    SentKey(id, key, sizeof(key));
    return MP_SaveData_Get(key) != 0;
}

static void GiftText(const struct Gift *g, u8 *dst)
{
    u8 q[8];
    CopyItemName(g->item, dst);
    StringAppend(dst, (const u8[]){CHAR_SPACE, EOS});
    MpUi_Qty(q, g->qty);
    StringAppend(dst, q);
}

static void Draw(void)
{
    u16 i, first, count = sGiftUi.friendCount + 1;
    u8 gift[32], title[48];
    sGiftUi.dirty = FALSE;
    MpUi_Begin("DAILY GIFT");
    GiftText(TodaysGift(), gift);
    MP_AsciiToGame(title, "TODAY: ", 10);
    StringAppend(title, gift);
    MpUi_TitleRight(title);
    first = MpUi_ScrollFor(sGiftUi.cursor, count, &sGiftUi.scroll);
    for (i = first; i < count && i < first + MPUI_ROWS; i++)
    {
        u8 name[PLAYER_NAME_LENGTH + 1];
        if (i == sGiftUi.friendCount)
        {
            MpUi_RowAscii(i - first, i == sGiftUi.cursor, "CLOSE", NULL);
            continue;
        }
        StringCopyN(name, gMpPlayers[sGiftUi.friends[i]].name, PLAYER_NAME_LENGTH);
        name[PLAYER_NAME_LENGTH] = EOS;
        if (SentToday(sGiftUi.friends[i]))
        {
            u8 r[24];
            MP_AsciiToGame(r, "sent today", 16);
            MpUi_Row(i - first, i == sGiftUi.cursor, name, r);
        }
        else
            MpUi_Row(i - first, i == sGiftUi.cursor, name, NULL);
    }
    if (sGiftUi.friendCount == 0)
        MpUi_TextAscii(2, "Nobody else is online right now.");
    MpUi_End();
    if (sGiftUi.note[0])
        MpUi_HelpAscii(sGiftUi.note);
    else
        MpUi_HelpAscii("One gift per friend each day.  {A} SEND  {B} CLOSE");
}

static void Close(void)
{
    MpUi_Close();
    DestroyTask(sGiftUi.taskId);
    sGiftUi.taskId = TASK_NONE;
}

static void Task_Gift(u8 taskId)
{
    u16 count = sGiftUi.friendCount + 1;
    if (sGiftUi.note[0] && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY))
    {
        sGiftUi.note[0] = 0;
        Draw();
        return;
    }
    if (MpUi_MoveCursor(&sGiftUi.cursor, count))
        sGiftUi.dirty = TRUE;
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sGiftUi.cursor == sGiftUi.friendCount))
    {
        PlaySE(SE_SELECT);
        Close();
        return;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u8 id = sGiftUi.friends[sGiftUi.cursor];
        if (!gMpPlayers[id].present)
        {
            PlaySE(SE_FAILURE);
            snprintf(sGiftUi.note, sizeof(sGiftUi.note), "They left.");
        }
        else if (SentToday(id))
        {
            PlaySE(SE_FAILURE);
            snprintf(sGiftUi.note, sizeof(sGiftUi.note), "Already sent today. Come back tomorrow!");
        }
        else
        {
            const struct Gift *g = TodaysGift();
            u8 buf[5] = {MPR_GIFT_SEND, g->item & 0xFF, g->item >> 8, g->qty & 0xFF, g->qty >> 8};
            char key[48], name[16];
            MP_SendRelay(id, buf, sizeof(buf));
            SentKey(id, key, sizeof(key));
            MP_SaveData_Set(key, 1);
            MP_Events_OnGiftSent();
            MP_GameToAscii(name, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
            snprintf(sGiftUi.note, sizeof(sGiftUi.note), "You sent %s a gift!", name);
            PlaySE(SE_SUCCESS);
        }
        sGiftUi.dirty = TRUE;
    }
    if (sGiftUi.dirty)
        Draw();
}

void MP_Gift_OpenMenu(void)
{
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        MP_ShowNotice("Go online (CO-OP menu: {COOP}) to send gifts to friends.");
        return;
    }
    memset(&sGiftUi, 0, sizeof(sGiftUi));
    sGiftUi.friendCount = MP_Friends_List(sGiftUi.friends);
    MpUi_Open();
    sGiftUi.taskId = CreateTask(Task_Gift, 80);
    Draw();
}

void MP_Gift_OnRelay(u8 from, const u8 *data, u16 len)
{
    char name[16], msg[96];
    u8 text[96];
    if (from >= MP_MAX_PLAYERS || len < 1)
        return;
    MP_GameToAscii(name, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
    switch (data[0])
    {
    case MPR_GIFT_SEND:
        if (len >= 5)
        {
            u16 item = data[1] | (data[2] << 8), qty = data[3] | (data[4] << 8);
            u8 ok = 0, reply[2] = {MPR_GIFT_THANKS, 0};
            char itemName[24];
            u8 itemGame[24];
            if (item == ITEM_NONE || item >= ITEMS_COUNT || qty == 0 || qty > 10)
                return;
            if (AddBagItem(item, qty))
                ok = 1;
            else
                AddMoney(&gSaveBlock1Ptr->money, 500);   // bag full: some money instead
            CopyItemName(item, itemGame);
            MP_GameToAscii(itemName, itemGame, 20);
            if (ok)
                snprintf(msg, sizeof(msg), "%s sent you a gift:\n%s x%d!", name, itemName, qty);
            else
                snprintf(msg, sizeof(msg), "%s sent you a gift!\nYour BAG was full: $500 instead.", name);
            MP_AsciiToGame(text, msg, 90);
            MP_Toast(text);
            PlaySE(SE_SUCCESS);
            reply[1] = ok;
            MP_SendRelay(from, reply, 2);
            MPNet_Log("gift: got item %d x%d from player %d", item, qty, from + 1);
        }
        break;
    case MPR_GIFT_THANKS:
    {
        // The thank-you bonus: more players online, bigger bonus
        u32 bonus = THANKS_MONEY_PER_PLAYER * MP_ConnectedCount();
        AddMoney(&gSaveBlock1Ptr->money, bonus);
        MPNet_Log("gift: player %d says thanks (+$%u)", from + 1, (unsigned)bonus);
        snprintf(msg, sizeof(msg), "%s got your gift!\nThank-you bonus: $%u", name, (unsigned)bonus);
        MP_AsciiToGame(text, msg, 90);
        MP_Toast(text);
        break;
    }
    }
}
