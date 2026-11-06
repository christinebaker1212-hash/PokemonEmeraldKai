#include "mp_version.h"
#include "aw_file.h"   // (sidecars written in the background: src/platform/aw_file.c)
// Moemon Co-op: a bigger BAG (PC only).
//
// The pockets hold more kinds of items than the GBA's save block has room for
// (MP_BAG_*_COUNT, include/constants/global.h) and a stack goes up to 999
// (MP_BAG_ITEM_CAPACITY). gBagPockets point at the arrays here; the first slots of each
// pocket are also the save block's own pocket (copied there before every save write,
// so the save file stays a normal Emerald save), and the slots past them are written
// next to it in "<save>.bag" along with the save's counter. Loading takes the save
// block's slots and, when "<save>.bag" was written with that same save (counter and
// the save block's slots match), the rest. The save backups keep theirs as
// "<save>.bakN.bag" (sdl2.c BackUpSaveFile), so a .bak file renamed back to the save
// finds its own: every one of them is tried. A save played somewhere else in between
// just has the save block's pocket.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "string_util.h"
#include "mp.h"
#include "constants/items.h"

extern char gPlatformSavePath[512];
extern u32 gSaveCounter;

static struct ItemSlot sItems[MP_BAG_ITEMS_COUNT];
static struct ItemSlot sKeyItems[MP_BAG_KEYITEMS_COUNT];
static struct ItemSlot sPokeBalls[MP_BAG_POKEBALLS_COUNT];
static struct ItemSlot sTMHM[MP_BAG_TMHM_COUNT];
static struct ItemSlot sBerries[MP_BAG_BERRIES_COUNT];

struct PocketDef
{
    struct ItemSlot *big;
    u8 bigCount;
    u16 saveOffset;   // the save block's pocket
    u8 saveCount;
};

static const struct PocketDef sPockets[POCKETS_COUNT] = {
    [ITEMS_POCKET]    = {sItems, MP_BAG_ITEMS_COUNT, offsetof(struct SaveBlock1, bagPocket_Items), BAG_ITEMS_COUNT},
    [BALLS_POCKET]    = {sPokeBalls, MP_BAG_POKEBALLS_COUNT, offsetof(struct SaveBlock1, bagPocket_PokeBalls), BAG_POKEBALLS_COUNT},
    [TMHM_POCKET]     = {sTMHM, MP_BAG_TMHM_COUNT, offsetof(struct SaveBlock1, bagPocket_TMHM), BAG_TMHM_COUNT},
    [BERRIES_POCKET]  = {sBerries, MP_BAG_BERRIES_COUNT, offsetof(struct SaveBlock1, bagPocket_Berries), BAG_BERRIES_COUNT},
    [KEYITEMS_POCKET] = {sKeyItems, MP_BAG_KEYITEMS_COUNT, offsetof(struct SaveBlock1, bagPocket_KeyItems), BAG_KEYITEMS_COUNT},
};

static struct ItemSlot *SaveSlots(u8 pocket)
{
    return (struct ItemSlot *)((u8 *)gSaveBlock1Ptr + sPockets[pocket].saveOffset);
}

// item.c SetBagItemsPointers: the pockets are ours (their contents stay where they are)
void MP_Bag_SetPointers(struct BagPocket *pockets)
{
    u8 p;
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        pockets[p].itemSlots = sPockets[p].big;
        pockets[p].capacity = sPockets[p].bigCount;
    }
}

// Before the save block goes to the save file: its pockets get our first slots
void MP_Bag_SyncToSaveBlock(void)
{
    u8 p;
    if (gSaveBlock1Ptr == NULL)
        return;
    for (p = 0; p < POCKETS_COUNT; p++)
        memcpy(SaveSlots(p), sPockets[p].big, sPockets[p].saveCount * sizeof(struct ItemSlot));
}

static void MarksFromCurrentOrder(void);
static bool8 ReadMarks(FILE *f);
static void WriteMarks(AwFile *f);

#define BAG_MAGIC 0x4D424147   // "MBAG"

static void SidecarPath(char *out, size_t size)
{
    snprintf(out, size, "%s.bag", gPlatformSavePath);
}

static u32 Checksum(const void *data, u32 size)
{
    const u8 *b = data;
    u32 sum = 2166136261u, i;
    for (i = 0; i < size; i++)
        sum = (sum ^ b[i]) * 16777619u;
    return sum;
}

