// Moemon Kai: KANTO's TRAINER TOWER as FireRed's (audit case 10): pret/pokefirered src/trainer_tower.c ported, its
// four challenges of eight floors (src/data/kanto_trainer_tower.h from tools/quetzal_port/gen_trainer_tower.py), the
// TIME ATTACK clock, the OWNER's prize and the Time Board. The scripts are FireRed's (data/scripts/kanto_trainer_tower.inc)
// and call CallKantoTrainerTowerFunc with VAR_0x8004 = TRAINER_TOWER_FUNC_*, as FireRed's call CallTrainerTowerFunc.
//
// What FireRed keeps in its SaveBlock1 (trainerTower[], towerChallengeId) lives in the save's vars here
// (include/constants/kanto_trainer_tower.h): no save layout change. The battles go through Emerald's TRAINER HILL
// battle type, whose hooks in trainer_hill.c (class, name, picture, words, music) ask MP_TrainerTower_InBattle first:
// the same rules FireRed's BATTLE_TYPE_TRAINER_TOWER had (no EXP, no money, nothing stolen).
#include <stdio.h>
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "battle_main.h"
#include "easy_chat.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/easy_chat.h"
#include "constants/layouts.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/event_objects.h"
#include "constants/kanto_trainer_tower.h"
#include "constants/qz_port.h"

struct KttTrainer
{
    u8 name[11];
    u8 facilityClass;
    u8 textColor;
    u16 speechBefore[6];
    u16 speechWin[6];
    u16 speechLose[6];
    u16 speechAfter[6];
    struct BattleTowerPokemon mons[PARTY_SIZE];
};

struct KttFloor
{
    u8 id;
    u8 floorIdx;
    u8 challengeType;
    u8 prize;
    struct KttTrainer trainers[MAX_TRAINERS_PER_FLOOR];
};

#include "../data/kanto_trainer_tower.h"

u8 GetSpecialBattleTransition(s32 id);
void MP_BufferItemPocket(void);   // (mp_bag.c)

// the state var's bits (FireRed's TrainerTower struct, towerChallengeId)
#define ST_CHALLENGE       0x0003
#define ST_SPOKE_TO_OWNER  0x0004
#define ST_CHECKED_TIME    0x0008
#define ST_LOST            0x0010
#define ST_PRIZE(c)        (0x0020 << (c))   // per challenge (FireRed's receivedPrize)

static bool8 sInBattle;

static const u16 sFloorMaps[MAX_TRAINER_TOWER_FLOORS + 1] = {
    MAP_KANTO_TRAINER_TOWER_1F, MAP_KANTO_TRAINER_TOWER_2F, MAP_KANTO_TRAINER_TOWER_3F, MAP_KANTO_TRAINER_TOWER_4F,
    MAP_KANTO_TRAINER_TOWER_5F, MAP_KANTO_TRAINER_TOWER_6F, MAP_KANTO_TRAINER_TOWER_7F, MAP_KANTO_TRAINER_TOWER_8F,
    MAP_KANTO_TRAINER_TOWER_ROOF,
};

// - 1 excludes Mixed challenge, which just uses one of the 3 other types
static const u16 sFloorLayouts[MAX_TRAINER_TOWER_FLOORS][NUM_TOWER_CHALLENGE_TYPES - 1] = {
    {LAYOUT_KANTO_TRAINER_TOWER_1F, LAYOUT_KANTO_TRAINER_TOWER_1F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_1F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_2F, LAYOUT_KANTO_TRAINER_TOWER_2F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_2F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_3F, LAYOUT_KANTO_TRAINER_TOWER_3F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_3F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_4F, LAYOUT_KANTO_TRAINER_TOWER_4F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_4F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_5F, LAYOUT_KANTO_TRAINER_TOWER_5F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_5F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_6F, LAYOUT_KANTO_TRAINER_TOWER_6F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_6F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_7F, LAYOUT_KANTO_TRAINER_TOWER_7F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_7F_KNOCKOUT},
    {LAYOUT_KANTO_TRAINER_TOWER_8F, LAYOUT_KANTO_TRAINER_TOWER_8F_DOUBLES, LAYOUT_KANTO_TRAINER_TOWER_8F_KNOCKOUT},
};

