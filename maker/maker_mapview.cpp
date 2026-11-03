// Moemon Maker: the MAP panel and the TILES panel (docs/MOEMON_MAKER.md "Modes"). The map's tiles are the game's own,
// loaded and animated by the game (src/maker/maker_tiles.c); each of the 1024 metatiles is drawn once into an atlas
// texture (again when an animation changes tiles), and the map is drawn as squares of that atlas, so any size and zoom
// stays fast. SINNOH's maps have no tile pictures (Platinum's 3D world is drawn over them): their walkable grid is
// shown by what each tile does.
#include <SDL.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "imgui.h"
#include "imgui_internal.h"
#include "maker_ui.h"

namespace maker {

static const int kAtlasCols = 32;   // 1024 metatiles: 32 x 32 of 16x16 pixels
static SDL_Texture *sAtlas;
static MakerTilesInfo sTiles;
static std::vector<int> sBlocks;    // the open map's blocks (metatile | collision << 10 | elevation << 12)
static int sW, sH;
static int sLoadedMap = -1;
static int sSelectedMetatile = 1;
static ImVec2 sScrollTo(-1, -1);    // (tests and opening: the map scrolled so this tile is in the middle)
static std::vector<MakerObjectInfo> sObjs;
static std::vector<MakerWarpInfo> sWarps;
static std::vector<MakerBgInfo> sBgs;
static std::vector<MakerCoordInfo> sCoords;
struct SpritePic { SDL_Texture *tex; int w, h; };
static std::map<int, SpritePic> sSprites;   // by look * 4 + facing

static SpritePic Sprite(int gfx, int facing)
{
    int key = gfx * 4 + facing;
    auto it = sSprites.find(key);
    if (it != sSprites.end())
        return it->second;
    static unsigned int px[64 * 64];
    SpritePic p = {NULL, 0, 0};
    if (MakerApi_RenderObject(gfx, facing, px, 64, 64, &p.w, &p.h))
    {
        p.tex = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, p.w, p.h);
        if (p.tex)
        {
            SDL_UpdateTexture(p.tex, NULL, px, p.w * 4);
            SDL_SetTextureBlendMode(p.tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(p.tex, SDL_ScaleModeNearest);
        }
    }
    sSprites[key] = p;
    return p;
}

static void FreeSprites()
{
    for (auto &kv : sSprites)
        if (kv.second.tex)
            SDL_DestroyTexture(kv.second.tex);
    sSprites.clear();
}

static ImTextureID TexId(SDL_Texture *t)
{
    return (ImTextureID)(intptr_t)t;
}

static void RenderAtlas()
{
    if (!sAtlas)
    {
        sAtlas = SDL_CreateTexture(gEd.ren, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, kAtlasCols * 16, kAtlasCols * 16);
        if (!sAtlas)
            return;
        SDL_SetTextureScaleMode(sAtlas, SDL_ScaleModeNearest);
        SDL_SetTextureBlendMode(sAtlas, SDL_BLENDMODE_NONE);
    }
    void *pixels;
    int pitch;
    if (SDL_LockTexture(sAtlas, NULL, &pixels, &pitch) != 0)
        return;
    unsigned int mt[256];
    for (int id = 0; id < kAtlasCols * kAtlasCols; id++)
    {
        MakerApi_RenderMetatile(id, mt, 3);
        int ax = (id % kAtlasCols) * 16, ay = (id / kAtlasCols) * 16;
        for (int y = 0; y < 16; y++)
            memcpy((char *)pixels + (ay + y) * pitch + ax * 4, &mt[y * 16], 64);
    }
    SDL_UnlockTexture(sAtlas);
}

void MapView_Opened()
{
    sLoadedMap = -1;
}

void MapView_Closed()
{
    if (sAtlas)
        SDL_DestroyTexture(sAtlas);
    sAtlas = NULL;
    FreeSprites();
    sLoadedMap = -1;
    MakerApi_UnloadMapTiles();
}

static void LoadMap()
{
    if (gEd.map == sLoadedMap)
        return;
    sLoadedMap = gEd.map;
    MakerApi_LoadMapTiles(gEd.map, &sTiles);
    sW = sTiles.width;
    sH = sTiles.height;
    sBlocks.assign(sW * sH, 0);
    for (int y = 0; y < sH; y++)
        for (int x = 0; x < sW; x++)
            sBlocks[y * sW + x] = MakerApi_MapBlock(gEd.map, x, y);
    sScrollTo = ImVec2(sW / 2.0f, sH / 2.0f);
    sObjs.resize(MakerApi_ObjectCount(gEd.map));
    for (size_t i = 0; i < sObjs.size(); i++)
        MakerApi_ObjectInfo(gEd.map, (int)i, &sObjs[i]);
    sWarps.resize(MakerApi_WarpCount(gEd.map));
    for (size_t i = 0; i < sWarps.size(); i++)
        MakerApi_WarpInfo(gEd.map, (int)i, &sWarps[i]);
    sBgs.resize(MakerApi_BgCount(gEd.map));
    for (size_t i = 0; i < sBgs.size(); i++)
        MakerApi_BgInfo(gEd.map, (int)i, &sBgs[i]);
    sCoords.resize(MakerApi_CoordCount(gEd.map));
    for (size_t i = 0; i < sCoords.size(); i++)
        MakerApi_CoordInfo(gEd.map, (int)i, &sCoords[i]);
    gEd.selKind = SEL_NONE;
    gEd.selIndex = -1;
}

// An event marker (Events mode): a square with a letter, as Porymap and RPG Maker show them
static void Marker(ImDrawList *dl, ImVec2 p, float s, ImU32 col, const char *letter, bool selected)
{
    float in = s * 0.12f;
    dl->AddRectFilled(ImVec2(p.x + in, p.y + in), ImVec2(p.x + s - in, p.y + s - in), (col & 0x00FFFFFF) | 0x90000000, 2);
    dl->AddRect(ImVec2(p.x + in, p.y + in), ImVec2(p.x + s - in, p.y + s - in), col, 2, 0, selected ? 3.0f : 1.5f);
    ImFont *f = gEd.fontBold;
    float fs = s * 0.5f;
    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, letter);
    dl->AddText(f, fs, ImVec2(p.x + (s - ts.x) / 2, p.y + (s - ts.y) / 2), IM_COL32(255, 255, 255, 255), letter);
}

