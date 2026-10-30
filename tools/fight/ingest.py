#!/usr/bin/env python3
"""
Moemon Fight: the VS cabinet's asset ingestion (6.6). Finds the characters, stages and shared files of mugen/ and
what each depends on (tools/fight/mugen_files.py), and for every asset that is new, changed (a dependency's content),
or was last processed under other rules or settings:

  1. repairs that keep behaviour, applied at once:
       - sound banks and songs brought to GBA sound (tools/fight/gba_audio.py: 8-bit mono 13379 Hz clips, 13379 Hz Ogg
         songs, the .defs and loop points following)
       - debug text controllers (DisplayToClipboard / AppendToClipboard) left on in the always-running states -3..-1
         switched off (type = null): they only write the debug clipboard, and some ask for missing helpers every frame
  2. audits: the static character checks (check_chars.py), stage validation (its files, sprites, music, camera and
     player sections), and the roster's balance audit (balance_audit.py) when any character changed
  3. proposals that change behaviour or are a judgement (a ChangeState / TargetState to a missing state where one
     state of the same block of ten exists, like 935 for 930): listed, applied only with --apply-proposals

Removed assets are dropped from the record. Everything goes to mugen/ingestion_log.txt; what was done to each asset
and the fingerprints of its files to mugen/processed_assets.json (written after each asset: an interrupted run picks
up where it stopped, and an asset marked in progress is done again; every repair is safe to repeat). Unchanged assets
are skipped. mugen/processed_assets.idx (size, modification time, path) lets the game see changes at start without
hashing.

  ingest.py [--root mugen] [--force] [--apply-proposals] [--check] [--quiet]
      --force             process everything again
      --check             change nothing; exit 1 when something would be processed
The game's build (Makefile: fight-ingest) runs it when mugen/ changed; the game checks the record at start.
"""
import argparse
import hashlib
import json
import os
import re
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import mugen_files as M  # noqa: E402

RULES_VERSION = 1           # bump when what ingestion does changes: every asset is processed again
TOOLS = ['ingest.py', 'gba_audio.py', 'check_chars.py', 'balance_audit.py', 'mugen_files.py', 'balance_baseline.json']


def sha1(path):
    h = hashlib.sha1()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(1 << 20), b''):
            h.update(chunk)
    return h.hexdigest()


def config_hash():
    h = hashlib.sha1()
    for t in TOOLS:
        p = os.path.join(HERE, t)
        if os.path.exists(p):
            h.update(t.encode())
            h.update(open(p, 'rb').read())
    return h.hexdigest()[:16]


class Log:
    def __init__(self, path, quiet):
        self.path, self.quiet = path, quiet
        self.lines = []

    def __call__(self, text):
        line = '%s  %s' % (time.strftime('%Y-%m-%d %H:%M:%S'), text)
        self.lines.append(line)
        if not self.quiet:
            print(text)

    def flush(self, check):
        if check or not self.lines:
            return
        with open(self.path, 'a', encoding='utf-8') as f:
            f.write('\n'.join(self.lines) + '\n')
        self.lines = []


# ------------------------------------------------------------------ discovery

def discover(pack):
    """{asset id: (kind, main file, [dependency files])}"""
    root = pack.root
    assets = {}
    rel = lambda p: os.path.relpath(p, root)
    # characters: the select list's, and any character .def in chars/ (a new folder not listed yet)
    seen = set()
    chars = pack.roster()
    for d in sorted(set(os.path.join(dp, f) for dp, _, fs in os.walk(os.path.join(root, 'chars')) for f in fs
                        if f.lower().endswith('.def'))):
        if any(c.def_path and os.path.samefile(c.def_path, d) for c in chars):
            continue
        sec = M.section(M.read_ini(d), 'Files')
        if sec.get('cns') or sec.get('sprite'):
            chars.append(M.Character(pack, os.path.relpath(d, os.path.join(root, 'chars'))))
    for c in chars:
        if not c.def_path or c.def_path in seen:
            continue
        seen.add(c.def_path)
        assets['char:' + rel(c.def_path)] = ('char', c.def_path, c.dependencies(), c)
    # stages: stages/*.def and every stage the cabinet or a character names
    stage_defs = set(pack.stages())
    sdir = os.path.join(root, 'stages')
    if os.path.isdir(sdir):
        stage_defs |= {os.path.join(sdir, f) for f in os.listdir(sdir) if f.lower().endswith('.def')}
    for s in sorted(stage_defs):
        assets['stage:' + rel(s)] = ('stage', s, stage_deps(pack, s), None)
    # the shared files: config, screenpack, fight definition and the files they name
    core = [p for p in (os.path.join(root, 'data', 'mugen.cfg'), pack.motif, pack.select_def, pack.fight_def,
                        os.path.join(root, 'cabinets.ini')) if p and os.path.exists(p)]
    for d in (pack.motif, pack.fight_def):
        if d:
            for name, kv in M.read_ini(d):
                if name.lower() == 'files':
                    for k, v, _ in kv:
                        p = pack.find(v, d) if v else None
                        if p and os.path.isfile(p):
                            core.append(p)
    assets['core'] = ('core', pack.motif, sorted(set(core)), None)
    return assets


