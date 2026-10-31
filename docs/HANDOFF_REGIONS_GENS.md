# Handoff: JOHTO / KANTO extras, playthroughs and the GENERATION audit

Updated after every commit of this work, so another session can pick it up. Read CLAUDE.md first, then
docs/REGIONS_PLAN.md and COOP_NOTES "JOHTO and KANTO" / "Gens 5-9 and GENERATION" / "The battle GIMMICKS".

Branches: work on `claude/exciting-sagan-17lq9j` (worktree /home/user/g5, local branch gens5-9-work, pushed with
`git push origin HEAD:claude/exciting-sagan-17lq9j`). Releases: the default branch `claude/clever-clarke-hqati2`
(fast-forward it to the work branch; the release workflow runs when include/mp_version.h changes, but GitHub's path
filter sees only 300 files a push, so run release.yml by workflow_dispatch on that ref when a push is big).

## What the user asked for, in order
1. Finish the JOHTO / KANTO extras (list below), then play through both regions from a new game to catch script
   errors and breakage.
2. Each gimmick on its own row of HOST A GAME > GAME RULES. (DONE)
3. After 1: playthroughs JOHTO -> KANTO, KANTO -> JOHTO, HOENN -> KANTO, HOENN -> JOHTO, KANTO -> HOENN,
   JOHTO -> HOENN. No script may break along the way.
4. After 3: audit what was found; every item attainable at each GENERATION; every POKéMON obtainable at each
   GENERATION (wild, prizes, in-game trades, gifts) in some capacity, HOENN at GEN 3 included; the NATIONAL DEX
   limited to the species of the GENERATION (GEN 3: 386, GEN 4: 493, GEN 5: 649, GEN 6: 721, GEN 7: 809,
   GEN 8: 905, GEN 9: 1025). Note Kai's species ids are not national numbers past 386 (Gen 4 = 387-493 in Kai's
   own order, Gens 5-9 = species 519-1050): count by national dex number (SpeciesToNationalPokedexNum).
5. Keep this file up to date after each commit.

## Done
- 1.61.0 released (Gens 5-9, gimmicks, GENERATION).
- Bug-Catching Contest (src/qz_bugcontest.c, test qz_bugcontest), RUINS OF ALPH sliding puzzles
  (src/qz_alphpuzzle.c, test qz_alph_puzzle), scripts' -1 warp coordinates (event_data.c GetVarPointer).
- GAME RULES gimmick rows (mp_setup.c RULE_MEGA..RULE_TERA, test host_rules).
- JOHTO / KANTO at GEN 3 sealed every POKéMON with a later ability (mp_region.c UseCap: never below Gen 3; the
  Gen 2 cap is only for the regions' pools). Credits after their HALL OF FAME (credits.c no-catch guard,
  post_battle_event_funcs.c region-aware GameClear, qz_specials.c QzDoCredits / QzAfterCreditsReset; test region_hof).
- 1.61.1 released (the seal fix, contest, puzzles, credits, GAME RULES gimmick rows).

## Next steps
1. Check the 1.61.1 release run finished (GitHub Actions release.yml on the default branch).
2. Extras still open (task list):
   - Quetzal postgame events, still stubs in src/qz_specials_stub.c (each logs and re-enables the script, so
     none freezes, but the events don't happen): GIOVANNI's story (time travel to the past), dream events,
     SS ANNE dream / exterior events, SEVAULT house, PALLET postgame, QzTimeTravelChoosePartyMon. Port them from
     the Quetzal disassembly notes in docs/quetzal/ (move each from the stub file to src/qz_*.c;
     tools/quetzal_port/import.py regenerates the stub list from what src/qz_*.c defines).
   - HEADBUTT trees, FRLG door animations, 3x2 map borders.
   - Sightseeing walk on first arrival, SAFFRON station, everyone following the host on a region trip,
     per-region co-op shared PCs.
3. Playthroughs of JOHTO and KANTO from a new game (MOEMON_DEBUG_REGION=1 / 2): story order, every gym, rival,
   key event; plus an automated sweep that talks to every NPC / sign / trigger of every Johto and Kanto map
   (mp warp + A on each object event) and greps logs for "not ported", crashes, stuck script contexts.
4. Cross-region playthroughs in all six directions (item 3 above): tickets / boats / trains between regions,
   per-region state kept, levels, PCs, return trips.
5. The GENERATION audit (item 4 above): a script under tools/ that, for each GEN 3-9, lists species obtainable from
   wild tables (all three regions, by MP_Gen pools), static / gift / trade scripts, prizes (GAME CORNERS, contests,
   MoeGatcha), evolutions and breeding of those; and every item's sources (marts by GEN, pickups, held items,
   hidden / ball items). Fill the gaps (wild slots, trades, prizes). Then cap the POKéDEX's NATIONAL count and
   lists at the GEN (pokedex.c, the dex completion checks / diplomas, MoeGatcha). Tests per GEN.
