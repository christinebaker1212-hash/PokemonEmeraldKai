// Moemon Co-op: CO-OP menu > TRADE. Trade with a friend anywhere: a
// Pokémon, up to two kinds of items and money, in any mix (a Pokémon for
// items, money for a Pokémon...). Pokémon that evolve by trading evolve.
//
// Both players build an offer; any change un-readies both. When both are
// ready, both confirm, then each sends its Pokémon (if any) with the version
// of both offers it agreed to. Nothing changes hands until each side has the
// other's confirmation for exactly the same offers.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "pokedex.h"
#include "mp_net.h"
#include "field_weather.h"
#include "evolution_scene.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "script.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "constants/characters.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
u8 MP_GiveMonKeepOT(struct Pokemon *mon);

enum {
    MPR_TRADE_ASK = MPR_TRADE_FIRST,
    MPR_TRADE_ANSWER,      // u8 answer
    MPR_TRADE_OFFER,       // see PackOffer
    MPR_TRADE_COMMIT,      // u16 mySeq, u16 yourSeq, u8 hasMon, [struct Pokemon]
    MPR_TRADE_CANCEL,      // u8 reason
};

enum { ANSWER_NO, ANSWER_YES, ANSWER_BUSY };
enum { CANCEL_PLAIN, CANCEL_ROOM };

#define MAX_BAG 120
#define ASK_TIMEOUT (60 * 30)

struct Offer
{
    u16 seq;
    u16 seenSeq;      // the partner's offer this one was made against
    bool8 ready;
    u8 monSlot;       // party slot, 0xFF = none (only meaningful for our own offer)
    u16 species;
    u8 level;
    bool8 isEgg;
    u8 nick[POKEMON_NAME_LENGTH + 1];
    u16 item[2];
    u16 qty[2];
    u32 money;
};

struct BagEntry
{
    u16 item;
    u16 qty;
};

enum { ST_IDLE, ST_PICK_FRIEND, ST_ASKING, ST_ASKED, ST_TRADING, ST_CONFIRM, ST_COMMITTED, ST_EVOLVE };
enum { ROW_MON, ROW_ITEM1, ROW_ITEM2, ROW_MONEY, ROW_READY, ROW_CANCEL, ROW_COUNT };

static EWRAM_DATA struct
{
    u8 state;
    u8 taskId;
    s8 partner;
    u8 partnerName[PLAYER_NAME_LENGTH + 1];
    u16 timer;
    u16 cursor, scroll;
    u8 friends[MP_MAX_PLAYERS];
    u8 friendCount;
    struct Offer mine, theirs;
    bool8 haveTheirs;
    struct BagEntry bag[MAX_BAG];
    u16 bagCount;
    u8 itemChoice[2];      // index into bag (0xFF none)
    bool8 gotCommit;
    struct Pokemon theirMon;
    bool8 theirHasMon;
    u16 commitTheirSeq, commitMySeq;
    bool8 sentCommit;
    s8 askedBy;            // someone asked us
    u16 askedTimer;
    u8 evolveSlot;
    char note[72];
    bool8 dirty;
} sTrade = {0};

static void Task_Trade(u8 taskId);
static bool8 sInit;
static void ResetTrade(void);

static void InitIfNeeded(void)
{
    if (sInit)
        return;
    sInit = TRUE;
    sTrade.askedBy = -1;
    ResetTrade();
}

// ---------------------------------------------------------------------------
// Helpers

static void SetNote(const char *ascii)
{
    snprintf(sTrade.note, sizeof(sTrade.note), "%s", ascii);
    sTrade.dirty = TRUE;
}

static bool8 PartnerHere(void)
{
    return sTrade.partner >= 0 && sTrade.partner < MP_MAX_PLAYERS && gMpPlayers[sTrade.partner].present;
}

static void SendTo(u8 to, const u8 *data, u16 len)
{
    MP_SendRelay(to, data, len);
}

static void SendByte(u8 to, u8 type, u8 v)
{
    u8 buf[2] = {type, v};
    SendTo(to, buf, 2);
}

static void BuildBag(void)
{
    static const u8 pockets[] = {ITEMS_POCKET, BALLS_POCKET, BERRIES_POCKET, TMHM_POCKET};
    u8 p;
    sTrade.bagCount = 0;
    for (p = 0; p < ARRAY_COUNT(pockets); p++)
    {
        u16 i;
        for (i = 0; i < gBagPockets[pockets[p]].capacity && sTrade.bagCount < MAX_BAG; i++)
        {
            u16 item = BagGetItemIdByPocketPosition(pockets[p] + 1, i);
            u16 qty = BagGetQuantityByPocketPosition(pockets[p] + 1, i);
            if (item == ITEM_NONE || qty == 0 || (item >= ITEM_HM01 && item <= ITEM_HM08))
                continue;
            sTrade.bag[sTrade.bagCount].item = item;
            sTrade.bag[sTrade.bagCount].qty = qty;
            sTrade.bagCount++;
        }
    }
}

