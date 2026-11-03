// SINNOH's BATTLE TOWER as Platinum's (docs/SINNOH_TOWER_PORT.md): Platinum's frontier trainers and sets
// (src/data/sinnoh_frontier.h, tools/sinnoh/gen_frontier.py), their names and lines from the text pack's frontier bank
// (tools/sinnoh/build_text.py frontier_bank(): the pack's last), and Platinum's rules from its battle_tower.c /
// unk_02049D08.c.
#include <stdio.h>
#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/sinnoh_gfx.h"
#include "sinnoh_tower.h"
#include "mp.h"

#include "../data/sinnoh_frontier.h"

const u8 *S3D_Text(u16 bank, u16 msg);
u16 S3D_TextBanks(void);

#define FRONTIER_TEXT_KINDS 4   // the name, then the lines before, after winning, after losing (build_text.py FRONTIER_KINDS)

// Which of Platinum's frontier trainers each battle opponent is (TRAINER_SINNOH_FRONTIER_A / _B)
static u16 sOpponent[2];

void Sinnoh_Frontier_SetOpponent(u16 battleTrainerId, u16 frontierTrainer)
{
    sOpponent[battleTrainerId == TRAINER_SINNOH_FRONTIER_B] = frontierTrainer;
}

u16 Sinnoh_Frontier_OpponentOf(u16 battleTrainerId)
{
    u16 t = sOpponent[battleTrainerId == TRAINER_SINNOH_FRONTIER_B];
    return t < SINNOH_FRONTIER_TRAINERS ? t : 0;
}

static const struct SinnohFrontierTrainer *Of(u16 battleTrainerId)
{
    return &sSinnohFrontierTrainers[Sinnoh_Frontier_OpponentOf(battleTrainerId)];
}

u8 Sinnoh_Frontier_Class(u16 battleTrainerId)
{
    return Of(battleTrainerId)->trainerClass;
}

u8 Sinnoh_Frontier_Pic(u16 battleTrainerId)
{
    return Of(battleTrainerId)->trainerPic;
}

u8 Sinnoh_Frontier_SinnohPic(u16 battleTrainerId)
{
    return Of(battleTrainerId)->sinnohPic;
}

static const u8 *Line(u16 battleTrainerId, u8 kind)
{
    u16 banks = S3D_TextBanks();
    return S3D_Text(banks ? banks - 1 : 0, Sinnoh_Frontier_OpponentOf(battleTrainerId) * FRONTIER_TEXT_KINDS + kind);
}

void Sinnoh_Frontier_Name(u8 *dst, u16 battleTrainerId)
{
    StringCopyN(dst, Line(battleTrainerId, 0), TRAINER_NAME_LENGTH);
    dst[TRAINER_NAME_LENGTH] = EOS;
}

void Sinnoh_Frontier_Speech(u16 battleTrainerId, u8 kind)
{
    StringCopy(gStringVar4, Line(battleTrainerId, 1 + kind));
}

// Platinum's IVs for a frontier trainer, by its number (BattleTower_GetIVsFromTrainerID)
static u8 IvsOf(u16 frontierTrainer)
{
    static const u16 sUpTo[] = {100, 120, 140, 160, 180, 200, 220};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sUpTo); i++)
        if (frontierTrainer < sUpTo[i])
            return MAX_PER_STAT_IVS / 8 * (i + 1);
    return MAX_PER_STAT_IVS;
}

static bool8 InList(const u16 *list, u8 count, u16 value)
{
    u32 i;
    for (i = 0; list != NULL && i < count; i++)
        if (list[i] == value && value != 0)
            return TRUE;
    return FALSE;
}

