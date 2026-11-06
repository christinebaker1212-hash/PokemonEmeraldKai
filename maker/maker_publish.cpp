// Moemon Maker: the PUBLISH window (docs/MOEMON_MAKER.md decisions 8, 25, 26). The checker first: errors (what would break
// the game for players) stop Publish until fixed, warnings are listed. Then Publish: everything saved, then sent as one
// commit through GitHub's API (src/maker/maker_publish.c) with the token pasted here once (moemon_maker.ini, this PC
// only). The publish history lists earlier publishes; "Roll back to this" publishes that state again and puts its files
// in the working copy.
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <set>
#include <string>
#include <vector>
#include "imgui.h"
#include "maker_ui.h"

namespace maker {

static char sTokenEdit[256];
static char sMessage[256] = "Moemon Maker: ";
static std::string sResult;
static int sConfirmRollback = -1;
static bool sHistoryAsked;

// ---------------------------------------------------------------------------------------------------------------------
// The checker

struct Issue { bool error; int map; std::string text; };

static int WarpCountOf(int map)
{
    int n = MakerApi_WarpCount(map);
    if (const MapEdits *e = FindEdits(map))
        for (auto &a : e->adds)
            if (a.first.rfind("warp", 0) == 0)
                n++;
    return n;
}

// Where the player can walk from (x, y): collision 0, joined side to side (elevations and ledges set aside: this
// only has to find a spot an edit closed in)
static std::vector<char> Walkable(int map, int w, int h, bool edited)
{
    std::vector<char> out(w * h, 0);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            int b = MakerApi_MapBlock(map, x, y);
            if (b < 0)
                continue;
            if (edited)
                b = CurrentBlock(map, x, y, b);
            out[y * w + x] = ((b >> 10) & 3) == 0;
        }
    return out;
}

// From warp `from`, can the player reach another warp or the map's edge?
static bool CanLeave(const std::vector<char> &walk, int w, int h, const std::vector<std::pair<int, int>> &warps, int from, bool edges)
{
    std::vector<char> seen(w * h, 0);
    std::vector<int> stack;
    int sx = warps[from].first, sy = warps[from].second;
    if (sx < 0 || sy < 0 || sx >= w || sy >= h)
        return true;
    // (the warp tile itself, then its walkable neighbours)
    stack.push_back(sy * w + sx);
    seen[sy * w + sx] = 1;
    while (!stack.empty())
    {
        int at = stack.back(), x = at % w, y = at / w;
        stack.pop_back();
        for (size_t i = 0; i < warps.size(); i++)
            if ((int)i != from && warps[i].first == x && warps[i].second == y)
                return true;
        if (edges && (x == 0 || y == 0 || x == w - 1 || y == h - 1))
            return true;
        static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto &dd : d)
        {
            int nx = x + dd[0], ny = y + dd[1];
            if (nx < 0 || ny < 0 || nx >= w || ny >= h || seen[ny * w + nx])
                continue;
            bool warpThere = false;
            for (auto &wp : warps)
                warpThere |= wp.first == nx && wp.second == ny;
            if (!walk[ny * w + nx] && !warpThere)
                continue;
            seen[ny * w + nx] = 1;
            stack.push_back(ny * w + nx);
        }
    }
    return false;
}

// ---- M2c: the events' checks (docs/MOEMON_MAKER.md decision 25: errors block Publish, warnings don't)

static std::vector<std::string> EvLines(const std::string &body)
{
    std::vector<std::string> out;
    size_t pos = 0;
    while (pos < body.size())
    {
        size_t nl = body.find('\n', pos);
        std::string l = body.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
        size_t a = l.find_first_not_of(" \t");
        out.push_back(a == std::string::npos ? "" : l.substr(a));
        pos = nl == std::string::npos ? body.size() : nl + 1;
    }
    return out;
}

static std::vector<std::string> Quoted(const std::string &l)
{
    std::vector<std::string> out;
    size_t pos = 0;
    while ((pos = l.find('"', pos)) != std::string::npos)
    {
        size_t end = l.find('"', pos + 1);
        if (end == std::string::npos)
            break;
        out.push_back(l.substr(pos + 1, end - pos - 1));
        pos = end + 1;
    }
    return out;
}

