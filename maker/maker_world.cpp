// Moemon Maker, M2b: new maps and the World view (docs/MOEMON_MAKER.md "Build order" M2). File > New map makes a map
// of the user's own (a blank one or a copy of another's tiles, existing tilesets, its music, its region; templates for
// a route, a town and a gym), written to MoemonMaker/maps/MAP_X.txt and sent to the game at once
// (MakerApi_SetNewMap: src/maker/maker_newmaps.c), then edited like any map. Tools > World shows a region's maps laid
// out by their connections; double-click opens one, dragging a map against another's edge connects them both ways.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#define MKDIR(p) mkdir(p, 0755)
#endif
#include "maker_ui.h"

namespace maker {

void ReloadMapList();   // maker_ui.cpp

// ---- New map

struct Template { const char *label; int w, h; const char *primary, *secondary, *music; int fill; const char *hint; };
static const Template kTemplates[] = {
    {"Blank", 20, 20, "General", "Petalburg", "MUS_LITTLEROOT", 0x3001, "Grass all over."},
    {"New Route", 40, 20, "General", "Petalburg", "MUS_ROUTE101", 0x300D, "A long route of tall grass to fill in."},
    {"New Town", 30, 30, "General", "Petalburg", "MUS_LITTLEROOT", 0x3001, "Room for houses, a POKeMON CENTER and a MART."},
    {"New Gym", 15, 20, "Building", "PetalburgGym", "MUS_GYM", 0x3001, "An inside map for a gym's floor."},
};

static bool sNewOpen;
static char sNewName[48] = "MY_TOWN";
static int sNewW = 20, sNewH = 20, sNewRegion = 0, sNewTemplate = 0, sNewCopy = -1, sNewPrimary = -1, sNewSecondary = -1;
static int sNewMusic = -1;
static std::string sNewError;

static int TilesetIndex(const char *name)
{
    for (int i = 0; i < MakerApi_TilesetCount(); i++)
        if (!strcmp(MakerApi_TilesetName(i, NULL), name))
            return i;
    return -1;
}

static void ApplyTemplate(int t)
{
    const Template &tp = kTemplates[t];
    sNewW = tp.w, sNewH = tp.h;
    sNewPrimary = TilesetIndex(tp.primary);
    sNewSecondary = TilesetIndex(tp.secondary);
    sNewMusic = MakerApi_SongId(tp.music);
}

static bool TilesetCombo(const char *id, int *sel, bool secondary)
{
    bool changed = false;
    const char *cur = *sel >= 0 ? MakerApi_TilesetName(*sel, NULL) : "?";
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, cur, ImGuiComboFlags_HeightLarge))
    {
        for (int i = 0; i < MakerApi_TilesetCount(); i++)
        {
            int sec = 0;
            const char *n = MakerApi_TilesetName(i, &sec);
            if ((sec != 0) != secondary)
                continue;
            if (ImGui::Selectable(n, i == *sel))
                *sel = i, changed = true;
        }
        ImGui::EndCombo();
    }
    return changed;
}

// The new map's text (MoemonMaker/maps/MAP_X.txt: src/maker/maker_newmaps.c's format)
static std::string NewMapText(const std::string &name)
{
    static const char *const kRegions[] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    char line[160];
    std::string t = "# Moemon Maker: a map of the user's own (written by the editor)\n";
    snprintf(line, sizeof(line), "newmap %s %d %d\n", name.c_str(), sNewW, sNewH);
    t += line;
    t += std::string("tilesets ") + MakerApi_TilesetName(sNewPrimary, NULL) + " " + MakerApi_TilesetName(sNewSecondary, NULL) + "\n";
    snprintf(line, sizeof(line), "header music=%s type=%d weather=2 region=%s cycling=1 running=1 escape=0 name=1\n",
             sNewMusic >= 0 ? MakerApi_SongName(sNewMusic) : "MUS_LITTLEROOT", sNewTemplate == 3 ? 8 : 2,
             kRegions[sNewRegion & 3]);
    t += line;
    int fill = kTemplates[sNewTemplate].fill;
    snprintf(line, sizeof(line), "border 0x%04X 0x%04X 0x%04X 0x%04X\n", fill, fill, fill, fill);
    t += line;
    for (int y = 0; y < sNewH; y++)
    {
        snprintf(line, sizeof(line), "row %d", y);
        t += line;
        for (int x = 0; x < sNewW; x++)
        {
            int b = fill;
            if (sNewCopy >= 0 && x < gEd.maps[sNewCopy].width && y < gEd.maps[sNewCopy].height)
            {
                int v = MakerApi_MapBlock(sNewCopy, x, y);
                if (v >= 0)
                    b = v;
            }
            snprintf(line, sizeof(line), " 0x%04X", b);
            t += line;
        }
        t += "\n";
    }
    return t;
}

