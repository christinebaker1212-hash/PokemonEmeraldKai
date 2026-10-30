#!/usr/bin/env python3
"""Johto / Kanto story items Kai lacks (GS BALL, SQUIRTBOTTLE, the wings, the bells...), ids after the Gen 4 berries.

    python3 tools/quetzal_port/gen_items.py EXPANSION_DIR

Icons come from pokeemerald-expansion (graphics/items/icons, icon_palettes): its own where it has one, else a close
shape in new colors (the palette's hues turned, see RECOLOR). Writes the constants into include/constants/items.h
(between the "Johto / Kanto items" markers), src/data/items_qz.h, src/data/text/item_descriptions_qz.h,
src/data/graphics/items_qz.h, src/data/item_icons_qz.h, graphics/items/{icons,icon_palettes}/<name>.
"""
import colorsys
import os
import re
import shutil
import sys

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
FIRST = 495

# const, name, pocket, icon (expansion name), recolor (hue in degrees, saturation factor, value factor) or None,
# description, price
ITEMS = [
    ('CLEAR_BELL', 'CLEAR BELL', 'KEY', 'soothe_bell', (190, 0.6, 1.1),
     'A clear-sounding\nbell. It makes a\ngentle ringing.', 0),
    ('TIDAL_BELL', 'TIDAL BELL', 'KEY', 'shell_bell', (200, 1.2, 1.0),
     'A bell that rings\nwith the sound of\nthe sea.', 0),
    ('GS_BALL', 'GS BALL', 'KEY', 'poke_ball', (45, 1.0, 1.1),
     'A mysterious BALL\nmarked GS. KURT\nwants to see it.', 0),
    ('SILVER_WING', 'SILVER WING', 'KEY', 'pretty_feather', (220, 0.2, 1.15),
     'A strange, silver-\ncolored feather\nthat sparkles.', 0),
    ('RAINBOW_WING', 'RAINBOW WING', 'KEY', 'pretty_feather', (330, 1.4, 1.1),
     'A mystical feather\nof rainbow colors\nthat sparkles.', 0),
    ('RED_SCALE', 'RED SCALE', 'KEY', 'dragon_scale', (0, 1.3, 1.0),
     'A scale from the\nred GYARADOS. It\nglows like fire.', 0),
    ('SECRET_POTION', 'SECRETPOTION', 'KEY', 'large_potion', (280, 1.0, 1.0),
     'A fantastic\nmedicine dispensed\nby the pharmacy.', 0),
    ('MYSTERY_EGG', 'MYSTERY EGG', 'KEY', 'lucky_egg', None,
     'An EGG obtained\nfrom MR. POKéMON.\nWhat\'s in it?', 0),
    ('PARCEL', 'OAK\'S PARCEL', 'KEY', 'parcel', None,
     'A parcel for\nPROF. OAK from\nVIRIDIAN\'s MART.', 0),
    ('SQUIRTBOTTLE', 'SQUIRTBOTTLE', 'KEY', 'aux_bottle', (200, 1.2, 1.0),
     'A bottle used for\nwatering plants\nin flower shops.', 0),
    ('GRACIDEA', 'GRACIDEA', 'KEY', 'gracidea', None,
     'A flower sometimes\nbundled in\nbouquets of thanks.', 0),
    ('SPORT_BALL', 'SPORT BALL', 'KEY', 'sport_ball', None,
     'A special BALL for\nthe BUG-CATCHING\nCONTEST.', 0),
    ('UPGRADE', 'UP-GRADE', 'ITEMS', 'upgrade', None,
     'A transparent\ndevice filled with\nall sorts of data.', 2100),
]


def camel(c):
    return ''.join(w.capitalize() for w in c.split('_'))


def snake(c):
    return c.lower()


def read_pal(path):
    lines = open(path).read().split()
    n = int(lines[2])
    return [tuple(int(v) for v in lines[3 + i * 3:6 + i * 3]) for i in range(n)]


def write_pal(path, pal):
    with open(path, 'w', newline='\r\n') as f:
        f.write('JASC-PAL\n0100\n%d\n' % len(pal))
        for c in pal:
            f.write('%d %d %d\n' % c)


def recolor(pal, spec):
    hue, sat, val = spec
    out = [pal[0]]
    for r, g, b in pal[1:]:
        h, s, v = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
        if s > 0.12:
            h = hue / 360.0
        s = min(1.0, s * sat)
        v = min(1.0, v * val)
        r2, g2, b2 = colorsys.hsv_to_rgb(h, s, v)
        out.append((round(r2 * 255) & 0xF8, round(g2 * 255) & 0xF8, round(b2 * 255) & 0xF8))
    return out


