// Moemon Kai: SINNOH's TV, as Platinum's (the user, 2026-11-02: "Pokemon Platinum's TV system be built and function
// properly"). Ported from pret/pokeplatinum's src/tv_segment.c, src/overlay006/ov6_022465FC.c, tv_commercials.c,
// src/unk_0202E2CC.c and scrcmd_tv_broadcast.c:
// - the broadcast: the program on air is Platinum's weekly schedule's for the quarter hour (gSinnohTvSchedule from
//   tools/sinnoh/tv_data.py); a TV greets, then plays up to four of the program's segments (one a look), then says
//   farewell; a finished program plays a commercial until the next quarter hour
// - the segments: TRAINER SIGHTINGS, RECORDS and INTERVIEWS come from what the player did in SINNOH (four of each kept,
//   a new one replacing one played three times), SINNOH NOW and the VARIETY HOUR from the world as it is
// - the reporters' interviews (one word from the game's own easy chat, GABBY and TY's screen)
// Its words are Platinum's (the TV banks of sinnoh/platinum_text.pak, by Platinum's message names: S3D_TextNamed). The
// records are kept in "<save>.region" (chunk TVBC, mp_region.c), saved with the game.
// Kept to what this game has: online-only segments (groups, Capture the Flag, the GTS) and the Underground's traps and
// flags never air (its treasures do: the digging is here, s3d_mining.c);
// the ones about systems SINNOH doesn't have yet (the dress-up photos, the berry patches, the roamers, the VILLA) wait
// for them (docs/SINNOH_CHECKLIST.md). The SEAL CLUB SHOW is about POKé BALL effects (they take the Seals' place).
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "event_data.h"
#include "easy_chat.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "strings.h"
#include "battle_main.h"
#include "data.h"
#include "mp.h"
#include "mp_region.h"
#include "s3d.h"
#include "s3d_contest.h"
#include "berry.h"
#include "constants/berry.h"
#include "constants/battle.h"
#include "battle.h"
#include "battle_message.h"
#include "constants/characters.h"
#include "constants/easy_chat.h"
#include "constants/weather.h"
#include "constants/items.h"
#include "constants/region_map_sections.h"
#include "constants/species.h"

extern const u8 gSinnohTvSchedule[];
extern const u8 gSinnohTvPrograms[];
extern u8 gSinnohStrVar[3][64];
extern const u8 *const gNatureNamePointers[];

#define TV_INTERVIEWS 1
#define TV_SIGHTINGS  2
#define TV_RECORDS    3
#define TV_SINNOH_NOW 4
#define TV_VARIETY    5
#define TV_MAX_PLAYED 4
#define TV_KEPT       4
#define TV_SEGMENT_SIZE 40
#define VAR_SINNOH_INTERVIEW_QUESTION 0x5210   // (Platinum's VAR_RANDOM_INTERVIEW_QUESTION: tools/sinnoh/ids.py)

struct TvInstance { u8 segment, timesPlayed, pad[2]; u32 day; u8 data[TV_SEGMENT_SIZE]; };

// What a segment keeps (TV_SEGMENT_SIZE bytes each)
struct TvMon { u16 species; u8 gender, hasNick; u8 nick[POKEMON_NAME_LENGTH + 2]; };   // (16 bytes)
struct TvCatch { struct TvMon mon; u16 ball, balls; u8 result; };
struct TvFishing { struct TvMon mon; u16 rod; u8 caught; };
struct TvHidden { u16 item, mapsec; };
struct TvShopping { u16 item, amount; };
struct TvEgg { struct TvMon mon; u16 mapsec; };
struct TvSafari { struct TvMon mon; u8 caught; };
struct TvBerries { u16 item, amount; u8 rating; };
struct TvBallFx { struct TvMon mon; u8 effect[20]; };
struct TvTower { struct TvMon mon; u16 streak; u8 single; };
struct TvRibbon { struct TvMon mon; u8 ribbon, count; };
struct TvSlots { u32 coins, start, minutes; };
struct TvCounter { u32 count; };
struct TvWord { u16 word; };
struct TvTowerTalk { u8 win; u8 pad; u16 streak; u16 word; };
struct TvPokemonTalk { struct TvMon mon; u16 word; };
struct TvPoketchTalk { u8 app; u8 pad; u16 word; };
struct TvContestTalk { u16 species; u8 gender, type, rank, place; u16 word; };
struct TvPersonality { u8 type; };
struct TvPoffinTalk { u8 poffin; u8 pad; u16 word; };
struct TvAmityTalk { struct TvMon mon; u8 nature, kind, accessory, pad; u16 item, word; };
struct TvFrontierTalk { struct TvMon mon; u16 word; };
struct TvFrontierMultiTalk { u8 facility, gender; u8 name[PLAYER_NAME_LENGTH + 1]; u16 word; };

// The save's side ("<save>.region" chunk TVBC)
#define TV_MAGIC 0x31435654   // "TVC1"
static struct
{
    u32 magic;
    u32 slot;                          // the quarter hour (days * 96 + hour * 4 + quarter) the played list is of
    u8 played[TV_MAX_PLAYED], finished, pad[3];
    struct TvInstance kept[3][TV_KEPT];   // INTERVIEWS, TRAINER SIGHTINGS, RECORDS
    // what the interviews ask about (Platinum's TVBroadcast's pending records: set by the events, taken by a reporter)
    struct { u8 valid, type, rank, place; u16 species; u8 gender, pad; } contest;
    struct { u8 valid, kind, nature, accessory; u16 item; struct TvMon mon; } amity;
    struct { u8 valid, poffin; } poffin;
    struct { u8 valid, win; u16 streak; } tower;
    struct { u8 active, caught; struct TvMon mon; } safari;
    struct { u8 valid, pad; struct TvMon mon; } frontier;
    struct { u8 valid, facility, gender, pad; u8 name[PLAYER_NAME_LENGTH + 1]; } frontierMulti;
    u32 bp, trades;                    // RECORDS' counters (Platinum's BP earned, GTS trades: flushed daily)
    u32 counterDay;
    u8 questionDay, question;          // the ROUTE 214 gate's question of the day (Platinum's VAR_RANDOM_INTERVIEW_QUESTION)
    u16 word;                          // the word the easy chat gave (Sinnoh_TvChooseWord)
} sTv;

static u8 *Slot(u8 i)
{
    return i == 0 ? gStringVar1 : i == 1 ? gStringVar2 : i == 2 ? gStringVar3 : gSinnohStrVar[i - 3];
}

bool8 Sinnoh_TownVisited(u16 mapsec);
bool8 Sinnoh_SwarmToday(u16 *mapsec, u16 *species);
bool8 Sinnoh_DexCompleted(void);
u8 Sinnoh_PoketchCurrentApp(void);

// JUBILIFE TV's dress-up photo (Platinum's ImageClips' newest DressUpPhoto: s3d_fashion.c keeps the studio's one)
static bool8 Sinnoh_HasDressUpPhoto(void) { return Sinnoh_ContestPhoto(PHOTO_STUDIO) != NULL; }
static u16 Sinnoh_DressUpPhotoSpecies(void)
{
    const struct ContestEntrant *e = Sinnoh_ContestPhoto(PHOTO_STUDIO);
    return e ? e->species : SPECIES_NONE;
}

// Platinum's contest type and rank names (its contest_text bank)
static void Sinnoh_ContestTypeName(u8 *dest, u8 type)
{
    static const char *const sNames[] = {"Contest_Text_CoolContest", "Contest_Text_BeautyContest", "Contest_Text_CuteContest",
                                         "Contest_Text_SmartContest", "Contest_Text_ToughContest"};
    StringCopy(dest, S3D_TextByName(sNames[type < 5 ? type : 0]));
}

static void Sinnoh_ContestRankName(u8 *dest, u8 rank)
{
    static const char *const sNames[] = {"Contest_Text_NormalRank", "Contest_Text_GreatRank", "Contest_Text_UltraRank",
                                         "Contest_Text_MasterRank", "Contest_Text_Link"};
    StringCopy(dest, S3D_TextByName(sNames[rank < 5 ? rank : 4]));
}

// ---------------------------------------------------------------------------------------------- the save's side
static void Fresh(void)
{
    if (sTv.magic != TV_MAGIC)
    {
        memset(&sTv, 0, sizeof(sTv));
        sTv.magic = TV_MAGIC;
    }
}

const void *Sinnoh_TvData(u32 *size)
{
    Fresh();
    *size = sizeof(sTv);
    return &sTv;
}

void Sinnoh_TvLoad(const void *data, u32 size)
{
    memset(&sTv, 0, sizeof(sTv));
    if (size == sizeof(sTv) && ((const u32 *)data)[0] == TV_MAGIC)
        memcpy(&sTv, data, size);
    Fresh();
}

void Sinnoh_TvReset(void)   // (a new game: mp_region.c)
{
    memset(&sTv, 0, sizeof(sTv));
    Fresh();
}

static bool8 InSinnoh(void)
{
    return MP_Region_Current() == REGION_SINNOH;
}

static u32 Today(void)
{
    return RtcGetLocalDayCount();
}

static void SaveSegment(u8 type, u8 segment, const void *data, u32 size)
{
    struct TvInstance *k;
    u32 i;
    if (!InSinnoh() || type < TV_INTERVIEWS || type > TV_RECORDS)
        return;
    Fresh();
    k = sTv.kept[type - 1];
    // (Platinum's TVBroadcast_SaveSegmentData: the same segment again replaces itself only once played three times;
    // else an empty place, else one played three times)
    for (i = 0; i < TV_KEPT; i++)
        if (k[i].segment == segment)
        {
            if (k[i].timesPlayed < 3)
                return;
            break;
        }
    if (i == TV_KEPT)
        for (i = 0; i < TV_KEPT && k[i].segment != 0; i++)
            ;
    if (i == TV_KEPT)
        for (i = 0; i < TV_KEPT && k[i].timesPlayed < 3; i++)
            ;
    if (i == TV_KEPT)
        return;
    memset(&k[i], 0, sizeof(k[i]));
    k[i].segment = segment;
    k[i].day = Today();
    memcpy(k[i].data, data, size < TV_SEGMENT_SIZE ? size : TV_SEGMENT_SIZE);
    MPNet_Log("sinnoh tv: segment %d of program type %d kept", segment, type);
}

static struct TvInstance *Kept(u8 type, u8 segment)
{
    u32 i;
    if (type < TV_INTERVIEWS || type > TV_RECORDS)
        return NULL;
    for (i = 0; i < TV_KEPT; i++)
        if (sTv.kept[type - 1][i].segment == segment)
            return &sTv.kept[type - 1][i];
    return NULL;
}

