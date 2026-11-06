#ifndef GUARD_MAKER_H
#define GUARD_MAKER_H

// Moemon Maker: the game's side (docs/MOEMON_MAKER.md). The editor itself is C++ in maker/ and only talks to the game
// through include/maker_api.h.

// The way in (src/maker/maker_gate.c)
void Maker_Typed(void);
bool8 Maker_Unlocked(void);
bool8 Maker_TakeMenuRefresh(void);
void Maker_FrameUpdate(void);
void Maker_InitFromEnv(void);

// The names the editor shows and edits are saved by (src/maker/maker_names.c, from the build's own constants and
// map.json files: tools/maker/gen_names.py)
struct MakerMapName { const char *name; const char *group; unsigned char mapGroup, mapNum; };
struct MakerObjName { const char *localId, *gfx, *script; };
struct MakerBgName { const char *type, *script; };
struct MakerCoordName { const char *var, *script; };
struct MakerMapEventNames { const char *map; unsigned short obj, objCount, bg, bgCount, coord, coordCount; };
struct MakerNamedValue { const char *name; unsigned int value; };
struct MakerSpecialName { const char *name; unsigned char waitstate; };
int Maker_SpecialIndex(const char *name, bool8 *waits);   // -1 if none
int Maker_MapCount(void);
const struct MakerMapName *Maker_Map(int index);
int Maker_MapIndex(u16 mapGroup, u16 mapNum);   // -1 if none
const struct MakerMapEventNames *Maker_MapEventNames(int index);
const struct MakerObjName *Maker_ObjName(int i);
const struct MakerBgName *Maker_BgName(int i);
const struct MakerCoordName *Maker_CoordName(int i);
const struct MapHeader *Maker_BaseMapHeader(u16 mapGroup, u16 mapNum);   // the map as built (never the edited one)

// Flags for what the edits add (src/maker/maker_flags.c): ids past the save's, kept by name in "<save>.maker"
#define MAKER_FLAGS_START 0x8000
#define MAKER_FLAGS_COUNT 0x2000
u16 Maker_FlagId(const char *name);
u16 Maker_FlagFind(const char *name);   // (lookup only)
extern bool8 gMakerDebugOn;   // (M4: the debugger, maker_events.c)
bool8 Maker_DebugHold(const u8 *ptr);
void Maker_DebugReset(void);
u8 *Maker_FlagPointer(u16 id);
void Maker_Flags_NewGame(void);
void Maker_Flags_LoadFromSave(void);
void Maker_Flags_WriteSidecar(void);

// New maps (M2b: src/maker/maker_newmaps.c): map group MAKER_MAP_GROUP, layout ids from MAKER_LAYOUT_FIRST, in the
// Maker's map list after the game's own
#define MAKER_MAP_GROUP 47   // MAP_GROUPS_COUNT: one past the game's own groups
#define MAKER_LAYOUT_FIRST 0x4000
#define MAKER_NEW_MAPS_MAX 300
int Maker_NewMapCount(void);
const struct MakerMapName *Maker_NewMapName(int n);
int Maker_NewMapRegion(int n);
const struct MapHeader *Maker_NewMapHeader(u16 num);
const struct MapLayout *Maker_NewMapLayout(u16 layoutId);
int Maker_NewMapFromText(const char *text);
void Maker_NewMaps_Load(void);
int Maker_NewMapBlock(int n, int x, int y);

// Trainers (M2c: src/maker/maker_trainers.c)
void Maker_LoadTrainers(void);
void Maker_LoadDatabase(void);   // (M3a: maker_database.c)
void Maker_LoadArt(void);   // (M3: maker_art.c)
int Maker_SetDatabase(const char *text);
int Maker_SetTrainers(const char *text);
u32 Maker_TrainerHash(u16 id);

// Events (M2: src/maker/maker_events.c): compiled at run time into the game's own script commands
enum { MAKER_EVENT_ON_PERSON, MAKER_EVENT_ON_SIGN, MAKER_EVENT_ON_TRIGGER, MAKER_EVENT_ON_MAP };
const u8 *Maker_EventScript(const char *map, const char *event, u8 kind, const char *source);
const u8 *Maker_EventAutorun(const char *map, const char *event, const char *source);
bool8 Maker_TryAutorun(void);

// Playing from the editor, F5 (src/maker/maker_play.c)
bool8 Maker_IsPlaying(void);
bool8 Maker_PlayNewGamePending(void);
bool8 Maker_PlayNewGame(void);
void Maker_StopPlay(void);
void Maker_PlayFrame(void);
void Maker_EditsChangedWhilePlaying(int map, bool8 blocksOnly);
void Maker_BackToWaiting(void);

// The editor (src/maker/maker_main.c)
void Maker_Open(void);          // from the main menu: the editor window opens, the game waits on a black screen
bool8 Maker_IsOpen(void);

#endif // GUARD_MAKER_H
