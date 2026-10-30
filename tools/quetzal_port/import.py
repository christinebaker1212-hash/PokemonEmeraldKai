#!/usr/bin/env python3
"""Imports Moemon Quetzal's Johto and Kanto (quetzal_rip/, with the Gen 4 cap of quetzal_rip/gen4/overlay over it)
into Kai's own data, so the regions build with the game (docs/REGIONS_PLAN.md).

    python3 tools/quetzal_port/import.py

Run once: afterwards the imported files are Kai's to edit (story changes, fixes). Running it again rewrites every
imported map, layout, tileset and generated file from the rip, so hand edits there would be lost; the script refuses
unless --force. What it writes:
- data/maps/<Johto_*|Kanto_*>/ (map.json + scripts.inc), groups gMapGroup_Johto1-2 / gMapGroup_Kanto1-4 (Quetzal's
  numbers 34-39) appended to data/maps/map_groups.json
- data/layouts/<name>/ + entries in data/layouts/layouts.json
- data/tilesets/{primary,secondary}/<snake name>/ + src/data/tilesets/qz_{graphics,metatiles,headers}.h
- data/scripts/qz/*.inc (scripts several maps share) and data/event_scripts_qz.s, which assembles them all
- include/constants/qz_port.h: every placeholder the rip uses (flags, vars, graphics, music, trainers) on a Kai value
- include/constants/region_map_sections_johto.h + src/data/region_map/johto_map_sections.h: Johto's map sections
  (0x100 and up, map header's u16) and their names
- data/qz_specials.inc + src/qz_specials_stub.c: Quetzal's specials as Kai specials (QzName), stubbed until ported
"""
import glob
import json
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
RIP = os.path.join(REPO, 'quetzal_rip')
OVERLAY = os.path.join(RIP, 'gen4', 'overlay')

# Flags: Quetzal's 0x1000-0x2FFF stay where they are (kept in the region sidecar, src/mp/mp_region.c); the few
# below 0x1000 that aren't temp flags, the Kanto hidden items and the Gen 4 cap flag move to free ranges there.
FLAG_LOW_BASE = 0x2A00       # Quetzal's flags below 0x1000 (reused Emerald numbers): 0x2A00 + n
FLAG_HIDDEN_BASE = 0x2B00    # FLAG_QZ_KANTO_HIDDEN_ITEM_<n>
FLAG_GEN_CAPPED = 0x2CFF
VAR_LOW_BASE = 0x5200        # Quetzal's vars in Emerald's range 0x40xx: 0x5200 + n


def rip_path(rel):
    o = os.path.join(OVERLAY, rel)
    return o if os.path.exists(o) else os.path.join(RIP, rel)


def snake(name):
    s = re.sub(r'(?<=[a-z0-9])([A-Z])', r'_\1', name)
    s = re.sub(r'(?<=[A-Z])([A-Z][a-z])', r'_\1', s)
    return s.lower()


def load(path):
    return json.load(open(path, encoding='utf-8'))


def dump(obj, path):
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(obj, f, indent=2, ensure_ascii=False)
        f.write('\n')


def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(text)


# ---------------------------------------------------------------------------------------------------- tilesets
def kai_tileset_for(kind, rip_dir):
    """A stock Emerald tileset the rip copied: the Kai tileset with the same metatiles."""
    mt = open(os.path.join(rip_dir, 'metatiles.bin'), 'rb').read()
    hdr = open(os.path.join(REPO, 'src/data/tilesets/metatiles.h'), encoding='utf-8').read()
    best = (0, None)
    for d in glob.glob(os.path.join(REPO, 'data', 'tilesets', kind, '*')):
        f = os.path.join(d, 'metatiles.bin')
        if not os.path.exists(f):
            continue
        k = open(f, 'rb').read()
        same = sum(a == b for a, b in zip(mt, k)) / max(len(mt), len(k))
        m = re.search(r'const u16 gMetatiles_(\w+)\[\] = INCBIN_U16\("data/tilesets/%s/%s/metatiles.bin"\)'
                      % (kind, os.path.basename(d)), hdr)
        if m and same > best[0]:
            best = (same, 'gTileset_' + m.group(1))
    if best[0] > 0.95:   # Kai's own edits of a stock tileset win
        return best[1]
    raise SystemExit('no Kai tileset matches %s' % rip_dir)


