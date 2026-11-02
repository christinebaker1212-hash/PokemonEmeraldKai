// Moemon Co-op: the TRAINER CARD's own records, "<save>.card" (include/mp_tcard.h). Nothing in the save changes: a
// region's badges are its badge flags, and this file only adds what the card and the BADGE CASE show about them.
// A badge flag seen set for the first time is a badge just won (whichever script set it: the gyms of all four
// regions need no change): its date, the lead POKéMON and the friends of the co-op battle just before are kept.
// Badges already had when the file is first made are "before records". Polish works as Platinum's
// (pret/pokeplatinum src/trainer_case_badge_dirt.c and applications/trainer_case/main.c): 140 when won, 10 less a
// day on the PC's own clock (never the room host's shared one), rubs raise it a point at a time.
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "main.h"
#include "event_data.h"
#include "pokemon.h"
#include "pokedex.h"
#include "money.h"
#include "overworld.h"
#include "constants/game_stat.h"
#include "string_util.h"
#include "constants/characters.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_tcard.h"

extern char gPlatformSavePath[512];

#define TCARD_MAGIC 0x44524354   // "TCRD"
#define TCARD_VERSION 1
#define NUM_TCARD_BADGES (REGION_COUNT * TCARD_BADGES_PER_REGION)
// a badge counts the co-op battle that ended this many frames before it (time for the leader's words and the badge)
#define PARTNER_FRAMES (60 * 60 * 10)

// rubs per point of polish at each level (Platinum's sPolishThresholds)
static const u8 sRubsPerPoint[] = {1, 3, 4, 15, 15};

static struct
{
    bool8 loaded;
    u32 owner;                          // the save's trainer ID and secret ID: another game begun means a new file
    u32 known;                          // badges seen (bit region * 8 + badge)
    u32 lastDay;                        // the PC's day number fading counted up to (0: not yet)
    u8 slowCarry;                       // FADE SLOW: days counted towards the next 10
    u8 fade, showTitle;
    u16 title;
    struct MpTCardBadge badges[NUM_TCARD_BADGES];
    u8 teamCount;
    struct MpTCardMon team[PARTY_SIZE];
    // not kept
    u8 progress[NUM_TCARD_BADGES];      // rubs towards the next point
    u8 lastPartnerCount;
    u8 lastPartners[TCARD_PARTNERS][MP_NAME_LEN + 1];
    u32 lastPartnerFrame;
    s32 debugDays;
    u16 checkTimer;
} sT;

static u32 Owner(void)
{
    const u8 *id = gSaveBlock2Ptr->playerTrainerId;
    return id[0] | (id[1] << 8) | (id[2] << 16) | ((u32)id[3] << 24);
}

static void Path(char *out, size_t size)
{
    snprintf(out, size, "%s.card", gPlatformSavePath);
}

// the PC's local day number (with the tests' days added)
static u32 Today(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    s32 days;
    if (t == NULL)
        return 0;
    // days from a fixed point by the local date (a day changes at local midnight)
    days = (t->tm_year - 70) * 365 + (t->tm_year - 69) / 4 + t->tm_yday;
    return (u32)(days + sT.debugDays);
}

static bool8 FlagOf(u8 i)
{
    return FlagGet(MP_Region_BadgeFlag(i / TCARD_BADGES_PER_REGION, i % TCARD_BADGES_PER_REGION)) != 0;
}

static void Put(FILE *f, const void *p, size_t n) { fwrite(p, 1, n, f); }
static bool8 Get(FILE *f, void *p, size_t n) { return fread(p, 1, n, f) == n; }

