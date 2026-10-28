// Moemon Kai: the player's Triple Triad cards and HOENN's rules. The rules spread the way Final Fantasy VIII's do:
// each region keeps its own rules, the player carries the rules of the region they last played in, and challenging
// someone in another region can spread a carried rule there or abolish one of its rules the player doesn't carry.
// The CARD QUEEN makes both likelier wherever she is, moves on from time to time, and sometimes brings a rule from
// afar (one of Final Fantasy XIV's). Saved as "<save>.tt": tagged records, so newer fields never break older files.
#include <stdio.h>
#include <string.h>
#include "tt/collection.h"
#include "tt/duel.h"

#define R(x) TT_RULE_BIT(TT_RULE_##x)
static const char *const sRegionNames[TT_REGIONS] = {
    "PETALBURG", "RUSTBORO", "SLATEPORT", "MAUVILLE", "LAVARIDGE", "LILYCOVE", "SOOTOPOLIS", "EVER GRANDE",
};
// Each region's own rules at the start, as Final Fantasy VIII's regions had theirs
static const uint32_t sStartRules[TT_REGIONS] = {
    [TT_REGION_PETALBURG] = R(OPEN),
    [TT_REGION_RUSTBORO] = R(OPEN) | R(SAME),
    [TT_REGION_SLATEPORT] = R(RANDOM) | R(PLUS),
    [TT_REGION_MAUVILLE] = R(SAME) | R(PLUS),
    [TT_REGION_LAVARIDGE] = R(ELEMENTAL) | R(SUDDEN_DEATH),
    [TT_REGION_LILYCOVE] = R(OPEN) | R(SAME) | R(SAME_WALL),
    [TT_REGION_SOOTOPOLIS] = R(ELEMENTAL) | R(PLUS) | R(RANDOM),
    [TT_REGION_EVER_GRANDE] = R(OPEN) | R(SAME) | R(PLUS) | R(SUDDEN_DEATH) | R(SAME_WALL) | R(ELEMENTAL),
};
static const uint8_t sStartTrade[TT_REGIONS] = {
    TT_TRADE_ONE, TT_TRADE_ONE, TT_TRADE_ONE, TT_TRADE_DIFF, TT_TRADE_ONE, TT_TRADE_DIFF, TT_TRADE_DIRECT, TT_TRADE_ONE,
};
const char *Tt_RegionName(unsigned region) { return region < TT_REGIONS ? sRegionNames[region] : "HOENN"; }
uint32_t Tt_RegionStartRules(unsigned region) { return region < TT_REGIONS ? sStartRules[region] : 0; }
unsigned Tt_RegionStartTrade(unsigned region) { return region < TT_REGIONS ? sStartTrade[region] : 0; }

void Tt_CollectionInit(struct TtCollection *c, uint32_t seed)
{
    unsigned i;
    memset(c, 0, sizeof(*c));
    c->rng = seed ? seed : 0x7417;
    for (i = 0; i < TT_REGIONS; i++)
    {
        c->regionRules[i] = sStartRules[i];
        c->regionTrade[i] = sStartTrade[i];
    }
    c->carriedFrom = TT_REGION_NONE;
    c->queenRegion = TT_REGION_MAUVILLE;
    for (i = 0; i < TT_DECKS; i++)
        snprintf(c->decks[i].name, sizeof(c->decks[i].name), "DECK %u", i + 1);
}

