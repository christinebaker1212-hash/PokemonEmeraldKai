#!/usr/bin/env python3
"""ORRE's maps as the field engine's maps (docs/ORRE_DECISIONS.md): each map of tools/orre/maps.py as a map of
gMapGroup_Orre with map.json, scripts.inc, a layout and their places in map_groups.json / layouts.json, Orre's
script file (data/event_scripts_orre.s) and the 3D engine's tables of them (src/orre/orre_maps.h: the maps, and
where each entry of a room lands and faces). Read from the disc: each map's size in tiles (its collision,
tools/orre/grid.py), and where its doors, stairs, elevators and exits lead (the game's own tables:
tools/orre/maps.py, colo_rooms.py); nothing of Colosseum's art or words is written here. The layouts' blocks are placeholders (every tile blocked): the game takes each tile's collision from
ORRE's pack when it loads the map (src/orre/o3d.c O3D_LayoutBlocks).

    python3 -I tools/orre/gen_maps.py UNPACKED_DIR      (run from the repository; safe to run again)
"""
import json
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import grid  # noqa: E402
import maps as orre_maps  # noqa: E402
import colo_data  # noqa: E402

MAPSEC_FIRST = 0x1C0
ISO_FILES = os.environ.get("ORRE_ISO_FILES", os.path.expanduser("~/.cache/orre/iso"))   # (the disc's archives: ids)   # ORRE's map sections, after SINNOH's (0x140 to 0x1BA)


def mapsec(name):
    return "MAPSEC_ORRE_" + "".join(c if c.isalnum() else "_" for c in name.upper()).strip("_").replace("__", "_")

GROUP = "gMapGroup_Orre"


