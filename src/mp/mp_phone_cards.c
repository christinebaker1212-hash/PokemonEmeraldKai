// Moemon Co-op: the card games on the phone, the game's side (docs/COMPANION_APP.md "Card games", docs/PHONE_LINK.md
// "TRIPLE TRIAD on the phone"). The phone plays with the game's own engine (built into the app); the collection stays
// the game's: the snapshot's "tt" shows it (cards, decks, unopened packs, today's prizes), and orders change it:
//   "ttdeck"  a deck rebuilt on the phone (the game checks it as its own deck builder does),
//   "ttwin"   a prize: a pack for a duel won on the phone (3 a day) or the daily puzzle solved (1 a day),
//             counted per calendar day in "<save>.phonecards" so the phone never outdoes the GAME CENTER,
//   "ttopen"  a pack opened with the game's own pack math (the cards come back in the answer).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "mp.h"
#include "tt/collection.h"
#include "tt/integration.h"
#include "tcg/integration.h"
#include "mp_phone_int.h"

extern char gPlatformSavePath[512];
unsigned Tt_Tier(void);             // tt_field.c
unsigned Tt_CurrentRegion(void);    // tt_field.c

#define CARDS_MAGIC 0x44524350   // "PCRD"
#define DUEL_PACKS_A_DAY 3
#define PUZZLES_A_DAY 1

static struct
{
    u32 magic, day;
    u8 packs[3], puzzles[3];   // per game: 0 TRIPLE TRIAD (1 POKéMON, 2 YU-GI-OH! later)
    u8 pad[2];
} sDaily;
static bool8 sDailyLoaded;

static void DailyPath(char *out, size_t size)
{
    snprintf(out, size, "%s.phonecards", gPlatformSavePath);
}

static void DailyLoad(void)
{
    char path[600];
    FILE *f;
    if (sDailyLoaded)
        return;
    memset(&sDaily, 0, sizeof(sDaily));
    DailyPath(path, sizeof(path));
    if ((f = fopen(path, "rb")) != NULL)
    {
        if (fread(&sDaily, sizeof(sDaily), 1, f) != 1 || sDaily.magic != CARDS_MAGIC)
            memset(&sDaily, 0, sizeof(sDaily));
        fclose(f);
    }
    sDaily.magic = CARDS_MAGIC;
    sDailyLoaded = TRUE;
}

static void DailyToday(void)
{
    DailyLoad();
    if (sDaily.day != Tcg_Today())
    {
        memset(sDaily.packs, 0, sizeof(sDaily.packs));
        memset(sDaily.puzzles, 0, sizeof(sDaily.puzzles));
        sDaily.day = Tcg_Today();
    }
}

static void DailyWrite(void)
{
    char path[600];
    FILE *f;
    DailyPath(path, sizeof(path));
    if ((f = fopen(path, "wb")) != NULL)
    {
        fwrite(&sDaily, sizeof(sDaily), 1, f);
        fclose(f);
    }
}

void PhoneCards_Load(void)
{
    sDailyLoaded = FALSE;
}

// --------------------------------------------------------------------------------------------- the snapshot

static u32 Fnv(u32 h, const void *data, u32 n)
{
    const u8 *p = data;
    while (n--)
        h = (h ^ *p++) * 16777619u;
    return h;
}

u32 PhoneCards_Hash(void)
{
    struct TtCollection *c = Tt_GetCollection();
    u32 h = 2166136261u;
    if (c == NULL)
        return 0;
    DailyToday();
    h = Fnv(h, &c->optedIn, 1);
    h = Fnv(h, &c->activeDeck, 1);
    h = Fnv(h, c->owned, sizeof(c->owned));
    h = Fnv(h, c->decks, sizeof(c->decks));
    h = Fnv(h, c->packs, sizeof(c->packs));
    h = Fnv(h, &sDaily, sizeof(sDaily));
    return h;
}

void PhoneCards_Snapshot(struct Buf *b)
{
    struct TtCollection *c = Tt_GetCollection();
    unsigned i, k, n, region = Tt_CurrentRegion();
    if (c == NULL)
        return;
    DailyToday();
    if (region >= TT_REGIONS)
        region = 0;
    BufAdd(b, ",\"tt\":{\"on\":%u,\"day\":%u,\"region\":%u,\"tier\":%u,\"rules\":%u,\"trade\":%u,\"skill\":%d,\"active\":%u,"
              "\"rec\":[%u,%u,%u],\"today\":{\"packs\":%u,\"packsMax\":%u,\"puzzles\":%u,\"puzzlesMax\":%u},\"packs\":[",
           c->optedIn, Tcg_Today(), region, Tt_Tier(), (unsigned)c->regionRules[region], c->regionTrade[region], c->skill,
           c->activeDeck, (unsigned)c->wins, (unsigned)c->losses, (unsigned)c->draws, sDaily.packs[0], DUEL_PACKS_A_DAY,
           sDaily.puzzles[0], PUZZLES_A_DAY);
    for (i = 0; i < TT_PACK_TYPES; i++)
        BufAdd(b, "%s%u", i ? "," : "", c->packs[i]);
    BufAdd(b, "],\"decks\":[");
    for (i = 0; i < TT_DECKS; i++)
    {
        BufAdd(b, "%s{\"n\":\"", i ? "," : "");
        for (k = 0; k < TT_DECK_NAME && c->decks[i].name[k]; k++)
            BufAdd(b, "%c", c->decks[i].name[k] == '"' || c->decks[i].name[k] == '\\' ? '_' : c->decks[i].name[k]);
        BufAdd(b, "\",\"c\":[");
        for (k = 0; k < TT_HAND; k++)
            BufAdd(b, "%s%u", k ? "," : "", c->decks[i].cards[k]);
        BufAdd(b, "]}");
    }
    BufAdd(b, "],\"owned\":[");
    for (i = 1, n = 0; i <= TT_CARD_MAX; i++)
        if (c->owned[i])
            BufAdd(b, "%s[%u,%u]", n++ ? "," : "", i, c->owned[i]);
    BufAdd(b, "],\"seen\":[");
    for (i = 1, n = 0; i <= TT_CARD_MAX; i++)
        if (!c->owned[i] && Tt_Seen(c, i))
            BufAdd(b, "%s%u", n++ ? "," : "", i);
    BufAdd(b, "]}");
}

