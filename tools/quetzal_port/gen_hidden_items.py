#!/usr/bin/env python3
"""Crystal's hidden items in JOHTO (docs/REGIONS_AUDIT.md case 131; Quetzal dropped every one).

    python3 tools/quetzal_port/gen_hidden_items.py [--dry]

Reads pokecrystal's maps (POKECRYSTAL_DIR) for `hiddenitem` bg events and adds each to the JOHTO map of the same name
as a hidden_item bg event with its own flag (FLAG_JOHTO_HIDDEN_ITEM_<n>, 0x1600 + n, kai_regions.h). Quetzal redrew
the maps, so a spot moves by the offset of Crystal's nearest warp that has a match here (same destination), then to
the nearest tile the player can face from a walkable one; SPOTS overrides a spot picked by hand (map_ascii.py).
Maps Quetzal merged or lacks are in MERGED. Running it again changes nothing (an item already there is kept).
"""
import json
import os
import re
import struct
import sys

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CRYSTAL = os.environ.get('POKECRYSTAL_DIR', '/tmp/claude-0/frlg/pokecrystal')
FLAG_BASE = 0x1600

# Crystal's item names that differ here
ITEMS = {'ELIXER': 'ELIXIR', 'MAX_ELIXER': 'MAX_ELIXIR', 'PARLYZ_HEAL': 'PARALYZE_HEAL'}
# Crystal map -> our map, where the names differ (Quetzal's one map for several of Crystal's)
MERGED = {'OlivineLighthouse5F': 'Johto_OlivineLighthouse',   # Quetzal's lighthouse is one map (its spot in SPOTS)
          'NationalParkBugContest': None}           # the same park as NationalPark (one item would be twice)
# (Crystal map, item index in the map) -> (x, y) here, chosen by hand where the offset lands badly
SPOTS = {
    ('CianwoodCity', 0): (12, 28), ('CianwoodCity', 1): (14, 32),
    ('DragonsDenB1F', 0): (50, 48), ('DragonsDenB1F', 1): (33, 34), ('DragonsDenB1F', 2): (43, 32),
    ('EcruteakCity', 0): (25, 15),
    ('GoldenrodUnderground', 0): (6, 15), ('GoldenrodUnderground', 1): (4, 19), ('GoldenrodUnderground', 2): (14, 10),
    ('GoldenrodUndergroundSwitchRoomEntrances', 0): (9, 9), ('GoldenrodUndergroundSwitchRoomEntrances', 1): (2, 9),
    ('IcePathB1F', 0): (25, 42), ('IcePathB2FMahoganySide', 0): (8, 21),
    ('IlexForest', 0): (37, 17), ('IlexForest', 1): (57, 26), ('IlexForest', 2): (9, 31),
    ('LakeOfRage', 0): (19, 39), ('LakeOfRage', 1): (10, 4), ('LakeOfRage', 2): (55, 12),
    ('MountMortarB1F', 0): (4, 6),
    ('OlivineLighthouse5F', 0): (119, 15),     # (Quetzal's lighthouse: every floor side by side, 5F the fifth)
    ('OlivinePort', 0): (1, 20),
    ('Route32', 0): (26, 78), ('Route34', 0): (26, 62), ('Route34', 1): (30, 34), ('Route37', 0): (11, 13),
    ('Route41', 0): (17, 63),
    ('SilverCaveOutside', 0): (11, 20), ('SilverCaveRoom1', 0): (50, 33), ('SilverCaveRoom1', 1): (50, 12),
    ('SilverCaveRoom2', 0): (23, 48),
    ('TinTower5F', 0): (15, 16), ('TinTower5F', 1): (3, 16),
    ('WhirlIslandB1F', 0): (75, 8), ('WhirlIslandB1F', 1): (78, 36), ('WhirlIslandB1F', 2): (12, 52),
}


def norm(s):
    return re.sub(r'[^A-Z0-9]', '', s.upper().replace('MAP_JOHTO_', ''))


def crystal_maps():
    out = {}
    for fn in sorted(os.listdir(os.path.join(CRYSTAL, 'maps'))):
        if not fn.endswith('.asm'):
            continue
        text = open(os.path.join(CRYSTAL, 'maps', fn)).read()
        if 'BGEVENT_ITEM' not in text:
            continue
        name = fn[:-4]
        warps = [(int(x), int(y), '%s/%d' % (d, int(n) - 1))
                 for x, y, d, n in re.findall(r'warp_event\s+(\d+),\s*(\d+),\s*(\w+),\s*(\d+)', text)]
        items = []
        for x, y, label in re.findall(r'bg_event\s+(\d+),\s*(\d+),\s*BGEVENT_ITEM,\s*(\w+)', text):
            m = re.search(r'^%s:\n\s*hiddenitem (\w+), (\w+)' % label, text, re.M)
            items.append((int(x), int(y), m.group(1), m.group(2)))
        out[name] = (warps, items)
    return out


