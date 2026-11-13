// Moemon Kai: key items stay in their region (the user, 2026-11-08 and 2026-11-13; docs/REGION_ITEMS.md). "I want the
// player to need to earn every tool legitimately for the region they're in. But in the POSTGAME, they get to use every
// key item." Every copy of a key item is keyed by the region it was earned in (the GOOD ROD (HOENN) and the GOOD ROD
// (KANTO) are two): leaving a region, every key item but Kai's own (the CARD CASE, the CANDY JAR, the BATTLE FLAG, the
// boats' tickets) goes to the GLOBAL PC under its key; arriving, the copies keyed to that region come back. In a region
// whose LEAGUE the player has beaten, every tool copy from every region is in the BAG as one item (each copy goes back
// to the GLOBAL PC under its own key on leaving). In a region whose LEAGUE isn't beaten, a copy keyed to another region
// goes to the GLOBAL PC by itself (on every map load): until 1.199.2 a tool from a beaten region crossed with the
// player (a champion of HOENN landed in KANTO with its bikes and rods), and those saves are sorted so. HMs wait in the
// GLOBAL PC (customs sends them there with the rest of the BAG) and merge into the BAG in a beaten region. Kept in the
// region side file (mp_region.c chunk "KIST": the keyed GLOBAL PC is its "stash").
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mp.h"
#include "mp_region.h"
#include "constants/items.h"
#include "constants/qz_port.h"
#include "constants/kai_regions.h"

#define STASH_SLOTS 64
#define FREE_SLOTS 64
#define HM_SLOTS 16
#define HELD_SLOTS 96
#define KIST_VERSION 3   // (3: the BAG's copies keyed, held[]; 2: UNOVA's region; version 1 had four regions' stashes,
                         // struct RegionItems4: converted)

struct RegionItems
{
    u32 version;
    struct { u16 item, qty; } stash[REGION_COUNT][STASH_SLOTS];   // the GLOBAL PC's key items, by the region earned in
    u16 free[FREE_SLOTS];                                         // (before 1.199.2: tools that travelled. Unused)
    struct { u16 item; u8 regions, pad; } hmFrom[HM_SLOTS];       // where the GLOBAL PC's HMs came from
    struct { u16 item; u8 region, pad; } held[HELD_SLOTS];        // the copies the BAG's key items are, keyed
};
struct RegionItems4
{
    u32 version;
    struct { u16 item, qty; } stash[REGION_COUNT_BEFORE_UNOVA][STASH_SLOTS];
    u16 free[FREE_SLOTS];
    struct { u16 item; u8 regions, pad; } hmFrom[HM_SLOTS];
};
static struct RegionItems sKi;
static bool8 sSort;   // a save from before 1.199.2 (or 1.183.0): its BAG's key items keyed once by where they belong

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
    {ITEM_XTRANSCEIVER, REGION_UNOVA}, {ITEM_PROP_CASE, REGION_UNOVA},
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
    {ITEM_GRAM_1, REGION_UNOVA}, {ITEM_GRAM_2, REGION_UNOVA}, {ITEM_GRAM_3, REGION_UNOVA},
    {ITEM_LIBERTY_PASS, REGION_UNOVA}, {ITEM_PASS_ORB, REGION_UNOVA}, {ITEM_DRAGON_SKULL, REGION_UNOVA},
    {ITEM_LIGHT_STONE, REGION_UNOVA}, {ITEM_DARK_STONE, REGION_UNOVA}, {ITEM_COVER_FOSSIL, REGION_UNOVA},
    {ITEM_PLUME_FOSSIL, REGION_UNOVA},
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

static bool8 Kept(u16 item)
{
    return item == ITEM_NONE || IS_ITEM_HM(item) || Kind(item) == KI_TRAVEL;
}

// The item goes along at customs: Kai's own only (the user, 2026-11-13). An HM never goes along: customs puts it in
// the GLOBAL PC, and it's back once the region the player is in is beaten (the user, 2026-11-10)
bool8 MP_RegionItems_Travels(u16 item)
{
    return Kind(item) == KI_TRAVEL;
}

