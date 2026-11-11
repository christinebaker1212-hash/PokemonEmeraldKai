// Moemon Kai: the POKéDEX's own side (the user, 2026-11-11).
//
// The co-op POKéDEX is one for the whole group: what a friend sees or catches is added to everyone's (mp_dex.c). This
// keeps what the player saw and caught THEMSELVES beside it, so the POKéDEX can show either: OWN, or SHARED (the
// save's flags, as before). Kept in "<save>.owndex" (the save's layout never changes): a bit a species for seen and
// for caught, the list's view (OWN / SHARED, the SHOW filter) and the entry it was left on.
//
// A save from before starts with what its POKéDEX holds, less the catches a friend is credited with (mp_dex.c
// MP_Dex_CaughtBy): what a friend only showed it can't be told from its own sightings, so those count as seen.
//
// The list screen's header is drawn over the frame (MP_DexHud_Composite, src/platform/sdl2.c): the game's own
// screen has no layer left for it (its list's layer scrolls).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "mp.h"
#include "palette.h"
#include "pokedex.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "aw_file.h"
#include "mp_dexplus.h"

extern char gPlatformSavePath[512];
extern bool8 gMpDexApplying;
u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
u8 MP_RenderFontGlyph(u8 fontId, u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);
void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define DEX_BYTES ((NATIONAL_DEX_COUNT + 7) / 8)

static struct
{
    bool8 loaded;
    char path[512];
    u8 seen[DEX_BYTES], caught[DEX_BYTES];
    u8 shared, filter;
    u16 last;        // the entry the list was left on (a national number; 0 none)
    u16 order[NATIONAL_DEX_COUNT + 1], orderCount;   // the player's own catches, the first first
} sOwn;

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.owndex", gPlatformSavePath);
}

static void Write(void)
{
    char path[600];
    AwFile *f;
    u32 i;
    Path(path, sizeof(path));
    if ((f = AwFile_Open(path)) == NULL)
        return;
    AwFile_Printf(f, "owndex 1\nseen ");
    for (i = 0; i < DEX_BYTES; i++)
        AwFile_Printf(f, "%02x", sOwn.seen[i]);
    AwFile_Printf(f, "\ncaught ");
    for (i = 0; i < DEX_BYTES; i++)
        AwFile_Printf(f, "%02x", sOwn.caught[i]);
    AwFile_Printf(f, "\nview %u %u %u\n", sOwn.shared, sOwn.filter, sOwn.last);
    AwFile_Printf(f, "order");
    for (i = 0; i < sOwn.orderCount; i++)
        AwFile_Printf(f, " %u", sOwn.order[i]);
    AwFile_Printf(f, "\n");
    AwFile_Close(f);
}

static void Hex(const char *p, u8 *out)
{
    u32 i;
    for (i = 0; i < DEX_BYTES; i++)
    {
        unsigned v;
        if (sscanf(p + 2 * i, "%2x", &v) != 1)
            break;
        out[i] = (u8)v;
    }
}

static void Load(void)
{
    char path[600];
    static char line[NATIONAL_DEX_COUNT * 6 + 64];
    FILE *f;
    if (sOwn.loaded && strcmp(sOwn.path, gPlatformSavePath) == 0)
        return;
    memset(&sOwn, 0, sizeof(sOwn));
    sOwn.loaded = TRUE;
    sOwn.shared = TRUE;
    snprintf(sOwn.path, sizeof(sOwn.path), "%s", gPlatformSavePath);
    Path(path, sizeof(path));
    if ((f = fopen(path, "r")) == NULL)
    {
        // (a save from before: what its POKéDEX holds, less a friend's catches)
        u16 nat;
        if (gSaveBlock2Ptr == NULL || gSaveBlock1Ptr == NULL)
        {
            sOwn.loaded = FALSE;
            return;
        }
        for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
        {
            if (GetSetPokedexFlag(nat, FLAG_GET_SEEN))
                sOwn.seen[(nat - 1) / 8] |= 1 << ((nat - 1) % 8);
            if (GetSetPokedexFlag(nat, FLAG_GET_CAUGHT) && MP_Dex_CaughtBy(nat) == NULL)
                sOwn.caught[(nat - 1) / 8] |= 1 << ((nat - 1) % 8), sOwn.order[sOwn.orderCount++] = nat;
        }
        return;
    }
    while (fgets(line, sizeof(line), f))
    {
        unsigned a, b, c;
        if (strncmp(line, "seen ", 5) == 0)
            Hex(line + 5, sOwn.seen);
        else if (strncmp(line, "caught ", 7) == 0)
            Hex(line + 7, sOwn.caught);
        else if (sscanf(line, "view %u %u %u", &a, &b, &c) == 3)
            sOwn.shared = a != 0, sOwn.filter = b < MP_DEXSHOW_COUNT ? b : 0, sOwn.last = (u16)c;
        else if (strncmp(line, "order", 5) == 0)
        {
            char *p = line + 5;
            while (*p && sOwn.orderCount < NATIONAL_DEX_COUNT)
            {
                long v = strtol(p, &p, 10);
                if (v < 1 || v > NATIONAL_DEX_COUNT)
                    break;
                sOwn.order[sOwn.orderCount++] = (u16)v;
            }
        }
    }
    fclose(f);
}

// A new game: nothing seen
void MP_DexOwn_Reset(void)
{
    Load();
    if (!sOwn.loaded)
        return;
    memset(sOwn.seen, 0, sizeof(sOwn.seen));
    memset(sOwn.caught, 0, sizeof(sOwn.caught));
    sOwn.last = 0;
    sOwn.orderCount = 0;
    Write();
}

// pokedex.c GetSetPokedexFlag: the game marks a species seen or caught (not while a friend's entries are added)
void MP_DexOwn_Mark(u16 nat, bool8 caught)
{
    u8 mask, *bits;
    if (gMpDexApplying || nat < 1 || nat > NATIONAL_DEX_COUNT)
        return;
    Load();
    if (!sOwn.loaded)
        return;
    mask = 1 << ((nat - 1) % 8);
    bits = caught ? sOwn.caught : sOwn.seen;
    if ((bits[(nat - 1) / 8] & mask) && (sOwn.seen[(nat - 1) / 8] & mask))
        return;
    if (caught && !(sOwn.caught[(nat - 1) / 8] & mask) && sOwn.orderCount < NATIONAL_DEX_COUNT)
        sOwn.order[sOwn.orderCount++] = nat;
    bits[(nat - 1) / 8] |= mask;
    sOwn.seen[(nat - 1) / 8] |= mask;   // (caught is seen)
    Write();
}

