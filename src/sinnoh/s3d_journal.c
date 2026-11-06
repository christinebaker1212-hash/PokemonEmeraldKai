// Moemon Kai: SINNOH's JOURNAL (Platinum's src/journal.c and applications/journal_display, at the pinned commit): the
// day's page of what the player did, ten pages kept, a new one each day the game is loaded. Platinum's own words
// (journal_entries, gym_names, month_names, times_of_day: sinnoh/platinum_text.pak by name, tools/sinnoh/tv_data.py),
// its notebook (f_note_gra.narc: tools/sinnoh/build_journal.py, "JRNL" in sinnoh/platinum_gfx.pak), its rules for what
// is written and when:
// - a page a day: Platinum starts a new page when the game is loaded on a new date (Journal_GetSavedPage), titled with
//   the date and where the player started (sub_02053494); here also when the player comes to SINNOH from another region
//   (Platinum's game always loads there), and when the JOURNAL is given;
// - the first visit to a town (CreateFlyLocationJournalEvent: a FLY spot not yet unlocked), else on a warp the map
//   transition (CreateMapTransitionJournalEvent): out of a cave, out of the player's home / the lab / a named building
//   (sMapsInfo, "Exited" or "Departed"), into a GYM without its badge ("was too tough...");
// - wild POKéMON caught, or defeated from the fifth on (encounter.c UpdateJournal), trainers beaten (gym leaders, the
//   Elite Four and the Champion among the place's events, the others the page's trainer line);
// - a POKé MART's shopping (shop_menu.c), a PC Box used (box_app_manager.c: something moved), field moves (Cut, Fly,
//   Surf... scrcmd.c / field_move_tasks.c / start_menu.c / party_menu), item balls, the GAME CORNER, the Underground's
//   digging, the BATTLE FRONTIER (Platinum's CreateJournalEvent, here `sinnoh SINNOH_JOURNAL`);
// - friends' battles (the online lines: "Beat X (Single)").
// Saved as the region side file's chunk "JRNL" (mp_region.c). Only in SINNOH, only with the JOURNAL in the BAG.
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "bg.h"
#include "battle.h"
#include "battle_setup.h"
#include "data.h"
#include "menu.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "item.h"
#include "main.h"
#include "malloc.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "s3d.h"
#include "mp_region.h"
#include "constants/battle.h"
#include "constants/field_effects.h"
#include "constants/items.h"
#include "constants/rgb.h"
#include "constants/map_types.h"
#include "constants/region_map_sections.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/trainers_sinnoh.h"

bool8 S3D_Gfx_Journal(u8 girl, const u8 **px, const u16 **pal, u16 *w, u16 *h);   // (s3d_gfx.c)
extern void (*gMpBgPictureLine)(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy);
extern int gMpBgPictureBg;

#define PAGES 10
#define LOC_EVENTS 4
#define ONLINE_EVENTS 2
#define MAGIC 0x4C4E524A   // "JRNL"

// Platinum's LOCATION_EVENT_* (generated/journal_location_events.txt), in its order
enum
{
    LE_NONE, LE_RESTED_AT_HOME, LE_LEFT_RESEARCH_LAB, LE_USED_PC_BOX, LE_SHOPPED_AT_MART, LE_LOTS_OF_SHOPPING,
    LE_SOLD_A_LITTLE, LE_SOLD_A_LOT, LE_BUSINESS_AT_MART, LE_GYM_WAS_TOO_TOUGH, LE_BEAT_GYM_LEADER,
    LE_BEAT_ELITE_FOUR_MEMBER, LE_BEAT_CHAMPION, LE_ARRIVED_IN_LOCATION, LE_LEFT_CAVE, LE_LEFT_BUILDING,
    LE_GAME_CORNER, LE_SAFARI_GAME, LE_ITEM_WAS_OBTAINED, LE_USED_CUT, LE_FLEW_TO_LOCATION, LE_USED_SURF,
    LE_USED_STRENGTH, LE_USED_DEFOG, LE_USED_ROCK_SMASH, LE_USED_WATERFALL, LE_USED_ROCK_CLIMB, LE_USED_FLASH,
    LE_WARPED_TO_LOCATION, LE_USED_DIG, LE_LURED_POKEMON, LE_UNUSED, LE_USED_MILK_DRINK, LE_USED_SOFTBOILED,
    LE_DUG_UNDERGROUND, LE_BUILT_SECRET_BASE, LE_BATTLE_TOWER, LE_BATTLE_FACTORY, LE_BATTLE_CASTLE, LE_BATTLE_HALL,
    LE_BATTLE_ARCADE,
    LE_COUNT
};
#define JOURNAL_GIVEN 0xFF   // (`sinnoh SINNOH_JOURNAL, 255`: Platinum's GiveJournal, a page for today)

struct LocEvent
{
    u8 type, pad;
    u16 where;     // a map section, a GYM (0-7, sGyms), an item
    u16 trainer;
};

struct OnlineEvent
{
    u8 kind;       // 1 single, 2 double, 3 multi
    u8 result;     // 0 won, 1 lost, 2 tie
    u8 name[2][8];
};

struct Page
{
    u8 year, month, day, week;   // (year - 2000; month 1-12; 0: no page)
    u16 start;                   // the map section the day started in
    u16 pad;
    struct LocEvent loc[LOC_EVENTS];
    u8 monResult;                // 1 caught, 2 defeated
    u8 monVariant, monTime, monGender;
    u16 species;
    u16 trainer, trainerWhere;   // (trainer 0: none)
    struct OnlineEvent online[ONLINE_EVENTS];
};

static struct
{
    u32 magic;
    struct Page p[PAGES];
} sJ;

// (not saved: Platinum's FieldSystem keeps these for the session)
static struct
{
    u16 lastMap, arrivedMap;
    bool8 haveLast;
    u8 pendingMove;           // LE_FLEW_TO_LOCATION / LE_WARPED_TO_LOCATION: written on arriving
    u8 wildDefeated;          // wildBattleMetadata.wildMonDefeated
    u8 bought, sold;          // a POKé MART visit's (shop_menu.c itemPurchaseCount / itemSoldCount)
    bool8 boxUsed;            // recordBoxUseInJournal
} sS;

