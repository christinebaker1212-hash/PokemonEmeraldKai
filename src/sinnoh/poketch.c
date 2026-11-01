// Moemon Kai: the POKéTCH (ITEM_POKETCH, a key item usable in every region): Platinum's watch of apps, drawn over the
// game's picture while it's open (MP_Poketch_Composite) with Platinum's own app screens and sprites (sinnoh/
// platinum_gfx.pak "PKCH": tools/sinnoh/build_poketch.py), in LCD shades colored by the COLOR CHANGER. While open it
// takes the buttons (MP_Poketch_FilterKeys, from ReadKeys): L / R or the side button change the app, B closes it, the
// D-pad moves a stylus and A touches with it; a mouse click touches where it points (Platform_PoketchMouse). Apps
// come with SINNOH's badges (sApps[].badges). Its state: "<save>.poketch".
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include "global.h"
#include "main.h"
#include "event_data.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "random.h"
#include "rtc.h"
#include "sound.h"
#include "fieldmap.h"
#include "constants/songs.h"
#include "constants/flags.h"
#include "constants/event_bg.h"
#include "constants/rgb.h"
#include "mp.h"
#include "mp_region.h"
#include "battle_main.h"
#include "daycare.h"
#include "item.h"
#include "berry.h"
#include "constants/event_objects.h"
#include "constants/berry.h"
#include "constants/items.h"
#include "constants/pokemon.h"

const u8 *S3D_Gfx_PoketchScreen(const char *name);
bool8 S3D_Gfx_PoketchCell(const char *sheet, u16 cell, s16 *x, s16 *y, u16 *w, u16 *h, const u8 **px);
bool8 Platform_PoketchMouse(s32 *x, s32 *y, bool8 *down);   // (sdl2.c: the mouse in game pixels; FALSE: not moved)
extern char gPlatformSavePath[];

#define PW 192
#define PH 160
#define PX 24          // the LCD's left on the game's screen
#define MEMO_W 160     // the MEMO PAD's drawing area (its tools on the right)

enum
{
    APP_WATCH, APP_CALC, APP_MEMO, APP_PEDOMETER, APP_PARTY, APP_COUNTER, APP_ANALOG, APP_COIN, APP_COLOR, APP_STOPWATCH,
    APP_DOWSING, APP_CALENDAR, APP_LINK, APP_FRIENDSHIP, APP_DAYCARE, APP_HISTORY, APP_MAP, APP_MOVES, APP_MATCHUP,
    APP_ROULETTE, APP_TIMER, APP_ALARM, APP_DOTS, APP_BACKLIGHT, APP_BERRIES, APP_COUNT
};
static const struct { u8 badges; } sApps[APP_COUNT] = {
    [APP_WATCH] = {0}, [APP_CALC] = {0}, [APP_MEMO] = {0}, [APP_PEDOMETER] = {0}, [APP_PARTY] = {0},
    [APP_COUNTER] = {1}, [APP_ANALOG] = {1}, [APP_COIN] = {1}, [APP_COLOR] = {2}, [APP_STOPWATCH] = {3},
    [APP_DOWSING] = {2}, [APP_CALENDAR] = {4}, [APP_LINK] = {0}, [APP_FRIENDSHIP] = {1}, [APP_DAYCARE] = {2},
    [APP_HISTORY] = {3}, [APP_MAP] = {3}, [APP_MOVES] = {4}, [APP_MATCHUP] = {5}, [APP_ROULETTE] = {5},
    [APP_TIMER] = {6}, [APP_ALARM] = {6}, [APP_DOTS] = {7}, [APP_BACKLIGHT] = {0}, [APP_BERRIES] = {2},
};

// The LCD's colors, lightest first: GREEN (Platinum's own) and the COLOR CHANGER's others
static const u8 sColors[8][4][3] = {
    {{115, 180, 115}, {82, 131, 82}, {57, 82, 49}, {16, 41, 24}},
    {{214, 198, 115}, {164, 148, 74}, {115, 98, 41}, {49, 41, 16}},
    {{238, 164, 98}, {197, 115, 57}, {139, 74, 33}, {57, 24, 8}},
    {{238, 139, 139}, {197, 90, 98}, {139, 49, 57}, {57, 16, 24}},
    {{205, 148, 222}, {156, 98, 180}, {106, 57, 131}, {41, 16, 57}},
    {{139, 172, 238}, {90, 123, 197}, {57, 82, 148}, {16, 24, 65}},
    {{123, 205, 205}, {74, 156, 164}, {41, 106, 115}, {8, 41, 49}},
    {{197, 197, 197}, {139, 139, 139}, {90, 90, 90}, {24, 24, 24}},
};

static struct
{
    bool8 loaded;
    char path[256];
    bool8 open;
    u8 app;
    s16 sx, sy;            // the stylus, in the LCD's pixels
    bool8 touch, touchNew;
    u8 cursorTimer;        // the stylus shows a while after the D-pad moved it
    u16 newKeys, heldKeys, repKeys;
    u32 frame;
    // remembered (<save>.poketch)
    u8 color;
    u32 stepBase;
    u16 counter;
    u8 memo[MEMO_W * PH / 8];
    u8 dots[24 * 20 / 4];              // the DOT ARTIST's 24x20 dots, 2 bits each
    u8 markX[6], markY[6];             // the MARKING MAP's markers
    u16 histSpecies[12];               // POKéMON HISTORY: the last caught / hatched, newest first
    u32 histPersonality[12];
    u8 alarmH, alarmM, alarmOn, backlight;
    // per app, not remembered
    s32 calcValue, calcEntry;
    u8 calcOp, calcDigits;
    bool8 calcFresh, calcError;
    u8 memoTool;           // 0 pencil, 1 eraser
    u8 coinFrames, coinSide;
    u32 swStart, swElapsed;
    bool8 swRunning;
    s16 dowseX, dowseY;
    u8 dowseFrames;
    s8 dowseFoundX, dowseFoundY;
    u8 grab;               // the MARKING MAP's marker being moved (0xFF none)
    u8 movesType[3];       // the MOVE TESTER's attacking type and the two defending ones (0xFF: none)
    u8 matchA, matchB;     // the MATCHUP CHECKER's two party slots
    s32 roulAngle, roulSpeed;
    u32 timerEnd, timerLeft; // the KITCHEN TIMER (frames)
    bool8 timerRunning;
    u8 ringing;            // the KITCHEN TIMER / ALARM ringing (frames left)
    u8 pressed;            // the button held down (an app's own numbering; 0xFF none)
    u8 testTap;            // (tests: frames left of a touch at the stylus, `mp poketchtap`)
} sP;

static u8 sCanvas[PH][PW];

// ------------------------------------------------------------------------------------------------- remembered state
#define SAVE_MAGIC 0x50544B50   // "PKTP"
struct SaveFile
{
    u32 magic; u16 version; u8 color, app; u32 stepBase; u16 counter, pad; u8 memo[MEMO_W * PH / 8];
    // version 2
    u8 dots[24 * 20 / 4]; u8 markX[6], markY[6]; u16 histSpecies[12]; u32 histPersonality[12];
    u8 alarmH, alarmM, alarmOn, backlight;
};
#define SAVE_V1_SIZE offsetof(struct SaveFile, dots)

