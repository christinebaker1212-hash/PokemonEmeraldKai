// Moemon Kai: held HMs. A Pokemon holding an HM can use that HM's field move (CUT, SURF...) as if it
// knew it; in battle the HM does nothing. HMs are never used up, so a held HM must never leave the
// player: one held by a Pokemon that is released, traded or put in the shared PC goes back to the BAG,
// and a Pokemon that arrives holding one (an older game) arrives without it.
#include "global.h"
#include "mp.h"
#include "item.h"
#include "party_menu.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/moves.h"

u16 MP_HeldHmMove(struct Pokemon *mon)
{
    u16 item;
    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
        return MOVE_NONE;
    item = GetMonData(mon, MON_DATA_HELD_ITEM);
    return MP_IS_ITEM_HM(item) ? ItemIdToBattleMoveId(item) : MOVE_NONE;
}

bool8 MP_StripHeldHm(struct Pokemon *mon)
{
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM), none = ITEM_NONE;
    if (!MP_IS_ITEM_HM(item))
        return FALSE;
    SetMonData(mon, MON_DATA_HELD_ITEM, &none);
    return TRUE;
}

bool8 MP_ReturnHeldHm(struct Pokemon *mon)
{
    u16 item = GetMonData(mon, MON_DATA_HELD_ITEM);
    if (!MP_IS_ITEM_HM(item))
        return FALSE;
    AddBagItem(item, 1);
    MPNet_Log("held HM %d back to the bag", item);
    return MP_StripHeldHm(mon);
}

bool8 MP_ItemCanBeHeld(u16 item)
{
    return !GetItemImportance(item) || MP_IS_ITEM_HM(item);
}
