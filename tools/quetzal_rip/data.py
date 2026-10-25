"""Trainers and wild encounters of Quetzal's Kanto and Johto.

Trainers (32 bytes, GetTrainer = 0x082A164C):
  +0x00 u32 aiFlags          +0x04 party*           +0x08 u16 items[4]
  +0x10 u8 trainerClass      +0x11 u8 encounterMusic (bit 7 = female)   +0x12 u8 trainerPic
  +0x13 u8 name[11]          +0x1E u8 bit0 doubleBattle, bits 1-3 party type   +0x1F u8 partySize
  Hoenn ids 0-973 (0x0850F188); Johto ids 0x1000-0x1380 (0x095A8028, a presence byte per id at
  0x095AF048); Kanto ids 0x2000-0x22E8 (0x095B357C, presence 0x095B929C). Kanto trainerPic + 0xDF
  is the real front pic id (0x082A16D4).
Party types (GetTrainerHighestLevel 0x0803F894, CreateNPCTrainerParty ~0x08040108):
  0: 8 bytes  {u16 iv, u16 lvl, u16 species, u16 pad}
  1: 16 bytes {u16 iv, u16 lvl, u16 species, u16 moves[4], pad}
  2: 8 bytes  {u16 iv, u16 lvl, u16 species, u16 heldItem}
  3: 16 bytes {u16 iv, u16 lvl, u16 species, u16 heldItem, u16 moves[4]}
  4: 28 bytes {u16 iv, u8 evs[6], u8 lvl, u8 pad, u16 species, u8 nature?, u8 abilityNum?,
               u16 heldItem, u8 unknown(+0x10), u8 pad, u16 moves[4], u16 pad}
Levels go through 0x08262D18 when the party is built (Quetzal's level scaling / difficulty).

Wild encounters: header 28 bytes {u8 group, u8 num, u16 pad, 6 x WildPokemonInfo*} ending at group 0xFF
(0x0918955C). Slots: 0 land (12), 1 water (5), 2 rock smash (3), 3 headbutt-style trees (5; rate 60,
fixed levels, Johto routes with trees), 4 fishing (10), 5 unused. WildPokemonInfo {u8 rate, ptr};
WildPokemon {u8 min, u8 max, u16 species}.
"""
import json
import os

from qrom import decode

TRAINER_TABLES = {'hoenn': (0x50F188, 0, 0x3CD, None), 'johto': (0x15A8028, 0x1000, 0x380, 0x15AF048),
                  'kanto': (0x15B357C, 0x2000, 0x2E8, 0x15B929C)}
CLASS_NAMES = 0x50ED90
NUM_CLASSES = 78
WILD_HEADERS = 0x118955C
WILD_SLOTS = [('land_mons', 12), ('water_mons', 5), ('rock_smash_mons', 3), ('headbutt_mons', 5),
              ('fishing_mons', 10), ('slot5_mons', 5)]
PARTY_SIZES = {0: 8, 1: 16, 2: 8, 3: 16, 4: 28}


def class_names(rom):
    return [decode(rom, CLASS_NAMES + 13 * i, 13)[0] for i in range(NUM_CLASSES)]


def party(rom, names, p, ptype, n):
    out = []
    sz = PARTY_SIZES.get(ptype)
    if sz is None or p is None:
        return out
    for i in range(n):
        e = p + sz * i
        if ptype == 4:
            m = {'iv': rom.u16(e), 'evs': list(rom.d[e + 2:e + 8]), 'lvl': rom.u8(e + 8),
                 'species': names.species_name(rom.u16(e + 10)), 'nature': rom.u8(e + 12),
                 'ability_num': rom.u8(e + 13), 'held_item': names.item_name(rom.u16(e + 14)),
                 'unknown_0x10': rom.u8(e + 16),
                 'moves': [names.move_name(rom.u16(e + 18 + 2 * k)) for k in range(4)]}
        else:
            m = {'iv': rom.u16(e), 'lvl': rom.u16(e + 2), 'species': names.species_name(rom.u16(e + 4))}
            if ptype in (2, 3):
                m['held_item'] = names.item_name(rom.u16(e + 6))
            if ptype == 1:
                m['moves'] = [names.move_name(rom.u16(e + 6 + 2 * k)) for k in range(4)]
            if ptype == 3:
                m['moves'] = [names.move_name(rom.u16(e + 8 + 2 * k)) for k in range(4)]
        out.append(m)
    return out


