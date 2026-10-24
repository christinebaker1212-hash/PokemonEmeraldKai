#!/usr/bin/env python3
"""Movement parity (MOEMON_MOVETRACE, mp_follower.c MoveTrace): every player and follower takes the same steps and
faces the same ways on every screen.

Usage: check_movetrace.py RESULT_DIR
Reads a.log (player 0) and b.log (player 1). For each player and its follower, the tiles and facings logged by its
own game must come out in the same order in the other game, from where the other game first shows it to the end
(at the end of the test both have settled, so the last ones match too)."""
import sys, os, re

d = sys.argv[1]
TAIL = 3

def read(name):
    seqs = {}
    for line in open(os.path.join(d, name), errors='replace'):
        m = re.search(r'\[test\] mv ([PFG])(\d)(:[0-9a-f]+)? (-?\d+) (-?\d+) (\d+)', line)
        if m:
            seqs.setdefault(m.group(1) + m.group(2) + (m.group(3) or ''), []).append(
                (int(m.group(4)), int(m.group(5)), int(m.group(6))))
    return seqs

a, b = read('a.log'), read('b.log')
bad = False
checks = [('P0', a, b), ('F0', a, b), ('P1', b, a), ('F1', b, a)]
# Garden Pokémon (G<owner>:<key>): each owner's own against the other game's view of them
checks += [(k, a, b) for k in sorted(a) if k.startswith('G0:')] + [(k, b, a) for k in sorted(b) if k.startswith('G1:')]
for ent, own, view in checks:
    src, dst = own.get(ent, []), view.get(ent, [])
    if not src or not dst:
        print('%s: own %d steps, seen %d steps' % (ent, len(src), len(dst)))
        if ent[0] in 'PG':
            bad = True
        continue
    # line up: the first state the viewer shows, wherever it is in the owner's sequence (it may come up more than
    # once: any start that makes the rest the same will do; else the one that matches longest is reported)
    starts = [i for i in range(len(src)) if src[i] == dst[0]]
    if not starts:
        print('%s: first seen %s never happened in its own game' % (ent, dst[0]))
        bad = True
        continue
    # the same from there on; the viewer may trail by a few steps at the very end (the games quit while the
    # owner's last steps are still on their way: a quarter second of lag)
    ok = [i for i in starts if src[i:i + len(dst)] == dst and len(src) - i - len(dst) <= TAIL]
    if ok:
        trail = len(src) - ok[0] - len(dst)
        print('%s: %d steps, the same%s' % (ent, len(dst), ' (%d still on their way at the end)' % trail if trail else ''))
        continue
    def run(i):
        k = 0
        while k < len(dst) and i + k < len(src) and src[i + k] == dst[k]:
            k += 1
        return k
    start = max(starts, key=run)
    want = src[start:]
    bad = True
    for k in range(max(len(dst), len(want))):
        w = want[k] if k < len(want) else None
        g = dst[k] if k < len(dst) else None
        if w != g:
            print('%s: differ at step %d: own %s, seen %s' % (ent, k, w, g))
            print('   own  ...%s' % want[max(0, k - 3):k + 4])
            print('   seen ...%s' % dst[max(0, k - 3):k + 4])
            break
sys.exit(1 if bad else 0)
