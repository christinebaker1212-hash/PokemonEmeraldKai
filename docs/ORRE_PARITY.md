# ORRE's parity with Pokémon Colosseum

The user (2026-11-11): "Be certain that scenes occur with parity to Colosseum's original scenes: every Shadow
Pokémon, every cutscene, every line of dialogue. And be certain that the MOéMON / POKéMON text swaps are respected."
And, the same day, of the walking tests: "It makes me suspicious that a walking playthrough ever truly succeeded."

This file is the ledger: what has been checked against the disc, how, and what is still open. Checks that only a
walk can make are not ticked here until a walk with plain key presses (no planner) has made them.

## How each thing is checked

| What | How | State |
|---|---|---|
| Every battle's teams | `mp orreaudit` (src/orre/orre_battle.c Orre_BattleAudit): every Colosseum battle's opponents and each POKéMON as the game builds it | 566 battles, 1738 POKéMON, none left out |
| Every Shadow Pokémon | the same audit: each Shadow slot built as its Shadow row (species, an opponent's Shadow record) | 111 slots made, all 48 Shadow rows met |
| Shadow Pokémon in a real battle | scenario `orre_first_shadow` (the Mayor's house scene, FOLLY, TRUDLY's MAKUHITA: the aura, RUI's call, the snag) | passes (1.189.6) |
| Every line the scripts say | every message id a room's script names (std054_characterTalk, winMsgOpenField...) looked up in orre/orre_text.pak | 1843 ids named outright; 7 were missing (sung lines, below), 0 now |
| Lines dropped by the converter | tools/orre/build_text.py: every line of the disc not in the pack, by its letters | 521 dropped, all Japanese; 12 English ones restored |
| The word for the creatures | ORRE's text carries Kai's word (MOéMON); OPTIONS > CREATURES swaps it as it's printed (src/mp/mp_textopts.c, every text printer) | pack: 0 lines with POKéMON spelled out; ORRE's C strings: 2 fixed (below) |
| Script commands | every native a room's script calls against src/orre/orre_vm.c's switch and its quiet list | below |
| Every room reached, every door | scenarios orre_all_maps, orre_all_doors, orre_all_places | pass; **but they use the tests' planner: see "What the walking tests did not prove"** |
| The story in order | orre_story_walk / orre_story_chain (hours; by flags) | to PYRITE so far; not a parity proof yet |

## Found and fixed by this audit (2026-11-11)

- **Scenes that begin as a room is entered never played** (1.189.6): a room was entered a frame before its map
  loaded and restarted by the load; the scene had set its first flag by then. The Mayor's house scene (FOLLY, TRUDLY,
  the first Shadow Pokémon) was the one found; any room opening with a scene was at risk.
- **No story trainer's Shadow Pokémon was Shadow in battle** (1.189.6): its record was dropped as the battle began.
- **RUI never called out a Shadow Pokémon** (1.189.6): not built. Now Colosseum's lines (0x7729 / 0x772A in battle
  17, else 0x7717 until flag 0x9A0, then 0x770A). Open: she says it each battle, Colosseum the first time each
  Shadow Pokémon is ever seen; "Why don't you have any POKé BALLS" (0x772B) isn't in.
