#!/bin/bash
log="$1/a.log"
n=$(grep -c "gatcha: training bonus, x" "$log")
low=$(grep -oE "training bonus, x[0-9]+" "$log" | grep -oE "[0-9]+$" | awk '$1 < 4' | wc -l)
[ "$n" -ge 10 ] && [ "$low" -eq 0 ] && echo "ok: $n bonus prizes of 20, none under x4" || { echo "$n bonus prizes, $low under x4"; exit 1; }
