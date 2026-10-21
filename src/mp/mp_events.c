// Moemon Co-op: playing together every day. CO-OP menu > DAILY.
//
//   * Team goals: three new goals every day for everyone on the server. The
//     targets grow with the number of players who've been online today, and
//     with an odd number of players no goal needs pairs, so nobody's left out.
//     Each finished goal rewards everyone; all three give a MoeGatcha pull.
//     If the day ends with goals left, everyone who played gets a
//     consolation prize picked for what they're short of.
//   * Play-together streak: days in a row you played at least 10 minutes with
//     a friend online. Rewards grow with the streak (shown on the TRAINER
//     CARD too).
//   * Daily outbreak: one Moemon shows up a lot on one route today, for
//     everyone on the server. More players online, more often.
//   * Shared achievements: team totals (Pokemon caught, trainers beaten...).
//     Every milestone gives each player a MoeGatcha pull.
//   * Weekly race: one challenge a week with a leaderboard. The winner gets a
//     prize, everyone else who took part a consolation prize.
//
// Days and weeks use UTC so friends in different countries share them. The
// numbers live in the server's room storage (counters, see mp_store.c);
// rewards already received are remembered next to the save (mp_savedata.c).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "mp_net.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "script.h"
#include "money.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "constants/battle.h"
#include "constants/pokemon.h"
#include "battle_main.h"
#include "battle.h"
#include "battle_setup.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
const char *MP_Lobby_RoomName(void);
s32 MP_Gatcha_Pulls(void);
void MP_Gatcha_AddPulls(s32 n);
void MP_Gatcha_Open(void);

#define TOGETHER_SECONDS (10 * 60)
#define SYNC_FRAMES      (60 * 60 * 5)
#define CHECK_FRAMES     60

// ---------------------------------------------------------------------------
// Toasts one after another

#define TOAST_QUEUE 6
static char sToasts[TOAST_QUEUE][96];
static u8 sToastHead, sToastCount;
static u16 sToastTimer;

static void QueueToast(const char *ascii)
{
    if (sToastCount == TOAST_QUEUE)
        return;
    snprintf(sToasts[(sToastHead + sToastCount) % TOAST_QUEUE], sizeof(sToasts[0]), "%s", ascii);
    sToastCount++;
}

void MP_Events_QueueToast(const char *ascii)
{
    QueueToast(ascii);
}

static void UpdateToasts(void);

// Every frame, online or not (MP_FrameUpdate)
void MP_Events_UpdateToasts(void)
{
    UpdateToasts();
}

static void UpdateToasts(void)
{
    if (sToastTimer)
        sToastTimer--;
    if (sToastTimer == 0 && sToastCount && MP_IsInOverworld() && !ArePlayerFieldControlsLocked())
    {
        u8 text[100];
        MP_AsciiToGame(text, sToasts[sToastHead], 96);
        MP_Toast(text);
        PlaySE(SE_SUCCESS);
        sToastHead = (sToastHead + 1) % TOAST_QUEUE;
        sToastCount--;
        sToastTimer = 60 * 5;
    }
}

// ---------------------------------------------------------------------------
// Keys and helpers

static s32 TodayUTC(void)
{
    return MP_Today();
}

static s32 DateFromDayNumber(s32 day)
{
    time_t t = (time_t)(day + 10957) * 86400 + 43200;   // noon UTC that day
    struct tm *g = gmtime(&t);
    return (g->tm_year + 1900) * 10000 + (g->tm_mon + 1) * 100 + g->tm_mday;
}

static s32 YesterdayUTC(void)
{
    return DateFromDayNumber(MP_DayNumber(TodayUTC()) - 1);
}

static void DayKey(char *out, s32 date, const char *what)
{
    snprintf(out, MP_STORE_KEY_MAX + 1, "d:%d:%s", (int)date, what);
}

