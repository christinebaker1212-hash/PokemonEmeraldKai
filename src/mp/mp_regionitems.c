// Moemon Kai: key items stay in their region (the user, 2026-11-08; docs/REGION_ITEMS.md). A region's story items,
// and the tools every region has its own of (bikes, devices, containers, rods, the COIN CASE, the ITEMFINDER, the
// S.S. TICKET), wait in that region's stash when the player leaves it, and come back on the return. A tool travels
// for good once the region it came from is beaten: a tool with the same name in other regions (a rod, the
// BICYCLE...) is then one item, every copy in the stashes and the GLOBAL PC merged into the one in the BAG. HMs wait in
// the GLOBAL PC (customs sends them there with the rest of the BAG) until the region they came from is beaten, and
// then merge into the BAG the same way. The CARD CASE, the CANDY JAR and Kai's own co-op and travel items always come
// along. Kept in the region side file (mp_region.c chunk "KIST").
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mp.h"
#include "mp_region.h"
#include "constants/items.h"

#define STASH_SLOTS 64
#define FREE_SLOTS 64
#define HM_SLOTS 16
#define KIST_VERSION 1

struct RegionItems
{
    u32 version;
    struct { u16 item, qty; } stash[REGION_COUNT][STASH_SLOTS];   // each region's key items left behind
    u16 free[FREE_SLOTS];                                         // the items that travel for good
    struct { u16 item; u8 regions, pad; } hmFrom[HM_SLOTS];       // where the GLOBAL PC's HMs came from
};
static struct RegionItems sKi;
static bool8 sMigrate;   // the save had no record: its BAG is sorted out once (old saves let every key item travel)

// Always along: the CARD CASE and CANDY JAR (the user's exceptions), Kai's co-op BATTLE FLAG, the tickets of the boats
// every region's port sails (ORRE, BATTLE ISLAND)
static const u16 sTravel[] = {ITEM_CARD_CASE, ITEM_CANDY_JAR, ITEM_BATTLE_FLAG, ITEM_ORRE_TICKET, ITEM_BATTLE_TICKET};

// Tools: each region's own (a regional version: bikes, devices, containers) or the same in every region (rods,
// COIN CASE, ITEMFINDER, S.S. TICKET). Their home, for saves from before (0xFF: every region has it)
static const struct { u16 item; u8 home; } sTools[] = {
    {ITEM_MACH_BIKE, REGION_HOENN}, {ITEM_ACRO_BIKE, REGION_HOENN}, {ITEM_MOENAV, REGION_HOENN},
    {ITEM_POKEBLOCK_CASE, REGION_HOENN}, {ITEM_WAILMER_PAIL, REGION_HOENN}, {ITEM_SOOT_SACK, REGION_HOENN},
    {ITEM_GO_GOGGLES, REGION_HOENN}, {ITEM_CONTEST_PASS, REGION_HOENN},
    {ITEM_BICYCLE, 0xFF}, {ITEM_VS_SEEKER, 0xFF},   // (KANTO's and JOHTO's / KANTO's and SINNOH's)
    {ITEM_TOWN_MAP, REGION_KANTO}, {ITEM_FAME_CHECKER, REGION_KANTO}, {ITEM_TEACHY_TV, REGION_KANTO},
    {ITEM_TM_CASE, REGION_KANTO}, {ITEM_BERRY_POUCH, REGION_KANTO},
    {ITEM_POKEGEAR, REGION_JOHTO},
    {ITEM_MTN_BIKE, REGION_SINNOH}, {ITEM_POKETCH, REGION_SINNOH}, {ITEM_POKE_RADAR, REGION_SINNOH},
    {ITEM_JOURNAL, REGION_SINNOH}, {ITEM_POFFIN_CASE, REGION_SINNOH}, {ITEM_SEAL_CASE, REGION_SINNOH},
    {ITEM_FASHION_CASE, REGION_SINNOH}, {ITEM_SPRAYDUCK, REGION_SINNOH}, {ITEM_VS_RECORDER, REGION_SINNOH},
    {ITEM_PAL_PAD, REGION_SINNOH}, {ITEM_EXPLORER_KIT, REGION_SINNOH},
    {ITEM_COIN_CASE, 0xFF}, {ITEM_ITEMFINDER, 0xFF}, {ITEM_OLD_ROD, 0xFF}, {ITEM_GOOD_ROD, 0xFF},
    {ITEM_SUPER_ROD, 0xFF}, {ITEM_SS_TICKET, 0xFF},
};

