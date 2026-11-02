// Moemon Kai: MOM's money in JOHTO, as Crystal's BANK OF MOM (pokecrystal engine/events/mom.asm, engine/battle/core.asm):
// after the player leaves with a POKéMON she offers to save money; while she does, a quarter of every battle's winnings
// in JOHTO and KANTO goes to her ("Sent some to MOM!"); at home she asks "Or is this about your money?": GET / SAVE /
// CHANGE (start or stop saving) / CANCEL, the amount entered digit by digit (UP / DOWN change it, LEFT / RIGHT move,
// A takes it, B cancels) beside SAVED and HELD. Her words are Crystal's (data/maps/Johto_PlayersHouse1F/scripts.inc).
// Kept in JOHTO's vars: the savings in 0x52E8 (low) / 0x52E9 (high), the state in 0x52EA (bit 0 asked, bit 1 saving).
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "money.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"
#include "mp_region.h"

#define VAR_MOM_MONEY_LO 0x52E8
#define VAR_MOM_MONEY_HI 0x52E9
#define VAR_MOM_STATE    0x52EA
#define MOM_ASKED  (1 << 0)
#define MOM_SAVING (1 << 1)
#define MOM_MAX    999999
#define DIGITS     6

static u32 MomMoney(void)
{
    return VarGet(VAR_MOM_MONEY_LO) | ((u32)VarGet(VAR_MOM_MONEY_HI) << 16);
}

static void SetMomMoney(u32 v)
{
    if (v > MOM_MAX)
        v = MOM_MAX;
    VarSet(VAR_MOM_MONEY_LO, v & 0xFFFF);
    VarSet(VAR_MOM_MONEY_HI, v >> 16);
}

// The battle's winnings (battle_script_commands.c Cmd_getmoneyreward): MOM's share while she saves, in JOHTO and
// KANTO, until she has the most there can be (the rest is the player's)
u32 MP_MomShare(u32 reward)
{
    u32 share, room;
    u8 region = MP_Region_Current();
    if (!(VarGet(VAR_MOM_STATE) & MOM_SAVING) || (region != REGION_JOHTO && region != REGION_KANTO))
        return 0;
    room = MOM_MAX - MomMoney();
    share = reward / 4;
    if (share > room)
        share = room;
    SetMomMoney(MomMoney() + share);
    return share;
}

// VAR_RESULT: 0 never asked, 1 asked (not saving), 2 saving
u16 QzMomBankState(void)
{
    u16 s = VarGet(VAR_MOM_STATE);
    return gSpecialVar_Result = !(s & MOM_ASKED) ? 0 : (s & MOM_SAVING) ? 2 : 1;
}

// VAR_0x8004: 1 she saves, 0 she doesn't (and she has asked)
u16 QzMomBankSetSaving(void)
{
    VarSet(VAR_MOM_STATE, MOM_ASKED | (gSpecialVar_0x8004 ? MOM_SAVING : 0));
    return 0;
}

// The amount window: VAR_0x8004 0 SAVE, 1 GET. VAR_RESULT after: 0 cancelled, 1 done, 2 she hasn't saved that much,
// 3 the wallet can't take it, 4 the player hasn't that much, 5 she can't save that much
static EWRAM_DATA struct
{
    u8 window, cursor, take;
    u32 amount;
} sBank = {0};

static const u8 sText_Saved[] = _("SAVED");
static const u8 sText_Held[] = _("HELD");
static const u8 sText_Save[] = _("SAVE");
static const u8 sText_Get[] = _("GET");
static const u8 sText_Yen[] = _("¥");

static void PrintAmountLine(u8 y, const u8 *label, u32 value, bool8 cursor)
{
    u8 buf[16], *p;
    AddTextPrinterParameterized(sBank.window, FONT_NORMAL, label, 0, y, TEXT_SKIP_DRAW, NULL);
    p = StringCopy(buf, sText_Yen);
    ConvertIntToDecimalStringN(p, value, STR_CONV_MODE_LEADING_ZEROS, DIGITS);
    AddTextPrinterParameterized(sBank.window, FONT_NORMAL, buf, 44, y, TEXT_SKIP_DRAW, NULL);
    if (cursor)   // a bar under the digit being changed
        FillWindowPixelRect(sBank.window, PIXEL_FILL(2), 44 + 6 + sBank.cursor * 6, y + 13, 6, 1);
}

