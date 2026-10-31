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
5. After 4: every card of every card game (POKéMON TCG, Yu-Gi-Oh!, Triple Triad) obtainable in some way in every
   region (HOENN, JOHTO, KANTO): shops, duelists' prizes, packs, trades. Every NPC that can be challenged to a card
   game must always be reachable to talk to (not behind a counter / blocked, not hidden by flags after events).
   A GAME CENTER in JOHTO and KANTO too, placed where it fits best in each region.
6. Final pass: audit the achievements and add more for each region.
7. Keep this file up to date after each commit.

## Done
- 1.61.0 released (Gens 5-9, gimmicks, GENERATION).
- Bug-Catching Contest (src/qz_bugcontest.c, test qz_bugcontest), RUINS OF ALPH sliding puzzles
  (src/qz_alphpuzzle.c, test qz_alph_puzzle), scripts' -1 warp coordinates (event_data.c GetVarPointer).
- GAME RULES gimmick rows (mp_setup.c RULE_MEGA..RULE_TERA, test host_rules).
- JOHTO / KANTO at GEN 3 sealed every POKéMON with a later ability (mp_region.c UseCap: never below Gen 3; the
  Gen 2 cap is only for the regions' pools). Credits after their HALL OF FAME (credits.c no-catch guard,
  post_battle_event_funcs.c region-aware GameClear, qz_specials.c QzDoCredits / QzAfterCreditsReset; test region_hof).
- JOHTO / KANTO's overworld POKéMON of Gens 5-9 (146 objects: legendaries, MAGEARNA, REGIELEKI...) and 131 more
  static encounters, hidden by the rip's Gen 4 cap, are back from their GENERATION on
  (tools/quetzal_port/gen_later_objects.py + gen_static.py; mp_region.c MP_Gen_ObjectHidden; test region_later_mons).
  Rerun both after tools/quetzal_port/import.py.
- KANTO's FireRed `textcolor` ran its color as a command (2 = end, 3 = return) and cut 41 scripts short (S.S. ANNE
  captain, OAK's LAB...): ScrCmd_textcolor; check_scripts.py guards it (test qz_textcolor).
- Graphics variables (VIRIDIAN's old man, FUCHSIA, BIRTH ISLAND, ICE PATH, MELOETTA): event_data.c QzVarGraphicsId,
  tools/quetzal_rip/var_sprites.py for the sprites only scripts name.
- MELOETTA's CHANSEY Dance in SEVAULT CANYON (QzSevaultHouseEventEligibility; test qz_meloetta_dance).
- 1.61.2 released with those.
- The DARKRAI dreams (JOHTO's bed at home -> DARK CAVE; KANTO's S.S. ANNE cabin) and PALLET's ARCEUS vision
  (qz_specials.c; tests qz_dream, qz_dream_kanto). DARKRAI's statics show only in a dream (RefreshStatics).
- The S.S. ANNE truck and MEW (test qz_mew_truck).
- HEADBUTT trees and ROCK SMASH in JOHTO / KANTO (test qz_headbutt); tools/test/npc_sweep.sh.
- The NPC sweep's first finds, fixed: every JOHTO mart clerk crashed (badge marts), CUT on JOHTO's trees crashed,
  KURT's / the underground shops divided by a zero price, JOHTO / KANTO's Triple Triad sellers crashed.
  KANTO's CUT / ROCK SMASH objects and every JOHTO / KANTO in-game trade crashed (fixed; test qz_trade); KANTO's
  nurses froze the game (fixed; test qz_nurse_kanto).
  The sweep skips card duelists (their own tests); check them in JOHTO / KANTO as part of the cards step.
- SAFFRON's MAGNET TRAIN counter (test region_train_saffron).
- Friends follow the host's trip (test region_coop_follow).
- Regions' first-arrival setup (Quetzal's hidden-at-start objects) and GIOVANNI-story flag refresh on JOHTO warps.
- Six cross-region round trips (tools/test/gen_region_trips.py, tests region_trip_*); story_johto plays JOHTO's
  opening to the starter.
- 3x2 borders for KANTO's seven FireRed layouts; door animations: none in Quetzal either (left as is).
- 1.61.1 released (the seal fix, contest, puzzles, credits, GAME RULES gimmick rows). 1.61.2 released.
- The MOéMON GARDEN and the GAME CENTER floors are shared by every region (no trip into HOENN from JOHTO / KANTO;
  back out where the player came in): tests garden_johto, gc_johto, gc_kanto.
