/* The player's Yu-Gi-Oh! cards: decks, packs, the shop's prices, the save file. */
#include <stdio.h>
#include <string.h>
#include "mp_version.h"
#include <stdlib.h>
#include "ygo/collection.h"
#include "ygo/duel.h"
#include "ygo_edo.h"

static uint32_t Rand(uint32_t *s)
{
    uint32_t x = *s ? *s : 0x2545F491u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *s = x;
}

void Ygo_CollectionInit(struct YgoCollection *c, uint32_t seed)
{
    memset(c, 0, sizeof(*c));
    c->rng = seed ? seed : 1;
}

/* ---- decks */
unsigned Ygo_DeckCount(const uint16_t *cards, unsigned max)
{
    unsigned n = 0;
    while (n < max && cards[n])
        n++;
    return n;
}

unsigned Ygo_GameLimit(unsigned card)
{
    const struct YgoCard *c = Ygo_Card(card);
    if (!c)
        return 0;
    return c->limit < YGO_COPIES ? c->limit : YGO_COPIES;
}
static unsigned sBanlist;   /* 0 the game's own, else EDOPro's list sBanlist - 1 */
unsigned Ygo_BanlistCount(void) { return 1 + YgoEdo_ListCount(); }
const char *Ygo_BanlistName(unsigned list) { return list ? YgoEdo_ListName(list - 1) : "MOEMON (STAIRWAY)"; }
unsigned Ygo_UseBanlist(const char *name)
{
    unsigned i, n = Ygo_BanlistCount();
    sBanlist = 0;
    for (i = 1; name && name[0] && i < n; i++)
        if (!strcmp(Ygo_BanlistName(i), name))
            sBanlist = i;
    return sBanlist;
}
unsigned Ygo_Banlist(void) { return sBanlist; }
unsigned Ygo_CopyLimit(unsigned card)
{
    unsigned n;
    if (!sBanlist)
        return Ygo_GameLimit(card);
    if (!Ygo_Card(card))
        return 0;
    n = YgoEdo_ListLimit(sBanlist - 1, YgoEdo_CodeOfId(card));
    return n < YGO_COPIES ? n : YGO_COPIES;
}

static unsigned Copies(const uint16_t *main, const uint16_t *extra, unsigned card)
{
    unsigned i, n = 0;
    for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
        n += main[i] == card;
    for (i = 0; extra && i < YGO_FUSION_MAX && extra[i]; i++)
        n += extra[i] == card;
    return n;
}

int Ygo_DeckValidate(const uint16_t *main, const uint16_t *extra, const uint16_t *owned, char *reason, size_t size)
{
    unsigned n = Ygo_DeckCount(main, YGO_DECK_MAX), e = extra ? Ygo_DeckCount(extra, YGO_FUSION_MAX) : 0, i;
    if (n < YGO_DECK_MIN)
    {
        snprintf(reason, size, "A deck needs at least %u cards (it has %u).", YGO_DECK_MIN, n);
        return 0;
    }
    for (i = 0; i < n + e; i++)
    {
        unsigned card = i < n ? main[i] : extra[i - n];
        const struct YgoCard *c = Ygo_Card(card);
        unsigned copies;
        if (!c || !Ygo_CardReady(card))
        {
            snprintf(reason, size, "%s can't be played yet.", c ? c->name : "A card");
            return 0;
        }
        if ((i < n) == (c->frame == YGO_FUSION))
        {
            snprintf(reason, size, "%s belongs in the %s.", c->name, c->frame == YGO_FUSION ? "Fusion Deck" : "Deck");
            return 0;
        }
        copies = Copies(main, extra, card);
        if (copies > Ygo_CopyLimit(card))
        {
            if (Ygo_CopyLimit(card))
                snprintf(reason, size, "Only %u %s in a deck.", Ygo_CopyLimit(card), c->name);
            else
                snprintf(reason, size, "%s is Forbidden.", c->name);
            return 0;
        }
        if (owned && copies > owned[card])
        {
            snprintf(reason, size, "You have %u %s.", owned[card], c->name);
            return 0;
        }
    }
    return 1;
}

