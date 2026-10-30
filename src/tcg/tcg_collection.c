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
    return 1;
}
/* The rares WotC printed as holofoils: numbers 1-16 of Base Set and Jungle, 1-15 of Fossil
 * (tools/tcg/printed_art.json "printing"). */
static const uint16_t sHolo[] = {
    TCG_CARD_VENUSAUR_LV67, TCG_CARD_NIDOQUEEN, TCG_CARD_NIDOKING, TCG_CARD_VILEPLUME, TCG_CARD_VENOMOTH,
    TCG_CARD_VICTREEBEL, TCG_CARD_MUK, TCG_CARD_SCYTHER, TCG_CARD_PINSIR, TCG_CARD_CHARIZARD,
    TCG_CARD_NINETALES_LV32, TCG_CARD_FLAREON_LV28, TCG_CARD_MOLTRES_LV35, TCG_CARD_BLASTOISE, TCG_CARD_POLIWRATH,
    TCG_CARD_GYARADOS, TCG_CARD_VAPOREON_LV42, TCG_CARD_ARTICUNO_LV35, TCG_CARD_RAICHU_LV40, TCG_CARD_RAICHU_LV45,
    TCG_CARD_MAGNETON_LV28, TCG_CARD_MAGNETON_LV35, TCG_CARD_ELECTRODE_LV42, TCG_CARD_JOLTEON_LV29,
    TCG_CARD_ZAPDOS_LV40, TCG_CARD_ZAPDOS_LV64, TCG_CARD_MACHAMP, TCG_CARD_HITMONLEE, TCG_CARD_HITMONCHAN,
    TCG_CARD_KABUTOPS, TCG_CARD_AERODACTYL, TCG_CARD_ALAKAZAM, TCG_CARD_HAUNTER_LV17, TCG_CARD_GENGAR,
    TCG_CARD_HYPNO, TCG_CARD_MR_MIME, TCG_CARD_MEWTWO_LV53, TCG_CARD_PIDGEOT_LV40, TCG_CARD_CLEFAIRY,
    TCG_CARD_CLEFABLE, TCG_CARD_WIGGLYTUFF, TCG_CARD_CHANSEY, TCG_CARD_KANGASKHAN, TCG_CARD_SNORLAX,
    TCG_CARD_DRAGONITE_LV45,
};
int Tcg_IsHolo(unsigned card)
{
    unsigned i;
    if (card && card <= TCG_CARD_COUNT && gTcgCards[card].holo)
        return 1;   /* (GB2's: generate_catalog.py) */
    for (i = 0; i < sizeof(sHolo) / sizeof(sHolo[0]); i++)
        if (sHolo[i] == card)
            return 1;
    return 0;
}
int Tcg_InPack(unsigned card, unsigned pack)
{
    const struct TcgCard *c = Tcg_Card(card);
    unsigned set = pack < TCG_PACK_GB2 ? pack : pack - TCG_PACK_GB2 + TCG_SET_GB2;
    return c && pack < TCG_PACK_TYPES && (c->set == set || (pack >= TCG_PACK_GB2 && c->set2 == set));
}
/* A card sold in boosters (not a promo or a Basic Energy) */
static int Boosted(const struct TcgCard *c) { return c && c->set != TCG_SET_PROMO && c->set != TCG_SET_ENERGY; }
int Tcg_OpenPack(struct TcgCollection *c, unsigned pack, uint16_t result[10])
{
    struct TcgCollection next;
    /* commons, uncommons, plain rares, holos, holos the player doesn't have */
    uint16_t pools[5][TCG_CARD_COUNT];
    unsigned sizes[5] = {0}, i;
    if (!c->optedIn || pack >= TCG_PACK_TYPES || !c->packs[pack])
        return 0;
    for (i = 1; i <= TCG_CARD_COUNT; i++)
    {
        const struct TcgCard *card = &gTcgCards[i];
        unsigned pool = card->rarity;
        if (!Tcg_InPack(i, pack) || card->kind == TCG_ENERGY || card->rarity >= 3)
            continue;
        if (pool == 2 && Tcg_IsHolo(i))
        {
            pool = 3;
            if (!c->owned[i])
                pools[4][sizes[4]++] = i;
        }
        pools[pool][sizes[pool]++] = i;
    }
    if (!sizes[0] || !sizes[1] || (!sizes[2] && !sizes[3]))
        return 0;
    next = *c;
    if (next.drought[pack] < 255)
        next.drought[pack]++;
    /* six commons, three uncommons, the rare last (no Basic Energy: it's free and endless at the deck builder) */
    for (i = 0; i < 10; i++)
    {
        unsigned pool = i < 6 ? 0 : i < 9 ? 1 : 2, id;
        if (i == 9)
        {
            /* The rare: a holo one pack in TCG_HOLO_ODDS (a new one after a long wait) */
            if (sizes[4] && next.drought[pack] >= TCG_HOLO_PITY)
                pool = 4;
            else if (!sizes[2] || (sizes[3] && Tcg_RandomBelow(&next.rng, TCG_HOLO_ODDS) == 0))
                pool = 3;
        }
        id = pools[pool][Tcg_RandomBelow(&next.rng, sizes[pool])];
        if (next.owned[id] >= TCG_COLLECTION_MAX)
            return 0;
        if (pool >= 3 && !next.owned[id])
            next.drought[pack] = 0;
        result[i] = id;
        next.owned[id]++;
    }
    next.packs[pack]--;
    next.packsOpened++;
    *c = next;
    return 1;
}
/* CARD POINTS for a spare and the price of a card you pick, by rarity (a holo's: TCG_HOLO_POINTS;
 * holos can't be picked) */
