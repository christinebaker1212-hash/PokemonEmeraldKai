#!/usr/bin/env python3
"""Stairway to the Destined Duel's card list and Forbidden & Limited list, read from the player's ROM.

usage: tools/ygo/import_sdd.py ROM.gba [OUT.json [ART.bin]]   (defaults tools/ygo/sdd.json, graphics/ygo/sdd_art.bin)

Needs the USA release (Yu-Gi-Oh! Worldwide Edition: Stairway to the Destined Duel, game code AYWE). The ROM is never
copied into the repository: only the facts this writes (names, numbers, stats, passwords, limits).

The tables (USA ROM offsets; the layout is the one Eternal Duelist Soul uses, see CosmicScribe64/eds-decomp's wiki,
data/card-table.md and special-card-lists.md):
  card names       0x3854A4  64-byte English records, card ID order (ID = record - 2; ID 0 is empty)
  card stats       0x832B84  u32 per ID: DEF/10 bits 0-8, ATK/10 9-17, kind 18-19 (normal, effect, fusion, ritual),
                             type 20-24 (21 Trap, 22 Spell), level 25-28, attribute 29-31; Spells/Traps: subtype 17-19
  card numbers     0x833C80  u16 per ID: Konami's card number (0 = Blue-Eyes White Dragon, 2000+ = alternate art)
  passwords        0x834500  4 BCD bytes per ID (FF FF FF FF: none)
  ban list         0x23F6F4  50 {u16 card number, u16 limit} pairs
  card pictures    0x3964E4  72x80, 9x10 tiles of 8x8 row-major, 6 bits per pixel as a little-endian bit stream
                             (8 pixels in 6 bytes), 0x10E0 bytes per ID
  their palettes   0x810C04  64 BGR555 colors (0x80 bytes) per ID

ART.bin (src/ygo/ygo_gfx.c reads it): "YGOA", u32 count (1086), then per Stairway card 1..count its 0x10E0 bytes of
pixels as stored in the ROM and its 0x80-byte palette.
"""
import json
import os
import struct
import sys

NAMES, STATS, NUMBERS, PASSWORDS, BANLIST = 0x3854A4 + 2 * 64, 0x832B84, 0x833C80, 0x834500, 0x23F6F4
ART, PALETTES, ART_SIZE, PAL_SIZE = 0x3964E4, 0x810C04, 0x10E0, 0x80
COUNT, NUMBERED, BANNED = 1087, 1084, 50
TYPES = ['', 'Dragon', 'Zombie', 'Fiend', 'Pyro', 'Sea Serpent', 'Rock', 'Machine', 'Fish', 'Dinosaur', 'Insect',
         'Beast', 'Beast-Warrior', 'Plant', 'Aqua', 'Warrior', 'Winged Beast', 'Fairy', 'Spellcaster', 'Thunder',
         'Reptile', 'Trap', 'Spell', 'Ticket', 'Divine']
ATTRIBUTES = ['', 'LIGHT', 'DARK', 'WATER', 'FIRE', 'EARTH', 'WIND']
KINDS = ['Normal', 'Effect', 'Fusion', 'Ritual']
SUBTYPES = ['Normal', 'Counter', 'Field', 'Equip', 'Continuous', 'Quick-Play', 'Ritual', '']


def main():
    rom = open(sys.argv[1], 'rb').read()
    out = sys.argv[2] if len(sys.argv) > 2 else 'tools/ygo/sdd.json'
    art = sys.argv[3] if len(sys.argv) > 3 else 'graphics/ygo/sdd_art.bin'
    if rom[0xAC:0xB0] != b'AYWE':
        sys.exit('not the USA Stairway to the Destined Duel (game code %r, want AYWE)' % rom[0xAC:0xB0])
    cards = []
    for cid in range(1, COUNT):
        name = rom[NAMES + cid * 64:NAMES + (cid + 1) * 64].split(b'\0')[0].decode('latin1')
        v = struct.unpack_from('<I', rom, STATS + 4 * cid)[0]
        number = struct.unpack_from('<H', rom, NUMBERS + 2 * cid)[0] if cid < NUMBERED else 1920 + cid - 1083
        pw = rom[PASSWORDS + 4 * cid:PASSWORDS + 4 * cid + 4].hex()
        typ = (v >> 20) & 31
        card = {'id': cid, 'number': number, 'name': name, 'password': pw if pw.isdigit() else None,
                'type': TYPES[typ]}
        if typ in (21, 22):
            card['subtype'] = SUBTYPES[(v >> 17) & 7]
        else:
            card.update(kind=KINDS[(v >> 18) & 3], level=(v >> 25) & 15, attribute=ATTRIBUTES[v >> 29],
                        atk=((v >> 9) & 0x1FF) * 10, defense=(v & 0x1FF) * 10)
        cards.append(card)
    bynum = {c['number']: c['name'] for c in cards}
    limits = []
    for k in range(BANNED):
        number, limit = struct.unpack_from('<HH', rom, BANLIST + 4 * k)
        limits.append({'number': number, 'limit': limit, 'name': bynum.get(number)})
    json.dump({'source': 'Yu-Gi-Oh! Worldwide Edition: Stairway to the Destined Duel (USA, AYWE)',
               'cards': cards, 'limits': limits}, open(out, 'w'), indent=1, ensure_ascii=False)
    blob = bytearray(b'YGOA' + struct.pack('<I', COUNT - 1))
    for cid in range(1, COUNT):
        blob += rom[ART + cid * ART_SIZE:ART + (cid + 1) * ART_SIZE] + rom[PALETTES + cid * PAL_SIZE:PALETTES + (cid + 1) * PAL_SIZE]
    os.makedirs(os.path.dirname(art), exist_ok=True)
    open(art, 'wb').write(blob)
    print('%d cards, %d limits -> %s; pictures -> %s' % (len(cards), len(limits), out, art))


if __name__ == '__main__':
    main()
