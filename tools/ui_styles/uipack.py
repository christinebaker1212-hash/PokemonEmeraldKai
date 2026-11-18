"""UI STYLE packs: what the game draws a style from (docs/ui_styles/UI_STYLES.md "Building it").

A style is built here from the same code that drew its mock-ups (gbstyle.py, fr.py, pt.py, bw.py) and written into one
pack the game reads (src/mp/mp_uistyle.c): the style's pieces as pictures, its fonts (a glyph for every character
code of the game's own charset, charmap.txt) and its SPECS, one per thing it draws (a battle box of each kind, the
battle's message, command and move panels, the field's message box, the START menu): a list of OPS, each drawn when
its condition holds, the live words and numbers filled in by the game (FIELDS). render() below is the game's drawing
done in Python, so a spec can be checked against the approved mock-up before the game ever draws it.

The pack (little-endian):
    'UIS1' u16 version u16 nImages u16 nFonts u16 nSpecs u16 nStrings u32 0
    images:  u16 w u16 h s16 dx s16 dy u8 alpha(0/1) u8 0 u16 0, w*h u16 colours (BGR555, bit 15 = drawn),
             [w*h u8 alpha]: the picture's drawn area, (dx, dy) from its own top-left
    fonts:   u8 cellH u8 0 u16 0, 256 x (u8 adv s8 dx s8 dy u8 w u8 h u8 0 u16 0 u32 offset), u32 nBytes, bytes:
             a byte a pixel, 0 clear, 1 ink, 2 shadow, 3 outline
    specs:   char name[24] u16 w u16 h u16 canvasW u16 canvasH u16 nOps u16 0, ops (OP_SIZE bytes each)
    strings: u16 len, bytes (the game's charset; FIELD a field: 0xFD then the field's number)
"""
import struct

from PIL import Image

VERSION = 2

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
    for ch, c in EXTRA_CODES.items():
        out[c] = ch
    return out


FIELD = 0xFD           # (a field in a string: 0xFD and the field's number)
EOS = 0xFF
NEWLINE = 0xFE

# the fields a string can hold (src/mp/mp_uistyle.c fills them)
F_NAME, F_LEVEL, F_HP, F_MAXHP, F_HP3, F_MAXHP3, F_GENDER = 1, 2, 3, 4, 5, 6, 7
F_ITEM0 = 16           # a list's row (START's entries, the moves): F_ITEM0 + i
F_PP, F_MAXPP, F_TYPE = 32, 33, 34
F_TEXT0 = 40           # a captured window's words (the battle's message, its prompt...): F_TEXT0 + window


EXTRA_CODES = {'◀': 0x7B, '▼': 0x7A, '▲': 0x79}   # (characters the game's charset has no letter for, at its LEFT_ARROW and
#                                        DOWN_ARROW codes; a style's font draws them)


def encode(s):
    """S in the game's charset; '{N}' writes field N; '{LV}' the level mark; '\\n' a new line"""
    out, i, cm = bytearray(), 0, dict(charmap(), **EXTRA_CODES)
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
C_MOEWORD = 1 << 25     # the battle's POKéMON command reads MOéMON
C_STDCMD = 1 << 26      # the battle's commands are FIGHT, BAG, POKéMON, RUN
C_HELD0 = 1 << 14       # (b.moves) the move held to swap (SELECT), rows 0-3: C_HELD0 << i (C_SEL0's rows 4-7)
C_CAUGHT = 1 << 27      # (a battle box's spec) a wild foe whose kind the POKéDEX has caught
C_NOBG = 1 << 28        # (a battle box's spec) OPTIONS > DISPLAY > WHITE BG OFF: no paper behind the box
C_ROWS0 = 1 << 24       # (a list's rows: drawn row i when i < the rows, the op's N; not a bit)

TYPE_NAMES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', '???', 'FIRE',
              'WATER', 'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']   # (include/constants/pokemon.h)

STATUS_BITS = {'PSN': C_PSN, 'PAR': C_PAR, 'SLP': C_SLP, 'FRZ': C_FRZ, 'BRN': C_BRN}

