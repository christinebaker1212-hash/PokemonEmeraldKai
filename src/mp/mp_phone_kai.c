// Moemon Co-op: Kai's own things on the phone (the app's ME page, docs/PHONE_LINK.md "The ME page"): the
// achievements (mp_achieve.c), the MOEGATCHA (mp_gatcha.c MP_Gatcha_PhonePull: the reel's own prizes), the CANDY JAR
// (mp_candy.c) and renaming a POKéMON as the summary does (any POKéMON but an EGG).
//   snapshot "kai": {"ach":[done, total], "pulls", "tickets", "free", "jar": 0/1, "candy":[a count per type],
//                    "time":[hours, minutes], "farm":[[berry item, stage, minutes to the next, yield, watered 0-4]...]
//                    (the save's own BERRY farm in the GARDEN, its open plots: mp_farm.c)}
//   {"t":"ach"} -> {"t":"ach","cats":[name...],"list":[[cat, name, words, status, reward, done]...]}
//   {"t":"gatcha"} -> {"t":"gatcha","pulls","tickets","banners":[[name, tagline, free 1, free 10]...]}
//   orders "rename" {at, p, o, name: hex}, "candy" {type: -1 every kind}, "pull" {banner, how},
//          "mgift" {code: the scanned code's hash, hex} -> {item, qty, tier, n: name, left}
//   MYSTERY GIFT (the user, 2026-11-05): a QR code or a product's bar code scanned by the app gives an item, up to
//   GIFTS_A_DAY a day on the player's own clock; the code, the day and the Trainer ID pick it (the same code twice in
//   a day gives nothing more), from useless to amazing ("gift:<day>:n" / "gift:<day>:h<k>" in mp_savedata.c).
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "global.h"
#include "mp.h"
#include "item.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "berry.h"
#include "mp_farm.h"
#include "mp_farm.h"
#include "mp_garden_map.h"
#include "mp_phone_int.h"

extern u16 gMpCandies[NUMBER_OF_MON_TYPES];
u16 BerryTypeToItemId(u16 berry);
u16 MP_Achieve_Total(void);
u16 MP_Achieve_DoneCount(void);
u8 MP_Achieve_CatCount(void);
const char *MP_Achieve_CatName(u8 cat);
u16 MP_Achieve_CountIn(u8 cat, u16 *done);
u16 MP_Achieve_IndexIn(u8 cat, u16 row);
bool8 MP_Achieve_IsDone(u16 i);
void MP_Achieve_Info(u16 i, const char **name, const char **desc, char *status, int statusLen, char *reward, int rewardLen);
s32 MP_Gatcha_Pulls(void);
bool8 MP_Candy_ConvertType(u8 type, u16 *made);
bool8 MP_Candy_Convert(u16 *made);
void MPNet_Log(const char *fmt, ...);
int PhoneKai_GiftsLeft(void);

static void Ascii(struct Buf *b, const char *s)
{
    BufAdd(b, "\"");
    for (; s && *s; s++)
    {
        if (*s == '~')
            BufAdd(b, "\\u00e9");   // (the game's é in ASCII text)
        else if (*s == '"' || *s == '\\')
            BufAdd(b, "\\%c", *s);
        else if ((u8)*s >= 0x20)
            BufAdd(b, "%c", *s);
    }
    BufAdd(b, "\"");
}

static bool8 HasJar(void)
{
    return CheckBagHasItem(ITEM_CANDY_JAR, 1);
}