static void MonOf(struct Pokemon *mon, struct TvMon *out)
{
    memset(out, 0, sizeof(*out));
    out->species = GetMonData(mon, MON_DATA_SPECIES);
    out->gender = GetMonGender(mon);
    GetMonData(mon, MON_DATA_NICKNAME, out->nick);
    out->nick[POKEMON_NAME_LENGTH] = EOS;
    out->hasNick = StringCompare(out->nick, gSpeciesNames[out->species]) != 0;
}

static bool8 Seen(u16 species)
{
    return species != SPECIES_NONE && GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN);
}

static bool8 Arrived(u16 mapsec)
{
    // (Platinum's first-arrival flags: this game's SINNOH FLY spots, set on a first visit: s3d_townmap.c)
    bool8 Sinnoh_TownVisited(u16 mapsec);
    return Sinnoh_TownVisited(mapsec);
}

static void SetPlayer(u8 slot)
{
    StringCopy(Slot(slot), gSaveBlock2Ptr->playerName);
}

static void SetSpecies(u8 slot, u16 species)
{
    StringCopy(Slot(slot), gSpeciesNames[species < NUM_SPECIES ? species : 0]);
}

static void SetNumber(u8 slot, u32 n, u8 digits)
{
    ConvertIntToDecimalStringN(Slot(slot), n, STR_CONV_MODE_LEFT_ALIGN, digits);
}

static void SetItem(u8 slot, u16 item)
{
    CopyItemName(item, Slot(slot));
}

static void SetPlace(u8 slot, u16 mapsec)
{
    GetMapName(Slot(slot), mapsec, 0);
}

static void SetWord(u8 slot, u16 word)
{
    CopyEasyChatWord(Slot(slot), word);
}

static void SetPlatinum(u8 slot, const char *name)
{
    StringCopy(Slot(slot), S3D_TextByName(name));
}

static void SetPoketchApp(u8 slot, u8 app)   // (Platinum's app number: its poketch_app_names bank)
{
    char name[24];
    snprintf(name, sizeof(name), "pl_msg_00000457_%05u", app);
    SetPlatinum(slot, name);
}

static void SetRival(u8 slot)
{
    static const u8 sBarry[] = _("BARRY");
    StringCopy(Slot(slot), sBarry);
}

static void SetCounterpart(u8 slot)
{
    static const u8 sDawn[] = _("DAWN"), sLucas[] = _("LUCAS");
    StringCopy(Slot(slot), gSaveBlock2Ptr->playerGender == MALE ? sDawn : sLucas);
}

static void SetFlavor(u8 slot, u8 flavor)   // (Platinum's flavor_names bank: spicy, dry, sweet, bitter, sour)
{
    char name[24];
    snprintf(name, sizeof(name), "pl_msg_00000606_%05u", flavor);
    SetPlatinum(slot, name);
}

// ---------------------------------------------------------------------------------------------- recorded by events
// A wild battle in SINNOH (battle_setup.c CB2_EndWildBattle): Platinum's CATCH THAT POKéMON SHOW. RESULT: B_OUTCOME_*
void Sinnoh_TvWildBattleEnd(struct Pokemon *mon, u8 result, u16 ballsThrown, u16 ball)
{
    struct TvCatch c;
    if (ballsThrown == 0)
        return;
    memset(&c, 0, sizeof(c));
    MonOf(mon, &c.mon);
    c.ball = ball;
    c.balls = ballsThrown > 999 ? 999 : ballsThrown;
    c.result = result;
    if (result == B_OUTCOME_CAUGHT)
        SaveSegment(TV_SIGHTINGS, 1, &c, sizeof(c));
    else if ((result == B_OUTCOME_WON || result == B_OUTCOME_RAN || result == B_OUTCOME_MON_FLED) && c.balls > 2)
        SaveSegment(TV_SIGHTINGS, 2, &c, sizeof(c));
}

// Fishing in SINNOH (fishing: field_player_avatar.c): WHAT'S FISHING. MON: what bit (NULL: not even a nibble)
void Sinnoh_TvFishing(struct Pokemon *mon, u16 rod)
{
    struct TvFishing f;
    memset(&f, 0, sizeof(f));
    if (mon != NULL)
        MonOf(mon, &f.mon);
    f.rod = rod;
    f.caught = mon != NULL;
    SaveSegment(TV_SIGHTINGS, 3, &f, sizeof(f));
}

// A hidden item found in SINNOH: HIDDEN ITEM BREAKING NEWS
void Sinnoh_TvHiddenItem(u16 item)
{
    struct TvHidden h = {item, gMapHeader.regionMapSectionId};
    SaveSegment(TV_SIGHTINGS, 6, &h, sizeof(h));
}

// A treasure dug up in the Underground (s3d_mining.c): UNDERGROUND TREASURES CORNER
void Sinnoh_TvUndergroundTreasure(u16 item)
{
    struct TvHidden h = {item, 0};
    SaveSegment(TV_SIGHTINGS, 13, &h, sizeof(h));
}

// Shopping in SINNOH (shop.c): SINNOH SHOPPING CHAMP CORNER, ten or more at once
void Sinnoh_TvShopping(u16 item, u16 amount)
{
    struct TvShopping s = {item, amount > 999 ? 999 : amount};
    if (amount >= 10)
        SaveSegment(TV_SIGHTINGS, 7, &s, sizeof(s));
}

// An Egg hatched in SINNOH (egg_hatch.c): HAPPY HAPPY EGG CLUB
void Sinnoh_TvEggHatched(struct Pokemon *mon)
{
    struct TvEgg e;
    MonOf(mon, &e.mon);
    e.mapsec = gMapHeader.regionMapSectionId;
    SaveSegment(TV_SIGHTINGS, 8, &e, sizeof(e));
}

// A new nickname from the name rater (Platinum's script command 2B8): RATE THAT NAME CHANGE
void Sinnoh_TvNameRated(struct Pokemon *mon)
{
    struct TvMon m;
    MonOf(mon, &m);
    SaveSegment(TV_SIGHTINGS, 10, &m, sizeof(m));
}

// The GREAT MARSH's SAFARI GAME: SAFARI GAME SPECIAL NEWS BULLETIN (started, a catch, the end)
void Sinnoh_TvSafariStart(void)
{
    Fresh();
    sTv.safari.active = TRUE;
    sTv.safari.caught = 0;
}

void Sinnoh_TvSafariCatch(struct Pokemon *mon)
{
    if (!sTv.safari.active)
        return;
    if (sTv.safari.caught == 0)
        MonOf(mon, &sTv.safari.mon);
    if (sTv.safari.caught < 99)
        sTv.safari.caught++;
}

void Sinnoh_TvSafariEnd(void)
{
    struct TvSafari s;
    if (sTv.safari.active && sTv.safari.caught > 0)
    {
        s.mon = sTv.safari.mon;
        s.caught = sTv.safari.caught;
        SaveSegment(TV_SIGHTINGS, 15, &s, sizeof(s));
    }
    sTv.safari.active = FALSE;
}

// The party all one gender at a POKéMON CENTER's PC (Platinum's SaveTVSegmentPokemonStorageBulletin)
void Sinnoh_TvStorageBulletin(void)
{
    bool8 male = FALSE, female = FALSE, none = FALSE;
    struct TvMon m;
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES) || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        switch (GetMonGender(mon))
        {
        case MON_MALE: male = TRUE; break;
        case MON_FEMALE: female = TRUE; break;
        default: none = TRUE; break;
        }
    }
    if (none || male == female)
        return;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            break;
    if (i == PARTY_SIZE)
        return;
    MonOf(&gPlayerParty[i], &m);
    SaveSegment(TV_SIGHTINGS, 16, &m, sizeof(m));
}

// BERRIES picked in SINNOH: PLANTING AND WATERING SHOW (RATING 4: a perfect crop, 0: none)
void Sinnoh_TvBerries(u16 item, u8 rating, u16 amount)
{
    struct TvBerries b = {item, amount, rating};
    if (rating == 4)
        SaveSegment(TV_SIGHTINGS, 18, &b, sizeof(b));
    else if (rating == 0)
        SaveSegment(TV_SIGHTINGS, 19, &b, sizeof(b));
}

// A POKé BALL effect given to a POKéMON in SINNOH (mp_ballfx.c, in the Seals' place): the SEAL CLUB SHOW
void Sinnoh_TvBallEffect(struct BoxPokemon *box, const char *effectName)
{
    struct TvBallFx b;
    struct Pokemon mon;
    memset(&b, 0, sizeof(b));
    if (!InSinnoh())
        return;
    BoxMonToMon(box, &mon);
    MonOf(&mon, &b.mon);
    MP_AsciiToGame(b.effect, effectName, sizeof(b.effect) - 1);
    SaveSegment(TV_SIGHTINGS, 21, &b, sizeof(b));
}

// The VILLA's furniture (Platinum's SaveTVSegmentHomeAndManor*)
void Sinnoh_TvVilla(u8 furniture, bool8 none)
{
    struct { u8 furniture; } v = {furniture};
    SaveSegment(TV_SIGHTINGS, none ? 25 : 26, &v, sizeof(v));
}

// RECORDS: a BATTLE TOWER challenge ended (its streak, the first POKéMON, singles or doubles)
void Sinnoh_TvTowerRecord(struct Pokemon *mon, u16 streak, bool8 single)
{
    struct TvTower t;
    MonOf(mon, &t.mon);
    t.streak = streak;
    t.single = single;
    SaveSegment(TV_RECORDS, 1, &t, sizeof(t));
}

// RECORDS: a ribbon won; on its 15th, 20th... 40th
void Sinnoh_TvRibbon(struct Pokemon *mon, u8 ribbon, u8 count)
{
    struct TvRibbon r;
    if (count < 15 || count > 40 || count % 5 != 0)
        return;
    MonOf(mon, &r.mon);
    r.ribbon = ribbon;
    r.count = count;
    SaveSegment(TV_RECORDS, 5, &r, sizeof(r));
}

// RECORDS: the GAME CORNER's slots left with 1,000 COINS more than at the start
void Sinnoh_TvSlots(u32 coins, u32 start, u32 minutes)
{
    struct TvSlots s = {coins, start, minutes};
    if (coins >= start + 1000)
        SaveSegment(TV_RECORDS, 4, &s, sizeof(s));
}

// RECORDS: BP earned (Platinum's frontier commands), flushed daily into a segment at 30
void Sinnoh_TvBattlePoints(u32 bp)
{
    Fresh();
    sTv.bp = sTv.bp + bp > 9999 ? 9999 : sTv.bp + bp;
}

// The interviews' subjects (Platinum's TVBroadcast pending records)
void Sinnoh_TvContestResult(struct Pokemon *mon, u8 type, u8 rank, u8 place)
{
    Fresh();
    sTv.contest.valid = TRUE;
    sTv.contest.species = GetMonData(mon, MON_DATA_SPECIES);
    sTv.contest.gender = GetMonGender(mon);
    sTv.contest.type = type;
    sTv.contest.rank = rank;
    sTv.contest.place = place;
}

