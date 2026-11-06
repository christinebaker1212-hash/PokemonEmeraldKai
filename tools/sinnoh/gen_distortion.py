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
        self.values['GIRATINA_SHADOW_ANIM_B4F_1'], self.values['GIRATINA_SHADOW_ANIM_B4F_2'] = 0, 1   # (system_flags.h)
        self.values['TRUE'], self.values['FALSE'], self.values['NULL'] = 1, 0, 0
        for k, n in enumerate(('FACE_UP', 'FACE_DOWN', 'FACE_LEFT', 'FACE_RIGHT')):   # (constants/map_object.h's order)
            self.values[n] = k
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
    evs, named = events(c)
    return dict(plats=plats, paths=paths, conns=conns, props=props, c=c, events=evs, named=named, root=root,
                people=people(c, root),
                falls=[(c.eval(t['mapHeaderID']), c.eval(t['tileX']), c.eval(t['tileZ']), c.eval(t['flagIndex']))
                       for t in c.initializer('sBoulderFallLocations') if c.eval(t['mapHeaderID']) != 0xFFFF],
                shadows=[flatten(c, param_fields(c, 'DistWorldGiratinaShadowTemplate'), t)
                         for t in c.initializer('sGiratinaShadowExternal')])


def param_fields(c, typename):
    """A params struct's fields in order, nested structs and VecFx32s flattened ([(name, count)])"""
    m = c.re.search(r'typedef struct %s \{(.*?)\} %s;' % (typename, typename), c.clean, c.re.S)
    out = []
    for line in m.group(1).split(';'):
        line = line.strip()
        if not line:
            continue
        typ, name = line.rsplit(None, 1)
        if typ.startswith('CmdParams'):
            out.append((name, param_fields(c, typ)))
        elif typ == 'VecFx32':
            out.append((name, 3))
        else:
            out.append((name, 1))
    return out


def flatten(c, fields, init):
    vals = []
    for k, (name, n) in enumerate(fields):
        if isinstance(init, dict):
            v = init.get(name, 0)
        else:   # (positional: a struct written without designators, { { .x = ... } })
            v = init[k] if isinstance(init, list) and k < len(init) else 0
        if isinstance(n, list):
            vals += flatten(c, n, v if isinstance(v, dict) else {})
        elif n == 3:
            vals += [c.eval(x) for x in (v if isinstance(v, list) else [0, 0, 0])]
        else:
            vals.append(c.eval(v) if not isinstance(v, (list, dict)) else 0)
    return vals


# each command kind's params struct (overlay009's handlers cast them so)
CMD_PARAMS = {
    'EVENT_CMD_SET_MAP_OBJECT_ANIMATION': 'CmdParamsSetMapObjectAnimation',
    'EVENT_CMD_MOVE_PLATFORM': 'CmdParamsMovePlatform',
    'EVENT_CMD_ADD_MAP_OBJECT_WITH_LOCAL_ID': 'CmdParamsAddMapObjWithLocalID',
    'EVENT_CMD_DELETE_MAP_OBJECT_WITH_LOCAL_ID': 'CmdParamsDeleteMapObjWithLocalID',
    'EVENT_CMD_CASCADE_UP': 'CmdParamsCascadeUp',
    'EVENT_CMD_CASCADE_DOWN': 'CmdParamsCascadeDown',
    'EVENT_CMD_START_SCRIPT': 'CmdParamsStartScript',
    'EVENT_CMD_SET_DISTORTION_WORLD_PROGRESS': 'CmdParamsSetDistortionWorldProgress',
    'EVENT_CMD_SET_GIRATINA_ANIMATION_FLAG': 'CmdParamsSetGiratinaAnimationFlag',
    'EVENT_CMD_SET_PUZZLE_FLAG': 'CmdParamsPuzzleFlag',
    'EVENT_CMD_CLEAR_PUZZLE_FLAG': 'CmdParamsPuzzleFlag',
    'EVENT_CMD_SHOW_GIRATINA_SHADOW': 'DistWorldGiratinaShadowTemplate',   # (tile x y z, turn, sound, scale x3, delta x3, steps)
}


