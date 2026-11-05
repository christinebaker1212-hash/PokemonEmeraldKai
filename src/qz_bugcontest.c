// Moemon Kai: JOHTO's Bug-Catching Contest in the NATIONAL PARK on Crystal's rules (audit case 97: pret/pokecrystal
// engine/events/bug_contest/*.asm, data/events/bug_contest_winners.asm; the scripts in
// data/maps/Johto_Route35NationalParkGate and Johto_NationalParkBugContest call these specials).
// TUESDAY, THURSDAY and SATURDAY, once a day; the lead Pokémon alone, 20 SAFARI BALLS for Crystal's PARK BALLS (the
// player's own are kept aside), 20 minutes; one catch is kept (a second asks to switch), judged by Crystal's score
// against five of Crystal's ten contestants; the catch is the player's at the end (party, or the PC).
// While it runs the party is the lead and the catches: SAVE and autosave are off (QzBugContest_Active), so a save
// never holds the party apart from the Pokémon kept here.
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "item.h"
#include "item_menu.h"
#include "main.h"
#include "mp.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "qz_specials.h"
#include "random.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "constants/event_object_movement.h"
#include "constants/items.h"
#include "constants/kai_regions.h"
#include "constants/map_groups.h"
#include "constants/qz_port.h"
#include "constants/species.h"

#define CONTEST_BALLS   20               // (Crystal's BUG_CONTEST_BALLS)
#define CONTEST_FRAMES  (20 * 60 * 60)   // 20 minutes (BUG_CONTEST_MINUTES)
#define CONTEST_CATCHES 1

enum { CONTEST_OFF, CONTEST_ON };

struct Contestant
{
    const u8 *name;
    u16 species;
    u16 score;
    bool8 shiny;
};

static EWRAM_DATA struct
{
    u8 state;
    u8 side;              // which gate they came in by (VAR_0x8004 at the start)
    u8 playerPlace;       // 1-3, or 4 (not placed)
    u16 ownSafariBalls;   // the player's own SAFARI BALLS, kept aside
    u32 startFrame;
    struct Pokemon party[PARTY_SIZE];   // the party as it was (the lead's own slot refreshed at the end)
    struct Contestant board[5];         // ranked after the judging (the player is one of them)
    u16 prize;
    u16 present;          // which of Crystal's ten contestants came today (SelectRandomBugContestContestants)
    bool8 keptMons;       // the officer kept the rest of the party (Crystal's EVENT_LEFT_MONS_WITH_CONTEST_OFFICER)
} sBug = {0};

// Crystal's ten contestants stand in the park (map.json objects from this local id on, in Crystal's order), the
// ones not here today hidden by FLAG_TEMP_11 + n
#define CONTESTANT_LOCALID 29

static const u8 sBugName0[] = _("BUG CATCHER DON");
static const u8 sBugName1[] = _("BUG CATCHER ED");
static const u8 sBugName2[] = _("COOLTRAINER NICK");
static const u8 sBugName3[] = _("POKéFAN WILLIAM");
static const u8 sBugName4[] = _("BUG CATCHER BENNY");
static const u8 sBugName5[] = _("CAMPER BARRY");
static const u8 sBugName6[] = _("PICNICKER CINDY");
static const u8 sBugName7[] = _("BUG CATCHER JOSH");
static const u8 sBugName8[] = _("YOUNGSTER SAMUEL");
static const u8 sBugName9[] = _("SCHOOLBOY KIPP");
static const u8 sBugName10[] = _("SUNDAY");
static const u8 sBugName11[] = _("MONDAY");
static const u8 sBugName12[] = _("TUESDAY");
static const u8 sBugName13[] = _("WEDNESDAY");
static const u8 sBugName14[] = _("THURSDAY");
static const u8 sBugName15[] = _("FRIDAY");
static const u8 sBugName16[] = _("SATURDAY");

