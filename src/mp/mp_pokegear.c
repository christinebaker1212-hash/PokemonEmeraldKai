// Moemon Kai: JOHTO's POKéGEAR, as Crystal's / HeartGold and SoulSilver's (the user, 2026-11-02: display,
// functions, calls; no clock setting: the time is the game's own clock), drawn as a 2000s multifunction phone (the
// user, the same day: Crystal's screens for inspiration, "2000s multifunction cell phone" for the design language):
// Crystal's orange body around a green backlit LCD; a status bar with the signal bars, Crystal's four card icons
// (the open one inverted, as a phone's menu) and the battery and time; Crystal's bordered message box; soft-key
// labels under it. LEFT / RIGHT change cards, B is off:
// - CLOCK: the time in big seven-segment digits, the day; any button turns it off (Crystal's "Press any button to exit.")
// - MAP: the region's TOWN MAP (field_region_map.c), back to the POKéGEAR after
// - PHONE: the phone book (MOM, PROF. ELM, empty "----------:" rows), "Whom do you want to call?"; a call shows its
//   timer, Crystal's words in the message box; MOM's savings question on the soft keys YES / NO (mp_mombank.c);
//   Crystal's words (pokecrystal data/phone/text/mom.asm, elm.asm)
// - RADIO: a black FM dial, UP / DOWN tune the red needle 0.0-20.0; a station plays only on its frequency: 4.5 OAK's
//   POKéMON TALK (MARY and PROF. OAK on a POKéMON seen around a place of the region, from its wild tables, in Crystal's
//   words: data/text/common_1.asm _OPT_*), 20.0 the POKé FLUTE. Crystal's other stations need music the game hasn't
//   got (the MARCH, the LULLABY...).
// On START in JOHTO once MOM has given it (Johto_PlayersHouse1F); elsewhere a key item (ITEM_POKEGEAR, registrable)
// once JOHTO's LEAGUE is beaten (mp_region.c MP_Region_DeviceHere / MP_Region_GiveDevices).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "wild_encounter.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mp_region.h"

enum { CARD_CLOCK, CARD_MAP, CARD_PHONE, CARD_RADIO, CARD_COUNT };
enum { WIN_STATUS, WIN_MAIN, WIN_MSG, WIN_KEYS, WIN_COUNT };   // (the LCD, top to bottom; the body is the backdrop)
enum { PHONE_LIST, PHONE_CALL, PHONE_ASK };

// palette 15
#define C_LCD    1   // the backlit screen
#define C_INK    2
#define C_SHADOW 3   // the text's shadow on the LCD
#define C_DARK   4   // inverted bars, borders
#define C_WHITE  5
#define C_RED    6
#define C_MID    7   // dotted rules, the screen's inner edge
#define C_GLOW   8   // the screen's lit edge
#define C_BODY   9   // the POKéGEAR's orange (BG1, one tile all over)
#define BODY RGB(31, 17, 8)   // Crystal's POKéGEAR orange

static const u16 sPalette[16] = {
    0, RGB(24, 29, 18), RGB(2, 4, 2), RGB(18, 23, 13), RGB(5, 8, 4), RGB(31, 31, 31), RGB(29, 5, 4),
    RGB(17, 22, 12), RGB(28, 31, 23), BODY, 0, 0, 0, 0, 0, 0,
};

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},   // the body
};

#define LCD_W 224
static const struct WindowTemplate sWindows[] = {
    [WIN_STATUS] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 1},
    [WIN_MAIN] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 3, .width = 28, .height = 9, .paletteNum = 15, .baseBlock = 57},
    [WIN_MSG] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 12, .width = 28, .height = 5, .paletteNum = 15, .baseBlock = 309},
    [WIN_KEYS] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 17, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 449},
    DUMMY_WIN_TEMPLATE,
};

static const u8 sColLcd[] = {C_LCD, C_INK, C_SHADOW};
static const u8 sColInv[] = {C_DARK, C_LCD, C_DARK};
static const u8 sColDial[] = {C_DARK, C_WHITE, C_DARK};

static const u8 sText_Days[7][10] = {_("SUNDAY"), _("MONDAY"), _("TUESDAY"), _("WEDNESDAY"), _("THURSDAY"),
                                      _("FRIDAY"), _("SATURDAY")};