static std::string CleanName(const char *s)
{
    std::string out = "MAP_";
    for (const char *p = s; *p; p++)
        out += isalnum((unsigned char)*p) ? (char)toupper((unsigned char)*p) : '_';
    if (out.compare(0, 8, "MAP_MAP_") == 0)
        out = out.substr(4);
    return out;
}

// Makes it: the file, the game told, the map list again, the map opened. Its index, or -1
static int CreateNewMap()
{
    std::string name = CleanName(sNewName);
    if (name.size() < 5)
        return sNewError = "Give it a name.", -1;
    if (MakerApi_FindMap(name.c_str()) >= 0)
        return sNewError = name + " is taken.", -1;
    if (sNewPrimary < 0 || sNewSecondary < 0)
        return sNewError = "Pick both tilesets.", -1;
    std::string text = NewMapText(name);
    std::string dir = EditsDir();
    MKDIR(dir.c_str());
    MKDIR((dir + "/maps").c_str());
    FILE *f = fopen((dir + "/maps/" + name + ".txt").c_str(), "wb");
    if (!f)
        return sNewError = "Couldn't write the map's file.", -1;
    fwrite(text.data(), 1, text.size(), f);
    fclose(f);
    int index = MakerApi_SetNewMap(text.c_str());
    if (index < 0)
        return sNewError = "The game couldn't read the new map.", -1;
    ReloadMapList();
    SelectMap(index);
    Message("Made %s", name.c_str());
    sNewError.clear();
    return index;
}

