#!/usr/bin/env python3
"""Shadow Pokémon's dark palettes (the user, 2026-11-07: "special, dark palettes, similar to Shadow Lugia ... Since
there are a finite amount of Shadow Pokemon, we can curate what they look like"; docs/SHADOW_POKEMON.md "Dark
palettes"): one palette per Shadow Pokémon (src/data/orre_shadow.h's 48) for each of Kai's art sets, MOéMON
(graphics/moemon/gMonPalette_X.bin) and POKéMON (graphics/pokemon/x/normal.pal), shown while it is Shadow (shiny or not),
its own colours again once purified.

Each follows Shadow Lugia's own scheme (XD), its colours' roles working against each other (BODY ... OUTLINE below): the
largest part a deep indigo-plum, the largest light part a pale cool lavender-grey (Lugia's belly), the next part apart
blue-black to a steel-blue edge (its fins and plates), the eyes and small bright accents a crimson-magenta glow,
highlights icy lilac, outlines near-black violet, MOéMON's skin (the Stadium hues' skin slots, src/data/mp_hues.h) an
ashen lilac. Each colour keeps its lightness order along its role's ramp (roles(): which part a colour is, by the
share of the picture it covers and its hue family). Then CURATED gives any Pokémon's
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


# Shadow Lugia's scheme (XD): each role a ramp from its darkest to its lightest shade (0-255 RGB)
BODY = ((34, 18, 60), (130, 88, 172))       # deep indigo-plum: the largest part
UNDER = ((116, 110, 150), (216, 210, 236))  # pale cool lavender-grey: the largest light part (Lugia's belly, wings' undersides)
HARD = ((12, 22, 56), (72, 122, 196))       # blue-black to a steel-blue edge: fins, plates, the next part apart
SKIN = ((146, 134, 178), (230, 224, 242))   # MOéMON's skin: ashen lilac
EYE = ((150, 18, 64), (255, 98, 152))       # crimson-magenta glow: eyes and small bright accents
HILITE = (230, 226, 250)                    # icy lilac highlights
OUTLINE = ((14, 8, 24), (40, 30, 64))       # near-black violet


def lum_of(rgb):
    return (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]) / 255.0


def ramp(r, t):
    t = max(0.0, min(1.0, t))
    return tuple(a + (b - a) * t for a, b in zip(r[0], r[1]))


def roles(colours, counts, skin_mask):
    """{slot: role}: each of the palette's colours given a part of Shadow Lugia's scheme by what it covers."""
    total = max(1, sum(counts[1:]))
    role, groups = {}, {}
    for i in range(1, 16):
        if counts[i] == 0 and colours[i] == colours[0]:
            continue
        r, g, b = (v / 255.0 for v in colours[i])
        h, sat, val = colorsys.rgb_to_hsv(r, g, b)
        lum, share = lum_of(colours[i]), counts[i] / total
        if lum < 0.16:
            role[i] = "outline"
        elif skin_mask >> i & 1:
            role[i] = "skin"
        elif share < 0.025 and sat > 0.45 and val > 0.45:
            role[i] = "eye"
        elif sat < 0.18 and lum > 0.8 and share < 0.04:
            role[i] = "hilite"
        else:
            key = "neutral-light" if sat < 0.2 and lum > 0.6 else "neutral" if sat < 0.2 else int(((h * 360) + 15) % 360 // 30)
            groups.setdefault(key, []).append(i)
    # the parts, largest first: the largest the body; of the rest, the lightest large one the underside, then the next
    # large one the hard parts; the others go to the role nearest their lightness
    cover = lambda k: sum(counts[i] for i in groups[k])
    light = lambda k: sum(lum_of(colours[i]) * counts[i] for i in groups[k]) / max(1, cover(k))
    order = sorted(groups, key=cover, reverse=True)
    assigned = {}
    if order:
        assigned[order[0]] = "body"
        rest = [k for k in order[1:] if cover(k) / total > 0.06]
        under = max(rest, key=light, default=None)
        if under is not None and light(under) > light(order[0]) + 0.08:
            assigned[under] = "under"
        hard = [k for k in order[1:] if k not in assigned and cover(k) / total > 0.03]
        if hard:
            assigned[hard[0]] = "hard"
        for k in order:
            if k not in assigned:
                lk = light(k)
                assigned[k] = "under" if lk > 0.72 else "hard" if lk < 0.35 else "body"
    for k, slots in groups.items():
        for i in slots:
            role[i] = assigned[k]
    return role


def make(colours, px, skin_mask, curated):
    counts = [0] * 16
    for row in px:
        for i in row:
            counts[i & 15] += 1
    role = roles(colours, counts, skin_mask)
    ramps = {"body": BODY, "under": UNDER, "hard": HARD, "skin": SKIN, "eye": EYE, "outline": OUTLINE}
    # each role's colours keep their lightness order along its ramp
    by_role = {}
    for i, rl in role.items():
        by_role.setdefault(rl, []).append(i)
    out = list(colours)
    for rl, slots in by_role.items():
        if rl == "hilite":
            for i in slots:
                out[i] = HILITE
            continue
        ls = [lum_of(colours[i]) for i in slots]
        lo, hi = min(ls), max(ls)
        for i, l in zip(slots, ls):
            t = 0.15 + 0.75 * (l - lo) / (hi - lo) if hi - lo > 0.04 else 0.25 + 0.6 * l
            out[i] = ramp(ramps[rl], t)
    for i, c in curated.items():
        out[i] = c
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
