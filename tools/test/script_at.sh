#!/bin/bash
# Names the script label a harness `mp ctx` line points into. usage: script_at.sh OFFSET [BIN]
#   OFFSET = the number after "ctx script at gStdScripts" (e.g. +123456)
BIN=${2:-$(dirname "$0")/../../pokeemerald64}
nm -n "$BIN" | python3 -c '
import sys
off = int(sys.argv[1])
syms = [(int(a, 16), n) for a, t, n in (l.split()[:3] for l in sys.stdin if len(l.split()) >= 3)]
base = next(a for a, n in syms if n == "gStdScripts")
want, best = base + off, None
for a, n in syms:
    if a > want:
        break
    best = (a, n)
print("%s+%d" % (best[1], want - best[0]))
' "${1#+}"