// A frontier trainer's party as Platinum makes it (BattleTower_CreateRandomTrainerParty): COUNT of its sets at random, no
// species twice (nor one of AVOIDSPECIES: the player's, a partner's), no held item twice (nor one of AVOIDITEMS) until
// 50 tries fail, then Platinum's reserved items; IVs by the trainer's number, the set's EVs (510 shared, 255 at most),
// a personality of the set's nature that isn't shiny, the ability by its first bit, friendship full (none with
// FRUSTRATION). Returns how many were made
u8 Sinnoh_Frontier_FillParty(struct Pokemon *party, u16 frontierTrainer, u8 count, const u16 *avoidSpecies,
                             const u16 *avoidItems, u8 avoidCount, u8 level)
{
    static const u16 sReservedItems[] = {ITEM_BRIGHT_POWDER, ITEM_LUM_BERRY, ITEM_LEFTOVERS, ITEM_QUICK_CLAW};
    const struct SinnohFrontierTrainer *t;
    u16 setIds[PARTY_SIZE], species[PARTY_SIZE], items[PARTY_SIZE];
    u32 otId, tries = 0, failsafe = 0, i, n = 0;
    u8 ivs;
    bool8 reserved;

    if (frontierTrainer >= SINNOH_FRONTIER_TRAINERS || count > PARTY_SIZE)
        return 0;
    t = &sSinnohFrontierTrainers[frontierTrainer];
    if (t->setCount == 0)
        return 0;
    while (n < count && tries++ < 5000)
    {
        u16 id = sSinnohFrontierSetLists[t->firstSet + Random() % t->setCount];
        const struct SinnohFrontierSet *set = &sSinnohFrontierSets[id];
        if (set->species == SPECIES_NONE || InList(species, n, set->species)
         || InList(avoidSpecies, avoidCount, set->species))
            continue;
        if (failsafe < 50 && (InList(items, n, set->item) || InList(avoidItems, avoidCount, set->item)))
        {
            failsafe++;
            continue;
        }
        setIds[n] = id;
        species[n] = set->species;
        items[n] = set->item;
        n++;
    }
    reserved = failsafe >= 50;
    ivs = IvsOf(frontierTrainer);
    otId = Random32();
    for (i = 0; i < n; i++)
    {
        const struct SinnohFrontierSet *set = &sSinnohFrontierSets[setIds[i]];
        u32 personality, k, flagged = 0, evs;
        u8 friendship = MAX_FRIENDSHIP, abilityNum;
        u16 item = reserved && i < ARRAY_COUNT(sReservedItems) ? sReservedItems[i] : set->item;
        do
            personality = Random32();
        while (GetNatureFromPersonality(personality) != set->nature || IsShinyOtIdPersonality(otId, personality));
        CreateMon(&party[i], set->species, level, ivs, TRUE, personality, OT_ID_PRESET, otId);
        for (k = 0; k < MAX_MON_MOVES; k++)
        {
            SetMonMoveSlot(&party[i], set->moves[k], k);
            if (set->moves[k] == MOVE_FRUSTRATION)
                friendship = 0;
        }
        SetMonData(&party[i], MON_DATA_HELD_ITEM, &item);
        SetMonData(&party[i], MON_DATA_FRIENDSHIP, &friendship);
        abilityNum = personality & 1;
        SetMonData(&party[i], MON_DATA_ABILITY_NUM, &abilityNum);
        for (k = 0; k < NUM_STATS; k++)
            flagged += (set->evFlags >> k) & 1;
        evs = flagged ? MAX_TOTAL_EVS / flagged : 0;
        if (evs > MAX_PER_STAT_EVS)
            evs = MAX_PER_STAT_EVS;
        {
            // (Platinum's stat order HP, ATK, DEF, SPEED, SP. ATK, SP. DEF; the game's EV fields by name)
            static const u8 sEvField[NUM_STATS] = {MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV,
                                                   MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV};
            for (k = 0; k < NUM_STATS; k++)
                if ((set->evFlags >> k) & 1)
                    SetMonData(&party[i], sEvField[k], &evs);
        }
        CalculateMonStats(&party[i]);
    }
    return n;
}

// ------------------------------------------------------------------------------------------- Platinum's BATTLE TOWER
// Its BattleTower struct (the challenge under way: mode, the POKéMON chosen, the seven opponents, the wins) and its
// save's tower records (struct TowerSave: Platinum's frontier stats and WifiBattleTowerRecord's room numbers and
// streak bits, kept in "<save>.region" chunk TOWR); the BATTLE POINTS are the game's own. Functions by Platinum's
// BT_FUNC_* numbers (its unk_020494DC.c and frscrcmd_battle_tower.c), the rules from unk_02049D08.c, battle_tower.c
// and ov104_0223A0C4.c.

#include "battle_main.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "event_data.h"
#include "load_save.h"
#include "main.h"
#include "overworld.h"
#include "party_menu.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/battle_frontier.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "constants/game_stat.h"
#include "constants/trainers_sinnoh.h"
#include "string_util.h"
#include "strings.h"
#include "data.h"

void ChoosePartyForBattleFrontier(void);

enum { BT_MODE_SINGLE, BT_MODE_DOUBLE, BT_MODE_MULTI, BT_MODE_LINK_MULTI, BT_MODE_WIFI, BT_MODE_5, BT_MODE_6,
       BT_RECORD_MODES = 4 };   // (Platinum's BATTLE_TOWER_MODE_*; WIFI and up are online: closed)
#define BT_OPPONENTS 7
#define BT_PARTNERS 5   // CHERYL, MIRA, RILEY, MARLEY, BUCK (Platinum's BT_PARTNER_*)

// the partners as the game's own trainers (their names, their pictures from behind: battle_tower.c SetQzPartner) and
// their sprites
static const u16 sPartnerTrainers[BT_PARTNERS] = {TRAINER_SINNOH_CHERYL_ETERNA_FOREST, TRAINER_SINNOH_MIRA_WAYWARD_CAVE,
                                                  TRAINER_SINNOH_RILEY_IRON_ISLAND, TRAINER_SINNOH_MARLEY_VICTORY_ROAD,
                                                  TRAINER_SINNOH_BUCK_STARK_MOUNTAIN};
static const u16 sPartnerGfx[BT_PARTNERS] = {OBJ_EVENT_GFX_SINNOH_CHERYL, OBJ_EVENT_GFX_SINNOH_MIRA,
                                             OBJ_EVENT_GFX_SINNOH_RILEY, OBJ_EVENT_GFX_SINNOH_MARLEY,
                                             OBJ_EVENT_GFX_SINNOH_BUCK};
void MP_SetupTowerPartner(u16 trainer);   // (battle_tower.c)