// Story items' home, for saves from before (what isn't listed: HOENN's, the save's own items)
static const struct { u16 item; u8 home; } sStoryHome[] = {
    {ITEM_POKE_FLUTE, REGION_KANTO}, {ITEM_SECRET_KEY, REGION_KANTO}, {ITEM_BIKE_VOUCHER, REGION_KANTO},
    {ITEM_GOLD_TEETH, REGION_KANTO}, {ITEM_OLD_AMBER, REGION_KANTO}, {ITEM_CARD_KEY, REGION_KANTO},
    {ITEM_LIFT_KEY, REGION_KANTO}, {ITEM_HELIX_FOSSIL, REGION_KANTO}, {ITEM_DOME_FOSSIL, REGION_KANTO},
    {ITEM_SILPH_SCOPE, REGION_KANTO}, {ITEM_TRI_PASS, REGION_KANTO}, {ITEM_RAINBOW_PASS, REGION_KANTO},
    {ITEM_TEA, REGION_KANTO}, {ITEM_POWDER_JAR, REGION_KANTO}, {ITEM_RUBY, REGION_KANTO},
    {ITEM_SAPPHIRE, REGION_KANTO}, {ITEM_PARCEL, REGION_KANTO},
    {ITEM_CLEAR_BELL, REGION_JOHTO}, {ITEM_TIDAL_BELL, REGION_JOHTO}, {ITEM_GS_BALL, REGION_JOHTO},
    {ITEM_SILVER_WING, REGION_JOHTO}, {ITEM_RAINBOW_WING, REGION_JOHTO}, {ITEM_RED_SCALE, REGION_JOHTO},
    {ITEM_SECRET_POTION, REGION_JOHTO}, {ITEM_MYSTERY_EGG, REGION_JOHTO}, {ITEM_SQUIRTBOTTLE, REGION_JOHTO},
    {ITEM_SPORT_BALL, REGION_JOHTO},
    {ITEM_GALACTIC_KEY, REGION_SINNOH}, {ITEM_AZURE_FLUTE, REGION_SINNOH}, {ITEM_WORKS_KEY, REGION_SINNOH},
    {ITEM_SUITE_KEY, REGION_SINNOH}, {ITEM_MEMBER_CARD, REGION_SINNOH}, {ITEM_OLD_CHARM, REGION_SINNOH},
    {ITEM_LUNAR_WING, REGION_SINNOH}, {ITEM_COUPON_1, REGION_SINNOH}, {ITEM_COUPON_2, REGION_SINNOH},
    {ITEM_COUPON_3, REGION_SINNOH}, {ITEM_ARMOR_FOSSIL, REGION_SINNOH}, {ITEM_SKULL_FOSSIL, REGION_SINNOH},
    {ITEM_GRACIDEA, REGION_SINNOH}, {ITEM_SINNOH_PARCEL, REGION_SINNOH},
};

enum { KI_STORY, KI_TRAVEL, KI_TOOL };

static u8 Kind(u16 item)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sTravel); i++)
        if (sTravel[i] == item)
            return KI_TRAVEL;
    if (IS_ITEM_HM(item))
        return KI_TOOL;
    for (i = 0; i < ARRAY_COUNT(sTools); i++)
        if (sTools[i].item == item)
            return KI_TOOL;
    return KI_STORY;
}

static u8 Home(u16 item)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sTools); i++)
        if (sTools[i].item == item)
            return sTools[i].home;
    for (i = 0; i < ARRAY_COUNT(sStoryHome); i++)
        if (sStoryHome[i].item == item)
            return sStoryHome[i].home;
    return REGION_HOENN;
}

// The region the player is in: the region state's (during a trip the map is still the last region's)
static u8 Here(void)
{
    u8 r = gRegionState.current < REGION_COUNT ? gRegionState.current : MP_Region_Current();
    return r < REGION_COUNT ? r : REGION_HOENN;
}

static bool8 IsFree(u16 item)
{
    u32 i;
    for (i = 0; i < FREE_SLOTS; i++)
        if (sKi.free[i] == item)
            return TRUE;
    return FALSE;
}

void MP_RegionItems_SetFree(u16 item)
{
    u32 i;
    if (item == ITEM_NONE || IsFree(item))
        return;
    for (i = 0; i < FREE_SLOTS; i++)
        if (sKi.free[i] == ITEM_NONE)
        {
            sKi.free[i] = item;
            MPNet_Log("region items: %u travels now", item);
            return;
        }
}

// The item goes along at customs (the rest of a KEY ITEMS pocket waits in the region's stash; HMs that don't, in the
// GLOBAL PC; every other item in the GLOBAL PC as before)
bool8 MP_RegionItems_Travels(u16 item)
{
    u8 k = Kind(item);
    if (k == KI_TRAVEL || IsFree(item))
        return TRUE;
    return k == KI_TOOL && MP_Region_Cleared(Here());
}

