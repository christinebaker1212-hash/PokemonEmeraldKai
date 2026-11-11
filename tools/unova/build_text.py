#!/usr/bin/env python3
"""UNOVA's words: Black's own message banks (tools/unova/text.py decrypts them from the user's ROM) in the game's
character set, as unova/black_text.pak beside the game (docs/UNOVA_HANDOFF.md). Like the 3D pack it is made when the
release is built and shipped beside the game; nothing of it is in this repository. The game reads it when an UNOVA
text is first shown (src/unova/u3d_text.c); scripts point at a message by bank and number through a placeholder
(unova_text_bytes()).

    python3 -I tools/unova/build_text.py OUT.pak

Banks: the system banks (a/0/0/2) first, then the story banks (a/0/0/3: a zone's messages): bank_number(). A message:
MOéMON's words (tools/moemon_text/moe_words.py, as HOENN's and SINNOH's text), Black's breaks as Emerald's: a new line,
{BE00} (wait, then the next line scrolls up) "\\l", {BE01} (wait, then a new box) "\\p", a box ending in {BE01} waits for
A at its end; {BE02:n} / {BE09:n} wait n frames; a word slot ({01xx:n} / {02xx:n}: a name, a number...) slot n: 0-2 the
game's STR_VAR_1-3, 3 and up UNOVA's own (gUnovaStrVar); colours, sounds and positions are left out; a character the
game has no glyph for is a '?'.

File: "U3TZ", u32 the size of the pack, the pack deflated (zlib). Pack (little endian): "U3DT" u32 version 1, u32 banks,
u32 offset of each bank; a bank: u32 messages, u32 offset of each (from the pack's start; 0: none), the messages EOS
terminated.
"""
import importlib.util
import os
import re
import struct
import sys
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(os.path.dirname(HERE), 'moemon_text'))
import narc  # noqa: E402
import text as tx  # noqa: E402
from moe_words import moe  # noqa: E402

_spec = importlib.util.spec_from_file_location('sinnoh_build_text', os.path.join(os.path.dirname(HERE), 'sinnoh',
                                                                                 'build_text.py'))
_sinnoh = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_sinnoh)
charmap = _sinnoh.charmap   # (the game's own charmap.txt, read as a tool)

VERSION = 1
EOS, NEWLINE, SCROLL, PAGE = 0xFF, 0xFE, 0xFA, 0xFB
PAUSE_UNTIL_PRESS = (0xFC, 0x09)
PAUSE = 0x08                      # (0xFC 0x08 n: wait n frames)
PLACEHOLDER_BEGIN = 0xFD
STR_VAR = (0x02, 0x03, 0x04)      # (Black's slots 0-2)
PLACEHOLDER_ID_UNOVA_TEXT = 0x45  # include/constants/characters.h
PLACEHOLDER_ID_UNOVA_VAR = 0x50   # + slot (3-31): gUnovaStrVar
SLOTS = 32
_CTRL = re.compile(r'\{([0-9A-F]{4}):([0-9A-F,]*)\}')


def system_banks():
    return len(narc.arc(tx.SYSTEM))


def bank_number(arc, n):
    """The pack's bank of Black's message file N of ARC (tx.SYSTEM or tx.STORY)"""
    return n if arc == tx.SYSTEM else system_banks() + n


def unova_text_bytes(bank, msg):
    """The placeholder a script's text is: bank and message as two 7-bit bytes each (never EOS), then EOS"""
    return [PLACEHOLDER_BEGIN, PLACEHOLDER_ID_UNOVA_TEXT, (bank >> 7) & 0x7F, bank & 0x7F, (msg >> 7) & 0x7F,
            msg & 0x7F, EOS]


def slot_bytes(slot):
    if slot < len(STR_VAR):
        return [PLACEHOLDER_BEGIN, STR_VAR[slot]]
    return [PLACEHOLDER_BEGIN, PLACEHOLDER_ID_UNOVA_VAR + min(slot, SLOTS - 1)]


def convert(line, cmap, wait_at_end=True):
    """WAIT_AT_END: a message ending in {BE01} waits for A there (a script's message then its close); battle lines
    don't (the battle waits itself)"""
    out = []
    i = 0
    unknown = cmap.get('?', 0xAC)
    while i < len(line):
        m = _CTRL.match(line, i)
        if m:
            kind, args = m.group(1), [int(a, 16) for a in m.group(2).split(',') if a]
            i = m.end()
            if kind in ('BE00', 'BE01'):
                out.append(SCROLL if kind == 'BE00' else PAGE)
                if i < len(line) and line[i] == '\n':
                    i += 1   # (the break that follows is the box's own)
            elif kind in ('BE02', 'BE09') and args:
                out += [0xFC, PAUSE, min(args[0], 0xFF)]
            elif kind[:2] in ('01', '02') and args:
                out += slot_bytes(args[0])
            continue
        ch = line[i]
        i += 1
        if ch == '\n':
            out.append(NEWLINE)
        elif ch == '　':
            out.append(cmap.get(' ', 0x00))
        elif ch == '"':
            out.append(cmap.get('”', unknown))   # (Black closes its quotes with a straight one)
        else:
            out.append(cmap.get(ch, unknown))
    wait = bool(out) and out[-1] in (SCROLL, PAGE)
    while out and out[-1] in (NEWLINE, SCROLL, PAGE):
        out.pop()
    if wait and wait_at_end:
        out += PAUSE_UNTIL_PRESS
    return bytes(out) + bytes([EOS])


def main():
    out_path = sys.argv[1]
    cmap = charmap()
    banks = []
    for arc in (tx.SYSTEM, tx.STORY):
        for f in narc.arc(arc):
            banks.append([convert(moe(line), cmap) for line in tx.bank(f)])
    head = bytearray(b'U3DT' + struct.pack('<II', VERSION, len(banks)))
    table_at = len(head)
    head += b'\0' * 4 * len(banks)
    body = bytearray()
    base = len(head)
    offsets = []
    for msgs in banks:
        offsets.append(base + len(body))
        start = len(body)
        body += struct.pack('<I', len(msgs)) + b'\0' * 4 * len(msgs)
        for k, m in enumerate(msgs):
            struct.pack_into('<I', body, start + 4 + 4 * k, base + len(body))
            body += m
    for k, off in enumerate(offsets):
        struct.pack_into('<I', head, table_at + 4 * k, off)
    pack = bytes(head + body)
    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    open(out_path, 'wb').write(b'U3TZ' + struct.pack('<I', len(pack)) + zlib.compress(pack, 6))
    print('%s: %d banks, %d messages' % (out_path, len(banks), sum(len(b) for b in banks)))


if __name__ == '__main__':
    main()
