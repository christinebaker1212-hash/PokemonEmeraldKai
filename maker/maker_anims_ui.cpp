// Moemon Maker, M5: the ANIMATIONS window (Tools > Animations). A move's own animation on a timeline: one row an element
// (sound, flash, shake, ring, burst, beam, image), a bar from its first frame for its length (drag it to move it), its
// line to edit, and a preview on a stand-in battle scene that plays or steps frame by frame. Saved as
// MoemonMaker/anims/MOVE_X.txt (pictures beside it as PNGs); the language and the drawing are src/maker/maker_anims.c's.
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
static int sFile = -1, sSel = -1, sFrame = 0;
static bool sPlaying;
static std::vector<std::string> sLines;   // the open animation's lines (0 the header)
static std::string sSaved;
static char sNewMove[48] = "MOVE_TACKLE";
static SDL_Texture *sPreview;

static std::string Dir() { return std::string(MakerApi_EditsDir()) + "/anims"; }

static std::string Join()
{
    std::string t;
    for (auto &l : sLines)
        t += l + "\n";
    return t;
}

static void List()
{
    sFiles.clear();
    if (DIR *d = opendir(Dir().c_str()))
    {
        while (struct dirent *e = readdir(d))
        {
            size_t n = strlen(e->d_name);
            if (n > 4 && strcmp(e->d_name + n - 4, ".txt") == 0)
                sFiles.push_back(std::string(e->d_name, n - 4));
        }
        closedir(d);
    }
    std::sort(sFiles.begin(), sFiles.end());
}

static void Open(int i)
{
    sFile = i, sSel = -1, sFrame = 0, sPlaying = false;
    sLines.clear();
    if (i < 0 || i >= (int)sFiles.size())
        return;
    if (FILE *f = fopen((Dir() + "/" + sFiles[i] + ".txt").c_str(), "rb"))
    {
        char line[256];
        while (fgets(line, sizeof(line), f))
        {
            std::string l = line;
            while (!l.empty() && (l.back() == '\n' || l.back() == '\r'))
                l.pop_back();
            if (!l.empty())
                sLines.push_back(l);
        }
        fclose(f);
    }
    if (sLines.empty())
        sLines.push_back("anim " + sFiles[i] + " frames=48 replace=1");
    sSaved = Join();
}

static void Save()
{
    if (sFile < 0)
        return;
#ifdef _WIN32
    _mkdir(MakerApi_EditsDir());
    _mkdir(Dir().c_str());
#else
    mkdir(MakerApi_EditsDir(), 0755);
    mkdir(Dir().c_str(), 0755);
#endif
    if (FILE *f = fopen((Dir() + "/" + sFiles[sFile] + ".txt").c_str(), "wb"))
    {
        std::string t = Join();
        fwrite(t.data(), 1, t.size(), f);
        fclose(f);
        sSaved = t;
        MakerApi_AnimReload();
        printf("[maker] anim %s saved\n", sFiles[sFile].c_str());
    }
}

static void New(const char *move)
{
    unsigned v;
    if (!MakerApi_ValueOf(MAKER_NAMES_MOVE, move, &v))
        return;
    List();
    auto it = std::find(sFiles.begin(), sFiles.end(), std::string(move));
    if (it == sFiles.end())
    {
        sFiles.push_back(move);
        std::sort(sFiles.begin(), sFiles.end());
        it = std::find(sFiles.begin(), sFiles.end(), std::string(move));
        sFile = (int)(it - sFiles.begin());
        sLines = {std::string("anim ") + move + " frames=48 replace=1", "sound 0 SE_M_TACKLE",
                  "beam 4 18 FFE080 6", "flash 18 8 FFFFFF 40", "shake 20 12 target 3", "burst 20 16 target FFD040 12"};
        sSel = -1, sFrame = 0;
        Save();
    }
    else
        Open((int)(it - sFiles.begin()));
}

// an element line's first frame and length (for its bar)
static void Span(const std::string &l, int *start, int *len)
{
    char kind[16];
    *start = 0, *len = 1;
    if (sscanf(l.c_str(), "%15s %d %d", kind, start, len) < 2)
        return;
    if (!strcmp(kind, "sound"))
        *len = 2;
}

