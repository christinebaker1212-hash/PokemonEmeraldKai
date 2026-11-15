// Moemon Kai: UNOVA's gates' electric news boards, Black's ov048 (field_gimmick_gate.c, gimmick_obj_elboard.c) and the
// weather they report (arm9 zone_weather.c, ov010 calender.c). A gate's board is its zone's record (archive 0xAE:
// u3d_elboard.h from tools/unova/gen_elboard.py) placed as Black places it, pack gimmick 48's model; its seven lanes
// are 1024 x 16 textures that a message each is written into (Black's EL_SCOREBOARD_CreateLamp: from 8 characters in,
// letters colour 3 and their shadow colour 2 of the lane's palette, the lamps' palette flickering between orange and
// amber every 16 frames) and scrolled across the board by the lane's texture track; the lines run one after another
// (GIMMICK_OBJ_ELBOARD_UpdateLine: the next starts once this one's text and 3 blank characters have passed). Its
// lines: the date, the weather of up to four places, the three sayings of the day and the week's notice, or the
// story's news (archive 0xA6, by event flag) in the sayings' place. The TV beside it plays the record's picture.
// Its own code, never SINNOH's or ORRE's.
#include <stdlib.h>
#include "global.h"
#include "event_data.h"
#include "rtc.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "mp.h"
#include "u3d.h"
#include "u3d_elboard.h"

u8 MP_RenderSmallGlyph(u16 glyphId, u8 fgColor, u8 bgColor, u8 shadowColor);   // text.c
bool8 U3D_Vm_Flag(u16 id);
void U3D_Vm_ClearFlag(u16 id);
u16 U3D_Vm_RoamerWeather(u16 zone);

#define EL_MODEL 0
#define EL_LANES 7
#define EL_W 1024
#define EL_H 16
#define EL_LEAD 8      // (GateElboardSetup: 8 blank characters before a line, 3 after it)
#define EL_TRAIL 3
#define EL_BANK 38     // (Black's message file 0x26 of archive 2: the boards')
#define EL_PLACES 89   // (its place names)
#define EL_MONTHS 278  // (the months' names, short)
#define BLACK 21       // (a record's version: 0 both, 20 White, 21 Black)

static struct
{
    bool8 on;
    const struct ElBoard *b;
    u8 n, tick;
    struct { u8 lane; bool8 playing, nextStarted, request; float elapsed, next, end; } line[EL_LANES];
    u8 *idx[EL_LANES];        // (each lane's text: Black's palette indices, written in colour as the lamps change)
    u32 *tex[EL_LANES];
    u8 colours;               // (which of Black's lamp palettes the lanes were last written in)
} sEl;

// (EL_SCOREBOARD_LampPaletteColors: the lamps' colours 2 (the letters' shadow) and 3 (the letters), in turn)
static const u8 sLamp[4][2][3] = {{{19, 11, 3}, {31, 16, 0}}, {{19, 11, 3}, {31, 16, 0}},
                                  {{19, 15, 3}, {31, 24, 0}}, {{19, 15, 3}, {31, 24, 0}}};

static u32 Rgba(const u8 c[3])
{
    return (u32)(c[0] * 255 / 31) | (u32)(c[1] * 255 / 31) << 8 | (u32)(c[2] * 255 / 31) << 16 | 0xFFu << 24;
}

static void Paint(u8 lane, u8 set)
{
    u32 k, lit = Rgba(sLamp[set][1]), dim = Rgba(sLamp[set][0]);
    if (sEl.tex[lane] == NULL || sEl.idx[lane] == NULL)
        return;
    for (k = 0; k < EL_W * EL_H; k++)
        sEl.tex[lane][k] = sEl.idx[lane][k] == 3 ? lit : sEl.idx[lane][k] == 2 ? dim : 0;
}

