#!/usr/bin/env python3
"""Moemon VMU pet: mock-ups of Kai's side (the PC's VM2 option and the VM2 box), drawn over the game's own screens
(harness shots at 2x: the PC menu and the box screen) with the game's own font (gbafont.py), icons and pictures.

usage: kai_mockups.py PC_MENU.png BOXES.png OUT.png
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.join(HERE, '..', '..')
sys.path.insert(0, HERE)
import gbafont  # noqa: E402
from PIL import Image, ImageDraw  # noqa: E402

FG, SHADOW, WHITE = (98, 98, 98), (213, 213, 205), (255, 255, 255)
GREY_FG, GREY_SHADOW = (189, 189, 189), (238, 238, 230)


def jasc(path):
    lines = open(path).read().split('\n')
    return [tuple(int(v) for v in l.split()) for l in lines[3:3 + int(lines[2])]]


def icon(name, pal=1):
    """A box icon (first frame, 32x32) in its icon palette, transparent where index 0."""
    im = Image.open(os.path.join(REPO, 'graphics/pokemon/%s/icon.png' % name))
    cols = jasc(os.path.join(REPO, 'graphics/pokemon/icon_palettes/icon_palette_%d.pal' % pal))
    out = Image.new('RGBA', (32, 32))
    for y in range(32):
        for x in range(32):
            v = im.getpixel((x, y))
            if v:
                out.putpixel((x, y), cols[v] + (255,))
    return out


def front(name):
    im = Image.open(os.path.join(REPO, 'graphics/pokemon/%s/front.png' % name)).convert('RGBA')
    pal = Image.open(os.path.join(REPO, 'graphics/pokemon/%s/front.png' % name))
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            if pal.getpixel((x, y)) == 0:
                px[x, y] = (0, 0, 0, 0)
    return im.crop((0, 0, 64, 64))


def x2(im):
    return im.resize((im.width * 2, im.height * 2), Image.NEAREST)


def pc_menu(shot, greyed):
    """The PC menu with VM2 above LOG OFF (rows are 32 pixels at 2x; the text at x 32)."""
    im = shot.copy()
    # the window grows a row: the LOG OFF row and the frame under it move down 32
    im.paste(shot.crop((0, 146, 200, 192)), (0, 178))
    # the VM2 row: GLOBAL PC's row, its text wiped
    band = shot.crop((0, 114, 200, 146))
    px = band.load()
    for y in range(band.height):
        for x in range(20, 182):
            if px[x, y] in (FG, SHADOW):
                px[x, y] = WHITE
    im.paste(band, (0, 146))
    gbafont.draw(im, 32, 146, 'VM2', GREY_FG if greyed else FG, GREY_SHADOW if greyed else SHADOW)
    return im


def message(im, lines):
    """The field's message box (y 228-308 at 2x), its words replaced."""
    d = ImageDraw.Draw(im)
    d.rectangle([20, 236, 456, 300], fill=WHITE)
    for i, t in enumerate(lines):
        gbafont.draw(im, 28, 238 + i * 32, t)
    return im


