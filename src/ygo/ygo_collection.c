/* The player's Yu-Gi-Oh! cards: decks, packs, the shop's prices, the save file. */
#include <stdio.h>
#include <string.h>
#include "mp_version.h"
#include "ygo/collection.h"
#include "ygo/duel.h"

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

unsigned Ygo_CopyLimit(unsigned card)
{
    const struct YgoCard *c = Ygo_Card(card);
    if (!c)
        return 0;
    return c->limit < YGO_COPIES ? c->limit : YGO_COPIES;
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

int Ygo_DeckRemove(struct YgoCollection *c, unsigned deck, unsigned card)
{
    if (deck >= YGO_SAVED_DECKS)
        return 0;
    return RemoveFrom(c->decks[deck].extra, YGO_FUSION_MAX, card) || RemoveFrom(c->decks[deck].main, YGO_DECK_MAX, card);
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
    snprintf(d->name, sizeof(d->name), "%s", s->name + (strncmp(s->name, "Starter Deck: ", 14) ? 0 : 14));
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
        /* Stairway's cards that no 2003 pack had: by how strong they are */
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
#define YGO_FILE_FORMAT 2   /* 2 (1.36.0): the GAME CENTER clerk's sold singles and decks; 1 still loads */
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
#define PAYLOAD_SIZE_1                                                                                        \
    (2 * (YGO_CARD_COUNT + 1) + YGO_SAVED_DECKS * (YGO_NAME_LEN + 2 * (YGO_DECK_MAX + YGO_FUSION_MAX)) +       \
     2 * YGO_SET_COUNT + 4 * 10 + 3 + 6 * YGO_DAILY_SLOTS)
#define PAYLOAD_SIZE (PAYLOAD_SIZE_1 + YGO_GC_SINGLE_BYTES + 1)

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
    Put(&p, c->gcDecks, 1);
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
    static unsigned char data[12 + PAYLOAD_SIZE];
    const unsigned char *p = data + 4;
    FILE *f = fopen(path, "rb");
    unsigned i, j, n, format, size;
    if (!f)
        return 0;
    n = (unsigned)fread(data, 1, sizeof(data), f);
    fclose(f);
    if (n < 12 || memcmp(data, "MYGO", 4))
        return 0;
    format = Get(&p, 4);
    size = format == 1 ? PAYLOAD_SIZE_1 : PAYLOAD_SIZE;
    if ((format != 1 && format != YGO_FILE_FORMAT) || n != 12 + size || Get(&p, 4) != Crc(data + 12, size))
        return 0;
    memset(c, 0, sizeof(*c));
    for (i = 0; i <= YGO_CARD_COUNT; i++)
        c->owned[i] = (uint16_t)Get(&p, 2);
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
    for (i = 0; i < YGO_SET_COUNT; i++)
        c->packs[i] = (uint16_t)Get(&p, 2);
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
        for (i = 0; i < YGO_GC_SINGLE_BYTES; i++)
            c->gcSingles[i] = (uint8_t)Get(&p, 1);
        c->gcDecks = (uint8_t)Get(&p, 1);
    }
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
