# ORRE handoff (2026-11-07)

Pokémon Colosseum's region in Kai. **Read `docs/ORRE_DECISIONS.md` first**: the user's decisions (own engine, 2D
sprites rendered from the models, Kai's battles, Colosseum's own words and scripts, Shadow Pokémon in Orre only), the
rules, and what each tool does. This file is where things stand, what a new session needs, and what comes next.

## Where it stands
- **Rip: done and checked.** `tools/orre/make_pack.sh DISC` -> orre/orre3d.pak (174 maps, 764 textures, the grids of
  the maps in the game) and orre/orre_data.pak (text, collision, scripts, floor data, cameras, map tables, character
  models). Never committed (orre/ is ignored).
- **Shipping: wired, not switched on.** The release step "ORRE's packs" makes MoemonOrre.zip (78 MB) and an update.txt
  `orre` line; the game downloads it with Updater_Orre* (the DANCE songs' downloader, one `struct Extra` each). It
  rips only when the `ORRE_DISC_URL` repository secret is set: the user adds it once ORRE is completable and full
  (their decision, 2026-11-07). No in-game download screen yet.
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
  `mp orreflag N` (src/orre/orre_story.c, not saved yet). Tests: scenarios `orre_worldmap`, `orre_worldmap_pan`.
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
0. **Shadow Pokémon come before the story** (the user, 2026-11-07): their design and the user's decisions are in
   `docs/SHADOW_POKEMON.md`; its open questions are settled with the user as each part is built. Built: the data, the
   gauge, battles, HYPER MODE / CALL, the aura, snagging, purification, the battle confinement (SHADOW_POKEMON.md
   "Built so far"). Next (the user agreed, 2026-11-07): the PC and trade confinement, then the people (step 2, 4).
   **ORRE is a post-game place reached by boat** (ORRE_DECISIONS.md "A post-game place": the ORRE TICKET after any
   ELITE FOUR, HOENN's ferries and every port, landing at the OUTSKIRT STAND by the sailor: src/orre/orre_ferry.c).
1. **Music** (the user, 2026-11-07: "Colosseum's own world map screen, then convert the music"): see 6.
   Then the world map's own song (worldmap_song) and the travel scene: the user's plan, "The bike ride" below.
2. **Where people stand.** Doors and entries are done (below, "Rooms, doors and entries"). The people: each room's
   characters table (the floor record's + 0x14 slot, floor_data.c floorDataBiosGetCharInfo; records of 36 bytes:
   flags, u16 angle, u16 model, ..., the talk script at + 0x14 (floorCharacterBiosGetTalkSctID), the move script at
   + 0x10, x y z at + 0x18; colo_rooms.py reads the slots the same way) for where they stand and which model they are.
3. **Maps: done** (all 121 field rooms). The stadiums' battle stages (kind 2 rooms: M1_water_colo, M2_earth_colo,
   M4_cylinder_colo, M4_bottom_colo, T1_ancient_colo) aren't maps: Kai's battles. The `_ev` and `_bf` copies aren't
   either (a scene's or a battle's).
4. **Sprites.** Render the character models (orre_data.pak, `.dat`) into GBA-style sprite sheets (the user's
   decision): their animations (each model set's joint animations, after the root joint) aren't read yet; hsd.py
   draws the bind pose. tools/sinnoh/build_starter.py pre-draws Platinum's models, as a reference.
5. **Words and scripts.** The map scripts (.scd, the GS VM) converted to Kai's (as tools/sinnoh/gen_scripts.py does
   SINNOH's); the text (msg.py) into Kai's charset, controls mapped (`{hero}`, `{rui}`, `{npc}`, `{item}`...). Never
   write ORRE's words or scripts by hand.
6. **Music** (the user, 2026-11-07: converted, it can't be found online; next): the MusyX projects in common.fsys
   (snd_music_proj / _pool / _sdir) and the songs (0x08 members), to .kaisong; the decomp's MusyX library is the
   reference. Each map gets its song as its song converts.
7. **Snagging and Shadow Pokémon**, in Orre only (state in a sidecar: saves never change layout; Colosseum's story
   flags, src/orre/orre_story.c, go there too).
8. The in-game download screen when orre/ is missing (as the DANCE machine's rh_screen.c StateNoSongs).

## Open questions for the user
- The sky is left out (Platinum's camera looks down at the ground). Keep it that way?
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
