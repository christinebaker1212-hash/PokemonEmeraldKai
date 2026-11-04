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
// - RADIO: a black FM dial, UP / DOWN tune the red needle 0.0-20.5; Crystal's stations on Crystal's frequencies, with
//   Crystal's own songs (tools/kaisong/crystal_radio.py) and words: in JOHTO 4.5 OAK's POKéMON TALK (the POKéDEX SHOW in
//   the morning), 7.5 POKéMON MUSIC (the MARCH or the LULLABY by the day), 8.5 the LUCKY CHANNEL, 10.5 BUENA's PASSWORD
//   (6 PM to midnight), 13.5 the UNOWN's sounds at the RUINS OF ALPH, 20.5 the EVOLUTION radio around MAHOGANY while
//   TEAM ROCKET is there (and TEAM ROCKET on every JOHTO station while they hold the RADIO TOWER); out of JOHTO 16.5
//   PLACES AND PEOPLE, 18.5 LET'S ALL SING!, 20.0 the POKé FLUTE
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
#include "data.h"
#include "gba/m4a_internal.h"
#include "m4a.h"
#include "mp_kaisong.h"
#include "lottery_corner.h"
#include "pokedex.h"
#include "constants/pokedex.h"
#include "constants/qz_port.h"
#include "constants/region_map_sections.h"
#include "constants/trainers.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mp_region.h"
#include "mp_qzphone.h"

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
    u8 card, cursor, phone, page, pages, freq, station, show, talkStep, drag, midnight;
    u8 top, nContacts, contacts[48];   // (the phone book's first row shown; JOHTO's numbers: mp_qzphone.c)
    u16 talkTimer, lastMinute, song, callFrames;
    u16 talkSpecies;
    const u8 *dexText;   // (the POKéDEX SHOW's entry, where it has got to)
    u8 talkPlace[24];
    u8 lines[24][40];   // the call's lines (two to a page), or the radio's now
    bool8 leaving;
} *sG = NULL;
static EWRAM_DATA u8 sReturnCard = 0;
static EWRAM_DATA bool8 sToField = FALSE;   // (from the BAG or SELECT: back to the field, not to START)

#define FREQ_TALK  9    // 4.5
#define FREQ_FLUTE 40   // 20.0
#define FREQ_MAX   41   // 20.5 (the dial's end, as Crystal's)

// the radio's stations (StationAt, ShowLine below)
enum
{
    ST_NONE, ST_TALK, ST_DEX, ST_MUSIC, ST_LUCKY, ST_BUENA, ST_UNOWN, ST_PLACES, ST_SING, ST_FLUTE, ST_EVOLUTION,
    ST_ROCKET,
};
static const u8 sName_Dex[] = _("POKéDEX Show");
static const u8 sName_Music[] = _("POKéMON Music");
static const u8 sName_Lucky[] = _("Lucky Channel");
static const u8 sName_Buena[] = _("BUENA’S PASSWORD");
static const u8 sName_Unown[] = _("?????");
static const u8 sName_Places[] = _("Places & People");
static const u8 sName_Sing[] = _("Let’s All Sing!");
static const struct { const u8 *name; u16 song; } sStations[] = {
    [ST_NONE] = {sText_NoSignal, 0},
    [ST_TALK] = {sText_Talk, MUS_JOHTO_RADIO_POKEMON_TALK},
    [ST_DEX] = {sName_Dex, MUS_JOHTO_RADIO_POKEDEX_SHOW},
    [ST_MUSIC] = {sName_Music, MUS_JOHTO_RADIO_POKEMON_MARCH},   // (the LULLABY on Mondays, Wednesdays, Fridays)
    [ST_LUCKY] = {sName_Lucky, MUS_JOHTO_RADIO_LUCKY_CHANNEL},
    [ST_BUENA] = {sName_Buena, MUS_JOHTO_RADIO_BUENAS_PASSWORD},
    [ST_UNOWN] = {sName_Unown, MUS_JOHTO_RADIO_UNOWN},
    [ST_PLACES] = {sName_Places, MUS_JOHTO_RADIO_PLACES_AND_PEOPLE},
    [ST_SING] = {sName_Sing, MUS_JOHTO_RADIO_POKEMON_MARCH},
    [ST_FLUTE] = {sText_Flute, MUS_JOHTO_RADIO_POKE_FLUTE},
    [ST_EVOLUTION] = {sName_Unown, MUS_JOHTO_RADIO_EVOLUTION},
    [ST_ROCKET] = {NULL, MUS_JOHTO_RADIO_ROCKET},   // (on the tuned station, under its own name)
};

