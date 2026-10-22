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
    MPNet_Log("bag: loaded%s", sidecar ? " with the extra slots" : " (extra slots not from this save)");
}