static void Save(void)
{
    char path[600];
    FILE *f;
    u32 head[3] = {TCARD_MAGIC, TCARD_VERSION, sT.owner};
    u8 i;
    Path(path, sizeof(path));
    f = fopen(path, "wb");
    if (f == NULL)
        return;
    Put(f, head, sizeof(head));
    Put(f, &sT.known, 4);
    Put(f, &sT.lastDay, 4);
    Put(f, &sT.slowCarry, 1);
    Put(f, &sT.fade, 1);
    Put(f, &sT.showTitle, 1);
    Put(f, &sT.title, 2);
    for (i = 0; i < NUM_TCARD_BADGES; i++)
    {
        const struct MpTCardBadge *b = &sT.badges[i];
        Put(f, &b->when, 4);
        Put(f, &b->leadSpecies, 2);
        Put(f, &b->leadLevel, 1);
        Put(f, &b->polish, 1);
        Put(f, &b->partnerCount, 1);
        Put(f, b->partners, sizeof(b->partners));
    }
    Put(f, &sT.teamCount, 1);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        const struct MpTCardMon *m = &sT.team[i];
        Put(f, &m->species, 2);
        Put(f, &m->level, 1);
        Put(f, &m->shiny, 1);
        Put(f, &m->personality, 4);
        Put(f, m->nickname, sizeof(m->nickname));
    }
    fclose(f);
}

void MP_TCard_SaveNow(void)
{
    if (sT.loaded)
        Save();
}

static bool8 Read(FILE *f)
{
    u32 head[3];
    u8 i;
    if (!Get(f, head, sizeof(head)) || head[0] != TCARD_MAGIC || head[1] != TCARD_VERSION || head[2] != sT.owner)
        return FALSE;
    if (!Get(f, &sT.known, 4) || !Get(f, &sT.lastDay, 4) || !Get(f, &sT.slowCarry, 1) || !Get(f, &sT.fade, 1)
     || !Get(f, &sT.showTitle, 1) || !Get(f, &sT.title, 2))
        return FALSE;
    for (i = 0; i < NUM_TCARD_BADGES; i++)
    {
        struct MpTCardBadge *b = &sT.badges[i];
        if (!Get(f, &b->when, 4) || !Get(f, &b->leadSpecies, 2) || !Get(f, &b->leadLevel, 1) || !Get(f, &b->polish, 1)
         || !Get(f, &b->partnerCount, 1) || !Get(f, b->partners, sizeof(b->partners)))
            return FALSE;
        if (b->polish > TCARD_POLISH_MAX)
            b->polish = TCARD_POLISH_MAX;
        if (b->partnerCount > TCARD_PARTNERS)
            b->partnerCount = TCARD_PARTNERS;
    }
    if (!Get(f, &sT.teamCount, 1))
        return FALSE;
    if (sT.teamCount > PARTY_SIZE)
        sT.teamCount = PARTY_SIZE;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct MpTCardMon *m = &sT.team[i];
        if (!Get(f, &m->species, 2) || !Get(f, &m->level, 1) || !Get(f, &m->shiny, 1) || !Get(f, &m->personality, 4)
         || !Get(f, m->nickname, sizeof(m->nickname)))
            return FALSE;
        m->nickname[POKEMON_NAME_LENGTH] = EOS;
    }
    if (sT.fade >= TCARD_FADE_COUNT)
        sT.fade = TCARD_FADE_PLATINUM;
    return TRUE;
}

static void Load(void)
{
    char path[600];
    FILE *f;
    u8 i;
    s32 debugDays = sT.debugDays;
    memset(&sT, 0, sizeof(sT));
    sT.debugDays = debugDays;
    sT.loaded = TRUE;
    sT.owner = Owner();
    Path(path, sizeof(path));
    f = fopen(path, "rb");
    if (f != NULL)
    {
        bool8 ok = Read(f);
        fclose(f);
        if (ok)
            return;
        s32 keep = sT.debugDays;
        memset(&sT, 0, sizeof(sT));
        sT.debugDays = keep;
        sT.loaded = TRUE;
        sT.owner = Owner();
    }
    // the first time (or another game begun on this save): the badges had now are from before the records
    sT.showTitle = TRUE;
    sT.fade = TCARD_FADE_PLATINUM;
    for (i = 0; i < NUM_TCARD_BADGES; i++)
    {
        sT.badges[i].polish = TCARD_POLISH_NORMAL;
        if (FlagOf(i))
            sT.known |= 1u << i;
    }
    sT.lastDay = Today();
    Save();
}

static void Check(void)
{
    if (!sT.loaded || sT.owner != Owner())
        Load();
}

void MP_TCard_Forget(void)
{
    sT.loaded = FALSE;
}

