// Moemon Co-op: the phone's walk and training facilities, the game's side (docs/COMPANION_APP.md "Pillar 2: the walk",
// "Training facilities"; docs/PHONE_LINK.md "Phase 3").
//
// The walk: one POKéMON leaves the save for the phone ("walk" order) and the game saves at once, like a trade: the
// phone holds it. "<save>.walk" keeps it as it left (the walk's ID, the phone, where it was sent from), so a lost
// phone never means a lost POKéMON (FORGET PHONES brings it home as it left). Coming home ("home" order) the phone
// says how far it walked, what it found and what it caught; the game checks all of it against its own rules (10,000
// steps a day since it left by the PC's clock, finds from the place's own items, catches from the place's own wild
// table at its levels, 3 a day) and only then gives it: EXP as the day care gives it (to the badge cap), Friendship
// moments by the Garden's rules, a JOURNAL line, the finds to the BAG, the catches to the PC, an EGG's steps; then it
// saves at once again. A save that still holds the POKéMON the file names (closed between the two writes) drops the
// file: the walk never began.
//
// Training: a box POKéMON trains while it stays in the boxes ("<save>.train"): STAT (EVs in one stat, one every 5
// minutes, to 252 and 510), LEVEL (10 EXP a minute, to the badge cap) or REST (Friendship and PP). Gains are worked out
// when collected, from at most 24 hours since the last collection, on the PC's own clock. Taking it into the party
// collects and ends it (so a POKéMON in training never battles); the PC shows TRAINING where its item would be.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "mp_life.h"
#include "mp_region.h"
#include "event_data.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokedex.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "berry.h"
#include "constants/berry.h"
#include "save.h"
#include "mail.h"
#include "new_game.h"
#include "fieldmap.h"
#include "constants/characters.h"
#include "constants/songs.h"
#include "sound.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/vars.h"
#include "phone_proto.h"
#include "mp_phone_int.h"
#include "../data/mp_map_group_sizes.h"

u8 MP_CopyMonToPC(struct Pokemon *mon);                  // pokemon.c
void MP_ApplyDaycareExperience(struct Pokemon *mon);     // daycare.c
bool8 MP_Life_PhoneVerb(struct BoxPokemon *box, u8 verb, u16 item, u8 *out);   // mp_life_interact.c
u8 MP_Events_PhoneGoals(char out[][64], s32 *progress, s32 *target, u8 max);    // mp_events.c
extern char gPlatformSavePath[512];

#define WALK_MAGIC    0x4B4C574D   // "MWLK"
#define WALK_VERSION  1
#define STEPS_A_DAY   10000
#define STEPS_A_FIND  2000
#define STEPS_A_MEET  3000
#define STEPS_A_MOMENT 2500
#define MEETS_A_DAY   3
#define SCR_SETORCOPYVAR 0x1A

struct PhoneWalk
{
    u32 magic, version, id, sentAt;
    u8 phone[PHONE_ID_SIZE];
    u16 mapsec;
    u8 group, num, region, cap, fromBox, fromSlot;   // (fromSlot 0xFF: the party)
    u32 pid, ot;
    struct BoxPokemon mon;
};

static struct PhoneWalk sWalk;
static bool8 sWalking;

static void FilePath(char *out, size_t size, const char *ext)
{
    snprintf(out, size, "%s.%s", gPlatformSavePath, ext);
}

static bool8 WalkWrite(void)
{
    char path[600];
    FILE *f;
    bool8 ok;
    FilePath(path, sizeof(path), "walk");
    f = fopen(path, "wb");
    if (f == NULL)
        return FALSE;
    ok = fwrite(&sWalk, sizeof(sWalk), 1, f) == 1;
    ok = fflush(f) == 0 && ok;
    return fclose(f) == 0 && ok;
}

static void WalkErase(void)
{
    char path[600];
    FilePath(path, sizeof(path), "walk");
    remove(path);
}

// Is this POKéMON (personality, Trainer ID) anywhere in the save: the party or the player's own boxes?
static bool8 InSave(u32 pid, u32 ot)
{
    struct PokemonStorage *st = Storage();
    u32 b, i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == pid && GetMonData(&gPlayerParty[i], MON_DATA_OT_ID) == ot)
            return TRUE;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (i = 0; i < IN_BOX_COUNT; i++)
            if (GetBoxMonData(&st->boxes[b][i], MON_DATA_SPECIES) != SPECIES_NONE
             && GetBoxMonData(&st->boxes[b][i], MON_DATA_PERSONALITY) == pid && GetBoxMonData(&st->boxes[b][i], MON_DATA_OT_ID) == ot)
                return TRUE;
    return FALSE;
}

// The game saved now, as a trade saves (FALSE: it couldn't, or a new game over an older save that only a save by hand
// may replace)
static bool8 SaveNow(void)
{
    if (gDifferentSaveFile && gSaveFileStatus != SAVE_STATUS_EMPTY && gSaveFileStatus != SAVE_STATUS_CORRUPT)
        return FALSE;
    SaveMapView();
    return TrySavingData(SAVE_NORMAL) == SAVE_STATUS_OK;
}

