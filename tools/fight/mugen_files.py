#!/usr/bin/env python3
"""
Moemon Fight: reading a MUGEN pack the way the engine finds it, for the tools that check it (check_chars.py,
balance_audit.py, ingest.py).

  Pack(root)                    the mugen/ folder: its config, screenpack (motif), select list, cabinets
  pack.roster()                 the select list's characters, in order: [Character]
  pack.stages()                 every stage a fight can use: the cabinet's list, [ExtraStages], each character's own
  Character(pack, entry)        a character: .def, its files (cmd, cns/st, stcommon, sprite, anim, sound, palettes)
  char.states()                 {stateno: State} from its state files and stcommon (char's own win, as MUGEN does)
  read_air / read_sff / read_snd / read_ini

Paths are resolved as MUGEN does: relative to the file that names them, then to the mugen folder, then data/.
"""
import os
import re
import struct


def read_text(path):
    with open(path, 'rb') as f:
        return f.read().decode('latin-1')


def lines(text):
    """The text's lines, split at newlines only: str.splitlines() also splits at bytes like 0x85 and 0x1C, which
    are second bytes of Shift-JIS characters in Japanese packs."""
    return [l[:-1] if l.endswith('\r') else l for l in text.split('\n')]


def strip_comment(line):
    """A line without its ; comment (not inside quotes)."""
    out, q = [], False
    for ch in line:
        if ch == '"':
            q = not q
        elif ch == ';' and not q:
            break
        out.append(ch)
    return ''.join(out).strip()


def read_ini(path):
    """[(section, [(key_lower, value, line_number)])] in file order (sections may repeat)."""
    sections = []
    cur = None
    for n, raw in enumerate(lines(read_text(path)), 1):
        line = strip_comment(raw)
        if not line:
            continue
        if line.startswith('[') and ']' in line:
            cur = (line[1:line.index(']')].strip(), [])
            sections.append(cur)
            continue
        if cur is None:
            cur = ('', [])
            sections.append(cur)
        if '=' in line:
            k, v = line.split('=', 1)
            cur[1].append((k.strip().lower(), v.strip(), n))
        else:
            cur[1].append((line.strip().lower(), None, n))
    return sections


def section(sections, name):
    for s, kv in sections:
        if s.lower() == name.lower():
            return {k: v for k, v, _ in kv if v is not None}
    return {}


def unquote(v):
    v = (v or '').strip()
    if len(v) >= 2 and v[0] == '"' and v[-1] == '"':
        v = v[1:-1]
    return v.replace('\\', '/')


class Pack:
    def __init__(self, root='mugen', cabinet='mauville_gc'):
        self.root = os.path.abspath(root)
        self.cabinet = cabinet
        cfg = read_ini(os.path.join(self.root, 'data', 'mugen.cfg'))
        self.cfg = cfg
        cab = {}
        cpath = os.path.join(self.root, 'cabinets.ini')
        if os.path.exists(cpath):
            cab = section(read_ini(cpath), cabinet)
        self.cab = cab
        motif = unquote(cab.get('motif') or section(cfg, 'Options').get('motif') or 'data/system.def')
        self.motif = self.find(motif)
        files = section(read_ini(self.motif), 'Files') if self.motif else {}
        self.select_def = self.find(unquote(files.get('select', 'select.def')), self.motif)
        self.fight_def = self.find(unquote(files.get('fight', 'fight.def')), self.motif)

    def find(self, rel, near=None, subdirs=('', 'data')):
        """The file rel names, from near's folder, then the pack's folder (and data/), case-insensitively."""
        rel = unquote(rel)
        if not rel:
            return None
        bases = []
        if near:
            bases.append(os.path.dirname(near))
        bases += [os.path.join(self.root, s) for s in subdirs]
        for b in bases:
            p = os.path.normpath(os.path.join(b, rel))
            if os.path.exists(p):
                return p
            q = find_ci(p)
            if q:
                return q
        return None

    def select_lines(self):
        """The [Characters] entries: (def path or 'randomselect', options dict, line)."""
        out = []
        if not self.select_def:
            return out
        cur = None
        for n, raw in enumerate(lines(read_text(self.select_def)), 1):
            line = strip_comment(raw)
            if not line:
                continue
            if line.startswith('['):
                cur = line.strip('[]').strip().lower()
                continue
            if cur == 'characters':
                parts = [p.strip() for p in line.split(',')]
                opts = {}
                stage = None
                for p in parts[1:]:
                    if '=' in p:
                        k, v = p.split('=', 1)
                        opts[k.strip().lower()] = v.strip()
                    elif p:
                        stage = p
                out.append((parts[0], stage, opts, n))
        return out

    def roster(self):
        """The characters a fight can have, in select-list order (randomselect left out)."""
        chars = []
        for name, stage, opts, n in self.select_lines():
            if name.lower() == 'randomselect':
                continue
            chars.append(Character(self, name, stage, opts))
        return chars

    def stages(self):
        """Every stage a fight can use: the cabinet's list, else [ExtraStages], plus the characters' own."""
        out = []
        listed = [s.strip() for s in (self.cab.get('stages') or '').split(',') if s.strip()]
        if listed:
            for s in listed:
                p = self.find(s if s.lower().endswith('.def') else 'stages/%s.def' % s)
                if p:
                    out.append(p)
        elif self.select_def:
            cur = None
            for raw in lines(read_text(self.select_def)):
                line = strip_comment(raw)
                if line.startswith('['):
                    cur = line.strip('[]').strip().lower()
                elif line and cur == 'extrastages':
                    p = self.find(line.split(',')[0].strip())
                    if p:
                        out.append(p)
        for c in self.roster():
            if c.stage:
                p = self.find(c.stage)
                if p and p not in out:
                    out.append(p)
        return out


