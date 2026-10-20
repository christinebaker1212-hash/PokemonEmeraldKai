// Moemon Co-op: rescue instead of blacking out. Lose a battle with a friend
// close by (same map, within RESCUE_RANGE tiles, free to act) and you stay
// where you are while they're asked to help: a REVIVE from their bag brings
// your first Moemon back at half HP, a MAX REVIVE at full HP, or they can pay
// to patch up your whole team (the price goes by how many Moemon you have and
// how much HP they're missing, see PatchUpPrice). If they say no, don't answer
// in time, or you give up (B), you black out as usual.
//
// battle_setup.c asks MP_Rescue_TryStart() before CB2_WhiteOut. The downed
// player waits on the field with a small co-op window; the friend gets a
// prompt (MPR_RESCUE).
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "battle.h"
#include "event_data.h"
#include "constants/flags.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "item.h"
#include "main.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "field_weather.h"
#include "constants/items.h"
#include "constants/songs.h"

#define RESCUE_RANGE 10
#define RESCUE_PER_MON 150    // patch-up price: per Moemon
#define RESCUE_PER_HP 4       // ... plus per missing HP, then x(1 + badges / 2)
#define RESCUE_MAX_PRICE 60000
#define RESCUE_NO_PATCH 0xFFFF   // in a cave or dungeon: no paid patch-up, only REVIVEs
#define RESCUE_WAIT (15 * 60)

enum { RS_ASK, RS_YES_REVIVE, RS_YES_MAX_REVIVE, RS_YES_MONEY, RS_NO, RS_BUSY, RS_SELF };   // RS_SELF: used their own REVIVE

static EWRAM_DATA struct
{
    s8 friendId;          // downed: who was asked
    u16 timer;
    u8 answer;            // 0xFF = waiting
    bool8 done;           // showing the result
    u8 helperId;          // helper: who asked
    u16 cursor;
    u8 options[4];
    u8 optionCount;
    u16 price;            // patch-up price (both sides), RESCUE_NO_PATCH in a dungeon
    u8 monCount;          // Moemon on the downed team
    bool8 selfFull;       // downed: used their own MAX REVIVE
} sRescue = {0};

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

static void PlayerName(u8 id, char *out)
{
    u8 name[PLAYER_NAME_LENGTH + 1];
    if (id >= MP_MAX_PLAYERS)
    {
        strcpy(out, "?");
        return;
    }
    StringCopyN(name, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    name[PLAYER_NAME_LENGTH] = EOS;
    MP_GameToAscii(out, name, PLAYER_NAME_LENGTH + 1);
}

static void Send(u8 to, u8 kind)
{
    u8 buf[2] = {MPR_RESCUE, kind};
    MP_SendRelay(to, buf, sizeof(buf));
}

static int Badges(void)
{
    int i, n = 0;
    for (i = 0; i < 8; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) ? 1 : 0;
    return n;
}

// What it costs to patch up the whole team: RESCUE_PER_MON for each Moemon plus
// RESCUE_PER_HP for each HP it's missing, times 1 + badges / 2. Dear on purpose:
// REVIVEs bought before a trip are the cheap way back up, and down in a cave or
// dungeon they're the only way (MP_InDungeon).
static u16 PatchUpPrice(u8 *count)
{
    int i, price = 0;
    *count = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            break;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        (*count)++;
        price += RESCUE_PER_MON + RESCUE_PER_HP * (GetMonData(mon, MON_DATA_MAX_HP) - GetMonData(mon, MON_DATA_HP));
    }
    if (MP_InDungeon())
        return RESCUE_NO_PATCH;
    price = price * (2 + Badges()) / 2;
    return min(price, RESCUE_MAX_PRICE);
}

// The nearest friend in range who's free to answer (-1 = nobody)
static s8 FindHelper(void)
{
    struct ObjectEvent *me;
    s8 best = -1;
    int bestD = RESCUE_RANGE + 1, i;
    if (!MP_IsConnected() || gMpLocalId < 0 || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return -1;
    me = &gObjectEvents[gPlayerAvatar.objectEventId];
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct MpPlayer *p = &gMpPlayers[i];
        int d;
        if (i == gMpLocalId || !p->present || !(p->state.flags & MPF_FREE))
            continue;
        if (p->state.mapGroup != gSaveBlock1Ptr->location.mapGroup || p->state.mapNum != gSaveBlock1Ptr->location.mapNum)
            continue;
        d = max(abs(p->state.x - me->currentCoords.x), abs(p->state.y - me->currentCoords.y));
        if (d < bestD)
        {
            bestD = d;
            best = i;
        }
    }
    return best;
}

