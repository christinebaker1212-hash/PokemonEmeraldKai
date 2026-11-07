#!/bin/bash
# breed_gen9: the four "breed2" lines (conf says what each wants)
mapfile -t L < <(grep -o 'breed2 .*' "$1/a.log")
[ ${#L[@]} = 4 ] || { echo "expected 4 breed2 lines, got ${#L[@]}"; exit 1; }
v() { echo "${L[$1]}" | grep -o "$2 [0-9/:]*" | head -1 | awk '{print $2}'; }
sp() { echo "${L[$1]}" | awk '{print $3" "$4}'; }   # "S1:n1 S2:n2"
fail() { echo "line $(($1 + 1)): $2 (${L[$1]})"; exit 1; }
both() { # line, species x, species y: both seen, each at least 120 of 400, nothing else
    local s; s=$(sp $1)
    for x in "$2" "$3"; do
        n=$(echo "$s" | tr ' ' '\n' | awk -F: -v x=$x '$1==x{print $2}')
        [ -n "$n" ] && [ "$n" -ge 120 ] || fail $1 "species $x seen ${n:-0} times"
    done
    [ "$(v $1 other)" = 0 ] || fail $1 "other species"
}
both 0 29 32
[ "$(v 0 ballA)" = 400 ] || fail 0 "not all in the GREAT BALL"
[ "$(v 0 power)" = 400 ] || fail 0 "the POWER BRACER's ATTACK didn't always pass"
both 1 386 387
[ "$(v 2 ballA)" = 400 ] || fail 2 "not all in the mother's MASTER BALL"
[ "$(v 2 knowsA)" = 400 ] || fail 2 "the mother's egg move didn't always pass"
[ "$(v 2 knowsB)" = 0 ] || fail 2 "a TM move passed down"
[ "$(sp 3)" = "350:400 0:0" ] || fail 3 "not all AZURILL"
[ "$(v 3 ballA)" = 0 ] || fail 3 "an EGG came in the ASTRAL BALL"
a=$(v 3 ability); same=${a%/*}; all=${a#*/}
[ "$all" = 400 ] && [ "$same" -ge 280 ] && [ "$same" -le 360 ] || fail 3 "Ability slot from MARILL $a (want about 320/400)"
echo "breeding gen 9 ok"
