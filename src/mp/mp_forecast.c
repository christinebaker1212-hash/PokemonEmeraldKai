// Moemon Co-op: CO-OP > ME > FORECAST (the user, 2026-11-24: "a weather forecast to the CO-OP menu, to see what the
// weather will be like all day, and for the next 7 days"; the mock-up docs/coop_menu/forecast_mockup.png, approved).
//
// The hourly weather (mp_weather.c) is worked out from the hour alone, so it can be read ahead: today's 24 hours as
// icons (the night's on a pale band, now boxed), then the next seven days by their night, morning, afternoon and
// evening with a word, for a place of the region: where the player is, LEFT / RIGHT going round the region's towns and
// routes. UP / DOWN picks TODAY or a day and the box below says it in words. A place with weather of its own (ROUTE
// 119's rain, a desert's sandstorm, UNOVA's calendar) shows that all day; story events' weather can't be foreseen.
//
// Drawn in the CO-OP menu's own window (MpUi), shown in BG palette 13 for the time it's open: the theme's colours
// with ten of the weather's in the slots the screen doesn't use (the theme's own come back as it closes).
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_weather.h"
#include "task.h"
#include "main.h"
#include "palette.h"
#include "window.h"
#include "text.h"
#include "sound.h"
#include "rtc.h"
#include "overworld.h"
#include "region_map.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "string_util.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "../data/mp_map_group_sizes.h"

#define FC_PAL 13
#define PLACES_MAX 200
#define ROWS 8   // TODAY, then seven days

// the weather's colours, in the slots the screen leaves free of the theme's (MPT_BG 1, TEXT 2, SHADOW 3, RED 4,
// GREY 13 stay)
enum { C_O = 5, C_Y, C_W, C_C, C_G, C_D, C_B, C_I, C_T = 14, C_N = 15 };
static const u16 sWxColors[][2] = {
    {C_O, RGB(30, 17, 5)},  {C_Y, RGB(31, 27, 8)}, {C_W, RGB(31, 31, 25)}, {C_C, RGB(27, 28, 29)},
    {C_G, RGB(17, 18, 20)}, {C_D, RGB(12, 13, 15)}, {C_B, RGB(8, 17, 29)},  {C_I, RGB(25, 29, 31)},
    {C_T, RGB(27, 22, 14)}, {C_N, RGB(28, 28, 30)},
};

enum { IC_SUN, IC_FAIR, IC_MOON, IC_RAIN, IC_STORM, IC_HAIL, IC_SAND, IC_FOG };
static const char *const sIcons[][8] = {
    [IC_SUN]   = {"o..oo..o", "...yy...", "..yyyy..", "oyywyyyo", "oyyyyyyo", "..yyyy..", "...yy...", "o..oo..o"},
    [IC_FAIR]  = {"...o....", ".o.yy.o.", "..yyyy..", ".oyycgg.", "..ygcccg", ".gcccccc", "gcccccc.", ".gggggg."},
    [IC_MOON]  = {"..yww...", ".yww....", "yww.....", "yww.....", "yww.....", "ywww...y", ".ywwwwy.", "..yyyy.."},
    [IC_RAIN]  = {"..ggg...", ".gcccgg.", "gccccccc", "gggggggg", ".b..b..b", "b..b..b.", ".b..b..b", "........"},
    [IC_STORM] = {"..ddd...", ".dgggdd.", "dggggggd", "dddydddd", ".b.yy.b.", "b..y..b.", "..y...b.", "........"},
    [IC_HAIL]  = {"..ggg...", ".gcccgg.", "gccccccg", "gggggggg", ".i..i..i", "........", "i..i..i.", "........"},
    [IC_SAND]  = {"........", ".ttttg..", "t.....g.", "..tttg.g", ".g....t.", "g.ttgg..", ".t......", "..tgg..."},
    [IC_FOG]   = {"........", ".gggggg.", "........", "gggggg..", "........", "..gggggg", "........", ".gggggg."},
};

