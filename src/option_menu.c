#include "mp_version.h"
// OPTIONS (START > OPTIONS, and the title screen's OPTION): the game's own options screen, with tabs.
// Moemon Co-op: one place for every setting. GAME (the original options, type icons, name case, creatures), DISPLAY
// (fullscreen, window scale, scaling, filter, vsync), CONTROLS (controller, buttons, stick run, rumble) and CO-OP
// (friends' screens with a page per friend online, new friends' size, autosave, the BATTLE FLAG's range).
// L / R switch tabs. Values change live with Left / Right (or A); B or CLOSE saves and leaves.
#include "global.h"
#include "option_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "list_menu.h"
#include "mp.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "platform/display.h"
#include "platform/gamepad.h"
#include "constants/characters.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "field_player_avatar.h"
#include "overworld.h"

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

enum { TAB_GAME, TAB_DISPLAY, TAB_CONTROLS, TAB_COOP, TAB_COUNT };

// What a row sets
enum
{
    OPT_TEXTSPEED,
    OPT_BATTLESCENE,
    OPT_BATTLESTYLE,
    OPT_SOUND,
    OPT_BUTTONMODE,
    OPT_FRAME,
    OPT_TYPEICONS,
    OPT_NAMECASE,
    OPT_CREATURES,
    OPT_FOLLOWSPRITES,
    OPT_BIKEMUSIC,
    OPT_SURFMUSIC,
    OPT_LEVELSCALE,
    OPT_FULLSCREEN,
    OPT_WINSCALE,
    OPT_SCALING,
    OPT_FILTER,
    OPT_VSYNC,
    OPT_PADSELECT,
    OPT_PADLAYOUT,
    OPT_ANALOGRUN,
    OPT_RUMBLE,
    OPT_BUTTONICONS,
    OPT_SPECPANEL,
    OPT_SPECFRIEND,   // a friend online: A opens their page
    OPT_SPECNEW,
    OPT_AUTOSAVE,
    OPT_FLAGRANGE,
    OPT_MOENAV,
    OPT_FR_SHOW,      // a friend's page
    OPT_FR_PLACE,
    OPT_FR_SIZE,
    OPT_FR_BACK,
    OPT_SC_ON,        // the LEVEL SCALING page (MP_SCALE_*, in order)
    OPT_SC_WILD,
    OPT_SC_TRAINERS,
    OPT_SC_BOSS,
    OPT_SC_CAPS,
    OPT_SC_PAST8,
    OPT_SC_BACK,
    OPT_CLOSE,
};
#define SC_RULE(opt) ((opt) - OPT_SC_ON)

#define MAX_ROWS 16
static EWRAM_DATA struct { u8 opt, arg; } sRows[MAX_ROWS] = {0};
static EWRAM_DATA u8 sRowCount = 0;
static EWRAM_DATA u8 sTab = 0;
static EWRAM_DATA u8 sSel = 0;
static EWRAM_DATA u16 sTop = 0;
static EWRAM_DATA u8 sFriendPage = 0;      // 0: the CO-OP tab, 1 + player id: that friend's page
static EWRAM_DATA bool8 sScalePage = 0;    // GAME > LEVEL SCALING's page
static EWRAM_DATA u8 sCoopCursor = 0;      // where the CO-OP tab's cursor was before a friend's page
static EWRAM_DATA u8 sArrowTask = 0;
static EWRAM_DATA u8 sStartCreatures = 0;
static EWRAM_DATA u32 sShownKey = 0;       // what the screen shows (redrawn when something changes elsewhere)
static EWRAM_DATA u8 sStartMusic = 0;      // BIKE MUSIC / SURF MUSIC when OPTIONS opened
static EWRAM_DATA u8 sLabelEnd[MAX_ROWS] = {0};   // where each row's label ends (its choices start after it)

// Rows 14 pixels apart; seven show at a time (the list scrolls) and a hint line under them
#define OPTION_ROW_H 14
#define OPTION_VISIBLE_ROWS 7
#define VALUE_X 104
#define VALUE_R 198
#define ROW_Y(i) ((s16)((i) - (s16)sTop) * OPTION_ROW_H)

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void DrawBgWindowFrames(void);
static void RedrawAll(void);

static const u16 sOptionMenuText_Pal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCGFX_U8("graphics/interface/option_menu_equals_sign.png", ".4bpp");

static const u8 sText_TabGame[] = _("GAME");
static const u8 sText_TabDisplay[] = _("DISPLAY");
static const u8 sText_TabControls[] = _("CONTROLS");
static const u8 sText_TabCoop[] = _("CO-OP");
static const u8 *const sTabNames[TAB_COUNT] = {sText_TabGame, sText_TabDisplay, sText_TabControls, sText_TabCoop};

static const u8 sText_TypeIcons[] = _("TYPE ICONS");
static const u8 sText_NameCase[] = _("NAME CASE");
static const u8 sText_Creatures[] = _("CREATURES");
static const u8 sText_Fullscreen[] = _("FULLSCREEN");
static const u8 sText_WinScale[] = _("WINDOW SCALE");
static const u8 sText_Scaling[] = _("SCALING");
static const u8 sText_Filter[] = _("FILTER");
static const u8 sText_VSync[] = _("VSYNC");
static const u8 sText_Controller[] = _("CONTROLLER");
static const u8 sText_PadButtons[] = _("PAD BUTTONS");
static const u8 sText_StickRun[] = _("STICK RUN");
static const u8 sText_Rumble[] = _("RUMBLE");
static const u8 sText_FriendsScreens[] = _("FRIENDS' SCREENS");
static const u8 sText_NewFriends[] = _("NEW FRIENDS");
static const u8 sText_Autosave[] = _("AUTOSAVE");
static const u8 sText_FlagRange[] = _("FLAG RANGE");
static const u8 sText_BikeMusic[] = _("BIKE MUSIC");
static const u8 sText_Followers[] = _("FOLLOWERS");
static const u8 sText_FollowDefault[] = _("DEFAULT");
static const u8 sText_FollowPmd[] = _("PMD");
static const u8 sText_FollowOriginal[] = _("ORIGINAL");
static const u8 sText_LevelScale[] = _("LEVEL SCALING");
static const u8 sText_ScWild[] = _("WILD");
static const u8 sText_ScTrainers[] = _("TRAINERS");
static const u8 sText_ScBoss[] = _("BOSS MARGIN");
static const u8 sText_ScCaps[] = _("BADGE CAPS");
static const u8 sText_ScPast8[] = _("PAST 8 BADGES");
static const u8 sText_ScLow[] = _("LOW");
static const u8 sText_ScMedium[] = _("MEDIUM");
static const u8 sText_ScHigh[] = _("HIGH");
static const u8 sText_ScBelow[] = _("BELOW");
static const u8 sText_ScEven[] = _("EVEN");
static const u8 sText_ScAbove[] = _("ABOVE");
static const u8 sText_ScToLeague[] = _("TO LEAGUE");
static const u8 sText_ScFree[] = _("FREE");
static const u8 sText_ScCapped[] = _("CAPPED");
static const u8 sText_ScAuto[] = _("AUTO");
static const u8 sText_SurfMusic[] = _("SURF MUSIC");
static const u8 sText_ButtonIcons[] = _("BUTTON ICONS");
static const u8 sText_MoeNav[] = _("MOENAV CALLS");
static const u8 sText_Show[] = _("SHOW");
static const u8 sText_Place[] = _("PLACE");
static const u8 sText_Size[] = _("SIZE");
static const u8 sText_Back[] = _("BACK");
static const u8 sText_Close[] = _("CLOSE");

