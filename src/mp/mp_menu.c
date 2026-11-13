// Moemon Co-op: CO-OP start menu entry (status + character customization)
#include <stdio.h>
#include <string.h>
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
#include "constants/trainers.h"
#include "decompress.h"
#include "data.h"
#include "sprite.h"
#include "item.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "mp_life_field.h"
#include "constants/items.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
static void Task_CoopMenu(u8 taskId);
u8 MP_Events_PhoneGoals(char out[][64], s32 *progress, s32 *target, u8 max);

// Pages of the CO-OP menu. Rows are per page; each page ends with BACK/CLOSE.
// No CLOSE row: B or START closes the menu
enum { MAIN_EMOTES, MAIN_FRIENDS, MAIN_TRADE, MAIN_SHARED_PC, MAIN_GATCHA, MAIN_DAILY, MAIN_TOURNEY, MAIN_BERRY, MAIN_REMINDER, MAIN_LOOK, MAIN_ONLINE, MAIN_COUNT, MAIN_CLOSE = 0xFF };
// LOOK rows. BIKE only shows for a character who rides a Pokemon or a vehicle on a bike: the page's
// cursor counts the rows shown (LookRowAt / LookCursorOf)
enum { ROW_GROUP, ROW_SPRITE, ROW_BIKE, ROW_FRONT, ROW_BACKPIC, ROW_SKIN, ROW_HAIR, ROW_OUTFIT, ROW_ACCENT, ROW_CREATE, ROW_EDIT, ROW_SAVE, ROW_REVERT, LOOK_COUNT };
// LOOK > CREATE: a trainer from the parts kit (CUSTOM 1-3)
// Every row works the same way (like LOOK): Left/Right changes it, L/R jumps; each part's color is its own row
enum { CR_SLOT, CR_BODY, CR_SKIN, CR_HAIR, CR_HAIRC, CR_FHAIR, CR_TOP, CR_TOPC, CR_BOTTOM, CR_BOTC,
       CR_HAT, CR_HATC, CR_BAG, CR_BAGC, CR_SAVE, CR_CANCEL, CREATE_COUNT };

#define LINE_H 11
#define VISIBLE_ROWS 9   // more rows than this scroll
extern const u8 EventScript_MpMoveRelearner[];
void MP_Gatcha_Open(void);
#define MENU_W 12   // tiles; the window sits right of the player so you can see your look change

static void (*sIntroDone)(void);   // set while LOOK is open in the new game's intro

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

// The hub's windows (the field shows between them) and FRIENDS' wide one; LOOK / CREATE keep the two above
enum { LAYOUT_NONE, LAYOUT_HUB, LAYOUT_LOOK, LAYOUT_WIDE };
static const struct WindowTemplate sHubMenuTemplate = {
    .bg = 0, .tilemapLeft = 18, .tilemapTop = 1, .width = 11, .height = 13, .paletteNum = 15, .baseBlock = 0x8,
};
static const struct WindowTemplate sHubRoomTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 12, .height = 10, .paletteNum = 15, .baseBlock = 0x8 + 11 * 13,
};
static const struct WindowTemplate sHubHelpTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = 28, .height = 3, .paletteNum = 15, .baseBlock = 0x8 + 11 * 13 + 12 * 10,
};
static const struct WindowTemplate sWideTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 13, .paletteNum = 15, .baseBlock = 0x8,
};
static const struct WindowTemplate sWideHelpTemplate = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = 28, .height = 3, .paletteNum = 15, .baseBlock = 0x8 + 28 * 13,
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

static bool8 LookRowShown(u8 row)
{
    return row != ROW_BIKE || MP_Avatar_HasRide(&gMpConfig.app);
}

static u8 LookRowCount(void)
{
    u8 row, n = 0;
    for (row = 0; row < LOOK_COUNT; row++)
        n += LookRowShown(row);
    return n;
}

// The LOOK row at a cursor position, and the cursor position of a row (the next one shown)
static u8 LookRowAt(u8 cursor)
{
    u8 row;
    for (row = 0; row < LOOK_COUNT; row++)
        if (LookRowShown(row) && cursor-- == 0)
            return row;
    return ROW_REVERT;
}

static u8 LookCursorOf(u8 row)
{
    u8 r, n = 0;
    for (r = 0; r < row && r < LOOK_COUNT; r++)
        n += LookRowShown(r);
    return n;
}

static u16 NumChoices(u8 row)
{
    switch (row)
    {
    case ROW_SPRITE:  return MP_Avatar_ChoiceCount();
    case ROW_FRONT:   return MP_Avatar_FrontChoiceCount();
    case ROW_BACKPIC: return MP_Avatar_BackChoiceCount();
    case ROW_SKIN:    return MP_SKIN_COUNT;
    case ROW_HAIR:    return MP_HAIR_COUNT;
    default:          return MP_COLOR_COUNT;
    }
}

static u8 *RowValue(u8 row)
{
    switch (row)
    {
    case ROW_SPRITE: return &gMpConfig.app.base;
    case ROW_SKIN:   return &gMpConfig.app.skin;
    case ROW_HAIR:   return &gMpConfig.app.hair;
    case ROW_BACKPIC: return &gMpConfig.app.backSel;
    case ROW_OUTFIT: return &gMpConfig.app.outfit;
    default:         return &gMpConfig.app.accent;
    }
}

// The players line (names and top levels), into buf (128 bytes)
void MP_Menu_BuildPlayersLine(u8 *buf)
{
    u8 i, n = 0;
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
            // (StringAppendN copies exactly 7 bytes and never ends the string: a full 7-letter name like
            // SATOSHI ran on into whatever followed, which could hold a placeholder code and crash)
            *StringAppendN(buf, name, PLAYER_NAME_LENGTH) = EOS;
            {
                u8 lv[] = {CHAR_SPACE, CHAR_LV, EOS};
                StringAppend(buf, lv);
            }
            ConvertIntToDecimalStringN(lvl, level, STR_CONV_MODE_LEFT_ALIGN, 3);
            StringAppend(buf, lvl);
        }
    }
}

static void DrawPlayersWindow(u8 windowId)
{
    u8 buf[128];
    MpTheme_Panel(windowId, 0);
    MP_Menu_BuildPlayersLine(buf);
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
    case MP_MENU_PAGE_LOOK:   return LookRowCount();
    case MP_MENU_PAGE_CREATE: return CREATE_COUNT;
    case MP_MENU_PAGE_FRIENDS:
    {
        u8 ids[MP_MAX_PLAYERS];
        return MP_Friends_List(ids) + 1;   // + BACK
    }
    default:                  return MAIN_COUNT;
    }
}

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

static u8 sScroll, sPageShown = 0xFF;

// A small triangle at the right edge: more rows above or below
static void DrawScrollArrow(u8 windowId, int y, bool8 up)
{
    int i, cx = MENU_W * 8 - 6;
    for (i = 0; i < 3; i++)
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_TEXT), cx - i, up ? y + i : y + 2 - i, i * 2 + 1, 1);
}

// LOOK > CREATE state: the slot being made and its parts
static u8 sCreateSlot;                    // 0-2
static struct MpCreated sCreateWork;
static struct MpAppearance sCreateBefore; // the look when CREATE was opened (CANCEL)
static bool8 sCreateShowBack;

// A part's style name, or its color name ("-" when there's only one)
static const char *Create_PartName(u8 cat, bool8 color)
{
    const struct MpCreated *r = &sCreateWork;
    u8 style, col;
    bool8 optional = (cat == MP_CR_HAT || cat == MP_CR_BAG);
    switch (cat)
    {
    case MP_CR_HAIR:   style = r->hair; col = r->hairColor; break;
    case MP_CR_TOP:    style = r->top; col = r->topColor; break;
    case MP_CR_BOTTOM: style = r->bottom; col = r->bottomColor; break;
    case MP_CR_HAT:    style = r->hat; col = r->hatColor; break;
    default:           style = r->bag; col = r->bagColor; break;
    }
    if (optional)
    {
        if (style == 0)
            return color ? "-" : "NONE";
        style--;
    }
    if (!color)
        return MP_Created_StyleName(r->girl, cat, style);
    if (MP_Created_ColorCount(r->girl, cat, style) <= 1)
        return "-";
    return MP_Created_ColorName(r->girl, cat, style, col);
}

// The part a row is about (0xFF: not a part row) and whether it's the color row
static const u8 sCreateRowCat[CREATE_COUNT] = {
    [CR_SLOT] = 0xFF, [CR_BODY] = 0xFF, [CR_SKIN] = 0xFF, [CR_FHAIR] = 0xFF, [CR_SAVE] = 0xFF, [CR_CANCEL] = 0xFF,
    [CR_HAIR] = MP_CR_HAIR, [CR_HAIRC] = MP_CR_HAIR, [CR_TOP] = MP_CR_TOP, [CR_TOPC] = MP_CR_TOP,
    [CR_BOTTOM] = MP_CR_BOTTOM, [CR_BOTC] = MP_CR_BOTTOM, [CR_HAT] = MP_CR_HAT, [CR_HATC] = MP_CR_HAT,
    [CR_BAG] = MP_CR_BAG, [CR_BAGC] = MP_CR_BAG,
};
#define CREATE_IS_COLOR(row) ((row) == CR_HAIRC || (row) == CR_TOPC || (row) == CR_BOTC || (row) == CR_HATC || (row) == CR_BAGC)

static void Create_PrintRow(u8 windowId, u8 row, u8 y)
{
    static const char *const labels[CREATE_COUNT] = {"SLOT", "BODY", "SKIN", "HAIR", " COLOR", "FIELD", "TOP", " COLOR",
                                                     "BOTTOM", " COLOR", "HAT", " COLOR", "BAG", " COLOR", "SAVE", "CANCEL"};
    char value[48];
    switch (row)
    {
    case CR_SLOT: snprintf(value, sizeof(value), "CUSTOM %d", sCreateSlot + 1); break;
    case CR_BODY: snprintf(value, sizeof(value), "%s", sCreateWork.girl ? "GIRL" : "BOY"); break;
    case CR_SKIN: snprintf(value, sizeof(value), "%s", MP_Created_SkinName(sCreateWork.skin)); break;
    case CR_FHAIR: snprintf(value, sizeof(value), "%s", MP_Created_FieldHairName(sCreateWork.fieldHair)); break;
    case CR_SAVE:
    case CR_CANCEL:
        PrintAscii(windowId, labels[row], 8, y);
        return;
    default: snprintf(value, sizeof(value), "%s", Create_PartName(sCreateRowCat[row], CREATE_IS_COLOR(row))); break;
    }
    PrintChoice(windowId, labels[row], value, y, 38);
}

static bool8 LookIsCreated(void)
{
    return gMpConfig.app.base == MP_BASE_CREATED && gMpConfig.app.recipe.made;
}

// ---------------------------------------------------------------------------
// The CO-OP menu's hub (the user's design, 2026-11-13): over the field like the START menu, the room at the top left
// (who's here and where), the menu at the right in three tabs (L / R), each row an item's icon, its name and a line
// of news, the help line along the bottom. The rows are MAIN_* (tCursor holds one), in the tabs' order here.
enum { TAB_PEOPLE, TAB_PLAY, TAB_ME, TAB_COUNT };
#define TAB_ROWS 4
static const u8 sTabRows[TAB_COUNT][TAB_ROWS] = {
    [TAB_PEOPLE] = {MAIN_FRIENDS, MAIN_EMOTES, MAIN_TRADE, MAIN_SHARED_PC},
    [TAB_PLAY]   = {MAIN_TOURNEY, MAIN_GATCHA, MAIN_DAILY, MAIN_BERRY},
    [TAB_ME]     = {MAIN_LOOK, MAIN_REMINDER, MAIN_ONLINE, MAIN_CLOSE},
};
static const char *const sTabNames[TAB_COUNT] = {"PEOPLE", "PLAY", "ME"};
static const u8 sMainIcon[MAIN_COUNT] = {
    [MAIN_EMOTES] = MP_COOPICON_SOCIAL, [MAIN_FRIENDS] = MP_COOPICON_FRIENDS, [MAIN_TRADE] = MP_COOPICON_TRADE,
    [MAIN_SHARED_PC] = MP_COOPICON_SHAREDPC, [MAIN_GATCHA] = MP_COOPICON_GATCHA, [MAIN_DAILY] = MP_COOPICON_DAILY,
    [MAIN_TOURNEY] = MP_COOPICON_TOURNEY, [MAIN_BERRY] = MP_COOPICON_BERRY, [MAIN_REMINDER] = MP_COOPICON_TUTOR,
    [MAIN_LOOK] = MP_COOPICON_LOOK, [MAIN_ONLINE] = MP_COOPICON_ONLINE,
};
#define HUB_ROW_Y0 22
#define HUB_ROW_H  20

