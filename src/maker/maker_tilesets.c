// Moemon Maker, M5: tileset edits (the editor's TILESETS window, maker/maker_tilesets_ui.cpp). A metatile's eight tile
// entries (tile | hflip 0x400 | vflip 0x800 | palette << 12; 0-3 the bottom layer, 4-7 the top) and its attributes
// (behavior | layer type << 12) changed per tileset, kept in MoemonMaker/tilesets/NAME.txt as lines with the value each
// replaced, like the map edits:
//
//   metatile ID SLOT WAS NOW        (hex entries, e.g. "metatile 0x01A 3 0x3014 0x3015")
//   attr ID WAS NOW                 (hex attributes)
//
// An edit whose WAS isn't the game's value any more is left out (the game changed there since: the editor's merge
// lists it). The game reads a tileset's metatiles and attributes through Maker_TilesetMetatiles / Maker_TilesetAttributes
// (field_camera.c, fieldmap.c, shop.c and the editor's own drawing), patched copies where there are edits.
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "maker.h"
#include "maker_api.h"

int Tilesets_MakerMetatileCount(const struct Tileset *tileset, const char **name);
const struct Tileset *Tilesets_MakerByName(const char *name);
const char *Maker_ReadDir(void);
void Maker_EditsChangedWhilePlaying(int map, bool8 blocksOnly);
void Maker_TilesChanged(void);

struct TsEdit { u16 id; s8 slot; u16 was, now; };   // slot -1: the attributes
struct TsEdits
{
    const struct Tileset *ts;
    char name[40];
    int count;            // metatiles in the tileset
    u16 *metatiles, *attrs;   // patched copies (NULL: none edited)
    struct TsEdit *edits;
    int editCount, skipped;
};
static struct TsEdits *sTs;
static int sTsCount;
static bool8 sLoaded;

static struct TsEdits *Find(const struct Tileset *ts)
{
    int i;
    for (i = 0; i < sTsCount; i++)
        if (sTs[i].ts == ts)
            return &sTs[i];
    return NULL;
}

static void Build(struct TsEdits *t)
{
    int i;
    free(t->metatiles), free(t->attrs);
    t->metatiles = t->attrs = NULL;
    t->skipped = 0;
    for (i = 0; i < t->editCount; i++)
    {
        const struct TsEdit *e = &t->edits[i];
        if (e->id >= t->count || e->slot > 7)
        {
            t->skipped++;
            continue;
        }
        if (e->slot < 0)
        {
            if (t->ts->metatileAttributes[e->id] != e->was)
            {
                t->skipped++;
                continue;
            }
            if (!t->attrs)
            {
                t->attrs = malloc(t->count * sizeof(u16));
                memcpy(t->attrs, t->ts->metatileAttributes, t->count * sizeof(u16));
            }
            t->attrs[e->id] = e->now;
        }
        else
        {
            if (t->ts->metatiles[e->id * 8 + e->slot] != e->was)
            {
                t->skipped++;
                continue;
            }
            if (!t->metatiles)
            {
                t->metatiles = malloc(t->count * 8 * sizeof(u16));
                memcpy(t->metatiles, t->ts->metatiles, t->count * 8 * sizeof(u16));
            }
            t->metatiles[e->id * 8 + e->slot] = e->now;
        }
    }
    if (t->skipped)
        printf("[maker] tileset %s: %d edits left out (the game changed there since)\n", t->name, t->skipped);
}

