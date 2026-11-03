// Moemon Maker: the MAP panel, the TILES panel and the map's side of the INSPECTOR and PROBLEMS (docs/MOEMON_MAKER.md
// "Modes"). The map's tiles are the game's own, loaded and animated by the game (src/maker/maker_tiles.c); each of the
// 1024 metatiles is drawn once into an atlas texture (again when an animation changes tiles), and the map is drawn as
// squares of that atlas, so any size and zoom stays fast. SINNOH's maps have no tile pictures (Platinum's 3D world is
// drawn over them): their walkable grid is shown by what each tile does.
//
// Editing (the user's edits: maker_edits.cpp):
//   Map mode        pencil / rectangle / fill / picker with the stamp picked in TILES (drag there for several tiles);
//                   right-drag on the map copies its tiles as the stamp
//   Collision mode  Walkable / Blocked keeping each tile's elevation, or the full collision + elevation grid
//   Events mode     click a person, warp, sign or trigger to pick it, drag to move it; its details in the INSPECTOR
#include <SDL.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <deque>
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
static std::vector<int> sBlocks;    // the open map's own blocks, as built (metatile | collision << 10 | elevation << 12)
static int sW, sH;
static int sLoadedMap = -1;
static ImVec2 sScrollTo(-1, -1);    // (tests and opening: the map scrolled so this tile is in the middle)

// the stamp painted in Map mode: sStampW x sStampH metatiles
static int sStampW = 1, sStampH = 1;
static std::vector<int> sStamp(1, 1);
static int sPalDragFrom = -1;
// Collision mode's brush
static int sCollBrush = 1, sElevBrush = 3;
static bool sCollAdvanced;
// painting on the map
static bool sStroke, sRect, sCopy;
static int sRectX0, sRectY0, sLastX = -1, sLastY = -1;
// dragging an event
static int sDragKind, sDragIndex = -1, sDragDX, sDragDY;
static bool sDragMoved;

// the open map's own people, warps, signs, triggers (as built)
static std::vector<MakerObjectInfo> sObjs;
static std::vector<MakerWarpInfo> sWarps;
static std::vector<MakerBgInfo> sBgs;
static std::vector<MakerCoordInfo> sCoords;

struct SpritePic { SDL_Texture *tex; int w, h; };
static std::map<int, SpritePic> sSprites;   // by look * 4 + facing

static ImTextureID TexId(SDL_Texture *t)
{
    return (ImTextureID)(intptr_t)t;
}

// ---------------------------------------------------------------------------------------------------------------------
// Pictures

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

static ImVec2 AtlasUv(int id)
{
    return ImVec2((id % kAtlasCols) / (float)kAtlasCols, (id / kAtlasCols) / (float)kAtlasCols);
}

static void DrawMetatile(ImDrawList *dl, int id, ImVec2 p, float s, ImU32 tint = IM_COL32_WHITE)
{
    ImVec2 uv0 = AtlasUv(id & 0x3FF), uv1(uv0.x + 1.0f / kAtlasCols, uv0.y + 1.0f / kAtlasCols);
    dl->AddImage(TexId(sAtlas), p, ImVec2(p.x + s, p.y + s), uv0, uv1, tint);
}

// ---------------------------------------------------------------------------------------------------------------------
// The open map

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
    if (gEd.map == sLoadedMap || gEd.map < 0)
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
    sDragIndex = -1;
}

static int Base(int x, int y)
{
    return sBlocks[y * sW + x];
}

static int Cur(int x, int y)
{
    int b = Base(x, y);
    return b < 0 ? b : CurrentBlock(gEd.map, x, y, b);
}

// The keys edits are saved by (maker_overrides.c KeyOf): a LOCALID_ name, else SCRIPT@X,Y at its own first spot
static std::string PersonKey(int i)
{
    const MakerObjectInfo &o = sObjs[i];
    if (o.localIdName[0])
        return o.localIdName;
    char k[160];
    snprintf(k, sizeof(k), "%s@%d,%d", o.scriptName, o.x, o.y);
    return k;
}

static std::string SpotKey(const char *script, int x, int y)
{
    char k[160];
    snprintf(k, sizeof(k), "%s@%d,%d", script, x, y);
    return k;
}

static std::string WarpKey(int i)
{
    return std::to_string(i);
}

static MakerObjectInfo CurPerson(int i)
{
    MakerObjectInfo c = sObjs[i];
    const MakerObjectInfo &b = sObjs[i];
    std::string k = PersonKey(i);
    int m = gEd.map;
    c.x = (int)CurrentProp(m, "person", k, "x", b.x);
    c.y = (int)CurrentProp(m, "person", k, "y", b.y);
    c.elevation = (int)CurrentProp(m, "person", k, "elevation", b.elevation);
    c.graphicsId = (int)CurrentProp(m, "person", k, "look", b.graphicsId);
    c.movementType = (int)CurrentProp(m, "person", k, "movement", b.movementType);
    c.rangeX = (int)CurrentProp(m, "person", k, "range_x", b.rangeX);
    c.rangeY = (int)CurrentProp(m, "person", k, "range_y", b.rangeY);
    c.trainerType = (int)CurrentProp(m, "person", k, "trainer", b.trainerType);
    c.sight = (int)CurrentProp(m, "person", k, "sight", b.sight);
    c.flag = (int)CurrentProp(m, "person", k, "flag", b.flag);
    c.facing = MakerApi_MovementFacing(c.movementType);
    return c;
}

static MakerWarpInfo CurWarp(int i)
{
    MakerWarpInfo c = sWarps[i];
    std::string k = WarpKey(i);
    c.x = (int)CurrentProp(gEd.map, "warp", k, "x", sWarps[i].x);
    c.y = (int)CurrentProp(gEd.map, "warp", k, "y", sWarps[i].y);
    c.elevation = (int)CurrentProp(gEd.map, "warp", k, "elevation", sWarps[i].elevation);
    long dest = CurrentProp(gEd.map, "warp", k, "dest", ((long)sWarps[i].destMap << 8) | sWarps[i].destWarp);
    c.destMap = (int)(dest >> 8);
    c.destWarp = (int)(dest & 0xFF);
    return c;
}