int Ygo_DeckAdd(struct YgoCollection *c, unsigned deck, unsigned card, char *reason, size_t size)
{
    struct YgoSavedDeck *d;
    const struct YgoCard *k = Ygo_Card(card);
    unsigned copies;
    if (deck >= YGO_SAVED_DECKS || !k)
        return 0;
    d = &c->decks[deck];
    if (!Ygo_CardReady(card))
    {
        snprintf(reason, size, "%s can't be played yet.", k->name);
        return 0;
    }
    copies = Copies(d->main, d->extra, card);
    if (copies >= Ygo_CopyLimit(card))
    {
        if (Ygo_CopyLimit(card))
            snprintf(reason, size, "Only %u %s in a deck.", Ygo_CopyLimit(card), k->name);
        else
            snprintf(reason, size, "%s is Forbidden.", k->name);
        return 0;
    }
    if (copies >= c->owned[card])
    {
        snprintf(reason, size, "You have %u %s.", c->owned[card], k->name);
        return 0;
    }
    if (k->frame == YGO_FUSION)
    {
        unsigned n = Ygo_DeckCount(d->extra, YGO_FUSION_MAX);
        if (n >= YGO_FUSION_MAX)
        {
            snprintf(reason, size, "The Fusion Deck holds %u.", YGO_FUSION_MAX);
            return 0;
        }
        d->extra[n] = card;
    }
    else
    {
        unsigned n = Ygo_DeckCount(d->main, YGO_DECK_MAX);
        if (n >= YGO_DECK_MAX)
        {
            snprintf(reason, size, "A deck holds %u cards.", YGO_DECK_MAX);
            return 0;
        }
        d->main[n] = card;
    }
    return 1;
}

static int RemoveFrom(uint16_t *cards, unsigned max, unsigned card)
{
    unsigned i, n = Ygo_DeckCount(cards, max);
    for (i = 0; i < n; i++)
        if (cards[i] == card)
        {
            memmove(cards + i, cards + i + 1, (n - i - 1) * sizeof(*cards));
            cards[n - 1] = 0;
            return 1;
        }
    return 0;
}

unsigned Ygo_DeckMascot(const uint16_t *main, unsigned count)
{
    unsigned i, best = 0;
    int bestAtk = -1;
    for (i = 0; i < count && main[i]; i++)
    {
        const struct YgoCard *c = Ygo_Card(main[i]);
        int atk = c && Ygo_IsMonster(c) && c->atk != YGO_STAT_UNKNOWN ? c->atk : -1;
        if (atk > bestAtk)
            bestAtk = atk, best = main[i];
    }
    return best;
}

unsigned Ygo_DeckAce(const struct YgoCollection *c, unsigned deck)
{
    const struct YgoSavedDeck *d;
    if (deck >= YGO_SAVED_DECKS)
        return 0;
    d = &c->decks[deck];
    if (c->ace[deck] && Copies(d->main, d->extra, c->ace[deck]))
        return c->ace[deck];
    return Ygo_DeckMascot(d->main, YGO_DECK_MAX);
}

unsigned Ygo_RecipeOwned(const struct YgoCollection *c, const uint16_t *main, const uint16_t *extra, unsigned *total)
{
    static uint16_t used[YGO_CARD_COUNT + 1];
    unsigned i, n = 0, all = 0;
    memset(used, 0, sizeof(used));
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        unsigned id = i < YGO_DECK_MAX ? main[i] : extra ? extra[i - YGO_DECK_MAX] : 0;
        if (!id || id > YGO_CARD_COUNT)
            continue;
        all++;
        if (used[id]++ < c->owned[id])
            n++;
    }
    if (total)
        *total = all;
    return n;
}

unsigned Ygo_DeckFromRecipe(struct YgoCollection *c, unsigned deck, const uint16_t *main, const uint16_t *extra,
                            const char *name, unsigned *missing)
{
    static uint16_t used[YGO_CARD_COUNT + 1];
    struct YgoSavedDeck *d;
    unsigned i, n = 0, e = 0, lost = 0;
    if (deck >= YGO_SAVED_DECKS)
        return 0;
    d = &c->decks[deck];
    memset(d, 0, sizeof(*d));
    memset(used, 0, sizeof(used));
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        int fusionList = i >= YGO_DECK_MAX;
        unsigned id = !fusionList ? main[i] : extra ? extra[i - YGO_DECK_MAX] : 0;
        if (!id || id > YGO_CARD_COUNT)
            continue;
        if (used[id] >= c->owned[id] || !Ygo_CardReady(id) || used[id] >= Ygo_CopyLimit(id))
        {
            lost++;
            continue;
        }
        used[id]++;
        if (fusionList && e < YGO_FUSION_MAX)
            d->extra[e++] = (uint16_t)id;
        else if (!fusionList && n < YGO_DECK_MAX)
            d->main[n++] = (uint16_t)id;
    }
    snprintf(d->name, YGO_NAME_LEN, "%s", name);
    c->ace[deck] = 0;
    if (missing)
        *missing = lost;
    return n + e;
}

int Ygo_DeckRemove(struct YgoCollection *c, unsigned deck, unsigned card)
{
    if (deck >= YGO_SAVED_DECKS)
        return 0;
    return RemoveFrom(c->decks[deck].extra, YGO_FUSION_MAX, card) || RemoveFrom(c->decks[deck].main, YGO_DECK_MAX, card);
}