// The player saw (caught) it themselves; never more than the save's own POKéDEX says
bool8 MP_DexOwn_Has(u16 nat, bool8 caught)
{
    if (nat < 1 || nat > NATIONAL_DEX_COUNT)
        return FALSE;
    Load();
    if (!sOwn.loaded)
        return GetSetPokedexFlag(nat, caught ? FLAG_GET_CAUGHT : FLAG_GET_SEEN);
    if (!((caught ? sOwn.caught : sOwn.seen)[(nat - 1) / 8] >> ((nat - 1) % 8) & 1))
        return FALSE;
    return GetSetPokedexFlag(nat, caught ? FLAG_GET_CAUGHT : FLAG_GET_SEEN);
}

bool8 MP_DexView_Shared(void)
{
    Load();
    return sOwn.shared;
}

u8 MP_DexView_Filter(void)
{
    Load();
    return sOwn.filter;
}

u16 MP_DexView_Last(void)
{
    Load();
    return sOwn.last;
}

void MP_DexView_Set(bool8 shared, u8 filter, u16 last)
{
    Load();
    if (sOwn.shared == shared && sOwn.filter == filter && sOwn.last == last)
        return;
    sOwn.shared = shared, sOwn.filter = filter, sOwn.last = last;
    if (sOwn.loaded)
        Write();
}

// ------------------------------------------------------------------------------------------------ the list's header
static void DrawPanel(u16 *frame);
static void DrawInfo(u16 *frame);
static bool8 InfoFresh(void);
static const char *const sEggNames[16];
static bool8 PanelIsOpen(void);
static const char *const sShowNames[MP_DEXSHOW_COUNT];
#define INFO_ROWS_MAX 9   // (a page's list rows: DrawInfo's INFO_ROWS)
static struct MpDexHud sHud;
static struct { u16 nat; bool8 seen, owned; u8 page; u32 frame; bool8 on; s16 scroll, rows; } sInfo;
static u32 sHudFrame;
static bool8 sHudOn;

void MP_DexHud_Show(const struct MpDexHud *hud)
{
    sHud = *hud;
    sHudFrame = gMain.vblankCounter1;
    sHudOn = TRUE;
}

static u16 Mix(u16 a, u16 b, s32 t)   // t of 16 toward b
{
    s32 r = (a & 31) + (((b & 31) - (a & 31)) * t) / 16;
    s32 g = ((a >> 5) & 31) + ((((b >> 5) & 31) - ((a >> 5) & 31)) * t) / 16;
    s32 bl = ((a >> 10) & 31) + ((((b >> 10) & 31) - ((a >> 10) & 31)) * t) / 16;
    return (u16)(r | g << 5 | bl << 10 | 0x8000);   // (the frame's pixels: bit 15 set)
}

static void Box(u16 *frame, s32 x0, s32 y0, s32 x1, s32 y1, u16 color, s32 t)
{
    s32 x, y;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++)
        {
            bool8 corner = (x == x0 || x == x1) && (y == y0 || y == y1);
            if (!corner && x >= 0 && y >= 0 && x < DISPLAY_WIDTH && y < DISPLAY_HEIGHT)
                frame[y * DISPLAY_WIDTH + x] = Mix(frame[y * DISPLAY_WIDTH + x], color, t);
        }
}

// The POKéDEX's own look (the user, 2026-11-11: "not like an addition added on top, but something that was always
// there"): its START menu's blue box and white letters with their grey shadow, the pink buttons beside START and
// SELECT, the entry's pale card with dark letters
#define NO_SHADOW 0x7FFE
#define C_MENU    RGB(6, 17, 31)
#define C_MENU_HI RGB(22, 25, 30)
#define C_WHITE   RGB(31, 31, 31)
#define C_SHADOW  RGB(12, 12, 14)
#define C_PINK    RGB(31, 0, 23)
#define C_CARD    RGB(27, 27, 27)
#define C_INK     RGB(2, 2, 2)
#define C_INK_SH  RGB(23, 23, 23)

// The font the text is drawn in: the game's own (FONT_NORMAL: its menus' letters; FONT_NARROW: an entry's lines;
// FONT_SMALL: the letter inside a button)
static u8 sFont = FONT_SMALL;

// ASCII in that font; returns where it ends
static s32 Text(u16 *frame, s32 x, s32 y, const char *ascii, u16 color, u16 shadow)
{
    u8 text[48];
    u32 i;
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    for (i = 0; text[i] != EOS; i++)
    {
        s32 w, gx, gy;
        if (text[i] == CHAR_SPACE)
        {
            x += sFont == FONT_SMALL ? 3 : MP_RenderFontGlyph(sFont, CHAR_SPACE, 1, 0, 2);
            continue;
        }
        w = MP_RenderFontGlyph(sFont, text[i], 1, 0, 2);
        for (gy = 0; gy < 16; gy++)
        {
            const u32 *rows = gy < 8 ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
            for (gx = 0; gx < w && gx < 16; gx++)
            {
                u8 c = (rows[(gx < 8 ? 0 : 8) + (gy % 8)] >> ((gx % 8) * 4)) & 0xF;
                s32 px = x + gx, py = y + gy - (sFont == FONT_SMALL ? 1 : 0);
                if ((c == 1 || c == 2) && px >= 0 && py >= 0 && px < DISPLAY_WIDTH && py < DISPLAY_HEIGHT)
                    {
                    if (c == 1)
                        frame[py * DISPLAY_WIDTH + px] = (u16)(color | 0x8000);
                    else if (shadow != NO_SHADOW)
                        frame[py * DISPLAY_WIDTH + px] = (u16)(shadow | 0x8000);
                }
            }
        }
        x += w;
    }
    return x;
}

static s32 Width(const char *ascii)
{
    u8 text[48];
    u32 i;
    s32 w = 0;
    MP_AsciiToGame(text, ascii, sizeof(text) - 1);
    for (i = 0; text[i] != EOS; i++)
        w += text[i] == CHAR_SPACE && sFont == FONT_SMALL ? 3 : MP_RenderFontGlyph(sFont, text[i], 1, 0, 2);
    return w;
}

// A button as the list's own START and SELECT are drawn: a pink pill with its name in white, then what it does
static s32 Key(u16 *frame, s32 x, s32 y, const char *key, const char *label, u16 ink, u16 shadow)
{
    u8 font = sFont;
    s32 w, end, dy = font == FONT_SMALL ? 0 : 2;
    sFont = FONT_SMALL;
    w = Width(key);
    Box(frame, x, y + dy, x + w + 4, y + dy + 9, RGB(0, 0, 5), 16);
    Box(frame, x + 1, y + dy + 1, x + w + 3, y + dy + 8, C_PINK, 16);
    Text(frame, x + 3, y + dy, key, C_WHITE, NO_SHADOW);
    sFont = font;
    end = Text(frame, x + w + 7, y, label, ink, shadow) + 5;
    return end;
}