static MakerBgInfo CurBg(int i)
{
    MakerBgInfo c = sBgs[i];
    std::string k = SpotKey(sBgs[i].scriptName, sBgs[i].x, sBgs[i].y);
    c.x = (int)CurrentProp(gEd.map, "sign", k, "x", sBgs[i].x);
    c.y = (int)CurrentProp(gEd.map, "sign", k, "y", sBgs[i].y);
    return c;
}

static MakerCoordInfo CurCoord(int i)
{
    MakerCoordInfo c = sCoords[i];
    std::string k = SpotKey(sCoords[i].scriptName, sCoords[i].x, sCoords[i].y);
    c.x = (int)CurrentProp(gEd.map, "trigger", k, "x", sCoords[i].x);
    c.y = (int)CurrentProp(gEd.map, "trigger", k, "y", sCoords[i].y);
    return c;
}

// An event's kind and key for edits
static void EventKey(int kind, int i, const char **k, std::string *key)
{
    switch (kind)
    {
    case SEL_OBJECT: *k = "person"; *key = PersonKey(i); break;
    case SEL_WARP: *k = "warp"; *key = WarpKey(i); break;
    case SEL_BG: *k = "sign"; *key = SpotKey(sBgs[i].scriptName, sBgs[i].x, sBgs[i].y); break;
    default: *k = "trigger"; *key = SpotKey(sCoords[i].scriptName, sCoords[i].x, sCoords[i].y); break;
    }
}

static void EventPos(int kind, int i, int *x, int *y, int *bx, int *by)
{
    switch (kind)
    {
    case SEL_OBJECT: { MakerObjectInfo c = CurPerson(i); *x = c.x; *y = c.y; *bx = sObjs[i].x; *by = sObjs[i].y; break; }
    case SEL_WARP: { MakerWarpInfo c = CurWarp(i); *x = c.x; *y = c.y; *bx = sWarps[i].x; *by = sWarps[i].y; break; }
    case SEL_BG: { MakerBgInfo c = CurBg(i); *x = c.x; *y = c.y; *bx = sBgs[i].x; *by = sBgs[i].y; break; }
    default: { MakerCoordInfo c = CurCoord(i); *x = c.x; *y = c.y; *bx = sCoords[i].x; *by = sCoords[i].y; break; }
    }
}

static void MoveEvent(int kind, int i, int x, int y)
{
    const char *k;
    std::string key;
    int cx, cy, bx, by;
    if (x < 0 || y < 0 || x >= sW || y >= sH)
        return;
    EventKey(kind, i, &k, &key);
    EventPos(kind, i, &cx, &cy, &bx, &by);
    if (x == cx && y == cy)
        return;
    BeginChange(gEd.map);
    SetProp(gEd.map, k, key, "x", bx, x);
    SetProp(gEd.map, k, key, "y", by, y);
    EndChange();
}

void MapView_EditsChanged(int map)
{
    (void)map;
}

// ---------------------------------------------------------------------------------------------------------------------
// Painting

static void PaintMetatile(int x, int y, int id)
{
    if (x < 0 || y < 0 || x >= sW || y >= sH || Base(x, y) < 0)
        return;
    int cur = Cur(x, y);
    SetBlock(gEd.map, x, y, Base(x, y), (cur & ~0x3FF) | (id & 0x3FF));
}

// The stamp tiled from x0, y0
static int StampAt(int x0, int y0, int x, int y)
{
    int sx = ((x - x0) % sStampW + sStampW) % sStampW, sy = ((y - y0) % sStampH + sStampH) % sStampH;
    return sStamp[sy * sStampW + sx];
}

static void PaintCollision(int x, int y)
{
    if (x < 0 || y < 0 || x >= sW || y >= sH || Base(x, y) < 0)
        return;
    int cur = Cur(x, y);
    int now = (cur & ~0x0C00) | ((sCollBrush & 3) << 10);
    if (sCollAdvanced)
        now = (now & ~0xF000) | ((sElevBrush & 15) << 12);
    SetBlock(gEd.map, x, y, Base(x, y), now);
}

static void PaintAt(int x, int y, int x0, int y0)
{
    if (gEd.mode == MODE_COLLISION)
        PaintCollision(x, y);
    else
        PaintMetatile(x, y, StampAt(x0, y0, x, y));
}

// Fill: the tiles joined to x, y that are the same (the same metatile; in Collision mode the same collision and
// elevation) become the stamp / brush
static void Fill(int x, int y)
{
    if (x < 0 || y < 0 || x >= sW || y >= sH || Base(x, y) < 0)
        return;
    int mask = gEd.mode == MODE_COLLISION ? 0xFC00 : 0x3FF;
    int match = Cur(x, y) & mask;
    std::vector<char> seen(sW * sH, 0);
    std::deque<std::pair<int, int>> q;
    q.push_back({x, y});
    seen[y * sW + x] = 1;
    std::vector<std::pair<int, int>> tiles;
    while (!q.empty() && tiles.size() < 20000)
    {
        std::pair<int, int> t = q.front();
        q.pop_front();
        tiles.push_back(t);
        static const int d[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto &dd : d)
        {
            int nx = t.first + dd[0], ny = t.second + dd[1];
            if (nx < 0 || ny < 0 || nx >= sW || ny >= sH || seen[ny * sW + nx] || Base(nx, ny) < 0)
                continue;
            if ((Cur(nx, ny) & mask) != match)
                continue;
            seen[ny * sW + nx] = 1;
            q.push_back({nx, ny});
        }
    }
    for (auto &t : tiles)
        PaintAt(t.first, t.second, x, y);
}

static void Pick(int x, int y)
{
    int b = Cur(x, y);
    if (b < 0)
        return;
    if (gEd.mode == MODE_COLLISION)
    {
        sCollBrush = (b >> 10) & 3;
        sElevBrush = (b >> 12) & 15;
        return;
    }
    sStampW = sStampH = 1;
    sStamp.assign(1, b & 0x3FF);
}

// ---------------------------------------------------------------------------------------------------------------------
// Drawing

static ImU32 SinnohColor(int block)
{
    int cls = MakerApi_BehaviorClass(MakerApi_MetatileBehavior(block & 0x3FF));
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
        ImVec2 a(c.x - d.x * r + q.x * r, c.y - d.y * r + q.y * r), b(c.x - d.x * r - q.x * r, c.y - d.y * r - q.y * r);
        dl->AddTriangleFilled(tip, a, b, ink);
        dl->AddTriangle(tip, a, b, edge);
        break;
    }
    default:
        break;
    }
}

