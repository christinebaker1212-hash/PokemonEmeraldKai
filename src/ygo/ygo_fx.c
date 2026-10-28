/* Yu-Gi-Oh! card effects: the table of every card's effects (each ygo_fx_*.c file holds a stage's cards). */
#include <string.h>
#include "ygo_internal.h"

/* Each file's table; a card's effects sit together in one table */
extern const struct YgoEffect gYgoFxBasic[];
extern const unsigned gYgoFxBasicCount;

static const struct
{
    const struct YgoEffect *list;
    const unsigned *count;
} sTables[] = {
    {gYgoFxBasic, &gYgoFxBasicCount},
};

static uint16_t sFirst[YGO_CARD_COUNT + 1], sCount[YGO_CARD_COUNT + 1];
static uint8_t sTable[YGO_CARD_COUNT + 1], sBuilt;

static void Build(void)
{
    unsigned t, i;
    for (t = 0; t < sizeof(sTables) / sizeof(sTables[0]); t++)
        for (i = 0; i < *sTables[t].count; i++)
        {
            unsigned card = sTables[t].list[i].card;
            if (card > YGO_CARD_COUNT)
                continue;
            if (!sCount[card])
            {
                sFirst[card] = i;
                sTable[card] = t;
            }
            sCount[card]++;
        }
    sBuilt = 1;
}

const struct YgoEffect *Ygo_Effects(unsigned card, unsigned *count)
{
    if (!sBuilt)
        Build();
    if (card > YGO_CARD_COUNT || !sCount[card])
    {
        *count = 0;
        return NULL;
    }
    *count = sCount[card];
    return sTables[sTable[card]].list + sFirst[card];
}

int Ygo_CardHasRules(unsigned card)
{
    unsigned n;
    Ygo_Effects(card, &n);
    return n > 0;
}

int Ygo_CardReady(unsigned card)
{
    const struct YgoCard *c = Ygo_Card(card);
    if (!c || c->frame == YGO_TOKEN)
        return 0;
    if (c->frame == YGO_NORMAL)
        return 1;
    return Ygo_CardHasRules(card);
}
