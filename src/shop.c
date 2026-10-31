#include <stdio.h>
#include "global.h"
#include "mp_region.h"
#include "event_data.h"
#include "battle_gimmick.h"
#include "mp.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "decoration.h"
#include "decoration_inventory.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "item_menu.h"
#include "list_menu.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "menu_helpers.h"
#include "money.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "scanline_effect.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "text_window.h"
#include "tv.h"
#include "constants/decorations.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/metatile_behaviors.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define TAG_SCROLL_ARROW   2100
#define TAG_ITEM_ICON_BASE 2110

#define MAX_ITEMS_SHOWN 8
#define SHOP_MENU_PALETTE_ID 12

enum {
    WIN_BUY_SELL_QUIT,
    WIN_BUY_QUIT,
};

enum {
    WIN_MONEY,
    WIN_ITEM_LIST,
    WIN_ITEM_DESCRIPTION,
    WIN_QUANTITY_IN_BAG,
    WIN_QUANTITY_PRICE,
    WIN_MESSAGE,
};

enum {
    COLORID_NORMAL,      // Item descriptions, quantity in bag, and quantity/price
    COLORID_ITEM_LIST,   // The text in the item list, and the cursor normally
    COLORID_GRAY_CURSOR, // When the cursor has selected an item to purchase
};

enum {
    MART_TYPE_NORMAL,
    MART_TYPE_DECOR,
    MART_TYPE_DECOR2,
};

// shop view window NPC info enum
enum
{
    OBJ_EVENT_ID,
    X_COORD,
    Y_COORD,
    ANIM_NUM,
    LAYER_TYPE
};

struct MartInfo
{
    void (*callback)(void);
    const struct MenuAction *menuActions;
    const u16 *itemList;
    u16 itemCount;
    u8 windowId;
    u8 martType;
};

struct ShopData
{
    u16 tilemapBuffers[4][0x400];
    u32 totalCost;
    u16 itemsShowed;
    u16 selectedRow;
    u16 scrollOffset;
    u16 maxQuantity;   // Moemon Co-op: up to MP_BAG_ITEM_CAPACITY (999; a u8 wrapped it round)
    u8 scrollIndicatorsTaskId;
    u8 iconSlot;
    u8 itemSpriteIds[2];
    s16 viewportObjects[OBJECT_EVENTS_COUNT][5];
};

static EWRAM_DATA struct MartInfo sMartInfo = {0};
static EWRAM_DATA struct ShopData *sShopData = NULL;
static EWRAM_DATA struct ListMenuItem *sListMenuItems = NULL;
static EWRAM_DATA u8 (*sItemNames)[ITEM_NAME_LENGTH + 2] = {0};
static EWRAM_DATA u8 sPurchaseHistoryId = 0;
EWRAM_DATA struct ItemSlot gMartPurchaseHistory[SMARTSHOPPER_NUM_ITEMS] = {0};

static void Task_ShopMenu(u8 taskId);
static void Task_HandleShopMenuQuit(u8 taskId);
static void CB2_InitBuyMenu(void);
static void Task_GoToBuyOrSellMenu(u8 taskId);
static void MapPostLoadHook_ReturnToShopMenu(void);
static void Task_ReturnToShopMenu(u8 taskId);
static void ShowShopMenuAfterExitingBuyOrSellMenu(u8 taskId);
static void BuyMenuDrawGraphics(void);
static void BuyMenuAddScrollIndicatorArrows(void);
static void Task_BuyMenu(u8 taskId);
static void BuyMenuBuildListMenuTemplate(void);
static void BuyMenuInitBgs(void);
static void BuyMenuInitWindows(void);
static void BuyMenuDecompressBgGraphics(void);
static void BuyMenuSetListEntry(struct ListMenuItem *, u16, u8 *);
static void BuyMenuAddItemIcon(u16, u8);
static void BuyMenuRemoveItemIcon(u16, u8);
static void BuyMenuPrint(u8 windowId, const u8 *text, u8 x, u8 y, s8 speed, u8 colorSet);
static void BuyMenuDrawMapGraphics(void);
static void BuyMenuCopyMenuBgToBg1TilemapBuffer(void);
static void BuyMenuCollectObjectEventData(void);
static void BuyMenuDrawObjectEvents(void);
static void BuyMenuDrawMapBg(void);
static bool8 BuyMenuCheckForOverlapWithMenuBg(int, int);
static void BuyMenuDrawMapMetatile(s16, s16, const u16 *, u8);
static void BuyMenuDrawMapMetatileLayer(u16 *dest, s16 offset1, s16 offset2, const u16 *src);
static bool8 BuyMenuCheckIfObjectEventOverlapsMenuBg(s16 *);
static void ExitBuyMenu(u8 taskId);
static void Task_ExitBuyMenu(u8 taskId);
static void BuyMenuTryMakePurchase(u8 taskId);
static void BuyMenuReturnToItemList(u8 taskId);
static void Task_BuyHowManyDialogueInit(u8 taskId);
static void BuyMenuConfirmPurchase(u8 taskId);
static void BuyMenuPrintItemQuantityAndPrice(u8 taskId);
static void Task_BuyHowManyDialogueHandleInput(u8 taskId);
static void BuyMenuSubtractMoney(u8 taskId);
static void RecordItemPurchase(u8 taskId);
static void Task_ReturnToItemListAfterItemPurchase(u8 taskId);
static void Task_ReturnToItemListAfterDecorationPurchase(u8 taskId);
static void Task_HandleShopMenuBuy(u8 taskId);
static void Task_HandleShopMenuSell(u8 taskId);
static void BuyMenuPrintItemDescriptionAndShowItemIcon(s32 item, bool8 onInit, struct ListMenu *list);
static void BuyMenuPrintPriceInList(u8 windowId, u32 itemId, u8 y);

static const struct YesNoFuncTable sShopPurchaseYesNoFuncs =
{
    BuyMenuTryMakePurchase,
    BuyMenuReturnToItemList
};

static const struct MenuAction sShopMenuActions_BuySellQuit[] =
{
    { gText_ShopBuy, {.void_u8=Task_HandleShopMenuBuy} },
    { gText_ShopSell, {.void_u8=Task_HandleShopMenuSell} },
    { gText_ShopQuit, {.void_u8=Task_HandleShopMenuQuit} }
};

static const struct MenuAction sShopMenuActions_BuyQuit[] =
{
    { gText_ShopBuy, {.void_u8=Task_HandleShopMenuBuy} },
    { gText_ShopQuit, {.void_u8=Task_HandleShopMenuQuit} }
};

static const struct WindowTemplate sShopMenuWindowTemplates[] =
{
    [WIN_BUY_SELL_QUIT] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 9,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x0008,
    },
    // Separate shop menu window for decorations, which can't be sold
    [WIN_BUY_QUIT] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 9,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x0008,
    }
};

static const struct ListMenuTemplate sShopBuyMenuListTemplate =
{
    .items = NULL,
    .moveCursorFunc = BuyMenuPrintItemDescriptionAndShowItemIcon,
    .itemPrintFunc = BuyMenuPrintPriceInList,
    .totalItems = 0,
    .maxShowed = 0,
    .windowId = WIN_ITEM_LIST,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 2,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NARROW,
    .cursorKind = CURSOR_BLACK_ARROW
};

static const struct BgTemplate sShopBuyMenuBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sShopBuyMenuWindowTemplates[] =
{
    [WIN_MONEY] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 10,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x001E,
    },
    [WIN_ITEM_LIST] = {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 2,
        .width = 15,
        .height = 16,
        .paletteNum = 15,
        .baseBlock = 0x0032,
    },
    [WIN_ITEM_DESCRIPTION] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 13,
        .width = 14,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x0122,
    },
    [WIN_QUANTITY_IN_BAG] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 11,
        .width = 12,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x0176,
    },
    [WIN_QUANTITY_PRICE] = {
        .bg = 0,
        .tilemapLeft = 18,
        .tilemapTop = 11,
        .width = 10,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x018E,
    },
    [WIN_MESSAGE] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 27,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x01A2,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct WindowTemplate sShopBuyMenuYesNoWindowTemplates =
{
    .bg = 0,
    .tilemapLeft = 21,
    .tilemapTop = 9,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x020E,
};

static const u8 sShopBuyMenuTextColors[][3] =
{
    [COLORID_NORMAL]      = {1, 2, 3},
    [COLORID_ITEM_LIST]   = {0, 2, 3},
    [COLORID_GRAY_CURSOR] = {0, 3, 2},
};