// Collision mode's colors: blocked red; in the full grid, elevations as hues
static ImU32 CollisionColor(int block)
{
    int c = (block >> 10) & 3, e = (block >> 12) & 15;
    if (sCollAdvanced)
    {
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(e / 16.0f, 0.75f, 0.95f, r, g, b);
        return c ? IM_COL32(200, 30, 30, 150) : ImGui::GetColorU32(ImVec4(r, g, b, 0.45f));
    }
    return c ? IM_COL32(220, 30, 30, 120) : 0;
}

static void Marker(ImDrawList *dl, ImVec2 p, float s, ImU32 col, const char *letter, bool selected)
{
    float in = s * 0.12f;
    dl->AddRectFilled(ImVec2(p.x + in, p.y + in), ImVec2(p.x + s - in, p.y + s - in), (col & 0x00FFFFFF) | 0x90000000, 2);
    dl->AddRect(ImVec2(p.x + in, p.y + in), ImVec2(p.x + s - in, p.y + s - in), selected ? IM_COL32(255, 220, 0, 255) : col, 2, 0,
                selected ? 3.0f : 1.5f);
    ImFont *f = gEd.fontBold;
    float fs = s * 0.5f;
    ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, letter);
    dl->AddText(f, fs, ImVec2(p.x + (s - ts.x) / 2, p.y + (s - ts.y) / 2), IM_COL32(255, 255, 255, 255), letter);
}

static void DrawPerson(ImDrawList *dl, const MakerObjectInfo &o, ImVec2 tile, float s, bool sel, bool events, ImU32 tint)
{
    if (sel && (o.rangeX || o.rangeY))
    {
        ImVec2 a(tile.x - o.rangeX * s, tile.y - o.rangeY * s), b(tile.x + (o.rangeX + 1) * s, tile.y + (o.rangeY + 1) * s);
        dl->AddRectFilled(a, b, IM_COL32(60, 140, 255, 50));
        dl->AddRect(a, b, IM_COL32(60, 140, 255, 220), 0, 0, 2);
    }
    SpritePic p = Sprite(o.graphicsId, o.facing);
    if (p.tex)
    {
        float z = s / 16.0f;
        ImVec2 a(tile.x + (s - p.w * z) / 2, tile.y + s - p.h * z);
        dl->AddImage(TexId(p.tex), a, ImVec2(a.x + p.w * z, a.y + p.h * z), ImVec2(0, 0), ImVec2(1, 1), tint);
    }
    else
        Marker(dl, tile, s, IM_COL32(40, 120, 220, 255), "P", sel);
    if (events || sel)
        dl->AddRect(tile, ImVec2(tile.x + s, tile.y + s), sel ? IM_COL32(255, 220, 0, 255) : IM_COL32(40, 120, 220, 220), 0, 0,
                    sel ? 3.0f : 1.5f);
}

static void DrawEvents(ImDrawList *dl, ImVec2 origin, float s)
{
    bool events = gEd.mode == MODE_EVENTS;
    auto at = [&](int x, int y) { return ImVec2(origin.x + x * s, origin.y + y * s); };
    if (events)
    {
        for (size_t i = 0; i < sCoords.size(); i++)
        {
            MakerCoordInfo c = CurCoord((int)i);
            Marker(dl, at(c.x, c.y), s, IM_COL32(150, 60, 200, 255), "T", gEd.selKind == SEL_COORD && gEd.selIndex == (int)i);
        }
        for (size_t i = 0; i < sBgs.size(); i++)
        {
            MakerBgInfo c = CurBg((int)i);
            Marker(dl, at(c.x, c.y), s, IM_COL32(200, 40, 120, 255), strstr(c.typeName, "hidden") ? "H" : "S",
                   gEd.selKind == SEL_BG && gEd.selIndex == (int)i);
        }
        for (size_t i = 0; i < sWarps.size(); i++)
        {
            MakerWarpInfo c = CurWarp((int)i);
            Marker(dl, at(c.x, c.y), s, IM_COL32(230, 120, 20, 255), "W", gEd.selKind == SEL_WARP && gEd.selIndex == (int)i);
        }
    }
    if (!gEd.viewNpcs && !events)
        return;
    for (size_t i = 0; i < sObjs.size(); i++)
    {
        MakerObjectInfo o = CurPerson((int)i);
        bool sel = gEd.selKind == SEL_OBJECT && gEd.selIndex == (int)i;
        DrawPerson(dl, o, at(o.x, o.y), s, sel, events, IM_COL32_WHITE);
    }
}

// What's at a tile (Events mode): people first, then warps, signs, triggers
static bool PickEvent(int tx, int ty, int *kind, int *index)
{
    for (size_t i = 0; i < sObjs.size(); i++)
    {
        MakerObjectInfo o = CurPerson((int)i);
        if (o.x == tx && o.y == ty)
            return *kind = SEL_OBJECT, *index = (int)i, true;
    }
    for (size_t i = 0; i < sWarps.size(); i++)
    {
        MakerWarpInfo w = CurWarp((int)i);
        if (w.x == tx && w.y == ty)
            return *kind = SEL_WARP, *index = (int)i, true;
    }
    for (size_t i = 0; i < sBgs.size(); i++)
    {
        MakerBgInfo b = CurBg((int)i);
        if (b.x == tx && b.y == ty)
            return *kind = SEL_BG, *index = (int)i, true;
    }
    for (size_t i = 0; i < sCoords.size(); i++)
    {
        MakerCoordInfo c = CurCoord((int)i);
        if (c.x == tx && c.y == ty)
            return *kind = SEL_COORD, *index = (int)i, true;
    }
    return false;
}

// ---------------------------------------------------------------------------------------------------------------------
// The MAP panel

