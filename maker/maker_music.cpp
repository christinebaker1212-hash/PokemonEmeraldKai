// Moemon Maker: music (docs/MOEMON_MAKER.md decision 5). The MUSIC window (Tools > Music): new tracks (a .kaisong
// copied into MoemonMaker/music/ under a name of its own: maps can play it), and replacements (the <song>@<place>
// rule by picking: this song, in this place, plays this file instead). Everything plays in the editor through the
// game's own player. A map's music is picked in the INSPECTOR (SongPicker). Files can be dropped on the window.
#include <SDL.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <string.h>
#include <string>
#include <vector>
#include "imgui.h"
#include "maker_ui.h"

namespace maker {

static char sImportPath[1024];
static char sTrackName[64] = "mus_";
static int sReplaceSong = -1, sReplacePlace = 0, sReplaceScene = 0;
static std::string sNote;

static std::string MusicDir()
{
    return EditsDir() + "/music";
}

static bool CopyFile(const std::string &from, const std::string &to)
{
    FILE *in = fopen(from.c_str(), "rb");
    if (!in)
        return false;
    FILE *out = fopen(to.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        return false;
    }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in);
    fclose(out);
    return true;
}

static void MakeMusicDir()
{
#ifdef _WIN32
    _mkdir(EditsDir().c_str());
    _mkdir(MusicDir().c_str());
#else
    mkdir(EditsDir().c_str(), 0755);
    mkdir(MusicDir().c_str(), 0755);
#endif
}

static bool ValidTrackName(const char *n)
{
    if (strncmp(n, "mus_", 4) != 0 || !n[4])
        return false;
    for (const char *p = n; *p; p++)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '_'))
            return false;
    return MakerApi_SongId(n) < 0 || MakerApi_SongId(n) >= 0x4000;   // (not one of the game's own songs)
}

// A .kaisong copied in as a new track (name: mus_...), or as a replacement (file name: <song>[@<place>].kaisong)
static bool Import(const std::string &from, const std::string &fileName)
{
    MakeMusicDir();
    std::string to = MusicDir() + "/" + fileName;
    if (!CopyFile(from, to))
    {
        sNote = "Couldn't copy " + from;
        return false;
    }
    MakerApi_MusicChanged();
    sNote = "Added " + fileName;
    Message("Added %s", fileName.c_str());
    return true;
}

