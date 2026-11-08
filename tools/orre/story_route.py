#!/usr/bin/env python3
"""ORRE's story as a route (docs/ORRE_HANDOFF.md "The story runs"): Colosseum's story flags in their order (the flag
table's linked list, as the game's story jump walks it: `mp orreorder` prints it from the game), each with the
functions of the room script banks that set it (std078) and how each such function is reached: a person's talk or
move script, a check or touch spot (src/data/orre_room_checks.h), the room's own set-up (+ 0x2C / + 0x30), main loop
(+ 0x34) or leave script (+ 0x38), or a call from another function (followed back to one of those). For finding
what a playthrough must do next, and what the story test drives.

    python3 -I tools/orre/story_route.py ORDER.txt [UNPACKED]   (ORDER.txt: the game's `mp orreorder` lines)
"""
import collections
import glob
import os
import re
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import colo_data  # noqa: E402


def banks(unpacked):
    """{file: {func: (calls, sets)}}, every room's bank by gsvm.py"""
    out = {}
    for f in sorted(glob.glob(os.path.join(unpacked, '*', '*.scd'))):
        tag = os.path.basename(f)[:-4]
        txt = subprocess.run([sys.executable, '-I', os.path.join(HERE, 'gsvm.py'), f], capture_output=True,
                             text=True).stdout
        funcs, fn = {}, None
        for line in txt.splitlines():
            m = re.match(r'-- function (\d+)', line)
            if m:
                fn = int(m.group(1))
                funcs[fn] = (set(), set())
                continue
            if fn is None:
                continue
            m = re.search(r'call func (\d+)', line)
            if m:
                funcs[fn][0].add(int(m.group(1)))
            m = re.search(r'std078_flagSetFalse\((\d+)\)', line)
            if m:
                funcs[fn][1].add(int(m.group(1)))
        out[tag] = funcs
    return out


def entries(unpacked):
    """{(file, func): [how it's reached]}: people, spots, the room's own scripts"""
    data = colo_data._data(unpacked)
    rooms = {f['room']: f for f in colo_data.floors(unpacked)}
    out = collections.defaultdict(list)
    for room, f in rooms.items():
        if not f['file']:
            continue
        rec = colo_data.FLOOR_TABLE + f['index'] * colo_data.FLOOR_SIZE
        for off, what in ((0x2C, 'set-up'), (0x30, 'set-up 2'), (0x34, 'main loop'), (0x38, 'on leaving')):
            sid = struct.unpack_from('>I', data, rec + off)[0]
            if sid >> 16 == 0x100:
                out[(f['file'], sid & 0xFFFF)].append(what)
    people = open(os.path.join(ROOT, 'src', 'data', 'orre_room_people.h')).read()
    for m in re.finditer(r'\{(\d+), (\d+), 0x\w+, 0x\w+, -?\d+, 0x\w+, \d+, 0x(\w+), 0x(\w+), [^}]*\},\s+// (\w+)', people):
        for sid, what in ((int(m.group(3), 16), 'moves'), (int(m.group(4), 16), 'talk')):
            if sid >> 16 == 0x100:
                out[(m.group(5), sid & 0xFFFF)].append('person %s %s' % (m.group(2), what))
    checks = open(os.path.join(ROOT, 'src', 'data', 'orre_room_checks.h')).read()
    kind = 'check'
    for line in checks.splitlines():
        if 'sOrreRoomTouches' in line:
            kind = 'touch'
        m = re.match(r'\s+\{(\d+), (\d+), 0x(\w+), .*// (\w+)', line)
        if m and int(m.group(3), 16) >> 16 == 0x100:
            out[(m.group(4), int(m.group(3), 16) & 0xFFFF)].append('%s spot %s' % (kind, m.group(2)))
    return out


def main():
    order = [l.split() for l in open(sys.argv[1]) if 'orre order' in l]
    unpacked = sys.argv[2] if len(sys.argv) > 2 else os.path.expanduser('~/.cache/moemon/orre/unpacked')
    b, ent = banks(unpacked), entries(unpacked)
    callers = collections.defaultdict(set)
    setters = collections.defaultdict(set)
    for tag, funcs in b.items():
        for fn, (calls, sets) in funcs.items():
            for c in calls:
                callers[(tag, c)].add((tag, fn))
            for fl in sets:
                setters[fl].add((tag, fn))

    def reach(k, seen=()):
        if k in ent:
            return ['%s:%d %s' % (k[0], k[1], '/'.join(ent[k]))]
        out = []
        for c in sorted(callers.get(k, ())):
            if c not in seen:
                out += reach(c, seen + (k,))
        return out or ['%s:%d (not reached)' % k]

    for l in order:
        pos, flag = int(l[3]), int(l[5])
        if flag not in setters:
            continue
        ways = []
        for k in sorted(setters[flag]):
            ways += reach(k)
        print('%4d flag %4d  %s' % (pos, flag, '; '.join(sorted(set(ways))[:4])))


if __name__ == '__main__':
    main()
