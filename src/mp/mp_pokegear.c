// Moemon Kai: JOHTO’s POKéGEAR, as Crystal’s / HeartGold and SoulSilver’s (the user, 2026-11-02: display,
// functions, calls; no clock setting: the time is the game’s own clock). A screen of its own with four cards,
// LEFT / RIGHT between them, B to leave:
// - CLOCK: the day and the time
// - MAP: the region’s TOWN MAP (field_region_map.c), back to the POKéGEAR after
// - PHONE: MOM (Crystal’s call: where you are, then her savings: keep saving or start, mp_mombank.c) and PROF. ELM
//   (Crystal’s calls by how far the player is); Crystal’s words (pokecrystal data/phone/text/mom.asm, elm.asm)
// - RADIO: OAK’s POKéMON TALK (4.5: MARY and PROF. OAK on a POKéMON seen around a place of the region, from its
//   wild tables, in Crystal’s words: data/text/common_1.asm _OPT_*) and the POKé FLUTE (20.0); the music plays while
//   the card is open. Crystal’s other stations need music the game hasn’t got (the MARCH, the LULLABY...).
// On START in JOHTO once MOM has given it (Johto_PlayersHouse1F); elsewhere a key item (ITEM_POKEGEAR, registrable)
// once JOHTO’s LEAGUE is beaten (mp_region.c MP_Region_DeviceHere / MP_Region_GiveDevices).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "bg.h"
#include "event_data.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "rtc.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "wild_encounter.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "mp_region.h"

enum { CARD_CLOCK, CARD_MAP, CARD_PHONE, CARD_RADIO, CARD_COUNT };
enum { WIN_TABS, WIN_BODY, WIN_HINT, WIN_COUNT };   // (the device's frame is the backdrop color)
enum { PHONE_LIST, PHONE_CALL, PHONE_ASK };   // the phone card’s state

// colors of palette 15 (both layers)
#define C_FRAME   1
#define C_SCREEN  2
#define C_INK     3
#define C_INKSH   4
#define C_TAB     5
#define C_TABSEL  6
#define C_WHITE   7
#define C_WHITESH 8
#define C_EDGE    9

static const u16 sPalette[16] = {
    RGB(0, 0, 0), RGB(5, 8, 15), RGB(24, 28, 19), RGB(3, 5, 4), RGB(17, 21, 14), RGB(9, 13, 22), RGB(29, 22, 6),
    RGB(31, 31, 31), RGB(4, 6, 11), RGB(16, 20, 27), 0, 0, 0, 0, 0, 0,
};

static const struct BgTemplate sBgTemplates[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0},
};

static const struct WindowTemplate sWindows[] = {
    [WIN_TABS] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 1, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 1},
    [WIN_BODY] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 4, .width = 28, .height = 12, .paletteNum = 15, .baseBlock = 60},
    [WIN_HINT] = {.bg = 0, .tilemapLeft = 1, .tilemapTop = 17, .width = 28, .height = 2, .paletteNum = 15, .baseBlock = 400},
    DUMMY_WIN_TEMPLATE,
};

static const u8 sColInk[] = {C_SCREEN, C_INK, C_INKSH};
static const u8 sColTab[] = {C_TAB, C_WHITE, C_WHITESH};
static const u8 sColTabSel[] = {C_TABSEL, C_INK, C_TABSEL};
static const u8 sColHint[] = {C_FRAME, C_WHITE, C_WHITESH};

static const u8 sText_Cards[CARD_COUNT][8] = {_("CLOCK"), _("MAP"), _("PHONE"), _("RADIO")};
static const u8 sText_Days[7][10] = {_("SUNDAY"), _("MONDAY"), _("TUESDAY"), _("WEDNESDAY"), _("THURSDAY"),
                                      _("FRIDAY"), _("SATURDAY")};
