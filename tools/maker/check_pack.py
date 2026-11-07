#!/usr/bin/env python3
"""Moemon Maker: the repository's MoemonMaker/ checked against this checkout's game (docs/MOEMON_MAKER.md "Every release
checks every pack"), by .github/workflows/maker-pack.yml before the pack is published (and after every game release).

Errors (the job fails, nothing is published): a file the game can't read (a line it would leave out), a map, look,
movement type, item, species or song that doesn't exist, a music file that isn't a .kaisong.
Warnings (listed; the game is safe: it leaves such an edit out and the editor's PROBLEMS shows it): an edit whose
"was" value the game no longer has (a later update changed the same tile or person).

usage: tools/maker/check_pack.py [MOEMONMAKER_DIR]
"""
import glob
import json
import os
import re
import struct
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
SRC = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, 'MoemonMaker')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


def defines(paths, prefix):
    names = set()
    for p in paths:
        for m in re.finditer(r'#define\s+(%s\w*)\s' % prefix, read(p)):
            names.add(m.group(1))
    return names


GFX = defines(['include/constants/event_objects.h', 'include/constants/qz_port.h', 'include/constants/sinnoh_gfx.h',
               'include/mp.h'], 'OBJ_EVENT_GFX_')
MOVEMENT = defines(['include/constants/event_object_movement.h'], 'MOVEMENT_TYPE_')
TRAINERTYPE = defines(['include/constants/trainer_types.h'], 'TRAINER_TYPE_')
ITEMS = defines(['include/constants/items.h'], 'ITEM_')
SPECIES = defines(['include/constants/species.h'], 'SPECIES_')
FLAGS = defines(['include/constants/flags.h'], 'FLAG_')
SONGS = defines(['include/constants/songs.h'], 'MUS_')
MAPS = {}
for path in glob.glob(os.path.join(ROOT, 'data/maps/*/map.json')):
    with open(path, encoding='utf-8') as f:
        m = json.load(f)
    if 'id' in m:
        MAPS[m['id']] = (m, os.path.dirname(path))
LAYOUTS = {l['id']: l for l in json.load(open(os.path.join(ROOT, 'data/layouts/layouts.json')))['layouts'] if 'id' in l}
TRACKS = {os.path.basename(p)[:-8] for p in glob.glob(os.path.join(SRC, 'music', '*.kaisong'))}

# events (M2: src/maker/maker_events.c): the commands an event's indented lines may hold, a page's conditions, and the
# game's own script macros a `raw` line assembles (src/maker/maker_raw.c, from the same asm/macros files)
EVENT_COMMANDS = {'text', 'yesno', 'choice', 'if', 'else', 'end', 'stop', 'setflag', 'clearflag', 'self', 'setvar',
                  'addvar', 'subvar', 'giveitem', 'takeitem', 'givemon', 'money', 'heal', 'lua', 'special', 'battle',
                  'wildbattle', 'music', 'fanfare', 'sound', 'wait', 'fade', 'move', 'remove', 'warp', 'lock',
                  'release', 'face', 'log', 'raw', 'label'}
PAGE_KEYS = {'trigger', 'flag', 'noflag', 'self', 'noself', 'var', 'item', 'badges', 'time'}
MACROS = set()
for p in ('asm/macros/event.inc', 'asm/macros/qz_event.inc', 'asm/macros/movement.inc'):
    if os.path.exists(os.path.join(ROOT, p)):
        MACROS |= set(re.findall(r'^\s*\.macro\s+(\w+)', read(p), re.M))

errors, warnings = [], []
TABLES = {'look': GFX, 'movement': MOVEMENT, 'trainer': TRAINERTYPE, 'flag': FLAGS | {'0'}, 'item': ITEMS}
PERSON = {'x', 'y', 'elevation', 'look', 'movement', 'range_x', 'range_y', 'trainer', 'sight', 'flag', 'removed'}


def value_ok(prop, v):
    if prop in TABLES and not re.match(r'^-?\d+$|^0x', v):
        return v in TABLES[prop]
    if prop == 'dest':
        return ':' in v and v.split(':')[0] in MAPS
    if prop == 'map':
        return v in MAPS
    if prop == 'direction':
        return v in ('up', 'down', 'left', 'right', 'dive', 'emerge')
    return bool(re.match(r'^-?(\d+|0x[0-9A-Fa-f]+)$', v))


def blocks_of(map_id):
    """the map's own blocks (SINNOH's come from the 3D pack: not checked here)"""
    m, _ = MAPS[map_id]
    l = LAYOUTS.get(m.get('layout'))
    if not l or map_id.startswith('MAP_SINNOH_'):
        return None, 0, 0
    try:
        data = open(os.path.join(ROOT, l['blockdata_filepath']), 'rb').read()
    except OSError:
        return None, 0, 0
    return data, l['width'], l['height']


