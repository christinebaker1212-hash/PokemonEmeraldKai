#!/usr/bin/env python3
"""Where Quetzal's KANTO layouts differ from FireRed's own, FireRed's win (the user, 2026-11-03: Quetzal's changes to
FireRed's or Crystal's maps are never used). The rip (quetzal_rip/layouts) stays Quetzal's bytes; the game's copies in
data/layouts get FireRed's blocks back here, by import.py and tools/quetzal_rip/rip_alt_layouts.py.

Each entry: the layout's folder name -> [(x, y, FireRed's block)], checked against pret/pokefirered's data/layouts
(compare with `python3 -I tools/quetzal_port/frlg_blocks.py --check POKEFIRERED_DIR`).
"""
import os
import struct
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

# SEAFOAM ISLANDS B3F: Quetzal made two floor tiles by the south ledge solid (collision 1), FireRed walks them
FIXES = {
    'Kanto_SeafoamIslands_B3F': (38, [(6, 18, 0x3286), (9, 18, 0x3286)]),
    'Kanto_SeafoamIslands_B3F_CurrentStopped': (38, [(6, 18, 0x3286), (9, 18, 0x3286)]),
}
# (the game's folder name -> pokefirered's)
FRLG_NAME = {k: k[len('Kanto_'):] for k in FIXES}


def apply(name, path):
    """FireRed's blocks into the map.bin at PATH (the game's copy of layout NAME); True when it had any"""
    if name not in FIXES:
        return False
    width, blocks = FIXES[name]
    data = bytearray(open(path, 'rb').read())
    for x, y, v in blocks:
        struct.pack_into('<H', data, 2 * (y * width + x), v)
    open(path, 'wb').write(bytes(data))
    return True


def check(frlg_dir):
    bad = 0
    for name in FIXES:
        ours = open(os.path.join(REPO, 'data/layouts', name, 'map.bin'), 'rb').read()
        theirs = open(os.path.join(frlg_dir, 'data/layouts', FRLG_NAME[name], 'map.bin'), 'rb').read()
        print('%s: %s' % (name, 'FireRed\'s' if ours == theirs else 'DIFFERS'))
        bad += ours != theirs
    return bad


if __name__ == '__main__':
    if len(sys.argv) == 3 and sys.argv[1] == '--check':
        sys.exit(check(sys.argv[2]))
    for name in FIXES:
        apply(name, os.path.join(REPO, 'data/layouts', name, 'map.bin'))