/* A set's name without "Starter Deck: " / "Structure Deck: " (a deck's name, a short row) */
const char *Ygo_SetShortName(unsigned set)
{
    const char *n = set < YGO_SET_COUNT ? gYgoSets[set].name : "";
    if (!strncmp(n, "Starter Deck: ", 14))
        return n + 14;
    if (!strncmp(n, "Structure Deck: ", 16))
        return n + 16;
    return n;
}

int Ygo_ChooseStarter(struct YgoCollection *c, unsigned set)
{
    const struct YgoSet *s;
    struct YgoSavedDeck *d = &c->decks[0];
    unsigned i, k, n = 0, e = 0;
    if (set >= YGO_SET_COUNT || !gYgoSets[set].starter || c->starter)
        return 0;
    s = &gYgoSets[set];
    memset(d, 0, sizeof(*d));
    snprintf(d->name, sizeof(d->name), "%s", Ygo_SetShortName(set));
    for (i = 0; i < s->count; i++)
    {
        const struct YgoCard *card = Ygo_Card(s->cards[i].card);
        for (k = 0; k < s->cards[i].copies; k++)
        {
            if (c->owned[card->id] < YGO_COLLECTION_MAX)
                c->owned[card->id]++;
            if (!Ygo_CardReady(card->id))
                continue;
            if (card->frame == YGO_FUSION && e < YGO_FUSION_MAX)
                d->extra[e++] = card->id;
            else if (card->frame != YGO_FUSION && n < YGO_DECK_MAX)
                d->main[n++] = card->id;
        }
    }
    c->starter = set + 1;
    c->optedIn = 1;
    c->activeDeck = 0;
    return 1;
}

/* ---- packs */
int Ygo_SetAvailable(unsigned set)
{
    const struct YgoSet *s;
    unsigned i, ready = 0;
    if (set >= YGO_SET_COUNT || gYgoSets[set].starter)
        return 0;
    s = &gYgoSets[set];
    for (i = 0; i < s->count; i++)
        ready += Ygo_CardReady(s->cards[i].card);
    return ready * 10 >= s->count * 9;
}

unsigned Ygo_RandomSet(uint32_t *rng)
{
    unsigned sets[YGO_SET_COUNT], n = 0, i;
    for (i = 0; i < YGO_SET_COUNT; i++)
        if (Ygo_SetAvailable(i))
            sets[n++] = i;
    return n ? sets[Rand(rng) % n] : YGO_SET_LOB;
}

/* A random working card of set with rarity (or a lower one if none works) */
static unsigned DrawRarity(uint32_t *rng, const struct YgoSet *s, unsigned rarity)
{
    for (;;)
    {
        unsigned i, total = 0, pick;
        for (i = 0; i < s->count; i++)
            if (Ygo_CardReady(s->cards[i].card))
            {
                unsigned r = s->cards[i].rarity;
                if (rarity <= YGO_SUPER_SHORT_PRINT ? r <= YGO_SUPER_SHORT_PRINT : r == rarity)
                    total += rarity <= YGO_SUPER_SHORT_PRINT ? (r == YGO_COMMON ? 4 : r == YGO_SHORT_PRINT ? 2 : 1) : 1;
            }
        if (total)
        {
            pick = Rand(rng) % total;
            for (i = 0; i < s->count; i++)
                if (Ygo_CardReady(s->cards[i].card))
                {
                    unsigned r = s->cards[i].rarity, w;
                    if (rarity <= YGO_SUPER_SHORT_PRINT ? r > YGO_SUPER_SHORT_PRINT : r != rarity)
                        continue;
                    w = rarity <= YGO_SUPER_SHORT_PRINT ? (r == YGO_COMMON ? 4 : r == YGO_SHORT_PRINT ? 2 : 1) : 1;
                    if (pick < w)
                        return s->cards[i].card;
                    pick -= w;
                }
        }
        if (rarity <= YGO_COMMON)
            return 0;
        rarity = rarity == YGO_RARE ? YGO_COMMON : rarity - 1;
    }
}

int Ygo_OpenPack(struct YgoCollection *c, unsigned set, uint16_t result[YGO_PACK_SIZE])
{
    const struct YgoSet *s;
    unsigned i, roll, rare;
    if (set >= YGO_SET_COUNT || !c->packs[set] || gYgoSets[set].starter)
        return 0;
    s = &gYgoSets[set];
    for (i = 0; i < YGO_PACK_SIZE - 1; i++)
        result[i] = DrawRarity(&c->rng, s, YGO_COMMON);
    roll = Rand(&c->rng) % YGO_SECRET_ODDS;
    rare = roll == 0                                                    ? YGO_SECRET_RARE
           : roll < YGO_SECRET_ODDS / YGO_ULTRA_ODDS + 1                ? YGO_ULTRA_RARE
           : roll < YGO_SECRET_ODDS / YGO_ULTRA_ODDS + 1 + YGO_SECRET_ODDS / YGO_SUPER_ODDS ? YGO_SUPER_RARE
                                                                        : YGO_RARE;
    result[YGO_PACK_SIZE - 1] = DrawRarity(&c->rng, s, rare);
    for (i = 0; i < YGO_PACK_SIZE; i++)
        if (result[i] && c->owned[result[i]] < YGO_COLLECTION_MAX)
            c->owned[result[i]]++;
    c->packs[set]--;
    c->packsOpened++;
    return 1;
}

