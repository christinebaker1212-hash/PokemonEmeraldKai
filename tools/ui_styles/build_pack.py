"""Builds UI STYLE packs, the pictures, fonts and specs the game draws a style from (src/mp/mp_uistyle.c; the format in
uipack.py; docs/ui_styles/UI_STYLES.md "Building it").

usage: build_pack.py STYLE=SOURCE OUT.pak
    red=POKERED_DIR crystal=POKECRYSTAL_DIR      (pret/pokered, pret/pokecrystal: uistyle/red.pak, crystal.pak)
    firered=POKEFIRERED_DIR                      (pret/pokefirered: uistyle/firered.pak)
    platinum=POKEPLATINUM_DIR:ROM                (sinnoh/ui_platinum.pak: never committed, the release builds it)
    black=ROM                                    (unova/ui_black.pak: never committed, the release builds it)
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))


def build(kind, src):
    if kind in ('red', 'crystal'):
        import gb_build
        return gb_build.GBPack(kind, src).make()
    if kind == 'firered':
        import fr_build
        return fr_build.FRPack(src).make()
    if kind == 'platinum':
        import pt_build
        pret, rom = src.split(':', 1) if ':' in src else (src, None)
        return pt_build.PTPack(pret, rom).make()
    if kind == 'black':
        import bw_build
        return bw_build.BWPack(src).make()
    raise SystemExit('no style %r' % kind)


def main():
    if len(sys.argv) != 3 or '=' not in sys.argv[1]:
        raise SystemExit(__doc__)
    kind, src = sys.argv[1].split('=', 1)
    out = sys.argv[2]
    pack = build(kind, src)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    n = pack.write(out)
    print('%s: %d bytes (%d pictures, %d fonts, %d specs)' % (out, n, len(pack.images), len(pack.fonts),
                                                               len(pack.specs)))


if __name__ == '__main__':
    main()
