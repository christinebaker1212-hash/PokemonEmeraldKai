#!/usr/bin/env python3
"""SINNOH's gyms that move (src/sinnoh/s3d_gyms.c): what they need from Platinum's data, as numbers, written to
src/data/sinnoh_gyms.h. Platinum's code for them is overlay008/gym_features.c; nothing of it is copied, only where
its parts lie: the models it loads at run time (tools/sinnoh/gymprops.py: their numbers), and per gym the tiles that
matter (from the blocks' terrain attributes and props).

    python3 -I tools/sinnoh/gen_gyms.py POKEPLATINUM_DIR      (run from the repository; safe to run again)

PASTORIA: the pool's tiles (behavior 0x59: deep water where the water stands above the floor), its gates (0x56 / 0x57
/ 0x58: open at the water's low / middle / high level) and its buttons (the blue, green and orange button props: the
water to high / middle / low), map tiles.
SUNYSHORE: each room's gears (model, tile, start quarter turn, turning way, upright), its collision regions and which
of them block at each of the four turns (Platinum's tables, read from its gym_features.c), and its buttons (the
rooms' coord events: a quarter turn, back a quarter, or a half turn).
"""
import json
import re
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source  # noqa: E402
from gen_maps import places  # noqa: E402
from platinum import Land  # noqa: E402
import gymprops  # noqa: E402


def blocks(src, p):
    """[(map x0, y0 of the block, Land)] of a place"""
    mt = src.matrix(p['matrix'])
    out = []
    for cz in range(p['z0'], p['z1'] + 1):
        for cx in range(p['x0'], p['x1'] + 1):
            out.append(((cx - p['x0']) * 32, (cz - p['z0']) * 32, Land(src.land(mt.cell(cx, cz)[0]))))
    return out


def tiles_with(src, p, behaviors):
    out = []
    for bx, by, land in blocks(src, p):
        for t in range(1024):
            b = land.attrs[t] & 0xFF
            if b in behaviors:
                out.append((bx + t % 32, by + t // 32, b))
    return out


def props_named(src, p, names):
    """[(map x, y, name)] of the place's props of those models (the tile under the prop's point)"""
    out = []
    for bx, by, land in blocks(src, p):
        for pr in land.props:
            n = src._props[pr.model] if pr.model < len(src._props) else ''
            if n in names:
                out.append((bx + int(pr.pos[0] // 16 + 16), by + int(pr.pos[2] // 16 + 16), n))
    return out


def c_struct_list(text, name):
    """The initializers of C array NAME as dicts of .field = value"""
    m = re.search(r'%s\[[^]]*\]\s*=\s*\{(.*?)\n\};' % re.escape(name), text, re.S)
    out = []
    for body in re.findall(r'\{([^{}]*)\}', m.group(1)):
        out.append({k: v.strip() for k, v in re.findall(r'\.(\w+)\s*=\s*([^,]+)', body)})
    return out


def c_list(text, name):
    m = re.search(r'%s\[\]\s*=\s*\{([^}]*)\}' % re.escape(name), text)
    return [int(v) for v in m.group(1).replace(',', ' ').split()]


SUNYSHORE_BUTTONS = {1: {2: 1}, 2: {2: 1, 3: 3}, 3: {2: 1, 3: 2}}   # (room: script entry -> quarter turns: 1, 3 back, 2)


def sunyshore(root):
    text = open(os.path.join(root, 'src/overlay008/gym_features.c'), encoding='utf-8').read()
    lines = ['', '// SUNYSHORE: per room its gears {model, x, y, z (tiles), start quarter, clockwise, upright}, its regions {x,',
             '// z, w, d (tiles)}, the regions that block at each quarter turn (0-3), its buttons {x, z, quarter turns}']
    for room in (1, 2, 3):
        gears = c_struct_list(text, 'sSunyshoreRoom%dGears' % room)
        regions = c_struct_list(text, 'sSunyshoreRoom%dCollisionRegions' % room)
        lines.append('static const struct SunyshoreGear sSunyshoreGears%d[] = {%s};' % (room, ', '.join(
            '{S3D_MODEL_%s, %s, %s, %s, %s, %s, %s}' % (ident(g['modelID'].replace('_nsbmd', '.nsbmd')), g['x'], g['y'],
                                                        g['z'], g['initialRotation'], int(g['turnsClockwise'] == 'TRUE'),
                                                        int(g['isVertical'] == 'TRUE')) for g in gears)))
        lines.append('static const u8 sSunyshoreRegions%d[][4] = {%s};' % (room, ', '.join(
            '{%s, %s, %s, %s}' % (r['x'], r['z'], r['sizeX'], r['sizeZ']) for r in regions)))
        n = len(c_list(text, 'sSunyshoreRoom%dState0Collisions' % room))
        lines.append('static const u8 sSunyshoreBlock%d[4][%d] = {%s};' % (room, n, ', '.join(
            '{%s}' % ', '.join(str(v) for v in c_list(text, 'sSunyshoreRoom%dState%dCollisions' % (room, st))[:n]) for st in range(4))))
        ev = json.load(open(os.path.join(root, 'res/field/events/events_sunyshore_city_gym_room_%d.json' % room)))
        buttons = [(c['x'], c['z'], SUNYSHORE_BUTTONS[room][c['script']]) for c in ev['coord_events']]
        lines.append('static const u8 sSunyshoreButtons%d[][3] = {%s};' % (room, ', '.join('{%d, %d, %d}' % b for b in buttons)))
    return lines


def ident(name):
    return name.replace('.nsbmd', '').upper()


def main():
    root = sys.argv[1]
    src = open_source(root)
    by = {p['name']: p for p in places(src, root)}
    out = ['// Generated by tools/sinnoh/gen_gyms.py from pret/pokeplatinum: SINNOH\'s gyms that move (src/sinnoh/s3d_gyms.c)', '']
    out.append('// Platinum\'s model numbers of the props its gyms load at run time (the pack\'s named meshes)')
    for n in gymprops.model_names():
        out.append('#define S3D_MODEL_%s %d' % (ident(n), src._props.index(n)))
    out.append('')
    p = by['Sinnoh_PastoriaCityGym']
    deep = [(x, y) for x, y, b in tiles_with(src, p, {0x59})]
    gates = [(x, y, {0x56: 0, 0x57: 1, 0x58: 2}[b]) for x, y, b in tiles_with(src, p, {0x56, 0x57, 0x58})]
    color = {'pastoria_gym_orange_button.nsbmd': 0, 'pastoria_gym_green_button.nsbmd': 1, 'pastoria_gym_blue_button.nsbmd': 2}
    buttons = [(x, y, color[n]) for x, y, n in props_named(src, p, set(color))]
    out.append('// PASTORIA: the deep water\'s tiles, the gates (open at water level 0 low, 1 middle, 2 high), the buttons')
    out.append('// (the water to that level)')
    out.append('static const u8 sPastoriaDeep[][2] = {%s};' % ', '.join('{%d, %d}' % t for t in deep))
    out.append('static const u8 sPastoriaGates[][3] = {%s};' % ', '.join('{%d, %d, %d}' % t for t in gates))
    out.append('static const u8 sPastoriaButtons[][3] = {%s};' % ', '.join('{%d, %d, %d}' % t for t in sorted(buttons)))
    out += sunyshore(root)
    open('src/data/sinnoh_gyms.h', 'w').write('\n'.join(out) + '\n')
    print('%d models; PASTORIA: %d deep tiles, %d gates, %d buttons' % (len(gymprops.model_names()), len(deep), len(gates),
                                                                     len(buttons)))
    return buttons


if __name__ == '__main__':
    main()