static const u16 sPrizeList[] = {
    ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CARBOS, ITEM_CALCIUM, ITEM_ZINC, ITEM_BRIGHT_POWDER, ITEM_WHITE_HERB,
    ITEM_MENTAL_HERB, ITEM_CHOICE_BAND, ITEM_KINGS_ROCK, ITEM_SCOPE_LENS, ITEM_METAL_COAT, ITEM_DRAGON_SCALE,
    ITEM_UP_GRADE,
};

// FireRed's sTrainerTowerEncounterMusic, with Kai's KANTO songs (battle_setup.c PlayTrainerEncounterMusic)
static const u16 sEncounterMusic[] = {
    [TRAINER_ENCOUNTER_MUSIC_MALE]        = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_FEMALE]      = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_GIRL]        = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS]  = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_INTENSE]     = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_COOL]        = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_AQUA]        = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_MAGMA]       = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_SWIMMER]     = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_TWINS]       = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_ELITE_FOUR]  = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_HIKER]       = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_INTERVIEWER] = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_RICH]        = MUS_RG_ENCOUNTER_BOY,
};

// The trainer only uses two Pokemon from the encoded pool, based on the current floor
static const u8 sSingleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 2}, {1, 3}, {2, 4}, {3, 5}, {4, 1}, {5, 2}, {0, 3}, {1, 4}
};

// Each trainer only uses one Pokemon from the encoded pool, based on the current floor
static const u8 sDoubleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 1}, {1, 3}, {2, 0}, {3, 4}, {4, 2}, {5, 2}, {0, 3}, {1, 5}
};

static const u8 sKnockoutChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][3] = {
    {0, 2, 4}, {1, 3, 5}, {2, 3, 1}, {3, 4, 0}, {4, 1, 2}, {5, 0, 3}, {0, 5, 2}, {1, 4, 5}
};

static const u8 sText_SpacePocket[] = _(" POCKET");
static const u8 sText_TimeBoard[] = _("TIME BOARD");
static const u8 sText_XMinYZSec[] = _("{STR_VAR_1} min. {STR_VAR_2}.{STR_VAR_3} sec.");
static const u8 sText_Single[] = _("SINGLE");
static const u8 sText_Double[] = _("DOUBLE");
static const u8 sText_Knockout[] = _("KNOCKOUT");
static const u8 sText_Mixed[] = _("MIXED");
static const u8 *const sChallengeTypeTexts[NUM_TOWER_CHALLENGE_TYPES] = {
    sText_Single, sText_Double, sText_Knockout, sText_Mixed,
};

static const struct WindowTemplate sTimeBoardWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 3,
    .tilemapTop = 1,
    .width = 24,
    .height = 12,
    .paletteNum = 15,
    .baseBlock = 0x100,
};

// ---- the save's vars

static u16 State(void)
{
    return VarGet(VAR_KANTO_TT_STATE);
}

static void SetState(u16 bits, bool32 on)
{
    u16 v = State();
    VarSet(VAR_KANTO_TT_STATE, on ? (v | bits) : (v & ~bits));
}

static u8 Challenge(void)
{
    return State() & ST_CHALLENGE;
}

static u32 *TimerPtr(void)
{
    return (u32 *)GetVarPointer(VAR_KANTO_TT_TIMER);
}

static u32 ReadU32(u16 var)
{
    u32 v;
    memcpy(&v, GetVarPointer(var), sizeof(v));
    return v;
}

static void WriteU32(u16 var, u32 v)
{
    memcpy(GetVarPointer(var), &v, sizeof(v));
}

static u32 Timer(void)
{
    return ReadU32(VAR_KANTO_TT_TIMER);
}

// a best time never set reads as FireRed's starting one (TRAINER_TOWER_MAX_TIME)
static u32 BestTime(u8 challenge)
{
    u32 v = ReadU32(VAR_KANTO_TT_BEST + challenge * 2);
    return (v == 0 || v > TRAINER_TOWER_MAX_TIME) ? TRAINER_TOWER_MAX_TIME : v;
}

static void SetBestTime(u8 challenge, u32 v)
{
    WriteU32(VAR_KANTO_TT_BEST + challenge * 2, v == 0 ? 1 : v);
}

// ---- the floor

// 0-7 for 1F-8F, 8 for the ROOF, -1 elsewhere
static s32 FloorIdx(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    s32 i;
    for (i = 0; i < (s32)ARRAY_COUNT(sFloorMaps); i++)
    {
        if (sFloorMaps[i] == map)
            return i;
    }
    return -1;
}

