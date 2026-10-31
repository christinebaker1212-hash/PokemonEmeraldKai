# HANDOFF: Sinnoh (Pokémon Platinum) region for Moemon Emerald Kai Co-op

Queued after the 1.61.6 release and the KANTO / JOHTO sweep (the user's plan, 2026-10-31).

## Where things stand
- This is planning only: no Sinnoh code exists yet. Start any work from the latest default branch.
- Read CLAUDE.md, BUILDING_COOP.md, docs/REGIONS_PLAN.md and COOP_NOTES ("JOHTO and KANTO",
  "Gen 4 battles", "Gens 5-9 and GENERATION", "The GAME CENTER") first.
- Reference decomp: https://github.com/pret/pokeplatinum
- The user's ROM: Google Drive file 13njqEG8nKIltrVkMAgXtM5E6X3dV33as,
  "Pokemon - Platinum Version (USA) (Rev 1).zip" (about 51 MB; the .nds inside). Download it into its own folder
  outside the repo; tools read it; ROM-derived files are never committed (same rule as the Quetzal rip).

## The goal
Add SINNOH as a third region beside JOHTO / KANTO, built from Pokémon Platinum:
- Shown in its ORIGINAL Platinum look (no Gen 3 redraw)
- The game's EXISTING battle system as is (Gen 4 species, moves, abilities and items are already in).
- New work: overworld (pictures, collision, height), scripts/story, data.
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
- Map pictures past the GBA's tile limits with an "over" layer: mp_mappic.c (GAME CENTER).
- Frame overlays (mp_compass.c, mp_ballfx.c) and registered-item hotkeys, which the PÓKETCH can reuse.

## The plan (high level)
1. Maps: pre-render Platinum's 3D maps offline (orthographic camera at Platinum's tilt, so the
   16x16 grid stays exact) and show them through mp_mappic.c with "over" layers; looping frames for water etc.
2. Collision and height: Platinum's tile permissions -> Emerald behaviours; BDHC heights -> elevations
   and sprite offsets (stairs, slopes, bridges, Mt. Coronet, the League).
3. Data converters: trainers, wild, items, marts, text, music; small new systems for Honey Trees,
   Poké Radar, swarms and the Great Marsh; Rock Climb and Defog as field moves.
4. Scripts: convert the common commands, write the cutscenes by hand. This is the bulk of the work
   (Team Galactic, the lakes, Spear Pillar, the Distortion World).
5. PÓKETCH Key Item and its apps (touch apps use the D-pad cursor or mouse). Self-contained, can come early.
6. Not planned: Super Contests (Gen 3 contests instead); the Underground only if the user asks.

## First step
A proof of concept: Twinleaf Town + Route 201 pre-rendered, the LOOK walking with correct collision,
warp in from HOENN, from the user's ROM above.

## Risks
Scripts are the biggest job; height/BDHC is the most likely to surprise; needs the user's ROM.

## Rules to keep
Main update, not a dev update; bump MP_PROTOCOL_VERSION on protocol changes; mark stock-file hooks
"Moemon Co-op" / "Moemon Kai"; test headless per CLAUDE.md (no missable moves); release = MOEMON_VERSION +
tools/release/notes/v<ver>.md (player-facing only); no PR unless asked.
