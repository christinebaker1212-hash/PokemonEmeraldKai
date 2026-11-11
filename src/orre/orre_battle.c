// Moemon Kai: ORRE's battles (docs/ORRE_HANDOFF.md step 3, "Trainers and battles"). Colosseum's own battle, trainer,
// trainer class and trainer Pokémon tables, read at run time from common_rel in ORRE's data pack (nothing of them is
// in this repository), fought as Kai's own trainer battles (the user's decision: Kai's battles, Doubles as Colosseum's):
//
//   battles (fightEncount)   .data + 0x109FBC, 0x38 each (count + 0x14A6B8): + 0 the kind, + 1 trainers a side,
//                            + 2 a loss ends the game (zenmetu: Kai's whiteout), + 4 the field, + 8 the battle's name,
//                            + 0xC its song, + 0x18 four slots {u16 trainer, u16 pad, u32 who plays it: 1 the player}
//                            (the first "trainers a side" slots are the player's side, the next the opponents')
//   trainers                 .data + 0x91684, 0x34 each (count + 0x14A6AC): + 0 gender, + 2 class, + 4 the first of
//                            their Pokémon, + 6 AI, + 8 name, + 0x14 eight items, + 0x24 / 0x28 / 0x2C lines
//   classes (fightTrainerKind) .data + 0x8F724, 0xC each (count + 0x14A684): + 0 the prize rate, + 4 the name
//   trainer Pokémon          .data + 0x9E5DC, 0x50 each (count + 0x14A6B4): + 0 Ability slot, + 1 gender, + 2 nature
//                            (255 any), + 3 the Shadow id, + 4 level, + 0xA species (Gen 3's numbers: Kai's), + 0xC
//                            ball, + 0x12 item, + 0x1C IVs x6, + 0x22 EVs x6 (u16), moves at + 0x36 + 8n (0: its
//                            level's own; 0xFFFF none)
//
// A battle fills TRAINER_ORRE_A (and _B for a second opponent) in a writable copy of the game's trainer table (the
// Moemon Maker's patched one, if any, copied with it): the trainer's name, its class's name (battle_message.c asks
// Orre_ClassName), up to four of its items; CreateNPCTrainerParty asks Orre_Party for the team, built here with
// Colosseum's natures, genders, IVs, EVs, Abilities and moves (Shadow Pokémon: orre_shadow.c), raised by level
// scaling like any trainer's. The prize: Colosseum's (fight_range_exact_8021D9C0.c): the class's rate x the highest
// level on the field x 2 in a Double Battle. The battle picture is the person a script names (Orre_SetFoePerson).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "orre.h"
#include "orre_data.h"
#include "orre_people.h"
#include "orre_shadow.h"
#include "pokemon.h"
#include "random.h"
#include "string_util.h"
#include "constants/abilities.h"
#include "constants/characters.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "mp.h"
#include "sound.h"
#include "orre_sound.h"

void Orre_Vm_RoomSound(void);   // orre_vm.c

#define BATTLES 0x109FBC
#define BATTLE_COUNT 0x14A6B8
#define BATTLE_SIZE 0x38
#define TRAINERS 0x91684
#define TRAINER_COUNT 0x14A6AC
#define TRAINER_SIZE 0x34
#define KINDS 0x8F724
#define KIND_COUNT 0x14A684
#define KIND_SIZE 0xC
#define TMONS 0x9E5DC
#define TMON_COUNT 0x14A6B4
#define TMON_SIZE 0x50

#define LAST_COLO_ITEM 348        // (Colosseum's items to here are Gen 3's numbers; past it its own key items)

extern const struct Trainer gTrainersBase[];
extern const u32 gTrainersBaseCount;
void MPNet_Log(const char *fmt, ...);

static u8 *sRel;
static const u8 *sData;
static u32 sDataSize;
static bool8 sTried;

static struct Trainer *sTable;          // the writable copy the game reads while ORRE's trainers are set
static struct Pokemon sTeam[2][PARTY_SIZE];
static u8 sTeamCount[2];
static u16 sColoTrainer[2];             // the Colosseum trainers in TRAINER_ORRE_A / _B
static u8 sClassName[2][TRAINER_NAME_LENGTH + 4];
static u16 sBattle;                     // the battle under way (0: none)
static u8 sResult;                      // the last battle's: ORRE_BATTLE_WON / _LOST

