#!/usr/bin/env python3
"""ORRE's words: every line of Pokemon Colosseum's text in Kai's own character set, as orre/orre_text.pak beside the
other packs (docs/ORRE_HANDOFF.md step 2). Like them it is made from the disc (tools/orre/make_pack.sh) and never
committed. The game reads it when an ORRE line is first shown (src/orre/orre_text.c); a script names a line by
Colosseum's own message id through a placeholder (orre_text_bytes()).

    python3 -I tools/orre/build_text.py UNPACKED_DIR OUT.pak [--dump FILE.txt]

Colosseum's message ids are one numbering across its files (6,726 ids; the same id in several files is the same line,
copied into each module's archive), so the pack is one table by id: every .msg file of the disc plus common_rel's own
block (place, item, move and POKeMON names: colo_data.common_text). Where copies differ (eight menu lines, the move
viewer's), the first file in FILE_ORDER's order wins.

A line, converted:
- MOeMON's words (tools/moemon_text/moe_words.py, as HOENN's and SINNOH's text); "‱" (the font's "???", a speaker
  not yet named) is "???"; "$" Kai's money sign; straight quotes Kai's curly ones; the P★DA's star a "·" (Kai's font
  has no star); any other character Kai's font hasn't got is "?".
- Re-wrapped to Kai's message box (WIDTH pixels of the normal font, gFontNormalLatinGlyphWidths): Colosseum's box is
  wider and holds four lines. Its {wait_key} starts a new box (Kai's page); within a box, a line Colosseum ends on a
  sentence's end stays a break, the others flow together; then two lines a box, the box scrolling for each line after.
- {hero} is the player's name; {rui} RUI. Every other word the game fills in ({npc} the speaker's name, {item},
  {pokemon}, {digit}, {money}...) is ORRE's own string slot, the placeholder 0x80 + Colosseum's control id
  (src/orre/orre_text.c gOrreStrVar: what the scripts put there, as Colosseum's scripts do).
- {wait:N} is Kai's PAUSE N; the rest (fonts, colours, sounds, ruby) isn't Kai's message box's and goes.
- Strings in Japanese (cut events the US release never shows) aren't kept: their ids say "Message missing".

File: "O3TZ", u32 the size of the pack, the pack deflated (zlib). Pack (little endian): "ORTX" u32 version 1, u32
lines, then (u32 id, u32 offset from the pack's start) per line sorted by id, the lines EOS terminated.
"""
import os
import re
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(ROOT, 'tools', 'moemon_text'))
import msg  # noqa: E402
from moe_words import moe  # noqa: E402

VERSION = 1
EOS, NEWLINE, SCROLL, PAGE, PLACEHOLDER_BEGIN, EXT_CTRL = 0xFF, 0xFE, 0xFA, 0xFB, 0xFD, 0xFC
PAUSE = 0x08
PLACEHOLDER_ID_PLAYER = 0x01
PLACEHOLDER_ID_ORRE_TEXT = 0x44          # include/constants/characters.h
PLACEHOLDER_ID_ORRE_VAR = 0x80           # + Colosseum's control id: ORRE's string slots (gOrreStrVar)
WIDTH = 200                              # pixels a line of the field message box holds (216, a margin kept)
RUI = 'RUI'

# Files first, the developers' move viewer last (its copies of the battle lines are older)
LAST_FILES = ('waza_viewer/',)

# What a slot's words are likely to take, in pixels, for the wrapping (a name about 7 letters, a POKeMON's 10...)
SLOT_WIDTH = {'npc': 48, 'item': 66, 'item2': 66, 'item_name': 66, 'pokemon': 60, 'pokemon2': 60, 'pokemon_id': 60,
              'move': 66, 'move_name': 66, 'digit': 18, 'digit2': 18, 'money': 30, 'menu_digit': 18,
              'menu_udigit': 18, 'menu_money': 30, 'time': 30}
SLOT_DEFAULT = 54
PLAYER_WIDTH = 42

# Controls with no words of their own that Kai's box has no use for
DROPPED = {'font', 'color', 'palette', 'shadow', 'align', 'talk_se', 'line_space', 'baseline', 'se_off', 'se_on',
           'snd_play', 'snd_wait', 'ruby', 'ruby_top', 'ruby_end', 'indent_off', 'ctl10', 'ctl11', 'ctl12', 'ctl58',
           'ctl59', 'ctl60', 'ctl63', 'ctl64'}
SENTENCE_END = tuple('.!?…)”"~:')
SUBSTITUTE = {'‱': '???', '‥': '..', '−': '-', '‘': "'", '’': "'",
              '$': '¥',     # (Kai's money sign; its '$' is the end of a string)
              '★': '·'}     # (the P★DA: Kai's font has no star)


def charmap():
    out = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line.rstrip('\n'))
        if m and m.group(1) not in out:
            out[m.group(1)] = int(m.group(2), 16)
    out["'"] = out.get('’', out.get("'", 0xB4))
    return out