// Platinum's BT_FUNC_* (include/constants/battle_tower_functions.h)
enum
{
    BT_FUNC_CHECK_ENOUGH_VALID_POKEMON = 1, BT_FUNC_RESET_SYSTEM, BT_FUNC_UNK_03, BT_FUNC_UNK_04,
    BT_FUNC_SET_COMMUNICATION_CLUB_ACCESSIBLE, BT_FUNC_CLEAR_COMMUNICATION_CLUB_ACCESSIBLE,
    BT_FUNC_UNK_08 = 8, BT_FUNC_UNK_09, BT_FUNC_UNK_10, BT_FUNC_UNK_11, BT_FUNC_UNK_12, BT_FUNC_UNK_14 = 14, BT_FUNC_UNK_15,
    BT_FUNC_UNK_16, BT_FUNC_UNK_30 = 30, BT_FUNC_UNK_31, BT_FUNC_CHECK_DUPLICATE_SPECIES_AND_HELD_ITEMS,
    BT_FUNC_GET_NEXT_OPPONENT_NUM, BT_FUNC_UNK_34, BT_FUNC_HAS_DEFEATED_SEVEN_TRAINERS, BT_FUNC_UNK_36,
    BT_FUNC_UPDATE_GAME_RECORDS, BT_FUNC_UPDATE_GAME_RECORDS_AND_JOURNAL, BT_FUNC_UNK_39, BT_FUNC_SET_OPPONENT_TEAMS,
    BT_FUNC_GET_OPPONENT_OBJECT_ID, BT_FUNC_UNK_42, BT_FUNC_GET_CHALLENGE_MODE, BT_FUNC_SET_BEAT_PALMER,
    BT_FUNC_GET_BEAT_PALMER, BT_FUNC_UNK_46, BT_FUNC_UNK_47, BT_FUNC_UNK_48, BT_FUNC_UNK_49, BT_FUNC_SET_PARTNER_ID,
    BT_FUNC_GET_PARTNER_ID, BT_FUNC_UNK_52, BT_FUNC_GET_SLOT_INDEX, BT_FUNC_UNK_54, BT_FUNC_GET_PARTNER_PARAM,
    BT_FUNC_UNK_56, BT_FUNC_UNK_57, BT_FUNC_UNK_58, BT_FUNC_UNK_59, BT_FUNC_CHECK_IS_NULL = 100,
};

static struct TowerSave
{
    u16 record[BT_RECORD_MODES];    // the best streak (Platinum's frontier stat mode * 2)
    u16 latest[BT_RECORD_MODES];    // the last challenge's streak (mode * 2 + 1)
    u16 room[BT_RECORD_MODES];      // the rooms of seven cleared in a row (WifiBattleTowerRecord roomNums)
    u8 streakOn[BT_RECORD_MODES];   // the last challenge was won and goes on (its flag bit 8 + mode)
    u8 gotRibbons[2];               // the 50 and 100 streaks' gifts given (its flag bits 0, 1)
    // the challenge REST saved (WifiBattleTowerSave: sub_0204A8C8)
    u8 inProgress, mode, nextOpponentNum, partnerID;
    u8 slots[4];
    u16 trainerIDs[BT_OPPONENTS * 2];
    struct Pokemon partnerMons[2];  // the BATTLE SALON partner's team (Platinum keeps its sets and personalities)
    // game records
    u32 victories, challenges, pointsReceived;
} sTS;

static struct
{
    bool8 active;                   // InitBattleTower made it (Platinum's fieldSystem->battleTower != NULL)
    u8 challengeMode, partySize, nextOpponentNum, wins;
    u8 defeatedSeven, beatPalmer, partnerID, ribbons;
    u8 slots[4];
    u16 species[4], items[4];
    u16 trainerIDs[BT_OPPONENTS * 2];
    u16 streakBefore, roomNum;
    bool8 wonBattle, choosing;
    struct Pokemon opponents[2][4];
    u8 opponentCount[2];
    struct Pokemon partners[BT_PARTNERS][2];   // the BATTLE SALON's partners' teams (BT_FUNC_UNK_56)
    u8 partnerCount[BT_PARTNERS];
    struct Pokemon saved[PARTY_SIZE];
} sT;

void Sinnoh_TowerReset(void)
{
    memset(&sTS, 0, sizeof(sTS));
    sT.active = FALSE;
}

const void *Sinnoh_TowerData(u32 *size)
{
    *size = sizeof(sTS);
    return &sTS;
}

void Sinnoh_TowerLoad(const void *data, u32 size)
{
    memset(&sTS, 0, sizeof(sTS));
    memcpy(&sTS, data, size < sizeof(sTS) ? size : sizeof(sTS));
}

static const u16 sBanned[] = {SPECIES_MEWTWO, SPECIES_MEW, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI, SPECIES_KYOGRE,
                              SPECIES_GROUDON, SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS, SPECIES_DIALGA,
                              SPECIES_PALKIA, SPECIES_GIRATINA, SPECIES_PHIONE, SPECIES_MANAPHY, SPECIES_DARKRAI,
                              SPECIES_SHAYMIN, SPECIES_ARCEUS};   // (Platinum's sBattleFrontierBanlist)

bool8 Sinnoh_Tower_Banned(u16 species)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sBanned); i++)
        if (sBanned[i] == species)
            return TRUE;
    return FALSE;
}

bool8 Sinnoh_Tower_Choosing(void)
{
    return sT.choosing;
}

static u8 PartySize(u8 mode)
{
    switch (mode)
    {
    case BT_MODE_DOUBLE: return 4;
    case BT_MODE_MULTI: case BT_MODE_LINK_MULTI: case BT_MODE_6: return 2;
    default: return 3;
    }
}

// Platinum's sub_02049EC4: COUNT POKéMON of different kinds holding different items (none counts as different) that
// may enter
static bool8 EnoughValid(u8 count)
{
    u16 species[PARTY_SIZE], items[PARTY_SIZE];
    u32 i, j, n = 0, a, b, c, d;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 s = GetMonData(mon, MON_DATA_SPECIES), it = GetMonData(mon, MON_DATA_HELD_ITEM);
        if (s == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || Sinnoh_Tower_Banned(s))
            continue;
        for (j = 0; j < n; j++)
            if (species[j] == s && (it == 0 || items[j] == it))
                break;
        if (j < n)
            continue;
        species[n] = s, items[n] = it, n++;
    }
    if (n < count)
        return FALSE;
