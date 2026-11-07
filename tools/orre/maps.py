#!/usr/bin/env python3
"""ORRE's maps in the game: each Colosseum map (its file name on the disc) that is a map of the field engine, in
gMapGroup_Orre in this order (map numbers stay as they are: new maps go at the end).

  start: where the player can stand (Colosseum units, x z): the tile grid keeps only what is reached from there
         (tools/orre/grid.py), and it is where a debug warp puts the player
  place: the map's name, one of Colosseum's own place names (common_rel's messages 3501-3527, tools/orre/colo_data.py),
         the one its floor record's area is (area 1, the two S1_ rooms: 3521 OUTSKIRT STAND)
"""

MAPS = [
    {"file": "S1_out", "id": "MAP_ORRE_OUTSKIRT_STAND", "name": "Orre_OutskirtStand", "start": (-16.0, -24.0),
     "place": 3521},
]


def by_file():
    return {m["file"]: m for m in MAPS}
