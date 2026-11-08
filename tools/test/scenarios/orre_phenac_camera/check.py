#!/usr/bin/env python3
"""orre_phenac_camera: after the kidnappers' scene the field's camera is back (its line after every camera line)"""
import sys
lines = open(sys.argv[1] + '/a.log', errors='replace').read().splitlines()
cam = [i for i, l in enumerate(lines) if 'orre vm: camera' in l or 'o3d cam:' in l]
ok = bool(cam) and "back to the field's camera" in lines[cam[-1]]
print(('ok  ' if ok else 'FAIL') + ' last camera line: %s' % (lines[cam[-1]].strip() if cam else 'none'))
sys.exit(0 if ok else 1)