- New-game breaks found by tools/quetzal_port/story_vars.py and checks around it (all fixed, COOP_NOTES "JOHTO and
  KANTO"): Quetzal's system flags (POKéMON / POKéDEX / RUNNING SHOES / STRENGTH) weren't Emerald's; STRENGTH couldn't
  push Quetzal's boulders and FireRed's boulder switches never ran; GOLDENROD's elevator couldn't be entered;
  NEW BARK's house special was a stub. tools/test/story_sweep.sh (fires all ~500 story triggers) is the next check:
  run it, work through its report (FAIL / HELD / NOFIRE / STILL), as with npc_sweep.sh.
- NPC sweep recheck of the 67 maps: all pass; the last holds fixed (TRAINER TOWER sign, CERULEAN's badge list) or
  B-ignoring menus with an EXIT row (not bugs).
- GENERATION audit (item 4): tools/moemon_gen59/obtain_audit.py + item_audit.py, run by regress.sh; every POKéMON
  and item has a source at GEN 3-9 (COOP_NOTES "Gens 5-9 and GENERATION", "Everything obtainable"). The NATIONAL
  DEX is limited to the GENERATION (MP_Gen_DexCount). Caveat: the audits read sources from the data; they don't
  prove a place is reachable in the story (a static behind a postgame flag counts).

- Story sweep round 3 fixes: JOHTO's ROCKET HIDEOUT multi battle beside LANCE (Quetzal's special battle 12; the party
  comes back after it; LANCE shown from behind with his LOOK's back picture; test story_rocket_lance); a special done
  at once (ScriptContext_Enable) before a waitstate froze the script (scrcmd.c; S.S. ANNE's departure, every stub;
  test story_ssanne); stubs answer VAR_RESULT 0 ("not available"); JOHTO's HALL OF FAME (test region_hof_johto);
  achievements for JOHTO / KANTO (test ach_regions). The test-only MOEMON_TEST_FOE_HP1 no longer writes into the
  player's party in two-trainer battles. story_sweep.sh: on-warp scripts and no-variable triggers aren't NOFIRE;
  20000 frames per event (the DANCE THEATER's five battles).

## Next steps
1. Ship 1.61.3 (hotfix: JOHTO mart clerks, CUT, KURT's shop, Triple Triad sellers crashed; plus the extras since
   1.61.2) once the NPC sweep's report is worked through: bump include/mp_version.h, tools/release/notes/v1.61.3.md,
   tests, Windows build, fast-forward the default branch, check the release run.
2. Extras still open (task list):
   - GIOVANNI's past: DONE (COOP_NOTES "GIOVANNI's alternate past"; tests story_giovanni, story_giovanni_no).
   - BILL's time travel: REMOVED (the user: "Remove it entirely"; COOP_NOTES "BILL's time travel"). No Quetzal special
     is a stub any more (src/qz_specials_stub.c is empty).
   - Sightseeing walk on first arrival (a guide in the port city before the ride to the first town).
   - Per-region co-op shared PCs (the server's 14 shared boxes, one set per region).
3. (The user: "In the New Game playthrough sweep, be absolutely certain that every story event triggers correctly.")
   Playthroughs of JOHTO and KANTO from a new game (MOEMON_DEBUG_REGION=1 / 2): story order, every gym, rival,
   key event; plus an automated sweep that talks to every NPC / sign / trigger of every Johto and Kanto map
   (tools/test/npc_sweep.sh does this now: run it, fix what it reports) and greps logs for "not ported",
   crashes, stuck script contexts.
4. Cross-region playthroughs in all six directions (item 3 above): the round trips pass (region_trip_*); still to do:
   play each destination's opening after a first arrival by boat (party not empty: story_johto-style), and extend
   story_johto past the starter (ROUTE 29, MR. POKeMON, the rival) plus a KANTO one (OAK, the parcel).
   Also port Quetzal's legendary unlock conditions (COOP_NOTES "JOHTO and KANTO": special 704 / 757's kind switch).
5. The GENERATION audit (item 4 above): DONE (see Done). Left: MoeGatcha's random picks are still 1-386 at every
   GENERATION (mp_gatcha.c RandomSpecies) - could widen to MP_Gen_DexCount; the per-region JOHTO / KANTO dexes
   aren't touched.
6. Cards in every region (item 5 above): list where each card comes from per region (tools/card_guide data,
   tcg_field.c / ygo_field.c / tt_field.c pickers, shops); add sellers / duelists / packs in JOHTO and KANTO where
   a card is missing; check every challengeable NPC's position and visibility flags (an automated walk-up test);
   Cards: DONE (every card at the GAME CENTER, reachable from every region: COOP_NOTES "Every card in every region").
   GAME CENTER: DONE for JOHTO / KANTO (an attendant in GOLDENROD's / CELADON's GAME CORNER, 2F and 3F only, by the
   user's rule; COOP_NOTES "The GAME CENTER"). Still to do here: a per-region card audit (which cards only come from
   HOENN's special trainers: TCG promos (DONE: JOHTO / KANTO's special trainers, tcg_field.c sQzSpecials - the user:
   promos are won from the respective trainers as in HOENN, never sold), YGO signatures, TT 4-/5-star cards (NO
   seller gets these) and JOHTO / KANTO counterparts (their gym
   leaders, Elite Four, champions, rivals as special duelists giving the same prizes), checked by a tool like
   obtain_audit.py over the card guide's dumps (tools/card_guide).
7. Achievements (item 6 above): JOHTO / KANTO tabs added (32 achievements; COOP_NOTES "Achievements for JOHTO and
   KANTO"). Could add more: story beats (RADIO TOWER, SILPH CO., S.S. ANNE, the SEVII ISLANDS), the Bug-Catching
   Contest, the RUINS OF ALPH puzzles, the GAME CORNERS.
8. (The user, during the playthrough tests:) run state passes beside the story checks: after each story milestone,
   check what the player sees changed with it - the START menu's rows and the NATIONAL DEX (harness mp startmenu,
   now in story_johto / story_kanto / story_kanto_dex), RUNNING SHOES, badges on the trainer card, FLY / the Town
   Map, the BAG's key items. The user found KANTO's menu without POKeDEX / POKeMON after the starter and the NATIONAL
   DEX on 1.61.2: the system-flag fix in 1.61.3 covers it (tests above), so it is not a known issue.
9. FINAL, FINAL PASS (the user's): be absolutely certain 2 and 3 players don't break any script or battle - every
   JOHTO / KANTO story scene, special battle (LANCE's multi battle uses the Triple Battle second ally's back picture
   slot: check with a friend in the room / in the battle), co-op doubles and triples, joining a battle, followers,
   region travel together; two- and three-client runs of the story sweep's scenes (MOEMON_LOBBY, barrier).
   Done so far: FILL / TEAM PADDING respects the GENERATION (coop_fill_gen3); a REVIVE on a friend with nothing out
   in co-op doubles / triples brings their Pokemon back on every screen (COOP_NOTES "Revived after having nothing
   out"; tests coop_revive, tri_revive_*). The two-player story sweep (tools/test/story_sweep.sh PLAYERS=2) ran over
   JOHTO and KANTO.
   Also done: JOHTO / KANTO trainers in co-op (joinable, badges for friends, BROCK's long speech crash: COOP_NOTES
   "JOHTO and KANTO", tests join_gym_johto / join_gym_kanto). The PLAYERS=2 sweep is clean (0 FAIL; OWN = a friend's
   own scene, ENDS = the credits). Left: PLAYERS=3 sweep; a co-op team-up with a JOHTO route trainer (wait for a
   partner) and a KANTO multi battle with a friend in the room (LANCE / GIOVANNI / ETHAN battles 12-14).
10. THE ABSOLUTE LAST PASS, after everything is committed and built: MoeGatcha - every banner by GENERATION, Gen 3
   (today's banners) as the base: newer POKeMON / items join each banner as the GENERATION rises (mp_gatcha.c;
   RandomSpecies is still 1-386 at every GENERATION). DONE (COOP_NOTES "Banners by GENERATION"; tests gatcha_gen3 / 9).
11. THE FINAL PASS AFTER THE MOEGATCHA ONE (the user, once everything else is finished, pushed, built and updated):
   every Moemon must have an equivalent POKeMON battle sprite. The user switched from Moemon appearance to POKeMON
   appearance and a battle showed nothing at all, and the game locked up. Check every species' front / back picture
   in that mode (Gens 4-9 especially, the forms, MEGA / gimmick pictures) and that a missing one can't freeze a battle.
12. AFTER EVERYTHING ABOVE (the user, in this order of asking): Pokemon Stadium nickname colors (mp_hue.c) for every
   new Pokemon (Gens 4-9); the modern Gen 4-9 cries sourced and added with a GBA sound filter pass; a token icon for
   Yu-Gi-Oh! tokens (ids past YGO_CARD_COUNT); SEALED (past the GENERATION) Pokemon marked with an icon and/or tint in
   the party, PC and summary.
13. (The user, after 12:) the department stores are redundant: both clerks of a floor sell the same extended list
   (shop.c sMpShopExtras is keyed by map only: LILYCOVE 3F's two clerks both get every battle item and MEGA STONE;
   GOLDENROD / CELADON the same). Spread the extended selection through the department store (each clerk / floor its
   own theme), and give sensible POKe MARTs in HOENN, JOHTO and KANTO a smaller selection that grows with progression
   (badges).
14. Instead of full cross-region playthroughs (item 4; the user, once the PLAYERS=3 sweep is clean): the NATIONAL DEX
   given at the start of a HOENN game (only the GENERATION's Pokemon shown), then one scripted chain per order
   (HOENN -> JOHTO -> KANTO and one other order): all badges + the Elite Four (forced by script) in a region, customs
   refusing two Pokemon and passing one Lv 70, Lv 5 on arrival, the 8 badges + Elite Four + credits there, GLOBAL PC
   linked, the Pokemon back at 70 if below it (and keeping a higher level: both branches), a save and reload between
   regions, then the third region.
15. AFTER EVERYTHING (the user): the save and its sidecars crowd the exe's folder. Without changing any file or its
   format, move them into a tidy folder structure; the auto-updater (and the game) moves existing files there.
   Then up to 3 completely isolated save files, each in its own folder (save + all sidecars), so they never mix.
   The slot is chosen on the title screen: the save files are listed there as additional files to pick (the user).
