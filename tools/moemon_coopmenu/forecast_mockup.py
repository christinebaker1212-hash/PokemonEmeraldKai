#!/usr/bin/env python3
"""A mock-up of the CO-OP menu's FORECAST (the user, 2026-11-24: "a weather forecast to the CO-OP menu, to see what the
weather will be like all day, and for the next 7 days"), drawn with the game's own window (from a DAILY screenshot),
its own fonts and the weather icons the screen would use, at 1x and 3x.

usage: tools/moemon_coopmenu/forecast_mockup.py DAILY_SCREENSHOT.png OUT_DIR
"""
import os
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# the screen's sixteen colours: the window's text ones, and the weather's
INK = (96, 96, 96)
SHADOW = (208, 208, 200)
PAPER = (255, 255, 255)
COL = {
    'o': (240, 136, 40),    # the sun's rays
    'y': (248, 216, 64),    # the sun
    'w': (255, 248, 200),   # its light / the moon
    'm': (232, 224, 160),   # the moon's shade
    'c': (216, 224, 232),   # a cloud
    'g': (136, 144, 160),   # a cloud's edge
    'd': (96, 104, 120),    # a storm cloud
    'b': (64, 136, 232),    # rain
    'i': (200, 232, 248),   # hail
    't': (216, 176, 112),   # sand
    'T': (168, 128, 72),    # sand's shade
    'n': (224, 228, 240),   # a night hour's band
}

ICONS = {
    'sun': ["o..oo..o",
            "...yy...",
            "..yyyy..",
            "oyywyyyo",
            "oyyyyyyo",
            "..yyyy..",
            "...yy...",
            "o..oo..o"],
    'fair': ["...o....",
             ".o.yy.o.",
             "..yyyy..",
             ".oyycgg.",
             "..ygcccg",
             ".gccccccg"[:8],
             "gcccccc.",
             ".gggggg."],
    'moon': ["..mww...",
             ".mww....",
             "mww.....",
             "mww.....",
             "mww.....",
             "mwww...m",
             ".mwwwwm.",
             "..mmmm.."],
    'rain': ["..ggg...",
             ".gcccgg.",
             "gcccccccg"[:8],
             "gggggggg",
             ".b..b..b",
             "b..b..b.",
             ".b..b..b",
             "........"],
    'storm': ["..ddd...",
              ".dgggdd.",
              "dggggggd",
              "dddydddd",
              ".b.yy.b.",
              "b..y..b.",
              "..y...b.",
              "........"],
    'hail': ["..ggg...",
             ".gcccgg.",
             "gccccccg",
             "gggggggg",
             ".i..i..i",
             "........",
             "i..i..i.",
             "........"],
    'sand': ["........",
             ".tttTT..",
             "t.....T.",
             "..tttT.T",
             ".T....t.",
             "T.ttTT..",
             ".t......",
             "..tTT..."],
}


def charmap():
    out = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        line = line.split('@')[0].strip()
        if len(line) >= 3 and line[0] == "'" and line[2] == "'" and '=' in line:
            v = line.split('=')[1].split()
            if len(v) == 1 and len(v[0]) == 2:
                out.setdefault(line[1], int(v[0], 16))
    return out


def widths(name):
    src = open(os.path.join(ROOT, 'src/fonts.c')).read()
    at = src.index('gFont%sLatinGlyphWidths[] = {' % name)
    body = src[at:src.index('};', at)].split('{', 1)[1]
    return [int(x) for x in body.replace('\n', ' ').split(',') if x.strip()]


CM = charmap()
FONTS = {'n': (Image.open(os.path.join(ROOT, 'graphics/fonts/latin_normal.png')), widths('Normal')),
         's': (Image.open(os.path.join(ROOT, 'graphics/fonts/latin_small.png')), widths('Small'))}