static void SetStart(int line, int start)
{
    char kind[16];
    int s, rest = 0;
    std::string &l = sLines[line];
    if (sscanf(l.c_str(), "%15s %d %n", kind, &s, &rest) < 2 || rest <= 0)
        return;
    l = std::string(kind) + " " + std::to_string(std::max(0, start)) + " " + l.substr(rest);
}

void AnimsWindow()
{
    if (!sOpen)
        return;
    ImGui::SetNextWindowSize(ImVec2(1000 * gEd.uiScale, 640 * gEd.uiScale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ANIMATIONS", &sOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::BeginChild("files", ImVec2(200 * gEd.uiScale, 0), ImGuiChildFlags_Borders);
    for (int i = 0; i < (int)sFiles.size(); i++)
        if (ImGui::Selectable(sFiles[i].c_str(), i == sFile))
            Open(i);
    ImGui::Separator();
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##move", sNewMove, sizeof(sNewMove));
    if (ImGui::Button("New animation for this move"))
        New(sNewMove);
    ImGui::TextDisabled("(a MOVE_ name)");
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("main", ImVec2(0, 0));
    if (sFile < 0 || sLines.empty())
    {
        ImGui::TextDisabled("Pick an animation, or make one for a move.");
        ImGui::EndChild();
        ImGui::End();
        return;
    }
    std::string text = Join();
    // the preview
    static unsigned int px[240 * 160];
    int frames = MakerApi_AnimPreview(text.c_str(), sFrame, px);
    if (!sPreview)
    {
        sPreview = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 240, 160);
        SDL_SetTextureScaleMode(sPreview, SDL_ScaleModeNearest);
    }
    SDL_UpdateTexture(sPreview, NULL, px, 240 * 4);
    ImGui::Image((ImTextureID)(intptr_t)sPreview, ImVec2(480 * gEd.uiScale, 320 * gEd.uiScale));
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("%s", sFiles[sFile].c_str());
    if (!frames)
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.2f, 1), "The first line must be: anim MOVE_X frames=N replace=0|1");
    else
        ImGui::TextDisabled("%d frames (%.2f s)", frames, frames / 60.0f);
    if (ImGui::Button(sPlaying ? "Pause" : "Play"))
        sPlaying = !sPlaying;
    ImGui::SameLine();
    if (ImGui::Button("<"))
        sFrame = std::max(0, sFrame - 1), sPlaying = false;
    ImGui::SameLine();
    if (ImGui::Button(">"))
        sFrame = frames ? (sFrame + 1) % frames : 0, sPlaying = false;
    if (sPlaying && frames)
        sFrame = (sFrame + 1) % frames;
    ImGui::SetNextItemWidth(220 * gEd.uiScale);
    ImGui::SliderInt("frame", &sFrame, 0, frames > 0 ? frames - 1 : 0);
    bool dirty = text != sSaved;
    if (ImGui::Button(dirty ? "Save*" : "Save"))
        Save();
    ImGui::SeparatorText("ADD");
    static const char *const kAdd[][2] = {
        {"Sound", "sound 0 SE_M_TACKLE"}, {"Flash", "flash 0 8 FFFFFF 40"}, {"Shake", "shake 0 12 target 3"},
        {"Ring", "ring 0 16 target 80C0FF 4 30"}, {"Burst", "burst 0 16 target FFD040 12"}, {"Beam", "beam 0 18 FFE080 6"},
        {"Image", "image 0 20 picture.png attacker target 100"}};
    for (int k = 0; k < 7; k++)
    {
        if (k % 4)
            ImGui::SameLine();
        if (ImGui::Button(kAdd[k][0]))
        {
            sLines.push_back(kAdd[k][1]);
            SetStart((int)sLines.size() - 1, sFrame);
            sSel = (int)sLines.size() - 1;
        }
    }
    ImGui::EndGroup();
    // the header and the picked line
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##head", &sLines[0]);
    if (sSel > 0 && sSel < (int)sLines.size())
    {
        ImGui::SetNextItemWidth(-90 * gEd.uiScale);
        ImGui::InputText("##line", &sLines[sSel]);
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            sLines.erase(sLines.begin() + sSel);
            sSel = -1;
        }
    }
    // the timeline
    ImGui::SeparatorText("TIMELINE");
    ImGui::BeginChild("tl", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    float row = 22 * gEd.uiScale, scale = 6 * gEd.uiScale, label = 70 * gEd.uiScale;
    ImVec2 o = ImGui::GetCursorScreenPos();
    int n = (int)sLines.size() - 1;
    float wTotal = label + (frames > 0 ? frames : 60) * scale + 20;
    ImGui::InvisibleButton("##canvas", ImVec2(wTotal, row * (n + 1) + 4));
    for (int f = 0; f <= frames; f += 10)
    {
        float x = o.x + label + f * scale;
        dl->AddLine(ImVec2(x, o.y), ImVec2(x, o.y + row * (n + 1)), IM_COL32(128, 128, 128, 60));
        char b[8];
        snprintf(b, sizeof(b), "%d", f);
        dl->AddText(ImVec2(x + 2, o.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), b);
    }
    float cx = o.x + label + sFrame * scale;
    dl->AddLine(ImVec2(cx, o.y), ImVec2(cx, o.y + row * (n + 1)), IM_COL32(255, 80, 60, 220), 2);
    static int sDrag = -1;
    static float sDragX;
    static int sDragStart;
    ImVec2 m = ImGui::GetMousePos();
    for (int i = 1; i <= n; i++)
    {
        int start, len;
        Span(sLines[i], &start, &len);
        char kind[16] = "";
        sscanf(sLines[i].c_str(), "%15s", kind);
        float y = o.y + row * i;
        dl->AddText(ImVec2(o.x + 4, y + 3), ImGui::GetColorU32(ImGuiCol_Text), kind);
        ImU32 col = !strcmp(kind, "sound") ? IM_COL32(120, 180, 255, 255) : !strcmp(kind, "flash") ? IM_COL32(250, 230, 120, 255)
                  : !strcmp(kind, "shake") ? IM_COL32(220, 140, 90, 255) : !strcmp(kind, "image") ? IM_COL32(170, 120, 220, 255)
                  : IM_COL32(120, 200, 140, 255);
        ImVec2 a(o.x + label + start * scale, y + 3), b(o.x + label + (start + len) * scale, y + row - 3);
        dl->AddRectFilled(a, b, col, 3);
        if (i == sSel)
            dl->AddRect(a, b, IM_COL32(255, 255, 255, 255), 3, 0, 2);
        if (ImGui::IsMouseClicked(0) && m.x >= a.x && m.x <= b.x && m.y >= a.y && m.y <= b.y)
            sSel = i, sDrag = i, sDragX = m.x, sDragStart = start;
    }
    if (sDrag > 0 && ImGui::IsMouseDown(0))
        SetStart(sDrag, sDragStart + (int)((m.x - sDragX) / scale));
    else
        sDrag = -1;
    if (ImGui::IsItemClicked() && sDrag < 0 && m.x > o.x + label)
        sFrame = std::max(0, std::min(frames > 0 ? frames - 1 : 0, (int)((m.x - o.x - label) / scale)));
    ImGui::EndChild();
    ImGui::EndChild();
    ImGui::End();
}

void OpenAnims()
{
    sOpen = true;
    List();
}

// tests: "anim open", "anim new MOVE_X", "anim frame N", "anim add LINE", "anim save"
void Anims_TestCommand(const char *cmd, const char *arg)
{
    char what[16] = "";
    sscanf(arg, "%15s", what);
    const char *rest = strchr(arg, ' ') ? strchr(arg, ' ') + 1 : "";
    if (!strcmp(what, "open"))
        OpenAnims();
    else if (!strcmp(what, "new"))
        OpenAnims(), New(rest);
    else if (!strcmp(what, "frame"))
        sFrame = atoi(rest), sPlaying = false;
    else if (!strcmp(what, "add") && sFile >= 0)
        sLines.push_back(rest);
    else if (!strcmp(what, "save"))
        Save();
    (void)cmd;
}

} // namespace maker
