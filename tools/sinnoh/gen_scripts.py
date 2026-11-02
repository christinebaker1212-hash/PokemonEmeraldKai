#!/usr/bin/env python3
"""SINNOH's scripts: Platinum's own event scripts (pret/pokeplatinum res/field/scripts, at the pinned commit of
fetch_decomp.sh) turned into this game's script commands when the game is built, with Platinum's own words (each
message a placeholder for its line in sinnoh/platinum_text.pak, tools/sinnoh/build_text.py). Like the 3D pack and the
words, none of it is in the repository: the output, sinnoh/platinum_scripts.inc, is made by tools/sinnoh/make_pack.sh
(the release runs it before building the game) and included by data/event_scripts_sinnoh.s.

The repository's maps only point at it: a person / sign / trigger whose Platinum script converts runs
`goto SinnohP_<script file>_<entry number>` (tools/sinnoh/gen_words.py writes those stubs: numbers only, no names, no
words). Without the decomp, `gen_scripts.py --stub` defines every label the maps point at as an `end`, so the game still
builds (SINNOH then says nothing: it needs the packs anyway).

    python3 -I tools/sinnoh/gen_scripts.py POKEPLATINUM_DIR [OUT.inc]     (default sinnoh/platinum_scripts.inc)
    python3 -I tools/sinnoh/gen_scripts.py --stub [OUT.inc]
    python3 -I tools/sinnoh/gen_scripts.py --auto [OUT.inc]      the build's: from the decomp make_pack.sh used
                                                                 (sinnoh/decomp_dir), else --stub
    python3 -I tools/sinnoh/gen_scripts.py --report POKEPLATINUM_DIR      what converts, what doesn't yet (and why)

A script converts when every command it reaches (its labels, the labels it goes to or calls, the ones it falls into)
is one this file knows (CONVERT below); the rest keep this game's written words until their commands are added
(docs/SINNOH_HANDOFF.md step 4). Platinum's flags and variables get SINNOH's numbers (tools/sinnoh/ids.py).
"""
import collections
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import ids  # noqa: E402
import build_text  # noqa: E402

OUT = os.path.join(ROOT, 'sinnoh', 'platinum_scripts.inc')
PREFIX = 'SinnohP_'
TERMINAL = {'End', 'Return', 'GoTo', 'EndMovement', 'ReturnToField'}

# this game's SINNOH script command (data/script_cmd_table.inc 0xF6, src/sinnoh/s3d_scrcmd.c)
SINNOH_BUFFER_PLAYER, SINNOH_BUFFER_RIVAL, SINNOH_BUFFER_COUNTERPART, SINNOH_YESNO = 1, 2, 3, 4

# Platinum's comparisons (CompareVar / CheckFlag then GoToIf / CallIf): the same codes as this game's
COND = {'Lt': 0, 'Eq': 1, 'Gt': 2, 'Le': 3, 'Ge': 4, 'Ne': 5}

# Platinum's sound effects that have a match here (others are left out: a script never waits on a missing one)
SE = {'SE_CONFIRM_sseq_3': 'SE_SELECT', 'SE_CONFIRM': 'SE_SELECT', 'SEQ_SE_CONFIRM': 'SE_SELECT'}

CONSTANTS = {'TRUE': 1, 'FALSE': 0, 'MENU_YES': 0, 'MENU_NO': 1, 'GENDER_MALE': 0, 'GENDER_FEMALE': 1}


class Unsupported(Exception):
    pass


# ----------------------------------------------------------------------------------------------------- the decomp

def parse(path):
    """{label: [(command, [args])]}, the entries (ScriptEntry names in order), the labels in file order"""
    labels, entries, order, cur = {}, [], [], None
    for raw in open(path, encoding='utf-8', errors='replace'):
        line = raw.split('//')[0].rstrip()
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        m = re.match(r'^(\w+):\s*$', line)
        if m:
            cur = m.group(1)
            labels[cur] = []
            order.append(cur)
            continue
        m = re.match(r'^\s+(\w+)\s*(.*)$', line)
        if not m:
            continue
        cmd, args = m.group(1), m.group(2).strip()
        if cmd == 'ScriptEntry':
            entries.append(args)
            continue
        if cmd in ('ScriptEntryEnd', '.balign', '.align'):
            continue
        if cur:
            labels[cur].append((cmd, [a.strip() for a in args.split(',')] if args else []))
    return labels, entries, order


