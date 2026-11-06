#!/usr/bin/env python3
"""A small glTF binary (.glb) for the Moemon Maker's 3D models test (tools/test/maker_models.sh): a cube one Blender
unit across standing on the ground, its faces in a 4x4 checker texture (PNG), as Blender's exporter writes one.
usage: make_test_glb.py OUT.glb"""
import json
import struct
import sys
import zlib


def png(w, h, rgba):
    def chunk(t, d):
        return struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    raw = b''.join(b'\0' + bytes(rgba[y * w * 4:(y + 1) * w * 4]) for y in range(h))
    return b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) + \
        chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b'')


def main(out):
    tex = []
    for y in range(4):
        for x in range(4):
            tex += [230, 60, 60, 255] if (x + y) % 2 else [250, 220, 80, 255]
    image = png(4, 4, tex)
    pos, nor, uv, idx = [], [], [], []
    faces = [((0, 0, 1), (1, 0, 0), (0, 1, 0)), ((0, 0, -1), (-1, 0, 0), (0, 1, 0)), ((1, 0, 0), (0, 0, -1), (0, 1, 0)),
             ((-1, 0, 0), (0, 0, 1), (0, 1, 0)), ((0, 1, 0), (1, 0, 0), (0, 0, -1)), ((0, -1, 0), (1, 0, 0), (0, 0, 1))]
    for n, u, v in faces:
        base = len(pos)
        for su, sv in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
            p = [0.5 * (n[k] + su * u[k] + sv * v[k]) for k in range(3)]
            p[1] += 0.5   # (standing on the ground)
            pos.append(p)
            nor.append(list(n))
            uv.append([(su + 1) / 2, (1 - sv) / 2])
        idx += [base, base + 1, base + 2, base, base + 2, base + 3]
    bin_ = b''
    views = []

    def add(data):
        nonlocal bin_
        while len(bin_) % 4:
            bin_ += b'\0'
        views.append({'buffer': 0, 'byteOffset': len(bin_), 'byteLength': len(data)})
        bin_ += data
        return len(views) - 1
    vp = add(b''.join(struct.pack('<3f', *p) for p in pos))
    vn = add(b''.join(struct.pack('<3f', *p) for p in nor))
    vt = add(b''.join(struct.pack('<2f', *p) for p in uv))
    vi = add(b''.join(struct.pack('<H', i) for i in idx))
    vimg = add(image)
    while len(bin_) % 4:
        bin_ += b'\0'
    j = {'asset': {'version': '2.0', 'generator': 'make_test_glb.py'},
         'scenes': [{'nodes': [0]}], 'nodes': [{'mesh': 0, 'translation': [0, 0, 0]}],
         'meshes': [{'primitives': [{'attributes': {'POSITION': 0, 'NORMAL': 1, 'TEXCOORD_0': 2}, 'indices': 3,
                                     'material': 0}]}],
         'materials': [{'pbrMetallicRoughness': {'baseColorTexture': {'index': 0}}}],
         'textures': [{'source': 0}], 'images': [{'bufferView': vimg, 'mimeType': 'image/png'}],
         'accessors': [{'bufferView': vp, 'componentType': 5126, 'count': len(pos), 'type': 'VEC3',
                        'min': [-0.5, 0, -0.5], 'max': [0.5, 1, 0.5]},
                       {'bufferView': vn, 'componentType': 5126, 'count': len(nor), 'type': 'VEC3'},
                       {'bufferView': vt, 'componentType': 5126, 'count': len(uv), 'type': 'VEC2'},
                       {'bufferView': vi, 'componentType': 5123, 'count': len(idx), 'type': 'SCALAR'}],
         'bufferViews': views, 'buffers': [{'byteLength': len(bin_)}]}
    js = json.dumps(j).encode()
    while len(js) % 4:
        js += b' '
    data = struct.pack('<4sII', b'glTF', 2, 12 + 8 + len(js) + 8 + len(bin_))
    data += struct.pack('<I4s', len(js), b'JSON') + js + struct.pack('<I4s', len(bin_), b'BIN\0') + bin_
    open(out, 'wb').write(data)


if __name__ == '__main__':
    main(sys.argv[1])
