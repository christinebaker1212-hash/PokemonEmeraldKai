"""Map headers, layouts, events and connections (Emerald layout, with Quetzal's u16 graphics ids)."""
from qrom import MAP_GROUPS, NUM_GROUPS, MAPSEC_MAIN, MAPSEC_JOHTO, JOHTO_GROUPS, KANTO_GROUPS, plain

MAP_TYPES = ['MAP_TYPE_NONE', 'MAP_TYPE_TOWN', 'MAP_TYPE_CITY', 'MAP_TYPE_ROUTE', 'MAP_TYPE_UNDERGROUND',
             'MAP_TYPE_UNDERWATER', 'MAP_TYPE_OCEAN_ROUTE', 'MAP_TYPE_UNKNOWN', 'MAP_TYPE_INDOOR',
             'MAP_TYPE_SECRET_BASE']
WEATHER = ['WEATHER_NONE', 'WEATHER_SUNNY_CLOUDS', 'WEATHER_SUNNY', 'WEATHER_RAIN', 'WEATHER_SNOW',
           'WEATHER_RAIN_THUNDERSTORM', 'WEATHER_FOG_HORIZONTAL', 'WEATHER_VOLCANIC_ASH', 'WEATHER_SANDSTORM',
           'WEATHER_FOG_DIAGONAL', 'WEATHER_UNDERWATER', 'WEATHER_SHADE', 'WEATHER_DROUGHT',
           'WEATHER_DOWNPOUR', 'WEATHER_UNDERWATER_BUBBLES', 'WEATHER_ABNORMAL']
WEATHER_EXTRA = {0x14: 'WEATHER_ROUTE119_CYCLE', 0x15: 'WEATHER_ROUTE123_CYCLE'}
BATTLE_SCENES = ['MAP_BATTLE_SCENE_NORMAL', 'MAP_BATTLE_SCENE_GYM', 'MAP_BATTLE_SCENE_MAGMA',
                 'MAP_BATTLE_SCENE_AQUA', 'MAP_BATTLE_SCENE_SIDNEY', 'MAP_BATTLE_SCENE_PHOEBE',
                 'MAP_BATTLE_SCENE_GLACIA', 'MAP_BATTLE_SCENE_DRAKE', 'MAP_BATTLE_SCENE_FRONTIER']
DIRS = {1: 'down', 2: 'up', 3: 'left', 4: 'right', 5: 'dive', 6: 'emerge'}
MAP_SCRIPT_TYPES = {1: 'MAP_SCRIPT_ON_LOAD', 2: 'MAP_SCRIPT_ON_FRAME_TABLE', 3: 'MAP_SCRIPT_ON_TRANSITION',
                    4: 'MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE', 5: 'MAP_SCRIPT_ON_RESUME',
                    6: 'MAP_SCRIPT_ON_DIVE_WARP', 7: 'MAP_SCRIPT_ON_RETURN_TO_FIELD'}
BG_KINDS = {0: 'BG_EVENT_PLAYER_FACING_ANY', 1: 'BG_EVENT_PLAYER_FACING_NORTH', 2: 'BG_EVENT_PLAYER_FACING_SOUTH',
            3: 'BG_EVENT_PLAYER_FACING_EAST', 4: 'BG_EVENT_PLAYER_FACING_WEST', 5: 'BG_EVENT_5', 6: 'BG_EVENT_6',
            7: 'BG_EVENT_HIDDEN_ITEM', 8: 'BG_EVENT_SECRET_BASE'}


