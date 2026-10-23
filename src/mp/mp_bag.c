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
#include "item.h"
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
static void WriteMarks(FILE *f);

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
    char path[600];
    FILE *f;
    u32 header[3];
    u8 p;
    if (gSaveBlock1Ptr == NULL)
        return;
    SidecarPath(path, sizeof(path));
    f = fopen(path, "wb");
    if (!f)
        return;
    header[0] = BAG_MAGIC;
    header[1] = gSaveCounter;
    header[2] = SaveBlockPocketsSum();
    fwrite(header, sizeof(header), 1, f);
    for (p = 0; p < POCKETS_COUNT; p++)
    {
        u8 n = sPockets[p].bigCount - sPockets[p].saveCount;
        fwrite(&n, 1, 1, f);
        fwrite(sPockets[p].big + sPockets[p].saveCount, sizeof(struct ItemSlot), n, f);
    }
    WriteMarks(f);   // the items' "new" marks and when they came in
    MP_Candy_Write(f);   // the CANDY JAR's candies (mp_candy.c)
    fclose(f);
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
            MP_Candy_Read(f);
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

static void WriteMarks(FILE *f)
{
    u32 magic = MARKS_MAGIC;
    u16 count = ITEMS_COUNT;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&count, sizeof(count), 1, f);
    fwrite(&sNextObtained, sizeof(sNextObtained), 1, f);
    fwrite(sObtained, sizeof(sObtained[0]), ITEMS_COUNT, f);
    fwrite(sNewMark, 1, sizeof(sNewMark), f);
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

u8 MP_Bag_SortMode(u8 pocket)
{
    char key[16];
    s32 v;
    if (pocket >= POCKETS_COUNT)
        return MP_BAGSORT_MANUAL;
    snprintf(key, sizeof(key), SORT_KEY, pocket);
    v = MP_SaveData_Get(key);
    return (v >= 1 && v <= MP_BAGSORT_COUNT) ? v - 1 : DefaultSort(pocket);
}

void MP_Bag_SetSortMode(u8 pocket, u8 mode)
{
    char key[16];
    if (pocket >= POCKETS_COUNT || mode >= MP_BAGSORT_COUNT)
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
    return d ? d : (int)a - (int)b;   // then by number (ID: only that)
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
