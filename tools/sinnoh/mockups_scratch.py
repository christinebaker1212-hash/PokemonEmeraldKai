#!/usr/bin/env python3
"""Mock-ups for the user (2026-11-15; cleaned the same day) of the BATTLE FRONTIER's Scratch-Off Cards (Platinum's
src/applications/scratch_off_cards) on one 240 x 160 screen, from Platinum's own art in the user's ROM
(tools/sinnoh/build_scratch.py: the very pictures the game draws, src/sinnoh/s3d_scratch.c) and its own words
(res/text/scratch_off_cards_app.json), with the game's font and window frame (tools/unova/mockups.py). Nothing is
scaled; each of Platinum's two screens is shown where its content lies whole:

1. choose: the top screen's picture (DS rows 30-190) behind the four cards, laid 50 apart so all four show whole (the
   DS's 56 runs past 240), the left one in front as Platinum's; LEFT / RIGHT raise a card, A takes it;
2. start: the card on the touch screen (DS 8-168: the nine cells and the card's foot), "Start card no. 1!" in
   Platinum's centre box;
3. scratch: the D-Pad moves the coin from cell to cell, A held scratches the film off where it stands (Platinum's
   stylus); two alike flash in Platinum's boxes; R / B in the card's foot;
4. win: three alike: Platinum's pop-up with the three;
5. the prize card (Platinum's top screen while scratching, DS rows 27-187: all four rows) while R is held;
6. B: "Stop scratching this card?" with YES / NO.

    python3 -I tools/sinnoh/mockups_scratch.py ROM OUT_DIR    (scratch_*.png at 2x and sheet.png)
"""
import os
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(REPO, 'tools', 'unova'))
import build_scratch as B  # noqa: E402
import mockups as mk  # noqa: E402

W, H = mk.W, mk.H
CARD_X = [0, 50, 101, 152]                        # (the four cards, whole)
CARD_Y = 6
CHOOSE_DY, CARD_D, PRIZE_DY = -30, (-8, -8), -27  # (each DS screen's rows shown)
MON = [(41, 18), (108, 18), (175, 18), (41, 66), (108, 66), (175, 66), (41, 114), (108, 114), (175, 114)]
CELLS = [(32, 89, 21, 54), (102, 157, 21, 54), (170, 227, 21, 54), (32, 89, 68, 100), (102, 157, 68, 100),
         (170, 227, 68, 100), (32, 89, 114, 148), (102, 157, 114, 148), (170, 227, 114, 148)]


