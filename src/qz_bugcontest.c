// Moemon Kai: JOHTO's Bug-Catching Contest in the NATIONAL PARK (Moemon Quetzal's rules, its scripts in
// data/maps/Johto_Route35NationalParkGate and Johto_NationalParkBugContest call these specials).
// The lead Pokémon alone, 30 SAFARI BALLS (the player's own are kept aside), eight minutes, up to five catches;
// every catch is kept (sent to the PC at the end), the best one is judged against four other contestants.
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
#include "script.h"
#include "string_util.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/qz_port.h"
#include "constants/species.h"

#define CONTEST_BALLS   30
#define CONTEST_FRAMES  (8 * 60 * 60)   // eight minutes
#define CONTEST_CATCHES 5

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
    struct Pokemon party[PARTY_SIZE];   // the party as it was (the lead goes back as it was: no contest EXP)
    struct Contestant board[5];         // ranked after the judging (the player is one of them)
    u16 prize;
} sBug = {0};

extern const u8 EventScript_QzBugContestOver[];
extern const u8 EventScript_QzBugContestLeft[];

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

// a catch's score: level, IVs, rarity (its catch rate), shininess
static u16 Score(u16 species, u8 level, u32 ivSum, bool8 shiny)
{
    u8 rate = species < NUM_SPECIES ? gSpeciesInfo[species].catchRate : 255;
    return level * 4 + ivSum / 2 + (255 - rate) / 2 + (shiny ? 100 : 0);
}

static u16 MonScore(struct Pokemon *mon)
{
    u32 iv = 0, i;
    for (i = 0; i < NUM_STATS; i++)
        iv += GetMonData(mon, MON_DATA_HP_IV + i, NULL);
    return Score(GetMonData(mon, MON_DATA_SPECIES, NULL), GetMonData(mon, MON_DATA_LEVEL, NULL), iv, IsMonShiny(mon));
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
    if (FreePcSlots() < CONTEST_CATCHES)
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
    memcpy(sBug.party, gPlayerParty, sizeof(sBug.party));
    for (i = 1; i < PARTY_SIZE; i++)
        ZeroMonData(&gPlayerParty[i]);
    CalculatePlayerPartyCount();
    sBug.side = gSpecialVar_0x8004 != 0;
    sBug.state = CONTEST_ON;
    sBug.startFrame = gMain.vblankCounter1;
    sBug.prize = ITEM_NONE;
    MPNet_Log("bug contest: started (side %d, %d own SAFARI BALLS kept)", sBug.side, sBug.ownSafariBalls);
    return gSpecialVar_Result = 0;
}

// the catches go to the PC: VAR_RESULT 1 when all did, 0 when a box ran out
u16 QzBugContestReleaseCaught(void)
{
    u32 i;
    for (i = 1; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            continue;
        if (MP_SendMonToPC(&gPlayerParty[i]) != MON_GIVEN_TO_PC)
            return gSpecialVar_Result = 0;
        ZeroMonData(&gPlayerParty[i]);
    }
    CalculatePlayerPartyCount();
    return gSpecialVar_Result = 1;
}

// the party as it was, the player's own SAFARI BALLS back: VAR_RESULT 1, or 0 when the bag has no room for them
u16 QzBugContestEnd(void)
{
    u16 left;
    if (sBug.state != CONTEST_ON)
        return gSpecialVar_Result = 1;
    QzBugContestReleaseCaught();   // (anything still in the party: the scripts sent the catches first)
    left = CountTotalItemQuantityInBag(ITEM_SAFARI_BALL);
    if (left)
        RemoveBagItem(ITEM_SAFARI_BALL, left);
    if (sBug.ownSafariBalls && !AddBagItem(ITEM_SAFARI_BALL, sBug.ownSafariBalls))
    {
        if (left)
            AddBagItem(ITEM_SAFARI_BALL, left);
        return gSpecialVar_Result = 0;
    }
    memcpy(gPlayerParty, sBug.party, sizeof(sBug.party));
    CalculatePlayerPartyCount();
    sBug.state = CONTEST_OFF;
    MPNet_Log("bug contest: over, place %d", sBug.playerPlace);
    return gSpecialVar_Result = 1;
}