def events(c):
    """overlay009's sMapEvents: [(map header, x, y, z, condition, value, [(kind, params...)])], and the commands
    reached by name from its code (the B4F / B5F waterfalls: sMapEventB4F_Waterfall...)"""
    # (LOCALID_PLAYER and the movement actions the events use: the game's own values here)
    c.values.setdefault('LOCALID_PLAYER', 0xFF)
    for k, d in enumerate(('NORTH', 'SOUTH', 'WEST', 'EAST')):
        c.values['MOVEMENT_ACTION_JUMP_DISTORTION_WORLD_' + d] = 0x1000 + k
    def cmds_of(name):
        out = []
        for cmd in c.initializer(name):
            if isinstance(cmd, dict):
                kind, params = cmd['kind'], cmd.get('params', 'NULL')
            else:
                kind, params = cmd[0], cmd[1]
            kname = kind.strip()
            if kname == 'EVENT_CMD_END':
                break
            vals = []
            if kname in CMD_PARAMS and params.strip() not in ('NULL', '0'):
                vals = flatten(c, param_fields(c, CMD_PARAMS[kname]), c.initializer(params.strip().lstrip('&')))
            out.append((c.values[kname], vals))
        return out
    evs = []
    for entry in c.initializer('sMapEvents'):
        hid = c.eval(entry[0] if isinstance(entry, list) else entry['mapHeaderID'])
        if hid == 0xFFFF:
            break
        lst = entry[1] if isinstance(entry, list) else entry['events']
        for e in c.initializer(lst.strip()):
            if isinstance(e, list):
                continue   # (the terminator)
            evs.append((hid, c.eval(e['tileX']), c.eval(e['tileY']), c.eval(e['tileZ']), c.eval(e['flagCond']),
                        c.eval(e['flagCondVal']), cmds_of(e['cmds'].strip())))
    named = {}
    for n in ('sMapEventB4F_Waterfall', 'sMapEventB5F_Waterfall'):
        try:
            named[n] = cmds_of(n)
        except KeyError:
            pass
    return evs, named


DW_ITEM_FLAGS = 0x3C00   # an item ball's flag here (free: the trainers' flags end at 0x391F, the legendaries' start at 0x3D00)


# looks the sprite table leaves out here: the trio's DISTORTION WORLD copies are their own, GIRATINA's ORIGIN FORME
# stands as its ALTERED one (this game has one GIRATINA overworld sprite), INVISIBLE ones are spawned hidden
DW_LOOKS = {'DIST_WORLD_B1F_MESPRIT': 'MESPRIT', 'DIST_WORLD_B6F_AZELF': 'AZELF', 'DIST_WORLD_B6F_MESPRIT': 'MESPRIT',
            'DIST_WORLD_B6F_UXIE': 'UXIE', 'GIRATINA_ORIGIN': 'GIRATINA_ALTERED'}


def people(c, root, scripts_inc=None):
    """overlay009's sMapObjectEvents: [(map header, local id, this game's look, x, z, height tile, facing, condition,
    value, hide flag, invisible, script label or None)] (spawned by s3d_distortion.c as Platinum adds them; their
    scripts the converted SinnohP_<map file>_<n>: gen_scripts.py converts every one this lists)"""
    import sprites
    ids_ = header_ids(root)
    names = {i: n for n, i in ids_.items()}
    out = []
    for entry in c.initializer('sMapObjectEvents'):
        hid = c.eval(entry[0] if isinstance(entry, list) else entry['mapHeaderID'])
        if hid == 0xFFFF:
            break
        lst = (entry[1] if isinstance(entry, list) else entry['objEvents']).strip()
        for ref in c.initializer(lst):
            ref = ref.strip()
            if ref in ('NULL', '0'):
                continue
            o = c.initializer(ref.lstrip('&'))
            e = o['objEvent']
            gfx = e['graphicsID'].strip()[len('OBJ_EVENT_GFX_'):]
            invisible = gfx == 'INVISIBLE'
            look = 'OBJ_EVENT_GFX_MAN_1' if invisible else sprites.sprite(DW_LOOKS.get(gfx, gfx))
            assert look, 'gen_distortion: no look for ' + gfx
            low = names[hid][len('MAP_HEADER_'):].lower()
            m = c.re.match(r'SCRIPT_ID\(VISIBLE_ITEMS,\s*(\d+)\)', e['script'].strip())
            hide = 0
            if m:   # an item ball: the game's own item script (data/scripts/sinnoh_items.inc), its own flag
                import items
                ball = items.ball(root, items.VISIBLE_BASE + int(m.group(1)))
                lab = items.script_label(*ball) if ball else None
                hide = DW_ITEM_FLAGS + len([q for q in out if q[-3]])
            elif gfx == 'STRENGTH_BOULDER':   # (the game's own STRENGTH script, as gen_maps.py's OBSTACLES)
                lab = 'EventScript_StrengthBoulder'
            else:
                script = c.eval(e['script'])
                lab = 'SinnohP_%s_%d' % (low, script) if script else None
            out.append((hid, c.eval(e['localID']), look, c.eval(e['x']), c.eval(e['z']), c.eval(e['y']) // 4096 // 16,
                        c.eval(e['dir']), c.eval(o['flagCond']), c.eval(o['flagCondVal']), hide, invisible, lab))
    return out