static const u8 sText_HintClock[] = _("{DPAD_LEFTRIGHT} CARDS   {B_BUTTON} OFF");
static const u8 sText_HintMap[] = _("{A_BUTTON} TOWN MAP   {DPAD_LEFTRIGHT} CARDS   {B_BUTTON} OFF");
static const u8 sText_HintPhone[] = _("{A_BUTTON} CALL   {DPAD_LEFTRIGHT} CARDS   {B_BUTTON} OFF");
static const u8 sText_HintCall[] = _("{A_BUTTON} NEXT");
static const u8 sText_HintAsk[] = _("{A_BUTTON} YES   {B_BUTTON} NO");
static const u8 sText_HintRadio[] = _("{A_BUTTON} TUNE   {DPAD_LEFTRIGHT} CARDS   {B_BUTTON} OFF");
static const u8 sText_TownMap[] = _("TOWN MAP");
static const u8 sText_Johto[] = _("JOHTO");
static const u8 sText_Kanto[] = _("KANTO");
static const u8 sText_Am[] = _(" AM");
static const u8 sText_Pm[] = _(" PM");
static const u8 sText_Mom[] = _("MOM");
static const u8 sText_Elm[] = _("PROF. ELM");
static const u8 sText_Calling[] = _("Calling…");
static const u8 sText_Talk[] = _("4.5  OAK’s POKéMON TALK");
static const u8 sText_Flute[] = _("20.0 POKé FLUTE");
static const u8 sText_RadioOff[] = _("     (OFF)");

static EWRAM_DATA struct
{
    u8 card, cursor, phone, page, pages, station, talkStep;
    u16 talkTimer, lastMinute, song;
    u16 talkSpecies;
    u8 talkPlace[24];
    u8 lines[12][40];   // the call’s pages, two lines each
    bool8 leaving, asking;
} *sG = NULL;
static EWRAM_DATA u8 sReturnCard = 0;
static EWRAM_DATA bool8 sToField = FALSE;   // (from the BAG or SELECT: back to the field, not to START)

static void DrawAll(void);

// ---------------------------------------------------------------------------------------------------- drawing
static void Print(u8 win, const u8 *text, u8 x, u8 y, const u8 *colors)
{
    AddTextPrinterParameterized3(win, FONT_NORMAL, x, y, colors, TEXT_SKIP_DRAW, text);
}

// the screen's rim, around the card
static void DrawRim(void)
{
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(C_EDGE), 0, 0, 224, 3);
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(C_EDGE), 0, 93, 224, 3);
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(C_EDGE), 0, 0, 3, 96);
    FillWindowPixelRect(WIN_BODY, PIXEL_FILL(C_EDGE), 221, 0, 3, 96);
}

static void DrawTabs(void)
{
    u8 i;
    FillWindowPixelBuffer(WIN_TABS, PIXEL_FILL(C_TAB));
    for (i = 0; i < CARD_COUNT; i++)
    {
        if (i == sG->card)
            FillWindowPixelRect(WIN_TABS, PIXEL_FILL(C_TABSEL), i * 56, 0, 54, 16);
        Print(WIN_TABS, sText_Cards[i], i * 56 + 10, 0, i == sG->card ? sColTabSel : sColTab);
    }
    CopyWindowToVram(WIN_TABS, COPYWIN_FULL);
}

static void DrawHint(const u8 *text)
{
    FillWindowPixelBuffer(WIN_HINT, PIXEL_FILL(C_FRAME));
    Print(WIN_HINT, text, 8, 0, sColHint);
    CopyWindowToVram(WIN_HINT, COPYWIN_FULL);
}

static const u8 *RegionName(void)
{
    return MP_Region_Current() == REGION_KANTO ? sText_Kanto : sText_Johto;
}

static void DrawClock(void)
{
    u8 buf[24], *p;
    u8 hour;
    RtcCalcLocalTime();
    hour = gLocalTime.hours % 12 ? gLocalTime.hours % 12 : 12;
    Print(WIN_BODY, sText_Days[gLocalTime.days % 7], 72, 20, sColInk);
    p = ConvertIntToDecimalStringN(buf, hour, STR_CONV_MODE_LEFT_ALIGN, 2);
    *p++ = CHAR_COLON;
    p = ConvertIntToDecimalStringN(p, gLocalTime.minutes, STR_CONV_MODE_LEADING_ZEROS, 2);
    StringCopy(p, gLocalTime.hours < 12 ? sText_Am : sText_Pm);
    Print(WIN_BODY, buf, 72, 40, sColInk);
    Print(WIN_BODY, RegionName(), 72, 60, sColInk);
    sG->lastMinute = gLocalTime.minutes;
}

static void DrawMap(void)
{
    Print(WIN_BODY, sText_TownMap, 80, 28, sColInk);
    Print(WIN_BODY, RegionName(), 80, 48, sColInk);
}

