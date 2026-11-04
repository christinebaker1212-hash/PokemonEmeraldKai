// Moemon Kai: flags and vars this game added to JOHTO / KANTO (free numbers in Quetzal's ranges, saved in the region
// side file like the rest of 0x1000-0x3FFF: mp_region.c). Plain #defines: the map scripts read this too
// (data/event_scripts_qz.s, data/event_scripts.s) as well as the C (include/mp_region.h).
#ifndef GUARD_CONSTANTS_KAI_REGIONS_H
#define GUARD_CONSTANTS_KAI_REGIONS_H

// Always set (mp_region.c on every load and new game): Quetzal's people and things this game took out of JOHTO /
// KANTO (docs/REGIONS_AUDIT.md) wear it as their object flag, so the maps' local ids stay as they are
#define FLAG_KAI_GONE                 0x1033

// JOHTO (0x1000-0x1FFF)
#define FLAG_JOHTO_GOT_KILN_CHARCOAL  0x1021   // Crystal's EVENT_GOT_CHARCOAL_IN_CHARCOAL_KILN (0x1030 is HM01 CUT's)
#define FLAG_JOHTO_RADIO_CARD          0x1064   // Crystal's ENGINE_RADIO_CARD: the POKéGEAR's RADIO (RADIO TOWER 1F's quiz)
#define FLAG_JOHTO_HIDE_KILN_FARFETCHD 0x102F   // the kiln's FARFETCH'D, home once HM01 is given (set / cleared on entering)

#endif // GUARD_CONSTANTS_KAI_REGIONS_H