int Tt_Give(struct TtCollection *c, unsigned id)
{
    int first;
    if (!Tt_Card(id))
        return 0;
    first = !c->owned[id];
    if (c->owned[id] < TT_MAX_COPIES)
        c->owned[id]++;
    Tt_See(c, id);
    return first;
}
int Tt_Has(const struct TtCollection *c, unsigned id) { return id <= TT_CARD_MAX && c->owned[id]; }
void Tt_See(struct TtCollection *c, unsigned id)
{
    if (id && id <= TT_CARD_MAX)
        c->seen[id / 8] |= 1 << (id % 8);
}
int Tt_Seen(const struct TtCollection *c, unsigned id)
{
    return id && id <= TT_CARD_MAX && (c->seen[id / 8] >> (id % 8) & 1);
}
unsigned Tt_InDecks(const struct TtCollection *c, unsigned id)
{
    unsigned d, i, n = 0;
    for (d = 0; d < TT_DECKS; d++)
        for (i = 0; i < TT_HAND; i++)
            if (c->decks[d].cards[i] == id)
            {
                n = 1;   // (a deck holds one copy; every deck can share it)
                break;
            }
    return n;
}
unsigned Tt_OwnedCount(const struct TtCollection *c, unsigned stars, unsigned *total)
{
    unsigned id, n = 0, all = 0;
    for (id = 1; id <= TT_CARD_MAX; id++)
    {
        const struct TtCard *card = Tt_Card(id);
        if (!card || (stars && card->stars != stars))
            continue;
        all++;
        n += c->owned[id] != 0;
    }
    if (total)
        *total = all;
    return n;
}

void Tt_GiveStarter(struct TtCollection *c)
{
    // The cards XIV hands out with its lesson
    static const uint16_t sStarter[TT_HAND] = {TT_CARD_DODO, TT_CARD_SABOTENDER, TT_CARD_BOMB, TT_CARD_MANDRAGORA,
                                               TT_CARD_COEURL};
    unsigned i;
    for (i = 0; i < TT_HAND; i++)
    {
        Tt_Give(c, sStarter[i]);
        c->decks[0].cards[i] = sStarter[i];
    }
    snprintf(c->decks[0].name, sizeof(c->decks[0].name), "FIRST HAND");
    c->activeDeck = 0;
    c->optedIn = 1;
}

int Tt_DeckReady(const struct TtCollection *c, unsigned deck)
{
    char why[80];
    unsigned i;
    if (deck >= TT_DECKS || !Tt_DeckValid(c->decks[deck].cards, why, sizeof(why)))
        return 0;
    for (i = 0; i < TT_HAND; i++)
        if (!c->owned[c->decks[deck].cards[i]])
            return 0;
    return 1;
}

int Tt_OpenPack(struct TtCollection *c, unsigned pack, uint16_t out[TT_PACK_SIZE])
{
    unsigned id, total = 0, n = 0, k, stars = pack + 1;
    if (pack >= TT_PACK_TYPES || !c->packs[pack])
        return 0;
    for (id = 1; id <= TT_CARD_MAX; id++)
        total += Tt_Card(id) && Tt_Card(id)->stars == stars;
    if (total < TT_PACK_SIZE)
        return 0;
    while (n < TT_PACK_SIZE)
    {
        unsigned pick = Tt_Rand(&c->rng) % total;
        for (id = 1; id <= TT_CARD_MAX; id++)
            if (Tt_Card(id) && Tt_Card(id)->stars == stars && !pick--)
                break;
        for (k = 0; k < n && out[k] != id; k++)
            ;
        if (k == n)
            out[n++] = (uint16_t)id;
    }
    c->packs[pack]--;
    c->packsOpened++;
    for (k = 0; k < TT_PACK_SIZE; k++)
        Tt_Give(c, out[k]);
    return 1;
}

// ---- the rules spreading

static unsigned PickBit(uint32_t mask, uint32_t roll)
{
    unsigned n = (unsigned)__builtin_popcount(mask), i;
    if (!n)
        return 0xFF;
    n = roll % n;
    for (i = 0; i < 32; i++)
        if (mask >> i & 1 && !n--)
            return i;
    return 0xFF;
}

