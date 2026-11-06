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
int MakerApi_SetNewMap(const char *text);   // M2b: a new map made or changed (MoemonMaker/maps/MAP_X.txt); its index
int MakerApi_IsNewMap(int index);           // 1 for a map the user made
int MakerApi_TilesetCount(void);
const char *MakerApi_TilesetName(int i, int *secondary);

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
int MakerApi_Render3D(int map, int tileX, int tileY, unsigned int *rgba);   // SINNOH: 240 x 160 RGBA; 0 if it can't

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
       MAKER_NAMES_SPECIES, MAKER_NAMES_FLAG, MAKER_NAMES_VAR, MAKER_NAMES_TRAINER, MAKER_NAMES_MAPSEC, MAKER_NAMES_MOVE,
       MAKER_NAMES_TYPE, MAKER_NAMES_ABILITY, MAKER_NAMES_COUNT };
int MakerApi_NameCount(int table);
const char *MakerApi_NameAt(int table, int i, unsigned int *value);
const char *MakerApi_NameOf(int table, unsigned int value);       // "" if it has none
int MakerApi_ValueOf(int table, const char *name, unsigned int *value);   // 0 if no such name
// Trainers (M2c: src/maker/maker_trainers.c): the game's own, and the user's edits (MoemonMaker/trainers.txt)
struct MakerTrainerMon { int species, level, iv, item, moves[4]; };
struct MakerTrainerInfo
{
    char name[16];
    int trainerClass, pic, music, doubleBattle, partySize;
    unsigned int ai, hash;
    int items[4];
    struct MakerTrainerMon party[6];
};
int MakerApi_TrainerInfo(int id, struct MakerTrainerInfo *info);   // 0 if there's no such trainer
int MakerApi_SetTrainers(const char *text);
// The database (M3a: src/maker/maker_database.c): the game's own Pokémon, moves and items, and the user's edits
// (MoemonMaker/database.txt)
enum { MAKER_DB_SPECIES, MAKER_DB_MOVE, MAKER_DB_ITEM };
#define MAKER_NEW_SPECIES_FIRST 1051   // (M3b: the new POKéMON, include/constants/species.h SPECIES_MAKER_FIRST..LAST)
#define MAKER_NEW_SPECIES_LAST 1178
#define MAKER_NEW_MOVE_FIRST 848   // (moves.h MOVE_MAKER_FIRST..LAST)
#define MAKER_NEW_MOVE_LAST 911
#define MAKER_NEW_ITEM_FIRST 746   // (items.h ITEM_MAKER_FIRST..LAST)
#define MAKER_NEW_ITEM_LAST 809
#define MAKER_LEARNSET_MAX 128
struct MakerSpeciesInfo
{
    char name[24];
    int stats[6], types[2], abilities[2], evs[6], items[2], eggGroups[2];
    int catchRate, expYield, gender, eggCycles, friendship, growth, flee, color;
    int learnCount, learnLevel[MAKER_LEARNSET_MAX], learnMove[MAKER_LEARNSET_MAX];
    unsigned int learnHash;
};
struct MakerMoveInfo { char name[24]; int power, type, accuracy, pp, chance, target, priority, flags, effect, category; };
struct MakerItemInfo { char name[24]; char description[400]; int price, holdEffect, holdParam, pocket; unsigned int descriptionHash; };
int MakerApi_DatabaseCount(int kind);
int MakerApi_SpeciesInfo(int id, struct MakerSpeciesInfo *info);   // 0 if there's no such Pokémon
int MakerApi_MoveInfo(int id, struct MakerMoveInfo *info);
int MakerApi_ItemInfo(int id, struct MakerItemInfo *info);
int MakerApi_SetDatabase(const char *text);   // the edited entries
int MakerApi_DatabaseConflicts(void);
// The battle bench's damage estimate (perfect IVs, no EVs, neutral natures; before abilities, items and weather)
struct MakerDamage { int min, max, defenderHp, effectiveness, stab; };   // effectiveness in percent
int MakerApi_Damage(int atkSpecies, int atkLevel, int defSpecies, int defLevel, int move, struct MakerDamage *out);
// Pictures (M3: src/maker/maker_art.c): MoemonMaker/art/SPECIES_X_front / _back / _shiny.png
void MakerApi_ReloadArt(void);
int MakerApi_SpeciesPicture(int species, int back, int shiny, unsigned char *rgba);   // 64x64 RGBA as the game shows it   // lines left out by the last MakerApi_SetDatabase: the game changed under them
int MakerApi_TextWidth(const char *utf8);   // in the message box's font, in pixels (M2c's checker)
const char *MakerApi_CheckEvent(const char *map, const char *event, int kind, const char *source);   // "" when it compiles
// (M4b) the body of an event that does what the game's script SCRIPTNAME does (kind 0 person, 1 sign, 2 trigger), from
// the build's pack of the game's scripts (src/maker/maker_scripts.c), or NULL when there is none
const char *MakerApi_GameScript(const char *scriptName, int kind);
// (M4c) edits the game left out because it changed under them since they were made: KEY as the editor's ("person KEY
// look", "music", "connection 0 offset", "block X Y", "event NAME"), WAS (the game's then), NOW (the edit), BASE (the
// game's now; GONE: nothing by that key any more). Returns how many (OUT gets up to MAX).
struct MakerStale { char key[128]; long was, now, base; int gone; };
// (M5) Lua hooks (src/maker/maker_lua.cpp): MoemonMaker/lua/*.lua loaded again, a script's text checked ("" when it
// compiles), the last problem the game had loading or running one ("" when none)
void MakerApi_LuaReload(void);
// (M5) tileset edits (src/maker/maker_tilesets.c): a tileset's file text as the editor makes it; the game's own entry
// (SLOT 0-7) or attributes (SLOT -1) of a metatile; how many of its edits were left out
void MakerApi_SetTilesetEdits(const char *name, const char *text);
int MakerApi_TilesetBase(const char *name, int id, int slot);
int MakerApi_TilesetSkipped(const char *name);
void MakerApi_RenderTile(int entry, unsigned int *rgba);   // 8x8 RGBA of one tile entry, as the loaded map's tiles show it
const char *MakerApi_BehaviorName(int behavior);           // MB_ name, "" when none
// (M5) move animations (src/maker/maker_anims.c): loaded again from MoemonMaker/anims; frame FRAME of an animation's text
// on a stand-in scene (240x160 RGBA), returning its frame count (0: the text isn't an animation)
void MakerApi_AnimReload(void);
int MakerApi_AnimPreview(const char *text, int frame, unsigned int *rgba);
// (M5) SINNOH's 3D models (src/maker/maker_models.c): MoemonMaker/models/MAP_X.txt changed (placed again if playing there)
void MakerApi_ModelsChanged(const char *mapName);
const char *MakerApi_LuaCheck(const char *name, const char *text);
const char *MakerApi_LuaError(void);
int MakerApi_Stale(int map, struct MakerStale *out, int max);
int MakerApi_SpecialCount(void);
const char *MakerApi_SpecialName(int i);

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
const char *MakerApi_EditsDir(void);   // MoemonMaker/ beside the game: the editor's working copy
const char *MakerApi_PackDir(void);    // MoemonMakerPack/ beside the game: the published pack
void MakerApi_ReloadEdits(void);       // the saved edits again (the editor's unsaved ones dropped)