static u32 Be32(const u8 *p) { return (u32)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static u16 Be16(const u8 *p) { return p[0] << 8 | p[1]; }

static bool8 Load(void)
{
    u32 size, st, off;
    if (sTried)
        return sData != NULL;
    sTried = TRUE;
    if ((sRel = OrreData_Read("common/common_rel.rel", &size)) == NULL)
        return sTried = FALSE;   // (no data pack yet: looked for again next time: OrreData_Read keeps the pace)
    if (size < 0x40)
        return FALSE;
    st = Be32(sRel + 0x10);
    if (Be32(sRel + 0x0C) < 5 || st + 8 * 5 > size)
        return FALSE;
    off = Be32(sRel + st + 8 * 4) & ~3u;
    sDataSize = Be32(sRel + st + 8 * 4 + 4);
    if (off + sDataSize > size || sDataSize < BATTLE_COUNT + 4)
        return FALSE;
    sData = sRel + off;
    return TRUE;
}

static const u8 *Rec(u32 table, u32 countAt, u32 size, u32 i)
{
    if (!Load() || i >= Be32(sData + countAt) || table + (i + 1) * size > sDataSize)
        return NULL;
    return sData + table + i * size;
}

// Colosseum's battle under way (0: none)
u16 Orre_BattleCurrent(void)
{
    return sBattle;
}

u16 Orre_BattleCount(void)
{
    return Load() ? Be32(sData + BATTLE_COUNT) : 0;
}

// ---- the trainer table the game reads

static struct Trainer *Slot(u16 trainerId)
{
    if (gTrainersTable != sTable)   // (the base table, or the Maker's patched copy read again: copied as it is)
    {
        if (sTable == NULL && (sTable = malloc(sizeof(struct Trainer) * gTrainersBaseCount)) == NULL)
            return NULL;
        memcpy(sTable, gTrainersTable, sizeof(struct Trainer) * gTrainersBaseCount);
        gTrainersTable = sTable;
    }
    return &sTable[trainerId];
}

// Colosseum's text (orre_text.c) in at most N letters (a trainer's name: TRAINER_NAME_LENGTH)
static void TextInto(u8 *out, u32 msg, u32 n)
{
    const u8 *t = Orre_Text(msg);
    u32 i;
    for (i = 0; i < n && t[i] != EOS && t[i] != CHAR_NEWLINE; i++)
        out[i] = t[i];
    out[i] = EOS;
}

static u16 Item(u16 coloItem)
{
    return coloItem <= LAST_COLO_ITEM ? coloItem : ITEM_NONE;
}

// ---- a trainer's team

static void BuildMon(struct Pokemon *mon, const u8 *r, u16 trainerId, u8 partyMax)
{
    u16 species = Be16(r + 0xA), item = Item(Be16(r + 0x12)), moves[MAX_MON_MOVES], ev;
    u8 level = r[4], nature = r[2] < NUM_NATURES ? r[2] : Random() % NUM_NATURES, gender, i, iv, abil = r[0] & 1;
    u8 ball = Be16(r + 0xC) && Be16(r + 0xC) <= ITEM_PREMIER_BALL ? Be16(r + 0xC) : ITEM_POKE_BALL;
    u8 shadow = r[3];
    u8 scaled = MP_LevelScale_TrainerMon(trainerId, level, partyMax);
    gender = r[1] == 0 ? MON_MALE : r[1] == 1 ? MON_FEMALE : MON_GENDERLESS;
    if (shadow && Orre_Shadow_MakeEnemy(mon, shadow))
    {
        // (Colosseum's Shadow table has the rest: its level raised with its trainer's team when scaling is on)
        if (scaled > GetMonData(mon, MON_DATA_LEVEL))
        {
            u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][scaled];
            SetMonData(mon, MON_DATA_EXP, &exp);
            CalculateMonStats(mon);
        }
        return;
    }
    if (scaled + gMpEnemyLevelBonus > level)
        level = min(MAX_LEVEL, scaled + gMpEnemyLevelBonus);
    CreateMonWithGenderNatureLetter(mon, species, level, 0, gender, nature, 0);
    for (i = 0; i < NUM_STATS; i++)
    {
        iv = r[0x1C + i] > MAX_PER_STAT_IVS ? MAX_PER_STAT_IVS : r[0x1C + i];
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
        ev = Be16(r + 0x22 + 2 * i) > 255 ? 255 : Be16(r + 0x22 + 2 * i);
        SetMonData(mon, MON_DATA_HP_EV + i, &ev);
    }
    if (gSpeciesInfo[species].abilities[1] != ABILITY_NONE)
        SetMonData(mon, MON_DATA_ABILITY_NUM, &abil);
    SetMonData(mon, MON_DATA_HELD_ITEM, &item);
    SetMonData(mon, MON_DATA_POKEBALL, &ball);
    for (i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = Be16(r + 0x36 + 8 * i);
    if (moves[0] != 0 && moves[0] != 0xFFFF)   // (else the moves its level gives it, as Colosseum's 0 means)
        for (i = 0; i < MAX_MON_MOVES; i++)
        {
            u16 m = moves[i] <= MOVE_PSYCHO_BOOST && moves[i] != 0xFFFF ? moves[i] : MOVE_NONE;
            u8 pp = m ? gBattleMoves[m].pp : 0;
            SetMonData(mon, MON_DATA_MOVE1 + i, &m);
            SetMonData(mon, MON_DATA_PP1 + i, &pp);
        }
    if (scaled > r[4])
        MP_LevelScale_EvolveMon(mon);
    CalculateMonStats(mon);
}

static u8 BuildTeam(u8 which, u16 coloTrainer, u16 trainerId)
{
    const u8 *t = Rec(TRAINERS, TRAINER_COUNT, TRAINER_SIZE, coloTrainer), *r;
    u16 first;
    u8 i, n = 0, partyMax = 0;
    memset(sTeam[which], 0, sizeof(sTeam[which]));
    if (t == NULL)
        return 0;
    first = Be16(t + 4);
    for (i = 0; i < PARTY_SIZE; i++)
        if ((r = Rec(TMONS, TMON_COUNT, TMON_SIZE, first + i)) != NULL && Be16(r + 0xA) != SPECIES_NONE && r[4] > partyMax)
            partyMax = r[4];
    MP_LevelScale_LogTrainer(trainerId, partyMax);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        r = Rec(TMONS, TMON_COUNT, TMON_SIZE, first + i);
        if (r == NULL || Be16(r + 0xA) == SPECIES_NONE || Be16(r + 0xA) > SPECIES_CHIMECHO)
            continue;
        BuildMon(&sTeam[which][n++], r, trainerId, partyMax);
    }
    return n;
}

