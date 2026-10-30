// Moemon Co-op: the HOST / JOIN / PLAY SOLO screen shown after pressing START
// on the title screen. Hosting needs only a server name (and an optional
// password); joining is picking a server from the list. Servers live on the
// online lobby built into the game, or on the local network as a fallback.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp_net.h"
#include "global.h"
#include "mp.h"
#include "bg.h"
#include "gpu_regs.h"
#include "main.h"
#include "main_menu.h"
#include "menu.h"
#include "naming_screen.h"
#include "walda_phrase.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define FRAME_TILE   0x1F0
#define FRAME_PAL    14
#define TEXT_PAL     15
#define LINE_H       16
#define HOST_BACK_X  150  // BACK, on the START HOSTING line
#define HOST_LINE_H  11   // the host form has more rows (START HOSTING and BACK share the last one)
#define LIST_ROWS    5
#define REFRESH_FRAMES (60 * 3)
#define LOCAL_PORT_TRIES 5

enum {
    WIN_TITLE,
    WIN_MAIN,
    WIN_HELP,
    WIN_FULL,      // PATCH NOTES reader: the whole screen under the title (only shown there)
    WIN_COUNT
};

static const struct WindowTemplate sWindowTemplates[] = {
    // The title sits on the background (no frame); the others are framed boxes
    [WIN_TITLE] = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 0, .width = 28, .height = 2, .paletteNum = TEXT_PAL, .baseBlock = 1 },
    [WIN_MAIN]  = { .bg = 0, .tilemapLeft = 2, .tilemapTop = 3, .width = 26, .height = 10, .paletteNum = TEXT_PAL, .baseBlock = 1 + 56 },
    [WIN_HELP]  = { .bg = 0, .tilemapLeft = 2, .tilemapTop = 15, .width = 26, .height = 4, .paletteNum = TEXT_PAL, .baseBlock = 1 + 56 + 260 },
    // past the frame's tiles (FRAME_TILE): 28 x 16 = 448 tiles, up to the tilemap at 0xF800 (tile 960)
    [WIN_FULL]  = { .bg = 0, .tilemapLeft = 1, .tilemapTop = 3, .width = 28, .height = 16, .paletteNum = TEXT_PAL, .baseBlock = 512 },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgTemplates[] = {
    { .bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },
};

