// POKéMON STADIUM's cups (docs/STADIUM.md, docs/stadium/RULES.md): their rules and the user's four switches, the
// registered teams, the records ("<save>.stadium"), a run of eight trainers, and its battles on Kai's battle system.
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_stadium.h"
#include "mp_stadium_cup.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "mp_kaisong_scene.h"

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const struct PokedexEntry gPokedexEntries[];
extern const u8 gTypeEffectiveness[];
extern char gPlatformSavePath[512];

// One type against another: 0 no effect, 1 not very effective, 2 normal, 4 super effective (battle_main.c's table)
static s32 GetTypeEffectiveness_Mp(u8 atk, u8 def)
{
    u32 i;
    for (i = 0; gTypeEffectiveness[i] != 0xFF; i += 3)
    {
        if (gTypeEffectiveness[i] == 0xFE)
            continue;
        if (gTypeEffectiveness[i] == atk && gTypeEffectiveness[i + 1] == def)
            return gTypeEffectiveness[i + 2] / 5;
    }
    return 2;
}

// ---------------------------------------------------------------------------------------------------------------
// The cups

static const struct StadiumCupInfo sCups[STADIUM_CUP_COUNT] = {
    //                 name                 tiers min max  sum  petit  mewtwo/mew allowed   divisions (POKé..MASTER)
    [STADIUM_CUP_PIKA]      = {"PIKA CUP",           1, 15,  20,  50, FALSE, FALSE, {STADIUM_PIKA}},
    [STADIUM_CUP_PETIT]     = {"PETIT CUP",          1, 25,  30,  80, TRUE,  FALSE, {STADIUM_PETIT}},
    [STADIUM_CUP_POKE]      = {"POKé CUP",           4, 50,  55, 155, FALSE, FALSE,
                               {STADIUM_POKE_POKE, STADIUM_POKE_GREAT, STADIUM_POKE_ULTRA, STADIUM_POKE_MASTER}},
    [STADIUM_CUP_PRIME]     = {"PRIME CUP",          4,  1, 100,   0, FALSE, TRUE,
                               {STADIUM_PRIME_POKE, STADIUM_PRIME_GREAT, STADIUM_PRIME_ULTRA, STADIUM_PRIME_MASTER}},
    [STADIUM_CUP_CHALLENGE] = {"CHALLENGE CUP",      4, 30,  75,   0, FALSE, FALSE, {0}},
    [STADIUM_CUP_CASTLE]    = {"GYM LEADER CASTLE",  1, 50,  55, 155, FALSE, FALSE, {STADIUM_GYM_1}},
    [STADIUM_CUP_MEWTWO]    = {"VS. MEWTWO",         1,  1, 100,   0, FALSE, TRUE,  {STADIUM_MEWTWO}},
};

const struct StadiumCupInfo *StadiumCup_Info(u8 cup)
{
    return cup < STADIUM_CUP_COUNT ? &sCups[cup] : &sCups[0];
}

static const u8 sCcLevels[4] = {30, 45, 60, 75};

// The rental list a cup lends from (the CHALLENGE CUP lends its own six)
static u8 RentalList(u8 cup)
{
    switch (cup)
    {
    case STADIUM_CUP_PIKA: return STADIUM_RENT_PIKA;
    case STADIUM_CUP_PETIT: return STADIUM_RENT_PETIT;
    case STADIUM_CUP_POKE: return STADIUM_RENT_POKE;
    case STADIUM_CUP_CASTLE: return STADIUM_RENT_CASTLE;
    default: return STADIUM_RENT_PRIME;
    }
}

u16 StadiumCup_RentalCount(u8 round, u8 cup)
{
    return cup == STADIUM_CUP_CHALLENGE ? 0 : Stadium_RentalCount(round, RentalList(cup));
}

const struct StadiumMon *StadiumCup_Rental(u8 round, u8 cup, u16 i)
{
    return Stadium_Rental(round, RentalList(cup), i);
}

// The level an entry battles at with LEVEL CAP on: the cap, or for a cup with a total the level any three fit at
u8 StadiumCup_CapLevel(u8 cup, u8 tier)
{
    const struct StadiumCupInfo *c = StadiumCup_Info(cup);
    if (cup == STADIUM_CUP_CHALLENGE)
        return sCcLevels[tier & 3];
    if (c->levelSum)
        return c->levelSum / 3 > c->levelMax ? c->levelMax : c->levelSum / 3;
    return c->levelMax;
}

