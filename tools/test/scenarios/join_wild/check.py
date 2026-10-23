#!/usr/bin/env python3
# Every player has the same Moemon after the battle as before it ("[test] partyids:" lines, first and last): the same
# ones (species, personality, OT ID) in the same slots, none lost, swapped, doubled or a BAD EGG, and none lost
# a level or EXP. (A test that lets one evolve says so with EVOLVE=slot:species in the environment.)
import os, re, sys
d = sys.argv[1]
bad = 0
for c in "abcd":
    if not os.path.exists(f"{d}/{c}.log"):
        continue
    lines = [l for l in open(f"{d}/{c}.log", errors="replace") if l.startswith("[test] partyids:")]
    if len(lines) < 2:
        print(f"{c}: {len(lines)} partyids lines"); bad += 1; continue
    def mons(l):
        return [m.split(":") for m in l.split("partyids:", 1)[1].split()]
    before, after = mons(lines[0]), mons(lines[-1])
    ok = len(before) == len(after) and "BADEGG" not in lines[-1]
    for b, a in zip(before, after):
        if b[:3] != a[:3] or int(a[3][1:]) < int(b[3][1:]) or int(a[4][1:]) < int(b[4][1:]):
            ok = False
    print(f"{c}: {'kept' if ok else 'CHANGED'}: {' '.join(':'.join(m) for m in before)} -> {' '.join(':'.join(m) for m in after)}")
    if not ok:
        bad += 1
sys.exit(1 if bad else 0)
