// Moemon Kai: Shadow Pokémon (docs/SHADOW_POKEMON.md), Colosseum 1:1: its 48 Shadow Pokémon, the heart gauge and its
// sources at the game's own rates (src/data/orre_shadow.h, from tools/orre/shadow_data.py). Their state is kept in
// "<save>.orre" (src/orre/orre_shadow.c): saves never change layout.
#ifndef GUARD_ORRE_SHADOW_H
#define GUARD_ORRE_SHADOW_H

// a Shadow Pokémon of Colosseum's table (common_rel's: tools/orre/shadow_data.py has the layout)
struct OrreShadowMon
{
    u8 id;              // Colosseum's Shadow id (the table's row: 1-96)
    u16 species;
    u8 catchRate;       // the snag's catch rate
    u8 gauge;           // the heart gauge's size (DP)
    u8 level;
    u16 trainers[2];    // Colosseum's trainers that hold it: the first meeting, and a second chance (0 none)
    u8 nature;          // 255: chosen when met (as gender, IVs)
    u8 gender;
    u8 ability;
    s16 friendship;
    u16 item;
    u16 moves[4];       // its own moves (hidden while it is Shadow)
    u8 ivs[6];
    u16 evs[6];
    u16 flags[6];       // Colosseum's story flags of its states (met, snagged, purified...)
};

// a nature's part in the heart gauge (Colosseum's PokemonSeikakuData)
struct OrreShadowNature
{
    u8 rate[5];         // the rate (sOrreDpRate) for sources 0-4
    u8 hyperMode[7];    // (not "hyper": a Windows macro) Hyper Mode's chance (%) by the gauge's level
    u8 call[7];         // CALL's chance (%) by the gauge's level
    u8 disobey[6];      // Hyper Mode's ways of ignoring orders (%): its own move, the opposing trainer, its own trainer,
                        // itself, too confused, its item
};

// the heart gauge's sources (Colosseum's DP filters)
enum
{
    ORRE_DP_BATTLE,     // sent out in battle
    ORRE_DP_WALK,       // every 256 steps in the party
    ORRE_DP_CALL,       // CALLed in battle
    ORRE_DP_DAYCARE,    // every 256 steps at the day care (and time in the Garden: docs/SHADOW_POKEMON.md decision 7)
    ORRE_DP_SCENT,      // a scent (times its strength)
    ORRE_DP_EMPTY,      // the gauge emptied
};

// the gauge's level (Colosseum's pokemonGetDarkPokemonLevel): 0 full, 1-5 its bars opening, 6 open (ready to be
// purified), 7 not a Shadow Pokémon
#define ORRE_SHADOW_OPEN 6
#define ORRE_SHADOW_NONE 7

void Orre_Shadow_Reset(void);           // a new game
void Orre_Shadow_LoadFromSave(void);    // the save just loaded: its "<save>.orre"
void Orre_Shadow_WriteSidecar(void);    // the game just saved

const struct OrreShadowMon *Orre_Shadow_Data(u8 id);   // Colosseum's row (NULL: none)
u8 Orre_Shadow_Count(void);                             // the rows (48)
const struct OrreShadowMon *Orre_Shadow_DataAt(u8 index);

bool8 Orre_Shadow_Is(struct Pokemon *mon);
bool8 Orre_Shadow_IsKey(u32 otId, u32 personality);
// its curated dark palette (src/data/orre_shadow_palettes.h) over PAL's 16 colours (the transparent one kept), for the
// MOéMON or the POKéMON pictures; FALSE (PAL as it was) if the species has none
bool8 Orre_Shadow_Recolor(u16 *pal, u16 species, bool8 pokemonArt);
bool8 Orre_Shadow_IsBox(struct BoxPokemon *box);
u8 Orre_Shadow_Id(struct Pokemon *mon);                 // its Colosseum id, 0 if not Shadow
// turns MON into Colosseum's Shadow Pokémon ID (snagged): its heart gauge closed, at the table's size
bool8 Orre_Shadow_Make(struct Pokemon *mon, u8 id);
u8 Orre_Shadow_Level(struct Pokemon *mon);              // the gauge's level (above)
s32 Orre_Shadow_Dp(struct Pokemon *mon);                // the gauge in hundredths of DP (-1 not Shadow)
u8 Orre_Shadow_Gauge(struct Pokemon *mon);              // its size in DP
void Orre_Shadow_Drop(struct Pokemon *mon, u8 source, u8 strength);   // a source opens the gauge (Colosseum's rule)
u8 Orre_Shadow_HyperChance(struct Pokemon *mon);        // % (0 not Shadow)
u8 Orre_Shadow_CallChance(struct Pokemon *mon);
void Orre_Shadow_Purified(struct Pokemon *mon);         // no longer Shadow (its record goes)
void Orre_Shadow_Step(void);            // a step taken: the party's (and the day care's) step counts

