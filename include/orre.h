// Moemon Kai: ORRE's screens and story state (docs/ORRE_DECISIONS.md), apart from its 3D engine (o3d.h)
#ifndef GUARD_ORRE_H
#define GUARD_ORRE_H

// Colosseum's story flags (std081_flagCheckTrue's numbers) kept as Colosseum keeps them: values of 1-32 bits in three
// levels, from common_rel's table (src/orre/orre_story.c); a number past the table is Kai's own
#define ORRE_KAI_FLAG_FIRST 0xF00
#define ORRE_FLAG_LEVEL_WORDS 256
bool8 Orre_Flag(u16 flag);                       // set (any value but 0)
void Orre_SetFlag(u16 flag, bool8 on);           // 1 or 0 (std078 / std079)
u32 Orre_FlagValue(u16 flag);                    // std082_flagGetValue
void Orre_SetFlagValue(u16 flag, u32 value);     // _flagSet (the low bits that fit)
u16 Orre_FlagCount(void);                        // Colosseum's flags (0: no pack)
u8 Orre_FlagWidth(u16 flag);
void Orre_StoryJump(u16 flag);                   // Colosseum's story jump to FLAG (tests, debug)
u8 *Orre_StoryState(u32 *size);                  // the values, for "<save>.orre" (orre_shadow.c)
void Orre_StoryReset(void);
void Orre_StoryFromOldBits(const u8 *bits, u32 size);   // a format 1 sidecar's flags
// Kai's own flags (ORRE_KAI_FLAG_FIRST on; Colosseum's numbers stay below)
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
void Orre_ShadowAboard(void);
void Orre_CheckBoat(void);      // (special, waitstate) ORRE's packs here? (VAR_RESULT: orre_ferry.c)
void Orre_DownloadPacks(void);  // (special, waitstate) ... their download, with its progress   // (special) VAR_RESULT TRUE: a Shadow Pokémon in the party (it can't leave ORRE)

// Colosseum's words (src/orre/orre_text.c: orre/orre_text.pak from tools/orre/build_text.py): a line by Colosseum's own
// message id, through PLACEHOLDER_ID_ORRE_TEXT; the words it fills in are ORRE's string slots, one per Colosseum
// control id (tools/orre/msg.py CONTROLS: 89 npc, 45 item, 50 pokemon, 47 digit, 75 money...)
#define ORRE_STR_VARS 0x60
#define ORRE_STR_VAR_SIZE 48
u32 Orre_TextCount(void);
bool8 Orre_HasText(u32 id);
const u8 *Orre_Text(u32 id);
void Orre_TextPlaceholder(u8 *out, u32 id);   // OUT: 6 bytes
void Orre_SetStrVar(u8 ctl, const u8 *str);
const u8 *Orre_StrVar(u8 ctl);
void Orre_ClearStrVars(void);
const u8 *Orre_ExpandPlaceholder(u8 id, const u8 *src, u8 **dest);   // (string_util.c)

#endif
