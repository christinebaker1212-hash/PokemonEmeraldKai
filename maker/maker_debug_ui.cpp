// Moemon Maker, M4: the DEBUGGER window (Tools > Debugger). Breakpoints on the lines of the open map's events (a click
// on a line), and while a test game plays (F5): where a script is held, STEP (F10) to the next line, CONTINUE (F8) to the
// next breakpoint, and WATCH: flags (FLAG_... or the user's own by name), variables (VAR_...) and self switches
// (SELF:MAP:EVENT:A) with their values as they are now. The runtime side is src/maker/maker_events.c's debugger.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

static bool sOpen;
static std::string sEvent;            // the open map's event shown
static std::vector<std::string> sWatch = {"VAR_RESULT", "FLAG_SYS_POKEMON_GET"};
static char sWatchAdd[96];

// An event's text as the compiler numbers it: blank lines left out, the first line 1
static std::vector<std::string> Lines(const std::string &body)
{
    std::vector<std::string> out;
    size_t a = 0;
    while (a <= body.size())
    {
        size_t b = body.find('\n', a);
        if (b == std::string::npos)
            b = body.size();
        if (b > a)
            out.push_back(body.substr(a, b - a));
        a = b + 1;
    }
    return out;
}

static bool IsCommand(const std::string &line)
{
    size_t i = line.find_first_not_of(" \t");
    return i != std::string::npos && line[i] != '#' && line.compare(i, 4, "page") != 0;
}