static struct
{
    u8 taskId;
    u16 cursor;
    u16 place, count;
    u32 drawnHour;
    struct { u8 group, num; u16 mapsec; } places[PLACES_MAX];
    u16 savedPal[2][16];
} sFc = {.taskId = TASK_NONE};

// ---------------------------------------------------------------------------------------------------- the places
static bool8 OutdoorType(u8 type)
{
    return type == MAP_TYPE_ROUTE || type == MAP_TYPE_TOWN || type == MAP_TYPE_CITY || type == MAP_TYPE_OCEAN_ROUTE;
}

// The region's towns and routes, one map a place (by its map section), in the Town Map's order
static void BuildPlaces(void)
{
    u8 region = MP_Region_Current(), g;
    u16 n, i, j;
    sFc.count = 0;
    for (g = 0; g < MAP_GROUPS_COUNT; g++)
    {
        for (n = 0; n < sMapGroupSizes[g] && sFc.count < PLACES_MAX; n++)
        {
            const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(g, n);
            u16 sec;
            if (h == NULL || !OutdoorType(h->mapType) || MP_Region_IsSharedMap(g, n) || MP_Region_OfGroup(g) != region)
                continue;
            sec = h->regionMapSectionId;
            if (sec == MAPSEC_NONE)
                continue;
            for (i = 0; i < sFc.count && sFc.places[i].mapsec != sec; i++)
                ;
            if (i < sFc.count)
                continue;
            // (kept in map-section order)
            for (j = sFc.count; j > 0 && sFc.places[j - 1].mapsec > sec; j--)
                sFc.places[j] = sFc.places[j - 1];
            sFc.places[j].group = g, sFc.places[j].num = (u8)n, sFc.places[j].mapsec = sec;
            sFc.count++;
        }
    }
}

// Where the player is: the place of the map's section (a house in a town is the town), else the first
static u16 PlaceHere(void)
{
    const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup,
                                                                   gSaveBlock1Ptr->location.mapNum);
    u16 i;
    for (i = 0; h != NULL && i < sFc.count; i++)
        if (sFc.places[i].mapsec == h->regionMapSectionId)
            return i;
    return 0;
}

// ---------------------------------------------------------------------------------------------------- the sky
static u32 HourZero(u8 *localNow)
{
    RtcCalcLocalTime();
    *localNow = gLocalTime.hours;
    return MP_Weather_Hour() - gLocalTime.hours;   // (the UTC hour of today's 12 AM, by the game's clock)
}

static u8 SkyAt(u16 place, u32 hour, u8 local, bool8 *own)
{
    if (place >= sFc.count)
    {
        *own = TRUE;
        return MP_WX_CLEAR;
    }
    return MP_Weather_Forecast(sFc.places[place].group, sFc.places[place].num, hour, local % 24, own);
}

static u8 IconOf(u8 wx, bool8 night)
{
    switch (wx)
    {
    case MP_WX_RAIN:  return IC_RAIN;
    case MP_WX_STORM: return IC_STORM;
    case MP_WX_SUN:   return IC_SUN;
    case MP_WX_HAIL:  return IC_HAIL;
    case MP_WX_SAND:  return IC_SAND;
    case MP_WX_FOG:   return IC_FOG;
    default:          return night ? IC_MOON : IC_FAIR;
    }
}

// A part of a day (six hours from START): what stands out in it
static u8 PartOf(u16 place, u32 day0, u8 start)
{
    u8 n[MP_WX_COUNT] = {0}, h;
    bool8 own;
    for (h = start; h < start + 6; h++)
        n[SkyAt(place, day0 + h, h, &own)]++;
    if (n[MP_WX_STORM])
        return MP_WX_STORM;
    if (n[MP_WX_HAIL] >= 2)
        return MP_WX_HAIL;
    if (n[MP_WX_SAND])
        return MP_WX_SAND;
    if (n[MP_WX_FOG] >= 3)
        return MP_WX_FOG;
    if (n[MP_WX_RAIN] >= 2)
        return MP_WX_RAIN;
    if (n[MP_WX_SUN] >= 3)
        return MP_WX_SUN;
    return MP_WX_CLEAR;
}