int Ygo_AwardPack(struct YgoCollection *c, unsigned set)
{
    if (set >= YGO_SET_COUNT || gYgoSets[set].starter || c->packs[set] >= 999)
        return 0;
    c->packs[set]++;
    return 1;
}

/* ---- spares, points, the shop */
static unsigned BestRarity(unsigned card)
{
    const struct YgoCard *c = Ygo_Card(card);
    return c ? c->rarity : 0;
}

unsigned Ygo_Spare(const struct YgoCollection *c, unsigned card)
{
    unsigned need = 0, d, keep;
    for (d = 0; d < YGO_SAVED_DECKS; d++)
    {
        unsigned n = Copies(c->decks[d].main, c->decks[d].extra, card);
        if (n > need)
            need = n;
    }
    keep = need > YGO_COPIES ? need : YGO_COPIES;
    return c->owned[card] > keep ? c->owned[card] - keep : 0;
}

unsigned Ygo_PointValue(unsigned card)
{
    static const uint8_t values[] = {1, 2, 3, 3, 10, 20, 30};
    return values[BestRarity(card)];
}

unsigned Ygo_PointCost(unsigned card)
{
    static const uint8_t costs[] = {5, 8, 12, 15, 50, 100, 150};
    const struct YgoCard *c = Ygo_Card(card);
    if (!c || !Ygo_CardReady(card) || c->frame == YGO_TOKEN)
        return 0;
    if (!c->sets)
    {
        /* cards no real set had (Stairway's and World Championship 2006's own): by how strong they are */
        if (c->frame <= YGO_RITUAL && (c->level >= 7 || (c->atk != YGO_STAT_UNKNOWN && c->atk >= 2500)))
            return 50;
        return c->frame == YGO_NORMAL ? 5 : 15;
    }
    return costs[BestRarity(card)];
}

unsigned Ygo_MoneyPrice(unsigned card)
{
    switch (Ygo_PointCost(card))
    {
    case 0: return 0;
    case 5: return 300;
    case 8: return 400;
    case 12: return 500;
    case 15: return 800;
    case 50: return 2000;
    case 100: return 3000;
    default: return 4000;
    }
}

unsigned Ygo_SetOwned(const struct YgoCollection *c, unsigned set, unsigned *total)
{
    unsigned i, n = 0, t = 0;
    if (set >= YGO_SET_COUNT)
    {
        for (i = 1; i <= YGO_CARD_COUNT; i++)
            if (Ygo_Card(i)->frame != YGO_TOKEN)
            {
                t++;
                n += c->owned[i] > 0;
            }
    }
    else
        for (i = 0; i < gYgoSets[set].count; i++)
        {
            t++;
            n += c->owned[gYgoSets[set].cards[i].card] > 0;
        }
    if (total)
        *total = t;
    return n;
}

int Ygo_DailyAvailable(const struct YgoCollection *c, uint32_t key, unsigned day)
{
    unsigned i;
    for (i = 0; i < YGO_DAILY_SLOTS; i++)
        if (c->daily[i].key == key && c->daily[i].day == (uint16_t)day)
            return 0;
    return 1;
}

void Ygo_DailyTake(struct YgoCollection *c, uint32_t key, unsigned day)
{
    unsigned i, oldest = 0;
    int16_t worst = -32768;
    for (i = 0; i < YGO_DAILY_SLOTS; i++)
    {
        int16_t age;
        if (c->daily[i].key == key || !c->daily[i].key)
        {
            oldest = i;
            break;
        }
        age = (int16_t)((uint16_t)day - c->daily[i].day);
        if (age > worst)
        {
            worst = age;
            oldest = i;
        }
    }
    c->daily[oldest].key = key;
    c->daily[oldest].day = (uint16_t)day;
}

/* ---- the file: "MYGO", u32 format, u32 crc of the rest, then the fields, little-endian */
#define YGO_FILE_FORMAT 6   /* 2 (1.36.0): the GAME CENTER clerk's sold singles and decks; 3 (1.38.0): each deck's
                                   ACE CARD; 4 (1.48.0): the Forbidden & Limited list; 5 (1.49.0): the skill
                                   correction; 6 (1.56.0): the card and set counts it was saved with (the catalog grows),
                                   32 bits of sold decks; 1-5 still load (1509 cards, 13 sets) */
