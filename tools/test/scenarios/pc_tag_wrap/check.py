# The display registers logged by the scenario (mp gpuregs): BG0 (DISPCNT bit 8) must be on once the group has joined
# the party ("after"), as it is before the group is held ("before")
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
marks = [m.group(1) for m in re.finditer(r'\[test\] gpuregs dispcnt ([0-9a-f]{4})', log)]
print('dispcnt:', marks)
if len(marks) < 3:
    sys.exit('expected three gpuregs lines')
bad = [d for d in (marks[0], marks[-2], marks[-1]) if not int(d, 16) & 0x100]
if bad:
    sys.exit('BG0 hidden: %s' % marks)
print('BG0 shown before and after')