static void LoadState(void)
{
    struct SaveFile f;
    FILE *fp;
    char path[256];
    snprintf(path, sizeof(path), "%s.poketch", gPlatformSavePath);
    if (sP.loaded && strcmp(path, sP.path) == 0)
        return;
    memset(&sP, 0, sizeof(sP));
    sP.loaded = TRUE;
    strncpy(sP.path, path, sizeof(sP.path) - 1);
    sP.stepBase = GetGameStat(GAME_STAT_STEPS);
    sP.pressed = 0xFF;
    sP.sx = PW / 2, sP.sy = PH / 2;
    sP.grab = 0xFF;
    sP.movesType[0] = 1, sP.movesType[1] = 0, sP.movesType[2] = 0xFF;
    sP.matchB = 1;
    for (u32 i = 0; i < 6; i++)
        sP.markX[i] = 172, sP.markY[i] = 16 + i * 24;
    sP.alarmH = 7;
    if ((fp = fopen(path, "rb")) == NULL)
        return;
    memset(&f, 0, sizeof(f));
    if (fread(&f, 1, sizeof(f), fp) >= SAVE_V1_SIZE && f.magic == SAVE_MAGIC && (f.version == 1 || f.version == 2))
    {
        if (f.version == 2)
        {
            memcpy(sP.dots, f.dots, sizeof(sP.dots));
            memcpy(sP.markX, f.markX, 6), memcpy(sP.markY, f.markY, 6);
            memcpy(sP.histSpecies, f.histSpecies, sizeof(sP.histSpecies));
            memcpy(sP.histPersonality, f.histPersonality, sizeof(sP.histPersonality));
            sP.alarmH = f.alarmH % 24, sP.alarmM = f.alarmM % 60, sP.alarmOn = f.alarmOn, sP.backlight = f.backlight;
        }
        sP.color = f.color % 8;
        sP.app = f.app % APP_COUNT;
        sP.stepBase = f.stepBase;
        sP.counter = f.counter;
        memcpy(sP.memo, f.memo, sizeof(sP.memo));
    }
    fclose(fp);
}

static void SaveState(void)
{
    struct SaveFile f = {SAVE_MAGIC, 2, sP.color, sP.app, sP.stepBase, sP.counter, 0};
    FILE *fp;
    memcpy(f.memo, sP.memo, sizeof(f.memo));
    memcpy(f.dots, sP.dots, sizeof(f.dots));
    memcpy(f.markX, sP.markX, 6), memcpy(f.markY, sP.markY, 6);
    memcpy(f.histSpecies, sP.histSpecies, sizeof(f.histSpecies));
    memcpy(f.histPersonality, sP.histPersonality, sizeof(f.histPersonality));
    f.alarmH = sP.alarmH, f.alarmM = sP.alarmM, f.alarmOn = sP.alarmOn, f.backlight = sP.backlight;
    if ((fp = fopen(sP.path, "wb")) != NULL)
    {
        fwrite(&f, sizeof(f), 1, fp);
        fclose(fp);
    }
}

// Apps come with badges from any region: the most the player has in one (the POKéTCH works in every region)
static bool8 AppUnlocked(u8 app)
{
    u8 r, most = 0;
    for (r = 0; r < REGION_COUNT; r++)
        if (MP_Region_BadgeCount(r) > most)
            most = MP_Region_BadgeCount(r);
    return most >= sApps[app].badges;
}

// ------------------------------------------------------------------------------------------------- drawing
static void Fill(s32 x, s32 y, s32 w, s32 h, u8 shade)
{
    s32 i, j;
    for (j = y; j < y + h; j++)
        for (i = x; i < x + w; i++)
            if (i >= 0 && i < PW && j >= 0 && j < PH)
                sCanvas[j][i] = shade;
}

static void Screen(const char *name)
{
    const u8 *px = S3D_Gfx_PoketchScreen(name);
    if (px != NULL)
        memcpy(sCanvas, px, sizeof(sCanvas));
    else
        memset(sCanvas, 0, sizeof(sCanvas));
}

static void Cell(const char *sheet, u16 cell, s32 x, s32 y)
{
    s16 cx, cy;
    u16 w, h, i, j;
    const u8 *px;
    if (!S3D_Gfx_PoketchCell(sheet, cell, &cx, &cy, &w, &h, &px))
        return;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
        {
            s32 X = x + cx + i, Y = y + cy + j;
            if (px[j * w + i] != 255 && X >= 0 && X < PW && Y >= 0 && Y < PH)
                sCanvas[Y][X] = px[j * w + i];
        }
}

// A small font for numbers and names: 3x5, 4 pixels a letter
static const u16 sFont[] = {   // (rows top first, 3 bits each, the left pixel high)
    ['0'] = 0x7B6F, ['1'] = 0x2C97, ['2'] = 0x73E7, ['3'] = 0x72CF, ['4'] = 0x5BC9, ['5'] = 0x79CF, ['6'] = 0x79EF,
    ['7'] = 0x7249, ['8'] = 0x7BEF, ['9'] = 0x7BCF, ['A'] = 0x2BED, ['B'] = 0x6BAE, ['C'] = 0x3923, ['D'] = 0x6B6E,
    ['E'] = 0x79A7, ['F'] = 0x79A4, ['G'] = 0x396B, ['H'] = 0x5BED, ['I'] = 0x7497, ['J'] = 0x126A, ['K'] = 0x5BAD,
    ['L'] = 0x4927, ['M'] = 0x5FED, ['N'] = 0x7B6D, ['O'] = 0x2B6A, ['P'] = 0x6BA4, ['Q'] = 0x2B79, ['R'] = 0x6BAD,
    ['S'] = 0x388E, ['T'] = 0x7492, ['U'] = 0x5B6F, ['V'] = 0x5B6A, ['W'] = 0x5BFD, ['X'] = 0x5AAD, ['Y'] = 0x5A92,
    ['Z'] = 0x72A7, ['.'] = 0x0002, [':'] = 0x0410, ['-'] = 0x01C0, ['/'] = 0x12A4, ['+'] = 0x05D0,
};

static s32 Text(s32 x, s32 y, const char *s, u8 shade, u8 scale)
{
    for (; *s; s++, x += 4 * scale)
    {
        u8 c = (u8)*s;
        u16 g;
        s32 r, b;
        if (c >= 'a' && c <= 'z')
            c -= 32;
        g = c < ARRAY_COUNT(sFont) ? sFont[c] : 0;
        for (r = 0; r < 5; r++)
            for (b = 0; b < 3; b++)
                if (g & (1 << (14 - r * 3 - b)))
                    Fill(x + b * scale, y + r * scale, scale, scale, shade);
    }
    return x;
}

// Platinum's 16x32 digits (its "digits" sheet), N of them right-aligned to end at X
static void Digits(s32 xRight, s32 yCenter, u32 value, u8 count, bool8 zeros)
{
    u8 i;
    for (i = 0; i < count; i++)
    {
        if (value == 0 && i > 0 && !zeros)
            break;
        Cell("digits", value % 10, xRight - 8 - i * 16, yCenter);
        value /= 10;
    }
}

static void Line(s32 x0, s32 y0, s32 x1, s32 y1, u8 shade, u8 thick)
{
    s32 dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1, n = dx > dy ? dx : dy, i;
    for (i = 0; i <= n; i++)
    {
        s32 x = x0 + (n ? (x1 - x0) * i / n : 0), y = y0 + (n ? (y1 - y0) * i / n : 0);
        Fill(x - thick / 2, y - thick / 2, thick, thick, shade);
    }
}

