#!/usr/bin/env python3
"""The SINNOH maps as the field engine's maps (docs/SINNOH_HANDOFF.md): every outdoor place on Platinum's map matrix 0
(its towns, cities, routes and lakefronts) as a map of group 40, with map.json, scripts.inc, a layout and their places
in map_groups.json / layouts.json, and the 3D engine's table of where each one lies on the matrix
(src/sinnoh/s3d_maps.h). Only the places' names, rectangles, music, type and weather are read from pret/pokeplatinum
(its map matrix and map header table); nothing of Platinum's art is written here. The layouts' blocks are placeholders
(every cell blocked): the game takes each cell's collision and behavior from the 3D pack when it loads the map
(src/sinnoh/s3d.c S3D_LayoutBlocks, from Platinum's terrain attributes by build_pack.py).

    python3 -I tools/sinnoh/gen_maps.py POKEPLATINUM_DIR      (run from the repository; safe to run again)

Buildings (each its own matrix, tools/sinnoh/places.py) go in groups 41 on, 120 a group (a warp's map number is an
s8), by name. Every place gets
its warps from its events file (warps into places left out are dropped). A place covering an L of matrix cells (ETERNA CITY, OREBURGH CITY, ROUTE 209, ROUTE 224) becomes the rectangle around
it; the corner left over is the matrix's filler (sea, mountains), blocked. Maps whose rectangles touch are connected.
Map numbers stay as they were (TWINLEAF TOWN 0, ROUTE 201 1); new places are added after the last.
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from places import places as kept_places, events  # noqa: E402
import ids  # noqa: E402
import items  # noqa: E402
import gen_trainers  # noqa: E402
import sprites  # noqa: E402
import legends  # noqa: E402

GROUP = 'gMapGroup_Sinnoh'                      # the outdoor places
INDOOR_GROUP = 'gMapGroup_SinnohIndoor%d'       # the buildings, PER_GROUP a group (1, 2, 3...)
PER_GROUP = 120                 # (a warp keeps its map number as an s8: at most 127 maps a group)
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3
# the tilesets aren't used: the 3D engine draws the picture and each block's metatile id carries its behavior
# (tools/sinnoh/build_pack.py BEHAVIOR, fieldmap.c GetMetatileAttributesById)
SECONDARY = 'gTileset_Johto_GoldenrodUndergroundEntrances'

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
    'SEQ_D_LAKE_sseq': 'lake', 'SEQ_D_RYAYHY_sseq': 'cavern_on_the_lake',
    # buildings, caves and the rest
    'SEQ_PC_01_sseq': 'pokecenter', 'SEQ_FS_sseq': 'pokemart', 'SEQ_GYM_sseq': 'gym', 'SEQ_KENKYUJO_sseq': 'research_lab',
    'SEQ_D_01_sseq': 'victory_road', 'SEQ_D_02_sseq': 'eterna_forest', 'SEQ_D_03_sseq': 'cavern_on_the_lake',
    'SEQ_D_04_sseq': 'oreburgh_mine', 'SEQ_D_05_sseq': 'mt_coronet', 'SEQ_D_06_sseq': 'iron_mountain',
    'SEQ_D_MOUNT1_sseq': 'mt_coronet', 'SEQ_D_MOUNT2_sseq': 'spear_pillar', 'SEQ_D_GINLOBBY_sseq': 'galactic_building',
    'SEQ_D_AGITO_sseq': 'galactic_hideout', 'SEQ_THE_EVENT04_sseq': 'galactic_hideout_deep',
    'SEQ_D_LEAGUE_sseq': 'league_showdown', 'SEQ_BLD_DENDO_sseq': 'hall_of_fame', 'SEQ_D_SAFARI_sseq': 'great_marsh',
    'SEQ_D_KOUEN_sseq': 'amity_square', 'SEQ_BLD_GAME_sseq': 'game_corner', 'SEQ_BLD_TV_sseq': 'jubilife',
    'SEQ_BF_TOWWER_sseq': 'battle_tower', 'SEQ_PL_BF_FACTORY_sseq': 'battle_tower', 'SEQ_PL_BF_STAGE_sseq': 'battle_tower',
    'SEQ_PL_BF_CASTLE_sseq': 'battle_tower', 'SEQ_PL_BF_ROULETTE_sseq': 'battle_tower',
    'SEQ_PL_D_GIRATINA_sseq': 'distortion_world', 'SEQ_AUS_sseq': 'arceus', 'SEQ_SILENCE_FIELD_sseq': 'villa',
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
    out = []
    for kp in kept_places(src, root):
        header, f, cs = kp['header'], kp['fields'], kp['cells']
        xs, zs = [c[0] for c in cs], [c[1] for c in cs]
        const, camel = ident(header)
        ptype = f.get('mapType')
        if ptype == 'MAP_TYPE_TOWN_CITY':
            mtype = 'MAP_TYPE_CITY' if 'CITY' in const else 'MAP_TYPE_TOWN'
        elif ptype == 'MAP_TYPE_CAVE':
            mtype = 'MAP_TYPE_UNDERGROUND'
        elif ptype == 'MAP_TYPE_INDOORS':
            mtype = 'MAP_TYPE_INDOOR'
        else:
            mtype = 'MAP_TYPE_ROUTE'
        out.append({'header': header, 'id': 'MAP_SINNOH_' + const, 'name': 'Sinnoh_' + camel,
                    'matrix': kp['matrix'], 'outdoor': kp['outdoor'], 'place': kp,
                    'x0': min(xs), 'z0': min(zs), 'x1': max(xs), 'z1': max(zs),
                    'music': 'MUS_SINNOH_' + MUSIC.get(f.get('dayMusicID'), 'twinleaf').upper(),
                    'type': mtype, 'weather': WEATHER.get(f.get('weather'), 'WEATHER_SUNNY'),
                    'bike': f.get('isBikeAllowed') == 'TRUE'})
    return out





def labels(name):
    """The scripts a map's own scripts.inc has (SINNOH's words are written for this game, not taken from Platinum's
    scripts: docs/SINNOH_HANDOFF.md step 4). A person, sign or trigger runs <map>_EventScript_Person<n> / _Sign<n> /
    _Bg<n> / _Trigger<n> when the file has it (n: its number in Platinum's lists, from 1)"""
    path = 'data/maps/%s/scripts.inc' % name
    if not os.path.exists(path):
        return set()
    return set(re.findall(r'^(\w+)::', open(path, encoding='utf-8').read(), re.M))


used_items = set()   # (item, count) of every item ball: data/scripts/sinnoh_items.inc


def inside(p, x, z):
    """A Platinum spot (whole-matrix tiles) inside the place's map"""
    return 0 <= x - p['x0'] * 32 < 32 * (p['x1'] - p['x0'] + 1) and 0 <= z - p['z0'] * 32 < 32 * (p['z1'] - p['z0'] + 1)


# People this game adds to a place, after Platinum's (their local ids follow): the GAME CENTER's attendant (as in
# GOLDENROD's and CELADON's GAME CORNERS: data/scripts/mp_region.inc EventScript_GameCenterGuide)
EXTRA_PEOPLE = {
    'Sinnoh_VeilstoneCityPrizeExchange': [
        {'graphics_id': 'OBJ_EVENT_GFX_SINNOH_UNUSED_RECEPTIONIST', 'x': 7, 'y': 4, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN',
         'script': 'EventScript_GameCenterGuide'},
    ],
}


def people(p, root, have):
    """The place's people: where they stand, the game's closest sprite (tools/sinnoh/sprites.py), how they move, their
    hide flag (SINNOH's number, tools/sinnoh/ids.py) and their script. Local ids follow Platinum's order; those with
    nothing to show stay, invisible (FLAG_SINNOH_HIDDEN, 0x3FFF, always set), to keep the order: signs (read as the
    map's signs), a neighbor's people Platinum loads early, and anyone the story shows or hides (a hide flag) until
    their script is written."""
    out = []
    for i, o in enumerate(events(root, p['place'])['object_events']):
        label = '%s_EventScript_Person%d' % (p['name'], i + 1)
        gfx = sprites.sprite(o['graphics_id'])
        hide = o.get('hidden_flag')
        flag = ids.flag(root, hide) if isinstance(hide, str) and hide.startswith('FLAG_') else None
        ghost = isinstance(hide, str) and hide.startswith('MAP_HEADER_')
        script = label if label in have else '0x0'
        trainer = isinstance(o['script'], str) and o['script'].startswith('TRAINER_')
        if trainer:
            # a trainer: its battle script (data/scripts/sinnoh_trainers.inc, tools/sinnoh/gen_trainers.py) and sight
            script = gen_trainers.label(o['script'])
        legend = legends.scripts().get((p['name'], i + 1))
        if legend:
            # a legendary: its battle (tools/sinnoh/legends.py), shown by its hide flag (Sinnoh_LegendsRefresh)
            out.append({'graphics_id': gfx, 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32, 'elevation': 3,
                        'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0, 'movement_range_y': 0,
                        'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                        'script': legend, 'flag': '0x%X' % flag})
            continue
        ball = items.ball(root, o['script']) if o['graphics_id'].endswith('POKEBALL') else None
        if ball and not inside(p, o['x'], o['z']):
            ball = None
        if ball:
            # an item ball: the shared script for its item (data/scripts/sinnoh_items.inc), its flag set once taken
            assert flag, '%s: item ball without a flag' % label
            script = items.script_label(*ball)
            used_items.add(ball)
            flag_s = '0x%X' % flag
        elif gfx is None or ghost or (flag and label not in have) or o['graphics_id'].endswith('POKEBALL') \
                or not inside(p, o['x'], o['z']):
            flag_s = '0x3FFF'
        else:
            flag_s = ('0x%X' % flag) if flag else '0'
        out.append({'graphics_id': gfx or 'OBJ_EVENT_GFX_ITEM_BALL', 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32,
                    'elevation': 3, 'movement_type': sprites.movement(o['movement_type']),
                    'movement_range_x': o.get('movement_range_x', 0), 'movement_range_y': o.get('movement_range_z', 0),
                    'trainer_type': 'TRAINER_TYPE_NORMAL' if trainer else 'TRAINER_TYPE_NONE',
                    'trainer_sight_or_berry_tree_id': str((o.get('data') or [0])[0]) if trainer else '0',
                    'script': script, 'flag': flag_s})
    for e in EXTRA_PEOPLE.get(p['name'], []):
        out.append({'graphics_id': e['graphics_id'], 'x': e['x'], 'y': e['y'], 'elevation': 3,
                    'movement_type': e['movement_type'], 'movement_range_x': 0, 'movement_range_y': 0,
                    'trainer_type': 'TRAINER_TYPE_NONE', 'trainer_sight_or_berry_tree_id': '0',
                    'script': e['script'], 'flag': '0'})
    return out


def signs(p, root, have):
    """Platinum's signs are people (a signpost, a mailbox, a book): each is a sign where it stands; then its own signs"""
    facing = {'BG_EVENT_DIR_NORTH': 'BG_EVENT_PLAYER_FACING_NORTH', 'BG_EVENT_DIR_SOUTH': 'BG_EVENT_PLAYER_FACING_SOUTH',
              'BG_EVENT_DIR_WEST': 'BG_EVENT_PLAYER_FACING_WEST', 'BG_EVENT_DIR_EAST': 'BG_EVENT_PLAYER_FACING_EAST'}
    out = []
    ev = events(root, p['place'])
    for i, o in enumerate(ev['object_events']):
        label = '%s_EventScript_Sign%d' % (p['name'], i + 1)
        if sprites.sign(o['graphics_id']) and label in have and inside(p, o['x'], o['z']):
            out.append({'type': 'sign', 'x': o['x'] - p['x0'] * 32, 'y': o['z'] - p['z0'] * 32, 'elevation': 0,
                        'player_facing_dir': 'BG_EVENT_PLAYER_FACING_ANY', 'script': label})
    for i, b in enumerate(ev['bg_events']):
        label = '%s_EventScript_Bg%d' % (p['name'], i + 1)
        if not inside(p, b['x'], b['z']):
            continue
        hid = items.buried(root, b['script']) if b.get('type') == 2 else None
        if hid:
            item, count, flag = hid    # (a count above 1 is given as 1: the game's hidden items hold one)
            out.append({'type': 'hidden_item', 'x': b['x'] - p['x0'] * 32, 'y': b['z'] - p['z0'] * 32, 'elevation': 0,
                        'item': item, 'flag': '0x%X' % flag})
            continue
        if b.get('type') != 0 or label not in have:
            continue
        out.append({'type': 'sign', 'x': b['x'] - p['x0'] * 32, 'y': b['z'] - p['z0'] * 32, 'elevation': 0,
                    'player_facing_dir': facing.get(b['player_facing_dir'], 'BG_EVENT_PLAYER_FACING_ANY'),
                    'script': label})
    return out


def triggers(p, root, have):
    """Platinum's coord events cover an area: the game's are one tile each. Only those with a script of this game's"""
    out = []
    for i, c in enumerate(events(root, p['place'])['coord_events']):
        label = '%s_EventScript_Trigger%d' % (p['name'], i + 1)
        v = ids.var(root, c['var']) if isinstance(c.get('var'), str) else None
        if v is None or label not in have or not inside(p, c['x'], c['z']):
            continue
        for dz in range(max(1, c.get('length', 1))):
            for dx in range(max(1, c.get('width', 1))):
                out.append({'type': 'trigger', 'x': c['x'] - p['x0'] * 32 + dx, 'y': c['z'] - p['z0'] * 32 + dz,
                            'elevation': 0, 'var': v if isinstance(v, str) else '0x%X' % v,
                            'var_value': str(int(c['value'])) if isinstance(c['value'], (bool, int)) else str(c['value']),
                            'script': label})
    return out


def kept_warps(p, ids, root):
    """[(Platinum's warp number, warp)] of the place's warps that lead to kept places with a way out of their own (an
    elevator room has none: Platinum's elevators run by script, so its doors stay shut until those are written)"""
    return [(i, w) for i, w in enumerate(events(root, p['place'])['warp_events'])
            if w['dest_header_id'] in ids and w['dest_header_id'] in _exits(ids, root)]


_exit_cache = {}


def _exits(ids, root):
    """Headers of the kept places that have a warp to a kept place"""
    key = id(ids)
    if key not in _exit_cache:
        _exit_cache[key] = {h for h, p in _places_by_header.items()
                            if any(w['dest_header_id'] in ids for w in events(root, p['place'])['warp_events'])}
    return _exit_cache[key]


_places_by_header = {}


def warps(p, ids, root, numbers):
    """The place's warps as the field engine's: local tiles, the kept place each leads to (warps to places left out
    are dropped) and the warp there, renumbered as that place's kept warps are (numbers: {map id: {Platinum's warp
    number: ours}}); a warp to one that was dropped goes to that place's first"""
    out = []
    for _, w in kept_warps(p, ids, root):
        dest = ids[w['dest_header_id']]
        to = numbers.get(dest, {}).get(w['dest_warp_id'], 0)
        out.append({'x': w['x'] - p['x0'] * 32, 'y': w['z'] - p['z0'] * 32, 'elevation': 0, 'dest_map': dest,
                    'dest_warp_id': str(to)})
    return out


def connections(p, every):
    conns = []
    if not p['outdoor']:
        return conns
    for q in every:
        if q is p or not q['outdoor']:
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
    map_ids = {p['header']: p['id'] for p in every}
    _places_by_header.update({p['header']: p for p in every})
    groups = json.load(open('data/maps/map_groups.json'))
    layouts = json.load(open('data/layouts/layouts.json'))
    # the outdoor places in group 40 (numbers kept), the buildings after them in groups of PER_GROUP (by name, so a
    # building's number only changes when places are added before it)
    outdoor = [p for p in every if p['outdoor']]
    indoor = sorted((p for p in every if not p['outdoor']), key=lambda p: p['name'])
    if GROUP not in groups:
        groups['group_order'].append(GROUP)
        groups[GROUP] = []
    known = groups[GROUP]
    outdoor.sort(key=lambda p: (p['name'] not in known, known.index(p['name']) if p['name'] in known else 0, p['name']))
    for p in outdoor:
        if p['name'] not in known:
            known.append(p['name'])
    order = list(groups['group_order'])
    for g in [g for g in order if g.startswith('gMapGroup_SinnohIndoor')]:
        groups['group_order'].remove(g)
        del groups[g]
    nindoor = (len(indoor) + PER_GROUP - 1) // PER_GROUP
    at = groups['group_order'].index(GROUP) + 1
    for k in range(nindoor):
        name = INDOOR_GROUP % (k + 1)
        groups['group_order'].insert(at + k, name)
        groups[name] = [p['name'] for p in indoor[k * PER_GROUP:(k + 1) * PER_GROUP]]
    numbers = {p['id']: {pi: k for k, (pi, _) in enumerate(kept_warps(p, map_ids, root))} for p in every}
    for p in outdoor + indoor:
        bw, bh = p['x1'] - p['x0'] + 1, p['z1'] - p['z0'] + 1
        w, h = 32 * bw, 32 * bh
        layout_id = 'LAYOUT_' + p['id'][4:]
        ldir = 'data/layouts/' + p['name']
        os.makedirs(ldir, exist_ok=True)
        open(ldir + '/map.bin', 'wb').write(struct.pack('<H', BLOCKED) * (w * h))
        open(ldir + '/border.bin', 'wb').write(struct.pack('<H', BLOCKED) * 4)
        entry = {'id': layout_id, 'name': p['name'] + '_Layout', 'width': w, 'height': h,
                 'primary_tileset': 'gTileset_General', 'secondary_tileset': SECONDARY,
                 'border_filepath': ldir + '/border.bin', 'blockdata_filepath': ldir + '/map.bin'}
        layouts['layouts'] = [l for l in layouts['layouts'] if l.get('id') != layout_id] + [entry]
        mdir = 'data/maps/' + p['name']
        os.makedirs(mdir, exist_ok=True)
        path = mdir + '/map.json'
        old = json.load(open(path)) if os.path.exists(path) else {}
        m = {'id': p['id'], 'name': p['name'], 'layout': layout_id, 'music': p['music'],
             'region_map_section': 'MAPSEC_NONE', 'requires_flash': False, 'weather': p['weather'],
             'map_type': p['type'], 'allow_cycling': p['bike'], 'allow_escaping': not p['outdoor'],
             'allow_running': True, 'show_map_name': False, 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL',
             'connections': connections(p, every)}
        have = labels(p['name'])
        m['object_events'] = people(p, root, have)
        m['warp_events'] = warps(p, map_ids, root, numbers)
        m['coord_events'] = triggers(p, root, have)
        m['bg_events'] = signs(p, root, have)
        json.dump(m, open(path, 'w'), indent=2)
        open(path, 'a').write('\n')
        if not os.path.exists(mdir + '/scripts.inc'):
            open(mdir + '/scripts.inc', 'w').write('%s_MapScripts::\n\t.byte 0\n' % p['name'])
    json.dump(groups, open('data/maps/map_groups.json', 'w'), indent=2)
    open('data/maps/map_groups.json', 'a').write('\n')
    json.dump(layouts, open('data/layouts/layouts.json', 'w'), indent=2)
    open('data/layouts/layouts.json', 'a').write('\n')
    # the maps' scripts, and the 3D engine's table (map, its matrix, its top-left cell there, its size in cells)
    names = known + [p['name'] for p in indoor]
    path = 'data/scripts/sinnoh_items.inc'
    open(path, 'w').write('@ Generated by tools/sinnoh/gen_maps.py: SINNOH\'s item balls, one script an item (tools/sinnoh/items.py)\n' +
                          ''.join('%s::\n\tfinditem %s, %d\n\tend\n\n' % (items.script_label(i, c), i, c)
                                  for i, c in sorted(used_items)))
    assert ids.FLAGS_START + ids.counts(root)[0] <= items.HIDDEN_FLAGS, 'SINNOH flags run into the hidden items\''
    path = 'data/event_scripts_sinnoh.s'
    text = open(path).read()
    head = text[:text.index('\t.include "data/maps/Sinnoh_')]
    if 'sinnoh_items.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_items.inc"\n'
    if 'sinnoh_marts.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_marts.inc"\n'
    if 'sinnoh_trainers.inc' not in head:
        head += '\t.include "data/scripts/sinnoh_trainers.inc"\n'
    open(path, 'w').write(head + ''.join('\t.include "data/maps/%s/scripts.inc"\n' % n for n in names))
    rows = ''.join('    {%s, %d, %d, %d, %d, %d},\n' % (p['id'], p['matrix'], p['x0'], p['z0'], p['x1'] - p['x0'] + 1,
                                                       p['z1'] - p['z0'] + 1) for p in outdoor + indoor)
    open('src/sinnoh/s3d_maps.h', 'w').write(
        '// Generated by tools/sinnoh/gen_maps.py: each SINNOH map, its Platinum matrix (0 the region), the cell of its\n'
        '// top-left block there and its size in blocks (src/sinnoh/s3d.c)\n'
        '#define SINNOH_GROUPS %d   // group 40 and the buildings\' groups after it\n' % (1 + nindoor) +
        'static const struct { u16 map, mat; u8 cx, cz, bw, bh; } sMaps[] = {\n' + rows + '};\n')
    print('%d maps (%d outdoor, %d buildings in %d groups)' % (len(every), len(outdoor), len(indoor), nindoor))


if __name__ == '__main__':
    main()
