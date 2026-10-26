#!/usr/bin/env python3
"""The battle POKé BALLS from the HEARTGOLD / SOULSILVER sheet (hgss_sheet.png, ripped by Sunday-Doneko).

The sheet: one 16 px column per ball in HGSS order, on its own background color; rows 0-7 (16 px each) the ball
spinning (row 0 = the closed ball, as in menus), row 8 (y 128-143) the ball squashed by a bounce (not used), rows 9-10
(y 144-167) the ball open. The last two columns are the
SAFARI ZONE rock and mud (not used).

Writes graphics/balls/<name>.png for the 12 Hoenn balls and the special balls (MoeGatcha): 16x352, eleven 16x32 frames
(pokeball.c, BALL_FRAME_*): 0 closed (the first spinning frame), 1 open, 2 empty (the ball is gone once the Moemon is out), 3-10 spinning.
The ball's middle stays where the old 16x16 balls had theirs (8, 8), 8 px lower in the taller frame: rest and spin
frames are centered at (8, 16), the open ball's base on the closed ball's.
One 16-color palette per ball (index 0 transparent). POKé BALL's three reds come first (slots 1-3, dark to light: the
ball colors recolor them, mp_ballstyle.c) and it uses no slot past 8 (decorations take 9-15).

usage: tools/moemon_balls/hgss_balls.py   (from the repo root)
"""
import os, sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(REPO, 'graphics', 'balls')
COLUMNS = ['master', 'ultra', 'great', 'poke', 'safari', 'net', 'dive', 'nest', 'repeat', 'timer', 'luxury',
           'premier', 'dusk', 'heal', 'quick', 'cherish', 'fast', 'level', 'lure', 'heavy', 'love', 'friend', 'moon',
           'sport', 'park']
WANTED = ['master', 'ultra', 'great', 'poke', 'safari', 'net', 'dive', 'nest', 'repeat', 'timer', 'luxury', 'premier',
          'love', 'friend', 'fast', 'level', 'lure', 'heavy', 'moon', 'heal', 'dusk', 'quick']
FRAMES = 11
FH = 32
# (source rect top, height) -> destination y in the frame
REST = (0, 16, 8)   # the first row: the closed ball
OPEN = (144, 24, 0)
SPIN = [(r * 16, 16, 8) for r in range(8)]


def main():
    sheet = Image.open(os.path.join(HERE, 'hgss_sheet.png')).convert('RGBA')
    for name in WANTED:
        c = COLUMNS.index(name)
        x0 = c * 16
        bg = sheet.getpixel((x0, 0))
        frames = [REST, OPEN, None] + SPIN
        canvas = [[None] * 16 for _ in range(FRAMES * FH)]
        for f, spec in enumerate(frames):
            if spec is None:
                continue
            top, h, dy = spec
            for y in range(h):
                for x in range(16):
                    p = sheet.getpixel((x0 + x, top + y))
                    if p == bg or p[3] == 0:
                        continue
                    ty = f * FH + dy + y
                    assert 0 <= dy + y < FH, (name, f)
                    canvas[ty][x] = p[:3]
        colors = {}
        for row in canvas:
            for p in row:
                if p is not None:
                    colors[p] = colors.get(p, 0) + 1
        def lum(c):
            return c[0] * 3 + c[1] * 6 + c[2]
        order = sorted(colors, key=lambda c: -colors[c])
        if name == 'poke':
            reds = sorted([c for c in colors if c[0] > c[1] + 60 and c[0] > c[2] + 60], key=lum)
            assert len(reds) == 3, reds
            order = reds + [c for c in order if c not in reds]
            assert len(order) <= 8, len(order)
        assert len(order) <= 15, (name, len(order))
        pal = [bg[:3]] + order + [(0, 0, 0)] * (15 - len(order))
        idx = {c: i for i, c in enumerate(pal) if i > 0}
        out = Image.new('P', (16, FRAMES * FH), 0)
        out.putpalette([v for c in pal for v in c])
        px = out.load()
        for y, row in enumerate(canvas):
            for x, p in enumerate(row):
                if p is not None:
                    px[x, y] = idx[p]
        out.save(os.path.join(OUT, name + '.png'), transparency=0)
        print('%-8s %2d colors' % (name, len(order)))


if __name__ == '__main__':
    main()
