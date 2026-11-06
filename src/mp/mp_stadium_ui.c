// POKéMON STADIUM's screens (docs/STADIUM.md): Stadium's flow in the game's own style, drawn in a 480x320 picture
// like the BADGE CASE (display.c; the game waits while it's open: sdl2.c ScreenStep). The cups' pictures and the
// classes' banners are Stadium's own (graphics/stadium/stadium.bin); trainers and POKéMON are the game's pictures.
//
//   SELECT    the cups (L / R: Round 1 / Round 2), their rules and the classes won
//   CLASS     POKé / GREAT / ULTRA / MASTER BALL (Stadium's banners), the next one open once one is won
//   OPTIONS   the user's switches: LEVEL CAP, SPECIES, HELD ITEMS (RENTALS ONLY counted from the team), START
//   REGISTER  six from the PARTY, the BOXES, the GLOBAL PC or the RENTALS (the CHALLENGE CUP lends its six)
//   VS        the opponent and their six, the clauses, the player's six: pick three in order, START battles
//   RESULT    won / lost (a continue), the cup won
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "global.h"
#include "main.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"
#include "mp_stadium_teams.h"
#include "data.h"
#include "decompress.h"
#include "graphics.h"
#include "palette.h"
#include "gpu_regs.h"
#include "constants/rgb.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "pokemon_storage_system.h"
#include "pokemon_summary_screen.h"
#include "overworld.h"
#include "task.h"
#include "battle_main.h"
#include "item.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "../tcg/tcg_gfx.h"

#define SU_W 480
#define SU_H 320
#define C_BG       0x203058
#define C_BG2      0x284070
#define C_PANEL    0xf8f0d8
#define C_EDGE     0x98a0c0
#define C_INK      0x383838
#define C_SOFT     0x707080
#define C_GOLD     0xe0b030
#define C_RED      0xd04838
#define C_GREEN    0x40a050
#define C_LOCK     0x606878

extern const u8 gMpStadiumGfx[];
const u16 *MP_CardScreen_Halve(SDL_Renderer *r, SDL_Texture *canvas, u16 *out);

enum { SCR_SELECT, SCR_CLASS, SCR_OPTIONS, SCR_REGISTER, SCR_VS, SCR_RESULT, SCR_TEAMS };
enum { SRC_TAB_PARTY, SRC_TAB_BOXES, SRC_TAB_GLOBAL, SRC_TAB_RENTALS, SRC_TAB_COUNT };
enum { RES_WON, RES_LOST, RES_CONTINUE, RES_CUP, RES_OVER };

#define LIST_MAX 512
#define SPRITE_CACHE 48

struct SpriteTex { u32 key; SDL_Texture *tex; u32 age; };

static struct
{
    bool8 active, draining, pending;
    u16 prevKeys;
    u8 screen;
    u8 round, cup, tier;
    u8 opt;                 // the switches (STADIUM_OPT_*)
    u8 optCursor;
    u8 tab, listCursor;
    u16 listTop;
    u8 vsCursor;
    u8 result;
    u8 reopen;              // the screen to come back to after a battle / the SUMMARY
    bool8 menu, rules;      // the REGISTER pop-up (A), the clauses page (VS: SELECT)
    u8 menuCursor;
    u8 teamCursor, teamTop;   // SAVED TEAMS
    u16 lastFoePic;           // RESULT: the trainer just battled (the run has moved on to the next)
    bool8 confirm;            // SAVED TEAMS: ERASE asked
    u16 age;
    char note[96];          // a line at the bottom (why not, what happened)
    // the registration list for the tab: what's there and where from
    u16 listCount;
    struct { u8 source; u16 rental; struct BoxPokemon mon; } list[LIST_MAX];
    SDL_Renderer *owner;
    SDL_Texture *canvas, *pics[24], *greyBalls[4];
    bool8 hasCastle;
    struct SpriteTex sprites[SPRITE_CACHE];
    u32 spriteAge;
    u16 frame[240 * 160];
    bool8 frameRead;
} U;

// ---------------------------------------------------------------------------------------------------------------
// Pictures

static u32 Rgb555(u16 c)
{
    return 0xFF000000 | ((c & 31) * 255 / 31) << 16 | (((c >> 5) & 31) * 255 / 31) << 8 | (((c >> 10) & 31) * 255 / 31);
}

// stadium.bin's pictures (tools/stadium/extract_gfx.py)
#define PIC_BALL(t)   (4 + (t))
#define PIC_AREA(n)   (8 + (n))    // the areas' name signs
#define PIC_PLATE(n)  (18 + (n))   // name plates
enum { AREA_STADIUM, AREA_FREE, AREA_CASTLE, AREA_MEWTWO };
#define PIC_CHALLENGE 22           // the pictures the user sent (STADIUM 2's)
#define PIC_CASTLE    23