def letter_widths(cmap):
    src = open(os.path.join(ROOT, 'src', 'fonts.c')).read()
    m = re.search(r'gFontNormalLatinGlyphWidths\[\] = \{(.*?)\};', src, re.S)
    widths = [int(x) for x in re.findall(r'\d+', m.group(1))]
    return {ch: widths[code] for ch, code in cmap.items() if code < len(widths)}


def control_ids():
    return {name: cid for cid, (name, _) in msg.CONTROLS.items()}


CTL_ID = control_ids()


def japanese(parts):
    # (only what's shown counts: a line's padding after its {end}, wide spaces, isn't Japanese: 20227 "RUI became a
    # partner!" was dropped for it)
    for p in parts:
        if isinstance(p, tuple) and p[0] == 'end':
            break
        if isinstance(p, str) and any(ord(c) > 0x3000 for c in p):
            return True
    return False


def orre_text_bytes(msg_id):
    """The placeholder a script's text is: Colosseum's message id as three 7-bit bytes (never EOS), then EOS"""
    return bytes([PLACEHOLDER_BEGIN, PLACEHOLDER_ID_ORRE_TEXT, (msg_id >> 14) & 0x7F, (msg_id >> 7) & 0x7F,
                  msg_id & 0x7F, EOS])


class Conv:
    def __init__(self):
        self.cmap = charmap()
        self.widths = letter_widths(self.cmap)
        self.question = self.cmap.get('?', 0xAC)
        self.unmapped = {}

    # A line's pieces: ('t', text) | ('p', bytes, width) | ('br',) | ('page',) | ('pause', n)
    def pieces(self, parts):
        out = []
        for p in parts:
            if isinstance(p, str):
                t = ''.join(SUBSTITUTE.get(c, c) for c in p)
                while '"' in t:   # (straight quotes as Kai's opening and closing ones: it has no straight double quote)
                    t = t.replace('"', '“' if self.quote_open else '”', 1)
                    self.quote_open = not self.quote_open
                out.append(('t', moe(t)))
                continue
            name, prm = p
            if name == 'br':
                out.append(('br',))
            elif name == 'wait_key':
                out.append(('page',))
            elif name == 'end':
                break
            elif name == 'wait':
                out.append(('pause', prm[0] if prm else 0))
            elif name == 'hero':
                out.append(('p', bytes([PLACEHOLDER_BEGIN, PLACEHOLDER_ID_PLAYER]), PLAYER_WIDTH))
            elif name == 'rui':
                out.append(('t', RUI))
            elif name in DROPPED:
                continue
            else:
                out.append(('p', bytes([PLACEHOLDER_BEGIN, PLACEHOLDER_ID_ORRE_VAR + CTL_ID[name]]),
                            SLOT_WIDTH.get(name, SLOT_DEFAULT)))
        return out

    def encode_text(self, t):
        b = bytearray()
        for c in t:
            code = self.cmap.get(c)
            if code is None:
                self.unmapped[c] = self.unmapped.get(c, 0) + 1
                code = self.question
            b.append(code)
        return bytes(b)

    def text_width(self, t):
        return sum(self.widths.get(c, 6) for c in t)

    def convert(self, parts):
        """-> the line's bytes (EOS terminated), or None (Japanese)"""
        if japanese(parts):
            return None
        self.quote_open = True
        # pages of Colosseum lines; a Colosseum line is a list of items: ('t', text) / ('p', bytes, w) / ('pause', n)
        pages, line = [[]], []
        for pc in self.pieces(parts):
            if pc[0] == 'br':
                pages[-1].append(line)
                line = []
            elif pc[0] == 'page':
                pages[-1].append(line)
                line = []
                pages.append([])
            else:
                line.append(pc)
        pages[-1].append(line)
        pages = [[ln for ln in pg if ln] for pg in pages]
        pages = [pg for pg in pages if pg]
        out = bytearray()
        for k, pg in enumerate(pages):
            if k:
                out.append(PAGE)
            lines = self.wrap_page(pg)
            for i, ln in enumerate(lines):
                if i == 1:
                    out.append(NEWLINE)
                elif i > 1:
                    out.append(SCROLL)
                out += ln
        return bytes(out) + bytes([EOS])

    def words(self, segment):
        """A segment's words: lists of items, split at spaces only (a placeholder stays glued to its 's)"""
        words, cur = [], []
        for it in segment:
            if it[0] != 't':
                cur.append(it)
                continue
            chunks = it[1].split(' ')
            for j, ch in enumerate(chunks):
                if j:
                    if cur:
                        words.append(cur)
                    cur = []
                if ch:
                    cur.append(('t', ch))
        if cur:
            words.append(cur)
        return words

    def word_width(self, w):
        return sum(self.text_width(it[1]) if it[0] == 't' else (it[2] if it[0] == 'p' else 0) for it in w)

    def word_bytes(self, w):
        b = bytearray()
        for it in w:
            if it[0] == 't':
                b += self.encode_text(it[1])
            elif it[0] == 'p':
                b += it[1]
            elif it[0] == 'pause':
                b += bytes([EXT_CTRL, PAUSE, it[1]])
        return bytes(b)

    def wrap_page(self, pg):
        """A box's Colosseum lines -> Kai's lines (bytes): lines ending a sentence stay breaks, the rest flow"""
        segments, cur = [], []
        for ln in pg:
            if cur:
                cur.append(('t', ' '))
            cur += ln
            last = next((it for it in reversed(ln) if it[0] != 'pause'), None)
            if last is not None and last[0] == 't' and last[1].rstrip().endswith(SENTENCE_END):
                segments.append(cur)
                cur = []
        if cur:
            segments.append(cur)
        space = self.text_width(' ')
        lines = []
        for seg in segments:
            cur_w, cur_b = 0, bytearray()
            for w in self.words(seg):
                ww, wb = self.word_width(w), self.word_bytes(w)
                if cur_b and cur_w + space + ww > WIDTH:
                    lines.append(bytes(cur_b))
                    cur_w, cur_b = 0, bytearray()
                if cur_b:
                    cur_b += self.encode_text(' ')
                    cur_w += space
                cur_b += wb
                cur_w += ww
            if cur_b:
                lines.append(bytes(cur_b))
        return lines