// Every flag of the user's own any event anywhere sets (setflag X)
static std::set<std::string> FlagsSet()
{
    std::set<std::string> out;
    for (int m = 0; m < (int)gEd.maps.size(); m++)
        if (const MapEdits *e = FindEdits(m))
            for (auto &ev : e->events)
                for (const std::string &l : EvLines(ev.second.body))
                    if (l.rfind("setflag ", 0) == 0)
                        out.insert(l.substr(8, l.find(' ', 8) == std::string::npos ? std::string::npos : l.find(' ', 8) - 8));
    return out;
}

static void CheckEvents(int m, const MapEdits &e, std::vector<Issue> &out)
{
    static std::set<std::string> sSet;
    static Uint32 sSetAt;
    if (SDL_GetTicks() - sSetAt > 1000 || sSet.empty())
        sSet = FlagsSet(), sSetAt = SDL_GetTicks();
    const MakerMapInfo &info = gEd.maps[m];
    char t[400];
    for (auto &ev : e.events)
    {
        const std::string &name = ev.first;
        std::string src;
        for (const std::string &l : EvLines(ev.second.body))
            src += "  " + l + "\n";
        int kind = ev.second.on.rfind("sign:", 0) == 0 ? 1 : ev.second.on.rfind("trigger:", 0) == 0 ? 2 : ev.second.on == "map" ? 3 : 0;
        const char *problem = MakerApi_CheckEvent(info.name, name.c_str(), kind, src.c_str());
        if (problem && problem[0])
        {
            snprintf(t, sizeof(t), "%s: event %s, %s", info.title, name.c_str(), problem);
            out.push_back(Issue{true, m, t});
        }
        std::vector<std::string> lines = EvLines(ev.second.body);
        for (size_t i = 0; i < lines.size(); i++)
        {
            const std::string &l = lines[i];
            // an autorun page that never turns itself off runs forever
            if (l.rfind("page", 0) == 0 && l.find("trigger=autorun") != std::string::npos)
            {
                bool changes = false;
                for (size_t k = i + 1; k < lines.size() && lines[k].rfind("page", 0) != 0; k++)
                    if (lines[k].rfind("self ", 0) == 0 || lines[k].rfind("setflag ", 0) == 0 || lines[k].rfind("clearflag ", 0) == 0
                     || lines[k].rfind("setvar ", 0) == 0 || lines[k].rfind("addvar ", 0) == 0 || lines[k].rfind("warp ", 0) == 0)
                        changes = true;
                if (!changes || (l.find("self=") == std::string::npos && l.find("flag=") == std::string::npos && l.find("var=") == std::string::npos))
                {
                    snprintf(t, sizeof(t), "%s: event %s has an autorun page that never turns itself off (it would run forever): "
                             "give it a condition and change it inside (self A on, with noself=A on the page).", info.title, name.c_str());
                    out.push_back(Issue{true, m, t});
                }
            }
            // flags of the user's own that nothing sets
            for (const char *key : {"flag=", "if flag ", "if !flag "})
            {
                size_t at = l.find(key);
                if (at == std::string::npos)
                    continue;
                size_t a = at + strlen(key), b = l.find(' ', a);
                std::string flag = l.substr(a, b == std::string::npos ? std::string::npos : b - a);
                if (flag.rfind("FLAG_", 0) != 0 && !flag.empty() && !sSet.count(flag) && l.find("noflag=" + flag) == std::string::npos)
                {
                    snprintf(t, sizeof(t), "%s: event %s asks for the flag %s, but nothing sets it.", info.title, name.c_str(), flag.c_str());
                    out.push_back(Issue{false, m, t});
                }
            }
            // words: letters the font doesn't have, lines too wide for the message box
            for (const std::string &q : Quoted(l))
            {
                for (size_t c = 0; c < q.size(); c++)
                {
                    unsigned char ch = q[c];
                    if (ch >= 0x80 && !(ch == 0xC3 && c + 1 < q.size() && ((unsigned char)q[c + 1] == 0xA9 || (unsigned char)q[c + 1] == 0x89)))
                    {
                        snprintf(t, sizeof(t), "%s: event %s: the game's font has no \"%s\" (only English letters, é and É).", info.title,
                                 name.c_str(), q.substr(c, 2).c_str());
                        out.push_back(Issue{true, m, t});
                        break;
                    }
                    if (ch == 0xC3)
                        c++;
                }
                size_t pos = 0;
                while (pos <= q.size())
                {
                    size_t brk = q.size();
                    for (const char *sep : {"\\n", "\\p", "\\l"})
                    {
                        size_t f = q.find(sep, pos);
                        if (f != std::string::npos && f < brk)
                            brk = f;
                    }
                    std::string part = q.substr(pos, brk - pos);
                    if (MakerApi_TextWidth(part.c_str()) > 208)
                    {
                        snprintf(t, sizeof(t), "%s: event %s: \"%s\" is too wide for the message box (use \\n for a new line).",
                                 info.title, name.c_str(), part.c_str());
                        out.push_back(Issue{false, m, t});
                    }
                    pos = brk + 2;
                }
            }
        }
    }
}

