// Moemon Co-op: CO-OP menu > SHARED PC, and the client side of the room
// storage (mp_store.c on the server).
//
// The shared PC belongs to the server: Pokémon and items put in by anyone
// in the room can be taken out by anyone, and it's still there the next time
// the same server name and password are hosted. Pokémon keep their original
// trainer. SHARED PC asks MOEMON / ITEMS / CANCEL: MOEMON opens the PC box
// screen on the shared boxes (mp_sharedbox.c, pokemon_storage_system.c),
// ITEMS the list here (SHARED / MY PC, friends' cursors on the rows).
//
// Also here: the counter cache (team goals, achievements, the weekly race),
// see MP_Counter_*.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "platform/gamepad.h"
#include "pokedex.h"
#include "mp_net.h"
#include "item.h"
#include "main.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/characters.h"
#include "field_screen_effect.h"
#include "script.h"
#include "field_weather.h"
#include "strings.h"
#include "palette.h"
#include "event_object_lock.h"
#include "field_player_avatar.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);

STATIC_ASSERT(sizeof(struct Pokemon) + PLAYER_NAME_LENGTH + 1 <= MP_STORE_MON_BYTES, StoredMonFits);

// ---------------------------------------------------------------------------
// Talking to the server

static bool8 CanUseStore(void)
{
    return MP_IsConnected() && gMpLocalId >= 0;
}

static bool8 Online(void)
{
    return CanUseStore();
}

static void StoreSend(const u8 *data, u16 len)
{
    if (CanUseStore())
        MPNet_ClientSend(MPMSG_C_STORE, data, len);
}

// ---------------------------------------------------------------------------
// Counters

#define CACHE_SIZE 256

struct CachedCounter
{
    char key[MP_STORE_KEY_MAX + 1];
    s32 value;
};

static struct CachedCounter sCache[CACHE_SIZE];
static u16 sCacheCount;
static u32 sCounterVersion;   // bumps whenever a value arrives

static void CachePut(const char *key, s32 value)
{
    u16 i;
    for (i = 0; i < sCacheCount; i++)
        if (strcmp(sCache[i].key, key) == 0)
            break;
    if (i == sCacheCount)
    {
        if (sCacheCount == CACHE_SIZE)
        {
            memmove(&sCache[0], &sCache[1], sizeof(sCache[0]) * (CACHE_SIZE - 1));
            i = CACHE_SIZE - 1;
        }
        else
            sCacheCount++;
        snprintf(sCache[i].key, sizeof(sCache[i].key), "%s", key);
    }
    sCache[i].value = value;
    sCounterVersion++;
}

// Last known value (0 if never heard of)
s32 MP_Counter_Get(const char *key)
{
    u16 i;
    for (i = 0; i < sCacheCount; i++)
        if (strcmp(sCache[i].key, key) == 0)
            return sCache[i].value;
    return 0;
}

u32 MP_Counter_Version(void)
{
    return sCounterVersion;
}

static void SendCounterOp(u8 op, const char *key, s32 v)
{
    u8 buf[8 + MP_STORE_KEY_MAX];
    int n = (int)strlen(key);
    if (n == 0 || n > MP_STORE_KEY_MAX)
        return;
    buf[0] = op;
    buf[1] = v; buf[2] = v >> 8; buf[3] = v >> 16; buf[4] = v >> 24;
    buf[5] = (u8)n;
    memcpy(buf + 6, key, n);
    StoreSend(buf, 6 + n);
}

void MP_Counter_Add(const char *key, s32 delta)
{
    SendCounterOp(MP_ST_CTR_ADD, key, delta);
}

void MP_Counter_Max(const char *key, s32 value)
{
    SendCounterOp(MP_ST_CTR_MAX, key, value);
}

// Asks for every counter starting with prefix (answers arrive in a moment)
void MP_Counter_Request(const char *prefix)
{
    u8 buf[2 + MP_STORE_KEY_MAX];
    int n = (int)strlen(prefix);
    if (n > MP_STORE_KEY_MAX)
        n = MP_STORE_KEY_MAX;
    buf[0] = MP_ST_CTR_GET;
    buf[1] = (u8)n;
    memcpy(buf + 2, prefix, n);
    StoreSend(buf, 2 + n);
}

// Calls fn for every cached counter starting with prefix
void MP_Counter_ForEach(const char *prefix, void (*fn)(const char *key, s32 value, void *arg), void *arg)
{
    u16 i;
    size_t pl = strlen(prefix);
    for (i = 0; i < sCacheCount; i++)
        if (strncmp(sCache[i].key, prefix, pl) == 0)
            fn(sCache[i].key, sCache[i].value, arg);
}