def text(im, x, y, s, font='n', ink=INK, shadow=SHADOW, align='l'):
    sheet, wd = FONTS[font]
    codes = [CM.get(ch, 0) for ch in s]
    w = sum(wd[c] for c in codes)
    if align == 'r':
        x -= w
    for c in codes:
        cell = sheet.crop(((c % 16) * 16, (c // 16) * 16, (c % 16) * 16 + 16, (c // 16) * 16 + 16))
        for yy in range(16):
            for xx in range(wd[c]):
                p = cell.getpixel((xx, yy))
                if p == 1 and ink:
                    im.putpixel((x + xx, y + yy), ink)
                elif p == 2 and shadow:
                    im.putpixel((x + xx, y + yy), shadow)
        x += wd[c]
    return w


def icon(im, x, y, name):
    for yy, row in enumerate(ICONS[name]):
        for xx, ch in enumerate(row[:8]):
            if ch != '.':
                im.putpixel((x + xx, y + yy), COL[ch])


def rect(im, x0, y0, x1, y1, col):
    for y in range(y0, y1):
        for x in range(x0, x1):
            im.putpixel((x, y), col)


def main():
    daily, out = sys.argv[1], sys.argv[2]
    shot = Image.open(daily).convert('RGB')
    scale = shot.width // 240
    base = shot.resize((240, 160), Image.NEAREST)
    im = base.copy()
    # the windows' insides cleared (the frames kept), the help keys kept from DAILY's box
    keys = base.crop((140, 138, 232, 154))
    rect(im, 6, 6, 234, 114, PAPER)
    rect(im, 6, 124, 234, 155, PAPER)
    # title, and the place (LEFT / RIGHT goes round the region's places)
    text(im, 9, 5, 'FORECAST')
    text(im, 230, 9, '{ ROUTE 102 }'.replace('{', '<').replace('}', '>'), font='s', align='r')
    rect(im, 8, 21, 232, 22, SHADOW)
    # TODAY: every hour, the night's hours on a pale band, now marked
    today = (['moon'] * 4 + ['rain'] * 2 + ['fair'] * 3 + ['sun'] * 4 + ['fair'] * 2 + ['rain'] * 2 + ['storm'] +
             ['rain'] + ['fair'] * 1 + ['moon'] * 4)[:24]
    now = 13
    text(im, 20, 22, 'TODAY', font='s')
    text(im, 12, 22, '>', font='s')   # (the cursor: this row's chosen)
    text(im, 230, 22, 'NOW 1 PM: FAIR', font='s', align='r', ink=(224, 96, 48))
    x0 = 12
    for h in range(24):
        cx = x0 + h * 9
        if h < 6 or h >= 20:
            rect(im, cx, 33, cx + 9, 43, COL['n'])
        if h == now:
            rect(im, cx, 32, cx + 9, 44, (248, 200, 120))
            rect(im, cx + 1, 33, cx + 8, 43, PAPER)
        icon(im, cx, 34, today[h])
    for h, lab in ((0, '12A'), (6, '6A'), (12, '12P'), (18, '6P')):
        text(im, x0 + h * 9, 39, lab, font='s')
    # the next 7 days: night, morning, afternoon, evening, and a word
    days = [('WED', ['moon', 'sun', 'sun', 'fair'], 'SUNNY'),
            ('THU', ['moon', 'rain', 'rain', 'fair'], 'MORNING SHOWERS'),
            ('FRI', ['rain', 'storm', 'rain', 'rain'], 'STORMS'),
            ('SAT', ['moon', 'fair', 'sun', 'sun'], 'BRIGHT SUN'),
            ('SUN', ['moon', 'fair', 'fair', 'rain'], 'RAIN LATER'),
            ('MON', ['rain', 'rain', 'fair', 'moon'], 'CLEARING'),
            ('TUE', ['moon', 'sun', 'fair', 'fair'], 'FAIR')]
    y = 52
    for name, parts, word in days:
        text(im, 20, y - 3, name, font='s')
        for k, p in enumerate(parts):
            icon(im, 50 + k * 11, y, p)
        text(im, 98, y - 3, word, font='s')
        y += 9
    # the box: the chosen row in words, the keys
    text(im, 9, 123, 'Fair now. Rain from 4 PM to 7 PM,')
    text(im, 9, 137, 'a storm at 6 PM.', font='n')
    im.paste(keys, (140, 138))
    os.makedirs(out, exist_ok=True)
    im.save(os.path.join(out, 'forecast_1x.png'))
    im.resize((240 * 3, 160 * 3), Image.NEAREST).save(os.path.join(out, 'forecast.png'))
    # the icons on their own, big
    sheet = Image.new('RGB', (len(ICONS) * 12 + 4, 14), PAPER)
    for k, n in enumerate(ICONS):
        icon(sheet, 4 + k * 12, 3, n)
    sheet.resize((sheet.width * 8, sheet.height * 8), Image.NEAREST).save(os.path.join(out, 'forecast_icons.png'))


if __name__ == '__main__':
    main()