static u32 SaveBlockPocketsSum(void)
{
    u32 sum = 0;
    u8 p;
    for (p = 0; p < POCKETS_COUNT; p++)
        sum = sum * 31 + Checksum(SaveSlots(p), sPockets[p].saveCount * sizeof(struct ItemSlot));
    return sum;
}

// After the save file was written (platform StoreSaveFile): the slots past the save block's
void MP_Bag_WriteSidecar(void)
{
    if (MOEMON_DEV_BUILD)
        return;
    char path[600];
    AwFile *f;
    u32 header[3];
    u8 p;
    if (gSaveBlock1Ptr == NULL)
        return;
    SidecarPath(path, sizeof(path));
    f = AwFile_Open(path);
    if (!f)
        return;
    header[0] = BAG_MAGIC;
    header[1] = gSaveCounter;
    header[2] = SaveBlockPocketsSum();
    AwFile_Write(header, sizeof(header), 1, f);
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        u8 n = sPockets[p].bigCount - sPockets[p].saveCount;
        AwFile_Write(&n, 1, 1, f);
        AwFile_Write(sPockets[p].big + sPockets[p].saveCount, sizeof(struct ItemSlot), n, f);
    }
    WriteMarks(f);   // the items' "new" marks and when they came in
    MP_Candy_Write(f);   // the CANDY JAR's candies (mp_candy.c)
    MP_DexExt_Write(f);  // Moemon Kai: dex flags past the save block's (pokedex.c)
    AwFile_Close(f);
}

// The extra slots from one sidecar file, if it was written with this very save
static bool8 LoadSidecar(const char *path)
{
    FILE *f;
    u32 header[3];
    u8 p;
    bool8 sidecar = FALSE;

    f = fopen(path, "rb");
    if (!f)
        return FALSE;
    if (fread(header, sizeof(header), 1, f) == 1 && header[0] == BAG_MAGIC
     && header[1] == gSaveCounter && header[2] == SaveBlockPocketsSum())
    {
        sidecar = TRUE;
        for (p = 0; p < POCKETS_COUNT; p++)
        {
            u8 n = 0, max = sPockets[p].bigCount - sPockets[p].saveCount;
            if (fread(&n, 1, 1, f) != 1)
                break;
            if (n > max)
            {
                // written by a build with bigger pockets: keep what fits
                fread(sPockets[p].big + sPockets[p].saveCount, sizeof(struct ItemSlot), max, f);
                fseek(f, (long)(n - max) * sizeof(struct ItemSlot), SEEK_CUR);
            }
            else
            {
                fread(sPockets[p].big + sPockets[p].saveCount, sizeof(struct ItemSlot), n, f);
            }
        }
        if (p != POCKETS_COUNT || !ReadMarks(f))
            MarksFromCurrentOrder();   // a sidecar from before the marks
        else
        {
            MP_Candy_Read(f);
            MP_DexExt_Read(f);   // Moemon Kai
        }
    }
    fclose(f);
    return sidecar;
}

// After a save was loaded: the save block's slots, then the rest from "<save>.bag" (or the
// backup's own, for a .bak file renamed back to the save)
void MP_Bag_LoadFromSave(void)
{
    char path[600];
    u8 p, i;
    bool8 sidecar = FALSE;

    MP_Candy_Reset();   // unless the sidecar has them
    MP_DexExt_Reset();
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        memset(sPockets[p].big, 0, sPockets[p].bigCount * sizeof(struct ItemSlot));
        memcpy(sPockets[p].big, SaveSlots(p), sPockets[p].saveCount * sizeof(struct ItemSlot));
    }
    for (i = 0; i <= 3 && !sidecar; i++)
    {
        if (i == 0)
            SidecarPath(path, sizeof(path));
        else
            snprintf(path, sizeof(path), "%s.bak%d.bag", gPlatformSavePath, i);
        sidecar = LoadSidecar(path);
    }
    if (!sidecar)
        MarksFromCurrentOrder();
    MPNet_Log("bag: loaded%s", sidecar ? " with the extra slots" : " (extra slots not from this save)");
}

