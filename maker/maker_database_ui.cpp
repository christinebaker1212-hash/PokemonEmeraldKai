// Moemon Maker, M3a: the DATABASE window (Tools > Database). Tabs for POKeMON, MOVES and ITEMS: any value the game has
// for them (stats, types, abilities, learnset...; power, type, accuracy...; name, price, words), kept as
// MoemonMaker/database.txt (src/maker/maker_database.c's format: each line with the value it replaced) and sent to the
// game as it changes, so F5 plays the edited values at once. Ctrl+S (or the window's Save) writes the file.
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include "maker_ui.h"

namespace maker {

// One edited line: what it is now and what it replaced, both as the file writes them
struct LineEdit
{
    std::string now, was;
};

struct EntryEdit
{
    std::map<std::string, LineEdit> lines;   // by key (stats, types, name, learnset...)
};

static std::map<int, EntryEdit> sDb[3];   // by MAKER_DB_* then id
static bool sOpen, sLoaded, sDirty;
static int sTab, sSel[3] = {1, 1, 1};
static char sFilter[3][64];
static const char *const kHead[3] = {"species", "move", "item"};
static const int kTable[3] = {MAKER_NAMES_SPECIES, MAKER_NAMES_MOVE, MAKER_NAMES_ITEM};
static const char *const kPrefix[3] = {"SPECIES_", "MOVE_", "ITEM_"};

// M3b: the user's new POKeMON (src/maker/maker_database.c `new species ID "NAME" base=SPECIES_X`): ids from 1051,
// handed out once and never again (the file keeps the next one), so a saved POKeMON keeps being the same one
struct NewSpecies
{
    int base;
    std::string name;
};
static std::map<int, NewSpecies> sNewK[3];   // by MAKER_DB_* then id (POKeMON, moves, items)
static const int kNewFirst[3] = {MAKER_NEW_SPECIES_FIRST, MAKER_NEW_MOVE_FIRST, MAKER_NEW_ITEM_FIRST};
static const int kNewLast[3] = {MAKER_NEW_SPECIES_LAST, MAKER_NEW_MOVE_LAST, MAKER_NEW_ITEM_LAST};
static const char *const kNewWord[3] = {"species", "move", "item"};
static const char *const kNewWhat[3] = {"POKeMON", "move", "item"};
static int sNextNewK[3] = {MAKER_NEW_SPECIES_FIRST, MAKER_NEW_MOVE_FIRST, MAKER_NEW_ITEM_FIRST};
#define sNew sNewK[0]
static bool IsNew(int k, int id) { return sNewK[k].count(id) != 0; }

static const char *Name(int table, int v)
{
    const char *n = MakerApi_NameOf(table, (unsigned)v);
    return n && n[0] ? n : "?";
}

static unsigned Value(int table, const char *s)
{
    unsigned v = 0;
    if (table < 0 || !MakerApi_ValueOf(table, s, &v))
        v = strtoul(s, NULL, 0);
    return v;
}

static std::string Join(const int *v, int n, int table)
{
    std::string out;
    char num[24];
    for (int i = 0; i < n; i++)
    {
        if (i)
            out += " ";
        if (table >= 0 && MakerApi_NameOf(table, (unsigned)v[i])[0])
            out += MakerApi_NameOf(table, (unsigned)v[i]);
        else
            snprintf(num, sizeof(num), "%d", v[i]), out += num;
    }
    return out;
}

static std::string Quote(const std::string &s)
{
    std::string out = "\"";
    for (char c : s)
    {
        if (c == '"' || c == '\\')
            out += '\\';
        out += c;
    }
    return out + "\"";
}

static std::string DatabaseText()
{
    std::string out = "# Moemon Maker: the user's database edits (written by the editor; was= is the game's own value)\n";
    for (int k = 0; k < 3; k++)
    {
        if (!sNewK[k].empty() || sNextNewK[k] != kNewFirst[k])
            out += std::string("# next new ") + kNewWord[k] + " " + std::to_string(sNextNewK[k]) + "\n";
        for (auto &kv : sNewK[k])
        {
            out += std::string("new ") + kNewWord[k] + " " + std::to_string(kv.first) + " " + Quote(kv.second.name) +
                   " base=" + Name(kTable[k], kv.second.base) + "\n";
            if (sDb[k].count(kv.first))
                for (auto &l : sDb[k][kv.first].lines)
                    out += "  " + l.first + " " + l.second.now + "\n";   // (nothing of the game's to clash with: no was=)
        }
    }
    for (int k = 0; k < 3; k++)
        for (auto &kv : sDb[k])
        {
            if (kv.second.lines.empty() || IsNew(k, kv.first))
                continue;
            out += std::string(kHead[k]) + " " + Name(kTable[k], kv.first) + "\n";
            for (auto &l : kv.second.lines)
                out += "  " + l.first + " " + l.second.now + " was=" + l.second.was + "\n";
        }
    return out;
}

// Reads MoemonMaker/database.txt back into the editor
static void Load()
{
    sLoaded = true;
    for (auto &d : sDb)
        d.clear();
    for (int k = 0; k < 3; k++)
        sNewK[k].clear(), sNextNewK[k] = kNewFirst[k];
    FILE *f = fopen((EditsDir() + "/database.txt").c_str(), "rb");
    if (!f)
        return;
    char line[2600];
    EntryEdit *cur = nullptr;
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;
        char a[128] = "";
        if (line[0] != ' ')
        {
            cur = nullptr;
            int nid, next, nk = -1;
            for (int k = 0; k < 3; k++)
            {
                std::string w = std::string("# next new ") + kNewWord[k] + " %d", h = std::string("new ") + kNewWord[k] + " %d";
                if (sscanf(line, w.c_str(), &next) == 1)
                    sNextNewK[k] = std::max(sNextNewK[k], next);
                if (sscanf(line, h.c_str(), &nid) == 1)
                    nk = k;
            }
            if (nk >= 0)
            {
                NewSpecies n{0, "NEW"};
                if (const char *q = strchr(line, '"'))
                {
                    const char *e = strchr(q + 1, '"');
                    n.name = e ? std::string(q + 1, e - q - 1) : std::string(q + 1);
                }
                if (const char *b = strstr(line, "base="))
                {
                    char bs[96] = "";
                    sscanf(b + 5, "%95s", bs);
                    n.base = (int)Value(kTable[nk], bs);
                }
                sNewK[nk][nid] = n;
                sNextNewK[nk] = std::max(sNextNewK[nk], nid + 1);
                cur = &sDb[nk][nid];
                continue;
            }
            for (int k = 0; k < 3; k++)
            {
                std::string head = std::string(kHead[k]) + " ";
                if (!strncmp(line, head.c_str(), head.size()) && sscanf(line + head.size(), "%127s", a) == 1)
                    cur = &sDb[k][(int)Value(kTable[k], a)];
            }
            continue;
        }
        if (!cur || sscanf(line, " %127s", a) != 1)
            continue;
        const char *rest = strstr(line, a) + strlen(a);
        while (*rest == ' ')
            rest++;
        const char *was = strstr(rest, " was=");
        LineEdit e;
        e.now = was ? std::string(rest, was - rest) : std::string(rest);
        e.was = was ? std::string(was + 5) : "";
        cur->lines[a] = e;
    }
    fclose(f);
    MakerApi_SetDatabase(DatabaseText().c_str());
}

