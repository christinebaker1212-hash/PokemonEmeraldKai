#!/usr/bin/env python3
"""OPTIONS > DISPLAY > THEME: DARK's palettes, designed (not a flip of the light ones).

Every window in the game takes its colors from a few palettes; DARK swaps each for one made here:
  - the menu text palette (std_menu.pal) and the four text window palettes: a night-slate fill, warm white text with a
    shadow darker than the fill (light text on dark reads with its shadow below it, as the battle box's white text
    does), and each colored text (red, green, blue) its own bright tone over a deep shadow of the same hue;
  - every window frame (Emerald's 20, FireRed's 9) and the field's message box: each color sorted by what it is in
    the frame - the dark outline stays, the frame's own colors keep their hue and go to a dusk tone, the light rims
    that met the white fill become the slate of the dark fill a step or two lighter, so the frame still reads as
    itself around a dark window. A frame can be tuned by hand below (OVERRIDES), color by color;
  - battles: the action / move windows' text palette, and the white boxes of the battle text box.
Out: src/data/mp_dark_palettes.h (mp_darktheme.c), and with --preview DIR a sheet of every frame, light and dark.

usage: python3 tools/moemon_darktheme/gen_dark.py [--preview DIR]
"""
import colorsys
import os
import sys
from PIL import Image, ImageDraw

BAG_STRIP_Y = 0   # (from mp_bag_gfx.h)
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# The dark theme's own colors (8-bit; the game keeps 5 bits a channel)
FILL = (40, 44, 58)          # the windows' fill: a night slate
RIM = [(64, 70, 88), (88, 96, 116), (120, 128, 148)]   # lighter slates for the rims that met the fill
TEXT = (240, 240, 232)       # warm white
SHADOW = (16, 18, 26)        # darker than the fill
TEXT_COLORS = {              # (text, shadow) for the menus' colored text
    'red': ((255, 120, 104), (88, 24, 24)),
    'green': ((128, 224, 120), (24, 72, 32)),
    'blue': ((136, 184, 255), (24, 48, 104)),
}

# Hand tuning: frame number (1-29) -> {palette index: (r, g, b)}
OVERRIDES = {}


def rgb555(c):
    return (c[0] >> 3) | ((c[1] >> 3) << 5) | ((c[2] >> 3) << 10)


def from555(v):
    return ((v & 31) << 3 | (v & 31) >> 2, ((v >> 5) & 31) << 3 | ((v >> 5) & 31) >> 2, ((v >> 10) & 31) << 3 | ((v >> 10) & 31) >> 2)


def jasc(path):
    lines = open(path).read().split('\n')[3:]
    return [tuple(int(v) for v in ln.split()[:3]) for ln in lines if ln.strip()][:16]


def png_pal(path):
    raw = Image.open(path).getpalette()[:48]
    pal = [tuple(raw[i:i + 3]) for i in range(0, len(raw), 3)]
    return (pal + [(0, 0, 0)] * 16)[:16]


def lum(c):
    return (0.299 * c[0] + 0.587 * c[1] + 0.114 * c[2]) / 255


def sat(c):
    return colorsys.rgb_to_hsv(*(v / 255 for v in c))[1]


# ---------------------------------------------------------------- text palettes

def dark_text(pal):
    """std_menu.pal / text_pal*: 1 fill, 2 text, 3 shadow, 4-9 red / green / blue text and their shadows"""
    out = list(pal)
    out[1], out[2], out[3] = FILL, TEXT, SHADOW
    for i, name in ((4, 'red'), (6, 'green'), (8, 'blue')):
        out[i], out[i + 1] = TEXT_COLORS[name]
    # the rest (a screen's own extra colors): lightness turned over, hue kept
    for i in range(10, 16):
        out[i] = turn(pal[i])
    return out


def turn(c):
    h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
    if l > 0.7:
        return RIM[0] if s < 0.25 else hls(h, 0.32, s * 0.8)
    if l < 0.45:
        return hls(h, min(0.92, 1.05 - l), s * 0.8)
    return c


def hls(h, l, s):
    return tuple(int(round(v * 255)) for v in colorsys.hls_to_rgb(h, l, s))


# ---------------------------------------------------------------- frames