// a badge just won: its date, the lead POKéMON and the friends of the co-op battle just before
static void Won(u8 i)
{
    struct MpTCardBadge *b = &sT.badges[i];
    u8 k;
    memset(b, 0, sizeof(*b));
    b->when = (u32)time(NULL);
    b->polish = TCARD_POLISH_NORMAL;
    for (k = 0; k < PARTY_SIZE; k++)
    {
        struct Pokemon *mon = &gPlayerParty[k];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
        {
            b->leadSpecies = GetMonData(mon, MON_DATA_SPECIES);
            b->leadLevel = GetMonData(mon, MON_DATA_LEVEL);
            break;
        }
    }
    if (sT.lastPartnerCount && gMain.vblankCounter1 - sT.lastPartnerFrame < PARTNER_FRAMES)
    {
        b->partnerCount = sT.lastPartnerCount;
        memcpy(b->partners, sT.lastPartners, sizeof(b->partners));
    }
    sT.progress[i] = 0;
    MPNet_Log("card: badge %d of region %d won (lead %d Lv%d, %d friends)", i % TCARD_BADGES_PER_REGION, i / TCARD_BADGES_PER_REGION,
              b->leadSpecies, b->leadLevel, b->partnerCount);
}

// Platinum's daily fading, by the PC's own date
static bool8 FadeDays(void)
{
    u32 today = Today();
    u32 days, points;
    u8 i;
    if (today == 0)
        return FALSE;
    if (sT.lastDay == 0 || today < sT.lastDay)   // (the clock went back: no fading for it)
    {
        sT.lastDay = today;
        return TRUE;
    }
    if (today == sT.lastDay)
        return FALSE;
    days = today - sT.lastDay;
    sT.lastDay = today;
    switch (sT.fade)
    {
    case TCARD_FADE_OFF:
        return TRUE;
    case TCARD_FADE_SLOW:
        days += sT.slowCarry;
        sT.slowCarry = days % 3;
        days /= 3;
        break;
    }
    points = days > 25 ? 250 : days * 10;
    for (i = 0; i < NUM_TCARD_BADGES; i++)
    {
        struct MpTCardBadge *b = &sT.badges[i];
        if (!(sT.known & (1u << i)))
            continue;
        b->polish = b->polish > points ? b->polish - points : 0;
    }
    return TRUE;
}

void MP_TCard_Update(void)
{
    bool8 changed = FALSE;
    u8 i;
    if (gSaveBlock2Ptr == NULL || !MP_IsInOverworld())
    {
        // in a co-op battle: who's in it (a badge won right after it keeps their names)
        if (gMain.inBattle && MP_Coop_SeatCount() > 1)
        {
            u8 seat, n = 0;
            memset(sT.lastPartners, 0, sizeof(sT.lastPartners));
            for (seat = 0; seat < MP_Coop_SeatCount() && n < TCARD_PARTNERS; seat++)
            {
                u8 id = MP_Coop_SeatPlayer(seat);
                if (seat == MP_Coop_Seat() || id >= MP_MAX_PLAYERS)
                    continue;
                StringCopyN(sT.lastPartners[n], gMpPlayers[id].name, MP_NAME_LEN);
                sT.lastPartners[n][MP_NAME_LEN] = EOS;
                n++;
            }
            sT.lastPartnerCount = n;
            sT.lastPartnerFrame = gMain.vblankCounter1;
        }
        return;
    }
    // (twice a second is plenty)
    if (++sT.checkTimer < 30)
        return;
    sT.checkTimer = 0;
    Check();
    for (i = 0; i < NUM_TCARD_BADGES; i++)
    {
        if ((sT.known & (1u << i)) || !FlagOf(i))
            continue;
        sT.known |= 1u << i;
        Won(i);
        changed = TRUE;
    }
    if (FadeDays())
        changed = TRUE;
    if (changed)
        Save();
}

bool8 MP_TCard_HasBadge(u8 region, u8 badge)
{
    if (region >= REGION_COUNT || badge >= TCARD_BADGES_PER_REGION)
        return FALSE;
    return FlagGet(MP_Region_BadgeFlag(region, badge)) != 0;
}

