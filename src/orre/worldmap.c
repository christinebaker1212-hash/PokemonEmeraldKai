// Moemon Kai: Colosseum's own world map screen (the user, 2026-11-07: leaving a place through its exits opens it,
// never a Kai-style town map). Everything on it is the game's: the menu items it is built from and their sprites
// (main.dol's menu tables, dol/menu_items.bin and dol/menu_sprites.bin: the Colosseum decomp's
// src/game/menu/menu_bios_range_8005D7F8.c), its textures (world_map/menu_039.tex the map, menu_038.tex the pieces,
// mapsnap*.tex each place's picture), its tables (worldmap_menu_rel's .data: src/rel/worldmap_menu_rel/) and its
// words (world_map/world_map.msg), all from ORRE's data pack (src/orre/orre_data.c). The screen's own code is the
// decomp's src/game/gs_title.c wmapCursorMove / wmapDrawInfo, followed here:
//   places (lbl_3_data_0, 9 of 0x10): u32 flag (shown when 0 or set), room (the floor table's), menu item, picture
//   descriptions (lbl_3_data_90, 12 of 0x28): u32 flag, msg[9]: the first row whose flag is 0 or set gives each place
//     its words, place i's at msg[order[i]] (main.dol lbl_802E4F58, dol/wm_desc_order.bin)
//   links (lbl_3_data_3B0, 66 of 0x0C): u8 directions (1 up, 2 right, 4 down, 8 left; both for a diagonal), u32 from,
//     to (menu items): the first link from the cursor's place that has every direction pressed, to a shown place
// Moving the cursor takes 90 units at 4.5 a frame (the picture and words of the place left fade out over the first
// half, the new place's in over the second). Drawn at 640x480 by the game, here 2/3 of that in a 480x320 picture of
// its own, as the BADGE CASE's (display.c, sdl2.c ScreenStep); the game waits while it is open.
// A on a place ORRE has a map for travels there (its start: src/orre/orre_maps.h, by Colosseum's room); B cancels.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "sound.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "gx_tex.h"
#include "orre.h"
#include "orre_data.h"
#include "orre_maps.h"
#include "../tcg/tcg_gfx.h"

void MPNet_Log(const char *fmt, ...);

#define CW 480
#define CH 320
#define SCALE (320.0f / 480.0f)
#define OX ((CW - 640 * SCALE) / 2)

#define ITEM_FIRST 0x906   // the screen's first menu item (the map)
#define ITEM_PICTURE 0x91B // the place's picture
#define ITEM_WORDS 0x91C   // its description
#define ITEM_SELECT 0xCA9
#define ITEM_CANCEL 0xCAA
#define ITEM_DOT 0x91A     // the cursor: its red diamond, the four brackets round it, the cross lines through it
#define ITEM_HLINE 0x935
#define ITEM_VLINE 0x936
#define ITEM_CORNER0 0x931
#define ITEM_CORNER3 0x934
#define MSG_SELECT 15588
#define MSG_CANCEL 15583
#define MOVE_TIME 90.0f
#define MOVE_STEP 4.5f
#define FADE 16
#define MAX_PLACES 16
#define MAX_TEX 24

enum { WM_NONE, WM_OPEN, WM_MAIN, WM_CLOSE };

typedef struct { u8 flags; s16 x, y, w, h, a, b, next; } Item;

static struct
{
    u8 *items, *sprites, *rel, *msg, *order;
    u32 nItems, nSprites, nMsg, nOrder;
    const u8 *places, *rows, *links;
    u32 nPlaces, nRows, nLinks;
    u8 state;
    u8 t;          // the fade
    float move;    // the cursor's move left (MOVE_TIME..0)
    s8 cur, prev;  // places
    s8 chosen;     // sOrreMaps index to travel to, -1 none
    u16 prevKeys;
    u32 frame;
    bool8 enableScript;
    SDL_Renderer *owner;
    SDL_Texture *canvas;
    struct { u32 id; SDL_Texture *t; } tex[MAX_TEX];
    u32 nTex;
    u16 bigFrame[CW * CH];
    bool8 frameOk;
} W;

