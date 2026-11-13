// Moemon Co-op: the list windows used by the co-op screens (shared PC, trades,
// gifts, goals...). A main window with a title and up to MPUI_ROWS rows, and a
// one-line help window under it, both drawn over the overworld.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "platform/gamepad.h"
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
#include "sprite.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/characters.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

#define MAIN_W 28   // the wide layout's list window (tiles)
#define MAIN_H 13
#define SLIM_W 11   // the compact layout's list window, at the right, beside the room
#define ROOM_W 12
#define ROOM_H 10
#define ROW_H  12
#define ROW_Y0 18   // under the title and its line
#define HELP_H 3    // tiles: two lines of small text

// Two layouts (the user's design, 2026-11-13: "allow some of the field to still be visible"):
//   WIDE: one list window over most of the screen, for screens with more to say (DAILY, CHAT LOG, TRADE...)
//   COMPACT: a narrow list at the right, the room (who's here, where) at the top left, the field between them;
//            for short lists (SOCIAL, BERRY GAMES...). A screen asks for it with MpUi_Compact() before MpUi_Begin.
// The help line is the same in both, along the bottom.
static const struct WindowTemplate sMainTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = MAIN_W, .height = MAIN_H,
    .paletteNum = 15, .baseBlock = 0x8,
};
static const struct WindowTemplate sSlimTemplate = {
    .bg = 0, .tilemapLeft = 30 - 1 - SLIM_W, .tilemapTop = 1, .width = SLIM_W, .height = MAIN_H,
    .paletteNum = 15, .baseBlock = 0x8,
};
static const struct WindowTemplate sRoomTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = ROOM_W, .height = ROOM_H,
    .paletteNum = 15, .baseBlock = 0x8 + SLIM_W * MAIN_H,
};

static const struct WindowTemplate sHelpTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = MAIN_W, .height = HELP_H,
    .paletteNum = 15, .baseBlock = 0x8 + MAIN_W * MAIN_H,
};

static u8 sMain = WINDOW_NONE, sHelp = WINDOW_NONE, sRoom = WINDOW_NONE;
static u8 sHelpText[200];   // redrawn when the player switches keyboard / pad
static s16 sScrollTop = -1, sScrollCount, sSelY = -1;
static s16 sTitleEnd;   // x where the title text ends
static bool8 sCompact, sWantCompact;
static u8 sW = MAIN_W;  // the list window's width now (tiles)

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
    u16 white;
    MP_Theme_StdPalette(pal);
    white = pal[1];   // the fill: 1 white, 2 dark grey, 3 light grey... (THEME DARK: a night blue, light text)
    pal[MPT_BAND] = Mix(white, a, 64);                         // title band
    pal[MPT_BAND2] = Mix(white, a, 120);                       // its top line
    pal[MPT_SEL] = Mix(white, a, 52);                          // selected row
    if (MP_Theme_Dark())
    {
        pal[MPT_GOLD] = colorful ? Mix(RGB(31, 31, 31), a, 120) : RGB(30, 30, 31);   // highlighted text, light
        pal[MPT_GREY] = RGB(19, 19, 21);                                             // less important text
    }
    else
    {
        pal[MPT_GOLD] = colorful ? Mix(0, a, 120) : RGB(7, 7, 8);  // highlighted text (dark, in the frame's color)
        pal[MPT_GREY] = RGB(14, 14, 15);                           // less important text
    }
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

// ---------------------------------------------------------------------------
// Pictures beside the text: players' heads (their look, MP_Avatar_HeadIcon) and the CO-OP menu's item icons, as
// 16x16 sprites in slots the screens name. MpUi_PicBegin marks every slot unused, MpUi_Head / MpUi_Icon place one
// (made again only when what it shows changes), MpUi_PicEnd takes away the ones not placed this time.
#include "../data/mp_coopmenu_icons.h"

#define TAG_MPUI_PIC 0x5C40
enum { PIC_NONE, PIC_HEAD, PIC_ICON, PIC_EMOTE };
static struct { u8 kind, sprite; s16 key; bool8 used; } sPics[MPUI_PICS];
static u8 sPicTiles[MPUI_PICS][128];
static u16 sPicPal[MPUI_PICS][16];

static const struct OamData sOam_Pic = {
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 0,
};