static u8 UsableMons(s8 exceptSlot)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (i == exceptSlot)
            continue;
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) > 0)
            n++;
    }
    return n;
}

static void FillMonInOffer(struct Offer *o)
{
    if (o->monSlot == 0xFF)
    {
        o->species = SPECIES_NONE;
        return;
    }
    o->species = GetMonData(&gPlayerParty[o->monSlot], MON_DATA_SPECIES);
    o->level = GetMonData(&gPlayerParty[o->monSlot], MON_DATA_LEVEL);
    o->isEgg = GetMonData(&gPlayerParty[o->monSlot], MON_DATA_IS_EGG);
    GetMonData(&gPlayerParty[o->monSlot], MON_DATA_NICKNAME, o->nick);
    StringGet_Nickname(o->nick);
}

static void PutU16(u8 *p, u16 v) { p[0] = v; p[1] = v >> 8; }
static u16 GetU16(const u8 *p) { return p[0] | (p[1] << 8); }

static void SendOffer(void)
{
    u8 buf[48];
    u8 i;
    memset(buf, 0, sizeof(buf));
    buf[0] = MPR_TRADE_OFFER;
    PutU16(buf + 1, sTrade.mine.seq);
    PutU16(buf + 3, sTrade.mine.seenSeq);
    buf[5] = sTrade.mine.ready;
    PutU16(buf + 6, sTrade.mine.species);
    buf[8] = sTrade.mine.level;
    buf[9] = sTrade.mine.isEgg;
    for (i = 0; i < POKEMON_NAME_LENGTH + 1; i++)
        buf[10 + i] = sTrade.mine.nick[i];
    PutU16(buf + 21, sTrade.mine.item[0]);
    PutU16(buf + 23, sTrade.mine.qty[0]);
    PutU16(buf + 25, sTrade.mine.item[1]);
    PutU16(buf + 27, sTrade.mine.qty[1]);
    buf[29] = sTrade.mine.money; buf[30] = sTrade.mine.money >> 8; buf[31] = sTrade.mine.money >> 16; buf[32] = sTrade.mine.money >> 24;
    SendTo(sTrade.partner, buf, 33);
}

// Our offer changed: new version, not ready
static void OfferChanged(void)
{
    FillMonInOffer(&sTrade.mine);
    sTrade.mine.seq++;
    sTrade.mine.ready = FALSE;
    sTrade.mine.seenSeq = sTrade.theirs.seq;
    SendOffer();
    sTrade.dirty = TRUE;
}

static void ResetTrade(void)
{
    u8 askedBy = sTrade.askedBy;
    u16 askedTimer = sTrade.askedTimer;
    memset(&sTrade, 0, sizeof(sTrade));
    sTrade.partner = -1;
    sTrade.askedBy = askedBy;
    sTrade.askedTimer = askedTimer;
    sTrade.taskId = TASK_NONE;
    sTrade.mine.monSlot = 0xFF;
    sTrade.itemChoice[0] = sTrade.itemChoice[1] = 0xFF;
}

static void CloseUi(void)
{
    if (MpUi_IsOpen())
        MpUi_Close();
    if (sTrade.taskId != TASK_NONE && FuncIsActiveTask(Task_Trade))
        DestroyTask(sTrade.taskId);
    sTrade.taskId = TASK_NONE;
}

static void EndTrade(const char *notice)
{
    CloseUi();
    ResetTrade();
    sTrade.askedBy = -1;
    if (notice)
        MP_ShowNotice(notice);
}

static void StartTradeScreen(void)
{
    BuildBag();
    sTrade.state = ST_TRADING;
    sTrade.cursor = 0;
    sTrade.mine.monSlot = 0xFF;
    sTrade.itemChoice[0] = sTrade.itemChoice[1] = 0xFF;
    memset(&sTrade.theirs, 0, sizeof(sTrade.theirs));
    sTrade.haveTheirs = FALSE;
    OfferChanged();
}

// ---------------------------------------------------------------------------
// Drawing