// ---------------------------------------------------------------------------------------------------------------
// The rules

// The restricted legendaries and mythicals (the user's SPECIES switch, off: everything else may enter)
static const u16 sRestricted[] = {
    SPECIES_MEWTWO, SPECIES_MEW, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI, SPECIES_KYOGRE, SPECIES_GROUDON,
    SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS, SPECIES_DIALGA, SPECIES_PALKIA, SPECIES_GIRATINA,
    SPECIES_PHIONE, SPECIES_MANAPHY, SPECIES_DARKRAI, SPECIES_SHAYMIN, SPECIES_ARCEUS, SPECIES_VICTINI,
    SPECIES_RESHIRAM, SPECIES_ZEKROM, SPECIES_KYUREM, SPECIES_KELDEO, SPECIES_MELOETTA, SPECIES_GENESECT,
    SPECIES_XERNEAS, SPECIES_YVELTAL, SPECIES_ZYGARDE, SPECIES_DIANCIE, SPECIES_HOOPA, SPECIES_VOLCANION,
    SPECIES_COSMOG, SPECIES_COSMOEM, SPECIES_SOLGALEO, SPECIES_LUNALA, SPECIES_NECROZMA, SPECIES_MAGEARNA,
    SPECIES_MARSHADOW, SPECIES_ZERAORA, SPECIES_MELTAN, SPECIES_MELMETAL, SPECIES_ZACIAN, SPECIES_ZAMAZENTA,
    SPECIES_ETERNATUS, SPECIES_CALYREX, SPECIES_ZARUDE, SPECIES_KORAIDON, SPECIES_MIRAIDON, SPECIES_TERAPAGOS,
    SPECIES_PECHARUNT,
};

static bool8 Restricted(u16 species)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sRestricted); i++)
        if (sRestricted[i] == species)
            return TRUE;
    return FALSE;
}

// Has it a pre-evolution (the PETIT CUP takes the lowest stage only)? Among the first 151 with SPECIES on
static bool8 HasPreEvolution(u16 species, bool8 gen1Only)
{
    u16 s;
    u8 i;
    for (s = 1; s < NUM_SPECIES; s++)
    {
        if (gen1Only && s > 151)
            break;
        for (i = 0; i < EVOS_PER_MON; i++)
            if (gEvolutionTable[s][i].method != 0 && gEvolutionTable[s][i].targetSpecies == species)
                return TRUE;
    }
    return FALSE;
}

// Can this POKéMON enter? NULL, or the reason why not (shown on the registration screen)
const char *StadiumCup_Refuse(u8 cup, u8 tier, u8 options, u16 species, u8 level, const u16 *moves, u16 item)
{
    const struct StadiumCupInfo *c = StadiumCup_Info(cup);
    bool8 gen1 = (options & STADIUM_OPT_SPECIES) != 0;
    u16 dex = SpeciesToNationalPokedexNum(species);
    u8 i;
    (void)tier;
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return "AN EGG CAN'T ENTER";
    if (gen1 && (dex == 0 || dex > 151))
        return "ONLY THE FIRST 151";
    if (gen1)
        for (i = 0; i < MAX_MON_MOVES; i++)
            if (moves[i] > MOVE_SUBSTITUTE)
                return "A MOVE FROM A LATER GAME";
    if (!c->allowsMewtwo && (species == SPECIES_MEWTWO || species == SPECIES_MEW))
        return "NOT ALLOWED IN THIS CUP";
    if (!gen1 && !c->allowsMewtwo && Restricted(species))
        return "A RESTRICTED LEGENDARY";
    if (!(options & STADIUM_OPT_LEVEL_CAP) && (level < c->levelMin || level > c->levelMax))
        return c->levelMin > 1 ? "ITS LEVEL DOESN'T FIT" : "ITS LEVEL DOESN'T FIT";
    if (c->petit)
    {
        if (dex == 0 || gPokedexEntries[dex].height > 20 || gPokedexEntries[dex].weight > 200)
            return "TOO TALL OR TOO HEAVY";
        if (HasPreEvolution(species, gen1))
            return "ONLY THE LOWEST STAGE";
    }
    if (item != ITEM_NONE && !(options & STADIUM_OPT_HELD_ITEMS))
        return NULL;   // (its item stays home: HELD ITEMS off)
    return NULL;
}

