# UNOVA: handoff

Pokémon Black as a playable region, built the way SINNOH was from Platinum (docs/SINNOH_HANDOFF.md). The user's
decisions: docs/UNOVA_DECISIONS.md (read it first: every Platinum decision holds unless it says otherwise).

## Where things stand (2026-11-11)

- The engine draws UNOVA (2026-11-11): src/unova/u3d.c (include/u3d.h) reads unova/black3d.pak, which
  tools/unova/build_pack.py builds from the ROM (every matrix: 238, about 30 s, 19 MB; `build_pack.py OUT [MATRIX...]`
  for a few). 533 maps from Black's zones (tools/unova/gen_maps.py: one a block outdoors, one a zone indoors, groups
  gMapGroup_Unova1-5, MAPSEC_UNOVA_* from 0x200 with Black's place names). Black's camera (setting 0: 237 units away,
  53.2 degrees down), its seasons (the month: spring, summer, autumn, winter, round again), its lights by area and
  season with its time zones' hours, doors walked into and mats walked off (warps from Black's connections), its
  collision grids as the maps' blocks. Test by hand: `MOEMON_U3D_PAK=unova/black3d.pak` (+ MOEMON_U3D_HOUR,
  MOEMON_U3D_SEASON), `mp warp 52,12,16,16` (NUVEMA TOWN), `mp warp 52,14,5,6` (the player's room: Black's
  opening plays). No music, battles or way in for players yet (the region choice and the boat come with the first
  area).