static void PicFree(u8 slot)
{
    if (sPics[slot].kind != PIC_NONE && sPics[slot].sprite < MAX_SPRITES)
        DestroySprite(&gSprites[sPics[slot].sprite]);
    if (sPics[slot].kind != PIC_NONE)
    {
        FreeSpriteTilesByTag(TAG_MPUI_PIC + slot);
        FreeSpritePaletteByTag(TAG_MPUI_PIC + slot);
    }
    sPics[slot].kind = PIC_NONE;
    sPics[slot].sprite = MAX_SPRITES;
}

static void PicPlace(u8 slot, u8 kind, s16 key, int x, int y)
{
    if (slot >= MPUI_PICS)
        return;
    if (sPics[slot].kind != kind || sPics[slot].key != key)
    {
        struct SpriteSheet sheet = {sPicTiles[slot], 128, TAG_MPUI_PIC + slot};
        struct SpritePalette pal = {sPicPal[slot], TAG_MPUI_PIC + slot};
        struct SpriteTemplate tmpl = {TAG_MPUI_PIC + slot, TAG_MPUI_PIC + slot, &sOam_Pic, gDummySpriteAnimTable, NULL,
                                      gDummySpriteAffineAnimTable, SpriteCallbackDummy};
        PicFree(slot);
        if (kind == PIC_HEAD)
        {
            if (!MP_Avatar_HeadIcon(key, sPicTiles[slot], sPicPal[slot], key < 0 ? RGB_WHITE : MP_Compass_PlayerColor(key)))
                return;
        }
        else if (kind == PIC_EMOTE)
            MP_Emote_IconTiles(key, sPicTiles[slot], sPicPal[slot]);
        else
        {
            memcpy(sPicTiles[slot], sCoopIconTiles[key % MP_COOPICON_COUNT], 128);
            memcpy(sPicPal[slot], sCoopIconPals[key % MP_COOPICON_COUNT], 32);
        }
        LoadSpriteSheet(&sheet);
        LoadSpritePalette(&pal);
        sPics[slot].sprite = CreateSprite(&tmpl, x + 8, y + 8, 0);
        sPics[slot].kind = kind;
        sPics[slot].key = key;
        if (sPics[slot].sprite >= MAX_SPRITES)
        {
            FreeSpriteTilesByTag(TAG_MPUI_PIC + slot);
            FreeSpritePaletteByTag(TAG_MPUI_PIC + slot);
            sPics[slot].kind = PIC_NONE;
            return;
        }
    }
    if (sPics[slot].sprite < MAX_SPRITES)
    {
        gSprites[sPics[slot].sprite].x = x + 8;
        gSprites[sPics[slot].sprite].y = y + 8;
    }
    sPics[slot].used = TRUE;
}

void MpUi_PicBegin(void)
{
    u8 i;
    for (i = 0; i < MPUI_PICS; i++)
        sPics[i].used = FALSE;
}

void MpUi_PicEnd(void)
{
    u8 i;
    for (i = 0; i < MPUI_PICS; i++)
        if (!sPics[i].used && sPics[i].kind != PIC_NONE)
            PicFree(i);
}

void MpUi_PicsClear(void)
{
    u8 i;
    for (i = 0; i < MPUI_PICS; i++)
        PicFree(i);
}

// A player's head (-1: you) / a CO-OP menu icon (MP_COOPICON_*), its top left at (x, y) on the screen
void MpUi_Head(u8 slot, s8 player, int x, int y)
{
    PicPlace(slot, PIC_HEAD, player, x, y);
}

void MpUi_Icon(u8 slot, u8 icon, int x, int y)
{
    PicPlace(slot, PIC_ICON, icon, x, y);
}

void MpUi_Emote(u8 slot, u8 kind, int x, int y)
{
    PicPlace(slot, PIC_EMOTE, kind, x, y);
}

// ---------------------------------------------------------------------------
// The room: who else is here (up to three), where each is or what they're doing. Two lines a person, so a 7-letter
// name and a long place both fit (the user, 2026-11-13). PICK highlights one (-1 none); RIGHT (if given) replaces each
// one's place line (a prank's cost...). Heads in picture slots MPUI_PIC_ROOM.. (12 x 10 tile window)

