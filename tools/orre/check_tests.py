#!/usr/bin/env python3
"""Checks an ORRE whole-region scenario's log (tools/orre/gen_tests.py): each `[test] where map G.N xy X,Y` line, in
order, against the scenario's expect.txt ("G.N xy X,Y", "~1" after it: a tile either way will do).

    python3 -I tools/orre/check_tests.py EXPECT.txt RESULT_DIR
"""
import re
import sys


def main(expect_path, result_dir):
    want = [l.split("#")[0].strip() for l in open(expect_path) if l.split("#")[0].strip()]
    notes = [l.split("#", 1)[1].strip() if "#" in l else "" for l in open(expect_path) if l.split("#")[0].strip()]
    got = re.findall(r"\[test\] where map (\d+)\.(\d+) xy (-?\d+),(-?\d+)", open(result_dir + "/a.log").read())
    bad = 0
    for i, w in enumerate(want):
        m = re.match(r"(\d+)\.(\d+) xy (-?\d+),(-?\d+)( ~1)?", w)
        g, n, x, y = (int(v) for v in m.groups()[:4])
        if i >= len(got):
            print("missing: %s %s" % (w, notes[i]))
            bad += 1
            continue
        G, N, X, Y = (int(v) for v in got[i])
        slack = 1 if m.group(5) else 0
        if (G, N) != (g, n) or abs(X - x) + abs(Y - y) > slack:
            print("want %s, got %d.%d xy %d,%d   %s" % (w, G, N, X, Y, notes[i]))
            bad += 1
    print("%d of %d right" % (len(want) - bad, len(want)))
    return 1 if bad else 0


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    sys.exit(main(sys.argv[1], sys.argv[2]))
