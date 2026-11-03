// Moemon Maker: the user's edits in the editor (docs/MOEMON_MAKER.md "Why first"): per map, the tiles and the people /
// warps / signs / triggers' properties changed, each with the value it replaced, written as MoemonMaker/base/MAP_X.txt
// (the format: src/maker/maker_overrides.c, which the game reads). Every change is sent to the game at once
// (MakerApi_SetMapEdits), kept in a recovery file within seconds (MoemonMaker/.recovery/), and written by Ctrl+S.
// Undo / redo by whole changes (a stroke of the pencil is one).
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>
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

static std::map<int, MapEdits> sEdits;
static std::map<int, bool> sDirty;          // maps changed since the last Ctrl+S
struct UndoStep { int map; MapEdits before, after; };
static std::vector<UndoStep> sUndo, sRedo;
static int sChangeMap = -1;
static MapEdits sChangeBefore;
static bool sInChange;
static Uint32 sRecoveryDue;                 // when the recovery files are next written (0: nothing to write)

std::string EditsDir()
{
    const char *env = getenv("MOEMON_MAKER_DIR");
    if (env && env[0])
        return env;
    return MakerApi_EditsDir();
}

static std::string MapFile(int map, const char *sub)
{
    return EditsDir() + "/" + sub + "/" + gEd.maps[map].name + ".txt";
}

const MapEdits *FindEdits(int map)
{
    auto it = sEdits.find(map);
    return it == sEdits.end() ? nullptr : &it->second;
}

MapEdits &Edits(int map)
{
    MapEdits &e = sEdits[map];
    if (e.w == 0)
    {
        e.w = gEd.maps[map].width;
        e.h = gEd.maps[map].height;
    }
    return e;
}

int CurrentBlock(int map, int x, int y, int base)
{
    const MapEdits *e = FindEdits(map);
    if (!e)
        return base;
    auto it = e->blocks.find(y * e->w + x);
    if (it == e->blocks.end() || it->second.first != base)
        return base;   // (an edit whose WAS the game no longer has isn't used: Problems shows it)
    return it->second.second;
}

void SetBlock(int map, int x, int y, int base, int now)
{
    MapEdits &e = Edits(map);
    int at = y * e.w + x;
    auto it = e.blocks.find(at);
    if (now == base)
    {
        if (it != e.blocks.end())
            e.blocks.erase(it);
        return;
    }
    if (it != e.blocks.end())
        it->second.second = now;
    else
        e.blocks[at] = std::make_pair(base, now);
}

static std::string PropKey(const char *kind, const std::string &key, const char *prop)
{
    return std::string(kind) + " " + key + " " + prop;
}

long CurrentProp(int map, const char *kind, const std::string &key, const char *prop, long base)
{
    const MapEdits *e = FindEdits(map);
    if (!e)
        return base;
    auto it = e->props.find(PropKey(kind, key, prop));
    if (it == e->props.end() || it->second.was != base)
        return base;
    return it->second.now;
}

void SetProp(int map, const char *kind, const std::string &key, const char *prop, long base, long now)
{
    MapEdits &e = Edits(map);
    std::string k = PropKey(kind, key, prop);
    auto it = e.props.find(k);
    if (now == base)
    {
        if (it != e.props.end())
            e.props.erase(it);
        return;
    }
    if (it != e.props.end())
        it->second.now = now;
    else
        e.props[k] = PropEdit{base, now};
}

// ---------------------------------------------------------------------------------------------------------------------
// The text (MoemonMaker/base/MAP_X.txt): sorted, so the same edits always read the same and git shows only changes

static int PropTable(const std::string &prop)
{
    if (prop == "look") return MAKER_NAMES_GFX;
    if (prop == "movement") return MAKER_NAMES_MOVEMENT;
    if (prop == "trainer") return MAKER_NAMES_TRAINERTYPE;
    if (prop == "flag") return MAKER_NAMES_FLAG;
    return -1;
}

static std::string ValueText(const std::string &prop, long v)
{
    char buf[96];
    int table = PropTable(prop);
    if (prop == "dest")
    {
        int m = (int)(v >> 8);
        snprintf(buf, sizeof(buf), "%s:%ld", m >= 0 && m < (int)gEd.maps.size() ? gEd.maps[m].name : "MAP_UNDEFINED", v & 0xFF);
        return buf;
    }
    if (table >= 0)
    {
        const char *n = MakerApi_NameOf(table, (unsigned)v);
        if (n && n[0])
            return n;
    }
    snprintf(buf, sizeof(buf), "%ld", v);
    return buf;
}