static void Changed()
{
    sDirty = true;
    MakerApi_SetDatabase(DatabaseText().c_str());
}

int Database_Save()
{
    if (!sDirty)
        return 0;
    std::string path = EditsDir() + "/database.txt";
    bool any = false;
    for (auto &d : sDb)
        for (auto &kv : d)
            any |= !kv.second.lines.empty();
    if (!any)
        remove(path.c_str());
    else if (FILE *f = fopen(path.c_str(), "wb"))
    {
        std::string text = DatabaseText();
        fwrite(text.data(), 1, text.size(), f);
        fclose(f);
    }
    sDirty = false;
    return 1;
}

bool Database_Unsaved()
{
    return sDirty;
}

static bool Picker(const char *id, int table, int *value)
{
    bool picked = false;   // (the caller sets the width)
    if (ImGui::BeginCombo(id, Name(table, *value), ImGuiComboFlags_HeightLarge))
    {
        static char filter[64];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputTextWithHint("##f", "Search...", filter, sizeof(filter));
        std::string f = filter;
        std::transform(f.begin(), f.end(), f.begin(), ::toupper);
        int n = MakerApi_NameCount(table);
        for (int i = 0; i < n; i++)
        {
            unsigned v;
            const char *nm = MakerApi_NameAt(table, i, &v);
            if (!f.empty() && !strstr(nm, f.c_str()))
                continue;
            ImGui::PushID(i);
            if (ImGui::Selectable(nm, (int)v == *value))
                *value = (int)v, picked = true;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return picked;
}

// The values a line shows: the edit's when there is one, else the game's own
static void Current(EntryEdit *e, const char *key, int *v, int n, int table, const int *base)
{
    auto it = e ? e->lines.find(key) : std::map<std::string, LineEdit>::iterator();
    if (!e || it == e->lines.end())
    {
        memcpy(v, base, sizeof(int) * n);
        return;
    }
    char buf[400];
    snprintf(buf, sizeof(buf), "%s", it->second.now.c_str());
    int i = 0;
    for (char *tok = strtok(buf, " "); tok && i < n; tok = strtok(NULL, " "))
        v[i++] = (int)Value(table, tok);
    for (; i < n; i++)
        v[i] = base[i];
}

// A row of n numbers (or names from a table); true when it changed (the edit is made or dropped)
static bool Row(int kind, int id, const char *key, const char *label, const char *const *sub, int n, int table,
                const int *base, int lo, int hi)
{
    EntryEdit *e = sDb[kind].count(id) ? &sDb[kind][id] : nullptr;
    int v[8];
    Current(e, key, v, n, table, base);
    bool edited = e && e->lines.count(key), ch = false;
    ImGui::PushID(key);
    ImGui::AlignTextToFramePadding();
    if (edited)
        ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.1f, 1), "%s", label);
    else
        ImGui::TextUnformatted(label);
    float w = (ImGui::GetContentRegionAvail().x - 130 * gEd.uiScale) / n - 6;
    for (int i = 0; i < n; i++)
    {
        ImGui::SameLine(i == 0 ? 130 * gEd.uiScale : 0);
        ImGui::PushID(i);
        ImGui::BeginGroup();
        if (sub)
            ImGui::TextDisabled("%s", sub[i]);
        ImGui::SetNextItemWidth(std::max(60.0f, w));
        if (table >= 0)
            ch |= Picker("##v", table, &v[i]);
        else
        {
            ch |= ImGui::InputInt("##v", &v[i], 0);
            v[i] = std::max(lo, std::min(v[i], hi));
        }
        ImGui::EndGroup();
        ImGui::PopID();
    }
    if (edited)
    {
        ImGui::SameLine();
        if (ImGui::SmallButton("x"))
        {
            e->lines.erase(key);
            ch = false;
            Changed();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Back to the game's own: %s", Join(base, n, table).c_str());
    }
    ImGui::PopID();
    if (ch)
    {
        LineEdit &l = sDb[kind][id].lines[key];
        l.now = Join(v, n, table);
        if (l.was.empty())
            l.was = Join(base, n, table);
        if (l.now == l.was)
            sDb[kind][id].lines.erase(key);
        Changed();
        return true;
    }
    return false;
}

// A quoted text line (a name or an item's words); was= the game's text (names) or its hash (words)
static void TextRow(int kind, int id, const char *key, const char *label, const char *base, unsigned hash, int maxLen,
                    bool multiline)
{
    EntryEdit *e = sDb[kind].count(id) ? &sDb[kind][id] : nullptr;
    auto it = e ? e->lines.find(key) : std::map<std::string, LineEdit>::iterator();
    bool edited = e && it != e->lines.end();
    char buf[400];
    if (edited)
    {
        // (the file's quoted form, back to the text)
        std::string s = it->second.now, out;
        for (size_t i = 1; i + 1 < s.size(); i++)
        {
            if (s[i] == '\\' && i + 2 < s.size())
                i++;
            out += s[i];
        }
        snprintf(buf, sizeof(buf), "%s", out.c_str());
    }
    else
        snprintf(buf, sizeof(buf), "%s", base);
    ImGui::PushID(key);
    ImGui::AlignTextToFramePadding();
    if (edited)
        ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.1f, 1), "%s", label);
    else
        ImGui::TextUnformatted(label);
    ImGui::SameLine(130 * gEd.uiScale);
    bool ch;
    ImGui::SetNextItemWidth(-30 * gEd.uiScale);
    if (multiline)
        ch = ImGui::InputTextMultiline("##t", buf, std::min((int)sizeof(buf), maxLen + 1), ImVec2(-30 * gEd.uiScale, 60 * gEd.uiScale));
    else
        ch = ImGui::InputText("##t", buf, std::min((int)sizeof(buf), maxLen + 1));
    if (edited)
    {
        ImGui::SameLine();
        if (ImGui::SmallButton("x"))
        {
            e->lines.erase(key);
            Changed();
            ch = false;
        }
    }
    ImGui::PopID();
    if (ch)
    {
        for (char *p = buf; *p; p++)
            if (*p == '\n' || *p == '\r')
                *p = ' ';
        LineEdit &l = sDb[kind][id].lines[key];
        l.now = Quote(buf);
        char h[24];
        snprintf(h, sizeof(h), "0x%08X", hash);
        if (l.was.empty())
            l.was = hash ? std::string(h) : Quote(base);
        if (!strcmp(buf, base))
            sDb[kind][id].lines.erase(key);
        Changed();
    }
}

