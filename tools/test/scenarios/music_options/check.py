#!/usr/bin/env python3
# music_options: the songs, in order (see conf)
import re, sys
d = sys.argv[-1]
got = [int(m) for m in re.findall(r"\[test\] music (\d+)", open(d + "/a.log", errors="replace").read())]
# start, Mach Bike, off, Mach Bike with BIKE MUSIC off, surfing with SURF MUSIC off, the map's song with it on, then off,
# again, dived with it off, the map's song with it on, again
want = [362, 403, 362, 362, 362, 365, 362, 362, 362, 411, 411]
if got != want:
    sys.exit("songs %s, want %s" % (got, want))
print("ok: songs", got)
