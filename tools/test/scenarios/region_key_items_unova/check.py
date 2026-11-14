"""region_key_items_unova: UNOVA's key items kept in UNOVA, then waiting in the GLOBAL PC under UNOVA's code."""
import re
import sys

log = open(sys.argv[1] + "/a.log", errors="replace").read()
items = (361, 671, 678, 505, 360, 812)
want = {"in UNOVA": ({i: 1 for i in items}, {}), "in HOENN": ({i: 0 for i in items}, {i: 16 for i in items})}
bad = 0
for part in re.split(r"\[ki\] ", log)[1:]:
    name = part.split("\n", 1)[0].strip()
    if name not in want:
        continue
    bag = {int(a): int(b) for a, b in re.findall(r"bagqty (\d+): (\d+)", part)}
    pc = {int(a): int(b) for a, b in re.findall(r"keyedpc (\d+): (\d+)", part)}
    for i, n in want[name][0].items():
        if bag.get(i) != n:
            print("%s: bag item %d x%s, want %d" % (name, i, bag.get(i), n)); bad += 1
    for i, n in want[name][1].items():
        if pc.get(i) != n:
            print("%s: GLOBAL PC item %d keys %s, want %d" % (name, i, pc.get(i), n)); bad += 1
    del want[name]
for name in want:
    print("no section: " + name); bad += 1
print("ok" if not bad else "%d wrong" % bad)
sys.exit(1 if bad else 0)
