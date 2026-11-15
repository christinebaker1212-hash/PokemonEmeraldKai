// Moemon Kai: SINNOH's BATTLE HALL, by Platinum's rules (its overlay104 battle_hall*.c: the numbers, not its code). One
// POKéMON (Lv 30+, no legend) battles one opponent at a time; the player picks the type of each opponent, and each type
// has ten ranks that rise with every win against it. The opponent's level comes from Platinum's formula (the player's
// level less three times its square root, plus the rank and the number of types battled), its IVs from the rank (8 +
// 2 a rank), its kind from that type's POKéMON grouped by base stat total in four (ranks 1-2 the weakest group, 9-10
// the strongest two). Opponents are the game's own frontier trainers; the battle is a frontier battle (no EXP, no
// prize, no whiteout), the POKéMON healed after each. Every 10 wins give BATTLE POINTS. The streak and the ranks are
// the chosen POKéMON's (a new one starts over) and kept in "<save>.region" (chunk HALL, mp_region.c).
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "s3d.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_transition.h"
#include "event_data.h"
#include "main.h"
#include "mp.h"
#include "mp_region.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/battle_frontier.h"
#include "constants/battle_frontier_trainers.h"
#include "constants/script_menu.h"
#include "constants/field_specials.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "sinnoh_tower.h"

u8 Sinnoh_BrainNext(u8 brain);   // (s3d_frontier.c: the Frontier Brains, 0 the HALL's)
void Sinnoh_BrainBattle(u8 kind);

extern const u8 *gQzScrollOptions[];
extern u8 gQzScrollCount;
extern u16 (*gQzScrollResult)(u16 choice);
void ShowScrollableMultichoice(void);
extern const u16 gFrontierBannedSpecies[];
extern const struct BattleFrontierTrainer gBattleFrontierTrainers[];

#define HALL_RANKS 10
#define HALL_BATTLES_PER_ROUND 10
#define HALL_RECORDS 640

static struct HallSave
{
    u16 species;          // the POKéMON the record is for
    u32 personality;
    u8 ranks[NUMBER_OF_MON_TYPES];
    u16 streak, best;
    u8 prints;            // the HALL MATRON's prints won (bit 0 Silver, 1 Gold; added 1.114.0)
    u8 recordsSeeded;     // (1.199.10: the record before the table, put in it once)
    u16 recordCount;      // each kind's best streak, as Platinum's BattleHallWinRecords (the record keeper, SERENA and
    struct { u16 species, best; } records[HALL_RECORDS];   // WINSTON count them; added 1.199.10, older saves load short)
    u16 lastStreak;       // the last challenge's streak where it ended (Platinum's latest streak: the reporter at 50)
} sHall;

// Platinum's VAR_BATTLE_HALL_LOBBY_LOAD_ACTION (tools/sinnoh/ids.py): 1 the lobby's people are looked at again as it
// loads (the HALL's OnLoad: the day's visitor, the reporter). The game's own value 6: a challenge ended, the next load
// of the lobby is that one (Sinnoh_HallLobbyLoad)
#define VAR_SINNOH_HALL_LOBBY_LOAD 0x5197
#define HALL_LOBBY_PENDING 6

static struct
{
    u8 slot, type, opponent;
    struct Pokemon saved[PARTY_SIZE];
} sRun;

static const u8 sTypes[] = {TYPE_NORMAL, TYPE_FIRE, TYPE_WATER, TYPE_ELECTRIC, TYPE_GRASS, TYPE_ICE, TYPE_FIGHTING,
                            TYPE_POISON, TYPE_GROUND, TYPE_FLYING, TYPE_PSYCHIC, TYPE_BUG, TYPE_ROCK, TYPE_GHOST,
                            TYPE_DRAGON, TYPE_DARK, TYPE_STEEL, TYPE_FAIRY};

void Sinnoh_HallReset(void)
{
    memset(&sHall, 0, sizeof(sHall));
}

const void *Sinnoh_HallData(u32 *size)
{
    *size = sizeof(sHall);
    return &sHall;
}

static void KeepRecord(u16 species, u16 streak)
{
    u32 i;
    if (species == SPECIES_NONE || streak == 0)
        return;
    for (i = 0; i < sHall.recordCount; i++)
        if (sHall.records[i].species == species)
        {
            if (sHall.records[i].best < streak)
                sHall.records[i].best = streak;
            return;
        }
    if (sHall.recordCount < HALL_RECORDS)
    {
        sHall.records[sHall.recordCount].species = species;
        sHall.records[sHall.recordCount++].best = streak;
    }
}

