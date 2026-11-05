// Moemon Kai: MAGIC's booster packs, set by set (gMtgSets: Fourth Edition to Scourge, src/mtg/mtg_sets_data.c from
// tools/mtg/fetch_sets.py), as the POKeMON and YU-GI-OH! sides have theirs: boosters are won in duels or bought at the
// CARD SHOP, held in the CARD CASE by set, and opened there. A booster is drawn from the set's cards the game plays
// (Mtg_CanUse), slot by slot by the rarity they had in the set (a slot with none of its rarity takes the next one
// down), no card twice in a pack while the set has enough. Plain C, no SDL: tools/mtg/test.sh tests it.
#include <string.h>
#include "mtg/collection.h"

// a set's usable cards, worked out once per pack load (the ids move with the pack, the names don't)
#define SET_CARDS_MAX 512   // (the biggest, Fifth Edition: 429)
static struct {
    int count;
    uint16_t ids[SET_CARDS_MAX];
    uint8_t rarity[SET_CARDS_MAX];
} sSet[MTG_SETS];
static int sDbCount = -1;

static void Prepare(void)
{
    int s, i;
    if (sDbCount == Mtg_DbCount())
        return;
    sDbCount = Mtg_DbCount();
    for (s = 0; s < MTG_SETS; s++) {
        const MtgSet *set = &gMtgSets[s];
        sSet[s].count = 0;
        for (i = 0; i < set->count && sSet[s].count < SET_CARDS_MAX; i++) {
            int id = Mtg_CardByHash(gMtgSetCards[set->first + i].hash);
            if (id < 0 || !Mtg_CanUse(id) || Mtg_IsBasicLand(id))
                continue;
            sSet[s].ids[sSet[s].count] = (uint16_t)id;
            sSet[s].rarity[sSet[s].count++] = gMtgSetCards[set->first + i].rarity;
        }
    }
}

int Mtg_SetCards(int set, uint16_t *out, uint8_t *rarity)
{
    int i;
    if (set < 0 || set >= MTG_SETS)
        return 0;
    Prepare();
    for (i = 0; i < sSet[set].count; i++) {
        if (out)
            out[i] = sSet[set].ids[i];
        if (rarity)
            rarity[i] = sSet[set].rarity[i];
    }
    return sSet[set].count;
}

static int CountOf(int set, int rarity)
{
    int i, n = 0;
    for (i = 0; i < sSet[set].count; i++)
        n += sSet[set].rarity[i] == rarity;
    return n;
}

// enough plays to fill its boosters with different cards: its commons' slots and more, and its rarer slots
int Mtg_SetAvailable(int set)
{
    const MtgSet *s;
    if (set < 0 || set >= MTG_SETS)
        return 0;
    Prepare();
    s = &gMtgSets[set];
    return CountOf(set, MTG_RARITY_COMMON) >= s->commons + 3
        && CountOf(set, MTG_RARITY_UNCOMMON) + CountOf(set, MTG_RARITY_RARE) >= s->uncommons + s->rares + 2
        && CountOf(set, MTG_RARITY_RARE) >= 1;
}

int Mtg_RandomSet(uint32_t *rng)
{
    int sets[MTG_SETS], n = 0, i;
    for (i = 0; i < MTG_SETS; i++)
        if (Mtg_SetAvailable(i))
            sets[n++] = i;
    return n ? sets[Mtg_Rand(rng) % (uint32_t)n] : 0;
}

int Mtg_SetByCode(const char *code)
{
    int i;
    for (i = 0; i < MTG_SETS; i++)
        if (strcmp(gMtgSets[i].code, code) == 0)
            return i;
    return -1;
}

// the card on its pack: its rare with the highest mana value (a creature first), by name when even
int Mtg_SetMascot(int set)
{
    int i, best = -1, bestScore = -1;
    Prepare();
    if (set < 0 || set >= MTG_SETS)
        return -1;
    for (i = 0; i < sSet[set].count; i++) {
        int id = sSet[set].ids[i], score;
        const MtgCardDef *c = Mtg_Card(id);
        if (sSet[set].rarity[i] != MTG_RARITY_RARE)
            continue;
        score = ((c->types & MTG_T_CREATURE) ? 100 : 0) + Mtg_ManaCostTotal(&c->cost) * 10 + (c->power > 0 ? c->power : 0);
        if (score > bestScore || (score == bestScore && strcmp(c->name, Mtg_Card(best)->name) < 0))
            bestScore = score, best = id;
    }
    return best >= 0 ? best : sSet[set].count ? sSet[set].ids[0] : -1;
}

int Mtg_SetOwned(const MtgCollection *c, int set, int *total)
{
    int i, have = 0;
    Prepare();
    if (set < 0 || set >= MTG_SETS) {
        *total = 0;
        return 0;
    }
    for (i = 0; i < sSet[set].count; i++)
        have += c->owned[sSet[set].ids[i]] != 0;
    *total = sSet[set].count;
    return have;
}

int Mtg_AwardPack(MtgCollection *c, int set)
{
    if (set < 0 || set >= MTG_SETS || c->packs[set] >= MTG_PACKS_MAX)
        return 0;
    c->packs[set]++;
    return 1;
}

// one card of the rarity (or the next one down when the set has none), not one already in the pack if it can help it
static int Draw(MtgCollection *c, int set, int rarity, const uint16_t *pack, int n, uint8_t *got)
{
    for (; rarity >= 0; rarity--) {
        int pool = CountOf(set, rarity), tries;
        if (!pool)
            continue;
        for (tries = 0; tries < 64; tries++) {
            int k = (int)(Mtg_Rand(&c->rng) % (uint32_t)pool), i, j, dup = 0;
            for (i = 0, j = -1; i < sSet[set].count; i++)
                if (sSet[set].rarity[i] == rarity && ++j == k)
                    break;
            for (j = 0; j < n; j++)
                dup |= pack[j] == sSet[set].ids[i];
            if (!dup || tries == 63) {
                *got = (uint8_t)rarity;
                return sSet[set].ids[i];
            }
        }
    }
    return -1;
}

int Mtg_OpenPack(MtgCollection *c, int set, uint16_t out[MTG_PACK_MAX], uint8_t rarity[MTG_PACK_MAX])
{
    const MtgSet *s;
    int slot, n = 0;
    if (set < 0 || set >= MTG_SETS || !c->packs[set])
        return 0;
    Prepare();
    s = &gMtgSets[set];
    c->packs[set]--;
    for (slot = 0; slot < s->commons + s->uncommons + s->rares && n < MTG_PACK_MAX; slot++) {
        int want = slot < s->commons ? MTG_RARITY_COMMON : slot < s->commons + s->uncommons ? MTG_RARITY_UNCOMMON
                 : MTG_RARITY_RARE, id;
        uint8_t got = 0;
        id = Draw(c, set, want, out, n, &got);
        if (id < 0)
            continue;
        if (c->owned[id] < MTG_OWNED_MAX)
            c->owned[id]++;
        rarity[n] = got;
        out[n++] = (uint16_t)id;
    }
    if (c->opened[set] < 255)
        c->opened[set]++;
    if (c->packsOpened < 0xFFFF)
        c->packsOpened++;
    return n;
}