static bool8 TouchIn(s32 x, s32 y, s32 w, s32 h)
{
    return sP.sx >= x && sP.sx < x + w && sP.sy >= y && sP.sy < y + h;
}

// ------------------------------------------------------------------------------------------------- the apps
static void Watch(void)
{
    RtcCalcLocalTime();
    Screen("digital_watch");
    Cell("bigdigits", gLocalTime.hours / 10, 8, 40);
    Cell("bigdigits", gLocalTime.hours % 10, 48, 40);
    Cell("bigdigits", gLocalTime.minutes / 10, 104, 40);
    Cell("bigdigits", gLocalTime.minutes % 10, 144, 40);
}

static void Analog(void)
{
    static const s8 sSin[60] = {0, 10, 21, 31, 41, 50, 59, 67, 74, 81, 87, 91, 95, 98, 99, 100, 99, 98, 95, 91, 87, 81,
                                74, 67, 59, 50, 41, 31, 21, 10, 0, -10, -21, -31, -41, -50, -59, -67, -74, -81, -87, -91,
                                -95, -98, -99, -100, -99, -98, -95, -91, -87, -81, -74, -67, -59, -50, -41, -31, -21, -10};
    u32 h, m;
    RtcCalcLocalTime();
    m = gLocalTime.minutes;
    h = (gLocalTime.hours % 12) * 5 + m / 12;
    Screen("analog_watch");
    Line(100, 84, 100 + sSin[h] * 36 / 100, 84 - sSin[(h + 15) % 60] * 36 / 100, 3, 4);
    Line(100, 84, 100 + sSin[m] * 56 / 100, 84 - sSin[(m + 15) % 60] * 56 / 100, 3, 2);
    Fill(97, 81, 6, 6, 3);
}

// The CALCULATOR: Platinum's keys (7 8 9 C / 4 5 6 + - / 1 2 3 x / / 0 . =), 32 pixels a column from x 16, y 32
static const char sCalcKeys[4][5] = {{'7', '8', '9', 'C', 'C'}, {'4', '5', '6', '+', '-'}, {'1', '2', '3', '*', '/'},
                                     {'0', '0', '.', '=', '='}};

static void CalcApply(void)
{
    s64 a = sP.calcValue, b = sP.calcEntry, r = b;
    switch (sP.calcOp)
    {
    case '+': r = a + b; break;
    case '-': r = a - b; break;
    case '*': r = a * b; break;
    case '/': if (b == 0) { sP.calcError = TRUE; r = 0; } else r = a / b; break;
    }
    if (r > 9999999999LL || r < -999999999LL)
        sP.calcError = TRUE, r = 0;
    sP.calcValue = (s32)r;
}

static void CalcKey(char k)
{
    PlaySE(SE_SELECT);
    if (k == 'C')
    {
        sP.calcValue = sP.calcEntry = 0, sP.calcOp = 0, sP.calcDigits = 0, sP.calcError = FALSE, sP.calcFresh = FALSE;
        return;
    }
    if (sP.calcError)
        return;
    if (k >= '0' && k <= '9')
    {
        if (sP.calcFresh)
            sP.calcEntry = 0, sP.calcDigits = 0, sP.calcFresh = FALSE;
        if (sP.calcDigits < 9)
            sP.calcEntry = sP.calcEntry * 10 + (k - '0'), sP.calcDigits += (sP.calcEntry != 0);
        return;
    }
    if (k == '=')
        printf("[test] poketch calc: %d\n", (int)(sP.calcOp ? (CalcApply(), sP.calcOp = 0, sP.calcEntry = sP.calcValue, sP.calcFresh = TRUE, sP.calcValue) : sP.calcEntry));
    if (k == '=')
        return;
    if (k == '.')
        return;   // (whole numbers: the LCD keeps nine digits)
    if (sP.calcOp)
        CalcApply();
    else
        sP.calcValue = sP.calcEntry;
    sP.calcOp = k == '=' ? 0 : k;
    sP.calcEntry = sP.calcValue;
    sP.calcFresh = TRUE;
}

static void Calc(void)
{
    char buf[16];
    s32 r, c, x;
    if (sP.touchNew)
        for (r = 0; r < 4; r++)
            for (c = 0; c < 5; c++)
                if (TouchIn(16 + c * 32, 32 + r * 32, 30, 30))
                {
                    CalcKey(sCalcKeys[r][c]);
                    sP.pressed = r * 5 + c;
                }
    if (!sP.touch)
        sP.pressed = 0xFF;
    Screen("calculator");
    if (sP.pressed != 0xFF)   // (the key held: lit)
    {
        r = sP.pressed / 5, c = sP.pressed % 5;
        x = 16 + c * 32;
        if ((c == 4 && sCalcKeys[r][3] == sCalcKeys[r][4]) || (c == 1 && r == 3))
            x -= 32;
        for (s32 j = 32 + r * 32 + 2; j < 32 + r * 32 + 27; j++)
            for (s32 i = x + 2; i < x + ((sCalcKeys[r][c] == sCalcKeys[r][c ^ 1] && r != 2) ? 60 : 28); i++)
                if (i < PW && sCanvas[j][i] == 0)
                    sCanvas[j][i] = 1;
    }
    Fill(26, 10, 160, 13, 0);
    if (sP.calcError)
        snprintf(buf, sizeof(buf), "E");
    else
        snprintf(buf, sizeof(buf), "%d", (int)sP.calcEntry);
    Text(186 - (s32)strlen(buf) * 12 + 1, 9, buf, 3, 3);
}

static void Memo(void)
{
    s32 x, y, r = sP.memoTool ? 4 : 1;
    if (sP.touchNew && sP.sx >= MEMO_W)
    {
        sP.memoTool = sP.sy >= PH / 2;
        PlaySE(SE_SELECT);
    }
    else if (sP.touch && sP.sx < MEMO_W)
        for (y = sP.sy - r; y <= sP.sy + r; y++)
            for (x = sP.sx - r; x <= sP.sx + r; x++)
                if (x >= 0 && x < MEMO_W && y >= 0 && y < PH)
                {
                    u32 bit = y * MEMO_W + x;
                    if (sP.memoTool)
                        sP.memo[bit / 8] &= ~(1 << (bit % 8));
                    else
                        sP.memo[bit / 8] |= 1 << (bit % 8);
                }
    if (sP.newKeys & SELECT_BUTTON)
        memset(sP.memo, 0, sizeof(sP.memo));
    Screen("memo_pad");
    for (y = 0; y < PH; y++)
        for (x = 0; x < MEMO_W; x++)
            if (sP.memo[(y * MEMO_W + x) / 8] & (1 << ((y * MEMO_W + x) % 8)))
                sCanvas[y][x] = 3;
    Cell("memo_pad", sP.memoTool == 0 ? 0 : 2, 176, 40);
    Cell("memo_pad", sP.memoTool == 1 ? 1 : 3, 176, 120);
}

static void Pedometer(void)
{
    u32 steps = GetGameStat(GAME_STAT_STEPS) - sP.stepBase;
    bool8 down = sP.touch && TouchIn(64, 80, 64, 64);
    if (sP.touchNew && down)
    {
        sP.stepBase = GetGameStat(GAME_STAT_STEPS);
        PlaySE(SE_SELECT);
    }
    Screen("pedometer");
    Digits(136, 48, steps > 99999 ? 99999 : steps, 5, TRUE);
    Cell("pedometer", down ? 1 : 0, 96, 112);
}