# ------------------------------------------------------------------------------------------------- ops
OP_IMG = 1        # image A at (x, y)
OP_TEXT = 2       # string A in font F, colours ink/shadow/outline, align (0 left, 1 right, 2 centre, 3 centre on the
#                   8-pixel grid), up to width W
#                   (wider: font B, or 0); the x after it into register R; H: nothing drawn from H rows below y
OP_BAR = 3        # bar: field A (a fraction, F_HP: hp / maxhp, F_EXP...) through fill images B (green), C (yellow),
#                   D (red) at (x, y); E the fill's direction (0 from the left, 1 from the right)
OP_FILL = 4       # rectangle (x, y, w, h) in colour ink, alpha in A (255 solid)
OP_MSG = 5        # captured window A's words, their page: lines from (x, y) H apart, N of them, font F; a line wider
#                   than W wrapped at its last space that fits; a page of more lines than N closes them up to rows B
#                   apart from y D, C of them (past that the last ones printed show), or (no B) in font E - 1 when
#                   that fits them in N; where the last line ends into
#                   register R (x) and R + 1 (its y)
# (an op with REL is placed from register R's x, with RELY from register R + 1's y)
# a spec named words.<menu> holds a style's words for each entry of a menu: OP_TEXT ops, A the string, B the entry
# (start_menu.c MENU_ACTION_*), C its line; one named map.<menu> a value for each (any op: B the entry, C the value)
OP_LONG = 6       # set C_LONG when field A's width in font F is over W
OP_BACKING = 7    # (a canvas spec) colour ink at alpha A behind the drawn pixels' box, B pixels round it
OP_IMGSEL = 8     # image A + (the field B's value, a list's row type...) at (x, y)
OP_ROWMAP = 9     # (a canvas spec) the canvas's rows kept, string A (a byte a row), when it's placed
OP_BLEND = 10     # image A over (x, y) blended as the DS's BLDALPHA: each pixel its colour plus EVB/16 of what's under
OP_ICON = 11      # where Kai's sprite A (0 a party slot's POKéMON icon, 1 its held item, 2 its status) goes: its middle
#                   at (x, y) (the game draws it there; nothing is drawn by the op)
P_EMPTY, P_EGG, P_DESC, P_FAINT, P_SWITCH = 1 << 27, 1 << 28, 1 << 29, 1 << 30, 1 << 31   # (a party slot's spec)
F_EXP = 64
F_HPFRAC = 65
F_ICON0 = 70           # a list row's icon (START's: the style's map.start of each entry)
OP_CHOICES = 12   # an OPTIONS row's choices (fields A...: F_NCHOICE of them, F_CUR the one on) from (x, y) in W: all of them
#                   N apart (ALIGN 0 from x, 1 ending at x + W), the one on in ink / shadow, the rest in colours C / D;
#                   only the one on when they don't fit (font E - 1 when it's wider than W); glyph B just before the one on
OP_LIST = 13      # (a screen's) its list: N rows from (x, y), H apart, each drawn with the screen's '<name>.row' spec
OP_ICONSET = 15   # (spec "icons") a style's own party icons: string A a byte a national POKéDEX number (0 the EGG, the last
                  # any other) giving its icon, pictures from B (icon i's two frames B + 2i, B + 2i + 1), C of them; D the set
                  # (0 ORIGINAL, 1 EXTENDED: OPTIONS > DISPLAY > MENU POKéMON)
OP_SNAP = 14      # the last field or battle frame the game showed (a style's BAG drawn over it, as RED's ITEM box is)
F_QTY, F_QTY2, F_TAG = 93, 94, 95         # a BAG row's count, its count two wide, the button it's registered to
B_CLOSE, B_QTY, B_TAG, B_SWAP = 1 << 27, 1 << 28, 1 << 29, 1 << 30   # a BAG row: CLOSE BAG, has a count, a button, moved
SNAP = -1                                 # (render()'s DATA: the picture OP_SNAP draws)
# the summary (src/mp/mp_uistyle.c SummaryScreen: MP_Summary_Words)
(F_SPECIES, F_DEXNO, F_OT, F_IDNO, F_HELD, F_NATURE, F_ABILITY, F_METLV, F_METPLACE, F_EXPPTS,
 F_NEXTLV) = range(96, 107)
