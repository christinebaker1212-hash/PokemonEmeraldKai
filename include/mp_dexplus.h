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
};
void MP_DexHud_Show(const struct MpDexHud *hud);
void MP_DexHud_Composite(u16 *frame);

#endif
