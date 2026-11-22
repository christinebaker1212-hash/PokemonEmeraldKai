// Moemon Kai: the generic duelists' Yu-Gi-Oh! decks (the user, 2026-11-21: docs/YGO_DECISIONS.md "NPC decks from
// YGOPRODeck"): every playable deck uploaded to YGOPRODeck, cut to its era's Forbidden & Limited list, rated by
// AI-against-AI duels on the rules (tools/ygo/rate_pool.py). graphics/ygo/npc_pool.bin (tools/ygo/write_npc_pool.py,
// built in by ygo_npc_pool_pak.c) is inflated the first time a duelist needs a deck; each deck's GEN (its newest
// card's era) worked out then. A duelist picks one near their own rating, inside what their standing in the world
// allows (how strong, how competitive: ygo_field.c), among those the YU-GI-OH! GEN allows (MP_YgoGen), not one met
// lately.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo_internal.h"
#include "ygo/collection.h"

char *stbi_zlib_decode_noheader_malloc(const char *buffer, int len, int *outlen);
extern const uint8_t gYgoNpcPoolPak[], gYgoNpcPoolPakEnd[];

struct PoolDeck
{
    uint32_t off;        // its cards in the payload
    uint32_t nameAt;     // its name in the payload (nameLen bytes)
    uint16_t rating, year;
    uint8_t format, comp, placing, era, nameLen;   // comp: 0 casual .. 3 a tournament deck (write_npc_pool.py)
};

static struct
{
    uint8_t tried, ok;
    uint8_t *data;
    uint32_t size, count;
    struct PoolDeck *decks;
} P;

#define RECENT 24
static int32_t sRecent[RECENT] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static uint8_t sRecentAt;

static uint32_t U32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
static uint16_t U16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

static int Load(void)
{
    size_t size = (size_t)(gYgoNpcPoolPakEnd - gYgoNpcPoolPak);
    int out = 0;
    uint32_t i, at = 0;
    if (P.tried)
        return P.ok;
    P.tried = 1;
    if (size < 16 || memcmp(gYgoNpcPoolPak, "NPCP", 4) || U32(gYgoNpcPoolPak + 4) != 2)
        return 0;
    P.count = U32(gYgoNpcPoolPak + 8);
    P.data = (uint8_t *)stbi_zlib_decode_noheader_malloc((const char *)gYgoNpcPoolPak + 16, (int)(size - 16), &out);
    if (!P.data || (uint32_t)out != U32(gYgoNpcPoolPak + 12) || !P.count)
    {
        printf("[ygo] pool: the decks won't inflate\n");
        return 0;
    }
    P.size = (uint32_t)out;
    P.decks = calloc(P.count, sizeof(*P.decks));
    if (!P.decks)
        return 0;
    for (i = 0; i < P.count; i++)
    {
        struct PoolDeck *d = &P.decks[i];
        uint8_t nlen, nm, nx, era = 0;
        unsigned k;
        if (at + 10 > P.size)
            break;
        d->rating = U16(P.data + at);
        d->format = P.data[at + 2];
        d->comp = P.data[at + 3];
        d->year = U16(P.data + at + 4);
        d->placing = P.data[at + 6];
        nlen = P.data[at + 7];
        d->nameAt = at + 8;
        d->nameLen = nlen;
        at += 8 + nlen;
        if (at + 2 > P.size)
            break;
        nm = P.data[at];
        nx = P.data[at + 1];
        d->off = at;
        at += 2;
        for (k = 0; k < (unsigned)nm + nx && at + 3 <= P.size; k++, at += 3)
        {
            const struct YgoCard *c = Ygo_Card(U16(P.data + at));
            if (c && c->era > era)
                era = c->era;
        }
        d->era = era;
    }
    P.count = i;
    P.ok = P.count > 0;
    printf("[ygo] pool: %u generic duelists' decks (%u KB)\n", (unsigned)P.count, (unsigned)(P.size / 1024));
    return P.ok;
}

