// Moemon Maker: the editor's window (docs/MOEMON_MAKER.md "The editor, laid out like MZ"). Dear ImGui on its own SDL
// window and renderer, opened from the main menu (src/maker/maker_main.c); the game's window is hidden meanwhile. The
// game is reached only through include/maker_api.h.
//
// Layout (RPG Maker's): menu bar, toolbar, then docked panels: TILES and MAPS on the left, the MAP in the middle, the
// INSPECTOR on the right, PROBLEMS below; a status bar at the bottom. Classic light look by default, dark in View.
// Settings in moemon_maker.ini, the panels' places in moemon_maker_layout.ini (beside the game's other settings).
#include <SDL.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "maker_ui.h"

extern "C" {
#include "maker_api.h"
extern const unsigned char gMakerFontRegular[], gMakerFontRegularEnd[];
extern const unsigned char gMakerFontBold[], gMakerFontBoldEnd[];
}

namespace maker {

Editor gEd;

static const char *const kRegionNames[MAKER_REGION_COUNT] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
static const char *const kSettingsFile = "moemon_maker.ini";
static const char *const kLayoutFile = "moemon_maker_layout.ini";

// ---------------------------------------------------------------------------------------------------------------------
// Settings

static void LoadSettings()
{
    FILE *f = fopen(kSettingsFile, "r");
    char line[256];
    if (!f)
        return;
    while (fgets(line, sizeof(line), f))
    {
        char key[64], val[160];
        if (sscanf(line, " %63[^= ] = %159[^\r\n]", key, val) != 2)
            continue;
        std::string k = key;
        if (k == "theme") gEd.dark = strcmp(val, "dark") == 0;
        else if (k == "zoom") gEd.zoom = atoi(val);
        else if (k == "last_map") gEd.lastMap = val;
        else if (k == "window") sscanf(val, "%d %d %d", &gEd.winW, &gEd.winH, &gEd.winMax);
        else if (k == "view_anim") gEd.viewAnim = atoi(val) != 0;
        else if (k == "view_npcs") gEd.viewNpcs = atoi(val) != 0;
        else if (k == "view_frame") gEd.viewFrame = atoi(val) != 0;
        else if (k == "view_grid") gEd.viewGrid = atoi(val) != 0;
        else if (k == "view_icons") gEd.viewIcons = atoi(val) != 0;
    }
    fclose(f);
    if (gEd.zoom < 1 || gEd.zoom > 8)
        gEd.zoom = 2;
}

void SaveSettings()
{
    FILE *f = fopen(kSettingsFile, "w");
    if (!f)
        return;
    fprintf(f, "# Moemon Maker's settings (written by the editor)\n");
    fprintf(f, "theme = %s\n", gEd.dark ? "dark" : "classic");
    fprintf(f, "zoom = %d\n", gEd.zoom);
    if (gEd.map >= 0)
        fprintf(f, "last_map = %s\n", gEd.maps[gEd.map].name);
    fprintf(f, "window = %d %d %d\n", gEd.winW, gEd.winH, gEd.winMax);
    fprintf(f, "view_anim = %d\nview_npcs = %d\nview_frame = %d\nview_grid = %d\nview_icons = %d\n",
            gEd.viewAnim, gEd.viewNpcs, gEd.viewFrame, gEd.viewGrid, gEd.viewIcons);
    fclose(f);
}

// ---------------------------------------------------------------------------------------------------------------------
// Looks: RPG Maker's classic light tool look, or dark

void ApplyTheme()
{
    ImGuiStyle &st = ImGui::GetStyle();
    st = ImGuiStyle();
    st.WindowRounding = 0;
    st.ChildRounding = 0;
    st.FrameRounding = 2;
    st.PopupRounding = 2;
    st.TabRounding = 2;
    st.ScrollbarRounding = 2;
    st.GrabRounding = 2;
    st.WindowBorderSize = 1;
    st.FrameBorderSize = 1;
    st.TabBorderSize = 1;
    st.WindowPadding = ImVec2(6, 6);
    st.FramePadding = ImVec2(6, 3);
    st.ItemSpacing = ImVec2(6, 4);
    st.IndentSpacing = 16;
    st.ScrollbarSize = 15;
    st.WindowMenuButtonPosition = ImGuiDir_None;
    ImVec4 *c = st.Colors;
    if (!gEd.dark)
    {
        ImGui::StyleColorsLight(&st);
        c[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        c[ImGuiCol_TextDisabled] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
        c[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
        c[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
        c[ImGuiCol_Border] = ImVec4(0.67f, 0.67f, 0.67f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.80f, 0.89f, 0.98f, 1.00f);
        c[ImGuiCol_TitleBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.80f, 0.86f, 0.95f, 1.00f);
        c[ImGuiCol_MenuBarBg] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
        c[ImGuiCol_Button] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.87f, 0.93f, 0.99f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.87f, 0.98f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.80f, 0.89f, 0.98f, 1.00f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.70f, 0.84f, 0.98f, 1.00f);
        c[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
        c[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
        c[ImGuiCol_Tab] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        c[ImGuiCol_TabHovered] = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
        c[ImGuiCol_TabSelected] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        c[ImGuiCol_TabDimmed] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
        c[ImGuiCol_TabDimmedSelected] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        c[ImGuiCol_TabSelectedOverline] = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
        c[ImGuiCol_DockingPreview] = ImVec4(0.00f, 0.47f, 0.84f, 0.35f);
        c[ImGuiCol_Separator] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        c[ImGuiCol_ScrollbarBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        c[ImGuiCol_TableHeaderBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        c[ImGuiCol_TableRowBgAlt] = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
    }
    else
    {
        ImGui::StyleColorsDark(&st);
        c[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        c[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.17f, 1.00f);
        c[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
        c[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.30f, 0.38f, 1.00f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.36f, 0.52f, 1.00f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.26f, 0.38f, 1.00f);
        c[ImGuiCol_MenuBarBg] = ImVec4(0.17f, 0.17f, 0.18f, 1.00f);
        c[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.38f, 0.52f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.44f, 0.68f, 1.00f);
        c[ImGuiCol_Header] = ImVec4(0.22f, 0.36f, 0.56f, 1.00f);
        c[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.32f, 0.42f, 1.00f);
        c[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.44f, 0.68f, 1.00f);
        c[ImGuiCol_TabSelectedOverline] = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    }
    st.ScaleAllSizes(gEd.uiScale);
}

// ---------------------------------------------------------------------------------------------------------------------
// The map list, grouped as the MAPS tree shows it: region, then the game's own name for the place

static std::string Pretty(const char *groupName)
{
    // "gMapGroup_IndoorLittleroot" -> "Indoor Littleroot"
    std::string s = groupName, out;
    if (s.rfind("gMapGroup_", 0) == 0)
        s = s.substr(10);
    for (size_t i = 0; i < s.size(); i++)
    {
        if (i && isupper((unsigned char)s[i]) && islower((unsigned char)s[i - 1]))
            out += ' ';
        out += s[i];
    }
    return out;
}

static void LoadMaps()
{
    int n = MakerApi_MapCount();
    gEd.maps.clear();
    gEd.maps.resize(n);
    for (int i = 0; i < n; i++)
        MakerApi_MapInfo(i, &gEd.maps[i]);
    for (int r = 0; r < MAKER_REGION_COUNT; r++)
        gEd.places[r].clear();
    for (int i = 0; i < n; i++)
    {
        const MakerMapInfo &m = gEd.maps[i];
        std::string place = m.place[0] ? m.place : Pretty(m.groupName);
        std::vector<Place> &ps = gEd.places[m.region];
        size_t p;
        for (p = 0; p < ps.size(); p++)
            if (ps[p].name == place)
                break;
        if (p == ps.size())
            ps.push_back(Place{place, {}});
        ps[p].maps.push_back(i);
    }
}

void SelectMap(int index)
{
    if (index < 0 || index >= (int)gEd.maps.size())
        return;
    gEd.map = index;
    gEd.scrollToMap = true;
    MapView_Opened();
}

// ---------------------------------------------------------------------------------------------------------------------
// Panels

static bool Matches(const char *text, const char *filter)
{
    // (case-insensitive; every word of the filter somewhere in the text)
    char f[64];
    snprintf(f, sizeof(f), "%s", filter);
    for (char *w = strtok(f, " "); w; w = strtok(NULL, " "))
    {
        bool found = false;
        size_t wl = strlen(w);
        for (const char *t = text; *t && !found; t++)
            found = strncasecmp(t, w, wl) == 0;
        if (!found)
            return false;
    }
    return true;
}

static void MapsPanel()
{
    if (!ImGui::Begin("MAPS"))
    {
        ImGui::End();
        return;
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##find", "Find a map...", gEd.mapFilter, sizeof(gEd.mapFilter));
    ImGui::BeginChild("tree", ImVec2(0, 0), ImGuiChildFlags_Borders);
    bool filtering = gEd.mapFilter[0] != 0;
    for (int r = 0; r < MAKER_REGION_COUNT; r++)
    {
        const MakerMapInfo *cur = gEd.map >= 0 ? &gEd.maps[gEd.map] : NULL;
        if (filtering || (gEd.scrollToMap && cur && cur->region == r))
            ImGui::SetNextItemOpen(true);
        if (!ImGui::TreeNodeEx(kRegionNames[r], ImGuiTreeNodeFlags_SpanAvailWidth))
            continue;
        for (const Place &p : gEd.places[r])
        {
            bool any = !filtering;
            if (filtering)
                for (int i : p.maps)
                {
                    std::string hay = p.name + " " + gEd.maps[i].title + " " + gEd.maps[i].name;
                    if (Matches(hay.c_str(), gEd.mapFilter))
                    {
                        any = true;
                        break;
                    }
                }
            if (!any)
                continue;
            bool hasCur = false;
            for (int i : p.maps)
                hasCur |= i == gEd.map;
            if (filtering || (gEd.scrollToMap && hasCur))
                ImGui::SetNextItemOpen(true);
            if (p.maps.size() == 1)
            {
                // (a place with one map: the map itself)
                int i = p.maps[0];
                ImGuiTreeNodeFlags fl = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (i == gEd.map)
                    fl |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void *)(intptr_t)i, fl, "%s", gEd.maps[i].title);
                if (ImGui::IsItemClicked())
                    SelectMap(i);
                if (i == gEd.map && gEd.scrollToMap)
                    ImGui::SetScrollHereY(0.4f);
                continue;
            }
            if (!ImGui::TreeNodeEx(p.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth))
                continue;
            for (int i : p.maps)
            {
                const MakerMapInfo &m = gEd.maps[i];
                std::string hay = p.name + " " + m.title + " " + m.name;
                if (filtering && !Matches(hay.c_str(), gEd.mapFilter))
                    continue;
                ImGuiTreeNodeFlags fl = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                if (i == gEd.map)
                    fl |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void *)(intptr_t)i, fl, "%s", m.title);
                if (ImGui::IsItemClicked())
                    SelectMap(i);
                if (i == gEd.map && gEd.scrollToMap)
                    ImGui::SetScrollHereY(0.4f);
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    gEd.scrollToMap = false;
    ImGui::EndChild();
    ImGui::End();
}

static void InspectorPanel()
{
    if (!ImGui::Begin("INSPECTOR"))
    {
        ImGui::End();
        return;
    }
    if (gEd.map < 0)
    {
        ImGui::TextDisabled("Pick a map in MAPS.");
        ImGui::End();
        return;
    }
    const MakerMapInfo &m = gEd.maps[gEd.map];
    if (ImGui::BeginTable("map", 2, ImGuiTableFlags_SizingFixedFit))
    {
        auto row = [](const char *k, const char *v) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%s", k);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(v);
        };
        char size[32];
        snprintf(size, sizeof(size), "%d x %d", m.width, m.height);
        row("Map", m.title);
        row("Name", m.name);
        row("Region", kRegionNames[m.region]);
        row("Place", m.place[0] ? m.place : "-");
        row("Size", size);
        ImGui::EndTable();
    }
    MapView_Inspector();
    ImGui::End();
}

static void ProblemsPanel()
{
    if (!ImGui::Begin("PROBLEMS"))
    {
        ImGui::End();
        return;
    }
    if (gEd.recovered)
    {
        ImGui::TextColored(ImVec4(0.1f, 0.45f, 0.85f, 1), "Unsaved edits of %d maps were kept from last time. Ctrl+S saves them.",
                           gEd.recovered);
        ImGui::SameLine();
        if (ImGui::SmallButton("OK"))
            gEd.recovered = 0;
    }
    if (gEd.map >= 0)
        MapView_Problems();
    else
        ImGui::TextDisabled("No problems.");
    ImGui::End();
}

void Message(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    gEd.message = buf;
    gEd.messageUntil = SDL_GetTicks() + 4000;
    printf("[maker] %s\n", buf);
}

static void Save()
{
    int n = SaveAll();
    Message(n ? "Saved %d %s." : "Nothing to save.", n, n == 1 ? "map" : "maps");
}

// ---------------------------------------------------------------------------------------------------------------------
// Menu bar, toolbar, status bar

static void MenuBar()
{
    if (!ImGui::BeginMainMenuBar())
        return;
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save", "Ctrl+S", false, AnyUnsaved()))
            Save();
        ImGui::MenuItem("Publish...", NULL, false, false);
        ImGui::Separator();
        if (ImGui::MenuItem("Close Moemon Maker"))
            gEd.wantClose = true;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, CanUndo()))
            Undo();
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, CanRedo()))
            Redo();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Mode"))
    {
        for (int i = 0; i < MODE_COUNT; i++)
            if (ImGui::MenuItem(kModeNames[i], kModeKeys[i], gEd.mode == i))
                gEd.mode = i;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Draw"))
    {
        for (int i = 0; i < TOOL_COUNT; i++)
            if (ImGui::MenuItem(kToolNames[i], kToolKeys[i], gEd.tool == i))
                gEd.tool = i;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Scale"))
    {
        static const int kZooms[] = {1, 2, 3, 4, 6};
        for (int z : kZooms)
        {
            char label[16];
            snprintf(label, sizeof(label), "%d%%", z * 100);
            if (ImGui::MenuItem(label, NULL, gEd.zoom == z))
                gEd.zoom = z;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("Animated tiles", NULL, &gEd.viewAnim);
        ImGui::MenuItem("People", NULL, &gEd.viewNpcs);
        ImGui::MenuItem("Screen frame", NULL, &gEd.viewFrame);
        ImGui::MenuItem("Grid", NULL, &gEd.viewGrid);
        ImGui::MenuItem("Behavior icons", NULL, &gEd.viewIcons);
        ImGui::Separator();
        if (ImGui::MenuItem("Dark theme", NULL, gEd.dark))
        {
            gEd.dark = !gEd.dark;
            ApplyTheme();
        }
        if (ImGui::MenuItem("Reset panels"))
            gEd.layoutBuilt = false;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help"))
    {
        ImGui::TextDisabled("Moemon Maker for Moemon Emerald Kai Co-op %s", MakerApi_GameVersion());
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

static float ToolbarHeight()
{
    return 30.0f * gEd.uiScale + 2 * ImGui::GetStyle().WindowPadding.y;
}

static void Toolbar()
{
    ImGuiViewport *vp = ImGui::GetMainViewport();
    float h = ToolbarHeight();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, h));
    ImGuiWindowFlags fl = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking
                        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("##toolbar", NULL, fl);
    ImGui::PopStyleVar();
    if (IconButton("save", ICON_SAVE, "Save (Ctrl+S)", false, AnyUnsaved()))
        Save();
    ImGui::SameLine();
    if (IconButton("undo", ICON_UNDO, "Undo (Ctrl+Z)", false, CanUndo()))
        Undo();
    ImGui::SameLine();
    if (IconButton("redo", ICON_REDO, "Redo (Ctrl+Y)", false, CanRedo()))
        Redo();
    ToolbarSeparator();
    for (int i = 0; i < MODE_COUNT; i++)
    {
        char tip[64];
        snprintf(tip, sizeof(tip), "%s mode (%s)", kModeNames[i], kModeKeys[i]);
        if (IconButton(kModeNames[i], ICON_MODE_MAP + i, tip, gEd.mode == i, true))
            gEd.mode = i;
        ImGui::SameLine();
    }
    ToolbarSeparator();
    for (int i = 0; i < TOOL_COUNT; i++)
    {
        char tip[64];
        snprintf(tip, sizeof(tip), "%s (%s)", kToolNames[i], kToolKeys[i]);
        if (IconButton(kToolNames[i], ICON_PENCIL + i, tip, gEd.tool == i, true))
            gEd.tool = i;
        ImGui::SameLine();
    }
    ToolbarSeparator();
    if (IconButton("zoomout", ICON_ZOOM_OUT, "Zoom out", false, gEd.zoom > 1) && gEd.zoom > 1)
        gEd.zoom--;
    ImGui::SameLine();
    if (IconButton("zoomin", ICON_ZOOM_IN, "Zoom in", false, gEd.zoom < 8) && gEd.zoom < 8)
        gEd.zoom++;
    ToolbarSeparator();
    if (IconButton("play", ICON_PLAY, "Play from here (F5)", false, gEd.map >= 0)) {}
    ImGui::End();
}

static float StatusHeight()
{
    return ImGui::GetFrameHeight() + 4;
}

static void StatusBar()
{
    ImGuiViewport *vp = ImGui::GetMainViewport();
    float h = StatusHeight();
    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - h));
    ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, h));
    ImGuiWindowFlags fl = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking
                        | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
    ImGui::Begin("##status", NULL, fl);
    ImGui::PopStyleVar();
    if (gEd.map >= 0)
    {
        const MakerMapInfo &m = gEd.maps[gEd.map];
        ImGui::Text("%s  (%s)   %d x %d", m.title, m.name, m.width, m.height);
        if (gEd.hoverX >= 0 && gEd.hoverBlock >= 0)
        {
            int b = gEd.hoverBlock;
            ImGui::SameLine();
            ImGui::TextDisabled("   tile %d, %d   metatile 0x%03X   %s   elevation %d   behavior 0x%02X", gEd.hoverX, gEd.hoverY,
                                b & 0x3FF, (b >> 10) & 3 ? "blocked" : "walkable", (b >> 12) & 15,
                                MakerApi_MetatileBehavior(b & 0x3FF));
        }
    }
    else
        ImGui::TextDisabled("No map open");
    char right[160];
    if (!gEd.message.empty() && !SDL_TICKS_PASSED(SDL_GetTicks(), gEd.messageUntil))
        snprintf(right, sizeof(right), "%s      Zoom %d%%", gEd.message.c_str(), gEd.zoom * 100);
    else
        snprintf(right, sizeof(right), "%s%d edited maps   Zoom %d%%   Moemon Maker %s", AnyUnsaved() ? "Unsaved changes   " : "",
                 EditedMapCount(), gEd.zoom * 100, MakerApi_GameVersion());
    ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(right).x - 12);
    ImGui::TextDisabled("%s", right);
    ImGui::End();
}

// The docked panels' first layout (and View > Reset panels)
static void BuildLayout(ImGuiID dock)
{
    ImGui::DockBuilderRemoveNode(dock);
    ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dock, ImGui::GetMainViewport()->WorkSize);
    ImGuiID left, right, center, bottom, leftTop, leftBottom;
    ImGui::DockBuilderSplitNode(dock, ImGuiDir_Left, 0.24f, &left, &center);
    ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.26f, &right, &center);
    ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.18f, &bottom, &center);
    ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.55f, &leftTop, &leftBottom);
    ImGui::DockBuilderDockWindow("TILES", leftTop);
    ImGui::DockBuilderDockWindow("MAPS", leftBottom);
    ImGui::DockBuilderDockWindow("MAP", center);
    ImGui::DockBuilderDockWindow("INSPECTOR", right);
    ImGui::DockBuilderDockWindow("PROBLEMS", bottom);
    ImGui::DockBuilderFinish(dock);
}

