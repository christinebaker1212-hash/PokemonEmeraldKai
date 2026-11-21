// Moemon VMU manager: VMU images on a VM2's SD card (in the PC, or over USB through the VM2 itself), on the PC, or in
// the VM2's internal memory. Browse them, move files between them, edit icons (the colour ones and the VMU's own 1-bit
// one), descriptions and names; extract and load files (.VMS + .VMI, .DCI). Dear ImGui on SDL2.
//
//   vmu_manager                     the window
//   vmu_manager --selftest          checks the VMU code (no window)
//   vmu_manager --shot OUT.png IMG  draws the window with IMG open into OUT.png (headless: SDL_VIDEODRIVER=dummy)
#include <SDL.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "misc/imgui_stdlib.h"

#include "font_data.h"
#include "font_bold.h"
#include "pngio.h"
#include "vm2link.h"
#include "vmu.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fs = std::filesystem;
using vmu::Bytes;

static const char* kTitle = "Moemon VMU Manager";

// ---------------------------------------------------------------- small helpers

static bool readFile(const std::string& path, Bytes& out) {
    std::ifstream f(fs::u8path(path), std::ios::binary);
    if(!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

static bool writeFile(const std::string& path, const Bytes& data) {
    std::ofstream f(fs::u8path(path), std::ios::binary);
    if(!f) return false;
    f.write((const char*)data.data(), (std::streamsize)data.size());
    return (bool)f;
}

static std::string upper(std::string s) {
    for(auto& c : s) c = (char)toupper((unsigned char)c);
    return s;
}

static std::string baseDir() {
    char* p = SDL_GetBasePath();
    std::string s = p ? p : "./";
    SDL_free(p);
    return s;
}

static std::string nowStamp() {
    time_t t = time(nullptr);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    char b[32];
    strftime(b, sizeof b, "%Y%m%d-%H%M%S", &tmv);
    return b;
}


// messages: small notices in the corner (drawToasts; defined near main)
static void status(const std::string& s, bool bad = false);

// Every image is copied here before anything overwrites it (an SD card file, the VM2's memory, a PC file).
static std::string backup(const std::string& what, const Bytes& data) {
    std::string dir = baseDir() + "backups";
    std::error_code ec;
    fs::create_directories(fs::u8path(dir), ec);
    std::string safe;
    for(char c : what) safe += (isalnum((unsigned char)c) || c == '.' || c == '-') ? c : '_';
    std::string path = dir + "/" + nowStamp() + "_" + safe;
    if(path.size() < 4 || path.substr(path.size() - 4) != ".bin") path += ".bin";
    writeFile(path, data);
    return path;
}

// ---------------------------------------------------------------- background jobs (VM2 transfers)

struct Job {
    std::string label;
    std::atomic<int> done{0}, total{0};
    std::atomic<bool> finished{false};
    bool ok = false;
    std::string err;
    std::function<bool(Job&, std::string*)> work;
    std::function<void(Job&)> then;
    std::thread thread;
    vm2::Progress progress() {
        return [this](int d, int t) { done = d; total = t; };
    }
};
static std::unique_ptr<Job> g_job;

static bool busy() { return g_job != nullptr; }

static void runJob(const std::string& label, std::function<bool(Job&, std::string*)> work, std::function<void(Job&)> then) {
    if(busy()) { status("Wait: " + g_job->label); return; }
    g_job = std::make_unique<Job>();
    g_job->label = label;
    g_job->work = std::move(work);
    g_job->then = std::move(then);
    Job* j = g_job.get();
    j->thread = std::thread([j] {
        j->ok = j->work(*j, &j->err);
        j->finished = true;
    });
}

static void pollJob() {
    if(!g_job || !g_job->finished) return;
    g_job->thread.join();
    std::unique_ptr<Job> j = std::move(g_job);
    if(!j->ok) status(j->label + " failed: " + j->err);
    if(j->then) j->then(*j);
}

// ---------------------------------------------------------------- open VMU images ("documents")

enum class Src { New, PcFile, Vm2Sd, Vm2Internal };

struct Editor {
    int entry = -1;  // the file being edited (its directory entry)
    vmu::File file;
    bool isVms = false, isIconData = false;
    vmu::VmsHeader h, hOrig;
    vmu::IconData icon, iconOrig;
    std::string name;
    bool copyProtected = false;
    int frame = 0, color = 1;
    bool editingMono = true;
    bool dirty = false;
};

struct Doc {
    std::string title;
    Src src = Src::New;
    std::string path;     // a PC file, or the VM2 SD path
    vmu::Image img;
    Bytes original;
    int selected = -1;    // directory entry
    Editor ed;
    bool open = true;
    bool dirty() const { return img.raw != original; }
    std::string where() const {
        switch(src) {
            case Src::New: return "new, not saved yet";
            case Src::PcFile: return path;
            case Src::Vm2Sd: return "VM2 SD card " + path;
            case Src::Vm2Internal: return "VM2 internal memory";
        }
        return "";
    }
};
static std::vector<std::unique_ptr<Doc>> g_docs;
static int g_active = -1;

static Doc* activeDoc() { return g_active >= 0 && g_active < (int)g_docs.size() ? g_docs[g_active].get() : nullptr; }

static Doc* openDoc(const std::string& title, Src src, const std::string& path, const Bytes& data, std::string* err) {
    auto d = std::make_unique<Doc>();
    if(!d->img.load(data, err)) return nullptr;
    if(!d->img.formatted()) { if(err) *err = "this isn't a formatted VMU image"; return nullptr; }
    d->title = title;
    d->src = src;
    d->path = path;
    d->original = d->img.raw;
    for(int i = 0; i < (int)g_docs.size(); ++i)
        if(g_docs[i]->src == src && g_docs[i]->path == path && !path.empty() && src != Src::New) {
            g_docs[i] = std::move(d);  // opened again: the fresh copy
            g_active = i;
            return g_docs[i].get();
        }
    g_docs.push_back(std::move(d));
    g_active = (int)g_docs.size() - 1;
    return g_docs.back().get();
}

// ---------------------------------------------------------------- the VM2

static vm2::Device g_vm2;
static std::vector<vm2::PortInfo> g_ports;
static int g_portPick = 0;  // 0 = find it
struct SdFolder {
    bool loaded = false;
    std::vector<vm2::Entry> entries;
};
static std::map<std::string, SdFolder> g_sd;  // "\" and "\NAME"
static std::string g_sdClipboard;            // a file path to paste

static void sdRefresh(const std::string& folder) {
    auto out = std::make_shared<std::vector<vm2::Entry>>();
    runJob("Listing " + folder, [out, folder](Job&, std::string* err) { return g_vm2.list(folder, *out, err); },
           [out, folder](Job& j) {
               if(!j.ok) return;
               g_sd[folder].entries = *out;
               g_sd[folder].loaded = true;
           });
}

static std::string sdJoin(const std::string& folder, const std::string& name) {
    return folder == "\\" ? "\\" + name : folder + "\\" + name;
}

static void vm2Connect() {
    std::string port = g_portPick > 0 && g_portPick <= (int)g_ports.size() ? g_ports[g_portPick - 1].name : "";
    runJob("Connecting to the VM2", [port](Job&, std::string* err) {
        if(!g_vm2.connect(port, err)) return false;
        std::string e2;
        g_vm2.syncClock(&e2);  // as DreamMods' GUI does on connecting
        return true;
    }, [](Job& j) {
        if(!j.ok) return;
        g_sd.clear();
        status("VM2 connected on " + g_vm2.port + " (firmware " + g_vm2.fw + ")");
    });
}

// ---------------------------------------------------------------- a file browser (the same on Windows and Linux)

struct Browser {
    bool active = false, save = false, folders = false;
    std::string title, dir, name;
    std::vector<std::string> exts;  // lowercase, with the dot; empty = everything
    std::function<void(const std::string&)> done;
    std::vector<fs::directory_entry> list;
    std::string listed;
    std::string err;
};
static Browser g_browse;
static std::string g_lastDir;

static void browse(const std::string& title, bool save, bool folders, std::vector<std::string> exts,
                   const std::string& name, std::function<void(const std::string&)> done) {
    g_browse = Browser();
    g_browse.active = true;
    g_browse.title = title;
    g_browse.save = save;
    g_browse.folders = folders;
    g_browse.exts = std::move(exts);
    g_browse.name = name;
    g_browse.done = std::move(done);
    std::error_code ec;
    g_browse.dir = !g_lastDir.empty() ? g_lastDir : fs::current_path(ec).u8string();
}

static void drawBrowser() {
    if(!g_browse.active) return;
    Browser& b = g_browse;
    ImGui::OpenPopup(b.title.c_str());
    ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_Appearing);
    if(!ImGui::BeginPopupModal(b.title.c_str(), nullptr)) return;
    if(b.listed != b.dir) {
        b.list.clear();
        b.err.clear();
        std::error_code ec;
        for(auto& e : fs::directory_iterator(fs::u8path(b.dir), fs::directory_options::skip_permission_denied, ec)) {
            std::error_code e2;
            if(e.is_directory(e2)) { b.list.push_back(e); continue; }
            if(b.folders) continue;
            std::string ext = e.path().extension().u8string();
            for(auto& c : ext) c = (char)tolower((unsigned char)c);
            if(b.exts.empty() || std::find(b.exts.begin(), b.exts.end(), ext) != b.exts.end()) b.list.push_back(e);
        }
        if(ec) b.err = ec.message();
        std::sort(b.list.begin(), b.list.end(), [](const fs::directory_entry& x, const fs::directory_entry& y) {
            std::error_code e1, e2;
            bool dx = x.is_directory(e1), dy = y.is_directory(e2);
            if(dx != dy) return dx;
            return upper(x.path().filename().u8string()) < upper(y.path().filename().u8string());
        });
        b.listed = b.dir;
    }
#ifdef _WIN32
    DWORD drives = GetLogicalDrives();
    for(int i = 0; i < 26; ++i)
        if(drives & (1u << i)) {
            char d[4] = {(char)('A' + i), ':', '\\', 0};
            if(ImGui::SmallButton(d)) b.dir = d;
            ImGui::SameLine();
        }
    ImGui::NewLine();
#endif
    if(ImGui::Button("Up")) {
        fs::path p = fs::u8path(b.dir);
        if(p.has_parent_path() && p.parent_path() != p) b.dir = p.parent_path().u8string();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    std::string edit = b.dir;
    if(ImGui::InputText("##dir", &edit, ImGuiInputTextFlags_EnterReturnsTrue)) b.dir = edit;
    if(!b.err.empty()) ImGui::TextColored(ImVec4(1, .4f, .4f, 1), "%s", b.err.c_str());
    ImGui::BeginChild("list", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2.2f), ImGuiChildFlags_Borders);
    std::string go;
    for(auto& e : b.list) {
        std::error_code ec;
        bool dir = e.is_directory(ec);
        std::string n = e.path().filename().u8string();
        std::string label = (dir ? "[" + n + "]" : n);
        if(ImGui::Selectable(label.c_str(), !dir && n == b.name, ImGuiSelectableFlags_AllowDoubleClick)) {
            if(dir) go = e.path().u8string();
            else {
                b.name = n;
                if(ImGui::IsMouseDoubleClicked(0)) {
                    g_lastDir = b.dir;
                    auto done = b.done;
                    std::string pick = (fs::u8path(b.dir) / fs::u8path(n)).u8string();
                    b.active = false;
                    ImGui::CloseCurrentPopup();
                    ImGui::EndChild();
                    ImGui::EndPopup();
                    done(pick);
                    return;
                }
            }
        }
    }
    ImGui::EndChild();
    if(!go.empty()) b.dir = go;
    if(!b.folders) {
        ImGui::SetNextItemWidth(-200);
        ImGui::InputText("##name", &b.name);
        ImGui::SameLine();
    }
    bool ok = ImGui::Button(b.folders ? "Use this folder" : (b.save ? "Save" : "Open"), ImVec2(120, 0));
    ImGui::SameLine();
    bool cancel = ImGui::Button("Cancel", ImVec2(70, 0));
    if(ok && (b.folders || !b.name.empty())) {
        g_lastDir = b.dir;
        auto done = b.done;
        std::string pick = b.folders ? b.dir : (fs::u8path(b.dir) / fs::u8path(b.name)).u8string();
        b.active = false;
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        done(pick);
        return;
    }
    if(cancel) { b.active = false; ImGui::CloseCurrentPopup(); }
    ImGui::EndPopup();
}

// ---------------------------------------------------------------- confirm / ask

struct Ask {
    bool active = false;
    std::string title, text, input;
    bool wantsInput = false;
    std::function<void(const std::string&)> yes;
};
static Ask g_ask;

static void ask(const std::string& title, const std::string& text, std::function<void(const std::string&)> yes,
                bool wantsInput = false, const std::string& input = "") {
    g_ask = Ask{true, title, text, input, wantsInput, std::move(yes)};
}

static void drawAsk() {
    if(!g_ask.active) return;
    ImGui::OpenPopup(g_ask.title.c_str());
    if(!ImGui::BeginPopupModal(g_ask.title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::TextWrapped("%s", g_ask.text.c_str());
    if(g_ask.wantsInput) {
        if(ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(320);
        ImGui::InputText("##ask", &g_ask.input);
    }
    if(ImGui::Button("OK", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        auto yes = g_ask.yes;
        std::string in = g_ask.input;
        g_ask.active = false;
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        yes(in);
        return;
    }
    ImGui::SameLine();
    if(ImGui::Button("Cancel", ImVec2(100, 0)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        g_ask.active = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

// ---------------------------------------------------------------- icon textures

static SDL_Renderer* g_ren = nullptr;
struct Tex {
    SDL_Texture* tex = nullptr;
    uint64_t hash = 0;
    int lastUsed = 0;
};
static std::map<std::string, Tex> g_tex;
static int g_frameNo = 0;

static uint64_t fnv(const void* p, size_t n, uint64_t h = 1469598103934665603ull) {
    const uint8_t* b = (const uint8_t*)p;
    for(size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
    return h;
}

static SDL_Texture* texFor(const std::string& key, const uint32_t* rgba32x32) {
    Tex& t = g_tex[key];
    uint64_t h = fnv(rgba32x32, 32 * 32 * 4);
    t.lastUsed = g_frameNo;
    if(t.tex && t.hash == h) return t.tex;
    if(!t.tex) {
        t.tex = SDL_CreateTexture(g_ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, 32, 32);
        SDL_SetTextureBlendMode(t.tex, SDL_BLENDMODE_BLEND);
    }
    SDL_UpdateTexture(t.tex, nullptr, rgba32x32, 32 * 4);
    t.hash = h;
    return t.tex;
}

static void iconRgba(const vmu::Icon& ic, const std::array<uint16_t, 16>& pal, uint32_t* out) {
    for(int y = 0; y < 32; ++y)
        for(int x = 0; x < 32; ++x) out[y * 32 + x] = vmu::argb4444ToRgba(pal[ic.at(x, y)]);
}

static void monoRgba(const vmu::MonoIcon& m, uint32_t* out) {
    for(int y = 0; y < 32; ++y)
        for(int x = 0; x < 32; ++x) out[y * 32 + x] = m.at(x, y) ? 0xFF283024u : 0xFF8CAA9Cu;  // VMU LCD greens
}

static void gcTextures() {
    for(auto it = g_tex.begin(); it != g_tex.end();) {
        if(g_frameNo - it->second.lastUsed > 120) {
            SDL_DestroyTexture(it->second.tex);
            it = g_tex.erase(it);
        } else ++it;
    }
}

// The icon of a file in a VMU (its first frame, or the frame the animation is on)
static SDL_Texture* fileIcon(Doc& d, const vmu::File& f) {
    Bytes b = d.img.read(f);
    uint32_t px[1024];
    if(f.isIconData()) {
        vmu::IconData id;
        if(!vmu::parseIconData(b, id)) return nullptr;
        if(id.hasColor) iconRgba(id.color, id.palette, px);
        else monoRgba(id.mono, px);
    } else {
        vmu::VmsHeader h;
        if(!vmu::parseVms(b, f.headerOffset, h) || h.icons.empty()) return nullptr;
        int speed = std::max<int>(1, h.animSpeed);
        int frame = (int)(SDL_GetTicks() / (speed * 1000 / 30)) % (int)h.icons.size();
        iconRgba(h.icons[frame], h.palette, px);
    }
    return texFor(d.title + "/" + f.name, px);
}

// ---------------------------------------------------------------- the editor (the selected file)

static void loadEditor(Doc& d) {
    Editor& e = d.ed;
    e = Editor();
    e.entry = d.selected;
    for(auto& f : d.img.files())
        if(f.entry == d.selected) {
            e.file = f;
            e.name = f.name;
            e.copyProtected = f.copyProtected;
            Bytes b = d.img.read(f);
            if(f.isIconData()) e.isIconData = vmu::parseIconData(b, e.icon);
            else e.isVms = vmu::parseVms(b, f.headerOffset, e.h);
            e.hOrig = e.h;
            e.iconOrig = e.icon;
            return;
        }
    e.entry = -1;
}

static void selectByName(Doc& d, const std::string& name) {
    d.selected = -1;
    for(auto& f : d.img.files()) if(f.name == name) d.selected = f.entry;
    loadEditor(d);
}

static void applyEditor(Doc& d) {
    Editor& e = d.ed;
    std::string err;
    std::string newName = upper(e.name);
    if(newName != e.file.name && !d.img.rename(e.file, newName, &err)) { status(err); return; }
    e.file.name = newName;
    d.img.setCopyProtected(e.file, e.copyProtected);
    Bytes b = d.img.read(e.file);
    if(e.isVms) {
        bool isData = !e.file.isGame();
        if(e.h.icons.size() == e.hOrig.icons.size()) {
            if(!vmu::writeVmsInPlace(b, e.file.headerOffset, e.h, isData)) { status("the header doesn't fit this file"); return; }
        } else if(isData) {
            b = vmu::rebuildData(b, e.hOrig, e.h);
        } else { status("a GAME's icons can't change in number (its program follows them)"); return; }
        if(!d.img.replace(e.file, b, &err)) { status(err); return; }
    } else if(e.isIconData) {
        if(!d.img.replace(e.file, vmu::buildIconData(e.icon), &err)) { status(err); return; }
    }
    selectByName(d, newName);
    status("Changes made to " + newName + " (Save to write the VMU back)");
}

// Quantize a picture to the 16-colour palette: transparent pixels to colour 0, the rest to the closest colours
// (the most used ones first).
static void pngToIcon(const png::Rgba& img, int frameX, vmu::Icon& ic, std::array<uint16_t, 16>& pal, bool makePalette) {
    std::map<uint16_t, int> counts;
    auto px = [&](int x, int y) { return img.px[(size_t)y * img.w + frameX * 32 + x]; };
    if(makePalette) {
        for(int y = 0; y < 32; ++y)
            for(int x = 0; x < 32; ++x) {
                uint32_t c = px(x, y);
                if((c >> 24) < 128) continue;
                counts[vmu::rgbaToArgb4444(c, c >> 8, c >> 16, 255)]++;
            }
        std::vector<std::pair<int, uint16_t>> byUse;
        for(auto& kv : counts) byUse.push_back({-kv.second, kv.first});
        std::sort(byUse.begin(), byUse.end());
        pal.fill(0xF000);
        pal[0] = 0x0000;  // transparent
        for(int i = 0; i < 15 && i < (int)byUse.size(); ++i) pal[i + 1] = byUse[i].second;
    }
    for(int y = 0; y < 32; ++y)
        for(int x = 0; x < 32; ++x) {
            uint32_t c = px(x, y);
            if((c >> 24) < 128) { ic.set(x, y, 0); continue; }
            int best = 1, bestD = 1 << 30;
            for(int i = 0; i < 16; ++i) {
                if(((pal[i] >> 12) & 15) < 8) continue;
                int r = ((pal[i] >> 8) & 15) * 17 - (int)(c & 255), g = ((pal[i] >> 4) & 15) * 17 - (int)((c >> 8) & 255),
                    bl = (pal[i] & 15) * 17 - (int)((c >> 16) & 255);
                int dd = r * r * 3 + g * g * 4 + bl * bl * 2;
                if(dd < bestD) { bestD = dd; best = i; }
            }
            ic.set(x, y, (uint8_t)best);
        }
}

static bool g_shotMode = false;
static std::string g_startPort;  // --port: connect to the VM2 there at start
static int g_shotTab = 0;        // --shot: the inspector tab shown (SHOT_TAB)

// A 32x32 pixel canvas. Returns true when it changed something.
static bool canvas(const char* id, const uint32_t* rgba, float zoom, std::function<void(int, int, int)> paint) {
    ImGui::PushID(id);
    SDL_Texture* t = texFor(std::string("canvas/") + id, rgba);
    ImVec2 at = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    float s = 32 * zoom;
    for(int y = 0; y < 8; ++y)  // a checkerboard under transparent pixels
        for(int x = 0; x < 8; ++x)
            dl->AddRectFilled(ImVec2(at.x + x * s / 8, at.y + y * s / 8), ImVec2(at.x + (x + 1) * s / 8, at.y + (y + 1) * s / 8),
                              ((x + y) & 1) ? IM_COL32(90, 90, 90, 255) : IM_COL32(60, 60, 60, 255));
    ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(s, s));
    bool changed = false;
    if(ImGui::IsItemHovered()) {
        ImVec2 m = ImGui::GetMousePos();
        int x = (int)((m.x - at.x) / zoom), y = (int)((m.y - at.y) / zoom);
        if(x >= 0 && x < 32 && y >= 0 && y < 32) {
            dl->AddRect(ImVec2(at.x + x * zoom, at.y + y * zoom), ImVec2(at.x + (x + 1) * zoom, at.y + (y + 1) * zoom),
                        IM_COL32(255, 255, 0, 255));
            int button = ImGui::IsMouseDown(0) ? 0 : ImGui::IsMouseDown(1) ? 1 : -1;
            if(button >= 0) { paint(x, y, button); changed = true; }
        }
    }
    ImGui::PopID();
    return changed;
}

static bool paletteEditor(std::array<uint16_t, 16>& pal, int& pick) {
    bool changed = false;
    for(int i = 0; i < 16; ++i) {
        ImGui::PushID(i);
        uint16_t c = pal[i];
        ImVec4 col(((c >> 8) & 15) / 15.f, ((c >> 4) & 15) / 15.f, (c & 15) / 15.f, ((c >> 12) & 15) / 15.f);
        if(i == pick) ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 0, 1));
        if(ImGui::ColorButton("##c", col, ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(22, 22))) pick = i;
        if(i == pick) ImGui::PopStyleColor();
        if(ImGui::BeginPopupContextItem("edit")) {
            float f[4] = {col.x, col.y, col.z, col.w};
            if(ImGui::ColorPicker4("##pick", f, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)) {
                pal[i] = vmu::rgbaToArgb4444((uint8_t)(f[0] * 255), (uint8_t)(f[1] * 255), (uint8_t)(f[2] * 255), (uint8_t)(f[3] * 255));
                changed = true;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
        if(i % 8 != 7) ImGui::SameLine();
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::TextWrapped("Click a colour to paint with it; right-click it to change it (4 bits a channel, as the Dreamcast keeps it).");
    ImGui::PopStyleColor();
    return changed;
}

// ---------------------------------------------------------------- look: theme, fonts, little drawings

static ImFont* g_fontBold = nullptr;
static ImFont* g_fontTitle = nullptr;

namespace col {
static const ImU32 accent = IM_COL32(226, 108, 52, 255);       // warm orange
static const ImU32 accentDark = IM_COL32(190, 82, 32, 255);
static const ImU32 ink = IM_COL32(43, 41, 38, 255);
static const ImU32 soft = IM_COL32(138, 132, 123, 255);
static const ImU32 card = IM_COL32(255, 255, 255, 255);
static const ImU32 cardHover = IM_COL32(253, 244, 236, 255);
static const ImU32 cardSel = IM_COL32(252, 226, 208, 255);
static const ImU32 line = IM_COL32(226, 221, 213, 255);
static const ImU32 ok = IM_COL32(66, 160, 96, 255);
static const ImU32 off = IM_COL32(170, 165, 158, 255);
static const ImU32 warn = IM_COL32(232, 170, 40, 255);
}  // namespace col

static ImVec4 v4(ImU32 c) { return ImGui::ColorConvertU32ToFloat4(c); }

static void applyTheme() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImGui::StyleColorsLight();
    s.WindowPadding = ImVec2(14, 12);
    s.FramePadding = ImVec2(10, 6);
    s.ItemSpacing = ImVec2(10, 8);
    s.ItemInnerSpacing = ImVec2(8, 6);
    s.FrameRounding = 6;
    s.ChildRounding = 10;
    s.PopupRounding = 8;
    s.GrabRounding = 6;
    s.TabRounding = 8;
    s.WindowRounding = 10;
    s.ScrollbarRounding = 8;
    s.ScrollbarSize = 12;
    s.ChildBorderSize = 1;
    s.FrameBorderSize = 0;
    s.TabBarBorderSize = 0;
    ImVec4* c = s.Colors;
    c[ImGuiCol_Text] = v4(col::ink);
    c[ImGuiCol_TextDisabled] = v4(col::soft);
    c[ImGuiCol_WindowBg] = ImVec4(0.957f, 0.945f, 0.925f, 1);
    c[ImGuiCol_ChildBg] = ImVec4(1, 1, 1, 1);
    c[ImGuiCol_PopupBg] = ImVec4(1, 1, 1, 1);
    c[ImGuiCol_Border] = v4(col::line);
    c[ImGuiCol_FrameBg] = ImVec4(0.945f, 0.930f, 0.905f, 1);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.925f, 0.905f, 0.875f, 1);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.905f, 0.880f, 0.845f, 1);
    c[ImGuiCol_Button] = ImVec4(0.925f, 0.905f, 0.875f, 1);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.985f, 0.880f, 0.800f, 1);
    c[ImGuiCol_ButtonActive] = ImVec4(0.965f, 0.800f, 0.690f, 1);
    c[ImGuiCol_Header] = v4(col::cardSel);
    c[ImGuiCol_HeaderHovered] = v4(col::cardHover);
    c[ImGuiCol_HeaderActive] = v4(col::cardSel);
    c[ImGuiCol_CheckMark] = v4(col::accent);
    c[ImGuiCol_SliderGrab] = v4(col::accent);
    c[ImGuiCol_SliderGrabActive] = v4(col::accentDark);
    c[ImGuiCol_Tab] = ImVec4(0.925f, 0.905f, 0.875f, 1);
    c[ImGuiCol_TabHovered] = ImVec4(0.985f, 0.880f, 0.800f, 1);
    c[ImGuiCol_TabSelected] = ImVec4(1, 1, 1, 1);
    c[ImGuiCol_TabSelectedOverline] = v4(col::accent);
    c[ImGuiCol_TabDimmedSelected] = ImVec4(1, 1, 1, 1);
    c[ImGuiCol_Separator] = v4(col::line);
    c[ImGuiCol_PlotHistogram] = v4(col::accent);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.82f, 0.79f, 0.75f, 1);
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2f, 0.18f, 0.15f, 0.35f);
    c[ImGuiCol_DragDropTarget] = v4(col::accent);
    c[ImGuiCol_MenuBarBg] = ImVec4(1, 1, 1, 1);
}

static bool primaryButton(const char* label, ImVec2 size = ImVec2(0, 0)) {
    ImGui::PushStyleColor(ImGuiCol_Button, v4(col::accent));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, v4(IM_COL32(236, 128, 76, 255)));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, v4(col::accentDark));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    bool r = ImGui::Button(label, size);
    ImGui::PopStyleColor(4);
    return r;
}

static void heading(const char* text) {
    ImGui::PushFont(g_fontBold);
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
}

static void hint(const char* text) {
    ImGui::PushStyleColor(ImGuiCol_Text, v4(col::soft));
    ImGui::TextWrapped("%s", text);
    ImGui::PopStyleColor();
}

static void pill(const char* text, ImU32 color) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 ts = ImGui::CalcTextSize(text);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 sz(ts.x + 22, ts.y + 6);
    dl->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), (color & 0x00FFFFFF) | 0x30000000, sz.y / 2);
    dl->AddCircleFilled(ImVec2(p.x + 9, p.y + sz.y / 2), 3.5f, color);
    dl->AddText(ImVec2(p.x + 16, p.y + 3), col::ink, text);
    ImGui::Dummy(sz);
}