// A POKéMON's icon in the LCD's shades (its 32x32 picture, each color by its lightness)
static void MonIcon(u16 species, u32 personality, s32 x0, s32 y0)
{
    const u8 *tiles = GetMonIconPtr(species, personality, TRUE);
    const u16 *pal = GetValidMonIconPalettePtr(species);
    s32 x, y;
    if (tiles == NULL || pal == NULL)
        return;
    for (y = 0; y < 32; y++)
        for (x = 0; x < 32; x++)
        {
            u8 b = tiles[((y / 8) * 4 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2], c = (x & 1) ? b >> 4 : b & 15;
            u16 col;
            u32 lum;
            if (c == 0)
                continue;
            col = pal[c];
            lum = (col & 31) * 3 + (col >> 5 & 31) * 6 + (col >> 10 & 31);   // (0-310)
            if (x0 + x >= 0 && x0 + x < PW && y0 + y >= 0 && y0 + y < PH)
                sCanvas[y0 + y][x0 + x] = lum > 220 ? 0 : lum > 150 ? 1 : lum > 80 ? 2 : 3;
        }
}

static void Party(void)
{
    u8 i, n = CalculatePlayerPartyCount();
    Fill(0, 0, PW, PH, 0);
    for (i = 0; i < n; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        s32 x = 8 + (i % 2) * 96, y = 6 + (i / 2) * 52;
        u32 hp = GetMonData(mon, MON_DATA_HP), max = GetMonData(mon, MON_DATA_MAX_HP);
        char buf[12];
        MonIcon(species, GetMonData(mon, MON_DATA_PERSONALITY), x, y + ((sP.frame / 16 + i) & 1));
        if (species == SPECIES_EGG)
            continue;
        Fill(x + 36, y + 14, 50, 8, 3);
        Fill(x + 37, y + 15, 48, 6, 0);
        if (max)
            Fill(x + 37, y + 15, 48 * hp / max ? 48 * hp / max : (hp ? 1 : 0), 6, hp * 4 <= max ? 3 : 2);
        snprintf(buf, sizeof(buf), "%u/%u", (unsigned)hp, (unsigned)max);
        Text(x + 36, y + 26, buf, 3, 1);
        if (sP.touchNew && TouchIn(x, y, 90, 48))
            PlayCry_Normal(species, 0);
    }
}

static void Counter(void)
{
    bool8 down = sP.touch && TouchIn(64, 88, 64, 64);
    if (sP.touchNew && down)
    {
        sP.counter = sP.counter >= 9999 ? 0 : sP.counter + 1;
        PlaySE(SE_SELECT);
    }
    if (sP.newKeys & SELECT_BUTTON)
        sP.counter = 0;
    Screen("counter");
    Digits(128, 44, sP.counter, 4, TRUE);
    Cell("counter", down ? 1 : 0, 96, 120);
}

static void Coin(void)
{
    if (sP.touchNew && sP.coinFrames == 0 && TouchIn(48, 32, 96, 96))
    {
        sP.coinFrames = 40;
        sP.coinSide = Random() & 1;
        PlaySE(SE_BALL_BOUNCE_1);
    }
    Screen("coin_toss");
    if (sP.coinFrames)
    {
        static const u8 sSpin[4] = {0, 1, 2, 1};
        sP.coinFrames--;
        Cell("coin_toss", sSpin[(sP.coinFrames / 3) % 4], 96, 72 - (sP.coinFrames > 20 ? 40 - sP.coinFrames : sP.coinFrames));
    }
    else
        Cell("coin_toss", sP.coinSide ? 2 : 0, 96, 72);
}

static void ColorChanger(void)
{
    s32 i;
    if (sP.touchNew && sP.sy >= 112 && sP.sy < 144 && sP.sx >= 16 && sP.sx < 176)
    {
        sP.color = (sP.sx - 16) * 8 / 160;
        PlaySE(SE_SELECT);
    }
    Screen("color_changer");
    for (i = 0; i < 8; i++)
        if (i == sP.color)
            Cell("color_changer", 0, 26 + i * 20, 128);
}

// The STOPWATCH: Platinum's Voltorb starts and stops it when touched; SELECT (stopped) sets it back to 0
static void Stopwatch(void)
{
    u32 t = sP.swElapsed + (sP.swRunning ? gMain.vblankCounter1 - sP.swStart : 0), cs = t * 100 / 60;
    if (sP.touchNew && TouchIn(48, 56, 96, 100))
    {
        if (sP.swRunning)
            sP.swElapsed = t, sP.swRunning = FALSE;
        else
            sP.swStart = gMain.vblankCounter1, sP.swRunning = TRUE;
        PlaySE(SE_SELECT);
    }
    if ((sP.newKeys & SELECT_BUTTON) && !sP.swRunning)
    {
        sP.swElapsed = 0;
        PlaySE(SE_SELECT);
    }
    Screen("stopwatch");
    if (cs >= 100 * 60 * 100)
        cs = 100 * 60 * 100 - 1;
    Digits(64, 48, cs / 6000, 2, TRUE);
    Digits(112, 48, cs / 100 % 60, 2, TRUE);
    Digits(160, 48, cs % 100, 2, TRUE);
}

static void Dowsing(void)
{
    const struct MapEvents *events = gMapHeader.events;
    s32 i, px, py;
    if (sP.touchNew)
    {
        // the LCD shows the 15x12 tiles around the player (12 pixels a tile); an item within 2 tiles of the touch shows
        sP.dowseX = sP.sx, sP.dowseY = sP.sy, sP.dowseFrames = 60, sP.dowseFoundX = -128;
        px = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET + (sP.sx - PW / 2) / 12;
        py = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET + (sP.sy - PH / 2) / 12;
        PlaySE(SE_DOWNPOUR_STOP);
        for (i = 0; events != NULL && i < events->bgEventCount; i++)
            if (events->bgEvents[i].kind == BG_EVENT_HIDDEN_ITEM
             && !FlagGet(events->bgEvents[i].bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START)
             && events->bgEvents[i].x >= px - 2 && events->bgEvents[i].x <= px + 2
             && events->bgEvents[i].y >= py - 2 && events->bgEvents[i].y <= py + 2)
            {
                sP.dowseFoundX = events->bgEvents[i].x - px, sP.dowseFoundY = events->bgEvents[i].y - py;
                PlaySE(SE_ITEMFINDER);
                break;
            }
    }
    Screen("dowsing_machine");
    if (sP.dowseFrames)
    {
        sP.dowseFrames--;
        Cell("dowsing_machine", 3 - sP.dowseFrames / 15, sP.dowseX, sP.dowseY);
        if (sP.dowseFoundX != -128 && (sP.dowseFrames & 8))
            Cell("dowsing_machine", 0, sP.dowseX + sP.dowseFoundX * 12, sP.dowseY + sP.dowseFoundY * 12);
    }
}

static void Calendar(void)
{
    static const u8 sDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    s32 d, first, days, x, y;
    char buf[8];
    Screen("calendar");
    days = sDays[t->tm_mon] + (t->tm_mon == 1 && (t->tm_year % 4 == 0) && ((t->tm_year + 1900) % 100 || (t->tm_year + 1900) % 400 == 0));
    first = (t->tm_wday - (t->tm_mday - 1) % 7 + 7) % 7;
    snprintf(buf, sizeof(buf), "%d", t->tm_mon + 1);
    Text(PW / 2 - (s32)strlen(buf) * 6, 4, buf, 3, 3);
    for (d = 1; d <= days; d++)
    {
        s32 cell = first + d - 1;
        x = 12 + (cell % 7) * 24, y = 30 + (cell / 7) * 21;
        snprintf(buf, sizeof(buf), "%d", (int)d);
        if (d == t->tm_mday)
        {
            Fill(x - 2, y - 2, 22, 15, 3);
            Text(x + 10 - (s32)strlen(buf) * 4, y + 1, buf, 0, 2);
        }
        else
            Text(x + 10 - (s32)strlen(buf) * 4, y + 1, buf, cell % 7 == 0 ? 2 : 3, 2);
    }
}

static void LinkSearcher(void)
{
    s32 i, row = 0;
    char name[MP_NAME_LEN + 2];
    Screen("link_searcher");
    if (!MP_IsConnected())
    {
        Text(40, 70, "NO ONE NEARBY", 3, 2);
        return;
    }
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        struct MpPlayer *p = &gMpPlayers[i];
        s32 bars;
        if (i == gMpLocalId || !p->present)
            continue;
        MP_GameToAscii(name, p->name, MP_NAME_LEN);
        if (p->state.mapGroup == gSaveBlock1Ptr->location.mapGroup && p->state.mapNum == gSaveBlock1Ptr->location.mapNum)
            bars = 4;
        else if (MP_Region_OfGroup(p->state.mapGroup) == MP_Region_Current())
            bars = 2;
        else
            bars = 1;
        Text(16, 16 + row * 36, name, 3, 2);
        for (s32 b = 0; b < 4; b++)
            Fill(140 + b * 10, 30 + row * 36 - b * 5, 7, 5 + b * 5, b < bars ? 3 : 1);
        row++;
    }
    if (row == 0)
        Text(40, 70, "NO ONE NEARBY", 3, 2);
}


