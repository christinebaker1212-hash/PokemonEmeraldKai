#!/usr/bin/env python3
"""Platinum's DISTORTION WORLD data for SINNOH (src/sinnoh/s3d_distortion.c), converted from the decomp when the game
is built, never committed: sinnoh/platinum_dw.h.

From res/prebuilt/fielddata/tornworld/tw_arc.narc (member 0: each floor's map header, file and offsets in the stacked
world; members 1..: per floor, its floating platforms, their jump points, the camera's angle zones and ghost props) and
tw_arc_attr.narc (each platform's 32 x 32 tile attributes), read as overlay009 (src/overlay009/ov9_02249960.c:
DistWorldMapInfo, DistWorldMapFile*, DistWorldFloatingPlatformTemplate, ...JumpPointTemplate, ...CameraAngleTemplate)
reads them. Coordinates stay Platinum's: tiles in the stacked world (a floor's own tile + its offset; the height in
tiles from offsetAltitude).

usage: gen_distortion.py POKEPLATINUM_DIR OUT.h | --auto OUT.h (the decomp named in sinnoh/decomp_dir; stubs without)
"""
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

# Platinum's map header -> this game's map (data/maps/map_groups.json names)
FLOORS = [
    ('MAP_HEADER_DISTORTION_WORLD_1F', 'MAP_SINNOH_DISTORTION_WORLD1F'),
    ('MAP_HEADER_DISTORTION_WORLD_B1F', 'MAP_SINNOH_DISTORTION_WORLD_B1F'),
    ('MAP_HEADER_DISTORTION_WORLD_B2F', 'MAP_SINNOH_DISTORTION_WORLD_B2F'),
    ('MAP_HEADER_DISTORTION_WORLD_B3F', 'MAP_SINNOH_DISTORTION_WORLD_B3F'),
    ('MAP_HEADER_DISTORTION_WORLD_B4F', 'MAP_SINNOH_DISTORTION_WORLD_B4F'),
    ('MAP_HEADER_DISTORTION_WORLD_B5F', 'MAP_SINNOH_DISTORTION_WORLD_B5F'),
    ('MAP_HEADER_DISTORTION_WORLD_B6F', 'MAP_SINNOH_DISTORTION_WORLD_B6F'),
    ('MAP_HEADER_DISTORTION_WORLD_B7F', 'MAP_SINNOH_DISTORTION_WORLD_B7F'),
    ('MAP_HEADER_DISTORTION_WORLD_GIRATINA_ROOM', 'MAP_SINNOH_DISTORTION_WORLD_GIRATINA_ROOM'),
    ('MAP_HEADER_DISTORTION_WORLD_TURNBACK_CAVE_ROOM', 'MAP_SINNOH_DISTORTION_WORLD_TURNBACK_CAVE_ROOM'),
]
TW = 'res/prebuilt/fielddata/tornworld/'


def header_ids(root):
    names = [l.strip() for l in open(os.path.join(root, 'generated/map_headers.txt')) if l.strip()]
    return {n: i for i, n in enumerate(names)}


