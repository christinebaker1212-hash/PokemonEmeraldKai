// Moemon Co-op: the game's own link features, online with friends.
//
//   * Record mixing: TV shows, Secret Bases, the Old Man's songs, Dewford's
//     trends, Battle Tower trainers, apprentices... mix by themselves while
//     friends are online (when someone joins, then every 20 minutes).
//     The game's own mixing code does
//     the work (record_mixing.c); this only moves the records around.
//   * Secret Base visits: walk into a friend's base from anywhere
//     (CO-OP > FRIENDS > a friend).
//   * Trainer Cards: look at a friend's card (same place).
//   * Day Care: a POKéMON alone in your DAY CARE can make an EGG with one alone
//     in a friend's (daycare.c). Each of you gets your own EGG.
//   * Shared roamers and weather: Latias/Latios roam the same routes for
//     everyone, and Routes 119/123's weather and the abnormal weather
//     (Kyogre/Groudon, after the game) are the same for everyone.
//   * Hall of Fame together: entering the Hall of Fame shows the teams of the
//     friends online too, and they stand with you at the end.
//
// The player with the lowest number leads the shared things.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "daycare.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "link.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "secret_base.h"
#include "sound.h"
#include "string_util.h"
#include "task.h"
#include "trainer_card.h"
#include "constants/weather.h"
#include "constants/region_map_sections.h"
#include "region_map.h"
#include "constants/flags.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/vars.h"

void MP_AsciiToGame(u8 *dst, const char *src, int maxLen);
void MP_GameToAscii(char *dst, const u8 *src, int maxLen);
void MP_Gatcha_AddPulls(s32 n);
bool8 NetLink_BeginRecordMix(u8 linkId, u8 count);
void NetLink_EndRecordMix(void);
void NetLink_FillLocalLinkPlayer(struct LinkPlayer *out);
u32 MP_RecordMix_RecordSize(void);
void MP_RecordMix_Prepare(void *dest);
void MP_RecordMix_Receive(void *records, u8 multiplayerId);
const struct SecretBase *MP_SecretBase_Own(void);
u8 MP_SecretBase_Store(const struct SecretBase *base, u8 version, u8 language);
bool8 MP_SecretBase_PrepareVisit(u8 slot);
u8 MP_Daycare_Compatibility(const struct BoxPokemon *a, const struct BoxPokemon *b);
bool8 MP_Roamer_Get(u8 *mapGroup, u8 *mapNum);
void MP_Roamer_Set(u8 mapGroup, u8 mapNum);
u16 MP_Hof_BuildTeam(u8 *out);
extern const u8 EventScript_MpVisitSecretBase[];
extern char gPlatformSavePath[512];

enum {
    LX_MIX_START = MPR_LINKX_FIRST,  // coordinator -> all: u8 round, u8 mask, u8 manual
    LX_MIX_REC,                      // -> all: u8 round, u8 mask, LinkPlayer, record (or nothing: can't mix now)
    LX_MIX_ASK,                      // -> coordinator: please mix now
    LX_CARD_REQ,                     // -> friend: show me your card
    LX_CARD,                         // -> asker: struct TrainerCard, u8 language
    LX_BASE_REQ,                     // -> friend: your Secret Base please
    LX_BASE,                         // -> asker: u8 version, u8 language, struct SecretBase
    LX_WORLD,                        // -> all: weather stage, abnormal weather, roamer
    LX_DAYCARE,                      // -> all: u8 has, struct BoxPokemon
    LX_HOF_START,                    // -> all: I'm entering the Hall of Fame
    LX_HOF_TEAM,                     // -> champion: team, u16 pic, name
    LX_LAST,
};

#define REC_MAX       0x1500
#define MIX_SETTLE    (60 * 10)        // after someone joins
#define MIX_PERIOD    (60 * 60 * 20)   // while friends are online
#define MIX_TIMEOUT   (60 * 30)
#define WORLD_PERIOD  (60 * 10)
#define DAYCARE_PERIOD (60 * 30)

static u32 sFrame;

static void Name(u8 id, char *out)
{
    MP_GameToAscii(out, gMpPlayers[id].name, PLAYER_NAME_LENGTH);
}

static bool8 FriendHere(u8 id)
{
    return id < MP_MAX_PLAYERS && id != gMpLocalId && gMpPlayers[id].present;
}

// Bit mask of everyone online (us included)
static u8 PresentMask(void)
{
    u8 i, m = 0;
    if (!MP_IsConnected())
        return 0;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i == gMpLocalId || gMpPlayers[i].present)
            m |= 1 << i;
    return m;
}

static u8 Lowest(u8 mask)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (mask & (1 << i))
            return i;
    return 0xFF;
}

static u8 BitCount(u8 m)
{
    u8 n = 0;
    while (m)
    {
        n += m & 1;
        m >>= 1;
    }
    return n;
}

static bool8 InFieldAndFree(void)
{
    return MP_IsLocalFree() && !CurMapIsSecretBase() && !gNetLinkActive;
}

// ---------------------------------------------------------------------------
// Record mixing

static struct
{
    u8 round, mask, count, myRank;
    bool8 active, manual, haveMine, applyPending;
    u8 got;                       // ranks received
    u32 startFrame;
    u8 lastMask;
    u32 nextMix;                  // coordinator: frame of the next round (0 = none)
    struct LinkPlayer players[MAX_LINK_PLAYERS];
} sMix;