static const u8 sText_Johto[] = _("JOHTO");
static const u8 sText_Kanto[] = _("KANTO");
static const u8 sText_Am[] = _("AM");
static const u8 sText_Pm[] = _("PM");
static const u8 sText_Mom[] = _("MOM:");
static const u8 sText_Elm[] = _("PROF.ELM:");
static const u8 sText_Empty[] = _("----------:");
static const u8 sText_MomName[] = _("MOM");
static const u8 sText_ElmName[] = _("PROF.ELM");
static const u8 sText_TownMap[] = _("TOWN MAP");
static const u8 sText_Calling[] = _("CALLING…");
static const u8 sText_Talk[] = _("OAK’s POKéMON Talk");
static const u8 sText_Flute[] = _("POKé FLUTE");
static const u8 sText_NoSignal[] = _("- - - - -");
static const u8 sText_Tuning[] = _("TUNING");
static const u8 sText_Dial[5][3] = {_("0"), _("5"), _("10"), _("15"), _("20")};
static const u8 sMsg_Exit[] = _("Press any button\nto exit.");
static const u8 sMsg_Map[] = _("Press {A_BUTTON} to see the\nTOWN MAP.");
static const u8 sMsg_Whom[] = _("Whom do you want\nto call?");
static const u8 sMsg_Tune[] = _("Tune in with {DPAD_UPDOWN}.");
static const u8 sKey_Exit[] = _("EXIT{B_BUTTON}");
static const u8 sKey_Back[] = _("BACK{B_BUTTON}");
static const u8 sKey_No[] = _("NO{B_BUTTON}");
static const u8 sKey_Open[] = _("{A_BUTTON}OPEN");
static const u8 sKey_Call[] = _("{A_BUTTON}CALL");
static const u8 sKey_Next[] = _("{A_BUTTON}NEXT");
static const u8 sKey_Yes[] = _("{A_BUTTON}YES");
static const u8 sKey_Cards[] = _("{DPAD_LEFTRIGHT}");
static const u8 sKey_Tune[] = _("{DPAD_UPDOWN}TUNE");

// Crystal's card icons, as an LCD draws them ('#': a pixel)
static const char *const sIcons[CARD_COUNT][14] = {
    {"....######....", "..##......##..", ".#....#.....#.", ".#....#.....#.", "#.....#......#", "#.....#......#",
     "#.....####...#", "#............#", "#............#", ".#..........#.", ".#..........#.", "..##......##..",
     "....######....", ".............."},
    {"##############", "#...#....#...#", "#...#.##.#...#", "#...#....#.#.#", "#.#.#....#...#", "#...#..#.#...#",
     "#...#....#...#", "#..##....#.#.#", "#...#....#...#", "#.#.#.#..#...#", "#...#....#...#", "##############",
     "..............", ".............."},
    {"..........#...", "..........#...", "....#######...", "....#.....#...", "....#.###.#...", "....#.###.#...",
     "....#.....#...", "....#.#.#.#...", "....#.....#...", "....#.#.#.#...", "....#.....#...", "....#.#.#.#...",
     "....#######...", ".............."},
    {"...........#..", "..........#...", ".........#....", "##############", "#............#", "#.###....##..#",
     "#.###...#..#.#", "#.###...#..#.#", "#........##..#", "#............#", "##############", "..............",
     "..............", ".............."},
};

static EWRAM_DATA struct
{
    u8 card, cursor, phone, page, pages, freq, station, talkStep;
    u16 talkTimer, lastMinute, song, callFrames;
    u16 talkSpecies;
    u8 talkPlace[24];
    u8 lines[12][40];   // the call's lines (two to a page), or the radio's now
    bool8 leaving;
} *sG = NULL;
static EWRAM_DATA u8 sReturnCard = 0;
static EWRAM_DATA bool8 sToField = FALSE;   // (from the BAG or SELECT: back to the field, not to START)

#define FREQ_TALK  9    // 4.5
#define FREQ_FLUTE 40   // 20.0

// ---------------------------------------------------------------------------------------------------- drawing
static void Print(u8 win, u8 font, const u8 *text, u8 x, u8 y, const u8 *colors)
{
    AddTextPrinterParameterized3(win, font, x, y, colors, TEXT_SKIP_DRAW, text);
}

static void PrintRight(u8 win, u8 font, const u8 *text, u8 right, u8 y, const u8 *colors)
{
    Print(win, font, text, right - GetStringWidth(font, text, 0), y, colors);
}

static void PrintCentered(u8 win, u8 font, const u8 *text, u8 y, const u8 *colors)
{
    Print(win, font, text, (LCD_W - GetStringWidth(font, text, 0)) / 2, y, colors);
}

static void Rect(u8 win, u8 color, u16 x, u16 y, u16 w, u16 h)
{
    FillWindowPixelRect(win, PIXEL_FILL(color), x, y, w, h);
}

// the LCD's sides (every window), its top corners (status) and bottom corners (soft keys)
static void Sides(u8 win, u8 h)
{
    Rect(win, C_DARK, 0, 0, 2, h);
    Rect(win, C_DARK, LCD_W - 2, 0, 2, h);
    Rect(win, C_GLOW, 2, 0, 1, h);
}

static void Corners(u8 win, bool8 top, u8 h)
{
    u8 y0 = top ? 0 : h - 1, y1 = top ? 1 : h - 2;
    Rect(win, 0, 0, y0, 3, 1);
    Rect(win, 0, LCD_W - 3, y0, 3, 1);
    Rect(win, 0, 0, y1, 1, 1);
    Rect(win, 0, LCD_W - 1, y1, 1, 1);
}