def file_order(unpacked):
    files = []
    for d in sorted(os.listdir(unpacked)):
        p = os.path.join(unpacked, d)
        if os.path.isdir(p):
            files += [os.path.join(d, f) for f in sorted(os.listdir(p)) if f.endswith('.msg')]
    return sorted(files, key=lambda f: (f.startswith(LAST_FILES), f))


def collect(unpacked):
    """{id: parts}: common_rel's block, then every .msg file (the first copy of an id wins)"""
    import colo_data
    lines = {}
    try:
        for k, parts in colo_data.common_text(unpacked).items():
            lines.setdefault(k, parts)
    except (OSError, ValueError, KeyError) as e:
        print('build_text.py: no common text (%s)' % e, file=sys.stderr)
    for f in file_order(unpacked):
        for k, parts in msg.parse(open(os.path.join(unpacked, f), 'rb').read()).items():
            lines.setdefault(k, parts)
    return lines


def build(unpacked):
    conv = Conv()
    out = {}
    for k, parts in sorted(collect(unpacked).items()):
        b = conv.convert(parts)
        if b is not None:
            out[k] = b
    return out, conv


def write(lines, path):
    head = bytearray(b'ORTX' + struct.pack('<II', VERSION, len(lines)))
    table_at = len(head)
    head += b'\0' * 8 * len(lines)
    body = bytearray()
    base = len(head)
    for i, (k, b) in enumerate(sorted(lines.items())):
        struct.pack_into('<II', head, table_at + 8 * i, k, base + len(body))
        body += b
    pack = bytes(head + body)
    with open(path, 'wb') as f:
        f.write(b'O3TZ' + struct.pack('<I', len(pack)) + zlib.compress(pack, 9))


def show(b, cmap):
    """A converted line as text again (for --dump and the tests)"""
    rev = {}
    for ch, code in cmap.items():
        rev.setdefault(code, ch)
    s, i = [], 0
    while i < len(b) and b[i] != EOS:
        c = b[i]
        if c == NEWLINE:
            s.append('\\n')
        elif c == SCROLL:
            s.append('\\l')
        elif c == PAGE:
            s.append('\\p')
        elif c == PLACEHOLDER_BEGIN:
            i += 1
            pid = b[i]
            s.append('{PLAYER}' if pid == PLACEHOLDER_ID_PLAYER else
                     '{%s}' % msg.CONTROLS[pid - PLACEHOLDER_ID_ORRE_VAR][0].upper() if pid >= PLACEHOLDER_ID_ORRE_VAR
                     else '{ph%d}' % pid)
        elif c == EXT_CTRL:
            s.append('{pause %d}' % b[i + 2])
            i += 2
        else:
            s.append(rev.get(c, '?'))
        i += 1
    return ''.join(s)


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    unpacked, out_path = sys.argv[1], sys.argv[2]
    lines, conv = build(unpacked)
    write(lines, out_path)
    print('%s: %d lines' % (out_path, len(lines)))
    if conv.unmapped:
        print('characters Kai has no glyph for (shown as ?): %s' % ' '.join(
            '%s x%d' % (c, n) for c, n in sorted(conv.unmapped.items(), key=lambda x: -x[1])), file=sys.stderr)
    if '--dump' in sys.argv:
        with open(sys.argv[sys.argv.index('--dump') + 1], 'w', encoding='utf-8') as f:
            for k, b in sorted(lines.items()):
                f.write('%d: %s\n' % (k, show(b, conv.cmap)))


if __name__ == '__main__':
    main()
