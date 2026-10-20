// Moemon Co-op: achievements. Personal accomplishments (each save earns its own),
// shown in CO-OP menu > DAILY > ACHIEVEMENTS. Most are read straight from the
// game (badges, game stats like eggs hatched or contests won, the POKéDEX); the
// rest are counted here by hooks around the co-op code (MP_Achieve_Count) or
// unlocked by a single feat (MP_Achieve_Unlock: a flawless gym battle, a shiny...).
// Every one gives MOEGATCHA pulls: 3, a 10-pull for the harder ones, 20 for the hardest.
//
// Unlocked achievements are kept next to the save ("av:<id>", mp_savedata.c),
// counters as "acnt:<name>".
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "event_data.h"
#include "money.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "data.h"
#include "constants/trainers.h"
#include "constants/species.h"
#include "constants/flags.h"
#include "constants/game_stat.h"

s32 MP_Gatcha_Pulls(void);
void MP_Gatcha_AddPulls(s32 n);
void MP_Events_QueueToast(const char *ascii);
s32 MP_Events_StreakDays(void);

enum { AK_FLAG, AK_STAT, AK_COUNT, AK_CAUGHT, AK_SEEN, AK_LEVEL, AK_EVENT, AK_STREAK, AK_BADGES };

struct Achievement
{
    const char *id;      // also the counter name for AK_COUNT
    const char *name;
    const char *desc;
    u8 kind;
    u16 arg;             // flag / game stat
    u16 target;
    u8 tier;             // reward tier: 0 = 3 pulls, 1 = 10 pulls, 2 = 20 pulls (AchPulls)
};

static u8 AchPulls(u8 tier)
{
    static const u8 sPulls[] = {3, 10, 20};
    return sPulls[tier < ARRAY_COUNT(sPulls) ? tier : 0];
}