void Sinnoh_HallLoad(const void *data, u32 size)
{
    Sinnoh_HallReset();
    memcpy(&sHall, data, size < sizeof(sHall) ? size : sizeof(sHall));
    if (sHall.recordCount > HALL_RECORDS)
        sHall.recordCount = 0;
    if (!sHall.recordsSeeded)
    {
        sHall.recordsSeeded = 1;   // (a save from before the table: its one record goes in)
        KeepRecord(sHall.species, sHall.best);
    }
}

// Platinum's BATTLE HALL records, summed over every kind: GetBattleHallTotalSinglesRecord (the daily visitors, at most
// 10000) and the record keeper's total
static u32 HallTotal(void)
{
    u32 i, total = 0;
    for (i = 0; i < sHall.recordCount; i++)
        total += sHall.records[i].best;
    return total;
}

u16 Sinnoh_HallTotalRecord(void)
{
    u32 total = HallTotal();
    return total > 10000 ? 10000 : total;
}

// GetNumSpeciesWithBattleHallRecords: how many kinds have a record (SERENA and WINSTON)
u16 Sinnoh_HallKindsWithRecords(void)
{
    return sHall.recordCount;
}

// GetBattleHallRecordKeeperStats (Platinum's scrcmd_battle_hall.c): the total into TEXT slots 1-4 with the milestone
// passed, the next one and the BP earned; *next the milestone reached so far; the answer 0 no record, 1 BP given,
// 2 none this time, 3 every milestone met
static const struct { u32 record; u16 bp; } sMilestones[] = {
    {10, 1}, {30, 3}, {50, 5}, {100, 5}, {150, 5}, {200, 5}, {250, 5}, {300, 5}, {350, 5}, {400, 5}, {450, 5},
    {500, 10}, {600, 10}, {700, 10}, {800, 10}, {900, 10}, {1000, 10}, {1200, 30}, {1400, 30}, {1600, 30}, {1800, 30},
    {2000, 50}, {2500, 50}, {3000, 50}, {3500, 50}, {4000, 50}, {4500, 50}, {5000, 50}, {5500, 50}, {6000, 50},
    {6500, 50}, {7000, 50}, {7500, 50}, {8000, 50}, {8500, 50}, {9000, 50}, {9500, 50}, {10000, 100}, {20000, 200},
    {30000, 300}, {40000, 400}, {50000, 500}, {60000, 600}, {70000, 700}, {80000, 800}, {90000, 900}, {100000, 1000},
};

u16 Sinnoh_HallRecordKeeper(u16 *next, u8 *(*slot)(u8))
{
    u32 total = HallTotal(), i, reached = 0, first = *next;
    u16 bp = 0;
    ConvertIntToDecimalStringN(slot(1), total, STR_CONV_MODE_LEFT_ALIGN, 6);
    for (i = *next; i < ARRAY_COUNT(sMilestones); i++)
        if (sMilestones[i].record <= total)
        {
            bp += sMilestones[i].bp;
            (*next)++;
            reached = i;
        }
    if (bp != 0)
    {
        gSaveBlock2Ptr->frontier.battlePoints += bp;
        if (gSaveBlock2Ptr->frontier.battlePoints > MAX_BATTLE_FRONTIER_POINTS)
            gSaveBlock2Ptr->frontier.battlePoints = MAX_BATTLE_FRONTIER_POINTS;
        Sinnoh_TvBattlePoints(bp);
    }
    MPNet_Log("hall: the record keeper: total %u, milestone %u, %u BP", total, *next, bp);
    if (total == 0)
        return 0;
    if (first >= ARRAY_COUNT(sMilestones))
        return 3;
    ConvertIntToDecimalStringN(slot(2), sMilestones[reached].record, STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(slot(3), sMilestones[*next < ARRAY_COUNT(sMilestones) ? *next : ARRAY_COUNT(sMilestones) - 1].record,
                               STR_CONV_MODE_LEFT_ALIGN, 6);
    ConvertIntToDecimalStringN(slot(4), bp, STR_CONV_MODE_LEFT_ALIGN, 4);
    return first != *next ? 1 : 2;
}

static u8 TypeCount(void)
{
    return MP_Gen_Cap(MP_Region_Current()) >= 6 ? ARRAY_COUNT(sTypes) : ARRAY_COUNT(sTypes) - 1;   // (FAIRY from GEN 6)
}

static bool8 IsLegend(u16 species)
{
    u32 i;
    for (i = 0; gFrontierBannedSpecies[i] != 0xFFFF; i++)
        if (gFrontierBannedSpecies[i] == species)
            return TRUE;
    return gSpeciesInfo[species].catchRate <= 5;   // (the legends and myths: Platinum bans them all)
}

static u16 Bst(u16 s)
{
    const struct SpeciesInfo *i = &gSpeciesInfo[s];
    return i->baseHP + i->baseAttack + i->baseDefense + i->baseSpeed + i->baseSpAttack + i->baseSpDefense;
}

// The record's STR_VAR_1 streak, STR_VAR_2 best, STR_VAR_3 the POKéMON's name (VAR_RESULT: a record at all)
u16 Sinnoh_HallInfo(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sHall.streak, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, sHall.best, STR_CONV_MODE_LEFT_ALIGN, 4);
    if (sHall.species != SPECIES_NONE)
        GetSpeciesName(gStringVar3, sHall.species);
    return gSpecialVar_Result = sHall.species != SPECIES_NONE;
}

