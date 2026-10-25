# Headless tests

Runs the game with no window (SDL dummy driver), driven by the script harness in
`src/platform/sdl2.c`. Linux only; needs `make linux` to work. `convert` (ImageMagick) turns
screenshots into PNGs if it's installed.

    tools/test/regress.sh              # build, run every scenario, report ok / FAIL
    tools/test/regress.sh coop_wild    # just one
    NOBUILD=1 OUT=/tmp/t tools/test/regress.sh

Three-client scenarios (`MODE=3`) use `runN.sh` (2-4 clients; `c.txt`, `WARP_C`, `ENV_C`, `EXPECT_C`).
Every multi-client scenario also runs at the worst connection we plan for with `WORST=1`
(250 ms each way + 80 ms jitter through lagproxy, like NA <-> JP with the relay far from both).
`JOBS=3` runs three at once.

| scenario | what it checks |
|---|---|
| `solo_wild` | one client: wild battle to the action menu, win, back to the field |
| `coop_wild` | Route 101: BOB's encounter pulls ALICE in, they win together; record mixing on join |
| `coop_wild_lag` | the same, always at worst-case lag |
| `coop_catch` | co-op 2-on-2, a MASTER BALL (L = last ball) catches one, they finish the other |
| `coop_levelup` | co-op win one EXP short of Lv16 on both: level up + Torchic -> Combusken on both |
| `coop_icons_flash` | co-op 2-on-2 at NA-JP lag with TYPE ICONS on: THUNDER's screen flashes leave the icons and the window frame alone (screenshots) |
| `coop_weather` | co-op 2-on-2 at NA-JP lag: SANDSTORM hits all four at once (one line, then quiet turns), win on both |
| `coop_flash` | Granite Cave B2F (dark) at NA-JP lag: each sees the other's light, BOB's FLASH lights up around him on ALICE's screen; co-op notes show in the dark |
| `coop_world` | NA-JP lag: a CUT tree is gone for a friend who walks onto the map later (list), a smashed rock vanishes live, a STRENGTH boulder moves on both games |
| `coop_world2` | NA-JP lag: CUT grass is cut on the friend's game; Route 113's ash ALICE walks through clears for BOB, the soot is only hers |
| `coop_heal` | ALICE uses a POTION on BOB's Moemon from her bag (partner's gold slot) |
| `coop_trainer` | Roxanne: ALICE waits for a partner, BOB walks up and joins, BOB's RUN is refused, they win |
| `coop_run` | co-op wild: ALICE alone picks RUN (it only says so), then both pick RUN until they get away |
| `coop_ball_target` | co-op 2-on-2: both throw at the same wild Pokemon (BOB moves his target cursor); ALICE catches it, BOB's ball comes back |
| `coop_abort_catch` / `coop_abort_catch2` | a catch, then the other player disconnects: the catcher keeps it (master / non-master) |
| `pvp_battle` | BATTLE FLAG: BOB (no CHALLENGE phrase: no phrase box) spots ALICE, link battle to the end, the winner gets a tenth of the loser's money |
| `pvp_switch` | PvP where ALICE's lead faints and she sends in her second Moemon |
| `trade` | CO-OP > TRADE: both offer their lead, READY, confirm, swapped (and their MOéMON GARDEN records travel) |
| `blender` / `crush` | BERRY GAMES invite, accept, play to the results |
| `contest` | link CONTEST at the Lilycove counter, start to finish (long) |
| `card_base` | FRIENDS > TRAINER CARD and VISIT SECRET BASE |
| `spectate` | A on a friend in a battle: watch it, B to stop |
| `rescue` | BOB loses, ALICE gets HELP? and revives his lead |
| `lend` | BOB cuts a tree with ALICE's Moemon's CUT |
| `piggyback` | BOB rides on ALICE's back into Oldale |
| `photo` | SOCIAL > TEAM PHOTO saved |
| `gift` | DAILY > GIFT sent, received, thanked |
| `tri_wild` | three clients: CARL's encounter pulls ALICE and BOB in, a wild Triple Battle to the win |
| `tri_target_level` | Roxanne co-op Triple at NA-JP lag: every player opens the target window; its Lv must match the cards (no "target level mismatch" in any log) |
| `tri_trainer` | Roxanne: ALICE waits, BOB joins, ALICE keeps waiting, CARL joins: a trainer Triple Battle |
| `tri_catch` | wild triple: ALICE and BOB each catch a different wild Pokemon (each gets their own) |
| `tri_heal` | wild triple: ALICE uses a POTION on CARL's Moemon (the third panel on the left: the second ally's Moemon out) |
| `tri_party` | wild triple: ALICE's party screen against the user's mock-up pixel for pixel (actives left, reserves right), cursor RIGHT / DOWN / CANCEL |
| `tri_abort` | wild triple: ALICE catches one, BOB disconnects: called off, ALICE keeps the catch |
| `look_chars` | NA-JP lag: ALICE wears new characters: PROF. BIRCH's 64x64 bikes, SERENA's ridden RHYHORN then a bicycle (LOOK > BIKE), her diving sprite, an old ini name (`steven`); BOB sees each |
| `look_created_remote` | NA-JP lag: BOB receives ALICE's whole CREATE recipe (FIELD HAIR 5) and later her LOOK > BIKE (`mp remoteapp`); shots of her sprite on both screens |
| `shared_pc_cursor` | the shared PC's hand on PARTY / MY PC / CLOSE: over the buttons, never upside down (also holding a Moemon, and after MY PC and back) |
| `catch_exp` | solo: a MASTER BALL at a full-HP ZIGZAGOON gives 10% of a faint's EXP; the lead TORCHIC grows and evolves after the catch |
| `coop_catch_exp` | NA-JP lag, co-op wild: ALICE catches a ZIGZAGOON at 1 HP (95%); both leads get EXP, grow and evolve after the battle |
| `flag_phrases` | NA-JP lag: BOB uses the BATTLE FLAG (PHRASES: the phrase builder and back to the list, then PLANT), both screens show his CHALLENGE, the result quotes his phrase; check.py: the winner got a tenth, the loser kept their money |
| `spec_sides` | 4 players, NA-JP lag: ALICE (1920x1080) shows BOB LEFT 1X, CARL RIGHT 2X, DANA ABOVE 1X, then DANA in her own window, then closes it (DANA only goes off); check_spectrace.py |
| `options_tabs` | START > OPTIONS with real keys: TEXT SPEED, L/R tabs, BUTTON ICONS PLAYSTATION (icons switch), AUTOSAVE 10, MOENAV CALLS BATTLES (random chat calls stop); check.sh: saved to the inis |
| `music_options` | BIKE MUSIC / SURF MUSIC: the Mach Bike, surfing Petalburg's pond and diving, songs checked in order (check.py) |
| `dex_text` | POKéDEX TEXT tab: the long entry's start with an arrow, A / DOWN / UP / B scrolling, DOWN to the next Pokémon after, seen-only does nothing, AREA still opens; `mp dexcheck` builds all 386 texts |
| `life_core` | MOéMON GARDEN records: temperament, memories, a moment raises Friendship once, the .life sidecar saved and loaded, the JOURNAL page and its pages |
| `life_field` | Route 104: tap A (the fitting verb), hold A (the wheel, FEED A BERRY: a memory and a moment), the hidden-item hint, a nap, hurrying back |
| `life_story` | a badge becomes a memory (part / witness) and news; a curious MUDKIP asks to come out: party icon wiggle, WANTS TO COME in the follower picker |
| `life_battle_gym` | the first Gym (ROXANNE): the rare pink line after "... defeated LEADER ROXANNE!" with a chime, a boss's final knockout as a memory |
| `life_battle_save` | wild double: TORCHIC knocks out a foe while its best friend MUDKIP hangs on at 3 HP: sparkle / heart bubbles, the line after "fainted!", MEM_PARTNER for both |
| `life_word` | word spreads: a proud TORCHIC tells MUDKIP (who wasn't there) about the first badge: MEM_HEARD from TORCHIC |
| `life_social` | NA-JP lag, Oldale: ALICE's follower meets BOB's (a bond in her save), its bubbles show on BOB's screen, BOB pets it (her game answers: a sidestep, PET is beyond its boundary), BOB's new LOOK gets a double take |
| `garden_map` | OLDALE's CENTER escalator rides up into the MOéMON GARDEN (first visit: welcome, a memory; every growth mark with all badges) and its escalator back down to OLDALE |
| `garden_centers` | all 17 CENTERs (LEAGUE and BATTLE FRONTIER too): up into the Garden and back down to the same CENTER |
| `tm_reuse` | TMs reusable: a save with TM39's flag but no TM39 (and TM08 still in the BAG) gets only TM39 back from the LOTTERY CORNER clerk, once; TM39 taught from the BAG stays there; a POKé MART sells TM01 once, one at a time, and refuses TM01 and TM39 after |
| `surf_remote` | a friend surfing Petalburg's pond: their blob under them, again after a map reload (OPTIONS and back), and hidden with them once BOB is 28 tiles away on ROUTE 102 (before 1.17.2 it wrapped round onto his screen) |
| `garden_decorate` | DECORATE with the real buttons, the SECRET BASE's way: DOLL > the PIKACHU DOLL moved with the decorating cursor, "Place it here?" YES, back out and walk (1.17.2 locked the player); the copy shows in use; PUT AWAY with the cursor; walk again; a jump down the plateau's ledge lands below it |
| `garden_places` | a place for every type: TORCHIC is sent to each of the 17 types' places with no badges, then with all eight, and walks all the way there round the trees, rocks and water (check_garden_places.py) |
| `garden_stuck` | an older save standing inside a tree goes back to the landing; with no CENTER recorded the way down follows the heal location |
| `garden_life` | three Pokémon live in the Garden: play, talking to one, a PIKACHU DOLL placed and played with, WATCH mode (A tells, B back) |
| `garden_travel` | START > GARDEN from OLDALE, back down to its CENTER; GARDEN in the START menu; a CENTER PC's row; refused in RUSTURF TUNNEL and before having a Pokémon |
| `garden_longterm` | TREECKO's pursuit reaches a milestone; MUDKIP coaxes a nervous TORCHIC into the pond (seen: in the journal), stargazing; TORCHIC evolves there (new sprite) and the others come to see; COMBUSKEN's JOURNAL page |
| `life_family` | MUDKIP and TREECKO make an EGG (the DAY CARE's code), it hatches on a step, meets its parents ("Met its parent"), TREECKO evolves and the follower reacts |
| `levelscale_wild` | a Lv46 team on Route 101: wild Pokémon capped at 18 with no badges, 40-43 and evolved with eight |
| `levelscale_trainer` | ROXANNE's ace Lv15 -> 20 against a team averaging 42 (boss margin, no-badge cap) |
| `levelscale_off` | LEVEL SCALING off: no scaling |
| `coop_levelscale` | NA-JP lag: ROXANNE scaled for the average of both teams (Lv30 and Lv50) |
| `summary_wrap` | the summary screen wraps (LEFT on the first page -> the one-page JOURNAL, RIGHT -> back); A on a one-page journal does nothing and opens no move selection |
| `battle_weather` | rain, hail, sandstorm and sunlight on screen in a wild battle (shots), a turn in the rain with no repeated line |
| `battle_weather_map` | a wild battle on rainy Route 120 starts in the rain, on screen |
| `follow_ground` | the follower in tall grass (Route 101), its reflection (Petalburg's pond), footprints (Route 109), puddle splashes (Route 120), hot springs (Lavaridge) |
| `follow_ground_remote` | NA-JP lag: BOB sees the grass shake around ALICE's follower |
| `follow_reflect_remote` | NA-JP lag at Petalburg's pond: each screen shows the friend's and the TORCHIC's reflections washed out like the player's (shots) |
| `life_return` | Route 104: the follower off on its own runs back after its Trainer and falls in behind them, never popping over (check_followtrace.py) |
| `life_arrival` | arrival bubbles only in a new area: nothing at the start or in and out of a house, a curious look in PETALBURG (check_arrival.py) |
| `life_watch` | WATCH QUIETLY: learned, nothing new, nothing new, then learned again half an hour later (check_watch.py) |
| `move_parity` | NA-JP lag, Oldale: both players' and followers' steps and facings the same on both screens, a follower on its own and running back included (check_movetrace.py) |
| `coop_switch_twice` | Co-op wild double at lag: ALICE switches, then opens the party menu again (the party in battle order crashed 1.15.0); both finish |
| `soft_reset` | A+B+START+SELECT on the overworld closes the game like CTRL+R (it crashed in the wireless adapter code) |
| `garden_parity` | NA-JP lag: both players' Garden Pokémon take the same steps and face the same ways on both screens (check_movetrace.py) |
| `garden_together` | NA-JP lag: two in the Garden: each sees the other's Pokémon and decorations, BOB talks to ALICE's MUDKIP; BOB's eight badges grow ALICE's Garden live |
| `version_check` | (`version_check.sh`, part of the full run) a server one co-op version ahead: the game says so |

A scenario is a folder in `scenarios/` with `a.txt` (+ `b.txt` for a second client) and `conf`:
`MODE=1|2`, optional `DELAY`/`JIT` (ms, through lagproxy), `FPS`, `TMO` (seconds), `WARP`/`WARP_B`,
`ENV_A`/`ENV_B` (extra environment, e.g. `MOEMON_DEBUG_PARTY=280:12`; LEVEL SCALING is off in tests unless a scenario
sets `MOEMON_LEVEL_SCALING=1`, common.sh),
and `EXPECT_A=(...)` / `EXPECT_B=(...)`: text that must show up in that client's log.

Scripts wait on game state (`waitfor`/`pressuntil` with a condition such as `field`, `action`, `battle`,
`partymenu`, `down`, `helpask`) or on a log line: `waitlog N TEXT` waits up to N frames for an `[mp]`
line containing TEXT, `pressuntillog KEYS N TEXT` taps KEYS meanwhile, and `watchlog TEXT` starts
watching before the key press that causes it (then a bare `waitlog N`). Fixed `wait`s break at high ping. `barrier NAME COUNT` holds a client until COUNT clients (they share the result folder) reach it: the
next lines run on the same frame (shared_pc_race grabs the same Moemon at once).
Species ids are the game's internal ones (Torchic 280, Mudkip 283; 252-276 are unused "?" slots). A log with
`TIMEOUT` (a `waitfor`/`pressuntil` that never got there) or a crash fails the scenario too.

The Windows build under Wine (`apt install wine64`), from the unzipped package:

    tools/test/wine_smoke.sh PKG_DIR     # solo boot + battle, then Windows server + Windows host + Linux joiner at 250 ms

Lower level, for trying things out (results and logs land in DIR):

    tools/test/run1.sh DIR [VAR=value...]       # DIR/a.txt, one client
    tools/test/run2.sh DIR [DELAY_MS [JIT_MS]]  # DIR/a.txt host + DIR/b.txt joiner, own relay server

`run2.sh` builds the relay server into `tools/test/.bin/` and picks free ports, so several runs can go at once.
Harness commands and conditions: see CLAUDE.md ("Headless testing") and `MP_TestCondition` in `src/mp/`.
Anything network related should pass `coop_wild_lag`-style settings: at 250 ms an ask-and-answer
through the relay takes over a second.