def stage_deps(pack, path):
    deps = [path]
    ini = M.read_ini(path)
    for name, kv in ini:
        for k, v, _ in kv:
            if v and (k == 'spr' or k == 'bgmusic'):
                p = pack.find(v, path, subdirs=('', 'stages', 'sound', 'data'))
                if p and os.path.isfile(p):
                    deps.append(p)
    return deps


def fingerprint(paths, root):
    return {os.path.relpath(p, root): sha1(p) for p in paths if os.path.isfile(p)}


# ------------------------------------------------------------------ repairs and audits

ALWAYS_STATES = (-3, -2, -1)
DEBUG_CTRL = re.compile(rb'^(\s*type\s*=\s*)(DisplayToClipboard|AppendToClipboard)(\s*(?:;[^\r\n]*)?)(\r?)$',
                        re.I | re.M)


def repair_debug_text(char, check, log):
    """Switches off the debug text controllers of the states that run every frame. Returns the files changed."""
    changed = []
    for path in char.state_files()[0] + ([char.file('cmd')] if char.file('cmd') else []):
        if not path or not os.path.isfile(path):
            continue
        data = open(path, 'rb').read()
        lines = data.split(b'\n')
        cur = None
        hit = False
        for i, line in enumerate(lines):
            m = re.match(rb'^\s*\[\s*statedef\s+(-?\d+)', line, re.I)
            if m:
                cur = int(m.group(1))
                continue
            if cur in ALWAYS_STATES and DEBUG_CTRL.match(line):
                lines[i] = DEBUG_CTRL.sub(rb'\1null;\2 ;switched off by ingestion: debug text only\4', line)
                hit = True
        if hit:
            changed.append(path)
            if not check:
                tmp = path + '.part'
                open(tmp, 'wb').write(b'\n'.join(lines))
                os.replace(tmp, path)
    return changed


def repair_audio(paths, root, check, log):
    import gba_audio as A
    done = []
    for p in paths:
        low = p.lower()
        try:
            if low.endswith('.snd') and A.snd_needs(p):
                if not check:
                    A.convert_snd(p)
                done.append(('snd', p))
            elif low.endswith(A.SONG_EXTS) and A.song_needs(p):
                if not check:
                    new, rate = A.convert_song(p)
                    if new:
                        A.retarget_defs(root, {os.path.abspath(p): (os.path.abspath(new), rate)})
                done.append(('song', p))
        except Exception as e:  # a file ffmpeg can't read: leave it, say so
            log('  could not convert %s: %s' % (os.path.relpath(p, root), e))
    return done


def validate_stage(pack, path):
    out = []
    ini = M.read_ini(path)
    names = {n.lower() for n, _ in ini}
    for need in ('info', 'camera', 'playerinfo', 'stageinfo', 'bgdef'):
        if need not in names:
            out.append('no [%s] section' % need)
    bgdef = M.section(ini, 'BGdef')
    spr = pack.find(bgdef.get('spr', ''), path, subdirs=('', 'stages')) if bgdef.get('spr') else None
    if not spr:
        out.append('its sprite file (%s) is missing' % bgdef.get('spr', 'spr ='))
    else:
        sprites = M.read_sff(spr)
        for name, kv in ini:
            d = {k: v for k, v, _ in kv if v is not None}
            if 'spriteno' in d and d.get('type', '').lower() != 'anim':
                try:
                    g, n = (int(x) for x in d['spriteno'].split(',')[:2])
                except ValueError:
                    continue
                if (g, n) not in sprites and g >= 0:
                    out.append('[%s] uses sprite %d,%d, which the .sff lacks' % (name, g, n))
    music = M.section(ini, 'Music').get('bgmusic')
    if music and not pack.find(music, path, subdirs=('', 'sound', 'stages')):
        out.append('its song (%s) is missing: the stage plays no music' % music)
    return out


