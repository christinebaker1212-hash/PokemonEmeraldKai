// Moemon Maker, M5: the TILESETS window (Tools > Tileset editor). The open map's two tilesets: every metatile as the
// game draws it; a picked one's eight tile entries (the bottom layer's four, the top layer's four: which tile of the
// tileset, its palette, flipped or not, picked from the tile sheet) and its behavior and layer type. Every change is a
// line with the value it replaced in MoemonMaker/tilesets/NAME.txt (src/maker/maker_tilesets.c reads it), sent to the
// game at once so the map, the play test and the TILES panel show it.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

static bool sOpen;
static int sWhich;          // 0 the primary, 1 the secondary
static int sSel = -1;       // the picked metatile (its map id)
static int sSlot = 0;       // the picked entry 0-7
static std::map<std::string, std::map<std::string, std::pair<int, int>>> sEdits;   // tileset -> "metatile ID SLOT" / "attr ID" -> was, now
static std::map<std::string, bool> sDirty, sLoaded;
static SDL_Texture *sSheet;                 // the tile sheet, 16 tiles a row
static int sSheetMap = -1, sSheetPal = -1;
static unsigned sStamp;

static std::string Dir() { return std::string(MakerApi_EditsDir()) + "/tilesets"; }

static void LoadFile(const std::string &ts)
{
    if (sLoaded[ts])
        return;
    sLoaded[ts] = true;
    FILE *f = fopen((Dir() + "/" + ts + ".txt").c_str(), "rb");
    if (!f)
        return;
    char line[200];
    while (fgets(line, sizeof(line), f))
    {
        unsigned id, slot, was, now;
        char key[64];
        if (sscanf(line, "metatile %x %u %x %x", &id, &slot, &was, &now) == 4)
            snprintf(key, sizeof(key), "metatile 0x%03X %u", id, slot), sEdits[ts][key] = {(int)was, (int)now};
        else if (sscanf(line, "attr %x %x %x", &id, &was, &now) == 3)
            snprintf(key, sizeof(key), "attr 0x%03X", id), sEdits[ts][key] = {(int)was, (int)now};
    }
    fclose(f);
}

static std::string Text(const std::string &ts)
{
    std::string out = "# Moemon Maker: edits to the tileset " + ts + " (each line: the value it replaced, then the new one)\n";
    char buf[96];
    for (auto &e : sEdits[ts])
    {
        snprintf(buf, sizeof(buf), "%s 0x%04X 0x%04X\n", e.first.c_str(), e.second.first & 0xFFFF, e.second.second & 0xFFFF);
        out += buf;
    }
    return out;
}

static void Send(const std::string &ts)
{
    MakerApi_SetTilesetEdits(ts.c_str(), Text(ts).c_str());
    sStamp++;
}

static void SaveTilesets()
{
#ifdef _WIN32
    _mkdir(MakerApi_EditsDir());
    _mkdir(Dir().c_str());
#else
    mkdir(MakerApi_EditsDir(), 0755);
    mkdir(Dir().c_str(), 0755);
#endif
    for (auto &d : sDirty)
    {
        if (!d.second)
            continue;
        std::string path = Dir() + "/" + d.first + ".txt";
        if (sEdits[d.first].empty())
            remove(path.c_str());
        else if (FILE *f = fopen(path.c_str(), "wb"))
        {
            std::string t = Text(d.first);
            fwrite(t.data(), 1, t.size(), f);
            fclose(f);
        }
        d.second = false;
        printf("[maker] tileset %s saved\n", d.first.c_str());
    }
}

// A metatile's value now (an edit's, else the game's): SLOT 0-7 an entry, -1 the attributes
static int Value(const std::string &ts, int localId, int slot)
{
    char key[64];
    if (slot < 0)
        snprintf(key, sizeof(key), "attr 0x%03X", localId);
    else
        snprintf(key, sizeof(key), "metatile 0x%03X %d", localId, slot);
    auto it = sEdits[ts].find(key);
    if (it != sEdits[ts].end())
        return it->second.second;
    return MakerApi_TilesetBase(ts.c_str(), localId, slot);
}