static void DrawIcon(u8 win, u8 card, u8 x, u8 y, u8 color)
{
    u8 i, j;
    for (j = 0; j < 14; j++)
        for (i = 0; i < 14; i++)
            if (sIcons[card][j][i] == '#')
                Rect(win, color, x + i, y + j, 1, 1);
}

// a seven-segment digit, W x H, segments T thick
static void Digit(u8 win, u8 d, u16 x, u16 y, u8 w, u8 h, u8 t, u8 color)
{
    static const u8 sSegs[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};   // (gfedcba)
    u8 s = sSegs[d % 10], half = h / 2;
    if (s & 0x01) Rect(win, color, x + 1, y, w - 2, t);                       // a
    if (s & 0x02) Rect(win, color, x + w - t, y + 1, t, half - 1);            // b
    if (s & 0x04) Rect(win, color, x + w - t, y + half, t, half - 1);         // c
    if (s & 0x08) Rect(win, color, x + 1, y + h - t, w - 2, t);               // d
    if (s & 0x10) Rect(win, color, x, y + half, t, half - 1);                 // e
    if (s & 0x20) Rect(win, color, x, y + 1, t, half - 1);                    // f
    if (s & 0x40) Rect(win, color, x + 1, y + half - t / 2, w - 2, t);        // g
}

static const u8 *RegionName(void)
{
    return MP_Region_Current() == REGION_KANTO ? sText_Kanto : sText_Johto;
}

static void DrawStatus(void)
{
    u8 i, buf[8], *p;
    FillWindowPixelBuffer(WIN_STATUS, PIXEL_FILL(C_LCD));
    Rect(WIN_STATUS, C_DARK, 0, 0, LCD_W, 2);
    Rect(WIN_STATUS, C_GLOW, 2, 2, LCD_W - 4, 1);
    Sides(WIN_STATUS, 16);
    Corners(WIN_STATUS, TRUE, 16);
    for (i = 0; i < 4; i++)   // the signal: four bars
        Rect(WIN_STATUS, C_INK, 8 + i * 4, 13 - (i + 1) * 2 - 1, 2, (i + 1) * 2 + 1);
    for (i = 0; i < CARD_COUNT; i++)   // the cards, the open one inverted
    {
        u8 x = 64 + i * 24;
        if (i == sG->card)
        {
            Rect(WIN_STATUS, C_DARK, x - 2, 3, 18, 13);
            DrawIcon(WIN_STATUS, i, x, 3, C_LCD);
        }
        else
        {
            DrawIcon(WIN_STATUS, i, x, 3, C_INK);
        }
    }
    RtcCalcLocalTime();   // the time, then the battery
    p = ConvertIntToDecimalStringN(buf, gLocalTime.hours, STR_CONV_MODE_LEADING_ZEROS, 2);
    *p++ = CHAR_COLON;
    ConvertIntToDecimalStringN(p, gLocalTime.minutes, STR_CONV_MODE_LEADING_ZEROS, 2);
    PrintRight(WIN_STATUS, FONT_SMALL, buf, 190, 4, sColLcd);
    Rect(WIN_STATUS, C_INK, 196, 5, 18, 8);
    Rect(WIN_STATUS, C_LCD, 197, 6, 16, 6);
    Rect(WIN_STATUS, C_INK, 214, 7, 2, 4);
    for (i = 0; i < 3; i++)
        Rect(WIN_STATUS, C_INK, 198 + i * 5, 7, 4, 4);
    CopyWindowToVram(WIN_STATUS, COPYWIN_FULL);
}

static void DrawKeys(const u8 *left, const u8 *right, const u8 *mid)
{
    FillWindowPixelBuffer(WIN_KEYS, PIXEL_FILL(C_LCD));
    Rect(WIN_KEYS, C_MID, 4, 0, LCD_W - 8, 1);
    Sides(WIN_KEYS, 16);
    Rect(WIN_KEYS, C_DARK, 0, 14, LCD_W, 2);
    Corners(WIN_KEYS, FALSE, 16);
    if (left != NULL)
        Print(WIN_KEYS, FONT_NORMAL, left, 8, 0, sColLcd);
    if (right != NULL)
        PrintRight(WIN_KEYS, FONT_NORMAL, right, LCD_W - 8, 0, sColLcd);
    if (mid != NULL)
        PrintCentered(WIN_KEYS, FONT_NORMAL, mid, 0, sColLcd);
    CopyWindowToVram(WIN_KEYS, COPYWIN_FULL);
}