// Platinum's sGymsInfo order (BeatLocationsGymLeader / GymWasTooTough name the GYM by it: gym_names)
static const struct { u16 trainer, map; u8 badge; } sGyms[8] = {
    {TRAINER_SINNOH_LEADER_ROARK, MAP_SINNOH_OREBURGH_CITY_GYM, 0},
    {TRAINER_SINNOH_LEADER_GARDENIA, MAP_SINNOH_ETERNA_CITY_GYM, 1},
    {TRAINER_SINNOH_LEADER_WAKE, MAP_SINNOH_PASTORIA_CITY_GYM, 3},
    {TRAINER_SINNOH_LEADER_MAYLENE, MAP_SINNOH_VEILSTONE_CITY_GYM, 2},
    {TRAINER_SINNOH_LEADER_FANTINA, MAP_SINNOH_HEARTHOME_CITY_GYM_ENTRANCE_ROOM, 4},
    {TRAINER_SINNOH_LEADER_CANDICE, MAP_SINNOH_SNOWPOINT_CITY_GYM, 6},
    {TRAINER_SINNOH_LEADER_BYRON, MAP_SINNOH_CANALAVE_CITY_GYM, 5},
    {TRAINER_SINNOH_LEADER_VOLKNER, MAP_SINNOH_SUNYSHORE_CITY_GYM_ROOM1, 7},
};

// Platinum's sMapsInfo: the buildings whose leaving is written, TRUE: "Exited from", else "Departed from"
static const struct { u16 mapsec; bool8 exited; } sBuildings[] = {
    {MAPSEC_SINNOH_JUBILIFE_TV, TRUE}, {MAPSEC_SINNOH_POKETCH_CO, FALSE}, {MAPSEC_SINNOH_TRAINERS_SCHOOL, TRUE},
    {MAPSEC_SINNOH_MINING_MUSEUM, FALSE}, {MAPSEC_SINNOH_FLOWER_SHOP, TRUE}, {MAPSEC_SINNOH_CYCLE_SHOP, TRUE},
    {MAPSEC_SINNOH_CONTEST_HALL, TRUE}, {MAPSEC_SINNOH_POFFIN_HOUSE, TRUE}, {MAPSEC_SINNOH_FOREIGN_BUILDING, FALSE},
    {MAPSEC_SINNOH_POKEMON_DAY_CARE, TRUE}, {MAPSEC_SINNOH_VEILSTONE_STORE, FALSE}, {MAPSEC_SINNOH_GAME_CORNER, TRUE},
    {MAPSEC_SINNOH_GALACTIC_HQ, FALSE}, {MAPSEC_SINNOH_CANALAVE_LIBRARY, FALSE}, {MAPSEC_SINNOH_SNOWPOINT_TEMPLE, FALSE},
    {MAPSEC_SINNOH_VISTA_LIGHTHOUSE, FALSE}, {MAPSEC_SINNOH_SUNYSHORE_MARKET, TRUE}, {MAPSEC_SINNOH_BATTLE_TOWER, TRUE},
    {MAPSEC_SINNOH_POKEMON_MANSION, FALSE}, {MAPSEC_SINNOH_FOOTSTEP_HOUSE, TRUE}, {MAPSEC_SINNOH_CAFE, FALSE},
    {MAPSEC_SINNOH_GRAND_LAKE, FALSE}, {MAPSEC_SINNOH_RESTAURANT, FALSE}, {MAPSEC_SINNOH_VALLEY_WINDWORKS, FALSE},
    {MAPSEC_SINNOH_FUEGO_IRONWORKS, FALSE}, {MAPSEC_SINNOH_OLD_CHATEAU, FALSE},
};

// ---------------------------------------------------------------------------------------------------- the save
const void *Sinnoh_JournalData(u32 *size)
{
    sJ.magic = MAGIC;
    *size = sizeof(sJ);
    return &sJ;
}

void Sinnoh_JournalLoad(const void *data, u32 size)
{
    memset(&sJ, 0, sizeof(sJ));
    if (size == sizeof(sJ) && ((const u32 *)data)[0] == MAGIC)
        memcpy(&sJ, data, size);
    memset(&sS, 0, sizeof(sS));
}

void Sinnoh_JournalReset(void)
{
    memset(&sJ, 0, sizeof(sJ));
    memset(&sS, 0, sizeof(sS));
}

// ---------------------------------------------------------------------------------------------------- the rules
static u16 CurMap(void)
{
    return gSaveBlock1Ptr->location.mapNum | gSaveBlock1Ptr->location.mapGroup << 8;
}

static bool8 InSinnoh(void)
{
    return S3D_IsSinnohGroup(gSaveBlock1Ptr->location.mapGroup);
}

static bool8 Active(void)
{
    return InSinnoh() && CheckBagHasItem(ITEM_JOURNAL, 1);
}

static void Today(u8 *year, u8 *month, u8 *day, u8 *week)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    *year = t ? (u8)((t->tm_year + 1900 - 2000) & 0x7F) : 0;
    *month = t ? t->tm_mon + 1 : 1;
    *day = t ? t->tm_mday : 1;
    *week = t ? t->tm_wday : 0;
}

static bool8 IsToday(const struct Page *p)
{
    u8 y, m, d, w;
    Today(&y, &m, &d, &w);
    return p->year == y && p->month == m && p->day == d && p->week == w;
}

// Journal_GetSavedPage: a page from another day moves down, today's starts empty
static void NewDay(void)
{
    u32 i;
    if (sJ.p[0].month != 0 && !IsToday(&sJ.p[0]))
    {
        for (i = PAGES - 1; i >= 1; i--)
            sJ.p[i] = sJ.p[i - 1];
        memset(&sJ.p[0], 0, sizeof(sJ.p[0]));
    }
}

// JournalEntry_SaveTitle: today's date and where the player is (only once a day)
static void Title(void)
{
    struct Page *p = &sJ.p[0];
    if (p->month != 0 && IsToday(p))
        return;
    Today(&p->year, &p->month, &p->day, &p->week);
    p->start = gMapHeader.regionMapSectionId;
}