static u8 CreateShopMenu(u8 martType)
{
    int numMenuItems;

    LockPlayerFieldControls();
    sMartInfo.martType = martType;

    if (martType == MART_TYPE_NORMAL)
    {
        struct WindowTemplate winTemplate = sShopMenuWindowTemplates[WIN_BUY_SELL_QUIT];
        winTemplate.width = GetMaxWidthInMenuTable(sShopMenuActions_BuySellQuit, ARRAY_COUNT(sShopMenuActions_BuySellQuit));
        sMartInfo.windowId = AddWindow(&winTemplate);
        sMartInfo.menuActions = sShopMenuActions_BuySellQuit;
        numMenuItems = ARRAY_COUNT(sShopMenuActions_BuySellQuit);
    }
    else
    {
        struct WindowTemplate winTemplate = sShopMenuWindowTemplates[WIN_BUY_QUIT];
        winTemplate.width = GetMaxWidthInMenuTable(sShopMenuActions_BuyQuit, ARRAY_COUNT(sShopMenuActions_BuyQuit));
        sMartInfo.windowId = AddWindow(&winTemplate);
        sMartInfo.menuActions = sShopMenuActions_BuyQuit;
        numMenuItems = ARRAY_COUNT(sShopMenuActions_BuyQuit);
    }

    SetStandardWindowBorderStyle(sMartInfo.windowId, FALSE);
    PrintMenuTable(sMartInfo.windowId, numMenuItems, sMartInfo.menuActions);
    InitMenuInUpperLeftCornerNormal(sMartInfo.windowId, numMenuItems, 0);
    PutWindowTilemap(sMartInfo.windowId);
    CopyWindowToVram(sMartInfo.windowId, COPYWIN_MAP);

    return CreateTask(Task_ShopMenu, 8);
}

static void SetShopMenuCallback(void (*callback)(void))
{
    sMartInfo.callback = callback;
}

static void SetShopItemsForSale(const u16 *items)
{
    u16 i = 0;

    sMartInfo.itemList = items;
    sMartInfo.itemCount = 0;

    // Read items until ITEM_NONE / DECOR_NONE is reached
    while (sMartInfo.itemList[i])
    {
        sMartInfo.itemCount++;
        i++;
    }
}

static void Task_ShopMenu(u8 taskId)
{
    s8 inputCode = Menu_ProcessInputNoWrap();
    switch (inputCode)
    {
    case MENU_NOTHING_CHOSEN:
        break;
    case MENU_B_PRESSED:
        PlaySE(SE_SELECT);
        Task_HandleShopMenuQuit(taskId);
        break;
    default:
        sMartInfo.menuActions[inputCode].func.void_u8(taskId);
        break;
    }
}

#define tItemCount  data[1]
#define tItemId     data[5]
#define tListTaskId data[7]

static void Task_HandleShopMenuBuy(u8 taskId)
{
    gTasks[taskId].ptr.funcPtr = CB2_InitBuyMenu;
    gTasks[taskId].func = Task_GoToBuyOrSellMenu;
    FadeScreen(FADE_TO_BLACK, 0);
}

static void Task_HandleShopMenuSell(u8 taskId)
{
    gTasks[taskId].ptr.funcPtr = CB2_GoToSellMenu;
    gTasks[taskId].func = Task_GoToBuyOrSellMenu;
    FadeScreen(FADE_TO_BLACK, 0);
}

void CB2_ExitSellMenu(void)
{
    gFieldCallback = MapPostLoadHook_ReturnToShopMenu;
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_HandleShopMenuQuit(u8 taskId)
{
    ClearStdWindowAndFrameToTransparent(sMartInfo.windowId, 2); // Incorrect use, making it not copy it to vram.
    RemoveWindow(sMartInfo.windowId);
    TryPutSmartShopperOnAir();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);

    if (sMartInfo.callback)
        sMartInfo.callback();
}

static void Task_GoToBuyOrSellMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        SetMainCallback2(gTasks[taskId].ptr.funcPtr);
    }
}

static void MapPostLoadHook_ReturnToShopMenu(void)
{
    FadeInFromBlack();
    CreateTask(Task_ReturnToShopMenu, 8);
}

static void Task_ReturnToShopMenu(u8 taskId)
{
    if (IsWeatherNotFadingIn() == TRUE)
    {
        if (sMartInfo.martType == MART_TYPE_DECOR2)
            DisplayItemMessageOnField(taskId, gText_CanIHelpWithAnythingElse, ShowShopMenuAfterExitingBuyOrSellMenu);
        else
            DisplayItemMessageOnField(taskId, gText_AnythingElseICanHelp, ShowShopMenuAfterExitingBuyOrSellMenu);
    }
}

static void ShowShopMenuAfterExitingBuyOrSellMenu(u8 taskId)
{
    CreateShopMenu(sMartInfo.martType);
    DestroyTask(taskId);
}

static void CB2_BuyMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void VBlankCB_BuyMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_InitBuyMenu(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        CpuFastFill(0, (void *)OAM, OAM_SIZE);
        ScanlineEffect_Stop();
        ResetTempTileDataBuffers();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        ResetSpriteData();
        ResetTasks();
        ClearScheduledBgCopiesToVram();
        sShopData = AllocZeroed(sizeof(struct ShopData));
        sShopData->scrollIndicatorsTaskId = TASK_NONE;
        sShopData->itemSpriteIds[0] = SPRITE_NONE;
        sShopData->itemSpriteIds[1] = SPRITE_NONE;
        BuyMenuBuildListMenuTemplate();
        BuyMenuInitBgs();
        FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(2, 0, 0, 0, 0x20, 0x20);
        FillBgTilemapBufferRect_Palette0(3, 0, 0, 0, 0x20, 0x20);
        BuyMenuInitWindows();
        BuyMenuDecompressBgGraphics();
        gMain.state++;
        break;
    case 1:
        if (!FreeTempTileDataBuffersIfPossible())
            gMain.state++;
        break;
    default:
        BuyMenuDrawGraphics();
        BuyMenuAddScrollIndicatorArrows();
        taskId = CreateTask(Task_BuyMenu, 8);
        gTasks[taskId].tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, 0, 0);
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_BuyMenu);
        SetMainCallback2(CB2_BuyMenu);
        break;
    }
}

static void BuyMenuFreeMemory(void)
{
    Free(sShopData);
    Free(sListMenuItems);
    Free(sItemNames);
    FreeAllWindowBuffers();
}

static void BuyMenuBuildListMenuTemplate(void)
{
    u16 i;

    sListMenuItems = Alloc((sMartInfo.itemCount + 1) * sizeof(*sListMenuItems));
    sItemNames = Alloc((sMartInfo.itemCount + 1) * sizeof(*sItemNames));
    for (i = 0; i < sMartInfo.itemCount; i++)
        BuyMenuSetListEntry(&sListMenuItems[i], sMartInfo.itemList[i], sItemNames[i]);

    StringCopy(sItemNames[i], gText_Cancel2);
    sListMenuItems[i].name = sItemNames[i];
    sListMenuItems[i].id = LIST_CANCEL;

    gMultiuseListMenuTemplate = sShopBuyMenuListTemplate;
    gMultiuseListMenuTemplate.items = sListMenuItems;
    gMultiuseListMenuTemplate.totalItems = sMartInfo.itemCount + 1;
    if (gMultiuseListMenuTemplate.totalItems > MAX_ITEMS_SHOWN)
        gMultiuseListMenuTemplate.maxShowed = MAX_ITEMS_SHOWN;
    else
        gMultiuseListMenuTemplate.maxShowed = gMultiuseListMenuTemplate.totalItems;

    sShopData->itemsShowed = gMultiuseListMenuTemplate.maxShowed;
}

static void BuyMenuSetListEntry(struct ListMenuItem *menuItem, u16 item, u8 *name)
{
    if (sMartInfo.martType == MART_TYPE_NORMAL)
        CopyItemName(item, name);
    else
        StringCopy(name, gDecorations[item].name);

    menuItem->name = name;
    menuItem->id = item;
}

static void BuyMenuPrintItemDescriptionAndShowItemIcon(s32 item, bool8 onInit, struct ListMenu *list)
{
    const u8 *description;
    if (onInit != TRUE)
        PlaySE(SE_SELECT);

    if (item != LIST_CANCEL)
        BuyMenuAddItemIcon(item, sShopData->iconSlot);
    else
        BuyMenuAddItemIcon(ITEM_LIST_END, sShopData->iconSlot);

    BuyMenuRemoveItemIcon(item, sShopData->iconSlot ^ 1);
    sShopData->iconSlot ^= 1;
    if (item != LIST_CANCEL)
    {
        if (sMartInfo.martType == MART_TYPE_NORMAL)
            description = GetItemDescription(item);
        else
            description = gDecorations[item].description;
    }
    else
    {
        description = gText_QuitShopping;
    }

    FillWindowPixelBuffer(WIN_ITEM_DESCRIPTION, PIXEL_FILL(0));
    if (item != LIST_CANCEL && sMartInfo.martType == MART_TYPE_NORMAL)   // Moemon Co-op: wrapped to the box (mp_itemdesc.c)
        MP_ItemDesc_Print(WIN_ITEM_DESCRIPTION, description, 3, 1, 14 * 8 - 4, 48, sShopBuyMenuTextColors[COLORID_NORMAL], PIXEL_FILL(0));
    else
        BuyMenuPrint(WIN_ITEM_DESCRIPTION, description, 3, 1, 0, COLORID_NORMAL);
}

static void BuyMenuPrintPriceInList(u8 windowId, u32 itemId, u8 y)
{
    u8 x;

    if (itemId != LIST_CANCEL)
    {
        if (sMartInfo.martType == MART_TYPE_NORMAL)
        {
            ConvertIntToDecimalStringN(
                gStringVar1,
                GetItemPrice(itemId) >> IsPokeNewsActive(POKENEWS_SLATEPORT),
                STR_CONV_MODE_LEFT_ALIGN,
                5);
        }
        else
        {
            ConvertIntToDecimalStringN(
                gStringVar1,
                gDecorations[itemId].price,
                STR_CONV_MODE_LEFT_ALIGN,
                5);
        }

        StringExpandPlaceholders(gStringVar4, gText_PokedollarVar1);
        x = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 120);
        AddTextPrinterParameterized4(windowId, FONT_NARROW, x, y, 0, 0, sShopBuyMenuTextColors[COLORID_ITEM_LIST], TEXT_SKIP_DRAW, gStringVar4);
    }
}

