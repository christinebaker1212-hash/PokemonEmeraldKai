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

## 3. SINNOH: Platinum's 3D briefcase (after 2)
- pokeplatinum src/choose_starter/choose_starter_app.c; models res/prebuilt/graphic/ev_pokeselect.narc (0 BCA case
  opening, 1 closed case, 2/4/6 BCA + 3/5/7 the balls, 8 the open case, 9 the table; tools/sinnoh/nitro.py reads the
  BMD0s). Needs NSBCA joint animation baked into frames for the pack (build_pack.py), a small scene in s3d, and it
  replaces Sinnoh_StarterMenu's list (src/sinnoh/s3d_places.c).

## 4. Drop shadows (an option)
- OPTIONS > DISPLAY: SHADOWS ON / OFF. A see-through dark oval under the player, every NPC in every region (SINNOH's
  3D people too), followers and Garden POKéMON. Not drawn in water / while surfing / diving, for flying or
  hidden objects; sized by the sprite's width (big PMD sprites larger).
- Fix with it: some follower POKéMON vanish in tall grass (mp_followfx.c / the grass effect's priority over the
  follower), and some followers' puddle reflections sit far below where they should (the reflection's y for tall /
  big sprites: mp_followfx.c, fldeff reflection).

## 5. Settings per save file, and switching save files without restarting
- moemon_coop.ini, moemon_controls.ini, lobby.txt, moemon_display.ini and the card sidecar (pokeemerald.sav.cards)
  belong to the save file in use (saves/slotN, src/platform/save_slots.c), copied from the shared ones the first
  time a slot has none. Today: display.c CONFIG_NAME, gamepad.c "moemon_controls.ini", mp_main.c "lobby.txt",
  updater.c / the co-op config "moemon_coop.ini"; .cards is already "<save>.cards" (tcg_integration.c), so check why
  it looked shared to the user (keyed by Trainer ID: sCollection.owner).
- The title screen's SAVE FILE switch restarts the program today (SaveSlots_Switch: execv / CreateProcess). Wanted:
  switch in place: write slot.txt, set gPlatformSavePath, reload every sidecar and setting (cards, ygo, tt, life,
  region, sim, search...; display / controls / co-op settings) and go back to the title screen. Every module that
  caches a sidecar must drop its cache (Tcg_GetCollection keys by owner; check the others).

## 6. Card games: NEW tags and centered booster packs (the user, 2026-11-01)
- In the BINDER (all three card games: src/tcg/tcg_ui.c, src/ygo/ygo_ui.c, src/tt/), a "NEW" tag on every card
  obtained recently (until it has been looked at in the BINDER; kept in each game's sidecar).
- On every booster pack screen (GAME CENTER CARD SHOP, the CARD CLUB counters, Yu-Gi-Oh! / Triple Triad sellers), the
  pack under the cursor sits in the middle of the screen while choosing (the row scrolls to keep it centered).

## 7. 40 PC boxes (the user, 2026-11-01, "the final task")
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
- Pending: tasks 3-7 above.