static EWRAM_DATA u8 *sMixRecv = NULL;   // MAX_LINK_PLAYERS records, RecordSize apart
static EWRAM_DATA u8 *sMixSend = NULL;   // our record (+ header)

static u8 RankOf(u8 id, u8 mask)
{
    return BitCount(mask & ((1 << id) - 1));
}

static bool8 MixBuffers(void)
{
    if (MP_RecordMix_RecordSize() > REC_MAX)
        return FALSE;
    // PC memory, not the game's heap: these stay around between rounds, and taking
    // ~27 KB of gHeap for good left battles short (a move animation's graphics
    // failed to load and the game crashed)
    if (sMixRecv == NULL)
        sMixRecv = calloc(1, REC_MAX * MAX_LINK_PLAYERS);
    if (sMixSend == NULL)
        sMixSend = calloc(1, REC_MAX + 8 + sizeof(struct LinkPlayer) + 0x400);
    return sMixRecv != NULL && sMixSend != NULL;
}

static void StartMixRound(bool8 manual)
{
    u8 buf[4];
    u8 mask = PresentMask();
    if (BitCount(mask) < 2)
        return;
    buf[0] = LX_MIX_START;
    buf[1] = (u8)(sMix.round + 1);
    buf[2] = mask;
    buf[3] = manual;
    MP_SendRelay(0xFF, buf, 4);
    MP_Linkx_OnRelay(gMpLocalId, buf, 4);   // and ourselves
}

static void AbortMix(void)
{
    sMix.active = FALSE;
    sMix.applyPending = FALSE;
}

static void OnMixStart(u8 from, const u8 *data, u16 len)
{
    u8 mask, rank, count;
    u32 size = MP_RecordMix_RecordSize();
    u8 *p;
    if (len < 4 || !MixBuffers())
        return;
    mask = data[2];
    if (!(mask & (1 << gMpLocalId)))
        return;
    sMix.round = data[1];
    sMix.mask = mask;
    sMix.manual = data[3];
    sMix.count = count = BitCount(mask);
    sMix.myRank = rank = RankOf(gMpLocalId, mask);
    sMix.got = 0;
    sMix.active = TRUE;
    sMix.applyPending = FALSE;
    sMix.startFrame = sFrame;
    memset(sMixRecv, 0, REC_MAX * MAX_LINK_PLAYERS);

    // Our record, made as the link player with our rank
    p = sMixSend;
    p[0] = LX_MIX_REC;
    p[1] = sMix.round;
    p[2] = mask;
    if (count > MAX_LINK_PLAYERS || !NetLink_BeginRecordMix(rank, count))
    {
        MP_SendRelay(0xFF, p, 3);   // can't now (in a battle): everyone drops this round
        AbortMix();
        return;
    }
    MP_RecordMix_Prepare(sMixRecv + rank * size);   // straight into our own slot
    NetLink_EndRecordMix();
    NetLink_FillLocalLinkPlayer(&sMix.players[rank]);
    memcpy(p + 3, &sMix.players[rank], sizeof(struct LinkPlayer));
    memcpy(p + 3 + sizeof(struct LinkPlayer), sMixRecv + rank * size, size);
    sMix.got |= 1 << rank;
    MP_SendRelay(0xFF, p, 3 + sizeof(struct LinkPlayer) + size);
    MPNet_Log("mix: round %d, %d players, rank %d", sMix.round, count, rank);
}

static void OnMixRec(u8 from, const u8 *data, u16 len)
{
    u32 size = MP_RecordMix_RecordSize();
    u8 rank;
    if (!sMix.active || len < 3 || data[1] != sMix.round || data[2] != sMix.mask || !(sMix.mask & (1 << from)))
        return;
    if (len < 3 + sizeof(struct LinkPlayer) + size)
    {
        AbortMix();   // they can't right now
        return;
    }
    rank = RankOf(from, sMix.mask);
    memcpy(&sMix.players[rank], data + 3, sizeof(struct LinkPlayer));
    memcpy(sMixRecv + rank * size, data + 3 + sizeof(struct LinkPlayer), size);
    sMix.got |= 1 << rank;
    if (sMix.got == (1 << sMix.count) - 1)
        sMix.applyPending = TRUE;
}

static void ApplyMix(void)
{
    u8 i, before[SECRET_BASES_COUNT], newBases = 0;
    char names[48] = "", one[16], msg[120];

    for (i = 0; i < SECRET_BASES_COUNT; i++)
        before[i] = gSaveBlock1Ptr->secretBases[i].secretBaseId;
    for (i = 0; i < sMix.count; i++)
    {
        gLinkPlayers[i] = sMix.players[i];
        gLinkPlayers[i].id = i;
    }
    if (!NetLink_BeginRecordMix(sMix.myRank, sMix.count))
        return;
    MP_RecordMix_Receive(sMixRecv, sMix.myRank);
    NetLink_EndRecordMix();
    for (i = 1; i < SECRET_BASES_COUNT; i++)
        if (gSaveBlock1Ptr->secretBases[i].secretBaseId && gSaveBlock1Ptr->secretBases[i].secretBaseId != before[i])
            newBases++;
    for (i = 0; i < MP_MAX_PLAYERS; i++)
        if (i != gMpLocalId && (sMix.mask & (1 << i)) && gMpPlayers[i].present)
        {
            Name(i, one);
            if (names[0])
                strncat(names, " & ", sizeof(names) - strlen(names) - 1);
            strncat(names, one, sizeof(names) - strlen(names) - 1);
        }
    MPNet_Log("mix: applied (new bases %d)", newBases);
    snprintf(msg, sizeof(msg), "Mixed records with %s!", names);
    if (sMix.manual || newBases)
        MP_Events_QueueToast(msg);
    if (newBases)
        MP_Events_QueueToast("A new SECRET BASE appeared in your world!");
    MP_Social_Log(names, "mixed records with you");
}

