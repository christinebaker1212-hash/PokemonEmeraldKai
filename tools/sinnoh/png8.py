"""A paletted PNG reader without Pillow (the release's runner has only the standard library): Platinum's sprite sheets
and trainer pictures (tools/sinnoh/build_gfx.py) are indexed PNGs of 1, 2, 4 or 8 bits a pixel."""
import struct
import zlib


class Image:
    def __init__(self, path):
        data = open(path, 'rb').read()
        if data[:8] != b'\x89PNG\r\n\x1a\n':
            raise ValueError('%s: not a PNG' % path)
        pos, idat, self.palette = 8, b'', []
        while pos < len(data):
            n, kind = struct.unpack('>I4s', data[pos:pos + 8])
            body = data[pos + 8:pos + 8 + n]
            pos += 12 + n
            if kind == b'IHDR':
                self.w, self.h, depth, ctype, _, _, interlace = struct.unpack('>IIBBBBB', body)
            elif kind == b'PLTE':
                self.palette = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
            elif kind == b'IDAT':
                idat += body
        self.paletted = ctype in (0, 3) and not interlace   # (0: gray levels as the indices, as the decomp's 4bpp sheets)
        self.size = (self.w, self.h)
        self.mode = 'P' if self.paletted else '?'
        if not self.paletted:
            return
        raw = zlib.decompress(idat)
        stride = (self.w * depth + 7) // 8
        bpp = 1
        rows, prev, p = [], bytearray(stride), 0
        for _ in range(self.h):
            f, line = raw[p], bytearray(raw[p + 1:p + 1 + stride])
            p += 1 + stride
            for i in range(stride):
                a = line[i - bpp] if i >= bpp else 0
                b = prev[i]
                c = prev[i - bpp] if i >= bpp else 0
                if f == 1:
                    line[i] = (line[i] + a) & 255
                elif f == 2:
                    line[i] = (line[i] + b) & 255
                elif f == 3:
                    line[i] = (line[i] + (a + b) // 2) & 255
                elif f == 4:
                    pa, pb, pc = abs(b - c), abs(a - c), abs(a + b - 2 * c)
                    line[i] = (line[i] + (a if pa <= pb and pa <= pc else b if pb <= pc else c)) & 255
            prev = line
            if depth == 8:
                rows.append(list(line))
            else:
                per, mask = 8 // depth, (1 << depth) - 1
                rows.append([(line[x // per] >> (8 - depth * (x % per + 1))) & mask for x in range(self.w)])
        self.rows = rows

    def px(self, x, y):
        return self.rows[y][x]

    def getpalette(self):
        return [c for rgb in self.palette for c in rgb]
