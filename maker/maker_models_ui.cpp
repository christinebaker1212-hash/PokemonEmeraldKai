// Moemon Maker, M5: a SINNOH map's 3D models (the inspector's MODELS section). The .glb files in MoemonMaker/models/
// (exported from Blender: File > Export > glTF 2.0, glTF Binary) placed on the map's tiles with a height, a turn and a
// size; kept in MoemonMaker/models/MAP_X.txt (src/maker/maker_models.c reads it and places them in the 3D picture).
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <algorithm>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

struct Placement { std::string file; int x = 0, y = 0; float height = 0, turn = 0, scale = 100; };
static int sMap = -1;
static std::vector<Placement> sPlaced;
static std::vector<std::string> sFiles;
static int sPick;

static std::string Dir() { return std::string(MakerApi_EditsDir()) + "/models"; }

static void Load(int map)
{
    sMap = map;
    sPlaced.clear();
    sFiles.clear();
    if (DIR *d = opendir(Dir().c_str()))
    {
        while (struct dirent *e = readdir(d))
        {
            size_t n = strlen(e->d_name);
            if (n > 4 && !strcmp(e->d_name + n - 4, ".glb"))
                sFiles.push_back(e->d_name);
        }
        closedir(d);
    }
    std::sort(sFiles.begin(), sFiles.end());
    if (FILE *f = fopen((Dir() + "/" + gEd.maps[map].name + ".txt").c_str(), "rb"))
    {
        char line[200], file[64];
        Placement p;
        while (fgets(line, sizeof(line), f))
        {
            p = Placement();
            if (sscanf(line, "place %63s %d %d %f %f %f", file, &p.x, &p.y, &p.height, &p.turn, &p.scale) >= 3)
                p.file = file, sPlaced.push_back(p);
        }
        fclose(f);
    }
}

static void Save()
{
#ifdef _WIN32
    _mkdir(MakerApi_EditsDir());
    _mkdir(Dir().c_str());
#else
    mkdir(MakerApi_EditsDir(), 0755);
    mkdir(Dir().c_str(), 0755);
#endif
    std::string path = Dir() + "/" + gEd.maps[sMap].name + ".txt";
    if (sPlaced.empty())
        remove(path.c_str());
    else if (FILE *f = fopen(path.c_str(), "wb"))
    {
        for (auto &p : sPlaced)
            fprintf(f, "place %s %d %d %g %g %g\n", p.file.c_str(), p.x, p.y, p.height, p.turn, p.scale);
        fclose(f);
    }
    MakerApi_ModelsChanged(gEd.maps[sMap].name);
}

void ModelsSection(int map)
{
    if (map != sMap)
        Load(map);
    ImGui::Spacing();
    ImGui::SeparatorText("3D MODELS");
    if (sFiles.empty())
    {
        ImGui::TextWrapped("Put .glb models (Blender: File > Export > glTF 2.0, glTF Binary) in MoemonMaker/models to place "
                           "them here. One Blender unit is one tile.");
        if (ImGui::SmallButton("Look again"))
            Load(map);
        return;
    }
    int remove = -1;
    static bool dirty;
    float w = ImGui::GetContentRegionAvail().x - 70 * gEd.uiScale;   // (one value a line, labels on the right)
    for (int i = 0; i < (int)sPlaced.size(); i++)
    {
        Placement &p = sPlaced[i];
        ImGui::PushID(i);
        ImGui::Text("%s", p.file.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove"))
            remove = i;
        ImGui::SetNextItemWidth(w);
        dirty |= ImGui::InputInt("x", &p.x);
        ImGui::SetNextItemWidth(w);
        dirty |= ImGui::InputInt("y", &p.y);
        ImGui::SetNextItemWidth(w);
        dirty |= ImGui::DragFloat("height", &p.height, 0.05f, -8, 32, "%.2f tiles");
        ImGui::SetNextItemWidth(w);
        dirty |= ImGui::DragFloat("turn", &p.turn, 1, -360, 360, "%.0f deg");
        ImGui::SetNextItemWidth(w);
        dirty |= ImGui::DragFloat("size", &p.scale, 1, 5, 800, "%.0f%%");
        ImGui::Separator();
        ImGui::PopID();
    }
    if (remove >= 0)
        sPlaced.erase(sPlaced.begin() + remove), dirty = true;
    std::vector<const char *> names;
    for (auto &f : sFiles)
        names.push_back(f.c_str());
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::Combo("##glb", &sPick, names.data(), (int)names.size());
    if (ImGui::Button("Place on the last tile pointed at"))
    {
        Placement p;
        p.file = sFiles[std::min(sPick, (int)sFiles.size() - 1)];
        p.x = gEd.hoverX > 0 ? gEd.hoverX : 0, p.y = gEd.hoverY > 0 ? gEd.hoverY : 0;   // (the tile last under the mouse)
        sPlaced.push_back(p);
        dirty = true;
    }
    if (dirty && !ImGui::IsAnyItemActive())   // (saved once a drag or a typed value is let go)
        Save(), dirty = false;
}

// tests: "model place FILE X Y [HEIGHT TURN SCALE]", "model list"
void Models_TestCommand(const char *cmd, const char *arg)
{
    char what[16] = "", file[64] = "";
    Placement p;
    if (gEd.map < 0)
        return;
    if (gEd.map != sMap)
        Load(gEd.map);
    int n = sscanf(arg, "%15s %63s %d %d %f %f %f", what, file, &p.x, &p.y, &p.height, &p.turn, &p.scale);
    if (!strcmp(what, "place") && n >= 4)
        p.file = file, sPlaced.push_back(p), Save();
    else if (!strcmp(what, "list"))
        for (auto &q : sPlaced)
            printf("[maker] model %s at %d,%d\n", q.file.c_str(), q.x, q.y);
    (void)cmd;
}

} // namespace maker