static const u8 sText_On[] = _("ON");
static const u8 sText_Off[] = _("OFF");
static const u8 sText_NameCaseAsTyped[] = _("UNCORRECTED");
static const u8 sText_NameCaseCaps[] = _("ALL CAPS");
static const u8 sText_NameCaseMixed[] = _("Mixed Case");
static const u8 sText_CreaturesMoemon[] = _("MOéMON");
static const u8 sText_CreaturesPokemon[] = _("POKéMON");
static const u8 sText_PixelPerfect[] = _("PIXEL PERFECT");
static const u8 sText_SharpFit[] = _("SHARP FIT");
static const u8 sText_Stretch[] = _("STRETCH");
static const u8 sText_Scanlines[] = _("SCANLINES");
static const u8 sText_LcdGrid[] = _("LCD GRID");
static const u8 sText_All[] = _("ALL");
static const u8 sText_PressAButton[] = _("PRESS A BUTTON");
static const u8 sText_ABottom[] = _("A = BOTTOM");
static const u8 sText_ARight[] = _("A = RIGHT");
static const u8 sText_AutoBottom[] = _("AUTO (BOTTOM)");
static const u8 sText_AutoRight[] = _("AUTO (RIGHT)");
static const u8 sText_Below[] = _("BELOW");
static const u8 sText_Above[] = _("ABOVE");
static const u8 sText_Left[] = _("LEFT");
static const u8 sText_Right[] = _("RIGHT");
static const u8 sText_OwnWindow[] = _("OWN WINDOW");
static const u8 sText_Auto[] = _("AUTO");
static const u8 sText_Keyboard[] = _("KEYBOARD");
static const u8 sText_Xbox[] = _("XBOX");
static const u8 sText_PlayStation[] = _("PLAYSTATION");
static const u8 sText_Nintendo[] = _("NINTENDO");
static const u8 sText_AllCalls[] = _("ALL");
static const u8 sText_Battles[] = _("BATTLES");
static const u8 *const sPlaceNames[5] = {sText_Below, sText_Above, sText_Left, sText_Right, sText_OwnWindow};

static const u16 sAutosaveSteps[] = {0, 5, 10, 15, 20, 30, 45, 60};

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// ---------------------------------------------------------------------------
// The rows of each tab

static void AddRow(u8 opt, u8 arg)
{
    if (sRowCount < MAX_ROWS)
    {
        sRows[sRowCount].opt = opt;
        sRows[sRowCount].arg = arg;
        sRowCount++;
    }
}

static void BuildRows(void)
{
    u8 ids[MP_MAX_PLAYERS], n, i;
    sRowCount = 0;
    if (sScalePage)
    {
        for (i = OPT_SC_ON; i <= OPT_SC_BACK; i++)
            AddRow(i, 0);
        return;
    }
    if (sFriendPage)
    {
        AddRow(OPT_FR_SHOW, sFriendPage - 1);
        AddRow(OPT_FR_PLACE, sFriendPage - 1);
        AddRow(OPT_FR_SIZE, sFriendPage - 1);
        AddRow(OPT_FR_BACK, 0);
        return;
    }
    switch (sTab)
    {
    case TAB_GAME:
        AddRow(OPT_TEXTSPEED, 0);
        AddRow(OPT_BATTLESCENE, 0);
        AddRow(OPT_BATTLESTYLE, 0);
        AddRow(OPT_SOUND, 0);
        AddRow(OPT_BUTTONMODE, 0);
        AddRow(OPT_FRAME, 0);
        AddRow(OPT_TYPEICONS, 0);
        AddRow(OPT_NAMECASE, 0);
        AddRow(OPT_CREATURES, 0);
        AddRow(OPT_FOLLOWSPRITES, 0);
        AddRow(OPT_BIKEMUSIC, 0);
        AddRow(OPT_SURFMUSIC, 0);
        AddRow(OPT_LEVELSCALE, 0);
        break;
    case TAB_DISPLAY:
        AddRow(OPT_FULLSCREEN, 0);
        AddRow(OPT_WINSCALE, 0);
        AddRow(OPT_SCALING, 0);
        AddRow(OPT_FILTER, 0);
        AddRow(OPT_VSYNC, 0);
        break;
    case TAB_CONTROLS:
        AddRow(OPT_PADSELECT, 0);
        AddRow(OPT_PADLAYOUT, 0);
        AddRow(OPT_ANALOGRUN, 0);
        AddRow(OPT_RUMBLE, 0);
        AddRow(OPT_BUTTONICONS, 0);
        break;
    case TAB_COOP:
        AddRow(OPT_SPECPANEL, 0);
        n = MP_Spectate_ListFriends(ids);
        for (i = 0; i < n; i++)
            AddRow(OPT_SPECFRIEND, ids[i]);
        AddRow(OPT_SPECNEW, 0);
        AddRow(OPT_AUTOSAVE, 0);
        AddRow(OPT_FLAGRANGE, 0);
        AddRow(OPT_MOENAV, 0);
        break;
    }
    AddRow(OPT_CLOSE, 0);
}

// ---------------------------------------------------------------------------
// Values

static int Wrap(int v, int n)
{
    return (v % n + n) % n;
}

// Online in a room someone else made, LEVEL SCALING is theirs: shown, not changed
static bool8 ScaleLocked(void)
{
    return MP_Lobby_InRoom() && !MP_LevelScalingIsOurs();
}

static u8 AutosaveIndex(void)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sAutosaveSteps) - 1 && sAutosaveSteps[i] < gMpConfig.autosaveMinutes; i++)
        ;
    return i;
}

static u8 FriendPlace(const struct MpSpecFriend *fr)
{
    return fr->popout ? 4 : fr->side;
}