BLOCKED = {0x07, 0x08, 0x12, 0x15, 0x1a, 0x1d, 0x28} | set(range(0x90, 0xa0))


def crystal_grid(name):
    """Crystal's map as a numpy grid of 16x16 steps: 1 where the player can't walk (walls, trees, counters)"""
    import numpy as np
    maps = re.findall(r'^\tmap (\w+), (TILESET_\w+)', open(os.path.join(CRYSTAL, 'data/maps/maps.asm')).read(), re.M)
    dims = re.findall(r'^\tmap_const \w+,\s*(\d+),\s*(\d+)', open(os.path.join(CRYSTAL, 'constants/map_constants.asm')).read(), re.M)
    k = [m[0] for m in maps].index(name)
    tileset = maps[k][1][len('TILESET_'):].lower()
    bw, bh = int(dims[k][0]), int(dims[k][1])
    coll = []
    consts = {}
    for l in open(os.path.join(CRYSTAL, 'constants/collision_constants.asm')):
        m = re.match(r'DEF COLL_(\w+)\s+EQU \$(\w+)', l)
        if m:
            consts[m.group(1)] = int(m.group(2), 16)
    for l in open(os.path.join(CRYSTAL, 'data/tilesets/%s_collision.asm' % tileset)):
        m = re.match(r'\s*tilecoll ([^;]+)', l)
        if m:
            coll.append([consts[v.strip()] if v.strip() in consts else int(v.strip(), 16) for v in m.group(1).split(',')])
    blk = open(os.path.join(CRYSTAL, 'maps/%s.blk' % name), 'rb').read()
    g = np.zeros((bh * 2, bw * 2), dtype=np.int8)
    for by in range(bh):
        for bx in range(bw):
            c = coll[blk[by * bw + bx]]
            for q in range(4):
                g[by * 2 + q // 2, bx * 2 + q % 2] = 1 if c[q] in BLOCKED else 0
    return g


def our_grid(mj):
    import numpy as np
    w, h, cells = layout(mj)
    return np.array([[1 if (cells[y * w + x] >> 10) & 3 else 0 for x in range(w)] for y in range(h)], dtype=np.int8)


def align(cg, og, x, y, guess, r=8, near=10):
    """the offset (dx, dy) whose window of Crystal's map around (x, y) best matches ours, and how well (0..1);
    offsets near the warp's guess win ties"""
    ch, cw = cg.shape
    oh, ow = og.shape
    x0, x1, y0, y1 = max(0, x - r), min(cw, x + r + 1), max(0, y - r), min(ch, y + r + 1)
    win = cg[y0:y1, x0:x1]
    best = None
    for dy in range(-y0, oh - y1 + 1):
        for dx in range(-x0, ow - x1 + 1):
            o = og[y0 + dy:y1 + dy, x0 + dx:x1 + dx].astype(float)
            a, b = win.astype(float) - win.mean(), o - o.mean()
            den = (a * a).sum() ** 0.5 * (b * b).sum() ** 0.5
            score = (a * b).sum() / den if den else 0.0
            d = abs(dx - guess[0]) + abs(dy - guess[1]) if guess else 0
            if guess and max(abs(dx - guess[0]), abs(dy - guess[1])) > near:
                continue
            key = (round(score, 3), -d)
            if best is None or key > best[0]:
                best = (key, dx, dy, score)
    if best is None:
        return (guess or (0, 0)) + (0.0,)
    return best[1], best[2], best[3]


def layout(mj):
    lay = json.load(open(os.path.join(REPO, 'data/layouts/layouts.json')))
    l = next(l for l in lay['layouts'] if l.get('id') == mj['layout'])
    w, h = l['width'], l['height']
    data = open(os.path.join(REPO, l['blockdata_filepath']), 'rb').read()
    return w, h, struct.unpack('<%dH' % (w * h), data[:w * h * 2])


def place(mj, x, y):
    """the nearest tile to (x, y) the player can face from a walkable tile, and its elevation"""
    w, h, cells = layout(mj)
    taken = {(o['x'], o['y']) for o in mj.get('object_events', [])} | \
            {(v['x'], v['y']) for v in mj.get('warp_events', [])} | \
            {(b['x'], b['y']) for b in mj.get('bg_events', [])} | \
            {(c['x'], c['y']) for c in mj.get('coord_events', [])}

    def walk(cx, cy):
        return 0 <= cx < w and 0 <= cy < h and not (cells[cy * w + cx] >> 10) & 3 and (cx, cy) not in taken

    for r in range(0, 6):
        best = None
        for cy in range(y - r, y + r + 1):
            for cx in range(x - r, x + r + 1):
                if max(abs(cx - x), abs(cy - y)) != r or not (0 <= cx < w and 0 <= cy < h) or (cx, cy) in taken:
                    continue
                if walk(cx, cy):
                    elev = cells[cy * w + cx] >> 12
                else:
                    nb = [(nx, ny) for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)) if walk(nx, ny)]
                    if not nb:
                        continue
                    elev = cells[nb[0][1] * w + nb[0][0]] >> 12
                d = abs(cx - x) + abs(cy - y)
                if best is None or d < best[0]:
                    best = (d, cx, cy, elev)
        if best:
            return best[1], best[2], best[3]
    return None