// The START menu's box
static void MenuBox(u16 *frame, s32 x0, s32 y0, s32 x1, s32 y1)
{
    Box(frame, x0, y0, x1, y1, RGB(0, 0, 5), 16);
    Box(frame, x0 + 1, y0 + 1, x1 - 1, y1 - 1, C_MENU_HI, 16);
    Box(frame, x0 + 2, y0 + 2, x1 - 2, y1 - 2, C_MENU, 16);
}

// Its cursor: the pink arrow
static void Cursor(u16 *frame, s32 x, s32 y)
{
    s32 i, k;
    for (i = 0; i < 4; i++)
        for (k = i; k < 7 - i; k++)
            if (y + k >= 0 && y + k < DISPLAY_HEIGHT)
                frame[(y + k) * DISPLAY_WIDTH + x + i] = C_PINK | 0x8000;
}

static const char *const sShowNames[MP_DEXSHOW_COUNT] = {"ALL", "NOT SEEN", "NOT CAUGHT", "CAUGHT", "FRIENDS'"};

// Nothing is drawn over the frame: the list's buttons are sprites, FIND a window on the START menu's layer and MORE a
// window on the entry's (pokedex.c). The pictures are drawn here into a 240 x 160 sheet (0: see-through) and handed
// over as tiles in the POKéDEX's own palette (MP_DexDraw_*)
void MP_DexHud_Composite(u16 *frame)
{
    (void)frame;
}

static u16 sSheet[DISPLAY_WIDTH * DISPLAY_HEIGHT];

// One palette for the three: index 0 see-through
static const u16 sDexPal[16] = {
    0, RGB(31, 31, 31), RGB(12, 12, 14), RGB(6, 17, 31), RGB(22, 25, 30), RGB(0, 0, 5), RGB(31, 0, 23), RGB(31, 28, 10),
    RGB(27, 27, 27), RGB(2, 2, 2), RGB(23, 23, 23), RGB(24, 4, 4), RGB(4, 10, 22), RGB(8, 28, 12), RGB(19, 19, 20),
    RGB(31, 12, 8),
};

const u16 *MP_DexDraw_Palette(void)
{
    return sDexPal;
}