static void Notice(const u8 *nick, const char *what)
{
    char name[24], msg[96];
    MP_GameToAscii(name, nick, POKEMON_NAME_LENGTH);
    snprintf(msg, sizeof(msg), "%s %s", name, what);
    MP_Events_QueueToast(msg);   // (a toast on the field: never in the way of play)
}

// ------------------------------------------------------------------------------------------- the walk's pools

static const struct MapHeader *Header(u32 group, u32 num)
{
    return group < MAP_GROUPS_COUNT && num < sMapGroupSizes[group] ? Overworld_GetMapHeaderByGroupAndId(group, num) : NULL;
}

// The wild POKéMON of where the walk began, as the game would make them now (the GENERATION's pools): its own map's,
// else its map section's; [species, min level, max level]
static u32 WildPool(u16 out[][3], u32 max)
{
    static const u8 sSlots[4] = {12, 5, 5, 10};
    u32 pass, i, k, j, n = 0;
    for (pass = 0; pass < 2 && n == 0; pass++)
        for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        {
            const struct WildPokemonHeader *w = &gWildMonHeaders[i];
            const struct WildPokemonInfo *infos[4] = {w->landMonsInfo, w->waterMonsInfo, w->rockSmashMonsInfo, w->fishingMonsInfo};
            const struct MapHeader *h = Header(w->mapGroup, w->mapNum);
            if (pass == 0 ? (w->mapGroup != sWalk.group || w->mapNum != sWalk.num) : (h == NULL || h->regionMapSectionId != sWalk.mapsec))
                continue;
            for (k = 0; k < 4; k++)
            {
                const struct WildPokemonInfo *info = infos[k] ? MP_Region_WildInfo(infos[k]) : NULL;
                for (j = 0; info != NULL && j < sSlots[k]; j++)
                {
                    u16 sp = MP_WildSlotSpecies(info, j), d;
                    u8 lo = info->wildPokemon[j].minLevel, hi = info->wildPokemon[j].maxLevel;
                    if (sp == SPECIES_NONE || sp >= NUM_SPECIES)
                        continue;
                    for (d = 0; d < n && out[d][0] != sp; d++)
                        ;
                    if (d < n)
                    {
                        out[d][1] = lo < out[d][1] ? lo : out[d][1];
                        out[d][2] = hi > out[d][2] ? hi : out[d][2];
                    }
                    else if (n < max)
                    {
                        out[n][0] = sp, out[n][1] = lo, out[n][2] = hi;
                        n++;
                    }
                }
            }
        }
    return n;
}

static bool8 Has(const u16 *list, u32 n, u16 v)
{
    while (n--)
        if (*list++ == v)
            return TRUE;
    return FALSE;
}

// What a walk can find: the items of the place it began (its item balls and hidden items), and the things any
// POKéMON picks up on the way (PICKUP's first finds)
static u32 FindPool(u16 *out, u32 max)
{
    static const u16 sPickup[] = {ITEM_POTION, ITEM_ANTIDOTE, ITEM_SUPER_POTION, ITEM_GREAT_BALL, ITEM_REPEL,
                                  ITEM_ESCAPE_ROPE, ITEM_FULL_HEAL, ITEM_ULTRA_BALL, ITEM_ORAN_BERRY};
    u32 g, m, i, n = 0;
    for (g = 0; g < MAP_GROUPS_COUNT; g++)
        for (m = 0; m < sMapGroupSizes[g]; m++)
        {
            const struct MapHeader *h = Header(g, m);
            if (h == NULL || h->regionMapSectionId != sWalk.mapsec || h->events == NULL)
                continue;
            for (i = 0; i < h->events->objectEventCount && n < max; i++)
            {
                const struct ObjectEventTemplate *o = &h->events->objectEvents[i];
                const u8 *s = o->script;
                u16 item;
                if (TEMPLATE_GFX_ID(o) != OBJ_EVENT_GFX_ITEM_BALL || s == NULL || s[0] != SCR_SETORCOPYVAR
                 || (s[1] | (s[2] << 8)) != VAR_0x8000)
                    continue;
                item = s[3] | (s[4] << 8);
                if (item != ITEM_NONE && item < ITEMS_COUNT && GetItemPocket(item) != POCKET_KEY_ITEMS && !Has(out, n, item))
                    out[n++] = item;
            }
            for (i = 0; i < h->events->bgEventCount && n < max; i++)
            {
                const struct BgEvent *e = &h->events->bgEvents[i];
                u16 item = e->bgUnion.hiddenItem.item;
                if (e->kind == BG_EVENT_HIDDEN_ITEM && item != ITEM_NONE && item < ITEMS_COUNT
                 && GetItemPocket(item) != POCKET_KEY_ITEMS && !Has(out, n, item))
                    out[n++] = item;
            }
        }
    for (i = 0; i < ARRAY_COUNT(sPickup) && n < max; i++)
        if (!Has(out, n, sPickup[i]))
            out[n++] = sPickup[i];
    return n;
}