def frame_roles(tiles, pal):
    """Which palette index is what in a frame: by where it's drawn. The frame's 9 tiles (3x3 of 8x8): the outer ring of
    pixels is the outline, the pixels next to the inside (the white fill) are the rim, the rest is the frame's body"""
    img = [[0] * 24 for _ in range(24)]
    for t in range(9):
        tx, ty = (t % 3) * 8, (t // 3) * 8
        for y in range(8):
            for x in range(4):
                v = tiles[t * 32 + y * 4 + x]
                img[ty + y][tx + x * 2] = v & 15
                img[ty + y][tx + x * 2 + 1] = v >> 4
    roles = {}
    for y in range(24):
        for x in range(24):
            if 8 <= x < 16 and 8 <= y < 16:
                continue
            i = img[y][x]
            if i == 0:
                continue
            # distance to the inside square (8..15) and to the outside
            dx = 0 if 8 <= x < 16 else (8 - x if x < 8 else x - 15)
            dy = 0 if 8 <= y < 16 else (8 - y if y < 8 else y - 15)
            d_in = max(dx, dy)
            roles.setdefault(i, []).append(d_in)
    return img, roles


def dark_frame(pal, tiles, num):
    _, roles = frame_roles(tiles, pal)
    out = list(pal)
    used = sorted(roles, key=lambda i: sum(roles[i]) / len(roles[i]))   # nearest the inside first
    darkest = min(used, key=lambda i: lum(pal[i])) if used else None
    lights = [i for i in used if lum(pal[i]) > 0.62 and sat(pal[i]) < 0.35]
    for i in used:
        c = pal[i]
        if i in lights:
            # the light rims: slates, lighter the further from the fill (as the white ones were); a tinted light
            # (a pastel frame's pink, mint...) keeps its tint at a dusk lightness
            k = lights.index(i)
            h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
            if s > 0.18:
                out[i] = hls(h, 0.30 + 0.07 * min(k, 2), min(0.55, s * 0.9))
            else:
                out[i] = RIM[min(k, len(RIM) - 1)]
        elif i == darkest and lum(c) < 0.3:
            out[i] = tuple(max(0, int(v * 0.8)) for v in c)   # the outline: kept, a touch deeper
        else:
            # the frame's own colors: hue kept, a dusk tone (less light, a little less saturated); a grey goes to the
            # slates' own blue-grey so it never warms to khaki
            h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
            if sat(c) < 0.15:
                out[i] = hls(0.63, max(0.16, min(0.5, l * 0.62)), 0.14)
            else:
                out[i] = hls(h, max(0.16, min(0.5, l * 0.62)), min(1.0, s * 0.85))
    for i, c in OVERRIDES.get(num, {}).items():
        out[i] = c
    return out


def frames():
    """[(number, tiles bytes, palette)] for all 29: Emerald's 1-20 then FireRed's 9"""
    out = []
    for n in range(1, 21):
        im = Image.open(os.path.join(ROOT, 'graphics/text_window/%d.png' % n))
        pal = png_pal(os.path.join(ROOT, 'graphics/text_window/%d.png' % n))
        px = im.load()
        tiles = bytearray()
        for t in range(9):
            tx, ty = (t % 3) * 8, (t // 3) * 8
            for y in range(8):
                for x in range(0, 8, 2):
                    tiles.append(px[tx + x, ty + y] & 15 | ((px[tx + x + 1, ty + y] & 15) << 4))
        out.append((n, bytes(tiles), pal))
    tb = open(os.path.join(ROOT, 'graphics/moemon/ext/frame_tiles.bin'), 'rb').read()
    pb = open(os.path.join(ROOT, 'graphics/moemon/ext/frame_pals.bin'), 'rb').read()
    for k in range(9):
        pal = [from555(int.from_bytes(pb[k * 32 + i * 2:k * 32 + i * 2 + 2], 'little')) for i in range(16)]
        out.append((21 + k, tb[k * 0x120:(k + 1) * 0x120], pal))
    return out


# ---------------------------------------------------------------- the message box, battles

def dark_message_box(pal):
    """message_box.png's palette: it shares its slot with the text (1-3), the rest is the box's border"""
    out = dark_text(pal)
    # (the user, 2026-11-13: "the dialogue edges are too bright"): the border's light parts the first slate, its
    # colors a deep dusk, so the box sits quietly around its words
    for i in range(10, 16):
        c = pal[i]
        if lum(c) > 0.62 and sat(c) < 0.35:
            out[i] = RIM[0]
        else:
            h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
            out[i] = hls(h, max(0.14, l * 0.4), s * 0.6)
    return out


def dark_battle_text(pal):
    """battle_interface/text.pal (the action / move windows): 14 the white fill, 13 / 12 the text, 15 the shadow"""
    out = list(pal)
    for i in range(1, 16):
        c = pal[i]
        if c == (0, 0, 0):
            continue
        if lum(c) > 0.9:
            out[i] = FILL
        elif lum(c) > 0.7:
            out[i] = SHADOW
        elif lum(c) < 0.35 and sat(c) < 0.2:
            out[i] = TEXT
        else:
            out[i] = turn(c)
    return out


def dark_battle_box(pal32):
    """battle_interface/textbox (two palettes): the white boxes (the action menu's) and their light rims go slate,
    the message box's teal deepens; the message's white text (0:1), the red frame and the rest stay"""
    out = list(pal32)
    for i, c in enumerate(pal32):
        if i % 16 == 0 or i == 1:
            continue
        h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
        if i == 15:
            out[i] = hls(h, l * 0.42, s * 0.9)
        elif l > 0.76 and s < 0.4:
            out[i] = FILL if l > 0.95 else RIM[0]
    return out


def pp_colors(pal):
    """text_pp.pal: (number, shadow) pairs for full / some / little / no PP"""
    out = list(pal)
    for k in range(0, 8, 2):
        t, sh = pal[k], pal[k + 1]
        if lum(t) < 0.35:      # the plain one: dark grey on white
            out[k], out[k + 1] = TEXT, SHADOW
        else:                  # yellow / orange / red: a little brighter, a deep shadow of their hue
            h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in t))
            out[k] = hls(h, min(0.72, l * 1.08), s)
            out[k + 1] = hls(h, 0.18, s * 0.8)
    return out


