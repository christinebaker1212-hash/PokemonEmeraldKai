// Moemon Kai: JOHTO's BATTLE TOWER as Crystal's (audit case 37): pret/pokecrystal's engine/events/battle_tower/
// (battle_tower.asm, load_trainer.asm, rules.asm, trainer_text.asm) and mobile_46.asm's level choice, as the special
// CallJohtoBattleTowerFunc (VAR_0x8004 = JBT_FUNC_*, include/constants/johto_battle_tower.h) for the scripts in
// data/scripts/johto_battle_tower.inc. Crystal's 70 trainers, ten level groups of 21 POKéMON and their words are
// src/data/johto_battle_tower.h (tools/quetzal_port/gen_battle_tower_gsc.py).
//
// Crystal's rules: three POKéMON, all different, holding different items, no EGG, none above the level group chosen
// (L:10-L:40 before JOHTO's HALL OF FAME, then to L:100), no MEWTWO, MEW, LUGIA, HO-OH or CELEBI under L:70 below
// level 70; seven trainers in a row, each with three of the group's POKéMON (none twice, nor items twice, nor one of
// the last two trainers' POKéMON), the party healed before and after each battle, SET battle style; seven wins give
// five of a stat item (HP UP, PROTEIN, IRON, CARBOS or CALCIUM). What Crystal keeps in its SRAM lives in the save's
// vars (VAR_JOHTO_BT_*). The battles run as Emerald's TRAINER HILL type (trainer_hill.c's hooks ask
// MP_JohtoTower_InBattle): no EXP, no money, nothing stolen, as Crystal's BATTLE TOWER battles.
#include <stdio.h>
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle_main.h"
#include "event_data.h"
#include "field_specials.h"
#include "item.h"
#include "load_save.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/event_objects.h"
#include "constants/script_menu.h"
#include "constants/field_specials.h"
#include "constants/characters.h"
#include "script_pokemon_util.h"
#include "constants/johto_battle_tower.h"
#include "constants/qz_port.h"

#include "../data/johto_battle_tower.h"

u8 GetSpecialBattleTransition(s32 id);
extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
extern u16 (*gQzScrollResult)(u16 choice);
u16 MP_JohtoLeagueBeaten(void);   // (mp_johto_legend.c)

// the state var (Crystal's sBattleTowerChallengeState, sBattleTowerSaveFileFlags, sBTChoiceOfLevelGroup,
// sNrOfBeatenBattleTowerTrainers, sBattleTowerReward)
#define ST_CHALLENGE      0x0007
#define ST_EXPLAINED      0x0008
#define ST_GROUP_SHIFT    4
#define ST_GROUP          0x00F0
#define ST_BEATEN_SHIFT   8
#define ST_BEATEN         0x0F00
#define ST_REWARD_SHIFT   12
#define ST_REWARD         0xF000

static const u16 sRewards[] = {ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CARBOS, ITEM_CALCIUM};

// Crystal's BattleTower_UbersCheck: MEWTWO, MEW and LUGIA to CELEBI
static const u16 sUbers[] = {SPECIES_MEWTWO, SPECIES_MEW, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI};

static EWRAM_DATA u8 sTrainer = 0;            // the opponent (sJbtTrainers)
static EWRAM_DATA u8 sMons[3] = {0};          // its POKéMON (sJbtMons[group - 1])
static EWRAM_DATA u8 sPrevMons[2][3] = {0};   // the last two trainers' (Crystal's sBTMonPrevTrainer / PrevPrevTrainer)
static EWRAM_DATA u8 sTextIndex = 0;          // Crystal's wBT_TrainerTextIndex
static EWRAM_DATA bool8 sInBattle = FALSE;
static EWRAM_DATA u8 sSavedStyle = 0;