// a message's words into its lane from x = 8 characters (Black's x 8 * 14, y 1), the small font's letters: its width
static u32 Write(u8 lane, const u8 *str, bool8 filled)
{
    u32 x = EL_LEAD * 14, i = 0;
    u8 *idx = sEl.idx[lane];
    if (idx == NULL)
        return 0;
    memset(idx, 0, EL_W * EL_H);
    if (str[0] == CHAR_QUESTION_MARK && str[1] == CHAR_SPACE)   // (Black's bullet, a glyph the game hasn't: drawn here,
    {                                                           //  ◇ before a saying, ◆ before the news)
        s32 dx, dy;
        for (dy = -3; dy <= 3; dy++)
            for (dx = -3; dx <= 3; dx++)
                if (abs(dx) + abs(dy) == 3 || (filled && abs(dx) + abs(dy) < 3))
                    idx[(7 + dy) * EL_W + x + 3 + dx] = 3;
        x += 8, i = 1;
    }
    for (; str[i] != EOS && x < EL_W; i++)
    {
        u32 w, gx, gy;
        if (str[i] == CHAR_SPACE)
        {
            x += 3;
            continue;
        }
        if (str[i] == CHAR_NEWLINE || str[i] == CHAR_PROMPT_SCROLL || str[i] == CHAR_PROMPT_CLEAR)
        {
            x += 3;
            continue;
        }
        if (str[i] >= 0xF7)   // (a control code and its arguments: none in the boards' words but colours)
        {
            if (str[i] == EXT_CTRL_CODE_BEGIN)
                i += 1 + (str[i + 1] == EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW ? 3 : 1);
            continue;
        }
        w = MP_RenderSmallGlyph(str[i], 3, 0, 2);
        for (gy = 0; gy < 16; gy++)
        {
            const u32 *rows = gy < 8 ? gCurGlyph.gfxBufferTop : gCurGlyph.gfxBufferBottom;
            for (gx = 0; gx < w && gx < 16; gx++)
            {
                u8 c = (rows[(gx < 8 ? 0 : 8) + gy % 8] >> ((gx % 8) * 4)) & 0xF;
                s32 y = (s32)gy + 1 - 2;   // (the small font sits low in its 16: Black's y 1 on a 16 high lane)
                if ((c == 3 || c == 2) && y >= 0 && y < EL_H && x + gx < EL_W && (c == 3 || idx[y * EL_W + x + gx] == 0))
                    idx[y * EL_W + x + gx] = c;
            }
        }
        x += w;
    }
    return x - EL_LEAD * 14;
}

