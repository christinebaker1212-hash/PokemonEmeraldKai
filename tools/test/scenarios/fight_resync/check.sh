#!/bin/bash
# fight_resync: after the host's state was loaded, both games agree at every check to the end
d=$1
for c in a b; do
    after=$(sed -n '/fight: desync: \(both games go on\|the host.s state loaded\)/,$p' "$d/$c.log" | grep -c "fight: desync at")
    [ "$after" = 0 ] || { echo "$c.log: $after desyncs after the recovery"; exit 1; }
    grep -q "1 recovered" "$d/$c.log" || { echo "$c.log: no recovery counted"; exit 1; }
done
ea=$(grep -o "online result [0-9]* (engine [0-9]*" "$d/a.log" | grep -o "engine [0-9]*")
eb=$(grep -o "online result [0-9]* (engine [0-9]*" "$d/b.log" | grep -o "engine [0-9]*")
[ "$ea" = "$eb" ] || { echo "the games ended differently: $ea / $eb"; exit 1; }
echo ok
