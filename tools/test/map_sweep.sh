#!/bin/bash
# Every map of some map groups, one game each: start there (MOEMON_DEBUG_WARP), let its arrival scripts run with A
# mashed through their text, walk a little, and report a crash, a hang or an out-of-memory heap.
# usage: tools/test/map_sweep.sh [FIRST_GROUP LAST_GROUP]   (default 34 39: JOHTO and KANTO)
#   JOBS=n games at once (default 4), LIMIT=n only the first n maps; a failed map's log stays in $OUT/map_sweep/<map>.log
. "$(dirname "$0")/common.sh"
G0=${1:-34}; G1=${2:-39}; JOBS=${JOBS:-4}
D="$OUT/map_sweep"; rm -rf "$D"; mkdir -p "$D"
printf 'pressuntil A field 5000\nwait 60\nmash A 12 10\npress LEFT 20\npress UP 20\nmash B 6 10\npress RIGHT 20\npress DOWN 20\nwait 60\nmp savesum\nquit\n' > "$D/a.txt"
# GROUP NUM X Y NAME per map: a warp's spot, else the middle of its layout
python3 - "$REPO" "$G0" "$G1" > "$D/maps.txt" <<'PY'
import json, os, sys
repo, g0, g1 = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
groups = json.load(open(os.path.join(repo, 'data/maps/map_groups.json')))
layouts = {l['id']: l for l in json.load(open(os.path.join(repo, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
for g in range(g0, g1 + 1):
    for n, name in enumerate(groups[groups['group_order'][g]]):
        m = json.load(open(os.path.join(repo, 'data/maps', name, 'map.json')))
        w = m.get('warp_events') or []
        if w:
            x, y = w[0]['x'], w[0]['y']
        else:
            l = layouts.get(m.get('layout'), {})
            x, y = l.get('width', 10) // 2, l.get('height', 10) // 2
        print(g, n, x, y, name)
PY
# rooms only a special flow enters, which a direct start can't set up (the same in the original game):
# the e-Reader trainer's basement (a valid e-Reader trainer first), the UNION ROOM (a link), BATTLE PIKE rooms (a challenge)
grep -vE " (SootopolisCity_MysteryEventsHouse_B1F|UnionRoom|BattleFrontier_BattlePikeRoom\w*)$" "$D/maps.txt" > "$D/m"; mv "$D/m" "$D/maps.txt"
[ -n "$LIMIT" ] && { head -n "$LIMIT" "$D/maps.txt" > "$D/m"; mv "$D/m" "$D/maps.txt"; }
one() {   # one GROUP NUM X Y NAME
    local d="$D/run_$5"; mkdir -p "$d"
    (cd "$d" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$D/a.txt" MOEMON_SHOTDIR="$d" \
        MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_WARP="$1,$2,$3,$4" \
        MOEMON_DEBUG_PARTY=280:30,277:30 MOEMON_COOP_CONFIG="$d/a.ini" MOEMON_CONTROLS_CONFIG="$d/c.ini" \
        MOEMON_SAVE="$d/a.sav" timeout -k 5 90 "$BIN" > "$d/log.txt" 2>&1)
    local rc=$? why=""
    [ $rc = 124 ] || [ $rc = 137 ] && why="hang (timeout)"
    [ -z "$why" ] && [ $rc != 0 ] && why="exit $rc"
    grep -q "\[heap\] out of memory" "$d/log.txt" && why="${why:+$why, }heap out of memory"
    [ -e "$d/crash_log.txt" ] && why="${why:+$why, }crash_log.txt"
    grep -q "^\[test\] savesum:" "$d/log.txt" || why="${why:-did not finish}"
    if [ -n "$why" ]; then cp "$d/log.txt" "$D/$5.log"; echo "FAIL $1.$2 $5: $why"; fi
    [ -n "$KEEP" ] || rm -rf "$d"
}
export -f one; export D BIN
total=$(wc -l < "$D/maps.txt")
xargs -P "$JOBS" -L 1 bash -c 'one "$@"' _ < "$D/maps.txt" | tee "$D/fails.txt"
nf=$(grep -c "^FAIL" "$D/fails.txt")
echo "map sweep: $((total - nf))/$total maps ok (groups $G0-$G1)"
[ "$nf" = 0 ]