static void DrawPhone(void)
{
    if (sG->phone == PHONE_LIST)
    {
        Print(WIN_BODY, sText_Mom, 32, 12, sColInk);
        Print(WIN_BODY, sText_Elm, 32, 32, sColInk);
        Print(WIN_BODY, gText_SelectorArrow2, 16, 12 + sG->cursor * 20, sColInk);
        return;
    }
    Print(WIN_BODY, sG->cursor == 0 ? sText_Mom : sText_Elm, 12, 6, sColInk);
    Print(WIN_BODY, sG->lines[sG->page * 2], 12, 34, sColInk);
    Print(WIN_BODY, sG->lines[sG->page * 2 + 1], 12, 50, sColInk);
}

static void DrawRadio(void)
{
    Print(WIN_BODY, sText_Talk, 28, 6, sColInk);
    Print(WIN_BODY, sText_Flute, 28, 22, sColInk);
    Print(WIN_BODY, sText_RadioOff, 28, 38, sColInk);
    Print(WIN_BODY, gText_SelectorArrow2, 14, 6 + sG->cursor * 16, sColInk);
    if (sG->station == 1)   // OAK’s POKéMON TALK: what MARY and the PROF. say now
    {
        Print(WIN_BODY, sG->lines[0], 12, 58, sColInk);
        Print(WIN_BODY, sG->lines[1], 12, 74, sColInk);
    }
}

static void DrawAll(void)
{
    FillWindowPixelBuffer(WIN_BODY, PIXEL_FILL(C_SCREEN));
    DrawRim();
    switch (sG->card)
    {
    case CARD_CLOCK: DrawClock(); DrawHint(sText_HintClock); break;
    case CARD_MAP: DrawMap(); DrawHint(sText_HintMap); break;
    case CARD_PHONE:
        DrawPhone();
        DrawHint(sG->phone == PHONE_LIST ? sText_HintPhone : sG->phone == PHONE_ASK && sG->page == sG->pages - 1 ? sText_HintAsk : sText_HintCall);
        break;
    case CARD_RADIO: DrawRadio(); DrawHint(sText_HintRadio); break;
    }
    CopyWindowToVram(WIN_BODY, COPYWIN_FULL);
    DrawTabs();
}

// ---------------------------------------------------------------------------------------------------- calls
// Crystal’s words: each string a line of the call (two to a page); {PLAYER} and the buffers filled first
static void SetLines(const u8 *const *lines, u8 count)
{
    u8 i;
    memset(sG->lines, EOS, sizeof(sG->lines));
    for (i = 0; i < count && i < ARRAY_COUNT(sG->lines); i++)
        StringExpandPlaceholders(sG->lines[i], lines[i]);
    sG->pages = (count + 1) / 2;
    sG->page = 0;
}

static const u8 sMom_Hello1[] = _("Hello?");
static const u8 sMom_Hello2[] = _("Oh, hi, {PLAYER}! Working hard?");
static const u8 sMom_Where1[] = _("Really, you’re in {STR_VAR_1}?");
static const u8 sMom_Where2[] = _("I’ve never gone there. That’s");
static const u8 sMom_Where3[] = _("kind of neat, {PLAYER}.");
static const u8 sMom_Home1[] = _("What? You’re in {STR_VAR_1}?");
static const u8 sMom_Home2[] = _("Come see your MOM sometime!");
static const u8 sMom_Saved1[] = _("By the way, you’ve saved up");
static const u8 sMom_Saved2[] = _("¥{STR_VAR_2}. Do you want to keep on saving?");
static const u8 sMom_NotSaving1[] = _("Oh, {PLAYER}, you’re not saving any");
static const u8 sMom_NotSaving2[] = _("money. Would you like to save?");
static const u8 sMom_Bye1[] = _("{PLAYER}, keep it up! I’m rooting");
static const u8 sMom_Bye2[] = _("for you, baby!");
static const u8 sMom_Important[] = _("It’s important to save your money.");
static const u8 sMom_WontSave[] = _("OK. I won’t save your money.");
static const u8 sMom_WillSave[] = _("OK. I’ll save your money.");
static const u8 sEmpty[] = _("");

