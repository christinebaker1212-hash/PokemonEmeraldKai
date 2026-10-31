#!/usr/bin/env python3
"""Who and what is on a SINNOH map, for writing its scripts (docs/SINNOH_HANDOFF.md step 4): each person, sign and trigger
with the number its script label takes (<map>_EventScript_Person<n> / _Sign<n> / _Bg<n> / _Trigger<n>: gen_maps.py),
its spot, sprite and what Platinum's script entry is called (a hint at what it is: a TV, a nurse...). Only names are
read; the words are written for this game.

    python3 -I tools/sinnoh/who.py POKEPLATINUM_DIR PLACE...     (PLACE: twinleaf_town, sandgem_town_mart...)
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import sprites  # noqa: E402


def entries(root, place):
    path = os.path.join(root, 'res/field/scripts/scripts_%s.s' % place)
    if not os.path.exists(path):
        return []
    return re.findall(r'^\s*ScriptEntry (\w+)', open(path, encoding='utf-8', errors='replace').read(), re.M)


def main():
    root = sys.argv[1]
    for place in sys.argv[2:]:
        d = json.load(open(os.path.join(root, 'res/field/events/events_%s.json' % place)))
        names = entries(root, place)

        def what(n):
            return names[n - 1] if isinstance(n, int) and 1 <= n <= len(names) else ('common %s' % n)
        print('== %s' % place)
        for i, o in enumerate(d['object_events']):
            kind = 'Sign' if sprites.sign(o['graphics_id']) else 'Person'
            hide = o.get('hidden_flag')
            print('  %s%-3d %-28s at %d,%d  %s%s' % (kind, i + 1, o['graphics_id'][14:], o['x'], o['z'], what(o['script']),
                                                  ('  (shown by the story: %s)' % hide) if isinstance(hide, str) else ''))
        for i, b in enumerate(d['bg_events']):
            if b.get('type') == 0:
                print('  Bg%-7d at %d,%d  %s' % (i + 1, b['x'], b['z'], what(b['script'])))
        for i, c in enumerate(d['coord_events']):
            print('  Trigger%-2d at %d,%d %dx%d  %s = %s  %s' % (i + 1, c['x'], c['z'], c.get('width', 1), c.get('length', 1),
                                                          c.get('var'), c.get('value'), what(c['script'])))


if __name__ == '__main__':
    main()