#define DIFF(x, y) (species[x] != species[y] && (items[x] == 0 || items[x] != items[y]))
    for (a = 0; a < n; a++)
        for (b = a + 1; b < n; b++)
        {
            if (!DIFF(a, b))
                continue;
            if (count == 2)
                return TRUE;
            for (c = b + 1; c < n; c++)
            {
                if (!DIFF(a, c) || !DIFF(b, c))
                    continue;
                if (count == 3)
                    return TRUE;
                for (d = c + 1; d < n; d++)
                    if (DIFF(a, d) && DIFF(b, d) && DIFF(c, d))
                        return TRUE;
            }
        }
#undef DIFF
    return count <= 1 && n >= count;
}

static void Init(u16 mode, bool8 resume)
{
    u32 i;
    memset(&sT, 0, sizeof(sT) - sizeof(sT.saved));
    sT.active = TRUE;
    if (!resume)
    {
        sT.challengeMode = mode;
        sT.nextOpponentNum = 1;
        memset(sT.slots, 0xFF, sizeof(sT.slots));
        for (i = 0; i < ARRAY_COUNT(sT.trainerIDs); i++)
            sT.trainerIDs[i] = 0xFFFF;
        sTS.inProgress = FALSE;
    }
    else
    {
        sT.challengeMode = sTS.mode;
        sT.nextOpponentNum = sTS.nextOpponentNum;
        sT.wins = sT.nextOpponentNum - 1;
        memcpy(sT.slots, sTS.slots, sizeof(sT.slots));
        memcpy(sT.trainerIDs, sTS.trainerIDs, sizeof(sT.trainerIDs));
        sT.partnerID = sTS.partnerID % BT_PARTNERS;
        sT.partners[sT.partnerID][0] = sTS.partnerMons[0];
        sT.partners[sT.partnerID][1] = sTS.partnerMons[1];
        sT.partnerCount[sT.partnerID] = 2;
        sTS.inProgress = FALSE;
    }
    sT.partySize = PartySize(sT.challengeMode);
    for (i = 0; i < sT.partySize && i < 4; i++)
        if (sT.slots[i] < PARTY_SIZE)
        {
            sT.species[i] = GetMonData(&gPlayerParty[sT.slots[i]], MON_DATA_SPECIES);
            sT.items[i] = GetMonData(&gPlayerParty[sT.slots[i]], MON_DATA_HELD_ITEM);
        }
    if (sT.challengeMode < BT_RECORD_MODES && sTS.streakOn[sT.challengeMode])
    {
        sT.streakBefore = sTS.latest[sT.challengeMode];
        sT.roomNum = sTS.room[sT.challengeMode];
    }
    MPNet_Log("tower: %s mode %d (streak %d, room %d)", resume ? "resumed" : "started", sT.challengeMode,
              sT.streakBefore, sT.roomNum);
}

// Platinum's BattleTower_GetTrainerIDForRoomAndOpponentNum
static u16 TrainerFor(u8 roomNum, u8 opponentNum)
{
    const u16 *range;
    if (sT.challengeMode == BT_MODE_SINGLE && opponentNum == BT_OPPONENTS - 1)
    {
        if (roomNum == 2)
            return SINNOH_FT_TOWER_TYCOON_PALMER_SILVER;   // (the 21st battle)
        if (roomNum == 6)
            return SINNOH_FT_TOWER_TYCOON_PALMER_GOLD;     // (the 49th)
    }
    if (roomNum < 7)
        range = opponentNum == BT_OPPONENTS - 1 ? sSinnohTowerBosses[roomNum] : sSinnohTowerRooms[roomNum];
    else
        range = sSinnohTowerRooms[7];
    return range[0] + Random() % (range[1] - range[0] + 1);
}

// Platinum's sub_0204A4C8: the seven opponents (two a battle in multi battles), none twice
static void ChooseOpponents(void)
{
    bool8 multi = sT.challengeMode == BT_MODE_MULTI || sT.challengeMode == BT_MODE_LINK_MULTI;
    u32 n = multi ? BT_OPPONENTS * 2 : BT_OPPONENTS, i, j;
    for (i = 0; i < n; i++)
    {
        u16 t;
        do
        {
            t = TrainerFor(sT.roomNum, multi ? i / 2 : i);
            for (j = 0; j < i && sT.trainerIDs[j] != t; j++)
                ;
        } while (j < i);
        sT.trainerIDs[i] = t;
    }
}

// Platinum's BattleTower_CreateOpponentParties
static void MakeOpponents(void)
{
    u8 n = sT.nextOpponentNum - 1;
    memset(sT.opponents, 0, sizeof(sT.opponents));
    if (sT.challengeMode == BT_MODE_MULTI || sT.challengeMode == BT_MODE_LINK_MULTI)
    {
        u16 species[4], items[4];
        u32 i;
        sT.opponentCount[0] = Sinnoh_Frontier_FillParty(sT.opponents[0], sT.trainerIDs[n * 2], sT.partySize, NULL, NULL,
                                                        0, 50);
        for (i = 0; i < sT.opponentCount[0]; i++)
        {
            species[i] = GetMonData(&sT.opponents[0][i], MON_DATA_SPECIES);
            items[i] = GetMonData(&sT.opponents[0][i], MON_DATA_HELD_ITEM);
        }
        sT.opponentCount[1] = Sinnoh_Frontier_FillParty(sT.opponents[1], sT.trainerIDs[n * 2 + 1], sT.partySize,
                                                        species, items, sT.opponentCount[0], 50);
        Sinnoh_Frontier_SetOpponent(TRAINER_SINNOH_FRONTIER_A, sT.trainerIDs[n * 2]);
        Sinnoh_Frontier_SetOpponent(TRAINER_SINNOH_FRONTIER_B, sT.trainerIDs[n * 2 + 1]);
    }
    else
    {
        sT.opponentCount[0] = Sinnoh_Frontier_FillParty(sT.opponents[0], sT.trainerIDs[n], sT.partySize, NULL, NULL, 0,
                                                        50);
        sT.opponentCount[1] = 0;
        Sinnoh_Frontier_SetOpponent(TRAINER_SINNOH_FRONTIER_A, sT.trainerIDs[n]);
    }
    MPNet_Log("tower: battle %d against trainer %d", sT.nextOpponentNum, sT.trainerIDs[n]);
}