static void DrawBank(void)
{
    FillWindowPixelBuffer(sBank.window, PIXEL_FILL(1));
    PrintAmountLine(0, sText_Saved, MomMoney(), FALSE);
    PrintAmountLine(16, sText_Held, GetMoney(&gSaveBlock1Ptr->money), FALSE);
    PrintAmountLine(32, sBank.take ? sText_Get : sText_Save, sBank.amount, TRUE);
    CopyWindowToVram(sBank.window, COPYWIN_FULL);
}

static u32 DigitPow(u8 cursor)
{
    u32 p = 1;
    u8 i;
    for (i = 0; i < DIGITS - 1 - cursor; i++)
        p *= 10;
    return p;
}

static void Finish(u8 taskId, u16 result)
{
    ClearStdWindowAndFrameToTransparent(sBank.window, TRUE);
    RemoveWindow(sBank.window);
    gSpecialVar_Result = result;
    DestroyTask(taskId);
    ScriptContext_Enable();
}

static void Task_Bank(u8 taskId)
{
    u32 pow = DigitPow(sBank.cursor), digit = (sBank.amount / pow) % 10;
    if (JOY_NEW(DPAD_UP))
    {
        sBank.amount = digit == 9 ? sBank.amount - 9 * pow : sBank.amount + pow;
        PlaySE(SE_SELECT);
        DrawBank();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        sBank.amount = digit == 0 ? sBank.amount + 9 * pow : sBank.amount - pow;
        PlaySE(SE_SELECT);
        DrawBank();
    }
    else if (JOY_NEW(DPAD_LEFT) && sBank.cursor > 0)
    {
        sBank.cursor--;
        PlaySE(SE_SELECT);
        DrawBank();
    }
    else if (JOY_NEW(DPAD_RIGHT) && sBank.cursor < DIGITS - 1)
    {
        sBank.cursor++;
        PlaySE(SE_SELECT);
        DrawBank();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Finish(taskId, 0);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u32 held = GetMoney(&gSaveBlock1Ptr->money), saved = MomMoney();
        PlaySE(SE_SELECT);
        if (sBank.amount == 0)
            Finish(taskId, 0);
        else if (sBank.take && sBank.amount > saved)
            Finish(taskId, 2);
        else if (sBank.take && held + sBank.amount > MOM_MAX)
            Finish(taskId, 3);
        else if (!sBank.take && sBank.amount > held)
            Finish(taskId, 4);
        else if (!sBank.take && saved + sBank.amount > MOM_MAX)
            Finish(taskId, 5);
        else
        {
            if (sBank.take)
            {
                SetMomMoney(saved - sBank.amount);
                AddMoney(&gSaveBlock1Ptr->money, sBank.amount);
            }
            else
            {
                SetMomMoney(saved + sBank.amount);
                RemoveMoney(&gSaveBlock1Ptr->money, sBank.amount);
            }
            PlaySE(SE_SHOP);
            Finish(taskId, 1);
        }
    }
}

u16 QzMomBankAmount(void)
{
    struct WindowTemplate t = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 13, .height = 6,
                               .paletteNum = 15, .baseBlock = 100};   // (where script menus go: clear of the message box)
    sBank.window = AddWindow(&t);
    sBank.take = gSpecialVar_0x8004 != 0;
    sBank.cursor = DIGITS - 1;
    sBank.amount = 0;
    DrawStdWindowFrame(sBank.window, FALSE);
    PutWindowTilemap(sBank.window);
    DrawBank();
    CreateTask(Task_Bank, 80);
    return 0;
}

// Tests (mp mombank [SAVINGS]): MOM's savings and whether she saves
void MP_MomBank_Debug(const char *arg)
{
    if (arg[0])
        SetMomMoney(atoi(arg));
    printf("[test] mom bank: saved %u, state %u, held %u\n", (unsigned)MomMoney(), VarGet(VAR_MOM_STATE),
           (unsigned)GetMoney(&gSaveBlock1Ptr->money));
}

// The POKéGEAR's call to MOM (mp_pokegear.c): her savings and whether she saves
bool8 MP_MomBank_Saving(void)
{
    return (VarGet(VAR_MOM_STATE) & MOM_SAVING) != 0;
}

bool8 MP_MomBank_Asked(void)
{
    return (VarGet(VAR_MOM_STATE) & MOM_ASKED) != 0;
}

void MP_MomBank_SetSaving(bool8 on)
{
    VarSet(VAR_MOM_STATE, MOM_ASKED | (on ? MOM_SAVING : 0));
}

u32 MP_MomBank_Money(void)
{
    return MomMoney();
}