static const u8 sText_L10[] = _("L:10");
static const u8 sText_L20[] = _("L:20");
static const u8 sText_L30[] = _("L:30");
static const u8 sText_L40[] = _("L:40");
static const u8 sText_L50[] = _("L:50");
static const u8 sText_L60[] = _("L:60");
static const u8 sText_L70[] = _("L:70");
static const u8 sText_L80[] = _("L:80");
static const u8 sText_L90[] = _("L:90");
static const u8 sText_L100[] = _("L:100");
static const u8 sText_Cancel[] = _("CANCEL");
static const u8 *const sLevelTexts[10] = {
    sText_L10, sText_L20, sText_L30, sText_L40, sText_L50, sText_L60, sText_L70, sText_L80, sText_L90, sText_L100,
};

static u16 State(void)
{
    return VarGet(VAR_JOHTO_BT_STATE);
}

static void SetField(u16 mask, u8 shift, u16 value)
{
    VarSet(VAR_JOHTO_BT_STATE, (State() & ~mask) | ((value << shift) & mask));
}

static u8 Group(void)
{
    u8 g = (State() & ST_GROUP) >> ST_GROUP_SHIFT;
    return (g < 1 || g > 10) ? 1 : g;
}

static u8 Beaten(void)
{
    return (State() & ST_BEATEN) >> ST_BEATEN_SHIFT;
}

// the streak's trainers so far (Crystal's sBTTrainers): 7 bytes in 4 vars
static u8 Seen(u8 i)
{
    u16 v = VarGet(VAR_JOHTO_BT_SEEN + i / 2);
    return (i & 1) ? (v >> 8) : (v & 0xFF);
}

static void SetSeen(u8 i, u8 trainer)
{
    u16 v = VarGet(VAR_JOHTO_BT_SEEN + i / 2);
    v = (i & 1) ? ((v & 0x00FF) | (trainer << 8)) : ((v & 0xFF00) | trainer);
    VarSet(VAR_JOHTO_BT_SEEN + i / 2, v);
}

static void ResetSeen(void)
{
    u8 i;
    for (i = 0; i < 4; i++)
        VarSet(VAR_JOHTO_BT_SEEN + i, 0xFFFF);
}

bool8 MP_JohtoTower_InBattle(void)
{
    return sInBattle && (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL);
}

// ---- the battle hooks (trainer_hill.c)

u8 MP_JohtoTower_OpponentClass(void)
{
    return sJbtClasses[sJbtTrainers[sTrainer % 70].cls].trainerClass;
}

void MP_JohtoTower_OpponentName(u8 *dst)
{
    *StringCopyN(dst, sJbtTrainers[sTrainer % 70].name, 10) = EOS;
}

u16 MP_JohtoTower_FrontSpriteId(void)
{
    return sJbtClasses[sJbtTrainers[sTrainer % 70].cls].trainerPic;
}

u8 MP_JohtoTower_EncounterMusicId(void)
{
    return sJbtClasses[sJbtTrainers[sTrainer % 70].cls].music;
}

// which: 1 the greeting, 2 the player lost, 3 the player won (Crystal's BATTLETOWERTEXT_*)
static void BufferText(u8 which)
{
    bool8 female = sJbtClasses[sJbtTrainers[sTrainer % 70].cls].female;
    const u8 *text;
    if (which < 1 || which > 3)
        which = 1;
    if (female)
        text = sJbtTextsF[sTextIndex % ARRAY_COUNT(sJbtTextsF)][which - 1];
    else
        text = sJbtTextsM[sTextIndex % ARRAY_COUNT(sJbtTextsM)][which - 1];
    StringExpandPlaceholders(gStringVar4, text);
}

// (the trainer hill's TEXT_PLAYER_LOST 3 / PLAYER_WON 4)
void MP_JohtoTower_TrainerText(u8 which)
{
    BufferText(which == 3 ? 2 : which == 4 ? 3 : 1);
}

// ---- Crystal's actions

static void GetState(void)
{
    gSpecialVar_Result = State() & ST_CHALLENGE;
}

