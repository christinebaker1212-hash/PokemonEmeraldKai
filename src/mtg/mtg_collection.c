// Moemon Kai: a save's Magic cards and decks, saved by each card's name hash (a pack rebuilt with more cards moves
// the ids, never the names), the starter, boosters. File "<save>.mtg" (little-endian):
//   "MTGC", u32 version (3; 1 and 2 are read too), u32 owner, u32 rng, u8 optedIn, flags, activeDeck, starter, u16 wins, losses,
//   u32 beaten, u32 owned count, then {u32 hash, u8 count} each, then MTG_DECKS decks {char name[16], u16 count,
//   then count u32 hashes}; from version 2, MTG_DECKS u32s: each deck's ACE CARD by its hash (0: none chosen);
//   from version 3, MTG_DECKS u8s: each deck's hand tray color (TCG_TRAY_*).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtg/collection.h"

#define MTGC_VERSION 3

uint32_t Mtg_Rand(uint32_t *rng)
{
    uint32_t x = *rng ? *rng : 0x6D2B79F5u;   // xorshift32
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *rng = x;
    return x;
}

uint32_t Mtg_NameHash(const char *name)
{
    uint32_t h = 2166136261u;   // FNV-1a
    while (*name)
        h = (h ^ (uint8_t)*name++) * 16777619u;
    return h;
}

typedef struct { uint32_t hash; uint16_t id; } HashEntry;
static HashEntry *sByHash;
static int sByHashCount;

static int CompareHash(const void *a, const void *b)
{
    uint32_t x = ((const HashEntry *)a)->hash, y = ((const HashEntry *)b)->hash;
    return x < y ? -1 : x > y;
}

