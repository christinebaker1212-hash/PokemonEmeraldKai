// Moemon Maker: the user's edits to the base game, as the game uses them (docs/MOEMON_MAKER.md "Why first" and "When a
// Claude session changes the base game"). One text file per map in MoemonMaker/base/ (MAP_X.txt), one edit a line,
// each with the value it replaced:
//
//   map MAP_LITTLEROOT_TOWN 20 20                  the map, and its size when it was edited
//   block X Y WAS NOW                              a tile (metatile | collision << 10 | elevation << 12, hex)
//   person KEY PROP WAS NOW                        KEY: its LOCALID_, else SCRIPT@X,Y (its script and first spot)
//   warp INDEX PROP WAS NOW                        PROP: x y elevation dest (MAP_X:N)
//   sign KEY PROP WAS NOW                          KEY: SCRIPT@X,Y
//   trigger KEY PROP WAS NOW                       KEY: SCRIPT@X,Y
//   add person N x=.. y=.. elevation=.. look=.. movement=.. range_x=.. range_y=.. script=.. item=.. flag=..
//   add hidden N x=.. y=.. elevation=.. item=..    a hidden item
//   add sign N x=.. y=.. elevation=.. script=..    a sign reading one of the map's own signs' scripts
//   wild AREA SLOT PROP WAS NOW                    AREA land / water / rock / fish, PROP species / min / max
//   music WAS NOW                                  the map's music: MUS_X, or a new track's name (MoemonMaker/music/)
//   connection INDEX PROP WAS NOW                  PROP direction (up / down / left / right / dive / emerge), map,
//                                                  offset; or removed 0 1
//   add connection N direction=.. map=MAP_X offset=..
//   add warp N x=.. y=.. elevation=.. dest=MAP_X:N a new warp (after the map's own: theirs keep their numbers)
//
// Person PROPs: x y elevation look movement range_x range_y trainer sight flag (values by name where they have one);
// "removed" 0 1 takes a person, sign or trigger away. An added person's script is one the map's own people, signs or
// triggers use (by its label), "itemball" (an item ball holding item=, taken once: a Maker flag of its own) or
// "nothing".
//
// An edit applies only while the game still has its WAS there: where a later update changed the same thing, the
// game's new version wins and the editor shows the clash (nothing is ever applied blindly). The edits reach the map
// through the one place every map header is looked up (overworld.c Overworld_GetMapHeaderByGroupAndId: an edited
// map's header is a patched copy) and through its blocks (fieldmap.c Maker_MapBlocks), so the field, connections,
// warps and scripts all see the edited map. The editor sends its edits as it makes them (MakerApi_SetMapEdits), so a
// map played from the editor is always the edited one.
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "global.fieldmap.h"
#include "overworld.h"
#include "s3d.h"
#include "o3d.h"   // Moemon Kai: ORRE
#include "event_data.h"
#include "constants/event_bg.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "wild_encounter.h"
#include "maker.h"
#include "maker_api.h"

extern const u8 Maker_EventScript_ItemBall[];
extern const u8 Maker_EventScript_Nothing[];

const char *Updater_ExeFile(const char *name);

enum { KIND_PERSON, KIND_WARP, KIND_SIGN, KIND_TRIGGER, KIND_CONNECTION };
enum { PROP_X, PROP_Y, PROP_ELEVATION, PROP_LOOK, PROP_MOVEMENT, PROP_RANGE_X, PROP_RANGE_Y, PROP_TRAINER, PROP_SIGHT,
       PROP_FLAG, PROP_DEST, PROP_REMOVED, PROP_ITEM, PROP_DIRECTION, PROP_MAP, PROP_OFFSET, PROP_COUNT };
static const char *const sPropNames[PROP_COUNT] = {"x", "y", "elevation", "look", "movement", "range_x", "range_y",
                                                   "trainer", "sight", "flag", "dest", "removed", "item", "direction",
                                                   "map", "offset"};
static const char *const sDirections[] = {"", "down", "up", "left", "right", "dive", "emerge"};   // CONNECTION_*

struct BlockEdit { u16 x, y, was, now; };
struct WildEdit { u8 area, slot, prop; s32 was, now; };   // prop: 0 species, 1 min, 2 max
enum { ADD_PERSON, ADD_HIDDEN, ADD_SIGN, ADD_CONNECTION, ADD_WARP };
struct AddEdit { u8 kind; u16 n; s32 v[PROP_COUNT]; bool8 has[PROP_COUNT]; char script[80]; };
struct PropEdit { u8 kind, prop; char key[80]; s32 was, now; };

struct MapEdits
{
    char text[1];   // (unused: the edits are kept parsed)
    u16 w, h;
    struct BlockEdit *blocks;
    u32 blockCount, blockCap;
    struct PropEdit *props;
    u32 propCount, propCap;
    struct AddEdit *adds;
    u32 addCount, addCap;
    struct WildEdit *wild;
    u32 wildCount, wildCap;
    bool8 hasMusic;
    s32 musicWas, musicNow;
    // built from the edits: the patched header (NULL until asked for) and blocks
    struct MapHeader *header;
    struct MapEvents *events;
    u16 *patchedBlocks;
    const u16 *patchedFrom;   // (the base blocks they were made from)
    u32 skipped;              // edits whose WAS the game no longer has
};

static struct MapEdits *sEdits[1700];
static bool8 sAny, sLoaded;

static struct MapEdits *EditsFor(int map)
{
    if (map < 0 || map >= Maker_MapCount() || map >= (int)ARRAY_COUNT(sEdits))
        return NULL;
    return sEdits[map];
}

