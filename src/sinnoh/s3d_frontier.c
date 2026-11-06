// Moemon Kai: SINNOH's BATTLE CASTLE and BATTLE ARCADE (data/scripts/sinnoh_places.inc), Platinum's ideas on the
// game's own frontier pieces. Both: three of the player's POKéMON (copies at Lv 50: the party itself never changes)
// against sets of seven frontier trainers (gBattleFrontierTrainers, each three of its frontier POKéMON at Lv 50, the
// trainer's strength rising with the streak), HP and PP carried from battle to battle within a set; a set won gives
// BATTLE POINTS (3 + the sets won before, 10 at most) and the next set starts healed.
//   CASTLE: CASTLE POINTS (CP) won by each battle (10, 5 more with no one fainted, 2 for each above half HP) and spent
//   before a battle: REST (the three healed, 10 CP) or BERRIES (a SITRUS BERRY for each, 6 CP).
//   ARCADE: before each battle its board lands on an event for that battle (healing, a status for one side, weather,
//   levels, berries, a free win, a BATTLE POINT...).
// The streaks and bests are kept in "<save>.region" (chunk FRNT, mp_region.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "s3d.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_transition.h"
#include "event_data.h"
#include "field_weather.h"
#include "main.h"
#include "mp.h"
#include "mp_region.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_trainers.h"
#include "constants/field_specials.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/songs.h"
#include "constants/weather.h"
#include "sinnoh_tower.h"

const u8 *S3D_TextByName(const char *name);
u16 Sinnoh_HallStreak(void);   // (s3d_hall.c)
u8 *Sinnoh_HallPrints(void);

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
extern u16 (*gQzScrollResult)(u16 choice);
void ShowScrollableMultichoice(void);
extern const u16 gFrontierBannedSpecies[];
extern const struct BattleFrontierTrainer gBattleFrontierTrainers[];
extern const struct FacilityMon gBattleFrontierMons[];

enum { FACILITY_CASTLE, FACILITY_ARCADE };
#define SET_BATTLES 7

static struct FrontierSave
{
    u16 streak[2], best[2], sets[2];
    u16 cp;
    u8 prints[2];   // the Frontier Brain's prints won (bit 0 Silver, 1 Gold; added 1.114.0: older records read 0)
} sSave;

static struct
{
    u8 facility, count, slots[3], battle, event;
    struct Pokemon team[3];          // the set's copies
    struct Pokemon saved[PARTY_SIZE];
    u8 weather;
} sRun;

void Sinnoh_FrontierReset(void)
{
    memset(&sSave, 0, sizeof(sSave));
}

const void *Sinnoh_FrontierData(u32 *size)
{
    *size = sizeof(sSave);
    return &sSave;
}

void Sinnoh_FrontierLoad(const void *data, u32 size)
{
    Sinnoh_FrontierReset();
    memcpy(&sSave, data, size < sizeof(sSave) ? size : sizeof(sSave));
}

static void SetLevel(struct Pokemon *mon, u8 level)
{
    u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][level];
    u16 hp = GetMonData(mon, MON_DATA_HP), maxOld = GetMonData(mon, MON_DATA_MAX_HP), maxNew;
    SetMonData(mon, MON_DATA_EXP, &exp);
    SetMonData(mon, MON_DATA_LEVEL, &level);
    CalculateMonStats(mon);
    maxNew = GetMonData(mon, MON_DATA_MAX_HP);
    hp = maxOld ? (u16)((u32)hp * maxNew / maxOld) : maxNew;
    SetMonData(mon, MON_DATA_HP, &hp);
}

static void Heal(struct Pokemon *mon)
{
    u16 hp = GetMonData(mon, MON_DATA_MAX_HP);
    u32 status = 0;
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_STATUS, &status);
    MonRestorePP(mon);
}

// VAR_0x8004 the facility (0 CASTLE, 1 ARCADE): STR_VAR_1 its streak, STR_VAR_2 its best, STR_VAR_3 the CP
u16 Sinnoh_FrontierInfo(void)
{
    sRun.facility = gSpecialVar_0x8004 ? FACILITY_ARCADE : FACILITY_CASTLE;
    ConvertIntToDecimalStringN(gStringVar1, sSave.streak[sRun.facility], STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, sSave.best[sRun.facility], STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar3, sSave.cp, STR_CONV_MODE_LEFT_ALIGN, 4);
    sRun.count = 0;
    return gSpecialVar_Result = sSave.streak[sRun.facility] > 0;
}