// Crystal's BugContestantPointers: each one's three possible catches and their base scores
static const struct { const u8 *name; u16 species[3]; u16 score[3]; } sContestants[10] = {
    {sBugName0, {SPECIES_KAKUNA, SPECIES_METAPOD, SPECIES_CATERPIE}, {300, 285, 226}},
    {sBugName1, {SPECIES_BUTTERFREE, SPECIES_BUTTERFREE, SPECIES_CATERPIE}, {286, 251, 237}},
    {sBugName2, {SPECIES_SCYTHER, SPECIES_BUTTERFREE, SPECIES_PINSIR}, {357, 349, 368}},
    {sBugName3, {SPECIES_PINSIR, SPECIES_BUTTERFREE, SPECIES_VENONAT}, {332, 324, 321}},
    {sBugName4, {SPECIES_BUTTERFREE, SPECIES_WEEDLE, SPECIES_CATERPIE}, {318, 295, 285}},
    {sBugName5, {SPECIES_PINSIR, SPECIES_VENONAT, SPECIES_KAKUNA}, {366, 329, 314}},
    {sBugName6, {SPECIES_BUTTERFREE, SPECIES_METAPOD, SPECIES_CATERPIE}, {341, 301, 264}},
    {sBugName7, {SPECIES_SCYTHER, SPECIES_BUTTERFREE, SPECIES_METAPOD}, {326, 292, 282}},
    {sBugName8, {SPECIES_WEEDLE, SPECIES_PINSIR, SPECIES_CATERPIE}, {270, 282, 251}},
    {sBugName9, {SPECIES_VENONAT, SPECIES_PARAS, SPECIES_KAKUNA}, {267, 254, 259}},
};

// five of the ten, at random
static void PickContestants(void)
{
    u32 n = 0;
    sBug.present = 0;
    while (n < 5)
    {
        u32 k = Random() % 10;
        if (!(sBug.present & (1 << k)))
            sBug.present |= 1 << k, n++;
    }
}

extern const u8 EventScript_QzBugContestOver[];
extern const u8 EventScript_QzBugContestNoBalls[];
extern const u8 EventScript_QzBugContestLeft[];
extern const u8 EventScript_QzBugContestSwitch[];

extern char *getenv(const char *name);
extern int atoi(const char *s);

static u32 ContestFrames(void)
{
    const char *v = getenv("MOEMON_BUGCONTEST_FRAMES");   // tests: a shorter contest
    return v ? (u32)atoi(v) : CONTEST_FRAMES;
}

bool8 QzBugContest_Active(void)
{
    return sBug.state == CONTEST_ON;
}

static bool8 OnContestMap(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_JOHTO_NATIONAL_PARK_BUG_CONTEST);
}

static u8 Catches(void)
{
    u8 n = CalculatePlayerPartyCount();
    return n > 1 ? n - 1 : 0;
}