struct TtMix Tt_MixRules(struct TtCollection *c, unsigned region, uint32_t roll)
{
    struct TtMix mix = {TT_MIX_NONE, 0xFF};
    uint32_t local, spreadable, abolishable;
    unsigned spreadChance, abolishChance, r1 = roll % 100, r2 = (roll >> 8) % 100;
    if (region >= TT_REGIONS || c->carriedFrom >= TT_REGIONS || c->carriedFrom == region)
        return mix;
    local = c->regionRules[region];
    spreadable = c->carried & ~local;
    abolishable = local & ~c->carried;
    // The CARD QUEEN's region takes in new rules and drops old ones more readily
    spreadChance = c->queenRegion == region ? 50 : 30;
    abolishChance = c->queenRegion == region ? 40 : 20;
    if (spreadable && __builtin_popcount(local) < TT_REGION_MAX_RULES && r1 < spreadChance)
    {
        mix.kind = TT_MIX_SPREAD;
        mix.rule = (uint8_t)PickBit(spreadable, roll >> 16);
        c->regionRules[region] |= 1u << mix.rule;
        c->spreads++;
    }
    else if (abolishable && r2 < abolishChance)
    {
        mix.kind = TT_MIX_ABOLISH;
        mix.rule = (uint8_t)PickBit(abolishable, roll >> 16);
        c->regionRules[region] &= ~(1u << mix.rule);
        c->abolishes++;
    }
    return mix;
}

void Tt_PlayedIn(struct TtCollection *c, unsigned region)
{
    if (region >= TT_REGIONS)
        return;
    c->carried = c->regionRules[region];
    c->carriedFrom = (uint8_t)region;
    if (c->regionGames[region] < 0xFFFF)
        c->regionGames[region]++;
}

unsigned Tt_QueenMoves(struct TtCollection *c, unsigned region, unsigned day, uint32_t roll)
{
    uint32_t missing;
    unsigned rule;
    if (region >= TT_REGIONS)
        return 0xFF;
    c->queenRegion = (uint8_t)region;
    c->queenDay = (uint16_t)day;
    // Three times in ten she brings a rule the region hasn't got: one of Final Fantasy VIII's, or a far-off land's
    missing = ((1u << TT_RULE_COUNT) - 1) & ~c->regionRules[region];
    // (never both of a pair that undo each other)
    if (c->regionRules[region] & (R(ORDER) | R(CHAOS)))
        missing &= ~(R(ORDER) | R(CHAOS));
    if (c->regionRules[region] & (R(ASCENSION) | R(DESCENSION)))
        missing &= ~(R(ASCENSION) | R(DESCENSION));
    if (c->regionRules[region] & (R(OPEN) | R(THREE_OPEN)))
        missing &= ~(R(OPEN) | R(THREE_OPEN));
    if (roll % 10 >= 3 || !missing || __builtin_popcount(c->regionRules[region]) >= TT_REGION_MAX_RULES)
        return 0xFF;
    rule = PickBit(missing, roll >> 8);
    c->regionRules[region] |= 1u << rule;
    return rule;
}

// ---- the file: "MTTR", u32 format, u32 crc of the rest, then records: u16 tag, u16 length, the bytes

#define TT_FILE_FORMAT 1
enum
{
    TAG_HEADER = 1,   /* owner, flags, rng, optedIn, activeDeck */
    TAG_OWNED,
    TAG_SEEN,
    TAG_BOUGHT,
    TAG_DECKS,
    TAG_PACKS,
    TAG_REGIONS,      /* rules, trade, games per region; carried, carriedFrom, queen */
    TAG_RECORDS,
};
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
#define FILE_MAX 8192
static unsigned char *Begin(unsigned char **p, unsigned tag)
{
    Put(p, tag, 2);
    *p += 2;
    return *p;
}
static void End(unsigned char **p, unsigned char *start)
{
    unsigned len = (unsigned)(*p - start);
    start[-2] = (unsigned char)len;
    start[-1] = (unsigned char)(len >> 8);
}