// The people, and in Events mode the warps, signs and triggers; returns what's at tile (tx, ty) for picking
static void DrawEvents(ImDrawList *dl, ImVec2 origin, float s)
{
    bool events = gEd.mode == MODE_EVENTS;
    if (events)
    {
        for (size_t i = 0; i < sCoords.size(); i++)
            Marker(dl, ImVec2(origin.x + sCoords[i].x * s, origin.y + sCoords[i].y * s), s, IM_COL32(150, 60, 200, 255), "T",
                   gEd.selKind == SEL_COORD && gEd.selIndex == (int)i);
        for (size_t i = 0; i < sBgs.size(); i++)
            Marker(dl, ImVec2(origin.x + sBgs[i].x * s, origin.y + sBgs[i].y * s), s, IM_COL32(200, 40, 120, 255),
                   strstr(sBgs[i].typeName, "hidden") ? "H" : "S", gEd.selKind == SEL_BG && gEd.selIndex == (int)i);
        for (size_t i = 0; i < sWarps.size(); i++)
            Marker(dl, ImVec2(origin.x + sWarps[i].x * s, origin.y + sWarps[i].y * s), s, IM_COL32(230, 120, 20, 255), "W",
                   gEd.selKind == SEL_WARP && gEd.selIndex == (int)i);
    }
    if (!gEd.viewNpcs && !events)
        return;
    for (size_t i = 0; i < sObjs.size(); i++)
    {
        const MakerObjectInfo &o = sObjs[i];
        bool sel = gEd.selKind == SEL_OBJECT && gEd.selIndex == (int)i;
        ImVec2 tile(origin.x + o.x * s, origin.y + o.y * s);
        if (sel && (o.rangeX || o.rangeY))
        {
            // (its walking range)
            ImVec2 a(tile.x - o.rangeX * s, tile.y - o.rangeY * s), b(tile.x + (o.rangeX + 1) * s, tile.y + (o.rangeY + 1) * s);
            dl->AddRectFilled(a, b, IM_COL32(60, 140, 255, 50));
            dl->AddRect(a, b, IM_COL32(60, 140, 255, 220), 0, 0, 2);
        }
        SpritePic p = Sprite(o.graphicsId, o.facing);
        if (p.tex)
        {
            // (feet on the tile, centered: as the field places a person)
            float z = s / 16.0f;
            ImVec2 a(tile.x + (s - p.w * z) / 2, tile.y + s - p.h * z);
            dl->AddImage(TexId(p.tex), a, ImVec2(a.x + p.w * z, a.y + p.h * z));
        }
        else
            Marker(dl, tile, s, IM_COL32(40, 120, 220, 255), "P", sel);
        if (events || sel)
            dl->AddRect(tile, ImVec2(tile.x + s, tile.y + s), sel ? IM_COL32(255, 220, 0, 255) : IM_COL32(40, 120, 220, 220), 0, 0, sel ? 3.0f : 1.5f);
    }
}