// Crystal's message box: a double rule round two lines
static void DrawMsg(const u8 *a, const u8 *b)
{
    FillWindowPixelBuffer(WIN_MSG, PIXEL_FILL(C_LCD));
    Sides(WIN_MSG, 40);
    Rect(WIN_MSG, C_INK, 6, 2, LCD_W - 12, 36);
    Rect(WIN_MSG, C_LCD, 7, 3, LCD_W - 14, 34);
    Rect(WIN_MSG, C_INK, 8, 4, LCD_W - 16, 32);
    Rect(WIN_MSG, C_LCD, 10, 5, LCD_W - 20, 30);
    Rect(WIN_MSG, C_LCD, 6, 2, 2, 1);   // (the rounded corners)
    Rect(WIN_MSG, C_LCD, LCD_W - 8, 2, 2, 1);
    Rect(WIN_MSG, C_LCD, 6, 37, 2, 1);
    Rect(WIN_MSG, C_LCD, LCD_W - 8, 37, 2, 1);
    if (a != NULL)
        Print(WIN_MSG, FONT_NORMAL, a, 14, b == NULL ? 4 : 5, sColLcd);
    if (b != NULL)
        Print(WIN_MSG, FONT_NORMAL, b, 14, 19, sColLcd);
    CopyWindowToVram(WIN_MSG, COPYWIN_FULL);
}

static void MainBegin(void)
{
    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(C_LCD));
    Sides(WIN_MAIN, 72);
}

static void DrawClock(void)
{
    u8 hour, x, buf[12];
    RtcCalcLocalTime();
    hour = gLocalTime.hours % 12 ? gLocalTime.hours % 12 : 12;
    x = 54;
    if (hour >= 10)
        Digit(WIN_MAIN, hour / 10, x, 6, 18, 30, 4, C_INK);
    Digit(WIN_MAIN, hour % 10, x + 24, 6, 18, 30, 4, C_INK);
    Rect(WIN_MAIN, C_INK, x + 47, 13, 4, 4);   // (the colon)
    Rect(WIN_MAIN, C_INK, x + 47, 25, 4, 4);
    Digit(WIN_MAIN, gLocalTime.minutes / 10, x + 56, 6, 18, 30, 4, C_INK);
    Digit(WIN_MAIN, gLocalTime.minutes % 10, x + 80, 6, 18, 30, 4, C_INK);
    Print(WIN_MAIN, FONT_NORMAL, gLocalTime.hours < 12 ? sText_Am : sText_Pm, x + 104, 22, sColLcd);
    StringCopy(buf, sText_Days[gLocalTime.days % 7]);
    PrintCentered(WIN_MAIN, FONT_NORMAL, buf, 42, sColLcd);
    PrintCentered(WIN_MAIN, FONT_SMALL, RegionName(), 58, sColLcd);
    sG->lastMinute = gLocalTime.minutes;
}

static void DrawMap(void)
{
    u8 i, j;
    for (j = 0; j < 14; j++)   // the map icon, twice its size
        for (i = 0; i < 14; i++)
            if (sIcons[CARD_MAP][j][i] == '#')
                Rect(WIN_MAIN, C_INK, 98 + i * 2, 8 + j * 2, 2, 2);
    PrintCentered(WIN_MAIN, FONT_NORMAL, sText_TownMap, 36, sColLcd);
    PrintCentered(WIN_MAIN, FONT_SMALL, RegionName(), 54, sColLcd);
}

