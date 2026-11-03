#!/usr/bin/env python3
"""sinnoh_lake_water: LAKE VERITY's water moves between lake_a and lake_b (more than a few hundred pixels change)"""
import os
import sys
from PIL import Image, ImageChops

d = sys.argv[1]
a = Image.open(os.path.join(d, 'lake_a.png')).convert('RGB').crop((0, 0, 480, 200))   # (above any message box)
b = Image.open(os.path.join(d, 'lake_b.png')).convert('RGB').crop((0, 0, 480, 200))
diff = ImageChops.difference(a, b).convert('L')
changed = sum(diff.histogram()[9:])
print('lake water: %d pixels changed' % changed)
sys.exit(0 if changed > 300 else 1)