static void Set(const std::string &ts, int localId, int slot, int now)
{
    char key[64];
    if (slot < 0)
        snprintf(key, sizeof(key), "attr 0x%03X", localId);
    else
        snprintf(key, sizeof(key), "metatile 0x%03X %d", localId, slot);
    int was = MakerApi_TilesetBase(ts.c_str(), localId, slot);
    if (was < 0)
        return;
    if (now == was)
        sEdits[ts].erase(key);
    else
        sEdits[ts][key] = {was, now};
    sDirty[ts] = true;
    Send(ts);
}

static void BuildSheet(int pal)
{
    if (sSheet)
        SDL_DestroyTexture(sSheet);
    sSheet = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 16 * 8, 64 * 8);
    std::vector<unsigned int> px(128 * 512);
    unsigned int t[64];
    for (int i = 0; i < 1024; i++)
    {
        MakerApi_RenderTile(i | (pal << 12), t);
        int ox = (i % 16) * 8, oy = (i / 16) * 8;
        for (int y = 0; y < 8; y++)
            memcpy(&px[(oy + y) * 128 + ox], t + y * 8, 32);
    }
    SDL_UpdateTexture(sSheet, NULL, px.data(), 128 * 4);
    SDL_SetTextureScaleMode(sSheet, SDL_ScaleModeNearest);
    sSheetMap = gEd.map, sSheetPal = pal;
}

static ImTextureID Tex(SDL_Texture *t) { return (ImTextureID)(intptr_t)t; }

static void MetatileImage(int id, float size)
{
    static std::map<int, SDL_Texture *> cache;
    static unsigned stamp = ~0u;
    static int map = -1;
    if (stamp != sStamp || map != gEd.map)
    {
        for (auto &c : cache)
            SDL_DestroyTexture(c.second);
        cache.clear();
        stamp = sStamp, map = gEd.map;
    }
    SDL_Texture *&t = cache[id];
    if (!t)
    {
        unsigned int px[256];
        MakerApi_RenderMetatile(id, px, 3);
        t = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 16, 16);
        SDL_UpdateTexture(t, NULL, px, 16 * 4);
        SDL_SetTextureScaleMode(t, SDL_ScaleModeNearest);
    }
    ImGui::Image(Tex(t), ImVec2(size, size));
}

// ---------------------------------------------------------------------------------------------------------------------
// TERRAINS (AUTOTILE, src/maker/maker_autotile.c), as RPG Maker's autotiles: a terrain is a 3x3 block (corners, edges,
// centre) and four inner corners. NEW AUTOTILE: a click on the top-left tile of its 3x3 block on the tileset (as the
// game's own tilesets lay their paths and ponds out), and if it has them, INNER CORNERS: the top-left of their 2x2.
// A learned terrain shows the same: a click on a slot, then on a tile, changes it (outlined); a right-click gives it
// back to what was learned. Every shape a terrain can take is drawn from those slots. (MORE SHAPES, folded away: all 47
// shapes one by one.) Kept at once in MoemonMaker/autotile/.
static bool sTerrainsTab;          // the window shows TERRAINS
static int sTerrain = -1;          // the picked terrain
static int sShape = -1;            // ... its picked shape
static int sPhase;                 // ... at this place of its pattern (a tree's left half...)
static char sRename[32];
static int sNewFrom = -1;          // the tile a new terrain is made from (picked in the tileset)
static int sTSlot = -1;             // the picked slot of the 3x3 + inner corners
static int sPickMode;              // 1: the next tile clicked is a new autotile's 3x3's top-left, 2: the inner corners' 2x2's

static void ShapeDiagram(ImDrawList *dl, int shape, ImVec2 p, float s)
{
    // (a 3x3 of the centre and its 8 neighbours: the same terrain filled)
    static const int sPos[8][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};
    float c = s / 3;
    dl->AddRectFilled(ImVec2(p.x + c, p.y + c), ImVec2(p.x + 2 * c, p.y + 2 * c), IM_COL32(60, 120, 200, 255));
    for (int i = 0; i < 8; i++)
    {
        ImVec2 a(p.x + sPos[i][0] * c, p.y + sPos[i][1] * c), b(a.x + c, a.y + c);
        dl->AddRectFilled(a, b, (shape >> i) & 1 ? IM_COL32(60, 120, 200, 255) : IM_COL32(220, 220, 220, 255));
    }
    dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(90, 90, 90, 255));
}