void Sinnoh_TvPoffin(u8 poffin)
{
    Fresh();
    sTv.poffin.valid = TRUE;
    sTv.poffin.poffin = poffin;
}

void Sinnoh_TvTowerOutcome(bool8 win, u16 streak)
{
    Fresh();
    sTv.tower.valid = TRUE;
    sTv.tower.win = win;
    sTv.tower.streak = streak;
}

void Sinnoh_TvFrontierHall(struct Pokemon *mon)
{
    Fresh();
    sTv.frontier.valid = TRUE;
    MonOf(mon, &sTv.frontier.mon);
}

void Sinnoh_TvFrontierMulti(u8 facility, const u8 *partner, u8 gender)
{
    Fresh();
    sTv.frontierMulti.valid = TRUE;
    sTv.frontierMulti.facility = facility;
    sTv.frontierMulti.gender = gender;
    StringCopyN(sTv.frontierMulti.name, partner, PLAYER_NAME_LENGTH);
    sTv.frontierMulti.name[PLAYER_NAME_LENGTH] = EOS;
}

// AMITY SQUARE (Platinum's ScrCmd_27C): the walk's POKéMON (0), an item found (1), an accessory found (2)
void Sinnoh_TvAmity(u8 what, u16 arg)
{
    Fresh();
    switch (what)
    {
    case 0:
        if (arg < PARTY_SIZE)
        {
            sTv.amity.valid = TRUE;
            sTv.amity.kind = 0;
            sTv.amity.nature = GetNature(&gPlayerParty[arg]);
            MonOf(&gPlayerParty[arg], &sTv.amity.mon);
        }
        break;
    case 1:
        sTv.amity.kind = 1;
        sTv.amity.item = arg;
        break;
    case 2:
        sTv.amity.kind = 2;
        sTv.amity.accessory = arg;
        break;
    }
}

// Each day (Platinum's sub_0206F2F0): RECORDS' counters become segments
static void DailyFlush(void)
{
    u32 today = Today();
    if (sTv.counterDay == today)
        return;
    sTv.counterDay = today;
    if (sTv.bp >= 30)
    {
        struct TvCounter c = {sTv.bp};
        SaveSegment(TV_RECORDS, 10, &c, sizeof(c));
    }
    sTv.bp = 0;
    sTv.trades = 0;
    // the ROUTE 214 gate's reporter's question of the day (Platinum's VAR_RANDOM_INTERVIEW_QUESTION, tools/sinnoh/ids.py)
    VarSet(VAR_SINNOH_INTERVIEW_QUESTION, Random() % 4);
}

// On each SINNOH map load (s3d_scrcmd.c): the day's changes
void Sinnoh_TvMapLoad(void)
{
    Fresh();
    DailyFlush();
}

// ---------------------------------------------------------------------------------------------- the broadcast
static u32 QuarterHour(u8 *day, u8 *hour, u8 *minute)
{
    RtcCalcLocalTime();
    *day = (RtcGetLocalDayCount() + 5) % 7;   // (day 1 of the PC's calendar was a Saturday: 0 Monday ... 6 Sunday)
    *hour = gLocalTime.hours;
    *minute = gLocalTime.minutes;
    return (u32)RtcGetLocalDayCount() * 96 + *hour * 4 + *minute / 15;
}

static void UpdateSlot(void)
{
    u8 d, h, m;
    u32 slot = QuarterHour(&d, &h, &m);
    Fresh();
    DailyFlush();
    if (slot != sTv.slot)
    {
        sTv.slot = slot;
        sTv.finished = FALSE;
        memset(sTv.played, 0, sizeof(sTv.played));
    }
}

u8 gSinnohTvForcedProgram;   // (tests: mp tvprogram)

// (tests: mp tvprogram N) program N on air from now on, its broadcast from its start
void Sinnoh_TvForceProgram(u8 program)
{
    gSinnohTvForcedProgram = program;
    sTv.finished = FALSE;
    memset(sTv.played, 0, sizeof(sTv.played));
}

static u8 ProgramOnAir(void)
{
    u8 d, h, m, id;
    QuarterHour(&d, &h, &m);
    if (gSinnohTvForcedProgram >= 1 && gSinnohTvForcedProgram <= 9)   // (tests: mp tvprogram)
        return gSinnohTvForcedProgram;
    if (getenv("MOEMON_TEST_TV_PROGRAM"))   // (tests)
        return atoi(getenv("MOEMON_TEST_TV_PROGRAM"));
    id = gSinnohTvSchedule[(((19 + h) % 24) * 4 + m / 15) * 7 + d];
    return id >= 1 && id <= 9 ? id : 0;
}

static u8 ProgramType(u8 program)
{
    return program ? gSinnohTvPrograms[(program - 1) * 12] : 0;
}

static u32 PlayedCount(void)
{
    u32 i, n = 0;
    for (i = 0; i < TV_MAX_PLAYED; i++)
        n += sTv.played[i] != 0;
    return n;
}

static bool8 Played(u8 segment)
{
    u32 i;
    for (i = 0; i < TV_MAX_PLAYED; i++)
        if (sTv.played[i] == segment)
            return TRUE;
    return FALSE;
}

// The segments: each one's message (filling the string slots) and whether it can air
typedef const char *(*TvMessageFn)(struct TvInstance *k);
typedef bool8 (*TvEligibleFn)(struct TvInstance *k);
struct TvSegment { TvMessageFn message; TvEligibleFn eligible; };

static void SetMonName(u8 slot, const struct TvMon *m)
{
    if (m->hasNick)
        StringCopy(Slot(slot), m->nick);
    else
        SetSpecies(slot, m->species);
}

// TRAINER SIGHTINGS
static const char *Catch_Success(struct TvInstance *k)
{
    struct TvCatch *c = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, c->mon.species);
    SetItem(2, c->ball);
    SetNumber(3, c->balls, 3);
    if (c->mon.hasNick)
    {
        StringCopy(Slot(4), c->mon.nick);
        return "TVProgramTrainerSightings_Text_CatchThatPokemonShow_Nicknamed";
    }
    return "TVProgramTrainerSightings_Text_CatchThatPokemonShow_NotNicknamed";
}
static bool8 Catch_SuccessOk(struct TvInstance *k) { return Seen(((struct TvCatch *)k->data)->mon.species); }
static const char *Catch_Failure(struct TvInstance *k)
{
    struct TvCatch *c = (void *)k->data;
    SetPlayer(0);
    SetNumber(1, c->balls, 3);
    return c->result == B_OUTCOME_WON ? "TVProgramTrainerSightings_Text_CatchThatPokemonShow_Fainted"
                                      : "TVProgramTrainerSightings_Text_CatchThatPokemonShow_RanAway";
}
static const char *Fishing(struct TvInstance *k)
{
    struct TvFishing *f = (void *)k->data;
    SetPlayer(0);
    if (!f->caught)
        return "TVProgramTrainerSightings_Text_WhatsFishing_Failure";
    SetItem(1, f->rod);
    SetSpecies(2, f->mon.species);
    return "TVProgramTrainerSightings_Text_WhatsFishing_Success";
}
static bool8 FishingOk(struct TvInstance *k)
{
    struct TvFishing *f = (void *)k->data;
    return !f->caught || Seen(f->mon.species);
}
static const char *Hidden(struct TvInstance *k)
{
    struct TvHidden *h = (void *)k->data;
    SetPlace(0, h->mapsec);
    SetPlayer(1);
    SetItem(2, h->item);
    return "TVProgramTrainerSightings_Text_HiddenItemBreakingNews";
}
static const char *Treasure(struct TvInstance *k)
{
    struct TvHidden *h = (void *)k->data;
    SetPlayer(0);
    SetItem(1, h->item);
    return "TVProgramTrainerSightings_Text_UndergroundTreasuresCorner";
}
static const char *Shopping(struct TvInstance *k)
{
    struct TvShopping *s = (void *)k->data;
    SetItem(0, s->item);
    SetNumber(1, s->amount, 3);
    SetPlayer(2);
    CopyItemNameHandlePlural(s->item, Slot(3), 2);
    return "TVProgramTrainerSightings_Text_SinnohShoppingChampCorner";
}
static const char *Egg(struct TvInstance *k)
{
    struct TvEgg *e = (void *)k->data;
    SetPlace(1, e->mapsec);
    SetPlayer(0);
    SetSpecies(2, e->mon.species);
    return "TVProgramTrainerSightings_Text_HappyHappyEggClub";
}
static bool8 EggOk(struct TvInstance *k) { return Seen(((struct TvEgg *)k->data)->mon.species); }
static const char *NameRater(struct TvInstance *k)
{
    static char sName[4];
    struct TvMon *m = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, m->species);
    StringCopy(Slot(2), m->nick);
    sName[0] = '#';   // (Platinum: RateThatNameChange_MoreAttractive or one of the four after it: Resolve)
    sName[1] = '0' + Random() % 5;
    sName[2] = 0;
    return sName;
}
static bool8 NameRaterOk(struct TvInstance *k) { return Seen(((struct TvMon *)k->data)->species); }
static const char *Safari(struct TvInstance *k)
{
    struct TvSafari *s = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, s->mon.species);
    SetNumber(2, s->caught, 2);
    return "TVProgramTrainerSightings_Text_SafariGameSpecialNewsBulletin";
}
static bool8 SafariOk(struct TvInstance *k) { return Seen(((struct TvSafari *)k->data)->mon.species); }
static const char *Storage(struct TvInstance *k)
{
    struct TvMon *m = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, m->species);
    return m->gender == MON_MALE ? "TVProgramTrainerSightings_Text_PokemonStorageSpecialNewsBulletin_Male"
                                 : "TVProgramTrainerSightings_Text_PokemonStorageSpecialNewsBulletin_Female";
}
static bool8 StorageOk(struct TvInstance *k) { return Seen(((struct TvMon *)k->data)->species); }
static const char *Berries(struct TvInstance *k)
{
    struct TvBerries *b = (void *)k->data;
    SetPlayer(0);
    SetItem(1, b->item);
    SetNumber(2, b->amount, 2);
    return "TVProgramTrainerSightings_Text_PlantingAndWateringShow";
}
static const char *NoBerries(struct TvInstance *k)
{
    struct TvBerries *b = (void *)k->data;
    SetPlayer(0);
    SetItem(1, b->item);
    return "TVProgramTrainerSightings_Text_PlantingAndWateringShow_NoBerries";
}
static const char *BallFx(struct TvInstance *k)
{
    struct TvBallFx *b = (void *)k->data;
    static char sName[8];
    SetPlayer(0);
    StringCopy(Slot(1), b->effect);
    SetSpecies(2, b->mon.species);
    sName[0] = '#';
    sName[1] = '0' + Random() % 3;   // (Platinum: CleverlyCalculated and the two after it)
    sName[2] = 0;
    return sName;
}
static bool8 BallFxOk(struct TvInstance *k) { return Seen(((struct TvBallFx *)k->data)->mon.species); }
static bool8 VillaOk(struct TvInstance *k) { return Arrived(MAPSEC_SINNOH_RESORT_AREA); }
static const char *VillaNone(struct TvInstance *k)
{
    SetPlayer(0);
    return "TVProgramTrainerSightings_Text_HomeAndManor_NoFurniture";
}
static const char *Villa(struct TvInstance *k)
{
    char name[40];
    SetPlayer(0);
    snprintf(name, sizeof(name), "pl_msg_00000603_%05d", k->data[0]);   // (the VILLA's furniture, when it's built)
    SetPlatinum(1, name);
    return "TVProgramTrainerSightings_Text_HomeAndManor";
}

