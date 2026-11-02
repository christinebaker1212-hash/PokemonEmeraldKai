# HANDOFF: Sinnoh (Pokémon Platinum) region for Moemon Emerald Kai Co-op (released in 1.63.0)

Queued after the 1.61.6 release and the KANTO / JOHTO sweep (the user's plan, 2026-10-31).

## Where things stand
- 2026-11-01, the latest: SINNOH is complete as planned. All eight gyms' moving puzzles (PASTORIA's water, SUNYSHORE's
  gears, CANALAVE's platforms, ETERNA's clock, VEILSTONE's bags, SNOWPOINT's sloped ice, HEARTHOME's dark rooms), the
  field obstacles (CUT / ROCK SMASH / STRENGTH with SINNOH's own HM badges), the story's people coming and going with it
  (tools/sinnoh/story_people.py: 256 hide flags), the DISTORTION WORLD (a scene in GIRATINA's arena: CYRUS, GIRATINA,
  CYNTHIA), fossils at the MINING MUSEUM, and the BATTLE FRONTIER's five facilities (TOWER, FACTORY through the game's
  own halls, HALL, CASTLE, ARCADE). Left out on purpose: the Underground, Super Contests, Wi-Fi, the VILLA, the Frontier
  Brains' and the HALL MATRON's teams, the DISTORTION WORLD's walking on walls.
- 1.63.1: a NEW GAME can start in SINNOH (BIRCH's menu: KANTO, JOHTO, HOENN, SINNOH): TWINLEAF bedroom, no team,
  the starter from ROUTE 201's briefcase, the POKéDEX from ROWAN (COOP_NOTES "A new game in SINNOH"). 1.63.3: the
  briefcase is Platinum's own screen, pre-drawn from its models and joint animations (tools/sinnoh/build_starter.py;
  COOP_NOTES "SINNOH's briefcase: Platinum's starter screen").
- RELEASED as 1.63.0 "Welcome to SINNOH!" (2026-11-01): sinnoh-work merged into the main branch (local `live`, pushed to
  the default branch claude/clever-clarke-hqati2); protocol 69, so friends online need 1.63.0. The release workflow
  builds SINNOH's packs itself (tools/sinnoh/make_pack.sh: fetch_decomp.sh PARTS 6 + build_pack.py + build_gfx.py) and
  ships them in sinnoh/. From here on SINNOH work goes on the main branch like any other feature.
- Ideas if the user wants more: Platinum's Frontier Brains (their teams would have to be made, Platinum's data isn't
  fetched), multi battles with the BATTLE SALON's partners in the TOWER, the DISTORTION WORLD's floors walked on
  walls (overlay009's 14,000 lines), the Underground, Super Contests. Tests to keep green: everything `sinnoh_*`
  (89 with the region, HM, POKéTCH, TOWER and SIM ones: `NOBUILD=1 JOBS=6 tools/test/regress.sh $(ls
  tools/test/scenarios | grep -E "^sinnoh_|^poketch|^honey|^region_|^hm_|^tower|^sim_")`).
- Proof of concept done (2026-10-31, branch sinnoh-work pushed to claude/exciting-sagan-17lq9j): TWINLEAF TOWN and
  ROUTE 201 drawn by the new 3D engine from the user's ROM, walked with Platinum's collision, the player and every field
  sprite standing in the scene (hidden by trees and roofs in front of them), lit by the area's lights for the hour.
  tools/sinnoh/test_poc.sh checks it against the offline reference render. COOP_NOTES "SINNOH's 3D engine" has the
  how. Heights (BDHC) done: the camera and every sprite stand on Platinum's ground (COOP_NOTES). Every outdoor map of the
  matrix is in (66, connected; tools/sinnoh/gen_maps.py). The buildings too (433 maps in groups
  41-44), with every place's warps: doors lead in, mats out. The pack ships compressed (7 MB). SINNOH is a region of its own
  (REGION_SINNOH: GENERATION cap 4+, badges from flag 0x3320, CANALAVE's dock for the boat, off the boat's list
  until its story is in), with Platinum's wild POKéMON (gen_wild.py). Next: data (step 3: trainers, items, marts),
  scripts and text (step 4: Platinum's OWN words and scripts, converted when the game is built and never committed
  (DECIDED by the user, 2026-11-02, undoing a session's 2026-10-31 call to write original words instead: "we have
  all the original dialogue and scripts"; tools/sinnoh/build_text.py -> sinnoh/platinum_text.pak, tools/sinnoh/
  gen_scripts.py -> sinnoh/platinum_scripts.inc, both by make_pack.sh; COOP_NOTES "Platinum's own words and
  scripts"); the words written for this game (below) stand in only where a script doesn't convert yet; tools/sinnoh/words.py + gen_words.py, who.py to see
  a map's people: TWINLEAF, ROUTE 201 and SANDGEM written, COOP_NOTES "Words"; done 2026-11-01: item balls, hidden
  items, the 403 route trainers with lines of their own, TM51-TM92, the gym leaders, the ELITE FOUR and the CHAMPION
  (the region cleared), boats inside SINNOH and the boat from the other regions: SINNOH is playable from start to
  HALL OF FAME; the GREAT MARSH's SAFARI GAME with Platinum's daily POKéMON; wild POKéMON by the time of day, the daily
  swarms and HONEY trees), the PÓKETCH (step 5);
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

## Platinum's own words and scripts: where it stands (2026-11-02) and what's next

The user's call (2026-11-02): SINNOH's dialogue and scripts are Platinum's own, converted from the decomp at build
time and never committed. Never write original words or scripts for SINNOH again.

Done:
- Words: tools/sinnoh/build_text.py -> sinnoh/platinum_text.pak (the 685 message banks, plus one bank of every trainer's
  own lines: build_text.trainer_message()). Trainers' intro / defeat / after-battle lines are Platinum's
  (gen_trainers.py writes only their pack numbers into data/scripts/sinnoh_trainers.inc).
- Scripts: tools/sinnoh/gen_scripts.py converts Platinum's event scripts to this game's commands
  (sinnoh/platinum_scripts.inc, made by make_pack.sh before the builds; the Makefile's `--auto` rule; stubs without
  the decomp). It follows calls into scripts_common.s, translates movements (30 fps -> 60: delays doubled), maps
  items / species / moves / trainers by name to this game's (gen_trainers.RENAME), Platinum-only constants from the
  decomp's generated/*.txt and include/constants, flags / variables through ids.py (never change ids.py's numbering:
  saves hold them). The `sinnoh OP, ARG, ARG2` command (0xF6, src/sinnoh/s3d_scrcmd.c) covers what this game's
  commands lack (names into buffers, YES = 0, Platinum's facing / position / pockets / badges / TM move names /
  NATIONAL DEX).
- Coverage: `python3 -I tools/sinnoh/gen_scripts.py --report POKEPLATINUM_DIR` (1804 of 2511 entries at the end of
  2026-11-02). gen_words.py (given the decomp) points every person / sign / bg whose script converts at it; the game's
  roles (nurses, clerks, gym leaders, ELITE FOUR, CHAMPION, boats, gifts that don't convert) keep this game's scripts.

The user's rules for the rest (2026-11-02):
- **Kai's systems stay.** Anything Kai already has (marts, nurses, gyms and their puzzles, the BATTLE TOWER and the
  frontier, partner battles, two-trainer battles, the camera object, the bike, followers, the starter briefcase, the
  heal spots...) is used by SINNOH as it is: a Platinum command maps onto Kai's system, never a second copy of it.
  Examples so far: StartTagBattle = Kai's partner battle (battle_tower.c MP_StartPartnerBattle, the one Quetzal's LANCE
  uses; DoSpecialTrainerBattle 15) with STEVEN's party pick; StartTrainerBattle with two = the game's own two-trainer
  battle (battle_setup.c MP_StartTwoTrainerBattle); AddFreeCamera = special SpawnCameraObject (the 3D view follows it);
  StartChooseStarterScene = Sinnoh_StarterMenu; GiveRunningShoes = FLAG_SYS_B_DASH.