// The POKéMON in slot VAR_0x8004 joins the team: VAR_RESULT the team's size, or 10 an EGG / a legend, 11 already in
u16 Sinnoh_FrontierAddMon(void)
{
    u8 slot = gSpecialVar_0x8004, i;
    struct Pokemon *mon;
    u16 species;
    if (slot >= PARTY_SIZE)
        return gSpecialVar_Result = 10;
    mon = &gPlayerParty[slot];
    species = GetMonData(mon, MON_DATA_SPECIES);
    if (GetMonData(mon, MON_DATA_IS_EGG) || species == SPECIES_NONE || gSpeciesInfo[species].catchRate <= 5)
        return gSpecialVar_Result = 10;
    for (i = 0; gFrontierBannedSpecies[i] != 0xFFFF; i++)
        if (gFrontierBannedSpecies[i] == species)
            return gSpecialVar_Result = 10;
    for (i = 0; i < sRun.count; i++)
        if (sRun.slots[i] == slot)
            return gSpecialVar_Result = 11;
    sRun.slots[sRun.count] = slot;
    sRun.team[sRun.count] = *mon;
    SetLevel(&sRun.team[sRun.count], 50);
    Heal(&sRun.team[sRun.count]);
    sRun.count++;
    if (sRun.count == 3)
    {
        sRun.battle = 0;
        MPNet_Log("frontier: facility %d team %d %d %d", sRun.facility, GetMonData(&sRun.team[0], MON_DATA_SPECIES),
                  GetMonData(&sRun.team[1], MON_DATA_SPECIES), GetMonData(&sRun.team[2], MON_DATA_SPECIES));
    }
    return gSpecialVar_Result = sRun.count;
}

// ---------------------------------------------------------------------------------------------- the CASTLE's menu
static const u8 sCastleRows[][16] = {_("BATTLE"), _("REST  10 CP"), _("BERRIES  6 CP")};

static u16 CastlePicked(u16 row)
{
    return row < ARRAY_COUNT(sCastleRows) ? row : MULTI_B_PRESSED;
}

// Before a CASTLE battle (waitstate; VAR_RESULT 0 BATTLE, 1 REST, 2 BERRIES, MULTI_B_PRESSED stop; STR_VAR_3 the CP)
u16 Sinnoh_CastleMenu(void)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sCastleRows); i++)
        gQzScrollOptions[i] = sCastleRows[i];
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = CastlePicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ConvertIntToDecimalStringN(gStringVar3, sSave.cp, STR_CONV_MODE_LEFT_ALIGN, 4);
    ShowScrollableMultichoice();
    return 0;
}

// VAR_0x8004 1 REST, 2 BERRIES bought: VAR_RESULT TRUE done, FALSE too few CP
u16 Sinnoh_CastleBuy(void)
{
    u16 price = gSpecialVar_0x8004 == 1 ? 10 : 6, berry = ITEM_SITRUS_BERRY;
    u32 i;
    if (sSave.cp < price)
        return gSpecialVar_Result = FALSE;
    sSave.cp -= price;
    for (i = 0; i < 3; i++)
    {
        if (gSpecialVar_0x8004 == 1)
            Heal(&sRun.team[i]);
        else if (GetMonData(&sRun.team[i], MON_DATA_HELD_ITEM) == ITEM_NONE)
            SetMonData(&sRun.team[i], MON_DATA_HELD_ITEM, &berry);
    }
    ConvertIntToDecimalStringN(gStringVar3, sSave.cp, STR_CONV_MODE_LEFT_ALIGN, 4);
    MPNet_Log("frontier: castle bought %d, %d CP left", gSpecialVar_0x8004, sSave.cp);
    return gSpecialVar_Result = TRUE;
}