static void Badge(u8 win, int x, int y, const u8 *text, u8 fill)
{
    // a small rounded mark: a count, or "!"
    FillWindowPixelRect(win, PIXEL_FILL(fill), x + 1, y, 9, 11);
    FillWindowPixelRect(win, PIXEL_FILL(fill), x, y + 1, 11, 9);
    MpTheme_Print(win, FONT_SMALL, text, x + 5 - GetStringWidth(FONT_SMALL, text, 0) / 2, y - 1, MPT_BG, fill);
}

void MpUi_RoomMark(u8 win, int x, int y, char mark, bool8 blue)
{
    u8 t[4];
    char a[2] = {mark, 0};
    MP_AsciiToGame(t, a, 2);
    Badge(win, x, y, t, blue ? MPT_BLUE : MPT_RED);
}

static void CutToWidth(u8 *text, u8 font, int w)
{
    int len;
    for (len = StringLength(text); len > 1 && GetStringWidth(font, text, 0) > w; len--)
        text[len - 1] = EOS;
}

void MpUi_DrawRoom(u8 win, s8 pick, const char *const *right)
{
    u8 ids[MP_MAX_PLAYERS], buf[48];
    char ascii[48];
    int n = MP_IsConnected() ? MP_Friends_List(ids) : 0, i, winX, winY;
    int w = GetWindowAttribute(win, WINDOW_WIDTH) * 8;
    winX = GetWindowAttribute(win, WINDOW_TILEMAP_LEFT) * 8;
    winY = GetWindowAttribute(win, WINDOW_TILEMAP_TOP) * 8;
    FillWindowPixelBuffer(win, PIXEL_FILL(MPT_BG));
    if (!MP_IsConnected())
    {
        MP_AsciiToGame(buf, "PLAYING ALONE", 40);
        MpTheme_Print(win, FONT_SMALL, buf, 0, 0, MPT_TEXT, MPT_BG);
    }
    else
    {
        const char *room = MP_Lobby_RoomName();
        snprintf(ascii, sizeof(ascii), "%s", room && room[0] ? room : "CO-OP ROOM");
        for (i = 0; ascii[i]; i++)
            if (ascii[i] >= 'a' && ascii[i] <= 'z')
                ascii[i] -= 32;
        MP_AsciiToGame(buf, ascii, 40);
        CutToWidth(buf, FONT_SMALL, w - 22);
        MpTheme_Print(win, FONT_SMALL, buf, 0, 0, MPT_TEXT, MPT_BG);
        snprintf(ascii, sizeof(ascii), "%d/%d", MP_ConnectedCount(), MP_MAX_PLAYERS);
        MP_AsciiToGame(buf, ascii, 8);
        MpTheme_Print(win, FONT_SMALL, buf, w - GetStringWidth(FONT_SMALL, buf, 0), 0, MPT_GREY, MPT_BG);
    }
    for (i = 0; i < MP_MAX_PLAYERS - 1; i++)
    {
        int y = 13 + i * 22;
        if (i >= n)
        {
            // an empty seat
            FillWindowPixelRect(win, PIXEL_FILL(MPT_SHADOW), 0, y + 1, 16, 16);
            FillWindowPixelRect(win, PIXEL_FILL(MPT_BG), 1, y + 2, 14, 14);
            MP_AsciiToGame(buf, "- empty seat -", 20);
            MpTheme_Print(win, FONT_SMALL_NARROW, buf, 20, y + 3, MPT_GREY, MPT_BG);
            continue;
        }
        {
            u8 id = ids[i];
            struct MpPlayer *p = &gMpPlayers[id];
            if (pick == i)
            {
                FillWindowPixelRect(win, PIXEL_FILL(MPT_SEL), 0, y - 2, w, 21);
                FillWindowPixelRect(win, PIXEL_FILL(MPT_BORDER), 0, y - 2, 2, 21);
            }
            MpUi_Head(MPUI_PIC_ROOM + i, id, winX + 2, winY + y);
            StringCopyN(buf, p->name, PLAYER_NAME_LENGTH);
            buf[PLAYER_NAME_LENGTH] = EOS;
            MpTheme_Print(win, FONT_SMALL, buf, 20, y - 2, MPT_TEXT, 0);
            if (right && right[i])
                MP_AsciiToGame(buf, right[i], 40);
            else if (p->hasState && (p->state.flags & MPF_IN_BATTLE))
                MP_AsciiToGame(buf, "IN A BATTLE", 40);
            else if (p->hasState && p->state.cardDuel)
                MP_AsciiToGame(buf, "IN A CARD DUEL", 40);
            else
                MP_Friends_LocationName(id, buf);
            CutToWidth(buf, FONT_SMALL_NARROW, w - 20);
            MpTheme_Print(win, FONT_SMALL_NARROW, buf, 20, y + 8, MPT_GREY, 0);
            if (p->hasState && ((p->state.flags & MPF_IN_BATTLE) || p->state.cardDuel))
                MpUi_RoomMark(win, w - 11, y - 1, '!', TRUE);
        }
    }
}

