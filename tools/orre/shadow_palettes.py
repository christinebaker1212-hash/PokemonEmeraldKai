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
ashen lilac. A role goes to a whole material (materials(): the shades of one hue that touch each other in the
picture, so a gradient is one ramp and never splits into stray pixels), and each colour keeps its lightness order along
its role's ramp (a highlight stays the lightest). PICKS then gives each Pokémon its own accents, some outside Shadow
Lugia's colours (shadow fire for fire, gold for what glows or blooms, crimson blades, cyan crystal), looked at one by
one (the user, 2026-11-07); CURATED can still set any slot by hand. `--preview OUT.png` shows each one's MOéMON and
POKéMON pictures, own colours and dark, side by side (`--only NAME,NAME` for a few, larger).

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
# accents outside Shadow Lugia's own colours, chosen Pokémon by Pokémon (PICKS below)
# (a ramp may have more than two stops: evenly spaced)
FLAME = ((120, 14, 150), (214, 46, 214), (255, 128, 236), (255, 238, 255))   # shadow fire: magenta to a white-hot core
GOLD = ((132, 70, 18), (236, 164, 34), (255, 240, 150))    # gold-amber, violet's complement: orbs, antlers, petals, rings
CYAN = ((18, 60, 120), (40, 170, 220), (200, 246, 255))    # ice-cyan: crystals, gems, wings that glow
CRIMSON = ((80, 6, 30), (190, 18, 60), (255, 96, 120))     # a deep red: markings that should stay red (blades, crests)
GLOW = ((150, 18, 120), (255, 70, 170), (255, 200, 236))   # a hot pink glow: orbs, pearls, paint

