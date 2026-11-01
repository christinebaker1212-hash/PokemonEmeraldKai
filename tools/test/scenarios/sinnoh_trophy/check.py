#!/usr/bin/env python3
"""The garden's slot 6 is MR. BACKLOT's POKéMON after he tells of it"""
import re, sys
log = open(sys.argv[1] + '/a.log', errors='replace').read()
told = re.findall(r'trophy garden: species (\d+)', log)
slots = re.findall(r'^\[test\] land slots 6-7: (\d+) (\d+)', log, re.M)
assert told and len(slots) == 2, (told, slots)
assert slots[1] == (told[0], '172'), (told, slots)
print('slot 6 is', told[0])