// ---------------------------------------------------------------------------------------------------------------------
// Values by name

static bool8 ParseValue(u8 prop, const char *s, s32 *out)
{
    unsigned int v;
    char *end;
    int table = -1;
    switch (prop)
    {
    case PROP_LOOK: table = MAKER_NAMES_GFX; break;
    case PROP_MOVEMENT: table = MAKER_NAMES_MOVEMENT; break;
    case PROP_TRAINER: table = MAKER_NAMES_TRAINERTYPE; break;
    case PROP_FLAG: table = MAKER_NAMES_FLAG; break;
    case PROP_ITEM: table = MAKER_NAMES_ITEM; break;
    case PROP_DIRECTION:
    {
        u32 i;
        for (i = 1; i < ARRAY_COUNT(sDirections); i++)
            if (strcmp(s, sDirections[i]) == 0)
            {
                *out = i;
                return TRUE;
            }
        return FALSE;
    }
    case PROP_MAP:
    {
        int i = MakerApi_FindMap(s);
        if (i < 0)
            return FALSE;
        *out = i;
        return TRUE;
    }
    case PROP_DEST:
    {
        // MAP_X:N -> map index << 8 | warp id
        char map[64];
        const char *colon = strchr(s, ':');
        int i, len = colon ? (int)(colon - s) : (int)strlen(s);
        if (!colon || len >= (int)sizeof(map))
            return FALSE;
        memcpy(map, s, len);
        map[len] = 0;
        i = MakerApi_FindMap(map);
        if (i < 0)
            return FALSE;
        *out = (i << 8) | (atoi(colon + 1) & 0xFF);
        return TRUE;
    }
    }
    if (table >= 0 && (isalpha((unsigned char)s[0]) || s[0] == '_'))
    {
        if (!MakerApi_ValueOf(table, s, &v))
            return FALSE;
        *out = (s32)v;
        return TRUE;
    }
    *out = (s32)strtol(s, &end, 0);
    return end != s;
}

// ---------------------------------------------------------------------------------------------------------------------
// Reading a map's edits

static void FreeBuilt(struct MapEdits *e)
{
    // (a patched header the field still uses, the current map's, is kept: gMapHeader copied it)
    if (e->header && gMapHeader.events != e->events)
    {
        free(e->events ? (void *)e->events->objectEvents : NULL);
        free(e->events ? (void *)e->events->warps : NULL);
        free(e->events ? (void *)e->events->coordEvents : NULL);
        free(e->events ? (void *)e->events->bgEvents : NULL);
        free(e->events);
        free(e->header);   // (an edited connections copy is small and kept: the field may still read it)
    }
    e->header = NULL;
    e->events = NULL;
    free(e->patchedBlocks);
    e->patchedBlocks = NULL;
    e->patchedFrom = NULL;
}

static void FreeEdits(int map)
{
    struct MapEdits *e = EditsFor(map);
    if (!e)
        return;
    FreeBuilt(e);
    free(e->blocks);
    free(e->props);
    free(e->adds);
    free(e->wild);
    free(e);
    sEdits[map] = NULL;
}

static void AddBlock(struct MapEdits *e, u16 x, u16 y, u16 was, u16 now)
{
    if (e->blockCount == e->blockCap)
    {
        e->blockCap = e->blockCap ? e->blockCap * 2 : 64;
        e->blocks = realloc(e->blocks, e->blockCap * sizeof(*e->blocks));
    }
    e->blocks[e->blockCount++] = (struct BlockEdit){x, y, was, now};
}

static void AddProp(struct MapEdits *e, u8 kind, const char *key, u8 prop, s32 was, s32 now)
{
    struct PropEdit *p;
    if (e->propCount == e->propCap)
    {
        e->propCap = e->propCap ? e->propCap * 2 : 32;
        e->props = realloc(e->props, e->propCap * sizeof(*e->props));
    }
    p = &e->props[e->propCount++];
    p->kind = kind;
    p->prop = prop;
    snprintf(p->key, sizeof(p->key), "%s", key);
    p->was = was;
    p->now = now;
}

// "add KIND N k=v k=v..."
static bool8 ParseAdd(struct MapEdits *e, const char *line)
{
    static const char *const sKinds[] = {"person", "hidden", "sign", "connection", "warp"};
    char kind[16], word[128];
    const char *p;
    int n, used, k, prop;
    struct AddEdit a;
    if (sscanf(line, "add %15s %d %n", kind, &n, &used) < 2)
        return FALSE;
    for (k = 0; k < 5 && strcmp(kind, sKinds[k]) != 0; k++)
        ;
    if (k == 5)
        return FALSE;
    memset(&a, 0, sizeof(a));
    a.kind = k;
    a.n = n;
    for (p = line + used; sscanf(p, "%127s%n", word, &used) == 1; p += used)
    {
        char *eq = strchr(word, '=');
        if (!eq)
            return FALSE;
        *eq = 0;
        if (strcmp(word, "script") == 0)
        {
            snprintf(a.script, sizeof(a.script), "%s", eq + 1);
            continue;
        }
        for (prop = 0; prop < PROP_COUNT && strcmp(word, sPropNames[prop]) != 0; prop++)
            ;
        if (prop == PROP_COUNT || !ParseValue(prop, eq + 1, &a.v[prop]))
            return FALSE;
        a.has[prop] = TRUE;
    }
    if (k == ADD_CONNECTION ? !a.has[PROP_DIRECTION] || !a.has[PROP_MAP] : !a.has[PROP_X] || !a.has[PROP_Y])
        return FALSE;
    if (e->addCount == e->addCap)
    {
        e->addCap = e->addCap ? e->addCap * 2 : 8;
        e->adds = realloc(e->adds, e->addCap * sizeof(*e->adds));
    }
    e->adds[e->addCount++] = a;
    return TRUE;
}

