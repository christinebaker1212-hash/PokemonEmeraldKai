#!/usr/bin/env python3
"""Black's rail maps (docs/UNOVA_HANDOFF.md "Rail maps"): the zones where Black moves the player along "rails" (lines
between points, each a strip some cells wide: CASTELIA's streets, the bridges, the towers' spirals), laid onto the zone's
tile grid so the game walks them as it walks any map. Read as aletheia's ov021 field_rail_loader.c / field_rail.c read
them: a zone's no-grid resource (arm9's zone -> kind table, then a/1/0/9: u16 rail file, scene area file), the rail file
(a/0/7/9: points 0x70 bytes, lines 0x48, cameras 0x24, line shapes 0x24; positions fx32, 16 units a cell).

A line runs from its start point to its end point, `length` cells long; its shape is straight or an arc about the
shape's anchor (shape function 1); its width (Black's lineWidth, 16 units a cell either side of the middle) goes from
the start point's to the end point's (FIELD_RAIL_LINE_GetWidthAt). A rail place (a person's, a trigger's, a door's) is
a line, a cell along it and a cell across it (BgEventRailPos / RailLocation: railIndex, lineGrid, widthGrid).

    python3 -I tools/unova/rails.py ZONE      (the zone's rails: lines, and the tiles they cover)
"""
import functools
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import narc  # noqa: E402

ARM9_BASE = 0x02004000
KIND_TABLE, KINDS = 0x0209E026, 0x14    # (ZONEDATA_GetSpecialKind: u16 zone 11 bits, kind 5)
RAIL_ZONES, RAIL_ZONE_COUNT = 0x0209E010, 11   # (ZONEDATA_IsRailMap: the zones moved on rails throughout)
RES_ARC, RAIL_ARC = 'a/1/0/9', 'a/0/7/9'
NONE = 0xFFFF
UNIT = 4096.0
CELL = 16.0


@functools.lru_cache(None)
def _arm9():
    return open(os.path.join(os.path.dirname(narc.EXTRACT), 'arm9_dec.bin'), 'rb').read()


@functools.lru_cache(None)
def rail_files():
    """{zone: rail file} for every zone with rails"""
    arm, res = _arm9(), narc.arc(RES_ARC)
    out = {}
    for i in range(KINDS):
        v = struct.unpack_from('<H', arm, KIND_TABLE - ARM9_BASE + 2 * i)[0]
        zone, kind = v & 0x7FF, v >> 11
        if kind < len(res):
            rail = struct.unpack_from('<H', res[kind], 0)[0]
            if rail != NONE:
                out[zone] = rail
    return out


@functools.lru_cache(None)
def rail_map_zones():
    """the zones moved on rails throughout (no grid of their own)"""
    arm = _arm9()
    return set(struct.unpack_from('<%dH' % RAIL_ZONE_COUNT, arm, RAIL_ZONES - ARM9_BASE))