const struct MpTCardBadge *MP_TCard_Badge(u8 region, u8 badge)
{
    Check();
    if (region >= REGION_COUNT || badge >= TCARD_BADGES_PER_REGION)
        region = badge = 0;
    return &sT.badges[region * TCARD_BADGES_PER_REGION + badge];
}

u8 MP_TCard_PolishLevel(u8 polish)
{
    if (polish < TCARD_POLISH_DIRTY)
        return TCARD_POLISH_LEVEL_FILTHY;
    if (polish < TCARD_POLISH_NORMAL)
        return TCARD_POLISH_LEVEL_DIRTY;
    if (polish < TCARD_POLISH_2_SPARKLES)
        return TCARD_POLISH_LEVEL_NORMAL;
    if (polish < TCARD_POLISH_4_SPARKLES)
        return TCARD_POLISH_LEVEL_2_SPARKLES;
    return TCARD_POLISH_LEVEL_4_SPARKLES;
}

// Platinum's TrainerCase_PolishBadge: rubs add up to a point of polish (more of them the shinier it is)
bool8 MP_TCard_Rub(u8 region, u8 badge)
{
    u8 i = region * TCARD_BADGES_PER_REGION + badge;
    struct MpTCardBadge *b;
    u8 before;
    Check();
    if (region >= REGION_COUNT || badge >= TCARD_BADGES_PER_REGION || !MP_TCard_HasBadge(region, badge))
        return FALSE;
    b = &sT.badges[i];
    if (b->polish >= TCARD_POLISH_MAX)
        return FALSE;
    before = MP_TCard_PolishLevel(b->polish);
    if (++sT.progress[i] < sRubsPerPoint[before])
        return FALSE;
    sT.progress[i] = 0;
    b->polish++;
    return MP_TCard_PolishLevel(b->polish) != before;
}

u8 MP_TCard_Fade(void) { Check(); return sT.fade; }
void MP_TCard_SetFade(u8 fade)
{
    Check();
    if (fade < TCARD_FADE_COUNT && fade != sT.fade)
    {
        sT.fade = fade;
        sT.slowCarry = 0;
        Save();
    }
}
bool8 MP_TCard_ShowTitle(void) { Check(); return sT.showTitle; }
void MP_TCard_SetShowTitle(bool8 show)
{
    Check();
    if ((show != 0) != (sT.showTitle != 0))
    {
        sT.showTitle = show != 0;
        Save();
    }
}
u16 MP_TCard_Title(void) { Check(); return sT.title; }
void MP_TCard_SetTitle(u16 title)
{
    Check();
    if (title != sT.title)
    {
        sT.title = title;
        Save();
    }
}

u8 MP_TCard_TeamCount(void) { Check(); return sT.teamCount; }
const struct MpTCardMon *MP_TCard_TeamMon(u8 i)
{
    Check();
    return i < PARTY_SIZE ? &sT.team[i] : &sT.team[0];
}

