#!/usr/bin/env python3
"""Pokémon that Quetzal hands out from ROM tables rather than from scripts, plus its species list.

    python3 tools/quetzal_rip/gifts.py [QUETZAL.gba] [RIPDIR]     (default quetzal_rip; writes RIPDIR/data)

rip.py runs it at the end of a rip.

Writes gifts.json and names.json (every species id: const, name, types, base stat total; every move and item id:
const, name). Tables (ROM addresses):
- Static encounters, Johto 0x095BAA00: 106 x 32 bytes {u8 group, u8 num, u16 ?, u16 species, u16 level, u16 item,
  u16 flags[3] (+0xA show, +0xC, +0xE), ..., u8 kind (+0x1F)} (JohtoStaticEncountersRefresh 0x... special 704).
- Static encounters, Kanto 0x095BB74C: 123 x 16 bytes {u16 species, u16 flags[4], u8 group, u8 num, u8 local id?,
  ...} (special 755); the level comes from the map script.
- Starter choices 0x095C0FE8: u16 species list ending at 0x0FE8 (84 entries, every generation's starters plus
  Quetzal's three: Browt, Pombon, Gecqua); the Johto and Kanto choosers (specials 689 / 691) use it. Hoenn's list is
  0x09181EB8 (76).
- In-game trades 0x085DAC98: 60-byte entries (GetInGameTradeSpeciesInfo 0x080B9A64) {u8 nickname[14], u16 species
  (+0xE), u8 ivs[6], u8 abilityNum, pad, u32 otId (+0x18), u8 conditions[5], pad, u32 personality (+0x24),
  u16 heldItem (+0x28), u8 mailNum, u8 otName[8] (+0x2B), ..., u16 requestedSpecies (+0x38)}; scripts pick one
  with VAR_0x8008.
"""
import json
import os
import re
import sys

from qrom import Rom, REPO, Names, plain, find_rom

STATIC_JOHTO, STATIC_JOHTO_N = 0x15BAA00, 106
STATIC_KANTO, STATIC_KANTO_N = 0x15BB74C, 123
STARTERS = 0x15C0FE8
STARTERS_HOENN = 0x1181EB8
TRADES, TRADES_N = 0x5DAC98, 17
BASE_STATS = 0x550A58 - 36       # 36 bytes per species from 1: stats +0..5 (HP Atk Def Spe SpA SpD), types +6/+7
TYPES = ['NORMAL', 'FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'MYSTERY', 'FIRE',
         'WATER', 'GRASS', 'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']


def species_list(rom, base, names):
    out = []
    while True:
        v = rom.u16(base + 2 * len(out))
        if v == 0 or v >= len(names.species_const):
            return out
        out.append(names.species_const[v])


def trade_users(root):
    """trade index -> scripts that start it (setvar VAR_0x8008, N right before the trade specials, or before a
    call to Kanto's shared trade script Quetzal_EventScript_8459170)."""
    users = {}
    for dirpath, _, files in [w for sub in ('maps', 'scripts') for w in os.walk(os.path.join(root, sub))]:
        for f in files:
            if not f.endswith('.inc'):
                continue
            p = os.path.join(dirpath, f)
            lines = open(p, encoding='utf-8').read().split('\n')
            for i, l in enumerate(lines):
                m = re.match(r'\s*setvar VAR_0x8008, (\w+)', l)
                if m and any('GetInGameTradeSpeciesInfo' in x or 'Quetzal_EventScript_8459170' in x
                             for x in lines[i + 1:i + 4]):
                    users.setdefault(int(m.group(1), 0), []).append(os.path.relpath(p, root))
    return users


def main():
    rom = Rom(sys.argv[1] if len(sys.argv) > 1 else find_rom())
    root = sys.argv[2] if len(sys.argv) > 2 else os.path.join(REPO, 'quetzal_rip')
    write(rom, Names(rom), root)