// ---------------------------------------------------------------------------------------------------------------
// The records and the registered teams: "<save>.stadium"

#define STADIUM_FILE_MAGIC 0x4D445453   // "STDM"
#define STADIUM_FILE_VERSION 1

static struct StadiumSave sSave;
static bool8 sLoaded;
static char sLoadedFor[512];

static void FilePath(char *out, size_t size)
{
    snprintf(out, size, "%s.stadium", gPlatformSavePath);
}

static void Load(void)
{
    char path[600];
    FILE *f;
    u32 magic = 0, version = 0;
    if (sLoaded && strcmp(sLoadedFor, gPlatformSavePath) == 0)
        return;
    sLoaded = TRUE;
    snprintf(sLoadedFor, sizeof(sLoadedFor), "%s", gPlatformSavePath);
    memset(&sSave, 0, sizeof(sSave));
    FilePath(path, sizeof(path));
    if (!(f = fopen(path, "rb")))
        return;
    if (fread(&magic, 4, 1, f) == 1 && magic == STADIUM_FILE_MAGIC && fread(&version, 4, 1, f) == 1
     && version == STADIUM_FILE_VERSION)
    {
        if (fread(&sSave, sizeof(sSave), 1, f) != 1)
            memset(&sSave, 0, sizeof(sSave));
    }
    fclose(f);
}

void StadiumCup_SaveNow(void)
{
    char path[600], tmp[610];
    FILE *f;
    u32 magic = STADIUM_FILE_MAGIC, version = STADIUM_FILE_VERSION;
    Load();
    FilePath(path, sizeof(path));
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if (!(f = fopen(tmp, "wb")))
        return;
    fwrite(&magic, 4, 1, f);
    fwrite(&version, 4, 1, f);
    fwrite(&sSave, sizeof(sSave), 1, f);
    fclose(f);
    remove(path);
    rename(tmp, path);
}

struct StadiumSave *StadiumCup_Save(void)
{
    Load();
    return &sSave;
}

// The hardest settings a cup was won with: the four switches' hard sides as bits (STADIUM_HARD_*)
static u8 Hardness(u8 options, bool8 rentalsOnly)
{
    u8 h = 0;
    if (!(options & STADIUM_OPT_LEVEL_CAP))
        h |= STADIUM_HARD_LEVEL;
    if (options & STADIUM_OPT_SPECIES)
        h |= STADIUM_HARD_SPECIES;
    if (!(options & STADIUM_OPT_HELD_ITEMS))
        h |= STADIUM_HARD_ITEMS;
    if (rentalsOnly)
        h |= STADIUM_HARD_RENTALS;
    return h;
}

bool8 StadiumCup_Cleared(u8 round, u8 cup, u8 tier)
{
    Load();
    return round < STADIUM_ROUNDS && cup < STADIUM_CUP_COUNT && tier < 4 && sSave.cleared[round][cup][tier] != 0;
}

// A tier opens once the one before it is won (STADIUM's); Round 2 once MEWTWO is beaten in Round 1; MEWTWO once
// every cup's every tier and the castle are won in that round
bool8 StadiumCup_Open(u8 round, u8 cup, u8 tier)
{
    u8 c, t;
    if (round > 0 && !StadiumCup_Cleared(0, STADIUM_CUP_MEWTWO, 0))
        return FALSE;
    if (tier > 0 && !StadiumCup_Cleared(round, cup, tier - 1))
        return FALSE;
    if (cup == STADIUM_CUP_MEWTWO)
        for (c = 0; c < STADIUM_CUP_MEWTWO; c++)
            if (c != STADIUM_CUP_CHALLENGE)
                for (t = 0; t < sCups[c].tiers; t++)
                    if (!StadiumCup_Cleared(round, c, t))
                        return FALSE;
    return TRUE;
}

u16 StadiumCup_WinsTotal(void)
{
    u16 n = 0;
    u8 r, c, t;
    Load();
    for (r = 0; r < STADIUM_ROUNDS; r++)
        for (c = 0; c < STADIUM_CUP_COUNT; c++)
            for (t = 0; t < 4; t++)
                n += sSave.cleared[r][c][t] != 0;
    return n;
}