void NewMapWindow()
{
    if (!sNewOpen)
        return;
    if (sNewPrimary < 0)
        ApplyTemplate(sNewTemplate);
    ImGui::SetNextWindowSize(ImVec2(440 * gEd.uiScale, 0), ImGuiCond_Appearing);
    if (!ImGui::Begin("New map", &sNewOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }
    if (ImGui::BeginTable("nm", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("k", ImGuiTableColumnFlags_WidthFixed, 100 * gEd.uiScale);
        ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch);
        auto label = [](const char *k) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextDisabled("%s", k);
            ImGui::TableNextColumn();
        };
        label("Start from");
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##tpl", kTemplates[sNewTemplate].label))
        {
            for (int i = 0; i < (int)IM_ARRAYSIZE(kTemplates); i++)
                if (ImGui::Selectable(kTemplates[i].label, i == sNewTemplate))
                    sNewTemplate = i, ApplyTemplate(i);
            ImGui::EndCombo();
        }
        label("");
        ImGui::TextDisabled("%s", kTemplates[sNewTemplate].hint);
        label("Name");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##nm", sNewName, sizeof(sNewName));
        label("");
        ImGui::TextDisabled("%s", CleanName(sNewName).c_str());
        label("Size");
        ImGui::SetNextItemWidth(80 * gEd.uiScale);
        ImGui::InputInt("##w", &sNewW, 1, 5);
        ImGui::SameLine();
        ImGui::TextDisabled("x");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80 * gEd.uiScale);
        ImGui::InputInt("##h", &sNewH, 1, 5);
        sNewW = std::max(15, std::min(sNewW, 255));
        sNewH = std::max(10, std::min(sNewH, 255));
        label("Region");
        static const char *const kRegionNames[] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::Combo("##rg", &sNewRegion, kRegionNames, 4);
        label("Tileset");
        TilesetCombo("##pt", &sNewPrimary, false);
        label("Its second");
        TilesetCombo("##st", &sNewSecondary, true);
        label("Music");
        SongPicker("##mus", &sNewMusic);
        label("Copy tiles of");
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##cp", sNewCopy >= 0 ? gEd.maps[sNewCopy].title : "(none: a blank map)", ImGuiComboFlags_HeightLarge))
        {
            static char filter[64];
            if (ImGui::IsWindowAppearing())
                filter[0] = 0;
            ImGui::InputTextWithHint("##f", "Search...", filter, sizeof(filter));
            if (ImGui::Selectable("(none: a blank map)", sNewCopy < 0))
                sNewCopy = -1;
            for (int i = 0; i < (int)gEd.maps.size(); i++)
            {
                std::string t = gEd.maps[i].title, f = filter;
                std::transform(t.begin(), t.end(), t.begin(), ::tolower);
                std::transform(f.begin(), f.end(), f.begin(), ::tolower);
                if (filter[0] && t.find(f) == std::string::npos)
                    continue;
                ImGui::PushID(i);
                if (ImGui::Selectable(gEd.maps[i].title, i == sNewCopy))
                {
                    sNewCopy = i;
                    sNewW = gEd.maps[i].width, sNewH = gEd.maps[i].height;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::EndTable();
    }
    if (!sNewError.empty())
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "%s", sNewError.c_str());
    if (ImGui::Button("Make it") && CreateNewMap() >= 0)
        sNewOpen = false;
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
        sNewOpen = false;
    ImGui::TextDisabled("Doors and warps to it: put a warp on another map and pick this one.");
    ImGui::End();
}

void OpenNewMap()
{
    sNewOpen = true;
    sNewError.clear();
}

// ---- The World view

struct WorldMap { int map, x, y, w, h; };
static int sWorldRegion = 0, sWorldDrag = -1;
static ImVec2 sWorldDragFrom, sWorldPan(40, 40);
static float sWorldZoom = 2.0f;
static std::map<int, ImVec2> sWorldDragDelta;

// A map's connections: the game's and the user's (edits)
struct Conn { int dir, offset, dest; };
static std::vector<Conn> Connections(int map)
{
    std::vector<Conn> out;
    int n = MakerApi_ConnectionCount(map);
    for (int i = 0; i < n; i++)
    {
        Conn c;
        if (MakerApi_ConnectionInfo(map, i, &c.dir, &c.offset, &c.dest) && c.dest >= 0)
            out.push_back(c);
    }
    if (const MapEdits *e = FindEdits(map))
        for (auto &a : e->adds)
            if (a.first.rfind("connection ", 0) == 0)
            {
                static const char *const kDirs[] = {"", "down", "up", "left", "right"};
                Conn c{0, atoi(a.second.count("offset") ? a.second.at("offset").c_str() : "0"), -1};
                for (int d = 1; d <= 4; d++)
                    if (a.second.count("direction") && a.second.at("direction") == kDirs[d])
                        c.dir = d;
                if (a.second.count("map"))
                    c.dest = MakerApi_FindMap(a.second.at("map").c_str());
                if (c.dir && c.dest >= 0)
                    out.push_back(c);
            }
    return out;
}

// Lays out the region's connected maps from each still-unplaced one (in metatiles)
static std::vector<WorldMap> Layout(int region)
{
    std::vector<WorldMap> out;
    std::map<int, int> placed;
    int nextX = 0;
    for (int start = 0; start < (int)gEd.maps.size(); start++)
    {
        const MakerMapInfo &s = gEd.maps[start];
        if (s.region != region || placed.count(start) || Connections(start).empty())
            continue;
        std::vector<WorldMap> group;
        std::vector<int> queue{start};
        std::map<int, std::pair<int, int>> pos{{start, {0, 0}}};
        while (!queue.empty())
        {
            int m = queue.back();
            queue.pop_back();
            if (placed.count(m))
                continue;
            placed[m] = 1;
            auto p = pos[m];
            group.push_back(WorldMap{m, p.first, p.second, gEd.maps[m].width, gEd.maps[m].height});
            for (const Conn &c : Connections(m))
            {
                if (placed.count(c.dest) || gEd.maps[c.dest].region != region)
                    continue;
                int x = p.first, y = p.second;
                const MakerMapInfo &d = gEd.maps[c.dest];
                switch (c.dir)
                {
                case 1: x += c.offset; y += gEd.maps[m].height; break;   // down
                case 2: x += c.offset; y -= d.height; break;             // up
                case 3: x -= d.width; y += c.offset; break;              // left
                case 4: x += gEd.maps[m].width; y += c.offset; break;    // right
                default: continue;
                }
                if (!pos.count(c.dest))
                    pos[c.dest] = {x, y};
                queue.push_back(c.dest);
            }
        }
        int minX = 1 << 30, maxX = -(1 << 30);
        for (auto &g : group)
            minX = std::min(minX, g.x), maxX = std::max(maxX, g.x + g.w);
        for (auto &g : group)
        {
            g.x += nextX - minX;
            out.push_back(g);
        }
        nextX += maxX - minX + 10;
    }
    return out;
}

// Dropped against another map's edge: connect them both ways (the user's edits on each). Directions are
// CONNECTION_*: 1 down, 2 up, 3 left, 4 right; the offset is along the shared edge, from the map it's written on.
static void ConnectByDrop(const WorldMap &a, int ax, int ay, const std::vector<WorldMap> &all)
{
    static const char *const kDirs[] = {"", "down", "up", "left", "right"};
    for (const WorldMap &b : all)
    {
        if (b.map == a.map)
            continue;
        bool overlapX = ax < b.x + b.w && ax + a.w > b.x, overlapY = ay < b.y + b.h && ay + a.h > b.y;
        int fromB = 0, offset = 0;
        if (overlapX && std::abs(ay - (b.y + b.h)) <= 2)
            fromB = 1, offset = ax - b.x;   // A just below B
        else if (overlapX && std::abs(ay + a.h - b.y) <= 2)
            fromB = 2, offset = ax - b.x;   // A just above B
        else if (overlapY && std::abs(ax + a.w - b.x) <= 2)
            fromB = 3, offset = ay - b.y;   // A just left of B
        else if (overlapY && std::abs(ax - (b.x + b.w)) <= 2)
            fromB = 4, offset = ay - b.y;   // A just right of B
        if (!fromB)
            continue;
        int fromA = fromB == 1 ? 2 : fromB == 2 ? 1 : fromB == 3 ? 4 : 3;
        auto add = [&](int map, int d, int dest, int off) {
            MapEdits &e = Edits(map);
            int n = 1;
            for (auto &x : e.adds)
                n = std::max(n, atoi(x.first.substr(x.first.find(' ') + 1).c_str()) + 1);
            BeginChange(map);
            Edits(map).adds["connection " + std::to_string(n)] = {{"direction", kDirs[d]}, {"map", gEd.maps[dest].name},
                                                                 {"offset", std::to_string(off)}};
            EndChange();
        };
        add(b.map, fromB, a.map, offset);
        add(a.map, fromA, b.map, -offset);
        Message("Connected %s and %s", gEd.maps[a.map].title, gEd.maps[b.map].title);
        return;
    }
}

static bool sWorldOpen;
void OpenWorld() { sWorldOpen = true; }

void WorldWindow()
{
    if (!sWorldOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(900 * gEd.uiScale, 600 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("WORLD", &sWorldOpen))
    {
        ImGui::End();
        return;
    }
    static const char *const kRegionNames[] = {"HOENN", "JOHTO", "KANTO", "SINNOH"};
    ImGui::SetNextItemWidth(140 * gEd.uiScale);
    ImGui::Combo("##wr", &sWorldRegion, kRegionNames, 4);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140 * gEd.uiScale);
    ImGui::SliderFloat("Zoom", &sWorldZoom, 0.5f, 6.0f, "%.1f");
    ImGui::SameLine();
    ImGui::TextDisabled("Double-click opens a map. Drag one against another's edge to connect them.");
    std::vector<WorldMap> maps = Layout(sWorldRegion);
    // the user's new maps of this region, not connected yet, beside the rest
    {
        int x = 0, y = -40;
        for (int i = 0; i < (int)gEd.maps.size(); i++)
            if (MakerApi_IsNewMap(i) && gEd.maps[i].region == sWorldRegion
             && std::none_of(maps.begin(), maps.end(), [&](const WorldMap &w) { return w.map == i; }))
            {
                maps.push_back(WorldMap{i, x, y - gEd.maps[i].height, gEd.maps[i].width, gEd.maps[i].height});
                x += gEd.maps[i].width + 6;
            }
    }
    ImGui::BeginChild("canvas", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton("world", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool hovered = ImGui::IsItemHovered();
    ImGuiIO &io = ImGui::GetIO();
    if (hovered && io.MouseWheel != 0)
        sWorldZoom = std::max(0.5f, std::min(6.0f, sWorldZoom * (io.MouseWheel > 0 ? 1.15f : 1 / 1.15f)));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        sWorldPan.x += io.MouseDelta.x, sWorldPan.y += io.MouseDelta.y;
    float z = sWorldZoom;
    auto screen = [&](float x, float y) { return ImVec2(origin.x + sWorldPan.x + x * z, origin.y + sWorldPan.y + y * z); };
    int hover = -1;
    for (int k = 0; k < (int)maps.size(); k++)
    {
        const WorldMap &w = maps[k];
        ImVec2 off = sWorldDrag == k ? ImVec2(io.MousePos.x - sWorldDragFrom.x, io.MousePos.y - sWorldDragFrom.y) : ImVec2(0, 0);
        ImVec2 a = screen(w.x, w.y), b = screen(w.x + w.w, w.y + w.h);
        a.x += off.x, a.y += off.y, b.x += off.x, b.y += off.y;
        bool isNew = MakerApi_IsNewMap(w.map), open = w.map == gEd.map;
        if (io.MousePos.x >= a.x && io.MousePos.x < b.x && io.MousePos.y >= a.y && io.MousePos.y < b.y && hovered)
            hover = k;
        ImU32 fill = isNew ? IM_COL32(120, 200, 120, 170) : IM_COL32(110, 150, 210, 150);
        if (open)
            fill = IM_COL32(240, 200, 90, 200);
        dl->AddRectFilled(a, b, fill);
        dl->AddRect(a, b, hover == k ? IM_COL32(255, 255, 255, 255) : IM_COL32(20, 30, 50, 220), 0, 0, hover == k ? 2.5f : 1.0f);
        if (b.x - a.x > 30)
        {
            dl->PushClipRect(a, b, true);
            dl->AddText(ImVec2(a.x + 3, a.y + 2), IM_COL32(10, 15, 25, 255), gEd.maps[w.map].title);
            dl->PopClipRect();
        }
    }
    if (hover >= 0)
        ImGui::SetTooltip("%s\n%d x %d%s", gEd.maps[maps[hover].map].title, maps[hover].w, maps[hover].h,
                          MakerApi_IsNewMap(maps[hover].map) ? "\n(a map of your own)" : "");
    if (hover >= 0 && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        SelectMap(maps[hover].map);
    else if (hover >= 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        sWorldDrag = hover, sWorldDragFrom = io.MousePos;
    if (sWorldDrag >= 0 && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        const WorldMap &w = maps[sWorldDrag < (int)maps.size() ? sWorldDrag : 0];
        int dx = (int)((io.MousePos.x - sWorldDragFrom.x) / z + (io.MousePos.x >= sWorldDragFrom.x ? 0.5f : -0.5f));
        int dy = (int)((io.MousePos.y - sWorldDragFrom.y) / z + (io.MousePos.y >= sWorldDragFrom.y ? 0.5f : -0.5f));
        if ((dx || dy) && sWorldDrag < (int)maps.size())
            ConnectByDrop(w, w.x + dx, w.y + dy, maps);
        sWorldDrag = -1;
    }
    ImGui::EndChild();
    ImGui::End();
}

// Tests: "maker newmap NAME W H [TEMPLATE]" makes one; "maker world REGION" opens the World view
void World_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (c == "newmap")
    {
        char name[48] = "";
        int w = 20, h = 20, t = 0;
        sscanf(arg, "%47s %d %d %d", name, &w, &h, &t);
        snprintf(sNewName, sizeof(sNewName), "%s", name);
        sNewTemplate = std::max(0, std::min(t, (int)IM_ARRAYSIZE(kTemplates) - 1));
        ApplyTemplate(sNewTemplate);
        sNewW = w, sNewH = h, sNewCopy = -1;
        int index = CreateNewMap();
        printf("[maker] newmap %s: %d%s%s\n", name, index, sNewError.empty() ? "" : " ", sNewError.c_str());
    }
    else if (c == "newmapwindow")
        sNewOpen = true;
    else if (c == "world")
    {
        sWorldOpen = true;
        sWorldRegion = atoi(arg);
    }
}

} // namespace maker