// FRIENDSHIP CHECKER: the party wanders; a touch on one shows its hearts
static void Friendship(void)
{
    u8 i, n = CalculatePlayerPartyCount();
    Fill(0, 0, PW, PH, 0);
    for (i = 0; i < n; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        s32 t = sP.frame + i * 97, x = 16 + (i % 3) * 56 + ((t / 40) % 4 == 1 ? 4 : (t / 40) % 4 == 3 ? -4 : 0);
        s32 y = 24 + (i / 3) * 64 + ((t / 8) & 1);
        MonIcon(species, GetMonData(mon, MON_DATA_PERSONALITY), x, y);
        if (sP.touch && species != SPECIES_EGG && TouchIn(x, y, 32, 32))
        {
            u8 f = GetMonData(mon, MON_DATA_FRIENDSHIP);
            Cell("friendship_checker", f >= 200 ? 3 : f >= 150 ? 2 : f >= 70 ? 1 : 4, x + 16, y - 4);
            if (sP.touchNew)
                PlayCry_Normal(species, 0);
        }
    }
}

// DAY-CARE CHECKER: the two POKéMON at the DAY CARE, their levels now, and an EGG when there is one
static void Daycare(void)
{
    u8 i;
    char buf[8];
    Screen("daycare_checker");
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        struct BoxPokemon box = gSaveBlock1Ptr->daycare.mons[i].mon;
        u16 species = GetBoxMonData(&box, MON_DATA_SPECIES);
        u32 exp;
        if (species == SPECIES_NONE)
            continue;
        exp = GetBoxMonData(&box, MON_DATA_EXP) + gSaveBlock1Ptr->daycare.mons[i].steps;
        SetBoxMonData(&box, MON_DATA_EXP, &exp);
        snprintf(buf, sizeof(buf), "%d", (int)GetLevelFromBoxMonExp(&box));
        Text(26 + i * 98, 8, buf, 0, 2);
        MonIcon(species, GetBoxMonData(&box, MON_DATA_PERSONALITY), 24 + i * 96, 52);
    }
    if (gSaveBlock1Ptr->daycare.offspringPersonality)
        Cell("daycare_checker", 12, 96, 130);
}

// POKéMON HISTORY: the last twelve caught or hatched
void MP_Poketch_OnCatch(struct Pokemon *mon)
{
    u32 i;
    if (!CheckBagHasItem(ITEM_POKETCH, 1))
        return;
    LoadState();
    for (i = ARRAY_COUNT(sP.histSpecies) - 1; i > 0; i--)
        sP.histSpecies[i] = sP.histSpecies[i - 1], sP.histPersonality[i] = sP.histPersonality[i - 1];
    sP.histSpecies[0] = GetMonData(mon, MON_DATA_SPECIES);
    sP.histPersonality[0] = GetMonData(mon, MON_DATA_PERSONALITY);
    SaveState();
}

static void History(void)
{
    u32 i;
    Fill(0, 0, PW, PH, 0);
    for (i = 0; i < ARRAY_COUNT(sP.histSpecies); i++)
        if (sP.histSpecies[i] != SPECIES_NONE)
        {
            s32 x = 12 + (i % 4) * 44, y = 8 + (i / 4) * 50;
            MonIcon(sP.histSpecies[i], sP.histPersonality[i], x, y + ((sP.frame / 16 + i) & 1));
            if (sP.touchNew && TouchIn(x, y, 32, 32))
                PlayCry_Normal(sP.histSpecies[i], 0);
        }
}

// MARKING MAP: SINNOH, where the player is (blinking) and six markers to move around (Platinum's map cells: matrix
// cell (x, z) at (10 + 6x, 8 + 6(z - 5)))
bool8 S3D_PlayerMatrixCell(s32 *cx, s32 *cz);   // (s3d.c)
u8 MP_Daycare_Compatibility(const struct BoxPokemon *a, const struct BoxPokemon *b);   // (daycare.c)
void MP_RegionMap_PoketchGrid(u8 types[15][28], s8 *px, s8 *py);   // (region_map.c)

// Outside SINNOH the map is the region's own town map as cells, 6 pixels each: routes and towns
static void RegionGridMap(void)
{
    static u8 sTypes[15][28];
    s8 px, py;
    s32 x, y;
    MP_RegionMap_PoketchGrid(sTypes, &px, &py);
    Fill(0, 0, PW, PH, 0);
    for (y = 0; y < 15; y++)
        for (x = 0; x < 28; x++)
            if (sTypes[y][x])
                Fill(12 + x * 6 + (sTypes[y][x] == 1), 35 + y * 6 + (sTypes[y][x] == 1), sTypes[y][x] == 1 ? 4 : 6,
                     sTypes[y][x] == 1 ? 4 : 6, sTypes[y][x] == 1 ? 1 : 3);
    if (px >= 0 && ((sP.frame / 20) & 1))
        Cell("map", 0, 12 + px * 6 + 3, 35 + py * 6 + 3);
}