static void BufWalk(struct Buf *b)
{
    static u16 sWild[40][3], sFinds[48];
    u32 i, n;
    BufAdd(b, "\"walk\":{\"id\":%u,\"sent\":%u,\"now\":%u,\"mapsec\":%u,\"region\":%u,\"cap\":%u,\"mon\":", (unsigned)sWalk.id,
           (unsigned)sWalk.sentAt, (unsigned)time(NULL), sWalk.mapsec, sWalk.region, sWalk.cap);
    BufBoxMon(b, &sWalk.mon);
    n = WildPool(sWild, ARRAY_COUNT(sWild));
    BufAdd(b, ",\"wild\":[");
    for (i = 0; i < n; i++)
        BufAdd(b, "%s[%u,%u,%u]", i ? "," : "", sWild[i][0], sWild[i][1], sWild[i][2]);
    n = FindPool(sFinds, ARRAY_COUNT(sFinds));
    BufAdd(b, "],\"finds\":[");
    for (i = 0; i < n; i++)
        BufAdd(b, "%s%u", i ? "," : "", sFinds[i]);
    BufAdd(b, "],\"rules\":[%u,%u,%u,%u,%u]}", STEPS_A_DAY, STEPS_A_FIND, STEPS_A_MEET, STEPS_A_MOMENT, MEETS_A_DAY);
}

// --------------------------------------------------------------------------------------------- JSON numbers

// The numbers in the array after "key" (nested arrays flattened), up to max; -1 when there's no such array
static int JsonNums(const char *json, const char *key, long *out, int max)
{
    char pat[40];
    const char *p;
    int n = 0, depth = 0;
    snprintf(pat, sizeof(pat), "\"%s\"", key);
    p = strstr(json, pat);
    if (p == NULL)
        return -1;
    p = strchr(p + strlen(pat), '[');
    if (p == NULL)
        return -1;
    for (; *p; p++)
    {
        if (*p == '[')
            depth++;
        else if (*p == ']')
        {
            if (--depth == 0)
                break;
        }
        else if ((*p >= '0' && *p <= '9') || *p == '-')
        {
            char *end;
            long v = strtol(p, &end, 10);
            if (n < max)
                out[n++] = v;
            p = end - 1;
        }
    }
    return n;
}

// ---------------------------------------------------------------------------------------------------- orders

static const char *DoWalk(const char *json)
{
    struct Loc at;
    long long p = 0, o = 0;
    struct BoxPokemon *box;
    u16 item;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &p) || !JsonNum(json, "o", &o))
        return "bad";
    if (sWalking)
        return "walking";
    if (Empty(&at))
        return "gone";
    if (!Is(&at, (u32)p, (u32)o))
        return "changed";
    if (MP_Phone_OnOffer((u32)p, (u32)o))
        return "onoffer";
    box = BoxAt(&at);
    if (at.kind == LOC_PARTY && !GetBoxMonData(box, MON_DATA_IS_EGG) && NonEggsInParty() <= 1)
        return "last";
    item = GetBoxMonData(box, MON_DATA_HELD_ITEM);
    if (ItemIsMail(item))
        return "mail";
    {
        // a held HM stays with the player: back to the BAG first
        static struct Pokemon sTmp;
        BoxMonToMon(box, &sTmp);
        if (MP_ReturnHeldHm(&sTmp))
        {
            item = ITEM_NONE;
            SetBoxMonData(box, MON_DATA_HELD_ITEM, &item);
            sBagChanged = TRUE;
        }
    }
    memset(&sWalk, 0, sizeof(sWalk));
    sWalk.magic = WALK_MAGIC;
    sWalk.version = WALK_VERSION;
    PhoneProto_Random((u8 *)&sWalk.id, sizeof(sWalk.id));
    sWalk.id |= 1;
    sWalk.sentAt = (u32)time(NULL);
    memcpy(sWalk.phone, PhoneOrderPhone(), PHONE_ID_SIZE);
    sWalk.mapsec = gMapHeader.regionMapSectionId;
    sWalk.group = gSaveBlock1Ptr->location.mapGroup;
    sWalk.num = gSaveBlock1Ptr->location.mapNum;
    sWalk.region = MP_Region_Current();
    sWalk.cap = MP_RareCandyCap();
    sWalk.fromBox = at.kind == LOC_BOX ? at.box : 0xFF;
    sWalk.fromSlot = at.kind == LOC_BOX ? at.slot : 0xFF;
    sWalk.pid = (u32)p;
    sWalk.ot = (u32)o;
    sWalk.mon = *box;
    // the file first, then the save without it: either the walk is written down or it never began
    if (!WalkWrite())
        return "cantsave";
    if (at.kind == LOC_PARTY)
    {
        ZeroMonData(&gPlayerParty[at.slot]);
        CompactPartySlots();
        CalculatePlayerPartyCount();
        PartyChanged();
    }
    else
    {
        ZeroBoxMonData(box);
        Changed(&at);
    }
    if (!SaveNow())
    {
        // (couldn't save: it stays)
        if (at.kind == LOC_PARTY)
        {
            BoxMonToMon(&sWalk.mon, &gPlayerParty[gPlayerPartyCount]);
            CalculatePlayerPartyCount();
        }
        else
            *box = sWalk.mon;
        WalkErase();
        return "cantsave";
    }
    sWalking = TRUE;
    {
        u8 nick[POKEMON_NAME_LENGTH + 1];
        GetBoxMonData(&sWalk.mon, MON_DATA_NICKNAME, nick);
        nick[POKEMON_NAME_LENGTH] = EOS;
        Notice(nick, "went for a walk with your phone!");
        if (!GetBoxMonData(&sWalk.mon, MON_DATA_IS_EGG))
            PlayCry_Normal(GetBoxMonData(&sWalk.mon, MON_DATA_SPECIES), 0);
        PlaySE(SE_M_TELEPORT);
    }
    MPNet_Log("phone: %u went for a walk (walk %u)", (unsigned)sWalk.pid, (unsigned)sWalk.id);
    BufWalk(&sDoneExtra);
    return NULL;
}

