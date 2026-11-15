#!/usr/bin/env python3
"""Mock-ups for the user (2026-11-15) of OPTIONS > DISPLAY > UI STYLE: the game's interface in six styles (KAI, the
game as it is; RED, CRYSTAL, FIRERED, PLATINUM, BLACK, each from its own game's art) on every screen the style changes:
the message box, the START menu, a battle's command (single, double, triple), its moves, the party, the summary, the
BAG, OPTIONS. KAI's are the game's own pictures (tools/test/scenarios/uicap_*: rendershot, the battles' scenes again
with the interface hidden: `mp hud 0`); the others are drawn over those scenes.

    python3 tools/ui_styles/mockups.py KAI_SHOTS EXPORT OUT_DIR [--pokered DIR] [--pokecrystal DIR] [--pokefirered DIR]
            [--platinum ROM] [--black ROM]

EXPORT: the game's MOEMON_EXPORT_APP folder (creature pictures, icons, item icons). Writes OUT_DIR/<style>/<screen>.png
(2x), OUT_DIR/<style>.png (a style's ten screens) and OUT_DIR/<screen>.png (a screen in every style).
"""
import argparse
import os
import sys

from PIL import Image, ImageDraw

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import STYLES, H, W, shot  # noqa: E402
import check  # noqa: E402

SCREENS = ['message', 'start_menu', 'battle_single', 'battle_moves', 'battle_double', 'battle_triple', 'party',
           'summary', 'bag', 'options']
KAI_SHOTS = {'message': 'k_msg', 'start_menu': 'k_start', 'battle_single': 'k_single', 'battle_moves': 'k_moves',
             'battle_double': 'k_double', 'battle_triple': 'k_triple', 'party': 'k_party', 'summary': 'k_summary',
             'bag': 'k_bag', 'options': 'k_options'}


def grid(pics, cols, label=None):
    rows = (len(pics) + cols - 1) // cols
    top = 18 if label else 0
    sheet = Image.new('RGB', (cols * (W * 2 + 8) + 8, top + rows * (H * 2 + 8) + 8), (32, 32, 40))
    if label:
        ImageDraw.Draw(sheet).text((10, 4), label, fill=(240, 240, 240))
    for i, im in enumerate(pics):
        sheet.paste(im.convert('RGB').resize((W * 2, H * 2), Image.NEAREST),
                    (8 + (i % cols) * (W * 2 + 8), top + 8 + (i // cols) * (H * 2 + 8)))
    return sheet


def main():
    a = argparse.ArgumentParser()
    a.add_argument('kai')
    a.add_argument('export')
    a.add_argument('out')
    a.add_argument('--pokered')
    a.add_argument('--pokecrystal')
    a.add_argument('--pokefirered')
    a.add_argument('--platinum')
    a.add_argument('--black')
    a.add_argument('--only', default='')
    o = a.parse_args()
    styles = {'KAI': {s: shot(o.kai, KAI_SHOTS[s]) for s in SCREENS}}
    want = set(o.only.upper().split(',')) if o.only else set(STYLES)
    makers = {}
    if o.pokered and 'RED' in want:
        import gbstyle
        makers['RED'] = lambda: gbstyle.GB('red', o.pokered, o.kai, o.export,
                                           o.pokecrystal and os.path.join(o.pokecrystal, 'gfx', 'pack', 'pack.png'))
    if o.pokecrystal and 'CRYSTAL' in want:
        import gbstyle
        makers['CRYSTAL'] = lambda: gbstyle.GB('crystal', o.pokecrystal, o.kai, o.export)
    if o.pokefirered and 'FIRERED' in want:
        import fr
        makers['FIRERED'] = lambda: fr.FireRed(o.pokefirered, o.kai, o.export)
    if o.platinum and 'PLATINUM' in want:
        import ds
        makers['PLATINUM'] = lambda: ds.Platinum(o.platinum, o.kai, o.export)
    if o.black and 'BLACK' in want:
        import ds
        makers['BLACK'] = lambda: ds.Black(o.black, o.kai, o.export)
    report = []
    for name, make in makers.items():
        st = make()
        if hasattr(st, 'logs'):
            bare = check.bare_screens(make())
            check.log_text(st)
        styles[name] = st.all()
        found, kept = check.check_style(st, styles[name], o.kai, bare) if hasattr(st, 'logs') else (['(not checked)'], [])
        report += ['%s: %s' % (name, 'OK' if not found else '%d findings' % len(found))] + ['  ' + f for f in found]
        report += ['  ' + f for f in kept]
    print('\n'.join(report))
    os.makedirs(o.out, exist_ok=True)
    for name, screens in styles.items():
        d = os.path.join(o.out, name.lower())
        os.makedirs(d, exist_ok=True)
        for s in SCREENS:
            screens[s].convert('RGB').resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(d, s + '.png'))
        grid([screens[s] for s in SCREENS], 5, 'UI STYLE: ' + name).save(os.path.join(o.out, name.lower() + '.png'))
    order = [s for s in STYLES if s in styles]
    for s in SCREENS:
        grid([styles[k][s] for k in order], 3, s.upper() + ': ' + ' / '.join(order)).save(os.path.join(o.out, s + '.png'))
    open(os.path.join(o.out, 'report.txt'), 'w').write('\n'.join(report) + '\n')
    print(o.out, order)


if __name__ == '__main__':
    main()