static s8 sChosen = -1;

static u32 Be32(const u8 *p) { return (u32)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static s16 Be16(const u8 *p) { return (s16)(p[0] << 8 | p[1]); }

static bool8 GetItem(u32 i, Item *it)
{
    const u8 *p;
    if (i >= W.nItems)
        return FALSE;
    p = W.items + i * 0x1C;
    it->flags = p[0];
    it->x = Be16(p + 2);
    it->y = Be16(p + 4);
    it->w = Be16(p + 6);
    it->h = Be16(p + 8);
    it->a = Be16(p + 10);
    it->b = Be16(p + 12);
    it->next = Be16(p + 0x18);
    return TRUE;
}

// ---- the tables

static u32 PlaceFlag(u32 i) { return Be32(W.places + i * 16); }
static u32 PlaceRoom(u32 i) { return Be32(W.places + i * 16 + 4); }
static u32 PlaceItem(u32 i) { return Be32(W.places + i * 16 + 8); }

static bool8 FlagOn(u32 flag)
{
    return flag == 0 || Orre_Flag(flag);
}

static bool8 PlaceShown(s32 i)
{
    return i >= 0 && (u32)i < W.nPlaces && FlagOn(PlaceFlag(i));
}

static s32 PlaceOfItem(u32 item)
{
    u32 i;
    for (i = 0; i < W.nPlaces; i++)
        if (PlaceItem(i) == item)
            return i;
    return -1;
}

// the place's picture (fn_80024A2C: THE UNDER's, once flag 0x45D is set, is mapsnap008)
static u32 PlacePicture(u32 i)
{
    if (PlaceRoom(i) == 0x66 && Orre_Flag(0x45D))
        return 0x0C5F1200;
    return Be32(W.places + i * 16 + 12);
}

// the sOrreMaps entry whose room is the place's (-1: not in the game yet)
static s32 PlaceMap(u32 i)
{
    u32 k;
    for (k = 0; k < ARRAY_COUNT(sOrreMaps); k++)
        if (sOrreMaps[k].room == PlaceRoom(i))
            return k;
    return -1;
}

// the place's words (wmapDrawInfo): the first description row whose flag is 0 or set
static u32 PlaceWords(u32 i)
{
    u32 r, slot;
    for (r = 0; r < W.nRows; r++)
        if (FlagOn(Be32(W.rows + r * 40)))
        {
            slot = i < W.nOrder ? W.order[i] : 0;
            return Be32(W.rows + r * 40 + 4 + (slot > 8 ? 0 : slot) * 4);
        }
    return 1;
}

// a message of world_map.msg as UTF-8 for G_Text (its controls left out; a line break kept)
static const char *Msg(u32 key)
{
    static char out[512];
    u32 n = 0, i, o = 0, count;
    out[0] = 0;
    if (W.msg == NULL || key == 1)
        return out;
    count = (u16)Be16(W.msg + 4);
    for (i = 0; i < count && 0x10 + i * 8 + 8 <= W.nMsg; i++)
        if (Be32(W.msg + 0x10 + i * 8) == key)
        {
            o = Be32(W.msg + 0x10 + i * 8 + 4);
            break;
        }
    if (i == count)
        return out;
    while (o + 2 <= W.nMsg && n < sizeof(out) - 4)
    {
        u16 c = (u16)Be16(W.msg + o);
        o += 2;
        if (c == 0)
            break;
        if (c == 0xFFFF)
        {
            u8 id = W.msg[o++];
            // (the parameter bytes of the controls that have them: tools/orre/msg.py CONTROLS)
            static const u8 sParams[][2] = {{7, 1}, {8, 4}, {9, 1}, {56, 1}, {82, 1}, {83, 1}, {84, 1}, {91, 1}, {92, 1}};
            u32 k;
            if (id <= 1)
                out[n++] = '\n';
            for (k = 0; k < ARRAY_COUNT(sParams); k++)
                if (sParams[k][0] == id)
                    o += sParams[k][1];
            continue;
        }
        if (c == '\n' || (c >= 0x20 && c < 0x7F))
            out[n++] = (char)c;
        else if (c == 0xE9)
        {
            out[n++] = (char)0xC3;
            out[n++] = (char)0xA9;
        }
        else
            out[n++] = '?';
    }
    out[n] = 0;
    return out;
}

static void FreeData(void)
{
    free(W.items);
    free(W.sprites);
    free(W.rel);
    free(W.msg);
    free(W.order);
    W.items = W.sprites = W.rel = W.msg = W.order = NULL;
}

static bool8 LoadData(void)
{
    u32 relSize, st, off, size;
    if (W.items)
        return TRUE;
    W.items = OrreData_Read("dol/menu_items.bin", &W.nItems);
    W.sprites = OrreData_Read("dol/menu_sprites.bin", &W.nSprites);
    W.rel = OrreData_Read("world_map/worldmap_menu_rel.rel", &relSize);
    W.msg = OrreData_Read("world_map/world_map.msg", &W.nMsg);
    W.order = OrreData_Read("dol/wm_desc_order.bin", &W.nOrder);
    if (!W.items || !W.sprites || !W.rel || !W.msg || !W.order || relSize < 0x40)
    {
        printf("[orre] the world map's data isn't in ORRE's pack (%s)\n", OrreData_Path());
        FreeData();
        return FALSE;
    }
    W.nItems /= 0x1C;
    W.nSprites /= 0x18;
    // the module's .data: section 3 (src/rel/worldmap_menu_rel/worldmap_menu_rel.c)
    st = Be32(W.rel + 0x10);
    off = Be32(W.rel + st + 8 * 3) & ~3u;
    size = Be32(W.rel + st + 8 * 3 + 4);
    if (st + 8 * 4 > relSize || off + size > relSize || size < 0x6D8)
    {
        printf("[orre] worldmap_menu_rel isn't the module the decomp has\n");
        FreeData();
        return FALSE;
    }
    W.places = W.rel + off;
    W.rows = W.rel + off + 0x90;
    W.links = W.rel + off + 0x3B0;
    W.nPlaces = min(Be32(W.rel + off + 0x6C8), MAX_PLACES);
    W.nRows = Be32(W.rel + off + 0x6CC);
    W.nLinks = Be32(W.rel + off + 0x6D4);
    return TRUE;
}

// ---- the textures (by resource id: index/ids.txt; the screen's are all in world_map)

static SDL_Texture *Texture(SDL_Renderer *r, u32 id)
{
    char name[96], path[128];
    const char *base;
    u8 *d;
    u32 size, i, *px;
    u16 w, h;
    u8 fmt;
    SDL_Texture *t = NULL;
    for (i = 0; i < W.nTex; i++)
        if (W.tex[i].id == id)
            return W.tex[i].t;
    if (W.nTex == MAX_TEX)
        return NULL;
    if (OrreData_ResourceName(id, name, sizeof(name)))
    {
        base = strrchr(name, '/') ? strrchr(name, '/') + 1 : name;
        snprintf(path, sizeof(path), "world_map/%s", base);
        if ((d = OrreData_Read(path, &size)) != NULL)
        {
            w = (u16)Be16(d);
            h = (u16)Be16(d + 2);
            fmt = d[4] == 0x04 ? 14 : d[4] == 0x20 ? 6 : 0xFF;   // (the .tex's code: CMPR, RGBA8)
            px = malloc((size_t)w * h * 4);
            if (fmt != 0xFF && px && size > 0x80
             && GxTex_Decode(fmt, w, h, d + 0x80, size - 0x80, 0, NULL, 0, px)
             && (t = SDL_CreateTexture(r, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h)) != NULL)
            {
                SDL_UpdateTexture(t, NULL, px, w * 4);
                SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(t, SDL_ScaleModeLinear);
            }
            free(px);
            free(d);
        }
    }
    if (t == NULL)
        printf("[orre] the world map's texture %08x won't load\n", id);
    W.tex[W.nTex].id = id;
    W.tex[W.nTex].t = t;
    W.nTex++;
    return t;
}

// a menu item's sprite at the item's place (moved by dx, dy; a negative width / height mirrors it), on the 640x480
// screen; picture: another texture in the sprite's rectangle (0: its own)
static void DrawSprite(SDL_Renderer *r, const Item *it, float dx, float dy, u32 picture, u8 alpha)
{
    const u8 *s;
    SDL_Rect src;
    SDL_FRect dst;
    SDL_Texture *t;
    int flip = 0;
    if (it->a <= 0 || (u32)it->a >= W.nSprites)
        return;
    s = W.sprites + it->a * 0x18;
    src.x = (u16)Be16(s + 8);
    src.y = (u16)Be16(s + 10);
    src.w = (u16)Be16(s + 12);
    src.h = (u16)Be16(s + 14);
    if ((t = Texture(r, picture ? picture : Be32(s + 0x10))) == NULL)
        return;
    dst.x = OX + (it->x + dx) * SCALE;
    dst.y = (it->y + dy) * SCALE;
    dst.w = abs(it->w) * SCALE;
    dst.h = abs(it->h) * SCALE;
    if (it->w < 0)
        flip |= SDL_FLIP_HORIZONTAL;
    if (it->h < 0)
        flip |= SDL_FLIP_VERTICAL;
    SDL_SetTextureAlphaMod(t, alpha);
    SDL_RenderCopyExF(r, t, &src, &dst, 0, NULL, flip);
    SDL_SetTextureAlphaMod(t, 255);
}

// a place's centre on the 640x480 screen
static void PlaceCentre(s32 i, float *x, float *y)
{
    Item it;
    *x = *y = 0;
    if (i >= 0 && GetItem(PlaceItem(i), &it))
    {
        *x = it.x + it.w / 2.0f;
        *y = it.y + it.h / 2.0f;
    }
}

// where the cursor is (sliding from the place left to the new one while it moves)
static void CursorAt(float *x, float *y)
{
    float ax, ay, bx, by, k = 1 - W.move / MOVE_TIME;
    PlaceCentre(W.cur, &bx, &by);
    if (W.move <= 0 || W.prev < 0)
    {
        *x = bx;
        *y = by;
        return;
    }
    PlaceCentre(W.prev, &ax, &ay);
    k = k * k * (3 - 2 * k);
    *x = ax + (bx - ax) * k;
    *y = ay + (by - ay) * k;
}

// the place whose picture and words show now, and how solid (titleUpdateCursor)
static s32 ShownPlace(u8 *alpha)
{
    if (W.move > MOVE_TIME / 2 && W.prev >= 0)
    {
        *alpha = (u8)(255 * (W.move - MOVE_TIME / 2) / (MOVE_TIME / 2));
        return W.prev;
    }
    *alpha = W.move > 0 ? (u8)(255 * (MOVE_TIME / 2 - W.move) / (MOVE_TIME / 2)) : 255;
    return W.cur;
}

static void Draw(SDL_Renderer *r)
{
    Item it, dot;
    u32 i = ITEM_FIRST, n = 0;
    float cx, cy, gx, gy, spread;
    s32 place;
    u8 alpha;
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    CursorAt(&cx, &cy);
    // (the cursor's pieces stand round the red diamond: moved with it from where the table puts them)
    GetItem(ITEM_DOT, &dot);
    gx = cx - (dot.x + dot.w / 2.0f);
    gy = cy - (dot.y + dot.h / 2.0f);
    spread = W.state == WM_OPEN ? 24.0f * W.t / FADE : 1.5f + 1.5f * sinf(W.frame * 0.12f);
    place = ShownPlace(&alpha);
    while (n++ < 200 && GetItem(i, &it))
    {
        s32 p = PlaceOfItem(i);
        if (p >= 0)
        {
            if (PlaceShown(p))
                DrawSprite(r, &it, 0, 0, 0, 255);
        }
        else if (i == ITEM_DOT)
            DrawSprite(r, &it, gx, gy, 0, 255);
        else if (i == ITEM_HLINE)
            DrawSprite(r, &it, 0, cy - (it.y + 1.5f), 0, 160);
        else if (i == ITEM_VLINE)
            DrawSprite(r, &it, cx - (it.x + 1.5f), 0, 0, 160);
        else if (i >= ITEM_CORNER0 && i <= ITEM_CORNER3)
            DrawSprite(r, &it, gx + (it.w > 0 ? -spread : spread), gy + (it.h > 0 ? -spread : spread), 0, 255);
        else if (i == ITEM_PICTURE)
        {
            if (place >= 0)
                DrawSprite(r, &it, 0, 0, PlacePicture(place), alpha);
        }
        else if (i == ITEM_WORDS)
        {
            if (place >= 0)
            {
                G_TextAlpha(alpha);
                G_Text((int)(OX + it.x * SCALE) + 2, (int)(it.y * SCALE) + 10, 0xFFFFFF, 0x303030,
                       Msg(PlaceWords(place)), (int)(it.w * SCALE) - 4, TF_NORMAL);
                G_TextAlpha(255);
            }
        }
        else if (i == ITEM_SELECT || i == ITEM_CANCEL)
        {
            const char *s = Msg(i == ITEM_SELECT ? MSG_SELECT : MSG_CANCEL);
            G_Text((int)(OX + it.x * SCALE), (int)(it.y * SCALE), 0xFFFFFF, 0x303030, s, 0, TF_NORMAL);
        }
        else if (i != 0xA41)   // (the second picture: the one before, which Kai's fade draws in the first's place)
            DrawSprite(r, &it, 0, 0, 0, 255);
        if (it.flags & 0x40)
            break;
        i = (u16)it.next;
    }
    if ((W.state == WM_OPEN || W.state == WM_CLOSE) && W.t)
    {
        SDL_SetRenderDrawColor(r, 0, 0, 0, (u8)(255 * W.t / FADE));
        SDL_RenderFillRect(r, NULL);
    }
}

SDL_Texture *Orre_WorldMap_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    u32 i;
    if (W.state == WM_NONE)
        return NULL;
    G_Begin(r);
    if (W.owner != r)
    {
        // (a new renderer: the old one's textures went with it)
        for (i = 0; i < W.nTex; i++)
            W.tex[i].t = NULL;
        W.nTex = 0;
        W.canvas = NULL;
        W.owner = r;
    }
    if (!W.canvas)
    {
        W.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CW, CH);
        if (!W.canvas)
            return NULL;
        SDL_SetTextureBlendMode(W.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(W.canvas, SDL_ScaleModeLinear);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, W.canvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    Draw(r);
    SDL_SetRenderTarget(r, W.canvas);
    W.frameOk = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, W.bigFrame, CW * 2) == 0;
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return W.canvas;
}

