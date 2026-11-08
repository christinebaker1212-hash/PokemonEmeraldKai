#!/usr/bin/env python3
"""Pokemon Colosseum's script banks (the GS VM: the Colosseum decomp's src/game/gs_vm.c), read and listed: what ORRE's
interpreter (src/orre/orre_vm.c) runs. For looking at what a room's scripts do, never to write ORRE's scripts.

    python3 -I tools/orre/gsvm.py FILE.scd [FUNC]      (a room's bank: ~/.cache/moemon/orre/unpacked/S1_out/S1_out.scd)
    python3 -I tools/orre/gsvm.py common UNPACKED [FUNC]   (the shared bank 0x0596 in common_rel)

A bank (big endian): u16 id, u16, u16 functions (+ 4), u16 relocations (+ 6), u32 their table (+ 0xC), s32 the
globals (+ 0x10), the functions' offsets (+ 0x18). Code: an opcode byte, then its operands (OPS); 8 pushes a word,
37 calls the native whose index the code pushed first (natives.json beside the decomp's table lbl_802E1CF0), 4 calls a
function of the bank, 7 jumps when the popped value is 0.
"""
import json
import os
import struct
import sys

# opcode: (name, operand bytes)
OPS = {0: ('nop', 0), 1: ('exit', 0), 2: ('push-1', 0), 3: ('pop', 3), 4: ('call', 8), 5: ('return', 0),
       6: ('jump', 4), 7: ('jumpfalse', 4), 8: ('push', 5), 13: ('and', 2), 14: ('or', 2), 15: ('ref', 1),
       16: ('pushn', 3), 17: ('not', 1), 18: ('neg', 1), 19: ('load', 1), 20: ('mul', 2), 21: ('div', 2),
       22: ('mod', 2), 23: ('add', 2), 24: ('sub', 2), 25: ('lt', 2), 26: ('le', 2), 27: ('gt', 2), 28: ('ge', 2),
       29: ('eq', 2), 30: ('ne', 2), 31: ('set', 2), 32: ('copy', 4), 33: ('land', 2), 34: ('lor', 2),
       35: ('printf', 4), 36: ('wait', 4), 37: ('native', 4)}
HERE = os.path.dirname(os.path.abspath(__file__))
COMMON_BANK, COMMON_BANK_SIZE = 0x89CFC, 8120


def natives():
    p = os.path.join(HERE, 'natives.json')
    return [n[0] for n in json.load(open(p))] if os.path.exists(p) else []


def header(d):
    bid, _, nf, nr = struct.unpack_from('>HHHH', d, 0)
    ro, go = struct.unpack_from('>Ii', d, 0xC)
    return bid, list(struct.unpack_from('>%dI' % nf, d, 0x18)), ro, nr, go


def listing(d, func, names):
    _, offs, _, _, _ = header(d)
    ends = sorted(set(offs + [len(d)]))
    o = offs[func]
    end = min(e for e in ends if e > o)
    ip, out, st = o, [], []   # st: what the code pushed (constants, None for the rest), to name the natives
    while ip < end:
        op = d[ip]
        if op not in OPS:
            out.append('%05x: ?? %02x' % (ip, op))
            break
        name, n = OPS[op]
        a = d[ip + 1:ip + 1 + n]
        txt = name
        if op == 8:
            v = struct.unpack_from('>I', a, 1)[0]
            f = struct.unpack_from('>f', a, 1)[0]
            txt = 'push %d' % v if a[0] == 2 or v < 0x10000 else 'push %d (%g)' % (v, f)
            st.append(v)
        elif op in (6, 7):
            txt = '%s %05x' % (name, struct.unpack_from('>I', a)[0])
            if op == 7 and st:
                st.pop()
        elif op == 4:
            fn, argc, loc = struct.unpack_from('>IHH', a)
            txt = 'call func %d (%d args, %d locals)' % (fn, argc, loc)
        elif op == 37:
            argc = struct.unpack_from('>H', a, 2)[0]
            args = st[-argc:] if argc and len(st) >= argc else [None] * argc
            nm = names[args[0]] if args and args[0] is not None and args[0] < len(names) else '?'
            txt = 'native %s(%s)' % (nm, ', '.join('?' if x is None else str(x) for x in args[1:]))
            del st[len(st) - argc:]
            if st:
                st[-1] = None   # (the result slot)
        elif op == 3:
            k = struct.unpack_from('>H', a, 1)[0]
            del st[max(0, len(st) - k):]
            txt = 'pop %d' % k
        elif op in (16, 19, 15, 17, 18):
            if op == 16:
                k = struct.unpack_from('>H', a, 1)[0]
                if st:
                    st.pop()
                st += [None] * (2 * k)
            else:
                if st:
                    st.pop()
                st.append(None)
            txt = '%s %s' % (name, a.hex())
        elif 13 <= op <= 34:
            del st[max(0, len(st) - 2):]
            st.append(None)
            txt = '%s %s' % (name, a.hex())
        elif n:
            txt = '%s %s' % (name, a.hex())
        out.append('%05x: %s' % (ip, txt))
        ip += 1 + n
    return out


def main():
    names = natives()
    if sys.argv[1] == 'common':
        sys.path.insert(0, HERE)
        import colo_data
        d = colo_data._data(sys.argv[2])[COMMON_BANK:COMMON_BANK + COMMON_BANK_SIZE]
        rest = sys.argv[3:]
    else:
        d = open(sys.argv[1], 'rb').read()
        rest = sys.argv[2:]
    bid, offs, ro, nr, go = header(d)
    print('bank %04x: %d functions, %d relocations, globals at %x' % (bid, len(offs), nr, go))
    for f in ([int(rest[0])] if rest else range(len(offs))):
        print('-- function %d (%05x)' % (f, offs[f]))
        print('\n'.join(listing(d, f, names)))


if __name__ == '__main__':
    main()
