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

static const struct WindowTemplate sMainTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = MAIN_W, .height = MAIN_H,
    .paletteNum = 15, .baseBlock = 0x8,
};

static const struct WindowTemplate sHelpTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = MAIN_W, .height = 2,
    .paletteNum = 15, .baseBlock = 0x8 + MAIN_W * MAIN_H,
};

static u8 sMain = WINDOW_NONE, sHelp = WINDOW_NONE;
static s16 sScrollTop = -1, sScrollCount, sSelY = -1;

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
    u8 buf[64];
    sScrollTop = -1;
    sSelY = -1;
    MpTheme_Panel(sMain, 13);
    MP_AsciiToGame(buf, title, 60);
    MpTheme_Print(sMain, FONT_SMALL, buf, 6, 1, MPT_GOLD, MPT_BAND);
}

// Text on the right end of the title line (e.g. "POKéMON  ITEMS" tabs)
void MpUi_TitleRight(const u8 *text)
{
    MpTheme_Print(sMain, FONT_SMALL, text, MAIN_W * 8 - 6 - GetStringWidth(FONT_SMALL, text, 0), 1, MPT_TEXT, MPT_BAND);
}

// One row: left text, and optional right-aligned text. Long left text is cut.
void MpUi_Row(u8 row, bool8 selected, const u8 *left, const u8 *right)
{
    u8 y = ROW_Y0 + row * ROW_H;
    u8 cut[80];
    int maxW = MAIN_W * 8 - 22 - (right ? GetStringWidth(FONT_SMALL_NARROW, right, 0) + 8 : 4);
    int len;
    u8 bg = selected ? MPT_SEL : MPT_BG;
    if (selected)
    {
        u8 arrow[] = {0xEF, EOS};
        MpTheme_SelBar(sMain, y, ROW_H - 1);
        MpTheme_Print(sMain, FONT_SMALL, arrow, 6, y, MPT_GOLD, bg);
        sSelY = y;
    }
    StringCopyN(cut, left, sizeof(cut) - 1);
    cut[sizeof(cut) - 1] = EOS;
    for (len = StringLength(cut); len > 1 && GetStringWidth(FONT_SMALL_NARROW, cut, 0) > maxW; len--)
        cut[len - 1] = EOS;
    MpTheme_Print(sMain, FONT_SMALL_NARROW, cut, 15, y, selected ? MPT_GOLD : MPT_TEXT, bg);
    if (right)
        MpTheme_Print(sMain, FONT_SMALL_NARROW, right, MAIN_W * 8 - 12 - GetStringWidth(FONT_SMALL_NARROW, right, 0), y,
                      selected ? MPT_TEXT : MPT_GREY, bg);
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
    u8 buf[96];
    MP_AsciiToGame(buf, text, 92);
    MpTheme_Print(sMain, FONT_SMALL, buf, 15, ROW_Y0 + row * ROW_H, MPT_TEXT, MPT_BG);
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

void MpUi_Help(const u8 *text)
{
    MpTheme_Panel(sHelp, 0);
    MpTheme_Print(sHelp, FONT_SMALL, text, 4, 1, MPT_GREY, MPT_BG);
    CopyWindowToVram(sHelp, COPYWIN_FULL);
}

void MpUi_HelpAscii(const char *ascii)
{
    u8 buf[96];
    MP_AsciiToGame(buf, ascii, 92);
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
    u8 buf[160];
    int i, n = 0;
    for (i = 0; desc[i] != EOS && n < (int)sizeof(buf) - 1; i++)
        buf[n++] = (desc[i] == CHAR_NEWLINE) ? CHAR_SPACE : desc[i];
    buf[n] = EOS;
    while (n > 1 && GetStringWidth(FONT_SMALL_NARROW, buf, 0) > MAIN_W * 8 - 8)
        buf[--n] = EOS;
    MpTheme_Panel(sHelp, 0);
    MpTheme_Print(sHelp, FONT_SMALL_NARROW, buf, 4, 1, MPT_GREY, MPT_BG);
    CopyWindowToVram(sHelp, COPYWIN_FULL);
}