static const unsigned sPointValue[] = {1, 2, 6, 20};
static const unsigned sPointCost[] = {5, 15, 80, 0};
#define TCG_HOLO_POINTS 20
unsigned Tcg_PointValue(unsigned card)
{
    const struct TcgCard *def = Tcg_Card(card);
    if (def && card > 6 && Tcg_IsHolo(card))
        return TCG_HOLO_POINTS;
    return def && card > 6 ? sPointValue[def->rarity & 3] : 0;
}
unsigned Tcg_PointCost(unsigned card)
{
    const struct TcgCard *def = Tcg_Card(card);
    /* The CARD CLUB counters: commons and uncommons (rares at the GAME CENTER, Tcg_ShopPrice). Promo cards
     * come only from the trainers who hold them, holos only from packs and trades; Basic Energy is free. */
    return def && card > 6 && Boosted(def) && def->kind != TCG_ENERGY && def->rarity < 2 ? sPointCost[def->rarity & 3] : 0;
}
/* The GAME CENTER CARD SHOP's price in money: every card of the four sets but the holos */
static const unsigned sShopPrice[] = {300, 800, 4000, 0};
unsigned Tcg_ShopPrice(unsigned card)
{
    const struct TcgCard *def = Tcg_Card(card);
    return def && card > 6 && Boosted(def) && def->kind != TCG_ENERGY && !Tcg_IsHolo(card) ? sShopPrice[def->rarity & 3] : 0;
}
unsigned Tcg_Spare(const struct TcgCollection *c, unsigned card)
{
    unsigned d, i, keep = 4;
    if (!Tcg_Card(card) || card <= 6)
        return 0;
    for (d = 0; d < TCG_SAVED_DECKS; d++)
    {
        unsigned needed = 0;
        for (i = 0; i < TCG_DECK_SIZE; i++)
            needed += c->decks[d].cards[i] == card;
        if (needed > keep)
            keep = needed;
    }
    return c->owned[card] > keep ? c->owned[card] - keep : 0;
}
int Tcg_ExchangeDuplicates(struct TcgCollection *c, unsigned card, unsigned count)
{
    unsigned value = Tcg_PointValue(card);
    if (!c->optedIn || !value || !count || count > Tcg_Spare(c, card) || count > (UINT32_MAX - c->dust) / value)
        return 0;
    c->owned[card] -= count;
    c->dust += count * value;
    return 1;
}
int Tcg_CraftCard(struct TcgCollection *c, unsigned card)
{
    unsigned cost = Tcg_PointCost(card);
    if (!c->optedIn || !cost || c->owned[card] >= TCG_COLLECTION_MAX || c->dust < cost)
        return 0;
    c->dust -= cost;
    c->owned[card]++;
    return 1;
}
int Tcg_DailyAvailable(const struct TcgCollection *c, uint32_t key, unsigned day)
{
    unsigned i;
    for (i = 0; i < TCG_DAILY_SLOTS; i++)
        if (c->daily[i].key == key && key)
            return c->daily[i].day != (uint16_t)day;
    return 1;
}
void Tcg_DailyTake(struct TcgCollection *c, uint32_t key, unsigned day)
{
    unsigned i, slot = 0;
    for (i = 0; i < TCG_DAILY_SLOTS; i++)
    {
        if (c->daily[i].key == key)
        {
            slot = i;
            break;
        }
        /* Otherwise an empty slot, or the one taken longest ago (a clock set back
         * can make a day look like the future: that's the newest, not the oldest). */
        if (!c->daily[i].key || (int16_t)(day - c->daily[i].day) > (int16_t)(day - c->daily[slot].day))
            slot = i;
        if (!c->daily[i].key)
            break;
    }
    c->daily[slot].key = key;
    c->daily[slot].day = (uint16_t)day;
}
unsigned Tcg_SetOwned(const struct TcgCollection *c, unsigned set, unsigned *total)
{
    unsigned id, have = 0, all = 0;
    for (id = 1; id <= TCG_CARD_COUNT; id++)
    {
        if (set < TCG_SET_COUNT && !Tcg_InPack(id, set))
            continue;
        all++;
        have += c->owned[id] != 0;
    }
    if (total)
        *total = all;
    return have;
}
/* Milestone bits: the first game's sets in milestones (set * 3 + n), GB2's in milestonesHi ((set - 4) * 3 + n) */
int Tcg_Milestone(const struct TcgCollection *c, unsigned set, unsigned n)
{
    return set < TCG_PACK_GB2 ? (c->milestones >> (set * 3 + n)) & 1 : (c->milestonesHi >> ((set - TCG_PACK_GB2) * 3 + n)) & 1;
}
void Tcg_SetMilestone(struct TcgCollection *c, unsigned set, unsigned n)
{
    if (set < TCG_PACK_GB2)
        c->milestones |= 1u << (set * 3 + n);
    else if (set < TCG_SET_COUNT)
        c->milestonesHi |= 1u << ((set - TCG_PACK_GB2) * 3 + n);
}
unsigned Tcg_TakeMilestones(struct TcgCollection *c)
{
    unsigned set, n, got = 0;
    for (set = 0; set < TCG_SET_COUNT; set++)
    {
        unsigned total, have = Tcg_SetOwned(c, set, &total);
        for (n = 0; n < 3; n++)
            if (have * 4 >= total * (n + 1) && !Tcg_Milestone(c, set, n))
            {
                Tcg_SetMilestone(c, set, n);
                got++;
            }
    }
    return got;
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
    c->ace[to] = c->ace[from];
    return 1;
}

