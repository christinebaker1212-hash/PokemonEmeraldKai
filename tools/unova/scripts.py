#!/usr/bin/env python3
"""Black's scripts (docs/UNOVA_HANDOFF.md "Black's scripts"), read as aletheia's script_sys.c runs them: archive 0x39
(a/0/5/7), a file per zone (its header's script arc id) and 46 shared files by script id ranges (ov010 d_021701A8:
u16 first id, last id, file, text arc (3: the story banks), text file). A file begins with u32 offsets, one a script,
ended by u16 0xFD13; script n starts 4 * n + 4 + its offset in. A zone's script id 1 is its file's script 0; a shared
range's first id its file's 0. Commands: tools/unova/scrcmd.py; jumps and calls are u32 offsets from after the
offset.

    python3 -I tools/unova/scripts.py check            (decode every file: the commands' arguments hold?)
    python3 -I tools/unova/scripts.py zone ZONE        (a zone's scripts, decoded)
    python3 -I tools/unova/scripts.py file FILE        (a script file, decoded)
"""
import functools
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import narc  # noqa: E402
import scrcmd  # noqa: E402
import zones as zr  # noqa: E402

ARC = 'a/0/5/7'
RANGES_AT, RANGES = 0x021701A8, 46
END_MARK = 0xFD13
CODE_TARGETS = {'SCRIPT_WORK_CmdJump': 0, 'SCRIPT_WORK_CmdCall': 0, 'SCRIPT_WORK_CmdIfJump': 1,
                'SCRIPT_WORK_CmdIfCall': 1}
ENDS = {'SCRIPT_WORK_CmdEnd', 'SCRIPT_WORK_CmdJump', 'SCRIPT_WORK_CmdReturn', 'SCRIPT_WORK_CmdHalt',
        'SCRIPT_WORK_CmdChangeScript'}
SIZES = {'v': 2, 'p': 2, 'h': 2, 'w': 4, 'b': 1}


@functools.lru_cache(None)
def ranges():
    ram, d = scrcmd.ov_image(10)
    return [struct.unpack_from('<5H', d, RANGES_AT - ram + 10 * i) for i in range(RANGES)]


def script_file(zone, script_id):
    """(file, index in it, text arc, text file) of a script id as the zone runs it"""
    for first, last, f, arc, msg in ranges():
        if first <= script_id <= last:
            return f, script_id - first, arc, msg
    z = zr.zones()[zone]
    return z['script'], script_id - 1, 3, z['msg']


@functools.lru_cache(None)
def entries(f):
    """The file's scripts' start offsets"""
    d = narc.arc(ARC)[f]
    out, o = [], 0
    while o + 2 <= len(d) and struct.unpack_from('<H', d, o)[0] != END_MARK and o + 4 <= len(d):
        start = o + 4 + struct.unpack_from('<i', d, o)[0]
        if start < len(d):
            out.append(start)
        o += 4
    return out


@functools.lru_cache(None)
def init_files():
    """The files zones' headers name as init scripts (tables of scene / init scripts, not code)"""
    return {z['init_script'] for z in zr.zones()}


@functools.lru_cache(None)
def table():
    return {i: (name, args) for i, name, args, _ in scrcmd.table()}


def decode(f, starts=None):
    """{offset: (cmd id, name, [args], next offset)} reached from STARTS (default: every script), plus the errors and
    the offsets jumps and movements point at"""
    d = narc.arc(ARC)[f]
    cmds, errors, moves = {}, [], set()
    todo = list(starts if starts is not None else entries(f))
    while todo:
        o = todo.pop()
        while o not in cmds:
            if o < 0 or o + 2 > len(d):
                errors.append((o, 'outside'))
                break
            op = struct.unpack_from('<H', d, o)[0]
            if op not in table():
                errors.append((o, 'op %04X' % op))
                break
            name, sig = table()[op]
            p, args = o + 2, []
            for k in sig:
                n = SIZES[k]
                if p + n > len(d):
                    break
                args.append(int.from_bytes(d[p:p + n], 'little', signed=(k == 'w')))
                p += n
            cmds[o] = (op, name, args, p)
            if name in CODE_TARGETS:
                todo.append(p + args[CODE_TARGETS[name]])
            elif 'w' in sig and name not in CODE_TARGETS:
                k = sig.index('w')
                moves.add(o + 2 + sum(SIZES[c] for c in sig[:k + 1]) + args[k])
            if name in ENDS:
                break
            o = p
    return cmds, errors, moves


def check():
    bad, total = 0, 0
    from collections import Counter
    culprits = Counter()
    for f in range(len(narc.arc(ARC))):
        if f in init_files():
            continue
        try:
            cmds, errors, _ = decode(f)
        except Exception as e:   # noqa: BLE001
            errors, cmds = [(0, repr(e))], {}
        total += 1
        if errors:
            bad += 1
            # the command just before the first error is the likely culprit
            o = errors[0][0]
            prev = max((k for k in cmds if cmds[k][3] == o), default=None)
            if prev is not None:
                culprits[cmds[prev][1]] += 1
    print('%d of %d files decode with errors' % (bad, total))
    for name, n in culprits.most_common(30):
        print('  %4d after %s %s' % (n, name, next(a for i, (nm, a) in table().items() if nm == name)))


def show(f, starts=None, label=''):
    cmds, errors, moves = decode(f, starts)
    ents = entries(f)
    for o in sorted(cmds):
        if o in ents:
            print('-- script %d (%s%d)' % (ents.index(o), label, ents.index(o) + 1))
        op, name, args, _ = cmds[o]
        print('%05X %-44s %s' % (o, name.replace('SCRIPT_WORK_Cmd', '').replace('SCRCMD_', ''),
                                 ' '.join('%X' % (a & 0xFFFFFFFF) for a in args)))
    for o, why in errors:
        print('!! %05X %s' % (o, why))


def main():
    if sys.argv[1] == 'check':
        check()
    elif sys.argv[1] == 'zone':
        z = zr.zones()[int(sys.argv[2])]
        show(z['script'])
    else:
        show(int(sys.argv[2]))


if __name__ == '__main__':
    main()