def write(rom, names, root):
    """root = the rip folder (reads its maps/index.json and scripts, writes its data/)."""
    out = os.path.join(root, 'data')
    index = {}
    p = os.path.join(root, 'maps', 'index.json')
    if os.path.exists(p):
        index = {(m['group'], m['num']): m['name'] for m in json.load(open(p))['maps']}
    sp = names.species_const

    statics = []
    for i in range(STATIC_JOHTO_N):
        o = STATIC_JOHTO + 32 * i
        g, n = rom.u8(o), rom.u8(o + 1)
        statics.append({'region': 'johto', 'index': i, 'map': index.get((g, n)), 'group': g, 'num': n,
                        'species': sp[rom.u16(o + 4)], 'level': rom.u16(o + 6), 'item': names.item_name(rom.u16(o + 8)),
                        'flags': ['0x%X' % rom.u16(o + k) for k in (0xA, 0xC, 0xE)], 'kind': rom.u8(o + 0x1F),
                        'raw': rom.d[o:o + 32].hex()})
    for i in range(STATIC_KANTO_N):
        o = STATIC_KANTO + 16 * i
        g, n = rom.u8(o + 10), rom.u8(o + 11)
        statics.append({'region': 'kanto', 'index': i, 'map': index.get((g, n)), 'group': g, 'num': n,
                        'species': sp[rom.u16(o)], 'flags': ['0x%X' % rom.u16(o + k) for k in (2, 4, 6, 8)],
                        'byte_0xC': rom.u8(o + 12), 'raw': rom.d[o:o + 16].hex()})

    users = trade_users(root)
    trades = []
    for i in range(TRADES_N):
        o = TRADES + 0x3C * i
        trades.append({'index': i, 'nickname': plain(rom, o, 14), 'species': sp[rom.u16(o + 0xE)],
                       'requested_species': sp[rom.u16(o + 0x38)], 'held_item': names.item_name(rom.u16(o + 0x28)),
                       'ivs': list(rom.d[o + 0x10:o + 0x16]), 'ability_num': rom.u8(o + 0x16),
                       'ot_id': rom.u32(o + 0x18), 'personality': rom.u32(o + 0x24),
                       'ot_name': plain(rom, o + 0x2B, 8), 'used_by': users.get(i, []),
                       'raw': rom.d[o:o + 0x3C].hex()})

    gifts = {'_note': 'see tools/quetzal_rip/gifts.py for the table layouts',
             'starter_choices': species_list(rom, STARTERS, names),
             'starter_choices_hoenn': species_list(rom, STARTERS_HOENN, names),
             'static_encounters': statics, 'in_game_trades': trades}
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, 'gifts.json'), 'w', encoding='utf-8') as fh:
        json.dump(gifts, fh, indent=1, ensure_ascii=False)
        fh.write('\n')
    with open(os.path.join(out, 'names.json'), 'w', encoding='utf-8') as fh:
        fh.write('{"_note": "Quetzal species ids: 1-898 national dex, 899-948 megas, then forms (same name as the '
                 'base species), Gen 9 / Hisui and Quetzal\'s own. Rows: [id, const, name, types (Quetzal\'s, with Fairy), '
                 'base stat total]", "species": [\n')
        rows = []
        for i, c in enumerate(sp):
            o = BASE_STATS + 36 * i
            st = list(rom.d[o:o + 6]) if i else [0] * 6
            ty = sorted({TYPES[t] if t < len(TYPES) else str(t) for t in rom.d[o + 6:o + 8]}) if i else []
            rows.append(json.dumps([i, c, names.species[i] if i < len(names.species) else '', ty, sum(st)],
                                   ensure_ascii=False))
        fh.write(',\n'.join(rows))
        fh.write('\n],\n"moves": [\n')
        fh.write(',\n'.join(json.dumps([i, c, names.moves[i]], ensure_ascii=False) for i, c in enumerate(names.move_const)))
        fh.write('\n],\n"items": [\n')
        fh.write(',\n'.join(json.dumps([i, c, names.items[i]], ensure_ascii=False) for i, c in enumerate(names.item_const)))
        fh.write('\n]}\n')
    print('gifts: %d statics, %d starters, %d trades; %d species' % (
        len(statics), len(gifts['starter_choices']), len(trades), len(sp)))


if __name__ == '__main__':
    main()