static u8 TabOf(u8 main)
{
    u8 t, r;
    for (t = 0; t < TAB_COUNT; t++)
        for (r = 0; r < TAB_ROWS; r++)
            if (sTabRows[t][r] == main)
                return t;
    return TAB_PEOPLE;
}

static u8 RowOf(u8 main)
{
    u8 r, t = TabOf(main);
    for (r = 0; r < TAB_ROWS; r++)
        if (sTabRows[t][r] == main)
            return r;
    return 0;
}

static u8 TabRowCount(u8 tab)
{
    u8 r;
    for (r = 0; r < TAB_ROWS && sTabRows[tab][r] != MAIN_CLOSE; r++)
        ;
    return r;
}

// A row's name and its line of news (ASCII)
static void HubRowText(u8 main, char *name, char *news, int size)
{
    snprintf(news, size, "%s", "");
    switch (main)
    {
    case MAIN_FRIENDS:
    {
        u8 ids[MP_MAX_PLAYERS], n = MP_IsConnected() ? MP_Friends_List(ids) : 0;
        snprintf(name, size, "FRIENDS");
        if (!MP_IsConnected())
            snprintf(news, size, "OFFLINE");
        else
            snprintf(news, size, n ? "%d ONLINE" : "NOBODY YET", n);
        break;
    }
    case MAIN_EMOTES:    snprintf(name, size, "SOCIAL"), snprintf(news, size, "EMOTES, CHAT"); break;
    case MAIN_TRADE:     snprintf(name, size, "TRADE"), snprintf(news, size, "WITH A FRIEND"); break;
    case MAIN_SHARED_PC: snprintf(name, size, "SHARED PC"), snprintf(news, size, "BOXES, ITEMS"); break;
    case MAIN_TOURNEY:   snprintf(name, size, "TOURNAMENT"), snprintf(news, size, "BATTLES, CARDS"); break;
    case MAIN_GATCHA:
    {
        s32 MP_Gatcha_Pulls(void);
        s32 n = MP_Gatcha_Pulls();
        snprintf(name, size, "MOEGATCHA");
        snprintf(news, size, n == 1 ? "1 PULL" : "%d PULLS", (int)n);
        break;
    }
    case MAIN_DAILY:
    {
        char lines[3][64];
        s32 prog[3], target[3];
        u8 n = MP_Events_PhoneGoals(lines, prog, target, 3), i, done = 0;
        for (i = 0; i < n; i++)
            if (prog[i] >= target[i])
                done++;
        snprintf(name, size, "DAILY");
        snprintf(news, size, "%d OF %d DONE", done, n);
        break;
    }
    case MAIN_BERRY:     snprintf(name, size, "BERRY GAMES"), snprintf(news, size, "BLEND, CRUSH"); break;
    case MAIN_LOOK:      snprintf(name, size, "LOOK"), snprintf(news, size, "YOUR TRAINER"); break;
    case MAIN_REMINDER:
    {
        u16 n = CountTotalItemQuantityInBag(ITEM_HEART_SCALE);
        snprintf(name, size, "MOVE TUTOR");
        snprintf(news, size, n == 1 ? "1 HEART SCALE" : "%d HEART SCALES", n);
        break;
    }
    case MAIN_ONLINE:
        if (MP_Lobby_InRoom())
        {
            snprintf(name, size, "SERVER");
            if (MP_InviteCode()[0])
                snprintf(news, size, "CODE %s", MP_InviteCode());
            else
                snprintf(news, size, "ROOM, RULES");
        }
        else
            snprintf(name, size, "GO ONLINE"), snprintf(news, size, "HOST OR JOIN");
        break;
    default:
        snprintf(name, size, "%s", "");
        break;
    }
}

static void PrintCut(u8 win, u8 font, const char *ascii, int x, int y, int w, u8 fg, u8 bg)
{
    u8 buf[64];
    int len;
    MP_AsciiToGame(buf, ascii, 60);
    for (len = StringLength(buf); len > 1 && GetStringWidth(font, buf, 0) > w; len--)
        buf[len - 1] = EOS;
    MpTheme_Print(win, font, buf, x, y, fg, bg);
}

static void DrawHub(u8 win, u8 roomWin, u8 cursor)
{
    u8 tab = TabOf(cursor), row = RowOf(cursor), r, buf[32];
    int w = GetWindowAttribute(win, WINDOW_WIDTH) * 8, i;
    int winX = GetWindowAttribute(win, WINDOW_TILEMAP_LEFT) * 8, winY = GetWindowAttribute(win, WINDOW_TILEMAP_TOP) * 8;
    MpUi_PicBegin();
    if (roomWin != WINDOW_NONE)
    {
        MpUi_DrawRoom(roomWin, -1, NULL);
        CopyWindowToVram(roomWin, COPYWIN_FULL);
    }
    MpTheme_Panel(win, 0);
    // the tab between L and R, three dots for which of the three
    MP_AsciiToGame(buf, "{L}", 8);
    MpTheme_Print(win, FONT_SMALL, buf, 0, 1, MPT_TEXT, MPT_BG);
    MP_AsciiToGame(buf, "{R}", 8);
    MpTheme_Print(win, FONT_SMALL, buf, w - GetStringWidth(FONT_SMALL, buf, 0), 1, MPT_TEXT, MPT_BG);
    MP_AsciiToGame(buf, sTabNames[tab], 20);
    MpTheme_Print(win, FONT_NORMAL, buf, w / 2 - GetStringWidth(FONT_NORMAL, buf, 0) / 2, 0, MPT_TEXT, MPT_BG);
    for (i = 0; i < TAB_COUNT; i++)
    {
        int x = w / 2 - 11 + i * 8;
        FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), x, 15, 5, 5);
        if (i != tab)
            FillWindowPixelRect(win, PIXEL_FILL(MPT_BG), x + 1, 16, 3, 3);
    }
    for (r = 0; r < TabRowCount(tab); r++)
    {
        char name[24], news[32];
        u8 main = sTabRows[tab][r];
        int y = HUB_ROW_Y0 + r * HUB_ROW_H;
        if (r == row)
        {
            // the game's own cursor (the user, 2026-11-13: "I just want the classic cursor")
            u8 arrow[] = {0xEF /* ▶ */, EOS};
            MpTheme_Print(win, FONT_SMALL, arrow, 0, y + 2, MPT_TEXT, 0);
        }
        MpUi_Icon(r, sMainIcon[main], winX + 7, winY + y + 1);
        HubRowText(main, name, news, sizeof(news));
        PrintCut(win, FONT_SMALL, name, 24, y - 2, w - 24, MPT_TEXT, 0);
        PrintCut(win, FONT_SMALL_NARROW, news, 24, y + 7, w - 24, MPT_GREY, 0);
    }
    MpUi_PicEnd();
    CopyWindowToVram(win, COPYWIN_FULL);
}

// The help line under the hub: what the row is for, and the buttons
static void DrawHubHelp(u8 win, u8 cursor)
{
    char ascii[160];
    u8 buf[170];
    const char *keys = "{A} OPEN  {B} CLOSE";
    switch (cursor)
    {
    case MAIN_FRIENDS:
        if (MP_IsConnected() && MP_ExpBonusPercent())
            snprintf(ascii, sizeof(ascii), "Together, your POK~MON earn EXP +%d%%.  %s", MP_ExpBonusPercent(), keys);
        else
            snprintf(ascii, sizeof(ascii), "See where your friends are.  %s", keys);
        break;
    case MAIN_EMOTES:    snprintf(ascii, sizeof(ascii), "Hold {LT} anywhere for emotes, {RT} for phrases.  %s", keys); break;
    case MAIN_TRADE:     snprintf(ascii, sizeof(ascii), "Trade POK~MON, items and money.  %s", keys); break;
    case MAIN_SHARED_PC: snprintf(ascii, sizeof(ascii), "The boxes everyone in the room shares.  %s", keys); break;
    case MAIN_TOURNEY:   snprintf(ascii, sizeof(ascii), "Everyone plays everyone once.  %s", keys); break;
    case MAIN_GATCHA:    snprintf(ascii, sizeof(ascii), "Event MOEMON, items and money.  %s", keys); break;
    case MAIN_DAILY:     snprintf(ascii, sizeof(ascii), "Today's goals, gifts and the weekly race.  %s", keys); break;
    case MAIN_BERRY:     snprintf(ascii, sizeof(ascii), "Blend BERRIES into POK~BLOCKS.  %s", keys); break;
    case MAIN_LOOK:      snprintf(ascii, sizeof(ascii), "Change how you look to everyone.  %s", keys); break;
    case MAIN_REMINDER:  snprintf(ascii, sizeof(ascii), "Remember a move for a HEART SCALE.  %s", keys); break;
    default:
        snprintf(ascii, sizeof(ascii), MP_Lobby_InRoom() ? "Your room, its CODE and rules.  %s" : "Host a game or join a friend's.  %s", keys);
        break;
    }
    MP_AsciiToGame(buf, ascii, 166);
    MpTheme_Panel(win, 0);
    {
        MpUi_HelpInto(win, buf);
    }
    CopyWindowToVram(win, COPYWIN_FULL);
}

// FRIENDS: everyone in the room, one tall row each (head, name, where, what they're doing); A opens their page
static void DrawFriendsList(u8 win, u8 cursor)
{
    u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids), buf[48], i;
    int w = GetWindowAttribute(win, WINDOW_WIDTH) * 8;
    int winX = GetWindowAttribute(win, WINDOW_TILEMAP_LEFT) * 8, winY = GetWindowAttribute(win, WINDOW_TILEMAP_TOP) * 8;
    char ascii[24];
    MpUi_PicBegin();
    MpTheme_Panel(win, 0);
    MP_AsciiToGame(buf, "FRIENDS", 20);
    MpTheme_Print(win, FONT_NORMAL, buf, 2, 0, MPT_TEXT, MPT_BG);
    snprintf(ascii, sizeof(ascii), "%d ONLINE", n);
    MP_AsciiToGame(buf, ascii, 20);
    MpTheme_Print(win, FONT_SMALL, buf, w - 2 - GetStringWidth(FONT_SMALL, buf, 0), 2, MPT_GREY, MPT_BG);
    FillWindowPixelRect(win, PIXEL_FILL(MPT_SHADOW), 0, 14, w, 1);
    for (i = 0; i < n && i < MP_MAX_PLAYERS - 1; i++)
    {
        struct MpPlayer *p = &gMpPlayers[ids[i]];
        int y = 16 + i * 27, rw = 0;
        
        const char *note = NULL;
        if (i == cursor)
        {
            u8 arrow[] = {0xEF /* ▶ */, EOS};
            MpTheme_Print(win, FONT_SMALL, arrow, 3, y + 7, MPT_TEXT, 0);
        }
        MpUi_Head(MPUI_PIC_ROWS + i, ids[i], winX + 12, winY + y + 5);
        StringCopyN(buf, p->name, PLAYER_NAME_LENGTH);
        buf[PLAYER_NAME_LENGTH] = EOS;
        MpTheme_Print(win, FONT_NORMAL, buf, 32, y, MPT_TEXT, 0);
        if (p->hasState && (p->state.flags & MPF_IN_BATTLE))
            note = "IN A BATTLE";
        else if (p->hasState && p->state.cardDuel)
            note = "IN A CARD DUEL";
        if (note)
        {
            MP_AsciiToGame(buf, note, 20);
            rw = GetStringWidth(FONT_SMALL, buf, 0);
            MpTheme_Print(win, FONT_SMALL, buf, w - 4 - rw, y + 6, MPT_GREY, 0);
            MpUi_RoomMark(win, w - 4 - rw - 15, y + 7, '!', TRUE);
            rw += 20;
        }
        MP_Friends_LocationName(ids[i], buf);
        {
            int len;
            for (len = StringLength(buf); len > 1 && GetStringWidth(FONT_SMALL, buf, 0) > w - 36 - rw; len--)
                buf[len - 1] = EOS;
        }
        MpTheme_Print(win, FONT_SMALL, buf, 32, y + 12, MPT_GREY, 0);
    }
    if (n == 0)
    {
        MP_AsciiToGame(buf, "Nobody else is in the room yet.", 40);
        MpTheme_Print(win, FONT_SMALL, buf, 11, 20, MPT_TEXT, MPT_BG);
    }
    MpUi_PicEnd();
    CopyWindowToVram(win, COPYWIN_FULL);
}