extern const struct PokedexEntry gPokedexEntries[];

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
    for (i = 0; i < 4; i++)   // the signal: four bars
        Rect(WIN_STATUS, C_INK, 8 + i * 4, 13 - (i + 1) * 2 - 1, 2, (i + 1) * 2 + 1);
    for (i = 0; i < CARD_COUNT; i++)   // the cards, the open one inverted
    {
        u8 x = 64 + i * 24;
        if (i == CARD_RADIO && !FlagGet(FLAG_JOHTO_RADIO_CARD))
            continue;   // (Crystal's RADIO CARD: the RADIO TOWER's quiz)
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
    PrintRight(WIN_STATUS, FONT_SMALL, buf, 190, 3, sColLcd);
    Rect(WIN_STATUS, C_INK, 196, 5, 18, 8);
    Rect(WIN_STATUS, C_LCD, 197, 6, 16, 6);
    Rect(WIN_STATUS, C_INK, 214, 7, 2, 4);
    for (i = 0; i < 3; i++)
        Rect(WIN_STATUS, C_INK, 198 + i * 5, 7, 4, 4);
    Rect(WIN_STATUS, C_DARK, 0, 0, LCD_W, 2);   // (the frame last: over any text's box)
    Rect(WIN_STATUS, C_GLOW, 2, 2, LCD_W - 4, 1);
    Sides(WIN_STATUS, 16);
    Corners(WIN_STATUS, TRUE, 16);
    CopyWindowToVram(WIN_STATUS, COPYWIN_FULL);
}

static void DrawKeys(const u8 *left, const u8 *right, const u8 *mid)
{
    FillWindowPixelBuffer(WIN_KEYS, PIXEL_FILL(C_LCD));
    if (left != NULL)   // (the labels first: a text's background is a box; the rules go over it)
        Print(WIN_KEYS, FONT_NORMAL, left, 8, 1, sColLcd);
    if (right != NULL)
        PrintRight(WIN_KEYS, FONT_NORMAL, right, LCD_W - 8, 1, sColLcd);
    if (mid != NULL)
        PrintCentered(WIN_KEYS, FONT_NORMAL, mid, 1, sColLcd);
    Rect(WIN_KEYS, C_MID, 4, 0, LCD_W - 8, 1);
    Sides(WIN_KEYS, 16);
    Rect(WIN_KEYS, C_DARK, 0, 14, LCD_W, 2);
    Corners(WIN_KEYS, FALSE, 16);
    CopyWindowToVram(WIN_KEYS, COPYWIN_FULL);
}

// Crystal's message box: a double rule round two lines
static void DrawMsg(const u8 *a, const u8 *b)
{
    FillWindowPixelBuffer(WIN_MSG, PIXEL_FILL(C_LCD));
    if (a != NULL)   // (the text first, then the double rule round it)
        Print(WIN_MSG, FONT_NORMAL, a, 14, 6, sColLcd);
    if (b != NULL)
        Print(WIN_MSG, FONT_NORMAL, b, 14, 20, sColLcd);
    Sides(WIN_MSG, 40);
    Rect(WIN_MSG, C_INK, 6, 2, LCD_W - 12, 1);
    Rect(WIN_MSG, C_INK, 6, 37, LCD_W - 12, 1);
    Rect(WIN_MSG, C_INK, 6, 2, 1, 36);
    Rect(WIN_MSG, C_INK, LCD_W - 7, 2, 1, 36);
    Rect(WIN_MSG, C_INK, 8, 4, LCD_W - 16, 1);
    Rect(WIN_MSG, C_INK, 8, 35, LCD_W - 16, 1);
    Rect(WIN_MSG, C_INK, 8, 4, 1, 32);
    Rect(WIN_MSG, C_INK, LCD_W - 9, 4, 1, 32);
    Rect(WIN_MSG, C_LCD, 6, 2, 2, 1);   // (the rounded corners)
    Rect(WIN_MSG, C_LCD, LCD_W - 8, 2, 2, 1);
    Rect(WIN_MSG, C_LCD, 6, 37, 2, 1);
    Rect(WIN_MSG, C_LCD, LCD_W - 8, 37, 2, 1);
    Rect(WIN_MSG, C_LCD, 6, 3, 1, 1);
    Rect(WIN_MSG, C_LCD, LCD_W - 7, 3, 1, 1);
    Rect(WIN_MSG, C_LCD, 6, 36, 1, 1);
    Rect(WIN_MSG, C_LCD, LCD_W - 7, 36, 1, 1);
    CopyWindowToVram(WIN_MSG, COPYWIN_FULL);
}