def c_kind_shadow(tbl):
    return tbl['c'].values['EVENT_CMD_SHOW_GIRATINA_SHADOW']


def event_scripts(c, root, evs):
    """[(map header, script number, label)] the events' START_SCRIPT commands run (Platinum's scripts of that floor)"""
    names = {i: n for n, i in header_ids(root).items()}
    kind = c.values['EVENT_CMD_START_SCRIPT']
    out = set()
    for hid, x, y, z, cond, val, lst in evs:
        for k, vals in lst:
            if k == kind and vals:
                out.add((hid, vals[0], 'SinnohP_%s_%d' % (names[hid][len('MAP_HEADER_'):].lower(), vals[0])))
    b6f = header_ids(root)['MAP_HEADER_DISTORTION_WORLD_B6F']
    for n in (5, 6, 7):   # (a boulder in its pit: DistWorldFallingBoulder_TickToCorrectPit's scripts)
        out.add((b6f, n, 'SinnohP_distortion_world_b6f_%d' % n))
    return sorted(out)


def script_labels(root):
    """The converted scripts the DISTORTION WORLD's people and events use (gen_scripts.py converts them), none without
    the decomp"""
    try:
        c = CTables(root)
    except (OSError, KeyError):
        return set()
    evs, named = events(c)
    return {p[-1] for p in people(c, root) if p[-1] and p[-1].startswith('SinnohP_')} | \
        {e[2] for e in event_scripts(c, root, evs)}


