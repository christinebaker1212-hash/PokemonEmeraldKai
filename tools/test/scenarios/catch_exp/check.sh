#!/bin/bash
# the ZIGZAGOON was at full HP (its HP is random): "catch exp: hp N/N, 10% of a faint"
grep -qE "catch exp: hp ([0-9]+)/\1, 10% of a faint" "$1/a.log" && echo "ok: full HP, 10%" || { echo "no full-HP catch at 10%"; exit 1; }