static void StashAdd(u8 region, u16 item, u16 qty)
{
    u32 i;
    if (region >= REGION_COUNT || item == ITEM_NONE)
        return;
    if (Kind(item) == KI_TOOL)
        qty = 1;   // (one copy a region)
    for (i = 0; i < STASH_SLOTS; i++)
        if (sKi.stash[region][i].item == item)
        {
            if (Kind(item) != KI_TOOL)
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
    MPNet_Log("region items: region %u's shelf is full, %u lost", region, item);
}

static bool8 Held(u16 item, u8 region)
{
    u32 i;
    for (i = 0; i < HELD_SLOTS; i++)
        if (sKi.held[i].item == item && (region == 0xFF || sKi.held[i].region == region))
            return TRUE;
    return FALSE;
}

static void HeldAdd(u16 item, u8 region)
{
    u32 i;
    if (Held(item, region))
        return;
    for (i = 0; i < HELD_SLOTS; i++)
        if (sKi.held[i].item == ITEM_NONE)
        {
            sKi.held[i].item = item;
            sKi.held[i].region = region;
            return;
        }
}

// Customs, leaving FROM (mp_region.c MP_Region_SendLuggage, and every crossing: MP_Region_Travel): every key item but
// Kai's own to the GLOBAL PC, each copy under its key (one the BAG has no key for was earned in FROM). How many
u16 MP_RegionItems_Leave(u8 from)
{
    u16 items[64], n = 0, i, moved = 0;
    u32 j;
    if (from >= REGION_COUNT)
        return 0;
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity && n < ARRAY_COUNT(items); i++)
    {
        u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
        if (!Kept(item))
            items[n++] = item;
    }
    for (i = 0; i < n; i++)
    {
        u16 qty = CountTotalItemQuantityInBag(items[i]), copies = 0;
        if (qty == 0)
            continue;
        for (j = 0; j < HELD_SLOTS; j++)
            if (sKi.held[j].item == items[i])
            {
                StashAdd(sKi.held[j].region, items[i], copies == 0 ? qty : 1);
                sKi.held[j].item = ITEM_NONE;
                copies++;
            }
        if (copies == 0)
            StashAdd(from, items[i], qty);
        RemoveBagItem(items[i], qty);
        if (gSaveBlock1Ptr->registeredItem == items[i])
            gSaveBlock1Ptr->registeredItem = ITEM_NONE;
        moved++;
    }
    for (j = 0; j < HELD_SLOTS; j++)
        if (!CheckBagHasItem(sKi.held[j].item, 1))
            sKi.held[j].item = ITEM_NONE;   // (Kai's own and HMs are never keyed)
    if (moved)
        MPNet_Log("region items: %u key items to the GLOBAL PC from region %u", moved, from);
    return moved;
}

// Back in TO (mp_region.c MP_Region_Travel): the GLOBAL PC's copies keyed to it into the BAG
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
        if (CheckBagHasItem(item, 1) && Kind(item) == KI_TOOL)
            qty = 0;
        if (qty && !AddBagItem(item, qty))
            continue;   // (no room: it waits on)
        HeldAdd(item, to);
        sKi.stash[to][i].item = ITEM_NONE;
        sKi.stash[to][i].qty = 0;
        back++;
    }
    if (back)
        MPNet_Log("region items: %u key items back from the GLOBAL PC (region %u's)", back, to);
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

// An HM out of the GLOBAL PC into the BAG, merged with the BAG's own into one
static void MergeHm(u16 item)
{
    u32 i;
    bool8 found = CheckBagHasItem(item, 1);
    if (MP_Region_GlobalItemTake(item))
        found = TRUE;
    for (i = 0; i < HM_SLOTS; i++)
        if (sKi.hmFrom[i].item == item)
            sKi.hmFrom[i].item = ITEM_NONE, sKi.hmFrom[i].regions = 0;
    if (found && !CheckBagHasItem(item, 1))
        AddBagItem(item, 1);
}

