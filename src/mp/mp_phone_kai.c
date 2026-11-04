// Moemon Co-op: Kai's own things on the phone (the app's ME page, docs/PHONE_LINK.md "The ME page"): the
// achievements (mp_achieve.c), the MOEGATCHA (mp_gatcha.c MP_Gatcha_PhonePull: the reel's own prizes), the CANDY JAR
// (mp_candy.c) and renaming a POKéMON as the summary does (any POKéMON but an EGG).
//   snapshot "kai": {"ach":[done, total], "pulls", "tickets", "free", "jar": 0/1, "candy":[a count per type],
//                    "time":[hours, minutes], "farm":[[berry item, stage, minutes to the next, yield, watered 0-4]...]
//                    (the save's own BERRY farm in the GARDEN, its open plots: mp_farm.c)}
//   {"t":"ach"} -> {"t":"ach","cats":[name...],"list":[[cat, name, words, status, reward, done]...]}
//   {"t":"gatcha"} -> {"t":"gatcha","pulls","tickets","banners":[[name, tagline, free 1, free 10]...]}
//   orders "rename" {at, p, o, name: hex}, "candy" {type: -1 every kind}, "pull" {banner, how}
#include <stdio.h>
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
    BufAdd(b, "]}");
}

// (the snapshot's rev: achievements done, pulls, candies)
u32 PhoneKai_Hash(void)
{
    u32 h = 2166136261u, t;
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
    return (const char *)-1;
}