static void UpdateMix(void)
{
    u8 mask = PresentMask();
    bool8 lead = mask && Lowest(mask) == (u8)gMpLocalId;

    if (mask != sMix.lastMask)
    {
        // Someone joined: mix once everyone has settled in
        if (lead && BitCount(mask) >= 2 && (mask & ~sMix.lastMask))
            sMix.nextMix = sFrame + MIX_SETTLE;
        sMix.lastMask = mask;
    }
    if (lead && BitCount(mask) >= 2)
    {
        if (sMix.nextMix == 0)
            sMix.nextMix = sFrame + MIX_PERIOD;
        if (sFrame >= sMix.nextMix && !sMix.active && InFieldAndFree())
        {
            sMix.nextMix = sFrame + MIX_PERIOD;
            StartMixRound(FALSE);
        }
    }
    else
        sMix.nextMix = 0;
    if (sMix.active && sFrame - sMix.startFrame > MIX_TIMEOUT)
        AbortMix();
    if (sMix.applyPending && InFieldAndFree() && gMain.callback2 == CB2_Overworld)
    {
        sMix.applyPending = FALSE;
        sMix.active = FALSE;
        ApplyMix();
    }
}

// ---------------------------------------------------------------------------
// Trainer cards and Secret Bases (asked for from CO-OP > FRIENDS)

static struct
{
    u8 waitKind;       // LX_CARD / LX_BASE while waiting for an answer
    u8 waitFrom;
    u32 waitStart;
    bool8 cardReady;
    u8 cardFrom;
    u16 cardPic;
    bool8 baseReady;
    u8 baseSlot;       // 0xFF: they have none / can't visit
    char baseNote[80];
} sAsk;

static struct TrainerCard sCard;
static u8 sCardLanguage;
static u16 sCardPicForCard = 0xFFFF;

u16 MP_Linkx_CardPicOr(u16 defaultPic)
{
    return sCardPicForCard != 0xFFFF ? sCardPicForCard : defaultPic;
}

static void OnCardReq(u8 from)
{
    u8 buf[1 + sizeof(struct TrainerCard) + 1];
    char name[16], msg[64];
    struct TrainerCard card;
    TrainerCard_GenerateCardForLinkPlayer(&card);
    buf[0] = LX_CARD;
    memcpy(buf + 1, &card, sizeof(card));
    buf[1 + sizeof(card)] = gGameLanguage;
    MP_SendRelay(from, buf, sizeof(buf));
    Name(from, name);
    snprintf(msg, sizeof(msg), "%s is looking at your TRAINER CARD!", name);
    MP_Events_QueueToast(msg);
}

static void OnCard(u8 from, const u8 *data, u16 len)
{
    if (sAsk.waitKind != LX_CARD || from != sAsk.waitFrom || len < 1 + sizeof(struct TrainerCard) + 1)
        return;
    memcpy(&sCard, data + 1, sizeof(sCard));
    sCardLanguage = data[1 + sizeof(sCard)];
    sAsk.cardReady = TRUE;
    MPNet_Log("linkx: got player %d's TRAINER CARD", from + 1);
    MP_Events_Did(MP_DID_CARD);
    sAsk.cardFrom = from;
    sAsk.waitKind = 0;
}

static void OnBaseReq(u8 from)
{
    u8 buf[3 + sizeof(struct SecretBase)];
    buf[0] = LX_BASE;
    buf[1] = gGameVersion;
    buf[2] = gGameLanguage;
    memcpy(buf + 3, MP_SecretBase_Own(), sizeof(struct SecretBase));
    MP_SendRelay(from, buf, sizeof(buf));
}

static void OnBase(u8 from, const u8 *data, u16 len)
{
    struct SecretBase base;
    char name[16];
    if (sAsk.waitKind != LX_BASE || from != sAsk.waitFrom || len < 3 + sizeof(struct SecretBase))
        return;
    memcpy(&base, data + 3, sizeof(base));
    Name(from, name);
    MPNet_Log("linkx: got player %d's SECRET BASE (id %d)", from + 1, base.secretBaseId);
    sAsk.waitKind = 0;
    sAsk.baseReady = TRUE;
    sAsk.baseSlot = 0xFF;
    if (base.secretBaseId == 0)
        snprintf(sAsk.baseNote, sizeof(sAsk.baseNote), "%s hasn't made a SECRET BASE yet.", name);
    else if (base.secretBaseId == gSaveBlock1Ptr->secretBases[0].secretBaseId)
        snprintf(sAsk.baseNote, sizeof(sAsk.baseNote), "%s's SECRET BASE is in the same spot as yours!", name);
    else
    {
        sAsk.baseSlot = MP_SecretBase_Store(&base, data[1], data[2]);
        if (sAsk.baseSlot == 0xFF)
            snprintf(sAsk.baseNote, sizeof(sAsk.baseNote), "Couldn't find the way to %s's SECRET BASE.", name);
    }
}

