// Moemon Maker, M2: events in the editor (docs/MOEMON_MAKER.md "Events", decisions 17 and 18). A person, sign or
// trigger can have an event: pages (conditions and a trigger) with an indented command list. Two views of the same
// text, both editable: the command list (double-click a line to change it; Insert opens the command picker) and the
// text itself (Ctrl+T). The game compiles it as the user types (MakerApi_CheckEvent: the first problem shows under
// it), and F5 plays it. The language: src/maker/maker_events.c's header.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>
#include "maker_ui.h"
#include "imgui_internal.h"

namespace maker {

// ---- The command picker: every command with what it does and the line it starts as

struct CommandInfo
{
    const char *tab, *name, *help;
    std::vector<const char *> lines;   // (an "if" brings its "end")
};

static const char *const kTabs[] = {"Message", "Flow", "Party & Items", "Movement", "Screen", "Sound", "Battle", "Advanced"};

static const std::vector<CommandInfo> &Commands()
{
    static const std::vector<CommandInfo> sCmds = {
        {"Message", "Show Text", "A message box (\\n: a new line, \\p: a new box, {PLAYER}: the player's name).", {"text \"Hello!\""}},
        {"Message", "Yes / No Question", "A question; then \"if yes\" / \"if no\".", {"yesno \"Do you want it?\"", "if yes", "  text \"Here you go!\"", "else", "  text \"Oh, okay.\"", "end"}},
        {"Message", "Show Choices", "Up to 8 answers; then \"if choice 1\", \"if choice 2\"...", {"choice \"YES\" \"NO\" \"MAYBE\"", "if choice 1", "  text \"Great!\"", "end"}},
        {"Flow", "If a Flag Is Set", "Runs the lines inside when the flag is set (a game FLAG_ or a name of your own).", {"if flag MY_FLAG", "  text \"...\"", "end"}},
        {"Flow", "If a Flag Isn't Set", "Runs the lines inside when the flag isn't set.", {"if !flag MY_FLAG", "  text \"...\"", "end"}},
        {"Flow", "If a Self Switch", "This event's own switches A to D.", {"if self A", "  text \"...\"", "end"}},
        {"Flow", "If a Variable", "Compares one of the game's VAR_ variables (== != < > <= >=).", {"if var VAR_TEMP_1 >= 1", "  text \"...\"", "end"}},
        {"Flow", "If the Player Has an Item", "With how many (1 if left out).", {"if item ITEM_POTION 1", "  text \"...\"", "end"}},
        {"Flow", "If the Player Has Money", "At least this much.", {"if money 1000", "  text \"...\"", "end"}},
        {"Flow", "Else", "The other way, inside an if.", {"else"}},
        {"Flow", "Set a Flag", "", {"setflag MY_FLAG"}},
        {"Flow", "Clear a Flag", "", {"clearflag MY_FLAG"}},
        {"Flow", "Self Switch", "Turns one of this event's switches on or off (pages can ask for them).", {"self A on"}},
        {"Flow", "Set a Variable", "", {"setvar VAR_TEMP_1 1"}},
        {"Flow", "Add to a Variable", "", {"addvar VAR_TEMP_1 1"}},
        {"Flow", "Stop Here", "Ends the event at this line.", {"stop"}},
        {"Party & Items", "Give an Item", "With its fanfare and words.", {"giveitem ITEM_POTION 1"}},
        {"Party & Items", "Take an Item", "", {"takeitem ITEM_POTION 1"}},
        {"Party & Items", "Give a POKeMON", "Species, level, held item (optional).", {"givemon SPECIES_PIKACHU 5"}},
        {"Party & Items", "Money", "+ gives, - takes.", {"money +500"}},
        {"Party & Items", "Heal the Party", "", {"heal"}},
        {"Movement", "Move Someone", "player, this or a LOCALID_; up down left right, fast_up..., face_up..., jump, excl, wait.", {"move this up up"}},
        {"Movement", "Move the Player", "", {"move player left"}},
        {"Movement", "Remove from the Map", "This person goes away (until the map loads again).", {"remove this"}},
        {"Movement", "Warp", "To a map's tile.", {"warp MAP_LITTLEROOT_TOWN 5 5"}},
        {"Screen", "Fade Out", "", {"fade out"}},
        {"Screen", "Fade In", "", {"fade in"}},
        {"Screen", "Wait", "In frames (60 a second).", {"wait 30"}},
        {"Sound", "Play Music", "", {"music MUS_ROUTE101"}},
        {"Sound", "Play a Fanfare", "Waits for it to end.", {"fanfare MUS_OBTAIN_ITEM"}},
        {"Sound", "Play a Sound", "", {"sound SE_SELECT"}},
        {"Battle", "Trainer Battle", "The trainer's own battle; the lines after it run once they're beaten.", {"battle TRAINER_CALVIN_1 \"Let's battle!\" \"You won!\"", "text \"That was a good battle.\""}},
        {"Battle", "Wild Battle", "", {"wildbattle SPECIES_ZIGZAGOON 5"}},
        {"Advanced", "Special", "One of the game's specials by name.", {"special HealPlayerParty"}},
        {"Advanced", "Face the Player", "", {"face"}},
        {"Advanced", "Lock", "Stops others moving.", {"lock"}},
        {"Advanced", "Release", "", {"release"}},
    };
    return sCmds;
}

// ---- The open event

static std::string sOpenMapName, sOpenEvent;   // the event in the inspector (by map name and its name)
static int sSelLine = -1;
static bool sTextView;
static std::string sTextBuf;                    // the text view's copy while it's being typed in
static bool sTextEditing;
static int sPickerOpen;                         // 1: open the picker (inserting after sSelLine)
static char sPickerFilter[64];
static int sEditLine = -1;                      // the line in the line editor
static char sEditBuf[512];

std::string EventOn(int map, const std::string &on)
{
    const MapEdits *e = FindEdits(map);
    if (!e)
        return "";
    for (auto &ev : e->events)
        if (ev.second.on == on)
            return ev.first;
    return "";
}

static std::vector<std::string> Lines(const std::string &body)
{
    std::vector<std::string> out;
    size_t pos = 0;
    while (pos < body.size())
    {
        size_t nl = body.find('\n', pos);
        out.push_back(body.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos));
        pos = nl == std::string::npos ? body.size() : nl + 1;
    }
    return out;
}

