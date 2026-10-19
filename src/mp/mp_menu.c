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
#include "region_map.h"
#include "constants/region_map_sections.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// Pages of the CO-OP menu. Rows are per page; each page ends with BACK/CLOSE.
// No CLOSE row: B or START closes the menu
enum { MAIN_EMOTES, MAIN_FRIENDS, MAIN_TRADE, MAIN_SHARED_PC, MAIN_GATCHA, MAIN_DAILY, MAIN_TOURNEY, MAIN_REMINDER, MAIN_LOOK, MAIN_ONLINE, MAIN_COUNT, MAIN_CLOSE = 0xFF };
enum { ROW_SPRITE, ROW_SKIN, ROW_OUTFIT, ROW_ACCENT, ROW_EDIT, ROW_BACK, LOOK_COUNT };
enum { EMO_ICON, EMO_PHRASE1, EMO_PHRASE4 = EMO_PHRASE1 + MP_NUM_PHRASES - 1, EMO_QUICK, EMO_BACK, EMO_COUNT };

#define LINE_H 11
#define VISIBLE_ROWS 9   // more rows than this scroll
extern const u8 EventScript_MpMoveRelearner[];
void MP_Gatcha_Open(void);
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

// Text colors for the next prints (the theme: white on navy, gold when selected)
static u8 sTxtFg = MPT_TEXT, sTxtBg = MPT_BG;

static void MText(u8 windowId, u8 font, const u8 *str, int x, int y)
{
    MpTheme_Print(windowId, font, str, x, y, sTxtFg, sTxtBg);
}

static void PrintAscii(u8 windowId, const char *ascii, u8 x, u8 y)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    MText(windowId, FONT_SMALL, buf, x, y);
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
    MpTheme_Panel(windowId, 0);
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
    {
        // Buddy bonus, and when hosting on this PC the code friends type to join (on the right)
        int rightW = 0;
        if (MP_InviteCode()[0] || MP_ExpBonusPercent())
        {
            u8 code[48];
            char ascii[48];
            if (MP_InviteCode()[0] && MP_ExpBonusPercent())
                snprintf(ascii, sizeof(ascii), "+%d%%  CODE %s", MP_ExpBonusPercent(), MP_InviteCode());
            else if (MP_InviteCode()[0])
                snprintf(ascii, sizeof(ascii), "CODE %s", MP_InviteCode());
            else if (MP_Together_Level())
                snprintf(ascii, sizeof(ascii), "TOGETHER Lv%d  EXP +%d%%", MP_Together_Level(), MP_ExpBonusPercent());
            else
                snprintf(ascii, sizeof(ascii), "EXP +%d%%", MP_ExpBonusPercent());
            MP_AsciiToGame(code, ascii, 44);
            rightW = GetStringWidth(FONT_SMALL, code, 0);
            MText(windowId, FONT_SMALL, code, 28 * 8 - 2 - rightW, 1);
        }
        MpUi_PrintWrapped(windowId, buf, 2, 1, 28 * 8 - 4 - (rightW ? rightW + 8 : 0), 1, sTxtFg, sTxtBg);
    }
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

static u8 RowCount(u8 page)
{
    switch (page)
    {
    case MP_MENU_PAGE_LOOK:   return LOOK_COUNT;
    case MP_MENU_PAGE_EMOTES: return EMO_COUNT;
    case MP_MENU_PAGE_FRIENDS:
    {
        u8 ids[MP_MAX_PLAYERS];
        return MP_Friends_List(ids) + 1;   // + BACK
    }
    default:                  return MAIN_COUNT;
    }
}

static const char *const sIconNames[] = {"!", "?", "HEART", "MUSIC"};

// Prints "label   < value >" on a row
static void PrintChoice(u8 windowId, const char *label, const char *value, u8 y, u8 valueX)
{
    u8 lab[24], val[64];
    u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
    int len;
    MP_AsciiToGame(lab, label, 20);
    MText(windowId, FONT_SMALL_NARROW, lab, 8, y);
    MP_AsciiToGame(val, value, 60);
    // Long names are cut to fit the narrow window
    for (len = StringLength(val); len > 1 && GetStringWidth(FONT_SMALL_NARROW, val, 0) > MENU_W * 8 - valueX - 18; len--)
        val[len - 1] = EOS;
    MText(windowId, FONT_SMALL, lt, valueX, y);
    MText(windowId, FONT_SMALL_NARROW, val, valueX + 7, y);
    MText(windowId, FONT_SMALL, rt, valueX + 8 + GetStringWidth(FONT_SMALL_NARROW, val, 0), y);
}

