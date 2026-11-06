# ORRE handoff (2026-11-06)

Pokémon Colosseum's region in Kai. **Read `docs/ORRE_DECISIONS.md` first**: the user's decisions (own engine, 2D
sprites rendered from the models, Kai's battles, Colosseum's own words and scripts, Shadow Pokémon in Orre only), the
rules, and what each tool does. This file is where things stand, what a new session needs, and what comes next.

## Where it stands
- **Rip: done and checked.** `tools/orre/make_pack.sh DISC` -> orre/orre3d.pak (174 maps, 764 textures, the grids of
  the maps in the game) and orre/orre_data.pak (text, collision, scripts, floor data, cameras, map tables, character
  models). Never committed (orre/ is ignored).
- **Shipping: wired, not switched on.** The release step "ORRE's packs" makes MoemonOrre.zip (78 MB) and an update.txt
  `orre` line; the game downloads it with Updater_Orre* (the DANCE songs' downloader, one `struct Extra` each). It
  rips only when the `ORRE_DISC_URL` repository secret is set: **the user hasn't added it yet** (Settings -> Secrets
  and variables -> Actions, the disc's link below). No in-game download screen yet.
- **Engine: ORRE's own**, src/orre/o3d.c (+ gx_tex.c, orre_maps.h, include/o3d.h), a copy of SINNOH's that goes its
  own way. Never put ORRE code in src/sinnoh/ or s3d.h (the user, 2026-11-04).
- **One map walks: the OUTSKIRT STAND** (Orre_OutskirtStand, group 45 map 0; `MOEMON_DEBUG_WARP=45,0,25,29` starts
  at the foot of the diner's ramp). No warps, people, scripts or music of its own yet.

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
1. **The diner's door.** Its trigger is collision object 13 of S1_out.col (the `D` tiles at the ramp's top, row 27).
   Add S1_shop_1F to tools/orre/maps.py (a start point inside: find it with gridview.py), have gen_maps.py write
   warp events for each map's door triggers (which trigger leads where: the map modules' tables or the scripts;
   until then, by hand in maps.py), and check the warp both ways in a scenario.
2. **The way out.** The trigger loop round the parking lot (object 2) is how Colosseum leaves for its world map
   (worldmap_menu_rel in the decomp). Decide with the user how ORRE's world map works in Kai (Colosseum's own screen,
   or a Kai-style town map) before building it.
3. **Where people stand.** The map modules (.rel) publish tables (REL_M1.md: "FieldTable" descriptors, fields not
   named). S1_out.rel has (direction, x, y, z)-like records whose stride isn't clear yet. Decode them (the decomp's
   readers of those tables, or XD's decomp) for start positions, people and their models.
4. **Sprites.** Render the character models (orre_data.pak, `.dat`) into GBA-style sprite sheets (the user's
   decision): their animations (each model set's joint animations, after the root joint) aren't read yet; hsd.py
   draws the bind pose. tools/sinnoh/build_starter.py pre-draws Platinum's models, as a reference.
5. **Words and scripts.** The map scripts (.scd, the GS VM) converted to Kai's (as tools/sinnoh/gen_scripts.py does
   SINNOH's); the text (msg.py) into Kai's charset, controls mapped (`{hero}`, `{rui}`, `{npc}`, `{item}`...). Never
   write ORRE's words or scripts by hand.
6. **Snagging and Shadow Pokémon**, in Orre only (state in a sidecar: saves never change layout).
7. The in-game download screen when orre/ is missing (as the DANCE machine's rh_screen.c StateNoSongs).

## Open questions for the user
- The tile scale: 8 Colosseum units a tile (a person is about two tiles, as Kai's sprites). It looks right on the
  OUTSKIRT STAND; confirm before more maps.
- The sky is left out (Platinum's camera looks down at the ground). Keep it that way?
- Placeholders until ORRE has its own: the music (MUS_DESERT), the map section (MAPSEC_SPECIAL_AREA: no name shown).
  Colosseum's music is on the disc but not ripped (its sound archives aren't FSYS members); how should ORRE sound?
- Turning on shipping: add the `ORRE_DISC_URL` secret now, or once ORRE can be reached in the game?

## Learned (formats and pitfalls)
- FSYS member types (byte 2 of the id): 0x02 map model, 0x04 character / object model, 0x06 collision, 0x0A text,
  0x0C floor data (not HSD), 0x0E map script, 0x18 camera shot, 0x1C map module (REL), 0x1E Pokémon models, 0x20 move
  animations. Members of one name and different types live in one archive: fsys.py names them by type.
- HSD: instance joints (flag 0x1000) draw the one joint their child points to, never its siblings (drawing the
  siblings blew one map up to 3.4 million triangles). Envelope vertices bound to one joint at full weight are in that
  joint's space. Baked shadows are see-through black meshes (render mode bit 0x40000000). GX intensity textures (I4,
  I8) carry their intensity as alpha too. Colosseum's baked vertex colours are half strength (128 = white).
- Collision walls lie on tile edges: test steps between tile centres, not tiles (the flood fill leaked behind the
  diner until it did). The floor goes on past a map's exits: keep only what is reached from the start.
- Text: 0xFFFF starts a control (an id byte, then its parameters: font, wait, palette, shadow, align, talk sound, line
  space and baseline take 1 byte, colour 4). Ids 10-12, 58-60, 63-64 have no handler; only menu strings use them.
- The build rewrites mugen/processed_assets.idx: restore it before committing. `rm` with an unset variable is
  refused by the session's safety check: write `"${G:?}"/...`.