// little drawings for the sources (no icon font needed)
static void glyphVmu(ImDrawList* dl, ImVec2 p, float s, ImU32 c) {
    dl->AddRectFilled(ImVec2(p.x + s * .18f, p.y), ImVec2(p.x + s * .82f, p.y + s), c, s * .14f);
    dl->AddRectFilled(ImVec2(p.x + s * .28f, p.y + s * .12f), ImVec2(p.x + s * .72f, p.y + s * .45f), IM_COL32(205, 222, 196, 255), 2);
    dl->AddCircleFilled(ImVec2(p.x + s * .36f, p.y + s * .70f), s * .08f, IM_COL32(255, 255, 255, 220));
    dl->AddCircleFilled(ImVec2(p.x + s * .64f, p.y + s * .66f), s * .08f, IM_COL32(255, 255, 255, 220));
}

static void glyphFolder(ImDrawList* dl, ImVec2 p, float s, ImU32 c) {
    dl->AddRectFilled(ImVec2(p.x, p.y + s * .15f), ImVec2(p.x + s * .45f, p.y + s * .35f), c, 2);
    dl->AddRectFilled(ImVec2(p.x, p.y + s * .27f), ImVec2(p.x + s, p.y + s * .9f), c, 3);
}

static void glyphChip(ImDrawList* dl, ImVec2 p, float s, ImU32 c) {  // the VM2's own memory
    dl->AddRectFilled(ImVec2(p.x + s * .2f, p.y + s * .2f), ImVec2(p.x + s * .8f, p.y + s * .8f), c, 3);
    for(int i = 0; i < 3; ++i) {
        float y = p.y + s * (.3f + i * .2f);
        dl->AddLine(ImVec2(p.x + s * .05f, y), ImVec2(p.x + s * .2f, y), c, 2);
        dl->AddLine(ImVec2(p.x + s * .8f, y), ImVec2(p.x + s * .95f, y), c, 2);
    }
}