static bool8 Wet(u8 wx) { return wx == MP_WX_RAIN || wx == MP_WX_STORM; }

// A day's word, from its night, morning, afternoon and evening
static const char *DayWord(const u8 *p, bool8 own)
{
    u8 i, wet = 0, sun = 0;
    for (i = 0; i < 4; i++)
    {
        if (p[i] == MP_WX_STORM)
            return own ? "STORMY" : "STORMS";
        wet += Wet(p[i]), sun += p[i] == MP_WX_SUN;
    }
    for (i = 0; i < 4; i++)
        if (p[i] == MP_WX_HAIL)
            return "HAIL";
    for (i = 0; i < 4; i++)
        if (p[i] == MP_WX_SAND)
            return "SANDSTORM";
    for (i = 0; i < 4; i++)
        if (p[i] == MP_WX_FOG)
            return "FOGGY";
    if (wet >= 3)
        return "RAINY";
    if (wet)
    {
        if (Wet(p[1]) && !Wet(p[0]) && !Wet(p[2]) && !Wet(p[3]))
            return "MORNING SHOWERS";
        if (!Wet(p[2]) && !Wet(p[3]))
            return Wet(p[0]) && !Wet(p[1]) ? "NIGHT RAIN" : "CLEARING";
        if (!Wet(p[0]) && !Wet(p[1]))
            return "RAIN LATER";
        return "SHOWERS";
    }
    return sun >= 3 ? "BRIGHT SUN" : sun == 2 ? "SUNNY" : sun ? "SUNNY SPELLS" : "FAIR";
}

static const char *KindWord(u8 wx, bool8 night)
{
    switch (wx)
    {
    case MP_WX_RAIN:  return "rain";
    case MP_WX_STORM: return "a storm";
    case MP_WX_SUN:   return "bright sun";
    case MP_WX_HAIL:  return "hail";
    case MP_WX_SAND:  return "a sandstorm";
    case MP_WX_FOG:   return "fog";
    default:          return night ? "clear" : "fair";
    }
}

static void HourText(char *out, int size, u8 h)
{
    h %= 24;
    snprintf(out, size, "%d %s", h % 12 ? h % 12 : 12, h < 12 ? "AM" : "PM");
}

static void Capital(char *s)
{
    if (s[0] >= 'a' && s[0] <= 'z')
        s[0] -= 32;
}

// TODAY in words: now, then what's coming (the first stretch of rain, sun, hail or sandstorm, and a storm in it)
static void TodayWords(char *out, int size, u16 place, u32 day0, u8 localNow)
{
    bool8 own;
    u8 now = SkyAt(place, day0 + localNow, localNow, &own), h, kind = 0xFF, from = 0, to = 0, storm = 0xFF;
    char a[12], b[12], s[12];
    if (own)
    {
        snprintf(out, size, "Weather of its own here: %s.", KindWord(now, localNow < 6 || localNow >= 20));
        Capital(out + 25);
        return;
    }
    for (h = localNow + 1; h < 24; h++)
    {
        u8 wx = SkyAt(place, day0 + h, h, &own), k = wx == MP_WX_STORM ? MP_WX_RAIN : wx;
        if (kind == 0xFF && k != MP_WX_CLEAR && !(Wet(now) && Wet(wx)) && k != (now == MP_WX_STORM ? MP_WX_RAIN : now))
            kind = k, from = h, to = h;
        else if (kind != 0xFF && k == kind && to == h - 1)
            to = h;
        if (wx == MP_WX_STORM && storm == 0xFF && h > localNow)
            storm = h;
    }
    snprintf(out, size, "%s now.", KindWord(now, localNow < 6 || localNow >= 20));
    Capital(out);
    if (kind != 0xFF)
    {
        int n = strlen(out);
        HourText(a, sizeof(a), from);
        HourText(b, sizeof(b), to + 1);
        if (to == 23)
            snprintf(out + n, size - n, " %s from %s on", KindWord(kind, FALSE), a);
        else
            snprintf(out + n, size - n, " %s from %s to %s", KindWord(kind, FALSE), a, b);
        Capital(out + n + 1);
        n = strlen(out);
        if (storm != 0xFF && kind == MP_WX_RAIN)
        {
            HourText(s, sizeof(s), storm);
            snprintf(out + n, size - n, ", a storm at %s.", s);
        }
        else
            snprintf(out + n, size - n, ".");
    }
    else
    {
        int n = strlen(out);
        snprintf(out + n, size - n, Wet(now) || now == MP_WX_SUN ? " So for the rest of the day." :
                 " Clear for the rest of the day.");
    }
}

