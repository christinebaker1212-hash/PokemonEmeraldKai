#!/usr/bin/env python3
"""UNOVA's TOWN MAP and FLY (src/region_map.c's UNOVA page, src/unova/u3d_townmap.c), from Black's own town map:

- Black's town records (a/0/8/6: 61 rows of 27 u16, ov072 TOWNMAP_TownGetField): 0 the zone, 2 / 3 its point on the
  map, 11 its kind (0 a city, 1 a route, 2 a place, 3 / 4 / 5 the LEAGUE, BLACK CITY and LIBERTY GARDEN's kinds),
  12 FLY can go there, 15 the flag Black sets on arriving (FLY's: the town drawn as visited), 16 a flag that has to
  be set before it shows (LIBERTY GARDEN's)
- the picture: Black's top screen town map (a/0/8/5: BG chars 7 / screen 16 under chars 5 / screen 17, 256-colour
  extended palettes from file 0), 256 x 168, set into the game's 28 x 15 cell grid at 5/7 its size (one page)
- each place a cell of the grid (its point scaled likewise; a city wins a cell over a route or a place)
- FLY: each FLY town lands in front of its POKéMON CENTER's door (the CENTER: the zone whose person runs Black's
  nurse script 2100), a heal location appended to src/data/heal_locations.json after SINNOH's

    python3 -I tools/unova/gen_townmap.py       (run from the repository after gen_maps.py; safe to run again)

Writes src/data/region_map/unova_townmap.h and the heal locations. The picture is built into unova/black_gfx.pak by
build_gfx.py (picture() below), never committed.
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402
import zones as zr  # noqa: E402
import events as ev  # noqa: E402
import gen_maps  # noqa: E402

TOWN_ARC, ART_ARC = 'a/0/8/6', 'a/0/8/5'
ROW = 27
COLS, ROWS = 28, 15                  # the game's grid (region_map.c MAP_WIDTH / MAP_HEIGHT)
GRID_X, GRID_Y = 8, 16               # its top-left on the screen (MAPCURSOR_X_MIN / _Y_MIN cells of 8)
BW, BH = 256, 168                    # Black's map (the top screen's map part)
S = 5 / 7                            # Black's map onto the grid: 168 rows into 120
OX = GRID_X + (COLS * 8 - BW * S) / 2
OY = GRID_Y
TEX_W, TEX_H = 480, 320              # the picture, the full view's screen at twice its resolution
NURSE_SCRIPT = 2100
PRIORITY = {0: 0, 3: 1, 4: 1, 5: 1, 2: 2, 1: 3}


def rows():
    d = narc.arc(TOWN_ARC)[0]
    return [struct.unpack_from('<%dH' % ROW, d, i * ROW * 2) for i in range(len(d) // (ROW * 2))]


def cell(x, y):
    return int((OX + x * S) // 8) - GRID_X // 8, int((OY + y * S) // 8) - GRID_Y // 8


def _chunk(d, magic):
    o, n = struct.unpack_from('<HH', d, 12)
    for _ in range(n):
        m, size = d[o:o + 4], struct.unpack_from('<I', d, o + 4)[0]
        if m == magic:
            return d[o:o + size]
        o += size
    raise KeyError(magic)


def _black_map():
    """Black's map as RGB555 rows (BW x BH)"""
    a = narc.arc(ART_ARC)
    pc = _chunk(a[0], b'TTLP')
    size = struct.unpack_from('<I', pc, 16)[0]
    pal = [struct.unpack_from('<H', pc, 24 + i * 2)[0] for i in range(size // 2)]
    img = [[pal[0] & 0x7FFF] * BW for _ in range(BH)]
    for ci, si in ((7, 0x10), (5, 0x11)):
        cc = _chunk(a[ci], b'RAHC')
        depth = struct.unpack_from('<I', cc, 12)[0]
        csize, coff = struct.unpack_from('<II', cc, 24)
        g = cc[8 + coff:8 + coff + csize]
        sc = _chunk(a[si], b'NRCS')
        w, h, _, ssize = struct.unpack_from('<HHII', sc, 8)
        e = struct.unpack_from('<%dH' % (ssize // 2), sc, 20)
        for ty in range(min(h, BH + 7) // 8):
            for tx in range(w // 8):
                v = e[ty * (w // 8) + tx]
                t, hf, vf, p = v & 0x3FF, v >> 10 & 1, v >> 11 & 1, v >> 12
                for y in range(8):
                    if ty * 8 + y >= BH:
                        break
                    for x in range(8):
                        sx, sy = (7 - x if hf else x), (7 - y if vf else y)
                        if depth == 4:
                            c = g[t * 64 + sy * 8 + sx]
                            k = p * 256 + c
                        else:
                            b = g[t * 32 + sy * 4 + sx // 2]
                            c = (b >> (4 * (sx & 1))) & 15
                            k = p * 16 + c
                        if c and k < len(pal):
                            img[ty * 8 + y][tx * 8 + x] = pal[k] & 0x7FFF
    return img


def picture():
    """The TOWN MAP picture as RGB555 rows (TEX_W x TEX_H): Black's map in the full view's screen space at twice its
    resolution, each pixel the box of Black's pixels it covers (5/7 of a pixel a texel) blended"""
    src = _black_map()

    def rgb(c):
        return c & 31, c >> 5 & 31, c >> 10 & 31
    out = []
    for Y in range(TEX_H):
        row = []
        for X in range(TEX_W):
            r = g = b = 0.0
            for oy in (0.25, 0.75):
                for ox in (0.25, 0.75):
                    fx = ((X + ox) / 2 - OX) / S - 0.5
                    fy = ((Y + oy) / 2 - OY) / S - 0.5
                    fx = min(max(fx, 0.0), BW - 1.0)
                    fy = min(max(fy, 0.0), BH - 1.0)
                    x0, y0 = int(fx), int(fy)
                    x1, y1 = min(x0 + 1, BW - 1), min(y0 + 1, BH - 1)
                    ax, ay = fx - x0, fy - y0
                    for (px, py, wgt) in ((x0, y0, (1 - ax) * (1 - ay)), (x1, y0, ax * (1 - ay)),
                                          (x0, y1, (1 - ax) * ay), (x1, y1, ax * ay)):
                        cr, cg, cb = rgb(src[py][px])
                        r += cr * wgt / 4
                        g += cg * wgt / 4
                        b += cb * wgt / 4
            row.append(int(r + 0.5) | int(g + 0.5) << 5 | int(b + 0.5) << 10)
        out.append(row)
    return out


def main():
    maps = gen_maps.zone_maps()
    zl = zr.zones()
    by_zone = {}
    for m in maps:
        by_zone.setdefault(m['zone'], []).append(m)
    mapjson = {}
    for m in maps:
        p = 'data/maps/%s/map.json' % m['name']
        if os.path.exists(p):
            mapjson[m['id']] = json.load(open(p))

    def sec(zone):
        ms = by_zone.get(zone)
        return 'MAPSEC_UNOVA_' + gen_maps.snake(ms[0]['place']) if ms else None

    def center_zone(zone):
        """the POKéMON CENTER a door of ZONE's maps leads into (its nurse's script)"""
        for m in by_zone.get(zone, []):
            for w in mapjson.get(m['id'], {}).get('warp_events', []):
                for t in maps:
                    if t['id'] == w['dest_map'] and not t['outdoor']:
                        try:
                            people = ev.events(t['zone'])['people']
                        except Exception:
                            continue
                        if any(p['script'] == NURSE_SCRIPT for p in people):
                            return m['id'], w['x'], w['y'] + 1
        # (no CENTER: NUVEMA, where Black lands at home: the zone's own start spot on the world's matrix, else its
        # first door)
        z = zl[zone]
        if z['matrix'] == 0:
            sx, _, sz = z['start']
            for m in by_zone.get(zone, []):
                x0, z0, x1, z1 = m['rect']
                if x0 * 32 <= sx < (x1 + 1) * 32 and z0 * 32 <= sz < (z1 + 1) * 32:
                    return m['id'], sx - x0 * 32, sz - z0 * 32
        for m in by_zone.get(zone, []):
            for w in mapjson.get(m['id'], {}).get('warp_events', []):
                return m['id'], w['x'], w['y'] + 1
        return None

    layout = [['MAPSEC_NONE'] * COLS for _ in range(ROWS)]
    prio = [[99] * COLS for _ in range(ROWS)]
    places, flies, arrive, heals = [], [], [], []
    for r in rows():
        zone, kind = r[0], r[11]
        s = sec(zone)
        if s is None:
            continue
        cx, cy = cell(r[2], r[3])
        cx, cy = min(max(cx, 0), COLS - 1), min(max(cy, 0), ROWS - 1)
        if PRIORITY.get(kind, 3) < prio[cy][cx]:
            prio[cy][cx] = PRIORITY.get(kind, 3)
            layout[cy][cx] = s
        if s not in [p[0] for p in places]:
            places.append((s, cx, cy))
        if r[15] != 0xFFFF:
            arrive.append((zone, r[15]))
        if r[12] and r[15] != 0xFFFF:
            door = center_zone(zone)
            if door is None:
                print('no POKéMON CENTER door for %s (zone %d)' % (s, zone))
                continue
            heal = 'HEAL_LOCATION_' + s[len('MAPSEC_'):]
            if heal in [h['id'] for h in heals]:
                continue
            heals.append({'id': heal, 'map': door[0], 'x': door[1], 'y': door[2]})
            flies.append((s, heal, r[15], r[16]))
    path = 'src/data/region_map/unova_townmap.h'
    with open(path, 'w') as f:
        f.write('// Generated by tools/unova/gen_townmap.py. Do not edit.\n')
        f.write("// UNOVA's TOWN MAP page (region_map.c QZMAP_UNOVA) and FLY spots, from Black's own town map\n")
        f.write('static const mapsec_u16_t sUnovaMapLayout[MAP_HEIGHT][MAP_WIDTH] = {\n')
        for y in range(ROWS):
            f.write('    {%s},\n' % ', '.join(layout[y]))
        f.write('};\n\n')
        f.write('static const struct QzMapPlace sUnovaMapPlaces[] = {\n')
        for s, cx, cy in places:
            f.write('    {%s, QZMAP_UNOVA, %d, %d, 1, 1},\n' % (s, cx, cy))
        f.write('};\n\n')
        f.write('// (flag: Black\'s own arrival flag, u3d_vm.c U3D_Vm_FlagOn; the lock: a Black flag that must be set '
                'first, 0xFFFF none)\n')
        f.write('static const struct QzFlyDest sUnovaFlyDests[] = {\n')
        for s, heal, flag, _lock in flies:
            f.write('    {%s, QZMAP_UNOVA, %s, 0x%X},\n' % (s, heal, flag))
        f.write('};\n\n')
        f.write('static const u16 sUnovaFlyLocks[] = {%s};\n\n' % ', '.join('0x%X' % lk for *_x, lk in flies))
        f.write('// Black\'s arrival flags: the zone, the flag (set on entering it, as Black\'s field does)\n')
        f.write('static const u16 sUnovaArrive[][2] = {\n')
        for zone, flag in arrive:
            f.write('    {%d, 0x%X},\n' % (zone, flag))
        f.write('};\n')
    p = 'src/data/heal_locations.json'
    data = json.load(open(p))
    data['heal_locations'] = [h for h in data['heal_locations'] if not h['map'].startswith('MAP_UNOVA_')] + heals
    with open(p, 'w') as f:
        json.dump(data, f, indent=2)
        f.write('\n')
    print('%s: %d places, %d FLY spots, %d arrival flags' % (path, len(places), len(flies), len(arrive)))


if __name__ == '__main__':
    main()
