#ifndef GUARD_MAKER_API_H
#define GUARD_MAKER_API_H

// Moemon Maker: everything the editor (C++, maker/) asks of the game (C, src/maker/maker_api.c). Plain C types only, so
// the editor never includes the game's headers. docs/MOEMON_MAKER.md.

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    MAKER_REGION_HOENN,
    MAKER_REGION_JOHTO,
    MAKER_REGION_KANTO,
    MAKER_REGION_SINNOH,
    MAKER_REGION_COUNT
};

struct MakerMapInfo
{
    const char *name;        // the map's constant, e.g. "MAP_ROUTE101": what overrides are saved by
    const char *groupName;   // its map group's, e.g. "gMapGroup_TownsAndRoutes"
    char title[48];          // shown in the editor: the name without MAP_ and its region, e.g. "ROUTE101" -> "ROUTE 101"
    char place[40];          // the game's own name for where it is (its map section), e.g. "LITTLEROOT TOWN" (UTF-8)
    int region;              // MAKER_REGION_*
    int mapGroup, mapNum;
    int width, height;       // in metatiles (16x16 pixels)
    int music;               // the song number in its header
};

int MakerApi_MapCount(void);
int MakerApi_MapInfo(int index, struct MakerMapInfo *info);   // 0 if there's no such map
int MakerApi_FindMap(const char *name);                      // its index, or -1

// A map's tiles (src/maker/maker_tiles.c): loaded into the game's own BG VRAM while the editor is open, animated by
// the game's own tile animations
struct MakerTilesInfo
{
    int width, height;                 // the map, in metatiles
    int primaryMetatiles;              // metatiles 0.. of the primary tileset
    int secondaryStart;                // the secondary's first metatile id (512, or 640 for FRLG-style primaries)
    int secondaryMetatiles;
    char primaryName[40], secondaryName[40];
    int sinnoh;                        // SINNOH: blocks from the 3D pack, behavior in the id (no tile pictures)
};
int MakerApi_LoadMapTiles(int index, struct MakerTilesInfo *info);
void MakerApi_UnloadMapTiles(void);
void MakerApi_RenderMetatile(int id, unsigned int *rgba, int layers);   // 16x16 RGBA; layers 1 bottom, 2 top, 3 both
int MakerApi_MetatileBehavior(int id);
int MakerApi_MetatileLayerType(int id);
int MakerApi_MapBlock(int index, int x, int y);   // metatile | collision << 10 | elevation << 12, or -1
void MakerApi_SetAnimating(int on);
int MakerApi_TakeTilesChanged(void);   // 1 once after the tiles changed (loaded, or an animation frame)
enum
{
    MAKER_BEHAVIOR_NONE, MAKER_BEHAVIOR_GRASS, MAKER_BEHAVIOR_WATER, MAKER_BEHAVIOR_WATERFALL, MAKER_BEHAVIOR_CAVE,
    MAKER_BEHAVIOR_DOOR, MAKER_BEHAVIOR_WARP, MAKER_BEHAVIOR_LEDGE_E, MAKER_BEHAVIOR_LEDGE_W, MAKER_BEHAVIOR_LEDGE_N,
    MAKER_BEHAVIOR_LEDGE_S, MAKER_BEHAVIOR_ICE, MAKER_BEHAVIOR_COUNTER,
};
int MakerApi_BehaviorClass(int behavior);   // MAKER_BEHAVIOR_*, by the game's own checks

// A map's people, warps, signs and triggers (src/maker/maker_objects.c): the map's own, named by map.json
struct MakerObjectInfo
{
    int localId, graphicsId, kind;
    int x, y, elevation;
    int movementType, rangeX, rangeY;
    int trainerType, sight;            // (sight: a trainer's sight range, or a berry tree's id)
    int flag;                          // its hide flag (0: always there)
    int facing;                        // the way it faces when the map loads: 0 south, 1 north, 2 west, 3 east
    const char *localIdName;           // "LOCALID_LITTLEROOT_TWIN", or "" (then it's known by its script)
    const char *gfxName, *scriptName;
};
struct MakerWarpInfo { int x, y, elevation, destMap, destWarp; };
struct MakerBgInfo { int x, y, elevation, kind; const char *typeName, *scriptName; };
struct MakerCoordInfo { int x, y, elevation; const char *varName, *scriptName; };
int MakerApi_ObjectCount(int map);
int MakerApi_ObjectInfo(int map, int i, struct MakerObjectInfo *o);
int MakerApi_WarpCount(int map);
int MakerApi_WarpInfo(int map, int i, struct MakerWarpInfo *w);
int MakerApi_BgCount(int map);
int MakerApi_BgInfo(int map, int i, struct MakerBgInfo *b);
int MakerApi_CoordCount(int map);
int MakerApi_CoordInfo(int map, int i, struct MakerCoordInfo *c);
// A look's standing picture (facing 0 south, 1 north, 2 west, 3 east) as RGBA (0 = clear); 0 if none fits maxW x maxH
int MakerApi_RenderObject(int graphicsId, int facing, unsigned int *rgba, int maxW, int maxH, int *w, int *h);
int MakerApi_MovementFacing(int movementType);
int MakerApi_ConnectionCount(int map);
int MakerApi_ConnectionInfo(int map, int i, int *direction, int *offset, int *destMap);   // direction 1 down 2 up 3 left 4 right 5 dive 6 emerge