// The challenge's RNG seed (BT_FUNC_UNK_57) is the game's own: Random()

static u16 Streak(void)
{
    u32 s = (u32)sT.streakBefore + sT.wins;
    return s > 0xFFFF ? 0xFFFF : s;
}

// Platinum's sub_0204ACC8: 50 in a row outside single battles earns the challenge's POKéMON a ribbon
static void CheckRibbonStreak(void)
{
    if (sT.challengeMode != BT_MODE_SINGLE && sT.challengeMode < BT_MODE_WIFI && Streak() >= 50)
        sT.ribbons = TRUE;
}

static void EndRecords(bool8 wonRoom)
{
    u8 m = sT.challengeMode;
    if (m >= BT_RECORD_MODES)
        return;
    sTS.latest[m] = Streak();
    if (sTS.latest[m] > sTS.record[m])
        sTS.record[m] = sTS.latest[m];
    sTS.streakOn[m] = wonRoom;
    sTS.victories += wonRoom ? BT_OPPONENTS : sT.wins;
    if (wonRoom)
    {
        if (sTS.room[m] < 65534)
            sTS.room[m]++;
    }
    else
    {
        sTS.room[m] = 0;
    }
    sTS.challenges++;
    CheckRibbonStreak();
    MPNet_Log("tower: challenge %s, streak %d (best %d), room %d", wonRoom ? "won" : "over", sTS.latest[m],
              sTS.record[m], sTS.room[m]);
}

// Platinum's BattleTower_GiveBattlePointsReward
static u16 GiveBattlePoints(void)
{
    static const u8 sSolo[] = {0, 3, 3, 4, 4, 5, 5, 7};
    static const u8 sLinkMulti[] = {0, 8, 9, 11, 12, 14, 15, 18};
    u16 room = sT.challengeMode < BT_RECORD_MODES ? sTS.room[sT.challengeMode] : 0, bp;
    if (sT.challengeMode == BT_MODE_LINK_MULTI)
        bp = room >= 7 ? 18 : sLinkMulti[room];
    else if (sT.beatPalmer)
        bp = 20;
    else
        bp = room >= 7 ? 7 : sSolo[room];
    gSaveBlock2Ptr->frontier.battlePoints = min(gSaveBlock2Ptr->frontier.battlePoints + bp, MAX_BATTLE_FRONTIER_POINTS);
    sTS.pointsReceived += bp;
    MPNet_Log("tower: %d BP", bp);
    return bp;
}

// Platinum's sub_0204AC54: Platinum's ribbon RIBBON for the challenge's POKéMON (TRUE: one got it; s3d_party.c keeps
// the Gen 4 ones)
bool8 Sinnoh_MonHasRibbon(struct Pokemon *mon, u8 ribbon);
void Sinnoh_MonGiveRibbon(struct Pokemon *mon, u8 ribbon);
enum { RIBBON_ABILITY = 53, RIBBON_GREAT_ABILITY, RIBBON_DOUBLE_ABILITY, RIBBON_MULTI_ABILITY };   // (Platinum's)

static bool8 GiveRibbon(u8 ribbon)
{
    u8 n = 0;
    u32 i;
    for (i = 0; i < sT.partySize && i < 4; i++)
    {
        struct Pokemon *mon;
        if (sT.slots[i] >= PARTY_SIZE)
            continue;
        mon = &gPlayerParty[sT.slots[i]];
        if (Sinnoh_MonHasRibbon(mon, ribbon))
            continue;
        Sinnoh_MonGiveRibbon(mon, ribbon);
        n++;
    }
    return n != 0;
}

