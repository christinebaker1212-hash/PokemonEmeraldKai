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
- **Shadow Pokémon**: only in Orre (snagging, the heart gauge, Hyper Mode, Shadow moves, purification).
- **The packs ship with the game** (the user: "completely rip the 3D models and text from the Colosseum ISO so that it
  can be bundled in a download, without requiring the ISO from the user"): players need no disc.
- **As their own zip** (the user, 2026-11-04: "make it its own zip"): MoemonOrre.zip, like the DANCE machine's songs,
  downloaded when a player first goes to ORRE, not in the game's own zip.

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
- **orre_data.pak** (18 MB): the game's own bytes of every text file (36,249 strings), collision, map script, floor
  data, camera shot, map module (their tables) and character / object model (254: for the sprites), one copy each.
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
- **Maps** (tools/orre/maps.py: a Colosseum map file, its map id and a start point): tools/orre/gen_maps.py UNPACKED_DIR
  writes data/maps/<name>/ (map.json, scripts.inc), data/layouts/<name>/ (placeholder blocks, all blocked),
  gMapGroup_Orre (group 45, after SINNOH's), data/event_scripts_orre.s and src/sinnoh/orre_maps.h (sOrreMaps: file,
  blocks, start tile). Only the map's size comes from the disc. MUSIC is MUS_DESERT and the section
  MAPSEC_SPECIAL_AREA until ORRE has its own songs, names and town map. Debug warp: MOEMON_DEBUG_WARP=45,0,25,29.
- **Grid** (tools/orre/grid.py; gridview.py draws it over a top-down picture): the .col's objects have three triangle
  grids: +0x24 the floor, +0x28 walls, +0x2C triggers (doors, the exits to the world map). A tile is 8 Colosseum units
  (a person is about 15: two tiles, as Kai's 32-pixel sprites); a tile is walkable with floor under its centre that
  isn't too steep, no wall across it at a person's height, and reached from the start without crossing a wall or a
  step over 6 units; triggers are reached, not walked through (so the world past the exits and the insides of props'
  wall outlines stay blocked). Where a wall lies between two walkable tiles, the one nearer it is blocked (Emerald's
  grid has no walls between tiles). The grid is in the pack (version 2), blocks of 32 x 32 tiles, the floor centred.
- **Engine** (src/orre/o3d.c): ORRE's maps (src/orre/orre_maps.h) read at first visit (MapLoad: the grid as the
  field's blocks and ground heights, the meshes in Colosseum's units * 16 / tile; textures decoded by src/orre/gx_tex.c,
  the same as gxtex.py: tools/orre/test_gxtex.sh, and halved to at most 256; vertex colours doubled, as GX's TEV does
  with baked lighting; both faces drawn; the sky left out). SINNOH's camera, rasterizer, sprites as upright cards and
  fades, copied. About 5 ms a frame for the stand. Its hooks: gba_easy_draw.c gMpO3dLine / gMpO3dOamHidden,
  sprite.c O3D_PlaceSprite / O3D_NoteOam / O3D_CommitOam, overworld.c O3D_Update, maker_overrides.c Maker_MapBlocks
  (O3D_LayoutBlocks after SINNOH's), fieldmap.c / field_camera.c (O3D_METATILE_BEHAVIOR on O3D_IsOrreGroup maps).
  orre/ from Updater_OrreRoot, or MOEMON_ORRE_PAK; MOEMON_O3D_PERF prints a frame's time.

## Next
1. The OUTSKIRT STAND's door: its trigger (the ramp's top) as a warp into S1_shop_1F, made a map the same way, and
   the exit loop as the way out to the world map (Colosseum's world map screen: worldmap_menu_rel).
2. Colosseum's start positions and people: the map modules' tables (.rel; their records aren't decoded yet: the
   direction / x / y / z records seen in S1_out.rel have no clear stride) for where people stand, and their models.
3. Sprites: render the character models (with their own animations, still to be read) into GBA-style sprite sheets.
4. Words and scripts: the map scripts (.scd, the GS VM: the decomp's gs_vm.c) converted to Kai's, like SINNOH's.
5. Snagging and Shadow Pokémon, in Orre only.
- The download screen in the game when ORRE's packs are missing (as the DANCE machine's StateNoSongs), shown where a
  player first travels to ORRE. MoemonOrre.zip is 78 MB.