bool SongPicker(const char *id, int *song)
{
    bool picked = false;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, *song >= 0 ? MakerApi_SongName(*song) : "?", ImGuiComboFlags_HeightLarge))
    {
        static char filter[48];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##f", "Search...", filter, sizeof(filter));
        auto hit = [&](const char *name) {
            if (!filter[0])
                return true;
            for (const char *p = name; *p; p++)
                if (strncasecmp(p, filter, strlen(filter)) == 0)
                    return true;
            return false;
        };
        int tracks = MakerApi_TrackCount();
        if (tracks)
            ImGui::SeparatorText("Your tracks");
        for (int i = 0; i < tracks; i++)
        {
            const char *name = MakerApi_TrackName(i);
            int sid = MakerApi_SongId(name);
            if (hit(name) && ImGui::Selectable(name, sid == *song))
                *song = sid, picked = true;
        }
        ImGui::SeparatorText("The game's songs");
        int n = MakerApi_NameCount(MAKER_NAMES_SONG);
        for (int i = 0; i < n; i++)
        {
            unsigned v;
            const char *name = MakerApi_NameAt(MAKER_NAMES_SONG, i, &v);
            if (strncmp(name, "MUS_", 4) != 0 || !hit(name))
                continue;
            ImGui::PushID(i);
            if (ImGui::Selectable(name, (int)v == *song))
                *song = v, picked = true;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return picked;
}

void MusicWindow()
{
    if (!gEd.musicOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(560 * gEd.uiScale, 520 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("MUSIC", &gEd.musicOpen, ImGuiWindowFlags_NoDocking))
    {
        ImGui::End();
        return;
    }
    ImGui::TextWrapped("Your music lives in %s. Drop a .kaisong file on this window, or type its path.", MusicDir().c_str());
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##path", "Path of a .kaisong file (Moemon Music Studio's export)", sImportPath, sizeof(sImportPath));
    if (ImGui::Button("Stop music"))
        MakerApi_StopSong();
    if (!sNote.empty())
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", sNote.c_str());
    }

    ImGui::SeparatorText("NEW TRACKS");
    ImGui::TextDisabled("A track of your own, with a name maps can use (mus_...).");
    ImGui::SetNextItemWidth(200 * gEd.uiScale);
    ImGui::InputText("##name", sTrackName, sizeof(sTrackName));
    ImGui::SameLine();
    bool okName = ValidTrackName(sTrackName);
    if (!okName)
        ImGui::BeginDisabled();
    if (ImGui::Button("Add as a new track") && sImportPath[0])
        Import(sImportPath, std::string(sTrackName) + ".kaisong");
    if (!okName)
    {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("(mus_ then a-z, 0-9, _; not a game song's name)");
    }
    int tracks = MakerApi_TrackCount();
    for (int i = 0; i < tracks; i++)
    {
        const char *name = MakerApi_TrackName(i);
        ImGui::PushID(i);
        if (ImGui::SmallButton("Play"))
            MakerApi_PlaySong(MakerApi_SongId(name));
        ImGui::SameLine();
        ImGui::Text("%s", name);
        ImGui::SameLine();
        if (ImGui::SmallButton("Remove"))
        {
            std::string path = MusicDir() + "/" + name + ".kaisong";
            remove(path.c_str());
            MakerApi_MusicChanged();
            Message("Removed %s", name);
        }
        ImGui::PopID();
    }
    if (!tracks)
        ImGui::TextDisabled("No tracks yet.");

    ImGui::SeparatorText("REPLACE A SONG");
    ImGui::TextDisabled("Play the file above instead of a game song: everywhere, on one map, or in one scene.");
    if (ImGui::BeginTable("rep", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("k", ImGuiTableColumnFlags_WidthFixed, 80 * gEd.uiScale);
        ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Song");
        ImGui::TableNextColumn();
        SongPicker("##rsong", &sReplaceSong);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Where");
        ImGui::TableNextColumn();
        ImGui::RadioButton("Everywhere", &sReplacePlace, 0);
        ImGui::SameLine();
        ImGui::RadioButton(gEd.map >= 0 ? gEd.maps[gEd.map].title : "This map", &sReplacePlace, 1);
        ImGui::SameLine();
        ImGui::RadioButton("A scene", &sReplacePlace, 2);
        if (sReplacePlace == 2)
        {
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##scene", MakerApi_SceneKey(sReplaceScene) ? MakerApi_SceneKey(sReplaceScene) : "?"))
            {
                for (int i = 0; i < MakerApi_SceneCount(); i++)
                    if (ImGui::Selectable(MakerApi_SceneKey(i), i == sReplaceScene))
                        sReplaceScene = i;
                ImGui::EndCombo();
            }
        }
        ImGui::EndTable();
    }
    bool can = sReplaceSong >= 0 && sReplaceSong < 0x4000 && sImportPath[0] && MakerApi_SongFileName(sReplaceSong)
            && (sReplacePlace != 1 || gEd.map >= 0);
    if (!can)
        ImGui::BeginDisabled();
    if (ImGui::Button("Replace it"))
    {
        std::string file = MakerApi_SongFileName(sReplaceSong);
        if (sReplacePlace == 1)
        {
            // (a map's place key: its MAP_ name without MAP_, lowercase)
            std::string m = gEd.maps[gEd.map].name + 4;
            for (char &ch : m)
                ch = (char)tolower((unsigned char)ch);
            file += "@" + m;
        }
        else if (sReplacePlace == 2)
            file += std::string("@") + MakerApi_SceneKey(sReplaceScene);
        Import(sImportPath, file + ".kaisong");
    }
    if (!can)
        ImGui::EndDisabled();
    ImGui::End();
}

void Music_Dropped(const std::string &path)
{
    snprintf(sImportPath, sizeof(sImportPath), "%s", path.c_str());
    gEd.musicOpen = true;
    sNote = "Dropped: pick what it's for below.";
}

// Tests: "maker track NAME PATH" (a new track), "maker replace SONG PLACE PATH" (PLACE: all / map / a scene's key)
void Music_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    char a[96], b[96], path[600];
    if (c == "track" && sscanf(arg, "%95s %599s", a, path) == 2)
    {
        if (ValidTrackName(a))
            Import(path, std::string(a) + ".kaisong");
    }
    else if (c == "replace" && sscanf(arg, "%95s %95s %599s", a, b, path) == 3)
    {
        int song = MakerApi_SongId(a);
        if (song < 0 || !MakerApi_SongFileName(song))
            return;
        std::string file = MakerApi_SongFileName(song), place = b;
        if (place == "map" && gEd.map >= 0)
        {
            std::string m = gEd.maps[gEd.map].name + 4;
            for (char &ch : m)
                ch = (char)tolower((unsigned char)ch);
            file += "@" + m;
        }
        else if (place != "all")
            file += "@" + place;
        Import(path, file + ".kaisong");
    }
}

} // namespace maker