@functools.lru_cache(None)
def load(rail):
    f = narc.arc(RAIL_ARC)[rail]
    hdr = struct.unpack_from('<7I', f, 0)
    points, lines, shapes = [], [], []
    for k in range((hdr[3] - hdr[2]) // 0x70):
        o = hdr[2] + k * 0x70
        points.append({'lines': struct.unpack_from('<4I', f, o), 'widths': struct.unpack_from('<4i', f, o + 0x20),
                       'pos': tuple(v / UNIT for v in struct.unpack_from('<3i', f, o + 0x30))})
    for k in range((hdr[4] - hdr[3]) // 0x48):
        s, e, key, shape, cam, length = struct.unpack_from('<6I', f, hdr[3] + k * 0x48)
        lines.append({'start': s, 'end': e, 'shape': shape, 'camera': cam, 'length': length})
    for k in range((hdr[6] - hdr[5]) // 0x24):
        o = hdr[5] + k * 0x24
        shapes.append({'func': struct.unpack_from('<I', f, o)[0],
                       'anchor': tuple(v / UNIT for v in struct.unpack_from('<3i', f, o + 4))})
    return {'points': points, 'lines': lines, 'shapes': shapes}


def _width_at(r, li, off, length):
    """the line's half width at OFF (units: FIELD_RAIL_LINE_GetWidthAt)"""
    line = r['lines'][li]
    ws = we = -1
    for k in range(4):
        if r['points'][line['start']]['lines'][k] == li:
            ws = r['points'][line['start']]['widths'][k]
        if r['points'][line['end']]['lines'][k] == li:
            we = r['points'][line['end']]['widths'][k]
    if ws < 0:
        ws = we
    if we < 0:
        we = ws
    span = length - CELL
    off = min(off, span)
    return ws + ((we - ws) * off / span if span > 0 else 0)


def place(r, li, off_cells, side_cells):
    """the world position (x, y, z units) of a cell of line LI: OFF_CELLS along it, SIDE_CELLS across (right of the
    way from its start to its end)"""
    line = r['lines'][li]
    a, b = r['points'][line['start']]['pos'], r['points'][line['end']]['pos']
    length = max(1, line['length']) * CELL
    t = min(1.0, max(0.0, off_cells * CELL / length)) if length > CELL else 0.0
    side = side_cells * CELL
    shape = r['shapes'][line['shape']] if line['shape'] < len(r['shapes']) else {'func': 0}
    y = a[1] + (b[1] - a[1]) * t
    if shape['func'] in (1, 2):   # (an arc about the anchor, in x / z: the angle and the radius between the ends')
        c = shape['anchor']
        aa, ab = math.atan2(a[2] - c[2], a[0] - c[0]), math.atan2(b[2] - c[2], b[0] - c[0])
        d = ab - aa
        while d > math.pi:
            d -= 2 * math.pi
        while d < -math.pi:
            d += 2 * math.pi
        ra, rb = math.hypot(a[0] - c[0], a[2] - c[2]), math.hypot(b[0] - c[0], b[2] - c[2])
        ang, rad = aa + d * t, ra + (rb - ra) * t
        rad += side if d > 0 else -side   # (right of the way: outwards on a turn one way, inwards the other)
        return c[0] + rad * math.cos(ang), y, c[2] + rad * math.sin(ang)
    dx, dz = b[0] - a[0], b[2] - a[2]
    n = math.hypot(dx, dz) or 1.0
    rx, rz = -dz / n, dx / n   # (right of the way, x right and z down as the map has them)
    return a[0] + dx * t + rx * side, y, a[2] + dz * t + rz * side


def cells(rail):
    """{(tile x, tile z): [heights]} every cell of every line covers (world tiles: units / 16)"""
    r = load(rail)
    out = {}
    for li, line in enumerate(r['lines']):
        length = max(1, line['length']) * CELL
        for off in range(max(1, line['length'])):
            half = int(_width_at(r, li, off * CELL, length) // CELL)
            for s in range(-half, half + 1):
                # (sub-steps: an arc or a slant covers every tile it passes over)
                for f in (0.0, 0.5):
                    x, y, z = place(r, li, off + f, s)
                    hs = out.setdefault((int(math.floor(x / CELL)), int(math.floor(z / CELL))), [])
                    if all(abs(h - y) > 4 for h in hs):
                        hs.append(y)
    return out


def tile_of(rail, li, along, across):
    """a rail place's tile (world tiles)"""
    r = load(rail)
    if li >= len(r['lines']):
        return None
    x, _, z = place(r, li, along, across)
    return int(math.floor(x / CELL)), int(math.floor(z / CELL))


def _s16(v):
    return v - 0x10000 if v >= 0x8000 else v


def event_tiles(zone, kind, rec):
    """the world tiles of a rail event of ZONE (tools/unova/events.py's records): a person's or a sign's one; a trigger's
    or a door's area (EVENTDATA_SYSTEM_IsPosEventRailHit: a line, cells along from a start for a count, cells across
    from a start for a count)"""
    rail = rail_files().get(zone)
    if rail is None or not rec.get('rail'):
        return []
    if kind == 'people':   # (MmdlHeader's rail place: line, along, across)
        t = tile_of(rail, rec['x'], rec['z'], _s16(rec['y'] & 0xFFFF))
        return [t] if t else []
    if kind == 'bg':       # (BgEventRailPos: u16 line, u16 along, s16 across)
        t = tile_of(rail, rec['x'] & 0xFFFF, (rec['x'] >> 16) & 0xFFFF, _s16(rec['z'] & 0xFFFF))
        return [t] if t else []
    if kind == 'pos':
        line, along, across, n_along, n_across = rec['x'], rec['z'], rec['w'], rec['h'], rec['y']
    else:                  # (a connection: line, along, across, then the counts)
        line, along, across, n_along, n_across = rec['x'], rec['y'], rec['z'], rec['w'], rec['h']
    out = []
    for a in range(max(1, n_along)):
        for c in range(max(1, n_across)):
            t = tile_of(rail, line, along + a, across + c)
            if t and t not in out:
                out.append(t)
    return out


def main():
    zone = int(sys.argv[1])
    rail = rail_files().get(zone)
    if rail is None:
        sys.exit('zone %d has no rails' % zone)
    r = load(rail)
    for li, line in enumerate(r['lines']):
        print('line %d: %d -> %d, %d cells, shape %d' % (li, line['start'], line['end'], line['length'],
                                                      r['shapes'][line['shape']]['func']))
    cs = cells(rail)
    xs, zs = [c[0] for c in cs], [c[1] for c in cs]
    print('rail file %d: %d tiles, x %d-%d, z %d-%d' % (rail, len(cs), min(xs), max(xs), min(zs), max(zs)))


if __name__ == '__main__':
    main()