static void MyName(char *out)
{
    MP_GameToAscii(out, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
}

static bool8 Online(void)
{
    return MP_IsConnected() && gMpLocalId >= 0;
}

static u32 Hash(const char *s, u32 h)
{
    for (; *s; s++)
        h = (h ^ (u8)*s) * 16777619u;
    return h;
}

static bool8 GiveItemOrMoney(u16 item, u16 qty, u32 money)
{
    if (item && AddBagItem(item, qty))
        return TRUE;
    AddMoney(&gSaveBlock1Ptr->money, money);
    return FALSE;
}

static void ItemAscii(u16 item, char *out)
{
    u8 name[24];
    CopyItemName(item, name);
    MP_GameToAscii(out, name, 20);
}

// ---------------------------------------------------------------------------
// Team goals

// Three goals a day: one everyone helps with (slot 0), one for co-op battles
// (slot 1) and one about doing something together (slot 2). Alone, slots 1
// and 2 become other "everyone" goals.
enum {
    // everyone helps
    G_TRAINERS, G_CATCH, G_NEWDEX, G_OUTBREAK, G_TYPE, G_FLAWLESS, G_STRONG,
    // co-op battles
    G_COOP, G_COOPCATCH, G_COOPTRAINER, G_LEND,
    // together
    G_GIFTS, G_TRADE, G_PHOTO, G_PVP, G_SPECTATE, G_BLEND, G_BASE, G_CARD,
    G_TYPES
};
#define G_SOLO_FIRST     G_TRAINERS
#define G_SOLO_COUNT     (G_COOP - G_TRAINERS)
#define G_TEAM_FIRST     G_COOP
#define G_TEAM_COUNT     (G_GIFTS - G_COOP)
#define G_TOGETHER_FIRST G_GIFTS
#define G_TOGETHER_COUNT (G_TYPES - G_GIFTS)

struct GoalDef
{
    u8 perPlayer;   // target = perPlayer x players today
    u8 fixed;       // or a fixed target (0 = per player)
    const char *text;   // %d = target
};

static const struct GoalDef sGoalDefs[G_TYPES] = {
    [G_TRAINERS]    = {3, 0, "Beat %d trainers"},
    [G_CATCH]       = {2, 0, "Catch %d POK~MON"},
    [G_NEWDEX]      = {1, 0, "Register %d new POK~DEX entries"},
    [G_OUTBREAK]    = {1, 0, "Catch %d of today's outbreak POK~MON"},
    [G_TYPE]        = {2, 0, "Catch %d %s-type POK~MON"},     // see GoalText
    [G_FLAWLESS]    = {1, 0, "Beat %d trainers without a faint"},
    [G_STRONG]      = {1, 0, "Catch %d POK~MON at your badges' level"},
    [G_COOP]        = {1, 0, "Win co-op battles with each friend"},   // see CoopText
    [G_COOPCATCH]   = {1, 0, "Catch %d POK~MON in co-op wild battles"},
    [G_COOPTRAINER] = {1, 0, "Win %d co-op trainer battles"},
    [G_LEND]        = {1, 0, "Use a friend's field move %d times"},
    [G_GIFTS]       = {1, 0, "Send %d daily gifts"},
    [G_TRADE]       = {1, 0, "Trade POK~MON with friends (%d)"},
    [G_PHOTO]       = {0, 1, "Take a TEAM PHOTO"},
    [G_PVP]         = {1, 0, "Battle friends (%d)"},
    [G_SPECTATE]    = {0, 1, "Watch a friend battle and bet on it"},
    [G_BLEND]       = {0, 1, "Blend BERRIES with friends"},
    [G_BASE]        = {0, 1, "Visit a friend's SECRET BASE"},
    [G_CARD]        = {0, 1, "Swap TRAINER CARDS with a friend"},
};

// Today's type for G_TYPE (common ones, so it's always possible)
static const u8 sGoalTypes[] = {TYPE_WATER, TYPE_GRASS, TYPE_BUG, TYPE_FLYING, TYPE_NORMAL, TYPE_POISON};

// G_STRONG: a catch at or above this level, by your badge count
static const u8 sStrongLevel[9] = {5, 8, 13, 18, 22, 26, 30, 34, 38};

#define DAILY_PULLS_PER_WEEK 5   // "all 3 goals" pulls; after that, money

static u8 TodaysGoalType(s32 date);
static u8 BadgeCount(void);
static bool8 IsTodaysOutbreak(u16 species);

// Players online today (at least the ones online right now)
static u8 PeakPlayers(s32 date)
{
    char key[40];
    s32 peak;
    DayKey(key, date, "peak");
    peak = MP_Counter_Get(key);
    if (date == TodayUTC() && peak < MP_ConnectedCount())
        peak = MP_ConnectedCount();
    return peak < 1 ? 1 : (peak > MP_MAX_PLAYERS ? MP_MAX_PLAYERS : peak);
}

static bool8 OutbreakIsEarly(void);

// Shuffles one category with the day's hash; the first entry is the pick
static void ShufflePool(u8 *pool, u8 count, u32 *h)
{
    u8 i;
    for (i = 0; i + 1 < count; i++)
    {
        u8 j = i + (*h % (count - i)), t = pool[i];
        pool[i] = pool[j];
        pool[j] = t;
        *h = *h * 1103515245u + 12345u;
    }
}

// The day's goals, the same on every game: types[0..2] = everyone / co-op /
// together, solo[] = the "everyone" pool in the day's order (for playing alone)
static void PickGoalsFull(s32 date, u8 *types, u8 *solo)
{
    u32 h = Hash(MP_Lobby_RoomName(), (u32)date * 2654435761u);
    u8 team[G_TEAM_COUNT], together[G_TOGETHER_COUNT], i, n = 0;
    u8 pool[G_SOLO_COUNT];
    for (i = 0; i < G_SOLO_COUNT; i++)
        pool[i] = G_SOLO_FIRST + i;
    for (i = 0; i < G_TEAM_COUNT; i++)
        team[i] = G_TEAM_FIRST + i;
    for (i = 0; i < G_TOGETHER_COUNT; i++)
        together[i] = G_TOGETHER_FIRST + i;
    ShufflePool(pool, G_SOLO_COUNT, &h);
    ShufflePool(team, G_TEAM_COUNT, &h);
    ShufflePool(together, G_TOGETHER_COUNT, &h);
    // The outbreak goal only when today's outbreak is somewhere everyone can reach early
    for (i = 0; i < G_SOLO_COUNT; i++)
        if (pool[i] != G_OUTBREAK || OutbreakIsEarly())
            solo[n++] = pool[i];
    while (n < G_SOLO_COUNT)
        solo[n++] = G_CATCH;
    types[0] = solo[0];
    types[1] = team[0];
    types[2] = together[0];
}

static void PickGoals(s32 date, u8 *types)
{
    u8 solo[G_SOLO_COUNT];
    PickGoalsFull(date, types, solo);
}

// What the day's goals mean with this many players: alone, the co-op and
// together goals become the next "everyone" goals of the day
static u8 EffectiveType(s32 date, u8 slot, u8 players)
{
    u8 t[3], solo[G_SOLO_COUNT];
    PickGoalsFull(date, t, solo);
    if (players >= 2 || slot == 0)
        return t[slot];
    return solo[slot];
}

static u16 GoalTarget(u8 effType, u8 players)
{
    if (sGoalDefs[effType].fixed)
        return sGoalDefs[effType].fixed;
    return sGoalDefs[effType].perPlayer * players;
}

// ---- Co-op battle goals: partner by partner -------------------------------
// "Win 1 co-op battle with BOB and CARL": every pair of today's players has to
// team up, so with three players everyone battles with everyone and nobody is
// a third wheel. With just two players it's two wins together.

#define MAX_ROSTER 8
#define ROSTER_MINUTES 5   // minutes online today before a friend counts for pair goals

struct Roster
{
    char names[MAX_ROSTER][16];
    u8 count;
};

static void AddToRoster(struct Roster *r, const char *name)
{
    u8 i;
    if (!name[0])
        return;
    for (i = 0; i < r->count; i++)
        if (strcmp(r->names[i], name) == 0)
            return;
    if (r->count < MAX_ROSTER)
        snprintf(r->names[r->count++], sizeof(r->names[0]), "%s", name);
}

static void RosterFromCounter(const char *key, s32 value, void *arg)
{
    const char *name = strstr(key, ":h:");
    if (name && value >= ROSTER_MINUTES)
        AddToRoster((struct Roster *)arg, name + 3);
}

static void MyName(char *out);

// Everyone who's been online today (and everyone online now)
static void TodaysRoster(s32 date, struct Roster *r)
{
    char prefix[32], name[16];
    u8 i;
    memset(r, 0, sizeof(*r));
    DayKey(prefix, date, "h:");
    MP_Counter_ForEach(prefix, RosterFromCounter, r);
    (void)i;
    if (date == TodayUTC())
    {
        // Me; friends join once they've been online a few minutes today (someone
        // who popped in for a moment used to make the goal impossible)
        MyName(name);
        AddToRoster(r, name);
    }
}

static void PairKey(char *out, s32 date, const char *a, const char *b)
{
    char what[24];
    if (strcmp(a, b) > 0)
    {
        const char *t = a;
        a = b;
        b = t;
    }
    snprintf(what, sizeof(what), "p:%s+%s", a, b);
    DayKey(out, date, what);
}

static s32 WinsPerPair(u8 rosterCount)
{
    return rosterCount <= 2 ? 2 : 1;
}

static s32 PairWins(s32 date, const char *a, const char *b)
{
    char key[40];
    PairKey(key, date, a, b);
    return MP_Counter_Get(key);
}

static void CoopNumbers(s32 date, s32 *progress, s32 *target)
{
    struct Roster r;
    u8 i, j;
    s32 need;
    TodaysRoster(date, &r);
    need = WinsPerPair(r.count);
    *progress = *target = 0;
    for (i = 0; i < r.count; i++)
        for (j = i + 1; j < r.count; j++)
        {
            (*target)++;
            if (PairWins(date, r.names[i], r.names[j]) >= need)
                (*progress)++;
        }
    if (*target == 0)
        *target = 1;
}

static void CoopText(s32 date, char *out, int len)
{
    struct Roster r;
    char me[16], others[3][16];
    u8 i, n = 0;
    s32 need;
    TodaysRoster(date, &r);
    need = WinsPerPair(r.count);
    MyName(me);
    for (i = 0; i < r.count && n < 3; i++)
        if (strcmp(r.names[i], me) != 0)
            snprintf(others[n++], sizeof(others[0]), "%s", r.names[i]);
    if (n == 1)
        snprintf(out, len, "Win %d co-op battles with %s", (int)need, others[0]);
    else if (n == 2)
        snprintf(out, len, "Win a co-op battle with %s and %s", others[0], others[1]);
    else
        snprintf(out, len, "Win a co-op battle with each friend");
}

// "BOB 1/1  CARL 0/1" for the help line
static void CoopStatus(s32 date, char *out, int len)
{
    struct Roster r;
    char me[16];
    u8 i;
    int pos = 0;
    s32 need;
    TodaysRoster(date, &r);
    need = WinsPerPair(r.count);
    MyName(me);
    out[0] = 0;
    for (i = 0; i < r.count && pos < len - 1; i++)
    {
        s32 w;
        if (strcmp(r.names[i], me) == 0)
            continue;
        w = PairWins(date, me, r.names[i]);
        pos += snprintf(out + pos, len - pos, "%s%s %d/%d", pos ? "  " : "", r.names[i], (int)(w < need ? w : need), (int)need);
    }
    if (!out[0])
        snprintf(out, len, "Team up with a friend for a co-op battle.");
}

static void SoloKey(char *out, s32 date, u8 slot);

// A goal's numbers and text for everyone
static void GoalNumbers(s32 date, u8 slot, s32 *progress, s32 *target)
{
    char key[40], what[4] = {'g', '0' + slot, 0};
    u8 types[3], players = PeakPlayers(date), t;
    PickGoals(date, types);
    t = EffectiveType(date, slot, players);
    if (t == G_COOP)
    {
        CoopNumbers(date, progress, target);
        return;
    }
    if (Online())
    {
        DayKey(key, date, what);
        *progress = MP_Counter_Get(key);
    }
    else
    {
        SoloKey(key, date, slot);
        *progress = MP_SaveData_Get(key);
    }
    *target = GoalTarget(t, players);
}

static void GoalText(s32 date, u8 slot, char *out, int len)
{
    u8 types[3], players = PeakPlayers(date), t;
    PickGoals(date, types);
    t = EffectiveType(date, slot, players);
    if (t == G_COOP)
        CoopText(date, out, len);
    else if (t == G_TYPE)
    {
        char type[12];
        MP_GameToAscii(type, gTypeNames[TodaysGoalType(date)], 10);
        snprintf(out, len, sGoalDefs[t].text, GoalTarget(t, players), type);
    }
    else if (t == G_STRONG)
        snprintf(out, len, "Catch %d POK~MON at Lv %d+", GoalTarget(t, players), sStrongLevel[BadgeCount()]);
    else
        snprintf(out, len, sGoalDefs[t].text, GoalTarget(t, players));
    if (GoalTarget(t, players) == 1)
    {
        // "Beat 1 trainers" -> "Beat 1 trainer"
        static const char *const sPlural[][2] = {{"trainers", "trainer"}, {"entries", "entry"}, {"times", "time"},
                                                 {"battles", "battle"}, {"gifts", "gift"}};
        u8 k;
        for (k = 0; k < ARRAY_COUNT(sPlural); k++)
        {
            char *p = strstr(out, sPlural[k][0]);
            if (p)
                memmove(p + strlen(sPlural[k][1]), p + strlen(sPlural[k][0]), strlen(p + strlen(sPlural[k][0])) + 1),
                memcpy(p, sPlural[k][1], strlen(sPlural[k][1]));
        }
    }
}

static bool8 GoalDone(s32 date, u8 slot)
{
    s32 p, t;
    GoalNumbers(date, slot, &p, &t);
    return p >= t;
}

static u8 TodaysGoalType(s32 date)
{
    return sGoalTypes[Hash(MP_Lobby_RoomName(), (u32)date * 31u + 3) % ARRAY_COUNT(sGoalTypes)];
}

// Offline goal progress ("solo:DATE:S", next to the save)
static void SoloKey(char *out, s32 date, u8 slot)
{
    snprintf(out, 40, "solo:%d:%d", (int)date, slot);
}

static void ContributeGoals(u8 eventType)
{
    s32 date = TodayUTC();
    {
        // Helped today: the day's consolation prize needs it
        char key[32];
        snprintf(key, sizeof(key), "contrib:%d", (int)date);
        if (!MP_SaveData_Get(key))
            MP_SaveData_Set(key, 1);
    }
    u8 types[3], players = PeakPlayers(date), s;
    PickGoals(date, types);
    for (s = 0; s < 3; s++)
    {
        if (EffectiveType(date, s, players) == eventType)
        {
            char key[40], what[4] = {'g', '0' + s, 0};
            if (!Online())
            {
                // Playing alone: the day's goals are counted on this save
                SoloKey(key, date, s);
                MP_SaveData_Add(key, 1);
                continue;
            }
            DayKey(key, date, what);
            MP_Counter_Add(key, 1);
        }
    }
}

// ---------------------------------------------------------------------------
// Weekly race

enum { RACE_TRAINERS, RACE_CATCHES, RACE_BIGGEST, RACE_WILD, RACE_TYPES };

static const char *const sRaceNames[RACE_TYPES] = {
    "MOST TRAINERS BEATEN", "MOST POK~MON CAUGHT", "HIGHEST-LEVEL CATCH", "MOST WILD POK~MON DEFEATED",
};

static u8 RaceType(s32 week)
{
    return (u8)(week % RACE_TYPES);
}

static void RaceKey(char *out, s32 week, const char *name)
{
    snprintf(out, MP_STORE_KEY_MAX + 1, "w:%d:%s", (int)week, name);
}

static void RaceAdd(u8 type, s32 value)
{
    s32 week = MP_WeekNumber();
    char key[40], name[16];
    if (RaceType(week) != type)
        return;
    MyName(name);
    RaceKey(key, week, name);
    if (type == RACE_BIGGEST)
        MP_Counter_Max(key, value);
    else
        MP_Counter_Add(key, value);
}

// ---------------------------------------------------------------------------
// Achievements are personal now: mp_achieve.c. The team totals ("a:caught",
// "a:trainers"...) stay as counters for the "while you were away" note.

// ---------------------------------------------------------------------------
// Outbreak

static const u16 sOutbreakSpecies[] = {
    SPECIES_PIKACHU, SPECIES_EEVEE, SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE,
    SPECIES_CHIKORITA, SPECIES_CYNDAQUIL, SPECIES_TOTODILE, SPECIES_DRATINI, SPECIES_LARVITAR,
    SPECIES_SCYTHER, SPECIES_TOGEPI, SPECIES_MAREEP, SPECIES_TEDDIURSA, SPECIES_HOUNDOUR,
    SPECIES_SNEASEL, SPECIES_GROWLITHE, SPECIES_VULPIX, SPECIES_GASTLY, SPECIES_CHANSEY,
    SPECIES_PINECO, SPECIES_SUNKERN, SPECIES_HOPPIP, SPECIES_SNUBBULL, SPECIES_PHANPY,
};

static const u16 sOutbreakMaps[] = {
    MAP_ROUTE101, MAP_ROUTE102, MAP_ROUTE103, MAP_ROUTE104, MAP_ROUTE110, MAP_ROUTE113,
    MAP_ROUTE114, MAP_ROUTE115, MAP_ROUTE116, MAP_ROUTE117, MAP_ROUTE118, MAP_ROUTE119,
    MAP_ROUTE120, MAP_ROUTE121, MAP_ROUTE123, MAP_PETALBURG_WOODS,
};

static void TodaysOutbreak(u16 *species, u16 *map)
{
    u32 h = Hash(MP_Lobby_RoomName(), (u32)TodayUTC() * 40503u + 7);
    *species = sOutbreakSpecies[h % ARRAY_COUNT(sOutbreakSpecies)];
    *map = sOutbreakMaps[(h / 97) % ARRAY_COUNT(sOutbreakMaps)];
}

static bool8 IsTodaysOutbreak(u16 species)
{
    u16 sp, map;
    TodaysOutbreak(&sp, &map);
    return species == sp;
}

// The outbreak goal only when the outbreak is on a route everyone reaches early
static bool8 OutbreakIsEarly(void)
{
    static const u16 sEarly[] = {MAP_ROUTE101, MAP_ROUTE102, MAP_ROUTE103, MAP_ROUTE104, MAP_ROUTE110, MAP_ROUTE116, MAP_PETALBURG_WOODS};
    u16 sp, map, i;
    TodaysOutbreak(&sp, &map);
    for (i = 0; i < ARRAY_COUNT(sEarly); i++)
        if (map == sEarly[i])
            return TRUE;
    return FALSE;
}

// From wild_encounter.c: maybe swap a land encounter for the outbreak species
u16 MP_Events_MaybeOutbreak(u16 species)
{
    u16 osp, omap;
    u8 rate;
    TodaysOutbreak(&osp, &omap);
    if (gSaveBlock1Ptr->location.mapGroup != (omap >> 8) || gSaveBlock1Ptr->location.mapNum != (omap & 0xFF))
        return species;
    // 20%, +10% for every other player online (up to 50%)
    rate = 20 + 10 * (MP_ConnectedCount() > 1 ? MP_ConnectedCount() - 1 : 0);
    if (rate > 50)
        rate = 50;
    return (Random() % 100 < rate) ? osp : species;
}

static void OutbreakText(char *out, int len)
{
    u16 sp, map;
    u8 name[POKEMON_NAME_LENGTH + 1], where[32];
    char a[16], b[32];
    TodaysOutbreak(&sp, &map);
    GetSpeciesName(name, sp);
    GetMapName(where, Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF)->regionMapSectionId, 0);
    MP_GameToAscii(a, name, POKEMON_NAME_LENGTH);
    MP_GameToAscii(b, where, 30);
    snprintf(out, len, "%s on %s", a, b);
}