void PhoneKai_Snapshot(struct Buf *b)
{
    u8 t;
    BufAdd(b, ",\"kai\":{\"ach\":[%u,%u],\"pulls\":%d,\"tickets\":%d,\"free\":%d,\"jar\":%u,\"candy\":[",
           MP_Achieve_DoneCount(), MP_Achieve_Total(), (int)MP_Gatcha_Pulls(), (int)MP_Gatcha_Tickets(),
           (int)MP_Gatcha_FreeWaiting(), HasJar());
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        BufAdd(b, "%s%u", t ? "," : "", gMpCandies[t]);
    BufAdd(b, "],\"time\":[%u,%u],\"farm\":[", gSaveBlock2Ptr->playTimeHours, gSaveBlock2Ptr->playTimeMinutes);
    {
        u8 i, open = MP_Farm_IsFriends() ? 0 : MP_Farm_Plots();   // (the save's own farm only)
        for (i = 0; i < open; i++)
        {
            struct BerryTree *t = &gSaveBlock1Ptr->berryTrees[GARDEN_FARM_TREE_FIRST + i];
            BufAdd(b, "%s[%u,%u,%u,%u,%u]", i ? "," : "", t->berry ? BerryTypeToItemId(t->berry) : 0, t->stage,
                   t->minutesUntilNextStage, t->berryYield, t->watered1 + t->watered2 + t->watered3 + t->watered4);
        }
    }
    BufAdd(b, "],\"gift\":%d}", PhoneKai_GiftsLeft());
}

// ------------------------------------------------------------------------------------------------ MYSTERY GIFT
#define GIFTS_A_DAY 12

static const u16 sGiftUseless[] = {
    ITEM_POKE_DOLL, ITEM_FLUFFY_TAIL, ITEM_RED_SHARD, ITEM_BLUE_SHARD, ITEM_YELLOW_SHARD, ITEM_GREEN_SHARD,
    ITEM_HEART_SCALE, ITEM_SHOAL_SALT, ITEM_SHOAL_SHELL, ITEM_TINY_MUSHROOM, ITEM_BLACK_FLUTE, ITEM_WHITE_FLUTE,
    ITEM_ESCAPE_ROPE, ITEM_REPEL, ITEM_ORAN_BERRY,
};
static const u16 sGiftCommon[] = {
    ITEM_POTION, ITEM_SUPER_POTION, ITEM_GREAT_BALL, ITEM_ANTIDOTE, ITEM_ETHER, ITEM_BIG_MUSHROOM, ITEM_PEARL,
    ITEM_X_ATTACK, ITEM_SUPER_REPEL, ITEM_FULL_HEAL, ITEM_SITRUS_BERRY,
};
static const u16 sGiftGood[] = {
    ITEM_ULTRA_BALL, ITEM_HYPER_POTION, ITEM_REVIVE, ITEM_MAX_REPEL, ITEM_STARDUST, ITEM_MAX_ETHER, ITEM_ELIXIR,
    ITEM_TIMER_BALL, ITEM_NET_BALL,
};
static const u16 sGiftGreat[] = {
    ITEM_RARE_CANDY, ITEM_PP_UP, ITEM_NUGGET, ITEM_BIG_PEARL, ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CARBOS,
    ITEM_CALCIUM, ITEM_ZINC, ITEM_MAX_REVIVE, ITEM_FULL_RESTORE, ITEM_STAR_PIECE,
};
static const u16 sGiftAmazing[] = {
    ITEM_MASTER_BALL, ITEM_PP_MAX, ITEM_LUCKY_EGG, ITEM_LEFTOVERS, ITEM_KINGS_ROCK, ITEM_CHOICE_BAND,
    ITEM_SCOPE_LENS, ITEM_QUICK_CLAW, ITEM_EXP_SHARE,
};
// (out of 100: useless 40, common 30, good 18, great 9, amazing 3)
static const u8 sGiftOdds[5] = {40, 30, 18, 9, 3};

// today on the player's own clock, as YYYYMMDD
static long GiftDay(void)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    return t ? (t->tm_year + 1900) * 10000L + (t->tm_mon + 1) * 100 + t->tm_mday : 0;
}

int PhoneKai_GiftsLeft(void)
{
    char key[32];
    snprintf(key, sizeof(key), "gift:%ld:n", GiftDay());
    return GIFTS_A_DAY - (int)MP_SaveData_Get(key);
}

