// Moemon Kai: UNOVA's BP exchanges (the GEAR STATION's two counters: Black's shops 254, items, and 253, TMs; ov021
// scrcmd_proc_fld.c), their lists from UNOVA's scripts pack (u3d_vm.c U3D_Vm_BpList). One thing at a time for its BP,
// in Black's own words (story bank 423: the price, "You don't have enough BP.", "Here you are! Thank you!"); the BP
// are the game's own (the BATTLE FRONTIER's, the BATTLE SUBWAY's: u3d_subway.c). A scrolling list as the game's
// exchange corners have, the BP in a window above it.
#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "field_message_box.h"
#include "item.h"
#include "list_menu.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle_frontier.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "u3d.h"

void MPNet_Log(const char *fmt, ...);
const u8 *U3D_Vm_BpList(u8 which, u32 *count);   // (u16 Black's item, pad, s32 BP each)
u16 U3D_Vm_Item(u16 black);

#define BANK_SHOP (288 + 423)   // (story bank 423: the shops' words; the system banks come first)
#define MAX_ROWS 48

static struct
{
    u16 items[MAX_ROWS];
    u32 prices[MAX_ROWS];
    u8 names[MAX_ROWS][24];
    struct ListMenuItem rows[MAX_ROWS + 1];
    u8 count, listWindow, bpWindow, listTask, chosen;
    u16 scroll, row;
} *sS;

