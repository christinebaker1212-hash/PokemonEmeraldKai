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
VERSION = 1   # (a message's wait at its end since 2026-11-02: the pack is rebuilt with the game, no reader change)
EOS, NEWLINE, SCROLL, PAGE = 0xFF, 0xFE, 0xFA, 0xFB
PAUSE_UNTIL_PRESS = (0xFC, 0x09)   # (charmap.txt: the box waits for A, its arrow shown)
PLACEHOLDER_BEGIN, STR_VAR = 0xFD, (0x02, 0x03, 0x04, 0x41, 0x42, 0x43)   # (slots 3-5: SINNOH's own, gSinnohStrVar4-6)
PLACEHOLDER_ID_SINNOH_TEXT = 0x40   # include/constants/characters.h


def bank_numbers(root):
    """{bank file name without .json: number}"""
    names = sorted(f[:-5] for f in os.listdir(os.path.join(root, 'res/text')) if f.endswith('.json'))
    return {n: i for i, n in enumerate(names)}


def bank_of_header_field(field):
    """TEXT_BANK_TWINLEAF_TOWN -> twinleaf_town"""
    return field[len('TEXT_BANK_'):].lower() if field.startswith('TEXT_BANK_') else None


# Platinum's trainers' own lines (res/trainers/data/<trainer>.json "messages"): one more bank after the message banks,
# every trainer's file in name order, TRAINER_TYPES messages each (a message the trainer doesn't have is left empty)
TRAINER_TYPES = ['TRMSG_PRE_BATTLE', 'TRMSG_DEFEAT', 'TRMSG_POST_BATTLE', 'TRMSG_REMATCH',
                 'TRMSG_PRE_DOUBLE_BATTLE_1', 'TRMSG_DOUBLE_BATTLE_DEFEAT_1', 'TRMSG_POST_DOUBLE_BATTLE_1',
                 'TRMSG_DOUBLE_BATTLE_NOT_ENOUGH_POKEMON_1', 'TRMSG_PRE_DOUBLE_BATTLE_2', 'TRMSG_DOUBLE_BATTLE_DEFEAT_2',
                 'TRMSG_POST_DOUBLE_BATTLE_2', 'TRMSG_DOUBLE_BATTLE_NOT_ENOUGH_POKEMON_2', 'TRMSG_DOUBLE_BATTLE_REMATCH_1',
                 'TRMSG_DOUBLE_BATTLE_REMATCH_2', 'TRMSG_LAST_BATTLER_HALF_HP', 'TRMSG_LAST_BATTLER', 'TRMSG_FIRST_DAMAGE',
                 'TRMSG_ACTIVE_BATTLER_HALF_HP']


# Platinum's frontier trainers (res/trainers/frontier/data, in generated/frontier_trainers.txt's order): one more bank
# after the trainers', FRONTIER_KINDS entries each: the name, then its lines (src/sinnoh/s3d_tower.c)
FRONTIER_KINDS = ['name', 'TRMSG_PRE_BATTLE', 'TRMSG_WIN', 'TRMSG_DEFEAT']


def frontier_trainers(root):
    """the frontier trainers' file names (youngster_jim...) in Platinum's order, None where a file is missing"""
    out = []
    for line in open(os.path.join(root, 'generated/frontier_trainers.txt'), encoding='utf-8'):
        n = line.strip()
        if n.startswith('FRONTIER_TRAINER_'):
            f = n[len('FRONTIER_TRAINER_'):].lower()
            out.append(f if os.path.exists(os.path.join(root, 'res/trainers/frontier/data', f + '.json')) else None)
    return out


def frontier_bank(root):
    """the frontier trainers' bank number (after the message banks and the trainers')"""
    return len(bank_numbers(root)) + 1


def trainer_files(root):
    d = os.path.join(root, 'res/trainers/data')
    return sorted(f[:-5] for f in os.listdir(d) if f.endswith('.json'))


def trainer_message(root, const, kind):
    """(bank, message) of a Platinum trainer's line (TRAINER_ACE_TRAINER_ABEL, 'TRMSG_DEFEAT'), None if it has none"""
    import json
    files = trainer_files(root)
    name = const[len('TRAINER_'):].lower()
    if name not in files or kind not in TRAINER_TYPES:
        return None
    d = json.load(open(os.path.join(root, 'res/trainers/data', name + '.json'), encoding='utf-8'))
    if not any(m.get('type') == kind for m in d.get('messages', [])):
        return None
    return len(bank_numbers(root)), files.index(name) * len(TRAINER_TYPES) + TRAINER_TYPES.index(kind)


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


def convert(text, cmap, wait_at_end=True):
    """WAIT_AT_END: a message ending in Platinum's "\\r" waits for A there, as Platinum's do (a script's Message then
    CloseMessage); battle lines don't (the battle waits itself)"""
    out = []
    i = 0
    while i < len(text):
        ch = text[i]
        if ch == '{':
            end = text.find('}', i)
            code = text[i + 1:end] if end > 0 else ''
            i = end + 1 if end > 0 else len(text)
            m = re.match(r'STRVAR_\d+ \d+, (\d+)', code)
            if m and int(m.group(1)) < len(STR_VAR):
                out += [PLACEHOLDER_BEGIN, STR_VAR[int(m.group(1))]]
            continue                     # (colors, sizes and the rest: plain text)
        i += 1
        if ch == '\n':
            out.append(NEWLINE)
        elif ch == '\r':
            out.append(PAGE)             # (Platinum's "\r" scrolls both lines away: the next text starts a fresh box, as
                                         # this game's page break; its one-line scroll put a page's second line out of the box)
        elif ch == '\f':
            out.append(PAGE)
        else:
            out.append(cmap.get(ch, cmap.get('?', 0xAC)))
    wait = bool(out) and out[-1] in (SCROLL, PAGE)
    while out and out[-1] in (NEWLINE, SCROLL, PAGE):
        out.pop()
    if wait and wait_at_end:
        out += PAUSE_UNTIL_PRESS
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
    # the trainers' own lines (trainer_message())
    tr = []
    for name in trainer_files(root):
        d = json.load(open(os.path.join(root, 'res/trainers/data', name + '.json'), encoding='utf-8'))
        by = {m.get('type'): m.get('en_US') for m in d.get('messages', [])}
        for kind in TRAINER_TYPES:
            t = text(by.get(kind)) if kind in by else None
            tr.append(convert(moe(t), cmap, wait_at_end=False) if t is not None else None)
    banks.append(tr)
    # the frontier trainers' names and lines (frontier_bank())
    fr = []
    if os.path.isdir(os.path.join(root, 'res/trainers/frontier/data')):
        for name in frontier_trainers(root):
            d = json.load(open(os.path.join(root, 'res/trainers/frontier/data', name + '.json'), encoding='utf-8')) \
                if name else {}
            by = {m.get('type'): m.get('en_US') for m in d.get('messages', [])}
            fr.append(convert(moe(d['name'].upper()), cmap, wait_at_end=False) if d.get('name') else None)
            for kind in FRONTIER_KINDS[1:]:
                t = text(by.get(kind)) if kind in by else None
                fr.append(convert(moe(t), cmap, wait_at_end=False) if t is not None else None)
    banks.append(fr)
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