void MP_TCard_SetTeamFromParty(const u8 *slots, u8 count)
{
    u8 i, n = 0;
    Check();
    memset(sT.team, 0, sizeof(sT.team));
    for (i = 0; i < count && i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[slots[i]];
        struct MpTCardMon *m = &sT.team[n];
        if (slots[i] >= PARTY_SIZE || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        m->species = GetMonData(mon, MON_DATA_SPECIES);
        m->level = GetMonData(mon, MON_DATA_LEVEL);
        m->shiny = IsMonShiny(mon);
        m->personality = GetMonData(mon, MON_DATA_PERSONALITY);
        GetMonData(mon, MON_DATA_NICKNAME, m->nickname);
        m->nickname[POKEMON_NAME_LENGTH] = EOS;
        n++;
    }
    sT.teamCount = n;
    Save();
}

void MP_TCard_DebugDays(s32 days)
{
    Check();
    sT.debugDays += days;
    if (FadeDays())
        Save();
}

// ----------------------------------------------------------------------------------------------------- card views

const char *MP_TCard_TitleText(void)
{
    const char *name, *desc;
    char status[8], reward[8];
    u16 t;
    Check();
    t = sT.title;
    if (!sT.showTitle || t == 0 || t > MP_Achieve_Total() || !MP_Achieve_IsDone(t - 1))
        return NULL;
    MP_Achieve_Info(t - 1, &name, &desc, status, sizeof(status), reward, sizeof(reward));
    return name;
}

void MP_CardView_FillOwn(struct MpCardView *v)
{
    u8 r, b, i;
    const char *title;
    Check();
    memset(v, 0, sizeof(*v) - sizeof(v->pic));
    v->own = TRUE;
    v->gender = gSaveBlock2Ptr->playerGender;
    MP_GameToAscii(v->name, gSaveBlock2Ptr->playerName, sizeof(v->name));
    v->trainerId = gSaveBlock2Ptr->playerTrainerId[0] | (gSaveBlock2Ptr->playerTrainerId[1] << 8);
    v->money = GetMoney(&gSaveBlock1Ptr->money);
    v->dex = GetNationalPokedexCount(FLAG_GET_CAUGHT);
    v->hours = gSaveBlock2Ptr->playTimeHours;
    v->minutes = gSaveBlock2Ptr->playTimeMinutes;
    for (r = 0; r < REGION_COUNT; r++)
    {
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
        {
            i = r * TCARD_BADGES_PER_REGION + b;
            if (MP_TCard_HasBadge(r, b))
                v->badges |= 1u << i;
            v->polish[i] = sT.badges[i].polish;
        }
        if (MP_Region_Cleared(r))
            v->cleared |= 1 << r;
    }
    v->linkWins = GetGameStat(GAME_STAT_LINK_BATTLE_WINS) > 9999 ? 9999 : GetGameStat(GAME_STAT_LINK_BATTLE_WINS);
    v->linkLosses = GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES) > 9999 ? 9999 : GetGameStat(GAME_STAT_LINK_BATTLE_LOSSES);
    v->trades = GetGameStat(GAME_STAT_POKEMON_TRADES) > 9999 ? 9999 : GetGameStat(GAME_STAT_POKEMON_TRADES);
    // the card's team: the one chosen (MY CARD), else the party as it is
    if (sT.teamCount)
    {
        for (i = 0; i < sT.teamCount; i++)
        {
            v->team[i] = sT.team[i].species;
            if (sT.team[i].shiny)
                v->teamShiny |= 1 << i;
        }
        v->teamCount = sT.teamCount;
    }
    else
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                continue;
            if (IsMonShiny(mon))
                v->teamShiny |= 1 << v->teamCount;
            v->team[v->teamCount++] = GetMonData(mon, MON_DATA_SPECIES);
        }
    }
    title = MP_TCard_TitleText();
    if (title)
        snprintf(v->title, sizeof(v->title), "%s", title);
}

bool8 MP_CardView_HasBadge(const struct MpCardView *v, u8 region, u8 badge)
{
    if (region >= REGION_COUNT || badge >= TCARD_BADGES_PER_REGION)
        return FALSE;
    return (v->badges >> (region * TCARD_BADGES_PER_REGION + badge)) & 1;
}

// A card's color by its region's stars: its CHAMPION beaten, all 8 badges, all 8 with 4 sparkles, all 32 badges;
// and a fifth (the black card) for every region's CHAMPION beaten
u8 MP_CardView_Stars(const struct MpCardView *v, u8 region)
{
    u8 stars = 0, b, all = TRUE;
    u32 mask = 0xFFu << (region * TCARD_BADGES_PER_REGION);
    if (region >= REGION_COUNT)
        return 0;
    if (v->cleared & (1 << region))
        stars++;
    if ((v->badges & mask) == mask)
    {
        stars++;
        for (b = 0; b < TCARD_BADGES_PER_REGION; b++)
            if (v->polish[region * TCARD_BADGES_PER_REGION + b] < TCARD_POLISH_4_SPARKLES)
                all = FALSE;
        if (all)
            stars++;
    }
    if (v->badges == 0xFFFFFFFFu)
        stars++;
    if ((v->cleared & 0xF) == 0xF)
        stars++;
    return stars > 5 ? 5 : stars;
}