// ---------------------------------------------------------------------------------------------- the ARCADE's board
enum
{
    EV_NOTHING, EV_HEAL, EV_FOE_POISON, EV_FOE_PARALYSIS, EV_FOE_BURN, EV_FOE_SLEEP, EV_POISON, EV_PARALYSIS, EV_BURN,
    EV_SUN, EV_RAIN, EV_SAND, EV_LEVEL_UP, EV_FOE_LEVEL_UP, EV_BERRIES, EV_FOE_BERRIES, EV_FREE_WIN, EV_BP,
    EV_COUNT
};
static const u8 sEventNames[EV_COUNT][24] = {
    _("NOTHING AT ALL"), _("YOUR TEAM HEALED"), _("FOES POISONED"), _("FOES PARALYZED"), _("FOES BURNED"),
    _("FOES ASLEEP"), _("YOUR TEAM POISONED"), _("YOUR TEAM PARALYZED"), _("YOUR TEAM BURNED"), _("BLAZING SUN"),
    _("POURING RAIN"), _("A SANDSTORM"), _("YOUR LEVELS UP 3"), _("FOE LEVELS UP 3"), _("BERRIES FOR YOU"),
    _("BERRIES FOR FOES"), _("A FREE WIN"), _("A BONUS BP"),
};
// (the board's weights: the good and the bad about even, the free win rare)
static const u8 sEventWeights[EV_COUNT] = {8, 8, 5, 5, 5, 4, 5, 5, 5, 6, 6, 6, 5, 5, 6, 6, 2, 4};

// The board spins (VAR_RESULT the event; STR_VAR_1 its name)
u16 Sinnoh_ArcadeSpin(void)
{
    u32 total = 0, i, r;
    for (i = 0; i < EV_COUNT; i++)
        total += sEventWeights[i];
    r = Random() % total;
    for (i = 0; i < EV_COUNT - 1 && r >= sEventWeights[i]; i++)
        r -= sEventWeights[i];
    if (getenv("MOEMON_TEST_ARCADE"))   // (tests)
        i = atoi(getenv("MOEMON_TEST_ARCADE")) % EV_COUNT;
    sRun.event = i;
    StringCopy(gStringVar1, sEventNames[i]);
    if (i == EV_HEAL)
        for (r = 0; r < 3; r++)
            Heal(&sRun.team[r]);
    if (i == EV_BP)
        gSaveBlock2Ptr->frontier.battlePoints += gSaveBlock2Ptr->frontier.battlePoints < MAX_BATTLE_FRONTIER_POINTS;
    MPNet_Log("frontier: arcade event %d", i);
    return gSpecialVar_Result = i;
}

static void SetStatus(struct Pokemon *mon, u32 status)
{
    if (GetMonData(mon, MON_DATA_HP) > 0 && GetMonData(mon, MON_DATA_STATUS) == 0)
        SetMonData(mon, MON_DATA_STATUS, &status);
}

static void ApplyEvent(void)
{
    static const u32 sStatus[] = {STATUS1_POISON, STATUS1_PARALYSIS, STATUS1_BURN, 3};   // (3: asleep three turns)
    u16 berry = ITEM_SITRUS_BERRY;
    u32 i;
    for (i = 0; i < 3; i++)
    {
        struct Pokemon *mine = &gPlayerParty[i], *foe = &gEnemyParty[i];
        switch (sRun.event)
        {
        case EV_FOE_POISON: case EV_FOE_PARALYSIS: case EV_FOE_BURN: case EV_FOE_SLEEP:
            SetStatus(foe, sStatus[sRun.event - EV_FOE_POISON]);
            break;
        case EV_POISON: case EV_PARALYSIS: case EV_BURN:
            SetStatus(mine, sStatus[sRun.event - EV_POISON]);
            break;
        case EV_LEVEL_UP:
            SetLevel(mine, 53);
            break;
        case EV_FOE_LEVEL_UP:
            SetLevel(foe, 53);
            break;
        case EV_BERRIES:
            if (GetMonData(mine, MON_DATA_HELD_ITEM) == ITEM_NONE)
                SetMonData(mine, MON_DATA_HELD_ITEM, &berry);
            break;
        case EV_FOE_BERRIES:
            SetMonData(foe, MON_DATA_HELD_ITEM, &berry);
            break;
        }
    }
    sRun.weather = sRun.event == EV_SUN ? WEATHER_DROUGHT : sRun.event == EV_RAIN ? WEATHER_RAIN
                 : sRun.event == EV_SAND ? WEATHER_SANDSTORM : WEATHER_NONE;
    if (sRun.weather != WEATHER_NONE)
        SetCurrentAndNextWeatherNoDelay(sRun.weather);
}

