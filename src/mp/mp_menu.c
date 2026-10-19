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

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);

// Pages of the CO-OP menu. Rows are per page; each page ends with BACK/CLOSE.
// No CLOSE row: B or START closes the menu
enum { MAIN_EMOTES, MAIN_FRIENDS, MAIN_TRADE, MAIN_SHARED_PC, MAIN_GATCHA, MAIN_DAILY, MAIN_TOURNEY, MAIN_BERRY, MAIN_REMINDER, MAIN_LOOK, MAIN_ONLINE, MAIN_COUNT, MAIN_CLOSE = 0xFF };
enum { ROW_SPRITE, ROW_FRONT, ROW_BACKPIC, ROW_SKIN, ROW_HAIR, ROW_OUTFIT, ROW_ACCENT, ROW_CREATE, ROW_EDIT, ROW_SAVE, ROW_REVERT, LOOK_COUNT };
// LOOK > CREATE: a trainer from the parts kit (CUSTOM 1-3)
enum { CR_SLOT, CR_BODY, CR_SKIN, CR_HAIR, CR_FHAIR, CR_TOP, CR_BOTTOM, CR_HAT, CR_BAG, CR_SAVE, CR_CANCEL, CREATE_COUNT };
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
    case MP_MENU_PAGE_CREATE: return CREATE_COUNT;
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

// LOOK > CREATE state: the slot being made and its parts
static u8 sCreateSlot;                    // 0-2
static struct MpCreated sCreateWork;
static struct MpAppearance sCreateBefore; // the look when CREATE was opened (CANCEL)
static bool8 sCreateShowBack;

static const char *Create_PartName(u8 cat, bool8 withColor)
{
    static char buf[64];
    const struct MpCreated *r = &sCreateWork;
    u8 style, color;
    bool8 optional = (cat == MP_CR_HAT || cat == MP_CR_BAG);
    switch (cat)
    {
    case MP_CR_HAIR:   style = r->hair; color = r->hairColor; break;
    case MP_CR_TOP:    style = r->top; color = r->topColor; break;
    case MP_CR_BOTTOM: style = r->bottom; color = r->bottomColor; break;
    case MP_CR_HAT:    style = r->hat; color = r->hatColor; break;
    default:           style = r->bag; color = r->bagColor; break;
    }
    if (optional)
    {
        if (style == 0)
            return "NONE";
        style--;
    }
    if (withColor && MP_Created_ColorCount(r->girl, cat, style) > 1)
        snprintf(buf, sizeof(buf), "%s, %s", MP_Created_StyleName(r->girl, cat, style), MP_Created_ColorName(r->girl, cat, style, color));
    else
        snprintf(buf, sizeof(buf), "%s", MP_Created_StyleName(r->girl, cat, style));
    return buf;
}

static const u8 sCreateRowCat[CREATE_COUNT] = {
    [CR_HAIR] = MP_CR_HAIR, [CR_TOP] = MP_CR_TOP, [CR_BOTTOM] = MP_CR_BOTTOM, [CR_HAT] = MP_CR_HAT, [CR_BAG] = MP_CR_BAG,
};

static void Create_PrintRow(u8 windowId, u8 row, u8 y)
{
    static const char *const labels[CREATE_COUNT] = {"SLOT", "BODY", "SKIN", "HAIR", "FIELD", "TOP", "BOTTOM", "HAT", "BAG", "SAVE", "CANCEL"};
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
    default: snprintf(value, sizeof(value), "%s", Create_PartName(sCreateRowCat[row], FALSE)); break;
    }
    PrintChoice(windowId, labels[row], value, y, 38);
}

