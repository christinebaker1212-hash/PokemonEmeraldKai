#!/usr/bin/env python3
"""Crystal's own items in JOHTO's item balls (docs/REGIONS_AUDIT.md case 91; Quetzal changed them).

    python3 tools/quetzal_port/fix_item_balls.py

For each map, the balls (STD_FIND_ITEM scripts, in file order) take Crystal's non-TM items in order; a ball left
over is taken out (kai_gone.py: FLAG_KAI_GONE). Crystal's TM balls are case 104's (the TM work), not here. A
ball Crystal has and Quetzal lacks gets an object where Crystal has it (NEW). Running it again changes nothing.
"""
import json
import os
import re
import subprocess

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))

CRYSTAL = {
    'DarkCaveVioletEntrance': ['ITEM_POTION', 'ITEM_FULL_HEAL', 'ITEM_HYPER_POTION', 'ITEM_DIRE_HIT'],
    'DarkCaveBlackthornEntrance': ['ITEM_REVIVE'],
    'IlexForest': ['ITEM_REVIVE', 'ITEM_X_ATTACK', 'ITEM_ANTIDOTE', 'ITEM_ETHER'],
    'Route27': ['ITEM_RARE_CANDY'],
    'Route31': ['ITEM_POTION', 'ITEM_POKE_BALL'],
    'Route32': ['ITEM_GREAT_BALL', 'ITEM_REPEL'],
    'Route46': ['ITEM_X_SPEED'],
    'SlowpokeWellB1F': ['ITEM_SUPER_POTION'],
    'SlowpokeWellB2F': [],                     # its KING'S ROCK is the man's again (case 92)
    'SproutTower2F': ['ITEM_X_ACCURACY'],
    'UnionCaveB1F': ['ITEM_X_DEFEND'],
    'UnionCaveB2F': ['ITEM_ELIXIR', 'ITEM_HYPER_POTION'],
    'VioletCity': ['ITEM_PP_UP', 'ITEM_RARE_CANDY'],
}
# Crystal's balls Quetzal's maps have no ball for: (map, x, y, item), at the matching spot of Quetzal's redrawn map
# (tools/quetzal_port/map_ascii.py)
NEW = [
    ('Route30', 20, 36, 'ITEM_ANTIDOTE'),     # (Crystal's 8,35: the left dead end below the ROUTE 30 house)
    ('Route26', 15, 15, 'ITEM_MAX_ELIXIR'),   # (Crystal's 9,15 from the VICTORY ROAD gate)
]


def balls(text):
    """[(local id, label, item)] of the map's item-ball scripts in file order"""
    out = []
    for m in re.finditer(r'@ \w+ object (\d+) \(local id (\d+)[^\n]*\n(\w+)::\n\tsetorcopyvar VAR_0x8000, (ITEM_\w+)'
                         r'(?:[^\n]*)\n\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_FIND_ITEM', text):
        out.append((int(m.group(2)), m.group(3), m.group(4)))
    return out


def main():
    for name, items in CRYSTAL.items():
        path = os.path.join(REPO, 'data/maps/Johto_%s/scripts.inc' % name)
        mj = json.load(open(os.path.join(REPO, 'data/maps/Johto_%s/map.json' % name)))
        text = open(path).read()
        found = [b for b in balls(text) if mj['object_events'][b[0] - 1]['flag'] != 'FLAG_KAI_GONE']
        gone = []
        for k, (lid, label, item) in enumerate(found):
            if k < len(items):
                text = re.sub(r'(%s::\n\tsetorcopyvar VAR_0x8000, )ITEM_\w+[^\n]*' % label,
                              r'\g<1>%s   @ Moemon Kai: Crystal\'s (fix_item_balls.py)' % items[k], text)
            else:
                gone.append('id:%d' % lid)
        open(path, 'w').write(text)
        if gone:
            subprocess.check_call(['python3', os.path.join(HERE, 'kai_gone.py'), 'Johto_' + name] + gone)
        print(name, items, 'gone', gone)
    for name, x, y, item in NEW:
        mpath = os.path.join(REPO, 'data/maps/Johto_%s/map.json' % name)
        mj = json.load(open(mpath))
        label = 'Johto_%s_EventScript_CrystalBall_%d_%d' % (name, x, y)
        if any(o['script'] == label for o in mj['object_events']):
            continue
        # a free item-ball flag: the region's own NEW balls (kai_regions.h FLAG_JOHTO_ITEM_*)
        flag = 'FLAG_JOHTO_ITEM_%s' % name.upper()
        mj['object_events'].append({'graphics_id': 'OBJ_EVENT_GFX_QZ_15006', 'x': x, 'y': y, 'elevation': 0,
                                    'movement_type': 'MOVEMENT_TYPE_NONE', 'movement_range_x': 0,
                                    'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                                    'trainer_sight_or_berry_tree_id': '0', 'script': label, 'flag': flag})
        json.dump(mj, open(mpath, 'w'), indent=2)
        open(mpath, 'a').write('\n')
        path = os.path.join(REPO, 'data/maps/Johto_%s/scripts.inc' % name)
        text = open(path).read()
        text += ('\n@ Moemon Kai: Crystal\'s item ball (fix_item_balls.py)\n%s::\n\tsetorcopyvar VAR_0x8000, %s\n'
                 '\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_FIND_ITEM\n\tend\n' % (label, item))
        open(path, 'w').write(text)
        print(name, 'new ball', item, 'at', x, y)


if __name__ == '__main__':
    main()
