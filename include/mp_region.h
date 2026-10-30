#ifndef GUARD_MP_REGION_H
#define GUARD_MP_REGION_H
// Moemon Kai: HOENN, JOHTO and KANTO (src/mp/mp_region.c, docs/REGIONS_PLAN.md)

#define REGION_HOENN 0
#define REGION_JOHTO 1
#define REGION_KANTO 2
#define REGION_COUNT 3

// A Pokémon met in JOHTO: its MET_GAME (MON_DATA_MET_LOCATION holds the low byte of the JOHTO map section)
#define MP_METGAME_JOHTO 14

// Johto / Kanto flags and vars past the save block's (tools/quetzal_port/import.py places them)
#define REGION_FLAGS_START 0x1000
#define REGION_FLAGS_END   0x4000   // SPECIAL_FLAGS_START
#define REGION_FLAG_BYTES  ((REGION_FLAGS_END - REGION_FLAGS_START) / 8)
#define REGION_VARS_START  0x5000
#define REGION_VARS_COUNT  0x300

// Per-region progress, in the region side file
struct RegionState
{
    u8 cleared;      // bit per region: its Elite Four beaten (Hoenn: FLAG_SYS_GAME_CLEAR too)
    u8 started;      // bit per region: the player has been there
    u8 current;      // the region of the last trip (the boat's maps take it from here)
    u8 filler[61];   // room for later fields without a format change
};
extern struct RegionState gRegionState;

u8 MP_Region_OfGroup(u8 group);
u16 MP_Region_BadgeFlag(u8 region, u8 badge);
u8 MP_Region_BadgeCount(u8 region);
bool8 MP_Region_Cleared(u8 region);
void MP_Region_SetCleared(u8 region);
bool8 MP_Region_AnyCleared(void);   // an Elite Four beaten in any region (Gen 4 Pokémon appear)
u8 MP_Region_Current(void);          // the region of the map the player is on
u8 *MP_Region_FlagPointer(u16 id);   // NULL outside REGION_FLAGS_START..END
u16 *MP_Region_VarPointer(u16 id);
void MP_Region_NewGame(void);
void MP_Region_WriteSidecar(void);   // "<save>.region", after the save file is written
void MP_Region_LoadFromSave(void);
struct PokemonStorage;
struct BoxPokemon;
struct Pokemon;
struct PokemonStorage *MP_Region_GlobalPC(void);   // the GLOBAL PC's boxes (made on first use)
bool8 MP_Region_DepositGlobal(const struct BoxPokemon *mon);
u16 MP_Region_GlobalFree(void);
bool8 MP_Region_FitBoxMon(struct BoxPokemon *b, u8 region);   // its EXP for that region (Lv 5 the first time)
void MP_Region_FitMon(struct Pokemon *mon, u8 region);
void MP_Region_FitAll(void);                       // party + own boxes at the current region's levels
void MP_Region_Travel(u8 from, u8 to);             // PC boxes change places, the party takes the new levels
struct WildPokemonInfo;
struct Trainer;
void MP_Region_TagMet(struct BoxPokemon *b);      // met / hatched here: JOHTO's places get MP_METGAME_JOHTO
bool8 MP_Region_Gen4Open(void);                    // an Elite Four beaten in another region (or in HOENN)
const struct WildPokemonInfo *MP_Region_WildInfo(const struct WildPokemonInfo *info);   // the Gen 3 table before it
const struct Trainer *MP_Region_TrainerData(u16 trainerNum);                            // the Gen 3 team before it
#endif