static void DrawMenu(u8 windowId, u8 page, u8 cursor, u8 choice)
{
    static const char *const mainLabels[] = {"SOCIAL", "FRIENDS", "TRADE", "SHARED PC", "MOEGATCHA", "DAILY", "TOURNAMENT", "BERRY GAMES", "MOVE TUTOR", "LOOK", ""};
    const char *const lookLabels[] = {"GROUP", "SPRITE", "BIKE", "ID PIC", "BK PIC", "SKIN", "HAIR", "OUTFIT", "ACCENT", "CREATE...",
                                      "EDIT SPRITES...", sIntroDone ? "ALL DONE!" : "SAVE LOOK", "REVERT"};
    char line[64];
    u8 row;

    if (page == MP_MENU_PAGE_MAIN)
    {
        // the room's window grows and shrinks with who's there (and goes when no one else is)
        u8 taskId = FindTaskIdByFunc(Task_CoopMenu), room = WINDOW_NONE;
        if (taskId != TASK_NONE)
            room = gTasks[taskId].data[7] /* tRoom */ = MpUi_RoomWindowFit(gTasks[taskId].data[7], sHubRoomTemplate.baseBlock);
        DrawHub(windowId, room, cursor);
        return;
    }
    if (page == MP_MENU_PAGE_FRIENDS)
    {
        DrawFriendsList(windowId, cursor);
        return;
    }
    MpTheme_Panel(windowId, 0);
    if (page == MP_MENU_PAGE_LOOK)
        snprintf(line, sizeof(line), "LOOK");
    else if (page == MP_MENU_PAGE_CREATE)
        snprintf(line, sizeof(line), "CREATE");
    else if (page == MP_MENU_PAGE_FRIENDS)
        snprintf(line, sizeof(line), "FRIENDS");
    else if (MP_IsConnected())
        snprintf(line, sizeof(line), "CO-OP  %d/%d ONLINE", MP_ConnectedCount(), MP_MAX_PLAYERS);
    else
        snprintf(line, sizeof(line), "CO-OP  OFFLINE");
    {
        // the title in the game's large font, a line under it (as every co-op screen)
        u8 title[64];
        MP_AsciiToGame(title, line, 60);
        MpTheme_Print(windowId, FONT_NORMAL, title, 2, 0, MPT_TEXT, MPT_BG);
        FillWindowPixelRect(windowId, PIXEL_FILL(MPT_SHADOW), 0, 13, MENU_W * 8, 1);
    }
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
            // The lit bar and the game's own cursor
            u8 arrow[] = {0xEF /* ▶ */, EOS};
            MText(windowId, FONT_SMALL, arrow, 1, y);
        }
        if (page == MP_MENU_PAGE_MAIN)
        {
            if (row == MAIN_ONLINE)
                PrintAscii(windowId, MP_Lobby_InRoom() ? "SERVER" : "GO ONLINE", 8, y);
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
        else if (page == MP_MENU_PAGE_CREATE)
        {
            Create_PrintRow(windowId, row, y);
        }
        else if (LookRowAt(row) == ROW_CREATE || LookRowAt(row) == ROW_EDIT || LookRowAt(row) == ROW_SAVE || LookRowAt(row) == ROW_REVERT)
        {
            PrintAscii(windowId, lookLabels[LookRowAt(row)], 8, y);
        }
        else
        {
            u8 look = LookRowAt(row);
            switch (look)
            {
            case ROW_GROUP:  snprintf(line, sizeof(line), "%s", MP_Avatar_GroupName(MP_Avatar_ChoiceGroup(MP_Avatar_GetChoice(&gMpConfig.app)))); break;
            case ROW_SPRITE: snprintf(line, sizeof(line), "%s", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app))); break;
            case ROW_BIKE:   snprintf(line, sizeof(line), "%s", gMpConfig.app.bike ? "BIKE" : "RIDE"); break;
            case ROW_FRONT:  snprintf(line, sizeof(line), "%s", MP_Avatar_FrontSelName(gMpConfig.app.frontSel)); break;
            case ROW_BACKPIC: snprintf(line, sizeof(line), "%s", MP_Avatar_BackSelName(gMpConfig.app.backSel)); break;
            case ROW_SKIN:   snprintf(line, sizeof(line), "%s", MP_Avatar_SkinName(*RowValue(look))); break;
            case ROW_HAIR:   snprintf(line, sizeof(line), "%s", MP_Avatar_HairName(*RowValue(look))); break;
            default:         snprintf(line, sizeof(line), "%s", MP_Avatar_ColorName(*RowValue(look))); break;
            }
            // A CUSTOM look's colors come from its parts
            if (look >= ROW_SKIN && look <= ROW_ACCENT && LookIsCreated())
                snprintf(line, sizeof(line), "CREATE");
            if (look == ROW_GROUP)
                PrintChoice(windowId, "", line, y, 14);   // the group's name across the row: < NAMED NPC >
            else
                PrintChoice(windowId, lookLabels[look], line, y, 38);
        }
    }
    sTxtFg = MPT_TEXT;
    sTxtBg = MPT_BG;
    CopyWindowToVram(windowId, COPYWIN_FULL);
}

// The bottom window: players online, or button help on the EMOTES page
static void DrawBottom(u8 windowId, u8 page, u8 cursor)
{
    if (page == MP_MENU_PAGE_MAIN)
    {
        DrawHubHelp(windowId, cursor);
        return;
    }
    if (page == MP_MENU_PAGE_FRIENDS)
    {
        u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids), buf[120], where[40];
        char ascii[120], name[16];
        MpTheme_Panel(windowId, 0);
        if (n == 0)
            snprintf(ascii, sizeof(ascii), "Share your room's CODE (ME > SERVER).  {B} BACK");
        else
        {
            MP_GameToAscii(name, gMpPlayers[ids[cursor < n ? cursor : 0]].name, PLAYER_NAME_LENGTH);
            MP_Friends_LocationName(ids[cursor < n ? cursor : 0], where);
            snprintf(ascii, sizeof(ascii), "GO TO, TRAINER CARD, SECRET BASE, CARD GAMES.  {A} OPEN  {B} BACK");
        }
        MP_AsciiToGame(buf, ascii, 116);
        MpUi_HelpInto(windowId, buf);
        CopyWindowToVram(windowId, COPYWIN_FULL);
        return;
    }
    if (FALSE)
    {
        u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids), buf[96], town[32];
        MpTheme_Panel(windowId, 0);
        if (n == 0)
            MP_AsciiToGame(buf, "Nobody else is online right now.", 60);
        else if (cursor >= n)
            MP_AsciiToGame(buf, "Back to the CO-OP menu.", 70);
        else
        {
            (void)town;
            MP_AsciiToGame(buf, "{A} GO TO, TRAINER CARD, SECRET BASE", 70);
        }
        MpUi_PrintWrapped(windowId, buf, 2, 1, 28 * 8 - 4, 1, sTxtFg, sTxtBg);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
    else if (page == MP_MENU_PAGE_LOOK)
    {
        // The full name (the row cuts long ones), who drew it, and the keys
        char ascii[160];
        u8 buf[170];
        MpTheme_Panel(windowId, 0);
        cursor = LookRowAt(cursor);
        if (cursor == ROW_GROUP)
            snprintf(ascii, sizeof(ascii), "%s: %s", MP_Avatar_GroupName(MP_Avatar_ChoiceGroup(MP_Avatar_GetChoice(&gMpConfig.app))),
                MP_Avatar_ChoiceGroup(MP_Avatar_GetChoice(&gMpConfig.app)) == MP_LOOK_NAMED ? "people with a name   {LEFTRIGHT} group"
                : MP_Avatar_ChoiceGroup(MP_Avatar_GetChoice(&gMpConfig.app)) == MP_LOOK_GENERIC ? "classes and townsfolk   {LEFTRIGHT} group"
                : "CREATE and sprite sheets   {LEFTRIGHT} group");
        else if (cursor == ROW_BIKE)
            snprintf(ascii, sizeof(ascii), "%s   {LEFTRIGHT} change",
                gMpConfig.app.bike ? "BIKE: on a bike, a bicycle" : "RIDE: on a bike, their ride as drawn");
        else if (cursor == ROW_SPRITE && gMpConfig.app.base == MP_BASE_CREATED && !gMpConfig.app.recipe.made)
            snprintf(ascii, sizeof(ascii), "%s: not made yet (looks like DEFAULT)", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app)));
        else if (cursor == ROW_SPRITE)
            snprintf(ascii, sizeof(ascii), "%s   {START} match pics   {SELECT} undo", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app)));
        else if (cursor == ROW_FRONT && MP_Avatar_FrontSelArtist(gMpConfig.app.frontSel)[0])
            snprintf(ascii, sizeof(ascii), "%s  Art by %s, shown as drawn", MP_Avatar_FrontSelName(gMpConfig.app.frontSel), MP_Avatar_FrontSelArtist(gMpConfig.app.frontSel));
        else if (cursor == ROW_FRONT)
            snprintf(ascii, sizeof(ascii), "%s   {START} match   {SELECT} undo", gMpConfig.app.frontSel ? MP_Avatar_FrontSelName(gMpConfig.app.frontSel) : "MATCH SPRITE");
        else if (cursor == ROW_BACKPIC)
            snprintf(ascii, sizeof(ascii), "%s   {START} match   {SELECT} undo", gMpConfig.app.backSel ? MP_Avatar_BackSelName(gMpConfig.app.backSel) : "MATCH SPRITE");
        else if (cursor >= ROW_SKIN && cursor <= ROW_ACCENT && LookIsCreated())
            snprintf(ascii, sizeof(ascii), "CUSTOM colors are set in CREATE. {A} open it");
        else if (cursor == ROW_HAIR)
            snprintf(ascii, sizeof(ascii), "HAIR (a hat if it hides the hair)   {SELECT} undo");
        else if (cursor <= ROW_ACCENT)
            snprintf(ascii, sizeof(ascii), "{LEFTRIGHT} change   {SELECT} undo");
        else if (cursor == ROW_CREATE)
            snprintf(ascii, sizeof(ascii), "Make your own trainer (CUSTOM 1-3).");
        else if (cursor == ROW_EDIT && sIntroDone)
            snprintf(ascii, sizeof(ascii), "The Sprite Editor opens later from {START} > CO-OP > LOOK.");
        else if (cursor == ROW_EDIT)
            snprintf(ascii, sizeof(ascii), "Draw your own look in the Sprite Editor.");
        else if (cursor == ROW_SAVE && sIntroDone)
            snprintf(ascii, sizeof(ascii), "Show PROF. BIRCH your look. ({B} does too.)");
        else if (cursor == ROW_SAVE)
            snprintf(ascii, sizeof(ascii), "Keep this look and go back. ({B} does too.)");
        else
            snprintf(ascii, sizeof(ascii), "Undo every change made since you opened LOOK.");
        MP_AsciiToGame(buf, ascii, 150);
        MpUi_PrintWrapped(windowId, buf, 2, 1, 28 * 8 - 4, 1, sTxtFg, sTxtBg);
        CopyWindowToVram(windowId, COPYWIN_FULL);
    }
    else if (page == MP_MENU_PAGE_CREATE)
    {
        char ascii[160];
        u8 buf[170];
        MpTheme_Panel(windowId, 0);
        switch (cursor)
        {
        case CR_SLOT:
            snprintf(ascii, sizeof(ascii), "CUSTOM %d%s   {LEFTRIGHT} slot   {START} flip pic", sCreateSlot + 1,
                gMpConfig.custom[sCreateSlot].made ? "" : " (new)");
            break;
        case CR_BODY:
            snprintf(ascii, sizeof(ascii), "%s   {LEFTRIGHT} change (resets the parts)", sCreateWork.girl ? "GIRL" : "BOY");
            break;
        case CR_SKIN:
            snprintf(ascii, sizeof(ascii), "%s   {LEFTRIGHT} change   {SELECT} undo", MP_Created_SkinName(sCreateWork.skin));
            break;
        case CR_FHAIR:
            snprintf(ascii, sizeof(ascii), "FIELD HAIR: %s%s   {L}{R} jump", MP_Created_FieldHairName(sCreateWork.fieldHair),
                sCreateWork.fieldHair ? "" : " (like HAIR)");
            break;
        case CR_SAVE:
            snprintf(ascii, sizeof(ascii), "Save as CUSTOM %d and wear it.", sCreateSlot + 1);
            break;
        case CR_CANCEL:
            snprintf(ascii, sizeof(ascii), "Back to LOOK without saving. ({B} does too.)");
            break;
        default:
            if (CREATE_IS_COLOR(cursor))
                snprintf(ascii, sizeof(ascii), "%s color: %s   {LEFTRIGHT} change", Create_PartName(sCreateRowCat[cursor], FALSE),
                    Create_PartName(sCreateRowCat[cursor], TRUE));
            else if (cursor == CR_BAG)
                snprintf(ascii, sizeof(ascii), "%s  (only on the field sprite)", Create_PartName(MP_CR_BAG, FALSE));
            else
                snprintf(ascii, sizeof(ascii), "%s   {LEFTRIGHT} style  {L}{R} jump", Create_PartName(sCreateRowCat[cursor], FALSE));
            break;
        }
        MP_AsciiToGame(buf, ascii, 150);
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
#define tRoom     data[7]   // the room window (the hub's layout)
#define tLayout   data[8]   // LAYOUT_*: which windows are up

// ---------------------------------------------------------------------------
// LOOK page preview: the trainer picture (or the back picture on BACK PIC)
// in a panel left of the player, updated as the choices change

#define PREVIEW_TAG 0x5E11
static u8 sPreviewWin = WINDOW_NONE;
static u8 sPreviewSprite = MAX_SPRITES;
static ALIGNED(4) u8 sPreviewTiles[TRAINER_PIC_SIZE];
static ALIGNED(4) u16 sPreviewPal[16];

static const struct WindowTemplate sPreviewWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 1,
    .width = 10,
    .height = 11,
    .paletteNum = 15,
    .baseBlock = 0x8 + MENU_W * 15 + 28 * 2,
};

