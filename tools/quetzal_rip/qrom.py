"""ROM access and the table addresses found in Moemon Quetzal (English Alpha 9 v1).

Every address here was found by hand (see docs/quetzal/ROM_MAP.md for how).
Offsets are file offsets (ROM address - 0x08000000).
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))

# ---- known tables -------------------------------------------------------
MAP_GROUPS = 0x93ADCC          # gMapGroups, 40 groups
NUM_GROUPS = 40
MAPSEC_MAIN = 0x1232A00        # region map entries {x,y,w,h,u8 pad?, names*}; names* -> 4 name ptrs (EN, ES, ES, short)
MAPSEC_JOHTO = 0x1233C94       # same format, used while in Johto (map groups 34-35)
SCRIPT_CMDS = 0x2BE9D0         # gScriptCmdTable, 246 entries (0x00-0xF5)
SCRIPT_CMDS_END = 0x2BEDA8
SPECIAL_VARS = 0x2BEDAC        # gSpecialVars (0x8000-0x8015)
SPECIALS = 0x2BEE04            # gSpecials
OBJ_GFX_INFO = 0x10EA9E8       # gObjectEventGraphicsInfoPointers, 20000 entries (u16 graphics ids)
SPECIES_NAMES = 0x516B48       # 13 bytes each
MOVE_NAMES = 0x520692          # 17 bytes each
ITEM_NAMES = 0x11EECE8         # 20 bytes each, English; 632-681 are empty; Spanish names follow at 890
NUM_ITEMS = 890

JOHTO_GROUPS = (34, 35)
KANTO_GROUPS = (36, 37, 38, 39)


def find_rom():
    for p in sys.argv[1:]:
        if p.endswith('.gba') and os.path.exists(p):
            return p
    p = os.environ.get('QUETZAL_ROM')
    if p and os.path.exists(p):
        return p
    raise SystemExit('give the Moemon Quetzal ROM path (argument or QUETZAL_ROM)')


class Rom:
    def __init__(self, path):
        self.d = open(path, 'rb').read()
        self.n = len(self.d)

    def u8(self, o): return self.d[o]
    def s8(self, o): return struct.unpack_from('<b', self.d, o)[0]
    def u16(self, o): return struct.unpack_from('<H', self.d, o)[0]
    def s16(self, o): return struct.unpack_from('<h', self.d, o)[0]
    def u32(self, o): return struct.unpack_from('<I', self.d, o)[0]
    def s32(self, o): return struct.unpack_from('<i', self.d, o)[0]

    def isptr(self, v): return 0x08000000 <= v < 0x08000000 + self.n
    def ptr(self, o):
        """Read a ROM pointer at o and return it as a file offset (None if 0 / not a ROM pointer)."""
        v = self.u32(o)
        return v - 0x08000000 if self.isptr(v) else None


# ---- text ----------------------------------------------------------------
def _load_charmap():
    single = {}
    multi = []   # (bytes, name)
    path = os.path.join(REPO, 'charmap.txt')
    for line in open(path, encoding='utf-8'):
        line = line.split('@')[0].rstrip()
        m = re.match(r"^'(.+)'\s*=\s*([0-9A-Fa-f ]+)$", line)
        if m:
            ch = m.group(1)
            if ch == "\\'":
                ch = "'"
            bs = bytes.fromhex(m.group(2))
            if len(bs) == 1:
                single.setdefault(bs[0], ch)
            continue
        m = re.match(r'^(\w+)\s*=\s*([0-9A-Fa-f ]+)$', line)
        if m:
            multi.append((bytes.fromhex(m.group(2)), m.group(1)))
    return single, multi


CHARS, MULTI = _load_charmap()
CHARS[0xFE] = '\\n'
CHARS[0xFA] = '\\l'
CHARS[0xFB] = '\\p'
CHARS[0x00] = ' '
for k, v in list(CHARS.items()):
    if v == '"':
        CHARS[k] = '\\"'
# names for FD xx placeholders and F8/F9 symbols
_FD, _F8, _F9 = {}, {}, {}
for _b, _n in MULTI:      # first definition wins: charmap.txt lists PLAYER before B_BUFF2 / MUS_RG_VS_TRAINER
    if len(_b) == 2 and _b[0] in (0xFD, 0xF8, 0xF9):
        {0xFD: _FD, 0xF8: _F8, 0xF9: _F9}[_b[0]].setdefault(_b[1], _n)
_COLORS = {0: 'TRANSPARENT', 1: 'WHITE', 2: 'DARK_GRAY', 3: 'LIGHT_GRAY', 4: 'RED', 5: 'LIGHT_RED',
           6: 'GREEN', 7: 'LIGHT_GREEN', 8: 'BLUE', 9: 'LIGHT_BLUE'}
# FC control codes: name, number of argument bytes
_FC = {0x00: ('NAME_END', 0), 0x01: ('COLOR', 1), 0x02: ('HIGHLIGHT', 1), 0x03: ('SHADOW', 1),
       0x04: ('COLOR_HIGHLIGHT_SHADOW', 3), 0x05: ('PALETTE', 1), 0x06: ('FONT', 1), 0x07: ('RESET_FONT', 0),
       0x08: ('PAUSE', 1), 0x09: ('PAUSE_UNTIL_PRESS', 0), 0x0A: ('WAIT_SE', 0), 0x0B: ('PLAY_BGM', 2),
       0x0C: ('ESCAPE', 1), 0x0D: ('SHIFT_RIGHT', 1), 0x0E: ('SHIFT_DOWN', 1), 0x0F: ('FILL_WINDOW', 0),
       0x10: ('PLAY_SE', 2), 0x11: ('CLEAR', 1), 0x12: ('SKIP', 1), 0x13: ('CLEAR_TO', 1),
       0x14: ('MIN_LETTER_SPACING', 1), 0x15: ('JPN', 0), 0x16: ('ENG', 0), 0x17: ('PAUSE_MUSIC', 0),
       0x18: ('RESUME_MUSIC', 0)}


def decode(rom, o, maxlen=8192):
    """Decode a text string at file offset o. Returns (string in decomp syntax, length incl. terminator)."""
    d = rom.d
    out = []
    i = o
    end = min(rom.n, o + maxlen)
    while i < end:
        c = d[i]
        if c == 0xFF:
            return ''.join(out), i - o + 1
        if c == 0xFD:
            out.append('{%s}' % _FD.get(d[i + 1], 'FD_0x%02X' % d[i + 1]))
            i += 2
            continue
        if c == 0xFC:
            code = d[i + 1]
            name, nargs = _FC.get(code, ('FC_0x%02X' % code, 0))
            args = list(d[i + 2:i + 2 + nargs])
            if name in ('COLOR', 'HIGHLIGHT', 'SHADOW'):
                s = '{%s %s}' % (name, _COLORS.get(args[0], '0x%02X' % args[0]))
            elif name == 'COLOR_HIGHLIGHT_SHADOW':
                s = '{%s %s}' % (name, ' '.join(_COLORS.get(a, '0x%02X' % a) for a in args))
            elif name in ('PLAY_BGM', 'PLAY_SE'):
                s = '{%s 0x%04X}' % (name, args[0] | args[1] << 8)
            elif nargs:
                s = '{%s %s}' % (name, ' '.join('0x%02X' % a for a in args))
            else:
                s = '{%s}' % name
            out.append(s)
            i += 2 + nargs
            continue
        if c == 0xF8:
            out.append('{%s}' % _F8.get(d[i + 1], 'F8_0x%02X' % d[i + 1]))
            i += 2
            continue
        if c == 0xF9:
            out.append('{%s}' % _F9.get(d[i + 1], 'F9_0x%02X' % d[i + 1]))
            i += 2
            continue
        if c == 0xF7:
            out.append('{DYNAMIC 0x%02X}' % d[i + 1])
            i += 2
            continue
        if c == 0x53:      # PK / PKMN glyphs
            if d[i + 1] == 0x54:
                out.append('{PKMN}')
                i += 2
            else:
                out.append('{PK}')
                i += 1
            continue
        ch = CHARS.get(c)
        if ch is None:
            ch = '{0x%02X}' % c
        out.append(ch)
        i += 1
    return ''.join(out), i - o


def looks_like_text(rom, o, minlen=2):
    if o is None or not (0 <= o < rom.n):
        return False
    s, n = decode(rom, o, 2048)
    if n >= 2048 or n < minlen:
        return False
    if rom.d[o + n - 1] != 0xFF:
        return False
    bad = s.count('{0x')
    return bad <= max(1, n // 20)


def plain(rom, o, maxlen=64):
    """Short display text (names): strip control codes and trailing spaces."""
    s, _ = decode(rom, o, maxlen)
    return s.strip()


# ---- name tables ---------------------------------------------------------
def const_name(prefix, name, used):
    s = name.upper()
    s = s.replace('♀', '_F').replace('♂', '_M').replace('É', 'E').replace('é', 'E').replace('’', '')
    s = s.replace("'", '').replace('.', '').replace('-', '_').replace(' ', '_').replace(':', '')
    s = re.sub(r'[^A-Z0-9_]', '', s)
    s = re.sub(r'_+', '_', s).strip('_') or 'NONE'
    base = prefix + s
    out = base
    if out in used:
        out = '%s_%d' % (base, used[base] + 1)
        used[base] += 1
    else:
        used[base] = 1
    return out


def name_table(rom, base, stride, count, blanks=False):
    names = []
    for i in range(count):
        raw = rom.d[base + stride * i: base + stride * (i + 1)]
        if 0xFF not in raw:
            if blanks and not any(raw):
                names.append('')          # an empty slot inside the table
                continue
            break
        names.append(plain(rom, base + stride * i, stride))
    return names


class Names:
    def __init__(self, rom):
        self.species = name_table(rom, SPECIES_NAMES, 13, 3058)
        self.moves = name_table(rom, MOVE_NAMES, 17, 1000)
        self.items = name_table(rom, ITEM_NAMES, 20, NUM_ITEMS, blanks=True)
        self.species_const = self._consts('SPECIES_', self.species, {0: 'SPECIES_NONE'})
        self.move_const = self._consts('MOVE_', self.moves, {0: 'MOVE_NONE'})
        self.item_const = self._consts('ITEM_', self.items, {0: 'ITEM_NONE'})

    @staticmethod
    def _consts(prefix, names, fixed):
        used = {}
        out = []
        for i, n in enumerate(names):
            if i in fixed:
                out.append(fixed[i])
                used[fixed[i]] = 1
            elif not n:
                out.append('%sQZ_%d' % (prefix, i))
            else:
                out.append(const_name(prefix, n, used))
        return out

    def species_name(self, v):
        return self.species_const[v] if 0 <= v < len(self.species_const) else '%d' % v

    def move_name(self, v):
        return self.move_const[v] if 0 <= v < len(self.move_const) else '%d' % v

    def item_name(self, v):
        return self.item_const[v] if 0 <= v < len(self.item_const) else '%d' % v