// Fill TRAINER_ORRE_A + WHICH as Colosseum's trainer COLOTRAINER (FALSE: no such trainer)
static bool8 SetTrainer(u8 which, u16 coloTrainer, bool8 doubles)
{
    const u8 *t = Rec(TRAINERS, TRAINER_COUNT, TRAINER_SIZE, coloTrainer), *k;
    u16 id = which ? TRAINER_ORRE_B : TRAINER_ORRE_A;
    struct Trainer *s = Slot(id);
    u8 i, n;
    if (t == NULL || s == NULL)
        return FALSE;
    *s = gTrainersBase[id];
    TextInto(s->trainerName, Be32(t + 8), TRAINER_NAME_LENGTH);
    s->encounterMusic_gender = (t[0] ? F_TRAINER_FEMALE : 0) | TRAINER_ENCOUNTER_MUSIC_MALE;
    for (i = 0, n = 0; i < 8 && n < MAX_TRAINER_ITEMS; i++)
        if (Item(Be16(t + 0x14 + 2 * i)) != ITEM_NONE)
            s->items[n++] = Item(Be16(t + 0x14 + 2 * i));
    s->doubleBattle = doubles;
    s->aiFlags = AI_SCRIPT_CHECK_BAD_MOVE | AI_SCRIPT_TRY_TO_FAINT | AI_SCRIPT_CHECK_VIABILITY;
    sTeamCount[which] = BuildTeam(which, coloTrainer, id);
    s->partySize = sTeamCount[which];
    sColoTrainer[which] = coloTrainer;
    k = Rec(KINDS, KIND_COUNT, KIND_SIZE, Be16(t + 2));
    if (k)
        TextInto(sClassName[which], Be32(k + 4), TRAINER_NAME_LENGTH + 2);
    else
        sClassName[which][0] = EOS;
    MPNet_Log("orre: trainer %s %d (Colosseum's %d): %d POKeMON, %s", which ? "B" : "A", id, coloTrainer,
              sTeamCount[which], doubles ? "double" : "single");
    return TRUE;
}

