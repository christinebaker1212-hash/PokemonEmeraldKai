// Moemon Co-op: the list windows used by the co-op screens (shared PC, trades,
// gifts, goals...). A main window with a title and up to MPUI_ROWS rows, and a
// one-line help window under it, both drawn over the overworld.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "main.h"
#include "event_object_lock.h"
#include "map_name_popup.h"
#include "menu.h"
#include "item.h"
#include "field_message_box.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/characters.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define MAIN_W 28
#define MAIN_H 13
#define ROW_H  12
#define ROW_Y0 16
#define HELP_H 3   // tiles: two lines of small text

static const struct WindowTemplate sMainTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = MAIN_W, .height = MAIN_H,
    .paletteNum = 15, .baseBlock = 0x8,
};

static const struct WindowTemplate sHelpTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = MAIN_W, .height = HELP_H,
    .paletteNum = 15, .baseBlock = 0x8 + MAIN_W * MAIN_H,
};

static u8 sMain = WINDOW_NONE, sHelp = WINDOW_NONE;
static s16 sScrollTop = -1, sScrollCount, sSelY = -1;
static s16 sTitleEnd;   // x where the title text ends

// ---------------------------------------------------------------------------
// Theme: a dark navy panel with a gold trim and a purple title band. The text
// palette (BG palette 15) is swapped while a co-op menu is open. Text printed
// with the default colors (2 on 1, shadow 3) comes out white on navy.

static u16 sThemePal[16];

#define C_R(c) ((c) & 31)
#define C_G(c) (((c) >> 5) & 31)
#define C_B(c) (((c) >> 10) & 31)

// a + (b - a) * f / 256
static u16 Mix(u16 a, u16 b, int f)
{
    int r = C_R(a) + (C_R(b) - C_R(a)) * f / 256;
    int g = C_G(a) + (C_G(b) - C_G(a)) * f / 256;
    int bl = C_B(a) + (C_B(b) - C_B(a)) * f / 256;
    return RGB(r, g, bl);
}

// The main color of the player's chosen window frame (OPTIONS > FRAME): the
// colorful shade that covers the most of it
static u16 FrameAccent(bool8 *colorful)
{
    const struct TilesPal *frame = GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType);
    u32 count[16] = {0}, best = 0;
    u16 accent = RGB(16, 16, 20);
    int i, k;
    for (i = 0; i < 0x120; i++)
    {
        count[frame->tiles[i] & 15]++;
        count[frame->tiles[i] >> 4]++;
    }
    *colorful = FALSE;
    for (k = 1; k < 16; k++)
    {
        u16 c = frame->pal[k];
        int mx = max(C_R(c), max(C_G(c), C_B(c))), mn = min(C_R(c), min(C_G(c), C_B(c)));
        u32 score = count[k] * (mx - mn + 1);
        if (count[k] == 0 || mx < 8)
            continue;
        if (score > best)
        {
            best = score;
            accent = c;
            *colorful = (mx - mn) >= 6;
        }
    }
    // Same hue at full brightness, so dark and light frames both work
    {
        int mx = max(C_R(accent), max(C_G(accent), C_B(accent)));
        if (mx > 0)
            accent = RGB(C_R(accent) * 31 / mx, C_G(accent) * 31 / mx, C_B(accent) * 31 / mx);
    }
    return accent;
}

// The co-op menu colors: white panels and dark text like the game's own
// menus (std_menu.pal), with light accents in the color of the player's
// window frame for the title band and the selected row
void MpTheme_BuildPalette(u16 *pal)
{
    bool8 colorful;
    u16 a = FrameAccent(&colorful);
    u16 white = RGB(31, 31, 31);
    CpuCopy16(gStandardMenuPalette, pal, PLTT_SIZE_4BPP);   // 1 white, 2 dark grey, 3 light grey...
    pal[MPT_BAND] = Mix(white, a, 64);                         // title band
    pal[MPT_BAND2] = Mix(white, a, 120);                       // its top line
    pal[MPT_SEL] = Mix(white, a, 52);                          // selected row
    pal[MPT_GOLD] = colorful ? Mix(0, a, 120) : RGB(7, 7, 8);  // highlighted text (dark, in the frame's color)
    pal[MPT_GREY] = RGB(14, 14, 15);                           // less important text
    pal[MPT_BORDER] = a;                                       // accent lines
}