static struct Page *Cur(void)
{
    if (sJ.p[0].month == 0)
        Title();
    return &sJ.p[0];
}

// The game was loaded (or the player came to SINNOH, or got the JOURNAL): Platinum's load-time page and title
void Sinnoh_JournalLoaded(void)
{
    if (!Active())
        return;
    NewDay();
    Title();
}

static u32 DayNumber(u8 year, u8 month, u8 day)
{
    struct tm t = {0};
    t.tm_year = year + 100;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = 12;
    return (u32)(mktime(&t) / 86400);
}

// Journal_CheckOpenOnContinue: the game loaded two days or more after the last page (or in another year)
bool8 Sinnoh_JournalOpenOnContinue(void)
{
    const struct Page *p = &sJ.p[0];
    u8 y, m, d, w;
    s32 diff;
    if (!Active() || p->month == 0)
        return FALSE;
    Today(&y, &m, &d, &w);
    diff = (s32)DayNumber(y, m, d) - (s32)DayNumber(p->year, p->month, p->day);
    if ((m == 12 && d == 31 && p->month == 1 && p->day == 1) || (m == 1 && d == 1 && p->month == 12 && p->day == 31))
        return (s32)y - (s32)p->year >= 2 || (s32)y - (s32)p->year <= -2;
    if (diff <= -2 || diff >= 2)
        return TRUE;
    return y != p->year;
}

// JournalEntry_GetEmptyLocationEvent: the first free line, else everything moves up a line
static struct LocEvent *FreeLoc(struct LocEvent *e)
{
    u32 i;
    for (i = 0; i < LOC_EVENTS; i++)
        if (e[i].type == LE_NONE)
            return &e[i];
    for (i = 0; i < LOC_EVENTS - 1; i++)
        e[i] = e[i + 1];
    memset(&e[i], 0, sizeof(e[i]));
    return &e[i];
}

static const struct LocEvent *LastLoc(const struct LocEvent *e)
{
    u32 i;
    for (i = 0; i < LOC_EVENTS && e[i].type != LE_NONE; i++)
        ;
    return i ? &e[i - 1] : NULL;
}

static void RemoveLoc(struct LocEvent *e, u32 i)
{
    for (; i < LOC_EVENTS - 1; i++)
        e[i] = e[i + 1];
    memset(&e[LOC_EVENTS - 1], 0, sizeof(e[0]));
}

// JournalEntry_SaveLocationEvent and its kinds
static void AddLoc(u8 type, u16 where, u16 trainer)
{
    struct LocEvent *e, *f;
    const struct LocEvent *last;
    u32 i;
    if (!Active() || type == LE_NONE || type >= LE_COUNT)
        return;
    e = Cur()->loc;
    last = LastLoc(e);
    switch (type)
    {
    case LE_GYM_WAS_TOO_TOUGH:
    case LE_BEAT_GYM_LEADER:
        for (i = 0; i < LOC_EVENTS; i++)   // (beating the GYM takes its "too tough" away)
            if (e[i].type == LE_GYM_WAS_TOO_TOUGH && e[i].where == where)
            {
                RemoveLoc(e, i);
                break;
            }
        break;
    case LE_BEAT_ELITE_FOUR_MEMBER:
        for (i = 0; i < LOC_EVENTS; i++)   // (only the last of the Elite Four stays)
            if (e[i].type == LE_BEAT_ELITE_FOUR_MEMBER)
            {
                RemoveLoc(e, i);
                break;
            }
        break;
    case LE_BEAT_CHAMPION:
        break;
    case LE_ARRIVED_IN_LOCATION:
    case LE_LEFT_CAVE:
    case LE_LEFT_BUILDING:
    case LE_ITEM_WAS_OBTAINED:
    case LE_USED_CUT ... LE_USED_SOFTBOILED:
        if (last && last->type == type && last->where == where)   // (the same again: once)
            return;
        trainer = 0;
        break;
    default:   // (the misc ones: once in a row)
        if (last && last->type == type)
            return;
        where = trainer = 0;
        break;
    }
    f = FreeLoc(e);
    f->type = type;
    f->where = where;
    f->trainer = trainer;
}

static u8 GymOfTrainer(u16 trainer)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sGyms); i++)
        if (sGyms[i].trainer == trainer)
            return i;
    return 0xFF;
}

// JournalEntry_GetGymTooTough: a GYM entered without its badge
static u8 GymTooTough(u16 map)
{
    u8 i;
    for (i = 0; i < ARRAY_COUNT(sGyms); i++)
        if (sGyms[i].map == map)
            return FlagGet(MP_Region_BadgeFlag(REGION_SINNOH, sGyms[i].badge)) ? 0xFF : i;
    return 0xFF;
}

static const struct MapHeader *Header(u16 map)
{
    return Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF);
}

static bool8 IsCave(const struct MapHeader *h)
{
    return h->mapType == MAP_TYPE_UNDERGROUND;
}

static bool8 IsBuilding(const struct MapHeader *h)
{
    return h->mapType == MAP_TYPE_INDOOR || h->mapType == MAP_TYPE_SECRET_BASE;
}

static bool8 IsOutdoors(const struct MapHeader *h)
{
    return h->mapType == MAP_TYPE_TOWN || h->mapType == MAP_TYPE_CITY || h->mapType == MAP_TYPE_ROUTE
        || h->mapType == MAP_TYPE_OCEAN_ROUTE;
}

static s32 BuildingIndex(u16 mapsec)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sBuildings); i++)
        if (sBuildings[i].mapsec == mapsec)
            return i;
    return -1;
}

// JournalEntry_CreateAndSaveEventMapTransition
static void MapTransition(u16 prev, u16 cur)
{
    const struct MapHeader *ph = Header(prev), *ch = Header(cur);
    if (ph == NULL || ch == NULL)
        return;
    if (IsCave(ph))
    {
        if (IsOutdoors(ch))
            AddLoc(LE_LEFT_CAVE, ph->regionMapSectionId, 0);
    }
    else if (IsBuilding(ph))
    {
        if (IsOutdoors(ch))
        {
            if (prev == MAP_SINNOH_TWINLEAF_TOWN_PLAYER_HOUSE1F)
                AddLoc(LE_RESTED_AT_HOME, 0, 0);
            else if (prev == MAP_SINNOH_SANDGEM_TOWN_POKEMON_RESEARCH_LAB)
                AddLoc(LE_LEFT_RESEARCH_LAB, 0, 0);
            else if (BuildingIndex(ph->regionMapSectionId) >= 0)
                AddLoc(LE_LEFT_BUILDING, ph->regionMapSectionId, 0);
        }
    }
    else if (IsOutdoors(ph) && IsBuilding(ch))
    {
        u8 gym = GymTooTough(cur);
        if (gym != 0xFF)
            AddLoc(LE_GYM_WAS_TOO_TOUGH, gym, 0);
    }
}