// Named values (songs, looks, movement types, trainer types, items, species, flags): MAKER_NAMES_*
enum { MAKER_NAMES_SONG, MAKER_NAMES_GFX, MAKER_NAMES_MOVEMENT, MAKER_NAMES_TRAINERTYPE, MAKER_NAMES_ITEM,
       MAKER_NAMES_SPECIES, MAKER_NAMES_FLAG, MAKER_NAMES_COUNT };
int MakerApi_NameCount(int table);
const char *MakerApi_NameAt(int table, int i, unsigned int *value);
const char *MakerApi_NameOf(int table, unsigned int value);       // "" if it has none
int MakerApi_ValueOf(int table, const char *name, unsigned int *value);   // 0 if no such name

// A map's wild Pokémon as built (src/maker/maker_wild.c)
enum { MAKER_WILD_LAND, MAKER_WILD_WATER, MAKER_WILD_ROCK, MAKER_WILD_FISH, MAKER_WILD_AREAS };
struct MakerWildSlot { int species, minLevel, maxLevel; };
int MakerApi_WildSlotCount(int area);   // 12, 5, 5, 10 (fishing: OLD ROD 0-1, GOOD ROD 2-4, SUPER ROD 5-9)
int MakerApi_WildArea(int map, int area, struct MakerWildSlot *slots, int *rate);   // its slot count, 0 if none

// Music (src/maker/maker_music.c): the game's songs and the user's new tracks (MoemonMaker/music/), played through
// the game's own player
int MakerApi_TrackCount(void);
const char *MakerApi_TrackName(int i);
int MakerApi_SongId(const char *name);         // MUS_X or a new track's name; -1 if none
const char *MakerApi_SongName(int song);
const char *MakerApi_SongFileName(int song);   // the lowercase name its files use (mus_route101)
int MakerApi_SceneCount(void);
const char *MakerApi_SceneKey(int i);         // the scenes a song can be replaced in (tcg, sim...)
void MakerApi_PlaySong(int song);
void MakerApi_StopSong(void);
void MakerApi_MusicChanged(void);

// The editor's edits of one map, as the text of its MoemonMaker/base/MAP_X.txt (src/maker/maker_overrides.c)
void MakerApi_SetMapEdits(const char *mapName, const char *text);
const char *MakerApi_EditsDir(void);   // MoemonMaker/ beside the game
void MakerApi_ReloadEdits(void);       // the saved edits again (the editor's unsaved ones dropped)

// Playing from the editor, F5 (src/maker/maker_play.c)
enum { MAKER_PLAY_SAVE, MAKER_PLAY_START, MAKER_PLAY_BADGES4, MAKER_PLAY_CHAMPION, MAKER_PLAY_TEST, MAKER_PLAY_KINDS };
int MakerApi_Play(int map, int x, int y, int kind, int slot);   // slot: the save file copied (1-3; 0 = the one in use)
void MakerApi_StopPlay(void);
int MakerApi_IsPlaying(void);
unsigned int MakerApi_GameWindowId(void);   // the game's own window (F12 / closing it ends the test)

// The game side of the editor (src/maker/maker_main.c)
void MakerApi_Close(void);            // the editor closed: back to the main menu
const char *MakerApi_GameVersion(void);

// The editor (maker/maker_ui.cpp), called by the platform layer (src/platform/sdl2.c)
int MakerUi_Open(void);               // the editor's window; 0 if it couldn't be made
void MakerUi_Close(void);
int MakerUi_IsOpen(void);
int MakerUi_HandleEvent(const void *sdlEvent);   // 1 if the event was the editor's
void MakerUi_Frame(void);             // draws the editor (at most 60 times a second)
void MakerUi_TestCommand(const char *line);      // tests: "maker ..." in a MOEMON_TEST script
int MakerUi_SaveShot(const char *path);          // tests: the editor's window as a BMP
void MakerUi_PlayStopped(void);                  // back from F5: the map's tiles loaded again

#ifdef __cplusplus
}
#endif

#endif // GUARD_MAKER_API_H
