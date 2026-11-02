#!/usr/bin/env python3
"""The seven APRICORNs' item icons (items 686-692, JOHTO's fruit trees: src/mp/mp_apricorn.c), from PokeAPI's item
sprites (sprites/items/<color>-apricorn.png, 30x30): the picture's middle 24x24, at most 15 colors and the
transparent one, written as graphics/items/icons/<name>.png and icon_palettes/<name>.pal like the game's others.

    python3 tools/moemon_items/gen_apricorns.py
"""
import io
import os
import urllib.request
from PIL import Image

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RAW = 'https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/items/%s-apricorn.png'
COLORS = [('red', 'red_apricorn'), ('blue', 'blu_apricorn'), ('yellow', 'ylw_apricorn'), ('green', 'grn_apricorn'),
          ('white', 'wht_apricorn'), ('black', 'blk_apricorn'), ('pink', 'pnk_apricorn')]
KEY = (255, 0, 128)   # (the game's transparent color, index 0)


def main():
    for color, name in COLORS:
        im = Image.open(io.BytesIO(urllib.request.urlopen(RAW % color).read())).convert('RGBA').crop((3, 3, 27, 27))
        alpha = im.split()[3]
        rgb = Image.new('RGB', im.size, (0, 0, 0))
        rgb.paste(im, mask=alpha)
        q = rgb.quantize(colors=15, method=Image.MEDIANCUT)
        pal = (q.getpalette()[:45] + [0] * 45)[:45]
        out = Image.new('P', im.size, 0)
        out.putpalette(list(KEY) + pal + [0] * (768 - 48))
        for y in range(24):
            for x in range(24):
                if alpha.getpixel((x, y)) >= 128:
                    out.putpixel((x, y), q.getpixel((x, y)) + 1)
        out.save(os.path.join(REPO, 'graphics/items/icons/%s.png' % name))
        cols = [KEY] + [tuple(pal[i:i + 3]) for i in range(0, 45, 3)]
        with open(os.path.join(REPO, 'graphics/items/icon_palettes/%s.pal' % name), 'w', newline='\r\n') as f:
            f.write('JASC-PAL\n0100\n16\n' + ''.join('%d %d %d\n' % c for c in cols))
        print(name)


if __name__ == '__main__':
    main()