// ---- what the battle asks

bool8 Orre_IsTrainer(u16 trainerId)
{
    return IS_ORRE_TRAINER(trainerId);
}

// The class's name of TRAINER_ORRE_A / _B (Colosseum's: RIDER, CIPHER PEON...)
const u8 *Orre_ClassName(u16 trainerId)
{
    return sClassName[trainerId == TRAINER_ORRE_B];
}

// battle_main.c CreateNPCTrainerParty: the team, into PARTY
u8 Orre_Party(struct Pokemon *party, u16 trainerId, bool8 firstTrainer)
{
    u8 which = trainerId == TRAINER_ORRE_B, i, max = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? PARTY_SIZE / 2 : PARTY_SIZE;
    if (firstTrainer)
        ZeroEnemyPartyMons();
    for (i = 0; i < sTeamCount[which] && i < max; i++)
        party[i] = sTeam[which][i];
    return sTeamCount[which];
}

// battle_script_commands.c GetTrainerMoneyToGive: Colosseum's prize
u32 Orre_Money(u16 trainerId)
{
    const u8 *t = Rec(TRAINERS, TRAINER_COUNT, TRAINER_SIZE, sColoTrainer[trainerId == TRAINER_ORRE_B]), *k;
    u32 best = 0, i, rate;
    if (t == NULL || (k = Rec(KINDS, KIND_COUNT, KIND_SIZE, Be16(t + 2))) == NULL)
        return 0;
    rate = Be16(k);
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gEnemyParty[i], MON_DATA_LEVEL) > best)
            best = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
    return rate * best * ((gBattleTypeFlags & BATTLE_TYPE_DOUBLE) ? 2 : 1) * gBattleStruct->moneyMultiplier;
}

// ---- a battle

// Set up Colosseum's battle BATTLE (its opponents in TRAINER_ORRE_A / _B): FALSE if there's no such battle or no
// opponent in it
bool8 Orre_PrepareBattle(u16 battle)
{
    const u8 *b = Rec(BATTLES, BATTLE_COUNT, BATTLE_SIZE, battle);
    u8 perSide, i, n = 0;
    bool8 doubles;
    u16 foes[2] = {0, 0};
    if (b == NULL)
    {
        MPNet_Log("orre: no battle %d (%d in the pack)", battle, Orre_BattleCount());
        return FALSE;
    }
    perSide = b[1] ? b[1] : 1;
    for (i = perSide; i < 2 * perSide && i < 4 && n < 2; i++)
        if (Be16(b + 0x18 + 8 * i) != 0 && Be32(b + 0x1C + 8 * i) != 1)
            foes[n++] = Be16(b + 0x18 + 8 * i);
    if (n == 0)
        return FALSE;
    // (ORRE's battles are Doubles, as Colosseum's are: the user's decision. With only one POKéMON able to fight the
    // player can't fill both places (the engine would send the same one out twice): a Single Battle then, the second
    // trainer's team after the first's)
    {
        u8 able = 0;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
             && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
                able++;
        doubles = able >= 2;
    }
    for (i = 0; i < n; i++)
        SetTrainer(i, foes[i], doubles);
    if (!doubles && n > 1)
    {
        for (i = 0; i < sTeamCount[1] && sTeamCount[0] < PARTY_SIZE; i++)
            sTeam[0][sTeamCount[0]++] = sTeam[1][i];
        if (Slot(TRAINER_ORRE_A) != NULL)
            Slot(TRAINER_ORRE_A)->partySize = sTeamCount[0];
        n = 1;
    }
    sBattle = battle;
    gTrainerBattleOpponent_A = TRAINER_ORRE_A;
    gTrainerBattleOpponent_B = n > 1 ? TRAINER_ORRE_B : 0;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | (doubles ? BATTLE_TYPE_DOUBLE : 0) | (n > 1 ? BATTLE_TYPE_TWO_OPPONENTS : 0);
    MPNet_Log("orre: battle %d (kind %d, field %d): %d opponent%s, a loss %s", battle, b[0], Be16(b + 4), n,
              n == 1 ? "" : "s", b[2] ? "whites out" : "goes on");
    return TRUE;
}