static void LearnsetEditor(int id, const MakerSpeciesInfo &base)
{
    EntryEdit *e = sDb[0].count(id) ? &sDb[0][id] : nullptr;
    auto it = e ? e->lines.find("learnset") : std::map<std::string, LineEdit>::iterator();
    bool edited = e && it != e->lines.end();
    std::vector<std::pair<int, int>> list;   // (level, move)
    if (edited)
    {
        char buf[2600];
        snprintf(buf, sizeof(buf), "%s", it->second.now.c_str());
        for (char *tok = strtok(buf, " "); tok; tok = strtok(NULL, " "))
            if (char *c = strchr(tok, ':'))
                *c = 0, list.push_back({atoi(tok), (int)Value(MAKER_NAMES_MOVE, c + 1)});
    }
    else
        for (int i = 0; i < base.learnCount; i++)
            list.push_back({base.learnLevel[i], base.learnMove[i]});
    ImGui::SeparatorText(edited ? "LEARNSET (edited)" : "LEARNSET");
    bool ch = false;
    int remove = -1;
    if (ImGui::BeginTable("learn", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY,
                          ImVec2(0, 220 * gEd.uiScale)))
    {
        ImGui::TableSetupColumn("Lv", ImGuiTableColumnFlags_WidthFixed, 60 * gEd.uiScale);
        ImGui::TableSetupColumn("Move");
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 30 * gEd.uiScale);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < list.size(); i++)
        {
            ImGui::PushID((int)i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputInt("##lv", &list[i].first, 0))
                list[i].first = std::max(0, std::min(list[i].first, 100)), ch = true;
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ch |= Picker("##mv", MAKER_NAMES_MOVE, &list[i].second);
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("-"))
                remove = (int)i;
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (remove >= 0)
        list.erase(list.begin() + remove), ch = true;
    if (list.size() < MAKER_LEARNSET_MAX && ImGui::Button("Add a move"))
        list.push_back({list.empty() ? 1 : list.back().first, list.empty() ? 1 : list.back().second}), ch = true;
    if (edited)
    {
        ImGui::SameLine();
        if (ImGui::Button("Learnset back to the game's own"))
        {
            e->lines.erase("learnset");
            Changed();
            return;
        }
    }
    ImGui::TextDisabled("Level 0: learned when it evolves into this POKeMON. The list is kept in level order.");
    if (ch)
    {
        std::stable_sort(list.begin(), list.end(), [](const std::pair<int, int> &a, const std::pair<int, int> &b) { return a.first < b.first; });
        std::string now;
        char item[96];
        for (auto &p : list)
        {
            snprintf(item, sizeof(item), "%s%d:%s", now.empty() ? "" : " ", p.first, Name(MAKER_NAMES_MOVE, p.second));
            now += item;
        }
        LineEdit &l = sDb[0][id].lines["learnset"];
        l.now = now;
        char h[24];
        snprintf(h, sizeof(h), "0x%08X", base.learnHash);
        if (l.was.empty())
            l.was = h;
        Changed();
    }
}