static void BuyMenuAddScrollIndicatorArrows(void)
{
    if (sShopData->scrollIndicatorsTaskId == TASK_NONE && sMartInfo.itemCount + 1 > MAX_ITEMS_SHOWN)
    {
        sShopData->scrollIndicatorsTaskId = AddScrollIndicatorArrowPairParameterized(
            SCROLL_ARROW_UP,
            172,
            12,
            148,
            sMartInfo.itemCount - (MAX_ITEMS_SHOWN - 1),
            TAG_SCROLL_ARROW,
            TAG_SCROLL_ARROW,
            &sShopData->scrollOffset);
    }
}

static void BuyMenuRemoveScrollIndicatorArrows(void)
{
    if (sShopData->scrollIndicatorsTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sShopData->scrollIndicatorsTaskId);
        sShopData->scrollIndicatorsTaskId = TASK_NONE;
    }
}

static void BuyMenuPrintCursor(u8 scrollIndicatorsTaskId, u8 colorSet)
{
    u8 y = ListMenuGetYCoordForPrintingArrowCursor(scrollIndicatorsTaskId);
    BuyMenuPrint(WIN_ITEM_LIST, gText_SelectorArrow2, 0, y, 0, colorSet);
}

static void BuyMenuAddItemIcon(u16 item, u8 iconSlot)
{
    u8 spriteId;
    u8 *spriteIdPtr = &sShopData->itemSpriteIds[iconSlot];
    if (*spriteIdPtr != SPRITE_NONE)
        return;

    if (sMartInfo.martType == MART_TYPE_NORMAL || item == ITEM_LIST_END)
    {
        spriteId = AddItemIconSprite(iconSlot + TAG_ITEM_ICON_BASE, iconSlot + TAG_ITEM_ICON_BASE, item);
        if (spriteId != MAX_SPRITES)
        {
            *spriteIdPtr = spriteId;
            gSprites[spriteId].x2 = 24;
            gSprites[spriteId].y2 = 88;
        }
    }
    else
    {
        spriteId = AddDecorationIconObject(item, 20, 84, 1, iconSlot + TAG_ITEM_ICON_BASE, iconSlot + TAG_ITEM_ICON_BASE);
        if (spriteId != MAX_SPRITES)
            *spriteIdPtr = spriteId;
    }
}

static void BuyMenuRemoveItemIcon(u16 item, u8 iconSlot)
{
    u8 *spriteIdPtr = &sShopData->itemSpriteIds[iconSlot];
    if (*spriteIdPtr == SPRITE_NONE)
        return;

    FreeSpriteTilesByTag(iconSlot + TAG_ITEM_ICON_BASE);
    FreeSpritePaletteByTag(iconSlot + TAG_ITEM_ICON_BASE);
    DestroySprite(&gSprites[*spriteIdPtr]);
    *spriteIdPtr = SPRITE_NONE;
}

static void BuyMenuInitBgs(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sShopBuyMenuBgTemplates, ARRAY_COUNT(sShopBuyMenuBgTemplates));
    SetBgTilemapBuffer(1, sShopData->tilemapBuffers[1]);
    SetBgTilemapBuffer(2, sShopData->tilemapBuffers[3]);
    SetBgTilemapBuffer(3, sShopData->tilemapBuffers[2]);
    SetGpuReg(REG_OFFSET_BG0HOFS, 0);
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    SetGpuReg(REG_OFFSET_BG1HOFS, 0);
    SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    SetGpuReg(REG_OFFSET_BG2HOFS, 0);
    SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    SetGpuReg(REG_OFFSET_BG3HOFS, 0);
    SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    ShowBg(3);
}

static void BuyMenuDecompressBgGraphics(void)
{
    DecompressAndCopyTileDataToVram(1, gShopMenu_Gfx, 0x3A0, 0x3E3, 0);
    LZDecompressWram(gShopMenu_Tilemap, sShopData->tilemapBuffers[0]);
    LoadCompressedPalette(gShopMenu_Pal, BG_PLTT_ID(SHOP_MENU_PALETTE_ID), PLTT_SIZE_4BPP);
}

static void BuyMenuInitWindows(void)
{
    InitWindows(sShopBuyMenuWindowTemplates);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(WIN_MONEY, 1, BG_PLTT_ID(13));
    LoadMessageBoxGfx(WIN_MONEY, 0xA, BG_PLTT_ID(14));
    PutWindowTilemap(WIN_MONEY);
    PutWindowTilemap(WIN_ITEM_LIST);
    PutWindowTilemap(WIN_ITEM_DESCRIPTION);
}

static void BuyMenuPrint(u8 windowId, const u8 *text, u8 x, u8 y, s8 speed, u8 colorSet)
{
    AddTextPrinterParameterized4(windowId, FONT_NORMAL, x, y, 0, 0, sShopBuyMenuTextColors[colorSet], speed, text);
}

static void BuyMenuDisplayMessage(u8 taskId, const u8 *text, TaskFunc callback)
{
    DisplayMessageAndContinueTask(taskId, WIN_MESSAGE, 10, 14, FONT_NORMAL, GetPlayerTextSpeedDelay(), text, callback);
    ScheduleBgCopyTilemapToVram(0);
}

static void BuyMenuDrawGraphics(void)
{
    BuyMenuDrawMapGraphics();
    BuyMenuCopyMenuBgToBg1TilemapBuffer();
    AddMoneyLabelObject(19, 11);
    PrintMoneyAmountInMoneyBoxWithBorder(WIN_MONEY, 1, 13, GetMoney(&gSaveBlock1Ptr->money));
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
    ScheduleBgCopyTilemapToVram(3);
}

static void BuyMenuDrawMapGraphics(void)
{
    BuyMenuCollectObjectEventData();
    BuyMenuDrawObjectEvents();
    BuyMenuDrawMapBg();
}

static void BuyMenuDrawMapBg(void)
{
    s16 i, j;
    s16 x, y;
    const struct MapLayout *mapLayout;
    u16 metatile;
    u8 metatileLayerType;

    mapLayout = gMapHeader.mapLayout;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    x -= 4;
    y -= 4;

    for (j = 0; j < 10; j++)
    {
        for (i = 0; i < 15; i++)
        {
            metatile = MapGridGetMetatileIdAt(x + i, y + j);
            if (BuyMenuCheckForOverlapWithMenuBg(i, j) == TRUE)
                metatileLayerType = MapGridGetMetatileLayerTypeAt(x + i, y + j);
            else
                metatileLayerType = METATILE_LAYER_TYPE_COVERED;

            if (metatile < Fieldmap_PrimaryMetatiles(mapLayout))   // Moemon Kai: FRLG split
                BuyMenuDrawMapMetatile(i, j, mapLayout->primaryTileset->metatiles + metatile * NUM_TILES_PER_METATILE, metatileLayerType);
            else
                BuyMenuDrawMapMetatile(i, j, mapLayout->secondaryTileset->metatiles + ((metatile - Fieldmap_PrimaryMetatiles(mapLayout)) * NUM_TILES_PER_METATILE), metatileLayerType);
        }
    }
}

static void BuyMenuDrawMapMetatile(s16 x, s16 y, const u16 *src, u8 metatileLayerType)
{
    u16 offset1 = x * 2;
    u16 offset2 = y * 64;

    switch (metatileLayerType)
    {
    case METATILE_LAYER_TYPE_NORMAL:
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[3], offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[1], offset1, offset2, src + 4);
        break;
    case METATILE_LAYER_TYPE_COVERED:
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[2], offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[3], offset1, offset2, src + 4);
        break;
    case METATILE_LAYER_TYPE_SPLIT:
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[2], offset1, offset2, src);
        BuyMenuDrawMapMetatileLayer(sShopData->tilemapBuffers[1], offset1, offset2, src + 4);
        break;
    }
}

static void BuyMenuDrawMapMetatileLayer(u16 *dest, s16 offset1, s16 offset2, const u16 *src)
{
    // This function draws a whole 2x2 metatile.
    dest[offset1 + offset2] = src[0]; // top left
    dest[offset1 + offset2 + 1] = src[1]; // top right
    dest[offset1 + offset2 + 32] = src[2]; // bottom left
    dest[offset1 + offset2 + 33] = src[3]; // bottom right
}