static const struct TvSegment sSightings[] = {
    [1] = {Catch_Success, Catch_SuccessOk},
    [2] = {Catch_Failure, NULL},
    [3] = {Fishing, FishingOk},
    [6] = {Hidden, NULL},
    [7] = {Shopping, NULL},
    [8] = {Egg, EggOk},
    [10] = {NameRater, NameRaterOk},
    [13] = {Treasure, NULL},
    [15] = {Safari, SafariOk},
    [16] = {Storage, StorageOk},
    [18] = {Berries, NULL},
    [19] = {NoBerries, NULL},
    [21] = {BallFx, BallFxOk},
    [25] = {VillaNone, VillaOk},
    [26] = {Villa, VillaOk},
    [27] = {NULL, NULL},
    // (4, 20: groups, 22, 23: Capture the Flag: online, never on air)
};

// RECORDS (Platinum's records bank: its messages in order, so the names are its message numbers)
static const char *Rec_Tower(struct TvInstance *k)
{
    struct TvTower *t = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, t->mon.species);
    SetNumber(2, t->streak, 4);
    return t->single ? "pl_msg_00000417_00000" : "pl_msg_00000417_00001";
}
static bool8 Rec_TowerOk(struct TvInstance *k)
{
    return Seen(((struct TvTower *)k->data)->mon.species) && Arrived(MAPSEC_SINNOH_FIGHT_AREA);
}
static const char *Rec_Size(struct TvInstance *k)
{
    struct TvMon *m = (void *)k->data;
    u32 size = ((k->data[sizeof(struct TvMon)] * 1000) / 254 + 5) / 10;
    SetPlayer(0);
    SetSpecies(1, m->species);
    SetNumber(2, size / 10, 3);
    SetNumber(3, size % 10, 1);
    return "pl_msg_00000417_00002";
}
static bool8 Rec_SizeOk(struct TvInstance *k) { return Seen(((struct TvMon *)k->data)->species); }
static const char *Rec_Slots(struct TvInstance *k)
{
    struct TvSlots *s = (void *)k->data;
    SetPlayer(0);
    SetNumber(1, s->minutes, 10);
    SetNumber(2, s->start, 6);
    SetNumber(3, s->coins, 6);
    return "pl_msg_00000417_00003";
}
static const char *Rec_Ribbon(struct TvInstance *k)
{
    struct TvRibbon *r = (void *)k->data;
    SetPlayer(0);
    SetMonName(1, &r->mon);
    static const char *const sNames[] = {
        "RibbonName_Text_HoennChampion", "RibbonName_Text_HoennCool", "RibbonName_Text_HoennCoolSuper",
        "RibbonName_Text_HoennCoolHyper", "RibbonName_Text_HoennCoolMaster", "RibbonName_Text_HoennBeauty",
        "RibbonName_Text_HoennBeautySuper", "RibbonName_Text_HoennBeautyHyper", "RibbonName_Text_HoennBeautyMaster",
        "RibbonName_Text_HoennCute", "RibbonName_Text_HoennCuteSuper", "RibbonName_Text_HoennCuteHyper",
        "RibbonName_Text_HoennCuteMaster", "RibbonName_Text_HoennSmart", "RibbonName_Text_HoennSmartSuper",
        "RibbonName_Text_HoennSmartHyper", "RibbonName_Text_HoennSmartMaster", "RibbonName_Text_HoennTough",
        "RibbonName_Text_HoennToughSuper", "RibbonName_Text_HoennToughHyper", "RibbonName_Text_HoennToughMaster",
        "RibbonName_Text_HoennWinning", "RibbonName_Text_HoennVictory", "RibbonName_Text_HoennArtist",
        "RibbonName_Text_HoennEffort", "RibbonName_Text_HoennMarine", "RibbonName_Text_HoennLand",
        "RibbonName_Text_HoennSky", "RibbonName_Text_HoennCountry", "RibbonName_Text_HoennNational",
        "RibbonName_Text_HoennEarth", "RibbonName_Text_HoennWorld", "RibbonName_Text_SinnohChamp", "RibbonName_Text_Cool",
        "RibbonName_Text_CoolGreat", "RibbonName_Text_CoolUltra", "RibbonName_Text_CoolMaster", "RibbonName_Text_Beauty",
        "RibbonName_Text_BeautyGreat", "RibbonName_Text_BeautyUltra", "RibbonName_Text_BeautyMaster",
        "RibbonName_Text_Cute", "RibbonName_Text_CuteGreat", "RibbonName_Text_CuteUltra", "RibbonName_Text_CuteMaster",
        "RibbonName_Text_Smart", "RibbonName_Text_SmartGreat", "RibbonName_Text_SmartUltra",
        "RibbonName_Text_SmartMaster", "RibbonName_Text_Tough", "RibbonName_Text_ToughGreat",
        "RibbonName_Text_ToughUltra", "RibbonName_Text_ToughMaster", "RibbonName_Text_Ability",
        "RibbonName_Text_GreatAbility", "RibbonName_Text_DoubleAbility", "RibbonName_Text_MultiAbility",
        "RibbonName_Text_PairAbility", "RibbonName_Text_WorldAbility", "RibbonName_Text_Alert", "RibbonName_Text_Shock",
        "RibbonName_Text_Downcast", "RibbonName_Text_Careless", "RibbonName_Text_Relax", "RibbonName_Text_Snooze",
        "RibbonName_Text_Smile", "RibbonName_Text_Gorgeous", "RibbonName_Text_Royal", "RibbonName_Text_GorgeousRoyal",
        "RibbonName_Text_Footprint", "RibbonName_Text_Record", "RibbonName_Text_History", "RibbonName_Text_Legend",
        "RibbonName_Text_Red", "RibbonName_Text_Green", "RibbonName_Text_Blue", "RibbonName_Text_Festival",
        "RibbonName_Text_Carnival", "RibbonName_Text_Classic", "RibbonName_Text_Premier"};   // (Platinum's ribbon.c order)
    if (r->ribbon < ARRAY_COUNT(sNames))
        SetPlatinum(2, sNames[r->ribbon]);
    else
        StringCopy(Slot(2), gText_EmptyString2);
    SetNumber(3, r->count, 3);
    return "pl_msg_00000417_00004";
}
static const char *Rec_Bp(struct TvInstance *k)
{
    SetPlayer(0);
    SetNumber(1, ((struct TvCounter *)k->data)->count, 4);
    return "pl_msg_00000417_00009";
}
static bool8 Rec_BpOk(struct TvInstance *k) { return Arrived(MAPSEC_SINNOH_FIGHT_AREA); }

static const struct TvSegment sRecords[] = {
    [1] = {Rec_Tower, Rec_TowerOk},
    [3] = {Rec_Size, Rec_SizeOk},
    [4] = {Rec_Slots, NULL},
    [5] = {Rec_Ribbon, NULL},
    [10] = {Rec_Bp, Rec_BpOk},
    [12] = {NULL, NULL},
    // (8, 9: the Underground's traps and flags; 11: GTS trades: never on air)
};

// INTERVIEWS
static const char *Int_Tower(struct TvInstance *k)
{
    struct TvTowerTalk *t = (void *)k->data;
    SetNumber(0, t->streak, 4);
    SetPlayer(1);
    SetWord(2, t->word);
    return t->win ? "TVProgramInterviews_Text_BattleTowerCorner_Win" : "TVProgramInterviews_Text_BattleTowerCorner_Lose";
}
static bool8 FightAreaOk(struct TvInstance *k) { return Arrived(MAPSEC_SINNOH_FIGHT_AREA); }
static const char *Int_Pokemon(struct TvInstance *k)
{
    struct TvPokemonTalk *p = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, p->mon.species);
    SetWord(3, p->word);
    if (p->mon.hasNick)
    {
        StringCopy(Slot(2), p->mon.nick);
        return "TVProgramInterviews_Text_YourPokemonCorner_Nickname";
    }
    return "TVProgramInterviews_Text_YourPokemonCorner_NoNickname";
}
static bool8 Int_PokemonOk(struct TvInstance *k) { return Seen(((struct TvPokemonTalk *)k->data)->mon.species); }
static const char *Int_Poketch(struct TvInstance *k)
{
    struct TvPoketchTalk *p = (void *)k->data;
    SetPlayer(0);
    SetPoketchApp(1, p->app);
    SetWord(2, p->word);
    return "TVProgramInterviews_Text_ThePoketchWatch";
}
static const char *Int_Contest(struct TvInstance *k)
{
    struct TvContestTalk *c = (void *)k->data;
    SetSpecies(0, c->species);
    Sinnoh_ContestTypeName(Slot(1), c->type);
    Sinnoh_ContestRankName(Slot(2), c->rank);
    SetNumber(3, c->place, 1);
    SetPlayer(4);
    SetWord(5, c->word);
    return c->place == 1 ? "TVProgramInterviews_Text_ContestHall_Win" : "TVProgramInterviews_Text_ContestHall_Lose";
}
static bool8 Int_ContestOk(struct TvInstance *k) { return Seen(((struct TvContestTalk *)k->data)->species); }
static const char *Int_Photo(struct TvInstance *k)
{
    struct TvPokemonTalk *p = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, p->mon.species);
    SetWord(2, p->word);
    return "TVProgramInterviews_Text_RightOnPhotoCorner";
}
static const char *Int_Personality(struct TvInstance *k)
{
    static char sName[8];
    SetPlayer(0);
    // (Platinum: NormalType's message and the 16 types' after it, in Platinum's type order)
    snprintf(sName, sizeof(sName), "#I%d", ((struct TvPersonality *)k->data)->type - 1);
    return sName;
}
static const char *Int_Poffin(struct TvInstance *k)
{
    struct TvPoffinTalk *p = (void *)k->data;
    char name[24];
    SetPlayer(0);
    snprintf(name, sizeof(name), "pl_msg_00000465_%05u", p->poffin);
    SetPlatinum(1, name);
    SetWord(2, p->word);
    switch (p->poffin)
    {
    case 25: return "TVProgramInterviews_Text_ThreeCheersForPoffinCorner_RichPoffin";   // (POFFIN_TYPE_RICH..MILD)
    case 26: return "TVProgramInterviews_Text_ThreeCheersForPoffinCorner_OverripePoffin";
    case 27: return "TVProgramInterviews_Text_ThreeCheersForPoffinCorner_FoulPoffin";
    case 28: return "TVProgramInterviews_Text_ThreeCheersForPoffinCorner_MildPoffin";
    }
    return "TVProgramInterviews_Text_ThreeCheersForPoffinCorner_RegularPoffin";
}
static const char *Int_Amity(struct TvInstance *k)
{
    struct TvAmityTalk *a = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, a->mon.species);
    StringCopy(Slot(2), gNatureNamePointers[a->nature % NUM_NATURES]);
    SetWord(5, a->word);
    if (a->kind == 1)
    {
        SetItem(3, a->item);
        return "TVProgramInterviews_Text_AmitySquareWatch_FoundItem";
    }
    if (a->kind == 2)
    {
        char name[48];
        snprintf(name, sizeof(name), "pl_msg_00000386_%05d", a->accessory);
        SetPlatinum(3, name);
        return "TVProgramInterviews_Text_AmitySquareWatch_FoundAccessory";
    }
    return "TVProgramInterviews_Text_AmitySquareWatch";
}
static const char *Int_Frontier(struct TvInstance *k)
{
    struct TvFrontierTalk *f = (void *)k->data;
    SetPlayer(0);
    SetSpecies(1, f->mon.species);
    SetMonName(2, &f->mon);
    SetWord(3, f->word);
    return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Single";
}
static const char *Int_Question(struct TvInstance *k)
{
    static const char *const sQ[] = {"TVProgramInterviews_Text_InYourFaceInterview_Question1",
                                     "TVProgramInterviews_Text_InYourFaceInterview_Question2",
                                     "TVProgramInterviews_Text_InYourFaceInterview_Question3",
                                     "TVProgramInterviews_Text_InYourFaceInterview_Question4"};
    SetPlayer(0);
    SetWord(1, ((struct TvWord *)k->data)->word);
    return sQ[(k->segment - 15) & 3];
}
static const char *Int_FrontierMulti(struct TvInstance *k)
{
    struct TvFrontierMultiTalk *f = (void *)k->data;
    SetPlayer(0);
    StringCopy(Slot(1), f->name);
    SetWord(2, f->word);
    switch (f->facility)
    {
    case 4: return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Multi_BattleCastle";
    case 5: return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Multi_BattleHall";
    case 2:
    case 3: return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Multi_BattleFactory";
    case 6: return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Multi_BattleArcade";
    }
    return "TVProgramInterviews_Text_BattleFrontierFrontlineNews_Multi_BattleTower";
}