static void MouseOnMap(int tx, int ty, bool inside)
{
    bool l = ImGui::IsMouseClicked(ImGuiMouseButton_Left), r = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
    int cx = tx < 0 ? 0 : (tx >= sW ? sW - 1 : tx), cy = ty < 0 ? 0 : (ty >= sH ? sH - 1 : ty);
    if (gEd.mode == MODE_EVENTS)
    {
        if (l && inside)
        {
            int k, i;
            if (PickEvent(tx, ty, &k, &i))
            {
                int x, y, bx, by;
                gEd.selKind = k, gEd.selIndex = i;
                EventPos(k, i, &x, &y, &bx, &by);
                sDragKind = k, sDragIndex = i, sDragDX = tx - x, sDragDY = ty - y, sDragMoved = false;
            }
            else
                gEd.selKind = SEL_NONE, gEd.selIndex = -1;
        }
        if (sDragIndex >= 0 && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 2))
            sDragMoved = true;
        if (sDragIndex >= 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (sDragMoved)
                MoveEvent(sDragKind, sDragIndex, cx - sDragDX, cy - sDragDY);
            sDragIndex = -1;
        }
        return;
    }
    if (gEd.mode != MODE_MAP && gEd.mode != MODE_COLLISION)
        return;
    if (sTiles.sinnoh && gEd.mode == MODE_MAP)
        return;   // (SINNOH's tiles are its 3D world's: Collision mode edits what they do)
    // right-drag: in Map mode copies the tiles under it as the stamp; a right-click picks
    if (r && inside)
    {
        sCopy = true;
        sRectX0 = tx, sRectY0 = ty;
    }
    if (sCopy && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        sCopy = false;
        int x0 = std::min(sRectX0, cx), x1 = std::max(sRectX0, cx), y0 = std::min(sRectY0, cy), y1 = std::max(sRectY0, cy);
        if (x0 == x1 && y0 == y1)
            Pick(x0, y0);
        else if (gEd.mode == MODE_MAP)
        {
            sStampW = x1 - x0 + 1, sStampH = y1 - y0 + 1;
            sStamp.resize(sStampW * sStampH);
            for (int y = y0; y <= y1; y++)
                for (int x = x0; x <= x1; x++)
                    sStamp[(y - y0) * sStampW + (x - x0)] = Cur(x, y) & 0x3FF;
        }
    }
    if (l && inside)
    {
        switch (gEd.tool)
        {
        case TOOL_PICKER:
            Pick(tx, ty);
            break;
        case TOOL_FILL:
            BeginChange(gEd.map);
            Fill(tx, ty);
            EndChange();
            break;
        case TOOL_RECT:
            sRect = true;
            sRectX0 = tx, sRectY0 = ty;
            break;
        default:
            sStroke = true;
            BeginChange(gEd.map);
            sRectX0 = tx, sRectY0 = ty;
            sLastX = sLastY = -1;
            break;
        }
    }
    if (sStroke)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && inside && (tx != sLastX || ty != sLastY))
        {
            // (the stamp lands on its own grid from where the stroke began, so tiled pieces line up)
            int ox = tx - (((tx - sRectX0) % sStampW) + sStampW) % sStampW;
            int oy = ty - (((ty - sRectY0) % sStampH) + sStampH) % sStampH;
            if (gEd.mode == MODE_COLLISION)
                PaintCollision(tx, ty);
            else
                for (int y = 0; y < sStampH; y++)
                    for (int x = 0; x < sStampW; x++)
                        PaintMetatile(ox + x, oy + y, sStamp[y * sStampW + x]);
            sLastX = tx, sLastY = ty;
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            sStroke = false;
            EndChange();
        }
    }
    if (sRect && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        sRect = false;
        int x0 = std::min(sRectX0, cx), x1 = std::max(sRectX0, cx), y0 = std::min(sRectY0, cy), y1 = std::max(sRectY0, cy);
        BeginChange(gEd.map);
        for (int y = y0; y <= y1; y++)
            for (int x = x0; x <= x1; x++)
                PaintAt(x, y, x0, y0);
        EndChange();
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
    ImGui::BeginChild("canvas", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    origin.x += 32;
    origin.y += 32;
    if (sScrollTo.x >= 0)
    {
        ImGui::SetScrollX(sScrollTo.x * s + 32 - ImGui::GetWindowWidth() / 2);
        ImGui::SetScrollY(sScrollTo.y * s + 32 - ImGui::GetWindowHeight() / 2);
        sScrollTo = ImVec2(-1, -1);
    }
    ImVec2 clipMin = ImGui::GetWindowPos(), clipMax(clipMin.x + ImGui::GetWindowWidth(), clipMin.y + ImGui::GetWindowHeight());
    int x0 = std::max(0, (int)floorf((clipMin.x - origin.x) / s)), y0 = std::max(0, (int)floorf((clipMin.y - origin.y) / s));
    int x1 = std::min(sW, (int)ceilf((clipMax.x - origin.x) / s)), y1 = std::min(sH, (int)ceilf((clipMax.y - origin.y) / s));
    dl->AddRectFilled(ImVec2(origin.x - 1, origin.y - 1), ImVec2(origin.x + sW * s + 1, origin.y + sH * s + 1), IM_COL32(0, 0, 0, 255));
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
        {
            int b = Cur(x, y);
            ImVec2 p(origin.x + x * s, origin.y + y * s);
            if (b < 0)
                continue;
            if (sTiles.sinnoh)
                dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s), SinnohColor(b));
            else
                DrawMetatile(dl, b, p, s);
        }
    if (gEd.mode == MODE_COLLISION)
        for (int y = y0; y < y1; y++)
            for (int x = x0; x < x1; x++)
            {
                int b = Cur(x, y);
                if (b < 0)
                    continue;
                ImVec2 p(origin.x + x * s, origin.y + y * s);
                ImU32 c = CollisionColor(b);
                if (c)
                    dl->AddRectFilled(p, ImVec2(p.x + s, p.y + s), c);
                if (sCollAdvanced && s >= 24)
                {
                    char t[8];
                    snprintf(t, sizeof(t), "%d", (b >> 12) & 15);
                    dl->AddText(gEd.fontBold, s * 0.42f, ImVec2(p.x + 3, p.y + 2), IM_COL32(0, 0, 0, 230), t);
                }
            }
    else if (gEd.viewIcons)
        for (int y = y0; y < y1; y++)
            for (int x = x0; x < x1; x++)
            {
                int b = Cur(x, y);
                if (b < 0)
                    continue;
                int cls = MakerApi_BehaviorClass(MakerApi_MetatileBehavior(b & 0x3FF));
                if (cls != MAKER_BEHAVIOR_NONE && cls != MAKER_BEHAVIOR_CAVE && cls != MAKER_BEHAVIOR_COUNTER && cls != MAKER_BEHAVIOR_ICE)
                    DrawBehaviorIcon(dl, cls, ImVec2(origin.x + x * s, origin.y + y * s), s);
            }
    // (edited tiles whose game tile changed since: a red corner, PROBLEMS says more)
    if (const MapEdits *e = FindEdits(gEd.map))
        for (auto &b : e->blocks)
        {
            int x = b.first % e->w, y = b.first / e->w;
            if (x < sW && y < sH && Base(x, y) != b.second.first)
            {
                ImVec2 p(origin.x + x * s, origin.y + y * s);
                dl->AddTriangleFilled(p, ImVec2(p.x + s * 0.4f, p.y), ImVec2(p.x, p.y + s * 0.4f), IM_COL32(230, 30, 30, 255));
            }
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
    ImGui::SetCursorScreenPos(ImVec2(origin.x - 32, origin.y - 32));
    ImGui::InvisibleButton("map", ImVec2(sW * s + 64, sH * s + 64), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    bool hovered = ImGui::IsItemHovered(), active = ImGui::IsItemActive();
    ImVec2 m = ImGui::GetMousePos();
    int tx = (int)floorf((m.x - origin.x) / s), ty = (int)floorf((m.y - origin.y) / s);
    bool inside = hovered && tx >= 0 && ty >= 0 && tx < sW && ty < sH;
    if (hovered || active || sStroke || sRect || sCopy || sDragIndex >= 0)
        MouseOnMap(tx, ty, inside);
    if (inside)
    {
        gEd.hoverX = tx;
        gEd.hoverY = ty;
        gEd.hoverBlock = Cur(tx, ty);
        ImVec2 p(origin.x + tx * s, origin.y + ty * s);
        // what the brush would do: the stamp under the mouse (Map mode)
        bool stampTool = gEd.mode == MODE_MAP && !sTiles.sinnoh && (gEd.tool == TOOL_PENCIL || gEd.tool == TOOL_RECT);
        if (stampTool && !sRect && !sCopy)
            for (int y = 0; y < sStampH; y++)
                for (int x = 0; x < sStampW; x++)
                    if (tx + x < sW && ty + y < sH)
                        DrawMetatile(dl, sStamp[y * sStampW + x], ImVec2(p.x + x * s, p.y + y * s), s, IM_COL32(255, 255, 255, 170));
        if (gEd.viewFrame && gEd.mode != MODE_EVENTS)
        {
            ImVec2 f0(p.x - 7 * s, p.y - 4.5f * s), f1(f0.x + 15 * s, f0.y + 10 * s);
            dl->AddRect(f0, f1, IM_COL32(255, 255, 255, 200), 0, 0, 3);
            dl->AddRect(f0, f1, IM_COL32(0, 0, 0, 220), 0, 0, 1);
        }
        int w = stampTool && gEd.tool == TOOL_PENCIL ? sStampW : 1, h = stampTool && gEd.tool == TOOL_PENCIL ? sStampH : 1;
        dl->AddRect(p, ImVec2(p.x + s * w, p.y + s * h), IM_COL32(255, 255, 255, 255), 0, 0, 2);
        dl->AddRect(ImVec2(p.x - 1, p.y - 1), ImVec2(p.x + s * w + 1, p.y + s * h + 1), IM_COL32(0, 0, 0, 255), 0, 0, 1);
    }
    if (sRect || sCopy)
    {
        int cx = std::max(0, std::min(sW - 1, tx)), cy = std::max(0, std::min(sH - 1, ty));
        int rx0 = std::min(sRectX0, cx), rx1 = std::max(sRectX0, cx), ry0 = std::min(sRectY0, cy), ry1 = std::max(sRectY0, cy);
        ImVec2 a(origin.x + rx0 * s, origin.y + ry0 * s), b(origin.x + (rx1 + 1) * s, origin.y + (ry1 + 1) * s);
        dl->AddRectFilled(a, b, sCopy ? IM_COL32(255, 200, 0, 50) : IM_COL32(60, 140, 255, 60));
        dl->AddRect(a, b, sCopy ? IM_COL32(255, 200, 0, 255) : IM_COL32(60, 140, 255, 255), 0, 0, 2);
    }
    if (sDragIndex >= 0 && sDragMoved && inside)
    {
        ImVec2 p(origin.x + (tx - sDragDX) * s, origin.y + (ty - sDragDY) * s);
        if (sDragKind == SEL_OBJECT)
            DrawPerson(dl, CurPerson(sDragIndex), p, s, true, true, IM_COL32(255, 255, 255, 170));
        else
            dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 220, 0, 255), 0, 0, 3);
    }
    // Ctrl + wheel: zoom round the mouse
    ImGuiIO &io = ImGui::GetIO();
    if (hovered && io.KeyCtrl && io.MouseWheel != 0)
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
    ImGui::EndChild();
    ImGui::End();
}