// ---------------------------------------------------------------------------
// Consolation prizes, picked for what the player is short of

static u8 BadgeCount(void)
{
    u8 i, n = 0;
    for (i = 0; i < 8; i++)
        n += FlagGet(FLAG_BADGE01_GET + i) ? 1 : 0;
    return n;
}

static u16 CountItems(const u16 *items, u8 count)
{
    u16 n = 0;
    u8 i;
    for (i = 0; i < count; i++)
    {
        u16 q;
        for (q = 0; q < 999 && CheckBagHasItem(items[i], q + 1); q += 1)
            ;
        n += q;
        if (n >= 99)
            break;
    }
    return n;
}

static void GiveConsolation(const char *why)
{
    static const u16 sHealing[] = {ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_MAX_POTION, ITEM_FULL_RESTORE,
                                   ITEM_FRESH_WATER, ITEM_SODA_POP, ITEM_LEMONADE, ITEM_MOOMOO_MILK, ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY};
    static const u16 sBalls[] = {ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ULTRA_BALL, ITEM_NET_BALL, ITEM_DIVE_BALL,
                                 ITEM_NEST_BALL, ITEM_REPEAT_BALL, ITEM_TIMER_BALL, ITEM_LUXURY_BALL, ITEM_PREMIER_BALL};
    u8 i, n = 0, myTop = MP_GetPartyTopLevel(), badges = BadgeCount();
    u16 sum = 0;
    char msg[96], item[24];
    u16 give = ITEM_NONE, qty = 0;
    const char *reason;
    (void)reason;

    // The group's average strongest level
    for (i = 0; i < MP_MAX_PLAYERS; i++)
    {
        if (i == gMpLocalId)
            sum += myTop, n++;
        else if (gMpPlayers[i].present && gMpPlayers[i].hasState)
            sum += gMpPlayers[i].state.topLevel, n++;
    }
    (void)myTop;
    (void)sum;
    if (CountItems(sHealing, ARRAY_COUNT(sHealing)) < 5)
    {
        if (badges >= 4)
            give = ITEM_HYPER_POTION, qty = 1;
        else
            give = ITEM_SUPER_POTION, qty = 2;
        reason = "to stock up on healing";
    }
    else if (CountItems(sBalls, ARRAY_COUNT(sBalls)) < 5)
    {
        if (badges >= 5)
            give = ITEM_ULTRA_BALL, qty = 2;
        else
            give = ITEM_GREAT_BALL, qty = 3;
        reason = "for catching";
    }
    else
        reason = "as pocket money";

    if (give && GiveItemOrMoney(give, qty, 800))
    {
        ItemAscii(give, item);
        snprintf(msg, sizeof(msg), "%s\n%s x%d", why, item, qty);
    }
    else
    {
        AddMoney(&gSaveBlock1Ptr->money, 800);
        snprintf(msg, sizeof(msg), "%s\n$800", why);
    }
    QueueToast(msg);
}

