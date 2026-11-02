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
#include "easy_chat.h"
#include "overworld.h"
#include "constants/game_stat.h"
#include "string_util.h"
#include "constants/characters.h"
#include "mp.h"
#include "mp_region.h"
#include "mp_tcard.h"

extern char gPlatformSavePath[512];

#define TCARD_MAGIC 0x44524354   // "TCRD"
#define TCARD_VERSION 3   // 2: friends (what was done together, the album); 3: the card's look, showcase, signature, time per region
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
    u8 friendCount;
    struct MpTCardFriend friends[TCARD_FRIENDS];   // the most recently seen first
    u32 albumSeq[TCARD_FRIENDS];                    // when each card was seen (a counter: the album's order)
    u32 albumNext;
    struct MpCardStyle style;
    struct MpCardShowcase showcase;
    bool8 hasSig;
    u8 sig[TCARD_SIG_BYTES];
    u32 regionSeconds[REGION_COUNT];
    // not kept
    u16 secondFrames, saveSeconds;
    u8 metChecked;
    u8 progress[NUM_TCARD_BADGES];      // rubs towards the next point
    u8 lastPartnerCount;
    u8 lastPartners[TCARD_PARTNERS][MP_NAME_LEN + 1];
    u32 lastPartnerFrame;
    s32 debugDays;
    u16 checkTimer;
} sT;

static void MeetFriends(void);
static void DefaultStyle(void);
static struct MpCardView sAddonView;   // (the card a friend asked for, as sent)

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
    Put(f, &sT.friendCount, 1);
    Put(f, &sT.albumNext, 4);
    for (i = 0; i < sT.friendCount; i++)
    {
        Put(f, &sT.friends[i], sizeof(sT.friends[i]));
        Put(f, &sT.albumSeq[i], 4);
    }
    Put(f, &sT.style, sizeof(sT.style));
    Put(f, &sT.showcase, sizeof(sT.showcase));
    Put(f, &sT.hasSig, 1);
    Put(f, sT.sig, sizeof(sT.sig));
    Put(f, sT.regionSeconds, sizeof(sT.regionSeconds));
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
    if (!Get(f, head, sizeof(head)) || head[0] != TCARD_MAGIC || head[1] < 1 || head[1] > TCARD_VERSION || head[2] != sT.owner)
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
    if (head[1] >= 3)
    {
        if (!Get(f, &sT.friendCount, 1) || !Get(f, &sT.albumNext, 4))
            return FALSE;
        if (sT.friendCount > TCARD_FRIENDS)
            sT.friendCount = TCARD_FRIENDS;
        for (i = 0; i < sT.friendCount; i++)
            if (!Get(f, &sT.friends[i], sizeof(sT.friends[i])) || !Get(f, &sT.albumSeq[i], 4))
            {
                sT.friendCount = i;
                break;
            }
        if (!Get(f, &sT.style, sizeof(sT.style)) || !Get(f, &sT.showcase, sizeof(sT.showcase)) || !Get(f, &sT.hasSig, 1)
         || !Get(f, sT.sig, sizeof(sT.sig)) || !Get(f, sT.regionSeconds, sizeof(sT.regionSeconds)))
            DefaultStyle();
    }
    else
        DefaultStyle();   // (an older card file: friends' records were in the old form, kept from now on)
    if (sT.fade >= TCARD_FADE_COUNT)
        sT.fade = TCARD_FADE_PLATINUM;
    return TRUE;
}

