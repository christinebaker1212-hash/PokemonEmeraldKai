// Moemon Co-op: CO-OP menu > SHARED PC, and the client side of the room
// storage (mp_store.c on the server).
//
// The shared PC belongs to the server: Pokémon and items put in by anyone
// in the room can be taken out by anyone, and it's still there the next time
// the same server name and password are hosted. Pokémon keep their original
// trainer. The server decides who gets a Pokémon, so it can't be duplicated.
//
// Also here: the counter cache (team goals, achievements, the weekly race),
// see MP_Counter_*.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
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

void MPNet_Log(const char *fmt, ...);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);

STATIC_ASSERT(sizeof(struct Pokemon) + PLAYER_NAME_LENGTH + 1 <= MP_STORE_MON_BYTES, StoredMonFits);

// ---------------------------------------------------------------------------
// Talking to the server

static bool8 CanUseStore(void)
{
    return MP_IsConnected() && gMpLocalId >= 0;
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

// ---------------------------------------------------------------------------
// Shared PC screen

#define MAX_BAG_ENTRIES 120

enum { TAB_MONS, TAB_ITEMS };
enum { MODE_LOADING, MODE_LIST, MODE_PICK_PARTY, MODE_PICK_BAG, MODE_QTY, MODE_WAIT };

struct StoredMon
{
    u8 slot;
    struct Pokemon mon;
    u8 by[PLAYER_NAME_LENGTH + 1];
};

struct ItemEntry
{
    u16 item;
    u16 qty;
};

static EWRAM_DATA struct
{
    u8 taskId;
    u8 tab;
    u8 mode;
    u16 cursor, scroll;
    struct StoredMon mons[MP_STORE_MONS];
    u8 monCount;
    struct ItemEntry items[MP_STORE_ITEMS];
    u8 itemCount;
    struct ItemEntry bag[MAX_BAG_ENTRIES];
    u16 bagCount;
    u16 qtyItem, qty, qtyMax;
    bool8 qtyTake;
    u8 pendingPartySlot;       // party slot being put in (for a refusal)
    struct Pokemon pendingMon;
    bool8 dirty;
    u16 waitTimer;
    char note[64];             // shown in the help line until the next key press
} sPc = {0};

static void Task_SharedPc(u8 taskId);

static void SetNote(const char *ascii)
{
    snprintf(sPc.note, sizeof(sPc.note), "%s", ascii);
}

static void RequestLists(void)
{
    u8 op = MP_ST_PC_LIST;
    StoreSend(&op, 1);
    op = MP_ST_ITEM_LIST;
    StoreSend(&op, 1);
}

static u8 UsableMonCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG)
         && GetMonData(mon, MON_DATA_HP) > 0)
            n++;
    }
    return n;
}