static void DrawMenu(u8 windowId, u8 page, u8 cursor, u8 choice)
{
    static const char *const mainLabels[] = {"SOCIAL", "FRIENDS", "TRADE", "SHARED PC", "MOEGATCHA", "DAILY", "TOURNAMENT", "BERRY GAMES", "MOVE TUTOR", "LOOK", ""};
    static const char *const lookLabels[] = {"SPRITE", "ID PIC", "BK PIC", "SKIN", "HAIR", "OUTFIT", "ACCENT", "CREATE...", "EDIT SPRITES...", "SAVE LOOK", "REVERT"};
    char line[64];
    u8 row;

    MpTheme_Panel(windowId, 12);
    if (page == MP_MENU_PAGE_LOOK)
        snprintf(line, sizeof(line), "LOOK");
    else if (page == MP_MENU_PAGE_CREATE)
        snprintf(line, sizeof(line), "CREATE");
    else if (page == MP_MENU_PAGE_EMOTES)
        snprintf(line, sizeof(line), "EMOTES");
    else if (page == MP_MENU_PAGE_FRIENDS)
        snprintf(line, sizeof(line), "FRIENDS");
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
        else if (page == MP_MENU_PAGE_CREATE)
        {
            Create_PrintRow(windowId, row, y);
        }
        else if (row == ROW_CREATE || row == ROW_EDIT || row == ROW_SAVE || row == ROW_REVERT)
        {
            PrintAscii(windowId, lookLabels[row], 8, y);
        }
        else
        {
            switch (row)
            {
            case ROW_SPRITE: snprintf(line, sizeof(line), "%s", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app))); break;
            case ROW_FRONT:  snprintf(line, sizeof(line), "%s", MP_Avatar_FrontSelName(gMpConfig.app.frontSel)); break;
            case ROW_BACKPIC: snprintf(line, sizeof(line), "%s", MP_Avatar_BackSelName(gMpConfig.app.backSel)); break;
            case ROW_SKIN:   snprintf(line, sizeof(line), "%s", MP_Avatar_SkinName(*RowValue(row))); break;
            case ROW_HAIR:   snprintf(line, sizeof(line), "%s", MP_Avatar_HairName(*RowValue(row))); break;
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
            MP_AsciiToGame(buf, "Back to the CO-OP menu.", 70);
        else
        {
            (void)town;
            MP_AsciiToGame(buf, "A: GO TO, TRAINER CARD, SECRET BASE", 70);
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
        if (cursor == ROW_SPRITE && gMpConfig.app.base == MP_BASE_CREATED && !gMpConfig.app.recipe.made)
            snprintf(ascii, sizeof(ascii), "%s: not made yet (looks like DEFAULT)", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app)));
        else if (cursor == ROW_SPRITE)
            snprintf(ascii, sizeof(ascii), "%s   START: match pics   SELECT: undo", MP_Avatar_ChoiceName(MP_Avatar_GetChoice(&gMpConfig.app)));
        else if (cursor == ROW_FRONT && MP_Avatar_FrontSelArtist(gMpConfig.app.frontSel)[0])
            snprintf(ascii, sizeof(ascii), "%s  Art by %s, shown as drawn", MP_Avatar_FrontSelName(gMpConfig.app.frontSel), MP_Avatar_FrontSelArtist(gMpConfig.app.frontSel));
        else if (cursor == ROW_FRONT)
            snprintf(ascii, sizeof(ascii), "%s   START: match   SELECT: undo", gMpConfig.app.frontSel ? MP_Avatar_FrontSelName(gMpConfig.app.frontSel) : "MATCH SPRITE");
        else if (cursor == ROW_BACKPIC)
            snprintf(ascii, sizeof(ascii), "%s   START: match   SELECT: undo", gMpConfig.app.backSel ? MP_Avatar_BackSelName(gMpConfig.app.backSel) : "MATCH SPRITE");
        else if (cursor == ROW_HAIR)
            snprintf(ascii, sizeof(ascii), "HAIR (a hat if it hides the hair)   SELECT: undo");
        else if (cursor <= ROW_ACCENT)
            snprintf(ascii, sizeof(ascii), "Left/Right: change   SELECT: undo");
        else if (cursor == ROW_CREATE)
            snprintf(ascii, sizeof(ascii), "Make your own trainer (CUSTOM 1-3).");
        else if (cursor == ROW_EDIT)
            snprintf(ascii, sizeof(ascii), "Draw your own look in the Sprite Editor.");
        else if (cursor == ROW_SAVE)
            snprintf(ascii, sizeof(ascii), "Keep this look and go back. (B does too.)");
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
            snprintf(ascii, sizeof(ascii), "CUSTOM %d%s   Left/Right: slot   START: flip pic", sCreateSlot + 1,
                gMpConfig.custom[sCreateSlot].made ? "" : " (new)");
            break;
        case CR_BODY:
            snprintf(ascii, sizeof(ascii), "%s   Left/Right: change (resets the parts)", sCreateWork.girl ? "GIRL" : "BOY");
            break;
        case CR_SKIN:
            snprintf(ascii, sizeof(ascii), "%s   Left/Right: change   SELECT: undo", MP_Created_SkinName(sCreateWork.skin));
            break;
        case CR_FHAIR:
            snprintf(ascii, sizeof(ascii), "FIELD HAIR: %s%s   L/R: jump 5", MP_Created_FieldHairName(sCreateWork.fieldHair),
                sCreateWork.fieldHair ? "" : " (like HAIR)");
            break;
        case CR_SAVE:
            snprintf(ascii, sizeof(ascii), "Save as CUSTOM %d and wear it.", sCreateSlot + 1);
            break;
        case CR_CANCEL:
            snprintf(ascii, sizeof(ascii), "Back to LOOK without saving. (B does too.)");
            break;
        default:
            if (cursor == CR_BAG)
                snprintf(ascii, sizeof(ascii), "%s  (only on the field sprite)", Create_PartName(MP_CR_BAG, TRUE));
            else
                snprintf(ascii, sizeof(ascii), "%s   Left/Right: style  L/R: color", Create_PartName(sCreateRowCat[cursor], TRUE));
            break;
        }
        MP_AsciiToGame(buf, ascii, 150);
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
    MpTheme_Panel(sPreviewWin, 12);
    sTxtFg = MPT_GOLD;
    sTxtBg = MPT_BAND;
    MP_AsciiToGame(buf, back ? "BACK PIC" : "ID PIC", 30);
    MText(sPreviewWin, FONT_SMALL, buf, 4, 0);
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

