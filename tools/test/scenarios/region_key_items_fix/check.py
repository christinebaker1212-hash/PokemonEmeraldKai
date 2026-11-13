"""region_key_items_fix: each section's BAG (mp bagqty) and the GLOBAL PC's keyed copies (mp keyedpc: bits of the
regions whose copy waits there; HOENN 1, KANTO 4) as the keyed key items want them."""
import re
import sys

log = open(sys.argv[1] + "/a.log", errors="replace").read()
want = {
    "in KANTO": ({259: 0, 262: 0, 264: 0, 399: 1}, {}),           # (HOENN beaten: still only Kai's own cross)
    "put right": ({259: 0, 262: 0, 264: 1, 399: 1}, {259: 1, 262: 1, 264: 1}),
    # (a save that crossed under the old rule: HOENN's bike and OLD ROD to the GLOBAL PC under HOENN's key; the SUPER
    #  ROD KANTO's fisherman gave (flag set) is KANTO's)
    "KANTO rod": ({262: 1}, {262: 1}),                            # (KANTO's own OLD ROD earned; HOENN's still waits)
    "KANTO beaten": ({259: 1, 262: 1, 264: 1}, {259: 0, 262: 0}),  # (KANTO's postgame: every tool, one of each)
    "back in HOENN": ({259: 1, 262: 1, 264: 1, 399: 1}, {262: 0, 264: 0}),   # (HOENN's own back; HOENN is beaten:
                                                                             # KANTO's rods lent too, one of each)
}
parts = re.split(r"\[ki\] ", log)
bad = 0
for part in parts[1:]:
    name = part.split("\n", 1)[0].strip()
    if name not in want:
        continue
    bag = {int(a): int(b) for a, b in re.findall(r"bagqty (\d+): (\d+)", part)}
    pc = {int(a): int(b) for a, b in re.findall(r"keyedpc (\d+): (\d+)", part)}
    for item, n in want[name][0].items():
        if bag.get(item) != n:
            print("%s: bag item %d x%s, want %d" % (name, item, bag.get(item), n))
            bad += 1
    for item, n in want[name][1].items():
        if pc.get(item) != n:
            print("%s: GLOBAL PC item %d keys %s, want %d" % (name, item, pc.get(item), n))
            bad += 1
    del want[name]
for name in want:
    print("no section: " + name)
    bad += 1
print("ok" if not bad else "%d wrong" % bad)
sys.exit(1 if bad else 0)