// A day in words, part by part
static void DayWords(char *out, int size, const char *dayName, const u8 *p, bool8 own)
{
    static const char *const sPart[4] = {"overnight", "in the morning", "in the afternoon", "in the evening"};
    u8 i, said = 0, first = 0xFF;
    int n;
    snprintf(out, size, "%s: ", dayName);
    n = strlen(out);
    if (own)
    {
        snprintf(out + n, size - n, "weather of its own here, %s.", KindWord(p[1], FALSE));
        Capital(out + n);
        return;
    }
    for (i = 0; i < 4; i++)
        if (p[i] != MP_WX_CLEAR && said < 2)
        {
            n = strlen(out);
            if (said && p[i] == first)
                snprintf(out + n, size - n, " and %s", sPart[i]);
            else
            {
                snprintf(out + n, size - n, "%s%s %s", said ? ", " : "", KindWord(p[i], i == 0), sPart[i]);
                if (!said)
                    Capital(out + n);
                first = p[i];
            }
            said++;
        }
    n = strlen(out);
    if (!said)
        snprintf(out + n, size - n, "Fair all day.");
    else
        snprintf(out + n, size - n, p[3] == MP_WX_CLEAR && p[2] == MP_WX_CLEAR ? ", then fair." : ".");
}

// ---------------------------------------------------------------------------------------------------- drawing
static void Icon(u8 win, int x, int y, u8 icon)
{
    u8 r, c;
    for (r = 0; r < 8; r++)
        for (c = 0; c < 8; c++)
        {
            u8 ch = (u8)sIcons[icon][r][c], ix;
            switch (ch)
            {
            case 'o': ix = C_O; break;
            case 'y': ix = C_Y; break;
            case 'w': ix = C_W; break;
            case 'c': ix = C_C; break;
            case 'g': ix = C_G; break;
            case 'd': ix = C_D; break;
            case 'b': ix = C_B; break;
            case 'i': ix = C_I; break;
            case 't': ix = C_T; break;
            default:  continue;
            }
            FillWindowPixelRect(win, PIXEL_FILL(ix), x + c, y + r, 1, 1);
        }
}

static void Print(u8 win, const char *ascii, int x, int y, u8 fg)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    MpTheme_Print(win, FONT_SMALL, buf, x, y, fg, 0);
}

static int Width(const char *ascii)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    return GetStringWidth(FONT_SMALL, buf, 0);
}

