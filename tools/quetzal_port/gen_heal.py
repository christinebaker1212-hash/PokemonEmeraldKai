#!/usr/bin/env python3
"""Johto / Kanto heal locations (where a whiteout or ESCAPE ROPE / TELEPORT / FLY brings you) into
src/data/heal_locations.json, after Kai's 22: Quetzal numbers them 23-57 the same way, so the scripts' setrespawn
values stay right.   python3 tools/quetzal_port/gen_heal.py"""
import json
import os
import re

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def const(name):
    return re.sub(r'(?<=[a-z0-9])([A-Z])', r'_\1', name).upper()


def main():
    rip = json.load(open(os.path.join(REPO, 'quetzal_rip/region_map/fly_heal.json')))['heal_locations']
    p = os.path.join(REPO, 'src/data/heal_locations.json')
    kai = json.load(open(p))
    kai['heal_locations'] = [h for h in kai['heal_locations'] if not h['map'].startswith(('MAP_JOHTO_', 'MAP_KANTO_'))]
    assert len(kai['heal_locations']) == 22, 'Kai has %d heal locations: Quetzal numbers its own from 23' % len(kai['heal_locations'])
    hs = sorted(rip['johto'] + rip['kanto'], key=lambda h: h['heal_location'])
    for n, h in enumerate(hs):
        assert h['heal_location'] == 23 + n
        kai['heal_locations'].append({'id': 'HEAL_LOCATION_' + const(h['map']), 'map': 'MAP_' + const(h['map']), 'x': h['x'], 'y': h['y']})
    with open(p, 'w') as f:
        json.dump(kai, f, indent=2)
        f.write('\n')
    print('%d heal locations (23-%d)' % (len(hs), 22 + len(hs)))


if __name__ == '__main__':
    main()