static const u8 sElm_Heal1[] = _("Hello, {PLAYER}?");
static const u8 sElm_Heal2[] = _("Try not to overdo it.");
static const u8 sElm_Heal3[] = _("Be sure to heal your MOéMON if");
static const u8 sElm_Heal4[] = _("they are hurt.");
static const u8 sElm_Hatch1[] = _("Hello, {PLAYER}? I’ve discovered");
static const u8 sElm_Hatch2[] = _("something about MOéMON EGGS!");
static const u8 sElm_Hatch3[] = _("It seems that how long an EGG takes");
static const u8 sElm_Hatch4[] = _("to hatch depends on the MOéMON.");
static const u8 sElm_Moves1[] = _("Hello, {PLAYER}? I’m investigating");
static const u8 sElm_Moves2[] = _("MOéMON EGGS right now.");
static const u8 sElm_Moves3[] = _("It seems that some MOéMON hatch");
static const u8 sElm_Moves4[] = _("knowing moves their parents knew.");

extern bool8 MP_MomBank_Saving(void);
extern bool8 MP_MomBank_Asked(void);
extern void MP_MomBank_SetSaving(bool8 on);
extern u32 MP_MomBank_Money(void);

static void CallMom(void)
{
    const u8 *lines[8];
    u8 n = 0;
    u16 home = MP_Region_Current() == REGION_JOHTO
            && Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum)->regionMapSectionId
               == Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_JOHTO_NEW_BARK_TOWN), MAP_NUM(MAP_JOHTO_NEW_BARK_TOWN))->regionMapSectionId;
    GetMapName(gStringVar1, gMapHeader.regionMapSectionId, 0);
    ConvertIntToDecimalStringN(gStringVar2, MP_MomBank_Money(), STR_CONV_MODE_LEFT_ALIGN, 6);
    lines[n++] = sMom_Hello1;
    lines[n++] = sMom_Hello2;
    if (home)
    {
        lines[n++] = sMom_Home1;
        lines[n++] = sMom_Home2;
    }
    else
    {
        lines[n++] = sMom_Where1;
        lines[n++] = sMom_Where2;
        lines[n++] = sMom_Where3;
        lines[n++] = sEmpty;
    }
    if (MP_MomBank_Asked())
    {
        lines[n++] = MP_MomBank_Saving() ? sMom_Saved1 : sMom_NotSaving1;
        lines[n++] = MP_MomBank_Saving() ? sMom_Saved2 : sMom_NotSaving2;
        sG->phone = PHONE_ASK;
    }
    else
    {
        lines[n++] = sMom_Bye1;
        lines[n++] = sMom_Bye2;
        sG->phone = PHONE_CALL;
    }
    SetLines(lines, n);
}

static void MomAnswer(bool8 yes)
{
    const u8 *lines[4];
    bool8 wasSaving = MP_MomBank_Saving();
    MP_MomBank_SetSaving(yes);   // (saving: YES keeps on, NO stops; not saving: YES starts)
    lines[0] = wasSaving ? (yes ? sMom_Important : sMom_WontSave) : (yes ? sMom_WillSave : sMom_WontSave);
    lines[1] = sEmpty;
    lines[2] = sMom_Bye1;
    lines[3] = sMom_Bye2;
    SetLines(lines, 4);
    sG->phone = PHONE_CALL;
}

static void CallElm(void)
{
    static const u8 *const heal[] = {sElm_Heal1, sElm_Heal2, sElm_Heal3, sElm_Heal4};
    static const u8 *const hatch[] = {sElm_Hatch1, sElm_Hatch2, sElm_Hatch3, sElm_Hatch4};
    static const u8 *const moves[] = {sElm_Moves1, sElm_Moves2, sElm_Moves3, sElm_Moves4};
    // Crystal’s ELM talks about the EGG he studies once you’ve shown him what hatched; here, once JOHTO’s second
    // BADGE is won (Quetzal’s story has no TOGEPI EGG)
    if (MP_Region_BadgeCount(REGION_JOHTO) < 2)
        SetLines(heal, 4);
    else
        SetLines(Random() % 2 ? hatch : moves, 4);
    sG->phone = PHONE_CALL;
}

