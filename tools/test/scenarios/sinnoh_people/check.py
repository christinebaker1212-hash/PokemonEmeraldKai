#!/usr/bin/env python3
"""Each mode's people: GEN 4 all Platinum's (tags 0xb8xx), MIXED some Gen 3 and some smaller Platinum (0xb9xx), GEN 3
no Platinum sprite at all."""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
parts = re.split(r'\[test\] sinnoh sprites (\d)', log)
seen = {}
for i in range(1, len(parts), 2):
    tags = re.findall(r'objpal \d+: gfx \S+ (\d+x\d+) slot \d+ tag (0x[0-9a-f]+)', parts[i + 1])
    seen.setdefault(int(parts[i]), []).extend(tags)
for m in (0, 1, 2):
    print(m, sorted(set(seen.get(m, []))))
g4, mx, g3 = (set(t for _, t in seen.get(m, [])) for m in (0, 1, 2))
assert g4 and all(t.startswith('0xb8') for t in g4), 'GEN 4: not all Platinum'
assert any(t.startswith('0xb9') for t in mx) and any(not t.startswith('0xb') for t in mx), 'MIXED: no mix'
assert g3 and not any(t.startswith(('0xb8', '0xb9')) for t in g3), 'GEN 3: a Platinum sprite'