static void TerrainsTab()
{
    int n = MakerApi_AutotileLoad(gEd.map);
    if (n == 0 && gEd.map < 0)
    {
        ImGui::TextDisabled("Open a map in MAPS.");
        return;
    }
    float s = 32 * gEd.uiScale;
    // the terrains
    ImGui::BeginChild("terrainlist", ImVec2(220 * gEd.uiScale, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < n; i++)
    {
        MakerTerrain t;
        MakerApi_AutotileTerrain(i, &t);
        ImGui::PushID(i);
        char label[96];
        snprintf(label, sizeof(label), "%s%s%s", t.name, t.user ? " (yours)" : "", t.hidden ? " (hidden)" : "");
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (ImGui::Selectable("##t", i == sTerrain, 0, ImVec2(0, s)))
        {
            sTerrain = i, sShape = -1, sPhase = 0;
            snprintf(sRename, sizeof(sRename), "%s", t.name);
        }
        ImGui::SameLine(0, 0);
        if (t.px * t.py > 1)
            for (int j = 0; j < t.py; j++)
                for (int k = 0; k < t.px; k++)
                    MapView_DrawMetatile(ImGui::GetWindowDrawList(), t.q[j * t.px + k], ImVec2(p.x + k * s / 2, p.y + j * s / 2), s / 2);
        else
            MapView_DrawMetatile(ImGui::GetWindowDrawList(), t.centreTile, p, s);
        ImGui::SetCursorScreenPos(ImVec2(p.x + s + 6, p.y + s / 2 - ImGui::GetTextLineHeight() / 2));
        ImGui::TextUnformatted(label);
        ImGui::PopID();
    }
    ImGui::Separator();
    if (ImGui::Button(sPickMode == 1 ? "Click its 3x3's top-left tile..." : "NEW AUTOTILE", ImVec2(-FLT_MIN, 0)))
        sPickMode = sPickMode == 1 ? 0 : 1;
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("Your own autotile: click NEW AUTOTILE, then the top-left tile of its 3x3 block in the tileset\n"
                          "(top-left corner, top edge, top-right corner; left edge, centre, right edge; the bottom row).");
    ImGui::EndChild();
    ImGui::SameLine();
    // the picked one's shapes
    ImGui::BeginChild("shapes", ImVec2(ImGui::GetContentRegionAvail().x - 8 * 34 * gEd.uiScale - 24, 0), ImGuiChildFlags_Borders);
    MakerTerrain t;
    static int sRenameFor = -1;
    if (!MakerApi_AutotileTerrain(sTerrain, &t))
        ImGui::TextDisabled("Pick a terrain.");
    else
    {
        if (sRenameFor != sTerrain)
            sRenameFor = sTerrain, snprintf(sRename, sizeof(sRename), "%s", t.name);
        ImGui::SetNextItemWidth(200 * gEd.uiScale);
        if (ImGui::InputText("Name", sRename, sizeof(sRename), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            MakerApi_AutotileRename(sTerrain, sRename);
            MapView_AutotileChanged();
        }
        ImGui::SameLine();
        bool hidden = t.hidden;
        if (ImGui::Checkbox("Hidden", &hidden))
        {
            MakerApi_AutotileSetHidden(sTerrain, hidden);
            MapView_AutotileChanged();
        }
        if (t.px * t.py > 1)
        {
            ImGui::TextUnformatted("Place in its pattern:");
            for (int ph = 0; ph < 4; ph++)
            {
                if ((ph & 1) >= t.px || (ph >> 1) >= t.py)
                    continue;
                ImGui::SameLine();
                char b[16];
                snprintf(b, sizeof(b), "%s%s", (ph >> 1) ? "BOTTOM" : "TOP", t.px > 1 ? ((ph & 1) ? " R" : " L") : "");
                if (ImGui::RadioButton(b, sPhase == ph))
                    sPhase = ph;
            }
        }
        // the 3x3 block and the inner corners, RPG Maker's way
        static const char *const kSlot[13] = {"top-left corner", "top edge", "top-right corner", "left edge", "centre",
                                              "right edge", "bottom-left corner", "bottom edge", "bottom-right corner",
                                              "inner corner (top-left)", "inner corner (top-right)",
                                              "inner corner (bottom-left)", "inner corner (bottom-right)"};
        ImDrawList *dl = ImGui::GetWindowDrawList();
        float big = s * 1.5f;
        auto slot = [&](int k, ImVec2 p) {
            ImGui::SetCursorScreenPos(p);
            ImGui::PushID(k);
            if (ImGui::InvisibleButton("sl", ImVec2(big, big)))
                sTSlot = k, sPickMode = 0;
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                MakerApi_AutotileSetSlot(sTerrain, k, sPhase, -1);
                MapView_AutotileChanged();
            }
            bool hov = ImGui::IsItemHovered();
            ImGui::PopID();
            int fixed = -1, tile = MakerApi_AutotileSlotPiece(sTerrain, k, sPhase, &fixed);
            if (tile >= 0)
                MapView_DrawMetatile(dl, tile, p, big);
            if (fixed >= 0)
                dl->AddRect(p, ImVec2(p.x + big, p.y + big), IM_COL32(240, 140, 20, 255), 0, 0, 2);
            if (k == sTSlot)
                dl->AddRect(ImVec2(p.x - 2, p.y - 2), ImVec2(p.x + big + 2, p.y + big + 2), IM_COL32(0, 90, 200, 255), 0, 0, 3);
            if (hov)
                ImGui::SetTooltip("%s: metatile 0x%03X%s\nClick, then a tile in the tileset. Right-click: as learned.", kSlot[k],
                                  tile, fixed >= 0 ? " (yours)" : "");
        };
        ImGui::Spacing();
        ImGui::TextUnformatted("Its 3x3 block");
        ImVec2 o = ImGui::GetCursorScreenPos();
        for (int k = 0; k < 9; k++)
            slot(k, ImVec2(o.x + (k % 3) * (big + 2), o.y + (k / 3) * (big + 2)));
        ImVec2 o2(o.x + 3 * (big + 2) + 30, o.y);
        dl->AddText(ImVec2(o2.x, o2.y - ImGui::GetTextLineHeight() - 4), ImGui::GetColorU32(ImGuiCol_Text), "Inner corners");
        for (int k = 0; k < 4; k++)
            slot(9 + k, ImVec2(o2.x + (k % 2) * (big + 2), o2.y + (k / 2) * (big + 2)));
        ImGui::SetCursorScreenPos(ImVec2(o2.x, o2.y + 2 * (big + 2) + 6));
        if (ImGui::SmallButton(sPickMode == 2 ? "Click their 2x2's top-left..." : "INNER CORNERS FROM A 2x2"))
            sPickMode = sPickMode == 2 ? 0 : 2;
        ImGui::SetCursorScreenPos(ImVec2(o.x, o.y + 3 * (big + 2) + 10));
        ImGui::TextDisabled("Click a slot, then a tile in the tileset. Right-click a slot: as learned.");
        // every shape one by one (folded away: the slots draw them all)
        if (ImGui::CollapsingHeader("MORE SHAPES (advanced)"))
        {
            int shapes[64], ns = MakerApi_AutotileShapes(shapes);
            float cell = s * 2 + 12;
            int cols = std::max(1, (int)(ImGui::GetContentRegionAvail().x / (cell + 4)));
            for (int k = 0; k < ns; k++)
            {
                if (k % cols)
                    ImGui::SameLine(0, 4);
                ImGui::PushID(100 + k);
                ImVec2 p = ImGui::GetCursorScreenPos();
                if (ImGui::InvisibleButton("sh", ImVec2(cell, s + 4)))
                    sShape = shapes[k], sTSlot = -1;
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    MakerApi_AutotileSetFix(sTerrain, shapes[k], sPhase, -1);
                    MapView_AutotileChanged();
                }
                ImGui::PopID();
                int fixed = -1, tile = MakerApi_AutotilePiece(sTerrain, shapes[k], sPhase, &fixed);
                ShapeDiagram(dl, shapes[k], ImVec2(p.x + 2, p.y + 2), s);
                if (tile >= 0)
                    MapView_DrawMetatile(dl, tile, ImVec2(p.x + s + 8, p.y + 2), s);
                if (fixed >= 0)
                    dl->AddRect(ImVec2(p.x + s + 7, p.y + 1), ImVec2(p.x + 2 * s + 9, p.y + s + 3), IM_COL32(240, 140, 20, 255), 0, 0, 2);
                if (shapes[k] == sShape && sTSlot < 0)
                    dl->AddRect(p, ImVec2(p.x + cell, p.y + s + 4), IM_COL32(0, 90, 200, 255), 0, 0, 2);
            }
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    // the tileset: a click puts the tile in the picked shape (or picks a new terrain's centre)
    const MakerTilesInfo *ti = MapView_TilesInfo();
    ImGui::BeginChild("tsgrid", ImVec2(0, 0), ImGuiChildFlags_Borders);
    std::vector<int> area(1024);
    int na = sTerrain >= 0 ? MakerApi_AutotileArea(sTerrain, area.data(), 1024) : 0;
    area.resize(std::min(na, 1024));
    for (int part = 0; part < 2; part++)
    {
        int first = part ? ti->secondaryStart : 0, count = part ? ti->secondaryMetatiles : ti->primaryMetatiles;
        ImGui::SeparatorText(part ? ti->secondaryName : ti->primaryName);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        for (int i = 0; i < count; i++)
        {
            int id = first + i;
            if (i % 8)
                ImGui::SameLine(0, 2);
            ImGui::PushID(id);
            ImVec2 p = ImGui::GetCursorScreenPos();
            if (ImGui::InvisibleButton("m", ImVec2(s, s)))
            {
                sNewFrom = id;
                if (sPickMode == 1)
                {
                    sTerrain = MakerApi_AutotileNewBlock(id, -1), sTSlot = -1, sShape = -1, sPhase = 0;
                    MapView_AutotileChanged();
                }
                else if (sPickMode == 2 && sTerrain >= 0)
                {
                    MakerApi_AutotileSetInner(sTerrain, id);
                    MapView_AutotileChanged();
                }
                else if (sTerrain >= 0 && sTSlot >= 0)
                {
                    MakerApi_AutotileSetSlot(sTerrain, sTSlot, sPhase, id);
                    MapView_AutotileChanged();
                }
                else if (sTerrain >= 0 && sShape >= 0)
                {
                    MakerApi_AutotileSetFix(sTerrain, sShape, sPhase, id);
                    MapView_AutotileChanged();
                }
                sPickMode = 0;
            }
            bool hov = ImGui::IsItemHovered();
            ImGui::PopID();
            MapView_DrawMetatile(dl, id, p, s);
            if (std::find(area.begin(), area.end(), id) != area.end())
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(60, 120, 200, 255), 0, 0, 2);
            if (id == sNewFrom)
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 255), 0, 0, 2);
            if (hov)
                ImGui::SetTooltip("Metatile 0x%03X%s", id, std::find(area.begin(), area.end(), id) != area.end() ? " (this terrain's)" : "");
        }
    }
    ImGui::EndChild();
}

