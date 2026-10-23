#!/usr/bin/env python3
# BOB and CARL ask to join at about the same time, so either may get seat 1: they must get seats 1 and 2 of 3 between
# them. Then join_wild's check (everyone keeps their party).
import os, re, subprocess, sys
d = sys.argv[-1]
seats = []
for c in "bc":
    m = re.search(r"join: got the battle \(seat (\d) of 3\)", open(f"{d}/{c}.log", errors="replace").read())
    seats.append(m.group(1) if m else None)
print(f"seats: BOB {seats[0]}, CARL {seats[1]}")
ok = sorted(s or "" for s in seats) == ["1", "2"]
here = os.path.dirname(os.path.abspath(__file__))
r = subprocess.run([sys.executable, os.path.join(here, "..", "join_wild", "check.py"), d])
sys.exit(0 if ok and r.returncode == 0 else 1)
