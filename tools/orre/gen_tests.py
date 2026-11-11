#!/usr/bin/env python3
"""ORRE's whole-region tests, written from the maps (tools/orre/maps.py, gen_maps.py's map.json files):

  tools/test/scenarios/orre_all_maps    every map: warped to its start (Colosseum's entry 0), where the player lands
  tools/test/scenarios/orre_all_doors   every door, staircase and elevator: from the walkable tile in front of its
                                        trigger, a step onto it, and where the player comes out
  tools/test/scenarios/orre_all_places  Colosseum's world map: its cursor moved (the game's own links,
                                        src/orre/worldmap.c MoveCursor) to each of its nine places, every place's
                                        story flag set, and travelled to: where the player lands (REALGAM TOWER
                                        before and after its flag 0x45D, as the game's travel has it)

Each scenario's expect.txt says what each `mp where` must print, in order (tools/orre/check_tests.py checks the log:
the scenarios' CHECK). Run again after gen_maps.py:

    python3 -I tools/orre/gen_tests.py UNPACKED_DIR
"""
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import maps as orre_maps  # noqa: E402

GROUP = 45
SCEN = "tools/test/scenarios"
DIRS = {(0, -1): "UP", (0, 1): "DOWN", (-1, 0): "LEFT", (1, 0): "RIGHT"}


def write(name, conf, lines, expect):
    d = os.path.join(SCEN, name)
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, "a.txt"), "w").write("\n".join(lines) + "\n")
    open(os.path.join(d, "expect.txt"), "w").write("\n".join(expect) + "\n")
    open(os.path.join(d, "conf"), "w").write(conf)