static const u8 sPriceColors[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void Task_BpShop(u8 taskId);

static void Line(u16 msg)   // (Black's line MSG, its word slots filled, in the field's message box)
{
    StringExpandPlaceholders(gStringVar4, U3D_Text(BANK_SHOP, msg));
    ShowFieldMessage(gStringVar4);
}

static void PrintBp(void)
{
    ConvertIntToDecimalStringN(gUnovaStrVar[0], gSaveBlock2Ptr->frontier.battlePoints, STR_CONV_MODE_LEFT_ALIGN, 4);
    StringExpandPlaceholders(gStringVar4, U3D_Text(BANK_SHOP, 22));   // ("{N} BP")
    FillWindowPixelBuffer(sS->bpWindow, PIXEL_FILL(1));
    AddTextPrinterParameterized(sS->bpWindow, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sS->bpWindow, COPYWIN_FULL);
}

static void PrintPrice(u8 windowId, u32 id, u8 y)   // (each row's price, at the right)
{
    u8 buf[24];
    if (id >= sS->count)
        return;
    ConvertIntToDecimalStringN(gUnovaStrVar[1], sS->prices[id], STR_CONV_MODE_LEFT_ALIGN, 5);
    StringExpandPlaceholders(buf, U3D_Text(BANK_SHOP, 13));   // ("{N} BP")
    AddTextPrinterParameterized4(windowId, FONT_NARROW, GetWindowAttribute(windowId, WINDOW_WIDTH) * 8 - 4
                                 - GetStringWidth(FONT_NARROW, buf, 0), y, 0, 0, sPriceColors, TEXT_SKIP_DRAW, buf);
}

static void OpenList(void)
{
    struct ListMenuTemplate t = {0};
    struct WindowTemplate w = CreateWindowTemplate(0, 13, 1, 16, 12, 0xF, 0x64);
    sS->listWindow = AddWindow(&w);
    SetStandardWindowBorderStyle(sS->listWindow, FALSE);
    t.items = sS->rows;
    t.itemPrintFunc = PrintPrice;
    t.totalItems = sS->count + 1;
    t.maxShowed = 6;
    t.windowId = sS->listWindow;
    t.item_X = 8;
    t.upText_Y = 1;
    t.cursorPal = 2;
    t.fillValue = 1;
    t.cursorShadowPal = 3;
    t.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
    t.fontId = FONT_NORMAL;
    t.cursorKind = CURSOR_BLACK_ARROW;
    sS->listTask = ListMenuInit(&t, sS->scroll, sS->row);
    ScheduleBgCopyTilemapToVram(0);
}

static void CloseList(void)
{
    DestroyListMenuTask(sS->listTask, &sS->scroll, &sS->row);
    ClearStdWindowAndFrameToTransparent(sS->listWindow, TRUE);
    RemoveWindow(sS->listWindow);
}

// special U3D_VmOpenScreen's: WHICH 0 the items, 1 the TMs; the script goes on when it closes
void U3D_BpShop_Open(u8 which)
{
    u32 count, i;
    const u8 *list = U3D_Vm_BpList(which, &count);
    struct WindowTemplate w = CreateWindowTemplate(0, 1, 1, 8, 2, 0xF, 0x14);
    sS = AllocZeroed(sizeof(*sS));
    for (i = 0; list != NULL && i < count && sS->count < MAX_ROWS; i++)
    {
        const u8 *e = list + 8 * i;
        u16 item = U3D_Vm_Item(e[0] | e[1] << 8);
        if (item == ITEM_NONE)
            continue;
        sS->items[sS->count] = item;
        sS->prices[sS->count] = e[4] | e[5] << 8 | e[6] << 16 | (u32)e[7] << 24;
        CopyItemName(item, sS->names[sS->count]);
        sS->rows[sS->count].name = sS->names[sS->count];
        sS->rows[sS->count].id = sS->count;
        sS->count++;
    }
    sS->rows[sS->count].name = U3D_Text(BANK_SHOP, 11);   // ("Cancel")
    sS->rows[sS->count].id = LIST_CANCEL;
    MPNet_Log("u3d: BP exchange %u: %u things", which, sS->count);
    LockPlayerFieldControls();
    sS->bpWindow = AddWindow(&w);
    SetStandardWindowBorderStyle(sS->bpWindow, FALSE);
    PrintBp();
    OpenList();
    CreateTask(Task_BpShop, 8);
}

#define tState data[0]

static void Task_BpShop(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    switch (tState)
    {
    case 0:   // the list
    {
        s32 input = ListMenu_ProcessInput(sS->listTask);
        if (input == LIST_NOTHING_CHOSEN)
            break;
        PlaySE(SE_SELECT);
        if (input == LIST_CANCEL || input >= sS->count)
        {
            CloseList();
            ClearStdWindowAndFrameToTransparent(sS->bpWindow, TRUE);
            RemoveWindow(sS->bpWindow);
            HideFieldMessageBox();
            FREE_AND_SET_NULL(sS);
            DestroyTask(taskId);
            ScriptContext_Enable();
            break;
        }
        sS->chosen = input;
        CopyItemName(sS->items[input], gUnovaStrVar[0]);
        ConvertIntToDecimalStringN(gUnovaStrVar[2], sS->prices[input], STR_CONV_MODE_LEFT_ALIGN, 5);
        Line(6);   // ("{item} will cost {N} BP. Is that OK?")
        tState = 1;
        break;
    }
    case 1:
        if (IsTextPrinterActive(0))
            break;
        DisplayYesNoMenuDefaultYes();
        tState = 2;
        break;
    case 2:
    {
        s8 answer = Menu_ProcessInputNoWrapClearOnChoose();
        u16 item = sS->items[sS->chosen];
        u32 price = sS->prices[sS->chosen];
        if (answer == MENU_NOTHING_CHOSEN)
            break;
        if (answer != 0)   // (no, or B)
        {
            HideFieldMessageBox();
            tState = 0;
            break;
        }
        if (gSaveBlock2Ptr->frontier.battlePoints < price)
            Line(16);   // ("You don't have enough BP.")
        else if (!AddBagItem(item, 1))
            Line(10);   // ("You can't take any more of this item.")
        else
        {
            gSaveBlock2Ptr->frontier.battlePoints -= price;
            PrintBp();
            PlaySE(SE_SHOP);
            CopyItemName(item, gUnovaStrVar[0]);
            StringCopy(gUnovaStrVar[1], gPocketNamesStringsTable[GetItemPocket(item) - 1]);
            Line(9);   // ("Here you are! Thank you! You put away the {item} in the {pocket}.")
            MPNet_Log("u3d: BP exchange: item %u for %u BP", item, price);
        }
        tState = 3;
        break;
    }
    case 3:
        if (!IsTextPrinterActive(0) && JOY_NEW(A_BUTTON | B_BUTTON))
        {
            HideFieldMessageBox();
            tState = 0;
        }
        break;
    }
}
