// Moemon Kai: UNOVA's 3D engine (src/unova/u3d.c; docs/UNOVA_HANDOFF.md). Its own engine, never SINNOH's or ORRE's:
// where the three hook into the game each has its own line.
#ifndef GUARD_U3D_H
#define GUARD_U3D_H

struct MapLayout;
struct Sprite;

void U3D_Update(void);                           // every overworld frame: is this map drawn in 3D, and from where
bool8 U3D_Active(void);                          // this frame's field is drawn by UNOVA's engine
bool8 U3D_IsUnovaGroup(u8 mapGroup);
// A field sprite's place on the 3D ground: (x, feetY) on Emerald's screen -> where the view shows that spot at the
// ground's height there, and its depth (0: not on a 3D map). spriteId: whose (each keeps the floor it's on)
void U3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth);
extern float gU3dSpriteDepth[];
extern s16 gU3dSpriteFeet[];
void U3D_NoteOam(u16 spriteId, u16 first, u16 end);
void U3D_CommitOam(void);
// UNOVA's layout blocks: metatile ids from here carry a behavior (id - U3D_METATILE_BEHAVIOR; fieldmap.c
// GetMetatileAttributesById), tools/unova/build_pack.py
#define U3D_METATILE_BEHAVIOR 0x300
const u16 *U3D_LayoutBlocks(const struct MapLayout *layout);   // a UNOVA map's from the pack, others the layout's
float U3D_TestGround(s32 mapX, s32 mapY, float near);         // tests: the ground's height (map pixels), NaN none
void U3D_ShopPicture(bool8 on, s16 dx, s16 dy);               // shop.c's BUY screen: the field's last picture behind
u8 U3D_Season(void);                                          // Black's season now: 0 spring .. 3 winter
u16 U3D_CurrentZone(void);                                    // the Black zone the player is on (0xFFFF: not UNOVA)
s32 U3D_MapIndex(void);
bool8 U3D_MapOfZoneTile(u16 zone, u16 x, u16 z, u8 *group, u8 *num, s16 *mapX, s16 *mapY);                                       // the player's map's row of u3d_maps.h (-1: not UNOVA)
u8 U3D_MapCellX(s32 index);                                   // ... where it lies on its Black matrix (blocks)
u8 U3D_MapCellZ(s32 index);
// UNOVA's scripts (src/unova/u3d_vm.c: Black's own bytecode)
#define U3D_VAR_BASE 0xB000                                   // Emerald vars 0xB000 + n are Black's works 0x4000 + n
#define U3D_VAR_COUNT 0x140
u16 *U3D_Vm_VarPointer(u16 id);
bool8 U3D_Vm_ObjectHidden(u8 localId);                        // a person hidden by Black's flag
bool8 U3D_Vm_SceneStart(void);                                // script.c TryRunOnFrameMapScript: a scene starts
bool8 U3D_VmRunning(void);
u8 *U3D_Vm_BadgeFlags(void);                                  // UNOVA's 8 badges, FLAG_UNOVA_BADGE01 + n
void U3D_Vm_ResetWork(void);
const void *U3D_Vm_WorkData(u32 *size);                       // Black's event work, saved (mp_region.c chunk UNEW)
void U3D_Vm_WorkLoad(const void *data, u32 size);

// UNOVA's words (src/unova/u3d_text.c: Black's own, unova/black_text.pak from tools/unova/build_text.py)
#define UNOVA_STR_VARS 32
extern u8 gUnovaStrVar[UNOVA_STR_VARS][64];          // Black's word slots, all its own
const u8 *U3D_Text(u16 bank, u16 msg);
u16 U3D_TextBanks(void);
const u8 *U3D_ExpandPlaceholder(u8 id, const u8 *src, u8 **dest);   // string_util.c StringExpandPlaceholders

// UNOVA's people (src/unova/u3d_gfx.c: Black's own sprites, unova/black_gfx.pak from tools/unova/build_gfx.py)
bool8 U3D_Gfx_Sprite(u16 code, bool8 small, u8 *kind, const u16 **pal, const u8 **frames);

// UNOVA's battles (src/unova/u3d_battle.c: Black's trainers through TRAINER_UNOVA_A / _B)
struct Pokemon;
bool8 U3D_IsTrainer(u16 trainerId);
const u8 *U3D_ClassName(u16 trainerId);
u8 U3D_Party(struct Pokemon *party, u16 trainerId, bool8 firstTrainer);
u32 U3D_Money(u16 trainerId);
bool8 U3D_PrepareBattle(u16 first, u16 second);
void U3D_BattleEnded(bool8 won);
u8 U3D_BattleResult(void);
u16 U3D_BattleSong(void);               // pokemon.c GetBattleBGM on an UNOVA map: Black's own
u16 U3D_VictorySong(u16 song);          // ... MP_RegionVictorySong

// UNOVA's wild POKéMON (src/unova/u3d_wild.c: Black's encounter tables)
struct WildPokemonInfo;
bool8 U3D_WildInfo(u16 behavior, const struct WildPokemonInfo **info, u8 *area);

#endif