// A tool every region has: whether the region HERE's own giver has handed it over (the player may have it from here).
// KANTO's and JOHTO's people check the BAG first (the S.S. TICKET's, the fishermen's...), so a carried one may have
// set the flag instead; that copy counts as the region's. SINNOH's and UNOVA's give theirs by their own flags in
// Platinum's / Black's scripts: what's in the BAG there is kept
static bool8 GotHere(u16 item, u8 here)
{
    static const struct { u16 item, flag; } sKanto[] = {
        {ITEM_OLD_ROD, FLAG_QZ_0x215B}, {ITEM_GOOD_ROD, FLAG_QZ_0x214B}, {ITEM_SUPER_ROD, FLAG_QZ_0x2164},
        {ITEM_ITEMFINDER, FLAG_QZ_0x2154}, {ITEM_COIN_CASE, FLAG_QZ_0x2143}, {ITEM_BICYCLE, FLAG_QZ_0x2141},
        {ITEM_VS_SEEKER, FLAG_QZ_0x2176}, {ITEM_SS_TICKET, FLAG_QZ_0x2163},
    };
    u32 i;
    if (here == REGION_SINNOH || here == REGION_UNOVA)
        return TRUE;
    if (here == REGION_KANTO)
    {
        for (i = 0; i < ARRAY_COUNT(sKanto); i++)
            if (sKanto[i].item == item)
                return FlagGet(sKanto[i].flag);
        return FALSE;
    }
    if (here == REGION_JOHTO)
        return item == ITEM_ITEMFINDER && FlagGet(FLAG_JOHTO_GOT_ITEMFINDER);   // (the rest: given again on asking)
    return FALSE;
}

// The region a crossed tool came from: its own, or for one every region has, a beaten region (HOENN first)
static u8 CameFrom(u16 item, u8 here)
{
    u8 home = Home(item), r;
    if (home != 0xFF && home < REGION_COUNT)
        return home;
    if (here != REGION_HOENN && MP_Region_Cleared(REGION_HOENN))
        return REGION_HOENN;
    for (r = 0; r < REGION_COUNT; r++)
        if (r != here && MP_Region_Cleared(r))
            return r;
    return REGION_HOENN;
}

// Where a key item in the BAG of a save from before 1.199.2 was earned: a regional item's own region; one every region
// has, here if HERE's giver handed it over, else where it came from; a story item, its home
static u8 KeyFor(u16 item, u8 here)
{
    u8 home = Home(item);
    if (home < REGION_COUNT)
        return home;
    return GotHere(item, here) ? here : CameFrom(item, here);
}

// A save from before 1.199.2 (the user, 2026-11-13: "a hotfix that sends them to the GLOBAL PC"): each key item in the
// BAG keyed where it belongs; in a region whose LEAGUE isn't beaten, what belongs elsewhere goes to the GLOBAL PC
// (Update, right after). A save begun in SINNOH that hasn't seen KANTO or JOHTO has its BICYCLE as the MTN. BIKE
static void Sort(void)
{
    u16 i;
    u8 here = Here();
    if (here == REGION_SINNOH && CheckBagHasItem(ITEM_BICYCLE, 1)
     && !(gRegionState.started & ((1 << REGION_KANTO) | (1 << REGION_JOHTO))))
    {
        RemoveBagItem(ITEM_BICYCLE, CountTotalItemQuantityInBag(ITEM_BICYCLE));
        AddBagItem(ITEM_MTN_BIKE, 1);
        if (gSaveBlock1Ptr->registeredItem == ITEM_BICYCLE)
            gSaveBlock1Ptr->registeredItem = ITEM_MTN_BIKE;
        MPNet_Log("region items: SINNOH's BICYCLE is the MTN. BIKE");
    }
    memset(sKi.held, 0, sizeof(sKi.held));
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity; i++)
    {
        u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
        if (!Kept(item))
            HeldAdd(item, KeyFor(item, here));
    }
    memset(sKi.free, 0, sizeof(sKi.free));
    MPNet_Log("region items: a save from before: the BAG's key items keyed");
}

