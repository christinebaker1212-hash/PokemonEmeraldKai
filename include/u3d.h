// Moemon Kai: UNOVA's 3D engine (src/unova/u3d.c; docs/UNOVA_HANDOFF.md). Its own engine, never SINNOH's or ORRE's:
// where the three hook into the game each has its own line.
#ifndef GUARD_U3D_H
#define GUARD_U3D_H

struct MapLayout;
struct Sprite;

void U3D_Update(void);                           // every overworld frame: is this map drawn in 3D, and from where
bool8 U3D_Active(void);                          // this frame's field is drawn by UNOVA's engine
bool8 U3D_IsUnovaGroup(u8 mapGroup);
bool8 U3D_Available(void);   // UNOVA's packs (3D, scripts, words) are beside the game: the region can be chosen / sailed to
// A field sprite's place on the 3D ground: (x, feetY) on Emerald's screen -> where the view shows that spot at the
// ground's height there, and its depth (0: not on a 3D map). spriteId: whose (each keeps the floor it's on)
bool8 U3D_GridPoint(s32 x, s32 y, s32 *outX, s32 *outY);   // (Moemon Maker's 3D PREVIEW helper)
void U3D_PlaceSprite(u16 spriteId, s32 x, s32 feetY, s32 *outX, s32 *outY, float *depth);
extern float gU3dSpriteDepth[];
extern s16 gU3dSpriteFeet[];
void U3D_NoteOam(u16 spriteId, u16 first, u16 end);
void U3D_CommitOam(void);
// UNOVA's layout blocks: metatile ids from here carry a behavior (id - U3D_METATILE_BEHAVIOR; fieldmap.c
// GetMetatileAttributesById), tools/unova/build_pack.py
#define U3D_METATILE_BEHAVIOR 0x300
#define U3D_RAIL_WARP 0x49   // (MB_UNUSED_49: a rail map's way out, warps as it's stepped on: tools/unova/build_pack.py)
bool8 U3D_RailWarpArmed(void);
bool8 U3D_StepBlocked(bool8 isPlayer, s16 fromX, s16 fromY, s16 x, s16 y);   // (a door from the side, a floor's edge)
struct ObjectEvent;
bool8 U3D_ObjectsApart(const struct ObjectEvent *a, const struct ObjectEvent *b);   // (people on different floors)
// Black's gym machinery (field expansion objects, u3d.c): the gimmick of overlay OV's models where Black places them,
// MODEL's animation ANIM at FRAME (-1 its last; ANIM 0xFF: at rest) or played on, hidden, moved
void U3D_ExpLoad(u8 ov);
void U3D_ExpClear(void);
void U3D_ExpAnim(u16 model, u8 anim, s16 frame, bool8 play);
bool8 U3D_ExpPlaying(u16 model);
void U3D_ExpTex(u16 model, u8 anim, bool8 on, s16 frame, bool8 play, bool8 loop);   // (a texture track: Black's
bool8 U3D_ExpTexPlaying(u16 model, u8 anim);
void U3D_ExpGetPos(u16 model, float out[3]);                                        //  SetAnimeEnabled / Paused / Loop)
void U3D_ExpHide(u16 model, bool8 hidden);
void U3D_ExpPlace(u16 model, float x, float y, float z, u16 yaw);
void U3D_ExpPlaceRot(u16 model, float x, float y, float z, u16 pitch, u16 yaw);
void U3D_ExpSetY(u16 model, float y);
float U3D_ExpFrame(u16 model);
u16 U3D_ExpAnimFrames(u16 model, u8 anim);
bool8 U3D_Elboard_Load(u16 zone);   // u3d_elboard.c: the gates' news boards
void U3D_Elboard_Frame(void);
u8 U3D_ZoneWeather(u16 zone);
float U3D_GridGround(s32 x, s32 z, float near);
void U3D_ViewBob(float dy);
bool8 U3D_StoneStep(bool8 isPlayer, s16 fromX, s16 fromY, s16 x, s16 y);
u32 *U3D_ExpOwnTexture(u16 model, u8 anim, u16 w, u16 h);
// Black's 3D cut-ins (ov021 fld3d_ci.c): sequence SEQ over the field held still (SPECIES, PID, OTID: a POKeMON's, for
// sequence 3); busy until it's over; drawn over the finished frame (sdl2.c VDraw)
bool8 U3D_CiStart(u16 seq, u16 species, u32 pid, u32 otId);
bool8 U3D_CiBusy(void);
bool8 U3D_CiStartForClass(u8 trainerClass);
void U3D_CiKeepSong(u16 song);
bool8 U3D_CiSongKept(u16 song);   // (pokemon.c: the battle song a VS cut-in began, kept playing)   // (a VS name cut-in before a battle: its black held until the battle)
void U3D_CiComposite(u16 *frame);                               // (its animation's frame now, -1 none)
void U3D_PlayerFly(bool8 on, float x, float y, float z);     // (the player's sprite and the view at a world place)
void U3D_HexClear(void);                                     // Black's height areas (CLAY's gym's floors, u3d.c)
void U3D_HexSet(u8 i, s16 x, s16 z, s16 w, s16 d, s16 h);
bool8 U3D_HexActive(void);
bool8 U3D_PersonApart(u8 localId);
bool8 U3D_HexOpen(bool8 isPlayer, s16 x, s16 y);
void U3D_SetFloor(float h);
float U3D_Floor(void);
float U3D_HexTopAtPlayer(void);
bool8 U3D_HexHasPlayer(u8 i);
float U3D_ObjectHeight(const struct ObjectEvent *o);         // (a person's height: the player's floor, Black's height)
void U3D_Vm_FieldShown(void);
void U3D_Vm_FieldFrame(void);
bool8 U3D_DoorOpen(s16 x, s16 y, bool8 arriving);   // (u3d.c: Black's doors, at map tile X, Y, MAP_OFFSET in)
void U3D_DoorClose(s16 x, s16 y);
bool8 U3D_DoorBusy(void);
bool8 U3D_DoorShutAt(s16 x, s16 y);
void U3D_DoorsReset(void);
void U3D_DoorNearPlayer(u8 what);   // (0 opening, 1 open, 2 shutting, 3 shutting quietly)
void U3D_BModelPlay(u16 kind, s32 x, s32 z, u8 slot, u8 mode);   // (u3d.c: Black's scripts' build models)
u16 U3D_BModelNode(u16 kind, s32 x, s32 z);
void U3D_BModelNodeDelete(u16 handle);
void U3D_BModelNodePlay(u16 handle, s16 slot, u8 mode);
bool8 U3D_BModelNodeBusy(u16 handle);
bool8 U3D_Vm_CannonAhead(u16 direction);
bool8 U3D_Vm_CrowdLocal(u8 local);   // (u3d_vm.c: a passer-by of CASTELIA's crowd)   // (u3d_vm.c: SKYLA's gym's cannons, one two cells ahead)
bool8 U3D_Vm_IceSlideAhead(u16 direction);   // (u3d_vm.c: BRYCEN's gym's ice slides, the player on one's start)
bool8 U3D_Vm_DockStep(u16 direction);   // (u3d_vm.c: ELESA's gym's coasters, a dock stepped onto)   // (u3d_vm.c: a map first shown, a continued game's too)
const u16 *U3D_LayoutBlocks(const struct MapLayout *layout);
// UNOVA's battle backgrounds, Black's own (u3d_battlebg.c): src/battle_bg.c asks first
#define U3D_BBG_ALL 0
#define U3D_BBG_TILES 1
#define U3D_BBG_MAP 2
#define U3D_BBG_PALETTE 3
#define U3D_BBG_ENTRY 4
bool8 U3D_BattleBg(u8 part);
u8 U3D_BattleColumnAt(s16 x, s16 y);                          // (Black's battle background column of a map tile)
void U3D_LightColor(u8 rgb[3]);                               // (Black's light 0 now, 0-31; white without one)
bool8 U3D_CatchRules(void);                                   // (UNOVA's catches: Black's, u3d_battle.c)
u8 U3D_CatchCase(u8 battler, u16 ball, u8 catchRate, u8 kaiMultiplier);
void U3D_SetDarkGrassBattle(bool8 on);
bool8 U3D_DarkGrassBattle(void);
#define U3D_DARK_GRASS_DOUBLE_ODDS 40   // (Black's dark grass: 0x28 in 100 two at once, field_encount.c)
void U3D_BattleLeadAs(u8 level, u32 otId, u32 personality, const u8 *nickname);   // (u3d_battle.c: a traded POKeMON's battle)
void U3D_RefreshBlocks(void);   // the field's blocks again after Black replaced the map under the player   // a UNOVA map's from the pack, others the layout's
// Black's scene cameras (u3d.c, played by u3d_vm.c): to a target (Black's world units, fx32), pitch / yaw (65536ths of a
// turn) and distance over Black's frames, back to the player's view, at once back, still moving?
void U3D_CameraMoveTo(u16 pitch, u16 yaw, s32 dist, s32 x, s32 y, s32 z, u16 frames);
void U3D_CameraMoveBack(u16 frames);
void U3D_CameraReset(void);
void U3D_CameraPlace(const float eye[3], const float fwd[3]);
void U3D_CameraTurn(u16 pitch, u16 yaw, u16 frames);
void U3D_CameraLook(u16 frames);
void U3D_ObjectFly(u8 spriteId, bool8 on, float x, float y, float z);
void U3D_PlayerWorldPos(float out[3]);
void U3D_CameraEye(float eye[3], float fwd[3]);
void U3D_PlayerBob(float dy);
void U3D_PlayerDepthTest(u8 mode);   // (0 as always, 1 hidden by the scene, 2 always shown)
bool8 U3D_ObjectFlyPos(u8 spriteId, float out[3]);
void U3D_TileWorldPos(s16 x, s16 y, float out[3]);
void U3D_CameraShake(s16 x, s16 y, u16 steps, u8 total, s16 dx, s16 dy, u16 thresh, u16 period);
bool8 U3D_CameraShaking(void);
void U3D_CameraMoveMasked(u16 pitch, u16 yaw, s32 dist, s32 x, s32 y, s32 z, u16 frames, u8 mask);
bool8 U3D_CameraMoving(void);
float U3D_TestGround(s32 mapX, s32 mapY, float near);         // tests: the ground's height (map pixels), NaN none
void U3D_ShopPicture(bool8 on, s16 dx, s16 dy);               // shop.c's BUY screen: the field's last picture behind
u8 U3D_Season(void);                                          // Black's season now: 0 spring .. 3 winter
u16 U3D_CurrentZone(void);                                    // the Black zone the player is on (0xFFFF: not UNOVA)
s32 U3D_MapIndex(void);
bool8 U3D_Vm_HasCGear(void);
void U3D_Vm_GoToEntralink(void);   // (the C-GEAR's ENTRALINK button: src/unova/u3d_screen.c)
u16 U3D_Vm_Work(u16 id);
void U3D_Vm_ContinueHome(void);
u16 U3D_Vm_ObjCode(u16 code);
void U3D_Vm_WarpPos(s16 *x, s16 *y);
void U3D_Vm_NewDay(u16 days);   // UNOVA's daily changes (BLACK CITY's residents' moods)   // a warp tile where Black's script has moved its connection now   // a work object code's code (Black's 0xA2-0xB1: its work 0x4020 + n)
bool8 U3D_TryRoamerEncounter(u8 *level);   // UNOVA's roamers (TORNADUS, THUNDURUS) on this zone's grass
bool8 U3D_RoamerAfterBattle(struct Pokemon *mon, u8 outcome);   // after UNOVA's game clear: CONTINUE starts at home (Black's continue spot)   // a work of Black's event work   // FENNEL gave the C-GEAR (UNOVA's device)
void U3D_Vm_WhiteoutHome(void);   // a new game in UNOVA: a whiteout returns home to NUVEMA (Black's first warp point)
bool8 U3D_MapOfZoneTile(u16 zone, u16 x, u16 z, u8 *group, u8 *num, s16 *mapX, s16 *mapY);                                       // the player's map's row of u3d_maps.h (-1: not UNOVA)
u8 U3D_MapCellX(s32 index);                                   // ... where it lies on its Black matrix (blocks)
u8 U3D_MapCellZ(s32 index);
// UNOVA's scripts (src/unova/u3d_vm.c: Black's own bytecode)
#define U3D_VAR_BASE 0xB000                                   // Emerald vars 0xB000 + n are Black's works 0x4000 + n
#define U3D_VAR_COUNT 0x140
u16 *U3D_Vm_VarPointer(u16 id);
bool8 U3D_Vm_TrainerSight(void);   // field_control_avatar.c: a trainer of Black's has seen the player (their script runs)
void U3D_Vm_DebugSet(bool8 flag, u16 id, u16 value);   // tests: a Black work or flag (mp u3dwork / u3dflag)
bool8 U3D_Vm_ObjectHidden(u8 localId);                        // a person hidden by Black's flag
bool8 U3D_Vm_SceneStart(void);                                // script.c TryRunOnFrameMapScript: a scene starts
bool8 U3D_VmRunning(void);
u16 U3D_Vm_PersonGfx(u8 localId, u16 gfx);                     // (a person's look a script set: Black's NPC table)
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
bool8 U3D_Gfx_SmallIsArt(u16 code);   // MIXED's copy is AveonTrainer's (tools/aveon/aveon.py)

