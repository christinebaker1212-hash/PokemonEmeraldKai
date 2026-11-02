"""SINNOH's places: which of Platinum's map headers become the game's maps, and what each one is (tools/sinnoh/gen_maps.py
makes the maps, build_pack.py the 3D pack of their matrices; docs/SINNOH_HANDOFF.md).

Read from a pret/pokeplatinum checkout: the map header table (include/data/map_headers.h) and map matrix 0. Only names,
matrices, music, types and weather; nothing of Platinum's art or words.

A place is outdoor (its cells on matrix 0, the region) or a building (a matrix of its own, often shared: every POKéMON
CENTER has the same one). Left out, as the handoff says (Kai's own features replace them, or they need Nintendo's
servers): the Underground, the GTS / Global Terminal, PAL PARK, the Wi-Fi Plaza and the Communication Club's rooms,
Super Contests and the Poffin House.

Warps: every place's events file (res/field/events/<eventsArchiveID>.json) lists its warps; on matrix 0 their
coordinates are the region's tiles, in a building the matrix's own.
"""
import os
import re

from source import NONE

FILLER = 'MAP_HEADER_EVERYWHERE'
DROPPED = re.compile(r'GLOBAL_TERMINAL|PAL_PARK|WIFI|COMMUNICATION_CLUB|UNION_ROOM|'
                     r'MAP_HEADER_NOTHING|EVERYWHERE|UNUSED|UNKNOWN')   # (the last two: leftovers Platinum never shows)


def header_table(root):
    """{MAP_HEADER_x: {field: value}} for every header (the fields as written: area_data_006, map_matrix_128...)"""
    text = open(os.path.join(root, 'include/data/map_headers.h')).read()
    out = {}
    for name, body in re.findall(r'\[(MAP_HEADER_\w+)\] = \{(.*?)\n    \}', text, re.S):
        out[name] = dict(re.findall(r'\.(\w+) = (\w+)', body))
    return out


def matrix_number(field):
    return int(field.rsplit('_', 1)[1])


def places(src, root):
    """Every kept place: {'header', 'matrix' (number), 'cells' [(x, z)...] on it, 'outdoor', 'fields'}"""
    table = header_table(root)
    m0 = src.matrix(0)
    outdoor = {}
    for z in range(m0.height):
        for x in range(m0.width):
            lid, header, _ = m0.cell(x, z)
            if lid == NONE or not header or header == FILLER:
                continue
            outdoor.setdefault(header, []).append((x, z))
    out = []
    for header, fields in table.items():
        if DROPPED.search(header) or 'mapMatrixID' not in fields:
            continue
        n = matrix_number(fields['mapMatrixID'])
        if n == 0:
            if header not in outdoor:
                continue
            out.append({'header': header, 'matrix': 0, 'cells': outdoor[header], 'outdoor': True, 'fields': fields})
            continue
        m = src.matrix(n)
        cells = [(x, z) for z in range(m.height) for x in range(m.width) if m.cell(x, z)[0] != NONE]
        if cells:
            out.append({'header': header, 'matrix': n, 'cells': cells, 'outdoor': False, 'fields': fields})
    return out


def events(root, place):
    """The place's events file ({'warp_events': [...], 'object_events', 'bg_events', 'coord_events'}), or empty lists"""
    import json
    path = os.path.join(root, 'res/field/events', place['fields'].get('eventsArchiveID', '') + '.json')
    if not os.path.exists(path):
        return {'warp_events': [], 'object_events': [], 'bg_events': [], 'coord_events': []}
    return json.load(open(path))