unsigned Tcg_DeckMascot(const uint16_t cards[TCG_DECK_SIZE])
{
    unsigned i, best = 0, score = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        const struct TcgCard *card = Tcg_Card(cards[i]);
        unsigned s;
        if (!card || card->kind != TCG_POKEMON)
            continue;
        /* The most evolved, then the rarest, then the strongest. */
        s = card->stage * 1000 + card->rarity * 300 + card->hp;
        if (s > score)
            best = card->id, score = s;
    }
    return best;
}

unsigned Tcg_DeckAce(const struct TcgCollection *c, unsigned deck)
{
    unsigned i;
    if (deck >= TCG_SAVED_DECKS)
        return 0;
    if (c->ace[deck])
        for (i = 0; i < TCG_DECK_SIZE; i++)
            if (c->decks[deck].cards[i] == c->ace[deck])
                return c->ace[deck];
    return Tcg_DeckMascot(c->decks[deck].cards);
}

unsigned Tcg_RecipeOwned(const struct TcgCollection *c, const uint16_t recipe[TCG_DECK_SIZE])
{
    unsigned used[TCG_CARD_COUNT + 1] = {0}, i, n = 0;
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        unsigned id = recipe[i];
        if (!Tcg_Card(id))
            continue;
        if (id <= 6 || used[id] < c->owned[id])
            n++;
        used[id]++;
    }
    return n;
}

unsigned Tcg_DeckFromRecipe(struct TcgCollection *c, unsigned deck, const uint16_t recipe[TCG_DECK_SIZE],
                            const char *name, unsigned *missing)
{
    unsigned used[TCG_CARD_COUNT + 1] = {0}, i, n = 0, lost = 0;
    if (!c->optedIn || deck >= TCG_SAVED_DECKS)
        return 0;
    memset(c->decks[deck].cards, 0, sizeof(c->decks[deck].cards));
    for (i = 0; i < TCG_DECK_SIZE; i++)
    {
        unsigned id = recipe[i];
        if (!Tcg_Card(id))
            continue;
        if (id > 6 && used[id] >= c->owned[id])
        {
            lost++;
            continue;
        }
        used[id]++;
        c->decks[deck].cards[n++] = (uint16_t)id;
    }
    snprintf(c->decks[deck].name, TCG_NAME_LEN, "%s", name);
    c->ace[deck] = 0;
    if (missing)
        *missing = lost;
    return n;
}