// For the tournament: a consolation prize picked for what you're short of
void MP_Events_Consolation(const char *why)
{
    GiveConsolation(why);
}

// ---------------------------------------------------------------------------
// Events from the game

static u8 FaintedInParty(void)
{
    u8 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG) && GetMonData(mon, MON_DATA_HP) == 0)
            n++;
    }
    return n;
}

void MP_Events_OnTrainerWin(void)
{
    if (!gMpCoopBattle && !(gBattleTypeFlags & BATTLE_TYPE_LINK))
        MP_Achieve_OnTrainerWin(gTrainerBattleOpponent_A);
    ContributeGoals(G_TRAINERS);
    if (!gMpCoopBattle && !(gBattleTypeFlags & BATTLE_TYPE_LINK) && FaintedInParty() == 0)
        ContributeGoals(G_FLAWLESS);
    if (!Online())
        return;
    MP_Counter_Add("a:trainers", 1);
    RaceAdd(RACE_TRAINERS, 1);
}

// A co-op battle won with this partner. Only the battle leader's game
// reports it, so it's counted once.
void MP_Events_OnCoopWin(u8 partner)
{
    char key[40], me[16], them[16];
    if (!Online() || partner >= MP_MAX_PLAYERS)
        return;
    MyName(me);
    MP_GameToAscii(them, gMpPlayers[partner].name, PLAYER_NAME_LENGTH);
    PairKey(key, TodayUTC(), me, them);
    MP_Counter_Add(key, 1);
    MP_Counter_Add("a:coop", 1);
    if (!gMpCoopWild)
        ContributeGoals(G_COOPTRAINER);
}

// A co-op Triple Battle won (the master's game): every pair of the three
// counts once, the co-op battle itself once
void MP_Events_OnCoopWinGroup(const u8 *players, u8 count)
{
    char key[40], names[3][16];
    u8 i, j;
    if (!Online() || count < 2 || count > 3)
        return;
    MyName(names[0]);
    for (i = 1; i < count; i++)
    {
        if (players[i] >= MP_MAX_PLAYERS)
            return;
        MP_GameToAscii(names[i], gMpPlayers[players[i]].name, PLAYER_NAME_LENGTH);
    }
    for (i = 0; i < count; i++)
        for (j = i + 1; j < count; j++)
        {
            PairKey(key, TodayUTC(), names[i], names[j]);
            MP_Counter_Add(key, 1);
        }
    MP_Counter_Add("a:coop", 1);
    if (!gMpCoopWild)
        ContributeGoals(G_COOPTRAINER);
}

static bool8 CatchCounts(struct Pokemon *mon, u8 effType)
{
    u16 species = mon ? GetMonData(mon, MON_DATA_SPECIES) : SPECIES_NONE;
    switch (effType)
    {
    case G_OUTBREAK: return species != SPECIES_NONE && IsTodaysOutbreak(species);
    case G_TYPE:
    {
        u8 t = TodaysGoalType(TodayUTC());
        return species != SPECIES_NONE && (gSpeciesInfo[species].types[0] == t || gSpeciesInfo[species].types[1] == t);
    }
    case G_STRONG:   return mon && GetMonData(mon, MON_DATA_LEVEL) >= sStrongLevel[BadgeCount()];
    }
    return FALSE;
}