static void CB2_EndTowerBattle(void)
{
    sT.wonBattle = gBattleOutcome == B_OUTCOME_WON;
    memcpy(gPlayerParty, sT.saved, sizeof(gPlayerParty));   // (the battle had copies at level 50: the party as it was)
    CalculatePlayerPartyCount();
    VarSet(VAR_FRONTIER_FACILITY, 0);
    MP_MUSIC_SCENE_RELEASE();
    MPNet_Log("tower: battle %d %s", sT.nextOpponentNum, sT.wonBattle ? "won" : "lost");
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_StartTowerBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndTowerBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// Platinum's FieldBattleDTO_NewBattleTower: the chosen POKéMON (above level 50 brought down to it), the opponents
static void StartBattle(void)
{
    u32 i;
    memcpy(sT.saved, gPlayerParty, sizeof(gPlayerParty));
    ZeroPlayerPartyMons();
    for (i = 0; i < sT.partySize && i < 4; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (sT.slots[i] >= PARTY_SIZE)
            continue;
        *mon = sT.saved[sT.slots[i]];
        if (GetMonData(mon, MON_DATA_LEVEL) > 50)
        {
            u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][50];
            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
        }
    }
    CalculatePlayerPartyCount();
    ZeroEnemyPartyMons();
    for (i = 0; i < sT.opponentCount[0]; i++)
        gEnemyParty[i] = sT.opponents[0][i];
    for (i = 0; i < sT.opponentCount[1]; i++)
        gEnemyParty[3 + i] = sT.opponents[1][i];
    VarSet(VAR_FRONTIER_FACILITY, FRONTIER_FACILITY_TOWER);
    gTrainerBattleOpponent_A = TRAINER_SINNOH_FRONTIER_A;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER;
    if (sT.challengeMode == BT_MODE_DOUBLE)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
    if (sT.opponentCount[1])
    {
        gTrainerBattleOpponent_B = TRAINER_SINNOH_FRONTIER_B;
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS;
    }
    if (sT.challengeMode == BT_MODE_MULTI)
    {
        // beside the BATTLE SALON partner: its team in the party's second half (Platinum's BATTLE_TYPE_FRONTIER_WITH_AI_PARTNER)
        u8 p = sT.partnerID % BT_PARTNERS;
        for (i = 0; i < sT.partnerCount[p] && i < MULTI_PARTY_SIZE; i++)
            gPlayerParty[MULTI_PARTY_SIZE + i] = sT.partners[p][i];
        MP_SetupTowerPartner(sPartnerTrainers[p]);
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER;
    }
    CreateTask(Task_StartTowerBattle, 1);
    MP_MUSIC_SCENE_HOLD(MUS_VS_TRAINER, MUS_VICTORY_TRAINER);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_B_TOWER));
}

static u16 Choose(void)
{
    // Platinum's sub_0204A358: the party menu for PARTYSIZE POKéMON of any level
    sT.choosing = TRUE;
    gSpecialVar_0x8004 = FRONTIER_LVL_OPEN;
    gSpecialVar_0x8005 = sT.partySize;
    ChoosePartyForBattleFrontier();   // (its end starts the script again; BT_FUNC_UNK_31 reads the choice)
    return 0;
}

// Platinum's sub_0204A378: the POKéMON chosen (FALSE: none)
static bool8 Chosen(void)
{
    u32 i;
    sT.choosing = FALSE;
    if (gSelectedOrderFromParty[0] == 0)
        return FALSE;
    for (i = 0; i < sT.partySize && i < 4; i++)
    {
        sT.slots[i] = gSelectedOrderFromParty[i] - 1;
        sT.species[i] = GetMonData(&gPlayerParty[sT.slots[i]], MON_DATA_SPECIES);
        sT.items[i] = GetMonData(&gPlayerParty[sT.slots[i]], MON_DATA_HELD_ITEM);
    }
    return TRUE;
}

// Platinum's BattleTower_CheckDuplicateSpeciesAndHeldItems: 1 the same kind twice, 2 the same item
static u16 Duplicates(void)
{
    u32 i, j;
    for (i = 1; i < sT.partySize && i < 4; i++)
        for (j = 0; j < i; j++)
        {
            if (sT.species[i] == sT.species[j])
                return 1;
            if (sT.items[i] != ITEM_NONE && sT.items[i] == sT.items[j])
                return 2;
        }
    return 0;
}