unsigned Ygo_PoolCount(void)
{
    return Load() ? P.count : 0;
}

// a deck the GEN allows: every card's era before the GEN's step (ALL: any)
static int InGen(const struct PoolDeck *d, unsigned gen)
{
    return !gen || gen > YGO_GEN_STEPS || d->era < gen;
}

// what the duelist picking may bring (Ygo_PoolPickIn): how competitive at most, the ratings between
static struct { unsigned comp; int lo, hi; } sAllow = {3, -1, -1};
static int Allowed(const struct PoolDeck *d, unsigned gen)
{
    return InGen(d, gen) && d->comp <= sAllow.comp && (sAllow.lo < 0 || d->rating >= sAllow.lo) &&
           (sAllow.hi < 0 || d->rating <= sAllow.hi);
}

static int MetLately(uint32_t i)
{
    unsigned k;
    for (k = 0; k < RECENT; k++)
        if (sRecent[k] == (int32_t)i)
            return 1;
    return 0;
}

void Ygo_PoolNoteMet(int index)
{
    if (index < 0 || MetLately((uint32_t)index))
        return;
    sRecent[sRecentAt] = index;
    sRecentAt = (uint8_t)((sRecentAt + 1) % RECENT);
}

// The deck's name and where it came from ("Top 8", the format), for the challenge screen and the log
int Ygo_PoolInfo(unsigned index, char *name, size_t size, unsigned *rating, unsigned *year, unsigned *placing)
{
    const struct PoolDeck *d;
    if (!Load() || index >= P.count)
        return 0;
    d = &P.decks[index];
    if (name && size)
        snprintf(name, size, "%.*s", (int)d->nameLen, (const char *)P.data + d->nameAt);
    if (rating)
        *rating = d->rating;
    if (year)
        *year = d->year;
    if (placing)
        *placing = d->placing;
    return 1;
}
unsigned Ygo_PoolComp(unsigned index)
{
    return Load() && index < P.count ? P.decks[index].comp : 0;
}