// The end of a wild battle: mon = the wild POKéMON (caught or beaten)
void MP_Events_OnWildEnd(u8 outcome, struct Pokemon *mon)
{
    u8 level = mon ? GetMonData(mon, MON_DATA_LEVEL) : 0;
    if (outcome == B_OUTCOME_CAUGHT && mon)
        MP_Achieve_OnCatch(mon, IsTodaysOutbreak(GetMonData(mon, MON_DATA_SPECIES)));
    if (outcome == B_OUTCOME_CAUGHT)
    {
        // today's goals count offline too (on the save)
        s32 date = TodayUTC();
        u8 players = PeakPlayers(date), s;
        ContributeGoals(G_CATCH);
        for (s = 0; s < 3; s++)
        {
            u8 t = EffectiveType(date, s, players);
            if ((t == G_OUTBREAK || t == G_TYPE || t == G_STRONG) && CatchCounts(mon, t))
                ContributeGoals(t);
        }
    }
    if (!Online())
        return;
    if (outcome == B_OUTCOME_WON)
        RaceAdd(RACE_WILD, 1);
    else if (outcome == B_OUTCOME_CAUGHT)
    {
        if (gMpCoopWild)
        {
            ContributeGoals(G_COOPCATCH);
            MP_Achieve_Count("coopcatch", 1);
        }
        MP_Counter_Add("a:caught", 1);
        RaceAdd(RACE_CATCHES, 1);
        RaceAdd(RACE_BIGGEST, level);
    }
}

void MP_Events_OnTrade(void)
{
    if (Online())
        MP_Counter_Add("a:trades", 1);
}

void MP_Events_OnGiftSent(void)
{
    MP_Achieve_Count("gift", 1);
    if (!Online())
        return;
    ContributeGoals(G_GIFTS);
    MP_Counter_Add("a:gifts", 1);
}

// Things done with friends (hooks around the co-op code): today's goals and
// the achievements that count them
void MP_Events_Did(u8 what)
{
    static const struct { u8 goal; const char *ach; } sDid[MP_DID_COUNT] = {
        [MP_DID_NEWDEX]     = {G_NEWDEX, NULL},
        [MP_DID_COOPWIN]    = {0xFF, "coop"},         // a co-op trainer battle won (both players)
        [MP_DID_COOPGYM]    = {0xFF, "coopgym"},
        [MP_DID_BORROW]     = {G_LEND, NULL},          // used a friend's field move
        [MP_DID_LENT]       = {0xFF, "lend"},          // a friend used ours
        [MP_DID_TRADE]      = {G_TRADE, "trade"},      // a trade with a POKéMON in it
        [MP_DID_PHOTO]      = {G_PHOTO, NULL},
        [MP_DID_PVP]        = {G_PVP, NULL},
        [MP_DID_PVPWIN]     = {0xFF, "pvpwin"},
        [MP_DID_BET]        = {G_SPECTATE, NULL},
        [MP_DID_BETWIN]     = {0xFF, "bet"},
        [MP_DID_BLEND]      = {G_BLEND, "blend"},
        [MP_DID_BASE]       = {G_BASE, "base"},
        [MP_DID_CARD]       = {G_CARD, "card"},
        [MP_DID_RESCUE]     = {0xFF, "rescue"},
        [MP_DID_SELFREVIVE] = {0xFF, "selfrevive"},
    };
    if (what >= MP_DID_COUNT)
        return;
    if (sDid[what].ach)
        MP_Achieve_Count(sDid[what].ach, 1);
    if (sDid[what].goal != 0xFF)
        ContributeGoals(sDid[what].goal);   // offline: only "everyone" goals are on the list
}

// ---------------------------------------------------------------------------
// Checking for rewards

static EWRAM_DATA struct
{
    bool8 inRoom;
    s32 syncedDate;
    u32 syncTimer;
    u16 checkTimer;
    u16 settle;          // frames since the last sync request (answers take a moment)
    u8 lastBadges;
    bool8 badgesKnown;
    time_t lastTick;
    u8 lastConnected;
    bool8 catchupPending;    // waiting for the team counters after connecting
    u32 catchupVersion;
    u32 snapshotVersion;
    u16 minuteFrames;
} sEv = {0};

// ---------------------------------------------------------------------------
// Offline catch-up: what the team did while you were away

static const struct { const char *key; const char *what; } sCatchup[] = {
    {"a:caught", "%d POK~MON caught"},
    {"a:trainers", "%d trainers beaten"},
    {"a:coop", "%d co-op wins"},
    {"a:badges", "%d badges"},
    {"a:trades", "%d trades"},
    {"a:gifts", "%d gifts"},
};

static void SaveCatchupSnapshot(void)
{
    u8 i;
    char key[32];
    for (i = 0; i < ARRAY_COUNT(sCatchup); i++)
    {
        snprintf(key, sizeof(key), "seen:%s", sCatchup[i].key);
        MP_SaveData_Set(key, MP_Counter_Get(sCatchup[i].key) + 1);   // +1: 0 means never seen
    }
}

static void UpdateCatchup(void)
{
    if (sEv.catchupPending && MP_Counter_Version() != sEv.catchupVersion)
    {
        char msg[96], part[32];
        int n = 0, shown = 0;
        u8 i;
        sEv.catchupPending = FALSE;
        msg[0] = 0;
        for (i = 0; i < ARRAY_COUNT(sCatchup); i++)
        {
            char key[32];
            s32 before, now = MP_Counter_Get(sCatchup[i].key);
            snprintf(key, sizeof(key), "seen:%s", sCatchup[i].key);
            before = MP_SaveData_Get(key);
            if (before <= 0 || now - (before - 1) <= 0)
                continue;   // first time on this server, or nothing new
            if (shown < 2)
            {
                snprintf(part, sizeof(part), sCatchup[i].what, (int)(now - (before - 1)));
                n += snprintf(msg + n, sizeof(msg) - n, "%s%s", shown ? ", " : "", part);
            }
            shown++;
        }
        if (shown > 0)
        {
            char toast[120];
            snprintf(toast, sizeof(toast), "While you were away:\n%s%s", msg, shown > 2 ? "..." : "!");
            QueueToast(toast);
        }
        SaveCatchupSnapshot();
        sEv.snapshotVersion = MP_Counter_Version();
    }
    else if (!sEv.catchupPending && MP_Counter_Version() != sEv.snapshotVersion)
    {
        // Keep the snapshot current while online
        SaveCatchupSnapshot();
        sEv.snapshotVersion = MP_Counter_Version();
    }
}

static void RequestAll(void)
{
    char prefix[24];
    snprintf(prefix, sizeof(prefix), "d:%d:", (int)TodayUTC());
    MP_Counter_Request(prefix);
    snprintf(prefix, sizeof(prefix), "d:%d:", (int)YesterdayUTC());
    MP_Counter_Request(prefix);
    MP_Counter_Request("a:");
    snprintf(prefix, sizeof(prefix), "w:%d:", (int)MP_WeekNumber());
    MP_Counter_Request(prefix);
    snprintf(prefix, sizeof(prefix), "w:%d:", (int)MP_WeekNumber() - 1);
    MP_Counter_Request(prefix);
    sEv.settle = 0;
}

