// Moemon Kai: SINNOH places of this game's own (data/scripts/sinnoh_places.inc)
// VEILSTONE's PRIZE EXCHANGE: Platinum's 19 GAME CORNER prizes at half its COIN prices (the game's COIN CASE holds
// 9999: the dearest one at that), in a scrolling list of name and price
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "constants/species.h"
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

// The BATTLE FRONTIER's exchange booths: Platinum's lists for BATTLE POINTS (the left booth's items, the right one's
// TMs; the frontier's BP: HOENN's and the BATTLE TOWER's here)
static const struct { u16 item, bp; } sBpItems[] = {
    {ITEM_PROTEIN, 1}, {ITEM_CALCIUM, 1}, {ITEM_IRON, 1}, {ITEM_ZINC, 1}, {ITEM_CARBOS, 1}, {ITEM_HP_UP, 1},
    {ITEM_POWER_BRACER, 16}, {ITEM_POWER_BELT, 16}, {ITEM_POWER_LENS, 16}, {ITEM_POWER_BAND, 16},
    {ITEM_POWER_ANKLET, 16}, {ITEM_POWER_WEIGHT, 16}, {ITEM_TOXIC_ORB, 16}, {ITEM_FLAME_ORB, 16},
    {ITEM_WHITE_HERB, 32}, {ITEM_POWER_HERB, 32}, {ITEM_BRIGHT_POWDER, 48}, {ITEM_CHOICE_BAND, 48},
    {ITEM_FOCUS_BAND, 48}, {ITEM_SCOPE_LENS, 48}, {ITEM_MUSCLE_BAND, 48}, {ITEM_FOCUS_SASH, 48},
    {ITEM_CHOICE_SCARF, 48}, {ITEM_RAZOR_CLAW, 48}, {ITEM_RAZOR_FANG, 48}, {ITEM_RARE_CANDY, 48},
};
static const struct { u16 item, bp; } sBpTms[] = {
    {ITEM_TM06, 32}, {ITEM_TM73, 32}, {ITEM_TM61, 32}, {ITEM_TM45, 32}, {ITEM_TM40, 40}, {ITEM_TM31, 40},
    {ITEM_TM08, 48}, {ITEM_TM04, 48}, {ITEM_TM81, 64}, {ITEM_TM30, 64}, {ITEM_TM53, 64}, {ITEM_TM36, 80},
    {ITEM_TM59, 80}, {ITEM_TM71, 80}, {ITEM_TM26, 80},
};
static u8 sBpRows[ARRAY_COUNT(sBpItems)][32];
static u8 sBpList;

static u16 BpPicked(u16 row)
{
    u16 n = sBpList ? ARRAY_COUNT(sBpTms) : ARRAY_COUNT(sBpItems);
    if (row >= n)
        return MULTI_B_PRESSED;
    gSpecialVar_0x8005 = sBpList ? sBpTms[row].item : sBpItems[row].item;
    gSpecialVar_0x8006 = sBpList ? sBpTms[row].bp : sBpItems[row].bp;
    CopyItemName(gSpecialVar_0x8005, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, gSpecialVar_0x8006, STR_CONV_MODE_LEFT_ALIGN, 2);
    return row;
}

// VAR_0x8004 0: the items, 1: the TMs (waitstate; as Sinnoh_PrizeMenu, the price in BP)
u16 Sinnoh_BpMenu(void)
{
    static const u8 sBp[] = _(" BP");
    static const u8 sSep[] = _(" ");
    u32 i, n;
    sBpList = gSpecialVar_0x8004 != 0;
    n = sBpList ? ARRAY_COUNT(sBpTms) : ARRAY_COUNT(sBpItems);
    for (i = 0; i < n; i++)
    {
        u16 item = sBpList ? sBpTms[i].item : sBpItems[i].item, bp = sBpList ? sBpTms[i].bp : sBpItems[i].bp;
        CopyItemName(item, sBpRows[i]);
        StringAppend(sBpRows[i], sSep);
        ConvertIntToDecimalStringN(sBpRows[i] + StringLength(sBpRows[i]), bp, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(sBpRows[i], sBp);
        gQzScrollOptions[i] = sBpRows[i];
    }
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = BpPicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}

// The pick (VAR_0x8005, its BP VAR_0x8006) bought: VAR_RESULT 0 done, 1 not enough BP, 2 no room in the BAG
u16 Sinnoh_BpBuy(void)
{
    if (gSaveBlock2Ptr->frontier.battlePoints < gSpecialVar_0x8006)
        return gSpecialVar_Result = 1;
    if (!CheckBagHasSpace(gSpecialVar_0x8005, 1))
        return gSpecialVar_Result = 2;
    gSaveBlock2Ptr->frontier.battlePoints -= gSpecialVar_0x8006;
    AddBagItem(gSpecialVar_0x8005, 1);
    return gSpecialVar_Result = 0;
}

// PROF. ROWAN's briefcase on ROUTE 201 (a new game begun in SINNOH): Platinum's three (waitstate; VAR_RESULT the
// species, or 0 backed out)
static const u16 sStarters[] = {SPECIES_TURTWIG, SPECIES_CHIMCHAR, SPECIES_PIPLUP};
static u8 sStarterNames[ARRAY_COUNT(sStarters)][POKEMON_NAME_LENGTH + 1];

static u16 StarterPicked(u16 row)
{
    return row < ARRAY_COUNT(sStarters) ? sStarters[row] : 0;
}

u16 Sinnoh_StarterMenu(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sStarters); i++)
    {
        GetSpeciesName(sStarterNames[i], sStarters[i]);
        gQzScrollOptions[i] = sStarterNames[i];
    }
    gQzScrollCount = i;
    gQzScrollResult = StarterPicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}