static const struct OamData sPreviewOam = {
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
    .priority = 0,
};

static void Preview_Draw(bool8 back)
{
    bool8 has;
    u8 buf[40];
    if (sPreviewWin == WINDOW_NONE)
        return;
    has = back ? MP_Avatar_PreviewBack(sPreviewTiles, sPreviewPal) : MP_Avatar_PreviewFront(sPreviewTiles, sPreviewPal);
    MpTheme_Panel(sPreviewWin, 0);
    sTxtFg = MPT_TEXT;
    sTxtBg = MPT_BG;
    MP_AsciiToGame(buf, back ? "BACK PIC" : "ID PIC", 30);
    MText(sPreviewWin, FONT_SMALL, buf, 2, 0);
    FillWindowPixelRect(sPreviewWin, PIXEL_FILL(MPT_SHADOW), 0, 12, 10 * 8, 1);
    sTxtFg = MPT_TEXT;
    sTxtBg = MPT_BG;
    if (!has)
    {
        MP_AsciiToGame(buf, "NONE:", 30);
        MText(sPreviewWin, FONT_SMALL, buf, 6, 30);
        MP_AsciiToGame(buf, "hidden in", 30);
        MText(sPreviewWin, FONT_SMALL, buf, 6, 44);
        MP_AsciiToGame(buf, "battles", 30);
        MText(sPreviewWin, FONT_SMALL, buf, 6, 56);
    }
    CopyWindowToVram(sPreviewWin, COPYWIN_FULL);
    if (sPreviewSprite != MAX_SPRITES)
    {
        u16 tile = GetSpriteTileStartByTag(PREVIEW_TAG);
        u8 palIdx = IndexOfSpritePaletteTag(PREVIEW_TAG);
        gSprites[sPreviewSprite].invisible = !has;
        if (has && tile != 0xFFFF)
            memcpy((void *)(OBJ_VRAM0 + tile * TILE_SIZE_4BPP), sPreviewTiles, TRAINER_PIC_SIZE);
        if (has && palIdx != 0xFF)
            LoadPalette(sPreviewPal, OBJ_PLTT_ID(palIdx), PLTT_SIZE_4BPP);
    }
}

static void Preview_Show(bool8 back)
{
    if (sPreviewWin == WINDOW_NONE)
    {
        struct SpriteSheet sheet = {sPreviewTiles, TRAINER_PIC_SIZE, PREVIEW_TAG};
        struct SpritePalette pal = {sPreviewPal, PREVIEW_TAG};
        struct SpriteTemplate tmpl = {
            .tileTag = PREVIEW_TAG, .paletteTag = PREVIEW_TAG, .oam = &sPreviewOam,
            .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
            .callback = SpriteCallbackDummy,
        };
        sPreviewWin = AddWindow(&sPreviewWindowTemplate);
        DrawStdWindowFrame(sPreviewWin, FALSE);
        memset(sPreviewTiles, 0, sizeof(sPreviewTiles));
        memset(sPreviewPal, 0, sizeof(sPreviewPal));
        if (LoadSpriteSheet(&sheet) != 0 && LoadSpritePalette(&pal) != 0xFF)
            sPreviewSprite = CreateSprite(&tmpl, 8 + 40, 8 + 14 + 32, 0);
        else
            sPreviewSprite = MAX_SPRITES;
    }
    Preview_Draw(back);
}

static void Preview_Hide(void)
{
    if (sPreviewSprite != MAX_SPRITES)
    {
        DestroySprite(&gSprites[sPreviewSprite]);
        sPreviewSprite = MAX_SPRITES;
    }
    FreeSpriteTilesByTag(PREVIEW_TAG);
    FreeSpritePaletteByTag(PREVIEW_TAG);
    if (sPreviewWin != WINDOW_NONE)
    {
        ClearStdWindowAndFrame(sPreviewWin, TRUE);
        RemoveWindow(sPreviewWin);
        sPreviewWin = WINDOW_NONE;
    }
}

static void RemoveLayout(s16 *data)
{
    Preview_Hide();
    MpUi_PicsClear();
    if (tRoom != WINDOW_NONE)
    {
        ClearStdWindowAndFrame(tRoom, TRUE);
        RemoveWindow(tRoom);
        tRoom = WINDOW_NONE;
    }
    if (tLayout != LAYOUT_NONE)
    {
        ClearStdWindowAndFrame(tWindow, TRUE);
        ClearStdWindowAndFrame(tPlayers, TRUE);
        RemoveWindow(tWindow);
        RemoveWindow(tPlayers);
    }
    tLayout = LAYOUT_NONE;
}

// The windows a page needs: the hub's three, FRIENDS' wide one, LOOK / CREATE's
static void SetLayout(s16 *data, u8 page)
{
    u8 layout = page == MP_MENU_PAGE_MAIN ? LAYOUT_HUB : page == MP_MENU_PAGE_FRIENDS ? LAYOUT_WIDE : LAYOUT_LOOK;
    if (layout == tLayout)
        return;
    RemoveLayout(data);
    tLayout = layout;
    if (layout == LAYOUT_HUB)
    {
        tWindow = AddWindow(&sHubMenuTemplate);
        tRoom = MpUi_RoomWindowFit(WINDOW_NONE, sHubRoomTemplate.baseBlock);   // (none when playing alone)
        tPlayers = AddWindow(&sHubHelpTemplate);
    }
    else if (layout == LAYOUT_WIDE)
    {
        tWindow = AddWindow(&sWideTemplate);
        tPlayers = AddWindow(&sWideHelpTemplate);
    }
    else
    {
        tWindow = AddWindow(&sCoopWindowTemplate);
        tPlayers = AddWindow(&sPlayersWindowTemplate);
    }
    PutWindowTilemap(tWindow);
    PutWindowTilemap(tPlayers);
    DrawStdWindowFrame(tWindow, FALSE);
    DrawStdWindowFrame(tPlayers, FALSE);
}

static void CloseMenuWindows(s16 *data)
{
    RemoveLayout(data);
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

static void Look_Snapshot(void);
static void Create_Open(void);

static void GoToPage(s16 *data, u8 page, u8 cursor)
{
    u8 from = tPage;
    tPage = page;
    tCursor = cursor;
    tChoice = 0;
    SetLayout(data, page);
    if (page == MP_MENU_PAGE_LOOK)
    {
        if (from != MP_MENU_PAGE_CREATE)
            Look_Snapshot();
        Preview_Show(LookRowAt(cursor) == ROW_BACKPIC);
    }
    else if (page == MP_MENU_PAGE_CREATE)
        Preview_Show(sCreateShowBack);
    else
        Preview_Hide();
    DrawMenu(tWindow, tPage, tCursor, tChoice);
    DrawBottom(tPlayers, tPage, tCursor);
}

// The look when LOOK was opened, for REVERT and SELECT (undo one row)
static struct MpAppearance sLookBefore;
static char sLookBeforePack[sizeof(gMpConfig.customSprite)];

static void Look_Snapshot(void)
{
    sLookBefore = gMpConfig.app;
    memcpy(sLookBeforePack, gMpConfig.customSprite, sizeof(sLookBeforePack));
}

// Puts one row (or every row) back the way it was. TRUE when something changed.
static bool8 Look_Undo(u8 row)
{
    struct MpAppearance *app = &gMpConfig.app, before = *app;
    bool8 all = (row == ROW_REVERT);
    if (all || row == ROW_SPRITE || row == ROW_GROUP)
        MP_Avatar_RestoreSprite(&sLookBefore, sLookBeforePack);
    if (all || row == ROW_BIKE)    app->bike = sLookBefore.bike;
    if (all || row == ROW_FRONT)   app->frontSel = sLookBefore.frontSel;
    if (all || row == ROW_BACKPIC) app->backSel = sLookBefore.backSel;
    if (all || row == ROW_SKIN)    app->skin = sLookBefore.skin;
    if (all || row == ROW_HAIR)    app->hair = sLookBefore.hair;
    if (all || row == ROW_OUTFIT)  app->outfit = sLookBefore.outfit;
    if (all || row == ROW_ACCENT)  app->accent = sLookBefore.accent;
    return memcmp(&before, app, sizeof(before)) != 0;
}

static u16 sGroupLast[MP_LOOK_GROUP_COUNT];   // LOOK > GROUP: the sprite last worn in each group

static void HandleLookInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    bool8 changed = FALSE;
    u8 row = LookRowAt(tCursor);

    if ((row == ROW_CREATE && JOY_NEW(A_BUTTON))
     || (row >= ROW_SKIN && row <= ROW_ACCENT && LookIsCreated() && JOY_NEW(A_BUTTON | DPAD_LEFT | DPAD_RIGHT)))
    {
        PlaySE(SE_SELECT);
        Create_Open();
        GoToPage(data, MP_MENU_PAGE_CREATE, CR_SLOT);
        return;
    }
    if (row == ROW_EDIT && JOY_NEW(A_BUTTON) && sIntroDone)
    {
        PlaySE(SE_FAILURE);   // no editor during the new game's intro
        return;
    }
    if (row == ROW_EDIT && JOY_NEW(A_BUTTON))
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
    else if (row == ROW_SPRITE && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        // Many sprites: Left/Right steps through the group (hold to scroll), L/R jumps by 10
        u16 c = MP_Avatar_GetChoice(&gMpConfig.app);
        u8 g = MP_Avatar_ChoiceGroup(c);
        u16 first = MP_Avatar_GroupFirst(g), n = MP_Avatar_GroupSize(g);
        u16 step = JOY_REPEAT(L_BUTTON | R_BUTTON) ? 10 : 1;
        if (n > 0)
        {
            c -= first;
            if (JOY_REPEAT(DPAD_RIGHT | R_BUTTON))
                c = (c + step) % n;
            else
                c = (c + n - (step % n)) % n;
            MP_Avatar_SetChoice(&gMpConfig.app, first + c);
            sGroupLast[g] = first + c;
            changed = TRUE;
        }
    }
    else if (row == ROW_GROUP && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        // NAMED NPC / GENERICS / CUSTOM: the sprite last worn in that group (its first one at first)
        u8 g = MP_Avatar_ChoiceGroup(MP_Avatar_GetChoice(&gMpConfig.app));
        sGroupLast[g] = MP_Avatar_GetChoice(&gMpConfig.app);
        g = JOY_REPEAT(DPAD_RIGHT) ? (g + 1) % MP_LOOK_GROUP_COUNT : (g + MP_LOOK_GROUP_COUNT - 1) % MP_LOOK_GROUP_COUNT;
        if (MP_Avatar_ChoiceGroup(sGroupLast[g]) != g)
            sGroupLast[g] = MP_Avatar_GroupFirst(g);
        MP_Avatar_SetChoice(&gMpConfig.app, sGroupLast[g]);
        changed = TRUE;
    }
    else if (row == ROW_BIKE && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        gMpConfig.app.bike = !gMpConfig.app.bike;
        changed = TRUE;
    }
    else if (row == ROW_FRONT && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        // Hundreds of pictures: Left/Right steps, L/R jumps by 10
        u16 n = NumChoices(ROW_FRONT);
        u16 c = gMpConfig.app.frontSel;
        u16 step = JOY_REPEAT(L_BUTTON | R_BUTTON) ? 10 : 1;
        if (JOY_REPEAT(DPAD_RIGHT | R_BUTTON))
            c = (c + step) % n;
        else
            c = (c + n - (step % n)) % n;
        gMpConfig.app.frontSel = c;
        changed = TRUE;
    }
    else if ((row == ROW_BACKPIC || (row >= ROW_SKIN && row <= ROW_ACCENT)) && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        u8 *v = RowValue(row);
        u8 n = NumChoices(row);
        *v = JOY_REPEAT(DPAD_RIGHT) ? (*v + 1) % n : (*v + n - 1) % n;
        changed = TRUE;
    }
    else if ((JOY_NEW(SELECT_BUTTON) && row <= ROW_ACCENT) || (JOY_NEW(A_BUTTON) && row == ROW_REVERT))
    {
        // Undo this row (SELECT) or everything (REVERT)
        if (Look_Undo(row))
            changed = TRUE;
        else
            PlaySE(SE_FAILURE);
    }
    else if (JOY_NEW(START_BUTTON))
    {
        // ID PIC / BK PIC back to the ones that go with the sprite (NONE if it has none).
        // On those rows just that one, anywhere else both.
        struct MpAppearance before = gMpConfig.app;
        MP_Avatar_MatchPictures(&gMpConfig.app, row != ROW_BACKPIC, row != ROW_FRONT);
        if (memcmp(&before, &gMpConfig.app, sizeof(before)) != 0)
            changed = TRUE;
        else
            PlaySE(SE_FAILURE);
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && row == ROW_SAVE))
    {
        PlaySE(SE_SAVE);
        MP_SaveConfig();
        Look_Snapshot();
        if (sIntroDone)
        {
            // The new game's intro: back to PROF. BIRCH
            void (*done)(void) = sIntroDone;
            sIntroDone = NULL;
            CloseMenu(taskId);
            done();
            return;
        }
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_LOOK);
    }
    if (changed)
    {
        PlaySE(SE_SELECT);
        if (!LookRowShown(row))
            row = ROW_SPRITE;   // BIKE went away (REVERT)
        tCursor = LookCursorOf(row);
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        Preview_Draw(row == ROW_BACKPIC);
    }
}