// ---------------------------------------------------------------------------
// Sorting the pockets and the "!" on new items.
//
// Each pocket has a sort order (START in the BAG): TYPE, ID, ABC, OLDEST,
// NEWEST or MANUAL (the player's own order, moved with SELECT). The pocket is
// put in that order whenever the BAG shows it. Items keep the time they first
// came into the BAG (a running number) and a "new" mark that goes once the
// cursor is on them; both are written to "<save>.bag" with the extra slots.

#include "item_use.h"
#include "mail.h"
#include "string_util.h"

#define MARKS_MAGIC 0x4D4B5253   // "MKRS"

static u16 sObtained[ITEMS_COUNT];   // 0 = had before the marks were kept
static u8 sNewMark[(ITEMS_COUNT + 7) / 8];
static u16 sNextObtained = 1;

bool8 MP_Bag_IsNew(u16 item)
{
    return item < ITEMS_COUNT && (sNewMark[item / 8] & (1 << (item % 8)));
}

void MP_Bag_ClearNew(u16 item)
{
    if (item < ITEMS_COUNT)
        sNewMark[item / 8] &= ~(1 << (item % 8));
}

// item.c AddBagItem: an item that wasn't in the BAG came in
void MP_Bag_NoteAdded(u16 item)
{
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return;
    if (sNextObtained < 0xFFFF)
        sObtained[item] = sNextObtained++;
    sNewMark[item / 8] |= 1 << (item % 8);
}

// A new game (item.c ClearBag)
void MP_Bag_ResetMarks(void)
{
    memset(sObtained, 0, sizeof(sObtained));
    memset(sNewMark, 0, sizeof(sNewMark));
    sNextObtained = 1;
}

// A save from before the marks: what's there counts as obtained in the order it's in
static void MarksFromCurrentOrder(void)
{
    u8 p, i;
    MP_Bag_ResetMarks();
    for (p = 0; p < POCKETS_COUNT; p++)
        for (i = 0; i < sPockets[p].bigCount; i++)
        {
            u16 item = sPockets[p].big[i].itemId;
            if (item != ITEM_NONE && item < ITEMS_COUNT && sObtained[item] == 0)
                sObtained[item] = sNextObtained++;
        }
}

static void WriteMarks(AwFile *f)
{
    u32 magic = MARKS_MAGIC;
    u16 count = ITEMS_COUNT;
    AwFile_Write(&magic, sizeof(magic), 1, f);
    AwFile_Write(&count, sizeof(count), 1, f);
    AwFile_Write(&sNextObtained, sizeof(sNextObtained), 1, f);
    AwFile_Write(sObtained, sizeof(sObtained[0]), ITEMS_COUNT, f);
    AwFile_Write(sNewMark, 1, sizeof(sNewMark), f);
}

static bool8 ReadMarks(FILE *f)
{
    u32 magic = 0;
    u16 count = 0, i, v;
    u8 b;
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != MARKS_MAGIC || fread(&count, sizeof(count), 1, f) != 1
     || fread(&sNextObtained, sizeof(sNextObtained), 1, f) != 1)
        return FALSE;
    // written by a build with another item count: keep the items both know
    for (i = 0; i < count; i++)
    {
        if (fread(&v, sizeof(v), 1, f) != 1)
            return FALSE;
        if (i < ITEMS_COUNT)
            sObtained[i] = v;
    }
    for (i = 0; i < (count + 7) / 8; i++)
    {
        if (fread(&b, 1, 1, f) != 1)
            return FALSE;
        if (i < sizeof(sNewMark))
            sNewMark[i] = b;
    }
    return TRUE;
}

// Sort orders (stored per pocket; 0 = the pocket's usual one)
#define SORT_KEY "bag:sort%d"

static u8 DefaultSort(u8 pocket)
{
    // TMs and Berries are numbered: by number, as in the original game
    return (pocket == TMHM_POCKET || pocket == BERRIES_POCKET) ? MP_BAGSORT_ID : MP_BAGSORT_MANUAL;
}

// (1.63.14: pocket POCKETS_COUNT is FAVORITES' own order, MANUAL unless one is chosen)
u8 MP_Bag_SortMode(u8 pocket)
{
    char key[16];
    s32 v;
    if (pocket > POCKETS_COUNT)
        return MP_BAGSORT_MANUAL;
    snprintf(key, sizeof(key), SORT_KEY, pocket);
    v = MP_SaveData_Get(key);
    return (v >= 1 && v <= MP_BAGSORT_COUNT) ? v - 1 : pocket == POCKETS_COUNT ? MP_BAGSORT_MANUAL : DefaultSort(pocket);
}

