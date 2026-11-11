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

// UNOVA's words (src/unova/u3d_text.c: Black's own, unova/black_text.pak from tools/unova/build_text.py)
#define UNOVA_STR_VARS 32
extern u8 gUnovaStrVar[UNOVA_STR_VARS][64];          // Black's word slots (0-2 unused: gStringVar1-3 are those)
const u8 *U3D_Text(u16 bank, u16 msg);
u16 U3D_TextBanks(void);
const u8 *U3D_ExpandPlaceholder(u8 id, const u8 *src, u8 **dest);   // string_util.c StringExpandPlaceholders

#endif