static void CheckGoals(void)
{
    s32 date = TodayUTC();
    u8 types[3], players = PeakPlayers(date), s, done = 0;
    char key[48];
    PickGoals(date, types);
    for (s = 0; s < 3; s++)
    {
        if (!GoalDone(date, s))
            continue;
        done++;
        snprintf(key, sizeof(key), Online() ? "goal:%d:%d" : "sgoal:%d:%d", (int)date, s);
        if (!MP_SaveData_Get(key))
        {
            // Everyone: balls. Co-op: healing. Together: a REVIVE.
            char msg[96], item[24];
            u32 money = 300 * players;
            u8 b = BadgeCount();
            u16 give = s == 0 ? (b >= 5 ? ITEM_ULTRA_BALL : ITEM_GREAT_BALL)
                     : s == 1 ? (b >= 4 ? ITEM_HYPER_POTION : ITEM_SUPER_POTION) : ITEM_REVIVE;
            u8 qty = s == 2 ? 1 : (s == 1 && b >= 4 ? 2 : 3);
            MP_SaveData_Set(key, 1);
            ItemAscii(give, item);
            GiveItemOrMoney(give, qty, 1000);
            AddMoney(&gSaveBlock1Ptr->money, money);
            snprintf(msg, sizeof(msg), "%s goal done!\n%s x%d, $%u", Online() ? "Team" : "Daily", item, qty, (unsigned)money);
            QueueToast(msg);
            MPNet_Log("daily: goal %d done (%s)", s, Online() ? "team" : "solo");
        }
    }
    snprintf(key, sizeof(key), Online() ? "goalall:%d" : "sgoalall:%d", (int)date);
    if (done == 3 && !MP_SaveData_Get(key))
    {
        char week[24];
        MP_SaveData_Set(key, 1);
        MP_Achieve_Count("goals", 1);
        // A MOEGATCHA pull for the first five such days of the week
        snprintf(week, sizeof(week), "dailypulls:w%d", (int)MP_WeekNumber());
        if (MP_SaveData_Get(week) < DAILY_PULLS_PER_WEEK)
        {
            MP_SaveData_Add(week, 1);
            MP_Gatcha_AddPulls(1);
            QueueToast("All 3 goals done!\n+1 MOEGATCHA pull");
        }
        else
        {
            AddMoney(&gSaveBlock1Ptr->money, 2000);
            QueueToast("All 3 goals done!\n$2000 (this week's pulls are all yours)");
        }
    }
}

static void CheckYesterday(void)
{
    s32 y = YesterdayUTC();
    char key[48], played[48];
    u8 s, done = 0;
    snprintf(played, sizeof(played), "contrib:%d", (int)y);   // only for those who helped
    snprintf(key, sizeof(key), "cons:%d", (int)y);
    if (!MP_SaveData_Get(played) || MP_SaveData_Get(key))
        return;
    for (s = 0; s < 3; s++)
        if (GoalDone(y, s))
            done++;
    MP_SaveData_Set(key, 1);
    if (done < 3)
        GiveConsolation("Consolation prize!");
}

struct RaceScan
{
    s32 week;
    s32 best;
    s32 mine;
    u8 racers;   // players who scored
    char myKey[40];
};

static void ScanRace(const char *key, s32 value, void *arg)
{
    struct RaceScan *rs = (struct RaceScan *)arg;
    if (value > 0)
        rs->racers++;
    if (value > rs->best)
        rs->best = value;
    if (strcmp(key, rs->myKey) == 0)
        rs->mine = value;
}

static void CheckLastWeek(void)
{
    struct RaceScan rs;
    char prefix[24], name[16], key[32];
    s32 week = MP_WeekNumber() - 1;
    snprintf(key, sizeof(key), "race:%d", (int)week);
    if (MP_SaveData_Get(key))
        return;
    memset(&rs, 0, sizeof(rs));
    MyName(name);
    RaceKey(rs.myKey, week, name);
    snprintf(prefix, sizeof(prefix), "w:%d:", (int)week);
    MP_Counter_ForEach(prefix, ScanRace, &rs);
    if (rs.mine <= 0)
        return;   // didn't take part (or the results haven't arrived yet)
    MP_SaveData_Set(key, 1);
    if (rs.mine >= rs.best && rs.racers >= 2)
    {
        // A real race: a PP UP, money for every racer and a MOEGATCHA pull
        u32 money = 1000 * rs.racers;
        char msg[96];
        GiveItemOrMoney(ITEM_PP_UP, 1, 3000);
        AddMoney(&gSaveBlock1Ptr->money, money);
        MP_Gatcha_AddPulls(1);
        MP_Achieve_Count("racewin", 1);
        snprintf(msg, sizeof(msg), "You won the weekly race!\nPP UP, $%u, +1 MOEGATCHA pull", (unsigned)money);
        QueueToast(msg);
    }
    else if (rs.mine >= rs.best)
    {
        AddMoney(&gSaveBlock1Ptr->money, 1000);
        QueueToast("Top of the weekly race (racing alone)!\n$1000");
    }
    else
        GiveConsolation("Race consolation prize!");
}

// Play-together streak
static void UpdateStreak(void)
{
    time_t now = time(NULL);
    s32 today = TodayUTC();
    char key[48];
    bool8 together = FALSE;
    u8 i;
    // Together = a friend on the same map (not just online somewhere)
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && gMpPlayers[i].present && gMpPlayers[i].hasState
         && gMpPlayers[i].state.mapGroup == gSaveBlock1Ptr->location.mapGroup
         && gMpPlayers[i].state.mapNum == gSaveBlock1Ptr->location.mapNum)
            together = TRUE;
    if (sEv.lastTick == 0 || now < sEv.lastTick || now - sEv.lastTick > 5)
    {
        sEv.lastTick = now;
        return;
    }
    if (now == sEv.lastTick)
        return;
    {
        s32 secs = (s32)(now - sEv.lastTick);
        sEv.lastTick = now;
        if (!together)
            return;
        snprintf(key, sizeof(key), "together:%d", (int)today);
        if (MP_SaveData_Get(key) >= TOGETHER_SECONDS)
            return;
        MP_SaveData_Add(key, secs);
        if (MP_SaveData_Get(key) < TOGETHER_SECONDS)
            return;
    }
    // Today counts!
    {
        s32 day = MP_DayNumber(today), last = MP_SaveData_Get("streak:day"), count = MP_SaveData_Get("streak:count");
        char msg[96];
        u16 item = ITEM_POKE_BALL, qty = 3;
        if (last == day)
            return;
        count = (last == day - 1) ? count + 1 : 1;
        MP_SaveData_Set("streak:day", day);
        MP_SaveData_Set("streak:count", count);
        if (count >= 14)      item = ITEM_PP_UP, qty = 1;
        else if (count >= 7)  item = ITEM_REVIVE, qty = 1;
        else if (count >= 5)  item = ITEM_ULTRA_BALL, qty = 2;
        else if (count >= 3)  item = ITEM_GREAT_BALL, qty = 5;
        else if (count >= 2)  item = ITEM_SUPER_POTION, qty = 3;
        GiveItemOrMoney(item, qty, 1000);
        {
            char it[24];
            ItemAscii(item, it);
            snprintf(msg, sizeof(msg), "Co-op streak: %d day%s!\n%s x%d", (int)count, count == 1 ? "" : "s", it, qty);
        }
        QueueToast(msg);
        if (count % 7 == 0)
        {
            MP_Gatcha_AddPulls(1);
            QueueToast("A week together!\n+1 MOEGATCHA pull");
        }
    }
}

s32 MP_Events_StreakDays(void)
{
    s32 day = MP_DayNumber(TodayUTC()), last = MP_SaveData_Get("streak:day");
    if (last == day || last == day - 1)
        return MP_SaveData_Get("streak:count");
    return 0;
}