- Black's scripts run in UNOVA's VM (src/unova/u3d_vm.c, 2026-11-11): unova/black_scripts.pak
  (tools/unova/build_scripts.py: the script files, the shared ranges, the zones, the event files). Black's people,
  signs and trigger spots are the maps' object / bg / coord events (gen_maps.py events_of: a person's local id is
  its place in the event data + 1; coord events compare Emerald vars 0xB000 + n = Black's works 0x4000 + n), all
  running data/scripts/unova.inc, whose specials start Black's script and wait; screens (the gift box's choice:
  Kai's starter table for now) are opened from Unova_EventScript_Screen and the VM resumes after. Scenes from the
  zones' scene tables start from script.c TryRunOnFrameMapScript (U3D_Vm_SceneStart); the field-init and
  zone-change scripts on ON_TRANSITION; Black's new-game script (9600) the first time. Black's event work (works,
  flags, as Black lays them out) is the region side file's chunk UNEW. Commands not played yet are logged
  ("u3d vm: NNN not played yet") and read past: their arguments are known. Done: control flow, works / flags,
  messages (balloon, info, plain, board: Black's own words), yes / no, word slots (player, species, numbers,
  party), people (movement lists as the game's movements, turning, placing, removing), fades, POKéMON (heal, give,
  party count, cries), money, trainer battles (src/unova/u3d_battle.c: Black's trainers, a/0/9/2 and a/0/9/3 in the
  scripts pack v2, fought through TRAINER_UNOVA_A / _B; names bank 190, classes 191; Black's script decides what a
  loss means). The story plays from the opening to the end of ACCUMULA TOWN's opening (2026-11-11): the room, the
  mom scene and the Xtransceiver, BIANCA's house, JUNIPER's lab and the POKéDEX, the TOWN MAP, ROUTE 1 with JUNIPER
  ("One, two!"), ROUTE 1's north end, ACCUMULA's POKéMON CENTER tour and the nurse's heal, GHETSIS's speech, N and
  his battle. Also played: Black's scripts calling scripts (a frame stack), scene cameras (u3d.c U3D_CameraMoveTo:
  target, pitch, yaw, distance, eased), people a script adds (AddMmdl: set out once the field is up, so a zone's
  field-init script's survive loading), SetProcAction (the script run next, in the zone the player is then in:
  JUNIPER's ROUTE 1 scene), fades that wait for the fade before, a crowd's side windows (AddSubWinMsg) in the message
  box, the healing machine's tune, a POKéMON's params. Tested by a harness script that warps from trigger to
  trigger (a scene's trigger is one of its zone's Pos events: tools/unova/events.py) and mashes A; a scene alone with `mp u3dwork WORK VALUE` / `mp u3dflag FLAG 0|1` (hex;
  `u3dwork 4081 1` then `u3dwork FFFF 0` ends the room's opening). Since: ROUTE 2 (Running Shoes, BIANCA's battle),
  STRIATON's gatekeeper; JUNIPER's catching demonstration (u3d_battle.c U3D_CatchTutorial: the game's catching
  tutorial battle with Black's back picture of her); every trainer's battle picture (Black's fronts, cell 0 over its
  80x80 sheet; the backs: cell 0 of a 256x128 sheet, 2D mapping, halved: tools/unova/build_gfx.py, pack version 2);
  map replacement flags as Black's works (blocks not redrawn yet). Since (2026-11-12): STRIATON after the gym
  (FENNEL's errand, HM01 CUT), the DREAMYARD (CUT's trees: a deleted person without a flag stays gone until the map
  loads again, as Black's trees grow back; BIANCA, MUNNA, Team Plasma's two battles, GHETSIS, MUSHARNA), FENNEL's
  lab (the C-GEAR and PAL PAD: Black's script menus, MSGWIN_CmdInitMenu / AddMenuItem / StartMenu, in the game's
  multichoice window; the C-GEAR got is UNEW's `cgear` byte, U3D_Vm_HasCGear), ROUTE 3 (the twins' double battle,
  CHEREN's battle, the stolen POKéMON), WELLSPRING CAVE (Black's tag battle beside CHEREN: StartMultiTrainerBattle
  through the game's in-game partner battle, TRAINER_UNOVA_C, the player's first three POKéMON, CHEREN from behind
  with Black's back picture: u3d_battle.c U3D_PrepareTagBattle, U3D_Gfx_PartnerBack). Whiteouts return to Black's
  warp points (ov010's 17, scripts pack v6: SetWarpDestination sets the save's last heal place; a new game's is home).
  NACRENE (CHEREN at the gate: Black's SetNpcGridPos, a person's place for the zone, kept over its maps), N at the
  museum, HAWES' tour, the library's quiz trainers, LENORA (her badge and TM), Team Plasma's theft, PINWHEEL FOREST
  (BURGH, the grunts, the DRAGON SKULL, GORM). A shortcut test that skips a scene must set the flags that scene sets
  (BURGH in the forest: `mp u3dflag 228 0`, LENORA's). Next: SKYARROW BRIDGE and CASTELIA are Black's rail maps
  (below). Test scenes need `mp battlestyle 1` (SET) when A is mashed through a battle. Not yet: Black's 3D cut-ins (0x19F, 0x240),
  build-model animations (doors: 0x127-0x12A), redrawing replaced map blocks, warp destinations / whiteout points
  (0xD9), the value screen (0x155), people following the player (0x06B), the naming screen, Black's gift box screen,
  the region choice's boat and CASTELIA's pier, NACRENE onward.
- UNOVA is the game's fifth region (REGION_UNOVA 4, REGION_COUNT 5; 2026-11-11): the new game's region menu
  (the player's room in NUVEMA: `MOEMON_DEBUG_REGION=4`), the boat lists (CASTELIA (UNOVA), MULTI_REGIONS_FROM_UNOVA;
  the first boat lands in the room for now: no pier scene at CASTELIA yet), the ship's crossing (its passengers,
  PROF. JUNIPER's lab for the partner held there: Z396, local id 1), achievements' UNOVA tabs (C_UNOVA...), the BADGE
  CASE (Black's eight, PokeAPI badges 33-40: tools/moemon_badges/build_badges.py), the TRAINER CARD's time,
  UNOVA's badges as flags FLAG_UNOVA_BADGE01 + n (0x7F00, kept with Black's event work: event_data.c routes them).
  Side files from before convert on loading: ACHR 2, KIST 2, LVL5, TRAINER CARD 5. HOENN's harbor arrival keeps
  VAR_REGION_ARRIVAL 4 (its frame script's number). save_compat.sh's "format 0" summary mismatch is older than this
  (the build before it fails the same way).
- Wild POKéMON (src/unova/u3d_wild.c): Black's encounter tables (scripts pack v4: a/1/2/6 by the zone's encounter
  id, the zones' records 18 bytes from v4), the season's row, grass / dark grass / surfing at their own rates, fed to
  wild_encounter.c StandardWildEncounter (its UNOVA line). Dark grass is still one POKéMON (Black: two).
- Items (tools/unova/gen_items.py -> src/data/unova_items.h, with Black's bag pocket of each): Black's numbers to
  the game's by name / constant; a TM by the move it teaches (Black's TM01 HONE CLAWS is not FOCUS PUNCH). The user's
  rule: an item the game has is the game's, a new one is Black's: tools/unova/gen_new_items.py added Black's 54 new
  ones (812-865: the Xtransceiver, GRAM 1-3, PROP CASE, passes, DRAGON SKULL, LIGHT / DARK STONE, treasures, relics,
  COVER / PLUME FOSSIL, wings, the 17 gems (battle_gen59.c: half again the power once), PRISM SCALE, RED CARD, RING
  TARGET, BINDING BAND, EJECT BUTTON, SWEET HEART, POKé TOY) from PokeAPI's names, texts and icons, and
  tools/moemon_tms/gen_tm4.py the 25 TMs whose moves had no TM here (TM140-TM164, items 866-890, gTm4Learnsets).
  Still ITEM_NONE (87): HeartGold's / Platinum's own (Data Cards, Apricorn Box...: Black never gives them), Gen 5's
  mails, the Entralink's X items 2 / 3 / 6, Dire Hit 2 / 3 and Urges, the CHERISH BALL and the Drives (events).
  RED CARD / EJECT BUTTON / RING TARGET / BINDING BAND are held but do nothing in battle yet.
- Music: Black's whole soundtrack (204 songs: tools/kaisong/unova_songs.py slots / convert; MUS_UNOVA_* from 831,
  named after Black's own sequences; src/data/unova_songs.h maps Black's sequence numbers), the zones' songs as the
  maps' music (spring's), the scripts' song commands (u3d_vm.c: fanfares pushed and popped), Black's battle and
  victory songs by its trainers' classes and names (scripts pack v3 song kinds; u3d_battle.c U3D_BattleSong).
- Black's people: unova/black_gfx.pak (tools/unova/build_gfx.py, about two minutes: 759 of the 799 object codes),
  src/unova/u3d_gfx.c; graphics ids OBJ_EVENT_GFX_UNOVA_FIRST (0x600) + Black's object code (an object event keeps
  12 bits of its graphics id: 0x1000+ would wrap), palettes by tag 0xD000 / 0xD400 + code (event_object_movement.c
  UnovaObjectInfo); OPTIONS > DISPLAY > SPRITE SCALING > UNOVA PEOPLE: MIXED (Black's shrunk to 0.84, the default)
  or GEN 5.
- After the credits (2026-11-12): ALDER's rematch, the HALL OF FAME and the credits again (SetGameClearEventAndEnd
  mode 1), the continue spot at home (U3D_Vm_ContinueHome), roamers knocked out set off again (UnovaGameClear).
  White's own POKéMON: mixed into Black's slots (u3d_wild.c WhiteCounterpart), THUNDURUS roams beside TORNADUS,
  ZEKROM at the top of DRAGONSPIRAL after the credits (ZONE_ZEKROM). KYUREM, LANDORUS (with both genies), the
  Swords of Justice, VOLCARONA, CYNTHIA's villa in UNDELLA (her battle), LOOKER and the Seven Sages (the DREAMYARD's
  GORM played), BLACK CITY's residents (WFBC records, scripts pack v9: battles, talk, moods by the day) and its five
  stalls. LIBERTY GARDEN: the LIBERTY PASS comes with the clear (as HOENN's tickets with becoming Champion); CASTELIA's
  pier sailor and the boat (CheckBeaconTrainerKey always passes), VICTINI, the lighthouse grunt, JUNIPER's scene; the
  boat back by Black's rail-location warp (gen_maps.py u3d_railpos.h sUnovaRailWarps: across counts from the line's
  edge, FIELD_RAIL_MakeLineLocation). Trades (scripts pack v8). Clocks set and daily flags for a game begun outside
  HOENN (clock.c). Tests: tools in the session's scratchpad (scene.py: warp beside a tile of a zone, set works and
  flags, press keys; `steps=KEY:N:WAIT;...` for exact input, `cmd:mp ...` for debug commands).
- The BATTLE SUBWAY (2026-11-12): src/unova/u3d_subway.c ports Black's bsubway_scr.c / fld_btl_inst_tool.c (the
  GEAR STATION's lines run Black's own scripts; BSUBWAY_SCR_CmdTool's commands; the field's ones, the places and
  passengers, in u3d_vm.c SubwayField). Trainers drawn per train and battle (ov021's ranges, the bosses at 21 / 49),
  their sets built as Black builds them (scripts pack v10: a/2/1/4 sets, a/2/1/5 trainers, the tables), names system
  bank 11, greetings bank 184; level 50 copies of the chosen POKéMON (party menu: Black's banned list), no EXP or
  prize; the multi partner is the other hero (trainers 300-305, Black's back pictures 0 / 1); BP by streak into the
  game's own BP; records in UNEW (sWork.subway, u3d_subway.h); REST saves and resets; Battle Videos and wireless
  closed (flag 12 always set). Black's POKéMARTs (SHOP_StartShop): lists by badges and numbered (pack v10 after the
  subway's tables), the game's POKéMART opened straight on BUY or SELL (shop.c MP_CreatePokemartDirect), BLACK CITY's
  stalls at their residents' prices (shop.c MP_Shop_SetPrices), the BP exchanges (u3d_bpshop.c, Black's words).
  Gym gimmicks kept: CASTELIA's honey doors (u3d_vm.c PlateDoorsRefresh), NIMBASA's coasters (U3D_Vm_DockStep: a dock walked
  onto facing the rails rides its lane's cart, lever sWork.gym[0], to the loop's other stop; lanes 1, 2 locked till Black's
  flags 0x74 / 0x75, their trainers' scripts 7 and 8; Unova_EventScript_Ride; carts and rails not drawn).
  DRIFTVEIL's mine (Black's height areas, HEIGHT_EX: u3d.c U3D_HexSet; floors at 50 / 40 / 30 over the ground, the
  player on one at a time, U3D_StepBlocked / U3D_HexOpen; six lifts their levers send between two heights, the player
  riding, u3d_vm.c MineLift; people kept to their floors by Black's event heights, U3D_ObjectsApart; the unseen blockers
  as data, U3D_Vm_BlockerAt, or they'd fill the game's 16 people; a silent person over a sign lets the sign answer).
  MISTRALTON's cannons (ov041: walked into from two cells away, U3D_Vm_CannonAhead; Black's script 2 climbs in, the
  shot a silent warp to the landing cell of ov041's gCannonsFlights for the cannon and the facing; the flight itself
  and the cannons' turning not drawn). The CELESTIAL TOWER's bell (the gym's door, Black flag 0x2AE) is rung from the
  tile in front of it.
  BRYCEN's gym (2026-11-13, Black's ov045 and its rails' own attributes): the ice rink as Black's grid (0x18 slide
  ice), its curves, ramps and way down as forced rides from a jump-start tile ("the blue tile") to the far one
  (tools/unova/gen_gyms.py -> src/unova/u3d_gyms.h, u3d_vm.c U3D_Vm_IceSlideAhead / IceRide); a curve starts where
  Black checks the player's facing (pos events of check type 1-4) and runs only when its switch allows
  (ReplacePlayerIfRequirementMet, ov045's requirement table; the switches are works 0x4001-0x4003 flipped by Black's
  scripts as they move the spinners' unseen blockers). The rails' tiles are out of the walkable grid (build_pack.py
  lay_rails). Its ramps' arcs stand up (rails.py place: an anchor between the ends). Solved end to end like the Thonky
  walkthrough: each curve, all three switches, both ramps, to BRYCEN.
  DRAYDEN's gym (2026-11-13, Black's ov046): three dragons, each turned by two levers (thrown by buttons that bounce
  the player a tile: StartHopEvent) to a pose that opens one pair of its rail lines (sRotatingRoomPoseLines from the
  overlay; the tiles in src/unova/u3d_gyms.h, U3D_Vm_DragonBlocked in U3D_StepBlocked); the zone-change script sets
  the starting poses (Ov037Finish: 3, 1, 2). Not yet: the dragon models, and the heights between Black's levels (the
  flattened grid lets the player step between walkways at different heights, so the poses close arms but don't yet
  force the route).
  The TOWN MAP and FLY (2026-11-13): Black's own town map (a/0/8/5 picture, a/0/8/6 its 61 town records) on
  region_map.c's QZMAP_UNOVA page, at 5/7 its size in the 28 x 15 grid (tools/unova/gen_townmap.py: the header
  src/data/region_map/unova_townmap.h, the heal locations HEAL_LOCATION_UNOVA_*, the picture "TMAP" in black_gfx.pak
  drawn by src/unova/u3d_townmap.c); FLY lands in front of each town's POKéMON CENTER (NUVEMA: the zone's start) and
  opens with Black's arrival flags 0x9B0-0x9BF, set on entering the town's zone (U3D_TownMapArrive), and for older
  games by the UNOVA badges held.
  Not yet: the GEAR STATION's trains drawn on the platforms; the gyms' gimmicks (Black's field_gimmick_*: STRIATON's
  switches, NACRENE's books,
  ICIRRUS's ice, OPELUCID's bridges), MUSICALS, NIMBASA's stadiums, the ROYAL UNOVA, the survey HQ.
