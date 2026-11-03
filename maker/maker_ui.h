// Moemon Maker: the editor's shared state and pieces (maker/*.cpp). docs/MOEMON_MAKER.md.
#ifndef MAKER_UI_H
#define MAKER_UI_H

#include <stdlib.h>
#include <string>
#include <vector>
#include "imgui.h"
extern "C" {
#include "maker_api.h"
}

struct SDL_Window;
struct SDL_Renderer;

namespace maker {

enum { MODE_MAP, MODE_COLLISION, MODE_EVENTS, MODE_ENCOUNTERS, MODE_COUNT };
enum { TOOL_PENCIL, TOOL_RECT, TOOL_FILL, TOOL_PICKER, TOOL_COUNT };
static const char *const kModeNames[MODE_COUNT] = {"Map", "Collision", "Events", "Encounters"};
static const char *const kModeKeys[MODE_COUNT] = {"F1", "F2", "F3", "F4"};
static const char *const kToolNames[TOOL_COUNT] = {"Pencil", "Rectangle", "Fill", "Picker"};
static const char *const kToolKeys[TOOL_COUNT] = {"P", "R", "F", "I"};

enum { SEL_NONE, SEL_OBJECT, SEL_WARP, SEL_BG, SEL_COORD };

enum Icon
{
    ICON_SAVE, ICON_UNDO, ICON_REDO,
    ICON_MODE_MAP, ICON_MODE_COLLISION, ICON_MODE_EVENTS, ICON_MODE_ENCOUNTERS,
    ICON_PENCIL, ICON_RECT, ICON_FILL, ICON_PICKER,
    ICON_ZOOM_OUT, ICON_ZOOM_IN, ICON_PLAY,
};

struct Place
{
    std::string name;
    std::vector<int> maps;
};

struct Editor
{
    SDL_Window *win = nullptr;
    SDL_Renderer *ren = nullptr;
    ImGuiContext *ctx = nullptr;
    ImFont *font = nullptr, *fontBold = nullptr;
    float uiScale = 1.0f;
    unsigned long long lastFrame = 0;
    bool testMode = false, wantClose = false;
    std::string shotPath;

    // settings (moemon_maker.ini)
    bool dark = false;
    int zoom = 2;
    int winW = 1280, winH = 800, winMax = 0;
    bool viewAnim = true, viewNpcs = true, viewFrame = true, viewGrid = true, viewIcons = true;
    std::string lastMap;

    // panels
    bool layoutBuilt = false, layoutReset = false;
    std::vector<MakerMapInfo> maps;
    std::vector<Place> places[MAKER_REGION_COUNT];
    char mapFilter[64] = "";
    int map = -1;            // the open map (index into maps)
    bool scrollToMap = false;
    int mode = MODE_MAP, tool = TOOL_PENCIL;
    int hoverX = -1, hoverY = -1;   // the tile under the mouse in the MAP panel
    int hoverBlock = -1;
    int selKind = 0, selIndex = -1;   // the selected event in the MAP panel: SEL_*
};

extern Editor gEd;

void ApplyTheme();
void SaveSettings();
void SelectMap(int index);

// maker_icons.cpp
bool IconButton(const char *id, int icon, const char *tooltip, bool selected, bool enabled);
void ToolbarSeparator();

// maker_mapview.cpp
void MapView_Opened();
void MapView_Closed();
void MapView_Panel();
void TilesPanel();
void MapView_TestCommand(const char *cmd, const char *arg);
int MapView_SelectedMetatile();
void MapView_Inspector();   // the selected event's details (the INSPECTOR panel)

} // namespace maker

#endif // MAKER_UI_H
