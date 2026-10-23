#!/usr/bin/env python3
# Every "jar phase" line: no candy outside the glass, none deep inside another, all still.
import re, sys
log = open(sys.argv[1] + "/a.log", errors="replace").read()
lines = re.findall(r"candy: jar phase \d+, .*", log)
if not lines:
    sys.exit("no jarstate lines")
for l in lines:
    m = re.search(r"(\d+) outside the glass, (\d+) overlapping \(worst (\d+)/10 px\), top speed (\d+)/10, (\d+) hidden", l)
    if not m:
        sys.exit("can't read: " + l)
    outside, overlapping, worst, speed, hidden = map(int, m.groups())
    if outside or worst > 10 or speed > 5 or hidden:
        sys.exit("bad jar: " + l)
print("ok: %d jar states" % len(lines))
