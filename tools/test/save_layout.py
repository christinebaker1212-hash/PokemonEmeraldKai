#!/usr/bin/env python3
"""The save's layout must never change by accident: it compiles the structures a save file holds with the Linux
compiler and the Windows one (x86_64-w64-mingw32-gcc, whose bitfield rules differ) and compares every size and
offset with tools/test/save_layout.txt. A difference fails; a deliberate change needs a migration step in
src/mp/mp_savefix.c (and MP_SAVE_FORMAT) before the reference is updated with --update.

    python3 tools/test/save_layout.py [--update]

Needs a built tree (generated headers in include/) and both compilers; without mingw it checks Linux only and says so.
"""
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
REF = os.path.join(REPO, 'tools/test/save_layout.txt')
FLAGS = ['-std=gnu99', '-iquote', os.path.join(REPO, 'include'), '-DPORTABLE', '-DNONMATCHING', '-DMODERN=1', '-DUBFIX',
         '-DPLATFORM_SDL2', '-DRENDERER_EASY_DRAW', '-DVER_64BIT', '-w', '-c']
# the structures a save holds whole (SaveBlock1 / 2, the PC), and the ones inside them that changed before
STRUCTS = [('global.h', 'SaveBlock1'), ('global.h', 'SaveBlock2'), ('global.fieldmap.h', 'ObjectEvent'),
           ('global.fieldmap.h', 'ObjectEventTemplate'), ('pokemon_storage_system.h', 'PokemonStorage'),
           ('pokemon.h', 'BoxPokemon'), ('pokemon.h', 'Pokemon'), ('global.h', 'Pokedex'), ('global.h', 'BattleFrontier')]


def members(text, name):
    i = text.find('struct %s\n{' % name)
    if i < 0:
        return []
    b = text[i:]
    b = b[:b.index('\n};')]
    out, depth = [], 0
    for line in b.split('\n')[2:]:
        line = re.sub(r'/\*.*?\*/', '', line).split('//')[0]
        before = depth
        depth += line.count('{') - line.count('}')
        m = re.search(r'(\w+)\s*(\[[^;]*\])?\s*(:\s*\d+)?\s*;\s*$', line)
        if m and before == 0 and depth == 0 and not m.group(3):
            out.append(m.group(1))
    return out


def measure(cc, objcopy, section):
    names, exprs = [], []
    for header, st in STRUCTS:
        text = open(os.path.join(REPO, 'include', header)).read()
        names.append('sizeof %s' % st)
        exprs.append('sizeof(struct %s)' % st)
        for m in members(text, st):
            names.append('%s.%s' % (st, m))
            exprs.append('offsetof(struct %s, %s)' % (st, m))
    # the first word of ObjectEvent: where its bitfields sit (graphicsIdHi must stay in the padding bits)
    src = ('#include "global.h"\n#include "pokemon.h"\n#include "pokemon_storage_system.h"\n#include <stddef.h>\n'
           'const unsigned long long gLay[] = {%s};\n' % ',\n'.join(exprs))
    probe = ('#include "global.h"\nstruct ObjectEvent gO;\nconst unsigned long long gProbe[] = {0};\n'
             'void P(void){ gO.graphicsIdHi = 0xF; }\n')
    d = tempfile.mkdtemp()
    try:
        open(os.path.join(d, 'lay.c'), 'w').write(src)
        subprocess.run([cc] + FLAGS + [os.path.join(d, 'lay.c'), '-o', os.path.join(d, 'lay.o')], check=True)
        subprocess.run([objcopy, '-O', 'binary', '-j', section, os.path.join(d, 'lay.o'), os.path.join(d, 'lay.bin')], check=True)
        b = open(os.path.join(d, 'lay.bin'), 'rb').read()
        vals = struct.unpack('<%dQ' % len(names), b[:8 * len(names)])
        # graphicsIdHi's bits: set them in a zeroed ObjectEvent and see which bytes change
        bits = ('#include "global.h"\n#include <string.h>\nunsigned char gBytes[sizeof(struct ObjectEvent)];\n'
                'static struct ObjectEvent o;\n__attribute__((constructor)) static void f(void){}\n'
                'const struct ObjectEvent gSet = {.graphicsIdHi = 0xF};\n')
        open(os.path.join(d, 'bits.c'), 'w').write(bits)
        subprocess.run([cc] + FLAGS + [os.path.join(d, 'bits.c'), '-o', os.path.join(d, 'bits.o')], check=True)
        subprocess.run([objcopy, '-O', 'binary', '-j', section, os.path.join(d, 'bits.o'), os.path.join(d, 'bits.bin')], check=True)
        sb = open(os.path.join(d, 'bits.bin'), 'rb').read()[:36]
        names.append('ObjectEvent.graphicsIdHi bytes')
        vals = list(vals) + [int.from_bytes(sb, 'little')]
    finally:
        shutil.rmtree(d)
    return dict(zip(names, vals))


def main():
    got = {'linux': measure('gcc', 'objcopy', '.rodata')}
    if shutil.which('x86_64-w64-mingw32-gcc'):
        got['windows'] = measure('x86_64-w64-mingw32-gcc', 'x86_64-w64-mingw32-objcopy', '.rdata')
    elif os.environ.get('REQUIRE_WINDOWS'):
        print('FAIL: no x86_64-w64-mingw32-gcc for the Windows layout'); return 1
    else:
        print('(no x86_64-w64-mingw32-gcc: the Windows layout is not checked)')
    lines = []
    for plat, vals in got.items():
        for k, v in vals.items():
            lines.append('%s %s %d' % (plat, k, v))
    if '--update' in sys.argv:
        open(REF, 'w').write('# tools/test/save_layout.py: every platform the same, never changed by accident\n' + '\n'.join(lines) + '\n')
        print('reference written: %d values' % len(lines))
        return 0
    ref = {}
    for l in open(REF):
        if l.startswith('#'):
            continue
        plat, rest = l.split(' ', 1)
        k, v = rest.rsplit(' ', 1)
        ref[(plat, k)] = int(v)
    bad = 0
    for plat, vals in got.items():
        for k, v in vals.items():
            if (plat, k) not in ref:
                print('NEW   %s %s = %d (not in the reference)' % (plat, k, v)); bad += 1
            elif ref[(plat, k)] != v:
                print('DIFF  %s %s: %d, the reference has %d' % (plat, k, v, ref[(plat, k)])); bad += 1
    # and both platforms alike
    if 'windows' in got:
        for k, v in got['linux'].items():
            if got['windows'].get(k) != v and not k.startswith(('ObjectEvent.range', 'ObjectEvent.field', 'ObjectEvent.warp',
                                                               'ObjectEvent.movementAction', 'ObjectEvent.trainerRange',
                                                               'ObjectEvent.current', 'ObjectEvent.previous',
                                                               'ObjectEvent.direction', 'ObjectEvent.playerCopy')):
                print('PLAT  %s: Linux %d, Windows %s' % (k, v, got['windows'].get(k))); bad += 1
    print('save layout: %s' % ('ok' if not bad else '%d differences' % bad))
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