// Back in the save: the party when there's room, else the PC
static void PutBack(struct Pokemon *mon)
{
    if (gPlayerPartyCount < PARTY_SIZE)
    {
        gPlayerParty[gPlayerPartyCount] = *mon;
        CalculatePlayerPartyCount();
        PartyChanged();
    }
    else
        MP_CopyMonToPC(mon);
}

// The day (on the player's own clock) a moment falls on, as a number: the days of a walk are the ones it was out on
static u32 DayNumber(u32 t)
{
    time_t tt = t;
    struct tm lt = *localtime(&tt);
    lt.tm_hour = 12, lt.tm_min = 0, lt.tm_sec = 0;
    return (u32)((mktime(&lt) + 43200) / 86400);
}

static const char *DoHome(const char *json)
{
    static u16 sWild[40][3], sPool[48];
    static long sFinds[64], sCaught[3 * 32];
    static struct Pokemon sMon;
    long long id = 0, steps = 0;
    u32 now = (u32)time(NULL), days, maxSteps, nWild, nPool, i, k, finds = 0, caught = 0, gained = 0, moments;
    int nf, nc;
    u8 levelBefore, nick[POKEMON_NAME_LENGTH + 1];
    bool8 egg;
    if (!JsonNum(json, "walk", &id) || !JsonNum(json, "steps", &steps))
        return "bad";
    if (!sWalking || (u32)id != sWalk.id)
        return "nowalk";
    if (memcmp(sWalk.phone, PhoneOrderPhone(), PHONE_ID_SIZE) != 0)
        return "otherphone";
    // the rules: steps a day since it left, finds and meetings from what that many steps earn
    days = DayNumber(now) >= DayNumber(sWalk.sentAt) ? DayNumber(now) - DayNumber(sWalk.sentAt) + 1 : 1;
    maxSteps = days * STEPS_A_DAY;
    if (steps < 0)
        steps = 0;
    if ((u64)steps > maxSteps)
        steps = maxSteps;
    nWild = WildPool(sWild, ARRAY_COUNT(sWild));
    nPool = FindPool(sPool, ARRAY_COUNT(sPool));
    nf = JsonNums(json, "finds", sFinds, ARRAY_COUNT(sFinds));
    nc = JsonNums(json, "caught", sCaught, ARRAY_COUNT(sCaught));
    BoxMonToMon(&sWalk.mon, &sMon);
    egg = GetMonData(&sMon, MON_DATA_IS_EGG);
    levelBefore = GetMonData(&sMon, MON_DATA_LEVEL);
    GetMonData(&sMon, MON_DATA_NICKNAME, nick);
    nick[POKEMON_NAME_LENGTH] = EOS;
    if (egg)
    {
        // an EGG's steps: a cycle each 256 (it hatches in the game, on the next step once none are left)
        u8 cycles = GetMonData(&sMon, MON_DATA_FRIENDSHIP);
        u32 done = (u32)steps / 256;
        cycles = done >= cycles ? 0 : cycles - done;
        SetMonData(&sMon, MON_DATA_FRIENDSHIP, &cycles);
        moments = 0;
    }
    else
    {
        // EXP as the day care gives it (a point a step), to the badge cap's level
        u32 exp = GetMonData(&sMon, MON_DATA_EXP), cap = sWalk.cap > MAX_LEVEL ? MAX_LEVEL : sWalk.cap;
        u32 capExp = gExperienceTables[gSpeciesInfo[GetMonData(&sMon, MON_DATA_SPECIES)].growthRate][cap];
        u32 newExp = exp + (u32)steps;
        if (levelBefore < cap)
        {
            newExp = newExp > capExp ? capExp : newExp;
            if (newExp > exp)
            {
                gained = newExp - exp;
                SetMonData(&sMon, MON_DATA_EXP, &newExp);
                MP_ApplyDaycareExperience(&sMon);
            }
        }
        moments = (u32)steps / STEPS_A_MOMENT;
        moments = moments > 40 ? 40 : moments;
    }
    PutBack(&sMon);
    {
        // its Garden: a memory of the walk and its Friendship moments (by finding it where it went back to)
        struct BoxPokemon *box = NULL;
        struct PokemonStorage *st = Storage();
        u32 b;
        for (i = 0; i < PARTY_SIZE && box == NULL; i++)
            if (GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == sWalk.pid && GetMonData(&gPlayerParty[i], MON_DATA_OT_ID) == sWalk.ot
             && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
                box = &gPlayerParty[i].box;
        for (b = 0; b < TOTAL_BOXES_COUNT && box == NULL; b++)
            for (i = 0; i < IN_BOX_COUNT && box == NULL; i++)
                if (GetBoxMonData(&st->boxes[b][i], MON_DATA_PERSONALITY) == sWalk.pid
                 && GetBoxMonData(&st->boxes[b][i], MON_DATA_OT_ID) == sWalk.ot)
                    box = &st->boxes[b][i];
        if (box != NULL && !egg)
            MP_Life_WalkHome(box, (u32)steps, moments);
    }
    BufAdd(&sDoneExtra, "\"home\":{\"steps\":%lld,\"exp\":%u,\"levels\":%u,\"moments\":%u,\"finds\":[", steps,
           (unsigned)gained, (unsigned)(GetMonData(&sMon, MON_DATA_LEVEL) - levelBefore), (unsigned)moments);
    // its finds: each one of the place's (no more than the steps earned)
    for (i = 0; nf > 0 && i < (u32)nf && finds < (u32)steps / STEPS_A_FIND; i++)
    {
        u16 item = (u16)sFinds[i];
        if (!Has(sPool, nPool, item) || !AddBagItem(item, 1))
            continue;
        BufAdd(&sDoneExtra, "%s%u", finds++ ? "," : "", item);
        sBagChanged = TRUE;
    }
    BufAdd(&sDoneExtra, "],\"caught\":[");
    // its meetings: each from the place's own wild table at its levels (3 a day), to the PC
    for (i = 0; nc > 0 && i + 2 < (u32)nc + 0 && i < (u32)nc; i += 3)
    {
        u16 sp = (u16)sCaught[i];
        u8 lv = (u8)sCaught[i + 1];
        u16 ball = (u16)sCaught[i + 2];
        static struct Pokemon sNew;
        u16 metLoc = sWalk.mapsec;
        if (caught >= (u32)steps / STEPS_A_MEET || caught >= days * MEETS_A_DAY)
            break;
        for (k = 0; k < nWild && sWild[k][0] != sp; k++)
            ;
        if (k == nWild || lv < sWild[k][1] || lv > sWild[k][2])
            continue;
        if (GetItemPocket(ball) != POCKET_POKE_BALLS)
            ball = ITEM_POKE_BALL;
        CreateMon(&sNew, sp, lv, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
        SetMonData(&sNew, MON_DATA_POKEBALL, &ball);
        SetMonData(&sNew, MON_DATA_MET_LOCATION, &metLoc);
        if (MP_CopyMonToPC(&sNew) == MON_CANT_GIVE)
            break;
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_SEEN);
        GetSetPokedexFlag(SpeciesToNationalPokedexNum(sp), FLAG_SET_CAUGHT);
        BufAdd(&sDoneExtra, "%s[%u,%u]", caught++ ? "," : "", sp, lv);
    }
    BufAdd(&sDoneExtra, "]}");
    sWalking = FALSE;
    WalkErase();
    SaveNow();
    {
        char what[80];
        snprintf(what, sizeof(what), "came home! %lld steps, %u found, %u met.", steps, (unsigned)finds, (unsigned)caught);
        Notice(nick, what);
        if (!egg)
            PlayCry_Normal(GetMonData(&sMon, MON_DATA_SPECIES), 0);
    }
    MPNet_Log("phone: walk %u home: %lld steps, %u EXP, %u finds, %u caught", (unsigned)sWalk.id, steps, (unsigned)gained,
              (unsigned)finds, (unsigned)caught);
    return NULL;
}

// ---------------------------------------------------------------------------------------------------- training

#define TRAIN_MAGIC 0x4E525450   // "PTRN"
#define TRAIN_MAX   6
enum { TRAIN_STAT, TRAIN_LEVEL, TRAIN_REST };

struct TrainRec
{
    u32 pid, ot;
    u8 kind, stat, pad[2];
    u32 since, last;
};

static struct
{
    u32 magic, count;
    struct TrainRec rec[TRAIN_MAX];
} sTrain;

static void TrainWrite(void)
{
    char path[600];
    FILE *f;
    FilePath(path, sizeof(path), "train");
    if (sTrain.count == 0)
    {
        remove(path);
        return;
    }
    f = fopen(path, "wb");
    if (f != NULL)
    {
        fwrite(&sTrain, sizeof(sTrain), 1, f);
        fclose(f);
    }
}

static u32 TrainSlots(void)
{
    u32 n = 2 + MP_Region_TotalBadges() / 2;
    return n > TRAIN_MAX ? TRAIN_MAX : n;
}

static struct TrainRec *TrainFind(u32 pid, u32 ot)
{
    u32 i;
    for (i = 0; i < sTrain.count; i++)
        if (sTrain.rec[i].pid == pid && sTrain.rec[i].ot == ot)
            return &sTrain.rec[i];
    return NULL;
}

bool8 MP_Phone_InTraining(u32 pid, u32 ot)   // (the PC: TRAINING where its item would be)
{
    return TrainFind(pid, ot) != NULL;
}

static struct BoxPokemon *FindInBoxes(u32 pid, u32 ot)
{
    struct PokemonStorage *st = Storage();
    u32 b, i;
    for (b = 0; b < TOTAL_BOXES_COUNT; b++)
        for (i = 0; i < IN_BOX_COUNT; i++)
            if (GetBoxMonData(&st->boxes[b][i], MON_DATA_SPECIES) != SPECIES_NONE
             && GetBoxMonData(&st->boxes[b][i], MON_DATA_PERSONALITY) == pid && GetBoxMonData(&st->boxes[b][i], MON_DATA_OT_ID) == ot)
                return &st->boxes[b][i];
    return NULL;
}

// A training's gains since its last collection (at most a day's), on BOX; the amount ("gains": EVs, EXP, Friendship)
static u32 Collect(struct TrainRec *r, struct BoxPokemon *box)
{
    static struct Pokemon sMon;
    u32 now = (u32)time(NULL), secs = now > r->last ? now - r->last : 0, mins, gain = 0;
    secs = secs > 86400 ? 86400 : secs;
    mins = secs / 60;
    if (GetBoxMonData(box, MON_DATA_IS_EGG))
    {
        r->last = now;
        return 0;
    }
    switch (r->kind)
    {
    case TRAIN_STAT:
    {
        static const u8 sEv[6] = {MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV, MON_DATA_SPEED_EV, MON_DATA_SPATK_EV,
                                  MON_DATA_SPDEF_EV};
        u32 total = 0, i, ev, add = mins / 5;
        for (i = 0; i < 6; i++)
            total += GetBoxMonData(box, sEv[i]);
        ev = GetBoxMonData(box, sEv[r->stat % 6]);
        if (add > 252 - (ev > 252 ? 252 : ev))
            add = ev >= 252 ? 0 : 252 - ev;
        if (add > MAX_TOTAL_EVS - (total > MAX_TOTAL_EVS ? MAX_TOTAL_EVS : total))
            add = total >= MAX_TOTAL_EVS ? 0 : MAX_TOTAL_EVS - total;
        if (add > 0)
        {
            u8 v = ev + add;
            SetBoxMonData(box, sEv[r->stat % 6], &v);
        }
        gain = add;
        r->last += (mins / 5) * 5 * 60 < secs ? (mins / 5) * 5 * 60 : secs;   // (what's left of the 5 minutes carries over)
        if (r->last > now || add == 0)
            r->last = now;
        return gain;
    }
    case TRAIN_LEVEL:
    {
        u8 cap = MP_RareCandyCap(), lv;
        u32 exp, capExp, newExp;
        BoxMonToMon(box, &sMon);
        lv = GetMonData(&sMon, MON_DATA_LEVEL);
        exp = GetMonData(&sMon, MON_DATA_EXP);
        capExp = gExperienceTables[gSpeciesInfo[GetMonData(&sMon, MON_DATA_SPECIES)].growthRate][cap > MAX_LEVEL ? MAX_LEVEL : cap];
        newExp = exp + mins * 10;
        if (lv < cap && newExp > exp)
        {
            newExp = newExp > capExp ? capExp : newExp;
            SetMonData(&sMon, MON_DATA_EXP, &newExp);
            MP_ApplyDaycareExperience(&sMon);
            *box = sMon.box;
            gain = newExp - exp;
        }
        break;
    }
    default:
    {
        u8 f = GetBoxMonData(box, MON_DATA_FRIENDSHIP);
        u32 add = mins / 30;
        BoxMonToMon(box, &sMon);
        MonRestorePP(&sMon);
        *box = sMon.box;
        add = add > (u32)(MAX_FRIENDSHIP - f) ? (u32)(MAX_FRIENDSHIP - f) : add;   // (only ever up)
        f += add;
        SetBoxMonData(box, MON_DATA_FRIENDSHIP, &f);
        gain = add;
        break;
    }
    }
    r->last = now;
    return gain;
}

static void TrainRemove(struct TrainRec *r)
{
    u32 i = r - sTrain.rec;
    memmove(&sTrain.rec[i], &sTrain.rec[i + 1], (sTrain.count - i - 1) * sizeof(sTrain.rec[0]));
    sTrain.count--;
}

static const char *DoTrain(const char *json)
{
    struct Loc at;
    long long p = 0, o = 0, kind = 0, stat = 0;
    struct TrainRec *r;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &p) || !JsonNum(json, "o", &o) || !JsonNum(json, "k", &kind))
        return "bad";
    JsonNum(json, "s", &stat);
    if (at.kind != LOC_BOX)
        return "trainbox";
    if (Empty(&at))
        return "gone";
    if (!Is(&at, (u32)p, (u32)o))
        return "changed";
    if (GetBoxMonData(BoxAt(&at), MON_DATA_IS_EGG))
        return "egg";
    if (MP_Phone_OnOffer((u32)p, (u32)o))
        return "onoffer";
    if (kind < TRAIN_STAT || kind > TRAIN_REST || stat < 0 || stat > 5)
        return "bad";
    r = TrainFind((u32)p, (u32)o);
    if (r == NULL)
    {
        if (sTrain.count >= TrainSlots())
            return "slots";
        r = &sTrain.rec[sTrain.count++];
        r->pid = (u32)p;
        r->ot = (u32)o;
        r->since = (u32)time(NULL);
    }
    else
        Collect(r, BoxAt(&at));   // (a change of course: what it has done so far first)
    r->kind = kind;
    r->stat = stat;
    r->last = (u32)time(NULL);
    Changed(&at);
    TrainWrite();
    return NULL;
}