F_STAT0, F_IV0, F_EV0, F_TYPE1, F_TYPE2 = 107, 112, 118, 124, 125
F_POWER, F_ACC, F_APPEAL, F_JAM, F_RIBBONS, F_PAGENO, F_JPAGE, F_CTYPE, F_PROMPT, F_PAGETITLE = range(151, 161)
F_CREATURES = 161       # (val) 1 when the game says POKéMON (OPTIONS > CREATURES), 0 MOéMON
# the POKé BALL page's words (the ball, "Current ball" / "In bag: N", COLOR's, DECOR's, the effect, KAI's two help lines)
F_BALL, F_BALLSUB, F_BALLCOLOR, F_BALLDECOR, F_BALLFX, F_BALLHELP0, F_BALLHELP1 = range(162, 169)
SB_EDIT, SB_ASK, SB_IDLE = 1 << 20, 1 << 21, 1 << 30   # (the POKé BALL page) a ball being chosen, asked, its prompts
SB_NOCOLOR, SB_NODECOR = 1 << 10, 1 << 11             # ... a ball that takes no colour, no decoration
SM_EV, SM_IV, SM_CHOOSING = 1 << 20, 1 << 21, 1 << 23
SM_LEARN, SM_TWOTYPES, SM_MEMOPLAIN, SM_RENAME, SM_CLEAREV, SM_EGG = (1 << 26, 1 << 27, 1 << 28, 1 << 29, 1 << 30,
                                                                      1 << 31)
R_NEW, R_CANCEL, R_EMPTY = 1 << 27, 1 << 28, 1 << 29   # (a summary move row: the move to learn, CANCEL, none)
F_CHOICE0, F_NCHOICE, F_CUR, F_CURSTR, F_HINT, F_TITLE = 80, 88, 89, 90, 91, 92
S_UP, S_DOWN = 1 << 27, 1 << 28           # (a whole screen's list) more rows above, below
S_PAGE, S_HINT = 1 << 29, 1 << 30         # OPTIONS: a page's title in place of the tabs; a line about the row
O_ALL = 1 << 31                           # an OPTIONS row shows all its choices (two or three)
O_VALUE = 1 << 27                         # ... it has a value (a row opening a page has none)


def col(c):
    """a colour as an op's C / D field holds it"""
    return rgb555(c) | 0x8000

OP_FMT = '<BBBBIIhhHHHHHHHHHHH'
OP_SIZE = struct.calcsize(OP_FMT)
SPEC_NAME = 24