static void DockSpace()
{
    ImGuiViewport *vp = ImGui::GetMainViewport();
    float top = ToolbarHeight(), bottom = StatusHeight();
    ImVec2 pos(vp->WorkPos.x, vp->WorkPos.y + top), size(vp->WorkSize.x, vp->WorkSize.y - top - bottom);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags fl = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking
                        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoSavedSettings;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("##dockhost", NULL, fl);
    ImGui::PopStyleVar(2);
    ImGuiID dock = ImGui::GetID("MakerDock");
    if (!gEd.layoutBuilt)
    {
        if (ImGui::DockBuilderGetNode(dock) == NULL || gEd.layoutReset)
            BuildLayout(dock);
        gEd.layoutBuilt = true;
        gEd.layoutReset = true;   // (View > Reset panels from now on)
    }
    ImGui::DockSpace(dock, ImVec2(0, 0), ImGuiDockNodeFlags_None);
    ImGui::End();
}

static void Shortcuts()
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
        Save();
    if (io.WantTextInput)
        return;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
        io.KeyShift ? Redo() : Undo();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false))
        Redo();
    for (int i = 0; i < MODE_COUNT; i++)
        if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_F1 + i), false))
            gEd.mode = i;
    static const ImGuiKey kToolKeyCodes[TOOL_COUNT] = {ImGuiKey_P, ImGuiKey_R, ImGuiKey_F, ImGuiKey_I};
    if (!io.KeyCtrl)
        for (int i = 0; i < TOOL_COUNT; i++)
            if (ImGui::IsKeyPressed(kToolKeyCodes[i], false))
                gEd.tool = i;
}