// "untrain": collect and end one; "collect": collect every one (they go on)
static const char *DoCollect(const char *json, bool8 end)
{
    long long p = 0, o = 0;
    u32 i, n = 0;
    BufAdd(&sDoneExtra, "\"gains\":[");
    for (i = 0; i < sTrain.count;)
    {
        struct TrainRec *r = &sTrain.rec[i];
        struct BoxPokemon *box;
        if (end && (!JsonNum(json, "p", &p) || !JsonNum(json, "o", &o) || r->pid != (u32)p || r->ot != (u32)o))
        {
            i++;
            continue;
        }
        box = FindInBoxes(r->pid, r->ot);
        if (box != NULL)
        {
            u32 g = Collect(r, box);
            BufAdd(&sDoneExtra, "%s[%u,%u,%u,%u]", n++ ? "," : "", (unsigned)r->pid, (unsigned)r->ot, r->kind, (unsigned)g);
            {
                // (its place in the boxes, so the app sees the change)
                struct PokemonStorage *st = Storage();
                struct Loc l;
                l.kind = LOC_BOX;
                l.box = (box - &st->boxes[0][0]) / IN_BOX_COUNT;
                l.slot = (box - &st->boxes[0][0]) % IN_BOX_COUNT;
                Changed(&l);
            }
        }
        if (end || box == NULL)
            TrainRemove(r);
        else
            i++;
    }
    BufAdd(&sDoneExtra, "]");
    TrainWrite();
    return NULL;
}

