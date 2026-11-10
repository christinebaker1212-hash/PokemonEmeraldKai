"""region_key_items: each section's BAG counts (mp bagqty) as the key item rules want them."""
import re
import sys

log = open(sys.argv[1] + "/a.log", errors="replace").read()
want = {
    "in JOHTO": {259: 0, 262: 0, 399: 1, 339: 0, 275: 0},   # (HOENN unbeaten: its tools, story item and HM stay)
    "JOHTO beaten": {262: 1, 339: 1},                         # (JOHTO's rod travels for good; HM01 back from the GLOBAL PC:
                                                              # the region the player is in is beaten)
    "back in HOENN": {259: 1, 262: 1, 339: 0, 275: 1},        # (the stash back; one rod: HOENN's merged; the HM in the
                                                              # GLOBAL PC again: HOENN isn't beaten)
    "HOENN beaten": {339: 1},                                 # (HM01 out of the GLOBAL PC)
    "in KANTO": {259: 1, 262: 1, 339: 0, 275: 0, 399: 1},     # (HOENN beaten: its bike travels, its ticket stays; an HM
                                                              # never travels: KANTO isn't beaten)
}
parts = re.split(r"\[walk\] |\[ki\] ", log)
bad = 0
for part in parts[1:]:
    name = part.split("\n", 1)[0].strip()
    if name not in want:
        continue
    got = {int(a): int(b) for a, b in re.findall(r"bagqty (\d+): (\d+)", part)}
    for item, n in want[name].items():
        if got.get(item) != n:
            print("%s: item %d x%s, want %d" % (name, item, got.get(item), n))
            bad += 1
    del want[name]
for name in want:
    print("no section: " + name)
    bad += 1
print("ok" if not bad else "%d wrong" % bad)
sys.exit(1 if bad else 0)