// ---------------------------------------------------------------------------------------------- the Frontier Brains
// Platinum's Frontier Brains (its overlay104 battle_*_helpers.c): the HALL MATRON ARGENTA, the CASTLE VALET DARACH and
// the ARCADE STAR DAHLIA battle the player at the streak's set numbers (constants/battle_frontier.h: the CASTLE's and
// ARCADE's 21st and 49th, the HALL's 50th and 170th), with Platinum's frontier sets for them (s3d_tower.c), their words
// from Platinum's scene texts. A win gives the Silver or the Gold Print once.
enum { BRAIN_HALL, BRAIN_CASTLE, BRAIN_ARCADE };
static const u16 sBrainBattle[3][2] = {{50, 170}, {21, 49}, {21, 49}};
// (approaching, the intro, the intro for Gold, beaten, beaten for Gold, the Silver Print, the Gold Print)
static const char *const sBrainText[3][7] = {
    {"BattleHallScene_Text_HallMatronApproaching", "BattleHallScene_Text_ArgentaIntro",
     "BattleHallScene_Text_ArgentaIntroGold", "BattleHallScene_Text_BeatArgenta", "BattleHallScene_Text_BeatArgentaGold",
     "BattleHall_Text_SilverPrintAdded", "BattleHall_Text_GoldPrintAdded"},
    {"BattleCastleScene_Text_CastleValetApproaching", "BattleCastleScene_Text_DarachIntro",
     "BattleCastleScene_Text_DarachIntroGold", "BattleCastleScene_Text_BeatDarach",
     "BattleCastleScene_Text_BeatDarachGold", "BattleCastle_Text_SinglePrintAdded", "BattleCastle_Text_GoldPrintAdded"},
    {"BattleArcadeScene_Text_ArcadeStarApproaching", "BattleArcadeScene_Text_DahliaIntro",
     "BattleArcadeScene_Text_DahliaIntroGold", "BattleArcadeScene_Text_BeatDahlia", "BattleArcadeScene_Text_BeatDahliaGold",
     "BattleArcade_Text_SilverPrintAdded", "BattleArcade_Text_GoldPrintAdded"},
};
static u8 sBrainFought;    // the battle under way: 0 none, else the brain's kind (1 Silver, 2 Gold)
static u8 sBrainNewPrint;  // the beaten brain's print is new

// The brain the next battle is against: 0 none, 1 Silver, 2 Gold
u8 Sinnoh_BrainNext(u8 brain)
{
    u32 next = (brain == BRAIN_HALL ? Sinnoh_HallStreak() : sSave.streak[brain - BRAIN_CASTLE]) + 1;
    return next == sBrainBattle[brain][0] ? 1 : next == sBrainBattle[brain][1] ? 2 : 0;
}

// The battle about to start is (KIND 1 / 2) or isn't (0) against the brain
void Sinnoh_BrainBattle(u8 kind)
{
    sBrainFought = kind;
}

// Platinum's words about the brain (`sinnoh` scripts: VAR_0x8008 the facility, 0 CASTLE, 1 ARCADE, 2 HALL; VAR_0x8004
// 0 the brain comes (next battle), 1 its intro, 2 beaten (the battle just won), 3 the print added): gStringVar4,
// VAR_RESULT the brain's kind (1 Silver, 2 Gold) or 0 for nothing to say
u16 Sinnoh_BrainText(void)
{
    u8 brain = gSpecialVar_0x8008 >= 2 ? BRAIN_HALL : BRAIN_CASTLE + gSpecialVar_0x8008, step = gSpecialVar_0x8004, kind;
    const char *name;
    kind = step < 2 ? Sinnoh_BrainNext(brain) : sBrainFought;
    if (step == 3 && !sBrainNewPrint)
        kind = 0;
    if (kind == 0)
        return gSpecialVar_Result = 0;
    name = step == 0 ? sBrainText[brain][0] : step == 1 ? sBrainText[brain][kind] : step == 2 ? sBrainText[brain][2 + kind]
         : sBrainText[brain][4 + kind];
    if (step == 2)
    {
        // (Platinum's PRINT_STATE: each print once)
        u8 bit = 1 << (kind - 1), *prints = brain == BRAIN_HALL ? Sinnoh_HallPrints() : &sSave.prints[brain - BRAIN_CASTLE];
        sBrainNewPrint = !(*prints & bit);
        *prints |= bit;
        MPNet_Log("frontier: brain %d (%s) beaten%s", brain, kind == 2 ? "gold" : "silver", sBrainNewPrint ? ", a new print" : "");
    }
    StringCopy(gStringVar1, gSaveBlock2Ptr->playerName);   // (Platinum's BufferPlayerName 0)
    StringExpandPlaceholders(gStringVar4, S3D_TextByName(name));
    {
        u16 bank, msg;
        bool8 S3D_TextNamed(const char *name, u16 *bank, u16 *msg);
        if (!S3D_TextNamed(name, &bank, &msg))
            MPNet_Log("frontier: no text %s", name);
    }
    return gSpecialVar_Result = kind;
}

