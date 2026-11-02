#!/usr/bin/env python3
"""Platinum's time-of-day color for the 2D regions (HOENN, JOHTO, KANTO): sinnoh/platinum_daylight.bin, beside the 3D
pack and, like it, built from pret/pokeplatinum by tools/sinnoh/make_pack.sh (never in the repository).

    python3 -I tools/sinnoh/build_daylight.py DECOMP_DIR OUT

Platinum lights its outdoor maps by the area light set most of matrix 0 uses (its towns and routes), a block of lights
and material colors for each part of the day. Flat ground lit that way (light 0, the sun or moon, on an upward face:
the 3D engine's own sum, src/sinnoh/s3d.c Light) gives a color per channel; each half hour of the day is written as
that color over noon's, so the day looks as the 2D maps were drawn and the morning, evening and night take
Platinum's tints (src/mp/mp_daytint.c).

Format: "PLDL", then 48 half hours from 0:00, each u8 red, green, blue: the share of the map's own color, 128 = all.
"""
import os
import struct
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from source import open_source   # noqa: E402


def ground(block):
    """Flat ground's light (0-31 a channel) under one light block (s3d.c Light with light 0 and an upward normal)"""
    _, lights, cols = block
    diffuse, ambient, _, emission = cols
    on, lc, d = lights[0]
    acc = list(emission)
    if on:
        level = max(0.0, -d[1])   # the normal (0, 1, 0): level = -(d . n)
        for ch in range(3):
            acc[ch] += (diffuse[ch] * lc[ch] * level + ambient[ch] * lc[ch]) / 31.0
    return [min(31.0, a) for a in acc]


def at(blocks, units):
    """The block lit at a time of day (2-second units): the first that ends after it (s3d.c PickLight)"""
    for b in blocks:
        if b[0] > units:
            return b
    return blocks[-1]


def main():
    src = open_source(sys.argv[1])
    mx = src.matrix(0)
    sets = Counter()
    for z in range(mx.height):
        for x in range(mx.width):
            _, header, _ = mx.cell(x, z)
            if header is not None:
                sets[src.area_of(header)[2]] += 1
    blocks = src.lights(sets.most_common(1)[0][0])
    noon = ground(at(blocks, 12 * 1800))
    out = b'PLDL'
    for half in range(48):
        c = ground(at(blocks, half * 900))
        out += bytes(max(0, min(255, round(128 * c[ch] / noon[ch]))) if noon[ch] else 128 for ch in range(3))
    open(sys.argv[2], 'wb').write(out)
    print('daylight: light set %d, %d blocks' % (sets.most_common(1)[0][0], len(blocks)))


if __name__ == '__main__':
    main()