// (s3d_townmap.c Sinnoh_TownMapArrive: a FLY spot unlocked by arriving) CreateFlyLocationJournalEvent
void Sinnoh_JournalArrived(u16 mapsec)
{
    if (!Active() || !IsOutdoors(&gMapHeader))   // (Platinum's FLY spots are the towns' own maps, not their houses)
        return;
    AddLoc(LE_ARRIVED_IN_LOCATION, mapsec, 0);
    sS.arrivedMap = CurMap();
}

// A new map (overworld.c, WARP: by a warp, else walking over a map's edge): Platinum's map change journal events
void Sinnoh_JournalMapChanged(bool8 warp)
{
    u16 cur = CurMap(), prev = sS.lastMap;
    bool8 had = sS.haveLast;
    void Sinnoh_TownMapArrive(void);
    if (!InSinnoh())
    {
        sS.haveLast = FALSE;
        return;
    }
    sS.lastMap = cur;
    sS.haveLast = TRUE;
    if (!had)   // (come to SINNOH from another region, or the game loaded: Platinum's load-time page)
    {
        Sinnoh_JournalLoaded();
        return;
    }
    if (prev == cur)
        return;
    if (!warp)
        Sinnoh_TownMapArrive();   // (walking into a town unlocks its FLY spot too)
    if (sS.pendingMove)
    {
        AddLoc(sS.pendingMove, gMapHeader.regionMapSectionId, 0);
        sS.pendingMove = 0;
        return;
    }
    if (sS.arrivedMap == cur)
        return;
    if (warp)
        MapTransition(prev, cur);
}

// (field_effect.c FieldEffectStart) the field moves that show as field effects
void Sinnoh_JournalFieldEffect(u8 id)
{
    u16 here = gMapHeader.regionMapSectionId;
    if (!Active())
        return;
    switch (id)
    {
    case FLDEFF_USE_CUT_ON_GRASS:
    case FLDEFF_USE_CUT_ON_TREE: AddLoc(LE_USED_CUT, here, 0); break;
    case FLDEFF_USE_SURF: AddLoc(LE_USED_SURF, here, 0); break;
    case FLDEFF_USE_ROCK_SMASH: AddLoc(LE_USED_ROCK_SMASH, here, 0); break;
    case FLDEFF_USE_STRENGTH: AddLoc(LE_USED_STRENGTH, here, 0); break;
    case FLDEFF_USE_WATERFALL: AddLoc(LE_USED_WATERFALL, here, 0); break;
    case FLDEFF_USE_DIG: AddLoc(LE_USED_DIG, here, 0); break;
    case FLDEFF_SWEET_SCENT: AddLoc(LE_LURED_POKEMON, here, 0); break;
    case FLDEFF_USE_FLY: sS.pendingMove = LE_FLEW_TO_LOCATION; break;     // (start_menu.c: the place flown to)
    case FLDEFF_USE_TELEPORT: sS.pendingMove = LE_WARPED_TO_LOCATION; break;   // (field_warp.c: where it led)
    }
}

// (party_menu.c, a field move chosen: 1 FLASH, 2 MILK DRINK, 3 SOFTBOILED) the ones that aren't field effects;
// (s3d_misc.c) ROCK CLIMB, DEFOG
void Sinnoh_JournalFieldMove(u8 which)
{
    static const u8 sTypes[4] = {LE_NONE, LE_USED_FLASH, LE_USED_MILK_DRINK, LE_USED_SOFTBOILED};
    if (which < 4)
        AddLoc(sTypes[which], gMapHeader.regionMapSectionId, 0);
}

void Sinnoh_JournalRockClimb(void)
{
    AddLoc(LE_USED_ROCK_CLIMB, gMapHeader.regionMapSectionId, 0);
}

void Sinnoh_JournalDefog(void)
{
    AddLoc(LE_USED_DEFOG, gMapHeader.regionMapSectionId, 0);
}

void Sinnoh_JournalDug(void)
{
    AddLoc(LE_DUG_UNDERGROUND, 0, 0);
}

// (`sinnoh SINNOH_JOURNAL, TYPE, PARAM`: Platinum's CreateJournalEvent, ScrCmd_CreateJournalEvent's kinds) PARAM: the
// item for LE_ITEM_WAS_OBTAINED (written only when it was picked up: Std_FindItem's VAR_0x8007)
void Sinnoh_JournalScript(u16 type, u16 param)
{
    if (type == JOURNAL_GIVEN)
    {
        Sinnoh_JournalLoaded();
        return;
    }
    switch (type)
    {
    case LE_GAME_CORNER:
    case LE_SAFARI_GAME:
    case LE_BATTLE_TOWER ... LE_BATTLE_ARCADE:
        AddLoc(type, 0, 0);
        break;
    case LE_ITEM_WAS_OBTAINED:
        if (gSpecialVar_0x8007)
            AddLoc(type, param, 0);
        break;
    case LE_USED_CUT:
    case LE_USED_SURF:
    case LE_USED_STRENGTH:
    case LE_USED_DEFOG:
    case LE_USED_ROCK_SMASH:
    case LE_USED_WATERFALL:
    case LE_USED_ROCK_CLIMB:
        AddLoc(type, gMapHeader.regionMapSectionId, 0);
        break;
    }
}

// (shop.c) a POKé MART visit: each purchase / sale (more than one at once counts as a lot), then its end
void Sinnoh_JournalShopReset(void)
{
    sS.bought = sS.sold = 0;
}