// One map's edits from their text (the file's, or the editor's); returns the map, or -1
static int ParseEdits(const char *text, const char *from)
{
    char line[512];
    const char *p = text;
    struct MapEdits *e = NULL;
    int map = -1, lineNo = 0;
    while (*p)
    {
        char word[5][96];
        int n, len;
        const char *nl = strchr(p, '\n');
        len = nl ? (int)(nl - p) : (int)strlen(p);
        if (len >= (int)sizeof(line))
            len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : p + strlen(p);
        lineNo++;
        if (line[0] == '#' || line[0] == 0 || line[0] == '\r')
            continue;
        n = sscanf(line, "%95s %95s %95s %95s %95s", word[0], word[1], word[2], word[3], word[4]);
        if (n <= 0)
            continue;
        if (strcmp(word[0], "map") == 0 && n >= 2)
        {
            map = MakerApi_FindMap(word[1]);
            if (map < 0 || map >= (int)ARRAY_COUNT(sEdits))
            {
                printf("[maker] %s: no map %s (left out)\n", from, word[1]);
                return -1;
            }
            FreeEdits(map);
            e = calloc(1, sizeof(*e));
            sEdits[map] = e;
            if (n >= 4)
            {
                e->w = atoi(word[2]);
                e->h = atoi(word[3]);
            }
            continue;
        }
        if (e == NULL)
            continue;
        if (strcmp(word[0], "block") == 0 && n == 5)
            AddBlock(e, atoi(word[1]), atoi(word[2]), strtol(word[3], NULL, 0), strtol(word[4], NULL, 0));
        else if (strcmp(word[0], "music") == 0 && n == 3)
        {
            int was = MakerApi_SongId(word[1]), now = MakerApi_SongId(word[2]);
            if (was < 0 || now < 0)
            {
                printf("[maker] %s line %d: no such song (left out): %s\n", from, lineNo, line);
                continue;
            }
            e->hasMusic = TRUE;
            e->musicWas = was;
            e->musicNow = now;
        }
        else if (strcmp(word[0], "wild") == 0 && n == 5)
        {
            // wild AREA SLOT PROP WAS NOW (5 words read: the line's last two again)
            static const char *const sAreas[] = {"land", "water", "rock", "fish"};
            static const char *const sProps[] = {"species", "min", "max"};
            char wasW[96], nowW[96];
            int area, prop;
            unsigned v;
            struct WildEdit w;
            if (sscanf(line, "wild %*s %*s %*s %95s %95s", wasW, nowW) != 2)
                continue;
            for (area = 0; area < 4 && strcmp(word[1], sAreas[area]) != 0; area++)
                ;
            for (prop = 0; prop < 3 && strcmp(word[3], sProps[prop]) != 0; prop++)
                ;
            if (area == 4 || prop == 3)
            {
                printf("[maker] %s line %d: not understood (left out): %s\n", from, lineNo, line);
                continue;
            }
            w.area = area;
            w.slot = atoi(word[2]);
            w.prop = prop;
            if (prop == 0)
            {
                if (!MakerApi_ValueOf(MAKER_NAMES_SPECIES, wasW, &v))
                    continue;
                w.was = v;
                if (!MakerApi_ValueOf(MAKER_NAMES_SPECIES, nowW, &v))
                    continue;
                w.now = v;
            }
            else
            {
                w.was = atoi(wasW);
                w.now = atoi(nowW);
            }
            if (e->wildCount == e->wildCap)
            {
                e->wildCap = e->wildCap ? e->wildCap * 2 : 16;
                e->wild = realloc(e->wild, e->wildCap * sizeof(*e->wild));
            }
            e->wild[e->wildCount++] = w;
        }
        else if (strcmp(word[0], "add") == 0)
        {
            if (!ParseAdd(e, line))
                printf("[maker] %s line %d: not understood (left out): %s\n", from, lineNo, line);
        }
        else if (n == 5)
        {
            static const char *const sKinds[] = {"person", "warp", "sign", "trigger", "connection"};
            int k, prop;
            s32 was, now;
            for (k = 0; k < 5 && strcmp(word[0], sKinds[k]) != 0; k++)
                ;
            for (prop = 0; prop < PROP_COUNT && strcmp(word[2], sPropNames[prop]) != 0; prop++)
                ;
            if (k == 5 || prop == PROP_COUNT || !ParseValue(prop, word[3], &was) || !ParseValue(prop, word[4], &now))
            {
                printf("[maker] %s line %d: not understood (left out): %s\n", from, lineNo, line);
                continue;
            }
            AddProp(e, k, word[1], prop, was, now);
        }
        else
            printf("[maker] %s line %d: not understood (left out): %s\n", from, lineNo, line);
    }
    return map;
}

// The editor's working copy: MoemonMaker/ beside the game (MOEMON_MAKER_DIR in tests). Only the user's PC has one.
const char *MakerApi_EditsDir(void)
{
    static char dir[1100];
    const char *env = getenv("MOEMON_MAKER_DIR");
    snprintf(dir, sizeof(dir), "%s", env && env[0] ? env : Updater_ExeFile("MoemonMaker"));
    return dir;
}

