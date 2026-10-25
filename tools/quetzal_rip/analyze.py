"""Phase 1: read every Kanto / Johto map and walk all of their scripts."""
from qrom import Rom, Names, find_rom, decode
from maps import all_maps, parse_layout, parse_events, parse_connections, parse_map_scripts
from scripts import Disassembler


def map_entries(m):
    """(addr, what) for every script entry point a map has."""
    out = []
    for t, a, table in m['map_scripts']:
        if table is None:
            out.append((a, 'mapscript%d' % t))
        else:
            for var, val, s in table:
                out.append((s, 'mapscript%d' % t))
    for i, o in enumerate(m['events']['objects']):
        if o['script'] is not None:
            out.append((o['script'], 'object%d' % i))
    for i, c in enumerate(m['events']['coords']):
        if c['script'] is not None:
            out.append((c['script'], 'coord%d' % i))
    for i, b in enumerate(m['events']['bgs']):
        if b.get('script') is not None:
            out.append((b['script'], 'bg%d' % i))
    return out


def load(rom_path=None, regions=('johto', 'kanto')):
    rom = Rom(rom_path or find_rom())
    names = Names(rom)
    maps = all_maps(rom)
    for m in maps:
        m['layout'] = parse_layout(rom, m['layout_addr'])
        m['events'] = parse_events(rom, m['events_addr'])
        m['connections'] = parse_connections(rom, m['connections_addr'])
        m['map_scripts'] = parse_map_scripts(rom, m['scripts_addr'])
    return rom, names, maps


def walk(rom, names, maps, regions=('johto', 'kanto'), map_const=None):
    dis = Disassembler(rom, names, map_const or (lambda gn: 'MAP_QZ_%d_%d' % gn))
    for m in maps:
        if m['region'] not in regions:
            continue
        for a, what in map_entries(m):
            dis.add_entry(a, (m['group'], m['num']))
    return dis


if __name__ == '__main__':
    rom, names, maps = load()
    dis = walk(rom, names, maps)
    print('maps', sum(1 for m in maps if m['region'] != 'hoenn'))
    print('script blocks', sum(1 for k in dis.kind.values() if k == 'script'),
          'texts', sum(1 for k in dis.kind.values() if k == 'text'),
          'movements', sum(1 for k in dis.kind.values() if k == 'movement'),
          'marts', sum(1 for k in dis.kind.values() if k == 'mart'))
    print('problems', len(dis.problems))
    for p in dis.problems[:40]:
        print('  ', p)
    print('flags', len(dis.flags), 'vars', len(dis.vars), 'specials', sorted(dis.specials))