int Mtg_CardByHash(uint32_t hash)
{
    int lo = 0, hi;
    if (!sByHash || sByHashCount != Mtg_DbCount()) {
        int i;
        free(sByHash);
        sByHashCount = Mtg_DbCount();
        sByHash = malloc(sizeof(HashEntry) * (size_t)(sByHashCount ? sByHashCount : 1));
        for (i = 0; i < sByHashCount; i++) {
            sByHash[i].hash = Mtg_NameHash(Mtg_Card(i)->name);
            sByHash[i].id = (uint16_t)i;
        }
        qsort(sByHash, (size_t)sByHashCount, sizeof(HashEntry), CompareHash);
    }
    hi = sByHashCount - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (sByHash[mid].hash == hash)
            return sByHash[mid].id;
        if (sByHash[mid].hash < hash)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

int Mtg_IsBasicLand(int card)
{
    const MtgCardDef *c = Mtg_Card(card);
    return c && (c->types & MTG_T_LAND) && (c->supertypes & MTG_ST_BASIC);
}

int Mtg_CanUse(int card)
{
    const MtgCardDef *c = Mtg_Card(card);
    return c && c->supported && !c->banned;
}

void Mtg_CollectionInit(MtgCollection *c, uint32_t seed)
{
    int k;
    memset(c, 0, sizeof(*c));
    c->rng = seed ? seed : 1;
    for (k = 0; k < MTG_DECKS; k++)
        c->ace[k] = -1;
}

// ---- decks
int Mtg_DeckCopies(const MtgSavedDeck *d, int card)
{
    int i, n = 0;
    for (i = 0; i < d->count; i++)
        n += d->cards[i] == card;
    return n;
}

void Mtg_DeckAdd(MtgSavedDeck *d, int card)
{
    if (d->count < MTG_DECK_MAX)
        d->cards[d->count++] = (uint16_t)card;
}

void Mtg_DeckRemove(MtgSavedDeck *d, int card)
{
    int i;
    for (i = d->count - 1; i >= 0; i--)
        if (d->cards[i] == card) {
            memmove(&d->cards[i], &d->cards[i + 1], (size_t)(d->count - i - 1) * sizeof(d->cards[0]));
            d->count--;
            return;
        }
}

static int DeckOrder(const void *a, const void *b)
{
    const MtgCardDef *x = Mtg_Card(*(const uint16_t *)a), *y = Mtg_Card(*(const uint16_t *)b);
    int lx = (x->types & MTG_T_LAND) != 0, ly = (y->types & MTG_T_LAND) != 0, cx, cy;
    if (lx != ly)
        return lx - ly;
    cx = Mtg_ManaCostTotal(&x->cost);
    cy = Mtg_ManaCostTotal(&y->cost);
    if (cx != cy)
        return cx - cy;
    return strcmp(x->name, y->name);
}

void Mtg_DeckSort(MtgSavedDeck *d)
{
    qsort(d->cards, d->count, sizeof(d->cards[0]), DeckOrder);
}

uint8_t Mtg_DeckColors(const MtgSavedDeck *d)
{
    int i;
    uint8_t colors = 0;
    for (i = 0; i < d->count; i++)
        colors |= Mtg_Card(d->cards[i])->colors;
    return colors;
}

void Mtg_DeckToDuel(const MtgSavedDeck *d, MtgDeck *out)
{
    out->count = d->count;
    memcpy(out->cards, d->cards, d->count * sizeof(d->cards[0]));
}

const char *Mtg_DeckProblem(const MtgCollection *c, const MtgSavedDeck *d)
{
    int i;
    if (d->count < MTG_DECK_MIN)
        return "A deck needs 40 cards or more.";
    for (i = 0; i < d->count; i++) {
        int card = d->cards[i], copies;
        if (!Mtg_CanUse(card))
            return "A card in it can't be played yet.";
        if (Mtg_IsBasicLand(card))
            continue;
        copies = Mtg_DeckCopies(d, card);
        if (copies > MTG_COPIES)
            return "Four of a card at most (but basic lands).";
        if (c && copies > c->owned[card])
            return "It has cards you don't own.";
    }
    return NULL;
}

// ---- recipes
int Mtg_RecipeDeck(const MtgRecipe *r, MtgSavedDeck *out)
{
    const char *const *line;
    int missing = 0;
    memset(out, 0, sizeof(*out));
    strncpy(out->name, r->name, MTG_DECK_NAME - 1);
    for (line = r->cards; *line; line++) {
        char *end;
        long n = strtol(*line, &end, 10);
        int card;
        while (*end == ' ')
            end++;
        card = Mtg_CardByName(end);
        if (card < 0 || !Mtg_CanUse(card)) {
            missing++;
            continue;
        }
        while (n-- > 0)
            Mtg_DeckAdd(out, card);
    }
    Mtg_DeckSort(out);
    return missing;
}

void Mtg_GiveStarter(MtgCollection *c, unsigned starter)
{
    MtgSavedDeck *d = &c->decks[0];
    int i;
    if (starter >= MTG_STARTERS)
        starter = 0;
    Mtg_RecipeDeck(&gMtgStarters[starter], d);
    for (i = 0; i < d->count; i++)
        if (!Mtg_IsBasicLand(d->cards[i]) && c->owned[d->cards[i]] < MTG_OWNED_MAX)
            c->owned[d->cards[i]]++;
    c->optedIn = 1;
    c->starter = (uint8_t)starter;
    c->activeDeck = 0;
}

// ---- boosters
static int PickOfRarity(MtgCollection *c, int rarity)
{
    int n = Mtg_DbCount(), tries;
    for (tries = 0; tries < 4000; tries++) {
        int id = (int)(Mtg_Rand(&c->rng) % (uint32_t)n);
        const MtgCardDef *d = Mtg_Card(id);
        if (Mtg_CanUse(id) && !Mtg_IsBasicLand(id) && d->rarity == rarity)
            return id;
    }
    return -1;
}

int Mtg_OpenBooster(MtgCollection *c, uint16_t out[MTG_BOOSTER])
{
    static const uint8_t sSlots[MTG_BOOSTER] = {MTG_RARITY_COMMON, MTG_RARITY_COMMON, MTG_RARITY_COMMON,
        MTG_RARITY_COMMON, MTG_RARITY_COMMON, MTG_RARITY_UNCOMMON, MTG_RARITY_UNCOMMON, MTG_RARITY_RARE};
    int i, n = 0;
    for (i = 0; i < MTG_BOOSTER; i++) {
        int rarity = sSlots[i], id;
        if (rarity == MTG_RARITY_RARE && Mtg_Rand(&c->rng) % 4 != 0)
            rarity = MTG_RARITY_UNCOMMON;   // (the last slot: a rare one pack in four)
        id = PickOfRarity(c, rarity);
        if (id < 0)
            continue;
        if (c->owned[id] < MTG_OWNED_MAX)
            c->owned[id]++;
        out[n++] = (uint16_t)id;
    }
    return n;
}

// ---- the file
typedef struct { uint8_t *p; uint32_t n, cap; int ok; } Writer;

static void W8(Writer *w, uint32_t v)
{
    if (w->n < w->cap)
        w->p[w->n] = (uint8_t)v;
    else
        w->ok = 0;
    w->n++;
}
static void W16(Writer *w, uint32_t v) { W8(w, v); W8(w, v >> 8); }
static void W32(Writer *w, uint32_t v) { W16(w, v); W16(w, v >> 16); }

uint32_t Mtg_CollectionWrite(const MtgCollection *c, uint8_t *out, uint32_t cap)
{
    Writer w = {out, 0, cap, 1};
    uint32_t owned = 0;
    int i, k;
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++)
        owned += c->owned[i] != 0;
    W8(&w, 'M'), W8(&w, 'T'), W8(&w, 'G'), W8(&w, 'C');
    W32(&w, MTGC_VERSION);
    W32(&w, c->owner);
    W32(&w, c->rng);
    W8(&w, c->optedIn), W8(&w, c->flags), W8(&w, c->activeDeck), W8(&w, c->starter);
    W16(&w, c->wins), W16(&w, c->losses);
    W32(&w, c->beaten);
    W32(&w, owned);
    for (i = 0; i < Mtg_DbCount() && i < MTG_COLLECTION_CARDS; i++)
        if (c->owned[i]) {
            W32(&w, Mtg_NameHash(Mtg_Card(i)->name));
            W8(&w, c->owned[i]);
        }
    for (k = 0; k < MTG_DECKS; k++) {
        const MtgSavedDeck *d = &c->decks[k];
        for (i = 0; i < MTG_DECK_NAME; i++)
            W8(&w, (uint8_t)d->name[i]);
        W16(&w, d->count);
        for (i = 0; i < d->count; i++)
            W32(&w, Mtg_NameHash(Mtg_Card(d->cards[i])->name));
    }
    for (k = 0; k < MTG_DECKS; k++)
        W32(&w, c->ace[k] >= 0 && c->ace[k] < Mtg_DbCount() ? Mtg_NameHash(Mtg_Card(c->ace[k])->name) : 0);
    for (k = 0; k < MTG_DECKS; k++)
        W8(&w, c->tray[k]);
    return w.ok ? w.n : 0;
}

