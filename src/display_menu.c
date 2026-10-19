// Moemon Kai PC: in-game SETTINGS screen (START > SETTINGS): display and controller.
// Uses the game's own window/text system; values change live with Left/Right
// through the platform display module (src/platform/display.c).
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "main.h"
#include "menu.h"
#include "event_object_lock.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "platform/display.h"
#include "platform/gamepad.h"
#include "constants/songs.h"
#include "constants/characters.h"

enum {
    ROW_FULLSCREEN,
    ROW_SCALE,
    ROW_SCALING,
    ROW_FILTER,
    ROW_VSYNC,
    ROW_PAD_SELECT,
    ROW_PAD_LAYOUT,
    ROW_ANALOG_RUN,
    ROW_RUMBLE,
    ROW_CLOSE,
    ROW_COUNT
};

#define LINE_H   11
#define LABEL_X  8
#define VALUE_X  78
#define WIN_W    20

static const struct WindowTemplate sDisplayWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 5,
    .tilemapTop = 1,
    .width = WIN_W,
    .height = 18,
    .paletteNum = 15,
    .baseBlock = 0x8,
};

static const u8 sHintColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void (*sOnClose)(void);

// What the menu currently shows, to redraw when something changes elsewhere
// (F11 while the menu is open, the window being resized, ...)
struct DisplayMenuState
{
    int fullscreen, scale, mode, filter, filterShown, vsync, cursor;
    int padLayout, padRun, padRumble, padConnected, padNintendo, padSelecting, padCount;
    const char *padSelected;
};
static struct DisplayMenuState sShown;

static void AsciiToGame(u8 *dst, const char *src, int maxLen)
{
    int i, n = 0;
    for (i = 0; src[i] && n < maxLen; i++)
    {
        char c = src[i];
        u8 g;
        if (c >= 'A' && c <= 'Z') g = CHAR_A + (c - 'A');
        else if (c >= 'a' && c <= 'z') g = CHAR_a + (c - 'a');
        else if (c >= '0' && c <= '9') g = CHAR_0 + (c - '0');
        else if (c == ' ') g = CHAR_SPACE;
        else if (c == '+') g = CHAR_PLUS;
        else if (c == '.') g = CHAR_PERIOD;
        else if (c == '-') g = CHAR_HYPHEN;
        else if (c == '/') g = CHAR_SLASH;
        else if (c == ':') g = CHAR_COLON;
        else if (c == '=') g = CHAR_EQUALS;
        else if (c == '(') g = CHAR_LEFT_PAREN;
        else if (c == ')') g = CHAR_RIGHT_PAREN;
        else continue;
        dst[n++] = g;
    }
    dst[n] = EOS;
}

static void Print(u8 windowId, u8 fontId, const char *ascii, u8 x, u8 y)
{
    u8 buf[48];
    AsciiToGame(buf, ascii, 46);
    AddTextPrinterParameterized(windowId, fontId, buf, x, y, TEXT_SKIP_DRAW, NULL);
}

static void ReadState(struct DisplayMenuState *s, u8 cursor)
{
    s->fullscreen = Display_GetFullscreen();
    s->scale = Display_GetWindowScale();
    s->mode = Display_GetScaleMode();
    s->filter = Display_GetFilter();
    s->filterShown = Display_IsFilterShown();
    s->vsync = Display_GetVSync();
    s->cursor = cursor;
    s->padLayout = Gamepad_GetLayout();
    s->padRun = Gamepad_GetAnalogRun();
    s->padRumble = Gamepad_GetRumble();
    s->padConnected = Gamepad_IsConnected();
    s->padNintendo = Gamepad_EffectiveNintendoLayout();
    s->padSelecting = Gamepad_IsSelecting();
    s->padCount = Gamepad_Count();
    s->padSelected = Gamepad_SelectedName();
}

