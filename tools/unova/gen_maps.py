#!/usr/bin/env python3
"""UNOVA's maps as the field engine's maps (docs/UNOVA_HANDOFF.md): every zone of Black's zone table that has blocks on
its matrix becomes a map (map.json, scripts.inc, a layout; their places in map_groups.json / layouts.json; UNOVA's
script file data/event_scripts_unova.s) and UNOVA's 3D engine's table of where each lies on its matrix
(src/unova/u3d_maps.h); UNOVA's map sections (include/constants/region_map_sections_unova.h) with Black's own place
names (src/data/region_map/unova_region_map.h). Read from the user's Black ROM (tools/unova/: the zones, matrices and
event files); nothing of Black's art, words or scripts is written here. The layouts' blocks are placeholders (every
tile blocked): the game takes each tile's collision and behavior from the pack when it loads the map
(src/unova/u3d.c U3D_LayoutBlocks, from Black's attributes by build_pack.py).

    python3 -I tools/unova/gen_maps.py         (run from the repository; safe to run again)

Maps go in groups gMapGroup_Unova1, 2... (PER_GROUP a group: a warp keeps its map number as an s8), by zone. A zone's map
is the rectangle of its matrix's blocks; outdoors (a matrix with its own zone grid: matrix 0, UNOVA's land, and the
cities that have their own) each block is a map of its own, 32 x 32 tiles, joined to the blocks beside it: Black's zones
there are any shape (ROUTE 1 wraps round NUVEMA TOWN's corner), which rectangles of maps can't join. Warps come from the zone's connections (a door walked up into warps as it's stepped on; a mat
walked off down, right or left is an arrow warp: tools/unova/build_pack.py gives those tiles their behavior).
"""
import json
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402
import matrix as mxr  # noqa: E402
import zones as zr  # noqa: E402
import events as ev  # noqa: E402
import text  # noqa: E402
import rails  # noqa: E402
import build_pack  # noqa: E402

GROUP = 'gMapGroup_Unova%d'
PER_GROUP = 120
MAPSEC_FIRST = 0x200
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)
PRIMARY, SECONDARY = 'gTileset_General', 'gTileset_Johto_GoldenrodUndergroundEntrances'   # (not drawn: 3D)
PLACE_NAMES = 89


def camel(name):
    return ''.join(w[:1].upper() + w[1:] for w in re.split(r'[^A-Za-z0-9]+', name.replace("'", '')) if w)


def snake(name):
    out = re.sub(r'_+', '_', re.sub(r'[^A-Z0-9]', '_', name.replace('é', 'E').replace('É', 'E').upper())).strip('_')
    return out or 'NONE'   # (Black's unnamed place, written as a row of dashes)


def ascii_name(name):
    """Black's place name as Kai's charset can print it (é kept, anything else plain)"""
    name = name.replace('’', "'").replace('－', '-').replace('Poké', 'POKé').upper()
    return name.replace('É', 'é')   # (the game's font has é only in lower case: POKéMON)


# Black's people as the map's object events, in its event data's order (UNOVA's VM finds a person's local id by its
# place: u3d_vm.c LocalId), their signs and trigger spots; Black's own scripts run for them (data/scripts/unova.inc).
# Their pictures are Black's own, by object code (OBJ_EVENT_GFX_UNOVA_FIRST + code: src/unova/u3d_gfx.c draws them from
# the sprites pack, tools/unova/build_gfx.py; a stand-in without it)
STAND_IN = 'OBJ_EVENT_GFX_YOUNGSTER'
UNOVA_GFX, UNOVA_GFX_COUNT = 0x600, 0x320   # (include/constants/event_objects.h OBJ_EVENT_GFX_UNOVA_FIRST)
SIZED_BLOCKER = 185   # (an invisible hit box as wide and deep as its params: STRIATON's gym's doorways)
BLACK_DIR = {0: 'UP', 1: 'DOWN', 2: 'LEFT', 3: 'RIGHT'}
BLACK_CITY = 0   # (its zone: its residents, see events_of)


