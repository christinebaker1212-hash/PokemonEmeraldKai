"""UI STYLE packs: what the game draws a style from (docs/ui_styles/UI_STYLES.md "Building it").

A style is built here from the same code that drew its mock-ups (gbstyle.py, fr.py, pt.py, bw.py) and written into one
pack the game reads (src/mp/mp_uistyle.c): the style's pieces as pictures, its fonts (a glyph for every character
code of the game's own charset, charmap.txt) and its SPECS, one per thing it draws (a battle box of each kind, the
battle's message, command and move panels, the field's message box, the START menu): a list of OPS, each drawn when
its condition holds, the live words and numbers filled in by the game (FIELDS). render() below is the game's drawing
done in Python, so a spec can be checked against the approved mock-up before the game ever draws it.

The pack (little-endian):
    'UIS1' u16 version u16 nImages u16 nFonts u16 nSpecs u16 nStrings u32 0
    images:  u16 w u16 h u8 alpha(0/1) u8 0 u16 0, w*h u16 colours (BGR555, bit 15 = drawn), [w*h u8 alpha]
    fonts:   u8 cellH u8 0 u16 0, 256 x (u8 adv s8 dx s8 dy u8 w u8 h u8 0 u16 0 u32 offset), u32 nBytes, bytes:
             a byte a pixel, 0 clear, 1 ink, 2 shadow, 3 outline
    specs:   char name[24] u16 w u16 h u16 canvasW u16 canvasH u16 nOps u16 0, ops (OP_SIZE bytes each)
    strings: u16 len, bytes (the game's charset; FIELD a field: 0xFD then the field's number)
"""
import struct

from PIL import Image

VERSION = 1

# ------------------------------------------------------------------------------------------------- the game's charset
_CHARMAP = None


