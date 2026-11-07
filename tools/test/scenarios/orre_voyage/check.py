#!/usr/bin/env python3
"""shadow_voyage's order: the sailor refused while a Shadow Pokémon was along (no "sailing back" before "shadow
refused", the player still in ORRE then), and sailed back to SLATEPORT after."""
import sys
log = open(sys.argv[1] + "/a.log", errors="replace").read()
r, b = log.find("shadow refused"), log.find("orre: sailing back")
where = [l for l in log.split("\n") if l.startswith("[test] where map")]
bad = []
if r < 0 or b < 0 or b < r:
    bad.append("the sailor sailed with a Shadow Pokémon aboard (or never sailed)")
if len(where) < 3 or not where[1].startswith("[test] where map 45.0") or not where[2].startswith("[test] where map 9.9"):
    bad.append("where: %s" % where)
print("\n".join(bad) or "ok: %s" % where)
sys.exit(1 if bad else 0)