void DebuggerWindow()
{
    if (!sOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(720 * gEd.uiScale, 560 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("DEBUGGER", &sOpen))
    {
        ImGui::End();
        return;
    }
    char hmap[64] = "", hevent[64] = "";
    int hline = 0;
    bool playing = MakerApi_IsPlaying(), held = MakerApi_DebugPaused(hmap, sizeof(hmap), hevent, sizeof(hevent), &hline);
    if (held)
    {
        ImGui::TextColored(ImVec4(0.9f, 0.5f, 0.1f, 1), "Held at %s, %s, line %d", hmap, hevent, hline);
        ImGui::SameLine();
        if (ImGui::Button("Step (F10)") || ImGui::IsKeyPressed(ImGuiKey_F10))
            MakerApi_DebugStep();
        ImGui::SameLine();
        if (ImGui::Button("Continue (F8)") || ImGui::IsKeyPressed(ImGuiKey_F8))
            MakerApi_DebugContinue();
    }
    else
        ImGui::TextDisabled(playing ? "Playing: a breakpoint holds the event there." : "Not playing: F5 plays the map; breakpoints wait for it.");
    ImGui::Separator();

    // the open map's events and their lines
    const MapEdits *e = gEd.map >= 0 ? FindEdits(gEd.map) : nullptr;
    std::string mapName = gEd.map >= 0 ? gEd.maps[gEd.map].name : "";
    ImGui::BeginChild("events", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, 0), ImGuiChildFlags_Borders);
    if (!e || e->events.empty())
        ImGui::TextDisabled("The open map has no events of yours.");
    else
    {
        if (!e->events.count(sEvent))
            sEvent = e->events.begin()->first;
        if (held && hmap == mapName && e->events.count(hevent))
            sEvent = hevent;
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##ev", sEvent.c_str()))
        {
            for (auto &kv : e->events)
                if (ImGui::Selectable(kv.first.c_str(), kv.first == sEvent))
                    sEvent = kv.first;
            ImGui::EndCombo();
        }
        std::vector<std::string> lines = Lines(e->events.at(sEvent).body);
        for (size_t i = 0; i < lines.size(); i++)
        {
            int n = (int)i + 1;
            bool cmd = IsCommand(lines[i]);
            bool bp = cmd && MakerApi_DebugHasBreak(mapName.c_str(), sEvent.c_str(), n);
            bool here = held && hmap == mapName && sEvent == hevent && hline == n;
            ImGui::PushID(n);
            if (cmd)
            {
                if (ImGui::Selectable(bp ? "##bp_on" : "##bp", bp, 0, ImVec2(16 * gEd.uiScale, 0)))
                    MakerApi_DebugSetBreak(mapName.c_str(), sEvent.c_str(), n, !bp);
                ImGui::SameLine(0, 0);
                ImGui::TextColored(bp ? ImVec4(0.85f, 0.15f, 0.15f, 1) : ImVec4(0.5f, 0.5f, 0.5f, 0.6f), bp ? "*" : ".");
            }
            else
                ImGui::Dummy(ImVec2(22 * gEd.uiScale, 0));
            ImGui::SameLine();
            if (here)
                ImGui::TextColored(ImVec4(0.9f, 0.5f, 0.1f, 1), "%3d > %s", n, lines[i].c_str());
            else
                ImGui::TextDisabled("%3d", n), ImGui::SameLine(), ImGui::TextUnformatted(lines[i].c_str());
            ImGui::PopID();
        }
        ImGui::TextDisabled("Click the dot before a command to set or clear a breakpoint.");
        if (ImGui::SmallButton("Clear every breakpoint"))
            MakerApi_DebugClearBreaks();
    }
    ImGui::EndChild();
    ImGui::SameLine();

    // WATCH
    ImGui::BeginChild("watch", ImVec2(0, 0), ImGuiChildFlags_Borders);
    ImGui::SeparatorText("WATCH");
    int remove = -1;
    for (size_t i = 0; i < sWatch.size(); i++)
    {
        int v = 0;
        ImGui::PushID((int)i);
        if (ImGui::SmallButton("x"))
            remove = (int)i;
        ImGui::SameLine();
        if (!playing)
            ImGui::TextDisabled("%s", sWatch[i].c_str());
        else if (MakerApi_DebugValue(sWatch[i].c_str(), &v))
            ImGui::Text("%s = %d", sWatch[i].c_str(), v);
        else
            ImGui::TextDisabled("%s (no such name yet)", sWatch[i].c_str());
        ImGui::PopID();
    }
    if (remove >= 0)
        sWatch.erase(sWatch.begin() + remove);
    ImGui::SetNextItemWidth(-60 * gEd.uiScale);
    bool add = ImGui::InputTextWithHint("##wa", "FLAG_, VAR_, a flag of yours", sWatchAdd, sizeof(sWatchAdd),
                                        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    if ((ImGui::Button("Add") || add) && sWatchAdd[0])
        sWatch.push_back(sWatchAdd), sWatchAdd[0] = 0;
    ImGui::EndChild();
    ImGui::End();
}

void OpenDebugger()
{
    sOpen = true;
}

// Tests: "maker break MAP EVENT LINE" sets a breakpoint; "maker step" / "maker continue"; "maker held" prints where a
// script is held; "maker watch NAME" prints a value; "maker debugger" opens the window
void Debugger_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (c == "debugger")
        sOpen = true;
    else if (c == "break")
    {
        char m[64], ev[64];
        int line = 0;
        if (sscanf(arg, "%63s %63s %d", m, ev, &line) == 3)
            MakerApi_DebugSetBreak(m, ev, line, 1);
    }
    else if (c == "step")
        MakerApi_DebugStep();
    else if (c == "continue")
        MakerApi_DebugContinue();
    else if (c == "held")
    {
        char m[64], ev[64];
        int line;
        if (MakerApi_DebugPaused(m, sizeof(m), ev, sizeof(ev), &line))
            printf("[maker] held at %s %s line %d\n", m, ev, line);
        else
            printf("[maker] nothing held\n");
    }
    else if (c == "watch")
    {
        int v;
        if (MakerApi_DebugValue(arg, &v))
            printf("[maker] watch %s = %d\n", arg, v);
        else
            printf("[maker] watch %s: no such name\n", arg);
    }
}

} // namespace maker
