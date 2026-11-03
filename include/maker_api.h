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

#ifdef __cplusplus
}
#endif

#endif // GUARD_MAKER_API_H
