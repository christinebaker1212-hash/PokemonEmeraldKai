// Moemon Kai: SINNOH's 3D engine (src/sinnoh/s3d.c; docs/SINNOH_HANDOFF.md)
#ifndef GUARD_S3D_H
#define GUARD_S3D_H

struct MapLayout;

void S3D_Update(void);                           // every overworld frame: is this map drawn in 3D, and from where
bool8 S3D_Active(void);
bool8 S3D_Heal_Start(u8 count);   // field_effect.c FldEff_PokecenterHeal: SINNOH's machine (s3d_heal.c); FALSE: not here
void S3D_ShopPicture(bool8 on, s16 dx, s16 dy);   // shop.c's BUY screen: the field's last 3D picture behind it, moved                          // this frame's field is drawn by the 3D engine
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
s8 S3D_DoorAnimate(s32 x, s32 y, u8 kind);   // Platinum's door prop at a warp tile: 0 / 1 open / close walking in, 2 / 3 walking out; a task or -1
s8 S3D_DoorClose(s32 x, s32 y);              // its closing (after walking in or out)
s8 S3D_DoorSound(s32 x, s32 y);              // 0 hinged, 1 sliding, 2 the DEPT. STORE's chime; -1 no door prop there
bool8 S3D_DoorRunning(void);
bool8 S3D_DoorLocked(s16 x, s16 y);         // a door tile with no warp on it (a locked door: a wall); field coords + 7
// SINNOH's layout blocks: metatile ids from here carry a behavior (id - S3D_METATILE_BEHAVIOR; fieldmap.c
// GetMetatileAttributesById), tools/sinnoh/build_pack.py
#define S3D_METATILE_BEHAVIOR 0x300
float S3D_TestGround(s32 mapX, s32 mapY, float near);   // tests: the ground's height at a point (map pixels), NaN none           // a SINNOH map (its own music, battle songs...)
u16 S3D_MapMusic(u16 song);                      // a SINNOH map's song by night (Platinum's own night versions)
// SINNOH's text pack: a message of a bank (tools/sinnoh/build_text.py; "Message missing" without the pack)
const u8 *S3D_Text(u16 bank, u16 msg);
bool8 S3D_TextNamed(const char *name, u16 *bank, u16 *msg);   // a message by Platinum's own name (s3d_text.c)
const u8 *S3D_TextByName(const char *name);
// The blocks of a map's layout: a SINNOH map's come from the pack (Platinum's collision), others are the layout's
const u16 *S3D_LayoutBlocks(const struct MapLayout *layout);
// Props placed at run time (SINNOH's gyms that move, src/sinnoh/s3d_gyms.c): Platinum's model by its number, in the
// map's matrix world (16 units a tile; S3D_MapOrigin: the map's corner there), turned about the up axis in degrees
#define S3D_DYN_PROPS 128
bool8 S3D_DynProp(u8 slot, u16 model, float x, float y, float z, float turn);
bool8 S3D_DynPropTurned(u8 slot, u16 model, float x, float y, float z, float tilt, float turn);
void S3D_DynPropHide(u8 slot);
void S3D_DynPropShade(u8 slot, u8 alpha);
void S3D_DynPropFade(u8 slot, u8 alpha);    // (see-through: 0-31)   // (drawn as a see-through dark shape)
void S3D_DynPropsClear(void);
// (Moemon Maker M5) a model made outside Platinum (src/maker/maker_models.c: a .glb), added to the meshes: triangles of
// 8 floats a vertex (x y z, u v 0-1, nx ny nz), one RGBA texture or NULL; its mesh or -1; and a prop of it
s16 S3D_AddMesh(const float *v8, u32 nverts, const u32 *rgba, u16 tw, u16 th);
bool8 S3D_DynPropMesh(u8 slot, s16 mesh, float x, float y, float z, float turn, float scale);
#define S3D_MAKER_PROPS_FIRST 112   // (slots 112-127: the Maker's models on the current map)
#define S3D_DARK_LIGHTS 40
void S3D_SetDarkness(float level, const s16 (*lights)[3], u8 n);   // a dark room (0: none) lit around {x, y, radius}
bool8 S3D_MapOrigin(float *x, float *z);
bool8 S3D_MapCell(u16 *mat, u16 *cx, u16 *cz);
void S3D_SetLift(bool8 on, float x0, float z0, float x1, float z1, float y);   // (s3d_lift.c: props in the rectangle at y)   // the map's matrix and its top-left cell there
bool8 S3D_MapCells(s32 *cx, s32 *cz);   // the current map's top-left block in Platinum's matrix
// A gym's own ground heights (map tiles; NaN: the block's), asked with the height the object is nearest
extern float (*gS3dHeightHook)(s32 mapX, s32 mapY, float h);
extern float (*gS3dObjectHeightHook)(u8 localId, float h);   // a gym's people's heights by local id (NaN: the ground's)
void S3D_PlayerWorld(float *x, float *z);       // the player in the map's matrix world
void Sinnoh_StoryRefresh(void);                   // shows / hides SINNOH's story people by the story (s3d_legends.c)
void S3D_Gyms_MapLoaded(void);                    // a map's grid is (re)built: a gym's state on it (s3d_gyms.c)
void S3D_Gyms_Update(void);                       // every overworld frame
// The DISTORTION WORLD as Platinum's (src/sinnoh/s3d_distortion.c)
struct MapEvents;
struct ObjectEventTemplate;
void S3D_Dw_AddPeople(struct MapEvents *ev, struct ObjectEventTemplate *buf, u32 cap);   // (s3d_scrcmd.c, as a map loads)
void S3D_Dw_AddObject(u8 localId);      // Platinum's AddDistortionWorldMapObject (s3d_misc.c op 30)
void S3D_Dw_DeleteObject(u8 localId);   // DeleteDistortionWorldMapObject (op 31)
void S3D_Dw_InitPersisted(void);
bool8 S3D_Dw_FacingWater(void);         // SURF on a platform's water (field_control_avatar.c)
void S3D_Dw_Surf(void);                 // special: the hop onto it
struct ObjectEvent;
bool8 S3D_Dw_BoulderFallAt(s16 x, s16 y);   // a boulder may be pushed onto a hole / pit there (map coords)
bool8 S3D_Dw_BoulderPushed(struct ObjectEvent *boulder);   // after a push: TRUE, it falls        // InitPersistedMapFeaturesForDistortionWorld (op 29)
void S3D_Dw_Player3DPos(u16 *x, u16 *y, u16 *z);
void S3D_Dw_Shadow(u16 n);      // StartDistortionWorldGiratinaShadowEvent (op 32; 0xFFFF: Finish, op 33)   // GetPlayer3DPos (ops 35, 36)
bool8 S3D_Dw_Active(void);
bool8 S3D_Dw_TileOffset(s32 *x, s32 *z);
void S3D_Dw_ResetCamera(void);
void S3D_Dw_OnMapLoad(void);
bool8 S3D_Dw_PlayerStep(u8 dir, u16 heldKeys);   // (field_player_avatar.c PlayerStep: TRUE when handled)
void S3D_Dw_Update(void);
bool8 S3D_Dw_View(float *pitch, float *yaw, float *target);
bool8 S3D_Dw_PlayerSprite(u16 spriteId, s32 *outX, s32 *outY, float *depth);
void S3D_Dw_Print(void);
bool8 S3D_DynPropTurnedScaled(u8 slot, u16 model, float x, float y, float z, float tilt, float turn, float scale);
bool8 S3D_DynPropScaled(u8 slot, u16 model, float x, float y, float z, float turn, float scale);
bool8 S3D_ProjectWorld(float x, float y, float z, float *sx, float *sy, float *depth);   // (s3d.c: this map's world)
float S3D_PlayerGround(void);                     // (s3d.c: the player's ground height last frame)
bool8 S3D_Gyms_SeesPlayer(u8 localId);            // a gym's trainer can see the player (same floor)
bool8 S3D_Gyms_JumpAt(s16 x, s16 y, u8 direction);   // a step there jumps (ETERNA's hour hand; map coords + 7)
bool8 S3D_Gyms_PushAt(s16 x, s16 y, u8 direction);   // walking into it hits it (VEILSTONE's bags; map coords + 7)
void S3D_Gyms_RedirectWarp(s16 x, s16 y);       // a warp from there (map coords + 7): a gym's wrong door
bool8 S3D_Gyms_WrongDoor(s16 x, s16 y);         // (map coords) HEARTHOME's gym: a door its floor's clue rules out
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

// SINNOH's TV (s3d_tv.c): what the BATTLE FRONTIER, ribbons and the GAME CORNER give its RECORDS and interviews
struct Pokemon;
void Sinnoh_TvBattlePoints(u32 bp);
void Sinnoh_TvTowerRecord(struct Pokemon *mon, u16 streak, bool8 single);
void Sinnoh_TvTowerOutcome(bool8 win, u16 streak);
void Sinnoh_TvFrontierHall(struct Pokemon *mon);
void Sinnoh_TvRibbon(struct Pokemon *mon, u8 ribbon, u8 count);
void Sinnoh_TvSlots(u32 coins, u32 start, u32 minutes);

#endif