def events_of(m, w, h):
    evs = ev.events(m['zone'])
    ox, oz = (m['rect'][0] * 32, m['rect'][1] * 32) if m['outdoor'] else (0, 0)
    objs, coords, bgs = [], [], []
    for p in evs['people']:
        x, z = p['x'] - ox, p['z'] - oz
        if p['rail']:   # (a rail place: its tile, tools/unova/rails.py)
            t = rails.event_tiles(m['zone'], 'people', p)
            x, z = (t[0][0] - ox, t[0][1] - oz) if t else (-1, -1)
        inside = 0 <= x < w and 0 <= z < h
        face = BLACK_DIR.get(p['dir'], 'DOWN')
        wander = p['move'] == 3 and (p['limit'][0] or p['limit'][1])
        objs.append({'graphics_id': '0x%X' % (UNOVA_GFX + p['gfx']) if p['gfx'] < UNOVA_GFX_COUNT else STAND_IN, 'x': x if inside else 0, 'y': z if inside else 0, 'elevation': 3,
                     'movement_type': 'MOVEMENT_TYPE_WANDER_AROUND' if wander else 'MOVEMENT_TYPE_FACE_' + face,
                     'movement_range_x': max(0, p['limit'][0]) if wander else 0,
                     'movement_range_y': max(0, p['limit'][1]) if wander else 0,
                     'trainer_type': 'TRAINER_TYPE_NONE',
                     # (Black's sized blocker, object code 185: its params' width and depth, as width | depth << 8:
                     # event_object_movement.c DoesObjectCollideWithObjectAt)
                     'trainer_sight_or_berry_tree_id': str(p['params'][0] | p['params'][1] << 8)
                     if p['gfx'] == SIZED_BLOCKER and p['params'][0] and p['params'][1] else '0',
                     'script': 'Unova_EventScript_Talk',
                     # (one whose spot is another map's: u3d_vm.c U3D_Vm_ObjectHidden keeps it hidden here; its
                     # place in the list is kept for the VM's ids)
                     'flag': '0'})
    if m['zone'] == BLACK_CITY and not objs:
        # (BLACK CITY's ten residents, Black's WFBC: placed at run time from its spots (a/1/5/4 member 0x22), drawn as
        # the work object codes 0xA2 + n the VM sets to each resident's own: src/unova/u3d_vm.c "BLACK CITY")
        spots = narc.arc('a/1/5/4')[0x22]
        for i in range(10):
            objs.append({'graphics_id': '0x%X' % (UNOVA_GFX + 0xA2 + i), 'x': spots[2 * i] - ox, 'y': spots[2 * i + 1] - oz,
                         'elevation': 3, 'movement_type': 'MOVEMENT_TYPE_FACE_DOWN', 'movement_range_x': 0,
                         'movement_range_y': 0, 'trainer_type': 'TRAINER_TYPE_NONE',
                         'trainer_sight_or_berry_tree_id': '0', 'script': 'Unova_EventScript_Talk', 'flag': '0'})
    for b in evs['bg']:
        x, z = b['x'] - ox, b['z'] - oz
        if b['rail']:
            t = rails.event_tiles(m['zone'], 'bg', b)
            x, z = (t[0][0] - ox, t[0][1] - oz) if t else (-1, -1)
        if 0 <= x < w and 0 <= z < h:
            bgs.append({'type': 'sign', 'x': x, 'y': z, 'elevation': 0,
                        'player_facing_dir': 'BG_EVENT_PLAYER_FACING_ANY', 'script': 'Unova_EventScript_Sign'})
    for q in evs['pos']:
        if q['check'] != 0:
            continue
        if q['rail']:
            spots = [(tx - ox, tz - oz) for tx, tz in rails.event_tiles(m['zone'], 'pos', q)]
        else:
            spots = [(q['x'] + dx - ox, q['z'] + dz - oz) for dz in range(max(1, q['h'])) for dx in range(max(1, q['w']))]
        for x, z in spots:
                if 0 <= x < w and 0 <= z < h:
                    coords.append({'type': 'trigger', 'x': x, 'y': z, 'elevation': 0,
                                   'var': '0x%X' % (0xB000 + q['work'] - 0x4000) if q['work'] >= 0x4000 else 'VAR_TEMP_0',
                                   'var_value': str(q['value']), 'script': 'Unova_EventScript_Pos'})
    return objs, coords, bgs