static size_t Write(const struct TtCollection *c, unsigned char *data)
{
    unsigned char *p = data + 12, *s;
    unsigned i, j;
    s = Begin(&p, TAG_HEADER);
    Put(&p, c->owner, 4);
    Put(&p, c->flags, 4);
    Put(&p, c->rng, 4);
    Put(&p, c->optedIn, 1);
    Put(&p, c->activeDeck, 1);
    End(&p, s);
    s = Begin(&p, TAG_OWNED);
    for (i = 0; i <= TT_CARD_MAX; i++)
        Put(&p, c->owned[i], 1);
    End(&p, s);
    s = Begin(&p, TAG_SEEN);
    memcpy(p, c->seen, TT_CARD_BYTES);
    p += TT_CARD_BYTES;
    End(&p, s);
    s = Begin(&p, TAG_BOUGHT);
    memcpy(p, c->bought, TT_CARD_BYTES);
    p += TT_CARD_BYTES;
    End(&p, s);
    s = Begin(&p, TAG_DECKS);
    for (i = 0; i < TT_DECKS; i++)
    {
        memcpy(p, c->decks[i].name, TT_DECK_NAME);
        p += TT_DECK_NAME;
        for (j = 0; j < TT_HAND; j++)
            Put(&p, c->decks[i].cards[j], 2);
    }
    End(&p, s);
    s = Begin(&p, TAG_PACKS);
    for (i = 0; i < TT_PACK_TYPES; i++)
        Put(&p, c->packs[i], 2);
    End(&p, s);
    s = Begin(&p, TAG_REGIONS);
    Put(&p, TT_REGIONS, 1);
    for (i = 0; i < TT_REGIONS; i++)
    {
        Put(&p, c->regionRules[i], 4);
        Put(&p, c->regionTrade[i], 1);
        Put(&p, c->regionGames[i], 2);
    }
    Put(&p, c->carried, 4);
    Put(&p, c->carriedFrom, 1);
    Put(&p, c->queenRegion, 1);
    Put(&p, c->queenDay, 2);
    End(&p, s);
    s = Begin(&p, TAG_RECORDS);
    Put(&p, c->wins, 4);
    Put(&p, c->losses, 4);
    Put(&p, c->draws, 4);
    Put(&p, c->packsOpened, 4);
    Put(&p, c->onlineWins, 4);
    Put(&p, c->trades, 4);
    Put(&p, c->combos, 4);
    Put(&p, c->sames, 4);
    Put(&p, c->pluses, 4);
    Put(&p, c->spreads, 4);
    Put(&p, c->abolishes, 4);
    Put(&p, c->specialBeaten, 4);
    Put(&p, c->perfects, 4);
    Put(&p, c->lastDaily, 2);
    End(&p, s);
    {
        size_t n = (size_t)(p - data);
        unsigned char *h = data;
        memcpy(h, "MTTR", 4);
        h += 4;
        Put(&h, TT_FILE_FORMAT, 4);
        Put(&h, Crc(data + 12, n - 12), 4);
        return n;
    }
}

int Tt_CollectionSave(const struct TtCollection *c, const char *path)
{
#if MOEMON_DEV_BUILD
    (void)c;
    (void)path;
    return 0;
#else
    static unsigned char data[FILE_MAX];
    size_t n = Write(c, data);
    char tmp[1024];
    FILE *f;
    int ok;
    if (strlen(path) > sizeof(tmp) - 5)
        return 0;
    snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f)
        return 0;
    ok = fwrite(data, 1, n, f) == n;
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

