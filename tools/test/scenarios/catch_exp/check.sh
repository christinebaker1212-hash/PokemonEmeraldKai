#!/bin/bash
# the ZIGZAGOON was at full HP (its HP is random): "catch exp: hp N/N, 100% of a faint"
grep -qE "catch exp: hp ([0-9]+)/\1, 100% of a faint" "$1/a.log" && echo "ok: full HP, 100%" || { echo "no full-HP catch at 100%"; exit 1; }