// "from NAME" (24 bytes). StringAppendN copies exactly 7 bytes and never ends the string, so a full 7-letter name
// (SATOSHI) has no end marker of its own: end it here.
void MP_SharedPc_FromLabel(u8 *from, const u8 *by)
{
    MP_AsciiToGame(from, "from ", 8);
    *StringAppendN(from, by, PLAYER_NAME_LENGTH) = EOS;
}

// ---------------------------------------------------------------------------
// SHARED PC: MOEMON / ITEMS / CANCEL

enum { CHOOSE_MONS, CHOOSE_ITEMS, CHOOSE_CANCEL, CHOOSE_COUNT };
enum { MODE_CHOOSE, MODE_CONNECTING, MODE_FADE, MODE_LIST, MODE_ACTION, MODE_PICK_BAG, MODE_QTY, MODE_WAIT };
enum { VIEW_SHARED, VIEW_MYPC };
enum { ACT_WITHDRAW, ACT_SEND, ACT_CANCEL, ACT_COUNT };

#define MAX_BAG_ENTRIES 120
#define MAX_QUEUE 64

struct ItemEntry
{
    u16 item;
    u16 qty;
};

// An item move waiting for the server (done one at a time)
struct ItemJob
{
    u8 op;          // MP_ST_ITEM_TAKE / MP_ST_ITEM_PUT
    u8 dest;        // TAKE: to the BAG (0) or our PC (1); PUT: from the BAG (0) or our PC (1)
    u16 item, qty;
};

static EWRAM_DATA struct
{
    u8 taskId;
    u8 mode;
    u8 view;
    u16 cursor, scroll, chooseCursor, actionCursor;
    struct ItemEntry items[MP_STORE_ITEMS];
    u8 itemCount;
    struct ItemEntry bag[MAX_BAG_ENTRIES];
    u16 bagCount;
    u64 tags;                  // tagged rows (by item list position, the current view)
    u16 qtyItem, qty, qtyMax;
    u8 qtyAction;              // ACT_WITHDRAW / ACT_SEND, or 0xFF: deposit from the BAG
    struct ItemJob queue[MAX_QUEUE];
    u8 queueHead, queueCount;
    bool8 jobOut;
    u16 waitTimer;
    bool8 dirty;
    u16 cursorsSeen;           // redraw when friends' cursors move
    char note[64];
} sPc = {0};

static void Task_SharedPc(u8 taskId);
static void Draw(void);

static void SetNote(const char *ascii)
{
    snprintf(sPc.note, sizeof(sPc.note), "%s", ascii);
}

static bool8 IsHM(u16 item)
{
    return item >= ITEM_HM01 && item <= ITEM_HM08;
}

static void RequestItems(void)
{
    u8 op = MP_ST_ITEM_LIST;
    StoreSend(&op, 1);
}

// Our own PC's items, as a list like the shared one's
static u8 MyPcItems(struct ItemEntry *out)
{
    u8 i, n = 0;
    for (i = 0; i < PC_ITEMS_COUNT; i++)
        if (gSaveBlock1Ptr->pcItems[i].itemId != ITEM_NONE && gSaveBlock1Ptr->pcItems[i].quantity)
        {
            out[n].item = gSaveBlock1Ptr->pcItems[i].itemId;
            out[n].qty = gSaveBlock1Ptr->pcItems[i].quantity;
            n++;
        }
    return n;
}

static u8 ListCount(void)
{
    static struct ItemEntry sMine[PC_ITEMS_COUNT];
    return sPc.view == VIEW_SHARED ? sPc.itemCount : MyPcItems(sMine);
}

static struct ItemEntry ListItem(u16 i)
{
    static struct ItemEntry sMine[PC_ITEMS_COUNT];
    struct ItemEntry none = {0};
    if (sPc.view == VIEW_SHARED)
        return i < sPc.itemCount ? sPc.items[i] : none;
    if (i < MyPcItems(sMine))
        return sMine[i];
    return none;
}

static u8 PcItemIndex(u16 item)
{
    u8 i;
    for (i = 0; i < PC_ITEMS_COUNT; i++)
        if (gSaveBlock1Ptr->pcItems[i].itemId == item)
            return i;
    return 0xFF;
}

static void RemoveFromMyPc(u16 item, u16 qty)
{
    while (qty)
    {
        u8 i = PcItemIndex(item);
        u16 have, take;
        if (i == 0xFF)
            break;
        have = gSaveBlock1Ptr->pcItems[i].quantity;
        take = qty < have ? qty : have;
        RemovePCItem(i, take);
        qty -= take;
    }
}

