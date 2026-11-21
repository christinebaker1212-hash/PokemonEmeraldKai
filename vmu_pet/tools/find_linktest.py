#!/usr/bin/env python3
"""Moemon VMU pet: find LINK TEST's saves on a VM2's SD card (or in any VMU images).

usage: find_linktest.py PATH...   (a folder, e.g. the SD card's drive, searched for .bin / .vmu / .vms; or files)

For every VMU image holding the LINK TEST program, and every loose LINKTEST .VMS, prints the counts its SAVE page
holds (SENT / GOT / times started) and the last packet received. This answers where a program running on the VM2
saves: the image on the card that changed is the one it ran from.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import vmufs  # noqa: E402

MAGIC = b'MOEMONLINKTEST01'


def report(where, data):
    found = False
    for off in range(0, len(data) - 128 + 1, 128):
        if data[off:off + 16] == MAGIC:
            found = True
            sent, got, boots = data[off + 16], data[off + 17], data[off + 18]
            pkt = data[off + 32:off + 64]
            print('%s: SENT %d, GOT %d, started %d times; last packet received %s' % (
                where, sent, got, boots, pkt.hex() if any(pkt) else '(none)'))
    return found


def scan(path):
    n = 0
    data = open(path, 'rb').read()
    if len(data) == vmufs.BLOCK * vmufs.BLOCKS:
        try:
            img = vmufs.Image(data)
            for f in img.files():
                if f['type'] == 'GAME':
                    n += report('%s [%s]' % (path, f['name']), img.read(f['name']))
            return n
        except Exception as e:  # not a VMU image after all
            print('%s: not a VMU image (%s)' % (path, e))
    return report(path, data)


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    total = 0
    for p in argv[1:]:
        if os.path.isdir(p):
            for root, _, files in os.walk(p):
                for name in sorted(files):
                    if name.lower().endswith(('.bin', '.vmu', '.vms')):
                        total += scan(os.path.join(root, name))
        else:
            total += scan(p)
    if not total:
        print('No LINK TEST save found.')


if __name__ == '__main__':
    main(sys.argv)