// The published pack: MoemonMakerPack/ beside the game (in every release's zip, and downloaded between releases from
// the maker-pack release: updater.c Updater_MakerPack*)
const char *MakerApi_PackDir(void)
{
    static char dir[1100];
    snprintf(dir, sizeof(dir), "%s", Updater_ExeFile("MoemonMakerPack"));
    return dir;
}

// What the game plays: the working copy where there is one (the user's own PC: their edits as they make them), else the
// published pack
const char *Maker_ReadDir(void)
{
    char probe[1200];
    DIR *d;
    const char *work = MakerApi_EditsDir();
    if (getenv("MOEMON_MAKER_DIR"))
        return work;
    snprintf(probe, sizeof(probe), "%s/base", work);
    if ((d = opendir(probe)) != NULL)
    {
        closedir(d);
        return work;
    }
    snprintf(probe, sizeof(probe), "%s/music", work);
    if ((d = opendir(probe)) != NULL)
    {
        closedir(d);
        return work;
    }
    return MakerApi_PackDir();
}

// MoemonMaker/base/*.txt beside the game (MOEMON_MAKER_DIR in tests), when the game starts
static void LoadAll(void)
{
    char dir[1024], path[1200];
    DIR *d;
    struct dirent *de;
    int count = 0;
    sLoaded = TRUE;
    snprintf(dir, sizeof(dir), "%s/base", Maker_ReadDir());
    d = opendir(dir);
    if (!d)
        return;
    while ((de = readdir(d)) != NULL)
    {
        FILE *f;
        long size;
        char *text;
        size_t nlen = strlen(de->d_name);
        if (nlen < 5 || strcmp(de->d_name + nlen - 4, ".txt") != 0)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dir, de->d_name);
        f = fopen(path, "rb");
        if (!f)
            continue;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        text = malloc(size + 1);
        if (text && fread(text, 1, size, f) == (size_t)size)
        {
            text[size] = 0;
            if (ParseEdits(text, de->d_name) >= 0)
            {
                count++;
                sAny = TRUE;
            }
        }
        free(text);
        fclose(f);
    }
    closedir(d);
    printf("[maker] %d edited maps (%s)\n", count, dir);
}

void Maker_LoadEdits(void)
{
    if (!sLoaded)
        LoadAll();
}

void MakerApi_ReloadEdits(void)
{
    int i;
    for (i = 0; i < (int)ARRAY_COUNT(sEdits); i++)
        FreeEdits(i);
    sAny = FALSE;
    LoadAll();
}

// The editor's edits of one map (its whole text; "" when it has none left)
// (everything but the tiles, to tell a tile edit from one that needs the map loaded again: maker_play.c)
static u32 NonBlockHash(const char *text)
{
    u32 h = 0;   // (no lines: 0, as a map with no edits yet has)
    const char *p = text;
    while (p && *p)
    {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        if (strncmp(p, "block ", 6) != 0 && strncmp(p, "map ", 4) != 0 && p[0] != '#')
            for (size_t i = 0; i < len; i++)
                h = (h ^ (u8)p[i]) * 16777619u;
        p = nl ? nl + 1 : p + len;
    }
    return h;
}

void MakerApi_SetMapEdits(const char *mapName, const char *text)
{
    static u32 sHash[1700];
    int map = MakerApi_FindMap(mapName);
    u32 h;
    Maker_LoadEdits();
    if (map < 0 || map >= (int)ARRAY_COUNT(sHash))
        return;
    FreeEdits(map);
    if (text && text[0] && ParseEdits(text, mapName) == map)
        sAny = TRUE;
    h = NonBlockHash(text);
    Maker_EditsChangedWhilePlaying(map, h == sHash[map]);
    sHash[map] = h;
}

// ---------------------------------------------------------------------------------------------------------------------
// The edited map

// A person / sign / trigger's key: its LOCALID_ name, else SCRIPT@X,Y
static void KeyOf(char *key, int size, const char *localIdName, const char *script, int x, int y)
{
    if (localIdName && localIdName[0])
        snprintf(key, size, "%s", localIdName);
    else
        snprintf(key, size, "%s@%d,%d", script ? script : "", x, y);
}

static s32 *PersonField(struct ObjectEventTemplate *t, u8 prop, s32 *tmp)
{
    switch (prop)
    {
    case PROP_X: *tmp = t->x; return tmp;
    case PROP_Y: *tmp = t->y; return tmp;
    case PROP_ELEVATION: *tmp = t->elevation; return tmp;
    case PROP_LOOK: *tmp = t->graphicsIdLo | (t->graphicsIdHi << 8); return tmp;
    case PROP_MOVEMENT: *tmp = t->movementType; return tmp;
    case PROP_RANGE_X: *tmp = t->movementRangeX; return tmp;
    case PROP_RANGE_Y: *tmp = t->movementRangeY; return tmp;
    case PROP_TRAINER: *tmp = t->trainerType; return tmp;
    case PROP_SIGHT: *tmp = t->trainerRange_berryTreeId; return tmp;
    case PROP_FLAG: *tmp = t->flagId; return tmp;
    }
    return NULL;
}