// a new card's look: RENTAL PASS BLUE, a blue accent, a thin frame, light panels, the holo shine
static void DefaultStyle(void)
{
    memset(&sT.style, 0, sizeof(sT.style));
    sT.style.design = 0;
    sT.style.accent = 6;
    sT.style.frame = TCARD_FRAME_THIN;
    sT.style.panel = TCARD_PANEL_LIGHT;
    sT.style.holo = TRUE;
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
    DefaultStyle();
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
    // play time in each region (a second every 60 frames)
    if (sT.loaded && gSaveBlock2Ptr != NULL && ++sT.secondFrames >= 60)
    {
        u8 r = MP_Region_Current();
        sT.secondFrames = 0;
        if (r < REGION_COUNT)
            sT.regionSeconds[r]++;
        sT.saveSeconds++;
    }
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
    if (sT.saveSeconds >= 300)
        changed = TRUE;
    if (changed)
    {
        sT.saveSeconds = 0;
        Save();
    }
    // friends in the room: when we first met (every few seconds)
    if (++sT.metChecked >= 10)
    {
        sT.metChecked = 0;
        MeetFriends();
    }
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

// Game text as the card's own text (ASCII, é as UTF-8: the card screens' font has it), at most size - 1 bytes
void MP_TCard_GameToText(char *dst, const u8 *src, int size)
{
    int n = 0;
    for (; *src != EOS && n < size - 1; src++)
    {
        if (*src == CHAR_e_ACUTE || *src == CHAR_E_ACUTE)
        {
            if (n + 2 > size - 1)
                break;
            dst[n++] = (char)0xc3, dst[n++] = (char)0xa9;
        }
        else
        {
            u8 one[2] = {*src, EOS};
            char c[4];
            MP_GameToAscii(c, one, 1);
            dst[n++] = c[0] ? c[0] : '?';
        }
    }
    dst[n] = 0;
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
    v->style = sT.style;
    v->showcase = sT.showcase;
    if (!sT.showcase.pad)
    {
        // never chosen on MY CARD: the party's first POKéMON
        for (i = 0; i < PARTY_SIZE; i++)
        {
            struct Pokemon *mon = &gPlayerParty[i];
            if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
                continue;
            v->showcase.species = GetMonData(mon, MON_DATA_SPECIES);
            v->showcase.shiny = IsMonShiny(mon);
            v->showcase.personality = GetMonData(mon, MON_DATA_PERSONALITY);
            v->showcase.otId = GetMonData(mon, MON_DATA_OT_ID);
            GetMonData(mon, MON_DATA_NICKNAME, v->showcase.nickname);
            v->showcase.nickname[POKEMON_NAME_LENGTH] = EOS;
            break;
        }
    }
    memcpy(v->regionSeconds, sT.regionSeconds, sizeof(v->regionSeconds));
    v->hasSig = sT.hasSig;
    memcpy(v->sig, sT.sig, sizeof(v->sig));
    {
        static const u8 sStat[TCARD_STAT_COUNT] = {
            [TCARD_STAT_CATCHES] = GAME_STAT_POKEMON_CAPTURES, [TCARD_STAT_HATCHED] = GAME_STAT_HATCHED_EGGS,
            [TCARD_STAT_EVOLVED] = GAME_STAT_EVOLVED_POKEMON, [TCARD_STAT_BATTLES] = GAME_STAT_TOTAL_BATTLES,
            [TCARD_STAT_TRAINERS] = GAME_STAT_TRAINER_BATTLES, [TCARD_STAT_WILD] = GAME_STAT_WILD_BATTLES,
            [TCARD_STAT_STEPS] = GAME_STAT_STEPS, [TCARD_STAT_FISHED] = GAME_STAT_FISHING_ENCOUNTERS,
            [TCARD_STAT_HOF] = GAME_STAT_ENTERED_HOF, [TCARD_STAT_CONTESTS] = GAME_STAT_WON_CONTEST,
        };
        u32 together = 0;
        for (i = 0; i < TCARD_STAT_ACHIEVEMENTS; i++)
            v->stats[i] = GetGameStat(sStat[i]);
        v->stats[TCARD_STAT_ACHIEVEMENTS] = MP_Achieve_DoneCount();
        for (i = 0; i < sT.friendCount; i++)
            together += sT.friends[i].wonTogether;
        v->stats[TCARD_STAT_TOGETHER] = together;
    }
    // the catchphrase: the profile's four words (Emerald's own card profile)
    for (i = 0; i < 4; i++)
    {
        u8 word[64];
        CopyEasyChatWord(word, gSaveBlock1Ptr->easyChatProfile[i]);
        MP_TCard_GameToText((char *)v->profile[i], word, sizeof(v->profile[i]));
    }
}

const struct MpCardStyle *MP_TCard_Style(void) { Check(); return &sT.style; }
void MP_TCard_SetStyle(const struct MpCardStyle *style)
{
    Check();
    sT.style = *style;
    Save();
}
const struct MpCardShowcase *MP_TCard_Showcase(void) { Check(); return &sT.showcase; }
void MP_TCard_SetShowcaseFromParty(s8 slot)
{
    Check();
    memset(&sT.showcase, 0, sizeof(sT.showcase));
    sT.showcase.pad = TRUE;   // (chosen: NONE stays none)
    if (slot >= 0 && slot < PARTY_SIZE)
    {
        struct Pokemon *mon = &gPlayerParty[slot];
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && !GetMonData(mon, MON_DATA_IS_EGG))
        {
            sT.showcase.species = GetMonData(mon, MON_DATA_SPECIES);
            sT.showcase.shiny = IsMonShiny(mon);
            sT.showcase.personality = GetMonData(mon, MON_DATA_PERSONALITY);
            sT.showcase.otId = GetMonData(mon, MON_DATA_OT_ID);
            GetMonData(mon, MON_DATA_NICKNAME, sT.showcase.nickname);
            sT.showcase.nickname[POKEMON_NAME_LENGTH] = EOS;
        }
    }
    Save();
}
const u8 *MP_TCard_Signature(bool8 *has) { Check(); *has = sT.hasSig; return sT.sig; }
void MP_TCard_SetSignature(const u8 *sig, bool8 has)
{
    Check();
    memcpy(sT.sig, sig, sizeof(sT.sig));
    sT.hasSig = has;
    Save();
}