// ------------------------------------------------------------------------------------------------------ orders

static int JsonInts(const char *json, const char *key, long *out, int max)
{
    char pat[40];
    const char *p;
    int n = 0;
    snprintf(pat, sizeof(pat), "\"%s\":[", key);
    if ((p = strstr(json, pat)) == NULL)
        return -1;
    p += strlen(pat);
    while (*p && *p != ']' && n < max)
    {
        char *end;
        long v = strtol(p, &end, 10);
        if (end == p)
            break;
        out[n++] = v;
        p = end;
        while (*p == ',' || *p == ' ')
            p++;
    }
    return n;
}

static const char *Deck(const char *json)
{
    struct TtCollection *c = Tt_GetCollection();
    long long deck = -1;
    long cards[TT_HAND];
    uint16_t hand[TT_HAND];
    char name[TT_DECK_NAME + 1], reason[80];
    unsigned i, k;
    if (!JsonNum(json, "deck", &deck) || deck < 0 || deck >= TT_DECKS || JsonInts(json, "cards", cards, TT_HAND) != TT_HAND)
        return "bad";
    for (i = 0; i < TT_HAND; i++)
        hand[i] = (uint16_t)cards[i];
    if (!Tt_DeckValid(hand, reason, sizeof(reason)))
        return "deckrules";
    for (i = 0; i < TT_HAND; i++)
    {
        unsigned copies = 0;
        for (k = 0; k < TT_HAND; k++)
            copies += hand[k] == hand[i];
        if (c->owned[hand[i]] < copies)
            return "notowned";
    }
    memcpy(c->decks[deck].cards, hand, sizeof(hand));
    if (JsonStr(json, "name", name, sizeof(name)) && name[0])
    {
        for (i = 0; name[i]; i++)
            if (name[i] >= 'a' && name[i] <= 'z')
                name[i] -= 32;   // (the game's deck names are capitals)
        snprintf(c->decks[deck].name, sizeof(c->decks[deck].name), "%s", name);
    }
    {
        long long active = -1;
        if (JsonNum(json, "active", &active) && active == 1)
            c->activeDeck = (uint8_t)deck;
    }
    Tt_WriteSidecar();
    return NULL;
}

static const char *Win(const char *json)
{
    struct TtCollection *c = Tt_GetCollection();
    char kind[16];
    long long harder = 0, perfect = 0;
    unsigned pack;
    if (!JsonStr(json, "kind", kind, sizeof(kind)))
        return "bad";
    JsonNum(json, "harder", &harder);
    JsonNum(json, "perfect", &perfect);
    DailyToday();
    if (strcmp(kind, "puzzle") == 0)
    {
        if (sDaily.puzzles[0] >= PUZZLES_A_DAY)
            return "today";
        sDaily.puzzles[0]++;
        pack = 1;   // (SILVER)
    }
    else if (strcmp(kind, "duel") == 0)
    {
        if (sDaily.packs[0] >= DUEL_PACKS_A_DAY)
            return "today";
        sDaily.packs[0]++;
        pack = harder || perfect ? 1 : 0;   // (a tough or perfect win: SILVER, else BRONZE)
        c->wins++;
    }
    else
        return "bad";
    if (c->packs[pack] < 0xFFFF)
        c->packs[pack]++;
    DailyWrite();
    Tt_WriteSidecar();
    BufAdd(&sDoneExtra, "\"prize\":%u,\"left\":%u", pack,
           strcmp(kind, "puzzle") == 0 ? PUZZLES_A_DAY - sDaily.puzzles[0] : DUEL_PACKS_A_DAY - sDaily.packs[0]);
    MPNet_Log("phone cards: a %s won a %s pack", kind, Tt_PackName(pack));
    return NULL;
}

static const char *Open(const char *json)
{
    struct TtCollection *c = Tt_GetCollection();
    long long pack = -1;
    uint16_t out[TT_PACK_SIZE];
    unsigned i;
    if (!JsonNum(json, "pack", &pack) || pack < 0 || pack >= TT_PACK_TYPES)
        return "bad";
    if (!c->packs[pack])
        return "nopack";
    if (!Tt_OpenPack(c, (unsigned)pack, out))
        return "nopack";
    Tt_WriteSidecar();
    BufAdd(&sDoneExtra, "\"cards\":[");
    for (i = 0; i < TT_PACK_SIZE; i++)
        BufAdd(&sDoneExtra, "%s%u", i ? "," : "", out[i]);
    BufAdd(&sDoneExtra, "]");
    return NULL;
}

const char *PhoneCards_Apply(const char *op, const char *json)
{
    struct TtCollection *c = Tt_GetCollection();
    if (c == NULL || !c->optedIn)
        return "nocards";
    if (strcmp(op, "ttdeck") == 0)
        return Deck(json);
    if (strcmp(op, "ttwin") == 0)
        return Win(json);
    if (strcmp(op, "ttopen") == 0)
        return Open(json);
    return "unknown";
}
