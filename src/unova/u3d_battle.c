// Moemon Kai: UNOVA's battles (docs/UNOVA_HANDOFF.md). Black's own trainer tables (its trainers, a/0/9/2, and their
// teams, a/0/9/3, in UNOVA's scripts pack: tools/unova/build_scripts.py; nothing of them is in this repository), read
// at run time and fought as Kai's own trainer battles through TRAINER_UNOVA_A / _B, as ORRE fights Colosseum's (its
// own code, never orre_battle.c's): a battle fills the slots in a writable copy of the game's trainer table with the
// trainer's name and class (Black's words: banks 190 and 191), its team (Black's species, levels, strengths as IVs,
// held items and moves where Black gives them; GENERATION and level scaling as any trainer's), Black's battle kind
// (Doubles; Triples and Rotations as Triples: the user's decision, when the game can field them).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "pokemon.h"
#include "string_util.h"
#include "u3d.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "constants/characters.h"
#include "constants/songs.h"

extern const struct Trainer gTrainersBase[];
extern const u32 gTrainersBaseCount;

void MPNet_Log(const char *fmt, ...);
const u8 *U3D_Vm_Trainer(u16 id, u32 *size);
const u8 *U3D_Vm_TrainerTeam(u16 id, u32 *size);

#define BANK_TRAINER_NAMES 190
#define BANK_TRAINER_CLASSES 191

static struct Trainer *sTable;
static struct Pokemon sTeam[2][PARTY_SIZE];
static u8 sTeamCount[2];
static u8 sClassName[2][24];
static u16 sBlackTrainer[2];
static u8 sResult;

static struct Trainer *Slot(u16 trainerId)
{
    if (gTrainersTable != sTable)   // (the base table, or another region's / the Maker's copy: copied as it is)
    {
        if (sTable == NULL && (sTable = malloc(sizeof(struct Trainer) * gTrainersBaseCount)) == NULL)
            return NULL;
        memcpy(sTable, gTrainersTable, sizeof(struct Trainer) * gTrainersBaseCount);
        gTrainersTable = sTable;
    }
    return &sTable[trainerId];
}

static u16 Le16(const u8 *p) { return p[0] | p[1] << 8; }

static void TextInto(u8 *out, u16 bank, u16 msg, u32 n)
{
    u8 buf[96];
    StringExpandPlaceholders(buf, U3D_Text(bank, msg));
    StringCopyN(out, buf, n);
    out[n] = EOS;
}

// Black's team of trainer ID into sTeam[WHICH]
static u8 BuildTeam(u8 which, u16 id, const u8 *t)
{
    u32 size, i, stride;
    const u8 *p = U3D_Vm_TrainerTeam(id, &size);
    u8 flags = t[0], count = t[3], n = 0;
    stride = 8 + ((flags & 2) ? 2 : 0) + ((flags & 1) ? 8 : 0);
    for (i = 0; p && i < count && i < PARTY_SIZE && (i + 1) * stride <= size; i++)
    {
        const u8 *r = p + i * stride;
        u16 national = Le16(r + 4), species = national ? NationalPokedexNumToSpecies(national) : SPECIES_NONE;
        u8 level = r[2] ? r[2] : 1, iv = r[0] * MAX_PER_STAT_IVS / 255;
        struct Pokemon *mon = &sTeam[which][n];
        if (species == SPECIES_NONE)
            continue;
        CreateMon(mon, species, level, iv, FALSE, 0, OT_ID_RANDOM_NO_SHINY, 0);
        if (flags & 1)
        {
            const u8 *m = r + 8 + ((flags & 2) ? 2 : 0);
            u32 k;
            for (k = 0; k < MAX_MON_MOVES; k++)
            {
                u16 move = Le16(m + 2 * k);
                SetMonMoveSlot(mon, move, k);   // (Black's move numbers are the national ones, as the game's)
            }
        }
        n++;
    }
    return n;
}

static bool8 SetTrainer(u8 which, u16 id, bool8 doubles)
{
    u32 size;
    const u8 *t = U3D_Vm_Trainer(id, &size);
    u16 slot = which ? TRAINER_UNOVA_B : TRAINER_UNOVA_A;
    struct Trainer *s = Slot(slot);
    if (t == NULL || size < 20 || s == NULL)
        return FALSE;
    *s = gTrainersBase[slot];
    TextInto(s->trainerName, BANK_TRAINER_NAMES, id, TRAINER_NAME_LENGTH);
    s->doubleBattle = doubles;
    s->aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY;
    sTeamCount[which] = BuildTeam(which, id, t);
    s->partySize = sTeamCount[which];
    sBlackTrainer[which] = id;
    TextInto(sClassName[which], BANK_TRAINER_CLASSES, t[1], 20);
    MPNet_Log("unova: trainer %s %d (Black's %d, class %d): %d POKeMON, kind %d", which ? "B" : "A", slot, id, t[1],
              sTeamCount[which], t[2]);
    return sTeamCount[which] > 0;
}