- **Everything else 1:1 with Platinum.** What isn't, gets built (the DISTORTION WORLD first: it isn't 1:1).
- **Closed:** online-only features (GTS, Wi-Fi Club / Plaza, global rankings, distribution events: CheckDistributionEvent
  is always 0), and features that Kai supersedes or that rely on the DS itself (the PAL PARK and its GBA slot, the
  second screen's own uses: the POKéTCH is Kai's).

Where it stands:
- Map scripts (scripts_init_*.s: transition / load / resume and frame tables), Platinum's people (their hide flags,
  places and converted scripts) and triggers are emitted per map as gSinnohPlatinumMapScripts and swapped into
  gMapHeader when FLAG_SINNOH_PLATINUM_STORY (0x3D3B) is set (s3d_scrcmd.c Sinnoh_PlatinumMapScripts; the committed
  maps are untouched; a person whose script is one of Kai's roles keeps it). Sinnoh_StoryRefresh stands aside.
- Sinnoh_PlatinumStoryBegin (debug `mp platinumstory`) sets the flag and runs Platinum's own scripts_init_new_game.
  SINNOH is always Platinum's story (the user, 2026-11-02: no saves partway through the written one, no migration):
  any SINNOH map load without the flag begins it.
- Kai's ferry between the regions (CANALAVE's harbor sailor, EventScript_SinnohRegionBoat) shows only while the boat
  can sail (SINNOH's ELITE FOUR and the S.S. TICKET, from the CHAMPION's win) and on the first boat in, as JOHTO's and
  KANTO's ports (COOP_NOTES "CANALAVE's ferry sailor").
- Story coverage (map scripts, triggers, story people): 500 of 760 at the end of 2026-11-02; the rest are mostly Kai's
  place systems (REGI ruins, TURNBACK CAVE, platform lifts, gym puzzles, the frontier), which stay Kai's.
- Platinum's key items are items 662-684 (GALACTIC KEY ... GOOEY MULCH; Platinum's descriptions from PokeAPI, stand-in
  icons); the trainers its scripts battle (story scenes, the restaurant, rematches, tag partners) are appended to
  trainers_sinnoh.h (ids before them unchanged).

