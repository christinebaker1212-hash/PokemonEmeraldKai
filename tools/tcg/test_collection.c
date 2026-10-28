#include "mp_version.h"
#include "tcg/card_ids.h"
#include "tcg/tcg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned total(const struct TcgCollection *c)
{
    unsigned i, n = 0;
    for (i = 1; i <= TCG_CARD_COUNT; i++)
        n += c->owned[i];
    return n;
}
int main(int argc, char **argv)
{
    struct TcgCollection c, before, other;
    uint16_t cards[TCG_DECK_SIZE], a[10], b[10];
    char reason[128], path[1024];
    unsigned s, p, i;
    assert(argc == 2);
    for (i = 1; i <= TCG_CARD_COUNT; i++)
        assert(Tcg_Card(i)->id == i && Tcg_Card(i)->name[0]);
    assert(!Tcg_Card(0) && !Tcg_Card(229));
    for (s = 0; s < 3; s++)
    {
        Tcg_CollectionInit(&c, 123);
        assert(Tcg_ChooseStarter(&c, s));
        assert(Tcg_DeckValidate(c.decks[0].cards, c.owned, reason, sizeof(reason)));
        before = c;
        assert(!Tcg_ChooseStarter(&c, s));
        assert(!memcmp(&before, &c, sizeof(c)));
    }
    for (i = 0; i < TCG_DECK_SIZE; i++)
        cards[i] = TCG_CARD_FIRE_ENERGY;
    assert(!Tcg_DeckValidate(cards, NULL, reason, sizeof(reason)));
    assert(strstr(reason, "Basic"));
    cards[0] = TCG_CARD_CHARMANDER;
    assert(Tcg_DeckValidate(cards, NULL, reason, sizeof(reason)));
    memset(c.owned, 0, sizeof(c.owned));
    assert(!Tcg_DeckValidate(cards, c.owned, reason, sizeof(reason)));
    c.owned[TCG_CARD_CHARMANDER] = 1;
    assert(Tcg_DeckValidate(cards, c.owned, reason, sizeof(reason)));
    for (i = 0; i < 3; i++)
        cards[i] = TCG_CARD_PIKACHU_LV12;
    for (i = 3; i < 5; i++)
        cards[i] = TCG_CARD_PIKACHU_LV14;
    assert(!Tcg_DeckValidate(cards, NULL, reason, sizeof(reason)));
    assert(strstr(reason, "same name"));
    cards[0] = 0;
    assert(!Tcg_DeckValidate(cards, NULL, reason, sizeof(reason)));
    Tcg_CollectionInit(&c, 456);
    assert(!Tcg_AwardPack(&c, 0));
    assert(!Tcg_OpenPack(&c, 0, a));
    assert(Tcg_ChooseStarter(&c, 0));
    for (p = 0; p < TCG_PACK_TYPES; p++)
    {
        unsigned count = total(&c), rarities[3] = {0};
        assert(Tcg_AwardPack(&c, p));
        other = c;
        assert(Tcg_OpenPack(&c, p, a));
        assert(Tcg_OpenPack(&other, p, b));
        assert(!memcmp(a, b, sizeof(a)));
        for (i = 0; i < 9; i++)
        {
            const struct TcgCard *d = Tcg_Card(a[i]);
            assert(d->set == p);
            assert(d->kind != TCG_ENERGY);
            rarities[d->rarity]++;
        }
        assert(rarities[0] == 5 && rarities[1] == 3 && rarities[2] == 1);
        assert(a[9] >= 1 && a[9] <= 6);
        assert(total(&c) == count + 10);
        assert(!Tcg_OpenPack(&c, p, a));
    }
    c.owned[TCG_CARD_CHARMANDER] = 8;
    assert(Tcg_ExchangeDuplicates(&c, TCG_CARD_CHARMANDER, 4));
    assert(c.owned[TCG_CARD_CHARMANDER] == 4);
    before = c;
    assert(!Tcg_ExchangeDuplicates(&c, TCG_CARD_CHARMANDER, 1));
    assert(!memcmp(&before, &c, sizeof(c)));
    assert(!Tcg_ExchangeDuplicates(&c, TCG_CARD_FIRE_ENERGY, 1));
    c.dust = 80;
    assert(!Tcg_CraftCard(&c, TCG_CARD_CHARIZARD));
    assert(!Tcg_CraftCard(&c, TCG_CARD_BEEDRILL));
    c.dust = 15;
    assert(Tcg_PointCost(TCG_CARD_CHARMELEON) <= 15 && Tcg_CraftCard(&c, TCG_CARD_CHARMELEON));
    {
        /* Holos one pack in TCG_HOLO_ODDS, and never TCG_HOLO_PITY packs without a new one */
        unsigned holos = 0, opened = 0, longest = 0, run = 0, set = 1, k, have, total;
        struct TcgCollection h;
        Tcg_CollectionInit(&h, 77);
        assert(Tcg_ChooseStarter(&h, 0));
        while (opened < 3000)
        {
            unsigned newHolo = 0;
            uint16_t r[10];
            if (h.owned[1] > TCG_COLLECTION_MAX - 20)
                for (k = 1; k <= TCG_CARD_COUNT; k++)
                    if (h.owned[k] > 4)
                        h.owned[k] = 4;   /* (traded in) */
            {
                unsigned missingBefore = 0;
                for (k = 1; k <= TCG_CARD_COUNT; k++)
                    missingBefore += Tcg_IsHolo(k) && Tcg_Card(k)->set == set && !h.owned[k];
                assert(Tcg_AwardPack(&h, set) && Tcg_OpenPack(&h, set, r));
                opened++;
                holos += Tcg_IsHolo(r[8]);
                for (k = 1; k <= TCG_CARD_COUNT; k++)
                    newHolo += Tcg_IsHolo(k) && Tcg_Card(k)->set == set && !h.owned[k];
                if (missingBefore)
                {
                    run = newHolo < missingBefore ? 0 : run + 1;
                    if (run > longest)
                        longest = run;
                }
            }
            for (k = 0; k < 8; k++)
                assert(!Tcg_IsHolo(r[k]));
            if (opened == 400)
            {
                have = Tcg_SetOwned(&h, set, &total);
                assert(have == total);   /* 400 packs finish a set */
            }
        }
        assert(holos > 3000 / TCG_HOLO_ODDS - 120 && holos < 3000 / TCG_HOLO_ODDS + 120);
        assert(longest < TCG_HOLO_PITY);
        printf("PASS holos %u of %u packs, longest wait for a new one %u\n", holos, opened, longest);
    }
    Tcg_CollectionInit(&c, 123);
    assert(Tcg_ChooseStarter(&c, 0));
    assert(Tcg_DeckCopy(&c, 0, 1));
    assert(!memcmp(c.decks[0].cards, c.decks[1].cards, sizeof(c.decks[0].cards)));
    before = c;
    assert(!Tcg_DeckCopy(&c, 0, 0));
    assert(!memcmp(&c, &before, sizeof(c)));
    assert(!Tcg_DeckAdd(&c, 1, TCG_CARD_CHARMANDER, reason, sizeof(reason)));
    assert(Tcg_DeckRemove(&c, 1, TCG_CARD_CHARMANDER));
    assert(Tcg_DeckAdd(&c, 1, TCG_CARD_CHARMANDER, reason, sizeof(reason)));
    assert(Tcg_DeckValidate(c.decks[1].cards, c.owned, reason, sizeof(reason)));
    memset(c.decks[2].cards, 0, sizeof(c.decks[2].cards));
    c.owned[TCG_CARD_PIKACHU_LV12] = 4;
    c.owned[TCG_CARD_PIKACHU_LV14] = 4;
    for (i = 0; i < 4; i++)
        assert(Tcg_DeckAdd(&c, 2, TCG_CARD_PIKACHU_LV12, reason, sizeof(reason)));
    assert(!Tcg_DeckAdd(&c, 2, TCG_CARD_PIKACHU_LV14, reason, sizeof(reason)));
    assert(strstr(reason, "name"));
    c.owned[TCG_CARD_FIRE_ENERGY] = 0;
    assert(Tcg_DeckAdd(&c, 2, TCG_CARD_FIRE_ENERGY, reason, sizeof(reason)));
    assert(!Tcg_DeckAdd(&c, 2, TCG_CARD_MEW_LV8, reason, sizeof(reason)));
    puts("PASS deck editing, independent saved lists, print/name limits and free Basic Energy");
    {
        /* DECK RECIPES: built from what's owned, the rest counted missing; the ACE CARD stands for the deck while
         * it's in it */
        uint16_t recipe[TCG_DECK_SIZE] = {0};
        unsigned k, missing = 99, placed;
        struct TcgCollection r = c;
        for (k = 0; k < TCG_DECK_SIZE; k++)
            recipe[k] = k < 4 ? TCG_CARD_SQUIRTLE : k < 8 ? TCG_CARD_MEW_LV8 : 3;
        r.owned[TCG_CARD_SQUIRTLE] = 2;
        r.owned[TCG_CARD_MEW_LV8] = 0;
        assert(Tcg_RecipeOwned(&r, recipe) == 2 + 52);
        placed = Tcg_DeckFromRecipe(&r, 5, recipe, "Recipe", &missing);
        assert(placed == 54 && missing == 6 && !strcmp(r.decks[5].name, "Recipe"));
        assert(Tcg_DeckAce(&r, 5) == TCG_CARD_SQUIRTLE);
        r.ace[5] = TCG_CARD_MEW_LV8; /* not in the deck: the deck's best Pokemon instead */
        assert(Tcg_DeckAce(&r, 5) == TCG_CARD_SQUIRTLE);
        r.ace[5] = 3;
        assert(Tcg_DeckAce(&r, 5) == 3);
        puts("PASS deck recipes from owned cards and the ACE CARD");
    }
    snprintf(path, sizeof(path), "%s/profile.cards", argv[1]);
#if MOEMON_DEV_BUILD
    assert(!Tcg_CollectionSave(&c, path));
    {
        FILE *f = fopen(path, "rb");
        assert(!f);
    }
    puts("PASS dev collection writes are refused");
#else
    c.skill = -123; /* CARD SCALING's correction (format 4) goes both ways */
    c.rated = 17;
    c.gcDecks = 5; /* format 6: the GAME CENTER's sales and the records */
    c.gcSingles[3] = 0x41;
    c.ace[0] = TCG_CARD_SQUIRTLE; /* format 7: a deck's ACE CARD */
    Tcg_RecordResult(&c, 0x01020301u, 1, 9000);
    Tcg_RecordResult(&c, 0x01020301u, 0, 9001);
    Tcg_RecordResult(&c, 0xFF000002u, 0, 9001);
    assert(Tcg_Record(&c, 0x01020301u)->wins == 1 && Tcg_Record(&c, 0x01020301u)->losses == 1 &&
           Tcg_Record(&c, 0x01020301u)->lastWin == 9000 && !Tcg_Record(&c, 0x0BADu));
    assert(Tcg_CollectionSave(&c, path));
    Tcg_CollectionInit(&other, 789);
    assert(Tcg_CollectionLoad(&other, path));
    assert(!memcmp(&c, &other, sizeof(c)));
    {
        FILE *f = fopen(path, "r+b");
        assert(f);
        assert(!fseek(f, 25, SEEK_SET));
        fputc(0xff, f);
        fclose(f);
    }
    before = other;
    assert(!Tcg_CollectionLoad(&other, path));
    assert(!memcmp(&before, &other, sizeof(other)));
    puts("PASS collection round-trip and corrupt-file rejection");
    {
        /* A 1.27 file (format 2): read, with the new parts empty and the CARD CLUB met. */
        unsigned char v2[12 + TCG_CARD_COUNT * 2 + TCG_SAVED_DECKS * (TCG_NAME_LEN + TCG_DECK_SIZE * 2) + TCG_PACK_TYPES * 2 + 24];
        unsigned char *q = v2 + 12;
        uint32_t crc = ~0u;
        unsigned k, b, d;
        FILE *f;
        struct TcgCollection old;
        Tcg_CollectionInit(&old, 5);
        assert(Tcg_ChooseStarter(&old, 1));
        old.packs[2] = 3;
        old.wins = 7;
#define PUT(v, n) do { uint32_t x_ = (v); unsigned i_; for (i_ = 0; i_ < (n); i_++) { *q++ = (unsigned char)x_; x_ >>= 8; } } while (0)
        for (k = 1; k <= TCG_CARD_COUNT; k++)
            PUT(old.owned[k], 2);
        for (d = 0; d < TCG_SAVED_DECKS; d++)
        {
            memcpy(q, old.decks[d].name, TCG_NAME_LEN);
            q += TCG_NAME_LEN;
            for (k = 0; k < TCG_DECK_SIZE; k++)
                PUT(old.decks[d].cards[k], 2);
        }
        for (k = 0; k < TCG_PACK_TYPES; k++)
            PUT(old.packs[k], 2);
        PUT(old.wins, 4);
        PUT(old.dust, 4);
        PUT(old.vouchers, 4);
        PUT(old.rng, 4);
        PUT(old.optedIn, 1);
        PUT(old.activeDeck, 1);
        PUT(old.fastAnimations, 1);
        PUT(old.quiet, 1);
        PUT(old.owner, 4);
        assert(q == v2 + sizeof(v2));
        for (k = 12; k < sizeof(v2); k++)
        {
            crc ^= v2[k];
            for (b = 0; b < 8; b++)
                crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320u : 0);
        }
        crc = ~crc;
        q = v2;
        memcpy(q, "MTCG", 4);
        q += 4;
        PUT(2, 4);
        PUT(crc, 4);
#undef PUT
        f = fopen(path, "wb");
        assert(f && fwrite(v2, 1, sizeof(v2), f) == sizeof(v2));
        fclose(f);
        Tcg_CollectionInit(&other, 9);
        assert(Tcg_CollectionLoad(&other, path));
        assert(other.optedIn && other.packs[2] == 3 && other.wins == 7 && other.owned[TCG_CARD_SQUIRTLE] == old.owned[TCG_CARD_SQUIRTLE]);
        assert(other.flags == (TCG_FLAG_TUTORIAL | TCG_FLAG_CLUB_MET) && !other.specialBeaten && !other.daily[0].key);
        assert(!other.skill && !other.rated && !other.gcDecks && !Tcg_Record(&other, 0x01020301u));
        /* ... and saved again as the current format */
        assert(Tcg_CollectionSave(&other, path));
        Tcg_CollectionInit(&before, 3);
        assert(Tcg_CollectionLoad(&before, path) && !memcmp(&before, &other, sizeof(other)));
        puts("PASS a 1.27 collection (format 2) loads and saves as format 7");
    }
