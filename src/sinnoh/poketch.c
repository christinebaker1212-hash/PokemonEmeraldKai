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
#include "mail.h"
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
enum { VIEW_HIDDEN, VIEW_SMALL, VIEW_LARGE };

enum
{
    APP_WATCH, APP_CALC, APP_MEMO, APP_PEDOMETER, APP_PARTY, APP_COUNTER, APP_ANALOG, APP_COIN, APP_COLOR, APP_STOPWATCH,
    APP_DOWSING, APP_CALENDAR, APP_LINK, APP_FRIENDSHIP, APP_DAYCARE, APP_HISTORY, APP_MAP, APP_MOVES, APP_MATCHUP,
    APP_ROULETTE, APP_TIMER, APP_ALARM, APP_DOTS, APP_BACKLIGHT, APP_BERRIES, APP_RADAR, APP_COUNT
};
static const struct { u8 badges; } sApps[APP_COUNT] = {
    [APP_WATCH] = {0}, [APP_CALC] = {0}, [APP_MEMO] = {0}, [APP_PEDOMETER] = {0}, [APP_PARTY] = {0},
    [APP_COUNTER] = {1}, [APP_ANALOG] = {1}, [APP_COIN] = {1}, [APP_COLOR] = {2}, [APP_STOPWATCH] = {3},
    [APP_DOWSING] = {2}, [APP_CALENDAR] = {4}, [APP_LINK] = {0}, [APP_FRIENDSHIP] = {1}, [APP_DAYCARE] = {2},
    [APP_HISTORY] = {3}, [APP_MAP] = {3}, [APP_MOVES] = {4}, [APP_MATCHUP] = {5}, [APP_ROULETTE] = {5},
    [APP_TIMER] = {6}, [APP_ALARM] = {6}, [APP_DOTS] = {7}, [APP_BACKLIGHT] = {0}, [APP_BERRIES] = {2},
    [APP_RADAR] = {0},   // (the TRAINER COUNTER: with the POKé RADAR)
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
    u8 view;               // VIEW_HIDDEN / _SMALL / _LARGE (the right stick up / down)
    bool8 cursor;          // the cursor mode (the right stick click): it takes the D-pad and A
    u8 target;             // the UI element the cursor is on, in apps that have them
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
    u16 radarSpecies[3], radarChain[3];   // the TRAINER COUNTER's best three POKé RADAR chains
    u8 alarmH, alarmM, alarmOn, backlight;
    // per app, not remembered
    s32 calcValue, calcEntry;
    u8 calcOp, calcDigits;
    bool8 calcFresh, calcError;
    u8 memoTool;           // 0 pencil, 1 eraser
    u8 coinFrames, coinSide;
    u32 swStart, swElapsed;
    bool8 swRunning;
    u16 swHold;
    s16 dowseX, dowseY;
    u8 dowseFrames;
    s8 dowseFoundX, dowseFoundY;
    u8 grab;               // the MARKING MAP's marker being moved (0xFF none)
    u8 movesType[3];       // the MOVE TESTER's attacking type and the two defending ones (0xFF: none)
    u8 movesPressed, movesPressTime;
    u8 matchA, matchB;     // the MATCHUP CHECKER's two party slots
    u8 matchScore;         // (0xFF: not asked)
    u16 matchTime;
    s32 roulAngle, roulSpeed;
    bool8 roulStopping;
    u32 timerEnd, timerLeft; // the KITCHEN TIMER (frames)
    bool8 timerRunning;
    u8 ringing;            // the KITCHEN TIMER / ALARM ringing (frames left)
    u8 pressed;            // the button held down (an app's own numbering; 0xFF none)
    u8 testTap;            // (tests: frames left of a touch at the stylus, `mp poketchtap`)
    bool8 mouseWas;
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
    // version 3
    u16 radarSpecies[3], radarChain[3];
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
    if (fread(&f, 1, sizeof(f), fp) >= SAVE_V1_SIZE && f.magic == SAVE_MAGIC && f.version >= 1 && f.version <= 3)
    {
        if (f.version >= 3)
        {
            memcpy(sP.radarSpecies, f.radarSpecies, sizeof(sP.radarSpecies));
            memcpy(sP.radarChain, f.radarChain, sizeof(sP.radarChain));
        }
        if (f.version >= 2)
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
    struct SaveFile f = {SAVE_MAGIC, 3, sP.color, sP.app, sP.stepBase, sP.counter, 0};
    FILE *fp;
    memcpy(f.memo, sP.memo, sizeof(f.memo));
    memcpy(f.dots, sP.dots, sizeof(f.dots));
    memcpy(f.markX, sP.markX, 6), memcpy(f.markY, sP.markY, 6);
    memcpy(f.histSpecies, sP.histSpecies, sizeof(f.histSpecies));
    memcpy(f.histPersonality, sP.histPersonality, sizeof(f.histPersonality));
    f.alarmH = sP.alarmH, f.alarmM = sP.alarmM, f.alarmOn = sP.alarmOn, f.backlight = sP.backlight;
    memcpy(f.radarSpecies, sP.radarSpecies, sizeof(f.radarSpecies));
    memcpy(f.radarChain, sP.radarChain, sizeof(f.radarChain));
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
    if (app == APP_RADAR && !CheckBagHasItem(ITEM_POKE_RADAR, 1))
        return FALSE;
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

// A cell at (X, Y) of the LCD; FLIP 1 mirrors it left-right and 2 up-down, around its point (Platinum's flips)
static void CellF(const char *sheet, u16 cell, s32 x, s32 y, u8 flip)
{
    s16 cx, cy;
    u16 w, h, i, j;
    const u8 *px;
    if (cell == 0xFFFF || !S3D_Gfx_PoketchCell(sheet, cell, &cx, &cy, &w, &h, &px))
        return;
    for (j = 0; j < h; j++)
        for (i = 0; i < w; i++)
        {
            s32 X = x + ((flip & 1) ? -(cx + i) - 1 : cx + i), Y = y + ((flip & 2) ? -(cy + j) - 1 : cy + j);
            if (px[j * w + i] != 255 && X >= 0 && X < PW && Y >= 0 && Y < PH)
                sCanvas[Y][X] = px[j * w + i];
        }
}

static void Cell(const char *sheet, u16 cell, s32 x, s32 y)
{
    CellF(sheet, cell, x, y, 0);
}

u16 S3D_Gfx_PoketchAnimCell(const char *sheet, u16 seq, u32 t);   // (s3d_gfx.c)

// Platinum's sprite: animation ANIM of SHEET at its point (DX, DY) on the DS's screen (the LCD is at (16, 16) there),
// T frames into the animation
static void Spr(const char *sheet, u16 anim, s32 dx, s32 dy, u8 flip, u32 t)
{
    CellF(sheet, S3D_Gfx_PoketchAnimCell(sheet, anim, t), dx - 16, dy - 16, flip);
}

// A cell turned by ANGLE (65536 a turn, as Platinum's) around its point at (X, Y): the ANALOG WATCH's hands, the
// ROULETTE's arrow
static void CellRot(const char *sheet, u16 cell, s32 x, s32 y, u32 angle)
{
    static const s16 sSin64[65] = {0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 109, 115, 121,
        126, 132, 137, 142, 147, 152, 157, 162, 167, 172, 177, 181, 185, 190, 194, 198, 202, 206, 209, 213, 216, 220,
        223, 226, 229, 231, 234, 237, 239, 241, 243, 245, 247, 248, 250, 251, 252, 253, 254, 255, 255, 256, 256, 256};
    s16 cx, cy;
    u16 w, h;
    const u8 *px;
    s32 a = (angle >> 8) & 255, sn, cs, r, i, j;
    if (!S3D_Gfx_PoketchCell(sheet, cell, &cx, &cy, &w, &h, &px))
        return;
    sn = a < 64 ? sSin64[a] : a < 128 ? sSin64[128 - a] : a < 192 ? -sSin64[a - 128] : -sSin64[256 - a];
    a = (a + 64) & 255;
    cs = a < 64 ? sSin64[a] : a < 128 ? sSin64[128 - a] : a < 192 ? -sSin64[a - 128] : -sSin64[256 - a];
    r = 0;
    for (i = 0; i < 4; i++)
    {
        s32 ox = (i & 1) ? cx + w : cx, oy = (i & 2) ? cy + h : cy, d = ox * ox + oy * oy;
        if (d > r)
            r = d;
    }
    for (r = 1; r * r < (s32)(r * 0) + 0x7FFFFFFF && r * r <= (s32)0x7FFF; r++)
        if (r * r >= (s32)(cx * cx + cy * cy) && r * r >= (s32)((cx + w) * (cx + w) + (cy + h) * (cy + h))
         && r * r >= (s32)((cx + w) * (cx + w) + cy * cy) && r * r >= (s32)(cx * cx + (cy + h) * (cy + h)))
            break;
    for (j = -r; j <= r; j++)
        for (i = -r; i <= r; i++)
        {
            // (the picture's pixel that lands here: turned back by the angle; the DS turns clockwise on screen)
            s32 sx = (i * cs + j * sn) / 256 - cx, sy = (-i * sn + j * cs) / 256 - cy;
            if (sx >= 0 && sx < w && sy >= 0 && sy < h && px[sy * w + sx] != 255 && x + i >= 0 && x + i < PW && y + j >= 0 && y + j < PH)
                sCanvas[y + j][x + i] = px[sy * w + sx];
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
    u32 h, m;
    RtcCalcLocalTime();
    m = gLocalTime.minutes;
    h = (gLocalTime.hours % 12) * 30 + m / 2;
    Screen("analog_watch");
    CellRot("analog_watch", 1, 100, 84, h * 65536 / 360);   // (Platinum's hour hand, 360 places; the minute hand 60)
    CellRot("analog_watch", 0, 100, 84, m * 65536 / 60);
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
        sP.memoTool = sP.sy < PH / 2;   // (Platinum's: the ERASER above, the PENCIL below)
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
    Spr("memo_pad", sP.memoTool == 1 ? 1 : 0, 192, 56, 0, 0);
    Spr("memo_pad", sP.memoTool == 0 ? 3 : 2, 192, 136, 0, 0);
}

static void Pedometer(void)
{
    u32 steps = GetGameStat(GAME_STAT_STEPS) - sP.stepBase;
    bool8 down = sP.touch && TouchIn(66, 80, 64, 64);
    if (sP.touchNew && down)
    {
        sP.stepBase = GetGameStat(GAME_STAT_STEPS);
        PlaySE(SE_SELECT);
    }
    Screen("pedometer");
    Digits(136, 48, steps > 99999 ? 99999 : steps, 5, TRUE);
    Spr("pedometer", down ? 1 : 0, 114, 128, 0, 0);
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
        if (GetMonData(mon, MON_DATA_HELD_ITEM) != ITEM_NONE)   // (Platinum's marks: a held item, or MAIL)
            Spr("party_status", ItemIsMail(GetMonData(mon, MON_DATA_HELD_ITEM)) ? 1 : 0, x + 16 + 28, y + 16 + 28, 0, 0);
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
    bool8 down = sP.touch && TouchIn(66, 80, 64, 64);
    if (sP.touchNew && down)
    {
        sP.counter = sP.counter >= 9999 ? 0 : sP.counter + 1;
        PlaySE(SE_SELECT);
    }
    if (sP.newKeys & SELECT_BUTTON)
        sP.counter = 0;
    Screen("counter");
    Digits(128, 44, sP.counter, 4, TRUE);
    Spr("counter", down ? 1 : 0, 114, 128, 0, 0);
}

static void Coin(void)
{
    if (sP.touchNew && sP.coinFrames == 0 && TouchIn(56, 72, 80, 80))
    {
        sP.coinFrames = 40;
        sP.coinSide = Random() & 1;
        PlaySE(SE_BALL_BOUNCE_1);
    }
    Screen("coin_toss");
    if (sP.coinFrames)
    {
        // (Platinum's spin while it's in the air, up and back down)
        sP.coinFrames--;
        Spr("coin_toss", 0, 112, 144 - (sP.coinFrames > 20 ? 40 - sP.coinFrames : sP.coinFrames) * 3, 0, 40 - sP.coinFrames);
        if (sP.coinFrames == 0)
            PlaySE(SE_BALL_BOUNCE_2);
    }
    else
        Spr("coin_toss", sP.coinSide ? 2 : 1, 112, 144, 0, 0);
}

static void ColorChanger(void)
{
    if (sP.touch && sP.sy >= 112 && sP.sy < 152 && sP.sx >= 32 && sP.sx < 160)
    {
        u8 c = (sP.sx - 32) / 16;
        if (c != sP.color)
            sP.color = c, PlaySE(SE_SELECT);
    }
    Screen("color_changer");
    Spr("color_changer", 0, 56 + sP.color * 16, 148, 0, 0);
}

// The STOPWATCH: Platinum's Voltorb starts and stops it when touched; SELECT (stopped) sets it back to 0
// The STOPWATCH: Platinum's Voltorb starts and stops it when tapped; held, it blinks, faster, and blows up: back to 0
static void Stopwatch(void)
{
    u32 t = sP.swElapsed + (sP.swRunning ? gMain.vblankCounter1 - sP.swStart : 0), cs = t * 100 / 60, anim;
    bool8 on = sP.touch && TouchIn(56, 48, 80, 80);
    if (on)
        sP.swHold++;
    else
    {
        if (sP.swHold > 0 && sP.swHold < 30)
        {
            if (sP.swRunning)
                sP.swElapsed = t, sP.swRunning = FALSE;
            else
                sP.swStart = gMain.vblankCounter1, sP.swRunning = TRUE;
            PlaySE(SE_SELECT);
        }
        if (sP.swHold < 150)
            sP.swHold = 0;
    }
    if (sP.swHold >= 150)   // (the explosion and after)
    {
        if (sP.swHold == 150)
        {
            sP.swElapsed = 0, sP.swRunning = FALSE, t = cs = 0;
            PlaySE(SE_M_EXPLOSION);
        }
        if (++sP.swHold > 150 + 24 + 80)
            sP.swHold = 0;
    }
    anim = sP.swHold >= 150 + 24 ? 16 : sP.swHold >= 150 ? 15 : sP.swHold >= 90 ? 14 : sP.swHold >= 30 ? 13 : sP.swRunning ? 12 : 11;
    Screen("stopwatch");
    if (cs >= 100 * 3600 * 100)
        cs = 100 * 3600 * 100 - 1;
    {
        u32 d[8] = {cs / 360000 / 10 % 10, cs / 360000 % 10, cs / 6000 % 60 / 10, cs / 6000 % 10, cs / 100 % 60 / 10,
                    cs / 100 % 10, cs % 100 / 10, cs % 10}, i;
        static const u8 sX[8] = {32, 48, 80, 96, 128, 144, 176, 192};
        for (i = 0; i < 8; i++)
            Spr("stopwatch", d[i], sX[i], 40, 0, 0);
    }
    Spr("stopwatch", anim, 112, 96, 0, sP.swHold >= 150 ? sP.swHold - 150 : sP.frame);
}

static void Dowsing(void)
{
    const struct MapEvents *events = gMapHeader.events;
    s32 i, px, py;
    if (sP.touchNew)
    {
        // the LCD shows the 16x13 tiles around the player (12 pixels a tile); an item within 2 tiles of the touch shows
        sP.dowseX = sP.sx, sP.dowseY = sP.sy, sP.dowseFrames = 72, sP.dowseFoundX = -128;
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
        Spr("dowsing_machine", 0, sP.dowseX + 16, sP.dowseY + 16, 0, 72 - sP.dowseFrames);
        if (sP.dowseFoundX != -128)
            Spr("dowsing_machine", 1, sP.dowseX + 16 + sP.dowseFoundX * 12, sP.dowseY + 16 + sP.dowseFoundY * 12, 0, sP.frame);
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
    for (i = 0; MP_IsConnected() && i < MP_MAX_PLAYERS && row < 3; i++)
    {
        struct MpPlayer *p = &gMpPlayers[i];
        s32 bars, b;
        if (i == gMpLocalId || !p->present)
            continue;
        MP_GameToAscii(name, p->name, MP_NAME_LEN);
        if (p->state.mapGroup == gSaveBlock1Ptr->location.mapGroup && p->state.mapNum == gSaveBlock1Ptr->location.mapNum)
            bars = 4;
        else if (MP_Region_OfGroup(p->state.mapGroup) == MP_Region_Current())
            bars = 2;
        else
            bars = 1;
        Text(12, 6 + row * 18, name, 3, 2);
        for (b = 0; b < 4; b++)
            Fill(150 + b * 8, 16 + row * 18 - b * 3, 6, 3 + b * 3, b < bars ? 3 : 1);
        row++;
    }
    // Platinum's DS and its waves (searching: a friend in the room), or its "no one" mark
    Spr("link_searcher", 3, 112, 150, 0, 0);
    Spr("link_searcher", row ? 1 : 2, 112, 102, 0, sP.frame);
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
            // (Platinum's hearts over it: more, and livelier, the closer it is)
            Spr("friendship_checker", f >= 200 ? 2 : f >= 150 ? 1 : f >= 70 ? 0 : 3, x + 32, y + 12, 0, sP.frame);
            if (sP.touchNew)
                PlayCry_Normal(species, 0);
        }
    }
}

// DAY-CARE CHECKER: the two POKéMON at the DAY CARE, their levels now, and an EGG when there is one
// DAY-CARE CHECKER: the two POKéMON at the DAY CARE (their levels with the steps walked, Platinum's digits and gender
// marks), and the EGG when there is one
static void Daycare(void)
{
    u8 i, k;
    Screen("daycare_checker");
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
    {
        struct BoxPokemon box = gSaveBlock1Ptr->daycare.mons[i].mon;
        u16 species = GetBoxMonData(&box, MON_DATA_SPECIES);
        u32 exp, level, g;
        if (species == SPECIES_NONE)
            continue;
        exp = GetBoxMonData(&box, MON_DATA_EXP) + gSaveBlock1Ptr->daycare.mons[i].steps;
        SetBoxMonData(&box, MON_DATA_EXP, &exp);
        level = GetLevelFromBoxMonExp(&box);
        for (k = 0; k < 3; k++)
            if (k == 2 || level >= (k == 0 ? 100u : 10u))
                Spr("daycare_checker", k == 0 ? level / 100 : k == 1 ? level / 10 % 10 : level % 10, 48 + i * 104 + k * 16, 40, 0, 0);
        g = GetGenderFromSpeciesAndPersonality(species, GetBoxMonData(&box, MON_DATA_PERSONALITY));
        Spr("daycare_checker", g == MON_MALE ? 10 : g == MON_FEMALE ? 11 : 12, 96 + i * 104, 40, 0, 0);
        MonIcon(species, GetBoxMonData(&box, MON_DATA_PERSONALITY), 40 + i * 112 - 16, 112 - 16 + ((sP.frame / 16 + i) & 1));
    }
    if (gSaveBlock1Ptr->daycare.offspringPersonality)
        MonIcon(SPECIES_EGG, 0, 96 - 16, 120 - 16);
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

// TRAINER COUNTER: the POKé RADAR's chain now (its POKéMON and count, at the top) and the best three (src/mp/mp_pokeradar.c);
// a touch on one plays its cry (Platinum's places: icons at (96, 32), (112, 80), (176, 96), (48, 104) and their
// 8-pixel digits at (144, 40), (100, 144), (164, 160), (36, 168) on the DS's screen)
static void SmallDigits(s32 dx, s32 dy, u32 value)
{
    u32 i;
    for (i = 0; i < 3; i++)
    {
        u32 d = (value / (i == 0 ? 100 : i == 1 ? 10 : 1)) % 10;
        Spr("trainer_counter", d, dx + 8 * i, dy, 0, 0);
    }
}

static void TrainerCounter(void)
{
    static const s16 sIcon[4][2] = {{96, 32}, {112, 80}, {176, 96}, {48, 104}};
    static const s16 sDigits[4][2] = {{144, 40}, {100, 144}, {164, 160}, {36, 168}};
    u16 species[4], chain[4];
    u32 i;
    chain[0] = MP_PokeRadar_Chain(&species[0]);
    for (i = 0; i < 3; i++)
        species[i + 1] = sP.radarSpecies[i], chain[i + 1] = sP.radarChain[i];
    Screen("trainer_counter");
    for (i = 0; i < 4; i++)
    {
        s32 x = sIcon[i][0] - 16 - 16, y = sIcon[i][1] - 16 - 16;
        if (chain[i] == 0 || species[i] == SPECIES_NONE)
            continue;
        MonIcon(species[i], 0, x, y + (i == 0 && ((sP.frame / 16) & 1)));
        SmallDigits(sDigits[i][0], sDigits[i][1], chain[i] > 999 ? 999 : chain[i]);
        if (sP.touchNew && TouchIn(x, y, 32, 32))
            PlayCry_Normal(species[i], 0);
    }
}

// A chain grew or ended (mp_pokeradar.c): the best three are kept (one a POKéMON: its best)
void MP_Poketch_RadarChain(u16 species, u16 chain, bool8 ended)
{
    u32 i, low = 0;
    (void)ended;
    LoadState();
    for (i = 0; i < 3; i++)
        if (sP.radarSpecies[i] == species && sP.radarChain[i] > 0)
        {
            if (chain > sP.radarChain[i])
                sP.radarChain[i] = chain, SaveState();
            return;
        }
    for (i = 1; i < 3; i++)
        if (sP.radarChain[i] < sP.radarChain[low])
            low = i;
    if (chain > sP.radarChain[low])
    {
        sP.radarSpecies[low] = species, sP.radarChain[low] = chain;
        SaveState();
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
    if (px >= 0)
        Spr("map", 18, 12 + px * 6 + 3 + 16, 35 + py * 6 + 3 + 16, 0, sP.frame);
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
            Spr("map", 18, 10 + 6 * cx + 3 + 16, 8 + 6 * (cz - 5) + 3 + 16, 0, sP.frame);
    }
    else
        RegionGridMap();
    for (i = 0; i < 6; i++)
        Spr("map", 1 + i, sP.markX[i] + 16, sP.markY[i] + 16, 0, 0);
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

// MOVE TESTER: Platinum's screen: the move's type below (its arrows change it), the target's one or two types above,
// and how well it works in the bubble
static void MoveTester(void)
{
    static const u8 sArrowX[3][2] = {{28, 116}, {108, 196}, {108, 196}}, sArrowY[3] = {128, 40, 72};
    char buf[16];
    u32 mult, k, n = TypeCount();
    for (k = 0; k < 3; k++)
    {
        u8 side;
        for (side = 0; side < 2; side++)
            if (sP.touchNew && TouchIn(sArrowX[k][side] - 16 - 12, sArrowY[k] - 16 - 16, 24, 32))
            {
                s32 d = side ? 1 : -1, v = sP.movesType[k] == 0xFF ? (s32)n : sP.movesType[k];
                do
                    v = (v + d + n + (k == 2)) % (n + (k == 2));
                while (v == TYPE_MYSTERY);
                sP.movesType[k] = v == (s32)n ? 0xFF : v;
                sP.movesPressed = k * 2 + side, sP.movesPressTime = 8;
                PlaySE(SE_SELECT);
            }
    }
    if (sP.movesPressTime)
        sP.movesPressTime--;
    Screen("move_tester");
    for (k = 0; k < 3; k++)
    {
        s32 cx = (sArrowX[k][0] + sArrowX[k][1]) / 2 - 16;
        if (sP.movesType[k] == 0xFF)
            snprintf(buf, sizeof(buf), "-");
        else
            MP_GameToAscii(buf, gTypeNames[sP.movesType[k]], TYPE_NAME_LENGTH);
        Text(cx - (s32)strlen(buf) * 4, sArrowY[k] - 16 - 5, buf, 3, 2);
        Spr("move_tester", sP.movesPressTime && sP.movesPressed == k * 2 ? 1 : 0, sArrowX[k][0], sArrowY[k], 0, 0);
        Spr("move_tester", sP.movesPressTime && sP.movesPressed == k * 2 + 1 ? 3 : 2, sArrowX[k][1], sArrowY[k], 0, 0);
    }
    mult = TypeMult(sP.movesType[0], sP.movesType[1]);
    if (sP.movesType[2] != 0xFF && sP.movesType[2] != sP.movesType[1])
        mult = mult * TypeMult(sP.movesType[0], sP.movesType[2]) / 10;
    // (in the bubble: Platinum's marks, two for super effective, one for normal, none for less; and the number)
    if (mult > 10)
        Spr("move_tester", 5, 44, 48, 0, 0);
    else if (mult == 10)
        Spr("move_tester", 4, 44, 48, 0, 0);
    if (mult == 0)
        snprintf(buf, sizeof(buf), "X0");
    else if (mult < 10)
        snprintf(buf, sizeof(buf), mult == 5 ? "X.5" : "X.25");
    else
        snprintf(buf, sizeof(buf), "X%d", (int)(mult / 10));
    Text(30, 24, buf, 3, 2);
}

// MATCHUP CHECKER: two of the party (touch a side to change it) and how well they'd get along at the DAY CARE
// MATCHUP CHECKER: two of the party (tap one to change it); the button asks, and Platinum's heart meter answers
static void Matchup(void)
{
    u8 n = CalculatePlayerPartyCount(), k;
    if (n == 0)
    {
        Screen("matchup_checker");
        return;
    }
    if (sP.touchNew && TouchIn(8, 100, 64, 48))
        sP.matchA = (sP.matchA + 1) % n, sP.matchScore = 0xFF, PlaySE(SE_SELECT);
    if (sP.touchNew && TouchIn(120, 100, 64, 48))
        sP.matchB = (sP.matchB + 1) % n, sP.matchScore = 0xFF, PlaySE(SE_SELECT);
    if (sP.touchNew && TouchIn(64, 116, 64, 32))
    {
        sP.matchA %= n, sP.matchB %= n;
        sP.matchScore = sP.matchA == sP.matchB ? 0 : MP_Daycare_Compatibility(&gPlayerParty[sP.matchA].box, &gPlayerParty[sP.matchB].box);
        sP.matchTime = 0;
        PlaySE(SE_SELECT);
    }
    sP.matchA %= n, sP.matchB %= n;
    sP.matchTime++;
    Screen("matchup_checker");
    for (k = 0; k < 2; k++)
        MonIcon(GetMonData(&gPlayerParty[k ? sP.matchB : sP.matchA], MON_DATA_SPECIES_OR_EGG),
                GetMonData(&gPlayerParty[k ? sP.matchB : sP.matchA], MON_DATA_PERSONALITY), (k ? 176 : 48) - 32, 140 - 32);
    Spr("matchup_checker", sP.touch && TouchIn(64, 116, 64, 32) ? 10 : 9, 112, 148, 0, 0);
    if (sP.matchScore == 0xFF)
    {
        Spr("matchup_checker", 0, 112, 32, 0, 0);
        Spr("matchup_checker", 5, 48, 88, 0, 0);
        Spr("matchup_checker", 6, 176, 88, 0, 0);
    }
    else
    {
        u8 hearts = sP.matchScore >= 70 ? 3 : sP.matchScore >= 50 ? 2 : sP.matchScore >= 20 ? 1 : 0;
        Spr("matchup_checker", hearts, 112, 32, 0, sP.matchTime);
        // (the LUVDISC swim to each other the better they get along)
        Spr("matchup_checker", hearts >= 2 ? 7 : 5, 48 + (hearts >= 2 ? (sP.matchTime < 40 ? sP.matchTime : 40) / 2 : 0), 88, 0, sP.matchTime);
        Spr("matchup_checker", hearts >= 2 ? 8 : 6, 176 - (hearts >= 2 ? (sP.matchTime < 40 ? sP.matchTime : 40) / 2 : 0), 88, 0, sP.matchTime);
    }
}

// ROULETTE: draw on the wheel, then touch the button to spin it
// ROULETTE: draw on the wheel; Platinum's PLAY spins its arrow, STOP slows it to a stop, CLEAR wipes the drawing
static void Roulette(void)
{
    s32 x, y, r = 1;
    if (sP.touchNew && TouchIn(155, 18, 32, 32))
        sP.roulSpeed = 1200 + (Random() % 400), sP.roulStopping = FALSE, PlaySE(SE_SELECT);
    else if (sP.touchNew && TouchIn(155, 64, 32, 32))
        sP.roulStopping = TRUE, PlaySE(SE_SELECT);
    else if (sP.touchNew && TouchIn(155, 110, 32, 32))
        memset(sP.memo, 0, sizeof(sP.memo)), PlaySE(SE_SELECT);
    else if (sP.touch && sP.sx < 150)
        for (y = sP.sy - r; y <= sP.sy + r; y++)
            for (x = sP.sx - r; x <= sP.sx + r; x++)
                if (x >= 0 && x < MEMO_W && y >= 0 && y < PH)
                    sP.memo[(y * MEMO_W + x) / 8] |= 1 << ((y * MEMO_W + x) % 8);
    if (sP.roulSpeed > 0)
    {
        sP.roulAngle = (sP.roulAngle + sP.roulSpeed) & 0xFFFF;
        if (sP.roulStopping)
        {
            sP.roulSpeed -= 12;
            if (sP.roulSpeed <= 0)
                sP.roulSpeed = 0, PlaySE(SE_DING_DONG);
        }
    }
    Screen("roulette");
    for (y = 0; y < PH; y++)
        for (x = 0; x < 150; x++)
            if (sP.memo[(y * MEMO_W + x) / 8] & (1 << ((y * MEMO_W + x) % 8)))
                sCanvas[y][x] = 3;
    CellRot("roulette", 0, 80, 80, sP.roulAngle);
    Spr("roulette", sP.touch && TouchIn(155, 18, 32, 32) ? 2 : 1, 187, 50, 0, 0);
    Spr("roulette", sP.touch && TouchIn(155, 64, 32, 32) ? 4 : 3, 187, 96, 0, 0);
    Spr("roulette", sP.touch && TouchIn(155, 110, 32, 32) ? 6 : 5, 187, 142, 0, 0);
}

// KITCHEN TIMER: set the minutes and seconds with the arrows, then start; Snorlax wakes when it's up
// KITCHEN TIMER: Platinum's arrows set each digit, START / STOP / RESET; Snorlax's arms go up when it rings
static void Timer(void)
{
    static const u8 sX[4] = {80, 96, 128, 144};
    u32 left = sP.timerRunning ? (sP.timerEnd > gMain.vblankCounter1 ? sP.timerEnd - gMain.vblankCounter1 : 0) : sP.timerLeft;
    u32 secs, d[4], k;
    if (sP.timerRunning && left == 0)
        sP.timerRunning = FALSE, sP.timerLeft = 0, sP.ringing = 180;
    secs = (left + 59) / 60;
    d[0] = secs / 600 % 10, d[1] = secs / 60 % 10, d[2] = secs % 60 / 10, d[3] = secs % 10;
    if (sP.touchNew)
    {
        if (TouchIn(0, 128, 64, 32) && !sP.timerRunning && sP.timerLeft)
            sP.timerEnd = gMain.vblankCounter1 + sP.timerLeft, sP.timerRunning = TRUE, sP.ringing = 0, PlaySE(SE_SELECT);
        else if (TouchIn(64, 128, 64, 32) && sP.timerRunning)
            sP.timerLeft = left, sP.timerRunning = FALSE, PlaySE(SE_SELECT);
        else if (TouchIn(128, 128, 64, 32) && !sP.timerRunning)
            sP.timerLeft = 0, sP.ringing = 0, PlaySE(SE_SELECT);
        else if (!sP.timerRunning)
            for (k = 0; k < 4; k++)
            {
                s32 up = TouchIn(sX[k] - 16 - 8, 88 - 16 - 10, 16, 20), down = TouchIn(sX[k] - 16 - 8, 136 - 16 - 10, 16, 20);
                if (up || down)
                {
                    static const u32 sStep[4] = {600, 60, 10, 1}, sMax[4] = {9, 9, 5, 9};
                    s32 v = (s32)d[k] + (up ? 1 : -1);
                    v = v < 0 ? (s32)sMax[k] : v > (s32)sMax[k] ? 0 : v;
                    secs = secs + (v - (s32)d[k]) * (s32)sStep[k];
                    sP.timerLeft = secs * 60;
                    d[k] = v;
                    PlaySE(SE_SELECT);
                }
            }
    }
    Screen("kitchen_timer");
    for (k = 0; k < 4; k++)
    {
        Spr("digits", d[k], sX[k], 112, 0, 0);
        if (!sP.timerRunning)
        {
            Spr("kitchen_timer", 4, sX[k], 88, 0, sP.frame);
            Spr("kitchen_timer", 4, sX[k], 136, 2, sP.frame);
        }
    }
    {
        bool8 up = sP.ringing && (sP.ringing / 12) & 1;
        Spr("kitchen_timer", up ? 2 : 3, 48, 56, 0, 0);
        Spr("kitchen_timer", up ? 0 : 1, 176, 56, 0, 0);
    }
    Spr("kitchen_timer", sP.timerRunning || (sP.touch && TouchIn(0, 128, 64, 32)) ? 6 : 5, 48, 160, 0, 0);
    Spr("kitchen_timer", sP.touch && TouchIn(64, 128, 64, 32) ? 8 : 7, 112, 160, 0, 0);
    Spr("kitchen_timer", sP.touch && TouchIn(128, 128, 64, 32) ? 10 : 9, 176, 160, 0, 0);
}

// ALARM CLOCK: the time it rings (touch the upper / lower half of the hours or minutes) and the switch on the right
// ALARM CLOCK: Platinum's SET switch on the right; set, it shows the alarm's time with arrows to change it (otherwise
// the time now); ringing, the ears wave, the eyes open and the digits blink
static void Alarm(void)
{
    u32 h, m;
    bool8 ring = sP.ringing != 0;
    if (sP.touchNew)
    {
        if (TouchIn(160, 56, 32, 64))
            sP.alarmOn ^= 1, sP.ringing = 0, PlaySE(SE_SELECT);
        else if (sP.alarmOn && TouchIn(40, 88, 32, 32))
            sP.alarmH = (sP.alarmH + 1) % 24, PlaySE(SE_SELECT);
        else if (sP.alarmOn && TouchIn(40, 132, 32, 28))
            sP.alarmH = (sP.alarmH + 23) % 24, PlaySE(SE_SELECT);
        else if (sP.alarmOn && TouchIn(88, 88, 32, 32))
            sP.alarmM = (sP.alarmM + 1) % 60, PlaySE(SE_SELECT);
        else if (sP.alarmOn && TouchIn(88, 132, 32, 28))
            sP.alarmM = (sP.alarmM + 59) % 60, PlaySE(SE_SELECT);
    }
    RtcCalcLocalTime();
    h = sP.alarmOn ? sP.alarmH : gLocalTime.hours;
    m = sP.alarmOn ? sP.alarmM : gLocalTime.minutes;
    Screen("alarm_clock");
    Spr("alarm_clock", sP.alarmOn ? 2 : 3, 192, 104, 0, 0);
    Spr("alarm_clock", ring ? 1 : 0, 48, 48, 0, sP.frame);
    Spr("alarm_clock", ring ? 1 : 0, 144, 48, 1, sP.frame);
    Spr("alarm_clock", sP.alarmOn || ring ? 4 : 5, 56, 80, 0, 0);
    Spr("alarm_clock", sP.alarmOn || ring ? 4 : 5, 136, 80, 1, 0);
    if (sP.alarmOn && !ring)
    {
        Spr("alarm_clock", 6, 72, 120, 0, sP.frame);
        Spr("alarm_clock", 6, 72, 164, 2, sP.frame);
        Spr("alarm_clock", 6, 120, 120, 0, sP.frame);
        Spr("alarm_clock", 6, 120, 164, 2, sP.frame);
    }
    if (!ring || (sP.frame / 15) & 1)
    {
        Spr("digits", h / 10, 64, 144, 0, 0);
        Spr("digits", h % 10, 80, 144, 0, 0);
        Spr("digits", m / 10, 112, 144, 0, 0);
        Spr("digits", m % 10, 128, 144, 0, 0);
    }
}

// The KITCHEN TIMER and the ALARM ring wherever the POKéTCH is (it opens on the one ringing)
static void CheckRinging(void)
{
    static u8 sLastMinute = 0xFF;
    if (sP.timerRunning && gMain.vblankCounter1 >= sP.timerEnd)
    {
        sP.timerRunning = FALSE, sP.timerLeft = 0, sP.ringing = 180;
        if (sP.view == VIEW_HIDDEN && MP_IsLocalFree())
            sP.view = VIEW_SMALL, sP.app = APP_TIMER;
    }
    if (sP.alarmOn && (sP.frame % 60) == 0)
    {
        RtcCalcLocalTime();
        if (gLocalTime.hours == sP.alarmH && gLocalTime.minutes == sP.alarmM && sLastMinute != gLocalTime.minutes)
        {
            sP.ringing = 180;
            if (sP.view == VIEW_HIDDEN && MP_IsLocalFree())
                sP.view = VIEW_SMALL, sP.app = APP_ALARM;
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
    if (sP.touchNew && TouchIn(56, 56, 80, 64))
    {
        sP.backlight ^= 1;
        PlaySE(SE_SELECT);
    }
    Screen("backlight_toggle");
    Spr("backlight_toggle", sP.backlight ? 1 : 0, 112, 104, 0, 0);
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
    case APP_RADAR: TrainerCounter(); break;
    }
}

// ------------------------------------------------------------------------------------------------- open / input
// The UI elements an app's cursor goes between (no list: a free cursor, the MEMO PAD's kind)
struct Target { u8 x, y, w, h; };

static u8 Targets(struct Target *t)
{
    u8 n = 0, i, r, c;
    switch (sP.app)
    {
    case APP_CALC:
        for (r = 0; r < 4; r++)
            for (c = 0; c < 5; c++)
                if (!(c == 4 && sCalcKeys[r][3] == sCalcKeys[r][4]) && !(c == 1 && r == 3))
                    t[n++] = (struct Target){16 + c * 32, 32 + r * 32,
                                             (c == 3 && sCalcKeys[r][3] == sCalcKeys[r][4]) || (c == 0 && r == 3) ? 62 : 30, 30};
        break;
    case APP_PEDOMETER: t[n++] = (struct Target){66, 80, 64, 64}; break;
    case APP_COUNTER: t[n++] = (struct Target){66, 80, 64, 64}; break;
    case APP_COIN: t[n++] = (struct Target){56, 96, 80, 56}; break;
    case APP_STOPWATCH: t[n++] = (struct Target){56, 48, 80, 80}; break;
    case APP_BACKLIGHT: t[n++] = (struct Target){56, 56, 80, 64}; break;
    case APP_COLOR:
        for (i = 0; i < 8; i++)
            t[n++] = (struct Target){32 + i * 16, 116, 16, 32};
        break;
    case APP_PARTY:
        for (i = 0; i < CalculatePlayerPartyCount(); i++)
            t[n++] = (struct Target){8 + (i % 2) * 96, 6 + (i / 2) * 52, 88, 46};
        break;
    case APP_FRIENDSHIP:
        for (i = 0; i < CalculatePlayerPartyCount(); i++)
            t[n++] = (struct Target){16 + (i % 3) * 56, 24 + (i / 3) * 64, 32, 32};
        break;
    case APP_HISTORY:
        for (i = 0; i < ARRAY_COUNT(sP.histSpecies); i++)
            if (sP.histSpecies[i] != SPECIES_NONE)
                t[n++] = (struct Target){12 + (i % 4) * 44, 8 + (i / 4) * 50, 32, 32};
        break;
    case APP_MOVES:
    {
        static const u8 sArrowX[3][2] = {{28, 116}, {108, 196}, {108, 196}}, sArrowY[3] = {128, 40, 72};
        for (r = 0; r < 3; r++)
            for (c = 0; c < 2; c++)
                t[n++] = (struct Target){sArrowX[r][c] - 28, sArrowY[r] - 32, 24, 32};
        break;
    }
    case APP_MATCHUP:
        t[n++] = (struct Target){8, 100, 64, 48};
        t[n++] = (struct Target){64, 116, 64, 32};
        t[n++] = (struct Target){120, 100, 64, 48};
        break;
    case APP_TIMER:
    {
        static const u8 sX[4] = {80, 96, 128, 144};
        if (!sP.timerRunning)
            for (i = 0; i < 4; i++)
            {
                t[n++] = (struct Target){sX[i] - 24, 62, 16, 20};
                t[n++] = (struct Target){sX[i] - 24, 110, 16, 20};
            }
        for (i = 0; i < 3; i++)
            t[n++] = (struct Target){i * 64, 128, 64, 32};
        break;
    }
    case APP_ALARM:
        if (sP.alarmOn)
        {
            t[n++] = (struct Target){40, 88, 32, 32};
            t[n++] = (struct Target){40, 132, 32, 28};
            t[n++] = (struct Target){88, 88, 32, 32};
            t[n++] = (struct Target){88, 132, 32, 28};
        }
        t[n++] = (struct Target){160, 56, 32, 64};
        break;
    }
    return n;
}

// The locked cursor to the next element in direction (DX, DY): the nearest whose middle lies that way
static void MoveTarget(const struct Target *t, u8 n, s32 dx, s32 dy)
{
    s32 cx = t[sP.target].x + t[sP.target].w / 2, cy = t[sP.target].y + t[sP.target].h / 2, best = 0x7FFFFFFF, i;
    for (i = 0; i < n; i++)
    {
        s32 ox = t[i].x + t[i].w / 2 - cx, oy = t[i].y + t[i].h / 2 - cy, along = ox * dx + oy * dy, side, d;
        if (i == sP.target || along <= 0)
            continue;
        side = dx ? oy : ox;
        d = along + (side < 0 ? -side : side) * 3;
        if (d < best)
            best = d, sP.target = i;
    }
}

// Shown on the field while the player is free to move (a script, a menu or another screen hides it for the while)
static bool8 Available(void)
{
    return gMain.callback2 == CB2_Overworld && !gPaletteFade.active && MP_IsLocalFree();
}

static bool8 Shown(void)
{
    return sP.loaded && sP.view != VIEW_HIDDEN && Available();
}

bool8 MP_Poketch_IsOpen(void)
{
    return Shown();
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
    sP.target = 0;
    PlaySE(SE_DEX_PAGE);
    printf("[test] poketch: app %d\n", sP.app);
}

// Using the item (the BAG, a registered button): large, with the cursor
void MP_Poketch_Open(void)
{
    LoadState();
    if (!AppUnlocked(sP.app))
        sP.app = APP_WATCH;
    sP.view = VIEW_LARGE;
    sP.cursor = TRUE;
    PlaySE(SE_POKENAV_ON);
    printf("[test] poketch: open, app %d\n", sP.app);
}

void MP_Poketch_Close(void)
{
    if (sP.view == VIEW_HIDDEN)
        return;
    sP.view = VIEW_HIDDEN;
    sP.cursor = FALSE;
    SaveState();
    PlaySE(SE_POKENAV_OFF);
}

// The right stick (gamepad.c; I / K / J / L on the keyboard): DIR 0 up (shows it small, then large), 1 down (small,
// then hidden), 2 / 3 the app before / after. Only when the player could have it out (on the field, free to move).
void MP_Poketch_Stick(u8 dir)
{
    if (!Available() || !CheckBagHasItem(ITEM_POKETCH, 1))
        return;
    LoadState();
    if (!AppUnlocked(sP.app))
        sP.app = APP_WATCH;
    if (dir == 0 && sP.view < VIEW_LARGE)
    {
        sP.view++;
        PlaySE(sP.view == VIEW_SMALL ? SE_POKENAV_ON : SE_SELECT);
    }
    else if (dir == 1 && sP.view > VIEW_HIDDEN)
    {
        if (sP.view == VIEW_SMALL)
            MP_Poketch_Close();
        else
            sP.view--, PlaySE(SE_SELECT);
    }
    else if (dir >= 2 && sP.view != VIEW_HIDDEN)
        ChangeApp(dir == 3 ? 1 : -1);
    printf("[test] poketch: view %d app %d cursor %d\n", sP.view, sP.app, sP.cursor);
}

// The right stick click (O on the keyboard) while it shows: the cursor mode on / off. FALSE: hidden (the click is the
// FRIEND FINDER's)
bool8 MP_Poketch_StickClick(void)
{
    if (!Shown())
        return FALSE;
    sP.cursor = !sP.cursor;
    sP.target = 0;
    sP.touch = FALSE;
    PlaySE(sP.cursor ? SE_PIN : SE_BALL_TRAY_EXIT);
    printf("[test] poketch: cursor %d\n", sP.cursor);
    return TRUE;
}

// Where the LCD is on the screen in each view and its scale (Q8)
static void LcdRect(s32 *x, s32 *y, s32 *scale)
{
    if (sP.view == VIEW_LARGE)
        *x = 31, *y = 10, *scale = 224;    // (192x160 at 7/8: 168x140, in a 194x156 case)
    else
        *x = 158, *y = 93, *scale = 96;    // (3/8: 72x60, bottom right)
}

void MP_Poketch_FilterKeys(void)
{
    s32 mx, my, lx, ly, scale;
    bool8 down, wasTouch, mouseMoved;
    struct Target t[24];
    u8 n;
    if (sP.loaded && gMain.callback2 == CB2_Overworld)
        CheckRinging();
    if (!Shown())
    {
        if (sP.cursor && sP.loaded && gMain.callback2 == CB2_Overworld && !MP_IsLocalFree())
            sP.cursor = FALSE;   // (a script began: the controls are the game's again)
        return;
    }
    sP.frame++;
    wasTouch = sP.touch;
    sP.touch = FALSE;
    sP.newKeys = sP.heldKeys = sP.repKeys = 0;
    n = Targets(t);
    if (sP.target >= n)
        sP.target = 0;
    if (sP.cursor)
    {
        // it takes the D-pad (the left stick), A, B (leaves the cursor mode), L / R and SELECT
        u16 taken = DPAD_ANY | A_BUTTON | B_BUTTON | L_BUTTON | R_BUTTON | SELECT_BUTTON;
        sP.newKeys = gMain.newKeys & taken, sP.heldKeys = gMain.heldKeys & taken, sP.repKeys = gMain.newAndRepeatedKeys & taken;
        gMain.newKeys &= ~taken, gMain.heldKeys &= ~taken, gMain.newAndRepeatedKeys &= ~taken;
        if (sP.newKeys & B_BUTTON)
        {
            sP.cursor = FALSE;
            PlaySE(SE_BALL_TRAY_EXIT);
            return;
        }
        if (sP.newKeys & (L_BUTTON | R_BUTTON))
            ChangeApp(sP.newKeys & R_BUTTON ? 1 : -1), n = Targets(t);
        if (n && !sP.testTap)
        {
            // apps with buttons: the cursor goes from one to the next
            if (sP.repKeys & DPAD_LEFT) MoveTarget(t, n, -1, 0);
            if (sP.repKeys & DPAD_RIGHT) MoveTarget(t, n, 1, 0);
            if (sP.repKeys & DPAD_UP) MoveTarget(t, n, 0, -1);
            if (sP.repKeys & DPAD_DOWN) MoveTarget(t, n, 0, 1);
            sP.sx = t[sP.target].x + t[sP.target].w / 2, sP.sy = t[sP.target].y + t[sP.target].h / 2;
        }
        else if (sP.heldKeys & DPAD_ANY)
        {
            s32 v = sP.cursorTimer > 40 ? 4 : 2;
            if (sP.heldKeys & DPAD_LEFT) sP.sx -= v;
            if (sP.heldKeys & DPAD_RIGHT) sP.sx += v;
            if (sP.heldKeys & DPAD_UP) sP.sy -= v;
            if (sP.heldKeys & DPAD_DOWN) sP.sy += v;
            sP.sx = sP.sx < 0 ? 0 : sP.sx >= PW ? PW - 1 : sP.sx;
            sP.sy = sP.sy < 0 ? 0 : sP.sy >= PH ? PH - 1 : sP.sy;
            if (sP.cursorTimer < 250)
                sP.cursorTimer += 2;
        }
        else
            sP.cursorTimer = 0;
        sP.touch = (sP.heldKeys & A_BUTTON) != 0;
    }
    if (sP.testTap)
        sP.touch = --sP.testTap > 0;
    // the mouse: on the large one it touches where it points; a click on the small one makes it large
    mouseMoved = Platform_PoketchMouse(&mx, &my, &down);
    LcdRect(&lx, &ly, &scale);
    if ((mouseMoved || down) && mx >= lx && my >= ly && mx < lx + PW * scale / 256 && my < ly + PH * scale / 256)
    {
        if (sP.view == VIEW_SMALL)
        {
            if (down && !sP.mouseWas)
                sP.view = VIEW_LARGE, PlaySE(SE_SELECT);
        }
        else
        {
            sP.sx = (mx - lx) * 256 / scale, sP.sy = (my - ly) * 256 / scale;
            sP.touch = sP.touch || down;
        }
    }
    else if (down && !sP.mouseWas && sP.view == VIEW_LARGE && mx >= lx + PW * scale / 256 && mx < lx + PW * scale / 256 + 18
             && my >= ly + 30 && my < ly + 110)
        ChangeApp(1);   // (the side button)
    sP.mouseWas = down;
    sP.touchNew = sP.touch && !wasTouch;
}

// ------------------------------------------------------------------------------------------------- the picture
static u16 Blend(u16 a, u16 b, u32 alpha)   // (alpha of 16: a)
{
    u32 r = ((a & 31) * alpha + (b & 31) * (16 - alpha)) / 16, g = ((a >> 5 & 31) * alpha + (b >> 5 & 31) * (16 - alpha)) / 16;
    u32 bl = ((a >> 10 & 31) * alpha + (b >> 10 & 31) * (16 - alpha)) / 16;
    return r | g << 5 | bl << 10 | 0x8000;
}

static void Put(u16 *frame, s32 x, s32 y, u16 c, u32 alpha)
{
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT)
        frame[y * DISPLAY_WIDTH + x] = alpha >= 16 ? (c | 0x8000) : Blend(c, frame[y * DISPLAY_WIDTH + x], alpha);
}

void MP_Poketch_Composite(u16 *frame)
{
    s32 x, y, lx, ly, scale, lw, lh, cx0, cy0, cx1, cy1, side, n;
    u32 alpha, lcdR[4], lcdG[4], lcdB[4];
    struct Target t[24];
    if (!Shown())
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
        lcdR[x] = r, lcdG[x] = g, lcdB[x] = b;
    }
    LcdRect(&lx, &ly, &scale);
    lw = PW * scale / 256, lh = PH * scale / 256;
    alpha = sP.view == VIEW_LARGE ? 16 : 12;   // (the small one lets the field show through a little)
    side = sP.view == VIEW_LARGE ? 18 : 7;
    // the case: Platinum's red around the LCD, its side button on the right
    cx0 = lx - (sP.view == VIEW_LARGE ? 8 : 3), cy0 = ly - (sP.view == VIEW_LARGE ? 8 : 3);
    cx1 = lx + lw + side, cy1 = ly + lh + (sP.view == VIEW_LARGE ? 8 : 3);
    for (y = cy0; y < cy1; y++)
        for (x = cx0; x < cx1; x++)
        {
            bool8 corner = (x - cx0 < 3 || cx1 - 1 - x < 3) && (y - cy0 < 3 || cy1 - 1 - y < 3)
                        && (x - cx0) + (y - cy0) < 3 + (cx1 - 1 - x) * 0;
            bool8 edge = x == cx0 || x == cx1 - 1 || y == cy0 || y == cy1 - 1;
            s32 bx0 = lx + lw + 3, bx1 = cx1 - 3, by0 = ly + lh / 5, by1 = ly + lh * 4 / 5;
            u16 c = edge ? RGB(14, 4, 6) : RGB(29, 10, 11);
            if (corner && ((x == cx0 || x == cx1 - 1) && (y == cy0 || y == cy1 - 1)))
                continue;
            if (x >= bx0 && x < bx1 && y >= by0 && y < by1)
                c = (y == by0 || y == by1 - 1 || x == bx0 || x == bx1 - 1) ? RGB(16, 4, 6)
                  : (x < bx0 + 3 ? RGB(31, 20, 20) : RGB(31, 14, 15));
            if (x >= lx && x < lx + lw && y >= ly && y < ly + lh)
                continue;
            Put(frame, x, y, c, alpha);
        }
    // the LCD, scaled smoothly (each pixel the average of the LCD pixels it covers)
    n = (256 + scale - 1) / scale;   // samples a side
    for (y = 0; y < lh; y++)
        for (x = 0; x < lw; x++)
        {
            u32 r = 0, g = 0, b = 0, k = 0, i, j;
            for (j = 0; j < (u32)n; j++)
                for (i = 0; i < (u32)n; i++)
                {
                    s32 sx = ((x * 256 + (i * 256 + 128) / n) * 256 / scale) >> 8, sy = ((y * 256 + (j * 256 + 128) / n) * 256 / scale) >> 8;
                    u8 v = sCanvas[sy < PH ? sy : PH - 1][sx < PW ? sx : PW - 1];
                    r += lcdR[v], g += lcdG[v], b += lcdB[v], k++;
                }
            Put(frame, lx + x, ly + y, RGB(r / k >> 3, g / k >> 3, b / k >> 3), alpha);
        }
    if (!sP.cursor)
        return;
    // the cursor: a white and black frame on the element it's locked to, or a pointer (both blink a little)
    n = Targets(t);
    if (n && sP.target < n)
    {
        s32 x0 = lx + t[sP.target].x * scale / 256 - 1, y0 = ly + t[sP.target].y * scale / 256 - 1;
        s32 x1 = lx + (t[sP.target].x + t[sP.target].w) * scale / 256, y1 = ly + (t[sP.target].y + t[sP.target].h) * scale / 256;
        u16 c = (sP.frame / 12) & 1 ? RGB(31, 31, 31) : RGB(31, 22, 4);
        for (x = x0; x <= x1; x++)
            Put(frame, x, y0, c, 16), Put(frame, x, y1, c, 16), Put(frame, x, y0 - 1, RGB(2, 2, 2), 16), Put(frame, x, y1 + 1, RGB(2, 2, 2), 16);
        for (y = y0; y <= y1; y++)
            Put(frame, x0, y, c, 16), Put(frame, x1, y, c, 16), Put(frame, x0 - 1, y, RGB(2, 2, 2), 16), Put(frame, x1 + 1, y, RGB(2, 2, 2), 16);
    }
    else
    {
        s32 px = lx + sP.sx * scale / 256, py = ly + sP.sy * scale / 256;
        for (y = 0; y < 9; y++)
            for (x = 0; x <= y && x < 6; x++)
                Put(frame, px + x, py + y, (x == 0 || x == y || y == 8) ? RGB(2, 2, 2)
                                         : ((sP.frame / 12) & 1 ? RGB(31, 31, 31) : RGB(31, 22, 4)), 16);
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

// The app on the POKéTCH now, as Platinum numbers its apps (its poketch_app_names bank: the TV's POKéTCH WATCH)
u8 Sinnoh_PoketchCurrentApp(void)
{
    static const u8 sPlatinum[APP_COUNT] = {
        [APP_WATCH] = 0, [APP_CALC] = 1, [APP_MEMO] = 2, [APP_PEDOMETER] = 3, [APP_PARTY] = 4, [APP_FRIENDSHIP] = 5,
        [APP_DOWSING] = 6, [APP_BERRIES] = 7, [APP_DAYCARE] = 8, [APP_HISTORY] = 9, [APP_COUNTER] = 10,
        [APP_ANALOG] = 11, [APP_MAP] = 12, [APP_LINK] = 13, [APP_COIN] = 14, [APP_MOVES] = 15, [APP_CALENDAR] = 16,
        [APP_DOTS] = 17, [APP_ROULETTE] = 18, [APP_RADAR] = 19, [APP_TIMER] = 20, [APP_COLOR] = 21,
        [APP_MATCHUP] = 22, [APP_STOPWATCH] = 23, [APP_ALARM] = 24, [APP_BACKLIGHT] = 0,
    };
    return sP.app < APP_COUNT ? sPlatinum[sP.app] : 0;
}