static void PrintPhraseRow(u8 windowId, u8 slot, u8 y)
{
    u8 text[160], line[80];
    int i, n = 0, len;
    line[n++] = CHAR_1 + slot;
    line[n++] = CHAR_SPACE;
    if (MP_Emote_PhraseIsEmpty(slot))
    {
        MP_AsciiToGame(line + n, "(empty)", 20);
    }
    else
    {
        MP_Emote_PhraseText(slot, text);
        for (i = 0; text[i] != EOS && n < 70; i++)
            line[n++] = (text[i] == CHAR_NEWLINE) ? CHAR_SPACE : text[i];
        line[n] = EOS;
    }
    for (len = StringLength(line); len > 3 && GetStringWidth(FONT_SMALL_NARROW, line, 0) > MENU_W * 8 - 16; len--)
        line[len - 1] = EOS;
    MText(windowId, FONT_SMALL_NARROW, line, 8, y);
}

static u8 sScroll, sPageShown = 0xFF;

// A small triangle at the right edge: more rows above or below
static void DrawScrollArrow(u8 windowId, int y, bool8 up)
{
    int i, cx = MENU_W * 8 - 6;
    for (i = 0; i < 3; i++)
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_TEXT), cx - i, up ? y + i : y + 2 - i, i * 2 + 1, 1);
}

static void DrawMenu(u8 windowId, u8 page, u8 cursor, u8 choice)
{
    static const char *const mainLabels[] = {"SOCIAL...", "GO TO FRIEND...", "TRADE...", "SHARED PC...", "MOEGATCHA...", "DAILY...", "TOURNAMENT...", "MOVE TUTOR...", "LOOK...", ""};
    static const char *const lookLabels[] = {"SPRITE", "SKIN", "OUTFIT", "ACCENT", "EDIT SPRITES...", "BACK"};
    char line[64];
    u8 row;

    MpTheme_Panel(windowId, 12);
    if (page == MP_MENU_PAGE_LOOK)
        snprintf(line, sizeof(line), "LOOK");
    else if (page == MP_MENU_PAGE_EMOTES)
        snprintf(line, sizeof(line), "EMOTES");
    else if (page == MP_MENU_PAGE_FRIENDS)
        snprintf(line, sizeof(line), "GO TO FRIEND");
    else if (MP_IsConnected())
        snprintf(line, sizeof(line), "CO-OP  %d/%d ONLINE", MP_ConnectedCount(), MP_MAX_PLAYERS);
    else
        snprintf(line, sizeof(line), "CO-OP  OFFLINE");
    sTxtFg = MPT_GOLD;
    sTxtBg = MPT_BAND;
    PrintAscii(windowId, line, 4, 0);
    sTxtFg = MPT_TEXT;
    sTxtBg = MPT_BG;

    // Keep the cursor in view when the page has more rows than fit
    {
        u8 count = RowCount(page);
        if (sPageShown != page)
            sScroll = 0;
        sPageShown = page;
        if (cursor < sScroll)
            sScroll = cursor;
        if (cursor >= sScroll + VISIBLE_ROWS)
            sScroll = cursor - VISIBLE_ROWS + 1;
        if (count <= VISIBLE_ROWS)
            sScroll = 0;
        if (sScroll > 0)
            DrawScrollArrow(windowId, LINE_H + 3, TRUE);
        if (sScroll + VISIBLE_ROWS < count)
            DrawScrollArrow(windowId, 1 + LINE_H * (VISIBLE_ROWS + 1) + 2, FALSE);
    }
    for (row = sScroll; row < RowCount(page) && row < sScroll + VISIBLE_ROWS; row++)
    {
        u8 y = 1 + LINE_H * (row - sScroll + 1) + 2;
        sTxtFg = MPT_TEXT;
        sTxtBg = MPT_BG;
        if (row == cursor)
        {
            // The game's own cursor
            u8 arrow[] = {0xEF /* ▶ */, EOS};
            MText(windowId, FONT_SMALL, arrow, 1, y);
        }
        if (page == MP_MENU_PAGE_MAIN)
        {
            if (row == MAIN_ONLINE)
                PrintAscii(windowId, MP_Lobby_InRoom() ? "SERVER..." : "GO ONLINE...", 8, y);
            else
                PrintAscii(windowId, mainLabels[row], 8, y);
        }
        else if (page == MP_MENU_PAGE_FRIENDS)
        {
            u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids);
            if (row < n)
            {
                u8 name[PLAYER_NAME_LENGTH + 1], where[40];
                int len;
                StringCopyN(name, gMpPlayers[ids[row]].name, PLAYER_NAME_LENGTH);
                name[PLAYER_NAME_LENGTH] = EOS;
                MText(windowId, FONT_SMALL_NARROW, name, 8, y);
                MP_Friends_LocationName(ids[row], where);
                for (len = StringLength(where); len > 1 && GetStringWidth(FONT_SMALL_NARROW, where, 0) > MENU_W * 8 - 46; len--)
                    where[len - 1] = EOS;
                MText(windowId, FONT_SMALL_NARROW, where, 42, y);
            }
            else
                PrintAscii(windowId, "BACK", 8, y);
        }
        else if (page == MP_MENU_PAGE_EMOTES)
        {
            if (row == EMO_ICON)
                PrintChoice(windowId, "EMOTE", sIconNames[row == cursor ? choice % 4 : 0], y, 38);
            else if (row >= EMO_PHRASE1 && row <= EMO_PHRASE4)
                PrintPhraseRow(windowId, row - EMO_PHRASE1, y);
            else if (row == EMO_QUICK)
                PrintChoice(windowId, "SAY", MP_Emote_QuickPhrase(row == cursor ? choice : 0), y, 24);
            else
                PrintAscii(windowId, "BACK", 8, y);
        }
        else if (row == ROW_EDIT || row == ROW_BACK)
        {
            PrintAscii(windowId, lookLabels[row], 8, y);
        }
        else
        {
            switch (row)
            {
            case ROW_SPRITE: snprintf(line, sizeof(line), "%s", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app))); break;
            case ROW_SKIN:   snprintf(line, sizeof(line), "%s", MP_Avatar_SkinName(*RowValue(row))); break;
            default:         snprintf(line, sizeof(line), "%s", MP_Avatar_ColorName(*RowValue(row))); break;
            }
            PrintChoice(windowId, lookLabels[row], line, y, 38);
        }
    }
    sTxtFg = MPT_TEXT;
    sTxtBg = MPT_BG;
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

