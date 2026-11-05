// Moemon Kai: BUENA's PASSWORD (audit case 141): Crystal's engine/events/buena.asm, buena_menu.asm,
// data/radio/buenas_passwords.asm and data/items/buena_prizes.asm. The radio show (mp_pokegear.c, 6 PM to midnight)
// picks the day's password and says it; BUENA on RADIO TOWER 2F asks for it once a day (one point on the BLUE CARD,
// up to 30); the receptionist beside her trades points for Crystal's prizes. Scripts: data/maps/Johto_RadioTower2F.
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "field_specials.h"
#include "item.h"
#include "menu.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/field_specials.h"
#include "constants/items.h"
#include "constants/kai_regions.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"

#define BUENA_POINT_CAP 30   // (BLUE_CARD_POINT_CAP)

enum { BUENA_MON, BUENA_ITEM, BUENA_MOVE, BUENA_STRING };

static const u8 sNewBark[] = _("NEW BARK TOWN");
static const u8 sCherrygrove[] = _("CHERRYGROVE CITY");
static const u8 sAzalea[] = _("AZALEA TOWN");
static const u8 sFlying[] = _("FLYING");
static const u8 sBug[] = _("BUG");
static const u8 sGrass[] = _("GRASS");
static const u8 sTalk[] = _("MOéMON Talk");
static const u8 sMusic[] = _("MOéMON Music");
static const u8 sLucky[] = _("Lucky Channel");

// BuenasPasswordTable: the kind, then three choices (Crystal's points per category go unused there too: one point a
// right answer)
static const struct { u8 kind; u16 id[3]; const u8 *str[3]; } sPasswords[] = {
    {BUENA_MON,    {SPECIES_CYNDAQUIL, SPECIES_TOTODILE, SPECIES_CHIKORITA}},
    {BUENA_ITEM,   {ITEM_FRESH_WATER, ITEM_SODA_POP, ITEM_LEMONADE}},
    {BUENA_ITEM,   {ITEM_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL}},
    {BUENA_ITEM,   {ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ULTRA_BALL}},
    {BUENA_MON,    {SPECIES_PIKACHU, SPECIES_RATTATA, SPECIES_GEODUDE}},
    {BUENA_MON,    {SPECIES_HOOTHOOT, SPECIES_SPINARAK, SPECIES_DROWZEE}},
    {BUENA_STRING, {0}, {sNewBark, sCherrygrove, sAzalea}},
    {BUENA_STRING, {0}, {sFlying, sBug, sGrass}},
    {BUENA_MOVE,   {MOVE_TACKLE, MOVE_GROWL, MOVE_MUD_SLAP}},
    {BUENA_ITEM,   {ITEM_X_ATTACK, ITEM_X_DEFEND, ITEM_X_SPEED}},
    {BUENA_STRING, {0}, {sTalk, sMusic, sLucky}},
};

// BuenaPrizeItems: the prize and its points
static const u16 sPrizes[][2] = {
    {ITEM_ULTRA_BALL, 2}, {ITEM_FULL_RESTORE, 2}, {ITEM_NUGGET, 3}, {ITEM_RARE_CANDY, 3}, {ITEM_PROTEIN, 5},
    {ITEM_IRON, 5}, {ITEM_CARBOS, 5}, {ITEM_CALCIUM, 5}, {ITEM_HP_UP, 5},
};

static void Choice(u8 *dest, u32 cat, u32 n)
{
    u16 id = sPasswords[cat].id[n];
    switch (sPasswords[cat].kind)
    {
    case BUENA_MON:  StringCopy(dest, gSpeciesNames[id]); break;
    case BUENA_ITEM: CopyItemName(id, dest); break;
    case BUENA_MOVE: StringCopy(dest, gMoveNames[id]); break;
    default:         StringCopy(dest, sPasswords[cat].str[n]); break;
    }
}

// The radio's BuenasPassword4: today's password, picked once a day (FLAG_DAILY_JOHTO_BUENA_PASSWORD, Crystal's
// DAILYFLAGS2_BUENAS_PASSWORD_F, which BUENA also asks for: you've heard the show today)
void MP_Buena_TodaysPassword(u8 *dest)
{
    u16 pw;
    if (!FlagGet(FLAG_DAILY_JOHTO_BUENA_PASSWORD))
    {
        VarSet(VAR_JOHTO_BUENA_PASSWORD, ((Random() % ARRAY_COUNT(sPasswords)) << 4) | (Random() % 3));
        FlagSet(FLAG_DAILY_JOHTO_BUENA_PASSWORD);
    }
    pw = VarGet(VAR_JOHTO_BUENA_PASSWORD);
    Choice(dest, (pw >> 4) % ARRAY_COUNT(sPasswords), (pw & 0xF) % 3);
}

// STR_VAR_1..3: today's three choices (MULTI_BUENA_PASSWORD)
u16 MP_Buena_BufferChoices(void)
{
    u16 pw = VarGet(VAR_JOHTO_BUENA_PASSWORD);
    u32 cat = (pw >> 4) % ARRAY_COUNT(sPasswords);
    Choice(gStringVar1, cat, 0);
    Choice(gStringVar2, cat, 1);
    Choice(gStringVar3, cat, 2);
    return 0;
}

