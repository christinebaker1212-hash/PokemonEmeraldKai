#ifndef GUARD_MP_FARM_H
#define GUARD_MP_FARM_H

// Moemon Co-op: the MOéMON GARDEN's BERRY farm (src/mp/mp_farm.c). Two rows of soft soil by the landing: 4 plots
// open, 2 more with each badge. Each plot is a BERRY tree of the game's own (berry tree ids GARDEN_FARM_TREE_FIRST..,
// saved with the game, growing by Emerald's rules on the player's clock). Online, in a room someone else made, the
// plots show the room host's farm: everyone plants, waters and picks there, and the host's game decides who gets a
// plot or its BERRIES first. Garden Pokémon help by their nature (mp_garden_life.c GA_FARM).

struct BerryTree;

// berry.c
struct BerryTree *MP_Farm_Tree(u8 id);      // a plot of the host's farm (online, as a friend); NULL: the save's own tree
void MP_Farm_Watered(u8 id);                // the tree in front was just watered
void MP_Farm_Changed(u8 id);                // ... planted / picked
void MP_Farm_PickedCount(u8 id, u8 *berry, u8 *count);   // picking on a friend's farm: what its host handed over
// the BERRY tree script (specialvar: MP_FARM_* in constants/berry.h)
u16 MP_Farm_AskPlant(void);
u16 MP_Farm_AskPick(void);

void MP_Farm_OnMapLoad(void);               // mp_garden.c: the open plots' soil (before the objects appear)
void MP_Farm_FrameUpdate(void);             // every frame
void MP_Farm_OnRelay(u8 from, const u8 *data, u16 len);
u8 MP_Farm_Plots(void);                     // the open plots of the farm shown here
bool8 MP_Farm_IsFarmGround(s16 x, s16 y);   // map coords without MAP_OFFSET: the farm's rows and the ways to them
bool8 MP_Farm_IsFriends(void);              // showing the room host's farm
void MP_Farm_Debug(const char *args);

// Garden Pokémon's work (mp_garden_life.c)
enum { FARM_WORK_WATER, FARM_WORK_TEND, FARM_WORK_NIBBLE, FARM_WORK_COUNT };
s8 MP_Farm_FindWork(u8 work, s16 x, s16 y, u32 busy);   // the nearest plot needing it (map coords with MAP_OFFSET), -1 none
void MP_Farm_PlotPos(u8 plot, s16 *x, s16 *y);           // with MAP_OFFSET
bool8 MP_Farm_DoWork(u8 work, u8 plot, u16 dex);         // TRUE: it did it (the plot still needed it)

#define FARM_CARE_TENDED 0x07   // mpCare: the growth stage a Grass type tended (once a stage)
#define FARM_CARE_NIBBLED 0x08  // ... a Pokémon nibbled a BERRY of this crop

#endif // GUARD_MP_FARM_H