# ---------------------------------------------------------------- the BAG (mp_bagui.c's full-color picture)

def bag_colors():
    import re
    global BAG_STRIP_Y
    m = re.search(r'#define BAGUI_STRIP_Y (\d+)', open(os.path.join(ROOT, 'src/mp/mp_bag_gfx.h')).read())
    BAG_STRIP_Y = int(m.group(1))
    src = open(os.path.join(ROOT, 'src/mp/mp_bag_gfx.h')).read()
    body = src.split('sBagUI_Colors[')[1].split('{', 1)[1].split('}')[0]
    return [from555(int(v, 16)) for v in re.findall(r'0x[0-9A-Fa-f]{4}', body)]


def bag_bg():
    src = open(os.path.join(ROOT, 'src/mp/mp_bag_gfx.h')).read()
    body = src.split('sBagUI_Bg[160][240] = {')[1]
    rows = []
    for ln in body.split('\n'):
        ln = ln.strip()
        if ln.startswith('{'):
            rows.append([int(v) for v in ln.strip('{},').split(',')])
        if len(rows) == 160:
            break
    return rows


def dark_bag(cols):
    """The BAG's picture: its white list and plates the windows' slate, its light rims the rims' slates, its colors
    (the blue panel, the green field, the purple description box) at a dusk tone; the red cursor frame stays"""
    out = []
    for c in cols:
        h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
        if c == (255, 0, 0) or (s > 0.8 and l < 0.6 and abs(h) < 0.03):
            out.append(c)                                   # the cursor's red
        elif l > 0.93 and s < 0.3:
            out.append(FILL)
        elif l > 0.72 and s < 0.3:
            out.append(RIM[0] if l < 0.84 else RIM[1])
        elif s < 0.15:
            out.append(hls(0.63, max(0.12, l * 0.55), 0.14))
        elif l > 0.68:
            out.append(hls(h, l * 0.42, s * 0.55))          # a pastel (the sky-blue panel): deep and quiet
        else:
            out.append(hls(h, max(0.14, min(0.42, l * 0.6)), min(1.0, s * 0.8)))
    return out


def dark_bag_strip(cols):
    """The pocket icons' row: the white cells a lighter slate, the dark icons light (so they read on the dark)"""
    out = dark_bag(cols)
    for i, c in enumerate(cols):
        h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
        if l > 0.93 and s < 0.3:
            out[i] = RIM[0]
        elif l < 0.58:
            out[i] = hls(h, 0.82, min(0.6, s))
    return out


# ---------------------------------------------------------------- output

def c_array(name, pals):
    rows = ['static const u16 %s[%d][16] = {' % (name, len(pals))]
    for p in pals:
        rows.append('    {' + ', '.join('0x%04X' % rgb555(c) for c in p) + '},')
    rows.append('};')
    return rows