static u16 sSavedPal[16];
static bool8 sThemeOn;

static void WritePal15(const u16 *colors)
{
    // While the screen is faded out, only the unfaded copy changes
    CpuCopy16(colors, &gPlttBufferUnfaded[BG_PLTT_ID(15)], PLTT_SIZE_4BPP);
    if (!gPaletteFade.active && gPaletteFade.y == 0)
        CpuCopy16(colors, &gPlttBufferFaded[BG_PLTT_ID(15)], PLTT_SIZE_4BPP);
}

void MpTheme_Push(void)
{
    if (sThemeOn)
        return;
    CpuCopy16(&gPlttBufferUnfaded[BG_PLTT_ID(15)], sSavedPal, PLTT_SIZE_4BPP);
    MpTheme_BuildPalette(sThemePal);
    WritePal15(sThemePal);
    sThemeOn = TRUE;
}

void MpTheme_Pop(void)
{
    if (!sThemeOn)
        return;
    WritePal15(sSavedPal);
    sThemeOn = FALSE;
}

// The panel: a fill in the frame's color and an optional title band. The
// player's own window frame goes around it (DrawStdWindowFrame).
void MpTheme_Panel(u8 windowId, u8 bandHeight)
{
    int w = GetWindowAttribute(windowId, WINDOW_WIDTH) * 8;
    FillWindowPixelBuffer(windowId, PIXEL_FILL(MPT_BG));
    if (bandHeight)
    {
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_BAND), 0, 0, w, bandHeight);
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_BAND2), 0, 0, w, 1);
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_BORDER), 0, bandHeight, w, 1);
    }
}

// The highlight behind the selected row: a lit bar with a gold edge
void MpTheme_SelBar(u8 windowId, int y, int h)
{
    int w = GetWindowAttribute(windowId, WINDOW_WIDTH) * 8;
    FillWindowPixelRect(windowId, PIXEL_FILL(MPT_SEL), 1, y, w - 2, h);
    FillWindowPixelRect(windowId, PIXEL_FILL(MPT_BORDER), 1, y, 2, h);
}

void MpTheme_Print(u8 windowId, u8 font, const u8 *str, int x, int y, u8 fg, u8 bg)
{
    u8 colors[3] = {bg, fg, MPT_SHADOW};
    AddTextPrinterParameterized3(windowId, font, x, y, colors, TEXT_SKIP_DRAW, str);
}

// A small gold triangle pointing up or down (scroll hints)
static void DrawTriangle(u8 windowId, int cx, int y, bool8 up)
{
    int i;
    for (i = 0; i < 3; i++)
    {
        int row = up ? y + i : y + 2 - i;
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_GOLD), cx - i, row, i * 2 + 1, 1);
    }
}

bool8 MpUi_IsOpen(void)
{
    return sMain != WINDOW_NONE;
}

void MpUi_Open(void)
{
    if (sMain != WINDOW_NONE)
        return;
    LockPlayerFieldControls();
    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();   // the map name sign shares the frame's palette slot
    MpTheme_Push();
    sMain = AddWindow(&sMainTemplate);
    sHelp = AddWindow(&sHelpTemplate);
    PutWindowTilemap(sMain);
    PutWindowTilemap(sHelp);
    DrawStdWindowFrame(sMain, FALSE);
    DrawStdWindowFrame(sHelp, FALSE);
    MpTheme_Panel(sMain, 13);
    MpTheme_Panel(sHelp, 0);
    CopyWindowToVram(sMain, COPYWIN_FULL);
    CopyWindowToVram(sHelp, COPYWIN_FULL);
}

