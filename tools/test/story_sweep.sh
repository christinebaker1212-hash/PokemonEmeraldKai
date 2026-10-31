#!/bin/bash
# Fires every story trigger of some map groups, one game per map: each step trigger (a map's coord events) with its
# variable set to the value it waits for, the player standing on its cell (harness mp steptrigger); and each map's
# frame scripts (map_script_2: VAR, VALUE) by setting the variable and warping in. Then it mashes A through what plays
# (battles too: foes at 1 HP; card games conceded) and checks the game is free again and the variable moved on.
# Reports: FAIL (crash, hang, out-of-memory heap), HELD (a script still holding the player), STUB (a special that isn't
# ported), STILL (the variable didn't change: a guard that pushes the player back, or a story event that didn't
# advance; look at the list), NOFIRE (nothing ran: the trigger didn't start a script).
# usage: tools/test/story_sweep.sh [FIRST_GROUP LAST_GROUP]   (default 34 39: JOHTO and KANTO)
#   JOBS=n games at once (default 4), ONLY=MapName[,MapName...], KEEP=1 keeps every run folder, BIN=the game
. "$(dirname "$0")/common.sh"
G0=${1:-34}; G1=${2:-39}; JOBS=${JOBS:-4}
D="$OUT/story_sweep"; rm -rf "$D"; mkdir -p "$D"
python3 - "$REPO" "$G0" "$G1" "$D" "${ONLY:-}" > "$D/maps.txt" <<'PY'
import json, os, re, struct, sys
repo, g0, g1, out, only = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), sys.argv[4], sys.argv[5]
groups = json.load(open(os.path.join(repo, 'data/maps/map_groups.json')))
layouts = {l['id']: l for l in json.load(open(os.path.join(repo, 'quetzal_rip/layouts/layouts.json')))['layouts'] if 'id' in l}
layouts.update({l['id']: l for l in json.load(open(os.path.join(repo, 'data/layouts/layouts.json')))['layouts'] if 'id' in l})
consts = {}
for f in ('include/constants/vars.h', 'include/constants/qz_port.h'):
    p = os.path.join(repo, f)
    if os.path.exists(p):
        for m in re.finditer(r'#define (VAR_\w+)\s+(0x[0-9A-Fa-f]+|\d+)', open(p).read()):
            consts[m.group(1)] = int(m.group(2), 0)


def var_id(v):
    if v in consts:
        return consts[v]
    m = re.match(r'VAR_QZ_(0x[0-9A-Fa-f]+)$', v)
    return int(m.group(1), 16) if m else None