static const struct KttFloor *CurrFloor(void)
{
    s32 f = FloorIdx();
    if (f < 0 || f >= MAX_TRAINER_TOWER_FLOORS)
        f = 0;
    return sKttFloors[Challenge()][f];
}

bool8 MP_TrainerTower_InBattle(void)
{
    return sInBattle && (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL);
}

// ---- the battle hooks (trainer_hill.c): trainerId is the floor's trainer + 1

static const struct KttTrainer *BattleTrainer(u16 trainerId)
{
    u16 i = trainerId ? trainerId - 1 : 0;
    if (i >= MAX_TRAINERS_PER_FLOOR)
        i = 0;
    return &CurrFloor()->trainers[i];
}

u8 MP_TrainerTower_OpponentClass(u16 trainerId)
{
    return sKttClasses[BattleTrainer(trainerId)->facilityClass].trainerClass;
}

void MP_TrainerTower_OpponentName(u8 *dst, u16 trainerId)
{
    *StringCopyN(dst, BattleTrainer(trainerId)->name, 10) = EOS;
}

u16 MP_TrainerTower_FrontSpriteId(u16 trainerId)
{
    return sKttClasses[BattleTrainer(trainerId)->facilityClass].trainerPic;
}

u8 MP_TrainerTower_EncounterMusicId(u16 trainerId)
{
    return sKttClasses[BattleTrainer(trainerId)->facilityClass].music;
}

static void TT_ConvertEasyChatMessageToString(const u16 *ecWords, u8 *dest)
{
    s32 i;
    ConvertEasyChatWordsToString(dest, ecWords, 3, 2);
    if ((unsigned)GetStringWidth(FONT_NORMAL, dest, -1) > 196)
    {
        // Has to be printed 2x3
        ConvertEasyChatWordsToString(dest, ecWords, 2, 3);
        // Skip line 1
        i = 0;
        while (dest[i] != CHAR_NEWLINE && dest[i] != EOS)
            i++;
        if (dest[i] == EOS)
            return;
        i++;
        // Skip line 2
        while (dest[i] != CHAR_NEWLINE && dest[i] != EOS)
            i++;
        // Replace \n with \l at the end of line 2
        if (dest[i] == CHAR_NEWLINE)
            dest[i] = CHAR_PROMPT_SCROLL;
    }
}

// which: TRAINER_TOWER_TEXT_* (the trainer hill's TEXT_PLAYER_LOST / PLAYER_WON keep the same meaning)
void MP_TrainerTower_TrainerText(u8 which, u16 trainerId)
{
    const struct KttTrainer *t = BattleTrainer(trainerId);
    switch (which)
    {
    case TRAINER_TOWER_TEXT_INTRO:
        TT_ConvertEasyChatMessageToString(t->speechBefore, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_LOST:
        TT_ConvertEasyChatMessageToString(t->speechWin, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_WON:
        TT_ConvertEasyChatMessageToString(t->speechLose, gStringVar4);
        break;
    default:
        TT_ConvertEasyChatMessageToString(t->speechAfter, gStringVar4);
        break;
    }
}

// ---- FireRed's TRAINER_TOWER_FUNC_*

static void InitTrainerTowerFloor(void)
{
    s32 f = FloorIdx();
    if (f < 0 || f >= MAX_TRAINER_TOWER_FLOORS)
    {
        gSpecialVar_Result = 3; // Skip past usable challenge types
        SetCurrentMapLayout(LAYOUT_KANTO_TRAINER_TOWER_ROOF);
        return;
    }
    gSpecialVar_Result = CurrFloor()->challengeType;
    SetCurrentMapLayout(sFloorLayouts[f][gSpecialVar_Result]);
    switch (CurrFloor()->challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
        VarSet(VAR_OBJ_GFX_ID_1, sKttClasses[CurrFloor()->trainers[0].facilityClass].objGfx);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        VarSet(VAR_OBJ_GFX_ID_0, sKttClasses[CurrFloor()->trainers[0].facilityClass].objGfx);
        VarSet(VAR_OBJ_GFX_ID_3, sKttClasses[CurrFloor()->trainers[0].facilityClass].objGfx2);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        VarSet(VAR_OBJ_GFX_ID_2, sKttClasses[CurrFloor()->trainers[0].facilityClass].objGfx);
        VarSet(VAR_OBJ_GFX_ID_0, sKttClasses[CurrFloor()->trainers[1].facilityClass].objGfx);
        VarSet(VAR_OBJ_GFX_ID_1, sKttClasses[CurrFloor()->trainers[2].facilityClass].objGfx);
        break;
    }
}

static void BufferTowerOpponentSpeech(void)
{
    u16 trainerId = gSpecialVar_0x8006;
    if (trainerId >= MAX_TRAINERS_PER_FLOOR)
        trainerId = 0;
    MP_TrainerTower_TrainerText(gSpecialVar_0x8005, trainerId + 1);
}

static void CB2_EndTrainerTowerBattle(void)
{
    sInBattle = FALSE;
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_DoTrainerTowerBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndTrainerTowerBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

static s32 GetPartyMaxLevel(void)
{
    s32 topLevel = 0;
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_EGG)
        {
            s32 currLevel = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL);
            if (currLevel > topLevel)
                topLevel = currLevel;
        }
    }
    return topLevel;
}

