#!/bin/bash
# each of the two that fought got the foe's whole EXP
log="$1/a.log"
whole=$(grep -oE "exp: [0-9]+ from the foe" "$log" | head -1 | grep -oE "[0-9]+")
s0=$(grep -oE "exp: slot 0 \+[0-9]+" "$log" | head -1 | grep -oE "[0-9]+$")
s1=$(grep -oE "exp: slot 1 \+[0-9]+" "$log" | head -1 | grep -oE "[0-9]+$")
[ -n "$whole" ] && [ "$s0" = "$whole" ] && [ "$s1" = "$whole" ] && echo "ok: both got $whole" || { echo "whole $whole, slot 0 $s0, slot 1 $s1"; exit 1; }