os.makedirs(os.path.join(out, 'scripts'), exist_ok=True)
for g in range(g0, g1 + 1):
    for n, name in enumerate(groups[groups['group_order'][g]]):
        if only and name not in only.split(','):
            continue
        m = json.load(open(os.path.join(repo, 'data/maps', name, 'map.json')))
        lay = layouts.get(m['layout'])
        if not lay:
            continue
        blk = open(os.path.join(repo, lay['blockdata_filepath']), 'rb').read()
        w, h = lay['width'], lay['height']
        objs = {(o['x'], o['y']) for o in m.get('object_events') or []}

        def free(x, y):
            return 0 <= x < w and 0 <= y < h and not (struct.unpack_from('<H', blk, (y * w + x) * 2)[0] >> 10) & 3 \
                and (x, y) not in objs
        coords = {(c['x'], c['y']) for c in m.get('coord_events') or []}
        events = []   # (kind, var, value, x, y, from_x, from_y, dir, script)
        for c in m.get('coord_events') or []:
            if c.get('type', 'trigger') != 'trigger' or c.get('script') in (None, '0x0', 'NULL'):
                continue
            vid = var_id(c.get('var', '0')) if c.get('var', '0') not in ('0', 0) else 0
            if vid is None:
                continue
            start = None
            for strict in (True, False):   # (a cell with a trigger of its own only if nothing else is free)
                for dx, dy, d in ((0, 1, 'up'), (0, -1, 'down'), (1, 0, 'left'), (-1, 0, 'right')):
                    fx, fy = c['x'] + dx, c['y'] + dy
                    if free(fx, fy) and (not strict or (fx, fy) not in coords):
                        start = (fx, fy, d)
                        break
                if start:
                    break
            if start is None:
                continue
            events.append(('step', vid, int(str(c.get('var_value', '0')), 0), c['x'], c['y']) + start + (c['script'], c.get('elevation', 0)))
        sp = os.path.join(repo, 'data/maps', name, 'scripts.inc')
        text = open(sp).read() if os.path.exists(sp) else ''
        landing = None
        for wv in m.get('warp_events') or []:
            for dx, dy in ((0, 1), (0, -1), (1, 0), (-1, 0)):
                if free(wv['x'] + dx, wv['y'] + dy):
                    landing = (wv['x'] + dx, wv['y'] + dy)
                    break
            if landing:
                break
        for v, val, script in re.findall(r'map_script_2 (\w+), (\w+), (\w+)', text):
            vid = var_id(v)
            if vid is None or landing is None:
                continue
            try:
                val = int(val, 0)
            except ValueError:
                continue
            events.append(('frame', vid, val, landing[0], landing[1], landing[0], landing[1], 'down', script, 0))
        if not events:
            continue
        lines = ['pressuntil A field 5000', 'wait 400', 'mp teach 0 44', 'mp teach 1 44', 'mp battlestyle 1']   # (the start's achievements toast takes the first input)
        for kind, vid, val, x, y, fx, fy, d, script, elev in events:
            note = '%s_%s_%d,%d_0x%X=%d' % (kind, script, x, y, vid, val)
            if kind == 'step':
                # the player on the trigger's cell, facing the way they'd have come; the trigger run as a step would
                lines += ['mp warp %d,%d,%d,%d' % (g, n, x, y), 'wait 40']
                if vid:
                    lines.append('mp setvar 0x%X %d' % (vid, val))
                lines += ['mp face %s' % d, 'wait 4', 'mp sweepmark', 'mp steptrigger %d %d' % (x, y), 'wait 30']
            else:
                lines += ['mp setvar 0x%X %d' % (vid, val), 'mp sweepmark', 'mp warp %d,%d,%d,%d' % (g, n, fx, fy), 'wait 60']
            lines += ['mash A 40 8', 'pressuntil A field 7000', 'mp cards concede', 'mp ygo concede', 'mp tt concede',
                      'pressuntil A field 1500', 'pressuntil B field 600', 'press START 4', 'wait 30', 'press A 4',
                      'wait 60', 'pressuntil B field 600', 'pressuntil A field 3000', 'pressuntil B field 600', 'wait 20',
                      'mp ctx', 'mp getvar 0x%X' % vid, 'mp sweepnote %s' % note]
        lines += ['mp savesum', 'quit']
        open(os.path.join(out, 'scripts', name + '.txt'), 'w').write('\n'.join(lines) + '\n')
        region = 0 if g < 34 else (1 if g < 36 else 2)
        print(g, n, region, len(events), name)
PY
one() {   # one GROUP NUM REGION COUNT NAME
    local d="$D/run_$5"; mkdir -p "$d"
    local t=$(( 90 + $4 * 120 ))
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
    # per event: free after it? did a script run after the mark? did the variable change from the trigger value?
    python3 - "$d/log.txt" "$1.$2 $5" <<'PY'
import re, sys
log = open(sys.argv[1], errors='replace').read().split('\n')
free, ran, val = None, False, None
for l in log:
    if l.startswith('[test] sweepmark'):
        free, ran, val = None, False, None
    elif l.startswith('[test] ctx'):
        free = ' free 1' in l
    elif l.startswith('[test] var 0x'):
        m = re.search(r'= (\d+)', l)
        val = int(m.group(1)) if m else None
    elif l.startswith('[test] sweepnote'):
        note = l.split()[2]
        ran = int(l.split('starts ')[1]) > 0 if 'starts ' in l else True
        want = int(note.rsplit('=', 1)[1])
        if free is False:
            print('HELD %s: %s' % (sys.argv[2], note))
        if not ran:
            print('NOFIRE %s: %s' % (sys.argv[2], note))
        elif val is not None and val == want:
            print('STILL %s: %s' % (sys.argv[2], note))
PY
    local np=$(grep -o "special [A-Za-z0-9_]* not ported" "$d/log.txt" | sort -u | awk '{print $2}' | tr '\n' ' ')
    [ -n "$np" ] && echo "STUB $1.$2 $5: $np"
    if [ -n "$why" ]; then cp "$d/log.txt" "$D/$5.log"; echo "FAIL $1.$2 $5: $why"; fi
    [ -n "$KEEP" ] || rm -rf "$d"
}
export -f one; export D BIN REGION KEEP
total=$(wc -l < "$D/maps.txt")
events=$(awk '{s+=$4} END{print s}' "$D/maps.txt")
xargs -P "$JOBS" -L 1 bash -c 'one "$@"' _ < "$D/maps.txt" | tee "$D/report.txt"
nf=$(grep -c "^FAIL" "$D/report.txt"); nh=$(grep -c "^HELD" "$D/report.txt"); ns=$(grep -c "^STILL" "$D/report.txt")
nn=$(grep -c "^NOFIRE" "$D/report.txt")
echo "story sweep: $events triggers on $total maps; $nf maps failed, $nh held, $nn didn't fire, $ns left the variable (groups $G0-$G1)"
[ "$nf" = 0 ]