Done since (2026-11-02, later): walking partners (event_object_movement.c MOVEMENT_TYPE_FOLLOW_PLAYER; across maps as
LOCALID_FOLLOWER 0xF2, s3d_scrcmd.c Sinnoh_PartnerUpdate; Kai's POKéMON follower walks behind them), Platinum's
end-of-line waits (build_text.py: a message ending in "\r" waits for A), music that really stops (StopMapMusic, not
the game's pausing fadeoutbgm), the SINNOH POKéDEX (src/data/sinnoh_dex.h, tools/sinnoh/gen_sinnoh_dex.py), BARRY's
first battle on Kai's early-rival battle (Quetzal's trainerbattle 13), the catching tutorial on Emerald's WALLY
tutorial (battle_setup.c Sinnoh_CatchingTutorial, DAWN / LUCAS from Kai's back pictures), the counterpart's look
(VAR_OBJ_GFX_ID_n), Platinum's gym leaders / ELITE FOUR / CHAMPION scripts in place of the written ones where they
convert, every script-number range (SCRIPT_RANGES). The story plays from the TV in the player's room through ROUTE 201,
the briefcase and BARRY's battle to mom's Running Shoes (tests sinnoh_platinum_walk, sinnoh_platinum_route201).

To build (not 1:1 yet):
1. Partner wild battles: written (battle_setup.c TryPartnerWildBattle, test sinnoh_platinum_partner), being verified.
2. The DISTORTION WORLD 1:1: walking on walls and ceilings, its rotating rooms, the boulders and the lift, CYRUS and
   GIRATINA's scenes (now a scene in GIRATINA's arena: data/scripts/sinnoh_places.inc). Platinum's is
   src/overlay009/ov9_02249960.c (14,000 lines: DistWorldSystem; per-floor map info files with camera angles, moving /
   floating platforms and their jump points, ghost props, sky clouds, map objects that turn with the floor, event
   commands; its tables sMapEvents, sMovingPlatformsMapTemplates, sDistWorldMapConnectionList...). Its scripts need
   InitPersistedMapFeaturesForDistortionWorld, AddDistortionWorldMapObject, the DIST_WORLD_MAP_OBJECT_* ids,
   DIST_WORLD_PROGRESS_*, GetPlayer3DPos, SetPartyGiratinaForm, StartGiratinaOriginBattle. The 3D engine (s3d.c) needs
   a player that walks on any face (gravity per tile) and the camera Platinum's angle templates give.
3. The gym puzzles Kai doesn't have yet (ETERNA's clock: AdvanceEternaGymClock; VEILSTONE's, HEARTHOME's), the VILLA,
   the Frontier Brains' and the HALL MATRON's teams, the PASTORIA size contest, JUBILIFE's TV lottery, the JOURNAL,
   ROTOM's room, Platinum's POKéDEX ratings (LoadPokedexRating), UNOWN messages, DAWN's / LUCAS's own overworld
   sprites (stand-ins now), Platinum's songs the game lacks (`gen_scripts.py --report` lists them: SEQ_OPENING...).
4. A story test from TWINLEAF to the HALL OF FAME; then the written words go (words*.py, STORY_WORDS, trainer_words.py)
   once nothing uses them.
5. Decided by the user (2026-11-02): SUPER CONTESTS and their own functions are built (POKé BALL effects in the
   Seals' place, in the story, menus and shops); Platinum's TV is built (src/sinnoh/s3d_tv.c, done: COOP_NOTES
   "SINNOH's TV"). The Underground (the user, 2026-11-02): ONLY its fossil digging and SPIRITOMB's prerequisites;
   its wireless sides (secret bases, spheres, traps, flags) are superseded by CO-OP (done: COOP_NOTES "The
   Underground").
The working checklist: docs/SINNOH_CHECKLIST.md.

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
Features that Gen 3 also has use the Gen 3 versions, but for SUPER CONTESTS, which the user asked for again
(2026-11-02: built, with POKé BALL effects in the Seals' place). The Underground is built for its digging and
SPIRITOMB only (the user, 2026-11-02).
A new TRAINER CARD for Sinnoh comes later (the user's own design).
People look as in Platinum (the user, 2026-11-01): its overworld sprites and trainers' battle pictures from the decomp,
in sinnoh/platinum_gfx.pak (tools/sinnoh/build_gfx.py; COOP_NOTES).

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
3. Data converters: trainers, wild, items, marts, text (Platinum's own: build_text.py); small new systems for Honey Trees, Poké Radar,
   swarms and the Great Marsh; Rock Climb and Defog as field moves.
4. Scripts: Platinum's own, converted (tools/sinnoh/gen_scripts.py; DECIDED by the user 2026-11-02: never rewrite
   them or their words). This is the bulk of the work
   (Team Galactic, the lakes, Spear Pillar, the Distortion World; the Distortion World's own gravity and camera
   need the 3D engine's camera moves).
5. PÓKETCH Key Item and its apps (touch apps use the D-pad cursor or mouse). Self-contained, can come early.
6. (Superseded: SUPER CONTESTS and the Underground's digging were built after all, the user's later calls.)

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