def rgb555(c):
    r, g, b = c[:3]
    return (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10


class Op:
    def __init__(self, kind, x=0, y=0, a=0, b=0, c=0, d=0, e=0, font=0, ink=None, shadow=None, outline=None,
                 align=0, w=0, h=0, n=0, reg=0, cond=0, unless=0, rel=False, rely=False, evb=0):
        self.kind, self.x, self.y, self.a, self.b, self.c, self.d, self.e = kind, x, y, a, b, c, d, e
        self.font, self.ink, self.shadow, self.outline = font, ink, shadow, outline
        self.align, self.w, self.h, self.n, self.reg = align, w, h, n, reg
        self.cond, self.unless, self.rel, self.rely, self.evb = cond, unless, rel, rely, evb

    def pack(self):
        flags = (1 if self.rel else 0) | (self.reg & 7) << 1 | (0x10 if self.rely else 0) | (self.evb & 7) << 5
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


def spec_variant(pack, spec, name, ground_name, ground):
    """a copy of SPEC under NAME with its first picture (its ground) GROUND: a screen's other layout drawn the same
    way (the party's Double / Triple Battle screens: "party2", "party3")"""
    import copy
    v = Spec(name, spec.w, spec.h, spec.canvas)
    v.ops = [copy.copy(o) for o in spec.ops]
    for o in v.ops:
        if o.kind == OP_IMG:
            o.a = pack.image(ground_name, ground)
            break
    pack.spec(v)
    return v


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
            bb = im.getbbox() or (0, 0, 1, 1)
            dx, dy = bb[0], bb[1]
            im = im.crop(bb)          # (only what's drawn is kept: the game draws it from (dx, dy))
            w, h = im.size
            p = im.load()
            alpha = any(0 < p[x, y][3] < 255 for y in range(h) for x in range(w))
            out += struct.pack('<HHhhBBH', w, h, dx, dy, 1 if alpha else 0, 0, 0)
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
def glyph_draw(dst, f, x, y, code, ink, shadow, outline, clip=None, evb=0):
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
            if clip is not None and Y >= clip:
                continue
            if v and c is not None and 0 <= X < dst.width and 0 <= Y < dst.height:
                if evb:     # (blended as the style's window is: its colour plus EVB / 16 of what's under it)
                    u = p[X, Y]
                    p[X, Y] = tuple(min(248, c[k] + (u[k] * evb >> 4) & 0xF8) for k in range(3)) + (255,)
                else:
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


def flow(f, b, maxw):
    """the last page of B in lines (src/mp/mp_uistyle.c FlowPage): its own lines, any wider than MAXW broken at the
    last space that fits"""
    page = b.split(bytes([0xFB]))[-1]          # (CHAR_PROMPT_CLEAR)
    out = []
    for line in page.replace(bytes([0xFA]), bytes([NEWLINE])).split(bytes([NEWLINE])):
        cur = bytearray()
        for c in line:
            cur.append(c)
            if maxw and width(f, cur) > maxw:
                sp = cur.rfind(0x00)
                if sp > 0:
                    out.append(bytes(cur[:sp]))
                    cur = cur[sp + 1:]
        out.append(bytes(cur))
    return out


def render(pack, sp, data, flags, dst=None, at=(0, 0), text_only=False):
    """SP drawn as the game draws it: DATA {field: bytes, a number or a fraction}, FLAGS its condition bits; onto DST
    at AT (a new picture when DST is None). Returns the picture"""
    if dst is None:
        dst = Image.new('RGBA', (sp.w, sp.h), (0, 0, 0, 0))
    canvas = Image.new('RGBA', sp.canvas, (0, 0, 0, 0)) if sp.canvas else dst
    ox, oy = (0, 0) if sp.canvas else at
    regs = [0] * 8
    texts = {k: (v if isinstance(v, (bytes, bytearray)) else encode(str(v))) for k, v in data.items()
             if not isinstance(v, float) and k != SNAP}
    rowmap, place, backing = None, (0, 0), None
    for op in sp.ops:
        if (flags & op.cond) != op.cond or (flags & op.unless):
            continue
        if text_only and op.kind not in (OP_TEXT, OP_LONG, OP_CHOICES):
            continue
        if op.kind == OP_LIST:
            continue
        if op.kind == OP_SNAP:
            if data.get(SNAP) is not None:
                canvas.paste(data[SNAP].convert('RGBA'), (0, 0))
            continue
        if op.kind == OP_LONG:
            f = pack.fonts[op.font]
            if width(f, texts.get(op.a, b'')) > op.w:
                flags |= C_LONG
            continue
        x = op.x + (regs[op.reg] if op.rel else 0) + ox
        y = op.y + (regs[(op.reg + 1) & 7] if op.rely else 0) + oy
        if op.kind == OP_IMG:
            _paste(canvas, pack.images[op.a], x, y)
        elif op.kind == OP_IMGSEL:
            v = data.get(op.b, 0)
            if isinstance(v, str):      # (a type's name: its number in KAI's TYPE_* order)
                v = TYPE_NAMES.index(v) if v in TYPE_NAMES else 0
            _paste(canvas, pack.images[op.a + int(v)], x, y)
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
            if op.align == 3:     # (centred on the Game Boy's 8-pixel grid)
                x0 = int((x - wd // 2) / 8) * 8
            for code in s:
                x0 += glyph_draw(canvas, f, x0, y, code, op.ink, op.shadow, op.outline,
                                 clip=y + op.h if op.h else None, evb=op.evb)
            regs[op.reg] = x0 - ox
        elif op.kind == OP_MSG:
            f = pack.fonts[op.font]
            lines = flow(f, texts.get(F_TEXT0 + op.a, b''), op.w)
            if len(lines) > op.n and op.e and not op.b:
                lines2 = flow(pack.fonts[op.e - 1], texts.get(F_TEXT0 + op.a, b''), op.w)
                if len(lines2) <= op.n:
                    f, lines = pack.fonts[op.e - 1], lines2
            if len(lines) <= op.n or not op.b:
                rows, step, y0 = op.n, op.h, y
            else:
                rows, step, y0 = op.c or op.n, op.b, op.d + oy
            for i, line in enumerate(lines[-rows:] if rows else []):
                x0 = x - (width(f, line) if op.align == 1 else 0)
                for code in line:
                    x0 += glyph_draw(canvas, f, x0, y0 + i * step, code, op.ink, op.shadow, op.outline, evb=op.evb)
                regs[op.reg], regs[(op.reg + 1) & 7] = x0 - ox, y0 + i * step - oy
        elif op.kind == OP_BAR:
            _bar(canvas, pack, op, data.get(op.a, 0.0), x, y)
        elif op.kind == OP_CHOICES:
            _choices(canvas, pack, op, data, x, y)
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


def _choices(dst, pack, op, data, x, y):
    """OP_CHOICES as src/mp/mp_uistyle.c Choices draws it"""
    def c555(v):
        return None if not v else ((v & 31) << 3, (v >> 5 & 31) << 3, (v >> 10 & 31) << 3)
    n, cur = int(data.get(F_NCHOICE, 0)), int(data.get(F_CUR, 0))
    if n <= 0:
        return
    f = pack.fonts[op.font]
    names = [encode(data[op.a + i]) if isinstance(data.get(op.a + i), str) else data.get(op.a + i, b'')
             for i in range(n)]
    total = sum(width(f, t) for t in names) + op.n * (n - 1)
    mark = op.b

    def line(t, x0, ink, sh):
        for code in t:
            x0 += glyph_draw(dst, f, x0, y, code, ink, sh, op.outline)
        return x0
    if total > op.w or n == 1:
        t = names[cur] if 0 <= cur < n else None
        if t is None:
            return
        if op.e and width(f, t) > op.w:
            f = pack.fonts[op.e - 1]
        at = x + op.w - width(f, t) if op.align == 1 else x
        if mark:
            glyph_draw(dst, f, at - f.glyphs[mark][0], y, mark, op.ink, op.shadow, op.outline)
        line(t, at, op.ink, op.shadow)
        return
    at = x + op.w - total if op.align == 1 else x
    for i, t in enumerate(names):
        on = i == cur
        if on and mark:
            glyph_draw(dst, f, at - f.glyphs[mark][0], y, mark, op.ink, op.shadow, op.outline)
        at = line(t, at, op.ink if on else c555(op.c), op.shadow if on else c555(op.d)) + op.n


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
    green = pack.images[op.b]
    ncols = len([cx for cx in range(green.width) if any(green.getpixel((cx, cy))[3] for cy in range(green.height))])
    n = min(ncols, int(frac * ncols + 0.5))
    if frac > 0 and n == 0:
        n = 1
    tier = op.b if n > ncols * 50 // 100 else op.c if n > ncols * 20 // 100 else op.d
    im = pack.images[tier]
    cols = [cx for cx in range(im.width) if any(im.getpixel((cx, cy))[3] for cy in range(im.height))]
    if op.e:
        cols = cols[::-1]
    keep = set(cols[:n])
    part = Image.new('RGBA', im.size, (0, 0, 0, 0))
    q, p = im.load(), part.load()
    for cx in keep:
        for cy in range(im.height):
            p[cx, cy] = q[cx, cy]
    _paste(dst, part, x, y)


def grey(im):
    """IM's colours as greys of the same lightness (a held mark beside the chosen one), its alpha kept"""
    out = im.convert('RGBA').copy()
    q = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = q[x, y]
            v = (r * 30 + g * 59 + b * 11) // 100
            q[x, y] = (v, v, v, a)
    return out
