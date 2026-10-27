#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/tcg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const struct TcgCard *Tcg_Card(unsigned id) { return id && id <= TCG_CARD_COUNT ? &gTcgCards[id] : NULL; }
uint32_t Tcg_Random(uint32_t *state)
{
    uint32_t x = *state ? *state : 0x6d2b79f5u;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return *state = x;
}
unsigned Tcg_RandomBelow(uint32_t *state, unsigned limit)
{
    uint32_t value, threshold;
    if (!limit)
        return 0;
    threshold = (uint32_t)(-limit) % limit;
    do
    {
        value = Tcg_Random(state);
    } while (value < threshold);
    return value % limit;
}
static int Invalid(char *reason, size_t size, const char *text)
{
    if (size)
        snprintf(reason, size, "%s", text);
    return 0;
}
int Tcg_DeckValidate(const uint16_t cards[TCG_DECK_SIZE], const uint16_t *owned, char *reason, size_t size)
{
    unsigned counts[TCG_CARD_COUNT + 1] = {0}, basic = 0, i, j;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *c = Tcg_Card(cards[i]);
        unsigned same = 0;
        if (!c)
            return Invalid(reason, size, "A deck must contain exactly 60 valid cards.");
        counts[c->id]++;
        if (c->kind == TCG_POKEMON && !c->stage)
            basic++;
        /* Six Basic Energies are freely supplied; Double Colorless is not. */
        if (c->id <= 6)
            continue;
        if (owned && counts[c->id] > owned[c->id])
            return Invalid(reason, size, "You do not own enough copies of a card.");
        for (j = 0; j <= i; j++)
            if (!strcmp(gTcgCards[cards[j]].name, c->name))
                same++;
        if (same > 4)
            return Invalid(reason, size, "Only four cards with the same name are allowed.");
    }
    if (!basic)
        return Invalid(reason, size, "A deck needs at least one Basic Pokemon.");
    if (size)
        reason[0] = 0;
    return 1;
}
void Tcg_CollectionInit(struct TcgCollection *c, uint32_t seed)
{
    memset(c, 0, sizeof(*c));
    c->rng = seed ? seed : 1;
    c->quiet = 1;
}
int Tcg_ChooseStarter(struct TcgCollection *c, unsigned starter)
{
    static const char *names[] = {"CharmanderAndFriends", "SquirtleAndFriends", "BulbasaurAndFriends"};
    unsigned i, j;
    if (c->optedIn || starter >= 3)
        return 0;
    for (i = 0; i < gTcgDeckDefinitionCount; i++)
        if (!strcmp(gTcgDeckDefinitions[i].name, names[starter]))
        {
            memcpy(c->decks[0].cards, gTcgDeckDefinitions[i].cards, sizeof(c->decks[0].cards));
            snprintf(c->decks[0].name, TCG_NAME_LEN, "%s",
                     starter == 0   ? "Ember Friends"
                     : starter == 1 ? "Tidal Friends"
                                    : "Garden Friends");
            for (j = 0; j < TCG_DECK_SIZE; j++)
                c->owned[c->decks[0].cards[j]]++;
            for (j = 1; j <= 6; j++)
                c->owned[j] = TCG_DECK_SIZE;
            c->optedIn = 1;
            c->activeDeck = 0;
            return 1;
        }
    return 0;
}
int Tcg_AwardPack(struct TcgCollection *c, unsigned pack)
{
    if (!c->optedIn || pack >= TCG_PACK_TYPES || c->packs[pack] == 65535)
        return 0;
    c->packs[pack]++;
    c->wins++;
    return 1;
}
int Tcg_OpenPack(struct TcgCollection *c, unsigned pack, uint16_t result[10])
{
    struct TcgCollection next;
    uint16_t pools[3][TCG_CARD_COUNT];
    unsigned sizes[3] = {0}, i;
    if (!c->optedIn || pack >= TCG_PACK_TYPES || !c->packs[pack])
        return 0;
    for (i = 1; i <= TCG_CARD_COUNT; i++)
    {
        const struct TcgCard *card = &gTcgCards[i];
        if (card->set == pack && card->kind != TCG_ENERGY && card->rarity < 3)
            pools[card->rarity][sizes[card->rarity]++] = i;
    }
    if (!sizes[0] || !sizes[1] || !sizes[2])
        return 0;
    next = *c;
    for (i = 0; i < 10; i++)
    {
        unsigned rarity = i < 5 ? 0 : i < 8 ? 1 : 2;
        unsigned id =
            i == 9 ? 1 + Tcg_RandomBelow(&next.rng, 6) : pools[rarity][Tcg_RandomBelow(&next.rng, sizes[rarity])];
        if (next.owned[id] >= TCG_COLLECTION_MAX)
            return 0;
        result[i] = id;
        next.owned[id]++;
    }
    next.packs[pack]--;
    *c = next;
    return 1;
}
int Tcg_ExchangeDuplicates(struct TcgCollection *c, unsigned card, unsigned count)
{
    static const unsigned value[] = {1, 3, 10, 20};
    unsigned d, i, keep = 4;
    const struct TcgCard *def = Tcg_Card(card);
    if (!c->optedIn || !def || card <= 6 || !count)
        return 0;
    for (d = 0; d < TCG_SAVED_DECKS; d++)
    {
        unsigned needed = 0;
        for (i = 0; i < TCG_DECK_SIZE; i++)
            needed += c->decks[d].cards[i] == card;
        if (needed > keep)
            keep = needed;
    }
    if (c->owned[card] < keep || count > c->owned[card] - keep || count > (UINT32_MAX - c->dust) / value[def->rarity])
        return 0;
    c->owned[card] -= count;
    c->dust += count * value[def->rarity];
    return 1;
}
int Tcg_CraftCard(struct TcgCollection *c, unsigned card)
{
    static const unsigned cost[] = {5, 15, 50, 100};
    const struct TcgCard *def = Tcg_Card(card);
    if (!c->optedIn || !def || card <= 6 || c->owned[card] >= TCG_COLLECTION_MAX || c->dust < cost[def->rarity])
        return 0;
    c->dust -= cost[def->rarity];
    c->owned[card]++;
    return 1;
}
int Tcg_DeckAdd(struct TcgCollection *c, unsigned deck, unsigned card, char *reason, size_t size)
{
    const struct TcgCard *def = Tcg_Card(card);
    unsigned i, sameName = 0, samePrint = 0, empty = TCG_DECK_SIZE;
    if (!c->optedIn || deck >= TCG_SAVED_DECKS || !def)
        return Invalid(reason, size, "Choose a valid card and deck.");
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        unsigned id = c->decks[deck].cards[i];
        if (!id)
        {
            if (empty == TCG_DECK_SIZE)
                empty = i;
            continue;
        }
        samePrint += id == card;
        sameName += !strcmp(gTcgCards[id].name, def->name);
    }
    if (empty == TCG_DECK_SIZE)
        return Invalid(reason, size, "This deck already has sixty cards.");
    if (card > 6 && sameName >= 4)
        return Invalid(reason, size, "Four cards with this name are already in this deck.");
    if (card > 6 && samePrint >= c->owned[card])
        return Invalid(reason, size, "You do not own another copy of this printing.");
    c->decks[deck].cards[empty] = card;
    if (size)
        reason[0] = 0;
    return 1;
}

