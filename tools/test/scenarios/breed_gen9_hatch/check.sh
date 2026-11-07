#!/bin/bash
# breed_gen9_hatch: the hatchling keeps the EGG's Ability slot
a=$(grep -o 'hatch: egg 1 .* ability [0-9]' "$1/a.log" | awk '{print $NF}')
b=$(grep -o 'hatched: egg 0 .* ability [0-9]' "$1/a.log" | awk '{print $NF}')
[ -n "$a" ] && [ "$a" = "$b" ] || { echo "Ability slot egg '$a' hatched '$b'"; exit 1; }
echo "hatch keeps ball and Ability ok"