// Every frame (from MP_FrameUpdate) while in a room
void MP_Events_Update(void)
{
    static u16 sFreeTimer;
    bool8 inRoom = Online() && MP_IsInOverworld();
    // the MoeGatcha's free pulls for today / this week (they pile up)
    if (++sFreeTimer >= 600 && MP_IsInOverworld())
    {
        sFreeTimer = 0;
        MP_Gatcha_GrantFree();
    }
    if (!Online())
    {
        // Playing alone: the day's goals still count (on the save)
        sEv.inRoom = FALSE;
        if (MP_IsInOverworld() && ++sEv.checkTimer >= CHECK_FRAMES && MP_IsLocalFree())
        {
            sEv.checkTimer = 0;
            CheckGoals();
        }
        return;
    }
    if (!inRoom && !sEv.inRoom)
        return;
    if (!sEv.inRoom || sEv.syncedDate != TodayUTC() || ++sEv.syncTimer >= SYNC_FRAMES)
    {
        char played[32];
        u16 sp, map;
        bool8 first = !sEv.inRoom;
        sEv.inRoom = TRUE;
        sEv.syncedDate = TodayUTC();
        sEv.syncTimer = 0;
        RequestAll();
        snprintf(played, sizeof(played), "played:%d", (int)TodayUTC());
        MP_SaveData_Set(played, 1);
        sEv.minuteFrames = 60 * 60 - 1;   // report our minutes right away
        if (first)
        {
            sEv.catchupPending = TRUE;
            sEv.catchupVersion = MP_Counter_Version();
        }
        if (first)
        {
            char msg[96], where[64];
            TodaysOutbreak(&sp, &map);
            OutbreakText(where, sizeof(where));
            snprintf(msg, sizeof(msg), "Outbreak today: %s", where);
            QueueToast(msg);
        }
    }
    UpdateCatchup();
    // Minutes online today (up to ROSTER_MINUTES): friends count for pair goals after a few
    if (++sEv.minuteFrames >= 60 * 60)
    {
        char name[16], who[24], key[40], mine[32];
        s32 mins;
        sEv.minuteFrames = 0;
        snprintf(mine, sizeof(mine), "mins:%d", (int)TodayUTC());
        mins = MP_SaveData_Get(mine);
        if (mins < ROSTER_MINUTES)
            MP_SaveData_Set(mine, ++mins);
        MyName(name);
        snprintf(who, sizeof(who), "h:%s", name);
        DayKey(key, TodayUTC(), who);
        MP_Counter_Max(key, mins);
    }
    if (sEv.lastConnected != MP_ConnectedCount())
    {
        char key[40];
        sEv.lastConnected = MP_ConnectedCount();
        DayKey(key, TodayUTC(), "peak");
        MP_Counter_Max(key, sEv.lastConnected);
    }
    // New badges count for the team
    {
        u8 b = BadgeCount();
        if (sEv.badgesKnown && b > sEv.lastBadges)
            MP_Counter_Add("a:badges", b - sEv.lastBadges);
        sEv.lastBadges = b;
        sEv.badgesKnown = TRUE;
    }
    UpdateStreak();
    if (sEv.settle < 0xFFFF)
        sEv.settle++;
    if (++sEv.checkTimer >= CHECK_FRAMES && sEv.settle > 60 * 3 && MP_IsLocalFree())
    {
        sEv.checkTimer = 0;
        CheckGoals();
        CheckYesterday();
        CheckLastWeek();
    }
}

// ---------------------------------------------------------------------------
// CO-OP menu > DAILY

enum { DS_HUB, DS_RACE, DS_ACHIEVEMENTS };
enum { ROW_GIFTS, ROW_GOAL1, ROW_GOAL2, ROW_GOAL3, ROW_OUTBREAK, ROW_STREAK, ROW_RACE, ROW_ACH, ROW_CLOSE, ROW_COUNT };

// Offline: just the day's goals, the outbreak and achievements
static const u8 sSoloRows[] = {ROW_GOAL1, ROW_GOAL2, ROW_GOAL3, ROW_OUTBREAK, ROW_ACH, ROW_CLOSE};

static u8 HubRowCount(void)
{
    return Online() ? ROW_COUNT : ARRAY_COUNT(sSoloRows);
}

static u8 HubRow(u8 i)
{
    return Online() ? i : sSoloRows[i < ARRAY_COUNT(sSoloRows) ? i : ARRAY_COUNT(sSoloRows) - 1];
}

static EWRAM_DATA struct
{
    u8 taskId;
    u8 screen;
    u16 cursor, scroll;
    u32 seenVersion;
} sDaily = {0};

struct RaceRow
{
    char name[16];
    s32 value;
};

static struct RaceRow sRaceRows[16];
static u8 sRaceRowCount;

static void CollectRace(const char *key, s32 value, void *arg)
{
    const char *name = strchr(key + 2, ':');
    (void)arg;
    if (!name || sRaceRowCount >= ARRAY_COUNT(sRaceRows))
        return;
    snprintf(sRaceRows[sRaceRowCount].name, sizeof(sRaceRows[0].name), "%s", name + 1);
    sRaceRows[sRaceRowCount].value = value;
    sRaceRowCount++;
}

static void DrawHub(void)
{
    s32 date = TodayUTC();
    u8 types[3], players = PeakPlayers(date);
    u16 i, first;
    char left[80], right[32];
    PickGoals(date, types);
    MpUi_Begin("DAILY");
    if (Online())
        snprintf(right, sizeof(right), "%d player%s today", players, players == 1 ? "" : "s");
    else
        snprintf(right, sizeof(right), "playing alone");
    {
        u8 r[32];
        MP_AsciiToGame(r, right, 30);
        MpUi_TitleRight(r);
    }
    first = MpUi_ScrollFor(sDaily.cursor, HubRowCount(), &sDaily.scroll);
    for (i = first; i < HubRowCount() && i < first + MPUI_ROWS; i++)
    {
        bool8 sel = i == sDaily.cursor;
        right[0] = 0;
        switch (HubRow(i))
        {
        case ROW_GIFTS:
            snprintf(left, sizeof(left), "DAILY GIFTS...");
            break;
        case ROW_GOAL1:
        case ROW_GOAL2:
        case ROW_GOAL3:
        {
            u8 s = HubRow(i) - ROW_GOAL1;
            s32 p, target;
            GoalNumbers(date, s, &p, &target);
            GoalText(date, s, left, sizeof(left));
            if (p >= target)
                snprintf(right, sizeof(right), "DONE!");
            else
                snprintf(right, sizeof(right), "%d/%d", (int)p, (int)target);
            break;
        }
        case ROW_OUTBREAK:
        {
            char where[64];
            OutbreakText(where, sizeof(where));
            snprintf(left, sizeof(left), "Outbreak: %s", where);
            break;
        }
        case ROW_STREAK:
        {
            char key[32];
            s32 secs;
            snprintf(key, sizeof(key), "together:%d", (int)date);
            secs = MP_SaveData_Get(key);
            snprintf(left, sizeof(left), "Play-together streak: %d day%s", (int)MP_Events_StreakDays(), MP_Events_StreakDays() == 1 ? "" : "s");
            if (secs < TOGETHER_SECONDS)
                snprintf(right, sizeof(right), "%d min to go", (int)((TOGETHER_SECONDS - secs + 59) / 60));
            else
                snprintf(right, sizeof(right), "today: yes");
            break;
        }
        case ROW_RACE:
            snprintf(left, sizeof(left), "WEEKLY RACE...");
            snprintf(right, sizeof(right), "%s", sRaceNames[RaceType(MP_WeekNumber())]);
            break;
        case ROW_ACH:
            snprintf(left, sizeof(left), "ACHIEVEMENTS...");
            break;
        default:
            snprintf(left, sizeof(left), "CLOSE");
            break;
        }
        MpUi_RowAscii(i - first, sel, left, right[0] ? right : NULL);
    }
    MpUi_End();
    if (sDaily.cursor >= ROW_GOAL1 && sDaily.cursor <= ROW_GOAL3)
    {
        u8 eff = EffectiveType(date, sDaily.cursor - ROW_GOAL1, players);
        if (eff == G_COOP)
        {
            char status[96];
            CoopStatus(date, status, sizeof(status));
            MpUi_HelpAscii(status);
        }
        else if (sDaily.cursor == ROW_GOAL1 || players < 2)
            MpUi_HelpAscii("Everyone on the server helps. Reward: POKE BALLS + money.");
        else if (sDaily.cursor == ROW_GOAL2)
            MpUi_HelpAscii("Team up with friends for this one. Reward: healing + money.");
        else
            MpUi_HelpAscii("Do this together with friends. Reward: a REVIVE + money.");
    }
    else if (sDaily.cursor == ROW_OUTBREAK)
        MpUi_HelpAscii("It appears more often with more players online.");
    else if (sDaily.cursor == ROW_STREAK)
        MpUi_HelpAscii("Play 10 minutes on the same map as a friend each day.");
    else
        MpUi_HelpAscii("{A} SELECT  {B} CLOSE");
}