static void BuildBagList(void)
{
    static const u8 pockets[] = {ITEMS_POCKET, BALLS_POCKET, BERRIES_POCKET, TMHM_POCKET};
    u8 p;
    sPc.bagCount = 0;
    for (p = 0; p < ARRAY_COUNT(pockets); p++)
    {
        u16 i;
        for (i = 0; i < gBagPockets[pockets[p]].capacity && sPc.bagCount < MAX_BAG_ENTRIES; i++)
        {
            u16 item = BagGetItemIdByPocketPosition(pockets[p] + 1, i);
            u16 qty = BagGetQuantityByPocketPosition(pockets[p] + 1, i);
            if (item == ITEM_NONE || qty == 0 || IsHM(item))
                continue;
            sPc.bag[sPc.bagCount].item = item;
            sPc.bag[sPc.bagCount].qty = qty;
            sPc.bagCount++;
        }
    }
}

// Items that can't go to the shared PC from our own: key items and HMs stay with us
static bool8 CanShare(u16 item)
{
    return !IsHM(item) && GetPocketByItemId(item) != POCKET_KEY_ITEMS;
}

static void Queue(u8 op, u8 dest, u16 item, u16 qty)
{
    struct ItemJob *j;
    if (sPc.queueCount >= MAX_QUEUE || qty == 0)
        return;
    j = &sPc.queue[(sPc.queueHead + sPc.queueCount) % MAX_QUEUE];
    j->op = op;
    j->dest = dest;
    j->item = item;
    j->qty = qty;
    sPc.queueCount++;
}

static void RunQueue(void)
{
    struct ItemJob *j;
    u8 buf[5];
    if (sPc.jobOut || sPc.queueCount == 0)
        return;
    j = &sPc.queue[sPc.queueHead];
    buf[0] = j->op;
    buf[1] = j->item & 0xFF;
    buf[2] = j->item >> 8;
    buf[3] = j->qty & 0xFF;
    buf[4] = j->qty >> 8;
    StoreSend(buf, 5);
    sPc.jobOut = TRUE;
    sPc.waitTimer = 0;
}

static void Close(void)
{
    MpUi_Close();
    DestroyTask(sPc.taskId);
    sPc.taskId = TASK_NONE;
    MP_SharedBox_SendCursor(MP_PCV_NONE, 0, 0, 0, 0, 0);
}

static void DrawChooser(void)
{
    static const char *const sRows[CHOOSE_COUNT] = {"MO~MON", "ITEMS", "CANCEL"};
    u8 i;
    MpUi_Begin("SHARED PC");
    if (sPc.mode == MODE_CONNECTING)
    {
        MpUi_TextAscii(1, "Connecting to the shared PC...");
        MpUi_End();
        MpUi_HelpAscii("{B} BACK");
        return;
    }
    for (i = 0; i < CHOOSE_COUNT; i++)
        MpUi_RowAscii(i, i == sPc.chooseCursor, sRows[i], NULL);
    MpUi_End();
    if (sPc.note[0])
        MpUi_HelpAscii(sPc.note);
    else
        MpUi_HelpAscii(sPc.chooseCursor == CHOOSE_MONS ? "The shared boxes, like a PC's. Your own boxes are a button away."
                     : sPc.chooseCursor == CHOOSE_ITEMS ? "Shared items, and your own PC's."
                     : "{A} OK  {B} CLOSE");
}

// Friends on this item's row (shared list): "LUNA" (or "" if none)
static void FriendsOnItem(u16 item, u8 *out)
{
    u8 p, n = 0;
    out[0] = EOS;
    if (sPc.view != VIEW_SHARED)
        return;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        struct MpPcCursor c;
        u8 k;
        if (!MP_SharedBox_GetCursor(p, &c) || c.view != MP_PCV_ITEMS || c.row != item)
            continue;
        if (n)
            out[n++] = CHAR_SPACE;
        out[n++] = CHAR_BLACK_TRIANGLE;   // ▶
        for (k = 0; k < PLAYER_NAME_LENGTH && gMpPlayers[p].name[k] != EOS; k++)
            out[n++] = gMpPlayers[p].name[k];
        out[n] = EOS;
    }
}