static void glyphCard(ImDrawList* dl, ImVec2 p, float s, ImU32 c) {  // an SD card
    ImVec2 pts[5] = {ImVec2(p.x + s * .2f, p.y + s * .05f), ImVec2(p.x + s * .65f, p.y + s * .05f), ImVec2(p.x + s * .82f, p.y + s * .22f),
                     ImVec2(p.x + s * .82f, p.y + s * .95f), ImVec2(p.x + s * .2f, p.y + s * .95f)};
    dl->AddConvexPolyFilled(pts, 5, c);
}

// A row in the sources list: a drawing, a label, a grey second line; returns true when clicked
static bool sourceRow(const char* id, void (*glyph)(ImDrawList*, ImVec2, float, ImU32), ImU32 gc, const std::string& label,
                      const std::string& sub, bool selected, float indent = 0) {
    ImGui::PushID(id);
    float h = sub.empty() ? 30.f : 42.f;
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    bool clicked = ImGui::InvisibleButton("row", ImVec2(w, h));
    bool hov = ImGui::IsItemHovered();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if(selected || hov) dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), selected ? col::cardSel : col::cardHover, 6);
    glyph(dl, ImVec2(p.x + 8 + indent, p.y + (h - 20) / 2), 20, gc);
    float tx = p.x + 38 + indent;
    if(sub.empty()) dl->AddText(ImVec2(tx, p.y + (h - ImGui::GetTextLineHeight()) / 2), col::ink, label.c_str());
    else {
        dl->AddText(ImVec2(tx, p.y + 4), col::ink, label.c_str());
        dl->AddText(ImVec2(tx, p.y + 22), col::soft, sub.c_str());
    }
    ImGui::PopID();
    return clicked;
}

// ---------------------------------------------------------------- messages (small notices, bottom right)

struct Toast {
    std::string text;
    double until;
    bool bad;
};
static std::vector<Toast> g_toasts;

static void drawToasts(float w, float h) {
    double now = ImGui::GetTime();
    g_toasts.erase(std::remove_if(g_toasts.begin(), g_toasts.end(), [now](const Toast& t) { return t.until < now; }), g_toasts.end());
    float y = h - 16;
    for(int i = (int)g_toasts.size() - 1; i >= 0; --i) {
        Toast& t = g_toasts[i];
        ImVec2 ts = ImGui::CalcTextSize(t.text.c_str(), nullptr, false, 420);
        ImVec2 sz(ts.x + 32, ts.y + 20);
        y -= sz.y;
        ImGui::SetNextWindowPos(ImVec2(w - sz.x - 16, y));
        ImGui::SetNextWindowSize(sz);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, t.bad ? v4(IM_COL32(120, 40, 30, 240)) : v4(IM_COL32(43, 41, 38, 235)));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        char id[32];
        snprintf(id, sizeof id, "##toast%d", i);
        ImGui::Begin(id, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing |
                                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoSavedSettings);
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 420);
        ImGui::TextUnformatted(t.text.c_str());
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleColor(2);
        y -= 8;
    }
}

// ---------------------------------------------------------------- the inspector (the selected file)