// ---------------------------------------------------------------------------------------------------------------------
// The TILES panel: the metatiles to paint with (drag over several for a stamp), or Collision mode's brush

static void CollisionPalette()
{
    ImGui::TextWrapped("Paint where the player can walk. Each tile keeps its elevation unless you turn on the full grid.");
    ImGui::Spacing();
    float bw = (ImGui::GetContentRegionAvail().x - 8) / 2;
    auto brush = [&](const char *label, int coll, ImU32 col) {
        bool sel = sCollBrush == coll && !sCollAdvanced;
        if (sel)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
        ImVec2 p = ImGui::GetCursorScreenPos();
        if (ImGui::Button(label, ImVec2(bw, 40 * gEd.uiScale)))
            sCollBrush = coll, sCollAdvanced = false;
        if (sel)
            ImGui::PopStyleColor();
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + 6, p.y + 6), ImVec2(p.x + 18, p.y + 18), col);
    };
    brush("Walkable", 0, IM_COL32(80, 180, 80, 255));
    ImGui::SameLine();
    brush("Blocked", 1, IM_COL32(220, 30, 30, 255));
    ImGui::Spacing();
    ImGui::Checkbox("Full collision + elevation grid", &sCollAdvanced);
    if (!sCollAdvanced)
        return;
    ImGui::TextDisabled("Collision 0 walkable, 1-3 blocked; elevation 0-15 (bridges, ledges, levels).");
    if (ImGui::BeginTable("coll", 5, ImGuiTableFlags_SizingFixedFit))
    {
        for (int e = 0; e < 16; e++)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("elev %2d", e);
            for (int c = 0; c < 4; c++)
            {
                ImGui::TableNextColumn();
                char id[16];
                snprintf(id, sizeof(id), "%X%d##%d", c, e, e * 4 + c);
                bool sel = sCollBrush == c && sElevBrush == e;
                ImVec4 col(0, 0, 0, 1);
                ImGui::ColorConvertHSVtoRGB(e / 16.0f, 0.75f, 0.95f, col.x, col.y, col.z);
                if (c)
                    col = ImVec4(0.8f, 0.15f, 0.15f, 1);
                ImGui::PushStyleColor(ImGuiCol_Button, col);
                if (sel)
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3);
                if (ImGui::Button(id, ImVec2(34 * gEd.uiScale, 0)))
                    sCollBrush = c, sElevBrush = e;
                if (sel)
                    ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndTable();
    }
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
    if (gEd.mode == MODE_COLLISION)
    {
        CollisionPalette();
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
    auto inStamp = [&](int id) {
        for (int v : sStamp)
            if (v == id)
                return true;
        return false;
    };
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
            ImGui::InvisibleButton("mt", ImVec2(s, s));
            bool hov = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
            if (ImGui::IsItemClicked())
                sPalDragFrom = id;
            if (sPalDragFrom >= 0 && hov && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // (the rectangle of metatiles from where the drag began to here, as the stamp)
                int fa = sPalDragFrom - first, fb = id - first;
                if (fa >= 0 && fa < count)
                {
                    int ax = fa % cols, ay = fa / cols, bx = fb % cols, by = fb / cols;
                    int x0 = std::min(ax, bx), x1 = std::max(ax, bx), y0 = std::min(ay, by), y1 = std::max(ay, by);
                    sStampW = x1 - x0 + 1, sStampH = y1 - y0 + 1;
                    sStamp.resize(sStampW * sStampH);
                    for (int y = y0; y <= y1; y++)
                        for (int x = x0; x <= x1; x++)
                        {
                            int k = y * cols + x;
                            sStamp[(y - y0) * sStampW + (x - x0)] = first + (k < count ? k : 0);
                        }
                    if (gEd.tool == TOOL_PICKER || gEd.tool == TOOL_FILL)
                        gEd.tool = TOOL_PENCIL;
                }
            }
            ImGui::PopID();
            DrawMetatile(dl, id, p, s);
            if (inStamp(id))
            {
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 255), 0, 0, 3);
                dl->AddRect(ImVec2(p.x - 1, p.y - 1), ImVec2(p.x + s + 1, p.y + s + 1), IM_COL32(0, 90, 200, 255), 0, 0, 2);
            }
            else if (hov)
                dl->AddRect(p, ImVec2(p.x + s, p.y + s), IM_COL32(255, 255, 255, 180), 0, 0, 1);
            if (hov && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                ImGui::SetTooltip("Metatile 0x%03X", id);
        }
    };
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        sPalDragFrom = -1;
    ImGui::BeginChild("tiles", ImVec2(0, 0), ImGuiChildFlags_None);
    char title[96];
    snprintf(title, sizeof(title), "%s (primary)", sTiles.primaryName);
    section(title, 0, sTiles.primaryMetatiles);
    snprintf(title, sizeof(title), "%s (secondary)", sTiles.secondaryName);
    section(title, sTiles.secondaryStart, sTiles.secondaryMetatiles);
    ImGui::EndChild();
    ImGui::End();
}

