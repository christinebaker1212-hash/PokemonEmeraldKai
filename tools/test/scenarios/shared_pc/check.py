#!/usr/bin/env python3
# Every Pokemon in exactly one place at the end: the server's storage file (boxes and hands) or one player's own
# storage (their last "monids" after the shared PC closed). Nothing doubled, nothing lost except what was released.
import glob, re, struct, sys
# check.py [any] DIR ("any": just no doubles, for the other shared PC scenarios)
d = sys.argv[-1]
any_mode = len(sys.argv) > 2 and sys.argv[1] == "any"
SLOTS, BLOB, HANDS, HAND = 14 * 30, 112, 4, 36
store = glob.glob(d + "/coop_store_*.dat")
if not store:
    sys.exit("no store file")
raw = open(store[0], "rb").read()
assert raw.startswith(b"MOEST2\n"), raw[:7]
p = 7 + 32
used = raw[p:p + SLOTS]; p += SLOTS
mons = raw[p:p + SLOTS * BLOB]; p += SLOTS * BLOB
p += 14 * 9 + 14
hands = raw[p:p + HANDS * (1 + HAND * BLOB)]
server = []
for s in range(SLOTS):
    if used[s]:
        server.append("%08x:%08x" % struct.unpack_from("<II", mons, s * BLOB))
for h in range(HANDS):
    base = h * (1 + HAND * BLOB)
    for i in range(hands[base]):
        server.append("%08x:%08x" % struct.unpack_from("<II", hands, base + 1 + i * BLOB))
def last_ids(log):
    text = open(d + "/" + log, errors="replace").read()
    blocks = text.split("[test] monids done")
    if len(blocks) < 2:
        sys.exit(log + ": no monids")
    last = blocks[-2]
    # (the parties are left out: test players all start with the same two, same IDs and all)
    return re.findall(r"\[test\] monid own ([0-9a-f:]+) \d+ box", last), re.findall(r"\[test\] monid hand ([0-9a-f:]+)", last)
a_own, a_hand = last_ids("a.log")
b_own, b_hand = last_ids("b.log")
if a_hand or b_hand:
    sys.exit("something still in a hand: %s %s" % (a_hand, b_hand))
everything = server + a_own + b_own
dups = sorted(set(x for x in everything if everything.count(x) > 1))
if dups:
    sys.exit("in two places: " + " ".join(dups))
print("server %d, ALICE %d, BOB %d" % (len(server), len(a_own), len(b_own)))
# ALICE put 4 in; BOB released one: 3 left between the server and the players' boxes (besides their parties)
if any_mode:
    sys.exit(0)
if len(server) != 1:
    sys.exit("the server should keep 1 (BULBASAUR), has %d" % len(server))
