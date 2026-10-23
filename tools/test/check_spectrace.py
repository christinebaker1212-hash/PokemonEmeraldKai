#!/usr/bin/env python3
# usage: check_spectrace.py [MIN] DIR
# The SPECTATE panel test (MOEMON_SPECTRACE=1): every picture a viewer made of a friend's screen must be the one the
# friend sent, pixel for pixel (the same checksum for the same update), and each viewer must have checked at least
# MIN (default 30) pictures from each friend it showed.
import re, sys, glob, os
args = sys.argv[1:]
need = int(args.pop(0)) if len(args) > 1 else 30
d = args[0]
ids, sent, seen, sizes = {}, {}, {}, {}
for log in sorted(glob.glob(os.path.join(d, '[a-d].log'))):
    c = os.path.basename(log)[0]
    for line in open(log, errors='replace'):
        m = re.search(r'joined ".*" as player (\d)', line)
        if m:
            ids[c] = int(m.group(1))
        m = re.search(r'spectrace: send seq (\d+) sum ([0-9a-f]+)(?: bytes (\d+))?', line)
        if m:
            sent.setdefault(c, {})[int(m.group(1))] = m.group(2)
            if m.group(3):
                sizes.setdefault(c, []).append(int(m.group(3)))
        m = re.search(r'spectrace: player (\d) seq (\d+) sum ([0-9a-f]+)', line)
        if m:
            seen.setdefault(c, []).append((int(m.group(1)), int(m.group(2)), m.group(3)))
by_id = {v: k for k, v in ids.items()}
bad = 0
for viewer, rows in sorted(seen.items()):
    counts = {}
    for pid, seq, s in rows:
        src = by_id.get(pid)
        if src is None or seq not in sent.get(src, {}):
            print(f'{viewer}: player {pid} seq {seq}: the sender never logged it')
            bad += 1
            continue
        if sent[src][seq] != s:
            print(f'{viewer}: player {pid} seq {seq}: {s}, sent {sent[src][seq]}')
            bad += 1
        counts[pid] = counts.get(pid, 0) + 1
    for pid, n in sorted(counts.items()):
        print(f'{viewer} (player {ids.get(viewer)}): {n} pictures from player {pid} checked')
        if n < need:
            print(f'{viewer}: only {n} from player {pid} (want {need})')
            bad += 1
for c, v in sorted(sizes.items()):
    # (a sender's own upload: one copy when every friend watches, else one per viewer)
    print(f'{c} sent {len(v)} updates, {sum(v) / len(v):.0f} bytes on average, largest {max(v)}')
if not seen:
    print('no viewer logged anything')
    bad += 1
print('spectrace:', 'ok' if bad == 0 else f'{bad} problems')
sys.exit(1 if bad else 0)