static u32 FreePcSlots(void)
{
    struct PokemonStorage *pc = MP_PersonalStorage();
    u32 b, i, n = 0;
    if (pc == NULL)
        return 0;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (i = 0; i < IN_BOX_COUNT; i++)
            if (GetBoxMonData(&pc->boxes[b][i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
                n++;
    return n;
}

// Crystal's ContestScore: max HP x 4, the other five stats, a few DV bits (Gen 3's IVs / 2), remaining HP / 8 and
// one for a held item
static u16 MonScore(struct Pokemon *mon)
{
    u32 score = 0, i;
    u8 dv[NUM_STATS];
    if (GetMonData(mon, MON_DATA_SPECIES, NULL) == SPECIES_NONE)
        return 0;
    for (i = 0; i < NUM_STATS; i++)
        dv[i] = GetMonData(mon, MON_DATA_HP_IV + i, NULL) >> 1;
    score += 4 * (GetMonData(mon, MON_DATA_MAX_HP, NULL) & 0xFF);
    score += GetMonData(mon, MON_DATA_ATK, NULL) & 0xFF;
    score += GetMonData(mon, MON_DATA_DEF, NULL) & 0xFF;
    score += GetMonData(mon, MON_DATA_SPEED, NULL) & 0xFF;
    score += GetMonData(mon, MON_DATA_SPATK, NULL) & 0xFF;
    score += GetMonData(mon, MON_DATA_SPDEF, NULL) & 0xFF;
    // (Crystal: bit 1 of each DV: ATK x 8, DEF x 16, SPEED x 1, SPECIAL x 4)
    score += ((dv[STAT_ATK] & 2) ? 8 : 0) + ((dv[STAT_DEF] & 2) ? 16 : 0) + ((dv[STAT_SPEED] & 2) ? 1 : 0)
           + ((dv[STAT_SPATK] & 2) ? 4 : 0);
    score += (GetMonData(mon, MON_DATA_HP, NULL) & 0xFF) / 8;
    if (GetMonData(mon, MON_DATA_HELD_ITEM, NULL) != ITEM_NONE)
        score += 1;
    return score;
}

// VAR_0x8004: the gate side. VAR_RESULT: 0 entered, 1 the lead can't battle, 2 no room in the PC for five,
// 3 no room for the SAFARI BALLS, 4 (Nuzlocke, never here)
u16 QzBugContestStart(void)
{
    struct Pokemon *lead = &gPlayerParty[0];
    u32 i;
    if (GetMonData(lead, MON_DATA_SPECIES, NULL) == SPECIES_NONE || GetMonData(lead, MON_DATA_IS_EGG, NULL)
     || GetMonData(lead, MON_DATA_HP, NULL) == 0)
        return gSpecialVar_Result = 1;
    if (CalculatePlayerPartyCount() >= PARTY_SIZE && FreePcSlots() < CONTEST_CATCHES)   // (no room for the catch)
        return gSpecialVar_Result = 2;
    sBug.ownSafariBalls = CountTotalItemQuantityInBag(ITEM_SAFARI_BALL);
    if (sBug.ownSafariBalls)
        RemoveBagItem(ITEM_SAFARI_BALL, sBug.ownSafariBalls);
    if (!AddBagItem(ITEM_SAFARI_BALL, CONTEST_BALLS))
    {
        if (sBug.ownSafariBalls)
            AddBagItem(ITEM_SAFARI_BALL, sBug.ownSafariBalls);
        return gSpecialVar_Result = 3;
    }
    sBug.keptMons = gPlayerPartyCount > 1;
    memcpy(sBug.party, gPlayerParty, sizeof(sBug.party));
    for (i = 1; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
    CalculatePlayerPartyCount();
    sBug.side = gSpecialVar_0x8004 != 0;
    sBug.state = CONTEST_ON;
    sBug.startFrame = gMain.vblankCounter1;
    sBug.prize = ITEM_NONE;
    PickContestants();
    RtcCalcLocalTime();
    VarSet(VAR_JOHTO_BUG_CONTEST_DAY, gLocalTime.days + 1);
    VarSet(VAR_JOHTO_BUG_CONTEST_PRIZE, ITEM_NONE);
    MPNet_Log("bug contest: started (side %d, %d own SAFARI BALLS kept)", sBug.side, sBug.ownSafariBalls);
    return gSpecialVar_Result = 0;
}

// (the catch is kept for the end, QzBugContestEnd: the party, or the PC)
u16 QzBugContestReleaseCaught(void)
{
    return gSpecialVar_Result = 1;
}

// the party as it was, the player's own SAFARI BALLS back: VAR_RESULT 1 (2: the catch went to the PC, Crystal's
// BUGCONTEST_BOXED_MON), or 0 when the bag has no room for the SAFARI BALLS
u16 QzBugContestEnd(void)
{
    u16 left;
    bool32 boxed = FALSE;
    struct Pokemon caught;
    if (sBug.state != CONTEST_ON)
        return gSpecialVar_Result = 1;
    caught = gPlayerParty[1];
    left = CountTotalItemQuantityInBag(ITEM_SAFARI_BALL);
    if (left)
        RemoveBagItem(ITEM_SAFARI_BALL, left);
    if (sBug.ownSafariBalls && !AddBagItem(ITEM_SAFARI_BALL, sBug.ownSafariBalls))
    {
        if (left)
            AddBagItem(ITEM_SAFARI_BALL, left);
        return gSpecialVar_Result = 0;
    }
    // the rest of the party back; the lead keeps what it gained (Crystal's ContestReturnMons)
    sBug.party[0] = gPlayerParty[0];
    memcpy(gPlayerParty, sBug.party, sizeof(sBug.party));
    CalculatePlayerPartyCount();
    if (GetMonData(&caught, MON_DATA_SPECIES, NULL) != SPECIES_NONE)
    {
        // the catch is the player's (Crystal's CheckPartyFullAfterContest): the party, or the PC
        boxed = gPlayerPartyCount >= PARTY_SIZE;
        if (!boxed)
            gPlayerParty[gPlayerPartyCount] = caught;
        else
            MP_SendMonToPC(&caught);
        CalculatePlayerPartyCount();
    }
    sBug.state = CONTEST_OFF;
    MPNet_Log("bug contest: over, place %d%s", sBug.playerPlace, boxed ? " (the catch to the PC)" : "");
    return gSpecialVar_Result = boxed ? 2 : 1;
}

u16 QzBugContestGetCaughtCount(void)
{
    return gSpecialVar_Result = sBug.state == CONTEST_ON ? Catches() : 0;
}

// Crystal's BugContest_JudgeContestants: each contestant present today brings one of their three catches with its
// score plus 0-7; the player's catch is scored by ContestScore; the best three win
u16 QzBugContestJudge(void)
{
    u32 i, j, n = 0;
    struct Contestant all[11], tmp;
    all[n].name = gSaveBlock2Ptr->playerName;
    all[n].species = GetMonData(&gPlayerParty[1], MON_DATA_SPECIES, NULL);
    all[n].score = MonScore(&gPlayerParty[1]);
    all[n].shiny = all[n].species != SPECIES_NONE && IsMonShiny(&gPlayerParty[1]);
    n++;
    for (i = 0; i < 10; i++)
    {
        u32 pick = Random() % 3;
        if (!(sBug.present & (1 << i)))
            continue;
        all[n].name = sContestants[i].name;
        all[n].species = sContestants[i].species[pick];
        all[n].score = sContestants[i].score[pick] + (Random() & 7);
        all[n].shiny = FALSE;
        n++;
    }
    for (i = 0; i < n; i++)   // best first (a tie keeps the earlier one: the player, as in Crystal)
        for (j = i + 1; j < n; j++)
            if (all[j].score > all[i].score)
                tmp = all[i], all[i] = all[j], all[j] = tmp;
    sBug.playerPlace = 4;
    for (i = 0; i < 3 && i < n; i++)
        if (all[i].name == gSaveBlock2Ptr->playerName && all[i].species != SPECIES_NONE)
            sBug.playerPlace = i + 1;
    for (i = 0; i < 4; i++)
        sBug.board[i] = all[i];
    if (sBug.playerPlace == 4)   // the player's own line
        for (i = 0; i < n; i++)
            if (all[i].name == gSaveBlock2Ptr->playerName)
                sBug.board[3] = all[i];
    sBug.prize = sBug.playerPlace == 1 ? ITEM_SUN_STONE : sBug.playerPlace == 2 ? ITEM_EVERSTONE
               : sBug.playerPlace == 3 ? ITEM_SITRUS_BERRY : ITEM_ORAN_BERRY;   // (Crystal's GOLD BERRY / BERRY)
    MPNet_Log("bug contest: judged, player %d (score %d)", sBug.playerPlace, sBug.board[sBug.playerPlace - 1].score);
    if (sBug.playerPlace == 1)
    {
        void MP_Achieve_Unlock(const char *id);
        MP_Achieve_Unlock("jbugwin");   // Moemon Kai: BUG-CATCHING CHAMP (mp_achieve.c)
    }
    return gSpecialVar_Result = sBug.playerPlace;
}

// VAR_0x8004 = place 1-4: STR_VAR_1 the contestant, STR_VAR_2 the POKéMON, STR_VAR_3 the score; VAR_0x8005 = the
// player's prize
u16 QzBugContestBufferPlaceWinner(void)
{
    static const u8 sShiny[] = _("SHINY ");
    u8 place = gSpecialVar_0x8004 >= 1 && gSpecialVar_0x8004 <= 4 ? gSpecialVar_0x8004 : 4;
    struct Contestant *c = &sBug.board[place - 1];
    u8 *s;
    StringCopy(gStringVar1, c->name ? c->name : gSaveBlock2Ptr->playerName);
    s = gStringVar2;
    if (c->shiny)
        s = StringCopy(s, sShiny);
    StringCopy(s, gSpeciesNames[c->species < NUM_SPECIES ? c->species : SPECIES_NONE]);
    ConvertIntToDecimalStringN(gStringVar3, c->score, STR_CONV_MODE_LEFT_ALIGN, 4);
    gSpecialVar_0x8005 = sBug.prize;
    return gSpecialVar_Result = 1;
}

// VAR_RESULT the player's place (4: none); VAR_0x8005 the prize; STR_VAR_1 the place, STR_VAR_2 the prize's name
u16 QzBugContestGetPlayerPlace(void)
{
    gSpecialVar_0x8005 = sBug.prize;
    ConvertIntToDecimalStringN(gStringVar1, sBug.playerPlace, STR_CONV_MODE_LEFT_ALIGN, 1);
    CopyItemName(sBug.prize, gStringVar2);
    return gSpecialVar_Result = sBug.playerPlace;
}

u16 QzBugContestGetFlag2(void) { return gSpecialVar_Result = sBug.keptMons; }   // the officer kept the party
u16 QzBugContestGetFlag3(void) { return gSpecialVar_Result = sBug.side; }  // which gate to go back to
u16 QzBugContestClearFlag1(void) { return 0; }

// VAR_RESULT 1 when the player's own SAFARI BALLS will fit back in the bag
u16 QzBugContestCheckState(void)
{
    if (sBug.state != CONTEST_ON || !sBug.ownSafariBalls)
        return gSpecialVar_Result = 1;
    return gSpecialVar_Result = CheckBagHasSpace(ITEM_SAFARI_BALL, sBug.ownSafariBalls + CountTotalItemQuantityInBag(ITEM_SAFARI_BALL));
}

static void CB2_BugContestBag(void)
{
    GoToBagMenu(ITEMMENULOCATION_FIELD, POCKETS_COUNT, CB2_ReturnToFieldContinueScript);
}

// the bag, to make room (the script waits)
u16 QzBugContestChooseFromBag(void)
{
    SetMainCallback2(CB2_BugContestBag);
    return 0;
}

// The field each frame (DoCB1_Overworld, controls free): a second catch asks to switch; the contest ends when time
// is up or with no SAFARI BALL left; leaving the park ends it at once. TRUE: a script was started
bool8 QzBugContest_FieldCheck(void)
{
    if (sBug.state != CONTEST_ON)
        return FALSE;
    if (!OnContestMap())
    {
        // out of the park some other way (the gates' officers stand at its ways out): the catch kept, the party
        // back, the contest over
        QzBugContestReleaseCaught();
        QzBugContestEnd();
        if (sBug.state == CONTEST_ON)
        {
            // the bag can't take the player's own SAFARI BALLS back: they're lost rather than the party
            sBug.ownSafariBalls = 0;
            QzBugContestEnd();
        }
        VarSet(VAR_QZ_0x5019, 0);
        ScriptContext_SetupScript(EventScript_QzBugContestLeft);
        return TRUE;
    }
    if (VarGet(VAR_QZ_0x5019) != 2)
        return FALSE;
    if (Catches() > CONTEST_CATCHES)
    {
        // Crystal's BugContest_SetCaughtContestMon: "You already caught a ... Switch POKéMON?"
        StringCopy(gStringVar2, gSpeciesNames[GetMonData(&gPlayerParty[1], MON_DATA_SPECIES, NULL)]);
        ScriptContext_SetupScript(EventScript_QzBugContestSwitch);
        return TRUE;
    }
    if (gMain.vblankCounter1 - sBug.startFrame >= ContestFrames() || !CheckBagHasItem(ITEM_SAFARI_BALL, 1))
    {
        bool32 timeUp = gMain.vblankCounter1 - sBug.startFrame >= ContestFrames();
        MPNet_Log("bug contest: %s (%u frames, %d catches)", timeUp ? "time up" : "no SAFARI BALLS left",
                  gMain.vblankCounter1 - sBug.startFrame, Catches());
        ScriptContext_SetupScript(timeUp ? EventScript_QzBugContestOver : EventScript_QzBugContestNoBalls);
        return TRUE;
    }
    return FALSE;
}

// VAR_RESULT 1: the new catch takes the kept one's place; 0: the new one goes back to the wild
u16 QzBugContestSwitch(void)
{
    if (gSpecialVar_Result)
        gPlayerParty[1] = gPlayerParty[2];
    ZeroMonData(&gPlayerParty[2]);
    CalculatePlayerPartyCount();
    return 0;
}

// TUESDAY, THURSDAY, SATURDAY: VAR_RESULT 1 (and STR_VAR_1 the day), 2 when today's contest was already entered
u16 QzBugContestDay(void)
{
    static const u8 *const sDays[7] = {sBugName10, sBugName11, sBugName12,
        sBugName13, sBugName14, sBugName15, sBugName16};
    u16 MP_JohtoWeekday(void);
    u16 day = MP_JohtoWeekday();
    RtcCalcLocalTime();
    StringCopy(gStringVar1, sDays[day % 7]);
    if (day != 2 && day != 4 && day != 6)
        return gSpecialVar_Result = 0;
    if (VarGet(VAR_JOHTO_BUG_CONTEST_DAY) == (u16)(gLocalTime.days + 1))
        return gSpecialVar_Result = 2;
    return gSpecialVar_Result = 1;
}

// STR_VAR_1: the lead POKéMON's name (Crystal's "You'll have to use ..., the first POKéMON")
u16 QzBugContestBufferLead(void)
{
    GetMonData(&gPlayerParty[0], MON_DATA_NICKNAME, gStringVar1);
    StringGet_Nickname(gStringVar1);
    return gSpecialVar_Result = CalculatePlayerPartyCount();
}

// before the start (Crystal's order): VAR_RESULT 2 both the party and the PC full, 5 an EGG first, 1 the first
// POKéMON can't battle, 0 it can go
u16 QzBugContestCanEnter(void)
{
    struct Pokemon *lead = &gPlayerParty[0];
    if (CalculatePlayerPartyCount() >= PARTY_SIZE && FreePcSlots() < CONTEST_CATCHES)
        return gSpecialVar_Result = 2;
    if (GetMonData(lead, MON_DATA_IS_EGG, NULL))
        return gSpecialVar_Result = 5;
    if (GetMonData(lead, MON_DATA_SPECIES, NULL) == SPECIES_NONE || GetMonData(lead, MON_DATA_HP, NULL) == 0)
        return gSpecialVar_Result = 1;
    return gSpecialVar_Result = 0;
}

// STR_VAR_1: the minutes left, as Crystal says them (VAR_CONTESTMINUTES + 1)
u16 QzBugContestMinutesLeft(void)
{
    u32 used = gMain.vblankCounter1 - sBug.startFrame, total = ContestFrames();
    u32 left = used < total ? (total - used) / 3600 : 0;
    ConvertIntToDecimalStringN(gStringVar1, left + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    return gSpecialVar_Result = left + 1;
}

// The prize the officer keeps when the BAG is full (Crystal's EVENT_CONTEST_OFFICER_HAS_*): only for today.
// Hold: VAR_0x8005 the prize. Held: VAR_RESULT 1 and VAR_0x8005 the prize when there's one for today.
u16 QzBugContestHoldPrize(void)
{
    VarSet(VAR_JOHTO_BUG_CONTEST_PRIZE, gSpecialVar_0x8005);
    return 0;
}

u16 QzBugContestHeldPrize(void)
{
    RtcCalcLocalTime();
    if (VarGet(VAR_JOHTO_BUG_CONTEST_DAY) != (u16)(gLocalTime.days + 1))
        VarSet(VAR_JOHTO_BUG_CONTEST_PRIZE, ITEM_NONE);
    gSpecialVar_0x8005 = VarGet(VAR_JOHTO_BUG_CONTEST_PRIZE);
    return gSpecialVar_Result = gSpecialVar_0x8005 != ITEM_NONE;
}

u16 QzBugContestClearPrize(void)
{
    VarSet(VAR_JOHTO_BUG_CONTEST_PRIZE, ITEM_NONE);
    return 0;
}

// The park's ON_TRANSITION: today's five contestants out, the others hidden; at the judging (VAR_QZ_0x5019 3) they
// stand in a line beside the player
u16 QzBugContestSetupPark(void)
{
    static const s8 sLine[5] = {16, 18, 22, 24, 26};
    u32 i, slot = 0;
    for (i = 0; i < 10; i++)
    {
        if (sBug.state != CONTEST_ON || !(sBug.present & (1 << i)))
        {
            FlagSet(FLAG_TEMP_11 + i);
            continue;
        }
        if (VarGet(VAR_QZ_0x5019) == 3 && slot < 5)
        {
            SetObjEventTemplateCoords(CONTESTANT_LOCALID + i, sLine[slot++], 28);
            SetObjEventTemplateMovementType(CONTESTANT_LOCALID + i, MOVEMENT_TYPE_FACE_UP);
        }
    }
    return 0;
}
