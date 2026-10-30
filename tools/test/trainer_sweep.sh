#!/bin/bash
# Every trainer battle, one game each: two Lv. 100 MEWTWO with BITE (no type is immune to it) against the trainer's
# own team left at 1 HP (MOEMON_TEST_FOE_HP1), so each battle is a few turns. Reports a crash, a hang (no return to
# the field) or an out-of-memory heap; a battle that isn't won is listed apart ("not won": a WONDER GUARD or such).
# usage: tools/test/trainer_sweep.sh [IDS...]   (default: HOENN 1-854 and JOHTO / KANTO 864 to the last)
#   JOBS=n games at once (default 4); a failed trainer's log stays in $OUT/trainer_sweep/<id>.log
. "$(dirname "$0")/common.sh"
JOBS=${JOBS:-4}
D="$OUT/trainer_sweep"; rm -rf "$D"; mkdir -p "$D"
printf 'pressuntil A field 5000\nmp moves 0 44\nmp moves 1 44\nmp money 0\nmp battlestyle 1\nmp trainer %s\npressuntil A field 20000\nwait 60\nmp savesum\nquit\n' '$ID' > "$D/a.tmpl"
if [ $# -gt 0 ]; then printf '%s\n' "$@" > "$D/ids.txt"; else
    last=$(grep -oE "define TRAINER_QZ_[0-9]+ +[0-9]+" "$REPO/include/constants/qz_port.h" | awk '{print $3}' | sort -n | tail -1)
    count=$(grep -oE "define TRAINERS_COUNT +[0-9]+" "$REPO/include/constants/opponents.h" | awk '{print $3}')
    { seq 1 $((count - 1)); [ -n "$last" ] && seq 864 "$last" | grep -vxE "1022|1023|1024"; } > "$D/ids.txt"   # (Emerald's special numbers)
fi
one() {   # one ID
    local d="$D/run_$1"; mkdir -p "$d"
    sed "s/\$ID/$1/" "$D/a.tmpl" > "$d/a.txt"
    (cd "$d" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$d/a.txt" MOEMON_SHOTDIR="$d" \
        MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_PARTY=150:100,150:100 MOEMON_TEST_FOE_HP1=1 \
        MOEMON_COOP_CONFIG="$d/a.ini" MOEMON_CONTROLS_CONFIG="$d/c.ini" \
        MOEMON_SAVE="$d/a.sav" timeout -k 5 150 "$BIN" > "$d/log.txt" 2>&1)
    local rc=$? why=""
    [ $rc = 124 ] || [ $rc = 137 ] && why="hang (timeout)"
    [ -z "$why" ] && [ $rc != 0 ] && why="exit $rc"
    grep -q "\[heap\] out of memory" "$d/log.txt" && why="${why:+$why, }heap out of memory"
    grep -q "\[crash\]" "$d/log.txt" && why="${why:+$why, }crash"
    grep -q "TIMEOUT" "$d/log.txt" && why="${why:+$why, }$(grep -m1 TIMEOUT "$d/log.txt" | cut -c1-80)"
    grep -q "^\[test\] savesum:" "$d/log.txt" || why="${why:-did not finish}"
    if [ -n "$why" ]; then cp "$d/log.txt" "$D/$1.log"; echo "FAIL $1: $why"
    elif grep -q "savesum: .* money 0 " "$d/log.txt"; then echo "LOST $1"; fi
    [ -n "$KEEP" ] || rm -rf "$d"
}
export -f one; export D BIN
total=$(wc -l < "$D/ids.txt")
xargs -P "$JOBS" -I{} bash -c 'one {}' < "$D/ids.txt" > "$D/fails.txt" 2>/dev/null
nf=$(grep -c "^FAIL" "$D/fails.txt"); nl=$(grep -c "^LOST" "$D/fails.txt")
cat "$D/fails.txt"
echo "trainer sweep: $((total - nf)) of $total trainers without a crash or hang ($nl not won)"
[ "$nf" = 0 ]
