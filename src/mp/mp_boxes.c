// Moemon Kai: 40 PC boxes (1.63.7). The game holds 40 (TOTAL_BOXES_COUNT); a save holds the first 14 exactly as
// before (struct PokemonStorageLegacy in the save's storage sectors: save.c UpdateSaveAddresses packs them,
// LoadGameSave unpacks them here), so saves keep their layout. Boxes 15-40, their names and wallpapers and the box
// in use when it's past 14 are in "<save>.boxes", written with every save (sdl2.c StoreSaveFile) along with the
// save's counter and a checksum of the 14 it went with; the save's backups keep theirs as "<save>.bakN.boxes". A save
// loads the boxes file written with it (or a backup's, for a .bak renamed back); failing that, the newest boxes file
// anyway (a POKéMON in boxes 15-40 is never dropped because the save fell back to its older copy); none (an older
// save, a new game) leaves 15-40 empty.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "strings.h"
#include "mp.h"
#include "mp_version.h"
#define MAX_DEFAULT_WALLPAPER 3   // (WALLPAPER_SAVANNA: src/data/wallpapers.h)

extern char gPlatformSavePath[512];
extern u32 gSaveCounter;

#define EXTRA_BOXES (TOTAL_BOXES_COUNT - LEGACY_BOXES_COUNT)
#define BOXES_MAGIC 0x584F424D   // "MBOX"
#define BOXES_VERSION 1

struct BoxesFile
{
    u32 magic, version, saveCounter, legacySum;
    u8 currentBox, pad[3];
    struct BoxPokemon boxes[EXTRA_BOXES][IN_BOX_COUNT];
    u8 boxNames[EXTRA_BOXES][BOX_NAME_LENGTH + 1];
    u8 boxWallpapers[EXTRA_BOXES];
};

static struct BoxesFile sFile;

// The 14 boxes' POKéMON, names and wallpapers (not the box in use)
static u32 LegacySum(const struct PokemonStorage *st)
{
    const u8 *parts[3] = {(const u8 *)st->boxes, (const u8 *)st->boxNames, st->boxWallpapers};
    const u32 sizes[3] = {sizeof(st->boxes[0]) * LEGACY_BOXES_COUNT, sizeof(st->boxNames[0]) * LEGACY_BOXES_COUNT,
                          LEGACY_BOXES_COUNT};
    u32 sum = 2166136261u, i, k;
    for (k = 0; k < 3; k++)
        for (i = 0; i < sizes[k]; i++)
            sum = (sum ^ parts[k][i]) * 16777619u;
    return sum;
}

void MP_Boxes_ToLegacy(struct PokemonStorageLegacy *dst, const struct PokemonStorage *src)
{
    dst->currentBox = src->currentBox < LEGACY_BOXES_COUNT ? src->currentBox : 0;   // (past 14: in the boxes file)
    memcpy(dst->boxes, src->boxes, sizeof(dst->boxes));
    memcpy(dst->boxNames, src->boxNames, sizeof(dst->boxNames));
    memcpy(dst->boxWallpapers, src->boxWallpapers, sizeof(dst->boxWallpapers));
}

