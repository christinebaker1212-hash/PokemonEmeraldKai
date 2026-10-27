// Moemon Co-op: smart targeting. When an item is used from the BAG, the party screen's cursor
// starts on the Pokemon it's for (only the cursor: nothing is used until A). A status cure goes
// to a Pokemon with a status it cures, in battle the one choosing first, then the other active
// ones, then the rest in party order; a REVIVE to the Pokemon that fainted last.
#include "global.h"
#include <stdio.h>
#include "battle.h"
#include "battle_anim.h"
#include "item.h"
#include "util.h"
#include "mp.h"
#include "pokemon.h"
#include "constants/battle.h"
#include "constants/item_effects.h"
#include "constants/items.h"

// The faints seen this session, newest stamp last (not saved: after a reload the first
// fainted Pokemon is picked)
static EWRAM_DATA u32 sFaintPersonality[PARTY_SIZE] = {0};
static EWRAM_DATA u32 sFaintStamp[PARTY_SIZE] = {0};
static EWRAM_DATA u32 sFaintClock = 0;

void MP_NoteFaint(u32 personality)
{
    u8 i, slot = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (sFaintPersonality[i] == personality)
        {
            slot = i;
            break;
        }
        if (sFaintStamp[i] < sFaintStamp[slot])
            slot = i;
    }
    sFaintPersonality[slot] = personality;
    sFaintStamp[slot] = ++sFaintClock;
}

static u32 FaintStamp(u32 personality)
{
    u8 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (sFaintStamp[i] && sFaintPersonality[i] == personality)
            return sFaintStamp[i];
    return 0;
}

static const u8 *ItemEffect(u16 item)
{
    if (!ITEM_HAS_EFFECT(item) || item == ITEM_ENIGMA_BERRY)
        return NULL;
    return gItemEffectTable[item - ITEM_POTION];
}

// The player's battler showing this party Pokemon, or MAX_BATTLERS_COUNT
static u8 ActiveBattler(struct Pokemon *mon)
{
    u8 b;
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY);
    if (!gMain.inBattle)
        return MAX_BATTLERS_COUNT;
    for (b = 0; b < gBattlersCount; b++)
        if (GetBattlerSide(b) == B_SIDE_PLAYER && !(gAbsentBattlerFlags & gBitTable[b])
         && gBattleMons[b].personality == personality && gBattleMons[b].hp)
            return b;
    return MAX_BATTLERS_COUNT;
}

// Does the status cure in effect[3] fix this Pokemon?
static bool8 Cures(struct Pokemon *mon, u8 cure)
{
    u8 b = ActiveBattler(mon);
    u32 status1 = b < MAX_BATTLERS_COUNT ? gBattleMons[b].status1 : GetMonData(mon, MON_DATA_STATUS);
    u32 status2 = b < MAX_BATTLERS_COUNT ? gBattleMons[b].status2 : 0;
    if (GetMonData(mon, MON_DATA_HP) == 0)
        return FALSE;
    return ((cure & ITEM3_SLEEP) && (status1 & STATUS1_SLEEP))
        || ((cure & ITEM3_POISON) && (status1 & STATUS1_PSN_ANY))
        || ((cure & ITEM3_BURN) && (status1 & STATUS1_BURN))
        || ((cure & ITEM3_FREEZE) && (status1 & STATUS1_FREEZE))
        || ((cure & ITEM3_PARALYSIS) && (status1 & STATUS1_PARALYSIS))
        || ((cure & ITEM3_CONFUSION) && (status2 & STATUS2_CONFUSION));
}

s8 MP_SmartItemSlot(u16 item)
{
    const u8 *effect = ItemEffect(item);
    s8 best = -1;
    u8 i, rank, bestRank = 0;
    u32 stamp, bestStamp = 0;
    if (effect == NULL)
        return -1;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (effect[4] & ITEM4_REVIVE)
        {
            // the one that fainted last (one never seen fainting: the first in the party)
            if (GetMonData(mon, MON_DATA_HP) != 0)
                continue;
            stamp = FaintStamp(GetMonData(mon, MON_DATA_PERSONALITY));
            if (best < 0 || stamp > bestStamp)
                best = i, bestStamp = stamp;
        }
        else if ((effect[3] & ITEM3_STATUS_ALL) && Cures(mon, effect[3]))
        {
            // 3 the one choosing in battle, 2 another active one, 1 the rest
            u8 b = ActiveBattler(mon);
            rank = b == MAX_BATTLERS_COUNT ? 1 : b == gBattlerInMenuId ? 3 : 2;
            if (rank > bestRank)
                best = i, bestRank = rank;
        }
    }
    printf("[mp] smart cursor: item %u -> slot %d\n", item, best);
    return best;
}
