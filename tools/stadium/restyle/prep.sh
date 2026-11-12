#!/bin/bash
# What the mock-up scripts need beside them: the narrow font as glyphs, STADIUM's pictures as PNGs.
# usage: tools/stadium/restyle/prep.sh   (from the repository's root; tools/gbagfx must be built: make tools)
set -e
D="$(cd "$(dirname "$0")" && pwd)"; R="$(cd "$D/../../.." && pwd)"
"$R/tools/gbagfx/gbagfx" "$R/graphics/fonts/latin_small_narrow.png" "$D/narrow.latfont"
mkdir -p "$D/pics"
python3 - "$R/graphics/stadium/stadium.bin" "$D/pics" <<'PY'
import sys, struct
from PIL import Image
d = open(sys.argv[1], 'rb').read()
n = struct.unpack_from('<H', d, 4)[0]; p = 6
for i in range(n):
    w, h = struct.unpack_from('<HH', d, p); p += 4
    im = Image.new('RGBA', (w, h))
    im.putdata([(((v >> 11) & 31) * 255 // 31, ((v >> 6) & 31) * 255 // 31, ((v >> 1) & 31) * 255 // 31, 255 if v & 1 else 0)
                for v in (d[p + k * 2] | d[p + k * 2 + 1] << 8 for k in range(w * h))])
    im.save(f'{sys.argv[2]}/pic{i:02d}.png'); p += w * h * 2
PY
echo "ready: python3 $D/screens.py; python3 $D/screens2.py; python3 $D/screens3.py (pictures land beside the scripts)"
