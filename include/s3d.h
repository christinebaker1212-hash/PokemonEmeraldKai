// Moemon Kai: SINNOH's 3D engine (src/sinnoh/s3d.c; docs/SINNOH_HANDOFF.md)
#ifndef GUARD_S3D_H
#define GUARD_S3D_H

struct MapLayout;

void S3D_Update(void);                           // every overworld frame: is this map drawn in 3D, and from where
bool8 S3D_Active(void);                          // this frame's field is drawn by the 3D engine
// A field sprite's place on the 3D ground: (x, feetY) on Emerald's screen -> where the 3D view shows that spot at the
// ground's height there (BDHC), and its depth (0: not on a 3D map). spriteId: whose (each keeps the bridge layer it's
// on); 0xFFFF none
void S3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth);
// Sprites stand in the 3D scene as upright cards: sprite.c notes each sprite's depth and feet for its OAM entries,
// LoadOam commits them with the OAM, and the renderer hides a sprite pixel the 3D map is in front of
extern float gS3dSpriteDepth[];
extern s16 gS3dSpriteFeet[];
void S3D_NoteOam(u16 spriteId, u16 first, u16 end);
void S3D_CommitOam(void);
bool8 S3D_IsSinnohGroup(u8 mapGroup);
float S3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a point (map pixels), NaN none           // a SINNOH map (its own music, battle songs...)
u16 S3D_MapMusic(u16 song);                      // a SINNOH map's song by night (Platinum's own night versions)
// SINNOH's text pack: a message of a bank (tools/sinnoh/build_text.py; "Message missing" without the pack)
const u8 *S3D_Text(u16 bank, u16 msg);
// The blocks of a map's layout: a SINNOH map's come from the pack (Platinum's collision), others are the layout's
const u16 *S3D_LayoutBlocks(const struct MapLayout *layout);

#endif
