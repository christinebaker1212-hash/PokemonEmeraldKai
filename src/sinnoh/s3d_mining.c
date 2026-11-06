// Moemon Kai: the Underground's digging screen, as Platinum has it (pret/pokeplatinum src/underground/mining.c): a
// wall of 13 x 10 cells, two layers of dirt everywhere and random patches of four and six (Mining_RandomizeDirtLayers),
// two to four treasures (three the first time) picked by their weights for the Trainer ID's oddness and the NATIONAL
// DEX (each Plate dug but once, never twice in one wall), up to a hundred tries at rocks, then the PICKAXE (two off the
// cell hit, one off its four neighbours, four off the wall) and the HAMMER (two off the cell and its neighbours, one off
// the corners, eight off the wall) until every treasure lies bare or the wall's 196 runs out. A rock bared stops a
// blow's spread. The treasures bared go in the BAG ("Too bad! Your Bag is full!" otherwise); spheres aren't buried here
// (the Underground is here for its treasures and fossils only: the user, 2026-11-02; the spheres were for the secret
// bases and trades CO-OP supersedes). Its art is Platinum's (sinnoh/platinum_gfx.pak "MINE", tools/sinnoh/build_mining.py),
// its words Platinum's (underground_common); the DS's touch screen is a cursor here: the + Control Pad moves it, A
// strikes, L / R / SELECT / B change tools. Drawn at 480x320 like the BADGE CASE (sdl2.c / display.c).
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "pokedex.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"
#include "mp.h"
#include "s3d.h"
#include "s3d_underground.h"
#include "../tcg/tcg_gfx.h"

#define CW 480
#define CH 320
#define GW 13                 // (MINING_GAME_WIDTH / _HEIGHT)
#define GH 10
#define BURIED 8              // (MAX_BURIED_OBJECTS)
#define INTEGRITY 196         // (INITIAL_WALL_INTEGRITY)
#define PIC_SCREEN 0          // (build_mining.py's pictures)
#define PIC_CRACK 1
#define PIC_BUTTONS 2
#define PIC_DIRT 6
#define PIC_HIT 13
#define PIC_OBJECTS 18
#define PICS 128

bool8 S3D_Gfx_MiningPicture(u16 n, u16 *w, u16 *h, const u16 **px);
void Sinnoh_TvUndergroundTreasure(u16 item);
extern u8 gSinnohMessage[];

struct MiningObject
{
    u16 item;                 // (0xFFFF: a rock)
    u8 w, h;                  // (cells)
    u8 weights[4];            // (odd Trainer ID, even, odd with the NATIONAL DEX, even with it)
    u8 shape[36];             // (6 x 6)
};
extern const u8 gSinnohMiningObjects[];   // (sinnoh/platinum_scripts.inc, tools/sinnoh/contest_data.py mining_lines)
extern const u16 gSinnohUndergroundFlags[];   // (... and Platinum's FLAG_ENTERED_UNDERGROUND, FLAG_DIGGING_FOR_FOSSILS)

enum { MS_NONE, MS_START, MS_PINGED, MS_TUTORIAL, MS_MAIN, MS_ALL_DUG, MS_COLLAPSE, MS_COLLAPSE_FADE, MS_TALK, MS_GIVE,
       MS_FADE_OUT };

static struct
{
    bool8 active, enableScript;
    u8 phase;
    u16 prevKeys, held;
    u32 t;
    u16 timer;
    u32 rng;
    // the wall
    u8 dirt[GH][GW], grid[GH][GW];
    struct { s16 obj; u8 x, y; bool8 bare, give; u8 shine; } b[BURIED];
    u8 count, items;
    bool8 tooBad;
    u8 integrity;
    bool8 pickaxe;
    u8 cx, cy;
    // a blow
    s16 hitT;
    u8 hitX, hitY;
    bool8 hitHammer, hitRock, hitItem;
    u16 chime;
    u16 shake;
    s8 sx, sy;
    u8 fade;   // (0-16: the screen going dark)
    // the words
    char pages[10][160];
    u8 pageCount, page;
    u16 typed;
    u16 autoNext;   // (frames left before a read page goes on by itself: Platinum's 60)
    u8 afterTalk;
    // textures
    SDL_Renderer *owner;
    SDL_Texture *canvas, *ds, *pic[PICS];
    u16 bigFrame[CW * CH];
    bool8 frameOk;
} M;