- The ROM: the user's Pokemon Black (USA, Europe) (NDSi Enhanced), SHA-1 26ad0b9967aa279c4a266ee69f52b9b2332399a5,
  from their Google Drive (the link in docs/UNOVA_DECISIONS.md's source message; the release fetches it through the
  UNOVA_ROM_URL secret, added by the user 2026-11-11). Never committed; locally in ~/.cache/moemon/unova/black.nds.
- The decomp: Project-Holon/aletheia (byte-matching, 100%; code only, assets from the ROM), cloned to
  ~/.cache/moemon/unova/aletheia. Its own tools unpack the ROM:
  `python3 -I tools/ndsrom.py extract black.nds -o ~/.cache/moemon/unova/extract` (247 NitroFS files, 237 overlays).
  tools/assets/narc.py reads NARCs, sdat.py / sseq.py / sbnk.py / swav.py the sound archive (wb_sound_data.sdat).

## What the ROM holds (found so far)

- Zone headers: a/0/1/2 file 0, 427 records of 0x30 bytes (src/game/arm9/zonedata.c struct ZoneHeader): map type,
  area id, matrix id, script / init-script / message archive ids, four BGMs (one per season), encounter id, event
  data id, parent zone, place name, weather, camera, flags (bike, run, fly...), camera area, the start position.
