#!/usr/bin/env python3
"""The SINNOH maps as the field engine's maps (docs/SINNOH_HANDOFF.md): every outdoor place on Platinum's map matrix 0
(its towns, cities, routes and lakefronts) as a map of group 40, with map.json, scripts.inc, a layout and their places
in map_groups.json / layouts.json, and the 3D engine's table of where each one lies on the matrix
(src/sinnoh/s3d_maps.h). Only the places' names, rectangles, music, type and weather are read from pret/pokeplatinum
(its map matrix and map header table); nothing of Platinum's art is written here. The layouts' blocks are placeholders
(every cell blocked): the game takes each cell's collision and behavior from the 3D pack when it loads the map
(src/sinnoh/s3d.c S3D_LayoutBlocks, from Platinum's terrain attributes by build_pack.py).

    python3 -I tools/sinnoh/gen_maps.py POKEPLATINUM_DIR      (run from the repository; safe to run again)

A place covering an L of matrix cells (ETERNA CITY, OREBURGH CITY, ROUTE 209, ROUTE 224) becomes the rectangle around
it; the corner left over is the matrix's filler (sea, mountains), blocked. Maps whose rectangles touch are connected.
Map numbers stay as they were (TWINLEAF TOWN 0, ROUTE 201 1); new places are added after the last.
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source, NONE  # noqa: E402

GROUP = 'gMapGroup_Sinnoh'
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3
FILLER = 'MAP_HEADER_EVERYWHERE'

# Platinum's day songs -> SINNOH's song slots (tools/kaisong/sinnoh_songs.py; S3D_MapMusic plays <slot>_night by night)
MUSIC = {
    'SEQ_TOWN01_D_sseq': 'twinleaf', 'SEQ_TOWN02_D_sseq': 'sandgem', 'SEQ_TOWN03_D_sseq': 'floaroma',
    'SEQ_TOWN06_D_sseq': 'route225', 'SEQ_TOWN07_D_sseq': 'valor_lakefront',
    'SEQ_CITY01_D_sseq': 'jubilife', 'SEQ_CITY02_D_sseq': 'canalave', 'SEQ_CITY03_D_sseq': 'oreburgh',
    'SEQ_CITY04_D_sseq': 'eterna', 'SEQ_CITY05_D_sseq': 'hearthome', 'SEQ_CITY06_D_sseq': 'solaceon',
    'SEQ_CITY07_D_sseq': 'veilstone', 'SEQ_CITY08_D_sseq': 'sunyshore', 'SEQ_CITY09_D_sseq': 'snowpoint',
    'SEQ_CITY10_D_sseq': 'league', 'SEQ_CITY11_D_sseq': 'fight_area',
    'SEQ_ROAD_A_D_sseq': 'route201', 'SEQ_ROAD_B_D_sseq': 'route203', 'SEQ_ROAD_C_D_sseq': 'route205',
    'SEQ_ROAD_D_D_sseq': 'route206', 'SEQ_ROAD_E_D_sseq': 'route209', 'SEQ_ROAD_F_D_sseq': 'route210',
    'SEQ_ROAD_SNOW_D_sseq': 'route216', 'SEQ_ROAD_BZA_D_sseq': 'route228',
    'SEQ_D_02_sseq': 'lake', 'SEQ_D_03_sseq': 'lake',
}
# Platinum's overworld weather -> Emerald's
WEATHER = {
    'OVERWORLD_WEATHER_RAINING': 'WEATHER_RAIN', 'OVERWORLD_WEATHER_ROUTE_212_SOUTH': 'WEATHER_RAIN',
    'OVERWORLD_WEATHER_FOG': 'WEATHER_FOG_HORIZONTAL', 'OVERWORLD_WEATHER_BLIZZARD': 'WEATHER_SNOW',
    'OVERWORLD_WEATHER_SNOWPOINT_CITY': 'WEATHER_SNOW', 'OVERWORLD_WEATHER_ACUITY_LAKEFRONT': 'WEATHER_SNOW',
    'OVERWORLD_WEATHER_ROUTE_216': 'WEATHER_SNOW', 'OVERWORLD_WEATHER_SANDSTORM': 'WEATHER_SANDSTORM',
    'OVERWORLD_WEATHER_SLOW_ASHFALL': 'WEATHER_VOLCANIC_ASH',
}


def ident(header):
    """MAP_HEADER_ROUTE_204_NORTH -> ('ROUTE204_NORTH', 'Route204North')"""
    name = header[len('MAP_HEADER_'):]
    const = re.sub(r'_(\d)', r'\1', name)
    camel = ''.join(p.capitalize() for p in re.split(r'_', name))
    return const, camel


def places(src, root):
    m = src.matrix(0)
    cells = {}
    for z in range(m.height):
        for x in range(m.width):
            lid, header, _ = m.cell(x, z)
            if lid == NONE or not header or header == FILLER:
                continue
            cells.setdefault(header, []).append((x, z))
    text = open(os.path.join(root, 'include/data/map_headers.h')).read()
    out = []
    for header, cs in cells.items():
        block = re.search(r'\[%s\] = \{(.*?)\n    \}' % header, text, re.S).group(1)
        field = lambda k: re.search(r'\.%s = (\w+)' % k, block).group(1)   # noqa: E731
        xs, zs = [c[0] for c in cs], [c[1] for c in cs]
        const, camel = ident(header)
        ptype = field('mapType')
        if ptype == 'MAP_TYPE_TOWN_CITY':
            mtype = 'MAP_TYPE_CITY' if 'CITY' in const else 'MAP_TYPE_TOWN'
        elif ptype == 'MAP_TYPE_CAVE':
            mtype = 'MAP_TYPE_UNDERGROUND'
        else:
            mtype = 'MAP_TYPE_ROUTE'
        out.append({'header': header, 'id': 'MAP_SINNOH_' + const, 'name': 'Sinnoh_' + camel,
                    'x0': min(xs), 'z0': min(zs), 'x1': max(xs), 'z1': max(zs),
                    'music': 'MUS_SINNOH_' + MUSIC.get(field('dayMusicID'), 'route201').upper(),
                    'type': mtype, 'weather': WEATHER.get(field('weather'), 'WEATHER_SUNNY'),
                    'bike': field('isBikeAllowed') == 'TRUE'})
    return out


def connections(p, every):
    conns = []
    for q in every:
        if q is p:
            continue
        if q['z1'] + 1 == p['z0'] or q['z0'] == p['z1'] + 1:        # above / below, overlapping in x
            if q['x0'] <= p['x1'] and q['x1'] >= p['x0']:
                conns.append({'map': q['id'], 'offset': (q['x0'] - p['x0']) * 32,
                              'direction': 'up' if q['z1'] + 1 == p['z0'] else 'down'})
        elif q['x1'] + 1 == p['x0'] or q['x0'] == p['x1'] + 1:      # left / right, overlapping in z
            if q['z0'] <= p['z1'] and q['z1'] >= p['z0']:
                conns.append({'map': q['id'], 'offset': (q['z0'] - p['z0']) * 32,
                              'direction': 'left' if q['x1'] + 1 == p['x0'] else 'right'})
    return sorted(conns, key=lambda c: (c['direction'], c['offset'], c['map']))


def main():
    root = sys.argv[1]
    src = open_source(root)
    every = places(src, root)
    groups = json.load(open('data/maps/map_groups.json'))
    layouts = json.load(open('data/layouts/layouts.json'))
    if GROUP not in groups:
        groups['group_order'].append(GROUP)
        groups[GROUP] = []
    known = groups[GROUP]
    every.sort(key=lambda p: (p['name'] not in known, known.index(p['name']) if p['name'] in known else 0, p['name']))
    for p in every:
        bw, bh = p['x1'] - p['x0'] + 1, p['z1'] - p['z0'] + 1
        w, h = 32 * bw, 32 * bh
        layout_id = 'LAYOUT_' + p['id'][4:]
        ldir = 'data/layouts/' + p['name']
        os.makedirs(ldir, exist_ok=True)
        open(ldir + '/map.bin', 'wb').write(struct.pack('<H', BLOCKED) * (w * h))
        open(ldir + '/border.bin', 'wb').write(struct.pack('<H', BLOCKED) * 4)
        entry = {'id': layout_id, 'name': p['name'] + '_Layout', 'width': w, 'height': h,
                 'primary_tileset': 'gTileset_General', 'secondary_tileset': 'gTileset_Petalburg',
                 'border_filepath': ldir + '/border.bin', 'blockdata_filepath': ldir + '/map.bin'}
        layouts['layouts'] = [l for l in layouts['layouts'] if l.get('id') != layout_id] + [entry]
        mdir = 'data/maps/' + p['name']
        os.makedirs(mdir, exist_ok=True)
        path = mdir + '/map.json'
        old = json.load(open(path)) if os.path.exists(path) else {}
        m = {'id': p['id'], 'name': p['name'], 'layout': layout_id, 'music': p['music'],
             'region_map_section': 'MAPSEC_NONE', 'requires_flash': False, 'weather': p['weather'],
             'map_type': p['type'], 'allow_cycling': p['bike'], 'allow_escaping': False, 'allow_running': True,
             'show_map_name': False, 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL',
             'connections': connections(p, every)}
        for k in ('object_events', 'warp_events', 'coord_events', 'bg_events'):   # (events added by hand stay)
            m[k] = old.get(k, [])
        json.dump(m, open(path, 'w'), indent=2)
        open(path, 'a').write('\n')
        if not os.path.exists(mdir + '/scripts.inc'):
            open(mdir + '/scripts.inc', 'w').write('%s_MapScripts::\n\t.byte 0\n' % p['name'])
        if p['name'] not in known:
            known.append(p['name'])
    json.dump(groups, open('data/maps/map_groups.json', 'w'), indent=2)
    open('data/maps/map_groups.json', 'a').write('\n')
    json.dump(layouts, open('data/layouts/layouts.json', 'w'), indent=2)
    open('data/layouts/layouts.json', 'a').write('\n')
    # the maps' scripts, and the 3D engine's table (map, its top-left matrix cell, its size in cells)
    path = 'data/event_scripts_sinnoh.s'
    text = open(path).read()
    head = text[:text.index('\t.include "data/maps/Sinnoh_')]
    open(path, 'w').write(head + ''.join('\t.include "data/maps/%s/scripts.inc"\n' % n for n in known))
    rows = ''.join('    {%s, %d, %d, %d, %d},\n' % (p['id'], p['x0'], p['z0'], p['x1'] - p['x0'] + 1, p['z1'] - p['z0'] + 1)
                   for p in every)
    open('src/sinnoh/s3d_maps.h', 'w').write(
        '// Generated by tools/sinnoh/gen_maps.py: each SINNOH map, the matrix cell of its top-left block and its size\n'
        '// in blocks (src/sinnoh/s3d.c)\n'
        'static const struct { u16 map; u8 cx, cz, bw, bh; } sMaps[] = {\n' + rows + '};\n')
    print('%d maps: %s' % (len(every), ', '.join(p['name'] for p in every)))


if __name__ == '__main__':
    main()
