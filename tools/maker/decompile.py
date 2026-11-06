#!/usr/bin/env python3
"""Moemon Maker, M4b: the game's own scripts as Maker events (docs/MAKER_HANDOFF.md "M4b").

Each block of a scripts file (a label and its lines, up to the next label) becomes an event of `raw` lines: the game's
own macros, assembled at run time by src/maker/maker_raw.c into the same bytes. Labels outside the block stay
pointers to the game's (its texts, movements and other scripts: the same addresses).

  decompile.py --proof OUT.txt     every block of the game's scripts (data/event_scripts*.s and what they include) as
                                   "@ LABEL" and its raw lines, for the in-game proof ("mp makerproof OUT.txt":
                                   tools/test/maker_decompile.sh), which compiles each and walks it beside the built
                                   script
  decompile.py MAP_JSON_DIR        a map's scripts as events (stdout), e.g. data/maps/LittlerootTown
  decompile.py --pack OUT.h        every person's, sign's and trigger's script of every map as the body of a Maker
                                   event (its branches, movements and texts it reaches in its own file copied in, the
                                   texts as quoted strings where the Maker's text says them exactly), zlib-compressed
                                   into a C array for src/maker/maker_scripts.c ("Start from the game's script")
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
LABEL = re.compile(r'^([A-Za-z_]\w*)(::?)\s*(.*)$')


def read(path):
    with open(os.path.join(ROOT, path), encoding='utf-8', errors='replace') as f:
        return f.read()


def clean(text):
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    out = []
    for line in text.splitlines():
        q, i = False, 0
        while i < len(line):
            if line[i] == '"':
                q = not q
            elif (line[i] == '@' or line.startswith('//', i)) and not q:
                line = line[:i]
                break
            i += 1
        out.append(line.strip())
    return out


def blocks(path):
    """(label, global?, [lines]) for each label of a scripts file"""
    out, cur = [], None
    for line in clean(read(path)):
        m = LABEL.match(line)
        if m and not line.startswith('.'):
            cur = (m.group(1), m.group(2) == '::', [])
            out.append(cur)
            if m.group(3):
                cur[2].append(m.group(3))
            continue
        if cur is not None and line:
            cur[2].append(line)
    return out


def script_files():
    """every scripts file the game assembles: data/event_scripts*.s (HOENN, KANTO / JOHTO, SINNOH, ORRE, the
    STADIUM) and what they include"""
    seen, out = set(), []

    def walk(path):
        if path in seen or not os.path.exists(os.path.join(ROOT, path)):
            return
        seen.add(path)
        for line in read(path).splitlines():
            m = re.match(r'\s*\.include\s+"([^"]+)"', line)
            if m and m.group(1).endswith('.inc') and not m.group(1).startswith(('asm/', 'constants/')) \
                    and 'maker_globals' not in m.group(1):
                out.append(m.group(1))
                walk(m.group(1))
    for f in sorted(os.listdir(os.path.join(ROOT, 'data'))):
        if f.startswith('event_scripts') and f.endswith('.s'):
            walk('data/' + f)
    return out


def usable(lines):
    """a block the raw lines can say: macro calls and plain values (texts are data the game keeps: never raw)"""
    for l in lines:
        if l.startswith('.string') or l.startswith('.include') or l.startswith('.align') or l.startswith('.balign') or l.startswith('.ifdef') \
                or l.startswith('.if') or l.startswith('.endif') or l.startswith('.else') or l.startswith('.set') \
                or l.startswith('.macro') or l.startswith('.incbin') or l.startswith('#'):
            return False
    return bool(lines)


def proof(out_path):
    n = skipped = 0
    with open(out_path, 'w') as f:
        for path in script_files():
            for name, glob, lines in blocks(path):
                if not glob or re.match(r'g[A-Z]', name):   # (gName: a table the C code reads, never a script)
                    continue
                if not usable(lines):
                    skipped += 1
                    continue
                f.write('@ %s\n' % name)
                for l in lines:
                    f.write('raw %s\n' % l)
                n += 1
    print('%d blocks (%d left out: texts and assembler-only data)' % (n, skipped), file=sys.stderr)


TERMINATORS = ('end', 'return', 'goto', 'step_end', 'releaseall_end', 'release_end')
TEXT_OK = re.compile(r"^[A-Za-z0-9 !?.\-/,:'()&+=%\u00e9]*$")


def text_of(lines):
    """the Maker's quoted string for a .string block, or None when the Maker's text can't say it exactly"""
    out = ''
    for l in lines:
        m = re.match(r'\.string\s+"(.*)"\s*$', l)
        if not m:
            return None
        out += m.group(1)
    if not out.endswith('$') or '$' in out[:-1]:
        return None
    out = out[:-1]
    plain = re.sub(r'\\[npl]|\{PLAYER\}', '', out)
    if not TEXT_OK.match(plain) or len(out) > 900:
        return None
    return '"' + out + '"'


