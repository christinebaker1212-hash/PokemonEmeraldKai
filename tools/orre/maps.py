#!/usr/bin/env python3
"""ORRE's maps in the game: every room of Colosseum's field (tools/orre/colo_rooms.py field_rooms: 121 rooms), as maps
of gMapGroup_Orre in ORDER (map numbers stay as they are: a new map goes at the end, never in between).

Everything about a map comes from the game's own tables (colo_rooms.py), nothing by hand:
  entries  where the game puts a player entering it (x, z, angle): the reach of the tile grid starts from all of
           them (tools/orre/grid.py), entry 0 is where the world map lands and where a debug warp puts the player
  place    its place name (the floor record's: one of common_rel's messages 3501-3527)
  warps    {collision id: (file, entry)} or (file, ("elevator", n)): its doors, stairs and elevators, the triggers of
           those collision ids warping when stepped on (MB_NON_ANIMATED_DOOR); arriving on the entry's tile (or the
           other elevator's trigger)
  exits    collision ids that are the way out to Colosseum's world map (src/orre/worldmap.c)
  gates    collision objects the room's scripts switch on and off (story gates): left open until the scripts are
           converted, so every room can be walked in whole

Links into a stadium's battle stage (THE UNDER's and PYRITE's colosseums: rooms of kind 2) aren't maps: Kai's battles.

    python3 -I tools/orre/maps.py UNPACKED_DIR        (prints the maps and their links)
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import colo_data  # noqa: E402
import colo_rooms  # noqa: E402

STEP_WARP = 0x60   # MB_NON_ANIMATED_DOOR: warps when stepped on

# the maps' order in gMapGroup_Orre (map numbers): never reorder, add new ones at the end
ORDER = [
    "S1_out", "S1_shop_1F",
    "M1_out", "M1_pc_1F", "M1_houseA_1F", "M1_houseA_2F", "M1_houseB_1F", "M1_houseC_1F", "M1_shop_1F", "M1_shop_2F",
    "M1_gym_1F", "M1_gym_B1", "M1_stadium_1F",
    "M2_out", "M2_police_1F", "M2_hotel_1F", "M2_uranai_1F", "M2_houseA_1F", "M2_guild_1F_1", "M2_guild_1F_2",
    "M2_shop_1F", "M2_windmill_1F", "M2_windmill_2F", "M2_building_1F", "M2_building_2F", "M2_building_3F",
    "M2_building_4F", "M2_cave_1F_1", "M2_cave_1F_2", "M2_cave_2F_1", "M2_cave_2F_2", "M2_cave_3F_1", "M2_cave_3F_2",
    "M3_out", "M3_cave_1F_1", "M3_cave_1F_2", "M3_shrine_1F", "M3_houseA_1F", "M3_houseB_1F", "M3_pc_1F",
    "M3_shop_1F", "M3_houseC_1F", "M3_houseC_2F", "M3_houseD_1F",
    "M4_out", "M4_junk_1F", "M4_junk_B1", "M4_hotel_1F", "M4_hotel_B1", "M4_shop_1F", "M4_labo_1F", "M4_labo_B1",
    "M4_houseA_1F", "M4_subway_B1", "M4_subway_B2_1", "M4_subway_B2_2", "M4_subway_B3", "M4_train_1F",
    "M4_bottom_1F",
    "D1_out", "D1_labo_1F", "D1_labo_B1", "D1_labo_B2", "D1_labo_B3", "D1_garage_1F", "D1_garage_B1",
    "D1_garage_B2_1", "D1_garage_B2_2",
    "D2_out", "D2_pc_1F", "D2_valley_1", "D2_rest_1", "D2_valley_2", "D2_rest_2", "D2_valley_3", "D2_rest_3",
    "D2_magma_1", "D2_rest_4", "D2_magma_2", "D2_rest_5", "D2_magma_3", "D2_rest_6", "D2_cloud_1", "D2_rest_7",
    "D2_cloud_2", "D2_rest_8", "D2_cloud_3", "D2_rest_9", "D2_cloud_4", "D2_crater_colo",
    "D4_out", "D4_dome_1", "D4_dome_2", "D4_dome_3", "D4_dome_4", "D4_dome_5", "D4_dome_6", "D4_dome_7", "D4_dome_8",
    "D4_dome_9", "D4_dome_10", "D4_dome_11", "D4_tower_1F_1", "D4_tower_1F_2", "D4_tower_1F_3", "D4_tower_2F",
    "D4_casino_colo",
    "M1_pc_B1", "M2_enter_1F", "M4_enter_1F", "M4_out_2", "D4_out_2", "D4_tower_2F_2", "M4_enter_B1", "M4_kanpo_1F",
    "M1_carde_1",
    "S2_building_1F_2", "S2_building_2F_2", "S2_building_3F_2", "S2_out_2",
    "M2_enter_1F_2",
]

# the first two maps' names, as they were before the rest came (the others are named from their place and file)
NAMES = {"S1_out": ("MAP_ORRE_OUTSKIRT_STAND", "Orre_OutskirtStand"),
         "S1_shop_1F": ("MAP_ORRE_OUTSKIRT_STAND_DINER", "Orre_OutskirtStandDiner")}


def _camel(s):
    return "".join(w[:1].upper() + w[1:] for w in re.split(r"[^A-Za-z0-9]+", s) if w)


def _names(file, place):
    """("MAP_ORRE_...", "Orre_..."): the place's name, and the file's own part after its area (M1_pc_1F: Pc1F)."""
    if file in NAMES:
        return NAMES[file]
    rest = file.split("_", 1)[1]
    pl = _camel(place.title().replace("'S", "s"))
    name = "Orre_" + pl + ("" if rest == "out" else "_" + _camel(rest))
    ident = "MAP_ORRE_" + "_".join(re.split(r"[^A-Za-z0-9]+", place.replace("'", "")) + ([] if rest == "out" else [rest])).upper()
    return ident, name


def load(unpacked):
    """[map dict] in ORDER: file, id, name, room, place (message), place_name, entries [(x, z, angle)], start (x, z),
    gates, warps, exits."""
    rs = colo_rooms.rooms(unpacked)
    text = colo_data.common_text(unpacked)
    by_file = {r.file: r for r in colo_rooms.field_rooms(rs)}
    missing = sorted(set(by_file) - set(ORDER))
    if missing:
        raise SystemExit("rooms not in tools/orre/maps.py ORDER (add them at the end): %s" % ", ".join(missing))
    out, seen = [], set()
    for file in ORDER:
        r = by_file[file]
        place = colo_data.plain(text[r.place])
        ident, name = _names(file, place)
        if name in seen:
            raise SystemExit("two maps named %s" % name)
        seen.add(name)
        warps, exits = {}, []
        for col, link in sorted(r.links.items()):
            room, entry = colo_rooms.link_target(rs, link)
            if room == colo_rooms.WORLD_MAP:
                exits.append(col)
            elif room in rs and rs[room].file in by_file:
                to = rs[room]
                if isinstance(entry, int) and entry >= len(to.entries):
                    entry = 0   # (REALGAM's lift to its casino names an entry the room hasn't: its only one)
                warps[col] = (to.file, entry)
        out.append({"file": file, "id": ident, "name": name, "room": r.room, "place": r.place, "place_name": place,
                    "entries": [(e[1], e[3], e[0]) for e in r.entries], "entry_ys": [e[2] for e in r.entries], "start": (r.entries[0][1], r.entries[0][3]),
                    "gates": sorted(r.gates), "doors": list(r.doors), "warps": warps, "exits": exits,
                    "elevators": {col: l[3] for col, l in r.links.items() if l[0] == "elevator"}})
    return out


def door_objects(m):
    """The collision objects that are doors' leaves (Colosseum's door table: tools/orre/colo_rooms.py Room.doors)"""
    return {d[1] for d in m["doors"] if d[1] >= 0}


def stops(m):
    """The collision ids whose triggers lead somewhere: reached, not walked through (tools/orre/grid.py)."""
    return set(m["warps"]) | set(m["exits"])


def build_grid(unpacked, m):
    """The map's tile grid (tools/orre/grid.py), as gen_maps.py and build_pack.py both make it."""
    import grid
    p = os.path.join(unpacked, m["file"], m["file"] + ".col")
    # (built with every story gate and every door open: src/orre/orre_vm.c shuts their tiles while they're closed)
    return grid.build(grid.read(open(p, "rb").read(), set(m["gates"]) | door_objects(m)), grid.TILE,
                      [(x, z) for x, z, _ in m["entries"]], stops(m))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    for i, m in enumerate(load(sys.argv[1])):
        print("%3d %-18s %-40s room %3d %-18s warps %s exits %s" % (i, m["file"], m["name"], m["room"],
              m["place_name"], m["warps"], m["exits"]))
