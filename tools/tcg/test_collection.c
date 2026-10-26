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
    c.dust = 50;
    assert(Tcg_CraftCard(&c, TCG_CARD_CHARIZARD));
    assert(c.dust == 0);
    assert(!Tcg_CraftCard(&c, TCG_CARD_CHARIZARD));
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
    snprintf(path, sizeof(path), "%s/profile.cards", argv[1]);
#if MOEMON_DEV_BUILD
    assert(!Tcg_CollectionSave(&c, path));
    {
        FILE *f = fopen(path, "rb");
        assert(!f);
    }
    puts("PASS dev collection writes are refused");
#else
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
#endif
    puts("PASS catalogue, starters, deck limits, deterministic packs, duplicate protection and crafting");
    return 0;
}
