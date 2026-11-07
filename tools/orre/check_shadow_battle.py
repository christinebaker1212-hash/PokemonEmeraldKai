#!/usr/bin/env python3
"""Checks the scenario shadow_battle: SHADOW RUSH used (its name in the battle log), the Pokémon's own moves and PP the
same before and after (never written over), EXP not stored at a closed gauge and stored once it's open, and the two
battles' battling drops.

    python3 -I tools/orre/check_shadow_battle.py RESULT_DIR
"""
import re
import sys

log = open(sys.argv[1] + "/a.log").read()
states = re.findall(r"\[test\] shadow 0: id (\d+) dp (-?\d+) gauge \d+ level (\d+) .*moves (\S+) (\S+) (\S+) (\S+) exp (\d+) stored (\d+)", log)
exps = [(int(a), int(b)) for a, b in re.findall(r"shadow: Shadow 1 EXP (\d+) \(stored (\d+)\)", log)]
sent = re.findall(r"shadow: Shadow 1 sent out, gauge (\d+)", log)
bad = []
if len(states) < 3:
    bad.append("expected 3 state lines, got %d" % len(states))
else:
    first, mid, last = states[0], states[-2], states[-1]   # (made; after the scent; at the end)
    if first[3:7] != last[3:7]:
        bad.append("its own moves / PP changed: %s -> %s" % (first[3:7], last[3:7]))
    if first[7] != last[7]:
        bad.append("its EXP changed (a Shadow Pokemon takes none): %s -> %s" % (first[7], last[7]))
    if int(mid[2]) < 4:
        bad.append("the scent didn't open the gauge to level 4 (level %s)" % mid[2])
if len(sent) != 2:
    bad.append("expected 2 battling drops, got %d" % len(sent))
if len(exps) != 2 or exps[0][1] != 0 or exps[1][1] <= 0:
    bad.append("EXP: want none stored at a closed gauge, some once open: %s" % exps)
print("\n".join(bad) or "ok: moves kept, %d drops, EXP stored %s" % (len(sent), exps))
sys.exit(1 if bad else 0)