void MpUi_Close(void)
{
    if (sMain == WINDOW_NONE)
        return;
    ClearStdWindowAndFrame(sMain, TRUE);
    ClearStdWindowAndFrame(sHelp, TRUE);
    RemoveWindow(sMain);
    RemoveWindow(sHelp);
    sMain = sHelp = WINDOW_NONE;
    MpTheme_Pop();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void MpUi_Begin(const char *title)
{
    u8 buf[96];
    sScrollTop = -1;
    sSelY = -1;
    MpTheme_Panel(sMain, 13);
    MP_AsciiToGame(buf, title, 92);
    MpUi_PrintWrapped(sMain, buf, 6, 1, MAIN_W * 8 - 12, 1, MPT_GOLD, MPT_BAND);
    sTitleEnd = 6 + GetStringWidth(FONT_SMALL, buf, 0);
}

// Text on the right end of the title line (e.g. "POKéMON  ITEMS" tabs)
void MpUi_TitleRight(const u8 *text)
{
    u8 cut[64];
    u8 font = FONT_SMALL;
    int len, room = MAIN_W * 8 - 6 - (sTitleEnd + 8);
    StringCopyN(cut, text, sizeof(cut) - 1);
    cut[sizeof(cut) - 1] = EOS;
    if (GetStringWidth(font, cut, 0) > room)
        font = FONT_SMALL_NARROW;
    for (len = StringLength(cut); len > 1 && GetStringWidth(font, cut, 0) > room; len--)
        cut[len - 1] = EOS;
    if (room <= 0)
        return;
    MpTheme_Print(sMain, font, cut, MAIN_W * 8 - 6 - GetStringWidth(font, cut, 0), 1, MPT_TEXT, MPT_BAND);
}

// One row: left text, and optional right-aligned text. Long left text is cut.
void MpUi_Row(u8 row, bool8 selected, const u8 *left, const u8 *right)
{
    u8 y = ROW_Y0 + row * ROW_H;
    u8 cut[80];
    u8 rcut[64];
    int maxW, len2;
    if (right)
    {
        // Right text gets at most half the row
        StringCopyN(rcut, right, sizeof(rcut) - 1);
        rcut[sizeof(rcut) - 1] = EOS;
        for (len2 = StringLength(rcut); len2 > 1 && GetStringWidth(FONT_SMALL_NARROW, rcut, 0) > MAIN_W * 4; len2--)
            rcut[len2 - 1] = EOS;
        right = rcut;
    }
    maxW = MAIN_W * 8 - 22 - (right ? GetStringWidth(FONT_SMALL_NARROW, right, 0) + 8 : 4);
    int len;
    u8 bg = MPT_BG;
    if (selected)
    {
        // The game's own cursor
        u8 arrow[] = {0xEF /* ▶ */, EOS};
        MpTheme_Print(sMain, FONT_SMALL, arrow, 6, y, MPT_TEXT, bg);
        sSelY = y;
    }
    StringCopyN(cut, left, sizeof(cut) - 1);
    cut[sizeof(cut) - 1] = EOS;
    for (len = StringLength(cut); len > 1 && GetStringWidth(FONT_SMALL_NARROW, cut, 0) > maxW; len--)
        cut[len - 1] = EOS;
    MpTheme_Print(sMain, FONT_SMALL_NARROW, cut, 15, y, MPT_TEXT, bg);
    if (right)
        MpTheme_Print(sMain, FONT_SMALL_NARROW, right, MAIN_W * 8 - 12 - GetStringWidth(FONT_SMALL_NARROW, right, 0), y,
                      MPT_GREY, bg);
}

void MpUi_RowAscii(u8 row, bool8 selected, const char *left, const char *right)
{
    u8 l[80], r[40];
    MP_AsciiToGame(l, left, 76);
    if (right)
        MP_AsciiToGame(r, right, 36);
    MpUi_Row(row, selected, l, right ? r : NULL);
}

// A line of text anywhere in the list area (row = 0..MPUI_ROWS-1)
void MpUi_TextAscii(u8 row, const char *text)
{
    u8 buf[160];
    MP_AsciiToGame(buf, text, 156);
    MpUi_PrintWrapped(sMain, buf, 15, ROW_Y0 + row * ROW_H, MAIN_W * 8 - 15 - 6, 1, MPT_TEXT, MPT_BG);
}

void MpUi_End(void)
{
    // Scroll hints when the list goes on above or below
    if (sScrollTop >= 0 && sScrollCount > MPUI_ROWS)
    {
        if (sScrollTop > 0)
            DrawTriangle(sMain, MAIN_W * 8 - 6, ROW_Y0 - 1, TRUE);
        if (sScrollTop + MPUI_ROWS < sScrollCount)
            DrawTriangle(sMain, MAIN_W * 8 - 6, ROW_Y0 + MPUI_ROWS * ROW_H - 2, FALSE);
    }
    CopyWindowToVram(sMain, COPYWIN_FULL);
}

// Splits text into lines no wider than maxW (breaking at spaces); returns how many
// lines it needs. Only the first maxLines are written.
static int WrapText(const u8 *text, u8 font, int maxW, u8 out[][128], int maxLines)
{
    int lines = 0, i = 0;
    while (text[i] != EOS)
    {
        u8 line[128];
        int n = 0, lastSpace = -1, lastSpaceSrc = -1, j = i;
        while (text[j] == CHAR_SPACE)
            j++;
        i = j;
        if (text[i] == EOS)
            break;
        for (;;)
        {
            u8 c = text[j];
            if (c == EOS || c == CHAR_NEWLINE || n >= 126)
                break;
            line[n] = c;
            line[n + 1] = EOS;
            if (GetStringWidth(font, line, 0) > maxW)
            {
                if (lastSpace > 0)
                {
                    n = lastSpace;
                    j = lastSpaceSrc;
                }
                break;
            }
            if (c == CHAR_SPACE)
            {
                lastSpace = n;
                lastSpaceSrc = j;
            }
            n++;
            j++;
        }
        if (n == 0 && text[j] != EOS && text[j] != CHAR_NEWLINE)   // a single character wider than the line: take it anyway
        {
            line[n++] = text[j];
            j++;
        }
        line[n] = EOS;
        if (lines < maxLines)
            StringCopy(out[lines], line);
        lines++;
        i = j;
        if (text[i] == CHAR_NEWLINE)
            i++;
    }
    return lines;
}

// Prints text in two lines at most in a window `width` pixels wide, picking the
// narrow font and cutting it short with "..." only when it really doesn't fit.
void MpUi_PrintWrapped(u8 windowId, const u8 *text, int x, int y, int width, int maxLines, u8 fg, u8 bg)
{
    u8 lines[4][128];
    u8 font = FONT_SMALL;
    int count, i;
    if (maxLines > 4)
        maxLines = 4;
    count = WrapText(text, font, width, lines, maxLines);
    if (count > maxLines)
    {
        font = FONT_SMALL_NARROW;
        count = WrapText(text, font, width, lines, maxLines);
    }
    if (count > maxLines)
    {
        // Still too long: end the last line with "..."
        u8 *last = lines[maxLines - 1];
        static const u8 sDots[] = {CHAR_PERIOD, CHAR_PERIOD, CHAR_PERIOD, EOS};
        int len = StringLength(last);
        for (;;)
        {
            u8 tmp[132];
            StringCopy(tmp, last);
            StringAppend(tmp, sDots);
            if (len <= 1 || GetStringWidth(font, tmp, 0) <= width)
            {
                StringCopy(last, tmp);
                break;
            }
            last[--len] = EOS;
        }
        count = maxLines;
    }
    for (i = 0; i < count; i++)
        MpTheme_Print(windowId, font, lines[i], x, y + i * 12, fg, bg);
}

void MpUi_Help(const u8 *text)
{
    if (sHelp == WINDOW_NONE)
        return;
    MpTheme_Panel(sHelp, 0);
    MpUi_PrintWrapped(sHelp, text, 4, 0, MAIN_W * 8 - 8, 2, MPT_GREY, MPT_BG);
    CopyWindowToVram(sHelp, COPYWIN_FULL);
}

void MpUi_HelpAscii(const char *ascii)
{
    u8 buf[200];
    MP_AsciiToGame(buf, ascii, 196);
    MpUi_Help(buf);
}

// Up/Down with wrap-around; returns TRUE when the cursor moved
bool8 MpUi_MoveCursor(u16 *cursor, u16 count)
{
    if (count == 0)
        return FALSE;
    if (JOY_REPEAT(DPAD_UP))
        *cursor = (*cursor == 0) ? count - 1 : *cursor - 1;
    else if (JOY_REPEAT(DPAD_DOWN))
        *cursor = (*cursor + 1) % count;
    else
        return FALSE;
    PlaySE(SE_SELECT);
    return TRUE;
}

// First row to show so the cursor stays on screen
u16 MpUi_ScrollFor(u16 cursor, u16 count, u16 *scroll)
{
    if (cursor < *scroll)
        *scroll = cursor;
    if (cursor >= *scroll + MPUI_ROWS)
        *scroll = cursor - MPUI_ROWS + 1;
    if (count <= MPUI_ROWS)
        *scroll = 0;
    sScrollTop = *scroll;
    sScrollCount = count;
    return *scroll;
}

// "x12" style quantity text
void MpUi_Qty(u8 *dst, u32 qty)
{
    dst[0] = CHAR_x;
    ConvertIntToDecimalStringN(dst + 1, qty, STR_CONV_MODE_LEFT_ALIGN, 4);
}

// The help line shows an item's description (its lines joined into one)
void MpUi_HelpItem(u16 item)
{
    const u8 *desc = GetItemDescription(item);
    u8 buf[200];
    int i, n = 0;
    for (i = 0; desc[i] != EOS && n < (int)sizeof(buf) - 1; i++)
        buf[n++] = (desc[i] == CHAR_NEWLINE) ? CHAR_SPACE : desc[i];
    buf[n] = EOS;
    MpUi_Help(buf);
}

// ---------------------------------------------------------------------------
// Field message box text: wrapped to the box width, two lines per page (the
// game waits for A between pages). Text with special codes is left alone.

#define MSGBOX_TEXT_W 208

static void FitMessageBoxText(u8 *dst, int dstSize, const u8 *src)
{
    int n = 0, i = 0, lineStart = 0, lineOnPage = 0;
    for (i = 0; src[i] != EOS; i++)
        if (src[i] == EXT_CTRL_CODE_BEGIN || src[i] == PLACEHOLDER_BEGIN)
        {
            StringCopyN(dst, src, dstSize - 1);
            dst[dstSize - 1] = EOS;
            return;
        }
    i = 0;
    while (src[i] != EOS && n < dstSize - 2)
    {
        u8 c = src[i];
        if (c == CHAR_NEWLINE || c == CHAR_PROMPT_SCROLL || c == CHAR_PROMPT_CLEAR)
        {
            if (c == CHAR_NEWLINE && lineOnPage == 0)
                dst[n++] = CHAR_NEWLINE, lineOnPage = 1;
            else
                dst[n++] = CHAR_PROMPT_CLEAR, lineOnPage = 0;
            lineStart = n;
            i++;
            continue;
        }
        if (c == CHAR_SPACE && n == lineStart)
        {
            i++;   // no spaces at the start of a line
            continue;
        }
        {
            // The next word, with the space before it
            int j = i, k, w;
            u8 tmp[256];
            int t = 0;
            if (src[j] == CHAR_SPACE)
                j++;
            while (src[j] != EOS && src[j] != CHAR_SPACE && src[j] != CHAR_NEWLINE
                && src[j] != CHAR_PROMPT_SCROLL && src[j] != CHAR_PROMPT_CLEAR)
                j++;
            for (k = lineStart; k < n && t < 200; k++)
                tmp[t++] = dst[k];
            for (k = i; k < j && t < 250; k++)
                tmp[t++] = src[k];
            tmp[t] = EOS;
            w = GetStringWidth(FONT_NORMAL, tmp, 0);
            if (w > MSGBOX_TEXT_W && n > lineStart)
            {
                // Break before this word
                dst[n++] = lineOnPage == 0 ? CHAR_NEWLINE : CHAR_PROMPT_CLEAR;
                lineOnPage ^= 1;
                lineStart = n;
                if (src[i] == CHAR_SPACE)
                    i++;
                continue;
            }
            for (k = i; k < j && n < dstSize - 2; k++)
                dst[n++] = src[k];
            i = j;
        }
    }
    dst[n] = EOS;
}

void MP_ShowFieldMessageFitted(const u8 *text)
{
    static u8 sExpanded[400], sFitted[440];
    StringExpandPlaceholders(sExpanded, text);
    FitMessageBoxText(sFitted, sizeof(sFitted), sExpanded);
    ShowFieldMessage(sFitted);
}

// ---------------------------------------------------------------------------
// Joined frames: where two co-op windows meet (one's bottom edge on the row
// above the other's top edge, or both edges on the same row), the frame art's
// see-through rows let the map show through as a thin gap. While a co-op menu
// is up, the frames there are drawn with joint tiles instead (menu.c calls
// these from the standard frame draw / clear).
#include "bg.h"

#define JOIN_BASE     0x2F0   // BG0 tiles 0x2F0-0x2FE (free in the field layout: tiles 0x8000-0xDFFF)
#define STD_WINDOW_PALETTE_NUM   14      // as in menu.c
#define STD_WINDOW_BASE_TILE_NUM 0x214
#define JOIN_EXT_DOWN 0       // + 0..2: bottom edge tiles 6..8, filled to the bottom
#define JOIN_EXT_UP   3       // + 0..2: top edge tiles 0..2, filled to the top
#define JOIN_MERGED   6       // + (bottom 0..2) * 3 + top 0..2: one shared row

static bool8 JoinUsable(u8 bg)
{
    return sThemeOn && GetBgAttribute(bg, BG_ATTR_CHARBASEINDEX) == 2 && GetBgAttribute(bg, BG_ATTR_MAPBASEINDEX) == 31;
}

static u8 Px(const u8 *t, int x, int y)
{
    u8 b = t[y * 4 + x / 2];
    return (x & 1) ? b >> 4 : b & 0xF;
}

static void SetPx(u8 *t, int x, int y, u8 v)
{
    u8 *b = &t[y * 4 + x / 2];
    *b = (x & 1) ? (*b & 0x0F) | (v << 4) : (*b & 0xF0) | v;
}

static void LoadJoinTiles(u8 bg)
{
    static u8 sJoinTiles[15][32];
    const u8 *frame = (const u8 *)GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles;
    int i, j, x, y;
    for (i = 0; i < 3; i++)
    {
        u8 *down = sJoinTiles[JOIN_EXT_DOWN + i], *up = sJoinTiles[JOIN_EXT_UP + i];
        memcpy(down, frame + (6 + i) * 32, 32);
        memcpy(up, frame + i * 32, 32);
        for (x = 0; x < 8; x++)
        {
            // See-through pixels under the last drawn one take its color (and above the first one)
            for (y = 7; y >= 0 && !Px(down, x, y); y--)
                ;
            if (y >= 0)
                for (j = y + 1; j < 8; j++)
                    SetPx(down, x, j, Px(down, x, y));
            for (y = 0; y < 8 && !Px(up, x, y); y++)
                ;
            if (y < 8)
                for (j = y - 1; j >= 0; j--)
                    SetPx(up, x, j, Px(up, x, y));
        }
        for (j = 0; j < 3; j++)
        {
            u8 *m = sJoinTiles[JOIN_MERGED + i * 3 + j];
            memcpy(m, frame + (6 + i) * 32, 16);          // the upper window's bottom edge, top half
            memcpy(m + 16, frame + j * 32 + 16, 16);      // the lower window's top edge, bottom half
        }
    }
    LoadBgTiles(bg, sJoinTiles, sizeof(sJoinTiles), JOIN_BASE);
}

// The standard frame piece (0-8) at a tilemap entry, or -1
static int FramePiece(u16 entry)
{
    int t = (entry & 0x3FF) - STD_WINDOW_BASE_TILE_NUM;
    return ((entry >> 12) == STD_WINDOW_PALETTE_NUM && t >= 0 && t <= 8) ? t : -1;
}

static int JoinPiece(u16 entry)
{
    int t = (entry & 0x3FF) - JOIN_BASE;
    return ((entry >> 12) == STD_WINDOW_PALETTE_NUM && t >= 0 && t < 15) ? t : -1;
}

static u16 Entry(int tile)
{
    return tile | (STD_WINDOW_PALETTE_NUM << 12);
}

// Before a frame is drawn: the tiles under its top and bottom rows
void MP_FrameJoin_Save(u8 bg, u8 left, u8 top, u8 width, u8 height, u16 *saved)
{
    u16 *map = GetBgTilemapBuffer(bg);
    int x, i = 0;
    if (!map || !JoinUsable(bg))
        return;
    for (x = left - 1; x <= left + width; x++, i++)
    {
        saved[i * 2] = (top >= 1 && x >= 0 && x < 32) ? map[(top - 1) * 32 + x] : 0;
        saved[i * 2 + 1] = (top + height < 32 && x >= 0 && x < 32) ? map[(top + height) * 32 + x] : 0;
    }
}

// After it is drawn: joint tiles where it meets another frame
void MP_FrameJoin_Apply(u8 bg, u8 left, u8 top, u8 width, u8 height, const u16 *saved)
{
    u16 *map = GetBgTilemapBuffer(bg);
    int x, i = 0, rt = top - 1, rb = top + height;
    bool8 any = FALSE;
    if (!map || !JoinUsable(bg))
        return;
    for (x = left - 1; x <= left + width; x++, i++)
    {
        int edge = (x == left - 1) ? 0 : (x == left + width) ? 2 : 1;   // which corner / edge piece
        int old, p;
        if (x < 0 || x >= 32)
            continue;
        // Top row: on another frame's bottom edge, or just under it
        if (rt >= 0)
        {
            old = FramePiece(saved[i * 2]);
            if (old >= 6)
            {
                map[rt * 32 + x] = Entry(JOIN_BASE + JOIN_MERGED + (old - 6) * 3 + edge);
                any = TRUE;
            }
            else if (rt >= 1 && (p = FramePiece(map[(rt - 1) * 32 + x])) >= 6)
            {
                map[(rt - 1) * 32 + x] = Entry(JOIN_BASE + JOIN_EXT_DOWN + p - 6);
                map[rt * 32 + x] = Entry(JOIN_BASE + JOIN_EXT_UP + edge);
                any = TRUE;
            }
        }
        // Bottom row: on another frame's top edge, or just over it
        if (rb < 32)
        {
            old = FramePiece(saved[i * 2 + 1]);
            if (old >= 0 && old <= 2)
            {
                map[rb * 32 + x] = Entry(JOIN_BASE + JOIN_MERGED + edge * 3 + old);
                any = TRUE;
            }
            else if (rb + 1 < 32 && (p = FramePiece(map[(rb + 1) * 32 + x])) >= 0 && p <= 2)
            {
                map[rb * 32 + x] = Entry(JOIN_BASE + JOIN_EXT_DOWN + edge);
                map[(rb + 1) * 32 + x] = Entry(JOIN_BASE + JOIN_EXT_UP + p);
                any = TRUE;
            }
        }
    }
    if (any)
        LoadJoinTiles(bg);
}

// After a frame is cleared: the frame it was joined to gets its own edge back
void MP_FrameJoin_Clear(u8 bg, u8 left, u8 top, u8 width, u8 height, const u16 *saved)
{
    u16 *map = GetBgTilemapBuffer(bg);
    int x, i = 0, rt = top - 1, rb = top + height;
    if (!map || !JoinUsable(bg))
        return;
    for (x = left - 1; x <= left + width; x++, i++)
    {
        int j;
        if (x < 0 || x >= 32)
            continue;
        if (rt >= 0)
        {
            j = JoinPiece(saved[i * 2]);
            if (j >= JOIN_MERGED)
                map[rt * 32 + x] = Entry(STD_WINDOW_BASE_TILE_NUM + 6 + (j - JOIN_MERGED) / 3);
            if (rt >= 1 && (j = JoinPiece(map[(rt - 1) * 32 + x])) >= JOIN_EXT_DOWN && j < JOIN_EXT_UP)
                map[(rt - 1) * 32 + x] = Entry(STD_WINDOW_BASE_TILE_NUM + 6 + j - JOIN_EXT_DOWN);
        }
        if (rb < 32)
        {
            j = JoinPiece(saved[i * 2 + 1]);
            if (j >= JOIN_MERGED)
                map[rb * 32 + x] = Entry(STD_WINDOW_BASE_TILE_NUM + (j - JOIN_MERGED) % 3);
            if (rb + 1 < 32 && (j = JoinPiece(map[(rb + 1) * 32 + x])) >= JOIN_EXT_UP && j < JOIN_MERGED)
                map[(rb + 1) * 32 + x] = Entry(STD_WINDOW_BASE_TILE_NUM + j - JOIN_EXT_UP);
        }
    }
}
