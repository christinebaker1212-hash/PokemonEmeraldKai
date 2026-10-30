#!/usr/bin/env python3
"""
Moemon Fight: static checks of the cabinet's characters (6.4), from their own files (tools/fight/mugen_files.py):
what a script names that isn't there, where the value is written out (expressions are left to the runtime
audit, tools/fight/audit_chars.sh):

  state     ChangeState / SelfState / TargetState / Helper stateno / HitDef and ReversalDef p1stateno, p2stateno /
            HitOverride stateno to a state neither the character nor common1.cns has
  anim      a Statedef's anim, ChangeAnim, an Explod's / Projectile's / HitDef spark anim (not F: the character's
            .air) missing from the .air; F ones checked against the fight's fightfx.air
  sound     PlaySnd / HitDef hitsound, guardsound with S (or PlaySnd without F) missing from the .snd; F / plain
            hit sounds checked against the fight's common.snd
  sprite    an .air frame whose sprite the .sff lacks (MUGEN draws nothing for it; packs do it on purpose to hide a
            frame, so these are listed as information)
  var       var(n) / fvar(n) / sysvar(n) / sysfvar(n) past the engine's 60 / 40 / 5 / 5
  syntax    a value whose brackets don't balance (ranges like [0,5) count as balanced)
States no fight can reach (nothing names them) are reported apart: dead code, not a fault of the fights.

  check_chars.py [--root mugen] [--json OUT] [NAME...]     exit 1 when anything was found
"""
import argparse
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mugen_files as M  # noqa: E402

INT = re.compile(r'^\s*(-?\d+)\s*$')
VAR = re.compile(r'\b(sys)?(f)?var\s*\(\s*(\d+)\s*\)', re.I)
VAR_LIMITS = {(False, False): 60, (False, True): 40, (True, False): 5, (True, True): 5}
STATE_KEYS = {
    'changestate': ['value'], 'selfstate': ['value'], 'targetstate': ['value'], 'helper': ['stateno'],
    'hitdef': ['p1stateno', 'p2stateno'], 'reversaldef': ['p1stateno', 'p2stateno'], 'hitoverride': ['stateno'],
    'projectile': ['p1stateno', 'p2stateno'],
}
ANIM_KEYS = {'changeanim': ['value'], 'explod': ['anim'], 'gamemakeanim': ['value'],
             'projectile': ['projanim', 'projhitanim', 'projremanim', 'projcancelanim', 'sparkno', 'guard.sparkno'],
             'hitdef': ['sparkno', 'guard.sparkno'], 'reversaldef': ['sparkno']}
SOUND_KEYS = {'playsnd': ['value'], 'hitdef': ['hitsound', 'guardsound'], 'projectile': ['hitsound', 'guardsound'],
              'reversaldef': ['hitsound']}


def literal(v):
    m = INT.match(v or '')
    return int(m.group(1)) if m else None


def ref(v, default_common):
    """(common?, int or (g, n)) of a value like F60, S5,0, 5,0 or 200; None when it is an expression."""
    v = (v or '').strip()
    common = default_common
    if v[:1] in 'Ff' and len(v) > 1 and (v[1].isdigit() or v[1] in ' -'):
        common, v = True, v[1:]
    elif v[:1] in 'Ss' and len(v) > 1 and (v[1].isdigit() or v[1] in ' -'):
        common, v = False, v[1:]
    parts = [p.strip() for p in v.split(',')]
    try:
        if len(parts) >= 2 and parts[1] != '':
            return common, (int(parts[0]), int(parts[1]))
        return common, int(parts[0])
    except ValueError:
        return None


NUMBER = re.compile(r'(?<![\w.])(\d+)(?![\w.])')
STATE_REF_KEYS = {k for v in STATE_KEYS.values() for k in v}


def reachable_states(char, states):
    """The states a fight can reach: the ones the engine enters itself (-3..-1 always run, 0-199 basics, intros,
    wins, 5000-5999 getting hit) and those the character's commands (the cmd file's state -1) and reachable states
    name, literally or anywhere inside an expression (any number there may be a state)."""
    roots = {n for n in states if n < 200 or 5000 <= n < 6000}
    cmd = char.file('cmd')
    extra = []
    if cmd:
        extra = M.parse_states(cmd)
    todo = list(roots) + [st.number for st in extra]
    blocks_of = {n: st for n, st in states.items()}
    for st in extra:
        blocks_of.setdefault(st.number, st)
    seen = set()
    while todo:
        n = todo.pop()
        if n in seen:
            continue
        seen.add(n)
        st = blocks_of.get(n)
        if st is None:
            continue
        for lbl, kv, ln in st.controllers:
            t = (kv.get('type') or '').lower()
            for key in STATE_KEYS.get(t, []):
                for m in NUMBER.finditer(kv.get(key) or ''):
                    v = int(m.group(1))
                    if v not in seen:
                        todo.append(v)
    return seen