// The sheet's W x H tiles from (X0, Y0) as 4bpp tiles, a row of tiles after another (a window's or a sprite's)
void MP_DexDraw_Tiles(s32 x0, s32 y0, s32 w, s32 h, u8 *out)
{
    s32 x, y, i;
    memset(out, 0, (size_t)(w * h * 32));
    for (y = 0; y < h * 8; y++)
        for (x = 0; x < w * 8; x++)
        {
            u16 c = y0 + y < DISPLAY_HEIGHT && x0 + x < DISPLAY_WIDTH ? sSheet[(y0 + y) * DISPLAY_WIDTH + x0 + x] : 0;
            s32 best = 0, bestD = 1 << 30;
            if (c == 0)
                continue;
            for (i = 1; i < 16; i++)
            {
                s32 dr = (c & 31) - (sDexPal[i] & 31), dg = ((c >> 5) & 31) - ((sDexPal[i] >> 5) & 31);
                s32 db = ((c >> 10) & 31) - ((sDexPal[i] >> 10) & 31), d = dr * dr + dg * dg + db * db;
                if (d < bestD)
                    bestD = d, best = i;
            }
            out[((y / 8) * w + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2] |= (u8)(best << ((x & 1) * 4));
        }
}

// The list's buttons (two sprites: under the picture, and beside the list's arrow)
void MP_DexDraw_Buttons(const struct MpDexHud *hud)
{
    u16 *frame = sSheet;
    char buf[8];
    s32 x, y, w, own, all;
    memset(sSheet, 0, sizeof(sSheet));
    sHud = *hud;
    buf[0] = 0;
    // under the picture, beside the list's own START and SELECT: which POKéDEX (L), whose (R); FIND (LT / F)
    sFont = FONT_NARROW;
    if (sHud.nationalOn)
        Key(frame, 65, 135, "L", sHud.national ? "NATIONAL" : "REGION", C_WHITE, RGB(0, 0, 5));
    Key(frame, 65, 147, "R", sHud.shared ? "SHARED" : "OWN", C_WHITE, RGB(0, 0, 5));
    Key(frame, 134, 147, "LT", sHud.nothing ? "NONE" : sHud.narrowed || sHud.filter ? "FIND ON" : "FIND", C_INK, C_INK_SH);
    // how much of the list is caught: a line along the picture's frame (the friends' part behind the player's own)
    w = 60;
    y = 133;
    own = w * sHud.ownCaught / (sHud.total ? sHud.total : 1);
    all = w * sHud.allCaught / (sHud.total ? sHud.total : 1);
    for (x = 0; x < w; x++)
        frame[y * DISPLAY_WIDTH + 66 + x] = frame[(y + 1) * DISPLAY_WIDTH + 66 + x]
            = (x < own ? RGB(31, 26, 6) : sHud.shared && x < all ? RGB(9, 20, 31) : RGB(12, 12, 14)) | 0x8000;
    (void)buf;
    sFont = FONT_SMALL;
}

void MP_DexDraw_Panel(void)
{
    memset(sSheet, 0, sizeof(sSheet));
    DrawPanel(sSheet);
}

// An entry's page (MP_DEXPAGE_*) from row SCROLL of its list; returns the row it starts from (kept inside the list)
s32 MP_DexDraw_Info(u16 nat, bool8 seen, bool8 owned, u8 page, s32 scroll)
{
    u32 pass;
    sInfo.nat = nat, sInfo.seen = seen, sInfo.owned = owned, sInfo.page = page;
    sInfo.scroll = (s16)(scroll < 0 ? 0 : scroll);
    for (pass = 0; pass < 2; pass++)
    {
        memset(sSheet, 0, sizeof(sSheet));
        DrawInfo(sSheet);
        if (sInfo.rows <= INFO_ROWS_MAX)
        {
            if (sInfo.scroll == 0)
                break;
            sInfo.scroll = 0;
        }
        else if (sInfo.scroll + INFO_ROWS_MAX > sInfo.rows)
            sInfo.scroll = sInfo.rows - INFO_ROWS_MAX;
        else
            break;
    }
    return sInfo.scroll;
}

// ------------------------------------------------------------------------------------------------ FIND & SORT
// SELECT on the list: a panel over it (the user's picks, 2026-11-11): jump to an entry by typing, more sort orders,
// and the list narrowed by generation, type, Ability, move or egg group. Ability, move, egg group and the stat
// orders only ever tell of POKéMON caught (in the view): nothing is given away.
#include "battle_main.h"
#include "data.h"
#include "item.h"
#include "pokemon.h"
#include "sound.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/moves.h"
#include "constants/abilities.h"
#include "constants/songs.h"
#include "platform_text.h"

void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
u16 ItemIdToBattleMoveId(u16 item);

enum { ROW_JUMP, ROW_SORT, ROW_SHOW, ROW_GEN, ROW_TYPE1, ROW_TYPE2, ROW_ABILITY, ROW_MOVE, ROW_EGG, ROW_CLEAR, ROW_COUNT };

static struct MpDexQuery sQuery = {0, 0, 0xFF, 0xFF, 0, 0, 0};
static struct
{
    bool8 open, typing, closing;
    s16 slide;        // how far below its place it is: it slides up as START's menu does, and back down
    u8 row;
    char jump[12];
    u8 hasAbility[(ABILITIES_COUNT + 7) / 8], hasMove[(MOVES_COUNT + 7) / 8];
} sPanel;

static const char *const sSortNames[MP_DEXSORT_COUNT] = {
    "NUMBER", "A TO Z", "HEAVIEST", "LIGHTEST", "TALLEST", "SMALLEST", "HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF",
    "SPEED", "TOTAL", "FIRST CAUGHT",
};
static const char *const sGenNames[10] = {
    "ANY", "1 KANTO", "2 JOHTO", "3 HOENN", "4 SINNOH", "5 UNOVA", "6 KALOS", "7 ALOLA", "8 GALAR", "9 PALDEA",
};
static const u16 sGenLast[10] = {0, 151, 251, 386, 493, 649, 721, 809, 905, 1025};
static const char *const sEggNames[16] = {
    "ANY", "MONSTER", "WATER 1", "BUG", "FLYING", "FIELD", "FAIRY", "GRASS", "HUMAN-LIKE", "WATER 3", "MINERAL",
    "AMORPHOUS", "WATER 2", "DITTO", "DRAGON", "NO EGGS",
};

static bool8 PanelIsOpen(void)
{
    return sPanel.open;
}

const struct MpDexQuery *MP_DexQuery(void)
{
    return &sQuery;
}

bool8 MP_DexQuery_Active(void)
{
    return sQuery.gen || sQuery.type1 != 0xFF || sQuery.type2 != 0xFF || sQuery.ability || sQuery.move || sQuery.egg
        || sQuery.sort >= MP_DEXSORT_HP;
}

static bool8 Learns(u16 species, u16 move)
{
    u32 i;
    for (i = 0; gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
        if ((gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_ID) == move)
            return TRUE;
    for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
        if (ItemIdToBattleMoveId(ITEM_TM01 + i) == move && CanSpeciesLearnTMHM(species, i))
            return TRUE;
    return FALSE;
}

// Whether the list keeps the entry (seen / owned: in the list's view)
bool8 MP_DexQuery_Match(u16 nat, bool8 seen, bool8 owned)
{
    u16 species = NationalPokedexNumToSpecies(nat);
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    if (sQuery.gen && (nat <= sGenLast[sQuery.gen - 1] || nat > sGenLast[sQuery.gen]))
        return FALSE;
    if (sQuery.type1 != 0xFF || sQuery.type2 != 0xFF)
    {
        if (!seen)
            return FALSE;
        if (sQuery.type1 != 0xFF && info->types[0] != sQuery.type1 && info->types[1] != sQuery.type1)
            return FALSE;
        if (sQuery.type2 != 0xFF && info->types[0] != sQuery.type2 && info->types[1] != sQuery.type2)
            return FALSE;
    }
    if ((sQuery.ability || sQuery.move || sQuery.egg || sQuery.sort >= MP_DEXSORT_HP) && !owned)
        return FALSE;
    if (sQuery.ability && info->abilities[0] != sQuery.ability && info->abilities[1] != sQuery.ability)
        return FALSE;
    if (sQuery.egg && info->eggGroups[0] != sQuery.egg && info->eggGroups[1] != sQuery.egg)
        return FALSE;
    if (sQuery.move && !Learns(species, sQuery.move))
        return FALSE;
    return TRUE;
}

// The stat orders' key (the biggest first); FIRST CAUGHT: the earliest first
s32 MP_DexQuery_SortKey(u16 nat)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[NationalPokedexNumToSpecies(nat)];
    u32 i;
    switch (sQuery.sort)
    {
    case MP_DEXSORT_HP:    return info->baseHP;
    case MP_DEXSORT_ATK:   return info->baseAttack;
    case MP_DEXSORT_DEF:   return info->baseDefense;
    case MP_DEXSORT_SPATK: return info->baseSpAttack;
    case MP_DEXSORT_SPDEF: return info->baseSpDefense;
    case MP_DEXSORT_SPEED: return info->baseSpeed;
    case MP_DEXSORT_TOTAL: return info->baseHP + info->baseAttack + info->baseDefense + info->baseSpAttack + info->baseSpDefense + info->baseSpeed;
    case MP_DEXSORT_FIRST:
        Load();
        for (i = 0; i < sOwn.orderCount; i++)
            if (sOwn.order[i] == nat)
                return -(s32)i;
        return -(s32)(NATIONAL_DEX_COUNT + nat);   // (caught by a friend, or before the order was kept: after, by number)
    }
    return 0;
}

int MP_DexPanel_Close(void)
{
    if (sPanel.typing || sPanel.closing)
        return 0;
    sPanel.closing = TRUE;
    PlaySE(SE_PC_OFF);
    return 0;
}

const char *MP_DexPanel_Jump(void)
{
    return sPanel.jump;
}

void MP_DexPanel_Open(u8 order)
{
    u16 nat;
    u32 i;
    memset(&sPanel, 0, sizeof(sPanel));
    sPanel.open = TRUE;
    sPanel.slide = 160;
    if (sQuery.sort < MP_DEXSORT_HP)
        sQuery.sort = order;
    // (the Abilities and moves there are to pick from: the caught POKéMON's)
    for (nat = 1; nat <= NATIONAL_DEX_COUNT; nat++)
    {
        u16 species;
        bool8 owned = sOwn.shared ? GetSetPokedexFlag(nat, FLAG_GET_CAUGHT) : MP_DexOwn_Has(nat, TRUE);
        if (!owned)
            continue;
        species = NationalPokedexNumToSpecies(nat);
        for (i = 0; i < 2; i++)
            if (gSpeciesInfo[species].abilities[i] && gSpeciesInfo[species].abilities[i] < ABILITIES_COUNT)
                sPanel.hasAbility[gSpeciesInfo[species].abilities[i] / 8] |= 1 << (gSpeciesInfo[species].abilities[i] % 8);
        for (i = 0; gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
        {
            u16 m = gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_ID;
            if (m && m < MOVES_COUNT)
                sPanel.hasMove[m / 8] |= 1 << (m % 8);
        }
        for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
            if (CanSpeciesLearnTMHM(species, i))
            {
                u16 m = ItemIdToBattleMoveId(ITEM_TM01 + i);
                if (m && m < MOVES_COUNT)
                    sPanel.hasMove[m / 8] |= 1 << (m % 8);
            }
    }
    PlaySE(SE_PC_LOGIN);
}

static void NameOf(u8 row, char *out, int size)
{
    switch (row)
    {
    case ROW_JUMP:
        snprintf(out, size, "%s%s", sPanel.jump, sPanel.typing ? "_" : sPanel.jump[0] ? "" : "-");
        break;
    case ROW_SORT:  snprintf(out, size, "%s", sSortNames[sQuery.sort % MP_DEXSORT_COUNT]); break;
    case ROW_SHOW:  snprintf(out, size, "%s", sShowNames[sOwn.filter % MP_DEXSHOW_COUNT]); break;
    case ROW_GEN:   snprintf(out, size, "%s", sGenNames[sQuery.gen % 10]); break;
    case ROW_TYPE1:
    case ROW_TYPE2:
    {
        u8 t = row == ROW_TYPE1 ? sQuery.type1 : sQuery.type2;
        if (t == 0xFF)
            snprintf(out, size, "ANY");
        else
            MP_GameToAscii(out, gTypeNames[t], size - 1);
        break;
    }
    case ROW_ABILITY:
        if (!sQuery.ability)
            snprintf(out, size, "ANY");
        else
            MP_GameToAscii(out, gAbilityNames[sQuery.ability], size - 1);
        break;
    case ROW_MOVE:
        if (!sQuery.move)
            snprintf(out, size, "ANY");
        else
            MP_GameToAscii(out, gMoveNames[sQuery.move], size - 1);
        break;
    case ROW_EGG:   snprintf(out, size, "%s", sEggNames[sQuery.egg % 16]); break;
    default:        out[0] = 0; break;
    }
}

// The next Ability / move there is to pick, DIR along; LETTER: the next whose name starts with another letter
static u16 NextOf(const u8 *has, u16 count, u16 cur, s32 dir, bool8 letter, const u8 *(*name)(u16))
{
    u16 n = cur, first = cur ? name(cur)[0] : 0;
    u32 guard;
    for (guard = 0; guard <= count; guard++)
    {
        n = (u16)((n + count + dir) % count);
        if (n == 0)
            return 0;   // (ANY)
        if ((has[n / 8] >> (n % 8) & 1) && (!letter || name(n)[0] != first))
            return n;
    }
    return 0;
}

static const u8 *AbilityName(u16 id) { return gAbilityNames[id]; }
static const u8 *MoveName(u16 id) { return gMoveNames[id]; }

static void Cycle(s32 dir, bool8 letter)
{
    switch (sPanel.row)
    {
    case ROW_JUMP:
        // (a first letter, for a controller)
        if (sPanel.jump[0] < 'A' || sPanel.jump[0] > 'Z' || sPanel.jump[1])
            snprintf(sPanel.jump, sizeof(sPanel.jump), dir > 0 ? "A" : "Z");
        else
            sPanel.jump[0] = (char)('A' + (sPanel.jump[0] - 'A' + 26 + dir) % 26);
        break;
    case ROW_SORT:  sQuery.sort = (u8)((sQuery.sort + MP_DEXSORT_COUNT + dir) % MP_DEXSORT_COUNT); break;
    case ROW_SHOW:
        sOwn.filter = (u8)((sOwn.filter + MP_DEXSHOW_COUNT + dir) % MP_DEXSHOW_COUNT);
        if (sOwn.filter == MP_DEXSHOW_FRIENDS && !sOwn.shared)
            sOwn.filter = dir > 0 ? MP_DEXSHOW_ALL : MP_DEXSHOW_CAUGHT;
        Write();
        break;
    case ROW_GEN:   sQuery.gen = (u8)((sQuery.gen + 10 + dir) % 10); break;
    case ROW_TYPE1:
    case ROW_TYPE2:
    {
        u8 *t = sPanel.row == ROW_TYPE1 ? &sQuery.type1 : &sQuery.type2;
        s32 v = *t == 0xFF ? -1 : *t;
        do
            v = (v + 1 + dir + NUMBER_OF_MON_TYPES + 1) % (NUMBER_OF_MON_TYPES + 1) - 1;
        while (v == TYPE_MYSTERY);
        *t = v < 0 ? 0xFF : (u8)v;
        break;
    }
    case ROW_ABILITY: sQuery.ability = NextOf(sPanel.hasAbility, ABILITIES_COUNT, sQuery.ability, dir, letter, AbilityName); break;
    case ROW_MOVE:    sQuery.move = NextOf(sPanel.hasMove, MOVES_COUNT, sQuery.move, dir, letter, MoveName); break;
    case ROW_EGG:     sQuery.egg = (u8)((sQuery.egg + 16 + dir) % 16); break;
    }
    PlaySE(SE_SELECT);
}

// A frame of the panel: 0 still open, 1 closed (the list is to be made again), 2 the game's own SEARCH asked for
int MP_DexPanel_Step(void)
{
    if (!sPanel.open)
        return 1;
    if (sPanel.closing)
    {
        sPanel.open = sPanel.closing = FALSE;   // (pokedex.c slides its layer down and away, as START's menu goes)
        return 1;
    }
    if (sPanel.typing)
    {
        int c;
        while ((c = Platform_PopTypedChar()) != 0)
        {
            size_t n = strlen(sPanel.jump);
            if (c == TYPED_ENTER)
            {
                Platform_SetTextInput(0);
                sPanel.typing = FALSE;
                sPanel.closing = TRUE;
                return 0;
            }
            if (c == TYPED_BACKSPACE)
            {
                if (n)
                    sPanel.jump[n - 1] = 0;
            }
            else if (n + 1 < sizeof(sPanel.jump) && c >= ' ' && c < 127)
                sPanel.jump[n] = (char)(c >= 'a' && c <= 'z' ? c - 32 : c), sPanel.jump[n + 1] = 0;
        }
        if (JOY_NEW(B_BUTTON))
        {
            Platform_SetTextInput(0);
            sPanel.typing = FALSE;
        }
        return 0;
    }
    if (JOY_NEW(B_BUTTON | SELECT_BUTTON | START_BUTTON))
    {
        sPanel.closing = TRUE;
        PlaySE(SE_PC_OFF);
        return 0;
    }
    if (JOY_REPEAT(DPAD_UP))
        sPanel.row = (u8)((sPanel.row + ROW_COUNT - 1) % ROW_COUNT), PlaySE(SE_SELECT);
    else if (JOY_REPEAT(DPAD_DOWN))
        sPanel.row = (u8)((sPanel.row + 1) % ROW_COUNT), PlaySE(SE_SELECT);
    else if (JOY_REPEAT(DPAD_LEFT))
        Cycle(-1, FALSE);
    else if (JOY_REPEAT(DPAD_RIGHT))
        Cycle(1, FALSE);
    else if (JOY_REPEAT(L_BUTTON))
        Cycle(-1, TRUE);
    else if (JOY_REPEAT(R_BUTTON))
        Cycle(1, TRUE);
    else if (JOY_NEW(A_BUTTON))
    {
        if (sPanel.row == ROW_JUMP)
        {
            if (sPanel.jump[0] && !sPanel.jump[1] && sPanel.jump[0] >= 'A' && sPanel.jump[0] <= 'Z')
            {
                sPanel.closing = TRUE;   // (a letter picked with the pad: there at once)
                return 0;
            }
            sPanel.jump[0] = 0;
            sPanel.typing = TRUE;
            Platform_SetTextInput(1);
            while (Platform_PopTypedChar())
                ;
        }
        else if (sPanel.row == ROW_CLEAR)
        {
            sQuery.gen = 0, sQuery.type1 = sQuery.type2 = 0xFF, sQuery.ability = sQuery.move = 0, sQuery.egg = 0;
            sQuery.sort = MP_DEXSORT_NUMBER;
            sOwn.filter = MP_DEXSHOW_ALL;
            sPanel.jump[0] = 0;
            Write();
            PlaySE(SE_SELECT);
        }
        else
            Cycle(1, FALSE);
    }
    return 0;
}

static void DrawPanel(u16 *frame)
{
    static const char *const sLabels[ROW_COUNT] = {
        "JUMP TO", "ORDER", "SHOW", "GENERATION", "TYPE", "TYPE 2", "ABILITY", "MOVE", "EGG GROUP", "CLEAR ALL",
    };
    s32 i, y, o = 0;
    char buf[40];
    // (the START menu's own box and letters, as tall as its rows need)
    MenuBox(frame, 24, 6 + o, 215, 157 + o);
    sFont = FONT_NORMAL;
    for (i = 0; i < ROW_COUNT; i++)
    {
        bool8 on = i == sPanel.row;
        y = 11 + 14 * i + o;
        if (on)
            Cursor(frame, 28, y + 4);
        Text(frame, 38, y, sLabels[i], C_WHITE, C_SHADOW);
        NameOf((u8)i, buf, sizeof(buf));
        if (buf[0])
            Text(frame, 124, y, buf, on ? RGB(31, 30, 14) : C_WHITE, C_SHADOW);
    }
    sFont = FONT_SMALL;
}

// ------------------------------------------------------------------------------------------------ an entry's MORE
// R on an entry (the user's picks, 2026-11-11): three pages over it. STATS for a POKéMON caught (its types, Abilities,
// egg groups and base stats), its EVOLUTION line (the ones not seen yet as ?????), and WHERE it's found in the wild
// in the region the player is in (the game's own encounter tables).
#include "overworld.h"
#include "region_map.h"
#include "wild_encounter.h"
#include "mp_region.h"
#include "constants/maps.h"

extern const struct Evolution gEvolutionTable[NUM_SPECIES][EVOS_PER_MON];



void MP_DexInfo_Show(u16 nat, bool8 seen, bool8 owned, u8 page)
{
    sInfo.nat = nat, sInfo.seen = seen, sInfo.owned = owned, sInfo.page = page;
    sInfo.frame = gMain.vblankCounter1;
    sInfo.on = TRUE;
}

static bool8 ViewSeen(u16 species)
{
    u16 nat = SpeciesToNationalPokedexNum(species);
    return sOwn.shared ? GetSetPokedexFlag(nat, FLAG_GET_SEEN) : MP_DexOwn_Has(nat, FALSE);
}

static void SpeciesAscii(u16 species, char *out, int size)
{
    u8 name[POKEMON_NAME_LENGTH + 1];
    GetSpeciesName(name, species);
    MP_GameToAscii(out, name, size - 1);
}

static void EvoHow(const struct Evolution *e, char *out, int size)
{
    char item[24];
    item[0] = 0;
    switch (e->method)
    {
    case EVO_FRIENDSHIP:        snprintf(out, size, "FRIENDSHIP"); break;
    case EVO_FRIENDSHIP_DAY:    snprintf(out, size, "FRIENDSHIP, DAY"); break;
    case EVO_FRIENDSHIP_NIGHT:  snprintf(out, size, "FRIENDSHIP, NIGHT"); break;
    case EVO_LEVEL: case EVO_LEVEL_ATK_GT_DEF: case EVO_LEVEL_ATK_EQ_DEF: case EVO_LEVEL_ATK_LT_DEF:
    case EVO_LEVEL_SILCOON: case EVO_LEVEL_CASCOON: case EVO_LEVEL_NINJASK: case EVO_LEVEL_SHEDINJA:
    case EVO_LEVEL_MALE: case EVO_LEVEL_FEMALE:
        snprintf(out, size, "LV %u", e->param);
        break;
    case EVO_TRADE:             snprintf(out, size, "TRADE"); break;
    case EVO_BEAUTY:            snprintf(out, size, "BEAUTY"); break;
    case EVO_TRADE_ITEM: case EVO_ITEM: case EVO_ITEM_MALE: case EVO_ITEM_FEMALE:
    case EVO_ITEM_HOLD: case EVO_ITEM_HOLD_DAY: case EVO_ITEM_HOLD_NIGHT:
        MP_GameToAscii(item, GetItemName(e->param), sizeof(item) - 1);
        snprintf(out, size, "%s%s", e->method == EVO_TRADE_ITEM ? "TRADE + " : e->method >= EVO_ITEM_HOLD_DAY && e->method <= EVO_ITEM_HOLD ? "HOLD " : "", item);
        break;
    case EVO_MOVE:
        MP_GameToAscii(item, gMoveNames[e->param], sizeof(item) - 1);
        snprintf(out, size, "KNOW %s", item);
        break;
    case EVO_SPECIES_IN_PARTY:  snprintf(out, size, "WITH A FRIEND"); break;
    default:                    snprintf(out, size, "SPECIAL"); break;
    }
}

static s32 EvoLines(u16 *frame, u16 species, s32 depth, s32 y, const char *how)
{
    char name[16], line[48];
    u32 i;
    if (y > 126 || depth > 3)
        return y;
    if (ViewSeen(species))
        SpeciesAscii(species, name, sizeof(name));
    else
        snprintf(name, sizeof(name), "?????");
    Text(frame, 18 + depth * 10, y, name, NationalPokedexNumToSpecies(sInfo.nat) == species ? RGB(24, 4, 4) : C_INK, C_INK_SH);
    if (how[0])
    {
        snprintf(line, sizeof(line), "%s", how);
        Text(frame, 124, y, line, RGB(4, 10, 22), C_INK_SH);
    }
    y += 11;
    for (i = 0; i < EVOS_PER_MON; i++)
        if (gEvolutionTable[species][i].method && gEvolutionTable[species][i].targetSpecies)
        {
            char next[40];
            EvoHow(&gEvolutionTable[species][i], next, sizeof(next));
            y = EvoLines(frame, gEvolutionTable[species][i].targetSpecies, depth + 1, y, next);
        }
    return y;
}

static bool8 InTable(const struct WildPokemonInfo *info, u32 count, u16 species)
{
    u32 i;
    if (info == NULL)
        return FALSE;
    for (i = 0; i < count; i++)
        if (info->wildPokemon[i].species == species)
            return TRUE;
    return FALSE;
}

// A list's rows from the scroll on: TRUE while row K is one to draw, at *Y
#define INFO_ROWS 9
static bool8 Row(s32 k, s32 *y)
{
    if (k < sInfo.scroll || k >= sInfo.scroll + INFO_ROWS)
        return FALSE;
    *y = 34 + (k - sInfo.scroll) * 11;
    return TRUE;
}

static void DrawInfo(u16 *frame)
{
    static const char *const sPages[MP_DEXPAGE_WHERE] = {"PROFILE", "STATS", "EVOLUTION", "MOVES BY LEVEL", "MOVES BY TM", "WHERE"};
    u16 species = NationalPokedexNumToSpecies(sInfo.nat);
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    char buf[64], a[24], b[24];
    s32 y = 34, i;
    sInfo.rows = 0;
    sFont = FONT_NARROW;
    // (the entry's own card: pale, a grey strip for its heading, dark letters)
    Box(frame, 8, 16, 223, 143, RGB(0, 0, 5), 16);
    Box(frame, 9, 17, 222, 142, C_WHITE, 16);
    Box(frame, 11, 19, 220, 140, C_CARD, 16);
    Box(frame, 11, 19, 220, 31, RGB(23, 23, 23), 16);
    SpeciesAscii(species, a, sizeof(a));
    snprintf(buf, sizeof(buf), "%s  %s", a, sPages[(sInfo.page - 1) % MP_DEXPAGE_WHERE]);
    Text(frame, 16, 19, buf, C_INK, C_WHITE);
    if (sInfo.page < MP_DEXPAGE_WHERE)
    {
        snprintf(buf, sizeof(buf), "< %d/%d >", sInfo.page, MP_DEXPAGE_WHERE - 1);
        Text(frame, 216 - Width(buf), 19, buf, C_INK, C_WHITE);
    }
    if (!sInfo.owned && (sInfo.page == MP_DEXPAGE_PROFILE || sInfo.page == MP_DEXPAGE_STATS || sInfo.page == MP_DEXPAGE_LEVEL
                         || sInfo.page == MP_DEXPAGE_TM))
    {
        // (nothing of one not caught is given away)
        Text(frame, 18, 62, "CATCH IT TO FILL IN THIS PAGE.", C_INK, C_INK_SH);
        sFont = FONT_SMALL;
        return;
    }
    switch (sInfo.page)
    {
    case MP_DEXPAGE_PROFILE:
        MP_GameToAscii(a, gTypeNames[info->types[0]], sizeof(a) - 1);
        MP_GameToAscii(b, gTypeNames[info->types[1]], sizeof(b) - 1);
        snprintf(buf, sizeof(buf), info->types[0] == info->types[1] ? "%s" : "%s / %s", a, b);
        Text(frame, 18, y, "TYPE", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        MP_GameToAscii(a, gAbilityNames[info->abilities[0]], sizeof(a) - 1);
        MP_GameToAscii(b, gAbilityNames[info->abilities[1]], sizeof(b) - 1);
        snprintf(buf, sizeof(buf), info->abilities[1] && info->abilities[1] != info->abilities[0] ? "%s / %s" : "%s", a, b);
        Text(frame, 18, y, "ABILITY", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        if (info->genderRatio == MON_GENDERLESS)
            snprintf(buf, sizeof(buf), "NONE");
        else if (info->genderRatio == MON_MALE)
            snprintf(buf, sizeof(buf), "ALWAYS MALE");
        else if (info->genderRatio == MON_FEMALE)
            snprintf(buf, sizeof(buf), "ALWAYS FEMALE");
        else
        {
            // (the game's own rule: female when the low byte of its number is under the ratio)
            s32 tenths = info->genderRatio * 1000 / 256;
            tenths = tenths > 120 && tenths < 130 ? 125 : tenths > 870 && tenths < 880 ? 875 : (tenths + 2) / 5 * 5;
            snprintf(buf, sizeof(buf), "MALE %d.%d%%  FEMALE %d.%d%%", (int)(1000 - tenths) / 10, (int)(1000 - tenths) % 10,
                     (int)tenths / 10, (int)tenths % 10);
        }
        Text(frame, 18, y, "GENDER", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        snprintf(buf, sizeof(buf), info->eggGroups[0] == info->eggGroups[1] ? "%s" : "%s / %s",
                 sEggNames[info->eggGroups[0] % 16], sEggNames[info->eggGroups[1] % 16]);
        Text(frame, 18, y, "EGG GROUPS", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        // (an EGG's count goes down one every 32 steps: daycare.c)
        if (info->eggGroups[0] == EGG_GROUP_NO_EGGS_DISCOVERED)
            snprintf(buf, sizeof(buf), "NO EGGS");
        else
            snprintf(buf, sizeof(buf), "%d STEPS", info->eggCycles * 32);
        Text(frame, 18, y, "EGG HATCHES", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        snprintf(buf, sizeof(buf), "%d OF 255", info->catchRate);
        Text(frame, 18, y, "CATCH RATE", C_INK, C_INK_SH), Text(frame, 84, y, buf, RGB(4, 10, 22), C_INK_SH), y += 13;
        break;
    case MP_DEXPAGE_STATS:
    {
        static const char *const sStat[6] = {"HP", "ATTACK", "DEFENSE", "SP. ATK", "SP. DEF", "SPEED"};
        u8 v[6] = {info->baseHP, info->baseAttack, info->baseDefense, info->baseSpAttack, info->baseSpDefense, info->baseSpeed};
        s32 total = 0, x;
        for (i = 0; i < 6; i++)
        {
            s32 w = v[i] * 96 / 200;
            Text(frame, 18, y, sStat[i], C_INK, C_INK_SH);
            snprintf(buf, sizeof(buf), "%d", v[i]);
            Text(frame, 74, y, buf, C_INK, C_INK_SH);
            if (w > 110)
                w = 110;
            Box(frame, 100, y + 4, 100 + 110, y + 8, RGB(19, 19, 20), 16);
            for (x = 0; x < w; x++)
                frame[(y + 5) * DISPLAY_WIDTH + 101 + x] = frame[(y + 6) * DISPLAY_WIDTH + 101 + x] = frame[(y + 7) * DISPLAY_WIDTH + 101 + x]
                    = (v[i] >= 110 ? RGB(8, 28, 12) : v[i] >= 70 ? RGB(31, 26, 6) : RGB(31, 12, 8)) | 0x8000;
            total += v[i];
            y += 13;
        }
        snprintf(buf, sizeof(buf), "%d", (int)total);
        Text(frame, 18, y + 3, "TOTAL", RGB(24, 4, 4), C_INK_SH), Text(frame, 74, y + 3, buf, RGB(24, 4, 4), C_INK_SH);
        break;
    }
    case MP_DEXPAGE_EVO:
    {
        // (back to the first of its line: whoever evolves into it, three steps at most)
        u16 base = species, sp;
        for (i = 0; i < 3; i++)
        {
            bool8 found = FALSE;
            u32 k;
            for (sp = 1; sp < NUM_SPECIES && !found; sp++)
                for (k = 0; k < EVOS_PER_MON; k++)
                    if (gEvolutionTable[sp][k].method && gEvolutionTable[sp][k].targetSpecies == base)
                    {
                        base = sp, found = TRUE;
                        break;
                    }
            if (!found)
                break;
        }
        if (EvoLines(frame, base, 0, y, "") == y + 11)
            Text(frame, 18, y + 14, "IT DOESN'T EVOLVE.", C_INK, C_INK_SH);
        break;
    }
    case MP_DEXPAGE_LEVEL:
        for (i = 0; gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
        {
            u32 e = gLevelUpLearnsets[species][i];
            if (Row(i, &y))
            {
                snprintf(buf, sizeof(buf), "LV %d", (int)(e >> 16));
                MP_GameToAscii(a, gMoveNames[e & LEVEL_UP_MOVE_ID], sizeof(a) - 1);
                Text(frame, 18, y, buf, RGB(4, 10, 22), C_INK_SH), Text(frame, 64, y, a, C_INK, C_INK_SH);
            }
        }
        sInfo.rows = i;
        break;
    case MP_DEXPAGE_TM:
    {
        s32 n = 0;
        for (i = 0; i < NUM_TMHMS; i++)
        {
            u16 item = TMHM_ITEM(i);
            if (!CanSpeciesLearnTMHM(species, i))
                continue;
            if (Row(n, &y))
            {
                MP_GameToAscii(b, GetItemName(item), sizeof(b) - 1);
                MP_GameToAscii(a, gMoveNames[ItemIdToBattleMoveId(item)], sizeof(a) - 1);
                Text(frame, 18, y, b, RGB(4, 10, 22), C_INK_SH), Text(frame, 64, y, a, C_INK, C_INK_SH);
            }
            n++;
        }
        sInfo.rows = n;
        if (n == 0)
            Text(frame, 18, 62, "IT LEARNS NONE.", C_INK, C_INK_SH);
        break;
    }
    case MP_DEXPAGE_WHERE:
    {
        struct { u16 sec; u8 kinds; } place[INFO_ROWS];
        u32 n = 0, k, h;
        u8 region = MP_Region_Current();
        for (h = 0; gWildMonHeaders[h].mapGroup != MAP_GROUP(MAP_UNDEFINED); h++)
        {
            const struct WildPokemonHeader *w = &gWildMonHeaders[h];
            u8 kinds = (InTable(w->landMonsInfo, 12, species) ? 1 : 0) | (InTable(w->waterMonsInfo, 5, species) ? 2 : 0)
                     | (InTable(w->fishingMonsInfo, 10, species) ? 4 : 0) | (InTable(w->rockSmashMonsInfo, 5, species) ? 8 : 0);
            u16 sec;
            if (!kinds || MP_Region_OfGroup(w->mapGroup) != region)
                continue;
            sec = Overworld_GetMapHeaderByGroupAndId(w->mapGroup, w->mapNum)->regionMapSectionId;
            for (k = 0; k < n; k++)
                if (place[k].sec == sec)
                    break;
            if (k == n && n < ARRAY_COUNT(place))
                place[n].sec = sec, place[n].kinds = 0, n++;
            if (k < ARRAY_COUNT(place))
                place[k].kinds |= kinds;
        }
        if (n == 0)
            Text(frame, 18, 62, "NOT FOUND IN THE WILD IN THIS REGION.", C_INK, C_INK_SH);
        for (k = 0; k < n; k++)
        {
            u8 name[32];
            GetMapName(name, place[k].sec, 0);
            MP_GameToAscii(a, name, sizeof(a) - 1);
            Text(frame, 18, y, a, C_INK, C_INK_SH);
            snprintf(buf, sizeof(buf), "%s%s%s%s", place[k].kinds & 1 ? "GRASS " : "", place[k].kinds & 2 ? "WATER " : "",
                     place[k].kinds & 4 ? "ROD " : "", place[k].kinds & 8 ? "ROCKS" : "");
            Text(frame, 124, y, buf, RGB(4, 10, 22), C_INK_SH);
            y += 11;
        }
        break;
    }
    }
    // (a list longer than the card: which way there is more)
    if (sInfo.rows > INFO_ROWS)
    {
        if (sInfo.scroll > 0)
            Text(frame, 206, 34, "^", RGB(24, 4, 4), C_INK_SH);
        if (sInfo.scroll + INFO_ROWS < sInfo.rows)
            Text(frame, 206, 124, "v", RGB(24, 4, 4), C_INK_SH);
    }
    sFont = FONT_SMALL;
}

static bool8 InfoFresh(void)
{
    return sInfo.on && gMain.vblankCounter1 - sInfo.frame <= 2;
}