def trainers(rom, names, used_ids):
    classes = class_names(rom)
    out = []
    for region, (base, first, count, presence) in TRAINER_TABLES.items():
        if region == 'hoenn':
            continue
        for i in range(count + 1):
            tid = first + i
            if presence is not None and rom.d[presence + i] == 0 and tid not in used_ids:
                continue
            t = base + 32 * i
            flags = rom.u8(t + 0x1E)
            ptype = flags >> 1 & 7
            n = rom.u8(t + 0x1F)
            name = decode(rom, t + 0x13, 11)[0]
            if not name and not n:
                continue
            cls = rom.u8(t + 0x10)
            pic = rom.u8(t + 0x12)
            out.append({
                'id': tid, 'const': 'TRAINER_QZ_%d' % tid, 'region': region,
                'present': bool(rom.d[presence + i]) if presence is not None else True,
                'used_by_scripts': tid in used_ids,
                'name': name, 'class': cls, 'class_name': classes[cls] if cls < len(classes) else None,
                'encounter_music': rom.u8(t + 0x11) & 0x7F, 'female': bool(rom.u8(t + 0x11) & 0x80),
                'trainer_pic': pic, 'front_pic_id': pic + 0xDF if region == 'kanto' else pic,
                'items': [names.item_name(rom.u16(t + 8 + 2 * k)) for k in range(4)],
                'double_battle': bool(flags & 1), 'ai_flags': '0x%X' % rom.u32(t),
                'party_type': ptype, 'party_size': n,
                'party_addr': '0x%X' % rom.u32(t + 4),
                'party': party(rom, names, rom.ptr(t + 4), ptype, n)})
    return out


def wild(rom, names, map_const, groups):
    out = []
    o = WILD_HEADERS
    while rom.d[o] != 0xFF:
        g, n = rom.d[o], rom.d[o + 1]
        if g in groups:
            e = {'map': map_const((g, n)), 'base_label': 'gQuetzal_%d_%d' % (g, n)}
            for k, (field, count) in enumerate(WILD_SLOTS):
                p = rom.ptr(o + 4 + 4 * k)
                if p is None:
                    continue
                rate = rom.u8(p)
                mp = rom.ptr(p + 4)
                mons = [{'min_level': rom.u8(mp + 4 * i), 'max_level': rom.u8(mp + 4 * i + 1),
                         'species': names.species_name(rom.u16(mp + 4 * i + 2))} for i in range(count)]
                e[field] = {'encounter_rate': rate, 'mons': mons}
            out.append(e)
        o += 28
    return out


def write_all(out, rom, names, map_const, used_trainer_ids, groups):
    d = os.path.join(out, 'data')
    os.makedirs(d, exist_ok=True)
    tr = trainers(rom, names, used_trainer_ids)
    with open(os.path.join(d, 'trainers.json'), 'w', encoding='utf-8') as fh:
        json.dump({'class_names': class_names(rom), 'trainers': tr}, fh, indent=1, ensure_ascii=False)
        fh.write('\n')
    wl = wild(rom, names, map_const, groups)
    fields = [{'type': 'land_mons', 'encounter_rates': [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1]},
              {'type': 'water_mons', 'encounter_rates': [60, 30, 5, 4, 1]},
              {'type': 'rock_smash_mons', 'encounter_rates': [60, 30, 10], 'note': '3 slots in Quetzal (Emerald has 5); rates are a guess'},
              {'type': 'headbutt_mons', 'encounter_rates': [50, 15, 15, 10, 10], 'note': 'new slot; rates are a guess'},
              {'type': 'fishing_mons', 'encounter_rates': [70, 30, 60, 20, 20, 40, 40, 15, 4, 1],
               'groups': {'old_rod': [0, 1], 'good_rod': [2, 3, 4], 'super_rod': [5, 6, 7, 8, 9]}}]
    with open(os.path.join(d, 'wild_encounters.json'), 'w') as fh:
        json.dump({'wild_encounter_groups': [{'label': 'gWildMonHeaders_Quetzal', 'for_maps': True,
                                              'fields': fields, 'encounters': wl}]}, fh, indent=1)
        fh.write('\n')
    return tr, wl


def mapsecs(rom, used):
    """Region map sections the Kanto / Johto maps use: {x, y, w, h, names (EN, ES, ES, short)}.
    Johto has its own table (0x09233C94) indexed by the same mapsec byte; Kanto uses the main one."""
    from qrom import MAPSEC_MAIN, MAPSEC_JOHTO
    out = []
    for region, sec in sorted(used):
        t = (MAPSEC_JOHTO if region == 'johto' else MAPSEC_MAIN) + 8 * sec
        names = []
        p = rom.ptr(t + 4)
        if p is not None:
            for k in range(4):
                q = rom.ptr(p + 4 * k)
                names.append(decode(rom, q, 40)[0] if q is not None else None)
        out.append({'region': region, 'index': sec, 'x': rom.u8(t), 'y': rom.u8(t + 1),
                    'width': rom.u8(t + 2), 'height': rom.u8(t + 3), 'name': names[0] if names else None,
                    'names_all_languages': names})
    return out