class Pack:
    def __init__(self):
        self.files = {}    # path -> [(name, glob, lines)]
        self.where = {}    # label -> (path, index)
        for path in script_files():
            bl = blocks(path)
            self.files[path] = bl
            for i, (name, glob, lines) in enumerate(bl):
                self.where.setdefault(name, (path, i))

    def event(self, entry):
        if entry not in self.where:
            return None
        path, i = self.where[entry]
        bl = self.files[path]
        name, glob, lines = bl[i]
        m = re.match(r'goto\s+(\w+)$', lines[0]) if len(lines) == 1 else None
        if m and m.group(1) in self.where and self.where[m.group(1)][0] != path:   # (a stub: SINNOH's maps)
            path, i = self.where[m.group(1)]
            bl = self.files[path]
        if not usable(bl[i][2]):
            return None
        local = {n: k for k, (n, g, l) in enumerate(bl)}
        texts, want, todo = {}, set(), [i]
        while todo:   # every block of this file the entry reaches (texts inlined, other data left as pointers)
            k = todo.pop()
            if k in want:
                continue
            want.add(k)
            ln = bl[k][2]
            if not self.terminates(ln) and k + 1 < len(bl) and usable(bl[k + 1][2]):
                todo.append(k + 1)   # (it runs on into the next label)
            for l in ln:
                for w in re.findall(r'\b([A-Za-z_]\w*)\b', re.sub(r'"[^"]*"', '', l)):
                    if w in local and local[w] not in want:
                        tk = local[w]
                        if usable(bl[tk][2]):
                            todo.append(tk)
                        elif w not in texts:
                            t = text_of(bl[tk][2])
                            if t:
                                texts[w] = t
        out, done = [], set()

        def emit(k):
            while k not in done:
                done.add(k)
                n, g, ln = bl[k]
                out.append('label ' + n)
                for l in ln:
                    for w, t in texts.items():
                        if w in l:
                            l = re.sub(r'\b%s\b' % re.escape(w), lambda _m: t, l)
                    out.append('raw ' + l)
                if self.terminates(ln) or k + 1 not in want:
                    return
                k += 1
            out.append('raw goto ' + bl[k][0])   # (it ran on into a block already placed)
        emit(i)
        for k in sorted(want):
            if k not in done:
                emit(k)
        return out

    @staticmethod
    def terminates(lines):
        if not lines:
            return False
        w = lines[-1].split()[0] if lines[-1].split() else ''
        return w in TERMINATORS or w.startswith('.')   # (data runs on into nothing)


def scripted(path_json):
    import json
    j = json.loads(read(path_json))
    for o in j.get('object_events', []):
        if o.get('script') and o['script'] != '0x0':
            yield o['script'], ''
    for o in j.get('bg_events', []):
        if o.get('type') == 'sign' and o.get('script'):
            yield o['script'], ''
    for o in j.get('coord_events', []):
        if o.get('type') == 'trigger' and o.get('script'):
            cond = 'var=%s==%s' % (o['var'], o['var_value']) if str(o.get('var', '')).startswith('VAR_') else ''
            yield o['script'], cond


def pack(out_path):
    import zlib
    pk = Pack()
    entries, missing = {}, 0
    for d in sorted(os.listdir(os.path.join(ROOT, 'data/maps'))):
        mj = os.path.join('data/maps', d, 'map.json')
        if not os.path.exists(os.path.join(ROOT, mj)):
            continue
        for label, cond in scripted(mj):
            if label in entries:
                continue
            ev = pk.event(label)
            if ev is None:
                missing += 1
                continue
            entries[label] = (cond, ev)
    blob = b''
    for label in sorted(entries):
        cond, ev = entries[label]
        blob += ('%s %s\n%s\n' % (label, cond, '\n'.join(ev))).encode('utf-8') + b'\0'
    z = zlib.compress(blob, 9)
    text = '// Generated by tools/maker/decompile.py --pack: the game\'s scripts as Maker events (zlib), never committed\n'
    text += 'static const int sMakerScriptsLen = %d;\n' % len(blob)
    text += 'static const unsigned char sMakerScriptsZ[%d] = {\n' % len(z)
    for i in range(0, len(z), 24):
        text += ''.join('%d,' % b for b in z[i:i + 24]) + '\n'
    text += '};\n'
    try:
        with open(out_path) as f:
            if f.read() == text:
                return
    except OSError:
        pass
    with open(out_path, 'w') as f:
        f.write(text)
    print('%d scripts as events (%d bytes, %d compressed; %d left as they are)' % (len(entries), len(blob), len(z), missing),
          file=sys.stderr)


def map_events(map_dir):
    pk = Pack()
    for label, cond in scripted(os.path.join(map_dir, 'map.json')):
        ev = pk.event(label)
        if ev is None:
            continue
        print('event %s' % label)
        print('  page%s' % ((' ' + cond) if cond else ''))
        for l in ev:
            print('    ' + l)


def main():
    if len(sys.argv) == 3 and sys.argv[1] == '--proof':
        proof(sys.argv[2])
    elif len(sys.argv) == 3 and sys.argv[1] == '--pack':
        pack(sys.argv[2])
    elif len(sys.argv) == 2:
        map_events(sys.argv[1])
    else:
        print(__doc__, file=sys.stderr)
        sys.exit(2)


if __name__ == '__main__':
    main()