static void DrawList(void)
{
    u16 count = ListCount() + 1, first, i;
    u8 left[64], right[48], tabs[40], who[40];

    MpUi_Begin("SHARED PC: ITEMS");
    MP_AsciiToGame(tabs, sPc.view == VIEW_SHARED ? "SHARED  ({L}{R} MY PC)" : "MY PC  ({L}{R} SHARED)", 36);
    MpUi_TitleRight(tabs);
    first = MpUi_ScrollFor(sPc.cursor, count, &sPc.scroll);
    for (i = first; i < count && i < first + MPUI_ROWS; i++)
    {
        bool8 sel = (i == sPc.cursor) && sPc.mode == MODE_LIST;
        u8 row = i - first;
        if (i == 0)
        {
            FriendsOnItem(0, who);
            MP_AsciiToGame(left, "DEPOSIT FROM BAG...", 30);
            MpUi_Row(row, sel, left, who[0] ? who : NULL);
            continue;
        }
        {
            struct ItemEntry e = ListItem(i - 1);
            CopyItemName(e.item, left);
            if (sPc.tags & (1ull << (i - 1)))
            {
                // tagged: a "+" before the name (0xF7 and such are text control codes: never as a mark)
                u8 tagged[64];
                tagged[0] = CHAR_PLUS;
                tagged[1] = CHAR_SPACE;
                StringCopy(tagged + 2, left);
                StringCopy(left, tagged);
            }
            MpUi_Qty(right, e.qty);
            FriendsOnItem(e.item, who);
            if (who[0])
            {
                u8 both[64];
                StringCopy(both, who);
                StringAppend(both, gText_Space);
                StringAppend(both, right);
                StringCopy(right, both);
            }
            MpUi_Row(row, sel, left, right);
        }
    }
    if (count == 1)
        MpUi_TextAscii(2, sPc.view == VIEW_SHARED ? "No items stored yet." : "No items in your PC.");
    MpUi_End();
}

static void DrawAction(void)
{
    u8 title[48], name[24], i;
    struct ItemEntry e = ListItem(sPc.cursor - 1);
    u8 tagged = 0;
    for (i = 0; i < 64; i++)
        if (sPc.tags & (1ull << i))
            tagged++;
    if (tagged)
        snprintf((char *)name, sizeof(name), "%d tagged", tagged);
    MpUi_Begin("SHARED PC: ITEMS");
    if (tagged)
        MP_AsciiToGame(title, (const char *)name, 20);
    else
    {
        CopyItemName(e.item, title);
    }
    MpUi_Row(0, FALSE, title, NULL);
    MpUi_RowAscii(1, sPc.actionCursor == ACT_WITHDRAW, tagged ? "WITHDRAW ALL" : "WITHDRAW", NULL);
    MpUi_RowAscii(2, sPc.actionCursor == ACT_SEND,
                  sPc.view == VIEW_SHARED ? (tagged ? "SEND ALL TO PC" : "SEND TO PC") : (tagged ? "SEND ALL TO SHARED" : "SEND TO SHARED"), NULL);
    MpUi_RowAscii(3, sPc.actionCursor == ACT_CANCEL, "CANCEL", NULL);
    MpUi_End();
}

static void DrawPickBag(void)
{
    u16 count = sPc.bagCount + 1, first, i;
    u8 left[64], right[24];
    MpUi_Begin(sPc.view == VIEW_SHARED ? "SHARED PC: PUT IN WHICH ITEM?" : "MY PC: PUT IN WHICH ITEM?");
    first = MpUi_ScrollFor(sPc.cursor, count, &sPc.scroll);
    for (i = first; i < count && i < first + MPUI_ROWS; i++)
    {
        bool8 sel = (i == sPc.cursor) && sPc.mode == MODE_PICK_BAG;
        u8 row = i - first;
        if (i == sPc.bagCount)
            MpUi_RowAscii(row, sel, "CANCEL", NULL);
        else
        {
            CopyItemName(sPc.bag[i].item, left);
            MpUi_Qty(right, sPc.bag[i].qty);
            MpUi_Row(row, sel, left, right);
        }
    }
    MpUi_End();
}