void Sinnoh_JournalShopBuy(u16 amount, bool8 sold)
{
    u8 *n = sold ? &sS.sold : &sS.bought;
    if (amount > 1)
        *n = 2;
    else if (*n != 255)
        (*n)++;
}

void Sinnoh_JournalShopEnd(void)
{
    u16 map = CurMap();
    if (gMapHeader.regionMapSectionId != MAPSEC_SINNOH_VEILSTONE_STORE && map != MAP_SINNOH_ETERNA_CITY_HERB_SHOP
        && map != MAP_SINNOH_CELESTIC_TOWN_NORTHWEST_HOUSE)
    {
        if (sS.bought && sS.sold)
            AddLoc(LE_BUSINESS_AT_MART, 0, 0);
        else if (sS.bought > 1)
            AddLoc(LE_LOTS_OF_SHOPPING, 0, 0);
        else if (sS.sold > 1)
            AddLoc(LE_SOLD_A_LOT, 0, 0);
        else if (sS.bought)
            AddLoc(LE_SHOPPED_AT_MART, 0, 0);
        else if (sS.sold)
            AddLoc(LE_SOLD_A_LITTLE, 0, 0);
    }
    sS.bought = sS.sold = 0;
}

// (pokemon_storage_system.c) a POKéMON moved in the PC Box, then the box closed
void Sinnoh_JournalBoxUsed(void)
{
    sS.boxUsed = TRUE;
}

void Sinnoh_JournalBoxClosed(void)
{
    if (sS.boxUsed)
        AddLoc(LE_USED_PC_BOX, 0, 0);
    sS.boxUsed = FALSE;
}

// Platinum's time of day (morning 0, day, evening, night, late night 4: SINNOH_TIME_OF_DAY's)
static u8 TimeOfDay(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    u8 h = t ? t->tm_hour : 12;
    return h >= 20 ? 3 : h >= 17 ? 2 : h >= 10 ? 1 : h >= 4 ? 0 : 4;
}

static void SetMon(u8 result, struct Pokemon *mon)
{
    struct Page *p = Cur();
    u8 tens = gSaveBlock2Ptr->playTimeMinutes / 10;
    p->monResult = result;
    p->species = GetMonData(mon, MON_DATA_SPECIES);
    p->monGender = GetMonGender(mon);
    p->monTime = TimeOfDay();
    p->monVariant = (tens == 0 || tens == 2 || tens == 4) ? 0 : (tens == 1 || tens == 3) ? 1 : 2;
}

// (battle_setup.c, a battle's end) encounter.c UpdateJournal
void Sinnoh_JournalBattleEnd(void)
{
    u16 trainer;
    if (!Active() || (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_RECORDED)))
        return;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
    {
        if (gBattleOutcome == B_OUTCOME_WON)
        {
            if (sS.wildDefeated < 255)
                sS.wildDefeated++;
            if (sS.wildDefeated >= 5)
                SetMon(2, &gEnemyParty[0]);
        }
        else if (gBattleOutcome == B_OUTCOME_CAUGHT)
            SetMon(1, &gEnemyParty[gBattlerPartyIndexes[gBattlerTarget] < PARTY_SIZE ? gBattlerPartyIndexes[gBattlerTarget] : 0]);
        return;
    }
    if (gBattleOutcome != B_OUTCOME_WON)
        return;
    trainer = gTrainerBattleOpponent_A;
    if (GymOfTrainer(trainer) != 0xFF)
        AddLoc(LE_BEAT_GYM_LEADER, GymOfTrainer(trainer), trainer);
    else if (trainer == TRAINER_SINNOH_ELITE_FOUR_AARON || trainer == TRAINER_SINNOH_ELITE_FOUR_AARON_REMATCH
          || trainer == TRAINER_SINNOH_ELITE_FOUR_BERTHA || trainer == TRAINER_SINNOH_ELITE_FOUR_BERTHA_REMATCH
          || trainer == TRAINER_SINNOH_ELITE_FOUR_FLINT || trainer == TRAINER_SINNOH_ELITE_FOUR_FLINT_REMATCH
          || trainer == TRAINER_SINNOH_ELITE_FOUR_LUCIAN || trainer == TRAINER_SINNOH_ELITE_FOUR_LUCIAN_REMATCH)
        AddLoc(LE_BEAT_ELITE_FOUR_MEMBER, 0, trainer);
    else if (trainer == TRAINER_SINNOH_CHAMPION_CYNTHIA || trainer == TRAINER_SINNOH_CHAMPION_CYNTHIA_REMATCH)
        AddLoc(LE_BEAT_CHAMPION, 0, trainer);
    else
    {
        struct Page *p = Cur();
        p->trainer = trainer;
        p->trainerWhere = gMapHeader.regionMapSectionId;
    }
}

// (mp_battle.c, a battle with friends ended in SINNOH) KIND 1 single, 2 double, 3 multi; RESULT 0 won, 1 lost, 2 tie
void Sinnoh_JournalFriendBattle(u8 kind, u8 result, const u8 *name1, const u8 *name2)
{
    struct OnlineEvent *e;
    struct Page *p;
    u32 i;
    if (!Active())
        return;
    p = Cur();
    for (i = 0; i < ONLINE_EVENTS && p->online[i].kind; i++)
        ;
    if (i == ONLINE_EVENTS)   // (JournalEntry_GetEmptyOnlineEvent)
    {
        p->online[0] = p->online[1];
        i = ONLINE_EVENTS - 1;
    }
    e = &p->online[i];
    memset(e, 0, sizeof(*e));
    e->kind = kind;
    e->result = result;
    if (name1)
        *StringCopyN(e->name[0], name1, 7) = EOS;
    if (name2)
        *StringCopyN(e->name[1], name2, 7) = EOS;
}

// ---------------------------------------------------------------------------------------------------- the words
static u8 sLine[160];

static const u8 *Words(const char *name)
{
    char full[64];
    snprintf(full, sizeof(full), "JournalEntries_Text_%s", name);
    return S3D_TextByName(full);
}

static const u8 *Numbered(u32 bank, u32 n)
{
    char full[32];
    snprintf(full, sizeof(full), "pl_msg_%08u_%05u", (unsigned)bank, (unsigned)n);
    return S3D_TextByName(full);
}