static void Draw(void)
{
    static const char *const sDays[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    static const char *const sDayNames[7] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
    u8 win, localNow, h, d, p[4], arrow[] = {0xEF, EOS}, title[48], name[32];
    u32 day0 = HourZero(&localNow);
    char line[64], words[160], help[200];
    bool8 own;
    int wday;
    {
        time_t t = time(NULL) + (time_t)MP_ClockShift();
        struct tm *tm = localtime(&t);
        wday = tm ? tm->tm_wday : 0;
    }
    MpUi_Begin("FORECAST");
    win = MpUi_MainWindow();
    // the place
    GetMapName(name, sFc.count ? sFc.places[sFc.place].mapsec : MAPSEC_NONE, 0);
    title[0] = 0x85;   // <
    title[1] = CHAR_SPACE;
    StringCopy(title + 2, name);
    {
        u8 *e = title + StringLength(title);
        *e++ = CHAR_SPACE;
        *e++ = 0x86;   // >
        *e = EOS;
    }
    MpUi_TitleRight(title);
    // TODAY: every hour, the night's on a band, now boxed
    if (sFc.cursor == 0)
        MpTheme_Print(win, FONT_SMALL, arrow, 1, 11, MPT_TEXT, 0);
    Print(win, "TODAY", 9, 11, MPT_TEXT);
    {
        u8 now = SkyAt(sFc.place, day0 + localNow, localNow, &own);
        char hs[12];
        HourText(hs, sizeof(hs), localNow);
        snprintf(line, sizeof(line), "NOW %s: %s", hs, own ? "OWN" : "");
        if (!own)
        {
            const char *k = KindWord(now, localNow < 6 || localNow >= 20);
            int n = strlen(line), i;
            for (i = 0; k[i] && n < (int)sizeof(line) - 1; i++)
                line[n++] = (k[i] >= 'a' && k[i] <= 'z') ? k[i] - 32 : k[i];
            line[n] = 0;
        }
        else
            snprintf(line + strlen(line) - 3, 8, "%s", "OWN");
        Print(win, line, 222 - Width(line), 11, MPT_RED);
    }
    for (h = 0; h < 24; h++)
    {
        int cx = 8 + h * 9;
        bool8 night = h < 6 || h >= 20;
        if (night)
            FillWindowPixelRect(win, PIXEL_FILL(C_N), cx, 23, 9, 10);
        if (h == localNow)
        {
            FillWindowPixelRect(win, PIXEL_FILL(C_O), cx, 22, 9, 12);
            FillWindowPixelRect(win, PIXEL_FILL(MPT_BG), cx + 1, 23, 7, 10);
        }
        Icon(win, cx, 24, IconOf(SkyAt(sFc.place, day0 + h, h, &own), night));
    }
    Print(win, "12A", 8, 30, MPT_TEXT);
    Print(win, "6A", 8 + 6 * 9, 30, MPT_TEXT);
    Print(win, "12P", 8 + 12 * 9, 30, MPT_TEXT);
    Print(win, "6P", 8 + 18 * 9, 30, MPT_TEXT);
    // the next seven days
    for (d = 0; d < 7; d++)
    {
        int y = 42 + d * 9;
        u32 dz = day0 + 24 * (d + 1);
        u8 k;
        for (k = 0; k < 4; k++)
            p[k] = PartOf(sFc.place, dz, k * 6);
        SkyAt(sFc.place, dz + 12, 12, &own);
        if (sFc.cursor == d + 1)
            MpTheme_Print(win, FONT_SMALL, arrow, 1, y - 4, MPT_TEXT, 0);
        Print(win, sDays[(wday + d + 1) % 7], 9, y - 4, MPT_TEXT);
        for (k = 0; k < 4; k++)
            Icon(win, 40 + k * 11, y, IconOf(p[k], k == 0));
        Print(win, DayWord(p, own), 88, y - 4, MPT_TEXT);
        if (sFc.cursor == d + 1)
            DayWords(words, sizeof(words), sDayNames[(wday + d + 1) % 7], p, own);
    }
    if (sFc.cursor == 0)
        TodayWords(words, sizeof(words), sFc.place, day0, localNow);
    MpUi_End();
    snprintf(help, sizeof(help), "%s  {LEFTRIGHT} PLACE  {B} BACK", words);
    MpUi_HelpAscii(help);
    sFc.drawnHour = MP_Weather_Hour();
}

// ---------------------------------------------------------------------------------------------------- the screen
static void Close(void)
{
    MpUi_MainPalette(15);
    CpuCopy16(sFc.savedPal[0], &gPlttBufferUnfaded[BG_PLTT_ID(FC_PAL)], PLTT_SIZE_4BPP);
    CpuCopy16(sFc.savedPal[1], &gPlttBufferFaded[BG_PLTT_ID(FC_PAL)], PLTT_SIZE_4BPP);
    MpUi_Close();
    DestroyTask(sFc.taskId);
    sFc.taskId = TASK_NONE;
}

static void Task_Forecast(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Close();
        return;
    }
    if (MpUi_MoveCursor(&sFc.cursor, ROWS))
    {
        Draw();
        return;
    }
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT) && sFc.count > 1)
    {
        sFc.place = (sFc.place + (JOY_NEW(DPAD_LEFT) ? sFc.count - 1 : 1)) % sFc.count;
        PlaySE(SE_SELECT);
        Draw();
        return;
    }
    if (MP_Weather_Hour() != sFc.drawnHour)   // (the hour turning while it's open)
        Draw();
}