def import_tilesets():
    info = load(os.path.join(RIP, 'tilesets', 'tilesets.json'))
    rename = {}
    gfx, mts, hdrs = [], [], []
    for t in info:
        kind = t['kind']
        short = t['name'][len('gTileset_'):]
        src = os.path.join(RIP, 'tilesets', kind, short)
        if short.startswith('Emerald_'):
            rename[t['name']] = kai_tileset_for(kind, src)
            continue
        sym = t['name']
        d = 'data/tilesets/%s/%s' % (kind, snake(short))
        dst = os.path.join(REPO, d)
        if os.path.exists(dst):
            shutil.rmtree(dst)
        shutil.copytree(src, dst)
        frlg = kind == 'primary' and (t['tiles'] > 512 or t['metatiles'] > 512)
        gfx.append('const u32 gTilesetTiles_%s[] = INCGFX_U32("%s/tiles.png", ".4bpp.lz", "-num_tiles %d -Wnum_tiles");\n'
                   % (short, d, t['tiles']))
        gfx.append('const u16 gTilesetPalettes_%s[][16] =\n{\n%s};\n\n' % (short, ''.join(
            '\tINCGFX_U16("%s/palettes/%02d.pal", ".gbapal"),\n' % (d, i) for i in range(16))))
        mts.append('const u16 gMetatiles_%s[] = INCBIN_U16("%s/metatiles.bin");\n' % (short, d))
        mts.append('const u16 gMetatileAttributes_%s[] = INCBIN_U16("%s/metatile_attributes.bin");\n' % (short, d))
        hdrs.append('const struct Tileset %s =\n{\n    .isCompressed = TRUE,\n    .isSecondary = %s,\n%s'
                    '    .tiles = gTilesetTiles_%s,\n    .palettes = gTilesetPalettes_%s,\n    .metatiles = gMetatiles_%s,\n'
                    '    .metatileAttributes = gMetatileAttributes_%s,\n    .callback = NULL,\n};\n\n'
                    % (sym, 'TRUE' if kind == 'secondary' else 'FALSE', '    .isFrlg = TRUE,\n' if frlg else '',
                       short, short, short, short))
    head = '// Generated by tools/quetzal_port/import.py: Johto and Kanto (Moemon Quetzal)\n'
    write(os.path.join(REPO, 'src/data/tilesets/qz_graphics.h'), head + ''.join(gfx))
    write(os.path.join(REPO, 'src/data/tilesets/qz_metatiles.h'), head + ''.join(mts))
    write(os.path.join(REPO, 'src/data/tilesets/qz_headers.h'), head + ''.join(hdrs))
    print('tilesets: %d imported, %d stock ones mapped onto Kai\'s' % (len(hdrs), len(rename)))
    return rename


# ---------------------------------------------------------------------------------------------------- layouts
def import_layouts(rename):
    rip = load(os.path.join(RIP, 'layouts', 'layouts.json'))
    kai_path = os.path.join(REPO, 'data', 'layouts', 'layouts.json')
    kai = load(kai_path)
    kai['layouts'] = [l for l in kai['layouts'] if not l.get('quetzal')]
    for l in rip['layouts']:
        name = l['name'][:-len('_Layout')]
        d = 'data/layouts/%s' % name
        dst = os.path.join(REPO, d)
        os.makedirs(dst, exist_ok=True)
        for f in ('map.bin', 'border.bin'):
            shutil.copy(os.path.join(REPO, l['%s_filepath' % ('blockdata' if f == 'map.bin' else 'border')]), os.path.join(dst, f))
        kai['layouts'].append({
            'id': l['id'], 'name': l['name'], 'width': l['width'], 'height': l['height'],
            'primary_tileset': rename.get(l['primary_tileset'], l['primary_tileset']),
            'secondary_tileset': rename.get(l['secondary_tileset'], l['secondary_tileset']),
            'border_filepath': d + '/border.bin', 'blockdata_filepath': d + '/map.bin', 'quetzal': True,
        })
        if (l.get('border_width', 2), l.get('border_height', 2)) != (2, 2):
            print('  note: %s has a %dx%d border (Kai draws 2x2)' % (name, l['border_width'], l['border_height']))
    dump(kai, kai_path)
    print('layouts: %d' % len(rip['layouts']))