void TilesetsWindow()
{
    if (!sOpen || gEd.map < 0)
        return;
    const MakerTilesInfo *ti = MapView_TilesInfo();
    ImGui::SetNextWindowSize(ImVec2(1120 * gEd.uiScale, 700 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("TILESETS", &sOpen))
    {
        ImGui::End();
        return;
    }
    if (ti->world3d)
    {
        ImGui::TextWrapped("%s's maps are drawn from its game's 3D models: their tiles aren't a tileset to edit here.",
                           MakerApi_RegionName(ti->world3d));
        ImGui::End();
        return;
    }
    if (ImGui::BeginTabBar("tstabs"))
    {
        if (ImGui::BeginTabItem("METATILES", NULL, sTerrainsTab ? 0 : 0))
        {
            if (ImGui::IsItemActivated())
                sTerrainsTab = false;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("TERRAINS (AUTOTILE)", NULL, sTerrainsTab ? ImGuiTabItemFlags_SetSelected : 0))
        {
            sTerrainsTab = true;
            ImGui::EndTabItem();
        }
        else
            sTerrainsTab = false;
        ImGui::EndTabBar();
    }
    if (sTerrainsTab)
    {
        TerrainsTab();
        ImGui::End();
        return;
    }
    std::string ts = sWhich ? ti->secondaryName : ti->primaryName;
    int first = sWhich ? ti->secondaryStart : 0, count = sWhich ? ti->secondaryMetatiles : ti->primaryMetatiles;
    LoadFile(ti->primaryName);
    LoadFile(ti->secondaryName);
    if (ImGui::RadioButton(("Primary: " + std::string(ti->primaryName)).c_str(), sWhich == 0))
        sWhich = 0, sSel = -1;
    ImGui::SameLine();
    if (ImGui::RadioButton(("Secondary: " + std::string(ti->secondaryName)).c_str(), sWhich == 1))
        sWhich = 1, sSel = -1;
    ImGui::SameLine();
    bool dirty = false;
    for (auto &d : sDirty)
        dirty |= d.second;
    if (ImGui::Button(dirty ? "Save tilesets*" : "Save tilesets"))
        SaveTilesets();
    int skipped = MakerApi_TilesetSkipped(ts.c_str());
    if (skipped)
        ImGui::TextColored(ImVec4(0.85f, 0.35f, 0.1f, 1), "%d of this tileset's edits are left out: the game changed there since.", skipped);
    // every metatile
    ImGui::BeginChild("grid", ImVec2(8 * 34 * gEd.uiScale + 20, 0), ImGuiChildFlags_Borders);
    float s = 32 * gEd.uiScale;
    for (int i = 0; i < count; i++)
    {
        int id = first + i;
        ImGui::PushID(id);
        ImVec2 p = ImGui::GetCursorScreenPos();
        MetatileImage(id, s);
        if (ImGui::IsItemClicked())
            sSel = id, sSlot = 0;
        char key[32];
        snprintf(key, sizeof(key), "attr 0x%03X", i);
        bool edited = sEdits[ts].count(key) > 0;
        for (int k = 0; k < 8 && !edited; k++)
            snprintf(key, sizeof(key), "metatile 0x%03X %d", i, k), edited = sEdits[ts].count(key) > 0;
        if (edited)
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 160, 0, 255), 0, 0, 2);
        if (sSel == id)
            ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(40, 120, 255, 255), 0, 0, 2);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Metatile 0x%03X", id);
        ImGui::PopID();
        if ((i + 1) % 8)
            ImGui::SameLine(0, 2);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("edit", ImVec2(0, 0));
    if (sSel >= first && sSel < first + count)
    {
        int local = sSel - first;
        ImGui::Text("Metatile 0x%03X of %s", sSel, ts.c_str());
        MetatileImage(sSel, 96 * gEd.uiScale);
        ImGui::SeparatorText("TILES");
        for (int k = 0; k < 8; k++)
        {
            int e = Value(ts, local, k);
            unsigned int px[64];
            MakerApi_RenderTile(e, px);
            static SDL_Texture *slotTex[8];
            if (!slotTex[k])
            {
                slotTex[k] = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 8, 8);
                SDL_SetTextureScaleMode(slotTex[k], SDL_ScaleModeNearest);
            }
            SDL_UpdateTexture(slotTex[k], NULL, px, 32);
            if (k == 4)
                ImGui::NewLine();
            ImGui::PushID(k);
            ImGui::BeginGroup();
            ImGui::TextDisabled(k < 4 ? "bottom %d" : "top %d", k & 3);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::Image(Tex(slotTex[k]), ImVec2(40 * gEd.uiScale, 40 * gEd.uiScale));
            if (ImGui::IsItemClicked())
                sSlot = k;
            if (sSlot == k)
                ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + 40 * gEd.uiScale, p.y + 40 * gEd.uiScale), IM_COL32(40, 120, 255, 255), 0, 0, 2);
            ImGui::EndGroup();
            ImGui::PopID();
            if (k != 3 && k != 7)
                ImGui::SameLine();
        }
        int e = Value(ts, local, sSlot);
        int tile = e & 0x3FF, pal = (e >> 12) & 15;
        bool hf = e & 0x400, vf = e & 0x800;
        ImGui::Text("%s %d:", sSlot < 4 ? "Bottom" : "Top", sSlot & 3);
        ImGui::SetNextItemWidth(120 * gEd.uiScale);
        bool ch = ImGui::InputInt("tile", &tile);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90 * gEd.uiScale);
        ch |= ImGui::InputInt("palette", &pal);
        ImGui::SameLine();
        ch |= ImGui::Checkbox("flip H", &hf);
        ImGui::SameLine();
        ch |= ImGui::Checkbox("flip V", &vf);
        if (ch)
        {
            tile = tile < 0 ? 0 : tile > 1023 ? 1023 : tile;
            pal = pal < 0 ? 0 : pal > 15 ? 15 : pal;
            Set(ts, local, sSlot, tile | (hf ? 0x400 : 0) | (vf ? 0x800 : 0) | (pal << 12));
        }
        ImGui::SeparatorText("WHAT IT IS");
        int attr = Value(ts, local, -1);
        int behavior = attr & 0xFF, layer = (attr >> 12) & 0xF;
        ImGui::SetNextItemWidth(120 * gEd.uiScale);
        bool ach = ImGui::InputInt("behavior", &behavior);
        ImGui::SameLine();
        const char *bn = MakerApi_BehaviorName(behavior & 0xFF);
        ImGui::TextDisabled("%s", bn[0] ? bn : "(no name)");
        static const char *const kLayers[] = {"Normal (over the player's feet)", "Covered (both layers under)", "Split (top over the player)"};
        ImGui::SetNextItemWidth(260 * gEd.uiScale);
        ach |= ImGui::Combo("layer", &layer, kLayers, 3);
        if (ach)
        {
            behavior = behavior < 0 ? 0 : behavior > 255 ? 255 : behavior;
            Set(ts, local, -1, (attr & ~0xF0FF) | behavior | (layer << 12));
        }
        if (ImGui::Button("Back to the game's"))
        {
            for (int k = -1; k < 8; k++)
                Set(ts, local, k, MakerApi_TilesetBase(ts.c_str(), local, k));
        }
        // the tile sheet in the picked palette: a click puts that tile in the picked entry
        ImGui::SeparatorText("TILE SHEET (click a tile)");
        if (!sSheet || sSheetMap != gEd.map || sSheetPal != pal)
            BuildSheet(pal);
        float z = 2 * gEd.uiScale;
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::BeginChild("sheet", ImVec2(128 * z + 16, 0), ImGuiChildFlags_Borders);
        p = ImGui::GetCursorScreenPos();
        ImGui::Image(Tex(sSheet), ImVec2(128 * z, 512 * z));
        if (ImGui::IsItemClicked())
        {
            ImVec2 m = ImGui::GetMousePos();
            int tx = (int)((m.x - p.x) / (8 * z)), ty = (int)((m.y - p.y) / (8 * z));
            if (tx >= 0 && tx < 16 && ty >= 0 && ty < 64)
                Set(ts, local, sSlot, (e & ~0x3FF) | (ty * 16 + tx));
        }
        ImGui::EndChild();
    }
    else
        ImGui::TextDisabled("Pick a metatile.");
    ImGui::EndChild();
    ImGui::End();
}