static void MonText(const struct Offer *o, u8 *dst)
{
    if (o->species == SPECIES_NONE)
    {
        MP_AsciiToGame(dst, "-", 4);
        return;
    }
    StringCopy(dst, o->nick);
    if (!o->isEgg)
    {
        u8 lv[8] = {CHAR_SPACE, CHAR_LV};
        ConvertIntToDecimalStringN(lv + 2, o->level, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(dst, lv);
    }
}

static void ItemText(u16 item, u16 qty, u8 *dst)
{
    u8 q[8];
    if (item == ITEM_NONE || qty == 0)
    {
        MP_AsciiToGame(dst, "-", 4);
        return;
    }
    CopyItemName(item, dst);
    StringAppend(dst, (const u8[]){CHAR_SPACE, EOS});
    MpUi_Qty(q, qty);
    StringAppend(dst, q);
}

static void MoneyText(u32 money, u8 *dst)
{
    dst[0] = CHAR_CURRENCY;
    ConvertIntToDecimalStringN(dst + 1, money, STR_CONV_MODE_LEFT_ALIGN, 7);
}

static void DrawTrading(void)
{
    static const char *const labels[] = {"MON ", "ITEM ", "ITEM ", "MONEY "};
    char title[48], them[16];
    u8 i, left[64], right[48], tmp[48];
    MP_GameToAscii(them, sTrade.partnerName, PLAYER_NAME_LENGTH);
    snprintf(title, sizeof(title), "TRADE  -  YOU   |   %s", them);
    MpUi_Begin(title);
    for (i = 0; i < ROW_COUNT; i++)
    {
        bool8 sel = (i == sTrade.cursor) && sTrade.state == ST_TRADING;
        if (i <= ROW_MONEY)
        {
            MP_AsciiToGame(left, labels[i], 8);
            if (i == ROW_MON)
            {
                MonText(&sTrade.mine, tmp);
                StringAppend(left, tmp);
                MonText(&sTrade.theirs, right);
            }
            else if (i == ROW_MONEY)
            {
                MoneyText(sTrade.mine.money, tmp);
                StringAppend(left, tmp);
                MoneyText(sTrade.theirs.money, right);
            }
            else
            {
                u8 k = i - ROW_ITEM1;
                ItemText(sTrade.mine.item[k], sTrade.mine.qty[k], tmp);
                StringAppend(left, tmp);
                ItemText(sTrade.theirs.item[k], sTrade.theirs.qty[k], right);
            }
            if (!sTrade.haveTheirs)
                MP_AsciiToGame(right, "...", 4);
            MpUi_Row(i, sel, left, right);
        }
        else if (i == ROW_READY)
        {
            MpUi_RowAscii(i, sel, sTrade.mine.ready ? "READY!  ({A} CHANGE)" : "READY",
                          sTrade.theirs.ready ? "READY!" : "choosing...");
        }
        else
            MpUi_RowAscii(i, sel, "CANCEL TRADE", NULL);
    }
    MpUi_End();
    if (sTrade.note[0])
        MpUi_HelpAscii(sTrade.note);
    else if (sTrade.state == ST_CONFIRM)
        MpUi_HelpAscii("Both ready! Trade these?  {A} YES   {B} NO");
    else if (sTrade.state == ST_COMMITTED)
        MpUi_HelpAscii("Trading...");
    else if (sTrade.cursor == ROW_MON)
        MpUi_HelpAscii("{LEFTRIGHT} pick a POK~MON from your party");
    else if ((sTrade.cursor == ROW_ITEM1 || sTrade.cursor == ROW_ITEM2) && sTrade.mine.item[sTrade.cursor - ROW_ITEM1] != ITEM_NONE)
        MpUi_HelpItem(sTrade.mine.item[sTrade.cursor - ROW_ITEM1]);
    else if (sTrade.cursor == ROW_ITEM1 || sTrade.cursor == ROW_ITEM2)
        MpUi_HelpAscii("{LEFTRIGHT} item   {L}{R} how many");
    else if (sTrade.cursor == ROW_MONEY)
        MpUi_HelpAscii("{LEFTRIGHT} 100   {L}{R} 1000");
    else
        MpUi_HelpAscii("{A} SELECT");
}

static void Draw(void)
{
    sTrade.dirty = FALSE;
    if (!MpUi_IsOpen())
        return;
    switch (sTrade.state)
    {
    case ST_PICK_FRIEND:
    {
        u16 i, first;
        MpUi_Begin("TRADE WITH WHO?");
        first = MpUi_ScrollFor(sTrade.cursor, sTrade.friendCount + 1, &sTrade.scroll);
        for (i = first; i < sTrade.friendCount + 1 && i < first + MPUI_ROWS; i++)
        {
            u8 name[PLAYER_NAME_LENGTH + 1], where[32];
            if (i == sTrade.friendCount)
            {
                MpUi_RowAscii(i - first, i == sTrade.cursor, "CANCEL", NULL);
                continue;
            }
            StringCopyN(name, gMpPlayers[sTrade.friends[i]].name, PLAYER_NAME_LENGTH);
            name[PLAYER_NAME_LENGTH] = EOS;
            MP_Friends_LocationName(sTrade.friends[i], where);
            MpUi_Row(i - first, i == sTrade.cursor, name, where);
        }
        if (sTrade.friendCount == 0)
            MpUi_TextAscii(2, "Nobody else is online right now.");
        MpUi_End();
        MpUi_HelpAscii(sTrade.note[0] ? sTrade.note : "Friends can be anywhere.  {A} ASK  {B} BACK");
        break;
    }
    case ST_ASKING:
    {
        char line[64], name[16];
        MP_GameToAscii(name, sTrade.partnerName, PLAYER_NAME_LENGTH);
        MpUi_Begin("TRADE");
        snprintf(line, sizeof(line), "Waiting for %s to answer...", name);
        MpUi_TextAscii(1, line);
        MpUi_End();
        MpUi_HelpAscii("{B} CANCEL");
        break;
    }
    case ST_ASKED:
    {
        char line[64], name[16];
        MP_GameToAscii(name, sTrade.partnerName, PLAYER_NAME_LENGTH);
        MpUi_Begin("TRADE REQUEST");
        snprintf(line, sizeof(line), "%s wants to trade with you!", name);
        MpUi_TextAscii(0, line);
        MpUi_RowAscii(2, sTrade.cursor == 0, "TRADE", NULL);
        MpUi_RowAscii(3, sTrade.cursor == 1, "NO THANKS", NULL);
        MpUi_End();
        MpUi_HelpAscii("{A} SELECT");
        break;
    }
    case ST_TRADING:
    case ST_CONFIRM:
    case ST_COMMITTED:
        DrawTrading();
        break;
    }
}

// ---------------------------------------------------------------------------
// Doing the trade

static bool8 OfferStillValid(void)
{
    u8 k;
    if (sTrade.mine.monSlot != 0xFF && GetMonData(&gPlayerParty[sTrade.mine.monSlot], MON_DATA_SPECIES) != sTrade.mine.species)
        return FALSE;
    for (k = 0; k < 2; k++)
        if (sTrade.mine.item[k] && !CheckBagHasItem(sTrade.mine.item[k], sTrade.mine.qty[k] + (sTrade.mine.item[k] == sTrade.mine.item[k ^ 1] && k ? sTrade.mine.qty[0] : 0)))
            return FALSE;
    return IsEnoughMoney(&gSaveBlock1Ptr->money, sTrade.mine.money);
}

// Can we take what they offer? (party/PC space is checked when giving)
static bool8 CanReceive(char *why, int whyLen)
{
    u8 k;
    for (k = 0; k < 2; k++)
        if (sTrade.theirs.item[k] && !CheckBagHasSpace(sTrade.theirs.item[k], sTrade.theirs.qty[k]))
        {
            snprintf(why, whyLen, "There's no room in your BAG for their items.");
            return FALSE;
        }
    if (sTrade.mine.monSlot != 0xFF && !sTrade.mine.isEgg && UsableMons(sTrade.mine.monSlot) == 0
     && (sTrade.theirs.species == SPECIES_NONE || sTrade.theirs.isEgg))
    {
        snprintf(why, whyLen, "Keep at least one POK~MON that can battle.");
        return FALSE;
    }
    if (GetMoney(&gSaveBlock1Ptr->money) - sTrade.mine.money + sTrade.theirs.money > 999999)
    {
        snprintf(why, whyLen, "You can't carry that much money.");
        return FALSE;
    }
    return TRUE;
}

static void SendCommit(void)
{
    u8 buf[6 + sizeof(struct Pokemon)];
    buf[0] = MPR_TRADE_COMMIT;
    PutU16(buf + 1, sTrade.mine.seq);
    PutU16(buf + 3, sTrade.theirs.seq);
    buf[5] = sTrade.mine.monSlot != 0xFF;
    if (buf[5])
        memcpy(buf + 6, &gPlayerParty[sTrade.mine.monSlot], sizeof(struct Pokemon));
    SendTo(sTrade.partner, buf, buf[5] ? sizeof(buf) : 6);
    sTrade.sentCommit = TRUE;
}

static void Task_TradeEvolve(u8 taskId);

// Both confirmed the same offers: swap
static void ApplyTrade(void)
{
    u8 k, gotSlot = 0xFF;
    char msg[160], them[16];
    struct Offer theirs = sTrade.theirs;

    // Give away ours
    if (sTrade.mine.monSlot != 0xFF)
    {
        ZeroMonData(&gPlayerParty[sTrade.mine.monSlot]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
    }
    for (k = 0; k < 2; k++)
        if (sTrade.mine.item[k])
            RemoveBagItem(sTrade.mine.item[k], sTrade.mine.qty[k]);
    if (sTrade.mine.money)
        RemoveMoney(&gSaveBlock1Ptr->money, sTrade.mine.money);

    // Take theirs
    for (k = 0; k < 2; k++)
        if (theirs.item[k])
            AddBagItem(theirs.item[k], theirs.qty[k]);
    if (theirs.money)
        AddMoney(&gSaveBlock1Ptr->money, theirs.money);
    if (sTrade.theirHasMon)
    {
        u8 result = MP_GiveMonKeepOT(&sTrade.theirMon);
        if (result == MON_GIVEN_TO_PARTY)
            gotSlot = gPlayerPartyCount - 1;
        if (!GetMonData(&sTrade.theirMon, MON_DATA_IS_EGG))
        {
            u16 nat = SpeciesToNationalPokedexNum(GetMonData(&sTrade.theirMon, MON_DATA_SPECIES));
            GetSetPokedexFlag(nat, FLAG_SET_SEEN);
            GetSetPokedexFlag(nat, FLAG_SET_CAUGHT);
        }
    }
    PlaySE(SE_SUCCESS);
    MP_Events_OnTrade();
    MP_GameToAscii(them, sTrade.partnerName, PLAYER_NAME_LENGTH);
    snprintf(msg, sizeof(msg), "Trade with %s complete!", them);
    MPNet_Log("trade with player %d complete", sTrade.partner + 1);

    // A Pokemon that evolves by trading evolves now
    if (gotSlot != 0xFF && GetEvolutionTargetSpecies(&gPlayerParty[gotSlot], EVO_MODE_TRADE, ITEM_NONE) != SPECIES_NONE)
    {
        CloseUi();
        sTrade.state = ST_EVOLVE;
        sTrade.evolveSlot = gotSlot;
        LockPlayerFieldControls();
        FadeScreen(FADE_TO_BLACK, 0);
        CreateTask(Task_TradeEvolve, 10);
        return;
    }
    EndTrade(NULL);
    MpUi_Open();
    MpUi_Begin("TRADE");
    MpUi_TextAscii(1, msg);
    MpUi_End();
    MpUi_HelpAscii("{A} OK");
    sTrade.state = ST_IDLE;
    sTrade.taskId = CreateTask(Task_Trade, 80);
    sTrade.note[0] = 1; // marks "done, waiting for A"
}

static void Task_TradeEvolve(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    {
        u8 slot = sTrade.evolveSlot;
        u16 species = GetEvolutionTargetSpecies(&gPlayerParty[slot], EVO_MODE_TRADE, ITEM_NONE);
        DestroyTask(taskId);
        ResetTrade();
        sTrade.askedBy = -1;
        CleanupOverworldWindowsAndTilemaps();
        gCB2_AfterEvolution = CB2_ReturnToField;
        BeginEvolutionScene(&gPlayerParty[slot], species, FALSE, slot);
    }
}

static void TryFinish(void)
{
    if (sTrade.state != ST_COMMITTED || !sTrade.gotCommit)
        return;
    if (sTrade.commitTheirSeq != sTrade.theirs.seq || sTrade.commitMySeq != sTrade.mine.seq)
    {
        // They agreed to different versions: back to choosing
        sTrade.state = ST_TRADING;
        sTrade.gotCommit = sTrade.sentCommit = FALSE;
        OfferChanged();
        return;
    }
    ApplyTrade();
}

// ---------------------------------------------------------------------------
// Input

static void ChangeRow(s8 dir, bool8 big)
{
    switch (sTrade.cursor)
    {
    case ROW_MON:
    {
        // NONE, then each party slot
        s16 v = (sTrade.mine.monSlot == 0xFF) ? 0 : sTrade.mine.monSlot + 1;
        s16 n = gPlayerPartyCount + 1;
        v = (v + dir + n) % n;
        sTrade.mine.monSlot = (v == 0) ? 0xFF : v - 1;
        break;
    }
    case ROW_ITEM1:
    case ROW_ITEM2:
    {
        u8 k = sTrade.cursor - ROW_ITEM1;
        if (big)
        {
            if (sTrade.itemChoice[k] == 0xFF)
                return;
            {
                u16 max = sTrade.bag[sTrade.itemChoice[k]].qty, q = sTrade.mine.qty[k];
                if (dir > 0)
                    q = (q >= max) ? 1 : q + 1;
                else
                    q = (q <= 1) ? max : q - 1;
                sTrade.mine.qty[k] = q;
            }
        }
        else
        {
            s16 v = (sTrade.itemChoice[k] == 0xFF) ? 0 : sTrade.itemChoice[k] + 1;
            s16 n = sTrade.bagCount + 1;
            v = (v + dir + n) % n;
            sTrade.itemChoice[k] = (v == 0) ? 0xFF : v - 1;
            sTrade.mine.item[k] = (v == 0) ? ITEM_NONE : sTrade.bag[v - 1].item;
            sTrade.mine.qty[k] = (v == 0) ? 0 : 1;
        }
        break;
    }
    case ROW_MONEY:
    {
        s32 m = (s32)sTrade.mine.money + dir * (big ? 1000 : 100);
        s32 have = GetMoney(&gSaveBlock1Ptr->money);
        if (m < 0)
            m = 0;
        if (m > have)
            m = have;
        sTrade.mine.money = m;
        break;
    }
    default:
        return;
    }
    PlaySE(SE_SELECT);
    OfferChanged();
}

static void HandleTradingInput(void)
{
    if (MpUi_MoveCursor(&sTrade.cursor, ROW_COUNT))
    {
        sTrade.dirty = TRUE;
        return;
    }
    if (JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
        ChangeRow(JOY_REPEAT(DPAD_RIGHT) ? 1 : -1, FALSE);
    else if (JOY_REPEAT(L_BUTTON | R_BUTTON))
        ChangeRow(JOY_REPEAT(R_BUTTON) ? 1 : -1, TRUE);
    else if (JOY_NEW(B_BUTTON))
    {
        sTrade.cursor = ROW_CANCEL;
        PlaySE(SE_SELECT);
        sTrade.dirty = TRUE;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sTrade.cursor == ROW_CANCEL)
        {
            SendByte(sTrade.partner, MPR_TRADE_CANCEL, CANCEL_PLAIN);
            PlaySE(SE_SELECT);
            EndTrade(NULL);
            return;
        }
        if (sTrade.cursor == ROW_READY)
        {
            char why[72];
            if (!sTrade.mine.ready && !CanReceive(why, sizeof(why)))
            {
                PlaySE(SE_FAILURE);
                SetNote(why);
                return;
            }
            if (!sTrade.mine.ready && sTrade.mine.species == SPECIES_NONE && !sTrade.mine.item[0] && !sTrade.mine.item[1]
             && !sTrade.mine.money && sTrade.theirs.species == SPECIES_NONE && !sTrade.theirs.item[0]
             && !sTrade.theirs.item[1] && !sTrade.theirs.money)
            {
                PlaySE(SE_FAILURE);
                SetNote("Nothing to trade yet!");
                return;
            }
            sTrade.mine.ready = !sTrade.mine.ready;
            sTrade.mine.seenSeq = sTrade.theirs.seq;
            PlaySE(SE_SELECT);
            SendOffer();
            sTrade.dirty = TRUE;
        }
    }
}

// Both ready on the same versions -> confirm
static void CheckBothReady(void)
{
    if (sTrade.state == ST_TRADING && sTrade.mine.ready && sTrade.theirs.ready && sTrade.haveTheirs
     && sTrade.theirs.seenSeq == sTrade.mine.seq && sTrade.mine.seenSeq == sTrade.theirs.seq)
    {
        sTrade.state = ST_CONFIRM;
        PlaySE(SE_PIN);
        sTrade.dirty = TRUE;
    }
}

static void Task_Trade(u8 taskId)
{
    // The finished screen: A closes it
    if (sTrade.state == ST_IDLE)
    {
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            sTrade.note[0] = 0;
            CloseUi();
        }
        return;
    }
    if (sTrade.state != ST_PICK_FRIEND && !PartnerHere())
    {
        EndTrade("Your trading partner left, so the trade was called off.");
        return;
    }
    if (sTrade.note[0] && JOY_NEW(A_BUTTON | B_BUTTON | DPAD_ANY | L_BUTTON | R_BUTTON))
    {
        sTrade.note[0] = 0;
        sTrade.dirty = TRUE;
        return;
    }
    switch (sTrade.state)
    {
    case ST_PICK_FRIEND:
        if (MpUi_MoveCursor(&sTrade.cursor, sTrade.friendCount + 1))
            sTrade.dirty = TRUE;
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sTrade.cursor == sTrade.friendCount))
        {
            PlaySE(SE_SELECT);
            EndTrade(NULL);
            return;
        }
        else if (JOY_NEW(A_BUTTON))
        {
            u8 id = sTrade.friends[sTrade.cursor];
            PlaySE(SE_SELECT);
            sTrade.partner = id;
            StringCopyN(sTrade.partnerName, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
            sTrade.partnerName[PLAYER_NAME_LENGTH] = EOS;
            {
                u8 b = MPR_TRADE_ASK;
                SendTo(id, &b, 1);
            }
            sTrade.state = ST_ASKING;
            sTrade.timer = 0;
            sTrade.dirty = TRUE;
        }
        break;
    case ST_ASKING:
        if (JOY_NEW(B_BUTTON) || ++sTrade.timer > ASK_TIMEOUT + 60 * 5)
        {
            SendByte(sTrade.partner, MPR_TRADE_CANCEL, CANCEL_PLAIN);
            EndTrade(JOY_NEW(B_BUTTON) ? NULL : "No answer, so the trade request was called off.");
            return;
        }
        break;
    case ST_ASKED:
        if (MpUi_MoveCursor(&sTrade.cursor, 2))
            sTrade.dirty = TRUE;
        else if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            bool8 yes = JOY_NEW(A_BUTTON) && sTrade.cursor == 0;
            PlaySE(SE_SELECT);
            SendByte(sTrade.partner, MPR_TRADE_ANSWER, yes ? ANSWER_YES : ANSWER_NO);
            if (yes)
                StartTradeScreen();
            else
            {
                EndTrade(NULL);
                return;
            }
        }
        break;
    case ST_TRADING:
        HandleTradingInput();
        if (sTrade.taskId == TASK_NONE)
            return;
        CheckBothReady();
        break;
    case ST_CONFIRM:
        if (JOY_NEW(A_BUTTON))
        {
            if (!OfferStillValid())
            {
                PlaySE(SE_FAILURE);
                sTrade.state = ST_TRADING;
                OfferChanged();
                SetNote("Your offer changed, so check it again.");
                break;
            }
            PlaySE(SE_SELECT);
            sTrade.state = ST_COMMITTED;
            SendCommit();
            sTrade.dirty = TRUE;
            TryFinish();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sTrade.state = ST_TRADING;
            sTrade.mine.ready = FALSE;
            sTrade.gotCommit = FALSE;
            SendOffer();
            sTrade.dirty = TRUE;
        }
        break;
    case ST_COMMITTED:
        TryFinish();
        break;
    }
    if (sTrade.taskId != TASK_NONE && sTrade.dirty)
        Draw();
}