static const char *RowValue(const struct DisplayMenuState *s, u8 row, char *buf, int size)
{
    static const char *const scaleNames[DISPLAY_SCALE_COUNT] = {"PIXEL PERFECT", "SHARP FIT", "STRETCH"};
    static const char *const filterNames[DISPLAY_FILTER_COUNT] = {"OFF", "SCANLINES", "LCD GRID"};

    switch (row)
    {
    case ROW_FULLSCREEN: return s->fullscreen ? "ON" : "OFF";
    case ROW_SCALE:      snprintf(buf, size, "%dx", s->scale); return buf;
    case ROW_SCALING:    return scaleNames[s->mode];
    case ROW_FILTER:     return filterNames[s->filter];
    case ROW_VSYNC:      return s->vsync ? "ON" : "OFF";
    case ROW_PAD_SELECT:
        if (s->padSelecting) return "PRESS A BUTTON";
        if (strcmp(s->padSelected, "ALL CONTROLLERS") == 0) return "ALL";
        snprintf(buf, size < 13 ? size : 13, "%s", s->padSelected); // short name; the full one is in the hint
        return buf;
    case ROW_PAD_LAYOUT:
        if (s->padLayout == GAMEPAD_LAYOUT_STANDARD) return "A = BOTTOM";
        if (s->padLayout == GAMEPAD_LAYOUT_NINTENDO) return "A = RIGHT";
        return s->padNintendo ? "AUTO (A = RIGHT)" : "AUTO (A = BOTTOM)";
    case ROW_ANALOG_RUN: return s->padRun ? "ON" : "OFF";
    case ROW_RUMBLE:     return s->padRumble ? "ON" : "OFF";
    }
    return "";
}

static void DrawMenu(u8 windowId, u8 cursor)
{
    static const char *const labels[ROW_COUNT] = {"FULLSCREEN", "WINDOW SCALE", "SCALING", "FILTER", "VSYNC",
                                                  "CONTROLLER", "PAD BUTTONS", "STICK RUN", "RUMBLE", "CLOSE"};
    u8 buf[48];
    char tmp[32];
    u8 row;

    ReadState(&sShown, cursor);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));

    AsciiToGame(buf, "SETTINGS", 46);
    AddTextPrinterParameterized(windowId, FONT_SMALL, buf, (WIN_W * 8 - GetStringWidth(FONT_SMALL, buf, 0)) / 2, 1, TEXT_SKIP_DRAW, NULL);

    for (row = 0; row < ROW_COUNT; row++)
    {
        u8 y = 1 + LINE_H * (row + 1) + 2;
        if (row == cursor)
        {
            u8 arrow[] = {0xEF, EOS}; // cursor
            AddTextPrinterParameterized(windowId, FONT_SMALL, arrow, 1, y, TEXT_SKIP_DRAW, NULL);
        }
        Print(windowId, FONT_SMALL, labels[row], LABEL_X, y);
        if (row != ROW_CLOSE)
        {
            u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
            AsciiToGame(buf, RowValue(&sShown, row, tmp, sizeof(tmp)), 30);
            AddTextPrinterParameterized(windowId, FONT_SMALL, lt, VALUE_X, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL_NARROW, buf, VALUE_X + 7, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL, rt, VALUE_X + 8 + GetStringWidth(FONT_SMALL_NARROW, buf, 0), y, TEXT_SKIP_DRAW, NULL);
        }
    }

    if (sShown.filter != DISPLAY_FILTER_OFF && !sShown.filterShown)
        AsciiToGame(buf, "WINDOW TOO SMALL FOR FILTER", 46);
    else if (sShown.padSelecting)
        AsciiToGame(buf, "PRESS ANY BUTTON ON THE CONTROLLER TO USE", 46);
    else if (cursor == ROW_PAD_SELECT)
    {
        char name[64];
        if (!sShown.padConnected)
            snprintf(name, sizeof(name), "NO CONTROLLER CONNECTED");
        else
            snprintf(name, sizeof(name), "%s  (A: PICK)", sShown.padSelected);
        AsciiToGame(buf, name, 44);
    }
    else if (cursor >= ROW_PAD_LAYOUT && cursor <= ROW_RUMBLE)
    {
        char name[48];
        snprintf(name, sizeof(name), "%s", sShown.padConnected ? Gamepad_Name() : "NO CONTROLLER CONNECTED");
        AsciiToGame(buf, name, 30);
    }
    else
        AsciiToGame(buf, "F11 OR ALT+ENTER: FULLSCREEN", 46);
    AddTextPrinterParameterized3(windowId, FONT_SMALL, (WIN_W * 8 - GetStringWidth(FONT_SMALL, buf, 0)) / 2,
                                 1 + LINE_H * (ROW_COUNT + 1) + 4, sHintColors, TEXT_SKIP_DRAW, buf);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static int Wrap(int v, int n)
{
    return (v % n + n) % n;
}