// On every map load (overworld.c through MP_Region_GiveDevices) and after travel
void MP_RegionItems_Update(void)
{
    u16 i, j;
    u8 here = Here();
    bool8 beaten;
    if (gSaveBlock1Ptr == NULL)
        return;
    if (sSort)
    {
        sSort = FALSE;
        Sort();
    }
    beaten = MP_Region_Cleared(here);
    // what the BAG has without a key was earned here
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity; i++)
    {
        u16 item = gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId;
        if (!Kept(item) && !Held(item, 0xFF))
            HeldAdd(item, here);
    }
    if (!beaten)
    {
        // the LEAGUE here isn't beaten: only what was earned here. A copy keyed elsewhere goes to the GLOBAL PC
        for (i = 0; i < HELD_SLOTS; i++)
        {
            u16 item = sKi.held[i].item;
            u8 r = sKi.held[i].region;
            if (item == ITEM_NONE || r == here)
                continue;
            sKi.held[i].item = ITEM_NONE;
            if (!Held(item, here))
            {
                u16 qty = CountTotalItemQuantityInBag(item);
                StashAdd(r, item, qty ? qty : 1);
                RemoveBagItem(item, qty);
                if (gSaveBlock1Ptr->registeredItem == item)
                    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
            }
            else
                StashAdd(r, item, 1);
            MPNet_Log("region items: %u (region %u's) to the GLOBAL PC: region %u isn't beaten", item, r, here);
        }
        return;
    }
    // the postgame here: every tool copy from every region is in the BAG, as one item
    for (j = 0; j < REGION_COUNT; j++)
        for (i = 0; i < STASH_SLOTS; i++)
        {
            u16 item = sKi.stash[j][i].item;
            if (item == ITEM_NONE || Kind(item) != KI_TOOL || IS_ITEM_HM(item))
                continue;
            if (!CheckBagHasItem(item, 1) && !AddBagItem(item, 1))
                continue;
            HeldAdd(item, j);
            sKi.stash[j][i].item = ITEM_NONE;
            sKi.stash[j][i].qty = 0;
        }
    // (the GLOBAL PC's HMs: back, each merged with the BAG's own into one)
    for (i = ITEM_HM01; i < ITEMS_COUNT; i++)
        if (IS_ITEM_HM(i) && MP_Region_GlobalItemHas(i))
        {
            MergeHm(i);
            MPNet_Log("region items: HM %u back from the GLOBAL PC (region %u is beaten)", i, here);
        }
}

// The key items waiting in the GLOBAL PC keyed to REGION (the test harness)
u16 MP_RegionItems_StashCount(u8 region)
{
    u32 i, n = 0;
    for (i = 0; region < REGION_COUNT && i < STASH_SLOTS; i++)
        n += sKi.stash[region][i].item != ITEM_NONE;
    return n;
}

// The regions whose copy of ITEM waits in the GLOBAL PC, as bits (the test harness)
u8 MP_RegionItems_KeyedPc(u16 item)
{
    u32 r, i;
    u8 m = 0;
    for (r = 0; r < REGION_COUNT; r++)
        for (i = 0; i < STASH_SLOTS; i++)
            if (sKi.stash[r][i].item == item)
                m |= 1 << r;
    return m;
}

// The BAG has key items that would wait here (customs' second line)
u16 MP_RegionKeyItemsStay(void)
{
    u32 i;
    for (i = 0; i < gBagPockets[KEYITEMS_POCKET].capacity; i++)
        if (!Kept(gBagPockets[KEYITEMS_POCKET].itemSlots[i].itemId))
            return gSpecialVar_Result = TRUE;
    return gSpecialVar_Result = FALSE;
}

// The region side file (mp_region.c): chunk "KIST". No record: a save from before 1.183.0, sorted once
void MP_RegionItems_Reset(void)
{
    memset(&sKi, 0, sizeof(sKi));
    sKi.version = KIST_VERSION;
    sSort = TRUE;
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
    if (size >= 4 && *(const u32 *)data == 1)
    {
        // a record from before UNOVA: four regions' stashes, then the rest
        struct RegionItems4 old;
        memset(&old, 0, sizeof(old));
        memcpy(&old, data, size < sizeof(old) ? size : sizeof(old));
        memcpy(sKi.stash, old.stash, sizeof(old.stash));
        memcpy(sKi.hmFrom, old.hmFrom, sizeof(old.hmFrom));
    }
    else
        memcpy(&sKi, data, size < sizeof(sKi) ? size : sizeof(sKi));
    sSort = size < 4 || *(const u32 *)data < 3;   // (from before 1.199.2: nothing keyed, tools may have crossed)
    sKi.version = KIST_VERSION;
}

// The harness: as a record from before 1.199.2 (sorted on the next map load)
void MP_RegionItems_DebugOld(void)
{
    sSort = TRUE;
}

// A new game: nothing to sort out
void MP_RegionItems_NewGame(void)
{
    MP_RegionItems_Reset();
    sSort = FALSE;
}