static void DrawPhone(void)
{
    static const u8 *const sRows[] = {sText_Mom, sText_Elm, sText_Empty, sText_Empty};
    u8 i, x;
    if (sG->phone != PHONE_LIST)   // a call: who, and its timer
    {
        u8 buf[12], *p;
        u16 s = sG->callFrames / 60;
        DrawIcon(WIN_MAIN, CARD_PHONE, 100, 6, C_INK);
        PrintCentered(WIN_MAIN, FONT_NORMAL, sG->cursor == 0 ? sText_MomName : sText_ElmName, 24, sColLcd);
        p = ConvertIntToDecimalStringN(buf, s / 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        *p++ = CHAR_COLON;
        ConvertIntToDecimalStringN(p, s % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        PrintCentered(WIN_MAIN, FONT_NORMAL, sG->callFrames < 60 ? sText_Calling : buf, 44, sColLcd);
        return;
    }
    for (i = 0; i < ARRAY_COUNT(sRows); i++)
    {
        u8 y = 4 + i * 16;
        if (i == sG->cursor)
            Rect(WIN_MAIN, C_DARK, 6, y, LCD_W - 12, 15);
        Print(WIN_MAIN, FONT_NORMAL, sRows[i], 12, y, i == sG->cursor ? sColInv : sColLcd);
        if (i < ARRAY_COUNT(sRows) - 1)
            for (x = 8; x < LCD_W - 8; x += 3)   // (dotted rules)
                Rect(WIN_MAIN, C_MID, x, y + 15, 1, 1);
    }
}

static u16 FreqX(u8 freq)
{
    return 16 + freq * 46 / 10;
}

static void DrawRadio(void)
{
    u8 f, buf[12], *p;
    Rect(WIN_MAIN, C_DARK, 8, 2, LCD_W - 16, 38);   // the dial
    for (f = 0; f < 5; f++)   // (the MHz, over their ticks)
        Print(WIN_MAIN, FONT_SMALL, sText_Dial[f], FreqX(f * 10) - GetStringWidth(FONT_SMALL, sText_Dial[f], 0) / 2, 2, sColDial);
    for (f = 0; f <= 40; f++)
        Rect(WIN_MAIN, C_WHITE, FreqX(f), f % 10 == 0 ? 14 : 17, 1, f % 10 == 0 ? 6 : 3);
    Rect(WIN_MAIN, C_WHITE, FreqX(0), 20, FreqX(40) - FreqX(0) + 1, 1);
    Rect(WIN_MAIN, C_RED, FreqX(sG->freq) - 1, 10, 3, 18);   // the needle
    p = ConvertIntToDecimalStringN(buf, sG->freq / 2, STR_CONV_MODE_LEFT_ALIGN, 2);
    *p++ = CHAR_PERIOD;
    *p++ = CHAR_0 + (sG->freq % 2) * 5;
    *p = EOS;
    Print(WIN_MAIN, FONT_SMALL, buf, 16, 28, sColDial);
    PrintRight(WIN_MAIN, FONT_SMALL, sText_Tuning, LCD_W - 14, 28, sColDial);
    Rect(WIN_MAIN, C_INK, 40, 46, LCD_W - 80, 22);   // the station's box
    Rect(WIN_MAIN, C_LCD, 41, 47, LCD_W - 82, 20);
    PrintCentered(WIN_MAIN, FONT_NORMAL, sG->station == 1 ? sText_Talk : sG->station == 2 ? sText_Flute : sText_NoSignal,
                  49, sColLcd);
}

static void DrawAll(void)
{
    DrawStatus();
    MainBegin();
    switch (sG->card)
    {
    case CARD_CLOCK:
        DrawClock();
        DrawMsg(sMsg_Exit, NULL);
        DrawKeys(NULL, sKey_Exit, sKey_Cards);
        break;
    case CARD_MAP:
        DrawMap();
        DrawMsg(sMsg_Map, NULL);
        DrawKeys(sKey_Open, sKey_Exit, sKey_Cards);
        break;
    case CARD_PHONE:
        DrawPhone();
        if (sG->phone == PHONE_LIST)
        {
            DrawMsg(sMsg_Whom, NULL);
            DrawKeys(sKey_Call, sKey_Exit, sKey_Cards);
        }
        else
        {
            bool8 ask = sG->phone == PHONE_ASK && sG->page == sG->pages - 1;
            DrawMsg(sG->lines[sG->page * 2], sG->lines[sG->page * 2 + 1]);
            DrawKeys(ask ? sKey_Yes : sKey_Next, ask ? sKey_No : NULL, NULL);
        }
        break;
    case CARD_RADIO:
        DrawRadio();
        if (sG->station == 1)
            DrawMsg(sG->lines[0], sG->lines[1]);
        else
            DrawMsg(sMsg_Tune, NULL);
        DrawKeys(NULL, sKey_Exit, sKey_Tune);
        break;
    }
    CopyWindowToVram(WIN_MAIN, COPYWIN_FULL);
}

// ---------------------------------------------------------------------------------------------------- calls
// Crystal’s words: each string a line of the call (two to a page); {PLAYER} and the buffers filled first
static void SetLines(const u8 *const *lines, u8 count)
{
    u8 i;
    memset(sG->lines, EOS, sizeof(sG->lines));
    for (i = 0; i < count && i < ARRAY_COUNT(sG->lines); i++)
        StringExpandPlaceholders(sG->lines[i], lines[i]);
    sG->pages = (count + 1) / 2;
    sG->page = 0;
}

static const u8 sMom_Hello1[] = _("Hello?");
static const u8 sMom_Hello2[] = _("Oh, hi, {PLAYER}! Working hard?");
static const u8 sMom_Where1[] = _("Really, you’re in {STR_VAR_1}?");
static const u8 sMom_Where2[] = _("I’ve never gone there. That’s");
static const u8 sMom_Where3[] = _("kind of neat, {PLAYER}.");
static const u8 sMom_Home1[] = _("What? You’re in {STR_VAR_1}?");
static const u8 sMom_Home2[] = _("Come see your MOM sometime!");
static const u8 sMom_Saved1[] = _("By the way, you’ve saved up");
static const u8 sMom_Saved2[] = _("¥{STR_VAR_2}. Do you want to keep on saving?");
static const u8 sMom_NotSaving1[] = _("Oh, {PLAYER}, you’re not saving any");
static const u8 sMom_NotSaving2[] = _("money. Would you like to save?");
static const u8 sMom_Bye1[] = _("{PLAYER}, keep it up! I’m rooting");
static const u8 sMom_Bye2[] = _("for you, baby!");
static const u8 sMom_Important[] = _("It’s important to save your money.");
static const u8 sMom_WontSave[] = _("OK. I won’t save your money.");
static const u8 sMom_WillSave[] = _("OK. I’ll save your money.");
static const u8 sEmpty[] = _("");

static const u8 sElm_Heal1[] = _("Hello, {PLAYER}?");
static const u8 sElm_Heal2[] = _("Try not to overdo it.");
static const u8 sElm_Heal3[] = _("Be sure to heal your MOéMON if");
static const u8 sElm_Heal4[] = _("they are hurt.");
static const u8 sElm_Hatch1[] = _("Hello, {PLAYER}? I’ve discovered");
static const u8 sElm_Hatch2[] = _("something about MOéMON EGGS!");
static const u8 sElm_Hatch3[] = _("It seems that how long an EGG takes");
static const u8 sElm_Hatch4[] = _("to hatch depends on the MOéMON.");
static const u8 sElm_Moves1[] = _("Hello, {PLAYER}? I’m investigating");
static const u8 sElm_Moves2[] = _("MOéMON EGGS right now.");
static const u8 sElm_Moves3[] = _("It seems that some MOéMON hatch");
static const u8 sElm_Moves4[] = _("knowing moves their parents knew.");

extern bool8 MP_MomBank_Saving(void);
extern bool8 MP_MomBank_Asked(void);
extern void MP_MomBank_SetSaving(bool8 on);
extern u32 MP_MomBank_Money(void);

static void CallMom(void)
{
    const u8 *lines[8];
    u8 n = 0;
    u16 home = MP_Region_Current() == REGION_JOHTO
            && Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum)->regionMapSectionId
               == Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN), MAP_NUM(MAP_JOHTO_NEW_BARK_TOWN))->regionMapSectionId;
    GetMapName(gStringVar1, gMapHeader.regionMapSectionId, 0);
    ConvertIntToDecimalStringN(gStringVar2, MP_MomBank_Money(), STR_CONV_MODE_LEFT_ALIGN, 6);
    lines[n++] = sMom_Hello1;
    lines[n++] = sMom_Hello2;
    if (home)
    {
        lines[n++] = sMom_Home1;
        lines[n++] = sMom_Home2;
    }
    else
    {
        lines[n++] = sMom_Where1;
        lines[n++] = sMom_Where2;
        lines[n++] = sMom_Where3;
        lines[n++] = sEmpty;
    }
    if (MP_MomBank_Asked())
    {
        lines[n++] = MP_MomBank_Saving() ? sMom_Saved1 : sMom_NotSaving1;
        lines[n++] = MP_MomBank_Saving() ? sMom_Saved2 : sMom_NotSaving2;
        sG->phone = PHONE_ASK;
    }
    else
    {
        lines[n++] = sMom_Bye1;
        lines[n++] = sMom_Bye2;
        sG->phone = PHONE_CALL;
    }
    SetLines(lines, n);
}