static const struct Achievement sAch[] = {
    // ---- The journey
    {"badge1", "STONE BADGE", "Beat ROXANNE in RUSTBORO CITY.", AK_FLAG, FLAG_BADGE01_GET, 1, 0},
    {"badge2", "KNUCKLE BADGE", "Beat BRAWLY in DEWFORD TOWN.", AK_FLAG, FLAG_BADGE02_GET, 1, 0},
    {"badge3", "DYNAMO BADGE", "Beat WATTSON in MAUVILLE CITY.", AK_FLAG, FLAG_BADGE03_GET, 1, 0},
    {"badge4", "HEAT BADGE", "Beat FLANNERY in LAVARIDGE TOWN.", AK_FLAG, FLAG_BADGE04_GET, 1, 1},
    {"badge5", "BALANCE BADGE", "Beat NORMAN in PETALBURG CITY.", AK_FLAG, FLAG_BADGE05_GET, 1, 0},
    {"badge6", "FEATHER BADGE", "Beat WINONA in FORTREE CITY.", AK_FLAG, FLAG_BADGE06_GET, 1, 0},
    {"badge7", "MIND BADGE", "Beat TATE & LIZA in MOSSDEEP CITY.", AK_FLAG, FLAG_BADGE07_GET, 1, 0},
    {"badge8", "RAIN BADGE", "Beat JUAN in SOOTOPOLIS CITY.", AK_FLAG, FLAG_BADGE08_GET, 1, 1},
    {"champion", "CHAMPION", "Enter the HALL OF FAME.", AK_FLAG, FLAG_SYS_GAME_CLEAR, 1, 2},
    {"kyogre", "SEA BASIN", "Face KYOGRE in the CAVE OF ORIGIN.", AK_FLAG, FLAG_DEFEATED_KYOGRE, 1, 1},
    {"groudon", "CONTINENT", "Face GROUDON.", AK_FLAG, FLAG_DEFEATED_GROUDON, 1, 1},
    {"rayquaza", "SKY HIGH", "Face RAYQUAZA at the top of SKY PILLAR.", AK_FLAG, FLAG_DEFEATED_RAYQUAZA, 1, 1},
    {"regis", "THE SEALED ONES", "Face REGIROCK, REGICE and REGISTEEL.", AK_EVENT, 0, 1, 2},

    // ---- Battle feats
    {"flawless", "FLAWLESS", "Beat a gym leader without one of your POK~MON fainting.", AK_EVENT, 0, 1, 1},
    {"underdog", "UNDERDOG", "Beat a gym leader while their ace outlevels all of your POK~MON.", AK_EVENT, 0, 1, 1},
    {"solo", "ONE-MAN ARMY", "Beat a gym leader with just one POK~MON in your party.", AK_EVENT, 0, 1, 2},
    {"trainers50", "ROOKIE", "Battle 50 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 50, 0},
    {"trainers200", "VETERAN", "Battle 200 trainers.", AK_STAT, GAME_STAT_TRAINER_BATTLES, 200, 1},
    {"wild250", "WILD AT HEART", "Battle 250 wild POK~MON.", AK_STAT, GAME_STAT_WILD_BATTLES, 250, 0},
    {"lv50", "HALFWAY THERE", "Raise a POK~MON to Lv 50.", AK_LEVEL, 0, 50, 1},
    {"lv100", "LEVEL 100", "Raise a POK~MON to Lv 100.", AK_LEVEL, 0, 100, 2},
    {"evolve1", "IT'S EVOLVING!", "Evolve a POK~MON.", AK_STAT, GAME_STAT_EVOLVED_POKEMON, 1, 0},
    {"evolve20", "EVOLUTIONIST", "Evolve 20 POK~MON.", AK_STAT, GAME_STAT_EVOLVED_POKEMON, 20, 1},
    {"struggle", "LAST RESORT", "Run out of PP and use STRUGGLE.", AK_STAT, GAME_STAT_USED_STRUGGLE, 1, 0},

    // ---- Catching
    {"dex25", "COLLECTOR", "Catch 25 kinds of POK~MON.", AK_CAUGHT, 0, 25, 0},
    {"dex60", "RESEARCHER", "Catch 60 kinds of POK~MON.", AK_CAUGHT, 0, 60, 1},
    {"dex120", "PROFESSOR'S PRIDE", "Catch 120 kinds of POK~MON.", AK_CAUGHT, 0, 120, 1},
    {"dex200", "LIVING DEX", "Catch 200 kinds of POK~MON.", AK_CAUGHT, 0, 200, 2},
    {"seen150", "WATCHER", "See 150 kinds of POK~MON.", AK_SEEN, 0, 150, 0},
    {"shiny", "SHINY!", "Catch a shiny POK~MON.", AK_EVENT, 0, 1, 2},
    {"legend", "LEGENDARY", "Catch a legendary POK~MON.", AK_EVENT, 0, 1, 1},
    {"outbreak", "OUTBREAK HUNTER", "Catch the day's outbreak POK~MON.", AK_EVENT, 0, 1, 0},
    {"bigcatch", "BIG GAME", "Catch a wild POK~MON at Lv 40 or higher.", AK_EVENT, 0, 1, 0},
    {"fish50", "ANGLER", "Hook 50 POK~MON with a fishing rod.", AK_STAT, GAME_STAT_FISHING_ENCOUNTERS, 50, 0},
    {"hatch1", "NEW LIFE", "Hatch an EGG.", AK_STAT, GAME_STAT_HATCHED_EGGS, 1, 0},
    {"hatch20", "BREEDER", "Hatch 20 EGGS.", AK_STAT, GAME_STAT_HATCHED_EGGS, 20, 1},

    // ---- Around HOENN
    {"steps10k", "ON THE ROAD", "Walk 10,000 steps.", AK_STAT, GAME_STAT_STEPS, 10000, 0},
    {"steps60k", "GLOBETROTTER", "Walk 60,000 steps.", AK_STAT, GAME_STAT_STEPS, 60000, 1},
    {"contest1", "STAR OF THE STAGE", "Win a POK~MON CONTEST.", AK_STAT, GAME_STAT_WON_CONTEST, 1, 0},
    {"contest10", "CONTEST IDOL", "Win 10 POK~MON CONTESTS.", AK_STAT, GAME_STAT_WON_CONTEST, 10, 1},
    {"ribbons", "DECORATED", "Earn 10 ribbons.", AK_STAT, GAME_STAT_RECEIVED_RIBBONS, 10, 1},
    {"jackpot", "JACKPOT!", "Hit the jackpot at the GAME CORNER slots.", AK_STAT, GAME_STAT_SLOT_JACKPOTS, 1, 1},
    {"berries", "GREEN THUMB", "Plant 30 BERRIES.", AK_STAT, GAME_STAT_PLANTED_BERRIES, 30, 0},
    {"safari", "SAFARI GOER", "Visit the SAFARI ZONE 5 times.", AK_STAT, GAME_STAT_ENTERED_SAFARI_ZONE, 5, 0},
    {"cablecar", "HIGH ROAD", "Ride the cable car 10 times.", AK_STAT, GAME_STAT_RODE_CABLE_CAR, 10, 0},
    {"ledges", "PARKOUR", "Jump down 300 ledges.", AK_STAT, GAME_STAT_JUMPED_DOWN_LEDGES, 300, 0},

    // ---- Together
    {"coop1", "BETTER TOGETHER", "Win a co-op trainer battle with a friend.", AK_COUNT, 0, 1, 0},
    {"coop25", "DYNAMIC DUO", "Win 25 co-op trainer battles.", AK_COUNT, 0, 25, 1},
    {"coopgym", "TAG-TEAM GYM", "Beat a gym leader in a co-op battle.", AK_COUNT, 0, 1, 1},
    {"coopcatch", "CATCH TOGETHER", "Catch a POK~MON in a co-op wild battle.", AK_COUNT, 0, 1, 0},
    {"coopcatch20", "PACK HUNTERS", "Catch 20 POK~MON in co-op wild battles.", AK_COUNT, 0, 20, 1},
    {"rescue", "LIFESAVER", "Help a downed friend back up.", AK_COUNT, 0, 1, 1},
    {"selfrevive", "SECOND WIND", "Get back up with your own REVIVE.", AK_COUNT, 0, 1, 0},
    {"lend", "HELPING HAND", "Have friends use your field moves 10 times.", AK_COUNT, 0, 10, 1},
    {"trade", "TRADER", "Trade 5 POK~MON with friends.", AK_COUNT, 0, 5, 1},
    {"gift", "GENEROUS", "Send 30 daily gifts.", AK_COUNT, 0, 30, 1},
    {"photo3", "SAY CHEESE!", "Take a TEAM PHOTO with 3 or more players.", AK_EVENT, 0, 1, 0},
    {"fullhouse", "FULL HOUSE", "Play with four players online at once.", AK_EVENT, 0, 1, 0},
    {"pvpwin", "FRIENDLY RIVALS", "Win a battle against a friend.", AK_COUNT, 0, 1, 0},
    {"pvpwin10", "CHAMPION OF FRIENDS", "Win 10 battles against friends.", AK_COUNT, 0, 10, 1},
    {"bet", "HIGH ROLLER", "Win 5 bets while watching friends battle.", AK_COUNT, 0, 5, 0},
    {"tourwin", "TOURNAMENT CHAMP", "Win a battle tournament.", AK_COUNT, 0, 1, 2},
    {"rematch", "GYM CIRCUIT", "Win 5 gym team rematches.", AK_COUNT, 0, 5, 1},
    {"blend", "BERRY BLENDERS", "Blend BERRIES with friends 5 times.", AK_COUNT, 0, 5, 0},
    {"base", "HOUSE CALL", "Visit a friend's SECRET BASE.", AK_COUNT, 0, 1, 0},
    {"card", "PEN PALS", "Swap TRAINER CARDS with a friend.", AK_COUNT, 0, 1, 0},
    {"streak7", "A WEEK TOGETHER", "Keep a play-together streak for 7 days.", AK_STREAK, 0, 7, 1},
    {"streak30", "BEST FRIENDS", "Keep a play-together streak for 30 days.", AK_STREAK, 0, 30, 2},
    {"goals10", "GOAL GETTERS", "Finish all three daily goals on 10 days.", AK_COUNT, 0, 10, 1},
    {"racewin", "PHOTO FINISH", "Win a weekly race against friends.", AK_COUNT, 0, 1, 1},
};

#define NUM_ACH ARRAY_COUNT(sAch)

static void UnlockKey(char *out, const struct Achievement *a)
{
    snprintf(out, 48, "av:%s", a->id);
}

static void CountKey(char *out, const char *name)
{
    snprintf(out, 48, "acnt:%s", name);
}

static u8 TopLevel(void)
{
    u8 i, top = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            continue;
        if (GetMonData(&gPlayerParty[i], MON_DATA_LEVEL) > top)
            top = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
    }
    return top;
}