// What's at a tile (Events mode picking): people first, then triggers, signs, warps
static bool Pick(int tx, int ty, int *kind, int *index)
{
    for (size_t i = 0; i < sObjs.size(); i++)
        if (sObjs[i].x == tx && sObjs[i].y == ty)
            return *kind = SEL_OBJECT, *index = (int)i, true;
    for (size_t i = 0; i < sWarps.size(); i++)
        if (sWarps[i].x == tx && sWarps[i].y == ty)
            return *kind = SEL_WARP, *index = (int)i, true;
    for (size_t i = 0; i < sBgs.size(); i++)
        if (sBgs[i].x == tx && sBgs[i].y == ty)
            return *kind = SEL_BG, *index = (int)i, true;
    for (size_t i = 0; i < sCoords.size(); i++)
        if (sCoords[i].x == tx && sCoords[i].y == ty)
            return *kind = SEL_COORD, *index = (int)i, true;
    return false;
}

static ImVec2 AtlasUv(int id)
{
    return ImVec2((id % kAtlasCols) / (float)kAtlasCols, (id / kAtlasCols) / (float)kAtlasCols);
}

static void DrawMetatile(ImDrawList *dl, int id, ImVec2 p, float s)
{
    ImVec2 uv0 = AtlasUv(id), uv1(uv0.x + 1.0f / kAtlasCols, uv0.y + 1.0f / kAtlasCols);
    dl->AddImage(TexId(sAtlas), p, ImVec2(p.x + s, p.y + s), uv0, uv1);
}

// SINNOH: a tile shown by what it does (walkable, grass, water, ledge...), the collision as a darker square
static ImU32 SinnohColor(int block)
{
    int behavior = MakerApi_MetatileBehavior(block & 0x3FF);
    int cls = MakerApi_BehaviorClass(behavior);
    bool blocked = ((block >> 10) & 3) != 0;
    switch (cls)
    {
    case MAKER_BEHAVIOR_GRASS: return IM_COL32(96, 176, 80, 255);
    case MAKER_BEHAVIOR_WATER: return IM_COL32(72, 128, 208, 255);
    case MAKER_BEHAVIOR_WATERFALL: return IM_COL32(120, 170, 240, 255);
    case MAKER_BEHAVIOR_DOOR: case MAKER_BEHAVIOR_WARP: return IM_COL32(220, 120, 40, 255);
    case MAKER_BEHAVIOR_CAVE: return IM_COL32(150, 130, 100, 255);
    case MAKER_BEHAVIOR_ICE: return IM_COL32(190, 230, 250, 255);
    default: break;
    }
    if (cls >= MAKER_BEHAVIOR_LEDGE_E && cls <= MAKER_BEHAVIOR_LEDGE_S)
        return IM_COL32(170, 140, 90, 255);
    return blocked ? IM_COL32(70, 70, 76, 255) : IM_COL32(222, 214, 186, 255);
}