# Each Shadow Pokémon's own choices, made looking at the preview (the user, 2026-11-07: "take a look at each one to
# make sure the colors make sense ... Quilava's fire should be brighter ... If there are complimentary colors that
# would look really good that aren't in Shadow Lugia's palette, use them"): {species: [(colours, role)]}, for both art
# sets. COLOURS picks the palette's colours by their own look (FAMILIES); ROLE is one of the ramps (body, under, hard,
# eye, flame, gold, cyan, crimson) or "keep" (its own colour).
FAMILIES = {
    "warm": lambda h, s, v, l: (h < 55 or h >= 340) and s > 0.45 and v > 0.5,      # fire, orange, red-orange
    "red": lambda h, s, v, l: (h < 18 or h >= 335) and s > 0.45 and v > 0.35,
    "yellow": lambda h, s, v, l: 38 <= h < 70 and s > 0.35 and v > 0.5,
    "green": lambda h, s, v, l: 70 <= h < 165 and s > 0.25,
    "blue": lambda h, s, v, l: 180 <= h < 250 and s > 0.25,
    "pink": lambda h, s, v, l: (300 <= h < 345) and s > 0.2 and v > 0.5,
    "light": lambda h, s, v, l: l > 0.78 and s < 0.25,
    "orange": lambda h, s, v, l: 18 <= h < 45 and s > 0.45 and v > 0.5,
    "cyan": lambda h, s, v, l: 165 <= h < 200 and s > 0.2,
    "purple": lambda h, s, v, l: 250 <= h < 300 and s > 0.2,
}
W, R, Y, G, B, P, L, O = "warm", "red", "yellow", "green", "blue", "pink", "light", "orange"
PICKS = {
    # fire burns bright: shadow fire, magenta to white-hot
    "SPECIES_QUILAVA": {"pkmn": [((10, 11, 12, 13), "flame"), ((6, 7, 8, 9), "under"), ((2, 3, 4), "hard")],
                        "moe": [((6, 8, 10, 13), "flame"), ((2, 3, 4), "hard"), ((5, 7, 9), "body")]},
    "SPECIES_SLUGMA": {"pkmn": [((2, 3, 6, 7, 8, 9), "flame"), ((4, 5), "body")],
                       "moe": [((4, 5, 6), "flame"), ((2, 3), "body")]},
    "SPECIES_HOUNDOOM": {"moe": [((13, 14, 15), "flame")], "pkmn": [((6, 7, 8, 9, 10, 11), "flame"), (L, "under")]},
    "SPECIES_ENTEI": {"moe": [(R, "crimson"), (Y, "gold"), (L, "under")],
                      "pkmn": [((2, 3, 4, 5, 9), "body"), ((10, 11), "gold"), ((6, 7, 8), "crimson"),
                               ((1, 12, 13), "under")]},
    # gold where the Pokémon glows, shines or blooms (violet's complement)
    "SPECIES_FLAAFFY": {"both": [(B, "gold"), ("cyan", "gold"), (L, "under")]},
    "SPECIES_SKIPLOOM": {"pkmn": [(Y, "gold"), (O, "gold")], "moe": [(Y, "gold")]},
    "SPECIES_SUNFLORA": {"moe": [(Y, "gold"), (O, "gold")], "pkmn": [((3, 4, 5, 6, 9), "gold"), ((7, 8), "under")]},
    "SPECIES_STANTLER": {"moe": [(Y, "gold")], "pkmn": [((7, 8, 9, 10), "gold")]},
    "SPECIES_URSARING": {"pkmn": [((7, 8), "gold")]},
    "SPECIES_QWILFISH": {"pkmn": [(Y, "gold")]},
    "SPECIES_TROPIUS": {"pkmn": [(Y, "gold")], "moe": [(Y, "gold")]},
    "SPECIES_MURKROW": {"both": [(Y, "gold"), (O, "gold")]},
    "SPECIES_AIPOM": {"moe": [(Y, "gold"), (O, "gold")]},
    "SPECIES_ARIADOS": {"pkmn": [(Y, "gold")], "moe": [(Y, "gold")]},
    "SPECIES_RAIKOU": {"moe": [(Y, "gold")], "pkmn": [(L, "under")]},
    "SPECIES_HERACROSS": {"pkmn": [(Y, "gold")]},
    "SPECIES_SNEASEL": {"both": [(R, "crimson"), (Y, "gold")]},
    # red that should stay red: blades, crests, shells
    "SPECIES_SKARMORY": {"both": [(R, "crimson"), (O, "crimson")]},
    "SPECIES_CROCONAW": {"both": [(R, "crimson")]},
    "SPECIES_SHUCKLE": {"pkmn": [(R, "crimson")]},
    "SPECIES_LEDIAN": {"moe": [(R, "crimson")]},
    "SPECIES_YANMA": {"pkmn": [(R, "crimson"), (O, "glow"), (L, "under")]},
    # glowing pearls, orbs and paint
    "SPECIES_MISDREAVUS": {"both": [(R, "glow"), (P, "glow")]},
    "SPECIES_SUDOWOODO": {"both": [(G, "glow")]},
    "SPECIES_SMEARGLE": {"pkmn": [(G, "glow")], "moe": [(G, "glow")]},
    # pale as Shadow Lugia's own underside
    "SPECIES_ABSOL": {"both": [(L, "under")]},
    "SPECIES_NOCTOWL": {"pkmn": [(Y, "under")]},
    "SPECIES_SWABLU": {"both": [(L, "under")]},
    "SPECIES_TOGETIC": {"both": [(R, "crimson"), (B, "cyan")]},
    "SPECIES_DELIBIRD": {"both": [(L, "under")], "moe": [(R, "body")], "pkmn": [((2, 3, 4, 5), "body")]},
    # ice and crystal
    "SPECIES_VIBRAVA": {"pkmn": [(G, "cyan")]},
    "SPECIES_TYRANITAR": {"pkmn": [(B, "cyan")]},
}


def lum_of(rgb):
    return (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]) / 255.0


def ramp(r, t):
    t = max(0.0, min(1.0, t)) * (len(r) - 1)
    k = min(int(t), len(r) - 2)
    t -= k
    return tuple(a + (b - a) * t for a, b in zip(r[k], r[k + 1]))