def vmu_wallpaper():
    """The VM2 box's wallpaper at GBA size (153x115): a VMU drawn as the games draw things, its screen the box's one
    place."""
    w, h = 153, 115
    im = Image.new('RGBA', (w, h), (184, 200, 240, 255))
    d = ImageDraw.Draw(im)
    for y in range(0, h, 8):            # a soft diamond lattice, as Emerald's wallpapers
        for x in range((y // 8) % 2 * 8, w, 16):
            d.point((x, y), fill=(168, 184, 232))
            d.point((x + 1, y), fill=(168, 184, 232))
    bx, by, bw, bh = 41, 6, 72, 104      # the VMU's body
    body = [(bx + 10, by), (bx + bw - 10, by), (bx + bw, by + 10), (bx + bw, by + bh - 34), (bx + bw - 6, by + bh - 14),
            (bx + bw - 20, by + bh), (bx + 20, by + bh), (bx + 6, by + bh - 14), (bx, by + bh - 34), (bx, by + 10)]
    d.polygon(body, fill=(232, 236, 244), outline=(56, 64, 104))
    d.line(body + [body[0]], fill=(56, 64, 104), width=2)
    d.rounded_rectangle([bx + 4, by + 4, bx + bw - 4, by + bh - 50], 6, fill=(200, 208, 224))
    sx, sy, sw, sh = bx + 12, by + 10, 48, 40   # the LCD
    d.rectangle([sx - 2, sy - 2, sx + sw + 2, sy + sh + 2], fill=(72, 80, 112))
    d.rectangle([sx, sy, sx + sw, sy + sh], fill=(164, 196, 148))
    d.line([sx, sy + sh, sx + sw, sy + sh], fill=(132, 164, 120))
    cx, cy = bx + 18, by + 76            # the D-pad
    d.rectangle([cx - 9, cy - 3, cx + 9, cy + 3], fill=(72, 80, 112))
    d.rectangle([cx - 3, cy - 9, cx + 3, cy + 9], fill=(72, 80, 112))
    for ax, ay, l in ((bx + 52, by + 72, 'A'), (bx + 60, by + 82, 'B')):
        d.ellipse([ax - 6, ay - 6, ax + 6, ay + 6], fill=(216, 88, 88), outline=(120, 40, 56))
    for i in range(2):                   # SLEEP / MODE
        d.rounded_rectangle([bx + 22 + i * 18, by + 94, bx + 34 + i * 18, by + 98], 2, fill=(152, 160, 184))
    return im, (sx + sw // 2 - 16, sy + sh // 2 - 18)


def vm2_box(shot, mon, name_lines):
    """The box screen at the VM2's box: its name VM2, the VMU wallpaper with one place, mon's icon there, and the
    PKMN DATA panel showing it."""
    im = shot.copy().convert('RGBA')
    wall, slot = vmu_wallpaper()
    W = x2(wall)
    im.paste(W, (166, 86))
    # the box's name plate: plain, VM2 in the middle
    d = ImageDraw.Draw(im)
    d.rectangle([200, 42, 438, 74], fill=(184, 200, 240))
    d.rectangle([200, 42, 438, 45], fill=(208, 220, 248))
    t = 'VM2'
    gbafont.draw(im, 319 - gbafont.width(t), 42, t)
    # the MOeMON / Egg in the VMU's screen, the hand over it
    ic = x2(icon(mon[0], mon[1]))
    ix, iy = 166 + slot[0] * 2, 86 + slot[1] * 2
    im.alpha_composite(ic, (ix, iy))
    hand = shot.crop((182, 138, 218, 176)).convert('RGBA')
    hp = hand.load()
    bg = shot.getpixel((184, 140))
    for y in range(hand.height):
        for x in range(hand.width):
            r, g, b, a = hp[x, y]
            if not ((r > 220 and g > 220 and b > 220) or (r < 140 and g < 140 and b < 160 and abs(r - g) < 30)):
                hp[x, y] = (0, 0, 0, 0)
    im.alpha_composite(hand, (ix + 14, iy - 30))
    # PKMN DATA: its picture and words
    pic = x2(front(mon[2]))
    im.alpha_composite(pic, (16, 32))
    for i, (t, col) in enumerate(name_lines):
        gbafont.draw(im, 12, 170 + i * 30, t, col[0], col[1])
    return im.convert('RGB')


def label(img, text):
    out = Image.new('RGB', (img.width, img.height + 30), (40, 44, 52))
    out.paste(img, (0, 0))
    ImageDraw.Draw(out).text((8, img.height + 9), text, fill=(230, 230, 230))
    return out


def main():
    menu, boxes = Image.open(sys.argv[1]).convert('RGB'), Image.open(sys.argv[2]).convert('RGB')
    data = ((255, 255, 255), (98, 98, 98))
    panels = [
        label(pc_menu(menu, False), '1  PC: VM2 plugged in (GUI-mode): VM2 above LOG OFF'),
        label(pc_menu(menu, True), '2  no VM2: the row greyed, A does nothing'),
        label(message(pc_menu(menu, False), ['Connected to the VM2.', 'It holds a MOéMON EGG.']), '3  VM2 chosen: it is read'),
        label(vm2_box(boxes, ('egg', 1, 'egg'), [('EGG', data), ('VM2', data)]), '4  the VM2 box: an EGG on it'),
        label(vm2_box(boxes, ('venusaur', 1, 'venusaur'), [('VENUSAUR', data), ('Lv50', data)]), '5  a grown MOeMON (Lv50) on it'),
        label(message(menu.copy(), ['The VM2 was disconnected.', 'It’s safe to unplug it.']), '6  CLOSE BOX: written back, VM2 restarted'),
    ]
    cols = 3
    pw, ph = panels[0].width, panels[0].height
    out = Image.new('RGB', (cols * (pw + 12) + 12, 2 * (ph + 12) + 12), (24, 26, 30))
    for i, p in enumerate(panels):
        out.paste(p, (12 + (i % cols) * (pw + 12), 12 + (i // cols) * (ph + 12)))
    out.save(sys.argv[3])


if __name__ == '__main__':
    main()
