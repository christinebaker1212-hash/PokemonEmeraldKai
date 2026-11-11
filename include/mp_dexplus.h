#ifndef GUARD_MP_DEXPLUS_H
#define GUARD_MP_DEXPLUS_H

// Moemon Kai: the POKéDEX's own side and its list's view (src/mp/mp_dexplus.c)

// What the list shows (the SHOW filter)
enum { MP_DEXSHOW_ALL, MP_DEXSHOW_NOT_SEEN, MP_DEXSHOW_NOT_CAUGHT, MP_DEXSHOW_CAUGHT, MP_DEXSHOW_FRIENDS, MP_DEXSHOW_COUNT };

void MP_DexOwn_Reset(void);
void MP_DexOwn_Mark(u16 nat, bool8 caught);
bool8 MP_DexOwn_Has(u16 nat, bool8 caught);

bool8 MP_DexView_Shared(void);   // SHARED (the group's POKéDEX: the save's flags) or OWN
u8 MP_DexView_Filter(void);
u16 MP_DexView_Last(void);       // the entry the list was left on (a national number, 0 none)
void MP_DexView_Set(bool8 shared, u8 filter, u16 last);

// The list screen's header, drawn over the frame while pokedex.c shows it each frame
struct MpDexHud
{
    bool8 nationalOn, national, shared;
    u8 filter;
    u16 total, ownCaught, allCaught;
    bool8 nothing;    // FIND & SORT found nothing: the list is as it was
    bool8 narrowed;   // FIND & SORT narrows the list
};
void MP_DexHud_Show(const struct MpDexHud *hud);
void MP_DexHud_Composite(u16 *frame);

// FIND & SORT (SELECT on the list): the panel, what it asks of the list
enum { MP_DEXSORT_NUMBER, MP_DEXSORT_AZ, MP_DEXSORT_HEAVIEST, MP_DEXSORT_LIGHTEST, MP_DEXSORT_TALLEST, MP_DEXSORT_SMALLEST,
       MP_DEXSORT_HP, MP_DEXSORT_ATK, MP_DEXSORT_DEF, MP_DEXSORT_SPATK, MP_DEXSORT_SPDEF, MP_DEXSORT_SPEED, MP_DEXSORT_TOTAL,
       MP_DEXSORT_FIRST, MP_DEXSORT_COUNT };
struct MpDexQuery { u8 sort, gen, type1, type2; u16 ability, move; u8 egg; };
const struct MpDexQuery *MP_DexQuery(void);
bool8 MP_DexQuery_Active(void);
bool8 MP_DexQuery_Match(u16 nat, bool8 seen, bool8 owned);
s32 MP_DexQuery_SortKey(u16 nat);
void MP_DexPanel_Open(u8 order);
int MP_DexPanel_Step(void);
const char *MP_DexPanel_Jump(void);
// An entry's MORE (R on it): PAGE 0 its hint, 1 STATS, 2 EVOLUTION, 3 WHERE
void MP_DexInfo_Show(u16 nat, bool8 seen, bool8 owned, u8 page);

#endif