static bool TextValue(const std::string &prop, const char *s, long *v)
{
    int table = PropTable(prop);
    if (prop == "dest")
    {
        const char *colon = strchr(s, ':');
        if (!colon)
            return false;
        int m = MakerApi_FindMap(std::string(s, colon - s).c_str());
        if (m < 0)
            return false;
        *v = ((long)m << 8) | (atol(colon + 1) & 0xFF);
        return true;
    }
    if (table >= 0 && (isalpha((unsigned char)s[0]) || s[0] == '_'))
    {
        unsigned int u;
        if (!MakerApi_ValueOf(table, s, &u))
            return false;
        *v = u;
        return true;
    }
    char *end;
    *v = strtol(s, &end, 0);
    return end != s;
}

std::string EditsText(int map)
{
    const MapEdits *e = FindEdits(map);
    if (!e || e->Empty())
        return "";
    std::string out = "# Moemon Maker: edits to " + std::string(gEd.maps[map].name) + " (written by the editor; each line"
                      " has the value it replaced, then the new one)\n";
    char line[256];
    snprintf(line, sizeof(line), "map %s %d %d\n", gEd.maps[map].name, e->w, e->h);
    out += line;
    for (auto &b : e->blocks)
    {
        snprintf(line, sizeof(line), "block %d %d 0x%04X 0x%04X\n", b.first % e->w, b.first / e->w, b.second.first, b.second.second);
        out += line;
    }
    for (auto &p : e->props)
    {
        std::string prop = p.first.substr(p.first.rfind(' ') + 1);
        out += p.first + " " + ValueText(prop, p.second.was) + " " + ValueText(prop, p.second.now) + "\n";
    }
    for (auto &a : e->adds)
    {
        out += "add " + a.first;
        for (auto &kv : a.second)
            out += " " + kv.first + "=" + kv.second;
        out += "\n";
    }
    return out;
}

static bool ParseText(const std::string &text, int *mapOut, MapEdits *out)
{
    int map = -1;
    MapEdits e;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t nl = text.find('\n', pos);
        std::string line = text.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
        pos = nl == std::string::npos ? text.size() : nl + 1;
        if (line.empty() || line[0] == '#')
            continue;
        char w[5][96];
        int n = sscanf(line.c_str(), "%95s %95s %95s %95s %95s", w[0], w[1], w[2], w[3], w[4]);
        if (n >= 2 && !strcmp(w[0], "map"))
        {
            map = MakerApi_FindMap(w[1]);
            if (map < 0)
                return false;
            e.w = n >= 4 ? atoi(w[2]) : gEd.maps[map].width;
            e.h = n >= 4 ? atoi(w[3]) : gEd.maps[map].height;
        }
        else if (n >= 3 && !strcmp(w[0], "add"))
        {
            // add KIND N k=v k=v...
            std::map<std::string, std::string> kv;
            size_t at = line.find(' ', line.find(' ', 4) + 1);
            while (at != std::string::npos && at < line.size())
            {
                size_t start = line.find_first_not_of(' ', at);
                if (start == std::string::npos)
                    break;
                size_t end = line.find(' ', start);
                std::string word = line.substr(start, end == std::string::npos ? std::string::npos : end - start);
                size_t eq = word.find('=');
                if (eq != std::string::npos)
                    kv[word.substr(0, eq)] = word.substr(eq + 1);
                at = end;
            }
            e.adds[std::string(w[1]) + " " + w[2]] = kv;
        }
        else if (n == 5 && !strcmp(w[0], "block") && e.w)
            e.blocks[atoi(w[2]) * e.w + atoi(w[1])] = std::make_pair((int)strtol(w[3], NULL, 0), (int)strtol(w[4], NULL, 0));
        else if (n == 5)
        {
            long was, now;
            if (TextValue(w[2], w[3], &was) && TextValue(w[2], w[4], &now))
                e.props[std::string(w[0]) + " " + w[1] + " " + w[2]] = PropEdit{was, now};
        }
    }
    if (map < 0)
        return false;
    *mapOut = map;
    *out = e;
    return true;
}

static bool ReadFile(const std::string &path, std::string *text)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
        return false;
    char buf[4096];
    size_t n;
    text->clear();
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
        text->append(buf, n);
    fclose(f);
    return true;
}

static bool WriteFile(const std::string &path, const std::string &text)
{
    FILE *f = fopen(path.c_str(), "wb");
    if (!f)
        return false;
    fwrite(text.data(), 1, text.size(), f);
    fclose(f);
    return true;
}

static void MakeDirs()
{
    std::string d = EditsDir();
    MKDIR(d.c_str());
    MKDIR((d + "/base").c_str());
    MKDIR((d + "/.recovery").c_str());
}

