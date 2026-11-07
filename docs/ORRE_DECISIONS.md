# ORRE (Pokémon Colosseum's region): the user's decisions, and where it stands

Orre as a real playable region, ported from Pokémon Colosseum (GC6E01, NTSC-U) through the Colosseum decomp
(christinebaker1212-hash/pkmn-colosseum: every function and data byte matched) and the disc itself.

## Decided (the user, 2026-11-04)
- **Overworld**: drawn in 3D from Colosseum's own map models, with every Colosseum camera replaced by a fixed
  Platinum-style camera. Game logic stays on Emerald's grid, as SINNOH's does.
- **ORRE has its own engine, never SINNOH's** (the user, 2026-11-04: "Keep Orre and Sinnoh as separate engines. Do not
  conflate the two - they will both need specific and tailored changes that may break both if bundled together"):
  src/orre/o3d.c (include/o3d.h) began as a copy of SINNOH's src/sinnoh/s3d.c and goes its own way; ORRE's features go
  there, and nothing of ORRE goes into src/sinnoh/ or its headers. Where the game calls a 3D engine (the renderer,
  sprites, the overworld's frame, map blocks, tile behaviors) ORRE's hook is its own line beside SINNOH's.
- **Sprites**: everything on the map is a 2D sprite standing in the 3D scene, never Colosseum's 3D characters.
  NPC sprites (and trainers' battle pictures) are rendered from Colosseum's own models so they look like GBA sprites.
- **Battles**: Kai's own battle system (2D, co-op, PvP), never Colosseum's 3D battle scenes.
- **Words and scripts**: Colosseum's own, converted from the game (as SINNOH's are Platinum's): never original
  dialogue or scripts for Orre.
- **Shadow Pokémon**: only in Orre (snagging, the heart gauge, Hyper Mode, Shadow moves, purification). Settled in
  full before the story (the user, 2026-11-07): **docs/SHADOW_POKEMON.md** (Colosseum 1:1; confined to ORRE and kept
  out of PvP, facilities and trades until purified; co-op: the first to snag gets it; more Shadow Pokémon on Cipher's
  teams from GENERATION 4, added on top; missed ones met again later; the Garden opens the gauge too).
- **The packs ship with the game** (the user: "completely rip the 3D models and text from the Colosseum ISO so that it
  can be bundled in a download, without requiring the ISO from the user"): players need no disc.
- **As their own zip** (the user, 2026-11-04: "make it its own zip"): MoemonOrre.zip, like the DANCE machine's songs,
  not in the game's own zip. **Fetched at launch** (the user, 2026-11-07: "It should be pre-downloaded upon launch as
  part of the updater. If it can't be downloaded, it skips until next time"): after the launch's update check
  (src/platform/updater.c OrrePreWorker) the packs download and unpack in the background while the game runs; a
  failure only waits for the next launch. The boat checks them before it sails (src/orre/orre_ferry.c Orre_CheckBoat):
  still downloading, it shows the progress; missing, it offers the download; no release with them, it doesn't sail
  (never an empty ORRE: docs/user_images/74.png). Test tools/test/orre_download_test.sh orre ("the boat", "at launch").
- **Shipped early** (the user, 2026-11-07, after the boat landed in an empty ORRE without the packs: "Let's go ahead
  and ship it. My players won't visit until it's ready"): this replaces "shipping waits for a complete ORRE"; the
  release ships MoemonOrre.zip once the user adds the `ORRE_DISC_URL` repository secret (a link to the disc).
- **Music: Colosseum's own, converted** (the user, 2026-11-07: "The music cannot be found online, and thus, must be
  converted"), from the disc's sound data into .kaisong songs; never a placeholder: a map without its song yet is
  silent ("Placeholder music - none").
- **Map names from the game's own areas** (the user, 2026-11-07): each map's name is Colosseum's name for its place.
- **The tile scale is Claude's call** (the user, 2026-11-07): 8 Colosseum units a tile, as the OUTSKIRT STAND has it.
- **Every map, then the hard work** (the user, 2026-11-07: "The baseline is that all maps will be able to be traveled
  to and are finished. After that, the REALLY hard work begins"): all of Colosseum's field rooms as maps, their doors
  where the game's tables lead (docs/ORRE_HANDOFF.md "Rooms, doors and entries"); people, scripts and the rest after.
- **Colosseum's own world map screen** (the user, 2026-11-07): leaving a place through its exits opens Colosseum's
  world map (its own screen, art and travel), never a Kai-style town map. Then the music is converted.
- **The world map fits the screen** (the user, 2026-11-07: "fit the map to the screen, use zooms and panning if
  needed"): no bars at the sides; the map fills Kai's 3:2 screen and pans under the boxes (ORRE_HANDOFF "Drawn").
- **A post-game place, reached by boat** (the user, 2026-11-07: "Orre will not be selectable as a region to start in,
  rather, it is always a post-game region, and a special one at that - it goes on the BATTLE FRONTIER, BATTLE ISLAND,
  FARADAY ISLAND, etc menu. Players who did not unlock Orre in a postgame save will be given the proper ticket"): ORRE
  is never one of NEW GAME's regions (it isn't one of Kai's REGION_*: it's a shared place like BATTLE ISLAND, the
  player stays in their region there: mp_region.c MP_Region_IsSharedMap). The **ORRE TICKET** (item 810, after the
  Maker's slots) comes **after any region's ELITE FOUR** (the user's choice), brought by BATTLE ISLAND's courier after
  the BATTLE TICKET (SCOTT in HOENN, a sailor in JOHTO / KANTO, a letter in SINNOH; old post-game saves get it the
  same way: mp_battleisland.c). LILYCOVE's and SLATEPORT's ferry clerks list ORRE among their destinations (the lists
  built at run time, scrolling when they're too long: script_menu.c), every other region's port offers it before its
  usual trips (data/scripts/orre.inc EventScript_OrreOffer). The boat **lands at the OUTSKIRT STAND** (Colosseum's own
  start) **in front of a sailor** who sails back to the port the player came from (the user's choice: ORRE's one
  person with Kai's words, tools/orre/gen_maps.py KAI_OBJECTS; src/orre/orre_ferry.c), never with a Shadow Pokémon in
  the party (docs/SHADOW_POKEMON.md decision 2). Tests orre_ticket, orre_voyage, orre_lilycove.
- **The bike ride between places** (the user, 2026-11-07, for later): a custom version of the SS TIDAL's porthole
  scene, the bike's graphics against the background of Colosseum's original ride, with the SS TIDAL scene's
  animation (ORRE_HANDOFF "The bike ride").

## Rules to keep
- The disc image is never committed, and neither are the packs (orre/ is ignored): tools/orre/make_pack.sh builds
  them. The release (step "ORRE's packs") puts them in MoemonOrre.zip (tools/release/package_orre.sh) with an
  update.txt line "orre <tag> <zip> <bytes> <sha256> <id>"; the id is a hash of tools/orre/, so while those tools
  stay the same the last release's line is kept and the disc isn't fetched at all. When they change, it rips them
  from the disc whose link is the `ORRE_DISC_URL` repository secret (a Google Drive share link works); without the
  secret the last release's packs stay (a warning).
- The game: src/platform/updater.c Updater_Orre* (Check / Download / Cancel / State / Root: the songs' code, one
  `struct Extra` each) unpacks it into orre/ next to the exe, the id in orre/orre_version.txt. Debug commands
  `mp orrecheck`, `mp orredownload`, `mp orrestate`; test tools/test/orre_download_test.sh [ORRE_DIR].
- Only the disc the decomp matches: make_pack.sh checks main.dol's SHA-1 (870e8b96...).
- Saves never change layout: Orre's state (heart gauges, Shadow flags) goes in a sidecar.

## Where it stands (2026-11-04)
The rip is done and checked: `tools/orre/make_pack.sh DISC` (a .rvz / .iso, or a .zip holding one) -> orre/:
- **orre3d.pak** (94 MB): all 174 map models (1.7 million triangles without the skies), their 764 textures kept in
  the disc's own GX encoding (lossless; CMPR at 4 bits a texel), meshes indexed. Format at the top of
  tools/orre/build_pack.py.
- **orre_data.pak** (27 MB): the game's own bytes of every text file (36,249 strings), collision, map script, floor
  data, camera shot, map module (their tables), character / object model (254: for the sprites), and the music and
  sounds (78 songs, the MusyX projects, their samples), one copy each.
- `ORRE_TEST=1 tools/orre/make_pack.sh DISC` also runs tools/orre/test_pack.py: every map's triangles and textures
  and every data file read back from the packs equal the disc's. 174 maps, 764 textures, 2058 files: OK.

The tools (Python, no disc needed once the packs exist):
- fsys.py: the disc's FSYS archives (the decomp's reader, MIT, with members of one name kept apart by type).
- hsd.py + gxtex.py: HAL's HSD models (joints, instances, envelopes, materials, display lists) and every GX texture
  format, from the decomp's struct layouts (include/hsd/). msg.py: the text, every control named from the game's
  own table (src/game/data/data_80363774.c). pak.py reads the packs back (render a map, print a text file);
  preview.py is a quick software render for checking by eye.

What the disc's files are (member type = byte 2 of the id): 0x02 map model, 0x04 character / object model, 0x06
collision, 0x0A text, 0x0C floor data, 0x0E map script, 0x18 camera shot, 0x1C map module (REL), 0x1E Pokémon models,
0x20 move animations (those two not ripped: Kai's battles).

Learned:
- A map's `scene_data` holds model sets: the place (sometimes in pieces), and usually a sky (a dome or a far
  backdrop). The pack marks the sky as a hint (kind 2) and keeps every set.
- Baked shadows are separate see-through black meshes (render mode bit 0x40000000): draw them blended.
- Instance joints (flag 0x1000) draw the one joint their child points to again; never its siblings.
- Envelope vertices bound to one joint at full weight are in that joint's space; blended ones in the bind pose.
- Some text is still Japanese: strings from cut events the US release never used. `‱` is the font's "???" glyph
  (speakers not yet named).

## In the game: the OUTSKIRT STAND (2026-11-04)
The user: "load the Outskirt Stand". It walks: Colosseum's own model (the locomotive diner, the lot, the wheels and
their baked shadows) drawn by the SINNOH engine under Platinum's camera, the player on Emerald's grid made from
Colosseum's collision. Test: scenario `orre_outskirt_stand` (needs orre/ from make_pack.sh).
- **Maps** (since 2026-11-07 all 121 field rooms, everything about them from the game's tables: tools/orre/maps.py,
  colo_rooms.py, docs/ORRE_HANDOFF.md "Rooms, doors and entries"): tools/orre/gen_maps.py UNPACKED_DIR
  writes data/maps/<name>/ (map.json, scripts.inc), data/layouts/<name>/ (placeholder blocks, all blocked),
  gMapGroup_Orre (group 45, after SINNOH's), data/event_scripts_orre.s and src/orre/orre_maps.h (sOrreMaps: file,
  blocks, start tile, room; sOrreArrivals: each entry's landing and facing). No music (MUS_NONE) until ORRE's songs
  are converted; the map section is ORRE's own, named from Colosseum's text (below). Debug warp:
  MOEMON_DEBUG_WARP=45,0,33,40 (the OUTSKIRT STAND's entry 0).
- **Grid** (tools/orre/grid.py; gridview.py draws it over a top-down picture, with the collision's walls in yellow and
  floor edges in cyan): the .col's objects have three triangle grids: +0x24 the floor, +0x28 walls, +0x2C triggers
  (doors, the exits to the world map). A tile is 8 Colosseum units (a person is about 15: two tiles, as Kai's
  32-pixel sprites; the tile scale is Claude's call, the user 2026-11-07). What a person reaches from the start is
  worked out on a fine grid (1 unit): every wall at a person's height (1-10 units above the ground) thickened by a
  person's radius (2.5), so a gap passes only if a person fits through it; the floor only, no step over 6 units;
  trigger cells are reached, not walked through (so the world past the exits and the insides of props' wall outlines
  stay out). A tile is walkable when that reach comes within 2 units of its centre (a trigger tile: when it touches
  the trigger). Where a wall still lies between two walkable tiles, the one nearer it is blocked (Emerald's grid has
  no walls between tiles). Tile-by-tile tests failed both ways (2026-11-07): one closed the diner's 11-unit aisle,
  one opened a gap behind the OUTSKIRT STAND's train. The grid is in the pack (version 2), blocks of 32 x 32 tiles.
- **Doors** (tools/orre/maps.py `warps`, from the game's event table: docs/ORRE_HANDOFF.md "Rooms, doors and
  entries"): gen_maps.py writes a warp event on every reached tile of a door's trigger, leading to the landing of the
  entry the game names over there (or the middle tile of the other elevator's trigger); the pack gives those tiles
  MB_NON_ANIMATED_DOOR (0x60, the grid record's behavior byte), stepped on to warp. Arriving on a door, the player
  walks on the way they were going (field_screen_effect.c OrreArrivalDir); on an entry, they face its angle.
- **Names** (2026-11-07): each map's `place` in tools/orre/maps.py is one of Colosseum's place names (common_rel's
  messages 3501-3527), the one its floor record's area is; gen_maps.py reads the text from the disc and writes ORRE's
  map sections (include/constants/region_map_sections_orre.h, 0x1C0 on, included from the region_map_sections
  template; names src/data/region_map/orre_region_map.h; region_map.c GetMapName's ORRE branch). Outdoor maps
  (`_out`) show the name on arrival. `mp mapname` prints a map's section and name.
- **Engine** (src/orre/o3d.c): ORRE's maps (src/orre/orre_maps.h) read at first visit (MapLoad: the grid as the
  field's blocks and ground heights, the meshes in Colosseum's units * 16 / tile; textures decoded by src/orre/gx_tex.c,
  the same as gxtex.py: tools/orre/test_gxtex.sh, and halved to at most 256; vertex colours doubled, as GX's TEV does
  with baked lighting; both faces drawn; the sky left out). SINNOH's camera, rasterizer, sprites as upright cards and
  fades, copied. About 5 ms a frame for the stand. Its hooks: gba_easy_draw.c gMpO3dLine / gMpO3dOamHidden,
  sprite.c O3D_PlaceSprite / O3D_NoteOam / O3D_CommitOam, overworld.c O3D_Update, maker_overrides.c Maker_MapBlocks
  (O3D_LayoutBlocks after SINNOH's), fieldmap.c / field_camera.c (O3D_METATILE_BEHAVIOR on O3D_IsOrreGroup maps).
  orre/ from Updater_OrreRoot, or MOEMON_ORRE_PAK; MOEMON_O3D_PERF prints a frame's time.

- **The world map** (2026-11-07, src/orre/worldmap.c): Colosseum's own screen (its map, pieces, pictures, words,
  cursor links and which places show, from the game's tables: docs/ORRE_HANDOFF.md "The world map") in a 480x320
  picture of its own (display.c / sdl2.c, as the BADGE CASE's). maps.py `exits` -> gen_maps.py's coord events ->
  data/scripts/orre.inc Orre_EventScript_WorldMap (specials Orre_WorldMap, Orre_WorldMapTravel). Colosseum's story
  flags: src/orre/orre_story.c (`mp orreflag N [0|1]`; not saved yet). The pack's reader: src/orre/orre_data.c.

## Next (the full list, with what is known of each: docs/ORRE_HANDOFF.md)
1. The music, converted (the user, 2026-11-07), then the world map's travel scene (the bike ride, as the SS TIDAL's
   porthole scene: above).
2. Colosseum's start positions and people: the map modules' tables (.rel; their records aren't decoded yet: the
   direction / x / y / z records seen in S1_out.rel have no clear stride) for where people stand, and their models.
3. Sprites: render the character models (with their own animations, still to be read) into GBA-style sprite sheets.
4. Words and scripts: the map scripts (.scd, the GS VM: the decomp's gs_vm.c) converted to Kai's, like SINNOH's.
5. Snagging and Shadow Pokémon, in Orre only.
- The download screen in the game when ORRE's packs are missing (as the DANCE machine's StateNoSongs), shown where a
  player first travels to ORRE. MoemonOrre.zip is 78 MB.