static void BuyMenuCollectObjectEventData(void)
{
    s16 facingX;
    s16 facingY;
    u8 y;
    u8 x;
    u8 numObjects = 0;

    GetXYCoordsOneStepInFrontOfPlayer(&facingX, &facingY);

    for (y = 0; y < OBJECT_EVENTS_COUNT; y++)
        sShopData->viewportObjects[y][OBJ_EVENT_ID] = OBJECT_EVENTS_COUNT;

    for (y = 0; y < 5; y++)
    {
        for (x = 0; x < 7; x++)
        {
            u8 objEventId = GetObjectEventIdByXY(facingX - 4 + x, facingY - 2 + y);

            if (objEventId != OBJECT_EVENTS_COUNT)
            {
                sShopData->viewportObjects[numObjects][OBJ_EVENT_ID] = objEventId;
                sShopData->viewportObjects[numObjects][X_COORD] = x;
                sShopData->viewportObjects[numObjects][Y_COORD] = y;
                sShopData->viewportObjects[numObjects][LAYER_TYPE] = MapGridGetMetatileLayerTypeAt(facingX - 4 + x, facingY - 2 + y);

                switch (gObjectEvents[objEventId].facingDirection)
                {
                case DIR_SOUTH:
                    sShopData->viewportObjects[numObjects][ANIM_NUM] = ANIM_STD_FACE_SOUTH;
                    break;
                case DIR_NORTH:
                    sShopData->viewportObjects[numObjects][ANIM_NUM] = ANIM_STD_FACE_NORTH;
                    break;
                case DIR_WEST:
                    sShopData->viewportObjects[numObjects][ANIM_NUM] = ANIM_STD_FACE_WEST;
                    break;
                case DIR_EAST:
                default:
                    sShopData->viewportObjects[numObjects][ANIM_NUM] = ANIM_STD_FACE_EAST;
                    break;
                }
                numObjects++;
            }
        }
    }
}

static void BuyMenuDrawObjectEvents(void)
{
    u8 i;
    u8 spriteId;
    const struct ObjectEventGraphicsInfo *graphicsInfo;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (sShopData->viewportObjects[i][OBJ_EVENT_ID] == OBJECT_EVENTS_COUNT)
            continue;

        graphicsInfo = GetObjectEventGraphicsInfo(OBJ_GFX_ID(&gObjectEvents[sShopData->viewportObjects[i][OBJ_EVENT_ID]]));

        spriteId = CreateObjectGraphicsSprite(
            OBJ_GFX_ID(&gObjectEvents[sShopData->viewportObjects[i][OBJ_EVENT_ID]]),
            SpriteCallbackDummy,
            (u16)sShopData->viewportObjects[i][X_COORD] * 16 + 8,
            (u16)sShopData->viewportObjects[i][Y_COORD] * 16 + 48 - graphicsInfo->height / 2,
            2);

        if (BuyMenuCheckIfObjectEventOverlapsMenuBg(sShopData->viewportObjects[i]) == TRUE)
        {
            gSprites[spriteId].subspriteTableNum = 4;
            gSprites[spriteId].subspriteMode = SUBSPRITES_ON;
        }

        // Moemon Co-op: other players' avatars in their own colors
        MP_Overworld_MenuSpritePalette(spriteId, OBJ_GFX_ID(&gObjectEvents[sShopData->viewportObjects[i][OBJ_EVENT_ID]]));
        StartSpriteAnim(&gSprites[spriteId], sShopData->viewportObjects[i][ANIM_NUM]);
    }
}

static bool8 BuyMenuCheckIfObjectEventOverlapsMenuBg(s16 *object)
{
    if (!BuyMenuCheckForOverlapWithMenuBg(object[X_COORD], object[Y_COORD] + 2) && object[LAYER_TYPE] != METATILE_LAYER_TYPE_COVERED)
        return TRUE;
    else
        return FALSE;
}

static void BuyMenuCopyMenuBgToBg1TilemapBuffer(void)
{
    s16 i;
    u16 *dest = sShopData->tilemapBuffers[1];
    const u16 *src = sShopData->tilemapBuffers[0];

    for (i = 0; i < 1024; i++)
    {
        if (src[i] != 0)
            dest[i] = src[i] + ((SHOP_MENU_PALETTE_ID << 12) | 0x3E3);
    }
}

static bool8 BuyMenuCheckForOverlapWithMenuBg(int x, int y)
{
    const u16 *metatile = sShopData->tilemapBuffers[0];
    int offset1 = x * 2;
    int offset2 = y * 64;

    if (metatile[offset2 + offset1] == 0 &&
        metatile[offset2 + offset1 + 32] == 0 &&
        metatile[offset2 + offset1 + 1] == 0 &&
        metatile[offset2 + offset1 + 33] == 0)
        return TRUE;

    return FALSE;
}

static void Task_BuyMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        s32 itemId = ListMenu_ProcessInput(tListTaskId);
        ListMenuGetScrollAndRow(tListTaskId, &sShopData->scrollOffset, &sShopData->selectedRow);

        switch (itemId)
        {
        case LIST_NOTHING_CHOSEN:
            break;
        case LIST_CANCEL:
            PlaySE(SE_SELECT);
            ExitBuyMenu(taskId);
            break;
        default:
            PlaySE(SE_SELECT);
            tItemId = itemId;
            ClearWindowTilemap(WIN_ITEM_DESCRIPTION);
            BuyMenuRemoveScrollIndicatorArrows();
            BuyMenuPrintCursor(tListTaskId, COLORID_GRAY_CURSOR);

            if (sMartInfo.martType == MART_TYPE_NORMAL)
                sShopData->totalCost = (GetItemPrice(itemId) >> IsPokeNewsActive(POKENEWS_SLATEPORT));
            else
                sShopData->totalCost = gDecorations[itemId].price;

            if (sMartInfo.martType == MART_TYPE_NORMAL && GetItemPocket(itemId) == POCKET_TM_HM
             && (CheckBagHasItem(itemId, 1) || CheckPCHasItem(itemId, 1)))
            {
                // Moemon Co-op: TMs are reusable (as in later games): one is all anyone needs
                static u8 sHaveIt[80];
                MP_AsciiToGame(sHaveIt, "You already have that TM.\nIt can be used again and again!", sizeof(sHaveIt) - 1);
                BuyMenuDisplayMessage(taskId, sHaveIt, BuyMenuReturnToItemList);
            }
            else if (!IsEnoughMoney(&gSaveBlock1Ptr->money, sShopData->totalCost))
            {
                BuyMenuDisplayMessage(taskId, gText_YouDontHaveMoney, BuyMenuReturnToItemList);
            }
            else
            {
                if (sMartInfo.martType == MART_TYPE_NORMAL)
                {
                    CopyItemName(itemId, gStringVar1);
                    if (GetItemPocket(itemId) == POCKET_TM_HM)
                    {
                        StringCopy(gStringVar2, gMoveNames[ItemIdToBattleMoveId(itemId)]);
                        BuyMenuDisplayMessage(taskId, gText_Var1CertainlyHowMany2, Task_BuyHowManyDialogueInit);
                    }
                    else
                    {
                        BuyMenuDisplayMessage(taskId, gText_Var1CertainlyHowMany, Task_BuyHowManyDialogueInit);
                    }
                }
                else
                {
                    StringCopy(gStringVar1, gDecorations[itemId].name);
                    ConvertIntToDecimalStringN(gStringVar2, sShopData->totalCost, STR_CONV_MODE_LEFT_ALIGN, 6);

                    if (sMartInfo.martType == MART_TYPE_DECOR)
                        StringExpandPlaceholders(gStringVar4, gText_Var1IsItThatllBeVar2);
                    else // MART_TYPE_DECOR2
                        StringExpandPlaceholders(gStringVar4, gText_YouWantedVar1ThatllBeVar2);

                    BuyMenuDisplayMessage(taskId, gStringVar4, BuyMenuConfirmPurchase);
                }
            }
            break;
        }
    }
}

static void Task_BuyHowManyDialogueInit(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    u16 quantityInBag = CountTotalItemQuantityInBag(tItemId);
    u16 maxQuantity;

    DrawStdFrameWithCustomTileAndPalette(WIN_QUANTITY_IN_BAG, FALSE, 1, 13);
    ConvertIntToDecimalStringN(gStringVar1, quantityInBag, STR_CONV_MODE_RIGHT_ALIGN, MAX_ITEM_DIGITS + 1);
    StringExpandPlaceholders(gStringVar4, gText_InBagVar1);
    BuyMenuPrint(WIN_QUANTITY_IN_BAG, gStringVar4, 0, 1, 0, COLORID_NORMAL);
    tItemCount = 1;
    DrawStdFrameWithCustomTileAndPalette(WIN_QUANTITY_PRICE, FALSE, 1, 13);
    BuyMenuPrintItemQuantityAndPrice(taskId);
    ScheduleBgCopyTilemapToVram(0);

    // Moemon Kai: an item sold for nothing (JOHTO's KURT) has no money limit
    maxQuantity = sShopData->totalCost ? GetMoney(&gSaveBlock1Ptr->money) / sShopData->totalCost : MP_BAG_ITEM_CAPACITY;

    if (maxQuantity > MP_BAG_ITEM_CAPACITY)
        sShopData->maxQuantity = MP_BAG_ITEM_CAPACITY;
    else
        sShopData->maxQuantity = maxQuantity;
    if (GetItemPocket(tItemId) == POCKET_TM_HM && sShopData->maxQuantity > 1)
        sShopData->maxQuantity = 1;   // Moemon Co-op: a reusable TM sells one at a time

    gTasks[taskId].func = Task_BuyHowManyDialogueHandleInput;
}

static void Task_BuyHowManyDialogueHandleInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, sShopData->maxQuantity) == TRUE)
    {
        sShopData->totalCost = (GetItemPrice(tItemId) >> IsPokeNewsActive(POKENEWS_SLATEPORT)) * tItemCount;
        BuyMenuPrintItemQuantityAndPrice(taskId);
    }
    else
    {
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            ClearStdWindowAndFrameToTransparent(WIN_QUANTITY_PRICE, FALSE);
            ClearStdWindowAndFrameToTransparent(WIN_QUANTITY_IN_BAG, FALSE);
            ClearWindowTilemap(WIN_QUANTITY_PRICE);
            ClearWindowTilemap(WIN_QUANTITY_IN_BAG);
            PutWindowTilemap(WIN_ITEM_LIST);
            CopyItemName(tItemId, gStringVar1);
            ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, BAG_ITEM_CAPACITY_DIGITS);
            ConvertIntToDecimalStringN(gStringVar3, sShopData->totalCost, STR_CONV_MODE_LEFT_ALIGN, 6);
            BuyMenuDisplayMessage(taskId, gText_Var1AndYouWantedVar2, BuyMenuConfirmPurchase);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            ClearStdWindowAndFrameToTransparent(WIN_QUANTITY_PRICE, FALSE);
            ClearStdWindowAndFrameToTransparent(WIN_QUANTITY_IN_BAG, FALSE);
            ClearWindowTilemap(WIN_QUANTITY_PRICE);
            ClearWindowTilemap(WIN_QUANTITY_IN_BAG);
            BuyMenuReturnToItemList(taskId);
        }
    }
}

static void BuyMenuConfirmPurchase(u8 taskId)
{
    CreateYesNoMenuWithCallbacks(taskId, &sShopBuyMenuYesNoWindowTemplates, 1, 0, 0, 1, 13, &sShopPurchaseYesNoFuncs);
}

static void BuyMenuTryMakePurchase(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    PutWindowTilemap(WIN_ITEM_LIST);

    if (sMartInfo.martType == MART_TYPE_NORMAL)
    {
        if (AddBagItem(tItemId, tItemCount) == TRUE)
        {
            BuyMenuDisplayMessage(taskId, gText_HereYouGoThankYou, BuyMenuSubtractMoney);
            RecordItemPurchase(taskId);
        }
        else
        {
            BuyMenuDisplayMessage(taskId, gText_NoMoreRoomForThis, BuyMenuReturnToItemList);
        }
    }
    else
    {
        if (DecorationAdd(tItemId))
        {
            if (sMartInfo.martType == MART_TYPE_DECOR)
                BuyMenuDisplayMessage(taskId, gText_ThankYouIllSendItHome, BuyMenuSubtractMoney);
            else // MART_TYPE_DECOR2
                BuyMenuDisplayMessage(taskId, gText_ThanksIllSendItHome, BuyMenuSubtractMoney);
        }
        else
        {
            BuyMenuDisplayMessage(taskId, gText_SpaceForVar1Full, BuyMenuReturnToItemList);
        }
    }
}

static void BuyMenuSubtractMoney(u8 taskId)
{
    IncrementGameStat(GAME_STAT_SHOPPED);
    RemoveMoney(&gSaveBlock1Ptr->money, sShopData->totalCost);
    PlaySE(SE_SHOP);
    PrintMoneyAmountInMoneyBox(WIN_MONEY, GetMoney(&gSaveBlock1Ptr->money), 0);

    if (sMartInfo.martType == MART_TYPE_NORMAL)
        gTasks[taskId].func = Task_ReturnToItemListAfterItemPurchase;
    else
        gTasks[taskId].func = Task_ReturnToItemListAfterDecorationPurchase;
}

static void Task_ReturnToItemListAfterItemPurchase(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);

        // Purchasing 10+ Poke Balls gets the player a Premier Ball
        if (tItemId == ITEM_POKE_BALL && tItemCount >= 10 && AddBagItem(ITEM_PREMIER_BALL, 1) == TRUE)
            BuyMenuDisplayMessage(taskId, gText_ThrowInPremierBall, BuyMenuReturnToItemList);
        else
            BuyMenuReturnToItemList(taskId);
    }
}

static void Task_ReturnToItemListAfterDecorationPurchase(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BuyMenuReturnToItemList(taskId);
    }
}

static void BuyMenuReturnToItemList(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ClearDialogWindowAndFrameToTransparent(WIN_MESSAGE, FALSE);
    BuyMenuPrintCursor(tListTaskId, COLORID_ITEM_LIST);
    PutWindowTilemap(WIN_ITEM_LIST);
    PutWindowTilemap(WIN_ITEM_DESCRIPTION);
    ScheduleBgCopyTilemapToVram(0);
    BuyMenuAddScrollIndicatorArrows();
    gTasks[taskId].func = Task_BuyMenu;
}

static void BuyMenuPrintItemQuantityAndPrice(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    FillWindowPixelBuffer(WIN_QUANTITY_PRICE, PIXEL_FILL(1));
    PrintMoneyAmount(WIN_QUANTITY_PRICE, 38, 1, sShopData->totalCost, TEXT_SKIP_DRAW);
    ConvertIntToDecimalStringN(gStringVar1, tItemCount, STR_CONV_MODE_LEADING_ZEROS, BAG_ITEM_CAPACITY_DIGITS);
    StringExpandPlaceholders(gStringVar4, gText_xVar1);
    BuyMenuPrint(WIN_QUANTITY_PRICE, gStringVar4, 0, 1, 0, COLORID_NORMAL);
}

static void ExitBuyMenu(u8 taskId)
{
    gFieldCallback = MapPostLoadHook_ReturnToShopMenu;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ExitBuyMenu;
}

static void Task_ExitBuyMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        RemoveMoneyLabelObject();
        BuyMenuFreeMemory();
        SetMainCallback2(CB2_ReturnToField);
        DestroyTask(taskId);
    }
}

static void ClearItemPurchases(void)
{
    sPurchaseHistoryId = 0;
    memset(gMartPurchaseHistory, 0, sizeof(gMartPurchaseHistory));
}

static void RecordItemPurchase(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    u16 i;

    for (i = 0; i < ARRAY_COUNT(gMartPurchaseHistory); i++)
    {
        if (gMartPurchaseHistory[i].itemId == tItemId && gMartPurchaseHistory[i].quantity != 0)
        {
            if (gMartPurchaseHistory[i].quantity + tItemCount > 255)
                gMartPurchaseHistory[i].quantity = 255;
            else
                gMartPurchaseHistory[i].quantity += tItemCount;
            return;
        }
    }

    if (sPurchaseHistoryId < ARRAY_COUNT(gMartPurchaseHistory))
    {
        gMartPurchaseHistory[sPurchaseHistoryId].itemId = tItemId;
        gMartPurchaseHistory[sPurchaseHistoryId].quantity = tItemCount;
        sPurchaseHistoryId++;
    }
}

#undef tItemCount
#undef tItemId
#undef tListTaskId