def rgba(p):
    im = Image.new('RGBA', (p.w, p.h), (0, 0, 0, 0))
    px = im.load()
    for i, c in enumerate(p.px):
        if c & 0x8000:
            px[i % p.w, i // p.w] = ((c & 31) << 3, (c >> 5 & 31) << 3, (c >> 10 & 31) << 3, 255)
    return im


def put(im, pic, x, y):
    layer = Image.new('RGBA', im.size, (0, 0, 0, 0))
    layer.paste(pic, (x, y), pic)
    im.alpha_composite(layer)


def ds(x, y, d=CARD_D):
    return x + d[0], y + d[1]


def coin(im, x, y):
    """the coin the D-Pad moves (the stylus's place): a gold coin in the game's style"""
    d = ImageDraw.Draw(im)
    d.ellipse((x - 7, y - 7, x + 7, y + 7), fill=(248, 200, 64, 255), outline=(136, 88, 24, 255))
    d.ellipse((x - 4, y - 4, x + 4, y + 4), outline=(200, 136, 32, 255))
    d.point([(x - 3, y - 5), (x - 4, y - 4), (x - 5, y - 3)], fill=(255, 248, 200, 255))


def hint(im, x, y, button, label, align='l'):
    """a controller button in the card's foot, Platinum's word beside it (the game's G_Hint)"""
    d = ImageDraw.Draw(im)
    w = (mk.text(Image.new('RGB', (1, 1)), 0, 0, label, font=mk.s2.SMALL) + 14) if label else 11
    if align == 'r':
        x -= w
    d.rounded_rectangle((x, y + 1, x + 10, y + 10), 2, fill=(248, 248, 248), outline=(48, 48, 48))
    mk.text(im, x + 3, y + 1, button, color=(48, 48, 48), shadow=None, font=mk.s2.SMALL)
    if label:
        mk.text(im, x + 14, y, label, color=(248, 248, 248), shadow=(40, 72, 160), font=mk.s2.SMALL)


def main():
    rom, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    P = [rgba(p) for p in B.pictures(rom)]
    TOP, PRIZE, CARD, FILM, CARDS, MONS, BOX, FLASH, POP = 0, 1, 5, 9, 10, 14, 19, 27, 35

    def message(im, s):
        mk.window(im, 0, H - 48, 28, 4)
        mk.text(im, 16, H - 40, s)

    # 1. choosing
    choose = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    put(choose, P[TOP], -8, CHOOSE_DY)
    sel = 1
    for i in reversed(range(4)):   # (the left card in front: priority 20 + i)
        put(choose, P[CARDS + i], CARD_X[i], CARD_Y - (6 if i == sel else 0))
    choose = choose.convert('RGB')
    message(choose, 'Please select card no. 1.')

    # the card being scratched: the card in its colours, the icons under the film, the film where not scratched
    def card(colour, icons, gone):
        im = Image.new('RGBA', (W, H), (0, 0, 0, 255))
        put(im, P[CARD + colour], *CARD_D)
        for i, (x, y) in enumerate(MON):
            put(im, P[MONS + icons[i]], *ds(x, y))
        film = P[FILM].copy()
        fp = film.load()
        for i, part in gone:   # (the film taken off: Platinum's scratched pixels, all of a cell or part of one)
            l, r, t, b = CELLS[i]
            cx, cy = (l + r) // 2, (t + b) // 2
            for yy in range(t, b + 1):
                for xx in range(l, r + 1):
                    if part >= 1 or (((xx - cx) * 0.8) ** 2 + (yy - cy) ** 2) ** 0.5 < part * 22 \
                            and (xx * 7 + yy * 3) % 11 > 1:
                        fp[xx, yy] = (0, 0, 0, 0)
        put(im, film, *CARD_D)
        return im

    def foot(im, last=False):
        hint(im, 6, 145, 'R', '')   # (the prize card: shown as each card begins, and while R is held)
        hint(im, 234, 145, 'B', 'CANCEL' if last else 'NEXT CARD', 'r')

    icons = [0, 1, 4, 2, 0, 3, 1, 2, 0]
    # 2. a card begins: Platinum's centre box
    start = card(1, icons, [])
    put(start, P[BOX + 7], *ds(68, 68))
    start = start.convert('RGB')
    mk.text(start, 120, 69, 'Start card no. 1!', align='c')
    foot(start)

    # 3. scratching: two alike flashing in Platinum's boxes, the coin at work on a third
    play = card(1, icons, [(0, 1), (4, 1), (8, 0.6)])
    for i in (0, 4):
        put(play, P[FLASH + 3], *ds(MON[i][0] - 20, MON[i][1] - 10))
    l, r, t, b = CELLS[8]
    play = play.convert('RGB')
    coin(play, *ds((l + r) // 2 + 9, (t + b) // 2 + 5))
    foot(play)

    # 4. three alike: Platinum's winning pop-up with the three
    win = card(1, icons, [(0, 1), (4, 1), (8, 1)])
    put(win, P[POP + 4], *ds(38, 56))
    for x in (52, 108, 164):
        put(win, P[MONS + 0], *ds(x, 68))
    win = win.convert('RGB')
    foot(win)

    # 5. the prize card while R is held: each POKéMON x 3 and the BERRY it wins (Platinum's top screen)
    prize = Image.new('RGBA', (W, H), (0, 0, 0, 255))
    put(prize, P[PRIZE + 1], -8, PRIZE_DY)
    for row in range(4):
        put(prize, P[MONS + row], 44 - 8, 26 + row * 42 + PRIZE_DY)
    prize = prize.convert('RGB')
    for row, (berry, dy) in enumerate(zip(('POMEG BERRY', 'NUGGET', 'QUALOT BERRY', 'HONDEW BERRY'), (10, 12, 6, 8))):
        mk.text(prize, 176 - 8, (3, 8, 14, 19)[row] * 8 + dy + PRIZE_DY, berry, align='c')

    # 6. B: stop scratching?
    stop = card(1, icons, [(0, 1), (4, 0.5)]).convert('RGB')
    coin(stop, *ds(129, 84))
    mk.window(stop, 168, 56, 7, 4)
    mk.text(stop, 192, 64, 'YES')
    mk.text(stop, 192, 80, 'NO')
    mk.cursor(stop, 184, 68)
    message(stop, 'Stop scratching this card?')

    shots = [('scratch_choose', choose), ('scratch_start', start), ('scratch_play', play), ('scratch_win', win),
             ('scratch_prizes', prize), ('scratch_stop', stop)]
    for f in os.listdir(out):
        if f.startswith('scratch_') and f.endswith('.png'):
            os.remove(os.path.join(out, f))
    for name, im in shots:
        im.resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(out, name + '.png'))
    cols = 3
    sheet = Image.new('RGB', (cols * (W * 2 + 8) + 8, 2 * (H * 2 + 8) + 8), (32, 32, 40))
    for i, (name, im) in enumerate(shots):
        sheet.paste(im.resize((W * 2, H * 2), Image.NEAREST),
                    (8 + (i % cols) * (W * 2 + 8), 8 + (i // cols) * (H * 2 + 8)))
    sheet.save(os.path.join(out, 'sheet.png'))
    print(out)


if __name__ == '__main__':
    main()
