#!/bin/bash
# breed_items: line 1 no items (3 IVs, natures by chance), 2 EVERSTONE on A (all A's nature), 3 on B (all B's),
# 4 DESTINY KNOT (at least 5 IVs), 5 EVERSTONE + DESTINY KNOT (A's nature, 5 IVs)
L=($(grep -o 'natureA [0-9]* natureB [0-9]* .* min [0-9]*' "$1/a.log" | awk '{print $2"/"$4"/"$(NF)}'))
[ ${#L[@]} = 5 ] || { echo "expected 5 lines, got ${#L[@]}"; exit 1; }
ok() { echo "${L[$1]}" | awk -F/ -v a="$2" -v b="$3" -v m="$4" '{exit !((a=="*"||$1==a)&&(b=="*"||$2==b)&&$3>=m)}' || { echo "line $1: ${L[$1]} (want $2/$3/min $4)"; exit 1; }; }
ok 0 '*' '*' 3; ok 1 400 0 3; ok 2 0 400 3; ok 3 '*' '*' 5; ok 4 400 0 5
echo "breeding ok"
