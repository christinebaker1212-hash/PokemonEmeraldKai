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


# a 3x5 pixel font for the VMU's few words (as small as the games draw a logo)
TINY = {
    'D': ['110', '101', '101', '101', '110'], 'r': ['000', '011', '100', '100', '100'],
    'e': ['000', '011', '111', '100', '011'], 'a': ['000', '110', '011', '101', '011'],
    'm': ['00000', '11110', '10101', '10101', '10101'], 'c': ['000', '011', '100', '100', '011'],
    's': ['000', '011', '110', '001', '110'], 't': ['010', '111', '010', '010', '001'],
    'A': ['010', '101', '111', '101', '101'], 'B': ['110', '101', '110', '101', '110'],
}


def tiny(d, x, y, text, col):
    for c in text:
        g = TINY[c]
        for yy, row in enumerate(g):
            for xx, v in enumerate(row):
                if v == '1':
                    d.point((x + xx, y + yy), fill=col)
        x += len(g[0]) + 1


def vmu_wallpaper():
    """The VM2 box's wallpaper at GBA size (153x115): a VMU drawn from the user's photo (docs/user_images/97: the
    front, straight on), its proportions kept: a tall light-grey body with rails down both sides and an arched cap
    at the top, "Dreamcast" above a large teal screen, SLEEP / MODE under it on the right, a round D-pad bottom left,
    A lower and B higher on the right. The screen is the box's one place."""
    w, h = 153, 115
    im = Image.new('RGBA', (w, h), (184, 200, 240, 255))
    d = ImageDraw.Draw(im)
    for y in range(0, h, 8):            # a soft diamond lattice, as Emerald's wallpapers
        for x in range((y // 8) % 2 * 8, w, 16):
            d.point((x, y), fill=(168, 184, 232))
            d.point((x + 1, y), fill=(168, 184, 232))
    OUT, BODY, SEAM, SHADE, LIGHT = (112, 110, 104), (226, 224, 214), (178, 176, 168), (202, 200, 190), (240, 238, 232)
    BTN, BTN_D, BTN_L, INK = (128, 128, 150), (70, 70, 84), (166, 166, 186), (88, 88, 92)
    X, Y, W, H = 43, 3, 66, 110          # the body
    # the body: the rails' tops at y 3, the middle's top arching up to y 0, the bottom corners round
    d.rounded_rectangle([X, Y + 3, X + W, Y + H], 6, fill=BODY, outline=OUT)
    px = im.load()
    for x in range(X + 9, X + W - 8):    # the cap's top: an arch above the rails
        t = (x - (X + W / 2)) / (W / 2 - 9)
        top = Y + int(round(3 - 3 * (1 - t * t)))
        for y in range(top, Y + 4):
            px[x, y] = BODY + (255,)
        px[x, top] = OUT + (255,)
    for x in (X + 9, X + W - 9):         # the rails' seams down both sides
        d.line([x, Y + 3, x, Y + H - 5], fill=SEAM)
    for x in range(X + 10, X + W - 9):   # the cap's arched seam
        t = (x - (X + W / 2)) / (W / 2 - 10)
        px[x, Y + int(round(15 - 4 * (1 - t * t)))] = SEAM + (255,)
    d.arc([X + 28, Y + 1, X + 38, Y + 9], 15, 165, fill=SEAM)      # the lid's circle
    d.line([X + W - 1, Y + 8, X + W - 1, Y + H - 6], fill=SHADE)    # a little shade right and below
    d.line([X + 6, Y + H - 1, X + W - 6, Y + H - 1], fill=SHADE)
    d.line([X + 1, Y + 8, X + 1, Y + H - 6], fill=LIGHT)
    tiny(d, X + 16, Y + 19, 'Dreamcast', INK)
    # the screen
    sx, sy, sw, sh = X + 12, Y + 30, 42, 36
    d.rectangle([sx - 1, sy - 1, sx + sw + 1, sy + sh + 1], fill=SEAM)
    d.rectangle([sx, sy, sx + sw, sy + sh], fill=(104, 134, 122))
    d.line([sx, sy, sx + sw, sy], fill=(84, 112, 102))
    d.line([sx, sy, sx, sy + sh], fill=(84, 112, 102))
    # SLEEP / MODE: their words as a line of tiny marks, their buttons under
    for x0 in (X + 32, X + 42):
        for i in range(0, 8, 2):
            d.point((x0 + i, Y + 70), fill=INK)

    def button(cx, cy, r):
        d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=BTN, outline=BTN_D)
        d.point((cx - r // 2, cy - r // 2), fill=BTN_L)
    button(X + 36, Y + 77, 3)
    button(X + 46, Y + 77, 3)
    # the D-pad: a dark round base, a grey cross
    cx, cy = X + 17, Y + 84
    d.ellipse([cx - 10, cy - 10, cx + 10, cy + 10], fill=(96, 96, 108), outline=BTN_D)
    d.ellipse([cx - 9, cy - 9, cx + 9, cy + 9], fill=(118, 118, 134))
    for (x0, y0, x1, y1) in ((cx - 9, cy - 3, cx + 9, cy + 3), (cx - 3, cy - 9, cx + 3, cy + 9)):
        d.rectangle([x0, y0, x1, y1], fill=BTN_L)
    d.line([cx + 1, cy + 4, cx + 3, cy + 4], fill=BTN)   # its arms' lower edges a shade darker
    d.line([cx + 4, cy + 1, cx + 4, cy + 3], fill=BTN)
    d.point((cx, cy), fill=BTN)
    # A lower, B higher, their letters under
    button(X + 41, Y + 88, 5)
    button(X + 55, Y + 85, 5)
    tiny(d, X + 40, Y + 96, 'A', INK)
    tiny(d, X + 54, Y + 93, 'B', INK)
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
    im.alpha_composite(hand, (ix + 22, iy - 14))
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