// ---------------------------------------------------------------------------------------------------------------------
// The INSPECTOR: the picked event's details, editable

static bool ContainsNoCase(const char *hay, const char *needle)
{
    size_t n = strlen(needle);
    for (; *hay; hay++)
        if (strncasecmp(hay, needle, n) == 0)
            return true;
    return n == 0;
}

// A searchable list of named values (looks, movement types, flags...); true when one was picked
static bool NamePicker(const char *id, int table, long *value, const char *noneLabel = NULL)
{
    const char *cur = MakerApi_NameOf(table, (unsigned)*value);
    char preview[96];
    snprintf(preview, sizeof(preview), "%s", noneLabel && *value == 0 ? noneLabel : (cur && cur[0] ? cur : "?"));
    bool picked = false;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, preview, ImGuiComboFlags_HeightLarge))
    {
        static char filter[64];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##filter", "Search...", filter, sizeof(filter));
        if (noneLabel && ImGui::Selectable(noneLabel, *value == 0))
            *value = 0, picked = true;
        int n = MakerApi_NameCount(table);
        for (int i = 0; i < n; i++)
        {
            unsigned v;
            const char *name = MakerApi_NameAt(table, i, &v);
            if (filter[0] && !ContainsNoCase(name, filter))
                continue;
            if (ImGui::Selectable(name, (long)v == *value))
                *value = v, picked = true;
        }
        ImGui::EndCombo();
    }
    return picked;
}