def main():
    dry = '--dry' in sys.argv
    flags = []
    n = 0
    for name, (cwarps, items) in crystal_maps().items():
        ours = MERGED.get(name, 'Johto_' + name)
        if ours is None or not os.path.isdir(os.path.join(REPO, 'data/maps', ours)):
            print('skip', name, [i[2] for i in items])
            continue
        mpath = os.path.join(REPO, 'data/maps', ours, 'map.json')
        mj = json.load(open(mpath))
        owarps = [(v['x'], v['y'], '%s/%s' % (norm(v['dest_map']), v['dest_warp_id'])) for v in mj.get('warp_events', [])]
        key = lambda d: norm(d.split('/')[0]) + '/' + d.split('/')[1]
        pairs = [(cx, cy, ox - cx, oy - cy) for cx, cy, cd in cwarps for ox, oy, od in owarps if key(cd) == od]
        changed = False
        cg, og = crystal_grid(name), our_grid(mj)
        for k, (x, y, item, event) in enumerate(items):
            n += 1
            flag = 'FLAG_JOHTO_HIDDEN_ITEM_%d' % n
            flags.append((flag, FLAG_BASE + n, '%s %s (%s)' % (name, item, event)))
            item = 'ITEM_' + ITEMS.get(item, item)
            if any(b.get('flag') == flag for b in mj['bg_events']):
                continue
            if (name, k) in SPOTS:
                tx, ty = SPOTS[(name, k)]
                how = 'by hand'
            else:
                p = min(pairs, key=lambda p: abs(p[0] - x) + abs(p[1] - y)) if pairs else None
                dx, dy, score = align(cg, og, x, y, (p[2], p[3]) if p else None)
                tx, ty = x + dx, y + dy
                how = 'match %.2f %+d,%+d; warp %s' % (score, dx, dy, '%+d,%+d' % (p[2], p[3]) if p else '-')
            spot = place(mj, tx, ty)
            if spot is None:
                print('!! no spot', name, item, tx, ty)
                continue
            print('%-40s %-18s crystal %2d,%2d -> %2d,%2d (%s) -> %2d,%2d elev %d' %
                  (ours, item, x, y, tx, ty, how, spot[0], spot[1], spot[2]))
            # (elevation 0 matches the player at any height: on the shore, surfing, beside a ledge)
            mj['bg_events'].append({'type': 'hidden_item', 'x': spot[0], 'y': spot[1], 'elevation': 0,
                                    'item': item, 'flag': flag})
            changed = True
        if changed and not dry:
            json.dump(mj, open(mpath, 'w'), indent=2)
            open(mpath, 'a').write('\n')
    hdr = os.path.join(REPO, 'include/constants/kai_regions.h')
    text = open(hdr).read()
    block = '// Crystal\'s hidden items (case 131, tools/quetzal_port/gen_hidden_items.py)\n' + ''.join(
        '#define %-34s 0x%04X   // %s\n' % (f, v, c) for f, v, c in flags)
    text = re.sub(r'// Crystal\'s hidden items \(case 131[^\n]*\n(#define FLAG_JOHTO_HIDDEN_ITEM[^\n]*\n)*', '', text)
    text = text.replace('\n#endif // GUARD_CONSTANTS_KAI_REGIONS_H', block + '\n#endif // GUARD_CONSTANTS_KAI_REGIONS_H')
    if not dry:
        open(hdr, 'w').write(text)


if __name__ == '__main__':
    main()