# ---------------------------------------------------------------------------------------------------- symbols
def flag_value(sym):
    m = re.match(r'FLAG_QZ_0x([0-9A-Fa-f]+)$', sym)
    if m:
        v = int(m.group(1), 16)
        if v < 0x20:
            return v          # temp flags: Emerald's own numbers
        if v < 0x1000:
            return None       # assigned below (0x2A00 + n)
        return v
    m = re.match(r'FLAG_QZ_KANTO_HIDDEN_ITEM_(\d+)$', sym)
    if m:
        return FLAG_HIDDEN_BASE + int(m.group(1))
    if sym == 'FLAG_QZ_GEN_CAPPED':
        return FLAG_GEN_CAPPED
    raise SystemExit('unknown flag ' + sym)


def gather(pattern, texts):
    out = set()
    for t in texts:
        out |= set(re.findall(pattern, t))
    return out


def write_constants(texts):
    obj = load(os.path.join(RIP, 'object_events', 'object_events.json'))
    lines = ['// Generated by tools/quetzal_port/import.py: what the Johto / Kanto content names, on Kai values.',
             '#ifndef GUARD_CONSTANTS_QZ_PORT_H', '#define GUARD_CONSTANTS_QZ_PORT_H', '']
    flags = sorted(gather(r'\bFLAG_QZ_\w+', texts))
    low = sorted(f for f in flags if flag_value(f) is None)
    lines.append('// flags (Quetzal 0x1000-0x2FFF kept; its reused Emerald numbers from 0x%X, Kanto hidden items from 0x%X)'
                 % (FLAG_LOW_BASE, FLAG_HIDDEN_BASE))
    for f in flags:
        v = flag_value(f)
        if v is None:
            v = FLAG_LOW_BASE + low.index(f)
        lines.append('#define %-40s 0x%X' % (f, v))
    vars_ = sorted(gather(r'\bVAR_QZ_\w+', texts))
    lowv = [v for v in vars_ if int(v.split('0x')[1], 16) < 0x5000]
    lines.append('\n// vars (Quetzal 0x5000-0x51FF kept; its Emerald-range ones from 0x%X)' % VAR_LOW_BASE)
    for v in vars_:
        n = int(v.split('0x')[1], 16)
        lines.append('#define %-40s 0x%X' % (v, n if n >= 0x5000 else VAR_LOW_BASE + lowv.index(v)))
    objs_path = os.path.join(HERE, 'objects.json')   # tools/quetzal_port/gen_objects.py
    objs = load(objs_path) if os.path.exists(objs_path) else {}
    lines.append('\n// overworld graphics (tools/quetzal_port/gen_objects.py)')
    for g in sorted(gather(r'\bOBJ_EVENT_GFX_QZ_\w+', texts), key=lambda s: int(s.rsplit('_', 1)[1])):
        lines.append('#define %-40s %s' % (g, objs.get(g, 'OBJ_EVENT_GFX_BOY_1')))
    lines.append('\n// music (placeholders until the songs are imported)')
    for g in sorted(gather(r'\bMUS_QZ_\w+', texts), key=lambda s: int(s.rsplit('_', 1)[1])):
        lines.append('#define %-40s MUS_ROUTE101' % g)
    lines.append('\n// Quetzal\'s own movement types: its Pokémon objects idle in place (0x54-0x57: down, up, left, right as guessed\n'
                 '// from where they stand), 0x64 NPCs stand facing down')
    for g in sorted(gather(r'\bMOVEMENT_TYPE_QZ_\w+', texts)):
        v = int(g.rsplit('_', 1)[1], 16)
        lines.append('#define %-40s %s' % (g, {0x54: 'MOVEMENT_TYPE_WALK_SLOWLY_IN_PLACE_DOWN', 0x55: 'MOVEMENT_TYPE_WALK_SLOWLY_IN_PLACE_UP',
                                                0x56: 'MOVEMENT_TYPE_WALK_SLOWLY_IN_PLACE_LEFT', 0x57: 'MOVEMENT_TYPE_WALK_SLOWLY_IN_PLACE_RIGHT'}
                                               .get(v, 'MOVEMENT_TYPE_FACE_DOWN')))
    lines.append('\n// trainers (placeholders until the trainers are imported)')
    for g in sorted(gather(r'\bTRAINER_QZ_\w+', texts)):
        lines.append('#define %-40s TRAINER_NONE' % g)
    lines += ['', '#endif', '']
    write(os.path.join(REPO, 'include/constants/qz_port.h'), '\n'.join(lines))
    print('constants: %d flags, %d vars' % (len(flags), len(vars_)))


