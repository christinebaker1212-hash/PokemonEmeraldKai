// Moemon Kai: flags and vars this game added to JOHTO / KANTO (free numbers in Quetzal's ranges, saved in the region
// side file like the rest of 0x1000-0x3FFF: mp_region.c). Plain #defines: the map scripts read this too
// (data/event_scripts_qz.s, data/event_scripts.s) as well as the C (include/mp_region.h).
#ifndef GUARD_CONSTANTS_KAI_REGIONS_H
#define GUARD_CONSTANTS_KAI_REGIONS_H

// Always set (mp_region.c on every load and new game): Quetzal's people and things this game took out of JOHTO /
// KANTO (docs/REGIONS_AUDIT.md) wear it as their object flag, so the maps' local ids stay as they are
#define FLAG_KAI_GONE                 0x1033

// Daily (cleared each day with Emerald's daily flags)
#define FLAG_DAILY_JOHTO_PLATEAU_RIVAL  0x923   // Crystal's ENGINE_INDIGO_PLATEAU_RIVAL_FIGHT (Emerald's FLAG_UNUSED_0x923)
#define FLAG_DAILY_JOHTO_KURT_GS_BALL   0x924   // Crystal's ENGINE_KURT_MAKING_BALLS for the GS BALL: KURT checks it until the next day
#define FLAG_DAILY_JOHTO_SHUCKIE        0x925   // Crystal's ENGINE_GOT_SHUCKIE_TODAY: MANIA asks SHUCKIE back from the next day

// KANTO vars
#define VAR_KANTO_RESORT_STEPS        0x514B   // FireRed's VAR_RESORT_GOREGEOUS_STEP_COUNTER (SELPHY's wish lasts 250 steps)
#define VAR_KANTO_RIVAL_NAME          0x514C   // ..0x514F: the rival's name as the player gave it (8 bytes; none: BLUE)
#define VAR_KANTO_MAGIKARP_SIZE       0x5151   // ROUTE 12 FISHING HOUSE's MAGIKARP record (FireRed's VAR_MAGIKARP_SIZE_RECORD)
#define VAR_JOHTO_MAGIKARP_SIZE       0x5040   // LAKE OF RAGE's MAGIKARP record (Crystal's wBestMagikarpLength)
#define VAR_KANTO_HERACROSS_SIZE      0x5152   // SIX ISLAND's HERACROSS record (FireRed's VAR_HERACROSS_SIZE_RECORD)
#define VAR_KANTO_MASSAGE_STEPS       0x5150   // FireRed's VAR_MASSAGE_COOLDOWN_STEP_COUNTER: DAISY grooms again after 500 steps

// KANTO flags
#define FLAG_KANTO_HIDE_GYM_GIOVANNI  0x221D   // FireRed's FLAG_HIDE_VIRIDIAN_GIOVANNI: gone after his farewell in the GYM

// JOHTO (0x1000-0x1FFF)
#define FLAG_JOHTO_GOT_KILN_CHARCOAL  0x1021   // Crystal's EVENT_GOT_CHARCOAL_IN_CHARCOAL_KILN (0x1030 is HM01 CUT's)
#define FLAG_JOHTO_HIDE_DEN_RIVAL      0x1065   // the rival training in DRAGON'S DEN (Tuesdays, Thursdays: set / cleared on entering)
#define FLAG_JOHTO_RADIO_CARD          0x1064   // Crystal's ENGINE_RADIO_CARD: the POKéGEAR's RADIO (RADIO TOWER 1F's quiz)
#define FLAG_JOHTO_HIDE_KILN_FARFETCHD 0x102F   // the kiln's FARFETCH'D, home once HM01 is given (set / cleared on entering)
#define FLAG_JOHTO_GOT_GS_BALL         0x1066   // Crystal's EVENT_GOT_GS_BALL_FROM_GOLDENROD_POKEMON_CENTER
#define FLAG_JOHTO_HIDE_GS_RECEPTIONIST 0x1038  // GOLDENROD POKéMON CENTER's receptionist who brings it (set on entering)
#define FLAG_JOHTO_GAVE_GS_BALL        0x1039   // Crystal's EVENT_GAVE_GS_BALL_TO_KURT
#define FLAG_JOHTO_MAGIKARP_RECORD     0x104A   // a LAKE OF RAGE record was set (the first MAGIKARP shown always is one)
#define FLAG_JOHTO_ELIXER_STANDBY      0x104B   // Crystal's EVENT_LAKE_OF_RAGE_ELIXIR_ON_STANDBY
#define FLAG_JOHTO_GOT_ITEMFINDER      0x10A3   // Crystal's EVENT_GOT_ITEMFINDER (ECRUTEAK)
#define FLAG_JOHTO_GOT_SOFT_SAND       0x10B2   // Crystal's EVENT_GOT_SOFT_SAND_FROM_KATE (ROUTE 34)

#endif // GUARD_CONSTANTS_KAI_REGIONS_H
