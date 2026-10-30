#!/bin/bash
# tt_online: both games' last "[tt-net]" lines name the same game (hash), the same winner and score
D=${1:?result folder}
a=$(grep "\[tt-net\]" "$D/a.log" | tail -1 | sed 's/.*hash \([0-9a-f]*\).*over \([0-9]\) winner \([0-9]\) score \(.*\)/\1 \2 \3 \4/')
b=$(grep "\[tt-net\]" "$D/b.log" | tail -1 | sed 's/.*hash \([0-9a-f]*\).*over \([0-9]\) winner \([0-9]\) score \(.*\)/\1 \2 \3 \4/')
echo "a: $a"; echo "b: $b"
[ -n "$a" ] && [ "$a" = "$b" ] && [ "$(echo "$a" | cut -d" " -f2)" = 1 ]
