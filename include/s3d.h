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
// SINNOH's layout blocks: metatile ids from here carry a behavior (id - S3D_METATILE_BEHAVIOR; fieldmap.c
// GetMetatileAttributesById), tools/sinnoh/build_pack.py
#define S3D_METATILE_BEHAVIOR 0x300
float S3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a point (map pixels), NaN none           // a SINNOH map (its own music, battle songs...)
u16 S3D_MapMusic(u16 song);                      // a SINNOH map's song by night (Platinum's own night versions)
// The blocks of a map's layout: a SINNOH map's come from the pack (Platinum's collision), others are the layout's
const u16 *S3D_LayoutBlocks(const struct MapLayout *layout);
// Props placed at run time (SINNOH's gyms that move, src/sinnoh/s3d_gyms.c): Platinum's model by its number, in the
// map's matrix world (16 units a tile; S3D_MapOrigin: the map's corner there), turned about the up axis in degrees
#define S3D_DYN_PROPS 40
bool8 S3D_DynProp(u8 slot, u16 model, float x, float y, float z, float turn);
bool8 S3D_DynPropTurned(u8 slot, u16 model, float x, float y, float z, float tilt, float turn);
void S3D_DynPropHide(u8 slot);
void S3D_DynPropsClear(void);
#define S3D_DARK_LIGHTS 40
void S3D_SetDarkness(float level, const s16 (*lights)[3], u8 n);   // a dark room (0: none) lit around {x, y, radius}
bool8 S3D_MapOrigin(float *x, float *z);
// A gym's own ground heights (map tiles; NaN: the block's), asked with the height the object is nearest
extern float (*gS3dHeightHook)(s32 mapX, s32 mapY, float h);
extern float (*gS3dObjectHeightHook)(u8 localId, float h);   // a gym's people's heights by local id (NaN: the ground's)
void S3D_PlayerWorld(float *x, float *z);       // the player in the map's matrix world
void Sinnoh_StoryRefresh(void);                   // shows / hides SINNOH's story people by the story (s3d_legends.c)
void S3D_Gyms_MapLoaded(void);                    // a map's grid is (re)built: a gym's state on it (s3d_gyms.c)
void S3D_Gyms_Update(void);                       // every overworld frame
bool8 S3D_Gyms_SeesPlayer(u8 localId);            // a gym's trainer can see the player (same floor)
bool8 S3D_Gyms_JumpAt(s16 x, s16 y, u8 direction);   // a step there jumps (ETERNA's hour hand; map coords + 7)
bool8 S3D_Gyms_PushAt(s16 x, s16 y, u8 direction);   // walking into it hits it (VEILSTONE's bags; map coords + 7)
void S3D_Gyms_RedirectWarp(s16 x, s16 y);       // a warp from there (map coords + 7): a gym's wrong door
void S3D_Gyms_BreakAt(s16 x, s16 y);              // something runs into it fast (SNOWPOINT's snowballs)
s8 S3D_Gyms_IceSlope(s16 x, s16 y, u8 direction);   // SNOWPOINT's ice: +1 a slide goes up a slope there, -1 down
bool8 S3D_Gyms_StepBlocked(s16 fromX, s16 fromY, s16 toX, s16 toY);   // a gym's own rules for a step (map coords + 7)
float S3D_PlayerHeight(void);                    // the ground height the player stands at (NaN: unknown)
float S3D_TileFloor(s32 mapX, s32 mapY, float near);   // the block's ground height on a map tile nearest NEAR (NaN: none)

// The briefcase's starter screen drawn ahead of time (tools/sinnoh/build_starter.py, section STRT of the graphics pack):
// w x h RGB555 pictures (bit 15: drawn), the opening's frames then the camera's; each ball's bounce as patches
struct S3dStarterGfx
{
    u16 w, h, openFrames, camFrames, bounceFrames;
    const u16 *frames;
    struct { u16 x, y, w, h; const u16 *px; } ball[3];
    const u16 *cursor;   // 32 x 32 RGB555 (bit 15: drawn): Platinum's glove
    s16 cursorPos[3][2]; // its middle over each ball
};
const struct S3dStarterGfx *S3D_Gfx_Starter(void);   // NULL: the pack has none

#endif