static void Place(u8 *dst, u16 mapsec)
{
    GetMapName(dst, mapsec, 0);
}

static void Format(const u8 *words)
{
    StringExpandPlaceholders(sLine, words);
}

static u32 Len(const u8 *s)
{
    return StringLength(s);
}

// One place event's line (journal_printer.c's JournalPrinter_Print*)
static void LocLine(const struct LocEvent *e)
{
    static const char *const sMisc[LE_COUNT] = {
        [LE_RESTED_AT_HOME] = "RestedAtHome", [LE_LEFT_RESEARCH_LAB] = "LeftResearchLab", [LE_USED_PC_BOX] = "UsedPcBox",
        [LE_SHOPPED_AT_MART] = "ShoppedAtPokeMart", [LE_LOTS_OF_SHOPPING] = "LotsOfShoppingAtPokeMart",
        [LE_SOLD_A_LITTLE] = "Sold_a_little_at_poke_mart", [LE_SOLD_A_LOT] = "Sold_a_lot_at_poke_mart",
        [LE_BUSINESS_AT_MART] = "DidBusinessAtPokeMart", [LE_GAME_CORNER] = "PlayedAtGameCorner",
        [LE_SAFARI_GAME] = "PlayedSafariGame", [LE_DUG_UNDERGROUND] = "DugUnderground",
        [LE_BUILT_SECRET_BASE] = "BuiltSecretBase", [LE_BATTLE_TOWER] = "PlayedAtBattleTower",
        [LE_BATTLE_FACTORY] = "PlayedAtBattleFactory", [LE_BATTLE_CASTLE] = "PlayedAtBattleCastle",
        [LE_BATTLE_HALL] = "PlayedAtBattleHall", [LE_BATTLE_ARCADE] = "PlayedAtBattleArcade",
        [LE_LURED_POKEMON] = "LuredPokemon", [LE_USED_SOFTBOILED] = "UsedSoftboiled", [LE_USED_MILK_DRINK] = "UsedMilkDrink",
        [LE_USED_CUT] = "UsedCutAtLocation", [LE_FLEW_TO_LOCATION] = "FlewToLocation", [LE_USED_SURF] = "UsedSurfAtLocation",
        [LE_USED_STRENGTH] = "UsedStrengthAtLocation", [LE_USED_DEFOG] = "UsedDefogAtLocation",
        [LE_USED_ROCK_SMASH] = "UsedRockSmashAtLocation", [LE_USED_WATERFALL] = "UsedWaterfallAtLocation",
        [LE_USED_ROCK_CLIMB] = "UsedRockClimbAtLocation", [LE_USED_FLASH] = "UsedFlashAtLocation",
        [LE_WARPED_TO_LOCATION] = "WarpedToLocation", [LE_USED_DIG] = "UsedDigAtLocation",
        [LE_ARRIVED_IN_LOCATION] = "ArrivedInLocation",
    };
    sLine[0] = EOS;
    switch (e->type)
    {
    case LE_GYM_WAS_TOO_TOUGH:
        StringCopy(gStringVar1, Numbered(378, e->where & 7));
        Format(Words("GymWasTooTough"));
        break;
    case LE_BEAT_GYM_LEADER:
        StringCopy(gStringVar1, Numbered(378, e->where & 7));
        StringCopy(gStringVar2, GetTrainerNameFromId(e->trainer));
        Format(Words("BeatLocationsGymLeader"));
        break;
    case LE_BEAT_ELITE_FOUR_MEMBER:
        StringCopy(gStringVar1, GetTrainerNameFromId(e->trainer));
        Format(Words("BeatEliteFourMember"));
        break;
    case LE_BEAT_CHAMPION:
        StringCopy(gStringVar1, GetTrainerNameFromId(e->trainer));
        Format(Words("BeatChampion"));
        break;
    case LE_LEFT_CAVE:   // (JournalPrinter_PrintLeftCave: these three were "departed from")
        Place(gStringVar1, e->where);
        Format(Words(e->where == MAPSEC_SINNOH_VALLEY_WINDWORKS || e->where == MAPSEC_SINNOH_SNOWPOINT_TEMPLE
                     || e->where == MAPSEC_SINNOH_FUEGO_IRONWORKS ? "DepartedFromLocation" : "GotThroughLocation"));
        break;
    case LE_LEFT_BUILDING:
    {
        s32 b = BuildingIndex(e->where);
        Place(gStringVar1, e->where);
        Format(Words(b >= 0 && sBuildings[b].exited ? "ExitedFromLocation" : "DepartedFromLocation"));
        break;
    }
    case LE_ITEM_WAS_OBTAINED:
        CopyItemName(e->where, gStringVar1);
        Format(Words("ItemWasObtained"));
        break;
    default:
        if (e->type < LE_COUNT && sMisc[e->type])
        {
            Place(gStringVar1, e->where);
            Format(Words(sMisc[e->type]));
        }
        break;
    }
}

static void MonLine(const struct Page *p)
{
    static const char *const sCaught[3] = {"CaughtPokemon", "PokemonWasCaught", NULL};
    static const char *const sBeat[3] = {"PokemonWasDefeated", "DefeatedPokemon", NULL};
    const char *name;
    sLine[0] = EOS;
    if (p->monResult != 1 && p->monResult != 2)
        return;
    name = (p->monResult == 1 ? sCaught : sBeat)[p->monVariant > 2 ? 2 : p->monVariant];
    if (name == NULL)   // (the third: by gender)
        name = p->monGender == MON_MALE ? (p->monResult == 1 ? "CaughtMalePokemon" : "DefeatedMalePokemon")
             : p->monGender == MON_FEMALE ? (p->monResult == 1 ? "CaughtFemalePokemon" : "DefeatedFemalePokemon")
             : (p->monResult == 1 ? "CaughtPokemon" : "DefeatedPokemon");
    StringCopy(gStringVar1, gSpeciesNames[p->species < NUM_SPECIES ? p->species : 0]);
    StringCopy(gStringVar2, Numbered(608, p->monTime < 5 ? p->monTime : 1));
    Format(Words(name));
}