// ---------------------------------------------------------------------------------------------------- radio
static const u8 sOpt_Intro1[] = _("MARY: PROF. OAK’S MOéMON TALK!");
static const u8 sOpt_Intro2[] = _("With me, MARY!");
static const u8 sOpt_Oak1[] = _("OAK: {STR_VAR_1}");
static const u8 sOpt_Oak2[] = _("may be seen around {STR_VAR_2}.");
static const u8 sOpt_Mary1[] = _("MARY: {STR_VAR_1}’s");
static const u8 sOpt_Adverbs[][20] = {
    _("sweet and adorably"),
    _("wiggly and slickly"),
    _("aptly named and"),
    _("undeniably kind of"),
    _("so, so unbearably"),
    _("wow, impressively"),
    _("almost poisonously"),
    _("ooh, so sensually"),
    _("so mischievously"),
    _("so very topically"),
    _("sure addictively"),
    _("looks in water is"),
    _("evolution must be"),
    _("provocatively"),
    _("so flipped out and"),
    _("heart-meltingly"),
};
static const u8 sOpt_Adjectives[][20] = {
    _("cute."),
    _("weird."),
    _("pleasant."),
    _("bold, sort of."),
    _("frightening."),
    _("suave & debonair!"),
    _("powerful."),
    _("exciting."),
    _("groovy!"),
    _("inspiring."),
    _("friendly."),
    _("hot, hot, hot!"),
    _("stimulating."),
    _("guarded."),
    _("lovely."),
    _("speedy."),
};

// a POKéMON of the region’s grass and where (its wild tables, as Crystal’s show picks a route’s)
static void PickTalkMon(void)
{
    u32 n = 0, i, pick;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        if (gWildMonHeaders[i].landMonsInfo != NULL && MP_Region_OfGroup(gWildMonHeaders[i].mapGroup) == MP_Region_Current())
            n++;
    sG->talkSpecies = SPECIES_NONE;
    if (n == 0)
        return;
    pick = Random() % n;
    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildPokemonHeader *h = &gWildMonHeaders[i];
        if (h->landMonsInfo == NULL || MP_Region_OfGroup(h->mapGroup) != MP_Region_Current() || pick-- != 0)
            continue;
        sG->talkSpecies = MP_Region_WildInfo(h->landMonsInfo)->wildPokemon[Random() % 12].species;   // (as the GENERATION has it)
        GetMapName(sG->talkPlace, Overworld_GetMapHeaderByGroupAndId(h->mapGroup, h->mapNum)->regionMapSectionId, 0);
        return;
    }
}

// the show’s lines now: the greeting, then OAK and MARY in turn, a new POKéMON each time round
static void TalkLines(void)
{
    const u8 *a = sEmpty, *b = sEmpty;
    switch (sG->talkStep % 4)
    {
    case 0: a = sOpt_Intro1; b = sOpt_Intro2; break;
    case 1:
        PickTalkMon();
        GetSpeciesName(gStringVar1, sG->talkSpecies);
        StringCopy(gStringVar2, sG->talkPlace);
        a = sOpt_Oak1; b = sOpt_Oak2;
        break;
    case 2:
        GetSpeciesName(gStringVar1, sG->talkSpecies);
        a = sOpt_Mary1; b = sOpt_Adverbs[Random() % ARRAY_COUNT(sOpt_Adverbs)];
        break;
    case 3:
        a = sOpt_Adjectives[Random() % ARRAY_COUNT(sOpt_Adjectives)];
        break;
    }
    StringExpandPlaceholders(sG->lines[0], a);
    StringExpandPlaceholders(sG->lines[1], b);
}

static void Tune(u8 station)
{
    static const u16 sSongs[] = {0, MUS_JOHTO_PROF_OAK, MUS_RG_POKE_FLUTE};
    sG->station = station;
    sG->talkStep = 0;
    sG->talkTimer = 0;
    if (station == 0)
        PlayBGM(sG->song);
    else
        PlayBGM(sSongs[station]);
    if (station == 1)
        TalkLines();
}