// When the editor opens: the saved edits, then any recovery files newer than them (edits not saved with Ctrl+S)
void LoadAllEdits()
{
    sEdits.clear();
    sDirty.clear();
    sUndo.clear();
    sRedo.clear();
    gEd.recovered = 0;
    for (int m = 0; m < (int)gEd.maps.size(); m++)
    {
        std::string text;
        int map;
        MapEdits e;
        if (ReadFile(MapFile(m, "base"), &text) && ParseText(text, &map, &e) && map == m)
            sEdits[m] = e;
        if (ReadFile(MapFile(m, ".recovery"), &text))
        {
            MapEdits r;
            if (text.empty() || (ParseText(text, &map, &r) && map == m))
            {
                if (text.empty())
                    sEdits.erase(m);
                else
                    sEdits[m] = r;
                sDirty[m] = true;
                gEd.recovered++;
            }
        }
    }
    if (gEd.recovered)
        printf("[maker] %d maps' unsaved edits recovered\n", gEd.recovered);
}

void SendToGame(int map)
{
    MakerApi_SetMapEdits(gEd.maps[map].name, EditsText(map).c_str());
}

void BeginChange(int map)
{
    if (sInChange)
        return;
    sInChange = true;
    sChangeMap = map;
    auto it = sEdits.find(map);
    sChangeBefore = it == sEdits.end() ? MapEdits() : it->second;
}

void EndChange()
{
    if (!sInChange)
        return;
    sInChange = false;
    int map = sChangeMap;
    auto it = sEdits.find(map);
    MapEdits after = it == sEdits.end() ? MapEdits() : it->second;
    if (after.blocks == sChangeBefore.blocks && after.props == sChangeBefore.props && after.adds == sChangeBefore.adds)
        return;   // (nothing changed: no undo step)
    sUndo.push_back(UndoStep{map, sChangeBefore, after});
    if (sUndo.size() > 500)
        sUndo.erase(sUndo.begin());
    sRedo.clear();
    sDirty[map] = true;
    SendToGame(map);
    sRecoveryDue = SDL_GetTicks() + 2000;
    MapView_EditsChanged(map);
}

bool InChange()
{
    return sInChange;
}

static void Restore(int map, const MapEdits &e)
{
    if (e.Empty())
        sEdits.erase(map);
    else
        sEdits[map] = e;
    sDirty[map] = true;
    SendToGame(map);
    sRecoveryDue = SDL_GetTicks() + 2000;
    MapView_EditsChanged(map);
    if (map != gEd.map)
        SelectMap(map);   // (undo shows what it undid)
}

bool CanUndo() { return !sUndo.empty() && !sInChange; }
bool CanRedo() { return !sRedo.empty() && !sInChange; }

void Undo()
{
    if (!CanUndo())
        return;
    UndoStep s = sUndo.back();
    sUndo.pop_back();
    sRedo.push_back(s);
    Restore(s.map, s.before);
}

void Redo()
{
    if (!CanRedo())
        return;
    UndoStep s = sRedo.back();
    sRedo.pop_back();
    sUndo.push_back(s);
    Restore(s.map, s.after);
}

bool AnyUnsaved()
{
    for (auto &d : sDirty)
        if (d.second)
            return true;
    return false;
}

bool MapUnsaved(int map)
{
    auto it = sDirty.find(map);
    return it != sDirty.end() && it->second;
}

int EditedMapCount()
{
    int n = 0;
    for (auto &e : sEdits)
        if (!e.second.Empty())
            n++;
    return n;
}

// Ctrl+S: every changed map's file (removed when it has no edits left); the recovery files go
int SaveAll()
{
    MakeDirs();
    int written = 0;
    for (auto &d : sDirty)
    {
        if (!d.second)
            continue;
        std::string text = EditsText(d.first), path = MapFile(d.first, "base");
        if (text.empty())
            remove(path.c_str());
        else if (!WriteFile(path, text))
        {
            printf("[maker] couldn't write %s\n", path.c_str());
            continue;
        }
        remove(MapFile(d.first, ".recovery").c_str());
        d.second = false;
        written++;
    }
    sRecoveryDue = 0;
    printf("[maker] saved %d maps\n", written);
    return written;
}

// Every frame: the recovery files, a couple of seconds after the last change
void RecoveryTick()
{
    if (!sRecoveryDue || SDL_TICKS_PASSED(SDL_GetTicks(), sRecoveryDue) == 0)
        return;
    sRecoveryDue = 0;
    MakeDirs();
    for (auto &d : sDirty)
        if (d.second)
            WriteFile(MapFile(d.first, ".recovery"), EditsText(d.first));
}

// "Don't save": the recovery files go and the game goes back to the saved edits
void DiscardUnsaved()
{
    for (auto &d : sDirty)
        if (d.second)
            remove(MapFile(d.first, ".recovery").c_str());
    sDirty.clear();
    sRecoveryDue = 0;
    MakerApi_ReloadEdits();
}

void SendAllToGame()
{
    for (auto &e : sEdits)
        SendToGame(e.first);
}

} // namespace maker