static void drawIconTab(Doc& d) {
    Editor& e = d.ed;
    if(e.isVms) {
        if(e.frame >= (int)e.h.icons.size()) e.frame = 0;
        if(e.h.icons.empty()) { hint("This file has no icon."); return; }
        vmu::Icon& ic = e.h.icons[e.frame];
        uint32_t px[1024];
        iconRgba(ic, e.h.palette, px);
        float zoom = std::max(4.f, std::min(9.f, std::floor((ImGui::GetContentRegionAvail().x - 160) / 32)));
        if(canvas("icon", px, zoom, [&](int x, int y, int b) {
               if(b == 0) ic.set(x, y, (uint8_t)e.color);
               else e.color = ic.at(x, y);
           }))
            e.dirty = true;
        ImGui::SameLine();
        ImGui::BeginGroup();
        heading("As it plays");
        int speedMs = std::max<int>(1, e.h.animSpeed) * 1000 / 30;
        int f = (int)(SDL_GetTicks() / speedMs) % (int)e.h.icons.size();
        uint32_t prev[1024];
        iconRgba(e.h.icons[f], e.h.palette, prev);
        ImGui::Image((ImTextureID)(intptr_t)texFor("preview/" + d.title, prev), ImVec2(64, 64));
        ImGui::Spacing();
        heading("Frames");
        for(int i = 0; i < (int)e.h.icons.size(); ++i) {
            char l[8];
            snprintf(l, sizeof l, "%d", i + 1);
            if(i) ImGui::SameLine();
            bool sel = e.frame == i;
            if(sel) ImGui::PushStyleColor(ImGuiCol_Button, v4(col::cardSel));
            if(ImGui::Button(l, ImVec2(34, 0))) e.frame = i;
            if(sel) ImGui::PopStyleColor();
        }
        if(!e.file.isGame()) {
            int frames = (int)e.h.icons.size();
            ImGui::BeginDisabled(frames >= 3);
            if(ImGui::Button("+")) { e.h.icons.push_back(e.h.icons.back()); e.frame = frames; e.dirty = true; }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(frames <= 1);
            if(ImGui::Button("-")) { e.h.icons.erase(e.h.icons.begin() + e.frame); e.frame = 0; e.dirty = true; }
            ImGui::EndDisabled();
        }
        int speed = e.h.animSpeed;
        ImGui::SetNextItemWidth(120);
        if(ImGui::SliderInt("speed", &speed, 1, 60)) { e.h.animSpeed = (uint16_t)speed; e.dirty = true; }
        ImGui::EndGroup();
        ImGui::Spacing();
        heading("Colours");
        if(paletteEditor(e.h.palette, e.color)) e.dirty = true;
        hint("Left-click the picture to paint, right-click to pick up a colour.");
        if(ImGui::Button("Load a picture (PNG)...")) {
            browse("A picture for the icon (32x32, or 32 wide a frame)", false, false, {".png"}, "", [&d](const std::string& p) {
                Bytes f;
                png::Rgba img;
                if(!readFile(p, f) || !png::load(f, img) || img.h != 32 || img.w < 32) { status("An icon picture is 32 pixels high and 32 wide for each frame."); return; }
                Editor& e = d.ed;
                int n = std::min(img.w / 32, e.file.isGame() ? (int)e.h.icons.size() : 3);
                if(!e.file.isGame()) e.h.icons.resize(n);
                for(int i = 0; i < n && i < (int)e.h.icons.size(); ++i) pngToIcon(img, i, e.h.icons[i], e.h.palette, i == 0);
                e.dirty = true;
                status("The picture is in. Apply it to keep it.");
            });
        }
        ImGui::SameLine();
        if(ImGui::Button("Save as a picture...")) {
            browse("Save the icon as a picture", true, false, {".png"}, e.file.name + ".png", [&d](const std::string& p) {
                Editor& e = d.ed;
                png::Rgba img;
                img.w = 32 * (int)e.h.icons.size();
                img.h = 32;
                img.px.resize((size_t)img.w * 32);
                for(size_t i = 0; i < e.h.icons.size(); ++i) {
                    uint32_t px[1024];
                    iconRgba(e.h.icons[i], e.h.palette, px);
                    for(int y = 0; y < 32; ++y) memcpy(&img.px[(size_t)y * img.w + i * 32], &px[y * 32], 128);
                }
                status(writeFile(p, png::save(img)) ? "Saved " + p : "Couldn't write " + p);
            });
        }
    } else if(e.isIconData) {
        uint32_t px[1024];
        ImGui::BeginGroup();
        heading("On the VMU (1-bit)");
        monoRgba(e.icon.mono, px);
        if(canvas("mono", px, 6.f, [&](int x, int y, int b) { e.icon.mono.set(x, y, b == 0); })) e.dirty = true;
        hint("Left: dark. Right: light.");
        ImGui::EndGroup();
        ImGui::SameLine(0, 18);
        ImGui::BeginGroup();
        heading("On the Dreamcast (colour)");
        if(e.icon.hasColor) {
            iconRgba(e.icon.color, e.icon.palette, px);
            if(canvas("color", px, 6.f, [&](int x, int y, int b) {
                   if(b == 0) e.icon.color.set(x, y, (uint8_t)e.color);
                   else e.color = e.icon.color.at(x, y);
               }))
                e.dirty = true;
        } else if(ImGui::Button("Give it a colour icon")) { e.icon.hasColor = true; e.dirty = true; }
        ImGui::EndGroup();
        if(e.icon.hasColor) {
            heading("Colours");
            if(paletteEditor(e.icon.palette, e.color)) e.dirty = true;
        }
        if(ImGui::Button("Make the 1-bit one from the colour one") && e.icon.hasColor) {
            for(int y = 0; y < 32; ++y)
                for(int x = 0; x < 32; ++x) {
                    uint16_t c = e.icon.palette[e.icon.color.at(x, y)];
                    int a = (c >> 12) & 15, l = ((c >> 8) & 15) * 3 + ((c >> 4) & 15) * 6 + (c & 15);
                    e.icon.mono.set(x, y, a >= 8 && l < 75);
                }
            e.dirty = true;
        }
        if(ImGui::Button("Load a picture for the colour one...")) {
            browse("A 32x32 picture", false, false, {".png"}, "", [&d](const std::string& p) {
                Bytes f;
                png::Rgba img;
                if(!readFile(p, f) || !png::load(f, img) || img.w < 32 || img.h != 32) { status("The icon is a 32x32 picture."); return; }
                d.ed.icon.hasColor = true;
                pngToIcon(img, 0, d.ed.icon.color, d.ed.icon.palette, true);
                d.ed.dirty = true;
            });
        }
        ImGui::SameLine();
        if(ImGui::Button("...for the 1-bit one")) {
            browse("A 32x32 picture (dark pixels become dark)", false, false, {".png"}, "", [&d](const std::string& p) {
                Bytes f;
                png::Rgba img;
                if(!readFile(p, f) || !png::load(f, img) || img.w < 32 || img.h != 32) { status("The icon is a 32x32 picture."); return; }
                for(int y = 0; y < 32; ++y)
                    for(int x = 0; x < 32; ++x) {
                        uint32_t c = img.px[(size_t)y * img.w + x];
                        int l = (int)(c & 255) * 3 + (int)((c >> 8) & 255) * 6 + (int)((c >> 16) & 255);
                        d.ed.icon.mono.set(x, y, (c >> 24) >= 128 && l < 128 * 10);
                    }
                d.ed.dirty = true;
            });
        }
    } else hint("This file has no icon (it's plain data).");
}

static void extractFile(Doc& d, const vmu::File& f, bool dci);

