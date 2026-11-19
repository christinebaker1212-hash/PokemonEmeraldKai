"""exp_curve_scaled: the foe's EXP is its species' yield times 5 levels under its battle level, over 7 (the game's own
formula at the lower level), and nothing cuts it afterwards"""
import re
import sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
m = re.search(r'exp: (\d+) from the foe \(species (\d+) Lv(\d+), worked out at Lv(\d+)\), (\d+) each', log)
if not m:
    sys.exit('no exp line')
exp, species, lv, at, each = map(int, m.groups())
if at != max(1, lv - 5):
    sys.exit('worked out at Lv%d for a Lv%d foe' % (at, lv))
print('ok: Lv%d foe worked out at Lv%d: %d EXP, %d to the one that fought' % (lv, at, exp, each))
if each != exp:
    sys.exit('the one that fought got %d of %d' % (each, exp))