// Progress toward an achievement (for the menu); unlocked ones read as done
static s32 Progress(const struct Achievement *a)
{
    char key[48];
    switch (a->kind)
    {
    case AK_FLAG:   return FlagGet(a->arg) ? 1 : 0;
    case AK_STAT:   return GetGameStat(a->arg);
    case AK_CAUGHT: return GetNationalPokedexCount(FLAG_GET_CAUGHT);
    case AK_SEEN:   return GetNationalPokedexCount(FLAG_GET_SEEN);
    case AK_LEVEL:  return TopLevel();
    case AK_STREAK: return MP_Events_StreakDays();
    case AK_COUNT:
        // Tiers of one counter share it ("coop1" and "coop25" both count "coop")
        {
            char name[24];
            int n = 0;
            while (a->id[n] && (a->id[n] < '0' || a->id[n] > '9') && n < 23)
            {
                name[n] = a->id[n];
                n++;
            }
            name[n] = 0;
            CountKey(key, name);
            return MP_SaveData_Get(key);
        }
    }
    return 0;
}

bool8 MP_Achieve_IsDone(u16 i)
{
    char key[48];
    if (i >= NUM_ACH)
        return FALSE;
    UnlockKey(key, &sAch[i]);
    return MP_SaveData_Get(key) != 0;
}

