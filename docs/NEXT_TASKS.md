# Next tasks (the user, 2026-11-01): the plan, in order

Written so another session can carry on if this one runs out. Work in a worktree of the session branch
(claude/exciting-sagan-17lq9j); releases go to the default branch claude/clever-clarke-hqati2 (CLAUDE.md "Key locations").
The user's mock-ups: docs/user_images/57.png (the starter table) and the drop shadow (sent in chat: a soft dark oval,
about 14x5 pixels, under the player's feet, see-through).

## 1. Release 1.63.1: a NEW GAME in SINNOH (DONE: pushed to the default branch 2026-11-01)
- BIRCH's region menu KANTO, JOHTO, HOENN, SINNOH; SINNOH's new game (COOP_NOTES "A new game in SINNOH"). The
  sinnoh_tower test now sets the frontier's foes to 1 HP (MOEMON_TEST_FOE_HP1 in CB2_InitBattleInternal) and presses
  A until the run ends: its fixed turn count broke when the tower's draw changed with the date.
- Check the release run on Actions (release.yml on claude/clever-clarke-hqati2) and that v1.63.1 has both zips.

## 2. KANTO / JOHTO starter table (DONE, 1.63.2): COOP_NOTES "The KANTO / JOHTO starter table and BLUE's teams"
- Also BLUE's teams as FireRed's in every battle (the user: "match it across the entire game"), and in 1.63.2 the
  SINNOH door fixes (BARRY's mom's visit scene; COOP_NOTES "SINNOH: nobody stands on a door").

## 3. SINNOH: Platinum's 3D briefcase (DONE, 1.63.3): COOP_NOTES "SINNOH's briefcase: Platinum's starter screen"

## 4. Drop shadows (DONE, 1.63.4): COOP_NOTES "Drop shadows and followers in grass and puddles"

## 5. Settings per save file, switching in place (DONE, 1.63.5): COOP_NOTES "Settings per save file, and switching
save files in place"

## 6. Card games: NEW tags and centered booster packs (DONE, 1.63.6): COOP_NOTES "NEW in the BINDER, centered
booster packs"

## 7. 40 PC boxes (DONE, 1.63.7; the user, 2026-11-01, "the final task"): COOP_NOTES "40 PC boxes"
- The player's own PC (14 boxes today, in the save: the layout can't change, tools/test/save_layout.py): 40 boxes, the
  26 more in a sidecar ("<save>.boxes" or the like, converted by mp_savefix.c) that MP_PersonalStorage() / the box
  screen read as boxes 15-40; box names and wallpapers too.
- The shared PC (mp_sharedbox.c: 14 boxes mirrored from the server) and the Global PC (the server's): 40 boxes each;
  the server (tools/mp_server) keeps them; protocol bump (MP_PROTOCOL_VERSION).

## Task list to date (this session's work, all done unless marked)
- SINNOH region (1.63.0, released): 3D engine, maps, people and words, trainers, gyms (moving puzzles), LEAGUE,
  HALL OF FAME, story people, legendaries, the DISTORTION WORLD, fossils, field obstacles, the POKéTCH, FLY and the
  TOWN MAP, wild by time / swarms / HONEY / POKé RADAR, GREAT MARSH, the BATTLE FRONTIER's five facilities, GAME
  CORNER, card players, SINNOH PEOPLE and SPRITE SCALING options, POKéDEX AREA, starters out of the wild, TM51-TM92,
  JOHTO's HGSS music.
- 1.63.0's release build fix (a letter the game's font lacks), re-queued, published with both zips and update.txt.
- NEW GAME in SINNOH + region menu order: released as 1.63.1 (task 1).
- 1.63.2: the starter table, BLUE's FireRed teams, SINNOH's door fixes.
- 1.63.3: SINNOH's briefcase.
- 1.63.4: drop shadows, small followers in tall grass, big followers' reflections.
- 1.63.5: settings per save file, SAVE FILE switches in place.
- 1.63.6: NEW in the BINDER, centered booster packs.
- 1.63.6's Windows build fix (math.h; the release re-queued).
- 1.63.7: 40 PC boxes (own, each region's, GLOBAL, shared; protocol 70).