// Every two seconds: a POKéMON that left the boxes stops training (taken into the party: what it did is collected
// first; released or traded away: it simply ends)
static void TrainTick(void)
{
    static u8 sTimer;
    u32 i;
    bool8 changed = FALSE;
    if (++sTimer < 120 || sTrain.count == 0)
        return;
    sTimer = 0;
    for (i = 0; i < sTrain.count;)
    {
        struct TrainRec *r = &sTrain.rec[i];
        u32 k;
        if (FindInBoxes(r->pid, r->ot) != NULL)
        {
            i++;
            continue;
        }
        for (k = 0; k < PARTY_SIZE; k++)
            if (GetMonData(&gPlayerParty[k], MON_DATA_SPECIES) != SPECIES_NONE
             && GetMonData(&gPlayerParty[k], MON_DATA_PERSONALITY) == r->pid && GetMonData(&gPlayerParty[k], MON_DATA_OT_ID) == r->ot)
            {
                Collect(r, &gPlayerParty[k].box);
                CalculateMonStats(&gPlayerParty[k]);
                break;
            }
        MPNet_Log("phone: %u left the boxes: its training ends", (unsigned)r->pid);
        TrainRemove(r);
        changed = TRUE;
    }
    if (changed)
        TrainWrite();
}

// ------------------------------------------------------------------------------------------ the Garden's kindness