def songs():
    """{Black's sequence number: its song slot's name} (tools/kaisong/unova_songs.py)"""
    import importlib.util
    spec = importlib.util.spec_from_file_location('unova_songs', os.path.join(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__))), 'kaisong', 'unova_songs.py'))
    us = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(us)
    try:
        return {i: us.slot_name(n).upper() for i, n in us.sequences()}
    except OSError:
        return {}


def zone_maps():
    """[{zone, name, id, matrix, rect (x0, z0, x1, z1 in cells), place, outdoor}] in zone order: an indoor zone's blocks
    as one map, each outdoor block (a matrix with its own zone grid) a map"""
    names = text.bank(narc.arc(text.SYSTEM)[PLACE_NAMES])
    out, seen = [], {}
    mats = {}
    for z in zr.zones():
        if z['matrix'] not in mats:
            mats[z['matrix']] = mxr.matrix(z['matrix'], z['id'])
        outdoor = mats[z['matrix']]['mode'] == 1
        mx = mats[z['matrix']] if outdoor else mxr.matrix(z['matrix'], z['id'])
        cells = [i for i in range(mx['w'] * mx['h']) if mx['zones'][i] == z['id'] and mx['blocks'][i] != mxr.NONE]
        if not cells:
            continue
        place = names[z['place']] if z['place'] < len(names) else 'Unova'
        base = camel(place) or 'Place'
        stem = 'Unova_' + base if base not in seen else 'Unova_%s_Z%d' % (base, z['id'])
        seen[base] = True
        if outdoor and len(cells) > 1:
            pieces = [((i % mx['w'], i // mx['w'], i % mx['w'], i // mx['w']), '%s_%d_%d' % (stem, i % mx['w'],
                                                                                                i // mx['w']))
                      for i in cells]
        elif outdoor:
            i = cells[0]
            pieces = [((i % mx['w'], i // mx['w'], i % mx['w'], i // mx['w']), stem)]
        else:
            xs, zs = [i % mx['w'] for i in cells], [i // mx['w'] for i in cells]
            pieces = [((min(xs), min(zs), max(xs), max(zs)), stem)]
        k = build_pack.scale_of([z['id']])
        if k > 1 or z['id'] in rails.rail_files():
            # (a matrix of bigger blocks: its maps in 512-unit cells; rails past the blocks: the map grown to hold them)
            grown = []
            for (x0, z0, x1, z1), name in pieces:
                x0, z0, x1, z1 = x0 * k, z0 * k, (x1 + 1) * k - 1, (z1 + 1) * k - 1
                if z['id'] in rails.rail_files() and len(pieces) == 1:
                    ts = [t for t in rails.cells(rails.rail_files()[z['id']]) if t[0] >= 0 and t[1] >= 0]
                    x1, z1 = max(x1, max(t[0] for t in ts) // 32), max(z1, max(t[1] for t in ts) // 32)
                grown.append(((x0, z0, x1, z1), name))
            pieces = grown
        for rect, name in pieces:
            out.append({'zone': z['id'], 'name': name, 'id': 'MAP_' + snake(name), 'matrix': z['matrix'],
                        'rect': rect, 'place': place, 'outdoor': outdoor})
    return out


def main():
    maps = zone_maps()

    def holder(zone, tx, tz):
        """the map of ZONE holding its matrix tile (tx, tz)"""
        for m in maps:
            if m['zone'] == zone and m['rect'][0] * 32 <= tx < (m['rect'][2] + 1) * 32 \
                    and m['rect'][1] * 32 <= tz < (m['rect'][3] + 1) * 32:
                return m
        return None
    groups = json.load(open('data/maps/map_groups.json'))
    layouts = json.load(open('data/layouts/layouts.json'))
    ngroups = (len(maps) + PER_GROUP - 1) // PER_GROUP
    for g in range(1, ngroups + 1):
        if GROUP % g not in groups['group_order']:
            groups['group_order'].append(GROUP % g)
        groups[GROUP % g] = []
    current = {'LAYOUT_' + m['id'][4:] for m in maps}
    layouts['layouts'] = [l for l in layouts['layouts']
                          if not l.get('id', '').startswith('LAYOUT_UNOVA_') or l['id'] in current]
    # places (map sections): Black's own names, in the order maps first name them
    places = []
    for m in maps:
        sec = 'MAPSEC_UNOVA_' + snake(m['place'])
        if all(s != sec for s, _ in places):
            places.append((sec, m['place']))
    # every map's warps first (a Black connection may cover several tiles: each a warp, to the middle one of its
    # destination's: a mat three wide is landed on in its middle, as Black puts the player), then the maps
    warps = {m['name']: [] for m in maps}
    landing = {}   # (zone, Black's connection index) -> (map, its middle warp there)
    for z in sorted({m['zone'] for m in maps}):
        for k, c in enumerate(ev.events(z)['connect']):
            if c['rail']:   # (a door on a rail: its tiles, tools/unova/rails.py)
                tiles = rails.event_tiles(z, 'connect', c)
            else:
                tiles = [(c['x'] // 16 + dx, c['z'] // 16 + dz) for dz in range(max(1, c['h']))
                         for dx in range(max(1, c['w']))]
            if not tiles:
                continue
            m = holder(z, tiles[0][0], tiles[0][1])
            if m is None:
                continue
            landing[(z, k)] = (m, len(warps[m['name']]) + len(tiles) // 2)
            for tx, tz in tiles:
                warps[m['name']].append((tx - m['rect'][0] * 32, tz - m['rect'][1] * 32, c['zone'], c['exit'],
                                         c['dir']))
    table = []
    song_of = songs()
    for k, m in enumerate(maps):
        group = GROUP % (k // PER_GROUP + 1)
        w = (m['rect'][2] - m['rect'][0] + 1) * 32
        h = (m['rect'][3] - m['rect'][1] + 1) * 32
        lid = 'LAYOUT_' + m['id'][4:]
        ldir = 'data/layouts/%s' % m['name']
        os.makedirs(ldir, exist_ok=True)
        open(ldir + '/map.bin', 'wb').write(struct.pack('<%dH' % (w * h), *([BLOCKED] * (w * h))))
        open(ldir + '/border.bin', 'wb').write(struct.pack('<4H', *([BLOCKED] * 4)))
        entry = {'id': lid, 'name': m['name'] + '_Layout', 'width': w, 'height': h,
                 'primary_tileset': PRIMARY, 'secondary_tileset': SECONDARY,
                 'border_filepath': ldir + '/border.bin', 'blockdata_filepath': ldir + '/map.bin'}
        for i, l in enumerate(layouts['layouts']):
            if l.get('id') == lid:
                layouts['layouts'][i] = entry
                break
        else:
            layouts['layouts'].append(entry)
        we = []
        for tx, tz, dzone, dexit, ddir in warps[m['name']]:
            if (dzone, dexit) not in landing or not (0 <= tx < w and 0 <= tz < h):
                continue
            d, dest = landing[(dzone, dexit)]
            we.append({'x': tx, 'y': tz, 'elevation': 0, 'dest_map': d['id'], 'dest_warp_id': str(dest)})
        # (connections: outdoor maps on the same matrix whose rectangles touch)
        conns = []
        if m['outdoor']:
            x0, z0, x1, z1 = m['rect']
            for o in maps:
                if o is m or o['matrix'] != m['matrix'] or not o['outdoor']:
                    continue
                ox0, oz0, ox1, oz1 = o['rect']
                if ox0 == x1 + 1 and oz0 <= z1 and oz1 >= z0:
                    conns.append({'map': o['id'], 'offset': (oz0 - z0) * 32, 'direction': 'right'})
                elif ox1 == x0 - 1 and oz0 <= z1 and oz1 >= z0:
                    conns.append({'map': o['id'], 'offset': (oz0 - z0) * 32, 'direction': 'left'})
                elif oz0 == z1 + 1 and ox0 <= x1 and ox1 >= x0:
                    conns.append({'map': o['id'], 'offset': (ox0 - x0) * 32, 'direction': 'down'})
                elif oz1 == z0 - 1 and ox0 <= x1 and ox1 >= x0:
                    conns.append({'map': o['id'], 'offset': (ox0 - x0) * 32, 'direction': 'up'})
        objs, coords, bgs = events_of(m, w, h)
        mdir = 'data/maps/%s' % m['name']
        os.makedirs(mdir, exist_ok=True)
        bgm = zr.zones()[m['zone']]['bgm'][0]   # (spring's: the VM plays the season's, u3d_vm.c U3D_ZoneSong)
        mj = {'id': m['id'], 'name': m['name'], 'layout': lid, 'music': song_of.get(bgm, 'MUS_NONE'),
              'region_map_section': 'MAPSEC_UNOVA_' + snake(m['place']), 'requires_flash': False,
              'weather': 'WEATHER_NONE', 'map_type': 'MAP_TYPE_TOWN' if m['outdoor'] else 'MAP_TYPE_INDOOR',
              'allow_cycling': m['outdoor'], 'allow_escaping': False, 'allow_running': True,
              'show_map_name': m['outdoor'], 'battle_scene': 'MAP_BATTLE_SCENE_NORMAL',
              'connections': conns or None, 'object_events': objs, 'warp_events': we, 'coord_events': coords,
              'bg_events': bgs}
        json.dump(mj, open(mdir + '/map.json', 'w'), indent=2)
        open(mdir + '/map.json', 'a').write('\n')
        sp = mdir + '/scripts.inc'
        # (Black's own scripts run in UNOVA's VM, src/unova/u3d_vm.c: a map only says when)
        open(sp, 'w').write('%s_MapScripts::\n\tmap_script MAP_SCRIPT_ON_TRANSITION, Unova_EventScript_MapLoad\n'
                            '\t.byte 0\n' % m['name'])
        groups[group].append(m['name'])
        table.append('    {%s, %d, %d, %d, %d, %d, %d},' % (m['id'], m['zone'], m['matrix'], m['rect'][0], m['rect'][1],
                                                        m['rect'][2] - m['rect'][0] + 1, m['rect'][3] - m['rect'][1] + 1))
    json.dump(groups, open('data/maps/map_groups.json', 'w'), indent=2)
    open('data/maps/map_groups.json', 'a').write('\n')
    json.dump(layouts, open('data/layouts/layouts.json', 'w'), indent=2)
    open('data/layouts/layouts.json', 'a').write('\n')
    sp = 'data/event_scripts_unova.s'
    lines = open(sp).read().split('\n') if os.path.exists(sp) else [
        "@ Moemon Kai: UNOVA's map scripts (docs/UNOVA_HANDOFF.md; the maps from tools/unova/gen_maps.py)",
        '#include "constants/global.h"', '#include "constants/flags.h"', '#include "constants/vars.h"',
        '#include "constants/comparison_operators.h"', '#include "constants/items.h"',
        '#include "constants/map_scripts.h"', '#include "constants/map_groups.h"', '#include "constants/maps.h"', '#include "constants/songs.h"', '\t.include "asm/macros.inc"', '\t.include "constants/constants.inc"',
        '\t.include "asm/macros/event.inc"', '\t.section script_data, "aw"', '']
    for m in maps:
        inc = '\t.include "data/maps/%s/scripts.inc"' % m['name']
        if inc not in lines:
            at = max([i for i, l in enumerate(lines) if '.include "data/maps/' in l] or
                     [i for i, l in enumerate(lines) if 'script_data' in l])
            lines.insert(at + 1, inc)
    open(sp, 'w').write('\n'.join(lines))
    with open('include/constants/region_map_sections_unova.h', 'w') as f:
        f.write("// Generated by tools/unova/gen_maps.py: UNOVA's map sections, Black's own place names\n"
                '#ifndef GUARD_CONSTANTS_REGION_MAP_SECTIONS_UNOVA_H\n#define GUARD_CONSTANTS_REGION_MAP_SECTIONS_UNOVA_H\n')
        f.write('#define MAPSEC_UNOVA_FIRST 0x%X\n' % MAPSEC_FIRST)
        for i, (c, _) in enumerate(places):
            f.write('#define %-44s (MAPSEC_UNOVA_FIRST + %d)\n' % (c, i))
        f.write('#define MAPSEC_UNOVA_END (MAPSEC_UNOVA_FIRST + %d)\n#endif\n' % len(places))
    with open('src/data/region_map/unova_region_map.h', 'w') as f:
        f.write("// Generated by tools/unova/gen_maps.py: UNOVA's map section names, Black's own (its place names)\n")
        f.write('static const u8 *const gUnovaMapSectionNames[] = {\n')
        for c, n in places:
            f.write('    [%s - MAPSEC_UNOVA_FIRST] = (const u8[])_("%s"),\n' % (c, ascii_name(n)))
        f.write('};\n')
    # rail triggers' tiles (zone tiles), for the VM to tell which one the player stepped on (u3d_vm.c U3D_VmPos)
    rows = []
    for zone in sorted({m['zone'] for m in maps}):
        for i, q in enumerate(ev.events(zone)['pos']):
            if q['rail']:
                for tx, tz in rails.event_tiles(zone, 'pos', q):
                    if tx >= 0 and tz >= 0:
                        rows.append('    {%d, %d, %d, %d},' % (zone, i, tx, tz))
    with open('src/unova/u3d_railpos.h', 'w') as f:
        f.write("// Generated by tools/unova/gen_maps.py: each rail trigger's tiles (zone, its place in the zone's\n"
                "// triggers, zone tile x, z: tools/unova/rails.py), for u3d_vm.c U3D_VmPos\n")
        f.write('static const struct { u16 zone; u8 index; u16 x, z; } sUnovaRailPos[] = {\n')
        f.write('\n'.join(rows) + '\n};\n')
        # the scripts' warps to a rail place (WarpToRailLocation*: zone, rail line, along, across), as tiles
        import scripts as scr
        seen = set()
        for sf in range(len(narc.arc(scr.ARC))):
            if sf in scr.init_files():
                continue
            try:
                cmds, _, _ = scr.decode(sf)
            except Exception:   # noqa: BLE001
                continue
            for _, name, args, _ in cmds.values():
                if 'WarpToRailLocation' in name and rails.rail_files().get(args[0]) is not None:
                    # (FIELD_RAIL_MakeLineLocation: the place across counts from the line's edge, its half width
                    # there taken off)
                    rf = rails.rail_files()[args[0]]
                    rl = rails.load(rf)
                    if args[1] >= len(rl['lines']):
                        continue
                    ln = max(1, rl['lines'][args[1]]['length']) * rails.CELL
                    half = int(rails._width_at(rl, args[1], args[2] * rails.CELL, ln)) // 16
                    t = rails.tile_of(rf, args[1], args[2], args[3] - half)
                    if t:
                        seen.add('    {%d, %d, %d, %d, %d, %d},' % (args[0], args[1], args[2], args[3], t[0], t[1]))
        f.write('static const struct { u16 zone, line, along, across, x, z; } sUnovaRailWarps[] = {\n')
        f.write('\n'.join(sorted(seen)) + '\n};\n')
    # matrices a story flag replaces (Black's map replacement, selectors 3-12: the flag's work holding its value)
    import scrcmd
    ram, ov = scrcmd.ov_image(10)
    flags = [struct.unpack_from('<BBHH', ov, 0x02170FB0 - ram + 6 * i) for i in range(10)]
    rep = []
    for m, kind, sel, ids in build_pack.replace_records():
        f = [x for x in flags if x[0] == sel]
        if kind == 1 and sel >= 3 and f:
            rep.append('    {%d, %d, 0x%04X, %d},' % (m, ids[1], f[0][2], f[0][3]))
    with open('src/unova/u3d_replace.h', 'w') as f:
        f.write("// Generated by tools/unova/gen_maps.py: Black's map replacements a story flag switches on (matrix, its\n"
                "// replacement, the work, its value: ov010 map_replace.c), for u3d.c\n")
        f.write('static const struct { u16 mat, alt, work, value; } sUnovaReplace[] = {\n' + '\n'.join(rep) + '\n};\n')
    os.makedirs('src/unova', exist_ok=True)
    with open('src/unova/u3d_maps.h', 'w') as f:
        f.write("// Generated by tools/unova/gen_maps.py: each UNOVA map, its Black zone, its matrix, the cell of its\n"
                "// top-left block there and its size in blocks (src/unova/u3d.c)\n")
        f.write('#define UNOVA_GROUPS %d\n' % ngroups)
        f.write('static const struct { u16 map, zone, mat; u8 cx, cz, bw, bh; } sUnovaMaps[] = {\n')
        f.write('\n'.join(table) + '\n};\n')
    print('%d UNOVA maps in %d groups, %d places' % (len(maps), ngroups, len(places)))


if __name__ == '__main__':
    main()
