#!/usr/bin/env python3
"""Black's script commands (docs/UNOVA_HANDOFF.md "Black's scripts"): the field script VM's table (ov010 d_0217062C,
d_021705D8 entries; aletheia's script_sys.c SCRIPT_SYS_CreateVm) read from the user's ROM, each entry named by the
decomp's symbols (layout/function_addrs.tsv), and each command's arguments read from its decompiled body, in the order
the body reads them:
  v  u16, a work (0x4000 and up: a variable) or a constant   SCRIPT_WORK_ReadWorkValue
  p  u16, a work to write                                     SCRIPT_WORK_ReadWorkPointer
  h  u16                                                      VM_ReadU16
  w  u32 (a jump's offset, a movement list's)                 VM_GetU32
  b  u8                                                       a direct read of the cursor
Calls the body makes with the VM are followed (a command's helper reading the rest). OVERRIDES fixes what the bodies
don't say plainly (checked by decoding every script: tools/unova/scripts.py).

    python3 -I tools/unova/scrcmd.py                (the table: id, name, arguments)
"""
import functools
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import narc  # noqa: E402

CACHE = os.path.dirname(narc.EXTRACT)
ALETHEIA = os.path.join(CACHE, 'aletheia')
OV010_TABLE, OV010_COUNT = 0x0217062C, 0x021705D8

# Commands whose bodies read the cursor their own way (checked against the scripts)
OVERRIDES = {
    'SCRIPT_WORK_CmdIfJump': 'bw',
    'SCRIPT_WORK_CmdIfCall': 'bw',
    'SCRIPT_WORK_CmdCall': 'w',
    'SCRIPT_WORK_CmdJump': 'w',
    'SCRIPT_WORK_CmdReportSave': 'p',       # (its result: the scripts give it one; the body's waiter writes it)
    'SCRCMD_PALPARK_GetSaveValue': 'bp',    # (reads the cursor as "core")
}


def ov_image(n):
    """(RAM address, decompressed bytes) of ARM9 overlay N"""
    sys.path.insert(0, os.path.join(ALETHEIA, 'tools'))
    import blz
    y9 = open(os.path.join(narc.EXTRACT, 'y9.bin'), 'rb').read()
    _, ram, _, _, _, _, _, flags = struct.unpack_from('<8I', y9, n * 32)
    d = open(os.path.join(narc.EXTRACT, 'overlays', 'y9_%04d.bin' % n), 'rb').read()
    if (flags >> 24) & 1:
        d = blz.decompress(d)
    return ram, d


FIELD_REGIONS = ('ov010', 'ov021', 'arm9')   # (what is loaded while a field script runs: overlays share addresses)


@functools.lru_cache(None)
def symbols():
    """{address: (name, source file)}, the field's overlays' names first"""
    out = {}
    for line in open(os.path.join(ALETHEIA, 'layout', 'function_addrs.tsv'), encoding='utf-8'):
        p = line.rstrip('\n').split('\t')
        if len(p) > 5 and p[1].startswith('0x'):
            a = int(p[1], 16)
            if a not in out or (p[0] in FIELD_REGIONS and out[a][2] not in FIELD_REGIONS):
                out[a] = (p[2], p[5], p[0])
    return {a: (n, f) for a, (n, f, _) in out.items()}


@functools.lru_cache(None)
def by_name():
    return {name: src for name, src in symbols().values()}


@functools.lru_cache(None)
def source(path):
    try:
        return open(os.path.join(ALETHEIA, path), encoding='utf-8', errors='replace').read()
    except OSError:
        return ''


def black_only(text):
    """The text as Black builds it: White's branches and Japanese-only blocks left out"""
    while True:
        m = re.search(r'if\s*\(\s*PM_VERSION_IS\(VERSION_WHITE\)\s*\)\s*\{', text)
        if not m:
            break
        i, depth = m.end() - 1, 0
        for j in range(i, len(text)):
            depth += text[j] == '{'
            depth -= text[j] == '}'
            if depth == 0:
                break
        rest = text[j + 1:]
        e = re.match(r'\s*else\b', rest)
        text = text[:m.start()] + (rest[e.end():] if e else rest)
    text = re.sub(r'#if LANGUAGE == LANGUAGE_JAPANESE.*?#else(.*?)#endif', r'\1', text, flags=re.S)
    return re.sub(r'#if LANGUAGE == LANGUAGE_JAPANESE.*?#endif', '', text, flags=re.S)