for path in sorted(glob.glob(os.path.join(SRC, 'base', '*.txt'))):
    name = os.path.basename(path)
    lines = open(path, encoding='utf-8').read().splitlines()
    map_id = None
    blocks, w, h = None, 0, 0
    in_event = False
    for n, line in enumerate(lines, 1):
        where = '%s line %d' % (name, n)
        if not line.strip() or line.startswith('#'):
            continue
        word = line.split()
        if line[0] in ' \t':   # an event's page or command (the indented lines after its "event" line)
            if not in_event:
                errors.append('%s: an indented line outside an event: %s' % (where, line.strip()))
            elif word[0] == 'page':
                for kv in word[1:]:
                    if kv.split('=')[0] not in PAGE_KEYS:
                        errors.append('%s: a page condition the game doesn\'t know: %s' % (where, kv))
            elif word[0] not in EVENT_COMMANDS:
                errors.append('%s: an event command the game doesn\'t know: %s' % (where, word[0]))
            elif word[0] == 'raw' and (len(word) < 2 or (MACROS and word[1].rstrip(',') not in MACROS)):
                errors.append('%s: a script command the game doesn\'t have: %s' % (where, line.strip()))
            continue
        in_event = False
        if word[0] == 'map':
            map_id = word[1]
            if map_id not in MAPS:
                errors.append('%s: no map %s' % (where, map_id))
                break
            blocks, w, h = blocks_of(map_id)
        elif map_id is None:
            errors.append('%s: before its "map" line' % where)
        elif word[0] == 'block' and len(word) == 5:
            x, y, was = int(word[1]), int(word[2]), int(word[3], 0)
            if blocks is not None and (x >= w or y >= h or struct.unpack_from('<H', blocks, 2 * (y * w + x))[0] != was):
                warnings.append('%s: tile %d, %d was changed by the game since (left out until the editor resolves it)' % (where, x, y))
        elif word[0] in ('person', 'warp', 'sign', 'trigger', 'connection') and len(word) == 5:
            if word[0] == 'person' and word[2] not in PERSON:
                errors.append('%s: a person has no "%s"' % (where, word[2]))
            elif not value_ok(word[2], word[3]) or not value_ok(word[2], word[4]):
                errors.append('%s: a value the game doesn\'t know: %s' % (where, line))
            elif word[0] == 'person' and word[1].startswith('LOCALID_'):
                people = [o for o in MAPS[map_id][0].get('object_events', []) if o.get('local_id') == word[1]]
                if not people:
                    warnings.append('%s: %s isn\'t on the map any more (left out)' % (where, word[1]))
        elif word[0] == 'wild' and len(word) == 6:
            if word[1] not in ('land', 'water', 'rock', 'fish') or word[3] not in ('species', 'min', 'max'):
                errors.append('%s: not understood: %s' % (where, line))
            elif word[3] == 'species' and (word[4] not in SPECIES or word[5] not in SPECIES):
                errors.append('%s: a species the game doesn\'t know: %s' % (where, line))
        elif word[0] == 'event' and len(word) == 3 and word[2].startswith('on='):
            on = word[2][3:]
            kind, _, key = on.partition(':')
            if not (on == 'map' or (kind in ('person', 'sign', 'trigger') and key)
                    or (kind == 'add' and re.match(r'^\d+$', key))):
                errors.append('%s: an event on something the game doesn\'t know: %s' % (where, on))
            in_event = True
        elif word[0] == 'music' and len(word) == 3:
            for v in word[1:]:
                if v not in SONGS and v not in TRACKS:
                    errors.append('%s: no song or track %s' % (where, v))
        elif word[0] == 'add' and len(word) >= 3:
            for kv in word[3:]:
                k, _, v = kv.partition('=')
                if k == 'script':
                    continue
                if not value_ok(k, v):
                    errors.append('%s: a value the game doesn\'t know: %s' % (where, kv))
        else:
            errors.append('%s: not understood: %s' % (where, line))

for path in glob.glob(os.path.join(SRC, 'music', '*')):
    if not path.endswith('.kaisong'):
        errors.append('music/%s: only .kaisong files belong here' % os.path.basename(path))
    elif open(path, 'rb').read(8) != b'KAISONG\0':
        errors.append('music/%s: not a .kaisong file' % os.path.basename(path))

for w_ in warnings:
    print('warning: ' + w_)
for e in errors:
    print('error: ' + e)
print('%d files, %d errors, %d warnings' % (len(glob.glob(os.path.join(SRC, 'base', '*.txt'))), len(errors), len(warnings)))
sys.exit(1 if errors else 0)