def main():
    fr = frames()
    std = jasc(os.path.join(ROOT, 'graphics/interface/std_menu.pal'))
    textpals = [jasc(os.path.join(ROOT, 'graphics/text_window/text_pal%d.pal' % k)) for k in range(1, 5)]
    mbox = png_pal(os.path.join(ROOT, 'graphics/text_window/message_box.png'))
    btext = jasc(os.path.join(ROOT, 'graphics/battle_interface/text.pal'))
    bbox = jasc(os.path.join(ROOT, 'graphics/battle_interface/textbox_0.pal')) + jasc(os.path.join(ROOT, 'graphics/battle_interface/textbox_1.pal'))
    pp = jasc(os.path.join(ROOT, 'graphics/battle_interface/text_pp.pal'))
    opt = jasc(os.path.join(ROOT, 'graphics/interface/option_menu_text.pal'))

    dframes = [dark_frame(p, t, n) for n, t, p in fr]
    out = ['// Generated by tools/moemon_darktheme/gen_dark.py - do not edit',
           '// OPTIONS > DISPLAY > THEME: DARK\'s palettes (mp_darktheme.c), each designed for its place, not a flip.', '']
    out += c_array('sDarkFrames', dframes) + ['']
    out += c_array('sDarkStdMenu', [dark_text(std)]) + ['']
    out += c_array('sDarkTextPals', [dark_text(p) for p in textpals]) + ['']
    out += c_array('sDarkMessageBox', [dark_message_box(mbox)]) + ['']
    out += c_array('sDarkBattleText', [dark_battle_text(btext)]) + ['']
    db = dark_battle_box(bbox)
    out += c_array('sDarkBattleBox', [db[:16], db[16:32]]) + ['']
    out += c_array('sDarkPP', [pp_colors(pp)]) + ['']
    out += c_array('sDarkOptionText', [dark_text(opt)]) + ['']
    bc = bag_colors()
    dbc = dark_bag(bc)
    out.append('static const u16 sDarkBagColors[%d] = {' % len(dbc))
    out.append('    ' + ', '.join('0x%04X' % rgb555(c) for c in dbc) + ',')
    out.append('};')
    out.append('static const u16 sDarkBagStripColors[%d] = {' % len(dbc))
    out.append('    ' + ', '.join('0x%04X' % rgb555(c) for c in dark_bag_strip(bc)) + ',')
    out.append('};')
    out.append('// the BAG\'s text (item_menu.c sBagTextColors): the list\'s text, white, the list\'s shadow')
    out.append('static const u16 sDarkBagText[3] = {0x%04X, 0x7FFF, 0x%04X};' % (rgb555(TEXT), rgb555(SHADOW)))
    out.append('')
    open(os.path.join(ROOT, 'src/data/mp_dark_palettes.h'), 'w').write('\n'.join(out) + '\n')
    print('wrote src/data/mp_dark_palettes.h')

    if '--preview' in sys.argv:
        d = sys.argv[sys.argv.index('--preview') + 1]
        preview(fr, dframes, dark_text(std), std, os.path.join(d, 'dark_frames.png'))
        bg = bag_bg()
        img = Image.new('RGB', (480, 160))
        dst = dark_bag_strip(bc)
        for half, pal in ((0, bc), (240, dbc)):
            for y in range(160):
                for x in range(240):
                    k = bg[y][x]
                    c = pal[k]
                    if half and BAG_STRIP_Y - 3 <= y < BAG_STRIP_Y + 13 and x < 104:
                        c = dst[k]
                    elif half and y < 18 and x < 58 and bc[k] == (255, 255, 255):
                        c = bc[k]
                    img.putpixel((half + x, y), c)
        img.resize((960, 320), Image.NEAREST).save(os.path.join(d, 'dark_bag.png'))


def draw_window(img, x0, y0, tiles, pal, textpal, label):
    """A window of 10 x 3 tiles inside the frame, its fill and a line of text in the menu font's colors"""
    W, H = 10, 3
    px = img.load()
    def tile(t, ox, oy):
        for y in range(8):
            for x in range(4):
                v = tiles[t * 32 + y * 4 + x]
                for k, i in enumerate((v & 15, v >> 4)):
                    if i:
                        px[ox + x * 2 + k, oy + y] = pal[i]
    for ty in range(H + 2):
        for tx in range(W + 2):
            ox, oy = x0 + tx * 8, y0 + ty * 8
            r = 0 if ty == 0 else 2 if ty == H + 1 else 1
            c = 0 if tx == 0 else 2 if tx == W + 1 else 1
            t = r * 3 + c
            if t == 4:
                for y in range(8):
                    for x in range(8):
                        px[ox + x, oy + y] = textpal[1]
            else:
                tile(t, ox, oy)
    dr = ImageDraw.Draw(img)
    dr.text((x0 + 12 + 1, y0 + 12 + 1), label, fill=textpal[3])
    dr.text((x0 + 12, y0 + 12), label, fill=textpal[2])


def preview(fr, dframes, dtext, std, path):
    cols = 6
    cw, ch = 12 * 8 + 6, 5 * 8 + 6
    rows = (len(fr) + cols - 1) // cols
    img = Image.new('RGB', (cols * cw * 2 + 20, rows * ch + 4), (112, 168, 136))
    for k, ((n, t, p), dp) in enumerate(zip(fr, dframes)):
        x, y = (k % cols) * cw * 2 + (10 if k % cols >= cols // 2 else 0), (k // cols) * ch + 2
        draw_window(img, x, y, t, p, std, 'FRAME %d' % n)
        draw_window(img, x + cw, y, t, dp, dtext, 'FRAME %d' % n)
    img = img.resize((img.width * 3, img.height * 3), Image.NEAREST)
    img.save(path)
    print('wrote', path)


if __name__ == '__main__':
    main()