const struct TcgRecord *Tcg_Record(const struct TcgCollection *c, uint32_t key)
{
    unsigned i;
    for (i = 0; i < TCG_RECORD_SLOTS; i++)
        if (c->records[i].key == key && (c->records[i].wins || c->records[i].losses))
            return &c->records[i];
    return NULL;
}
void Tcg_RecordResult(struct TcgCollection *c, uint32_t key, int won, unsigned day)
{
    struct TcgRecord *r = (struct TcgRecord *)Tcg_Record(c, key);
    unsigned i;
    if (!r)
    {
        /* a free slot, or the one played longest ago */
        r = &c->records[0];
        for (i = 0; i < TCG_RECORD_SLOTS; i++)
        {
            if (!c->records[i].wins && !c->records[i].losses)
            {
                r = &c->records[i];
                break;
            }
            if (c->records[i].lastSeen < r->lastSeen)
                r = &c->records[i];
        }
        memset(r, 0, sizeof(*r));
        r->key = key;
    }
    if (won)
    {
        if (r->wins < 0xFFFF)
            r->wins++;
        r->lastWin = (uint16_t)day;
    }
    else if (r->losses < 0xFFFF)
        r->losses++;
    r->lastSeen = (uint16_t)day;
}

/* Explicit little-endian encoding, independent of compiler padding and ABI. Formats up to 7 hold the first Game Boy
 * game's 228 cards and its 4 boosters; 8 adds Pokemon Card GB2's (TCG_CARD_COUNT, TCG_PACK_TYPES) and the
 * milestones' upper bits. A save of an older format loads into the new collection (the added cards at 0). */
#define FORMAT 9
#define OLD_CARDS 228
#define OLD_PACKS 4
#define CARDS_IN(format) ((format) >= 8 ? TCG_CARD_COUNT : OLD_CARDS)
#define PACKS_IN(format) ((format) >= 8 ? TCG_PACK_TYPES : OLD_PACKS)
#define GC_BYTES(cards) (((cards) + 8) / 8)
#define PAYLOAD_SIZE(format)                                                                                           \
    (CARDS_IN(format) * 2 + TCG_SAVED_DECKS * (TCG_NAME_LEN + TCG_DECK_SIZE * 2) + PACKS_IN(format) * 2 + 24)
/* Format 3 adds daily rewards, special trainers, milestones and counters; 4 CARD SCALING; 5 the
 * packs since the last new holo. */
