// Moemon Maker: flags for what the user adds (an item ball's "already taken", a hidden item's), past every flag the save
// has (docs/MOEMON_MAKER.md "Saves"). Flag ids MAKER_FLAGS_START.. (event_data.c GetFlagPointer), each with a name the
// edits give it ("MAKER_ITEM_MAP_ROUTE101_1"), kept by name in "<save>.maker" beside the save: the save's layout never
// changes, and a name keeps its id from one start to the next (the save's object templates hold ids).
//
// "<save>.maker" is text: "flag NAME ID 0|1" a line.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "maker.h"

extern char gPlatformSavePath[];

#define MAKER_FLAG_NAME 64

static char (*sNames)[MAKER_FLAG_NAME];   // by slot ("" free)
static u8 sBits[MAKER_FLAGS_COUNT / 8];
static u16 sCount;                          // slots used

static void Init(void)
{
    if (!sNames)
        sNames = calloc(MAKER_FLAGS_COUNT, MAKER_FLAG_NAME);
}

// A name's flag id, given one now if it has none (0 if all are used)
u16 Maker_FlagId(const char *name)
{
    u32 i;
    Init();
    for (i = 0; i < sCount; i++)
        if (strcmp(sNames[i], name) == 0)
            return MAKER_FLAGS_START + i;
    if (sCount >= MAKER_FLAGS_COUNT)
        return 0;
    snprintf(sNames[sCount], MAKER_FLAG_NAME, "%s", name);
    return MAKER_FLAGS_START + sCount++;
}

u8 *Maker_FlagPointer(u16 id)
{
    if (id < MAKER_FLAGS_START || id >= MAKER_FLAGS_START + MAKER_FLAGS_COUNT)
        return NULL;
    return &sBits[(id - MAKER_FLAGS_START) / 8];
}

void Maker_Flags_NewGame(void)
{
    Init();
    memset(sBits, 0, sizeof(sBits));
    // (names keep their ids: the maps' edited headers already hold them)
}

void Maker_Flags_LoadFromSave(void)
{
    char path[600], line[160], name[MAKER_FLAG_NAME + 1];
    unsigned id, on;
    FILE *f;
    Init();
    memset(sBits, 0, sizeof(sBits));
    snprintf(path, sizeof(path), "%s.maker", gPlatformSavePath);
    f = fopen(path, "r");
    if (!f)
        return;
    while (fgets(line, sizeof(line), f))
    {
        if (sscanf(line, "flag %64s %u %u", name, &id, &on) != 3 || id < MAKER_FLAGS_START
         || id >= MAKER_FLAGS_START + MAKER_FLAGS_COUNT)
            continue;
        id -= MAKER_FLAGS_START;
        if (sNames[id][0] && strcmp(sNames[id], name) != 0)
        {
            // (that id went to another name before the save was read: this one gets its own)
            id = Maker_FlagId(name) - MAKER_FLAGS_START;
        }
        else
        {
            snprintf(sNames[id], MAKER_FLAG_NAME, "%s", name);
            if (id >= sCount)
                sCount = id + 1;
        }
        if (on)
            sBits[id / 8] |= 1 << (id & 7);
    }
    fclose(f);
}

void Maker_Flags_WriteSidecar(void)
{
    char path[600], tmp[610];
    FILE *f;
    u32 i;
    if (!sNames || sCount == 0)
        return;
    snprintf(path, sizeof(path), "%s.maker", gPlatformSavePath);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "w");
    if (!f)
        return;
    fprintf(f, "# Moemon Maker: flags of what the edits add (written with the save)\n");
    for (i = 0; i < sCount; i++)
        if (sNames[i][0])
            fprintf(f, "flag %s %u %u\n", sNames[i], (unsigned)(MAKER_FLAGS_START + i), (sBits[i / 8] >> (i & 7)) & 1);
    fclose(f);
    remove(path);
    rename(tmp, path);
}