// the game picture while it's open (spectators, test shots): the canvas halved
const u16 *Orre_WorldMap_Frame(void)
{
    static u16 small[240 * 160];
    int x, y;
    if (W.state == WM_NONE || !W.frameOk)
        return NULL;
    for (y = 0; y < 160; y++)
        for (x = 0; x < 240; x++)
            small[y * 240 + x] = W.bigFrame[(y * 2) * CW + x * 2] & 0x7FFF;
    return small;
}

// ---- the steps

static void Close(void)
{
    W.state = WM_NONE;
    sChosen = W.chosen;
    if (W.enableScript)
        ScriptContext_Enable();
    W.enableScript = FALSE;
}

// wmapCursorMove: the first link from the cursor's place with every direction pressed, to a shown place
static void MoveCursor(u16 dirs)
{
    u8 mask = 0;
    u32 k, from = PlaceItem(W.cur);
    if (dirs & DPAD_UP)
        mask |= 1;
    if (dirs & DPAD_RIGHT)
        mask |= 2;
    if (dirs & DPAD_DOWN)
        mask |= 4;
    if (dirs & DPAD_LEFT)
        mask |= 8;
    if (!mask)
        return;
    for (k = 0; k < W.nLinks; k++)
    {
        const u8 *l = W.links + k * 12;
        s32 to;
        if ((l[0] & mask) != mask || Be32(l + 4) != from)
            continue;
        to = PlaceOfItem(Be32(l + 8));
        if (!PlaceShown(to))
            continue;
        W.prev = W.cur;
        W.cur = to;
        W.move = MOVE_TIME;
        PlaySE(SE_SELECT);
        return;
    }
}