static bool8 IsHM(u16 item)
{
    return item >= ITEM_HM01 && item <= ITEM_HM08;
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

static void MonLabel(const struct Pokemon *mon, u8 *dst)
{
    u8 lv[8];
    GetMonData((struct Pokemon *)mon, MON_DATA_NICKNAME, dst);
    StringGet_Nickname(dst);
    if (GetMonData((struct Pokemon *)mon, MON_DATA_IS_EGG))
        return;
    lv[0] = CHAR_SPACE;
    lv[1] = CHAR_LV;
    ConvertIntToDecimalStringN(lv + 2, GetMonData((struct Pokemon *)mon, MON_DATA_LEVEL), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(dst, lv);
}

static void ItemLabel(u16 item, u8 *dst)
{
    CopyItemName(item, dst);
}

static u16 RowCount(void)
{
    switch (sPc.mode)
    {
    case MODE_PICK_PARTY: return gPlayerPartyCount + 1;
    case MODE_PICK_BAG:   return sPc.bagCount + 1;
    default:              return 1 + (sPc.tab == TAB_MONS ? sPc.monCount : sPc.itemCount);
    }
}

static void Draw(void)
{
    u16 count = RowCount(), first, i;
    u8 left[64], right[24];

    if (sPc.mode == MODE_PICK_PARTY)
        MpUi_Begin("SHARED PC: PUT IN WHICH POK~MON?");
    else if (sPc.mode == MODE_PICK_BAG)
        MpUi_Begin("SHARED PC: PUT IN WHICH ITEM?");
    else
    {
        u8 tabs[40];
        MpUi_Begin("SHARED PC");
        MP_AsciiToGame(tabs, sPc.tab == TAB_MONS ? "POK~MON   ({L}{R} ITEMS)" : "ITEMS   ({L}{R} POK~MON)", 36);
        MpUi_TitleRight(tabs);
    }

    if (sPc.mode == MODE_LOADING)
    {
        MpUi_TextAscii(1, "Connecting to the shared PC...");
        MpUi_End();
        MpUi_HelpAscii("{B} CLOSE");
        return;
    }

    first = MpUi_ScrollFor(sPc.cursor, count, &sPc.scroll);
    for (i = first; i < count && i < first + MPUI_ROWS; i++)
    {
        bool8 sel = (i == sPc.cursor) && sPc.mode != MODE_QTY && sPc.mode != MODE_WAIT;
        u8 row = i - first;
        if (sPc.mode == MODE_PICK_PARTY)
        {
            if (i == gPlayerPartyCount)
                MpUi_RowAscii(row, sel, "CANCEL", NULL);
            else
            {
                MonLabel(&gPlayerParty[i], left);
                MpUi_Row(row, sel, left, NULL);
            }
        }
        else if (sPc.mode == MODE_PICK_BAG)
        {
            if (i == sPc.bagCount)
                MpUi_RowAscii(row, sel, "CANCEL", NULL);
            else
            {
                ItemLabel(sPc.bag[i].item, left);
                MpUi_Qty(right, sPc.bag[i].qty);
                MpUi_Row(row, sel, left, right);
            }
        }
        else if (i == 0)
        {
            MpUi_RowAscii(row, sel, sPc.tab == TAB_MONS ? "PUT IN A POK~MON..." : "PUT IN AN ITEM...", NULL);
        }
        else if (sPc.tab == TAB_MONS)
        {
            struct StoredMon *m = &sPc.mons[i - 1];
            u8 from[24];
            MonLabel(&m->mon, left);
            if (m->by[0] != EOS && m->by[0] != 0)
            {
                MP_AsciiToGame(from, "from ", 8);
                StringAppendN(from, m->by, PLAYER_NAME_LENGTH);
                MpUi_Row(row, sel, left, from);
            }
            else
                MpUi_Row(row, sel, left, NULL);
        }
        else
        {
            ItemLabel(sPc.items[i - 1].item, left);
            MpUi_Qty(right, sPc.items[i - 1].qty);
            MpUi_Row(row, sel, left, right);
        }
    }
    if (sPc.mode == MODE_LIST && count == 1)
        MpUi_TextAscii(2, sPc.tab == TAB_MONS ? "No POK~MON stored yet." : "No items stored yet.");
    MpUi_End();

    if (sPc.note[0])
        MpUi_HelpAscii(sPc.note);
    else if (sPc.mode == MODE_QTY)
    {
        u8 buf[80], name[24], num[8];
        MP_AsciiToGame(buf, sPc.qtyTake ? "TAKE " : "PUT IN ", 10);
        ItemLabel(sPc.qtyItem, name);
        StringAppend(buf, name);
        MP_AsciiToGame(buf + StringLength(buf), "  < ", 6);
        ConvertIntToDecimalStringN(num, sPc.qty, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(buf, num);
        MP_AsciiToGame(buf + StringLength(buf), " >  {A} OK  {B} BACK", 20);
        MpUi_Help(buf);
    }
    else if (sPc.mode == MODE_WAIT)
        MpUi_HelpAscii("One moment...");
    else if (sPc.mode == MODE_LIST && sPc.tab != TAB_MONS && sPc.cursor >= 1 && sPc.cursor <= sPc.itemCount)
        MpUi_HelpItem(sPc.items[sPc.cursor - 1].item);
    else if (sPc.mode == MODE_PICK_BAG && sPc.cursor < sPc.bagCount)
        MpUi_HelpItem(sPc.bag[sPc.cursor].item);
    else if (sPc.mode == MODE_LIST)
        MpUi_HelpAscii(sPc.tab == TAB_MONS ? "{A} TAKE  {L}{R} ITEMS  {B} CLOSE" : "{A} TAKE  {L}{R} POK~MON  {B} CLOSE");
    else
        MpUi_HelpAscii("{A} PUT IN  {B} BACK");
}

static void Close(void)
{
    if (sPc.mode == MODE_WAIT && sPc.pendingPartySlot != 0xFF)
        return; // don't leave while a Pokemon is on its way
    MpUi_Close();
    DestroyTask(sPc.taskId);
    sPc.taskId = TASK_NONE;
}

static void PutInMon(u8 partySlot)
{
    u8 buf[1 + MP_STORE_MON_BYTES];
    struct Pokemon *mon = &gPlayerParty[partySlot];
    memset(buf, 0, sizeof(buf));
    buf[0] = MP_ST_PC_PUT;
    memcpy(buf + 1, mon, sizeof(struct Pokemon));
    StringCopyN(buf + 1 + sizeof(struct Pokemon), gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    buf[1 + sizeof(struct Pokemon) + PLAYER_NAME_LENGTH] = EOS;
    // Out of the party now; back in only if the PC turns it down
    sPc.pendingMon = *mon;
    sPc.pendingPartySlot = partySlot;
    ZeroMonData(mon);
    CompactPartySlots();
    CalculatePlayerPartyCount();
    StoreSend(buf, sizeof(buf));
    sPc.mode = MODE_WAIT;
    sPc.waitTimer = 0;
}

static void SendItemOp(u8 op, u16 item, u16 qty)
{
    u8 buf[5] = {op, item & 0xFF, item >> 8, qty & 0xFF, qty >> 8};
    StoreSend(buf, 5);
    sPc.mode = MODE_WAIT;
    sPc.waitTimer = 0;
}

static void HandleListInput(void)
{
    u16 count = RowCount();
    if (MpUi_MoveCursor(&sPc.cursor, count))
    {
        sPc.dirty = TRUE;
        return;
    }
    if (JOY_NEW(L_BUTTON | R_BUTTON | DPAD_LEFT | DPAD_RIGHT))
    {
        sPc.tab ^= 1;
        sPc.cursor = sPc.scroll = 0;
        PlaySE(SE_SELECT);
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Close();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sPc.cursor == 0 && sPc.tab == TAB_MONS)
        {
            CalculatePlayerPartyCount();
            sPc.mode = MODE_PICK_PARTY;
            sPc.cursor = sPc.scroll = 0;
        }
        else if (sPc.cursor == 0)
        {
            BuildBagList();
            sPc.mode = MODE_PICK_BAG;
            sPc.cursor = sPc.scroll = 0;
        }
        else if (sPc.tab == TAB_MONS)
        {
            u8 op[2] = {MP_ST_PC_TAKE, sPc.mons[sPc.cursor - 1].slot};
            sPc.pendingPartySlot = 0xFF;
            StoreSend(op, 2);
            sPc.mode = MODE_WAIT;
            sPc.waitTimer = 0;
        }
        else
        {
            struct ItemEntry *e = &sPc.items[sPc.cursor - 1];
            sPc.qtyItem = e->item;
            sPc.qtyMax = e->qty;
            sPc.qty = 1;
            sPc.qtyTake = TRUE;
            sPc.mode = MODE_QTY;
        }
        sPc.dirty = TRUE;
    }
}

static void HandlePickInput(void)
{
    u16 count = RowCount();
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
    else if (JOY_NEW(A_BUTTON) && sPc.mode == MODE_PICK_PARTY)
    {
        struct Pokemon *mon = &gPlayerParty[sPc.cursor];
        bool8 usable = !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) > 0;
        if (usable && UsableMonCount() <= 1)
        {
            PlaySE(SE_FAILURE);
            SetNote("You need to keep at least one POK~MON that can battle.");
        }
        else if (sPc.monCount >= MP_STORE_MONS)
        {
            PlaySE(SE_FAILURE);
            SetNote("The shared PC is full.");
        }
        else
        {
            PlaySE(SE_SELECT);
            PutInMon(sPc.cursor);
        }
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON) && sPc.mode == MODE_PICK_BAG)
    {
        PlaySE(SE_SELECT);
        sPc.qtyItem = sPc.bag[sPc.cursor].item;
        sPc.qtyMax = sPc.bag[sPc.cursor].qty;
        sPc.qty = 1;
        sPc.qtyTake = FALSE;
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
        sPc.mode = sPc.qtyTake ? MODE_LIST : MODE_PICK_BAG;
        sPc.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sPc.qtyTake && !CheckBagHasSpace(sPc.qtyItem, sPc.qty))
        {
            PlaySE(SE_FAILURE);
            SetNote("There's no room in your BAG for that.");
        }
        else if (!sPc.qtyTake && !RemoveBagItem(sPc.qtyItem, sPc.qty))
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            PlaySE(SE_SELECT);
            SendItemOp(sPc.qtyTake ? MP_ST_ITEM_TAKE : MP_ST_ITEM_PUT, sPc.qtyItem, sPc.qty);
        }
        sPc.dirty = TRUE;
    }
}

