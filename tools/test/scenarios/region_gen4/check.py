# The first roll (no Elite Four beaten) has no STARLY (421) or SHINX (428); the second (HOENN's beaten) has them
import sys
rolls = [l.split(':')[1].split() for l in open(sys.argv[1] + '/a.log') if l.startswith('[test] wildroll:')]
assert len(rolls) == 2 and len(rolls[0]) == 80 and len(rolls[1]) == 80, rolls
gen4 = {'421', '428'}
assert not gen4 & set(rolls[0]), 'Gen 4 before an Elite Four: %s' % rolls[0]
assert gen4 & set(rolls[1]), 'no Gen 4 after HOENN\'s Elite Four: %s' % rolls[1]
print('ok')