def check(char, fx_air, common_snd):
    out = []
    states = char.states()
    reach = reachable_states(char, states)
    cur_state = [None]

    def add(kind, where, text):
        dead = cur_state[0] is not None and cur_state[0] not in reach
        out.append({'char': char.name, 'kind': kind + (' (unused state)' if dead else ''), 'where': where,
                    'text': text, 'live': not dead})

    air_path, snd_path, sff_path = char.file('anim'), char.file('sound'), char.file('sprite')
    air = M.read_air(air_path) if air_path else {}
    snd = M.read_snd(snd_path) if snd_path else set()
    sff = M.read_sff(sff_path) if sff_path else set()
    own_files = set(char.state_files()[0]) - {char.state_files()[1]}
    for number, st in sorted(states.items()):
        cur_state[0] = number
        common = st.file not in own_files
        here = '%s:%d' % (os.path.relpath(st.file, char.pack.root), st.line)
        a = literal(st.params.get('anim'))
        if a is not None and a not in air and not common:
            add('anim', here, 'Statedef %d: anim %d is not in the .air' % (number, a))
        blocks = [('statedef', st.params, st.line)] + [(lbl, kv, ln) for lbl, kv, ln in st.controllers]
        for lbl, kv, ln in blocks:
            for k, v in kv.items():
                bare = re.sub(r'"[^"]*"', '""', v)
                if bare.count('(') + bare.count('[') != bare.count(')') + bare.count(']'):
                    add('syntax', '%s:%d' % (os.path.relpath(st.file, char.pack.root), ln),
                        'unbalanced brackets in %s = %s' % (k, v[:60]))
        for lbl, kv, ln in blocks:
            where = '%s:%d' % (os.path.relpath(st.file, char.pack.root), ln)
            t = (kv.get('type') or '').lower()
            for k, v in kv.items():
                for m in VAR.finditer(v):
                    sysv, fv, idx = bool(m.group(1)), bool(m.group(2)), int(m.group(3))
                    if idx >= VAR_LIMITS[(sysv, fv)]:
                        add('var', where, '%s(%d) in %s: past the engine\'s %d' % (
                            m.group(0).split('(')[0].strip(), idx, k, VAR_LIMITS[(sysv, fv)]))
            if t in ('varset', 'varadd', 'varrangeset'):
                for k in kv:
                    m = re.match(r'^(sys)?(f)?(v|var)\s*$', k) or re.match(r'^(sys)?(f)?v$', k)
                    if m and literal(kv[k]) is not None:
                        pass
                    m2 = re.match(r'^(sys)?(f)?v$', k)
                    if m2:
                        idx = literal(kv[k])
                        if idx is not None and idx >= VAR_LIMITS[(bool(m2.group(1)), bool(m2.group(2)))]:
                            add('var', where, '%s = %d: past the engine\'s limit' % (k, idx))
            if common:
                continue    # common1.cns uses each character's own anims and states: not this one's fault
            for key in STATE_KEYS.get(t, []):
                n = literal(kv.get(key))
                if n is not None and n >= 0 and n not in states:
                    add('state', where, '%s %s = %d: no such state' % (kv.get('type'), key, n))
            triggers = ' '.join(v for k2, v in kv.items() if k2.startswith('trigger')).lower().replace(' ', '')
            for key in ANIM_KEYS.get(t, []):
                r = ref(kv.get(key), default_common=key.endswith('sparkno'))
                if r is None or not isinstance(r[1], int) or r[1] < 0:
                    continue
                com, n = r
                if 'animexist(%d)' % n in triggers:
                    continue    # only when it exists (SelfAnimExist / AnimExist in the triggers)
                if com and fx_air is not None and n not in fx_air:
                    add('anim', where, '%s %s = F%d: not in the fight\'s fightfx.air' % (kv.get('type'), key, n))
                elif not com and n not in air:
                    add('anim', where, '%s %s = %d: not in the .air' % (kv.get('type'), key, n))
            for key in SOUND_KEYS.get(t, []):
                r = ref(kv.get(key), default_common=(t != 'playsnd'))
                if r is None or not isinstance(r[1], tuple) or r[1][0] < 0:
                    continue
                com, gn = r
                if com and common_snd is not None and gn not in common_snd:
                    add('sound', where, '%s %s = F%d,%d: not in the fight\'s common.snd' % (
                        kv.get('type'), key, gn[0], gn[1]))
                elif not com and gn not in snd:
                    add('sound', where, '%s %s = %d,%d: not in the .snd' % (kv.get('type'), key, gn[0], gn[1]))
    cur_state[0] = None
    if sff:
        missing = {}
        for action, frames in air.items():
            for g, n, x, y, t in frames:
                if g >= 0 and n >= 0 and (g, n) not in sff:
                    missing.setdefault((g, n), []).append(action)
        for (g, n), acts in sorted(missing.items()):
            out.append({'char': char.name, 'kind': 'sprite (info)', 'live': False,
                        'where': os.path.relpath(air_path, char.pack.root),
                        'text': 'sprite %d,%d (actions %s) is not in the .sff: a blank frame (often on purpose)' % (
                            g, n, ', '.join(str(a) for a in sorted(set(acts))[:6]))})
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default='mugen')
    ap.add_argument('--json')
    ap.add_argument('names', nargs='*')
    a = ap.parse_args()
    pack = M.Pack(a.root)
    fight = M.read_ini(pack.fight_def) if pack.fight_def else []
    files = M.section(fight, 'Files')
    fx = pack.find(files.get('fightfx.air', 'fightfx.air'), pack.fight_def)
    csnd = pack.find(files.get('common.snd', 'common.snd'), pack.fight_def)
    fx_air = M.read_air(fx) if fx else None
    common_snd = M.read_snd(csnd) if csnd else None
    found = []
    for c in pack.roster():
        if a.names and c.name.lower() not in [n.lower() for n in a.names]:
            continue
        found += check(c, fx_air, common_snd)
    for f in sorted(found, key=lambda f: (not f['live'], f['char'], f['kind'])):
        print('%-10s %-6s %s: %s' % (f['char'], f['kind'], f['where'], f['text']))
    live = [f for f in found if f['live']]
    print('%d finding(s), %d in states a fight reaches' % (len(found), len(live)))
    if a.json:
        with open(a.json, 'w') as fp:
            json.dump(found, fp, indent=1)
    return 1 if live else 0


if __name__ == '__main__':
    sys.exit(main())
