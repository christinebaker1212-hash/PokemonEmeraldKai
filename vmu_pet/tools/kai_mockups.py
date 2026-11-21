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


def pc_menu(shot, greyed=False):
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
    """The VM2 box's wallpaper at GBA size (153x115): a VMU as the Dreamcast's own (light grey, a curved bottom edge,
    the screen in the middle at the top, a lavender D-pad on the left, SLEEP and MODE small in the middle, A and B on
    the right, B lower), drawn as pixel art; its screen is the box's one place."""
    w, h = 153, 115
    im = Image.new('RGBA', (w, h), (184, 200, 240, 255))
    d = ImageDraw.Draw(im)
    for y in range(0, h, 8):            # a soft diamond lattice, as Emerald's wallpapers
        for x in range((y // 8) % 2 * 8, w, 16):
            d.point((x, y), fill=(168, 184, 232))
            d.point((x + 1, y), fill=(168, 184, 232))
    OUT, BODY, LIGHT, SHADE = (64, 64, 80), (220, 220, 224), (244, 244, 248), (184, 184, 196)
    BTN, BTN_D, BTN_L = (140, 148, 192), (76, 80, 120), (176, 184, 220)
    bx, bw, top, side_end, bottom = 36, 81, 3, 80, 112
    cx = bx + bw / 2

    def inside(x, y):
        if y < top or y > bottom or x < bx or x > bx + bw:
            return False
        r = 7                            # the top corners
        for ccx in (bx + r, bx + bw - r):
            if y < top + r and abs(x - ccx) > 0 and ((x < bx + r and ccx == bx + r) or (x > bx + bw - r and ccx == bx + bw - r)):
                if (x - ccx) ** 2 + (y - (top + r)) ** 2 > r * r:
                    return False
        if y > side_end:                 # the curved bottom: an ellipse's lower half
            ry = bottom - side_end
            rx = bw / 2
            if ((x - cx) / rx) ** 2 + ((y - side_end) / ry) ** 2 > 1:
                return False
        return True
    px = im.load()
    for y in range(h):
        for x in range(w):
            if inside(x + 0.5, y + 0.5):
                px[x, y] = BODY + (255,)
    for y in range(h):                   # outline, a light edge top-left, a shade bottom-right
        for x in range(w):
            if px[x, y][:3] != BODY:
                continue
            n = [inside(x + 0.5 + dx, y + 0.5 + dy) for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1))]
            if not all(n):
                px[x, y] = OUT + (255,)
            elif not inside(x - 1.5, y + 0.5) or not inside(x + 0.5, y - 1.5):
                px[x, y] = LIGHT + (255,)
            elif not inside(x + 2.5, y + 0.5) or not inside(x + 0.5, y + 2.5):
                px[x, y] = SHADE + (255,)
    # the top edge's strip (where the Dreamcast word sits)
    d.line([bx + 12, top + 6, bx + bw - 12, top + 6], fill=SHADE)
    # the screen: a grey frame and the LCD
    sx, sy, sw, sh = int(cx) - 26, top + 13, 51, 34
    d.rounded_rectangle([sx - 3, sy - 3, sx + sw + 3, sy + sh + 3], 3, fill=(156, 160, 172), outline=(120, 124, 140))
    d.rectangle([sx, sy, sx + sw, sy + sh], fill=(170, 186, 160))
    d.line([sx, sy, sx + sw, sy], fill=(140, 156, 132))
    d.line([sx, sy, sx, sy + sh], fill=(140, 156, 132))

    def button(x, y, r):
        d.ellipse([x - r, y - r, x + r, y + r], fill=BTN, outline=BTN_D)
        d.point((x - r // 2, y - r // 2), fill=BTN_L)
        d.point((x - r // 2 + 1, y - r // 2), fill=BTN_L)
    # the D-pad (left), SLEEP / MODE (middle), A / B (right, B lower)
    dx, dy = bx + 17, top + 67
    for (x0, y0, x1, y1) in ((dx - 9, dy - 3, dx + 9, dy + 3), (dx - 3, dy - 9, dx + 3, dy + 9)):
        d.rectangle([x0, y0, x1, y1], fill=BTN, outline=BTN_D)
    d.rectangle([dx - 2, dy - 2, dx + 2, dy + 2], fill=BTN)
    d.point((dx, dy), fill=BTN_D)
    button(int(cx) - 2, top + 61, 3)
    button(int(cx) + 8, top + 64, 3)
    button(bx + 53, top + 75, 6)
    button(bx + 66, top + 81, 6)
    return im, (sx + sw // 2 - 16, sy + sh // 2 - 17)


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
        label(menu.copy(), '2  no VM2 connected: no VM2 row at all'),
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