def find_ci(path):
    """path matched case-insensitively component by component (packs come from Windows)."""
    path = os.path.normpath(path)
    parts = path.split(os.sep)
    cur = os.sep if path.startswith(os.sep) else '.'
    for part in parts:
        if not part:
            continue
        nxt = os.path.join(cur, part)
        if os.path.exists(nxt):
            cur = nxt
            continue
        try:
            hits = [e for e in os.listdir(cur) if e.lower() == part.lower()]
        except OSError:
            return None
        if not hits:
            return None
        cur = os.path.join(cur, hits[0])
    return cur


class State:
    def __init__(self, number, file, line):
        self.number = number
        self.file = file
        self.line = line
        self.params = {}            # the [Statedef] lines
        self.controllers = []       # [(label, {key: value}, line)]


class Character:
    def __init__(self, pack, entry, stage=None, opts=None):
        self.pack = pack
        self.entry = entry
        self.stage = stage
        self.opts = opts or {}
        d = entry if entry.lower().endswith('.def') else '%s/%s.def' % (entry, os.path.basename(entry))
        self.def_path = pack.find(d, subdirs=('chars', '', 'data'))
        self.dir = os.path.dirname(self.def_path) if self.def_path else None
        self.ini = read_ini(self.def_path) if self.def_path else []
        self.info = section(self.ini, 'Info')
        self.files = section(self.ini, 'Files')
        self.name = unquote(self.info.get('displayname') or self.info.get('name') or entry)
        self._states = None

    def file(self, key):
        v = self.files.get(key)
        return self.pack.find(v, self.def_path) if v else None

    def state_files(self):
        """The state files in the order MUGEN reads them: st, st1..st9, then stcommon (cns also holds states)."""
        out = []
        keys = ['cns', 'st'] + ['st%d' % i for i in range(10)]
        for k in keys:
            p = self.file(k)
            if p and p not in out:
                out.append(p)
        common = self.file('stcommon')
        if common and common not in out:
            out.append(common)
        return out, common

    def dependencies(self):
        """Every file the character uses (for change detection)."""
        deps = [self.def_path]
        for k, v in self.files.items():
            p = self.pack.find(v, self.def_path)
            if p:
                deps.append(p)
        return sorted(set(p for p in deps if p))

    def constants(self):
        """The [Data], [Size], [Velocity], [Movement] sections of the cns file."""
        p = self.file('cns')
        if not p:
            return {}
        ini = read_ini(p)
        out = {}
        for name in ('Data', 'Size', 'Velocity', 'Movement'):
            out[name.lower()] = section(ini, name)
        return out

    def states(self):
        """{stateno: State}: the character's own states win over stcommon's (as MUGEN loads them)."""
        if self._states is not None:
            return self._states
        own, common = self.state_files()
        states = {}
        for path in [p for p in own if p != common] + ([common] if common else []):
            for st in parse_states(path):
                states.setdefault(st.number, st)
        self._states = states
        return states


STATEDEF = re.compile(r'^\[\s*statedef\s+(-?\d+)', re.I)
STATE = re.compile(r'^\[\s*state\s+(-?\d+)\s*(?:,\s*(.*?))?\s*\]', re.I)


