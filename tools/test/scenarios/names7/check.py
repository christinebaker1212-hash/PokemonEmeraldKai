#!/usr/bin/env python3
# Every client's players line holds all four names, each "NAME Lv<n>", and nothing else, and its length is right.
import re, sys
d = sys.argv[1]
names = {"SATOSHI", "TAKESHI", "MITSURU", "KENJIRO"}
bad = 0
for c in "abcd":
    lines = [l for l in open(f"{d}/{c}.log", errors="replace") if "namelines: players " in l]
    if len(lines) != 1:
        print(f"{c}: {len(lines)} players lines"); bad += 1; continue
    m = re.search(r"namelines: players (.*)\| \((\d+)\)", lines[0])
    text = m.group(1)
    # (the Lv glyph has no ASCII form: it shows as a space)
    parts = re.findall(r"([A-Z]{7}) .(\d+)", text)
    got = {p[0] for p in parts}
    if "  ".join(f"{n}  {lv}" for n, lv in parts) != text:
        print(f"{c}: junk in {text!r}"); bad += 1
    if got != names or len(parts) != 4 or len(text) != int(m.group(2)):
        print(f"{c}: players line {text!r} ({m.group(2)})"); bad += 1
    else:
        print(f"{c}: {text}")
sys.exit(1 if bad else 0)
