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
void O3D_CamStopShot(void);                  // a shot ends, the scene's own camera stays
// The scene's own camera (o3d_cam.c sFree: GSscene_SetMode(3)): its eye / rotation set or moved over SECONDS
void O3D_FreeMode(bool8 on);
void O3D_FreeSet(u8 which, const float v[3], float seconds);   // WHICH 0 the eye, 1 the rotation (radians X Y Z)
void O3D_FreeFov(float fov);
void O3D_FreeReturn(float seconds);           // cameraReturn: back to the field's camera
bool8 O3D_FreeMoving(void);
bool8 O3D_FreeActive(void);                   // the scene's camera is on (not on its way back)
bool8 O3D_FreeBack(void);                     // ... on its way back
void O3D_CamEaseBack(float seconds);          // the player walks: a held shot or the scene's camera back to the field's
void O3D_FieldView(float eye[3], float rot[3]);
float O3D_FieldFov(void);                    // ... its vertical field of view (degrees)   // the field camera's eye and rotation in Colosseum's units (o3d.c)
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
// event_object_movement.c SetObjectSubpriorityByElevation on an ORRE map: the subpriority a field sprite's distance
// from the camera gives (nearer in front, whatever way the camera looks); -1: not on ORRE's field
s32 O3D_DepthSubpriority(const struct Sprite *sprite);
u16 O3D_SpriteScale(u16 spriteId);
// This frame, the sprite stands on its owner's spot and is drawn LIFT texel rows above it (scaled as the spot is):
// an emotion bubble over a person's head (trainer_see.c SpriteCB_TrainerIcons)
void O3D_LiftSprite(u16 spriteId, s32 lift);   // under a shot: how big the sprite is where it stands (Q8; 0: as drawn)
void O3D_CommitOam(void);
// ORRE's tiles carry their behavior in their metatile id (O3D_METATILE_BEHAVIOR + behavior: src/orre/o3d.c)
#define O3D_METATILE_BEHAVIOR 0x300
// The blocks of a map's layout: an ORRE map's come from its pack (Colosseum's collision), others are the layout's
const u16 *O3D_LayoutBlocks(const struct MapLayout *layout);
s32 O3D_TriggerAt(s32 x, s32 y);   // a tile's trigger (Colosseum's collision id) on the current ORRE map; -1 none
float O3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a map pixel, NaN none
// Colosseum's units on the current ORRE map (its scripts' positions), and its file and room (src/orre/orre_vm.c)
bool8 O3D_TileToColo(s16 x, s16 y, float *cx, float *cz);
bool8 O3D_ColoToTile(float cx, float cz, s16 *x, s16 *y);
const char *O3D_RoomFile(u16 *room);
bool8 O3D_RoomEntry(u16 room, u32 entry, u16 *map, s16 *x, s16 *y);   // floorLink's destination
u32 O3D_EntryAt(s16 x, s16 y);

#endif