static std::string Join(const std::vector<std::string> &lines)
{
    std::string s;
    for (auto &l : lines)
        s += l + "\n";
    return s;
}

static int Indent(const std::string &l)
{
    size_t i = l.find_first_not_of(' ');
    return i == std::string::npos ? 0 : (int)i;
}

static std::string Trim(const std::string &l)
{
    size_t a = l.find_first_not_of(" \t");
    if (a == std::string::npos)
        return "";
    return l.substr(a);
}

static int KindOf(const std::string &on)
{
    if (on.rfind("sign:", 0) == 0)
        return 1;   // MAKER_EVENT_ON_SIGN
    if (on.rfind("trigger:", 0) == 0)
        return 2;
    if (on == "map")
        return 3;
    return 0;
}

static void SetBody(int map, const std::string &name, const std::string &body)
{
    BeginChange(map);
    Edits(map).events[name].body = body;
    EndChange();
}

// Inserts a command's lines after line AT (inside a block that line opens: indented one more)
static void InsertCommand(int map, const std::string &name, const CommandInfo &c, int at)
{
    std::vector<std::string> lines = Lines(Edits(map).events[name].body);
    int indent = 2;
    if (at >= 0 && at < (int)lines.size())
    {
        std::string t = Trim(lines[at]);
        indent = Indent(lines[at]);
        if (t.rfind("page", 0) == 0 || t.rfind("if ", 0) == 0 || t == "else")
            indent += 2;
    }
    else
        at = (int)lines.size() - 1;
    std::vector<std::string> add;
    for (const char *l : c.lines)
        add.push_back(std::string(indent, ' ') + l);
    lines.insert(lines.begin() + (at + 1), add.begin(), add.end());
    SetBody(map, name, Join(lines));
    sSelLine = at + 1;
}

// ---- Colors by what a line is

static ImVec4 LineColor(const std::string &t)
{
    bool dark = gEd.dark;
    if (t.rfind("page", 0) == 0)
        return dark ? ImVec4(1.0f, 0.8f, 0.4f, 1) : ImVec4(0.55f, 0.3f, 0.0f, 1);
    if (t.rfind("if ", 0) == 0 || t == "else" || t == "end" || t == "stop")
        return dark ? ImVec4(0.55f, 0.75f, 1.0f, 1) : ImVec4(0.1f, 0.3f, 0.75f, 1);
    if (t.rfind("text ", 0) == 0 || t.rfind("yesno ", 0) == 0 || t.rfind("choice ", 0) == 0)
        return dark ? ImVec4(0.75f, 0.95f, 0.75f, 1) : ImVec4(0.1f, 0.45f, 0.1f, 1);
    if (t.rfind("battle ", 0) == 0 || t.rfind("wildbattle ", 0) == 0)
        return dark ? ImVec4(1.0f, 0.6f, 0.6f, 1) : ImVec4(0.7f, 0.1f, 0.1f, 1);
    return ImGui::GetStyleColorVec4(ImGuiCol_Text);
}