// ---------------------------------------------------------------------------

bool8 MpUi_IsOpen(void)
{
    return sMain != WINDOW_NONE;
}

// The list windows for the layout asked for (and the room in COMPACT)
static void MakeWindows(bool8 compact)
{
    sCompact = compact;
    sW = compact ? SLIM_W : MAIN_W;
    sMain = AddWindow(compact ? &sSlimTemplate : &sMainTemplate);
    PutWindowTilemap(sMain);
    DrawStdWindowFrame(sMain, FALSE);
    MpTheme_Panel(sMain, 0);
    CopyWindowToVram(sMain, COPYWIN_FULL);
    if (compact)
    {
        sRoom = AddWindow(&sRoomTemplate);
        PutWindowTilemap(sRoom);
        DrawStdWindowFrame(sRoom, FALSE);
        MpUi_DrawRoom(sRoom, -1, NULL);
        CopyWindowToVram(sRoom, COPYWIN_FULL);
    }
}

static void RemoveListWindows(void)
{
    u8 i;
    if (sRoom != WINDOW_NONE)
    {
        ClearStdWindowAndFrame(sRoom, TRUE);
        RemoveWindow(sRoom);
        sRoom = WINDOW_NONE;
        for (i = MPUI_PIC_ROOM; i < MPUI_PIC_ROOM + MP_MAX_PLAYERS - 1; i++)
            PicFree(i);
    }
    if (sMain != WINDOW_NONE)
    {
        ClearStdWindowAndFrame(sMain, TRUE);
        RemoveWindow(sMain);
        sMain = WINDOW_NONE;
    }
}

void MpUi_Open(void)
{
    if (sMain != WINDOW_NONE)
        return;
    LockPlayerFieldControls();
    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();   // the map name sign shares the frame's palette slot
    MpTheme_Push();
    MakeWindows(sWantCompact);
    sHelp = AddWindow(&sHelpTemplate);
    PutWindowTilemap(sHelp);
    DrawStdWindowFrame(sHelp, FALSE);
    MpTheme_Panel(sHelp, 0);
    CopyWindowToVram(sHelp, COPYWIN_FULL);
}

