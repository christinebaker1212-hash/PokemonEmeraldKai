"""Graphics helpers: LZ77 (BIOS 0x10), 4bpp tiles -> indexed PNG, GBA palettes -> JASC .pal."""
import struct

try:
    from PIL import Image
except ImportError:      # PNGs are skipped without Pillow; .4bpp files are always written
    Image = None


def lz77(d, o):
    """Decompress GBA BIOS LZ77 data at offset o. Returns (bytes, compressed length)."""
    if d[o] != 0x10:
        raise ValueError('not LZ77 at 0x%X' % o)
    size = d[o + 1] | d[o + 2] << 8 | d[o + 3] << 16
    out = bytearray()
    p = o + 4
    while len(out) < size:
        flags = d[p]
        p += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                b1, b2 = d[p], d[p + 1]
                p += 2
                n = (b1 >> 4) + 3
                disp = ((b1 & 15) << 8 | b2) + 1
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(d[p])
                p += 1
    return bytes(out[:size]), p - o


def gba_colors(d, o, n=16):
    out = []
    for i in range(n):
        c = struct.unpack_from('<H', d, o + 2 * i)[0]
        r, g, b = c & 31, (c >> 5) & 31, (c >> 10) & 31
        out.append((r << 3 | r >> 2, g << 3 | g >> 2, b << 3 | b >> 2))
    return out


def write_jasc(path, colors):
    with open(path, 'w', newline='\r\n') as fh:
        fh.write('JASC-PAL\n0100\n%d\n' % len(colors))
        for r, g, b in colors:
            fh.write('%d %d %d\n' % (r, g, b))


def tiles_png(path, data4bpp, colors, width_tiles=16, bpp=4):
    """Write 4bpp (or 8bpp) tile data as an indexed PNG, width_tiles tiles across."""
    tsize = 32 if bpp == 4 else 64
    n = len(data4bpp) // tsize
    rows = (n + width_tiles - 1) // width_tiles
    w, h = width_tiles * 8, max(rows, 1) * 8
    px = bytearray(w * h)
    for t in range(n):
        tx, ty = (t % width_tiles) * 8, (t // width_tiles) * 8
        base = t * tsize
        for y in range(8):
            for x in range(8):
                if bpp == 4:
                    b = data4bpp[base + y * 4 + x // 2]
                    v = (b >> 4) if x & 1 else (b & 15)
                else:
                    v = data4bpp[base + y * 8 + x]
                px[(ty + y) * w + tx + x] = v
    if Image is None:
        return False
    im = Image.frombytes('P', (w, h), bytes(px))
    pal = []
    for c in colors:
        pal += list(c)
    pal += [0] * (768 - len(pal))
    im.putpalette(pal)
    im.save(path, optimize=True)
    return True


def sprite_png(path, data, colors, w, h, bpp=4):
    """Write a w x h sprite frame (tiles in row order, as OAM 1D mapping stores them)."""
    return tiles_png(path, data, colors, width_tiles=w // 8, bpp=bpp)
