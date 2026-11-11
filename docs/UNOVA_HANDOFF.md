# UNOVA: handoff

Pokémon Black as a playable region, built the way SINNOH was from Platinum (docs/SINNOH_HANDOFF.md). The user's
decisions: docs/UNOVA_DECISIONS.md (read it first: every Platinum decision holds unless it says otherwise).

## Where things stand (2026-11-11)

- Groundwork only. Nothing of UNOVA is in the game yet.
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
