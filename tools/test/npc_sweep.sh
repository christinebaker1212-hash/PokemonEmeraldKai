#!/bin/bash
# Talks to every person and reads every sign on every map of some map groups, one game per map: for each object
# event and sign, warp below it, face up, mash A through what it says (battles too: foes at 1 HP), back out with B,
# and check the game is free again. Reports a crash, a hang, an out-of-memory heap, a script still holding the
# player, and specials that aren't ported.
# usage: tools/test/npc_sweep.sh [FIRST_GROUP LAST_GROUP]   (default 34 39: JOHTO and KANTO)
#   JOBS=n games at once (default 4), LIMIT=n only the first n maps, ONLY=MapName one map, REGION=0|1|2 (default: the
#   map group's region), KEEP=1 keeps every run folder; a failed map's log stays in $OUT/npc_sweep/<map>.log
. "$(dirname "$0")/common.sh"
G0=${1:-34}; G1=${2:-39}; JOBS=${JOBS:-4}
D="$OUT/npc_sweep"; rm -rf "$D"; mkdir -p "$D"
# GROUP NUM REGION COUNT NAME per map, and a test script per map in $D/scripts/NAME.txt
python3 - "$REPO" "$G0" "$G1" "$D" "${ONLY:-}" > "$D/maps.txt" <<'PY'
import json, os, sys
repo, g0, g1, out, only = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], sys.argv[5]
groups = json.load(open(os.path.join(repo, 'data/maps/map_groups.json')))
CARD_SCRIPTS = ('EventScript_CardClub', 'EventScript_TtPlayer', 'EventScript_TtQueen', 'EventScript_YgoClub',
                'EventScript_GcCardShop', 'EventScript_GcClerk')
os.makedirs(os.path.join(out, 'scripts'), exist_ok=True)
for g in range(g0, g1 + 1):
    for n, name in enumerate(groups[groups['group_order'][g]]):
        if only and name != only:
            continue
        m = json.load(open(os.path.join(repo, 'data/maps', name, 'map.json')))
        spots = []
        for i, o in enumerate(m.get('object_events') or [], 1):
            if o.get('type', 'object') != 'object' or o.get('script') in (None, '0x0', '0', 'NULL'):
                continue
            if o['script'] in CARD_SCRIPTS:   # card duels have their own tests (tools/tcg, ygo, tt); sellers stay
                continue
            spots.append(('object %d %s' % (i, o['script']), o['x'], o['y']))
        for b in m.get('bg_events') or []:
            if b.get('type') == 'sign' and b.get('script') not in (None, '0x0', '0', 'NULL'):
                spots.append(('sign %s' % b['script'], b['x'], b['y']))
        if not spots:
            continue
        lines = ['pressuntil A field 5000', 'wait 30', 'mp teach 0 44', 'mp teach 1 44', 'mp battlestyle 1']
        for what, x, y in spots:
            # A through what it says and any battle (A keeps the battle cursor on FIGHT and the first move); B out of
            # menus and questions; a card game still going is conceded; START then A to close a naming screen; B, A (a
            # long scene still going) and B again
            lines += ['mp warp %d,%d,%d,%d' % (g, n, x, y + 1), 'wait 40', 'mp face up', 'mash A 40 8',
                      'pressuntil A field 7000', 'mp cards concede', 'mp ygo concede', 'mp tt concede',
                      'pressuntil A field 1500', 'pressuntil B field 600', 'press START 4', 'wait 30', 'press A 4',
                      'wait 60', 'pressuntil B field 600', 'pressuntil A field 3000', 'pressuntil B field 600', 'wait 20', 'mp ctx', 'mp sweepnote %s' % what.replace(' ', '_')]
        lines += ['mp savesum', 'quit']
        open(os.path.join(out, 'scripts', name + '.txt'), 'w').write('\n'.join(lines) + '\n')
        region = 0 if g < 34 else (1 if g < 36 else 2)
        print(g, n, region, len(spots), name)
PY
[ -n "$LIMIT" ] && { head -n "$LIMIT" "$D/maps.txt" > "$D/m"; mv "$D/m" "$D/maps.txt"; }
one() {   # one GROUP NUM REGION COUNT NAME
    local d="$D/run_$5"; mkdir -p "$d"
    local t=$(( 60 + $4 * 60 ))
    (cd "$d" && env SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy MOEMON_TEST="$D/scripts/$5.txt" MOEMON_SHOTDIR="$d" \
        MOEMON_SKIP_COOP_SETUP=1 MOEMON_DEBUG_QUICKSTART=1 MOEMON_DEBUG_WARP="$1,$2,1,1" MOEMON_DEBUG_REGION="${REGION:-$3}" \
        MOEMON_DEBUG_PARTY=6:100,9:100 MOEMON_TEST_FOE_HP1=1 MOEMON_COOP_CONFIG="$d/a.ini" MOEMON_CONTROLS_CONFIG="$d/c.ini" \
        MOEMON_SAVE="$d/a.sav" timeout -k 5 "$t" "$BIN" > "$d/log.txt" 2>&1)
    local rc=$? why=""
    [ $rc = 124 ] || [ $rc = 137 ] && why="hang (timeout)"
    [ -z "$why" ] && [ $rc != 0 ] && why="exit $rc"
    grep -q "\[heap\] out of memory" "$d/log.txt" && why="${why:+$why, }heap out of memory"
    [ -e "$d/crash_log.txt" ] && why="${why:+$why, }crash_log.txt"
    grep -q "^\[test\] savesum:" "$d/log.txt" || why="${why:-did not finish}"
    # a script still holding the player after all that: the note after a "free 0" ctx line names the spot
    local held=$(awk '/^\[test\] ctx/{l=($0 ~ /free 0/)} /^\[test\] sweepnote/{if(l)print $3; l=0}' "$d/log.txt" | sort -u | tr '\n' ' ')
    local np=$(grep -o "special [A-Za-z0-9_]* not ported" "$d/log.txt" | sort -u | awk '{print $2}' | tr '\n' ' ')
    [ -n "$held" ] && echo "HELD $1.$2 $5: $held"
    [ -n "$np" ] && echo "STUB $1.$2 $5: $np"
    if [ -n "$why" ]; then cp "$d/log.txt" "$D/$5.log"; echo "FAIL $1.$2 $5: $why"; fi
    [ -n "$KEEP" ] || rm -rf "$d"
}
export -f one; export D BIN REGION KEEP
total=$(wc -l < "$D/maps.txt")
xargs -P "$JOBS" -L 1 bash -c 'one "$@"' _ < "$D/maps.txt" | tee "$D/report.txt"
nf=$(grep -c "^FAIL" "$D/report.txt"); nh=$(grep -c "^HELD" "$D/report.txt")
echo "npc sweep: $((total - nf))/$total maps ok, $nh with a script still holding the player (groups $G0-$G1)"
[ "$nf" = 0 ]