// Moemon Kai: GENERATION in shops. Items past it aren't sold. The department stores (HOENN's LILYCOVE, JOHTO's
// GOLDENROD, KANTO's CELADON) add the newer items spread over their clerks by what each already sells (the clerk is
// known by the first item of their own list), and town marts add a few that suit the town once enough of the
// region's badges are won (sMpMartExtras).
static const u16 sMpEvoStones[] = {ITEM_SHINY_STONE, ITEM_DUSK_STONE, ITEM_DAWN_STONE, ITEM_ICE_STONE, ITEM_OVAL_STONE};
static const u16 sMpEvoTrade[] = {
    ITEM_PROTECTOR, ITEM_ELECTIRIZER, ITEM_MAGMARIZER, ITEM_DUBIOUS_DISC, ITEM_REAPER_CLOTH, ITEM_RAZOR_CLAW,
    ITEM_RAZOR_FANG, ITEM_UP_GRADE, ITEM_METAL_ALLOY, ITEM_AUSPICIOUS_ARMOR, ITEM_MALICIOUS_ARMOR, ITEM_LEADERS_CREST,
    ITEM_SCROLL_OF_DARKNESS, ITEM_SCROLL_OF_WATERS, ITEM_LINKING_CORD,
};
static const u16 sMpCurios[] = {
    ITEM_SWEET_APPLE, ITEM_TART_APPLE, ITEM_SYRUPY_APPLE, ITEM_CRACKED_POT, ITEM_CHIPPED_POT, ITEM_UNREMARKABLE_TEACUP,
    ITEM_MASTERPIECE_TEACUP, ITEM_WHIPPED_DREAM, ITEM_SACHET, ITEM_STRAWBERRY_SWEET, ITEM_LOVE_SWEET, ITEM_BERRY_SWEET,
    ITEM_CLOVER_SWEET, ITEM_FLOWER_SWEET, ITEM_STAR_SWEET, ITEM_RIBBON_SWEET, ITEM_RED_NECTAR, ITEM_YELLOW_NECTAR,
    ITEM_PINK_NECTAR, ITEM_PURPLE_NECTAR,
};
// (the mail Emerald only had from other games)
static const u16 sMpMail[] = {ITEM_DREAM_MAIL, ITEM_FAB_MAIL, ITEM_RETRO_MAIL};
static const u16 sMpTraining[] = {
    ITEM_POWER_BRACER, ITEM_POWER_BELT, ITEM_POWER_LENS, ITEM_POWER_BAND, ITEM_POWER_ANKLET, ITEM_POWER_WEIGHT,
    ITEM_EVIOLITE, ITEM_DESTINY_KNOT,
};
static const u16 sMpConsumables[] = {
    ITEM_FOCUS_SASH, ITEM_POWER_HERB, ITEM_AIR_BALLOON, ITEM_WEAKNESS_POLICY, ITEM_ABSORB_BULB, ITEM_CELL_BATTERY,
    ITEM_SNOWBALL, ITEM_ELECTRIC_SEED, ITEM_PSYCHIC_SEED, ITEM_MISTY_SEED, ITEM_GRASSY_SEED, ITEM_BOOSTER_ENERGY,
};
static const u16 sMpOffense[] = {
    ITEM_WIDE_LENS, ITEM_MUSCLE_BAND, ITEM_WISE_GLASSES, ITEM_EXPERT_BELT, ITEM_LIFE_ORB, ITEM_ZOOM_LENS,
    ITEM_METRONOME, ITEM_CHOICE_SCARF, ITEM_CHOICE_SPECS, ITEM_PIXIE_PLATE, ITEM_FAIRY_FEATHER, ITEM_TOXIC_ORB,
    ITEM_FLAME_ORB, ITEM_GRIP_CLAW,
};
static const u16 sMpDefense[] = {
    ITEM_LIGHT_CLAY, ITEM_ROCKY_HELMET, ITEM_ASSAULT_VEST, ITEM_HEAVY_DUTY_BOOTS, ITEM_SHED_SHELL, ITEM_BIG_ROOT,
    ITEM_BLACK_SLUDGE, ITEM_FLOAT_STONE, ITEM_IRON_BALL, ITEM_LAGGING_TAIL, ITEM_STICKY_BARB, ITEM_ICY_ROCK,
    ITEM_SMOOTH_ROCK, ITEM_HEAT_ROCK, ITEM_DAMP_ROCK,
};
// (MEGA STONES and Z-CRYSTALS only while their gimmicks are on: battle_gimmick.h)
static const u16 sMpMegaStones[] = {
    ITEM_VENUSAURITE, ITEM_CHARIZARDITE_X, ITEM_CHARIZARDITE_Y, ITEM_BLASTOISINITE, ITEM_BEEDRILLITE, ITEM_PIDGEOTITE,
    ITEM_ALAKAZITE, ITEM_SLOWBRONITE, ITEM_GENGARITE, ITEM_KANGASKHANITE, ITEM_PINSIRITE, ITEM_GYARADOSITE,
    ITEM_AERODACTYLITE, ITEM_MEWTWONITE_X, ITEM_MEWTWONITE_Y, ITEM_AMPHAROSITE, ITEM_STEELIXITE, ITEM_SCIZORITE,
    ITEM_HERACRONITE, ITEM_HOUNDOOMINITE, ITEM_TYRANITARITE, ITEM_SCEPTILITE, ITEM_BLAZIKENITE, ITEM_SWAMPERTITE,
    ITEM_GARDEVOIRITE, ITEM_SABLENITE, ITEM_MAWILITE, ITEM_AGGRONITE, ITEM_MEDICHAMITE, ITEM_MANECTITE,
    ITEM_SHARPEDONITE, ITEM_CAMERUPTITE, ITEM_ALTARIANITE, ITEM_BANETTITE, ITEM_ABSOLITE, ITEM_GLALITITE,
    ITEM_SALAMENCITE, ITEM_METAGROSSITE, ITEM_LATIASITE, ITEM_LATIOSITE, ITEM_LOPUNNITE, ITEM_GARCHOMPITE,
    ITEM_LUCARIONITE, ITEM_ABOMASITE, ITEM_GALLADITE, ITEM_AUDINITE, ITEM_DIANCITE,
};
static const u16 sMpZCrystals[] = {
    ITEM_NORMALIUM_Z, ITEM_FIGHTINIUM_Z, ITEM_FLYINIUM_Z, ITEM_POISONIUM_Z, ITEM_GROUNDIUM_Z, ITEM_ROCKIUM_Z,
    ITEM_BUGINIUM_Z, ITEM_GHOSTIUM_Z, ITEM_STEELIUM_Z, ITEM_FIRIUM_Z, ITEM_WATERIUM_Z, ITEM_GRASSIUM_Z,
    ITEM_ELECTRIUM_Z, ITEM_PSYCHIUM_Z, ITEM_ICIUM_Z, ITEM_DRAGONIUM_Z, ITEM_DARKINIUM_Z, ITEM_FAIRIUM_Z,
};

#define MP_ANY_CLERK 0xFFFF
#define MP_EXTRA(m, first, badges, list) {MAP_##m, first, badges, list, ARRAY_COUNT(list)}
#define MP_TOWN(m, badges, ...) {MAP_##m, MP_ANY_CLERK, badges, (const u16[]){__VA_ARGS__}, \
                                 ARRAY_COUNT(((const u16[]){__VA_ARGS__}))}
// (the badges each kind waits for in its region: mail and curios at once, training items from 2, the battle items
// from 3-4, evolution items from 4-5, MEGA STONES and Z-CRYSTALS from 6)
static const struct { u16 map; u16 firstItem; u8 badges; const u16 *items; u8 count; } sMpShopExtras[] = {
    // LILYCOVE: 2F balls / medicine, 3F vitamins / X items, 4F attack / defense TMs
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_2F, ITEM_POKE_BALL, 4, sMpEvoStones),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_2F, ITEM_POKE_BALL, 5, sMpEvoTrade),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_2F, ITEM_POTION, 0, sMpMail),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_2F, ITEM_POTION, 0, sMpCurios),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_3F, ITEM_PROTEIN, 2, sMpTraining),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_3F, ITEM_PROTEIN, 6, sMpZCrystals),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_3F, ITEM_X_SPEED, 3, sMpConsumables),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_3F, ITEM_X_SPEED, 6, sMpMegaStones),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_4F, ITEM_TM_FIRE_BLAST, 4, sMpOffense),
    MP_EXTRA(LILYCOVE_CITY_DEPARTMENT_STORE_4F, ITEM_TM_PROTECT, 4, sMpDefense),
    // GOLDENROD (on Crystal's lists, sMpBaseLists): 2F X items / balls, 3F the training and held items clerks,
    // 4F evolution items, 5F gimmick items
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE2F, ITEM_X_SPEED, 3, sMpConsumables),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE2F, ITEM_RARE_CANDY, 0, sMpCurios),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE3F, ITEM_MACHO_BRACE, 2, sMpTraining),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE3F, ITEM_MACHO_BRACE, 4, sMpOffense),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE3F, ITEM_NONE, 4, sMpDefense),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE4F, ITEM_WATER_STONE, 4, sMpEvoStones),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE4F, ITEM_WATER_STONE, 5, sMpEvoTrade),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE5F, ITEM_NONE, 6, sMpMegaStones),
    MP_EXTRA(JOHTO_GOLDENROD_DEPT_STORE5F, ITEM_NONE, 6, sMpZCrystals),
    // CELADON (on FRLG's lists): 2F TMs clerk, 4F herbs / vitamins clerks, 5F X items / treats clerks
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_2F, ITEM_BLUE_FLUTE, 4, sMpDefense),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_CHERI_BERRY, 4, sMpEvoStones),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_CHERI_BERRY, 5, sMpEvoTrade),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_ENERGY_POWDER, 3, sMpConsumables),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_HP_UP, 2, sMpTraining),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_HP_UP, 4, sMpOffense),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, ITEM_METEORITE, 6, sMpMegaStones),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, ITEM_METEORITE, 6, sMpZCrystals),
    MP_EXTRA(KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, ITEM_PEWTER_CRUNCHIES, 0, sMpCurios),

    // Town marts: a few that suit the town, from the region's badge count given
    MP_TOWN(RUSTBORO_CITY_MART, 1, ITEM_SMOOTH_ROCK, ITEM_SHED_SHELL),
    MP_TOWN(SLATEPORT_CITY_MART, 2, ITEM_AIR_BALLOON, ITEM_DAMP_ROCK, ITEM_ABSORB_BULB),
    MP_TOWN(MAUVILLE_CITY_MART, 3, ITEM_CELL_BATTERY, ITEM_ELECTRIC_SEED, ITEM_ELECTIRIZER),
    MP_TOWN(VERDANTURF_TOWN_MART, 3, ITEM_BIG_ROOT, ITEM_GRASSY_SEED, ITEM_MISTY_SEED),
    MP_TOWN(FALLARBOR_TOWN_MART, 3, ITEM_SMOOTH_ROCK, ITEM_SHINY_STONE),
    MP_TOWN(LAVARIDGE_TOWN_MART, 4, ITEM_HEAT_ROCK, ITEM_FLAME_ORB, ITEM_MAGMARIZER),
    MP_TOWN(FORTREE_CITY_MART, 6, ITEM_RAZOR_CLAW, ITEM_RAZOR_FANG, ITEM_WIDE_LENS),
    MP_TOWN(MOSSDEEP_CITY_MART, 7, ITEM_PSYCHIC_SEED, ITEM_LIGHT_CLAY, ITEM_DAWN_STONE),
    MP_TOWN(SOOTOPOLIS_CITY_MART, 8, ITEM_DUSK_STONE, ITEM_ICE_STONE, ITEM_DAMP_ROCK),
    MP_TOWN(JOHTO_VIOLET_MART, 0, ITEM_SHED_SHELL, ITEM_BIG_ROOT),
    MP_TOWN(JOHTO_AZALEA_MART, 1, ITEM_BLACK_SLUDGE, ITEM_TOXIC_ORB),
    MP_TOWN(JOHTO_ECRUTEAK_MART, 4, ITEM_REAPER_CLOTH, ITEM_DUSK_STONE),
    MP_TOWN(JOHTO_OLIVINE_MART, 4, ITEM_FLOAT_STONE, ITEM_AIR_BALLOON, ITEM_DAMP_ROCK),
    MP_TOWN(JOHTO_MAHOGANY_MART1F, 6, ITEM_ICY_ROCK, ITEM_SNOWBALL, ITEM_ICE_STONE),
    MP_TOWN(JOHTO_BLACKTHORN_MART, 7, ITEM_RAZOR_FANG, ITEM_WEAKNESS_POLICY, ITEM_LIFE_ORB),
    MP_TOWN(KANTO_PEWTER_CITY_MART, 1, ITEM_SMOOTH_ROCK, ITEM_FLOAT_STONE),
    MP_TOWN(KANTO_CERULEAN_CITY_MART, 2, ITEM_DAMP_ROCK, ITEM_MISTY_SEED),
    MP_TOWN(KANTO_VERMILION_CITY_MART, 3, ITEM_CELL_BATTERY, ITEM_ELECTRIC_SEED, ITEM_ELECTIRIZER),
    MP_TOWN(KANTO_LAVENDER_TOWN_MART, 3, ITEM_REAPER_CLOTH, ITEM_DUSK_STONE),
    MP_TOWN(KANTO_FUCHSIA_CITY_MART, 5, ITEM_BLACK_SLUDGE, ITEM_TOXIC_ORB, ITEM_GRIP_CLAW),
    MP_TOWN(KANTO_SAFFRON_CITY_MART, 6, ITEM_PSYCHIC_SEED, ITEM_LIGHT_CLAY, ITEM_DAWN_STONE),
    MP_TOWN(KANTO_CINNABAR_ISLAND_MART, 7, ITEM_HEAT_ROCK, ITEM_MAGMARIZER, ITEM_DUBIOUS_DISC),
    MP_TOWN(KANTO_THREE_ISLAND_MART, 7, ITEM_SHED_SHELL, ITEM_ROCKY_HELMET),
    MP_TOWN(KANTO_FOUR_ISLAND_MART, 7, ITEM_ICY_ROCK, ITEM_SNOWBALL),
    MP_TOWN(KANTO_SIX_ISLAND_MART, 7, ITEM_RAZOR_CLAW, ITEM_OVAL_STONE),
    MP_TOWN(KANTO_SEVEN_ISLAND_MART, 7, ITEM_EXPERT_BELT, ITEM_FOCUS_SASH),
};