class Messages:
    """The banks' numbers (build_text.py's) and each message's number in its bank"""

    def __init__(self, root):
        self.root = root
        self.banks = build_text.bank_numbers(root)
        self._ids = {}

    def bank_of_file(self, script_text):
        m = re.search(r'#include "res/text/bank/(\w+)\.h"', script_text)
        return m.group(1) if m else None

    def ids(self, bank):
        if bank not in self._ids:
            d = json.load(open(os.path.join(self.root, 'res/text', bank + '.json'), encoding='utf-8'))
            self._ids[bank] = {m['id']: i for i, m in enumerate(d.get('messages', []))}
        return self._ids[bank]


# ----------------------------------------------------------------------------------------------------- one file

class FileConverter:
    def __init__(self, root, msgs, path):
        self.root, self.msgs, self.path = root, msgs, path
        self.low = os.path.basename(path)[len('scripts_'):-2]
        text = open(path, encoding='utf-8', errors='replace').read()
        self.bank = msgs.bank_of_file(text)
        self.labels, self.entries, self.order = parse(path)
        self.texts = {}            # label -> (bank number, message number)
        self._ok = {}

    # ---- names
    def label(self, name):
        if name not in self.labels:
            raise Unsupported('goes to %s (another file)' % name)
        return '%s%s__%s' % (PREFIX, self.low, name)

    def entry_label(self, n):
        return '%s%s_%d' % (PREFIX, self.low, n)

    def text_label(self, msg_id):
        if self.bank is None:
            raise Unsupported('no text bank')
        ids_ = self.msgs.ids(self.bank)
        if msg_id not in ids_:
            raise Unsupported('message %s not in its bank' % msg_id)
        b, m = self.msgs.banks[self.bank], ids_[msg_id]
        lab = '%sT_%d_%d' % (PREFIX, b, m)
        self.texts[lab] = (b, m)
        return lab

    def value(self, x):
        if re.match(r'^-?(0x[0-9A-Fa-f]+|\d+)$', x):
            return str(int(x, 0))
        if x in CONSTANTS:
            return str(CONSTANTS[x])
        raise Unsupported('value %s' % x)

    def var(self, x):
        if re.match(r'^(0x[0-9A-Fa-f]+|\d+)$', x) and int(x, 0) >= 0x4000:
            raise Unsupported('a raw variable number %s' % x)
        v = ids.var(self.root, x) if x.startswith('VAR_') else None
        if v is None:
            raise Unsupported('variable %s' % x)
        return v if isinstance(v, str) else '0x%X' % v

    def is_var(self, x):
        return x.startswith('VAR_')

    def flag(self, x):
        f = ids.flag(self.root, x) if x.startswith('FLAG_') else None
        if f is None:
            raise Unsupported('flag %s' % x)
        return '0x%X' % f

    def compare(self, a, b):
        if self.is_var(b):
            return ['compare_var_to_var %s, %s' % (self.var(a), self.var(b))]
        return ['compare_var_to_value %s, %s' % (self.var(a), self.value(b))]

    # ---- one command
    def command(self, cmd, a):
        out = []
        m = re.match(r'^(GoTo|Call)If(Eq|Ne|Lt|Le|Gt|Ge)$', cmd)
        if m:
            out += self.compare(a[0], a[1])
            out.append('%s_if %d, %s' % ('goto' if m.group(1) == 'GoTo' else 'call', COND[m.group(2)], self.label(a[2])))
            return out
        m = re.match(r'^(GoTo|Call)If(Set|Unset)$', cmd)
        if m:
            return ['%s_if_%s %s, %s' % (m.group(1).lower(), m.group(2).lower(), self.flag(a[0]), self.label(a[1]))]
        if cmd in ('End',):
            return ['end']
        if cmd == 'Return':
            return ['return']
        if cmd == 'GoTo':
            return ['goto %s' % self.label(a[0])]
        if cmd == 'Call':
            return ['call %s' % self.label(a[0])]
        if cmd == 'CompareVar':
            return self.compare(a[0], a[1])
        if cmd == 'CheckFlag':
            return ['checkflag %s' % self.flag(a[0])]
        if cmd in ('GoToIf', 'CallIf'):
            return ['%s_if %s, %s' % ('goto' if cmd == 'GoToIf' else 'call', self.value(a[0]), self.label(a[1]))]
        if cmd == 'SetVar':
            if self.is_var(a[1]):
                return ['copyvar %s, %s' % (self.var(a[0]), self.var(a[1]))]
            return ['setvar %s, %s' % (self.var(a[0]), self.value(a[1]))]
        if cmd in ('AddVar', 'SubVar'):
            if self.is_var(a[1]):
                raise Unsupported('%s by a variable' % cmd)
            return ['%s %s, %s' % (cmd.lower(), self.var(a[0]), self.value(a[1]))]
        if cmd == 'CopyVar':
            return ['copyvar %s, %s' % (self.var(a[0]), self.var(a[1]))]
        if cmd in ('SetFlag', 'ClearFlag'):
            return ['%s %s' % (cmd.lower(), self.flag(a[0]))]
        if cmd == 'LockAll':
            return ['lockall']
        if cmd == 'ReleaseAll':
            return ['releaseall']
        if cmd == 'FacePlayer':
            return ['faceplayer']
        if cmd == 'PlaySE':
            se = SE.get(a[0])
            return ['playse %s' % se] if se else []
        if cmd == 'WaitSE':
            return ['waitse']
        if cmd == 'Message':
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd in ('WaitButton', 'WaitABXPadPress', 'WaitABPress'):
            return ['waitbuttonpress']
        if cmd == 'CloseMessage':
            return ['closemessage']
        if cmd == 'NPCMessage':
            return ['playse SE_SELECT', 'lockall', 'faceplayer', 'message %s' % self.text_label(a[0]), 'waitmessage',
                    'waitbuttonpress', 'closemessage', 'releaseall']
        if cmd == 'EventMessage':
            return ['playse SE_SELECT', 'lockall', 'message %s' % self.text_label(a[0]), 'waitmessage',
                    'waitbuttonpress', 'closemessage', 'releaseall']
        if cmd in ('ShowMapSign', 'ShowArrowSign', 'ShowLandmarkSign', 'ShowScrollingSign'):
            # (Platinum's signposts slide in with a picture of the sign; here: the sign's words in the message box)
            return ['lockall', 'message %s' % self.text_label(a[0]), 'waitmessage', 'waitbuttonpress', 'closemessage',
                    'releaseall']
        if cmd == 'BufferPlayerName':
            return ['sinnoh %d, %s' % (SINNOH_BUFFER_PLAYER, self.value(a[0]))]
        if cmd == 'BufferRivalName':
            return ['sinnoh %d, %s' % (SINNOH_BUFFER_RIVAL, self.value(a[0]))]
        if cmd == 'BufferCounterpartName':
            return ['sinnoh %d, %s' % (SINNOH_BUFFER_COUNTERPART, self.value(a[0]))]
        if cmd == 'ShowYesNoMenu':
            # (this game's YES is 1, Platinum's MENU_YES 0: the answer turned into Platinum's)
            return ['yesnobox 20, 8', 'sinnoh %d, %s' % (SINNOH_YESNO, self.var(a[0]))]
        if cmd in ('Delay8', 'Delay4', 'Delay2', 'Delay1', 'Delay16', 'Delay15', 'Delay32'):
            return ['delay %d' % int(cmd[len('Delay'):])]
        if cmd == 'WaitTime':
            return ['delay %s' % self.value(a[0])]
        if cmd in ('TurnOnTimeBasedEffects',):
            return []
        raise Unsupported(cmd)

    # ---- reach and convert
    def reach(self, start):
        """the labels an entry runs: those it goes to or calls, and those it falls into"""
        seen, stack = [], [start]
        while stack:
            lab = stack.pop()
            if lab in seen:
                continue
            if lab not in self.labels:
                raise Unsupported('goes to %s (another file)' % lab)
            seen.append(lab)
            body = self.labels[lab]
            for cmd, a in body:
                for x in a:
                    if x in self.labels and x not in seen and cmd != 'ApplyMovement':
                        stack.append(x)
            if not body or body[-1][0] not in TERMINAL:
                i = self.order.index(lab)
                if i + 1 < len(self.order):
                    stack.append(self.order[i + 1])
        return seen

    def check(self, n):
        """None if entry n (from 1) converts, else why not"""
        if n in self._ok:
            return self._ok[n]
        why = None
        try:
            if not 1 <= n <= len(self.entries):
                raise Unsupported('no entry %d' % n)
            for lab in self.reach(self.entries[n - 1]):
                for cmd, a in self.labels[lab]:
                    self.command(cmd, a)
        except Unsupported as e:
            why = str(e)
        except (IndexError, ValueError) as e:
            why = 'arguments: %s' % e
        self._ok[n] = why
        return why

    def emit(self, wanted):
        """this file's converted labels (the entries in WANTED that convert), in file order"""
        labels = set()
        entries = []
        for n in sorted(wanted):
            if self.check(n) is None:
                labels |= set(self.reach(self.entries[n - 1]))
                entries.append(n)
        out = []
        for n in entries:
            out.append('%s::' % self.entry_label(n))
            out.append('\tgoto %s' % self.label(self.entries[n - 1]))
        for lab in self.order:
            if lab not in labels:
                continue
            out.append('%s:' % self.label(lab))
            for cmd, a in self.labels[lab]:
                out += ['\t' + line for line in self.command(cmd, a)]
            body = self.labels[lab]
            if not body or body[-1][0] not in TERMINAL:
                i = self.order.index(lab)
                nxt = self.order[i + 1] if i + 1 < len(self.order) else None
                out.append('\tgoto %s' % self.label(nxt) if nxt in labels else '\tend')
        return out, entries


