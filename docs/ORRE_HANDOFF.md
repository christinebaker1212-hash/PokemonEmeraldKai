# ORRE handoff (2026-11-07)

Pokémon Colosseum's region in Kai. **Read `docs/ORRE_DECISIONS.md` first**: the user's decisions (own engine, 2D
sprites rendered from the models, Kai's battles, Colosseum's own words and scripts, Shadow Pokémon in Orre only), the
rules, and what each tool does. This file is where things stand, what a new session needs, and what comes next.

## Where it stands
- **Rip: done and checked.** `tools/orre/make_pack.sh DISC` -> orre/orre3d.pak (174 maps, 764 textures, the grids of
  the maps in the game) and orre/orre_data.pak (text, collision, scripts, floor data, cameras, map tables, character
  models). Never committed (orre/ is ignored).
- **Shipping: on as soon as the secret is there** (the user, 2026-11-07: "Let's go ahead and ship it"). The release
  step "ORRE's packs" makes MoemonOrre.zip (78 MB) and an update.txt `orre` line once the `ORRE_DISC_URL` repository
  secret is set (the disc's link below); the game fetches the packs at launch in the background (updater.c
  OrrePreWorker) and the boat checks them before sailing (src/orre/orre_ferry.c; ORRE_DECISIONS "As their own zip").
- **Engine: ORRE's own**, src/orre/o3d.c (+ gx_tex.c, orre_maps.h, include/o3d.h), a copy of SINNOH's that goes its
  own way. Never put ORRE code in src/sinnoh/ or s3d.h (the user, 2026-11-04).
- **Every map of Orre walks, with every door** (2026-11-07, the user: "the baseline is that all maps will be able to
  be traveled to and are finished"): all 121 of Colosseum's field rooms are maps of group 45 (tools/orre/maps.py
  ORDER: map 0 the OUTSKIRT STAND, `MOEMON_DEBUG_WARP=45,0,33,40`), each walkable on Emerald's grid from Colosseum's
  collision, named from Colosseum's place names, silent until the music is converted. Their doors, staircases,
  elevators, REALGAM TOWER's lift and the ways out to the world map all come from the game's own tables (below,
  "Rooms, doors and entries"): 264 links, every one walked through by the test `orre_all_doors`; every map loads and
  stands (`orre_all_maps`); the world map travels to its nine places (`orre_all_places`). Arriving, the player
  stands where Colosseum puts them (the room's entry) facing the way it does. No people or scripts yet.
  - Two rooms only Colosseum's scripts lead into: PYRITE's windmill top (M2_windmill_2F: a person outside carries the
    player up, M2_out's script 0x20; its lift goes down to THE UNDER) and M1_carde_1 (a story scene at PHENAC
    STADIUM). They walk, and their own ways out work; they're entered once the scripts are converted (never by a
    door made up here). `mp warp 45,N,X,Y` visits any map now (N: its place in ORDER; X Y its start: orre_maps.h).
  - Story gates (collision objects the scripts switch on and off, Room.gates) are left open, so every room can be
    walked in whole; the converted scripts will close them as Colosseum does.
  - Story versions: REALGAM TOWER's outside is D4_out until flag 0x45D, then D4_out_2 (with the tower's door), as
    the world map's travel has it (worldmap.c PlaceDestRoom); PYRITE's colosseum lobby M2_enter_1F_2 is reached by
    its own links. Rooms a script chooses between (a map-local link) take the event's own arguments for now.
- **Colosseum's own world map** (2026-11-07, src/orre/worldmap.c): stepping on the lot's way out (the trigger loop,
  collision object 2: maps.py `exits`) opens it, all of it the game's own (below, "The world map"). B goes back to the
  field; A on a place travels to Colosseum's entry 0 of its room (all nine are in the game). Only the OUTSKIRT STAND shows until Colosseum's story flags are set: nothing sets them yet but
  `mp orreflag N` (src/orre/orre_story.c: below, "Story flags"). Tests: scenarios `orre_worldmap`, `orre_worldmap_pan`.
  It fills Kai's screen (the user, 2026-11-07: "fit the map to the screen, use zooms and panning"): below, "Drawn".
  No travel scene yet: the user's plan for it is below ("The bike ride").

## Resources
- **The disc** (the user's, 2026-11-04): Pokemon Colosseum (USA), GC6E01, an RVZ inside a zip:
  https://drive.google.com/file/d/1QgmyVN27VIra9hJH9xi3BNxA4_eKu24l/view?usp=sharing
  Download it outside the repository (e.g. `~/.cache/orre/disc/`), never commit or ship it. Direct download:
  `curl -sSL -o colosseum.zip "https://drive.usercontent.google.com/download?id=1QgmyVN27VIra9hJH9xi3BNxA4_eKu24l&export=download&confirm=t"`
  (make_pack.sh takes the zip as it is). main.dol's SHA-1 must be 870e8b9693ca780782d80f22a6a4572d8ba9458f.
- **The Colosseum decomp**: https://github.com/christinebaker1212-hash/pkmn-colosseum (attached to these sessions;
  upstream dougchansan/pkmn-colosseum, MIT). What ORRE's tools came from:
  - FSYS archives + LZSS: tools/fsys_extract.py, src/game/fsys/fsys_file.c; REL modules docs/REL_MODULES.md, docs/REL_M1.md
  - HSD models: include/hsd/hsd_jobj.h, hsd_dobj.h, hsd_mobj.h, hsd_pobj.h, hsd_tobj.h; scene_data src/game/floor.c
  - collision: src/game/GScolsys2Walk.c (ColTri / ColGrid), GScolsys2Thru.c (triggers at object + 0x2C), gs_colsys*.c
  - text: src/game/gs_msg.c, the control table src/game/data/data_80363774.c (msgctrlcode), handlers src/game/msgctrl*.c,
    the common messages src/rel/common_rel/message_catalog.json (docs/recon/message_catalog.md)
  - scripts (the GS VM): src/game/gs_vm*.c, tools/gs_vm_script_source.py, one bank rebuilt in
    src/rel/common_rel/common_script_0596.json (docs/recon/common_script_0596.md); floor start positions FloorPos in floor.c
- **decomp-toolkit** v1.8.3 (reads the RVZ): https://github.com/encounter/decomp-toolkit/releases/tag/v1.8.3,
  linux binary SHA-256 97c60d797aa6d87093c367d7667c77c03204e91b825b6fdd6361e29468865d76 (make_pack.sh fetches and checks it).
- **Pokémon XD's decomp** (the same engine; the Colosseum decomp's docs cite it for names): TeamOrre/xd-decomp. Not
  used yet; worth a look where Colosseum's code is unnamed (scripts, map tables).
- **SINNOH's engine** (src/sinnoh/s3d.c) only as a reference for how something was done there: copy, never share.

## A new session: setting up
1. `apt-get install -y libsdl2-dev libpng-dev`, then `make linux -j$(nproc)` (after a build,
   `git checkout mugen/processed_assets.idx`: the build rewrites it).