def parse_states(path):
    """The [Statedef N] / [State N, label] blocks of a cns / st file."""
    out = []
    cur = None
    ctrl = None
    for n, raw in enumerate(lines(read_text(path)), 1):
        line = strip_comment(raw)
        if not line:
            continue
        m = STATEDEF.match(line)
        if m:
            cur = State(int(m.group(1)), path, n)
            out.append(cur)
            ctrl = None
            continue
        m = STATE.match(line)
        if m:
            ctrl = (m.group(2) or '', {}, n)
            if cur is not None:
                cur.controllers.append(ctrl)
            continue
        if line.startswith('['):
            cur = None
            ctrl = None
            continue
        if '=' not in line or cur is None:
            continue
        k, v = line.split('=', 1)
        k = k.strip().lower()
        v = v.strip()
        if ctrl is None:
            cur.params[k] = v
        else:
            if k in ctrl[1] and k.startswith('trigger'):
                ctrl[1][k] = ctrl[1][k] + ' && ' + v
            else:
                ctrl[1][k] = v
    return out


ACTION = re.compile(r'^\[\s*begin\s+action\s+(-?\d+)\s*\]', re.I)


def read_air(path):
    """{action: [(group, number, x, y, time)]} of a .air file."""
    actions = {}
    cur = None
    for raw in lines(read_text(path)):
        line = strip_comment(raw)
        if not line:
            continue
        m = ACTION.match(line)
        if m:
            cur = actions.setdefault(int(m.group(1)), [])
            continue
        if line.startswith('[') or cur is None:
            continue
        low = line.lower()
        if low.startswith('clsn') or low.startswith('loopstart') or low.startswith('interpolate'):
            continue
        parts = [p.strip() for p in line.split(',')]
        if len(parts) >= 5:
            try:
                cur.append((int(parts[0]), int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4])))
            except ValueError:
                pass
    return actions


def read_sff(path):
    """The (group, number) pairs of an SFF v1 or v2 sprite file."""
    with open(path, 'rb') as f:
        data = f.read()
    if data[:12] != b'ElecbyteSpr\x00':
        return set()
    major = data[15]               # version bytes are stored lowest first: 1.01 = 00 01 00 01, 2.00 = 00 00 00 02
    out = set()
    if major == 2:
        spr_off, spr_n = struct.unpack_from('<II', data, 0x24)
        for i in range(spr_n):
            g, n = struct.unpack_from('<HH', data, spr_off + i * 28)
            out.add((g, n))
        return out
    nimg, first = struct.unpack_from('<II', data, 20)
    off = first
    seen = set()
    for _ in range(nimg):
        if off <= 0 or off + 32 > len(data) or off in seen:
            break
        seen.add(off)
        nxt, length, x, y, g, n = struct.unpack_from('<IIhhHH', data, off)
        out.add((g, n))
        off = nxt
    return out


def read_snd(path):
    """The (group, number) pairs of an Elecbyte .snd file."""
    with open(path, 'rb') as f:
        data = f.read()
    if data[:11] != b'ElecbyteSnd':
        return set()
    count, off = struct.unpack_from('<ii', data, 16)
    out = set()
    seen = set()
    while off and off + 16 <= len(data) and len(out) < count + 1 and off not in seen:
        seen.add(off)
        nxt, size, g, n = struct.unpack_from('<iiii', data, off)
        out.add((g, n))
        off = nxt
    return out


class Frame:
    __slots__ = ('group', 'number', 'x', 'y', 'time', 'hit', 'hurt')

    def __init__(self, group, number, x, y, time, hit, hurt):
        self.group, self.number, self.x, self.y, self.time, self.hit, self.hurt = group, number, x, y, time, hit, hurt


def read_air_frames(path):
    """{action: [Frame]} with hit (Clsn1) and hurt (Clsn2) boxes per frame: "ClsnNDefault: n" holds for the frames
    after it, "ClsnN: n" for the next frame only (n = 0 clears)."""
    actions = {}
    cur = None
    default = {1: False, 2: False}
    once = {1: None, 2: None}
    for raw in lines(read_text(path)):
        line = strip_comment(raw)
        if not line:
            continue
        m = ACTION.match(line)
        if m:
            cur = actions.setdefault(int(m.group(1)), [])
            default = {1: False, 2: False}
            once = {1: None, 2: None}
            continue
        if line.startswith('[') or cur is None:
            continue
        low = line.lower().replace(' ', '')
        mm = re.match(r'^clsn([12])(default)?:(\d+)', low)
        if mm:
            k, n = int(mm.group(1)), int(mm.group(3))
            if mm.group(2):
                default[k] = n > 0
                once[k] = None
            else:
                once[k] = n > 0
            continue
        if low.startswith('clsn') or low.startswith('loopstart') or low.startswith('interpolate'):
            continue
        parts = [p.strip() for p in line.split(',')]
        if len(parts) >= 5:
            try:
                g, n, x, y, t = (int(parts[i]) for i in range(5))
            except ValueError:
                continue
            hit = once[1] if once[1] is not None else default[1]
            hurt = once[2] if once[2] is not None else default[2]
            cur.append(Frame(g, n, x, y, t, hit, hurt))
            once = {1: None, 2: None}
    return actions