// The bottom window: players online, or button help on the EMOTES page
static void DrawBottom(u8 windowId, u8 page, u8 cursor)
{
    if (page == MP_MENU_PAGE_FRIENDS)
    {
        u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids), buf[96], town[32];
        MpTheme_Panel(windowId, 0);
        if (n == 0)
            MP_AsciiToGame(buf, "Nobody else is online right now.", 60);
        else if (cursor >= n)
            MP_AsciiToGame(buf, "Fly to a friend's town (towns you've been to).", 70);
        else
        {
            u16 dest = MP_Friends_Destination(ids[cursor]);
            if (dest >= MAPSEC_NONE)
                MP_AsciiToGame(buf, "No town you've been to is near them.", 60);
            else
            {
                MP_AsciiToGame(buf, "A: FLY TO ", 20);
                GetMapName(town, dest, 0);
                StringAppend(buf, town);
            }
        }
        MpUi_PrintWrapped(windowId, buf, 2, 1, 28 * 8 - 4, 1, sTxtFg, sTxtBg);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
    else if (page == MP_MENU_PAGE_EMOTES)
    {
        u8 buf[80];
        MpTheme_Panel(windowId, 0);
        MP_AsciiToGame(buf, "A: SAY IT   SELECT: EDIT PHRASE   KEYS 1-8", 70);
        MpUi_PrintWrapped(windowId, buf, 2, 1, 28 * 8 - 4, 1, sTxtFg, sTxtBg);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
    else
        DrawPlayersWindow(windowId);
}

#define tState    data[0]
#define tCursor   data[1]
#define tWindow   data[2]
#define tPlayers  data[3]
#define tRefresh  data[4]
#define tPage     data[5]
#define tChoice   data[6]   // left/right value on the EMOTE and SAY rows
#define tEditSlot data[7]

static void CloseMenuWindows(s16 *data)
{
    ClearStdWindowAndFrame(tWindow, TRUE);
    ClearStdWindowAndFrame(tPlayers, TRUE);
    RemoveWindow(tWindow);
    RemoveWindow(tPlayers);
    MpTheme_Pop();
}

static void CloseMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    MP_SaveConfig();
    CloseMenuWindows(data);
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
}