def write_johto_sections():
    secs = [s for s in load(os.path.join(RIP, 'data', 'region_map_sections.json')) if s['region'] == 'johto']
    names = []
    lines = ['// Generated by tools/quetzal_port/import.py: Johto\'s map sections. They live past Kai\'s one-byte ones, in the',
             '// map header\'s u16 (struct MapHeader regionMapSectionId); names in src/data/region_map/johto_map_sections.h.',
             '#ifndef GUARD_CONSTANTS_REGION_MAP_SECTIONS_JOHTO_H', '#define GUARD_CONSTANTS_REGION_MAP_SECTIONS_JOHTO_H',
             '#define MAPSEC_JOHTO_FIRST 0x100']
    seen = {}
    for i, s in enumerate(secs):
        const = 'MAPSEC_JOHTO_' + re.sub(r'[^A-Z0-9]+', '_', s['name'].replace('’', '').replace('.', '')).strip('_')
        const = re.sub(r'ROUTE_(\d)', r'ROUTE\1', const)
        if const in seen:
            const += '_%d' % i
        seen[const] = i
        lines.append('#define %-40s (MAPSEC_JOHTO_FIRST + %d)' % (const, i))
        names.append(s)
    lines += ['#define MAPSEC_JOHTO_END (MAPSEC_JOHTO_FIRST + %d)' % len(secs), '#endif', '']
    write(os.path.join(REPO, 'include/constants/region_map_sections_johto.h'), '\n'.join(lines))
    c = ['// Generated by tools/quetzal_port/import.py: Johto\'s map section names and places on its map',
         'const u8 *const gJohtoMapSectionNames[] =', '{']
    for s in names:
        c.append('    (const u8 []) _("%s"),' % s['name'].replace('’', '\'').replace('MT MORTAR', 'MT. MORTAR'))
    c += ['};', '', 'const struct { u8 x, y, width, height; } gJohtoMapSectionPlaces[] =', '{']
    for s in names:
        c.append('    {%d, %d, %d, %d},' % (s['x'], s['y'], s['width'], s['height']))
    c += ['};', '']
    write(os.path.join(REPO, 'src/data/region_map/johto_map_sections.h'), '\n'.join(c))
    return {s: i for s, i in seen.items()}


def johto_section_names(mapsec_map):
    """The rip names Johto sections after its own spellings (MAPSEC_JOHTO_ROUTE29...): same as ours? Check."""
    return mapsec_map


