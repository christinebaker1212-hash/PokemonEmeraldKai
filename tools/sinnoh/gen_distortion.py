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
    return floors, plats, jumps, cams, [struct.unpack('<%dH' % (len(a) // 2), a) for a in attrs], tables(root)


# --------------------------------------------------------------------------- overlay009's own tables (its C source)
OV9 = 'src/overlay009/ov9_02249960.c'


class CTables:
    """overlay009's constant tables, read from its C source: defines, enums and designated initializers"""

    def __init__(self, root):
        import re
        self.re = re
        self.src = open(os.path.join(root, OV9)).read()
        consts = open(os.path.join(root, 'include/constants/distortion_world.h')).read() + \
            open(os.path.join(root, 'include/overlay009/ov9_02249960.h')).read()
        self.ids = header_ids(root)
        self.values = dict(self.ids)
        self.values['FX32_ONE'] = 4096
        self.values['MAP_HEADER_INVALID'] = 0xFFFF
        self.values['TRUE'], self.values['FALSE'], self.values['NULL'] = 1, 0, 0
        self.defs = {}
        for text in (consts, self.src):
            text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
            text = re.sub(r'//[^\n]*', '', text)
            for m in re.finditer(r'^#define\s+(\w+)\s+(.+)$', text, re.M):
                self.defs[m.group(1)] = m.group(2).strip()
            for m in re.finditer(r'enum\s*\w*\s*\{([^}]*)\}', text):
                n = -1
                for item in m.group(1).split(','):
                    item = item.strip()
                    if not item:
                        continue
                    if '=' in item:
                        name, val = [t.strip() for t in item.split('=', 1)]
                        n = self.eval(val)
                    else:
                        name, n = item, n + 1
                    self.values[name] = n
        self.clean = re.sub(r'//[^\n]*', '', re.sub(r'/\*.*?\*/', '', self.src, flags=re.S))

    def eval(self, expr):
        expr = str(expr).strip()
        for _ in range(20):
            def sub(m):
                w = m.group(0)
                if w in self.values:
                    return '(%d)' % self.values[w]
                if w in self.defs:
                    return '(%s)' % self.defs[w]
                return w
            new = self.re.sub(r'\b[A-Za-z_]\w*\b', sub, expr)
            if new == expr:
                break
            expr = new
        expr = self.re.sub(r'\b(0x[0-9A-Fa-f]+|\d+)[uUlL]+\b', r'\1', expr)
        return int(eval(expr.replace('/', '//'), {}))

    def initializer(self, name):
        """NAME's initializer as nested lists / dicts (designated fields by name, [INDEX] = by index)"""
        m = self.re.search(r'\b%s\s*(\[[^\]]*\])?\s*=\s*\{' % name, self.clean)
        if not m:
            raise KeyError(name)
        i = m.end() - 1
        val, _ = self._parse(self.clean, i)
        return val

    def _parse(self, t, i):
        # t[i] == '{'
        i += 1
        items, named = [], {}
        while True:
            while t[i] in ' \t\r\n,':
                i += 1
            if t[i] == '}':
                return (named if named else items), i + 1
            key = None
            m = self.re.match(r'\.(\w+)\s*=\s*', t[i:]) or self.re.match(r'\[([^\]]+)\]\s*=\s*', t[i:])
            if m:
                key = m.group(1)
                i += m.end()
            if t[i] == '{':
                v, i = self._parse(t, i)
            else:
                j, depth = i, 0
                while depth or t[j] not in ',}':
                    depth += t[j] == '('
                    depth -= t[j] == ')'
                    j += 1
                v, i = t[i:j].strip(), j
            if key is None:
                items.append(v)
            else:
                named[key] = v
                if not key.isidentifier():
                    named.setdefault('_order', []).append(key)


def tables(root):
    """The moving platforms by floor, the elevators' paths and the props' models (overlay009's tables)"""
    import gymprops
    c = CTables(root)
    plats = []
    for entry in c.initializer('sMovingPlatformsMapTemplates'):
        hid = c.eval(entry[0])
        for ref in c.initializer(entry[1]):
            ref = ref.strip()
            if ref in ('NULL', '0'):
                continue
            t = c.initializer(ref.lstrip('&'))
            plats.append((hid, c.eval(t['index']), c.eval(t['tileX']), c.eval(t['tileY']), c.eval(t['tileZ']),
                          c.eval(t['elevatorPathIndex']), c.eval(t['elevatorDir']), c.eval(t['destIndex']),
                          c.eval(t['propKind']), c.eval(t['persistedFlag'])))
    paths = []
    for t in c.initializer('sElevatorPlatformPaths'):
        d = [c.eval(v) for v in t['posDelta']]
        paths.append((c.eval(t['index']), c.eval(t['nextIndex']), c.eval(t['finalTileXOffset']),
                      c.eval(t['finalTileYOffset']), c.eval(t['finalTileZOffset']), c.eval(t['changeMapsTileXOffset']),
                      c.eval(t['changeMapsTileYOffset']), c.eval(t['changeMapsTileZOffset']), d[0], d[1], d[2],
                      c.eval(t['persistedFlagToSet']), c.eval(t['persistedFlagToClear'])))
    conns = [(c.eval(t['currID']), c.eval(t['prevID']), c.eval(t['nextID'])) for t in c.initializer('sDistWorldMapConnectionList')]
    # the props' models (field effect models in the pack: gymprops.py 'fx:' names) and their offsets
    models = c.initializer('sProp3DModelNARCIndexByKind')
    offs = c.initializer('sPropInitialPosOffsetByKind')
    scales = c.initializer('sPropScaleByKind')
    props = []
    for k in range(c.values['PROP_KIND_COUNT']):
        kind = [n for n, v in c.values.items() if n.startswith('PROP_KIND_') and v == k and n != 'PROP_KIND_INVALID'][0]
        name = 'fx:' + models[kind].replace('_nsbmd', '.nsbmd')
        num = gymprops.FX_BASE + gymprops.fx_names().index(name) if name in gymprops.fx_names() else 0xFFFF
        o = [c.eval(v) / 4096.0 for v in offs[kind]]
        sc = [c.eval(v) / 4096.0 for v in scales[kind]]
        props.append((num, o, sc[0] or 1.0))
    return dict(plats=plats, paths=paths, conns=conns, props=props, c=c)


def write(out, data):
    o = ['// SINNOH: Platinum\'s DISTORTION WORLD data (tools/sinnoh/gen_distortion.py from the decomp): not to be '
         'edited, never committed', '#define DW_HAVE_DATA %d' % (1 if data else 0)]
    if not data:
        o += ['static const struct DwFloor sDwFloors[1];', 'static const struct DwPlatform sDwPlatforms[1];',
              'static const struct DwJump sDwJumps[1];', 'static const struct DwCam sDwCams[1];',
              'static const u16 sDwAttrs[1][1024];', '#define DW_FLOOR_COUNT 0',
              'static const struct DwMoving sDwMoving[1];', '#define DW_MOVING_COUNT 0',
              'static const struct DwPath sDwPaths[1];', 'static const struct DwProp sDwProps[1];',
              'static const u16 sDwConns[1][3];', '#define DW_CONN_COUNT 0', '#define DW_PLATFORM_FLAGS_START 0']
        open(out, 'w').write('\n'.join(o) + '\n')
        return
    floors, plats, jumps, cams, attrs, tbl = data
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
    # overlay009's tables: the moving platforms by floor (map header, index, tile, elevator path, its way, the
    # platform it becomes on the floor it goes to, prop kind, the flag saying it's here), the elevators' paths,
    # the floors' order and each prop kind's model, offset and scale
    o.append('#define DW_MOVING_COUNT %d' % len(tbl['plats']))
    o.append('static const struct DwMoving sDwMoving[] = {')
    for m in tbl['plats']:
        o.append('    {%d, %d, {%d, %d, %d}, %d, %d, %d, %d, %d},' % m)
    o.append('};')
    o.append('static const struct DwPath sDwPaths[] = {')
    for q in tbl['paths']:
        o.append('    {%d, {%d, %d, %d}, {%d, %d, %d}, {%d, %d, %d}, %d, %d},' % q[1:])
    o.append('};')
    o.append('#define DW_CONN_COUNT %d' % len(tbl['conns']))
    o.append('static const u16 sDwConns[][3] = {')
    for q in tbl['conns']:
        o.append('    {%d, %d, %d},' % q)
    o.append('};')
    c = tbl['c']
    init = (1 << c.values['DIST_WORLD_PLATFORM_FLAG_B4F_1']) | (1 << c.values['DIST_WORLD_PLATFORM_FLAG_B5F_1'])
    o.append('#define DW_PLATFORM_FLAGS_START 0x%x   // (InitPersistedData)' % init)
    o.append('#define DW_PLATFORM_FLAG_INVALID %d' % c.values['DIST_WORLD_PLATFORM_FLAG_INVALID'])
    o.append('static const struct DwProp sDwProps[] = {')
    for num, off, sc in tbl['props']:
        o.append('    {%d, {%.2ff, %.2ff, %.2ff}, %.4ff},' % (num, off[0], off[1], off[2], sc))
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
