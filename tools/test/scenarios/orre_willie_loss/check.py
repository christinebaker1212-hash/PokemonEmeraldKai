#!/usr/bin/env python3
"""orre_willie_loss: WILLIE's scene starts again after the loss (his first line twice)"""
import sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
n = log.count('orre vm: message 20792')
print(('ok  ' if n >= 2 else 'FAIL') + ' WILLIE\'s scene played %d time(s) (want 2)' % n)
sys.exit(0 if n >= 2 else 1)