// ---------------------------------------------------------------------------------------------------- the screen
static void VBlankCB_Pokegear(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Pokegear(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void CB2_OpenPokegearAgain(void);

static void Leave(u8 taskId, MainCallback next)
{
    sG->leaving = TRUE;
    gTasks[taskId].data[0] = 0;
    SetWordTaskArg(taskId, 1, (uintptr_t)next);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}

static void Task_Pokegear(u8 taskId)
{
    if (sG->leaving)
    {
        if (!gPaletteFade.active)
        {
            MainCallback next = (MainCallback)GetWordTaskArg(taskId, 1);
            if (sG->station != 0)
                PlayBGM(sG->song);
            DestroyTask(taskId);
            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sG);
            SetMainCallback2(next);
        }
        return;
    }
    if (gPaletteFade.active)
        return;
    if (sG->card == CARD_CLOCK && (RtcCalcLocalTime(), gLocalTime.minutes != sG->lastMinute))
        DrawAll();
    if (sG->card == CARD_RADIO && sG->station == 1 && ++sG->talkTimer >= 150)   // (a line every two and a half seconds)
    {
        sG->talkTimer = 0;
        sG->talkStep++;
        TalkLines();
        DrawAll();
    }

    if (sG->card == CARD_PHONE && sG->phone != PHONE_LIST)   // a call: A turns the page; at MOM’s question A / B
    {
        bool8 last = sG->page >= sG->pages - 1;
        if (sG->phone == PHONE_ASK && last && (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON)))
        {
            PlaySE(SE_SELECT);
            MomAnswer(JOY_NEW(A_BUTTON));
            DrawAll();
        }
        else if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (last)
                sG->phone = PHONE_LIST;
            else
                sG->page++;
            DrawAll();
        }
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sReturnCard = 0;
        Leave(taskId, sToField ? CB2_ReturnToField : CB2_ReturnToFieldWithOpenMenu);
    }
    else if (JOY_NEW(DPAD_LEFT) || JOY_NEW(DPAD_RIGHT))
    {
        PlaySE(SE_SELECT);
        sG->card = (sG->card + (JOY_NEW(DPAD_RIGHT) ? 1 : CARD_COUNT - 1)) % CARD_COUNT;
        sG->cursor = 0;
        sG->phone = PHONE_LIST;
        if (sG->station != 0)
            Tune(0);   // (the radio plays while its card is open)
        DrawAll();
    }
    else if (JOY_NEW(DPAD_UP) && sG->cursor > 0)
        sG->cursor--, PlaySE(SE_SELECT), DrawAll();
    else if (JOY_NEW(DPAD_DOWN) && ((sG->card == CARD_PHONE && sG->cursor < 1) || (sG->card == CARD_RADIO && sG->cursor < 2)))
        sG->cursor++, PlaySE(SE_SELECT), DrawAll();
    else if (JOY_NEW(A_BUTTON))
    {
        switch (sG->card)
        {
        case CARD_MAP:
            PlaySE(SE_SELECT);
            sReturnCard = CARD_MAP;
            Leave(taskId, CB2_OpenPokegearAgain);
            break;
        case CARD_PHONE:
            PlaySE(SE_POKENAV_CALL);
            if (sG->cursor == 0)
                CallMom();
            else
                CallElm();
            DrawAll();
            break;
        case CARD_RADIO:
            PlaySE(SE_SELECT);
            Tune(sG->cursor == 2 ? 0 : sG->cursor + 1);
            DrawAll();
            break;
        }
    }
}

static void OpenPokegear(u8 card)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sWindows);
    DeactivateAllTextPrinters();
    LoadPalette(sPalette, BG_PLTT_ID(15), sizeof(sPalette));
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    sG = AllocZeroed(sizeof(*sG));
    sG->card = card;
    sG->song = GetCurrentMapMusic();
    LoadPalette(&sPalette[C_FRAME], BG_PLTT_ID(0), sizeof(u16));   // (the backdrop: the device's frame)
    PutWindowTilemap(WIN_TABS);
    PutWindowTilemap(WIN_BODY);
    PutWindowTilemap(WIN_HINT);
    DrawAll();
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    ShowBg(0);
    ShowBg(1);
    CreateTask(Task_Pokegear, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Pokegear);
    SetMainCallback2(CB2_Pokegear);
}

static void CB2_BackToPokegear(void)
{
    OpenPokegear(sReturnCard);
}

static void CB2_OpenPokegearAgain(void)
{
    FieldInitRegionMap(CB2_BackToPokegear);   // the MAP card’s TOWN MAP, then the POKéGEAR again
}

// START > POKéGEAR (start_menu.c) and the key item (item_use.c)
void CB2_MP_OpenPokegear(void)
{
    sToField = FALSE;
    OpenPokegear(CARD_CLOCK);
}

void CB2_MP_OpenPokegearToField(void)
{
    sToField = TRUE;
    OpenPokegear(CARD_CLOCK);
}

// Tests (harness condition "pokegear"): the screen is up and settled
bool8 MP_Pokegear_Active(void)
{
    return sG != NULL && !sG->leaving && !gPaletteFade.active;
}

// Tests (mp pokegear): the card, the cursor and the phone / radio state
void MP_Pokegear_Debug(void)
{
    if (sG == NULL)
        printf("[test] pokegear: closed\n");
    else
        printf("[test] pokegear: card %u cursor %u phone %u page %u/%u station %u\n", sG->card, sG->cursor, sG->phone,
               sG->page, sG->pages, sG->station);
}