static void MomAnswer(bool8 yes)
{
    const u8 *lines[4];
    bool8 wasSaving = MP_MomBank_Saving();
    MP_MomBank_SetSaving(yes);   // (saving: YES keeps on, NO stops; not saving: YES starts)
    lines[0] = wasSaving ? (yes ? sMom_Important : sMom_WontSave) : (yes ? sMom_WillSave : sMom_WontSave);
    lines[1] = sEmpty;
    lines[2] = sMom_Bye1;
    lines[3] = sMom_Bye2;
    SetLines(lines, 4);
    sG->phone = PHONE_CALL;
}

static void CallElm(void)
{
    static const u8 *const heal[] = {sElm_Heal1, sElm_Heal2, sElm_Heal3, sElm_Heal4};
    static const u8 *const hatch[] = {sElm_Hatch1, sElm_Hatch2, sElm_Hatch3, sElm_Hatch4};
    static const u8 *const moves[] = {sElm_Moves1, sElm_Moves2, sElm_Moves3, sElm_Moves4};
    // Crystal’s ELM talks about the EGG he studies once you’ve shown him what hatched; here, once JOHTO’s second
    // BADGE is won (Quetzal’s story has no TOGEPI EGG)
    if (MP_Region_BadgeCount(REGION_JOHTO) < 2)
        SetLines(heal, 4);
    else
        SetLines(Random() % 2 ? hatch : moves, 4);
    sG->phone = PHONE_CALL;
}

// ---------------------------------------------------------------------------------------------------- radio
static const u8 sOpt_Intro1[] = _("MARY: PROF. OAK’S MOéMON TALK!");
static const u8 sOpt_Intro2[] = _("With me, MARY!");
static const u8 sOpt_Oak1[] = _("OAK: {STR_VAR_1}");
static const u8 sOpt_Oak2[] = _("may be seen around {STR_VAR_2}.");
static const u8 sOpt_Mary1[] = _("MARY: {STR_VAR_1}’s");
static const u8 sOpt_Adverbs[][20] = {
    _("sweet and adorably"),
    _("wiggly and slickly"),
    _("aptly named and"),
    _("undeniably kind of"),
    _("so, so unbearably"),
    _("wow, impressively"),
    _("almost poisonously"),
    _("ooh, so sensually"),
    _("so mischievously"),
    _("so very topically"),
    _("sure addictively"),
    _("looks in water is"),
    _("evolution must be"),
    _("provocatively"),
    _("so flipped out and"),
    _("heart-meltingly"),
};
static const u8 sOpt_Adjectives[][20] = {
    _("cute."),
    _("weird."),
    _("pleasant."),
    _("bold, sort of."),
    _("frightening."),
    _("suave & debonair!"),
    _("powerful."),
    _("exciting."),
    _("groovy!"),
    _("inspiring."),
    _("friendly."),
    _("hot, hot, hot!"),
    _("stimulating."),
    _("guarded."),
    _("lovely."),
    _("speedy."),
};