// Playing from the editor, F5 (src/maker/maker_play.c)
enum { MAKER_PLAY_SAVE, MAKER_PLAY_START, MAKER_PLAY_BADGES4, MAKER_PLAY_CHAMPION, MAKER_PLAY_TEST, MAKER_PLAY_KINDS };
int MakerApi_Play(int map, int x, int y, int kind, int slot);
// The debugger (M4: maker_events.c): breakpoints on a Maker event's lines, step / continue, watched values
int MakerApi_DebugSetBreak(const char *map, const char *event, int line, int on);
int MakerApi_DebugHasBreak(const char *map, const char *event, int line);
void MakerApi_DebugClearBreaks(void);
int MakerApi_DebugPaused(char *map, int mapSize, char *event, int eventSize, int *line);   // 1 while a script is held
void MakerApi_DebugContinue(void);
void MakerApi_DebugStep(void);
int MakerApi_DebugValue(const char *name, int *value);   // 0 if there's no such flag / variable
int MakerApi_PlayBattle(int map, int x, int y, int kind, int slot, int trainer);   // (M3: the battle bench)   // slot: the save file copied (1-3; 0 = the one in use)
void MakerApi_StopPlay(void);
int MakerApi_IsPlaying(void);
unsigned int MakerApi_GameWindowId(void);   // the game's own window (F12 / closing it ends the test)

// Publish (src/maker/maker_publish.c): the working copy's files to the game's repository through GitHub's API, as one
// commit; the publish history and rolling back to one
enum { MAKER_PUBLISH_IDLE, MAKER_PUBLISH_WORKING, MAKER_PUBLISH_DONE, MAKER_PUBLISH_FAILED, MAKER_PUBLISH_ROLLED_BACK };
int MakerApi_Publish(const char *token, const char *message);
int MakerApi_PublishHistory(const char *token);
int MakerApi_PublishRollback(const char *token, const char *sha);
int MakerApi_PublishState(const char **result);
void MakerApi_PublishAcknowledge(void);
int MakerApi_PublishHistoryCount(void);
int MakerApi_PublishHistoryItem(int i, const char **sha, const char **date, const char **message);

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