static void GoToPage(s16 *data, u8 page, u8 cursor)
{
    tPage = page;
    tCursor = cursor;
    tChoice = 0;
    DrawMenu(tWindow, tPage, tCursor, tChoice);
    DrawBottom(tPlayers, tPage, tCursor);
}

static void HandleLookInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    bool8 changed = FALSE;

    if (tCursor == ROW_EDIT && JOY_NEW(A_BUTTON))
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
        CloseMenu(taskId);
        MP_ShowNotice(msg);
        return;
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
    else if (tCursor >= ROW_SKIN && tCursor <= ROW_ACCENT && JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        u8 *v = RowValue(tCursor);
        u8 n = NumChoices(tCursor);
        *v = JOY_NEW(DPAD_RIGHT) ? (*v + 1) % n : (*v + n - 1) % n;
        changed = TRUE;
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == ROW_BACK))
    {
        PlaySE(SE_SELECT);
        MP_SaveConfig();
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_LOOK);
    }
    if (changed)
    {
        PlaySE(SE_SELECT);
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        DrawMenu(tWindow, tPage, tCursor, tChoice);
    }
}

static void HandleEmoteInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 nQuick = MP_Emote_QuickPhraseCount();

    if ((tCursor == EMO_ICON || tCursor == EMO_QUICK) && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        u8 n = (tCursor == EMO_ICON) ? 4 : nQuick;
        tChoice = JOY_REPEAT(DPAD_RIGHT) ? (tChoice + 1) % n : (tChoice + n - 1) % n;
        PlaySE(SE_SELECT);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
    }
    else if (tCursor >= EMO_PHRASE1 && tCursor <= EMO_PHRASE4
          && (JOY_NEW(SELECT_BUTTON) || (JOY_NEW(A_BUTTON) && MP_Emote_PhraseIsEmpty(tCursor - EMO_PHRASE1))))
    {
        // To the word editor
        PlaySE(SE_SELECT);
        MP_SaveConfig();
        tEditSlot = tCursor - EMO_PHRASE1;
        CloseMenuWindows(data);
        FadeScreen(FADE_TO_BLACK, 0);
        tState = 3;
    }
    else if (JOY_NEW(A_BUTTON) && tCursor != EMO_BACK)
    {
        u8 cursor = tCursor, choice = tChoice;
        CloseMenu(taskId);   // close first so you see your own bubble
        if (cursor == EMO_ICON)
            MP_Emote_Hotkey(choice % 4);
        else if (cursor == EMO_QUICK)
            MP_Emote_SayQuick(choice);
        else
            MP_Emote_SayPhrase(cursor - EMO_PHRASE1);
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == EMO_BACK))
    {
        PlaySE(SE_SELECT);
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_EMOTES);
    }
}

static void HandleFriendsInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids);
    if (tCursor > n)
        tCursor = n; // someone left
    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == n))
    {
        PlaySE(SE_SELECT);
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_FRIENDS);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        if (MP_Friends_Destination(ids[tCursor]) >= MAPSEC_NONE)
        {
            PlaySE(SE_FAILURE);
            return;
        }
        CloseMenu(taskId);
        MP_Friends_GoTo(ids[tCursor]);
    }
}

