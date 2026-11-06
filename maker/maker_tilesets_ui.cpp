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

void TilesetsWindow()
{
    if (!sOpen || gEd.map < 0)
        return;
    const MakerTilesInfo *ti = MapView_TilesInfo();
    ImGui::SetNextWindowSize(ImVec2(900 * gEd.uiScale, 620 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("TILESETS", &sOpen))
    {
        ImGui::End();
        return;
    }
    if (ti->sinnoh)
    {
        ImGui::TextWrapped("SINNOH's maps are drawn from Platinum's 3D models: their tiles aren't a tileset to edit here.");
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