// The NURSE's words (mp_stadium.c MP_Stadium_BufferNews)
void StadiumRecords_News(char *out, size_t size)
{
    u16 wins = StadiumCup_WinsTotal();
    Load();
    if (wins == 0)
        snprintf(out, size, "No cups won yet. The STADIUM's\nchampions are waiting for you!");
    else
        snprintf(out, size, "You've won %d of the STADIUM's cups\nand classes, and %d battles in all.\f"
                            "The last cup you won: %s.", wins, sSave.battlesWon,
                 sSave.lastCup < STADIUM_CUP_COUNT ? sCups[sSave.lastCup].name : "-");
}

// ---------------------------------------------------------------------------------------------------------------
// A run: eight trainers (the castle: every gym's four, the ELITE FOUR, the RIVAL; MEWTWO: one), then the cup

static struct StadiumRun sRun;

struct StadiumRun *StadiumCup_Run(void)
{
    return &sRun;
}

u8 StadiumCup_TrainerCount(u8 round, u8 cup, u8 tier)
{
    const struct StadiumCupInfo *c = StadiumCup_Info(cup);
    if (cup == STADIUM_CUP_CHALLENGE)
        return 8;
    if (cup == STADIUM_CUP_CASTLE)
        return 8 * 4 + 4 + 1;
    return Stadium_TrainerCount(round, c->division[tier & 3]);
}

// The castle's trainer n: the gyms' four in order, the ELITE FOUR, the RIVAL (one of seven, by the player's six)
const struct StadiumTrainer *StadiumCup_CastleTrainer(u8 round, u8 n, u8 rival)
{
    if (n < 32)
        return Stadium_Trainer(round, STADIUM_GYM_1 + n / 4, n % 4);
    if (n < 36)
        return Stadium_Trainer(round, STADIUM_ELITE_FOUR, n - 32);
    return Stadium_Trainer(round, STADIUM_RIVAL, rival);
}

// The RIVAL's team: the one whose six do best against the player's (type matchups both ways)
u8 StadiumCup_PickRival(u8 round, const struct Pokemon *team, u8 count)
{
    u8 best = 6, k, i, j;
    s32 bestScore = -100000;
    for (k = 0; k < 7; k++)
    {
        const struct StadiumTrainer *t = Stadium_Trainer(round, STADIUM_RIVAL, k);
        s32 score = 0;
        if (t == NULL)
            continue;
        for (i = 0; i < t->count; i++)
        {
            const struct StadiumMon *m = Stadium_TrainerMon(t, i);
            for (j = 0; j < count; j++)
            {
                u16 s = GetMonData((struct Pokemon *)&team[j], MON_DATA_SPECIES);
                u8 a, b;
                for (a = 0; a < 2; a++)
                    for (b = 0; b < 2; b++)
                    {
                        score += GetTypeEffectiveness_Mp(gSpeciesInfo[m->species].types[a], gSpeciesInfo[s].types[b]);
                        score -= GetTypeEffectiveness_Mp(gSpeciesInfo[s].types[b], gSpeciesInfo[m->species].types[a]);
                    }
            }
        }
        if (score > bestScore)
            bestScore = score, best = k;
    }
    return best;
}

// ---------------------------------------------------------------------------------------------------------------
// The battles

bool8 gMpStadiumBattle;   // a STADIUM battle is on (battle_controller_opponent.c, battle_message.c)
static struct Pokemon sSavedParty[PARTY_SIZE];
static struct Pokemon sCcTeam[PARTY_SIZE];     // the CHALLENGE CUP's six for this run
static struct Pokemon sFoeTeam[PARTY_SIZE];    // the opponent's six
static u8 sFoeCount;
static const struct StadiumTrainer *sFoe;
static u8 sCcFoeClass;   // (the CHALLENGE CUP's trainers come from its own table)

// Full HP, no status, full PP, at a level (0: as it is)
static void Refresh(struct Pokemon *mon, u8 level)
{
    u16 hp;
    u32 status = 0;
    if (level)
    {
        u32 exp = gExperienceTables[gSpeciesInfo[GetMonData(mon, MON_DATA_SPECIES)].growthRate][level];
        SetMonData(mon, MON_DATA_EXP, &exp);
        SetMonData(mon, MON_DATA_LEVEL, &level);
    }
    CalculateMonStats(mon);
    hp = GetMonData(mon, MON_DATA_MAX_HP);
    SetMonData(mon, MON_DATA_HP, &hp);
    SetMonData(mon, MON_DATA_STATUS, &status);
    MonRestorePP(mon);
}