static const struct MiningObject *Obj(u16 n)
{
    return (const struct MiningObject *)(gSinnohMiningObjects + 2) + n;
}

static u16 ObjCount(void)
{
    return gSinnohMiningObjects[0] | gSinnohMiningObjects[1] << 8;
}

static u32 Rand(u32 n)
{
    M.rng = M.rng * 1103515245 + 12345;
    return n ? (M.rng >> 8) % n : 0;
}

// ----------------------------------------------------------------------------------------------------- the words

static void Utf8(char *dst, int cap, const u8 *src)
{
    int n = 0;
    for (; *src != EOS && n < cap - 3; src++)
    {
        u8 c = *src;
        if (c == EXT_CTRL_CODE_BEGIN)
        {
            src += GetExtCtrlCodeLength(src[1]) - 1;
            continue;
        }
        if (c == CHAR_NEWLINE)
            dst[n++] = '\n';
        else if (c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
            dst[n++] = '\f';
        else if (c == CHAR_e_ACUTE)
            dst[n++] = (char)0xC3, dst[n++] = (char)0xA9;
        else if (c == CHAR_SGL_QUOTE_RIGHT || c == CHAR_SGL_QUOTE_LEFT)
            dst[n++] = '\'';
        else if (c == CHAR_ELLIPSIS)
            dst[n++] = '.', dst[n++] = '.', dst[n++] = '.';
        else
        {
            char a[2];
            u8 one[2] = {c, EOS};
            MP_GameToAscii(a, one, 1);
            dst[n++] = a[0];
        }
    }
    dst[n] = 0;
}

static void Say(const char *name, u8 then)
{
    static u8 expanded[900];
    static char all[1600];
    char *p, *page;
    StringExpandPlaceholders(expanded, S3D_TextByName(name));
    Utf8(all, sizeof(all), expanded);
    M.pageCount = M.page = 0;
    M.typed = 0;
    M.autoNext = 60;
    for (page = all; page && *page && M.pageCount < ARRAY_COUNT(M.pages); page = p)
    {
        p = strchr(page, '\f');
        if (p)
            *p++ = 0;
        while (*page == '\n')
            page++;
        if (*page)
        {
            snprintf(M.pages[M.pageCount], sizeof(M.pages[0]), "%s", page);
            MPNet_Log("sinnoh mining: \"%s\"", M.pages[M.pageCount]);
            M.pageCount++;
        }
    }
    M.afterTalk = then;
    M.phase = MS_TALK;
}

static bool8 Talking(void)
{
    return M.page < M.pageCount;
}

// A turns the page (a one-page message also goes on by itself a second after it is all out, as Platinum's)
static void Read(u16 pressed)
{
    if (!Talking())
        return;
    if (M.typed < strlen(M.pages[M.page]))
    {
        M.typed += 2;
        if (pressed & A_BUTTON)
            M.typed = strlen(M.pages[M.page]);
        return;
    }
    if ((pressed & (A_BUTTON | B_BUTTON)) || (M.pageCount == 1 && M.autoNext && --M.autoNext == 0))
    {
        M.page++;
        M.typed = 0;
        if (pressed & (A_BUTTON | B_BUTTON))
            PlaySE(SE_SELECT);
    }
}

// ----------------------------------------------------------------------------------------------------- the wall

static bool8 InShape(const struct MiningObject *o, int x, int y)
{
    return o->shape[y * 6 + x] != 0;
}

static bool8 TryPlace(u16 n, int x, int y)
{
    const struct MiningObject *o = Obj(n);
    int i, j;
    if (M.count >= BURIED || x + o->w > GW || y + o->h > GH)
        return FALSE;
    for (i = 0; i < o->w; i++)
        for (j = 0; j < o->h; j++)
            if (InShape(o, i, j) && M.grid[y + j][x + i] != 0)
                return FALSE;
    M.b[M.count].obj = n;
    M.b[M.count].x = x;
    M.b[M.count].y = y;
    M.b[M.count].bare = M.b[M.count].give = FALSE;
    M.b[M.count].shine = 0;
    M.count++;
    for (i = 0; i < o->w; i++)
        for (j = 0; j < o->h; j++)
            if (InShape(o, i, j))
                M.grid[y + j][x + i] = M.count;
    return TRUE;
}

static u8 WeightOf(const struct MiningObject *o)
{
    bool8 odd = (gSaveBlock2Ptr->playerTrainerId[0] | gSaveBlock2Ptr->playerTrainerId[1] << 8) & 1;
    return o->weights[(IsNationalPokedexEnabled() ? 2 : 0) + (odd ? 0 : 1)];
}

static bool8 IsPlate(u16 item)
{
    return item >= ITEM_FLAME_PLATE && item <= ITEM_IRON_PLATE;
}

static void Layout(void)
{
    u32 total = 0, tries = 0;
    u16 n = ObjCount(), i, rocks = 0;
    u16 picked[BURIED];
    for (i = 0; i < n; i++)
    {
        if (Obj(i)->item == 0xFFFF)
            rocks++;
        else
            total += WeightOf(Obj(i));
    }
    M.items = Rand(3) + 2;
    if (!Sinnoh_UndergroundEverDug())
        M.items = 3;
    while (M.count < M.items && total && ++tries < 20000)
    {
        s32 r = Rand(total);
        u16 k, item;
        bool8 again = FALSE;
        for (k = 0; k < n; k++)
            if (Obj(k)->item != 0xFFFF && (r -= WeightOf(Obj(k))) < 0)
                break;
        if (k >= n)
            continue;
        item = Obj(k)->item;
        if (IsPlate(item))
        {
            if (Sinnoh_UndergroundPlateDug(item - ITEM_FLAME_PLATE))
                continue;
            for (i = 0; i < M.count; i++)
                if (picked[i] == item)
                    again = TRUE;
            if (again)
                continue;
        }
        picked[M.count] = item;
        TryPlace(k, Rand(GW), Rand(GH));
    }
    M.items = M.count;
    if (Sinnoh_UndergroundEverDug())
        for (i = 0; i < 100 && rocks; i++)
            TryPlace(n - rocks + Rand(rocks), Rand(GW), Rand(GH));
}

static void DirtLayers(void)
{
    static const u8 four[8][8] = {
        {0, 0, 4, 4, 4, 4, 0, 0}, {0, 4, 4, 4, 4, 4, 4, 0}, {4, 4, 4, 4, 4, 4, 0, 0}, {4, 4, 4, 4, 4, 4, 0, 0},
        {4, 4, 4, 4, 4, 4, 0, 0}, {4, 4, 4, 4, 4, 4, 0, 0}, {0, 4, 4, 4, 4, 4, 4, 0}, {0, 0, 4, 4, 4, 4, 0, 0},
    };
    static const u8 six[5][5] = {
        {0, 6, 6, 6, 0}, {6, 6, 6, 6, 6}, {6, 6, 6, 6, 6}, {6, 6, 6, 6, 6}, {0, 6, 6, 6, 0},
    };
    int i, x, y, sx, sy;
    for (i = 0; i < 10; i++)
    {
        sx = (int)Rand(GW + 8) - 8;
        sy = (int)Rand(GH + 8) - 5;   // (Platinum's own: the patch's height takes the six's)
        for (y = sy; y < sy + 8; y++)
            for (x = sx; x < sx + 8; x++)
                if (y >= 0 && y < GH && x >= 0 && x < GW && four[y - sy][x - sx])
                    M.dirt[y][x] = four[y - sy][x - sx];
    }
    for (i = 0; i < 15; i++)
    {
        bool8 ok = TRUE;
        sx = (int)Rand(GW + 5) - 5;
        sy = (int)Rand(GH + 5) - 5;
        for (y = sy; y < sy + 5 && ok; y++)
            for (x = sx; x < sx + 5; x++)
                if (y >= 0 && y < GH && x >= 0 && x < GW && four[y - sy][x - sx] && M.dirt[y][x] < 4)
                {
                    ok = FALSE;   // (Platinum checks the four's corner of the patch, as here)
                    break;
                }
        if (!ok)
            continue;
        for (y = sy; y < sy + 5; y++)
            for (x = sx; x < sx + 5; x++)
                if (y >= 0 && y < GH && x >= 0 && x < GW && six[y - sy][x - sx])
                    M.dirt[y][x] = six[y - sy][x - sx];
    }
}

static bool8 RockAt(int x, int y)
{
    return M.grid[y][x] && Obj(M.b[M.grid[y][x] - 1].obj)->item == 0xFFFF;
}

static bool8 ItemAt(int x, int y)
{
    return M.grid[y][x] && Obj(M.b[M.grid[y][x] - 1].obj)->item != 0xFFFF;
}

static void Off(int x, int y, u8 n)
{
    if (x < 0 || x >= GW || y < 0 || y >= GH)
        return;
    while (n-- && M.dirt[y][x])
        M.dirt[y][x]--;
}

// a blow at X, Y (Mining_RemoveDirt)
static void Strike(int x, int y)
{
    static const s8 diag[4][2] = {{1, 1}, {-1, -1}, {-1, 1}, {1, -1}};
    static const s8 adj[4][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};
    int i;
    Off(x, y, 2);
    M.hitRock = RockAt(x, y) && M.dirt[y][x] == 0;
    M.hitItem = ItemAt(x, y) && M.dirt[y][x] == 0;
    M.hitX = x;
    M.hitY = y;
    M.hitHammer = !M.pickaxe;
    M.hitT = 0;
    if (M.hitItem)
        M.chime = 15;
    if (M.hitRock)
        PlaySE(SE_M_COMET_PUNCH);
    else
        PlaySE(M.pickaxe ? SE_M_ROCK_THROW : SE_M_STRENGTH);
    if (!M.hitRock)
    {
        if (!M.pickaxe)
            for (i = 0; i < 4; i++)
                Off(x + diag[i][0], y + diag[i][1], 1);
        for (i = 0; i < 4; i++)
            Off(x + adj[i][0], y + adj[i][1], M.pickaxe ? 1 : 2);
    }
    M.integrity = M.integrity > (M.pickaxe ? 4 : 8) ? M.integrity - (M.pickaxe ? 4 : 8) : 0;
    M.shake = 1;
}

// every treasure bare? (the ones just bared shine)
static bool8 AllBare(void)
{
    bool8 bare[BURIED], all = TRUE;
    int x, y, i;
    for (i = 0; i < M.items; i++)
        bare[i] = TRUE;
    for (y = 0; y < GH; y++)
        for (x = 0; x < GW; x++)
            if (M.grid[y][x] && M.grid[y][x] <= M.items && M.dirt[y][x])
                bare[M.grid[y][x] - 1] = FALSE;
    for (i = 0; i < M.items; i++)
    {
        if (!bare[i])
            all = FALSE;
        else if (!M.b[i].bare)
        {
            M.b[i].bare = M.b[i].give = TRUE;
            M.b[i].shine = 1;
        }
    }
    return all;
}

static void ItemName(u8 *dst, u16 item)
{
    u8 name[40];
    CopyItemName(item, name);
    // (Platinum's "{an item} was obtained." with the article capitalized)
    switch (name[0])
    {
    case CHAR_A: case CHAR_E: case CHAR_I: case CHAR_O: case CHAR_U:
    {
        static const u8 sAn[] = {CHAR_A, CHAR_n, CHAR_SPACE, EOS};
        StringCopy(dst, sAn);
        break;
    }
    default:
    {
        static const u8 sA[] = {CHAR_A, CHAR_SPACE, EOS};
        StringCopy(dst, sA);
        break;
    }
    }
    StringAppend(dst, name);
}

// the next treasure bared: "... was obtained." (and Platinum's "Too bad! Your Bag is full!" after it when there is no
// room; FALSE: none left)
static bool8 NextTreasure(void)
{
    int i;
    if (M.tooBad)
    {
        M.tooBad = FALSE;
        Say("UndergroundCommon_Text_TooBadBagIsFull3", MS_GIVE);
        return TRUE;
    }
    for (i = 0; i < M.items; i++)
        if (M.b[i].give)
        {
            u16 item = Obj(M.b[i].obj)->item;
            M.b[i].give = FALSE;
            ItemName(gStringVar3, item);
            Sinnoh_TvUndergroundTreasure(item);
            if (gSinnohUndergroundFlags[1])
                FlagSet(gSinnohUndergroundFlags[1]);   // (the UNDERGROUND MAN's second mission done)
            if (CheckBagHasSpace(item, 1))
            {
                AddBagItem(item, 1);
                if (IsPlate(item))
                    Sinnoh_UndergroundSetPlateDug(item - ITEM_FLAME_PLATE);
                MPNet_Log("sinnoh mining: %u in the bag", item);
            }
            else
                M.tooBad = TRUE;
            Say("UndergroundCommon_Text_ItemWasObtained", MS_GIVE);
            return TRUE;
        }
    return FALSE;
}

// ----------------------------------------------------------------------------------------------------- textures

static u32 Argb(u16 c)
{
    return 0xFF000000 | ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}

static SDL_Texture *Pic(u16 n, u16 *w, u16 *h)
{
    static u32 px[256 * 192];
    const u16 *src;
    u32 i;
    if (!S3D_Gfx_MiningPicture(n, w, h, &src) || n >= PICS || (u32)*w * *h > ARRAY_COUNT(px))
        return NULL;
    if (M.pic[n])
        return M.pic[n];
    for (i = 0; i < (u32)*w * *h; i++)
        px[i] = (src[i] & 0x8000) ? Argb(src[i]) : 0;
    M.pic[n] = SDL_CreateTexture(M.owner, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, *w, *h);
    if (M.pic[n])
    {
        SDL_UpdateTexture(M.pic[n], NULL, px, *w * 4);
        SDL_SetTextureBlendMode(M.pic[n], SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(M.pic[n], SDL_ScaleModeNearest);
    }
    return M.pic[n];
}

static void Blit(u16 n, int sx, int sy, int sw, int sh, int x, int y)
{
    u16 w, h;
    SDL_Texture *t = Pic(n, &w, &h);
    SDL_Rect s = {sx, sy, sw ? sw : w, sh ? sh : h}, d = {x, y, s.w, s.h};
    if (t)
        SDL_RenderCopy(G_R(), t, &s, &d);
}

// ----------------------------------------------------------------------------------------------------- drawing

static void TextBox(void)
{
    char shown[160];
    if (!Talking())
        return;
    G_RoundFill(4, 146, 248, 42, 0x383848, 255);
    G_RoundFill(5, 147, 246, 40, 0xf8f8f0, 255);
    snprintf(shown, sizeof(shown), "%.*s", M.typed, M.pages[M.page]);
    G_Text(11, 150, 0x303030, G_NO_SHADOW, shown, 236, TF_NORMAL);
    if (M.typed >= strlen(M.pages[M.page]) && (M.t / 16) & 1)
        G_Arrow(240, 180, 1, 0x303030);
}

static void DrawWall(void)
{
    int x, y, i, crack, ox = M.sx, oy = M.sy;
    u8 integ = M.integrity / 4 * 4 + 8;
    Blit(PIC_SCREEN, 0, 0, 0, 0, ox, oy);
    crack = (GW * 16 - integ) / 8;
    if (crack > 0)
        Blit(PIC_CRACK, (26 - crack) * 8, 0, crack * 8, 32, ox + (26 - crack) * 8, oy);
    // the tools (the one in hand pressed)
    Blit(PIC_BUTTONS + (M.pickaxe ? 1 : 0), 0, 0, 0, 0, ox + 26 * 8, oy + 6 * 8);
    Blit(PIC_BUTTONS + (M.pickaxe ? 2 : 3), 0, 0, 0, 0, ox + 26 * 8, oy + 15 * 8);
    // what is buried, cell by cell of its shape
    for (i = 0; i < M.count; i++)
    {
        const struct MiningObject *o = Obj(M.b[i].obj);
        int cx, cy;
        for (cy = 0; cy < o->h; cy++)
            for (cx = 0; cx < o->w; cx++)
                if (InShape(o, cx, cy))
                    Blit(PIC_OBJECTS + M.b[i].obj, cx * 16, cy * 16, 16, 16, ox + (M.b[i].x + cx) * 16,
                         oy + (M.b[i].y + cy + 2) * 16);
        if (M.b[i].shine && M.b[i].shine < 40)
            for (cy = 0; cy < o->h; cy++)
                for (cx = 0; cx < o->w; cx++)
                    if (InShape(o, cx, cy))
                        G_FillAdd(ox + (M.b[i].x + cx) * 16, oy + (M.b[i].y + cy + 2) * 16, 16, 16, 0xffffff,
                                  (40 - M.b[i].shine) * 4);
    }
    // the dirt over it
    for (y = 0; y < GH; y++)
        for (x = 0; x < GW; x++)
            if (M.dirt[y][x])
                Blit(PIC_DIRT + (M.dirt[y][x] > 6 ? 6 : M.dirt[y][x]), 0, 0, 0, 0, ox + x * 16, oy + (y + 2) * 16);
    // the cursor (the touch screen's stylus)
    if (M.phase == MS_MAIN)
    {
        unsigned c = (M.t / 12) & 1 ? 0xf8f8f8 : 0x282828;
        G_Frame(ox + M.cx * 16, oy + (M.cy + 2) * 16, 16, 16, c, 255);
        G_Frame(ox + M.cx * 16 + 1, oy + (M.cy + 2) * 16 + 1, 14, 14, c ^ 0xd0d0d0, 160);
    }
    // a blow: the tool swung, its spark
    if (M.hitT >= 0 && M.hitT < 24)
    {
        int px = ox + M.hitX * 16 + 8, py = oy + (M.hitY + 2) * 16 + 8;
        u8 frame = (M.hitT / 4) & 1;
        Blit(PIC_HIT + (M.hitHammer ? 0 : 2) + frame, 0, 0, 0, 0, px - 4, py - 32);
        if (M.hitT >= 2 && M.hitT < 12)
            Blit(PIC_HIT + 4, 0, 0, 0, 0, px - 16, py - 16);
    }
}

static void Draw(void)
{
    SDL_Renderer *r = G_R();
    SDL_Rect dst = {27, 0, 426, 320};
    if (!M.ds)
    {
        M.ds = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 192);
        if (!M.ds)
            return;
        SDL_SetTextureBlendMode(M.ds, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(M.ds, SDL_ScaleModeLinear);
    }
    SDL_SetRenderTarget(r, M.ds);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    DrawWall();
    if (M.fade)
        G_FillA(0, 0, 256, 192, 0x000000, M.fade >= 16 ? 255 : M.fade * 16);
    TextBox();
    SDL_SetRenderTarget(r, M.canvas);
    SDL_SetRenderDrawColor(r, 0x10, 0x10, 0x14, 255);
    SDL_RenderClear(r);
    SDL_RenderCopy(r, M.ds, NULL, &dst);
}

SDL_Texture *Sinnoh_Mining_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!M.active)
        return NULL;
    G_Begin(r);
    if (M.owner != r)
    {
        memset(M.pic, 0, sizeof(M.pic));
        M.canvas = M.ds = NULL;
        M.owner = r;
    }
    if (!M.canvas)
    {
        M.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, CW, CH);
        if (!M.canvas)
            return NULL;
        SDL_SetTextureBlendMode(M.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(M.canvas, SDL_ScaleModeLinear);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, M.canvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_RenderSetScale(r, 1, 1);
    Draw();
    SDL_SetRenderTarget(r, M.canvas);
    M.frameOk = SDL_RenderReadPixels(r, NULL, SDL_PIXELFORMAT_ABGR1555, M.bigFrame, CW * 2) == 0;
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return M.canvas;
}

// the game picture while digging (spectators, test shots): the canvas halved
const u16 *Sinnoh_Mining_Frame(void)
{
    static u16 small[240 * 160];
    int x, y;
    if (!M.active || !M.frameOk)
        return NULL;
    for (y = 0; y < 160; y++)
        for (x = 0; x < 240; x++)
            small[y * 240 + x] = M.bigFrame[(y * 2) * CW + x * 2] & 0x7FFF;
    return small;
}

// ----------------------------------------------------------------------------------------------------- the steps

// the + Control Pad's repeat for the cursor
static u16 Repeat(u16 keys, u16 pressed)
{
    u16 dirs = keys & DPAD_ANY;
    if (!dirs)
    {
        M.held = 0;
        return 0;
    }
    if (pressed & DPAD_ANY)
    {
        M.held = 1;
        return pressed & DPAD_ANY;
    }
    if (++M.held > 16 && (M.held - 16) % 4 == 0)
        return dirs;
    return 0;
}

static void Shake(void)
{
    int dur = (INTEGRITY - M.integrity) / 15, mag = 3 + (INTEGRITY - M.integrity) / 50;
    if (!M.shake)
    {
        M.sx = M.sy = 0;
        return;
    }
    if (++M.shake > dur)
    {
        M.sx = M.sy = 0;
        M.shake = 0;
        return;
    }
    M.sx = (s8)(Rand(mag) - mag / 2);
    M.sy = (s8)(Rand(mag) - mag / 2);
}

static void StepMain(u16 keys, u16 pressed)
{
    u16 dirs = Repeat(keys, pressed);
    if (dirs & DPAD_LEFT)
        M.cx = M.cx ? M.cx - 1 : GW - 1;
    if (dirs & DPAD_RIGHT)
        M.cx = M.cx + 1 < GW ? M.cx + 1 : 0;
    if (dirs & DPAD_UP)
        M.cy = M.cy ? M.cy - 1 : GH - 1;
    if (dirs & DPAD_DOWN)
        M.cy = M.cy + 1 < GH ? M.cy + 1 : 0;
    if (pressed & (L_BUTTON | R_BUTTON | SELECT_BUTTON | B_BUTTON))
    {
        M.pickaxe = !M.pickaxe;
        PlaySE(SE_SELECT);
    }
    if (pressed & A_BUTTON)
        Strike(M.cx, M.cy);
    if (M.chime && --M.chime == 0)
        PlaySE(SE_SHINY);
    if (AllBare())
    {
        Sinnoh_UndergroundDugOnce();
        M.phase = MS_ALL_DUG;
        M.timer = 25;
        MPNet_Log("sinnoh mining: everything dug up (wall %u)", M.integrity);
    }
    else if (M.integrity == 0)
    {
        Sinnoh_UndergroundDugOnce();
        M.phase = MS_COLLAPSE;
        M.timer = 45;
        MPNet_Log("sinnoh mining: the wall collapsed");
    }
}

static void Close(void)
{
    M.active = FALSE;
    M.phase = MS_NONE;
    if (M.enableScript)
        ScriptContext_Enable();
    M.enableScript = FALSE;
}

bool8 Sinnoh_Mining_IsActive(void)
{
    return M.active;
}

bool8 Sinnoh_Mining_Step(u16 keys)
{
    u16 pressed;
    int i;
    if (!M.active)
        return FALSE;
    pressed = keys & ~M.prevKeys;
    M.prevKeys = keys;
    M.t++;
    if (M.hitT >= 0 && ++M.hitT > 40)
        M.hitT = -1;
    for (i = 0; i < M.count; i++)
        if (M.b[i].shine && M.b[i].shine < 40)
            M.b[i].shine++;
    if (M.phase != MS_COLLAPSE)
        Shake();
    switch (M.phase)
    {
    case MS_START:
        if (M.fade)
            M.fade--;
        else if (++M.timer > 20)
        {
            ConvertIntToDecimalStringN(gStringVar1, M.items, STR_CONV_MODE_LEFT_ALIGN, 1);
            Say("UndergroundCommon_Text_SomethingPingedInWall", Sinnoh_UndergroundEverDug() ? MS_MAIN : MS_TUTORIAL);
            M.autoNext = 80;
        }
        break;
    case MS_TUTORIAL:
        Say("UndergroundCommon_Text_MiningTutorial", MS_MAIN);
        break;
    case MS_TALK:
        Read(pressed);
        if (!Talking())
        {
            M.phase = M.afterTalk;
            if (M.phase == MS_GIVE && !NextTreasure())
                M.phase = MS_FADE_OUT;
        }
        break;
    case MS_MAIN:
        StepMain(keys, pressed);
        break;
    case MS_ALL_DUG:
        if (--M.timer == 0)
        {
            PlaySE(SE_SUCCESS);
            Say("UndergroundCommon_Text_EverythingDugUp", MS_GIVE);
        }
        break;
    case MS_COLLAPSE:
        M.shake = 1;
        Shake();
        M.shake = 1;
        if (--M.timer == 0)
        {
            M.phase = MS_COLLAPSE_FADE;
            M.timer = 0;
            M.sx = M.sy = 0;
            PlaySE(SE_M_EARTHQUAKE);
        }
        break;
    case MS_COLLAPSE_FADE:
        if (M.fade < 16)
            M.fade++;
        else
        {
            AllBare();   // (what lay bare when it fell is still had)
            Say("UndergroundCommon_Text_WallCollapsed", MS_GIVE);
        }
        break;
    case MS_GIVE:
        if (!NextTreasure())
            M.phase = MS_FADE_OUT;
        break;
    case MS_FADE_OUT:
        if (M.fade < 16)
            M.fade++;
        else
            Close();
        break;
    }
    return TRUE;
}

// ----------------------------------------------------------------------------------------------------- the field

// (special) "The wall is bulging slightly. Try digging into it?"
void Sinnoh_UndergroundBulging(void)
{
    StringCopy(gSinnohMessage, S3D_TextByName("UndergroundCommon_Text_WallIsBulging"));
}

// (special, waitstate) the digging, for the spot faced (s3d_underground.c)
void Sinnoh_UndergroundDig(void)
{
    s32 spot = Sinnoh_UndergroundDigSpot();
    {
        void Sinnoh_JournalDug(void);
        Sinnoh_JournalDug();   // (the JOURNAL: "Dug for Treasure underground.": s3d_journal.c)
    }
    memset(M.dirt, 2, sizeof(M.dirt));
    memset(M.grid, 0, sizeof(M.grid));
    M.count = M.items = 0;
    M.tooBad = FALSE;
    M.rng = Random32() ^ (u32)spot * 2654435761u;
    Layout();
    DirtLayers();
    M.integrity = INTEGRITY;
    M.pickaxe = TRUE;
    M.cx = GW / 2;
    M.cy = GH / 2;
    M.hitT = -1;
    M.chime = M.shake = 0;
    M.sx = M.sy = 0;
    M.fade = 16;
    M.timer = 0;
    M.pageCount = M.page = 0;
    M.prevKeys = 0xFFFF;
    M.t = 0;
    M.phase = MS_START;
    M.active = TRUE;
    M.enableScript = TRUE;
    Sinnoh_UndergroundSpotDug(spot);   // (a spot dug is gone, whatever comes of it)
    PlaySE(SE_M_DIG);
    MPNet_Log("sinnoh mining: spot %d, %u treasures, %u buried", spot, M.items, M.count);
}

// ----------------------------------------------------------------------------------------------------- tests

void Sinnoh_MiningDebug(void)
{
    int i;
    printf("[test] mining: active %d phase %u wall %u tool %s cursor %u,%u items %u buried %u:", M.active, M.phase,
           M.integrity, M.pickaxe ? "pickaxe" : "hammer", M.cx, M.cy, M.items, M.count);
    for (i = 0; i < M.count; i++)
        printf(" %u@%u,%u%s", Obj(M.b[i].obj)->item, M.b[i].x, M.b[i].y, M.b[i].bare ? "(bare)" : "");
    printf("\n");
}

// a cell over a treasure still under dirt, the one whose blow would bare the most of the treasures (the cursor goes
// there)
bool8 Sinnoh_MiningTestCell(u8 *x, u8 *y)
{
    int i, j, dx, dy, best = -1;
    for (j = 0; j < GH; j++)
        for (i = 0; i < GW; i++)
            if (M.grid[j][i] && M.grid[j][i] <= M.items && M.dirt[j][i])
            {
                int score = 0;
                for (dy = -1; dy <= 1; dy++)
                    for (dx = -1; dx <= 1; dx++)
                    {
                        int xx = i + dx, yy = j + dy;
                        if (xx >= 0 && xx < GW && yy >= 0 && yy < GH && M.grid[yy][xx] && M.grid[yy][xx] <= M.items)
                            score += M.dirt[yy][xx] > 2 ? 2 : M.dirt[yy][xx];
                    }
                if (score > best)
                {
                    best = score;
                    *x = i;
                    *y = j;
                }
            }
    if (best < 0)
        return FALSE;
    M.cx = *x;
    M.cy = *y;
    return TRUE;
}
