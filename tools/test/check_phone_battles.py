#!/usr/bin/env python3
"""johto_phone_battles' CHECK: with MOENAV CALLS on BATTLES, JOEY rang exactly once (his rematch call).

    python3 tools/test/check_phone_battles.py RESULT_DIR
"""
import os
import sys

log = open(os.path.join(sys.argv[1], 'a.log'), errors='replace').read()
n = log.count('qzphone: 15 calls')
if n != 1:
    sys.exit('FAIL: JOEY called %d times, not once' % n)
print('phone battles: one call, the rematch')