static void drawInspector(Doc& d, int index) {
    Editor& e = d.ed;
    if(e.entry < 0) {
        ImGui::Dummy(ImVec2(0, 40));
        hint("Pick a file to see it here.\n\nDrag a file onto another VMU's tab to copy it there.");
        return;
    }
    // the file's face: its icon, big, and its name
    {
        Bytes b = d.img.read(e.file);
        if(SDL_Texture* t = fileIcon(d, e.file)) ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(64, 64));
        else ImGui::Dummy(ImVec2(64, 64));
        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::PushFont(g_fontTitle);
        ImGui::TextUnformatted(e.file.name.c_str());
        ImGui::PopFont();
        std::string what = e.file.isIconData() ? "This VMU's own icon" : e.file.isGame() ? "A VMU program (GAME)" : "A save file";
        ImGui::TextDisabled("%s  -  %d blocks  -  %s", what.c_str(), e.file.blocks, e.file.stamp.text().c_str());
        ImGui::EndGroup();
    }
    ImGui::Spacing();
    if(ImGui::BeginTabBar("insp")) {
        auto tabFlag = [](int n) { return (g_shotMode && g_shotTab == n && ImGui::GetFrameCount() < 3) ? ImGuiTabItemFlags_SetSelected : 0; };
        if(ImGui::BeginTabItem("Details", nullptr, tabFlag(0))) {
            ImGui::Spacing();
            ImGui::TextDisabled("Name on the VMU");
            ImGui::SetNextItemWidth(-1);
            if(ImGui::InputText("##name", &e.name, ImGuiInputTextFlags_CharsUppercase)) { if(e.name.size() > 12) e.name.resize(12); e.dirty = true; }
            if(e.isVms) {
                ImGui::TextDisabled("Shown on the VMU (16 letters)");
                ImGui::SetNextItemWidth(-1);
                if(ImGui::InputText("##vmu", &e.h.vmuDesc)) { if(e.h.vmuDesc.size() > 16) e.h.vmuDesc.resize(16); e.dirty = true; }
                ImGui::TextDisabled("Shown on the Dreamcast (32 letters)");
                ImGui::SetNextItemWidth(-1);
                if(ImGui::InputText("##dc", &e.h.dcDesc)) { if(e.h.dcDesc.size() > 32) e.h.dcDesc.resize(32); e.dirty = true; }
                ImGui::TextDisabled("Made by");
                ImGui::SetNextItemWidth(-1);
                if(ImGui::InputText("##by", &e.h.creator)) { if(e.h.creator.size() > 16) e.h.creator.resize(16); e.dirty = true; }
            } else if(e.isIconData) {
                ImGui::TextDisabled("Description");
                ImGui::SetNextItemWidth(-1);
                if(ImGui::InputText("##icd", &e.icon.desc)) { if(e.icon.desc.size() > 16) e.icon.desc.resize(16); e.dirty = true; }
            }
            ImGui::Spacing();
            if(ImGui::Checkbox("Copy protected", &e.copyProtected)) e.dirty = true;
            ImGui::SameLine();
            ImGui::TextDisabled("(the Dreamcast won't copy it)");
            ImGui::EndTabItem();
        }
        if((e.isVms || e.isIconData) && ImGui::BeginTabItem("Icon", nullptr, tabFlag(1))) {
            ImGui::Spacing();
            drawIconTab(d);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Take it out", nullptr, tabFlag(2))) {
            ImGui::Spacing();
            hint("Save this file on the PC, to keep it or load it into another VMU or an emulator.");
            if(ImGui::Button("Save as .VMS + .VMI...", ImVec2(-1, 0))) extractFile(d, e.file, false);
            if(ImGui::Button("Save as .DCI (Nexus)...", ImVec2(-1, 0))) extractFile(d, e.file, true);
            ImGui::Spacing();
            ImGui::BeginDisabled(g_docs.size() < 2);
            if(ImGui::BeginCombo("##copyto", "Copy to another open VMU...")) {
                for(int i = 0; i < (int)g_docs.size(); ++i) {
                    if(i == index) continue;
                    if(ImGui::Selectable((g_docs[i]->title + "##" + std::to_string(i)).c_str())) {
                        std::string err;
                        if(g_docs[i]->img.add(e.file.name, e.file.type, d.img.read(e.file), e.file.copyProtected, e.file.stamp, &err))
                            status("Copied " + e.file.name + " to " + g_docs[i]->title + ". Save that VMU to keep it.");
                        else status("Couldn't copy it: " + err, true);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::EndDisabled();
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, v4(IM_COL32(170, 50, 40, 255)));
            if(ImGui::Button("Delete from this VMU", ImVec2(-1, 0))) {
                std::string name = e.file.name;
                Doc* dp = &d;
                ask("Delete " + name + "?", "It comes off this VMU when you save. (Saving keeps a backup of the old VMU.)", [dp, name](const std::string&) {
                    for(auto& ff : dp->img.files()) if(ff.name == name) dp->img.remove(ff, nullptr);
                    dp->selected = -1;
                    loadEditor(*dp);
                    status(name + " deleted. Save to make it stick.");
                });
            }
            ImGui::PopStyleColor();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    bool changed = e.dirty || e.name != e.file.name || e.copyProtected != e.file.copyProtected;
    if(changed) {
        ImGui::Spacing();
        ImGui::Separator();
        if(primaryButton("Apply changes", ImVec2(150, 0))) applyEditor(d);
        ImGui::SameLine();
        if(ImGui::Button("Undo", ImVec2(90, 0))) loadEditor(d);
    }
}

// ---------------------------------------------------------------- file actions

static void extractFile(Doc& d, const vmu::File& f, bool dci) {
    std::string base = f.name;
    for(auto& c : base) if(!isalnum((unsigned char)c) && c != '_' && c != '-') c = '_';
    browse(dci ? "Save as .DCI" : "Save as .VMS (and its .VMI)", true, false, {dci ? ".dci" : ".vms"},
           base + (dci ? ".DCI" : ".VMS"), [&d, f, dci](const std::string& p) {
               Bytes b = d.img.read(f);
               if(dci) { status(writeFile(p, vmu::buildDci(f, b)) ? "Saved " + p : "Couldn't write " + p, !fs::exists(fs::u8path(p))); return; }
               std::string vmi = fs::u8path(p).replace_extension(".VMI").u8string();
               std::string res = upper(fs::u8path(p).stem().u8string()).substr(0, 8);
               vmu::VmsHeader h;
               std::string desc = vmu::parseVms(b, f.headerOffset, h) ? h.dcDesc : f.name;
               bool ok = writeFile(p, b) && writeFile(vmi, vmu::buildVmi(f, res, desc, (uint32_t)b.size()));
               status(ok ? "Saved " + p + " and its .VMI" : "Couldn't write " + p, !ok);
           });
}

struct Import {
    bool active = false;
    Bytes data;
    std::string name;
    bool game = false, prot = false;
    vmu::Stamp stamp;
    int doc = -1;
};
static Import g_import;

static bool prepareImport(int docIndex, const std::string& p) {
    Import im;
    im.doc = docIndex;
    im.stamp = vmu::Stamp::now();
    std::string ext = upper(fs::u8path(p).extension().u8string());
    std::string err;
    if(ext == ".DCI") {
        Bytes f;
        vmu::File meta;
        if(!readFile(p, f) || !vmu::parseDci(f, meta, im.data, &err)) { status(p + ": " + err, true); return false; }
        im.name = meta.name; im.game = meta.isGame(); im.prot = meta.copyProtected; im.stamp = meta.stamp;
    } else {
        std::string vms = p;
        vmu::Vmi vmi;
        bool haveVmi = false;
        if(ext == ".VMI") {
            Bytes v;
            if(!readFile(p, v) || !vmu::parseVmi(v, vmi)) { status(p + " isn't a .VMI file", true); return false; }
            haveVmi = true;
            vms = (fs::u8path(p).parent_path() / fs::u8path(vmi.resource + ".VMS")).u8string();
            if(!fs::exists(fs::u8path(vms))) vms = fs::u8path(p).replace_extension(".VMS").u8string();
        } else {
            for(const char* e : {".VMI", ".vmi"}) {
                Bytes v;
                if(readFile(fs::u8path(p).replace_extension(e).u8string(), v) && vmu::parseVmi(v, vmi)) { haveVmi = true; break; }
            }
        }
        if(!readFile(vms, im.data)) { status("Can't read " + vms, true); return false; }
        if(haveVmi) { im.name = vmi.vmuName; im.game = vmi.game; im.prot = vmi.copyProtected; im.stamp = vmi.stamp; }
        else im.name = upper(fs::u8path(vms).stem().u8string()).substr(0, 12);
    }
    im.active = true;
    g_import = im;
    return true;
}

static void importInto(int docIndex) {
    browse("A file to put on the VMU (.VMS, .VMI or .DCI)", false, false, {".vms", ".vmi", ".dci"}, "",
           [docIndex](const std::string& p) { prepareImport(docIndex, p); });
}

static void drawImport() {
    if(!g_import.active) return;
    ImGui::OpenPopup("Put it on the VMU");
    if(!ImGui::BeginPopupModal("Put it on the VMU", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::Text("%u blocks", (unsigned)((g_import.data.size() + 511) / 512));
    ImGui::TextDisabled("Name on the VMU");
    ImGui::SetNextItemWidth(260);
    ImGui::InputText("##imname", &g_import.name, ImGuiInputTextFlags_CharsUppercase);
    if(g_import.name.size() > 12) g_import.name.resize(12);
    ImGui::Checkbox("It's a VMU program (GAME)", &g_import.game);
    ImGui::Checkbox("Copy protected", &g_import.prot);
    ImGui::Spacing();
    if(primaryButton("Put it on", ImVec2(120, 0))) {
        std::string err;
        Doc* d = g_import.doc >= 0 && g_import.doc < (int)g_docs.size() ? g_docs[g_import.doc].get() : nullptr;
        if(d && d->img.add(g_import.name, g_import.game ? vmu::kTypeGame : vmu::kTypeData, g_import.data, g_import.prot,
                           g_import.stamp, &err)) {
            selectByName(*d, g_import.name);
            status(g_import.name + " is on the VMU. Save to keep it.");
        } else status("Couldn't put it on: " + err, true);
        g_import.active = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if(ImGui::Button("Cancel", ImVec2(100, 0))) { g_import.active = false; ImGui::CloseCurrentPopup(); }
    ImGui::EndPopup();
}

static void saveTo(Doc& d, const std::string& path) {
    Bytes old;
    if(readFile(path, old)) backup(fs::u8path(path).filename().u8string(), old);
    if(!writeFile(path, d.img.raw)) { status("Couldn't write " + path, true); return; }
    d.src = Src::PcFile;
    d.path = path;
    d.title = fs::u8path(path).filename().u8string();
    d.original = d.img.raw;
    status("Saved " + path);
}

static void saveDoc(Doc& d, bool saveAs) {
    if(saveAs || d.src == Src::New) {
        browse("Save the VMU on this PC", true, false, {".bin", ".vmu"}, d.title.find('.') == std::string::npos ? d.title + ".bin" : d.title,
               [&d](const std::string& p) { saveTo(d, p); });
        return;
    }
    if(d.src == Src::PcFile) { saveTo(d, d.path); return; }
    Bytes data = d.img.raw;
    std::string label = d.src == Src::Vm2Sd ? "Saving " + d.title + " to the VM2's SD card" : "Saving the VM2's own memory";
    backup((d.src == Src::Vm2Sd ? "VM2SD_" + d.path : std::string("VM2_internal")), d.original);
    Src src = d.src;
    std::string path = d.path;
    Doc* dp = &d;
    runJob(label, [data, src, path](Job& j, std::string* err) {
        return src == Src::Vm2Sd ? g_vm2.writeFile(path, data, j.progress(), err) : g_vm2.writeInternal(data, j.progress(), err);
    }, [dp, data](Job& j) {
        for(auto& doc : g_docs) if(doc.get() == dp && j.ok) { dp->original = data; status("Saved on the VM2"); }
    });
}

// ---------------------------------------------------------------- a VMU: its memory, its files

static ImU32 fileColor(const vmu::File& f) {
    if(f.isGame()) return IM_COL32(226, 108, 52, 255);
    if(f.isIconData()) return IM_COL32(120, 160, 200, 255);
    static const ImU32 tones[] = {IM_COL32(96, 170, 130, 255), IM_COL32(150, 120, 200, 255), IM_COL32(220, 160, 60, 255),
                                  IM_COL32(90, 150, 210, 255), IM_COL32(210, 110, 150, 255), IM_COL32(130, 170, 70, 255)};
    return tones[fnv(f.name.data(), f.name.size()) % 6];
}

// The 200 blocks as the Dreamcast thinks of them: a cell each, coloured by the file in it
static void drawMemoryMap(Doc& d, const std::vector<vmu::File>& files) {
    std::vector<int> owner(256, -1);
    for(int i = 0; i < (int)files.size(); ++i)
        for(int b : d.img.chain(files[i].first)) if(b < 256) owner[b] = i;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float avail = ImGui::GetContentRegionAvail().x;
    int cols = 50;
    float cell = std::min(12.f, (avail - 4) / cols);
    float gap = 2;
    int hover = -1;
    ImVec2 m = ImGui::GetMousePos();
    for(int b = 0; b < 200; ++b) {
        int cx = b % cols, cy = b / cols;
        ImVec2 a(p.x + cx * cell, p.y + cy * cell), z(a.x + cell - gap, a.y + cell - gap);
        ImU32 c = owner[b] >= 0 ? fileColor(files[owner[b]]) : IM_COL32(236, 232, 225, 255);
        dl->AddRectFilled(a, z, c, 2);
        if(owner[b] >= 0 && files[owner[b]].entry == d.selected) dl->AddRect(ImVec2(a.x - 1, a.y - 1), ImVec2(z.x + 1, z.y + 1), col::ink, 2, 0, 1.5f);
        if(m.x >= a.x && m.x < z.x && m.y >= a.y && m.y < z.y) hover = b;
    }
    ImGui::Dummy(ImVec2(cols * cell, 4 * cell));
    if(hover >= 0 && ImGui::IsWindowHovered()) {
        if(owner[hover] >= 0) ImGui::SetTooltip("Block %d: %s", hover, files[owner[hover]].name.c_str());
        else ImGui::SetTooltip("Block %d: free", hover);
        if(owner[hover] >= 0 && ImGui::IsMouseClicked(0)) { d.selected = files[owner[hover]].entry; loadEditor(d); }
    }
}

struct DragFile {
    int doc;
    char name[16];
};

static void fileCard(Doc& d, int index, const vmu::File& f, float w) {
    ImGui::PushID(f.entry);
    float h = 144;
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton("card", ImVec2(w, h));
    bool hov = ImGui::IsItemHovered();
    bool sel = d.selected == f.entry;
    if(clicked) { d.selected = f.entry; loadEditor(d); }
    if(ImGui::BeginDragDropSource()) {
        DragFile df{index, {}};
        snprintf(df.name, sizeof df.name, "%s", f.name.c_str());
        ImGui::SetDragDropPayload("VMUFILE", &df, sizeof df);
        ImGui::Text("Copy %s to...", f.name.c_str());
        ImGui::EndDragDropSource();
    }
    if(ImGui::BeginPopupContextItem("cardmenu")) {
        d.selected = f.entry;
        if(d.ed.entry != f.entry) loadEditor(d);
        if(ImGui::MenuItem("Save as .VMS + .VMI...")) extractFile(d, f, false);
        if(ImGui::MenuItem("Save as .DCI...")) extractFile(d, f, true);
        if(ImGui::MenuItem("Delete")) {
            std::string name = f.name;
            Doc* dp = &d;
            ask("Delete " + name + "?", "It comes off this VMU when you save.", [dp, name](const std::string&) {
                for(auto& ff : dp->img.files()) if(ff.name == name) dp->img.remove(ff, nullptr);
                dp->selected = -1;
                loadEditor(*dp);
            });
        }
        ImGui::EndPopup();
    }
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), sel ? col::cardSel : hov ? col::cardHover : col::card, 10);
    dl->AddRect(p, ImVec2(p.x + w, p.y + h), sel ? col::accent : col::line, 10, 0, sel ? 2.f : 1.f);
    dl->AddRectFilled(ImVec2(p.x + 10, p.y + h - 6), ImVec2(p.x + w - 10, p.y + h - 3), fileColor(f), 2);
    if(SDL_Texture* t = fileIcon(d, f)) dl->AddImage((ImTextureID)(intptr_t)t, ImVec2(p.x + (w - 64) / 2, p.y + 10), ImVec2(p.x + (w + 64) / 2, p.y + 74));
    // the name, centred, and what it is
    auto centred = [&](const std::string& s, float y, ImU32 c, ImFont* font) {
        ImGui::PushFont(font);
        std::string t = s;
        while(!t.empty() && ImGui::CalcTextSize(t.c_str()).x > w - 12) t.pop_back();
        ImVec2 ts = ImGui::CalcTextSize(t.c_str());
        dl->AddText(ImVec2(p.x + (w - ts.x) / 2, y), c, t.c_str());
        ImGui::PopFont();
    };
    centred(f.name, p.y + 80, col::ink, g_fontBold);
    vmu::VmsHeader hh;
    std::string sub = f.isIconData() ? "this VMU's icon" : vmu::parseVms(d.img.read(f), f.headerOffset, hh) ? hh.vmuDesc : "";
    centred(sub, p.y + 99, col::ink, ImGui::GetFont());
    char blocks[32];
    snprintf(blocks, sizeof blocks, "%s%d block%s", f.isGame() ? "program, " : "", f.blocks, f.blocks == 1 ? "" : "s");
    centred(blocks, p.y + 118, col::soft, ImGui::GetFont());
    ImGui::PopID();
}

static void drawDoc(Doc& d, int index) {
    auto files = d.img.files();
    std::sort(files.begin(), files.end(), [](const vmu::File& x, const vmu::File& y) {
        if(x.isGame() != y.isGame()) return x.isGame();
        if(x.isIconData() != y.isIconData()) return x.isIconData();
        return x.name < y.name;
    });
    // the header
    ImGui::PushFont(g_fontTitle);
    ImGui::TextUnformatted(d.title.c_str());
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
    ImGui::TextDisabled("%s", d.where().c_str());
    int used = d.img.usedBlocks();
    ImGui::Text("%d of 200 blocks used", used);
    ImGui::SameLine();
    ImGui::TextDisabled("(%d free)", 200 - used);
    ImGui::SameLine(0, 24);
    uint8_t r, g, b, a;
    d.img.colorRgba(r, g, b, a);
    float c4[4] = {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
    bool custom = d.img.customColor();
    if(ImGui::Checkbox("Colour in the Dreamcast's menu", &custom)) d.img.setColor(custom, r, g, b, a);
    if(custom) {
        ImGui::SameLine();
        if(ImGui::ColorEdit4("##vcol", c4, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreviewHalf))
            d.img.setColor(true, (uint8_t)(c4[0] * 255), (uint8_t)(c4[1] * 255), (uint8_t)(c4[2] * 255), (uint8_t)(c4[3] * 255));
    }
    drawMemoryMap(d, files);

    // unsaved changes
    if(d.dirty()) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + w, p.y + 44), IM_COL32(253, 236, 200, 255), 8);
        ImGui::SetCursorScreenPos(ImVec2(p.x + 12, p.y + 7));
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("This VMU has changes that aren't saved yet.");
        ImGui::SameLine();
        std::string saveLabel = d.src == Src::Vm2Sd ? "Save to the VM2's SD card" : d.src == Src::Vm2Internal ? "Save to the VM2" : "Save";
        if(primaryButton(saveLabel.c_str())) saveDoc(d, false);
        ImGui::SameLine();
        if(ImGui::Button("Undo all")) { d.img.raw = d.original; d.selected = -1; loadEditor(d); }
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 52));
    }
    // actions
    if(ImGui::Button("Put a file on it...")) importInto(index);
    ImGui::SameLine();
    if(ImGui::Button("Save a copy on this PC...")) saveDoc(d, true);
    bool hasIconData = false;
    for(auto& f : files) if(f.isIconData()) hasIconData = true;
    if(!hasIconData) {
        ImGui::SameLine();
        if(ImGui::Button("Give this VMU an icon")) {
            vmu::IconData id;
            id.desc = "MOEMON VMU";
            id.palette[0] = 0xF8CA;
            id.palette[1] = 0xF233;
            for(int i = 0; i < 32; ++i) {
                id.mono.set(i, 0, true); id.mono.set(i, 31, true); id.mono.set(0, i, true); id.mono.set(31, i, true);
                id.color.set(i, 0, 1); id.color.set(i, 31, 1); id.color.set(0, i, 1); id.color.set(31, i, 1);
            }
            std::string err;
            if(d.img.add(vmu::kIconDataName, vmu::kTypeData, vmu::buildIconData(id), false, vmu::Stamp::now(), &err)) selectByName(d, vmu::kIconDataName);
            else status(err, true);
        }
        ImGui::SetItemTooltip("The icon the Dreamcast shows for this VMU (ICONDATA_VMS)");
    }
    ImGui::Spacing();
    // the files as cards, the inspector beside them
    float inspW = std::min(500.f, std::max(380.f, ImGui::GetContentRegionAvail().x * 0.42f));
    ImGui::BeginChild("cards", ImVec2(ImGui::GetContentRegionAvail().x - inspW - 10, 0), ImGuiChildFlags_None);
    if(files.empty()) {
        ImGui::Dummy(ImVec2(0, 30));
        hint("This VMU is empty.\n\nPut a file on it, drop .VMS / .DCI files onto this window, or drag a file here from another VMU.");
    }
    float cw = 150, gap = 12;
    int per = std::max(1, (int)((ImGui::GetContentRegionAvail().x + gap) / (cw + gap)));
    for(int i = 0; i < (int)files.size(); ++i) {
        if(i % per) ImGui::SameLine(0, gap);
        fileCard(d, index, files[i], cw);
    }
    ImGui::Dummy(ImVec2(0, 20));
    ImGui::EndChild();
    if(ImGui::BeginDragDropTarget()) {  // a file from another VMU dropped here
        if(const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("VMUFILE")) {
            DragFile df;
            memcpy(&df, pl->Data, sizeof df);
            if(df.doc != index && df.doc >= 0 && df.doc < (int)g_docs.size()) {
                Doc& from = *g_docs[df.doc];
                for(auto& f : from.img.files())
                    if(f.name == df.name) {
                        std::string err;
                        if(d.img.add(f.name, f.type, from.img.read(f), f.copyProtected, f.stamp, &err)) { selectByName(d, f.name); status("Copied " + f.name + ". Save to keep it."); }
                        else status("Couldn't copy it: " + err, true);
                    }
            }
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::SameLine(0, 10);
    ImGui::BeginChild("inspector", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    drawInspector(d, index);
    ImGui::EndChild();
}

// ---------------------------------------------------------------- the sources (left)

static std::vector<std::string> g_pcFolders;

static bool isVmuImageFile(const fs::directory_entry& e) {
    std::error_code ec;
    return e.is_regular_file(ec) && e.file_size(ec) == (uintmax_t)vmu::kImageSize;
}

static void openPcImage(const std::string& path) {
    Bytes b;
    std::string err;
    if(!readFile(path, b)) { status("Can't read " + path, true); return; }
    if(!openDoc(fs::u8path(path).filename().u8string(), Src::PcFile, path, b, &err)) status(path + ": " + err, true);
}

static bool isOpen(Src src, const std::string& path) {
    Doc* d = activeDoc();
    return d && d->src == src && d->path == path;
}

static void drawPcTree(const fs::path& dir, int depth) {
    std::error_code ec;
    std::vector<fs::directory_entry> items;
    for(auto& e : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) items.push_back(e);
    std::sort(items.begin(), items.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        std::error_code e1, e2;
        if(a.is_directory(e1) != b.is_directory(e2)) return a.is_directory(e1);
        return a.path().filename() < b.path().filename();
    });
    for(auto& e : items) {
        std::string n = e.path().filename().u8string();
        std::error_code e2;
        if(e.is_directory(e2)) {
            if(depth > 6 || n.empty() || n[0] == '.') continue;
            std::string title;
            Bytes t;
            if(readFile((e.path() / "title.txt").u8string(), t) || readFile((e.path() / "TITLE.TXT").u8string(), t))
                for(uint8_t c : t) { if(!c || c == '\r' || c == '\n') break; title += (char)c; }
            std::string label = (title.empty() ? n : title + "  (" + n + ")") + "##" + e.path().u8string();
            if(ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                drawPcTree(e.path(), depth + 1);
                ImGui::TreePop();
            }
        } else if(isVmuImageFile(e)) {
            std::string p = e.path().u8string();
            if(sourceRow(p.c_str(), glyphVmu, IM_COL32(120, 160, 200, 255), n, "", isOpen(Src::PcFile, p), 0)) openPcImage(p);
        }
    }
}

static void openSdImage(const std::string& full, const std::string& name) {
    auto data = std::make_shared<Bytes>();
    runJob("Opening " + name + " from the VM2", [data, full](Job& j, std::string* err) {
        return g_vm2.readFile(full, vmu::kImageSize, *data, j.progress(), err);
    }, [data, full, name](Job& j) {
        if(!j.ok) return;
        std::string err;
        if(!openDoc(name, Src::Vm2Sd, full, *data, &err)) status(full + ": " + err, true);
    });
}

static void sdFolder(const std::string& path);

static void sdEntryMenus(const vm2::Entry& e, const std::string& path, const std::string& full) {
    if(!ImGui::BeginPopupContextItem("sdmenu")) return;
    if(e.dir) {
        if(ImGui::MenuItem("Put a VMU from this PC in here...")) {
            browse("A VMU image to put on the SD card", false, false, {".bin", ".vmu"}, "", [full](const std::string& p) {
                Bytes b;
                if(!readFile(p, b) || b.size() != (size_t)vmu::kImageSize) { status("A VMU image is exactly 128 KB.", true); return; }
                std::string dst = sdJoin(full, upper(fs::u8path(p).filename().u8string()));
                runJob("Copying to the SD card", [b, dst](Job& j, std::string* err) { return g_vm2.writeFile(dst, b, j.progress(), err); },
                       [full](Job& j) { if(j.ok) { g_sd.erase(full); status("Copied to the SD card"); } });
            });
        }
        if(!g_sdClipboard.empty() && ImGui::MenuItem("Paste here")) {
            std::string src = g_sdClipboard, dst = sdJoin(full, g_sdClipboard.substr(g_sdClipboard.rfind('\\') + 1));
            runJob("Copying", [src, dst](Job&, std::string* err) { return g_vm2.copyFile(src, dst, err); }, [full](Job& j) { if(j.ok) g_sd.erase(full); });
        }
        if(ImGui::MenuItem("Rename...")) {
            ask("Rename the folder", "A new name: 1-8 letters or numbers, no spaces.", [full](const std::string& n) {
                std::string to = "\\" + upper(n);
                runJob("Renaming", [full, to](Job&, std::string* err) { return g_vm2.rename(full, to, err); }, [](Job& j) { if(j.ok) g_sd.clear(); });
            }, true, e.name);
        }
        if(ImGui::MenuItem("Delete the folder")) {
            ask("Delete " + e.name + "?", "Only an empty folder can be deleted.", [full](const std::string&) {
                runJob("Deleting", [full](Job&, std::string* err) { return g_vm2.removeFolder(full, err); }, [](Job& j) { if(j.ok) g_sd.clear(); });
            });
        }
    } else {
        if(ImGui::MenuItem("Open")) openSdImage(full, e.name);
        if(ImGui::MenuItem("Save a copy on this PC...")) {
            browse("Save the VMU on this PC", true, false, {".bin", ".vmu"}, e.name, [full](const std::string& p) {
                auto data = std::make_shared<Bytes>();
                runJob("Copying from the VM2", [data, full](Job& j, std::string* err) { return g_vm2.readFile(full, vmu::kImageSize, *data, j.progress(), err); },
                       [data, p](Job& j) { if(j.ok) status(writeFile(p, *data) ? "Saved " + p : "Couldn't write " + p); });
            });
        }
        if(ImGui::MenuItem("Copy (then Paste in a folder)")) g_sdClipboard = full;
        if(ImGui::MenuItem("Rename...")) {
            ask("Rename", "A new name, like VMU002.BIN (8 letters, a dot, 3 letters; no spaces).", [full, path](const std::string& n) {
                std::string to = sdJoin(path, upper(n));
                runJob("Renaming", [full, to](Job&, std::string* err) { return g_vm2.rename(full, to, err); }, [path](Job& j) { if(j.ok) g_sd.erase(path); });
            }, true, e.name);
        }
        if(ImGui::MenuItem("Delete")) {
            ask("Delete " + e.name + "?", "It comes off the SD card. A copy goes into the backups folder first.", [full, path](const std::string&) {
                auto data = std::make_shared<Bytes>();
                runJob("Deleting " + full, [data, full](Job& j, std::string* err) {
                    if(!g_vm2.readFile(full, vmu::kImageSize, *data, j.progress(), err)) return false;
                    backup("VM2SD_" + full, *data);
                    return g_vm2.removeFile(full, err);
                }, [path](Job& j) { if(j.ok) g_sd.erase(path); });
            });
        }
    }
    ImGui::EndPopup();
}

static void sdFolder(const std::string& path) {
    SdFolder& sf = g_sd[path];
    if(!sf.loaded) {
        if(!busy()) sdRefresh(path);
        ImGui::TextDisabled("    reading...");
        return;
    }
    for(auto& e : sf.entries) {
        std::string full = sdJoin(path, e.name);
        ImGui::PushID(full.c_str());
        if(e.dir) {
            std::string label = (e.title.empty() ? e.name : e.title) + "##dir";
            bool open = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
            if(!e.title.empty()) ImGui::SetItemTooltip("%s", e.name.c_str());
            sdEntryMenus(e, path, full);
            if(open) {
                sdFolder(full);
                ImGui::TreePop();
            }
        } else if(e.size == (uint32_t)vmu::kImageSize) {
            if(sourceRow("img", glyphVmu, IM_COL32(120, 160, 200, 255), e.name, "", isOpen(Src::Vm2Sd, full), 0)) openSdImage(full, e.name);
            sdEntryMenus(e, path, full);
        }
        ImGui::PopID();
    }
    if(sf.entries.empty()) ImGui::TextDisabled("    (empty)");
}

static void drawVm2Section() {
    heading("VM2 by USB");
    ImGui::SameLine();
    if(g_vm2.connected()) pill("connected", col::ok);
    else pill("not connected", col::off);
    if(!g_vm2.connected()) {
        static int lastScan = -100000;
        if(g_frameNo - lastScan > 180) { g_ports = vm2::listPorts(); lastScan = g_frameNo; }
        bool found = false;
        for(auto& p : g_ports) found |= p.ch340;
        if(primaryButton(busy() ? "Connecting..." : "Connect to the VM2", ImVec2(-1, 34)) && !busy()) vm2Connect();
        hint(found ? "A VM2 is plugged in. On it: Settings > System > GUI-mode, then Connect."
                   : "Plug the VM2 in with its USB-C cable and put it in GUI-mode (Settings > System > GUI-mode). Close DreamMods' VM2 GUI if it's open.");
        if(ImGui::TreeNode("Port")) {
            std::string cur = g_portPick == 0 ? "Find it by itself" : (g_portPick <= (int)g_ports.size() ? g_ports[g_portPick - 1].description : "?");
            ImGui::SetNextItemWidth(-1);
            if(ImGui::BeginCombo("##port", cur.c_str())) {
                if(ImGui::Selectable("Find it by itself", g_portPick == 0)) g_portPick = 0;
                for(int i = 0; i < (int)g_ports.size(); ++i)
                    if(ImGui::Selectable((g_ports[i].description + "##" + g_ports[i].name).c_str(), g_portPick == i + 1)) g_portPick = i + 1;
                ImGui::EndCombo();
            }
            ImGui::TreePop();
        }
        return;
    }
    ImGui::TextDisabled("%s  -  firmware %s", g_vm2.port.c_str(), g_vm2.fw.c_str());
    if(sourceRow("internal", glyphChip, IM_COL32(110, 110, 120, 255), "Its own memory", "the 200 blocks inside the VM2", isOpen(Src::Vm2Internal, "internal"))) {
        auto data = std::make_shared<Bytes>();
        runJob("Opening the VM2's own memory", [data](Job& j, std::string* err) { return g_vm2.readInternal(*data, j.progress(), err); },
               [data](Job& j) {
                   if(!j.ok) return;
                   std::string err;
                   if(!openDoc("VM2 memory", Src::Vm2Internal, "internal", *data, &err)) status("The VM2's memory: " + err, true);
               });
    }
    bool open = ImGui::TreeNodeEx("##sdroot", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "SD card");
    if(ImGui::BeginPopupContextItem("sdroot")) {
        if(ImGui::MenuItem("New folder...")) {
            ask("New folder", "A name: 1-8 letters or numbers, no spaces.", [](const std::string& n) {
                std::string p = "\\" + upper(n);
                runJob("Making " + p, [p](Job&, std::string* err) { return g_vm2.makeFolder(p, err); }, [](Job& j) { if(j.ok) g_sd.erase("\\"); });
            }, true, "MOEMON");
        }
        if(ImGui::MenuItem("Put a VMU from this PC on it...")) {
            browse("A VMU image to put on the SD card", false, false, {".bin", ".vmu"}, "", [](const std::string& p) {
                Bytes b;
                if(!readFile(p, b) || b.size() != (size_t)vmu::kImageSize) { status("A VMU image is exactly 128 KB.", true); return; }
                std::string dst = "\\" + upper(fs::u8path(p).filename().u8string());
                runJob("Copying to the SD card", [b, dst](Job& j, std::string* err) { return g_vm2.writeFile(dst, b, j.progress(), err); },
                       [](Job& j) { if(j.ok) { g_sd.erase("\\"); status("Copied to the SD card"); } });
            });
        }
        if(!g_sdClipboard.empty() && ImGui::MenuItem("Paste here")) {
            std::string src = g_sdClipboard, dst = "\\" + g_sdClipboard.substr(g_sdClipboard.rfind('\\') + 1);
            runJob("Copying", [src, dst](Job&, std::string* err) { return g_vm2.copyFile(src, dst, err); }, [](Job& j) { if(j.ok) g_sd.erase("\\"); });
        }
        if(ImGui::MenuItem("Refresh")) g_sd.clear();
        ImGui::EndPopup();
    }
    if(open) {
        sdFolder("\\");
        ImGui::TreePop();
    }
    if(ImGui::SmallButton("Disconnect")) { g_vm2.disconnect(); g_sd.clear(); }
}

static void drawSources() {
    drawVm2Section();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    heading("On this PC");
    if(ImGui::Button("Open an SD card or folder...", ImVec2(-1, 0)))
        browse("Pick the SD card (its drive) or a folder", false, true, {}, "", [](const std::string& p) {
            if(std::find(g_pcFolders.begin(), g_pcFolders.end(), p) == g_pcFolders.end()) g_pcFolders.push_back(p);
        });
    if(ImGui::Button("Open a VMU file...", ImVec2(-1, 0)))
        browse("A VMU image (.bin or .vmu, 128 KB)", false, false, {".bin", ".vmu"}, "", [](const std::string& p) { openPcImage(p); });
    if(ImGui::Button("New empty VMU", ImVec2(-1, 0))) {
        std::string err;
        vmu::Image blank;
        openDoc("NEW VMU " + std::to_string(g_docs.size() + 1), Src::New, "", blank.raw, &err);
    }
    for(size_t i = 0; i < g_pcFolders.size(); ++i) {
        ImGui::PushID((int)i);
        std::string name = fs::u8path(g_pcFolders[i]).filename().u8string();
        if(name.empty()) name = g_pcFolders[i];
        bool open = ImGui::TreeNodeEx("##pcroot", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", name.c_str());
        ImGui::SetItemTooltip("%s", g_pcFolders[i].c_str());
        bool closed = false;
        if(ImGui::BeginPopupContextItem("pcmenu")) {
            if(ImGui::MenuItem("Stop showing this folder")) closed = true;
            ImGui::EndPopup();
        }
        if(open) {
            drawPcTree(fs::u8path(g_pcFolders[i]), 0);
            ImGui::TreePop();
        }
        ImGui::PopID();
        if(closed) { g_pcFolders.erase(g_pcFolders.begin() + i); break; }
    }
}

// ---------------------------------------------------------------- the window

static void drawWelcome() {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + avail.x / 2 - 220, ImGui::GetCursorPosY() + avail.y / 2 - 170));
    ImGui::BeginGroup();
    ImVec2 p = ImGui::GetCursorScreenPos();
    glyphVmu(ImGui::GetWindowDrawList(), ImVec2(p.x + 170, p.y), 100, col::accent);
    ImGui::Dummy(ImVec2(440, 112));
    ImGui::PushFont(g_fontTitle);
    ImGui::TextUnformatted("Your VMUs, all in one place");
    ImGui::PopFont();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + 440);
    hint("Open one from the VM2 (connect it by USB on the left), from its SD card in a card reader, or from this PC. "
         "Changes stay here until you save, and everything you overwrite is backed up first.");
    ImGui::PopTextWrapPos();
    ImGui::Spacing();
    if(primaryButton("Connect to the VM2", ImVec2(210, 38)) && !busy()) vm2Connect();
    ImGui::SameLine();
    if(ImGui::Button("Open an SD card or folder...", ImVec2(220, 38)))
        browse("Pick the SD card (its drive) or a folder", false, true, {}, "", [](const std::string& p) {
            if(std::find(g_pcFolders.begin(), g_pcFolders.end(), p) == g_pcFolders.end()) g_pcFolders.push_back(p);
        });
    ImGui::EndGroup();
}