#define FORMAT5_CARDS 1509
#define FORMAT5_SETS 13
static void Put(unsigned char **p, uint32_t v, unsigned n)
{
    unsigned i;
    for (i = 0; i < n; i++)
        *(*p)++ = (unsigned char)(v >> (8 * i));
}
static uint32_t Get(const unsigned char **p, unsigned n)
{
    uint32_t v = 0;
    unsigned i;
    for (i = 0; i < n; i++)
        v |= (uint32_t)*(*p)++ << (8 * i);
    return v;
}
static uint32_t Crc(const unsigned char *p, size_t n)
{
    uint32_t c = 0xFFFFFFFFu;
    size_t i;
    unsigned k;
    for (i = 0; i < n; i++)
    {
        c ^= p[i];
        for (k = 0; k < 8; k++)
            c = (c >> 1) ^ (0xEDB88320u & (0u - (c & 1)));
    }
    return ~c;
}
/* The payload of a file of this format saved with this many cards and sets */
static size_t PayloadSize(unsigned format, unsigned cards, unsigned sets)
{
    size_t n = 2 * (cards + 1) + YGO_SAVED_DECKS * (YGO_NAME_LEN + 2 * (YGO_DECK_MAX + YGO_FUSION_MAX)) + 2 * sets +
               4 * 10 + 3 + 6 * YGO_DAILY_SLOTS;
    if (format >= 2)
        n += (cards + 8) / 8 + (format >= 6 ? 4 : 1);
    if (format >= 3)
        n += 2 * YGO_SAVED_DECKS;
    if (format >= 4)
        n += 32;
    if (format >= 5)
        n += 4;
    if (format >= 6)
        n += 4;   /* the counts */
    return n;
}
#define PAYLOAD_SIZE (4 * 10 + 3 + 6 * YGO_DAILY_SLOTS + 2 * (YGO_CARD_COUNT + 1) +                                    \
                      YGO_SAVED_DECKS * (YGO_NAME_LEN + 2 * (YGO_DECK_MAX + YGO_FUSION_MAX)) + 2 * YGO_SET_COUNT +     \
                      YGO_GC_SINGLE_BYTES + 4 + 2 * YGO_SAVED_DECKS + 32 + 4 + 4)
/* (the biggest file loaded: today's, or a format 5 one if the catalog were ever smaller) */
#define LOAD_MAX (PAYLOAD_SIZE + 2 * (FORMAT5_CARDS + 1) + 2 * FORMAT5_SETS + (FORMAT5_CARDS + 8) / 8)

int Ygo_CollectionSave(const struct YgoCollection *c, const char *path)
{
#if MOEMON_DEV_BUILD
    (void)c;
    (void)path;
    return 0;
#else
    static unsigned char data[12 + PAYLOAD_SIZE];
    unsigned char *p = data + 12;
    unsigned i, j, ok;
    char tmp[1024];
    FILE *f;
    if (strlen(path) > sizeof(tmp) - 5)
        return 0;
    Put(&p, YGO_CARD_COUNT, 2);
    Put(&p, YGO_SET_COUNT, 2);
    for (i = 0; i <= YGO_CARD_COUNT; i++)
        Put(&p, c->owned[i], 2);
    for (i = 0; i < YGO_SAVED_DECKS; i++)
    {
        memcpy(p, c->decks[i].name, YGO_NAME_LEN);
        p += YGO_NAME_LEN;
        for (j = 0; j < YGO_DECK_MAX; j++)
            Put(&p, c->decks[i].main[j], 2);
        for (j = 0; j < YGO_FUSION_MAX; j++)
            Put(&p, c->decks[i].extra[j], 2);
    }
    for (i = 0; i < YGO_SET_COUNT; i++)
        Put(&p, c->packs[i], 2);
    Put(&p, c->wins, 4);
    Put(&p, c->losses, 4);
    Put(&p, c->rng, 4);
    Put(&p, c->owner, 4);
    Put(&p, c->flags, 4);
    Put(&p, c->packsOpened, 4);
    Put(&p, c->onlineWins, 4);
    Put(&p, c->trades, 4);
    Put(&p, c->specialBeaten, 4);
    Put(&p, c->promoGiven, 4);
    Put(&p, c->optedIn, 1);
    Put(&p, c->activeDeck, 1);
    Put(&p, c->starter, 1);
    for (i = 0; i < YGO_DAILY_SLOTS; i++)
    {
        Put(&p, c->daily[i].key, 4);
        Put(&p, c->daily[i].day, 2);
    }
    for (i = 0; i < YGO_GC_SINGLE_BYTES; i++)
        Put(&p, c->gcSingles[i], 1);
    Put(&p, c->gcDecks, 4);
    for (i = 0; i < YGO_SAVED_DECKS; i++)
        Put(&p, c->ace[i], 2);
    memcpy(p, c->banlist, 32);
    p += 32;
    Put(&p, (uint16_t)c->skill, 2);
    Put(&p, c->rated, 2);
    if (p != data + 12 + PayloadSize(YGO_FILE_FORMAT, YGO_CARD_COUNT, YGO_SET_COUNT))
        return 0;
    p = data;
    memcpy(p, "MYGO", 4);
    p += 4;
    Put(&p, YGO_FILE_FORMAT, 4);
    Put(&p, Crc(data + 12, PAYLOAD_SIZE), 4);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return 0;
    ok = fwrite(data, 1, sizeof(data), f) == sizeof(data);
    if (fclose(f))
        ok = 0;
    if (!ok)
    {
        remove(tmp);
        return 0;
    }
    remove(path); /* Windows' rename never replaces a file */
    if (rename(tmp, path))
    {
        remove(tmp);
        return 0;
    }
    return 1;
#endif
}