// (tests: `mp brainstreak FACILITY N`, 0 CASTLE, 1 ARCADE, 2 HALL: the streak set to N, the prints shown)
void Sinnoh_BrainDebug(const char *arg)
{
    int facility = 0, n = 0;
    void Sinnoh_HallSetStreak(u16 streak);
    sscanf(arg, "%d %d", &facility, &n);
    if (facility >= 2)
        Sinnoh_HallSetStreak(n);
    else
        sSave.streak[facility] = n;
    printf("[test] brain: facility %d streak %d, prints %d %d %d, next brain %d\n", facility, n, sSave.prints[0],
           sSave.prints[1], *Sinnoh_HallPrints(), Sinnoh_BrainNext(facility >= 2 ? BRAIN_HALL : BRAIN_CASTLE + facility));
}

// ---------------------------------------------------------------------------------------------- the battles
static void CB2_EndFrontierBattle(void)
{
    bool8 won = gBattleOutcome == B_OUTCOME_WON;
    u32 i;
    for (i = 0; i < 3; i++)
    {
        sRun.team[i] = gPlayerParty[i];
        if (sRun.event == EV_LEVEL_UP)
            SetLevel(&sRun.team[i], 50);   // (the board's levels are for that battle)
    }
    memcpy(gPlayerParty, sRun.saved, sizeof(gPlayerParty));
    CalculatePlayerPartyCount();
    if (sRun.weather != WEATHER_NONE)
        SetCurrentAndNextWeatherNoDelay(GetSavedWeather());
    VarSet(VAR_FRONTIER_FACILITY, 0);
    MP_MUSIC_SCENE_RELEASE();
    gSpecialVar_Result = won;
    MPNet_Log("frontier: battle %s", won ? "won" : "lost");
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_StartFrontierBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndFrontierBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// The set's next battle (waitstate; VAR_RESULT TRUE: won; the ARCADE's free win needs no battle)
u16 Sinnoh_FrontierBattle(void)
{
    u32 i, band, streak = sSave.streak[sRun.facility];
    u8 lvlMode = gSaveBlock2Ptr->frontier.lvlMode;
    u8 brain = Sinnoh_BrainNext(BRAIN_CASTLE + sRun.facility);
    Sinnoh_BrainBattle(brain);
    if (sRun.facility == FACILITY_ARCADE && sRun.event == EV_FREE_WIN && !brain)   // (never past the brain)
    {
        gSpecialVar_Result = TRUE;
        ScriptContext_Enable();
        return 0;
    }
    memcpy(sRun.saved, gPlayerParty, sizeof(gPlayerParty));
    ZeroPlayerPartyMons();
    for (i = 0; i < 3; i++)
        gPlayerParty[i] = sRun.team[i];
    CalculatePlayerPartyCount();
    // the trainer: stronger as the streak grows (the frontier list runs weakest to strongest, 300 in ten bands)
    band = streak / SET_BATTLES;
    if (band > 9)
        band = 9;
    gTrainerBattleOpponent_A = band * (FRONTIER_TRAINERS_COUNT / 10) + Random() % (FRONTIER_TRAINERS_COUNT / 10);
    gFacilityTrainers = gBattleFrontierTrainers;
    gFacilityTrainerMons = gBattleFrontierMons;
    if (brain)
    {
        // the Frontier Brain: Platinum's sets for it, none of the player's kinds
        u16 ft = Sinnoh_Frontier_Brain(BRAIN_CASTLE + sRun.facility, brain - 1), avoid[3];
        for (i = 0; i < 3; i++)
            avoid[i] = GetMonData(&sRun.team[i], MON_DATA_SPECIES);
        ZeroEnemyPartyMons();
        Sinnoh_Frontier_SetOpponent(TRAINER_SINNOH_FRONTIER_A, ft);
        Sinnoh_Frontier_FillParty(gEnemyParty, ft, 3, avoid, NULL, 3, 50);
        gTrainerBattleOpponent_A = TRAINER_SINNOH_FRONTIER_A;
    }
    else
    {
        gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_50;
        FillFrontierTrainerParty(3);
        gSaveBlock2Ptr->frontier.lvlMode = lvlMode;
    }
    sRun.weather = WEATHER_NONE;
    if (sRun.facility == FACILITY_ARCADE)
        ApplyEvent();
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER;   // (one frontier flag: all of them were the PALACE's and ARENA's rules too)
    MPNet_Log("frontier: facility %d battle %d (streak %d): trainer %d", sRun.facility, sRun.battle + 1, streak,
              gTrainerBattleOpponent_A);
    CreateTask(Task_StartFrontierBattle, 1);
    if (brain)
    {
        MP_MUSIC_SCENE_HOLD(MUS_VS_FRONTIER_BRAIN, MUS_VICTORY_GYM_LEADER);
        PlayMapChosenOrBattleBGM(MUS_VS_FRONTIER_BRAIN);
    }
    else
    {
        MP_MUSIC_SCENE_HOLD(MUS_VS_TRAINER, MUS_VICTORY_TRAINER);
        PlayMapChosenOrBattleBGM(0);
    }
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_B_TOWER));
    return 0;
}