// ---------------------------------------------------------------------------
// Day Care

struct FriendDaycare
{
    bool8 has;
    struct BoxPokemon mon;
};
static struct FriendDaycare sFriendDc[MP_MAX_PLAYERS];
static u32 sDcSignature, sDcSentFrame;
static u8 sDcLastMask;

static bool8 MyLoneDaycareMon(struct BoxPokemon *out)
{
    u8 i, n = 0;
    for (i = 0; i < DAYCARE_MON_COUNT; i++)
        if (GetBoxMonData(&gSaveBlock1Ptr->daycare.mons[i].mon, MON_DATA_SANITY_HAS_SPECIES))
        {
            if (out)
                *out = gSaveBlock1Ptr->daycare.mons[i].mon;
            n++;
        }
    return n == 1;
}

bool8 MP_Linkx_FriendDaycareMon(u8 id, struct BoxPokemon *out)
{
    if (!FriendHere(id) || !sFriendDc[id].has)
        return FALSE;
    *out = sFriendDc[id].mon;
    return TRUE;
}

static void SendDaycare(void)
{
    u8 buf[2 + sizeof(struct BoxPokemon)];
    struct BoxPokemon mon;
    memset(buf, 0, sizeof(buf));
    buf[0] = LX_DAYCARE;
    buf[1] = MyLoneDaycareMon(&mon);
    if (buf[1])
        memcpy(buf + 2, &mon, sizeof(mon));
    MP_SendRelay(0xFF, buf, sizeof(buf));
    sDcSentFrame = sFrame;
}

static void UpdateDaycare(void)
{
    struct BoxPokemon mon;
    u32 sig = MyLoneDaycareMon(&mon) ? GetBoxMonData(&mon, MON_DATA_PERSONALITY) ^ 0x5A5A : 0;
    u8 mask = PresentMask();
    if (!MP_IsConnected())
        return;
    if (sig != sDcSignature || (mask & ~sDcLastMask) || sFrame - sDcSentFrame > DAYCARE_PERIOD)
    {
        sDcSignature = sig;
        SendDaycare();
    }
    sDcLastMask = mask;
}

// The EGG made with a friend's POKéMON waits next to the save ("<save>.coopegg")
static void EggPath(char *out, size_t size)
{
    snprintf(out, size, "%s.coopegg", gPlatformSavePath);
}

void MP_Linkx_SetPendingEgg(const struct Pokemon *egg, u8 friendId)
{
    char path[600], name[16], msg[120];
    u8 species[POKEMON_NAME_LENGTH + 1];
    FILE *f;
    EggPath(path, sizeof(path));
    f = fopen(path, "wb");
    if (f)
    {
        fwrite(egg, sizeof(*egg), 1, f);
        fclose(f);
    }
    Name(friendId, name);
    GetSpeciesName(species, GetBoxMonData(&sFriendDc[friendId].mon, MON_DATA_SPECIES));
    {
        char sp[16];
        MP_GameToAscii(sp, species, POKEMON_NAME_LENGTH);
        snprintf(msg, sizeof(msg), "Your DAY CARE POK~MON and %s's %s had an EGG!\nGo and see the DAY CARE MAN.", name, sp);
    }
    MP_ShowNotice(msg);
}

bool8 MP_Linkx_TakePendingEgg(struct Pokemon *egg)
{
    char path[600];
    FILE *f;
    bool8 ok = FALSE;
    EggPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (!f)
        return FALSE;
    ok = fread(egg, sizeof(*egg), 1, f) == 1;
    fclose(f);
    remove(path);
    return ok;
}

// The EGG file is there but the game wasn't saved after it was made: the
// DAY CARE MAN should still have it
static void CheckPendingEggOnLoad(void)
{
    static char sCheckedFor[512];
    char path[600];
    struct Pokemon egg;
    FILE *f;
    if (strcmp(sCheckedFor, gPlatformSavePath) == 0 || !MP_IsInOverworld())
        return;
    snprintf(sCheckedFor, sizeof(sCheckedFor), "%s", gPlatformSavePath);
    if (gSaveBlock1Ptr->daycare.offspringPersonality != 0)
        return;
    EggPath(path, sizeof(path));
    f = fopen(path, "rb");
    if (!f)
        return;
    if (fread(&egg, sizeof(egg), 1, f) == 1)
    {
        u32 p = GetMonData(&egg, MON_DATA_PERSONALITY);
        gSaveBlock1Ptr->daycare.offspringPersonality = p ? p : 1;
        FlagSet(FLAG_PENDING_DAYCARE_EGG);
    }
    fclose(f);
}

// ---------------------------------------------------------------------------
// Shared roamers and weather

static struct
{
    u8 stage;
    u16 abnormal;
    u8 flags;          // 1 game clear, 2 roamer active, 4 beat Kyogre, 8 beat Groudon
    u8 roamGroup, roamNum;
    bool8 valid;
    u32 frame;
} sWorld[MP_MAX_PLAYERS];
static u32 sWorldSent;
static u8 sWorldLastStage, sWorldLastRoam[2];
static u16 sWorldLastAbnormal;