static void CreateTowerMon(struct Pokemon *mon, const struct BattleTowerPokemon *src, u8 level)
{
    struct BattleTowerPokemon copy = *src;
    copy.level = level;
    CreateBattleTowerMon(mon, &copy);
}

static void BuildEnemyParty(void)
{
    const struct KttFloor *floor = CurrFloor();
    u16 trainerIdx = VarGet(VAR_TEMP_1);
    s32 level = GetPartyMaxLevel();
    u16 floorIdx = VarGet(VAR_KANTO_TT_FLOORS);
    s32 i;

    if (floorIdx >= MAX_TRAINER_TOWER_FLOORS)
        floorIdx = MAX_TRAINER_TOWER_FLOORS - 1;
    if (trainerIdx >= MAX_TRAINERS_PER_FLOOR)
        trainerIdx = 0;
    ZeroEnemyPartyMons();

    switch (floor->challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
    default:
        for (i = 0; i < 2; i++)
            CreateTowerMon(&gEnemyParty[i], &floor->trainers[trainerIdx].mons[sSingleBattleChallengeMonIdxs[floorIdx][i]], level);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        CreateTowerMon(&gEnemyParty[0], &floor->trainers[0].mons[sDoubleBattleChallengeMonIdxs[floorIdx][0]], level);
        CreateTowerMon(&gEnemyParty[1], &floor->trainers[1].mons[sDoubleBattleChallengeMonIdxs[floorIdx][1]], level);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        CreateTowerMon(&gEnemyParty[0], &floor->trainers[trainerIdx].mons[sKnockoutChallengeMonIdxs[floorIdx][trainerIdx]], level);
        break;
    }
}

static void DoTrainerTowerBattle(void)
{
    // FireRed's BATTLE_TYPE_TRAINER_TOWER is Emerald's TRAINER HILL battle here (see the top)
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_HILL;
    if (CurrFloor()->challengeType == CHALLENGE_TYPE_DOUBLE)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;

    // the trainer battled: the floor's trainer + 1 (a double's pair battles as its first)
    gTrainerBattleOpponent_A = (CurrFloor()->challengeType == CHALLENGE_TYPE_KNOCKOUT ? VarGet(VAR_TEMP_1) : 0) + 1;
    gTrainerBattleOpponent_B = 0;
    sInBattle = TRUE;
    BuildEnemyParty();
    CreateTask(Task_DoTrainerTowerBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_TRAINER_HILL));
}

static void TrainerTowerGetChallengeType(void)
{
    if (!gSpecialVar_0x8005)
        gSpecialVar_Result = CurrFloor()->challengeType;
}

static void TrainerTowerAddFloorCleared(void)
{
    VarSet(VAR_KANTO_TT_FLOORS, VarGet(VAR_KANTO_TT_FLOORS) + 1);
}

// So the player can safely go back through defeated floors to use the Poke Center (or exit challenge)
static void GetFloorAlreadyCleared(void)
{
    gSpecialVar_Result = !(FloorIdx() == (s32)VarGet(VAR_KANTO_TT_FLOORS));
}

static void StartTrainerTowerChallenge(void)
{
    u16 challenge = gSpecialVar_0x8005;
    if (challenge >= NUM_TOWER_CHALLENGE_TYPES)
        challenge = 0;
    VarSet(VAR_KANTO_TT_STATE, (State() & ~(ST_CHALLENGE | ST_SPOKE_TO_OWNER | ST_CHECKED_TIME | ST_LOST)) | challenge);
    VarSet(VAR_KANTO_TT_FLOORS, 0);
    WriteU32(VAR_KANTO_TT_TIMER, 0);
    SetTrainerHillVBlankCounter(TimerPtr());
}