static const struct TvSegment sInterviews[] = {
    [2] = {Int_Tower, FightAreaOk},
    [4] = {Int_Pokemon, Int_PokemonOk},
    [6] = {Int_Poketch, NULL},
    [7] = {Int_Contest, Int_ContestOk},
    [9] = {Int_Photo, Int_PokemonOk},
    [10] = {Int_Personality, NULL},
    [11] = {Int_Poffin, NULL},
    [13] = {Int_Amity, NULL},
    [14] = {Int_Frontier, FightAreaOk},
    [15] = {Int_Question, NULL},
    [16] = {Int_Question, NULL},
    [17] = {Int_Question, NULL},
    [18] = {Int_Question, NULL},
    [19] = {Int_FrontierMulti, FightAreaOk},
};

// SINNOH NOW (from the world as it is)
static const char *Now_Weather(struct TvInstance *k)
{
    static const u16 sMaps[] = {MAP_SINNOH_ROUTE212_SOUTH, MAP_SINNOH_ROUTE213, MAP_SINNOH_ROUTE216,
                                MAP_SINNOH_ACUITY_LAKEFRONT, MAP_SINNOH_SNOWPOINT_CITY};
    u16 map = sMaps[Random() % ARRAY_COUNT(sMaps)];
    const struct MapHeader *h = Overworld_GetMapHeaderByGroupAndId(map >> 8, map & 0xFF);
    SetPlace(0, h->regionMapSectionId);
    switch (h->weather)
    {
    case WEATHER_SHADE:           return "TVProgramSinnohNow_Text_OnTheSpotWeather_Cloudy";
    case WEATHER_RAIN:            return "TVProgramSinnohNow_Text_OnTheSpotWeather_Raining";
    case WEATHER_DOWNPOUR:        return "TVProgramSinnohNow_Text_OnTheSpotWeather_HeavyRain";
    case WEATHER_SNOW:            return "TVProgramSinnohNow_Text_OnTheSpotWeather_Snowing";
    case WEATHER_RAIN_THUNDERSTORM: return "TVProgramSinnohNow_Text_OnTheSpotWeather_Thunderstorm";
    }
    {
        static const char *const sClear[] = {"TVProgramSinnohNow_Text_OnTheSpotWeather_Clear1",
                                             "TVProgramSinnohNow_Text_OnTheSpotWeather_Clear2",
                                             "TVProgramSinnohNow_Text_OnTheSpotWeather_Clear3",
                                             "TVProgramSinnohNow_Text_OnTheSpotWeather_Clear4"};
        return sClear[Random() % 4];
    }
}
static bool8 Always(struct TvInstance *k) { return TRUE; }
static const char *Now_BestThree(struct TvInstance *k)
{
    static const struct { u16 mapsec; const char *name; } sTowns[] = {
        {MAPSEC_SINNOH_SANDGEM_TOWN, "TVProgramSinnohNow_Text_YourTownsBestThree_SandgemTown"},
        {MAPSEC_SINNOH_FLOAROMA_TOWN, "TVProgramSinnohNow_Text_YourTownsBestThree_FloaromaTown"},
        {MAPSEC_SINNOH_SOLACEON_TOWN, "TVProgramSinnohNow_Text_YourTownsBestThree_SolaceonTown"},
        {MAPSEC_SINNOH_CELESTIC_TOWN, "TVProgramSinnohNow_Text_YourTownsBestThree_CelesticTown"},
        {MAPSEC_SINNOH_JUBILIFE_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_JubilifeCity"},
        {MAPSEC_SINNOH_CANALAVE_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_CanalaveCity"},
        {MAPSEC_SINNOH_OREBURGH_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_OreburghCity"},
        {MAPSEC_SINNOH_ETERNA_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_EternaCity"},
        {MAPSEC_SINNOH_HEARTHOME_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_HearthomeCity"},
        {MAPSEC_SINNOH_PASTORIA_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_PastoriaCity"},
        {MAPSEC_SINNOH_VEILSTONE_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_VeilstoneCity"},
        {MAPSEC_SINNOH_SUNYSHORE_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_SunyshoreCity"},
        {MAPSEC_SINNOH_SNOWPOINT_CITY, "TVProgramSinnohNow_Text_YourTownsBestThree_SnowpointCity"},
    };
    u16 mapsec = gMapHeader.regionMapSectionId;
    u32 i;
    if (mapsec == MAPSEC_SINNOH_TWINLEAF_TOWN)
    {
        SetPlayer(0);
        SetRival(1);
        return "TVProgramSinnohNow_Text_YourTownsBestThree_TwinleafTown";
    }
    if (mapsec == MAPSEC_SINNOH_SANDGEM_TOWN)
        SetCounterpart(1);
    for (i = 0; i < ARRAY_COUNT(sTowns); i++)
        if (sTowns[i].mapsec == mapsec)
            return sTowns[i].name;
    SetPlayer(0);
    SetRival(1);
    return "TVProgramSinnohNow_Text_YourTownsBestThree_WhereWillWeGo";
}
static const char *Now_Swarm(struct TvInstance *k)
{
    u16 mapsec, species;
    Sinnoh_SwarmToday(&mapsec, &species);
    SetPlace(0, mapsec);
    SetSpecies(1, species);
    return "TVProgramSinnohNow_Text_SwarmNewsFlash";
}
static bool8 Now_SwarmOk(struct TvInstance *k)
{
    u16 mapsec, species;
    return Sinnoh_SwarmToday(&mapsec, &species);
}
bool8 Sinnoh_RoamerNews(u16 *species, u16 *mapsec);   // (s3d_roamer.c)
static const char *Now_Roamer(struct TvInstance *k)
{
    u16 mapsec, species;
    if (!Sinnoh_RoamerNews(&species, &mapsec))
        return NULL;
    SetPlace(0, mapsec);
    SetSpecies(1, species);
    return "TVProgramSinnohNow_Text_RoamerNewsFlash";
}
static bool8 Now_RoamerOk(struct TvInstance *k)
{
    u16 mapsec, species;
    return Sinnoh_RoamerNews(&species, &mapsec);
}
static const char *Now_Nature(struct TvInstance *k)
{
    // (Platinum: a random personality's nature; half the time the flavor it likes, else the stat it raises: the same
    // one, spicy Attack, dry Sp. Atk, sweet Speed, bitter Sp. Def, sour Defense)
    static const u8 sStatOfRow[] = {STAT_ATK, STAT_DEF, STAT_SPEED, STAT_SPATK, STAT_SPDEF};
    static const u8 sFlavorOfRow[] = {0, 4, 2, 1, 3};
    u32 personality = Random() % 0xFFFF;
    u8 nature = GetNatureFromPersonality(personality);
    StringCopy(Slot(0), gNatureNamePointers[nature]);
    if (nature % 6 == 0)   // (HARDY, DOCILE, SERIOUS, BASHFUL, QUIRKY: 0, 6, 12, 18, 24)
        return "TVProgramSinnohNow_Text_RichBoyNatureCorner_NeutralNature";
    if (personality % 2 == 0)
    {
        SetFlavor(2, sFlavorOfRow[nature / 5]);
        return "TVProgramSinnohNow_Text_RichBoyNatureCorner_FlavorAffinity";
    }
    StringCopy(Slot(1), gStatNamesTable[sStatOfRow[nature / 5]]);
    return "TVProgramSinnohNow_Text_RichBoyNatureCorner_StatAffinity";
}

// BERRY LOOKOUT: a soil patch in a part of SINNOH reached (Platinum's sBerryLookoutPatchInfo: runs of patches by their
// number, the first arrival that opens them, the map whose name is read) and how far its BERRY has grown
static const struct { u8 count; u16 arrival, map; } sLookout[] = {
    {2, MAPSEC_SINNOH_FLOAROMA_TOWN, MAP_SINNOH_FLOAROMA_TOWN},
    {4, MAPSEC_SINNOH_FLOAROMA_TOWN, MAP_SINNOH_ROUTE205_SOUTH},
    {4, MAPSEC_SINNOH_FLOAROMA_TOWN, MAP_SINNOH_ETERNA_FOREST_OUTSIDE},
    {4, MAPSEC_SINNOH_CELESTIC_TOWN, MAP_SINNOH_FUEGO_IRONWORKS_OUTSIDE},
    {4, MAPSEC_SINNOH_FLOAROMA_TOWN, MAP_SINNOH_ROUTE205_NORTH},
    {4, MAPSEC_SINNOH_OREBURGH_CITY, MAP_SINNOH_ROUTE206},
    {4, MAPSEC_SINNOH_OREBURGH_CITY, MAP_SINNOH_ROUTE207},
    {4, MAPSEC_SINNOH_OREBURGH_CITY, MAP_SINNOH_ROUTE208},
    {4, MAPSEC_SINNOH_HEARTHOME_CITY, MAP_SINNOH_ROUTE209},
    {4, MAPSEC_SINNOH_SOLACEON_TOWN, MAP_SINNOH_SOLACEON_TOWN},
    {4, MAPSEC_SINNOH_SOLACEON_TOWN, MAP_SINNOH_ROUTE210_SOUTH},
    {4, MAPSEC_SINNOH_CELESTIC_TOWN, MAP_SINNOH_ROUTE210_NORTH},
    {4, MAPSEC_SINNOH_CELESTIC_TOWN, MAP_SINNOH_ROUTE211_EAST},
    {4, MAPSEC_SINNOH_HEARTHOME_CITY, MAP_SINNOH_ROUTE212_NORTH},
    {4, MAPSEC_SINNOH_PASTORIA_CITY, MAP_SINNOH_ROUTE212_SOUTH},
    {4, MAPSEC_SINNOH_PASTORIA_CITY, MAP_SINNOH_PASTORIA_CITY},
    {4, MAPSEC_SINNOH_PASTORIA_CITY, MAP_SINNOH_ROUTE213},
    {4, MAPSEC_SINNOH_VEILSTONE_CITY, MAP_SINNOH_ROUTE214},
    {4, MAPSEC_SINNOH_VEILSTONE_CITY, MAP_SINNOH_ROUTE215},
    {4, MAPSEC_SINNOH_CELESTIC_TOWN, MAP_SINNOH_ROUTE218},
    {4, MAPSEC_SINNOH_CELESTIC_TOWN, MAP_SINNOH_ROUTE221},
    {4, MAPSEC_SINNOH_VEILSTONE_CITY, MAP_SINNOH_ROUTE222},
    {4, MAPSEC_SINNOH_POKEMON_LEAGUE, MAP_SINNOH_ROUTE224},   // (Platinum: outside VICTORY ROAD, the LEAGUE's FLY spot)
    {4, MAPSEC_SINNOH_FIGHT_AREA, MAP_SINNOH_FIGHT_AREA},
    {4, MAPSEC_SINNOH_FIGHT_AREA, MAP_SINNOH_ROUTE225},
    {4, MAPSEC_SINNOH_SURVIVAL_AREA, MAP_SINNOH_ROUTE226},
    {4, MAPSEC_SINNOH_SURVIVAL_AREA, MAP_SINNOH_ROUTE228},
    {4, MAPSEC_SINNOH_SURVIVAL_AREA, MAP_SINNOH_ROUTE229},
    {4, MAPSEC_SINNOH_RESORT_AREA, MAP_SINNOH_RESORT_AREA},
    {4, MAPSEC_SINNOH_SURVIVAL_AREA, MAP_SINNOH_ROUTE230},
};
struct BerryTree *MP_RegionBerryTree(u8 id);   // (mp_berrytrees.c: SINNOH's patch n is tree 128 + n)
static const char *Now_BerryLookout(struct TvInstance *k)
{
    u8 patches[128];
    u32 i, j, n = 0, count = 0;
    const struct BerryTree *tree;
    const struct MapHeader *h;
    for (i = 0; i < ARRAY_COUNT(sLookout); i++)
        for (j = 0; j < sLookout[i].count; j++, n++)
            if (Sinnoh_TownVisited(sLookout[i].arrival))
                patches[count++] = n;
    if (count == 0)
        return NULL;
    n = patches[Random() % count];
    for (i = 0, j = n; j >= sLookout[i].count; i++)
        j -= sLookout[i].count;
    h = Overworld_GetMapHeaderByGroupAndId(sLookout[i].map >> 8, sLookout[i].map & 0xFF);
    SetPlace(0, h->regionMapSectionId);
    tree = MP_RegionBerryTree(128 + n);
    switch (tree ? tree->stage : BERRY_STAGE_NO_BERRY)
    {
    case BERRY_STAGE_BERRIES:   return "TVProgramSinnohNow_Text_BerryLookout_Fruit";
    case BERRY_STAGE_FLOWERING: return "TVProgramSinnohNow_Text_BerryLookout_Blooming";
    case BERRY_STAGE_TALLER:    return "TVProgramSinnohNow_Text_BerryLookout_Growing";
    case BERRY_STAGE_SPROUTED:  return "TVProgramSinnohNow_Text_BerryLookout_Sprouted";
    }
    return "TVProgramSinnohNow_Text_BerryLookout_None";
}
static bool8 Now_BerryLookoutOk(struct TvInstance *k) { return Sinnoh_TownVisited(MAPSEC_SINNOH_OREBURGH_CITY); }

// POKéMON PHOTO RATING: the dress-up photo's trainer and the title they gave it
static const char *Now_PhotoRating(struct TvInstance *k)
{
    const struct ContestEntrant *e = Sinnoh_ContestPhoto(PHOTO_STUDIO);
    if (e == NULL)
        return NULL;
    memcpy(Slot(0), e->trainer, sizeof(e->trainer));
    Slot(0)[sizeof(e->trainer) - 1] = EOS;
    SetWord(1, *Sinnoh_PhotoTitle(PHOTO_STUDIO));
    return "TVProgramSinnohNow_Text_PokemonPhotoRating";
}
static bool8 Now_PhotoRatingOk(struct TvInstance *k) { return Sinnoh_HasDressUpPhoto(); }

static const struct TvSegment sSinnohNow[] = {
    [2] = {Now_Weather, Always},
    [3] = {Now_BestThree, NULL},
    [5] = {Now_Swarm, Now_SwarmOk},
    [9] = {Now_BerryLookout, Now_BerryLookoutOk},
    [12] = {Now_Nature, NULL},
    [15] = {Now_Roamer, Now_RoamerOk},
    [17] = {Now_PhotoRating, Now_PhotoRatingOk},
    [18] = {NULL, NULL},
    // (1: groups, online only)
};

// VARIETY HOUR (its messages by number: "#V" and the message's place in the bank)
static bool8 DexOk(struct TvInstance *k) { return FlagGet(FLAG_SYS_POKEDEX_GET); }
static const char *Var_Quiz(struct TvInstance *k)
{
    static char sName[24];
    u32 n = 0;
    if (MP_Region_Cleared(REGION_SINNOH))
        n = Random() % 8;
    else if (Arrived(MAPSEC_SINNOH_HEARTHOME_CITY))
        n = Random() % 5;
    else if (Arrived(MAPSEC_SINNOH_ETERNA_CITY))
        n = Random() % 4;
    else if (Arrived(MAPSEC_SINNOH_OREBURGH_CITY))
        n = Random() % 2;
    snprintf(sName, sizeof(sName), "pl_msg_00000419_%05u", (unsigned)n);
    return sName;
}
static u16 SeenSpecies(void)
{
    u16 n = Random() % (NATIONAL_DEX_COUNT - 1) + 1, i;
    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        if (GetSetPokedexFlag(n, FLAG_GET_SEEN))
            return NationalPokedexNumToSpecies(n);
        if (++n >= NATIONAL_DEX_COUNT)
            n = 1;
    }
    return SPECIES_BIDOOF;
}
static const char *Var_Seen(struct TvInstance *k)
{
    u32 r = Random() % 3;
    SetSpecies(0, SeenSpecies());
    if (r == 1)
    {
        SetNumber(1, Random() % 34 + 25, 2);
        return "pl_msg_00000419_00009";
    }
    return r == 0 ? "pl_msg_00000419_00008" : "pl_msg_00000419_00010";
}
static const char *Var_A(struct TvInstance *k)
{
    static const char *const s[] = {"pl_msg_00000419_00011", "pl_msg_00000419_00012", "pl_msg_00000419_00013"};
    return s[Random() % 3];
}
static const char *Var_B(struct TvInstance *k)
{
    static const char *const s[] = {"pl_msg_00000419_00014", "pl_msg_00000419_00015", "pl_msg_00000419_00016"};
    return s[Random() % 3];
}
static const char *Var_Lead(struct TvInstance *k)
{
    u32 i;
    char name[48];
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            break;
    SetSpecies(0, i < PARTY_SIZE ? GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) : SPECIES_BIDOOF);
    snprintf(name, sizeof(name), "pl_msg_00000386_%05u", (unsigned)(Random() % 100));
    SetPlatinum(1, name);
    SetSpecies(2, SeenSpecies());
    StringCopy(Slot(3), gMoveNames[Random() % 465 + 1]);
    return "pl_msg_00000419_00017";
}
static const char *Var_18(struct TvInstance *k) { return "pl_msg_00000419_00018"; }
static const char *Var_19(struct TvInstance *k) { return "pl_msg_00000419_00019"; }
static bool8 Var_19Ok(struct TvInstance *k) { return FlagGet(FLAG_SINNOH_BADGE01); }
static const char *Var_Last(struct TvInstance *k)
{
    static char sName[24];
    snprintf(sName, sizeof(sName), "pl_msg_00000419_%05u", (unsigned)(20 + Random() % 9));
    return sName;
}

static const struct TvSegment sVariety[] = {
    [1] = {Var_Quiz, NULL},
    [2] = {Var_Seen, DexOk},
    [3] = {Var_A, NULL},
    [4] = {Var_B, NULL},
    [5] = {Var_Lead, DexOk},
    [6] = {Var_18, NULL},
    [7] = {Var_19, Var_19Ok},
    [8] = {Var_Last, NULL},
};

static const struct TvSegment *Segment(u8 type, u8 segment)
{
    switch (type)
    {
    case TV_INTERVIEWS: return segment < ARRAY_COUNT(sInterviews) ? &sInterviews[segment] : NULL;
    case TV_SIGHTINGS:  return segment < ARRAY_COUNT(sSightings) ? &sSightings[segment] : NULL;
    case TV_RECORDS:    return segment < ARRAY_COUNT(sRecords) ? &sRecords[segment] : NULL;
    case TV_SINNOH_NOW: return segment < ARRAY_COUNT(sSinnohNow) ? &sSinnohNow[segment] : NULL;
    case TV_VARIETY:    return segment < ARRAY_COUNT(sVariety) ? &sVariety[segment] : NULL;
    }
    return NULL;
}

static bool8 Eligible(u8 type, u8 segment)
{
    const struct TvSegment *s = Segment(type, segment);
    struct TvInstance *k = NULL;
    if (s == NULL || s->message == NULL)
        return FALSE;
    if (type <= TV_RECORDS && (k = Kept(type, segment)) == NULL)
        return FALSE;
    return s->eligible == NULL || s->eligible(k);
}

// Platinum's TVBroadcast_GetNextSegmentID: a random one of the program's segments that can air and hasn't yet this
// quarter hour (first those new to the player, then any)
static u8 NextSegment(void)
{
    u8 program = ProgramOnAir(), type = ProgramType(program), list[11], n = 0, pass, i;
    if (program == 0)
        return 0;
    for (pass = 0; pass < 2 && n == 0; pass++)
        for (i = 1; i <= 11; i++)
        {
            u8 seg = gSinnohTvPrograms[(program - 1) * 12 + i];
            struct TvInstance *k;
            if (seg == 0)
                break;
            if (Played(seg) || !Eligible(type, seg))
                continue;
            k = Kept(type, seg);
            if (pass == 0 && k != NULL && k->timesPlayed > 0)
                continue;   // (first the ones not seen yet)
            list[n++] = seg;
        }
    return n ? list[Random() % n] : 0;
}

// A message name to (bank, message): Platinum's name, or "#In" (the personality checkup's n-th type after NORMAL),
// "#n" (the n-th message after the segment's first: FIRST)
static bool8 Resolve(u8 type, const char *name, u16 *bank, u16 *msg, const char *first)
{
    if (name[0] != '#')
        return S3D_TextNamed(name, bank, msg);
    if (name[1] == 'I')
    {
        if (!S3D_TextNamed("TVProgramInterviews_Text_StreetCornerPersonalityCheckup_NormalType", bank, msg))
            return FALSE;
        *msg += atoi(name + 2);
        return TRUE;
    }
    if (first && S3D_TextNamed(first, bank, msg))
    {
        *msg += atoi(name + 1);
        return TRUE;
    }
    return FALSE;
}

static bool8 SegmentMessage(u8 segment, u16 *bank, u16 *msg)
{
    u8 program = ProgramOnAir(), type = ProgramType(program);
    const struct TvSegment *s = Segment(type, segment);
    struct TvInstance *k = Kept(type, segment);
    const char *name, *first = NULL;
    u32 i;
    for (i = 0; i < TV_MAX_PLAYED; i++)
        if (sTv.played[i] == 0)
        {
            sTv.played[i] = segment;
            break;
        }
    if (s == NULL || s->message == NULL)
        return FALSE;
    if (k && k->timesPlayed < 255)
        k->timesPlayed++;
    name = s->message(k);
    if (type == TV_SIGHTINGS && segment == 10)
        first = "TVProgramTrainerSightings_Text_RateThatNameChange_MoreAttractive";
    else if (type == TV_SIGHTINGS && segment == 21)
        first = "TVProgramTrainerSightings_Text_SealClubShow_CleverlyCalculated";
    return Resolve(type, name, bank, msg, first);
}

static const char *const sFraming[9][7] = {
#define FRAMING(p) {"TVBroadcast_Text_" #p "_GoodMorning", "TVBroadcast_Text_" #p "_GoodDay", "TVBroadcast_Text_" #p "_GoodEvening", \
                    "TVBroadcast_Text_" #p "_LeadSegment", "TVBroadcast_Text_" #p "_NextSegment", "TVBroadcast_Text_" #p "_Farewell", \
                    "TVBroadcast_Text_" #p "_FarewellExtended"}
    FRAMING(BattlingTrainers), FRAMING(TrainerResearch), FRAMING(BattleWatch), FRAMING(TrainersDay),
    FRAMING(SinnohNewsNet), FRAMING(RackEmUpRecords), FRAMING(SinnohNow), FRAMING(TrendTracker), FRAMING(VarietyHour),
#undef FRAMING
};

static bool8 Framing(u8 kind, u16 *bank, u16 *msg)
{
    u8 program = ProgramOnAir(), which;
    if (program == 0)
        return FALSE;
    switch (kind)
    {
    case 0:   // greeting (Platinum's morning 4-10, day 10-17, else evening)
        RtcCalcLocalTime();
        which = gLocalTime.hours >= 4 && gLocalTime.hours < 10 ? 0 : gLocalTime.hours >= 10 && gLocalTime.hours < 17 ? 1 : 2;
        break;
    case 1:   which = PlayedCount() == 0 ? 3 : 4; break;
    case 2:   which = 5; break;
    default:  which = 6; break;
    }
    return S3D_TextNamed(sFraming[program - 1][which], bank, msg);
}

// Commercials (Platinum's tv_commercials.c): those whose place the player has been to (closed features' never)
static bool8 Commercial(u16 *bank, u16 *msg)
{
    static const struct { const char *name; u8 when; } sAds[] = {
        // when: 0 always, 1 FLOAROMA, 2 JUBILIFE, 3 OREBURGH, 4 ETERNA, 5 HEARTHOME, 6 FIGHT AREA, 7 five badges,
        // 8 the SINNOH POKéDEX completed, 9 a POKéDEX, 10 the game completed, 11 never (closed here)
        {"TVCommercial_Text_PokemonCenter", 0}, {"TVCommercial_Text_PokemonMart", 0}, {"TVCommercial_Text_JubilifeTV", 0},
        {"TVCommercial_Text_TrainersSchool", 0}, {"TVCommercial_Text_Poketch", 0},
        {"TVCommercial_Text_PoketchCompany", 2}, {"TVCommercial_Text_PickAPeckOfColorsFlowerShop", 1},
        {"TVCommercial_Text_RadRickshawCycleShop", 4}, {"TVCommercial_Text_HearthomeCityContests", 4},
        {"TVCommercial_Text_HearthomeAmitySquare", 5}, {"TVCommercial_Text_VeilstoneDepartmentStore", 5},
        {"TVCommercial_Text_VeilstoneGameCorner", 5}, {"TVCommercial_Text_SolaceonsPokemonDayCare", 5},
        {"TVCommercial_Text_HotelGrandLake", 5}, {"TVCommercial_Text_GreatMarshSafariGame", 5},
        {"TVCommercial_Text_SnowpointCity", 7}, {"TVCommercial_Text_SunyshoreMarket", 7},
        {"TVCommercial_Text_GlobalTradeStation", 11}, {"TVCommercial_Text_PalPark", 11},
        {"TVCommercial_Text_BattleZone_1", 10}, {"TVCommercial_Text_BattleZone_2", 10},
        {"TVCommercial_Text_RibbonSyndicate", 8}, {"TVCommercial_Text_TeamGalactic_1", 5},
        {"TVCommercial_Text_TeamGalactic_2", 5}, {"TVCommercial_Text_UnionRoom", 11}, {"TVCommercial_Text_NintendoWFC", 11},
        {"TVCommercial_Text_BattleTower_1", 11}, {"TVCommercial_Text_BattleTower_2", 11},
        {"TVCommercial_Text_SSSpiral", 10}, {"TVCommercial_Text_BattleHall", 6}, {"TVCommercial_Text_BattleCastle", 6},
        {"TVCommercial_Text_BattleArcade", 6}, {"TVCommercial_Text_BattleFactory", 6},
        {"TVCommercial_Text_GlobalTerminal", 11}, {"TVCommercial_Text_WiFiPlaza", 11},
    };
    bool8 ok[12];
    u8 list[ARRAY_COUNT(sAds)], n = 0;
    u32 i;
    ok[0] = TRUE;
    ok[1] = Arrived(MAPSEC_SINNOH_FLOAROMA_TOWN);
    ok[2] = Arrived(MAPSEC_SINNOH_JUBILIFE_CITY);
    ok[3] = Arrived(MAPSEC_SINNOH_OREBURGH_CITY);
    ok[4] = Arrived(MAPSEC_SINNOH_ETERNA_CITY);
    ok[5] = Arrived(MAPSEC_SINNOH_HEARTHOME_CITY);
    ok[6] = Arrived(MAPSEC_SINNOH_FIGHT_AREA);
    ok[7] = MP_Region_BadgeCount(REGION_SINNOH) >= 5;
    ok[8] = Sinnoh_DexCompleted();
    ok[9] = FlagGet(FLAG_SYS_POKEDEX_GET);
    ok[10] = MP_Region_Cleared(REGION_SINNOH);
    ok[11] = FALSE;
    for (i = 0; i < ARRAY_COUNT(sAds); i++)
        if (ok[sAds[i].when])
            list[n++] = i;
    return S3D_TextNamed(sAds[n ? list[Random() % n] : 0].name, bank, msg);
}

// ---------------------------------------------------------------------------------------------- the interviews
// (Platinum's scrcmd_tv_broadcast.c sInterviews: its reporter's question, and whether one asks)
static const char *const sQuestions[20] = {
    [2] = "TVReporterInterviews_Text_BattleTowerCorner",
    [4] = "TVReporterInterviews_Text_YourPokemonCorner",
    [6] = "TVReporterInterviews_Text_ThePoketchWatch",
    [7] = "TVReporterInterviews_Text_ContestHall",
    [9] = "TVReporterInterviews_Text_RightOnPhotoCorner",
    [10] = "TVReporterInterviews_Text_StreetCornerPersonalityCheckup",
    [11] = "TVReporterInterviews_Text_ThreeCheersForPoffinCorner",
    [13] = "TVReporterInterviews_Text_AmitySquareWatch",
    [14] = "TVReporterInterviews_Text_BattleFrontierFrontlineNews_Single",
    [15] = "TVReporterInterviews_Text_InYourFaceInterview_Question1",
    [16] = "TVReporterInterviews_Text_InYourFaceInterview_Question2",
    [17] = "TVReporterInterviews_Text_InYourFaceInterview_Question3",
    [18] = "TVReporterInterviews_Text_InYourFaceInterview_Question4",
    [19] = "TVReporterInterviews_Text_BattleFrontierFrontlineNews_Multi",
};

// The question's own words (Platinum's interviews' loadMessageFn: slot 0)
static void QuestionSlots(u8 segment)
{
    u32 i;
    switch (segment)
    {
    case 4:
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
            {
                SetSpecies(0, GetMonData(&gPlayerParty[i], MON_DATA_SPECIES));
                break;
            }
        break;
    case 6:
        SetPoketchApp(0, Sinnoh_PoketchCurrentApp());
        break;
    case 13:
        SetSpecies(0, sTv.amity.mon.species);
        break;
    case 14:
        SetSpecies(0, sTv.frontier.mon.species);
        break;
    case 19:
        StringCopy(Slot(0), sTv.frontierMulti.name);
        break;
    }
}

static bool8 InterviewEligible(u8 segment)
{
    struct TvInstance *k = Kept(TV_INTERVIEWS, segment);
    if (segment >= ARRAY_COUNT(sQuestions) || sQuestions[segment] == NULL)
        return FALSE;
    if (k && k->timesPlayed < 3)
        return FALSE;   // (Platinum: a kept interview not yet aired three times isn't asked again)
    switch (segment)
    {
    case 2:  return sTv.tower.valid;
    case 7:  return sTv.contest.valid;
    case 6:  return CheckBagHasItem(ITEM_POKETCH, 1);
    case 9:  return Sinnoh_HasDressUpPhoto();
    case 11: return sTv.poffin.valid;
    case 13: return sTv.amity.valid;
    case 14: return sTv.frontier.valid;
    case 19: return sTv.frontierMulti.valid;
    }
    return TRUE;
}

static void InterviewSave(u8 segment, u16 word)
{
    u8 data[TV_SEGMENT_SIZE];
    memset(data, 0, sizeof(data));
    switch (segment)
    {
    case 2:
    {
        struct TvTowerTalk *t = (void *)data;
        t->win = sTv.tower.win;
        t->streak = sTv.tower.streak;
        t->word = word;
        sTv.tower.valid = FALSE;
        break;
    }
    case 4:
    case 9:
    {
        struct TvPokemonTalk *p = (void *)data;
        u32 i;
        for (i = 0; i < PARTY_SIZE; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_SANITY_HAS_SPECIES) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                break;
        if (i < PARTY_SIZE)
            MonOf(&gPlayerParty[i], &p->mon);
        if (segment == 9)
            p->mon.species = Sinnoh_DressUpPhotoSpecies();
        p->word = word;
        break;
    }
    case 6:
    {
        struct TvPoketchTalk *p = (void *)data;
        p->app = Sinnoh_PoketchCurrentApp();
        p->word = word;
        break;
    }
    case 7:
    {
        struct TvContestTalk *c = (void *)data;
        c->species = sTv.contest.species;
        c->gender = sTv.contest.gender;
        c->type = sTv.contest.type;
        c->rank = sTv.contest.rank;
        c->place = sTv.contest.place;
        c->word = word;
        sTv.contest.valid = FALSE;
        break;
    }
    case 10:
        ((struct TvPersonality *)data)->type = word;   // (the type picked from the list)
        break;
    case 11:
    {
        struct TvPoffinTalk *p = (void *)data;
        p->poffin = sTv.poffin.poffin;
        p->word = word;
        sTv.poffin.valid = FALSE;
        break;
    }
    case 13:
    {
        struct TvAmityTalk *a = (void *)data;
        a->mon = sTv.amity.mon;
        a->nature = sTv.amity.nature;
        a->kind = sTv.amity.kind;
        a->accessory = sTv.amity.accessory;
        a->item = sTv.amity.item;
        a->word = word;
        sTv.amity.valid = FALSE;
        break;
    }
    case 14:
    {
        struct TvFrontierTalk *f = (void *)data;
        f->mon = sTv.frontier.mon;
        f->word = word;
        sTv.frontier.valid = FALSE;
        break;
    }
    case 19:
    {
        struct TvFrontierMultiTalk *f = (void *)data;
        f->facility = sTv.frontierMulti.facility;
        f->gender = sTv.frontierMulti.gender;
        memcpy(f->name, sTv.frontierMulti.name, sizeof(f->name));
        f->word = word;
        sTv.frontierMulti.valid = FALSE;
        break;
    }
    default:
        ((struct TvWord *)data)->word = word;
        break;
    }
    SaveSegment(TV_INTERVIEWS, segment, data, sizeof(data));
}

// ---------------------------------------------------------------------------------------------- the script side
// `sinnoh SINNOH_TV, CALL, A, B` (s3d_scrcmd.c): Platinum's CallTVBroadcast / CallTVInterview / CheckTVInterviewEligible
enum {
    TVCALL_STATUS,         // A: variable <- 0 finished, 1 finish, 2 begin, 3 continue (Platinum's TV_BROADCAST_STATUS_*)
    TVCALL_FRAMING,        // A: bank variable, B: message variable (the kind: TVCALL_FRAMING + kind << 8)
    TVCALL_FINISH,
    TVCALL_SEGMENT_ID,     // A: the segment's variable (kept for TVCALL_SEGMENT)
    TVCALL_SEGMENT,        // A: bank variable, B: message variable <- the kept segment's
    TVCALL_COMMERCIAL,     // A: bank variable, B: message variable
    TVCALL_NEXT,           // A: variable <- the next segment (0: none)
    TVCALL_INTERVIEW_ID,   // A: the interview's variable (kept for TVCALL_INTERVIEW_MESSAGE)
    TVCALL_INTERVIEW_MESSAGE, // A: bank variable, B: message variable <- its reporter's question
    TVCALL_INTERVIEW_SAVE, // A: the interview's variable, B: the word's variable
    TVCALL_INTERVIEW_OK,   // A: the interview's variable, B: variable <- a reporter asks
    TVCALL_WORD,           // A: variable <- TRUE when a word was picked, B: variable <- the word (after Sinnoh_TvChooseWord)
    TVCALL_HIDDEN_ITEM,    // A: the item (a value or a variable)
    TVCALL_NAME_RATED,     // A: the party slot's variable
    TVCALL_STORAGE,
    TVCALL_VILLA,          // A: the furniture (a value or variable), B: TRUE for none
    TVCALL_AMITY,          // A: what (0 the POKéMON, 1 an item, 2 an accessory), B: its variable
    TVCALL_QUESTION,       // A: variable <- the ROUTE 214 gate's question of the day (0-3)
};

static u8 sSegment, sInterview;

static void SetBankMessage(u16 bankVar, u16 msgVar, bool8 ok, u16 bank, u16 msg)
{
    VarSet(bankVar, ok ? bank : 0xFFFF);
    VarSet(msgVar, ok ? msg : 0);
}

void Sinnoh_TvCall(u16 call, u16 a, u16 b)
{
    u16 bank = 0, msg = 0;
    bool8 ok;
    UpdateSlot();
    switch (call & 0xFF)
    {
    case TVCALL_STATUS:
    {
        u32 n = PlayedCount();
        VarSet(a, sTv.finished || ProgramOnAir() == 0 ? 0 : n == 0 ? 2 : n >= TV_MAX_PLAYED ? 1 : 3);
        break;
    }
    case TVCALL_FRAMING:
        ok = Framing(call >> 8, &bank, &msg);
        SetBankMessage(a, b, ok, bank, msg);
        break;
    case TVCALL_FINISH:
        sTv.finished = TRUE;
        break;
    case TVCALL_SEGMENT_ID:
        sSegment = VarGet(a);
        break;
    case TVCALL_SEGMENT:
        ok = SegmentMessage(sSegment, &bank, &msg);
        SetBankMessage(a, b, ok, bank, msg);
        MPNet_Log("sinnoh tv: program %d segment %d: message %d/%d", ProgramOnAir(), sSegment, bank, msg);
        if (ok)
        {
            static u8 sText[600];
            char ascii[600];
            StringExpandPlaceholders(sText, S3D_Text(bank, msg));
            MP_GameToAscii(ascii, sText, sizeof(ascii) - 1);
            MPNet_Log("sinnoh tv: \"%s\"", ascii);
        }
        break;
    case TVCALL_COMMERCIAL:
        ok = Commercial(&bank, &msg);
        SetBankMessage(a, b, ok, bank, msg);
        break;
    case TVCALL_NEXT:
        VarSet(a, NextSegment());
        break;
    case TVCALL_INTERVIEW_ID:
        sInterview = VarGet(a);
        break;
    case TVCALL_INTERVIEW_MESSAGE:
        ok = sInterview < ARRAY_COUNT(sQuestions) && sQuestions[sInterview] && S3D_TextNamed(sQuestions[sInterview], &bank, &msg);
        if (ok)
            QuestionSlots(sInterview);
        SetBankMessage(a, b, ok, bank, msg);
        break;
    case TVCALL_INTERVIEW_SAVE:
        InterviewSave(VarGet(a), VarGet(b));
        break;
    case TVCALL_INTERVIEW_OK:
        VarSet(b, InSinnoh() && InterviewEligible(VarGet(a)));
        break;
    case TVCALL_WORD:
        VarSet(a, sTv.word != EC_EMPTY_WORD);
        VarSet(b, sTv.word);
        break;
    case TVCALL_HIDDEN_ITEM:
        Sinnoh_TvHiddenItem(VarGet(a));
        break;
    case TVCALL_NAME_RATED:
        if (VarGet(a) < PARTY_SIZE)
            Sinnoh_TvNameRated(&gPlayerParty[VarGet(a)]);
        break;
    case TVCALL_STORAGE:
        Sinnoh_TvStorageBulletin();
        break;
    case TVCALL_VILLA:
        Sinnoh_TvVilla(VarGet(a), b);
        break;
    case TVCALL_AMITY:
        Sinnoh_TvAmity(a, VarGet(b));
        break;
    case TVCALL_QUESTION:
        if (sTv.questionDay != (u8)Today())
        {
            sTv.questionDay = Today();
            sTv.question = Random() % 4;
        }
        VarSet(a, sTv.question);
        break;
    }
}

// Special (waitstate): the reporter's one word, on the game's own easy chat screen (GABBY and TY's: "Interview")
void Sinnoh_TvChooseWord(void)
{
    sTv.word = EC_EMPTY_WORD;
    CleanupOverworldWindowsAndTilemaps();
    DoEasyChatScreen(EASY_CHAT_TYPE_GABBY_AND_TY, &sTv.word, CB2_ReturnToFieldContinueScript,
                     EASY_CHAT_PERSON_REPORTER_FEMALE);
}

// Debug (mp_debug.c): `mp tv` shows what's kept
void Sinnoh_TvDebug(void)
{
    u32 t, i;
    UpdateSlot();
    printf("[test] tv: program %d type %d played %d %d %d %d finished %d\n", ProgramOnAir(), ProgramType(ProgramOnAir()),
           sTv.played[0], sTv.played[1], sTv.played[2], sTv.played[3], sTv.finished);
    for (t = 0; t < 3; t++)
        for (i = 0; i < TV_KEPT; i++)
            if (sTv.kept[t][i].segment)
                printf("[test] tv: kept type %d segment %d played %d eligible %d species %d\n", t + 1, sTv.kept[t][i].segment,
                       sTv.kept[t][i].timesPlayed, Eligible(t + 1, sTv.kept[t][i].segment),
                       ((struct TvMon *)sTv.kept[t][i].data)->species);
}

// (tests: mp tvnow) SINNOH NOW's BERRY LOOKOUT and PHOTO RATING as they would air now: eligible, message, slots
void Sinnoh_TvDebugNow(void)
{
    static const u8 sSegs[] = {9, 17};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSegs); i++)
    {
        const struct TvSegment *s = &sSinnohNow[sSegs[i]];
        bool8 ok = s->eligible == NULL || s->eligible(NULL);
        const char *name = ok ? s->message(NULL) : NULL;
        char a[40] = "", b[40] = "";
        if (name)
        {
            MP_GameToAscii(a, Slot(0), sizeof(a));
            MP_GameToAscii(b, Slot(1), sizeof(b));
        }
        printf("[test] tv now %d: eligible %d %s [%s] [%s]\n", sSegs[i], ok, name ? name : "-", a, b);
    }
}
