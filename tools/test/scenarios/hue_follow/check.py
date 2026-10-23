#!/usr/bin/env python3
"""hue_follow: BOB's view of ALICE's follower has the hue ALICE's game gives it."""
import os, re, sys
d = sys.argv[1]
a = open(os.path.join(d, 'a.log'), errors='replace').read()
b = open(os.path.join(d, 'b.log'), errors='replace').read()
ha = re.findall(r'\[test\] huemon 1 species 35 hue (-?\d+)', a)
hb = re.findall(r'follower: slot \d dex 35 hue (-?\d+)', b)
print('alice', ha, 'bob sees', hb)
if not ha or not hb or ha[-1] != hb[-1] or ha[-1] == '0':
    sys.exit('hue differs')