static void GetOwnerState(void)
{
    ClearTrainerHillVBlankCounter();
    gSpecialVar_Result = 0;

    if (State() & ST_SPOKE_TO_OWNER)
        gSpecialVar_Result++;
    if ((State() & ST_PRIZE(Challenge())) && (State() & ST_CHECKED_TIME))
        gSpecialVar_Result++;

    SetState(ST_SPOKE_TO_OWNER, TRUE);
}

static void GiveChallengePrize(void)
{
    u16 itemId = sPrizeList[sKttFloors[Challenge()][0]->prize];

    if (State() & ST_PRIZE(Challenge()))
    {
        gSpecialVar_Result = 2;
    }
    else if (AddBagItem(itemId, 1) == TRUE)
    {
        CopyItemName(itemId, gStringVar2);
        // FireRed's STDSTRING_ITEMS_POCKET for its Text_PutItemAway, as the BAG's own pocket here
        gSpecialVar_0x8000 = itemId;
        MP_BufferItemPocket();
        StringAppend(gStringVar3, sText_SpacePocket);
        SetState(ST_PRIZE(Challenge()), TRUE);
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
}

static void CheckFinalTime(void)
{
    if (State() & ST_CHECKED_TIME)
    {
        gSpecialVar_Result = 2;
    }
    else if (BestTime(Challenge()) > Timer())
    {
        SetBestTime(Challenge(), Timer());
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
    SetState(ST_CHECKED_TIME, TRUE);
}

static void TrainerTowerResumeTimer(void)
{
    if (!(State() & ST_SPOKE_TO_OWNER))
    {
        if (Timer() >= TRAINER_TOWER_MAX_TIME)
            WriteU32(VAR_KANTO_TT_TIMER, TRAINER_TOWER_MAX_TIME);
        else
            SetTrainerHillVBlankCounter(TimerPtr());
    }
}

// the battle moved the save (MoveSaveBlocks_ResetHeap) and stopped the clock: FireRed's InitTrainerTowerBattleStruct
// starts it again on the moved save (trainer_hill.c InitTrainerHillBattleStruct)
void MP_TrainerTower_BattleClock(void)
{
    TrainerTowerResumeTimer();
}

static void TrainerTowerSetPlayerLost(void)
{
    SetState(ST_LOST, TRUE);
}

static void GetTrainerTowerChallengeStatus(void)
{
    if (State() & ST_LOST)
    {
        SetState(ST_LOST, FALSE);
        gSpecialVar_Result = TT_STATUS_LOST;
    }
    else
    {
        gSpecialVar_Result = TT_STATUS_NORMAL;
    }
}

static void PrintTowerTime(u32 frames)
{
    s32 minutes, seconds, centiseconds;

    minutes = frames / (60 * 60);
    frames %= (60 * 60);
    seconds = frames / 60;
    frames %= 60;
    centiseconds = frames * 168 / 100;

    ConvertIntToDecimalStringN(gStringVar1, minutes, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, seconds, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, centiseconds, STR_CONV_MODE_LEADING_ZEROS, 2);
}

static void GetCurrentTime(void)
{
    if (Timer() >= TRAINER_TOWER_MAX_TIME)
    {
        ClearTrainerHillVBlankCounter();
        WriteU32(VAR_KANTO_TT_TIMER, TRAINER_TOWER_MAX_TIME);
    }
    PrintTowerTime(Timer());
}

// the Time Board at the reception (FireRed's ShowBattleRecords page for the TOWER: PrintTrainerTowerRecords)
static void ShowResultsBoard(void)
{
    u8 windowId;
    s32 i;

    windowId = AddWindow(&sTimeBoardWindowTemplate);
    DrawStdWindowFrame(windowId, FALSE);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, sText_TimeBoard,
                                (24 * 8 - GetStringWidth(FONT_NORMAL, sText_TimeBoard, 0)) / 2, 1, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PrintTowerTime(BestTime(i));
        StringExpandPlaceholders(gStringVar4, sText_XMinYZSec);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, sChallengeTypeTexts[i], 8, 22 + 18 * i, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 80, 22 + 18 * i, TEXT_SKIP_DRAW, NULL);
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    VarSet(VAR_TEMP_1, windowId);
}

static void CloseResultsBoard(void)
{
    u8 windowId = VarGet(VAR_TEMP_1);
    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
}

static void TrainerTowerGetDoublesEligiblity(void)
{
    gSpecialVar_Result = GetMonsStateToDoubles();
}

// FireRed's e-Reader floors could stop short of the eighth; its own four never do
static void TrainerTowerGetNumFloors(void)
{
    gSpecialVar_Result = FALSE;
}

// Dummied in FireRed too
static void ShouldWarpToCounter(void)
{
    gSpecialVar_Result = FALSE;
}

static void PlayTrainerTowerEncounterMusic(void)
{
    u16 idx = VarGet(VAR_TEMP_1);
    u8 music;
    if (idx >= MAX_TRAINERS_PER_FLOOR)
        idx = 0;
    music = sKttClasses[CurrFloor()->trainers[idx].facilityClass].music;
    PlayNewMapMusic(music < ARRAY_COUNT(sEncounterMusic) && sEncounterMusic[music] ? sEncounterMusic[music] : MUS_RG_ENCOUNTER_BOY);
}

static void HasSpokenToOwner(void)
{
    gSpecialVar_Result = (State() & ST_SPOKE_TO_OWNER) != 0;
}

static void (*const sTrainerTowerFunctions[])(void) = {
    [TRAINER_TOWER_FUNC_INIT_FLOOR]             = InitTrainerTowerFloor,
    [TRAINER_TOWER_FUNC_GET_SPEECH]             = BufferTowerOpponentSpeech,
    [TRAINER_TOWER_FUNC_DO_BATTLE]              = DoTrainerTowerBattle,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE]     = TrainerTowerGetChallengeType,
    [TRAINER_TOWER_FUNC_CLEARED_FLOOR]          = TrainerTowerAddFloorCleared,
    [TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED]      = GetFloorAlreadyCleared,
    [TRAINER_TOWER_FUNC_START_CHALLENGE]        = StartTrainerTowerChallenge,
    [TRAINER_TOWER_FUNC_GET_OWNER_STATE]        = GetOwnerState,
    [TRAINER_TOWER_FUNC_GIVE_PRIZE]             = GiveChallengePrize,
    [TRAINER_TOWER_FUNC_CHECK_FINAL_TIME]       = CheckFinalTime,
    [TRAINER_TOWER_FUNC_RESUME_TIMER]           = TrainerTowerResumeTimer,
    [TRAINER_TOWER_FUNC_SET_LOST]               = TrainerTowerSetPlayerLost,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS]   = GetTrainerTowerChallengeStatus,
    [TRAINER_TOWER_FUNC_GET_TIME]               = GetCurrentTime,
    [TRAINER_TOWER_FUNC_SHOW_RESULTS]           = ShowResultsBoard,
    [TRAINER_TOWER_FUNC_CLOSE_RESULTS]          = CloseResultsBoard,
    [TRAINER_TOWER_FUNC_CHECK_DOUBLES]          = TrainerTowerGetDoublesEligiblity,
    [TRAINER_TOWER_FUNC_GET_NUM_FLOORS]         = TrainerTowerGetNumFloors,
    [TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER] = ShouldWarpToCounter,
    [TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC]        = PlayTrainerTowerEncounterMusic,
    [TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE]     = HasSpokenToOwner,
};

void CallKantoTrainerTowerFunc(void)
{
    if (gSpecialVar_0x8004 < ARRAY_COUNT(sTrainerTowerFunctions))
        sTrainerTowerFunctions[gSpecialVar_0x8004]();
}

// SEVEN ISLAND's TRAINER TOWER grounds (their map's ON_TRANSITION): the clock stops once the player walks out, so it
// never runs on (FireRed leaves it running; harmless there, but here it would carry into another save file)
void MP_TrainerTower_Leave(void)
{
    ClearTrainerHillVBlankCounter();
}

// `mp ttower` (the test harness): the state in the log
void MP_TrainerTower_Log(void)
{
    printf("[test] ttower challenge %d floors %d state 0x%04X timer %u best %u %u %u %u floor %d\n",
           Challenge(), VarGet(VAR_KANTO_TT_FLOORS), State(), (unsigned)Timer(), (unsigned)BestTime(0),
           (unsigned)BestTime(1), (unsigned)BestTime(2), (unsigned)BestTime(3), (int)FloorIdx());
}