static void MarkingMap(void)
{
    s32 i, cx, cz;
    if (sP.touchNew)
        for (i = 5; i >= 0; i--)
            if (sP.sx >= sP.markX[i] - 8 && sP.sx < sP.markX[i] + 8 && sP.sy >= sP.markY[i] - 8 && sP.sy < sP.markY[i] + 8)
            {
                sP.grab = i;
                break;
            }
    if (!sP.touch)
        sP.grab = 0xFF;
    if (sP.grab != 0xFF)
        sP.markX[sP.grab] = sP.sx < 4 ? 4 : sP.sx > PW - 4 ? PW - 4 : sP.sx, sP.markY[sP.grab] = sP.sy < 4 ? 4 : sP.sy > PH - 4 ? PH - 4 : sP.sy;
    if (MP_Region_Current() == REGION_SINNOH)
    {
        Screen("marking_map");
        if (S3D_PlayerMatrixCell(&cx, &cz) && ((sP.frame / 20) & 1))
            Cell("map", 0, 10 + 6 * cx + 3, 8 + 6 * (cz - 5) + 3);
    }
    else
        RegionGridMap();
    for (i = 0; i < 6; i++)
        Cell("map", 2 + i, sP.markX[i] + 8, sP.markY[i] + 8);
}

// MOVE TESTER: an attacking type against one or two types: how well it works
static u16 TypeMult(u8 atk, u8 def)
{
    u32 i;
    for (i = 0; i + 2 < ARRAY_COUNT(gTypeEffectiveness) && gTypeEffectiveness[i] != TYPE_ENDTABLE; i += 3)
        if (gTypeEffectiveness[i] == atk && gTypeEffectiveness[i + 1] == def)
            return gTypeEffectiveness[i + 2];
    return 10;
}

static u8 TypeCount(void)
{
    return MP_Gen_Cap(MP_Region_Current()) >= 6 ? NUMBER_OF_MON_TYPES : TYPE_FAIRY;
}

static void MoveTester(void)
{
    char buf[24];
    u32 mult, k, n = TypeCount();
    for (k = 0; k < 3; k++)
    {
        s32 y = 10 + k * 34;
        if (sP.touchNew && sP.sy >= y && sP.sy < y + 28)
        {
            s32 d = sP.sx < PW / 2 ? -1 : 1, v = sP.movesType[k] == 0xFF ? (s32)n : sP.movesType[k];
            do
                v = (v + d + n + (k == 2) * 1) % (n + (k == 2));
            while (v == TYPE_MYSTERY);
            sP.movesType[k] = v == (s32)n ? 0xFF : v;
            PlaySE(SE_SELECT);
        }
    }
    Fill(0, 0, PW, PH, 0);
    for (k = 0; k < 3; k++)
    {
        s32 y = 10 + k * 34;
        const char *label = k == 0 ? "MOVE" : "TARGET";
        Fill(4, y, PW - 8, 28, k == 0 ? 1 : 0);
        Fill(4, y, PW - 8, 1, 3), Fill(4, y + 27, PW - 8, 1, 3), Fill(4, y, 1, 28, 3), Fill(PW - 5, y, 1, 28, 3);
        Text(8, y + 3, label, 2, 1);
        Text(10, y + 11, "<", 3, 2);
        Text(PW - 18, y + 11, ">", 3, 2);
        if (sP.movesType[k] == 0xFF)
            snprintf(buf, sizeof(buf), "-");
        else
            MP_GameToAscii(buf, gTypeNames[sP.movesType[k]], TYPE_NAME_LENGTH);
        Text(PW / 2 - (s32)strlen(buf) * 4, y + 10, buf, 3, 2);
    }
    mult = TypeMult(sP.movesType[0], sP.movesType[1]);
    if (sP.movesType[2] != 0xFF && sP.movesType[2] != sP.movesType[1])
        mult = mult * TypeMult(sP.movesType[0], sP.movesType[2]) / 10;
    snprintf(buf, sizeof(buf), "X%d%s", (int)(mult / 10), mult % 10 == 5 ? ".5" : mult % 10 ? ".25" : "");
    if (mult < 10 && mult % 5)
        snprintf(buf, sizeof(buf), "X0.25");
    Text(PW / 2 - (s32)strlen(buf) * 6, 118, buf, 3, 3);
    Text(PW / 2 - 40, 142, mult == 0 ? "NO EFFECT" : mult > 10 ? "SUPER" : mult < 10 ? "NOT VERY" : "NORMAL", 2, 2);
}

// MATCHUP CHECKER: two of the party (touch a side to change it) and how well they'd get along at the DAY CARE
static void Matchup(void)
{
    u8 n = CalculatePlayerPartyCount(), k, score;
    if (n == 0)
    {
        Fill(0, 0, PW, PH, 0);
        return;
    }
    if (sP.touchNew && sP.sy < 100)
    {
        u8 *slot = sP.sx < PW / 2 ? &sP.matchA : &sP.matchB;
        *slot = (*slot + 1) % n;
        PlaySE(SE_SELECT);
    }
    sP.matchA %= n, sP.matchB %= n;
    Screen("matchup_checker");
    for (k = 0; k < 2; k++)
        MonIcon(GetMonData(&gPlayerParty[k ? sP.matchB : sP.matchA], MON_DATA_SPECIES_OR_EGG),
                GetMonData(&gPlayerParty[k ? sP.matchB : sP.matchA], MON_DATA_PERSONALITY), 24 + k * 112, 40);
    score = sP.matchA == sP.matchB ? 0 : MP_Daycare_Compatibility(&gPlayerParty[sP.matchA].box, &gPlayerParty[sP.matchB].box);
    // (Platinum's hearts: none, one, two or three)
    Cell("matchup_checker", score >= 70 ? 3 : score >= 50 ? 2 : score >= 20 ? 1 : 0, 96, 128);
}

// ROULETTE: draw on the wheel, then touch the button to spin it
static void Roulette(void)
{
    s32 x, y, r = 1;
    static const s8 sSinT[16] = {0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38};
    if (sP.touchNew && sP.sx >= 152)
    {
        sP.roulSpeed = 600 + (Random() % 400);
        PlaySE(SE_SELECT);
    }
    else if (sP.touch && sP.sx < 152)
        for (y = sP.sy - r; y <= sP.sy + r; y++)
            for (x = sP.sx - r; x <= sP.sx + r; x++)
                if (x >= 0 && x < MEMO_W && y >= 0 && y < PH)
                    sP.memo[(y * MEMO_W + x) / 8] |= 1 << ((y * MEMO_W + x) % 8);
    if (sP.newKeys & SELECT_BUTTON)
        memset(sP.memo, 0, sizeof(sP.memo));
    if (sP.roulSpeed > 0)
    {
        sP.roulAngle = (sP.roulAngle + sP.roulSpeed / 40) % 1024;
        sP.roulSpeed -= 4;
        if (sP.roulSpeed <= 0)
            PlaySE(SE_DING_DONG);
    }
    Screen("roulette");
    for (y = 0; y < PH; y++)
        for (x = 0; x < 152; x++)
            if (sP.memo[(y * MEMO_W + x) / 8] & (1 << ((y * MEMO_W + x) % 8)))
                sCanvas[y][x] = 3;
    {
        u32 a = sP.roulAngle * 16 / 1024;   // (16 directions)
        Line(76, 80, 76 + sSinT[a] * 60 / 100, 80 - sSinT[(a + 4) % 16] * 60 / 100, 3, 3);
    }
}

