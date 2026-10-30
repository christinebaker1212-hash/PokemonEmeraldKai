// Triple Triad collection tests: the file round trip, packs, the starter, the rules spreading between regions.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tt/collection.h"
#include "tt/duel.h"

static int sFails, sChecks;
#define CHECK(c, ...) do { sChecks++; if (!(c)) { sFails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
    static struct TtCollection a, b;
    uint16_t pack[TT_PACK_SIZE];
    unsigned i, spreads = 0, abolishes = 0, k;
    const char *path = "/tmp/tt-tests/test.tt";
    Tt_CollectionInit(&a, 42);
    Tt_GiveStarter(&a);
    CHECK(Tt_DeckReady(&a, 0), "starter deck");
    for (i = 0; i < TT_PACK_TYPES; i++)
    {
        a.packs[i] = 2;
        CHECK(Tt_OpenPack(&a, i, pack), "pack %u", i);
        for (k = 0; k < TT_PACK_SIZE; k++)
            CHECK(Tt_Card(pack[k]) && Tt_Card(pack[k])->stars == i + 1 && a.owned[pack[k]], "pack card");
    }
    a.owner = 0x1234;
    a.wins = 7;
    a.regionRules[3] |= TT_RULE_BIT(TT_RULE_CHAOS);
    snprintf(a.decks[2].name, sizeof(a.decks[2].name), "MY DECK");
    a.carriedFrom = 2;
    a.carried = 5;
    CHECK(Tt_CollectionSave(&a, path), "save");
    CHECK(Tt_CollectionLoad(&b, path), "load");
    CHECK(!memcmp(a.owned, b.owned, sizeof(a.owned)) && b.wins == 7 && b.owner == 0x1234 &&
              b.regionRules[3] == a.regionRules[3] && !strcmp(b.decks[2].name, "MY DECK") && b.carried == 5 &&
              b.carriedFrom == 2 && b.packs[0] == 1 && b.packsOpened == 5 && b.optedIn,
          "round trip");
    // a damaged file doesn't load
    {
        FILE *f = fopen(path, "r+b");
        fseek(f, 40, SEEK_SET);
        fputc(0x55, f);
        fclose(f);
        CHECK(!Tt_CollectionLoad(&b, path), "damaged file");
    }
    // The rules spreading: carry PETALBURG's OPEN into SLATEPORT, then play there
    for (i = 0; i < 400; i++)
    {
        struct TtMix m;
        Tt_CollectionInit(&a, i + 1);
        Tt_PlayedIn(&a, TT_REGION_PETALBURG);
        CHECK(a.carried == Tt_RegionStartRules(TT_REGION_PETALBURG), "carried");
        m = Tt_MixRules(&a, TT_REGION_SLATEPORT, i * 2654435761u);
        if (m.kind == TT_MIX_SPREAD)
        {
            spreads++;
            CHECK(m.rule == TT_RULE_OPEN && (a.regionRules[TT_REGION_SLATEPORT] & TT_RULE_BIT(TT_RULE_OPEN)), "spread");
        }
        else if (m.kind == TT_MIX_ABOLISH)
        {
            abolishes++;
            CHECK((m.rule == TT_RULE_RANDOM || m.rule == TT_RULE_PLUS) &&
                      !(a.regionRules[TT_REGION_SLATEPORT] & TT_RULE_BIT(m.rule)), "abolish");
        }
        // nothing mixes in the region the rules came from
        CHECK(Tt_MixRules(&a, TT_REGION_PETALBURG, i).kind == TT_MIX_NONE, "home region");
    }
    CHECK(spreads > 80 && spreads < 160 && abolishes > 30, "spread %u abolish %u", spreads, abolishes);
    // The queen makes it likelier
    for (i = 0, k = 0; i < 400; i++)
    {
        Tt_CollectionInit(&a, i + 1);
        a.queenRegion = TT_REGION_SLATEPORT;
        Tt_PlayedIn(&a, TT_REGION_PETALBURG);
        k += Tt_MixRules(&a, TT_REGION_SLATEPORT, i * 2654435761u).kind == TT_MIX_SPREAD;
    }
    CHECK(k > spreads, "queen %u vs %u", k, spreads);
    // She brings rules from afar now and then, never past the limit
    for (i = 0, k = 0; i < 200; i++)
    {
        unsigned r = Tt_QueenMoves(&a, TT_REGION_EVER_GRANDE, 10, i * 7919u);
        if (r != 0xFF)
            k++;
        CHECK(__builtin_popcount(a.regionRules[TT_REGION_EVER_GRANDE]) <= TT_REGION_MAX_RULES, "rule limit");
    }
    CHECK(k >= 1, "queen rules");
    printf("collection: spreads %u, abolishes %u; %d checks, %d failed\n", spreads, abolishes, sChecks, sFails);
    return sFails != 0;
}
