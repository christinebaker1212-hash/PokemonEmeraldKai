// Moemon Kai: SINNOH places of this game's own (data/scripts/sinnoh_places.inc)
// VEILSTONE's PRIZE EXCHANGE: Platinum's 19 GAME CORNER prizes at half its COIN prices (the game's COIN CASE holds
// 9999: the dearest one at that), in a scrolling list of name and price
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "strings.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/field_specials.h"

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
extern u16 (*gQzScrollResult)(u16 choice);
void ShowScrollableMultichoice(void);

static const struct { u16 item, coins; } sPrizes[] = {
    {ITEM_SILK_SCARF, 500}, {ITEM_WIDE_LENS, 500}, {ITEM_ZOOM_LENS, 500}, {ITEM_METRONOME, 500},
    {ITEM_TM90, 1000}, {ITEM_TM58, 1000}, {ITEM_TM75, 2000}, {ITEM_TM32, 2000}, {ITEM_TM44, 3000},
    {ITEM_TM89, 3000}, {ITEM_TM10, 3000}, {ITEM_TM27, 4000}, {ITEM_TM21, 4000}, {ITEM_TM35, 5000},
    {ITEM_TM24, 5000}, {ITEM_TM13, 5000}, {ITEM_TM29, 5000}, {ITEM_TM74, 7500}, {ITEM_TM68, 9999},
};

static u8 sRows[ARRAY_COUNT(sPrizes)][32];

static u16 Picked(u16 row)
{
    if (row >= ARRAY_COUNT(sPrizes))
        return MULTI_B_PRESSED;
    gSpecialVar_0x8005 = sPrizes[row].item;
    gSpecialVar_0x8006 = sPrizes[row].coins;
    CopyItemName(sPrizes[row].item, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, sPrizes[row].coins, STR_CONV_MODE_LEFT_ALIGN, 4);
    return row;
}

// The prize list (waitstate; VAR_RESULT the row or MULTI_B_PRESSED; VAR_0x8005 the item, VAR_0x8006 its COINS,
// STR_VAR_1 / STR_VAR_2 their words)
u16 Sinnoh_PrizeMenu(void)
{
    static const u8 sSep[] = _(" ");
    static const u8 sCoins[] = _("C");
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPrizes); i++)
    {
        CopyItemName(sPrizes[i].item, sRows[i]);
        StringAppend(sRows[i], sSep);
        ConvertIntToDecimalStringN(sRows[i] + StringLength(sRows[i]), sPrizes[i].coins, STR_CONV_MODE_LEFT_ALIGN, 4);
        StringAppend(sRows[i], sCoins);
        gQzScrollOptions[i] = sRows[i];
    }
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = Picked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}