typedef struct { const uint8_t *p; uint32_t n, size; int ok; } Reader;

static uint32_t R8(Reader *r)
{
    if (r->n >= r->size) {
        r->ok = 0;
        return 0;
    }
    return r->p[r->n++];
}
static uint32_t R16(Reader *r) { uint32_t a = R8(r); return a | R8(r) << 8; }
static uint32_t R32(Reader *r) { uint32_t a = R16(r); return a | R16(r) << 16; }

int Mtg_CollectionRead(MtgCollection *c, const uint8_t *in, uint32_t size)
{
    Reader r = {in, 0, size, 1};
    uint32_t owned, i, version;
    int k;
    if (size < 8 || memcmp(in, "MTGC", 4) != 0)
        return 0;
    r.n = 4;
    version = R32(&r);
    if (version < 1 || version > MTGC_VERSION)
        return 0;
    Mtg_CollectionInit(c, 1);
    c->owner = R32(&r);
    c->rng = R32(&r);
    c->optedIn = (uint8_t)R8(&r), c->flags = (uint8_t)R8(&r), c->activeDeck = (uint8_t)R8(&r);
    c->starter = (uint8_t)R8(&r);
    c->wins = (uint16_t)R16(&r), c->losses = (uint16_t)R16(&r);
    c->beaten = R32(&r);
    owned = R32(&r);
    for (i = 0; i < owned && r.ok; i++) {
        int id = Mtg_CardByHash(R32(&r));
        uint32_t n = R8(&r);
        if (id >= 0 && id < MTG_COLLECTION_CARDS)
            c->owned[id] = (uint8_t)(n > MTG_OWNED_MAX ? MTG_OWNED_MAX : n);   // (a card the pack lost: dropped)
    }
    for (k = 0; k < MTG_DECKS && r.ok; k++) {
        MtgSavedDeck *d = &c->decks[k];
        uint32_t count;
        for (i = 0; i < MTG_DECK_NAME; i++)
            d->name[i] = (char)R8(&r);
        d->name[MTG_DECK_NAME - 1] = 0;
        count = R16(&r);
        for (i = 0; i < count && r.ok; i++) {
            int id = Mtg_CardByHash(R32(&r));
            if (id >= 0)
                Mtg_DeckAdd(d, id);
        }
    }
    if (version >= 2)
        for (k = 0; k < MTG_DECKS && r.ok; k++) {
            uint32_t h = R32(&r);
            c->ace[k] = (int16_t)(h ? Mtg_CardByHash(h) : -1);
        }
    if (version >= 3)
        for (k = 0; k < MTG_DECKS && r.ok; k++)
            c->tray[k] = (uint8_t)R8(&r);
    if (c->activeDeck >= MTG_DECKS)
        c->activeDeck = 0;
    return r.ok;
}

