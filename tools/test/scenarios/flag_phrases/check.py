#!/usr/bin/env python3
# flag_phrases: the winner got a tenth of the loser's money (from before the battle), the loser kept all of theirs
import re, sys
d = sys.argv[-1]
money = {}
for c in "ab":
    log = open("%s/%s.log" % (d, c), errors="replace").read()
    vals = [int(v) for v in re.findall(r"\[test\] money (\d+)", log)]
    if len(vals) < 2:
        sys.exit("%s: money not printed twice" % c)
    won = "won the battle: +" in log
    lost = "lost the battle: money kept" in log
    money[c] = (vals[0], vals[-1], won, lost)
winners = [c for c in "ab" if money[c][2]]
losers = [c for c in "ab" if money[c][3]]
if len(winners) != 1 or len(losers) != 1:
    sys.exit("no single winner / loser: %r" % money)
w, l = winners[0], losers[0]
if money[l][1] != money[l][0]:
    sys.exit("the loser's money changed: %d -> %d" % money[l][:2])
if money[w][1] != money[w][0] + money[l][0] // 10:
    sys.exit("the winner got %d, not a tenth of %d" % (money[w][1] - money[w][0], money[l][0]))
print("ok: %s won %d, %s kept %d" % (w, money[l][0] // 10, l, money[l][0]))