static void StashAdd(u8 region, u16 item, u16 qty)
{
    u32 i;
    for (i = 0; i < STASH_SLOTS; i++)
        if (sKi.stash[region][i].item == item)
        {
            sKi.stash[region][i].qty += qty;
            return;
        }
    for (i = 0; i < STASH_SLOTS; i++)
        if (sKi.stash[region][i].item == ITEM_NONE)
        {
            sKi.stash[region][i].item = item;
            sKi.stash[region][i].qty = qty;
            return;
        }
}

// Customs, leaving FROM (mp_region.c MP_Region_SendLuggage): the KEY ITEMS that don't travel into its stash. How many
u16 MP_RegionItems_Leave(u8 from)
{
    u16 items[64], n = 0, i, moved = 0;
    if (from >= REGION_COUNT)
        return 0;
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity && n < ARRAY_COUNT(items); i++)
    {
        u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
        if (item != ITEM_NONE && !MP_RegionItems_Travels(item))
            items[n++] = item;
    }
    for (i = 0; i < n; i++)
    {
        u16 qty = CountTotalItemQuantityInBag(items[i]);
        if (qty == 0)
            continue;
        StashAdd(from, items[i], qty);
        RemoveBagItem(items[i], qty);
        moved++;
    }
    if (moved)
        MPNet_Log("region items: %u key items wait in region %u", moved, from);
    return moved;
}

// Back in TO (mp_region.c MP_Region_Travel): its stash into the BAG (a key item the BAG has already stays one)
void MP_RegionItems_Arrive(u8 to)
{
    u32 i, back = 0;
    if (to >= REGION_COUNT)
        return;
    for (i = 0; i < STASH_SLOTS; i++)
    {
        u16 item = sKi.stash[to][i].item, qty = sKi.stash[to][i].qty;
        if (item == ITEM_NONE)
            continue;
        if (GetItemPocket(item) == POCKET_KEY_ITEMS && CheckBagHasItem(item, 1))
            qty = 0;
        if (qty && !AddBagItem(item, qty))
            continue;   // (no room: it waits on)
        sKi.stash[to][i].item = ITEM_NONE;
        sKi.stash[to][i].qty = 0;
        back++;
    }
    if (back)
        MPNet_Log("region items: %u key items back from region %u's stash", back, to);
}

// Customs sent an HM to the GLOBAL PC from FROM
void MP_RegionItems_NoteHm(u16 item, u8 from)
{
    u32 i;
    if (from >= REGION_COUNT)
        return;
    for (i = 0; i < HM_SLOTS; i++)
        if (sKi.hmFrom[i].item == item)
        {
            sKi.hmFrom[i].regions |= 1 << from;
            return;
        }
    for (i = 0; i < HM_SLOTS; i++)
        if (sKi.hmFrom[i].item == ITEM_NONE)
        {
            sKi.hmFrom[i].item = item;
            sKi.hmFrom[i].regions = 1 << from;
            return;
        }
}

static u8 HmRegions(u16 item)
{
    u32 i;
    for (i = 0; i < HM_SLOTS; i++)
        if (sKi.hmFrom[i].item == item)
            return sKi.hmFrom[i].regions;
    return 0;
}

static u8 ClearedMask(void)
{
    u8 r, m = 0;
    for (r = 0; r < REGION_COUNT; r++)
        if (MP_Region_Cleared(r))
            m |= 1 << r;
    return m;
}

// One of a free item: every copy in the stashes and the GLOBAL PC merged into the BAG's
static void Merge(u16 item)
{
    u8 r;
    u32 i;
    bool8 found = CheckBagHasItem(item, 1);
    for (r = 0; r < REGION_COUNT; r++)
        for (i = 0; i < STASH_SLOTS; i++)
            if (sKi.stash[r][i].item == item)
            {
                sKi.stash[r][i].item = ITEM_NONE;
                sKi.stash[r][i].qty = 0;
                found = TRUE;
            }
    if (MP_Region_GlobalItemTake(item))
        found = TRUE;
    for (i = 0; i < HM_SLOTS; i++)
        if (sKi.hmFrom[i].item == item)
            sKi.hmFrom[i].item = ITEM_NONE, sKi.hmFrom[i].regions = 0;
    if (found && !CheckBagHasItem(item, 1))
        AddBagItem(item, 1);
}

