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
                       'pos': tuple(v / UNIT for v in struct.unpack_from('<3i', f, o + 0x30)),
                       'camera': struct.unpack_from('<I', f, o + 0x3C)[0]})
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
    if shape['func'] in (1, 2):
        c = shape['anchor']
        ax, az, bx, bz = a[0] - c[0], a[2] - c[2], b[0] - c[0], b[2] - c[2]
        if abs(ax * bz - az * bx) < 1.0 and ax * bx + az * bz < 0:
            # (the anchor on the way between the ends: an arc standing up, BRYCEN's ramps' humps: straight on the
            # ground, its height the short way round the circle through the ends)
            dx, dz = b[0] - a[0], b[2] - a[2]
            n = math.hypot(dx, dz) or 1.0
            rx, rz = -dz / n, dx / n
            ua, ub = -math.hypot(ax, az), math.hypot(bx, bz)
            u = ua + (ub - ua) * t
            ra, rb = math.hypot(ua, a[1] - c[1]), math.hypot(ub, b[1] - c[1])
            rad = ra + (rb - ra) * t
            up = 1.0 if (a[1] - c[1]) + (b[1] - c[1]) >= 0 else -1.0
            y = c[1] + up * math.sqrt(max(0.0, rad * rad - u * u))
            return a[0] + dx * t + rx * side, y, a[2] + dz * t + rz * side
        # (an arc about the anchor, in x / z: the angle and the radius between the ends')
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


def cells(rail, lines=None):
    """{(tile x, tile z): [heights]} every cell of every line (or of LINES) covers (world tiles: units / 16)"""
    r = load(rail)
    out = {}
    for li, line in enumerate(r['lines']):
        if lines is not None and li not in lines:
            continue
        length = max(1, line['length']) * CELL
        last = {}
        for off in range(max(1, line['length'])):
            half = int(_width_at(r, li, off * CELL, length) // CELL)
            for s in range(-half, half + 1):
                # (sub-steps: an arc or a slant covers every tile it passes over)
                for f in (0.0, 0.5, 1.0):
                    x, y, z = place(r, li, off + f, s)
                    c = (int(math.floor(x / CELL)), int(math.floor(z / CELL)))
                    prev = last.get(s)
                    if prev is not None and abs(prev[0] - c[0]) == 1 and abs(prev[1] - c[1]) == 1:
                        # (a slant stepping a tile both ways at once: the tile between too, so a walker on the
                        # grid can follow it, DRAGONSPIRAL's spiral ramp)
                        _add(out, (c[0], prev[1]), (prev[2] + y) / 2)
                    _add(out, c, y)
                    last[s] = (c[0], c[1], y)
    return out


def _add(out, c, y):
    hs = out.setdefault(c, [])
    if all(abs(h - y) > 4 for h in hs):
        hs.append(y)


# Black's rail cameras (ov021 d_021DDDB8: 20 functions): FixedAngle (1) and its kin (15 at the centre, 17 / 19
# shifted) hold a pitch, a yaw (1/65536 turns) and a distance (fx32); the Blend kinds (2, 14, 16, 18) go from the line's
# start point's camera to its end point's along it; YawFromPoint (8) turns to face away from a point. Others keep the
# zone's own camera.
FIXED = {1, 15, 17, 19}
BLEND = {2, 14, 16, 18}
YAW_FROM_POINT = 8
ORBIT = {5, 6, 7}        # (BlendOrbit, Orbit, OrbitMovingPoint: a pitch, a distance, a centre)
EYE_TARGET = {3, 4}      # (FixedEyeTarget, BlendEyeTarget: an eye and a target)


@functools.lru_cache(None)
def _cameras(rail):
    f = narc.arc(RAIL_ARC)[rail]
    hdr = struct.unpack_from('<7I', f, 0)
    return [struct.unpack_from('<I8i', f, hdr[4] + k * 0x24) for k in range((hdr[5] - hdr[4]) // 0x24)]


def _fixed(rail, cam):
    cams = _cameras(rail)
    if cam >= len(cams) or cams[cam][0] not in FIXED:
        return None
    c = cams[cam]
    return c[1] & 0xFFFF, c[2] & 0xFFFF, c[3] / UNIT


def camera_at(rail, li, off_cells, side_cells):
    """(pitch, yaw, distance) of Black's camera at a cell of line LI, or None (the zone's own)"""
    r, cams = load(rail), _cameras(rail)
    line = r['lines'][li]
    cam = line['camera']
    if cam >= len(cams):
        return None
    kind = cams[cam][0]
    if kind in FIXED:
        return _fixed(rail, cam)
    if kind in BLEND:
        a = _fixed(rail, r['points'][line['start']].get('camera', NONE)) if 'camera' in r['points'][line['start']] else None
        b = _fixed(rail, r['points'][line['end']].get('camera', NONE)) if 'camera' in r['points'][line['end']] else None
        if a is None or b is None:
            return a or b
        t = min(1.0, max(0.0, off_cells / max(1, line['length'])))
        dy = ((b[1] - a[1] + 0x8000) & 0xFFFF) - 0x8000   # (the short way round)
        return (round(a[0] + (b[0] - a[0]) * t) & 0xFFFF, round(a[1] + dy * t) & 0xFFFF, a[2] + (b[2] - a[2]) * t)
    if kind in ORBIT:   # (about a centre: the eye out beyond the player from it, the city behind the player's back)
        c = cams[cam]
        x, _, z = place(r, li, off_cells, side_cells)
        cx, cz = c[3] / UNIT, c[5] / UNIT
        yaw = int(math.atan2(x - cx, z - cz) / (2 * math.pi) * 65536) & 0xFFFF
        dist = max(237.0, min(c[2] / UNIT - math.hypot(x - cx, z - cz), 420.0))
        return c[1] & 0xFFFF, yaw, dist
    if kind in EYE_TARGET:   # (a fixed eye on a fixed target: its direction kept, at the zone's distance)
        c = cams[cam]
        ex, ey, ez, tx, ty, tz = (v / UNIT for v in c[1:7])
        yaw = int(math.atan2(ex - tx, ez - tz) / (2 * math.pi) * 65536) & 0xFFFF
        pitch = int(math.atan2(ey - ty, math.hypot(ex - tx, ez - tz)) / (2 * math.pi) * 65536) & 0xFFFF
        return pitch, yaw, 237.0
    if kind == YAW_FROM_POINT:
        c = cams[cam]
        x, _, z = place(r, li, off_cells, side_cells)
        px, pz = c[3] / UNIT, c[4] / UNIT
        yaw = int(math.atan2(x - px, z - pz) / (2 * math.pi) * 65536) & 0xFFFF
        return c[1] & 0xFFFF, yaw, c[2] / UNIT
    return None


def tile_cameras(rail):
    """{(tile x, tile z): (pitch, yaw, distance)} of every rail tile with a camera of Black's"""
    r = load(rail)
    out = {}
    for li, line in enumerate(r['lines']):
        length = max(1, line['length']) * CELL
        for off in range(max(1, line['length'])):
            half = int(_width_at(r, li, off * CELL, length) // CELL)
            cam = camera_at(rail, li, off, 0)
            if cam is None:
                continue
            for s in range(-half, half + 1):
                x, _, z = place(r, li, off, s)
                out.setdefault((int(math.floor(x / CELL)), int(math.floor(z / CELL))), cam)
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
