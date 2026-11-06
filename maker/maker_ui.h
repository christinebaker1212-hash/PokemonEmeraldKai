// Moemon Maker: the editor's shared state and pieces (maker/*.cpp). docs/MOEMON_MAKER.md.
#ifndef MAKER_UI_H
#define MAKER_UI_H

#include <stdlib.h>
#include <map>
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

enum { SEL_NONE, SEL_OBJECT, SEL_WARP, SEL_BG, SEL_COORD, SEL_ADDED };

enum Icon
{
    ICON_SAVE, ICON_UNDO, ICON_REDO,
    ICON_MODE_MAP, ICON_MODE_COLLISION, ICON_MODE_EVENTS, ICON_MODE_ENCOUNTERS,
    ICON_PENCIL, ICON_RECT, ICON_FILL, ICON_PICKER,
    ICON_ZOOM_OUT, ICON_ZOOM_IN, ICON_PLAY, ICON_STOP,
};

struct PropEdit
{
    long was, now;
    bool operator==(const PropEdit &o) const { return was == o.was && now == o.now; }
};

// An event (M2): what it's on ("person:KEY", "sign:KEY", "trigger:KEY", "added" for an added person / sign's
// script=event:NAME, "map" for autorun only) and its pages and commands, one a line, indented by two spaces a level
struct EventDef
{
    std::string on, body;
    bool operator==(const EventDef &o) const { return on == o.on && body == o.body; }
};

// A map's edits (maker_edits.cpp): tiles by y * w + x, properties by "KIND KEY PROP" (person LOCALID_X x)
struct MapEdits
{
    int w = 0, h = 0;
    std::map<int, std::pair<int, int>> blocks;   // was, now
    std::map<std::string, PropEdit> props;
    std::map<std::string, std::map<std::string, std::string>> adds;   // "person 1" -> {"x": "10", "look": "OBJ_..."}
    std::map<std::string, EventDef> events;                             // by name
    bool Empty() const { return blocks.empty() && props.empty() && adds.empty() && events.empty(); }
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
    std::string selAdded;             // SEL_ADDED: its key in the map's adds ("person 1")
    int recovered = 0;                // maps whose unsaved edits came back from the recovery files
    std::string message;              // the status bar's last message ("Saved 3 maps")
    std::string title;
    bool musicOpen = false;           // Tools > Music
    bool publishOpen = false;         // File > Publish
    std::string githubToken;          // Publish's (moemon_maker.ini: this PC only)
    int playKind = 0, playSlot = 1;   // F5's last pick (moemon_maker.ini)
    bool wantPlay = false;            // F5 / the Play button: the "Play as" question
    int playX = -1, playY = -1;       // where F5 starts
    unsigned int messageUntil = 0;
};

extern Editor gEd;

void ApplyTheme();
void SaveSettings();
void SelectMap(int index);

// maker_edits.cpp
std::string EditsDir();
const MapEdits *FindEdits(int map);
MapEdits &Edits(int map);
unsigned EditsVersion();                                        // (counts every change sent to the game)
std::string EditValueText(const std::string &prop, long v);     // a property's value as the edit file writes it
bool Merge_Section(int map);                                    // (M4c: maker_merge.cpp) the GAME CHANGED section
void Merge_TestCommand(const char *cmd, const char *arg);
int CurrentBlock(int map, int x, int y, int base);
void SetBlock(int map, int x, int y, int base, int now);
long CurrentProp(int map, const char *kind, const std::string &key, const char *prop, long base);
void SetProp(int map, const char *kind, const std::string &key, const char *prop, long base, long now);
std::string EditsText(int map);
long CurrentMusic(int map, long base);
void SetMusic(int map, long base, long now);
void LoadAllEdits();
void SendToGame(int map);
void SendAllToGame();
void BeginChange(int map);
void EndChange();
bool InChange();
bool CanUndo();
bool CanRedo();
void Undo();
void Redo();
bool AnyUnsaved();
bool MapUnsaved(int map);
int EditedMapCount();
int SaveAll();
void RecoveryTick();
void DiscardUnsaved();
void Message(const char *fmt, ...);

// maker_events_ui.cpp (M2: events)
void EventSection(const char *on, const std::string &suggestedName, std::map<std::string, std::string> *added,
                  const char *gameScript = nullptr);   // (M4b: the game's script by name, to start from)
void EventPickerWindow();
void QuickEventMenu(int x, int y);
void Events_TestCommand(const char *cmd, const char *arg);
std::string EventOn(int map, const std::string &on);   // the event on something, by name ("" if none)

// maker_world.cpp (M2b: new maps, the World view)
void NewMapWindow();
void OpenNewMap();
void WorldWindow();
void OpenWorld();
void World_TestCommand(const char *cmd, const char *arg);

// maker_trainers_ui.cpp (M2c: trainers)
void TrainersWindow();
void OpenTrainers();
int Trainers_Save();
bool Trainers_Unsaved();
void Trainers_TestCommand(const char *cmd, const char *arg);

// maker_debug_ui.cpp (M4: the debugger)
void DebuggerWindow();
void LuaWindow();      // (M5: maker_lua_ui.cpp)
void OpenLua();
void Lua_TestCommand(const char *cmd, const char *arg);
void OpenDebugger();
void Debugger_TestCommand(const char *cmd, const char *arg);

// maker_database_ui.cpp (M3a: the database)
void DatabaseWindow();
void OpenDatabase();
int Database_Save();
bool Database_Unsaved();
void Database_TestCommand(const char *cmd, const char *arg);

// maker_wild.cpp
void WildPanel();
void Wild_TestCommand(const char *cmd, const char *arg);

// maker_music.cpp
void MusicWindow();
bool SongPicker(const char *id, int *song);
void Music_TestCommand(const char *cmd, const char *arg);
void Music_Dropped(const std::string &path);

// maker_publish.cpp
void PublishWindow();
int Publish_ErrorCount();
void Publish_TestCommand(const char *cmd, const char *arg);

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
void MapView_EditsChanged(int map);
void MapView_Problems();     // the PROBLEMS panel's entries
void MapView_MapInspector(); // the map's own: its music, its connections
int MapView_ProblemCount();
bool MapView_SelectedSpot(int *x, int *y);   // the picked event's tile

} // namespace maker

#endif // MAKER_UI_H