static void Grant(u16 i)
{
    char key[48], msg[96];
    const struct Achievement *a = &sAch[i];
    UnlockKey(key, a);
    if (MP_SaveData_Get(key))
        return;
    MP_SaveData_Set(key, 1);
    MP_Gatcha_AddPulls(AchPulls(a->tier));
    snprintf(msg, sizeof(msg), "Achievement: %s!\n+%d MOEGATCHA pulls", a->name, AchPulls(a->tier));
    MP_Events_QueueToast(msg);
}

// A counted achievement: name like "coop", "trade", "lend"...
void MP_Achieve_Count(const char *name, s32 n)
{
    char key[48];
    CountKey(key, name);
    MP_SaveData_Add(key, n);
}

// A feat (AK_EVENT achievements: "flawless", "shiny"...)
void MP_Achieve_Unlock(const char *id)
{
    u16 i;
    for (i = 0; i < NUM_ACH; i++)
        if (strcmp(sAch[i].id, id) == 0)
            Grant(i);
}

static void TopUpOldRewards(void);

// Every frame on the field: a few achievements checked each frame
void MP_Achieve_Update(void)
{
    static u16 sNext;
    u8 k;
    if (!MP_IsInOverworld())
        return;
    TopUpOldRewards();
    if (FlagGet(FLAG_DEFEATED_REGIROCK) && FlagGet(FLAG_DEFEATED_REGICE) && FlagGet(FLAG_DEFEATED_REGISTEEL))
        MP_Achieve_Unlock("regis");
    if (MP_ConnectedCount() >= 4)
        MP_Achieve_Unlock("fullhouse");
    for (k = 0; k < 4; k++, sNext = (sNext + 1) % NUM_ACH)
    {
        const struct Achievement *a = &sAch[sNext];
        if (a->kind == AK_EVENT || MP_Achieve_IsDone(sNext))
            continue;
        if (Progress(a) >= a->target)
            Grant(sNext);
    }
}

// ---- Feats noticed at the end of battles (battle_setup.c, mp_events.c)

static bool8 IsGymLeader(u16 trainerId)
{
    return gTrainers[trainerId].trainerClass == TRAINER_CLASS_LEADER;
}

// A solo trainer battle was won (gEnemyParty and gPlayerParty as they ended)
void MP_Achieve_OnTrainerWin(u16 trainerId)
{
    u8 i, fainted = 0, count = 0, top = 0, ace = 0;
    if (gMpCoopBattle || !IsGymLeader(trainerId))
        return;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        count++;
        if (GetMonData(mon, MON_DATA_HP) == 0)
            fainted++;
        if (GetMonData(mon, MON_DATA_LEVEL) > top)
            top = GetMonData(mon, MON_DATA_LEVEL);
    }
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(&gEnemyParty[i], MON_DATA_LEVEL) > ace)
            ace = GetMonData(&gEnemyParty[i], MON_DATA_LEVEL);
    if (fainted == 0)
        MP_Achieve_Unlock("flawless");
    if (top < ace)
        MP_Achieve_Unlock("underdog");
    if (count == 1)
        MP_Achieve_Unlock("solo");
}

