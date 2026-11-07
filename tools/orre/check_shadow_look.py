#!/usr/bin/env python3
"""Checks the scenario shadow_look: where the player's POKéMON stands (the battle picture's lower left), the dark
palette's shots have far more dark pixels than its own colours', in both art sets, and its own colours are back when
purified (the last shot: own_pokemon).

    python3 -I tools/orre/check_shadow_look.py RESULT_DIR
"""
import sys

from PIL import Image


def brightness(path):
    im = Image.open(path).convert("RGB")
    w, h = im.size
    box = im.crop((int(w * 0.17), int(h * 0.42), int(w * 0.50), int(h * 0.70)))   # (the back picture, above the text box)
    px = [box.getpixel((x, y)) for y in range(box.height) for x in range(box.width)]
    # (the share of dark pixels: the light background is the same in every shot)
    return 100.0 * sum(1 for r, g, b in px if 0.299 * r + 0.587 * g + 0.114 * b < 100) / len(px)


d = sys.argv[1]
b = {n: brightness("%s/%s.png" % (d, n)) for n in ("own_moe", "dark_moe", "dark_pokemon", "own_pokemon")}
bad = []
if b["dark_moe"] < 1.5 * b["own_moe"] + 3:
    bad.append("MOeMON: the dark palette isn't darker (%.1f%% dark pixels vs %.1f%%)" % (b["dark_moe"], b["own_moe"]))
if b["dark_pokemon"] < 1.5 * b["own_pokemon"] + 3:
    bad.append("POKeMON: the dark palette isn't darker (%.1f%% dark pixels vs %.1f%%)" % (b["dark_pokemon"], b["own_pokemon"]))
print("\n".join(bad) or "ok: %s" % ", ".join("%s %.1f" % kv for kv in b.items()))
sys.exit(1 if bad else 0)