// Saves from before 1.183.0: every key item came along. Each goes to its home's stash unless it travels now (a
// tool from a beaten region, one every region has once any is beaten, Kai's own)
static void Migrate(void)
{
    u16 items[64], n = 0, i;
    u8 here = Here();
    // SINNOH's bike was the BICYCLE before: a save begun there that hasn't been to KANTO or JOHTO has the MTN. BIKE
    if (here == REGION_SINNOH && CheckBagHasItem(ITEM_BICYCLE, 1)
     && !(gRegionState.started & ((1 << REGION_KANTO) | (1 << REGION_JOHTO))))
    {
        RemoveBagItem(ITEM_BICYCLE, CountTotalItemQuantityInBag(ITEM_BICYCLE));
        AddBagItem(ITEM_MTN_BIKE, 1);
        if (gSaveBlock1Ptr->registeredItem == ITEM_BICYCLE)
            gSaveBlock1Ptr->registeredItem = ITEM_MTN_BIKE;
        MPNet_Log("region items: SINNOH's BICYCLE is the MTN. BIKE");
    }
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity && n < ARRAY_COUNT(items); i++)
        if (gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId != ITEM_NONE)
            items[n++] = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
    for (i = 0; i < n; i++)
    {
        u16 item = items[i], qty;
        u8 home = Home(item), k = Kind(item);
        if (k == KI_TRAVEL)
            continue;
        if (k == KI_TOOL && (home == 0xFF ? ClearedMask() != 0 : MP_Region_Cleared(home)))
        {
            MP_RegionItems_SetFree(item);
            continue;
        }
        if (home == 0xFF || home == here)
            continue;
        qty = CountTotalItemQuantityInBag(item);
        StashAdd(home, item, qty);
        RemoveBagItem(item, qty);
        MPNet_Log("region items: %u home to region %u (a save from before)", item, home);
    }
}

// On every map load (overworld.c through MP_Region_GiveDevices) and after travel: in a beaten region its tools and
// HMs travel for good; the GLOBAL PC's HMs from beaten regions (or, from before, any while here is beaten) come back;
// every free item is one, in the BAG
void MP_RegionItems_Update(void)
{
    u16 i;
    u8 here = Here(), cleared = ClearedMask();
    if (gSaveBlock1Ptr == NULL)
        return;
    if (sMigrate)
    {
        sMigrate = FALSE;
        Migrate();
    }
    if (cleared & (1 << here))
    {
        for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity; i++)
        {
            u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
            if (item != ITEM_NONE && Kind(item) == KI_TOOL)
                MP_RegionItems_SetFree(item);
        }
        for (i = 0; i < gBagPockets[TMHM_POCKET].capacity; i++)
        {
            u16 item = gBagPockets[TMHM_POCKET].itemSlots[i].itemId;
            if (IS_ITEM_HM(item))
                MP_RegionItems_SetFree(item);
        }
    }
    for (i = ITEM_HM01; i < ITEMS_COUNT; i++)
    {
        u8 from;
        if (!IS_ITEM_HM(i) || IsFree(i) || !MP_Region_GlobalItemHas(i))
            continue;
        from = HmRegions(i);
        if ((from & cleared) || (from == 0 && (cleared & (1 << here))))
            MP_RegionItems_SetFree(i);
    }
    for (i = 0; i < FREE_SLOTS; i++)
        if (sKi.free[i] != ITEM_NONE)
            Merge(sKi.free[i]);
}

// The key items waiting in REGION's stash (the test harness, the customs line)
u16 MP_RegionItems_StashCount(u8 region)
{
    u32 i, n = 0;
    for (i = 0; region < REGION_COUNT && i < STASH_SLOTS; i++)
        n += sKi.stash[region][i].item != ITEM_NONE;
    return n;
}

// The BAG has key items that would wait here (customs' second line)
u16 MP_RegionKeyItemsStay(void)
{
    u32 i;
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity; i++)
    {
        u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
        if (item != ITEM_NONE && !MP_RegionItems_Travels(item))
            return gSpecialVar_Result = TRUE;
    }
    return gSpecialVar_Result = FALSE;
}

// The region side file (mp_region.c): chunk "KIST"
void MP_RegionItems_Reset(void)
{
    memset(&sKi, 0, sizeof(sKi));
    sKi.version = KIST_VERSION;
    sMigrate = TRUE;
}

const void *MP_RegionItems_Save(u32 *size)
{
    sKi.version = KIST_VERSION;
    *size = sizeof(sKi);
    return &sKi;
}

void MP_RegionItems_Load(const void *data, u32 size)
{
    memset(&sKi, 0, sizeof(sKi));
    memcpy(&sKi, data, size < sizeof(sKi) ? size : sizeof(sKi));
    sKi.version = KIST_VERSION;
    sMigrate = FALSE;
}

// A new game: nothing to sort out
void MP_RegionItems_NewGame(void)
{
    MP_RegionItems_Reset();
    sMigrate = FALSE;
}