static void DrawRace(void)
{
    s32 week = MP_WeekNumber();
    char prefix[24], line[64];
    u8 i, j;
    snprintf(prefix, sizeof(prefix), "w:%d:", (int)week);
    sRaceRowCount = 0;
    MP_Counter_ForEach(prefix, CollectRace, NULL);
    // Best first
    for (i = 0; i < sRaceRowCount; i++)
        for (j = i + 1; j < sRaceRowCount; j++)
            if (sRaceRows[j].value > sRaceRows[i].value)
            {
                struct RaceRow t = sRaceRows[i];
                sRaceRows[i] = sRaceRows[j];
                sRaceRows[j] = t;
            }
    snprintf(line, sizeof(line), "WEEKLY RACE: %s", sRaceNames[RaceType(week)]);
    MpUi_Begin(line);
    for (i = 0; i < sRaceRowCount && i < MPUI_ROWS - 1; i++)
    {
        char l[32], r[16];
        snprintf(l, sizeof(l), "%d. %s", i + 1, sRaceRows[i].name);
        snprintf(r, sizeof(r), RaceType(week) == RACE_BIGGEST ? "Lv%d" : "%d", (int)sRaceRows[i].value);
        MpUi_RowAscii(i, FALSE, l, r);
    }
    if (sRaceRowCount == 0)
        MpUi_TextAscii(1, "Nobody has scored yet this week.");
    {
        s32 dayInWeek = (MP_DayNumber(TodayUTC()) - 2) % 7;
        snprintf(line, sizeof(line), "Ends in %d day%s.", (int)(7 - dayInWeek), 7 - dayInWeek == 1 ? "" : "s");
        MpUi_TextAscii(MPUI_ROWS - 1, line);
    }
    MpUi_End();
    MpUi_HelpAscii("Winner (2+ racing): PP UP, money and a MOEGATCHA pull.  {B} BACK");
}

static void DrawAchievements(void)
{
    u16 i, first, count = MP_Achieve_Total();
    char title[32];
    u8 right[32];
    MpUi_Begin("ACHIEVEMENTS");
    snprintf(title, sizeof(title), "%d/%d", MP_Achieve_DoneCount(), count);
    MP_AsciiToGame(right, title, 30);
    MpUi_TitleRight(right);
    first = MpUi_ScrollFor(sDaily.cursor, count, &sDaily.scroll);
    for (i = first; i < count && i < first + MPUI_ROWS; i++)
    {
        const char *name, *desc;
        char status[16], reward[32];
        MP_Achieve_Info(i, &name, &desc, status, sizeof(status), reward, sizeof(reward));
        MpUi_RowAscii(i - first, i == sDaily.cursor, name, status);
    }
    MpUi_End();
    {
        const char *name, *desc;
        char status[16], reward[32], help[160];
        MP_Achieve_Info(sDaily.cursor, &name, &desc, status, sizeof(status), reward, sizeof(reward));
        snprintf(help, sizeof(help), "%s  Reward: %s", desc, reward);
        MpUi_HelpAscii(help);
    }
}

static void Draw(void)
{
    sDaily.seenVersion = MP_Counter_Version();
    switch (sDaily.screen)
    {
    case DS_HUB:          DrawHub(); break;
    case DS_RACE:         DrawRace(); break;
    case DS_ACHIEVEMENTS: DrawAchievements(); break;
    }
}

static void CloseDaily(void)
{
    MpUi_Close();
    DestroyTask(sDaily.taskId);
    sDaily.taskId = TASK_NONE;
}

static void Task_Daily(u8 taskId)
{
    if (sDaily.screen == DS_HUB)
    {
        if (MpUi_MoveCursor(&sDaily.cursor, HubRowCount()))
        {
            Draw();
            return;
        }
        if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && HubRow(sDaily.cursor) == ROW_CLOSE))
        {
            PlaySE(SE_SELECT);
            CloseDaily();
            return;
        }
        if (JOY_NEW(A_BUTTON))
        {
            switch (HubRow(sDaily.cursor))
            {
            case ROW_GIFTS:
                PlaySE(SE_SELECT);
                CloseDaily();
                MP_Gift_OpenMenu();
                return;
            case ROW_RACE:
                PlaySE(SE_SELECT);
                sDaily.screen = DS_RACE;
                break;
            case ROW_ACH:
                PlaySE(SE_SELECT);
                sDaily.screen = DS_ACHIEVEMENTS;
                sDaily.cursor = sDaily.scroll = 0;
                break;
            }
            Draw();
            return;
        }
    }
    else
    {
        if (sDaily.screen == DS_ACHIEVEMENTS && MpUi_MoveCursor(&sDaily.cursor, MP_Achieve_Total()))
        {
            Draw();
            return;
        }
        if (JOY_NEW(A_BUTTON | B_BUTTON))
        {
            PlaySE(SE_SELECT);
            {
                u8 want = sDaily.screen == DS_RACE ? ROW_RACE : ROW_ACH, k;
                sDaily.cursor = 0;
                for (k = 0; k < HubRowCount(); k++)
                    if (HubRow(k) == want)
                        sDaily.cursor = k;
            }
            sDaily.scroll = 0;
            sDaily.screen = DS_HUB;
            Draw();
            return;
        }
    }
    if (sDaily.seenVersion != MP_Counter_Version())
        Draw();
}

void MP_Daily_OpenMenu(void)
{
    memset(&sDaily, 0, sizeof(sDaily));
    if (Online())
        RequestAll();   // offline: the goals are on the save
    MpUi_Open();
    sDaily.taskId = CreateTask(Task_Daily, 80);
    Draw();
}

// Test harness: "mp goals" prints the goals for 1-4 players over a week
void MP_Events_DebugGoals(void)
{
    s32 day0 = MP_DayNumber(TodayUTC());
    u8 d, p, s, types[3];
    for (d = 0; d < 7; d++)
    {
        s32 date = DateFromDayNumber(day0 + d);
        PickGoals(date, types);
        for (p = 1; p <= 4; p++)
        {
            printf("[test] %d players=%d:", (int)date, p);
            for (s = 0; s < 3; s++)
            {
                u8 t = EffectiveType(date, s, p);
                printf(" [");
                if (t == G_COOP)
                    printf("co-op with each pair (%d each)", (int)WinsPerPair(p));
                else
                    printf(sGoalDefs[t].text, GoalTarget(t, p), "TYPE");
                printf("]");
            }
            printf("\n");
        }
    }
}