// ---------------------------------------------------------------------------------------------------------------------
// The window

static void LoadFonts()
{
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;   // (built into the game: never freed)
    cfg.OversampleH = 2;
    float px = 15.0f * gEd.uiScale;
    gEd.font = io.Fonts->AddFontFromMemoryTTF((void *)gMakerFontRegular, (int)(gMakerFontRegularEnd - gMakerFontRegular), px, &cfg,
                                              io.Fonts->GetGlyphRangesDefault());
    gEd.fontBold = io.Fonts->AddFontFromMemoryTTF((void *)gMakerFontBold, (int)(gMakerFontBoldEnd - gMakerFontBold), px, &cfg,
                                                  io.Fonts->GetGlyphRangesDefault());
}

static int Open()
{
    if (gEd.win)
        return 1;
    LoadSettings();
    const char *scale = getenv("MOEMON_MAKER_UI_SCALE");
    gEd.uiScale = scale ? (float)atof(scale) : 1.0f;
    if (gEd.uiScale < 0.75f || gEd.uiScale > 3.0f)
        gEd.uiScale = 1.0f;
    Uint32 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (gEd.winMax)
        flags |= SDL_WINDOW_MAXIMIZED;
    gEd.win = SDL_CreateWindow("Moemon Maker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               gEd.winW > 320 ? gEd.winW : 1280, gEd.winH > 240 ? gEd.winH : 800, flags);
    if (!gEd.win)
    {
        printf("[maker] no window: %s\n", SDL_GetError());
        return 0;
    }
    SDL_SetWindowMinimumSize(gEd.win, 800, 500);
    gEd.ren = SDL_CreateRenderer(gEd.win, -1, SDL_RENDERER_ACCELERATED);
    if (!gEd.ren)
        gEd.ren = SDL_CreateRenderer(gEd.win, -1, SDL_RENDERER_SOFTWARE);
    if (!gEd.ren)
    {
        printf("[maker] no renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(gEd.win);
        gEd.win = NULL;
        return 0;
    }
    IMGUI_CHECKVERSION();
    gEd.ctx = ImGui::CreateContext();
    ImGui::SetCurrentContext(gEd.ctx);
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = kLayoutFile;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    LoadFonts();
    ApplyTheme();
    ImGui_ImplSDL2_InitForSDLRenderer(gEd.win, gEd.ren);
    ImGui_ImplSDLRenderer2_Init(gEd.ren);
    LoadMaps();
    LoadAllEdits();
    SendAllToGame();   // (recovered edits too: the game plays what the editor shows)
    gEd.map = -1;
    gEd.layoutBuilt = false;
    gEd.layoutReset = false;
    gEd.wantClose = false;
    if (!gEd.lastMap.empty())
        SelectMap(MakerApi_FindMap(gEd.lastMap.c_str()));
    SDL_StartTextInput();
    gEd.lastFrame = 0;
    printf("[maker] opened (%d maps)\n", (int)gEd.maps.size());
    return 1;
}

static void Close()
{
    if (!gEd.win)
        return;
    int w = 0, h = 0;
    SDL_GetWindowSize(gEd.win, &w, &h);
    gEd.winMax = (SDL_GetWindowFlags(gEd.win) & SDL_WINDOW_MAXIMIZED) != 0;
    if (!gEd.winMax)
        gEd.winW = w, gEd.winH = h;
    SaveSettings();
    MapView_Closed();
    ImGui::SetCurrentContext(gEd.ctx);
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext(gEd.ctx);
    gEd.ctx = NULL;
    SDL_DestroyRenderer(gEd.ren);
    SDL_DestroyWindow(gEd.win);
    gEd.ren = NULL;
    gEd.win = NULL;
    SDL_StopTextInput();
    printf("[maker] closed\n");
    MakerApi_Close();
}

// Closing with unsaved edits: save them, or drop them (the game goes back to the saved ones)
static void ClosePrompt()
{
    if (gEd.wantClose && AnyUnsaved() && !gEd.testMode)
    {
        gEd.wantClose = false;
        ImGui::OpenPopup("Save your edits?");
    }
    if (ImGui::BeginPopupModal("Save your edits?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Some maps have edits that aren't saved yet.");
        ImGui::Spacing();
        if (ImGui::Button("Save", ImVec2(110, 0)))
        {
            Save();
            gEd.wantClose = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't save", ImVec2(110, 0)))
        {
            DiscardUnsaved();
            gEd.wantClose = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(110, 0)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

static void Frame()
{
    ImGui::SetCurrentContext(gEd.ctx);
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    gEd.hoverX = gEd.hoverY = gEd.hoverBlock = -1;
    Shortcuts();
    MenuBar();
    Toolbar();
    DockSpace();
    TilesPanel();
    MapsPanel();
    MapView_Panel();
    InspectorPanel();
    ProblemsPanel();
    StatusBar();
    ClosePrompt();
    // (a slider let go or a stroke ended: that's one undo step, sent to the game)
    if (InChange() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
        EndChange();
    RecoveryTick();
    {
        // the window's title: the map, and * for unsaved edits
        char title[160];
        snprintf(title, sizeof(title), "Moemon Maker%s%s%s", gEd.map >= 0 ? " - " : "", gEd.map >= 0 ? gEd.maps[gEd.map].title : "",
                 AnyUnsaved() ? " *" : "");
        if (gEd.title != title)
        {
            gEd.title = title;
            SDL_SetWindowTitle(gEd.win, title);
        }
    }
    ImGui::Render();
    ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    SDL_SetRenderDrawColor(gEd.ren, (Uint8)(bg.x * 255), (Uint8)(bg.y * 255), (Uint8)(bg.z * 255), 255);
    SDL_RenderClear(gEd.ren);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), gEd.ren);
    if (!gEd.shotPath.empty())
    {
        // (tests: the picture just drawn, before it's shown)
        int w, h;
        SDL_GetRendererOutputSize(gEd.ren, &w, &h);
        SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
        if (s && SDL_RenderReadPixels(gEd.ren, NULL, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch) == 0)
            printf("[maker] shot %s %s\n", gEd.shotPath.c_str(), SDL_SaveBMP(s, gEd.shotPath.c_str()) == 0 ? "ok" : "FAILED");
        else
            printf("[maker] shot %s FAILED: %s\n", gEd.shotPath.c_str(), SDL_GetError());
        if (s)
            SDL_FreeSurface(s);
        gEd.shotPath.clear();
    }
    SDL_RenderPresent(gEd.ren);
    if (gEd.wantClose)
        Close();
}

} // namespace maker

using namespace maker;

// ---------------------------------------------------------------------------------------------------------------------
// include/maker_api.h

extern "C" int MakerUi_Open(void)
{
    return Open();
}

extern "C" void MakerUi_Close(void)
{
    Close();
}

extern "C" int MakerUi_IsOpen(void)
{
    return gEd.win != NULL;
}

extern "C" int MakerUi_HandleEvent(const void *ev)
{
    const SDL_Event *e = (const SDL_Event *)ev;
    Uint32 id;
    if (!gEd.win)
        return 0;
    switch (e->type)
    {
    case SDL_WINDOWEVENT: id = e->window.windowID; break;
    case SDL_KEYDOWN: case SDL_KEYUP: id = e->key.windowID; break;
    case SDL_TEXTINPUT: id = e->text.windowID; break;
    case SDL_TEXTEDITING: id = e->edit.windowID; break;
    case SDL_MOUSEMOTION: id = e->motion.windowID; break;
    case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: id = e->button.windowID; break;
    case SDL_MOUSEWHEEL: id = e->wheel.windowID; break;
    default: return 0;
    }
    if (id != SDL_GetWindowID(gEd.win))
        return 0;
    ImGui::SetCurrentContext(gEd.ctx);
    ImGui_ImplSDL2_ProcessEvent(e);
    if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_CLOSE)
        gEd.wantClose = true;
    return 1;
}

extern "C" void MakerUi_Frame(void)
{
    if (!gEd.win)
        return;
    Uint64 now = SDL_GetPerformanceCounter(), freq = SDL_GetPerformanceFrequency();
    if (!gEd.testMode && gEd.lastFrame && now - gEd.lastFrame < freq / 60)
        return;
    gEd.lastFrame = now;
    Frame();
}

extern "C" int MakerUi_SaveShot(const char *path)
{
    if (!gEd.win)
        return -1;
    gEd.shotPath = path;
    return 0;
}

// Tests (MOEMON_TEST: "maker WORDS"): open MAP_X / theme dark|classic / size W H / mode N / zoom N / find TEXT / close
extern "C" void MakerUi_TestCommand(const char *line)
{
    char cmd[32] = "", arg[200] = "";
    gEd.testMode = true;
    sscanf(line, "%31s %199[^\n]", cmd, arg);
    std::string c = cmd;
    if (!gEd.win)
        return;
    ImGui::SetCurrentContext(gEd.ctx);
    if (c == "open")
        SelectMap(MakerApi_FindMap(arg));
    else if (c == "theme")
    {
        gEd.dark = strcmp(arg, "dark") == 0;
        ApplyTheme();
    }
    else if (c == "size")
    {
        int w = 1280, h = 800;
        sscanf(arg, "%d %d", &w, &h);
        SDL_SetWindowSize(gEd.win, w, h);
    }
    else if (c == "mode")
        gEd.mode = atoi(arg);
    else if (c == "tool")
        gEd.tool = atoi(arg);
    else if (c == "zoom")
        gEd.zoom = atoi(arg);
    else if (c == "find")
        snprintf(gEd.mapFilter, sizeof(gEd.mapFilter), "%s", arg);
    else if (c == "close")
        gEd.wantClose = true;
    else if (c == "save")
        Save();
    else if (c == "undo")
        Undo();
    else if (c == "redo")
        Redo();
    else if (c == "edits")
    {
        int m = arg[0] ? MakerApi_FindMap(arg) : gEd.map;
        if (m >= 0)
            printf("[maker] edits of %s:\n%s[maker] end\n", gEd.maps[m].name, EditsText(m).c_str());
    }
    else
        MapView_TestCommand(c.c_str(), arg);
}