// "kind": a pet (verb 0) or a BERRY (verb 1, item) for a POKéMON, as the Garden would have it; "line": what happened
static const char *DoKind(const char *json)
{
    struct Loc at;
    long long p = 0, o = 0, verb = 0, item = 0;
    static u8 sLine[160];
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &p) || !JsonNum(json, "o", &o) || !JsonNum(json, "verb", &verb))
        return "bad";
    JsonNum(json, "item", &item);
    if (Empty(&at))
        return "gone";
    if (!Is(&at, (u32)p, (u32)o))
        return "changed";
    if (GetBoxMonData(BoxAt(&at), MON_DATA_IS_EGG))
        return "egg";
    if (!MP_Life_PhoneVerb(BoxAt(&at), verb != 0, (u16)item, sLine))
        return "noitem";
    if (verb != 0)
        sBagChanged = TRUE;
    Changed(&at);
    BufAdd(&sDoneExtra, "\"line\":");
    BufText(&sDoneExtra, sLine, sizeof(sLine) - 1);
    return NULL;
}

// ---------------------------------------------------------------------------------------------- the link's hooks

const char *PhoneWalk_Apply(const char *op, const char *json)
{
    if (strcmp(op, "walk") == 0)
        return DoWalk(json);
    if (strcmp(op, "home") == 0)
        return DoHome(json);
    if (strcmp(op, "train") == 0)
        return DoTrain(json);
    if (strcmp(op, "untrain") == 0)
        return DoCollect(json, TRUE);
    if (strcmp(op, "collect") == 0)
        return DoCollect(json, FALSE);
    if (strcmp(op, "kind") == 0)
        return DoKind(json);
    return PhoneFriends_Apply(op, json);
}

