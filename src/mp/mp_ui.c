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
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
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
    sMain = AddWindow(&sMainTemplate);
    sHelp = AddWindow(&sHelpTemplate);
    DrawStdWindowFrame(sMain, FALSE);
    DrawStdWindowFrame(sHelp, FALSE);
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
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void MpUi_Begin(const char *title)
{
    u8 buf[64];
    FillWindowPixelBuffer(sMain, PIXEL_FILL(1));
    MP_AsciiToGame(buf, title, 60);
    AddTextPrinterParameterized(sMain, FONT_SMALL, buf, 4, 1, TEXT_SKIP_DRAW, NULL);
}

// Text on the right end of the title line (e.g. "POKéMON  ITEMS" tabs)
void MpUi_TitleRight(const u8 *text)
{
    AddTextPrinterParameterized(sMain, FONT_SMALL, text, MAIN_W * 8 - 4 - GetStringWidth(FONT_SMALL, text, 0), 1, TEXT_SKIP_DRAW, NULL);
}

// One row: left text, and optional right-aligned text. Long left text is cut.
void MpUi_Row(u8 row, bool8 selected, const u8 *left, const u8 *right)
{
    u8 y = ROW_Y0 + row * ROW_H;
    u8 cut[80];
    int maxW = MAIN_W * 8 - 14 - (right ? GetStringWidth(FONT_SMALL_NARROW, right, 0) + 8 : 4);
    int len;
    if (selected)
    {
        u8 arrow[] = {0xEF, EOS};
        AddTextPrinterParameterized(sMain, FONT_SMALL, arrow, 2, y, TEXT_SKIP_DRAW, NULL);
    }
    StringCopyN(cut, left, sizeof(cut) - 1);
    cut[sizeof(cut) - 1] = EOS;
    for (len = StringLength(cut); len > 1 && GetStringWidth(FONT_SMALL_NARROW, cut, 0) > maxW; len--)
        cut[len - 1] = EOS;
    AddTextPrinterParameterized(sMain, FONT_SMALL_NARROW, cut, 10, y, TEXT_SKIP_DRAW, NULL);
    if (right)
        AddTextPrinterParameterized(sMain, FONT_SMALL_NARROW, right, MAIN_W * 8 - 4 - GetStringWidth(FONT_SMALL_NARROW, right, 0), y, TEXT_SKIP_DRAW, NULL);
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
    AddTextPrinterParameterized(sMain, FONT_SMALL, buf, 10, ROW_Y0 + row * ROW_H, TEXT_SKIP_DRAW, NULL);
}

void MpUi_End(void)
{
    CopyWindowToVram(sMain, COPYWIN_FULL);
}

void MpUi_Help(const u8 *text)
{
    FillWindowPixelBuffer(sHelp, PIXEL_FILL(1));
    AddTextPrinterParameterized(sHelp, FONT_SMALL, text, 2, 1, TEXT_SKIP_DRAW, NULL);
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
    return *scroll;
}

// "x12" style quantity text
void MpUi_Qty(u8 *dst, u32 qty)
{
    dst[0] = CHAR_x;
    ConvertIntToDecimalStringN(dst + 1, qty, STR_CONV_MODE_LEFT_ALIGN, 4);
}
