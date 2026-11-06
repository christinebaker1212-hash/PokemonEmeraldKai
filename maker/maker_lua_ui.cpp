// Moemon Maker, M5: the LUA SCRIPTS window (Tools > Lua scripts). The files of MoemonMaker/lua/ (src/maker/maker_lua.cpp
// loads them into the game, sandboxed): a list, the text of the open one, the game's check of it as it's typed, the
// last problem the game had running a script, and what a script can use. Saving reloads them in the game.
#include <SDL.h>
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
#include "misc/imgui_stdlib.h"

namespace maker {

static bool sOpen;
static std::vector<std::string> sFiles;
static int sSel = -1;
static std::string sText, sSaved;
static char sNewName[64] = "hooks";

static std::string Dir() { return std::string(MakerApi_EditsDir()) + "/lua"; }

static void List()
{
    sFiles.clear();
    if (DIR *d = opendir(Dir().c_str()))
    {
        while (struct dirent *e = readdir(d))
        {
            size_t n = strlen(e->d_name);
            if (n > 4 && strcmp(e->d_name + n - 4, ".lua") == 0)
                sFiles.push_back(e->d_name);
        }
        closedir(d);
    }
    std::sort(sFiles.begin(), sFiles.end());
}

static void Open(int i)
{
    sSel = i;
    sText.clear();
    if (i < 0 || i >= (int)sFiles.size())
        return;
    if (FILE *f = fopen((Dir() + "/" + sFiles[i]).c_str(), "rb"))
    {
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
            sText.append(buf, n);
        fclose(f);
    }
    sSaved = sText;
}

static void Save()
{
    if (sSel < 0 || sSel >= (int)sFiles.size())
        return;
#ifdef _WIN32
    _mkdir(MakerApi_EditsDir());
    _mkdir(Dir().c_str());
#else
    mkdir(MakerApi_EditsDir(), 0755);
    mkdir(Dir().c_str(), 0755);
#endif
    if (FILE *f = fopen((Dir() + "/" + sFiles[sSel]).c_str(), "wb"))
    {
        fwrite(sText.data(), 1, sText.size(), f);
        fclose(f);
        sSaved = sText;
        MakerApi_LuaReload();
        printf("[maker] lua: saved %s\n", sFiles[sSel].c_str());
    }
}

static void NewFile(const char *name)
{
    std::string n = name;
    if (n.empty())
        return;
    if (n.size() < 4 || n.compare(n.size() - 4, 4, ".lua") != 0)
        n += ".lua";
    List();
    auto it = std::find(sFiles.begin(), sFiles.end(), n);
    if (it == sFiles.end())
    {
        sFiles.push_back(n);
        std::sort(sFiles.begin(), sFiles.end());
        it = std::find(sFiles.begin(), sFiles.end(), n);
        sSel = (int)(it - sFiles.begin());
        sText = "-- " + n + ": hooks for the game (Tools > Lua scripts lists what they can use)\n\n"
                "function OnStep(x, y, map)\n  -- return \"EVENT_NAME\" to start an event of this map\nend\n";
        Save();
    }
    else
        Open((int)(it - sFiles.begin()));
}

static const char *const kHelp =
    "Hooks the game calls when a script has them:\n"
    "  OnStep(x, y, map)   after each step; return an event's name to start it\n"
    "  OnMapEnter(map)     when a map loads\n"
    "  OnDamageCalc(attacker, target, move, damage, side)   return a new damage (side 0: the player's)\n"
    "  OnEndTurn(turn)     as each battle turn ends\n"
    "An event line  lua FUNCTION args  calls a function; what it returns goes in VAR_RESULT (if result N).\n\n"
    "game.flag(NAME)  game.setflag(NAME, on)   FLAG_ names or your own\n"
    "game.var(NAME)  game.setvar(NAME, n)      VAR_ names\n"
    "game.item(NAME)  game.giveitem(NAME, n)  game.takeitem(NAME, n)\n"
    "game.money()  game.addmoney(n)  game.partycount()  game.species(slot)  game.level(slot)\n"
    "game.map()  game.x()  game.y()  game.random(n)  game.log(text)\n\n"
    "No files, no network, no print: the same on every player's game.";

void LuaWindow()
{
    if (!sOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(820 * gEd.uiScale, 600 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("LUA SCRIPTS", &sOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::BeginChild("files", ImVec2(180 * gEd.uiScale, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < (int)sFiles.size(); i++)
        if (ImGui::Selectable(sFiles[i].c_str(), i == sSel))
            Open(i);
    ImGui::Separator();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##new", sNewName, sizeof(sNewName));
    if (ImGui::Button("New script"))
        NewFile(sNewName);
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("edit", ImVec2(0, 0));
    if (sSel >= 0 && sSel < (int)sFiles.size())
    {
        bool dirty = sText != sSaved;
        ImGui::Text("%s%s", sFiles[sSel].c_str(), dirty ? " (not saved)" : "");
        ImGui::SameLine();
        if (ImGui::Button("Save (Ctrl+S)") || (dirty && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)))
            Save();
        const char *problem = MakerApi_LuaCheck(sFiles[sSel].c_str(), sText.c_str());
        if (problem && problem[0])
            ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "%s", problem);
        else
            ImGui::TextDisabled("Lua reads it fine.");
        ImGui::InputTextMultiline("##lua", &sText, ImVec2(-FLT_MIN, -200 * gEd.uiScale), ImGuiInputTextFlags_AllowTabInput);
    }
    else
        ImGui::TextDisabled("Pick a script, or make a new one.");
    const char *err = MakerApi_LuaError();
    if (err && err[0])
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.1f, 1), "The game: %s", err);
    ImGui::SeparatorText("WHAT A SCRIPT CAN USE");
    ImGui::TextDisabled("%s", kHelp);
    ImGui::EndChild();
    ImGui::End();
}

void OpenLua()
{
    sOpen = true;
    List();
    if (sSel < 0 && !sFiles.empty())
        Open(0);
}

// tests: "lua open", "lua new NAME", "lua text TEXT" (the open script's text: \n for new lines), "lua save"
void Lua_TestCommand(const char *cmd, const char *arg)
{
    char what[16] = "";
    sscanf(arg, "%15s", what);
    const char *rest = strchr(arg, ' ') ? strchr(arg, ' ') + 1 : "";
    if (!strcmp(what, "open"))
        OpenLua();
    else if (!strcmp(what, "new"))
        OpenLua(), NewFile(rest);
    else if (!strcmp(what, "text"))
    {
        sText.clear();
        for (const char *p = rest; *p; p++)
            if (p[0] == '\\' && p[1] == 'n')
                sText += '\n', p++;
            else
                sText += *p;
    }
    else if (!strcmp(what, "save"))
        Save();
    else if (!strcmp(what, "check"))
        printf("[maker] lua check: %s\n", sSel >= 0 ? MakerApi_LuaCheck("check", sText.c_str()) : "-");
    (void)cmd;
}

} // namespace maker