static void MainBegin(void)
{
    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(C_LCD));
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
    u8 i, x;
    if (sG->phone != PHONE_LIST)   // a call: who, and its timer
    {
        u8 buf[12], *p;
        u16 s = sG->callFrames / 60;
        DrawIcon(WIN_MAIN, CARD_PHONE, 100, 6, C_INK);
        PrintCentered(WIN_MAIN, FONT_NORMAL, sG->cursor == 0 ? sText_MomName : sG->cursor == 1 ? sText_ElmName
                                           : MP_QzPhone_Name(sG->contacts[sG->cursor - 2]), 24, sColLcd);
        p = ConvertIntToDecimalStringN(buf, s / 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        *p++ = CHAR_COLON;
        ConvertIntToDecimalStringN(p, s % 60, STR_CONV_MODE_LEADING_ZEROS, 2);
        PrintCentered(WIN_MAIN, FONT_NORMAL, sG->callFrames < 60 ? sText_Calling : buf, 44, sColLcd);
        return;
    }
    for (i = 0; i < 4; i++)   // (MOM, PROF. ELM, then JOHTO's numbers as Crystal's: CLASS and name; four rows show)
    {
        u8 y = 4 + i * 16, r = sG->top + i;
        const u8 *colors = r == sG->cursor ? sColInv : sColLcd;
        if (r == sG->cursor)
            Rect(WIN_MAIN, C_DARK, 6, y, LCD_W - 12, 15);
        if (r == 0)
            Print(WIN_MAIN, FONT_NORMAL, sText_Mom, 12, y, colors);
        else if (r == 1)
            Print(WIN_MAIN, FONT_NORMAL, sText_Elm, 12, y, colors);
        else if (r < 2 + sG->nContacts)
        {
            Print(WIN_MAIN, FONT_SMALL, MP_QzPhone_ClassName(sG->contacts[r - 2]), 12, y + 2, colors);
            PrintRight(WIN_MAIN, FONT_NORMAL, MP_QzPhone_Name(sG->contacts[r - 2]), LCD_W - 14, y, colors);
        }
        else
            Print(WIN_MAIN, FONT_NORMAL, sText_Empty, 12, y, colors);
        if (i < 3)
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
    Rect(WIN_MAIN, C_DARK, 8, 2, LCD_W - 16, 40);   // the dial
    for (f = 0; f < 5; f++)   // (the MHz, over their ticks)
        Print(WIN_MAIN, FONT_SMALL, sText_Dial[f], FreqX(f * 10) - GetStringWidth(FONT_SMALL, sText_Dial[f], 0) / 2, 2, sColDial);
    for (f = 0; f <= FREQ_MAX; f++)
        Rect(WIN_MAIN, C_WHITE, FreqX(f), f % 10 == 0 ? 14 : 17, 1, f % 10 == 0 ? 6 : 3);
    Rect(WIN_MAIN, C_WHITE, FreqX(0), 20, FreqX(FREQ_MAX) - FreqX(0) + 1, 1);
    Rect(WIN_MAIN, C_RED, FreqX(sG->freq) - 1, 10, 3, 18);   // the needle
    p = ConvertIntToDecimalStringN(buf, sG->freq / 2, STR_CONV_MODE_LEFT_ALIGN, 2);
    *p++ = CHAR_PERIOD;
    *p++ = CHAR_0 + (sG->freq % 2) * 5;
    *p = EOS;
    Print(WIN_MAIN, FONT_SMALL, buf, 16, 24, sColDial);
    PrintRight(WIN_MAIN, FONT_SMALL, sText_Tuning, LCD_W - 14, 24, sColDial);
    Rect(WIN_MAIN, C_INK, 40, 46, LCD_W - 80, 22);   // the station's box
    Rect(WIN_MAIN, C_LCD, 41, 47, LCD_W - 82, 20);
    PrintCentered(WIN_MAIN, FONT_NORMAL, sStations[sG->station].name, 49, sColLcd);
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
        if (sG->station != 0 && sG->lines[1][0] != EOS)
            DrawMsg(sG->lines[0], sG->lines[1]);
        else
            DrawMsg(sMsg_Tune, NULL);
        DrawKeys(NULL, sKey_Exit, sKey_Tune);
        break;
    }
    Sides(WIN_MAIN, 72);   // (last: over any text's box)
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

// A JOHTO contact (Crystal's callee script: mp_qzphone.c): its words wrapped to the message box, a page per paragraph
// break the script gives (\p), two lines a page
static void CallContact(u8 c)
{
    static u8 sWords[0x600];
    u8 line[64];
    u16 i = 0, n = 0;
    MP_QzPhone_Call(c, sWords, sizeof(sWords));
    memset(sG->lines, EOS, sizeof(sG->lines));
    while (sWords[i] != EOS && n < ARRAY_COUNT(sG->lines))
    {
        u16 len = 0, lastSpace = 0, start = i;
        // as many words as fit LCD_W - 28 pixels (the box's inside)
        while (sWords[i] != EOS && sWords[i] != CHAR_PROMPT_CLEAR && len < sizeof(line) - 1)
        {
            line[len] = sWords[i];
            line[len + 1] = EOS;
            if (GetStringWidth(FONT_NORMAL, line, 0) > LCD_W - 28 || len >= 38)
                break;
            if (sWords[i] == CHAR_SPACE)
                lastSpace = len;
            len++;
            i++;
        }
        if (sWords[i] != EOS && sWords[i] != CHAR_PROMPT_CLEAR && lastSpace > 0)   // (back to the last space)
        {
            i = start + lastSpace + 1;
            len = lastSpace;
        }
        memcpy(sG->lines[n], line, len);
        sG->lines[n][len] = EOS;
        n++;
        if (sWords[i] == CHAR_PROMPT_CLEAR)   // (a new page: the next line starts one)
        {
            i++;
            n += n % 2;
        }
    }
    sG->pages = (n + 1) / 2;
    sG->page = 0;
    sG->phone = PHONE_CALL;
}

static void PhoneBook(void)
{
    sG->nContacts = MP_QzPhone_Numbers(sG->contacts, ARRAY_COUNT(sG->contacts));
}

// ---------------------------------------------------------------------------------------------------- radio
// Crystal's stations (pokecrystal engine/pokegear/pokegear.asm RadioChannels, engine/pokegear/radio.asm), their songs
// Crystal's own (tools/kaisong/crystal_radio.py) and their words Crystal's (data/text/common_1.asm), a line at a time
// scrolling up the message box as Crystal's radio prints them
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
static const u8 sBen1[] = _("BEN: POKéMON MUSIC");
static const u8 sBen2[] = _("CHANNEL!");
static const u8 sBen3[] = _("It’s me, DJ BEN!");
static const u8 sFern1[] = _("FERN: POKéMUSIC!");
static const u8 sFern2[] = _("With DJ FERN!");
static const u8 sBenFern1[] = _("Today’s {STR_VAR_1},");
static const u8 sBenFern2A[] = _("so let us jam to");
static const u8 sBenFern2B[] = _("so chill out to");
static const u8 sBenFern3A[] = _("POKéMON March!");
static const u8 sBenFern3B[] = _("POKéMON Lullaby!");
static const u8 sLucky[][24] = {
    _("REED: Yeehaw! How"), _("y’all doin’ now?"),
    _("Whether you’re up"), _("or way down low,"),
    _("don’t you miss the"), _("LUCKY NUMBER SHOW!"),
    _("This week’s Lucky"), _("Number is {STR_VAR_1}!"),
    _("I’ll repeat that!"), _("This week’s Lucky"),
    _("Number is {STR_VAR_1}!"), _("Match it and go to"),
    _("the RADIO TOWER!"),
};
static const u8 sLuckyDrag[][24] = {_("…Repeating myself"), _("gets to be a drag…")};
static const u8 sBuena[][24] = {
    _("BUENA: BUENA here!"), _("Today’s password!"),
    _("Let me think… It’s"), _("HELP!"),   // (the password BUENA asks for, RADIO TOWER 2F)
    _("Don’t forget it!"), _("I’m in GOLDENROD’s"),
    _("RADIO TOWER!"),
};
static const u8 sBuenaMidnight[][24] = {
    _("BUENA: Oh my…"), _("It’s midnight! I"),
    _("have to shut down!"), _("Thanks for tuning"),
    _("in to the end! But"), _("don’t stay up too"),
    _("late! Presented to"), _("you by DJ BUENA!"),
    _("I’m outta here!"), _("…"),
};
static const u8 sRocket[][24] = {
    _("… …Ahem, we are"), _("TEAM ROCKET!"),
    _("After three years"), _("of preparation, we"),
    _("have risen again"), _("from the ashes!"),
    _("GIOVANNI! Can you"), _("hear? We did it!"),
    _("Where is our boss?"), _("Is he listening?"),
};
static const u8 sPnP[][24] = {
    _("PLACES AND PEOPLE!"), _("Brought to you by"), _("me, DJ LILY!"),
};
static const u8 sPnPAdjectives[][24] = {
    _("is cute."), _("is sort of lazy."), _("is always happy."),
    _("is quite noisy."), _("is precocious."), _("is somewhat bold."),
    _("is too picky!"), _("is sort of OK."), _("is just so-so."),
    _("is actually great."), _("is just my type."), _("is so cool, no?"),
    _("is inspiring!"), _("is kind of weird."), _("is right for me?"),
    _("is definitely odd!"),
};

static bool8 RocketsInRadioTower(void)   // (TEAM ROCKET's takeover: after MAHOGANY's hideout, until the last EXECUTIVE)
{
    return VarGet(VAR_QZ_0x500A) >= 6 && VarGet(VAR_QZ_0x500A) < 10;
}

// the station on the dial at FREQ here and now (Crystal: JOHTO's stations in JOHTO, KANTO's out of it)
static u8 StationAt(u8 freq)
{
    bool8 johto = MP_Region_Current() == REGION_JOHTO;
    u16 sec = gMapHeader.regionMapSectionId;
    RtcCalcLocalTime();
    switch (freq)
    {
    case FREQ_TALK:   // the POKéDEX SHOW in the morning
        return !johto ? ST_NONE : gLocalTime.hours >= 4 && gLocalTime.hours < 10 ? ST_DEX : ST_TALK;
    case 15:   // 7.5
        return johto ? ST_MUSIC : ST_NONE;
    case 17:   // 8.5
        return johto ? ST_LUCKY : ST_NONE;
    case 21:   // 10.5: from 6 PM to midnight
        return johto && gLocalTime.hours >= 18 ? ST_BUENA : ST_NONE;
    case 27:   // 13.5
        return sec == MAPSEC_JOHTO_RUINS_OF_ALPH ? ST_UNOWN : ST_NONE;
    case 33:   // 16.5
        return johto ? ST_NONE : ST_PLACES;
    case 37:   // 18.5
        return johto ? ST_NONE : ST_SING;
    case FREQ_FLUTE:
        return johto ? ST_NONE : ST_FLUTE;
    case FREQ_MAX:   // 20.5: around MAHOGANY while TEAM ROCKET is there (from the LAKE OF RAGE's red GYARADOS on)
        if (VarGet(VAR_QZ_0x500C) >= 2 && VarGet(VAR_QZ_0x500A) < 6
         && (sec == MAPSEC_JOHTO_MAHOGANY_TOWN || sec == MAPSEC_JOHTO_ROUTE43 || sec == MAPSEC_JOHTO_LAKE_OF_RAGE))
            return ST_EVOLUTION;
        return ST_NONE;
    }
    return ST_NONE;
}

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

// a POKéMON the player has caught (Crystal's POKéDEX SHOW), its POKéDEX entry
static bool8 PickDexMon(void)
{
    u32 tries;
    for (tries = 0; tries < 4000; tries++)
    {
        u16 num = 1 + Random() % NATIONAL_DEX_COUNT;
        if (GetSetPokedexFlag(num, FLAG_GET_CAUGHT))
        {
            sG->talkSpecies = NationalPokedexNumToSpecies(num);
            sG->dexText = gPokedexEntries[num].description;
            return TRUE;
        }
    }
    return FALSE;
}

// the next line of the POKéDEX entry (it breaks at its own line ends)
static void DexLine(u8 *dst)
{
    u32 n = 0;
    while (*sG->dexText != EOS && *sG->dexText != CHAR_NEWLINE && n < 38)
        dst[n++] = *sG->dexText++;
    dst[n] = EOS;
    if (*sG->dexText == CHAR_NEWLINE)
        sG->dexText++;
}

// a JOHTO or KANTO trainer, "CLASS NAME" (Crystal's PLACES AND PEOPLE)
static void PickPerson(u8 *dst)
{
    u32 tries;
    for (tries = 0; tries < 200; tries++)
    {
        const struct Trainer *t = &gTrainers[FIRST_QZ_TRAINER + Random() % QZ_TRAINERS_COUNT];
        if (t->trainerName[0] != EOS && gTrainerClassNames[t->trainerClass][0] != EOS)
        {
            u8 *p = StringCopy(dst, gTrainerClassNames[t->trainerClass]);
            *p++ = CHAR_SPACE;
            StringCopy(p, t->trainerName);
            return;
        }
    }
    StringCopy(dst, sText_Johto);
}

// a new line under the last: Crystal's radio scrolls a line at a time
static void Say(const u8 *line)
{
    StringCopy(sG->lines[0], sG->lines[1]);
    StringExpandPlaceholders(sG->lines[1], line);
}

// the show's next line (sG->talkStep counts them); FALSE: the show has nothing more to say for now
static bool8 ShowLine(void)
{
    u16 step = sG->talkStep++;
    bool8 lullaby = (gLocalTime.days % 7) & 1;   // (Mondays, Wednesdays, Fridays)
    switch (sG->show)
    {
    case ST_TALK:   // the greeting, then OAK and MARY in turn, a new POKéMON each time round
        if (step < 2)
            Say(step == 0 ? sOpt_Intro1 : sOpt_Intro2);
        else switch ((step - 2) % 5)
        {
        case 0:
            PickTalkMon();
            GetSpeciesName(gStringVar1, sG->talkSpecies);
            Say(sOpt_Oak1);
            break;
        case 1:
            StringCopy(gStringVar2, sG->talkPlace);
            Say(sOpt_Oak2);
            break;
        case 2:
            GetSpeciesName(gStringVar1, sG->talkSpecies);
            Say(sOpt_Mary1);
            break;
        case 3: Say(sOpt_Adverbs[Random() % ARRAY_COUNT(sOpt_Adverbs)]); break;
        case 4: Say(sOpt_Adjectives[Random() % ARRAY_COUNT(sOpt_Adjectives)]); break;
        }
        return TRUE;
    case ST_DEX:   // a caught POKéMON's name, then its entry
        if (step == 0 || *sG->dexText == EOS)
        {
            if (!PickDexMon())
                return FALSE;
            GetSpeciesName(gStringVar1, sG->talkSpecies);
            Say(gStringVar1);
        }
        else
        {
            u8 line[40];
            DexLine(line);
            Say(line);
        }
        return TRUE;
    case ST_MUSIC:   // BEN's greeting (FERN's on LET'S ALL SING), the day and its song; then the music
    case ST_SING:
        if (step >= 6 || (sG->show == ST_SING && step >= 5))
            return FALSE;
        if (sG->show == ST_SING && step >= 2)
            step++;
        switch (step)
        {
        case 0: Say(sG->show == ST_SING ? sFern1 : sBen1); break;
        case 1: Say(sG->show == ST_SING ? sFern2 : sBen2); break;
        case 2: Say(sBen3); break;
        case 3:
            StringCopy(gStringVar1, sText_Days[gLocalTime.days % 7]);
            Say(sBenFern1);
            break;
        case 4: Say(lullaby ? sBenFern2B : sBenFern2A); break;
        case 5: Say(lullaby ? sBenFern3B : sBenFern3A); break;
        }
        return TRUE;
    case ST_LUCKY:   // REED and this week's number (the LOTTERY CORNER's), now and then a sigh
        if (step == 0)
            sG->drag = (Random() & 0xFF) == 0;
        if (step >= ARRAY_COUNT(sLucky) + (sG->drag ? 2 : 0))
        {
            sG->talkStep = 0;
            return ShowLine();
        }
        ConvertIntToDecimalStringN(gStringVar1, GetLotteryNumber() % 100000, STR_CONV_MODE_LEADING_ZEROS, 5);
        Say(step < ARRAY_COUNT(sLucky) ? sLucky[step] : sLuckyDrag[step - ARRAY_COUNT(sLucky)]);
        return TRUE;
    case ST_BUENA:   // BUENA's password, over and over until midnight
        RtcCalcLocalTime();
        if (gLocalTime.hours < 18)
        {
            if (sG->midnight >= ARRAY_COUNT(sBuenaMidnight))
                return FALSE;
            Say(sBuenaMidnight[sG->midnight++]);
            return TRUE;
        }
        Say(sBuena[step % ARRAY_COUNT(sBuena)]);
        return TRUE;
    case ST_PLACES:   // DJ LILY on a JOHTO or KANTO trainer or place
        if (step < ARRAY_COUNT(sPnP))
            Say(sPnP[step]);
        else if ((step - ARRAY_COUNT(sPnP)) % 2 == 0)
        {
            if (Random() % 100 < 49)
                PickPerson(gStringVar1);
            else
            {
                PickTalkMon();
                StringCopy(gStringVar1, sG->talkPlace);
            }
            Say(gStringVar1);
        }
        else
        {
            Say(sPnPAdjectives[Random() % ARRAY_COUNT(sPnPAdjectives)]);
            if (Random() % 100 < 4)   // (and the show from the top)
                sG->talkStep = 0;
        }
        return TRUE;
    case ST_ROCKET:
        Say(sRocket[step % ARRAY_COUNT(sRocket)]);
        return TRUE;
    }
    return FALSE;   // (the UNOWN, the POKé FLUTE, the EVOLUTION radio: only sounds)
}

// the dial at FREQ: on a station, its music and its show; between them the map's music
static void TuneTo(u8 freq)
{
    u8 station = StationAt(freq), show = station;
    u16 song;
    sG->freq = freq;
    if (station != ST_NONE && freq <= 21 && MP_Region_Current() == REGION_JOHTO && RocketsInRadioTower())
        show = ST_ROCKET;   // (TEAM ROCKET on every JOHTO station)
    if (station == sG->station && show == sG->show)
        return;
    sG->station = station;
    sG->show = show;
    sG->talkStep = 0;
    sG->talkTimer = 0;
    sG->midnight = 0;
    sG->lines[0][0] = sG->lines[1][0] = EOS;
    song = sStations[show].song;
    if (show == ST_MUSIC || show == ST_SING)
        song = (gLocalTime.days % 7) & 1 ? MUS_JOHTO_RADIO_POKEMON_LULLABY : MUS_JOHTO_RADIO_POKEMON_MARCH;
    PlayBGM(station == ST_NONE ? sG->song : song);
    if (station != ST_NONE)
        ShowLine();
}

// ------------------------------------------------------------------------------------------------- on the phone
// Moemon Co-op: the phone's POKéGEAR (mp_phone_dev.c, docs/PHONE_LINK.md): the same calls and radio shows, with a state
// of their own (never while the game's own POKéGEAR is open: its state is in use then)
static void *sPhoneState;

static bool8 PhoneBegin(void)
{
    if (sG != NULL)
        return FALSE;
    if (sPhoneState == NULL)
        sPhoneState = calloc(1, sizeof(*sG));
    if (sPhoneState == NULL)
        return FALSE;
    sG = sPhoneState;
    return TRUE;
}

static u8 PhoneLines(u8 out[][40], u8 max)
{
    u8 i, n = 0;
    for (i = 0; i < ARRAY_COUNT(sG->lines) && i < sG->pages * 2 && n < max; i++)
        memcpy(out[n++], sG->lines[i], 40);
    sG = NULL;
    return n;
}

// A call (0 MOM, 1 PROF. ELM): its lines (two to a page); *ask: MOM's savings question follows (answer below). 0: busy
u8 MP_Pokegear_PhoneCall(u8 who, u8 out[][40], u8 max, bool8 *ask)
{
    if (!PhoneBegin())
        return 0;
    if (who == 0)
        CallMom();
    else
        CallElm();
    *ask = sG->phone == PHONE_ASK;
    return PhoneLines(out, max);
}

u8 MP_Pokegear_PhoneAnswer(bool8 yes, u8 out[][40], u8 max)
{
    if (!PhoneBegin())
        return 0;
    MomAnswer(yes);
    return PhoneLines(out, max);
}

// The radio at FREQ (0-41, half a step each: 9 is 4.5): the station's name (NULL: no signal), its song, and the show's
// next line (asked again each second and a half, as the game's radio prints them; "" when the show is quiet)
bool8 MP_Pokegear_PhoneRadio(u8 freq, const u8 **name, u16 *song, u8 line[40])
{
    u8 station, show;
    if (!PhoneBegin())
        return FALSE;
    RtcCalcLocalTime();
    freq = freq > FREQ_MAX ? FREQ_MAX : freq;
    station = StationAt(freq);
    show = station;
    if (station != ST_NONE && freq <= 21 && MP_Region_Current() == REGION_JOHTO && RocketsInRadioTower())
        show = ST_ROCKET;
    line[0] = EOS;
    if (station != sG->station || show != sG->show || freq != sG->freq)
    {
        sG->freq = freq;
        sG->station = station;
        sG->show = show;
        sG->talkStep = 0;
        sG->midnight = 0;
        sG->lines[0][0] = sG->lines[1][0] = EOS;
    }
    *song = station == ST_NONE ? 0 : sStations[show].song;
    if (show == ST_MUSIC || show == ST_SING)
        *song = (gLocalTime.days % 7) & 1 ? MUS_JOHTO_RADIO_POKEMON_LULLABY : MUS_JOHTO_RADIO_POKEMON_MARCH;
    *name = station == ST_NONE ? NULL : sStations[station].name;
    if (station != ST_NONE)
    {
        sG->lines[1][0] = EOS;
        if (ShowLine())
            memcpy(line, sG->lines[1], 40);
    }
    sG = NULL;
    return TRUE;
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
    if (sG->card == CARD_RADIO && sG->station != 0 && ++sG->talkTimer >= 100)   // (a line every second and a half)
    {
        sG->talkTimer = 0;
        if (ShowLine())
            DrawAll();
        else if (sG->show == ST_BUENA)   // (BUENA's gone at midnight: off the air)
        {
            TuneTo(sG->freq);
            DrawAll();
        }
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
        if (sG->card == CARD_RADIO && !FlagGet(FLAG_JOHTO_RADIO_CARD))   // (no RADIO CARD yet: Crystal's)
            sG->card = (sG->card + (JOY_NEW(DPAD_RIGHT) ? 1 : CARD_COUNT - 1)) % CARD_COUNT;
        sG->cursor = sG->top = 0;
        PhoneBook();
        sG->phone = PHONE_LIST;
        if (sG->station != 0)   // (the radio plays while its card is open)
        {
            sG->station = sG->show = 0;
            PlayBGM(sG->song);
        }
        if (sG->card == CARD_RADIO)
            TuneTo(sG->freq);
        DrawAll();
    }
    else if (sG->card == CARD_RADIO && (JOY_REPEAT(DPAD_UP) || JOY_REPEAT(DPAD_DOWN)))
    {
        if (JOY_REPEAT(DPAD_UP) && sG->freq < FREQ_MAX)
            TuneTo(sG->freq + 1);
        else if (JOY_REPEAT(DPAD_DOWN) && sG->freq > 0)
            TuneTo(sG->freq - 1);
        DrawAll();
    }
    else if (sG->card == CARD_PHONE && JOY_REPEAT(DPAD_UP) && sG->cursor > 0)
    {
        sG->cursor--, PlaySE(SE_SELECT);
        if (sG->cursor < sG->top)
            sG->top = sG->cursor;
        DrawAll();
    }
    else if (sG->card == CARD_PHONE && JOY_REPEAT(DPAD_DOWN) && sG->cursor < (sG->nContacts > 2 ? sG->nContacts + 1 : 3))
    {
        sG->cursor++, PlaySE(SE_SELECT);
        if (sG->cursor >= sG->top + 4)
            sG->top = sG->cursor - 3;
        DrawAll();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (sG->card == CARD_MAP)
        {
            PlaySE(SE_SELECT);
            sReturnCard = CARD_MAP;
            Leave(taskId, CB2_OpenPokegearAgain);
        }
        else if (sG->card == CARD_PHONE && sG->cursor < 2 + sG->nContacts)   // (the empty rows have no number)
        {
            PlaySE(SE_POKENAV_CALL);
            sG->callFrames = 0;
            if (sG->cursor == 0)
                CallMom();
            else if (sG->cursor == 1)
                CallElm();
            else
                CallContact(sG->contacts[sG->cursor - 2]);
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
    ChangeBgX(0, 0, BG_COORD_SET);   // (the field's scroll would shift the screen)
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
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
    if (sG->card == CARD_RADIO && !FlagGet(FLAG_JOHTO_RADIO_CARD))
        sG->card = CARD_CLOCK;
    PhoneBook();
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

// Tests (mp pokegear [open | tune FREQ]): the card, the cursor, the phone and the radio
void MP_Pokegear_Debug(const char *arg)
{
    if (arg != NULL && strncmp(arg, "open", 4) == 0 && sG == NULL)
    {
        sToField = TRUE;
        SetMainCallback2(CB2_MP_OpenPokegearToField);
        return;
    }
    if (arg != NULL && strncmp(arg, "tune ", 5) == 0 && sG != NULL)
    {
        if (sG->card != CARD_RADIO)
            sG->card = CARD_RADIO;
        TuneTo(atoi(arg + 5));
        DrawAll();
    }
    if (sG == NULL)
        printf("[test] pokegear: closed\n");
    else
    {
        u8 line[48];
        u32 i;
        for (i = 0; i < sizeof(sG->lines[1]) && sG->lines[1][i] != EOS; i++)   // (the radio's last line, in ASCII)
        {
            u8 c = sG->lines[1][i];
            line[i] = c == CHAR_SPACE ? ' ' : c >= CHAR_A && c <= CHAR_Z ? 'A' + c - CHAR_A
                    : c >= CHAR_a && c <= CHAR_z ? 'a' + c - CHAR_a : c >= CHAR_0 && c <= CHAR_9 ? '0' + c - CHAR_0 : '.';
        }
        line[i] = 0;
        u16 song = MP_Kaisong_SongOf(gMPlayInfo_BGM.songHeader);
        printf("[test] pokegear: card %u cursor %u phone %u page %u/%u freq %u station %u show %u song %u%s line \"%s\"\n",
               sG->card, sG->cursor, sG->phone, sG->page, sG->pages, sG->freq, sG->station, sG->show, song,
               song != 0 && song <= MUS_JOHTO_RADIO_ROCKET && gMPlayInfo_BGM.songHeader != gSongTable[song].header
               ? " (kaisong)" : "", line);
    }
}