void MP_Trade_OpenMenu(void)
{
    InitIfNeeded();
    if (!MP_IsConnected() || gMpLocalId < 0)
    {
        MP_ShowNotice("Go online (CO-OP menu: {COOP}) to trade with friends.");
        return;
    }
    if (sTrade.state != ST_IDLE || sTrade.taskId != TASK_NONE)
        return;
    ResetTrade();
    sTrade.askedBy = -1;
    sTrade.friendCount = MP_Friends_List(sTrade.friends);
    sTrade.state = ST_PICK_FRIEND;
    MpUi_Open();
    sTrade.taskId = CreateTask(Task_Trade, 80);
    Draw();
}

// ---------------------------------------------------------------------------
// From the network

void MP_Trade_OnRelay(u8 from, const u8 *data, u16 len)
{
    InitIfNeeded();
    if (from >= MP_MAX_PLAYERS || len < 1)
        return;
    switch (data[0])
    {
    case MPR_TRADE_ASK:
        if (sTrade.state != ST_IDLE || sTrade.askedBy >= 0 || sTrade.taskId != TASK_NONE)
        {
            SendByte(from, MPR_TRADE_ANSWER, ANSWER_BUSY);
            return;
        }
        sTrade.askedBy = from;
        sTrade.askedTimer = 0;
        break;
    case MPR_TRADE_ANSWER:
        if (sTrade.state == ST_ASKING && from == sTrade.partner && len >= 2)
        {
            if (data[1] == ANSWER_YES)
                StartTradeScreen();
            else
            {
                char msg[96], name[16];
                MP_GameToAscii(name, sTrade.partnerName, PLAYER_NAME_LENGTH);
                snprintf(msg, sizeof(msg), data[1] == ANSWER_BUSY ? "%s is busy right now. Try again in a moment."
                                                                  : "%s doesn't want to trade right now.", name);
                EndTrade(msg);
            }
        }
        break;
    case MPR_TRADE_OFFER:
        if (from == sTrade.partner && len >= 33 && sTrade.state >= ST_TRADING && sTrade.state <= ST_COMMITTED)
        {
            struct Offer *o = &sTrade.theirs;
            u16 oldSeq = o->seq;
            u8 i;
            o->seq = GetU16(data + 1);
            o->seenSeq = GetU16(data + 3);
            o->ready = data[5];
            o->species = GetU16(data + 6);
            o->level = data[8];
            o->isEgg = data[9];
            for (i = 0; i < POKEMON_NAME_LENGTH + 1; i++)
                o->nick[i] = data[10 + i];
            o->nick[POKEMON_NAME_LENGTH] = EOS;
            o->item[0] = GetU16(data + 21);
            o->qty[0] = GetU16(data + 23);
            o->item[1] = GetU16(data + 25);
            o->qty[1] = GetU16(data + 27);
            o->money = data[29] | (data[30] << 8) | (data[31] << 16) | ((u32)data[32] << 24);
            if (o->species >= NUM_SPECIES)
                o->species = SPECIES_NONE;
            if (o->item[0] >= ITEMS_COUNT) o->item[0] = ITEM_NONE;
            if (o->item[1] >= ITEMS_COUNT) o->item[1] = ITEM_NONE;
            sTrade.haveTheirs = TRUE;
            // Their offer changed: we're no longer ready for it
            if (o->seq != oldSeq && (sTrade.state == ST_TRADING || sTrade.state == ST_CONFIRM))
            {
                sTrade.state = ST_TRADING;
                sTrade.gotCommit = FALSE;
                if (sTrade.mine.ready || sTrade.mine.seenSeq != o->seq)
                {
                    sTrade.mine.ready = FALSE;
                    sTrade.mine.seenSeq = o->seq;
                    SendOffer();
                }
            }
            else if ((sTrade.state == ST_CONFIRM || sTrade.state == ST_COMMITTED) && !o->ready)
            {
                // They backed out of the confirmation
                if (sTrade.state == ST_COMMITTED)
                {
                    sTrade.mine.ready = FALSE;
                    SendOffer();
                }
                sTrade.state = ST_TRADING;
                sTrade.gotCommit = sTrade.sentCommit = FALSE;
            }
            sTrade.dirty = TRUE;
        }
        break;
    case MPR_TRADE_COMMIT:
        if (from == sTrade.partner && len >= 6 && sTrade.state >= ST_TRADING && sTrade.state <= ST_COMMITTED)
        {
            sTrade.commitTheirSeq = GetU16(data + 1);   // their offer version they're giving
            sTrade.commitMySeq = GetU16(data + 3);      // our offer version they accept
            sTrade.theirHasMon = data[5] && len >= 6 + sizeof(struct Pokemon);
            if (sTrade.theirHasMon)
                memcpy(&sTrade.theirMon, data + 6, sizeof(struct Pokemon));
            sTrade.gotCommit = TRUE;
            TryFinish();
        }
        break;
    case MPR_TRADE_CANCEL:
        if (from == sTrade.partner && sTrade.state != ST_IDLE)
        {
            char msg[96], name[16];
            MP_GameToAscii(name, sTrade.partnerName, PLAYER_NAME_LENGTH);
            snprintf(msg, sizeof(msg), "%s called off the trade.", name);
            EndTrade(msg);
        }
        else if (from == sTrade.askedBy)
            sTrade.askedBy = -1;
        break;
    }
}

// Every frame while online: show a friend's trade request once we're free
void MP_Trade_Update(void)
{
    InitIfNeeded();
    if (sTrade.askedBy < 0)
        return;
    if (!gMpPlayers[sTrade.askedBy].present)
    {
        sTrade.askedBy = -1;
        return;
    }
    if (sTrade.state == ST_IDLE && sTrade.taskId == TASK_NONE && MP_IsLocalFree() && !MpUi_IsOpen())
    {
        u8 from = sTrade.askedBy;
        ResetTrade();
        sTrade.askedBy = -1;
        sTrade.partner = from;
        StringCopyN(sTrade.partnerName, gMpPlayers[from].name, PLAYER_NAME_LENGTH);
        sTrade.partnerName[PLAYER_NAME_LENGTH] = EOS;
        sTrade.state = ST_ASKED;
        sTrade.cursor = 0;
        PlaySE(SE_PIN);
        MpUi_Open();
        sTrade.taskId = CreateTask(Task_Trade, 80);
        Draw();
    }
    else if (++sTrade.askedTimer > ASK_TIMEOUT)
    {
        SendByte(sTrade.askedBy, MPR_TRADE_ANSWER, ANSWER_BUSY);
        sTrade.askedBy = -1;
    }
}
