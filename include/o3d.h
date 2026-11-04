// Moemon Kai: ORRE's 3D engine (src/orre/o3d.c; docs/ORRE_DECISIONS.md). Its own engine, cloned from SINNOH's
// (src/sinnoh/s3d.c) and kept apart from it: the two regions change on their own.
#ifndef GUARD_O3D_H
#define GUARD_O3D_H

struct MapLayout;

void O3D_Update(void);       // every overworld frame: is this an ORRE map, and from where is it seen
bool8 O3D_Active(void);      // this frame's field is drawn by ORRE's engine
bool8 O3D_IsOrreGroup(u8 mapGroup);
// A field sprite's place on ORRE's 3D ground: (x, feetY) on Emerald's screen -> where the view shows that spot, and its
// depth (0: not on an ORRE map); spriteId: whose (0xFFFF none)
void O3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth);
// Sprites stand in the scene as upright cards: sprite.c notes each one's depth and feet for its OAM entries, LoadOam
// commits them with the OAM, and the renderer hides a sprite pixel the map is in front of
extern float gO3dSpriteDepth[];
extern s16 gO3dSpriteFeet[];
void O3D_NoteOam(u16 spriteId, u16 first, u16 end);
void O3D_CommitOam(void);
// ORRE's tiles carry their behavior in their metatile id (O3D_METATILE_BEHAVIOR + behavior: src/orre/o3d.c)
#define O3D_METATILE_BEHAVIOR 0x300
// The blocks of a map's layout: an ORRE map's come from its pack (Colosseum's collision), others are the layout's
const u16 *O3D_LayoutBlocks(const struct MapLayout *layout);
float O3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a map pixel, NaN none

#endif