static void SetState(void)
{
    SetField(ST_CHALLENGE, 0, gSpecialVar_0x8005);
}

static void CheckExplanationRead(void)
{
    gSpecialVar_Result = (State() & ST_EXPLAINED) != 0;
}

static void SetExplanationRead(void)
{
    VarSet(VAR_JOHTO_BT_STATE, State() | ST_EXPLAINED);
}

// Crystal's _CheckForBattleTowerRules: a bit for each rule broken (JBT_RULE_*)
static void CheckRules(void)
{
    u8 i, j, count = 0;
    u16 result = 0;
    u16 species[PARTY_SIZE], items[PARTY_SIZE];

    for (i = 0; i < PARTY_SIZE; i++)
    {
        species[i] = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL);
        items[i] = GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM, NULL);
        if (species[i] != SPECIES_NONE)
            count++;
    }
    if (count != 3)
        result |= JBT_RULE_THREE;
    for (i = 0; i < count; i++)
    {
        for (j = i + 1; j < count; j++)
        {
            if (species[i] == species[j])
                result |= JBT_RULE_SPECIES;
            if (items[i] != ITEM_NONE && items[i] == items[j])
                result |= JBT_RULE_ITEMS;
        }
        if (species[i] == SPECIES_EGG)
            result |= JBT_RULE_EGG;
    }
    gSpecialVar_Result = result;
    // (for the script, a var each: VAR_TEMP_2 three, 3 species, 4 items, 5 EGG; and Crystal's "3" in STR_VAR_2)
    VarSet(VAR_TEMP_2, (result & JBT_RULE_THREE) != 0);
    VarSet(VAR_TEMP_3, (result & JBT_RULE_SPECIES) != 0);
    VarSet(VAR_TEMP_4, (result & JBT_RULE_ITEMS) != 0);
    VarSet(VAR_TEMP_5, (result & JBT_RULE_EGG) != 0);
    ConvertIntToDecimalStringN(gStringVar2, 3, STR_CONV_MODE_LEFT_ALIGN, 1);
}

static u16 LevelPicked(u16 row)
{
    return (row == MULTI_B_PRESSED || row >= gQzScrollCount - 1) ? 0 : row + 1;
}

// the level choice (mobile_46.asm's Strings_L10ToL100 / Strings_Ll0ToL40): VAR_RESULT the group, 0 CANCEL
static void LevelMenu(void)
{
    u8 i, n = MP_JohtoLeagueBeaten() ? 10 : 4;
    for (i = 0; i < n; i++)
        gQzScrollOptions[i] = sLevelTexts[i];
    gQzScrollOptions[i] = sText_Cancel;
    gQzScrollCount = n + 1;
    gQzScrollResult = LevelPicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
}

// BattleTower_LevelCheck / BattleTower_UbersCheck for group VAR_0x8005: 0 fine, 1 a POKéMON's level exceeds it,
// 2 an uber under L:70 (its name in STR_VAR_1)
static void LevelCheck(void)
{
    u8 i, j, level = gSpecialVar_0x8005 * 10;
    gSpecialVar_Result = 0;
    ConvertIntToDecimalStringN(gStringVar2, level, STR_CONV_MODE_LEFT_ALIGN, 3);   // (Crystal's text_decimal wScriptVar)
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL);
        if (sp == SPECIES_NONE || sp == SPECIES_EGG)
            continue;
        if (GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL) > level)
        {
            gSpecialVar_Result = 1;
            return;
        }
    }
    if (gSpecialVar_0x8005 >= 7)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 sp = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL);
        for (j = 0; j < ARRAY_COUNT(sUbers); j++)
        {
            if (sp == sUbers[j] && GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL) < 70)
            {
                GetSpeciesName(gStringVar1, sp);
                gSpecialVar_Result = 2;
                return;
            }
        }
    }
}

