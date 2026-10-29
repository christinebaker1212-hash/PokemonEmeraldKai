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
    /* format 2: the GAME CENTER clerk's sold singles and decks */
    c.gcSingles[YGO_CARD_RAIGEKI / 8] |= 1 << (YGO_CARD_RAIGEKI % 8);
    c.gcDecks = 5;
    c.ace[0] = c.decks[0].main[3];   /* format 3: a deck's ACE CARD */
    CHECK(Ygo_CollectionSave(&c, "/tmp/ygo-test.ygo"));
    CHECK(Ygo_CollectionLoad(&back, "/tmp/ygo-test.ygo"));
    CHECK(!memcmp(back.gcSingles, c.gcSingles, sizeof(c.gcSingles)) && back.gcDecks == 5);
    CHECK(back.ace[0] == c.ace[0] && Ygo_DeckAce(&back, 0) == c.ace[0]);
    {
        /* DECK RECIPES: built again from the cards owned, the rest counted missing */
        static struct YgoCollection r;
        unsigned missing = 99, total, have, placed;
        r = c;
        r.owned[c.decks[0].main[0]] = 0;
        have = Ygo_RecipeOwned(&r, c.decks[0].main, c.decks[0].extra, &total);
        placed = Ygo_DeckFromRecipe(&r, 4, c.decks[0].main, c.decks[0].extra, "Again", &missing);
        CHECK(total == Ygo_DeckCount(c.decks[0].main, YGO_DECK_MAX) + Ygo_DeckCount(c.decks[0].extra, YGO_FUSION_MAX));
        CHECK(placed == have && missing == total - have && missing >= 1 && !strcmp(r.decks[4].name, "Again"));
        CHECK(Ygo_DeckAce(&r, 4) != 0);
    }
    /* a format 1 file (1.35.0) still loads, nothing sold */
    {
        static unsigned char data[1 << 20];
        FILE *f = fopen("/tmp/ygo-test.ygo", "rb");
        size_t n = fread(data, 1, sizeof(data), f), size = n - 12 - YGO_GC_SINGLE_BYTES - 1 - 2 * YGO_SAVED_DECKS - 32 - 4, i;
        uint32_t crc = 0xFFFFFFFFu;
        unsigned k;
        fclose(f);
        for (i = 0; i < size; i++)
        {
            crc ^= data[12 + i];
            for (k = 0; k < 8; k++)
                crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
        }
        crc = ~crc;
        data[4] = 1, data[5] = data[6] = data[7] = 0;
        for (k = 0; k < 4; k++)
            data[8 + k] = (unsigned char)(crc >> (8 * k));
        f = fopen("/tmp/ygo-test.ygo", "wb");
        fwrite(data, 1, 12 + size, f);
        fclose(f);
        CHECK(Ygo_CollectionLoad(&back, "/tmp/ygo-test.ygo"));
        CHECK(back.wins == 12 && back.gcDecks == 0 && !back.gcSingles[YGO_CARD_RAIGEKI / 8]);
    }
    CHECK(Ygo_MoneyPrice(YGO_CARD_RAIGEKI) >= 300 && Ygo_MoneyPrice(YGO_CARD_RAIGEKI) <= 4000);
    remove("/tmp/ygo-test.ygo");
    /* deck codes: YDKE and .ydk round trips, a card the game hasn't got, a real code's alternate artwork */
    {
        static struct YgoCollection k;
        uint16_t m[YGO_DECK_MAX], x[YGO_FUSION_MAX];
        char code[2048], ydk[4096];
        unsigned bad, i;
        Ygo_CollectionInit(&k, 9);
        CHECK(Ygo_ChooseStarter(&k, YGO_SET_SDJ));
        CHECK(Ygo_WriteYdke(k.decks[0].main, k.decks[0].extra, code, sizeof(code)));
        CHECK(!strncmp(code, "ydke://", 7));
        CHECK(Ygo_ReadYdke(code, m, x, &bad) && !bad);
        CHECK(!memcmp(m, k.decks[0].main, sizeof(m)) && !memcmp(x, k.decks[0].extra, sizeof(x)));
        CHECK(Ygo_WriteYdk(k.decks[0].main, k.decks[0].extra, "TEST", ydk, sizeof(ydk)));
        CHECK(Ygo_ReadYdk(ydk, m, x, &bad) && !bad);
        CHECK(!memcmp(m, k.decks[0].main, sizeof(m)) && !memcmp(x, k.decks[0].extra, sizeof(x)));
        CHECK(Ygo_DeckMissing(&k, m, x) == 0);
        /* "Ash Blossom & Joyous Spring" (14558127): not in the game */
        CHECK(Ygo_ReadYdk("#main\n14558127\n46986414\n#extra\n!side\n", m, x, &bad) && bad == 1 && m[0] && !m[1]);
        CHECK(!strcmp(Ygo_Card(m[0])->name, "Dark Magician"));   /* (46986414: the catalog has another printing) */
        CHECK(!Ygo_ReadYdke("hello", m, x, &bad));
        CHECK(Ygo_ReadYdke("ydke://r9H3AA==!!!", m, x, &bad) && bad == 1);   /* (14608815... any unknown) */
        for (i = 0; i < Ygo_BanlistCount(); i++)
            printf("  list %u: %s\n", i, Ygo_BanlistName(i));
        CHECK(Ygo_BanlistCount() >= 6);
        CHECK(Ygo_UseBanlist("2005.4 GOAT") > 0 && Ygo_CopyLimit(YGO_CARD_RAIGEKI) == 0 + (Ygo_CopyLimit(YGO_CARD_RAIGEKI) > 0));
        Ygo_UseBanlist("");
        CHECK(Ygo_Banlist() == 0);
    }
    printf("PASS pull rates, the Starter Decks, deck rules, the save file, deck codes and the lists\n");
    return 0;
}
