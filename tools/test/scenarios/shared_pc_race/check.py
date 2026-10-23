#!/usr/bin/env python3
# Exactly one of the two lost the race, and BULBASAUR is in exactly one place (slot 5 or 6, both games agree)
import re, sys
d = sys.argv[1]
a = open(d + "/a.log", errors="replace").read()
b = open(d + "/b.log", errors="replace").read()
lost = a.count("lost a Pokemon to somebody quicker") + b.count("lost a Pokemon to somebody quicker")
if lost != 1:
    sys.exit("expected exactly one lost race, got %d" % lost)
def last_box(t):
    m = re.findall(r"\[test\] sharedbox 1:([^|]*)\|", t)
    return m[-1].split() if m else None
la, lb = last_box(a), last_box(b)
print("ALICE sees", la, "BOB sees", lb)
if la != lb or len(la) != 1 or not re.match(r"[56]:1\(ALICE\)", la[0]):
    sys.exit("BULBASAUR should be in one place, slot 5 or 6")
for t, who in ((a, "a"), (b, "b")):
    blocks = t.split("[test] monids done")
    if len(blocks) < 2:
        sys.exit(who + ": no monids")
    if re.findall(r"\[test\] monid (own [0-9a-f:]+ \d+ box|hand)", blocks[-2]):
        sys.exit(who + ": has a stray Moemon")
