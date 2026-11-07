#!/usr/bin/env python3
"""Checks the scenario shadow_look: where the player's POKéMON stands (the battle picture's lower left) and its front
picture on the summary screen (*_summary), the dark palette's shots differ clearly from its own colours' in both art
sets (the dark palettes aren't always darker: shadow fire is bright), and the purified shot (own_pokemon) is its own
colours again (its flames' warm reds and oranges back).

    python3 -I tools/orre/check_shadow_look.py RESULT_DIR
"""
import sys

from PIL import Image

BOXES = {"": (0.17, 0.42, 0.50, 0.70),          # the back picture, above the text box
         "_summary": (0.00, 0.30, 0.17, 0.80)}   # the summary's front picture


def pixels(path, box):
    im = Image.open(path).convert("RGB")
    w, h = im.size
    c = im.crop((int(w * box[0]), int(h * box[1]), int(w * box[2]), int(h * box[3])))
    return [c.getpixel((x, y)) for y in range(c.height) for x in range(c.width)]


def change(a, b):
    """the share of pixels whose colour moved a lot"""
    return 100.0 * sum(1 for p, q in zip(a, b) if sum(abs(u - v) for u, v in zip(p, q)) > 90) / len(a)


def warm(px):
    return 100.0 * sum(1 for r, g, b in px if r > 180 and r > g + 60 and r > b + 90) / len(px)


d = sys.argv[1]
bad, notes = [], []
for suf, box in BOXES.items():
    p = {n: pixels("%s/%s%s.png" % (d, n, suf), box) for n in ("own_moe", "dark_moe", "dark_pokemon", "own_pokemon")}
    for art, own, dark in (("MOeMON", "own_moe", "dark_moe"), ("POKeMON", "own_pokemon", "dark_pokemon")):
        ch = change(p[own], p[dark])
        notes.append("%s%s %.1f%%" % (art, suf, ch))
        if ch < 5:
            bad.append("%s%s: the dark palette barely changes the picture (%.1f%% of pixels)" % (art, suf, ch))
    if warm(p["own_pokemon"]) < 1 or warm(p["dark_pokemon"]) > 0.5 * warm(p["own_pokemon"]):
        bad.append("POKeMON%s: the flames' colours (warm %.1f%% dark, %.1f%% purified)" % (
            suf, warm(p["dark_pokemon"]), warm(p["own_pokemon"])))
print("\n".join(bad) or "ok: changed " + ", ".join(notes))
sys.exit(1 if bad else 0)