// The small mark drawn over a tile in View > Behavior icons
static void DrawBehaviorIcon(ImDrawList *dl, int cls, ImVec2 p, float s)
{
    float m = s * 0.5f;
    ImVec2 c(p.x + m, p.y + m);
    ImU32 ink = IM_COL32(255, 255, 255, 230), edge = IM_COL32(0, 0, 0, 160);
    float r = s * 0.16f;
    switch (cls)
    {
    case MAKER_BEHAVIOR_GRASS:
    {
        // (three blades, white with a dark edge: seen on any grass)
        float w = s * 0.07f;
        for (int i = -1; i <= 1; i++)
        {
            ImVec2 a(c.x + i * r * 0.9f, c.y + r * 1.1f), b(c.x + i * r * 1.3f, c.y - r * (i ? 0.6f : 1.1f));
            dl->AddLine(a, b, edge, w + 2);
            dl->AddLine(a, b, ink, w);
        }
        break;
    }
    case MAKER_BEHAVIOR_WATER:
    case MAKER_BEHAVIOR_WATERFALL:
        dl->AddBezierQuadratic(ImVec2(c.x - r * 1.5f, c.y), ImVec2(c.x - r * 0.75f, c.y - r), ImVec2(c.x, c.y), IM_COL32(30, 90, 200, 230), s * 0.06f);
        dl->AddBezierQuadratic(ImVec2(c.x, c.y), ImVec2(c.x + r * 0.75f, c.y + r), ImVec2(c.x + r * 1.5f, c.y), IM_COL32(30, 90, 200, 230), s * 0.06f);
        break;
    case MAKER_BEHAVIOR_DOOR:
    case MAKER_BEHAVIOR_WARP:
        dl->AddRectFilled(ImVec2(c.x - r, c.y - r * 1.3f), ImVec2(c.x + r, c.y + r * 1.3f), IM_COL32(230, 120, 30, 230), 1);
        dl->AddRect(ImVec2(c.x - r, c.y - r * 1.3f), ImVec2(c.x + r, c.y + r * 1.3f), edge, 1);
        break;
    case MAKER_BEHAVIOR_LEDGE_E: case MAKER_BEHAVIOR_LEDGE_W: case MAKER_BEHAVIOR_LEDGE_N: case MAKER_BEHAVIOR_LEDGE_S:
    {
        static const ImVec2 kDir[] = {ImVec2(1, 0), ImVec2(-1, 0), ImVec2(0, -1), ImVec2(0, 1)};
        ImVec2 d = kDir[cls - MAKER_BEHAVIOR_LEDGE_E], q(-d.y, d.x);
        ImVec2 tip(c.x + d.x * r * 1.4f, c.y + d.y * r * 1.4f);
        dl->AddTriangleFilled(tip, ImVec2(c.x - d.x * r + q.x * r, c.y - d.y * r + q.y * r),
                              ImVec2(c.x - d.x * r - q.x * r, c.y - d.y * r - q.y * r), ink);
        dl->AddTriangle(tip, ImVec2(c.x - d.x * r + q.x * r, c.y - d.y * r + q.y * r),
                        ImVec2(c.x - d.x * r - q.x * r, c.y - d.y * r - q.y * r), edge);
        break;
    }
    default:
        break;
    }
}