static void Draw(void)
{
    switch (sPc.mode)
    {
    case MODE_CHOOSE:
    case MODE_CONNECTING:
        DrawChooser();
        return;
    case MODE_FADE:
        return;
    case MODE_ACTION:
        DrawAction();
        break;
    case MODE_PICK_BAG:
    case MODE_QTY:
        if (sPc.qtyAction == 0xFF)
            DrawPickBag();
        else
            DrawList();
        break;
    default:
        DrawList();
        break;
    }
    if (sPc.note[0])
        MpUi_HelpAscii(sPc.note);
    else if (sPc.mode == MODE_QTY)
    {
        u8 buf[96], name[24], num[8];
        const char *verb = sPc.qtyAction == 0xFF ? "PUT IN " : sPc.qtyAction == ACT_WITHDRAW ? "WITHDRAW "
                         : sPc.view == VIEW_SHARED ? "TO MY PC: " : "TO SHARED: ";
        MP_AsciiToGame(buf, verb, 14);
        CopyItemName(sPc.qtyItem, name);
        StringAppend(buf, name);
        MP_AsciiToGame(buf + StringLength(buf), "  < ", 6);
        ConvertIntToDecimalStringN(num, sPc.qty, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(buf, num);
        MP_AsciiToGame(buf + StringLength(buf), " >  {A} OK  {B} BACK", 20);
        MpUi_Help(buf);
    }
    else if (sPc.mode == MODE_WAIT || sPc.jobOut || sPc.queueCount)
        MpUi_HelpAscii("One moment...");
    else if (sPc.mode == MODE_LIST && sPc.cursor >= 1 && sPc.cursor <= ListCount())
        MpUi_HelpItem(ListItem(sPc.cursor - 1).item);
    else if (sPc.mode == MODE_PICK_BAG && sPc.cursor < sPc.bagCount)
        MpUi_HelpItem(sPc.bag[sPc.cursor].item);
    else if (sPc.mode == MODE_LIST)
        // TAG is the pad's west button, the F key on a keyboard (where the west icon stands for SELECT)
        MpUi_HelpAscii(Gamepad_IconFamily() == GAMEPAD_ICONS_KEYBOARD ? "{A} CHOOSE  F KEY: TAG  {L}{R} SHARED / MY PC  {B} BACK"
                                                                      : "{A} CHOOSE  {WEST} TAG  {L}{R} SHARED / MY PC  {B} BACK");
    else
        MpUi_HelpAscii("{A} OK  {B} BACK");
}

// ---- MOEMON: the box screen

static void Task_SharedBoxReturn(u8 taskId)
{
    if (!gPaletteFade.active && IsWeatherNotFadingIn())
    {
        DestroyTask(taskId);
        ScriptUnfreezeObjectEvents();
        UnlockPlayerFieldControls();
        if (!Online())
            MP_ShowNotice("The connection was lost, so the shared PC closed.");
        else
            MP_SharedPc_Open();   // back at the SHARED PC menu, like the stock PC's
    }
}

static void FieldCB_SharedBoxReturn(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_SharedBoxReturn, 10);
}

static void Task_OpenSharedBoxes(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    if (!Online() || !MP_SharedBox_Loaded())
    {
        MP_SharedBox_End();
        FieldCB_SharedBoxReturn();
        return;
    }
    MP_PcStorage_OpenShared(FieldCB_SharedBoxReturn);
}

// ---- Input

static void HandleChooseInput(void)
{
    if (sPc.mode == MODE_CONNECTING)
    {
        if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            MP_SharedBox_End();
            sPc.mode = MODE_CHOOSE;
            sPc.dirty = TRUE;
        }
        else if (MP_SharedBox_Loaded())
        {
            // to the box screen
            PlaySE(SE_PC_LOGIN);
            sPc.mode = MODE_FADE;
            MpUi_Close();
            DestroyTask(sPc.taskId);
            sPc.taskId = TASK_NONE;
            LockPlayerFieldControls();
            FadeScreen(FADE_TO_BLACK, 0);
            CreateTask(Task_OpenSharedBoxes, 10);
        }
        return;
    }
    if (JOY_REPEAT(DPAD_UP))
    {
        sPc.chooseCursor = (sPc.chooseCursor + CHOOSE_COUNT - 1) % CHOOSE_COUNT;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        sPc.chooseCursor = (sPc.chooseCursor + 1) % CHOOSE_COUNT;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sPc.chooseCursor == CHOOSE_CANCEL))
    {
        PlaySE(SE_SELECT);
        Close();
    }
    else if (JOY_NEW(A_BUTTON) && sPc.chooseCursor == CHOOSE_MONS)
    {
        PlaySE(SE_SELECT);
        if (MP_SharedBox_IsOpen())
        {
            SetNote("One moment...");   // the last visit is still being sent
        }
        else
        {
            MP_SharedBox_Begin();
            sPc.mode = MODE_CONNECTING;
        }
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON) && sPc.chooseCursor == CHOOSE_ITEMS)
    {
        PlaySE(SE_SELECT);
        sPc.mode = MODE_WAIT;
        sPc.view = VIEW_SHARED;
        sPc.cursor = sPc.scroll = 0;
        sPc.tags = 0;
        MP_SharedBox_ResetCursors();
        RequestItems();
        sPc.dirty = TRUE;
    }
}

static u8 TaggedCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < 64; i++)
        if (sPc.tags & (1ull << i))
            n++;
    return n;
}

