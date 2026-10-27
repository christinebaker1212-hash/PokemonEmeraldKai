# Handoff (as of 1.26.3)

Read `CLAUDE.md` and `BUILDING_COOP.md` first. Releasing works like this: bump `MOEMON_VERSION` in
`include/mp_version.h`, add `tools/release/notes/v<ver>.md`, then push to `claude/clever-clarke-hqati2`. CI then builds,
tags and publishes the release, and the auto-updater delivers it. A push without a version bump publishes nothing.

## Released recently (all live through the auto-updater)
| Version | What |
|---|---|
| 1.25.0 | Battle weather on every screen, battle emotes shared, JOIN BATTLE / BATTLE ALONE when a trainer battling a friend spots you, 30% quicker screen fades |
| 1.25.1 | Castform crash in the PC (its 4-form palette overflowed a 32-byte buffer in mp_hue.c) |
| 1.25.2 | Never published (its CI run hung, so it was cancelled). Its HYPER BEAM fix shipped in 1.25.3 |
| 1.25.3 | **HYPER BEAM crash**: `gSolarBeamBigOrbAnimTable` had 7 entries but `AnimHyperBeamOrb` picks `% 8`. Added the 8th (the small orb, as the GBA read). **Foe CASTFORM crash** when its form changes in weather: `front_pic_anims.h` gave it `sAnims_MoemonStatic`, now `sAnims_Castform` (4 frames) |
| 1.26.0 | HGSS POKé BALLS (see COOP_NOTES "HGSS POKé BALLS"; art `docs/user_images/35.png` by Sunday-Doneko, credited in `dist_extras/README.txt`) |
| 1.26.1 | Ball opening: frame 2 = the sheet's squashed ball; anim 1 = squash, then open (held) |
| 1.26.2 | `LoadBallGfx` no longer copies the stock "fully open" ball (`gOpenPokeballGfx`) over the sheet (it landed on the HGSS open frame) |
| 1.26.3 | The player side's sent-out ball turns invisible when it opens (`SpriteCB_ReleaseMonFromBall`); the throw is unchanged |

The user confirmed that 1.26.2 fixed the open-ball overlay. 1.26.1 and 1.26.3 were pushed without tests or a look
in-game; their CI runs weren't checked either.

## Open work

### 0. Dev auto-updater (1.26.4)
- Main bootstrap version: **1.26.4**, "A Safe Test Flight!".
- `AllowSmallTweaks = 1` selects `dev-latest`; `0` returns to latest main even
  when that means downgrading. Updates retain SHA-256 / ZIP CRC validation,
  rollback and the player's settings.
- Dev builds disable manual save, autosave and player-progress disk writes,
  including sidecars; existing saves are read-only. DEV is shown in the title.
- Main releases refresh dev from the same commit. Explicit experiments are
  dispatches of the separate Dev Release workflow; keep experimental code gated with
  `MOEMON_DEV_BUILD` until promoted. Ordinary work remains main by default.
- Build/release instructions: BUILDING_COOP.md, "Dev update channel".
- Tests: tools/test/dev_save_test.sh and tools/test/dev_update_test.sh; the
  existing tools/test/update_test.sh continues to exercise update integrity.

### Card system (shipped in main 1.27.0, reworked in 1.28.0)
- Ungated and released as a main update. Status, design decisions, code map, rules fixes,
  effect coverage, online protocol and every test: `docs/TCG_IMPLEMENTATION.md`; the
  user's answers for the 1.28.0 rework: `docs/TCG_DECISIONS.md`.
- The duel screen follows the user's DS screenshot (docs/user_images/37.webp) at native
  240x160: board cursor + SELECT quick menu, hand strip that scrolls the mat, card
  effects and sounds, controller button icons everywhere. Do not describe the whole
  228-card system as finished (20 attacks and all Pokemon Powers are unported).
- Online duels between players are a hard requirement: keep `tools/tcg/test_online.sh`
  and `tools/tcg/test_online_board.sh` (both also with `250` lag) passing, and bump
  `TCG_LINK_VERSION` (now 4) on any rules/catalogue/wire change.
- Tests: `tools/tcg/test.sh`, `NOBUILD=1 tools/tcg/test_ui.sh`, the two online tests,
  `NOBUILD=1 tools/tcg/test_trade.sh`,
  `NOBUILD=1 tools/tcg/test_save.sh` (main saves), `NOBUILD=1 tools/test/dev_save_test.sh`.
  Send actual screenshots to the user.

### 1. Joining a battle against two trainers (user's report; unfinished, backed out of the branch)
Report: "if two players enter a battle that was started by the initial trainer being seen by two trainers, a third
player who attempts to join after the battle starts will never prompt the battling two players." In the code,
`JoinableReason` refused two-trainer co-op battles (`JOIN_CANT_KIND`).
- The work is in commits **589afed3** (the feature, protocol 51, debug `mp joinask N`, `mp animall` /
  `MOEMON_ANIMCHECK`, docs) and **9f079c7d** (test `coop_two_solo_join`). Both were reverted for the 1.26.0 release.
  To restore: `git revert <the revert commit>` (see `git log`, "Revert ... Join a battle against two trainers").
- **Works:** a 3rd player joins a co-op battle against two trainers, and it becomes a Triple Battle against both. The
  enemy party stays put (slots 0-2 trainer A, 3-5 trainer B); the new foe spot comes from the trainer with more
  Pokémon left; the battle keeps `BATTLE_TYPE_TWO_OPPONENTS`; `trainerIdB` and the second lose text reach the joiner.
  Test `coop_two_join3` passes at WORST=1.
- **Broken:** a friend joining a *solo* battle against two trainers (test `coop_two_solo_join`). The host logs
  "answer 2 (3) to player 2" (YES), but the joiner never logs "said yes", and its request seems to end silently.
  Suspect `Task_JoinFriend` case 0: the "battle is already over" exit (`MPF_IN_BATTLE` not yet seen), or `OnAnswer`
  conditions (`fstate`, `from == target`). Add a log line in `FriendShowEnd` and rerun.
- **Unexplained:** `join_double` hung mid-battle twice when run with another scenario (JOBS=2). It passed alone and
  in a copy with screenshots. It may be a timing flake, or it may be real; check before shipping.
- Draft release notes: `tools/release/notes` has none. Write new ones (player-facing; title format in CLAUDE.md).

### 2. Join in progress: garbage tiles on the joiner's screen before their trainer appears (task #210)
Still waiting for the user's screenshot.

### 3. Older crash log entries (builds of Oct 20 and Oct 23)
The `crash_log.txt` the user sent also lists `exe+3e0192` (writing 0, in battle, map 0.19) and `exe+24b09e`. These are
from older builds that aren't in the scratchpad; they may already be fixed. Ask the user whether they still happen.

## Useful
- Crash logs: resolve `exe+off` with `x86_64-w64-mingw32-nm -n` on that release's exe (ImageBase 0x140000000). The
  release zips are on GitHub releases.
- Tests: `NOBUILD=1 WORST=1 tools/test/regress.sh NAME...`. Run1 scenarios write screenshots into their scenario
  folder, so never commit those (`tools/test/scenarios/*/` keeps only `.txt` / `conf` / `check*`).
- User rules: never test with moves that can miss; simulated online lag, not LAN; hotfixes go straight out through the
  updater; release notes are for players (no "the design you sent"); give each build a catchy title; save user images
  in `docs/user_images` with an index row.