void MP_Forecast_Open(void)
{
    u16 pal[16];
    u8 i;
    if (sFc.taskId != TASK_NONE)
        return;
    BuildPlaces();
    sFc.place = PlaceHere();
    sFc.cursor = 0;
    MpUi_Open();
    // the window in its own palette: the theme's colours and the weather's
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(FC_PAL)], sFc.savedPal[0], PLTT_SIZE_4BPP);
    CpuCopy16(&gPlttBufferFaded[BG_PLTT_ID(FC_PAL)], sFc.savedPal[1], PLTT_SIZE_4BPP);
    memcpy(pal, MpTheme_Palette(), sizeof(pal));
    for (i = 0; i < ARRAY_COUNT(sWxColors); i++)
        pal[sWxColors[i][0]] = sWxColors[i][1];
    LoadPalette(pal, BG_PLTT_ID(FC_PAL), PLTT_SIZE_4BPP);
    MpUi_MainPalette(FC_PAL);
    sFc.taskId = CreateTask(Task_Forecast, 80);
    Draw();
}

// The sky where the player is now (the CO-OP menu's row: "RAIN NOW")
u8 MP_Forecast_Now(void)
{
    u8 localNow;
    u32 day0;
    bool8 own;
    BuildPlaces();
    day0 = HourZero(&localNow);
    return SkyAt(PlaceHere(), day0 + localNow, localNow, &own);
}

// Tests (mp forecast [place N] [day D]): the screen, on a place and a row; and the place's week as text
void MP_Forecast_Debug(const char *arg)
{
    int v;
    if (sFc.taskId == TASK_NONE)
        MP_Forecast_Open();
    if (arg && sscanf(arg, "place %d", &v) == 1 && sFc.count)
        sFc.place = (u16)(v % sFc.count);
    else if (arg && sscanf(arg, "day %d", &v) == 1)
        sFc.cursor = (u16)(v % ROWS);
    Draw();
    {
        u8 name[32], localNow, d, k, p[4];
        char a[32];
        u32 day0 = HourZero(&localNow);
        bool8 own;
        GetMapName(name, sFc.count ? sFc.places[sFc.place].mapsec : MAPSEC_NONE, 0);
        MP_GameToAscii(a, name, sizeof(a));
        printf("[forecast] %u places, place %u %s, now %u\n", sFc.count, sFc.place, a, localNow);
        for (d = 0; d < 8; d++)
        {
            for (k = 0; k < 4; k++)
                p[k] = PartOf(sFc.place, day0 + 24 * d, k * 6);
            SkyAt(sFc.place, day0 + 24 * d + 12, 12, &own);
            printf("[forecast] day %u: %u %u %u %u %s\n", d, p[0], p[1], p[2], p[3], DayWord(p, own));
        }
    }
}