void MP_Bag_SetSortMode(u8 pocket, u8 mode)
{
    char key[16];
    if (pocket > POCKETS_COUNT || mode >= MP_BAGSORT_COUNT)
        return;
    snprintf(key, sizeof(key), SORT_KEY, pocket);
    MP_SaveData_Set(key, mode + 1);
}

// TYPE: medicine, things used on a Moemon (vitamins, stones...), battle items,
// field items (REPEL...), hold items, mail, the rest
static u8 ItemCategory(u16 item)
{
    if (GetItemBattleUsage(item) == ITEM_B_USE_MEDICINE)
        return 0;
    if (GetItemType(item) == ITEM_USE_PARTY_MENU && GetItemFieldFunc(item) != ItemUseOutOfBattle_CannotUse)
        return 1;
    if (GetItemBattleUsage(item) != 0)
        return 2;
    if (GetItemFieldFunc(item) != NULL && GetItemFieldFunc(item) != ItemUseOutOfBattle_CannotUse)
        return 3;
    if (GetItemHoldEffect(item) != 0)
        return 4;
    if (ItemIsMail(item))
        return 5;
    return 6;
}

// < 0: a goes first
static int CompareItems(u16 a, u16 b, u8 mode)
{
    int d = 0;
    switch (mode)
    {
    case MP_BAGSORT_TYPE:
        d = ItemCategory(a) - ItemCategory(b);
        break;
    case MP_BAGSORT_ABC:
    {
        u8 na[ITEM_NAME_LENGTH + 12], nb[ITEM_NAME_LENGTH + 12];
        CopyItemName(a, na);
        CopyItemName(b, nb);
        d = StringCompare(na, nb);
        break;
    }
    case MP_BAGSORT_OLDEST:
        d = (int)sObtained[a < ITEMS_COUNT ? a : 0] - (int)sObtained[b < ITEMS_COUNT ? b : 0];
        break;
    case MP_BAGSORT_NEWEST:
        d = (int)sObtained[b < ITEMS_COUNT ? b : 0] - (int)sObtained[a < ITEMS_COUNT ? a : 0];
        break;
    }
    return d ? d : (int)MP_ItemNumberOrder(a) - (int)MP_ItemNumberOrder(b);   // then by number (ID: only that; TM51 after TM50)
}

// Puts a pocket in its sort order (the empty slots stay at the end). Stable,
// so two stacks of one item stay in their order.
void MP_Bag_Sort(u8 pocket)
{
    struct ItemSlot *slots;
    u8 mode = MP_Bag_SortMode(pocket), n = 0, i, j;
    if (pocket >= POCKETS_COUNT || mode == MP_BAGSORT_MANUAL)
        return;
    slots = sPockets[pocket].big;
    while (n < sPockets[pocket].bigCount && slots[n].itemId != ITEM_NONE)
        n++;
    for (i = 1; i < n; i++)
    {
        struct ItemSlot cur = slots[i];
        j = i;
        while (j > 0 && CompareItems(slots[j - 1].itemId, cur.itemId, mode) > 0)
        {
            slots[j] = slots[j - 1];
            j--;
        }
        slots[j] = cur;
    }
}

// ---------------------------------------------------------------------------
// The BAG's eight pockets (HGSS's: ITEMS, MEDICINE, POKé BALLS, TMs & HMs, BERRIES, BATTLE ITEMS, KEY ITEMS,
// FAVORITES). They're views of the save's five pockets: MEDICINE and BATTLE ITEMS are the ITEMS pocket's medicine and
// battle-only items, ITEMS the rest of it; FAVORITES the items the player marked, from any pocket. The save keeps its
// five pockets as they are (item_menu.c shows the views).

