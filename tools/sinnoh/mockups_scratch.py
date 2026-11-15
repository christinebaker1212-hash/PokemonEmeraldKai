#!/usr/bin/env python3
"""Mock-ups for the user (2026-11-15) of the BATTLE FRONTIER's Scratch-Off Cards (Platinum's
src/applications/scratch_off_cards) on one 240 x 160 screen, from Platinum's own art in the user's ROM
(resource/eng/scratch/scratch.narc) and its own words (res/text/scratch_off_cards*.json), with the game's font and
window frame (tools/unova/mockups.py). Platinum uses both DS screens and the stylus; here:

1. choose: the top screen's picture (members 35 / 37, palette 34) behind the four cards to pick (sprites 12 / 11 / 13),
   LEFT / RIGHT and A, "Please select card no. 1." in a message box;
2. scratch: the touch screen's card (4 / 6) and its nine grey cells (7 / 9, palette 0) over the POKéMON icons
   (sprites 32 / 31 / 33); the D-Pad moves a coin from cell to cell, A held scratches the film off where it stands
   (Platinum's stylus: the scratched pixels, 240 x 160 as its own region); three cells to scratch, NEXT CARD / CANCEL;
3. win: three of a kind, the matching cells boxed (sprites 16 / 15 / 17) and Platinum's winning pop-up (20 / 19 / 21);
4. the prize card (Platinum's top screen while scratching, 38 / 40): each POKéMON x 3 and the BERRY it wins, on R.

    python3 -I tools/sinnoh/mockups_scratch.py ROM OUT_DIR      (scratch_choose.png, scratch_play.png,
                                                                  scratch_win.png, scratch_prizes.png at 2x, sheet.png)
"""
import json
import os
import struct
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, 'tools', 'unova'))
import nds  # noqa: E402
import build_library_tv as L  # noqa: E402
import mockups as mk  # noqa: E402

W, H = mk.W, mk.H
OAM_SIZES = {(0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64), (1, 0): (16, 8), (1, 1): (32, 8),
             (1, 2): (32, 16), (1, 3): (64, 32), (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64)}


def rgb(c):
    return ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 255)


def bg(n, ch, sc, pal):
    lay = L.layer(L.unpack(n[ch]), L.unpack(n[sc]), pal)
    im = Image.new('RGBA', (256, 192), (0, 0, 0, 0))
    px = im.load()
    for y in range(192):
        for x in range(256):
            v = lay[y * 256 + x]
            if v & 0x8000:
                px[x, y] = rgb(v)
    return im