// ---------------------------------------------------------------------------
// The downed player

static void DrawWaiting(void)
{
    char who[16], line[64];
    PlayerName(sRescue.friendId, who);
    MpUi_Begin("DOWN!");
    MpUi_TextAscii(0, "Your team can't fight on...");
    if (!sRescue.done)
    {
        snprintf(line, sizeof(line), "%s is close by!", who);
        MpUi_TextAscii(1, line);
        if (sRescue.price == RESCUE_NO_PATCH)
            snprintf(line, sizeof(line), "This deep, only a REVIVE can help.");
        else
            snprintf(line, sizeof(line), "Patching up your team costs $%d.", sRescue.price);
        MpUi_TextAscii(2, line);
        snprintf(line, sizeof(line), "Waiting for %s to help (%d)", who, (RESCUE_WAIT - sRescue.timer) / 60 + 1);
        MpUi_TextAscii(3, line);
        MpUi_End();
        if (CheckBagHasItem(ITEM_REVIVE, 1))
            snprintf(line, sizeof(line), "{A} USE YOUR REVIVE (x%d)   {B} GIVE UP", (int)CountTotalItemQuantityInBag(ITEM_REVIVE));
        else if (CheckBagHasItem(ITEM_MAX_REVIVE, 1))
            snprintf(line, sizeof(line), "{A} USE YOUR MAX REVIVE (x%d)   {B} GIVE UP", (int)CountTotalItemQuantityInBag(ITEM_MAX_REVIVE));
        else
            snprintf(line, sizeof(line), "{B} GIVE UP");
        MpUi_HelpAscii(line);
        return;
    }
    switch (sRescue.answer)
    {
    case RS_YES_REVIVE:
    case RS_YES_MAX_REVIVE:
        snprintf(line, sizeof(line), "%s used a %s!", who, sRescue.answer == RS_YES_REVIVE ? "REVIVE" : "MAX REVIVE");
        MpUi_TextAscii(2, line);
        MpUi_TextAscii(3, sRescue.answer == RS_YES_REVIVE ? "Your lead MOeMON is back at half HP!"
                                                           : "Your lead MOeMON is back at full HP!");
        break;
    case RS_SELF:
        snprintf(line, sizeof(line), "%s kept watch while you used", who);
        MpUi_TextAscii(1, line);
        MpUi_TextAscii(2, sRescue.selfFull ? "your MAX REVIVE!" : "your REVIVE!");
        MpUi_TextAscii(3, sRescue.selfFull ? "Your lead MOeMON is back at full HP!"
                                           : "Your lead MOeMON is back at half HP!");
        break;
    case RS_YES_MONEY:
        snprintf(line, sizeof(line), "%s paid $%d to patch you up!", who, sRescue.price);
        MpUi_TextAscii(2, line);
        MpUi_TextAscii(3, "Your whole team is healed!");
        break;
    case RS_BUSY:
        snprintf(line, sizeof(line), "%s is busy right now.", who);
        MpUi_TextAscii(2, line);
        break;
    default:
        snprintf(line, sizeof(line), "%s couldn't help this time.", who);
        MpUi_TextAscii(2, line);
        break;
    }
    MpUi_End();
    MpUi_HelpAscii("{A} OK");
}

// Lead Moemon (the first one that fainted, from the top) back up
static void ReviveLead(bool8 full)
{
    int i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            break;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (GetMonData(mon, MON_DATA_HP) == 0)
        {
            u16 max = GetMonData(mon, MON_DATA_MAX_HP);
            u16 hp = full ? max : max(1, max / 2);
            u32 status = 0;
            SetMonData(mon, MON_DATA_HP, &hp);
            SetMonData(mon, MON_DATA_STATUS, &status);
            return;
        }
    }
}

static void Task_BlackOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        gMain.state = 0;
        SetMainCallback2(CB2_WhiteOut);
    }
}