bool8 Orre_WorldMap_IsActive(void)
{
    return W.state != WM_NONE;
}

bool8 Orre_WorldMap_Step(u16 keys)
{
    u16 pressed;
    if (W.state == WM_NONE)
        return FALSE;
    pressed = keys & ~W.prevKeys;
    W.prevKeys = keys;
    W.frame++;
    switch (W.state)
    {
    case WM_OPEN:
        if (W.t)
            W.t--;
        else
            W.state = WM_MAIN;
        break;
    case WM_MAIN:
        if (W.move > 0)
        {
            W.move -= MOVE_STEP;
            if (W.move < 0)
                W.move = 0;
            break;
        }
        if (pressed & A_BUTTON)
        {
            s32 m = PlaceMap(W.cur);
            if (m < 0)
            {
                PlaySE(SE_FAILURE);   // (a place ORRE has no map for yet)
                MPNet_Log("orre world map: room %u isn't in the game yet", PlaceRoom(W.cur));
                break;
            }
            PlaySE(SE_SELECT);
            W.chosen = m;
            W.state = WM_CLOSE;
            W.t = 0;
            MPNet_Log("orre world map: to %s", sOrreMaps[m].file);
        }
        else if (pressed & B_BUTTON)
        {
            PlaySE(SE_SELECT);
            W.chosen = -1;
            W.state = WM_CLOSE;
            W.t = 0;
            MPNet_Log("orre world map: cancelled");
        }
        else if (pressed & DPAD_ANY)
            MoveCursor(keys & DPAD_ANY);
        break;
    case WM_CLOSE:
        if (++W.t >= FADE)
            Close();
        break;
    }
    return TRUE;
}

