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
extern const u8 gText_Orre_HiddenMove[];   // "????": a hidden move's name (Colosseum's)
void Orre_Shadow_Debug(const char *arg);   // `mp shadow ...` (tests)

#endif
