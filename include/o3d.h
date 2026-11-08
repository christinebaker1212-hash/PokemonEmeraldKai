// Moemon Kai: ORRE's 3D engine (src/orre/o3d.c; docs/ORRE_DECISIONS.md). Its own engine, cloned from SINNOH's
// (src/sinnoh/s3d.c) and kept apart from it: the two regions change on their own.
#ifndef GUARD_O3D_H
#define GUARD_O3D_H

struct MapLayout;

void O3D_Update(void);       // every overworld frame: is this an ORRE map, and from where is it seen
bool8 O3D_Active(void);      // this frame's field is drawn by ORRE's engine
void O3D_ShopPicture(bool8 on, s16 dx, s16 dy);
// Colosseum's camera shots (src/orre/o3d_cam.c): the view looks through one while it plays
bool8 O3D_CamPlay(const char *name, float frame, bool8 loop);
bool8 O3D_CamPlayId(u32 id, float frame, bool8 loop);
void O3D_CamStop(void);
bool8 O3D_CamActive(void);
bool8 O3D_CamDone(void);
bool8 O3D_CamKeep(void);
void O3D_CamSetKeep(bool8 keep);   // the shot outlives the room's scripts (the opening's)
// A room that isn't a map (its pack name: the opening's) drawn instead of the field while shots play; NULL: the field
bool8 O3D_SceneRoom(const char *file);
void O3D_CamTick(void);
bool8 O3D_CamGet(float eye[3], float tgt[3], float *roll, float *fov);   // a mart's BUY screen shows the field's last picture (shop.c)
bool8 O3D_IsOrreGroup(u8 mapGroup);
bool8 O3D_PackPresent(void);   // ORRE's pack (orre/orre3d.pak) is here (the boat: src/orre/orre_ferry.c)
u8 O3D_ArrivalDir(u16 map, s16 x, s16 y);   // the way a player landing on an ORRE room's entry faces (DIR_NONE: none)
// A field sprite's place on ORRE's 3D ground: (x, feetY) on Emerald's screen -> where the view shows that spot, and its
// depth (0: not on an ORRE map); spriteId: whose (0xFFFF none)
void O3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth);
// Sprites stand in the scene as upright cards: sprite.c notes each one's depth and feet for its OAM entries, LoadOam
// commits them with the OAM, and the renderer hides a sprite pixel the map is in front of
extern float gO3dSpriteDepth[];
extern s16 gO3dSpriteFeet[];
void O3D_NoteOam(u16 spriteId, u16 first, u16 end);
u16 O3D_SpriteScale(u16 spriteId);   // under a shot: how big the sprite is where it stands (Q8; 0: as drawn)
void O3D_CommitOam(void);
// ORRE's tiles carry their behavior in their metatile id (O3D_METATILE_BEHAVIOR + behavior: src/orre/o3d.c)
#define O3D_METATILE_BEHAVIOR 0x300
// The blocks of a map's layout: an ORRE map's come from its pack (Colosseum's collision), others are the layout's
const u16 *O3D_LayoutBlocks(const struct MapLayout *layout);
float O3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a map pixel, NaN none
// Colosseum's units on the current ORRE map (its scripts' positions), and its file and room (src/orre/orre_vm.c)
bool8 O3D_TileToColo(s16 x, s16 y, float *cx, float *cz);
bool8 O3D_ColoToTile(float cx, float cz, s16 *x, s16 *y);
const char *O3D_RoomFile(u16 *room);
bool8 O3D_RoomEntry(u16 room, u32 entry, u16 *map, s16 *x, s16 *y);   // floorLink's destination
u32 O3D_EntryAt(s16 x, s16 y);

#endif