// Stadium's pictures from stadium.bin (RGBA 5551)
static void MakePics(SDL_Renderer *r)
{
    const u8 *p = gMpStadiumGfx;
    u16 count, i;
    if (memcmp(p, "STDM", 4) != 0)
        return;
    count = p[4] | (p[5] << 8);
    p += 6;
    for (i = 0; i < count && i < ARRAY_COUNT(U.pics); i++)
    {
        u16 w = p[0] | (p[1] << 8), h = p[2] | (p[3] << 8);
        static u32 px[256 * 96];
        u32 k;
        p += 4;
        for (k = 0; k < (u32)w * h && k < ARRAY_COUNT(px); k++)
        {
            u16 v = p[k * 2] | (p[k * 2 + 1] << 8);
            px[k] = (v & 1 ? 0xFF000000u : 0) | (((v >> 11) & 31) * 255 / 31) << 16 | (((v >> 6) & 31) * 255 / 31) << 8
                  | (((v >> 1) & 31) * 255 / 31);
        }
        if (i == PIC_CASTLE)
            U.hasCastle = (px[0] >> 24) != 0;
        if (i >= PIC_BALL(0) && i <= PIC_BALL(3))   // (and a grey one: a class not open yet)
        {
            static u32 grey[256 * 96];
            u32 k2;
            for (k2 = 0; k2 < (u32)w * h && k2 < ARRAY_COUNT(grey); k2++)
            {
                u32 c = px[k2], l = (((c >> 16) & 255) * 30 + ((c >> 8) & 255) * 59 + (c & 255) * 11) / 100;
                l = l * 3 / 4 + 20;
                grey[k2] = (c & 0xFF000000u) | l << 16 | l << 8 | l;
            }
            U.greyBalls[i - PIC_BALL(0)] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
            if (U.greyBalls[i - PIC_BALL(0)])
            {
                SDL_UpdateTexture(U.greyBalls[i - PIC_BALL(0)], NULL, grey, w * 4);
                SDL_SetTextureBlendMode(U.greyBalls[i - PIC_BALL(0)], SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(U.greyBalls[i - PIC_BALL(0)], SDL_ScaleModeLinear);
            }
        }   // (a clear stand-in until the user's picture is in graphics/stadium)
        p += w * h * 2;
        U.pics[i] = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
        if (U.pics[i])
        {
            SDL_UpdateTexture(U.pics[i], NULL, px, w * 4);
            SDL_SetTextureBlendMode(U.pics[i], SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(U.pics[i], SDL_ScaleModeLinear);
        }
    }
}

// The picture each cup shows (Stadium's for the four cups; the castle, the CHALLENGE CUP and MEWTWO: their POKéMON)
static SDL_Texture *CupPic(u8 cup)
{
    static const s8 sPic[STADIUM_CUP_COUNT] = {[STADIUM_CUP_PIKA] = 3, [STADIUM_CUP_PETIT] = 2, [STADIUM_CUP_POKE] = 0,
                                               [STADIUM_CUP_PRIME] = 1, [STADIUM_CUP_CHALLENGE] = -1,
                                               [STADIUM_CUP_CASTLE] = -1, [STADIUM_CUP_MEWTWO] = -1};
    if (cup == STADIUM_CUP_CHALLENGE)
        return U.pics[PIC_CHALLENGE];
    if (cup == STADIUM_CUP_CASTLE)
        return U.hasCastle ? U.pics[PIC_CASTLE] : NULL;
    return sPic[cup] >= 0 ? U.pics[(u8)sPic[cup]] : NULL;
}

static SDL_Texture *Cached(u32 key)
{
    u8 i;
    for (i = 0; i < SPRITE_CACHE; i++)
        if (U.sprites[i].tex && U.sprites[i].key == key)
        {
            U.sprites[i].age = ++U.spriteAge;
            return U.sprites[i].tex;
        }
    return NULL;
}

static SDL_Texture *Store(u32 key, const u32 *px, int w, int h)
{
    u8 i, old = 0;
    for (i = 0; i < SPRITE_CACHE; i++)
    {
        if (!U.sprites[i].tex)
        {
            old = i;
            break;
        }
        if (U.sprites[i].age < U.sprites[old].age)
            old = i;
    }
    if (U.sprites[old].tex)
        SDL_DestroyTexture(U.sprites[old].tex);
    U.sprites[old].tex = SDL_CreateTexture(U.owner, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
    U.sprites[old].key = key;
    U.sprites[old].age = ++U.spriteAge;
    if (U.sprites[old].tex)
    {
        SDL_UpdateTexture(U.sprites[old].tex, NULL, px, w * 4);
        SDL_SetTextureBlendMode(U.sprites[old].tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(U.sprites[old].tex, SDL_ScaleModeNearest);
    }
    return U.sprites[old].tex;
}

// A POKéMON's menu icon (first frame)
static SDL_Texture *IconTex(u16 species)
{
    static u32 px[32 * 32];
    const u8 *tiles;
    const u16 *pal;
    SDL_Texture *t;
    int tile, i;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return NULL;
    if ((t = Cached(0x10000 | species)))
        return t;
    tiles = GetMonIconTiles(species, 0);
    pal = gMonIconPalettes[GetMonIconPaletteIndexFromSpecies(species)];
    if (!tiles || !pal)
        return NULL;
    for (tile = 0; tile < 16; tile++)
        for (i = 0; i < 64; i++)
        {
            u8 b = tiles[tile * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
            int x = (tile % 4) * 8 + i % 8, y = (tile / 4) * 8 + i / 8;
            px[y * 32 + x] = v ? Rgb555(pal[v]) : 0;
        }
    return Store(0x10000 | species, px, 32, 32);
}

// A POKéMON's battle picture
static SDL_Texture *FrontTex(u16 species, u32 personality, u32 otId)
{
    static u8 tiles[MON_PIC_SIZE * 4];
    static u16 pal[64];
    static u32 px[64 * 64];
    u32 key = 0x20000000u | species;
    SDL_Texture *t;
    int x, y;
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return NULL;
    if ((t = Cached(key)))
        return t;
    HandleLoadSpecialPokePic(&gMonFrontPicTable[species], tiles, species, personality);
    LZ77UnCompWram(GetMonSpritePalFromSpeciesAndPersonality(species, otId, personality), pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 b = tiles[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 v = (x & 1) ? b >> 4 : b & 15;
            px[y * 64 + x] = v ? Rgb555(pal[v]) : 0;
        }
    return Store(key, px, 64, 64);
}

// A trainer's battle picture
static SDL_Texture *TrainerTex(u16 pic)
{
    static u8 tiles[64 * 64 / 2];
    static u16 pal[16];
    static u32 px[64 * 64];
    u32 key = 0x40000000u | pic;
    SDL_Texture *t;
    int x, y;
    if ((t = Cached(key)))
        return t;
    LZ77UnCompWram(gTrainerFrontPicTable[pic].data, tiles);
    LZ77UnCompWram(gTrainerFrontPicPaletteTable[pic].data, pal);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            u8 b = tiles[((y / 8) * 8 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
            u8 v = (x & 1) ? b >> 4 : b & 15;
            px[y * 64 + x] = v ? Rgb555(pal[v]) : 0;
        }
    return Store(key, px, 64, 64);
}

// Draw a texture at a 240x160 position (the canvas is twice that: half-pixel detail)
static void Blit(SDL_Texture *t, float x, float y, float w, float h, u8 alpha)
{
    SDL_FRect d = {x * 2, y * 2, w * 2, h * 2};
    if (!t)
        return;
    SDL_RenderSetScale(U.owner, 1, 1);
    SDL_SetTextureAlphaMod(t, alpha);
    SDL_RenderCopyF(U.owner, t, NULL, &d);
    SDL_SetTextureAlphaMod(t, 255);
    SDL_RenderSetScale(U.owner, 2, 2);
}

static void Ascii(char *out, const u8 *game, int n)
{
    MP_GameToAscii(out, game, n);
}

// ---------------------------------------------------------------------------------------------------------------
// The registration list


static const u32 sTypeRgb[NUMBER_OF_MON_TYPES] = {
    [TYPE_NORMAL] = 0xa8a878, [TYPE_FIGHTING] = 0xc03028, [TYPE_FLYING] = 0xa890f0, [TYPE_POISON] = 0xa040a0,
    [TYPE_GROUND] = 0xe0c068, [TYPE_ROCK] = 0xb8a038, [TYPE_BUG] = 0xa8b820, [TYPE_GHOST] = 0x705898,
    [TYPE_STEEL] = 0xb8b8d0, [TYPE_MYSTERY] = 0x68a090, [TYPE_FIRE] = 0xf08030, [TYPE_WATER] = 0x6890f0,
    [TYPE_GRASS] = 0x78c850, [TYPE_ELECTRIC] = 0xf8d030, [TYPE_PSYCHIC] = 0xf85888, [TYPE_ICE] = 0x98d8d8,
    [TYPE_DRAGON] = 0x7038f8, [TYPE_DARK] = 0x705848, [TYPE_FAIRY] = 0xee99ac,
};

static unsigned TypeRgb(u8 type)
{
    return type < NUMBER_OF_MON_TYPES ? sTypeRgb[type] : 0x808080;
}

// A raised panel: a soft shadow, an edge, a lit top line
static void Panel(int x, int y, int w, int h, unsigned fill, unsigned edge)
{
    G_RoundFill(x + 1, y + 2, w, h, 0x000000, 90);
    G_RoundFill(x, y, w, h, edge, 255);
    G_RoundFill(x + 1, y + 1, w - 2, h - 2, fill, 255);
    G_FillA(x + 3, y + 1, w - 6, 1, 0xffffff, 60);
}

u8 MP_TypeIconPal(u8 type);   // pokemon_summary_screen.c

// A type's icon as the SUMMARY draws it (32x16, shown at its own pixels: 16x8 here)
static SDL_Texture *TypeIconTex(u8 type)
{
    static u8 sTiles[(NUMBER_OF_MON_TYPES + 5) * 0x100];
    static u16 sPals[16 * 3];
    static bool8 sLoaded;
    static u32 px[32 * 16];
    SDL_Texture *t;
    int tile, i;
    if (type >= NUMBER_OF_MON_TYPES)
        return NULL;
    if ((t = Cached(0x30000 | type)))
        return t;
    if (!sLoaded)
    {
        LZDecompressWram(gMoveTypes_Gfx, sTiles);
        LZDecompressWram(gMoveTypes_Pal, sPals);
        sLoaded = TRUE;
    }
    for (tile = 0; tile < 8; tile++)
        for (i = 0; i < 64; i++)
        {
            u8 b = sTiles[type * 0x100 + tile * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
            int x = (tile % 4) * 8 + i % 8, y = (tile / 4) * 8 + i / 8;
            px[y * 32 + x] = v ? Rgb555(sPals[16 * MP_TypeIconPal(type) + v]) : 0;
        }
    return Store(0x30000 | type, px, 32, 16);
}

// A type's name on its color
static void TypePill(int x, int y, u8 type)
{
    SDL_Texture *icon = TypeIconTex(type);
    if (icon)
    {
        Blit(icon, x, y, 16, 8, 255);
        return;
    }
    char name[12];
    if (type >= NUMBER_OF_MON_TYPES)
        return;
    Ascii(name, gTypeNames[type], sizeof(name));
    G_RoundFill(x, y + 1, 30, 9, G_Mix(TypeRgb(type), 0x000000, 0.45f), 255);
    G_RoundFill(x, y, 30, 9, TypeRgb(type), 255);
    G_FillA(x + 2, y, 26, 1, 0xffffff, 90);
    G_TextAlign(x + 15, y, TA_CENTER, G_WHITE, 0x303030, name, TF_NARROW);
}

// A label on a colored chip; returns its width
static int Chip(int x, int y, const char *text, unsigned color)
{
    int w = G_Measure(text, TF_NARROW) + 8;
    G_RoundFill(x, y, w, 10, G_Mix(color, 0x000000, 0.5f), 255);
    G_RoundFill(x, y, w, 9, color, 255);
    G_FillA(x + 2, y, w - 4, 1, 0xffffff, 80);
    G_TextAlign(x + w / 2, y, TA_CENTER, G_WHITE, 0x202020, text, TF_NARROW);
    return w;
}

// A part of a texture (source rectangle in its own pixels)
static void BlitPart(SDL_Texture *t, int sx, int sy, int sw, int sh, float x, float y, float w, float h, u8 alpha)
{
    SDL_Rect src = {sx, sy, sw, sh};
    SDL_FRect d = {x * 2, y * 2, w * 2, h * 2};
    if (!t)
        return;
    SDL_RenderSetScale(U.owner, 1, 1);
    SDL_SetTextureAlphaMod(t, alpha);
    SDL_RenderCopyF(U.owner, t, &src, &d);
    SDL_SetTextureAlphaMod(t, 255);
    SDL_RenderSetScale(U.owner, 2, 2);
}

// The field's item ball (its overworld sprite, 16x16), for empty places on a team and the road of trainers
extern const u32 gObjectEventPic_ItemBall[];
extern const u16 gObjectEventPal_Npc3[];
static SDL_Texture *ItemBallTex(void)
{
    static u32 px[16 * 16];
    const u8 *tiles = (const u8 *)gObjectEventPic_ItemBall;
    SDL_Texture *t;
    int tile, i;
    if ((t = Cached(0x50000)))
        return t;
    for (tile = 0; tile < 4; tile++)
        for (i = 0; i < 64; i++)
        {
            u8 b = tiles[tile * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
            int x = (tile % 2) * 8 + i % 8, y = (tile / 2) * 8 + i / 8;
            px[y * 16 + x] = v ? Rgb555(gObjectEventPal_Npc3[v]) : 0;
        }
    return Store(0x50000, px, 16, 16);
}

// The ball's own pixels within its 16x16 frame (it sits low in it, as on the field): drawn centered on (cx, cy)
static void ItemBallSized(float cx, float cy, float scale, u8 alpha)
{
    static int x0 = -1, y0, x1, y1;
    if (x0 < 0)
    {
        const u8 *tiles = (const u8 *)gObjectEventPic_ItemBall;
        int tile, i;
        x0 = 16, y0 = 16, x1 = 0, y1 = 0;
        for (tile = 0; tile < 4; tile++)
            for (i = 0; i < 64; i++)
            {
                u8 b = tiles[tile * 32 + i / 2], v = (i & 1) ? b >> 4 : b & 15;
                int x = (tile % 2) * 8 + i % 8, y = (tile / 2) * 8 + i / 8;
                if (v)
                {
                    if (x < x0) x0 = x;
                    if (y < y0) y0 = y;
                    if (x + 1 > x1) x1 = x + 1;
                    if (y + 1 > y1) y1 = y + 1;
                }
            }
        if (x1 <= x0)
            x0 = 0, y0 = 0, x1 = 16, y1 = 16;
    }
    BlitPart(ItemBallTex(), x0, y0, x1 - x0, y1 - y0, cx - (x1 - x0) * scale / 2, cy - (y1 - y0) * scale / 2,
             (x1 - x0) * scale, (y1 - y0) * scale, alpha);
}

static void ItemBall(int cx, int cy, u8 alpha)
{
    ItemBallSized(cx, cy, 1.0f, alpha);
}

// A small POKé BALL
static void Ball(int cx, int cy, int r, unsigned top)
{
    G_Circle(cx, cy + 1, r, 0x000000, 90);
    G_Circle(cx, cy, r, 0x202020, 255);
    G_Circle(cx, cy, r - 1, 0xf0f0f0, 255);
    G_FillA(cx - r + 1, cy - r + 1, 2 * r - 2, r - 1, top, 255);
    G_Circle(cx, cy - r / 2, r - 2, top, 255);
    G_Fill(cx - r + 1, cy - 1, 2 * r - 2, 1, 0x202020);
    G_Circle(cx, cy, r / 3 + 1, 0x202020, 255);
    G_Circle(cx, cy, r / 3, 0xf8f8f8, 255);
}

static bool8 Has(u8 cup, const struct BoxPokemon *b)
{
    const struct StadiumTeam *t = &StadiumCup_Save()->team[cup];
    u8 i;
    for (i = 0; i < t->count; i++)
        if (memcmp(&t->slot[i].mon, b, sizeof(*b)) == 0)
            return TRUE;
    return FALSE;
}

static void AddToList(u8 source, u16 rental, const struct BoxPokemon *b)
{
    if (U.listCount >= LIST_MAX || GetBoxMonData((struct BoxPokemon *)b, MON_DATA_SPECIES) == SPECIES_NONE
     || GetBoxMonData((struct BoxPokemon *)b, MON_DATA_IS_EGG))
        return;
    U.list[U.listCount].source = source;
    U.list[U.listCount].rental = rental;
    U.list[U.listCount].mon = *b;
    U.listCount++;
}

static void BuildList(void)
{
    u16 i, b;
    U.listCount = 0;
    switch (U.tab)
    {
    case SRC_TAB_PARTY:
        for (i = 0; i < PARTY_SIZE; i++)
            AddToList(STADIUM_SRC_PARTY, 0, &gPlayerParty[i].box);
        break;
    case SRC_TAB_BOXES:
    {
        struct PokemonStorage *st = MP_PersonalStorage();
        if (st)
            for (b = 0; b < TOTAL_BOXES_COUNT; b++)
                for (i = 0; i < IN_BOX_COUNT; i++)
                    AddToList(STADIUM_SRC_BOX, 0, &st->boxes[b][i]);
        break;
    }
    case SRC_TAB_GLOBAL:
    {
        struct PokemonStorage *st = MP_Region_GlobalPC();
        if (st)
            for (b = 0; b < TOTAL_BOXES_COUNT; b++)
                for (i = 0; i < IN_BOX_COUNT; i++)
                    AddToList(STADIUM_SRC_GLOBAL, 0, &st->boxes[b][i]);
        break;
    }
    case SRC_TAB_RENTALS:
    {
        u16 n = StadiumCup_RentalCount(U.round, U.cup);
        for (i = 0; i < n; i++)
        {
            struct Pokemon m;
            Stadium_BuildMon(&m, StadiumCup_Rental(U.round, U.cup, i));
            AddToList(STADIUM_SRC_RENTAL, i, &m.box);
        }
        break;
    }
    }
    if (U.listCursor >= U.listCount)
        U.listCursor = U.listCount ? U.listCount - 1 : 0;
}

static const char *Refusal(const struct BoxPokemon *b, u8 source)
{
    u16 moves[MAX_MON_MOVES];
    u8 i;
    struct BoxPokemon *bb = (struct BoxPokemon *)b;
    if (source == STADIUM_SRC_RENTAL)
        return NULL;   // (STADIUM's own, made for the cup)
    if (U.opt & STADIUM_OPT_RENTALS)
        return "RENTALS ONLY";
    for (i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = GetBoxMonData(bb, MON_DATA_MOVE1 + i);
    return StadiumCup_Refuse(U.cup, U.tier, U.opt, GetBoxMonData(bb, MON_DATA_SPECIES),
                             GetLevelFromBoxMonExp(bb), moves, GetBoxMonData(bb, MON_DATA_HELD_ITEM));
}

static void ToggleEntry(void)
{
    struct StadiumSave *s = StadiumCup_Save();
    struct StadiumTeam *t = &s->team[U.cup];
    const char *why;
    u8 i;
    if (U.listCursor >= U.listCount)
        return;
    for (i = 0; i < t->count; i++)
        if (memcmp(&t->slot[i].mon, &U.list[U.listCursor].mon, sizeof(struct BoxPokemon)) == 0)
        {
            for (; i + 1 < t->count; i++)
                t->slot[i] = t->slot[i + 1];
            t->count--;
            PlaySE(SE_SELECT);
            snprintf(U.note, sizeof(U.note), "Taken off the team.");
            return;
        }
    if (t->count >= STADIUM_TEAM_SIZE)
    {
        PlaySE(SE_FAILURE);
        snprintf(U.note, sizeof(U.note), "The team has six already.");
        return;
    }
    if ((why = Refusal(&U.list[U.listCursor].mon, U.list[U.listCursor].source)))
    {
        PlaySE(SE_FAILURE);
        snprintf(U.note, sizeof(U.note), "%s.", why);
        return;
    }
    {
        u16 sp = GetBoxMonData(&U.list[U.listCursor].mon, MON_DATA_SPECIES);
        for (i = 0; i < t->count; i++)
            if (GetBoxMonData(&t->slot[i].mon, MON_DATA_SPECIES) == sp)
            {
                PlaySE(SE_FAILURE);
                snprintf(U.note, sizeof(U.note), "SPECIES CLAUSE: one of each POKéMON.");
                return;
            }
    }
    t->slot[t->count].source = U.list[U.listCursor].source;
    t->slot[t->count].rental = U.list[U.listCursor].rental;
    t->slot[t->count].mon = U.list[U.listCursor].mon;
    t->count++;
    PlaySE(SE_SELECT);
    snprintf(U.note, sizeof(U.note), "Registered! (%d of 6)", t->count);
}

// Every registered POKéMON still fits the cup with these switches (a team made under other ones may not)
static const char *TeamProblem(void)
{
    const struct StadiumTeam *t = &StadiumCup_Save()->team[U.cup];
    u8 i;
    if (U.cup == STADIUM_CUP_CHALLENGE)
        return NULL;
    if (t->count < STADIUM_TEAM_SIZE)
        return "Register six POKéMON first.";
    for (i = 0; i < t->count; i++)
    {
        const char *why = Refusal(&t->slot[i].mon, t->slot[i].source);
        if (why)
        {
            static char line[96];
            char name[16];
            Ascii(name, (const u8 *)"", 1);
            {
                u8 nick[POKEMON_NAME_LENGTH + 1];
                GetBoxMonData((struct BoxPokemon *)&t->slot[i].mon, MON_DATA_NICKNAME, nick);
                nick[POKEMON_NAME_LENGTH] = EOS;
                Ascii(name, nick, sizeof(name));
            }
            snprintf(line, sizeof(line), "%s: %s.", name, why);
            return line;
        }
    }
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------
// Drawing

static void Background(void)
{
    int y, i;
    int t = U.age;
    for (y = 0; y < 160; y += 2)
        G_Fill(0, y, 240, 2, G_Mix(0x20345e, 0x0a1226, y / 160.0f));
    // light bands drifting across, like STADIUM's menus
    for (i = -3; i < 7; i++)
    {
        int x0 = i * 56 + (t / 2) % 56;
        for (y = 16; y < 146; y += 2)
            G_FillA(x0 + (146 - y) / 2, y, 20, 2, 0x6080c0, 18);
    }
    // a great faint POKé BALL behind everything
    G_Circle(196, 96, 74, 0x3a5a9a, 22);
    G_Circle(196, 96, 66, 0x0a1226, 30);
    G_FillA(122, 94, 148, 5, 0x3a5a9a, 26);
    G_Circle(196, 96, 18, 0x3a5a9a, 28);
    G_Circle(196, 96, 12, 0x0a1226, 40);
}

static void Header(const char *title, const char *right)
{
    G_Fill(0, 0, 240, 16, 0x0c1424);
    G_FillA(0, 0, 240, 7, 0xffffff, 16);
    G_Fill(0, 15, 240, 1, C_GOLD);
    G_FillA(0, 16, 240, 2, 0x000000, 90);
    ItemBall(9, 8, 255);
    G_Text(18, 1, C_GOLD, 0x000000, title, 0, TF_NORMAL);
    if (right)
        G_TextAlign(234, 3, TA_RIGHT, G_WHITE, 0x000000, right, TF_SMALL);
}

static void Footer(const char *hint)
{
    G_Fill(0, 146, 240, 14, 0x0c1424);
    G_Fill(0, 146, 240, 1, C_GOLD);
    G_FillA(0, 147, 240, 5, 0xffffff, 12);
    if (U.note[0])
        G_Text(4, 147, 0xf8e8a0, 0x000000, U.note, 232, TF_SMALL);
    else
        G_Hint(4, 147, G_WHITE, 0x000000, hint, TF_SMALL, 1);
}

static const char *const sClassNames[4] = {"POKé BALL", "GREAT BALL", "ULTRA BALL", "MASTER BALL"};

// Stars for the classes won: gold with every restriction in place
static void ClearMarks(int x, int y, u8 cup)
{
    u8 t;
    for (t = 0; t < StadiumCup_Info(cup)->tiers; t++)
    {
        u16 c = StadiumCup_Save()->cleared[U.round][cup][t];
        unsigned col = (c & (1 << STADIUM_HARD_ALL)) ? C_GOLD : c ? G_WHITE : C_LOCK;
        G_Circle(x + t * 6, y, 2.5f, col, 255);
    }
}

static const char *CupLabel(u8 i)
{
    return i == STADIUM_CUP_CASTLE ? "GYM CASTLE" : i == STADIUM_CUP_CHALLENGE ? "CHALLENGE" : StadiumCup_Info(i)->name;
}

// The picture for the cups STADIUM has none of: the castle's gym leaders, MEWTWO, the CHALLENGE CUP's random six
static void CupScene(u8 cup, int x, int y, int w, int h, bool8 open)
{
    u8 a = open ? 255 : 90, i;
    for (i = 0; i < h; i += 2)
        G_FillA(x, y + i, w, 2, G_Mix(cup == STADIUM_CUP_MEWTWO ? 0x502878 : cup == STADIUM_CUP_CASTLE ? 0x784020 : 0x205870,
                                      0x0a0a18, (float)i / h), 255);
    if (cup == STADIUM_CUP_CASTLE)
    {
        static const u8 sLeaders[8] = {128, 129, 130, 131, 132, 133, 134, 135};
        for (i = 0; i < 8; i++)
        {
            float bob = (float)((U.age / 4 + i * 5) % 20 < 10 ? 0 : 1);
            Blit(TrainerTex(sLeaders[i]), x + 3 + (i % 4) * (w - 6) / 4.0f, y + 2 + (i / 4) * 34 + bob, 34, 34, a);
        }
        Blit(U.pics[PIC_AREA(AREA_CASTLE)], x + w / 2 - 49, y + h - 34, 98, 32, a);
    }
    else if (cup == STADIUM_CUP_MEWTWO)
    {
        G_Circle(x + w / 2, y + h / 2 - 8, 34 + (U.age / 3) % 6, 0xc070ff, 40);
        G_Circle(x + w / 2, y + h / 2 - 8, 26, 0xe0a0ff, 40);
        Blit(FrontTex(SPECIES_MEWTWO, 0, 0), x + w / 2 - 32, y + 4, 64, 64, a);
        Blit(U.pics[PIC_AREA(AREA_MEWTWO)], x + w / 2 - 49, y + h - 34, 98, 32, a);
    }
    else
    {
        static const u16 sSix[6] = {SPECIES_PORYGON, SPECIES_HITMONLEE, SPECIES_GYARADOS, SPECIES_SEAKING,
                                    SPECIES_HOOTHOOT, SPECIES_SNUBBULL};
        for (i = 0; i < 6; i++)
        {
            int k = (U.age / 2 + i * 60) % 360;
            float cx = x + w / 2 + 44 * SDL_cosf(k * 3.14159f / 180), cy = y + h / 2 - 6 + 26 * SDL_sinf(k * 3.14159f / 180);
            Blit(IconTex(sSix[i]), cx - 16, cy - 16, 32, 32, a);
        }
        G_Banner(x + w / 2, y + h / 2 - 6, "?", C_GOLD, 2.0f, a);
    }
}

// The cups listed on the left (STADIUM's sign above them), the one under the cursor large on the right with its rules
static void DrawSelect(void)
{
    u8 i;
    char line[96];
    snprintf(line, sizeof(line), "ROUND %d", U.round + 1);
    Header("POKéMON STADIUM", line);
    Blit(U.pics[PIC_AREA(AREA_STADIUM)], 4, 17, 98, 32, 255);
    for (i = 0; i < STADIUM_CUP_COUNT; i++)
    {
        int y = 50 + i * 13;
        bool8 open = StadiumCup_Open(U.round, i, 0);
        bool8 focus = U.cup == i;
        if (focus)
        {
            Panel(3, y, 100, 12, C_GOLD, 0xf8e080);
            G_FillAdd(4 + (U.age * 2) % 96, y + 1, 6, 10, 0xffffff, 60);
        }
        else
            Panel(3, y, 100, 12, open ? 0x22345c : 0x1a2034, open ? 0x5a70a8 : 0x3a4058);
        G_Text(7, y + 1, focus ? C_INK : open ? G_WHITE : C_LOCK, focus ? G_NO_SHADOW : 0x000000, CupLabel(i),
               70, TF_NARROW);
        ClearMarks(100 - StadiumCup_Info(i)->tiers * 6, y + 6, i);
    }
    // the picture, in a gold frame, a light passing over it
    {
        SDL_Texture *pic = CupPic(U.cup);
        bool8 open = StadiumCup_Open(U.round, U.cup, 0);
        int sweep = (U.age * 3) % 260 - 60;
        const int px = 108, py = 20, pw = 128, ph = 96;
        Panel(px - 2, py - 2, pw + 4, ph + 4, 0x101420, C_GOLD);
        G_Frame(px - 1, py - 1, pw + 2, ph + 2, 0xf8e8a0, 120);
        if (pic)
            Blit(pic, px, py, pw, ph, open ? 255 : 90);
        else
            CupScene(U.cup, px, py, pw, ph, open);
        if (open && sweep > -20 && sweep < 135)
        {
            int k;
            for (k = 0; k < ph; k += 2)
            {
                int sx = px + sweep + (ph - k) / 3;
                if (sx >= px && sx < px + pw - 6)
                    G_FillAdd(sx, py + k, 6, 2, 0xffffff, 26);
            }
        }
        if (!open)
        {
            G_FillA(px, py + 40, pw, 18, 0x000000, 160);
            G_TextAlign(px + pw / 2, py + 43, TA_CENTER, G_WHITE, 0x000000, "LOCKED", TF_NORMAL);
        }
    }
    // its rules as chips, and a line under them
    {
        const struct StadiumCupInfo *c = StadiumCup_Info(U.cup);
        int x = 109;
        Panel(106, 119, 132, 26, C_PANEL, 0xb0a888);
        if (!StadiumCup_Open(U.round, U.cup, 0))
            G_Text(110, 126, C_RED, G_NO_SHADOW, U.round ? "Beat MEWTWO in Round 1." : "Win every cup first.", 126,
                   TF_NARROW);
        else
        {
            if (U.cup == STADIUM_CUP_CHALLENGE)
            {
                x += Chip(x, 121, "6 LENT", 0x4878c0) + 2;
                x += Chip(x, 121, "Lv 30-75", 0x40a050) + 2;
                Chip(x, 121, "PICK 3", 0xc06030);
                snprintf(line, sizeof(line), "Random teams on both sides.");
            }
            else
            {
                if (c->levelSum)
                    snprintf(line, sizeof(line), "Lv %d-%d", c->levelMin, c->levelMax);
                else
                    snprintf(line, sizeof(line), "ANY LEVEL");
                x += Chip(x, 121, line, 0x40a050) + 2;
                if (c->levelSum)
                {
                    snprintf(line, sizeof(line), "SUM %d", c->levelSum);
                    x += Chip(x, 121, line, 0x4878c0) + 2;
                }
                if (U.cup != STADIUM_CUP_MEWTWO)
                    Chip(x, 121, U.cup == STADIUM_CUP_CASTLE ? "8 GYMS" : "8 FOES", 0xc06030);
                snprintf(line, sizeof(line), "%s", c->petit ? "2.0 m, 20 kg or less, lowest stage."
                         : c->allowsMewtwo ? "MEWTWO and MEW allowed." : "No MEWTWO or MEW.");
            }
            G_Text(110, 132, C_INK, G_NO_SHADOW, line, 126, TF_NARROW);
        }
    }
    Footer("{A} ENTER  {L}{R} ROUND  {B} EXIT");
}

// The classes as STADIUM's banners (the one under the cursor lit and pushed out), and on the right the class's eight
// opponents and the best won there (how many restrictions were in place)
static void DrawClass(void)
{
    u8 t, i;
    char line[64];
    Header(StadiumCup_Info(U.cup)->name, U.round ? "ROUND 2" : "ROUND 1");
    for (t = 0; t < 4; t++)
    {
        bool8 open = StadiumCup_Open(U.round, U.cup, t), focus = U.tier == t;
        int y = 19 + t * 31, push = focus ? 4 : 0;
        if (focus)
        {
            G_RoundFill(2, y - 1, 148, 31, C_GOLD, 120 + (U.age * 6) % 100);
            G_RoundFill(3, y, 146, 29, 0x101828, 200);
        }
        Blit(open ? U.pics[PIC_BALL(t)] : U.greyBalls[t], 10 + push, y, 110, 31, open ? 255 : 150);
        if (open && StadiumCup_Cleared(U.round, U.cup, t))
            Chip(118, y + 11, "WON", 0xc09020);
    }
    // the class under the cursor
    Panel(153, 18, 84, 126, C_PANEL, 0xb0a888);
    G_Text(157, 20, C_INK, G_NO_SHADOW, sClassNames[U.tier], 0, TF_SMALL);
    if (U.cup == STADIUM_CUP_CHALLENGE)
    {
        static const u8 sLevels[4] = {30, 45, 60, 75};
        snprintf(line, sizeof(line), "Every POKéMON Lv %d", sLevels[U.tier]);
        G_Text(157, 32, C_SOFT, G_NO_SHADOW, line, 78, TF_NARROW);
    }
    else
    {
        snprintf(line, sizeof(line), "%d TRAINERS", StadiumCup_TrainerCount(U.round, U.cup, U.tier));
        G_Text(157, 32, C_SOFT, G_NO_SHADOW, line, 78, TF_NARROW);
        for (i = 0; i < 8 && i < StadiumCup_TrainerCount(U.round, U.cup, U.tier); i++)
        {
            const struct StadiumTrainer *tr = Stadium_Trainer(U.round, StadiumCup_Info(U.cup)->division[U.tier], i);
            int x = 156 + (i % 4) * 20, y = 43 + (i / 4) * 24;
            G_RoundFill(x, y, 19, 22, i == 7 ? 0xe8c070 : 0xe0d8c0, 255);
            if (tr && tr->pic)
                BlitPart(TrainerTex(tr->pic), 8, 0, 48, 56, x, y + 1, 19, 21, StadiumCup_Open(U.round, U.cup, U.tier) ? 255 : 80);
        }
    }
    // the best won here
    {
        u16 c = StadiumCup_Save()->cleared[U.round][U.cup][U.tier];
        u8 best = 0, k;
        for (k = 0; k < 16; k++)   // (bit h: won with the hard sides h; the most of them)
            if ((c & (1 << k)) && __builtin_popcount(k) > best)
                best = __builtin_popcount(k);
        G_FillA(156, 95, 78, 1, 0xb0a888, 255);
        G_Text(157, 97, C_INK, G_NO_SHADOW, "BEST", 0, TF_NARROW);
        if (!c)
            G_Text(157, 108, C_SOFT, G_NO_SHADOW, "Not won yet.", 78, TF_NARROW);
        else
        {
            for (k = 0; k < 4; k++)
            {
                G_Circle(162 + k * 10, 113, 4, k < best ? (best == 4 ? C_GOLD : 0xe0a040) : 0xc8c0a8, 255);
                if (k < best)
                    G_Circle(161 + k * 10, 112, 1.3f, 0xffffff, 200);
            }
            snprintf(line, sizeof(line), "%d of 4 restrictions", best);
            G_Text(157, 121, C_SOFT, G_NO_SHADOW, line, 78, TF_NARROW);
        }
    }
    Footer("{A} CHOOSE  {B} BACK");
}

static const char *const sOptNames[3] = {"LEVEL CAP", "SPECIES", "HELD ITEMS"};

static u8 HardCount(void)
{
    u8 n = 0;
    if (!(U.opt & STADIUM_OPT_LEVEL_CAP)) n++;
    if (U.opt & STADIUM_OPT_SPECIES) n++;
    if (!(U.opt & STADIUM_OPT_HELD_ITEMS)) n++;
    if (U.opt & STADIUM_OPT_RENTALS) n++;
    return n;
}

static const char *const sOptHelp[4][2] = {
    {"Entries are set to the cup's level.", "Your own levels. Bigger rewards!"},
    {"First 151 only. Bigger rewards!", "Any POKéMON but the great legends."},
    {"Your POKéMON may hold items.", "No held items. Bigger rewards!"},
    {"Rentals only, LEVEL CAP off. Bigger rewards!", "Your own POKéMON may enter."},
};

// A switch drawn as a lit slider
static void Toggle(int x, int y, bool8 on)
{
    G_RoundFill(x - 4, y + 1, 34, 10, 0x000000, 80);
    G_RoundFill(x - 4, y, 34, 10, on ? 0x40a050 : 0x904040, 255);
    G_FillA(x - 2, y, 30, 1, 0xffffff, 80);
    G_Circle(on ? x + 25 : x + 1, y + 5, 4, 0xf8f8f8, 255);
    G_TextAlign(on ? x + 9 : x + 16, y, TA_CENTER, G_WHITE, 0x202020, on ? "ON" : "OFF", TF_NARROW);
}

static void DrawOptions(void)
{
    char line[96];
    u8 i, rows = U.cup == STADIUM_CUP_CHALLENGE ? 0 : 4, hard = HardCount();
    const struct StadiumTeam *t = &StadiumCup_Save()->team[U.cup];
    snprintf(line, sizeof(line), "%s%s%s", StadiumCup_Info(U.cup)->name, StadiumCup_Info(U.cup)->tiers > 1 ? "  " : "",
             StadiumCup_Info(U.cup)->tiers > 1 ? sClassNames[U.tier] : "");
    Header(line, U.round ? "ROUND 2" : "ROUND 1");
    // the cup's picture and its class banner
    {
        SDL_Texture *pic = CupPic(U.cup);
        Panel(3, 18, 116, 88, 0x101420, C_GOLD);
        if (pic)
            Blit(pic, 5, 20, 112, 84, 255);
        else
            CupScene(U.cup, 5, 20, 112, 84, TRUE);
        if (StadiumCup_Info(U.cup)->tiers > 1)
            Blit(U.pics[PIC_BALL(U.tier)], 13, 78, 96, 27, 255);
    }
    // the switches
    Panel(122, 18, 115, 88, C_PANEL, 0xb0a888);
    G_Text(126, 19, C_INK, G_NO_SHADOW, "RESTRICTIONS", 0, TF_SMALL);
    for (i = 0; i < 4; i++)   // stars: one for each in place
    {
        int cx = 202 + i * 9;
        G_Circle(cx, 25, 3.5f, i < hard ? (hard == 4 ? C_GOLD : 0xe0a040) : 0xc8c0a8, 255);
        if (i < hard)
            G_Circle(cx - 1, 24, 1.2f, 0xffffff, 200);
    }
    if (rows)
    {
        for (i = 0; i < 4; i++)
        {
            int y = 30 + i * 12;
            bool8 on = (U.opt & (1 << i)) != 0;
            bool8 isHard = i == 1 ? on : i == 3 ? on : !on;
            static const char *const sNames[4] = {"LEVEL CAP", "SPECIES", "HELD ITEMS", "RENTALS ONLY"};
            if (U.optCursor == i)
                G_RoundFill(124, y - 1, 111, 12, C_GOLD, 200);
            {
                bool8 held = i == 0 && (U.opt & STADIUM_OPT_RENTALS);   // (RENTALS ONLY keeps it off)
                G_Text(127, y, held ? C_LOCK : C_INK, G_NO_SHADOW, sNames[i], 0, TF_NARROW);
                Toggle(196, y, on);
                if (held)
                    G_RoundFill(192, y, 34, 10, 0xf8f0d8, 140);
            }
            if (isHard)
                G_Circle(231, y + 5, 2.5f, 0xe0a040, 255);
        }
        {
            u8 k = U.optCursor < 4 ? U.optCursor : 3;
            bool8 on = (U.opt & (1 << k)) != 0;
            G_FillA(125, 79, 109, 1, 0xb0a888, 255);
            G_Text(127, 81, hard == 4 ? 0x907020 : C_SOFT, G_NO_SHADOW,
                   hard == 4 ? "Every restriction in place: GREAT REWARDS!"
                   : k == 0 && (U.opt & STADIUM_OPT_RENTALS) ? "Off: rentals are already the cup's level." : sOptHelp[k][on ? 0 : 1],
                   106, TF_NARROW);
        }
    }
    else
        G_Text(127, 34, C_INK, G_NO_SHADOW, "The CHALLENGE CUP lends you six random POKéMON for every challenge. "
                                            "Pick three before each battle.", 106, TF_NARROW);
    // the team
    Panel(3, 109, 234, 18, 0x0c1424, 0x4a5c88);
    G_Text(7, 113, C_GOLD, 0x000000, rows ? "TEAM" : "LENT", 0, TF_NARROW);
    for (i = 0; i < STADIUM_TEAM_SIZE; i++)
    {
        int x = 32 + i * 30;
        G_RoundFill(x, 111, 27, 14, i < t->count && rows ? 0x3a5088 : 0x1a2034, 255);
        if (i < t->count && rows)
            Blit(IconTex(GetBoxMonData((struct BoxPokemon *)&t->slot[i].mon, MON_DATA_SPECIES)), x - 3, 98, 32, 32, 255);
        else
            ItemBall(x + 13, 118, rows ? 110 : 255);
    }
    if (rows)
    {
        snprintf(line, sizeof(line), "%d / 6", t->count);
        G_TextAlign(232, 113, TA_RIGHT, t->count == 6 ? C_GOLD : G_WHITE, 0x000000, line, TF_SMALL);
    }
    // the buttons
    {
        static const char *const sButtons[3] = {"REGISTER", "SAVED TEAMS", "START!"};
        for (i = 0; i < 3; i++)
        {
            bool8 focus = U.optCursor == rows + i, dim = i < 2 && !rows;
            int x = 3 + i * 79;
            if (focus)
            {
                Panel(x, 129, 76, 16, C_GOLD, 0xf8e080);
                G_FillAdd(x + 2 + (U.age * 2) % 68, 130, 6, 14, 0xffffff, 60);
            }
            else
                Panel(x, 129, 76, 16, dim ? 0x1a2034 : 0x22345c, dim ? 0x3a4058 : 0x5a70a8);
            G_TextAlign(x + 38, 131, TA_CENTER, focus ? C_INK : dim ? C_LOCK : G_WHITE, focus ? G_NO_SHADOW : 0x000000,
                        sButtons[i], TF_SMALL);
        }
    }
    Footer("{A} CHANGE / CHOOSE  {B} BACK");
}

#define REG_COLS 4
#define REG_ROWS 5
#define REG_VIEW (REG_COLS * REG_ROWS)

static const char *const sMenuRows[3] = {"REGISTER", "SUMMARY", "CANCEL"};

// The POKéMON under the cursor: picture, name, level, types, item, stats and moves
static void DetailCard(const struct BoxPokemon *b, u8 source)
{
    struct Pokemon m;
    u8 nick[POKEMON_NAME_LENGTH + 1];
    char line[48];
    u16 sp = GetBoxMonData((struct BoxPokemon *)b, MON_DATA_SPECIES);
    u16 item = GetBoxMonData((struct BoxPokemon *)b, MON_DATA_HELD_ITEM);
    static const char *const sStat[6] = {"HP", "ATK", "DEF", "SPA", "SPD", "SPE"};
    static const u8 sStatId[6] = {MON_DATA_MAX_HP, MON_DATA_ATK, MON_DATA_DEF, MON_DATA_SPATK, MON_DATA_SPDEF, MON_DATA_SPEED};
    u16 stats[6], top = 1;
    u8 i;
    const char *why = Refusal(b, source);
    BoxMonToMon(b, &m);
    Panel(124, 31, 113, 92, 0xf4ecd8, 0xb0a888);
    // the picture on a lit spot
    G_Circle(146, 52, 18, 0xffffff, 130);
    G_Circle(146, 55, 14, G_Mix(TypeRgb(gSpeciesInfo[sp].types[0]), 0xffffff, 0.55f), 150);
    Blit(FrontTex(sp, GetBoxMonData((struct BoxPokemon *)b, MON_DATA_PERSONALITY),
                  GetBoxMonData((struct BoxPokemon *)b, MON_DATA_OT_ID)), 126, 32, 40, 40, why ? 140 : 255);
    GetBoxMonData((struct BoxPokemon *)b, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    Ascii(line, nick, sizeof(line));
    G_Text(168, 32, C_INK, G_NO_SHADOW, line, 0, TF_SMALL);
    snprintf(line, sizeof(line), "Lv%d", GetMonData(&m, MON_DATA_LEVEL));
    G_Text(168, 43, C_SOFT, G_NO_SHADOW, line, 0, TF_NARROW);
    if (source == STADIUM_SRC_RENTAL)
        Chip(194, 43, "RENTAL", 0x4878c0);
    TypePill(168, 55, gSpeciesInfo[sp].types[0]);
    if (gSpeciesInfo[sp].types[1] != gSpeciesInfo[sp].types[0])
        TypePill(186, 55, gSpeciesInfo[sp].types[1]);
    if (item)
    {
        u8 name[ITEM_NAME_LENGTH + 4];
        CopyItemName(item, name);
        Ascii(line, name, sizeof(line));
        G_Text(168, 62, 0x806030, G_NO_SHADOW, line, 0, TF_NARROW);
    }
    // stats, two columns, as bars against the best of the six
    for (i = 0; i < 6; i++)
        if ((stats[i] = GetMonData(&m, sStatId[i])) > top)
            top = stats[i];
    for (i = 0; i < 6; i++)
    {
        int x = 127 + (i % 3) * 36, y = 73 + (i / 3) * 11;
        unsigned c = i == 0 ? 0x58b848 : i < 3 ? 0xe07838 : i < 5 ? 0x5888e0 : 0xe0c038;
        G_Text(x, y, C_SOFT, G_NO_SHADOW, sStat[i], 0, TF_NARROW);
        snprintf(line, sizeof(line), "%d", stats[i]);
        G_TextAlign(x + 33, y, TA_RIGHT, C_INK, G_NO_SHADOW, line, TF_NARROW);
        G_FillA(x, y + 10, 33, 1, 0xd8d0b8, 255);
        G_FillA(x, y + 10, 1 + stats[i] * 32 / top, 1, c, 255);
    }
    G_FillA(127, 97, 107, 1, 0xb0a888, 255);
    // the moves, a dot of their type each
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 mv = GetMonData(&m, MON_DATA_MOVE1 + i);
        int x = 126 + (i % 2) * 55, y = 99 + (i / 2) * 11;
        if (mv == MOVE_NONE || mv >= MOVES_COUNT)
            continue;
        G_Circle(x + 3, y + 4, 2.5f, TypeRgb(gBattleMoves[mv].type), 255);
        Ascii(line, gMoveNames[mv], sizeof(line));
        {
            SDL_Rect clip = {x + 7, y, 47, 11};
            SDL_RenderSetClipRect(U.owner, &clip);
            G_Text(x + 7, y, C_INK, G_NO_SHADOW, line, 0, TF_NARROW);
            SDL_RenderSetClipRect(U.owner, NULL);
        }
    }
    if (why)
    {
        G_FillA(125, 63, 111, 9, 0xc03028, 220);
        G_TextAlign(180, 63, TA_CENTER, G_WHITE, 0x000000, why, TF_NARROW);
    }
}

static void DrawRegister(void)
{
    static const char *const sTabs[SRC_TAB_COUNT] = {"PARTY", "BOXES", "GLOBAL PC", "RENTALS"};
    const struct StadiumTeam *t = &StadiumCup_Save()->team[U.cup];
    char line[96];
    u8 i;
    Header("REGISTER A TEAM", StadiumCup_Info(U.cup)->name);
    // where from: tabs
    {
        int x = 4;
        G_Arrow(x, 21, 2, G_WHITE);
        x += 6;
        for (i = 0; i < SRC_TAB_COUNT; i++)
        {
            int w = G_Measure(sTabs[i], TF_NARROW) + 8;
            if (i == U.tab)
                Panel(x, 18, w, 11, C_GOLD, 0xf8e080);
            else
                G_RoundFill(x, 19, w, 10, 0x22345c, 255);
            G_TextAlign(x + w / 2, 19, TA_CENTER, i == U.tab ? C_INK : 0xa8b8d8, i == U.tab ? G_NO_SHADOW : 0x000000,
                        sTabs[i], TF_NARROW);
            x += w + 1;
        }
        G_Arrow(x + 1, 21, 3, G_WHITE);
    }
    // the list: four across, five down
    Panel(3, 31, 119, 92, 0x18243e, 0x4a5c88);
    if (U.listCount == 0)
        G_TextAlign(62, 70, TA_CENTER, 0xa8b8d8, 0x000000, "Nobody here.", TF_SMALL);
    for (i = 0; i < REG_VIEW && U.listTop + i < U.listCount; i++)
    {
        u16 n = U.listTop + i;
        int x = 6 + (i % REG_COLS) * 28, y = 34 + (i / REG_COLS) * 17;
        const struct BoxPokemon *b = &U.list[n].mon;
        bool8 ok = Refusal(b, U.list[n].source) == NULL, has = Has(U.cup, b);
        G_RoundFill(x, y, 26, 16, has ? C_GOLD : ok ? 0x3a5088 : 0x2a2a38, 255);
        G_FillA(x + 2, y, 22, 1, 0xffffff, 50);
        Blit(IconTex(GetBoxMonData((struct BoxPokemon *)b, MON_DATA_SPECIES)), x - 3, y - 12 + ((n == U.listCursor && (U.age / 8) % 2) ? -1 : 0),
             32, 32, ok ? 255 : 100);
        if (has)
            ItemBallSized(x + 22, y + 12, 0.75f, 255);
        if (n == U.listCursor)
            G_Focus(x, y, 26, 16, G_WHITE, G_WHITE);
    }
    // where in the list
    if (U.listCount > REG_VIEW)
    {
        int rows = (U.listCount + REG_COLS - 1) / REG_COLS, h = 86 * REG_ROWS / rows;
        G_RoundFill(117, 34, 3, 86, 0x0a1020, 255);
        G_RoundFill(117, 34 + (86 - h) * (U.listTop / REG_COLS) / (rows - REG_ROWS > 0 ? rows - REG_ROWS : 1), 3,
                    h < 6 ? 6 : h, C_GOLD, 255);
    }
    if (U.listCursor < U.listCount)
        DetailCard(&U.list[U.listCursor].mon, U.list[U.listCursor].source);
    // the team so far
    Panel(3, 125, 234, 20, 0x0c1424, 0x4a5c88);
    G_Text(6, 130, C_GOLD, 0x000000, "TEAM", 0, TF_NARROW);
    for (i = 0; i < STADIUM_TEAM_SIZE; i++)
    {
        int x = 28 + i * 28;
        G_RoundFill(x, 127, 26, 16, i < t->count ? 0x3a5088 : 0x1a2034, 255);
        if (i < t->count)
            Blit(IconTex(GetBoxMonData((struct BoxPokemon *)&t->slot[i].mon, MON_DATA_SPECIES)), x - 3, 115, 32, 32, 255);
        else
            ItemBall(x + 13, 135, 110);
    }
    snprintf(line, sizeof(line), "%d / 6", t->count);
    G_TextAlign(232, 130, TA_RIGHT, t->count == 6 ? C_GOLD : G_WHITE, 0x000000, line, TF_SMALL);
    // A's pop-up
    if (U.menu)
    {
        int mx = 70, my = 60;
        Panel(mx, my, 56, 40, C_PANEL, 0xb0a888);
        for (i = 0; i < 3; i++)
        {
            const char *label = sMenuRows[i];
            if (i == 0 && U.listCursor < U.listCount && Has(U.cup, &U.list[U.listCursor].mon))
                label = "REMOVE";
            if (U.menuCursor == i)
                G_RoundFill(mx + 3, my + 3 + i * 12, 50, 11, C_GOLD, 255);
            G_Text(mx + 8, my + 4 + i * 12, C_INK, G_NO_SHADOW, label, 0, TF_SMALL);
        }
    }
    Footer(U.menu ? "{A} CHOOSE  {B} BACK" : "{A} MENU  {SELECT} SUMMARY  {L}{R} WHERE FROM  {B} DONE");
}

static const char *const sClauses[][2] = {
    {"SLEEP CLAUSE", "Your moves can put only one of their POKéMON to sleep."},
    {"FREEZE CLAUSE", "Only one of their POKéMON can be frozen at a time."},
    {"SELF-KO CLAUSE", "SELFDESTRUCT or EXPLOSION on your last POKéMON loses."},
    {"SPECIES CLAUSE", "One of each POKéMON."},
    {"NO ITEMS", "No BAG items in battle."},
};

// The opponent above (STADIUM's dark plate, their picture and six), VS, the player's six below: pick three in order
static void DrawVs(void)
{
    struct StadiumRun *r = StadiumCup_Run();
    const struct StadiumTrainer *foe = StadiumCup_Foe();
    struct Pokemon *foeTeam;
    u8 foeCount, i, n = StadiumCup_EntryCount();
    char line[96], name[16] = "";
    extern u16 MP_StadiumBattle_TrainerPic(void);
    extern const u8 *MP_StadiumBattle_ClassName(void);
    foeTeam = StadiumCup_FoeTeam(&foeCount);
    if (foe)
        Ascii(name, foe->name, sizeof(name));
    else if (r->cup == STADIUM_CUP_CHALLENGE)
    {
        extern const struct StadiumCcTrainer *StadiumCc_Trainer(u8 cls, u8 i);
        Ascii(name, StadiumCc_Trainer(r->tier, r->trainer)->name, sizeof(name));
    }
    if (r->cup == STADIUM_CUP_CASTLE)
        snprintf(line, sizeof(line), "BATTLE %d OF %d", r->trainer + 1, StadiumCup_TrainerCount(r->round, r->cup, r->tier));
    else
        snprintf(line, sizeof(line), r->trainer == 6 ? "SEMI-FINAL" : r->trainer == 7 ? "FINAL BATTLE" : "BATTLE %d",
                 r->trainer + 1);
    Header(line, NULL);
    if (r->continues)
    {
        snprintf(line, sizeof(line), "CONTINUES %d", r->continues);
        G_TextAlign(234, 3, TA_RIGHT, G_WHITE, 0x000000, line, TF_SMALL);
    }
    else
        G_TextAlign(234, 3, TA_RIGHT, G_WHITE, 0x000000, StadiumCup_Info(r->cup)->name, TF_SMALL);
    // the opponent
    {
        int slide = U.age < 16 ? (16 - U.age) * 8 : 0;
        Panel(4 + slide, 18, 232, 58, 0x281818, 0x806040);
        BlitPart(U.pics[PIC_PLATE(3)], 8, 8, 240, 80, 5 + slide, 19, 230, 56, 150);
        G_FillA(5 + slide, 19, 230, 56, 0x000000, 70);
        Blit(TrainerTex(MP_StadiumBattle_TrainerPic()), 172 + slide, 14, 62, 62, 255);
        {
            char cls[24];
            Ascii(cls, MP_StadiumBattle_ClassName(), sizeof(cls));
            G_Text(10 + slide, 21, 0xf0c080, 0x000000, cls, 0, TF_NARROW);
        }
        G_Text(10 + slide, 29, G_WHITE, 0x000000, name, 0, TF_NORMAL);
        for (i = 0; i < foeCount && i < PARTY_SIZE; i++)
        {
            int x = 10 + slide + i * 26;
            G_RoundFill(x, 52, 24, 20, 0x000000, 110);
            Blit(IconTex(GetMonData(&foeTeam[i], MON_DATA_SPECIES)), x - 4, 44, 32, 32, 255);
        }
    }
    // VS
    G_Banner(120, 82, "VS", C_GOLD, 1.6f, 255);
    // the player's six
    {
        int slide = U.age < 16 ? (16 - U.age) * 8 : 0;
        Panel(4 - slide, 88, 232, 44, 0x182038, 0x5a70a8);
        BlitPart(U.pics[PIC_PLATE(0)], 8, 8, 240, 80, 5 - slide, 89, 230, 42, 110);
        G_FillA(5 - slide, 89, 230, 42, 0x000020, 80);
        for (i = 0; i < n && i < PARTY_SIZE; i++)
        {
            struct Pokemon m;
            int x = 8 - slide + i * 38, y = 91;
            u8 k, order = 0;
            StadiumCup_Entry(&m, i);
            for (k = 0; k < STADIUM_PICK; k++)
                if (r->picks[k] == i)
                    order = k + 1;
            G_RoundFill(x, y, 36, 38, order ? C_GOLD : 0x3a5088, 255);
            G_FillA(x + 2, y, 32, 1, 0xffffff, 70);
            Blit(IconTex(GetMonData(&m, MON_DATA_SPECIES)), x + 2, y - 6 + ((U.vsCursor == i && (U.age / 8) % 2) ? -1 : 0),
                 32, 32, 255);
            snprintf(line, sizeof(line), "Lv%d", GetMonData(&m, MON_DATA_LEVEL));
            G_TextAlign(x + 18, y + 25, TA_CENTER, order ? C_INK : G_WHITE, order ? G_NO_SHADOW : 0x000000, line, TF_NARROW);
            if (order)
            {
                G_Circle(x + 6, y + 6, 5, C_RED, 255);
                snprintf(line, sizeof(line), "%d", order);
                G_TextAlign(x + 6, y + 1, TA_CENTER, G_WHITE, G_NO_SHADOW, line, TF_NARROW);
            }
            if (U.vsCursor == i)
                G_Focus(x, y, 36, 38, G_WHITE, G_WHITE);
        }
    }
    // the clauses as chips, the level total
    {
        const struct StadiumCupInfo *c = StadiumCup_Info(r->cup);
        int x = 4;
        static const char *const sShort[5] = {"SLEEP", "FREEZE", "SELF-KO", "SPECIES", "NO ITEMS"};
        for (i = 0; i < (c->levelSum ? 4 : 5); i++)
            x += Chip(x, 134, sShort[i], 0x585880) + 1;
        if (c->levelSum)
        {
            u16 sum = 0;
            for (i = 0; i < STADIUM_PICK; i++)
                if (r->picks[i] < n)
                {
                    struct Pokemon m;
                    StadiumCup_Entry(&m, r->picks[i]);
                    sum += GetMonData(&m, MON_DATA_LEVEL);
                }
            snprintf(line, sizeof(line), "Lv %d/%d", sum, c->levelSum);
            Chip(236 - G_Measure(line, TF_NARROW) - 8, 134, line, sum > c->levelSum ? C_RED : 0x40a050);
        }
    }
    // SELECT: what each clause means
    if (U.rules)
    {
        int y = 20;
        Panel(10, 18, 220, 127, C_PANEL, 0xb0a888);
        G_Text(16, y, C_INK, G_NO_SHADOW, "THE RULES", 0, TF_NORMAL);
        y += 14;
        for (i = 0; i < ARRAY_COUNT(sClauses); i++)
        {
            G_Text(16, y, 0x885020, G_NO_SHADOW, sClauses[i][0], 0, TF_NARROW);
            y = G_Text(96, y, C_INK, G_NO_SHADOW, sClauses[i][1], 128, TF_NARROW) + 2;
        }
    }
    Footer(U.rules ? "{A}{B} BACK" : "{A} PICK  {SELECT} RULES  {START} BATTLE!  {B} QUIT");
}

// After a battle: the cup's picture behind, the verdict as a banner, the opponent beaten (or the one who won), the
// road so far as eight POKé BALLs; a cup won rains confetti
static void DrawResult(void)
{
    struct StadiumRun *r = StadiumCup_Run();
    bool8 lost = U.result == RES_LOST || U.result == RES_OVER || U.result == RES_CONTINUE;
    const char *big = U.result == RES_CUP ? "CUP WON!" : U.result == RES_WON ? "YOU WIN!" : U.result == RES_CONTINUE ? "CONTINUE?" : "GAME OVER";
    SDL_Texture *pic = CupPic(r->cup);
    u8 i, total = StadiumCup_TrainerCount(r->round, r->cup, r->tier);
    extern u16 MP_StadiumBattle_TrainerPic(void);
    Header(StadiumCup_Info(r->cup)->name, NULL);
    if (pic)
        Blit(pic, 0, 16, 240, 130, lost ? 50 : 90);
    G_FillA(0, 16, 240, 130, 0x000000, 90);
    // the opponent
    Blit(TrainerTex(U.lastFoePic), 160, 26, 72, 72, lost ? 255 : 120);
    // the verdict
    G_Banner(90 + (U.age < 12 ? (12 - U.age) * 10 : 0), 52, big, lost ? C_RED : C_GOLD, 2.0f, 255);
    // the road: a ball for each trainer, lit for the beaten
    if (total > 8)
        total = 8;
    Panel(20, 92, 136, 22, 0x0c1424, 0x4a5c88);
    for (i = 0; i < total; i++)
    {
        bool8 beaten = U.result == RES_CUP || i < r->trainer;   // (after a win the run is on the next one already)
        ItemBall(32 + i * 16, 103, beaten ? 255 : 90);
    }
    if (r->cup != STADIUM_CUP_CASTLE)
    {
        char line[48];
        snprintf(line, sizeof(line), "CONTINUES %d", r->continues);
        G_TextAlign(88, 118, TA_CENTER, G_WHITE, 0x000000, line, TF_SMALL);
    }
    if (U.result == RES_CUP)
    {
        for (i = 0; i < 40; i++)   // confetti
        {
            int x = (i * 53 + U.age * (1 + i % 3)) % 240, y = 16 + (i * 37 + U.age * (2 + i % 2)) % 130;
            static const unsigned sColors[5] = {0xf8d040, 0xe85040, 0x50a0f0, 0x60d070, 0xf080c0};
            G_Fill(x, y, 2, 3, sColors[i % 5]);
        }
        G_TextAlign(120, 128, TA_CENTER, 0xf8e8a0, 0x000000, "Your prizes are on their way.", TF_SMALL);
    }
    else if (U.result == RES_CONTINUE)
        G_TextAlign(120, 128, TA_CENTER, G_WHITE, 0x000000, "Use a continue and try again?", TF_SMALL);
    Footer(U.result == RES_CONTINUE ? "{A} CONTINUE  {B} GIVE UP" : "{A} OK");
}


// ---- SAVED TEAMS: 99 slots, the server's online (everyone's: anyone may use, copy, save over or erase any of them),
// the save's own offline (mp_stadium_teams.c)

#define TEAM_ROWS 11
static const char *const sTeamMenuFull[5] = {"USE", "COPY", "SAVE OVER", "ERASE", "CANCEL"};
static const char *const sTeamMenuEmpty[2] = {"SAVE HERE", "CANCEL"};

static void DrawTeams(void)
{
    char line[64];
    u8 i;
    const struct StadiumSavedTeam *sel = StadiumTeams_Get(U.teamCursor);
    snprintf(line, sizeof(line), "%s %d/99", StadiumTeams_IsServer() ? "SERVER" : "THIS SAVE", StadiumTeams_Count());
    Header("SAVED TEAMS", line);
    // the slots
    Panel(3, 18, 117, 127, 0x18243e, 0x4a5c88);
    for (i = 0; i < TEAM_ROWS && U.teamTop + i < STADIUM_SAVED_TEAMS; i++)
    {
        u8 n = U.teamTop + i;
        const struct StadiumSavedTeam *t = StadiumTeams_Get(n);
        int y = 20 + i * 11;
        bool8 focus = n == U.teamCursor;
        if (focus)
            G_RoundFill(5, y, 113, 11, C_GOLD, 255);
        snprintf(line, sizeof(line), "%02d", n + 1);
        G_Text(8, y, focus ? C_INK : 0x8898b8, focus ? G_NO_SHADOW : 0x000000, line, 0, TF_NARROW);
        if (t)
        {
            char owner[16];
            Ascii(owner, t->owner, sizeof(owner));
            G_Text(22, y, focus ? C_INK : G_WHITE, focus ? G_NO_SHADOW : 0x000000, owner, 44, TF_NARROW);
            {
                SDL_Rect clip = {66, y, 50, 11};
                SDL_RenderSetClipRect(U.owner, &clip);
                G_Text(66, y, focus ? 0x604010 : 0xc0b080, focus ? G_NO_SHADOW : 0x000000,
                       t->cup < STADIUM_CUP_COUNT ? StadiumCup_Info(t->cup)->name : "?", 0, TF_NARROW);
                SDL_RenderSetClipRect(U.owner, NULL);
            }
        }
        else
            G_Text(22, y, focus ? C_SOFT : 0x485878, G_NO_SHADOW, "- - - - -", 0, TF_NARROW);
    }
    // where in the 99
    G_RoundFill(116, 20, 2, 122, 0x0a1020, 255);
    G_RoundFill(116, 20 + 108 * U.teamTop / (STADIUM_SAVED_TEAMS - TEAM_ROWS), 2, 14, C_GOLD, 255);
    // the slot under the cursor
    Panel(123, 18, 114, 127, C_PANEL, 0xb0a888);
    snprintf(line, sizeof(line), "SLOT %02d", U.teamCursor + 1);
    G_Text(127, 20, C_INK, G_NO_SHADOW, line, 0, TF_SMALL);
    if (sel)
    {
        char owner[16];
        Ascii(owner, sel->owner, sizeof(owner));
        snprintf(line, sizeof(line), "by %s", owner);
        G_Text(127, 32, C_SOFT, G_NO_SHADOW, line, 106, TF_NARROW);
        G_Text(127, 42, 0x806030, G_NO_SHADOW, sel->cup < STADIUM_CUP_COUNT ? StadiumCup_Info(sel->cup)->name : "?", 106, TF_NARROW);
        for (i = 0; i < STADIUM_TEAM_SIZE; i++)
        {
            int x = 127 + (i % 3) * 36, y = 54 + (i / 3) * 34;
            G_RoundFill(x, y, 34, 32, 0xe0d8c0, 255);
            if (i < sel->count)
            {
                struct BoxPokemon *b = (struct BoxPokemon *)&sel->slot[i].mon;
                Blit(IconTex(GetBoxMonData(b, MON_DATA_SPECIES)), x + 1, y - 4, 32, 32, 255);
                snprintf(line, sizeof(line), "Lv%d", GetLevelFromBoxMonExp(b));
                G_TextAlign(x + 17, y + 21, TA_CENTER, C_INK, G_NO_SHADOW, line, TF_NARROW);
            }
            else
                ItemBall(x + 17, y + 14, 110);
        }
    }
    else
        G_Text(127, 34, C_SOFT, G_NO_SHADOW, "Empty. Press A to save this cup's registered team here.", 106, TF_NARROW);
    // A's menu, ERASE's question
    if (U.menu)
    {
        u8 rows = sel ? 5 : 2;
        int my = 60;
        Panel(150, my, 64, 6 + rows * 12, C_PANEL, 0xb0a888);
        for (i = 0; i < rows; i++)
        {
            if (U.menuCursor == i)
                G_RoundFill(153, my + 3 + i * 12, 58, 11, C_GOLD, 255);
            G_Text(158, my + 4 + i * 12, i == 3 && sel ? C_RED : C_INK, G_NO_SHADOW, sel ? sTeamMenuFull[i] : sTeamMenuEmpty[i], 0,
                   TF_SMALL);
        }
    }
    if (U.confirm)
    {
        Panel(40, 60, 160, 36, C_PANEL, 0xb0a888);
        snprintf(line, sizeof(line), "Erase slot %02d's team?", U.teamCursor + 1);
        G_TextAlign(120, 64, TA_CENTER, C_INK, G_NO_SHADOW, line, TF_SMALL);
        G_TextAlign(120, 79, TA_CENTER, C_SOFT, G_NO_SHADOW, StadiumTeams_IsServer() ? "Everyone on the server loses it." : "It's gone for good.", TF_NARROW);
    }
    Footer(U.confirm ? "{A} ERASE  {B} KEEP" : U.menu ? "{A} CHOOSE  {B} BACK" : "{A} MENU  {L}{R} PAGE  {B} BACK");
}

static void TeamFromCup(struct StadiumSavedTeam *out)
{
    const struct StadiumTeam *t = &StadiumCup_Save()->team[U.cup];
    memset(out, 0, sizeof(*out));
    out->cup = U.cup;
    out->count = t->count;
    memcpy(out->owner, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    out->owner[PLAYER_NAME_LENGTH] = EOS;
    out->ownerId = gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8);
    memcpy(out->slot, t->slot, sizeof(out->slot));
}

static void TeamAction(u8 choice)
{
    const struct StadiumSavedTeam *sel = StadiumTeams_Get(U.teamCursor);
    struct StadiumSavedTeam team;
    struct StadiumTeam *cupTeam = &StadiumCup_Save()->team[U.cup];
    if (!sel)
        choice = choice == 0 ? 2 : 4;   // (an empty slot's SAVE HERE is SAVE OVER)
    switch (choice)
    {
    case 0:   // USE: the six become this cup's registered team (anyone's: no permission needed)
        cupTeam->count = sel->count;
        memcpy(cupTeam->slot, sel->slot, sizeof(cupTeam->slot));
        StadiumCup_SaveNow();
        PlaySE(SE_SELECT);
        snprintf(U.note, sizeof(U.note), "Slot %02d's team is registered for this cup.", U.teamCursor + 1);
        MPNet_Log("stadium teams: used slot %d", U.teamCursor + 1);
        break;
    case 1:   // COPY: into the first free slot
        team = *sel;
        {
            u8 to = StadiumTeams_FirstFree();
            if (to == 0xFF || !StadiumTeams_Put(to, &team))
            {
                PlaySE(SE_FAILURE);
                snprintf(U.note, sizeof(U.note), "All 99 slots are taken.");
            }
            else
            {
                PlaySE(SE_SELECT);
                snprintf(U.note, sizeof(U.note), "Copied to slot %02d.", to + 1);
            }
        }
        break;
    case 2:   // SAVE HERE / SAVE OVER: this cup's registered team
        if (cupTeam->count == 0)
        {
            PlaySE(SE_FAILURE);
            snprintf(U.note, sizeof(U.note), "Register a team for this cup first.");
            break;
        }
        TeamFromCup(&team);
        StadiumTeams_Put(U.teamCursor, &team);
        PlaySE(SE_SAVE);
        snprintf(U.note, sizeof(U.note), "Saved in slot %02d.", U.teamCursor + 1);
        break;
    case 3:   // ERASE: asked once
        U.confirm = TRUE;
        PlaySE(SE_SELECT);
        break;
    default:
        PlaySE(SE_SELECT);
        break;
    }
}

// ---------------------------------------------------------------------------------------------------------------
// The canvas (display.c), the frame (sdl2.c: spectators, test shots)

SDL_Texture *MP_StadiumUi_Canvas(SDL_Renderer *r)
{
    SDL_Texture *previous;
    SDL_Rect oldViewport, oldClip;
    SDL_bool clipped;
    float oldX, oldY;
    if (!U.active)
        return NULL;
    G_Begin(r);
    if (U.owner != r)
    {
        memset(U.pics, 0, sizeof(U.pics));
        memset(U.greyBalls, 0, sizeof(U.greyBalls));
        memset(U.sprites, 0, sizeof(U.sprites));
        U.canvas = NULL;
        U.owner = r;
    }
    if (!U.canvas)
    {
        U.canvas = SDL_CreateTexture(r, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SU_W, SU_H);
        if (!U.canvas)
            return NULL;
        SDL_SetTextureBlendMode(U.canvas, SDL_BLENDMODE_NONE);
        SDL_SetTextureScaleMode(U.canvas, SDL_ScaleModeLinear);
        MakePics(r);
    }
    previous = SDL_GetRenderTarget(r);
    SDL_RenderGetViewport(r, &oldViewport);
    clipped = SDL_RenderIsClipEnabled(r);
    SDL_RenderGetClipRect(r, &oldClip);
    SDL_RenderGetScale(r, &oldX, &oldY);
    SDL_SetRenderTarget(r, U.canvas);
    SDL_RenderSetViewport(r, NULL);
    SDL_RenderSetClipRect(r, NULL);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_RenderSetScale(r, 2, 2);
    Background();
    switch (U.screen)
    {
    case SCR_SELECT: DrawSelect(); break;
    case SCR_CLASS: DrawClass(); break;
    case SCR_OPTIONS: DrawOptions(); break;
    case SCR_REGISTER: DrawRegister(); break;
    case SCR_VS: DrawVs(); break;
    case SCR_RESULT: DrawResult(); break;
    case SCR_TEAMS: DrawTeams(); break;
    }
    U.frameRead = FALSE;
    SDL_SetRenderTarget(r, previous);
    SDL_RenderSetViewport(r, &oldViewport);
    SDL_RenderSetClipRect(r, clipped ? &oldClip : NULL);
    SDL_RenderSetScale(r, oldX, oldY);
    return U.canvas;
}

const u16 *MP_StadiumUi_Frame(void)
{
    if (!U.active || !U.canvas || !U.owner)
        return NULL;
    if (!U.frameRead)
        U.frameRead = MP_CardScreen_Halve(U.owner, U.canvas, U.frame) != NULL;
    return U.frameRead ? U.frame : NULL;
}

bool8 MP_StadiumUi_IsActive(void)
{
    return U.active;
}

// ---------------------------------------------------------------------------------------------------------------
// Input

static void Close(void)
{
    StadiumCup_SaveNow();
    U.active = FALSE;
    U.draining = TRUE;
    PlaySE(SE_PC_OFF);
    MPNet_Log("stadium: screens closed");
    UnlockPlayerFieldControls();
    ScriptContext_Enable();   // (the attendant's script goes on: data/maps/BattleIsland_Stadium/scripts.inc)
}

void MP_StadiumUi_Open(void)
{
    struct StadiumSave *s = StadiumCup_Save();
    memset(U.note, 0, sizeof(U.note));
    U.active = TRUE;
    U.screen = SCR_SELECT;
    U.cup = s->lastCup < STADIUM_CUP_COUNT ? s->lastCup : 0;
    U.round = 0;
    U.prevKeys = 0xFFFF;
    PlaySE(SE_PC_LOGIN);
    MPNet_Log("stadium: screens opened");
}

static void EnterOptions(void)
{
    struct StadiumSave *s = StadiumCup_Save();
    U.screen = SCR_OPTIONS;
    U.opt = s->options[U.cup] & 0x80 ? s->options[U.cup] & 0x7F : STADIUM_OPT_DEFAULT;
    if (U.opt & STADIUM_OPT_RENTALS)
        U.opt &= ~STADIUM_OPT_LEVEL_CAP;
    U.optCursor = U.cup == STADIUM_CUP_CHALLENGE ? 2 : 0;
}

static void AfterBattle(bool8 won);
static void OpenSummary(void);
static void ScrollList(void);

static void ToVs(void)
{
    struct StadiumRun *r = StadiumCup_Run();
    U.screen = SCR_VS;
    U.vsCursor = 0;
    memset(r->picks, 0xFF, sizeof(r->picks));
    StadiumCup_FoePicks();
}

static void StartRun(void)
{
    const char *problem = TeamProblem();
    if (problem)
    {
        PlaySE(SE_FAILURE);
        snprintf(U.note, sizeof(U.note), "%s", problem);
        return;
    }
    StadiumCup_Begin(U.round, U.cup, U.tier, U.opt);
    PlaySE(SE_SUCCESS);
    ToVs();
}

static void Battle(void)
{
    struct StadiumRun *r = StadiumCup_Run();
    const struct StadiumCupInfo *c = StadiumCup_Info(r->cup);
    u8 i, picked = 0;
    u16 sum = 0;
    for (i = 0; i < STADIUM_PICK; i++)
        if (r->picks[i] < StadiumCup_EntryCount())
        {
            struct Pokemon m;
            StadiumCup_Entry(&m, r->picks[i]);
            sum += GetMonData(&m, MON_DATA_LEVEL);
            picked++;
        }
    if (picked < STADIUM_PICK)
    {
        PlaySE(SE_FAILURE);
        snprintf(U.note, sizeof(U.note), "Choose three POKéMON.");
        return;
    }
    if (c->levelSum && sum > c->levelSum)
    {
        PlaySE(SE_FAILURE);
        snprintf(U.note, sizeof(U.note), "Their levels add up to more than %d.", c->levelSum);
        return;
    }
    U.active = FALSE;   // (the battle; the screens come back after it: MP_StadiumUi_Update)
    U.draining = TRUE;
    StadiumCup_StartBattle(AfterBattle);
}

static void AfterBattle(bool8 won)
{
    extern u16 MP_StadiumBattle_TrainerPic(void);
    U.pending = TRUE;
    U.reopen = SCR_RESULT;
    U.lastFoePic = MP_StadiumBattle_TrainerPic();
    if (won)
        U.result = StadiumCup_Advance() ? RES_CUP : RES_WON;
    else
        U.result = StadiumCup_Run()->continues > 0 ? RES_CONTINUE : RES_OVER;
    if (U.result == RES_OVER)
        StadiumCup_UseContinue();
    memset(U.note, 0, sizeof(U.note));
}

// The list scrolled so the cursor shows
static void ScrollList(void)
{
    if (U.listCursor >= U.listCount)
        U.listCursor = U.listCount ? U.listCount - 1 : 0;
    while (U.listCursor < U.listTop)
        U.listTop -= REG_COLS;
    while (U.listCursor >= U.listTop + REG_VIEW)
        U.listTop += REG_COLS;
}

// ---- the game's own SUMMARY of the list (rentals too): the field fades, the summary, back to the field and here ----

#define SUMMARY_MAX 200
static struct Pokemon sSummaryMons[SUMMARY_MAX];
static u16 sSummaryBase;

static void CB2_StadiumSummaryDone(void)
{
    U.listCursor = sSummaryBase + gLastViewedMonIndex;
    MPNet_Log("stadium: back from the summary at %d", U.listCursor);
    U.reopen = SCR_REGISTER;
    U.pending = TRUE;
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_OpenSummary(u8 taskId)
{
    u16 n, i;
    if (gPaletteFade.active)
        return;
    sSummaryBase = U.listCursor >= SUMMARY_MAX / 2 ? U.listCursor - SUMMARY_MAX / 2 : 0;
    n = U.listCount - sSummaryBase;
    if (n > SUMMARY_MAX)
        n = SUMMARY_MAX;
    for (i = 0; i < n; i++)
        BoxMonToMon(&U.list[sSummaryBase + i].mon, &sSummaryMons[i]);
    MPNet_Log("stadium: summary of %d (%d listed)", U.listCursor, U.listCount);
    CleanupOverworldWindowsAndTilemaps();
    ShowPokemonSummaryScreen(SUMMARY_MODE_LOCK_MOVES, sSummaryMons, U.listCursor - sSummaryBase, n - 1,
                             CB2_StadiumSummaryDone);
    DestroyTask(taskId);
}

static void OpenSummary(void)
{
    if (U.listCursor >= U.listCount)
        return;
    PlaySE(SE_SELECT);
    U.active = FALSE;   // (the field fades under nothing; the screens come back after: MP_StadiumUi_Update)
    U.draining = TRUE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    CreateTask(Task_OpenSummary, 0);
}

// Every frame (MP_FrameUpdate): back to the screens once the battle has handed the field back
void MP_StadiumUi_Update(void)
{
    if (U.pending && MP_IsInOverworld() && !gMain.inBattle)
    {
        U.pending = FALSE;
        U.active = TRUE;
        U.screen = U.reopen;
        MPNet_Log("stadium: screens back (%s)", U.reopen == SCR_RESULT ? "result" : "register");
        U.prevKeys = 0xFFFF;
        U.age = 0;
        if (U.screen == SCR_REGISTER)
            BuildList(), ScrollList();
    }
}

static void MoveCup(int dx, int dy)
{
    int c = U.cup, k;
    (void)dy;
    for (k = 0; k < STADIUM_CUP_COUNT; k++)   // (a cup not open yet is grey and skipped)
    {
        c = (c + dx + STADIUM_CUP_COUNT) % STADIUM_CUP_COUNT;
        if (StadiumCup_Open(U.round, c, 0))
            break;
    }
    if (!StadiumCup_Open(U.round, c, 0))
        c = U.cup;
    if (c != U.cup)
    {
        U.cup = c;
        PlaySE(SE_SELECT);
    }
}

bool8 MP_StadiumUi_Step(u16 keys)
{
    u16 pressed;
    struct StadiumRun *r = StadiumCup_Run();
    if (U.draining)
    {
        if (keys & (A_BUTTON | B_BUTTON | START_BUTTON))
            return U.active;
        U.draining = FALSE;
    }
    if (!U.active)
        return FALSE;
    U.age++;
    pressed = keys & ~U.prevKeys;
    U.prevKeys = keys;
    if (pressed)
        U.note[0] = 0;
    switch (U.screen)
    {
    case SCR_SELECT:
        if (pressed & B_BUTTON)
            Close();
        else if (pressed & (L_BUTTON | R_BUTTON))
        {
            U.round ^= 1;
            PlaySE(SE_SELECT);
        }
        else if (pressed & DPAD_UP) MoveCup(-1, 0);
        else if (pressed & DPAD_DOWN) MoveCup(1, 0);
        else if (pressed & A_BUTTON)
        {
            if (!StadiumCup_Open(U.round, U.cup, 0))
            {
                PlaySE(SE_FAILURE);
                snprintf(U.note, sizeof(U.note), "Not open yet.");
            }
            else if (StadiumCup_Info(U.cup)->tiers > 1)
            {
                U.screen = SCR_CLASS;
                U.tier = 0;
                while (U.tier < 3 && StadiumCup_Open(U.round, U.cup, U.tier + 1) && StadiumCup_Cleared(U.round, U.cup, U.tier))
                    U.tier++;
                PlaySE(SE_SELECT);
            }
            else
            {
                U.tier = 0;
                EnterOptions();
                PlaySE(SE_SELECT);
            }
        }
        break;
    case SCR_CLASS:
        if (pressed & B_BUTTON)
            U.screen = SCR_SELECT, PlaySE(SE_SELECT);
        else if (pressed & DPAD_UP && U.tier > 0)
            U.tier--, PlaySE(SE_SELECT);
        else if (pressed & DPAD_DOWN && U.tier < 3 && StadiumCup_Open(U.round, U.cup, U.tier + 1))
            U.tier++, PlaySE(SE_SELECT);   // (a class not open yet is grey and can't be chosen)
        else if (pressed & A_BUTTON)
        {
            if (StadiumCup_Open(U.round, U.cup, U.tier))
                EnterOptions(), PlaySE(SE_SELECT);
            else
            {
                PlaySE(SE_FAILURE);
                snprintf(U.note, sizeof(U.note), "Win the class before it first.");
            }
        }
        break;
    case SCR_OPTIONS:
    {
        u8 rows = U.cup == STADIUM_CUP_CHALLENGE ? 0 : 4;
        if (pressed & B_BUTTON)
            U.screen = StadiumCup_Info(U.cup)->tiers > 1 ? SCR_CLASS : SCR_SELECT, PlaySE(SE_SELECT);
        else if (pressed & DPAD_UP && U.optCursor > (rows ? 0 : 2))
            U.optCursor--, PlaySE(SE_SELECT);
        else if (pressed & DPAD_DOWN && U.optCursor < rows)
            U.optCursor++, PlaySE(SE_SELECT);
        else if (pressed & DPAD_RIGHT && U.optCursor >= rows && U.optCursor < rows + 2)
            U.optCursor++, PlaySE(SE_SELECT);
        else if (pressed & DPAD_LEFT && U.optCursor > rows && U.optCursor > (rows ? 0 : 2))
            U.optCursor--, PlaySE(SE_SELECT);
        else if (pressed & A_BUTTON)
        {
            if (U.optCursor == 0 && (U.opt & STADIUM_OPT_RENTALS))
            {
                // RENTALS ONLY keeps LEVEL CAP off: rentals are made for the cup already (the user: only that way round)
                PlaySE(SE_FAILURE);
                snprintf(U.note, sizeof(U.note), "Rentals are already the cup's level.");
            }
            else if (U.optCursor < rows)
            {
                U.opt ^= 1 << U.optCursor;
                if (U.optCursor == 3 && (U.opt & STADIUM_OPT_RENTALS))
                    U.opt &= ~STADIUM_OPT_LEVEL_CAP;
                PlaySE(SE_SELECT);
            }
            else if (U.optCursor == rows && rows)
            {
                U.screen = SCR_REGISTER;
                U.tab = (U.opt & STADIUM_OPT_RENTALS) ? SRC_TAB_RENTALS : SRC_TAB_PARTY;
                U.listCursor = 0, U.listTop = 0;
                BuildList();
                PlaySE(SE_SELECT);
            }
            else if (U.optCursor == rows + 1 && rows)
            {
                U.screen = SCR_TEAMS;
                U.confirm = FALSE, U.menu = FALSE;
                PlaySE(SE_SELECT);
            }
            else if (U.optCursor == rows + 2)
                StartRun();
        }
        break;
    }
    case SCR_TEAMS:
    {
        bool8 full = StadiumTeams_Get(U.teamCursor) != NULL;
        if (StadiumTeams_TakeFull())
            snprintf(U.note, sizeof(U.note), "The server's 99 slots are full.");
        if (U.confirm)
        {
            if (pressed & A_BUTTON)
            {
                StadiumTeams_Erase(U.teamCursor);
                U.confirm = FALSE;
                PlaySE(SE_SELECT);
                snprintf(U.note, sizeof(U.note), "Slot %02d erased.", U.teamCursor + 1);
            }
            else if (pressed & B_BUTTON)
                U.confirm = FALSE, PlaySE(SE_SELECT);
        }
        else if (U.menu)
        {
            u8 rows = full ? 5 : 2;
            if (pressed & B_BUTTON)
                U.menu = FALSE, PlaySE(SE_SELECT);
            else if (pressed & DPAD_UP && U.menuCursor > 0)
                U.menuCursor--, PlaySE(SE_SELECT);
            else if (pressed & DPAD_DOWN && U.menuCursor + 1 < rows)
                U.menuCursor++, PlaySE(SE_SELECT);
            else if (pressed & A_BUTTON)
            {
                U.menu = FALSE;
                TeamAction(U.menuCursor);
            }
        }
        else if (pressed & B_BUTTON)
            U.screen = SCR_OPTIONS, PlaySE(SE_SELECT);
        else if (pressed & A_BUTTON)
            U.menu = TRUE, U.menuCursor = 0, memset(U.note, 0, sizeof(U.note)), PlaySE(SE_SELECT);
        else if (pressed & (DPAD_UP | DPAD_DOWN | L_BUTTON | R_BUTTON))
        {
            int c = U.teamCursor + (pressed & DPAD_UP ? -1 : pressed & DPAD_DOWN ? 1 : pressed & L_BUTTON ? -TEAM_ROWS : TEAM_ROWS);
            if (c < 0) c = 0;
            if (c >= STADIUM_SAVED_TEAMS) c = STADIUM_SAVED_TEAMS - 1;
            U.teamCursor = c;
            if (U.teamCursor < U.teamTop)
                U.teamTop = U.teamCursor;
            if (U.teamCursor >= U.teamTop + TEAM_ROWS)
                U.teamTop = U.teamCursor - TEAM_ROWS + 1;
            memset(U.note, 0, sizeof(U.note));
            PlaySE(SE_SELECT);
        }
        break;
    }
    case SCR_REGISTER:
        if (U.menu)
        {
            if (pressed & B_BUTTON)
                U.menu = FALSE, PlaySE(SE_SELECT);
            else if (pressed & DPAD_UP && U.menuCursor > 0)
                U.menuCursor--, PlaySE(SE_SELECT);
            else if (pressed & DPAD_DOWN && U.menuCursor < 2)
                U.menuCursor++, PlaySE(SE_SELECT);
            else if (pressed & A_BUTTON)
            {
                U.menu = FALSE;
                if (U.menuCursor == 0)
                    ToggleEntry();
                else if (U.menuCursor == 1)
                    OpenSummary();
                else
                    PlaySE(SE_SELECT);
            }
        }
        else if (pressed & B_BUTTON)
        {
            StadiumCup_SaveNow();
            U.screen = SCR_OPTIONS;
            PlaySE(SE_SELECT);
        }
        else if (pressed & (L_BUTTON | R_BUTTON))
        {
            U.tab = (U.tab + (pressed & R_BUTTON ? 1 : SRC_TAB_COUNT - 1)) % SRC_TAB_COUNT;
            U.listCursor = 0, U.listTop = 0;
            BuildList();
            PlaySE(SE_SELECT);
        }
        else if (pressed & A_BUTTON && U.listCursor < U.listCount)
        {
            U.menu = TRUE;
            U.menuCursor = 0;
            memset(U.note, 0, sizeof(U.note));
            PlaySE(SE_SELECT);
        }
        else if (pressed & SELECT_BUTTON && U.listCursor < U.listCount)
            OpenSummary();
        else if (pressed & (DPAD_LEFT | DPAD_RIGHT | DPAD_UP | DPAD_DOWN) && U.listCount)
        {
            int c = U.listCursor;
            if (pressed & DPAD_LEFT) c--;
            if (pressed & DPAD_RIGHT) c++;
            if (pressed & DPAD_UP) c -= REG_COLS;
            if (pressed & DPAD_DOWN) c += REG_COLS;
            if (c < 0) c = 0;
            if (c >= U.listCount) c = U.listCount - 1;
            U.listCursor = c;
            ScrollList();
            memset(U.note, 0, sizeof(U.note));
            PlaySE(SE_SELECT);
        }
        break;
    case SCR_VS:
    {
        u8 n = StadiumCup_EntryCount();
        if (U.rules)
        {
            if (pressed & (A_BUTTON | B_BUTTON | SELECT_BUTTON))
                U.rules = FALSE, PlaySE(SE_SELECT);
        }
        else if (pressed & B_BUTTON)
        {
            r->active = FALSE;
            U.screen = SCR_SELECT;
            PlaySE(SE_SELECT);
        }
        else if (pressed & SELECT_BUTTON)
            U.rules = TRUE, PlaySE(SE_SELECT);
        else if (pressed & START_BUTTON)
            Battle();
        else if (pressed & A_BUTTON)
        {
            u8 k, at = 0xFF;
            for (k = 0; k < STADIUM_PICK; k++)
                if (r->picks[k] == U.vsCursor)
                    at = k;
            if (at != 0xFF)   // (picked: unpicked, and the ones after it move up)
            {
                for (k = at; k + 1 < STADIUM_PICK; k++)
                    r->picks[k] = r->picks[k + 1];
                r->picks[STADIUM_PICK - 1] = 0xFF;
            }
            else
                for (k = 0; k < STADIUM_PICK; k++)
                    if (r->picks[k] == 0xFF)
                    {
                        r->picks[k] = U.vsCursor;
                        break;
                    }
            PlaySE(SE_SELECT);
        }
        else if (pressed & DPAD_LEFT && U.vsCursor % 3)
            U.vsCursor--, PlaySE(SE_SELECT);
        else if (pressed & DPAD_RIGHT && U.vsCursor % 3 < 2 && U.vsCursor + 1 < n)
            U.vsCursor++, PlaySE(SE_SELECT);
        else if (pressed & DPAD_UP && U.vsCursor >= 3)
            U.vsCursor -= 3, PlaySE(SE_SELECT);
        else if (pressed & DPAD_DOWN && U.vsCursor + 3 < n)
            U.vsCursor += 3, PlaySE(SE_SELECT);
        break;
    }
    case SCR_RESULT:
        if (U.result == RES_CONTINUE)
        {
            if (pressed & A_BUTTON)
            {
                StadiumCup_UseContinue();
                ToVs();
                PlaySE(SE_SELECT);
            }
            else if (pressed & B_BUTTON)
            {
                r->active = FALSE;
                U.screen = SCR_SELECT;
                PlaySE(SE_SELECT);
            }
        }
        else if (pressed & A_BUTTON)
        {
            if (U.result == RES_WON)
                ToVs();
            else
                U.screen = SCR_SELECT;
            PlaySE(SE_SELECT);
        }
        break;
    }
    return TRUE;
}
