#!/usr/bin/env python3
"""Moemon Maker: the names the editor shows and saves overrides by (docs/MOEMON_MAKER.md "Names, not numbers").

Read from the game's own constant headers when the game is built, so the editor's lists are always this build's:
  include/constants/map_groups.h   (generated from data/maps/map_groups.json)  -> every map: MAP_ name, group, number
  data/maps/*/map.json             -> each map's people, signs and triggers by name (LOCALID_, look, script)
  and the named values in NAMED (songs, looks, movement types, trainer types, items, species, flags)

usage: gen_names.py OUT.h   (the Makefile: include/constants/maker_names.h, never committed)
       gen_names.py --tilesets OUT.h   (include/constants/maker_tilesets.h: every tileset with its metatile count by
                                        sizeof, included at the end of src/tilesets.c where the arrays are defined)
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8') as f:
        return f.read()


# Named values for the editor and for reading edits back (tag, headers, prefixes)
NAMED = [
    ('SONG', ['include/constants/songs.h'], ['MUS_', 'SE_']),
    ('GFX', ['include/constants/event_objects.h', 'include/constants/qz_port.h', 'include/constants/sinnoh_gfx.h',
             'include/mp.h'], ['OBJ_EVENT_GFX_']),
    ('MOVEMENT', ['include/constants/event_object_movement.h'], ['MOVEMENT_TYPE_']),
    ('TRAINERTYPE', ['include/constants/trainer_types.h'], ['TRAINER_TYPE_']),
    ('ITEM', ['include/constants/items.h'], ['ITEM_']),
    ('SPECIES', ['include/constants/species.h'], ['SPECIES_']),
    ('FLAG', ['include/constants/flags.h'], ['FLAG_']),
    ('VAR', ['include/constants/vars.h'], ['VAR_']),
    ('TRAINER', ['include/constants/opponents.h'], ['TRAINER_']),
    ('MAPSEC', ['include/constants/region_map_sections.h'], ['MAPSEC_']),
    ('MOVE', ['include/constants/moves.h'], ['MOVE_']),
    ('TYPE', ['include/constants/pokemon.h'], ['TYPE_']),   # (M3: the database)
    ('ABILITY', ['include/constants/abilities.h'], ['ABILITY_']),
]


def script_ops():
    """The script engine's command numbers (data/script_cmd_table.inc, in order): M2's event compiler
    (src/maker/maker_events.c) writes them"""
    out, n = [], 0
    for line in read('data/script_cmd_table.inc').splitlines():
        m = re.match(r'\s*script_cmd_table_entry\s+SCR_OP_(\w+)', line)
        if m:
            out.append('#define MAKER_OP_%s 0x%02X' % (m.group(1), n))
            n += 1
    return out


def specials():
    """Every special by name with its number and whether the script waits for it (data/specials.inc, Quetzal's in
    its place)"""
    names = []

    def walk(path):
        for line in read(path).splitlines():
            m = re.match(r'\s*def_special\s+(\w+)(?:\s*,\s*waitstate\s*=\s*(\d))?', line)
            if m:
                names.append((m.group(1), int(m.group(2) or 0)))
            m = re.match(r'\s*\.include\s+"([^"]+)"', line)
            if m:
                walk(m.group(1))
    walk('data/specials.inc')
    out = ['#define MAKER_SPECIAL_COUNT %d' % len(names),
           'static const struct MakerSpecialName sMakerSpecials[MAKER_SPECIAL_COUNT] = {']
    out += ['    {"%s", %d},' % (n, w) for n, w in names]
    out.append('};')
    return out


def maps():
    out = []
    group = None
    for line in read('include/constants/map_groups.h').splitlines():
        m = re.match(r'\s*// (gMapGroup_\w+)', line)
        if m:
            group = m.group(1)
            continue
        m = re.match(r'\s*(MAP_\w+)\s*=\s*\((\d+)\s*\|\s*\((\d+)\s*<<\s*8\)\)', line)
        if m:
            out.append((m.group(1), int(m.group(3)), int(m.group(2)), group))
    return out


def defines(path, prefixes):
    out = []
    seen = set()
    for line in read(path).splitlines():
        m = re.match(r'#define\s+((?:%s)\w*)\s+(\d+)\b' % '|'.join(prefixes), line)
        if m and m.group(1) not in seen:
            seen.add(m.group(1))
            out.append((m.group(1), int(m.group(2))))
    return out


def events():
    """Each map's people, signs and triggers by name, in map.json's order (the compiled arrays' order, mapjson): what
    the editor shows and what overrides are saved by (a person's LOCALID_, else its script)"""
    import glob
    import json
    objs, bgs, coords, maps_ = [], [], [], []
    for path in sorted(glob.glob(os.path.join(ROOT, 'data/maps/*/map.json'))):
        try:
            with open(path, encoding='utf-8') as f:
                m = json.load(f)
        except (OSError, ValueError):
            continue
        if 'id' not in m:
            continue
        o0, b0, c0 = len(objs), len(bgs), len(coords)
        for o in m.get('object_events', []):
            objs.append((o.get('local_id', ''), str(o.get('graphics_id', '')), str(o.get('script', ''))))
        for b in m.get('bg_events', []):
            bgs.append((str(b.get('type', '')), str(b.get('script', b.get('item', '')))))
        for c in m.get('coord_events', []):
            coords.append((str(c.get('var', '')), str(c.get('script', ''))))
        maps_.append((m['id'], o0, len(objs) - o0, b0, len(bgs) - b0, c0, len(coords) - c0))
    q = lambda v: '"%s"' % v.replace('\\', '\\\\').replace('"', '\\"')
    out = ['static const struct MakerObjName sMakerObjNames[] = {']
    out += ['    {%s, %s, %s},' % (q(a), q(b), q(c)) for a, b, c in objs] + ['    {"", "", ""},', '};']
    out += ['static const struct MakerBgName sMakerBgNames[] = {']
    out += ['    {%s, %s},' % (q(a), q(b)) for a, b in bgs] + ['    {"", ""},', '};']
    out += ['static const struct MakerCoordName sMakerCoordNames[] = {']
    out += ['    {%s, %s},' % (q(a), q(b)) for a, b in coords] + ['    {"", ""},', '};']
    out += ['#define MAKER_MAP_EVENTS_COUNT %d' % len(maps_),
            'static const struct MakerMapEventNames sMakerMapEventNames[MAKER_MAP_EVENTS_COUNT] = {']
    out += ['    {"%s", %d, %d, %d, %d, %d, %d},' % e for e in maps_] + ['};']
    return out


def write(out, text):
    try:
        if open(out, encoding='utf-8').read() == text:
            return   # (unchanged: nothing rebuilt)
    except OSError:
        pass
    with open(out, 'w', encoding='utf-8') as f:
        f.write(text)


def tilesets(out):
    lines = ['// Moemon Maker: generated by tools/maker/gen_names.py when the game is built. Do not edit or commit.',
             '// Included at the end of src/tilesets.c, where every tileset\'s arrays are defined (so sizeof counts them).',
             'static const struct MakerTilesetSize { const struct Tileset *tileset; unsigned short metatiles; const char *name; }'
             ' sMakerTilesetSizes[] = {']
    for path in ('src/data/tilesets/headers.h', 'src/data/tilesets/qz_headers.h'):
        name = None
        for line in read(path).splitlines():
            m = re.match(r'const struct Tileset (gTileset_\w+)', line)
            if m:
                name = m.group(1)
            m = re.match(r'\s*\.metatiles = (gMetatiles_\w+)', line)
            if m and name:
                lines.append('    {&%s, sizeof(%s) / 16, "%s"},' % (name, m.group(1), name[len('gTileset_'):]))
                name = None
    lines += ['};', '']
    write(out, '\n'.join(lines))


def main():
    if sys.argv[1] == '--tilesets':
        tilesets(sys.argv[2])
        return
    if sys.argv[1] == '--ops':   # include/constants/maker_ops.h: the script commands' numbers (maker_events.c)
        write(sys.argv[2], '\n'.join(['// Moemon Maker: generated by tools/maker/gen_names.py --ops. Do not edit or commit.',
                                      '#ifndef GUARD_CONSTANTS_MAKER_OPS_H', '#define GUARD_CONSTANTS_MAKER_OPS_H']
                                     + script_ops() + ['#endif', '']))
        return
    out = sys.argv[1]
    lines = ['// Moemon Maker: generated by tools/maker/gen_names.py when the game is built. Do not edit or commit.',
             '#ifndef GUARD_CONSTANTS_MAKER_NAMES_H', '#define GUARD_CONSTANTS_MAKER_NAMES_H', '',
             '// (its structs: include/maker.h; included only by src/maker/maker_names.c)']
    m = maps()
    lines.append('#define MAKER_MAP_COUNT %d' % len(m))
    lines.append('static const struct MakerMapName sMakerMapNames[MAKER_MAP_COUNT] = {')
    for name, g, n, group in m:
        lines.append('    {"%s", "%s", %d, %d},' % (name, group or '', g, n))
    lines.append('};')
    lines += events()
    # Named values, by the constants themselves (the compiler works each out: aliases and sums too), in the order the
    # headers define them. The file including this one includes the headers they come from (src/maker/maker_names.c).
    for tag, paths, prefixes in NAMED:
        names = []
        for path in paths:
            for line in read(path).splitlines():
                m = re.match(r'#define\s+((?:%s)\w*)\s+\S' % '|'.join(prefixes), line)
                if not m and tag == 'MAPSEC':   # (an enum: region_map_sections.h)
                    m = re.match(r'\s+((?:%s)\w*)\s*[,=]' % '|'.join(prefixes), line)
                if m and m.group(1) not in names and not m.group(1).endswith('_H'):
                    names.append(m.group(1))
        lines.append('#define MAKER_%s_COUNT %d' % (tag, len(names)))
        lines.append('static const struct MakerNamedValue s%sNames[MAKER_%s_COUNT] = {' % (tag.title(), tag))
        for n in names:
            lines.append('    {"%s", %s},' % (n, n))
        lines.append('};')
    lines += specials()
    lines += ['', '#endif // GUARD_CONSTANTS_MAKER_NAMES_H', '']
    write(out, '\n'.join(lines))


if __name__ == '__main__':
    main()