static const u16 sBgColor[] = {RGB(8, 14, 22)};
// Dark text with a light shadow on the white boxes; white text on the background
static const u8 sColorNormal[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sColorDim[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};     // help text: same contrast
static const u8 sColorDisabled[] = {TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};
static const u8 sColorAccent[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_GRAY};
static const u8 sColorError[] = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_GRAY};
// The host form's rows sit closer than a glyph cell: no background fill, so a row never covers the one above
static const u8 sHostNormal[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sHostAccent[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_GRAY};
static const u8 sHostDisabled[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};
static const u8 sHostArrow[] = {CHAR_RIGHT_ARROW, EOS};
static const u8 sColorTitle[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

// Screens
enum {
    SCR_MAIN,
    SCR_HOST,
    SCR_JOIN,
    SCR_WORKING,
    SCR_MESSAGE,
    SCR_PATCHES,       // PATCH NOTES: the versions
    SCR_PATCHTEXT,     // ... one version's notes, full screen
    SCR_RULES,         // HOST A GAME > GAME RULES
};

// Main screen choices
enum {
    MAIN_CONTINUE,     // already in a server (came back to the title screen)
    MAIN_LEAVE,
    MAIN_REJOIN,
    MAIN_HOST,
    MAIN_JOIN,
    MAIN_SOLO,
    MAIN_PATCHES,      // PATCH NOTES (only here, before the game starts)
};

// Host form rows
enum {
    HOST_NAME,
    HOST_PASSWORD,
    HOST_NOTE,
    HOST_LISTED,
    HOST_RULES,        // GAME RULES: team padding, battle speed, level scaling (SCR_RULES)
    HOST_START,
    HOST_BACK,
    HOST_ROWS
};

// What a "working" screen is waiting for
enum {
    WORK_HOST_ONLINE,
    WORK_HOST_LOCAL,
    WORK_JOIN,
    WORK_CODE,          // connecting to an invite code
};

// Where to go after a message
enum {
    AFTER_MAIN,
    AFTER_HOST,
    AFTER_JOIN,
    AFTER_PLAY,
};

#define MAX_JOIN_ENTRIES (1 + MP_LIST_MAX + MP_LAN_MAX_GAMES)

// PATCH NOTES
#define PATCH_LINES_MAX  128
#define PATCH_FONT       FONT_SMALL
#define PATCH_ROW_H      12
#define PATCH_ROWS       10      // rows on the reader's page (WIN_FULL is 128 px tall)
#define PATCH_TEXT_W     202     // WIN_FULL's 224 px less the margins and the scroll arrows' column (x 214)

struct PatchRow
{
    char text[80];
    u8 x;
    u8 kind;    // 'V' version title, 'H' heading, 'B' / 'S' items, 'P' paragraph
    bool8 mark; // the item's first row: the dash in front
};

struct JoinEntry
{
    struct MpLanRoom room;
    bool8 isLan;
    bool8 byName;
    bool8 byCode;
};

static EWRAM_DATA struct
{
    bool8 active;
    bool8 fromField;           // opened from CO-OP menu during the game
    u8 screen;
    u8 cursor;
    u8 scroll;
    u8 mainItems[6];
    u8 mainCount;
    // host form
    u8 roomName[WALDA_PHRASE_LENGTH + 1];      // game charset
    u8 password[WALDA_PHRASE_LENGTH + 1];
    bool8 listed;
    u8 hostCursor;
    u8 rulesCursor;
    // join list
    struct JoinEntry entries[MAX_JOIN_ENTRIES];
    u8 entryCount;
    u8 joinCursor;
    bool8 onlineTried;
    bool8 onlineFailed;
    u16 refreshTimer;
    u8 pendingJoin;            // entry index waiting for a password
    // working / message
    u8 work;
    u16 workTimer;
    u8 after;
    u8 message[240];
    u8 inviteCode[WALDA_PHRASE_LENGTH + 1];
    char joiningCode[24];      // the invite code being joined (remembered once we're in: gMpConfig.lastJoinCode)
    bool8 rejoining;           // REJOIN to a friend's own server (its own message when it can't be reached)
    bool8 triedKey;            // joining a locked room with the key remembered from last time (wrong: ask the password)
    u8 note[2][WALDA_PHRASE_LENGTH + 1];       // welcome note lines, game charset
    char codeHost[24];
    int codePort;
    bool8 messageIsError;
    // text entry return
    u8 returnTo;               // 0 none, 1 = host name, 2 = host password, 3 = join password, 4 = join by name, 5 = join by name password
    // PATCH NOTES
    u8 patchLineCount;
    struct { u16 first; u8 count; } patchLine[PATCH_LINES_MAX];   // a version line ("1.17"): its notes, newest first
    u8 patchCursor;
    u8 patchScroll;
    struct PatchRow *rows;     // the line's notes, laid out (malloc)
    u16 rowCount;
    u16 rowTop;
} sSetup;

static void Task_Setup(u8 taskId);
static void DrawScreen(void);
static void CB2_ReturnFromTextEntry(void);

// ---------------------------------------------------------------------------
// Text helpers

static void Print(u8 win, const char *ascii, u8 x, u8 y, const u8 *color)
{
    u8 buf[128];
    MP_AsciiToGame(buf, ascii, 120);
    AddTextPrinterParameterized3(win, FONT_NORMAL, x, y, color, TEXT_SKIP_DRAW, buf);
}

static void PrintGame(u8 win, const u8 *str, u8 x, u8 y, const u8 *color)
{
    AddTextPrinterParameterized3(win, FONT_NORMAL, x, y, color, TEXT_SKIP_DRAW, str);
}

static void PrintRight(u8 win, const char *ascii, u8 rightX, u8 y, const u8 *color)
{
    u8 buf[64];
    MP_AsciiToGame(buf, ascii, 60);
    AddTextPrinterParameterized3(win, FONT_NORMAL, rightX - GetStringWidth(FONT_NORMAL, buf, 0), y, color, TEXT_SKIP_DRAW, buf);
}

static void DrawCursor(u8 win, u8 y)
{
    u8 arrow[] = {CHAR_RIGHT_ARROW, EOS};
    AddTextPrinterParameterized3(win, FONT_NORMAL, 2, y, sColorNormal, TEXT_SKIP_DRAW, arrow);
}

// Word-wraps an ASCII message into the help/main window
static void PrintWrapped(u8 win, const char *ascii, u8 y, u8 width, const u8 *color)
{
    char line[96];
    const char *p = ascii;
    while (*p)
    {
        int n = 0, lastSpace = -1;
        u8 buf[100];
        while (p[n] && p[n] != '\n')
        {
            line[n] = p[n];
            line[n + 1] = 0;
            MP_AsciiToGame(buf, line, 95);
            if (GetStringWidth(FONT_NORMAL, buf, 0) > width && lastSpace > 0)
            {
                n = lastSpace;
                break;
            }
            if (p[n] == ' ')
                lastSpace = n;
            n++;
            if (n >= 90)
                break;
        }
        line[n] = 0;
        Print(win, line, 4, y, color);
        y += LINE_H;
        p += n;
        while (*p == ' ' || *p == '\n')
            p++;
    }
}

static void GameToAsciiTrim(char *dst, const u8 *src, int max)
{
    int n;
    MP_GameToAscii(dst, src, max);
    // trim spaces
    n = (int)strlen(dst);
    while (n > 0 && dst[n - 1] == ' ')
        dst[--n] = 0;
    if (dst[0] == ' ')
    {
        int i = 0;
        while (dst[i] == ' ')
            i++;
        memmove(dst, dst + i, strlen(dst + i) + 1);
    }
}

static bool8 HasSaveFile(void)
{
    return gSaveBlock2Ptr->playerName[0] != EOS && gSaveBlock2Ptr->playerName[0] != 0;
}

static void DefaultRoomName(void)
{
    char ascii[32];
    if (gMpConfig.lastHostName[0])
    {
        MP_AsciiToGame(sSetup.roomName, gMpConfig.lastHostName, WALDA_PHRASE_LENGTH);
        return;
    }
    if (HasSaveFile())
    {
        char name[16];
        MP_GameToAscii(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        GameToAsciiTrim(name, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
        snprintf(ascii, sizeof(ascii), "%s's Game", name);
    }
    else
        snprintf(ascii, sizeof(ascii), "Moemon Co-op");
    MP_AsciiToGame(sSetup.roomName, ascii, WALDA_PHRASE_LENGTH);
}

// ---------------------------------------------------------------------------
// Screen setup

static void VBlankCB_Setup(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Setup(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void InitGraphics(void)
{
    u8 i;
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
    DmaClear32(3, OAM, OAM_SIZE);
    DmaClear16(3, PLTT, PLTT_SIZE);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    InitWindows(sWindowTemplates);
    DeactivateAllTextPrinters();
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    FreeAllSpritePalettes();
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);
    LoadPalette(sBgColor, BG_PLTT_ID(0), sizeof(sBgColor));
    LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PAL), PLTT_SIZE_4BPP);
    LoadUserWindowBorderGfx(WIN_TITLE, FRAME_TILE, BG_PLTT_ID(FRAME_PAL));
    for (i = 0; i < WIN_FULL; i++)   // WIN_FULL only shows in the PATCH NOTES reader
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(i == WIN_TITLE ? 0 : 1));
        PutWindowTilemap(i);
        if (i != WIN_TITLE)
            DrawStdFrameWithCustomTileAndPalette(i, FALSE, FRAME_TILE, FRAME_PAL);
    }
    CopyBgTilemapBufferToVram(0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Setup);
    SetMainCallback2(CB2_Setup);
    CreateTask(Task_Setup, 0);
}

// ---------------------------------------------------------------------------
// Main screen

static void BuildMainItems(void)
{
    sSetup.mainCount = 0;
    if (MP_Lobby_InRoom() || MP_Lobby_State() == MP_LOBBY_RECONNECT_WAIT)
    {
        sSetup.mainItems[sSetup.mainCount++] = MAIN_CONTINUE;
        sSetup.mainItems[sSetup.mainCount++] = MAIN_LEAVE;
        if (!sSetup.fromField)
            sSetup.mainItems[sSetup.mainCount++] = MAIN_PATCHES;
        return;
    }
    if (gMpConfig.lastRoom[0] && gMpConfig.lastRoomKey[0] && strcmp(gMpConfig.lastRoomAddr, "home") != 0)
        sSetup.mainItems[sSetup.mainCount++] = MAIN_REJOIN;
    sSetup.mainItems[sSetup.mainCount++] = MAIN_HOST;
    sSetup.mainItems[sSetup.mainCount++] = MAIN_JOIN;
    sSetup.mainItems[sSetup.mainCount++] = MAIN_SOLO;   // "BACK" when opened during the game
    if (!sSetup.fromField)
        sSetup.mainItems[sSetup.mainCount++] = MAIN_PATCHES;   // what changed in each version (not during the game)
}

static void MainItemLabel(u8 item, char *out, int len)
{
    switch (item)
    {
    case MAIN_CONTINUE: snprintf(out, len, sSetup.fromField ? "BACK TO THE GAME" : "CONTINUE"); break;
    case MAIN_LEAVE:    snprintf(out, len, "LEAVE THE SERVER"); break;
    case MAIN_REJOIN:   snprintf(out, len, "REJOIN %s", gMpConfig.lastRoom); break;
    case MAIN_HOST:     snprintf(out, len, "HOST A GAME"); break;
    case MAIN_JOIN:     snprintf(out, len, "JOIN A GAME"); break;
    case MAIN_PATCHES:  snprintf(out, len, "PATCH NOTES"); break;
    default:            snprintf(out, len, sSetup.fromField ? "BACK" : "PLAY SOLO"); break;
    }
}

static const char *MainItemHelp(u8 item)
{
    switch (item)
    {
    case MAIN_CONTINUE:
        if (MP_InviteCode()[0])
        {
            static char sHelp[96];
            snprintf(sHelp, sizeof(sHelp), "Keep playing together. Your invite code for friends: %s", MP_InviteCode());
            return sHelp;
        }
        return "Keep playing together on this server.";
    case MAIN_LEAVE:    return "Disconnect and choose again.";
    case MAIN_REJOIN:   return "Join the server you played on last time.";
    case MAIN_HOST:     return "Open a server. Pick a name and an optional password.";
    case MAIN_JOIN:     return "Find a friend's server and hop in.";
    case MAIN_PATCHES:  return "What changed in each version, back to the very first build.";
    default:            return sSetup.fromField ? "Return to the game." : "Play alone. Go online later with {COOP}.";
    }
}

static void DrawMain(void)
{
    u8 i;
    char label[48];

    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    if (MP_Lobby_InRoom())
    {
        snprintf(label, sizeof(label), "You're on %s", MP_Lobby_RoomName());
        Print(WIN_MAIN, label, 4, 1, sColorAccent);
    }
    for (i = 0; i < sSetup.mainCount; i++)
    {
        u8 y = 1 + LINE_H * (i + (MP_Lobby_InRoom() ? 1 : 0));
        MainItemLabel(sSetup.mainItems[i], label, sizeof(label));
        Print(WIN_MAIN, label, 14, y, sColorNormal);
        if (i == sSetup.cursor)
            DrawCursor(WIN_MAIN, y);
    }
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    PrintWrapped(WIN_HELP, MainItemHelp(sSetup.mainItems[sSetup.cursor]), 0, 200, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

// ---------------------------------------------------------------------------
// Messages and working screens

static void ShowMessage(const char *ascii, bool8 isError, u8 after)
{
    snprintf((char *)sSetup.message, sizeof(sSetup.message), "%s", ascii);
    sSetup.messageIsError = isError;
    sSetup.after = after;
    sSetup.screen = SCR_MESSAGE;
    PlaySE(isError ? SE_FAILURE : SE_SUCCESS);
    DrawScreen();
}

static void StartWorking(u8 work)
{
    sSetup.work = work;
    sSetup.workTimer = 0;
    sSetup.screen = SCR_WORKING;
    DrawScreen();
}

static void DrawWorking(void)
{
    char text[96];
    static const char *const dots[] = {"", ".", "..", "..."};
    char room[MP_ROOM_NAME_MAX + 1];

    GameToAsciiTrim(room, sSetup.roomName, WALDA_PHRASE_LENGTH);
    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    switch (sSetup.work)
    {
    case WORK_HOST_ONLINE:
    case WORK_HOST_LOCAL:
        snprintf(text, sizeof(text), "Opening %s%s", room, dots[(sSetup.workTimer / 20) % 4]);
        break;
    case WORK_CODE:
        snprintf(text, sizeof(text), "Finding your friend's game%s", dots[(sSetup.workTimer / 20) % 4]);
        break;
    default:
        snprintf(text, sizeof(text), "Joining %s%s", MP_Lobby_RoomName()[0] ? MP_Lobby_RoomName() : "server", dots[(sSetup.workTimer / 20) % 4]);
        break;
    }
    if (sSetup.work == WORK_HOST_LOCAL && MP_Lobby_State() == MP_LOBBY_IN_ROOM)
        snprintf(text, sizeof(text), "Opening your game to the internet%s", dots[(sSetup.workTimer / 20) % 4]);
    Print(WIN_MAIN, text, 4, 1 + LINE_H, sColorNormal);
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    Print(WIN_HELP, "{B} Cancel", 4, 1, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static void DrawMessage(void)
{
    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    PrintWrapped(WIN_MAIN, (const char *)sSetup.message, 1, 196, sSetup.messageIsError ? sColorError : sColorNormal);
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    Print(WIN_HELP, "{A} OK", 4, 1, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

// Which side is out of date, when the other side told us its version
static const char *VersionText(void)
{
    static char text[160];
    u16 other = MP_Lobby_OtherVersion();
    if (other > MP_PROTOCOL_VERSION)
        snprintf(text, sizeof(text), "Your game is out of date (co-op v%d, they have v%d). Update the game to join.", MP_PROTOCOL_VERSION, other);
    else if (other != 0)
        snprintf(text, sizeof(text), "That game is out of date (co-op v%d, you have v%d). The host needs to update.", other, MP_PROTOCOL_VERSION);
    else
        snprintf(text, sizeof(text), "That server is running a different version of the game (you have co-op v%d). Everyone needs the same version.", MP_PROTOCOL_VERSION);
    return text;
}

static const char *LobbyErrorText(u8 err)
{
    switch (err)
    {
    case MP_LOBBY_ERR_NAME_TAKEN:   return "Someone is already using that server name. Try a different one!";
    case MP_LOBBY_ERR_NOT_FOUND:    return "That server isn't open anymore. Check the name, or ask your friend to host again.";
    case MP_LOBBY_ERR_BAD_PASSWORD: return "Wrong password. Ask the host for it and try again.";
    case MP_LOBBY_ERR_FULL:         return "That server is full (4 players).";
    case MP_LOBBY_ERR_VERSION:      return VersionText();
    case MP_LOBBY_ERR_BAD_NAME:     return "Server names can use letters, numbers, spaces and - . ! ? ' &";
    case MP_LOBBY_ERR_SLOW_DOWN:    return "Too many wrong passwords. Wait a few seconds and try again.";
    case MP_LOBBY_ERR_SERVER_FULL:  return "The online lobby is busy right now. Try again in a minute.";
    default:                        return "Couldn't connect. Check your internet connection and try again.";
    }
}

// ---------------------------------------------------------------------------
// Host form

static void DrawHost(void)
{
    u8 i;
    static const char *const labels[] = {"SERVER NAME", "PASSWORD", "WELCOME NOTE", "SHOW IN LIST", "GAME RULES", "START HOSTING", "BACK"};
    static const char *const help[] = {
        "The name your friends will look for.",
        "Optional. Friends need it to join.",
        "Optional. Two short lines your friends see when they join.",
        "NO hides it: friends must type the exact name.",
        "Team padding, battle speed and level scaling, for everyone.",
        "Open the server and start playing!",
        "Go back to HOST / JOIN.",
    };

    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    for (i = 0; i < HOST_ROWS; i++)
    {
        u8 y = 1 + HOST_LINE_H * (i > HOST_START ? HOST_START : i);
        u8 x = (i == HOST_BACK) ? HOST_BACK_X : 14;
        Print(WIN_MAIN, labels[i], x, y, sHostNormal);
        if (i == HOST_NAME)
            PrintGame(WIN_MAIN, sSetup.roomName, 104, y, sHostAccent);
        else if (i == HOST_PASSWORD)
        {
            if (sSetup.password[0] == EOS)
                Print(WIN_MAIN, "(none)", 104, y, sHostDisabled);
            else
                PrintGame(WIN_MAIN, sSetup.password, 104, y, sHostAccent);
        }
        else if (i == HOST_NOTE)
        {
            if (sSetup.note[0][0] == EOS && sSetup.note[1][0] == EOS)
                Print(WIN_MAIN, "(none)", 104, y, sHostDisabled);
            else
            {
                u8 shown[WALDA_PHRASE_LENGTH + 4];
                static const u8 sDots[] = {CHAR_PERIOD, CHAR_PERIOD, EOS};
                StringCopy(shown, sSetup.note[0][0] != EOS ? sSetup.note[0] : sSetup.note[1]);
                if (sSetup.note[0][0] != EOS && sSetup.note[1][0] != EOS)
                    StringAppend(shown, sDots);
                PrintGame(WIN_MAIN, shown, 104, y, sHostAccent);
            }
        }
        else if (i == HOST_LISTED)
        {
            u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
            PrintGame(WIN_MAIN, lt, 104, y, sHostNormal);
            Print(WIN_MAIN, sSetup.listed ? "YES" : "NO", 114, y, sHostAccent);
            PrintGame(WIN_MAIN, rt, 140, y, sHostNormal);
        }
        else if (i == HOST_RULES)
        {
            Print(WIN_MAIN, MP_LevelScaleOwnRule(MP_SCALE_ON) ? "SCALING ON" : "SCALING OFF", 104, y, sHostAccent);
        }
        if (i == sSetup.hostCursor)
            AddTextPrinterParameterized3(WIN_MAIN, FONT_NORMAL, x - 12, y, sHostNormal, TEXT_SKIP_DRAW, sHostArrow);
    }
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    PrintWrapped(WIN_HELP, help[sSetup.hostCursor], 0, 200, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static bool8 StartLocalServer(void)
{
    int i;
    char addr[32];
    for (i = 0; i < LOCAL_PORT_TRIES; i++)
    {
        int port = MP_DEFAULT_PORT + i;
        if (MPServer_IsRunning() || MPServer_Start(port, TRUE) == 0)
        {
            snprintf(addr, sizeof(addr), "127.0.0.1:%d", port);
            MP_Lobby_Connect(addr, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}

// The welcome note lives in the ini as "line 1|line 2"
static void LoadNote(void)
{
    char line[2][sizeof(gMpConfig.welcome)];
    char *bar;
    snprintf(line[0], sizeof(line[0]), "%s", gMpConfig.welcome);
    bar = strchr(line[0], '|');
    line[1][0] = 0;
    if (bar)
    {
        snprintf(line[1], sizeof(line[1]), "%s", bar + 1);
        *bar = 0;
    }
    MP_AsciiToGame(sSetup.note[0], line[0], WALDA_PHRASE_LENGTH);
    MP_AsciiToGame(sSetup.note[1], line[1], WALDA_PHRASE_LENGTH);
}

static void SaveNote(void)
{
    char a[WALDA_PHRASE_LENGTH + 1], b[WALDA_PHRASE_LENGTH + 1];
    GameToAsciiTrim(a, sSetup.note[0], WALDA_PHRASE_LENGTH);
    GameToAsciiTrim(b, sSetup.note[1], WALDA_PHRASE_LENGTH);
    if (b[0])
        snprintf(gMpConfig.welcome, sizeof(gMpConfig.welcome), "%s|%s", a, b);
    else
        snprintf(gMpConfig.welcome, sizeof(gMpConfig.welcome), "%s", a);
    MP_SaveConfig();
}

static void BeginHosting(bool8 online)
{
    char room[MP_ROOM_NAME_MAX + 1], password[WALDA_PHRASE_LENGTH + 1];
    GameToAsciiTrim(room, sSetup.roomName, WALDA_PHRASE_LENGTH);
    MP_GameToAscii(password, sSetup.password, WALDA_PHRASE_LENGTH);

    if (!MPNet_IsValidRoomName(room))
    {
        ShowMessage(LobbyErrorText(MP_LOBBY_ERR_BAD_NAME), TRUE, AFTER_HOST);
        return;
    }
    snprintf(gMpConfig.lastHostName, sizeof(gMpConfig.lastHostName), "%s", room);
    if (online)
    {
        MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
        MP_Lobby_CreateRoom(room, password, !sSetup.listed);
        StartWorking(WORK_HOST_ONLINE);
    }
    else
    {
        if (!StartLocalServer())
        {
            ShowMessage("Couldn't start a server on this PC. Is another copy of the game already hosting?", TRUE, AFTER_HOST);
            return;
        }
        MP_Lobby_CreateRoom(room, password, !sSetup.listed);
        MPPortMap_Open(MPServer_Port()); // open the router so friends anywhere can join
        StartWorking(WORK_HOST_LOCAL);
    }
}

// ---------------------------------------------------------------------------
// Join list

static void RefreshJoinList(void)
{
    struct MpLanRoom tmp[MP_LIST_MAX > MP_LAN_MAX_GAMES ? MP_LIST_MAX : MP_LAN_MAX_GAMES];
    int n, i;
    u8 prevCount = sSetup.entryCount;

    sSetup.entryCount = 0;
    // First rows: an invite code from a friend, or a hidden server's name
    memset(&sSetup.entries[0], 0, sizeof(sSetup.entries[0]) * 2);
    sSetup.entries[0].byCode = TRUE;
    sSetup.entries[1].byName = TRUE;
    sSetup.entryCount = 2;

    if (MP_Lobby_State() == MP_LOBBY_READY || MP_Lobby_ListReady())
    {
        n = MP_Lobby_GetList(tmp, MP_LIST_MAX);
        for (i = 0; i < n && sSetup.entryCount < MAX_JOIN_ENTRIES; i++)
        {
            struct JoinEntry *e = &sSetup.entries[sSetup.entryCount++];
            memset(e, 0, sizeof(*e));
            e->room = tmp[i];
        }
    }
    n = MPLan_Poll(tmp, MP_LAN_MAX_GAMES);
    for (i = 0; i < n && sSetup.entryCount < MAX_JOIN_ENTRIES; i++)
    {
        struct JoinEntry *e;
        int k;
        bool8 dup = FALSE;
        // A server hosted by this PC through the online lobby also answers here? No:
        // only local servers answer, but skip exact duplicates just in case.
        for (k = 2; k < sSetup.entryCount; k++)
            if (sSetup.entries[k].isLan && strcmp(sSetup.entries[k].room.name, tmp[i].name) == 0)
                dup = TRUE;
        if (dup)
            continue;
        e = &sSetup.entries[sSetup.entryCount++];
        memset(e, 0, sizeof(*e));
        e->room = tmp[i];
        e->isLan = TRUE;
    }
    if (sSetup.joinCursor >= sSetup.entryCount)
        sSetup.joinCursor = sSetup.entryCount - 1;
    // Start on the first server when the list fills in for the first time
    if (prevCount <= 2 && sSetup.entryCount > 2 && sSetup.joinCursor == 0)
        sSetup.joinCursor = 2;
}

static void DrawJoin(void)
{
    u8 i;
    char line[48];
    const char *help;

    if (sSetup.joinCursor < sSetup.scroll)
        sSetup.scroll = sSetup.joinCursor;
    if (sSetup.joinCursor >= sSetup.scroll + LIST_ROWS)
        sSetup.scroll = sSetup.joinCursor - LIST_ROWS + 1;

    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    for (i = 0; i < LIST_ROWS && sSetup.scroll + i < sSetup.entryCount; i++)
    {
        struct JoinEntry *e = &sSetup.entries[sSetup.scroll + i];
        u8 y = 1 + LINE_H * i;
        if (e->byCode)
            Print(WIN_MAIN, "ENTER CODE OR IP...", 14, y, sColorAccent);
        else if (e->byName)
            Print(WIN_MAIN, "JOIN BY NAME...", 14, y, sColorAccent);
        else
        {
            const u8 *color = (e->room.version != MP_PROTOCOL_VERSION || e->room.players >= e->room.maxPlayers) ? sColorDisabled : sColorNormal;
            Print(WIN_MAIN, e->room.name, 14, y, color);
            if (e->room.version > MP_PROTOCOL_VERSION)
                snprintf(line, sizeof(line), "NEWER");
            else if (e->room.version != MP_PROTOCOL_VERSION)
                snprintf(line, sizeof(line), "OLD");
            else
                snprintf(line, sizeof(line), "%s%d/%d", e->isLan ? "HOME " : "", e->room.players, e->room.maxPlayers);
            PrintRight(WIN_MAIN, line, 196, y, color);
            if (e->room.flags & MP_ROOM_FLAG_LOCKED)
            {
                u8 lock[16];
                MP_AsciiToGame(lock, "LOCKED", 10);
                AddTextPrinterParameterized3(WIN_MAIN, FONT_SMALL, 118, y + 1, sColorDim, TEXT_SKIP_DRAW, lock);
            }
        }
        if (sSetup.scroll + i == sSetup.joinCursor)
            DrawCursor(WIN_MAIN, y);
    }
    if (sSetup.scroll > 0)
    {
        u8 up[] = {CHAR_UP_ARROW, EOS};
        PrintGame(WIN_MAIN, up, 200, 1, sColorDim);
    }
    if (sSetup.scroll + LIST_ROWS < sSetup.entryCount)
    {
        u8 down[] = {CHAR_DOWN_ARROW, EOS};
        PrintGame(WIN_MAIN, down, 200, 1 + LINE_H * (LIST_ROWS - 1), sColorDim);
    }
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);

    if (sSetup.joinCursor == 0)
        help = "Type the invite code your friend sent you (like K7QM-3XPA). Works from anywhere.";
    else if (sSetup.joinCursor == 1)
        help = "For hidden servers: type the exact name.";
    else if (sSetup.entryCount <= 2)
    {
        if (sSetup.onlineFailed || !MP_HasOnlineLobby())
            help = "No servers found on your home network yet. Still looking...";
        else
            help = "Looking for servers... Ask a friend to host one!";
    }
    else if (sSetup.onlineFailed)
        help = "Online lobby unreachable. Showing home network only.";
    else
        help = "{A} Join  {B} Back   LOCKED servers need a password.";
    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    PrintWrapped(WIN_HELP, help, 0, 200, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static void EnterJoinList(void)
{
    sSetup.joiningCode[0] = 0;
    sSetup.rejoining = FALSE;
    sSetup.triedKey = FALSE;
    sSetup.screen = SCR_JOIN;
    sSetup.joinCursor = 0;
    sSetup.scroll = 0;
    sSetup.entryCount = 0;
    sSetup.onlineFailed = FALSE;
    sSetup.refreshTimer = 0;
    MPLan_StartSearch();
    if (MP_HasOnlineLobby())
    {
        if (MP_Lobby_State() != MP_LOBBY_READY)
            MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
        MP_Lobby_RequestList();
    }
    RefreshJoinList();
    DrawScreen();
}

// The key of the room we were last in (from the ini)
static void LastRoomKey(u8 *key)
{
    int i;
    for (i = 0; i < MP_ROOM_KEY_SIZE; i++)
    {
        unsigned v = 0;
        sscanf(gMpConfig.lastRoomKey + i * 2, "%2x", &v);
        key[i] = (u8)v;
    }
}

static void JoinEntryNow(u8 index)
{
    struct JoinEntry *e = &sSetup.entries[index];
    char password[WALDA_PHRASE_LENGTH + 1];
    char addr[96];

    MP_GameToAscii(password, sSetup.password, WALDA_PHRASE_LENGTH);
    if (e->isLan)
    {
        snprintf(addr, sizeof(addr), "%s:%d", e->room.host, e->room.port);
        MP_Lobby_Connect(addr, FALSE);
    }
    else if (MP_Lobby_State() != MP_LOBBY_READY)
        MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
    MP_Lobby_JoinRoom(e->room.name, password);
    MP_AsciiToGame(sSetup.roomName, e->room.name, WALDA_PHRASE_LENGTH);
    StartWorking(WORK_JOIN);
}

// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// PATCH NOTES: every version back to the first build (mp_patchnotes.c, from tools/release/notes). The list shows each
// version line ("1.17") with its latest summary; A opens the line's notes full screen, oldest update first.

static void PrintFont(u8 win, u8 font, const char *ascii, u8 x, u8 y, const u8 *color)
{
    u8 buf[128];
    MP_AsciiToGame(buf, ascii, 120);
    AddTextPrinterParameterized3(win, font, x, y, color, TEXT_SKIP_DRAW, buf);
}

static u16 AsciiWidth(u8 font, const char *ascii)
{
    u8 buf[128];
    MP_AsciiToGame(buf, ascii, 120);
    return GetStringWidth(font, buf, 0);
}

// "1.17.2" -> "1.17"
static void PatchLineName(const char *version, char *out, int len)
{
    int i, dots = 0;
    for (i = 0; version[i] && i < len - 1; i++)
    {
        if (version[i] == '.' && ++dots == 2)
            break;
        out[i] = version[i];
    }
    out[i] = 0;
}

static void BuildPatchLines(void)
{
    u16 i, n = MP_PatchNotes_Count();
    char prev[16] = "", name[16];

    sSetup.patchLineCount = 0;
    for (i = 0; i < n; i++)
    {
        PatchLineName(MP_PatchNotes_Get(i)->version, name, sizeof(name));
        if (sSetup.patchLineCount > 0 && strcmp(name, prev) == 0)
            sSetup.patchLine[sSetup.patchLineCount - 1].count++;
        else if (sSetup.patchLineCount < PATCH_LINES_MAX)
        {
            sSetup.patchLine[sSetup.patchLineCount].first = i;
            sSetup.patchLine[sSetup.patchLineCount].count = 1;
            sSetup.patchLineCount++;
        }
        else
            break;
        strcpy(prev, name);
    }
}

// Cuts an ASCII line to fit `width` pixels, ending it with "..."
static void FitAscii(char *text, u8 font, u16 width)
{
    int n = strlen(text);
    if (AsciiWidth(font, text) <= width)
        return;
    while (n > 0)
    {
        n--;
        while (n > 0 && text[n - 1] == ' ')
            n--;
        if ((u8)text[n] >= 0x80 && n > 0 && (u8)text[n - 1] == 0xC3)   // never split an é
            n--;
        strcpy(text + n, "...");
        if (AsciiWidth(font, text) <= width)
            return;
    }
}

static void DrawPatches(void)
{
    u8 r;
    char label[96];
    u8 arrowUp[] = {CHAR_UP_ARROW, EOS}, arrowDown[] = {CHAR_DOWN_ARROW, EOS};

    FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(1));
    for (r = 0; r < LIST_ROWS && sSetup.patchScroll + r < sSetup.patchLineCount; r++)
    {
        u8 idx = sSetup.patchScroll + r, y = 1 + LINE_H * r;
        const struct MpPatchNote *latest = MP_PatchNotes_Get(sSetup.patchLine[idx].first);
        const struct MpPatchNote *build = MP_PatchNotes_Get(sSetup.patchLine[idx].first + sSetup.patchLine[idx].count - 1);
        PatchLineName(latest->version, label, sizeof(label));
        Print(WIN_MAIN, label, 14, y, idx == 0 ? sColorAccent : sColorNormal);
        snprintf(label, sizeof(label), "%s", build->title);   // the line's own build (x.y.0): its title
        FitAscii(label, FONT_NORMAL, 196 - 50);
        Print(WIN_MAIN, label, 50, y, sColorNormal);
        if (idx == sSetup.patchCursor)
            DrawCursor(WIN_MAIN, y);
    }
    if (sSetup.patchScroll > 0)
        PrintGame(WIN_MAIN, arrowUp, 198, 1, sColorAccent);
    if (sSetup.patchScroll + LIST_ROWS < sSetup.patchLineCount)
        PrintGame(WIN_MAIN, arrowDown, 198, 1 + LINE_H * (LIST_ROWS - 1), sColorAccent);
    CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);

    FillWindowPixelBuffer(WIN_HELP, PIXEL_FILL(1));
    if (sSetup.patchLineCount > 0)
    {
        u16 first = sSetup.patchLine[sSetup.patchCursor].first;
        u8 count = sSetup.patchLine[sSetup.patchCursor].count;
        if (count == 1)
            snprintf(label, sizeof(label), "1 update: %s", MP_PatchNotes_Get(first)->version);
        else
            snprintf(label, sizeof(label), "%d updates: %s to %s", count, MP_PatchNotes_Get(first + count - 1)->version,
                     MP_PatchNotes_Get(first)->version);
        Print(WIN_HELP, label, 4, 1, sColorDim);
    }
    Print(WIN_HELP, "{A} READ    {B} BACK", 4, 1 + LINE_H, sColorDim);
    CopyWindowToVram(WIN_HELP, COPYWIN_GFX);
}

static void FreePatchRows(void)
{
    free(sSetup.rows);
    sSetup.rows = NULL;
    sSetup.rowCount = 0;
    sSetup.rowTop = 0;
}

static void AddPatchRow(const char *text, u8 x, u8 kind, bool8 mark)
{
    struct PatchRow *row;
    if (sSetup.rowCount % 64 == 0)
    {
        struct PatchRow *more = realloc(sSetup.rows, (sSetup.rowCount + 64) * sizeof(struct PatchRow));
        if (more == NULL)
            return;
        sSetup.rows = more;
    }
    row = &sSetup.rows[sSetup.rowCount++];
    snprintf(row->text, sizeof(row->text), "%s", text);
    row->x = x;
    row->kind = kind;
    row->mark = mark;
}

// Word-wraps one item of the notes into rows starting at x
static void WrapPatchItem(const char *src, int len, u8 x, u8 kind)
{
    char line[80], cand[80];
    u16 width = PATCH_TEXT_W - x;
    bool8 first = TRUE;
    int i = 0;

    line[0] = 0;
    while (i < len)
    {
        int start, wordLen;
        while (i < len && src[i] == ' ')
            i++;
        start = i;
        while (i < len && src[i] != ' ')
            i++;
        wordLen = i - start;
        if (wordLen == 0)
            break;
        if (wordLen > 70)
            wordLen = 70;
        if (line[0])
            snprintf(cand, sizeof(cand), "%s %.*s", line, wordLen, src + start);
        else
            snprintf(cand, sizeof(cand), "%.*s", wordLen, src + start);
        if (line[0] && (AsciiWidth(PATCH_FONT, cand) > width || strlen(cand) >= sizeof(line) - 1))
        {
            AddPatchRow(line, x, kind, first && (kind == 'B' || kind == 'S'));
            first = FALSE;
            snprintf(line, sizeof(line), "%.*s", wordLen, src + start);
        }
        else
            strcpy(line, cand);
    }
    if (line[0])
        AddPatchRow(line, x, kind, first && (kind == 'B' || kind == 'S'));
}

static void BuildPatchText(u8 lineIndex)
{
    u16 first = sSetup.patchLine[lineIndex].first;
    u8 count = sSetup.patchLine[lineIndex].count;
    int k;
    char name[16];

    FreePatchRows();
    for (k = count - 1; k >= 0; k--)   // oldest update first
    {
        const struct MpPatchNote *note = MP_PatchNotes_Get(first + k);
        const char *p = note->body;
        char title[160];

        snprintf(title, sizeof(title), "%s  %s", note->version, note->title);
        WrapPatchItem(title, strlen(title), 0, 'V');
        WrapPatchItem(note->summary, strlen(note->summary), 0, 'P');
        while (*p)
        {
            const char *end = strchr(p, '\n');
            int len = end ? (int)(end - p) : (int)strlen(p);
            char kind = *p;
            if (len > 1)
            {
                if (kind == 'H')
                {
                    char upper[64];
                    int j, n = len - 1 < (int)sizeof(upper) - 1 ? len - 1 : (int)sizeof(upper) - 1;
                    for (j = 0; j < n; j++)
                        upper[j] = (p[1 + j] >= 'a' && p[1 + j] <= 'z') ? p[1 + j] - 'a' + 'A' : p[1 + j];
                    upper[n] = 0;
                    WrapPatchItem(upper, n, 0, 'H');
                }
                else
                    WrapPatchItem(p + 1, len - 1, kind == 'B' ? 8 : kind == 'S' ? 18 : 0, kind == 'B' || kind == 'S' ? kind : 'P');
            }
            p += len;
            if (*p == '\n')
                p++;
        }
        if (k > 0)
            AddPatchRow("", 0, 'P', FALSE);   // a gap before the next update
    }
    PatchLineName(MP_PatchNotes_Get(first)->version, name, sizeof(name));
    MPNet_Log("patch notes: line %s, %d notes, %d rows", name, count, sSetup.rowCount);
}

static void DrawPatchText(void)
{
    u8 r;
    u8 arrowUp[] = {CHAR_UP_ARROW, EOS}, arrowDown[] = {CHAR_DOWN_ARROW, EOS};

    FillWindowPixelBuffer(WIN_FULL, PIXEL_FILL(1));
    for (r = 0; r < PATCH_ROWS && sSetup.rowTop + r < sSetup.rowCount; r++)
    {
        const struct PatchRow *row = &sSetup.rows[sSetup.rowTop + r];
        u8 y = 2 + PATCH_ROW_H * r, x = 4 + row->x;
        const u8 *color = row->kind == 'V' ? sColorAccent : row->kind == 'H' ? sColorError : sColorNormal;
        if (row->mark)
            PrintFont(WIN_FULL, PATCH_FONT, "-", x - 7, y, sColorNormal);
        PrintFont(WIN_FULL, PATCH_FONT, row->text, x, y, color);
    }
    if (sSetup.rowTop > 0)
        PrintGame(WIN_FULL, arrowUp, 214, 0, sColorAccent);
    if (sSetup.rowTop + PATCH_ROWS < sSetup.rowCount)
        PrintGame(WIN_FULL, arrowDown, 214, 112, sColorAccent);
    CopyWindowToVram(WIN_FULL, COPYWIN_GFX);
}

// The reader covers the whole screen under the title; leaving it brings the usual two boxes back
static void ShowPatchReader(bool8 on)
{
    FillBgTilemapBufferRect(0, 0, 0, 2, 30, 18, 0);
    if (on)
    {
        PutWindowTilemap(WIN_FULL);
        DrawStdFrameWithCustomTileAndPalette(WIN_FULL, FALSE, FRAME_TILE, FRAME_PAL);
    }
    else
    {
        PutWindowTilemap(WIN_MAIN);
        PutWindowTilemap(WIN_HELP);
        DrawStdFrameWithCustomTileAndPalette(WIN_MAIN, FALSE, FRAME_TILE, FRAME_PAL);
        DrawStdFrameWithCustomTileAndPalette(WIN_HELP, FALSE, FRAME_TILE, FRAME_PAL);
    }
    CopyBgTilemapBufferToVram(0);   // (this screen runs no scheduled copies)
}

// ---------------------------------------------------------------------------
// HOST A GAME > GAME RULES: the room's rules, for everyone (TEAM PADDING, ANIM SPEED and LEVEL SCALING's settings;
// playing alone, OPTIONS > GAME has the level scaling ones)

enum { RULE_PADDING, RULE_ANIM, RULE_SCALE_FIRST, RULE_GEN = RULE_SCALE_FIRST + MP_SCALE_COUNT, RULE_GIMMICKS, RULE_BACK, RULE_ROWS };
#define RULE_LINE_H 10

static void RuleValue(u8 row, char *out, int len)
{
    static const char *const sWild[] = {"OFF", "LOW", "MEDIUM", "HIGH"};
    static const char *const sTrainers[] = {"OFF", "BELOW", "EVEN", "ABOVE"};
    static const char *const sPast8[] = {"TO LEAGUE", "FREE", "ALWAYS CAPPED"};
    u8 v;
    if (row == RULE_PADDING)
        snprintf(out, len, "%s", gMpConfig.trainerPadding ? "ON" : "OFF");
    else if (row == RULE_ANIM)
        snprintf(out, len, "%s", gMpConfig.animSpeed == 30 ? "+30%" : gMpConfig.animSpeed == 25 ? "+25%" : "NORMAL");
    else if (row == RULE_GEN)
        snprintf(out, len, "GEN %d", gMpConfig.generation >= 3 && gMpConfig.generation <= 9 ? gMpConfig.generation : 3);
    else if (row == RULE_GIMMICKS)   // (each one in OPTIONS > GAME)
        snprintf(out, len, "%s", gMpConfig.gimmicks == 0x00 ? "AUTO" : gMpConfig.gimmicks == 0x55 ? "ALL ON"
                 : gMpConfig.gimmicks == 0xAA ? "ALL OFF" : "MIXED");
    else if (row >= RULE_SCALE_FIRST && row < RULE_GEN)
    {
        u8 rule = row - RULE_SCALE_FIRST;
        v = MP_LevelScaleOwnRule(rule);
        switch (rule)
        {
        case MP_SCALE_ON:
        case MP_SCALE_CAPS:     snprintf(out, len, "%s", v ? "ON" : "OFF"); break;
        case MP_SCALE_WILD:     snprintf(out, len, "%s", sWild[v & 3]); break;
        case MP_SCALE_TRAINERS: snprintf(out, len, "%s", sTrainers[v & 3]); break;
        case MP_SCALE_BOSS:     if (v == 0) snprintf(out, len, "AUTO"); else snprintf(out, len, "+%d", v - 1); break;
        default:                snprintf(out, len, "%s", sPast8[v % 3]); break;
        }
    }
    else
        out[0] = 0;
}

static void DrawRules(void)
{
    static const char *const labels[RULE_ROWS] = {"TEAM PADDING", "ANIM SPEED", "LEVEL SCALING", "  WILD", "  TRAINERS",
                                                  "  BOSS MARGIN", "  BADGE CAPS", "  PAST 8 BADGES", "GENERATION", "GIMMICKS", "BACK"};
    static const char *const help[RULE_ROWS] = {
        "ON: trainers get one extra Moemon when friends team up.",
        "How much faster battle animations play, for everyone.",
        "Wild Moemon and trainers keep up with the team.",
        "LOW: a quarter way to the team. MEDIUM: halfway. HIGH: near the team.",
        "Trainers' best Moemon: BELOW, EVEN with or ABOVE the team.",
        "Extra levels for GYM LEADERS, rivals, ELITE FOUR. AUTO: +2 to +5.",
        "Nothing scales past a level that rises with the badges.",
        "8 badges: capped to the LEAGUE then free, FREE, or ALWAYS capped.",
        "The Moemon, moves and types of these generations, for everyone.",
        "MEGA, Z-MOVES, DYNAMAX, TERA. AUTO: by GENERATION. Each: OPTIONS > GAME.",
        "Back to HOST A GAME.",
    };
    u8 i;
    FillWindowPixelBuffer(WIN_FULL, PIXEL_FILL(1));
    for (i = 0; i < RULE_ROWS; i++)
    {
        u8 y = 1 + RULE_LINE_H * i;
        char value[24];
        bool8 off = i > RULE_SCALE_FIRST && i < RULE_GEN && !MP_LevelScaleOwnRule(MP_SCALE_ON);
        Print(WIN_FULL, labels[i], 14, y, off ? sHostDisabled : sHostNormal);
        RuleValue(i, value, sizeof(value));
        if (value[0])
        {
            u8 lt[] = {CHAR_LEFT_ARROW, EOS}, rt[] = {CHAR_RIGHT_ARROW, EOS};
            PrintGame(WIN_FULL, lt, 120, y, sHostNormal);
            Print(WIN_FULL, value, 130, y, off ? sHostDisabled : sHostAccent);
            PrintGame(WIN_FULL, rt, 216, y, sHostNormal);
        }
        if (i == sSetup.rulesCursor)
            AddTextPrinterParameterized3(WIN_FULL, FONT_NORMAL, 2, y, sHostNormal, TEXT_SKIP_DRAW, sHostArrow);
    }
    PrintWrapped(WIN_FULL, help[sSetup.rulesCursor], 1 + RULE_LINE_H * RULE_ROWS + 2, 216, sColorDim);
    CopyWindowToVram(WIN_FULL, COPYWIN_GFX);
}

static void HandleRules(void)
{
    int dir = JOY_NEW(DPAD_LEFT) ? -1 : 1;
    u8 row = sSetup.rulesCursor;
    if (JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        sSetup.rulesCursor = JOY_NEW(DPAD_UP) ? (row + RULE_ROWS - 1) % RULE_ROWS : (row + 1) % RULE_ROWS;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && row == RULE_BACK))
    {
        PlaySE(SE_SELECT);
        sSetup.screen = SCR_HOST;
        ShowPatchReader(FALSE);
        DrawScreen();
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | A_BUTTON) && row != RULE_BACK)
    {
        if (row == RULE_PADDING)
            gMpConfig.trainerPadding = !gMpConfig.trainerPadding;
        else if (row == RULE_ANIM)
        {
            static const u8 sSpeeds[] = {0, 25, 30};
            u8 i = gMpConfig.animSpeed == 30 ? 2 : gMpConfig.animSpeed == 25 ? 1 : 0;
            gMpConfig.animSpeed = sSpeeds[(i + (dir < 0 ? 2 : 1)) % 3];
        }
        else if (row == RULE_GEN)
        {
            u8 g = gMpConfig.generation >= 3 && gMpConfig.generation <= 9 ? gMpConfig.generation : 3;
            gMpConfig.generation = 3 + (g - 3 + 7 + dir) % 7;
        }
        else if (row == RULE_GIMMICKS)
        {
            static const u8 sPresets[] = {0x00, 0x55, 0xAA};   // AUTO, ALL ON, ALL OFF
            u8 i = gMpConfig.gimmicks == 0x55 ? 1 : gMpConfig.gimmicks == 0xAA ? 2 : gMpConfig.gimmicks == 0 ? 0 : 2;
            gMpConfig.gimmicks = sPresets[(i + 3 + dir) % 3];
        }
        else
        {
            u8 rule = row - RULE_SCALE_FIRST, n = MP_LevelScaleRuleCount(rule);
            MP_LevelScaleSetOwn(rule, (MP_LevelScaleOwnRule(rule) + n + dir) % n);
        }
        MP_SaveConfig();   // remembered for the next server
        PlaySE(SE_SELECT);
        DrawScreen();
        MPNet_Log("setup: rule %d now %d", row, row == RULE_PADDING ? gMpConfig.trainerPadding : row == RULE_ANIM ? gMpConfig.animSpeed
                  : row == RULE_GEN ? gMpConfig.generation : row == RULE_GIMMICKS ? gMpConfig.gimmicks
                  : MP_LevelScaleOwnRule(row - RULE_SCALE_FIRST));
    }
}

static void LogPatchCursor(void)
{
    char name[16];
    PatchLineName(MP_PatchNotes_Get(sSetup.patchLine[sSetup.patchCursor].first)->version, name, sizeof(name));
    MPNet_Log("patch notes: cursor on %s", name);   // (tests find a line by name)
}

static void HandlePatches(void)
{
    if (JOY_REPEAT(DPAD_UP) && sSetup.patchCursor > 0)
    {
        sSetup.patchCursor--;
        if (sSetup.patchCursor < sSetup.patchScroll)
            sSetup.patchScroll = sSetup.patchCursor;
        PlaySE(SE_SELECT);
        DrawScreen();
        LogPatchCursor();
    }
    else if (JOY_REPEAT(DPAD_DOWN) && sSetup.patchCursor + 1 < sSetup.patchLineCount)
    {
        sSetup.patchCursor++;
        if (sSetup.patchCursor >= sSetup.patchScroll + LIST_ROWS)
            sSetup.patchScroll = sSetup.patchCursor - LIST_ROWS + 1;
        PlaySE(SE_SELECT);
        DrawScreen();
        LogPatchCursor();
    }
    else if (JOY_NEW(A_BUTTON) && sSetup.patchLineCount > 0)
    {
        PlaySE(SE_SELECT);
        BuildPatchText(sSetup.patchCursor);
        sSetup.screen = SCR_PATCHTEXT;
        ShowPatchReader(TRUE);
        DrawScreen();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sSetup.screen = SCR_MAIN;
        DrawScreen();
    }
}

static void HandlePatchText(void)
{
    u16 maxTop = sSetup.rowCount > PATCH_ROWS ? sSetup.rowCount - PATCH_ROWS : 0, top = sSetup.rowTop;

    if (JOY_REPEAT(DPAD_UP) && top > 0)
        top--;
    else if (JOY_REPEAT(DPAD_DOWN) && top < maxTop)
        top++;
    else if (JOY_REPEAT(DPAD_LEFT | L_BUTTON))
        top = top > PATCH_ROWS - 1 ? top - (PATCH_ROWS - 1) : 0;
    else if (JOY_REPEAT(DPAD_RIGHT | R_BUTTON | A_BUTTON))
        top = top + (PATCH_ROWS - 1) < maxTop ? top + (PATCH_ROWS - 1) : maxTop;
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        FreePatchRows();
        sSetup.screen = SCR_PATCHES;
        ShowPatchReader(FALSE);
        DrawScreen();
        return;
    }
    if (top != sSetup.rowTop)
    {
        sSetup.rowTop = top;
        DrawPatchText();
    }
}

static void DrawScreen(void)
{
    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(0));
    switch (sSetup.screen)
    {
    case SCR_HOST:    Print(WIN_TITLE, "HOST A GAME", 12, 2, sColorTitle); break;
    case SCR_JOIN:    Print(WIN_TITLE, "JOIN A GAME", 12, 2, sColorTitle); break;
    case SCR_PATCHES: Print(WIN_TITLE, "PATCH NOTES", 12, 2, sColorTitle); break;
    case SCR_RULES:   Print(WIN_TITLE, "GAME RULES", 12, 2, sColorTitle); break;
    case SCR_PATCHTEXT:
    {
        char title[32], name[16];
        PatchLineName(MP_PatchNotes_Get(sSetup.patchLine[sSetup.patchCursor].first)->version, name, sizeof(name));
        snprintf(title, sizeof(title), "PATCH NOTES  %s", name);
        Print(WIN_TITLE, title, 12, 2, sColorTitle);
        PrintRight(WIN_TITLE, "{UPDOWN} {B} BACK", 220, 2, sColorTitle);
        break;
    }
    default:          Print(WIN_TITLE, "PLAY WITH FRIENDS", 12, 2, sColorTitle); break;
    }
    CopyWindowToVram(WIN_TITLE, COPYWIN_GFX);
    switch (sSetup.screen)
    {
    case SCR_MAIN:    DrawMain(); break;
    case SCR_HOST:    DrawHost(); break;
    case SCR_JOIN:    DrawJoin(); break;
    case SCR_WORKING: DrawWorking(); break;
    case SCR_MESSAGE: DrawMessage(); break;
    case SCR_PATCHES: DrawPatches(); break;
    case SCR_PATCHTEXT: DrawPatchText(); break;
    case SCR_RULES:   DrawRules(); break;
    }
}

static void OpenTextEntry(u8 which, u8 template, u8 *buffer)
{
    sSetup.returnTo = which;
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[FindTaskIdByFunc(Task_Setup)].data[0] = 1; // fading out to the text entry
    gTasks[FindTaskIdByFunc(Task_Setup)].data[1] = template;
    gTasks[FindTaskIdByFunc(Task_Setup)].data[2] = (buffer == sSetup.roomName) ? 0 : (buffer == sSetup.inviteCode) ? 2
                                                 : (buffer == sSetup.note[0]) ? 3 : (buffer == sSetup.note[1]) ? 4 : 1;
}

static void Finish(void)
{
    FreePatchRows();
    MPLan_StopSearch();
    if (!MP_Lobby_InRoom() && MP_Lobby_State() != MP_LOBBY_RECONNECT_WAIT)
        MP_Lobby_Disconnect(); // the lobby connection used for the server list
    MP_SaveConfig();
    sSetup.active = FALSE;
    if (!sSetup.fromField)
        FadeOutBGM(4);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[FindTaskIdByFunc(Task_Setup)].data[0] = 2; // fading out to the main menu
}

static void HandleMain(void)
{
    if (JOY_NEW(DPAD_UP) && sSetup.cursor > 0)
    {
        sSetup.cursor--;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_NEW(DPAD_DOWN) && sSetup.cursor + 1 < sSetup.mainCount)
    {
        sSetup.cursor++;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_NEW(A_BUTTON | START_BUTTON))
    {
        PlaySE(SE_SELECT);
        switch (sSetup.mainItems[sSetup.cursor])
        {
        case MAIN_CONTINUE:
            Finish();
            break;
        case MAIN_LEAVE:
            MP_Lobby_Disconnect();
            MPServer_Stop();
            sSetup.cursor = 0;
            BuildMainItems();
            DrawScreen();
            break;
        case MAIN_REJOIN:
        {
            u8 key[MP_ROOM_KEY_SIZE];
            LastRoomKey(key);
            // Moemon Co-op: where the room was: a friend's own server (joined with an invite code, or at home) or
            // the online lobby. No code or password to type again.
            sSetup.rejoining = gMpConfig.lastRoomAddr[0] != 0;
            if (sSetup.rejoining)
                MP_Lobby_Connect(gMpConfig.lastRoomAddr, FALSE);
            else if (!MP_HasOnlineLobby())
            {
                EnterJoinList();
                break;
            }
            else
                MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
            MPNet_Log("rejoin: %s at %s", gMpConfig.lastRoom, sSetup.rejoining ? gMpConfig.lastRoomAddr : "the online lobby");
            MP_Lobby_JoinRoomWithKey(gMpConfig.lastRoom, key);
            MP_AsciiToGame(sSetup.roomName, gMpConfig.lastRoom, WALDA_PHRASE_LENGTH);
            StartWorking(WORK_JOIN);
            break;
        }
        case MAIN_HOST:
            sSetup.screen = SCR_HOST;
            sSetup.hostCursor = HOST_START;
            LoadNote();
            DrawScreen();
            break;
        case MAIN_JOIN:
            memset(sSetup.password, EOS, sizeof(sSetup.password));
            EnterJoinList();
            break;
        case MAIN_PATCHES:
            BuildPatchLines();
            sSetup.patchCursor = sSetup.patchScroll = 0;
            sSetup.screen = SCR_PATCHES;
            DrawScreen();
            break;
        default:
            Finish();
            break;
        }
    }
}

static void HandleHost(void)
{
    // START HOSTING and BACK share the last line: UP / DOWN treat it as one row, LEFT / RIGHT pick
    if (JOY_NEW(DPAD_UP))
    {
        if (sSetup.hostCursor >= HOST_START)
            sSetup.hostCursor = HOST_START - 1;
        else
            sSetup.hostCursor = (sSetup.hostCursor == 0) ? HOST_START : sSetup.hostCursor - 1;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        sSetup.hostCursor = (sSetup.hostCursor >= HOST_START) ? 0 : sSetup.hostCursor + 1;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (sSetup.hostCursor >= HOST_START && JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        sSetup.hostCursor = (sSetup.hostCursor == HOST_START) ? HOST_BACK : HOST_START;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (sSetup.hostCursor == HOST_LISTED && JOY_NEW(DPAD_LEFT | DPAD_RIGHT | A_BUTTON))
    {
        sSetup.listed = !sSetup.listed;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (sSetup.hostCursor == HOST_RULES && JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sSetup.rulesCursor = 0;
        sSetup.screen = SCR_RULES;
        ShowPatchReader(TRUE);   // (the full-screen window)
        DrawScreen();
        MPNet_Log("setup: game rules");
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sSetup.screen = SCR_MAIN;
        DrawScreen();
    }
    else if (JOY_NEW(A_BUTTON | START_BUTTON))
    {
        switch (sSetup.hostCursor)
        {
        case HOST_NAME:
            OpenTextEntry(1, NAMING_SCREEN_COOP_ROOM, sSetup.roomName);
            break;
        case HOST_PASSWORD:
            OpenTextEntry(2, NAMING_SCREEN_COOP_PASSWORD, sSetup.password);
            break;
        case HOST_NOTE:
            OpenTextEntry(7, NAMING_SCREEN_COOP_NOTE1, sSetup.note[0]);
            break;
        case HOST_START:
            PlaySE(SE_SELECT);
            BeginHosting(MP_HasOnlineLobby());
            break;
        case HOST_BACK:
            PlaySE(SE_SELECT);
            sSetup.screen = SCR_MAIN;
            DrawScreen();
            break;
        }
    }
}

static void HandleJoin(void)
{
    // Keep the list fresh
    if (++sSetup.refreshTimer >= REFRESH_FRAMES)
    {
        sSetup.refreshTimer = 0;
        MPLan_StartSearch();
        if (MP_HasOnlineLobby())
        {
            if (MP_Lobby_State() == MP_LOBBY_ERROR || MP_Lobby_State() == MP_LOBBY_OFF)
            {
                sSetup.onlineFailed = TRUE;
                MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
            }
            MP_Lobby_RequestList();
        }
    }
    if (MP_Lobby_State() == MP_LOBBY_ERROR)
        sSetup.onlineFailed = TRUE;
    if ((sSetup.refreshTimer % 20) == 10)
    {
        u8 before = sSetup.entryCount;
        RefreshJoinList();
        if (MP_Lobby_ListReady() && MP_Lobby_State() == MP_LOBBY_READY)
            sSetup.onlineFailed = FALSE;
        if (before != sSetup.entryCount || (sSetup.refreshTimer % 60) == 10)
            DrawScreen();
    }

    if (JOY_REPEAT(DPAD_UP) && sSetup.joinCursor > 0)
    {
        sSetup.joinCursor--;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_REPEAT(DPAD_DOWN) && sSetup.joinCursor + 1 < sSetup.entryCount)
    {
        sSetup.joinCursor++;
        PlaySE(SE_SELECT);
        DrawScreen();
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        MPLan_StopSearch();
        MP_Lobby_Disconnect();
        sSetup.screen = SCR_MAIN;
        DrawScreen();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        struct JoinEntry *e = &sSetup.entries[sSetup.joinCursor];
        if (e->byCode)
        {
            // (the last code that worked, ready to use again)
            memset(sSetup.inviteCode, EOS, sizeof(sSetup.inviteCode));
            MP_AsciiToGame(sSetup.inviteCode, gMpConfig.lastJoinCode, WALDA_PHRASE_LENGTH);
            OpenTextEntry(6, NAMING_SCREEN_COOP_CODE, sSetup.inviteCode);
            return;
        }
        if (e->byName)
        {
            memset(sSetup.roomName, EOS, sizeof(sSetup.roomName));
            OpenTextEntry(4, NAMING_SCREEN_COOP_ROOM, sSetup.roomName);
            return;
        }
        if (e->room.version != MP_PROTOCOL_VERSION)
        {
            MP_Lobby_SetOtherVersion(e->room.version);
            ShowMessage(LobbyErrorText(MP_LOBBY_ERR_VERSION), TRUE, AFTER_JOIN);
            return;
        }
        if (e->room.players >= e->room.maxPlayers)
        {
            ShowMessage(LobbyErrorText(MP_LOBBY_ERR_FULL), TRUE, AFTER_JOIN);
            return;
        }
        PlaySE(SE_SELECT);
        sSetup.pendingJoin = sSetup.joinCursor;
        memset(sSetup.password, EOS, sizeof(sSetup.password));
        if (e->room.flags & MP_ROOM_FLAG_LOCKED)
            OpenTextEntry(3, NAMING_SCREEN_COOP_PASSWORD, sSetup.password);
        else
            JoinEntryNow(sSetup.joinCursor);
    }
}

// What a home host is told: the invite code, or why there isn't one
static void HostingMessage(char *text, int len)
{
    extern void Platform_SetClipboardText(const char *text);
    const char *code = MP_InviteCode();
    char vpn[48], vpnName[32];
    if (MPPortMap_State() == MP_PM_OPEN)
    {
        Platform_SetClipboardText(code);
        if (MPNet_FindVpnAddress(vpn, sizeof(vpn), vpnName, sizeof(vpnName)))
            snprintf(text, len, "%s is open to the world! Invite code: %s (copied). Friends type it in JOIN A GAME > ENTER CODE OR IP. On %s they can type %s.",
                     MP_Lobby_RoomName(), code, vpnName, vpn);
        else
            snprintf(text, len, "%s is open to the world! Invite code: %s (copied). Send it to friends: they choose JOIN A GAME, then ENTER CODE OR IP.",
                     MP_Lobby_RoomName(), code);
    }
    else if (code[0])
    {
        Platform_SetClipboardText(code);
        if (MPNet_FindVpnAddress(vpn, sizeof(vpn), vpnName, sizeof(vpnName)))
            snprintf(text, len, "Your router couldn't be opened, so this uses %s. Friends on your %s network type %s (or code %s, copied).",
                     MP_VpnName(), MP_VpnName(), vpn, code);
        else
            snprintf(text, len, "Your router couldn't be opened, so this uses %s. Invite code: %s (copied). Friends must join your %s network first.",
                     MP_VpnName(), code, MP_VpnName());
    }
    else
    {
        const char *why = MPPortMap_Error() == MP_PM_ERR_CGNAT ? "Your internet provider shares one address between many homes"
                        : MPPortMap_Error() == MP_PM_ERR_REFUSED ? "Your router refused to open the game's port"
                        : "Your router has automatic port opening (UPnP) turned off";
        snprintf(text, len, "%s is open on your home network. %s, so friends elsewhere can't reach you. See README: UPnP / Radmin VPN.",
                 MP_Lobby_RoomName(), why);
    }
}

static void OnRoomJoined(void)
{
    char text[240];
    int i;
    const u8 *key = MP_Lobby_RoomKey();

    snprintf(gMpConfig.lastRoom, sizeof(gMpConfig.lastRoom), "%s", MP_Lobby_RoomName());
    for (i = 0; i < MP_ROOM_KEY_SIZE; i++)
        snprintf(gMpConfig.lastRoomKey + i * 2, 3, "%02x", key[i]);
    // Moemon Co-op: and where it is, so REJOIN finds it again (a friend's own server too: no code to type)
    if (sSetup.work == WORK_HOST_LOCAL)
        snprintf(gMpConfig.lastRoomAddr, sizeof(gMpConfig.lastRoomAddr), "home");
    else if (!MP_Lobby_ServerAddress(gMpConfig.lastRoomAddr, sizeof(gMpConfig.lastRoomAddr)))
        gMpConfig.lastRoomAddr[0] = 0;
    if (sSetup.joiningCode[0])
        snprintf(gMpConfig.lastJoinCode, sizeof(gMpConfig.lastJoinCode), "%s", sSetup.joiningCode);
    sSetup.joiningCode[0] = 0;
    sSetup.rejoining = FALSE;
    sSetup.triedKey = FALSE;
    MPNet_Log("joined %s (at %s)", gMpConfig.lastRoom, gMpConfig.lastRoomAddr[0] ? gMpConfig.lastRoomAddr : "the online lobby");
    MP_SaveConfig();
    MPLan_StopSearch();
    if (sSetup.work == WORK_HOST_ONLINE)
        snprintf(text, sizeof(text), "%s is open! Friends can join it from JOIN A GAME%s.",
                 MP_Lobby_RoomName(), sSetup.listed ? "" : " > JOIN BY NAME");
    else if (sSetup.work == WORK_HOST_LOCAL)
        HostingMessage(text, sizeof(text));
    else if (sSetup.fromField)
        snprintf(text, sizeof(text), "You joined %s! Your friends will appear in the world.", MP_Lobby_RoomName());
    else
        snprintf(text, sizeof(text), "You joined %s! Load your save or start a new game, and you'll see your friends in the world.",
                 MP_Lobby_RoomName());
    ShowMessage(text, FALSE, AFTER_PLAY);
}

static void HandleWorking(void)
{
    u8 state = MP_Lobby_State();
    sSetup.workTimer++;
    if ((sSetup.workTimer % 20) == 0)
        DrawScreen();

    if (state == MP_LOBBY_IN_ROOM)
    {
        // Hosting at home: wait for the router (at most ~15 s)
        if (sSetup.work == WORK_HOST_LOCAL && MPPortMap_State() == MP_PM_WORKING && sSetup.workTimer < 60 * 15)
            return;
        OnRoomJoined();
        return;
    }
    // Ask the friend's server for its game once connected (and again now and then)
    if (sSetup.work == WORK_CODE && state == MP_LOBBY_READY && !MP_Lobby_ListReady() && (sSetup.workTimer % 45) == 1)
        MP_Lobby_RequestList();
    if (sSetup.work == WORK_CODE && state == MP_LOBBY_READY && MP_Lobby_ListReady())
    {
        struct MpLanRoom rooms[MP_LIST_MAX];
        int n = MP_Lobby_GetList(rooms, MP_LIST_MAX);
        struct JoinEntry *e = &sSetup.entries[0];
        MPNet_Log("invite code: %d game(s) at %s:%d", n, sSetup.codeHost, sSetup.codePort);
        if (n <= 0)
        {
            MP_Lobby_Disconnect();
            ShowMessage("Your friend's PC answered, but nobody is hosting there right now.", TRUE, AFTER_JOIN);
            return;
        }
        // The friend's own server has their one game on it
        memset(e, 0, sizeof(*e));
        e->room = rooms[0];
        e->isLan = TRUE;
        snprintf(e->room.host, sizeof(e->room.host), "%s", sSetup.codeHost);
        e->room.port = sSetup.codePort;
        sSetup.entryCount = 1;
        if (e->room.version != MP_PROTOCOL_VERSION)
        {
            MP_Lobby_Disconnect();
            MP_Lobby_SetOtherVersion(e->room.version);
            ShowMessage(LobbyErrorText(MP_LOBBY_ERR_VERSION), TRUE, AFTER_JOIN);
            return;
        }
        sSetup.pendingJoin = 0;
        memset(sSetup.password, EOS, sizeof(sSetup.password));
        if ((e->room.flags & MP_ROOM_FLAG_LOCKED) && gMpConfig.lastRoomKey[0] && strcmp(e->room.name, gMpConfig.lastRoom) == 0)
        {
            // Moemon Co-op: the room we were in last time: its key, no password to type again
            u8 key[MP_ROOM_KEY_SIZE];
            LastRoomKey(key);
            MPNet_Log("invite code: %s again, with its key", e->room.name);
            sSetup.triedKey = TRUE;
            MP_Lobby_JoinRoomWithKey(e->room.name, key);
            MP_AsciiToGame(sSetup.roomName, e->room.name, WALDA_PHRASE_LENGTH);
            StartWorking(WORK_JOIN);
        }
        else if (e->room.flags & MP_ROOM_FLAG_LOCKED)
            OpenTextEntry(3, NAMING_SCREEN_COOP_PASSWORD, sSetup.password);
        else
            JoinEntryNow(0);
        return;
    }
    if (sSetup.work == WORK_CODE && sSetup.workTimer > 60 * 12)
    {
        MP_Lobby_Disconnect();
        ShowMessage("Couldn't reach your friend's game. Check the code, and ask them to make sure they're still hosting.", TRUE, AFTER_JOIN);
        return;
    }
    if (state == MP_LOBBY_ERROR)
    {
        u8 err = MP_Lobby_Error();
        // Online lobby down: host on the local network instead
        if (sSetup.work == WORK_HOST_ONLINE && err == MP_LOBBY_ERR_UNREACHABLE)
        {
            MP_Lobby_Disconnect();
            BeginHosting(FALSE);
            return;
        }
        if (sSetup.work == WORK_HOST_LOCAL)
            MPServer_Stop();
        MP_Lobby_Disconnect();
        if (sSetup.work == WORK_CODE)
            ShowMessage("Couldn't reach your friend's game. Check the code, and ask them to make sure they're still hosting.", TRUE, AFTER_JOIN);
        else if (sSetup.triedKey && err == MP_LOBBY_ERR_BAD_PASSWORD)
        {
            // the host changed the password since: ask it (the room's entry is still there to join)
            sSetup.triedKey = FALSE;
            MPNet_Log("invite code: the room's password changed, asking for it");
            OpenTextEntry(3, NAMING_SCREEN_COOP_PASSWORD, sSetup.password);
        }
        else if (sSetup.rejoining && (err == MP_LOBBY_ERR_UNREACHABLE || err == MP_LOBBY_ERR_NOT_FOUND))
        {
            sSetup.rejoining = FALSE;
            ShowMessage("Couldn't reach your friend's game. Ask them to make sure they're hosting. If their invite code changed, use JOIN A GAME > ENTER CODE OR IP.", TRUE, AFTER_JOIN);
        }
        else
            ShowMessage(LobbyErrorText(err), TRUE, sSetup.work == WORK_JOIN ? AFTER_JOIN : AFTER_HOST);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        MP_Lobby_Disconnect();
        if (sSetup.work == WORK_HOST_LOCAL)
            MPServer_Stop();
        if (sSetup.work == WORK_JOIN || sSetup.work == WORK_CODE)
            EnterJoinList();
        else
        {
            sSetup.screen = SCR_HOST;
            DrawScreen();
        }
    }
}

static void HandleMessage(void)
{
    if (!JOY_NEW(A_BUTTON | B_BUTTON | START_BUTTON))
        return;
    PlaySE(SE_SELECT);
    switch (sSetup.after)
    {
    case AFTER_PLAY:
        Finish();
        break;
    case AFTER_HOST:
        sSetup.screen = SCR_HOST;
        DrawScreen();
        break;
    case AFTER_JOIN:
        EnterJoinList();
        break;
    default:
        sSetup.screen = SCR_MAIN;
        BuildMainItems();
        sSetup.cursor = 0;
        DrawScreen();
        break;
    }
}

static void Task_Setup(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (data[0])
    {
    case 0: // running
        if (gPaletteFade.active)
            return;
        switch (sSetup.screen)
        {
        case SCR_MAIN:    HandleMain(); break;
        case SCR_HOST:    HandleHost(); break;
        case SCR_JOIN:    HandleJoin(); break;
        case SCR_WORKING: HandleWorking(); break;
        case SCR_MESSAGE: HandleMessage(); break;
        case SCR_PATCHES: HandlePatches(); break;
        case SCR_PATCHTEXT: HandlePatchText(); break;
        case SCR_RULES:   HandleRules(); break;
        }
        break;
    case 1: // fading out to the keyboard
        if (!gPaletteFade.active)
        {
            u8 template = data[1];
            u8 *buffer = data[2] == 4 ? sSetup.note[1] : data[2] == 3 ? sSetup.note[0] : data[2] == 2 ? sSetup.inviteCode
                       : data[2] ? sSetup.password : sSetup.roomName;
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            DoNamingScreen(template, buffer, 0, 0, 0, CB2_ReturnFromTextEntry);
        }
        break;
    case 2: // fading out to the main menu (or back to the game)
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            SetMainCallback2(sSetup.fromField ? gMain.savedCallback : CB2_InitMainMenu);
        }
        break;
    }
}

static void CB2_ReturnFromTextEntry(void)
{
    u8 which = sSetup.returnTo;
    sSetup.returnTo = 0;
    InitGraphics();
    switch (which)
    {
    case 1:
    case 2:
    case 8:
        sSetup.screen = SCR_HOST;
        SaveNote();
        DrawScreen();
        break;
    case 7: // welcome note line 1 done: now line 2
        sSetup.returnTo = 8;
        DestroyTask(FindTaskIdByFunc(Task_Setup));
        FreeAllWindowBuffers();
        DoNamingScreen(NAMING_SCREEN_COOP_NOTE2, sSetup.note[1], 0, 0, 0, CB2_ReturnFromTextEntry);
        break;
    case 3: // password for the chosen server
        sSetup.screen = SCR_JOIN;
        JoinEntryNow(sSetup.pendingJoin);
        break;
    case 4: // joined by name: now the password
    {
        char room[MP_ROOM_NAME_MAX + 1];
        GameToAsciiTrim(room, sSetup.roomName, WALDA_PHRASE_LENGTH);
        if (room[0] == 0)
        {
            EnterJoinList();
            break;
        }
        memset(sSetup.password, EOS, sizeof(sSetup.password));
        sSetup.returnTo = 5;
        DestroyTask(FindTaskIdByFunc(Task_Setup));
        FreeAllWindowBuffers();
        DoNamingScreen(NAMING_SCREEN_COOP_PASSWORD, sSetup.password, 0, 0, 0, CB2_ReturnFromTextEntry);
        break;
    }
    case 6: // invite code typed
    {
        char code[24], addr[48];
        uint32_t ip;
        int port;
        GameToAsciiTrim(code, sSetup.inviteCode, WALDA_PHRASE_LENGTH);
        MPNet_Log("invite code entered: \"%s\"", code);
        sSetup.screen = SCR_JOIN;
        if (code[0] == 0)
        {
            EnterJoinList();
            break;
        }
        // A plain address works too (Radmin VPN / Hamachi: 26.x.x.x, 25.x.x.x)
        {
            unsigned a, b, c, d, pt = MP_DEFAULT_PORT;
            int n = sscanf(code, "%u.%u.%u.%u:%u", &a, &b, &c, &d, &pt);
            if (n >= 4 && a < 256 && b < 256 && c < 256 && d < 256 && pt > 0 && pt < 65536)
            {
                ip = (a << 24) | (b << 16) | (c << 8) | d;
                port = (int)pt;
                goto haveAddress;
            }
        }
        if (!MPNet_ParseInviteCode(code, &ip, &port))
        {
            ShowMessage("That invite code doesn't look right. Check it with your friend and try again. (It looks like K7QM-3XPA, or an address like 26.12.34.56.)", TRUE, AFTER_JOIN);
            break;
        }
    haveAddress:
        snprintf(sSetup.joiningCode, sizeof(sSetup.joiningCode), "%s", code);
        snprintf(addr, sizeof(addr), "%u.%u.%u.%u:%d", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255, port);
        snprintf(sSetup.codeHost, sizeof(sSetup.codeHost), "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
        sSetup.codePort = port;
        MPLan_StopSearch();
        MP_Lobby_Connect(addr, FALSE);
        MP_Lobby_RequestList();
        StartWorking(WORK_CODE);
        break;
    }
    case 5:
    {
        char room[MP_ROOM_NAME_MAX + 1], password[WALDA_PHRASE_LENGTH + 1];
        GameToAsciiTrim(room, sSetup.roomName, WALDA_PHRASE_LENGTH);
        MP_GameToAscii(password, sSetup.password, WALDA_PHRASE_LENGTH);
        if (MP_Lobby_State() != MP_LOBBY_READY)
            MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
        MP_Lobby_JoinRoom(room, password);
        StartWorking(WORK_JOIN);
        break;
    }
    default:
        sSetup.screen = SCR_MAIN;
        DrawScreen();
        break;
    }
}

static void InitSetup(bool8 fromField)
{
    memset(&sSetup, 0, sizeof(sSetup));
    sSetup.fromField = fromField;
    memset(sSetup.roomName, EOS, sizeof(sSetup.roomName));
    memset(sSetup.password, EOS, sizeof(sSetup.password));
    sSetup.active = TRUE;
    sSetup.listed = TRUE;
    DefaultRoomName();
    BuildMainItems();
    sSetup.screen = SCR_MAIN;
    InitGraphics();
    DrawScreen();
    if (!fromField)
        PlayBGM(MUS_RG_UNION_ROOM);
}

void CB2_InitCoopSetup(void)
{
    // Automated tests skip this screen (MOEMON_SKIP_COOP_SETUP=0: a test of the screen itself)
    if (getenv("MOEMON_COOP_AUTO") || (getenv("MOEMON_SKIP_COOP_SETUP") && strcmp(getenv("MOEMON_SKIP_COOP_SETUP"), "0") != 0))
    {
        SetMainCallback2(CB2_InitMainMenu);
        return;
    }
    InitSetup(FALSE);
}

void CB2_InitCoopSetupFromField(void)
{
    InitSetup(TRUE);
}

// ---------------------------------------------------------------------------
// MOEMON_COOP_AUTO=host:NAME[:PASSWORD] / join:NAME[:PASSWORD] / hostlan:NAME[:PASSWORD]
// connects without the menu (automated tests). MOEMON_LOBBY picks the lobby.

void MP_AutoSession_Update(void)
{
    static bool8 sDone;
    static u8 sTries;
    static u16 sWait;
    const char *env;
    char buf[96], *mode, *name, *password;

    if (sDone)
    {
        // a joiner that started before the host's room was there (a slow, busy test machine): try again a few times
        u8 err = MP_Lobby_Error();
        if (sTries == 0 || sTries >= 5 || MP_Lobby_State() != MP_LOBBY_ERROR
         || (err != MP_LOBBY_ERR_NOT_FOUND && err != MP_LOBBY_ERR_UNREACHABLE) || ++sWait < 120)
            return;
        sWait = 0;
        MPNet_Log("auto session: join failed (error %d), trying again", err);
        MP_Lobby_Disconnect();
    }
    sDone = TRUE;
    env = getenv("MOEMON_COOP_AUTO");
    if (!env)
        return;
    snprintf(buf, sizeof(buf), "%s", env);
    mode = strtok(buf, ":");
    name = strtok(NULL, ":");
    password = strtok(NULL, ":");
    if (!mode || !name)
        return;
    if (strcmp(mode, "hostlan") == 0)
    {
        int i;
        char addr[32];
        for (i = 0; i < LOCAL_PORT_TRIES; i++)
            if (MPServer_Start(MP_DEFAULT_PORT + i, TRUE) == 0)
            {
                snprintf(addr, sizeof(addr), "127.0.0.1:%d", MP_DEFAULT_PORT + i);
                MP_Lobby_Connect(addr, TRUE);
                break;
            }
        MP_Lobby_CreateRoom(name, password ? password : "", FALSE);
        return;
    }
    MP_Lobby_Connect(MP_OnlineLobbyAddress(), FALSE);
    if (strcmp(mode, "host") == 0)
        MP_Lobby_CreateRoom(name, password ? password : "", FALSE);
    else
    {
        sTries++;
        MP_Lobby_JoinRoom(name, password ? password : "");
    }
}

// Test harness helpers
bool8 MP_Setup_IsReady(void)
{
    return sSetup.active && !gPaletteFade.active && FuncIsActiveTask(Task_Setup);
}

bool8 MP_Setup_ScreenIs(int screen)
{
    return MP_Setup_IsReady() && sSetup.screen == screen;
}
