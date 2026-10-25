#!/usr/bin/env python3
"""Rip Moemon Quetzal's Kanto and Johto into decomp-format files.

    python3 tools/quetzal_rip/rip.py QUETZAL.gba [--out quetzal_rip] [--crystal DIR] [--firered DIR]

pokecrystal / pokefirered checkouts are only used to name the maps (default: ../pokecrystal,
../pokefirered next to the repo, or $POKECRYSTAL / $POKEFIRERED).
"""
import argparse
import json
import os
import re
import shutil
from collections import defaultdict

from qrom import REPO, Rom, Names, decode
from maps import (MAP_TYPES, WEATHER, WEATHER_EXTRA, BATTLE_SCENES, DIRS, MAP_SCRIPT_TYPES, BG_KINDS)
from analyze import load, walk, map_entries
from scripts import Emitter, TERMINAL, COND, text_lines, flag_name, var_or_value, SONGS, NUM_VANILLA_SONGS
import naming
from tilesets import rip_tilesets
from sprites import rip_sprites
from data import write_all, mapsecs
from trainer_pics import rip_trainer_pics
import consts


def kai_list(path, pattern):
    out = {}
    for m in re.finditer(pattern, open(os.path.join(REPO, path)).read()):
        out.setdefault(int(m.group(2), 0), m.group(1))
    return out


MOVEMENT_TYPES = kai_list('include/constants/event_object_movement.h', r'#define (MOVEMENT_TYPE_\w+)\s+(0x[0-9A-Fa-f]+|\d+)')
TRAINER_TYPES = {0: 'TRAINER_TYPE_NONE', 1: 'TRAINER_TYPE_NORMAL', 2: 'TRAINER_TYPE_SEE_ALL_DIRECTIONS', 3: 'TRAINER_TYPE_BURIED'}
KAI_MAPSECS = [s['id'] for s in json.load(open(os.path.join(REPO, 'src/data/region_map/region_map_sections.json')))['map_sections']]


def upper_snake(name):
    s = re.sub(r'(?<=[a-z])(?=[A-Z])|(?<=[0-9])(?=[A-Z][a-z])', '_', name)
    return re.sub(r'_+', '_', s.replace('-', '_')).upper()