bool8 MP_CardView_HasBadge(const struct MpCardView *v, u8 region, u8 badge)
{
    if (region >= REGION_COUNT || badge >= TCARD_BADGES_PER_REGION)
        return FALSE;
    return (v->badges >> (region * TCARD_BADGES_PER_REGION + badge)) & 1;
}

// A region's stars on the card: its CHAMPION beaten, all 8 of its badges, all 8 shining (4 sparkles)
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
    return stars;
}

// ----------------------------------------------------------------------------------------------------- friends

static bool8 SameName(const u8 *a, const u8 *b)
{
    u8 k;
    for (k = 0; k < MP_NAME_LEN; k++)
    {
        if (a[k] != b[k])
            return FALSE;
        if (a[k] == EOS)
            return TRUE;
    }
    return TRUE;
}

// a friend's record, made when first met (the oldest dropped when there are too many)
static struct MpTCardFriend *Friend(const u8 *name, u32 otId, bool8 make)
{
    u8 i;
    struct MpTCardFriend *f;
    Check();
    for (i = 0; i < sT.friendCount; i++)
        if (SameName(sT.friends[i].name, name) && (otId == 0 || sT.friends[i].otId == 0 || sT.friends[i].otId == otId))
        {
            if (otId)
                sT.friends[i].otId = otId;
            return &sT.friends[i];
        }
    if (!make)
        return NULL;
    if (sT.friendCount < TCARD_FRIENDS)
        i = sT.friendCount++;
    else
    {
        // (the one whose card was seen longest ago)
        u8 k;
        i = 0;
        for (k = 1; k < TCARD_FRIENDS; k++)
            if (sT.albumSeq[k] < sT.albumSeq[i])
                i = k;
    }
    f = &sT.friends[i];
    memset(f, 0, sizeof(*f));
    sT.albumSeq[i] = 0;
    StringCopyN(f->name, name, MP_NAME_LEN);
    f->name[MP_NAME_LEN] = EOS;
    f->otId = otId;
    f->firstMet = (u32)time(NULL);
    return f;
}

static void MeetFriends(void)
{
    u8 id;
    bool8 changed = FALSE;
    for (id = 0; id < MP_MAX_PLAYERS; id++)
        if (gMpPlayers[id].present && gMpPlayers[id].hasInfo && gMpPlayers[id].name[0] != EOS
         && Friend(gMpPlayers[id].name, gMpPlayers[id].otId, FALSE) == NULL)
        {
            Friend(gMpPlayers[id].name, gMpPlayers[id].otId, TRUE);
            changed = TRUE;
        }
    if (changed)
        Save();
}