static void Task_SharedPc(u8 taskId)
{
    if (!CanUseStore())
    {
        // Lost the connection: give back a Pokemon that was on its way
        if (sPc.mode == MODE_WAIT && sPc.pendingPartySlot != 0xFF)
        {
            MP_GiveMonKeepOT(&sPc.pendingMon);
            sPc.pendingPartySlot = 0xFF;
        }
        sPc.mode = MODE_LIST;
        Close();
        MP_ShowNotice("The connection was lost, so the shared PC closed.");
        return;
    }
    if (sPc.note[0] && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY | L_BUTTON | R_BUTTON))
    {
        sPc.note[0] = 0;
        sPc.dirty = TRUE;
        return;
    }
    switch (sPc.mode)
    {
    case MODE_LOADING:
        if (JOY_NEW(B_BUTTON))
            Close();
        break;
    case MODE_LIST:
        HandleListInput();
        break;
    case MODE_PICK_PARTY:
    case MODE_PICK_BAG:
        HandlePickInput();
        break;
    case MODE_QTY:
        HandleQtyInput();
        break;
    case MODE_WAIT:
        sPc.waitTimer++;
        break;
    }
    if (sPc.taskId != TASK_NONE && sPc.dirty)
    {
        sPc.dirty = FALSE;
        Draw();
    }
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
    sPc.pendingPartySlot = 0xFF;
    sPc.mode = MODE_LOADING;
    MpUi_Open();
    sPc.taskId = CreateTask(Task_SharedPc, 80);
    RequestLists();
    Draw();
}