// JournalPrinter_PrintTrainerEvent: the longer the place and the name, the shorter the words
static void TrainerLine(const struct Page *p)
{
    u32 len;
    sLine[0] = EOS;
    if (p->trainer == 0)
        return;
    Place(gStringVar1, p->trainerWhere);
    StringCopy(gStringVar2, GetTrainerNameFromId(p->trainer));
    len = Len(gStringVar1) + Len(gStringVar2);
    Format(Words(len <= 14 ? "BattledTrainerAtLocation" : len <= 16 ? "BeatTrainerAtLocation"
               : len <= 19 ? "BeatTrainerAtLocation_2" : "MetLocationsTrainer"));
}

static void OnlineLine(const struct OnlineEvent *e)
{
    static const char *const sWords[3][3] = {
        {"BeatPlayerSingle", "LostToPlayerSingle", "TiedPlayerSingle"},
        {"BeatPlayerDouble", "LostToPlayerDouble", "TiedPlayerDouble"},
        {"BeatPlayersMulti", "LostToPlayersMulti", "TiedPlayersMulti"},
    };
    sLine[0] = EOS;
    if (e->kind < 1 || e->kind > 3 || e->result > 2)
        return;
    StringCopy(gStringVar1, e->name[0]);
    StringCopy(gStringVar2, e->name[1]);
    Format(Words(sWords[e->kind - 1][e->result]));
}

// ---------------------------------------------------------------------------------------------------- the screen
// Platinum's 256x192 page shown from (CROP_X, CROP_Y): its top dots, the title line and the nine lines below
#define CROP_X 8
#define CROP_Y 12
#define TITLE_X (16 - CROP_X)
#define TITLE_Y (16 - CROP_Y)
#define LINES_X (32 - CROP_X)
#define LINES_Y (32 - CROP_Y)
#define ROW_H 16
#define TURN_FRAMES 12
#define W DISPLAY_WIDTH
#define H DISPLAY_HEIGHT
#define SIG_A 0x1235   // (the palette's signature, as s3d_townmap.c's: another screen's palette turns the picture off)
#define SIG_B 0x0422

static const u16 sPal[16] = {0x7FFF, 0x0000, SIG_A, SIG_B};