def read(root):
    from tv_data import narc_members
    main = narc_members(os.path.join(root, TW + 'tw_arc.narc'))
    attrs = narc_members(os.path.join(root, TW + 'tw_arc_attr.narc'))
    ids = header_ids(root)
    info = main[0]
    count = struct.unpack('<i', info[:4])[0]
    infos = {}
    for i in range(count):
        hid, fidx, ox, oy, oz = struct.unpack('<IHhhh', info[4 + i * 12:16 + i * 12])
        infos[hid] = (fidx, ox, oy, oz)
    floors, plats, jumps, cams = [], [], [], []
    for pname, kname in FLOORS:
        hid = ids[pname]
        fidx, ox, oy, oz = infos[hid]
        d = main[fidx + 1]
        h = struct.unpack('<5i', d[:20])
        secs, p = [], 20
        for size in h[1:]:
            secs.append(d[p:p + size])
            p += size
        f = dict(kai=kname, hid=hid, off=(ox, oy, oz), plat=len(plats), jump=len(jumps), cam=len(cams))
        if secs[0]:
            n = struct.unpack('<i', secs[0][:4])[0]
            for i in range(n):
                plats.append(struct.unpack('<hH6hHH', secs[0][4 + i * 20:24 + i * 20]))
        if secs[1]:
            n = struct.unpack('<i', secs[1][:4])[0]
            for i in range(n):
                jumps.append(struct.unpack('<Hhi6h3hhhHHhhH', secs[1][4 + i * 40:44 + i * 40]))
        if secs[2]:
            n = struct.unpack('<i', secs[2][:4])[0]
            for i in range(n):
                cams.append(struct.unpack('<6h3Hhi', secs[2][4 + i * 24:28 + i * 24]))
        f['nplat'] = len(plats) - f['plat']
        f['njump'] = len(jumps) - f['jump']
        f['ncam'] = len(cams) - f['cam']
        floors.append(f)
    return floors, plats, jumps, cams, [struct.unpack('<%dH' % (len(a) // 2), a) for a in attrs]


def write(out, data):
    o = ['// SINNOH: Platinum\'s DISTORTION WORLD data (tools/sinnoh/gen_distortion.py from the decomp): not to be '
         'edited, never committed', '#define DW_HAVE_DATA %d' % (1 if data else 0)]
    if not data:
        o += ['static const struct DwFloor sDwFloors[1];', 'static const struct DwPlatform sDwPlatforms[1];',
              'static const struct DwJump sDwJumps[1];', 'static const struct DwCam sDwCams[1];',
              'static const u16 sDwAttrs[1][1024];', '#define DW_FLOOR_COUNT 0']
        open(out, 'w').write('\n'.join(o) + '\n')
        return
    floors, plats, jumps, cams, attrs = data
    o.append('#define DW_FLOOR_COUNT %d' % len(floors))
    o.append('static const struct DwFloor sDwFloors[] = {')
    for f in floors:
        o.append('    {%s, %d, {%d, %d, %d}, %d, %d, %d, %d, %d, %d},' % (
            f['kai'], f['hid'], f['off'][0], f['off'][1], f['off'][2], f['plat'], f['nplat'], f['jump'],
            f['njump'], f['cam'], f['ncam']))
    o.append('};')
    o.append('static const struct DwPlatform sDwPlatforms[] = {')
    for p in plats or [(4, 0, 0, 0, 0, 0, 0, 0, 0, 0)]:
        o.append('    {%d, %d, {%d, %d, %d, %d, %d, %d}, %d, %d},' % p)
    o.append('};')
    o.append('static const struct DwJump sDwJumps[] = {')
    for j in jumps:
        hi, pdir, _, bx, by, bz, sx, sy, sz, dx, dy, dz, rot, steps, axis, inv, face, kind, idx = j
        o.append('    {%d, {%d, %d, %d, %d, %d, %d}, {%d, %d, %d}, %d, %d, %d, %d, %d, %d, %d},' % (
            pdir, bx, by, bz, sx, sy, sz, dx, dy, dz, rot, steps, axis, inv, face, kind,
            idx if idx < 0x8000 else -1))
    o.append('};')
    o.append('static const struct DwCam sDwCams[] = {')
    for c in cams:
        o.append('    {{%d, %d, %d, %d, %d, %d}, {%d, %d, %d}, %d, %d},' % c)
    o.append('};')
    o.append('static const u16 sDwAttrs[%d][1024] = {' % len(attrs))
    for a in attrs:
        o.append('    {' + ','.join('0x%x' % v for v in a) + '},')
    o.append('};')
    open(out, 'w').write('\n'.join(o) + '\n')


def main():
    args = sys.argv[1:]
    if len(args) == 2 and args[0] == '--auto':
        out = args[1]
        try:
            root = open(os.path.join(os.path.dirname(out), 'decomp_dir')).read().strip()
        except OSError:
            root = ''
        data = None
        if root and os.path.exists(os.path.join(root, TW + 'tw_arc.narc')):
            data = read(root)
        write(out, data)
    elif len(args) == 2:
        write(args[1], read(args[0]))
    else:
        sys.exit(__doc__)
    print('%s: DISTORTION WORLD data written' % args[-1])


if __name__ == '__main__':
    main()