def main():
    exp = sys.argv[1]
    consts, data, desc, gfx, icons = [], [], [], [], []
    for i, (c, name, pocket, icon, rc, text, price) in enumerate(ITEMS):
        const = 'ITEM_' + c
        consts.append('#define %s %d' % (const, FIRST + i))
        shutil.copy(os.path.join(exp, 'graphics/items/icons/%s.png' % icon),
                    os.path.join(REPO, 'graphics/items/icons/%s.png' % snake(c)))
        pf = os.path.join(exp, 'graphics/items/icon_palettes/%s.pal' % icon)
        if os.path.exists(pf):
            pal = read_pal(pf)
        else:   # palettes several icons share have another name: the picture's own colors
            from PIL import Image
            raw = Image.open(os.path.join(exp, 'graphics/items/icons/%s.png' % icon)).getpalette()[:48]
            pal = [tuple(raw[k:k + 3]) for k in range(0, 48, 3)]
        write_pal(os.path.join(REPO, 'graphics/items/icon_palettes/%s.pal' % snake(c)), recolor(pal, rc) if rc else pal)
        key = pocket == 'KEY'
        data.append('    [%s] =\n    {\n        .name = _("%s"),\n        .itemId = %s,\n        .price = %d,\n'
                    '        .description = s%sDesc,\n%s        .pocket = POCKET_%s,\n        .type = ITEM_USE_BAG_MENU,\n'
                    '        .fieldUseFunc = ItemUseOutOfBattle_CannotUse,\n    },\n\n'
                    % (const, name, const, price, camel(c), '        .importance = 1,\n' if key else '',
                       'KEY_ITEMS' if key else 'ITEMS'))
        desc.append('static const u8 s%sDesc[] = _(\n%s);\n\n' % (camel(c), '\n'.join(
            '    "%s%s"' % (l.replace('"', '\\"'), '\\n' if j < len(text.split('\n')) - 1 else '')
            for j, l in enumerate(text.split('\n')))))
        gfx.append('const u32 gItemIcon_%s[] = INCGFX_U32("graphics/items/icons/%s.png", ".4bpp.lz");\n'
                   'const u32 gItemIconPalette_%s[] = INCGFX_U32("graphics/items/icon_palettes/%s.pal", ".gbapal.lz");\n'
                   % (camel(c), snake(c), camel(c), snake(c)))
        icons.append('    [%s] = {gItemIcon_%s, gItemIconPalette_%s}, \\\n' % (const, camel(c), camel(c)))
    head = '// Generated by tools/quetzal_port/gen_items.py. Do not edit.\n'
    p = os.path.join(REPO, 'include/constants/items.h')
    s = open(p, encoding='utf-8').read()
    block = ('// Moemon Kai: Johto / Kanto story items (tools/quetzal_port/gen_items.py)\n%s\n'
             '#define FIRST_QZ_ITEM ITEM_%s\n#define LAST_QZ_ITEM ITEM_%s\n\n#define ITEMS_COUNT %d\n// end of Johto / Kanto items\n'
             % ('\n'.join(consts), ITEMS[0][0], ITEMS[-1][0], FIRST + len(ITEMS)))
    if '// end of Johto / Kanto items\n' in s:
        s = re.sub(r'// Moemon Kai: Johto / Kanto story items.*?// end of Johto / Kanto items\n', block, s, flags=re.S)
    else:
        s = s.replace('#define ITEMS_COUNT %d\n// end of Gen 4 berries\n' % FIRST, '// end of Gen 4 berries\n\n' + block)
    open(p, 'w', encoding='utf-8').write(s)
    open(os.path.join(REPO, 'src/data/items_qz.h'), 'w', encoding='utf-8').write(head + ''.join(data))
    open(os.path.join(REPO, 'src/data/text/item_descriptions_qz.h'), 'w', encoding='utf-8').write(head + ''.join(desc))
    open(os.path.join(REPO, 'src/data/graphics/items_qz.h'), 'w', encoding='utf-8').write(head + ''.join(gfx))
    open(os.path.join(REPO, 'src/data/item_icons_qz.h'), 'w', encoding='utf-8').write(
        head + ''.join('extern const u32 gItemIcon_%s[];\nextern const u32 gItemIconPalette_%s[];\n' % (camel(c[0]), camel(c[0]))
                       for c in ITEMS) + '#define ITEM_ICONS_QZ \\\n' + ''.join(icons) + '\n')
    print('%d items, %d-%d' % (len(ITEMS), FIRST, FIRST + len(ITEMS) - 1))


if __name__ == '__main__':
    main()
