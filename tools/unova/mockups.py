#!/usr/bin/env python3
"""Mock-ups of UNOVA's new screens for the user (2026-11-14), drawn at the GBA's own 240x160 with the game's own font
(tools/stadium/restyle/s2.py's reader of graphics/moemon/gFont*.bin), its standard window frame
(graphics/text_window/1.png) and MOeMON pictures exported by the game (src/mp/mp_export.c, MOEMON_EXPORT_APP=DIR).

    python3 tools/unova/mockups.py EXPORT_DIR FIELD_SHOT OUT_DIR

FIELD_SHOT: a 480x320 screenshot of an UNOVA map (the C-GEAR is drawn over the field, as the POKeTCH is).
Writes cgear.png, cgear_ir.png, musical_dressup.png, musical_stage.png, musical_results.png, entralink_missions.png,
pass_powers.png at 2x, and sheet.png with all of them.
"""
import os
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'stadium', 'restyle'))
import s2  # noqa: E402

W, H = 240, 160
INK, SHADE = (72, 72, 72), (208, 208, 200)
WHITE = (255, 255, 255)
FRAME = Image.open(os.path.join(REPO, 'graphics', 'text_window', '1.png')).convert('RGB')
KEY = (98, 197, 98)


def text(im, x, y, s, color=INK, shadow=SHADE, font=s2.NORMAL, align='l'):
    face, sh = s2.text_masks(s, font)
    w = face.shape[1]
    if align == 'c':
        x -= w // 2
    elif align == 'r':
        x -= w
    px = im.load()
    for yy in range(face.shape[0]):
        for xx in range(w):
            X, Y = x + xx, y + yy
            if 0 <= X < im.width and 0 <= Y < im.height:
                if face[yy, xx]:
                    px[X, Y] = color
                elif sh[yy, xx] and shadow:
                    px[X, Y] = shadow
    return w


def window(im, x, y, w, h, fill=WHITE):
    """The game's standard frame round a box of w x h tiles' inside (x, y the frame's corner, in pixels)"""
    d = ImageDraw.Draw(im)
    d.rectangle([x + 8, y + 8, x + 8 + w * 8 - 1, y + 8 + h * 8 - 1], fill=fill)
    for ty in range(h + 2):
        for tx in range(w + 2):
            sx = 0 if tx == 0 else 16 if tx == w + 1 else 8
            sy = 0 if ty == 0 else 16 if ty == h + 1 else 8
            if sx == 8 and sy == 8:
                continue
            tile = FRAME.crop((sx, sy, sx + 8, sy + 8))
            for yy in range(8):
                for xx in range(8):
                    c = tile.getpixel((xx, yy))
                    if c != KEY:
                        im.putpixel((x + tx * 8 + xx, y + ty * 8 + yy), c)


def paste(im, pic, x, y):
    im.paste(pic, (x, y), pic)


_PALS = {}