// Which pocket an item shows in (never FAVORITES: that's as well)
u8 MP_Bag_ViewOfItem(u16 item)
{
    ItemUseFunc f;
    switch (GetItemPocket(item))
    {
    case POCKET_POKE_BALLS: return MP_BAGVIEW_BALLS;
    case POCKET_TM_HM:      return MP_BAGVIEW_TMHM;
    case POCKET_BERRIES:    return MP_BAGVIEW_BERRIES;
    case POCKET_KEY_ITEMS:  return MP_BAGVIEW_KEY;
    }
    f = GetItemFieldFunc(item);
    if (GetItemBattleUsage(item) == ITEM_B_USE_MEDICINE || f == ItemUseOutOfBattle_Medicine || f == ItemUseOutOfBattle_PPUp
     || f == ItemUseOutOfBattle_PPRecovery || f == ItemUseOutOfBattle_RareCandy || f == ItemUseOutOfBattle_SacredAsh)
        return MP_BAGVIEW_MEDICINE;
    if (GetItemBattleUsage(item) == ITEM_B_USE_OTHER)
        return MP_BAGVIEW_BATTLE;
    return MP_BAGVIEW_ITEMS;
}

// special MP_BufferItemPocket: STR_VAR_3 = the BAG pocket item VAR_0x8000 shows in (JOHTO's and KANTO's "put away"
// messages: Quetzal's scripts named its own pockets by its own numbers, so POKe BALLs read MEDICINE; 1.63.11)
void MP_BufferItemPocket(void)
{
    StringCopy(gStringVar3, MP_Bag_ViewName(MP_Bag_ViewOfItem(gSpecialVar_0x8000)));
}

// The save's pocket a view shows (FAVORITES: POCKETS_COUNT, all of them)
u8 MP_Bag_ViewPocket(u8 view)
{
    static const u8 sPocketOf[MP_BAGVIEW_COUNT] = {
        [MP_BAGVIEW_ITEMS] = ITEMS_POCKET, [MP_BAGVIEW_MEDICINE] = ITEMS_POCKET, [MP_BAGVIEW_BALLS] = BALLS_POCKET,
        [MP_BAGVIEW_TMHM] = TMHM_POCKET, [MP_BAGVIEW_BERRIES] = BERRIES_POCKET, [MP_BAGVIEW_BATTLE] = ITEMS_POCKET,
        [MP_BAGVIEW_KEY] = KEYITEMS_POCKET, [MP_BAGVIEW_FAVORITES] = POCKETS_COUNT,
    };
    return view < MP_BAGVIEW_COUNT ? sPocketOf[view] : ITEMS_POCKET;
}

// ... and the view a save pocket opens on (GoToBagMenu's pocket)
u8 MP_Bag_PocketView(u8 pocket)
{
    static const u8 sViewOf[POCKETS_COUNT] = {
        [ITEMS_POCKET] = MP_BAGVIEW_ITEMS, [BALLS_POCKET] = MP_BAGVIEW_BALLS, [TMHM_POCKET] = MP_BAGVIEW_TMHM,
        [BERRIES_POCKET] = MP_BAGVIEW_BERRIES, [KEYITEMS_POCKET] = MP_BAGVIEW_KEY,
    };
    return pocket < POCKETS_COUNT ? sViewOf[pocket] : MP_BAGVIEW_ITEMS;
}

static s32 FavoriteRank(u16 item);

// FAVORITES' stacks in its own order (1.63.14): MANUAL by each favorite's place (MP_Bag_FavoriteMove; marked ones
// after, in the order they were marked; ones from before ranks, after those, in pocket order), else by the sort chosen
// for it (START), as the pockets' own sorts compare. Stable: an item's stacks stay together in their order.
static void SortFavorites(struct ItemSlot **slots, u16 n)
{
    u8 mode = MP_Bag_SortMode(POCKETS_COUNT);
    u16 i, j;
    for (i = 1; i < n; i++)
    {
        struct ItemSlot *cur = slots[i];
        j = i;
        while (j > 0)
        {
            u16 a = slots[j - 1]->itemId, b = cur->itemId;
            int d;
            if (mode == MP_BAGSORT_MANUAL)
            {
                s32 ra = FavoriteRank(a), rb = FavoriteRank(b);
                d = (ra == 0) != (rb == 0) ? (ra == 0 ? 1 : -1) : (ra > rb) - (ra < rb);
            }
            else
            {
                d = a == b ? 0 : CompareItems(a, b, mode);
            }
            if (d <= 0)
                break;
            slots[j] = slots[j - 1];
            j--;
        }
        slots[j] = cur;
    }
}