// Wears the parts being made (the field sprite and the preview show them)
static void Create_Wear(void)
{
    struct MpAppearance *app = &gMpConfig.app;
    MP_Created_Clamp(&sCreateWork);
    app->base = MP_BASE_CREATED;
    app->created = sCreateSlot + 1;
    app->recipe = sCreateWork;
    app->recipe.made = 1;
    app->gfxId = 0;
    app->hasCustom = FALSE;
    app->frontSel = 0;
    app->backSel = 0;
    MP_Avatar_ApplyLocal();
    MP_SendAppearance();
}

static void Create_LoadSlot(u8 slot)
{
    sCreateSlot = slot;
    if (gMpConfig.custom[slot].made)
        sCreateWork = gMpConfig.custom[slot];
    else
        MP_Created_Default(&sCreateWork, gSaveBlock2Ptr->playerGender == FEMALE);
    Create_Wear();
}

static void Create_Open(void)
{
    u8 i, slot = 0;
    sCreateBefore = gMpConfig.app;
    sCreateShowBack = FALSE;
    // The CUSTOM being worn, else the first one not made yet
    if (gMpConfig.app.base == MP_BASE_CREATED && gMpConfig.app.created >= 1 && gMpConfig.app.created <= MP_CREATED_SLOTS)
        slot = gMpConfig.app.created - 1;
    else
    {
        for (i = 0; i < MP_CREATED_SLOTS; i++)
            if (!gMpConfig.custom[i].made)
                break;
        slot = (i < MP_CREATED_SLOTS) ? i : 0;
    }
    Create_LoadSlot(slot);
}

// Steps a part's style or color (dir: +-1, or +-5 for L/R)
static void Create_StepPart(u8 cat, bool8 color, int dir)
{
    struct MpCreated *r = &sCreateWork;
    bool8 optional = (cat == MP_CR_HAT || cat == MP_CR_BAG);
    u8 *style, *col, n;
    switch (cat)
    {
    case MP_CR_HAIR:   style = &r->hair; col = &r->hairColor; break;
    case MP_CR_TOP:    style = &r->top; col = &r->topColor; break;
    case MP_CR_BOTTOM: style = &r->bottom; col = &r->bottomColor; break;
    case MP_CR_HAT:    style = &r->hat; col = &r->hatColor; break;
    default:           style = &r->bag; col = &r->bagColor; break;
    }
    if (color)
    {
        if (optional && *style == 0)
            return;
        n = MP_Created_ColorCount(r->girl, cat, optional ? *style - 1 : *style);
        if (n > 1)
            *col = ((*col + dir) % n + n) % n;
        return;
    }
    n = MP_Created_StyleCount(r->girl, cat) + (optional ? 1 : 0);
    {
        // Keep the color when the new style has it (same name)
        const char *was = MP_Created_ColorName(r->girl, cat, optional ? *style - 1 : *style, *col);
        u8 i, nc;
        *style = ((*style + dir) % n + n) % n;
        *col = 0;
        if (optional && *style == 0)
            return;
        nc = MP_Created_ColorCount(r->girl, cat, optional ? *style - 1 : *style);
        for (i = 0; i < nc; i++)
            if (was[0] && strcmp(MP_Created_ColorName(r->girl, cat, optional ? *style - 1 : *style, i), was) == 0)
                *col = i;
    }
}

static void HandleCreateInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    struct MpCreated before = sCreateWork;
    u8 slotBefore = sCreateSlot;
    int dir = JOY_REPEAT(DPAD_RIGHT | R_BUTTON) ? 1 : -1;
    bool8 jump = JOY_REPEAT(L_BUTTON | R_BUTTON) && !JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT);
    u8 cat = sCreateRowCat[tCursor];

    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == CR_CANCEL))
    {
        // Back to the look you had
        PlaySE(SE_SELECT);
        gMpConfig.app = sCreateBefore;
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        GoToPage(data, MP_MENU_PAGE_LOOK, LookCursorOf(ROW_CREATE));
        return;
    }
    if (JOY_NEW(A_BUTTON) && tCursor == CR_SAVE)
    {
        PlaySE(SE_SAVE);
        sCreateWork.made = 1;
        gMpConfig.custom[sCreateSlot] = sCreateWork;
        Create_Wear();
        MP_SaveConfig();
        GoToPage(data, MP_MENU_PAGE_LOOK, LookCursorOf(ROW_SPRITE));
        return;
    }
    if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        sCreateShowBack = !sCreateShowBack;
        Preview_Draw(sCreateShowBack);
        return;
    }
    if (JOY_NEW(SELECT_BUTTON) && tCursor >= CR_SKIN && tCursor <= CR_BAGC)
    {
        // Undo this row: back to the slot's saved parts (or the starting ones)
        struct MpCreated orig;
        if (gMpConfig.custom[sCreateSlot].made && gMpConfig.custom[sCreateSlot].girl == sCreateWork.girl)
            orig = gMpConfig.custom[sCreateSlot];
        else
            MP_Created_Default(&orig, sCreateWork.girl);
        switch (tCursor)
        {
        case CR_SKIN:   sCreateWork.skin = orig.skin; break;
        case CR_HAIR:   sCreateWork.hair = orig.hair; sCreateWork.hairColor = orig.hairColor; break;
        case CR_HAIRC:  if (sCreateWork.hair == orig.hair) sCreateWork.hairColor = orig.hairColor; break;
        case CR_FHAIR:  sCreateWork.fieldHair = orig.fieldHair; break;
        case CR_TOP:    sCreateWork.top = orig.top; sCreateWork.topColor = orig.topColor; break;
        case CR_TOPC:   if (sCreateWork.top == orig.top) sCreateWork.topColor = orig.topColor; break;
        case CR_BOTTOM: sCreateWork.bottom = orig.bottom; sCreateWork.bottomColor = orig.bottomColor; break;
        case CR_BOTC:   if (sCreateWork.bottom == orig.bottom) sCreateWork.bottomColor = orig.bottomColor; break;
        case CR_HAT:    sCreateWork.hat = orig.hat; sCreateWork.hatColor = orig.hatColor; break;
        case CR_HATC:   if (sCreateWork.hat == orig.hat) sCreateWork.hatColor = orig.hatColor; break;
        case CR_BAG:    sCreateWork.bag = orig.bag; sCreateWork.bagColor = orig.bagColor; break;
        case CR_BAGC:   if (sCreateWork.bag == orig.bag) sCreateWork.bagColor = orig.bagColor; break;
        }
        if (memcmp(&before, &sCreateWork, sizeof(before)) == 0)
            PlaySE(SE_FAILURE);
    }
    else if (JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT) || jump)
    {
        int step = jump ? dir * 5 : dir;
        switch (tCursor)
        {
        case CR_SLOT:
            Create_LoadSlot((sCreateSlot + MP_CREATED_SLOTS + dir) % MP_CREATED_SLOTS);
            break;
        case CR_BODY:
        {
            u8 skin = sCreateWork.skin;
            MP_Created_Default(&sCreateWork, !sCreateWork.girl);
            sCreateWork.skin = skin;
            break;
        }
        case CR_SKIN:
            sCreateWork.skin = (sCreateWork.skin + MP_CR_SKIN_COUNT + dir) % MP_CR_SKIN_COUNT;
            break;
        case CR_FHAIR:
        {
            u8 n = MP_Created_FieldHairCount();
            sCreateWork.fieldHair = ((sCreateWork.fieldHair + step) % n + n) % n;
            break;
        }
        default:
            if (cat != 0xFF)
                Create_StepPart(cat, CREATE_IS_COLOR(tCursor), step);
            break;
        }
        if (memcmp(&before, &sCreateWork, sizeof(before)) == 0 && slotBefore == sCreateSlot && tCursor != CR_SAVE && tCursor != CR_CANCEL)
            PlaySE(SE_FAILURE);
    }
    if (memcmp(&before, &sCreateWork, sizeof(before)) != 0 || slotBefore != sCreateSlot)
    {
        PlaySE(SE_SELECT);
        if (slotBefore == sCreateSlot)
            Create_Wear();
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        Preview_Draw(sCreateShowBack);
    }
}

static void HandleFriendsInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids);
    if (n > MP_MAX_PLAYERS - 1)
        n = MP_MAX_PLAYERS - 1;
    if (tCursor >= n)
        tCursor = n ? n - 1 : 0; // someone left
    if (n > 1 && JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        tCursor = JOY_REPEAT(DPAD_UP) ? (tCursor + n - 1) % n : (tCursor + 1) % n;
        PlaySE(SE_SELECT);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && n == 0))
    {
        PlaySE(SE_SELECT);
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_FRIENDS);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u8 who = ids[tCursor];
        CloseMenu(taskId);
        MP_Linkx_OpenFriend(who);
    }
}

static void HandleMainInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 tab = TabOf(tCursor), row = RowOf(tCursor);
    if (JOY_REPEAT(DPAD_UP | DPAD_DOWN))
    {
        u8 n = TabRowCount(tab);
        row = JOY_REPEAT(DPAD_UP) ? (row + n - 1) % n : (row + 1) % n;
        tCursor = sTabRows[tab][row];
        PlaySE(SE_SELECT);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        return;
    }
    if (JOY_NEW(L_BUTTON | R_BUTTON | DPAD_LEFT | DPAD_RIGHT))
    {
        tab = JOY_NEW(L_BUTTON | DPAD_LEFT) ? (tab + TAB_COUNT - 1) % TAB_COUNT : (tab + 1) % TAB_COUNT;
        if (row >= TabRowCount(tab))
            row = TabRowCount(tab) - 1;
        tCursor = sTabRows[tab][row];
        PlaySE(SE_SELECT);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        return;
    }
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
    case MAIN_BERRY:
        CloseMenu(taskId);
        MP_Linkx_OpenUnion();
        break;
    case MAIN_TOURNEY:
        CloseMenu(taskId);
        MP_Tourney_OpenMenu();
        break;
    case MAIN_LOOK:
        GoToPage(data, MP_MENU_PAGE_LOOK, LookCursorOf(ROW_SPRITE));
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
        // The map name sign shares the frame's palette slot: load the player's frame again
        HideMapNamePopUpWindow();
        LoadMessageBoxAndBorderGfx();
        MpTheme_Push();
        tRoom = WINDOW_NONE;
        tLayout = LAYOUT_NONE;
        SetLayout(data, tPage);
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        if (tPage == MP_MENU_PAGE_LOOK)
        {
            Look_Snapshot();
            Preview_Show(LookRowAt(tCursor) == ROW_BACKPIC);
        }
        tState++;
        break;
    case 1:
        if (++tRefresh >= 60)
        {
            tRefresh = 0;
            DrawBottom(tPlayers, tPage, tCursor);
            DrawMenu(tWindow, tPage, tCursor, tChoice);
        }
        if (tPage == MP_MENU_PAGE_MAIN)
            HandleMainInput(taskId);
        else if (tPage == MP_MENU_PAGE_FRIENDS)
            HandleFriendsInput(taskId);
        else if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
        {
            u8 n = RowCount(tPage);
            tCursor = JOY_NEW(DPAD_UP) ? (tCursor == 0 ? n - 1 : tCursor - 1) : (tCursor + 1) % n;
            tChoice = 0;
            PlaySE(SE_SELECT);
            DrawMenu(tWindow, tPage, tCursor, tChoice);
            if (tPage == MP_MENU_PAGE_FRIENDS || tPage == MP_MENU_PAGE_LOOK || tPage == MP_MENU_PAGE_CREATE)
                DrawBottom(tPlayers, tPage, tCursor);
            if (tPage == MP_MENU_PAGE_LOOK)
                Preview_Draw(LookRowAt(tCursor) == ROW_BACKPIC);
        }
        else if (tPage == MP_MENU_PAGE_CREATE)
            HandleCreateInput(taskId);
        else if (tPage == MP_MENU_PAGE_FRIENDS)
            HandleFriendsInput(taskId);
        else if (tPage == MP_MENU_PAGE_LOOK)
            HandleLookInput(taskId);
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
    }
}

