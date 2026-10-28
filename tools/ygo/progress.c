/* How many Yu-Gi-Oh! cards can be played (have their rules: Ygo_CardReady): in Stairway to the Destined Duel, per
 * booster set, and the ones still to write. tools/ygo/progress.sh builds and runs it; `todo` lists them by set. */
#include <stdio.h>
#include <string.h>
#include "ygo/collection.h"
#include "ygo/ygo.h"

static const char *Frame(unsigned f)
{
    static const char *const names[] = {"Normal", "Effect", "Fusion", "Ritual", "Spell", "Trap", "Token"};
    return f < 7 ? names[f] : "?";
}
int main(int argc, char **argv)
{
    unsigned id, s, sdd = 0, sddReady = 0, pack = 0, packReady = 0, todo[8] = {0};
    int list = argc > 1 && !strcmp(argv[1], "todo");
    for (id = 1; id <= YGO_CARD_COUNT; id++)
    {
        const struct YgoCard *c = Ygo_Card(id);
        int ready = Ygo_CardReady(id) != 0;
        if (c->frame == YGO_TOKEN)
            continue;
        if (c->sdd)
        {
            sdd++;
            sddReady += ready;
            if (!ready && c->frame < 8)
                todo[c->frame]++;
        }
        else
            pack++, packReady += ready;
    }
    printf("Stairway to the Destined Duel: %u cards, %u playable, %u to write (Effect %u, Fusion %u, Ritual %u, "
           "Spell %u, Trap %u)\n", sdd, sddReady, sdd - sddReady, todo[YGO_EFFECT], todo[YGO_FUSION], todo[YGO_RITUAL],
           todo[YGO_SPELL], todo[YGO_TRAP]);
    printf("2003 booster cards Stairway lacks: %u, %u playable, %u to write\n", pack, packReady, pack - packReady);
    for (s = 0; s < YGO_SET_COUNT; s++)
    {
        unsigned n = 0, r = 0;
        for (id = 1; id <= YGO_CARD_COUNT; id++)
            if (Ygo_Card(id)->sets & (1u << s))
                n++, r += Ygo_CardReady(id) != 0;
        printf("  %-4s %-36s %4u cards, %4u playable%s\n", gYgoSets[s].code, gYgoSets[s].name, n, r,
               gYgoSets[s].starter ? "" : Ygo_SetAvailable(s) ? "  (in the shops)" : "  (not in the shops yet)");
    }
    if (list)
        for (s = 0; s <= YGO_SET_COUNT; s++)
        {
            printf("%s:\n", s < YGO_SET_COUNT ? gYgoSets[s].code : "Stairway only (no 2003 pack)");
            for (id = 1; id <= YGO_CARD_COUNT; id++)
            {
                const struct YgoCard *c = Ygo_Card(id);
                int here = s < YGO_SET_COUNT ? (c->sets & (1u << s)) != 0 : c->sets == 0;
                if (here && c->frame != YGO_TOKEN && !Ygo_CardReady(id))
                    printf("  %4u %-6s %s%s\n", id, Frame(c->frame), c->name, c->sdd ? "" : "  (not in Stairway)");
            }
        }
    return 0;
}
