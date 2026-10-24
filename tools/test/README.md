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
| `trade` | CO-OP > TRADE: both offer their lead, READY, confirm, swapped |
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
| `tri_heal` | wild triple: ALICE uses a POTION on CARL's Moemon (the gold slots are the allies') |
| `tri_abort` | wild triple: ALICE catches one, BOB disconnects: called off, ALICE keeps the catch |
| `look_chars` | NA-JP lag: ALICE wears new characters: PROF. BIRCH's 64x64 bikes, SERENA's ridden RHYHORN then a bicycle (LOOK > BIKE), her diving sprite, an old ini name (`steven`); BOB sees each |
| `look_created_remote` | NA-JP lag: BOB receives ALICE's whole CREATE recipe (FIELD HAIR 5) and later her LOOK > BIKE (`mp remoteapp`); shots of her sprite on both screens |
| `shared_pc_cursor` | the shared PC's hand on PARTY / MY PC / CLOSE: over the buttons, never upside down (also holding a Moemon, and after MY PC and back) |
| `catch_exp` | solo: a MASTER BALL at a full-HP ZIGZAGOON gives 10% of a faint's EXP; the lead TORCHIC grows and evolves after the catch |
| `coop_catch_exp` | NA-JP lag, co-op wild: ALICE catches a ZIGZAGOON at 1 HP (95%); both leads get EXP, grow and evolve after the battle |
| `flag_phrases` | NA-JP lag: BOB uses the BATTLE FLAG (PHRASES: the phrase builder and back to the list, then PLANT), both screens show his CHALLENGE, the result quotes his phrase; check.py: the winner got a tenth, the loser kept their money |
| `spec_sides` | 4 players, NA-JP lag: ALICE (1920x1080) shows BOB LEFT 1X, CARL RIGHT 2X, DANA ABOVE 1X, then DANA in her own window, then closes it (DANA only goes off); check_spectrace.py |
| `version_check` | (`version_check.sh`, part of the full run) a server one co-op version ahead: the game says so |

A scenario is a folder in `scenarios/` with `a.txt` (+ `b.txt` for a second client) and `conf`:
`MODE=1|2`, optional `DELAY`/`JIT` (ms, through lagproxy), `FPS`, `TMO` (seconds), `WARP`/`WARP_B`,
`ENV_A`/`ENV_B` (extra environment, e.g. `MOEMON_DEBUG_PARTY=280:12`),
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