// The row's choices: how many, which one is on, and their names. A row with a free-form value (a name, a count)
// has one choice, written into buf.
static u8 Choices(u8 row, const u8 **names, u8 *buf)
{
    u8 opt = sRows[row].opt, arg = sRows[row].arg;
    switch (opt)
    {
    case OPT_TEXTSPEED:
        names[0] = gText_TextSpeedSlow, names[1] = gText_TextSpeedMid, names[2] = gText_TextSpeedFast;
        return 3;
    case OPT_BATTLESCENE:
        names[0] = gText_BattleSceneOn, names[1] = gText_BattleSceneOff;
        return 2;
    case OPT_BATTLESTYLE:
        names[0] = gText_BattleStyleShift, names[1] = gText_BattleStyleSet;
        return 2;
    case OPT_SOUND:
        names[0] = gText_SoundMono, names[1] = gText_SoundStereo;
        return 2;
    case OPT_BUTTONMODE:
        names[0] = gText_ButtonTypeNormal, names[1] = gText_ButtonTypeLR, names[2] = gText_ButtonTypeLEqualsA;
        return 3;
    case OPT_TYPEICONS: case OPT_FULLSCREEN: case OPT_VSYNC: case OPT_ANALOGRUN: case OPT_RUMBLE: case OPT_SPECPANEL:
    case OPT_FR_SHOW: case OPT_BIKEMUSIC: case OPT_SURFMUSIC:
        names[0] = sText_Off, names[1] = sText_On;
        return 2;
    case OPT_NAMECASE:
        names[0] = sText_NameCaseAsTyped, names[1] = sText_NameCaseCaps, names[2] = sText_NameCaseMixed;
        return 3;
    case OPT_CREATURES:
        names[0] = sText_CreaturesMoemon, names[1] = sText_CreaturesPokemon;
        return 2;
    case OPT_SCALING:
        names[0] = sText_PixelPerfect, names[1] = sText_SharpFit, names[2] = sText_Stretch;
        return 3;
    case OPT_FILTER:
        names[0] = sText_Off, names[1] = sText_Scanlines, names[2] = sText_LcdGrid;
        return 3;
    case OPT_FR_PLACE:
        {
            u8 i;
            for (i = 0; i < 5; i++)
                names[i] = sPlaceNames[i];
            return 5;
        }
    case OPT_BUTTONICONS:
        names[0] = sText_Auto, names[1] = sText_Keyboard, names[2] = sText_Xbox, names[3] = sText_PlayStation;
        names[4] = sText_Nintendo;
        return 5;
    case OPT_MOENAV:
        names[0] = sText_AllCalls, names[1] = sText_Battles;
        return 2;
    case OPT_FOLLOWSPRITES:
        names[0] = sText_FollowDefault, names[1] = sText_FollowPmd, names[2] = sText_FollowOriginal;
        return 3;
    case OPT_SC_ON: case OPT_SC_CAPS:
        names[0] = sText_Off, names[1] = sText_On;
        return 2;
    case OPT_SC_WILD:
        names[0] = sText_Off, names[1] = sText_ScLow, names[2] = sText_ScMedium, names[3] = sText_ScHigh;
        return 4;
    case OPT_SC_TRAINERS:
        names[0] = sText_Off, names[1] = sText_ScBelow, names[2] = sText_ScEven, names[3] = sText_ScAbove;
        return 4;
    case OPT_SC_PAST8:
        names[0] = sText_ScToLeague, names[1] = sText_ScFree, names[2] = sText_ScCapped;
        return 3;
    }
    // one free-form value
    buf[0] = EOS;
    switch (opt)
    {
    case OPT_FRAME:
        {
            u8 *p = StringCopy(buf, gText_FrameType);
            *p++ = CHAR_SPACE;
            ConvertIntToDecimalStringN(p, gSaveBlock2Ptr->optionsWindowFrameType + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        }
        break;
    case OPT_WINSCALE:
        {
            u8 *p = ConvertIntToDecimalStringN(buf, Display_GetWindowScale(), STR_CONV_MODE_LEFT_ALIGN, 2);
            p[0] = CHAR_x, p[1] = EOS;
        }
        break;
    case OPT_LEVELSCALE:
        StringCopy(buf, MP_LevelScaleRule(MP_SCALE_ON) ? sText_On : sText_Off);
        break;
    case OPT_SC_BOSS:
        {
            u8 v = MP_LevelScaleRule(MP_SCALE_BOSS);
            if (v == 0)
                StringCopy(buf, sText_ScAuto);
            else
            {
                buf[0] = CHAR_PLUS;
                ConvertIntToDecimalStringN(buf + 1, v - 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            }
        }
        break;
    case OPT_PADSELECT:
        if (Gamepad_IsSelecting())
            StringCopy(buf, sText_PressAButton);
        else if (strcmp(Gamepad_SelectedName(), "ALL CONTROLLERS") == 0)
            StringCopy(buf, sText_All);
        else
            MP_AsciiToGame(buf, Gamepad_SelectedName(), 12);
        break;
    case OPT_PADLAYOUT:
        if (Gamepad_GetLayout() == GAMEPAD_LAYOUT_STANDARD)
            StringCopy(buf, sText_ABottom);
        else if (Gamepad_GetLayout() == GAMEPAD_LAYOUT_NINTENDO)
            StringCopy(buf, sText_ARight);
        else
            StringCopy(buf, Gamepad_EffectiveNintendoLayout() ? sText_AutoRight : sText_AutoBottom);
        break;
    case OPT_SPECFRIEND:
        {
            const struct MpSpecFriend *fr = MP_Spectate_Friend(arg, FALSE);
            u8 shown = MP_Spectate_ShownSize(arg);
            char ascii[24];
            static const char *const places[5] = {"BELOW", "ABOVE", "LEFT", "RIGHT", "WINDOW"};
            if (!fr->show)
                snprintf(ascii, sizeof(ascii), "OFF");
            else
                snprintf(ascii, sizeof(ascii), "%s %dX", places[FriendPlace(fr)], shown ? shown : fr->size);
            MP_AsciiToGame(buf, ascii, 20);
        }
        break;
    case OPT_SPECNEW:
    case OPT_FR_SIZE:
        {
            int size = opt == OPT_SPECNEW ? gMpConfig.spectateScale : MP_Spectate_Friend(arg, FALSE)->size;
            u8 *p = ConvertIntToDecimalStringN(buf, size, STR_CONV_MODE_LEFT_ALIGN, 1);
            p[0] = CHAR_X, p[1] = EOS;
        }
        break;
    case OPT_AUTOSAVE:
        if (gMpConfig.autosaveMinutes == 0)
            StringCopy(buf, sText_Off);
        else
        {
            char ascii[16];
            snprintf(ascii, sizeof(ascii), "%d MIN", gMpConfig.autosaveMinutes);
            MP_AsciiToGame(buf, ascii, 12);
        }
        break;
    case OPT_FLAGRANGE:
        {
            char ascii[16];
            snprintf(ascii, sizeof(ascii), "%d TILES", gMpConfig.trainerRange);
            MP_AsciiToGame(buf, ascii, 12);
        }
        break;
    default:
        return 0;   // BACK, CLOSE: no value
    }
    names[0] = buf;
    return 1;
}

// Which choice is on (rows with several choices)
static u8 Current(u8 row)
{
    u8 arg = sRows[row].arg;
    switch (sRows[row].opt)
    {
    case OPT_TEXTSPEED:   return gSaveBlock2Ptr->optionsTextSpeed;
    case OPT_BATTLESCENE: return gSaveBlock2Ptr->optionsBattleSceneOff;
    case OPT_BATTLESTYLE: return gSaveBlock2Ptr->optionsBattleStyle;
    case OPT_SOUND:       return gSaveBlock2Ptr->optionsSound;
    case OPT_BUTTONMODE:  return gSaveBlock2Ptr->optionsButtonMode;
    case OPT_TYPEICONS:   return gSaveBlock2Ptr->optionsTypeIcons != 0;
    case OPT_NAMECASE:    return gSaveBlock2Ptr->optionsNameCase % MP_NAMECASE_COUNT;
    case OPT_CREATURES:   return gSaveBlock2Ptr->optionsPokemonArt != 0;
    case OPT_FULLSCREEN:  return Display_GetFullscreen() != 0;
    case OPT_SCALING:     return Display_GetScaleMode();
    case OPT_FILTER:      return Display_GetFilter();
    case OPT_VSYNC:       return Display_GetVSync() != 0;
    case OPT_ANALOGRUN:   return Gamepad_GetAnalogRun() != 0;
    case OPT_RUMBLE:      return Gamepad_GetRumble() != 0;
    case OPT_SPECPANEL:   return gMpConfig.spectatePanel != 0;
    case OPT_FR_SHOW:     return MP_Spectate_Friend(arg, FALSE)->show != 0;
    case OPT_FR_PLACE:    return FriendPlace(MP_Spectate_Friend(arg, FALSE));
    case OPT_BIKEMUSIC:   return gMpConfig.bikeMusic != 0;
    case OPT_SURFMUSIC:   return gMpConfig.surfMusic != 0;
    case OPT_LEVELSCALE:  return 0;   // (the page has the settings)
    case OPT_SC_ON: case OPT_SC_WILD: case OPT_SC_TRAINERS: case OPT_SC_BOSS: case OPT_SC_CAPS: case OPT_SC_PAST8:
        return MP_LevelScaleRule(SC_RULE(sRows[row].opt));
    case OPT_BUTTONICONS: return Gamepad_GetIconSet();
    case OPT_MOENAV:      return gMpConfig.moenavCalls != 0;
    case OPT_FOLLOWSPRITES: return gMpConfig.followerSprites % MP_FOLLOW_SPRITES_COUNT;
    }
    return 0;
}

// Left / Right (dir -1 / +1), or A (+1)
static void Change(u8 row, int dir)
{
    u8 arg = sRows[row].arg;
    switch (sRows[row].opt)
    {
    case OPT_TEXTSPEED:   gSaveBlock2Ptr->optionsTextSpeed = Wrap(gSaveBlock2Ptr->optionsTextSpeed + dir, 3); break;
    case OPT_BATTLESCENE: gSaveBlock2Ptr->optionsBattleSceneOff ^= 1; break;
    case OPT_BATTLESTYLE: gSaveBlock2Ptr->optionsBattleStyle ^= 1; break;
    case OPT_SOUND:
        gSaveBlock2Ptr->optionsSound ^= 1;
        SetPokemonCryStereo(gSaveBlock2Ptr->optionsSound);
        break;
    case OPT_BUTTONMODE:  gSaveBlock2Ptr->optionsButtonMode = Wrap(gSaveBlock2Ptr->optionsButtonMode + dir, 3); break;
    case OPT_FRAME:
        gSaveBlock2Ptr->optionsWindowFrameType = Wrap(gSaveBlock2Ptr->optionsWindowFrameType + dir, WINDOW_FRAMES_COUNT);
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        break;
    case OPT_TYPEICONS:   gSaveBlock2Ptr->optionsTypeIcons = !gSaveBlock2Ptr->optionsTypeIcons; break;
    case OPT_NAMECASE:    gSaveBlock2Ptr->optionsNameCase = Wrap(gSaveBlock2Ptr->optionsNameCase + dir, MP_NAMECASE_COUNT); break;
    case OPT_CREATURES:   gSaveBlock2Ptr->optionsPokemonArt = !gSaveBlock2Ptr->optionsPokemonArt; break;
    case OPT_FULLSCREEN:  Display_SetFullscreen(!Display_GetFullscreen()); break;
    case OPT_WINSCALE:    Display_SetWindowScale(Wrap(Display_GetWindowScale() - 1 + dir, Display_GetMaxWindowScale()) + 1); break;
    case OPT_SCALING:     Display_SetScaleMode(Wrap(Display_GetScaleMode() + dir, DISPLAY_SCALE_COUNT)); break;
    case OPT_FILTER:      Display_SetFilter(Wrap(Display_GetFilter() + dir, DISPLAY_FILTER_COUNT)); break;
    case OPT_VSYNC:       Display_SetVSync(!Display_GetVSync()); break;
    case OPT_PADSELECT:   Gamepad_CycleSelected(dir); break;
    case OPT_PADLAYOUT:   Gamepad_SetLayout(Gamepad_GetLayout() + dir); break;
    case OPT_ANALOGRUN:   Gamepad_SetAnalogRun(!Gamepad_GetAnalogRun()); break;
    case OPT_RUMBLE:
        Gamepad_SetRumble(!Gamepad_GetRumble());
        Gamepad_Rumble(50, 200);
        break;
    case OPT_SPECPANEL:
        gMpConfig.spectatePanel = !gMpConfig.spectatePanel;
        MPNet_Log("spectate panel %s", gMpConfig.spectatePanel ? "on" : "off");
        break;
    case OPT_SPECNEW:     gMpConfig.spectateScale = Wrap(gMpConfig.spectateScale - 1 + dir, 4) + 1; break;
    case OPT_AUTOSAVE:    if (MOEMON_DEV_BUILD) break; gMpConfig.autosaveMinutes = sAutosaveSteps[Wrap(AutosaveIndex() + dir, ARRAY_COUNT(sAutosaveSteps))]; break;
    case OPT_FLAGRANGE:   gMpConfig.trainerRange = Wrap(gMpConfig.trainerRange - 1 + dir, 8) + 1; break;
    case OPT_BIKEMUSIC:   gMpConfig.bikeMusic = !gMpConfig.bikeMusic; break;
    case OPT_FOLLOWSPRITES:
        gMpConfig.followerSprites = Wrap(gMpConfig.followerSprites + dir, MP_FOLLOW_SPRITES_COUNT);
        MPNet_Log("follower sprites: %d", gMpConfig.followerSprites);
        break;
    case OPT_SURFMUSIC:   gMpConfig.surfMusic = !gMpConfig.surfMusic; break;
    case OPT_LEVELSCALE:  break;   // A opens its page
    case OPT_SC_ON: case OPT_SC_WILD: case OPT_SC_TRAINERS: case OPT_SC_BOSS: case OPT_SC_CAPS: case OPT_SC_PAST8:
        if (!ScaleLocked())
        {
            u8 rule = SC_RULE(sRows[row].opt);
            MP_LevelScaleSetOwn(rule, Wrap(MP_LevelScaleOwnRule(rule) + dir, MP_LevelScaleRuleCount(rule)));
            MPNet_Log("level scaling rule %d: %d", rule, MP_LevelScaleOwnRule(rule));
        }
        break;
    case OPT_BUTTONICONS: Gamepad_SetIconSet(Gamepad_GetIconSet() + dir); break;
    case OPT_MOENAV:
        gMpConfig.moenavCalls = !gMpConfig.moenavCalls;
        MPNet_Log("moenav calls: %s", gMpConfig.moenavCalls ? "battles" : "all");
        break;
    case OPT_FR_SHOW:
        {
            struct MpSpecFriend *fr = MP_Spectate_Friend(arg, TRUE);
            fr->show = !fr->show;
            MPNet_Log("spectate panel: player %d shown %d", arg + 1, fr->show);
        }
        break;
    case OPT_FR_PLACE:
        {
            static const char *const names[5] = {"BELOW", "ABOVE", "LEFT", "RIGHT", "OWN WINDOW"};
            struct MpSpecFriend *fr = MP_Spectate_Friend(arg, TRUE);
            int p = Wrap(FriendPlace(fr) + dir, 5);
            fr->popout = p == 4;
            if (p < 4)
                fr->side = p;
            fr->show = TRUE;
            MPNet_Log("spectate panel: player %d %s", arg + 1, names[p]);
        }
        break;
    case OPT_FR_SIZE:
        {
            struct MpSpecFriend *fr = MP_Spectate_Friend(arg, TRUE);
            fr->size = Wrap(fr->size - 1 + dir, 4) + 1;
            MPNet_Log("spectate panel: player %d size %dx", arg + 1, fr->size);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Drawing

static const u8 *RowLabel(u8 row, u8 *buf)
{
    switch (sRows[row].opt)
    {
    case OPT_TEXTSPEED:   return gText_TextSpeed;
    case OPT_BATTLESCENE: return gText_BattleScene;
    case OPT_BATTLESTYLE: return gText_BattleStyle;
    case OPT_SOUND:       return gText_Sound;
    case OPT_BUTTONMODE:  return gText_ButtonMode;
    case OPT_FRAME:       return gText_Frame;
    case OPT_TYPEICONS:   return sText_TypeIcons;
    case OPT_NAMECASE:    return sText_NameCase;
    case OPT_CREATURES:   return sText_Creatures;
    case OPT_FULLSCREEN:  return sText_Fullscreen;
    case OPT_WINSCALE:    return sText_WinScale;
    case OPT_SCALING:     return sText_Scaling;
    case OPT_FILTER:      return sText_Filter;
    case OPT_VSYNC:       return sText_VSync;
    case OPT_PADSELECT:   return sText_Controller;
    case OPT_PADLAYOUT:   return sText_PadButtons;
    case OPT_ANALOGRUN:   return sText_StickRun;
    case OPT_RUMBLE:      return sText_Rumble;
    case OPT_SPECPANEL:   return sText_FriendsScreens;
    case OPT_SPECNEW:     return sText_NewFriends;
    case OPT_AUTOSAVE:    return sText_Autosave;
    case OPT_FLAGRANGE:   return sText_FlagRange;
    case OPT_BIKEMUSIC:   return sText_BikeMusic;
    case OPT_FOLLOWSPRITES: return sText_Followers;
    case OPT_SC_ON:       return sText_LevelScale;
    case OPT_SC_WILD:     return sText_ScWild;
    case OPT_SC_TRAINERS: return sText_ScTrainers;
    case OPT_SC_BOSS:     return sText_ScBoss;
    case OPT_SC_CAPS:     return sText_ScCaps;
    case OPT_SC_PAST8:    return sText_ScPast8;
    case OPT_SC_BACK:     return sText_Back;
    case OPT_SURFMUSIC:   return sText_SurfMusic;
    case OPT_LEVELSCALE:  return sText_LevelScale;
    case OPT_BUTTONICONS: return sText_ButtonIcons;
    case OPT_MOENAV:      return sText_MoeNav;
    case OPT_FR_SHOW:     return sText_Show;
    case OPT_FR_PLACE:    return sText_Place;
    case OPT_FR_SIZE:     return sText_Size;
    case OPT_FR_BACK:     return sText_Back;
    case OPT_SPECFRIEND:
        StringCopyN(buf, gMpPlayers[sRows[row].arg].name, PLAYER_NAME_LENGTH);
        buf[PLAYER_NAME_LENGTH] = EOS;
        return buf;
    }
    return sText_Close;
}

// A choice in the original options' colors: red when it's on, gray-green when not
static void DrawChoice(const u8 *text, int x, s16 y, bool8 on, u8 fontId)
{
    u8 dst[40];
    u16 i = 0;
    if (y < 0 || y >= OPTION_VISIBLE_ROWS * OPTION_ROW_H)
        return;
    if (text[0] == EXT_CTRL_CODE_BEGIN)
        text += 6;   // the original texts start with their own color codes
    dst[i++] = EXT_CTRL_CODE_BEGIN;
    dst[i++] = EXT_CTRL_CODE_COLOR;
    dst[i++] = on ? TEXT_COLOR_RED : TEXT_COLOR_GREEN;
    dst[i++] = EXT_CTRL_CODE_BEGIN;
    dst[i++] = EXT_CTRL_CODE_SHADOW;
    dst[i++] = on ? TEXT_COLOR_LIGHT_RED : TEXT_COLOR_LIGHT_GREEN;
    for (; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);
    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, fontId, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static void DrawRowValue(u8 row)
{
    const u8 *names[5];
    u8 buf[40];
    u8 n = Choices(row, names, buf), cur = Current(row), i;
    s16 y = ROW_Y(row);
    int total = 0, gap, left = sLabelEnd[row] + 6 > VALUE_X ? sLabelEnd[row] + 6 : VALUE_X;
    bool8 filterOff = sRows[row].opt == OPT_CREATURES || sRows[row].opt == OPT_NAMECASE;

    if (y < 0 || y >= OPTION_VISIBLE_ROWS * OPTION_ROW_H || n == 0)
        return;
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), VALUE_X, y, VALUE_R + 8 - VALUE_X, OPTION_ROW_H);
    gMpTextFilterOff = filterOff;   // the names as written (the filter would recase "Mixed Case" or swap MOéMON)
    for (i = 0; i < n; i++)
        total += GetStringWidth(FONT_NORMAL, names[i][0] == EXT_CTRL_CODE_BEGIN ? names[i] + 6 : names[i], 0);
    gap = n > 1 ? (VALUE_R - left - total) / (n - 1) : 0;
    if (n >= 2 && n <= 3 && gap >= 6)
    {
        // every choice, the one that's on in red (as the original options show them)
        int x = left;
        for (i = 0; i < n; i++)
        {
            int w = GetStringWidth(FONT_NORMAL, names[i][0] == EXT_CTRL_CODE_BEGIN ? names[i] + 6 : names[i], 0);
            if (i == n - 1)
                x = VALUE_R - w;
            DrawChoice(names[i], x, y, i == cur, FONT_NORMAL);
            x += w + gap;
        }
    }
    else
    {
        // just the one that's on, in red, in the middle (smaller when it's long)
        const u8 *t = n == 1 ? names[0] : names[cur < n ? cur : 0];
        u8 font = GetStringWidth(FONT_NORMAL, t, 0) > VALUE_R - VALUE_X ? FONT_SMALL_NARROW : FONT_NORMAL;
        int w = GetStringWidth(font, t, 0);
        DrawChoice(t, VALUE_X + (VALUE_R - VALUE_X - w) / 2, y, TRUE, font);
    }
    gMpTextFilterOff = FALSE;
}

// The line under the rows: what the row does, or what's going on
static void DrawHint(void)
{
    char ascii[64];
    u8 buf[64];
    u8 opt = sRowCount ? sRows[sSel].opt : OPT_CLOSE, arg = sRows[sSel].arg;
    static const u8 sColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    int y = OPTION_VISIBLE_ROWS * OPTION_ROW_H + 1;

    if (Gamepad_IsSelecting())
        snprintf(ascii, sizeof(ascii), "PRESS ANY BUTTON ON THE CONTROLLER TO USE");
    else if (opt == OPT_FILTER && Display_GetFilter() != DISPLAY_FILTER_OFF && !Display_IsFilterShown())
        snprintf(ascii, sizeof(ascii), "WINDOW TOO SMALL FOR THE FILTER");
    else if (opt == OPT_PADSELECT)
        snprintf(ascii, sizeof(ascii), "%s", Gamepad_IsConnected() ? "A: PICK THE CONTROLLER TO USE" : "NO CONTROLLER CONNECTED");
    else if (opt == OPT_PADLAYOUT || opt == OPT_ANALOGRUN || opt == OPT_RUMBLE)
        snprintf(ascii, sizeof(ascii), "%.40s", Gamepad_IsConnected() ? Gamepad_Name() : "NO CONTROLLER CONNECTED");
    else if (opt == OPT_FULLSCREEN || opt == OPT_WINSCALE)
        snprintf(ascii, sizeof(ascii), "F11 OR ALT+ENTER: FULLSCREEN");
    else if (opt == OPT_SPECPANEL)
        snprintf(ascii, sizeof(ascii), "FRIENDS ONLINE GET A ROW: A OPENS IT");
    else if (opt == OPT_SPECFRIEND)
        snprintf(ascii, sizeof(ascii), "A: WHERE THEIR SCREEN GOES, HOW BIG");
    else if (opt == OPT_SPECNEW)
        snprintf(ascii, sizeof(ascii), "THE SIZE FOR FRIENDS NOT SET UP YET");
    else if (opt == OPT_AUTOSAVE)
        snprintf(ascii, sizeof(ascii), "A QUIET SAVE ON THE MAP THIS OFTEN");
    else if (opt == OPT_FLAGRANGE)
        snprintf(ascii, sizeof(ascii), "HOW FAR YOU SEE WITH THE BATTLE FLAG");
    else if (opt == OPT_BIKEMUSIC)
        snprintf(ascii, sizeof(ascii), "OFF: THE MAP'S OWN SONG ON A BIKE");
    else if (opt == OPT_FOLLOWSPRITES)
        snprintf(ascii, sizeof(ascii), "%s", gMpConfig.followerSprites == MP_FOLLOW_SPRITES_PMD ? "PMD PICTURES EVERYWHERE"
                 : gMpConfig.followerSprites == MP_FOLLOW_SPRITES_ORIGINAL ? "THE ORIGINAL SPRITES EVERYWHERE"
                 : "ORIGINAL OUTSIDE, PMD IN THE GARDEN");
    else if (opt == OPT_SURFMUSIC)
        snprintf(ascii, sizeof(ascii), "OFF: THE MAP'S SONG SURFING AND DIVING");
    else if (opt == OPT_LEVELSCALE)
        snprintf(ascii, sizeof(ascii), "%s", ScaleLocked() ? "ONLINE THE HOST DECIDES. A: SEE IT" : "A: HOW WILD AND TRAINERS KEEP UP");
    else if (opt >= OPT_SC_ON && opt <= OPT_SC_PAST8 && ScaleLocked())
        snprintf(ascii, sizeof(ascii), "THE HOST DECIDES THIS ONLINE");
    else if (opt == OPT_SC_ON)
        snprintf(ascii, sizeof(ascii), "WILD AND TRAINERS KEEP UP WITH YOU");
    else if (opt == OPT_SC_WILD)
        snprintf(ascii, sizeof(ascii), "%s", (const char *[]){"WILD ONES AT THEIR OWN LEVELS", "A QUARTER OF THE WAY TO YOUR TEAM",
                 "HALFWAY TO YOUR TEAM'S LEVEL", "A FEW LEVELS BELOW YOUR TEAM"}[MP_LevelScaleRule(MP_SCALE_WILD) & 3]);
    else if (opt == OPT_SC_TRAINERS)
        snprintf(ascii, sizeof(ascii), "%s", (const char *[]){"TRAINERS AT THEIR OWN LEVELS", "THEIR BEST 4 BELOW YOUR TEAM",
                 "THEIR BEST AT YOUR TEAM'S LEVEL", "THEIR BEST 3 ABOVE YOUR TEAM"}[MP_LevelScaleRule(MP_SCALE_TRAINERS) & 3]);
    else if (opt == OPT_SC_BOSS)
        snprintf(ascii, sizeof(ascii), "HOW FAR ABOVE YOUR TEAM BOSSES ARE");
    else if (opt == OPT_SC_CAPS)
        snprintf(ascii, sizeof(ascii), "A LEVEL CAP THAT RISES WITH BADGES");
    else if (opt == OPT_SC_PAST8)
        snprintf(ascii, sizeof(ascii), "%s", (const char *[]){"CAPPED UNTIL THE LEAGUE, THEN FREE", "NO CAP ONCE YOU HAVE ALL 8",
                 "ALWAYS A CAP (80 AFTER THE LEAGUE)"}[MP_LevelScaleRule(MP_SCALE_PAST8) % 3]);
    else if (opt == OPT_BUTTONICONS)
        snprintf(ascii, sizeof(ascii), "AUTO: THE KEYS OR PAD YOU USED LAST");
    else if (opt == OPT_MOENAV)
        snprintf(ascii, sizeof(ascii), "BATTLES: ONLY CALLS ASKING FOR A BATTLE");
    else if (opt == OPT_FR_PLACE)
        snprintf(ascii, sizeof(ascii), "%s", MP_Spectate_Friend(arg, FALSE)->popout ? "CLOSING ITS WINDOW HIDES IT" : "WHICH SIDE OF YOUR SCREEN");
    else if (opt == OPT_FR_SIZE)
    {
        u8 shown = MP_Spectate_ShownSize(arg);
        int size = MP_Spectate_Friend(arg, FALSE)->size;
        if (shown && shown != size)
            snprintf(ascii, sizeof(ascii), "%dX DOESN'T FIT: SHOWN AT %dX", size, shown);
        else
            snprintf(ascii, sizeof(ascii), "1X TO 4X");
    }
    else
        snprintf(ascii, sizeof(ascii), "L / R: TABS     MOEMON EMERALD KAI V1.0");
    MP_AsciiToGame(buf, ascii, 60);
    FillWindowPixelRect(WIN_OPTIONS, PIXEL_FILL(1), 0, y - 1, 26 * 8, OPTION_ROW_H);
    AddTextPrinterParameterized3(WIN_OPTIONS, FONT_SMALL, (26 * 8 - GetStringWidth(FONT_SMALL, buf, 0)) / 2, y, sColors,
                                 TEXT_SKIP_DRAW, buf);
}

static void DrawHeader(void)
{
    int i, x = 8;
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    if (sScalePage)
    {
        u8 buf[40];
        u8 *p = StringCopy(buf, sTabNames[TAB_GAME]);
        *p++ = CHAR_SPACE;
        *p++ = CHAR_GREATER_THAN;
        *p++ = CHAR_SPACE;
        StringCopy(p, sText_LevelScale);
        AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, buf, 8, 1, TEXT_SKIP_DRAW, NULL);
    }
    else if (sFriendPage)
    {
        u8 buf[32];
        u8 *p = StringCopy(buf, sText_TabCoop);
        *p++ = CHAR_SPACE;
        *p++ = CHAR_GREATER_THAN;
        *p++ = CHAR_SPACE;
        p = StringCopyN(p, gMpPlayers[sFriendPage - 1].name, PLAYER_NAME_LENGTH);
        *p = EOS;
        AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, buf, 8, 1, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        // the tabs, the one shown in red
        for (i = 0; i < TAB_COUNT; i++)
        {
            u8 dst[24];
            u8 j = 0;
            const u8 *t = sTabNames[i];
            dst[j++] = EXT_CTRL_CODE_BEGIN;
            dst[j++] = EXT_CTRL_CODE_COLOR;
            dst[j++] = i == sTab ? TEXT_COLOR_RED : TEXT_COLOR_GREEN;
            dst[j++] = EXT_CTRL_CODE_BEGIN;
            dst[j++] = EXT_CTRL_CODE_SHADOW;
            dst[j++] = i == sTab ? TEXT_COLOR_LIGHT_RED : TEXT_COLOR_LIGHT_GREEN;
            while (*t != EOS && j < sizeof(dst) - 1)
                dst[j++] = *t++;
            dst[j] = EOS;
            AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, dst, x, 1, TEXT_SKIP_DRAW, NULL);
            x += GetStringWidth(FONT_NORMAL, sTabNames[i], 0) + 16;
        }
    }
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void HighlightRow(void)
{
    u8 row = sSel - sTop;
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(row * OPTION_ROW_H + 40, row * OPTION_ROW_H + 40 + OPTION_ROW_H));
}

// A number for everything the rows show, to redraw when something changes elsewhere (F11, a controller, a friend)
static u32 ShownKey(void)
{
    u32 key = 5381 + sRowCount * 7 + sTab * 131 + sFriendPage * 977 + sScalePage * 31 + Gamepad_IsSelecting();
    u8 i, j;
    for (i = 0; i < sRowCount; i++)
    {
        const u8 *names[5];
        u8 buf[40], n = Choices(i, names, buf);
        key = key * 33 + sRows[i].opt * 7 + sRows[i].arg;
        key = key * 33 + Current(i);
        if (n == 1)
            for (j = 0; names[0][j] != EOS; j++)
                key = key * 33 + names[0][j];
    }
    key = key * 33 + Display_IsFilterShown() + Gamepad_IsConnected() * 3;
    if (sFriendPage)
        key = key * 33 + MP_Spectate_ShownSize(sFriendPage - 1);   // the SIZE hint says what fits
    return key;
}

static void SetScrollArrows(void)
{
    if (sArrowTask != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sArrowTask);
    sArrowTask = TASK_NONE;
    if (sRowCount > OPTION_VISIBLE_ROWS)
        sArrowTask = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, DISPLAY_WIDTH / 2, 36,
            40 + OPTION_VISIBLE_ROWS * OPTION_ROW_H + 2, sRowCount - OPTION_VISIBLE_ROWS, 0x2710, 0x2710, &sTop);
}

static void RedrawAll(void)
{
    u8 i, buf[16];
    if (sSel >= sRowCount)
        sSel = sRowCount - 1;
    if (sSel < sTop)
        sTop = sSel;
    else if (sSel >= sTop + OPTION_VISIBLE_ROWS)
        sTop = sSel - OPTION_VISIBLE_ROWS + 1;
    if (sRowCount <= OPTION_VISIBLE_ROWS)
        sTop = 0;
    DrawHeader();
    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = sTop; i < sRowCount && i < sTop + OPTION_VISIBLE_ROWS; i++)
    {
        const u8 *label = RowLabel(i, buf);
        // a long label in the narrow font, clear of the values
        u8 font = GetStringWidth(FONT_NORMAL, label, 0) > VALUE_X - 8 - 6 ? FONT_SMALL_NARROW : FONT_NORMAL;
        sLabelEnd[i] = 8 + GetStringWidth(font, label, 0);
        AddTextPrinterParameterized(WIN_OPTIONS, font, label, 8, ROW_Y(i) + 1, TEXT_SKIP_DRAW, NULL);
        DrawRowValue(i);
    }
    DrawHint();
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
    HighlightRow();
    sShownKey = ShownKey();
}

