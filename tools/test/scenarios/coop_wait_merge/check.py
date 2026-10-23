#!/usr/bin/env python3
# Exactly one of the two joined the other's battle, and the one who joined is the one who waited less.
import re, sys
d = sys.argv[-1]
a = open(f"{d}/a.log", errors="replace").read()
b = open(f"{d}/b.log", errors="replace").read()
joined = [n for n, log in (("ALICE", a), ("BOB", b)) if "co-op: in player" in log]
led = [n for n, log in (("ALICE", a), ("BOB", b)) if "joins the battle against trainer" in log]
m = re.search(r"waits on our trainer too(?:; we waited first| and waited first) \((\d+) vs (\d+) frames, round trip (\d+)\)", a)
print("joined:", joined, "led:", led, "decision:", m.group(0) if m else None)
ok = len(joined) == 1 and len(led) == 1 and joined != led and m is not None
if ok:
    first_is_alice = "; we waited first" in m.group(0)
    ok = (led == ["ALICE"]) == first_is_alice
sys.exit(0 if ok else 1)
