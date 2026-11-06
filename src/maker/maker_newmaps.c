// Moemon Maker, M2b: new maps (docs/MOEMON_MAKER.md "Build order" M2). A map the user makes lives only in MoemonMaker/
// (maps/MAP_X.txt: its size, tilesets, header and tiles), so no conversion script ever writes it. It joins the game as
// map group MAKER_MAP_GROUP (one past the game's own) and a layout id from MAKER_LAYOUT_FIRST, and the Maker's map list
// as entries after the game's own maps (maker_names.c), so everything the editor does to a map (people, warps,
// connections, events, wild slots: MoemonMaker/base/MAP_X.txt, maker_overrides.c) works on it as on any other.
//
//   newmap MAP_MAKER_MY_TOWN 20 15                       its name (MAP_ + letters, digits, _) and size in metatiles
//   tilesets General Petalburg                           primary and secondary, by the names the TILES panel shows
//   header music=MUS_LITTLEROOT type=2 weather=2 section=MAPSEC_LITTLEROOT_TOWN cycling=1 running=1 escape=0 name=1
//          region=HOENN battle=0
//   border 0x3001 0x3001 0x3001 0x3001
//   row 0 0x3001 0x3001 ...                              each row of tiles (metatile | collision << 10 | elevation << 12)
//
// A save made inside a new map whose file is gone later wakes up in LITTLEROOT TOWN (Overworld_GetMapHeaderByGroupAndId).
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "constants/map_groups.h"
#include "maker.h"
#include "maker_api.h"

const char *Maker_ReadDir(void);
const struct Tileset *Tilesets_MakerByName(const char *name);

struct NewMap
{
    struct MakerMapName names;
    char name[48], groupName[24];
    int region;
    struct MapLayout layout;
    u16 border[4];
    u16 *blocks;
    struct MapHeader header;
    struct MapEvents events;
};

static struct NewMap *sNew;
static int sNewCount, sNewCap;
static bool8 sScanned;

int Maker_NewMapCount(void)
{
    if (!sScanned)
        Maker_NewMaps_Load();
    return sNewCount;
}

const struct MakerMapName *Maker_NewMapName(int n)
{
    return n >= 0 && n < Maker_NewMapCount() ? &sNew[n].names : NULL;
}

int Maker_NewMapRegion(int n)
{
    return n >= 0 && n < Maker_NewMapCount() ? sNew[n].region : MAKER_REGION_HOENN;
}

const struct MapHeader *Maker_NewMapHeader(u16 num)
{
    return num < Maker_NewMapCount() ? &sNew[num].header : NULL;
}

const struct MapLayout *Maker_NewMapLayout(u16 layoutId)
{
    u16 n = layoutId - MAKER_LAYOUT_FIRST;
    return layoutId >= MAKER_LAYOUT_FIRST && n < Maker_NewMapCount() ? &sNew[n].layout : NULL;
}

static int RegionByName(const char *s)
{
    static const char *const sNames[] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    int i;
    for (i = 0; i < 4; i++)
        if (strcmp(s, sNames[i]) == 0)
            return i;
    return MAKER_REGION_HOENN;
}

