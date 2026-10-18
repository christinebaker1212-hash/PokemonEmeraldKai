// Moemon Co-op: CO-OP start menu entry (status + character customization)
#include <stdio.h>
#include "global.h"
#include "mp.h"
#include "main.h"
#include "menu.h"
#include "event_object_lock.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/songs.h"
#include "constants/characters.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

enum {
    ROW_SPRITE,
    ROW_SKIN,
    ROW_OUTFIT,
    ROW_ACCENT,
    ROW_CLOSE,
    ROW_COUNT
};

#define LINE_H 14

static const struct WindowTemplate sCoopWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 17,
    .tilemapTop = 1,
    .width = 12,
    .height = 14,
    .paletteNum = 15,
    .baseBlock = 0x8,
};

static const struct WindowTemplate sPlayersWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 17,
    .width = 28,
    .height = 2,
    .paletteNum = 15,
    .baseBlock = 0x8 + 13 * 14,
};

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void PrintAscii(u8 windowId, const char *ascii, u8 x, u8 y)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    AddTextPrinterParameterized(windowId, FONT_SMALL, buf, x, y, TEXT_SKIP_DRAW, NULL);
}

static u8 NumChoices(u8 row)
{
    switch (row)
    {
    case ROW_SPRITE: return MP_Avatar_LocalCustomBlob() ? MP_BASE_COUNT : MP_BASE_COUNT - 1;
    case ROW_SKIN:   return MP_SKIN_COUNT;
    default:         return MP_COLOR_COUNT;
    }
}

static u8 *RowValue(u8 row)
{
    switch (row)
    {
    case ROW_SPRITE: return &gMpConfig.app.base;
    case ROW_SKIN:   return &gMpConfig.app.skin;
    case ROW_OUTFIT: return &gMpConfig.app.outfit;
    default:         return &gMpConfig.app.accent;
    }
}

static void DrawPlayersWindow(u8 windowId)
{
    u8 buf[128];
    u8 i, n = 0;
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    buf[0] = EOS;
    if (!MP_IsConnected())
    {
        MP_AsciiToGame(buf, MP_StatusText(), 60);
    }
    else
    {
        for (i = 0; i < MP_MAX_PLAYERS; i++)
        {
            u8 lvl[8];
            const u8 *name;
            u8 level;
            if (i == gMpLocalId)
            {
                name = gSaveBlock2Ptr->playerName;
                level = MP_GetPartyTopLevel();
            }
            else if (gMpPlayers[i].present && gMpPlayers[i].hasInfo)
            {
                name = gMpPlayers[i].name;
                level = gMpPlayers[i].hasState ? gMpPlayers[i].state.topLevel : 0;
            }
            else
                continue;
            if (n++)
            {
                u8 sep[] = {CHAR_SPACE, CHAR_SPACE, EOS};
                StringAppend(buf, sep);
            }
            StringAppendN(buf, name, PLAYER_NAME_LENGTH);
            {
                u8 lv[] = {CHAR_SPACE, CHAR_LV, EOS};
                StringAppend(buf, lv);
            }
            ConvertIntToDecimalStringN(lvl, level, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(buf, lvl);
        }
    }
    AddTextPrinterParameterized(windowId, FONT_SMALL, buf, 2, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static void DrawMenu(u8 windowId, u8 cursor)
{
    static const char *const labels[] = {"SPRITE", "SKIN", "OUTFIT", "ACCENT", "SAVE AND CLOSE"};
    char line[64];
    u8 row;

    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    if (MP_IsConnected())
        snprintf(line, sizeof(line), "CO-OP  %d/%d ONLINE", MP_ConnectedCount(), MP_MAX_PLAYERS);
    else
        snprintf(line, sizeof(line), "CO-OP  OFFLINE");
    PrintAscii(windowId, line, 2, 1);

    for (row = 0; row < ROW_COUNT; row++)
    {
        u8 y = 1 + LINE_H * (row + 1) + 2;
        if (row == cursor)
        {
            u8 arrow[] = {0xEF, EOS};
            AddTextPrinterParameterized(windowId, FONT_SMALL, arrow, 1, y, TEXT_SKIP_DRAW, NULL);
        }
        if (row == ROW_CLOSE)
        {
            PrintAscii(windowId, labels[row], 9, y);
            continue;
        }
        snprintf(line, sizeof(line), "%s", labels[row]);
        PrintAscii(windowId, line, 8, y);
        switch (row)
        {
        case ROW_SPRITE: snprintf(line, sizeof(line), "%s", MP_Avatar_BaseName(*RowValue(row))); break;
        case ROW_SKIN:   snprintf(line, sizeof(line), "%s", MP_Avatar_SkinName(*RowValue(row))); break;
        default:         snprintf(line, sizeof(line), "%s", MP_Avatar_ColorName(*RowValue(row))); break;
        }
        {
            u8 val[40];
            u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
            MP_AsciiToGame(val, line, 30);
            AddTextPrinterParameterized(windowId, FONT_SMALL, lt, 40, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL_NARROW, val, 47, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL, rt, 48 + GetStringWidth(FONT_SMALL_NARROW, val, 0), y, TEXT_SKIP_DRAW, NULL);
        }
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

#define tState    data[0]
#define tCursor   data[1]
#define tWindow   data[2]
#define tPlayers  data[3]
#define tRefresh  data[4]

static void Task_CoopMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        tWindow = AddWindow(&sCoopWindowTemplate);
        tPlayers = AddWindow(&sPlayersWindowTemplate);
        DrawStdWindowFrame(tWindow, FALSE);
        DrawStdWindowFrame(tPlayers, FALSE);
        DrawMenu(tWindow, tCursor);
        DrawPlayersWindow(tPlayers);
        tState++;
        break;
    case 1:
    {
        bool8 changed = FALSE;
        if (++tRefresh >= 60)
        {
            tRefresh = 0;
            DrawPlayersWindow(tPlayers);
            DrawMenu(tWindow, tCursor);
        }
        if (JOY_NEW(DPAD_UP))
        {
            tCursor = (tCursor == 0) ? ROW_COUNT - 1 : tCursor - 1;
            PlaySE(SE_SELECT);
            DrawMenu(tWindow, tCursor);
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            tCursor = (tCursor + 1) % ROW_COUNT;
            PlaySE(SE_SELECT);
            DrawMenu(tWindow, tCursor);
        }
        else if (tCursor != ROW_CLOSE && JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
        {
            u8 *v = RowValue(tCursor);
            u8 n = NumChoices(tCursor);
            *v = JOY_NEW(DPAD_RIGHT) ? (*v + 1) % n : (*v + n - 1) % n;
            changed = TRUE;
        }
        else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == ROW_CLOSE) || JOY_NEW(START_BUTTON))
        {
            PlaySE(SE_SELECT);
            MP_SaveConfig();
            ClearStdWindowAndFrame(tWindow, TRUE);
            ClearStdWindowAndFrame(tPlayers, TRUE);
            RemoveWindow(tWindow);
            RemoveWindow(tPlayers);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            return;
        }
        if (changed)
        {
            PlaySE(SE_SELECT);
            MP_Avatar_ApplyLocal();
            MP_SendAppearance();
            DrawMenu(tWindow, tCursor);
        }
        break;
    }
    }
}

void MP_OpenMenu(void)
{
    u8 taskId;
    LockPlayerFieldControls();
    taskId = CreateTask(Task_CoopMenu, 80);
    gTasks[taskId].tCursor = 0;
}