int Tcg_DeckRemove(struct TcgCollection *c, unsigned deck, unsigned card)
{
    unsigned i;
    if (!c->optedIn || deck >= TCG_SAVED_DECKS || !Tcg_Card(card))
        return 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
        if (c->decks[deck].cards[i] == card)
        {
            memmove(c->decks[deck].cards + i, c->decks[deck].cards + i + 1,
                    (TCG_DECK_SIZE - i - 1) * sizeof(uint16_t));
            c->decks[deck].cards[TCG_DECK_SIZE - 1] = 0;
            return 1;
        }
    return 0;
}

int Tcg_DeckCopy(struct TcgCollection *c, unsigned from, unsigned to)
{
    if (!c->optedIn || from >= TCG_SAVED_DECKS || to >= TCG_SAVED_DECKS || from == to)
        return 0;
    memcpy(c->decks[to].cards, c->decks[from].cards, sizeof(c->decks[to].cards));
    if (!c->decks[to].name[0])
        snprintf(c->decks[to].name, TCG_NAME_LEN, "Deck %u", to + 1);
    return 1;
}

/* Explicit little-endian encoding, independent of compiler padding and ABI. */
#define PAYLOAD_SIZE                                                                                                   \
    (TCG_CARD_COUNT * 2 + TCG_SAVED_DECKS * (TCG_NAME_LEN + TCG_DECK_SIZE * 2) + TCG_PACK_TYPES * 2 + 24)
