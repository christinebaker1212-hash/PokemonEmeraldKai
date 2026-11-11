#!/usr/bin/env python3
"""UNOVA (docs/UNOVA_HANDOFF.md): a NitroSDK NARC (BTAF / BTNF / GMIF) as a list of its files, and Black's ROM
unpacked by aletheia's tools/ndsrom.py (default ~/.cache/moemon/unova/extract). Read-only; nothing here is committed
output."""
import os
import struct

EXTRACT = os.path.expanduser(os.environ.get('UNOVA_EXTRACT', '~/.cache/moemon/unova/extract'))


def files(data):
    """The NARC's files, in order (bytes each)"""
    assert data[:4] == b'NARC', 'not a NARC'
    off = struct.unpack_from('<H', data, 0x0C)[0]
    out, starts = [], []
    while off < len(data):
        magic, size = data[off:off + 4], struct.unpack_from('<I', data, off + 4)[0]
        if magic == b'BTAF':
            n = struct.unpack_from('<I', data, off + 8)[0]
            starts = [struct.unpack_from('<II', data, off + 12 + 8 * i) for i in range(n)]
        elif magic == b'GMIF':
            body = off + 8
            out = [data[body + a:body + b] for a, b in starts]
        off += size
    return out


def arc(path):
    """a/0/1/2 -> its files"""
    return files(open(os.path.join(EXTRACT, 'data', path), 'rb').read())