// One new map from its text (the file's, or the editor's when it's made or changed); its number, or -1
int Maker_NewMapFromText(const char *text)
{
    char line[4096], name[64] = "";
    const char *p = text;
    int w = 0, h = 0, n = -1, i;
    struct NewMap m;
    memset(&m, 0, sizeof(m));
    m.header.music = 0xFFFF;
    m.header.allowCycling = m.header.allowRunning = m.header.showMapName = TRUE;
    while (*p)
    {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p), y;
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : p + strlen(p);
        if (sscanf(line, "newmap %63s %d %d", name, &w, &h) == 3)
        {
            if (w < 1 || h < 1 || w > 255 || h > 255 || strncmp(name, "MAP_", 4) != 0)
                return -1;
            m.blocks = calloc(w * h, sizeof(u16));
            for (i = 0; i < w * h; i++)
                m.blocks[i] = 0x3001;   // (grass, walkable, until rows say otherwise)
            m.border[0] = m.border[1] = m.border[2] = m.border[3] = 0x3001;
        }
        else if (!m.blocks)
            continue;
        else if (strncmp(line, "tilesets ", 9) == 0)
        {
            char a[64], b[64];
            if (sscanf(line, "tilesets %63s %63s", a, b) == 2)
            {
                m.layout.primaryTileset = Tilesets_MakerByName(a);
                m.layout.secondaryTileset = Tilesets_MakerByName(b);
            }
        }
        else if (strncmp(line, "border ", 7) == 0)
        {
            unsigned b[4];
            if (sscanf(line, "border %i %i %i %i", &b[0], &b[1], &b[2], &b[3]) == 4)
                for (i = 0; i < 4; i++)
                    m.border[i] = b[i];
        }
        else if (sscanf(line, "row %d", &y) == 1 && y >= 0 && y < h)
        {
            char *q = strchr(line + 4, ' ');
            for (i = 0; q && i < w; i++)
            {
                char *end;
                long v = strtol(q, &end, 0);
                if (end == q)
                    break;
                m.blocks[y * w + i] = v;
                q = end;
            }
        }
        else if (strncmp(line, "header ", 7) == 0)
        {
            char *q = line + 7, key[32], val[64];
            int used;
            while (sscanf(q, " %31[^=]=%63s%n", key, val, &used) == 2)
            {
                unsigned v;
                q += used;
                if (strcmp(key, "music") == 0)
                    m.header.music = MakerApi_SongId(val) >= 0 ? MakerApi_SongId(val) : 0xFFFF;
                else if (strcmp(key, "region") == 0)
                    m.region = RegionByName(val);
                else if (strcmp(key, "section") == 0)
                    m.header.regionMapSectionId = MakerApi_ValueOf(MAKER_NAMES_MAPSEC, val, &v) ? v : strtol(val, NULL, 0);
                else
                {
                    long x = strtol(val, NULL, 0);
                    if (strcmp(key, "type") == 0) m.header.mapType = x;
                    else if (strcmp(key, "weather") == 0) m.header.weather = x;
                    else if (strcmp(key, "cave") == 0) m.header.cave = x;
                    else if (strcmp(key, "battle") == 0) m.header.battleType = x;
                    else if (strcmp(key, "cycling") == 0) m.header.allowCycling = x != 0;
                    else if (strcmp(key, "running") == 0) m.header.allowRunning = x != 0;
                    else if (strcmp(key, "escape") == 0) m.header.allowEscaping = x != 0;
                    else if (strcmp(key, "name") == 0) m.header.showMapName = x != 0;
                }
            }
        }
    }
    if (!m.blocks)
        return -1;
    if (!m.layout.primaryTileset || !m.layout.secondaryTileset)
    {
        m.layout.primaryTileset = Tilesets_MakerByName("General");
        m.layout.secondaryTileset = Tilesets_MakerByName("Petalburg");
    }
    // the same map again (the editor changed it): in its place, keeping its number
    for (i = 0; i < sNewCount; i++)
        if (strcmp(sNew[i].name, name) == 0)
            n = i;
    if (n < 0)
    {
        if (sNewCount >= MAKER_NEW_MAPS_MAX)
            return -1;
        if (sNewCount == sNewCap)
        {
            sNewCap = sNewCap ? sNewCap * 2 : 16;
            sNew = realloc(sNew, sNewCap * sizeof(*sNew));
            // (the names point into the entries: again after the move)
            for (i = 0; i < sNewCount; i++)
                sNew[i].names.name = sNew[i].name, sNew[i].names.group = sNew[i].groupName;
        }
        n = sNewCount++;
    }
    else
        free(sNew[n].blocks);   // (the old layout: a map being played reloads from the new one)
    snprintf(m.name, sizeof(m.name), "%s", name);
    snprintf(m.groupName, sizeof(m.groupName), "gMapGroup_NewMaps");
    m.names.name = sNew[n].name;
    m.names.group = sNew[n].groupName;
    m.names.mapGroup = MAKER_MAP_GROUP;
    m.names.mapNum = n;
    m.layout.width = w;
    m.layout.height = h;
    sNew[n] = m;
    sNew[n].names.name = sNew[n].name;
    sNew[n].names.group = sNew[n].groupName;
    sNew[n].layout.map = sNew[n].blocks;
    sNew[n].layout.border = sNew[n].border;
    sNew[n].header.mapLayout = &sNew[n].layout;
    sNew[n].header.mapLayoutId = MAKER_LAYOUT_FIRST + n;
    sNew[n].header.events = &sNew[n].events;
    sNew[n].header.mapScripts = NULL;
    sNew[n].header.regionMapSectionIdLo = sNew[n].header.regionMapSectionId & 0xFF;
    return n;
}

void Maker_NewMaps_Load(void)
{
    char dir[1100], path[1400];
    DIR *d;
    struct dirent *de;
    sScanned = TRUE;
    snprintf(dir, sizeof(dir), "%s/maps", Maker_ReadDir());
    if (!(d = opendir(dir)))
        return;
    while ((de = readdir(d)) != NULL)
    {
        size_t nlen = strlen(de->d_name);
        FILE *f;
        long size;
        char *text;
        if (nlen < 5 || strcmp(de->d_name + nlen - 4, ".txt") != 0)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, de->d_name);
        if (!(f = fopen(path, "rb")))
            continue;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        text = malloc(size + 1);
        if (text && fread(text, 1, size, f) == (size_t)size)
        {
            text[size] = 0;
            if (Maker_NewMapFromText(text) < 0)
                printf("[maker] %s: not a new map (left out)\n", de->d_name);
        }
        free(text);
        fclose(f);
    }
    closedir(d);
    if (sNewCount)
        printf("[maker] %d new map%s (%s)\n", sNewCount, sNewCount == 1 ? "" : "s", dir);
}

// The editor: a new map made, or one changed (its whole text, as MoemonMaker/maps/MAP_X.txt has it); the map's index
// in the Maker's list, or -1 if the text isn't a new map
int MakerApi_SetNewMap(const char *text)
{
    int n;
    if (!sScanned)
        Maker_NewMaps_Load();
    n = Maker_NewMapFromText(text);
    return n < 0 ? -1 : Maker_MapIndex(MAKER_MAP_GROUP, n);
}

// The base tiles of a new map, for the editor (metatile | collision << 10 | elevation << 12)
int Maker_NewMapBlock(int n, int x, int y)
{
    if (n < 0 || n >= Maker_NewMapCount() || x < 0 || y < 0 || x >= sNew[n].layout.width || y >= sNew[n].layout.height)
        return -1;
    return sNew[n].blocks[y * sNew[n].layout.width + x];
}
