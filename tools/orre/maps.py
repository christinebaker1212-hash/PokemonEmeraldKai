#!/usr/bin/env python3
"""ORRE's maps in the game: each Colosseum map (its file name on the disc) that is a map of the field engine, in
gMapGroup_Orre in this order (map numbers stay as they are: new maps go at the end).

  start: where the player can stand (Colosseum units, x z): the tile grid keeps only what is reached from there
         (tools/orre/grid.py), and it is where a debug warp puts the player
  place: the map's name, one of Colosseum's own place names (common_rel's messages 3501-3527, tools/orre/colo_data.py),
         the one its floor record's area is (area 1, the two S1_ rooms: 3521 OUTSKIRT STAND)
  warps: {trigger: (file, trigger there)}: a collision trigger (its object number in the map's .col) that is a door
         into another map, arriving on the middle of that map's trigger; its reached tiles warp when stepped on
         (MB_NON_ANIMATED_DOOR). Until the scripts say where each trigger leads, they are named here by hand.
"""

STEP_WARP = 0x60   # MB_NON_ANIMATED_DOOR: warps when stepped on

MAPS = [
    {"file": "S1_out", "id": "MAP_ORRE_OUTSKIRT_STAND", "name": "Orre_OutskirtStand", "start": (-16.0, -24.0),
     "place": 3521, "warps": {13: ("S1_shop_1F", 2)}},   # (the diner's door, at the ramp's top)
    {"file": "S1_shop_1F", "id": "MAP_ORRE_OUTSKIRT_STAND_DINER", "name": "Orre_OutskirtStandDiner",
     "start": (-52.0, 20.0), "place": 3521, "warps": {2: ("S1_out", 13)}},   # (its doorway, back out)
]


def by_file():
    return {m["file"]: m for m in MAPS}