// ---- the battle's song, Colosseum's own (fightEncountGetBgmSndDataId): the battle's (+ 0xC), else its battle
// floor's (+ 4 its row of the battle floors, .data 0x8F974, 24 bytes: + 0xC), else its first trainer's kind's
// (+ 8), else none. A sound id of Colosseum's table (include/orre_sound.h)
#define FIGHT_FLOORS 0x8F974
#define FIGHT_FLOOR_SIZE 24
#define FIGHT_FLOOR_ROWS 50
static bool8 sBattleSong;

static u32 BattleSong(void)
{
    const u8 *b = Rec(BATTLES, BATTLE_COUNT, BATTLE_SIZE, sBattle), *t, *k;
    u32 id, i;
    if (b == NULL || sBattle == 0)
        return 0;
    if ((id = Be32(b + 0xC)) != 0)
        return id;
    if (Be16(b + 4) < FIGHT_FLOOR_ROWS && sData != NULL
     && (id = Be32(sData + FIGHT_FLOORS + Be16(b + 4) * FIGHT_FLOOR_SIZE + 0xC)) != 0)
        return id;
    for (i = 0; i < 4; i++)
    {
        u16 tr = Be16(b + 0x18 + 8 * i);
        if (tr != 0 && (t = Rec(TRAINERS, TRAINER_COUNT, TRAINER_SIZE, tr)) != NULL
         && (k = Rec(KINDS, KIND_COUNT, KIND_SIZE, Be16(t + 2))) != NULL && (id = Be32(k + 8)) != 0)
            return id;
    }
    return 0;
}

// pokemon.c PlayBattleBGM / PlayMapChosenOrBattleBGM: an ORRE battle starts Colosseum's song for it in place of
// Kai's (TRUE: Kai plays none). The wind and the room's song give way; the room's sound script brings them back
bool8 Orre_BattleMusic(void)
{
    u32 id = BattleSong();
    if (id <= 1)
        return FALSE;
    OrreSound_StopAmbient(300);
    if (!OrreSound_Play(id, 127, TRUE))
        return FALSE;
    MPNet_Log("orre: battle %d plays Colosseum's song %u", sBattle, id);
    return sBattleSong = TRUE;
}

// the audio frame (orre_audio.c): Colosseum's song plays through this battle; Kai's own music starting in it (the
// victory fanfare) ends the song
bool8 Orre_BattleSongOn(void)
{
    if (sBattleSong && gMain.inBattle && IsBGMPlaying())
    {
        OrreSound_StopBgm(150);
        sBattleSong = FALSE;
    }
    return sBattleSong;
}

bool8 Orre_BattleLossWhitesOut(void)
{
    const u8 *b = Rec(BATTLES, BATTLE_COUNT, BATTLE_SIZE, sBattle);
    return b == NULL || b[2] != 0;
}

void Orre_BattleEnded(bool8 won)
{
    sResult = won ? ORRE_BATTLE_WON : ORRE_BATTLE_LOST;
    MPNet_Log("orre: battle %d %s", sBattle, won ? "won" : "lost");
    sBattle = 0;
    sBattleSong = FALSE;
    Orre_Vm_RoomSound();   // (Colosseum's floorStartSoundScript: the room's song and wind again)
    Orre_People_BattleEnd();
    Orre_VmBattleDone();   // (a scene's battle: its script goes on with the result, src/orre/orre_vm.c)
}

u8 Orre_BattleResult(void)
{
    return sResult;
}

// (special, from a script) Colosseum's battle VAR_0x8004; the picture: VAR_0x8005's person (0xFFFF none). The script
// goes on after it; VAR_RESULT is the result (Orre_BattleResultToVar)
void Orre_StartBattle(void)
{
    if (!Orre_PrepareBattle(gSpecialVar_0x8004))
        return;
    if (gSpecialVar_0x8005 != 0xFFFF)
        Orre_People_SetFoe(0, gSpecialVar_0x8005);
    BattleSetup_StartOrreBattle();
}

void Orre_BattleResultToVar(void)
{
    gSpecialVar_Result = sResult;
}
