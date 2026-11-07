// Moemon Kai: ORRE's screens and story state (docs/ORRE_DECISIONS.md), apart from its 3D engine (o3d.h)
#ifndef GUARD_ORRE_H
#define GUARD_ORRE_H

// Colosseum's story flags (std081_flagCheckTrue's numbers), as far as ORRE has them (src/orre/orre_story.c)
bool8 Orre_Flag(u16 flag);
void Orre_SetFlag(u16 flag, bool8 on);
// Kai's own flags past Colosseum's (its scripts' numbers stay below 0xF00)
#define ORRE_FLAG_SNAG_MACHINE 0xFFF   // the SNAG MACHINE (Colosseum's hero has it from the start: Kai's story gives it)

// Colosseum's world map screen (src/orre/worldmap.c): a 480x320 picture of its own like the BADGE CASE's
bool8 Orre_WorldMap_IsActive(void);
bool8 Orre_WorldMap_Step(u16 keys);
// (specials) Orre_WorldMap (waitstate): the screen; Orre_WorldMapTravel: off to the place chosen (VAR_RESULT TRUE)
void Orre_WorldMap(void);
void Orre_WorldMapTravel(void);
bool8 Orre_MapStart(u16 map, s16 *x, s16 *y);   // a map's start (Colosseum's entry 0)

// The boat to ORRE (src/orre/orre_ferry.c; docs/ORRE_DECISIONS.md "A post-game place"): ORRE is reached by boat with
// the ORRE TICKET, given after any region's ELITE FOUR, from HOENN's ferries and every region's port, landing at the
// OUTSKIRT STAND, where the boat's sailor waits to sail back to the port the player came from
void Orre_Sail(void);        // (special, waitstate) the voyage to ORRE
void Orre_SailBack(void);    // (special, waitstate) ... and back
void Orre_ShadowAboard(void);   // (special) VAR_RESULT TRUE: a Shadow Pokémon in the party (it can't leave ORRE)

#endif