// (CALENDER_GetDayOfYear: February 29 always counted)
static u16 DayOfYear(u8 month, u8 day)
{
    static const u8 sDays[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u16 sum = 0;
    u32 i;
    for (i = 0; i < month && i < 13; i++)
        sum += sDays[i];
    return sum + day - 1;
}

// (WEATHER_GetZoneWeatherId: a roamer's zone has its storm, else the zone's calendar for today, else its header)
u8 U3D_ZoneWeather(u16 zone)
{
    u16 r = U3D_Vm_RoamerWeather(zone);
    u32 i;
    if (r != 0xFFFF)
        return r;
    RtcCalcLocalTime();
    for (i = 0; i < ARRAY_COUNT(sCalZones); i++)
        if (sCalZones[i] == zone)
        {
            struct SiiRtcInfo rtc;
            RtcGetInfo(&rtc);
            return sCalDays[i][min(DayOfYear(ConvertBcdToBinary(rtc.month), ConvertBcdToBinary(rtc.day)), 365)];
        }
    return zone < ARRAY_COUNT(sZoneWeather) ? sZoneWeather[zone] : 0;
}

static u8 *SlotsSaved(bool8 save)
{
    static u8 sKeep[7][64];
    u32 k;
    for (k = 0; k < 7; k++)
        memcpy(save ? sKeep[k] : gUnovaStrVar[k], save ? gUnovaStrVar[k] : sKeep[k], 64);
    return NULL;
}

static void SetSlot(u8 slot, const u8 *str)
{
    u8 *end = StringCopyN(gUnovaStrVar[slot], str, 63);
    *end = EOS;
    gUnovaStrVar[slot][63] = EOS;
}

// a line: MSG of the boards' bank into LANE (GIMMICK_OBJ_ELBOARD_CreateLine: when the next line starts and when this
// one has left, as parts of the lane's 1024 scaled to its track's frames)
static void AddLine(u8 lane, u16 msg)
{
    static u8 sBuf[512];
    u32 width;
    float frames = U3D_ExpAnimFrames(EL_MODEL, lane);
    if (sEl.n >= EL_LANES || lane >= EL_LANES || frames == 0)
        return;
    StringExpandPlaceholders(sBuf, U3D_Text(EL_BANK, msg));
    width = Write(lane, sBuf, msg >= 197);
    sEl.line[sEl.n].lane = lane;
    sEl.line[sEl.n].playing = sEl.line[sEl.n].nextStarted = sEl.line[sEl.n].request = FALSE;
    sEl.line[sEl.n].elapsed = 0;
    sEl.line[sEl.n].next = (float)(width + EL_TRAIL * 13) / 1024.0f * frames;
    sEl.line[sEl.n].end = (float)(EL_LEAD * 13 + width) / 1024.0f * frames;
    sEl.n++;
}

static void AddDate(void)   // (AddDateMessage: month by name, the day, 20 and the year)
{
    struct SiiRtcInfo rtc;
    u8 *end;
    RtcCalcLocalTime();
    RtcGetInfo(&rtc);
    {
        u8 m = ConvertBcdToBinary(rtc.month);
        SetSlot(0, U3D_Text(EL_MONTHS, m >= 1 && m <= 12 ? m - 1 : 0));   // (WORDSET_SetSlotMonth: Jan. to Dec.)
    }
    end = ConvertIntToDecimalStringN(gUnovaStrVar[1], ConvertBcdToBinary(rtc.day), STR_CONV_MODE_LEFT_ALIGN, 2);
    *end = EOS;
    end = ConvertIntToDecimalStringN(gUnovaStrVar[2], ConvertBcdToBinary(rtc.year) % 100, STR_CONV_MODE_LEADING_ZEROS, 2);
    *end = EOS;
    AddLine(0, sEl.b->date);
}

static void AddPlaces(void)   // (AddPlaceListMessage: "place: weather" up to four, the record's places)
{
    static u8 sPair[4][64];
    u32 i, n = 0;
    for (i = 0; i < 4; i++)
    {
        u16 zone = sEl.b->place[i];
        if (zone == 0x1AB)
            continue;
        SetSlot(0, U3D_Text(EL_PLACES, zone < ARRAY_COUNT(sZonePlace) ? sZonePlace[zone] : 0));
        SetSlot(1, U3D_Text(EL_BANK, sElWeatherLabel[min(U3D_ZoneWeather(zone), ARRAY_COUNT(sElWeatherLabel) - 1)]));
        StringExpandPlaceholders(sPair[n], U3D_Text(EL_BANK, 0xC0));
        n++;
    }
    for (i = 0; i < n; i++)
        SetSlot(i, sPair[i]);
    if (n)
        AddLine(1, 0xC0 + n);
}

// (AddDailyMessage1-3, AddWeekdayMessage: Black's week starts its sayings on Tuesday, its notices on Monday)
static void AddDays(bool8 sayings)
{
    static const u8 sFirst[7] = {6, 3, 0, 4, 1, 5, 2};   // (by weekday, Sunday first)
    static const u8 sWeek[7] = {6, 0, 1, 2, 3, 4, 5};
    struct SiiRtcInfo rtc;
    u8 wd;
    RtcCalcLocalTime();
    RtcGetInfo(&rtc);
    wd = rtc.dayOfWeek % 7;
    if (sayings)
    {
        AddLine(3, sEl.b->day[sFirst[wd]]);
        AddLine(4, sEl.b->day[sFirst[wd] + 1]);
        AddLine(5, sEl.b->day[sFirst[wd] + 2]);
    }
    else
        AddLine(6, sEl.b->week[sWeek[wd]]);
}

static bool8 NewsFits(const struct ElNews *e, u16 zone)
{
    u32 k;
    if (e->version != 0 && e->version != BLACK)
        return FALSE;
    for (k = 0; k < 4; k++)
        if (e->zone[k] == 0x1AB || e->zone[k] == zone)
            return U3D_Vm_Flag(e->flag);
    return FALSE;
}

// (FillNoticeMessages: the story's news, a list of up to four on lanes 2-5, each flag cleared as it's shown)
static bool8 AddNews(u16 zone)
{
    u32 i, n = 0;
    for (i = 0; i < ARRAY_COUNT(sElNews) && n < 4; i++)
        if (NewsFits(&sElNews[i], zone))
        {
            AddLine(sElNoticeLanes[n++], sElNews[i].msg);
            if (sElNews[i].clears)
                U3D_Vm_ClearFlag(sElNews[i].flag);
        }
    return n != 0;
}

static void Start(u8 i)
{
    sEl.line[i].playing = TRUE, sEl.line[i].nextStarted = FALSE, sEl.line[i].elapsed = 0;
}

// a gate's zone shown: its board placed and its lines written (FIELD_GIMMICK_GATE_Init); FALSE when it has none
bool8 U3D_Elboard_Load(u16 zone)
{
    u32 i;
    bool8 news;
    sEl.on = FALSE;
    sEl.b = NULL;
    for (i = 0; i < ARRAY_COUNT(sElBoards); i++)
        if (sElBoards[i].zone == zone && (sElBoards[i].version == 0 || sElBoards[i].version == BLACK))
        {
            sEl.b = &sElBoards[i];
            break;
        }
    if (sEl.b == NULL)
        return FALSE;
    U3D_ExpLoad(48);
    {
        static const u16 sYaw[4] = {0x8000, 0, 0xC000, 0x4000};   // (FIELD_GIMMICK_GATE_PlaceBoardModel: up 180, down 0,
        U3D_ExpPlace(EL_MODEL, sEl.b->x, sEl.b->y, sEl.b->z, sYaw[sEl.b->facing & 3]);   //  left 270, right 90)
    }
    U3D_ExpHide(EL_MODEL, FALSE);
    if (sEl.b->tv < ARRAY_COUNT(sElTvAnim))
        U3D_ExpTex(EL_MODEL, sElTvAnim[sEl.b->tv], TRUE, 0, TRUE, TRUE);
    memset(sEl.line, 0, sizeof(sEl.line));
    sEl.n = 0, sEl.tick = 0, sEl.colours = 0xFF;
    for (i = 0; i < EL_LANES; i++)
    {
        if (sEl.idx[i] == NULL)
            sEl.idx[i] = malloc(EL_W * EL_H);
        sEl.tex[i] = U3D_ExpOwnTexture(EL_MODEL, i, EL_W, EL_H);
        if (sEl.idx[i])
            memset(sEl.idx[i], 0, EL_W * EL_H);
    }
    if (sEl.b->shown)
    {
        SlotsSaved(TRUE);
        AddDate();
        AddPlaces();
        news = AddNews(zone);
        if (!news)
            AddDays(TRUE);
        AddDays(FALSE);
        SlotsSaved(FALSE);
    }
    if (sEl.n)
        sEl.line[0].request = TRUE;   // (GIMMICK_OBJ_ELBOARD_Create: the first line asked to start)
    sEl.on = TRUE;
    MPNet_Log("u3d vm: zone %u's news board, %u lines", zone, sEl.n);
    return TRUE;
}

void U3D_Elboard_Clear(void)
{
    sEl.on = FALSE;
}

// a field frame: half of Black's step (GIMMICK_OBJ_ELBOARD_Advance by gElboardLayout's 1.0, Black's 30 a second)
void U3D_Elboard_Frame(void)
{
    u32 i;
    u8 set;
    if (!sEl.on)
        return;
    sEl.tick++;
    set = (sEl.tick / 2 & 31) / 8;
    if (set != sEl.colours)
    {
        for (i = 0; i < EL_LANES; i++)
            Paint(i, set);
        sEl.colours = set;
    }
    for (i = 0; i < sEl.n; i++)
    {
        typeof(sEl.line[0]) *l = &sEl.line[i];
        if (l->playing)
        {
            l->elapsed += 0.5f;
            if (l->next < l->elapsed && !l->nextStarted)
            {
                sEl.line[(i + 1) % sEl.n].request = TRUE;
                l->nextStarted = TRUE;
            }
            if (l->end < l->elapsed)
                l->playing = FALSE;
        }
        else if (l->request)
        {
            Start(i);
            l->request = FALSE;
        }
        U3D_ExpTex(EL_MODEL, l->lane, l->playing, l->playing ? (s16)l->elapsed : 0, FALSE, FALSE);
    }
}