// ---------------------------------------------------------------------------
// Replies from the server

static void OnPcList(const u8 *data, u16 len)
{
    u8 count, i;
    u16 pos = 2;
    if (len < 2)
        return;
    count = data[1];
    sPc.monCount = 0;
    for (i = 0; i < count && pos + 1 + MP_STORE_MON_BYTES <= len; i++)
    {
        struct StoredMon *m = &sPc.mons[sPc.monCount++];
        m->slot = data[pos];
        memcpy(&m->mon, data + pos + 1, sizeof(struct Pokemon));
        memcpy(m->by, data + pos + 1 + sizeof(struct Pokemon), PLAYER_NAME_LENGTH + 1);
        m->by[PLAYER_NAME_LENGTH] = EOS;
        pos += 1 + MP_STORE_MON_BYTES;
    }
}

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
    case MP_ST_PC_LIST:
        if (PcOpen())
        {
            OnPcList(data, len);
            if (sPc.mode == MODE_LOADING || sPc.mode == MODE_WAIT)
                sPc.mode = MODE_LIST;
            sPc.dirty = TRUE;
        }
        break;
    case MP_ST_ITEM_LIST:
        if (PcOpen())
        {
            OnItemList(data, len);
            sPc.dirty = TRUE;
        }
        break;
    case MP_ST_PC_CHANGED:
    case MP_ST_ITEMS_CHANGED:
        if (PcOpen() && sPc.mode != MODE_WAIT)
            RequestLists();
        break;
    case MP_ST_PC_PUT:
        if (len >= 3 && sPc.pendingPartySlot != 0xFF)
        {
            if (!data[1])
            {
                MP_GiveMonKeepOT(&sPc.pendingMon);  // turned down: back to the party
                SetNote("The shared PC is full.");
                PlaySE(SE_FAILURE);
            }
            else
            {
                PlaySE(SE_PC_LOGIN);
                SetNote("Stored in the shared PC!");
            }
            sPc.pendingPartySlot = 0xFF;
            if (PcOpen())
            {
                sPc.mode = MODE_LIST;
                sPc.cursor = sPc.scroll = 0;
                RequestLists();
                sPc.dirty = TRUE;
            }
        }
        break;
    case MP_ST_PC_TAKE:
        if (len >= 3 + MP_STORE_MON_BYTES && data[1])
        {
            struct Pokemon mon;
            u8 result;
            memcpy(&mon, data + 3, sizeof(mon));
            result = MP_GiveMonKeepOT(&mon);
            if (result == MON_CANT_GIVE)
            {
                // Party and boxes full: straight back into the shared PC
                u8 buf[1 + MP_STORE_MON_BYTES];
                buf[0] = MP_ST_PC_PUT;
                memcpy(buf + 1, data + 3, MP_STORE_MON_BYTES);
                StoreSend(buf, sizeof(buf));
                SetNote("Your party and boxes are full.");
                PlaySE(SE_FAILURE);
            }
            else
            {
                u16 species = GetMonData(&mon, MON_DATA_SPECIES);
                if (!GetMonData(&mon, MON_DATA_IS_EGG))
                {
                    GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_SEEN);
                    GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_SET_CAUGHT);
                }
                SetNote(result == MON_GIVEN_TO_PARTY ? "Added to your party!" : "Your party is full, so it went to your PC.");
                PlaySE(SE_PC_LOGIN);
            }
        }
        else if (len >= 3)
        {
            SetNote("Someone else took that one first.");
            PlaySE(SE_FAILURE);
        }
        if (PcOpen())
        {
            sPc.mode = MODE_LIST;
            RequestLists();
            sPc.dirty = TRUE;
        }
        break;
    case MP_ST_ITEM_PUT:
    case MP_ST_ITEM_TAKE:
        if (len >= 5)
        {
            u16 item = data[1] | (data[2] << 8), done = data[3] | (data[4] << 8);
            if (data[0] == MP_ST_ITEM_TAKE)
            {
                if (done)
                {
                    AddBagItem(item, done);
                    SetNote("Put in your BAG!");
                    PlaySE(SE_PC_LOGIN);
                }
                else
                    SetNote("Someone else took it first.");
            }
            else
            {
                // Whatever didn't fit goes back in the bag
                if (done < sPc.qty && sPc.qtyItem == item)
                    AddBagItem(item, sPc.qty - done);
                SetNote(done ? "Stored in the shared PC!" : "The shared PC is full.");
                PlaySE(done ? SE_PC_LOGIN : SE_FAILURE);
            }
        }
        if (PcOpen())
        {
            sPc.mode = MODE_LIST;
            RequestLists();
            sPc.dirty = TRUE;
        }
        break;
    }
}