// The POKéMON in party slot VAR_0x8004 takes the challenge: VAR_RESULT 0 ready, 1 below Lv 30, 2 a legend or an EGG
// (a POKéMON other than the record's starts a new record)
u16 Sinnoh_HallSetMon(void)
{
    struct Pokemon *mon = &gPlayerParty[gSpecialVar_0x8004 < PARTY_SIZE ? gSpecialVar_0x8004 : 0];
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    if (GetMonData(mon, MON_DATA_IS_EGG) || IsLegend(species) || !MP_Gen_Allowed(species))
        return gSpecialVar_Result = 2;
    if (GetMonData(mon, MON_DATA_LEVEL) < 30)
        return gSpecialVar_Result = 1;
    sRun.slot = gSpecialVar_0x8004;
    if (sHall.species != species || sHall.personality != personality)
    {
        // (the prints and every kind's records are the player's, not the POKéMON's: only the POKéMON's own go)
        memset(&sHall.ranks, 0, sizeof(sHall.ranks));
        sHall.streak = sHall.best = 0;
        sHall.species = species;
        sHall.personality = personality;
    }
    GetSpeciesName(gStringVar1, species);
    MPNet_Log("hall: %d takes the challenge (streak %d)", species, sHall.streak);
    return gSpecialVar_Result = 0;
}

static u8 sRows[ARRAY_COUNT(sTypes)][24];