void MapView_Panel()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool open = ImGui::Begin("MAP");
    ImGui::PopStyleVar();
    if (!open)
    {
        ImGui::End();
        return;
    }
    if (gEd.map < 0)
    {
        ImGui::SetCursorPos(ImVec2(10, 10));
        ImGui::TextDisabled("Pick a map in MAPS.");
        ImGui::End();
        return;
    }
    LoadMap();
    if (MakerApi_TakeTilesChanged() || !sAtlas)
        RenderAtlas();
    float s = 16.0f * gEd.zoom;
    ImGui::SetNextWindowContentSize(ImVec2(sW * s + 64, sH * s + 64));
    ImGui::BeginChild("canvas", ImVec2(0, 0), ImGuiChildFlags_None,
                      ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    origin.x += 32;
    origin.y += 32;
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (sScrollTo.x >= 0)
    {
        ImGui::SetScrollX(sScrollTo.x * s + 32 - ImGui::GetWindowWidth() / 2);
        ImGui::SetScrollY(sScrollTo.y * s + 32 - ImGui::GetWindowHeight() / 2);
        sScrollTo = ImVec2(-1, -1);
    }
    // (only the tiles in view)
    ImVec2 clipMin = ImGui::GetWindowPos(), clipMax(clipMin.x + ImGui::GetWindowWidth(), clipMin.y + ImGui::GetWindowHeight());
    int x0 = (int)floorf((clipMin.x - origin.x) / s), y0 = (int)floorf((clipMin.y - origin.y) / s);
    int x1 = (int)ceilf((clipMax.x - origin.x) / s), y1 = (int)ceilf((clipMax.y - origin.y) / s);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > sW) x1 = sW;
    if (y1 > sH) y1 = sH;
    dl->AddRectFilled(ImVec2(origin.x - 1, origin.y - 1), ImVec2(origin.x + sW * s + 1, origin.y + sH * s + 1), IM_COL32(0, 0, 0, 255));
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
        {
            int b = sBlocks[y * sW + x];
            ImVec2 p(origin.x + x * s, origin.y + y * s);
            if (b < 0)
                continue;
            if (sTiles.sinnoh)
                dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s), SinnohColor(b));
            else
                DrawMetatile(dl, b & 0x3FF, p, s);
        }
    if (gEd.viewIcons)
        for (int y = y0; y < y1; y++)
            for (int x = x0; x < x1; x++)
            {
                int b = sBlocks[y * sW + x];
                if (b < 0)
                    continue;
                int cls = MakerApi_BehaviorClass(MakerApi_MetatileBehavior(b & 0x3FF));
                if (cls != MAKER_BEHAVIOR_NONE && cls != MAKER_BEHAVIOR_CAVE && cls != MAKER_BEHAVIOR_COUNTER && cls != MAKER_BEHAVIOR_ICE)
                    DrawBehaviorIcon(dl, cls, ImVec2(origin.x + x * s, origin.y + y * s), s);
            }
    if (gEd.viewGrid && s >= 16)
    {
        ImU32 g = gEd.dark ? IM_COL32(255, 255, 255, 40) : IM_COL32(0, 0, 0, 45);
        for (int x = x0; x <= x1; x++)
            dl->AddLine(ImVec2(origin.x + x * s, origin.y + y0 * s), ImVec2(origin.x + x * s, origin.y + y1 * s), g);
        for (int y = y0; y <= y1; y++)
            dl->AddLine(ImVec2(origin.x + x0 * s, origin.y + y * s), ImVec2(origin.x + x1 * s, origin.y + y * s), g);
    }
    DrawEvents(dl, origin, s);
    // the tile under the mouse, and the screen frame round it (the GBA's 15 x 10 tiles, the player at their middle)
    ImGui::InvisibleButton("map", ImVec2(sW * s + 64, sH * s + 64), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    if (ImGui::IsItemHovered())
    {
        ImVec2 m = ImGui::GetMousePos();
        int tx = (int)floorf((m.x - origin.x) / s), ty = (int)floorf((m.y - origin.y) / s);
        if (tx >= 0 && ty >= 0 && tx < sW && ty < sH)
        {
            gEd.hoverX = tx;
            gEd.hoverY = ty;
            ImVec2 p(origin.x + tx * s, origin.y + ty * s);
            if (gEd.viewFrame)
            {
                ImVec2 f0(p.x - 7 * s, p.y - 4.5f * s), f1(f0.x + 15 * s, f0.y + 10 * s);
                dl->AddRect(f0, f1, IM_COL32(255, 255, 255, 200), 0, 0, 3);
                dl->AddRect(f0, f1, IM_COL32(0, 0, 0, 220), 0, 0, 1);
            }
            dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 255), 0, 0, 2);
            dl->AddRect(ImVec2(p.x - 1, p.y - 1), ImVec2(p.x + s + 1, p.y + s + 1), IM_COL32(0, 0, 0, 255), 0, 0, 1);
            gEd.hoverBlock = sBlocks[ty * sW + tx];
            if (gEd.mode == MODE_EVENTS && ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                int k, i;
                if (Pick(tx, ty, &k, &i))
                    gEd.selKind = k, gEd.selIndex = i;
                else
                    gEd.selKind = SEL_NONE, gEd.selIndex = -1;
            }
        }
        // Ctrl + wheel: zoom round the mouse
        ImGuiIO &io = ImGui::GetIO();
        if (io.KeyCtrl && io.MouseWheel != 0)
        {
            int z = gEd.zoom + (io.MouseWheel > 0 ? 1 : -1);
            if (z >= 1 && z <= 8)
            {
                float fx = (m.x - origin.x) / s, fy = (m.y - origin.y) / s;
                gEd.zoom = z;
                float ns = 16.0f * z;
                ImGui::SetScrollX(fx * ns + 32 - (m.x - ImGui::GetWindowPos().x));
                ImGui::SetScrollY(fy * ns + 32 - (m.y - ImGui::GetWindowPos().y));
            }
        }
    }
    (void)avail;
    ImGui::EndChild();
    ImGui::End();
}

