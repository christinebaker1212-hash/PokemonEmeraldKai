"""Checks every mock-up (tools/ui_styles/mockups.py runs them on each style it draws) for the faults a look can miss:

  edge      a battle screen changed outside KAI's own rectangles (layout.py): a box too wide, a stray pixel
  fit       a battle box whose drawn pixels don't fill its KAI rectangle (off by more than a pixel on a side; a
            style whose FIT is 'inside', the Game Boy's smaller HUDs, need only stay within it)
  seam      a one-pixel gap inside a box: the scene showing through between two drawn pixels
  overlap   two pieces of text whose inked pixels cross
  collide   text inked over something other than the plain ground it sits on (a bar, a picture, a frame): the
            screen is drawn again without its words to see what lies under each
  clip      text running off the screen

Each finding names the screen and the place; a clean style prints nothing but its OK line."""
from PIL import Image

import layout as Lo
from common import H, W, shot

BATTLES = {
    'battle_single': ('s_single', [Lo.SINGLE_FOE, Lo.SINGLE_MINE], []),
    'battle_moves': ('s_single', [Lo.SINGLE_FOE, Lo.SINGLE_MINE], []),
    'battle_double': ('s_double', Lo.DOUBLE_FOES + Lo.DOUBLE_MINE, []),
    'battle_triple': ('s_triple', Lo.TRIPLE_FOES + Lo.TRIPLE_MINE,
                      [(x, y, x + 11, y + 11) for x, y in Lo.TRIPLE_FOE_ICONS + Lo.TRIPLE_MINE_ICONS]),
}
PANEL = (0, 112, 240, 160)          # the message / command / move panels


def inside(x, y, rects):
    return any(r[0] <= x < r[2] and r[1] <= y < r[3] for r in rects)


def check_battle(name, im, kai, out, fill=True):
    scene_name, huds, icons = BATTLES[name]
    scene = shot(kai, scene_name).convert('RGB')
    im = im.convert('RGB')
    sp, ip = scene.load(), im.load()
    changed = [[ip[x, y] != sp[x, y] for x in range(W)] for y in range(H)]
    allowed = huds + icons + [PANEL]
    stray = [(x, y) for y in range(H) for x in range(W) if changed[y][x] and not inside(x, y, allowed)]
    if stray:
        xs, ys = [p[0] for p in stray], [p[1] for p in stray]
        out.append('%s edge: %d pixels changed outside KAI\'s rectangles, within (%d,%d)-(%d,%d)'
                   % (name, len(stray), min(xs), min(ys), max(xs), max(ys)))
    for r in huds:
        x0, y0, x1, y1 = r
        pts = [(x, y) for y in range(y0, y1) for x in range(x0, x1) if changed[y][x]]
        if not pts:
            out.append('%s fit: nothing drawn in %s' % (name, r))
            continue
        bx0, by0 = min(p[0] for p in pts), min(p[1] for p in pts)
        bx1, by1 = max(p[0] for p in pts) + 1, max(p[1] for p in pts) + 1
        if fill and max(abs(bx0 - x0), abs(by0 - y0), abs(bx1 - x1), abs(by1 - y1)) > 1:
            out.append('%s fit: box %s drawn over (%d,%d)-(%d,%d)' % (name, r, bx0, by0, bx1, by1))
        for y in range(y0, y1):
            for x in range(x0, x1):
                if changed[y][x]:
                    continue
                h = 0 < x < W - 1 and changed[y][x - 1] and changed[y][x + 1]
                v = 0 < y < H - 1 and changed[y - 1][x] and changed[y + 1][x]
                if h or v:
                    out.append('%s seam: the scene shows through at (%d,%d) inside %s' % (name, x, y, r))


def check_text(name, inks, out):
    for i in range(len(inks)):
        (a, sa) = inks[i]
        if a[0] < 0 or a[1] < 0 or a[2] > W or a[3] > H:
            out.append('%s clip: "%s" runs off the screen %s' % (name, sa, a))
        for j in range(i + 1, len(inks)):
            (b, sb) = inks[j]
            ox = min(a[2], b[2]) - max(a[0], b[0])
            oy = min(a[3], b[3]) - max(a[1], b[1])
            if ox > 0 and oy > 0 and ox * oy >= 2 and not (ox <= 1 or oy <= 1):
                out.append('%s overlap: "%s" %s and "%s" %s' % (name, sa, a, sb, b))


