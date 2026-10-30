#!/bin/bash
# Moemon Fight: the character audit (6.4). Every pairing of the cabinet's characters (mirrors too) fights CPU
# against CPU on each seed (a random stage per seed) through mf_selftest's "audit" (stuck states, positions off
# the stage, runaway helpers, slow frames, endless rounds, the states each side reached), then the same pairing
# with made-up buttons twice, once going back 7 frames every 7 (rollback): the hashes must agree. mugen.log lines
# that look like script errors are kept per run.
#   tools/fight/audit_chars.sh [OUT]      SEEDS="1 2" JOBS=N ONLY="0,3" (one pairing) NOBUILD=1
# Writes OUT/summary.txt (default build/fight/audit). Exit 1 when anything was found.
cd "$(dirname "$0")/../.."
OUT=${1:-build/fight/audit}
SEEDS=${SEEDS:-"1 2"}
JOBS=${JOBS:-$(nproc)}
BIN=build/fight/linux/mf_selftest
[ -n "$NOBUILD" ] || bash tools/fight/build_linux.sh > /dev/null || exit 1
rm -rf "$OUT"
mkdir -p "$OUT/runs"
REPO=$(pwd)
printf 'roster\n' > "$OUT/roster.txt.in"
N=$($BIN "$OUT/roster.txt.in" "$OUT" | sed -n 's/^roster \([0-9]*\) characters.*/\1/p')
[ -n "$N" ] || { echo "no roster"; exit 1; }
pairs=()
if [ -n "$ONLY" ]; then
    pairs=("$ONLY")
else
    for ((i = 0; i < N; i++)); do for ((j = i; j < N; j++)); do pairs+=("$i,$j"); done; done
fi
one() { # PAIR SEED
    local pair=$1 seed=$2 d="$OUT/runs/${1/,/_}_$2" m
    mkdir -p "$d/mugen"
    for m in "$REPO"/mugen/*; do ln -s "$m" "$d/mugen/"; done
    rm -f "$d/mugen/mugen.log" "$d/mugen/fightfit.log"
    printf 'begin cpu %s\naudit 30000\n' "$seed" > "$d/watch.in"
    MOEMON_FIGHT_SYNC=1 MOEMON_MUGEN_DIR="$d/mugen" MOEMON_FIGHT_PICK="$pair" MOEMON_FIGHT_WATCH=1 MOEMON_FIGHT_WARN=1 timeout 600 "$REPO/$BIN" "$d/watch.in" "$d" \
        > "$d/watch.txt" 2>&1 || echo "audit ISSUE the CPU fight stopped with status $?" >> "$d/watch.txt"
    cp "$d/mugen/mugen.log" "$d/mugen_watch.log" 2> /dev/null
    # the match's first frame depends on how long loading took (the select and VS screens run meanwhile; online
    # both games wait for each other there): the two runs are compared from the same first frame, tried again
    # until they start together
    local try start0 start7
    for try in 1 2 3 4 5 6; do
        for back in 0 7; do
            printf 'begin versus %s rollback\nwait_kept 3000\nfight 2400 %s %s 60\n' "$seed" "$back" "$seed" > "$d/rb$back.in"
            MOEMON_FIGHT_SYNC=1 MOEMON_MUGEN_DIR="$d/mugen" MOEMON_FIGHT_PICK="$pair" timeout 600 "$REPO/$BIN" "$d/rb$back.in" "$d" \
                > "$d/rb$back.txt" 2>&1
        done
        start0=$(sed -n 's/^wait_kept: kept=1 frame //p' "$d/rb0.txt")
        start7=$(sed -n 's/^wait_kept: kept=1 frame //p' "$d/rb7.txt")
        [ -n "$start0" ] && [ "$start0" = "$start7" ] && break
    done
    [ -n "$start0" ] && [ "$start0" = "$start7" ] ||
        echo "audit ISSUE rollback: no two runs started on the same frame ($start0, $start7)" >> "$d/watch.txt"
    if ! diff -q <(grep '^hash ' "$d/rb0.txt") <(grep '^hash ' "$d/rb7.txt") > /dev/null ||
       [ -z "$(grep '^hash ' "$d/rb0.txt")" ]; then
        echo "audit ISSUE rollback: the hashes going back 7 frames differ from playing straight" >> "$d/watch.txt"
    fi
    grep -E "failed rollbacks" "$d/rb7.txt" | grep -v " 0 failed" | sed 's/^/audit ISSUE /' >> "$d/watch.txt"
    grep -aiE "error|warning|invalid|doesn't exist|not found|can't|cannot" "$d/mugen/mugen.log" 2> /dev/null \
        | sort -u > "$d/log_issues.txt"
    rm -rf "$d/mugen" "$d"/*.ppm
}
export -f one
export OUT REPO BIN
for p in "${pairs[@]}"; do for s in $SEEDS; do echo "$p $s"; done; done | xargs -P "$JOBS" -n 2 bash -c 'one "$0" "$1"'
{
    echo "Character audit: $N characters, ${#pairs[@]} pairings, seeds $SEEDS"
    for d in "$OUT"/runs/*; do
        r=$(basename "$d")
        names=$(grep -h "out of control\|^audit:" "$d/watch.txt" | head -0)
        sed -n "s/^audit: /$r: /p" "$d/watch.txt"
        sed -n "s/^audit ISSUE /$r ISSUE: /p" "$d/watch.txt"
        sed "s/^/$r LOG: /" "$d/log_issues.txt"
        grep -a '^fightwarn: ' "$d/watch.txt" | sed 's/^fightwarn: //' | sort | uniq -c | sed "s/^ */$r WARN x/"
    done
} > "$OUT/summary.txt"
grep -c "ISSUE" "$OUT/summary.txt" > /dev/null && issues=$(grep -c "ISSUE" "$OUT/summary.txt") || issues=0
echo "$issues issue(s); $(grep -c ' LOG: ' "$OUT/summary.txt") log line(s); $(grep -c ' WARN x' "$OUT/summary.txt") kinds of warning; $OUT/summary.txt"
[ "$issues" = 0 ]