// The new game's intro (mp_intro.c): just LOOK (and CREATE), then onDone
void MP_OpenLookForIntro(void (*onDone)(void))
{
    sIntroDone = onDone;
    MP_OpenMenuPage(MP_MENU_PAGE_LOOK);
}

void MP_OpenMenuPageAt(u8 page, u8 cursor)
{
    u8 taskId;
    MP_OpenMenuPage(page);
    if (page == MP_MENU_PAGE_EMOTES)
        return;
    taskId = FindTaskIdByFunc(Task_CoopMenu);
    if (taskId != TASK_NONE && cursor < RowCount(page))
        gTasks[taskId].tCursor = cursor;
}

void MP_OpenMenuPage(u8 page)
{
    u8 taskId;
    if (page == MP_MENU_PAGE_EMOTES)
    {
        // Emotes and phrases live in the quick pick now
        MP_OpenQuickPick(MP_QUICK_PAGE_EMOTES, -1);
        return;
    }
    LockPlayerFieldControls();
    HideMapNamePopUpWindow(); // it shares window tiles with this menu
    MP_Avatar_RefreshPackList();   // pick up sheets saved since the last time
    taskId = CreateTask(Task_CoopMenu, 80);
    gTasks[taskId].tPage = page;
    if (page == MP_MENU_PAGE_MAIN)
        gTasks[taskId].tCursor = MP_Lobby_InRoom() ? MAIN_EMOTES : MAIN_ONLINE;
    else if (page == MP_MENU_PAGE_LOOK)
        gTasks[taskId].tCursor = LookCursorOf(ROW_SPRITE);
}

// Back from the BERRY GAMES page
void MP_OpenMenuAtUnion(void)
{
    MP_OpenMenuPageAt(MP_MENU_PAGE_MAIN, MAIN_BERRY);
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

// ---------------------------------------------------------------------------
// Quick pick: emotes and phrases around the player, one per direction (D-Pad or
// left stick). Pages (LB/RB, L/R keys): EMOTES, MINE (your 32 phrases on up to 8
// pages; an empty slot on the first offers a ready-made phrase) and the
// ready-made phrases in groups of 4. MINE 2-8 show once the page before has one.
//   Held (LT/RT, or Q/E on the keyboard): let go to send the picked one.
//   Open (CO-OP > SOCIAL): A sends, B closes.
// MINE: west button (SELECT) swaps an empty slot's ready-made phrase, north
// (START) edits the phrase. Other phrase pages: west pins the phrase to MINE.

#include "platform/gamepad.h"

#define qPage    data[0]
#define qSel     data[1]   // -1 nothing yet, else 0 up, 1 right, 2 down, 3 left
#define qFade    data[2]   // fading out to the word editor
#define qHold    data[3]   // the trigger/key holding it open (0 LT/Q, 1 RT/E), -1 = stays open
#define qWin(i)  data[4 + (i)]   // 4 directions + the bar

enum { QUICK_UP, QUICK_RIGHT, QUICK_DOWN, QUICK_LEFT, QUICK_BAR, QUICK_WINDOWS };
#define QUICK_MINE_PAGES (MP_NUM_PHRASES / 4)
#define QUICK_STOCK_FIRST (MP_QUICK_PAGE_MINE_LAST + 1)
#define QUICK_PAGES (QUICK_STOCK_FIRST + 4)   // EMOTES 1-4, MINE 1-8, 4 groups of ready-made phrases

static u8 sQuickPhrasePage = MP_QUICK_PAGE_MINE;     // the last phrase page, remembered
static u8 sQuickEmotePage = MP_QUICK_PAGE_EMOTES;    // ... and emote page
static const u8 sQuickArrows[4] = {CHAR_UP_ARROW, CHAR_RIGHT_ARROW, CHAR_DOWN_ARROW, CHAR_LEFT_ARROW};
// Emotes by page and direction (up, right, down, left)
static const u8 sQuickEmotes[MP_EMOTE_PAGES][4] = {
    {MP_EMOTE_EXCLAMATION, MP_EMOTE_HEART, MP_EMOTE_QUESTION, MP_EMOTE_NOTE},
    {MP_EMOTE_ANGRY, MP_EMOTE_SWEAT, MP_EMOTE_SLEEPY, MP_EMOTE_LAUGH},
    {MP_EMOTE_IDEA, MP_EMOTE_SPARKLE, MP_EMOTE_SAD, MP_EMOTE_DOTS},
    {MP_EMOTE_FIRE, MP_EMOTE_CROWN, MP_EMOTE_BALL, MP_EMOTE_SKULL},
};
static const char *const sQuickEmotePageNames[MP_EMOTE_PAGES] = {"EMOTES", "MOODS", "REACT", "HYPE"};
static const char *const sQuickStockNames[4] = {"GREET", "MOVE", "BATTLE", "MORE"};

static bool8 IsEmotePage(u8 page)
{
    return page <= MP_QUICK_PAGE_EMOTES_LAST;
}

static u8 QuickEmote(u8 page, u8 dir)
{
    return sQuickEmotes[page % MP_EMOTE_PAGES][dir & 3];
}

static bool8 IsMinePage(u8 page)
{
    return page >= MP_QUICK_PAGE_MINE && page <= MP_QUICK_PAGE_MINE_LAST;
}

static u8 MineSlot(u8 page, u8 dir)
{
    return (page - MP_QUICK_PAGE_MINE) * 4 + dir;
}

static bool8 MinePageUsed(u8 page)
{
    u8 d;
    for (d = 0; d < 4; d++)
        if (!MP_Emote_PhraseIsEmpty(MineSlot(page, d)))
            return TRUE;
    return FALSE;
}

// MINE 2-8 show up once the page before them has a phrase (or they have one)
static bool8 QuickPageShown(u8 page)
{
    if (!IsMinePage(page) || page == MP_QUICK_PAGE_MINE)
        return TRUE;
    return MinePageUsed(page - 1) || MinePageUsed(page);
}

// ---- For the battle pick (mp_battlechat.c): the same pages, sending only ----
u8 MP_Quick_PageCount(void) { return QUICK_PAGES; }
bool8 MP_Quick_PageShown(u8 page) { return QuickPageShown(page); }
void MP_Quick_SetPhrasePage(u8 page)
{
    if (IsEmotePage(page))
        sQuickEmotePage = page;
    else
        sQuickPhrasePage = page;
}

// The page to open: the last phrase page for MP_QUICK_PAGE_PHRASES, the last emote page for EMOTES
static u8 QuickOpenPage(u8 page)
{
    if (page == MP_QUICK_PAGE_PHRASES)
        return sQuickPhrasePage;
    if (page == MP_QUICK_PAGE_EMOTES)
        return sQuickEmotePage;
    return page % QUICK_PAGES;
}

u8 MP_Quick_StartPage(u8 page)
{
    u8 p = QuickOpenPage(page);
    return QuickPageShown(p) ? p : MP_QUICK_PAGE_MINE;
}

void MP_Quick_PageName(u8 page, char *out, int size)
{
    if (IsEmotePage(page))
        snprintf(out, size, "%s", sQuickEmotePageNames[page]);
    else if (IsMinePage(page))
        snprintf(out, size, page == MP_QUICK_PAGE_MINE ? "MINE" : "MINE %d", page - MP_QUICK_PAGE_MINE + 1);
    else
        snprintf(out, size, "%s", sQuickStockNames[(page - QUICK_STOCK_FIRST) % 4]);
}

static int QuickStockIndex(u8 page, u8 dir);
static bool8 QuickEmptySlot(u8 page, u8 dir);

// What a direction holds: MP_QUICK_EMOTE (*emoteKey: MP_EMOTE_*; display: its name in out),
// MP_QUICK_TEXT (the game text in out) or MP_QUICK_NONE (an empty phrase slot)
u8 MP_Quick_Choice(u8 page, u8 dir, u8 *emoteKey, u8 *out, int outSize, bool8 display)
{
    int stock;
    if (IsEmotePage(page))
    {
        *emoteKey = QuickEmote(page, dir);
        if (display && outSize > 0)
            MP_AsciiToGame(out, MP_Emote_Name(*emoteKey), outSize - 1);
        return MP_QUICK_EMOTE;
    }
    if (QuickEmptySlot(page, dir))
        return MP_QUICK_NONE;
    if (outSize <= 0)
        return MP_QUICK_TEXT;
    if ((stock = QuickStockIndex(page, dir)) >= 0)
        MP_AsciiToGame(out, MP_Emote_QuickPhrase(stock), outSize - 1);
    else
    {
        u8 text[200];
        MP_Emote_PhraseText(MineSlot(page, dir), text);
        StringCopyN(out, text, outSize - 1);
        out[outSize - 1] = EOS;
    }
    return MP_QUICK_TEXT;
}

// Around the player (the camera keeps them in the middle of the screen), one box a d-pad direction (the user's
// design, 2026-11-13): an emote page's four icons in small boxes, a phrase page's four phrases in speech balloons, the
// A-Life pick's four actions around the POKéMON; the bar along the bottom (pages, buttons). The boxes are windows
// with no frame, see-through but for what's drawn, so the field shows between them.
#define qGroup   data[9]    // which set of windows is up: QG_*
#define qOffX    data[10]   // the A-Life pick: the POKéMON beside the player (tiles of 8 pixels)
#define qOffY    data[11]
enum { QG_NONE, QG_SLOTS, QG_PHRASES };
#define SLOT_W 4   // tiles
#define SLOT_H 3
// Slots around the player: (left, top) in tiles, up / right / down / left
static const u8 sSlotPos[4][2] = {{13, 4}, {18, 8}, {13, 12}, {8, 8}};
static const struct WindowTemplate sPhraseWins[4] = {
    [QUICK_UP]    = {.bg = 0, .tilemapLeft = 2,  .tilemapTop = 5,  .width = 26, .height = 2, .paletteNum = 15, .baseBlock = 0x8},
    [QUICK_RIGHT] = {.bg = 0, .tilemapLeft = 18, .tilemapTop = 9,  .width = 11, .height = 2, .paletteNum = 15, .baseBlock = 0x8 + 52},
    [QUICK_DOWN]  = {.bg = 0, .tilemapLeft = 2,  .tilemapTop = 12, .width = 26, .height = 2, .paletteNum = 15, .baseBlock = 0x8 + 74},
    [QUICK_LEFT]  = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 9,  .width = 12, .height = 2, .paletteNum = 15, .baseBlock = 0x8 + 126},
};
static const struct WindowTemplate sQuickBarWin = {
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 16, .width = 28, .height = 3, .paletteNum = 15, .baseBlock = 0x8 + 176,
};

static void QuickRemoveWindows(s16 *data);

// A box: a 1-pixel outline with its corners cut, filled (Emerald's own small boxes)
static void DrawBox(u8 win, int x, int y, int w, int h, bool8 sel)
{
    // the picked one: a 2-pixel outline in the frame's color (the user, 2026-11-13: "an outline highlight")
    u8 edge = sel ? MPT_GOLD : MPT_TEXT, fill = MPT_BG;   // (GOLD: the frame's color, deep on light windows, light on dark)
    FillWindowPixelRect(win, PIXEL_FILL(edge), x + 1, y, w - 2, h);
    FillWindowPixelRect(win, PIXEL_FILL(edge), x, y + 1, w, h - 2);
    if (sel)
        FillWindowPixelRect(win, PIXEL_FILL(edge), x + 1, y + 1, w - 2, h - 2);
    FillWindowPixelRect(win, PIXEL_FILL(fill), x + 1 + sel, y + 1 + sel, w - 2 - 2 * sel, h - 2 - 2 * sel);
}

// The windows for a group (made again when the group changes)
static void QuickMakeWindows(s16 *data, u8 group)
{
    u8 i;
    if (qGroup == group)
        return;
    QuickRemoveWindows(data);
    MpTheme_Push();
    for (i = 0; i < 4; i++)
    {
        struct WindowTemplate t;
        if (group == QG_SLOTS)
        {
            int left = sSlotPos[i][0] + qOffX, top = sSlotPos[i][1] + qOffY;
            // the box on the player's side of the POKéMON stays beyond the player
            if ((qOffX > 0 && i == QUICK_LEFT) || (qOffX < 0 && i == QUICK_RIGHT) || (qOffY > 0 && i == QUICK_UP) || (qOffY < 0 && i == QUICK_DOWN))
                left = sSlotPos[i][0], top = sSlotPos[i][1];
            left = max(0, min(30 - SLOT_W, left));
            top = max(0, min(15 - SLOT_H, top));
            t = (struct WindowTemplate){.bg = 0, .tilemapLeft = left, .tilemapTop = top, .width = SLOT_W, .height = SLOT_H,
                                        .paletteNum = 15, .baseBlock = 0x8 + i * SLOT_W * SLOT_H};
        }
        else
            t = sPhraseWins[i];
        qWin(i) = AddWindow(&t);
        PutWindowTilemap(qWin(i));
    }
    qWin(QUICK_BAR) = AddWindow(&sQuickBarWin);
    PutWindowTilemap(qWin(QUICK_BAR));
    DrawStdWindowFrame(qWin(QUICK_BAR), FALSE);
    qGroup = group;
}