#define EXTRA_SIZE (TCG_DAILY_SLOTS * 6 + 7 * 4)
#define EXTRA4_SIZE 4
#define EXTRA5_SIZE(format) PACKS_IN(format)
/* 6: the GAME CENTER CARD SHOP's sales and the records against card players */
#define EXTRA6_SIZE(format) (1 + GC_BYTES(CARDS_IN(format)) + TCG_RECORD_SLOTS * 12)
/* 7: each deck's ACE CARD */
#define EXTRA7_SIZE (TCG_SAVED_DECKS * 2)
/* 8: the milestones' upper 32 bits (11 boosters x 3), the GAME CENTER's later structure decks (gcDecks bits 8-31) */
#define EXTRA8_SIZE 7
/* 9: the milestone promo cards claimed */
#define EXTRA9_SIZE 4
#define FILE_SIZE(format)                                                                                              \
    (12u + PAYLOAD_SIZE(format) + ((format) >= 3 ? EXTRA_SIZE : 0) + ((format) >= 4 ? EXTRA4_SIZE : 0) +               \
     ((format) >= 5 ? EXTRA5_SIZE(format) : 0) + ((format) >= 6 ? EXTRA6_SIZE(format) : 0) +                           \
     ((format) >= 7 ? EXTRA7_SIZE : 0) + ((format) >= 8 ? EXTRA8_SIZE : 0) + ((format) >= 9 ? EXTRA9_SIZE : 0))
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
    unsigned char data[FILE_SIZE(FORMAT) + 1];
    const unsigned char *p = data;
    struct TcgCollection next;
    unsigned i, j, format, size, cards, packs;
    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;
    size = (unsigned)fread(data, 1, sizeof(data), f);
    fclose(f);
    if (size < 12 || memcmp(p, "MTCG", 4))
        return 0;
    p += 4;
    format = Get(&p, 4);
    if (format < 2 || format > FORMAT || size != FILE_SIZE(format) ||
        Get(&p, 4) != Crc(data + 12, size - 12))
        return 0;
    cards = CARDS_IN(format);
    packs = PACKS_IN(format);
    Tcg_CollectionInit(&next, 1);
    for (i = 1; i <= cards; i++)
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
            if (next.decks[i].cards[j] > cards)
                return 0;
        }
    }
    for (i = 0; i < packs; i++)
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
    if (format >= 3)
    {
        for (i = 0; i < TCG_DAILY_SLOTS; i++)
        {
            next.daily[i].key = Get(&p, 4);
            next.daily[i].day = Get(&p, 2);
        }
        next.specialBeaten = Get(&p, 4);
        next.promoGiven = Get(&p, 4);
        next.milestones = Get(&p, 4);
        next.packsOpened = Get(&p, 4);
        next.onlineWins = Get(&p, 4);
        next.trades = Get(&p, 4);
        next.flags = Get(&p, 4);
        if (format >= 4)
        {
            next.skill = (int16_t)(uint16_t)Get(&p, 2);
            next.rated = Get(&p, 2);
        }
        if (format >= 5)
            for (i = 0; i < packs; i++)
                next.drought[i] = Get(&p, 1);
        if (format >= 6)
        {
            next.gcDecks = Get(&p, 1);
            for (i = 0; i < GC_BYTES(cards); i++)
                next.gcSingles[i] = Get(&p, 1);
            for (i = 0; i < TCG_RECORD_SLOTS; i++)
            {
                next.records[i].key = Get(&p, 4);
                next.records[i].wins = Get(&p, 2);
                next.records[i].losses = Get(&p, 2);
                next.records[i].lastWin = Get(&p, 2);
                next.records[i].lastSeen = Get(&p, 2);
            }
        }
        if (format >= 7)
            for (i = 0; i < TCG_SAVED_DECKS; i++)
            {
                next.ace[i] = Get(&p, 2);
                if (next.ace[i] > cards)
                    next.ace[i] = 0;
            }
        if (format >= 8)
        {
            next.milestonesHi = Get(&p, 4);
            next.gcDecks |= Get(&p, 3) << 8;
        }
        if (format >= 9)
            next.rewards = Get(&p, 4);
    }
    else if (next.optedIn)
        next.flags = TCG_FLAG_TUTORIAL | TCG_FLAG_CLUB_MET; /* they started before the CARD CLUB */
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
    unsigned char data[FILE_SIZE(FORMAT)], *p = data + 12;
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
    for (i = 0; i < TCG_DAILY_SLOTS; i++)
    {
        Put(&p, c->daily[i].key, 4);
        Put(&p, c->daily[i].day, 2);
    }
    Put(&p, c->specialBeaten, 4);
    Put(&p, c->promoGiven, 4);
    Put(&p, c->milestones, 4);
    Put(&p, c->packsOpened, 4);
    Put(&p, c->onlineWins, 4);
    Put(&p, c->trades, 4);
    Put(&p, c->flags, 4);
    Put(&p, (uint16_t)c->skill, 2);
    Put(&p, c->rated, 2);
    for (i = 0; i < TCG_PACK_TYPES; i++)
        Put(&p, c->drought[i], 1);
    Put(&p, c->gcDecks & 0xff, 1);
    for (i = 0; i < TCG_GC_SINGLE_BYTES; i++)
        Put(&p, c->gcSingles[i], 1);
    for (i = 0; i < TCG_RECORD_SLOTS; i++)
    {
        Put(&p, c->records[i].key, 4);
        Put(&p, c->records[i].wins, 2);
        Put(&p, c->records[i].losses, 2);
        Put(&p, c->records[i].lastWin, 2);
        Put(&p, c->records[i].lastSeen, 2);
    }
    for (i = 0; i < TCG_SAVED_DECKS; i++)
        Put(&p, c->ace[i], 2);
    Put(&p, c->milestonesHi, 4);
    Put(&p, c->gcDecks >> 8, 3);
    Put(&p, c->rewards, 4);
    p = data;
    memcpy(p, "MTCG", 4);
    p += 4;
    Put(&p, FORMAT, 4);
    Put(&p, Crc(data + 12, sizeof(data) - 12), 4);
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