// A page line read back for the page editor: its trigger and its conditions
struct PageInfo { std::string trigger = "talk"; std::vector<std::pair<std::string, std::string>> conds; };
static PageInfo ReadPage(const std::string &t)
{
    PageInfo p;
    size_t pos = 4;
    while (pos < t.size())
    {
        size_t a = t.find_first_not_of(' ', pos);
        if (a == std::string::npos)
            break;
        size_t b = t.find(' ', a);
        std::string w = t.substr(a, b == std::string::npos ? std::string::npos : b - a);
        size_t eq = w.find('=');
        if (eq != std::string::npos)
        {
            if (w.substr(0, eq) == "trigger")
                p.trigger = w.substr(eq + 1);
            else
                p.conds.push_back({w.substr(0, eq), w.substr(eq + 1)});
        }
        pos = b == std::string::npos ? t.size() : b;
    }
    return p;
}

static std::string WritePage(const PageInfo &p, int indent)
{
    std::string s = std::string(indent, ' ') + "page trigger=" + p.trigger;
    for (auto &c : p.conds)
        if (!c.first.empty() && !c.second.empty())
            s += " " + c.first + "=" + c.second;
    return s;
}

static const char *const kTriggers[] = {"talk", "step", "autorun"};
static const char *const kCondKeys[] = {"flag", "noflag", "self", "noself", "var", "item", "badges", "time"};