int Mtg_CollectionSave(const MtgCollection *c, const char *path)
{
    uint32_t cap = 64 + MTG_COLLECTION_CARDS * 5 + MTG_DECKS * (MTG_DECK_NAME + 2 + MTG_DECK_MAX * 4 + 5), n;
    uint8_t *buf = malloc(cap);
    char tmp[600];
    FILE *f;
    int ok;
    if (!buf)
        return 0;
    n = Mtg_CollectionWrite(c, buf, cap);
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);   // (written whole, then put in place)
    f = n ? fopen(tmp, "wb") : NULL;
    ok = f && fwrite(buf, 1, n, f) == n;
    if (f)
        ok = (fclose(f) == 0) && ok;
    free(buf);
    if (!ok)
        return 0;
    remove(path);
    return rename(tmp, path) == 0;
}

int Mtg_CollectionLoad(MtgCollection *c, const char *path)
{
    FILE *f = fopen(path, "rb");
    long size;
    uint8_t *buf;
    int ok;
    if (!f)
        return 0;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = size > 0 ? malloc((size_t)size) : NULL;
    ok = buf && fread(buf, 1, (size_t)size, f) == (size_t)size;
    fclose(f);
    ok = ok && Mtg_CollectionRead(c, buf, (uint32_t)size);
    free(buf);
    return ok;
}

int Mtg_AutoAce(const uint16_t *cards, int count)
{
    int best = -1, i, score = -1;
    for (i = 0; i < count; i++) {
        const MtgCardDef *c = Mtg_Card(cards[i]);
        int s;
        if (Mtg_IsBasicLand(cards[i]))
            continue;
        s = c->rarity * 100 + Mtg_ManaCostTotal(&c->cost) * 10 + ((c->types & MTG_T_CREATURE) ? 5 : 0);
        if (s > score)
            score = s, best = cards[i];
    }
    return best >= 0 ? best : count ? cards[0] : -1;
}

int Mtg_DeckAce(const MtgCollection *c, int deck)
{
    const MtgSavedDeck *d = &c->decks[deck];
    if (c->ace[deck] >= 0 && Mtg_DeckCopies(d, c->ace[deck]))
        return c->ace[deck];
    return Mtg_AutoAce(d->cards, d->count);
}
