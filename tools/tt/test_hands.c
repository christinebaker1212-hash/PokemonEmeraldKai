// Every card can be won: over two months of days, every card turns up in some player's hand (a region's players at
// a strength that allows it, or a special player); every hand keeps the deck limits.
#include <stdio.h>
#include <string.h>
#include "tt/npc.h"

int main(void)
{
    static uint8_t seen[TT_CARD_MAX + 1];
    unsigned day, r, tier, s, id, missing = 0, bad = 0, h;
    char why[96];
    for (day = 0; day < 60; day++)
    {
        for (r = 0; r < TT_REGIONS; r++)
            for (tier = 0; tier < 4; tier++)
                for (h = 0; h < 3; h++)
                {
                    uint16_t hand[TT_HAND];
                    unsigned k;
                    Tt_RegionHand(r, h * 0x9e3779b9u + r, tier, day, hand);
                    if (!Tt_DeckValid(hand, why, sizeof(why)))
                        bad++, printf("region %u tier %u day %u: %s\n", r, tier, day, why);
                    for (k = 0; k < TT_HAND; k++)
                        seen[hand[k]] = 1;
                }
        for (s = 0; s < TT_SPECIAL_COUNT; s++)
        {
            uint16_t hand[TT_HAND];
            unsigned k;
            Tt_SpecialHand(s, day, hand);
            if (!Tt_DeckValid(hand, why, sizeof(why)))
                bad++, printf("special %u day %u: %s\n", s, day, why);
            for (k = 0; k < TT_HAND; k++)
                seen[hand[k]] = 1;
        }
    }
    for (id = 1; id <= TT_CARD_MAX; id++)
        if (Tt_Card(id) && !seen[id])
            missing++, printf("never in a hand: %s\n", Tt_Card(id)->name);
    printf("hands: %u cards never seen, %u bad hands\n", missing, bad);
    return missing || bad;
}