void TilesPanel()
{
    if (!ImGui::Begin("TILES"))
    {
        ImGui::End();
        return;
    }
    if (gEd.map < 0 || sLoadedMap != gEd.map)
    {
        ImGui::TextDisabled("Open a map to see its tiles.");
        ImGui::End();
        return;
    }
    if (sTiles.sinnoh)
    {
        ImGui::TextWrapped("SINNOH's maps are drawn in 3D from Platinum's own models. Here they're edited by what each "
                           "tile does: paint them in Collision mode.");
        ImGui::End();
        return;
    }
    if (!sAtlas)
        RenderAtlas();
    const int cols = 8;
    float s = 32.0f * gEd.uiScale;
    auto section = [&](const char *title, int first, int count) {
        ImGui::SeparatorText(title);
        ImDrawList *dl = ImGui::GetWindowDrawList();
        for (int i = 0; i < count; i++)
        {
            int id = first + i;
            if (i % cols)
                ImGui::SameLine(0, 0);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::PushID(id);
            if (ImGui::InvisibleButton("mt", ImVec2(s, s)))
                sSelectedMetatile = id;
            bool hov = ImGui::IsItemHovered();
            ImGui::PopID();
            DrawMetatile(dl, id, p, s);
            if (id == sSelectedMetatile)
            {
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 255), 0, 0, 3);
                dl->AddRect(ImVec2(p.x - 1, p.y - 1), ImVec2(p.x + s + 1, p.y + s + 1), IM_COL32(0, 90, 200, 255), 0, 0, 2);
            }
            else if (hov)
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 180), 0, 0, 1);
            if (hov)
                ImGui::SetTooltip("Metatile %d (0x%03X)", id, id);
        }
    };
    ImGui::BeginChild("tiles", ImVec2(0, 0), ImGuiChildFlags_None);
    char title[64];
    snprintf(title, sizeof(title), "%s (primary)", sTiles.primaryName);
    section(title, 0, sTiles.primaryMetatiles);
    snprintf(title, sizeof(title), "%s (secondary)", sTiles.secondaryName);
    section(title, sTiles.secondaryStart, sTiles.secondaryMetatiles);
    ImGui::EndChild();
    ImGui::End();
}

int MapView_SelectedMetatile()
{
    return sSelectedMetatile;
}

static void Row(const char *k, const char *fmt, ...)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", k);
    ImGui::TableNextColumn();
    va_list ap;
    va_start(ap, fmt);
    ImGui::TextV(fmt, ap);
    va_end(ap);
}