- Words: a/0/0/2 (system banks) and a/0/0/3 (story banks; a zone's msgArcId indexes these), decrypted as
  msgdata.c does: tools/unova/text.py (`find WORDS`, `story N`). Place names: a/0/0/2 bank 89. Controls are
  0xF000 runs ({0100:0}: the player's name; BE01 / BE00: wait for a key / scroll), 0xFFFE a new line.
- Zones: tools/unova/zones.py lists the 427 with their names: NUVEMA TOWN 389 (its houses 390-396), ROUTE 1 317
  (outdoors share matrix 0, UNOVA's one big map), CASTELIA's 32.
- Archives by number: the game's _ArcPathTable (0x020A6C58 in the decompressed ARM9, 235 entries) names archive N
  a/<N as three digits> (0xE6 = a/2/3/0). The ARM9 and overlays are BLZ-compressed: aletheia's tools/blz.py.
- Matrices: a/0/0/9 (tools/unova/matrix.py; map_matrix.c): u16 mode, u16, u16 width, u16 height, then u32 a/0/0/8 block
  per cell and, in mode 1, u32 zone ids. Matrix 0 is UNOVA's land (29 x 27 blocks); CASTELIA and others have their own.
- Map blocks: a/0/0/8, "WB" (3 sections) or "GC" (4): u16 magic, u16 count, u32 offsets (field_g3d_map_block.c
  FieldG3dMapBlockHeader): the model (BMD0, read by tools/sinnoh/nitro.py as it is), the collision grid(s), the
  placed buildings, the end. Collision (field_g3d_tile_plane.c): u16 width, u16 height, then 8-byte tiles (u16 type:2 |
  normal index:14, u16 distance index, u32 attribute:31 | diagonal:1; type 2: a split record after the grid, one plane
  a triangle); the normals (329 triples, x4096) and distances are tables in overlay 21 (0x021DB9A0 / 0x021DC158).
  A block is 32 x 32 tiles of 16 units (512).
- Areas: a/0/1/3, 282 records of 10 bytes (model set, texture set, animations, inside / outside, light type, edge
  marking, billboard colour); areas 2..0xD1 come in fours, one per season (record = area + season).
- Map textures: a/0/1/4 (BTX0 by the area's texture set).
- Animated map textures (pack version 4, 2026-11-12): an area record's ita / itp (0xFF none). ITA: a/0/6/9, an NSBTA
  each (the sea, rivers, clouds scroll: SRT translations only, no area scales); ITP: a/0/7/0, u32 count, then (u32
  animation offset, u32 BTX0 offset) each; an animation: u32 frames, u16 start times, u8 textures, u8 palettes (each
  run padded to 4), u32 loop, u32, u32 end (Black's 30 Hz frames); the map's texture is named as the first frame
  ('hana01.1'). Buildings scroll too: a definition's +0x10, u8 kind (1 all looping, 3 the season's slot, 2 doors a
  script plays), then four u32 offsets from there (BTA0 / BTP0 / BCA0 / BMA0; ROUTE 1's r01_a_spring is the falling
  petals). tools/unova/build_pack.py UnovaPack (itp_anims, ita_anims, building_scrolls) packs the frames and each
  group's scrolling (its srt); u3d.c AnimateTextures plays them (sPack.shown, srtNow). Not yet: buildings' BTP0 /
  BCA0 (machines, lamps' frames, moving parts), BMA0; `mp u3dtex` counts them.
- Buildings (field_buildmodel.c): archive 0xE5 / 0xE6 (a/2/2/9 inside, a/2/3/0 outside), member (area - 2) / 4 (area
  - 0xD2 past the seasons' areas); an "AB" offset table: n = byte 2, the first n / 2 entries model definitions (u16
  table id first), the rest their BMD0s; their textures 0xB0 / 0xB1 (a/1/7/6 / a/1/7/7, the same member). A block's
  building section: u32 count, then 16-byte records (s32 x, y, z fx32, u16 rotation in 1/65536 turns, u8 id high,
  u8 id low); the world position is x, y, -z from the block's centre.
- tools/unova/render_ref.py draws a zone from all this (SINNOH's offline rasterizer): NUVEMA TOWN comes out right.
- docs/ASSETS.md in aletheia inventories every NARC, with guesses; the code (ZONEDATA_*, MSGDATA_*, SCRCMD_*) names the
  real uses. Still to pin down from the code: the map matrix, the map containers ("WB" files in a/0/0/8: model,
  permissions, heights), area data (textures, animations, lights per season), the event data (people, warps,
  triggers), the scripts (ov010 scrcmd_*), the message banks (encrypted: msgdata.c), encounters, trainers, the
  camera tables (and Black's "rail" maps, which move the camera along a rail: ZONEDATA_IsRailMap).

## Black's words and scripts

- Words: tools/unova/build_text.py -> unova/black_text.pak (760 banks: the 288 system banks, then the 472 story banks;
  56,700 messages, about a second). src/unova/u3d_text.c reads it; a script's text is PLACEHOLDER_ID_UNOVA_TEXT +
  bank + message (7-bit bytes), Black's word slots 0-2 gStringVar1-3, 3 and up PLACEHOLDER_ID_UNOVA_VAR + slot
  (gUnovaStrVar). {BE00} is "\\l", {BE01} "\\p", {BE02:n} a wait.
- Commands: tools/unova/scrcmd.py reads the field VM's table from overlay 10 (609 commands at 0x0217062C), names each
  from the decomp's symbols (the field overlays' first: overlays share addresses) and reads its arguments from its
  decompiled body (v work-or-constant u16, p work u16, h u16, w u32, b u8; White's branches dropped). Every one of
  the 471 code files of a/0/5/7 decodes end to end with them (tools/unova/scripts.py check; file 865 is data).
- Script ids: a zone's 1.. are its file's (the zone header's script id) 0..; 46 shared ranges (ov010 0x021701A8:
  first, last, file, text arc 3, text file: 2000 the common talk scripts, 3000-6999 trainers...). A file: u32 offsets
  to FD13, script n at 4n + 4 + offset. Jumps / calls: u32 from after the offset. Comparisons push and compare on a
  stack: PushWorkValue / PushConst / CompareStack k / IfJump 0xFF (pop).
- Init files (the zone header's init id, same archive): 6-byte entries u16 kind, u32 value to 0: 1 the scene table
  (value: offset from after the 0 to u16 work, value, script triples to 0: run when the work holds the value, as
  Emerald's ON_FRAME), 2 the zone-change script, 3 the recover script, 4 the field-init script (each load: people
  placed by the story's works).
- Event work (zonedata.c): 0x3EA bytes a save: works 0x4000.. (ZONEDATA_GetEventWorkAddress), 0x16C bytes of flags
  (0x0001-0x0B5F; 0xAA0-0xB5F cleared each day), 8 bytes of temporary flags 0x4000+; 0x8000+ the script's own works.
  Object 0xFF is the player.

## The plan

As SINNOH's, in this order (each step a release once it plays, area by area: the user's call):

1. **Readers and packs** (tools/unova/, never committed output; the release builds them from UNOVA_ROM_URL):
   zone headers, matrices, map containers, area data, textures, events, text (Black's own words), scripts, sprites
   and battle pictures (unova/black_gfx.pak), music (the SDAT converted to .kaisong at build time).
2. **UNOVA's own 3D engine** (src/unova/u3d.c, include/u3d.h: never SINNOH's or ORRE's code; Platinum's renderer
   may be copied as a start): Black's map models, seasons' textures, lights, the camera with its moves while walking
   (CASTELIA, the bridges, rail maps), fitted to 240x160; billboarded sprites (UNOVA PEOPLE scaling, MIXED default).
3. **The first area, proof of concept**: NUVEMA TOWN and ROUTE 1 walked, Black's opening (JUNIPER, CHEREN, BIANCA,
   the gift box) from Black's own scripts and words; the region chosen in the new game's intro; the voyage to
   CASTELIA.
4. **Scripts** (Black's own, converted: never rewritten), the story area by area to N's Castle and the credits.
5. **Systems**: the C-GEAR (its buttons as co-op), the XTRANSCEIVER (story calls, friends' video calls), seasons
   (the room host's month online), POKéMON MUSICALS (four on stage), the ENTRALINK (Black's forest list, missions,
   PASS POWERS shareable), dark grass doubles, shaking grass / dust / ripples, TRIPLE BATTLES (rotation trainers as
   triples), Black's catch math in UNOVA, Black's battle backgrounds.
6. **Parity with the other regions**: TMs appended, items, marts by badges + GENERATION, the CARD CLUB in
   CASTELIA, the GAME CENTER's door in NIMBASA, the GARDEN's door in every center, achievements, the TRAINER CARD /
   BADGE CASE, region items, co-op joins, the GLOBAL PC, FLY, Liberty Garden after the credits.

## Rules to keep

CLAUDE.md's: UNOVA's code in src/unova/ only (never SINNOH's or ORRE's), Black's own words and scripts only, the ROM
and its packs never committed, saves never change layout, protocol bumps, player-facing release notes, releases in
batches.

## Rail maps (next)

Black moves the player along "rails" in 18 zones: CASTELIA's streets (zones 28, 30, 36), SKYARROW BRIDGE (249),
VILLAGE BRIDGE (255), the GEAR STATION (66), DRAGONSPIRAL TOWER (209, 211), CELESTIAL TOWER (338-341), VICTORY ROAD
(214), the POKéMON LEAGUE (137), OPELUCID (121), zone 114 and the ABYSSAL RUINS (241-244). ZONEDATA_GetSpecialKind
(arm9 0x0209E026, 20 u16: zone 11 bits, kind 5) gives a no-grid resource (a/1/0/9: u16 rail file, scene area file);
the rail files are a/0/7/9 (17, then their attributes in the second half): ov021 field_rail_loader.c / field_rail.c
(points 0x70: four lines, keys, widths, a position, a camera; lines 0x48: start, end, key, shape, camera, length in
16-unit cells; cameras 0x24: a function of 20 and 8 parameters; shapes 0x24: 0 straight, 1 an arc about an anchor).
Rail events (people, triggers, signs, doors with `rail` 1) are placed by line, distance and side.

Done (2026-11-12): tools/unova/rails.py lays each line's cells (its length by its width, straight or an arc) onto the
zone's tile grid with their heights; build_pack.py (pack version 2) makes them walkable field blocks (rail doors warp
tiles) and grows a matrix's cells where a rail runs past its blocks; gen_maps.py places rail people, signs, triggers
and doors at their tiles and grows the map to hold the rails. Black's map types have block sizes (ov021 sMapTypeTable:
SKYARROW BRIDGE's type 1 is 2048, zone 136's type 5 1024, the rest 512): the pack keeps the field in 512 cells and a
matrix's `scale` (u3d.c Render draws its blocks from a coarser grid, each from its first cell at its middle). "NG"
blocks (rail maps') are a model, then buildings (no grids). CASTELIA's streets walk with their people; SKYARROW
walks end to end (triangles longer than 192 units are split when the pack is made: the renderer drops one with a
corner behind the eye, and a bridge deck is a few long ones). Rail cameras (pack version 3): each rail tile carries its
line's camera (rails.py camera_at: FixedAngle and kin, the Blend kinds between the line's points' cameras, Orbit about
a centre (CASTELIA's ring road), a fixed eye and target, YawFromPoint); u3d.c eases the view's pitch and distance to
it. The view's yaw stays north up: turning it wants the d-pad and the sprites' facings turned with it (next). Not yet:
rail scripts' own commands (people moved on rails), VICTORY ROAD's negative coordinates (tiles below 0 are dropped),
signs on rails (U3D_VmSign matches grid places only). A trigger on a rail: U3D_VmPos takes the rail trigger whose work
holds its value (gen_maps.py laid its tiles out). Story: CASTELIA's first scene plays (BURGH: "That way!", work 0x40AD
1 at the ring road), the street chase (zone 31: BURGH, a grunt's battle, IRIS and BIANCA), BURGH's gym (zone 29:
his battle; its honey doors are Black's gimmick 38, u3d_vm.c PlateDoorsRefresh: the floor switches' works light plates, a door opens once both its plates are lit and its blocker person goes, walked through north or south; the doors' honey models aren't drawn), the Xtransceiver's call after it (PROC_CallValueScreen N: Black's
scripted call, a/1/3/1 member N + 62 in the scripts pack v7, its words from system bank 0xBF + N shown over the field,
faded back in from the scene's fade). Next: Team Plasma's building in CASTELIA, ROUTE 4.