bool8 Sinnoh_TowerCall(u16 func, u16 arg, u16 destVar)
{
    u16 value = VarGet(arg), out = 0;
    bool8 set = TRUE;
    switch (func)
    {
    case SINNOH_TOWER_INIT:
        Init(value, FALSE);
        return FALSE;
    case SINNOH_TOWER_INIT_RESUME:
        Init(0, TRUE);
        return FALSE;
    case SINNOH_TOWER_FREE:
        sT.active = FALSE;
        return FALSE;
    case SINNOH_TOWER_INTRO:
        Sinnoh_Frontier_Speech(value ? TRAINER_SINNOH_FRONTIER_B : TRAINER_SINNOH_FRONTIER_A, SINNOH_FRONTIER_SPEECH_INTRO);
        return FALSE;
    case SINNOH_TOWER_BATTLE:
    case BT_FUNC_UNK_42:
        StartBattle();
        return TRUE;
    case SINNOH_TOWER_WON:
        out = sT.wonBattle;
        break;
    case SINNOH_TOWER_SAVE:
        TrySavingData(SAVE_NORMAL);
        return FALSE;
    case SINNOH_TOWER_CHOOSE:
    case BT_FUNC_UNK_30:
        Choose();
        return TRUE;
    case BT_FUNC_CHECK_ENOUGH_VALID_POKEMON:
        out = EnoughValid(value ? value : sT.partySize);
        break;
    case BT_FUNC_RESET_SYSTEM:
    {
        // (Platinum's reset after REST: the title screen, the save read again; the PC build's SoftReset quits)
        void MP_ReturnToTitleNow(void);
        MP_ReturnToTitleNow();
        return TRUE;
    }
    case BT_FUNC_UNK_03:   // (the saved challenge cleared: WifiBattleTowerSave_Init)
        sTS.inProgress = FALSE;
        set = FALSE;
        break;
    case BT_FUNC_UNK_04:   // a challenge REST saved
        out = sTS.inProgress;
        break;
    case BT_FUNC_SET_COMMUNICATION_CLUB_ACCESSIBLE:
        // (Platinum's special location: a game continued from here starts where the player stands)
        SetDynamicWarpWithCoords(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE,
                                 gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        SetContinueGameWarpStatusToDynamicWarp();
        set = FALSE;
        break;
    case BT_FUNC_CLEAR_COMMUNICATION_CLUB_ACCESSIBLE:
        ClearContinueGameWarpStatus();
        set = FALSE;
        break;
    case BT_FUNC_UNK_14:   // (a REST quit without saving: the streak is lost; Platinum's sub_0204A064)
        if (sTS.mode < BT_RECORD_MODES)
        {
            sTS.streakOn[sTS.mode] = FALSE;
            sTS.room[sTS.mode] = 0;
            sTS.latest[sTS.mode] = 0;
        }
        out = sTS.mode;
        break;
    case BT_FUNC_UNK_08:
        out = value < BT_RECORD_MODES ? sTS.latest[value] : 0;
        break;
    case BT_FUNC_UNK_31:
        out = Chosen();
        break;
    case BT_FUNC_CHECK_DUPLICATE_SPECIES_AND_HELD_ITEMS:
        out = Duplicates();
        break;
    case BT_FUNC_GET_NEXT_OPPONENT_NUM:
        out = sT.nextOpponentNum;
        break;
    case BT_FUNC_UNK_34:
        sT.wins++;
        out = ++sT.nextOpponentNum;
        break;
    case BT_FUNC_HAS_DEFEATED_SEVEN_TRAINERS:
        if (sT.nextOpponentNum > BT_OPPONENTS)
            sT.defeatedSeven = TRUE;
        out = sT.defeatedSeven;
        break;
    case BT_FUNC_UNK_36:
        out = Streak();
        break;
    case BT_FUNC_UPDATE_GAME_RECORDS:
        EndRecords(FALSE);
        set = FALSE;
        break;
    case BT_FUNC_UPDATE_GAME_RECORDS_AND_JOURNAL:
        EndRecords(TRUE);
        set = FALSE;
        break;
    case BT_FUNC_UNK_39:
        // REST: the challenge kept (sub_0204A8C8)
        sTS.inProgress = TRUE;
        sTS.mode = sT.challengeMode;
        sTS.nextOpponentNum = sT.nextOpponentNum;
        sTS.partnerID = sT.partnerID;
        memcpy(sTS.slots, sT.slots, sizeof(sTS.slots));
        memcpy(sTS.trainerIDs, sT.trainerIDs, sizeof(sTS.trainerIDs));
        sTS.partnerMons[0] = sT.partners[sT.partnerID % BT_PARTNERS][0];
        sTS.partnerMons[1] = sT.partners[sT.partnerID % BT_PARTNERS][1];
        set = FALSE;
        break;
    case BT_FUNC_SET_OPPONENT_TEAMS:
        MakeOpponents();
        set = FALSE;
        break;
    case BT_FUNC_GET_OPPONENT_OBJECT_ID:
        out = sSinnohFrontierTrainers[Sinnoh_Frontier_OpponentOf(value ? TRAINER_SINNOH_FRONTIER_B
                                                                       : TRAINER_SINNOH_FRONTIER_A)].objGfx;
        break;
    case BT_FUNC_GET_CHALLENGE_MODE:
        out = sT.challengeMode;
        break;
    case BT_FUNC_SET_BEAT_PALMER:
        sT.beatPalmer = value;
        set = FALSE;
        break;
    case BT_FUNC_GET_BEAT_PALMER:
        out = sT.beatPalmer;
        break;
    case BT_FUNC_UNK_46:
        out = GiveBattlePoints();
        break;
    case BT_FUNC_UNK_47:   // (the 50 / 100 streak gifts given: sub_0204AA7C)
    {
        u16 s = sT.challengeMode < BT_RECORD_MODES ? sTS.latest[sT.challengeMode] : 0;
        out = s >= 50 && !sTS.gotRibbons[s >= 100];
        break;
    }
    case BT_FUNC_UNK_48:   // PALMER beaten: the ribbon (sub_0204AB68)
        out = sT.challengeMode == BT_MODE_SINGLE && sT.beatPalmer
            ? GiveRibbon(sT.beatPalmer == 2 ? RIBBON_GREAT_ABILITY : RIBBON_ABILITY) : 0;
        break;
    case BT_FUNC_UNK_49:   // 50 in a row outside single battles: the ribbon (sub_0204ABA0)
        out = sT.ribbons && (sT.challengeMode == BT_MODE_DOUBLE || sT.challengeMode == BT_MODE_MULTI)
            ? GiveRibbon(sT.challengeMode == BT_MODE_DOUBLE ? RIBBON_DOUBLE_ABILITY : RIBBON_MULTI_ABILITY) : 0;
        break;
    case BT_FUNC_SET_PARTNER_ID:
        sT.partnerID = value % BT_PARTNERS;
        set = FALSE;
        break;
    case BT_FUNC_GET_PARTNER_ID:
        out = sT.partnerID;
        break;
    case BT_FUNC_UNK_52:
        ChooseOpponents();
        set = FALSE;
        break;
    case BT_FUNC_GET_SLOT_INDEX:
        out = value < 4 ? sT.slots[value] : 0xFF;
        break;
    case BT_FUNC_GET_PARTNER_PARAM:
        // (2 the partner's number, 1 its sprite: the player's own is the player walking in)
        out = value == 2 ? sT.partnerID : value == 1 ? sPartnerGfx[sT.partnerID % BT_PARTNERS] : 0;
        break;
    case BT_FUNC_UNK_57:
        out = Random() / 65535;
        break;
    case BT_FUNC_CHECK_IS_NULL:
        out = !sT.active;
        break;
    case BT_FUNC_UNK_56:
    {
        // the BATTLE SALON's partners' teams (sub_0204A97C): two each, none of the player's kinds or items
        u32 p;
        for (p = 0; p < BT_PARTNERS; p++)
            sT.partnerCount[p] = Sinnoh_Frontier_FillParty(sT.partners[p], SINNOH_FT_TRAINER_CHERYL_CHERYL + p,
                                                           sT.partySize, sT.species, sT.items, sT.partySize, 50);
        MPNet_Log("tower: the BATTLE SALON's partners' teams made");
        set = FALSE;
        break;
    }
    case SINNOH_TOWER_PARTNER_SPECIES:
    case SINNOH_TOWER_PARTNER_SPECIES + 1:
    case SINNOH_TOWER_PARTNER_MOVE:
    case SINNOH_TOWER_PARTNER_MOVE + 1:
    {
        // (Platinum's GetBattleTowerPartnerSpeciesAndMove: partner VALUE's POKéMON n, its kind or its first move)
        struct Pokemon *mon = &sT.partners[value % BT_PARTNERS][func & 1];
        out = func < SINNOH_TOWER_PARTNER_MOVE ? GetMonData(mon, MON_DATA_SPECIES) : GetMonData(mon, MON_DATA_MOVE1);
        break;
    }
    case SINNOH_TOWER_BUFFER_SPECIES:
    case SINNOH_TOWER_BUFFER_MOVE:
    {
        // (a name into text slot DESTVAR: 0-2 the game's STR_VAR_*, 3-5 SINNOH's own)
        u8 *StringVar(u16 n);
        if (func == SINNOH_TOWER_BUFFER_SPECIES)
            GetSpeciesName(StringVar(destVar), value);
        else
            StringCopy(StringVar(destVar), gMoveNames[value]);
        return FALSE;
    }
    case SINNOH_TOWER_RANKINGS:
    {
        void Sinnoh_RankingsStart(u16 machine);
        Sinnoh_RankingsStart(value);
        return TRUE;
    }
    case SINNOH_TOWER_JUDGE_TOTAL:
    case SINNOH_TOWER_JUDGE_STAT:
    case SINNOH_TOWER_JUDGE_BEST:
    {
        // Platinum's ScrCmd_JudgeStats: in its stat order HP, ATTACK, DEFENSE, SP. ATK, SP. DEF, SPEED, the best IV
        // (one of those tied, at random: picked with the total, kept for the next two)
        static const u8 sIvField[NUM_STATS] = {MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV, MON_DATA_SPATK_IV,
                                               MON_DATA_SPDEF_IV, MON_DATA_SPEED_IV};
        static u8 sBest;
        struct Pokemon *mon = &gPlayerParty[value < PARTY_SIZE ? value : 0];
        u8 ivs[NUM_STATS], top = 0, ties = 0;
        u32 k, total = 0;
        for (k = 0; k < NUM_STATS; k++)
        {
            ivs[k] = GetMonData(mon, sIvField[k]);
            total += ivs[k];
            if (ivs[k] > top)
                top = ivs[k];
        }
        if (func == SINNOH_TOWER_JUDGE_TOTAL)
        {
            for (k = 0; k < NUM_STATS; k++)
                ties += ivs[k] == top;
            ties = Random() % ties;
            for (k = 0; k < NUM_STATS; k++)
                if (ivs[k] == top && ties-- == 0)
                    break;
            sBest = k < NUM_STATS ? k : 0;
            out = total;
            MPNet_Log("tower: the STATS JUDGE: slot %d, total %u, best stat %d (%d)", value, total, sBest, top);
        }
        else
            out = func == SINNOH_TOWER_JUDGE_STAT ? 122 + sBest : ivs[sBest];
        break;
    }
    case BT_FUNC_UNK_09: case BT_FUNC_UNK_10: case BT_FUNC_UNK_11: case BT_FUNC_UNK_12: case BT_FUNC_UNK_15:
    case BT_FUNC_UNK_16: case BT_FUNC_UNK_54: case BT_FUNC_UNK_58: case BT_FUNC_UNK_59:
    default:
        break;   // (the Wi-Fi rooms' and the link's: closed; the partners' parties: the BATTLE SALON's own call)
    }
    if (set && destVar >= VARS_START)
        VarSet(destVar, out);
    return FALSE;
}

// (s3d_rankings.c: JUBILIFE TV's rankings machines) the best streak in MODE, the victories, the challenges
u16 Sinnoh_Tower_RecordStreak(u8 mode)
{
    return mode < BT_RECORD_MODES ? sTS.record[mode] : 0;
}

u32 Sinnoh_Tower_Victories(void)
{
    return sTS.victories;
}

u32 Sinnoh_Tower_Challenges(void)
{
    return sTS.challenges;
}

// (tests: `mp stower [MODE ROOM]`: the records, the challenge; MODE's streak set going at ROOM rooms of seven)
void Sinnoh_TowerDebug(const char *arg)
{
    int mode, room;
    if (sscanf(arg, "%d %d", &mode, &room) == 2 && mode >= 0 && mode < BT_RECORD_MODES)
    {
        sTS.room[mode] = room;
        sTS.latest[mode] = room * BT_OPPONENTS;
        sTS.streakOn[mode] = room > 0;
    }
    printf("[test] stower: active %d mode %d next %d wins %d palmer %d | single record %d latest %d room %d on %d"
           " | double record %d latest %d room %d | multi record %d latest %d room %d partner %d"
           " | bp %d victories %u challenges %u inprogress %d\n",
           sT.active, sT.challengeMode, sT.nextOpponentNum, sT.wins, sT.beatPalmer, sTS.record[0], sTS.latest[0],
           sTS.room[0], sTS.streakOn[0], sTS.record[1], sTS.latest[1], sTS.room[1], sTS.record[2], sTS.latest[2],
           sTS.room[2], sT.partnerID, gSaveBlock2Ptr->frontier.battlePoints,
           sTS.victories, sTS.challenges, sTS.inProgress);
}