def family(rgb, name):
    h, sat, val = colorsys.rgb_to_hsv(*(v / 255.0 for v in rgb))
    return FAMILIES[name](h * 360, sat, val, lum_of(rgb))


def materials(colours, px, counts, slots):
    """The picture's materials: SLOTS joined where shades of one hue (or greys) touch each other in the picture, so a
    gradient (a body's shading, a flame's) is one ramp and never split between two roles (stray pixels)."""
    adj = [[0] * 16 for _ in range(16)]
    for y in range(64):
        for x in range(64):
            a = px[y][x] & 15
            for dx, dy in ((1, 0), (0, 1)):
                if x + dx < 64 and y + dy < 64:
                    b = px[y + dy][x + dx] & 15
                    if a != b:
                        adj[a][b] += 1
                        adj[b][a] += 1
    hsv = {i: colorsys.rgb_to_hsv(*(v / 255.0 for v in colours[i])) for i in slots}
    parent = {i: i for i in slots}

    def find(i):
        while parent[i] != i:
            i = parent[i]
        return i

    total = max(1, sum(counts[1:]))

    def alike(i, j, tol=22, step=True):
        """I and J can be shades of one material: a step of a gradient (STEP: the two touching shades) or any two
        shades of a group (hue only: a gradient's dark end is more saturated than its light end)"""
        (hi, si, vi), (hj, sj, vj) = hsv[i], hsv[j]
        if step and abs(lum_of(colours[i]) - lum_of(colours[j])) > 0.3:
            return False
        d = abs(hi - hj) * 360
        d = min(d, 360 - d)
        if si < 0.2 and sj < 0.2:
            # greys: tinted ones only with their own tint
            return si < 0.06 or sj < 0.06 or d < 40
        if si < 0.2 or sj < 0.2 or step and abs(si - sj) > 0.4:
            return False
        # a small spot much more saturated than what it sits in (an eye, a cheek) stays apart
        for x, y in ((i, j), (j, i)) if step else ():
            if counts[x] / total < 0.025 and hsv[x][1] > hsv[y][1] + 0.18 and hsv[x][2] > 0.5:
                return False
        return d < tol

    # joined strongest touch first, and only where every shade of both is alike (no chains from yellow to red)
    pairs = sorted(((adj[i][j] / max(1, min(counts[i], counts[j])), i, j) for i in slots for j in slots if i < j),
                   reverse=True)
    for r, i, j in pairs:
        if r <= 0.08:
            break
        a, b = find(i), find(j)
        if a == b:
            continue
        ma = [k for k in slots if find(k) == a]
        mb = [k for k in slots if find(k) == b]
        if alike(i, j) and all(alike(x, y, 30, False) for x in ma for y in mb):
            parent[a] = b
    out = {}
    for i in slots:
        out.setdefault(find(i), []).append(i)
    return list(out.values())


