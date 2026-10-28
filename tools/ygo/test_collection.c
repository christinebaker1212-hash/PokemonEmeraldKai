/* Yu-Gi-Oh! collection tests (tools/ygo/test.sh): pull rates, starter decks, deck rules, the save file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ygo/collection.h"
#include "ygo/duel.h"

#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                     \
        }                                                                \
    } while (0)

int main(void)
{
    static struct YgoCollection c, back;
    uint16_t pack[YGO_PACK_SIZE];
    unsigned counts[7] = {0}, i, k, packs = 24000;
    char why[160];
    /* pull rates over 24000 Legend of Blue Eyes packs (1000 boxes) */
    Ygo_CollectionInit(&c, 42);
    CHECK(Ygo_SetAvailable(YGO_SET_LOB) && Ygo_SetAvailable(YGO_SET_MRD) && !Ygo_SetAvailable(YGO_SET_SDY));
    for (i = 0; i < packs; i++)
    {
        c.packs[YGO_SET_LOB] = 1;
        CHECK(Ygo_OpenPack(&c, YGO_SET_LOB, pack));
        for (k = 0; k < YGO_PACK_SIZE; k++)
        {
            unsigned j, r = 0;
            CHECK(pack[k]);
            for (j = 0; j < gYgoSets[YGO_SET_LOB].count; j++)
                if (gYgoSets[YGO_SET_LOB].cards[j].card == pack[k])
                    r = gYgoSets[YGO_SET_LOB].cards[j].rarity;
            CHECK(k == YGO_PACK_SIZE - 1 ? r >= YGO_RARE : r < YGO_RARE);
            counts[r]++;
        }
    }
    printf("  24000 packs: %u Secret (expect ~1000), %u Ultra (~2000), %u Super (~4000), %u Rare (~17000)\n",
           counts[YGO_SECRET_RARE], counts[YGO_ULTRA_RARE], counts[YGO_SUPER_RARE], counts[YGO_RARE]);
    CHECK(counts[YGO_SECRET_RARE] > 850 && counts[YGO_SECRET_RARE] < 1150);
    CHECK(counts[YGO_ULTRA_RARE] > 1800 && counts[YGO_ULTRA_RARE] < 2200);
    CHECK(counts[YGO_SUPER_RARE] > 3700 && counts[YGO_SUPER_RARE] < 4300);
    CHECK(counts[YGO_SHORT_PRINT] * 2 < counts[YGO_COMMON]);
    /* every Starter Deck is a legal deck of cards that work */
    for (i = 0; i < YGO_SET_COUNT; i++)
    {
        if (!gYgoSets[i].starter)
            continue;
        Ygo_CollectionInit(&c, 7);
        CHECK(Ygo_ChooseStarter(&c, i));
        if (!Ygo_DeckValidate(c.decks[0].main, c.decks[0].extra, c.owned, why, sizeof(why)))
        {
            fprintf(stderr, "%s: %s (%u cards)\n", gYgoSets[i].name, why, Ygo_DeckCount(c.decks[0].main, YGO_DECK_MAX));
            exit(1);
        }
        printf("  %s: %u cards, %u in the Fusion Deck\n", c.decks[0].name, Ygo_DeckCount(c.decks[0].main, YGO_DECK_MAX),
               Ygo_DeckCount(c.decks[0].extra, YGO_FUSION_MAX));
    }
    /* deck rules: copies, Limited cards, owning them */
    Ygo_CollectionInit(&c, 9);
    c.owned[YGO_CARD_POT_OF_GREED] = 3;
    c.owned[YGO_CARD_MYSTICAL_ELF] = 5;
    CHECK(Ygo_DeckAdd(&c, 1, YGO_CARD_POT_OF_GREED, why, sizeof(why)));
    CHECK(!Ygo_DeckAdd(&c, 1, YGO_CARD_POT_OF_GREED, why, sizeof(why)));   /* Limited */
    for (i = 0; i < 3; i++)
        CHECK(Ygo_DeckAdd(&c, 1, YGO_CARD_MYSTICAL_ELF, why, sizeof(why)));
    CHECK(!Ygo_DeckAdd(&c, 1, YGO_CARD_MYSTICAL_ELF, why, sizeof(why)));   /* 3 copies */
    CHECK(Ygo_DeckRemove(&c, 1, YGO_CARD_MYSTICAL_ELF));
    CHECK(Ygo_CopyLimit(YGO_CARD_SANGAN) == 2 && Ygo_CopyLimit(YGO_CARD_RAIGEKI) == 1);
    /* the file */
    Ygo_CollectionInit(&c, 11);
    CHECK(Ygo_ChooseStarter(&c, YGO_SET_SDK));
    c.packs[YGO_SET_MRD] = 4;
    c.wins = 12;
    c.owner = 0x12345678;
    Ygo_DailyTake(&c, 99, 300);
    CHECK(Ygo_CollectionSave(&c, "/tmp/ygo-test.ygo"));
    CHECK(Ygo_CollectionLoad(&back, "/tmp/ygo-test.ygo"));
    CHECK(!memcmp(back.owned, c.owned, sizeof(c.owned)) && !memcmp(&back.decks, &c.decks, sizeof(c.decks)));
    CHECK(back.packs[YGO_SET_MRD] == 4 && back.wins == 12 && back.owner == 0x12345678 && back.starter == c.starter);
    CHECK(!Ygo_DailyAvailable(&back, 99, 300) && Ygo_DailyAvailable(&back, 99, 301));
    remove("/tmp/ygo-test.ygo");
    printf("PASS pull rates, the Starter Decks, deck rules and the save file\n");
    return 0;
}