static void Task_Down(u8 taskId)
{
    if (!sRescue.done)
    {
        if (sRescue.answer == 0xFF)
        {
            if (++sRescue.timer >= RESCUE_WAIT)
                sRescue.answer = RS_NO;
            else if (JOY_NEW(B_BUTTON))
            {
                PlaySE(SE_SELECT);
                sRescue.answer = RS_NO;
                Send(sRescue.friendId, RS_NO);   // never mind
            }
            else if (JOY_NEW(A_BUTTON) && (CheckBagHasItem(ITEM_REVIVE, 1) || CheckBagHasItem(ITEM_MAX_REVIVE, 1)))
            {
                // Your own REVIVE (the cheaper one first) while your friend keeps watch
                u8 buf[3] = {MPR_RESCUE, RS_SELF, 0};
                sRescue.selfFull = !CheckBagHasItem(ITEM_REVIVE, 1);
                RemoveBagItem(sRescue.selfFull ? ITEM_MAX_REVIVE : ITEM_REVIVE, 1);
                buf[2] = sRescue.selfFull;
                MP_SendRelay(sRescue.friendId, buf, sizeof(buf));
                sRescue.answer = RS_SELF;
            }
            else if (sRescue.timer % 60 == 0)
                DrawWaiting();
        }
        if (sRescue.answer != 0xFF)
        {
            sRescue.done = TRUE;
            if (sRescue.answer == RS_YES_MONEY)
            {
                HealPlayerParty();
                PlaySE(SE_USE_ITEM);
            }
            else if (sRescue.answer == RS_YES_REVIVE || sRescue.answer == RS_YES_MAX_REVIVE || sRescue.answer == RS_SELF)
            {
                ReviveLead(sRescue.answer == RS_YES_MAX_REVIVE || (sRescue.answer == RS_SELF && sRescue.selfFull));
                PlaySE(SE_USE_ITEM);
            }
            else
                PlaySE(SE_FAILURE);
            DrawWaiting();
        }
        return;
    }
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        DestroyTask(taskId);
        if ((sRescue.answer >= RS_YES_REVIVE && sRescue.answer <= RS_YES_MONEY) || sRescue.answer == RS_SELF)
        {
            MpUi_Close();
            sRescue.friendId = -1;
            return;
        }
        // Black out after all
        MpUi_Close();
        LockPlayerFieldControls();
        sRescue.friendId = -1;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        CreateTask(Task_BlackOut, 10);
    }
}

static void Task_OpenDown(u8 taskId)
{
    if (!gPaletteFade.active && IsWeatherNotFadingIn())
    {
        DestroyTask(taskId);
        MpUi_Open();
        DrawWaiting();
        CreateTask(Task_Down, 80);
    }
}

static void FieldCB_Down(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_OpenDown, 10);
}

// After a lost battle (battle_setup.c): TRUE = a friend is being asked
bool8 MP_Rescue_TryStart(void)
{
    s8 id;
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_SAFARI))
        return FALSE;
    if (!MP_IsConnected() || MP_Coop_IsBusy() || gMpCoopWild)
        return FALSE;
    id = FindHelper();
    if (id < 0)
        return FALSE;
    sRescue.friendId = id;
    sRescue.timer = 0;
    sRescue.answer = 0xFF;
    sRescue.done = FALSE;
    sRescue.selfFull = FALSE;
    sRescue.price = PatchUpPrice(&sRescue.monCount);
    {
        u8 buf[5] = {MPR_RESCUE, RS_ASK, sRescue.price & 0xFF, sRescue.price >> 8, sRescue.monCount};
        MP_SendRelay(id, buf, sizeof(buf));
    }
    printf("[mp] rescue: asking player %d (patch-up $%d for %d)\n", id, sRescue.price, sRescue.monCount);
    ScriptContext_Init();   // a trainer's script doesn't go on as if you'd won
    gFieldCallback = FieldCB_Down;
    SetMainCallback2(CB2_ReturnToField);
    return TRUE;
}

// ---------------------------------------------------------------------------
// The friend who's asked

static void DrawPrompt(void)
{
    char who[16], line[64];
    u8 i;
    static const char *const sLabels[] = {
        [RS_YES_REVIVE] = "USE A REVIVE",
        [RS_YES_MAX_REVIVE] = "USE A MAX REVIVE",
        [RS_YES_MONEY] = "PAY TO HEAL THEIR TEAM",
        [RS_NO] = "NOT THIS TIME",
    };
    PlayerName(sRescue.helperId, who);
    snprintf(line, sizeof(line), "HELP %s?", who);
    MpUi_Begin(line);
    for (i = 0; i < sRescue.optionCount; i++)
    {
        char right[16] = "";
        u8 o = sRescue.options[i];
        if (o == RS_YES_REVIVE)
            snprintf(right, sizeof(right), "x%d", (int)CountTotalItemQuantityInBag(ITEM_REVIVE));
        else if (o == RS_YES_MAX_REVIVE)
            snprintf(right, sizeof(right), "x%d", (int)CountTotalItemQuantityInBag(ITEM_MAX_REVIVE));
        else if (o == RS_YES_MONEY)
            snprintf(right, sizeof(right), "$%d", sRescue.price);
        MpUi_RowAscii(i, i == sRescue.cursor, sLabels[o], right);
    }
    MpUi_End();
    if (sRescue.price == RESCUE_NO_PATCH)
        snprintf(line, sizeof(line), "%s's team is down! This deep, only a REVIVE helps.", who);
    else if (sRescue.monCount > 1)
        snprintf(line, sizeof(line), "%s's team of %d is down close by!", who, sRescue.monCount);
    else
        snprintf(line, sizeof(line), "%s's team is down close by!", who);
    MpUi_HelpAscii(line);
}