class Ripper:
    def __init__(self, rom_path, out, crystal, firered):
        self.rom, self.names, self.maps = load(rom_path)
        self.out = out
        self.by_key = {(m['group'], m['num']): m for m in self.maps}
        self.targets = [m for m in self.maps if m['region'] in ('johto', 'kanto')]
        self.dis = walk(self.rom, self.names, self.maps, map_const=self.map_const)
        self.dis.finish()
        self.crystal = crystal
        self.firered = firered
        self.name_maps()

    # ------------------------------------------------------------ naming -------
    def map_texts(self):
        mt = defaultdict(str)
        for a, k in self.dis.kind.items():
            if k == 'text':
                s, _ = decode(self.rom, a)
                for o in self.dis.owners.get(a, ()):
                    mt[o] += ' ' + s
        return mt

    def name_maps(self):
        kn, self.frlg_dropped = naming.kanto_names(self.maps, self.firered)
        # the alignment can't tell which of two identical-section maps was dropped; checked by hand
        self.frlg_dropped = ['SevenIsland_UnusedHouse' if x == 'SevenIsland_House_Room1' else x for x in self.frlg_dropped]
        jn = naming.johto_names(self.maps, self.map_texts(), self.crystal)
        used = set()
        counters = defaultdict(int)
        self.overrides = naming.overrides()
        for m in self.targets:
            k = (m['group'], m['num'])
            sec = naming.camel(m['mapsec_name'])
            if m['region'] == 'kanto':
                base = kn.get(k)
                m['source'] = {'pokefirered': base} if base else {'pokefirered': None, 'note': 'new map in Quetzal'}
                if not base:
                    counters[sec] += 1
                    base = '%s_Extra%d' % (sec, counters[sec])
                name = 'Kanto_' + base
            else:
                hit = jn.get(k)
                if hit:
                    base = hit[0]
                    m['source'] = {'pokecrystal': hit[0], 'text_match': hit[1]}
                else:
                    role = naming.role_of(m, self.dis)
                    counters[(sec, role)] += 1
                    base = '%s_%s%d' % (sec, role or 'Area', counters[(sec, role)])
                    m['source'] = {'pokecrystal': None}
                name = 'Johto_' + base
            ov = self.overrides.get(k) or self.overrides.get(('name', name))
            if ov:
                if m['region'] == 'kanto' and m['source'].get('pokefirered'):
                    m['source'] = {'pokefirered': ov[len('Kanto_'):], 'note': 'corrected by hand (kanto_frlg_flags.json)'}
                elif m['region'] == 'kanto':
                    m['source']['proposed_name'] = ov
                else:
                    m['source']['renamed_from'] = name
                name = ov
            while name in used:
                name += '_2'
            used.add(name)
            m['name'] = name
            m['const'] = 'MAP_' + upper_snake(name)

    def map_const(self, gn):
        m = self.by_key.get(tuple(gn))
        if m is None:
            if tuple(gn) == (0x7F, 0x7F):
                return 'MAP_DYNAMIC'
            if tuple(gn) == (0xFF, 0xFF) or tuple(gn) == (0, 0) and False:
                return 'MAP_UNDEFINED'
            return 'MAP_GROUP_%d_NUM_%d' % tuple(gn)
        if 'const' in m:
            return m['const']
        return self.hoenn_const(m['group'], m['num'])

    def hoenn_const(self, g, n):
        """Quetzal's Hoenn groups 0-33 have stock Emerald's sizes and order, so Kai's names apply."""
        if not hasattr(self, '_kai_groups'):
            mg = json.load(open(os.path.join(REPO, 'data/maps/map_groups.json')))
            self._kai_groups = [mg[x] for x in mg['group_order']]
        try:
            name = self._kai_groups[g][n]
        except IndexError:
            return 'MAP_HOENN_%d_%d' % (g, n)
        return json.load(open(os.path.join(REPO, 'data/maps', name, 'map.json')))['id']

    # ------------------------------------------------------------ labels -------
    def file_of(self, a):
        owners = self.dis.owners.get(a)
        if not owners:
            return 'scripts/unowned.inc'
        if len(owners) == 1:
            return 'maps/%s/scripts.inc' % self.by_key[next(iter(owners))]['name']
        regions = {self.by_key[o]['region'] for o in owners}
        if len(regions) == 1:
            return 'scripts/%s_shared.inc' % regions.pop()
        return 'scripts/shared.inc'

    def label(self, a):
        if a in self.labels:
            return self.labels[a]
        f = self.file_of(a)
        k = self.dis.kind[a]
        if f.startswith('maps/'):
            prefix = f.split('/')[1]
        else:
            prefix = 'Quetzal'
        kind = {'script': 'EventScript', 'text': 'Text', 'movement': 'Movement', 'mart': 'Mart',
                'braille': 'Braille'}[k]
        lab = '%s_%s_%X' % (prefix, kind, a + 0x08000000)
        self.labels[a] = lab
        return lab

    # ------------------------------------------------------------ output -------
    def run(self):
        if os.path.exists(self.out):
            for sub in ('maps', 'layouts', 'scripts'):
                shutil.rmtree(os.path.join(self.out, sub), ignore_errors=True)
        os.makedirs(self.out, exist_ok=True)
        self.labels = {}
        self.em = Emitter(self.dis, self.label, self.names)
        files = defaultdict(list)       # file -> [(addr, kind)]
        for a, k in self.dis.kind.items():
            files[self.file_of(a)].append(a)
        entry_notes = self.entry_notes()
        for f, addrs in files.items():
            self.write_inc(f, sorted(addrs), entry_notes)
        self.write_map_jsons()
        shutil.rmtree(os.path.join(self.out, 'tilesets'), ignore_errors=True)
        self.tileset_names = rip_tilesets(self.rom, self.maps, self.targets, self.out, self.firered, lambda m: m['name'])
        self.write_layouts()
        shutil.rmtree(os.path.join(self.out, 'object_events'), ignore_errors=True)
        self.sprites = rip_sprites(self.rom, self.targets, self.out, [('kai', REPO), ('pokefirered', self.firered)])
        self.trainers, self.wild = write_all(self.out, self.rom, self.names, self.map_const, self.dis.trainers,
                                             (34, 35, 36, 37, 38, 39))
        with open(os.path.join(self.out, 'data', 'region_map_sections.json'), 'w', encoding='utf-8') as fh:
            json.dump(mapsecs(self.rom, {(m['region'], m['mapsec']) for m in self.targets}), fh, indent=1, ensure_ascii=False)
            fh.write('\n')
        shutil.rmtree(os.path.join(self.out, 'trainer_pics'), ignore_errors=True)
        pics = sorted({t['front_pic_id'] for t in self.trainers})
        self.trainer_pics = rip_trainer_pics(self.rom, pics, os.path.join(self.out, 'trainer_pics'))
        consts.write(self)
        from overview import overview
        os.makedirs(os.path.join(self.out, 'previews'), exist_ok=True)
        overview(self.rom, self.maps, (34, 123), 0.25, os.path.join(self.out, 'previews', 'johto_overview.png'))
        overview(self.rom, self.maps, (37, 57), 0.25, os.path.join(self.out, 'previews', 'kanto_overview.png'))
        self.idmap_stats = consts.id_map(self)
        self.write_index()

    def entry_notes(self):
        notes = defaultdict(list)
        for m in self.targets:
            ev = m['events']
            for i, o in enumerate(ev['objects']):
                if o['script'] is not None:
                    notes[o['script']].append('%s object %d (local id %d, gfx 0x%X) at %d,%d' % (
                        m['name'], i + 1, o['local_id'], o['graphics_id'], o['x'], o['y']))
            for i, c in enumerate(ev['coords']):
                if c['script'] is not None:
                    notes[c['script']].append('%s trigger at %d,%d (%s == %d)' % (
                        m['name'], c['x'], c['y'], var_or_value(c['var']), c['var_value']))
            for i, b in enumerate(ev['bgs']):
                if b.get('script') is not None:
                    notes[b['script']].append('%s sign / bg event at %d,%d' % (m['name'], b['x'], b['y']))
        return notes

    def write_inc(self, f, addrs, entry_notes):
        path = os.path.join(self.out, f)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        lines = []
        mapname = f.split('/')[1] if f.startswith('maps/') else None
        if mapname:
            m = next(x for x in self.targets if x['name'] == mapname)
            lines += self.map_scripts_block(m)
        rom = self.rom
        scripts = [a for a in addrs if self.dis.kind[a] == 'script']
        data = [a for a in addrs if self.dis.kind[a] in ('movement', 'mart', 'braille')]
        texts = [a for a in addrs if self.dis.kind[a] == 'text']
        for i, a in enumerate(scripts):
            pcs, nxt = self.dis.blocks[a]
            lines.append('')
            for n in entry_notes.get(a, []):
                lines.append('@ %s' % n)
            lines.append('%s::' % self.label(a))
            lines += self.block_lines(pcs)
            if a in self.dis.runs_into_data:
                lines.append('\t@ (the ROM bytes after this point are data used elsewhere; the script never reaches them)')
            if nxt is not None:
                follows = i + 1 < len(scripts) and scripts[i + 1] == nxt
                if not follows:
                    lines.append('\tgoto %s  @ falls through in the ROM' % self.label(nxt))
        for a in data:
            k = self.dis.kind[a]
            lines.append('')
            lines.append('%s:' % self.label(a))
            if k == 'movement':
                for mv in self.dis.movement(a):
                    lines.append('\t' + mv)
            elif k == 'mart':
                for v in self.dis.mart(a):
                    lines.append('\t.2byte %s' % (self.names.item_name(v) if v else 'ITEM_NONE'))
            else:
                s, _ = decode(rom, a)
                lines.append('\t@ braille')
                lines += text_lines(s)
        for a in texts:
            s, _ = decode(rom, a)
            lines.append('')
            lines.append('%s::' % self.label(a))
            lines += text_lines(s)
        with open(path, 'w', encoding='utf-8') as fh:
            fh.write('\n'.join(lines).lstrip('\n') + '\n')

    def block_lines(self, pcs):
        """Instructions of one block, with Kai's shorthand macros folded in (same bytes)."""
        ins = [self.dis.insn[pc] for pc in pcs]
        out = []
        i = 0
        em = self.em
        while i < len(ins):
            name, args = ins[i][0], ins[i][1]
            nxt = ins[i + 1] if i + 1 < len(ins) else None
            if name == 'loadword' and args[0][1] == 0 and args[1][0] == 'T' and nxt and nxt[0] == 'callstd' \
                    and 2 <= nxt[1][0][1] <= 6 or (name == 'loadword' and args[0][1] == 0 and args[1][0] == 'T'
                                                   and nxt and nxt[0] == 'callstd' and nxt[1][0][1] in (9, 10)):
                out.append('\tmsgbox %s, %s' % (em.arg(*args[1]), em.arg(*nxt[1][0])))
                i += 2
                continue
            if name == 'checkflag' and nxt and nxt[0] in ('goto_if', 'call_if') and nxt[1][0][1] in (0, 1):
                cond = 'set' if nxt[1][0][1] == 1 else 'unset'   # TRUE / FALSE
                out.append('\t%s_%s %s, %s' % (nxt[0], cond, em.arg(*args[0]), em.arg(*nxt[1][1])))
                i += 2
                continue
            if name == 'compare_var_to_value' and nxt and nxt[0] in ('goto_if', 'call_if') and nxt[1][0][1] in COND \
                    and args[1][1] < 0x4000:   # Kai's compare picks compare_var_to_var for var ids
                out.append('\t%s_%s %s, %s, %s' % (nxt[0], COND[nxt[1][0][1]].lower(), em.arg(*args[0]),
                                                   '%d' % args[1][1], em.arg(*nxt[1][1])))
                i += 2
                continue
            out.append(em.insn(name, args))
            i += 1
        return out

    def map_scripts_block(self, m):
        lines = ['%s_MapScripts::' % m['name']]
        tables = []
        for t, a, table in m['map_scripts']:
            tn = MAP_SCRIPT_TYPES.get(t, str(t))
            if table is None:
                lines.append('\tmap_script %s, %s' % (tn, self.label(a) if a is not None else '0'))
            else:
                lab = '%s_%s_%X' % (m['name'], 'OnFrame' if t == 2 else 'OnWarp', a + 0x08000000)
                lines.append('\tmap_script %s, %s' % (tn, lab))
                tables.append((lab, table))
        lines.append('\t.byte 0')
        for lab, table in tables:
            lines.append('')
            lines.append('%s:' % lab)
            for var, val, s in table:
                lines.append('\tmap_script_2 %s, %d, %s' % (var_or_value(var), val, self.label(s)))
            lines.append('\t.2byte 0')
        return lines

    # ------------------------------------------------------------ map.json -----
    def layout_name(self, m):
        return self.layout_names[m['layout_addr']]

    def write_map_jsons(self):
        self.layout_names = {}
        lay_used = set()
        for m in self.targets:
            la = m['layout_addr']
            if la not in self.layout_names:
                n = m['name'] + '_Layout'
                self.layout_names[la] = n
        for m in self.targets:
            j = {
                'id': m['const'], 'name': m['name'],
                'layout': 'LAYOUT_' + upper_snake(self.layout_names[m['layout_addr']][:-7]),
                'music': SONGS.get(m['music'], 'MUS_QZ_%d' % m['music']) if m['music'] < NUM_VANILLA_SONGS else 'MUS_QZ_%d' % m['music'],
                'region_map_section': self.mapsec_const(m),
                'requires_flash': bool(m['cave']),
                'weather': WEATHER_EXTRA.get(m['weather'], WEATHER[m['weather']] if m['weather'] < len(WEATHER) else 'WEATHER_QZ_%d' % m['weather']),
                'map_type': MAP_TYPES[m['map_type']] if m['map_type'] < len(MAP_TYPES) else str(m['map_type']),
                'allow_cycling': m['allow_cycling'], 'allow_escaping': m['allow_escaping'],
                'allow_running': m['allow_running'], 'show_map_name': m['show_map_name'],
                'battle_scene': BATTLE_SCENES[m['battle_type']] if m['battle_type'] < len(BATTLE_SCENES) else str(m['battle_type']),
                'connections': [{'map': self.map_const(c['map']), 'offset': c['offset'],
                                 'direction': DIRS.get(c['direction'], str(c['direction']))} for c in m['connections']] or None,
                'object_events': [], 'warp_events': [], 'coord_events': [], 'bg_events': [],
            }
            ev = m['events']
            for o in ev['objects']:
                j['object_events'].append({
                    'local_id': str(o['local_id']),
                    'graphics_id': 'OBJ_EVENT_GFX_QZ_%d' % o['graphics_id'],
                    'x': o['x'], 'y': o['y'], 'elevation': o['elevation'],
                    'movement_type': MOVEMENT_TYPES.get(o['movement_type'], 'MOVEMENT_TYPE_QZ_0x%X' % o['movement_type']),
                    'movement_range_x': o['movement_range_x'], 'movement_range_y': o['movement_range_y'],
                    'trainer_type': TRAINER_TYPES.get(o['trainer_type'], str(o['trainer_type'])),
                    'trainer_sight_or_berry_tree_id': str(o['trainer_sight_or_berry_tree_id']),
                    'script': self.label(o['script']) if o['script'] is not None else '0x0',
                    'flag': flag_name(o['flag']) if o['flag'] else '0'})
                if o['kind'] or o['byte11'] or o['pad']:
                    j['object_events'][-1]['quetzal_extra'] = {'kind': o['kind'], 'byte11': o['byte11'], 'pad': o['pad']}
            for w in ev['warps']:
                j['warp_events'].append({'x': w['x'], 'y': w['y'], 'elevation': w['elevation'],
                                         'dest_map': self.map_const(w['dest_map']), 'dest_warp_id': str(w['dest_warp_id'])})
            for c in ev['coords']:
                j['coord_events'].append({'type': 'trigger', 'x': c['x'], 'y': c['y'], 'elevation': c['elevation'],
                                          'var': var_or_value(c['var']), 'var_value': str(c['var_value']),
                                          'script': self.label(c['script']) if c['script'] is not None else '0x0'})
            for b in ev['bgs']:
                if b['kind'] == 7:
                    j['bg_events'].append({'type': 'hidden_item', 'x': b['x'], 'y': b['y'], 'elevation': b['elevation'],
                                           'item': self.names.item_name(b['item']),
                                           # Emerald stores flag - FLAG_HIDDEN_ITEMS_START; Kanto's ids restart at 0
                                           'flag': 'FLAG_QZ_%s_HIDDEN_ITEM_%d' % (m['region'].upper(), b['flag'])})
                elif b['kind'] == 8:
                    j['bg_events'].append({'type': 'secret_base', 'x': b['x'], 'y': b['y'], 'elevation': b['elevation'],
                                           'secret_base_id': str(b['secret_base_id'])})
                else:
                    j['bg_events'].append({'type': 'sign', 'x': b['x'], 'y': b['y'], 'elevation': b['elevation'],
                                           'player_facing_dir': BG_KINDS.get(b['kind'], str(b['kind'])),
                                           'script': self.label(b['script']) if b.get('script') is not None else '0x0'})
            j['quetzal'] = {
                'group': m['group'], 'num': m['num'], 'region': m['region'],
                'header': '0x%X' % (m['header_addr'] + 0x08000000),
                'layout_id': m['layout_id'], 'mapsec_index': m['mapsec'], 'mapsec_name': m['mapsec_name'],
                'music_id': m['music'], 'weather_id': m['weather'], 'header_filler': m['filler'],
                'flags_high_bits': m['flags_extra'], 'source': m.get('source'),
            }
            d = os.path.join(self.out, 'maps', m['name'])
            os.makedirs(d, exist_ok=True)
            with open(os.path.join(d, 'map.json'), 'w', encoding='utf-8') as fh:
                json.dump(j, fh, indent=2, ensure_ascii=False)
                fh.write('\n')

    def mapsec_const(self, m):
        if m['region'] == 'johto':
            return 'MAPSEC_JOHTO_' + upper_snake(naming.camel(m['mapsec_name']))
        if m['mapsec'] < len(KAI_MAPSECS):
            return KAI_MAPSECS[m['mapsec']]
        return 'MAPSEC_QZ_%d' % m['mapsec']

    # ------------------------------------------------------------ layouts ------
    def write_layouts(self):
        rom = self.rom
        entries = []
        done = set()
        for m in self.targets:
            L = m['layout']
            if L['addr'] in done:
                continue
            done.add(L['addr'])
            n = self.layout_names[L['addr']][:-7]
            d = os.path.join(self.out, 'layouts', n)
            os.makedirs(d, exist_ok=True)
            w, h = L['width'], L['height']
            bw, bh = L['border_width'] or 2, L['border_height'] or 2
            open(os.path.join(d, 'map.bin'), 'wb').write(rom.d[L['blockdata']:L['blockdata'] + 2 * w * h])
            open(os.path.join(d, 'border.bin'), 'wb').write(rom.d[L['border']:L['border'] + 2 * bw * bh])
            entries.append({'id': 'LAYOUT_' + upper_snake(n), 'name': n + '_Layout', 'width': w, 'height': h,
                            'border_width': bw, 'border_height': bh,
                            'primary_tileset': 'gTileset_' + self.tileset_names[L['primary']],
                            'secondary_tileset': 'gTileset_' + self.tileset_names[L['secondary']],
                            'border_filepath': 'quetzal_rip/layouts/%s/border.bin' % n,
                            'blockdata_filepath': 'quetzal_rip/layouts/%s/map.bin' % n,
                            'quetzal_layout_id': m['layout_id'], 'quetzal_addr': '0x%X' % (L['addr'] + 0x08000000)})
        with open(os.path.join(self.out, 'layouts', 'layouts.json'), 'w') as fh:
            json.dump({'layouts_table_label': 'gMapLayouts_Quetzal', 'layouts': entries}, fh, indent=2)
            fh.write('\n')

    # ------------------------------------------------------------ index --------
    def write_index(self):
        groups = defaultdict(list)
        for m in self.targets:
            groups['gMapGroup_Quetzal%d_%s' % (m['group'], m['region'].capitalize())].append(m['name'])
        order = sorted(groups, key=lambda g: int(re.search(r'Quetzal(\d+)', g).group(1)))
        j = {'group_order': order}
        j.update(groups)
        with open(os.path.join(self.out, 'maps', 'map_groups.json'), 'w') as fh:
            json.dump(j, fh, indent=2)
            fh.write('\n')
        rows = []
        for m in self.targets:
            rows.append({'name': m['name'], 'const': m['const'], 'group': m['group'], 'num': m['num'],
                         'region': m['region'], 'mapsec': m['mapsec_name'], 'source': m.get('source'),
                         'width': m['layout']['width'], 'height': m['layout']['height']})
        with open(os.path.join(self.out, 'maps', 'index.json'), 'w', encoding='utf-8') as fh:
            json.dump({'maps': rows, 'frlg_maps_not_in_quetzal': self.frlg_dropped}, fh, indent=1, ensure_ascii=False)
            fh.write('\n')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('rom')
    ap.add_argument('--out', default=os.path.join(REPO, 'quetzal_rip'))
    ap.add_argument('--crystal', default=os.environ.get('POKECRYSTAL', os.path.join(REPO, '..', 'pokecrystal')))
    ap.add_argument('--firered', default=os.environ.get('POKEFIRERED', os.path.join(REPO, '..', 'pokefirered')))
    a = ap.parse_args()
    r = Ripper(a.rom, a.out, a.crystal, a.firered)
    r.run()
    print('maps:', len(r.targets), 'labels:', len(r.labels), 'problems:', r.dis.problems)
    print('ids used (total, missing in Kai):', r.idmap_stats)


if __name__ == '__main__':
    main()