// the place the player is at: the one whose map shares the current map's place in Colosseum (its file's first two
// letters: S1_out and S1_shop_1F are both the OUTSKIRT STAND)
static s8 CurrentPlace(void)
{
    u16 here = gSaveBlock1Ptr->location.mapGroup << 8 | gSaveBlock1Ptr->location.mapNum;
    u32 k, i;
    for (k = 0; k < ARRAY_COUNT(sOrreMaps); k++)
        if (sOrreMaps[k].map == here)
            for (i = 0; i < W.nPlaces; i++)
            {
                s32 m = PlaceMap(i);
                if (m >= 0 && PlaceShown(i) && strncmp(sOrreMaps[m].file, sOrreMaps[k].file, 2) == 0)
                    return i;
            }
    for (i = 0; i < W.nPlaces; i++)
        if (PlaceShown(i))
            return i;
    return 0;
}

// (special, waitstate) the screen
void Orre_WorldMap(void)
{
    u32 i, shown = 0;
    W.chosen = sChosen = -1;
    W.enableScript = TRUE;
    if (!LoadData())
    {
        Close();
        return;
    }
    W.cur = CurrentPlace();
    W.prev = -1;
    W.move = 0;
    W.state = WM_OPEN;
    W.t = FADE;
    W.prevKeys = 0xFFFF;   // (the step that opened it isn't a press)
    W.frame = 0;
    for (i = 0; i < W.nPlaces; i++)
        shown += PlaceShown(i);
    MPNet_Log("orre world map: open at room %u, %u of %u places shown", PlaceRoom(W.cur), shown, W.nPlaces);
}

// (special) off to the place chosen: its map's start; VAR_RESULT TRUE when it warps
void Orre_WorldMapTravel(void)
{
    gSpecialVar_Result = FALSE;
    if (sChosen < 0 || (u32)sChosen >= ARRAY_COUNT(sOrreMaps))
        return;
    SetWarpDestination(sOrreMaps[sChosen].map >> 8, sOrreMaps[sChosen].map & 0xFF, WARP_ID_NONE,
                       sOrreMaps[sChosen].startX, sOrreMaps[sChosen].startY);
    sChosen = -1;
    DoWarp();
    ResetInitialPlayerAvatarState();
    gSpecialVar_Result = TRUE;
}