static void HandleMainInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        return;
    if (JOY_NEW(B_BUTTON | START_BUTTON) || tCursor == MAIN_CLOSE)
    {
        PlaySE(SE_SELECT);
        CloseMenu(taskId);
        return;
    }
    PlaySE(SE_SELECT);
    switch (tCursor)
    {
    case MAIN_EMOTES:
        CloseMenu(taskId);
        MP_Social_Open();
        break;
    case MAIN_TRADE:
        CloseMenu(taskId);
        MP_Trade_OpenMenu();
        break;
    case MAIN_SHARED_PC:
        CloseMenu(taskId);
        MP_SharedPc_Open();
        break;
    case MAIN_DAILY:
        CloseMenu(taskId);
        MP_Daily_OpenMenu();
        break;
    case MAIN_GATCHA:
        CloseMenu(taskId);
        MP_Gatcha_Open();
        break;
    case MAIN_TOURNEY:
        CloseMenu(taskId);
        MP_Tourney_OpenMenu();
        break;
    case MAIN_LOOK:
        GoToPage(data, MP_MENU_PAGE_LOOK, ROW_SPRITE);
        break;
    case MAIN_REMINDER:
        // The Move Tutor's service, anywhere (still costs a HEART SCALE)
        CloseMenu(taskId);
        ScriptContext_SetupScript(EventScript_MpMoveRelearner);
        break;
    case MAIN_FRIENDS:
        GoToPage(data, MP_MENU_PAGE_FRIENDS, 0);
        break;
    case MAIN_ONLINE:
        MP_SaveConfig();
        CloseMenuWindows(data);
        FadeScreen(FADE_TO_BLACK, 0);
        tState = 2;
        break;
    }
}

static void Task_CoopMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        tWindow = AddWindow(&sCoopWindowTemplate);
        tPlayers = AddWindow(&sPlayersWindowTemplate);
        // The map name sign shares the frame's palette slot: load the player's frame again
        HideMapNamePopUpWindow();
        LoadMessageBoxAndBorderGfx();
        MpTheme_Push();
        DrawStdWindowFrame(tWindow, FALSE);
        DrawStdWindowFrame(tPlayers, FALSE);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        tState++;
        break;
    case 1:
        if (++tRefresh >= 60)
        {
            tRefresh = 0;
            DrawBottom(tPlayers, tPage, tCursor);
            DrawMenu(tWindow, tPage, tCursor, tChoice);
        }
        if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
        {
            u8 n = RowCount(tPage);
            tCursor = JOY_NEW(DPAD_UP) ? (tCursor == 0 ? n - 1 : tCursor - 1) : (tCursor + 1) % n;
            tChoice = 0;
            PlaySE(SE_SELECT);
            DrawMenu(tWindow, tPage, tCursor, tChoice);
            if (tPage == MP_MENU_PAGE_FRIENDS)
                DrawBottom(tPlayers, tPage, tCursor);
        }
        else if (tPage == MP_MENU_PAGE_FRIENDS)
            HandleFriendsInput(taskId);
        else if (tPage == MP_MENU_PAGE_LOOK)
            HandleLookInput(taskId);
        else if (tPage == MP_MENU_PAGE_EMOTES)
            HandleEmoteInput(taskId);
        else
            HandleMainInput(taskId);
        break;
    case 2: // to the HOST / JOIN screen
        if (!gPaletteFade.active)
        {
            CleanupOverworldWindowsAndTilemaps();
            gMain.savedCallback = CB2_ReturnToFieldWithOpenMenu;
            DestroyTask(taskId);
            SetMainCallback2(CB2_InitCoopSetupFromField);
        }
        break;
    case 3: // to the word editor for a phrase
        if (!gPaletteFade.active)
        {
            u8 slot = tEditSlot;
            DestroyTask(taskId);
            MP_Emote_EditPhrase(slot);
        }
        break;
    }
}

void MP_OpenMenuPageAt(u8 page, u8 cursor)
{
    u8 taskId;
    MP_OpenMenuPage(page);
    taskId = FindTaskIdByFunc(Task_CoopMenu);
    if (taskId != TASK_NONE && cursor < RowCount(page))
        gTasks[taskId].tCursor = cursor;
}

void MP_OpenMenuPage(u8 page)
{
    u8 taskId;
    LockPlayerFieldControls();
    HideMapNamePopUpWindow(); // it shares window tiles with this menu
    MP_Avatar_RefreshPackList();   // pick up sheets saved since the last time
    taskId = CreateTask(Task_CoopMenu, 80);
    gTasks[taskId].tPage = page;
    if (page == MP_MENU_PAGE_MAIN)
        gTasks[taskId].tCursor = MP_Lobby_InRoom() ? MAIN_EMOTES : MAIN_ONLINE;
}

// Back from the MOEGATCHA screen
void MP_OpenMenuAtGatcha(void)
{
    MP_OpenMenuPageAt(MP_MENU_PAGE_MAIN, MAIN_GATCHA);
}

void MP_OpenMenu(void)
{
    MP_OpenMenuPage(MP_MENU_PAGE_MAIN);
}