// A view's stacks in order, as the slots they are in the save's pockets (each pocket compacted and in its sort order
// first: MP_Bag_Sort). FAVORITES: in its own order (SortFavorites).
u16 MP_Bag_ViewSlots(u8 view, struct ItemSlot **out, u16 max)
{
    u16 n = 0;
    u8 p;
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        u8 i;
        if (view != MP_BAGVIEW_FAVORITES && p != MP_Bag_ViewPocket(view))
            continue;
        for (i = 0; i < sPockets[p].bigCount && n < max; i++)
        {
            u16 item = sPockets[p].big[i].itemId;
            if (item == ITEM_NONE)
                break;
            if (view == MP_BAGVIEW_FAVORITES ? MP_Bag_IsFavorite(item) : MP_Bag_ViewOfItem(item) == view)
                out[n++] = &sPockets[p].big[i];
        }
    }
    if (view == MP_BAGVIEW_FAVORITES)
        SortFavorites(out, n);
    return n;
}

// FAVORITES: marked in the item's menu (FAVORITE / UNFAVORITE), kept with the save's other BAG settings
#define FAV_KEY "bag:fav%d"
#define FAV_RANK_KEY "bag:fr%d"    // (1.63.14) a favorite's place in FAVORITES' MANUAL order (0: none yet)
#define FAV_NEXT_KEY "bag:frnext"  // the last place given

static s32 FavoriteRank(u16 item)
{
    char key[16];
    snprintf(key, sizeof(key), FAV_RANK_KEY, item);
    return MP_SaveData_Get(key);
}

// SELECT in FAVORITES (1.63.14): ITEM goes before BEFORE (ITEM_NONE: to the end), the others keep their order; the
// order becomes MANUAL, every favorite numbered as it now shows
void MP_Bag_FavoriteMove(u16 item, u16 before)
{
    static struct ItemSlot *slots[MP_BAG_ITEMS_COUNT + MP_BAG_KEYITEMS_COUNT + MP_BAG_POKEBALLS_COUNT + MP_BAG_TMHM_COUNT
                                  + MP_BAG_BERRIES_COUNT];
    static u16 order[ARRAY_COUNT(slots) + 1];
    u16 n = 0, count, i;
    char key[16];
    if (item == before)
        return;
    count = MP_Bag_ViewSlots(MP_BAGVIEW_FAVORITES, slots, ARRAY_COUNT(slots));
    for (i = 0; i < count; i++)   // its items, each once, as they show, without ITEM
        if (slots[i]->itemId != item && (n == 0 || order[n - 1] != slots[i]->itemId))
            order[n++] = slots[i]->itemId;
    for (i = 0; i < n && order[i] != before; i++)
        ;
    memmove(&order[i + 1], &order[i], (n - i) * sizeof(order[0]));
    order[i] = item;
    n++;
    MPNet_Log("bag: favorite %d moved to place %d of %d", item, i + 1, n);
    MP_Bag_SetSortMode(POCKETS_COUNT, MP_BAGSORT_MANUAL);
    for (i = 0; i < n; i++)
    {
        snprintf(key, sizeof(key), FAV_RANK_KEY, order[i]);
        if (MP_SaveData_Get(key) != i + 1)
            MP_SaveData_Set(key, i + 1);
    }
    MP_SaveData_Set(FAV_NEXT_KEY, n);
}

bool8 MP_Bag_IsFavorite(u16 item)
{
    char key[16];
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return FALSE;
    snprintf(key, sizeof(key), FAV_KEY, item / 31);
    return (MP_SaveData_Get(key) >> (item % 31)) & 1;
}

void MP_Bag_SetFavorite(u16 item, bool8 on)
{
    char key[16];
    s32 bits;
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return;
    snprintf(key, sizeof(key), FAV_KEY, item / 31);
    bits = MP_SaveData_Get(key);
    bits = on ? (bits | (1 << (item % 31))) : (bits & ~(1 << (item % 31)));
    MP_SaveData_Set(key, bits);
    // its place in FAVORITES' own order: last when marked, none when not
    snprintf(key, sizeof(key), FAV_RANK_KEY, item);
    if (on && MP_SaveData_Get(key) == 0)
    {
        s32 next = MP_SaveData_Get(FAV_NEXT_KEY) + 1;
        MP_SaveData_Set(FAV_NEXT_KEY, next);
        MP_SaveData_Set(key, next);
    }
    else if (!on && MP_SaveData_Get(key) != 0)
    {
        MP_SaveData_Set(key, 0);
    }
    MPNet_Log("bag: item %d %s", item, on ? "now a favorite" : "no longer a favorite");
}