// ---- in battle (Colosseum's rules: pokemon_data.c pokemonBiosGetPokemonWazaPtr, fight_range_*.c)
// A Shadow Pokémon's moves in battle: slot 0 is always SHADOW RUSH; slot 1 is "????" until the gauge's level 2, slot 2
// until 4, slot 3 until 5 (its own moves stay underneath, never written over by the battle)
bool8 Orre_Shadow_SlotHidden(struct Pokemon *mon, u8 slot);
bool8 Orre_Shadow_SlotOverlaid(struct Pokemon *mon, u8 slot);   // SHADOW RUSH's or a hidden one
void Orre_Shadow_BattleMoves(struct Pokemon *mon, u16 *moves, u8 *pp);
void Orre_Shadow_BattleStart(void);                 // a battle begins (each Shadow Pokémon's drop: once a battle)
void Orre_Shadow_SentOut(struct Pokemon *mon);      // sent out: the battling drop, the first time this battle
// EXP a Shadow Pokémon earned: none while its gauge is at level 0-2, stored (given back when purified) from level 3;
// TRUE when MON is Shadow (it takes no EXP of its own)
bool8 Orre_Shadow_TakeExp(struct Pokemon *mon, u32 exp);
u32 Orre_Shadow_StoredExp(struct Pokemon *mon);
// HYPER MODE (fight_range_*.c: a SHADOW RUSH may bring it on, by the nature's chance at the gauge's level; while in it
// the Pokémon ignores orders but for SHADOW RUSH, by its nature's chances, and SHADOW RUSH lands a critical hit 9 times
// in 10; it lasts past the battle until CALL brings it round or it faints) and CALL (in RUN's place in ORRE's trainer
// battles)
bool8 Orre_Shadow_InHyper(u8 battler);
u8 Orre_Shadow_BattleCommand(u8 mode);          // the battle script's ORRE modes (constants/battle_gen4.h ORRE_*)
u8 Orre_Shadow_HyperDisobedience(void);         // 0xFF: none (IsMonDisobedient's own rules then); else its result
bool8 Orre_Shadow_HyperCrit(void);              // SHADOW RUSH in HYPER MODE: a critical hit this time
bool8 Orre_Shadow_CallAvailable(void);          // the fourth command is CALL (ORRE's trainer battles)
void Orre_Shadow_Call(u8 battler);              // CALL, by BATTLER's trainer: its outcome chosen and done
const u8 *Orre_Shadow_BattleText(u8 line);      // STRINGID_ORRE_TEXT / _TEXT2's words
extern const u8 gText_Orre_BattleMenuCall[];    // the action menu with CALL
extern const u8 gText_Orre_HiddenMove[];   // "????": a hidden move's name (Colosseum's)
void Orre_Shadow_Debug(const char *arg);   // `mp shadow ...` (tests)
// the end of BATTLER's turn (battle_gen4.c's end-turn steps): HYPER MODE's reminder, or coming to its senses on its own
// (1 in 256); the script to run, NULL none
const u8 *Orre_Shadow_EndTurn(u8 battler);

// ---- the Shadow aura (src/orre/orre_aura.c): drawn over the frame around a Shadow Pokémon in battle, faint while
// it's out, a burst when Colosseum plays its aura
enum { ORRE_AURA_APPEAR, ORRE_AURA_HYPER, ORRE_AURA_PULSE };
void Orre_Aura_Burst(u8 battler, u8 kind);
void Orre_Aura_Composite(u16 *frame);   // sdl2.c VDraw

#endif