def main(src):
    maps = orre_maps.load(src)
    num = {m["file"]: i for i, m in enumerate(maps)}
    mj = {m["file"]: json.load(open("data/maps/%s/map.json" % m["name"])) for m in maps}
    grids = {m["file"]: orre_maps.build_grid(src, m) for m in maps}
    ident = {m["id"]: m["file"] for m in maps}

    # every map's start
    lines, expect = ["pressuntil A field 5000", "wait 30"], []
    hdr = open("src/orre/orre_maps.h").read()
    for m in maps:
        # (the start gen_maps.py wrote: src/orre/orre_maps.h, the landing of entry 0)
        row = [l for l in hdr.split("\n") if l.startswith("    {%s, \"%s\"" % (m["id"], m["file"]))][0]
        sx, sy = [int(v) for v in row.split("{", 1)[1].split(",")[4:6]]
        lines += ["mp warp %d,%d,%d,%d" % (GROUP, num[m["file"]], sx, sy), "wait 70", "mp where"]
        # (a start on a door's tile: the player steps off it)
        door = any(w["x"] == sx and w["y"] == sy and w["dest_map"] != m["id"] for w in mj[m["file"]]["warp_events"])
        expect.append("%d.%d xy %d,%d%s   # %s" % (GROUP, num[m["file"]], sx, sy, " ~1" if door else "", m["file"]))
    write("orre_all_maps", """# ORRE (tools/orre/gen_tests.py): every one of Colosseum's field rooms is a map that loads and can be stood in:
# a warp to each map's start (the landing of Colosseum's entry 0), where the player is (tools/orre/check_tests.py
# against expect.txt). Needs orre/ (tools/orre/make_pack.sh).
MODE=1
ENV_A="MOEMON_ORRE_SCRIPTS=0"
TMO=600
WARP=45,0,33,40
CHECK="python3 -I tools/orre/check_tests.py tools/test/scenarios/orre_all_maps/expect.txt"
EXPECT_A=("[test] where map 45.%d xy")
UNEXPECT_A=("[crash]" "TIMEOUT" "has no grid" "isn't the size of its map" "no ORRE pack")
""" % (len(maps) - 1), lines, expect)

    # every door: from the tile in front of its trigger
    lines, expect = ["pressuntil A field 5000", "wait 30"], []
    for m in maps:
        g = grids[m["file"]]
        we = mj[m["file"]]["warp_events"]
        for tid, (to, entry) in sorted(m["warps"].items()):
            tiles = [(w["x"], w["y"]) for w in we
                     if g["trigger"][w["y"], w["x"]] == tid and w["dest_map"] != m["id"]]
            ws = [i for i, w in enumerate(we) if (w["x"], w["y"]) in tiles]
            best = None
            for k in sorted(range(len(tiles)), key=lambda k: abs(k - len(tiles) // 2)):
                x, y = tiles[k]
                for (dx, dy), key in DIRS.items():
                    nx, ny = x - dx, y - dy
                    if 0 <= nx < g["w"] and 0 <= ny < g["h"] and g["walk"][ny, nx] and g["trigger"][ny, nx] < 0:
                        best = (nx, ny, key, ws[k])
                        break
                if best:
                    break
            if best is None:
                print("note: %s collision %d: no walkable tile in front of it to test from" % (m["file"], tid))
                continue
            nx, ny, key, wi = best
            dest = we[wi]
            dfile = ident[dest["dest_map"]]
            dw = mj[dfile]["warp_events"][int(dest["dest_warp_id"])]
            # (landing on a door: the player steps off it, a tile either way; landing on an entry: there)
            on_door = dw["dest_map"] != mj[dfile]["id"]
            lines += ["mp warp %d,%d,%d,%d" % (GROUP, num[m["file"]], nx, ny), "wait 60", "press %s 24" % key,
                      "wait 150", "mp where"]
            expect.append("%d.%d xy %d,%d%s   # %s collision %d -> %s" % (GROUP, num[dfile], dw["x"], dw["y"],
                          " ~1" if on_door else "", m["file"], tid, dfile))
    write("orre_all_doors", """# ORRE (tools/orre/gen_tests.py): every door, staircase and elevator of Colosseum's field rooms leads where the
# game's own tables say (tools/orre/colo_rooms.py): from the walkable tile in front of each trigger a step onto it,
# then where the player comes out (tools/orre/check_tests.py against expect.txt: the entry's tile, or a tile off an
# elevator). Needs orre/ (tools/orre/make_pack.sh). About 4 minutes.
MODE=1
ENV_A="MOEMON_ORRE_SCRIPTS=0"
TMO=1500
WARP=45,0,33,40
CHECK="python3 -I tools/orre/check_tests.py tools/test/scenarios/orre_all_doors/expect.txt"
UNEXPECT_A=("[crash]" "TIMEOUT" "has no grid" "isn't the size of its map" "no ORRE pack")
""", lines, expect)
    doors = len(expect)
    places(src, maps, num, mj, hdr)
    print("orre_all_maps: %d maps; orre_all_doors: %d doors" % (len(maps), doors))


def _be32(d, o):
    return int.from_bytes(d[o:o + 4], "big")


def places(src, maps, num, mj, hdr):
    """orre_all_places: the world map's tables (worldmap_menu_rel's .data, as src/orre/worldmap.c reads them)."""
    import colo_rooms
    rel = open(os.path.join(src, "world_map", "worldmap_menu_rel.rel"), "rb").read()
    off, _ = colo_rooms.rel_sections(rel)[3]
    n_places, n_links = _be32(rel, off + 0x6C8), _be32(rel, off + 0x6D4)
    pl = [(_be32(rel, off + 16 * i), _be32(rel, off + 16 * i + 4), _be32(rel, off + 16 * i + 8)) for i in range(n_places)]
    links = [(rel[off + 0x3B0 + 12 * k], _be32(rel, off + 0x3B0 + 12 * k + 4), _be32(rel, off + 0x3B0 + 12 * k + 8))
             for k in range(n_links)]
    item_place = {item: i for i, (_, _, item) in enumerate(pl)}
    keys = {1: "UP", 2: "RIGHT", 4: "DOWN", 8: "LEFT", 3: "UP+RIGHT", 6: "RIGHT+DOWN", 12: "DOWN+LEFT", 9: "UP+LEFT"}

    def move(cur, mask):   # (MoveCursor: the first link from here with every direction pressed; every place shown)
        for dirs, frm, to in links:
            if dirs & mask == mask and frm == pl[cur][2] and to in item_place:
                return item_place[to]
        return cur

    def path(a, b):
        from collections import deque
        prev, q = {a: None}, deque([a])
        while q:
            c = q.popleft()
            for mask in keys:
                n = move(c, mask)
                if n not in prev:
                    prev[n] = (c, mask)
                    q.append(n)
        out = []
        while b != a:
            b, mask = prev[b]
            out.append(keys[mask])
        return out[::-1]

    room_map = {m["room"]: m for m in maps}

    def start_of(m):
        row = [l for l in hdr.split("\n") if l.startswith("    {%s, \"%s\"" % (m["id"], m["file"]))][0]
        return [int(v) for v in row.split("{", 1)[1].split(",")[4:6]]

    lines = ["pressuntil A field 5000", "wait 30"] + ["mp orreflag 0x%X" % f for f, _, _ in pl if f]
    expect, cur = [], 0
    realgam = [i for i, p in enumerate(pl) if p[1] == 0x66][0]
    # every place in turn, back to the OUTSKIRT STAND, then REALGAM TOWER again once its flag 0x45D is set
    for step, i in enumerate(list(range(1, n_places)) + [0, realgam]):
        if step == n_places:
            lines.append("mp orreflag 0x45D")
        lines += ["mp orreworldmap", "wait 50"]
        for k in path(cur, i):
            lines += ["press %s 4" % k, "wait 30"]
        lines += ["press A 5", "wait 200", "mp where"]
        m = room_map[0x7B if step == n_places else pl[i][1]]
        sx, sy = start_of(m)
        door = any(w["x"] == sx and w["y"] == sy and w["dest_map"] != m["id"] for w in mj[m["file"]]["warp_events"])
        expect.append("45.%d xy %d,%d%s   # the world map's %s -> %s" % (num[m["file"]], sx, sy, " ~1" if door else "",
                                                                          m["place_name"], m["file"]))
        cur = i
    write("orre_all_places", """# ORRE (tools/orre/gen_tests.py): Colosseum's world map travels to each of its nine places: every place's story
# flag set, the cursor moved by the game's own links to each place in turn, A, and where the player lands (Colosseum's
# entry 0 of the place's room; REALGAM TOWER before and after its flag 0x45D: its first outside, then the one with the
# tower's door). tools/orre/check_tests.py against expect.txt. Needs orre/ (tools/orre/make_pack.sh).
MODE=1
ENV_A="MOEMON_ORRE_SCRIPTS=0"
TMO=300
WARP=45,0,33,40
CHECK="python3 -I tools/orre/check_tests.py tools/test/scenarios/orre_all_places/expect.txt"
UNEXPECT_A=("[crash]" "TIMEOUT" "isn't in the game yet" "won't load")
""", lines, expect)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1])