// KITCHEN TIMER: set the minutes and seconds with the arrows, then start; Snorlax wakes when it's up
static void Timer(void)
{
    u32 left = sP.timerRunning ? (sP.timerEnd > gMain.vblankCounter1 ? sP.timerEnd - gMain.vblankCounter1 : 0) : sP.timerLeft;
    if (sP.timerRunning && left == 0)
        sP.timerRunning = FALSE, sP.timerLeft = 0, sP.ringing = 180;
    if (sP.touchNew)
    {
        if (TouchIn(56, 128, 80, 32))
        {
            if (sP.timerRunning)
                sP.timerLeft = left, sP.timerRunning = FALSE;
            else if (sP.timerLeft)
                sP.timerEnd = gMain.vblankCounter1 + sP.timerLeft, sP.timerRunning = TRUE;
            sP.ringing = 0;
            PlaySE(SE_SELECT);
        }
        else if (!sP.timerRunning && sP.sy >= 48 && sP.sy < 128)
        {
            s32 step = sP.sx < PW / 2 ? 60 * 60 : 60, d = sP.sy < 92 ? 1 : -1;
            s32 v = (s32)sP.timerLeft + d * step;
            sP.timerLeft = v < 0 ? 0 : v > 99 * 3600 + 59 * 60 ? 99 * 3600 + 59 * 60 : v;
            PlaySE(SE_SELECT);
        }
    }
    Screen("kitchen_timer");
    left = (left + 59) / 60;
    Digits(92, 94, left / 60 % 100, 2, TRUE);
    Digits(132, 94, left % 60, 2, TRUE);
    if (!sP.timerRunning)
    {
        Cell("kitchen_timer", 4, 76, 66);
        Cell("kitchen_timer", 4, 116, 66);
        Cell("kitchen_timer", 5, 76, 122);
        Cell("kitchen_timer", 5, 116, 122);
    }
    Cell("kitchen_timer", sP.timerRunning ? 9 : 7, 96, 144);
}

// ALARM CLOCK: the time it rings (touch the upper / lower half of the hours or minutes) and the switch on the right
static void Alarm(void)
{
    if (sP.touchNew)
    {
        if (sP.sx >= 160)
            sP.alarmOn ^= 1, sP.ringing = 0;
        else if (sP.sx < 80)
            sP.alarmH = (sP.alarmH + (sP.sy < 128 ? 1 : 23)) % 24;
        else
            sP.alarmM = (sP.alarmM + (sP.sy < 128 ? 1 : 59)) % 60;
        PlaySE(SE_SELECT);
    }
    Screen("alarm_clock");
    Digits(76, 128, sP.alarmH, 2, TRUE);
    Digits(120, 128, sP.alarmM, 2, TRUE);
    Fill(162, 20, 26, 12, sP.alarmOn ? 0 : 3);
    Text(166, 22, sP.alarmOn ? "ON" : "OFF", sP.alarmOn ? 3 : 0, 2);
}

// The KITCHEN TIMER and the ALARM ring wherever the POKéTCH is (it opens on the one ringing)
static void CheckRinging(void)
{
    static u8 sLastMinute = 0xFF;
    if (sP.timerRunning && gMain.vblankCounter1 >= sP.timerEnd)
    {
        sP.timerRunning = FALSE, sP.timerLeft = 0, sP.ringing = 180;
        if (!sP.open && MP_IsLocalFree())
            sP.open = TRUE, sP.app = APP_TIMER;
    }
    if (sP.alarmOn && (sP.frame % 60) == 0)
    {
        RtcCalcLocalTime();
        if (gLocalTime.hours == sP.alarmH && gLocalTime.minutes == sP.alarmM && sLastMinute != gLocalTime.minutes)
        {
            sP.ringing = 180;
            if (!sP.open && MP_IsLocalFree())
                sP.open = TRUE, sP.app = APP_ALARM;
        }
        sLastMinute = gLocalTime.minutes;
    }
    if (sP.ringing)
    {
        if ((sP.ringing % 30) == 0)
            PlaySE(SE_DING_DONG);
        sP.ringing--;
    }
}

// DOT ARTIST: touch a dot to darken it (lightest after darkest)
static void Dots(void)
{
    s32 x, y;
    if (sP.touchNew)
    {
        u32 i = (sP.sy / 8) * 24 + sP.sx / 8, v = (sP.dots[i / 4] >> ((i % 4) * 2)) & 3;
        sP.dots[i / 4] = (sP.dots[i / 4] & ~(3 << ((i % 4) * 2))) | (((v + 1) & 3) << ((i % 4) * 2));
    }
    if (sP.newKeys & SELECT_BUTTON)
        memset(sP.dots, 0, sizeof(sP.dots));
    for (y = 0; y < 20; y++)
        for (x = 0; x < 24; x++)
        {
            u32 i = y * 24 + x;
            Fill(x * 8, y * 8, 8, 8, (sP.dots[i / 4] >> ((i % 4) * 2)) & 3);
            Fill(x * 8 + 7, y * 8, 1, 8, 1), Fill(x * 8, y * 8 + 7, 8, 1, 1);
        }
}

// BACKLIGHT: its switch lights the LCD
static void Backlight(void)
{
    if (sP.touchNew && TouchIn(48, 40, 96, 80))
    {
        sP.backlight ^= 1;
        PlaySE(SE_SELECT);
    }
    Screen("backlight_toggle");
    Cell("backlight_toggle", sP.backlight ? 1 : 0, 96, 80);
}

// BERRY SEARCHER: the map's BERRY trees around (6 pixels a tile, the player in the middle): ripe ones blink
static void BerrySearcher(void)
{
    const struct MapEvents *ev = gMapHeader.events;
    s32 i, px = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.x - MAP_OFFSET;
    s32 py = gObjectEvents[gPlayerAvatar.objectEventId].currentCoords.y - MAP_OFFSET;
    Fill(0, 0, PW, PH, 0);
    for (i = (PW / 2) % 6; i < PW; i += 6)
        Fill(i, 0, 1, PH, 1);
    for (i = (PH / 2) % 6; i < PH; i += 6)
        Fill(0, i, PW, 1, 1);
    for (i = 0; ev != NULL && i < ev->objectEventCount; i++)
    {
        const struct ObjectEventTemplate *t = &ev->objectEvents[i];
        s32 x, y;
        u8 stage;
        if (TEMPLATE_GFX_ID(t) != OBJ_EVENT_GFX_BERRY_TREE)
            continue;
        x = PW / 2 + (t->x - px) * 6, y = PH / 2 + (t->y - py) * 6;
        stage = GetStageByBerryTreeId(t->trainerRange_berryTreeId);
        if (stage == BERRY_STAGE_BERRIES)
        {
            if ((sP.frame / 15) & 1)
                Fill(x - 3, y - 3, 7, 7, 3);
        }
        else
            Fill(x - 2, y - 2, 5, 5, stage == BERRY_STAGE_NO_BERRY ? 2 : 3);
    }
    Fill(PW / 2 - 3, PH / 2 - 3, 7, 7, 3);
    Fill(PW / 2 - 1, PH / 2 - 1, 3, 3, 0);
}

static void DrawApp(void)
{
    switch (sP.app)
    {
    case APP_WATCH: Watch(); break;
    case APP_CALC: Calc(); break;
    case APP_MEMO: Memo(); break;
    case APP_PEDOMETER: Pedometer(); break;
    case APP_PARTY: Party(); break;
    case APP_COUNTER: Counter(); break;
    case APP_ANALOG: Analog(); break;
    case APP_COIN: Coin(); break;
    case APP_COLOR: ColorChanger(); break;
    case APP_STOPWATCH: Stopwatch(); break;
    case APP_DOWSING: Dowsing(); break;
    case APP_CALENDAR: Calendar(); break;
    case APP_LINK: LinkSearcher(); break;
    case APP_FRIENDSHIP: Friendship(); break;
    case APP_DAYCARE: Daycare(); break;
    case APP_HISTORY: History(); break;
    case APP_MAP: MarkingMap(); break;
    case APP_MOVES: MoveTester(); break;
    case APP_MATCHUP: Matchup(); break;
    case APP_ROULETTE: Roulette(); break;
    case APP_TIMER: Timer(); break;
    case APP_ALARM: Alarm(); break;
    case APP_DOTS: Dots(); break;
    case APP_BACKLIGHT: Backlight(); break;
    case APP_BERRIES: BerrySearcher(); break;
    }
}