static bool MapPicker(const char *id, int *map)
{
    bool picked = false;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(id, *map >= 0 ? gEd.maps[*map].title : "?", ImGuiComboFlags_HeightLarge))
    {
        static char filter[64];
        if (ImGui::IsWindowAppearing())
        {
            filter[0] = 0;
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##filter", "Search...", filter, sizeof(filter));
        for (int i = 0; i < (int)gEd.maps.size(); i++)
        {
            if (filter[0] && !ContainsNoCase(gEd.maps[i].title, filter) && !ContainsNoCase(gEd.maps[i].name, filter))
                continue;
            ImGui::PushID(i);
            if (ImGui::Selectable(gEd.maps[i].title, i == *map))
                *map = i, picked = true;
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return picked;
}

static void Label(const char *k)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", k);
    ImGui::TableNextColumn();
}

// A number edited in place (one undo step per drag: EndChange when the mouse lets go, maker_ui.cpp)
static void IntField(const char *label, const char *id, const char *kind, const std::string &key, const char *prop,
                     long base, long cur, int lo, int hi)
{
    Label(label);
    int v = (int)cur;
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::SliderInt(id, &v, lo, hi))
    {
        BeginChange(gEd.map);
        SetProp(gEd.map, kind, key, prop, base, v);
    }
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
    if (!ImGui::BeginTable("sel", 2, ImGuiTableFlags_SizingStretchProp))
        return;
    ImGui::TableSetupColumn("k", ImGuiTableColumnFlags_WidthFixed, 80 * gEd.uiScale);
    ImGui::TableSetupColumn("v", ImGuiTableColumnFlags_WidthStretch);
    int i = gEd.selIndex, m = gEd.map;
    if (gEd.selKind == SEL_OBJECT && i < (int)sObjs.size())
    {
        const MakerObjectInfo &b = sObjs[i];
        MakerObjectInfo o = CurPerson(i);
        std::string key = PersonKey(i);
        Label("Name");
        ImGui::TextWrapped("%s", b.localIdName[0] ? b.localIdName : "(no name: known by its script)");
        Label("Look");
        long look = o.graphicsId;
        if (NamePicker("##look", MAKER_NAMES_GFX, &look))
        {
            BeginChange(m);
            SetProp(m, "person", key, "look", b.graphicsId, look);
            EndChange();
        }
        Label("Moves");
        long mv = o.movementType;
        if (NamePicker("##move", MAKER_NAMES_MOVEMENT, &mv))
        {
            BeginChange(m);
            SetProp(m, "person", key, "movement", b.movementType, mv);
            EndChange();
        }
        IntField("Range X", "##rx", "person", key, "range_x", b.rangeX, o.rangeX, 0, 15);
        IntField("Range Y", "##ry", "person", key, "range_y", b.rangeY, o.rangeY, 0, 15);
        Label("At");
        ImGui::Text("%d, %d", o.x, o.y);
        IntField("Elevation", "##el", "person", key, "elevation", b.elevation, o.elevation, 0, 15);
        Label("Trainer");
        long tt = o.trainerType;
        if (NamePicker("##tt", MAKER_NAMES_TRAINERTYPE, &tt))
        {
            BeginChange(m);
            SetProp(m, "person", key, "trainer", b.trainerType, tt);
            EndChange();
        }
        if (o.trainerType)
            IntField("Sees", "##sight", "person", key, "sight", b.sight, o.sight, 0, 15);
        Label("Hide flag");
        long fl = o.flag;
        if (NamePicker("##flag", MAKER_NAMES_FLAG, &fl, "(always there)"))
        {
            BeginChange(m);
            SetProp(m, "person", key, "flag", b.flag, fl);
            EndChange();
        }
        Label("Script");
        ImGui::TextWrapped("%s", b.scriptName);
    }
    else if (gEd.selKind == SEL_WARP && i < (int)sWarps.size())
    {
        const MakerWarpInfo &b = sWarps[i];
        MakerWarpInfo w = CurWarp(i);
        std::string key = WarpKey(i);
        long baseDest = ((long)b.destMap << 8) | b.destWarp;
        Label("At");
        ImGui::Text("%d, %d", w.x, w.y);
        Label("To map");
        int dm = w.destMap;
        if (MapPicker("##dest", &dm))
        {
            BeginChange(m);
            SetProp(m, "warp", key, "dest", baseDest, ((long)dm << 8) | w.destWarp);
            EndChange();
        }
        Label("Its warp");
        int dw = w.destWarp;
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputInt("##dw", &dw) && dw >= 0 && dw < 256)
        {
            BeginChange(m);
            SetProp(m, "warp", key, "dest", baseDest, ((long)w.destMap << 8) | dw);
            EndChange();
        }
        if (w.destMap >= 0)
        {
            Label("");
            if (ImGui::SmallButton("Open that map"))
                SelectMap(w.destMap);
        }
    }
    else if (gEd.selKind == SEL_BG && i < (int)sBgs.size())
    {
        MakerBgInfo c = CurBg(i);
        Label("At");
        ImGui::Text("%d, %d", c.x, c.y);
        Label("Kind");
        ImGui::Text("%s", c.typeName);
        Label("Script");
        ImGui::TextWrapped("%s", c.scriptName);
    }
    else if (gEd.selKind == SEL_COORD && i < (int)sCoords.size())
    {
        MakerCoordInfo c = CurCoord(i);
        Label("At");
        ImGui::Text("%d, %d", c.x, c.y);
        Label("When");
        ImGui::Text("%s", c.varName);
        Label("Script");
        ImGui::TextWrapped("%s", c.scriptName);
    }
    ImGui::EndTable();
    ImGui::TextDisabled("Drag it on the map to move it.");
}

// ---------------------------------------------------------------------------------------------------------------------
// PROBLEMS: the open map's edits that the game no longer matches (a later update changed the same thing)

struct Problem { std::string text; bool block; int at; std::string propKey; long base; };

static long BasePersonProp(const MakerObjectInfo &o, const std::string &pr)
{
    return pr == "x" ? o.x : pr == "y" ? o.y : pr == "elevation" ? o.elevation : pr == "look" ? o.graphicsId
         : pr == "movement" ? o.movementType : pr == "range_x" ? o.rangeX : pr == "range_y" ? o.rangeY
         : pr == "trainer" ? o.trainerType : pr == "sight" ? o.sight : pr == "flag" ? o.flag : LONG_MIN;
}

static std::vector<Problem> Problems()
{
    std::vector<Problem> out;
    const MapEdits *e = FindEdits(gEd.map);
    if (!e || gEd.map != sLoadedMap)
        return out;
    char t[320];
    for (auto &b : e->blocks)
    {
        int x = b.first % e->w, y = b.first / e->w;
        if (x >= sW || y >= sH)
        {
            snprintf(t, sizeof(t), "Tile %d, %d is outside the map now (the game changed its size).", x, y);
            out.push_back(Problem{t, true, b.first, "", LONG_MIN});
        }
        else if (Base(x, y) != b.second.first)
        {
            snprintf(t, sizeof(t), "Tile %d, %d: the game changed it since your edit (it was 0x%04X, now 0x%04X).", x, y,
                     b.second.first, Base(x, y));
            out.push_back(Problem{t, true, b.first, "", Base(x, y)});
        }
    }
    for (auto &p : e->props)
    {
        // "KIND KEY PROP": the event it names, and its value as built
        char kind[16], key[160], prop[16];
        if (sscanf(p.first.c_str(), "%15s %159s %15s", kind, key, prop) != 3)
            continue;
        long base = LONG_MIN;
        std::string k = kind, pr = prop;
        if (k == "person")
        {
            for (size_t i = 0; i < sObjs.size(); i++)
                if (PersonKey((int)i) == key)
                    base = BasePersonProp(sObjs[i], pr);
        }
        else if (k == "warp" && atoi(key) < (int)sWarps.size())
        {
            const MakerWarpInfo &w = sWarps[atoi(key)];
            base = pr == "x" ? w.x : pr == "y" ? w.y : pr == "elevation" ? w.elevation : pr == "dest" ? (((long)w.destMap << 8) | w.destWarp) : LONG_MIN;
        }
        else if (k == "sign" || k == "trigger")
        {
            bool sign = k == "sign";
            size_t n = sign ? sBgs.size() : sCoords.size();
            for (size_t i = 0; i < n; i++)
            {
                int bx = sign ? sBgs[i].x : sCoords[i].x, by = sign ? sBgs[i].y : sCoords[i].y;
                if (SpotKey(sign ? sBgs[i].scriptName : sCoords[i].scriptName, bx, by) == key)
                    base = pr == "x" ? bx : pr == "y" ? by : LONG_MIN;
            }
        }
        if (base == LONG_MIN)
        {
            snprintf(t, sizeof(t), "%s %s: this edit points at something the map doesn't have now.", kind, key);
            out.push_back(Problem{t, false, 0, p.first, LONG_MIN});
        }
        else if (base != p.second.was)
        {
            snprintf(t, sizeof(t), "%s %s %s: the game changed it since your edit.", kind, key, prop);
            out.push_back(Problem{t, false, 0, p.first, base});
        }
    }
    return out;
}