2. Download the disc (above), then `tools/orre/make_pack.sh ~/.cache/orre/disc/colosseum.zip` (about 4 minutes;
   `ORRE_TEST=1` adds the full check, about 9). It leaves every disc member unpacked in
   `~/.cache/moemon/orre/unpacked/<archive>/<member>.<type>` for the other tools (gen_maps.py, grid.py, gridview.py).
3. Check: `NOBUILD=1 tools/test/regress.sh orre_outskirt_stand`, `tools/orre/test_gxtex.sh`,
   `NOBUILD=1 tools/test/orre_download_test.sh orre`, and `python3 -I tools/orre/test_pack.py ~/.cache/moemon/orre/unpacked orre`
   (about 5 minutes; **not yet run since the pack gained the grids**: run it first).
4. Looking at a map: `python3 -I tools/orre/pak.py orre render MAP out.png 50` (a quick render),
   `python3 -I tools/orre/gridview.py MAP.map MAP.col out.png 12 X Z` (the grid over a top-down picture; X Z = the
   start in Colosseum units), `python3 -I tools/orre/pak.py orre text S1_out/S1_out.msg` (a map's words).
5. SINNOH's scenarios need its pack: `tools/sinnoh/make_pack.sh` (fetches pret/pokeplatinum). Run a few of them
   after touching any hook both engines share a file with (gba_easy_draw.c, sprite.c, overworld.c, fieldmap.c,
   field_camera.c, maker_overrides.c): 2026-11-04 sinnoh_platinum_walk, sinnoh_doors, sinnoh_reflection,
   sinnoh_feet_row, sinnoh_lift_iron and sinnoh_gym passed with ORRE's hooks in.

## Next steps (in order)
The road to the end, agreed with the user 2026-11-07 (their answers: ORRE_DECISIONS "The road to the end"):
1. **Story state: done** (below, "Story flags"). 2. **Colosseum's text: done** (below, "Colosseum's text").
3. **Trainers: done** (below, "Trainers and battles"). 4. **Colosseum's scripts: running** (below, "Colosseum's scripts": its GS VM in the game; more natives as each
   part needs them).
5. **People on the maps: done** (below, "People on the maps"). 6. **Shops, POKéMON CENTERs, the PC: done** (below).
7. The opening (the player as the thief, the SNAG MACHINE), then the ride to the OUTSKIRT STAND: **started** (below,
   "Colosseum's cameras" and "The opening"). 8. RUI follows the host.
9. The colosseum challenges (PHENAC, PYRITE, THE UNDER, DEEP COLOSSEUM, REALGAM TOWER). 10. Story scenes, faithful.
11. Shadow Pokémon's open items (SHADOW_POKEMON.md "Still open"). 12. Music (alongside 4). 13. The bike ride.
14. MT. BATTLE and REALGAM TOWER's post-game, their rewards. 15. Kai's systems (POKéDEX, the card's star,
   achievements, the P★DA as ORRE's device, the download screen). 16. A full playthrough test, then a release.
The older notes per part:
0. **Shadow Pokémon come before the story** (the user, 2026-11-07): their design and the user's decisions are in
   `docs/SHADOW_POKEMON.md`; its open questions are settled with the user as each part is built. Built: the data, the
   gauge, battles, HYPER MODE / CALL, the aura, snagging, purification, the battle confinement (SHADOW_POKEMON.md
   "Built so far"). Next (the user agreed, 2026-11-07): the PC and trade confinement, then the people (step 2, 4).
   **ORRE is a post-game place reached by boat** (ORRE_DECISIONS.md "A post-game place": the ORRE TICKET after any
   ELITE FOUR, HOENN's ferries and every port, landing at the OUTSKIRT STAND by the sailor: src/orre/orre_ferry.c).
1. **Music** (the user, 2026-11-07: "Colosseum's own world map screen, then convert the music"): see 6.
   Then the world map's own song (worldmap_song) and the travel scene: the user's plan, "The bike ride" below.
2. **Where people stand: read** (2026-11-07). tools/orre/colo_rooms.py gives each room's `people` (the floor record's
   + 0x14 slot: 36-byte records, flags (visible 0x80), the angle at + 4, the people info row at + 6, the name at + 8,
   the move / talk scripts at + 0x10 / + 0x14, x y z at + 0x18) and `people_info()` each row's model file (common_rel
   + 0x13E440, 0x2C each, the u32 at + 0xC a file id: ken_b1.dat the hero, hizuki_b1/_b2.dat RUI, bag_b1.dat RUI's
   sack in PHENAC...): 757 people in the field rooms, 116 models. Many start hidden (the OUTSKIRT STAND's all are:
   the room's script shows them by the story), so placing them waits for the scripts (step 5). Next: their sprites
   (step 4).
3. **Maps: done** (all 121 field rooms). The stadiums' battle stages (kind 2 rooms: M1_water_colo, M2_earth_colo,
   M4_cylinder_colo, M4_bottom_colo, T1_ancient_colo) aren't maps: Kai's battles. The `_ev` and `_bf` copies aren't
   either (a scene's or a battle's).
4. **Sprites.** Render the character models (orre_data.pak, `.dat`) into GBA-style sprite sheets (the user's
   decision). Dry run shown to the user 2026-11-07 (WES 16x32, RUI 32x32 for her pigtails; awaiting their verdict):
   tools/orre/hsd_anim.py reads the joint animations (the model set's + 4 list; keyframe values little-endian) and
   solves the legs' two-bone IK (JOINT1 / JOINT2 / EFFECTOR joints, IK hints = bone lengths, the effector's RObj names
   the target joint); every field model's table starts idle 1, walk 2, run 3. tools/orre/people_sprites.py renders
   them (head joint scaled for GBA proportions, 15 colours, outlines); people_sprites_demo.py draws the sheet.
   **ORRE's people (built 2026-11-07, the user's calls): every Colosseum character model is a LOOK > CREATE recipe
   (its field sprite) and a Pokemon Showdown trainer sprite chosen to look like the model (its battle picture, shown at
   DS size, 80x80, as drawn; fan art never recoloured: 8 bpp where it has more than 15 colors).** The renders above
   are reference only. tools/orre/people.py: the PEOPLE table (styles and pictures picked by eye from each model's
   render, hair colours by eye, the top / bottom colours sampled from the model), `build` writes
   src/data/orre_people.h + graphics/orre/people_pics.bin (Showdown's art, committed like Kai's other Showdown
   pictures; credits in dist_extras/README.txt), `sheet OUT.png` a review sheet (needs the game built). The game:
   src/orre/orre_people.c (include/orre_people.h): object graphics OBJ_EVENT_GFX_ORRE_FIRST (0xF00) + person, palettes
   tag 0xBC00 + person (event_object_movement.c OrreObjectInfo, built from mp_creator.c MP_Created_BuildField); the
   battle picture by ORRE's own line after SINNOH's in the battle controllers, for the foe a script names first
   (special Orre_SetFoePerson: VAR_0x8004 person, VAR_0x8005 A / B; in ORRE only, cleared when the battle ends).
   92 people (agent_m_b1-b6 the Cipher Peons' colours), 73 pictures; the Pokemon and props in the people tables
   (pikachu, casey = ABRA, makunoshita = MAKUHITA, truck, bag...) aren't people: they'll be POKeMON objects / props.
   Debug: `mp orrenpc PERSON [DX DY]`, `mp orrefoe PERSON [1]`, `mp createddump HEX DIR`. Scenario orre_people.
   Placed on the maps since 2026-11-08 (below, "People on the maps").
5. **Words and scripts.** The map scripts (.scd, the GS VM) converted to Kai's (as tools/sinnoh/gen_scripts.py does
   SINNOH's); the text is done (above, "Colosseum's text": a line is its id's placeholder, the words it fills in ORRE's
   string slots). Never write ORRE's words or scripts by hand.
