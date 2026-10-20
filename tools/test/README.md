# Headless tests

Runs the game with no window (SDL dummy driver), driven by the script harness in
`src/platform/sdl2.c`. Linux only; needs `make linux` to work. `convert` (ImageMagick) turns
screenshots into PNGs if it's installed.

    tools/test/regress.sh              # build, run every scenario, report ok / FAIL
    tools/test/regress.sh coop_wild    # just one
    NOBUILD=1 OUT=/tmp/t tools/test/regress.sh

Every two-client scenario also runs at the worst connection we plan for with `WORST=1`
(250 ms each way + 80 ms jitter through lagproxy, like NA <-> JP with the relay far from both).
`JOBS=3` runs three at once.

| scenario | what it checks |
|---|---|
| `solo_wild` | one client: wild battle to the action menu, win, back to the field |
| `coop_wild` | Route 101: BOB's encounter pulls ALICE in, they win together; record mixing on join |
| `coop_wild_lag` | the same, always at worst-case lag |
| `coop_catch` | co-op 2-on-2, a MASTER BALL (L = last ball) catches one, they finish the other |
| `coop_levelup` | co-op win one EXP short of Lv16 on both: level up + Torchic -> Combusken on both |
| `coop_heal` | ALICE uses a POTION on BOB's Moemon from her bag (partner's gold slot) |
| `coop_trainer` | Roxanne: ALICE waits for a partner, BOB walks up and joins, BOB's RUN is refused, they win |
| `coop_run` | co-op wild: ALICE alone picks RUN (it only says so), then both pick RUN until they get away |
| `coop_ball_target` | co-op 2-on-2: both throw at the same wild Pokemon (BOB moves his target cursor); ALICE catches it, BOB's ball comes back |
| `coop_abort_catch` / `coop_abort_catch2` | a catch, then the other player disconnects: the catcher keeps it (master / non-master) |
| `pvp_battle` | BATTLE FLAG: BOB spots ALICE, link battle to the end, money changes hands |
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
| `version_check` | (`version_check.sh`, part of the full run) a server one co-op version ahead: the game says so |

A scenario is a folder in `scenarios/` with `a.txt` (+ `b.txt` for a second client) and `conf`:
`MODE=1|2`, optional `DELAY`/`JIT` (ms, through lagproxy), `FPS`, `TMO` (seconds), `WARP`/`WARP_B`,
`ENV_A`/`ENV_B` (extra environment, e.g. `MOEMON_DEBUG_PARTY=280:12`),
and `EXPECT_A=(...)` / `EXPECT_B=(...)`: text that must show up in that client's log.

Scripts wait on game state (`waitfor`/`pressuntil` with a condition such as `field`, `action`, `battle`,
`partymenu`, `down`, `helpask`) or on a log line: `waitlog N TEXT` waits up to N frames for an `[mp]`
line containing TEXT, `pressuntillog KEYS N TEXT` taps KEYS meanwhile, and `watchlog TEXT` starts
watching before the key press that causes it (then a bare `waitlog N`). Fixed `wait`s break at high ping.
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