# ----------------------------------------------------------------------------------------------------- all files

_converters = {}


def converter(root, low):
    """the FileConverter of Platinum's scripts_<low>.s (None: no such file)"""
    key = (root, low)
    if key not in _converters:
        path = os.path.join(root, 'res/field/scripts', 'scripts_%s.s' % low)
        msgs = _converters.setdefault((root, None), Messages(root))
        _converters[key] = FileConverter(root, msgs, path) if os.path.exists(path) else None
    return _converters[key]


def convertible(root, low, n):
    """the label a map's stub goes to when Platinum's entry N of scripts_<low>.s converts, else None"""
    c = converter(root, low)
    if c is None or c.check(n) is not None:
        return None
    return c.entry_label(n)


def referenced():
    """{SinnohP_ label} every map's scripts point at (data/maps/Sinnoh_*/scripts.inc)"""
    out = set()
    for path in glob.glob(os.path.join(ROOT, 'data/maps/Sinnoh_*/scripts.inc')):
        out |= set(re.findall(r'\b(%s\w+)' % PREFIX, open(path, encoding='utf-8').read()))
    return out


def text_bytes(b, m):
    return ', '.join('0x%02X' % x for x in build_text.sinnoh_text_bytes(b, m))


HEADER = ('@ SINNOH: Platinum\'s scripts as this game\'s (tools/sinnoh/gen_scripts.py, made from pret/pokeplatinum when '
          'the game is built: never committed)\n')


