# Fight character audit (6.4)

The owner's direction: the shipped characters will all be replaced or removed, so they aren't fixed one by one.
What matters is the patterns the tools must recognise in later packs. This file records those patterns with the
evidence for each, and the two fixes made before that direction came.

## The tools

- `tools/fight/check_chars.py`: static checks from the files (`tools/fight/mugen_files.py` reads the pack). It
  reports states, animations, sounds and sprites that are named but missing, variable indexes past the engine's
  limits, and unbalanced brackets. It works out which states a fight can reach, and reports findings in unreachable
  states separately. Missing sprites are reported as information only.
- `tools/fight/audit_chars.sh`: every pairing plays CPU against CPU through `mf_selftest`'s `audit`. The audit
  reports:
  - a fighter out of control in one state for 15 s;
  - intros over 20 s;
  - a round paused over half its first minute;
  - a round longer than 3 minutes;
  - a position off the stage or not a number;
  - over 40 helpers;
  - frames over 1 s;
  - the engine's player warnings (`MOEMON_FIGHT_WARN=1`);
  - each hit's damage (`audithit` lines, read by the balance audit).

  Each pairing is then played twice more with made-up buttons, once rolling back 7 frames every 7, and the hashes
  must match. Test hooks: `MOEMON_FIGHT_PICK=P1,P2[,STAGE]` and `MOEMON_FIGHT_WATCH=1` (fight_platform host),
  `MOEMON_FIGHT_SYNC=1`.
- Result of the run (46 of the 110 pairing-and-seed runs, all ten characters):
  - no stuck state;
  - no rollback mismatch;
  - no runaway helpers;
  - two long rounds, both explained below.

## Patterns for the parser and ingestion

| Pattern | Seen in | Evidence | What a tool should do |
|---|---|---|---|
| `TargetState` / `ChangeState` to a state the character lacks (a typo of a sibling state) | Fei Long throw escape: 935 where his escape state is 930, the same template as Ryu's | The target ends up in a state that doesn't exist and runs no state code | Flag as an error; suggest the nearest existing state number used by the same template |
| Unbalanced brackets in a trigger (`AnimElem) = 7`) | Fei Long 901 | MUGEN refuses to load such a file; our parser accepts it | Flag as a syntax error; don't count `[0,5)` range syntax |
| Debug `DisplayToClipboard` / `AppendToClipboard` left on in states -2 / -3 | Fei Long (redirects to a missing helper every frame), Gouki, Cammy, Ken, Ryu, Vega | Thousands of warnings a match; debug text only | Safe to switch off (`type = null`) automatically |
| Redirects to helpers that don't exist yet, without a `NumHelper` guard | Gouki: helpers 6700, 6200, 6100 (~20,000 warnings in 46 runs) | The trigger is false; MUGEN behaves the same way | Count as noise; report the totals once per character |
| Japanese (Shift-JIS) names and strings | Every SSF2X character (`Name = "SSF2X..."` comparisons) | `str.splitlines()` splits on bytes 0x85 and 0x1C inside the strings | Split files on `\n` only and read them as bytes or latin-1 |
| Backup state files nobody loads | Dhalsim `hikae_tech.st` (holds states 1007 and 1046 that `tech.st` points at) | The loaded files reach no such state | Check reachability before reporting a missing state |
| Missing animations and sounds | Chun-Li anim 93 and sound 2,12; Dhalsim anim 48, projremanim 1004, sound 2,13; Deejay intro anim 190 (confirmed at run time: "Changed to invalid action 190") | No crash. The engine keeps the last animation, and the sound doesn't play | Warning, not an error. Don't copy anims across characters: the same sprite numbers hold different art (Chun-Li's 91,x is a light burst, Dhalsim's is dust) |
| Blank frames on purpose (sprites the .sff lacks: `12345,6789`, `6000,6000`, `0,203489280`) | Vega, Ryu, Ken, Gouki, Dhalsim | A common way to hide a frame | Information only |
| A helper that slows the whole game with a 1-frame `Pause` on most frames | Chun-Li helper 3000: slow motion after her projectile hits, which keeps starting again against some opponents | Ryu vs Chun-Li: 180 of 210 s paused, so the round timer barely moves | Runtime only: the audit's "paused over half" check |
| Damage as expressions over option variables (`ceil(132*var(52)/3)` with `var(52) = 3` set every frame) | Ryu, Ken, Cammy, Fei Long, Gouki | Static numbers are wrong unless the constant is substituted | The balance audit substitutes vars set by `VarSet` with `trigger1 = 1` in states -2 / -3 |
| Several HitDefs in one state picked by conditions (not a multi-hit sequence) | Gouki normals | Adding them doubles the damage | Count HitDefs at different `AnimElem` as a sequence, the others as alternatives |
| Damage through `TargetLifeAdd` instead of HitDefs | Throws and supers | HitDef damage alone shows 0 | Count negative `TargetLifeAdd` values |

## Fixes made (before the direction)

- `mugen/chars/ssf2x_feilong/feilong.cns`: the throw escape now uses `TargetState 930` (it was 935). `AnimElem) = 7`
  became `AnimElem = 7`.
- `mugen/chars/ssf2x_feilong/option.txt`: the debug `AppendToClipboard` is off.

## Engine notes

- A trigger group stops at its first false line, as MUGEN does (`Trigger_TestList`). Redirect warnings behind a
  guard on an earlier line don't happen.
- The rollback comparison has to start both runs on the same frame. Without `MOEMON_FIGHT_SYNC`, background loading
  moves the match's first frame by one, which looked like a desync. Online play waits for both sides, and the
  online tests agree.