static const char *const kStats[6] = {"HP", "ATTACK", "DEFENSE", "SPEED", "SP. ATK", "SP. DEF"};
static const char *const kTwo[2] = {"1st", "2nd"};
static const char *const kItems2[2] = {"Common (50%)", "Rare (5%)"};

// PICTURES (M3: src/maker/maker_art.c): the front, back and shiny as the game shows them now, and the user's PNGs
static SDL_Texture *sPicTex[3];
static int sPicSpecies = -1, sPicStamp = -1, sArtStamp;
static char sPngPath[600];

static void MakeDir(const char *path)
{
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static void PicturesPanel(int id, const char *name)
{
    static const char *const kinds[3] = {"front", "back", "shiny"};
    ImGui::SeparatorText("PICTURES");
    if (sPicSpecies != id || sPicStamp != sArtStamp)
    {
        unsigned char rgba[64 * 64 * 4];
        for (int k = 0; k < 3; k++)
        {
            if (!sPicTex[k])
            {
                sPicTex[k] = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 64, 64);
                SDL_SetTextureBlendMode(sPicTex[k], SDL_BLENDMODE_BLEND);
            }
            if (sPicTex[k] && MakerApi_SpeciesPicture(id, k == 1, k == 2, rgba))
                SDL_UpdateTexture(sPicTex[k], NULL, rgba, 64 * 4);
        }
        sPicSpecies = id, sPicStamp = sArtStamp;
    }
    float sz = 128 * gEd.uiScale;
    for (int k = 0; k < 3; k++)
    {
        if (k)
            ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::TextDisabled("%s", k == 0 ? "Front" : k == 1 ? "Back" : "Shiny");
        if (sPicTex[k])
            ImGui::Image((ImTextureID)(intptr_t)sPicTex[k], ImVec2(sz, sz));
        std::string own = EditsDir() + "/art/" + name + "_" + kinds[k] + ".png";
        if (FILE *f = fopen(own.c_str(), "rb"))
        {
            fclose(f);
            ImGui::PushID(k);
            if (ImGui::SmallButton("Back to the game's own"))
            {
                remove(own.c_str());
                MakerApi_ReloadArt();
                sArtStamp++;
            }
            ImGui::PopID();
        }
        ImGui::EndGroup();
    }
    ImGui::TextDisabled("A PNG of your own (64x64; the front may be 64x128 for its two frames; clear or top-left color = background):");
    ImGui::SetNextItemWidth(-200 * gEd.uiScale);
    ImGui::InputTextWithHint("##png", "Path to a .png file", sPngPath, sizeof(sPngPath));
    for (int k = 0; k < 3; k++)
    {
        ImGui::SameLine();
        char label[32];
        snprintf(label, sizeof(label), "As %s", kinds[k]);
        if (ImGui::Button(label) && sPngPath[0])
        {
            FILE *in = fopen(sPngPath, "rb");
            if (!in)
            {
                Message("Can't open %s", sPngPath);
                continue;
            }
            std::string dir = EditsDir() + "/art";
            MakeDir(EditsDir().c_str());
            MakeDir(dir.c_str());
            std::string out = dir + "/" + name + "_" + kinds[k] + ".png";
            FILE *o = fopen(out.c_str(), "wb");
            char buf[8192];
            size_t n;
            while (o && (n = fread(buf, 1, sizeof(buf), in)) > 0)
                fwrite(buf, 1, n, o);
            if (o)
                fclose(o);
            fclose(in);
            MakerApi_ReloadArt();
            sArtStamp++;
            Message("%s's %s picture is now %s", name, kinds[k], sPngPath);
        }
    }
    ImGui::TextDisabled("Fit to the game's 15 colors + clear, shared by the front and back. They're in MoemonMaker/art/.");
}

