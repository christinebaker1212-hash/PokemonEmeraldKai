/* What Yu-Gi-Oh!'s power ratings (ygo_card_ratings.c, tools/ygo/rate_cards.sh) say: a deck's rating from its cards,
 * the chance one side beats another, the skill correction (src/ygo/ygo_internal.h). No libm in the game. */
#include "ygo_internal.h"
#include "ygo/collection.h"
#include "ygo/duel.h"
#include <string.h>

/* One of the Game Boy Advance games' opponents' decks (ygo_npc_decks.c, weakest first) as cards that can be played:
 * the game's own list's limits kept, more copies of the deck's own cards when that leaves it short; 0 when it can't
 * make 40 (ygo_field.c's duelists, the phone's FREE DUEL) */
int Ygo_NpcDeckCards(unsigned index, uint16_t main[YGO_DECK_MAX], uint16_t extra[YGO_FUSION_MAX])
{
    unsigned i, m = 0, x = 0, pass;
    const struct YgoNpcDeck *d;
    if (index >= gYgoNpcDeckCount)
        return 0;
    d = &gYgoNpcDecks[index];
    memset(main, 0, sizeof(uint16_t) * YGO_DECK_MAX);
    memset(extra, 0, sizeof(uint16_t) * YGO_FUSION_MAX);
    for (i = 0; i < d->count; i++)
    {
        unsigned id = d->cards[i], k, have = 0;
        const struct YgoCard *c = Ygo_Card(id);
        if (!c || !Ygo_CardReady(id))
            continue;
        for (k = 0; k < m; k++)
            have += main[k] == id;
        for (k = 0; k < x; k++)
            have += extra[k] == id;
        if (have >= Ygo_GameLimit(id))
            continue;   // (the game's own list)
        if (c->frame == YGO_FUSION && x < YGO_FUSION_MAX)
            extra[x++] = (uint16_t)id;
        else if (c->frame != YGO_FUSION && m < YGO_DECK_MAX)
            main[m++] = (uint16_t)id;
    }
    // the game's list took some out: more copies of the deck's own cards it allows (Monsters first)
    for (pass = 0; pass < 2 && m < YGO_DECK_MIN; pass++)
        for (i = 0; i < d->count && m < YGO_DECK_MIN; i++)
        {
            unsigned id = d->cards[i], k, have = 0;
            const struct YgoCard *c = Ygo_Card(id);
            if (!c || !Ygo_CardReady(id) || c->frame == YGO_FUSION || (pass == 0) != Ygo_IsMonster(c))
                continue;
            for (k = 0; k < m; k++)
                have += main[k] == id;
            if (have < Ygo_GameLimit(id))
                main[m++] = (uint16_t)id;
        }
    return m >= YGO_DECK_MIN;
}

int Ygo_DeckRating(const uint16_t *main)
{
    long total = 0;
    unsigned i, n = 0;
    for (i = 0; i < YGO_DECK_MAX && main[i]; i++)
        if (main[i] <= YGO_CARD_COUNT && gYgoCardRating[main[i]])
            total += gYgoCardRating[main[i]], n++;
    if (!n)
        return 1500;
    return (int)(gYgoDeckFitA + (long long)gYgoDeckFitB * (total / (long)n) / 1024);
}

/* 10^(d/400) */
static double Pow10Over400(int d)
{
    double base = 1.0057730762, out = 1;
    unsigned n = (unsigned)(d < 0 ? -d : d);
    while (n)
    {
        if (n & 1)
            out *= base;
        base *= base;
        n >>= 1;
    }
    return d < 0 ? 1 / out : out;
}
unsigned Ygo_Chance(int mine, int theirs) { return (unsigned)(1000 / (1 + Pow10Over400(theirs - mine)) + 0.5); }

void Ygo_SkillResult(struct YgoCollection *c, int mine, int theirs, int won)
{
    int k = c->rated < 20 ? 48 : 24, e = (int)Ygo_Chance(mine + c->skill, theirs), s = won ? 1000 : 0, skill;
    skill = c->skill + (k * (s - e) + (s >= e ? 500 : -500)) / 1000;
    c->skill = (int16_t)(skill > YGO_SKILL_MAX ? YGO_SKILL_MAX : skill < -YGO_SKILL_MAX ? -YGO_SKILL_MAX : skill);
    if (c->rated < 0xFFFF)
        c->rated++;
}