// the challenge begins (group VAR_0x8005): Crystal's RESETDATA, SaveBattleTowerLevelGroup, CHOOSEREWARD and
// BATTLETOWER_CHALLENGE_IN_PROGRESS
static void StartChallenge(void)
{
    u8 g = gSpecialVar_0x8005;
    if (g < 1 || g > 10)
        g = 1;
    ResetSeen();
    memset(sPrevMons, 0xFF, sizeof(sPrevMons));
    SetField(ST_GROUP, ST_GROUP_SHIFT, g);
    SetField(ST_BEATEN, ST_BEATEN_SHIFT, 0);
    SetField(ST_REWARD, ST_REWARD_SHIFT, Random() % ARRAY_COUNT(sRewards));
    SetField(ST_CHALLENGE, 0, JBT_CHALLENGE_IN_PROGRESS);
}

// LoadOpponentTrainerAndPokemonWithOTSprite: a trainer not met in this streak, three of the group's POKéMON
static void LoadOpponent(void)
{
    u8 i, k, t, m, tries;
    const struct BattleTowerPokemon *group = sJbtMons[Group() - 1];

    for (tries = 0; tries < 200; tries++)
    {
        t = Random() % 70;
        for (i = 0; i < 7; i++)
        {
            if (Seen(i) == t)
                break;
        }
        if (i == 7)
            break;
    }
    sTrainer = t;
    if (Beaten() < 7)
        SetSeen(Beaten(), t);

    for (k = 0; k < 3; k++)
    {
        for (tries = 0; tries < 200; tries++)
        {
            bool8 ok = TRUE;
            m = Random() % 21;
            for (i = 0; i < k; i++)
            {
                if (group[sMons[i]].species == group[m].species
                 || (group[m].heldItem != ITEM_NONE && group[sMons[i]].heldItem == group[m].heldItem))
                    ok = FALSE;
            }
            for (i = 0; i < 3; i++)
            {
                if ((sPrevMons[0][i] < 21 && group[sPrevMons[0][i]].species == group[m].species)
                 || (sPrevMons[1][i] < 21 && group[sPrevMons[1][i]].species == group[m].species))
                    ok = FALSE;
            }
            if (ok)
                break;
        }
        sMons[k] = m;
    }
    memcpy(sPrevMons[1], sPrevMons[0], 3);
    memcpy(sPrevMons[0], sMons, 3);
    sTextIndex = Random() % (sJbtClasses[sJbtTrainers[t].cls].female ? ARRAY_COUNT(sJbtTextsF) : ARRAY_COUNT(sJbtTextsM));
    VarSet(VAR_OBJ_GFX_ID_0, sJbtClasses[sJbtTrainers[t].cls].objGfx);
}

static void GetText(void)
{
    BufferText(gSpecialVar_0x8005);
}

static void BuildEnemyParty(void)
{
    u8 i;
    const struct BattleTowerPokemon *group = sJbtMons[Group() - 1];
    ZeroEnemyPartyMons();
    for (i = 0; i < 3; i++)
    {
        struct BattleTowerPokemon mon = group[sMons[i] % 21];
        GetSpeciesName(mon.nickname, mon.species);   // Crystal shows the species' names
        CreateBattleTowerMon(&gEnemyParty[i], &mon);
    }
}

