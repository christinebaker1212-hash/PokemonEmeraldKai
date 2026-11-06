#!/bin/bash
# Only the two 20% slots may come up more than 12% of the time (even with STATIC leading)
python3 - "$1/a.log" <<'P'
import re, sys
p = [float(m.group(1)) for m in re.finditer(r'wildodds: species \d+ \d+ of \d+ \(([\d.]+)%\)', open(sys.argv[1]).read())]
big = sorted(x for x in p if x > 12)
print('shares:', sorted(p, reverse=True))
sys.exit(0 if p and len(big) <= 2 else 1)
P