static void drawUi(int w, int h) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
    ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    // the title strip
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        glyphVmu(ImGui::GetWindowDrawList(), p, 26, col::accent);
        ImGui::Dummy(ImVec2(28, 26));
        ImGui::SameLine();
        ImGui::PushFont(g_fontTitle);
        ImGui::TextUnformatted(kTitle);
        ImGui::PopFont();
        if(g_job) {
            ImGui::SameLine(0, 30);
            int t = g_job->total, dn = g_job->done;
            char ov[200];
            snprintf(ov, sizeof ov, "%s  %d%%", g_job->label.c_str(), t ? dn * 100 / t : 0);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
            ImGui::ProgressBar(t ? (float)dn / t : -1.f * (float)ImGui::GetTime(), ImVec2(-1, 22), ov);
        }
    }
    ImGui::Spacing();
    ImGui::BeginChild("sources", ImVec2(310, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_AlwaysUseWindowPadding);
    drawSources();
    ImGui::EndChild();
    ImGui::SameLine(0, 12);
    ImGui::BeginChild("docs", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding);
    if(g_docs.empty()) drawWelcome();
    else if(ImGui::BeginTabBar("docs", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll)) {
        int closeIndex = -1;
        for(int i = 0; i < (int)g_docs.size(); ++i) {
            Doc& d = *g_docs[i];
            bool keep = true;
            std::string label = d.title + (d.dirty() ? " *" : "") + "###doc" + std::to_string((intptr_t)&d);
            ImGuiTabItemFlags fl = (g_shotMode && i == g_active) ? ImGuiTabItemFlags_SetSelected : 0;
            if(d.dirty()) fl |= ImGuiTabItemFlags_UnsavedDocument;
            bool vis = ImGui::BeginTabItem(label.c_str(), &keep, fl);
            if(ImGui::BeginDragDropTarget()) {  // a file card dropped on another VMU's tab: copy it there
                if(const ImGuiPayload* pl = ImGui::AcceptDragDropPayload("VMUFILE")) {
                    DragFile df;
                    memcpy(&df, pl->Data, sizeof df);
                    if(df.doc != i && df.doc >= 0 && df.doc < (int)g_docs.size())
                        for(auto& f : g_docs[df.doc]->img.files())
                            if(f.name == df.name) {
                                std::string err;
                                if(d.img.add(f.name, f.type, g_docs[df.doc]->img.read(f), f.copyProtected, f.stamp, &err))
                                    status("Copied " + f.name + " to " + d.title + ". Save it to keep it.");
                                else status("Couldn't copy it: " + err, true);
                            }
                }
                ImGui::EndDragDropTarget();
            }
            if(vis) {
                g_active = i;
                ImGui::Spacing();
                drawDoc(d, i);
                ImGui::EndTabItem();
            }
            if(!keep) closeIndex = i;
        }
        ImGui::EndTabBar();
        if(closeIndex >= 0) {
            Doc* d = g_docs[closeIndex].get();
            auto close = [d](const std::string&) {
                for(size_t i = 0; i < g_docs.size(); ++i) if(g_docs[i].get() == d) { g_docs.erase(g_docs.begin() + i); break; }
                g_active = (int)g_docs.size() - 1;
            };
            if(d->dirty()) ask("Close " + d->title + "?", "Its changes aren't saved and will be lost.", close);
            else close("");
        }
    }
    ImGui::EndChild();
    drawBrowser();
    drawAsk();
    drawImport();
    ImGui::End();
    drawToasts((float)w, (float)h);
}