#endif
    {
        /* Daily rewards: once a day per opponent, the oldest day makes room */
        unsigned k;
        Tcg_CollectionInit(&c, 1);
        assert(Tcg_DailyAvailable(&c, 0x1234, 100));
        Tcg_DailyTake(&c, 0x1234, 100);
        assert(!Tcg_DailyAvailable(&c, 0x1234, 100) && Tcg_DailyAvailable(&c, 0x1234, 101));
        for (k = 0; k < TCG_DAILY_SLOTS; k++)
            Tcg_DailyTake(&c, 0x5000 + k, 102 + k % 3);
        assert(Tcg_DailyAvailable(&c, 0x1234, 100)); /* pushed out: its day was the oldest */
        assert(!Tcg_DailyAvailable(&c, 0x5000 + TCG_DAILY_SLOTS - 1, 102 + (TCG_DAILY_SLOTS - 1) % 3));
        puts("PASS daily rewards");
    }
    {
        /* Spare cards, CARD POINTS and the card shop (no promo cards for points) */
        unsigned total, have;
        Tcg_CollectionInit(&c, 2);
        assert(Tcg_ChooseStarter(&c, 0));
        c.owned[TCG_CARD_CHARMANDER] = 9;
        assert(Tcg_Spare(&c, TCG_CARD_CHARMANDER) == 5);
        assert(Tcg_Spare(&c, TCG_CARD_FIRE_ENERGY) == 0);
        assert(Tcg_ExchangeDuplicates(&c, TCG_CARD_CHARMANDER, 5) && c.dust == 5 && c.owned[TCG_CARD_CHARMANDER] == 4);
        assert(!Tcg_ExchangeDuplicates(&c, TCG_CARD_CHARMANDER, 1));
        assert(Tcg_PointCost(TCG_CARD_MEW_LV8) == 0 && !Tcg_CraftCard(&c, TCG_CARD_MEW_LV8));
        /* A holo can't be picked; a plain rare costs 80 */
        assert(Tcg_IsHolo(TCG_CARD_CHARIZARD) && !Tcg_IsHolo(TCG_CARD_BEEDRILL));
        assert(Tcg_PointCost(TCG_CARD_CHARIZARD) == 0 && Tcg_PointValue(TCG_CARD_CHARIZARD) == 20);
        /* Rares aren't sold at the CARD CLUB; the GAME CENTER sells plain ones for money */
        assert(Tcg_PointCost(TCG_CARD_BEEDRILL) == 0 && Tcg_PointValue(TCG_CARD_BEEDRILL) == 6);
        assert(Tcg_ShopPrice(TCG_CARD_BEEDRILL) == 4000 && Tcg_ShopPrice(TCG_CARD_CHARIZARD) == 0);
        assert(Tcg_CraftCard(&c, TCG_CARD_CHARMELEON) == (Tcg_PointCost(TCG_CARD_CHARMELEON) <= 5));
        /* Set milestones: each reached once */
        have = Tcg_SetOwned(&c, 0, &total);
        assert(total > 0 && have <= total);
        for (have = 1; have <= TCG_CARD_COUNT; have++)
            if (gTcgCards[have].set == 0)
                c.owned[have] = 1;
        assert(Tcg_TakeMilestones(&c) == 7u);
        assert(Tcg_TakeMilestones(&c) == 0);
        assert(Tcg_SetOwned(&c, 0, &total) == total);
        puts("PASS spare cards, CARD POINTS, the card shop and set milestones");
    }
    puts("PASS catalogue, starters, deck limits, deterministic packs, duplicate protection and crafting");
    return 0;
}