static u16 TypePicked(u16 row)
{
    if (row >= TypeCount())
        return MULTI_B_PRESSED;
    sRun.type = sTypes[row];
    StringCopy(gStringVar1, gTypeNames[sRun.type]);
    ConvertIntToDecimalStringN(gStringVar2, sHall.ranks[sRun.type] + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    return row;
}

// The types to battle, each with its rank (waitstate; VAR_RESULT the row or MULTI_B_PRESSED: stop)
u16 Sinnoh_HallTypeMenu(void)
{
    static const u8 sRank[] = _(" RANK ");
    static const u8 sDone[] = _(" MAX");
    u32 i, n = TypeCount();
    for (i = 0; i < n; i++)
    {
        u8 r = sHall.ranks[sTypes[i]];
        StringCopy(sRows[i], gTypeNames[sTypes[i]]);
        if (r >= HALL_RANKS)
            StringAppend(sRows[i], sDone);
        else
        {
            StringAppend(sRows[i], sRank);
            ConvertIntToDecimalStringN(sRows[i] + StringLength(sRows[i]), r + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
        }
        gQzScrollOptions[i] = sRows[i];
    }
    gQzScrollOptions[i] = gText_Exit;
    gQzScrollCount = i + 1;
    gQzScrollResult = TypePicked;
    gSpecialVar_0x8004 = SCROLL_MULTI_QZ_DYNAMIC;
    ShowScrollableMultichoice();
    return 0;
}

// Platinum's opponent level (BattleHall_CalcOpponentsLevel)
static u8 OpponentLevel(u8 playerLevel, u8 type)
{
    float s = sqrtf(playerLevel), base = playerLevel - 3 * s, rank = sHall.ranks[type] < HALL_RANKS ? sHall.ranks[type] : HALL_RANKS - 1;
    float rankFactor = playerLevel / (5 * s) < 1.0f ? rank : rank * playerLevel / (5 * s), types = 0, level;
    u32 i;
    s32 out;
    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
        if (i == type || sHall.ranks[i] > 0)
            types += 1;
    if (types > 0)
        types -= 1;
    level = base + rankFactor + types * 0.5f;
    out = (s32)level;
    if (level != (float)out)
        out++;
    if (out > playerLevel)
        out = playerLevel;
    if (out > MAX_LEVEL)
        out = MAX_LEVEL;
    return out < 1 ? 1 : out;
}

// A POKéMON of TYPE from the rank's groups (Platinum's: the type's POKéMON in four by base stat total). BRAIN (Platinum's
// BattleHall_PickNextOpponentPokemon for the HALL MATRON): 1 Silver, from the group the player's own POKéMON is in
// (the strongest when it isn't of the type), 2 Gold, from the strongest group
static u16 OpponentSpecies(u8 type, u8 rank, u16 not, u8 brain)
{
    static u16 sPool[NUM_SPECIES];
    static const u8 sGroups[HALL_RANKS][2] = {{0, 0}, {0, 0}, {0, 1}, {0, 1}, {0, 1}, {1, 2}, {1, 2}, {1, 2}, {2, 3}, {2, 3}};
    u32 n = 0, i, j, lo, hi;
    for (i = 1; i < NUM_SPECIES; i++)
    {
        const struct SpeciesInfo *si = &gSpeciesInfo[i];
        if (si->baseHP == 0 || i == not || IsLegend(i) || !MP_Gen_Allowed(i))
            continue;
        if (si->types[0] == type || si->types[1] == type)
            sPool[n++] = i;
    }
    if (n == 0)
        return SPECIES_NONE;
    for (i = 1; i < n; i++)   // (by base stat total)
        for (j = i; j > 0 && Bst(sPool[j - 1]) > Bst(sPool[j]); j--)
        {
            u16 t = sPool[j];
            sPool[j] = sPool[j - 1];
            sPool[j - 1] = t;
        }
    if (rank >= HALL_RANKS)
        rank = HALL_RANKS - 1;
    lo = n * sGroups[rank][0] / 4;
    hi = n * (sGroups[rank][1] + 1) / 4;
    if (brain)
    {
        u32 group = 3;
        for (i = 0; brain == 1 && i < n; i++)
            if (sPool[i] == not || Bst(sPool[i]) >= Bst(not))
            {
                group = i * 4 / n;
                break;
            }
        lo = n * group / 4;
        hi = n * (group + 1) / 4;
    }
    if (hi <= lo)
        hi = lo + 1;
    if (hi > n)
        hi = n;
    return sPool[lo + Random() % (hi - lo)];
}

static void CB2_EndHallBattle(void)
{
    struct Pokemon mon = gPlayerParty[0];
    bool8 won = gBattleOutcome == B_OUTCOME_WON;
    memcpy(gPlayerParty, sRun.saved, sizeof(gPlayerParty));
    gPlayerParty[sRun.slot] = mon;
    {
        // (Platinum heals the POKéMON after every battle)
        u16 hp = GetMonData(&gPlayerParty[sRun.slot], MON_DATA_MAX_HP);
        u32 status = 0;
        SetMonData(&gPlayerParty[sRun.slot], MON_DATA_HP, &hp);
        SetMonData(&gPlayerParty[sRun.slot], MON_DATA_STATUS, &status);
        MonRestorePP(&gPlayerParty[sRun.slot]);
    }
    CalculatePlayerPartyCount();
    VarSet(VAR_FRONTIER_FACILITY, 0);
    MP_MUSIC_SCENE_RELEASE();
    gSpecialVar_Result = won;
    MPNet_Log("hall: battle %s", won ? "won" : "lost");
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_StartHallBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndHallBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// The battle against the type picked (waitstate; VAR_RESULT TRUE: won)
u16 Sinnoh_HallBattle(void)
{
    struct Pokemon *mine = &gPlayerParty[sRun.slot];
    u8 rank = sHall.ranks[sRun.type], level = OpponentLevel(GetMonData(mine, MON_DATA_LEVEL), sRun.type);
    u8 brain = Sinnoh_BrainNext(0), ivs = 8 + 2 * (rank < HALL_RANKS ? rank : HALL_RANKS - 1);
    u16 species = OpponentSpecies(sRun.type, rank, GetMonData(mine, MON_DATA_SPECIES), brain);
    u32 trainer;
    Sinnoh_BrainBattle(brain);
    if (brain)
    {
        // the HALL MATRON: the player's level, every IV full (BattleHall_GetOpponentLevel, BattleHall_LoadOpponentMonData)
        level = GetMonData(mine, MON_DATA_LEVEL);
        ivs = MAX_PER_STAT_IVS;
    }
    if (species == SPECIES_NONE)
    {
        gSpecialVar_Result = TRUE;
        ScriptContext_Enable();
        return 0;
    }
    // the player's one POKéMON alone; the opponent a frontier trainer of the rank's strength
    memcpy(sRun.saved, gPlayerParty, sizeof(gPlayerParty));
    ZeroPlayerPartyMons();
    gPlayerParty[0] = sRun.saved[sRun.slot];
    CalculatePlayerPartyCount();
    ZeroEnemyPartyMons();
    CreateMon(&gEnemyParty[0], species, level, ivs, FALSE, 0, OT_ID_RANDOM_NO_SHINY, 0);
    trainer = (rank < HALL_RANKS ? rank : HALL_RANKS - 1) * (FRONTIER_TRAINERS_COUNT / HALL_RANKS)
            + Random() % (FRONTIER_TRAINERS_COUNT / HALL_RANKS);
    if (brain)
    {
        Sinnoh_Frontier_SetOpponent(TRAINER_SINNOH_FRONTIER_A, Sinnoh_Frontier_Brain(0, brain - 1));
        trainer = TRAINER_SINNOH_FRONTIER_A;
    }
    gFacilityTrainers = gBattleFrontierTrainers;
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    gTrainerBattleOpponent_A = trainer;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER;   // (one frontier flag: all of them were the PALACE's and ARENA's rules too)
    MPNet_Log("hall: type %d rank %d: species %d Lv%d (trainer %d)", sRun.type, rank + 1, species, level, trainer);
    CreateTask(Task_StartHallBattle, 1);
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

// A win: the type's rank up, the streak on. STR_VAR_1 the streak, STR_VAR_2 the type's new rank; VAR_RESULT the
// BATTLE POINTS given (every 10 wins: Platinum's round), STR_VAR_3 their number
u16 Sinnoh_HallWon(void)
{
    u16 bp = 0;
    if (sHall.ranks[sRun.type] < HALL_RANKS)
        sHall.ranks[sRun.type]++;
    sHall.streak++;
    if (sHall.streak > sHall.best)
        sHall.best = sHall.streak;
    KeepRecord(sHall.species, sHall.streak);   // (the kind's best: the record keeper's total)
    if (sHall.streak % HALL_BATTLES_PER_ROUND == 0)
    {
        bp = 3 + sHall.streak / HALL_BATTLES_PER_ROUND;
        if (bp > 10)
            bp = 10;
        gSaveBlock2Ptr->frontier.battlePoints += bp;
        if (gSaveBlock2Ptr->frontier.battlePoints > MAX_BATTLE_FRONTIER_POINTS)
            gSaveBlock2Ptr->frontier.battlePoints = MAX_BATTLE_FRONTIER_POINTS;
        Sinnoh_TvBattlePoints(bp);   // (the TV's RECORDS: s3d_tv.c)
    }
    ConvertIntToDecimalStringN(gStringVar1, sHall.streak, STR_CONV_MODE_LEFT_ALIGN, 4);
    ConvertIntToDecimalStringN(gStringVar2, sHall.ranks[sRun.type], STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, bp, STR_CONV_MODE_LEFT_ALIGN, 2);
    MPNet_Log("hall: streak %d, type %d rank %d, %d BP", sHall.streak, sRun.type, sHall.ranks[sRun.type], bp);
    return gSpecialVar_Result = bp;
}

// The challenge ends (a loss or stopping): STR_VAR_1 the streak; the ranks start over
u16 Sinnoh_HallEnd(void)
{
    ConvertIntToDecimalStringN(gStringVar1, sHall.streak, STR_CONV_MODE_LEFT_ALIGN, 4);
    MPNet_Log("hall: the challenge ends at %d (best %d)", sHall.streak, sHall.best);
    if (sRun.slot < PARTY_SIZE)
        Sinnoh_TvFrontierHall(&gPlayerParty[sRun.slot]);   // (Platinum's FrontierScrCmd_C7: the TV's interview subject)
    sHall.lastStreak = sHall.streak;
    sHall.streak = 0;
    memset(sHall.ranks, 0, sizeof(sHall.ranks));
    VarSet(VAR_SINNOH_HALL_LOBBY_LOAD, HALL_LOBBY_PENDING);
    return 0;
}

// The HALL's lobby loads (s3d_scrcmd.c Sinnoh_PlatinumMapScripts, before its map scripts): after a challenge, once,
// Platinum's OnLoad looks at the lobby's people again (as it does coming back from its corridor)
void Sinnoh_HallLobbyLoad(void)
{
    u16 v = VarGet(VAR_SINNOH_HALL_LOBBY_LOAD);
    if (v == HALL_LOBBY_PENDING)
        VarSet(VAR_SINNOH_HALL_LOBBY_LOAD, 1);
    else if (v == 1)
        VarSet(VAR_SINNOH_HALL_LOBBY_LOAD, 0);
}

// (s3d_misc.c op 55, CheckIfBattleHallStreakIs50) the streak the last challenge ended at
u16 Sinnoh_HallLastStreak(void)
{
    return sHall.lastStreak;
}

// (tests: `mp hall`)
void Sinnoh_HallDebug(void)
{
    printf("[test] hall: species %d streak %d best %d ranks", sHall.species, sHall.streak, sHall.best);
    {
        u32 i;
        for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
            printf(" %d", sHall.ranks[i]);
    }
    printf("\n");
}

// (s3d_misc.c, the VILLA's order form) the HALL's best streak
u16 Sinnoh_HallBest(void)
{
    return sHall.best;
}

// (s3d_frontier.c, the Frontier Brains) the streak, and the HALL MATRON's prints
u16 Sinnoh_HallStreak(void)
{
    return sHall.streak;
}

u8 *Sinnoh_HallPrints(void)
{
    return &sHall.prints;
}

void Sinnoh_HallSetStreak(u16 streak)   // (tests)
{
    sHall.streak = streak;
}

// (the records boards: s3d_frontier.c) a kind's best, and the POKéMON the HALL knows last
u16 Sinnoh_HallSpeciesBest(u16 species)
{
    u32 i;
    for (i = 0; i < sHall.recordCount; i++)
        if (sHall.records[i].species == species)
            return sHall.records[i].best;
    return 0;
}

u16 Sinnoh_HallSpecies(void)
{
    return sHall.species;
}

// Platinum's CallBattleHallLobbyFunction on the records board: 0 BH_LOBBY_FUNC_CHECK_STREAK_ACTIVE, 1
// BH_LOBBY_FUNC_GET_CURRENT_STREAK_SPECIES
u16 Sinnoh_HallLobbyFunction(u16 fn)
{
    if (fn == 0)
        return sHall.species != SPECIES_NONE && sHall.streak > 0;
    return sHall.species;
}

// Platinum's ShowBattleHallRecordMonSelectionMenu: the kinds with a record whose names begin with letter group GROUP
// (0 ABC ... 8 YZ), by name, into the script's menu (s3d_scrcmd.c), the chosen one's species into VAR (MENU_CANCEL
// 0xFFFE: none)
void Sinnoh_HallRecordMenu(u16 group, u16 var)
{
    void Sinnoh_MenuBegin(u16 var, bool8 canExit);
    void Sinnoh_MenuAddLine(const u8 *text, u16 value);
    u16 order[HALL_RECORDS];
    u32 i, j, n = 0;
    u8 a[POKEMON_NAME_LENGTH + 1], b[POKEMON_NAME_LENGTH + 1];
    for (i = 0; i < sHall.recordCount; i++)
    {
        u8 c;
        GetSpeciesName(a, sHall.records[i].species);
        c = a[0];
        if (c >= CHAR_a && c <= CHAR_z)
            c = c - CHAR_a + CHAR_A;
        if (c < CHAR_A || c > CHAR_Z || (c - CHAR_A) / 3 != group)
            continue;
        for (j = n; j > 0; j--)   // (by name)
        {
            GetSpeciesName(b, order[j - 1]);
            if (StringCompare(b, a) <= 0)
                break;
            order[j] = order[j - 1];
        }
        order[j] = sHall.records[i].species;
        n++;
    }
    Sinnoh_MenuBegin(var, TRUE);
    for (i = 0; i < n && i < 22; i++)
    {
        GetSpeciesName(a, order[i]);
        Sinnoh_MenuAddLine(a, order[i]);
    }
    Sinnoh_MenuAddLine(S3D_TextByName("BattleFrontierRecords_Text_Cancel2"), 0xFFFE);
    MPNet_Log("hall: the records board's menu, group %d: %d kinds", group, n);
}

void Sinnoh_HallSetRecord(u16 species, u16 best)   // (tests: `mp hallrecord SPECIES BEST`)
{
    KeepRecord(species, best);
}