- **Twelve English lines were dropped as Japanese** (1.189.7): they hold the wave dash of a sung line ("Oh, thirty
  long years of cranking gears ～♪"): PYRITE's windmill man (20496, 20535, 23271, 23272), 20991, TERL's 21148, the
  KIDS GRID song 23383 and five no script names outright. Kai's font has no note: the tail is "…".
- **RUI's "That's not a SHADOW POKéMON!" lines were written out in C** with POKéMON spelled, whatever the option
  (1.189.7): they're Colosseum's own lines from the pack now (0x7702 / 0x7716). The boat's "There's a SHADOW ...
  with you" (Kai's own line: the boat is Kai's) uses Kai's word.
- **Doors** (1.189.4 / 1.189.5): Colosseum's 154 doors were walls for good, and their models looped their opening.
- **Two floors in one tile** (1.189.7): see below.
- **RUI's naming was skipped** (1.189.8): PHENAC's scene calls menuNameEntryOpen(1, 0) once she's out of the sack;
  it was taken for the hero's name and passed over. It opens Kai's naming screen under Colosseum's "Her name is..."
  (15688) now; the name is kept with the story (56 of Kai's flags from 0xF40: orre_text.c Orre_PartnerName) and
  `{rui}` in the text pack is a slot, not the letters RUI (tools/orre/build_text.py).
- **RUI was invisible from the sack until she joined** (1.189.8): the scene places and shows character 101, who was
  no one until std035_setPartnerFollowing. She is a scene person before that (orre_npcs.c Of), the follower after.
- **A scene's second part let the player walk** (1.189.8): when the thread holding the player let go or ended while
  another still held them (a room's loop back to stepping the player while the scene it started plays on), the
  field was given back. The hold passes on now (orre_vm.c PassHold).
- **Sprites under a scene's camera** (1.189.8): feet were drawn 8 lines under the spot stood on, whatever the camera
  (the GBA's rule): into the ground under a low camera. A card stands 6 units toward the field's camera from its
  tile's middle with its feet there (Platinum's rule, o3d.c FEET_Z), from any camera.

## What the walking tests did not prove

The user could not reach PHENAC's stadium: the walk round the fountain was cut on both sides of the stairs. The
tests "passed" there because the tests' walker plans a route on the tile grid and takes any that exists: it took a
one-tile sidestep at the north-west and, when that was gone, a way round through a glass door. A player sees a wide
paved ring and an invisible wall. **A route existing is not the same as the place being walkable as it looks.**

The cause was in the floor builder (tools/orre/grid.py _reach): a tile's floor height was its lowest one for the
whole tile, so where a raised walk and the basin 13 units below share tiles the walk was a drop. Fixed by taking the
floor's own heights under each fine cell wherever levels meet.

What follows from it, for every later check:
- a place is only "walkable" when it is crossed by plain key presses in the direction a player would hold
  (scenario `orre_phenac_ring`; more to be added per town as each is walked);
- tools/orre/grid_check.py compares the tiles with the fine flood, so it shares the flood's mistakes: it could not
  have caught this. A check of the flood itself against Colosseum's floor is still wanted.

## Script commands the game doesn't run yet

Used by room scripts and not converted (the rest are run or deliberately quiet: sounds' details, model
animations, lights, the P★DA):

| Native | Where | What it is | Plan |
|---|---|---|---|
| 84, 85 sequenceJump | M3_houseD_1F (AGATE) | the day care | Kai's day care behind it |
| 86 sequenceJump | D2_pc_1F (MT. BATTLE) | a move tutor | Kai's move tutor screen |
| 87 | M3_out (AGATE) | unknown sequence | read the decomp |
| 241 preReliveDisplayHokoraParticle | M3_shrine_1F | the RELIC STONE's particles before a purification | an effect |
| 34 heroMoveAddAutoEvent | D4_out_2 | an automatic walk | to do |
| 203 floorEventChangeTresure (24 uses) | D1_labo_B1, D1_labo_B3 | a treasure box's state | to do with the labs |
| 204-208 DNA samples | the labs, PYRITE, REALGAM | the e-Reader's DNA samples (Japan's game) | closed: not in the English game |
| 200, 201, 202, 207, 212, 213, 215, 220 | M1_carde_1, D1_labo_B1 | the e-Reader room, the GBA link | closed: the hardware's |
| 128, 198, 235 | M1_pc_B1 | trading / battling over the GBA link, its save menu | closed: Kai's own co-op |
| 16, 20, 21, 23, 24, 26, 27, 240 | menu scripts | Colosseum's own menus | Kai's menus |

## Still to do for parity

1. Walk every town with plain key presses, each way a player would go; add a scenario per place found cut.
2. The story walk to the end (the SNAGEM HIDEOUT, REALGAM TOWER), reading every scene's lines against the pack.
3. Every Shadow Pokémon's battle reached in the story (the audit proves each can be built; the walk must meet them).
4. RUI's call once per Shadow Pokémon ever, as Colosseum; 0x772B.
5. The natives in the table above.
6. The opening's people (its scene rooms show no one).
7. The TVs' pictures move in Colosseum (the programme's model animates); here each is a still (1.189.9:
   tools/orre/tv.py draws them as the pack is built; o3d.c O3D_TvSet, orre_vm.c natives 171 / 211).