// The line editor (double-click): a page line gets its trigger and conditions as fields, anything else its text
static void LineEditor(int map, const std::string &name)
{
    if (sEditLine < 0)
        return;
    std::vector<std::string> lines = Lines(Edits(map).events[name].body);
    if (sEditLine >= (int)lines.size())
    {
        sEditLine = -1;
        return;
    }
    ImGui::OpenPopup("Edit line");
    ImGui::SetNextWindowSize(ImVec2(520 * gEd.uiScale, 0), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal("Edit line", NULL, 0))
        return;
    std::string t = Trim(lines[sEditLine]);
    int indent = Indent(lines[sEditLine]);
    bool done = false, cancel = false;
    if (t.rfind("page", 0) == 0)
    {
        static PageInfo p;
        if (ImGui::IsWindowAppearing())
            p = ReadPage(t);
        ImGui::TextDisabled("Runs when");
        ImGui::SameLine();
        int trig = (int)(std::find(std::begin(kTriggers), std::end(kTriggers), p.trigger) - std::begin(kTriggers));
        ImGui::SetNextItemWidth(160 * gEd.uiScale);
        if (ImGui::Combo("##trig", &trig, kTriggers, 3))
            p.trigger = kTriggers[trig];
        ImGui::TextDisabled("The page shows only while all of these hold:");
        for (size_t i = 0; i < p.conds.size(); i++)
        {
            ImGui::PushID((int)i);
            int k = (int)(std::find(std::begin(kCondKeys), std::end(kCondKeys), p.conds[i].first) - std::begin(kCondKeys));
            ImGui::SetNextItemWidth(110 * gEd.uiScale);
            if (ImGui::Combo("##k", &k, kCondKeys, IM_ARRAYSIZE(kCondKeys)))
                p.conds[i].first = kCondKeys[k];
            ImGui::SameLine();
            char v[128];
            snprintf(v, sizeof(v), "%s", p.conds[i].second.c_str());
            ImGui::SetNextItemWidth(-40 * gEd.uiScale);
            if (ImGui::InputText("##v", v, sizeof(v)))
                p.conds[i].second = v;
            ImGui::SameLine();
            if (ImGui::SmallButton("x"))
                p.conds.erase(p.conds.begin() + i), i--;
            ImGui::PopID();
        }
        if (ImGui::SmallButton("+ condition"))
            p.conds.push_back({"flag", "MY_FLAG"});
        ImGui::TextDisabled("flag / noflag: FLAG_ or your own name; self: A-D; var: VAR_X>=1; item: ITEM_X*2;\n"
                            "badges: at least N; time: morning, day, evening or night");
        if (ImGui::Button("OK"))
        {
            lines[sEditLine] = WritePage(p, indent);
            done = true;
        }
    }
    else
    {
        if (ImGui::IsWindowAppearing())
        {
            snprintf(sEditBuf, sizeof(sEditBuf), "%s", t.c_str());
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool enter = ImGui::InputText("##line", sEditBuf, sizeof(sEditBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::Button("OK") || enter)
        {
            lines[sEditLine] = std::string(indent, ' ') + sEditBuf;
            done = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
        cancel = true;
    if (done)
        SetBody(map, name, Join(lines));
    if (done || cancel)
    {
        sEditLine = -1;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// The command picker (Insert / Ctrl+Space): tabs, or type to find
void EventPickerWindow()
{
    if (!sPickerOpen || gEd.map < 0 || sOpenEvent.empty())
        return;
    int map = gEd.map;
    if (sPickerOpen == 1)
    {
        ImGui::OpenPopup("Add a command");
        sPickerOpen = 2;
        sPickerFilter[0] = 0;
    }
    ImGui::SetNextWindowSize(ImVec2(560 * gEd.uiScale, 420 * gEd.uiScale), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal("Add a command", NULL, 0))
    {
        sPickerOpen = 0;
        return;
    }
    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##find", "Type to find a command (\"giv\" finds Give an Item)...", sPickerFilter, sizeof(sPickerFilter));
    const CommandInfo *pick = nullptr;
    auto row = [&](const CommandInfo &c) {
        ImGui::PushID(&c);
        if (ImGui::Selectable(c.name, false, ImGuiSelectableFlags_AllowDoubleClick))
            pick = &c;
        if (c.help[0])
        {
            ImGui::SameLine(220 * gEd.uiScale);
            ImGui::TextDisabled("%s", c.help);
        }
        ImGui::PopID();
    };
    if (sPickerFilter[0])
    {
        std::string f = sPickerFilter;
        std::transform(f.begin(), f.end(), f.begin(), ::tolower);
        ImGui::BeginChild("found", ImVec2(0, -30 * gEd.uiScale));
        for (const CommandInfo &c : Commands())
        {
            std::string n = std::string(c.name) + " " + c.lines[0];
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.find(f) != std::string::npos)
                row(c);
        }
        ImGui::EndChild();
        if (ImGui::IsKeyPressed(ImGuiKey_Enter))
            for (const CommandInfo &c : Commands())
            {
                std::string n = std::string(c.name) + " " + c.lines[0];
                std::transform(n.begin(), n.end(), n.begin(), ::tolower);
                if (n.find(f) != std::string::npos)
                {
                    pick = &c;
                    break;
                }
            }
    }
    else if (ImGui::BeginTabBar("tabs"))
    {
        for (const char *tab : kTabs)
            if (ImGui::BeginTabItem(tab))
            {
                ImGui::BeginChild("list", ImVec2(0, -30 * gEd.uiScale));
                for (const CommandInfo &c : Commands())
                    if (!strcmp(c.tab, tab))
                        row(c);
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
        ImGui::EndTabBar();
    }
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        sPickerOpen = 0;
        ImGui::CloseCurrentPopup();
    }
    if (pick)
    {
        InsertCommand(map, sOpenEvent, *pick, sSelLine);
        sPickerOpen = 0;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// ---- The inspector's EVENT section, for what's selected (ON: "person:KEY" etc.; ADDED: an added person / sign's
// properties, whose script= becomes event:NAME)

static std::string NewName(int map, const std::string &suggested)
{
    std::string base;
    for (char ch : suggested)
        base += isalnum((unsigned char)ch) ? (char)toupper((unsigned char)ch) : '_';
    if (base.empty())
        base = "EVENT";
    const MapEdits *e = FindEdits(map);
    std::string name = base;
    for (int n = 2; e && e->events.count(name); n++)
        name = base + "_" + std::to_string(n);
    return name;
}

void EventSection(const char *on, const std::string &suggestedName, std::map<std::string, std::string> *added)
{
    int map = gEd.map;
    std::string name;
    if (added)
    {
        auto it = added->find("script");
        if (it != added->end() && it->second.rfind("event:", 0) == 0)
            name = it->second.substr(6);
    }
    else
        name = EventOn(map, on);
    ImGui::Spacing();
    ImGui::PushFont(gEd.fontBold);
    ImGui::SeparatorText("EVENT");
    ImGui::PopFont();
    const MapEdits *fe = FindEdits(map);
    if (name.empty() || !fe || !fe->events.count(name))
    {
        ImGui::TextDisabled("It does what the game's own script does.");
        if (ImGui::Button("Give it an event of its own"))
        {
            name = NewName(map, suggestedName);
            BeginChange(map);
            EventDef &d = Edits(map).events[name];
            d.on = added ? "added" : on;
            d.body = std::string("page trigger=") + (KindOf(d.on) == 2 ? "step" : "talk") + "\n  text \"Hello!\"\n";
            if (added)
                (*added)["script"] = "event:" + name;
            EndChange();
            sSelLine = 1;
        }
        return;
    }
    sOpenMapName = gEd.maps[map].name;
    sOpenEvent = name;
    const EventDef &d = Edits(map).events[name];
    ImGui::Text("%s", name.c_str());
    ImGui::SameLine();
    if (ImGui::SmallButton(sTextView ? "Command list (Ctrl+T)" : "Text (Ctrl+T)"))
        sTextView = !sTextView, sTextEditing = false;
    ImGuiIO &io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_T, false) && !io.WantTextInput)
        sTextView = !sTextView, sTextEditing = false;
    // what the game makes of it
    std::string src;
    for (auto &l : Lines(d.body))
        src += "  " + l + "\n";
    const char *problem = MakerApi_CheckEvent(sOpenMapName.c_str(), name.c_str(), KindOf(d.on), src.c_str());
    if (problem && problem[0])
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "%s", problem);
    else
        ImGui::TextDisabled("The game reads it fine.");
    if (sTextView)
    {
        if (!sTextEditing)
            sTextBuf = d.body;
        sTextBuf.reserve(sTextBuf.size() + 4096);
        ImGui::InputTextMultiline("##evtext", &sTextBuf[0], sTextBuf.capacity(), ImVec2(-FLT_MIN, 260 * gEd.uiScale),
                                  ImGuiInputTextFlags_AllowTabInput);
        sTextBuf = sTextBuf.c_str();
        sTextEditing = ImGui::IsItemActive();
        if (ImGui::IsItemDeactivatedAfterEdit())
            SetBody(map, name, sTextBuf);
        return;
    }
    std::vector<std::string> lines = Lines(d.body);
    ImGui::BeginChild("cmds", ImVec2(-FLT_MIN, 260 * gEd.uiScale), ImGuiChildFlags_Borders);
    for (int i = 0; i < (int)lines.size(); i++)
    {
        std::string t = Trim(lines[i]);
        if (t.empty())
            continue;
        ImGui::PushID(i);
        bool page = t.rfind("page", 0) == 0;
        if (page)
            ImGui::PushFont(gEd.fontBold);
        ImGui::PushStyleColor(ImGuiCol_Text, LineColor(t));
        char label[600];
        snprintf(label, sizeof(label), "%*s%s %s", Indent(lines[i]), "", page ? "" : "*", t.c_str());
        if (ImGui::Selectable(label, sSelLine == i, ImGuiSelectableFlags_AllowDoubleClick))
        {
            sSelLine = i;
            if (ImGui::IsMouseDoubleClicked(0))
                sEditLine = i;
        }
        ImGui::PopStyleColor();
        if (page)
            ImGui::PopFont();
        ImGui::PopID();
    }
    ImGui::EndChild();
    if (ImGui::Button("Insert...") || (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Space, false)))
        sPickerOpen = 1;
    ImGui::SameLine();
    if (ImGui::Button("Edit") && sSelLine >= 0)
        sEditLine = sSelLine;
    ImGui::SameLine();
    if (ImGui::Button("Delete") && sSelLine >= 0 && sSelLine < (int)lines.size())
    {
        lines.erase(lines.begin() + sSelLine);
        SetBody(map, name, Join(lines));
        sSelLine = std::min(sSelLine, (int)lines.size() - 1);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("up", ImGuiDir_Up) && sSelLine > 0 && sSelLine < (int)lines.size())
    {
        std::swap(lines[sSelLine], lines[sSelLine - 1]);
        SetBody(map, name, Join(lines));
        sSelLine--;
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("down", ImGuiDir_Down) && sSelLine >= 0 && sSelLine + 1 < (int)lines.size())
    {
        std::swap(lines[sSelLine], lines[sSelLine + 1]);
        SetBody(map, name, Join(lines));
        sSelLine++;
    }
    ImGui::SameLine();
    if (ImGui::Button("New page"))
    {
        lines.push_back("page trigger=talk self=A");
        lines.push_back("  text \"...\"");
        SetBody(map, name, Join(lines));
        sSelLine = (int)lines.size() - 2;
    }
    if (ImGui::Button("Take the event away"))
    {
        BeginChange(map);
        Edits(map).events.erase(name);
        if (added)
            (*added)["script"] = "nothing";
        EndChange();
        sOpenEvent.clear();
        return;
    }
    ImGui::TextDisabled("Double-click a line to change it. The last page whose conditions hold is the one that runs.");
    LineEditor(map, name);
    EventPickerWindow();
}

// ---- Quick events (Events mode's right-click): a person or sign with an event ready to adjust

struct Quick { const char *label, *look, *body; bool sign; };
static const Quick kQuick[] = {
    {"Talking person", "OBJ_EVENT_GFX_BOY_1", "page trigger=talk\n  text \"Hello there!\"\n", false},
    {"Trainer", "OBJ_EVENT_GFX_YOUNGSTER", "page trigger=talk\n  battle TRAINER_CALVIN_1 \"Let's battle!\" \"You're strong!\"\n  text \"I need to train more.\"\n", false},
    {"Gift POKeMON", "OBJ_EVENT_GFX_SCIENTIST_1", "page trigger=talk\n  yesno \"Would you like this POKeMON?\"\n  if yes\n    givemon SPECIES_EEVEE 5\n    self A on\n  end\npage trigger=talk self=A\n  text \"Take good care of it!\"\n", false},
    {"Item giver", "OBJ_EVENT_GFX_WOMAN_2", "page trigger=talk\n  text \"Here, take this!\"\n  giveitem ITEM_POTION 1\n  self A on\npage trigger=talk self=A\n  text \"Use it well!\"\n", false},
    {"Nurse", "OBJ_EVENT_GFX_NURSE", "page trigger=talk\n  yesno \"Shall I heal your POKeMON?\"\n  if yes\n    fade out\n    heal\n    fanfare MUS_HEAL\n    fade in\n    text \"Your POKeMON are fighting fit!\"\n  end\n", false},
    {"Static encounter", "OBJ_EVENT_GFX_ZIGZAGOON_1", "page trigger=talk\n  wildbattle SPECIES_ZIGZAGOON 10\n  remove this\n", false},
    {"Sign", "", "page trigger=talk\n  text \"ROUTE 101\\nLITTLEROOT TOWN is to the south.\"\n", true},
};

void QuickEventMenu(int x, int y)
{
    if (!ImGui::BeginMenu("Quick event here"))
        return;
    for (const Quick &q : kQuick)
        if (ImGui::MenuItem(q.label))
        {
            int map = gEd.map;
            MapEdits &e = Edits(map);
            int n = 1;
            for (auto &a : e.adds)
                n = std::max(n, atoi(a.first.substr(a.first.find(' ') + 1).c_str()) + 1);
            std::string name = NewName(map, q.label);
            std::map<std::string, std::string> a;
            a["x"] = std::to_string(x);
            a["y"] = std::to_string(y);
            a["elevation"] = "3";
            if (!q.sign)
            {
                a["look"] = q.look;
                a["movement"] = "MOVEMENT_TYPE_FACE_DOWN";
            }
            a["script"] = "event:" + name;
            std::string key = std::string(q.sign ? "sign " : "person ") + std::to_string(n);
            BeginChange(map);
            e.adds[key] = a;
            e.events[name] = EventDef{"added", q.body};
            EndChange();
            gEd.selKind = SEL_ADDED;
            gEd.selAdded = key;
            sSelLine = 1;
        }
    ImGui::EndMenu();
}

// Tests: "maker event NAME ON" then "maker eventline TEXT" lines; "maker eventview list|text"; "maker eventpicker"
void Events_TestCommand(const char *cmd, const char *arg)
{
    static std::string sName;
    std::string c = cmd;
    if (gEd.map < 0)
        return;
    if (c == "event")
    {
        char name[64] = "", on[160] = "";
        sscanf(arg, "%63s %159s", name, on);
        sName = name;
        BeginChange(gEd.map);
        Edits(gEd.map).events[sName] = EventDef{on, ""};
        EndChange();
    }
    else if (c == "eventline" && !sName.empty())
    {
        BeginChange(gEd.map);
        Edits(gEd.map).events[sName].body += std::string(arg) + "\n";
        EndChange();
    }
    else if (c == "eventview")
        sTextView = !strcmp(arg, "text");
    else if (c == "eventpicker")
        sPickerOpen = 1;
    else if (c == "eventselect")
        sSelLine = atoi(arg);
}

} // namespace maker