// VAR_RESULT (the row picked) is today's password: VAR_RESULT 1 and a point on the BLUE CARD
u16 MP_Buena_CheckAnswer(void)
{
    if (gSpecialVar_Result != (VarGet(VAR_JOHTO_BUENA_PASSWORD) & 0xF))
        return gSpecialVar_Result = 0;
    if (VarGet(VAR_JOHTO_BUENA_POINTS) < BUENA_POINT_CAP)
        VarSet(VAR_JOHTO_BUENA_POINTS, VarGet(VAR_JOHTO_BUENA_POINTS) + 1);
    return gSpecialVar_Result = 1;
}

// VAR_RESULT: 1 when the BLUE CARD is full (30 points)
u16 MP_Buena_CardFull(void)
{
    return gSpecialVar_Result = VarGet(VAR_JOHTO_BUENA_POINTS) >= BUENA_POINT_CAP;
}

// The receptionist's prize desk (Crystal's BuenaPrize): the points box, the list, the chosen prize
static EWRAM_DATA u8 sPointsWin = 0;
static EWRAM_DATA bool8 sPointsShown = FALSE;
static const u8 sText_Points[] = _("Points");

static void PrintPoints(void)
{
    u8 *s;
    FillWindowPixelBuffer(sPointsWin, PIXEL_FILL(1));
    s = StringCopy(gStringVar4, sText_Points);
    *s++ = CHAR_SPACE;
    ConvertIntToDecimalStringN(s, VarGet(VAR_JOHTO_BUENA_POINTS), STR_CONV_MODE_RIGHT_ALIGN, 2);
    AddTextPrinterParameterized(sPointsWin, FONT_NORMAL, gStringVar4, 0, 1, 0, NULL);
    CopyWindowToVram(sPointsWin, COPYWIN_FULL);
}

u16 MP_Buena_ShowPoints(void)
{
    struct WindowTemplate t;
    if (sPointsShown)
        return 0;
    SetWindowTemplateFields(&t, 0, 21, 1, 8, 2, 0xF, 0x141);   // (the top right: the prize list takes the left)
    sPointsWin = AddWindow(&t);
    PutWindowTilemap(sPointsWin);
    DrawStdFrameWithCustomTileAndPalette(sPointsWin, FALSE, 0x214, 0xE);
    sPointsShown = TRUE;
    PrintPoints();
    return 0;
}

u16 MP_Buena_HidePoints(void)
{
    if (!sPointsShown)
        return 0;
    ClearStdWindowAndFrame(sPointsWin, TRUE);
    RemoveWindow(sPointsWin);
    sPointsShown = FALSE;
    return 0;
}

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
static u8 sPrizeRows[ARRAY_COUNT(sPrizes)][24];
static const u8 sText_Cancel[] = _("CANCEL");

// the list of prizes and their points (the script waits); VAR_RESULT the row, MULTI_B_PRESSED / the last row: none
u16 MP_Buena_PrizeMenu(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sPrizes); i++)
    {
        u8 *s;
        CopyItemName(sPrizes[i][0], sPrizeRows[i]);
        s = sPrizeRows[i] + StringLength(sPrizeRows[i]);
        *s++ = CHAR_SPACE;
        *s++ = CHAR_SPACE;
        ConvertIntToDecimalStringN(s, sPrizes[i][1], STR_CONV_MODE_LEFT_ALIGN, 1);
        gQzScrollOptions[i] = sPrizeRows[i];
    }
    gQzScrollOptions[i] = sText_Cancel;
    gQzScrollCount = i + 1;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}

// VAR_RESULT the row picked: STR_VAR_1 the prize, VAR_0x8005 the item; VAR_RESULT 0 when it's not a prize
u16 MP_Buena_BufferPrize(void)
{
    if (gSpecialVar_Result >= ARRAY_COUNT(sPrizes))
        return gSpecialVar_Result = 0;
    gSpecialVar_0x8005 = sPrizes[gSpecialVar_Result][0];
    gSpecialVar_0x8006 = sPrizes[gSpecialVar_Result][1];
    CopyItemName(gSpecialVar_0x8005, gStringVar1);
    return gSpecialVar_Result = 1;
}

// VAR_0x8006 points off the card (VAR_RESULT 0 when there aren't enough)
u16 MP_Buena_Pay(void)
{
    u16 have = VarGet(VAR_JOHTO_BUENA_POINTS);
    if (have < gSpecialVar_0x8006)
        return gSpecialVar_Result = 0;
    VarSet(VAR_JOHTO_BUENA_POINTS, have - gSpecialVar_0x8006);
    if (sPointsShown)
        PrintPoints();
    return gSpecialVar_Result = 1;
}

// VAR_RESULT: 1 when the card has the points for VAR_0x8006
u16 MP_Buena_CanAfford(void)
{
    return gSpecialVar_Result = VarGet(VAR_JOHTO_BUENA_POINTS) >= gSpecialVar_0x8006;
}