// Every tagged row's whole stack (or the one under the cursor) does the action
static void DoAction(u8 act, u16 item, u16 qty)
{
    if (sPc.view == VIEW_SHARED)
    {
        if (act == ACT_WITHDRAW)
            Queue(MP_ST_ITEM_TAKE, 0, item, qty);
        else
            Queue(MP_ST_ITEM_TAKE, 1, item, qty);
    }
    else if (act == ACT_WITHDRAW)
    {
        // our PC -> the BAG: no server needed
        u8 i = PcItemIndex(item);
        u16 have = (i == 0xFF) ? 0 : gSaveBlock1Ptr->pcItems[i].quantity;
        if (qty > have)
            qty = have;
        if (qty && CheckBagHasSpace(item, qty))
        {
            RemoveFromMyPc(item, qty);
            AddBagItem(item, qty);
            SetNote("Put in your BAG!");
        }
        else if (qty)
            SetNote("There's no room in your BAG for that.");
    }
    else if (!CanShare(item))
        SetNote("That stays with you (a key item or an HM).");
    else
    {
        u8 i = PcItemIndex(item);
        u16 have = (i == 0xFF) ? 0 : gSaveBlock1Ptr->pcItems[i].quantity;
        if (qty > have)
            qty = have;
        RemoveFromMyPc(item, qty);
        Queue(MP_ST_ITEM_PUT, 1, item, qty);   // out of our PC now; back in if it doesn't fit
    }
}

static void HandleListInput(void)
{
    u16 count = ListCount() + 1;
    if (MpUi_MoveCursor(&sPc.cursor, count))
    {
        sPc.dirty = TRUE;
        return;
    }
    if (gMpPcTagPressed)
    {
        gMpPcTagPressed = FALSE;
        if (sPc.cursor >= 1 && sPc.cursor <= ListCount() && sPc.cursor - 1 < 64)
        {
            sPc.tags ^= 1ull << (sPc.cursor - 1);
            PlaySE(SE_SELECT);
        }
        else
            PlaySE(SE_FAILURE);
        sPc.dirty = TRUE;
        return;
    }
    if (JOY_NEW(L_BUTTON | R_BUTTON | DPAD_LEFT | DPAD_RIGHT))
    {
        sPc.view ^= 1;
        sPc.cursor = sPc.scroll = 0;
        sPc.tags = 0;
        PlaySE(SE_SELECT);
        if (sPc.view == VIEW_SHARED)
            RequestItems();
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sPc.tags)
            sPc.tags = 0;   // B drops the tags first
        else
        {
            sPc.mode = MODE_CHOOSE;
            MP_SharedBox_SendCursor(MP_PCV_NONE, 0, 0, 0, 0, 0);
        }
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sPc.cursor == 0)
        {
            BuildBagList();
            sPc.qtyAction = 0xFF;
            sPc.mode = MODE_PICK_BAG;
            sPc.cursor = sPc.scroll = 0;
        }
        else
        {
            sPc.mode = MODE_ACTION;
            sPc.actionCursor = ACT_WITHDRAW;
        }
        sPc.dirty = TRUE;
    }
}

static void HandleActionInput(void)
{
    if (JOY_REPEAT(DPAD_UP))
    {
        sPc.actionCursor = (sPc.actionCursor + ACT_COUNT - 1) % ACT_COUNT;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        sPc.actionCursor = (sPc.actionCursor + 1) % ACT_COUNT;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sPc.actionCursor == ACT_CANCEL))
    {
        PlaySE(SE_SELECT);
        sPc.mode = MODE_LIST;
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (TaggedCount())
        {
            // every tagged stack, whole
            u8 i, n = ListCount();
            for (i = 0; i < n && i < 64; i++)
                if (sPc.tags & (1ull << i))
                {
                    struct ItemEntry e = ListItem(i);
                    DoAction(sPc.actionCursor, e.item, e.qty);
                }
            sPc.tags = 0;
            sPc.mode = MODE_LIST;
        }
        else
        {
            struct ItemEntry e = ListItem(sPc.cursor - 1);
            sPc.qtyItem = e.item;
            sPc.qtyMax = e.qty;
            sPc.qty = 1;
            sPc.qtyAction = sPc.actionCursor;
            sPc.mode = MODE_QTY;
        }
        sPc.dirty = TRUE;
    }
}

static void HandlePickInput(void)
{
    u16 count = sPc.bagCount + 1;
    if (MpUi_MoveCursor(&sPc.cursor, count))
    {
        sPc.dirty = TRUE;
        return;
    }
    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sPc.cursor == count - 1))
    {
        PlaySE(SE_SELECT);
        sPc.mode = MODE_LIST;
        sPc.cursor = sPc.scroll = 0;
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sPc.qtyItem = sPc.bag[sPc.cursor].item;
        sPc.qtyMax = sPc.bag[sPc.cursor].qty;
        sPc.qty = 1;
        sPc.qtyAction = 0xFF;
        sPc.mode = MODE_QTY;
        sPc.dirty = TRUE;
    }
}

