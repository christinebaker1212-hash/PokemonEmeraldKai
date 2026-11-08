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

// Colosseum's battles (src/orre/orre_battle.c): its battle, trainer and team tables from the pack, fought as Kai's
// trainer battles through TRAINER_ORRE_A / _B
struct Pokemon;
#define ORRE_BATTLE_WON  2   // (Colosseum's own: std103_battleGetResult, its scripts compare with 2)
#define ORRE_BATTLE_LOST 1
u16 Orre_BattleCount(void);
bool8 Orre_PrepareBattle(u16 battle);          // the battle's opponents set (gTrainerBattleOpponent_A / _B, flags)
bool8 Orre_IsTrainer(u16 trainerId);
const u8 *Orre_ClassName(u16 trainerId);       // (battle_message.c)
u8 Orre_Party(struct Pokemon *party, u16 trainerId, bool8 firstTrainer);   // (battle_main.c CreateNPCTrainerParty)
u32 Orre_Money(u16 trainerId);                 // (battle_script_commands.c)
bool8 Orre_BattleLossWhitesOut(void);
void Orre_BattleEnded(bool8 won);              // (battle_setup.c)
u8 Orre_BattleResult(void);                    // ORRE_BATTLE_WON / _LOST
void Orre_StartBattle(void);                   // (special) battle VAR_0x8004, its foe's picture VAR_0x8005's person
void Orre_BattleResultToVar(void);             // (special) VAR_RESULT = the last battle's result

// Colosseum's scripts (src/orre/orre_vm.c): its GS VM running the rooms' own script banks
void Orre_Vm_Frame(void);          // every overworld frame (O3D_Update)
void Orre_Vm_RoomEntered(void);
void Orre_Vm_Stop(void);
void Orre_Vm_Warped(void);         // (overworld.c LoadMapFromWarp)
void Orre_VmHold(void);            // (special, waitstate) Orre_EventScript_VmHold
void Orre_VmTalk(void);            // (special) Orre_EventScript_Talk: person VAR_0x8004's talk script
// The rooms' people (src/orre/orre_npcs.c): Colosseum's characters as object events, driven by its scripts
void Orre_Npcs_RoomEntered(u16 room);
void Orre_Npcs_Clear(void);
void Orre_Npcs_Place(void);
void Orre_Npc_SetDisp(u32 index, bool8 show);
void Orre_Npc_SetShown(u32 index, bool8 show);
void Orre_Npc_SetPosition(u32 group, u32 index, float x, float z);
void Orre_Npc_SetAngle(u32 group, u32 index, float degrees);
void Orre_Npc_FaceOther(u32 group, u32 index, u32 tgroup, u32 tindex);
void Orre_Npc_MoveTo(u32 group, u32 index, float x, float z);
void Orre_Npc_MoveToOther(u32 group, u32 index, u32 tgroup, u32 tindex);
bool8 Orre_Npc_Done(u32 group, u32 index);
u16 Orre_Npc_Name(u32 index);
const u8 *Orre_Check_InFront(void);                // (field_control_avatar.c) a check spot in front of the player
u32 Orre_Check_Script(u32 row, u32 *args);
bool8 Orre_Vm_Running(void);
void Orre_VmWarp(void);
void Orre_VmTouch(void);
void Orre_Touch_Frame(void);
u32 Orre_Touch_Script(u32 row, u32 *args);
void Orre_VmChoice(void);
void Orre_VmChoiceDone(void);
void Orre_Npc_TalkStart(u32 group, u32 index);
void Orre_Npc_WalkListReset(u32 index);
void Orre_Npc_WalkListAdd(u32 index, float x, float z);
void Orre_Npc_WalkListStart(u32 index, bool8 loop);
void Orre_Npcs_Frame(void);
void Orre_Npc_Surprise(u32 group, u32 index);
bool8 Orre_Npc_GetColo(u32 group, u32 index, float *x, float *z);
bool8 Orre_Npc_SeesHero(u32 index);
void Orre_Npc_TalkEnd(u32 group, u32 index);
const u8 *Orre_Npc_TalkScript(u8 localId);      // (field_control_avatar.c)
u32 Orre_Npc_TalkScriptId(u32 index);
u16 Orre_VmHolding(void);          // (specialvar) a scene still holds the player
void Orre_VmBattleDone(void);      // (orre_battle.c) its battle is over
bool8 Orre_Vm_Debug(u16 bank, u16 func);

#endif
