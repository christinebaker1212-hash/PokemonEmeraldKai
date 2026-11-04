# ORRE (Pokémon Colosseum's region): the user's decisions, and where it stands

Orre as a real playable region, ported from Pokémon Colosseum (GC6E01, NTSC-U) through the Colosseum decomp
(christinebaker1212-hash/pkmn-colosseum: every function and data byte matched) and the disc itself.

## Decided (the user, 2026-11-04)
- **Overworld**: drawn by the SINNOH 3D engine (src/sinnoh/s3d.c) from Colosseum's own map models, with every
  Colosseum camera replaced by a fixed Platinum-style camera. Game logic stays on Emerald's grid, as SINNOH's does.
- **Sprites**: everything on the map is a 2D sprite standing in the 3D scene, never Colosseum's 3D characters.
  NPC sprites (and trainers' battle pictures) are rendered from Colosseum's own models so they look like GBA sprites.
- **Battles**: Kai's own battle system (2D, co-op, PvP), never Colosseum's 3D battle scenes.
- **Words and scripts**: Colosseum's own, converted from the game (as SINNOH's are Platinum's): never original
  dialogue or scripts for Orre.
- **Shadow Pokémon**: only in Orre (snagging, the heart gauge, Hyper Mode, Shadow moves, purification).
- **The packs ship with the game** (the user: "completely rip the 3D models and text from the Colosseum ISO so that it
  can be bundled in a download, without requiring the ISO from the user"): players need no disc.

## Rules to keep
- The disc image is never committed, and neither are the packs (orre/ is ignored): tools/orre/make_pack.sh builds
  them, the release runs it with the disc's link from the `ORRE_DISC_URL` repository secret (a Google Drive share
  link works) and package.sh ships orre/. Without the secret a release has no orre/.
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

## Next
1. The engine: load an orre3d.pak map into src/sinnoh/s3d.c (a GX texture decoder in C, the meshes as its groups),
   the Outskirt Stand first, under the fixed camera.
2. Collision to the grid: the .col meshes rasterized to Emerald's 16-unit tiles with heights (the tile scale is
   still open: decided on the Outskirt Stand).
3. Sprites: render the character models (with their own animations, still to be read) into GBA-style sprite sheets.
4. Words and scripts: the map scripts (.scd, the GS VM: the decomp's gs_vm.c) converted to Kai's, like SINNOH's.
5. Snagging and Shadow Pokémon, in Orre only.
- Size: the packs add about 112 MB to the download. Like the DANCE machine's songs, they could become their own zip,
  fetched when a player first goes to Orre (updater.c Updater_Songs*), so updates stay small: the user's call.