u16 QzBugContestGetCaughtCount(void)
{
    return gSpecialVar_Result = sBug.state == CONTEST_ON ? Catches() : 0;
}

// the four other contestants caught something too: ranked with the player's best catch
u16 QzBugContestJudge(void)
{
    static const u8 sBelinda[] = _("BELINDA"), sEd[] = _("ED"), sShaun[] = _("SHAUN"), sBarry[] = _("BARRY");
    static const u8 *const sNames[4] = {sBelinda, sEd, sShaun, sBarry};
    static const u16 sBugs[] = {SPECIES_CATERPIE, SPECIES_METAPOD, SPECIES_BUTTERFREE, SPECIES_WEEDLE, SPECIES_KAKUNA,
                                SPECIES_BEEDRILL, SPECIES_PARAS, SPECIES_VENONAT, SPECIES_PINSIR, SPECIES_SCYTHER};
    u32 i, j, best = 1;
    struct Contestant tmp;
    for (i = 2; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
         && MonScore(&gPlayerParty[i]) > MonScore(&gPlayerParty[best]))
            best = i;
    sBug.board[0].name = gSaveBlock2Ptr->playerName;
    sBug.board[0].species = GetMonData(&gPlayerParty[best], MON_DATA_SPECIES, NULL);
    sBug.board[0].score = MonScore(&gPlayerParty[best]);
    sBug.board[0].shiny = IsMonShiny(&gPlayerParty[best]);
    for (i = 0; i < 4; i++)
    {
        struct Contestant *c = &sBug.board[1 + i];
        c->name = sNames[i];
        c->species = sBugs[Random() % ARRAY_COUNT(sBugs)];
        c->shiny = (Random() % 512) == 0;
        c->score = Score(c->species, 12 + Random() % 6, Random() % 187, c->shiny);
    }
    for (i = 0; i < 5; i++)   // best first
        for (j = i + 1; j < 5; j++)
            if (sBug.board[j].score > sBug.board[i].score)
                tmp = sBug.board[i], sBug.board[i] = sBug.board[j], sBug.board[j] = tmp;
    sBug.playerPlace = 4;
    for (i = 0; i < 3; i++)
        if (sBug.board[i].name == gSaveBlock2Ptr->playerName)
            sBug.playerPlace = i + 1;
    sBug.prize = sBug.playerPlace == 1 ? ITEM_SUN_STONE : sBug.playerPlace == 2 ? ITEM_EVERSTONE
               : sBug.playerPlace == 3 ? ITEM_SITRUS_BERRY : ITEM_ORAN_BERRY;
    if (sBug.playerPlace == 4)   // its own line: the player's best catch and score
        for (i = 3; i < 5; i++)
            if (sBug.board[i].name == gSaveBlock2Ptr->playerName)
                tmp = sBug.board[i], sBug.board[i] = sBug.board[3], sBug.board[3] = tmp;
    MPNet_Log("bug contest: judged, player %d (score %d)", sBug.playerPlace, sBug.board[sBug.playerPlace - 1].score);
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

u16 QzBugContestGetPlayerPlace(void)
{
    gSpecialVar_0x8005 = sBug.prize;
    return gSpecialVar_Result = sBug.playerPlace;
}

u16 QzBugContestGetFlag2(void) { return gSpecialVar_Result = 0; }          // (an exhibition run under Nuzlocke)
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

// The field each frame (DoCB1_Overworld, controls free): the contest ends when time is up, after the fifth catch or
// with no SAFARI BALL left; leaving the park ends it at once. TRUE: a script was started
bool8 QzBugContest_FieldCheck(void)
{
    if (sBug.state != CONTEST_ON)
        return FALSE;
    if (!OnContestMap())
    {
        // walked out of the park: the catches to the PC, the party back, the contest over
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
    if (gMain.vblankCounter1 - sBug.startFrame >= ContestFrames() || Catches() >= CONTEST_CATCHES
     || !CheckBagHasItem(ITEM_SAFARI_BALL, 1))
    {
        MPNet_Log("bug contest: time up (%u frames, %d catches)", gMain.vblankCounter1 - sBug.startFrame, Catches());
        ScriptContext_SetupScript(EventScript_QzBugContestOver);
        return TRUE;
    }
    return FALSE;
}
