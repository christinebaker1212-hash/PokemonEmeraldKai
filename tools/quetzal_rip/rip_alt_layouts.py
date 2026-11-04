#!/usr/bin/env python3
"""The alternate layouts Quetzal's scripts switch to (setmaplayoutindex) that no map header points at, so rip.py
never saw them: SEAFOAM ISLANDS B3F / B4F with the currents stopped (layouts 899 / 900, after the boulders) and
DUNSPARCE TUNNEL dug out (940). Read from Quetzal's layout table (ROM 0x930A8C, entry = layout id - 1; ROM_MAP.md's
0x08939B54 is a map header table), written into quetzal_rip/layouts and the game's data/layouts (appended, so no
other layout's number moves), as tools/quetzal_port/import.py copies the rest. Run again: nothing changes.

    python3 -I tools/quetzal_rip/rip_alt_layouts.py QUETZAL_ROM

Then rerun the import (or edit the three scripts' setmaplayoutindex) so the scripts name them.
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'quetzal_port'))
import frlg_blocks   # (FireRed's blocks where Quetzal changed them: the game's copy only)

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
TABLE = 0x930A8C
# Quetzal's id: (name, the layout it stands in for: same size and tilesets)
ALT = {
    899: ('Kanto_SeafoamIslands_B3F_CurrentStopped', 'LAYOUT_KANTO_SEAFOAM_ISLANDS_B3F'),
    900: ('Kanto_SeafoamIslands_B4F_CurrentStopped', 'LAYOUT_KANTO_SEAFOAM_ISLANDS_B4F'),
    940: ('Kanto_ThreeIsland_DunsparceTunnel_DugOut', 'LAYOUT_KANTO_THREE_ISLAND_DUNSPARCE_TUNNEL'),
}
SUFFIX = {'CurrentStopped': '_CURRENT_STOPPED', 'DugOut': '_DUG_OUT'}


def layout_const(name, base_id):
    return base_id + SUFFIX[name.rsplit('_', 1)[1]]


def main():
    rom = open(sys.argv[1], 'rb').read()
    u32 = lambda o: struct.unpack_from('<I', rom, o)[0]
    rip_path = os.path.join(REPO, 'quetzal_rip/layouts/layouts.json')
    kai_path = os.path.join(REPO, 'data/layouts/layouts.json')
    rip, kai = json.load(open(rip_path)), json.load(open(kai_path))
    rip_by_id = {l['id']: l for l in rip['layouts']}
    for lid, (name, base_id) in ALT.items():
        addr = u32(TABLE + 4 * (lid - 1))
        o = addr - 0x08000000
        w, h, border, blocks, prim, sec = struct.unpack_from('<IIIIII', rom, o)
        bw, bh = rom[o + 24], rom[o + 25]
        base = rip_by_id[base_id]
        bo = int(base['quetzal_addr'], 16) - 0x08000000
        if (w, h, prim, sec) != (base['width'], base['height']) + struct.unpack_from('<II', rom, bo + 16):
            raise SystemExit('%d: not the same size and tilesets as %s' % (lid, base_id))
        lid_const = layout_const(name, base_id)
        for root, table in (('quetzal_rip/layouts', rip), ('data/layouts', kai)):
            d = os.path.join(REPO, root, name)
            os.makedirs(d, exist_ok=True)
            open(os.path.join(d, 'map.bin'), 'wb').write(rom[blocks - 0x08000000:blocks - 0x08000000 + w * h * 2])
            open(os.path.join(d, 'border.bin'), 'wb').write(rom[border - 0x08000000:border - 0x08000000 + bw * bh * 2])
            table['layouts'] = [l for l in table['layouts'] if l['id'] != lid_const]
        frlg_blocks.apply(name, os.path.join(REPO, 'data/layouts', name, 'map.bin'))
        rip['layouts'].append({
            'id': lid_const, 'name': name + '_Layout', 'width': w, 'height': h, 'border_width': bw, 'border_height': bh,
            'primary_tileset': base['primary_tileset'], 'secondary_tileset': base['secondary_tileset'],
            'border_filepath': 'quetzal_rip/layouts/%s/border.bin' % name,
            'blockdata_filepath': 'quetzal_rip/layouts/%s/map.bin' % name,
            'quetzal_layout_id': lid, 'quetzal_addr': '0x%X' % addr})
        kbase = next(l for l in kai['layouts'] if l['id'] == base_id)
        kai['layouts'].append({
            'id': lid_const, 'name': name + '_Layout', 'width': w, 'height': h,
            'primary_tileset': kbase['primary_tileset'], 'secondary_tileset': kbase['secondary_tileset'],
            'border_filepath': 'data/layouts/%s/border.bin' % name, 'blockdata_filepath': 'data/layouts/%s/map.bin' % name,
            'quetzal': True})
        print('%d -> %s (%dx%d)' % (lid, lid_const, w, h))
    for path, table in ((rip_path, rip), (kai_path, kai)):
        with open(path, 'w', encoding='utf-8') as f:
            json.dump(table, f, indent=2, ensure_ascii=False)
            f.write('\n')


if __name__ == '__main__':
    main()
