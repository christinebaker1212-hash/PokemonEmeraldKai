#!/usr/bin/env python3
"""Moemon VMU pet: the file's fixed places hold (pet.s "the file's fixed places"): the program ends before the
version page ($BB00), the UPDATE receiver fits in $BC00-$BFFF, the file is 96 blocks. waterbear lets a later .org
write over earlier bytes without a word, so this is the check.

usage: check_layout.py PET.VMS   (its PET.VMS.debug.json beside it: waterbear --debug)
"""
import json
import os
import sys

vms = sys.argv[1]
d = json.load(open(vms + '.debug.json'))
labels = {l['name']: l['offset'] for l in d['labels']}
size = os.path.getsize(vms)
bad = []
if labels.get('code_end', 1 << 20) > 0xBB00:
    bad.append('the program runs to $%04X, past the version page ($BB00)' % labels['code_end'])
if labels.get('save_page') != 0xBB80:
    bad.append('the SAVE page is at $%04X, not $BB80' % labels.get('save_page', -1))
if labels.get('update_receive') != 0xBC00:
    bad.append('the receiver is at $%04X, not $BC00' % labels.get('update_receive', -1))
if labels.get('update_end', 1 << 20) > 0xBFFF:
    bad.append('the receiver runs to $%04X, past the file' % labels['update_end'])
if size != 0xC000:
    bad.append('the file is %d bytes, not 49152 (96 blocks)' % size)
if bad:
    sys.exit('layout: ' + '; '.join(bad))
print('layout: the program to $%04X of $BB00, the receiver $BC00-$%04X, 96 blocks' % (labels['code_end'], labels['update_end']))