static void SendWorld(void)
{
    u8 buf[8], g = 0, n = 0;
    bool8 roaming = MP_Roamer_Get(&g, &n);
    u16 ab = VarGet(VAR_ABNORMAL_WEATHER_LOCATION);
    buf[0] = LX_WORLD;
    buf[1] = gSaveBlock1Ptr->weatherCycleStage;
    buf[2] = ab & 0xFF;
    buf[3] = ab >> 8;
    buf[4] = (FlagGet(FLAG_SYS_GAME_CLEAR) ? 1 : 0) | (roaming ? 2 : 0)
           | (FlagGet(FLAG_DEFEATED_KYOGRE) ? 4 : 0) | (FlagGet(FLAG_DEFEATED_GROUDON) ? 8 : 0);
    buf[5] = g;
    buf[6] = n;
    MP_SendRelay(0xFF, buf, 7);
    sWorldSent = sFrame;
    sWorldLastStage = buf[1];
    sWorldLastAbnormal = ab;
    sWorldLastRoam[0] = g;
    sWorldLastRoam[1] = n;
}

static void OnWorld(u8 from, const u8 *data, u16 len)
{
    if (len < 7 || from >= MP_MAX_PLAYERS)
        return;
    sWorld[from].stage = data[1];
    sWorld[from].abnormal = data[2] | (data[3] << 8);
    sWorld[from].flags = data[4];
    sWorld[from].roamGroup = data[5];
    sWorld[from].roamNum = data[6];
    sWorld[from].valid = TRUE;
    sWorld[from].frame = sFrame;
}

// The lowest-numbered friend (below us) whose world we follow, for what `need` says
static s8 WorldLeader(u8 need)
{
    u8 i;
    for (i = 0; i < MP_MAX_PLAYERS && i < (u8)gMpLocalId; i++)
        if (FriendHere(i) && sWorld[i].valid && (sWorld[i].flags & need) == need && sFrame - sWorld[i].frame < WORLD_PERIOD * 3)
            return i;
    return -1;
}

static void UpdateWorld(void)
{
    u8 g = 0, n = 0;
    bool8 roaming;
    s8 lead;
    if (!MP_IsConnected() || !MP_IsInOverworld())
        return;
    roaming = MP_Roamer_Get(&g, &n);
    if (sFrame - sWorldSent > WORLD_PERIOD || gSaveBlock1Ptr->weatherCycleStage != sWorldLastStage
     || VarGet(VAR_ABNORMAL_WEATHER_LOCATION) != sWorldLastAbnormal || g != sWorldLastRoam[0] || n != sWorldLastRoam[1])
        SendWorld();

    // Routes 119/123 weather: everyone follows the leader
    lead = WorldLeader(0);
    if (lead >= 0 && gSaveBlock1Ptr->weatherCycleStage != sWorld[lead].stage)
        gSaveBlock1Ptr->weatherCycleStage = sWorld[lead].stage;

    // Latias/Latios: roam where the leader's does
    lead = WorldLeader(2);
    if (lead >= 0 && roaming && (g != sWorld[lead].roamGroup || n != sWorld[lead].roamNum))
    {
        MP_Roamer_Set(sWorld[lead].roamGroup, sWorld[lead].roamNum);
        sWorldLastRoam[0] = sWorld[lead].roamGroup;   // not news to send back
        sWorldLastRoam[1] = sWorld[lead].roamNum;
    }

    // Abnormal weather (after the game): the same for everyone who can still meet it
    lead = WorldLeader(1);
    if (lead >= 0 && FlagGet(FLAG_SYS_GAME_CLEAR))
    {
        u16 ab = sWorld[lead].abnormal, mine = VarGet(VAR_ABNORMAL_WEATHER_LOCATION);
        bool8 terra = ab >= TERRA_CAVE_LOCATIONS_START && ab < MARINE_CAVE_LOCATIONS_START;
        bool8 can = ab == ABNORMAL_WEATHER_NONE
                 || (terra ? !FlagGet(FLAG_DEFEATED_GROUDON) : !FlagGet(FLAG_DEFEATED_KYOGRE));
        if (can && ab != mine)
        {
            VarSet(VAR_ABNORMAL_WEATHER_LOCATION, ab);
            sWorldLastAbnormal = ab;
            if (ab != ABNORMAL_WEATHER_NONE)
                MP_Events_QueueToast(terra ? "Your friends report a heat wave somewhere in HOENN..." : "Your friends report a downpour somewhere in HOENN...");
        }
    }
}

// ---------------------------------------------------------------------------
// Hall of Fame together

#define HOF_TEAM_BYTES 120   // PARTY_SIZE x 20 (struct HallofFameMon)
#define HOF_FRIENDS 3

static struct
{
    bool8 waiting;
    u8 count;
    u8 team[HOF_FRIENDS][HOF_TEAM_BYTES];
    u8 name[HOF_FRIENDS][PLAYER_NAME_LENGTH + 1];
    u16 pic[HOF_FRIENDS];
} sHof;

void MP_Linkx_HofBegin(void)
{
    u8 b = LX_HOF_START;
    memset(&sHof, 0, sizeof(sHof));
    if (!MP_IsConnected())
        return;
    sHof.waiting = TRUE;
    MP_SendRelay(0xFF, &b, 1);
}

u8 MP_Linkx_HofFriendCount(void)
{
    return sHof.count;
}

