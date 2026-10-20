# Headless tests

Runs the game with no window (SDL dummy driver), driven by the script harness in
`src/platform/sdl2.c`. Linux only; needs `make linux` to work. `convert` (ImageMagick) turns
screenshots into PNGs if it's installed.

    tools/test/regress.sh              # build, run every scenario, report ok / FAIL
    tools/test/regress.sh coop_wild    # just one
    NOBUILD=1 OUT=/tmp/t tools/test/regress.sh

| scenario        | what it checks |
|-----------------|----------------|
| `solo_wild`     | one client: wild battle reaches the action menu, then wins and returns to the field |
| `coop_wild`     | two clients on Route 101 through a local relay: BOB's encounter pulls ALICE in, they win together |
| `version_check` | (`version_check.sh`, part of the full run) a relay server one co-op version ahead: the game reports the mismatch |
| `coop_wild_lag` | the same through `tools/mp_server/lagproxy.py` at 250 ms each way + 80 ms jitter (NA <-> JP, the worst case we plan for) |

A scenario is a folder in `scenarios/` with `a.txt` (+ `b.txt` for a second client) and `conf`:
`MODE=1|2`, optional `DELAY`/`JIT` (ms, through lagproxy), `FPS`, `TMO` (seconds), `WARP`/`WARP_B`,
and `EXPECT_A=(...)` / `EXPECT_B=(...)`: text that must show up in that client's log. A log with
`TIMEOUT` (a `waitfor`/`pressuntil` that never got there) or a crash fails the scenario too.

Lower level, for trying things out (results and logs land in DIR):

    tools/test/run1.sh DIR [VAR=value...]       # DIR/a.txt, one client
    tools/test/run2.sh DIR [DELAY_MS [JIT_MS]]  # DIR/a.txt host + DIR/b.txt joiner, own relay server

`run2.sh` builds the relay server into `tools/test/.bin/` and picks free ports, so several runs can go at once.
Harness commands and conditions: see CLAUDE.md ("Headless testing") and `MP_TestCondition` in `src/mp/`.
Anything network related should pass `coop_wild_lag`-style settings: at 250 ms an ask-and-answer
through the relay takes over a second.