// A script by its label among the map's own (its people's, signs', triggers'), or the Maker's own
static const u8 *ScriptOnMap(int map, const struct MapEvents *be, const char *label)
{
    const struct MakerMapEventNames *names = Maker_MapEventNames(map);
    u32 i;
    if (strcmp(label, "itemball") == 0)
        return Maker_EventScript_ItemBall;
    if (strcmp(label, "nothing") == 0 || label[0] == 0 || names == NULL)
        return Maker_EventScript_Nothing;
    for (i = 0; i < be->objectEventCount && i < names->objCount; i++)
        if (strcmp(Maker_ObjName(names->obj + i)->script, label) == 0)
            return be->objectEvents[i].script;
    for (i = 0; i < be->bgEventCount && i < names->bgCount; i++)
        if (be->bgEvents[i].kind != BG_EVENT_HIDDEN_ITEM && strcmp(Maker_BgName(names->bg + i)->script, label) == 0)
            return be->bgEvents[i].bgUnion.script;
    for (i = 0; i < be->coordEventCount && i < names->coordCount; i++)
        if (strcmp(Maker_CoordName(names->coord + i)->script, label) == 0)
            return be->coordEvents[i].script;
    return Maker_EventScript_Nothing;
}

static void SetPerson(struct ObjectEventTemplate *t, u8 prop, s32 v)
{
    switch (prop)
    {
    case PROP_X: t->x = v; break;
    case PROP_Y: t->y = v; break;
    case PROP_ELEVATION: t->elevation = v; break;
    case PROP_LOOK: t->graphicsIdLo = v & 0xFF; t->graphicsIdHi = (v >> 8) & 0xFF; break;
    case PROP_MOVEMENT: t->movementType = v; break;
    case PROP_RANGE_X: t->movementRangeX = v; break;
    case PROP_RANGE_Y: t->movementRangeY = v; break;
    case PROP_TRAINER: t->trainerType = v; break;
    case PROP_SIGHT: t->trainerRange_berryTreeId = v; break;
    case PROP_FLAG: t->flagId = v; break;
    }
}

// The edited connections: the map's own with their edits (some taken away), then the added ones
static void BuildConnections(struct MapEdits *e, const struct MapHeader *base)
{
    const struct MapConnections *bc = base->connections;
    struct MapConnections *mc;
    struct MapConnection *c;
    u32 i, j, n = 0, baseCount = bc ? bc->count : 0;
    bool8 any = FALSE;
    for (j = 0; j < e->propCount; j++)
        any |= e->props[j].kind == KIND_CONNECTION;
    for (j = 0; j < e->addCount; j++)
        any |= e->adds[j].kind == ADD_CONNECTION;
    if (!any)
        return;
    mc = malloc(sizeof(*mc));
    c = malloc(sizeof(*c) * (baseCount + e->addCount + 1));
    for (i = 0; i < baseCount; i++)
    {
        struct MapConnection v = bc->connections[i];
        const struct MapConnection *b = &bc->connections[i];
        bool8 gone = FALSE;
        int bm = Maker_MapIndex(b->mapGroup, b->mapNum);
        for (j = 0; j < e->propCount; j++)
        {
            const struct PropEdit *p = &e->props[j];
            const struct MakerMapName *dn;
            if (p->kind != KIND_CONNECTION || atoi(p->key) != (int)i)
                continue;
            switch (p->prop)
            {
            case PROP_REMOVED: gone = p->now != 0; break;
            case PROP_DIRECTION: if (b->direction == p->was) v.direction = p->now; else e->skipped++; break;
            case PROP_OFFSET: if ((s32)b->offset == p->was) v.offset = p->now; else e->skipped++; break;
            case PROP_MAP:
                dn = Maker_Map(p->now);
                if (bm == p->was && dn)
                    v.mapGroup = dn->mapGroup, v.mapNum = dn->mapNum;
                else
                    e->skipped++;
                break;
            }
        }
        if (!gone)
            c[n++] = v;
    }
    for (j = 0; j < e->addCount; j++)
    {
        const struct AddEdit *a = &e->adds[j];
        const struct MakerMapName *dn = Maker_Map(a->v[PROP_MAP]);
        if (a->kind != ADD_CONNECTION || dn == NULL)
            continue;
        c[n].direction = a->v[PROP_DIRECTION];
        c[n].offset = a->v[PROP_OFFSET];
        c[n].mapGroup = dn->mapGroup;
        c[n].mapNum = dn->mapNum;
        n++;
    }
    mc->count = n;
    mc->connections = c;
    e->header->connections = mc;
}