void MpUi_Close(void)
{
    if (sMain == WINDOW_NONE)
        return;
    RemoveListWindows();
    ClearStdWindowAndFrame(sHelp, TRUE);
    RemoveWindow(sHelp);
    sHelp = WINDOW_NONE;
    sHelpText[0] = EOS;
    sWantCompact = FALSE;
    MpUi_PicsClear();
    MpTheme_Pop();
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

// The next MpUi_Begin draws the COMPACT layout (a short list beside the room); without it, WIDE
void MpUi_Compact(void)
{
    sWantCompact = TRUE;
}

void MpUi_Begin(const char *title)
{
    u8 buf[96];
    int w;
    bool8 compact = sWantCompact;
    sWantCompact = FALSE;
    if (sMain != WINDOW_NONE && compact != sCompact)
    {
        RemoveListWindows();
        MakeWindows(compact);
    }
    else if (sRoom != WINDOW_NONE)
    {
        MpUi_DrawRoom(sRoom, -1, NULL);
        CopyWindowToVram(sRoom, COPYWIN_FULL);
    }
    sScrollTop = -1;
    sSelY = -1;
    MpUi_PicBegin();
    if (sRoom != WINDOW_NONE)
    {
        u8 i;
        for (i = MPUI_PIC_ROOM; i < MPUI_PIC_ROOM + MP_MAX_PLAYERS - 1; i++)
            if (sPics[i].kind != PIC_NONE)
                sPics[i].used = TRUE;
    }
    w = sW * 8;
    MpTheme_Panel(sMain, 0);
    MP_AsciiToGame(buf, title, 92);
    // The title in the game's own large font, a line under it (Emerald's menus); a long one in the small font
    if (GetStringWidth(FONT_NORMAL, buf, 0) <= w - 4)
    {
        MpTheme_Print(sMain, FONT_NORMAL, buf, 2, 0, MPT_TEXT, MPT_BG);
        sTitleEnd = 2 + GetStringWidth(FONT_NORMAL, buf, 0);
    }
    else
    {
        MpUi_PrintWrapped(sMain, buf, 2, 2, w - 4, 1, MPT_TEXT, MPT_BG);
        sTitleEnd = 2 + min(w - 4, GetStringWidth(FONT_SMALL, buf, 0));
    }
    FillWindowPixelRect(sMain, PIXEL_FILL(MPT_SHADOW), 0, 14, w, 1);
}

// Copies at most size-1 characters of a string and ends it (StringCopyN alone copies a fixed count, reading past
// short strings)
static void CopyText(u8 *dst, const u8 *src, int size)
{
    int len = StringLength(src);
    if (len > size - 1)
        len = size - 1;
    *StringCopyN(dst, src, len) = EOS;
}

// Text on the right end of the title line (e.g. "POKéMON  ITEMS" tabs)
void MpUi_TitleRight(const u8 *text)
{
    u8 cut[64];
    u8 font = FONT_SMALL;
    int len, room = sW * 8 - 2 - (sTitleEnd + 8);
    CopyText(cut, text, sizeof(cut));
    if (GetStringWidth(font, cut, 0) > room)
        font = FONT_SMALL_NARROW;
    for (len = StringLength(cut); len > 1 && GetStringWidth(font, cut, 0) > room; len--)
        cut[len - 1] = EOS;
    if (room <= 0)
        return;
    MpTheme_Print(sMain, font, cut, sW * 8 - 2 - GetStringWidth(font, cut, 0), 2, MPT_GREY, MPT_BG);
}

// One row at pixel line Y: left text, and optional right-aligned text. Long left text is cut. The selected row has
// the lit bar behind it and the game's own ▶.
static void RowAt(int y, bool8 selected, const u8 *left, const u8 *right)
{
    u8 cut[80];
    u8 rcut[64];
    u8 font;
    int maxW, w = sW * 8 - (sScrollCount > MPUI_ROWS && sScrollTop >= 0 ? 8 : 0);   // (room for the scroll arrows)
    if (right)
    {
        // Right text gets at most half the row
        CopyText(rcut, right, sizeof(rcut));
        CutToWidth(rcut, FONT_SMALL_NARROW, w / 2);
        right = rcut;
    }
    maxW = w - 10 - (right ? GetStringWidth(FONT_SMALL_NARROW, right, 0) + 6 : 0);
    if (selected)
    {
        u8 arrow[] = {0xEF /* ▶ */, EOS};
        MpTheme_SelBar(sMain, y - 1, ROW_H);
        MpTheme_Print(sMain, FONT_SMALL, arrow, 3, y - 1, MPT_TEXT, 0);
        sSelY = y;
    }
    CopyText(cut, left, sizeof(cut));
    font = GetStringWidth(FONT_SMALL, cut, 0) <= maxW ? FONT_SMALL : FONT_SMALL_NARROW;
    CutToWidth(cut, font, maxW);
    MpTheme_Print(sMain, font, cut, 10, y - 1, MPT_TEXT, 0);
    if (right)
        MpTheme_Print(sMain, FONT_SMALL_NARROW, right, w - 3 - GetStringWidth(FONT_SMALL_NARROW, right, 0), y - 1,
                      MPT_GREY, 0);
}

void MpUi_Row(u8 row, bool8 selected, const u8 *left, const u8 *right)
{
    RowAt(ROW_Y0 + row * ROW_H, selected, left, right);
}

void MpUi_RowAscii(u8 row, bool8 selected, const char *left, const char *right)
{
    u8 l[80], r[40];
    MP_AsciiToGame(l, left, 76);
    if (right)
        MP_AsciiToGame(r, right, 36);
    MpUi_Row(row, selected, l, right ? r : NULL);
}

// A friend as a tall row (slot 0-2): their head, their name in the large font, where they are under it, and at the
// right a note (RIGHT) with a mark (MARK: 0 none, else the character, red; BLUE: blue). The row after the last one is
// MpUi_RowBelowFriends.
#define FRIEND_H 25
void MpUi_FriendRow(u8 slot, bool8 selected, u8 id, const char *right, char mark, bool8 blue)
{
    u8 buf[48];
    int y = ROW_Y0 - 2 + slot * FRIEND_H, w = sW * 8, rw = 0;
    int winX = GetWindowAttribute(sMain, WINDOW_TILEMAP_LEFT) * 8, winY = GetWindowAttribute(sMain, WINDOW_TILEMAP_TOP) * 8;
    if (selected)
    {
        u8 arrow[] = {0xEF /* ▶ */, EOS};
        MpTheme_SelBar(sMain, y, FRIEND_H - 1);
        MpTheme_Print(sMain, FONT_SMALL, arrow, 3, y + 6, MPT_TEXT, 0);
        sSelY = y;
    }
    MpUi_Head(MPUI_PIC_ROWS + slot, id, winX + 12, winY + y + 4);
    StringCopyN(buf, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
    buf[PLAYER_NAME_LENGTH] = EOS;
    MpTheme_Print(sMain, FONT_NORMAL, buf, 32, y - 1, MPT_TEXT, 0);
    if (right)
    {
        MP_AsciiToGame(buf, right, 40);
        CutToWidth(buf, FONT_SMALL, w / 2 - 16);
        rw = GetStringWidth(FONT_SMALL, buf, 0);
        MpTheme_Print(sMain, FONT_SMALL, buf, w - 4 - rw, y + 5, mark ? MPT_TEXT : MPT_GREY, 0);
        if (mark)
            MpUi_RoomMark(sMain, w - 4 - rw - 15, y + 6, mark, blue);
    }
    MP_Friends_LocationName(id, buf);
    CutToWidth(buf, FONT_SMALL, w - 36 - (rw ? rw + 20 : 0));
    MpTheme_Print(sMain, FONT_SMALL, buf, 32, y + 11, MPT_GREY, 0);
}

// A plain row under up to three friend rows (BACK, CANCEL)
void MpUi_RowBelowFriends(u8 friends, bool8 selected, const char *left)
{
    u8 l[40];
    MP_AsciiToGame(l, left, 36);
    RowAt(ROW_Y0 + friends * FRIEND_H + 1, selected, l, NULL);
}

// A line of text anywhere in the list area (row = 0..MPUI_ROWS-1)
void MpUi_TextAscii(u8 row, const char *text)
{
    u8 buf[160];
    MP_AsciiToGame(buf, text, 156);
    MpUi_PrintWrapped(sMain, buf, 11, ROW_Y0 - 1 + row * ROW_H, sW * 8 - 11 - 3, 1, MPT_TEXT, MPT_BG);
}

void MpUi_End(void)
{
    // Scroll hints when the list goes on above or below
    if (sScrollTop >= 0 && sScrollCount > MPUI_ROWS)
    {
        if (sScrollTop > 0)
            DrawTriangle(sMain, sW * 8 - 6, ROW_Y0 - 2, TRUE);
        if (sScrollTop + MPUI_ROWS < sScrollCount)
            DrawTriangle(sMain, sW * 8 - 6, ROW_Y0 + MPUI_ROWS * ROW_H - 3, FALSE);
    }
    MpUi_PicEnd();
    CopyWindowToVram(sMain, COPYWIN_FULL);
}

// Splits text into lines no wider than maxW (breaking at spaces); returns how many
// lines it needs. Only the first maxLines are written. A button icon stays on the
// same line as the word after it ("{A} SELECT"), and a gap of two or more spaces
// (between two prompts) is the preferred place to break.
static int WrapText(const u8 *text, u8 font, int maxW, u8 out[][128], int maxLines)
{
    int lines = 0, i = 0;
    while (text[i] != EOS)
    {
        u8 line[128];
        int n = 0, lastSpace = -1, lastSpaceSrc = -1, lastGap = -1, lastGapSrc = -1, j = i;
        while (text[j] == CHAR_SPACE)
            j++;
        i = j;
        if (text[i] == EOS)
            break;
        for (;;)
        {
            u8 c = text[j];
            int len = (c == CHAR_KEYPAD_ICON && text[j + 1] != EOS) ? 2 : 1;   // a button icon: the code and its id
            if (c == EOS || c == CHAR_NEWLINE || n + len >= 127)
                break;
            memcpy(&line[n], &text[j], len);
            line[n + len] = EOS;
            if (GetStringWidth(font, line, 0) > maxW)
            {
                if (lastGap > 0 && lastGap * 2 >= n)
                {
                    n = lastGap;
                    j = lastGapSrc;
                }
                else if (lastSpace > 0)
                {
                    n = lastSpace;
                    j = lastSpaceSrc;
                }
                break;
            }
            if (c == CHAR_SPACE && !(n >= 2 && line[n - 2] == CHAR_KEYPAD_ICON))
            {
                lastSpace = n;
                lastSpaceSrc = j;
                if (n >= 1 && line[n - 1] == CHAR_SPACE)
                {
                    lastGap = n;
                    lastGapSrc = j;
                }
            }
            n += len;
            j += len;
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
            if (len > 0 && last[len - 1] == CHAR_KEYPAD_ICON)   // not half a button icon
                last[--len] = EOS;
        }
        count = maxLines;
    }
    for (i = 0; i < count; i++)
        MpTheme_Print(windowId, font, lines[i], x, y + i * 12, fg, bg);
}

// The help line: what the cursor's on, and the buttons (the "{A} SELECT  {B} BACK" part, from the first button
// icon on) at the right end of the second line, as Emerald's own prompts sit
// A help line into WIN (28 tiles): what the cursor's on at the left, and the button prompts ("{A} SELECT  {B} BACK":
// from the first button icon that follows two spaces) at the right end of the second line, as Emerald's prompts sit
void MpUi_HelpInto(u8 win, const u8 *text)
{
    u8 words[200], keys[96];
    int w = MAIN_W * 8 - 8, kw = 0, n = -1, i;
    if (text[0] == CHAR_KEYPAD_ICON)
    {
        // only prompts: all of them at the right of the second line
        int kw0 = GetStringWidth(FONT_SMALL, text, 0);
        if (kw0 <= w)
        {
            MpTheme_Print(win, FONT_SMALL, text, 4 + w - kw0, 12, MPT_GREY, 0);
            return;
        }
    }
    for (i = 2; text[i] != EOS; i++)
        if (text[i] == CHAR_KEYPAD_ICON && text[i - 1] == CHAR_SPACE && text[i - 2] == CHAR_SPACE)
        {
            n = i;
            break;
        }
    if (n > 0)
    {
        CopyText(keys, text + n, sizeof(keys));
        kw = GetStringWidth(FONT_SMALL, keys, 0);
        if (kw > w / 2)
            kw = 0;
    }
    if (kw)
    {
        memcpy(words, text, n);
        while (n > 0 && words[n - 1] == CHAR_SPACE)
            n--;
        words[n] = EOS;
        // the words on the first line (two if they need them and leave the buttons room), the buttons below right
        if (GetStringWidth(FONT_SMALL, words, 0) <= w)
            MpUi_PrintWrapped(win, words, 4, 0, w, 1, MPT_TEXT, MPT_BG);
        else
            MpUi_PrintWrapped(win, words, 4, 0, w - kw - 8, 2, MPT_TEXT, MPT_BG);
        MpTheme_Print(win, FONT_SMALL, keys, 4 + w - kw, 12, MPT_GREY, 0);
    }
    else
        MpUi_PrintWrapped(win, text, 4, 0, w, 2, MPT_TEXT, MPT_BG);
}

void MpUi_Help(const u8 *text)
{
    if (sHelp == WINDOW_NONE)
        return;
    if (text != sHelpText)
        CopyText(sHelpText, text, sizeof(sHelpText));
    MpTheme_Panel(sHelp, 0);
    MpUi_HelpInto(sHelp, text);
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

// ShowFieldMessage does nothing while another message is still printing: a friend
// who joins mid-sentence would leave the old text up under a new YES / NO
void MP_ReplaceFieldMessage(const u8 *text)
{
    if (!IsFieldMessageBoxHidden())
        StopFieldMessage();
    MP_ShowFieldMessageFitted(text);
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

// Every frame: a help line with button icons follows the device in use
void MpUi_RefreshIcons(void)
{
    static int sFamily = -1;
    int f = Gamepad_IconFamily();
    if (f == sFamily)
        return;
    sFamily = f;
    if (sHelp != WINDOW_NONE && sHelpText[0] != EOS)
    {
        const u8 *c;
        for (c = sHelpText; *c != EOS; c++)
            if (*c == CHAR_KEYPAD_ICON)
            {
                MpUi_Help(sHelpText);
                break;
            }
    }
}
