#!/usr/bin/env python3
"""Moemon Maker, M4b: the tables the Maker's `raw` command assembles the game's own script macros with at run time
(src/maker/maker_raw.c), so a decompiled event (tools/maker/decompile.py) is the same bytes as the built script.

  asm/macros/event.inc, qz_event.inc, movement.inc  -> every macro: its parameters (name, :req, =default) and its body
                                                       lines (comments stripped), assembled by maker_raw.c the way gas
                                                       would (.byte / .short / .int / ptrvalue / .if / .ifb / .set /
                                                       other macros)
  include/constants/*.h (+ the generated ones), the macro files' own `NAME = value`, the script command numbers
  (SCR_OP_), the specials (SPECIAL_)                -> every constant the scripts name, with its value
  data/event_scripts.s and what it includes          -> every global label (`Name::`), as a pointer, for the scripts
                                                       outside an event (maker_labels.h)

usage: gen_macros.py OUT_macros.h OUT_labels.h   (the Makefile: include/constants/maker_macros.h and maker_labels.h,
                                                  never committed)
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_names  # noqa: E402  (script_ops, specials)


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8', errors='replace') as f:
        return f.read()


def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    out = []
    for line in text.splitlines():
        q, i = False, 0
        while i < len(line):   # '@' starts a comment outside quotes
            if line[i] == '"':
                q = not q
            elif (line[i] == '@' or line.startswith('//', i)) and not q:
                line = line[:i]
                break
            i += 1
        out.append(line.rstrip())
    return out


def c_string(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n') + '"'


# ---- macros
def macros_of(path, macros, syms):
    lines = strip_comments(read(path))
    i = 0
    while i < len(lines):
        line = lines[i].strip()
        m = re.match(r'\.macro\s+(\S+)\s*(.*)$', line)
        if m:
            name, params = m.group(1), m.group(2).strip()
            body, depth = [], 1
            i += 1
            while i < len(lines):
                l = lines[i].strip()
                if l.startswith('.macro'):
                    depth += 1
                elif l.startswith('.endm'):
                    depth -= 1
                    if depth == 0:
                        break
                if l:
                    body.append(l)
                i += 1
            if name.startswith('\\'):
                pass   # (a macro that makes macros: expanded below)
            elif name == 'create_movement_action':
                pass
            else:
                macros[name] = (params, body)
        else:
            m = re.match(r'create_movement_action\s+(\w+)\s*,\s*(\w+)$', line)
            if m:
                macros[m.group(1)] = ('', ['.byte ' + m.group(2)])
            m = re.match(r'([A-Za-z_]\w*)\s*=\s*(.+)$', line)
            if m:
                syms.setdefault(m.group(1), m.group(2).strip())
            m = re.match(r'\.set\s+([A-Za-z_]\w*)\s*,\s*(.+)$', line)
            if m and not m.group(1).startswith('ALLOCATE_'):
                syms.setdefault(m.group(1), m.group(2).strip())
        i += 1


# ---- constants from the C headers
def c_defines(syms):
    paths = [os.path.join('include/constants', f) for f in sorted(os.listdir(os.path.join(ROOT, 'include/constants')))
             if f.endswith('.h') and not f.startswith('maker_')]
    for p in paths:
        text = read(p)
        for line in text.splitlines():
            m = re.match(r'\s*#define\s+([A-Za-z_]\w*)\s+(.+?)\s*(//.*|/\*.*)?$', line)
            if m and '(' not in m.group(1):
                syms.setdefault(m.group(1), m.group(2))
        for body in re.findall(r'\benum\s*\w*\s*\{(.*?)\}', re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S), re.S):
            prev = None   # (an entry without a value is the one before plus one)
            for item in body.split(','):
                item = item.strip()
                m = re.match(r'([A-Za-z_]\w*)\s*(?:=\s*(.+))?$', item, re.S)
                if not m:
                    continue
                expr = m.group(2).strip() if m.group(2) else ('(%s) + 1' % prev if prev else '0')
                syms.setdefault(m.group(1), expr)
                prev = m.group(1)


CAST = re.compile(r'\(\s*(?:u8|u16|u32|s8|s16|s32|int|unsigned|const)\s*\)')


def evaluate(syms):
    """name -> int where the value can be worked out (C expressions of numbers and other names)"""
    done, busy = {}, set()

    def val(name):
        if name in done:
            return done[name]
        if name in busy or name not in syms:
            raise KeyError(name)
        busy.add(name)
        e = CAST.sub('', syms[name]).replace('TRUE', '1').replace('FALSE', '0')
        e = re.sub(r'\b(0x[0-9A-Fa-f]+|\d+)[uUlL]+\b', r'\1', e)
        if re.search(r'["\'{}\[\]?:]', e):
            busy.discard(name)
            raise KeyError(name)
        names = set(re.findall(r'\b[A-Za-z_]\w*\b', e))
        env = {}
        for n in names:
            env[n] = val(n)
        e = e.replace('&&', ' and ').replace('||', ' or ').replace('!', ' not ').replace(' not =', '!=')
        try:
            v = int(eval(e, {'__builtins__': {}}, env))
        except Exception:
            busy.discard(name)
            raise KeyError(name)
        busy.discard(name)
        done[name] = v
        return v

    for n in list(syms):
        try:
            val(n)
        except (KeyError, RecursionError):
            pass
    return done


def script_roots():
    return ['data/' + f for f in sorted(os.listdir(os.path.join(ROOT, 'data')))
            if f.startswith('event_scripts') and f.endswith('.s')]


def walk_scripts(visit):
    """every scripts file the game assembles (data/event_scripts*.s and what they include), once each"""
    seen = set()

    def walk(path):
        if path in seen or not os.path.exists(os.path.join(ROOT, path)):
            return
        seen.add(path)
        text = read(path)
        visit(path, text)
        for line in text.splitlines():
            m = re.match(r'\s*\.include\s+"([^"]+)"', line)
            if m and not m.group(1).startswith('asm/') and not m.group(1).startswith('constants/'):
                walk(m.group(1))
    for r in script_roots():
        walk(r)


# ---- the constants the scripts files set themselves (`.set COINS_PRICE_50, 1000`, `NAME = value`)
def script_syms(syms):
    def visit(path, text):
        for line in strip_comments(text):
            m = re.match(r'\s*\.(?:set|equ)\s+([A-Za-z]\w*)\s*,\s*(.+)$', line) or \
                re.match(r'\s*([A-Za-z]\w*)\s*=\s*(.+)$', line)
            if m and not m.group(1).startswith('SCRIPT_CMD'):
                syms.setdefault(m.group(1), m.group(2).strip())
    walk_scripts(visit)


# ---- the global labels of the HOENN scripts
def labels():
    out = set()

    def visit(path, text):
        for line in text.splitlines():
            m = re.match(r'([A-Za-z_]\w*)::?(?!:)', line)
            if m:
                out.add((m.group(1), line[len(m.group(1)) + 1:len(m.group(1)) + 2] == ':'))
    walk_scripts(visit)
    return sorted(out)


# ---- the game's C names the scripts point at (gStringVar4, a callnative's function, gText_...): the operands of the
# scripts' lines that are neither labels nor constants, kept when src/ defines them (not static)
def c_names(known):
    used = set()

    def visit(path, text):
        if path.endswith('specials.inc'):
            return
        for line in strip_comments(text):
            s = line.strip()
            if not s or s[0] in '#.' and not re.match(r'\.(4byte|int|word|long)\b', s):
                continue
            s = re.sub(r'^[A-Za-z_]\w*::?', '', s).strip()
            s = re.sub(r'"[^"]*"', '', s)
            parts = s.split(None, 1)
            if len(parts) < 2 or parts[0] in ('special', 'specialvar', 'script_cmd_table_entry', 'def_special'):
                continue
            for n in re.findall(r'\b([A-Za-z_]\w*)\b(?!\s*=)', parts[1]):
                if n not in known:
                    used.add(n)
    walk_scripts(visit)
    defined = set()
    decl = re.compile(r'^(?!static\b|extern\b|typedef\b|struct\s+\w+\s*\{|enum\b|#|return\b)[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*(?:\[|\(|=|;)')
    for d, _, files in os.walk(os.path.join(ROOT, 'src')):
        for f in files:
            if not f.endswith('.c'):
                continue
            try:
                with open(os.path.join(d, f), encoding='utf-8', errors='replace') as fh:
                    for line in fh:
                        if line[:1].isalpha():
                            m = decl.match(line)
                            if m and m.group(1) in used:
                                defined.add(m.group(1))
            except OSError:
                pass
    return defined


# ---- C macros with one parameter the scripts use (MAP_NUM(map), ITEM_TO_BERRY(item)): the assembler's own (without ?:)
def c_funcs():
    out = {}
    for f in sorted(os.listdir(os.path.join(ROOT, 'include/constants'))):
        if not f.endswith('.h') or f.startswith('maker_'):
            continue
        for line in read('include/constants/' + f).splitlines():
            m = re.match(r'\s*#define\s+([A-Za-z_]\w*)\((\w+)\)\s+(.+?)\s*(//.*)?$', line)
            if m and '?' not in m.group(3) and not m.group(3).endswith('\\'):
                out[m.group(1)] = (m.group(2), m.group(3))
    return out


def write_if_changed(path, text):
    try:
        with open(path) as f:
            if f.read() == text:
                return
    except OSError:
        pass
    with open(path, 'w') as f:
        f.write(text)


def main():
    out_m, out_l = sys.argv[1], sys.argv[2]
    macros, asm_syms, syms = {}, {}, {}
    files = ['asm/macros/event.inc', 'asm/macros/qz_event.inc', 'asm/macros/movement.inc', 'asm/macros/map.inc',
             'asm/macros/battle_tent.inc', 'asm/macros/trainer_hill.inc']
    files += sorted(os.path.join('asm/macros/battle_frontier', f) for f in os.listdir(os.path.join(ROOT, 'asm/macros/battle_frontier'))
                    if f.endswith('.inc'))
    for p in files:
        macros_of(p, macros, asm_syms)
    for p in ('constants/global.inc', 'constants/gba_constants.inc', 'constants/tms_hms.inc'):
        macros_of(p, {}, asm_syms)
    walk_scripts(lambda path, text: '.macro' in text and macros_of(path, macros, {}))   # (a facility's own macros)
    c_defines(syms)
    for k, v in asm_syms.items():
        if not k.startswith('_'):
            syms[k] = v
    script_syms(syms)
    values = evaluate(syms)
    funcs = c_funcs()
    n = 0
    for line in read('data/script_cmd_table.inc').splitlines():
        m = re.match(r'\s*script_cmd_table_entry\s+(SCR_OP_\w+)', line)
        if m:
            values[m.group(1)] = n
            n += 1
    k = 0
    for line in gen_names.specials():   # (in order: a special's number is its place in the table)
        m = re.search(r'\{"(\w+)",\s*(\d+)\}', line)
        if m:
            values['SPECIAL_' + m.group(1)] = k
            values['SPECIAL_WAITSTATE_' + m.group(1)] = int(m.group(2))
            k += 1
    # the TMs and HMs by move (constants/tms_hms.inc: ITEM_TM_<MOVE> counts up from ITEM_TM01, the HMs from ITEM_HM01;
    # Kai's TM51-92 / TM93-139 from ITEM_TM51 / ITEM_TM93 when their lists say so)
    for hdr, macro, base, prefix in (('tms_hms.h', 'FOREACH_TM', 'ITEM_TM01', 'ITEM_TM_'), ('tms_hms.h', 'FOREACH_HM', 'ITEM_HM01', 'ITEM_HM_'),
                                     ('tms_hms_tm2.h', 'FOREACH_TM2', 'ITEM_TM51', 'ITEM_TM_'),
                                     ('tms_hms_tm3.h', 'FOREACH_TM3', 'ITEM_TM93', 'ITEM_TM_')):
        text = read('include/constants/' + hdr)
        m = re.search(r'#define\s+%s\(F\)((?:\s*\\\s*\n\s*F\(\w+\))+)' % macro, text)
        if not m or base not in values:
            continue
        for i, name in enumerate(re.findall(r'F\((\w+)\)', m.group(1))):
            values.setdefault(prefix + name, values[base] + i)
    # the string variables (the charmap's: the macros only compare them with each other) and the basics
    values.update({'STR_VAR_1': 0x1FD02, 'STR_VAR_2': 0x1FD03, 'STR_VAR_3': 0x1FD04, 'TRUE': 1, 'FALSE': 0,
                   'NULL': 0})
    with open(out_m, 'w') as f:
        f.write('// Generated by tools/maker/gen_macros.py from asm/macros/*.inc and include/constants: do not edit\n')
        f.write('struct MakerMacro { const char *name, *params, *body; };\n')
        f.write('static const struct MakerMacro sMakerMacros[] = {\n')
        for name in sorted(macros):
            params, body = macros[name]
            f.write('    {%s, %s, %s},\n' % (c_string(name), c_string(params), c_string('\n'.join(body))))
        f.write('};\n')
        f.write('struct MakerSym { const char *name; int value; };\n')
        f.write('static const struct MakerSym sMakerSyms[] = {\n')
        for name in sorted(values):
            f.write('    {%s, %d},\n' % (c_string(name), values[name] if -2**31 <= values[name] < 2**31 else 0))
        f.write('};\n')
        f.write('struct MakerFunc { const char *name, *param, *body; };\n')
        f.write('static const struct MakerFunc sMakerFuncs[] = {\n')
        for name in sorted(funcs):
            f.write('    {%s, %s, %s},\n' % (c_string(name), c_string(funcs[name][0]), c_string(funcs[name][1])))
        f.write('};\n')
    labs = labels()
    if len(sys.argv) > 3:   # data/maker_globals.inc: the file-local labels (texts, movements) made global for the Maker,
        # each where it is defined (every event_scripts*.s includes this file at its end)
        text = '@ Generated by tools/maker/gen_macros.py: the Maker reaches these labels (src/maker/maker_raw.c)\n'
        for name, glob in labs:
            if not glob:
                text += '.ifdef %s\n.global %s\n.endif\n' % (name, name)
        write_if_changed(sys.argv[3], text)
    names = set(n for n, g in labs)
    known = names | set(values) | set(macros) | set(funcs)
    names |= c_names(known)
    labs = sorted(names)
    text = '// Generated by tools/maker/gen_macros.py from data/event_scripts*.s: do not edit (src/maker/maker_labeltab.c)\n'
    text += ''.join('extern const u8 %s[];\n' % l for l in labs)
    text += 'const struct MakerLabel gMakerLabels[] = {\n'
    text += ''.join('    {"%s", %s},\n' % (l, l) for l in labs)
    text += '};\nconst int gMakerLabelCount = %d;\n' % len(labs)
    write_if_changed(out_l, text)
    print('%d macros, %d constants, %d labels' % (len(macros), len(values), len(labs)), file=sys.stderr)


if __name__ == '__main__':
    main()
