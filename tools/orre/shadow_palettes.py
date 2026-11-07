#!/usr/bin/env python3
"""Shadow Pokémon's dark palettes (the user, 2026-11-07: "special, dark palettes, similar to Shadow Lugia ... Since
there are a finite amount of Shadow Pokemon, we can curate what they look like"; docs/SHADOW_POKEMON.md "Dark
palettes"): one palette per Shadow Pokémon (src/data/orre_shadow.h's 48) for each of Kai's art sets, MOéMON
(graphics/moemon/gMonPalette_X.bin) and POKéMON (graphics/pokemon/x/normal.pal), shown while it is Shadow (shiny or not),
its own colours again once purified.

Each starts from one treatment, in the spirit of XD's Shadow Lugia (shadow_colour below): the body sunk into a deep
violet ramp that keeps each colour's lightness order and its parts apart, outlines near-black indigo, highlights a cold
lavender, the eyes and small bright accents (few pixels, strong colour) a glowing red-violet, MOéMON's skin (the
Stadium hues' skin slots, src/data/mp_hues.h) an ashen lavender rather than violet. Then CURATED gives any Pokémon's
own colours slot by slot, chosen looking at the preview (`--preview OUT.png`: each one's MOéMON and POKéMON pictures,
own colours and dark, side by side).

    python3 -I tools/orre/shadow_palettes.py [OUT.h] [--preview OUT.png]     (default src/data/orre_shadow_palettes.h)
"""
import colorsys
import os
import re
import struct
import sys

REPO = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")

# Hand curation: {species constant: {"moe" | "pkmn": {slot: (r, g, b)}}} over the treatment (0-255 colours). Empty until
# the user has looked at the preview: each entry here is a choice made by eye.
CURATED = {}


def lz77(data):
    """GBA LZ77 (type 0x10)."""
    size = data[1] | data[2] << 8 | data[3] << 16
    out, i = bytearray(), 4
    while len(out) < size:
        flags = data[i]
        i += 1
        for b in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> b):
                disp = ((data[i] & 0xF) << 8 | data[i + 1]) + 1
                n = (data[i] >> 4) + 3
                i += 2
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(data[i])
                i += 1
    return bytes(out[:size])


def bgr555(c):
    return (c & 31) * 255 // 31, (c >> 5 & 31) * 255 // 31, (c >> 10 & 31) * 255 // 31


def to555(rgb):
    r, g, b = (max(0, min(255, int(round(v)))) for v in rgb)
    return (r * 31 + 127) // 255 | ((g * 31 + 127) // 255) << 5 | ((b * 31 + 127) // 255) << 10


def species_list():
    """[(species number, constant)] of the Shadow Pokémon, from src/data/orre_shadow.h and include/constants/species.h."""
    h = open(os.path.join(REPO, "src/data/orre_shadow.h")).read()
    nums = [int(m) for m in re.findall(r"^    \{\d+, (\d+), ", h, re.M)]
    consts = {}
    for name, val in re.findall(r"#define (SPECIES_\w+) (\d+)\b", open(os.path.join(REPO, "include/constants/species.h")).read()):
        consts.setdefault(int(val), name)
    return [(n, consts[n]) for n in nums]


def skin_masks():
    h = open(os.path.join(REPO, "src/data/mp_hues.h")).read()
    body = h[h.index("sMpHueSkin[NUM_SPECIES] = {"):]
    body = body[:body.index("};")]
    out = {}
    for name, val in re.findall(r"\[(SPECIES_\w+)\] = (0x[0-9A-Fa-f]+|\d+)", body):
        out[name] = int(val, 0)
    return out


def camel(const):
    return "".join(w.capitalize() for w in const[len("SPECIES_"):].split("_"))