static void GoTo(u8 tab, u8 friendPage, u8 sel)
{
    sTab = tab;
    sFriendPage = friendPage;
    BuildRows();
    sSel = sel < sRowCount ? sel : 0;
    sTop = 0;
    SetScrollArrows();
    RedrawAll();
}

// ---------------------------------------------------------------------------

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(WIN_HEADER);
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(WIN_OPTIONS);
        gMain.state++;
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
        CreateTask(Task_OptionMenuFadeIn, 0);
        sStartCreatures = gSaveBlock2Ptr->optionsPokemonArt;
        sStartMusic = gMpConfig.bikeMusic | (gMpConfig.surfMusic << 1);
        sArrowTask = TASK_NONE;
        if (gSaveBlock2Ptr->optionsWindowFrameType >= WINDOW_FRAMES_COUNT)
            gSaveBlock2Ptr->optionsWindowFrameType = 0;
        sScalePage = FALSE;
        GoTo(TAB_GAME, 0, 0);
        gMain.state++;
        break;
    case 11:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

// Test harness (mp optionstab N): open on a tab
void MP_Options_DebugTab(u8 tab)
{
    if (tab < TAB_COUNT && FuncIsActiveTask(Task_OptionMenuProcessInput))
        GoTo(tab, 0, 0);
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
    u8 opt = sRows[sSel].opt;

    if (Gamepad_IsSelecting())
    {
        // waiting for a controller button; the keyboard can cancel
        if (JOY_NEW(B_BUTTON | START_BUTTON))
        {
            Gamepad_CancelSelect();
            PlaySE(SE_SELECT);
        }
    }
    else if (sFriendPage && !gMpPlayers[sFriendPage - 1].present)
    {
        GoTo(TAB_COOP, 0, 0);   // that friend left
        return;
    }
    else if (!sFriendPage && !sScalePage && JOY_NEW(L_BUTTON | R_BUTTON))
    {
        PlaySE(SE_SELECT);
        GoTo(Wrap(sTab + (JOY_NEW(R_BUTTON) ? 1 : -1), TAB_COUNT), 0, 0);
        return;
    }
    else if (JOY_NEW(A_BUTTON) && opt == OPT_LEVELSCALE)
    {
        PlaySE(SE_SELECT);
        sScalePage = TRUE;
        GoTo(TAB_GAME, 0, 0);
        return;
    }
    else if (sScalePage && (JOY_NEW(B_BUTTON | START_BUTTON) || (JOY_NEW(A_BUTTON) && opt == OPT_SC_BACK)))
    {
        u8 i;
        PlaySE(SE_SELECT);
        sScalePage = FALSE;
        GoTo(TAB_GAME, 0, 0);
        for (i = 0; i < sRowCount; i++)   // back on its row
            if (sRows[i].opt == OPT_LEVELSCALE)
                sSel = i;
        RedrawAll();
        return;
    }
    else if (JOY_NEW(B_BUTTON | START_BUTTON) || (JOY_NEW(A_BUTTON) && (opt == OPT_CLOSE || opt == OPT_FR_BACK)))
    {
        if (sFriendPage)
        {
            PlaySE(SE_SELECT);
            GoTo(TAB_COOP, 0, sCoopCursor);
            return;
        }
        gTasks[taskId].func = Task_OptionMenuSave;
        return;
    }
    else if (JOY_NEW(DPAD_UP | DPAD_DOWN))
    {
        if (JOY_NEW(DPAD_UP))
            sSel = sSel > 0 ? sSel - 1 : sRowCount - 1;
        else
            sSel = sSel + 1 < sRowCount ? sSel + 1 : 0;
        RedrawAll();
        return;
    }
    else if (JOY_NEW(A_BUTTON) && opt == OPT_SPECFRIEND)
    {
        PlaySE(SE_SELECT);
        sCoopCursor = sSel;
        GoTo(TAB_COOP, 1 + sRows[sSel].arg, 1);
        return;
    }
    else if (JOY_NEW(A_BUTTON) && opt == OPT_PADSELECT)
    {
        Gamepad_StartSelect();
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | A_BUTTON) && opt != OPT_SPECFRIEND)
    {
        Change(sSel, JOY_NEW(DPAD_LEFT) ? -1 : 1);
        if (opt == OPT_TEXTSPEED || opt == OPT_BUTTONMODE || opt == OPT_FRAME || opt == OPT_NAMECASE || opt == OPT_BATTLESCENE
         || opt == OPT_BATTLESTYLE || opt == OPT_SOUND || opt == OPT_TYPEICONS || opt == OPT_CREATURES)
            ;   // the original options make no sound
        else
            PlaySE(SE_SELECT);
    }
    // the rows again whenever what they show changed (here or elsewhere)
    if (ShownKey() != sShownKey)
    {
        if (sTab == TAB_COOP && !sFriendPage)
        {
            u8 count = sRowCount;
            BuildRows();   // a friend came or went
            if (count != sRowCount)
                SetScrollArrows();
        }
        RedrawAll();
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    if (gSaveBlock2Ptr->optionsPokemonArt != sStartCreatures)
        MP_MonArt_Changed();
    // BIKE MUSIC / SURF MUSIC changed on the map: the right song now, not at the next map
    if ((gMpConfig.bikeMusic | (gMpConfig.surfMusic << 1)) != sStartMusic && gMain.savedCallback == CB2_ReturnToFieldWithOpenMenu)
    {
        if (gMpConfig.bikeMusic && TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
            Overworld_SetSavedMusic(MUS_CYCLING);
        else if (!gMpConfig.bikeMusic && gSaveBlock1Ptr->savedMusic == MUS_CYCLING)
            Overworld_ClearSavedMusic();
        Overworld_PlaySpecialMapMusic();
    }
    Display_SaveSettings();
    Gamepad_SaveSettings();
    MP_SaveConfig();
    if (sArrowTask != TASK_NONE)
        RemoveScrollIndicatorArrowPair(sArrowTask);
    sArrowTask = TASK_NONE;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}