// {"op":"mgift","code":"<hex>"[,"day":YYYYMMDD]}: the code's gift for today, or for the day the phone scanned it
// without a link (up to a week back: that day's twelve and that day's gift, as if the game had been there)
static const char *DoGift(const char *json)
{
    char code[80], key[40];
    long day = GiftDay();
    long long scanned;
    if (JsonNum(json, "day", &scanned) && scanned > 20000101 && scanned <= day
        && MP_DayNumber((s32)scanned) >= MP_DayNumber((s32)day) - 7)
        day = (long)scanned;
    int used, k;
    u32 h = 2166136261u, codeHash = 2166136261u, roll, pick;
    u16 item, qty = 1;
    u8 tier = 0;
    const u16 *list;
    size_t len;
    const char *c;
    if (!JsonStr(json, "code", code, sizeof(code)) || code[0] == 0)
        return "bad";
    for (c = code; *c; c++)
        codeHash = (codeHash ^ (u8)*c) * 16777619u;
    if (codeHash == 0)
        codeHash = 1;
    snprintf(key, sizeof(key), "gift:%ld:n", day);
    used = (int)MP_SaveData_Get(key);
    if (used >= GIFTS_A_DAY)
        return "giftdone";
    for (k = 0; k < used; k++)
    {
        snprintf(key, sizeof(key), "gift:%ld:h%d", day, k);
        if ((u32)MP_SaveData_Get(key) == codeHash)
            return "giftsame";
    }
    // the code, the day and the Trainer ID: the same scan gives the same gift that day, another one tomorrow
    h = (h ^ codeHash) * 16777619u;
    h = (h ^ (u32)day) * 16777619u;
    for (k = 0; k < 4; k++)
        h = (h ^ gSaveBlock2Ptr->playerTrainerId[k]) * 16777619u;
    h ^= h >> 15;
    h *= 0x2C1B3C6Du;
    h ^= h >> 12;
    roll = h % 100;
    for (tier = 0; tier < 4 && roll >= sGiftOdds[tier]; tier++)
        roll -= sGiftOdds[tier];
    switch (tier)
    {
    case 0: list = sGiftUseless, len = ARRAY_COUNT(sGiftUseless); break;
    case 1: list = sGiftCommon, len = ARRAY_COUNT(sGiftCommon); break;
    case 2: list = sGiftGood, len = ARRAY_COUNT(sGiftGood); break;
    case 3: list = sGiftGreat, len = ARRAY_COUNT(sGiftGreat); break;
    default: list = sGiftAmazing, len = ARRAY_COUNT(sGiftAmazing); break;
    }
    pick = (h >> 8) % len;
    item = list[pick];
    if (tier == 1)
        qty = 1 + (h >> 20) % 3;
    else if (tier == 2)
        qty = 1 + (h >> 20) % 2;
    if (!AddBagItem(item, qty))
        return "bagfull";
    snprintf(key, sizeof(key), "gift:%ld:h%d", day, used);
    MP_SaveData_Set(key, (s32)codeHash);
    snprintf(key, sizeof(key), "gift:%ld:n", day);
    MP_SaveData_Set(key, used + 1);
    sBagChanged = TRUE;
    {
        u8 name[ITEM_NAME_LENGTH + 4];
        CopyItemName(item, name);
        BufAdd(&sDoneExtra, "\"item\":%u,\"qty\":%u,\"tier\":%u,\"left\":%d,\"day\":%ld,\"n\":", item, qty, tier,
               GIFTS_A_DAY - used - 1, day);
        BufText(&sDoneExtra, name, sizeof(name) - 1);
    }
    MPNet_Log("[phone] mystery gift: code %08x -> item %u x%u (tier %u), %d left today", codeHash, item, qty, tier,
              GIFTS_A_DAY - used - 1);
    return NULL;
}

