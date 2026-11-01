#!/usr/bin/env python3
"""SINNOH's heal locations: where a whiteout (or ESCAPE ROPE / TELEPORT) brings the player, one a MOéMON CENTER, in front
of its door in the town (as JOHTO's and KANTO's), appended to src/data/heal_locations.json after theirs; and each
CENTER's 1F sets it when entered (setrespawn in the map's own scripts, written here while the map has no other).

    python3 -I tools/sinnoh/gen_heal.py       (run from the repository, after gen_maps.py; safe to run again)
"""
import glob
import json
import os
import re

HEADER = '%s_MapScripts::\n\t.byte 0\n'
WITH = ('%s_MapScripts::\n\tmap_script MAP_SCRIPT_ON_TRANSITION, %s_OnTransition\n\t.byte 0\n\n'
        '%s_OnTransition:\n\tsetrespawn %s\n\tend\n')


def main():
    maps = {}
    for p in glob.glob('data/maps/Sinnoh_*/map.json'):
        d = json.load(open(p))
        maps[d['id']] = (os.path.basename(os.path.dirname(p)), d)
    found = []
    for mid, (name, d) in sorted(maps.items()):
        if not name.endswith('Pokecenter1f'):
            continue
        door = None
        for oid, (oname, od) in maps.items():
            if od['map_type'] in ('MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE'):
                for w in od['warp_events']:
                    if w['dest_map'] == mid:
                        door = (oid, w['x'], w['y'] + 1)
        if door is None:
            print('no door into %s' % name)
            continue
        heal = 'HEAL_LOCATION_' + door[0][len('MAP_'):]
        while heal in [f['id'] for f in found]:
            heal += '_2'      # (two CENTERs off one place: the MOéMON LEAGUE's)
        found.append({'id': heal, 'map': door[0], 'x': door[1], 'y': door[2]})
        path = 'data/maps/%s/scripts.inc' % name
        s = open(path, encoding='utf-8').read()
        if s.startswith(HEADER % name):
            s = WITH % (name, name, name, heal) + s[len(HEADER % name):]
        else:
            s = re.sub(r'(%s_OnTransition:\n\tsetrespawn )\w+' % name, lambda m: m.group(1) + heal, s)
        open(path, 'w', encoding='utf-8').write(s)
    p = 'src/data/heal_locations.json'
    data = json.load(open(p))
    data['heal_locations'] = [h for h in data['heal_locations'] if not h['map'].startswith('MAP_SINNOH_')] + found
    with open(p, 'w') as f:
        json.dump(data, f, indent=2)
        f.write('\n')
    print('%d SINNOH heal locations' % len(found))


if __name__ == '__main__':
    main()