// UNOVA's battles (src/unova/u3d_battle.c: Black's trainers through TRAINER_UNOVA_A / _B)
struct Pokemon;
bool8 U3D_IsTrainer(u16 trainerId);
const u8 *U3D_ClassName(u16 trainerId);
u8 U3D_Party(struct Pokemon *party, u16 trainerId, bool8 firstTrainer);
u32 U3D_Money(u16 trainerId);
bool8 U3D_PrepareBattle(u16 first, u16 second);
void U3D_BattleEnded(bool8 won);
u8 U3D_BattleResult(void);
u8 U3D_TrainerClass(u16 trainerId);
void U3D_CatchTutorial(void);              // JUNIPER's catching demonstration (the VM's 0x17D)
#define U3D_BACK_JUNIPER 2                 // Black's back pictures (archive 0x49): 0 / 1 the player characters, 2 JUNIPER
bool8 U3D_PrepareTagBattle(u16 partner, u16 first, u16 second);   // Black's StartMultiTrainerBattle: beside PARTNER
bool8 U3D_Gfx_Back(u8 index, const u8 **tiles, const u16 **pal);   // four 64x64 frames (GBA tiles) and their colours       // Black's class of TRAINER_UNOVA_A / _B (its battle picture)
u16 U3D_BattleSong(void);               // pokemon.c GetBattleBGM on an UNOVA map: Black's own
u16 U3D_VictorySong(u16 song);          // ... MP_RegionVictorySong

// UNOVA's wild POKéMON (src/unova/u3d_wild.c: Black's encounter tables)
struct WildPokemonInfo;
bool8 U3D_WildInfo(u16 behavior, const struct WildPokemonInfo **info, u8 *area);
bool8 U3D_WildSpecial(u8 type, const struct WildPokemonInfo **info);   // (Black's types 2, 4, 5, 6: phenomena, fishing)
bool8 U3D_PhenomSetting(u8 *kind, u8 *steps, u8 *percent);
u8 U3D_PhenomKindAt(s16 x, s16 y);                             // (a tile's phenomenon kind, 0xFF none: u3d_phenom.c)
void U3D_GridOfMap(s16 x, s16 y, s32 *gx, s32 *gz);

#endif
