#!/usr/bin/env python3
"""Write a save as older builds of Moemon Kai would have stored it, for tools/test/save_compat.sh.

    python3 tools/test/save_variants.py IN.sav OUT.sav KIND

KIND: linux160  1.60.0 on Linux: object event templates with kind at 0x01 and a 16-bit graphics id at 0x02, object
                events' graphics id high byte at 0x23 (Linux padding)
      win160    1.60.0 on Windows: the same templates, and object events 40 bytes long (the high byte at 0x24), so
                SaveBlock1 is 64 bytes longer
The input is a save in today's / 1.59's layout (they're the same but for padding). Only the newest save slot is
rewritten; its sectors keep their order and counter, with fresh checksums. Layout numbers: include/mp_savefix.h,
tools/test/save_layout.txt (SaveBlock1 offsets on 64-bit builds).
"""
import struct
import sys

SECTOR, DATA, PER_SLOT = 0x1000, 3968, 14
SB1_SIZE = 14680          # sizeof(struct SaveBlock1)
OBJ_EVENTS = 2608         # offsetof(struct SaveBlock1, objectEvents)
TEMPLATES = 3184          # offsetof(struct SaveBlock1, objectEventTemplates)
OBJ, NOBJ, TMPL, NTMPL = 36, 16, 32, 64


def checksum(data):
    s = 0
    for i in range(0, len(data) // 4 * 4, 4):
        s = (s + struct.unpack_from('<I', data, i)[0]) & 0xFFFFFFFF
    return ((s >> 16) + s) & 0xFFFF


def main():
    src, dst, kind = sys.argv[1:4]
    d = bytearray(open(src, 'rb').read())
    slots = []
    for slot in range(2):
        secs = {}
        for i in range(PER_SLOT):
            o = (slot * PER_SLOT + i) * SECTOR
            sid, cs, sig, ctr = struct.unpack_from('<HHII', d, o + 0xFF4)
            if sig == 0x8012025:
                secs[sid] = (o, ctr)
        if len(secs) == PER_SLOT:
            slots.append((max(c for _, c in secs.values()), secs))
    ctr, secs = max(slots, key=lambda s: s[0])
    sb1 = bytearray()
    for sid in range(1, 5):
        o = secs[sid][0]
        sb1 += d[o:o + min(DATA, SB1_SIZE - (sid - 1) * DATA)]
    assert len(sb1) == SB1_SIZE
    # templates: localId, graphicsIdLo, kind, graphicsIdHi -> localId, kind, graphicsId (u16)
    for i in range(NTMPL):
        t = TEMPLATES + TMPL * i
        lo, k = sb1[t + 1], sb1[t + 2]
        sb1[t + 1], sb1[t + 2] = k, lo
    events = sb1[OBJ_EVENTS:OBJ_EVENTS + OBJ * NOBJ]
    hi = [0] * NOBJ   # (the test save's objects are all below 0x100)
    if kind == 'linux160':
        for i in range(NOBJ):
            sb1[OBJ_EVENTS + OBJ * i + 0x23] = hi[i]
    elif kind == 'win160':
        wide = bytearray()
        for i in range(NOBJ):
            wide += events[OBJ * i:OBJ * (i + 1)] + bytes([hi[i], 0, 0, 0])
        sb1 = sb1[:OBJ_EVENTS] + wide + sb1[OBJ_EVENTS + OBJ * NOBJ:] + bytes(64)
        sb1 = sb1[:SB1_SIZE + 64]
    else:
        raise SystemExit('KIND: linux160 or win160')
    for sid in range(1, 5):
        o = secs[sid][0]
        chunk = sb1[(sid - 1) * DATA:sid * DATA]
        d[o:o + DATA] = bytes(chunk) + bytes(DATA - len(chunk))
        struct.pack_into('<H', d, o + 0xFF6, checksum(chunk))
    open(dst, 'wb').write(d)
    print('%s: slot counter %d, SaveBlock1 %d bytes' % (kind, ctr, len(sb1)))


if __name__ == '__main__':
    main()