// The deck as cards that can be played now: the game's own list's limits kept (more copies of its own cards when
// that leaves it short); 0 when it can't make 40
int Ygo_PoolCards(unsigned index, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX])
{
    const uint8_t *p;
    unsigned nm, nx, k, c, m = 0, x = 0;
    if (!Load() || index >= P.count)
        return 0;
    p = P.data + P.decks[index].off;
    nm = p[0];
    nx = p[1];
    p += 2;
    memset(main, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    memset(extra, 0, sizeof(uint16_t) * YGO_FUSION_MAX);
    for (k = 0; k < nm + nx; k++, p += 3)
    {
        unsigned id = U16(p), copies = p[2], allow;
        const struct YgoCard *card = Ygo_Card(id);
        if (!card || !Ygo_CardReady(id))
            continue;
        allow = Ygo_GameLimit(id);
        for (c = 0; c < copies && c < allow; c++)
        {
            if (k >= nm && x < YGO_FUSION_MAX)
                extra[x++] = (uint16_t)id;
            else if (k < nm && m < YGO_DECK_MAX)
                main[m++] = (uint16_t)id;
        }
    }
    // the game's list took some out: more copies of its own Main Deck cards it allows
    p = P.data + P.decks[index].off + 2;
    for (k = 0; k < nm && m < YGO_DECK_MIN; k++)
    {
        unsigned id = U16(p + 3 * k), have = 0, i;
        if (!Ygo_Card(id) || !Ygo_CardReady(id))
            continue;
        for (i = 0; i < m; i++)
            have += main[i] == id;
        while (have < Ygo_GameLimit(id) && m < YGO_DECK_MIN)
            main[m++] = (uint16_t)id, have++;
    }
    return m >= YGO_DECK_MIN;
}

// A deck near target (target < 0: any) the GEN allows, by the person's own number (seed): the nearer the likelier,
// those met lately only when nothing else fits; -1 when there's none
static int Pick(int target, unsigned gen, uint32_t seed)
{
    uint32_t i, lo = 0, hi, best = ~0u, window, n;
    uint64_t total = 0, pick;
    int fresh = 0;
    if (!Load())
        return -1;
    n = P.count;
    hi = n;
    if (target >= 0)
    {
        // (weakest first: the decks within reach of target, by halving)
        uint32_t a = 0, b = n;
        while (a < b)
        {
            uint32_t mid = (a + b) / 2;
            if ((int)P.decks[mid].rating < target)
                a = mid + 1;
            else
                b = mid;
        }
        for (window = 40;; window *= 2)
        {
            uint32_t l = a, h = a, ok = 0;
            while (l > 0 && target - (int)P.decks[l - 1].rating <= (int)window)
                l--;
            while (h < n && (int)P.decks[h].rating - target <= (int)window)
                h++;
            for (i = l; i < h && ok < 8; i++)
                ok += Allowed(&P.decks[i], gen) && !MetLately(i);
            if (ok >= 8 || window > 2000 || (l == 0 && h == n))
            {
                lo = l, hi = h;
                break;
            }
        }
        (void)best;
    }
    for (i = lo; i < hi; i++)
        fresh |= Allowed(&P.decks[i], gen) && !MetLately(i);
    for (i = lo; i < hi; i++)
        if (Allowed(&P.decks[i], gen) && (!fresh || !MetLately(i)))
            total += target < 0 ? 1 : 1 + 2000 / (8 + (uint32_t)abs((int)P.decks[i].rating - target));
    if (!total)
        return -1;
    pick = ((uint64_t)seed * 2654435761u) % total;
    for (i = lo; i < hi; i++)
        if (Allowed(&P.decks[i], gen) && (!fresh || !MetLately(i)))
        {
            uint64_t w = target < 0 ? 1 : 1 + 2000 / (8 + (uint32_t)abs((int)P.decks[i].rating - target));
            if (pick < w)
                return (int)i;
            pick -= w;
        }
    return -1;
}

int Ygo_PoolPick(int target, unsigned gen, uint32_t seed)
{
    sAllow.comp = 3, sAllow.lo = sAllow.hi = -1;
    return Pick(target, gen, seed);
}
// ... no more competitive than comp (0 casual .. 3 a tournament deck), its rating between lo and hi (< 0: no bound)
int Ygo_PoolPickIn(int target, unsigned gen, unsigned comp, int lo, int hi, uint32_t seed)
{
    int at;
    sAllow.comp = comp, sAllow.lo = lo, sAllow.hi = hi;
    at = Pick(target, gen, seed);
    sAllow.comp = 3, sAllow.lo = sAllow.hi = -1;
    return at;
}

// the weakest and the strongest rating the GEN allows (a duelist early on picks from the lower part)
int Ygo_PoolRange(unsigned gen, int *lo, int *hi)
{
    uint32_t i;
    *lo = *hi = -1;
    if (!Load())
        return 0;
    for (i = 0; i < P.count; i++)
        if (InGen(&P.decks[i], gen))
        {
            if (*lo < 0)
                *lo = P.decks[i].rating;
            *hi = P.decks[i].rating;
        }
    return *lo >= 0;
}

// the rating at a share (0-1024) of the way up the decks the GEN allows (the badges' tiers: how strong a deck may be)
int Ygo_PoolQuantile(unsigned gen, unsigned share)
{
    uint32_t i, n = 0, want, k = 0;
    if (!Load())
        return -1;
    for (i = 0; i < P.count; i++)
        n += InGen(&P.decks[i], gen);
    if (!n)
        return -1;
    want = (uint32_t)((uint64_t)(n - 1) * (share > 1024 ? 1024 : share) / 1024);
    for (i = 0; i < P.count; i++)
        if (InGen(&P.decks[i], gen) && k++ == want)
            return P.decks[i].rating;
    return P.decks[P.count - 1].rating;
}