static void HandleQtyInput(void)
{
    if (JOY_REPEAT(DPAD_RIGHT | DPAD_UP | R_BUTTON))
    {
        u16 step = JOY_REPEAT(R_BUTTON) ? 10 : 1;
        sPc.qty = (sPc.qty + step > sPc.qtyMax) ? (sPc.qty == sPc.qtyMax ? 1 : sPc.qtyMax) : sPc.qty + step;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_REPEAT(DPAD_LEFT | DPAD_DOWN | L_BUTTON))
    {
        u16 step = JOY_REPEAT(L_BUTTON) ? 10 : 1;
        sPc.qty = (sPc.qty <= step) ? (sPc.qty == 1 ? sPc.qtyMax : 1) : sPc.qty - step;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sPc.mode = sPc.qtyAction == 0xFF ? MODE_PICK_BAG : MODE_ACTION;
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sPc.qtyAction == 0xFF)
        {
            // from the BAG into the shared PC or ours
            if (RemoveBagItem(sPc.qtyItem, sPc.qty))
            {
                if (sPc.view == VIEW_SHARED)
                    Queue(MP_ST_ITEM_PUT, 0, sPc.qtyItem, sPc.qty);
                else if (AddPCItem(sPc.qtyItem, sPc.qty))
                    SetNote("Stored in your PC!");
                else
                {
                    AddBagItem(sPc.qtyItem, sPc.qty);
                    SetNote("Your PC is full.");
                }
            }
            sPc.cursor = sPc.scroll = 0;
        }
        else
            DoAction(sPc.qtyAction, sPc.qtyItem, sPc.qty);
        sPc.mode = MODE_LIST;
        sPc.dirty = TRUE;
    }
}

static void SendItemsCursor(void)
{
    u16 item = 0;
    if (sPc.mode == MODE_CHOOSE || sPc.mode == MODE_CONNECTING || sPc.view != VIEW_SHARED)
    {
        MP_SharedBox_SendCursor(MP_PCV_NONE, 0, 0, 0, 0, 0);
        return;
    }
    if (sPc.cursor >= 1)
        item = ListItem(sPc.cursor - 1).item;
    MP_SharedBox_SendCursorRow(MP_PCV_ITEMS, 0, 0, 0, 0, 0, item);
}

static u16 CursorsVersion(void)
{
    u16 v = 0;
    u8 p;
    for (p = 0; p < MP_MAX_PLAYERS; p++)
    {
        struct MpPcCursor c;
        if (MP_SharedBox_GetCursor(p, &c) && c.view == MP_PCV_ITEMS)
            v = v * 31 + c.row + 1;
    }
    return v;
}

static void Task_SharedPc(u8 taskId)
{
    if (!Online())
    {
        Close();
        MP_ShowNotice("The connection was lost, so the shared PC closed.");
        return;
    }
    RunQueue();
    if (sPc.note[0] && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY | L_BUTTON | R_BUTTON))
    {
        sPc.note[0] = 0;
        sPc.dirty = TRUE;
        return;
    }
    switch (sPc.mode)
    {
    case MODE_CHOOSE:
    case MODE_CONNECTING:
        HandleChooseInput();
        if (sPc.taskId == TASK_NONE)
            return;
        break;
    case MODE_LIST:
        if (!sPc.jobOut && !sPc.queueCount)
            HandleListInput();
        break;
    case MODE_ACTION:
        HandleActionInput();
        break;
    case MODE_PICK_BAG:
        HandlePickInput();
        break;
    case MODE_QTY:
        HandleQtyInput();
        break;
    case MODE_WAIT:
        if (JOY_NEW(B_BUTTON))
        {
            sPc.mode = MODE_CHOOSE;
            sPc.dirty = TRUE;
        }
        break;
    }
    gMpPcTagPressed = FALSE;
    if (sPc.taskId == TASK_NONE)
        return;
    SendItemsCursor();
    if (CursorsVersion() != sPc.cursorsSeen)
    {
        sPc.cursorsSeen = CursorsVersion();
        sPc.dirty = TRUE;
    }
    if (sPc.dirty)
    {
        sPc.dirty = FALSE;
        Draw();
    }
}

// Test harness: how many of an item the shared PC had when last listed
u16 MP_SharedPc_ListedQty(u16 item)
{
    u8 i;
    for (i = 0; i < sPc.itemCount; i++)
        if (sPc.items[i].item == item)
            return sPc.items[i].qty;
    return 0;
}

bool8 MP_SharedPc_ItemsOpen(void)
{
    return sPc.taskId != TASK_NONE && FuncIsActiveTask(Task_SharedPc)
        && sPc.mode != MODE_CHOOSE && sPc.mode != MODE_CONNECTING;
}