bool8 MP_Linkx_HofFriend(u8 i, u8 *team, u8 *name, u16 *pic)
{
    if (i >= sHof.count)
        return FALSE;
    if (team)
        memcpy(team, sHof.team[i], HOF_TEAM_BYTES);
    if (name)
        StringCopyN(name, sHof.name[i], PLAYER_NAME_LENGTH + 1);
    if (pic)
        *pic = sHof.pic[i];
    return TRUE;
}

static void OnHofStart(u8 from)
{
    u8 buf[1 + HOF_TEAM_BYTES + 2 + PLAYER_NAME_LENGTH + 1];
    u16 pic = MP_LocalFrontPicOr(gSaveBlock2Ptr->playerGender == FEMALE ? TRAINER_PIC_MAY : TRAINER_PIC_BRENDAN);
    char name[16], msg[96];
    memset(buf, 0, sizeof(buf));
    buf[0] = LX_HOF_TEAM;
    if (MP_Hof_BuildTeam(buf + 1) != HOF_TEAM_BYTES)
        return;
    buf[1 + HOF_TEAM_BYTES] = pic & 0xFF;
    buf[2 + HOF_TEAM_BYTES] = pic >> 8;
    StringCopyN(buf + 3 + HOF_TEAM_BYTES, gSaveBlock2Ptr->playerName, PLAYER_NAME_LENGTH);
    buf[3 + HOF_TEAM_BYTES + PLAYER_NAME_LENGTH] = EOS;
    MP_SendRelay(from, buf, sizeof(buf));
    Name(from, name);
    {
        // A pull the first time each friend makes it (Elite Four reruns don't count)
        char key[32];
        snprintf(key, sizeof(key), "hof:%s", name);
        if (!MP_SaveData_Get(key))
        {
            MP_SaveData_Set(key, 1);
            MP_Gatcha_AddPulls(1);
            snprintf(msg, sizeof(msg), "%s entered the HALL OF FAME, with your team cheering them on!\nYou got a MOEGATCHA pull.", name);
        }
        else
            snprintf(msg, sizeof(msg), "%s entered the HALL OF FAME again, with your team cheering them on!", name);
    }
    MP_ShowNotice(msg);
    MP_Social_Log(name, "entered the HALL OF FAME!");
}

static void OnHofTeam(u8 from, const u8 *data, u16 len)
{
    u8 i = sHof.count;
    if (!sHof.waiting || i >= HOF_FRIENDS || len < 1 + HOF_TEAM_BYTES + 2 + PLAYER_NAME_LENGTH + 1)
        return;
    memcpy(sHof.team[i], data + 1, HOF_TEAM_BYTES);
    sHof.pic[i] = data[1 + HOF_TEAM_BYTES] | (data[2 + HOF_TEAM_BYTES] << 8);
    StringCopyN(sHof.name[i], data + 3 + HOF_TEAM_BYTES, PLAYER_NAME_LENGTH);
    sHof.name[i][PLAYER_NAME_LENGTH] = EOS;
    sHof.count++;
}

// ---------------------------------------------------------------------------
// Messages and the frame update

void MP_Linkx_OnRelay(u8 from, const u8 *data, u16 len)
{
    if (len < 1)
        return;
    switch (data[0])
    {
    case LX_MIX_START: OnMixStart(from, data, len); break;
    case LX_MIX_REC:   OnMixRec(from, data, len); break;
    case LX_MIX_ASK:
        if (Lowest(PresentMask()) == (u8)gMpLocalId && !sMix.active)
            StartMixRound(TRUE);
        break;
    case LX_CARD_REQ:  OnCardReq(from); break;
    case LX_CARD:      OnCard(from, data, len); break;
    case LX_BASE_REQ:  OnBaseReq(from); break;
    case LX_BASE:      OnBase(from, data, len); break;
    case LX_WORLD:     OnWorld(from, data, len); break;
    case LX_DAYCARE:
        if (from < MP_MAX_PLAYERS && len >= 2 + sizeof(struct BoxPokemon))
        {
            sFriendDc[from].has = data[1];
            memcpy(&sFriendDc[from].mon, data + 2, sizeof(struct BoxPokemon));
        }
        break;
    case LX_HOF_START: OnHofStart(from); break;
    case LX_HOF_TEAM:  OnHofTeam(from, data, len); break;
    }
}

void MP_Linkx_OnPlayerLeft(u8 id)
{
    if (id >= MP_MAX_PLAYERS)
        return;
    sFriendDc[id].has = FALSE;
    sWorld[id].valid = FALSE;
    if (sMix.active && (sMix.mask & (1 << id)))
        AbortMix();
    if (sAsk.waitKind && sAsk.waitFrom == id)
        sAsk.waitKind = 0;
}

// Every frame
void MP_Linkx_Update(void)
{
    sFrame++;
    CheckPendingEggOnLoad();
    if (!MP_IsConnected())
    {
        sMix.active = sMix.applyPending = FALSE;
        sMix.lastMask = 0;
        return;
    }
    UpdateMix();
    UpdateDaycare();
    UpdateWorld();
}

// ---------------------------------------------------------------------------
// CO-OP > FRIENDS > (a friend): GO TO, TRAINER CARD, VISIT SECRET BASE
// CO-OP > BERRY GAMES: BERRY BLENDER, BERRY CRUSH

