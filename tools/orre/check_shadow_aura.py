#!/usr/bin/env python3
"""Checks the scenario shadow_aura: just above and beside the player's POKéMON (where its own-colours shot shows only the
battle background), the Shadow shots show the aura: dark violet smoke while it's out (faint, the send-out's burst), and
crimson in HYPER MODE (hyper_idle, the turn's end).

    python3 -I tools/orre/check_shadow_aura.py RESULT_DIR
"""
import glob
import sys

from PIL import Image


def band(path):
    im = Image.open(path).convert("RGB")
    w, h = im.size
    s = w / 240.0
    box = im.crop((int(36 * s), int(36 * s), int(120 * s), int(60 * s)))   # (above the QUILAVA's head)
    return [box.getpixel((x, y)) for y in range(box.height) for x in range(box.width)]


def violet(px):
    return sum(1 for r, g, b in px if 0.299 * r + 0.587 * g + 0.114 * b < 150 and b > g + 25 and r >= g)


def crimson(px):
    return sum(1 for r, g, b in px if 0.299 * r + 0.587 * g + 0.114 * b < 160 and r > g + 40 and r > b and b > g + 10)


d = sys.argv[1]
plain, faint, hyper = band(d + "/plain.png"), band(d + "/faint.png"), band(d + "/hyper_idle.png")
turn = max((band(f) for f in glob.glob(d + "/turn_*.png")), key=crimson)
burst = max((band(f) for f in glob.glob(d + "/sendout_*.png")), key=violet)
v = {"plain": violet(plain), "faint": violet(faint), "burst": violet(burst), "hyper": crimson(hyper),
     "turn": crimson(turn), "plain_red": crimson(plain)}
bad = []
if v["plain"] > 10 or v["plain_red"] > 10:
    bad.append("the own-colours shot has aura colours (%d violet, %d crimson)" % (v["plain"], v["plain_red"]))
if v["faint"] < 40:
    bad.append("no aura while it's out (%d violet pixels)" % v["faint"])
if v["burst"] < v["faint"] + 20:
    bad.append("no burst on the send-out (%d violet pixels, %d while out)" % (v["burst"], v["faint"]))
if v["hyper"] < 40 or v["turn"] < 40:
    bad.append("HYPER MODE isn't crimson (%d, the turn's end %d)" % (v["hyper"], v["turn"]))
print("\n".join(bad) or "ok: " + ", ".join("%s %d" % kv for kv in v.items()))
sys.exit(1 if bad else 0)