// (the snapshot's rev: achievements done, pulls, candies)
u32 PhoneKai_Hash(void)
{
    u32 h = 2166136261u, t;
    h = (h ^ (u32)PhoneKai_GiftsLeft()) * 16777619u;
    h = (h ^ MP_Achieve_DoneCount()) * 16777619u;
    h = (h ^ (u32)MP_Gatcha_Pulls()) * 16777619u;
    h = (h ^ (u32)MP_Gatcha_Tickets()) * 16777619u;
    h = (h ^ (u32)MP_Gatcha_FreeWaiting()) * 16777619u;
    h = (h ^ HasJar()) * 16777619u;
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        h = (h ^ gMpCandies[t]) * 16777619u;
    for (t = 0; t < GARDEN_FARM_PLOTS; t++)
    {
        struct BerryTree *tr = &gSaveBlock1Ptr->berryTrees[GARDEN_FARM_TREE_FIRST + t];
        h = (h ^ (tr->berry | tr->stage << 8 | tr->berryYield << 16)) * 16777619u;
    }
    return h;
}

static void SendAch(int c)
{
    u8 cat;
    u16 row, n, done;
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"ach\",\"cats\":[");
    for (cat = 0; cat < MP_Achieve_CatCount(); cat++)
    {
        BufAdd(&sOut, "%s", cat ? "," : "");
        Ascii(&sOut, MP_Achieve_CatName(cat));
    }
    BufAdd(&sOut, "],\"list\":[");
    for (cat = 0, n = 0; cat < MP_Achieve_CatCount(); cat++)
    {
        u16 count = MP_Achieve_CountIn(cat, &done);
        for (row = 0; row < count; row++)
        {
            u16 i = MP_Achieve_IndexIn(cat, row);
            const char *name, *desc;
            char status[24], reward[96];
            MP_Achieve_Info(i, &name, &desc, status, sizeof(status), reward, sizeof(reward));
            BufAdd(&sOut, "%s[%u,", n++ ? "," : "", cat);
            Ascii(&sOut, name);
            BufAdd(&sOut, ",");
            Ascii(&sOut, desc);
            BufAdd(&sOut, ",");
            Ascii(&sOut, status);
            BufAdd(&sOut, ",");
            Ascii(&sOut, reward);
            BufAdd(&sOut, ",%u]", MP_Achieve_IsDone(i));
        }
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

static void SendGatcha(int c)
{
    u8 b;
    MP_Gatcha_GrantFree();
    BufReset(&sOut);
    BufAdd(&sOut, "{\"t\":\"gatcha\",\"pulls\":%d,\"tickets\":%d,\"banners\":[", (int)MP_Gatcha_Pulls(),
           (int)MP_Gatcha_Tickets());
    for (b = 0; b < MP_Gatcha_BannerCount(); b++)
    {
        BufAdd(&sOut, "%s[", b ? "," : "");
        Ascii(&sOut, MP_Gatcha_BannerName(b));
        BufAdd(&sOut, ",");
        Ascii(&sOut, MP_Gatcha_BannerTagline(b));
        BufAdd(&sOut, ",%d,%d]", (int)MP_Gatcha_FreeStock(b, FALSE), (int)MP_Gatcha_FreeStock(b, TRUE));
    }
    BufAdd(&sOut, "]}");
    SendBuf(c, &sOut);
}

bool8 PhoneKai_OnSealed(int c, const char *t, const char *json)
{
    (void)json;
    if (strcmp(t, "ach") == 0)
        SendAch(c);
    else if (strcmp(t, "gatcha") == 0)
        SendGatcha(c);
    else
        return FALSE;
    return TRUE;
}

static int HexDigit(char ch)
{
    return ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : -1;
}

// {"op":"rename","at":..,"p":..,"o":..,"name":"<game text as hex>"}: as the summary's rename (SELECT on INFO)
static const char *DoRename(const char *json)
{
    struct Loc at;
    long long pid, ot;
    char hex[2 * POKEMON_NAME_LENGTH + 4];
    u8 name[POKEMON_NAME_LENGTH + 1];
    u32 i, n;
    struct BoxPokemon *box;
    if (!ParseLoc(json, "at", &at) || !JsonNum(json, "p", &pid) || !JsonNum(json, "o", &ot) || Empty(&at)
     || !JsonStr(json, "name", hex, sizeof(hex)))
        return "bad";
    n = strlen(hex) / 2;
    if (n == 0 || n > POKEMON_NAME_LENGTH || strlen(hex) % 2)
        return "badname";
    for (i = 0; i < n; i++)
    {
        int hi = HexDigit(hex[2 * i]), lo = HexDigit(hex[2 * i + 1]);
        if (hi < 0 || lo < 0 || (hi << 4 | lo) == EOS || (hi << 4 | lo) >= 0xF7)
            return "badname";   // (letters only: no control codes)
        name[i] = hi << 4 | lo;
    }
    for (; i <= POKEMON_NAME_LENGTH; i++)
        name[i] = EOS;
    if (!Is(&at, (u32)pid, (u32)ot))
        return "gone";
    box = BoxAt(&at);
    if (GetBoxMonData(box, MON_DATA_IS_EGG))
        return "egg";
    SetBoxMonData(box, MON_DATA_NICKNAME, name);
    if (at.kind == LOC_PARTY)
        SetMonData(&OwnParty()[at.slot], MON_DATA_NICKNAME, name);
    MPNet_Log("phone: renamed %s %u", at.kind == LOC_PARTY ? "party" : "box", at.slot);
    Changed(&at);
    return NULL;
}

// {"op":"candy","type":T}: each 10 CANDIES of that type (-1: every type) become a RARE CANDY
static const char *DoCandy(const char *json)
{
    long long type = -1;
    u16 made = 0;
    bool8 ok;
    if (!HasJar())
        return "nojar";
    JsonNum(json, "type", &type);
    if (type >= NUMBER_OF_MON_TYPES)
        return "bad";
    ok = type < 0 ? MP_Candy_Convert(&made) : MP_Candy_ConvertType((u8)type, &made);
    if (!ok)
        return made == 0 ? "fewcandy" : "bagfull";
    sBagChanged = TRUE;
    BufAdd(&sDoneExtra, "\"made\":%u", made);
    return NULL;
}

// {"op":"pull","banner":B,"how":H}: MOEGATCHA pulls; the prizes go back in the answer
static const char *DoPull(const char *json)
{
    static struct MpGatchaPrize sPrizes[10];
    long long banner, how;
    u8 n = 0, i;
    const char *why;
    if (!JsonNum(json, "banner", &banner) || !JsonNum(json, "how", &how) || banner < 0 || how < 0)
        return "bad";
    why = MP_Gatcha_PhonePull((u8)banner, (u8)how, sPrizes, &n);
    if (why)
        return why;
    sBagChanged = TRUE;
    PartyChanged();
    BufAdd(&sDoneExtra, "\"prizes\":[");
    for (i = 0; i < n; i++)
    {
        BufAdd(&sDoneExtra, "%s{\"r\":%u,\"k\":%u,\"id\":%u,\"sh\":%u,\"amt\":%u,\"n\":", i ? "," : "", sPrizes[i].rarity,
               sPrizes[i].kind, sPrizes[i].id, sPrizes[i].shiny, (unsigned)sPrizes[i].amount);
        BufText(&sDoneExtra, sPrizes[i].name, sizeof(sPrizes[i].name) - 1);
        BufAdd(&sDoneExtra, ",\"note\":");
        Ascii(&sDoneExtra, sPrizes[i].note);
        BufAdd(&sDoneExtra, "}");
    }
    BufAdd(&sDoneExtra, "]");
    return NULL;
}

const char *PhoneKai_Apply(const char *op, const char *json)
{
    if (strcmp(op, "rename") == 0)
        return DoRename(json);
    if (strcmp(op, "candy") == 0)
        return DoCandy(json);
    if (strcmp(op, "pull") == 0)
        return DoPull(json);
    if (strcmp(op, "mgift") == 0)
        return DoGift(json);
    return (const char *)-1;
}