enum { SC_FRIEND, SC_BERRY, SC_WAIT };
enum { FR_GOTO, FR_CARD, FR_BASE, FR_BACK, FR_COUNT };
enum { BG_BLENDER, BG_CRUSH, BG_BACK, BG_COUNT };

static EWRAM_DATA struct
{
    u8 taskId;
    u8 screen;
    u8 friend;
    u16 cursor;
    char note[96];
} sUi = {0};

void MP_OpenMenuPageAt(u8 page, u8 cursor);
void MP_OpenMenuAtUnion(void);

static void FriendTitle(char *out, int size)
{
    char name[16];
    Name(sUi.friend, name);
    snprintf(out, size, "%s", name);
}

static void Draw(void)
{
    char line[64];
    switch (sUi.screen)
    {
    case SC_FRIEND:
    {
        static const char *const sHelp[FR_COUNT] = {
            "Fly to the town nearest to them (towns you've been to).",
            "Look at their TRAINER CARD.",
            "Walk into their SECRET BASE, wherever you are.",
            "Back to the list of friends.",
        };
        u16 dest = MP_Friends_Destination(sUi.friend);
        char right[24] = "";
        u8 town[24];
        if (dest < MAPSEC_NONE)
        {
            GetMapName(town, dest, 0);
            MP_GameToAscii(right, town, sizeof(right) - 1);
        }
        FriendTitle(line, sizeof(line));
        MpUi_Begin(line);
        MpUi_RowAscii(FR_GOTO, sUi.cursor == FR_GOTO, "GO TO", dest < MAPSEC_NONE ? right : "-");
        MpUi_RowAscii(FR_CARD, sUi.cursor == FR_CARD, "TRAINER CARD", NULL);
        MpUi_RowAscii(FR_BASE, sUi.cursor == FR_BASE, "VISIT SECRET BASE", NULL);
        MpUi_RowAscii(FR_BACK, sUi.cursor == FR_BACK, "BACK", NULL);
        MpUi_End();
        MpUi_HelpAscii(sUi.note[0] ? sUi.note : sHelp[sUi.cursor]);
        break;
    }
    case SC_BERRY:
    {
        static const char *const sRows[BG_COUNT] = {"BERRY BLENDER", "BERRY CRUSH", "BACK"};
        static const char *const sHelp[BG_COUNT] = {
            "Blend BERRIES into POKEBLOCKS with friends, wherever you are.",
            "Mash {A} with friends to crush BERRIES into BERRY POWDER.",
            "Back to the CO-OP menu.",
        };
        u16 i;
        MpUi_Begin("BERRY GAMES");
        for (i = 0; i < BG_COUNT; i++)
            MpUi_RowAscii(i, i == sUi.cursor, sRows[i], NULL);
        MpUi_End();
        MpUi_HelpAscii(sUi.note[0] ? sUi.note : sHelp[sUi.cursor]);
        break;
    }
    case SC_WAIT:
    {
        char name[16];
        Name(sAsk.waitFrom, name);
        FriendTitle(line, sizeof(line));
        MpUi_Begin(line);
        snprintf(line, sizeof(line), "Asking %s...", name);
        MpUi_TextAscii(1, line);
        MpUi_End();
        MpUi_HelpAscii("{B} CANCEL");
        break;
    }
    }
}

static void CloseUi(u8 taskId)
{
    MpUi_Close();
    DestroyTask(taskId);
}

// Back to CO-OP > FRIENDS with this friend under the cursor
static void BackToFriends(u8 taskId)
{
    u8 ids[MP_MAX_PLAYERS], n = MP_Friends_List(ids), i, at = 0;
    for (i = 0; i < n; i++)
        if (ids[i] == sUi.friend)
            at = i;
    CloseUi(taskId);
    MP_OpenMenuPageAt(MP_MENU_PAGE_FRIENDS, at);
}

static void Task_LinkxUi(u8 taskId);

static void OpenScreen(u8 screen)
{
    sUi.screen = screen;
    sUi.cursor = 0;
    sUi.note[0] = 0;
    MpUi_Open();
    sUi.taskId = CreateTask(Task_LinkxUi, 80);
    Draw();
}

// After a friend's TRAINER CARD: back to the field and their page
static void Task_ReopenFriend(u8 taskId)
{
    if (IsWeatherNotFadingIn() == TRUE)
    {
        DestroyTask(taskId);
        UnlockPlayerFieldControls();
        OpenScreen(SC_FRIEND);
        sUi.cursor = FR_CARD;
        Draw();
    }
}

static void FieldCB_ReturnFromCard(void)
{
    LockPlayerFieldControls();
    FadeInFromBlack();
    CreateTask(Task_ReopenFriend, 80);
}

static void CB2_ReturnFromCard(void)
{
    sCardPicForCard = 0xFFFF;
    gFieldCallback = FieldCB_ReturnFromCard;
    SetMainCallback2(CB2_ReturnToField);
}

static void Task_ShowCard(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    CleanupOverworldWindowsAndTilemaps();
    CopyTrainerCardData(&gTrainerCards[1], &sCard, VERSION_EMERALD);
    gLinkPlayers[1].language = sCardLanguage;
    sCardPicForCard = MP_Avatar_RemoteFrontPic(sAsk.cardFrom);
    ShowTrainerCardInLink(1, CB2_ReturnFromCard);
}