// files dropped on the window from Windows Explorer / the desktop
static void dropped(const std::string& path) {
    std::string ext = upper(fs::u8path(path).extension().u8string());
    std::error_code ec;
    if(fs::is_directory(fs::u8path(path), ec)) {
        if(std::find(g_pcFolders.begin(), g_pcFolders.end(), path) == g_pcFolders.end()) g_pcFolders.push_back(path);
    } else if(ext == ".VMS" || ext == ".VMI" || ext == ".DCI") {
        if(!activeDoc()) { status("Open a VMU first, then drop the file onto it.", true); return; }
        prepareImport(g_active, path);
    } else openPcImage(path);
}

static int selftest() {
    int fails = 0;
    auto check = [&](bool ok, const char* what) {
        printf("%s %s\n", ok ? "ok  " : "FAIL", what);
        if(!ok) ++fails;
    };
    vmu::Image img;
    std::string err;
    check(img.formatted() && img.freeBlocks() == 200, "a blank VMU has 200 free blocks");
    Bytes game(3072, 0xAB);
    check(img.add("LINKTEST", vmu::kTypeGame, game, false, vmu::Stamp::now(), &err) && img.files()[0].first == 0, "a GAME goes at block 0");
    check(!img.add("SECOND", vmu::kTypeGame, game, false, vmu::Stamp::now(), &err), "only one GAME");
    // a data file with a VMS header, two icons, some data
    vmu::VmsHeader h;
    h.vmuDesc = "TEST SAVE"; h.dcDesc = "A TEST SAVE FILE"; h.creator = "MOEMON";
    h.icons.resize(2);
    h.palette[1] = 0xFF00;
    h.icons[1].set(3, 4, 1);
    Bytes data(128 + 1024 + 700, 0);
    h.dataBytes = 700;
    for(int i = 0; i < 700; ++i) data[128 + 1024 + i] = (uint8_t)i;
    vmu::writeVmsInPlace(data, 0, h, true);
    check(img.add("SAVE1", vmu::kTypeData, data, true, vmu::Stamp::now(), &err), "a data file goes in");
    const vmu::File* f = img.find("SAVE1");
    check(f && f->blocks == 4 && f->first == 199, "data from the top (block 199), 4 blocks");
    vmu::VmsHeader back;
    check(f && vmu::parseVms(img.read(*f), 0, back) && back.vmuDesc == "TEST SAVE" && back.icons.size() == 2 &&
          back.icons[1].at(3, 4) == 1 && back.crc == vmu::vmsCrc(img.read(*f), 0, back), "its header, icons and CRC read back");
    // three icons now: the data moves
    vmu::VmsHeader more = back;
    more.icons.push_back(more.icons[0]);
    Bytes rebuilt = vmu::rebuildData(img.read(*f), back, more);
    check(img.replace(*img.find("SAVE1"), rebuilt, &err), "a third icon (the file grows)");
    vmu::VmsHeader b3;
    Bytes r3 = img.read(*img.find("SAVE1"));
    check(vmu::parseVms(r3, 0, b3) && b3.icons.size() == 3 && r3[128 + 1536 + 699] == (uint8_t)699 &&
          b3.crc == vmu::vmsCrc(r3, 0, b3), "the data moved after the icons, CRC fresh");
    // VMI / DCI round trips
    vmu::File sf = *img.find("SAVE1");
    vmu::Vmi vmi;
    check(vmu::parseVmi(vmu::buildVmi(sf, "SAVE1", "A TEST SAVE FILE", (uint32_t)r3.size()), vmi) && vmi.vmuName == "SAVE1" &&
          vmi.copyProtected && !vmi.game, ".VMI round trip");
    vmu::File meta;
    Bytes body;
    check(vmu::parseDci(vmu::buildDci(sf, r3), meta, body, &err) && body == r3 && meta.name == "SAVE1", ".DCI round trip");
    // ICONDATA_VMS
    vmu::IconData id;
    id.desc = "MY VMU";
    id.mono.set(5, 6, true);
    id.color.set(7, 8, 9);
    id.palette[9] = 0xF0F0;
    vmu::IconData id2;
    check(vmu::parseIconData(vmu::buildIconData(id), id2) && id2.mono.at(5, 6) && id2.color.at(7, 8) == 9 && id2.palette[9] == 0xF0F0,
          "ICONDATA_VMS round trip");
    check(img.remove(*img.find("SAVE1"), &err) && img.freeBlocks() == 194, "delete frees its blocks");
    check(img.rename(*img.find("LINKTEST"), "LINK2", &err) && img.find("LINK2"), "rename");
    // PNG
    png::Rgba p;
    p.w = 32; p.h = 32; p.px.assign(1024, 0xFF0000FF);
    png::Rgba q;
    check(png::load(png::save(p), q) && q.w == 32 && q.px[100] == 0xFF0000FF, "PNG round trip");
    printf(fails ? "FAILED (%d)\n" : "PASSED\n", fails);
    return fails ? 1 : 0;
}

