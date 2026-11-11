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
