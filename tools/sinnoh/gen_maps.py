#!/usr/bin/env python3
"""The SINNOH maps as the field engine's maps (docs/SINNOH_HANDOFF.md): map.json, scripts.inc, the layouts and their
places in map_groups.json / layouts.json. Nothing here comes from the ROM: the layouts' blocks are placeholders (every
cell blocked), and the game takes each cell's collision and behavior from the 3D pack when it loads the map
(src/sinnoh/s3d.c S3D_LayoutBlocks, from Platinum's terrain attributes by build_pack.py).

    python3 -I tools/sinnoh/gen_maps.py        (run from the repository; safe to run again)

Each map: (name, MAP id, matrix cell x, z of its top-left block, width and height in blocks, music, connections).
"""
import json
import os
import struct

MAPS = [
    ('Sinnoh_TwinleafTown', 'MAP_SINNOH_TWINLEAF_TOWN', 3, 27, 1, 1, 'MUS_LITTLEROOT', 'MAP_TYPE_TOWN',
     [('MAP_SINNOH_ROUTE201', 0, 'up')]),
    ('Sinnoh_Route201', 'MAP_SINNOH_ROUTE201', 3, 26, 2, 1, 'MUS_ROUTE101', 'MAP_TYPE_ROUTE',
     [('MAP_SINNOH_TWINLEAF_TOWN', 0, 'down')]),
]
GROUP = 'gMapGroup_Sinnoh'
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3


def main():
    groups = json.load(open('data/maps/map_groups.json'))
    layouts = json.load(open('data/layouts/layouts.json'))
    if GROUP not in groups:
        groups['group_order'].append(GROUP)
        groups[GROUP] = []
    for name, map_id, cx, cz, bw, bh, music, mtype, conns in MAPS:
        w, h = 32 * bw, 32 * bh
        layout_id = 'LAYOUT_' + map_id[4:]
        ldir = 'data/layouts/' + name
        os.makedirs(ldir, exist_ok=True)
        open(ldir + '/map.bin', 'wb').write(struct.pack('<H', BLOCKED) * (w * h))
        open(ldir + '/border.bin', 'wb').write(struct.pack('<H', BLOCKED) * 4)
        entry = {'id': layout_id, 'name': name + '_Layout', 'width': w, 'height': h,
                 'primary_tileset': 'gTileset_General', 'secondary_tileset': 'gTileset_Petalburg',
                 'border_filepath': ldir + '/border.bin', 'blockdata_filepath': ldir + '/map.bin'}
        layouts['layouts'] = [l for l in layouts['layouts'] if l.get('id') != layout_id] + [entry]
        mdir = 'data/maps/' + name
        os.makedirs(mdir, exist_ok=True)
        m = {'id': map_id, 'name': name, 'layout': layout_id, 'music': music,
             'region_map_section': 'MAPSEC_NONE', 'requires_flash': False, 'weather': 'WEATHER_SUNNY',
             'map_type': mtype, 'allow_cycling': True, 'allow_escaping': False, 'allow_running': True,
             'show_map_name': False, 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL',
             'connections': [{'map': c[0], 'offset': c[1], 'direction': c[2]} for c in conns],
             'object_events': [], 'warp_events': [], 'coord_events': [], 'bg_events': []}
        json.dump(m, open(mdir + '/map.json', 'w'), indent=2)
        open(mdir + '/map.json', 'a').write('\n')
        if not os.path.exists(mdir + '/scripts.inc'):
            open(mdir + '/scripts.inc', 'w').write('%s_MapScripts::\n\t.byte 0\n' % name)
        if name not in groups[GROUP]:
            groups[GROUP].append(name)
    json.dump(groups, open('data/maps/map_groups.json', 'w'), indent=2)
    open('data/maps/map_groups.json', 'a').write('\n')
    json.dump(layouts, open('data/layouts/layouts.json', 'w'), indent=2)
    open('data/layouts/layouts.json', 'a').write('\n')
    print('maps:', ', '.join(m[0] for m in MAPS))


if __name__ == '__main__':
    main()