def check_collide(name, im, bare, inks, out):
    """text inked over something other than its ground: BARE is the same screen drawn without words"""
    a, b = im.convert('RGB').load(), bare.convert('RGB').load()
    for (bb, s) in inks:
        x0, y0, x1, y1 = bb
        x0, y0, x1, y1 = max(0, x0), max(0, y0), min(W, x1), min(H, y1)
        counts = {}
        for y in range(y0, y1):
            for x in range(x0, x1):
                counts[b[x, y]] = counts.get(b[x, y], 0) + 1
        area = max(1, (x1 - x0) * (y1 - y0))
        ground = {c for c, n in counts.items() if n * 5 >= area}
        hits = [(x, y) for y in range(y0, y1) for x in range(x0, x1) if a[x, y] != b[x, y] and b[x, y] not in ground]
        if len(hits) >= 4:
            xs, ys = [h[0] for h in hits], [h[1] for h in hits]
            out.append('%s collide: "%s" inked over other art at %d pixels, (%d,%d)-(%d,%d)'
                       % (name, s, len(hits), min(xs), min(ys), max(xs), max(ys)))


def check_style(style, screens, kai, bare=None):
    """the findings, and the ones the style's ORIGINAL list says are its game's own layout (kept and shown, with
    the reason, never hidden)"""
    out = []
    for name, im in screens.items():
        if name in BATTLES:
            check_battle(name, im, kai, out, getattr(style, 'FIT', 'fill') == 'fill')
        check_text(name, style.logs.get(name, []), out)
        if bare is not None:
            check_collide(name, im, bare[name], style.logs.get(name, []), out)
    allowed = getattr(style, 'ORIGINAL', [])
    found, kept = [], []
    for f in out:
        why = next((w for (screen, kind, words, w) in allowed
                    if f.startswith('%s %s:' % (screen, kind)) and
                    (words is None or any('"%s"' % t in f for t in words))), None)
        (kept if why else found).append(f if not why else f + '  [allowed: %s]' % why)
    return found, kept


def bare_screens(style):
    """the style's screens again without their words (each word measured, so everything else lands where it did)"""
    draw = style.text

    def wordless(im, x, y, s, *a, **k):
        return draw(Image.new('RGBA', im.size, (0, 0, 0, 0)), x, y, s, *a, **k)
    style.text = wordless
    try:
        return style.all()
    finally:
        style.text = draw


def log_text(style):
    """wrap a style's text() so each screen keeps its words' inked rectangles (style.inks)"""
    draw = style.text

    def logged(im, x, y, s, *a, **k):
        # (drawn on a canvas 64 pixels larger all round, so ink that would fall off the screen is seen)
        probe = Image.new('RGBA', (im.width + 128, im.height + 128), (0, 0, 0, 0))
        draw(probe, x + 64, y + 64, s, *a, **k)
        bb = probe.getbbox()
        if bb is not None and s.strip():
            style.inks.append(((bb[0] - 64, bb[1] - 64, bb[2] - 64, bb[3] - 64), s))
        return draw(im, x, y, s, *a, **k)
    style.text = logged
    style.inks = []


def worst_case(make, kai):
    """the battle screens again with the longest words a box must hold (ten-letter names, level 100, 999 HP), checked
    the same way: a style must never cut a name or a level, nor let them run into anything"""
    import base
    saved = {k: getattr(base, k) for k in ('FOE', 'MINE', 'DOUBLE_FOES', 'DOUBLE_MINE', 'TRIPLE_FOES', 'TRIPLE_MINE')}
    names = ['MASQUERAIN', 'CHARMELEON', 'BUTTERFREE']

    def big(d, i):
        d = dict(d, name=names[i % 3], level=100)
        if 'maxhp' in d:
            d.update(hp=999, maxhp=999)
        return d
    try:
        base.FOE, base.MINE = big(saved['FOE'], 0), big(saved['MINE'], 1)
        for k in ('DOUBLE_FOES', 'DOUBLE_MINE', 'TRIPLE_FOES', 'TRIPLE_MINE'):
            setattr(base, k, [big(m, i) for i, m in enumerate(saved[k])])
        screens = ('battle_single', 'battle_double', 'battle_triple')

        def run(st, wordless=False):
            if wordless:
                draw = st.text
                st.text = lambda im, *a, **k: draw(Image.new('RGBA', im.size, (0, 0, 0, 0)), *a, **k)
            out = {}
            for n in screens:
                st.inks = []
                out[n] = getattr(st, n)()
                st.logs[n] = st.inks
            return out
        bare = run(make(), True)
        st = make()
        log_text(st)
        try:
            shown = run(st)
        except ValueError as e:      # (a style that finds its words don't fit says so)
            return ['worst case: %s' % e], [], {}
        found, kept = check_style(st, shown, kai, bare)
        return ['worst case ' + f for f in found], ['worst case ' + f for f in kept], shown
    finally:
        for k, v in saved.items():
            setattr(base, k, v)