// ---------------------------------------------------------------- main


// ---------------------------------------------------------------- main

static void status(const std::string& s, bool bad) {
    g_toasts.push_back(Toast{s, ImGui::GetCurrentContext() ? ImGui::GetTime() + (bad ? 7.0 : 4.0) : 4.0, bad});
    if(g_toasts.size() > 4) g_toasts.erase(g_toasts.begin());
    fprintf(stderr, "%s\n", s.c_str());
}

int main(int argc, char** argv) {
    std::string shotPath;
    std::vector<std::string> shotImages, startImages;
    for(int i = 1; i < argc; ++i) {
        if(!strcmp(argv[i], "--selftest")) return selftest();
        if(!strcmp(argv[i], "--shot") && i + 1 < argc) { shotPath = argv[++i]; g_shotMode = true; }
        else if(!strcmp(argv[i], "--port") && i + 1 < argc) g_startPort = argv[++i];
        else (g_shotMode ? shotImages : startImages).push_back(argv[i]);
    }
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) { fprintf(stderr, "SDL: %s\n", SDL_GetError()); return 1; }
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    int W = 1366, H = 840;
    SDL_Window* win = SDL_CreateWindow(kTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H,
                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowMinimumSize(win, 1024, 640);
    g_ren = SDL_CreateRenderer(win, -1, g_shotMode ? SDL_RENDERER_SOFTWARE : (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    if(!g_ren) g_ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFontConfig fc;
    fc.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void*)kFontData, (int)sizeof kFontData, 17.f, &fc);
    g_fontBold = io.Fonts->AddFontFromMemoryTTF((void*)kFontBold, (int)sizeof kFontBold, 17.f, &fc);
    g_fontTitle = io.Fonts->AddFontFromMemoryTTF((void*)kFontBold, (int)sizeof kFontBold, 22.f, &fc);
    applyTheme();
    ImGui_ImplSDL2_InitForSDLRenderer(win, g_ren);
    ImGui_ImplSDLRenderer2_Init(g_ren);
    for(auto& p : startImages) dropped(p);
    for(auto& p : shotImages) openPcImage(p);
    if(g_shotMode && activeDoc()) {
        Doc& d = *activeDoc();
        auto fl = d.img.files();
        for(auto& f : fl) if(!f.isGame() && !f.isIconData()) { d.selected = f.entry; break; }
        if(d.selected < 0 && !fl.empty()) d.selected = fl[0].entry;
        loadEditor(d);
        if(getenv("SHOT_TAB")) g_shotTab = atoi(getenv("SHOT_TAB"));
    }
    if(!g_startPort.empty()) {
        std::string err;
        if(g_vm2.connect(g_startPort, &err)) status("VM2 connected on " + g_vm2.port);
        else status(err, true);
    }
    if(g_shotMode && getenv("SHOT_FOLDER")) g_pcFolders.push_back(getenv("SHOT_FOLDER"));
    if(g_shotMode && getenv("SHOT_TOAST")) status(getenv("SHOT_TOAST"));
    bool quit = false;
    int frames = 0;
    while(!quit) {
        SDL_Event ev;
        while(SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if(ev.type == SDL_DROPFILE) { dropped(ev.drop.file); SDL_free(ev.drop.file); }
            if(ev.type == SDL_QUIT) {
                bool unsaved = false;
                for(auto& d : g_docs) unsaved |= d->dirty();
                if(unsaved && !g_ask.active) ask("Quit?", "Some VMUs have changes that aren't saved.", [&quit](const std::string&) { quit = true; });
                else if(!unsaved) quit = true;
            }
        }
        pollJob();
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        SDL_GetRendererOutputSize(g_ren, &W, &H);
        drawUi(W, H);
        ImGui::Render();
        SDL_SetRenderDrawColor(g_ren, 244, 241, 236, 255);
        SDL_RenderClear(g_ren);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), g_ren);
        ++g_frameNo;
        if(g_frameNo % 60 == 0) gcTextures();
        if(g_shotMode && ++frames >= 6 && !busy() && (frames >= 60 || !g_vm2.connected())) {
            png::Rgba img;
            img.w = W; img.h = H; img.px.resize((size_t)W * H);
            SDL_RenderReadPixels(g_ren, nullptr, SDL_PIXELFORMAT_ABGR8888, img.px.data(), W * 4);
            writeFile(shotPath, png::save(img));
            quit = true;
        }
        SDL_RenderPresent(g_ren);
        if(!g_shotMode && !(SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS) && !busy()) SDL_Delay(30);
    }
    if(g_job && g_job->thread.joinable()) g_job->thread.join();
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(g_ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