// The 14 from an older PC; boxes 15-40 empty, named and papered as a new game's (ResetPokemonStorageSystem)
void MP_Boxes_FromLegacy(struct PokemonStorage *dst, const struct PokemonStorageLegacy *src, const u8 *name)
{
    u32 b;
    dst->currentBox = src->currentBox < LEGACY_BOXES_COUNT ? src->currentBox : 0;
    memcpy(dst->boxes, src->boxes, sizeof(src->boxes));
    memcpy(dst->boxNames, src->boxNames, sizeof(src->boxNames));
    memcpy(dst->boxWallpapers, src->boxWallpapers, sizeof(src->boxWallpapers));
    memset(dst->boxes[LEGACY_BOXES_COUNT], 0, sizeof(dst->boxes[0]) * EXTRA_BOXES);
    for (b = LEGACY_BOXES_COUNT; b < TOTAL_BOXES_COUNT; b++)
    {
        u8 *end = StringCopy(dst->boxNames[b], name != NULL ? name : gText_Box);
        ConvertIntToDecimalStringN(end, b + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        dst->boxWallpapers[b] = b % (MAX_DEFAULT_WALLPAPER + 1);
    }
}

// 2: written with this very save; 1: a boxes file, from another save; 0: none
static int ReadFile(const char *path, u32 sum)
{
    FILE *f = fopen(path, "rb");
    int got = 0;
    if (f == NULL)
        return 0;
    if (fread(&sFile, sizeof(sFile), 1, f) == 1 && sFile.magic == BOXES_MAGIC && sFile.version == BOXES_VERSION)
        got = sFile.saveCounter == gSaveCounter && sFile.legacySum == sum ? 2 : 1;
    fclose(f);
    return got;
}

static void Use(struct PokemonStorage *dst)
{
    memcpy(dst->boxes[LEGACY_BOXES_COUNT], sFile.boxes, sizeof(sFile.boxes));
    memcpy(dst->boxNames[LEGACY_BOXES_COUNT], sFile.boxNames, sizeof(sFile.boxNames));
    memcpy(dst->boxWallpapers + LEGACY_BOXES_COUNT, sFile.boxWallpapers, sizeof(sFile.boxWallpapers));
    if (sFile.currentBox >= LEGACY_BOXES_COUNT && sFile.currentBox < TOTAL_BOXES_COUNT && dst->currentBox == 0)
        dst->currentBox = sFile.currentBox;
}

void MP_Boxes_LoadFromSave(const struct PokemonStorageLegacy *saved)
{
    struct PokemonStorage *dst = MP_PersonalStorage();
    char path[600];
    u32 sum, i;
    bool8 newest = FALSE;
    MP_Boxes_FromLegacy(dst, saved, NULL);
    sum = LegacySum(dst);
    for (i = 0; i <= 3; i++)
    {
        int got;
        if (i == 0)
            snprintf(path, sizeof(path), "%s.boxes", gPlatformSavePath);
        else
            snprintf(path, sizeof(path), "%s.bak%u.boxes", gPlatformSavePath, (unsigned)i);
        got = ReadFile(path, sum);
        if (got == 2)
        {
            Use(dst);
            MPNet_Log("boxes: boxes 15-40 loaded (%s)", i == 0 ? "with the save" : "a backup's");
            return;
        }
        if (i == 0)
            newest = got == 1;
    }
    snprintf(path, sizeof(path), "%s.boxes", gPlatformSavePath);
    if (newest && ReadFile(path, sum) == 1)
    {
        Use(dst);
        MPNet_Log("boxes: boxes 15-40 from the newest boxes file (not written with this save)");
    }
}

void MP_Boxes_WriteSidecar(void)
{
    const struct PokemonStorage *src = MP_PersonalStorage();
    char path[600], tmp[610];
    FILE *f;
    if (MOEMON_DEV_BUILD || gSaveBlock2Ptr == NULL || src == NULL)
        return;
    memset(&sFile, 0, sizeof(sFile));
    sFile.magic = BOXES_MAGIC;
    sFile.version = BOXES_VERSION;
    sFile.saveCounter = gSaveCounter;
    sFile.legacySum = LegacySum(src);
    sFile.currentBox = src->currentBox;
    memcpy(sFile.boxes, src->boxes[LEGACY_BOXES_COUNT], sizeof(sFile.boxes));
    memcpy(sFile.boxNames, src->boxNames[LEGACY_BOXES_COUNT], sizeof(sFile.boxNames));
    memcpy(sFile.boxWallpapers, src->boxWallpapers + LEGACY_BOXES_COUNT, sizeof(sFile.boxWallpapers));
    snprintf(path, sizeof(path), "%s.boxes", gPlatformSavePath);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (f == NULL)
        return;
    if (fwrite(&sFile, sizeof(sFile), 1, f) != 1)
    {
        fclose(f);
        remove(tmp);
        return;
    }
    fclose(f);
    remove(path);
    rename(tmp, path);
}
