#!/usr/bin/env python3
"""Moemon Maker, M4b: the game's own scripts as Maker events (docs/MAKER_HANDOFF.md "M4b").

Each block of a scripts file (a label and its lines, up to the next label) becomes an event of `raw` lines: the game's
own macros, assembled at run time by src/maker/maker_raw.c into the same bytes. Labels outside the block stay
pointers to the game's (its texts, movements and other scripts: the same addresses).

  decompile.py --proof OUT.txt     every block of the game's scripts (data/event_scripts*.s and what they include) as
                                   "@ LABEL" and its raw lines, for the in-game proof ("mp makerproof OUT.txt":
                                   tools/test/maker_decompile.sh), which compiles each and walks it beside the built
                                   script
  decompile.py MAP_JSON_DIR        a map's scripts as events (stdout), e.g. data/maps/LittlerootTown
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
LABEL = re.compile(r'^([A-Za-z_]\w*)(::?)\s*(.*)$')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8', errors='replace') as f:
        return f.read()


def clean(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    out = []
    for line in text.splitlines():
        q, i = False, 0
        while i < len(line):
            if line[i] == '"':
                q = not q
            elif (line[i] == '@' or line.startswith('//', i)) and not q:
                line = line[:i]
                break
            i += 1
        out.append(line.strip())
    return out


def blocks(path):
    """(label, global?, [lines]) for each label of a scripts file"""
    out, cur = [], None
    for line in clean(read(path)):
        m = LABEL.match(line)
        if m and not line.startswith('.'):
            cur = (m.group(1), m.group(2) == '::', [])
            out.append(cur)
            if m.group(3):
                cur[2].append(m.group(3))
            continue
        if cur is not None and line:
            cur[2].append(line)
    return out


def script_files():
    """every scripts file the game assembles: data/event_scripts*.s (HOENN, KANTO / JOHTO, SINNOH, ORRE, the
    STADIUM) and what they include"""
    seen, out = set(), []

    def walk(path):
        if path in seen or not os.path.exists(os.path.join(ROOT, path)):
            return
        seen.add(path)
        for line in read(path).splitlines():
            m = re.match(r'\s*\.include\s+"([^"]+)"', line)
            if m and m.group(1).endswith('.inc') and not m.group(1).startswith(('asm/', 'constants/')) \
                    and 'maker_globals' not in m.group(1):
                out.append(m.group(1))
                walk(m.group(1))
    for f in sorted(os.listdir(os.path.join(ROOT, 'data'))):
        if f.startswith('event_scripts') and f.endswith('.s'):
            walk('data/' + f)
    return out


def usable(lines):
    """a block the raw lines can say: macro calls and plain values (texts are data the game keeps: never raw)"""
    for l in lines:
        if l.startswith('.string') or l.startswith('.include') or l.startswith('.align') or l.startswith('.balign') or l.startswith('.ifdef') \
                or l.startswith('.if') or l.startswith('.endif') or l.startswith('.else') or l.startswith('.set') \
                or l.startswith('.macro') or l.startswith('.incbin') or l.startswith('#'):
            return False
    return bool(lines)


def proof(out_path):
    n = skipped = 0
    with open(out_path, 'w') as f:
        for path in script_files():
            for name, glob, lines in blocks(path):
                if not glob or re.match(r'g[A-Z]', name):   # (gName: a table the C code reads, never a script)
                    continue
                if not usable(lines):
                    skipped += 1
                    continue
                f.write('@ %s\n' % name)
                for l in lines:
                    f.write('raw %s\n' % l)
                n += 1
    print('%d blocks (%d left out: texts and assembler-only data)' % (n, skipped), file=sys.stderr)


def map_events(map_dir):
    import json
    j = json.loads(read(os.path.join(map_dir, 'map.json')))
    bl = {name: lines for name, glob, lines in blocks(os.path.join(map_dir, 'scripts.inc'))}
    names = set()
    for o in j.get('object_events', []) + j.get('bg_events', []) + j.get('coord_events', []):
        s = o.get('script')
        if s and s in bl:
            names.add(s)
    for name in sorted(names):
        print('event %s' % name)
        print('  page')
        print('    label %s' % name)
        for l in bl[name]:
            print('    raw %s' % l)


def main():
    if len(sys.argv) == 3 and sys.argv[1] == '--proof':
        proof(sys.argv[2])
    elif len(sys.argv) == 2:
        map_events(sys.argv[1])
    else:
        print(__doc__, file=sys.stderr)
        sys.exit(2)


if __name__ == '__main__':
    main()