6. **Music** (the user, 2026-11-07: converted, it can't be found online; next): the MusyX projects in common.fsys
   (snd_music_proj / _pool / _sdir) and the songs (0x08 members), to .kaisong; the decomp's MusyX library is the
   reference. Each map gets its song as its song converts.
7. **Snagging and Shadow Pokémon**, in Orre only (state in a sidecar: saves never change layout; Colosseum's story
   flags, src/orre/orre_story.c, go there too).
8. The in-game download screen when orre/ is missing (as the DANCE machine's rh_screen.c StateNoSongs).

## Colosseum's scripts (src/orre/orre_vm.c, 2026-11-08)
- **Run, not translated** (Claude's call, ORRE_DECISIONS "Colosseum's scripts run as they are"): the game has Colosseum's
  GS VM, opcode for opcode from the decomp (src/game/gs_vm.c: operand descriptors 0x80 on the stack / 0x40 a frame
  variable / else a global, 0x20 / 0x100 an address; low six bits the type, 2 int else float; native frames: the
  result's slot, the native's index, the arguments, then the frame and the count; script calls and returns as
  cms_op04 / cms_op05; the first frame as cms_ScriptCreateThread). Addresses a script takes are tagged indices
  (VM_REF_*) and code offsets bank-relative, so nothing depends on the PC's pointer size. Banks: the room's .scd
  from the data pack (id 0x0100) and the shared bank 0x0596 (common_rel .data 0x89CFC, 8120 bytes).
- **Rooms**: Colosseum's floor record (common_rel .data 0x8BCF4, 0x4C each; + 0xC the room id) holds + 0x28 the sound
  script, + 0x2C / + 0x30 the set-up (who's shown), + 0x34 the main loop (the default 0x05960003: the shared bank's
  hero loop), + 0x38 what runs on leaving. Arriving (overworld.c LoadMapFromWarp -> Orre_Vm_Warped; the next
  O3D_Update frame), the set-up runs through and the main loop starts; every overworld frame the threads run until
  they wait (none while the screen fades: a scene waits for the field to be back). The main loop watches the story
  flags and the player (std058_characterCheckWithinBounds of person (0, 100), the hero: O3D_TileToColo gives the
  player's place in Colosseum's units) and starts the room's scenes.
- **A scene takes the player** (heroMoveInitEvent): Kai's script data/scripts/orre.inc Orre_EventScript_VmHold locks
  the field (started as soon as Kai's script engine is free) and waits (special Orre_VmHold); its battles start while it
  waits (fieldStartEncount / fightStartEncount: orre_battle.c), the battle's end hands the thread its result
  (Orre_VmBattleDone; std103_battleGetResult: 2 the player won, as Colosseum's scripts compare), the hold goes on
  while the scene still holds (specialvar Orre_VmHolding: VAR_RESULT isn't safe across a battle) and ends with
  heroMoveTermEvent.
- **Natives converted** (tools/orre/natives.json, from the decomp's table lbl_802E1CF0, names its indices): the flags
  (78 sets 1, 79 sets 0: the decomp's names are swapped; 80, 81, 82), messages (54 characterTalk, 7 / 8 winMsg
  open / close, 55 end of speech: Colosseum's line by id through orre_text.c, waiting for A where Colosseum's wait
  says), 11 YES / NO (0 YES), 132 msgctrlSetValue (a control's value made words: names by message, items, species,
  moves, numbers), 157 sleep and the yield, 58 the hero in a box, 92 / 93 room and group, 102 / 152 battles, 103 the
  result, 31 / 32 / 33 the hero, the people (below, "People on the maps"). **Not yet**: 51 animation (a sprite has
  none of the model's poses), the camera (36 / 37 / 40 / 41 / 140 / 141 / 179: Platinum's camera stays,
  maybe never), sounds and music (104 / 105 / 109: with the music), story gates (118 scriptSetCol), map objects
  (114 / 115), items and money, menus, shops, the PC; the rooms' event triggers (the floor's events: pass / touch /
  check) start scenes too. An unconverted native logs once
  ("native N not converted yet") and does nothing.
- **On in players' games** (2026-11-08, with the opening built): the rooms' scripts run; MOEMON_ORRE_SCRIPTS=0 turns
  them off (the maps alone: the maps' and Shadow Pokémon's own tests set it). MOEMON_ORRE_VM_TRACE=1 logs every native call and the threads' state.
- Tools: `python3 -I tools/orre/gsvm.py ROOM.scd [FUNC]` lists a bank (natives named, their constant arguments; a
  `pushn` argument, from a frame variable, shows as ?);
  `gsvm.py common UNPACKED` the shared one. Debug `mp orrescript BANK FUNC` runs a function as a scene. Scenario
  orre_scene_willie: WILLIE's scene at the OUTSKIRT STAND (flags 986 / 988 set: past the arrival) from the main loop,
  his lines, YES / NO, battle 10, his loss line, the flags it sets (990, 2650, 991).

## People on the maps (src/orre/orre_npcs.c, 2026-11-08)
- **Each room's people are Colosseum's own**: tools/orre/gen_people.py (`python3 -I tools/orre/gen_people.py
  [UNPACKED]`, from the unpacked disc: run it again after colo_rooms.py or people.py changes) writes
  src/data/orre_room_people.h, committed: 757 rows {room, index, flags, talk types, angle, ORRE person, name row,
  move / talk scripts, x, z}. The person is people.py's for the row's model (0xFFFF: a POKeMON or a prop, not placed
  yet: truck_b1, casey, pikachu...; hizuki_b2, RUI's other clothes, is RUI).
- **On the map as Kai objects**: local id 0x40 + the index (the maps' own objects stay below), graphics
  OBJ_EVENT_GFX_ORRE_FIRST + person, standing on the tile of Colosseum's x / z (O3D_ColoToTile: 8 units a tile),
  facing its angle (0 south, 90 east, 180 north, 270 west), MOVEMENT_TYPE_NONE.
- **Who shows: Colosseum's order** (floor.c _floorInitialize): the room's first set-up (+ 0x2C, run at once) may
  change a person's 0x80 (scriptFloorCharSetDisp, native 100: it writes Colosseum's table in memory, so it holds for
  the session: sDisp), then the people are placed (Orre_Npcs_Place: those with 0x80), then the second set-up
  (+ 0x30) and the main loop. At the OUTSKIRT STAND no one shows until the story does: WILLIE (3) once flag 990 is set.
- **Natives**: 50 peopleSetVisible / 56 peopleOpen / 57 peopleCloseByID (now), 59 position, 60 rotation, 68 turn,
  165 / 181 face a person, 65 look at the player, 61 moveToPosition and 177 moveToCharacter / 232 (a tile short of
  them) as Kai's scripted movement (along x, then along y: the walk goes on while the field is locked), 62
  peopleMoveCheck (its wait holds the thread until the walk is over: W_MOVE). The hero is (0, 100): their position
  is never set by a script (Kai's camera follows the player), their turns and walks are. 54 characterTalk turns the
  speaker to the player as Colosseum does (the record's talk start type, byte + 1 >> 6: 1 the head, 2 the body), 55
  end of speech turns them back (always for 1, for 2 when the end type, >> 4 & 3, is 1).
- **The speaker's name** (std054): the name table row (common_rel .data 0x1A34, 8 bytes: a hear flag, the name's
  message): its name once the flag is set (or with none), else "???" (0xFA3), into control 0x59.
- **Talking to them**: A in front of a person with a talk script (field_control_avatar.c
  GetInteractedObjectEventScript -> Orre_Npc_TalkScript) runs data/scripts/orre.inc Orre_EventScript_Talk: lockall,
  special Orre_VmTalk starts the talk script with Colosseum's arguments (1, index, 0, 0) as the scene holding the
  field, then the hold loop.
- Logs: "orre npcs: room R, N people, M shown", "N shown at X,Y", "N walks K steps to X,Y", "N walked to X,Y".
- **Not yet**: the people's move scripts (+ 0x10: wandering, set-up per person), the record's move types (2 wander,
  3...), look-at-the-player while near (flag 0x20: a sprite would turn as the player passes), animations (51), the
  floor's other objects (floorInitPeople: treasure and props), POKeMON and props as objects, more than Kai's 16
  objects on a map at once (a room has up to 48 people: those past the limit don't show).
- Scenario orre_scene_willie: WILLIE shown by his scene, walking in and stopping two steps south of the player (the
  test puts the player at 25,28, where Colosseum's hero stands for it), the battle, then the room's set-up showing him
  at his place and A in front of him running his talk script (0x0100000D: 20790).

## Shops, POKéMON CENTERs, the PC (2026-11-08)
- **Colosseum's own, through Kai's screens.** Nothing is written by hand: the nurse, the clerks and the PCs run their
  own scripts (orre_vm.c), and the natives open Kai's screens.
- **The nurse** (pc_b1, a person with a talk script: 15102 "Hello, and welcome to the POKéMON CENTER...", YES / NO,
  15104, 15106, 15108). Her std083 (Colosseum's pokemonCenterBallMachineSequence / HealSequence) heals the party
  (HealPlayerParty) with Kai's healing jingle (W_FANFARE). Those lines are in a **second message table of
  common_rel** (.data 0x647B4, 684 lines, ids 1-17646: the menus' and the centers' words), now in the text pack too
  (colo_data.py TEXT_BLOCK2, common_text).
- **People behind counters** are talked to across them: on an ORRE map, A on a blocked tile reaches the one past it,
  as Kai does over counters (field_control_avatar.c GetInteractedObjectEventScript; Colosseum reaches people a little
  way off: peopleFindTalkTarget).
- **Check spots** (Colosseum's floor events of kind 3: the PCs, signs, TVs, machines): tools/orre/gen_checks.py writes
  src/data/orre_room_checks.h (132 rows: the room, the collision id, the script and its four arguments, the box of its
  check triangles: each .col collision object's + 0x30 slot, a triangle's id at + 0x30; the trigger slot + 0x2C keeps
  its id at + 0x32). A with a spot's box half a tile around the point 6 units in front of the player's tile
  (orre_npcs.c Orre_Check_InFront, after the people) runs data/scripts/orre.inc Orre_EventScript_Check: special
  Orre_VmCheck starts its script with its arguments (heroDispatchEvent's cmsRunSync) holding the field.
- **The PC**: the shared script 0x0596000D warps to the menu room 913 (pcbox_menu); std098_mapWarpToMenuRoom(913) opens
  Kai's own PC (EventScript_PC: the boxes, the items, the GLOBAL PC), and turning it off goes back to the scene
  (pc.inc EventScript_TurnOffPC: specialvar Orre_VmHolding). The other menu rooms (the bag, the P★DA, the name entry...)
  log "menu room N not converted yet".
- **Shops**: menuShopOpen(N) opens Kai's POKéMART with Colosseum's list N. tools/orre/gen_shops.py writes
  src/data/orre_shops.h from pocket_menu.rel (its _prolog publishes the shops at .sbss 0x80478E54: u8 name, u8 kind,
  u16 the list's start; the lists at 0x80478E44, 0-ended; the count at 0x80478E50): 14 shops. Items are Kai's numbers
  to 348; Colosseum's own past it (its SCENTS 543-545) are left out until Kai has them (SHADOW_POKEMON.md:
  purification). Kinds 2-4 are the POKé COUPON counters: not opened yet (the coupons aren't converted). On ORRE's maps
  the BUY screen shows the field's last 3D picture (o3d.c O3D_ShopPicture, ORRE's own copy of SINNOH's).
- **Screens from a scene** (W_SCREEN): the thread asks for a screen; while the hold script waits, specialvar
  Orre_VmScreen tells it which (1 the shop: special Orre_VmShop, waitstate; 2 the PC) and back in the hold loop the
  thread goes on.
- Natives also converted: 133 / 134 money (scriptAddPokedoru / GetPokedoru), 120 / 219 items (scriptHaveItem /
  scriptAddItem, to 348).
- Scenario orre_center: PHENAC's POKéMON CENTER (45,3): the nurse heals (HP 10 -> full), the PC opens and closes;
  PHENAC's mart (45,8): a POTION bought (¥3000 -> ¥2700).
- Seen, not yet looked into: PHENAC's mart floor draws as blue speckles (a texture of that room's model).

## Colosseum's cameras (src/orre/o3d_cam.c, 2026-11-08)
- **A shot** (.cam, in orre_data.pak as the disc's bytes; resource ids in index/ids.txt) is an HSD scene: root
  "scene_data" = SceneDesc {model sets, **the camera set** (one, not a list), light sets, fog}; the camera set =
  {CObjDesc, CameraAnim list}. CObjDesc (perspective): class name, u16 flags, u16 projection, 8 u16 viewport and
  scissor, the eye's WObjDesc + 0x18, the target's + 0x1C (WObjDesc: class name, f32 x y z), roll + 0x20, up + 0x24,
  near + 0x28, far + 0x2C, fov + 0x30 (vertical, degrees). CameraAnim: its AObjDesc (tracks 9 roll in radians, 10 the
  field of view, 11 / 12 near / far), the eye's WObjAnim, the target's (each an AObjDesc first: tracks 5 6 7 x y z;
  4, rare: a path, not read yet). FObj keys as hsd_anim.py decodes them (little-endian values). 60 frames a second.
- **In ORRE's engine** (its own code, never SINNOH's): O3D_CamPlay / PlayId / Stop / Done; while a shot plays,
  o3d.c ShotView looks through it (Colosseum's units into the map's world: (x - x0, y, z - z0) * 16 / tile, the
  picture's centre the screen's, the vertical field of view over 160 lines); people stand where their tiles project.
  A shot ends with its room's scripts (orre_vm.c Orre_Vm_Stop) or the room (unless kept: the opening's).
- **Natives**: 42 cameraPlayAnime (group, shot id, frame, loop), 43 / 44 stop, 179 cameraReturn (the field's camera
  back), 46 cameraWaitSyncAnime (W_CAMERA: until it has played). **Not yet**: 40 / 41 / 140 / 141 the camera set and
  moved by position and rotation (their rotation's convention to be checked against a scene), 36 / 37 the scene's
  mode, 170 focus on a character.
- Tools: `python3 -I tools/orre/hsd_cam.py FILE.cam [STEP]` prints a shot frame by frame. Debug `mp orrecam NAME|ID
  [FRAME]`, `mp orrecam stop`. Scenario orre_camera.
- **Skies during shots only** (the user, 2026-11-08: "Draw Colosseum's own skies during story scenes and keep the
  field view as it is"): the pack's sky meshes (model sets of kind 2) load with the room (OMesh.sky); o3d.c's Render
  draws them first, depth cleared after, only while O3D_CamActive(); the field camera never shows them.

## The opening (step 7: found so far, 2026-11-08)
- Colosseum's opening is **its C code, not scripts**: the decomp's demoScene00-30 (src/game/gbaCommunication_*.c,
  table lbl_802EEC70 of 32), each a shot: map-object animations (std115_mapObjectPlayAnimation: the explosion's
  model 0x0B631000 and its particles .ptl), the characters' skeletal animations attached to their models
  (std070_characterAttachToCharacter: opening_characters_NN.dat's animations on snach_zako (the Cipher Peons),
  boss_s (GONZAP), ken_c1 (the hero)), cameraPlayAnime(cam_opening_NN), waits, then floorRequestChange(next room:
  0x82, 0x83, 0x87, 0x89...). The rooms' own scripts (S2_out_1, S2_hallway_1, S2_bossroom_1, S2_snatchroom_1) do
  nothing but wait.
- **Those four rooms aren't Kai maps yet** (maps.py ORDER has the 121 field rooms: S2_out_2, S2_building_* only); their
  models are in orre3d.pak. To play the opening they need maps (or a scene mode drawing a room without one).
- **Built (2026-11-08): src/orre/orre_opening.c.** The 31 scenes in order (demoScene00-30: each scene's room is the
  one its shot's archive is in, S2_out_1 -> S2_bossroom_1 -> S2_out_3 -> S2_snatchroom_1 -> S2_hallway_1 -> S2_out_3's
  19-shot bike escape; the last asks for room 1, the OUTSKIRT STAND), each shot played by its resource id and waited
  to its end, cuts within a room straight on, a new room through black (floorRequestChange's fade), START skips.
  The rooms aren't maps: ORRE's engine draws a **scene room** (o3d.c O3D_SceneRoom: any room of orre3d.pak by name, its
  grid if it has one, else Colosseum's units at the tile scale) instead of the field's map while a shot plays, the
  field's sprites hidden (OamHidden); triangles crossing the near plane are cut, not dropped (DrawClipped: a shot's
  ground passes under its camera). At its end the SNAG MACHINE (ORRE_FLAG_SNAG_MACHINE). It plays on the first
  landing at the OUTSKIRT STAND without the SNAG MACHINE while ORRE's story is on (data/maps/Orre_OutskirtStand/
  scripts.inc: ON_TRANSITION Orre_OpeningNeeded, ON_FRAME Orre_EventScript_Opening; Kai's glue, no words: the
  opening has none). Tests that start after it set MOEMON_ORRE_NO_OPENING=1. Scenario orre_opening.
- **Not yet**: the characters (the hero as the player's look, GONZAP, the peons, the bike: their models' root paths
  from opening_characters_NN.dat as sprites), the explosion's map-object animations and particles.

## The story runs (2026-11-08: toward a finishable ORRE)
- **More natives** (orre_vm.c): 95 floorLink / 125 floorLinkWithSE (a script's room change: o3d.c O3D_RoomEntry finds
  room R's entry K, the K-th of its map's sOrreArrivals; the hold script warps: SCREEN_WARP), 94 floorGetPrevFloorID,
  195 floorGetNextPosIndex (the entry come in by: floorLink's, else the arrival tile's), 180; 199 the choice menu
  (the script's array of message ids through its address; Kai's ScriptMenu_MultichoiceList; B the last row), 245
  YES / NO; 119 scriptGetItem (Colosseum's item fanfare and lines 0x3CB8 / 0x3CBD, a full bag to the PC 0x3CBA / BB);
  Colosseum's own items past 348 (its key items) as Kai-side ORRE flags (ORRE_KEY_ITEM_FLAG: 120 / 219 too); 131 the
  "!" over someone; 72 / 73 / 74 walk lists (looped ones go round: Orre_Npcs_Frame); 172 a person's position written
  through the script's address; 169 the person sees the player (right in front of them); 71 yield; 3 scriptExecTask
  (a thread alongside, its handle for 6 std006_threadWaitForCompletion), 209 (a script run to its end first);
  36 GSscene_SetMode(0) / 37 cameraSetTarget: the field camera back. A shot played by a thread that doesn't hold
  the field holds it until then (Colosseum's hero only moves when the main loop calls heroMoveMain).
- **Quiet natives** (sQuiet: what they return, nothing shown yet): sounds and music (until it's converted), the
  scene's own camera moves (40 / 41 / 140 / 141 / 127 / 170...), models' animations, lights, people's internal flags
  and walking speed, story gates (118: left open), doors and elevators (the maps' links), mail, the TV.
- **The leave script**: the floor record's + 0x38 runs as the player warps out (Orre_Vm_Warped): the diner's sets 988.
- **Touch spots**: gen_checks.py also writes sOrreRoomTouches (28: floor events of kind 2 but the doors' and
  elevators' shared scripts 5 / 6 / 12; the trigger slot + 0x2C, ids at + 0x32); stepping onto a new tile inside
  one runs its script holding the field (orre_npcs.c Orre_Touch_Frame, Orre_EventScript_Touch).
- **Tests walk as players do**: the harness's `walkto X Y [FRAMES]` holds the D-pad toward a tile by the shortest
  way (mp_debug.c MP_TestWalkKey), stopping when a script takes the player ("stopped (a script)").
- Scenario orre_story_stand: the landing to WILLIE's scene. The chain: walking near the stand (the main loop's box)
  plays the arrival (function 8, flag 986); the diner's news; leaving it (988); WILLIE (function 9).
- **The story, step by step** (tools/orre/story_route.py: which room function sets each story flag and how it's
  reached; story_order.txt: the game's `mp orreorder`): `python3 -I tools/orre/gen_story_tests.py
  tools/orre/story_order.txt` writes **orre_scenes_NN** (25 steps each, every step on its own from `mp orrestory`:
  probes, not committed, many need what earlier steps leave; tools/orre/check_scenes.py lists the ones that didn't
  set their flag) and **orre_story_chain** (every step in order from the landing, the story never jumped: the harness's
  `skipif orreflag:N K` passes over a step the story already did; in the room already, it walks (`skipif inmap:G,N`, `walkto`) instead of warping in again, which would set the room up afresh; at a main loop's step it presses A to each side, as a player looks around; the first "TIMEOUT waiting for log orre: flag N"
  is where the story stops). Flags change in the log with MOEMON_ORRE_FLAG_LOG=1 ("orre: flag N = V").
- **A walk that can't start** (a scene cut off mid-walk still holds its movement slot) puts the person where the
  walk ends, so the scene waiting for them goes on (orre_npcs.c WalkTo).
- **The RELIC STONE**: 194 preReliveSetParameter(0, 0) opens Kai's purification (party_menu.c Orre_RelicStone, the
  hold script's screen 5); 242 the party's Shadow Pokémon whose heart is open. MT. BATTLE's rest stops: 229 leaving
  the course (back to its reception, room 75); 223 save and quit, 236 / 239 POKé COUPONS: quiet (Kai saves any time;
  the coupon counters aren't open yet). Quiet too: 225 (the party noted for a challenge), 246 (the save's continue
  point). Left: the day care (84 / 85), a move tutor (86), 87, the e-Reader's DNA samples (203-208, Japan's), the GBA
  link (200-220), the P★DA and pocket menus (Kai's own).
- **Props**: a room's "people" without a character model (RUI's sack in PHENAC, the hunters' truck, POKéMON with no
  sprite yet) are placed unseen (an invisible object), so they block and can be examined: PHENAC's story needs the
  sack examined (its talk script, function 33, arms the main loop's scene).
- **16 object slots** (the save keeps 16 object events): a room's shown people have objects only near the player
  (±10 x ±8 tiles), the nearest first; far off they keep their place and facing in their record (orre_npcs.c
  Orre_Npcs_Frame), and anyone missing after a battle or a screen comes back.
- **RUI follows** (step 8): Colosseum's scripts call 35 std035_setPartnerFollowing (heroSetStatus 0x18; the partner
  is their character 101, the hero 100). Kai keeps it as ORRE_FLAG_RUI_FOLLOWS (0xFFE) and shows RUI (person 1,
  hizuki_b1) as character 101 with MOVEMENT_TYPE_FOLLOW_PLAYER (the walking partner SINNOH's BARRY uses: the tile the
  player left; Kai's POKéMON follower walks behind her), placed first on every room so she has a slot, back after
  battles; scenes move, turn and place her as 101 (orre_npcs.c PartnerAppear, Orre_Npc_PartnerFollow). Scenario
  orre_rui (PHENAC: the hunters' battle, the sack opened, RUI joins). Co-op (she follows the host on every screen):
  not yet; each player's own game shows her behind them.
- **Still to do for a finishable ORRE**: the colosseum challenges (std223 then menu room 917: not converted), the
  rest of the story room by room (probe with MOEMON_ORRE_VM_TRACE=1 for "not converted" natives), the opening and
  RUI (steps 7, 8), then scripts on for everyone and a full playthrough test.

## Trainers and battles (src/orre/orre_battle.c, 2026-11-08)
- **Read at run time** from common_rel in the data pack (nothing generated or committed): battles (fightEncount: .data
  0x109FBC, 0x38 each, 566), trainers (0x91684, 0x34, 820), classes (fightTrainerKind: 0x8F724, 0xC, 41: + 0 the prize
  rate, + 4 the name), trainer Pokémon (0x9E5DC, 0x50, 5510; moves at + 0x36 + 8n, 0 = the level's own moves). A battle
  record: + 0 kind, + 1 trainers a side, + 2 zenmetu (a loss is game over: Kai's whiteout), + 4 field, + 8 name, + 0xC
  song, + 0x18 four slots {u16 trainer, u32 1 = the player} (the first "a side" slots the player's side).
- **Fought as Kai's trainer battles** through two placeholder trainers after SINNOH's, TRAINER_ORRE_A / _B (2221 /
  2222, class TRAINER_CLASS_ORRE 107), filled before each battle in a writable copy of the trainer table (gTrainersTable;
  the Moemon Maker's patched copy is copied as it is, and the Maker's copy now covers every trainer id:
  gTrainersBaseCount; it stopped at TRAINERS_COUNT 855 before, past which JOHTO / KANTO / SINNOH's trainers were read
  out of bounds while the Maker had trainer edits). Name and class name from Colosseum's text (battle_message.c asks
  Orre_ClassName), up to four of its items, Doubles always (ORRE_DECISIONS), the team by battle_main.c's
  CreateNPCTrainerParty asking Orre_Party: Colosseum's natures, genders, IVs, EVs, Ability slots, moves, balls, held items
  (Shadow Pokémon by orre_shadow.c Orre_Shadow_MakeEnemy), raised by level scaling as any trainer's. The prize is
  Colosseum's (fight_range_exact_8021D9C0.c): the class's rate x the highest level x 2 in a Double Battle.
- **The picture** is the person a script names (the trainer records' model ids are Colosseum's 3D battle models, not
  the field people): special Orre_StartBattle takes VAR_0x8004 the battle, VAR_0x8005 the person (0xFFFF none).
  battle_setup.c BattleSetup_StartOrreBattle / CB2_EndOrreBattle: no trainer flags (Colosseum's scripts keep their own),
  a loss whites out only where zenmetu says, else the script goes on; Orre_BattleResultToVar: VAR_RESULT 2 won, 1 lost (Colosseum's numbers)
  (ORRE_BATTLE_*; what the scripts' std103_battleGetResult will answer).
- Debug: `mp orrebattle BATTLE [PERSON]`, `mp orreresult`; scenario orre_battle (WILLIE, battle 10: RIDER WILLIE, two
  ZIGZAGOON at Lv 24, the prize 240).

## Colosseum's text (src/orre/orre_text.c, 2026-11-08)
- **The pack**: orre/orre_text.pak, made by tools/orre/build_text.py from the unpacked disc (make_pack.sh runs it;
  package_orre.sh ships it in MoemonOrre.zip beside the other two; never committed). Every .msg file plus common_rel's
  own block (place, item, move and POKeMON names): 8,229 lines. Colosseum's message ids are **one numbering across its
  files** (6,726 ids; an id in several files is the same line copied into each module's archive; the eight that differ
  are menu copies and the developers' move viewer, which goes last), so the pack is one table by id.
- **Converted**: Kai's character set with MOeMON's words (moe_words.py, as HOENN and SINNOH); "‱" -> "???" (a speaker not
  yet named), "$" -> Kai's money sign (Kai's "$" is EOS!), straight quotes -> curly, the P★DA's star -> "·" (no star in
  Kai's font); Japanese lines (633, cut events) left out ("Message missing"). **Re-wrapped** to Kai's box (200 px of the
  normal font): {wait_key} is a new box (Kai's page); within a box a line ending a sentence stays a break and the rest
  flow; two lines a box, then the box scrolls. Speaker names stay in the line as Colosseum writes them ("WILLIE: ...").
- **Placeholders** (include/constants/characters.h): PLACEHOLDER_ID_ORRE_TEXT 0x44 + the id as three 7-bit bytes
  (build_text.py orre_text_bytes, Orre_TextPlaceholder); {hero} the player's name; {rui} "RUI"; every other word the game
  fills in is ORRE's string slot PLACEHOLDER_ID_ORRE_VAR (0x80) + Colosseum's control id (msg.py CONTROLS: 89 npc the
  speaker, 45 item, 50 pokemon, 47 digit, 75 money, 13-14 str0 / str1...): Orre_SetStrVar(ctl, words) is what the script
  converter will emit where Colosseum's scripts fill them. {wait:N} is Kai's PAUSE; fonts, colours, sounds go.
- Debug: `mp orretext ID` (the line in a message box, as text in the log), `mp orrevar CTL WORDS`; scenario orre_text;
  `python3 -I tools/orre/build_text.py UNPACKED OUT.pak --dump out.txt` writes every converted line as text.

## Story flags (src/orre/orre_story.c, 2026-11-07)
Colosseum's GS flag module (the decomp: gs_range_8018FE30.c, gs_flag_exact_801908D4.c, gs_flag_get_exact_801906A0.c):
3593 flags defined in common_rel .data 0x111E04 (count and the story order's head at 0x14A6C0), 8 bytes each: type << 6
| width (1-32 bits), the story jump's value index (.rodata 0x08: 0, 0, 1, 1, 9, 1023), an item swap (.rodata 0x00:
{u16 add, u16 remove}), an event (.rodata 0x5C, 24 bytes: party members, P★DA money, a room and a position), the bit
position (laid out at load: GSflagInitBitPos), the next flag in the story's order. Three levels (1: 40 bits, 2: 207,
3: 6414), all three in Colosseum's save. The game reads the table from the pack (common/common_rel.rel) and keeps the
values in "<save>.orre" (format 2; format 1, one bit a number, converted on load). Numbers from 0xF00 are Kai's own
(ORRE_KAI_FLAG_FIRST: the SNAG MACHINE 0xFFF). **The decomp's setter names are swapped**: std078 ("flagSetFalse")
sets 1, std079 ("flagSetTrue") sets 0: the script converter must follow what they do. Colosseum's story jump
(fn_8018FE30: the flags before a flag in the story's order set to their values, it and the rest cleared) is
`mp orrestory N`, for tests that start at a story point. Debug: `mp orreflag N [VALUE]`, `mp orreget N`. Scenario
orre_story.

## Open questions for the user
- Answered 2026-11-08: Colosseum's skies show during story scenes' shots only; the field view stays as it is.
- Answered 2026-11-07 (docs/ORRE_DECISIONS.md): shipping waits until ORRE is completable and full; the music is
  converted from the disc and there is none until then; map names come from Colosseum's areas; the tile scale stays 8.

## The game's own tables (tools/orre/colo_data.py)
- **The floor table**: every room, in common_rel's .data at 0x8BCF4 (196 records of 0x4C bytes, count at .data +
  0x14A670; the decomp's src/rel/common_rel/common_rel.c prolog publishes both): kind (>> 5), area (+1), the FSYS
  group id (+4, matching each archive's header + 8), the room id (+0xC, what warps and the game's room changes name).
  Areas: 1 OUTSKIRT STAND (S1_out, S1_shop_1F), 2 PHENAC, 3 PYRITE, 4 AGATE, 5 THE UNDER, 6 the LABORATORY,
  7 MT. BATTLE, 8 REALGAM TOWER, 10 the SNAGEM HIDEOUT (S2_); 0 and 9 the battle-field copies (_bf) and menus.
- **The common text**: common_rel's .data at 0x58044 is a message block in the .msg layout (2,018 strings): the place
  names (3501-3527), the item, move and Pokemon names, the debug room list (60020-60143: each file's name for the
  developers, e.g. "INSIDE THE DINER (S1_shop_1F)", not shown to players). The room -> place-name table the PDA uses
  isn't found yet (a table of (id, name) pairs at .data + 0x2C20 has other ids).
- **Sound** (in orre_data.pak since 2026-11-07, so the music converts without the disc): common.fsys holds the MusyX
  projects (common/snd_music_proj / _pool / _sdir.bin, and snd_se_* for sounds); the 78 songs are the 0x08 members
  (`<name>_song.snd`: stand_song, stand2_song (likely the OUTSKIRT STAND's), pokecen_song, worldmap_song,
  mt_battle_song, battle2..9, title_song...); their samples are the disc's files/sound/*.samp (snd_music.samp 3.5 MB),
  copied in as sound/*.samp. The decomp restored the MusyX library (pkmn-colosseum PR #668): the reference for the
  sequencer and the sample directory.

## The world map (src/orre/worldmap.c)
- **What it is made of**, all in orre_data.pak: main.dol's menu tables (dol/menu_items.bin: items of 0x1C, u8 flags
  (0x80 shown, 0x40 the last), s16 x y w h (640x480; negative mirrors), s16 sprites a b, s16 the next item at 0x18;
  dol/menu_sprites.bin: sprites of 0x18, u16 x y w h at 8, u32 the texture's resource id at 0x10: the decomp's
  src/game/menu/menu_bios_range_8005D7F8.c), the textures by resource id (index/ids.txt, from fsys.py's _ids.txt:
  .tex = u16 w h, u8 code at 4 (0x04 CMPR, 0x20 RGBA8), texels at 0x80), worldmap_menu_rel's .data (section 3) and
  world_map/world_map.msg. The screen's items run from 0x906 (the map) to 0x91C (the words' box).
- **Its tables** (src/rel/worldmap_menu_rel/worldmap_menu_rel.c; read by src/game/gs_title.c wmapCursorMove /
  wmapDrawInfo / fn_80024A2C): places (9: flag, room, menu item, picture; rooms S1_out, S2_out_2, D1_out, D2_out,
  D4_out, M1_out, M2_out, M3_out, M4_out_2), description rows (12: flag + 9 messages, the first row whose flag is 0 or
  set; place i's message is msg[order[i]], order = main.dol lbl_802E4F58 = 0 8 6 5 2 1 3 4 7, dol/wm_desc_order.bin),
  the travel scene's cameras (20: lbl_3_data_270) and the cursor's links (66: directions 1 up 2 right 4 down 8 left,
  from, to). THE UNDER's picture is mapsnap008 once flag 0x45D is set.
- **Drawn** as the game composes it: every piece 1:1 into a scene (nearest: stretched pieces never bleed into
  their neighbours on the texture), the scene scaled once onto the 480x320 canvas, then the words on top at the
  canvas's own size. **Fitted to the screen** (the user, 2026-11-07): the game's 640 across fills Kai's 480 (3/4), so
  the scene is a 640 x 426.7 window on the game's 640x480 screen; the bottom boxes (picture, words: ItemShift) are
  held to its bottom edge (53.3 up), the buttons' box stays top right, and the map with its places and cursor pans
  up and down under them (0-53.3, eased after the cursor: PanFor keeps the cursor's place in the middle of the map
  left clear above the words box). At the lowest place (PYRITE) the cursor's bottom brackets tuck a little under the
  words box, as in Colosseum's own layout. No zoom past 3/4 was needed: the places all fit across. The cursor's cross lines stop at the three boxes (picture, words, buttons). The GameCube's A / B
  pictures (0xCA7 / 0xCA8) are left out: Kai's own prompts go there (G_Hint "{A} Select" / "{B} Cancel", the icons
  of the controller in use), as on every other screen of the game.
- The cursor's pieces (0x91A the red diamond, 0x931-0x934 the brackets, 0x935 / 0x936 the cross lines) are drawn
  round the place's centre here; where Colosseum's code puts them exactly (it moves them from their table places)
  isn't read yet.

## Rooms, doors and entries (tools/orre/colo_rooms.py, maps.py)
All from the game's own tables, read from the pack's data (the decomp names them):
- **The floor table** (colo_data.py): per room its kind (1 a field room), area, archive, room id, the place name
  (+ 0x24) and four slots its module's _prolog fills (main.dol .sbss addresses, relocated in common_rel's REL: + 0x10
  the room's record, + 0x14 its characters, + 0x18 its entries, + 0x1C a box table). colo_rooms.py resolves a slot to
  the module's table by reading the REL's relocations and its _prolog (lis / addi / stw).
- **Entries**: FieldTable {count *, entries *}, 16 bytes each: u16 angle (degrees about Y, 0 facing +Z = the grid's
  south), pad, f32 x y z. floorLink(room, entry) puts the player there. Entry 0 is where the world map lands
  (gs_title.c: floorLink(floor, 0)).
- **Events** (common_rel .data + 0x146364, 0x1C each, count + 0x14A704; floor_character.c heroDispatchEvent): u8 kind
  (1 pass, 2 touch, 3 check, 4 ...), u16 room, u32 collision id, u32 script, u32 args x4. The collision id is the u16
  at + 0x32 of the trigger triangles in the room's .col (not the object's number: one object can hold several).
  Scripts of the shared bank 0x0596 (common_script_0596.json in the decomp): 4 link_floor_with_sound(room, entry,
  sound), 5 control_door (opens the door; the pass trigger behind it links), 6 leave_elevator(elevator, room,
  elevator there), 12 REALGAM's lift (rides through room 123, then on to the args' room), 11 a message, 13 the PC.
  Bank 0x0100 is the room's own .scd (colo_rooms.py scd_native_calls: the GS VM's constants pushed before each native
  call; floorLink is native 0x5F, floorLinkWithSE 0x7D, scriptSetCol (a collision object on / off) 0x76; names from
  main.dol's native table lbl_802E1CF0 in the decomp's data_8027A500.c).
- **In the game**: maps.py gives each map its warps (collision id -> file, entry or elevator) and exits (room 910, the
  world map); grid.py reaches from every entry, stopping at those triggers (and only those: a door's touch trigger or
  a sign's check trigger is walked over), and joins a door the thick walls kept out (a door frame) by the shortest
  way of up to 3 tiles; gen_maps.py writes a warp event on each reached trigger tile and a landing for every entry
  (its tile, or the nearest walkable), src/orre/orre_maps.h sOrreArrivals (overworld.c GetAdjustedInitialDirection
  faces the entry's angle: O3D_ArrivalDir). gen_tests.py writes the three whole-region scenarios from the same data
  (rerun it after gen_maps.py); check_tests.py checks their logs. `gridview.py --room UNPACKED ROOM out.png` draws a
  room's grid with its entries and links.

## The bike ride (the travel scene: the user's plan, 2026-11-07, for later)
A custom version of the SS TIDAL's porthole scene (src/field_special_scene.c Task_HandlePorthole /
ShowSSTidalWhileSailing): the bike's graphics in the ship's place, against the background of Colosseum's original
ride (move_demo.map, the per-place cameras cam_move_demo_01-15 and speeds of lbl_3_data_270), moving with the SS
TIDAL scene's animation. Not Colosseum's 3D ride itself.

## Learned (formats and pitfalls)
- FSYS member types (byte 2 of the id): 0x02 map model, 0x04 character / object model, 0x06 collision, 0x0A text,
  0x0C floor data (not HSD), 0x0E map script, 0x18 camera shot, 0x1C map module (REL), 0x1E Pokémon models, 0x20 move
  animations. Members of one name and different types live in one archive: fsys.py names them by type.
- HSD: instance joints (flag 0x1000) draw the one joint their child points to, never its siblings (drawing the
  siblings blew one map up to 3.4 million triangles). Envelope vertices bound to one joint at full weight are in that
  joint's space. Baked shadows are see-through black meshes (render mode bit 0x40000000). GX intensity textures (I4,
  I8) carry their intensity as alpha too. Colosseum's baked vertex colours are half strength (128 = white).
- grid.py's wall tests are numpy (35 times faster than per-segment Python: all 121 rooms in 35 s); gen_maps.py and
  build_pack.py both build every grid (maps.build_grid).
- Big 1024 x 1024 atlases (90 of them) look blocky up close under Platinum's camera: that is their own resolution
  (raising TEX_MAX past 256 changed nothing visible), not a decoding fault.
- Collision walls lie on tile edges: test steps between tile centres, not tiles (the flood fill leaked behind the
  diner until it did). The floor goes on past a map's exits: keep only what is reached from the start.
- Text: 0xFFFF starts a control (an id byte, then its parameters: font, wait, palette, shadow, align, talk sound, line
  space and baseline take 1 byte, colour 4). Ids 10-12, 58-60, 63-64 have no handler; only menu strings use them.
- Other sessions' work in shared files stays as they left it (the user, 2026-11-07): data/event_scripts_orre.s
  carries Moemon Maker's `data/maker_globals.inc` include; gen_maps.py only adds missing map includes there. Fetch
  the default branch before starting.
- include/constants/region_map_sections.h is generated (ignored): edit its template,
  src/data/region_map/region_map_sections.constants.json.txt.
- A fresh container's sinnoh/platinum_scripts.inc can be older than the Maker's label table: if the link misses
  SinnohP_* labels, regenerate it (`python3 -I tools/sinnoh/gen_scripts.py --auto sinnoh/platinum_scripts.inc`).
- The build rewrites mugen/processed_assets.idx: restore it before committing. `rm` with an unset variable is
  refused by the session's safety check: write `"${G:?}"/...`.
