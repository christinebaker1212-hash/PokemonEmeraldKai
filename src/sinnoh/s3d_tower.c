// SINNOH's BATTLE TOWER as Platinum's (docs/SINNOH_TOWER_PORT.md): Platinum's frontier trainers and sets
// (src/data/sinnoh_frontier.h, tools/sinnoh/gen_frontier.py), their names and lines from the text pack's frontier bank
// (tools/sinnoh/build_text.py frontier_bank(): the pack's last), and Platinum's rules from its battle_tower.c /
// unk_02049D08.c.
#include "global.h"
#include "battle.h"
#include "pokemon.h"
#include "random.h"
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
