#!/bin/bash
# src/orre/gx_tex.c (the game's GX texture decoder) against tools/orre/gxtex.py on every texture of ORRE's pack.
# usage: tools/orre/test_gxtex.sh [ORRE_DIR]   (default orre/)
set -e
cd "$(dirname "$0")/../.."
PAK="${1:-orre}/orre3d.pak"
T=$(mktemp -d)
cc -O2 -iquote include -o "$T/dump" tools/orre/test/gxtex_dump.c src/orre/gx_tex.c
"$T/dump" "$PAK" > "$T/c.txt"
python3 -I - "$PAK" > "$T/py.txt" <<'PY'
import sys, os
sys.path.insert(0, "tools/orre")
import pak
p = pak.Pack3D(sys.argv[1])
for i in range(len(p.tex)):
    b = p.image(i).astype("uint8").tobytes()
    h = 2166136261
    for c in b:
        h = ((h ^ c) * 16777619) & 0xFFFFFFFF
    print("%d %08x" % (i, h))
PY
if cmp -s "$T/c.txt" "$T/py.txt"; then echo "$(wc -l < "$T/c.txt") textures: the same"; rm -rf "$T"; exit 0; fi
diff "$T/c.txt" "$T/py.txt" | head; echo "FAILED"; exit 1