static void QuickRemoveWindows(s16 *data)
{
    u8 i;
    if (qGroup == QG_NONE)
        return;
    MpUi_PicsClear();
    for (i = 0; i < 4; i++)
    {
        FillWindowPixelBuffer(qWin(i), PIXEL_FILL(0));
        ClearWindowTilemap(qWin(i));
        CopyWindowToVram(qWin(i), COPYWIN_FULL);
        RemoveWindow(qWin(i));
    }
    ClearStdWindowAndFrame(qWin(QUICK_BAR), TRUE);
    RemoveWindow(qWin(QUICK_BAR));
    MpTheme_Pop();
    qGroup = QG_NONE;
}

// An emote slot: its box and the emote's icon in it (picture slot = the direction)
static void DrawSlot(s16 *data, u8 dir, bool8 sel, u8 emote, s8 icon)
{
    u8 win = qWin(dir);
    int x = GetWindowAttribute(win, WINDOW_TILEMAP_LEFT) * 8, y = GetWindowAttribute(win, WINDOW_TILEMAP_TOP) * 8;
    FillWindowPixelBuffer(win, PIXEL_FILL(0));
    if (emote || icon >= 0)
    {
        DrawBox(win, 5, 1, 22, 22, sel);
        if (sel)
        {
            // a small pointer in the box's own margin (the box keeps its size and place): 3 columns, 5 rows
            FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), 0, 8, 1, 7);
            FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), 1, 9, 1, 5);
            FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), 2, 10, 1, 3);
            FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), 3, 11, 1, 1);
        }
        if (icon >= 0)
            MpUi_Icon(dir, icon, x + 8, y + 4);
        else
            MpUi_Emote(dir, emote, x + 8, y + 4);
    }
    CopyWindowToVram(win, COPYWIN_FULL);
}

// A phrase balloon: up / down centered on the player, left ending beside them, right starting beside them
static void DrawBalloon(s16 *data, u8 dir, bool8 sel, const u8 *text, bool8 dim)
{
    u8 win = qWin(dir), line[96];
    int ww = GetWindowAttribute(win, WINDOW_WIDTH) * 8, w, x, len;
    FillWindowPixelBuffer(win, PIXEL_FILL(0));
    StringCopy(line, text);
    for (len = StringLength(line); len > 1 && GetStringWidth(FONT_SMALL, line, 0) > ww - 10; len--)
        line[len - 1] = EOS;
    w = GetStringWidth(FONT_SMALL, line, 0) + 10;
    if (dir == QUICK_LEFT)
        x = ww - w;
    else if (dir == QUICK_RIGHT)
        x = 0;
    else
        x = (120 - GetWindowAttribute(win, WINDOW_TILEMAP_LEFT) * 8) - w / 2;
    DrawBox(win, x, 0, w, 15, sel);
    MpTheme_Print(win, FONT_SMALL, line, x + 5, 0, dim && !sel ? MPT_GREY : MPT_TEXT, 0);
    CopyWindowToVram(win, COPYWIN_FULL);
}

// The bar's first line: L, the page (lit), R, then the group's other pages or its dots; RIGHT at the right end
static void DrawBarPages(u8 win, const char *const *names, u8 count, u8 cur, bool8 asDots, const char *right)
{
    u8 buf[48];
    int x = 2, i, w;
    MP_AsciiToGame(buf, "{L}", 8);
    MpTheme_Print(win, FONT_SMALL, buf, x, 0, MPT_TEXT, 0);
    x += GetStringWidth(FONT_SMALL, buf, 0) + 3;
    if (asDots)
    {
        MP_AsciiToGame(buf, names[cur], 20);
        w = 50;
        MpTheme_Print(win, FONT_SMALL, buf, x + w / 2 - GetStringWidth(FONT_SMALL, buf, 0) / 2, 0, MPT_TEXT, 0);
        x += w + 3;
        MP_AsciiToGame(buf, "{R}", 8);
        MpTheme_Print(win, FONT_SMALL, buf, x, 0, MPT_TEXT, 0);
        x += GetStringWidth(FONT_SMALL, buf, 0) + 6;
        for (i = 0; i < count; i++)
        {
            if (i > 0 && names[i][0] != names[i - 1][0])
                x += 3;   // a gap between the groups (MINE, the ready-made ones)
            FillWindowPixelRect(win, PIXEL_FILL(MPT_TEXT), x, 3, 5, 5);
            if (i != cur)
                FillWindowPixelRect(win, PIXEL_FILL(MPT_BG), x + 1, 4, 3, 3);
            x += 7;
        }
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            MP_AsciiToGame(buf, names[i], 20);
            w = GetStringWidth(FONT_SMALL, buf, 0);
            MpTheme_Print(win, FONT_SMALL, buf, x, 0, i == cur ? MPT_TEXT : MPT_GREY, 0);
            x += w + 6;
        }
        MP_AsciiToGame(buf, "{R}", 8);
        MpTheme_Print(win, FONT_SMALL, buf, x - 2, 0, MPT_TEXT, 0);
    }
    if (right && right[0])
    {
        MP_AsciiToGame(buf, right, 40);
        MpTheme_Print(win, FONT_SMALL, buf, 28 * 8 - 3 - GetStringWidth(FONT_SMALL, buf, 0), 0, MPT_TEXT, 0);
    }
}

static void DrawBarHints(u8 win, const char *ascii)
{
    u8 buf[120];
    MP_AsciiToGame(buf, ascii, 116);
    MpTheme_Print(win, FONT_SMALL, buf, 2, 12, MPT_GREY, 0);
}

// The ready-made phrase behind a direction on a phrase page (-1: your own phrase)
static int QuickStockIndex(u8 page, u8 dir)
{
    if (page == MP_QUICK_PAGE_MINE)
        return MP_Emote_PhraseIsEmpty(dir) ? gMpConfig.quickStock[dir] % MP_Emote_QuickPhraseCount() : -1;
    if (IsMinePage(page))
        return -1;
    return ((page - QUICK_STOCK_FIRST) * 4 + dir) % MP_Emote_QuickPhraseCount();
}

// An empty phrase slot on MINE 2-8 (nothing to send yet)
static bool8 QuickEmptySlot(u8 page, u8 dir)
{
    return IsMinePage(page) && page != MP_QUICK_PAGE_MINE && MP_Emote_PhraseIsEmpty(MineSlot(page, dir));
}

// Up to two lines (left / right boxes) or one
static void QuickChoiceText(u8 page, u8 dir, u8 *line1, u8 *line2, bool8 twoLines, int maxW)
{
    int stock, len, i, n = 0;
    u8 text[160];
    u8 *lines[2] = {line1, line2};
    line1[0] = line2[0] = EOS;
    if (IsEmotePage(page))
    {
        MP_AsciiToGame(line1, MP_Emote_Name(QuickEmote(page, dir)), 20);
        return;
    }
    stock = QuickStockIndex(page, dir);
    if (stock >= 0)
        MP_AsciiToGame(line1, MP_Emote_QuickPhrase(stock), 60);
    else if (QuickEmptySlot(page, dir))
        MP_AsciiToGame(line1, "- empty -", 20);
    else
    {
        // Your phrase: words 1-2 and 3-4
        u8 *dst = line1;
        MP_Emote_PhraseText(MineSlot(page, dir), text);
        for (i = 0; text[i] != EOS && n < 70; i++)
        {
            if (text[i] == CHAR_NEWLINE && twoLines)
            {
                dst[n] = EOS;
                dst = line2;
                n = 0;
                continue;
            }
            dst[n++] = (text[i] == CHAR_NEWLINE) ? CHAR_SPACE : text[i];
            dst[n] = EOS;
        }
    }
    for (i = 0; i < 2; i++)
        for (len = StringLength(lines[i]); len > 1 && GetStringWidth(FONT_SMALL_NARROW, lines[i], 0) > maxW; len--)
            lines[i][len - 1] = EOS;
}

static void QuickDrawChoice(s16 *data, u8 dir)
{
    bool8 sel = (qSel == dir);
    if (IsEmotePage(qPage))
        DrawSlot(data, dir, sel, QuickEmote(qPage, dir), -1);
    else
    {
        u8 l1[96], l2[96];
        bool8 dim = (qPage == MP_QUICK_PAGE_MINE && QuickStockIndex(qPage, dir) >= 0) || QuickEmptySlot(qPage, dir);
        QuickChoiceText(qPage, dir, l1, l2, FALSE, 200);
        DrawBalloon(data, dir, sel, l1, dim);
    }
}

// The bar: the group's pages, the picked one, what the buttons do now
static void QuickDrawBar(s16 *data)
{
    u8 win = qWin(QUICK_BAR);
    char hint[120], send[48], picked[24] = "";
    MpTheme_Panel(win, 0);
    if (IsEmotePage(qPage))
    {
        if (qSel >= 0)
        {
            const char *n = MP_Emote_Name(QuickEmote(qPage, qSel));
            while (*n && (n[0] == '!' || n[0] == '?' || n[0] == ' '))   // "!  SURPRISED" -> SURPRISED
                n++;
            snprintf(picked, sizeof(picked), "%s", n);
        }
        DrawBarPages(win, sQuickEmotePageNames, MP_EMOTE_PAGES, qPage, FALSE, picked);
    }
    else
    {
        static const char *names[QUICK_PAGES - MP_QUICK_PAGE_MINE];
        static char mineNames[QUICK_MINE_PAGES][8];
        u8 i, count = 0, cur = 0;
        for (i = MP_QUICK_PAGE_MINE; i < QUICK_PAGES; i++)
        {
            if (!QuickPageShown(i))
                continue;
            if (IsMinePage(i))
            {
                snprintf(mineNames[i - MP_QUICK_PAGE_MINE], 8, "MINE %d", i - MP_QUICK_PAGE_MINE + 1);
                names[count] = mineNames[i - MP_QUICK_PAGE_MINE];
            }
            else
                names[count] = sQuickStockNames[(i - QUICK_STOCK_FIRST) % 4];
            if (i == qPage)
                cur = count;
            count++;
        }
        DrawBarPages(win, names, count, cur, TRUE, NULL);
    }
    if (qHold >= 0)
        snprintf(send, sizeof(send), "{DPAD} pick  let go: %s", IsEmotePage(qPage) ? "send" : "say");
    else
        snprintf(send, sizeof(send), "{DPAD} pick  {A} %s  {B} close", IsEmotePage(qPage) ? "send" : "say");
    if (qPage == MP_QUICK_PAGE_MINE)
        snprintf(hint, sizeof(hint), "%s  {WEST} swap  {NORTH} edit", send);
    else if (IsMinePage(qPage))
        snprintf(hint, sizeof(hint), "%s  {NORTH} edit", send);
    else if (!IsEmotePage(qPage))
        snprintf(hint, sizeof(hint), "%s  {WEST} pin to MINE", send);
    else
        snprintf(hint, sizeof(hint), "%s", send);
    DrawBarHints(win, hint);
    CopyWindowToVram(win, COPYWIN_FULL);
}

static void QuickDrawAll(s16 *data)
{
    u8 i;
    QuickMakeWindows(data, IsEmotePage(qPage) ? QG_SLOTS : QG_PHRASES);
    MpUi_PicBegin();
    for (i = 0; i < 4; i++)
        QuickDrawChoice(data, i);
    MpUi_PicEnd();
    QuickDrawBar(data);
}

static void QuickPickClose(u8 taskId, bool8 send)
{
    s16 *data = gTasks[taskId].data;
    u8 page = qPage;
    s16 sel = qSel;
    int stock;
    QuickRemoveWindows(data);
    UnlockPlayerFieldControls();
    DestroyTask(taskId);
    if (!send || sel < 0)
        return;
    // After closing, so you see your own bubble
    MP_Achieve_Count("emote", 1);
    if (IsEmotePage(page))
        MP_Emote_Send(QuickEmote(page, sel));
    else if ((stock = QuickStockIndex(page, sel)) >= 0)
        MP_Emote_SayQuick(stock);
    else
        MP_Emote_SayPhrase(MineSlot(page, sel));
}

