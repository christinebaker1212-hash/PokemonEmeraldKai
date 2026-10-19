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
#include "field_screen_effect.h"
#include "field_weather.h"
#include "constants/field_weather.h"
#include "overworld.h"
#include "start_menu.h"
#include "map_name_popup.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

enum {
    ROW_ONLINE,
    ROW_SPRITE,
    ROW_SKIN,
    ROW_OUTFIT,
    ROW_ACCENT,
    ROW_EDIT,
    ROW_CLOSE,
    ROW_COUNT
};

#define LINE_H 14
#define MENU_W 12   // tiles; the window sits right of the player so you can see your look change

static const struct WindowTemplate sCoopWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 17,
    .tilemapTop = 1,
    .width = MENU_W,
    .height = 15,
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
    .baseBlock = 0x8 + MENU_W * 15,
};

static const u8 sTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void PrintAscii(u8 windowId, const char *ascii, u8 x, u8 y)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    AddTextPrinterParameterized(windowId, FONT_SMALL, buf, x, y, TEXT_SKIP_DRAW, NULL);
}

static u16 NumChoices(u8 row)
{
    switch (row)
    {
    case ROW_SPRITE: return MP_Avatar_ChoiceCount();
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
    static const char *const labels[] = {"", "SPRITE", "SKIN", "OUTFIT", "ACCENT", "EDIT SPRITES...", "SAVE AND CLOSE"};
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
        if (row == ROW_CLOSE || row == ROW_EDIT)
        {
            PrintAscii(windowId, labels[row], 9, y);
            continue;
        }
        if (row == ROW_ONLINE)
        {
            PrintAscii(windowId, MP_Lobby_InRoom() ? "SERVER..." : "GO ONLINE...", 9, y);
            continue;
        }
        {
            u8 lab[16];
            MP_AsciiToGame(lab, labels[row], 14);
            AddTextPrinterParameterized(windowId, FONT_SMALL_NARROW, lab, 7, y, TEXT_SKIP_DRAW, NULL);
        }
        switch (row)
        {
        case ROW_SPRITE: snprintf(line, sizeof(line), "%s", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app))); break;
        case ROW_SKIN:   snprintf(line, sizeof(line), "%s", MP_Avatar_SkinName(*RowValue(row))); break;
        default:         snprintf(line, sizeof(line), "%s", MP_Avatar_ColorName(*RowValue(row))); break;
        }
        {
            u8 val[40];
            u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
            int len;
            MP_AsciiToGame(val, line, 30);
            // Long names are cut to fit the narrow window
            for (len = StringLength(val); len > 1 && GetStringWidth(FONT_SMALL_NARROW, val, 0) > MENU_W * 8 - 56; len--)
                val[len - 1] = EOS;
            AddTextPrinterParameterized(windowId, FONT_SMALL, lt, 38, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL_NARROW, val, 45, y, TEXT_SKIP_DRAW, NULL);
            AddTextPrinterParameterized(windowId, FONT_SMALL, rt, 46 + GetStringWidth(FONT_SMALL_NARROW, val, 0), y, TEXT_SKIP_DRAW, NULL);
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
        else if (tCursor == ROW_EDIT && JOY_NEW(A_BUTTON))
        {
            // Save the current character as an editable sheet, then open the
            // editor and the sprites folder
            extern int Platform_OpenPath(const char *relPath);
            char path[200], msg[300];
            PlaySE(SE_SELECT);
            MP_SaveConfig();
            if (MP_Avatar_ExportLocal(path, sizeof(path)))
            {
                Platform_OpenPath("Sprite Editor.html");
                Platform_OpenPath("sprites");
                snprintf(msg, sizeof(msg), "Your look was saved in the game's\nsprites folder for the Sprite Editor.\fEdit it there, then drag the saved\nfile onto this window to wear it!");
            }
            else
                snprintf(msg, sizeof(msg), "Couldn't save the sprite sheet. Is the game folder read-only?");
            ClearStdWindowAndFrame(tWindow, TRUE);
            ClearStdWindowAndFrame(tPlayers, TRUE);
            RemoveWindow(tWindow);
            RemoveWindow(tPlayers);
            ScriptUnfreezeObjectEvents();
            UnlockPlayerFieldControls();
            DestroyTask(taskId);
            MP_ShowNotice(msg);
            return;
        }
        else if (tCursor == ROW_ONLINE && JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            MP_SaveConfig();
            ClearStdWindowAndFrame(tWindow, TRUE);
            ClearStdWindowAndFrame(tPlayers, TRUE);
            RemoveWindow(tWindow);
            RemoveWindow(tPlayers);
            FadeScreen(FADE_TO_BLACK, 0);
            tState = 2;
            break;
        }
        else if (tCursor == ROW_SPRITE && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
        {
            // Many sprites: Left/Right steps (hold to scroll), L/R jumps by 10
            u16 n = NumChoices(ROW_SPRITE);
            u16 c = MP_Avatar_GetChoice(&gMpConfig.app);
            u16 step = JOY_REPEAT(L_BUTTON | R_BUTTON) ? 10 : 1;
            if (JOY_REPEAT(DPAD_RIGHT | R_BUTTON))
                c = (c + step) % n;
            else
                c = (c + n - (step % n)) % n;
            MP_Avatar_SetChoice(&gMpConfig.app, c);
            changed = TRUE;
        }
        else if (tCursor != ROW_CLOSE && tCursor != ROW_ONLINE && tCursor != ROW_SPRITE && tCursor != ROW_EDIT && JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
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
    case 2: // to the HOST / JOIN screen
        if (!gPaletteFade.active)
        {
            CleanupOverworldWindowsAndTilemaps();
            gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;
            DestroyTask(taskId);
            SetMainCallback2(CB2_InitCoopSetupFromField);
        }
        break;
    }
}

void MP_OpenMenu(void)
{
    u8 taskId;
    LockPlayerFieldControls();
    HideMapNamePopUpWindow(); // it shares window tiles with this menu
    MP_Avatar_RefreshPackList();   // pick up sheets saved since the last time
    taskId = CreateTask(Task_CoopMenu, 80);
    gTasks[taskId].tCursor = MP_Lobby_InRoom() ? ROW_SPRITE : ROW_ONLINE;
}