// One tileset's edits from their text; FALSE when the tileset isn't known
static bool8 Parse(const char *name, const char *text)
{
    const struct Tileset *ts = Tilesets_MakerByName(name);
    struct TsEdits *t;
    const char *p = text;
    if (!ts)
    {
        printf("[maker] tilesets: no tileset %s (left out)\n", name);
        return FALSE;
    }
    t = Find(ts);
    if (!t)
    {
        sTs = realloc(sTs, (sTsCount + 1) * sizeof(*sTs));
        t = &sTs[sTsCount++];
        memset(t, 0, sizeof(*t));
        t->ts = ts;
        snprintf(t->name, sizeof(t->name), "%s", name);
        t->count = Tilesets_MakerMetatileCount(ts, NULL);
    }
    free(t->edits);
    t->edits = NULL;
    t->editCount = 0;
    while (p && *p)
    {
        char line[160];
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        unsigned id, slot, was, now;
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : NULL;
        if (sscanf(line, "metatile %x %u %x %x", &id, &slot, &was, &now) == 4 || (slot = 0xFF,
            sscanf(line, "attr %x %x %x", &id, &was, &now) == 3))
        {
            t->edits = realloc(t->edits, (t->editCount + 1) * sizeof(*t->edits));
            t->edits[t->editCount].id = id;
            t->edits[t->editCount].slot = slot == 0xFF ? -1 : (s8)slot;
            t->edits[t->editCount].was = was;
            t->edits[t->editCount].now = now;
            t->editCount++;
        }
        else if (line[0] && line[0] != '#')
            printf("[maker] tileset %s: not understood (left out): %s\n", name, line);
    }
    Build(t);
    return TRUE;
}

static void LoadAll(void)
{
    char dir[1100], path[1400];
    DIR *d;
    struct dirent *de;
    sLoaded = TRUE;
    snprintf(dir, sizeof(dir), "%s/tilesets", Maker_ReadDir());
    if ((d = opendir(dir)) == NULL)
        return;
    while ((de = readdir(d)) != NULL)
    {
        size_t n = strlen(de->d_name);
        FILE *f;
        char *text, name[64];
        long size;
        if (n < 5 || strcmp(de->d_name + n - 4, ".txt") != 0 || n - 4 >= sizeof(name))
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, de->d_name);
        if ((f = fopen(path, "rb")) == NULL)
            continue;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        text = calloc(1, size + 1);
        if (fread(text, 1, size, f) != (size_t)size)
            size = 0;
        fclose(f);
        memcpy(name, de->d_name, n - 4);
        name[n - 4] = 0;
        Parse(name, text);
        free(text);
    }
    closedir(d);
    if (sTsCount)
        printf("[maker] %d edited tilesets\n", sTsCount);
}

const u16 *Maker_TilesetMetatiles(const struct Tileset *ts)
{
    struct TsEdits *t;
    if (!ts)
        return NULL;
    if (!sLoaded)
        LoadAll();
    t = sTsCount ? Find(ts) : NULL;
    return t && t->metatiles ? t->metatiles : ts->metatiles;
}

const u16 *Maker_TilesetAttributes(const struct Tileset *ts)
{
    struct TsEdits *t;
    if (!ts)
        return NULL;
    if (!sLoaded)
        LoadAll();
    t = sTsCount ? Find(ts) : NULL;
    return t && t->attrs ? t->attrs : ts->metatileAttributes;
}

// The editor: a tileset's edits as it makes them (the text of its file); the field drawn again while the game plays
void MakerApi_SetTilesetEdits(const char *name, const char *text)
{
    if (!sLoaded)
        LoadAll();
    Parse(name, text ? text : "");
    Maker_TilesChanged();
    Maker_EditsChangedWhilePlaying(Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum), TRUE);
}

// The game's own entry / attributes of a tileset's metatile (the editor's WAS), and how many edits were left out
int MakerApi_TilesetBase(const char *name, int id, int slot)
{
    const struct Tileset *ts = Tilesets_MakerByName(name);
    if (!ts || id < 0 || id >= Tilesets_MakerMetatileCount(ts, NULL))
        return -1;
    return slot < 0 ? ts->metatileAttributes[id] : ts->metatiles[id * 8 + (slot & 7)];
}

int MakerApi_TilesetSkipped(const char *name)
{
    const struct Tileset *ts = Tilesets_MakerByName(name);
    struct TsEdits *t = ts ? Find(ts) : NULL;
    return t ? t->skipped : 0;
}