static std::vector<Issue> Check()
{
    std::vector<Issue> out;
    char t[320];
    for (int m = 0; m < (int)gEd.maps.size(); m++)
    {
        const MapEdits *e = FindEdits(m);
        if (!e || e->Empty())
            continue;
        const MakerMapInfo &info = gEd.maps[m];
        int w = info.width, h = info.height;
        // warps: where they lead
        std::vector<std::pair<int, int>> warps, baseWarps;
        for (int i = 0; i < MakerApi_WarpCount(m); i++)
        {
            MakerWarpInfo b;
            MakerApi_WarpInfo(m, i, &b);
            std::string k = std::to_string(i);
            int x = (int)CurrentProp(m, "warp", k, "x", b.x), y = (int)CurrentProp(m, "warp", k, "y", b.y);
            long dest = CurrentProp(m, "warp", k, "dest", ((long)b.destMap << 8) | b.destWarp);
            int dm = (int)(dest >> 8), dw = (int)(dest & 0xFF);
            warps.push_back({x, y});
            baseWarps.push_back({b.x, b.y});
            if ((dest != (((long)b.destMap << 8) | b.destWarp)) && (dm < 0 || dm >= (int)gEd.maps.size() || dw >= WarpCountOf(dm)))
            {
                snprintf(t, sizeof(t), "%s: warp %d leads to a warp that doesn't exist.", info.title, i);
                out.push_back(Issue{true, m, t});
            }
        }
        for (auto &a : e->adds)
        {
            const std::map<std::string, std::string> &v = a.second;
            auto get = [&](const char *k) { auto it = v.find(k); return it == v.end() ? std::string() : it->second; };
            int x = atoi(get("x").c_str()), y = atoi(get("y").c_str());
            if (a.first.rfind("warp", 0) == 0)
            {
                std::string dest = get("dest");
                size_t colon = dest.find(':');
                int dm = MakerApi_FindMap(dest.substr(0, colon).c_str()), dw = colon == std::string::npos ? -1 : atoi(dest.c_str() + colon + 1);
                if (dm < 0 || dw < 0 || dw >= WarpCountOf(dm))
                {
                    snprintf(t, sizeof(t), "%s: the added warp at %d, %d leads to a warp that doesn't exist.", info.title, x, y);
                    out.push_back(Issue{true, m, t});
                }
                warps.push_back({x, y});
                continue;
            }
            if (a.first.rfind("connection", 0) == 0)
            {
                if (MakerApi_FindMap(get("map").c_str()) < 0)
                {
                    snprintf(t, sizeof(t), "%s: a joined map doesn't exist.", info.title);
                    out.push_back(Issue{true, m, t});
                }
                continue;
            }
            if ((get("script") == "itemball" || a.first.rfind("hidden", 0) == 0) && (get("item").empty() || get("item") == "ITEM_NONE"))
            {
                snprintf(t, sizeof(t), "%s: the item at %d, %d holds no item.", info.title, x, y);
                out.push_back(Issue{true, m, t});
            }
            if (x < 0 || y < 0 || x >= w || y >= h)
            {
                snprintf(t, sizeof(t), "%s: something added at %d, %d is outside the map.", info.title, x, y);
                out.push_back(Issue{true, m, t});
            }
            else if (a.first.rfind("person", 0) == 0)
            {
                int b = MakerApi_MapBlock(m, x, y);
                if (b >= 0 && ((CurrentBlock(m, x, y, b) >> 10) & 3))
                {
                    snprintf(t, sizeof(t), "%s: the person added at %d, %d stands on a blocked tile.", info.title, x, y);
                    out.push_back(Issue{false, m, t});
                }
            }
        }
        // a warp the player could leave from before the edits, now closed in
        if (!e->blocks.empty() && w > 0 && h > 0 && w * h < 40000)
        {
            bool edges = MakerApi_ConnectionCount(m) > 0;
            std::vector<char> before = Walkable(m, w, h, false), after = Walkable(m, w, h, true);
            for (size_t i = 0; i < baseWarps.size(); i++)
                if (CanLeave(before, w, h, baseWarps, (int)i, edges) && !CanLeave(after, w, h, warps, (int)i, edges))
                {
                    snprintf(t, sizeof(t), "%s: from warp %d at %d, %d the player can't get out any more (blocked tiles).",
                             info.title, (int)i, warps[i].first, warps[i].second);
                    out.push_back(Issue{true, m, t});
                }
        }
        CheckEvents(m, *e, out);   // (M2c: events)
        if (int n = (m == gEd.map) ? MapView_ProblemCount() : 0)
        {
            snprintf(t, sizeof(t), "%s: %d edits the game changed under (see PROBLEMS): they're left out until you choose.", info.title, n);
            out.push_back(Issue{false, m, t});
        }
    }
    return out;
}

