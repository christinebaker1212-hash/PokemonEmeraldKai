# HANDOFF: Sinnoh (Pokémon Platinum) region for Moemon Emerald Kai Co-op

Queued after the 1.61.6 release and the KANTO / JOHTO sweep (the user's plan, 2026-10-31).

## Where things stand
- Proof of concept done (2026-10-31, branch sinnoh-work pushed to claude/exciting-sagan-17lq9j): TWINLEAF TOWN and
  ROUTE 201 drawn by the new 3D engine from the user's ROM, walked with Platinum's collision, the player and every field
  sprite standing in the scene (hidden by trees and roofs in front of them), lit by the area's lights for the hour.
  tools/sinnoh/test_poc.sh checks it against the offline reference render. COOP_NOTES "SINNOH's 3D engine" has the
  how. Heights (BDHC) done: the camera and every sprite stand on Platinum's ground (COOP_NOTES). Every outdoor map of the
  matrix is in (66, connected; tools/sinnoh/gen_maps.py). The buildings too (433 maps in groups
  41-44), with every place's warps: doors lead in, mats out. The pack ships compressed (7 MB). SINNOH is a region of its own
  (REGION_SINNOH: GENERATION cap 4+, badges from flag 0x3320, CANALAVE's dock for the boat, off the boat's list
  until its story is in), with Platinum's wild POKéMON (gen_wild.py). Next: data (step 3: trainers, items, marts),
  scripts and text (step 4: original writing for this game, committed per map; DECIDED 2026-10-31: nothing of
  Platinum's scripts or text is converted, committed or shipped; tools/sinnoh/words.py + gen_words.py, who.py to see
  a map's people: TWINLEAF, ROUTE 201 and SANDGEM written, COOP_NOTES "Words"; done 2026-11-01: item balls, hidden
  items, the 403 route trainers with lines of their own, TM51-TM92), the PÓKETCH (step 5);
  animated textures and Platinum's door animations when there's time (the 3D picture takes 4-5 ms a frame).
- DECIDED (the user, 2026-10-31): the 3D data comes from pret/pokeplatinum, not a ROM. The decomp holds every map
  block, texture set, prop model and light set as files (byte for byte the ROM's), so tools/sinnoh/make_pack.sh fetches
  it at a pinned commit (fetch_decomp.sh) and build_pack.py converts it; the release does that and ships
  sinnoh/platinum3d.pak beside the game. Players need nothing. Still nothing of it in this repository; the ROM path
  (build_pack.py --rom) stays for cross-checks.
- Read CLAUDE.md, BUILDING_COOP.md, docs/REGIONS_PLAN.md and COOP_NOTES ("JOHTO and KANTO",
  "Gen 4 battles", "Gens 5-9 and GENERATION", "The GAME CENTER") first.
- Reference decomp: https://github.com/pret/pokeplatinum
- Data: pret/pokeplatinum (tools/sinnoh/fetch_decomp.sh, pinned commit). The user's ROM, for cross-checks only:
  Google Drive file 13njqEG8nKIltrVkMAgXtM5E6X3dV33as, "Pokemon - Platinum Version (USA) (Rev 1).zip" (about 51 MB;
  the .nds inside), downloaded into its own folder outside the repo. Nothing from either is committed.

## The goal
Add SINNOH as a third region beside JOHTO / KANTO, built from Pokémon Platinum:
- Shown in its ORIGINAL Platinum look, in 3D: a new 3D engine draws Platinum's own map models (scope change,
  2026-10-31: replaces the earlier plan of pre-rendered pictures)
- The game's EXISTING battle system as is (Gen 4 species, moves, abilities and items are already in).
- New work: the 3D engine, overworld (models, collision, height), scripts/story, data.
- The PÓKETCH as a KEY ITEM: registerable to a hotkey, also reachable from the menu, drawn as an overlay
  on the main screen, mouse control or controller controls while focused (no second screen), usable in every region.

## Full feature parity (the user, 2026-10-31)
Sinnoh gets everything JOHTO and KANTO got:
- All three card games playable there (POKéMON cards, YU-GI-OH!, TRIPLE TRIAD): card players, a CARD CLUB / GAME
  CENTER equivalent, sellers, cards obtainable in the region (as COOP_NOTES "Cards from all three games ... in every
  region").
- Wild POKéMON pools that grow with GENERATION up to Gen 9 (as gen_hoenn_modern.py / gen_modern.py), trainers too.
- Every existing item by GENERATION: shops, pickups, held items, hidden items, trainers' items.
- Co-op, achievements, MoeGatcha, level scaling, followers, the Garden and the rest work there as in the other regions.

Removed, with the scripts behind them (they will never work here): PAL PARK, the GTS, Wi-Fi features (the Wi-Fi
Club, the Plaza, the Battle Tower's Wi-Fi rooms, Mystery Gift by Wi-Fi...). Removed where a Kai feature replaces
them: BALL SEALS (the POKé BALL page's styles and ball effects), and anything else the co-op features supersede.
Features that Gen 3 also has use the Gen 3 versions: POKéMON CONTESTS (not Super Contests), and the like. The
Underground and Super Contests in step 6 below are dropped unless the user asks again.
A new TRAINER CARD for Sinnoh comes later (the user's own design).

## What the game already gives us
- Regions: mp_region.c, "<save>.region", converters (tools/quetzal_port/), region chain from 1.61.4:
  after a League's credits play goes on from the save; beating a region's League lifts its level reset;
  the NATIONAL DEX from the start in every region. Sinnoh should follow the same rules.
- GENERATION setting (3-9, SEALED POKéMON shown gray since 1.61.5), level scaling, region shops that
  follow the original games' lists and unlock with badges + GENERATION (1.61.5: Crystal's for JOHTO,
  FRLG's for KANTO; Sinnoh would get Platinum's).
- Legendaries on the map wait for the region's League unless its own game gives them before it (1.61.6:
  qz_specials.c PreLeagueLegend). Sinnoh would follow Platinum's own order.
- Gen 4-9 POKéMON fully presented as of 1.61.5: cries, pictures, animations, Stadium colors.
- Co-op: friends can join gym/route trainer battles in the newer regions and share BADGES (1.61.4).
  Sinnoh's trainers should be joinable the same way (MP_Coop_JoinTrainerOk).
- Saves: since 1.61.5 there are three save files, each in its own folder under saves/ (saves/slot1...),
  with all its sidecar files. Any new Sinnoh sidecar belongs in that slot folder.
  Save layout never changes (tools/test/save_layout.py); migrations go in mp_savefix.c.
- Frame overlays (mp_compass.c, mp_ballfx.c) and registered-item hotkeys, which the PÓKETCH can reuse.
- mp_mappic.c (GAME CENTER: pictures pinned to maps): not used for Sinnoh any more (the 3D engine draws it).

## The 3D engine (scope change, 2026-10-31)
Platinum's overworld is real 3D (the DS GPU): map blocks of 32x32 tiles, each a textured model, plus building
models, texture / pattern animations (water, flowers), per-area lights that follow the time of day, and a tilted
perspective camera. The engine draws those models as Platinum does instead of pictures made from them.
- Data, read from the ROM by tools (never committed): the map matrix (which block sits where, per map),
  land_data blocks (model NSBMD, tile permissions, BDHC height data, the block's building list), building models
  (build_model), textures (NSBTX), texture and pattern animations (NSBTA / NSBTP), area lights and the camera
  per map. Reference: pokeplatinum's src/field/, src/overlay*/ (map_matrix, land_data, bdhc, area_data).
  A converter turns them into a compact form the game loads at run time (like graphics/ygo/edo.pak).
- Renderer: a DS-like software rasterizer (textured triangles / quads, vertex colors, alpha, the DS's per-polygon
  lights, fog, depth) drawing into the game's own frame. Software, so everything that reads the frame keeps working:
  display.c's scaling and filters, SPECTATE streaming and pop-outs, `shot`, headless tests with the dummy video driver,
  Wine. The DS drew about 2048 polygons at 256x192 a frame: easily done on the CPU.
- Sprites: the overworld objects (players, the LOOK, followers, NPCs, co-op friends, nametags, emotes) stand in the
  3D scene as camera-facing billboards at their tile and height, hidden behind the map where it's in front of
  them, as on the DS. The frame overlays (mp_compass.c, mp_ballfx.c, the PÓKETCH) stay drawn over the frame.
- The game logic stays on Emerald's grid: Sinnoh maps are still maps of the field engine (map headers, events,
  warps, scripts, connections, object events, the collision grid from Platinum's tile permissions), so movement,
  scripts, co-op sync, followers and the rest need no new logic. The 3D engine replaces only how a Sinnoh map is
  drawn, plus heights: BDHC gives the ground's height under each object (slopes, stairs, bridges, Mt. Coronet,
  the League), used for drawing and for which layer of a bridge an object is on.
- Camera: Platinum's per-map camera (distance, angle, field of view) and the scripted camera moves of cutscenes.
- Battles, menus and everything else stay as they are (2D, 240x160).

## The plan (high level)
1. The 3D engine: ROM reader + converter, the software renderer, a test viewer (a map block from a command line,
   compared against reference pictures), then inside the game: one Sinnoh map drawn in 3D with billboarded sprites.
2. Collision and height: Platinum's tile permissions -> Emerald behaviours; BDHC -> heights under objects, bridge
   layers.
3. Data converters: trainers, wild, items, marts (no text: SINNOH's words are written for this game); small new systems for Honey Trees, Poké Radar,
   swarms and the Great Marsh; Rock Climb and Defog as field moves.
4. Scripts: written for this game, with its own words (nothing converted from Platinum's scripts). This is the bulk of the work
   (Team Galactic, the lakes, Spear Pillar, the Distortion World; the Distortion World's own gravity and camera
   need the 3D engine's camera moves).
5. PÓKETCH Key Item and its apps (touch apps use the D-pad cursor or mouse). Self-contained, can come early.
6. Not planned: Super Contests (Gen 3 contests instead); the Underground only if the user asks.

## Decided (the user, 2026-10-31)
- The map screen is Emerald's: Sinnoh is drawn at 240x160 (Platinum's camera fitted to it: its field of view
  widened from 256x192's 4:3 to 3:2, the same height of the world shown). Emerald's battle system and menus stay as
  they are, so nothing else changes size.
- Native resolution first (the game's 240x160, with display.c's usual scaling and filters); a higher-resolution 3D
  option may come later.

## Music
Done (2026-10-31): the user's Diamond and Platinum 2SF rips became 127 song slots of SINNOH's own (MUS_SINNOH_*,
678-804) and music/mus_sinnoh_*.kaisong through Moemon Music Studio (tools/kaisong/sinnoh_songs.py: the track -> slot
table, `slots` writes the slots, `convert` the files). Towns and routes have Platinum's night versions as <slot>_night,
which S3D_MapMusic plays from 20:00 to 4:00; SINNOH's battles and victories pick their own slots (pokemon.c
MpSinnohBattleBGM / MP_RegionVictorySong on a SINNOH map). Left out: Wi-Fi, GTS, Mystery Gift, Super Contests,
Poffins, the Underground, title and opening. New maps take their slot as their map.json music (tools/sinnoh/gen_maps.py).
Tests sinnoh_music (day, battle), sinnoh_music_night.

## First step
A proof of concept: Twinleaf Town + Route 201 drawn by the 3D engine from the user's ROM, the LOOK walking on
them with correct collision and height, warp in from HOENN.

## Risks
The 3D engine is new and large (the DS GPU's look: texture formats, lighting, fog, edge cases); scripts are still
the biggest job; BDHC heights and the camera in cutscenes are the most likely to surprise; it needs the user's ROM.

## Rules to keep
Main update, not a dev update; bump MP_PROTOCOL_VERSION on protocol changes; mark stock-file hooks
"Moemon Co-op" / "Moemon Kai"; test headless per CLAUDE.md (no missable moves); release = MOEMON_VERSION +
tools/release/notes/v<ver>.md (player-facing only); no PR unless asked.