void PhoneWalk_Snapshot(struct Buf *b, const u8 *phone)
{
    u32 i;
    BufAdd(b, ",\"now\":%u", (unsigned)time(NULL));
    if (sWalking && phone != NULL && memcmp(phone, sWalk.phone, PHONE_ID_SIZE) == 0)
    {
        BufAdd(b, ",");
        BufWalk(b);
    }
    else if (sWalking)
        BufAdd(b, ",\"walkOther\":1");   // (another phone has a POKéMON out)
    {
        // the daily report's: today's goals, the BERRY trees ready and growing, MoeGatcha's free pulls and tickets
        static char sGoals[3][64];
        s32 prog[3], target[3];
        u8 n = MP_Events_PhoneGoals(sGoals, prog, target, 3), k;
        u32 ready = 0, growing = 0, t;
        BufAdd(b, ",\"today\":{\"goals\":[");
        for (k = 0; k < n; k++)
        {
            char *q;
            for (q = sGoals[k]; *q; q++)
                if (*q == '"' || *q == '\\' || (u8)*q < 32)
                    *q = ' ';
            BufAdd(b, "%s[\"%s\",%d,%d]", k ? "," : "", sGoals[k], (int)prog[k], (int)target[k]);
        }
        for (t = 1; t < BERRY_TREES_COUNT; t++)
        {
            u8 st = gSaveBlock1Ptr->berryTrees[t].stage;
            ready += st == BERRY_STAGE_BERRIES;
            growing += st >= BERRY_STAGE_PLANTED && st < BERRY_STAGE_BERRIES;
        }
        BufAdd(b, "],\"berries\":[%u,%u],\"gatcha\":[%d,%d]}", (unsigned)ready, (unsigned)growing,
               (int)MP_Gatcha_FreeWaiting(), (int)MP_Gatcha_Tickets());
    }
    BufAdd(b, ",\"train\":{\"slots\":%u,\"list\":[", (unsigned)TrainSlots());
    for (i = 0; i < sTrain.count; i++)
        BufAdd(b, "%s{\"p\":%u,\"o\":%u,\"k\":%u,\"s\":%u,\"since\":%u,\"last\":%u}", i ? "," : "",
               (unsigned)sTrain.rec[i].pid, (unsigned)sTrain.rec[i].ot, sTrain.rec[i].kind, sTrain.rec[i].stat,
               (unsigned)sTrain.rec[i].since, (unsigned)sTrain.rec[i].last);
    BufAdd(b, "]}");
}

void PhoneWalk_Load(void)
{
    char path[600];
    FILE *f;
    sWalking = FALSE;
    FilePath(path, sizeof(path), "walk");
    f = fopen(path, "rb");
    if (f != NULL)
    {
        if (fread(&sWalk, sizeof(sWalk), 1, f) == 1 && sWalk.magic == WALK_MAGIC && sWalk.version == WALK_VERSION)
            sWalking = TRUE;
        fclose(f);
        if (sWalking && InSave(sWalk.pid, sWalk.ot))
        {
            // the save still has it (the game closed between writing the walk and saving): the walk never began
            MPNet_Log("phone: walk %u never began (still in the save)", (unsigned)sWalk.id);
            sWalking = FALSE;
            WalkErase();
        }
    }
    memset(&sTrain, 0, sizeof(sTrain));
    FilePath(path, sizeof(path), "train");
    f = fopen(path, "rb");
    if (f != NULL)
    {
        if (fread(&sTrain, sizeof(sTrain), 1, f) != 1 || sTrain.magic != TRAIN_MAGIC || sTrain.count > TRAIN_MAX)
            memset(&sTrain, 0, sizeof(sTrain));
        fclose(f);
    }
    sTrain.magic = TRAIN_MAGIC;
}

void PhoneWalk_Update(void)
{
    TrainTick();
}

void PhoneWalk_ForgetAll(void)
{
    static struct Pokemon sMon;
    if (!sWalking)
        return;
    BoxMonToMon(&sWalk.mon, &sMon);
    PutBack(&sMon);
    sWalking = FALSE;
    if (SaveNow())
        WalkErase();   // (else the file stays: on the next start it's in the save, so the file goes then)
    MPNet_Log("phone: the walking POKéMON came home as it left");
}

// {"t":"journal","at":PLACE,"p","o"}: a POKéMON's Garden JOURNAL as the summary's page builds it (game text, newest
// first)
static void SendJournal(int c, const char *json)
{
    static u8 sLines[24][LIFE_JOURNAL_TEXT];
    struct Loc at;
    long long p = 0, o = 0;
    u8 n = 0, i;
    if (ParseLoc(json, "at", &at) && JsonNum(json, "p", &p) && JsonNum(json, "o", &o) && Is(&at, (u32)p, (u32)o)
     && !Empty(&at) && !GetBoxMonData(BoxAt(&at), MON_DATA_IS_EGG))
        n = MP_Life_Journal(BoxAt(&at), sLines, ARRAY_COUNT(sLines));
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"journal\",\"p\":%lld,\"o\":%lld,\"lines\":[", p, o);
    for (i = 0; i < n; i++)
    {
        BufAdd(&sOut, "%s", i ? "," : "");
        BufText(&sOut, sLines[i], LIFE_JOURNAL_TEXT - 1);
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

bool8 PhoneWalk_OnSealed(int c, const char *t, const char *json)
{
    if (strcmp(t, "journal") == 0)
    {
        SendJournal(c, json);
        return TRUE;
    }
    return FALSE;
}