def cells(ncgr, ncer, pal):
    """every cell of a sprite bank, each an RGBA picture with its (x0, y0) from the sprite's point"""
    ts, depth = L.tiles(ncgr)
    c = L.section(ncer, b'CEBK')
    n, attrs, off, mapping = struct.unpack_from('<HHII', c, 8)
    shift = mapping & 3
    base = 8 + off
    step = 16 if attrs & 1 else 8
    out = []
    for i in range(n):
        cnt, _, ooff = struct.unpack_from('<HHI', c, base + i * step)
        oams = []
        for k in range(cnt):
            a0, a1, a2 = struct.unpack_from('<HHH', c, base + n * step + ooff + 6 * k)
            y = (a0 & 0xFF) - (256 if a0 & 0x80 else 0)
            x = (a1 & 0x1FF) - (512 if a1 & 0x100 else 0)
            oams.append((x, y, OAM_SIZES[(a0 >> 14, a1 >> 14)], (a2 & 0x3FF) << shift, a2 >> 12,
                         bool(a1 & 0x1000), bool(a1 & 0x2000)))
        if not oams:
            out.append((Image.new('RGBA', (1, 1)), 0, 0))
            continue
        x0, y0 = min(o[0] for o in oams), min(o[1] for o in oams)
        x1, y1 = max(o[0] + o[2][0] for o in oams), max(o[1] + o[2][1] for o in oams)
        im = Image.new('RGBA', (x1 - x0, y1 - y0), (0, 0, 0, 0))
        px = im.load()
        for x, y, (ow, oh), t0, pn, hf, vf in reversed(oams):
            for ty in range(oh // 8):
                for tx in range(ow // 8):
                    t = t0 + ty * (ow // 8) + tx
                    if t >= len(ts):
                        continue
                    for i2, v in enumerate(ts[t]):
                        if not v:
                            continue
                        X, Y = tx * 8 + i2 % 8, ty * 8 + i2 // 8
                        if hf:
                            X = ow - 1 - X
                        if vf:
                            Y = oh - 1 - Y
                        k = pn * 16 + v
                        px[x - x0 + X, y - y0 + Y] = rgb(pal[k] if k < len(pal) else 0)
        out.append((im, x0, y0))
    return out


def put(im, cell, x, y):
    pic, x0, y0 = cell
    im.alpha_composite(pic, (x + x0, y + y0))


def words(name):
    root = open(os.path.join(REPO, 'sinnoh', 'decomp_dir')).read().strip()
    d = json.load(open(os.path.join(root, 'res', 'text', name + '.json')))
    return {m['id']: m.get('en_US') for m in d['messages']}


def line(s):
    s = ''.join(s) if isinstance(s, list) else s
    return s.replace('\r', '').replace('{STRVAR_1 50, 0, 0}', '1').replace('{YESNO 0}', '').replace('Pokémon', 'POKéMON')


def message(im, s):
    mk.window(im, 0, H - 48, 28, 4)
    for k, ln in enumerate([x for x in line(s).split('\n') if x][:2]):
        mk.text(im, 16, H - 40 + k * 16, ln)


def coin(im, x, y):
    """the coin the D-Pad moves (the stylus's place): a gold coin in the game's style"""
    d = ImageDraw.Draw(im)
    d.ellipse((x - 7, y - 7, x + 7, y + 7), fill=(248, 200, 64, 255), outline=(136, 88, 24, 255))
    d.ellipse((x - 4, y - 4, x + 4, y + 4), outline=(200, 136, 32, 255))
    d.point([(x - 3, y - 5), (x - 4, y - 4), (x - 5, y - 3)], fill=(255, 248, 200, 255))


def main():
    rom, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    n = nds.Rom(rom).narc('resource/eng/scratch/scratch.narc')
    U = lambda i: L.unpack(n[i])  # noqa: E731
    top_pal, bot_pal = L.palette(U(34)), L.palette(U(0))
    card_cells = cells(U(12), U(11), L.palette(U(13)))
    box_cells = cells(U(16), U(15), L.palette(U(17)))
    win_cells = cells(U(20), U(19), L.palette(U(21)))
    mon_cells = cells(U(32), U(31), L.palette(U(33)))
    app, shop = words('scratch_off_cards_app'), words('scratch_off_cards')
    # (the DS's 256 x 192 cropped to the screen's middle, never scaled)
    crop = (8, 16, 8 + W, 16 + H)

    # the nine cells (sScratchCellScreenPixelRegions, the DS's touch screen): their middles
    regions = [(32, 89, 21, 54), (102, 157, 21, 54), (170, 227, 21, 54), (32, 89, 68, 100), (102, 157, 68, 100),
               (170, 227, 68, 100), (32, 89, 114, 148), (102, 157, 114, 148), (170, 227, 114, 148)]
    mid = [((l + r) // 2, (t + b) // 2) for l, r, t, b in regions]

    def centered(im, cell, x, y):
        pic = cell[0]
        im.alpha_composite(pic, (x - pic.size[0] // 2, y - pic.size[1] // 2))

    # 1. choosing a card: the four cards fanned as Platinum's (sSelectableCardPositions x 0, 56, 112, 168), the
    # chosen one raised, the cursor over it
    top = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
    top.alpha_composite(bg(n, 35, 37, top_pal))
    for i in range(4):
        put(top, card_cells[i % len(card_cells)], i * 56, 22 - (10 if i == 1 else 0))
    choose = top.crop(crop).convert('RGB')
    mk.cursor(choose, 1 * 56 + 44 - 8, 2)
    message(choose, app['ScratchOffCardsApp_Text_SelectCard'])

    # 2. scratching
    def card_screen(revealed, scratched):
        sc = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
        sc.alpha_composite(bg(n, 4, 6, bot_pal))
        for i, (x, y) in enumerate(mid):   # (the icons under the film, seen only through their cell)
            layer = Image.new('RGBA', (256, 192), (0, 0, 0, 0))
            centered(layer, mon_cells[revealed[i] % len(mon_cells)], x, y)
            l, r, t, b = regions[i]
            sc.alpha_composite(layer.crop((l, t, r + 1, b + 1)), (l, t))
        film = bg(n, 7, 9, bot_pal)
        fp = film.load()
        for i, part in scratched:   # (the film taken off: Platinum's scratched pixels, a cell or part of one)
            l, r, t, b = regions[i]
            for yy in range(t, b + 1):
                for xx in range(l, r + 1):
                    if part >= 1 or ((xx - mid[i][0]) ** 2 + (yy - mid[i][1]) ** 2) ** 0.5 < part * 30 \
                            and (xx * 7 + yy * 3) % 11 > 1:
                        fp[xx, yy] = (0, 0, 0, 0)
        sc.alpha_composite(film)
        return sc
    icons = [0, 1, 0, 2, 0, 3, 1, 2, 3]
    play = card_screen(icons, [(0, 1), (4, 0.55)])
    coin(play, mid[4][0] + 10, mid[4][1] + 6)
    play = play.crop(crop).convert('RGB')
    mk.window(play, 140, 0, 10, 2)
    mk.text(play, 156, 8, line(app['ScratchOffCardsApp_Text_NextCard']))

    # 3. three of a kind: the matching cells boxed, Platinum's winning pop-up
    winners = (0, 4, 8)
    win = card_screen([0, 1, 2, 3, 0, 2, 1, 3, 0], [(i, 1) for i in winners])
    for i in winners:
        centered(win, box_cells[0], *mid[i])
    centered(win, win_cells[0], 128, 84)
    for k, x in enumerate((67, 126, 184)):   # (the three on the pop-up's ovals: sWinningPopupMonSpritePositions)
        centered(win, mon_cells[0], x, 80)
    win = win.crop(crop).convert('RGB')
    message(win, 'Received 1 POMEG BERRY!')

    # 4. the prize card (Platinum's top screen while scratching): each POKéMON's icon x 3 and its BERRY; R shows it
    prize = Image.new('RGBA', (256, 192), (0, 0, 0, 255))
    prize.alpha_composite(bg(n, 38, 40, top_pal))
    for row in range(4):
        centered(prize, mon_cells[row % len(mon_cells)], 98, 46 + row * 42)
    prize = prize.crop(crop).convert('RGB')
    for row, berry in enumerate(('POMEG BERRY', 'KELPSY BERRY', 'QUALOT BERRY', 'HONDEW BERRY')):
        mk.text(prize, 172, 22 + row * 42, berry, font=mk.s2.SMALL if hasattr(mk.s2, 'SMALL') else mk.s2.NORMAL)

    shots = [('scratch_choose', choose), ('scratch_play', play), ('scratch_win', win), ('scratch_prizes', prize)]
    for name, im in shots:
        im.resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(out, name + '.png'))
    sheet = Image.new('RGB', (2 * (W * 2 + 8) + 8, 2 * (H * 2 + 8) + 8), (32, 32, 40))
    for i, (name, im) in enumerate(shots):
        sheet.paste(im.resize((W * 2, H * 2), Image.NEAREST), (8 + (i % 2) * (W * 2 + 8), 8 + (i // 2) * (H * 2 + 8)))
    sheet.save(os.path.join(out, 'sheet.png'))
    print(out)


if __name__ == '__main__':
    main()