def moe_art(const):
    """(16 colours, the front picture's 64 x 64 indices) of a MOéMON picture."""
    c = camel(const)
    pal = lz77(open(os.path.join(REPO, "graphics/moemon/gMonPalette_%s.bin" % c), "rb").read())
    pic = lz77(open(os.path.join(REPO, "graphics/moemon/gMonFrontPic_%s.bin" % c), "rb").read())
    colours = [bgr555(struct.unpack_from("<H", pal, 2 * i)[0]) for i in range(16)]
    px = [[0] * 64 for _ in range(64)]
    for t in range(64):
        tx, ty = t % 8, t // 8
        for y in range(8):
            for x in range(8):
                b = pic[t * 32 + y * 4 + x // 2]
                px[ty * 8 + y][tx * 8 + x] = (b >> 4) if x & 1 else (b & 15)
    return colours, px


def pkmn_art(const):
    from PIL import Image
    d = os.path.join(REPO, "graphics/pokemon", const[len("SPECIES_"):].lower())
    lines = open(os.path.join(d, "normal.pal")).read().split("\n")[3:19]
    colours = [tuple(int(v) for v in l.split()[:3]) for l in lines]
    im = Image.open(os.path.join(d, "front.png"))
    px = [[im.getpixel((x, y)) for x in range(64)] for y in range(64)]
    return colours, px


def shadow_colour(rgb, share, skin):
    """One colour of the dark palette: SHARE of the picture's pixels it covers, SKIN a MOéMON skin slot."""
    r, g, b = (v / 255.0 for v in rgb)
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    lum = 0.299 * r + 0.587 * g + 0.114 * b
    if lum < 0.16:                                   # outline: near-black indigo
        return tuple(255 * x for x in colorsys.hsv_to_rgb(0.72, 0.55, 0.10 + 0.4 * lum))
    if skin:                                         # MOéMON's skin: ashen lavender
        return tuple(255 * x for x in colorsys.hsv_to_rgb(0.74, 0.14, 0.52 + 0.42 * lum))
    if share < 0.025 and s > 0.45 and v > 0.45:      # eyes, small bright accents: a glowing red-violet
        return tuple(255 * x for x in colorsys.hsv_to_rgb(0.94, 0.80, max(0.85, v)))
    if s < 0.18 and lum > 0.78 and share < 0.06:     # highlights (a white body sinks like the rest): cold lavender
        return tuple(255 * x for x in colorsys.hsv_to_rgb(0.75, 0.22, 0.70 + 0.25 * lum))
    # the body: a deep violet ramp, its parts kept apart by their hue (spread over violet to indigo)
    band = 0.64 + 0.22 * ((h + 0.08) % 1.0)
    return tuple(255 * x for x in colorsys.hsv_to_rgb(band, 0.32 + 0.45 * s, 0.10 + 0.60 * lum ** 0.9))


def make(colours, px, skin_mask, curated):
    counts = [0] * 16
    for row in px:
        for i in row:
            counts[i & 15] += 1
    total = max(1, sum(counts[1:]))
    out = [colours[0]]
    for i in range(1, 16):
        if i in curated:
            out.append(curated[i])
        else:
            out.append(shadow_colour(colours[i], counts[i] / total, bool(skin_mask >> i & 1)))
    return out


def build():
    skins = skin_masks()
    rows = []
    for num, const in species_list():
        moe = moe_art(const)
        pk = pkmn_art(const)
        cur = CURATED.get(const, {})
        rows.append((num, const, moe, make(*moe, skins.get(const, 0), cur.get("moe", {})),
                     pk, make(*pk, 0, cur.get("pkmn", {}))))
    return rows


def write(out, rows):
    L = ["// Generated by tools/orre/shadow_palettes.py: each Shadow Pokémon's dark palette for the MOéMON and POKéMON",
         "// pictures (docs/SHADOW_POKEMON.md \"Dark palettes\"). Don't edit: change the tool's CURATED and rerun.",
         "// Included once, by src/orre/orre_shadow.c.", "",
         "static const struct { u16 species; u16 moe[16], pokemon[16]; } sOrreShadowPalettes[] = {"]
    for num, const, moe, dmoe, pk, dpk in rows:
        L.append("    {%s, {%s},\n        {%s}}," % (const, ", ".join("0x%04X" % to555(c) for c in dmoe),
                                                     ", ".join("0x%04X" % to555(c) for c in dpk)))
    L.append("};")
    open(out, "w").write("\n".join(L) + "\n")


def preview(out, rows):
    from PIL import Image, ImageDraw
    cols, cw, chh = 3, 4 * 64 + 12, 64 + 14
    im = Image.new("RGB", (cols * cw, ((len(rows) + cols - 1) // cols) * chh), (40, 40, 48))
    d = ImageDraw.Draw(im)

    def paste(px, colours, x0, y0):
        for y in range(64):
            for x in range(64):
                i = px[y][x] & 15
                if i:
                    im.putpixel((x0 + x, y0 + y), tuple(int(v) for v in colours[i]))

    for k, (num, const, moe, dmoe, pk, dpk) in enumerate(rows):
        x0, y0 = (k % cols) * cw + 4, (k // cols) * chh + 12
        d.text((x0, y0 - 11), const[8:], fill=(220, 220, 230))
        paste(moe[1], moe[0], x0, y0)
        paste(moe[1], dmoe, x0 + 64, y0)
        paste(pk[1], pk[0], x0 + 128 + 4, y0)
        paste(pk[1], dpk, x0 + 192 + 4, y0)
    im = im.resize((im.width * 2, im.height * 2), Image.NEAREST)
    im.save(out)


if __name__ == "__main__":
    args = sys.argv[1:]
    prev = None
    if "--preview" in args:
        prev = args[args.index("--preview") + 1]
        del args[args.index("--preview"):args.index("--preview") + 2]
    out = args[0] if args else os.path.join(REPO, "src/data/orre_shadow_palettes.h")
    rows = build()
    write(out, rows)
    if prev:
        preview(prev, rows)
    print("%d Shadow Pokemon's dark palettes -> %s" % (len(rows), out))