void OpenTilesets()
{
    sOpen = true;
}

// tests: "tileset open", "tileset primary|secondary", "tileset pick ID", "tileset entry SLOT VALUE" (hex),
// "tileset attr VALUE" (hex), "tileset save", "tileset value SLOT" (prints the picked metatile's value)
void Tilesets_TestCommand(const char *cmd, const char *arg)
{
    {
        // (AUTOTILE's TERRAINS: "tileset terrains", "tileset terrain CENTRE", "tileset fix CENTRE SHAPE PHASE TILE",
        // "tileset newterrain TILE" (hex), "tileset shape SHAPE")
        char w[16] = "", centre[48] = "";
        int a = 0, b = 0, c = 0;
        sscanf(arg, "%15s", w);
        auto find = [&](const char *key) {
            int nn = MakerApi_AutotileLoad(gEd.map);
            for (int i = 0; i < nn; i++)
            {
                MakerTerrain t;
                MakerApi_AutotileTerrain(i, &t);
                if (!strcmp(t.centre, key) || !strcmp(t.name, key))
                    return i;
            }
            return -1;
        };
        if (!strcmp(w, "terrains"))
        {
            sOpen = true, sTerrainsTab = true;
            return;
        }
        if (!strcmp(w, "terrain") && sscanf(arg, "%*s %47s", centre) == 1)
        {
            sOpen = true, sTerrainsTab = true;
            sTerrain = find(centre), sShape = -1, sPhase = 0;
            return;
        }
        if (!strcmp(w, "fix") && sscanf(arg, "%*s %47s %i %i %i", centre, &a, &b, &c) == 4)
        {
            MakerApi_AutotileSetFix(find(centre), a, b, c);
            MapView_AutotileChanged();
            sShape = a;
            return;
        }
        if (!strcmp(w, "newterrain") && sscanf(arg, "%*s %i", &a) == 1)
        {
            int cc[1] = {a};
            sOpen = true, sTerrainsTab = true;
            sTerrain = MakerApi_AutotileNew(cc, 1), sShape = -1;
            MapView_AutotileChanged();
            return;
        }
        if (!strcmp(w, "shape") && sscanf(arg, "%*s %i", &a) == 1)
        {
            sShape = a, sTSlot = -1;
            return;
        }
        if (!strcmp(w, "slot") && sscanf(arg, "%*s %47s %i %i", centre, &a, &b) == 3)
        {
            MakerApi_AutotileSetSlot(find(centre), a, 0, b);
            MapView_AutotileChanged();
            sTSlot = a;
            return;
        }
        if (!strcmp(w, "newblock") && sscanf(arg, "%*s %i %i", &a, &b) >= 1)
        {
            int k = sscanf(arg, "%*s %i %i", &a, &b);
            sOpen = true, sTerrainsTab = true;
            sTerrain = MakerApi_AutotileNewBlock(a, k == 2 ? b : -1), sTSlot = -1;
            MapView_AutotileChanged();
            MakerTerrain t;
            if (MakerApi_AutotileTerrain(sTerrain, &t))
                printf("[maker] autotile new %s %s\n", t.name, t.centre);
            return;
        }
    }
    char what[16] = "";
    unsigned a = 0, b = 0;
    int n = sscanf(arg, "%15s %x %x", what, &a, &b);
    const MakerTilesInfo *ti = MapView_TilesInfo();
    std::string ts = sWhich ? ti->secondaryName : ti->primaryName;
    int first = sWhich ? ti->secondaryStart : 0;
    LoadFile(ti->primaryName);
    LoadFile(ti->secondaryName);
    if (!strcmp(what, "open"))
        sOpen = true;
    else if (!strcmp(what, "primary") || !strcmp(what, "secondary"))
        sWhich = !strcmp(what, "secondary");
    else if (!strcmp(what, "pick") && n >= 2)
        sSel = (int)a;
    else if (!strcmp(what, "entry") && n >= 3 && sSel >= first)
        Set(ts, sSel - first, (int)a, (int)b);
    else if (!strcmp(what, "attr") && n >= 2 && sSel >= first)
        Set(ts, sSel - first, -1, (int)a);
    else if (!strcmp(what, "save"))
        SaveTilesets();
    else if (!strcmp(what, "value") && n >= 2 && sSel >= first)
        printf("[maker] tileset value %s 0x%03X %d: 0x%04X\n", ts.c_str(), sSel, (int)a, Value(ts, sSel - first, (int)a) & 0xFFFF);
    (void)cmd;
}

} // namespace maker