void MP_TCard_NoteCoopWin(const u8 *seatPlayers, u8 count, u8 ownSeat)
{
    u8 seat;
    for (seat = 0; seat < count && seat < 3; seat++)
    {
        u8 id = seatPlayers[seat];
        struct MpTCardFriend *f;
        if (seat == ownSeat || id >= MP_MAX_PLAYERS)
            continue;
        f = Friend(gMpPlayers[id].name, gMpPlayers[id].otId, TRUE);
        if (f->wonTogether < 0xFFFF)
            f->wonTogether++;
    }
    Save();
}

void MP_TCard_NotePvp(const u8 *oppName, u8 outcome)
{
    struct MpTCardFriend *f = Friend(oppName, 0, TRUE);
    if (outcome == 1)   // (B_OUTCOME_WON)
        f->vsWins += f->vsWins < 0xFFFF;
    else if (outcome == 2)   // (B_OUTCOME_LOST)
        f->vsLosses += f->vsLosses < 0xFFFF;
    Save();
}

void MP_TCard_NoteTrade(u8 playerId)
{
    struct MpTCardFriend *f;
    if (playerId >= MP_MAX_PLAYERS)
        return;
    f = Friend(gMpPlayers[playerId].name, gMpPlayers[playerId].otId, TRUE);
    f->trades += f->trades < 0xFFFF;
    Save();
}

void MP_TCard_KeepFriendCard(const struct MpCardView *v)
{
    u8 name[MP_NAME_LEN + 1];
    struct MpTCardFriend *f;
    MP_AsciiToGame(name, v->name, MP_NAME_LEN);
    name[MP_NAME_LEN] = EOS;
    f = Friend(name, 0, TRUE);
    f->card = *v;
    f->card.own = FALSE;
    f->hasCard = TRUE;
    sT.albumSeq[f - sT.friends] = ++sT.albumNext;
    Save();
}

const struct MpTCardFriend *MP_TCard_FriendByName(const char *asciiName)
{
    u8 name[MP_NAME_LEN + 1];
    MP_AsciiToGame(name, asciiName, MP_NAME_LEN);
    name[MP_NAME_LEN] = EOS;
    return Friend(name, 0, FALSE);
}

u8 MP_TCard_AlbumCount(void)
{
    u8 i, n = 0;
    Check();
    for (i = 0; i < sT.friendCount; i++)
        n += sT.friends[i].hasCard;
    return n;
}

const struct MpTCardFriend *MP_TCard_AlbumFriend(u8 at)
{
    // the friends with a card, the one seen latest first
    u8 order[TCARD_FRIENDS], n = 0, i, k;
    Check();
    for (i = 0; i < sT.friendCount; i++)
        if (sT.friends[i].hasCard)
        {
            for (k = n; k > 0 && sT.albumSeq[order[k - 1]] < sT.albumSeq[i]; k--)
                order[k] = order[k - 1];
            order[k] = i;
            n++;
        }
    return at < n ? &sT.friends[order[at]] : NULL;
}

// ----------------------------------------------------------------------------------------------------- the co-op add-on
// After the TRAINER CARD a friend asks for (mp_linkx.c LX_CARD): every region's badges and their shine, the CHAMPIONs
// beaten, the card's team and its title. 1 + 4 + 32 + 1 + 1 + 12 + 1 + 24 + 1 bytes.
#define ADDON_VERSION 2   // 2: the card's look, showcase, records, time per region, catchphrase, signature