// A win: STR_VAR_1 the battle in the set, STR_VAR_2 the CP won (CASTLE); VAR_RESULT the BP given when the set is
// won (STR_VAR_3 their number; the next set starts healed), else 0
u16 Sinnoh_FrontierWon(void)
{
    u16 bp = 0, cp = 0;
    u32 i, fainted = 0, healthy = 0;
    for (i = 0; i < 3; i++)
    {
        u16 hp = GetMonData(&sRun.team[i], MON_DATA_HP), max = GetMonData(&sRun.team[i], MON_DATA_MAX_HP);
        fainted += hp == 0;
        healthy += hp * 2 > max;
    }
    if (sRun.facility == FACILITY_CASTLE)
    {
        cp = 10 + (fainted ? 0 : 5) + 2 * healthy;
        sSave.cp = sSave.cp + cp > 9999 ? 9999 : sSave.cp + cp;
    }
    sRun.battle++;
    sSave.streak[sRun.facility]++;
    if (sSave.streak[sRun.facility] > sSave.best[sRun.facility])
        sSave.best[sRun.facility] = sSave.streak[sRun.facility];
    ConvertIntToDecimalStringN(gStringVar1, sRun.battle, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, cp, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (sRun.battle >= SET_BATTLES)
    {
        bp = 3 + sSave.sets[sRun.facility];
        if (bp > 10)
            bp = 10;
        sSave.sets[sRun.facility]++;
        gSaveBlock2Ptr->frontier.battlePoints += bp;
        if (gSaveBlock2Ptr->frontier.battlePoints > MAX_BATTLE_FRONTIER_POINTS)
            gSaveBlock2Ptr->frontier.battlePoints = MAX_BATTLE_FRONTIER_POINTS;
        Sinnoh_TvBattlePoints(bp);   // (the TV's RECORDS: s3d_tv.c)
        sRun.battle = 0;
        for (i = 0; i < 3; i++)
            Heal(&sRun.team[i]);
    }
    ConvertIntToDecimalStringN(gStringVar3, bp, STR_CONV_MODE_LEFT_ALIGN, 2);
    MPNet_Log("frontier: facility %d streak %d, %d CP (%d), %d BP", sRun.facility, sSave.streak[sRun.facility], cp,
              sSave.cp, bp);
    return gSpecialVar_Result = bp;
}

// The challenge ends (a loss or stopping): STR_VAR_1 the streak
u16 Sinnoh_FrontierEnd(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sSave.streak[sRun.facility], STR_CONV_MODE_LEFT_ALIGN, 4);
    MPNet_Log("frontier: facility %d ends at %d (best %d)", sRun.facility, sSave.streak[sRun.facility],
              sSave.best[sRun.facility]);
    sSave.streak[sRun.facility] = 0;
    sSave.sets[sRun.facility] = 0;
    if (sRun.facility == FACILITY_CASTLE)
        sSave.cp = 0;
    return 0;
}

// (s3d_misc.c, the VILLA's order form) the CASTLE's (0) or the ARCADE's (1) best streak
u16 Sinnoh_FrontierBest(u8 facility)
{
    return sSave.best[facility & 1];
}