static void CB2_EndJohtoTowerBattle(void)
{
    sInBattle = FALSE;
    gSaveBlock2Ptr->optionsBattleStyle = sSavedStyle;
    HealPlayerParty();
    if (gBattleOutcome == B_OUTCOME_WON)
    {
        SetField(ST_BEATEN, ST_BEATEN_SHIFT, Beaten() + 1);
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
    ConvertIntToDecimalStringN(gStringVar1, Beaten() + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_DoJohtoTowerBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndJohtoTowerBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// RunBattleTowerTrainer: the party healed, SET style, the battle; afterwards (CB2_EndJohtoTowerBattle) healed again,
// VAR_RESULT 0 won / 1 not, STR_VAR_1 the next opponent's number
static void DoBattle(void)
{
    HealPlayerParty();
    sSavedStyle = gSaveBlock2Ptr->optionsBattleStyle;
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SET;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_HILL;
    gTrainerBattleOpponent_A = 1;
    gTrainerBattleOpponent_B = 0;
    sInBattle = TRUE;
    BuildEnemyParty();
    CreateTask(Task_DoJohtoTowerBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_TRAINER_HILL));
}

static void GetBeaten(void)
{
    gSpecialVar_Result = Beaten();
    ConvertIntToDecimalStringN(gStringVar1, Beaten() + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
}

// BattleTower_GiveReward + giveitem ITEM_FROM_MEM, BATTLETOWER_REWARD_QUANTITY: 0 given (its name in STR_VAR_2),
// 1 the BAG is full
static void GiveReward(void)
{
    u16 item = sRewards[((State() & ST_REWARD) >> ST_REWARD_SHIFT) % ARRAY_COUNT(sRewards)];
    SetField(ST_CHALLENGE, 0, JBT_WON_CHALLENGE);
    if (!CheckBagHasSpace(item, BATTLETOWER_REWARD_QUANTITY))
    {
        gSpecialVar_Result = 1;
        return;
    }
    AddBagItem(item, BATTLETOWER_REWARD_QUANTITY);
    CopyItemName(item, gStringVar2);
    SetField(ST_CHALLENGE, 0, JBT_RECEIVED_REWARD);
    gSpecialVar_Result = 0;
}

// "SAVE and end the session": BATTLETOWER_SAVED_AND_LEFT, saved to come back at the 1F (the dynamic warp, as the
// BATTLE FRONTIER's SaveGameFrontier); the script then resets
static void SaveAndQuit(void)
{
    SetField(ST_CHALLENGE, 0, JBT_SAVED_AND_LEFT);
    SetContinueGameWarpStatusToDynamicWarp();
    TrySavingData(SAVE_LINK);
    ClearContinueGameWarpStatus2();
}

static void GetGroup(void)
{
    gSpecialVar_Result = Group();
}

// TryQuickSave (before the BATTLE ROOM): the game saved where the player stands
static void QuickSave(void)
{
    gSpecialVar_Result = TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
}

static void (*const sFuncs[])(void) = {
    [JBT_FUNC_GET_STATE]         = GetState,
    [JBT_FUNC_SET_STATE]         = SetState,
    [JBT_FUNC_EXPLANATION_READ]  = CheckExplanationRead,
    [JBT_FUNC_SET_EXPLAINED]     = SetExplanationRead,
    [JBT_FUNC_CHECK_RULES]       = CheckRules,
    [JBT_FUNC_LEVEL_MENU]        = LevelMenu,
    [JBT_FUNC_LEVEL_CHECK]       = LevelCheck,
    [JBT_FUNC_START]             = StartChallenge,
    [JBT_FUNC_LOAD_OPPONENT]     = LoadOpponent,
    [JBT_FUNC_GET_TEXT]          = GetText,
    [JBT_FUNC_DO_BATTLE]         = DoBattle,
    [JBT_FUNC_GET_BEATEN]        = GetBeaten,
    [JBT_FUNC_GIVE_REWARD]       = GiveReward,
    [JBT_FUNC_SAVE_AND_QUIT]     = SaveAndQuit,
    [JBT_FUNC_GET_GROUP]         = GetGroup,
    [JBT_FUNC_QUICK_SAVE]        = QuickSave,
};

void CallJohtoBattleTowerFunc(void)
{
    if (gSpecialVar_0x8004 < ARRAY_COUNT(sFuncs))
        sFuncs[gSpecialVar_0x8004]();
}

// `mp jtower` (the test harness): the state in the log
void MP_JohtoTower_Log(void)
{
    printf("[test] jtower state %d group %d beaten %d explained %d trainer %d\n", State() & ST_CHALLENGE, Group(), Beaten(),
           (State() & ST_EXPLAINED) != 0, sTrainer);
}
