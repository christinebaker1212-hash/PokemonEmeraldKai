#!/bin/bash
# Runs the scenarios in tools/test/scenarios and checks their logs. usage: regress.sh [NAME...]
#   Each scenario folder has a.txt (+ b.txt for two clients) and conf (MODE=1|2, DELAY/JIT/FPS/TMO/WARP/WARP_B,
#   EXPECT_A / EXPECT_B = text that must show up in a.log / b.log). Results go to $OUT/NAME (default /tmp/moemon_tests).
#   Builds the game first unless NOBUILD=1. Exit code = number of failed scenarios.
. "$(dirname "$0")/common.sh"
if [ -z "$NOBUILD" ]; then
    (cd "$REPO" && make linux -j"$(nproc)" > "$OUT.build.log" 2>&1) || { echo "build failed, see $OUT.build.log"; exit 99; }
fi
names=("$@"); all=0; [ ${#names[@]} = 0 ] && { names=($(ls "$TOOLS/scenarios")); all=1; }
fail=0
for n in "${names[@]}"; do
    src="$TOOLS/scenarios/$n"; [ -f "$src/conf" ] || { echo "?? $n: no such scenario"; fail=$((fail+1)); continue; }
    d="$OUT/$n"; rm -rf "$d"; mkdir -p "$d"; cp "$src"/*.txt "$d/"
    (   unset MODE DELAY JIT EXPECT_A EXPECT_B
        . "$src/conf"
        export WARP WARP_B FPS TMO
        start=$SECONDS
        if [ "$MODE" = 2 ]; then
            "$TOOLS/run2.sh" "$d" "${DELAY:-0}" "${JIT:-0}" > "$d/run.txt" 2>&1
        else
            [ -n "$WARP" ] && export MOEMON_DEBUG_WARP=$WARP
            "$TOOLS/run1.sh" "$d" > "$d/run.txt" 2>&1; mv "$d/log.txt" "$d/a.log"
        fi
        bad=""
        for e in "${EXPECT_A[@]}"; do grep -qF "$e" "$d/a.log" || bad="$bad\n    a.log lacks: $e"; done
        for e in "${EXPECT_B[@]}"; do grep -qF "$e" "$d/b.log" || bad="$bad\n    b.log lacks: $e"; done
        for l in "$d"/a.log "$d"/b.log; do
            [ -f "$l" ] && grep -q "TIMEOUT\|Segmentation\|Aborted\|CRASH" "$l" && bad="$bad\n    $(basename "$l"): $(grep -m1 "TIMEOUT\|Segmentation\|Aborted\|CRASH" "$l")"
        done
        if [ -z "$bad" ]; then echo "ok   $n ($((SECONDS-start)) s)"; else echo -e "FAIL $n ($((SECONDS-start)) s)$bad"; exit 1; fi
    ) || fail=$((fail+1))
done
[ $all = 1 ] && { "$TOOLS/version_check.sh" || fail=$((fail+1)); }
echo "$fail failed, results in $OUT"
exit $fail