u16 MP_TCard_PackAddon(u8 *out)
{
    struct MpCardView *v = &sAddonView;
    u8 *p = out, i;
    MP_CardView_FillOwn(v);
    *p++ = ADDON_VERSION;
    *p++ = v->badges, *p++ = v->badges >> 8, *p++ = v->badges >> 16, *p++ = v->badges >> 24;
    memcpy(p, v->polish, sizeof(v->polish));
    p += sizeof(v->polish);
    *p++ = v->cleared;
    *p++ = v->teamCount;
    for (i = 0; i < PARTY_SIZE; i++)
        *p++ = v->team[i], *p++ = v->team[i] >> 8;
    *p++ = v->teamShiny;
    memcpy(p, v->title, TCARD_TITLE_LEN);
    p[TCARD_TITLE_LEN - 1] = 0;
    p += TCARD_TITLE_LEN;
    *p++ = v->gender;
    memcpy(p, &v->style, sizeof(v->style));
    p += sizeof(v->style);
    memcpy(p, &v->showcase, sizeof(v->showcase));
    p += sizeof(v->showcase);
    memcpy(p, v->stats, sizeof(v->stats));
    p += sizeof(v->stats);
    memcpy(p, v->regionSeconds, sizeof(v->regionSeconds));
    p += sizeof(v->regionSeconds);
    memcpy(p, v->profile, sizeof(v->profile));
    p += sizeof(v->profile);
    *p++ = v->hasSig;
    if (v->hasSig)
    {
        memcpy(p, v->sig, sizeof(v->sig));
        p += sizeof(v->sig);
    }
    return p - out;
}

bool8 MP_TCard_UnpackAddon(const u8 *in, u16 len, struct MpCardView *v)
{
    const u8 *p = in;
    u8 i;
    if (len < 1 + 4 + REGION_COUNT * TCARD_BADGES_PER_REGION + 1 + 1 + 2 * PARTY_SIZE + 1 + TCARD_TITLE_LEN + 1 || in[0] != ADDON_VERSION)
        return FALSE;
    p++;
    v->badges = p[0] | (p[1] << 8) | (p[2] << 16) | ((u32)p[3] << 24);
    p += 4;
    memcpy(v->polish, p, sizeof(v->polish));
    p += sizeof(v->polish);
    v->cleared = *p++;
    v->teamCount = *p++;
    if (v->teamCount > PARTY_SIZE)
        v->teamCount = PARTY_SIZE;
    for (i = 0; i < PARTY_SIZE; i++, p += 2)
    {
        v->team[i] = p[0] | (p[1] << 8);
        if (v->team[i] >= NUM_SPECIES)
            v->team[i] = SPECIES_NONE;
    }
    v->teamShiny = *p++;
    memcpy(v->title, p, TCARD_TITLE_LEN);
    v->title[TCARD_TITLE_LEN - 1] = 0;
    p += TCARD_TITLE_LEN;
    v->gender = *p++;
    if (p + sizeof(v->style) + sizeof(v->showcase) + sizeof(v->stats) + sizeof(v->regionSeconds) + sizeof(v->profile) + 1 > in + len)
        return FALSE;
    memcpy(&v->style, p, sizeof(v->style));
    p += sizeof(v->style);
    memcpy(&v->showcase, p, sizeof(v->showcase));
    p += sizeof(v->showcase);
    memcpy(v->stats, p, sizeof(v->stats));
    p += sizeof(v->stats);
    memcpy(v->regionSeconds, p, sizeof(v->regionSeconds));
    p += sizeof(v->regionSeconds);
    memcpy(v->profile, p, sizeof(v->profile));
    p += sizeof(v->profile);
    for (i = 0; i < 4; i++)
        v->profile[i][sizeof(v->profile[i]) - 1] = 0;
    v->hasSig = *p++ != 0;
    if (v->hasSig && p + sizeof(v->sig) <= in + len)
        memcpy(v->sig, p, sizeof(v->sig));
    else
        v->hasSig = FALSE;
    if (v->showcase.species >= NUM_SPECIES)
        v->showcase.species = SPECIES_NONE;
    v->showcase.nickname[POKEMON_NAME_LENGTH] = EOS;
    for (i = 0; i < sizeof(v->polish); i++)
        if (v->polish[i] > TCARD_POLISH_MAX)
            v->polish[i] = TCARD_POLISH_MAX;
    return TRUE;
}