// dir = +1 / -1
static void ChangeValue(u8 row, int dir)
{
    switch (row)
    {
    case ROW_FULLSCREEN:
        Display_SetFullscreen(!Display_GetFullscreen());
        break;
    case ROW_SCALE:
    {
        int max = Display_GetMaxWindowScale();
        Display_SetWindowScale(Wrap(Display_GetWindowScale() - 1 + dir, max) + 1);
        break;
    }
    case ROW_SCALING:
        Display_SetScaleMode(Wrap(Display_GetScaleMode() + dir, DISPLAY_SCALE_COUNT));
        break;
    case ROW_FILTER:
        Display_SetFilter(Wrap(Display_GetFilter() + dir, DISPLAY_FILTER_COUNT));
        break;
    case ROW_VSYNC:
        Display_SetVSync(!Display_GetVSync());
        break;
    case ROW_PAD_SELECT:
        Gamepad_CycleSelected(dir);
        break;
    case ROW_PAD_LAYOUT:
        Gamepad_SetLayout(Gamepad_GetLayout() + dir);
        break;
    case ROW_ANALOG_RUN:
        Gamepad_SetAnalogRun(!Gamepad_GetAnalogRun());
        break;
    case ROW_RUMBLE:
        Gamepad_SetRumble(!Gamepad_GetRumble());
        Gamepad_Rumble(50, 200);
        break;
    }
}

#define tState    data[0]
#define tCursor   data[1]
#define tWindow   data[2]

static void Task_DisplayMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        tWindow = AddWindow(&sDisplayWindowTemplate);
        DrawStdWindowFrame(tWindow, FALSE);
        DrawMenu(tWindow, tCursor);
        tState++;
        break;
    case 1:
    {
        struct DisplayMenuState now;
        if (Gamepad_IsSelecting())
        {
            // Waiting for a controller button; the keyboard can cancel
            if (JOY_NEW(B_BUTTON | START_BUTTON))
            {
                Gamepad_CancelSelect();
                PlaySE(SE_SELECT);
            }
        }
        else if (tCursor == ROW_PAD_SELECT && JOY_NEW(A_BUTTON))
        {
            Gamepad_StartSelect();
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(DPAD_UP))
        {
            tCursor = (tCursor == 0) ? ROW_COUNT - 1 : tCursor - 1;
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            tCursor = (tCursor + 1) % ROW_COUNT;
            PlaySE(SE_SELECT);
        }
        else if (tCursor != ROW_CLOSE && JOY_NEW(DPAD_LEFT | DPAD_RIGHT | A_BUTTON))
        {
            ChangeValue(tCursor, JOY_NEW(DPAD_LEFT) ? -1 : 1);
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(B_BUTTON | START_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == ROW_CLOSE))
        {
            void (*onClose)(void) = sOnClose;
            PlaySE(SE_SELECT);
            Display_SaveSettings();
            Gamepad_SaveSettings();
            ClearStdWindowAndFrame(tWindow, TRUE);
            RemoveWindow(tWindow);
            DestroyTask(taskId);
            sOnClose = NULL;
            if (onClose)
                onClose();
            return;
        }
        ReadState(&now, tCursor);
        if (now.fullscreen != sShown.fullscreen || now.scale != sShown.scale || now.mode != sShown.mode
         || now.filter != sShown.filter || now.filterShown != sShown.filterShown || now.vsync != sShown.vsync
         || now.cursor != sShown.cursor || now.padLayout != sShown.padLayout || now.padRun != sShown.padRun
         || now.padRumble != sShown.padRumble || now.padConnected != sShown.padConnected || now.padNintendo != sShown.padNintendo
         || now.padSelecting != sShown.padSelecting || now.padCount != sShown.padCount || now.padSelected != sShown.padSelected)
            DrawMenu(tWindow, tCursor);
        break;
    }
    }
}

void Display_OpenSettingsMenu(void (*onClose)(void))
{
    u8 taskId = CreateTask(Task_DisplayMenu, 80);
    sOnClose = onClose;
    gTasks[taskId].tCursor = 0;
}

static void FieldMenuClosed(void)
{
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

void Display_OpenFieldSettingsMenu(void)
{
    LockPlayerFieldControls();
    Display_OpenSettingsMenu(FieldMenuClosed);
}