static int sRemoveNew = -1, sRemoveNewK;

// M3b: a new entry's name and the entry of the game's it starts from; true if it's a new one
static bool NewHeader(int k, int id, int maxLen)
{
    if (!IsNew(k, id))
        return false;
    NewSpecies &n = sNewK[k][id];
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", n.name.c_str());
    ImGui::PushID("newhead");
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Name");
    ImGui::SameLine(130 * gEd.uiScale);
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    if (ImGui::InputText("##newname", buf, maxLen + 1))
    {
        for (char *p = buf; *p; p++)
            *p = (char)toupper((unsigned char)*p);
        n.name = buf;
        Changed();
    }
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Based on");
    ImGui::SameLine(130 * gEd.uiScale);
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    int base = n.base;
    if (Picker("##newbase", kTable[k], &base) && base > 0 && base < kNewFirst[k])
        n.base = base, Changed();
    ImGui::SameLine();
    if (ImGui::Button("Remove it"))
        sRemoveNew = id, sRemoveNewK = k;
    ImGui::TextDisabled("New %s #%d: its number never changes. Until a line below changes it, it's its base in everything.",
                        kNewWhat[k], id);
    ImGui::Separator();
    ImGui::PopID();
    return true;
}

static void SpeciesPage(int id)
{
    MakerSpeciesInfo b;
    bool isNew = IsNew(0, id);
    if (!MakerApi_SpeciesInfo(isNew ? sNew[id].base : id, &b))
    {
        ImGui::TextDisabled("Not a POKeMON the game has.");
        return;
    }
    int total = 0;
    for (int i = 0; i < 6; i++)
        total += b.stats[i];
    if (!NewHeader(0, id, 10))   // (a new one: its pictures, icon, cry, TMs and walk are its base's until changed)
        TextRow(0, id, "name", "Name", b.name, 0, 10, false);
    Row(0, id, "stats", "Base stats", kStats, 6, -1, b.stats, 1, 255);
    {
        int v[6];
        Current(sDb[0].count(id) ? &sDb[0][id] : nullptr, "stats", v, 6, -1, b.stats);
        int now = 0;
        for (int i = 0; i < 6; i++)
            now += v[i];
        ImGui::TextDisabled("Total %d (the game's own: %d)", now, total);
    }
    Row(0, id, "types", "Types", kTwo, 2, MAKER_NAMES_TYPE, b.types, 0, 255);
    Row(0, id, "abilities", "Abilities", kTwo, 2, MAKER_NAMES_ABILITY, b.abilities, 0, 65535);
    Row(0, id, "catch", "Catch rate", NULL, 1, -1, &b.catchRate, 0, 255);
    Row(0, id, "exp", "EXP yield", NULL, 1, -1, &b.expYield, 0, 255);
    Row(0, id, "evs", "EVs given", kStats, 6, -1, b.evs, 0, 3);
    Row(0, id, "items", "Held in the wild", kItems2, 2, MAKER_NAMES_ITEM, b.items, 0, 65535);
    Row(0, id, "gender", "Gender (0 male..254 female, 255 none)", NULL, 1, -1, &b.gender, 0, 255);
    Row(0, id, "eggcycles", "Egg cycles", NULL, 1, -1, &b.eggCycles, 0, 255);
    Row(0, id, "friendship", "Friendship", NULL, 1, -1, &b.friendship, 0, 255);
    Row(0, id, "growth", "Growth rate (0-5)", NULL, 1, -1, &b.growth, 0, 5);
    Row(0, id, "egggroups", "Egg groups", kTwo, 2, -1, b.eggGroups, 0, 15);
    LearnsetEditor(id, b);
    PicturesPanel(id, isNew ? ("NEW_" + std::to_string(id)).c_str() : Name(MAKER_NAMES_SPECIES, id));
}