// The registered team's POKéMON as it enters (a copy: level adjusted with LEVEL CAP on, its item home with HELD
// ITEMS off). The CHALLENGE CUP's are the run's own.
void StadiumCup_Entry(struct Pokemon *out, u8 slot)
{
    struct StadiumRun *r = &sRun;
    if (r->cup == STADIUM_CUP_CHALLENGE)
    {
        *out = sCcTeam[slot % PARTY_SIZE];
        Refresh(out, 0);
        return;
    }
    {
        struct StadiumSave *s = StadiumCup_Save();
        const struct StadiumTeamSlot *t = &s->team[r->cup].slot[slot % STADIUM_TEAM_SIZE];
        u8 level;
        BoxMonToMon(&t->mon, out);
        level = GetMonData(out, MON_DATA_LEVEL);
        if ((r->options & STADIUM_OPT_LEVEL_CAP) && t->source != STADIUM_SRC_RENTAL)
            level = StadiumCup_CapLevel(r->cup, r->tier);
        if (!(r->options & STADIUM_OPT_HELD_ITEMS))
        {
            u16 none = ITEM_NONE;
            SetMonData(out, MON_DATA_HELD_ITEM, &none);
        }
        Refresh(out, level);
    }
}

u8 StadiumCup_EntryCount(void)
{
    if (sRun.cup == STADIUM_CUP_CHALLENGE)
        return PARTY_SIZE;
    return StadiumCup_Save()->team[sRun.cup].count;
}

// The opponent now (and its six, built: sFoeTeam)
const struct StadiumTrainer *StadiumCup_Foe(void)
{
    return sFoe;
}

struct Pokemon *StadiumCup_FoeTeam(u8 *count)
{
    *count = sFoeCount;
    return sFoeTeam;
}

const struct StadiumCcTrainer *StadiumCc_Trainer(u8 cls, u8 i);   // mp_stadium.c

// Who the run's next opponent is, and their six (the RIVAL picked against the player's six the first time)
void StadiumCup_PrepareFoe(void)
{
    struct StadiumRun *r = &sRun;
    u8 i;
    sFoe = NULL;
    sFoeCount = 0;
    if (r->cup == STADIUM_CUP_CHALLENGE)
    {
        StadiumCc_MakeTeam(sFoeTeam, r->tier, r->round, r->trainer);
        sFoeCount = PARTY_SIZE;
        sCcFoeClass = r->tier;
        return;
    }
    if (r->cup == STADIUM_CUP_CASTLE)
    {
        if (r->trainer == 36)
        {
            struct Pokemon team[PARTY_SIZE];
            u8 n = StadiumCup_EntryCount();
            for (i = 0; i < n; i++)
                StadiumCup_Entry(&team[i], i);
            r->rival = StadiumCup_PickRival(r->round, team, n);
        }
        sFoe = StadiumCup_CastleTrainer(r->round, r->trainer, r->rival);
    }
    else
        sFoe = Stadium_Trainer(r->round, StadiumCup_Info(r->cup)->division[r->tier], r->trainer);
    if (sFoe == NULL)
        return;
    for (i = 0; i < sFoe->count && i < PARTY_SIZE; i++)
        Stadium_BuildMon(&sFoeTeam[i], Stadium_TrainerMon(sFoe, i));
    sFoeCount = i;
}