def charmap():
    """{'A': 0xBB, ...}: the single characters of charmap.txt (the first code each has)"""
    global _CHARMAP
    if _CHARMAP is None:
        import os
        import re
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'charmap.txt')
        m = {}
        for line in open(path, encoding='utf-8'):
            g = re.match(r"^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line.rstrip('\n'))
            if g and g.group(1) not in m:
                m[g.group(1)] = int(g.group(2), 16)
        m.setdefault(' ', 0x00)
        _CHARMAP = m
    return _CHARMAP


def kai_codes():
    """{code: char} for the codes a font holds a glyph for (the first character each code is written as)"""
    out = {}
    for ch, c in charmap().items():
        out.setdefault(c, ch)
    return out


FIELD = 0xFD           # (a field in a string: 0xFD and the field's number)
EOS = 0xFF
NEWLINE = 0xFE

# the fields a string can hold (src/mp/mp_uistyle.c fills them)
F_NAME, F_LEVEL, F_HP, F_MAXHP, F_HP3, F_MAXHP3, F_GENDER = 1, 2, 3, 4, 5, 6, 7
F_ITEM0 = 16           # a list's row (START's entries, the moves): F_ITEM0 + i
F_PP, F_MAXPP, F_TYPE = 32, 33, 34
F_TEXT0 = 40           # a captured window's words (the battle's message, its prompt...): F_TEXT0 + window


def encode(s):
    """S in the game's charset; '{N}' writes field N; '{LV}' the level mark; '\\n' a new line"""
    out, i, cm = bytearray(), 0, charmap()
    while i < len(s):
        if s[i] == '{':
            j = s.index('}', i)
            tok = s[i + 1:j]
            if tok == 'LV':
                out.append(0x34)
            else:
                out += bytes([FIELD, int(tok)])
            i = j + 1
            continue
        if s[i] == '\n':
            out.append(NEWLINE)
        else:
            out.append(cm.get(s[i], cm['?']))
        i += 1
    return bytes(out)


# ------------------------------------------------------------------------------------------------- conditions
# (bits the game sets before drawing a spec; an op is drawn when every bit of its COND is set and none of its UNLESS)
C_LV3 = 1 << 0          # a level of three digits
C_MALE = 1 << 1
C_FEMALE = 1 << 2
C_STATUS = 1 << 3       # any status; and which:
C_PSN, C_PAR, C_SLP, C_FRZ, C_BRN = (1 << 4), (1 << 5), (1 << 6), (1 << 7), (1 << 8)
C_LONG = 1 << 9         # the name wider than the spec's own measure (its OP_LONG)
C_SEL0 = 1 << 10        # the cursor on row 0..9: C_SEL0 << i
C_WAIT = 1 << 20        # a message waiting for A
C_EXP = 1 << 21         # the box shows an EXP bar (the player's own, singles)
C_LV1 = 1 << 22         # a level of one digit
C_ALT = 1 << 23         # the move's info box shows something other than its PP and type (R's view, a gimmick)
C_ROWS0 = 1 << 24       # (a list's rows: drawn row i when i < the rows, the op's N; not a bit)

STATUS_BITS = {'PSN': C_PSN, 'PAR': C_PAR, 'SLP': C_SLP, 'FRZ': C_FRZ, 'BRN': C_BRN}

# ------------------------------------------------------------------------------------------------- ops
OP_IMG = 1        # image A at (x, y)
OP_TEXT = 2       # string A in font F, colours ink/shadow/outline, align (0 left, 1 right, 2 centre), up to width W
#                   (wider: font B, or 0); the x after it into register R
OP_BAR = 3        # bar: field A (a fraction, F_HP: hp / maxhp, F_EXP...) through fill images B (green), C (yellow),
#                   D (red) at (x, y); E the fill's direction (0 from the left, 1 from the right)
OP_FILL = 4       # rectangle (x, y, w, h) in colour ink, alpha in A (255 solid)
OP_MSG = 5        # captured window A's words: lines from (x, y), H the line's height, N the lines shown, font F
OP_LONG = 6       # set C_LONG when field A's width in font F is over W
OP_BACKING = 7    # (a canvas spec) colour ink at alpha A behind the drawn pixels' box, B pixels round it
OP_IMGSEL = 8     # image A + (the field B's value, a list's row type...) at (x, y)
OP_ROWMAP = 9     # (a canvas spec) the canvas's rows kept, string A (a byte a row), when it's placed
OP_BLEND = 10     # image A over (x, y) blended as the DS's BLDALPHA: each pixel its colour plus EVB/16 of what's under
F_EXP = 64
F_HPFRAC = 65

OP_FMT = '<BBBBIIhhHHHHHHHHHHH'
OP_SIZE = struct.calcsize(OP_FMT)
SPEC_NAME = 24


def rgb555(c):
    r, g, b = c[:3]
    return (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10


class Op:
    def __init__(self, kind, x=0, y=0, a=0, b=0, c=0, d=0, e=0, font=0, ink=None, shadow=None, outline=None,
                 align=0, w=0, h=0, n=0, reg=0, cond=0, unless=0, rel=False):
        self.kind, self.x, self.y, self.a, self.b, self.c, self.d, self.e = kind, x, y, a, b, c, d, e
        self.font, self.ink, self.shadow, self.outline = font, ink, shadow, outline
        self.align, self.w, self.h, self.n, self.reg = align, w, h, n, reg
        self.cond, self.unless, self.rel = cond, unless, rel

    def pack(self):
        flags = (1 if self.rel else 0) | (self.reg & 7) << 1
        col = lambda c: 0 if c is None else rgb555(c) | 0x8000   # noqa: E731
        return struct.pack(OP_FMT, self.kind, self.font, self.align, flags, self.cond, self.unless, self.x, self.y,
                           self.a, self.b, self.c, self.d, self.e, col(self.ink), col(self.shadow),
                           col(self.outline), self.w, self.h, self.n)


class Spec:
    """one thing a style draws, W x H, and its ops. CANVAS (cw, ch): the ops draw onto a canvas that size, which its
    OP_ROWMAP then cuts (the rows kept) and places in the W x H, an OP_BACKING behind it (clipped to the W x H)"""
    def __init__(self, name, w=240, h=160, canvas=None):
        self.name, self.w, self.h, self.canvas, self.ops = name, w, h, canvas, []

    def add(self, *a, **k):
        self.ops.append(Op(*a, **k))
        return self


class Font:
    """a style's font as the game reads it: a glyph for each code of the game's charset"""
    def __init__(self, cell_h):
        self.cell_h, self.glyphs = cell_h, {}

    def add(self, code, adv, dx, dy, pix):
        """PIX: rows of 0 clear / 1 ink / 2 shadow / 3 outline, its top-left (dx, dy) from the pen"""
        self.glyphs[code] = (adv, dx, dy, pix)


def font_from(draw, measure, cell_h=16, extra=None):
    """a Font from a style's own text drawing: DRAW(im, x, y, s, ink, shadow, outline) draws S with those colours,
    MEASURE(s) its width. Each code is drawn alone in three colours and sorted back into ink, shadow and outline.
    EXTRA {code: str} draws a code as something else (the level mark, a style's own token)"""
    INK, SH, OUT = (255, 0, 0), (0, 255, 0), (0, 0, 255)
    f = Font(cell_h)
    codes = kai_codes()
    if extra:
        codes.update(extra)
    for code, ch in sorted(codes.items()):
        if code in (FIELD, EOS, NEWLINE):
            continue
        im = Image.new('RGBA', (96, 64), (0, 0, 0, 0))
        try:
            if ch != ' ':      # (a space is only its width: some styles' own drawing skips it)
                draw(im, 16, 16, ch, INK, SH, OUT)
            adv = measure(ch)
        except (KeyError, IndexError, ValueError):
            continue
        bb = im.getbbox()
        if bb is None:
            f.add(code, adv, 0, 0, [])
            continue
        p = im.load()
        pix = []
        for y in range(bb[1], bb[3]):
            row = []
            for x in range(bb[0], bb[2]):
                c = p[x, y]
                row.append(0 if c[3] == 0 else 1 if c[:3] == INK else 2 if c[:3] == SH else 3 if c[:3] == OUT
                           else 1)
            pix.append(row)
        f.add(code, adv, bb[0] - 16, bb[1] - 16, pix)
    return f


class Pack:
    def __init__(self, style):
        self.style = style
        self.images, self.image_ids = [], {}
        self.fonts, self.font_ids = [], {}
        self.specs = []
        self.strings, self.string_ids = [], {}

    def image(self, name, im):
        """IM (RGBA) under NAME: its number (one picture kept once)"""
        if name in self.image_ids:
            return self.image_ids[name]
        self.image_ids[name] = len(self.images)
        self.images.append(im.convert('RGBA'))
        return self.image_ids[name]

    def font(self, name, f):
        if name not in self.font_ids:
            self.font_ids[name] = len(self.fonts)
            self.fonts.append(f)
        return self.font_ids[name]

    def string(self, s):
        b = encode(s) if isinstance(s, str) else bytes(s)
        if b not in self.string_ids:
            self.string_ids[b] = len(self.strings)
            self.strings.append(b)
        return self.string_ids[b]

    def spec(self, sp):
        self.specs.append(sp)
        return sp

    def write(self, path):
        out = bytearray(struct.pack('<4sHHHHHI', b'UIS1', VERSION, len(self.images), len(self.fonts),
                                    len(self.specs), len(self.strings), 0))
        for im in self.images:
            w, h = im.size
            p = im.load()
            alpha = any(0 < p[x, y][3] < 255 for y in range(h) for x in range(w))
            out += struct.pack('<HHBBH', w, h, 1 if alpha else 0, 0, 0)
            for y in range(h):
                for x in range(w):
                    c = p[x, y]
                    out += struct.pack('<H', (rgb555(c) | 0x8000) if c[3] else 0)
            if alpha:
                out += bytes(p[x, y][3] for y in range(h) for x in range(w))
        for f in self.fonts:
            out += struct.pack('<BBH', f.cell_h, 0, 0)
            table, data = [], bytearray()
            for code in range(256):
                g = f.glyphs.get(code)
                if g is None:
                    table.append(struct.pack('<BbbBBBHI', 0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFF))
                    continue
                adv, dx, dy, pix = g
                h = len(pix)
                w = len(pix[0]) if h else 0
                table.append(struct.pack('<BbbBBBHI', adv, dx, dy, w, h, 0, 0, len(data)))
                for row in pix:
                    data += bytes(row)
            out += b''.join(table)
            out += struct.pack('<I', len(data)) + data
        for sp in self.specs:
            name = sp.name.encode()[:SPEC_NAME - 1]
            cw, ch = sp.canvas or (0, 0)
            out += struct.pack('<%dsHHHHHH' % SPEC_NAME, name, sp.w, sp.h, cw, ch, len(sp.ops), 0)
            for op in sp.ops:
                out += op.pack()
        for s in self.strings:
            out += struct.pack('<H', len(s)) + s
        open(path, 'wb').write(bytes(out))
        return len(out)


# ------------------------------------------------------------------------------------------------- the game's drawing
def glyph_draw(dst, f, x, y, code, ink, shadow, outline):
    g = f.glyphs.get(code)
    if g is None:
        return 0
    adv, dx, dy, pix = g
    p = dst.load()
    cols = (None, ink, shadow, outline)
    for yy, row in enumerate(pix):
        for xx, v in enumerate(row):
            c = cols[v]
            X, Y = x + dx + xx, y + dy + yy
            if v and c is not None and 0 <= X < dst.width and 0 <= Y < dst.height:
                p[X, Y] = tuple(c[:3]) + (255,)
    return adv


def expand(b, data):
    """a string with its fields filled in (DATA {field: bytes})"""
    out, i = bytearray(), 0
    while i < len(b):
        if b[i] == FIELD:
            out += data.get(b[i + 1], b'')
            i += 2
            continue
        out.append(b[i])
        i += 1
    return bytes(out)


def width(f, b):
    return sum(f.glyphs.get(c, (0,))[0] for c in b if c not in (NEWLINE,))


def render(pack, sp, data, flags, dst=None, at=(0, 0)):
    """SP drawn as the game draws it: DATA {field: bytes, a number or a fraction}, FLAGS its condition bits; onto DST
    at AT (a new picture when DST is None). Returns the picture"""
    if dst is None:
        dst = Image.new('RGBA', (sp.w, sp.h), (0, 0, 0, 0))
    canvas = Image.new('RGBA', sp.canvas, (0, 0, 0, 0)) if sp.canvas else dst
    ox, oy = (0, 0) if sp.canvas else at
    regs = [0] * 8
    texts = {k: (v if isinstance(v, (bytes, bytearray)) else encode(str(v))) for k, v in data.items()
             if not isinstance(v, float)}
    rowmap, place, backing = None, (0, 0), None
    for op in sp.ops:
        if op.kind == OP_LONG:
            f = pack.fonts[op.font]
            if width(f, texts.get(op.a, b'')) > op.w:
                flags |= C_LONG
            continue
        if (flags & op.cond) != op.cond or (flags & op.unless):
            continue
        x = op.x + (regs[op.reg] if op.rel else 0) + ox
        y = op.y + oy
        if op.kind == OP_IMG:
            _paste(canvas, pack.images[op.a], x, y)
        elif op.kind == OP_IMGSEL:
            _paste(canvas, pack.images[op.a + int(data.get(op.b, 0))], x, y)
        elif op.kind == OP_BLEND:
            _blend(canvas, pack.images[op.a], x, y, op.b)
        elif op.kind == OP_FILL:
            _paste(canvas, Image.new('RGBA', (op.w, op.h), tuple(op.ink[:3]) + (op.a,)), x, y)
        elif op.kind == OP_TEXT:
            f = pack.fonts[op.font]
            s = expand(pack.strings[op.a], texts)
            if op.w and op.b and width(f, s) > op.w:
                f = pack.fonts[op.b - 1]
            wd = width(f, s)
            x0 = x - (wd if op.align == 1 else wd // 2 if op.align == 2 else 0)
            for code in s:
                x0 += glyph_draw(canvas, f, x0, y, code, op.ink, op.shadow, op.outline)
            regs[op.reg] = x0 - ox
        elif op.kind == OP_MSG:
            f = pack.fonts[op.font]
            lines = texts.get(F_TEXT0 + op.a, b'').split(bytes([NEWLINE]))[-op.n:] if op.n else []
            for i, line in enumerate(lines):
                x0 = x
                for code in line:
                    x0 += glyph_draw(canvas, f, x0, y + i * op.h, code, op.ink, op.shadow, op.outline)
        elif op.kind == OP_BAR:
            _bar(canvas, pack, op, data.get(op.a, 0.0), x, y)
        elif op.kind == OP_BACKING:
            backing = op
        elif op.kind == OP_ROWMAP:
            rowmap, place = pack.strings[op.a], (op.x, op.y)
    if sp.canvas:
        if rowmap is not None:
            kept = Image.new('RGBA', (canvas.width, len(rowmap)), (0, 0, 0, 0))
            for ny, sy in enumerate(rowmap):
                kept.paste(canvas.crop((0, sy, canvas.width, sy + 1)), (0, ny))
            canvas = kept
        area = Image.new('RGBA', (sp.w, sp.h), (0, 0, 0, 0))
        if backing is not None:
            bb = canvas.getbbox()
            if bb:
                r = backing.b
                x0, y0 = max(0, place[0] + bb[0] - r), max(0, place[1] + bb[1] - r)
                x1, y1 = min(sp.w, place[0] + bb[2] + r), min(sp.h, place[1] + bb[3] + r)
                area.paste(tuple(backing.ink[:3]) + (backing.a,), (x0, y0, x1, y1))
        _paste(area, canvas, place[0], place[1])
        _paste(dst, area, at[0], at[1])
    return dst


def _paste(dst, im, x, y):
    layer = Image.new('RGBA', dst.size, (0, 0, 0, 0))
    layer.paste(im, (x, y), im)
    dst.alpha_composite(layer)


def _blend(dst, im, x, y, evb):
    p, q = dst.load(), im.load()
    for yy in range(im.height):
        for xx in range(im.width):
            c = q[xx, yy]
            X, Y = x + xx, y + yy
            if c[3] and 0 <= X < dst.width and 0 <= Y < dst.height:
                u = p[X, Y]
                p[X, Y] = tuple(min(248, c[k] + (u[k] * evb >> 4) & 0xF8) for k in range(3)) + (255,)


def _bar(dst, pack, op, frac, x, y):
    """the bar's fill: the fill picture of its colour (green over half, yellow over a fifth, red), its columns
    (left to right, or right to left) as far as FRAC goes; a sliver for anything above nothing"""
    tier = op.b if frac > 0.5 else op.c if frac > 0.2 else op.d
    im = pack.images[tier]
    cols = [cx for cx in range(im.width) if any(im.getpixel((cx, cy))[3] for cy in range(im.height))]
    if op.e:
        cols = cols[::-1]
    n = round(frac * len(cols))
    if frac > 0 and n == 0:
        n = 1
    keep = set(cols[:n])
    part = Image.new('RGBA', im.size, (0, 0, 0, 0))
    q, p = im.load(), part.load()
    for cx in keep:
        for cy in range(im.height):
            p[cx, cy] = q[cx, cy]
    _paste(dst, part, x, y)