static void UpdateWait(u8 taskId)
{
    if (sAsk.cardReady)
    {
        sAsk.cardReady = FALSE;
        CloseUi(taskId);
        LockPlayerFieldControls();
        FadeScreen(FADE_TO_BLACK, 0);
        CreateTask(Task_ShowCard, 80);
        return;
    }
    if (sAsk.baseReady)
    {
        sAsk.baseReady = FALSE;
        if (sAsk.baseSlot != 0xFF && MP_SecretBase_PrepareVisit(sAsk.baseSlot))
        {
            MP_Events_Did(MP_DID_BASE);
            CloseUi(taskId);
            ScriptContext_SetupScript(EventScript_MpVisitSecretBase);
            return;
        }
        sUi.screen = SC_FRIEND;
        sUi.cursor = FR_BASE;
        PlaySE(SE_FAILURE);
        snprintf(sUi.note, sizeof(sUi.note), "%s", sAsk.baseNote);
        Draw();
        return;
    }
    if (JOY_NEW(B_BUTTON) || sFrame - sAsk.waitStart > 60 * 8 || !sAsk.waitKind)
    {
        bool8 timeout = !JOY_NEW(B_BUTTON);
        u8 back = sAsk.waitKind == LX_BASE ? FR_BASE : FR_CARD;
        sAsk.waitKind = 0;
        sUi.screen = SC_FRIEND;
        sUi.cursor = back;
        sUi.note[0] = 0;
        if (timeout)
            snprintf(sUi.note, sizeof(sUi.note), "No answer. They may be busy right now.");
        Draw();
    }
}

static void Task_LinkxUi(u8 taskId)
{
    u16 count = sUi.screen == SC_FRIEND ? FR_COUNT : BG_COUNT;
    if (sUi.screen == SC_WAIT)
    {
        UpdateWait(taskId);
        return;
    }
    if (sUi.screen == SC_FRIEND && !gMpPlayers[sUi.friend].present)
    {
        // They went offline
        PlaySE(SE_FAILURE);
        CloseUi(taskId);
        MP_OpenMenuPageAt(MP_MENU_PAGE_FRIENDS, 0);
        return;
    }
    if (MpUi_MoveCursor(&sUi.cursor, count))
    {
        sUi.note[0] = 0;
        Draw();
        return;
    }
    if (JOY_NEW(B_BUTTON) || (JOY_NEW(A_BUTTON) && sUi.cursor == count - 1))
    {
        PlaySE(SE_SELECT);
        if (sUi.screen == SC_FRIEND)
            BackToFriends(taskId);
        else
        {
            CloseUi(taskId);
            MP_OpenMenuAtUnion();
        }
        return;
    }
    if (!JOY_NEW(A_BUTTON))
        return;
    PlaySE(SE_SELECT);
    sUi.note[0] = 0;
    if (sUi.screen == SC_BERRY)
    {
        if (!MP_IsConnected() || MP_ConnectedCount() < 2)
        {
            PlaySE(SE_FAILURE);
            snprintf(sUi.note, sizeof(sUi.note), MP_IsConnected() ? "Nobody else is online right now." : "Go online first (CO-OP menu: {COOP}).");
        }
        else if (gMain.callback2 != CB2_Overworld)
        {
            PlaySE(SE_FAILURE);
            snprintf(sUi.note, sizeof(sUi.note), "Head outside first.");
        }
        else
        {
            u8 which = sUi.cursor;
            CloseUi(taskId);
            if (which == BG_CRUSH)
                MP_Minigame_HostCrush();
            else
                MP_Minigame_HostBlender();
            return;
        }
        Draw();
        return;
    }
    switch (sUi.cursor)
    {
    case FR_GOTO:
    {
        u8 who = sUi.friend;
        if (MP_Friends_Destination(who) >= MAPSEC_NONE)
        {
            PlaySE(SE_FAILURE);
            snprintf(sUi.note, sizeof(sUi.note), "No town you've been to is near them.");
            break;
        }
        CloseUi(taskId);
        MP_Friends_GoTo(who);
        return;
    }
    case FR_CARD:
    case FR_BASE:
    {
        u8 b = sUi.cursor == FR_CARD ? LX_CARD_REQ : LX_BASE_REQ;
        if (sUi.cursor == FR_BASE && (CurMapIsSecretBase() || gMain.callback2 != CB2_Overworld))
        {
            PlaySE(SE_FAILURE);
            snprintf(sUi.note, sizeof(sUi.note), "Head outside first.");
            break;
        }
        sAsk.waitKind = sUi.cursor == FR_CARD ? LX_CARD : LX_BASE;
        sAsk.waitFrom = sUi.friend;
        sAsk.waitStart = sFrame;
        sAsk.cardReady = sAsk.baseReady = FALSE;
        MP_SendRelay(sUi.friend, &b, 1);
        sUi.screen = SC_WAIT;
        break;
    }
    }
    Draw();
}

// CO-OP > FRIENDS > a friend
void MP_Linkx_OpenFriend(u8 id)
{
    memset(&sUi, 0, sizeof(sUi));
    sUi.friend = id;
    OpenScreen(SC_FRIEND);
}

// CO-OP > BERRY GAMES (also where the BLENDER / CRUSH lobby goes back to)
void MP_Linkx_OpenUnion(void)
{
    memset(&sUi, 0, sizeof(sUi));
    OpenScreen(SC_BERRY);
}
