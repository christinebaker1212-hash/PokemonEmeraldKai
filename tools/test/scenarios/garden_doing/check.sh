#!/bin/bash
# garden_doing: the swimmer is in the water and sunk (deeper in its dive), the climber up, the digger in the ground
d=$1
L="$d/a.log"
grep -q "garden actor 0: dex 258 .* sink [1-9][0-9]* up 0 water 1" "$L" || { echo "MUDKIP never swam"; exit 1; }
s1=$(grep -o "garden actor 0: dex 258 .* sink [0-9]* up 0 water 1" "$L" | head -1 | sed 's/.* sink \([0-9]*\) .*/\1/')
s2=$(grep -o "garden actor 0: dex 258 .* sink [0-9]* up 0 water 1" "$L" | sed -n 2p | sed 's/.* sink \([0-9]*\) .*/\1/')
[ -n "$s2" ] && [ "$s2" -gt "$s1" ] || { echo "no dive (sink $s1 then $s2)"; exit 1; }
grep -q "garden actor 1: dex 252 .* up 20 water 0" "$L" || { echo "TREECKO isn't up the tree"; exit 1; }
grep -q "garden actor 2: dex 27 .* sink [1-9][0-9]* up 0 water 0" "$L" || { echo "SANDSHREW isn't in the ground"; exit 1; }
echo ok