static void Task_Help(u8 taskId)
{
    if (MpUi_MoveCursor(&sRescue.cursor, sRescue.optionCount))
    {
        DrawPrompt();
        return;
    }
    if (JOY_NEW(B_BUTTON) || JOY_NEW(A_BUTTON))
    {
        u8 pick = JOY_NEW(B_BUTTON) ? RS_NO : sRescue.options[sRescue.cursor];
        PlaySE(SE_SELECT);
        if (pick == RS_YES_REVIVE && !RemoveBagItem(ITEM_REVIVE, 1))
            pick = RS_NO;
        else if (pick == RS_YES_MAX_REVIVE && !RemoveBagItem(ITEM_MAX_REVIVE, 1))
            pick = RS_NO;
        else if (pick == RS_YES_MONEY)
        {
            if (IsEnoughMoney(&gSaveBlock1Ptr->money, sRescue.price))
                RemoveMoney(&gSaveBlock1Ptr->money, sRescue.price);
            else
                pick = RS_NO;
        }
        Send(sRescue.helperId, pick);
        DestroyTask(taskId);
        MpUi_Close();
        if (pick != RS_NO)
        {
            char who[16], note[80];
            PlayerName(sRescue.helperId, who);
            snprintf(note, sizeof(note), "You helped %s back up!", who);
            MP_ShowNotice(note);
        }
    }
}

void MP_Rescue_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (from >= MP_MAX_PLAYERS || len < 2)
        return;
    if (data[1] == RS_ASK)
    {
        // Only when free to answer right away
        if (!MP_IsLocalFree() || MpUi_IsOpen() || FuncIsActiveTask(Task_Help))
        {
            Send(from, RS_BUSY);
            return;
        }
        sRescue.helperId = from;
        sRescue.cursor = 0;
        sRescue.optionCount = 0;
        sRescue.price = len >= 5 ? (data[2] | (data[3] << 8)) : RESCUE_MAX_PRICE;
        if (sRescue.price != RESCUE_NO_PATCH)
            sRescue.price = min(sRescue.price, RESCUE_MAX_PRICE);
        sRescue.monCount = len >= 5 ? data[4] : 1;
        if (CheckBagHasItem(ITEM_REVIVE, 1))
            sRescue.options[sRescue.optionCount++] = RS_YES_REVIVE;
        if (CheckBagHasItem(ITEM_MAX_REVIVE, 1))
            sRescue.options[sRescue.optionCount++] = RS_YES_MAX_REVIVE;
        if (sRescue.price != RESCUE_NO_PATCH && IsEnoughMoney(&gSaveBlock1Ptr->money, sRescue.price))
            sRescue.options[sRescue.optionCount++] = RS_YES_MONEY;
        sRescue.options[sRescue.optionCount++] = RS_NO;
        PlaySE(SE_PIN);
        MpUi_Open();
        DrawPrompt();
        CreateTask(Task_Help, 80);
        return;
    }
    if ((data[1] == RS_NO || data[1] == RS_SELF) && FuncIsActiveTask(Task_Help) && from == sRescue.helperId)
    {
        // They gave up waiting, or got back up with their own REVIVE
        u8 t = FindTaskIdByFunc(Task_Help);
        DestroyTask(t);
        MpUi_Close();
        if (data[1] == RS_SELF)
        {
            char who[16], note[80];
            u8 text[96];
            PlayerName(from, who);
            snprintf(note, sizeof(note), "%s used their own %s!", who, len >= 3 && data[2] ? "MAX REVIVE" : "REVIVE");
            MP_AsciiToGame(text, note, sizeof(text) - 1);
            MP_Toast(text);
        }
        return;
    }
    // An answer to our call
    if (from == sRescue.friendId && sRescue.answer == 0xFF && data[1] >= RS_YES_REVIVE && data[1] <= RS_BUSY)
        sRescue.answer = data[1];
}

// Harness conditions "down" (waiting for help or showing the answer) and "helpask"
bool8 MP_Rescue_Down(void)
{
    return FuncIsActiveTask(Task_Down);
}

bool8 MP_Rescue_Asked(void)
{
    return FuncIsActiveTask(Task_Help);
}