// The edited header (people, warps, signs, triggers), made once per change of the edits
static const struct MapHeader *BuildHeader(int map, struct MapEdits *e)
{
    const struct MakerMapName *mn = Maker_Map(map);
    const struct MapHeader *base = Maker_BaseMapHeader(mn->mapGroup, mn->mapNum);
    const struct MakerMapEventNames *names = Maker_MapEventNames(map);
    const struct MapEvents *be = base->events;
    struct ObjectEventTemplate *objs;
    struct WarpEvent *warps;
    struct BgEvent *bgs;
    struct CoordEvent *coords;
    u32 i, j;
    char key[96];
    if (e->header)
        return e->header;
    e->header = malloc(sizeof(*e->header));
    *e->header = *base;
    e->skipped = 0;
    if (e->hasMusic)
    {
        if (base->music == e->musicWas)
            e->header->music = e->musicNow;
        else
            e->skipped++;
    }
    if (be == NULL)
        return e->header;
    e->events = malloc(sizeof(*e->events));
    *e->events = *be;
    objs = malloc(sizeof(*objs) * (be->objectEventCount + e->addCount + 1));
    warps = malloc(sizeof(*warps) * (be->warpCount + e->addCount + 1));
    bgs = malloc(sizeof(*bgs) * (be->bgEventCount + e->addCount + 1));
    coords = malloc(sizeof(*coords) * (be->coordEventCount + 1));
    memcpy(objs, be->objectEvents, sizeof(*objs) * be->objectEventCount);
    memcpy(warps, be->warps, sizeof(*warps) * be->warpCount);
    memcpy(bgs, be->bgEvents, sizeof(*bgs) * be->bgEventCount);
    memcpy(coords, be->coordEvents, sizeof(*coords) * be->coordEventCount);
    // (taken away: marked, then left out below)
    u8 *objGone = calloc(be->objectEventCount + 1, 1), *bgGone = calloc(be->bgEventCount + 1, 1);
    u8 *coordGone = calloc(be->coordEventCount + 1, 1);
    for (j = 0; j < e->propCount; j++)
    {
        const struct PropEdit *p = &e->props[j];
        bool8 applied = FALSE;
        s32 tmp, *cur;
        if (p->prop == PROP_REMOVED)
        {
            u32 count = p->kind == KIND_PERSON ? be->objectEventCount : p->kind == KIND_SIGN ? be->bgEventCount : be->coordEventCount;
            for (i = 0; i < count && p->kind != KIND_WARP; i++)
            {
                if (p->kind == KIND_PERSON)
                {
                    const struct MakerObjName *on = names && i < names->objCount ? Maker_ObjName(names->obj + i) : NULL;
                    KeyOf(key, sizeof(key), on ? on->localId : "", on ? on->script : "", be->objectEvents[i].x, be->objectEvents[i].y);
                }
                else if (p->kind == KIND_SIGN)
                    KeyOf(key, sizeof(key), "", names && i < names->bgCount ? Maker_BgName(names->bg + i)->script : "", be->bgEvents[i].x, be->bgEvents[i].y);
                else
                    KeyOf(key, sizeof(key), "", names && i < names->coordCount ? Maker_CoordName(names->coord + i)->script : "", be->coordEvents[i].x, be->coordEvents[i].y);
                if (strcmp(key, p->key) == 0 && p->now)
                {
                    (p->kind == KIND_PERSON ? objGone : p->kind == KIND_SIGN ? bgGone : coordGone)[i] = TRUE;
                    applied = TRUE;
                    break;
                }
            }
            if (!applied)
                e->skipped++;
            continue;
        }
        switch (p->kind)
        {
        case KIND_PERSON:
            for (i = 0; i < be->objectEventCount && !applied; i++)
            {
                const struct MakerObjName *on = names && i < names->objCount ? Maker_ObjName(names->obj + i) : NULL;
                KeyOf(key, sizeof(key), on ? on->localId : "", on ? on->script : "", be->objectEvents[i].x, be->objectEvents[i].y);
                if (strcmp(key, p->key) != 0)
                    continue;
                // (WAS against the game's own value, never one an earlier edit made)
                {
                    struct ObjectEventTemplate orig = be->objectEvents[i];
                    cur = PersonField(&orig, p->prop, &tmp);
                }
                if (cur && *cur == p->was)
                {
                    SetPerson(&objs[i], p->prop, p->now);
                    applied = TRUE;
                }
                break;
            }
            break;
        case KIND_WARP:
            i = atoi(p->key);
            if (i < be->warpCount)
            {
                const struct WarpEvent *w = &be->warps[i];
                const struct MakerMapName *dn;
                s32 v = -1;
                int d;
                switch (p->prop)
                {
                case PROP_X: v = w->x; break;
                case PROP_Y: v = w->y; break;
                case PROP_ELEVATION: v = w->elevation; break;
                case PROP_DEST:
                    d = Maker_MapIndex(w->mapGroup, w->mapNum);
                    v = d < 0 ? -1 : (d << 8) | w->warpId;
                    break;
                }
                if (v == p->was && v != -1)
                {
                    switch (p->prop)
                    {
                    case PROP_X: warps[i].x = p->now; break;
                    case PROP_Y: warps[i].y = p->now; break;
                    case PROP_ELEVATION: warps[i].elevation = p->now; break;
                    case PROP_DEST:
                        dn = Maker_Map(p->now >> 8);
                        if (dn)
                        {
                            warps[i].mapGroup = dn->mapGroup;
                            warps[i].mapNum = dn->mapNum;
                            warps[i].warpId = p->now & 0xFF;
                        }
                        break;
                    }
                    applied = TRUE;
                }
            }
            break;
        case KIND_CONNECTION:
            applied = TRUE;   // (with the connections: BuildConnections)
            break;
        case KIND_SIGN:
            for (i = 0; i < be->bgEventCount && !applied; i++)
            {
                const struct MakerBgName *bn = names && i < names->bgCount ? Maker_BgName(names->bg + i) : NULL;
                KeyOf(key, sizeof(key), "", bn ? bn->script : "", be->bgEvents[i].x, be->bgEvents[i].y);
                if (strcmp(key, p->key) != 0)
                    continue;
                if (p->prop == PROP_X && be->bgEvents[i].x == p->was)
                    bgs[i].x = p->now, applied = TRUE;
                else if (p->prop == PROP_Y && be->bgEvents[i].y == p->was)
                    bgs[i].y = p->now, applied = TRUE;
                else if (p->prop == PROP_ELEVATION && be->bgEvents[i].elevation == p->was)
                    bgs[i].elevation = p->now, applied = TRUE;
                break;
            }
            break;
        case KIND_TRIGGER:
            for (i = 0; i < be->coordEventCount && !applied; i++)
            {
                const struct MakerCoordName *cn = names && i < names->coordCount ? Maker_CoordName(names->coord + i) : NULL;
                KeyOf(key, sizeof(key), "", cn ? cn->script : "", be->coordEvents[i].x, be->coordEvents[i].y);
                if (strcmp(key, p->key) != 0)
                    continue;
                if (p->prop == PROP_X && be->coordEvents[i].x == p->was)
                    coords[i].x = p->now, applied = TRUE;
                else if (p->prop == PROP_Y && be->coordEvents[i].y == p->was)
                    coords[i].y = p->now, applied = TRUE;
                break;
            }
            break;
        }
        if (!applied)
            e->skipped++;
    }
    // what was taken away left out, what was added put in
    {
        u32 n = 0, maxLocal = 0;
        for (i = 0; i < be->objectEventCount; i++)
        {
            if (be->objectEvents[i].localId > maxLocal)
                maxLocal = be->objectEvents[i].localId;
            if (!objGone[i])
                objs[n++] = objs[i];
        }
        for (j = 0; j < e->addCount && n < OBJECT_EVENT_TEMPLATES_COUNT; j++)
        {
            const struct AddEdit *a = &e->adds[j];
            struct ObjectEventTemplate *t;
            char flagName[96];
            if (a->kind != ADD_PERSON)
                continue;
            t = &objs[n++];
            memset(t, 0, sizeof(*t));
            t->localId = maxLocal + 1 + a->n < 255 ? maxLocal + 1 + a->n : 254;
            SetPerson(t, PROP_LOOK, a->has[PROP_LOOK] ? a->v[PROP_LOOK] : 0);
            t->x = a->v[PROP_X];
            t->y = a->v[PROP_Y];
            t->elevation = a->has[PROP_ELEVATION] ? a->v[PROP_ELEVATION] : 3;
            t->movementType = a->has[PROP_MOVEMENT] ? a->v[PROP_MOVEMENT] : 0;
            t->movementRangeX = a->v[PROP_RANGE_X];
            t->movementRangeY = a->v[PROP_RANGE_Y];
            t->trainerType = a->v[PROP_TRAINER];
            t->script = ScriptOnMap(map, be, a->script);
            if (strcmp(a->script, "itemball") == 0)
            {
                // (the item rides in the berry tree / sight field, read by Maker_ItemBallItem; taken once)
                t->trainerRange_berryTreeId = a->v[PROP_ITEM];
                snprintf(flagName, sizeof(flagName), "MAKER_ITEM_%s_%u", mn->name, a->n);
                t->flagId = Maker_FlagId(flagName);
            }
            else
            {
                t->trainerRange_berryTreeId = a->v[PROP_SIGHT];
                t->flagId = a->v[PROP_FLAG];
            }
        }
        e->events->objectEventCount = n;
        n = 0;
        for (i = 0; i < be->bgEventCount; i++)
            if (!bgGone[i])
                bgs[n++] = bgs[i];
        for (j = 0; j < e->addCount; j++)
        {
            const struct AddEdit *a = &e->adds[j];
            struct BgEvent *b;
            char flagName[96];
            if (a->kind != ADD_HIDDEN && a->kind != ADD_SIGN)
                continue;
            b = &bgs[n++];
            memset(b, 0, sizeof(*b));
            b->x = a->v[PROP_X];
            b->y = a->v[PROP_Y];
            b->elevation = a->has[PROP_ELEVATION] ? a->v[PROP_ELEVATION] : 0;
            if (a->kind == ADD_HIDDEN)
            {
                snprintf(flagName, sizeof(flagName), "MAKER_HIDDEN_%s_%u", mn->name, a->n);
                b->kind = BG_EVENT_HIDDEN_ITEM;
                b->bgUnion.hiddenItem.item = a->v[PROP_ITEM];
                b->bgUnion.hiddenItem.hiddenItemId = Maker_FlagId(flagName) - FLAG_HIDDEN_ITEMS_START;
            }
            else
            {
                b->kind = BG_EVENT_PLAYER_FACING_ANY;
                b->bgUnion.script = ScriptOnMap(map, be, a->script);
            }
        }
        e->events->bgEventCount = n;
        n = 0;
        for (i = 0; i < be->coordEventCount; i++)
            if (!coordGone[i])
                coords[n++] = coords[i];
        e->events->coordEventCount = n;
    }
    // added warps after the map's own (theirs keep their numbers: other maps' doors lead to them by number)
    {
        u32 n = be->warpCount;
        for (j = 0; j < e->addCount; j++)
        {
            const struct AddEdit *a = &e->adds[j];
            const struct MakerMapName *dn;
            if (a->kind != ADD_WARP)
                continue;
            memset(&warps[n], 0, sizeof(warps[n]));
            warps[n].x = a->v[PROP_X];
            warps[n].y = a->v[PROP_Y];
            warps[n].elevation = a->has[PROP_ELEVATION] ? a->v[PROP_ELEVATION] : 0;
            dn = a->has[PROP_DEST] ? Maker_Map(a->v[PROP_DEST] >> 8) : mn;
            warps[n].mapGroup = dn->mapGroup;
            warps[n].mapNum = dn->mapNum;
            warps[n].warpId = a->has[PROP_DEST] ? a->v[PROP_DEST] & 0xFF : 0;
            n++;
        }
        e->events->warpCount = n;
    }
    free(objGone);
    free(bgGone);
    free(coordGone);
    e->events->objectEvents = objs;
    e->events->warps = warps;
    e->events->bgEvents = bgs;
    e->events->coordEvents = coords;
    e->header->events = e->events;
    BuildConnections(e, base);
    if (e->skipped)
        printf("[maker] %s: %u edits left out (the game changed there since)\n", mn->name, e->skipped);
    return e->header;
}