// a POKéMON of the region’s grass and where (its wild tables, as Crystal’s show picks a route’s)
static void PickTalkMon(void)
{
    u32 n = 0, i, pick;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].landMonsInfo != NULL && MP_Region_OfGroup(gWildMonHeaders[i].mapGroup) == MP_Region_Current())
            n++;
    sG->talkSpecies = SPECIES_NONE;
    if (n == 0)
        return;
    pick = Random() % n;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[i];
        if (h->landMonsInfo == NULL || MP_Region_OfGroup(h->mapGroup) != MP_Region_Current() || pick-- != 0)
            continue;
        sG->talkSpecies = MP_Region_WildInfo(h->landMonsInfo)->wildPokemon[Random() % 12].species;   // (as the GENERATION has it)
        GetMapName(sG->talkPlace, Overworld_GetMapHeaderByGroupAndId(h->mapGroup, h->mapNum)->regionMapSectionId, 0);
        return;
    }
}

// the show’s lines now: the greeting, then OAK and MARY in turn, a new POKéMON each time round
static void TalkLines(void)
{
    const u8 *a = sEmpty, *b = sEmpty;
    switch (sG->talkStep % 4)
    {
    case 0: a = sOpt_Intro1; b = sOpt_Intro2; break;
    case 1:
        PickTalkMon();
        GetSpeciesName(gStringVar1, sG->talkSpecies);
        StringCopy(gStringVar2, sG->talkPlace);
        a = sOpt_Oak1; b = sOpt_Oak2;
        break;
    case 2:
        GetSpeciesName(gStringVar1, sG->talkSpecies);
        a = sOpt_Mary1; b = sOpt_Adverbs[Random() % ARRAY_COUNT(sOpt_Adverbs)];
        break;
    case 3:
        a = sOpt_Adjectives[Random() % ARRAY_COUNT(sOpt_Adjectives)];
        break;
    }
    StringExpandPlaceholders(sG->lines[0], a);
    StringExpandPlaceholders(sG->lines[1], b);
}

// the dial at FREQ: on a station, its music (and the talk); between them the map's music
static void TuneTo(u8 freq)
{
    u8 station = freq == FREQ_TALK ? 1 : freq == FREQ_FLUTE ? 2 : 0;
    sG->freq = freq;
    if (station == sG->station)
        return;
    sG->station = station;
    sG->talkStep = 0;
    sG->talkTimer = 0;
    PlayBGM(station == 1 ? MUS_JOHTO_PROF_OAK : station == 2 ? MUS_RG_POKE_FLUTE : sG->song);
    if (station == 1)
        TalkLines();
}

