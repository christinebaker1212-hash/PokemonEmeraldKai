#!/usr/bin/env python3
"""ORRE's maps as the field engine's maps (docs/ORRE_DECISIONS.md): each map of tools/orre/maps.py as a map of
gMapGroup_Orre with map.json, scripts.inc, a layout and their places in map_groups.json / layouts.json, Orre's
script file (data/event_scripts_orre.s) and the 3D engine's table of them (src/orre/orre_maps.h). Only each map's
size in tiles is read from the disc (its collision, tools/orre/grid.py); nothing of Colosseum's art or words is
written here. The layouts' blocks are placeholders (every tile blocked): the game takes each tile's collision from
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

MAPSEC_FIRST = 0x1C0   # ORRE's map sections, after SINNOH's (0x140 to 0x1BA)


def mapsec(name):
    return "MAPSEC_ORRE_" + "".join(c if c.isalnum() else "_" for c in name.upper()).strip("_").replace("__", "_")

GROUP = "gMapGroup_Orre"
BLOCKED = 0x0001 | (1 << 10) | (3 << 12)   # metatile 1, collision, elevation 3 (as SINNOH's placeholders)
PRIMARY, SECONDARY = "gTileset_General", "gTileset_Johto_GoldenrodUndergroundEntrances"   # (not drawn: 3D)


def find(src, name):
    for arc in sorted(os.listdir(src)):
        p = os.path.join(src, arc, name)
        if os.path.exists(p):
            return p
    raise SystemExit("no %s in %s" % (name, src))


def main(src):
    text = colo_data.common_text(src)
    places = []   # (constant, Colosseum's name), in the order maps first name them
    for m in orre_maps.MAPS:
        name = colo_data.plain(text[m["place"]])
        if all(n != name for _, n in places):
            places.append((mapsec(name), name))
    groups = json.load(open("data/maps/map_groups.json"))
    layouts = json.load(open("data/layouts/layouts.json"))
    if GROUP not in groups["group_order"]:
        groups["group_order"].append(GROUP)
    groups[GROUP] = []
    table = []
    for m in orre_maps.MAPS:
        g = grid.build(grid.read(open(find(src, m["file"] + ".col"), "rb").read()), grid.TILE, m["start"])
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
        mj = {"id": m["id"], "name": m["name"], "layout": lid, "music": "MUS_NONE",   # (ORRE's songs come converted from the disc: none until then)
              "region_map_section": mapsec(colo_data.plain(text[m["place"]])), "requires_flash": False, "weather": "WEATHER_NONE",
              "map_type": "MAP_TYPE_TOWN", "allow_cycling": True, "allow_escaping": False, "allow_running": True,
              "show_map_name": m["file"].endswith("_out"), "battle_scene": "MAP_BATTLE_SCENE_NORMAL", "connections": None,
              "object_events": [], "warp_events": [], "coord_events": [], "bg_events": []}
        json.dump(mj, open(mdir + "/map.json", "w"), indent=2)
        open(mdir + "/map.json", "a").write("\n")
        sp = mdir + "/scripts.inc"
        if not os.path.exists(sp):   # (scripts come later, converted from Colosseum's own: never written by hand)
            open(sp, "w").write("%s_MapScripts::\n\t.byte 0\n" % m["name"])
        groups[GROUP].append(m["name"])
        sx, sz = int((m["start"][0] - g["x0"]) // g["tile"]), int((m["start"][1] - g["z0"]) // g["tile"])
        table.append("    {%s, \"%s\", %d, %d, %d, %d},   // start tile %d, %d" % (
            m["id"], m["file"], g["w"] // 32, g["h"] // 32, sx, sz, sx, sz))
    json.dump(groups, open("data/maps/map_groups.json", "w"), indent=2)
    open("data/maps/map_groups.json", "a").write("\n")
    json.dump(layouts, open("data/layouts/layouts.json", "w"), indent=2)
    open("data/layouts/layouts.json", "a").write("\n")
    # data/event_scripts_orre.s: only a map's include line is added when it's missing (after the last one); every
    # other line is left as it is (other tools' own lines: Moemon Maker's data/maker_globals.inc)
    sp = "data/event_scripts_orre.s"
    lines = open(sp).read().split("\n")
    for m in orre_maps.MAPS:
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
                "// size in blocks of 32 x 32 tiles and the tile the player can start on (src/orre/o3d.c)\n")
        f.write("static const struct { u16 map; const char *file; u8 bw, bh, startX, startY; } sOrreMaps[] = {\n")
        f.write("\n".join(table) + "\n};\n")
    print("%d ORRE maps" % len(orre_maps.MAPS))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