// overworld.c Overworld_GetMapHeaderByGroupAndId: an edited map's header, or NULL for the game's own
const struct MapHeader *Maker_EditedHeader(u16 mapGroup, u16 mapNum)
{
    int map;
    struct MapEdits *e;
    if (!sAny)
        return NULL;
    map = Maker_MapIndex(mapGroup, mapNum);
    e = EditsFor(map);
    if (e == NULL || (e->propCount == 0 && e->addCount == 0 && !e->hasMusic))
        return NULL;
    return BuildHeader(map, e);
}

// Which map a header is (the current map's copy, an edited copy, or the game's own)
static int MapOfHeader(const struct MapHeader *h)
{
    int i;
    if (h == &gMapHeader)
        return Maker_MapIndex(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    for (i = 0; i < Maker_MapCount() && i < (int)ARRAY_COUNT(sEdits); i++)
    {
        const struct MakerMapName *m;
        if (sEdits[i] && sEdits[i]->header == h)
            return i;
        m = Maker_Map(i);
        if (Maker_BaseMapHeader(m->mapGroup, m->mapNum) == h)
            return i;
    }
    return -1;
}

// fieldmap.c: a map's blocks as the field copies them (SINNOH's from the 3D pack), with the user's tiles
const u16 *Maker_MapBlocks(const struct MapHeader *h)
{
    const u16 *base = S3D_LayoutBlocks(h->mapLayout);
    if (base == h->mapLayout->map)
        base = O3D_LayoutBlocks(h->mapLayout);   // Moemon Kai: ORRE's from its own pack (src/orre/o3d.c)
    struct MapEdits *e;
    int map;
    u32 i, n, skipped = 0;
    if (!sAny)
        return base;
    map = MapOfHeader(h);
    e = EditsFor(map);
    if (e == NULL || e->blockCount == 0)
        return base;
    if (e->patchedBlocks && e->patchedFrom == base)
        return e->patchedBlocks;
    n = h->mapLayout->width * h->mapLayout->height;
    free(e->patchedBlocks);
    e->patchedBlocks = malloc(n * sizeof(u16));
    memcpy(e->patchedBlocks, base, n * sizeof(u16));
    e->patchedFrom = base;
    for (i = 0; i < e->blockCount; i++)
    {
        const struct BlockEdit *b = &e->blocks[i];
        u32 at = b->y * h->mapLayout->width + b->x;
        if (b->x < h->mapLayout->width && b->y < h->mapLayout->height && base[at] == b->was)
            e->patchedBlocks[at] = b->now;
        else
            skipped++;
    }
    if (skipped)
        printf("[maker] %s: %u tiles left out (the game changed there since)\n", Maker_Map(map)->name, skipped);
    return e->patchedBlocks;
}

// special Maker_ItemBallItem (data/scripts/maker.inc): the item ball talked to's item, for STD_FIND_ITEM
void Maker_ItemBallItem(void)
{
    u32 i;
    gSpecialVar_0x8000 = ITEM_NONE;
    gSpecialVar_0x8001 = 1;
    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
        if (gSaveBlock1Ptr->objectEventTemplates[i].localId == gSpecialVar_LastTalked)
        {
            gSpecialVar_0x8000 = gSaveBlock1Ptr->objectEventTemplates[i].trainerRange_berryTreeId;
            break;
        }
}

// mp_region.c MP_Region_WildInfo: a table with the user's slot edits, on top of what the game picked (`now`: the
// GENERATION's pools from the table as built, `base`); an edit applies only while `base` still has its WAS
const struct WildPokemonInfo *Maker_WildInfo(const struct WildPokemonInfo *base, const struct WildPokemonInfo *now)
{
    bool8 Maker_WildInfoOwner(const struct WildPokemonInfo *info, int *map, int *area);
    static struct WildPokemon sMons[12];
    static struct WildPokemonInfo sInfo;
    struct MapEdits *e;
    int map, area, count;
    u32 i;
    bool8 any = FALSE;
    if (!sAny || base == NULL || now == NULL || !Maker_WildInfoOwner(base, &map, &area))
        return now;
    e = EditsFor(map);
    if (e == NULL || e->wildCount == 0)
        return now;
    count = MakerApi_WildSlotCount(area);
    for (i = 0; i < e->wildCount; i++)
    {
        const struct WildEdit *w = &e->wild[i];
        const struct WildPokemon *b;
        s32 v;
        if (w->area != area || w->slot >= count)
            continue;
        if (!any)
        {
            memcpy(sMons, now->wildPokemon, sizeof(struct WildPokemon) * count);
            sInfo = *now;
            sInfo.wildPokemon = sMons;
            any = TRUE;
        }
        b = &base->wildPokemon[w->slot];
        v = w->prop == 0 ? b->species : w->prop == 1 ? b->minLevel : b->maxLevel;
        if (v != w->was)
            continue;
        if (w->prop == 0)
            sMons[w->slot].species = w->now;
        else if (w->prop == 1)
            sMons[w->slot].minLevel = w->now;
        else
            sMons[w->slot].maxLevel = w->now;
    }
    return any ? &sInfo : now;
}