def write(out, data):
    o = ['// SINNOH: Platinum\'s DISTORTION WORLD data (tools/sinnoh/gen_distortion.py from the decomp): not to be '
         'edited, never committed', '#define DW_HAVE_DATA %d' % (1 if data else 0)]
    if not data:
        o += ['static const struct DwFloor sDwFloors[1];', 'static const struct DwPlatform sDwPlatforms[1];',
              'static const struct DwJump sDwJumps[1];', 'static const struct DwCam sDwCams[1];',
              'static const u16 sDwAttrs[1][1024];', '#define DW_FLOOR_COUNT 0',
              'static const struct DwMoving sDwMoving[1];', '#define DW_MOVING_COUNT 0',
              'static const struct DwPath sDwPaths[1];', 'static const struct DwProp sDwProps[1];',
              'static const u16 sDwConns[1][3];', '#define DW_CONN_COUNT 0', '#define DW_PLATFORM_FLAGS_START 0',
              'static const struct DwEvent sDwEvents[1];', '#define DW_EVENT_COUNT 0', 'static const struct DwCmd sDwCmds[1];',
              '#define DW_CMDS_B4F_WATERFALL 0xFFFF', '#define DW_CMDS_B5F_WATERFALL 0xFFFF', '#define DW_VAR_PROGRESS 0x4000',
              '#define DW_FLAG_PUZZLE_FINISHED 0', '#define DW_PLATFORM_FLAG_INVALID 11', '#define DW_FLAG_STEPPING_STONES 0',
              'static const struct { u16 hid; s16 x, z; s16 flag; } sDwFalls[1];', '#define DW_FALL_COUNT 0',
              'static const struct DwPerson sDwPeople[1];', '#define DW_PEOPLE_COUNT 0',
              'static const struct DwCmd sDwShadows[1];', '#define DW_SHADOW_COUNT 0',
              '#define DW_PROP_GIRATINA_SHADOW 0',
              'static const struct { u16 hid, n; const u8 *script; } sDwEventScripts[1];', '#define DW_EVENT_SCRIPT_COUNT 0']
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
    # the floors' events (overlay009's sMapEvents): tile, condition and its value, then its commands
    cmds = []
    def add_cmds(lst):
        first = len(cmds)
        cmds.extend(lst)
        return first, len(lst)
    o.append('#define DW_EVENT_COUNT %d' % len(tbl['events']))
    o.append('static const struct DwEvent sDwEvents[] = {')
    for hid, x, y, z, cond, val, lst in tbl['events']:
        f, n = add_cmds(lst)
        o.append('    {%d, {%d, %d, %d}, %d, %d, %d, %d},' % (hid, x, y, z, cond, val, f, n))
    o.append('};')
    for name, macro in (('sMapEventB4F_Waterfall', 'B4F_WATERFALL'), ('sMapEventB5F_Waterfall', 'B5F_WATERFALL')):
        lst = tbl['named'].get(name)
        if lst is None:
            o.append('#define DW_CMDS_%s 0xFFFF' % macro)
        else:
            f, n = add_cmds(lst)
            o.append('#define DW_CMDS_%s %d   // (%d commands)' % (macro, f, n))
    o.append('static const struct DwCmd sDwCmds[] = {')
    for kind, vals in cmds:
        vals = (vals + [0] * 14)[:14]
        o.append('    {%d, {%s}},' % (kind, ', '.join(str(v) for v in vals)))
    o.append('};')
    import ids
    root = tbl['root']
    o.append('#define DW_VAR_PROGRESS 0x%X   // (Platinum\'s VAR_DISTORTION_WORLD_PROGRESS: tools/sinnoh/ids.py)' % ids.var(root, 'VAR_DISTORTION_WORLD_PROGRESS'))
    o.append('#define DW_FLAG_PUZZLE_FINISHED 0x%X' % (ids.flag(root, 'FLAG_DISTORTION_WORLD_PUZZLE_FINISHED') or 0))
    o.append('#define DW_FLAG_STEPPING_STONES 0x%X   // (B6F\'s platform 1 shows: Platinum\'s FLAG_DISTORTION_WORLD_STEPPING_STONES)'
             % (ids.flag(root, 'FLAG_DISTORTION_WORLD_STEPPING_STONES') or 0))
    # where a pushed boulder falls (sBoulderFallLocations: B5F's holes, B6F's pits; the pit's puzzle flag, or none)
    o.append('#define DW_FALL_COUNT %d' % len(tbl['falls']))
    o.append('static const struct { u16 hid; s16 x, z; s16 flag; } sDwFalls[] = {')
    for hid, x, z, fl in tbl['falls']:
        o.append('    {%d, %d, %d, %d},' % (hid, x, z, fl if fl < 0x8000 else -1))
    o.append('};')
    # the scripts the events start (START_SCRIPT: the floor's script by number)
    es = event_scripts(c, tbl['root'], tbl['events'])
    for lab in sorted({e[2] for e in es}):
        o.append('extern const u8 %s[];' % lab)
    o.append('#define DW_EVENT_SCRIPT_COUNT %d' % len(es))
    o.append('static const struct { u16 hid, n; const u8 *script; } sDwEventScripts[] = {')
    for hid, n, lab in es:
        o.append('    {%d, %d, %s},' % (hid, n, lab))
    o.append('};')
    # GIRATINA's shadow flying by for the scripts (sGiratinaShadowExternal: StartDistortionWorldGiratinaShadowEvent)
    o.append('#define DW_SHADOW_COUNT %d' % len(tbl['shadows']))
    o.append('#define DW_PROP_GIRATINA_SHADOW %d' % c.values['PROP_KIND_GIRATINA_SHADOW'])
    o.append('static const struct DwCmd sDwShadows[] = {')
    for v in tbl['shadows']:
        o.append('    {%d, {%s}},' % (c_kind_shadow(tbl), ', '.join(str(x) for x in v)))
    o.append('};')
    # the DISTORTION WORLD's own people (sMapObjectEvents)
    labs = sorted({p[-1] for p in tbl['people'] if p[-1]})
    for lab in labs:
        o.append('extern const u8 %s[];' % lab)
    o.append('#define DW_PEOPLE_COUNT %d' % len(tbl['people']))
    o.append('static const struct DwPerson sDwPeople[] = {')
    for hid, lid, look, x, z, y, d, cond, val, hide, inv, lab in tbl['people']:
        o.append('    {%d, %d, %s, {%d, %d, %d}, %d, %d, %d, 0x%X, %d, %s},' % (hid, lid, look, x, y, z, d, cond, val, hide, inv,
                                                                       lab or 'NULL'))
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