static void CloseMenuWindows(s16 *data)
{
    Preview_Hide();
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

static void Look_Snapshot(void);
static void Create_Open(void);

static void GoToPage(s16 *data, u8 page, u8 cursor)
{
    u8 from = tPage;
    tPage = page;
    tCursor = cursor;
    tChoice = 0;
    if (page == MP_MENU_PAGE_LOOK)
    {
        if (from != MP_MENU_PAGE_CREATE)
            Look_Snapshot();
        Preview_Show(cursor == ROW_BACKPIC);
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
    if (all || row == ROW_SPRITE)
        MP_Avatar_RestoreSprite(&sLookBefore, sLookBeforePack);
    if (all || row == ROW_FRONT)   app->frontSel = sLookBefore.frontSel;
    if (all || row == ROW_BACKPIC) app->backSel = sLookBefore.backSel;
    if (all || row == ROW_SKIN)    app->skin = sLookBefore.skin;
    if (all || row == ROW_HAIR)    app->hair = sLookBefore.hair;
    if (all || row == ROW_OUTFIT)  app->outfit = sLookBefore.outfit;
    if (all || row == ROW_ACCENT)  app->accent = sLookBefore.accent;
    return memcmp(&before, app, sizeof(before)) != 0;
}

static void HandleLookInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    bool8 changed = FALSE;

    if (tCursor == ROW_CREATE && JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        Create_Open();
        GoToPage(data, MP_MENU_PAGE_CREATE, CR_SLOT);
        return;
    }
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
    else if (tCursor == ROW_FRONT && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
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
    else if ((tCursor == ROW_BACKPIC || (tCursor >= ROW_SKIN && tCursor <= ROW_ACCENT)) && JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
        u8 *v = RowValue(tCursor);
        u8 n = NumChoices(tCursor);
        *v = JOY_REPEAT(DPAD_RIGHT) ? (*v + 1) % n : (*v + n - 1) % n;
        changed = TRUE;
    }
    else if ((JOY_NEW(SELECT_BUTTON) && tCursor <= ROW_ACCENT) || (JOY_NEW(A_BUTTON) && tCursor == ROW_REVERT))
    {
        // Undo this row (SELECT) or everything (REVERT)
        if (Look_Undo(tCursor))
            changed = TRUE;
        else
            PlaySE(SE_FAILURE);
    }
    else if (JOY_NEW(START_BUTTON))
    {
        // ID PIC / BK PIC back to the ones that go with the sprite (NONE if it has none).
        // On those rows just that one, anywhere else both.
        struct MpAppearance before = gMpConfig.app;
        MP_Avatar_MatchPictures(&gMpConfig.app, tCursor != ROW_BACKPIC, tCursor != ROW_FRONT);
        if (memcmp(&before, &gMpConfig.app, sizeof(before)) != 0)
            changed = TRUE;
        else
            PlaySE(SE_FAILURE);
    }
    else if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == ROW_SAVE))
    {
        PlaySE(SE_SAVE);
        MP_SaveConfig();
        Look_Snapshot();
        GoToPage(data, MP_MENU_PAGE_MAIN, MAIN_LOOK);
    }
    if (changed)
    {
        PlaySE(SE_SELECT);
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        DrawMenu(tWindow, tPage, tCursor, tChoice);
        DrawBottom(tPlayers, tPage, tCursor);
        Preview_Draw(tCursor == ROW_BACKPIC);
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

// Steps a part's style (Left/Right) or color (L/R)
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
            *col = (*col + n + dir) % n;
        return;
    }
    n = MP_Created_StyleCount(r->girl, cat) + (optional ? 1 : 0);
    {
        // Keep the color when the new style has it (same name)
        const char *was = MP_Created_ColorName(r->girl, cat, optional ? *style - 1 : *style, *col);
        u8 i, nc;
        *style = (*style + n + dir) % n;
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

    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && tCursor == CR_CANCEL))
    {
        // Back to the look you had
        PlaySE(SE_SELECT);
        gMpConfig.app = sCreateBefore;
        MP_Avatar_ApplyLocal();
        MP_SendAppearance();
        GoToPage(data, MP_MENU_PAGE_LOOK, ROW_CREATE);
        return;
    }
    if (JOY_NEW(A_BUTTON) && tCursor == CR_SAVE)
    {
        PlaySE(SE_SAVE);
        sCreateWork.made = 1;
        gMpConfig.custom[sCreateSlot] = sCreateWork;
        Create_Wear();
        MP_SaveConfig();
        GoToPage(data, MP_MENU_PAGE_LOOK, ROW_SPRITE);
        return;
    }
    if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        sCreateShowBack = !sCreateShowBack;
        Preview_Draw(sCreateShowBack);
        return;
    }
    if (JOY_NEW(SELECT_BUTTON) && tCursor >= CR_SKIN && tCursor <= CR_BAG)
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
        case CR_FHAIR:  sCreateWork.fieldHair = orig.fieldHair; break;
        case CR_TOP:    sCreateWork.top = orig.top; sCreateWork.topColor = orig.topColor; break;
        case CR_BOTTOM: sCreateWork.bottom = orig.bottom; sCreateWork.bottomColor = orig.bottomColor; break;
        case CR_HAT:    sCreateWork.hat = orig.hat; sCreateWork.hatColor = orig.hatColor; break;
        case CR_BAG:    sCreateWork.bag = orig.bag; sCreateWork.bagColor = orig.bagColor; break;
        }
        if (memcmp(&before, &sCreateWork, sizeof(before)) == 0)
            PlaySE(SE_FAILURE);
    }
    else if (JOY_REPEAT(DPAD_LEFT | DPAD_RIGHT))
    {
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
            sCreateWork.fieldHair = (sCreateWork.fieldHair + n + dir) % n;
            break;
        }
        case CR_HAIR: case CR_TOP: case CR_BOTTOM: case CR_HAT: case CR_BAG:
            Create_StepPart(sCreateRowCat[tCursor], FALSE, dir);
            break;
        }
    }
    else if (JOY_REPEAT(L_BUTTON | R_BUTTON) && tCursor == CR_FHAIR)
    {
        u8 n = MP_Created_FieldHairCount();
        sCreateWork.fieldHair = (sCreateWork.fieldHair + n + dir * 5) % n;
    }
    else if (JOY_REPEAT(L_BUTTON | R_BUTTON) && tCursor >= CR_HAIR && tCursor <= CR_BAG)
    {
        Create_StepPart(sCreateRowCat[tCursor], TRUE, dir);
        if (memcmp(&before, &sCreateWork, sizeof(before)) == 0)
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
        u8 who = ids[tCursor];
        CloseMenu(taskId);
        MP_Linkx_OpenFriend(who);
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
    case MAIN_BERRY:
        CloseMenu(taskId);
        MP_Linkx_OpenUnion();
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
        if (tPage == MP_MENU_PAGE_LOOK)
        {
            Look_Snapshot();
            Preview_Show(tCursor == ROW_BACKPIC);
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
        if (JOY_NEW(DPAD_UP) || JOY_NEW(DPAD_DOWN))
        {
            u8 n = RowCount(tPage);
            tCursor = JOY_NEW(DPAD_UP) ? (tCursor == 0 ? n - 1 : tCursor - 1) : (tCursor + 1) % n;
            tChoice = 0;
            PlaySE(SE_SELECT);
            DrawMenu(tWindow, tPage, tCursor, tChoice);
            if (tPage == MP_MENU_PAGE_FRIENDS || tPage == MP_MENU_PAGE_LOOK || tPage == MP_MENU_PAGE_CREATE)
                DrawBottom(tPlayers, tPage, tCursor);
            if (tPage == MP_MENU_PAGE_LOOK)
                Preview_Draw(tCursor == ROW_BACKPIC);
        }
        else if (tPage == MP_MENU_PAGE_CREATE)
            HandleCreateInput(taskId);
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
