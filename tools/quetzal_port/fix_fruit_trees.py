#!/usr/bin/env python3
"""JOHTO's fruit trees as Crystal's, in Quetzal's berry trees' places (src/mp/mp_apricorn.c holds the rest).

    python3 tools/quetzal_port/fix_fruit_trees.py      (safe to run again)

Quetzal put a BERRY tree of the game's own (Emerald's growing BERRIES) where Crystal has its fruit trees, drawn
as a tree, on tree numbers 90-109, ten of them the GARDEN farm's too (100-119). Now:
- where Crystal's tree bears APRICORNs (AZALEA TOWN, ROUTE 37, ROUTE 42) it is an APRICORN tree
  (Johto_EventScript_FruitTree, data/scripts/mp_region.inc); ROUTE 37 and ROUTE 42 have Crystal's three in a row
  (the two more beside Quetzal's one); drawn as Crystal's fruit tree, the same all day (OBJ_EVENT_GFX_FRUIT_TREE)
- everywhere else it is a BERRY patch (the game's own soil, 1x1) on a tree number of JOHTO's own, starting with
  Crystal's BERRY there (mp_apricorn.c sBerryPatches)
"""
import json
import os

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SCRIPT = 'Quetzal_EventScript_836266F'
PATCHES = {   # (map, x, y): tree number (mp_apricorn.c sBerryPatches)
    ('Route31', 16, 11): 90, ('VioletCity', 18, 49): 91, ('Route47', 51, 8): 92, ('Route26South', 19, 9): 93,
    ('Route33', 30, 26): 94, ('Route38', 21, 24): 96, ('Route39', 12, 27): 97, ('Route44', 7, 10): 98,
    ('Route45', 24, 87): 99, ('Route43', 3, 27): 120, ('Route43', 4, 25): 121, ('Route29', 15, 11): 122,
    ('Route30', 23, 38): 123, ('Route35', 11, 25): 124, ('Route36', 20, 13): 125, ('Route46', 9, 11): 126,
    ('Route30', 30, 4): 127,
}
APRICORNS = {('AzaleaTown', 12, 7): [], ('Route37', 33, 24): [32, 34], ('Route42', 40, 19): [39, 41]}


def main():
    maps = {m for m, _, _ in list(PATCHES) + list(APRICORNS)}
    for m in sorted(maps):
        path = os.path.join(REPO, 'data/maps/Johto_%s/map.json' % m)
        j = json.load(open(path))
        objs = j['object_events']
        for o in list(objs):
            key = (m, o['x'], o['y'])
            if key in PATCHES and o['script'] in (SCRIPT, 'BerryTreeScript'):
                o.update({'graphics_id': 'OBJ_EVENT_GFX_BERRY_TREE', 'movement_type': 'MOVEMENT_TYPE_BERRY_TREE_GROWTH',
                          'trainer_sight_or_berry_tree_id': str(PATCHES[key]), 'script': 'BerryTreeScript'})
            elif key in APRICORNS and o['script'] in (SCRIPT, 'Johto_EventScript_FruitTree'):
                o.update({'graphics_id': 'OBJ_EVENT_GFX_FRUIT_TREE', 'movement_type': 'MOVEMENT_TYPE_NONE',
                          'trainer_sight_or_berry_tree_id': '0', 'script': 'Johto_EventScript_FruitTree'})
                for x in APRICORNS[key]:
                    if not any(p['x'] == x and p['y'] == o['y'] for p in objs):
                        objs.append(dict(o, x=x))
            elif o['script'] == 'Johto_EventScript_FruitTree':   # (the two added beside it, from an earlier run)
                o.update({'graphics_id': 'OBJ_EVENT_GFX_FRUIT_TREE', 'movement_type': 'MOVEMENT_TYPE_NONE',
                          'trainer_sight_or_berry_tree_id': '0'})
        left = [o for o in objs if o['script'] == SCRIPT]
        if left:
            raise SystemExit('%s: a tree not placed: %s' % (m, [(o['x'], o['y']) for o in left]))
        with open(path, 'w') as f:
            json.dump(j, f, indent=2)
            f.write('\n')
        print(m)


if __name__ == '__main__':
    main()
