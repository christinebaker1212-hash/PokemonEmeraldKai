#!/bin/bash
# Runs the scenarios in tools/test/scenarios and checks their logs. usage: regress.sh [NAME...]
#   Each scenario folder has a.txt (+ b.txt, c.txt for two / three clients) and conf (MODE=1|2|3,
#   DELAY/JIT/FPS/TMO/WARP/WARP_B/WARP_C, EXPECT_A / EXPECT_B / EXPECT_C = text that must show up in a.log / b.log /
#   c.log). Results go to $OUT/NAME (default /tmp/moemon_tests).
#   Builds the game first unless NOBUILD=1. WORST=1 runs every two-client scenario at 250 ms + 80 ms jitter
#   each way (NA <-> JP). JOBS=N runs N scenarios at once (default 1). Exit code = number of failed scenarios.
. "$(dirname "$0")/common.sh"
mkdir -p "$OUT"
if [ -z "$NOBUILD" ]; then
    (cd "$REPO" && make linux -j"$(nproc)" > "$OUT/build.log" 2>&1) || { echo "build failed, see $OUT/build.log"; exit 99; }
fi
build_server || exit 99
names=("$@"); all=0; [ ${#names[@]} = 0 ] && { names=($(ls "$TOOLS/scenarios")); all=1; }

run_one() {
    local n=$1 src="$TOOLS/scenarios/$1" d="$OUT/$1" bad="" e l start=$SECONDS
    [ -f "$src/conf" ] || { echo "?? $n: no such scenario"; return 1; }
    rm -rf "$d"; mkdir -p "$d"; cp "$src"/*.txt "$d/"
    unset MODE DELAY JIT FPS TMO WARP WARP_B WARP_C EXPECT_A EXPECT_B EXPECT_C ENV_A ENV_B ENV_C
    . "$src/conf"
    if [ -n "$WORST" ] && [ "$MODE" -ge 2 ] && [ -z "$DELAY" ]; then   # NA <-> JP for every multi-client scenario
        DELAY=250; JIT=80; FPS=60; TMO=$(( ${TMO:-150} * 2 ))
    fi
    export WARP WARP_B WARP_C FPS TMO ENV_A ENV_B ENV_C
    if [ "$MODE" = 3 ]; then
        "$TOOLS/runN.sh" 3 "$d" "${DELAY:-0}" "${JIT:-0}" > "$d/run.txt" 2>&1
    elif [ "$MODE" = 2 ]; then
        "$TOOLS/run2.sh" "$d" "${DELAY:-0}" "${JIT:-0}" > "$d/run.txt" 2>&1
    else
        [ -n "$WARP" ] && export MOEMON_DEBUG_WARP=$WARP
        "$TOOLS/run1.sh" "$d" $ENV_A > "$d/run.txt" 2>&1; mv "$d/log.txt" "$d/a.log"
    fi
    for e in "${EXPECT_A[@]}"; do grep -qF -- "$e" "$d/a.log" || bad="$bad\n    a.log lacks: $e"; done
    for e in "${EXPECT_B[@]}"; do grep -qF -- "$e" "$d/b.log" || bad="$bad\n    b.log lacks: $e"; done
    for e in "${EXPECT_C[@]}"; do grep -qF -- "$e" "$d/c.log" || bad="$bad\n    c.log lacks: $e"; done
    for l in "$d"/a.log "$d"/b.log "$d"/c.log; do
        [ -f "$l" ] && grep -q "TIMEOUT\|Segmentation\|Aborted\|CRASH" "$l" && bad="$bad\n    $(basename "$l"): $(grep -m1 "TIMEOUT\|Segmentation\|Aborted\|CRASH" "$l")"
    done
    if [ -z "$bad" ]; then echo "ok   $n ($((SECONDS-start)) s)"; else echo -e "FAIL $n ($((SECONDS-start)) s)$bad"; return 1; fi
}

fail=0; pids=()
for n in "${names[@]}"; do
    if [ "${JOBS:-1}" -gt 1 ]; then
        while [ $(jobs -rp | wc -l) -ge "$JOBS" ]; do wait -n; done
        ( run_one "$n" > "$OUT/$n.result" 2>&1 ) &
    else
        run_one "$n" || fail=$((fail+1))
    fi
done
if [ "${JOBS:-1}" -gt 1 ]; then
    wait
    for n in "${names[@]}"; do cat "$OUT/$n.result"; grep -q "^ok" "$OUT/$n.result" || fail=$((fail+1)); done
fi
[ $all = 1 ] && { "$TOOLS/version_check.sh" || fail=$((fail+1)); }
echo "$fail failed, results in $OUT"
exit $fail