// JOHTO and KANTO sell what Pokemon Crystal (JOHTO: pret/pokecrystal data/items/marts.asm) and FireRed / LeafGreen
// (KANTO and the SEVII ISLANDS: pret/pokefirered's marts) sold, as Kai's items (no item those games had and Kai
// doesn't: Crystal's mail, SLOWPOKETAIL, RAGECANDYBAR, its punch TMs), plus only the items of Quetzal's own list
// from GEN 4 on (MP_Gen_OfItem; GENERATION decides). A clerk Quetzal added sells only those. Clerks are known by the
// first item of their Quetzal list (MP_REGION_LIST: a `pokemart 0` clerk).
#define MP_REGION_LIST 0xFFFE
#define MP_LIST(...) ((const u16[]){__VA_ARGS__, ITEM_NONE})
#define MP_BASE(m, first, ...) {MAP_##m, first, 0, MP_LIST(__VA_ARGS__)}
#define MP_BASE_BADGES(m, first, badges, ...) {MAP_##m, first, badges, MP_LIST(__VA_ARGS__)}
#define MP_ONLY_LATER(m, first) {MAP_##m, first, 0, (const u16[]){ITEM_NONE}}
static const struct { u16 map; u16 firstItem; u8 badges; const u16 *items; } sMpBaseLists[] = {
    // JOHTO (Crystal). CHERRYGROVE adds POKe BALLS with the POKeDEX (MP_ShopItems); MAHOGANY's second list once the
    // 7th badge is won (in Crystal: once TEAM ROCKET's hideout is cleared)
    MP_BASE(JOHTO_CHERRYGROVE_MART, MP_REGION_LIST, ITEM_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_AWAKENING),
    MP_BASE(JOHTO_VIOLET_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_POTION, ITEM_ESCAPE_ROPE, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_X_DEFEND, ITEM_X_ATTACK, ITEM_X_SPEED),
    MP_BASE(JOHTO_AZALEA_MART, MP_REGION_LIST, ITEM_CHARCOAL, ITEM_POKE_BALL, ITEM_POTION, ITEM_SUPER_POTION,
            ITEM_ESCAPE_ROPE, ITEM_REPEL, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL),
    MP_BASE(JOHTO_OLIVINE_MART, MP_REGION_LIST, ITEM_GREAT_BALL, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_ICE_HEAL, ITEM_SUPER_REPEL),
    MP_BASE(JOHTO_ECRUTEAK_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_POTION, ITEM_SUPER_POTION,
            ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ICE_HEAL, ITEM_REVIVE),
    MP_BASE_BADGES(JOHTO_MAHOGANY_MART1F, ITEM_PEWTER_CRUNCHIES, 7, ITEM_GREAT_BALL, ITEM_SUPER_POTION,
                   ITEM_HYPER_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_SUPER_REPEL, ITEM_REVIVE),
    MP_BASE(JOHTO_MAHOGANY_MART1F, ITEM_PEWTER_CRUNCHIES, ITEM_TINY_MUSHROOM, ITEM_POKE_BALL, ITEM_POTION),
    MP_BASE(JOHTO_BLACKTHORN_MART, MP_REGION_LIST, ITEM_GREAT_BALL, ITEM_ULTRA_BALL, ITEM_HYPER_POTION,
            ITEM_MAX_POTION, ITEM_FULL_HEAL, ITEM_REVIVE, ITEM_MAX_REPEL, ITEM_X_DEFEND, ITEM_X_ATTACK),
    MP_BASE(JOHTO_INDIGO_PLATEAU_POKECENTER1F, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_MAX_REPEL, ITEM_HYPER_POTION,
            ITEM_MAX_POTION, ITEM_FULL_RESTORE, ITEM_REVIVE, ITEM_FULL_HEAL),
    // GOLDENROD: 2F medicine, balls (Crystal's 2F) and X items (Crystal's 3F), 3F vitamins (Crystal's 4F); the
    // training, held items, stones and 5F clerks are Quetzal's
    MP_BASE(JOHTO_GOLDENROD_DEPT_STORE2F, MP_REGION_LIST, ITEM_POTION, ITEM_SUPER_POTION, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ICE_HEAL),
    MP_BASE(JOHTO_GOLDENROD_DEPT_STORE2F, ITEM_RARE_CANDY, ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ESCAPE_ROPE,
            ITEM_REPEL, ITEM_REVIVE, ITEM_FULL_HEAL, ITEM_POKE_DOLL),
    MP_BASE(JOHTO_GOLDENROD_DEPT_STORE2F, ITEM_X_SPEED, ITEM_X_SPEED, ITEM_X_SPECIAL, ITEM_X_DEFEND, ITEM_X_ATTACK,
            ITEM_DIRE_HIT, ITEM_GUARD_SPEC, ITEM_X_ACCURACY),
    MP_BASE(JOHTO_GOLDENROD_DEPT_STORE3F, ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CARBOS, ITEM_CALCIUM, ITEM_HP_UP),
    MP_ONLY_LATER(JOHTO_GOLDENROD_DEPT_STORE3F, ITEM_MACHO_BRACE),
    MP_ONLY_LATER(JOHTO_GOLDENROD_DEPT_STORE4F, ITEM_WATER_STONE),
    // KANTO (FireRed / LeafGreen)
    MP_BASE(KANTO_VIRIDIAN_CITY_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL),
    MP_BASE(KANTO_PEWTER_CITY_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL,
            ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ESCAPE_ROPE, ITEM_REPEL),
    MP_BASE(KANTO_CERULEAN_CITY_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_SUPER_POTION, ITEM_POTION, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ESCAPE_ROPE, ITEM_REPEL),
    MP_BASE(KANTO_VERMILION_CITY_MART, MP_REGION_LIST, ITEM_POKE_BALL, ITEM_SUPER_POTION, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_ICE_HEAL, ITEM_REPEL),
    MP_BASE(KANTO_LAVENDER_TOWN_MART, MP_REGION_LIST, ITEM_GREAT_BALL, ITEM_SUPER_POTION, ITEM_REVIVE, ITEM_ANTIDOTE,
            ITEM_PARALYZE_HEAL, ITEM_BURN_HEAL, ITEM_ICE_HEAL, ITEM_ESCAPE_ROPE, ITEM_SUPER_REPEL),
    MP_BASE(KANTO_FUCHSIA_CITY_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_GREAT_BALL, ITEM_SUPER_POTION, ITEM_REVIVE,
            ITEM_FULL_HEAL, ITEM_MAX_REPEL),
    MP_BASE(KANTO_SAFFRON_CITY_MART, MP_REGION_LIST, ITEM_GREAT_BALL, ITEM_HYPER_POTION, ITEM_REVIVE, ITEM_FULL_HEAL,
            ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL),
    MP_BASE(KANTO_CINNABAR_ISLAND_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_GREAT_BALL, ITEM_HYPER_POTION,
            ITEM_REVIVE, ITEM_FULL_HEAL, ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL),
    MP_BASE(KANTO_INDIGO_PLATEAU_POKEMON_CENTER_1F, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_GREAT_BALL,
            ITEM_FULL_RESTORE, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_FULL_HEAL, ITEM_MAX_REPEL),
    MP_BASE(KANTO_THREE_ISLAND_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_HYPER_POTION, ITEM_REVIVE, ITEM_FULL_HEAL,
            ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL),
    MP_BASE(KANTO_FOUR_ISLAND_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_FULL_RESTORE, ITEM_MAX_POTION, ITEM_REVIVE,
            ITEM_ICE_HEAL, ITEM_FULL_HEAL, ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL),
    MP_BASE(KANTO_SIX_ISLAND_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_FULL_RESTORE, ITEM_MAX_POTION, ITEM_REVIVE,
            ITEM_FULL_HEAL, ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL, ITEM_DREAM_MAIL),
    MP_BASE(KANTO_SEVEN_ISLAND_MART, MP_REGION_LIST, ITEM_ULTRA_BALL, ITEM_GREAT_BALL, ITEM_FULL_RESTORE,
            ITEM_MAX_POTION, ITEM_HYPER_POTION, ITEM_REVIVE, ITEM_FULL_HEAL, ITEM_ESCAPE_ROPE, ITEM_MAX_REPEL),
    // CELADON: 2F items and TMs, 4F the POKe DOLL, mail and stones, 5F X items and vitamins (FRLG's clerks, on
    // Quetzal's clerks by what they sell); Quetzal's balls, berries, herbs and treats clerks are its own
    MP_BASE(KANTO_CELADON_CITY_DEPARTMENT_STORE_2F, MP_REGION_LIST, ITEM_GREAT_BALL, ITEM_SUPER_POTION, ITEM_REVIVE,
            ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ICE_HEAL, ITEM_SUPER_REPEL),
    MP_BASE(KANTO_CELADON_CITY_DEPARTMENT_STORE_2F, ITEM_BLUE_FLUTE, ITEM_TM05, ITEM_TM15, ITEM_TM28, ITEM_TM31,
            ITEM_TM43, ITEM_TM45),
    MP_ONLY_LATER(KANTO_CELADON_CITY_DEPARTMENT_STORE_2F, ITEM_SPORT_BALL),
    MP_BASE(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_CHERI_BERRY, ITEM_POKE_DOLL, ITEM_RETRO_MAIL,
            ITEM_FIRE_STONE, ITEM_THUNDER_STONE, ITEM_WATER_STONE, ITEM_LEAF_STONE),
    MP_ONLY_LATER(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_CHILAN_BERRY),
    MP_ONLY_LATER(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_ENERGY_POWDER),
    MP_BASE(KANTO_CELADON_CITY_DEPARTMENT_STORE_4F, ITEM_HP_UP, ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CALCIUM,
            ITEM_ZINC, ITEM_CARBOS),
    MP_BASE(KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, ITEM_METEORITE, ITEM_X_ATTACK, ITEM_X_DEFEND, ITEM_X_SPEED,
            ITEM_X_SPECIAL, ITEM_X_ACCURACY, ITEM_GUARD_SPEC, ITEM_DIRE_HIT),
    MP_ONLY_LATER(KANTO_CELADON_CITY_DEPARTMENT_STORE_5F, ITEM_PEWTER_CRUNCHIES),
};