def roles(colours, counts, skin_mask, picks=(), px=None):
    """{slot: role}: each material of the picture (materials()) given a part of Shadow Lugia's scheme by what it covers."""
    total = max(1, sum(counts[1:]))
    role, rest = {}, []
    for i in range(1, 16):
        if counts[i] == 0 and colours[i] == colours[0]:
            continue
        lum = lum_of(colours[i])
        sat = colorsys.rgb_to_hsv(*(v / 255.0 for v in colours[i]))[1]
        if lum < 0.16:
            role[i] = "outline"
        elif skin_mask >> i & 1:
            role[i] = "skin"
        elif counts[i] == 0:
            role[i] = "body"        # (unused slots)
        elif sat < 0.18 and lum > 0.8 and counts[i] / total < 0.04 and lum_of(colours[i]) > 0.9:
            role[i] = "hilite"
        else:
            rest.append(i)
    groups = {}
    for k, mat in enumerate(materials(colours, px, counts, rest)):
        cov = sum(counts[i] for i in mat)
        sat = sum(colorsys.rgb_to_hsv(*(v / 255.0 for v in colours[i]))[1] * counts[i] for i in mat) / max(1, cov)
        val = max(colorsys.rgb_to_hsv(*(v / 255.0 for v in colours[i]))[2] for i in mat)
        hue = colorsys.rgb_to_hsv(*(v / 255.0 for v in colours[max(mat, key=lambda i: counts[i])]))[0] * 360
        red = hue < 20 or hue > 335
        if (cov / total < 0.025 and sat > 0.45 or cov / total < 0.05 and sat > 0.65
                or red and cov / total < 0.06 and sat > 0.5) and val > 0.45:
            for i in mat:
                role[i] = "eye"
        else:
            groups[k] = mat
    # the parts, largest first: the largest the body; of the rest, the lightest large one the underside, then the next
    # large one the hard parts; the others go to the role nearest their lightness
    cover = lambda k: sum(counts[i] for i in groups[k])
    light = lambda k: sum(lum_of(colours[i]) * counts[i] for i in groups[k]) / max(1, cover(k))
    order = sorted(groups, key=cover, reverse=True)
    assigned = {}
    if order:
        assigned[order[0]] = "body"
        big = [k for k in order[1:] if cover(k) / total > 0.06]
        under = max(big, key=light, default=None)
        if under is not None and light(under) > light(order[0]) + 0.08:
            assigned[under] = "under"
        hard = [k for k in order[1:] if k not in assigned and cover(k) / total > 0.03]
        if hard:
            assigned[hard[0]] = "hard"
        for k in order:
            if k not in assigned:
                lk = light(k)
                assigned[k] = "under" if lk > 0.72 else "hard" if lk < 0.35 else "body"
    # the Pokémon's own choices: a family takes a whole material when most of it is that family; slots are exact
    for fam, rl in picks:
        if isinstance(fam, str):
            for k, mat in groups.items():
                if sum(counts[i] for i in mat if family(colours[i], fam)) * 2 > cover(k):
                    assigned[k] = rl
            for i in list(role):
                if role[i] in ("eye", "hilite") and family(colours[i], fam):
                    role[i] = rl
    for k, mat in groups.items():
        for i in mat:
            role[i] = assigned[k]
    for fam, rl in picks:
        if not isinstance(fam, str):
            for i in (fam if isinstance(fam, tuple) else (fam,)):
                role[i] = rl
    return role


def make(colours, px, skin_mask, curated, picks=()):
    counts = [0] * 16
    for row in px:
        for i in row:
            counts[i & 15] += 1
    role = roles(colours, counts, skin_mask, picks, px)
    ramps = {"body": BODY, "under": UNDER, "hard": HARD, "skin": SKIN, "eye": EYE, "outline": OUTLINE,
             "flame": FLAME, "gold": GOLD, "cyan": CYAN, "crimson": CRIMSON, "glow": GLOW}
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
        if rl == "keep":
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
        pkart = pkmn_art(const)
        cur = CURATED.get(const, {})
        pk = PICKS.get(const, {})
        both = pk.get("both", [])
        rows.append((num, const, moe, make(*moe, skins.get(const, 0), cur.get("moe", {}), both + pk.get("moe", [])),
                     pkart, make(*pkart, 0, cur.get("pkmn", {}), both + pk.get("pkmn", []))))
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


def preview(out, rows, scale=2):
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
    im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    im.save(out)


if __name__ == "__main__":
    args = sys.argv[1:]
    prev = None
    if "--preview" in args:
        prev = args[args.index("--preview") + 1]
        del args[args.index("--preview"):args.index("--preview") + 2]
    only = None
    if "--only" in args:
        only = args[args.index("--only") + 1].upper().split(",")
        del args[args.index("--only"):args.index("--only") + 2]
    out = args[0] if args else os.path.join(REPO, "src/data/orre_shadow_palettes.h")
    rows = build()
    write(out, rows)
    if prev:
        preview(prev, [r for r in rows if only is None or r[1][8:] in only], 3 if only else 2)
    print("%d Shadow Pokemon's dark palettes -> %s" % (len(rows), out))
