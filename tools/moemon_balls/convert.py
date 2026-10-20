# usage: balls.py IN_DIR OUT_ROOT  -> graphics/items/icons/*.png, icon_palettes/*.pal, prints ramps
import sys, os, colorsys
from PIL import Image
IN, OUT = sys.argv[1], sys.argv[2]
names = ["Love","Friend","Fast","Level","Lure","Heavy","Moon","Dream","Heal","Dusk","Quick"]
def q5(c): return tuple(v >> 3 for v in c)
for n in names:
    im = Image.open(os.path.join(IN, n + "Ball.png")).convert("RGBA").crop((4, 4, 28, 28))
    px = im.load()
    opaque = [(x, y) for y in range(24) for x in range(24) if px[x, y][3] >= 128]
    # Quantize the opaque pixels to 15 colors (no dithering); GBA colors are 5 bits a channel
    rgb = Image.new("RGB", (24, 24), (0, 0, 0))
    for x, y in opaque:
        r, g, b, a = px[x, y]
        rgb.putpixel((x, y), (r & 0xF8, g & 0xF8, b & 0xF8))
    cols = sorted(set(rgb.getpixel(p) for p in opaque))
    if len(cols) > 15:
        sub = Image.new("RGB", (len(opaque), 1))
        for i, p in enumerate(opaque):
            sub.putpixel((i, 0), rgb.getpixel(p))
        qi = sub.quantize(colors=15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pal = qi.getpalette()[:45]
        for i, p in enumerate(opaque):
            k = qi.getpixel((i, 0))
            rgb.putpixel(p, tuple(v & 0xF8 for v in pal[k * 3:k * 3 + 3]))
        cols = sorted(set(rgb.getpixel(p) for p in opaque))
    palette = [(180, 180, 180)] + cols
    palette += [(0, 0, 0)] * (16 - len(palette))
    out = Image.new("P", (24, 24), 0)
    flat = []
    for c in palette:
        flat += list(c)
    out.putpalette(flat + [0] * (768 - len(flat)))
    for x, y in opaque:
        out.putpixel((x, y), palette.index(rgb.getpixel((x, y))))
    base = "moemon_" + n.lower() + "_ball"
    out.save(os.path.join(OUT, "graphics/items/icons", base + ".png"), bits=4)
    with open(os.path.join(OUT, "graphics/items/icon_palettes", base + ".pal"), "w", newline="\r\n") as f:
        f.write("JASC-PAL\n0100\n16\n")
        for c in palette:
            f.write("%d %d %d\n" % c)
    # Primary color: the most common saturated color on the top half of the ball
    counts = {}
    ys = [y for _, y in opaque]
    mid = (min(ys) + max(ys)) / 2
    for x, y in opaque:
        if y >= mid:
            continue
        c = rgb.getpixel((x, y))
        h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in c))
        if s < 0.25 or l < 0.12 or l > 0.92:
            continue
        counts[c] = counts.get(c, 0) + 1
    if not counts:   # grey balls (HEAVY): the most common mid tone
        for x, y in opaque:
            if y < mid:
                c = rgb.getpixel((x, y)); l = sum(c) / 765
                if 0.2 < l < 0.85:
                    counts[c] = counts.get(c, 0) + 1
    prim = max(counts, key=counts.get)
    h, l, s = colorsys.rgb_to_hls(*(v / 255 for v in prim))
    ramp = []
    for lv in (0.22, 0.34, l if 0.4 < l < 0.7 else 0.5, 0.68, 0.84):
        r, g, b = colorsys.hls_to_rgb(h, lv, min(1, s))
        ramp.append(tuple(int(v * 31 + 0.5) for v in (r, g, b)))
    print("%-7s colors=%2d primary=%s ramp=%s" % (n, len(cols), prim, " ".join("RGB(%d, %d, %d)" % c for c in ramp)))
