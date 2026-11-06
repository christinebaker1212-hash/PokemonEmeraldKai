// Moemon Maker, M4c: the three-way merge (docs/MOEMON_MAKER.md, principle 2). Every edit keeps the value it replaced
// (WAS); when a game update changes that value, the game leaves the edit out (src/maker/maker_overrides.c) rather than
// lose the game's change or the user's silently. The map's inspector then shows THE GAME CHANGED: each such edit with
// the game's value then, the game's value now and the user's, and two choices: KEEP MINE (the edit now replaces the
// game's new value) or TAKE THE GAME'S (the edit goes). An edit whose person / sign / warp is gone can only go.
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

static int sMap = -1;
static unsigned sVersion = ~0u;
static std::vector<MakerStale> sStale;

static void Refresh(int map)
{
    if (map == sMap && EditsVersion() == sVersion)
        return;
    sMap = map;
    sVersion = EditsVersion();
    sStale.clear();
    MakerStale buf[256];
    int n = MakerApi_Stale(map, buf, 256);
    for (int i = 0; i < n && i < 256; i++)
        sStale.push_back(buf[i]);
}

static std::string PropOf(const std::string &key)
{
    size_t sp = key.rfind(' ');
    return sp == std::string::npos ? key : key.substr(sp + 1);
}

static std::string Value(const MakerStale &s, long v)
{
    char buf[32];
    if (strncmp(s.key, "block ", 6) == 0)
    {
        snprintf(buf, sizeof(buf), "tile %ld", v & 0x3FF);
        return buf;
    }
    return EditValueText(PropOf(s.key), v);
}

// KEEP (the edit applies over the game's value now) or drop it
static void Resolve(int map, const MakerStale &s, bool keep)
{
    std::string key = s.key;
    BeginChange(map);
    MapEdits &e = Edits(map);
    unsigned x, y;
    if (sscanf(s.key, "block %u %u", &x, &y) == 2)
    {
        int at = (int)(y * e.w + x);
        auto it = e.blocks.find(at);
        if (it != e.blocks.end())
        {
            if (keep && !s.gone)
                it->second.first = (int)s.base;
            else
                e.blocks.erase(it);
        }
    }
    else if (key.rfind("event ", 0) == 0)
        e.events.erase(key.substr(6));
    else
    {
        auto it = e.props.find(key);
        if (it != e.props.end())
        {
            if (keep && !s.gone)
                it->second.was = s.base;
            else
                e.props.erase(it);
        }
    }
    EndChange();
}

// The map inspector's section, when the game changed under some of the map's edits; true when it showed
bool Merge_Section(int map)
{
    if (map < 0 || MakerApi_IsNewMap(map))
        return false;
    Refresh(map);
    if (sStale.empty())
        return false;
    bool dark = gEd.dark;
    ImVec4 warn = dark ? ImVec4(1.0f, 0.55f, 0.45f, 1) : ImVec4(0.75f, 0.15f, 0.05f, 1);
    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Text, warn);
    ImGui::SeparatorText(("THE GAME CHANGED (" + std::to_string(sStale.size()) + ")").c_str());
    ImGui::PopStyleColor();
    ImGui::TextWrapped("An update changed these since you edited them. They're left out until you choose: keep your edit, "
                       "or take the game's new value.");
    int resolve = -1;
    bool keep = false;
    for (size_t i = 0; i < sStale.size(); i++)   // (one entry each: what, then the three values, then the choices)
    {
        const MakerStale &s = sStale[i];
        bool event = strncmp(s.key, "event ", 6) == 0;
        ImGui::PushID((int)i);
        ImGui::Separator();
        ImGui::PushFont(gEd.fontBold);
        ImGui::TextWrapped("%s", s.key);
        ImGui::PopFont();
        if (event)
            ImGui::TextWrapped("Its own event, on something the game no longer has.");
        else
        {
            ImGui::TextDisabled("game then");
            ImGui::SameLine();
            ImGui::Text("%s", Value(s, s.was).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("  game now");
            ImGui::SameLine();
            if (s.gone)
                ImGui::TextColored(warn, "gone");
            else
                ImGui::Text("%s", Value(s, s.base).c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("  yours");
            ImGui::SameLine();
            ImGui::Text("%s", Value(s, s.now).c_str());
        }
        if (!s.gone)
        {
            if (ImGui::SmallButton("Keep mine"))
                resolve = (int)i, keep = true;
            ImGui::SameLine();
            if (ImGui::SmallButton("Take the game's"))
                resolve = (int)i, keep = false;
        }
        else if (ImGui::SmallButton("Remove it"))
            resolve = (int)i, keep = false;
        ImGui::PopID();
    }
    ImGui::Separator();
    if (resolve >= 0)
        Resolve(map, sStale[resolve], keep);
    return true;
}

// tests: "merge list" (each as "[maker] merge: KEY was base now gone"), "merge keep N", "merge take N"
void Merge_TestCommand(const char *cmd, const char *arg)
{
    if (gEd.map < 0)
        return;
    sVersion = ~0u;
    Refresh(gEd.map);
    char what[16] = "";
    int n = -1;
    sscanf(arg, "%15s %d", what, &n);
    if (!strcmp(what, "list"))
    {
        printf("[maker] merge: %d\n", (int)sStale.size());
        for (const MakerStale &s : sStale)
            printf("[maker] merge: %s %ld %ld %ld %d\n", s.key, s.was, s.base, s.now, s.gone);
    }
    else if ((!strcmp(what, "keep") || !strcmp(what, "take")) && n >= 0 && n < (int)sStale.size())
        Resolve(gEd.map, sStale[n], !strcmp(what, "keep"));
    (void)cmd;
}

} // namespace maker
