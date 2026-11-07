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

#endif