int MapView_ProblemCount()
{
    return (int)Problems().size();
}

void MapView_Problems()
{
    std::vector<Problem> ps = Problems();
    if (ps.empty())
    {
        ImGui::TextDisabled(FindEdits(gEd.map) ? "No problems with this map's edits." : "No problems.");
        return;
    }
    for (size_t i = 0; i < ps.size(); i++)
    {
        const Problem &p = ps[i];
        ImGui::PushID((int)i);
        ImGui::TextColored(ImVec4(0.85f, 0.2f, 0.1f, 1), "!");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", p.text.c_str());
        if (p.base != LONG_MIN)
        {
            if (ImGui::SmallButton("Keep mine"))
            {
                // (the edit made against the game's new version)
                BeginChange(gEd.map);
                MapEdits &e = Edits(gEd.map);
                if (p.block)
                    e.blocks[p.at].first = (int)p.base;
                else
                    e.props[p.propKey].was = p.base;
                EndChange();
            }
            ImGui::SameLine();
        }
        if (ImGui::SmallButton(p.base != LONG_MIN ? "Take the game's" : "Remove this edit"))
        {
            BeginChange(gEd.map);
            MapEdits &e = Edits(gEd.map);
            if (p.block)
                e.blocks.erase(p.at);
            else
                e.props.erase(p.propKey);
            EndChange();
        }
        ImGui::PopID();
    }
}

int MapView_SelectedMetatile()
{
    return sStamp.empty() ? 0 : sStamp[0];
}

// Tests: center X Y / anim 0|1 / pick X Y (Events mode's click) / stats / stamp ID [W H] / paint X Y / rect X0 Y0 X1 Y1 /
// fill X Y / coll C [E] / move X Y (the picked event) / prop NAME VALUE (the picked person's, by name)
void MapView_TestCommand(const char *cmd, const char *arg)
{
    std::string c = cmd;
    LoadMap();
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
        if (PickEvent(x, y, &k, &i))
            gEd.selKind = k, gEd.selIndex = i;
    }
    else if (c == "stamp")
    {
        int id = 0, w = 1, h = 1;
        sscanf(arg, "%i %d %d", &id, &w, &h);
        sStampW = w, sStampH = h;
        sStamp.resize(w * h);
        for (int i = 0; i < w * h; i++)
            sStamp[i] = id + (i / w) * 8 + i % w;   // (as the TILES panel lays them out: 8 a row)
    }
    else if (c == "paint" || c == "fill" || c == "rect")
    {
        int a = 0, b = 0, x1 = 0, y1 = 0;
        sscanf(arg, "%d %d %d %d", &a, &b, &x1, &y1);
        BeginChange(gEd.map);
        if (c == "paint")
        {
            if (gEd.mode == MODE_COLLISION)
                PaintCollision(a, b);
            else
                for (int y = 0; y < sStampH; y++)
                    for (int x = 0; x < sStampW; x++)
                        PaintMetatile(a + x, b + y, sStamp[y * sStampW + x]);
        }
        else if (c == "fill")
            Fill(a, b);
        else
            for (int y = std::min(b, y1); y <= std::max(b, y1); y++)
                for (int x = std::min(a, x1); x <= std::max(a, x1); x++)
                    PaintAt(x, y, std::min(a, x1), std::min(b, y1));
        EndChange();
    }
    else if (c == "coll")
    {
        int cc = 1, e = -1;
        sscanf(arg, "%d %d", &cc, &e);
        sCollBrush = cc;
        sCollAdvanced = e >= 0;
        if (e >= 0)
            sElevBrush = e;
    }
    else if (c == "move" && gEd.selKind != SEL_NONE)
    {
        int x = 0, y = 0;
        sscanf(arg, "%d %d", &x, &y);
        MoveEvent(gEd.selKind, gEd.selIndex, x, y);
    }
    else if (c == "prop" && gEd.selKind == SEL_OBJECT)
    {
        char name[32], value[96];
        if (sscanf(arg, "%31s %95s", name, value) == 2)
        {
            static const struct { const char *prop; int table; } kProps[] = {
                {"look", MAKER_NAMES_GFX}, {"movement", MAKER_NAMES_MOVEMENT}, {"trainer", MAKER_NAMES_TRAINERTYPE},
                {"flag", MAKER_NAMES_FLAG}, {"range_x", -1}, {"range_y", -1}, {"sight", -1}, {"elevation", -1}};
            for (auto &p : kProps)
            {
                if (strcmp(p.prop, name))
                    continue;
                unsigned v = 0;
                if (p.table >= 0 && !MakerApi_ValueOf(p.table, value, &v))
                    break;
                if (p.table < 0)
                    v = atoi(value);
                BeginChange(gEd.map);
                SetProp(gEd.map, "person", PersonKey(gEd.selIndex), name, BasePersonProp(sObjs[gEd.selIndex], name), v);
                EndChange();
            }
        }
    }
    else if (c == "stats")
    {
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
    else if (c == "problems")
        printf("[maker] problems %s: %d\n", gEd.map >= 0 ? gEd.maps[gEd.map].name : "-", MapView_ProblemCount());
}

} // namespace maker