// STADIUM's opponents pick their three after seeing the player's six: the ones whose types do best against them
void StadiumCup_FoePicks(void)
{
    struct StadiumRun *r = &sRun;
    s32 score[PARTY_SIZE];
    u8 i, j, n = StadiumCup_EntryCount(), want = sFoeCount < STADIUM_PICK ? sFoeCount : STADIUM_PICK;
    struct Pokemon mine[PARTY_SIZE];
    for (i = 0; i < n; i++)
        StadiumCup_Entry(&mine[i], i);
    for (i = 0; i < sFoeCount; i++)
    {
        u16 fs = GetMonData(&sFoeTeam[i], MON_DATA_SPECIES);
        score[i] = (Random() % 3);
        for (j = 0; j < n; j++)
        {
            u16 ms = GetMonData(&mine[j], MON_DATA_SPECIES);
            u8 a, b;
            for (a = 0; a < 2; a++)
                for (b = 0; b < 2; b++)
                    score[i] += GetTypeEffectiveness_Mp(gSpeciesInfo[fs].types[a], gSpeciesInfo[ms].types[b])
                              - GetTypeEffectiveness_Mp(gSpeciesInfo[ms].types[b], gSpeciesInfo[fs].types[a]);
        }
    }
    for (i = 0; i < want; i++)
    {
        u8 best = 0xFF;
        for (j = 0; j < sFoeCount; j++)
        {
            u8 k;
            bool8 taken = FALSE;
            for (k = 0; k < i; k++)
                taken |= r->foePicks[k] == j;
            if (!taken && (best == 0xFF || score[j] > score[best]))
                best = j;
        }
        r->foePicks[i] = best;
    }
    for (; i < STADIUM_PICK; i++)
        r->foePicks[i] = 0xFF;
}

// Start a run (the switches, and the CHALLENGE CUP's six for the player)
void StadiumCup_Begin(u8 round, u8 cup, u8 tier, u8 options)
{
    struct StadiumRun *r = &sRun;
    struct StadiumSave *s = StadiumCup_Save();
    u8 i;
    memset(r, 0, sizeof(*r));
    r->active = TRUE;
    r->round = round, r->cup = cup, r->tier = tier, r->options = options;
    r->rentalsOnly = cup != STADIUM_CUP_CHALLENGE && (options & STADIUM_OPT_RENTALS) && s->team[cup].count > 0;
    for (i = 0; i < s->team[cup].count; i++)
        if (s->team[cup].slot[i].source != STADIUM_SRC_RENTAL)
            r->rentalsOnly = FALSE;
    if (cup == STADIUM_CUP_CHALLENGE)
        StadiumCc_MakeTeam(sCcTeam, tier, round, STADIUM_CC_PLAYER);
    s->options[cup] = options | 0x80;
    StadiumCup_PrepareFoe();
    MPNet_Log("stadium: %s class %d round %d begins (options %d, rentals only %d)", StadiumCup_Info(cup)->name, tier,
              round + 1, options, r->rentalsOnly);
}

// The rewards, the ribbon and the achievements once a cup is won (mp_stadium_rewards.c)
void StadiumRewards_CupWon(u8 round, u8 cup, u8 tier, u8 hardness, bool8 first);

static void (*sAfterBattle)(bool8 won);

static void CB2_EndStadiumBattle(void)
{
    bool8 won = gBattleOutcome == B_OUTCOME_WON;
    struct StadiumRun *r = &sRun;
    u8 i, fainted = 0;
    for (i = 0; i < STADIUM_PICK; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gPlayerParty[i], MON_DATA_HP) == 0)
            fainted++;
    memcpy(gPlayerParty, sSavedParty, sizeof(gPlayerParty));
    CalculatePlayerPartyCount();
    gMpStadiumBattle = FALSE;
    MP_MUSIC_SCENE_RELEASE();
    r->lostNone = won && fainted == 0;
    MPNet_Log("stadium: battle %d %s (%d fainted)", r->trainer + 1, won ? "won" : "lost", fainted);
    if (won)
    {
        struct StadiumSave *s = StadiumCup_Save();
        s->battlesWon++;
        if (r->lostNone && r->continues < 9)
            r->continues++;
    }
    SetMainCallback2(CB2_ReturnToField);
    if (sAfterBattle)
        sAfterBattle(won);
}

// The run's opponent beaten: on to the next, or the cup won. TRUE: the cup is won (rewards given)
bool8 StadiumCup_Advance(void)
{
    struct StadiumRun *r = &sRun;
    struct StadiumSave *s = StadiumCup_Save();
    u8 hard, total = StadiumCup_TrainerCount(r->round, r->cup, r->tier);
    r->trainer++;
    if (r->trainer < total)
    {
        StadiumCup_PrepareFoe();
        return FALSE;
    }
    hard = Hardness(r->options, r->rentalsOnly);
    if (r->cup == STADIUM_CUP_CHALLENGE)
        hard = STADIUM_HARD_ALL;   // (its six are given: its own rewards, the highest)
    {
        bool8 first = !(s->cleared[r->round][r->cup][r->tier] & (1 << hard));
        s->cleared[r->round][r->cup][r->tier] |= 1 << hard;
        s->cupsWon++;
        s->lastCup = r->cup;
        StadiumCup_SaveNow();
        StadiumRewards_CupWon(r->round, r->cup, r->tier, hard, first);
    }
    r->active = FALSE;
    return TRUE;
}