def group_maps(rom, g):
    a = rom.ptr(MAP_GROUPS + 4 * g)
    b = rom.ptr(MAP_GROUPS + 4 * g + 4) if g < NUM_GROUPS - 1 else MAP_GROUPS
    return [rom.ptr(a + 4 * i) for i in range((b - a) // 4)]


def region_of(g):
    return 'johto' if g in JOHTO_GROUPS else 'kanto' if g in KANTO_GROUPS else 'hoenn'


def mapsec_name(rom, sec, region):
    t = MAPSEC_JOHTO if region == 'johto' else MAPSEC_MAIN
    p = rom.ptr(t + 8 * sec + 4)
    if p is None:
        return None
    q = rom.ptr(p)
    return plain(rom, q, 40) if q is not None else None


def parse_header(rom, h):
    m = {'header_addr': h}
    m['layout_addr'] = rom.ptr(h)
    m['events_addr'] = rom.ptr(h + 4)
    m['scripts_addr'] = rom.ptr(h + 8)
    m['connections_addr'] = rom.ptr(h + 12)
    m['music'] = rom.u16(h + 16)
    m['layout_id'] = rom.u16(h + 18)
    m['mapsec'] = rom.u8(h + 20)
    m['cave'] = rom.u8(h + 21)
    m['weather'] = rom.u8(h + 22)
    m['map_type'] = rom.u8(h + 23)
    fl = rom.u8(h + 26)
    m['allow_cycling'] = bool(fl & 1)
    m['allow_escaping'] = bool(fl & 2)
    m['allow_running'] = bool(fl & 4)
    m['show_map_name'] = bool(fl & 8)
    m['flags_extra'] = fl >> 4
    m['battle_type'] = rom.u8(h + 27)
    m['filler'] = rom.d[h + 24:h + 26].hex()
    return m


def parse_layout(rom, L):
    return {'addr': L, 'width': rom.u32(L), 'height': rom.u32(L + 4), 'border': rom.ptr(L + 8),
            'blockdata': rom.ptr(L + 12), 'primary': rom.ptr(L + 16), 'secondary': rom.ptr(L + 20),
            'border_width': rom.u8(L + 24), 'border_height': rom.u8(L + 25)}


def parse_events(rom, E):
    ev = {'objects': [], 'warps': [], 'coords': [], 'bgs': []}
    if E is None:
        return ev
    no, nw, nc, nb = rom.d[E:E + 4]
    o = rom.ptr(E + 4)
    for i in range(no):
        a = o + 24 * i
        mr = rom.u8(a + 10)
        ev['objects'].append({
            'local_id': rom.u8(a), 'kind': rom.u8(a + 1), 'graphics_id': rom.u16(a + 2),
            'x': rom.s16(a + 4), 'y': rom.s16(a + 6), 'elevation': rom.u8(a + 8),
            'movement_type': rom.u8(a + 9), 'movement_range_x': mr & 15, 'movement_range_y': mr >> 4,
            'byte11': rom.u8(a + 11), 'trainer_type': rom.u16(a + 12), 'trainer_sight_or_berry_tree_id': rom.u16(a + 14),
            'script': rom.ptr(a + 16), 'flag': rom.u16(a + 20), 'pad': rom.u16(a + 22)})
    o = rom.ptr(E + 8)
    for i in range(nw):
        a = o + 8 * i
        ev['warps'].append({'x': rom.s16(a), 'y': rom.s16(a + 2), 'elevation': rom.u8(a + 4),
                            'dest_warp_id': rom.u8(a + 5), 'dest_map': (rom.u8(a + 7), rom.u8(a + 6))})
    o = rom.ptr(E + 12)
    for i in range(nc):
        a = o + 16 * i
        ev['coords'].append({'x': rom.s16(a), 'y': rom.s16(a + 2), 'elevation': rom.u8(a + 4),
                             'var': rom.u16(a + 6), 'var_value': rom.u16(a + 8), 'script': rom.ptr(a + 12)})
    o = rom.ptr(E + 16)
    for i in range(nb):
        a = o + 12 * i
        kind = rom.u8(a + 5)
        bg = {'x': rom.s16(a), 'y': rom.s16(a + 2), 'elevation': rom.u8(a + 4), 'kind': kind}
        if kind == 7:
            bg['item'] = rom.u16(a + 8)
            bg['flag'] = rom.u16(a + 10)
        elif kind == 8:
            bg['secret_base_id'] = rom.u32(a + 8)
        else:
            bg['script'] = rom.ptr(a + 8)
        ev['bgs'].append(bg)
    return ev


def parse_connections(rom, C):
    out = []
    if C is None:
        return out
    n = rom.s32(C)
    lst = rom.ptr(C + 4)
    for i in range(n):
        a = lst + 12 * i
        out.append({'direction': rom.u8(a), 'offset': rom.s32(a + 4), 'map': (rom.u8(a + 8), rom.u8(a + 9))})
    return out


def parse_map_scripts(rom, S):
    """Returns [(type, addr, table_or_None)]; tables are [(var, value, script)]."""
    out = []
    if S is None:
        return out
    p = S
    while True:
        t = rom.u8(p)
        if t == 0:
            break
        a = rom.ptr(p + 1)
        table = None
        if t in (2, 4) and a is not None:
            table = []
            q = a
            while rom.u16(q) != 0:
                table.append((rom.u16(q), rom.u16(q + 2), rom.ptr(q + 4)))
                q += 8
        out.append((t, a, table))
        p += 5
    return out


def all_maps(rom):
    out = []
    for g in range(NUM_GROUPS):
        for n, h in enumerate(group_maps(rom, g)):
            m = parse_header(rom, h)
            m['group'] = g
            m['num'] = n
            m['region'] = region_of(g)
            m['mapsec_name'] = mapsec_name(rom, m['mapsec'], m['region'])
            out.append(m)
    return out
