#!/usr/bin/env python3
"""What SINNOH's words (tools/sinnoh/words.py) still lack: each map's people and signs that are shown and have no
script yet, with what Platinum's script entry is called (a hint at what the spot is). Trainers, item balls, story
people (hidden until their scenes) and things that aren't people are left out: they have their own scripts.

    python3 -I tools/sinnoh/todo.py POKEPLATINUM_DIR [outdoor|indoor|all] [MAP...]
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from gen_maps import places  # noqa: E402
from places import events  # noqa: E402
import sprites  # noqa: E402
import who  # noqa: E402
from words import WORDS  # noqa: E402
from words_indoor import kind  # noqa: E402


def todo(root, p):
    name = p['name']
    m = json.load(open('data/maps/%s/map.json' % name))
    ev = events(root, p['place'])
    entries = who.entries(root, p['place']['header'][len('MAP_HEADER_'):].lower())

    def what(n):
        return entries[n - 1] if isinstance(n, int) and 1 <= n <= len(entries) else ''
    have = WORDS.get(name, {})
    out = []
    for i, o in enumerate(ev['object_events']):
        sign = sprites.sign(o['graphics_id'])
        key = ('Sign%d' if sign else 'Person%d') % (i + 1)
        mo = m['object_events'][i]
        hide = o.get('hidden_flag')
        if name.startswith('Sinnoh_HearthomeCityDp') or 'OnFrame' in what(o['script']):
            continue          # (Diamond and Pearl's version of a map, not reached; a map's own frame script)
        if key in have or not what(o['script']) or o['graphics_id'].endswith(('POKECENTER_NURSE', 'CASHIER_F', 'CASHIER_M')):
            continue
        if kind(name, what(o['script']).split('_', 1)[-1]):
            continue
        if isinstance(hide, str) and hide.startswith(('FLAG_', 'MAP_HEADER_')):
            continue          # (a story person, or a neighbor's)
        if not sign and (mo['flag'] == '0x3FFF' or mo['trainer_type'] != 'TRAINER_TYPE_NONE'
                         or mo['script'] != '0x0'):
            continue
        out.append('%s %s  %s' % (key, o['graphics_id'][14:], what(o['script'])))
    for i, b in enumerate(ev['bg_events']):
        if b.get('type') == 0 and 'Bg%d' % (i + 1) not in have and what(b['script']) and 'OnFrame' not in what(b['script']) \
                and not kind(name, what(b['script']).split('_', 1)[-1]):
            out.append('Bg%d  %s' % (i + 1, what(b['script'])))
    return out


def main():
    root = sys.argv[1]
    kind = sys.argv[2] if len(sys.argv) > 2 else 'all'
    only = set(sys.argv[3:])
    total = 0
    for p in places(open_source(root), root):
        if only and p['name'] not in only:
            continue
        if (kind == 'outdoor' and not p['outdoor']) or (kind == 'indoor' and p['outdoor']):
            continue
        out = todo(root, p)
        if out:
            total += len(out)
            print('== %s' % p['name'])
            print('\n'.join('  ' + x for x in out))
    print('%d to write' % total)


if __name__ == '__main__':
    main()
