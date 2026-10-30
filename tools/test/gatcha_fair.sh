#!/bin/bash
# MoeGatcha fairness: (1) free pulls only for a later day: one save launched on the 27th, the 26th, the 27th again
#   and the 28th gets the daily free pulls twice (27th, 28th), not on every launch; (2) the same 10-pull on the same
#   frame of two identical launches gives different prizes (the game's random numbers are stirred with the PC's timer).
# usage: gatcha_fair.sh [OUT]
. "$(dirname "$0")/common.sh"
D="${1:-$OUT/gatcha_fair}"; rm -rf "$D"; mkdir -p "$D"; D="$(cd "$D" && pwd)"
fail=0
printf 'pressuntil A field 5000\nwait 700\nmp gatchastock\nquit\n' > "$D/a.txt"
got=""
for day in 20261027 20261026 20261027 20261028; do
    "$TOOLS/run1.sh" "$D" MOEMON_DEBUG_DATE=$day > /dev/null
    grep -q "gatcha: daily free pull" "$D/log.txt" && got="$got $day"
done
echo "daily free pulls given on:$got"
[ "$got" = " 20261027 20261028" ] || { echo "FAIL: free pulls for a clock set back"; fail=1; }
printf 'pressuntil A field 5000\nwait 700\nmp gatcha\nwait 120\npress A 2\nwait 30\npress A 2\nwait 900\nquit\n' > "$D/a.txt"
for i in 1 2; do
    rm -f "$D/a.sav.coop"
    "$TOOLS/run1.sh" "$D" MOEMON_DEBUG_DATE=20261027 > /dev/null
    grep -h "gatcha: results" "$D/log.txt" > "$D/pull$i.txt"
    echo "launch $i:$(cut -d: -f2- "$D/pull$i.txt" | sed 's/^ results//')"
done
[ -s "$D/pull1.txt" ] || { echo "FAIL: no pull"; fail=1; }
cmp -s "$D/pull1.txt" "$D/pull2.txt" && { echo "FAIL: the same pull twice"; fail=1; }
[ $fail = 0 ] && echo "PASS"
exit $fail