void MapView_Inspector()
{
    if (gEd.selKind == SEL_NONE || gEd.map != sLoadedMap)
        return;
    ImGui::Spacing();
    ImGui::PushFont(gEd.fontBold);
    static const char *const kKinds[] = {"", "PERSON", "WARP", "SIGN", "TRIGGER"};
    ImGui::SeparatorText(kKinds[gEd.selKind]);
    ImGui::PopFont();
    if (!ImGui::BeginTable("sel", 2, ImGuiTableFlags_SizingFixedFit))
        return;
    int i = gEd.selIndex;
    if (gEd.selKind == SEL_OBJECT && i < (int)sObjs.size())
    {
        const MakerObjectInfo &o = sObjs[i];
        static const char *const kFacing[] = {"south", "north", "west", "east"};
        Row("Name", "%s", o.localIdName[0] ? o.localIdName : "(no name)");
        Row("Look", "%s", o.gfxName);
        Row("At", "%d, %d   elevation %d", o.x, o.y, o.elevation);
        Row("Faces", "%s", kFacing[o.facing & 3]);
        Row("Moves", "type %d, range %d x %d", o.movementType, o.rangeX, o.rangeY);
        if (o.trainerType)
            Row("Trainer", "sees %d tiles", o.sight);
        Row("Script", "%s", o.scriptName);
        Row("Hide flag", o.flag ? "0x%03X" : "(always there)", o.flag);
    }
    else if (gEd.selKind == SEL_WARP && i < (int)sWarps.size())
    {
        const MakerWarpInfo &w = sWarps[i];
        Row("At", "%d, %d", w.x, w.y);
        Row("To", "%s", w.destMap >= 0 ? gEd.maps[w.destMap].title : "?");
        Row("Warp", "%d", w.destWarp);
    }
    else if (gEd.selKind == SEL_BG && i < (int)sBgs.size())
    {
        Row("At", "%d, %d", sBgs[i].x, sBgs[i].y);
        Row("Kind", "%s", sBgs[i].typeName);
        Row("Script", "%s", sBgs[i].scriptName);
    }
    else if (gEd.selKind == SEL_COORD && i < (int)sCoords.size())
    {
        Row("At", "%d, %d", sCoords[i].x, sCoords[i].y);
        Row("When", "%s", sCoords[i].varName);
        Row("Script", "%s", sCoords[i].scriptName);
    }
    ImGui::EndTable();
}

// Tests: "maker center X Y" (the map scrolled to that tile), "maker anim 0|1", "maker pick X Y" (Events mode's click)
void MapView_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    if (c == "center")
    {
        float x = 0, y = 0;
        sscanf(arg, "%f %f", &x, &y);
        sScrollTo = ImVec2(x, y);
    }
    else if (c == "anim")
        MakerApi_SetAnimating(atoi(arg));
    else if (c == "pick")
    {
        int x = 0, y = 0, k, i;
        sscanf(arg, "%d %d", &x, &y);
        if (Pick(x, y, &k, &i))
            gEd.selKind = k, gEd.selIndex = i;
    }
    else if (c == "stats")
    {
        // (the open map's tiles by what they do, and the behaviors seen: checks the MAP panel's marks)
        int counts[16] = {0};
        std::map<int, int> behaviors;
        for (int b : sBlocks)
            if (b >= 0)
            {
                int beh = MakerApi_MetatileBehavior(b & 0x3FF);
                counts[MakerApi_BehaviorClass(beh) & 15]++;
                behaviors[beh]++;
            }
        printf("[maker] stats %s:", gEd.map >= 0 ? gEd.maps[gEd.map].name : "-");
        for (int i = 0; i < 16; i++)
            if (counts[i])
                printf(" class%d=%d", i, counts[i]);
        printf(" | behaviors:");
        for (auto &kv : behaviors)
            printf(" %02X=%d", kv.first, kv.second);
        printf("\n");
    }
}

} // namespace maker
