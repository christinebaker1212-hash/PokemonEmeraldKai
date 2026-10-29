#!/bin/bash
# anim_modern's check: every move animation took as many frames on BATTLE ANIMS > MODERN as on CLASSIC (anim_classic,
# run first into the same results folder's parent). usage: check_anim_frames.sh RESULT_DIR
d="${1:?usage: check_anim_frames.sh RESULT_DIR}"
classic="$(dirname "$d")/anim_classic/a.log"
[ -f "$classic" ] || { echo "no anim_classic results: run it first"; exit 1; }
a=$(grep -o "animall: move [0-9]* took [0-9]* frames" "$classic")
b=$(grep -o "animall: move [0-9]* took [0-9]* frames" "$d/a.log")
n=$(echo "$a" | wc -l)
[ "$n" -gt 700 ] || { echo "only $n animations logged on CLASSIC"; exit 1; }
if [ "$a" != "$b" ]; then
    diff <(echo "$a") <(echo "$b") | head -20
    echo "MODERN animation lengths differ from CLASSIC"
    exit 1
fi
echo "all $n animations the same length on CLASSIC and MODERN"
