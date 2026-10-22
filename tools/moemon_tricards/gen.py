#!/usr/bin/env python3
"""Triple Battle cards from the hand-drawn mock-up (cards_mockup.png, 80x50):
the foe card (top, tail on the right) and the team card (bottom, tail on the left).

Writes src/mp/mp_tricard_gfx.h: both frames as color roles, the HP bar's end caps
and the original game's status tags (graphics/battle_interface/status.png).
The name, gender, level and HP fill are drawn by the game (src/mp/mp_tricards.c);
here they're cleared to the card's fill.

The fill color in the mock-up is (255,255,230). A few pixels next to the gender
symbol and the bar's ends are (255,255,222), (238,246,222) or (238,255,213): left
over from the original healthbox and from the striped background the foe card was
cut from, so they're fill too. The foe card's striped fill is the battle background
seen through the card.

usage: tools/moemon_tricards/gen.py  (from the repo root)
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(REPO, 'src', 'mp', 'mp_tricard_gfx.h')

W, H = 80, 22
FOE_TOP, TEAM_TOP = 0, 28
FOE_BODY, TEAM_BODY = 0, 6          # x of the card's body (its outer ring) in the 80-wide frame

# roles: order is the enum in the header
ROLES = [
    ('NONE', None),
    ('FILL', (255, 255, 230)),
    ('OUTER', (82, 106, 98)),
    ('OUTER_AA', (123, 148, 131)),
    ('CORNER', (57, 82, 65)),
    ('DARK', (32, 57, 0)),
    ('RING', (222, 213, 180)),
    ('RING_CORNER', (197, 189, 115)),
    ('TEXT', (74, 74, 82)),
    ('SHADOW', (222, 213, 180)),
    ('FEMALE', (255, 156, 148)),
    ('MALE', (65, 205, 255)),          # the original healthbox's male color, next to its female one
    ('BAR_EDGE', (82, 106, 90)),
    ('BAR_AA', (213, 205, 189)),
    ('BAR_WHITE', (255, 255, 255)),
    ('HP_TOP', (255, 180, 123)),
    ('HP_MID', (246, 148, 115)),
    ('HP_BOTTOM', (222, 106, 90)),
    # the HP fill: the original game's two rows per color (graphics/battle_interface/hpbar.png)
    ('GREEN_TOP', (90, 213, 131)),
    ('GREEN_BOTTOM', (115, 255, 172)),
    ('YELLOW_TOP', (205, 172, 8)),
    ('YELLOW_BOTTOM', (255, 230, 57)),
    ('RED_TOP', (172, 65, 74)),
    ('RED_BOTTOM', (255, 90, 57)),
    ('EMPTY_TOP', (74, 65, 90)),
    ('EMPTY_BOTTOM', (82, 106, 90)),
    # status tags: the original game's (status.png; sStatusIconColors in battle_interface.c)
    ('ST_LETTER', (255, 255, 222)),
    ('ST_AA', (222, 214, 181)),
    ('ST_PSN', None), ('ST_PAR', None), ('ST_SLP', None), ('ST_FRZ', None), ('ST_BRN', None),
    # owner strips in a linked Triple (seat colors, as before)
    ('OWN1', None), ('OWN2', None), ('OWN3', None),
]
GBA = {  # colors given as GBA RGB(r, g, b) 0-31
    'ST_PSN': (24, 12, 24), 'ST_PAR': (23, 23, 3), 'ST_SLP': (20, 20, 17), 'ST_FRZ': (17, 22, 28), 'ST_BRN': (28, 14, 10),
    'OWN1': (29, 10, 8), 'OWN2': (8, 17, 29), 'OWN3': (9, 23, 9),
}
IDX = {name: i for i, (name, _) in enumerate(ROLES)}

FRAME_COLORS = {
    (82, 106, 98): 'OUTER', (123, 148, 131): 'OUTER_AA', (57, 82, 65): 'CORNER', (32, 57, 0): 'DARK',
    (222, 213, 180): 'RING', (197, 189, 115): 'RING_CORNER',
}


def rgb555(c):
    r, g, b = c
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def role_color(name, col):
    if name in GBA:
        r, g, b = GBA[name]
        return r | (g << 5) | (b << 10)
    return rgb555(col)


def frame(im, top, body):
    """Roles of one card. Inside the ring (body+3..body+70, rows 3..18) everything is fill, except
    the ring's own corner pixels (the team card's top-left corner is drawn one pixel rounder)."""
    out = []
    for y in range(H):
        row = []
        for x in range(W):
            r, g, b, a = im.getpixel((x, top + y))
            inside = body + 3 <= x <= body + 70 and 3 <= y <= 18
            if a == 0:
                role = 'NONE'
            elif inside and (r, g, b) != (197, 189, 115):
                role = 'FILL'
            elif (r, g, b) in FRAME_COLORS:
                role = FRAME_COLORS[(r, g, b)]
            else:
                raise SystemExit('unexpected frame color %s at %d,%d' % ((r, g, b), x, top + y))
            row.append(IDX[role])
        out.append(row)
    return out


BAR_COLORS = {
    (82, 106, 90): 'BAR_EDGE', (213, 205, 189): 'BAR_AA', (255, 255, 255): 'BAR_WHITE',
    (255, 180, 123): 'HP_TOP', (246, 148, 115): 'HP_MID', (222, 106, 90): 'HP_BOTTOM',
    (90, 213, 131): 'GREEN_TOP', (115, 255, 172): 'GREEN_BOTTOM',
    (255, 255, 222): 'FILL', (238, 246, 222): 'FILL', (238, 255, 213): 'FILL', (255, 255, 230): 'FILL',
}


def bar(im):
    """The team card's bar: 6 rows from y=40, x=29 (left end) to x=76 (the ring's inside edge)."""
    rows = []
    for y in range(40, 46):
        rows.append([BAR_COLORS[im.getpixel((x, y))[:3]] for x in range(29, 77)])
    left = [r[:12] for r in rows]
    mid = [r[12] for r in rows]
    right = [r[-2:] for r in rows]
    for r in rows:   # every middle column is the same
        assert all(c == r[12] for c in r[12:-2]), r
    assert mid == ['BAR_EDGE', 'BAR_WHITE', 'GREEN_TOP', 'GREEN_BOTTOM', 'BAR_WHITE', 'BAR_EDGE'], mid
    return left, right


def check_foe_bar(im):
    """The foe card's bar has the same ends (its fill is see-through stripes)."""
    for y in range(12, 18):
        l = [BAR_COLORS.get(im.getpixel((x, y))[:3], 'FILL') for x in range(18, 30)]
        r = [BAR_COLORS.get(im.getpixel((x, y))[:3], 'FILL') for x in range(69, 71)]
        yield l, r