#include "data/qz_badge_marts.h"

static void MP_ShopAdd(u16 *out, u32 *n, u32 cap, u16 item)
{
    u32 k;
    for (k = 0; k < *n && out[k] != item; k++)
        ;
    if (k == *n && *n < cap && MP_Gen_ItemAllowed(item) && MP_Gimmick_ItemAllowed(item) && GetItemPrice(item) != 0)
        out[(*n)++] = item;
}

static const u16 *MP_ShopItems(const u16 *items)
{
    static u16 sItems[192];
    u16 here = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    u32 i, j, n = 0, cap = ARRAY_COUNT(sItems) - 1;
    u8 badges = MP_Region_BadgeCount(MP_Region_Current());
    u16 first;
    const u16 *base = NULL;
    if (items == NULL)   // Moemon Kai: JOHTO / KANTO's `pokemart 0`: the list for the region's badge count (Quetzal's)
    {
        items = sQzBadgeMarts[badges < ARRAY_COUNT(sQzBadgeMarts) ? badges : ARRAY_COUNT(sQzBadgeMarts) - 1];
        first = MP_REGION_LIST;
    }
    else
        first = items[0];
    // (the first list that fits: a list with a badge count before the same clerk's list without one)
    for (i = 0; i < ARRAY_COUNT(sMpBaseLists) && base == NULL; i++)
        if (sMpBaseLists[i].map == here && sMpBaseLists[i].firstItem == first && badges >= sMpBaseLists[i].badges)
            base = sMpBaseLists[i].items;
    if (base != NULL)
    {
        // the original game's list, then only Quetzal's items from GEN 4 on
        if (here == MAP_JOHTO_CHERRYGROVE_MART && FlagGet(FLAG_SYS_POKEDEX_GET))
            MP_ShopAdd(sItems, &n, cap, ITEM_POKE_BALL);
        for (i = 0; base[i] != ITEM_NONE; i++)
            MP_ShopAdd(sItems, &n, cap, base[i]);
        for (i = 0; items[i] != ITEM_NONE; i++)
            if (MP_Gen_OfItem(items[i]) >= 4)
                MP_ShopAdd(sItems, &n, cap, items[i]);
    }
    else
    {
        for (i = 0; items[i] != ITEM_NONE && n < cap; i++)
            if (MP_Gen_ItemAllowed(items[i]))
                sItems[n++] = items[i];
    }
    for (i = 0; i < ARRAY_COUNT(sMpShopExtras); i++)
    {
        if (sMpShopExtras[i].map != here || badges < sMpShopExtras[i].badges
         || (sMpShopExtras[i].firstItem != MP_ANY_CLERK && sMpShopExtras[i].firstItem != first))
            continue;
        for (j = 0; j < sMpShopExtras[i].count; j++)
            MP_ShopAdd(sItems, &n, cap, sMpShopExtras[i].items[j]);
    }
    sItems[n] = ITEM_NONE;
    return sItems;
}

// Moemon Kai (tests: mp shopdump): every changed clerk's list here, with the badges and GENERATION now (a clerk
// known by a first item stands in with just that item: their own Quetzal list's later items aren't shown)
void MP_ShopDump(void)
{
    u32 i, k;
    u16 saved = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    for (k = 0; k < ARRAY_COUNT(sMpBaseLists) + ARRAY_COUNT(sMpShopExtras); k++)
    {
        u16 map = k < ARRAY_COUNT(sMpBaseLists) ? sMpBaseLists[k].map : sMpShopExtras[k - ARRAY_COUNT(sMpBaseLists)].map;
        u16 first = k < ARRAY_COUNT(sMpBaseLists) ? sMpBaseLists[k].firstItem
                                                    : sMpShopExtras[k - ARRAY_COUNT(sMpBaseLists)].firstItem;
        u16 own[2] = {first, ITEM_NONE};
        const u16 *out;
        gSaveBlock1Ptr->location.mapGroup = map >> 8;
        gSaveBlock1Ptr->location.mapNum = map & 0xFF;
        out = MP_ShopItems(first == MP_REGION_LIST || first == MP_ANY_CLERK ? NULL : own);
        printf("[test] shop %04x %u:", map, first);
        for (i = 0; out[i] != ITEM_NONE; i++)
            printf(" %u", out[i]);
        printf("\n");
    }
    gSaveBlock1Ptr->location.mapGroup = saved >> 8;
    gSaveBlock1Ptr->location.mapNum = saved & 0xFF;
}

void CreatePokemartMenu(const u16 *itemsForSale)
{
    CreateShopMenu(MART_TYPE_NORMAL);
    SetShopItemsForSale(MP_ShopItems(itemsForSale));
    ClearItemPurchases();
    SetShopMenuCallback(ScriptContext_Enable);
}

void CreateDecorationShop1Menu(const u16 *itemsForSale)
{
    CreateShopMenu(MART_TYPE_DECOR);
    SetShopItemsForSale(itemsForSale);
    SetShopMenuCallback(ScriptContext_Enable);
}

void CreateDecorationShop2Menu(const u16 *itemsForSale)
{
    CreateShopMenu(MART_TYPE_DECOR2);
    SetShopItemsForSale(itemsForSale);
    SetShopMenuCallback(ScriptContext_Enable);
}

// Moemon Co-op: the shop's BUY / SELL / QUIT menu is open
bool8 MP_IsShopMenuOpen(void)
{
    return FuncIsActiveTask(Task_ShopMenu);
}
