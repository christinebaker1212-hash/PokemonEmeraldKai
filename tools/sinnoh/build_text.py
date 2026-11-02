#!/usr/bin/env python3
"""SINNOH's words: Platinum's message banks (pret/pokeplatinum res/text/*.json, English) in the game's own character set,
as sinnoh/platinum_text.pak beside the game (docs/SINNOH_HANDOFF.md step 4). Like the 3D pack, it is made from the
decomp when the release is built and shipped beside the game; nothing of it is in this repository. The game reads it
when a SINNOH text is first shown (src/sinnoh/s3d_text.c), and scripts point at a message by bank and number through a
placeholder (sinnoh_text_bytes()).

    python3 -I tools/sinnoh/build_text.py POKEPLATINUM_DIR OUT.pak

Banks are numbered by their file names in order (bank_numbers()): tools/sinnoh/gen_maps.py and the scripts use the same
numbers from the same pinned commit. A message: MOéMON's words (tools/moemon_text/moe_words.py, as HOENN's text), Platinum's line breaks become Emerald's ("\\n" a new line, "\\r" a
scroll, "\\f" a new page), its string buffers 0-2 STR_VAR_1-3, any character the game has no glyph for a '?'.

File: "S3TZ", u32 the size of the pack, the pack deflated (zlib). Pack (little endian): "S3DT" u32 version 1, u32 banks,
u32 offset of each bank; a bank: u32 messages, u32 offset of each (from the pack's start; 0: none), the messages EOS
terminated.
"""
import os
import re
import struct
import sys
import zlib

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'moemon_text'))
from moe_words import moe  # noqa: E402   (HOENN's words: POKéMON -> MOéMON, POKé BALL kept)

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
VERSION = 1
EOS, NEWLINE, SCROLL, PAGE = 0xFF, 0xFE, 0xFA, 0xFB
PLACEHOLDER_BEGIN, STR_VAR = 0xFD, (0x02, 0x03, 0x04)
PLACEHOLDER_ID_SINNOH_TEXT = 0x40   # include/constants/characters.h


def bank_numbers(root):
    """{bank file name without .json: number}"""
    names = sorted(f[:-5] for f in os.listdir(os.path.join(root, 'res/text')) if f.endswith('.json'))
    return {n: i for i, n in enumerate(names)}


def bank_of_header_field(field):
    """TEXT_BANK_TWINLEAF_TOWN -> twinleaf_town"""
    return field[len('TEXT_BANK_'):].lower() if field.startswith('TEXT_BANK_') else None


def sinnoh_text_bytes(bank, msg):
    """The placeholder a script's text is: bank and message as two 7-bit bytes each (never EOS), then EOS"""
    return [PLACEHOLDER_BEGIN, PLACEHOLDER_ID_SINNOH_TEXT, (bank >> 7) & 0x7F, bank & 0x7F, (msg >> 7) & 0x7F,
            msg & 0x7F, EOS]


def charmap():
    out = {}
    for line in open(os.path.join(ROOT, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line.rstrip('\n'))
        if m and m.group(1) not in out:
            out[m.group(1)] = int(m.group(2), 16)
    out["'"] = out.get('’', out.get("'", 0xB4))
    return out


def convert(text, cmap):
    out = []
    i = 0
    while i < len(text):
        ch = text[i]
        if ch == '{':
            end = text.find('}', i)
            code = text[i + 1:end] if end > 0 else ''
            i = end + 1 if end > 0 else len(text)
            m = re.match(r'STRVAR_\d+ \d+, (\d+)', code)
            if m and int(m.group(1)) < 3:
                out += [PLACEHOLDER_BEGIN, STR_VAR[int(m.group(1))]]
            continue                     # (colors, sizes and the rest: plain text)
        i += 1
        if ch == '\n':
            out.append(NEWLINE)
        elif ch == '\r':
            out.append(SCROLL)
        elif ch == '\f':
            out.append(PAGE)
        else:
            out.append(cmap.get(ch, cmap.get('?', 0xAC)))
    while out and out[-1] in (NEWLINE, SCROLL, PAGE):   # (no wait at the very end)
        out.pop()
    return bytes(out) + bytes([EOS])


def text(v):
    """A message as one string (the banks keep long ones as a list of lines, breaks included)"""
    if isinstance(v, list):
        return ''.join(x for x in v if isinstance(x, str))
    return v if isinstance(v, str) else None


def main():
    import json
    root, out_path = sys.argv[1], sys.argv[2]
    cmap = charmap()
    numbers = bank_numbers(root)
    banks = [None] * len(numbers)
    for name, n in numbers.items():
        data = json.load(open(os.path.join(root, 'res/text', name + '.json'), encoding='utf-8'))
        msgs = data.get('messages', []) if isinstance(data, dict) else []
        banks[n] = [convert(moe(text(m.get('en_US'))), cmap) if text(m.get('en_US')) is not None else None
                    for m in msgs]
    head = bytearray(b'S3DT' + struct.pack('<II', VERSION, len(banks)))
    table_at = len(head)
    head += b'\0' * 4 * len(banks)
    body = bytearray()
    base = len(head)
    bank_offsets = []
    for msgs in banks:
        bank_offsets.append(base + len(body))
        start = len(body)
        body += struct.pack('<I', len(msgs)) + b'\0' * 4 * len(msgs)
        for k, m in enumerate(msgs):
            if m is None:
                continue
            struct.pack_into('<I', body, start + 4 + 4 * k, base + len(body))
            body += m
    for k, off in enumerate(bank_offsets):
        struct.pack_into('<I', head, table_at + 4 * k, off)
    pack = bytes(head + body)
    open(out_path, 'wb').write(b'S3TZ' + struct.pack('<I', len(pack)) + zlib.compress(pack, 6))
    print('%s: %d banks, %d messages' % (out_path, len(banks), sum(len(b) for b in banks)))


if __name__ == '__main__':
    main()
