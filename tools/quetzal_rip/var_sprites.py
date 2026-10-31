#!/usr/bin/env python3
"""Overworld sprites that Quetzal's scripts put in a graphics variable, added to the rip's sprites.

    python3 tools/quetzal_rip/var_sprites.py QUETZAL_ROM

sprites.py rips the graphics ids map objects name. Objects drawn through a variable (OBJ_EVENT_GFX_QZ_240-255)
show what scripts store there: `setvar VAR_QZ_0x4010..0x401F` (VAR_OBJ_GFX_ID_0..F) and, on VIRIDIAN CITY,
BIRTH ISLAND and FUCHSIA CITY, VAR_QZ_0x513E (Quetzal's GetObjectEventGraphicsInfo reads it for variable 0 there).
This finds those values in the rip's scripts and adds their sprites to quetzal_rip/object_events (pics, palettes,
object_events.json rows). Then run tools/quetzal_port/gen_objects.py.
"""
import glob
import json
import os
import re
import shutil
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from qrom import REPO, Rom          # noqa: E402
from sprites import rip_sprites    # noqa: E402

RIP = os.path.join(REPO, 'quetzal_rip')
VAR_RE = re.compile(r'setvar\s+VAR_QZ_0x(401[0-9A-F]|513E),\s*(0x[0-9A-Fa-f]+|\d+)')


def var_ids():
    ids = set()
    for p in glob.glob(os.path.join(RIP, '**', '*.inc'), recursive=True):
        for m in VAR_RE.finditer(open(p, encoding='utf-8').read()):
            ids.add(int(m.group(2), 0))
    return ids


def main():
    rom = Rom(sys.argv[1])
    path = os.path.join(RIP, 'object_events', 'object_events.json')
    rows = json.load(open(path))
    have = {r['id'] for r in rows}
    want = sorted(i for i in var_ids() if i not in have and not 0xF0 <= i <= 0xFF)
    if not want:
        print('nothing to add')
        return
    tmp = tempfile.mkdtemp()
    target = {'name': 'graphics variables', 'events': {'objects': [{'graphics_id': g} for g in want]}}
    new = rip_sprites(rom, [target], tmp, [('kai', REPO)])
    for sub in ('pics', 'palettes'):
        for f in glob.glob(os.path.join(tmp, 'object_events', sub, '*')):
            dst = os.path.join(RIP, 'object_events', sub, os.path.basename(f))
            if not os.path.exists(dst):
                shutil.copy(f, dst)
    for r in new:
        r['used_by'] = ['(a script\'s graphics variable)']
    rows = sorted(rows + new, key=lambda r: r['id'])
    with open(path, 'w') as fh:
        json.dump(rows, fh, indent=1)
        fh.write('\n')
    shutil.rmtree(tmp)
    print('added %d sprites: %s' % (len(new), want))


if __name__ == '__main__':
    main()