int Ygo_CollectionLoad(struct YgoCollection *c, const char *path)
{
    static unsigned char data[12 + LOAD_MAX + 1];
    const unsigned char *p = data + 4;
    FILE *f = fopen(path, "rb");
    unsigned i, j, n, format, cards = FORMAT5_CARDS, sets = FORMAT5_SETS;
    size_t size;
    uint32_t crc;
    if (!f)
        return 0;
    n = (unsigned)fread(data, 1, sizeof(data), f);
    fclose(f);
    if (n < 16 || memcmp(data, "MYGO", 4))
        return 0;
    format = Get(&p, 4);
    crc = Get(&p, 4);
    if (format >= 6)
    {
        cards = Get(&p, 2);
        sets = Get(&p, 2);
        if (cards > YGO_CARD_COUNT + FORMAT5_CARDS || sets > YGO_SET_COUNT + FORMAT5_SETS)
            return 0;
    }
    size = PayloadSize(format, cards, sets);
    if (format < 1 || format > YGO_FILE_FORMAT || n != 12 + size || crc != Crc(data + 12, size))
        return 0;
    memset(c, 0, sizeof(*c));
    /* (cards and sets the catalog no longer has are dropped; ones it has since gained start at 0) */
    for (i = 0; i <= cards; i++)
    {
        unsigned v = Get(&p, 2);
        if (i <= YGO_CARD_COUNT)
            c->owned[i] = (uint16_t)v;
    }
    for (i = 0; i < YGO_SAVED_DECKS; i++)
    {
        memcpy(c->decks[i].name, p, YGO_NAME_LEN);
        c->decks[i].name[YGO_NAME_LEN - 1] = 0;
        p += YGO_NAME_LEN;
        for (j = 0; j < YGO_DECK_MAX; j++)
            c->decks[i].main[j] = (uint16_t)Get(&p, 2);
        for (j = 0; j < YGO_FUSION_MAX; j++)
            c->decks[i].extra[j] = (uint16_t)Get(&p, 2);
    }
    for (i = 0; i < sets; i++)
    {
        unsigned v = Get(&p, 2);
        if (i < YGO_SET_COUNT)
            c->packs[i] = (uint16_t)v;
    }
    c->wins = Get(&p, 4);
    c->losses = Get(&p, 4);
    c->rng = Get(&p, 4);
    c->owner = Get(&p, 4);
    c->flags = Get(&p, 4);
    c->packsOpened = Get(&p, 4);
    c->onlineWins = Get(&p, 4);
    c->trades = Get(&p, 4);
    c->specialBeaten = Get(&p, 4);
    c->promoGiven = Get(&p, 4);
    c->optedIn = (uint8_t)Get(&p, 1);
    c->activeDeck = (uint8_t)Get(&p, 1);
    c->starter = (uint8_t)Get(&p, 1);
    for (i = 0; i < YGO_DAILY_SLOTS; i++)
    {
        c->daily[i].key = Get(&p, 4);
        c->daily[i].day = (uint16_t)Get(&p, 2);
    }
    if (format >= 2)
    {
        for (i = 0; i < (cards + 8) / 8; i++)
        {
            unsigned v = Get(&p, 1);
            if (i < YGO_GC_SINGLE_BYTES)
                c->gcSingles[i] = (uint8_t)v;
        }
        c->gcDecks = Get(&p, format >= 6 ? 4 : 1);
    }
    if (format >= 3)
        for (i = 0; i < YGO_SAVED_DECKS; i++)
        {
            c->ace[i] = (uint16_t)Get(&p, 2);
            if (c->ace[i] > YGO_CARD_COUNT)
                c->ace[i] = 0;
        }
    if (format >= 4)
    {
        memcpy(c->banlist, p, 32);
        c->banlist[31] = 0;
        p += 32;
    }
    if (format >= 5)
    {
        int skill = (int16_t)Get(&p, 2);
        c->skill = (int16_t)(skill > YGO_SKILL_MAX ? YGO_SKILL_MAX : skill < -YGO_SKILL_MAX ? -YGO_SKILL_MAX : skill);
        c->rated = (uint16_t)Get(&p, 2);
    }
    if (c->starter > YGO_SET_COUNT)
        c->starter = 0;
    /* sanity: ids in range */
    for (i = 0; i < YGO_SAVED_DECKS; i++)
    {
        for (j = 0; j < YGO_DECK_MAX; j++)
            if (c->decks[i].main[j] > YGO_CARD_COUNT)
                c->decks[i].main[j] = 0;
        for (j = 0; j < YGO_FUSION_MAX; j++)
            if (c->decks[i].extra[j] > YGO_CARD_COUNT)
                c->decks[i].extra[j] = 0;
    }
    if (c->activeDeck >= YGO_SAVED_DECKS)
        c->activeDeck = 0;
    return 1;
}