// A wild POKéMON was caught (by us)
void MP_Achieve_OnCatch(struct Pokemon *mon, bool8 outbreak)
{
    static const u16 sLegends[] = {
        SPECIES_ARTICUNO, SPECIES_ZAPDOS, SPECIES_MOLTRES, SPECIES_MEWTWO, SPECIES_MEW, SPECIES_RAIKOU, SPECIES_ENTEI,
        SPECIES_SUICUNE, SPECIES_LUGIA, SPECIES_HO_OH, SPECIES_CELEBI, SPECIES_REGIROCK, SPECIES_REGICE, SPECIES_REGISTEEL,
        SPECIES_LATIAS, SPECIES_LATIOS, SPECIES_KYOGRE, SPECIES_GROUDON, SPECIES_RAYQUAZA, SPECIES_JIRACHI, SPECIES_DEOXYS,
    };
    u16 species = GetMonData(mon, MON_DATA_SPECIES);
    u8 i;
    if (IsMonShiny(mon))
        MP_Achieve_Unlock("shiny");
    for (i = 0; i < ARRAY_COUNT(sLegends); i++)
        if (species == sLegends[i])
            MP_Achieve_Unlock("legend");
    if (GetMonData(mon, MON_DATA_LEVEL) >= 40)
        MP_Achieve_Unlock("bigcatch");
    if (outbreak)
        MP_Achieve_Unlock("outbreak");
}

// ---- For the menu

u16 MP_Achieve_Total(void)
{
    return NUM_ACH;
}

u16 MP_Achieve_DoneCount(void)
{
    u16 i, n = 0;
    for (i = 0; i < NUM_ACH; i++)
        n += MP_Achieve_IsDone(i);
    return n;
}

// name, description, "12/25" or "DONE", reward ("+10 MOEGATCHA pulls")
void MP_Achieve_Info(u16 i, const char **name, const char **desc, char *status, int statusLen, char *reward, int rewardLen)
{
    const struct Achievement *a = &sAch[i];
    *name = a->name;
    *desc = a->desc;
    if (MP_Achieve_IsDone(i))
        snprintf(status, statusLen, "DONE");
    else if (a->target > 1)
    {
        s32 p = Progress(a);
        snprintf(status, statusLen, "%d/%d", (int)(p < a->target ? p : a->target), a->target);
    }
    else
        snprintf(status, statusLen, "-");
    snprintf(reward, rewardLen, "+%d MOEGATCHA pulls", AchPulls(a->tier));
}

// Achievements unlocked before the pull rewards went up (they gave $1500, 1 or 2
// pulls) get the difference once, with one toast
static void TopUpOldRewards(void)
{
    static const u8 sOldPulls[] = {0, 1, 2};
    u16 i;
    s32 extra = 0;
    char msg[96];
    if (MP_SaveData_Get("ach:rewards2"))
        return;
    MP_SaveData_Set("ach:rewards2", 1);
    for (i = 0; i < NUM_ACH; i++)
        if (MP_Achieve_IsDone(i))
            extra += AchPulls(sAch[i].tier) - sOldPulls[sAch[i].tier];
    if (extra <= 0)
        return;
    MP_Gatcha_AddPulls(extra);
    snprintf(msg, sizeof(msg), "Achievement rewards went up!\n+%d MOEGATCHA pulls", (int)extra);
    MP_Events_QueueToast(msg);
}

// RARE CANDIES can't raise a POKéMON past the next gym leader's ace (party_menu.c)
u8 MP_RareCandyCap(void)
{
    static const u8 sCap[9] = {15, 20, 27, 33, 36, 41, 47, 51, MAX_LEVEL};
    u8 i, badges = 0;
    for (i = 0; i < 8; i++)
        badges += FlagGet(FLAG_BADGE01_GET + i) ? 1 : 0;
    return sCap[badges];
}