// ---------------------------------------------------------------------------------------------------------------------
// The window

void PublishWindow()
{
    const char *result = nullptr;
    int state = MakerApi_PublishState(&result);
    if (state == MAKER_PUBLISH_DONE || state == MAKER_PUBLISH_FAILED || state == MAKER_PUBLISH_ROLLED_BACK)
    {
        sResult = result ? result : "";
        Message("%s", sResult.c_str());
        if (state == MAKER_PUBLISH_ROLLED_BACK)
        {
            // (the working copy has the old files now: the editor and the game read them again)
            LoadAllEdits();
            MakerApi_ReloadEdits();
            MakerApi_MusicChanged();
            MapView_Opened();
        }
        MakerApi_PublishAcknowledge();
    }
    if (!gEd.publishOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(620 * gEd.uiScale, 560 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("PUBLISH", &gEd.publishOpen, ImGuiWindowFlags_NoDocking))
    {
        ImGui::End();
        return;
    }
    bool working = state == MAKER_PUBLISH_WORKING;
    ImGui::SeparatorText("GITHUB TOKEN");
    if (gEd.githubToken.empty())
    {
        ImGui::TextWrapped("Paste a GitHub token that can write to the game's repository (a fine-grained token with "
                           "\"Contents: read and write\"). It's kept on this PC only.");
        ImGui::SetNextItemWidth(-90 * gEd.uiScale);
        ImGui::InputText("##token", sTokenEdit, sizeof(sTokenEdit), ImGuiInputTextFlags_Password);
        ImGui::SameLine();
        if (ImGui::Button("Keep it") && sTokenEdit[0])
        {
            gEd.githubToken = sTokenEdit;
            memset(sTokenEdit, 0, sizeof(sTokenEdit));
            SaveSettings();
        }
    }
    else
    {
        ImGui::Text("A token is kept on this PC.");
        ImGui::SameLine();
        if (ImGui::SmallButton("Forget it"))
        {
            gEd.githubToken.clear();
            SaveSettings();
        }
    }
    ImGui::SeparatorText("BEFORE PUBLISHING");
    std::vector<Issue> issues = Check();
    int errors = 0;
    for (const Issue &is : issues)
        errors += is.error;
    if (issues.empty())
        ImGui::TextColored(ImVec4(0.1f, 0.55f, 0.2f, 1), "No problems found in %d edited maps.", EditedMapCount());
    for (const Issue &is : issues)
    {
        ImGui::TextColored(is.error ? ImVec4(0.85f, 0.15f, 0.1f, 1) : ImVec4(0.75f, 0.5f, 0.0f, 1), is.error ? "Error" : "Warning");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", is.text.c_str());
    }
    ImGui::SeparatorText("PUBLISH");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##msg", sMessage, sizeof(sMessage));
    bool can = !working && errors == 0 && !gEd.githubToken.empty();
    if (!can)
        ImGui::BeginDisabled();
    if (ImGui::Button("Publish", ImVec2(140, 0)))
    {
        if (AnyUnsaved())
            SaveAll();   // (what's published is what's saved)
        MakerApi_Publish(gEd.githubToken.c_str(), sMessage);
    }
    if (!can)
        ImGui::EndDisabled();
    ImGui::SameLine();
    if (working)
        ImGui::TextDisabled("Working...");
    else if (errors)
        ImGui::TextColored(ImVec4(0.85f, 0.15f, 0.1f, 1), "Fix the errors above first.");
    else if (!sResult.empty())
        ImGui::TextWrapped("%s", sResult.c_str());
    ImGui::SeparatorText("PUBLISH HISTORY");
    if (ImGui::Button("Show the publish history") && !working && !gEd.githubToken.empty())
    {
        MakerApi_PublishHistory(gEd.githubToken.c_str());
        sHistoryAsked = true;
    }
    if (sHistoryAsked && ImGui::BeginTable("hist", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
    {
        for (int i = 0; i < MakerApi_PublishHistoryCount(); i++)
        {
            const char *sha, *date, *msg;
            MakerApi_PublishHistoryItem(i, &sha, &date, &msg);
            ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%.10s", date);
            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s", msg);
            ImGui::TableNextColumn();
            if (i > 0)
            {
                if (sConfirmRollback == i)
                {
                    if (ImGui::SmallButton("Yes, roll back") && !working)
                    {
                        MakerApi_PublishRollback(gEd.githubToken.c_str(), sha);
                        sConfirmRollback = -1;
                    }
                }
                else if (ImGui::SmallButton("Roll back to this"))
                    sConfirmRollback = i;
            }
            else
                ImGui::TextDisabled("(now)");
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

int Publish_ErrorCount()
{
    int n = 0;
    for (const Issue &is : Check())
        n += is.error;
    return n;
}

// Tests: "maker check" (the checker's findings), "maker token T", "maker publish MESSAGE", "maker history",
// "maker rollback N" (the history's Nth)
void Publish_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (c == "check")
        for (const Issue &is : Check())
            printf("[maker] check %s: %s\n", is.error ? "error" : "warning", is.text.c_str());
    else if (c == "token")
        gEd.githubToken = arg;
    else if (c == "publish")
    {
        if (Publish_ErrorCount())
        {
            printf("[maker] publish: refused, the checker found errors\n");
            return;
        }
        if (AnyUnsaved())
            SaveAll();
        MakerApi_Publish(gEd.githubToken.c_str(), arg);
    }
    else if (c == "history")
        MakerApi_PublishHistory(gEd.githubToken.c_str());
    else if (c == "historylist")
        for (int i = 0; i < MakerApi_PublishHistoryCount(); i++)
        {
            const char *sha, *date, *msg;
            MakerApi_PublishHistoryItem(i, &sha, &date, &msg);
            printf("[maker] history %d: %s %s %s\n", i, sha, date, msg);
        }
    else if (c == "rollback")
    {
        const char *sha, *date, *msg;
        if (MakerApi_PublishHistoryItem(atoi(arg), &sha, &date, &msg))
            MakerApi_PublishRollback(gEd.githubToken.c_str(), sha);
    }
}

} // namespace maker