// ---- what the battle asks
bool8 U3D_IsTrainer(u16 trainerId)
{
    return IS_UNOVA_TRAINER(trainerId);
}

const u8 *U3D_ClassName(u16 trainerId)
{
    return sClassName[trainerId == TRAINER_UNOVA_B];
}

u8 U3D_Party(struct Pokemon *party, u16 trainerId, bool8 firstTrainer)
{
    u8 which = trainerId == TRAINER_UNOVA_B, i, max = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? PARTY_SIZE / 2 : PARTY_SIZE;
    if (firstTrainer)
        ZeroEnemyPartyMons();
    for (i = 0; i < sTeamCount[which] && i < max; i++)
        party[i] = sTeam[which][i];
    return sTeamCount[which];
}

// Black's prize: the trainer's cash byte x 4 x the last POKéMON's level (as Gen 5 pays it)
u32 U3D_Money(u16 trainerId)
{
    u32 size, level = 1, i;
    const u8 *t = U3D_Vm_Trainer(sBlackTrainer[trainerId == TRAINER_UNOVA_B], &size);
    if (t == NULL || size < 20)
        return 0;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
            level = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
    return t[17] * 4 * level * gBattleStruct->moneyMultiplier;
}

// Black's StartTrainerBattle: the first trainer, a second (0 none); FALSE when Black has no such trainer
bool8 U3D_PrepareBattle(u16 first, u16 second)
{
    u32 size;
    const u8 *t = U3D_Vm_Trainer(first, &size);
    bool8 doubles;
    u8 able = 0, i, n;
    if (t == NULL || size < 20)
        return FALSE;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            able++;
    doubles = (t[2] != 0 || second != 0) && able >= 2;   // (Black's Doubles, Triples and Rotations; one able POKéMON: Single)
    if (!SetTrainer(0, first, doubles))
        return FALSE;
    n = 1;
    if (second != 0 && second != first && SetTrainer(1, second, doubles))
        n = 2;
    gTrainerBattleOpponent_A = TRAINER_UNOVA_A;
    gTrainerBattleOpponent_B = n > 1 ? TRAINER_UNOVA_B : 0;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | (doubles ? BATTLE_TYPE_DOUBLE : 0) | (n > 1 ? BATTLE_TYPE_TWO_OPPONENTS : 0);
    sResult = 0;
    return TRUE;
}

void U3D_BattleEnded(bool8 won)
{
    sResult = won ? 1 : 2;
    MPNet_Log("unova: battle %s", won ? "won" : "lost");
}

// Black's own battle songs (music/mus_unova_vs_*.kaisong): the trainer's by its kind, else the wild one's
u8 U3D_Vm_TrainerSongKind(u16 id);
u16 U3D_BattleSong(void)
{
    static const u16 sSongs[] = {MUS_UNOVA_VS_TRAINER, MUS_UNOVA_VS_GYMLEADER, MUS_UNOVA_VS_RIVAL, MUS_UNOVA_VS_PLASMA,
                                 MUS_UNOVA_VS_SHITENNO, MUS_UNOVA_VS_CHAMP, MUS_UNOVA_VS_N, MUS_UNOVA_VS_G_CIS,
                                 MUS_UNOVA_VS_SUBWAY_TRAINER};
    u8 kind;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return MUS_UNOVA_VS_NORAPOKE;
    if (!U3D_IsTrainer(gTrainerBattleOpponent_A))
        return MUS_UNOVA_VS_TRAINER;
    kind = U3D_Vm_TrainerSongKind(sBlackTrainer[0]);
    return kind < ARRAY_COUNT(sSongs) ? sSongs[kind] : MUS_UNOVA_VS_TRAINER;
}

// ... and the victory's (the game's kinds of victory song to Black's)
u16 U3D_VictorySong(u16 song)
{
    switch (song)
    {
    case MUS_VICTORY_WILD:        return MUS_UNOVA_WIN1;
    case MUS_VICTORY_TRAINER:     return MUS_UNOVA_WIN2;
    case MUS_VICTORY_GYM_LEADER:  return MUS_UNOVA_WIN3;
    case MUS_VICTORY_AQUA_MAGMA:  return MUS_UNOVA_WIN4;
    case MUS_VICTORY_LEAGUE:      return MUS_UNOVA_WIN5;
    }
    return song;
}

// 0: none yet, 1 won, 2 lost
u8 U3D_BattleResult(void)
{
    return sResult;
}