# ---------------------------------------------------------------------------------------------------- ids
class Ids:
    """Quetzal's raw item numbers and names Kai lacks, through the Gen 4 cap's rules (tools/quetzal_rip/gencap.py)."""
    def __init__(self):
        sys.path.insert(0, os.path.join(REPO, 'tools', 'quetzal_rip'))
        import gencap
        self.cap = gencap.Capper(4)
        names = load(os.path.join(RIP, 'data', 'names.json'))
        self.item_by_id = {i: c for i, c, n in names['items']}
        def consts(f, pre):
            return set(re.findall(r'#define (%s\w+)' % pre, open(os.path.join(REPO, f), encoding='utf-8').read()))
        self.kai = consts('include/constants/items.h', 'ITEM_') | consts('include/constants/moves.h', 'MOVE_') | \
            consts('include/constants/species.h', 'SPECIES_')
        self.missing = {}

    def item(self, const, where):
        if const in self.kai:
            return const
        new = self.cap.cap_item(const, where)
        if new not in self.kai:
            self.missing.setdefault(new, set()).add(where)
        return new

    def move(self, const, where):
        if const in self.kai:
            return const
        k = gencap.key(const[5:]) if False else None
        new = self.cap.qmove_by_key.get('') if False else None
        # a later move: the Gen 4 cap's choice for a level-up move of nobody: a move of the same type and power
        sub = {'MOVE_RAGE_FIST': 'MOVE_DYNAMIC_PUNCH'}.get(const)
        if sub is None:
            self.missing.setdefault(const, set()).add(where)
            return const
        return sub

    def fix(self, text, where):
        # given / found items written as Quetzal numbers
        def raw(m):
            n = int(m.group(2), 0)
            c = self.item_by_id.get(n)
            return m.group(1) + (self.item(c, where) if c else m.group(2))
        text = re.sub(r'(setorcopyvar VAR_0x8000, )(0x[0-9A-Fa-f]+|\d+)(?=\n\tsetorcopyvar VAR_0x8001, \w+\n\tcallstd STD_(?:OBTAIN|FIND)_ITEM)',
                      raw, text)
        text = re.sub(r'\bITEM_\w+\b', lambda m: self.item(m.group(0), where) if m.group(0) not in
                      ('ITEM_NONE', 'ITEM_USE_FIELD') else m.group(0), text)
        text = re.sub(r'\bMOVE_[A-Z0-9_]+\b', lambda m: self.move(m.group(0), where), text)
        for sp in set(re.findall(r'\bSPECIES_[A-Z0-9_]+\b', text)):
            if sp not in self.kai:
                self.missing.setdefault(sp, set()).add(where)
        return text


IDS = None


# ---------------------------------------------------------------------------------------------------- scripts
SPECIAL_RE = re.compile(r'\b(Qz\w+?)_(\d+)\b')


def script_text(text, where=''):
    text = SPECIAL_RE.sub(lambda m: m.group(1), text)
    return IDS.fix(text, where)


def import_maps():
    groups_path = os.path.join(REPO, 'data', 'maps', 'map_groups.json')
    groups = load(groups_path)
    rip_groups = load(os.path.join(RIP, 'maps', 'map_groups.json'))
    names = {'gMapGroup_Quetzal34_Johto': 'gMapGroup_Johto1', 'gMapGroup_Quetzal35_Johto': 'gMapGroup_Johto2',
             'gMapGroup_Quetzal36_Kanto': 'gMapGroup_Kanto1', 'gMapGroup_Quetzal37_Kanto': 'gMapGroup_Kanto2',
             'gMapGroup_Quetzal38_Kanto': 'gMapGroup_Kanto3', 'gMapGroup_Quetzal39_Kanto': 'gMapGroup_Kanto4'}
    for g in names.values():
        groups.pop(g, None)
    groups['group_order'] = [g for g in groups['group_order'] if g not in names.values()]
    texts = []
    maps = []
    for rg in rip_groups['group_order']:
        kg = names[rg]
        groups['group_order'].append(kg)
        groups[kg] = list(rip_groups[rg])
        maps += rip_groups[rg]
    for m in maps:
        src_json = rip_path('maps/%s/map.json' % m)
        mj = load(src_json)
        mj.pop('quetzal', None)
        for i, o in enumerate(mj.get('object_events') or []):
            lid = o.get('local_id')
            if lid is not None and str(lid).isdigit():
                if int(lid) != i + 1:
                    print('  note: %s object %d has local id %s' % (m, i, lid))
                del o['local_id']   # mapjson numbers them in order (and would #define the bare number)
        dst = os.path.join(REPO, 'data', 'maps', m)
        os.makedirs(dst, exist_ok=True)
        mj = json.loads(IDS.fix(json.dumps(mj, ensure_ascii=False), m))
        dump(mj, os.path.join(dst, 'map.json'))
        sp = rip_path('maps/%s/scripts.inc' % m)
        s = script_text(open(sp, encoding='utf-8').read(), m) if os.path.exists(sp) else '%s_MapScripts::\n\t.byte 0\n' % m
        write(os.path.join(dst, 'scripts.inc'), s)
        texts += [json.dumps(mj), s]
    order = groups.get('connections_include_order')
    dump(groups, groups_path)
    print('maps: %d in %d groups' % (len(maps), len(names)))
    return maps, texts


