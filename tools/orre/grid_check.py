#!/usr/bin/env python3
"""ORRE's grids against Colosseum's own walkable floor (the user, 2026-11-10: "any area deemed walkable in the
original game must be walkable here"). For every room: the fine flood of where a person fits in Colosseum
(tools/orre/grid.py _reach) against the tiles the grid opens, and the tile graph (with its blocked sides) against
what the flood joins.

    python3 -I tools/orre/grid_check.py [UNPACKED]     (default ~/.cache/moemon/orre/unpacked)

Prints the rooms that lose the most floor, the total, and FAILS (exit 1) when a room's walkable tiles aren't one
walk from each other where Colosseum's floor is, or floor lies farther than grid.FAR from every walkable tile.
"""
import os
import sys
from collections import deque

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import grid  # noqa: E402
import maps as M  # noqa: E402


def main():
    U = sys.argv[1] if len(sys.argv) > 1 else os.path.expanduser("~/.cache/moemon/orre/unpacked")
    tot = lost = bad = 0
    rows = []
    for m in M.load(U):
        g = M.build_grid(U, m)
        seen, k, _block, _trigf = grid._LAST
        walk, edges = g["walk"], g["edges"]
        h, w = walk.shape
        per = seen.reshape(h, k, w, k).sum((1, 3))
        lost_here, cells = int(per[~walk].sum()), int(seen.sum())
        tot += cells
        lost += lost_here
        # the tiles with reached floor: one walk from each other
        comp = np.full((h, w), -1)
        n = 0
        for y0, x0 in np.argwhere(walk & (per > 0)):
            if comp[y0, x0] >= 0:
                continue
            comp[y0, x0] = n
            q = deque([(int(y0), int(x0))])
            while q:
                y, x = q.popleft()
                for bit, ny, nx in ((1, y - 1, x), (2, y, x + 1), (4, y + 1, x), (8, y, x - 1)):
                    if 0 <= ny < h and 0 <= nx < w and walk[ny, nx] and comp[ny, nx] < 0 and not edges[y, x] & bit:
                        comp[ny, nx] = n
                        q.append((ny, nx))
            n += 1
        # (as many areas as Colosseum's own floor has: a room whose entries lead to parts that never meet)
        left = seen.copy()
        fine = 0
        for z0, x0 in np.argwhere(seen):
            if not left[z0, x0]:
                continue
            fine += 1
            left[z0, x0] = False
            st = [(int(z0), int(x0))]
            while st:
                z, x = st.pop()
                for nz, nx in ((z + 1, x), (z - 1, x), (z, x + 1), (z, x - 1)):
                    if 0 <= nz < left.shape[0] and 0 <= nx < left.shape[1] and left[nz, nx]:
                        left[nz, nx] = False
                        st.append((nz, nx))
        if n > fine:
            print("FAIL %s: %d separate areas of walkable tiles, %d in Colosseum" % (m["file"], n, fine))
            bad += 1
        rows.append((lost_here / max(cells, 1), m["file"], cells, lost_here, int(walk.sum())))
    rows.sort(reverse=True)
    for r in rows[:10]:
        print("%.3f %-22s floor %6d lost %5d tiles %d" % r)
    print("all rooms: %d of %d fine cells of floor lie in tiles that aren't walkable (%.2f%%)"
          % (lost, tot, 100.0 * lost / max(tot, 1)))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
