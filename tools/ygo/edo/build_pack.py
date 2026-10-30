#!/usr/bin/env python3
"""Builds graphics/ygo/edo.pak: what EDOPro's duel engine (edopro/ocgcore) reads, in one file the game embeds.

  cards.bin        every card in EDOPro's database (BabelCdb cards.cdb): stats, name, text, effect strings
  strings.conf     EDOPro's system strings (Project Ignis Distribution config/strings.conf)
  lflist/*.conf    every Forbidden & Limited list (LFLists)
  *.lua            the card scripts (CardScripts: the root's shared scripts and official/)

Also writes src/ygo/ygo_edo_codes.c: each catalog card's EDOPro code (by password, else by name).

usage: build_pack.py BABELCDB_DIR CARDSCRIPTS_DIR LFLISTS_DIR STRINGS_CONF
       build_pack.py --codes BABELCDB_DIR   (only src/ygo/ygo_edo_codes.c, after a catalog change)
  (git clone https://github.com/ProjectIgnis/BabelCdb, .../CardScripts, .../LFLists;
   strings.conf from https://raw.githubusercontent.com/ProjectIgnis/Distribution/master/config/strings.conf)

Pack format (little-endian): "EDOP", u32 count, u32 chunks, then count entries sorted by name {char name[40];
u32 chunk, offset, size} (offset in the chunk once inflated), then chunks entries {u32 offset, csize, size} (offset
from the start of the pack), then the chunks: files laid end to end in groups of about 256 KB, each group
raw-deflated (zlib wbits -15) on its own, so one script costs one small chunk to read.
cards.bin: "EDOC", u32 count, count records sorted by code {u32 code, alias; u64 setcode; u32 type, level,
attribute; u64 race; i32 atk, def; u32 ot, text} then the text block: per card name, desc, str1..str16, each
0-terminated (text = its offset in the block)."""
import glob, json, os, sqlite3, struct, sys, zlib

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..'))


def deflate(data):
    c = zlib.compressobj(9, zlib.DEFLATED, -15)
    return c.compress(data) + c.flush()


def cards_bin(cdb):
    db = sqlite3.connect(cdb)
    rows = db.execute('select d.id, d.alias, d.setcode, d.type, d.level, d.attribute, d.race, d.atk, d.def, d.ot, '
                      't.name, t.desc, ' + ', '.join('t.str%d' % i for i in range(1, 17)) +
                      ' from datas d join texts t on d.id = t.id order by d.id').fetchall()
    recs, text = [], bytearray()
    for r in rows:
        off = len(text)
        for s in r[10:]:
            text += (s or '').replace('\r', '').encode('utf-8') + b'\0'
        recs.append(struct.pack('<IIQIIIQiiII', r[0], r[1], r[2] & 0xFFFFFFFFFFFFFFFF, r[3], r[4] & 0xFFFFFFFF, r[5],
                                r[6], r[7], r[8], r[9], off))
    return b'EDOC' + struct.pack('<I', len(recs)) + b''.join(recs) + bytes(text), db


def main():
    if len(sys.argv) == 3 and sys.argv[1] == '--codes':
        write_codes(sqlite3.connect(os.path.join(sys.argv[2], 'cards.cdb')))
        return
    if len(sys.argv) != 5:
        sys.exit(__doc__)
    babel, scripts, lflists, strings = sys.argv[1:]
    files = {}
    blob, db = cards_bin(os.path.join(babel, 'cards.cdb'))
    files['cards.bin'] = blob
    files['strings.conf'] = open(strings, 'rb').read()
    for p in sorted(glob.glob(os.path.join(lflists, '*.lflist.conf'))):
        files['lflist/' + os.path.basename(p)] = open(p, 'rb').read()
    for p in sorted(glob.glob(os.path.join(scripts, '*.lua')) + glob.glob(os.path.join(scripts, 'official', '*.lua'))):
        name = os.path.basename(p)
        if name in files:
            continue   # (the root's own wins)
        files[name] = open(p, 'rb').read()
    names = sorted(files)
    assert all(len(n.encode()) < 40 for n in names)
    table, chunks, cur = [], [], bytearray()
    for n in names:
        if len(cur) + len(files[n]) > 256 * 1024 and cur:
            chunks.append(bytes(cur))
            cur = bytearray()
        table.append(n.encode().ljust(40, b'\0') + struct.pack('<III', len(chunks), len(cur), len(files[n])))
        cur += files[n]
    chunks.append(bytes(cur))
    head = b'EDOP' + struct.pack('<II', len(names), len(chunks))
    base = len(head) + 52 * len(names) + 12 * len(chunks)
    ctable, data = [], bytearray()
    for c in chunks:
        z = deflate(c)
        ctable.append(struct.pack('<III', base + len(data), len(z), len(c)))
        data += z
    table += ctable
    out = os.path.join(ROOT, 'graphics', 'ygo', 'edo.pak')
    with open(out, 'wb') as f:
        f.write(head + b''.join(table) + bytes(data))
    print('%s: %d files, %d cards, %.1f MB' % (out, len(names), struct.unpack_from('<I', blob, 4)[0],
                                              os.path.getsize(out) / 1e6))
    write_codes(db)


def write_codes(db):
    """the catalog's cards in EDOPro"""
    cat = json.load(open(os.path.join(ROOT, 'tools', 'ygo', 'catalog.json')))['cards']
    known = {r[0] for r in db.execute('select id from datas')}
    byname = {}
    for i, n in db.execute('select id, name from texts order by id desc'):
        byname[n] = i
    codes = {}
    for c in cat:
        pw = c['password']
        if pw not in known:
            pw = byname.get(c['name'])
            print('  %s: password %d not in EDOPro, by name %s' % (c['name'], c['password'], pw))
        codes[c['id']] = pw or 0
    n = max(codes)
    with open(os.path.join(ROOT, 'src', 'ygo', 'ygo_edo_codes.c'), 'w') as f:
        f.write('/* Generated by tools/ygo/edo/build_pack.py: each catalog card\'s code in EDOPro\'s database */\n'
                '#include <stdint.h>\n#include "ygo/ygo.h"\n\nconst uint32_t gYgoEdoCodes[YGO_CARD_COUNT + 1] = {\n    0,')
        for i in range(1, n + 1):
            f.write('%s%d,' % ('\n    ' if i % 8 == 1 else ' ', codes.get(i, 0)))
        f.write('\n};\n')


if __name__ == '__main__':
    main()
