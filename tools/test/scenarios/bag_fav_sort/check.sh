#!/bin/bash
# bag_fav_sort: the four FAVORITES orders in turn: marked, moved, ABC, MANUAL again
d=$1
got=$(grep "^\[test\] bagview 7:" "$d/a.log" | tr '\n' '|')
want="[test] bagview 7: 139 4 13|[test] bagview 7: 4 13 139|[test] bagview 7: 139 4 13|[test] bagview 7: 4 13 139|"
[ "$got" = "$want" ] || { echo "FAVORITES orders: $got (want $want)"; exit 1; }
echo ok
