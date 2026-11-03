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
int Maker_MapCount(void);
const struct MakerMapName *Maker_Map(int index);
int Maker_MapIndex(u16 mapGroup, u16 mapNum);   // -1 if none
const struct MakerMapEventNames *Maker_MapEventNames(int index);
const struct MakerObjName *Maker_ObjName(int i);
const struct MakerBgName *Maker_BgName(int i);
const struct MakerCoordName *Maker_CoordName(int i);
const struct MapHeader *Maker_BaseMapHeader(u16 mapGroup, u16 mapNum);   // the map as built (never the edited one)

// The editor (src/maker/maker_main.c)
void Maker_Open(void);          // from the main menu: the editor window opens, the game waits on a black screen
bool8 Maker_IsOpen(void);

#endif // GUARD_MAKER_H