// M3b: a new entry, starting as BASE (one of the game's), at the next id never used
static int MakeNew(int k, int base, const char *name)
{
    if (sNextNewK[k] > kNewLast[k])
    {
        Message("All %d new %s numbers are used", kNewLast[k] - kNewFirst[k] + 1, kNewWhat[k]);
        return 0;
    }
    int id = sNextNewK[k]++;
    sNewK[k][id] = NewSpecies{base, name};
    Changed();
    return id;
}

static void MovePage(int id)
{
    MakerMoveInfo b;
    if (!MakerApi_MoveInfo(IsNew(1, id) ? sNewK[1][id].base : id, &b))
    {
        ImGui::TextDisabled("Not a move the game has.");
        return;
    }
    if (!NewHeader(1, id, 12))   // (a new one: its animation, words and contest data are its base's)
        TextRow(1, id, "name", "Name", b.name, 0, 12, false);
    Row(1, id, "power", "Power", NULL, 1, -1, &b.power, 0, 255);
    Row(1, id, "type", "Type", NULL, 1, MAKER_NAMES_TYPE, &b.type, 0, 255);
    Row(1, id, "accuracy", "Accuracy (0: never misses)", NULL, 1, -1, &b.accuracy, 0, 100);
    Row(1, id, "pp", "PP", NULL, 1, -1, &b.pp, 1, 64);
    Row(1, id, "category", "Category (0 physical, 1 special, 2 status)", NULL, 1, -1, &b.category, 0, 2);
    Row(1, id, "priority", "Priority", NULL, 1, -1, &b.priority, -7, 7);
    Row(1, id, "chance", "Effect chance (%)", NULL, 1, -1, &b.chance, 0, 100);
    Row(1, id, "effect", "Effect (the game's own effects)", NULL, 1, -1, &b.effect, 0, 65535);
    Row(1, id, "target", "Target", NULL, 1, -1, &b.target, 0, 255);
    Row(1, id, "flags", "Flags", NULL, 1, -1, &b.flags, 0, 255);
    ImGui::TextDisabled("Effects are the game's own (numbers from include/constants/battle_move_effects.h); new ones come with Lua hooks.");
}