// ------------------------------------------------------------------------------------------------- open / input
bool8 MP_Poketch_IsOpen(void)
{
    return sP.open;
}

void MP_Poketch_Open(void)
{
    LoadState();
    if (!AppUnlocked(sP.app))
        sP.app = APP_WATCH;
    sP.open = TRUE;
    PlaySE(SE_POKENAV_ON);
    printf("[test] poketch: open, app %d\n", sP.app);
}

void MP_Poketch_Close(void)
{
    if (!sP.open)
        return;
    sP.open = FALSE;
    SaveState();
    PlaySE(SE_POKENAV_OFF);
}

static void ChangeApp(s8 dir)
{
    u8 i, app = sP.app;
    for (i = 0; i < APP_COUNT; i++)
    {
        app = (app + APP_COUNT + dir) % APP_COUNT;
        if (AppUnlocked(app))
            break;
    }
    sP.app = app;
    sP.pressed = 0xFF;
    PlaySE(SE_DEX_PAGE);
    printf("[test] poketch: app %d\n", sP.app);
}

// Shown and taking the buttons: on the field, free to move (a script or another screen closes it)
static bool8 Active(void)
{
    return sP.open && gMain.callback2 == CB2_Overworld && !gPaletteFade.active;
}

void MP_Poketch_FilterKeys(void)
{
    s32 mx, my;
    bool8 down, wasTouch;
    if (sP.loaded && gMain.callback2 == CB2_Overworld)
        CheckRinging();
    if (!sP.open)
        return;
    if (gMain.callback2 == CB2_Overworld && !gPaletteFade.active && !MP_IsLocalFree())
    {
        MP_Poketch_Close();   // (a script began: an NPC talks, a trainer spots the player...)
        return;
    }
    if (!Active())
        return;
    sP.newKeys = gMain.newKeys, sP.heldKeys = gMain.heldKeys, sP.repKeys = gMain.newAndRepeatedKeys;
    gMain.newKeys = gMain.heldKeys = gMain.newAndRepeatedKeys = 0;
    sP.frame++;
    wasTouch = sP.touch;
    if (sP.newKeys & B_BUTTON)
    {
        MP_Poketch_Close();
        return;
    }
    if (sP.newKeys & (L_BUTTON | R_BUTTON))
        ChangeApp(sP.newKeys & R_BUTTON ? 1 : -1);
    if (sP.heldKeys & DPAD_ANY)
    {
        s32 v = (sP.heldKeys & (DPAD_ANY)) && sP.cursorTimer > 40 ? 4 : 2;
        if (sP.heldKeys & DPAD_LEFT) sP.sx -= v;
        if (sP.heldKeys & DPAD_RIGHT) sP.sx += v;
        if (sP.heldKeys & DPAD_UP) sP.sy -= v;
        if (sP.heldKeys & DPAD_DOWN) sP.sy += v;
        sP.sx = sP.sx < 0 ? 0 : sP.sx >= PW ? PW - 1 : sP.sx;
        sP.sy = sP.sy < 0 ? 0 : sP.sy >= PH ? PH - 1 : sP.sy;
        if (sP.cursorTimer < 250)
            sP.cursorTimer += 2;
    }
    else if (sP.cursorTimer > 120)
        sP.cursorTimer = 120;
    sP.touch = (sP.heldKeys & A_BUTTON) != 0;
    if (sP.testTap)
        sP.touch = --sP.testTap > 0;
    if (Platform_PoketchMouse(&mx, &my, &down) || down)
    {
        if (mx >= 240 - PX + 2 && mx < 240 && my >= 40 && my < 120)   // (the side button)
        {
            if (down && !wasTouch)
                ChangeApp(1);
            sP.touch = down;
            sP.touchNew = FALSE;
            return;
        }
        if (mx >= PX && mx < PX + PW)
        {
            sP.sx = mx - PX, sP.sy = my;
            sP.touch = sP.touch || down;
            sP.cursorTimer = 0;
        }
    }
    sP.touchNew = sP.touch && !wasTouch;
    if (sP.cursorTimer && !(sP.heldKeys & DPAD_ANY))
        sP.cursorTimer--;
}

// ------------------------------------------------------------------------------------------------- the picture
void MP_Poketch_Composite(u16 *frame)
{
    s32 x, y;
    u16 lcd[4];
    if (!Active())
        return;
    DrawApp();
    sP.touchNew = FALSE;
    for (x = 0; x < 4; x++)
    {
        u32 r = sColors[sP.color][x][0], g = sColors[sP.color][x][1], b = sColors[sP.color][x][2];
        if (sP.backlight)   // (the BACKLIGHT: a quarter of the way to white)
            r += (255 - r) / 4, g += (255 - g) / 4, b += (255 - b) / 4;
        if (sP.ringing && (sP.ringing / 10) & 1)   // (ringing: the LCD flashes)
            r = 255 - r, g = 255 - g, b = 255 - b;
        lcd[x] = (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10 | 0x8000;
    }
    for (y = 0; y < PH; y++)
    {
        u16 *row = frame + y * DISPLAY_WIDTH;
        for (x = 0; x < PW; x++)
            row[PX + x] = lcd[sCanvas[y][x]];
        // the case: Platinum's red, its side button on the right
        for (x = 0; x < PX; x++)
        {
            bool8 edge = x == PX - 1 || x == 0;
            row[x] = edge ? RGB(14, 4, 6) | 0x8000 : RGB(29, 10, 11) | 0x8000;
            row[PX + PW + x] = (x == 0 || x == PX - 1) ? RGB(14, 4, 6) | 0x8000 : RGB(29, 10, 11) | 0x8000;
            if (y >= 40 && y < 120 && x >= 4 && x < PX - 4)
                row[PX + PW + x] = (y == 40 || y == 119 || x == 4 || x == PX - 5) ? RGB(16, 4, 6) | 0x8000
                                 : (x < 8 ? RGB(31, 20, 20) : RGB(31, 14, 15)) | 0x8000;
        }
    }
    if (sP.cursorTimer)   // the stylus
        for (y = 0; y < 7; y++)
            for (x = 0; x <= y && x < 5; x++)
            {
                s32 X = PX + sP.sx + x, Y = sP.sy + y;
                if (X < PX + PW && Y < PH)
                    frame[Y * DISPLAY_WIDTH + X] = (x == 0 || x == y || y == 6) ? RGB(31, 31, 31) | 0x8000 : RGB(2, 2, 2) | 0x8000;
            }
}

// Tests: `mp poketch APP` (opens it on app APP)
void MP_Poketch_Debug(u8 app)
{
    MP_Poketch_Open();
    sP.app = app % APP_COUNT;
    printf("[test] poketch: app %d\n", sP.app);
}

void MP_Poketch_DebugTap(s16 x, s16 y)
{
    sP.sx = x, sP.sy = y, sP.testTap = 4;
}
