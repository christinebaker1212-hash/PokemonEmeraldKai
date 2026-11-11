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
  MOEMON_U3D_SEASON), `mp warp 52,12,16,16` (NUVEMA TOWN). No people, words, scripts or music yet; no way in for
  players (the region choice and the boat come with the first area).
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