void MP_SharedPc_Open(void)
{
    if (!CanUseStore())
    {
        MP_ShowNotice("The shared PC belongs to the server. Go online (CO-OP menu: {COOP}) to use it.");
        return;
    }
    if (MP_InDungeon())
    {
        MP_ShowNotice("The shared PC can't reach you down here. Try again once you're out of the cave or dungeon.");
        return;
    }
    memset(&sPc, 0, sizeof(sPc));
    sPc.mode = MODE_CHOOSE;
    MpUi_Open();
    sPc.taskId = CreateTask(Task_SharedPc, 80);
    Draw();
}

// ---------------------------------------------------------------------------
// Replies from the server

static void OnItemList(const u8 *data, u16 len)
{
    u8 count, i;
    if (len < 2)
        return;
    count = data[1];
    sPc.itemCount = 0;
    for (i = 0; i < count && 2 + i * 4 + 4 <= len && sPc.itemCount < MP_STORE_ITEMS; i++)
    {
        const u8 *p = data + 2 + i * 4;
        sPc.items[sPc.itemCount].item = p[0] | (p[1] << 8);
        sPc.items[sPc.itemCount].qty = p[2] | (p[3] << 8);
        sPc.itemCount++;
    }
}

static bool8 PcOpen(void)
{
    return sPc.taskId != TASK_NONE && FuncIsActiveTask(Task_SharedPc);
}

// The answer to the job at the head of the queue
static void OnItemDone(u8 op, u16 item, u16 done)
{
    struct ItemJob *j = &sPc.queue[sPc.queueHead];
    if (!sPc.jobOut || j->op != op || j->item != item)
        return;
    if (op == MP_ST_ITEM_TAKE)
    {
        if (done == 0)
            SetNote("Someone else took it first.");
        else if (j->dest == 0)
        {
            if (CheckBagHasSpace(item, done))
            {
                AddBagItem(item, done);
                SetNote("Put in your BAG!");
            }
            else
            {
                Queue(MP_ST_ITEM_PUT, 2, item, done);   // no room after all: straight back
                SetNote("There's no room in your BAG for that.");
            }
        }
        else if (AddPCItem(item, done))
            SetNote("Sent to your PC!");
        else
        {
            Queue(MP_ST_ITEM_PUT, 2, item, done);
            SetNote("Your PC is full.");
        }
        PlaySE(done ? SE_PC_LOGIN : SE_FAILURE);
    }
    else
    {
        // whatever didn't fit goes back where it came from
        if (done < j->qty)
        {
            if (j->dest == 0)
                AddBagItem(item, j->qty - done);
            else if (j->dest == 1)
                AddPCItem(item, j->qty - done);
        }
        if (j->dest != 2)
        {
            SetNote(done ? (j->dest == 0 ? "Stored in the shared PC!" : "Sent to the shared PC!") : "The shared PC is full.");
            PlaySE(done ? SE_PC_LOGIN : SE_FAILURE);
        }
    }
    sPc.queueHead = (sPc.queueHead + 1) % MAX_QUEUE;
    sPc.queueCount--;
    sPc.jobOut = FALSE;
}

void MP_Store_OnMessage(const u8 *data, u16 len)
{
    if (len < 1)
        return;
    switch (data[0])
    {
    case MP_ST_CTR_VALUE:
        if (len >= 6 && data[5] > 0 && data[5] <= MP_STORE_KEY_MAX && 6 + data[5] <= len)
        {
            char key[MP_STORE_KEY_MAX + 1];
            memcpy(key, data + 6, data[5]);
            key[data[5]] = 0;
            CachePut(key, (s32)(data[1] | (data[2] << 8) | (data[3] << 16) | ((u32)data[4] << 24)));
        }
        break;
    case MP_ST_CTR_END:
        sCounterVersion++;
        break;
    case MP_ST_RH_TOP:
        MP_Rhythm_OnStore(data, len);   // Moemon Rhythm: a chart's records (mp_rhythm.c)
        break;
    case MP_ST_BOX_DATA:
    case MP_ST_TXN_RESULT:
        MP_SharedBox_OnMessage(data, len);
        break;
    case MP_ST_CURSOR:
        MP_SharedBox_OnCursor(data, len);
        break;
    case MP_ST_ITEM_LIST:
        if (PcOpen())
        {
            OnItemList(data, len);
            if (sPc.mode == MODE_WAIT)
                sPc.mode = MODE_LIST;
            sPc.dirty = TRUE;
        }
        break;
    case MP_ST_ITEMS_CHANGED:
        if (PcOpen() && sPc.view == VIEW_SHARED)
            RequestItems();
        break;
    case MP_ST_ITEM_PUT:
    case MP_ST_ITEM_TAKE:
        if (len >= 5)
            OnItemDone(data[0], data[1] | (data[2] << 8), data[3] | (data[4] << 8));
        if (PcOpen())
        {
            RequestItems();
            sPc.dirty = TRUE;
        }
        break;
    }
}