/* ---- deck codes */
static const char sB64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int B64(int c)
{
    const char *p = c ? strchr(sB64, c) : NULL;
    if (c == '-')
        return 62;   /* (the URL-safe spelling) */
    if (c == '_')
        return 63;
    return p ? (int)(p - sB64) : -1;
}
/* One part (up to '!'): passwords, 4 bytes each, into the deck; returns the characters read, -1 not base64 */
static int ReadPart(const char *s, uint32_t *codes, unsigned max, unsigned *count)
{
    uint8_t bytes[4];
    unsigned nb = 0, acc = 0, bits = 0;
    int i;
    *count = 0;
    for (i = 0; s[i] && s[i] != '!'; i++)
    {
        int v;
        if (s[i] == '=' || s[i] == ' ' || s[i] == '\r' || s[i] == '\n' || s[i] == '\t')
            continue;
        if ((v = B64((unsigned char)s[i])) < 0)
            return -1;
        acc = (acc << 6) | (unsigned)v;
        bits += 6;
        if (bits >= 8)
        {
            bits -= 8;
            bytes[nb++] = (uint8_t)(acc >> bits);
            acc &= (1u << bits) - 1;
            if (nb == 4)
            {
                if (*count < max)
                    codes[(*count)++] = bytes[0] | bytes[1] << 8 | bytes[2] << 16 | (uint32_t)bytes[3] << 24;
                nb = 0;
            }
        }
    }
    return i;
}
static void Place(const uint32_t *codes, unsigned n, uint16_t *mainOut, uint16_t *extraOut, unsigned *mains,
                  unsigned *extras, unsigned *notHere, int extraPart)
{
    unsigned i;
    for (i = 0; i < n; i++)
    {
        unsigned id = YgoEdo_CatalogOf(codes[i]);
        const struct YgoCard *c = id ? Ygo_Card(id) : NULL;
        if (!c || !Ygo_CardReady(id) || c->frame == YGO_TOKEN)
        {
            (*notHere)++;
            continue;
        }
        if (c->frame == YGO_FUSION || extraPart)
        {
            if (c->frame == YGO_FUSION && *extras < YGO_FUSION_MAX)
                extraOut[(*extras)++] = (uint16_t)id;
            else if (c->frame != YGO_FUSION && *mains < YGO_DECK_MAX)
                mainOut[(*mains)++] = (uint16_t)id;
        }
        else if (*mains < YGO_DECK_MAX)
            mainOut[(*mains)++] = (uint16_t)id;
    }
}
int Ygo_ReadYdke(const char *text, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX], unsigned *notHere)
{
    static uint32_t codes[200];
    const char *s = text ? strstr(text, "ydke://") : NULL;
    unsigned n, mains = 0, extras = 0, part;
    int used;
    memset(main, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    memset(extra, 0, sizeof(uint16_t) * YGO_FUSION_MAX);
    *notHere = 0;
    if (!s)
        return 0;
    s += 7;
    for (part = 0; part < 2; part++)
    {
        if ((used = ReadPart(s, codes, 200, &n)) < 0 || s[used] != '!')
            return 0;
        Place(codes, n, main, extra, &mains, &extras, notHere, part == 1);
        s += used + 1;
    }
    return mains + extras + *notHere > 0;
}
int Ygo_ReadYdk(const char *text, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX], unsigned *notHere)
{
    static uint32_t codes[200];
    unsigned n = 0, mains = 0, extras = 0, section = 0, any = 0;
    const char *line = text;
    memset(main, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    memset(extra, 0, sizeof(uint16_t) * YGO_FUSION_MAX);
    *notHere = 0;
    while (line && *line)
    {
        const char *end = line + strcspn(line, "\r\n");
        if (!strncmp(line, "#main", 5) || !strncmp(line, "#extra", 6) || !strncmp(line, "!side", 5))
        {
            Place(codes, n, main, extra, &mains, &extras, notHere, section == 1);
            n = 0;
            section = line[1] == 'm' ? 0 : line[1] == 'e' ? 1 : 2;
            any = 1;
        }
        else if (*line >= '0' && *line <= '9' && section < 2 && n < 200)
            codes[n++] = (uint32_t)strtoul(line, NULL, 10);
        line = *end ? end + 1 + (end[0] == '\r' && end[1] == '\n') : end;
    }
    if (section < 2)
        Place(codes, n, main, extra, &mains, &extras, notHere, section == 1);
    return any && mains + extras + *notHere > 0;
}
static int WritePart(const uint16_t *ids, unsigned max, char *buf, size_t size, size_t *len)
{
    uint8_t bytes[4 * (YGO_DECK_MAX + YGO_FUSION_MAX)];
    unsigned n = 0, i;
    for (i = 0; i < max && ids[i]; i++)
    {
        uint32_t code = YgoEdo_CodeOfId(ids[i]);
        bytes[n++] = (uint8_t)code;
        bytes[n++] = (uint8_t)(code >> 8);
        bytes[n++] = (uint8_t)(code >> 16);
        bytes[n++] = (uint8_t)(code >> 24);
    }
    for (i = 0; i < n; i += 3)
    {
        uint32_t v = (uint32_t)bytes[i] << 16 | (i + 1 < n ? (uint32_t)bytes[i + 1] << 8 : 0) | (i + 2 < n ? bytes[i + 2] : 0);
        if (*len + 5 > size)
            return 0;
        buf[(*len)++] = sB64[v >> 18 & 63];
        buf[(*len)++] = sB64[v >> 12 & 63];
        buf[(*len)++] = i + 1 < n ? sB64[v >> 6 & 63] : '=';
        buf[(*len)++] = i + 2 < n ? sB64[v & 63] : '=';
    }
    if (*len + 2 > size)
        return 0;
    buf[(*len)++] = '!';
    buf[*len] = 0;
    return 1;
}
int Ygo_WriteYdke(const uint16_t *main, const uint16_t *extra, char *buf, size_t size)
{
    size_t len;
    if (size < 16)
        return 0;
    memcpy(buf, "ydke://", 7);
    len = 7;
    if (!WritePart(main, YGO_DECK_MAX, buf, size, &len) || !WritePart(extra, YGO_FUSION_MAX, buf, size, &len))
        return 0;
    if (len + 2 > size)
        return 0;
    buf[len++] = '!';   /* (no Side Deck) */
    buf[len] = 0;
    return 1;
}
int Ygo_WriteYdk(const uint16_t *main, const uint16_t *extra, const char *name, char *buf, size_t size)
{
    size_t len = 0;
    unsigned i;
    int n = snprintf(buf, size, "#created by Moemon Emerald Kai Co-op: %s\n#main\n", name ? name : "");
    if (n < 0 || (size_t)n >= size)
        return 0;
    len = (size_t)n;
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX + 1; i++)
    {
        unsigned id = i < YGO_DECK_MAX ? main[i] : i < YGO_DECK_MAX + YGO_FUSION_MAX ? extra[i - YGO_DECK_MAX] : 0;
        if (i == YGO_DECK_MAX)
            n = snprintf(buf + len, size - len, "#extra\n");
        else if (i == YGO_DECK_MAX + YGO_FUSION_MAX)
            n = snprintf(buf + len, size - len, "!side\n");
        else
            n = 0;
        if (n < 0 || len + (size_t)n >= size)
            return 0;
        len += (size_t)n;
        if (id)
        {
            n = snprintf(buf + len, size - len, "%u\n", (unsigned)YgoEdo_CodeOfId(id));
            if (n < 0 || len + (size_t)n >= size)
                return 0;
            len += (size_t)n;
        }
    }
    return 1;
}
unsigned Ygo_DeckMissing(const struct YgoCollection *c, const uint16_t *main, const uint16_t *extra)
{
    static uint16_t used[YGO_CARD_COUNT + 1];
    unsigned i, n = 0;
    memset(used, 0, sizeof(used));
    for (i = 0; i < YGO_DECK_MAX + YGO_FUSION_MAX; i++)
    {
        unsigned id = i < YGO_DECK_MAX ? main[i] : extra[i - YGO_DECK_MAX];
        if (id && id <= YGO_CARD_COUNT && used[id]++ >= c->owned[id])
            n++;
    }
    return n;
}
