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
    u8 originGroup, originNum, originX, originY;   // where the player stepped into a shared map from (MP_Region_IsSharedMap)
    u8 originValid;
    u8 filler[56];   // room for later fields without a format change
};
extern struct RegionState gRegionState;

u8 MP_Region_OfGroup(u8 group);
u16 MP_Region_BadgeFlag(u8 region, u8 badge);
u8 MP_Region_BadgeCount(u8 region);
bool8 MP_Region_Cleared(u8 region);
void MP_Region_SetCleared(u8 region);
bool8 MP_Region_AnyCleared(void);   // an Elite Four beaten in any region (the GLOBAL PC)
u8 MP_Region_StartChoice(void);       // PROF. BIRCH's region question, until the new game starts
u8 MP_Region_Current(void);          // the region of the map the player is on (a shared map: the region they came from)
bool8 MP_Region_IsSharedMap(u8 group, u8 num);   // the MOéMON GARDEN and the GAME CENTER: every region's
void MP_Region_NoteWarp(u8 destGroup, u8 destNum);   // overworld.c ApplyCurrentWarp: stepping into a shared map
void MP_Region_SharedExit(u8 destGroup, u8 destNum);
u16 MP_Region_GameCenterOtherFloor(void);   // special: the elevator's other floor in JOHTO / KANTO (2F / 3F only)
bool8 MP_Region_WarpClosed(u8 destGroup, u8 destNum);   // field_control_avatar.c: JOHTO's / KANTO's GAME CENTER has no ROOFTOP    // field_control_avatar.c SetupWarp: leaving one goes back where the player came from
u8 *MP_Region_FlagPointer(u16 id);   // NULL outside REGION_FLAGS_START..END
u16 *MP_Region_VarPointer(u16 id);
void MP_Region_NewGame(void);
void MP_Region_SetStartRegion(u8 region);   // a new game's region (PROF. BIRCH's question)
void MP_Region_StartNewGame(void);
void MP_Region_GoToStartHouse(u8 region);  // the warp to JOHTO's / KANTO's first house (NEW BARK, PALLET)          // end of NewGameInitData: JOHTO / KANTO start at home there
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
void MP_Region_OnMapLoaded(void);                  // entering another region on foot: as a trip
void MP_Region_Travel(u8 from, u8 to);             // PC boxes change places, the party takes the new levels
struct WildPokemonInfo;
struct Trainer;
void MP_Region_TagMet(struct BoxPokemon *b);      // met / hatched here: JOHTO's places get MP_METGAME_JOHTO
extern u8 gMpGenSetting;                           // tests: a GENERATION over the setting's (0 = none)
u8 MP_Gen_Cap(u8 region);                          // the last generation a region shows: HOENN 3, JOHTO / KANTO 2
u8 MP_Gen_OfSpecies(u16 species);
u8 MP_Gen_OfMove(u16 move);
u8 MP_Gen_OfAbility(u16 ability);
bool8 MP_Gen_MoveAllowed(u16 move);                // a move past the cap can't be picked in battle
u8 MP_Gen_SpeciesType(u16 species, u8 which);      // its types as the generation has them (FAIRY from Gen 6)
u8 MP_Gen_MoveType(u8 type);                       // FAIRY moves are NORMAL before Gen 6
void MP_Gen_FitBattler(u8 battler);
u8 MP_Gen_OfItem(u16 item);                        // 0 = always there (Kai's own, story items)
bool8 MP_Gen_ItemAllowed(u16 item);                // past the GENERATION: kept in the bag, can't be used or held
void MP_Gen_ReturnHeldItems(void);                 // held items past it go back to the bag
bool8 MP_Gen_SealedSpecies(u16 species);           // past the GENERATION: kept, but can't battle
bool8 MP_Gen_SealedMon(struct Pokemon *mon);       // species, a move or the ability past it (never an EGG)
extern struct Pokemon gPlayerParty[];
#define MP_SEALED(party, i) ((party) == gPlayerParty && MP_Gen_SealedMon(&(party)[i]))   // in "can it battle" checks                // a battler's types and ability as the generation has them
u16 MP_Gen_DexCount(void);                       // the NATIONAL DEX at the GENERATION: 386 at GEN 3 ... 1025 at 9
bool8 MP_Gen_Allowed(u16 species);                 // shown in the region the player is in
bool8 MP_Gen_ObjectHidden(u8 localId);             // a later generation's POKéMON on the map (JOHTO / KANTO)
u16 MP_Gen_StandIn(u16 species);                   // its nearest earlier form that is
const struct WildPokemonInfo *MP_Region_WildInfo(const struct WildPokemonInfo *info);   // the Gen 3 table before it
const struct Trainer *MP_Region_TrainerData(u16 trainerNum);                            // the Gen 3 team before it
#endif