static const struct BgTemplate sBgs[] = {
    {.bg = 0, .charBaseIndex = 2, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
    {.bg = 3, .charBaseIndex = 0, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sWins[] = {
    {.bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 20, .paletteNum = 15, .baseBlock = 1},
    DUMMY_WIN_TEMPLATE,
};

static struct
{
    MainCallback back;
    u16 buf[2][W * H];   // (the shown page, the one coming in)
    u8 shown;            // (sUi->buf[shown]: the page on top)
    u8 page;
    s8 turn;             // (+1: to the older page, -1: to the newer one; 0: still)
    u8 turnFrame;
    u8 state;
    bool8 continuing;    // (opened on CONTINUE: today's page follows on closing)
} *sUi;

// The page's picture and its words into BUF (the window's 4bpp tiles over Platinum's picture in page PAGE's palette)
static void Compose(u16 *buf, u8 page)
{
    const u8 *px;
    const u16 *pal;
    u16 w, h, x, y;
    const u8 *tiles = gWindows[0].tileData;
    static const u16 sInk[16] = {0, 0, 0x14A5, 0x4E73};   // (2: the words, 3: their shadow)
    bool8 girl = gSaveBlock2Ptr->playerGender != MALE;
    if (S3D_Gfx_Journal(girl, &px, &pal, &w, &h))
    {
        const u16 *p = pal + 16 * (page & 15);
        for (y = 0; y < H; y++)
            for (x = 0; x < W; x++)
            {
                u32 sx = x + CROP_X, sy = y + CROP_Y;
                buf[y * W + x] = (sx < w && sy < h) ? p[px[sy * w + sx]] : 0xFFFF;
            }
    }
    else
        for (x = 0; x < W * H; x++)
            buf[x] = 0xFFFF;
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
        {
            const u8 *t = tiles + ((y / 8) * 30 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2;
            u8 c = (x & 1) ? (*t >> 4) : (*t & 15);
            if (c == 2 || c == 3)
                buf[y * W + x] = 0x8000 | sInk[c];
        }
}

static void Print(s16 x, s16 y, const u8 *s)
{
    static const u8 sColors[3] = {TEXT_COLOR_TRANSPARENT, 2, 3};
    u8 font = GetStringWidth(FONT_NORMAL, s, 0) <= W - x - 2 ? FONT_NORMAL : FONT_NARROW;
    AddTextPrinterParameterized4(0, font, x, y, 0, 0, sColors, TEXT_SKIP_DRAW, s);
}

// JournalPrinter_PrintEntry
static void DrawPage(u16 *buf, u8 page)
{
    const struct Page *p = &sJ.p[page];
    u32 i, x;
    FillWindowPixelBuffer(0, PIXEL_FILL(0));
    if (p->month != 0)
    {
        u8 day[8];
        StringCopy(gStringVar1, Numbered(414, (p->month - 1) % 12));
        ConvertIntToDecimalStringN(day, p->day, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(gStringVar2, day);
        Format(Words("Date"));
        Print(TITLE_X, TITLE_Y, sLine);
        x = TITLE_X + GetStringWidth(FONT_NORMAL, sLine, 0) + 12;
        {
            static const char *const sDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
            Print(x, TITLE_Y, Words(sDays[p->week % 7]));
        }
        Place(gStringVar1, p->start);
        Format(Words("StartedFromLocation"));
        Print(LINES_X, LINES_Y, sLine);
        for (i = 0; i < LOC_EVENTS; i++)
            if (p->loc[i].type != LE_NONE)
            {
                LocLine(&p->loc[i]);
                Print(LINES_X, LINES_Y + (1 + i) * ROW_H, sLine);
            }
        MonLine(p);
        Print(LINES_X, LINES_Y + 5 * ROW_H, sLine);
        TrainerLine(p);
        Print(LINES_X, LINES_Y + 6 * ROW_H, sLine);
        for (i = 0; i < ONLINE_EVENTS; i++)
        {
            OnlineLine(&p->online[i]);
            Print(LINES_X, LINES_Y + (7 + i) * ROW_H, sLine);
        }
    }
    Compose(buf, page);
}

static u16 Shade(u16 c, s32 white, s32 black)
{
    s32 wr = (white & 31), wg = (white >> 5 & 31), wb = (white >> 10 & 31);
    s32 br = (black & 31), bg = (black >> 5 & 31), bb = (black >> 10 & 31);
    return 0x8000 | ((c & 31) * (wr - br) / 31 + br) | ((c >> 5 & 31) * (wg - bg) / 31 + bg) << 5
         | ((c >> 10 & 31) * (wb - bb) / 31 + bb) << 10;
}

// BG3's line: the page on top, sliding away over the next one while a page turns (with a shadow at its edge)
static void Line(int line, u16 *layer, s32 x0, s32 y0, s32 dx, s32 dy)
{
    const u16 *pal = (const u16 *)PLTT + BG_PLTT_ID(0);
    const u16 *top, *under;
    s32 x, edge = W;
    u16 white, black;
    if (sUi == NULL || line < 0 || line >= H || gPlttBufferUnfaded[2] != SIG_A || gPlttBufferUnfaded[3] != SIG_B)
        return;
    white = pal[0], black = pal[1];
    top = sUi->buf[sUi->shown] + line * W;
    under = sUi->buf[sUi->shown ^ 1] + line * W;
    if (sUi->turn)
        edge = W - (W * sUi->turnFrame) / TURN_FRAMES;   // (the top page's right edge, moving left)
    for (x = 0; x < W; x++)
    {
        u16 c;
        if (sUi->turn > 0)   // (the shown page lifts off to the left: the older one under it)
            c = x < edge ? top[x + (W - edge)] : under[x];
        else if (sUi->turn < 0)   // (the newer page comes back in from the left over the shown one)
            c = x < W - edge ? under[x + edge] : top[x];
        else
            c = top[x];
        if (sUi->turn && (sUi->turn > 0 ? (x >= edge && x < edge + 4) : (x >= W - edge && x < W - edge + 4)))
            c = (c & 0x7BDE) >> 1 | ((c & 0x7BDE) >> 2);   // (the page's shadow)
        layer[x] = Shade(c, white, black);
    }
}

static bool8 PageExists(u8 page)
{
    return page < PAGES && sJ.p[page].month != 0;
}

static void VBlankCB_Journal(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Journal(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
    DoScheduledBgTilemapCopiesToVram();
}

static void Close(void)
{
    MainCallback back = sUi->back;
    bool8 continuing = sUi->continuing;
    gMpBgPictureLine = NULL;
    gMpBgPictureBg = -1;
    FreeAllWindowBuffers();
    Free(sUi);
    sUi = NULL;
    if (continuing)
        Sinnoh_JournalMapChanged(TRUE);   // (Platinum: the journal shown first, then today's page)
    SetMainCallback2(back);
}

static void Task_Journal(u8 taskId)
{
    switch (sUi->state)
    {
    case 0:
        if (!gPaletteFade.active)
            sUi->state = 1;
        break;
    case 1:
        if (sUi->turn)
        {
            if (++sUi->turnFrame >= TURN_FRAMES)
            {
                sUi->shown ^= 1;
                sUi->turn = 0;
                sUi->turnFrame = 0;
            }
            break;
        }
        if (JOY_NEW(DPAD_LEFT) || (JOY_NEW(B_BUTTON) && sUi->page != 0))
        {
            if (sUi->page != 0 && PageExists(sUi->page - 1))
            {
                sUi->page--;
                DrawPage(sUi->buf[sUi->shown ^ 1], sUi->page);
                sUi->turn = -1;
                PlaySE(SE_DEX_PAGE);
            }
        }
        else if (JOY_NEW(DPAD_RIGHT | A_BUTTON))
        {
            if (PageExists(sUi->page + 1))
            {
                sUi->page++;
                DrawPage(sUi->buf[sUi->shown ^ 1], sUi->page);
                sUi->turn = 1;
                PlaySE(SE_DEX_PAGE);
            }
        }
        else if (JOY_NEW(B_BUTTON | START_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            sUi->state = 2;
        }
        break;
    case 2:
        if (!gPaletteFade.active)
        {
            DestroyTask(taskId);
            Close();
        }
        break;
    }
}

// The JOURNAL's screen; BACK when it closes. CONTINUING: opened on CONTINUE (today's page follows on closing)
void Sinnoh_JournalOpen(MainCallback back, bool8 continuing)
{
    SetVBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgs, ARRAY_COUNT(sBgs));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);
    DmaFill16(3, 0, VRAM, VRAM_SIZE);
    DmaFill32(3, 0, OAM, OAM_SIZE);
    DmaFill16(3, 0, PLTT, PLTT_SIZE);
    ResetPaletteFade();
    ScanlineEffect_Stop();
    ResetSpriteData();
    FreeAllSpritePalettes();
    ResetTasks();
    InitWindows(sWins);
    DeactivateAllTextPrinters();
    LoadPalette(sPal, BG_PLTT_ID(0), sizeof(sPal));
    SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
    CopyBgTilemapBufferToVram(3);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    sUi = AllocZeroed(sizeof(*sUi));
    sUi->back = back;
    sUi->continuing = continuing;
    if (!continuing)
        Cur();   // (a page for today if there's none yet)
    DrawPage(sUi->buf[0], 0);
    gMpBgPictureBg = 3;
    gMpBgPictureLine = Line;
    ShowBg(3);
    CreateTask(Task_Journal, 80);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
    SetVBlankCallback(VBlankCB_Journal);
    SetMainCallback2(CB2_Journal);
}

// (debug) `mp journal`: a few lines for the test scenario
void Sinnoh_JournalDebug(void)
{
    u32 i;
    const struct Page *p = &sJ.p[0];
    printf("[journal] page 0: %d-%d-%d week %d start %d, mon %d species %d, trainer %d\n", p->year + 2000, p->month,
           p->day, p->week, p->start, p->monResult, p->species, p->trainer);
    for (i = 0; i < LOC_EVENTS; i++)
        if (p->loc[i].type)
            printf("[journal]   event %d where %d trainer %d\n", p->loc[i].type, p->loc[i].where, p->loc[i].trainer);
}