static void Task_QuickPick(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s16 sel = qSel;

    if (qFade)
    {
        // To the word editor for the picked phrase
        if (!gPaletteFade.active)
        {
            u8 slot = MineSlot(qPage, qSel);
            DestroyTask(taskId);
            MP_Emote_EditPhraseFromField(slot);
        }
        return;
    }
    if (JOY_HELD(DPAD_UP))
        sel = QUICK_UP;
    else if (JOY_HELD(DPAD_RIGHT))
        sel = QUICK_RIGHT;
    else if (JOY_HELD(DPAD_DOWN))
        sel = QUICK_DOWN;
    else if (JOY_HELD(DPAD_LEFT))
        sel = QUICK_LEFT;
    if (sel != qSel)
    {
        s16 old = qSel;
        qSel = sel;
        PlaySE(SE_SELECT);
        if (old >= 0)
            QuickDrawChoice(data, old);
        QuickDrawChoice(data, sel);
        QuickDrawBar(data);
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        QuickPickClose(taskId, FALSE);
    }
    else if (JOY_NEW(A_BUTTON) || (qHold >= 0 && !Gamepad_TriggerHeld(qHold)))
    {
        if ((qHold < 0 && qSel < 0) || (qHold < 0 && QuickEmptySlot(qPage, qSel)))
            PlaySE(SE_FAILURE);   // open: pick a direction (with something to say) first
        else
            QuickPickClose(taskId, TRUE);
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        // Held (LT emotes / RT phrases): L / R go round that group's pages; opened from SOCIAL: round them all
        bool8 emotes = IsEmotePage(qPage);
        do
            qPage = (qPage + (JOY_NEW(R_BUTTON) ? 1 : QUICK_PAGES - 1)) % QUICK_PAGES;
        while (!QuickPageShown(qPage) || (qHold >= 0 && IsEmotePage(qPage) != emotes));
        MP_Quick_SetPhrasePage(qPage);
        PlaySE(SE_SELECT);
        QuickDrawAll(data);
    }
    else if (!IsEmotePage(qPage) && JOY_NEW(SELECT_BUTTON))
    {
        // MINE: the next ready-made phrase for an empty slot; other pages: put this one on MINE
        if (qSel < 0 || (qPage == MP_QUICK_PAGE_MINE && !MP_Emote_PhraseIsEmpty(qSel))
         || (IsMinePage(qPage) && qPage != MP_QUICK_PAGE_MINE))
            PlaySE(SE_FAILURE);
        else
        {
            if (qPage == MP_QUICK_PAGE_MINE)
                gMpConfig.quickStock[qSel] = (gMpConfig.quickStock[qSel] + 1) % MP_Emote_QuickPhraseCount();
            else
                gMpConfig.quickStock[qSel] = QuickStockIndex(qPage, qSel);
            MP_SaveConfig();
            PlaySE(SE_BALL_TRAY_ENTER);
            QuickDrawChoice(data, qSel);
            if (qPage != MP_QUICK_PAGE_MINE && !MP_Emote_PhraseIsEmpty(qSel))
                MP_ShowNotice("Pinned. It shows on MINE while that phrase slot is empty.");
        }
    }
    else if (IsMinePage(qPage) && JOY_NEW(START_BUTTON))
    {
        if (qSel < 0)
            PlaySE(SE_FAILURE);
        else
        {
            PlaySE(SE_SELECT);
            QuickRemoveWindows(data);
            FadeScreen(FADE_TO_BLACK, 0);
            qFade = TRUE;
        }
    }
}

// page: MP_QUICK_PAGE_* (MP_QUICK_PAGE_PHRASES = the last phrase page); hold: 0 LT/Q, 1 RT/E, -1 stays open
void MP_OpenQuickPickAt(u8 page, s8 hold, s8 sel)
{
    u8 taskId, i;
    s16 *data;
    if (FindTaskIdByFunc(Task_QuickPick) != TASK_NONE || FindTaskIdByFunc(Task_CoopMenu) != TASK_NONE)
        return;
    LockPlayerFieldControls();
    HideMapNamePopUpWindow();   // it shares window tiles with these
    LoadMessageBoxAndBorderGfx();
    MpTheme_Push();
    taskId = CreateTask(Task_QuickPick, 80);
    data = gTasks[taskId].data;
    qPage = QuickOpenPage(page);
    if (!QuickPageShown(qPage))
        qPage = MP_QUICK_PAGE_MINE;
    qSel = sel;
    qFade = FALSE;
    qHold = hold;
    qGroup = QG_NONE;
    qOffX = qOffY = 0;
    (void)i;
    QuickDrawAll(data);
    PlaySE(SE_WIN_OPEN);
}

bool8 MP_QuickPickOpen(void)
{
    return FindTaskIdByFunc(Task_QuickPick) != TASK_NONE;
}

// ---------------------------------------------------------------------------
// MOéMON GARDEN: the verb wheel (hold A in front of your follower). The same boxes around you as the
// quick pick; L / R switch pages. Held: let go of A to do the picked one (nothing picked: the wheel
// stays open, A does it, B closes).

enum { VERB_PAGE_TOGETHER, VERB_PAGE_HEART, VERB_PAGE_QUIET, VERB_PAGES };
#define VERB_SIT_WITH LIFE_VERBS   // not a LIFE_VERB_* of its own (mp_life_interact.c)
static const s8 sVerbs[VERB_PAGES][4] = {
    [VERB_PAGE_TOGETHER] = {LIFE_VERB_PET, LIFE_VERB_PLAY, LIFE_VERB_FEED, LIFE_VERB_TALK},
    [VERB_PAGE_HEART]    = {LIFE_VERB_PRAISE, LIFE_VERB_COMFORT, LIFE_VERB_GROOM, VERB_SIT_WITH},
    [VERB_PAGE_QUIET]    = {LIFE_VERB_WATCH, LIFE_VERB_KEEPSAKE, -1, -1},
};
static const char *const sVerbNames[LIFE_VERBS + 1] = {
    [LIFE_VERB_TALK] = "TALK", [LIFE_VERB_PET] = "PET", [LIFE_VERB_FEED] = "FEED A BERRY",
    [LIFE_VERB_PLAY] = "PLAY", [LIFE_VERB_PRAISE] = "PRAISE", [LIFE_VERB_COMFORT] = "COMFORT",
    [LIFE_VERB_WATCH] = "WATCH QUIETLY", [LIFE_VERB_GROOM] = "GROOM", [LIFE_VERB_KEEPSAKE] = "GIVE A KEEPSAKE",
    [LIFE_VERB_PICKUP] = "", [VERB_SIT_WITH] = "SIT WITH",
};
static const char *const sVerbPageNames[VERB_PAGES] = {"TOGETHER", "HEART", "QUIET"};
static void (*sVerbDone)(s8 verb);
static u8 sVerbPage;   // remembered between openings

// Each action's picture: an item's (as the hub's icons) or an emote's
static void VerbPicture(s8 verb, u8 *emote, s8 *icon)
{
    *emote = 0, *icon = -1;
    switch (verb)
    {
    case LIFE_VERB_PET:      *icon = MP_COOPICON_PET; break;
    case LIFE_VERB_GROOM:    *icon = MP_COOPICON_GROOM; break;
    case LIFE_VERB_PLAY:     *icon = MP_COOPICON_PLAY; break;
    case LIFE_VERB_FEED:     *icon = MP_COOPICON_FEED; break;
    case LIFE_VERB_KEEPSAKE: *icon = MP_COOPICON_KEEPSAKE; break;
    case LIFE_VERB_TALK:     *emote = MP_EMOTE_NOTE; break;
    case LIFE_VERB_PRAISE:   *emote = MP_EMOTE_SPARKLE; break;
    case LIFE_VERB_COMFORT:  *emote = MP_EMOTE_HEART; break;
    case LIFE_VERB_WATCH:    *emote = MP_EMOTE_IDEA; break;
    case VERB_SIT_WITH:      *emote = MP_EMOTE_DOTS; break;
    }
}

static void VerbDrawChoice(s16 *data, u8 dir)
{
    s8 verb = sVerbs[qPage][dir], icon;
    u8 emote;
    if (verb < 0)
    {
        DrawSlot(data, dir, FALSE, 0, -1);
        return;
    }
    VerbPicture(verb, &emote, &icon);
    DrawSlot(data, dir, qSel == dir, emote, icon);
}

static void VerbDrawBar(s16 *data)
{
    u8 win = qWin(QUICK_BAR);
    s8 verb = qSel >= 0 ? sVerbs[qPage][qSel] : -1;
    MpTheme_Panel(win, 0);
    DrawBarPages(win, sVerbPageNames, VERB_PAGES, qPage, FALSE, verb >= 0 ? sVerbNames[verb] : NULL);
    DrawBarHints(win, qHold >= 0 ? "{DPAD} pick  let go of {A}: do it" : "{DPAD} pick  {A} do it  {B} close");
    CopyWindowToVram(win, COPYWIN_FULL);
}

static void VerbDrawAll(s16 *data)
{
    u8 i;
    QuickMakeWindows(data, QG_SLOTS);
    MpUi_PicBegin();
    for (i = 0; i < 4; i++)
        VerbDrawChoice(data, i);
    MpUi_PicEnd();
    VerbDrawBar(data);
}

static void VerbClose(u8 taskId, s8 verb)
{
    s16 *data = gTasks[taskId].data;
    void (*done)(s8) = sVerbDone;
    QuickRemoveWindows(data);
    DestroyTask(taskId);
    sVerbDone = NULL;
    if (done)
        done(verb);
}

static void Task_VerbWheel(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    s16 sel = qSel;
    if (JOY_HELD(DPAD_UP))
        sel = QUICK_UP;
    else if (JOY_HELD(DPAD_RIGHT))
        sel = QUICK_RIGHT;
    else if (JOY_HELD(DPAD_DOWN))
        sel = QUICK_DOWN;
    else if (JOY_HELD(DPAD_LEFT))
        sel = QUICK_LEFT;
    if (sel != qSel)
    {
        s16 old = qSel;
        qSel = sel;
        PlaySE(SE_SELECT);
        if (old >= 0)
            VerbDrawChoice(data, old);
        VerbDrawChoice(data, sel);
        VerbDrawBar(data);
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        VerbClose(taskId, -1);
    }
    else if ((qHold >= 0 && !JOY_HELD(A_BUTTON)) || (qHold < 0 && JOY_NEW(A_BUTTON)))
    {
        s8 verb = qSel >= 0 ? sVerbs[qPage][qSel] : -1;
        if (verb >= 0)
        {
            PlaySE(SE_SELECT);
            VerbClose(taskId, verb);
        }
        else if (qHold >= 0)
        {
            qHold = -1;   // let go without picking: the wheel stays open
            VerbDrawBar(data);
        }
        else
        {
            PlaySE(SE_FAILURE);
        }
    }
    else if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        qPage = (qPage + (JOY_NEW(R_BUTTON) ? 1 : VERB_PAGES - 1)) % VERB_PAGES;
        sVerbPage = qPage;
        PlaySE(SE_SELECT);
        VerbDrawAll(data);
    }
}

void MP_OpenVerbWheel(void (*done)(s8 verb))
{
    u8 taskId, i;
    s16 *data;
    if (FindTaskIdByFunc(Task_VerbWheel) != TASK_NONE)
        return;
    HideMapNamePopUpWindow();
    LoadMessageBoxAndBorderGfx();
    MpTheme_Push();
    taskId = CreateTask(Task_VerbWheel, 80);
    data = gTasks[taskId].data;
    qPage = sVerbPage % VERB_PAGES;
    qSel = -1;
    qFade = FALSE;
    qHold = 0;   // opened by holding A
    sVerbDone = done;
    qGroup = QG_NONE;
    qOffX = qOffY = 0;
    {
        // the boxes go round the POKéMON (beside the player: two tiles of 8 pixels a step)
        s16 fx, fy;
        u8 dir;
        struct ObjectEvent *me = &gObjectEvents[gPlayerAvatar.objectEventId];
        if (MP_Follower_LocalPos(&fx, &fy, &dir))
        {
            int dx = fx - me->currentCoords.x, dy = fy - me->currentCoords.y;
            if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1)
                qOffX = dx * 2, qOffY = dy * 2;
        }
    }
    (void)i;
    VerbDrawAll(data);
    PlaySE(SE_WIN_OPEN);
    MPNet_Log("life interact: wheel open");
}

bool8 MP_VerbWheelOpen(void)
{
    return FindTaskIdByFunc(Task_VerbWheel) != TASK_NONE;
}

void MP_OpenQuickPick(u8 page, s8 hold)
{
    MP_OpenQuickPickAt(page, hold, -1);
}

// The CO-OP menu is open (others see "IN MENU" over this player)
bool8 MP_Menu_IsCoopMenuOpen(void)
{
    return FuncIsActiveTask(Task_CoopMenu);
}