// ---------------------------------------------------------------------------------------------------- the screen
static void VBlankCB_Pokegear(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Pokegear(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void CB2_OpenPokegearAgain(void);

static void Leave(u8 taskId, MainCallback next)
{
    sG->leaving = TRUE;
    SetWordTaskArg(taskId, 1, (uintptr_t)next);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void Task_Pokegear(u8 taskId)
{
    if (sG->leaving)
    {
        if (!gPaletteFade.active)
        {
            MainCallback next = (MainCallback)GetWordTaskArg(taskId, 1);
            if (sG->station != 0)
                PlayBGM(sG->song);
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            Free(GetBgTilemapBuffer(1));
            FREE_AND_SET_NULL(sG);
            SetMainCallback2(next);
        }
        return;
    }
    if (gPaletteFade.active)
        return;
    RtcCalcLocalTime();
    if (gLocalTime.minutes != sG->lastMinute)   // (the status bar's time, the clock card)
    {
        sG->lastMinute = gLocalTime.minutes;
        DrawAll();
    }
    if (sG->card == CARD_RADIO && sG->station == 1 && ++sG->talkTimer >= 150)   // (a line every two and a half seconds)
    {
        sG->talkTimer = 0;
        sG->talkStep++;
        TalkLines();
        DrawAll();
    }

    if (sG->card == CARD_PHONE && sG->phone != PHONE_LIST)   // a call: A turns the page; MOM's question A YES / B NO
    {
        bool8 last = sG->page >= sG->pages - 1;
        if (++sG->callFrames % 60 == 0 || sG->callFrames == 60)
            DrawAll();
        if (sG->phone == PHONE_ASK && last && (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON)))
        {
            PlaySE(SE_SELECT);
            MomAnswer(JOY_NEW(A_BUTTON));
            DrawAll();
        }
        else if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (last)
                sG->phone = PHONE_LIST;
            else
                sG->page++;
            DrawAll();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON) || (sG->card == CARD_CLOCK && JOY_NEW(A_BUTTON)))
    {
        PlaySE(SE_SELECT);
        sReturnCard = 0;
        Leave(taskId, sToField ? CB2_ReturnToField : CB2_ReturnToFieldWithOpenMenu);
    }
    else if (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_RIGHT))
    {
        PlaySE(SE_SELECT);
        sG->card = (sG->card + (JOY_NEW(DPAD_RIGHT) ? 1 : CARD_COUNT - 1)) % CARD_COUNT;
        sG->cursor = 0;
        sG->phone = PHONE_LIST;
        if (sG->station != 0)   // (the radio plays while its card is open)
        {
            sG->station = 0;
            PlayBGM(sG->song);
        }
        if (sG->card == CARD_RADIO)
            TuneTo(sG->freq);
        DrawAll();
    }
    else if (sG->card == CARD_RADIO && (JOY_REPEAT(DPAD_UP) || JOY_REPEAT(DPAD_DOWN)))
    {
        if (JOY_REPEAT(DPAD_UP) && sG->freq < FREQ_FLUTE)
            TuneTo(sG->freq + 1);
        else if (JOY_REPEAT(DPAD_DOWN) && sG->freq > 0)
            TuneTo(sG->freq - 1);
        DrawAll();
    }
    else if (sG->card == CARD_PHONE && JOY_NEW(DPAD_UP) && sG->cursor > 0)
        sG->cursor--, PlaySE(SE_SELECT), DrawAll();
    else if (sG->card == CARD_PHONE && JOY_NEW(DPAD_DOWN) && sG->cursor < 3)
        sG->cursor++, PlaySE(SE_SELECT), DrawAll();
    else if (JOY_NEW(A_BUTTON))
    {
        if (sG->card == CARD_MAP)
        {
            PlaySE(SE_SELECT);
            sReturnCard = CARD_MAP;
            Leave(taskId, CB2_OpenPokegearAgain);
        }
        else if (sG->card == CARD_PHONE && sG->cursor < 2)   // (the empty rows have no number)
        {
            PlaySE(SE_POKENAV_CALL);
            sG->callFrames = 0;
            if (sG->cursor == 0)
                CallMom();
            else
                CallElm();
            DrawAll();
        }
    }
}

static void OpenPokegear(u8 card)
{
    static const u16 sBody = BODY;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadPalette(sPalette, BG_PLTT_ID(15), sizeof(sPalette));
    LoadPalette(&sBody, BG_PLTT_ID(0), sizeof(u16));   // (the backdrop: the POKéGEAR's body)
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    sG = AllocZeroed(sizeof(*sG));
    sG->card = card;
    sG->song = GetCurrentMapMusic();
    {
        static const u32 sBodyTile[8] = {0x99999999, 0x99999999, 0x99999999, 0x99999999,
                                         0x99999999, 0x99999999, 0x99999999, 0x99999999};   // (color 9, all over)
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        LoadBgTiles(1, sBodyTile, sizeof(sBodyTile), 1);
        FillBgTilemapBufferRect(1, 1, 0, 0, 32, 32, 15);
        CopyBgTilemapBufferToVram(1);
    }
    PutWindowTilemap(WIN_STATUS);
    PutWindowTilemap(WIN_MAIN);
    PutWindowTilemap(WIN_MSG);
    PutWindowTilemap(WIN_KEYS);
    DrawAll();
    ScheduleBgCopyTilemapToVram(0);
    ShowBg(0);
    ShowBg(1);
    CreateTask(Task_Pokegear, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Pokegear);
    SetMainCallback2(CB2_Pokegear);
}

static void CB2_BackToPokegear(void)
{
    OpenPokegear(sReturnCard);
}

static void CB2_OpenPokegearAgain(void)
{
    FieldInitRegionMap(CB2_BackToPokegear);   // the MAP card's TOWN MAP, then the POKéGEAR again
}

// START > POKéGEAR (start_menu.c) and the key item (item_use.c)
void CB2_MP_OpenPokegear(void)
{
    sToField = FALSE;
    OpenPokegear(CARD_CLOCK);
}

void CB2_MP_OpenPokegearToField(void)
{
    sToField = TRUE;
    OpenPokegear(CARD_CLOCK);
}

// Tests (harness condition "pokegear"): the screen is up and settled
bool8 MP_Pokegear_Active(void)
{
    return sG != NULL && !sG->leaving && !gPaletteFade.active;
}

// Tests (mp pokegear): the card, the cursor, the phone and the radio
void MP_Pokegear_Debug(void)
{
    if (sG == NULL)
        printf("[test] pokegear: closed\n");
    else
        printf("[test] pokegear: card %u cursor %u phone %u page %u/%u freq %u station %u\n", sG->card, sG->cursor,
               sG->phone, sG->page, sG->pages, sG->freq, sG->station);
}