def proposals_for(char):
    """Changes that need a decision: a ChangeState / TargetState / SelfState to a state the character lacks where
    exactly one state of the same block of ten exists (a likely typo)."""
    import check_chars as C
    states = char.states()
    out = []
    for f in C.check(char, None, None):
        if f['kind'] != 'state':
            continue
        m = re.search(r'= (\d+): no such state', f['text'])
        if not m:
            continue
        n = int(m.group(1))
        near = [s for s in states if s // 10 == n // 10]
        if len(near) == 1:
            out.append({'file': f['where'].rsplit(':', 1)[0], 'line': int(f['where'].rsplit(':', 1)[1]),
                        'from': n, 'to': near[0], 'why': f['text']})
    return out


def apply_proposal(root, p, log):
    path = os.path.join(root, p['file'])
    data = open(path, 'rb').read()
    lines = data.split(b'\n')
    i = p['line'] - 1
    if 0 <= i < len(lines) and re.search(rb'=\s*%d\b' % p['from'], lines[i]):
        lines[i] = re.sub(rb'(=\s*)%d\b' % p['from'], rb'\g<1>%d' % p['to'], lines[i], count=1)
        tmp = path + '.part'
        open(tmp, 'wb').write(b'\n'.join(lines))
        os.replace(tmp, path)
        log('  applied: %s:%d %d -> %d' % (p['file'], p['line'], p['from'], p['to']))
        return True
    return False


# ------------------------------------------------------------------ the run

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default=os.path.join(os.path.dirname(os.path.dirname(HERE)), 'mugen'))
    ap.add_argument('--force', action='store_true')
    ap.add_argument('--apply-proposals', action='store_true')
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--quiet', action='store_true')
    a = ap.parse_args()
    root = os.path.abspath(a.root)
    state_path = os.path.join(root, 'processed_assets.json')
    log = Log(os.path.join(root, 'ingestion_log.txt'), a.quiet)
    record = {'rules': RULES_VERSION, 'assets': {}}
    if os.path.exists(state_path):
        try:
            record = json.load(open(state_path))
        except ValueError:
            log('processed_assets.json was unreadable: starting again')
    cfg = config_hash()
    pack = M.Pack(root)
    assets = discover(pack)
    todo, unchanged = [], 0
    for aid, (kind, main, deps, obj) in sorted(assets.items()):
        old = record['assets'].get(aid)
        fp = fingerprint(deps, root)
        why = None
        if a.force:
            why = 'forced'
        elif old is None:
            why = 'new'
        elif old.get('status') != 'done':
            why = 'interrupted last time'
        elif old.get('rules') != RULES_VERSION or old.get('config') != cfg:
            why = 'the rules or settings changed'
        elif old.get('deps') != fp:
            changed = sorted(set(fp) ^ set(old.get('deps', {})) |
                             {k for k in fp if old.get('deps', {}).get(k) != fp[k]})
            why = 'changed: ' + ', '.join(changed[:4]) + (' ...' if len(changed) > 4 else '')
        if why:
            todo.append((aid, why))
        else:
            unchanged += 1
    removed = [aid for aid in record['assets'] if aid not in assets]
    if a.check:
        for aid, why in todo:
            print('would process %s (%s)' % (aid, why))
        for aid in removed:
            print('would drop %s (removed)' % aid)
        return 1 if todo or removed else 0
    if todo or removed:
        log('ingestion: %d to process, %d unchanged, %d removed' % (len(todo), unchanged, len(removed)))
    for aid in removed:
        log('%s: removed from the pack' % aid)
        del record['assets'][aid]

    def save():
        record['rules'] = RULES_VERSION
        tmp = state_path + '.part'
        with open(tmp, 'w') as f:
            json.dump(record, f, indent=1, sort_keys=True)
        os.replace(tmp, state_path)
        log.flush(False)

    chars_changed = False
    stop_after = int(os.environ.get('MOEMON_INGEST_STOP_AFTER', '0') or 0)   # tests: an interrupted run
    for n_done, (aid, why) in enumerate(todo):
        if stop_after and n_done == stop_after:
            entry = record['assets'].setdefault(aid, {})
            entry.update({'kind': assets[aid][0], 'status': 'in progress'})
            save()
            log('stopped (test) during %s' % aid)
            log.flush(False)
            return 3
        kind, main, deps, obj = assets[aid]
        entry = record['assets'].setdefault(aid, {})
        entry.update({'kind': kind, 'path': os.path.relpath(main, root) if main else None, 'status': 'in progress'})
        save()
        log('%s: %s' % (aid, why))
        repairs, findings, proposals = [], [], []
        for k, p in repair_audio(deps, root, False, log):
            repairs.append('%s brought to GBA sound: %s' % (k, os.path.relpath(p, root)))
        if kind == 'char':
            chars_changed = True
            for p in repair_debug_text(obj, False, log):
                repairs.append('debug text switched off in %s' % os.path.relpath(p, root))
            import check_chars as C
            obj = M.Character(pack, obj.entry, obj.stage, obj.opts)     # read again after the repairs
            live = [f for f in C.check(obj, None, None) if f['live']]
            findings = ['%s %s: %s' % (f['kind'], f['where'], f['text']) for f in live]
            proposals = proposals_for(obj)
            if a.apply_proposals:
                for p in proposals:
                    if apply_proposal(root, p, log):
                        repairs.append('proposal applied: %s:%d %d -> %d' % (p['file'], p['line'], p['from'], p['to']))
                proposals = proposals_for(M.Character(pack, obj.entry, obj.stage, obj.opts))
        elif kind == 'stage':
            findings = validate_stage(pack, main)
        for r in repairs:
            log('  repaired: ' + r)
        for f in findings:
            log('  found: ' + f)
        for p in proposals:
            log('  proposed (not applied; --apply-proposals): %s:%d %d -> %d (%s)' % (
                p['file'], p['line'], p['from'], p['to'], p['why']))
        # fingerprints after the repairs, so the next run sees nothing to do
        deps = discover(pack)[aid][2] if repairs else deps
        entry.update({'status': 'done', 'rules': RULES_VERSION, 'config': cfg, 'deps': fingerprint(deps, root),
                      'repairs': entry.get('repairs', []) + repairs, 'findings': findings, 'proposals': proposals,
                      'processed': time.strftime('%Y-%m-%d %H:%M:%S')})
        save()
    if chars_changed:
        import balance_audit as B
        base = json.loads(json.dumps(B.DEFAULT_BASELINE))
        bp = os.path.join(HERE, 'balance_baseline.json')
        if os.path.exists(bp):
            for k, v in json.load(open(bp)).items():
                if isinstance(v, dict) and isinstance(base.get(k), dict):
                    base[k].update(v)
                else:
                    base[k] = v
        roster, bl, bl_damage, found = B.audit(M.Pack(root), base)
        counts = {k: sum(1 for f in found if f['kind'] == k) for k in ('mistake', 'trait', 'runtime')}
        record['balance'] = {'counts': counts, 'mistakes': [f for f in found if f['kind'] == 'mistake'],
                             'when': time.strftime('%Y-%m-%d %H:%M:%S')}
        log('balance audit: %d likely mistakes, %d character traits, %d to check at run time' % (
            counts['mistake'], counts['trait'], counts['runtime']))
        for f in record['balance']['mistakes']:
            log('  likely mistake: %s %s: %s' % (f['char'], f['what'], f['detail']))
    # the game's quick check at start (size, modification time, path of every file the assets use)
    idx = []
    for aid, (kind, main, deps, obj) in sorted(discover(pack).items()):
        for p in deps:
            if os.path.isfile(p):
                st = os.stat(p)
                idx.append('%d %d %s' % (st.st_size, int(st.st_mtime), os.path.relpath(p, root).replace('\\', '/')))
    idx_path = os.path.join(root, 'processed_assets.idx')
    new_idx = '\n'.join(sorted(set(idx))) + '\n'
    if not os.path.exists(idx_path) or open(idx_path).read() != new_idx:
        open(idx_path, 'w').write(new_idx)
    save()
    if not todo and not removed and not a.quiet:
        print('nothing to do: %d assets unchanged' % unchanged)
    return 0


if __name__ == '__main__':
    sys.exit(main())