// A lost battle: a continue if there's one (TRUE), else the run ends
bool8 StadiumCup_UseContinue(void)
{
    struct StadiumRun *r = &sRun;
    if (r->continues > 0)
    {
        r->continues--;
        return TRUE;
    }
    r->active = FALSE;
    return FALSE;
}

static void Task_StartStadiumBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndStadiumBattle;
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

// The battle: the player's three chosen (r->picks) against the opponent's three (r->foePicks); afterwards
// `after(won)` runs (the screens come back)
void StadiumCup_StartBattle(void (*after)(bool8 won))
{
    struct StadiumRun *r = &sRun;
    u8 i;
    sAfterBattle = after;
    memcpy(sSavedParty, gPlayerParty, sizeof(gPlayerParty));
    ZeroPlayerPartyMons();
    for (i = 0; i < STADIUM_PICK; i++)
        if (r->picks[i] < StadiumCup_EntryCount())
            StadiumCup_Entry(&gPlayerParty[i], r->picks[i]);
    CalculatePlayerPartyCount();
    ZeroEnemyPartyMons();
    for (i = 0; i < STADIUM_PICK; i++)
        if (r->foePicks[i] < sFoeCount)
        {
            gEnemyParty[i] = sFoeTeam[r->foePicks[i]];
            Refresh(&gEnemyParty[i], 0);
        }
    gMpStadiumBattle = TRUE;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_EREADER_TRAINER;
    gTrainerBattleOpponent_A = 0;
    LockPlayerFieldControls();
    CreateTask(Task_StartStadiumBattle, 1);
    MP_MUSIC_SCENE_HOLD(MUS_VS_TRAINER, MUS_VICTORY_TRAINER);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_E_READER));
    MPNet_Log("stadium: battle %d against %s", r->trainer + 1, sFoe ? "a STADIUM trainer" : "a CHALLENGE CUP trainer");
}

// ---- the battle's side (battle_controller_opponent.c, battle_message.c) ----

u16 MP_StadiumBattle_TrainerPic(void)
{
    if (sRun.cup == STADIUM_CUP_CHALLENGE)
        return StadiumCc_Trainer(sCcFoeClass, sRun.trainer)->pic;
    return sFoe && sFoe->pic ? sFoe->pic : TRAINER_PIC_MP_OPPONENT;
}

static const u8 sText_Stadium[] = _("STADIUM");
static const u8 sText_Leader[] = _("LEADER");
static const u8 sText_EliteFour[] = _("ELITE FOUR");
static const u8 sText_Champion[] = _("CHAMPION");
static const u8 sText_Lose[] = _("What a battle! The crowd goes wild!");
static const u8 sText_Win[] = _("That's the STADIUM for you!");

const u8 *MP_StadiumBattle_ClassName(void)
{
    if (sRun.cup == STADIUM_CUP_CHALLENGE)
        return StadiumCc_Trainer(sCcFoeClass, sRun.trainer)->className;
    if (sRun.cup == STADIUM_CUP_CASTLE)
        return sRun.trainer >= 36 ? sText_Champion : sRun.trainer >= 32 ? sText_EliteFour
             : sRun.trainer % 4 == 3 ? sText_Leader : sText_Stadium;
    return sText_Stadium;
}

void MP_StadiumBattle_TrainerName(u8 *out)
{
    if (sRun.cup == STADIUM_CUP_CHALLENGE)
        StringCopy(out, StadiumCc_Trainer(sCcFoeClass, sRun.trainer)->name);
    else if (sFoe)
        StringCopy(out, sFoe->name);
    else
        out[0] = EOS;
}

const u8 *MP_StadiumBattle_LoseText(void) { return sText_Lose; }
const u8 *MP_StadiumBattle_WinText(void) { return sText_Win; }