static uint32_t Crc(const unsigned char *bytes, size_t size)
{
    uint32_t crc = ~0u;
    unsigned b;
    while (size--)
    {
        crc ^= *bytes++;
        for (b = 0; b < 8; b++)
            crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0);
    }
    return ~crc;
}
static uint32_t Get(const unsigned char **p, unsigned n)
{
    uint32_t v = 0;
    unsigned i;
    for (i = 0; i < n; i++)
        v |= (uint32_t)*(*p)++ << (8 * i);
    return v;
}
#if !MOEMON_DEV_BUILD
static void Put(unsigned char **p, uint32_t v, unsigned n)
{
    while (n--)
    {
        *(*p)++ = (unsigned char)v;
        v >>= 8;
    }
}
#endif
int Tcg_CollectionLoad(struct TcgCollection *c, const char *path)
{
    unsigned char data[PAYLOAD_SIZE + 12], extra;
    const unsigned char *p = data;
    struct TcgCollection next;
    unsigned i, j;
    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;
    if (fread(data, 1, sizeof(data), f) != sizeof(data) || fread(&extra, 1, 1, f))
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    if (memcmp(p, "MTCG", 4))
        return 0;
    p += 4;
    if (Get(&p, 4) != 2 || Get(&p, 4) != Crc(data + 12, PAYLOAD_SIZE))
        return 0;
    Tcg_CollectionInit(&next, 1);
    for (i = 1; i <= TCG_CARD_COUNT; i++)
    {
        next.owned[i] = Get(&p, 2);
        if (next.owned[i] > TCG_COLLECTION_MAX)
            return 0;
    }
    for (i = 0; i < TCG_SAVED_DECKS; i++)
    {
        memcpy(next.decks[i].name, p, TCG_NAME_LEN);
        p += TCG_NAME_LEN;
        next.decks[i].name[TCG_NAME_LEN - 1] = 0;
        for (j = 0; j < TCG_DECK_SIZE; j++)
        {
            next.decks[i].cards[j] = Get(&p, 2);
            if (next.decks[i].cards[j] > TCG_CARD_COUNT)
                return 0;
        }
    }
    for (i = 0; i < TCG_PACK_TYPES; i++)
        next.packs[i] = Get(&p, 2);
    next.wins = Get(&p, 4);
    next.dust = Get(&p, 4);
    next.vouchers = Get(&p, 4);
    next.rng = Get(&p, 4);
    next.optedIn = Get(&p, 1);
    next.activeDeck = Get(&p, 1);
    next.fastAnimations = Get(&p, 1);
    next.quiet = Get(&p, 1);
    next.owner = Get(&p, 4);
    if (next.optedIn > 1 || next.activeDeck >= TCG_SAVED_DECKS || next.fastAnimations > 1 || next.quiet > 1)
        return 0;
    *c = next;
    return 1;
}
int Tcg_CollectionSave(const struct TcgCollection *c, const char *path)
{
#if MOEMON_DEV_BUILD
    (void)c;
    (void)path;
    return 0;
#else
    unsigned char data[PAYLOAD_SIZE + 12], *p = data + 12;
    unsigned i, j;
    FILE *f;
    char tmp[1024];
    if (strlen(path) > sizeof(tmp) - 5)
        return 0;
    for (i = 1; i <= TCG_CARD_COUNT; i++)
        Put(&p, c->owned[i], 2);
    for (i = 0; i < TCG_SAVED_DECKS; i++)
    {
        memcpy(p, c->decks[i].name, TCG_NAME_LEN);
        p += TCG_NAME_LEN;
        for (j = 0; j < TCG_DECK_SIZE; j++)
            Put(&p, c->decks[i].cards[j], 2);
    }
    for (i = 0; i < TCG_PACK_TYPES; i++)
        Put(&p, c->packs[i], 2);
    Put(&p, c->wins, 4);
    Put(&p, c->dust, 4);
    Put(&p, c->vouchers, 4);
    Put(&p, c->rng, 4);
    Put(&p, c->optedIn, 1);
    Put(&p, c->activeDeck, 1);
    Put(&p, c->fastAnimations, 1);
    Put(&p, c->quiet, 1);
    Put(&p, c->owner, 4);
    p = data;
    memcpy(p, "MTCG", 4);
    p += 4;
    Put(&p, 2, 4);
    Put(&p, Crc(data + 12, PAYLOAD_SIZE), 4);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return 0;
    i = fwrite(data, 1, sizeof(data), f) == sizeof(data);
    if (fclose(f))
        i = 0;
    if (!i)
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