static void ItemPage(int id)
{
    MakerItemInfo b;
    if (!MakerApi_ItemInfo(IsNew(2, id) ? sNewK[2][id].base : id, &b))
    {
        ImGui::TextDisabled("Not an item the game has.");
        return;
    }
    if (!NewHeader(2, id, 13))   // (a new one: its icon and what it does are its base's)
        TextRow(2, id, "name", "Name", b.name, 0, 13, false);
    Row(2, id, "price", "Price", NULL, 1, -1, &b.price, 0, 65535);
    Row(2, id, "hold", "Hold effect", NULL, 1, -1, &b.holdEffect, 0, 255);
    Row(2, id, "holdparam", "Hold effect value", NULL, 1, -1, &b.holdParam, 0, 255);
    TextRow(2, id, "description", "Words", b.description, b.descriptionHash ? b.descriptionHash : 1, 300, true);
    ImGui::TextDisabled("The words wrap to each window as the game wraps them; no line breaks needed.");
}

void DatabaseWindow()
{
    if (!sOpen)
        return;
    if (!sLoaded)
        Load();
    ImGui::SetNextWindowSize(ImVec2(980 * gEd.uiScale, 640 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("DATABASE", &sOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::BeginTabBar("dbtabs"))
    {
        const char *const tabs[3] = {"POKeMON", "MOVES", "ITEMS"};
        for (int k = 0; k < 3; k++)
            if (ImGui::BeginTabItem(tabs[k]))
                sTab = k, ImGui::EndTabItem();
        ImGui::EndTabBar();
    }
    int k = sTab;
    ImGui::BeginChild("list", ImVec2(260 * gEd.uiScale, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##df", "Find...", sFilter[k], sizeof(sFilter[k]));
    std::string f = sFilter[k];
    std::transform(f.begin(), f.end(), f.begin(), ::toupper);
    int n = MakerApi_NameCount(kTable[k]), count = MakerApi_DatabaseCount(k);
    size_t pre = strlen(kPrefix[k]);
    {
        // M3b: the new ones first, by the names the user gave them
        std::string label = std::string("+ New ") + kNewWhat[k];
        if (ImGui::Button(label.c_str(), ImVec2(-FLT_MIN, 0)))
        {
            int base = sSel[k] > 0 && sSel[k] < kNewFirst[k] ? sSel[k] : (IsNew(k, sSel[k]) ? sNewK[k][sSel[k]].base : 1);
            if (int id = MakeNew(k, base, k == 1 ? "NEW MOVE" : k == 2 ? "NEW ITEM" : "NEW"))
                sSel[k] = id;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("A new %s, starting as the selected one (or the one it's based on)", kNewWhat[k]);
        for (auto &kv : sNewK[k])
        {
            if (!f.empty() && kv.second.name.find(f) == std::string::npos)
                continue;
            ImGui::PushID(kv.first);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.75f, 0.35f, 1));
            if (ImGui::Selectable((kv.second.name + "  (new)").c_str(), sSel[k] == kv.first))
                sSel[k] = kv.first;
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
        if (!sNewK[k].empty())
            ImGui::Separator();
    }
    for (int i = 0; i < n; i++)
    {
        unsigned v;
        const char *nm = MakerApi_NameAt(kTable[k], i, &v);
        if (v == 0 || (int)v >= count || IsNew(k, (int)v) || ((int)v >= kNewFirst[k] && (int)v <= kNewLast[k])
         || (!f.empty() && !strstr(nm, f.c_str())))
            continue;
        ImGui::PushID(i);
        bool edited = sDb[k].count((int)v) && !sDb[k][(int)v].lines.empty();
        if (edited)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.6f, 0.1f, 1));
        if (ImGui::Selectable(strlen(nm) > pre ? nm + pre : nm, sSel[k] == (int)v))
            sSel[k] = (int)v;
        if (edited)
            ImGui::PopStyleColor();
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("detail", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    ImGui::PushFont(gEd.fontBold);
    ImGui::Text("%s", IsNew(k, sSel[k]) ? sNewK[k][sSel[k]].name.c_str() : Name(kTable[k], sSel[k]));
    ImGui::PopFont();
    ImGui::TextDisabled("Edited values are orange; x puts one back to the game's own.");
    ImGui::Separator();
    if (k == 0)
        SpeciesPage(sSel[k]);
    else if (k == 1)
        MovePage(sSel[k]);
    else
        ItemPage(sSel[k]);
    ImGui::EndChild();
    if (MakerApi_DatabaseConflicts())
    {
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "%d edit%s left out: the game changed the value since (the game's own plays until you edit it again).",
                           MakerApi_DatabaseConflicts(), MakerApi_DatabaseConflicts() == 1 ? "" : "s");
        ImGui::SameLine();
    }
    if (ImGui::Button("Save (Ctrl+S)") && Database_Save())
        Message("Saved the database");
    ImGui::End();
    if (sRemoveNew >= 0)
    {
        // (its number isn't given out again: a saved one becomes the "?" until it's made again)
        sNewK[sRemoveNewK].erase(sRemoveNew);
        sDb[sRemoveNewK].erase(sRemoveNew);
        sSel[sRemoveNewK] = 1;
        sRemoveNew = -1;
        Changed();
    }
}

void OpenDatabase()
{
    sOpen = true;
}

// Tests: "maker db KIND NAME KEY VALUES..." edits one line (as the window does: was= the game's own) and saves;
// "maker database" opens the window
void Database_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (!sLoaded)
        Load();
    if (c == "database")
    {
        sOpen = true;
        if (arg[0])
        {
            unsigned v;
            if (MakerApi_ValueOf(MAKER_NAMES_SPECIES, arg, &v))
                sTab = 0, sSel[0] = (int)v;
        }
        return;
    }
    if (c == "newspecies" || c == "newmove" || c == "newitem")
    {
        // newspecies / newmove / newitem NAME BASE: a new one (saved)
        int k = c == "newmove" ? 1 : c == "newitem" ? 2 : 0;
        char nm[32] = "", bs[96] = "";
        if (sscanf(arg, "%31s %95s", nm, bs) == 2)
        {
            int id = MakeNew(k, (int)Value(kTable[k], bs), nm);
            printf("[maker] new %s %s %d\n", kNewWord[k], nm, id);
            sTab = k, sSel[k] = id;
            Database_Save();
        }
        return;
    }
    if (c == "pic")
    {
        // pic SPECIES: the front picture's middle pixel and a corner as the game shows them now
        unsigned v;
        unsigned char rgba[64 * 64 * 4];
        if (MakerApi_ValueOf(MAKER_NAMES_SPECIES, arg, &v) && MakerApi_SpeciesPicture((int)v, 0, 0, rgba))
        {
            const unsigned char *m = rgba + (32 * 64 + 32) * 4;
            printf("[maker] picture %s middle %d,%d,%d,%d corner alpha %d\n", arg, m[0], m[1], m[2], m[3], rgba[3]);
        }
        return;
    }
    char kind[32] = "", name[96] = "", key[32] = "";
    int used = 0;
    if (sscanf(arg, "%31s %95s %31s %n", kind, name, key, &used) < 3)
        return;
    int k = !strcmp(kind, "move") ? 1 : !strcmp(kind, "item") ? 2 : 0;
    int id = (int)Value(kTable[k], name);
    std::string now = arg + used, was;
    char buf[64];
    if (k == 0)
    {
        MakerSpeciesInfo b;
        if (!MakerApi_SpeciesInfo(id, &b))
            return;
        if (!strcmp(key, "stats")) was = Join(b.stats, 6, -1);
        else if (!strcmp(key, "types")) was = Join(b.types, 2, MAKER_NAMES_TYPE);
        else if (!strcmp(key, "abilities")) was = Join(b.abilities, 2, MAKER_NAMES_ABILITY);
        else if (!strcmp(key, "name")) was = Quote(b.name);
        else if (!strcmp(key, "learnset")) snprintf(buf, sizeof(buf), "0x%08X", b.learnHash), was = buf;
    }
    else if (k == 1)
    {
        MakerMoveInfo b;
        if (!MakerApi_MoveInfo(id, &b))
            return;
        if (!strcmp(key, "power")) was = std::to_string(b.power);
        else if (!strcmp(key, "type")) was = Name(MAKER_NAMES_TYPE, b.type);
        else if (!strcmp(key, "accuracy")) was = std::to_string(b.accuracy);
        else if (!strcmp(key, "pp")) was = std::to_string(b.pp);
        else if (!strcmp(key, "name")) was = Quote(b.name);
    }
    else
    {
        MakerItemInfo b;
        if (!MakerApi_ItemInfo(id, &b))
            return;
        if (!strcmp(key, "price")) was = std::to_string(b.price);
        else if (!strcmp(key, "name")) was = Quote(b.name);
        else if (!strcmp(key, "description")) snprintf(buf, sizeof(buf), "0x%08X", b.descriptionHash), was = buf;
    }
    if (was.empty())
        return;
    sDb[k][id].lines[key] = LineEdit{now, was};
    sTab = k, sSel[k] = id;
    Changed();
    Database_Save();
}

} // namespace maker
