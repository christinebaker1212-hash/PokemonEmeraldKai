#!/usr/bin/env python3
"""Mock-ups of the ball effect editor's quick picking (the user, 2026-11-17: "with so many effects, we are quickly
able to switch out and view everything at a glance"), drawn in the editor's own colours and the game's own fonts
over a real frame of its preview (mp_ballfx_editor.c: the panel x 112-239, the info box under the preview).

    python3 tools/moemon_ballfx/mockups.py PREVIEW.png OUTDIR
"""
import os
import re
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'stadium', 'restyle'))
import s2  # noqa: E402  (the game's fonts and character codes)


def c5(r, g, b):
    return (r * 255 // 31, g * 255 // 31, b * 255 // 31)


# mp_ballfx_editor.c sTextPal
WHITE, SHADOW, GREY, PANEL, BAND, GOLD, DIM, EDGE = (c5(31, 31, 31), c5(2, 3, 7), c5(20, 21, 25), c5(4, 6, 12),
                                                    c5(10, 14, 26), c5(31, 25, 6), c5(15, 17, 23), c5(16, 20, 30))
NEWC = c5(31, 12, 10)
NORMAL, SMALL = s2.NORMAL, s2.SMALL


def names(array):
    src = open(os.path.join(REPO, 'src', 'mp', 'mp_ballfx.c')).read()
    body = re.search(array + r'\[[^\]]*\]\s*=\s*\{(.*?)\};', src, re.S).group(1)
    return re.findall(r'"([^"]*)"', body)


def color_table():
    """the colours in sColorNames' order, each by its middle shade (sRamps; the classic RED is the game's own)"""
    src = open(os.path.join(REPO, 'src', 'mp', 'mp_ballstyle.c')).read()
    ramps = src[src.index('sRamps[MP_BALL_COLORS]'):]
    ramps = ramps[:ramps.index('};')]
    shades = {m.group(1): c5(*map(int, m.group(2, 3, 4))) for m in re.finditer(
        r'\[MP_BALL_(\w+)\]\s*=\s*\{RGB\(\d+, \d+, \d+\),\s*RGB\(\d+, \d+, \d+\),\s*RGB\((\d+), (\d+), (\d+)\)', ramps)}
    shades['RED'] = c5(31, 6, 6)
    names_ = src[src.index('sColorNames[MP_BALL_COLORS]'):]
    order = re.findall(r'\[MP_BALL_(\w+)\]\s*=\s*_\("', names_[:names_.index('};')])
    return [(n, shades.get(n, (128, 128, 128))) for n in order]


def text(im, x, y, s, col, font=SMALL, shadow=SHADOW):
    face, sh = s2.text_masks(s, font)
    p = im.load()
    for yy in range(face.shape[0]):
        for xx in range(face.shape[1]):
            X, Y = x + xx, y + yy
            if 0 <= X < im.width and 0 <= Y < im.height:
                if face[yy, xx]:
                    p[X, Y] = col
                elif sh[yy, xx] and shadow:
                    p[X, Y] = shadow
    return x + s2.measure(s, font)


def rect(im, x, y, w, h, col):
    p = im.load()
    for yy in range(max(0, y), min(im.height, y + h)):
        for xx in range(max(0, x), min(im.width, x + w)):
            p[xx, yy] = col


LEFT = ["..#", ".##", "###", ".##", "..#"]
RIGHT = ["#..", "##.", "###", "##.", "#.."]
STAR = ["...#...", "..###..", "#######", ".#####.", "..###..", ".##.##.", "##...##"]
LOCK = ["..###..", ".#...#.", ".#...#.", "#######", "###.###", "###.###", "#######"]
KEYS = {'A': ' A ', 'B': ' B ', 'L': ' L ', 'R': ' R ', 'SELECT': 'SELECT', 'START': 'START'}


def glyph(im, x, y, pic, col):
    for yy, row in enumerate(pic):
        for xx, ch in enumerate(row):
            if ch == '#':
                rect(im, x + xx, y + yy, 1, 1, col)


def key(im, x, y, k):
    """a button as the game's keypad icons draw it: a white rounded tag with dark letters"""
    s = KEYS[k].strip()
    w = s2.measure(s, SMALL) + 4
    rect(im, x + 1, y + 2, w - 2, 9, WHITE)
    rect(im, x, y + 3, w, 7, WHITE)
    text(im, x + 2, y, s, PANEL, SMALL, None)
    return x + w + 2


def base(preview, info):
    im = Image.new('RGB', (240, 160), PANEL)
    im.paste(preview.crop((0, 0, 112, 112)), (0, 0))
    rect(im, 0, 112, 112, 48, PANEL)
    rect(im, 0, 112, 112, 1, EDGE)
    rect(im, 112, 0, 1, 160, EDGE)
    y = 115
    for line in info:
        text(im, 4, y, line, WHITE)
        y += 13
    return im


def footer(im, parts):
    x = 118
    for k, label in parts:
        x = key(im, x, 146, k)
        x = text(im, x, 146, label, GREY) + 5


def rows_view(preview):
    """the rows as now, each with how many of its pieces there are and its favourites marked; A opens the list"""
    im = base(preview, ["The game's own: a", "white flash as it", "grows."])
    text(im, 118, 1, 'BALL EFFECTS', GOLD, NORMAL)
    rows = [('PRESET', 'DEFAULT', ''), ('MATERIALIZATION', 'SOMERSAULT', '14/57'), ('ENERGY COLOR', 'TEAL', '24/64'),
            ('ENTRANCE PARTICLES', 'STAR SHOWER', '22/32'), ('RECALL EFFECT', 'BACKFLIP', '30/64'),
            ('SOUND ACCENT', 'SOFT CHIME', '9/16')]
    for i, (label, value, count) in enumerate(rows):
        y = 17 + i * 17
        on = i == 1
        if on:
            rect(im, 114, y - 1, 124, 17, BAND)
        text(im, 118, y - 2, label, GREY if on else DIM)
        if count:
            text(im, 236 - s2.measure(count, SMALL), y - 2, count, GREY if on else DIM)
        if on:
            glyph(im, 118, y + 10, LEFT, WHITE)
            x = text(im, 124, y + 6, value, WHITE)
            glyph(im, x + 2, y + 10, RIGHT, WHITE)
            x += 6
        else:
            x = text(im, 124, y + 6, value, WHITE)
        if i in (1, 3):
            glyph(im, x + 3, y + 9, STAR, GOLD)
    text(im, 118, 17 + 6 * 17, 'SAVE AS', DIM)
    text(im, 156, 17 + 6 * 17, 'MY PRESET 1', WHITE)
    text(im, 118, 17 + 6 * 17 + 13, 'APPLY TO PARTY', DIM)
    footer(im, [('A', 'LIST'), ('L', ''), ('R', 'FAVORITE')])
    return im


def kind_header(im, title, tab):
    """L < the kind's name > R, and five dots: which of the five kinds"""
    x = key(im, 116, 0, 'L')
    w = s2.measure(title, SMALL)
    text(im, 177 - w // 2, 0, title, GOLD)
    key(im, 238 - (s2.measure('R', SMALL) + 4), 0, 'R')
    for i in range(5):
        rect(im, 162 + i * 6, 25 - 3, 3, 2, GOLD if i == tab else DIM)


def list_view(preview, title, items, sel, top, owned, favs, new, tab, filt='ALL', info=None):
    """A on a row: every piece of its kind in a list beside the preview, which plays the one under the cursor"""
    im = base(preview, info or ["A cartwheel up out", "of the ball, landing", "facing the foe."])
    kind_header(im, title, tab)
    # how many had, the filter
    have = sum(1 for o in owned if o)
    text(im, 118, 13, '%d OF %d' % (have, len(items)), GREY)
    text(im, 236 - s2.measure('SHOW ' + filt, SMALL), 13, 'SHOW ' + filt, GOLD)
    rect(im, 114, 25, 124, 1, EDGE)
    vis = 9
    for r in range(vis):
        i = top + r
        if i >= len(items):
            break
        y = 27 + r * 13
        if i == sel:
            rect(im, 114, y, 118, 13, BAND)
        if i in favs:
            glyph(im, 117, y + 3, STAR, GOLD)
        if owned[i]:
            x = text(im, 126, y - 1, items[i], WHITE)
        else:
            glyph(im, 117, y + 3, LOCK, DIM)
            x = text(im, 126, y - 1, '- - - - -', DIM)
        if i in new and owned[i]:
            text(im, 232 - s2.measure('NEW', SMALL), y - 1, 'NEW', NEWC)
    # where in the list (a scroll bar)
    rect(im, 234, 27, 3, vis * 13, BAND)
    h = max(6, vis * 13 * vis // len(items))
    yb = 27 + (vis * 13 - h) * top // max(1, len(items) - vis)
    rect(im, 234, yb, 3, h, GREY)
    footer2(im)
    return im


def footer2(im):
    x = key(im, 118, 146, 'A')
    x = text(im, x, 146, 'PICK', GREY) + 4
    x = key(im, x, 146, 'SELECT')
    glyph(im, x, 149, STAR, GOLD)
    x += 11
    key(im, x, 146, 'START')


def color_view(preview, sel, owned, favs):
    """ENERGY COLOR: every colour at once as swatches (the colour's own brightest shade), its name at the top"""
    cols = color_table()
    im = base(preview, ["The color of its", "energy: beams, glows,", "motes, pixels and ink."])
    kind_header(im, 'ENERGY COLOR', 1)
    name = cols[sel][0] if sel < len(cols) else 'NEW COLOR'
    text(im, 118, 13, name, WHITE)
    s = '%d OF 64' % sum(1 for o in owned if o)
    text(im, 236 - s2.measure(s, SMALL), 13, s, GREY)
    rect(im, 114, 25, 124, 1, EDGE)
    for i in range(64):
        cx, cy = 118 + (i % 8) * 15, 29 + (i // 8) * 14
        if i < len(cols) and owned[i]:
            rect(im, cx, cy, 12, 11, SHADOW)
            rect(im, cx + 1, cy + 1, 10, 9, cols[i][1])
            if i in favs:
                glyph(im, cx + 3, cy + 2, STAR, WHITE)
        else:
            rect(im, cx + 1, cy + 1, 10, 9, BAND)
            glyph(im, cx + 3, cy + 2, LOCK, DIM)
        if i == sel:
            for (dx, dy, w, h) in ((-2, -2, 16, 1), (-2, 12, 16, 1), (-2, -2, 1, 15), (13, -2, 1, 15)):
                rect(im, cx + dx, cy + dy, w, h, GOLD)
    footer2(im)
    return im


def preset_view(preview):
    """PRESET: every preset with what's in it, read at a glance"""
    im = base(preview, ["Your own mixes and", "the ready-made ones,", "each with its parts."])
    text(im, 118, 1, 'PRESETS', GOLD, NORMAL)
    rect(im, 114, 15, 124, 1, EDGE)
    rows = [('MY PRESET 1', 'SOMERSAULT', 'STAR SHOWER', 'BACKFLIP', (14, 24, 31)),
            ('MY PRESET 2', 'GEYSER', 'SPLASH', 'DIVE', (4, 18, 31)),
            ('MY PRESET 3', '- empty -', '', '', None),
            ('HAUNTED', 'HAUNTED FADE', 'WISPS + EYES', 'SMOKE RETREAT', (19, 6, 31)),
            ('ROYALTY', 'PRISM', 'ROYAL SPARKLES', 'CURTAIN CALL', (27, 23, 15))]
    for i, (n, f, p, r, col) in enumerate(rows):
        y = 17 + i * 25
        if i == 0:
            rect(im, 114, y, 124, 25, BAND)
        if col:
            rect(im, 117, y + 3, 6, 6, SHADOW)
            rect(im, 118, y + 4, 4, 4, c5(*col))
        text(im, 126, y - 1, n, GOLD if i == 0 else WHITE)
        if f != '- empty -':
            text(im, 126, y + 10, f, GREY)
        else:
            text(im, 126, y + 10, f, DIM)
    footer(im, [('A', 'USE'), ('START', 'SAVE HERE')])
    return im


def main():
    prev = Image.open(sys.argv[1]).convert('RGB').resize((240, 160))
    out = sys.argv[2]
    os.makedirs(out, exist_ok=True)
    forms = names('sFormNames') + ['FALLING LEAF', 'MIRROR SHARD', 'PAPER CRANE', 'ORIGAMI FOLD', 'TIDE POOL',
                                   'NEON SIGN', 'COMET TRAIL', 'AURORA VEIL', 'RIBBON TWIRL', 'SNOW GLOBE']
    owned = [i < 20 or i in (22, 24, 27, 30) for i in range(len(forms))]
    shots = {
        '1_rows': rows_view(prev),
        '2_list_forms': list_view(prev, 'MATERIALIZATION', forms, 13, 9, owned, {13, 17, 26}, {27, 30}, 0),
        '3_list_favorites': list_view(prev, 'MATERIALIZATION', [forms[i] for i in (13, 17, 26)], 0, 0,
                                      [True] * 3, {0, 1, 2}, set(), 0, 'FAV'),
        '4_colors': color_view(prev, 23, [i < 41 for i in range(64)], {23, 4, 30}),
        '5_presets': preset_view(prev),
    }
    sheet = Image.new('RGB', (240 * 3 + 16, 160 * 2 + 8), (40, 40, 48))
    for n, (name, im) in enumerate(shots.items()):
        im.save(os.path.join(out, name + '.png'))
        sheet.paste(im, ((n % 3) * 248, (n // 3) * 168))
    sheet = sheet.resize((sheet.width * 2, sheet.height * 2), Image.NEAREST)
    sheet.save(os.path.join(out, 'sheet.png'))
    print('wrote', out)


if __name__ == '__main__':
    main()