int Tt_CollectionLoad(struct TtCollection *c, const char *path)
{
    static unsigned char data[FILE_MAX];
    const unsigned char *p = data + 4, *end;
    FILE *f = fopen(path, "rb");
    size_t n;
    unsigned i, j;
    if (!f)
        return 0;
    n = fread(data, 1, sizeof(data), f);
    fclose(f);
    if (n < 12 || memcmp(data, "MTTR", 4))
        return 0;
    if (Get(&p, 4) < 1 || Get(&p, 4) != Crc(data + 12, n - 12))
        return 0;
    Tt_CollectionInit(c, 1);
    end = data + n;
    p = data + 12;
    while (p + 4 <= end)
    {
        unsigned tag = Get(&p, 2), len = Get(&p, 2);
        const unsigned char *q = p, *stop = p + len;
        if (stop > end)
            return 0;
        switch (tag)
        {
        case TAG_HEADER:
            c->owner = Get(&q, 4);
            c->flags = Get(&q, 4);
            c->rng = Get(&q, 4);
            c->optedIn = (uint8_t)Get(&q, 1);
            c->activeDeck = (uint8_t)Get(&q, 1);
            if (c->activeDeck >= TT_DECKS)
                c->activeDeck = 0;
            break;
        case TAG_OWNED:
            for (i = 0; i < len && i <= TT_CARD_MAX; i++)
                c->owned[i] = Tt_Card(i) ? q[i] : 0;
            break;
        case TAG_SEEN:
            memcpy(c->seen, q, len < TT_CARD_BYTES ? len : TT_CARD_BYTES);
            break;
        case TAG_BOUGHT:
            memcpy(c->bought, q, len < TT_CARD_BYTES ? len : TT_CARD_BYTES);
            break;
        case TAG_DECKS:
            for (i = 0; i < TT_DECKS && q + TT_DECK_NAME + 2 * TT_HAND <= stop; i++)
            {
                memcpy(c->decks[i].name, q, TT_DECK_NAME);
                c->decks[i].name[TT_DECK_NAME] = 0;
                q += TT_DECK_NAME;
                for (j = 0; j < TT_HAND; j++)
                {
                    unsigned id = Get(&q, 2);
                    c->decks[i].cards[j] = (uint16_t)(Tt_Card(id) ? id : 0);
                }
            }
            break;
        case TAG_PACKS:
            for (i = 0; i < TT_PACK_TYPES && q + 2 <= stop; i++)
                c->packs[i] = (uint16_t)Get(&q, 2);
            break;
        case TAG_REGIONS:
        {
            unsigned regions = Get(&q, 1);
            for (i = 0; i < regions && q + 7 <= stop; i++)
            {
                uint32_t rules = Get(&q, 4) & ((1u << TT_RULE_COUNT) - 1);
                unsigned trade = Get(&q, 1), games = Get(&q, 2);
                if (i >= TT_REGIONS)
                    continue;
                c->regionRules[i] = rules;
                c->regionTrade[i] = (uint8_t)(trade < TT_TRADE_COUNT ? trade : TT_TRADE_ONE);
                c->regionGames[i] = (uint16_t)games;
            }
            if (q + 8 <= stop)
            {
                c->carried = Get(&q, 4) & ((1u << TT_RULE_COUNT) - 1);
                c->carriedFrom = (uint8_t)Get(&q, 1);
                c->queenRegion = (uint8_t)Get(&q, 1);
                c->queenDay = (uint16_t)Get(&q, 2);
                if (c->carriedFrom >= TT_REGIONS)
                    c->carriedFrom = TT_REGION_NONE;
                if (c->queenRegion >= TT_REGIONS)
                    c->queenRegion = TT_REGION_MAUVILLE;
            }
            break;
        }
        case TAG_RECORDS:
        {
            uint32_t *fields[] = {&c->wins, &c->losses, &c->draws, &c->packsOpened, &c->onlineWins, &c->trades,
                                  &c->combos, &c->sames, &c->pluses, &c->spreads, &c->abolishes, &c->specialBeaten,
                                  &c->perfects};
            for (i = 0; i < sizeof(fields) / sizeof(fields[0]) && q + 4 <= stop; i++)
                *fields[i] = Get(&q, 4);
            if (q + 2 <= stop)
                c->lastDaily = (uint16_t)Get(&q, 2);
            break;
        }
        default:
            break;   // (a newer version's record)
        }
        p = stop;
    }
    return 1;
}
