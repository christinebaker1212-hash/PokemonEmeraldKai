#!/usr/bin/env python3
"""Converts every game MIDI (sound/songs/midi, -E songs) with Moemon Music Studio and with
tools/mid2agb (-N: no pattern compression) and checks the song bytes are the same.
  python3 check_mid2agb.py STUDIO_BINARY [song.mid ...]"""
import os, re, subprocess, sys, tempfile
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
STUDIO = os.path.abspath(sys.argv[1])
S = tempfile.mkdtemp(prefix='mms_test_')
if not os.path.exists(ROOT + '/tools/mid2agb/mid2agb'):
    subprocess.run(['make', '-C', ROOT + '/tools/mid2agb'], check=True, stdout=subprocess.DEVNULL)
sym = {}
for line in open(ROOT + '/sound/MPlayDef.s'):
    m = re.match(r'\s*\.equ\s+(\w+)\s*,\s*([^/\s]+)', line)
    if m: sym[m.group(1)] = m.group(2)
def ev(expr, local):
    expr = expr.strip()
    def rep(mo):
        w = mo.group(0)
        if w in local: return '(' + str(local[w]) + ')'
        if w in sym: return '(' + str(ev(sym[w], local)) + ')'
        return w
    e = re.sub(r'[A-Za-z_]\w*', rep, expr).replace('/', '//')
    return int(eval(e))
def parse(path):
    local, tracks, cur = {}, [], None
    lines = open(path).read().split('\n')
    for line in lines:
        m = re.match(r'\s*\.equ\s+(\w+)\s*,\s*(.+)', line)
        if m:
            v = m.group(2).strip()
            v = re.sub(r'voicegroup\w*', '0', v).replace('reverb_set', '128')
            local[m.group(1)] = ev(v, local)
    # two passes for labels
    for pas in range(2):
        tracks, cur, off = [], None, 0
        for line in lines:
            s = line.split('@')[0].strip()
            m = re.match(r'^(\w+):$', s)
            if m:
                name = m.group(1)
                if re.match(r'.*_\d+$', name) and not re.match(r'.*_\d+_\d+$', name):
                    cur = []; tracks.append(cur)
                if cur is not None: local[name] = len(cur)
                continue
            if cur is None: continue
            m = re.match(r'^\.byte\s+(.*)$', s)
            if m:
                for a in m.group(1).split(','):
                    a = a.strip()
                    if a: cur.append(ev(a, local) & 0xFF)
                continue
            m = re.match(r'^ptrvalue\s+(\w+)$', s)
            if m:
                v = local.get(m.group(1), 0)
                cur += [v & 255, (v >> 8) & 255, (v >> 16) & 255, (v >> 24) & 255]
                continue
            if s.startswith('.align') or s.startswith('.end'):
                if tracks and cur is tracks[-1] and s.startswith('.align'): cur = None
    return tracks
cfg = {}
for line in open(ROOT + '/sound/songs/midi/midi.cfg'):
    m = re.match(r'\s*(\w+\.mid)\s*:\s*(.*)', line)
    if m: cfg[m.group(1)] = m.group(2).split()
names = sys.argv[2:] or sorted(cfg)
ok = bad = 0
for n in names:
    opts = cfg[n]
    vol = 127
    for o in opts:
        if o.upper().startswith('-V'): vol = int(o[2:])
    out = S + '/m2a.s'
    r = subprocess.run([ROOT + '/tools/mid2agb/mid2agb', ROOT + '/sound/songs/midi/' + n, out, '-N'] + [o for o in opts if o != '-N'], capture_output=True)
    if r.returncode: print('mid2agb failed', n, r.stderr); continue
    want = parse(out)
    got = subprocess.run([STUDIO, '--dump-exact', ROOT + '/sound/songs/midi/' + n, str(vol)], capture_output=True, text=True).stdout.split('\n')
    got = [[int(x, 16) for x in l.split()[1:]] for l in got if l.startswith('T')]
    if '-E' not in [o.upper() for o in opts]:
        continue
    if want == got: ok += 1
    else:
        bad += 1
        print('DIFF', n, len(want), len(got))
        for i, (a, b) in enumerate(zip(want, got)):
            if a != b:
                k = next((j for j in range(min(len(a), len(b))) if a[j] != b[j]), min(len(a), len(b)))
                print('  track', i + 1, 'at', k, 'want', a[max(0,k-6):k+10], 'got', b[max(0,k-6):k+10], len(a), len(b))
                break
print('same:', ok, 'different:', bad)
sys.exit(1 if bad or not ok else 0)
