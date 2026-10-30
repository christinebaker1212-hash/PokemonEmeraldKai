# JOHTO shows Gen 1-2 only: ROUTE 29's first two rolls (before and after an Elite Four) are all national dex 1-251
# (Kai numbers them the same); with the GENERATION setting at 4 STARLY (421) / SHINX (428) appear
import sys
rolls = [l.split(':')[1].split() for l in open(sys.argv[1] + '/a.log') if l.startswith('[test] wildroll:')]
assert len(rolls) == 5 and all(len(r) in (80, 120) for r in rolls), rolls
for r in rolls[:2]:
    late = [s for s in r if int(s) > 251]
    assert not late, 'later than Gen 2 in JOHTO: %s' % late
assert {'421', '428'} & set(rolls[2]), 'no Gen 4 at GENERATION 4: %s' % rolls[2]
print('ok')
# GENERATION 5: Quetzal's own ROUTE 29 where it's Gen 1-5 (LILLIPUP 531), not APPLIN (865, Gen 8); GENERATION 9: APPLIN
assert '531' in rolls[3] and '865' not in rolls[3], 'GENERATION 5: %s' % rolls[3]
assert '865' in rolls[4], 'GENERATION 9: %s' % rolls[4]
print('generations 5 and 9 ok')