def write(lines, out_path):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    text = HEADER + '\n'.join(lines) + '\n'
    if os.path.exists(out_path) and open(out_path, encoding='utf-8').read() == text:
        return
    open(out_path, 'w', encoding='utf-8').write(text)


def stub(out_path):
    lines = ['%s::\n\tend' % lab for lab in sorted(referenced())]
    write(lines, out_path)
    print('%s: %d stubs (no decomp)' % (os.path.relpath(out_path, ROOT), len(referenced())))


def build(root, out_path):
    refs = referenced()
    wanted = collections.defaultdict(set)
    for lab in refs:
        m = re.match(r'^%s(\w+?)_(\d+)$' % PREFIX, lab)
        if m:
            wanted[m.group(1)].add(int(m.group(2)))
    lines, texts, done = [], {}, set()
    for low in sorted(wanted):
        c = converter(root, low)
        if c is None:
            continue
        body, entries = c.emit(wanted[low])
        lines += body
        texts.update(c.texts)
        done |= {c.entry_label(n) for n in entries}
    for lab in sorted(refs - done):
        print('gen_scripts: %s no longer converts: it ends' % lab, file=sys.stderr)
        lines.append('%s::\n\tend' % lab)
    for lab, (b, m) in sorted(texts.items()):
        lines.append('%s:\n\t.byte %s' % (lab, text_bytes(b, m)))
    write(lines, out_path)
    print('%s: %d scripts, %d texts' % (os.path.relpath(out_path, ROOT), len(done), len(texts)))


def report(root):
    why = collections.Counter()
    ok = total = 0
    for path in sorted(glob.glob(os.path.join(root, 'res/field/scripts/scripts_*.s'))):
        low = os.path.basename(path)[len('scripts_'):-2]
        c = converter(root, low)
        for n, name in enumerate(c.entries, 1):
            if re.search(r'OnTransition|OnFrame|OnLoad|OnResume|Init', name):
                continue
            total += 1
            w = c.check(n)
            if w is None:
                ok += 1
            else:
                why[w.split(' ')[0] if not w.startswith(('flag', 'variable', 'value')) else w] += 1
    print('%d of %d script entries convert' % (ok, total))
    for w, k in why.most_common(60):
        print('  %5d  %s' % (k, w))


def main():
    a = sys.argv[1:]
    if a and a[0] == '--auto':
        mark = os.path.join(ROOT, 'sinnoh', 'decomp_dir')
        root = open(mark).read().strip() if os.path.exists(mark) else ''
        if root and os.path.isdir(os.path.join(root, 'res/field/scripts')):
            build(root, a[1] if len(a) > 1 else OUT)
        else:
            stub(a[1] if len(a) > 1 else OUT)
    elif a and a[0] == '--stub':
        stub(a[1] if len(a) > 1 else OUT)
    elif a and a[0] == '--report':
        report(a[1])
    elif a:
        build(a[0], a[1] if len(a) > 1 else OUT)
    else:
        print(__doc__)
        sys.exit(1)


if __name__ == '__main__':
    main()