def status_tags():
    st = Image.open(os.path.join(REPO, 'graphics', 'battle_interface', 'status.png'))
    names = ['ST_PSN', 'ST_PAR', 'ST_SLP', 'ST_FRZ', 'ST_BRN']   # status.png row order
    tags = []
    for k in range(5):
        tag = []
        for y in range(8):
            row = []
            for x in range(20):
                v = st.getpixel((x, k * 8 + y)) & 15
                if v == 12:
                    row.append(names[k])
                elif v == 3:
                    row.append('ST_AA')
                elif v == 2:
                    # letters inside the tag; the corners outside it are the card's fill
                    edge = x in (0, 19) or y in (0, 7) or ((x in (1, 18)) and (y in (0, 7)))
                    row.append('FILL' if edge else 'ST_LETTER')
                else:
                    row.append('FILL')
            tag.append(row)
        tags.append(tag)
    return tags


def c_array(rows, width):
    return '\n'.join('    {' + ', '.join('%2d' % v for v in r) + '},' for r in rows)


def main():
    im = Image.open(os.path.join(HERE, 'cards_mockup.png')).convert('RGBA')
    assert im.size == (80, 50), im.size
    foe = frame(im, FOE_TOP, FOE_BODY)
    team = frame(im, TEAM_TOP, TEAM_BODY)
    left, right = bar(im)
    for l, r in check_foe_bar(im):
        pass
    tags = status_tags()

    o = []
    o.append('// Generated by tools/moemon_tricards/gen.py from tools/moemon_tricards/cards_mockup.png - do not edit.')
    o.append('// Triple Battle card art: frames as color roles (TC_*), HP bar ends, status tags.')
    o.append('enum {')
    for name, _ in ROLES:
        o.append('    TC_%s,' % name)
    o.append('    TC_COUNT')
    o.append('};')
    o.append('')
    o.append('// GBA BGR555 per role (TC_NONE unused)')
    o.append('static const u16 sTriCardColors[TC_COUNT] = {')
    for name, col in ROLES:
        if name == 'NONE':
            o.append('    [TC_NONE] = 0,')
        else:
            o.append('    [TC_%s] = 0x%04X,' % (name, role_color(name, col)))
    o.append('};')
    o.append('')
    o.append('#define TRICARD_FRAME_W %d' % W)
    o.append('#define TRICARD_FRAME_H %d' % H)
    o.append('#define TRICARD_FOE_BODY %d   // x of the card body (outer ring) in the frame' % FOE_BODY)
    o.append('#define TRICARD_TEAM_BODY %d' % TEAM_BODY)
    o.append('#define TRICARD_BODY_W 74')
    o.append('')
    for nm, fr in (('sTriCardFoeFrame', foe), ('sTriCardTeamFrame', team)):
        o.append('static const u8 %s[TRICARD_FRAME_H][TRICARD_FRAME_W] = {' % nm)
        o.append(c_array(fr, W))
        o.append('};')
        o.append('')
    o.append('// HP bar: 6 rows; 12 columns of left end (with the HP label), a middle column repeated, 2 columns of right end')
    o.append('static const u8 sTriCardBarLeft[6][12] = {')
    o.append(c_array([[IDX[c] for c in r] for r in left], 12))
    o.append('};')
    o.append('static const u8 sTriCardBarRight[6][2] = {')
    o.append(c_array([[IDX[c] for c in r] for r in right], 2))
    o.append('};')
    o.append('')
    o.append('// 20x8 status tags, PSN PAR SLP FRZ BRN')
    o.append('static const u8 sTriCardStatus[5][8][20] = {')
    for t in tags:
        o.append('  {')
        o.append(c_array([[IDX[c] for c in r] for r in t], 20))
        o.append('  },')
    o.append('};')
    open(OUT, 'w').write('\n'.join(o) + '\n')
    print('wrote', OUT)


if __name__ == '__main__':
    main()
