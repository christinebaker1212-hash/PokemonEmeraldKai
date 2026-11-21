#!/usr/bin/env python3
"""Moemon VMU pet: draw text in the game's own FONT_NORMAL (graphics/fonts/latin_normal.png, src/fonts.c's widths,
charmap.txt) onto a screenshot, for mock-ups of Kai's screens. Coordinates and the scale as the screenshot's
(the harness's shots are 2x)."""
import os
import re

REPO = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
_font = None


def _load():
    global _font
    if _font:
        return _font
    from PIL import Image
    sheet = Image.open(os.path.join(REPO, 'graphics/fonts/latin_normal.png'))
    widths = re.search(r'gFontNormalLatinGlyphWidths\[\]\s*=\s*\{(.*?)\};', open(os.path.join(REPO, 'src/fonts.c')).read(), re.S).group(1)
    widths = [int(v) for v in re.findall(r'\d+', widths)]
    cmap = {}
    for line in open(os.path.join(REPO, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"'(.|\\')'\s*=\s*([0-9A-F]{2})\s*$", line.strip())
        if m:
            cmap.setdefault(m.group(1), int(m.group(2), 16))
    cmap[' '] = 0
    _font = (sheet, widths, cmap)
    return _font


def width(text):
    sheet, widths, cmap = _load()
    return sum(widths[cmap.get(c, 0)] for c in text)


def draw(img, x, y, text, fg=(98, 98, 98), shadow=(213, 213, 205), scale=2):
    """Text at (x, y) (screenshot pixels; the glyphs' 16x16 cells' top left)."""
    sheet, widths, cmap = _load()
    px = img.load()
    for c in text:
        g = cmap.get(c, 0)
        gx, gy = (g % 16) * 16, (g // 16) * 16
        for yy in range(16):
            for xx in range(widths[g]):
                v = sheet.getpixel((gx + xx, gy + yy))
                col = fg if v == 1 else shadow if v == 2 else None
                if col:
                    for sy in range(scale):
                        for sx in range(scale):
                            X, Y = x + xx * scale + sx, y + yy * scale + sy
                            if 0 <= X < img.width and 0 <= Y < img.height:
                                px[X, Y] = col
        x += widths[g] * scale
    return x