def mon(exp, idx, size=64):
    """A MOeMON's picture: the export keeps colour indexes as grey (index * 17), the palette in data.json (RGB555)"""
    if not _PALS:
        import json
        for i, sp in enumerate(json.load(open(os.path.join(exp, 'data.json')))['species']):
            _PALS[i] = sp.get('pal', '')
    pal = _PALS.get(idx, '')
    cols = []
    for k in range(16):
        v = int(pal[4 * k:4 * k + 4] or '0', 16)
        cols.append(((v & 31) * 255 // 31, (v >> 5 & 31) * 255 // 31, (v >> 10 & 31) * 255 // 31, 255))
    p = Image.open(os.path.join(exp, 'front', '%d.png' % idx)).convert('RGBA')
    px = p.load()
    for y in range(p.height):
        for x in range(p.width):
            r, g, b, a = px[x, y]
            if a:
                px[x, y] = cols[r // 17]
    return p if size == 64 else p.resize((size, size), Image.NEAREST)


def icon(exp, idx):
    return Image.open(os.path.join(exp, 'icons', '%d.png' % idx)).convert('RGBA').crop((0, 0, 32, 32))


def heart(d, x, y, c):
    rows = ['.##.##.', '#######', '#######', '.#####.', '..###..', '...#...']
    for yy, row in enumerate(rows):
        for xx, ch in enumerate(row):
            if ch == '#':
                d.point((x + xx, y + yy), fill=c)


def cursor(im, x, y, color=INK):
    d = ImageDraw.Draw(im)
    d.polygon([(x, y), (x, y + 8), (x + 4, y + 4)], fill=color)


def rrect(d, box, r, fill, outline=None):
    d.rounded_rectangle(box, r, fill=fill, outline=outline)


# ------------------------------------------------------------------------------------------------- the C-GEAR
def hexes(im, box, base, light, dark):
    d = ImageDraw.Draw(im)
    x0, y0, x1, y1 = box
    d.rectangle(box, fill=base)
    r = 7
    for row in range(-1, (y1 - y0) // 11 + 2):
        for col in range(-1, (x1 - x0) // 13 + 2):
            cx = x0 + col * 13 + (6 if row % 2 else 0)
            cy = y0 + row * 11
            pts = [(cx, cy - r), (cx + 6, cy - r // 2 - 1), (cx + 6, cy + r // 2 + 1), (cx, cy + r),
                   (cx - 6, cy + r // 2 + 1), (cx - 6, cy - r // 2 - 1)]
            pts = [(min(max(px, x0), x1), min(max(py, y0), y1)) for px, py in pts]
            d.polygon(pts, outline=dark, fill=light if (row * 7 + col * 3) % 11 == 0 else None)


def cgear_body(field):
    im = field.copy()
    d = ImageDraw.Draw(im)
    rrect(d, [12, 6, 227, 153], 6, (56, 56, 64), (24, 24, 32))
    rrect(d, [14, 8, 225, 151], 5, None, (96, 96, 112))
    d.rectangle([20, 12, 219, 23], fill=(32, 32, 40))
    text(im, 24, 10, 'C-GEAR', (232, 232, 240), (88, 88, 104), s2.SMALL)
    text(im, 196, 10, '14:10', (232, 232, 240), (88, 88, 104), s2.SMALL, 'r')
    d.rectangle([201, 15, 214, 20], outline=(232, 232, 240))
    d.rectangle([203, 17, 211, 18], fill=(120, 216, 120))
    d.rectangle([215, 16, 216, 19], fill=(232, 232, 240))
    hexes(im, (20, 26, 219, 112), (40, 64, 112), (88, 136, 200), (64, 96, 160))
    return im


def cgear_buttons(im, sel):
    d = ImageDraw.Draw(im)
    names = [('IR', 'FRIEND'), ('WIRELESS', 'CO-OP'), ('ENTRALINK', '')]
    for i, (a, b) in enumerate(names):
        x = 22 + i * 66
        on = i == sel
        rrect(d, [x, 116, x + 62, 145], 4, (248, 224, 120) if on else (216, 216, 224), (24, 24, 32))
        rrect(d, [x + 2, 118, x + 60, 143], 3, None, (255, 248, 200) if on else WHITE)
        text(im, x + 31, 118 if b else 123, a, INK, SHADE, s2.NORMAL if len(a) < 9 else s2.SMALL, 'c')
        if b:
            text(im, x + 31, 131, b, (96, 96, 112), None, s2.SMALL, 'c')


def mock_cgear(field):
    im = cgear_body(field)
    d = ImageDraw.Draw(im)
    # the friends near: co-op players in the room, as Black shows passers-by
    for k, (nm, x, y) in enumerate([('ARIA', 64, 50), ('MIKE', 150, 74)]):
        rrect(d, [x - 2, y - 2, x + 42, y + 13], 3, (248, 248, 248), (24, 24, 32))
        text(im, x + 20, y - 1, nm, INK, SHADE, s2.SMALL, 'c')
    text(im, 120, 92, 'Friends in the room: 2', (232, 240, 255), (40, 56, 96), s2.SMALL, 'c')
    cgear_buttons(im, 0)
    return im


def mock_cgear_ir(field):
    im = cgear_body(field)
    cgear_buttons(im, 0)
    window(im, 120, 30, 10, 6)
    text(im, 136, 38, 'TRADE')
    text(im, 136, 54, 'BATTLE')
    text(im, 136, 70, 'CANCEL')
    cursor(im, 130, 42)
    window(im, 0, 112, 28, 4)
    text(im, 10, 120, 'ARIA is in front of you.')
    text(im, 10, 136, 'What would you like to do?')
    return im


# ------------------------------------------------------------------------------------------------- MUSICALS
PROPS = [
    ('top hat', [(2, 4, 13, 10, (40, 40, 48)), (0, 10, 15, 12, (40, 40, 48)), (2, 8, 13, 9, (200, 48, 64))]),
    ('ribbon', [(1, 5, 6, 10, (232, 96, 152)), (9, 5, 14, 10, (232, 96, 152)), (6, 6, 9, 9, (248, 168, 200))]),
    ('glasses', [(1, 6, 6, 10, (40, 40, 48)), (9, 6, 14, 10, (40, 40, 48)), (6, 7, 9, 8, (40, 40, 48))]),
    ('flower', [(4, 3, 11, 12, (248, 208, 64)), (6, 5, 9, 10, (232, 120, 48))]),
    ('mic', [(5, 2, 10, 7, (160, 160, 176)), (7, 7, 8, 14, (40, 40, 48))]),
    ('wand', [(7, 5, 8, 15, (120, 72, 40)), (5, 1, 10, 6, (248, 232, 96))]),
    ('crown', [(2, 6, 13, 11, (248, 200, 48)), (2, 3, 3, 6, (248, 200, 48)), (7, 2, 8, 6, (248, 200, 48)),
               (12, 3, 13, 6, (248, 200, 48))]),
    ('scarf', [(1, 6, 14, 9, (72, 136, 232)), (10, 9, 13, 14, (72, 136, 232))]),
    ('mask', [(1, 5, 14, 10, (248, 248, 248)), (3, 6, 6, 8, (40, 40, 48)), (9, 6, 12, 8, (40, 40, 48))]),
    ('bow tie', [(2, 6, 7, 10, (200, 48, 64)), (8, 6, 13, 10, (200, 48, 64)), (6, 7, 9, 9, (120, 24, 40))]),
    ('star', [(6, 2, 9, 13, (248, 216, 64)), (2, 6, 13, 9, (248, 216, 64))]),
    ('leaf', [(3, 5, 12, 10, (88, 176, 72)), (7, 3, 8, 12, (56, 120, 48))]),
]


def prop(name):
    im = Image.new('RGBA', (16, 16))
    d = ImageDraw.Draw(im)
    for x0, y0, x1, y1, c in dict(PROPS)[name]:
        d.rectangle([x0, y0, x1, y1], fill=c + (255,))
    # a dark outline, as the game's pictures have
    px = im.load()
    out = im.copy()
    po = out.load()
    for y in range(16):
        for x in range(16):
            if px[x, y][3] == 0 and any(0 <= x + dx < 16 and 0 <= y + dy < 16 and px[x + dx, y + dy][3]
                                        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                po[x, y] = (32, 32, 40, 255)
    return out


def mock_dressup(exp):
    im = Image.new('RGB', (W, H), (248, 232, 216))
    d = ImageDraw.Draw(im)
    for y in range(0, H, 8):                       # the dressing room's striped wallpaper
        for x in range(0, W, 16):
            d.rectangle([x, y, x + 7, y + 7], fill=(240, 216, 200) if (y // 8) % 2 == 0 else (248, 232, 216))
    d.rectangle([0, 0, W - 1, 15], fill=(176, 64, 96))
    text(im, 6, 0, 'DRESS UP', WHITE, (112, 32, 64))
    text(im, 234, 0, '1:12', WHITE, (112, 32, 64), align='r')
    # the mirror and the POKeMON on its stand
    d.ellipse([8, 104, 100, 124], fill=(192, 152, 120), outline=(120, 88, 64))
    rrect(d, [14, 20, 94, 110], 6, (216, 232, 240), (120, 88, 64))
    snivy = mon(exp, 520)
    paste(im, snivy, 22, 40)
    paste(im, prop('top hat'), 46, 34)
    paste(im, prop('bow tie'), 48, 70)
    text(im, 54, 106, 'SNIVY', INK, SHADE, s2.SMALL, 'c')
    # the prop case
    window(im, 104, 18, 15, 11)
    tabs = ['HEAD', 'FACE', 'BODY', 'HAND']
    for i, t in enumerate(tabs):
        x = 112 + i * 30
        d.rectangle([x, 26, x + 28, 35], fill=(176, 64, 96) if i == 0 else (232, 216, 216))
        text(im, x + 14, 24, t, WHITE if i == 0 else INK, None, s2.SMALL, 'c')
    names = [n for n, _ in PROPS]
    for i in range(12):
        x, y = 114 + (i % 4) * 29, 42 + (i // 4) * 22
        d.rectangle([x, y, x + 21, y + 19], fill=(248, 240, 240), outline=(200, 184, 184))
        paste(im, prop(names[i]), x + 3, y + 2)
    d.rectangle([113, 41, 136, 62], outline=(232, 64, 96))
    d.rectangle([112, 40, 137, 63], outline=(232, 64, 96))
    window(im, 0, 128, 28, 2)
    text(im, 10, 136, 'Put props on SNIVY! START: done.')
    return im


def stage_bg():
    im = Image.new('RGB', (W, H), (24, 16, 40))
    d = ImageDraw.Draw(im)
    for i in range(12):                            # the back wall's lights
        d.rectangle([20 + i * 18, 6, 22 + i * 18, 8], fill=(248, 216, 120))
    d.polygon([(0, 98), (W, 98), (W, 128), (0, 128)], fill=(152, 96, 56))
    for y in range(100, 128, 6):
        d.line([(0, y), (W, y)], fill=(128, 80, 48))
    for x0 in (60, 120, 180):                      # spotlights
        d.polygon([(x0 - 6, 0), (x0 + 6, 0), (x0 + 30, 112), (x0 - 30, 112)], fill=(56, 48, 72))
        d.ellipse([x0 - 30, 102, x0 + 30, 118], fill=(200, 160, 104))
    for side in (0, 1):                            # the curtains
        x = 0 if side == 0 else W - 26
        d.rectangle([x, 0, x + 25, 128], fill=(184, 32, 48))
        for k in range(0, 26, 6):
            d.line([(x + k, 0), (x + k, 128)], fill=(136, 16, 32))
    d.rectangle([0, 0, W - 1, 10], fill=(184, 32, 48))
    d.rectangle([0, 128, W - 1, H - 1], fill=(16, 8, 24))
    for k in range(16):                            # the audience
        cx = 8 + k * 15
        d.ellipse([cx - 6, 132, cx + 6, 146], fill=(40, 32, 56))
        d.rectangle([cx - 8, 142, cx + 8, H - 1], fill=(40, 32, 56))
    return im


def mock_stage(exp):
    im = stage_bg()
    who = [(520, 'YOU', 'top hat'), (597, 'ARIA', 'ribbon'), (612, 'MIKE', 'glasses'), (574, 'NPC', 'flower')]
    for i, (sp, nm, pr) in enumerate(who):
        x = 22 + i * 50
        p = mon(exp, sp, 48)
        paste(im, p, x, 58 + (6 if i % 2 else 0))
        paste(im, prop(pr), x + 16, 52 + (6 if i % 2 else 0))
        ImageDraw.Draw(im).rectangle([x + 4, 114, x + 44, 124], fill=(248, 248, 248), outline=(24, 24, 32))
        text(im, x + 24, 112, nm, INK, None, s2.SMALL, 'c')
    d = ImageDraw.Draw(im)
    for hx, hy in ((40, 40), (100, 30), (176, 44)):  # hearts from the audience
        heart(d, hx, hy, (248, 112, 160))
    text(im, 120, 12, 'TODAY\'S SHOW', (248, 232, 160), (96, 40, 32), align='c')
    text(im, 236, 146, 'A: SHOW OFF!', WHITE, (24, 24, 32), s2.SMALL, 'r')
    return im


def mock_results(exp):
    im = Image.new('RGB', (W, H), (40, 24, 56))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, W - 1, 15], fill=(176, 64, 96))
    text(im, 120, 0, 'RESULTS', WHITE, (112, 32, 64), align='c')
    rows = [(520, 'YOU', 5, 'top hat'), (612, 'MIKE', 4, 'glasses'), (597, 'ARIA', 3, 'ribbon'), (574, 'NPC', 1, 'flower')]
    for i, (sp, nm, hearts, pr) in enumerate(rows):
        y = 20 + i * 28
        rrect(d, [6, y, 233, y + 25], 3, (248, 240, 232) if i == 0 else (232, 224, 232), (24, 24, 32))
        text(im, 12, y + 5, '%d' % (i + 1), (200, 48, 64) if i == 0 else INK, SHADE)
        ic = icon(exp, sp)
        paste(im, ic, 22, y - 5)
        text(im, 56, y + 5, nm)
        for k in range(5):
            heart(d, 112 + k * 14, y + 7, (232, 72, 120) if k < hearts else (200, 184, 200))
        if i == 0:
            paste(im, prop('crown'), 210, y + 4)
    window(im, 0, 128, 28, 2)
    text(im, 10, 136, 'YOU won! A prop was received!')
    return im


# ------------------------------------------------------------------------------------------------- the ENTRALINK
def mock_missions():
    im = Image.new('RGB', (W, H), (232, 240, 216))
    d = ImageDraw.Draw(im)
    for y in range(0, H, 4):                       # the ENTRALINK's dreamy forest light
        d.line([(0, y), (W, y)], fill=(216 - y // 10, 232 - y // 12, 200 - y // 10))
    d.rectangle([0, 0, W - 1, 15], fill=(64, 120, 72))
    text(im, 6, 0, 'ENTRALINK MISSIONS', WHITE, (32, 72, 40))
    window(im, 0, 16, 18, 13)
    missions = [('Find hidden items!', '3:00'), ('Catch a POKeMON fast!', '2:00'),
                ('Gather fruit!', '3:00'), ('Battle 3 trainers!', '4:00'), ('Take a picture!', '2:00')]
    for i, (t, tm) in enumerate(missions):
        y = 24 + i * 16
        text(im, 16, y, t.replace('POKeMON', 'POKéMON'), INK, SHADE, s2.SMALL)
        text(im, 150, y, tm, INK, SHADE, s2.SMALL, 'r')
    cursor(im, 9, 27)
    window(im, 160, 16, 8, 13)
    text(im, 168, 22, 'JOINING', INK, SHADE, s2.SMALL)
    for i, nm in enumerate(('YOU', 'ARIA', 'MIKE')):
        text(im, 172, 38 + i * 14, nm, INK, SHADE, s2.SMALL)
    text(im, 168, 92, 'Room:', (96, 96, 112), None, s2.SMALL)
    text(im, 168, 104, '3 / 4', (96, 96, 112), None, s2.SMALL)
    window(im, 0, 128, 28, 2)
    text(im, 10, 136, 'Choose a mission for the room!')
    return im


def mock_powers():
    im = Image.new('RGB', (W, H), (232, 240, 216))
    d = ImageDraw.Draw(im)
    d.rectangle([0, 0, W - 1, 15], fill=(64, 120, 72))
    text(im, 6, 0, 'PASS POWERS', WHITE, (32, 72, 40))
    text(im, 234, 0, 'SHARE: ON', WHITE, (32, 72, 40), s2.SMALL, 'r')
    window(im, 0, 16, 28, 12)
    powers = [('ENCOUNTER POWER', 'Lv. 1', 3), ('EXP. POINT POWER', 'Lv. 2', 2), ('PRIZE MONEY POWER', 'Lv. 1', 1),
              ('CAPTURE POWER', 'Lv. 3', 3), ('HATCHING POWER', 'Lv. 1', 2), ('FRIENDSHIP POWER', 'Lv. 2', 1)]
    for i, (nm, lv, n) in enumerate(powers):
        y = 24 + i * 15
        text(im, 16, y, nm, INK, SHADE, s2.SMALL)
        text(im, 150, y, lv, INK, SHADE, s2.SMALL)
        for k in range(3):
            c = (88, 176, 72) if k < n else (200, 208, 192)
            d.ellipse([186 + k * 12, y + 4, 193 + k * 12, y + 11], fill=c, outline=(40, 72, 40))
    cursor(im, 9, 27)
    window(im, 0, 128, 28, 2)
    text(im, 10, 136, 'Use it for the whole room?')
    return im


def main():
    exp, shot, out = sys.argv[1], sys.argv[2], sys.argv[3]
    os.makedirs(out, exist_ok=True)
    field = Image.open(shot).convert('RGB').resize((W, H), Image.NEAREST)
    field = Image.blend(field, Image.new('RGB', (W, H), (0, 0, 0)), 0.25)
    shots = [('cgear', mock_cgear(field)), ('cgear_ir', mock_cgear_ir(field)),
             ('musical_dressup', mock_dressup(exp)), ('musical_stage', mock_stage(exp)),
             ('musical_results', mock_results(exp)), ('entralink_missions', mock_missions()),
             ('pass_powers', mock_powers())]
    for name, im in shots:
        im.resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(out, name + '.png'))
    cols = 2
    sheet = Image.new('RGB', (cols * (W * 2 + 8) + 8, ((len(shots) + 1) // cols) * (H * 2 + 8) + 8), (32, 32, 40))
    for i, (name, im) in enumerate(shots):
        sheet.paste(im.resize((W * 2, H * 2), Image.NEAREST),
                    (8 + (i % cols) * (W * 2 + 8), 8 + (i // cols) * (H * 2 + 8)))
    sheet.save(os.path.join(out, 'sheet.png'))


if __name__ == '__main__':
    main()