def body(name):
    """The function's body text (None: not found as C)"""
    src = source(by_name().get(name, ''))
    for m in re.finditer(r'^[A-Za-z_][\w \*]*?\b%s\s*\(([^;{]*)\)\s*\{' % re.escape(name), src, re.M):
        # (to the closing brace at the start of a line: the decomp's functions are written so, and a brace in a
        # comment or a character can't miscount)
        end = re.compile(r'^\}', re.M).search(src, m.end())
        one_line = re.match(r'[^\n]*\}\s*$', src[m.end() - 1:src.find('\n', m.end()) + 1])
        if one_line:
            return black_only(src[m.end() - 1:src.find('\n', m.end())])
        if end:
            return black_only(src[m.end() - 1:end.end()])
    return None


_READ = re.compile(r'SCRIPT_WORK_ReadWorkValue\s*\(|SCRIPT_WORK_ReadWorkPointer\s*\(|VM_ReadU16\s*\(|VM_GetU32\s*\('
                   r'|VM_ReadU32\s*\(|VM_ReadU8\s*\(|ScriptVm_ReadU8\s*\(|ScriptVm_ReadU16\s*\('
                   r'|vm->pc\s*=\s*[^;]*\+\s*(\d)\s*;|vm->pc\s*\+=\s*(\d)\s*;|vm->pc\+\+'
                   r'|\b([A-Za-z][A-Za-z0-9]*_[A-Za-z0-9_]+)\s*\(\s*vm\s*[,)]')
_KIND = {'SCRIPT_WORK_ReadWorkValue': 'v', 'SCRIPT_WORK_ReadWorkPointer': 'p', 'VM_ReadU16': 'h', 'VM_GetU32': 'w',
         'VM_ReadU32': 'w', 'VM_ReadU8': 'b', 'ScriptVm_ReadU8': 'b', 'ScriptVm_ReadU16': 'h'}
_STEP = {'1': 'b', '2': 'h', '4': 'w'}
_NOT_READS = {'VM_SetWait', 'VM_End', 'VM_Jump', 'VM_Call', 'VM_PushStack', 'VM_PopStack', 'VM_GetContext',
              'VM_Return', 'VM_Control'}


def signature(name, depth=0):
    """(args, sure): the arguments NAME reads; sure is False where a body couldn't be read or reads the cursor itself"""
    if name in OVERRIDES:
        return OVERRIDES[name], True
    b = body(name)
    if b is None or depth > 4:
        return '', False
    out, sure = [], True
    for m in _READ.finditer(b):
        fn = m.group(0).split('(')[0].strip()
        if fn in _KIND:
            out.append(_KIND[fn])
        elif m.group(1) or m.group(2):
            out.append(_STEP.get(m.group(1) or m.group(2), '?'))
        elif m.group(0).startswith('vm->pc++'):
            out.append('b')
        elif m.group(3) and m.group(3) not in _NOT_READS and m.group(3) != name:
            sub, s = signature(m.group(3), depth + 1)
            out.append(sub)
            sure = sure and s
    return ''.join(out), sure


@functools.lru_cache(None)
def table():
    """[(id, name, args, sure)]"""
    ram, d = ov_image(10)
    count = struct.unpack_from('<I', d, OV010_COUNT - ram)[0]
    out = []
    for i, a in enumerate(struct.unpack_from('<%dI' % count, d, OV010_TABLE - ram)):
        name = symbols().get(a & ~1, ('cmd_%03X' % i, ''))[0]
        args, sure = signature(name)
        out.append((i, name, args, sure))
    return out


def main():
    unsure = 0
    for i, name, args, sure in table():
        print('%03X %-50s %-8s%s' % (i, name, args, '' if sure else ' ?'))
        unsure += not sure
    print('%d commands, %d unsure' % (len(table()), unsure), file=sys.stderr)


if __name__ == '__main__':
    main()
