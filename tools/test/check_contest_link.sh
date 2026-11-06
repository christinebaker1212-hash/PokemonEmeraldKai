#!/bin/bash
# sinnoh_contest_link: both games ran the same contest (the same opponents, judges and theme, the same final scores)
d=$1
a=$(grep -o "sinnoh contest: rank .*" "$d/a.log" | head -1)
b=$(grep -o "sinnoh contest: rank .*" "$d/b.log" | head -1)
[ -n "$a" ] && [ "$a" = "$b" ] || { echo "contests differ: '$a' / '$b'"; exit 1; }
ra=$(grep -o "sinnoh contest: results:.*), the player" "$d/a.log" | tail -1)
rb=$(grep -o "sinnoh contest: results:.*), the player" "$d/b.log" | tail -1)
[ -n "$ra" ] && [ "$ra" = "$rb" ] || { echo "results differ: '$ra' / '$rb'"; exit 1; }
echo "same contest: $a; $ra"