def import_shared_scripts(maps):
    texts = []
    out = []
    for f in sorted(glob.glob(os.path.join(RIP, 'scripts', '*.inc'))):
        b = os.path.basename(f)
        s = script_text(open(rip_path('scripts/' + b), encoding='utf-8').read(), b)
        write(os.path.join(REPO, 'data', 'scripts', 'qz', b), s)
        texts.append(s)
        out.append(b)
    head = open(os.path.join(REPO, 'data', 'event_scripts.s'), encoding='utf-8').read()
    head = head[:head.index('\t.section script_data')]
    s = ['@ Generated by tools/quetzal_port/import.py: Johto and Kanto (Moemon Quetzal), every map script and shared script',
         head.rstrip(), '#include "constants/qz_port.h"', '#include "constants/region_map_sections_johto.h"',
         '\t.include "asm/macros/qz_event.inc"', '', '\t.section script_data, "aw"', '']
    s += ['\t.include "data/scripts/qz/%s"' % b for b in out]
    s += ['\t.include "data/maps/%s/scripts.inc"' % m for m in maps]
    write(os.path.join(REPO, 'data', 'event_scripts_qz.s'), '\n'.join(s) + '\n')
    return texts


def write_specials(texts):
    names = sorted(gather(r'\bspecial(?:var)?\s+(?:\w+,\s*)?(Qz\w+)', texts))
    write(os.path.join(REPO, 'data', 'qz_specials.inc'),
          '@ Generated by tools/quetzal_port/import.py: Quetzal\'s specials (src/qz_specials.c)\n' +
          ''.join('\tdef_special %s\n' % n for n in names))
    stub = os.path.join(REPO, 'src', 'qz_specials_stub.c')
    write(stub, '// Generated by tools/quetzal_port/import.py: Quetzal specials not ported yet (each one logs and does\n'
          '// nothing; src/qz_specials.c defines the ported ones and the list below shrinks as they move there)\n'
          '#include "global.h"\n#include "event_data.h"\n#include "qz_specials.h"\n\n' +
          ''.join('QZ_STUB(%s)\n' % n for n in names))
    print('specials: %d' % len(names))
    return names


def write_macros():
    src = open(os.path.join(RIP, 'include', 'quetzal_macros.inc'), encoding='utf-8').read()
    src = '\n'.join(l for l in src.split('\n') if not re.match(r'\s*\.set SPECIAL_(WAITSTATE_)?Qz', l))
    src = src.replace('@ Quetzal specials by their index in Quetzal\'s gSpecials (see docs/quetzal/SPECIALS.md)\n', '')
    src = src.replace('@ Generated by tools/quetzal_rip.', '@ Generated by tools/quetzal_port/import.py from tools/quetzal_rip\'s.')
    write(os.path.join(REPO, 'asm', 'macros', 'qz_event.inc'), src)


def main():
    if os.path.exists(os.path.join(REPO, 'data', 'maps', 'Johto_NewBarkTown')) and '--force' not in sys.argv:
        raise SystemExit('already imported (data/maps/Johto_NewBarkTown exists): --force rewrites every imported file')
    global IDS
    IDS = Ids()
    rename = import_tilesets()
    import_layouts(rename)
    maps, texts = import_maps()
    texts += import_shared_scripts(maps)
    write_johto_sections()
    write_constants(texts)
    write_specials(texts)
    write_macros()
    for c, where in sorted(IDS.missing.items()):
        print('  not in Kai: %s (%s)' % (c, ', '.join(sorted(where)[:4])))


if __name__ == '__main__':
    main()