def facing(angle):
    """The way an entry's angle faces (Colosseum's degrees about Y: 0 looks down +Z, the grid's south)."""
    return ("DIR_SOUTH", "DIR_EAST", "DIR_NORTH", "DIR_WEST")[int(round((angle % 360) / 90.0)) % 4]
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3 (as SINNOH's placeholders)
PRIMARY, SECONDARY = "gTileset_General", "gTileset_Johto_GoldenrodUndergroundEntrances"   # (not drawn: 3D)


def find(src, name):
    for arc in sorted(os.listdir(src)):
        p = os.path.join(src, arc, name)
        if os.path.exists(p):
            return p
    raise SystemExit("no %s in %s" % (name, src))


def main(src):
    maps = orre_maps.load(src)
    floors = colo_data.floors(src)
    places = []   # (constant, Colosseum's name), in the order maps first name them
    for m in maps:
        if all(n != m["place_name"] for _, n in places):
            places.append((mapsec(m["place_name"]), m["place_name"]))
    groups = json.load(open("data/maps/map_groups.json"))
    layouts = json.load(open("data/layouts/layouts.json"))
    if GROUP not in groups["group_order"]:
        groups["group_order"].append(GROUP)
    # (ORRE's layouts are all this tool's: one no map has any more goes)
    current = {"LAYOUT_" + m["id"][4:] for m in maps}
    layouts["layouts"] = [l for l in layouts["layouts"]
                          if not l.get("id", "").startswith("LAYOUT_ORRE_") or l["id"] in current]
    groups[GROUP] = []
    table, arrivals = [], []
    by_file = {m["file"]: m for m in maps}
    # every map's grid first, then its warp events: first each door's reached tiles (its collision ids in order, a
    # trigger's tiles left to right, top to bottom), then a landing for each of the room's entries that isn't on a
    # door (the entry's tile, or the nearest walkable one: never stepped on as a warp, only arrived at)
    grids, warps, landing = {}, {}, {}
    for m in maps:
        g = orre_maps.build_grid(src, m)
        grids[m["file"]] = g
        ws = []
        for tid in m["warps"]:
            tiles = sorted((y, x) for y in range(g["h"]) for x in range(g["w"]) if g["walk"][y, x] and g["trigger"][y, x] == tid)
            if not tiles:
                raise SystemExit("%s: collision %d leads nowhere (none of its tiles is reached)" % (m["file"], tid))
            ws += [(tid, x, y) for y, x in tiles]
        walk = [(y, x) for y in range(g["h"]) for x in range(g["w"]) if g["walk"][y, x]]
        land = []
        for ex, ez, ang in m["entries"]:
            tx, ty = int((ex - g["x0"]) // g["tile"]), int((ez - g["z0"]) // g["tile"])
            y, x = min(walk, key=lambda p: (abs(p[0] - ty) + abs(p[1] - tx), p))
            at = [i for i, w in enumerate(ws) if (w[1], w[2]) == (x, y)]
            if not at:
                ws.append((None, x, y))
                at = [len(ws) - 1]
            land.append(at[0])
            arrivals.append((m["id"], x, y, ang))
        warps[m["file"]], landing[m["file"]] = ws, land

    def dest_warp(file, entry):
        if isinstance(entry, int):
            return landing[file][entry]
        # an elevator: the middle tile of the trigger of the elevator over there (its own number is entry[1])
        cols = [c for c, n in by_file[file]["elevators"].items() if n == entry[1]]
        ws = [i for i, w in enumerate(warps[file]) if cols and w[0] == cols[0]]
        if not ws:
            raise SystemExit("%s has no elevator %d" % (file, entry[1]))
        return ws[len(ws) // 2]

    for m in maps:
        g = grids[m["file"]]
        lid = "LAYOUT_" + m["id"][4:]
        ldir = "data/layouts/%s" % m["name"]
        os.makedirs(ldir, exist_ok=True)
        open(ldir + "/map.bin", "wb").write(struct.pack("<%dH" % (g["w"] * g["h"]), *([BLOCKED] * (g["w"] * g["h"]))))
        open(ldir + "/border.bin", "wb").write(struct.pack("<4H", *([BLOCKED] * 4)))
        entry = {"id": lid, "name": m["name"] + "_Layout", "width": g["w"], "height": g["h"],
                 "primary_tileset": PRIMARY, "secondary_tileset": SECONDARY,
                 "border_filepath": ldir + "/border.bin", "blockdata_filepath": ldir + "/map.bin"}
        for i, l in enumerate(layouts["layouts"]):
            if l.get("id") == lid:
                layouts["layouts"][i] = entry
                break
        else:
            layouts["layouts"].append(entry)
        mdir = "data/maps/%s" % m["name"]
        os.makedirs(mdir, exist_ok=True)
        outdoor = "_out" in m["file"]
        we = []
        for k, (tid, x, y) in enumerate(warps[m["file"]]):
            if tid is None:   # (a landing: leads back to itself, never taken)
                we.append({"x": x, "y": y, "elevation": 0, "dest_map": m["id"], "dest_warp_id": str(k)})
            else:
                to, ent = m["warps"][tid]
                we.append({"x": x, "y": y, "elevation": 0, "dest_map": by_file[to]["id"],
                           "dest_warp_id": str(dest_warp(to, ent))})
        mj = {"id": m["id"], "name": m["name"], "layout": lid, "music": "MUS_NONE",   # (ORRE's songs come converted from the disc: none until then)
              "region_map_section": mapsec(m["place_name"]), "requires_flash": False, "weather": "WEATHER_NONE",
              "map_type": "MAP_TYPE_TOWN" if outdoor else "MAP_TYPE_INDOOR", "allow_cycling": outdoor,
              "allow_escaping": False, "allow_running": True,
              "show_map_name": outdoor, "battle_scene": "MAP_BATTLE_SCENE_NORMAL", "connections": None,
              "object_events": [], "bg_events": [],
              # (the way out: Colosseum's world map, from the exits' reached tiles)
              "coord_events": [{"type": "trigger", "x": x, "y": y, "elevation": 0, "var": "VAR_TEMP_0",
                                "var_value": "0", "script": "Orre_EventScript_WorldMap"}
                               for y in range(g["h"]) for x in range(g["w"])
                               if g["walk"][y, x] and g["trigger"][y, x] in m["exits"]],
              "warp_events": we}
        json.dump(mj, open(mdir + "/map.json", "w"), indent=2)
        open(mdir + "/map.json", "a").write("\n")
        sp = mdir + "/scripts.inc"
        if not os.path.exists(sp):   # (scripts come later, converted from Colosseum's own: never written by hand)
            open(sp, "w").write("%s_MapScripts::\n\t.byte 0\n" % m["name"])
        groups[GROUP].append(m["name"])
        sx, sz = [(x, y) for i, (_, x, y) in enumerate(warps[m["file"]]) if i == landing[m["file"]][0]][0]
        table.append("    {%s, \"%s\", %d, %d, %d, %d, %d},   // start (entry 0) %d, %d; Colosseum's room %d" % (
            m["id"], m["file"], g["w"] // 32, g["h"] // 32, sx, sz, m["room"], sx, sz, m["room"]))
    json.dump(groups, open("data/maps/map_groups.json", "w"), indent=2)
    open("data/maps/map_groups.json", "a").write("\n")
    json.dump(layouts, open("data/layouts/layouts.json", "w"), indent=2)
    open("data/layouts/layouts.json", "a").write("\n")
    # data/event_scripts_orre.s: only a map's include line is added when it's missing (after the last one); every
    # other line is left as it is (other tools' own lines: Moemon Maker's data/maker_globals.inc)
    sp = "data/event_scripts_orre.s"
    lines = open(sp).read().split("\n")
    if '#include "constants/comparison_operators.h"' not in lines:   # (orre.inc's goto_if_eq)
        lines.insert([i for i, l in enumerate(lines) if l.startswith("#include")][-1] + 1,
                     '#include "constants/comparison_operators.h"')
    if '\t.include "data/scripts/orre.inc"' not in lines:   # (ORRE's own glue: the world map's step events)
        at = [i for i, l in enumerate(lines) if "script_data" in l][0]
        lines.insert(at + 1, '\t.include "data/scripts/orre.inc"')
    for m in maps:
        inc = '\t.include "data/maps/%s/scripts.inc"' % m["name"]
        if inc not in lines:
            at = max([i for i, l in enumerate(lines) if '.include "data/maps/' in l] or
                     [i for i, l in enumerate(lines) if "script_data" in l])
            lines.insert(at + 1, inc)
    open(sp, "w").write("\n".join(lines))
    with open("include/constants/region_map_sections_orre.h", "w") as f:
        f.write("// Generated by tools/orre/gen_maps.py: ORRE's map sections, Colosseum's own place names\n"
                "#ifndef GUARD_CONSTANTS_REGION_MAP_SECTIONS_ORRE_H\n#define GUARD_CONSTANTS_REGION_MAP_SECTIONS_ORRE_H\n")
        f.write("#define MAPSEC_ORRE_FIRST 0x%X\n" % MAPSEC_FIRST)
        for i, (c, _) in enumerate(places):
            f.write("#define %-40s (MAPSEC_ORRE_FIRST + %d)\n" % (c, i))
        f.write("#define MAPSEC_ORRE_END (MAPSEC_ORRE_FIRST + %d)\n#endif\n" % len(places))
    with open("src/data/region_map/orre_region_map.h", "w") as f:
        f.write("// Generated by tools/orre/gen_maps.py: ORRE's map section names, Colosseum's own (common_rel's place names)\n")
        f.write("static const u8 *const gOrreMapSectionNames[] = {\n")
        for c, n in places:
            f.write("    [%s - MAPSEC_ORRE_FIRST] = (const u8[])_(\"%s\"),\n" % (c, n))
        f.write("};\n")
    with open("src/orre/orre_maps.h", "w") as f:
        f.write("// Generated by tools/orre/gen_maps.py: each ORRE map, its file in ORRE's pack (orre/orre3d.pak), its\n"
                "// size in blocks of 32 x 32 tiles, the tile the player can start on (src/orre/o3d.c) and its room in\n"
                "// Colosseum's floor table (tools/orre/colo_data.py: what the world map's destinations name)\n")
        f.write("static const struct { u16 map; const char *file; u8 bw, bh, startX, startY; u16 room; } sOrreMaps[] = {\n")
        f.write("\n".join(table) + "\n};\n")
        f.write("\n// where each entry of Colosseum's rooms lands, and the way the player faces there (the entry's angle)\n")
        f.write("static const struct { u16 map; u8 x, y, dir; } sOrreArrivals[] __attribute__((unused)) = {\n")
        for mid, x, y, ang in arrivals:
            f.write("    {%s, %d, %d, %s},\n" % (mid, x, y, facing(ang)))
        f.write("};\n")
    print("%d ORRE maps" % len(maps))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
