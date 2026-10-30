#!/usr/bin/env python3
"""
Moemon Fight: the cabinet's balance audit (6.5). Finds the roster and every file each character depends on by itself
(tools/fight/mugen_files.py: the screenpack's select list, each .def), then reads every move the commands start:

  health        life, attack, defence, power (meter) size, walk / run / jump speeds
  damage        each hit (HitDef, Projectile, and the hits of the helpers the move makes), multi-hit counts, guard
                damage, juggle points, power given and taken
  stun          hit stun (ground / air / guard hit time), hit pause on each side, knockdowns
  timing        startup, active and recovery frames from the move's animation and its hit boxes, and the advantage
                on hit and on guard they give (an estimate: cancels and state changes can cut a move short)
  meter         what a move costs (the command's power condition, negative PowerAdd) and gives
  cancels       what a move can be cancelled into on contact (its own ChangeStates and the commands' conditions)

and compares each character with a baseline (tools/fight/balance_baseline.json: the roster's median by default, or
fixed numbers). Findings come in three kinds:

  likely mistakes        inside one character, things that contradict themselves or the engine (a heavy hit weaker
                         than the light one of the same button family, more damage on guard than on hit, a super
                         that costs nothing, a hit nobody can guard that isn't a throw, a move that loops on itself)
  character traits       differences from the baseline that run through a whole character (all of its hits stronger,
                         a larger life bar): intentional design, listed so they're seen
  needs runtime check    what the files can't settle (damage or timing written as expressions, moves ended by state
                         changes, cancel loops); --runtime reads the hits the CPU fights of tools/fight/audit_chars.sh
                         logged (audithit lines) and puts measured damage next to the static numbers

  balance_audit.py [--root mugen] [--baseline FILE] [--runtime AUDIT_DIR] [--json OUT] [--report docs/FIGHT_BALANCE.md]
"""
import argparse
import glob
import json
import os
import re
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mugen_files as M  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_BASELINE = {
    "baseline": "median",
    "fixed": {"life": 1000, "attack": 100, "defence": 100,
              "damage": {"light": 30, "medium": 60, "heavy": 90, "special": 100, "super": 250}},
    "tolerance": {"damage": 0.4, "life": 0.15, "attack": 0.15, "defence": 0.15,
                  "advantage_hit": 12, "advantage_guard": 4, "startup_light": 6},
    "classes": {"light": ["x", "a"], "medium": ["y", "b"], "heavy": ["z", "c"]},
    "intentional": []
}
INT = re.compile(r'^\s*-?\d+(\.\d+)?\s*$')
NUM = re.compile(r'(?<![\w.])(\d+)(?![\w.])')


def num(v, default=None):
    """A literal number (first of a pair), else default."""
    if v is None:
        return default
    first = v.split(',')[0].strip()
    return float(first) if INT.match(first) else default


def pair(v):
    parts = [p.strip() for p in (v or '').split(',')]
    a = num(parts[0]) if parts and parts[0] else None
    b = num(parts[1]) if len(parts) > 1 and parts[1] else None
    return a, b


def is_literal(v):
    return v is not None and all(INT.match(p) for p in v.split(',') if p.strip())


# ------------------------------------------------------------------ expressions with constant options

def constant_vars(char):
    """Variables a character sets to a number every frame (VarSet with trigger1 = 1 in states -3 / -2: option
    switches like var(52) = 3, "the original damage")."""
    out = {}
    for st in char.states().values():
        if st.number not in (-2, -3):
            continue
        for lbl, kv, ln in st.controllers:
            if (kv.get('type') or '').lower() != 'varset':
                continue
            trig = [v.strip() for k, v in kv.items() if k.startswith('trigger')]
            if trig != ['1']:
                continue
            for k, v in kv.items():
                m = re.match(r'^(f)?var\((\d+)\)$', k.replace(' ', ''))
                if m and INT.match(v or ''):
                    out[('f' if m.group(1) else '') + 'var(%s)' % m.group(2)] = float(v)
    return out


SAFE = re.compile(r'^[\d\s.+\-*/(),<>=!&|a-z_]*$')


def evaluate(expr, consts):
    """expr with constant vars put in (ceil, floor, ifelse, arithmetic, comparisons), or None."""
    e = expr.lower().replace(' ', '')
    for k, v in consts.items():
        e = e.replace(k, '(%r)' % v)
    if 'var(' in e or not SAFE.match(e):
        return None
    e = re.sub(r'(?<![<>!=])=(?!=)', '==', e).replace('&&', ' and ').replace('||', ' or ')
    e = re.sub(r'!(?!=)', ' not ', e)
    try:
        import math
        v = eval(e, {'__builtins__': {}}, {'ceil': math.ceil, 'floor': math.floor,
                                          'ifelse': lambda c, a, b: a if c else b})
        return float(v) if isinstance(v, (int, float)) else None
    except Exception:
        return None


# ------------------------------------------------------------------ a character's moves

def split_top(v):
    """v split at its top-level commas (not those inside brackets)."""
    out, depth, cur = [], 0, ''
    for ch in v:
        if ch in '([':
            depth += 1
        elif ch in ')]':
            depth -= 1
        if ch == ',' and depth == 0:
            out.append(cur)
            cur = ''
        else:
            cur += ch
    out.append(cur)
    return out


class Hit:
    consts = {}
    own = True

    def __init__(self, state, kind, kv, where):
        self.state, self.kind, self.where = state, kind, where
        self.damage_raw = kv.get('damage', '0')
        self.damage, self.guard_damage = pair(self.damage_raw)
        evaluated = False
        if not is_literal(self.damage_raw):
            parts = split_top(self.damage_raw)
            d = evaluate(parts[0], Hit.consts)
            g = evaluate(parts[1], Hit.consts) if len(parts) > 1 and parts[1].strip() else 0.0
            if d is not None and g is not None:
                self.damage, self.guard_damage, evaluated = d, g, True
        if self.guard_damage is None and self.damage is not None and is_literal(self.damage_raw):
            self.guard_damage = 0.0
        self.attr = (kv.get('attr') or '').replace(' ', '').upper()
        self.hitflag = (kv.get('hitflag') or 'MAF').upper()
        self.guardflag = (kv.get('guardflag') or '').upper()
        self.p1pause, self.p2pause = pair(kv.get('pausetime', '0,0'))
        self.hittime = num(kv.get('ground.hittime'), 0.0 if 'ground.hittime' not in kv else None)
        self.guard_hittime = num(kv.get('guard.hittime'), self.hittime)
        self.air_hittime = num(kv.get('air.hittime'), 20.0 if 'air.hittime' not in kv else None)
        self.juggle = num(kv.get('juggle'), 0.0)
        self.fall = (kv.get('fall') or kv.get('air.fall') or '0').strip() not in ('0', '')
        self.p2stateno = kv.get('p2stateno')
        self.numhits = num(kv.get('numhits'), 1.0)
        self.getpower = pair(kv.get('getpower'))[0]
        self.givepower = pair(kv.get('givepower'))[0]
        self.trigger = ' '.join(v for k, v in kv.items() if k.startswith('trigger')).lower()
        self.expression = not is_literal(self.damage_raw) and not evaluated
        if kind == 'projectile':
            self.numhits = num(kv.get('projhits'), 1.0)

    @property
    def throw(self):
        return self.kind == 'lifeadd' or 'T' in self.attr[1:] or self.p2stateno is not None and 'T' in self.attr

    @property
    def guardable(self):
        return bool(self.guardflag.strip())


class Move:
    def __init__(self, char, state, commands, power):
        self.char, self.state, self.commands, self.power_cost = char, state, commands, power
        self.hits = []
        self.states = []
        self.helpers = []
        self.cancels = set()
        self.power_gain = 0.0
        self.timing = None
        self.notes = []
        self.air = False

    @property
    def kind(self):
        if self.throw and self.state < 3000 and self.power_cost < 1000:
            return 'throw'
        if self.state >= 3000 or self.power_cost >= 1000:
            return 'super'
        if self.state >= 1000:
            return 'special'
        if 200 <= self.state < 1000:
            return 'normal'
        return 'other'

    def button(self, classes):
        """light / medium / heavy from the command's last button (normals)."""
        for c in self.commands:
            keys = c.split(',')[-1].replace('+', ' ').replace('/', ' ').replace('~', ' ').split()
            for cls, buttons in classes.items():
                if any(k.strip().lower() in buttons for k in keys) and len(keys) == 1:
                    return cls
        return None

    def total_damage(self, follow_ups=False):
        """The move's own hits (its state, its helpers' and projectiles'); without any, the states it goes on to.
        HitDefs set at different animation elements are a sequence and add up; the ones at the same element (or
        with no element in their triggers) are alternatives picked by conditions: the strongest counts."""
        def total(hits):
            groups = {}
            for h in hits:
                if h.damage is None:
                    continue
                m = re.search(r'animelem\s*=\s*(\d+)', h.trigger)
                key = (h.state, h.kind, int(m.group(1)) if m else None)
                groups[key] = max(groups.get(key, 0), h.damage * max(1, h.numhits or 1))
            return sum(groups.values())
        own = total([h for h in self.hits if h.own])
        if own and not follow_ups:
            return own
        return total(self.hits)

    @property
    def throw(self):
        own = [h for h in self.hits if h.own and h.kind in ('hitdef', 'lifeadd')]
        return bool(own) and all(h.throw or (h.p2stateno is not None and not h.guardable) for h in own)


def command_table(char):
    """{command name: keys} from the cmd file's [Command] groups."""
    out = {}
    path = char.file('cmd')
    if not path:
        return out
    for name, kv in M.read_ini(path):
        if name.lower() != 'command':
            continue
        d = {k: v for k, v, _ in kv if v is not None}
        if 'name' in d:
            out.setdefault(M.unquote(d['name']), M.unquote(d.get('command', '')))
    return out


def move_entries(char):
    """{state: (commands, power cost)} from the cmd file's state -1 ChangeStates whose value is a number."""
    out = {}
    path = char.file('cmd')
    if not path:
        return out
    table = command_table(char)
    for st in M.parse_states(path):
        if st.number != -1:
            continue
        for lbl, kv, ln in st.controllers:
            if (kv.get('type') or '').lower() != 'changestate':
                continue
            v = kv.get('value', '')
            if not INT.match(v or ''):
                continue
            state = int(float(v))
            trig = ' '.join(val for k, val in kv.items() if k.startswith('trigger'))
            cmds = [table.get(c, c) for c in re.findall(r'command\s*=\s*"([^"]+)"', trig, re.I)]
            power = 0
            for m in re.finditer(r'power\s*(>=|>)\s*(\d+)', trig, re.I):
                power = max(power, int(m.group(2)) + (1 if m.group(1) == '>' else 0))
            prev = out.get(state)
            if prev:
                cmds = prev[0] + [c for c in cmds if c not in prev[0]]
                power = min(prev[1], power) if prev[1] and power else max(prev[1], power)
            out[state] = (cmds, power)
    return out


def analyse_char(char, classes):
    states = char.states()
    air_path = char.file('anim')
    air = M.read_air_frames(air_path) if air_path else {}
    consts = char.constants()
    data, vel = consts.get('data', {}), consts.get('velocity', {})
    info = {
        'name': char.name, 'def': os.path.relpath(char.def_path, char.pack.root),
        'files': [os.path.relpath(p, char.pack.root) for p in char.dependencies()],
        'life': num(data.get('life'), 1000.0), 'attack': num(data.get('attack'), 100.0),
        'defence': num(data.get('defence'), 100.0), 'power': num(data.get('power'), 3000.0),
        'airjuggle': num(data.get('airjuggle'), 15.0),
        'walk_fwd': num(vel.get('walk.fwd')), 'walk_back': num(vel.get('walk.back')),
        'run_fwd': num(vel.get('run.fwd')), 'jump_neu': pair(vel.get('jump.neu'))[1],
    }
    moves = []
    Hit.consts = constant_vars(char)
    info['constants'] = {k: v for k, v in Hit.consts.items()}
    for state, (cmds, power) in sorted(move_entries(char).items()):
        if state not in states or state < 200 or 5000 <= state < 6000:
            continue
        mv = Move(char, state, cmds, power)
        follow(mv, states, air, set())
        moves.append(mv)
    return info, moves


def follow(mv, states, air, seen, depth=0, helper=False):
    """The move's states: its own, the ones it changes to (literal values, up to 3 deep, not back to idle or
    getting hit), and the helpers it makes."""
    todo = [(mv.state, 0, helper)]
    while todo:
        n, d, from_helper = todo.pop(0)
        if n in seen or n not in states or d > 3:
            continue
        seen.add(n)
        st = states[n]
        mv.states.append(n)
        pa = num(st.params.get('poweradd'))
        if pa is not None:
            if pa < 0:
                mv.power_cost = max(mv.power_cost, -pa)
            else:
                mv.power_gain += pa
        own = n == mv.state or from_helper
        if n == mv.state:
            mv.timing = timing(st, air)
            mv.air = (st.params.get('type') or '').strip().upper().startswith('A')
        for lbl, kv, ln in st.controllers:
            t = (kv.get('type') or '').lower()
            where = '%s:%d' % (os.path.relpath(st.file, mv.char.pack.root), ln)
            trig = ' '.join(v for k, v in kv.items() if k.startswith('trigger')).lower()
            if t == 'hitdef':
                h = Hit(n, 'helper' if from_helper else 'hitdef', kv, where)
                h.own = own
                mv.hits.append(h)
            elif t == 'projectile':
                h = Hit(n, 'projectile', kv, where)
                h.own = own
                mv.hits.append(h)
            elif t == 'targetlifeadd':
                v = kv.get('value', '0')
                lit = num(v)
                if lit is None or lit < 0:
                    h = Hit(n, 'lifeadd', {'damage': str(-lit) if lit is not None else v, 'guardflag': '',
                                           'attr': 'S,NT'}, where)
                    h.own = own
                    mv.hits.append(h)
            elif t == 'poweradd':
                v = num(kv.get('value'))
                if v is not None and v < 0:
                    mv.power_cost = max(mv.power_cost, -v)
            elif t == 'helper':
                hs = num(kv.get('stateno'))
                if hs is not None:
                    mv.helpers.append(int(hs))
                    todo.append((int(hs), d + 1, True))
            elif t in ('changestate', 'selfstate'):
                for m in NUM.finditer(kv.get('value') or ''):
                    v = int(m.group(1))
                    if 'movecontact' in trig or 'movehit' in trig or 'moveguarded' in trig:
                        mv.cancels.add(v)
                    if v >= 200 and not 5000 <= v < 6000 and v not in seen:
                        todo.append((v, d + 1, from_helper))


def timing(st, air):
    """Startup, active and recovery frames of the state's animation (from its hit boxes), or None."""
    a = num(st.params.get('anim'))
    if a is None:
        for lbl, kv, ln in st.controllers:
            if (kv.get('type') or '').lower() == 'changeanim' and 'time=0' in ''.join(
                    v for k, v in kv.items() if k.startswith('trigger')).replace(' ', '').lower():
                a = num(kv.get('value'))
                break
    if a is None or int(a) not in air:
        return None
    frames = air[int(a)]
    if not frames:
        return None
    t = 0
    first = last = None
    endless = False
    for f in frames:
        if f.time < 0:
            # a last frame held forever: AnimTime reaches 0 when it starts, which is where moves end
            if first is None and f.hit:
                first = t
                last = t + 1
            break
        if f.hit and first is None:
            first = t
        if f.hit:
            last = t + f.time
        t += f.time
    if first is None:
        return {'anim': int(a), 'total': None if endless else t, 'startup': None, 'endless': endless}
    return {'anim': int(a), 'startup': first + 1, 'active': last - first, 'recovery': None if endless else t - last,
            'total': None if endless else t, 'endless': endless}


# ------------------------------------------------------------------ the audit

def median(xs):
    xs = [x for x in xs if x is not None]
    return statistics.median(xs) if xs else None


def audit(pack, base):
    tol, classes = base['tolerance'], base['classes']
    roster = []
    for c in pack.roster():
        info, moves = analyse_char(c, classes)
        roster.append((c, info, moves))
    # the baseline: the roster's medians (or the fixed numbers)
    def dmg_class(mv):
        if mv.kind == 'normal' and not mv.air:
            return mv.button(classes)
        return mv.kind if mv.kind in ('special', 'super') else None
    if base['baseline'] == 'fixed':
        bl = dict(base['fixed'])
        bl_damage = dict(base['fixed']['damage'])
    else:
        bl = {k: median(info[k] for _, info, _ in roster) for k in ('life', 'attack', 'defence', 'power')}
        bl_damage = {}
        for cls in ('light', 'medium', 'heavy', 'special', 'super'):
            per_char = []
            for _, info, moves in roster:
                vals = [mv.total_damage() for mv in moves if dmg_class(mv) == cls and mv.hits and
                        not any(h.expression for h in mv.hits)]
                if vals:
                    per_char.append(median(vals))
            bl_damage[cls] = median(per_char)
    findings = []
    intentional = set(base.get('intentional', []))

    def add(kind, char, what, detail, state=None):
        key = '%s:%s' % (char, state) if state is not None else char
        if key in intentional and kind == 'mistake':
            kind = 'trait'
            detail += ' (marked intentional in the baseline file)'
        findings.append({'kind': kind, 'char': char, 'state': state, 'what': what, 'detail': detail})

    for c, info, moves in roster:
        name = info['name']
        # whole-character traits
        for k in ('life', 'attack', 'defence', 'power'):
            if bl.get(k) and info[k] is not None and abs(info[k] - bl[k]) / bl[k] > tol.get(k, 0.15):
                add('trait', name, k, '%s %g against the baseline\'s %g' % (k, info[k], bl[k]))
        ratios = []
        for mv in moves:
            cls = dmg_class(mv)
            d = mv.total_damage()
            if cls and bl_damage.get(cls) and mv.hits and not any(h.expression for h in mv.hits):
                ratios.append(d / bl_damage[cls])
        overall = median(ratios)
        if overall and abs(overall - 1) > tol['damage']:
            add('trait', name, 'damage', 'its hits are %.0f%% of the roster\'s on the whole' % (overall * 100))
        # per move
        by_button = {}
        expr_moves, code_timed = [], []
        for mv in moves:
            cls = dmg_class(mv)
            d = mv.total_damage()
            exprs = [h for h in mv.hits if h.expression]
            if exprs:
                expr_moves.append((mv.state, exprs[0].damage_raw))
            if cls and bl_damage.get(cls) and mv.hits and not exprs and overall:
                r = d / bl_damage[cls] / overall
                if abs(r - 1) > tol['damage'] * 1.5:
                    add('runtime', name, 'damage',
                        '%s %s does %g, %.0f%% of what its class does in this character\'s scale' % (
                            cls, mv.state, d, r * 100), mv.state)
            if mv.kind == 'normal' and cls and d > 0 and not exprs:
                fam = 'punch' if any(b in ''.join(mv.commands).lower() for b in 'xyz') else 'kick'
                by_button.setdefault(fam, {}).setdefault(cls, []).append((mv, d))
            for h in mv.hits:
                if h.guard_damage is not None and h.damage is not None and h.guard_damage > h.damage > 0:
                    add('mistake', name, 'guard damage', 'state %d does more on guard (%g) than on hit (%g) (%s)' % (
                        h.state, h.guard_damage, h.damage, h.where), mv.state)
                if not h.guardable and not h.throw and h.kind != 'projectile' and h.damage and h.p2stateno is None:
                    add('runtime', name, 'unblockable', 'state %d: a hit nobody can guard that isn\'t a throw (%s)' % (
                        h.state, h.where), mv.state)
                if h.hittime is not None and h.hittime > 40 and not h.fall:
                    add('runtime', name, 'stun', 'state %d holds the opponent %g frames without knocking down (%s)' % (
                        h.state, h.hittime, h.where), mv.state)
            if mv.kind == 'super' and mv.hits and d == 0 and not exprs:
                add('runtime', name, 'damage', 'super %d does no damage the files show (LifeAdd on itself? a counter?)'
                    % mv.state, mv.state)
            if mv.kind == 'super' and mv.power_cost <= 0:
                add('mistake', name, 'meter', 'super %d costs no power' % mv.state, mv.state)
            if mv.state in mv.cancels:
                add('runtime', name, 'loop', 'move %d can cancel into itself on contact' % mv.state, mv.state)
            tm = mv.timing
            if tm and tm.get('startup') and mv.hits and tm.get('recovery') is not None:
                h0 = next((h for h in mv.hits if h.kind == 'hitdef'), None)
                if h0 and h0.hittime is not None:
                    rest = tm['active'] - 1 + tm['recovery']
                    adv_hit = h0.hittime - rest
                    adv_guard = (h0.guard_hittime if h0.guard_hittime is not None else h0.hittime) - rest
                    tm['adv_hit'], tm['adv_guard'] = adv_hit, adv_guard
                    if mv.kind == 'normal' and not mv.air and not h0.fall and adv_hit > tol['advantage_hit']:
                        add('runtime', name, 'advantage', 'normal %d leaves %+d frames on hit (estimate)' % (
                            mv.state, adv_hit), mv.state)
                    if mv.kind == 'normal' and not mv.air and adv_guard > tol['advantage_guard']:
                        add('runtime', name, 'advantage', 'normal %d leaves %+d frames on guard (estimate)' % (
                            mv.state, adv_guard), mv.state)
                if cls == 'light' and tm['startup'] > tol['startup_light'] + 4:
                    add('runtime', name, 'startup', 'light %d starts in %d frames' % (mv.state, tm['startup']),
                        mv.state)
            elif mv.hits and (not tm or not tm.get('startup')):
                code_timed.append(mv.state)
        if expr_moves:
            add('runtime', name, 'damage', '%d moves give their damage as an expression (%s, e.g. %d: %s): the '
                'measured damage settles them' % (len(expr_moves), ', '.join(str(x) for x, _ in expr_moves[:12]) +
                                                  (' ...' if len(expr_moves) > 12 else ''), expr_moves[0][0],
                                                  expr_moves[0][1][:40]))
        if code_timed:
            add('runtime', name, 'timing', '%d moves hit from boxes or states the animation alone doesn\'t show '
                '(%s): their frames need the fights' % (len(code_timed), ', '.join(str(x) for x in code_timed[:12]) +
                                                        (' ...' if len(code_timed) > 12 else '')))
        # light / medium / heavy order inside each button family
        order = ['light', 'medium', 'heavy']
        for fam, per in by_button.items():
            meds = {cls: median(d for _, d in per[cls]) for cls in per}
            for a, b in zip(order, order[1:]):
                if meds.get(a) and meds.get(b) and meds[a] > meds[b] * 1.25:
                    add('mistake', name, 'damage order', '%s %s hits (%g) stronger than %s ones (%g)' % (
                        a, fam, meds[a], b, meds[b]))
    return roster, bl, bl_damage, findings


def runtime(dir_, roster):
    """Damage measured in the CPU fights (audithit lines), per character name and state."""
    out = {}
    for path in glob.glob(os.path.join(dir_, 'runs', '*', 'watch.txt')):
        for line in M.lines(M.read_text(path)):
            m = re.match(r'^audithit (.*) state (-?\d+) -> (.*) state (-?\d+) damage (\d+)', line)
            if m:
                out.setdefault((m.group(1), int(m.group(2))), []).append(int(m.group(5)))
    return out


def report(path, pack, roster, bl, bl_damage, findings, measured, internal_names):
    L = []
    L.append('# Fight balance audit')
    L.append('')
    L.append('Made by `tools/fight/balance_audit.py` (6.5) from the files in `mugen/`; run it again after any change '
             'to the characters (tools/fight/ingest.py does). Numbers are MUGEN\'s units: life points, frames at 60 a '
             'second. Timing is estimated from each move\'s animation and hit boxes; advantage = hit (or guard) stun '
             'minus the frames left after the first active one.')
    L.append('')
    L.append('Baseline: %s. Life %s, attack %s, defence %s, power %s; damage by class: %s.' % (
        'the roster\'s median' if not isinstance(bl.get('damage'), dict) else 'fixed numbers',
        bl.get('life'), bl.get('attack'), bl.get('defence'), bl.get('power'),
        ', '.join('%s %s' % (k, ('%g' % v) if v else '-') for k, v in bl_damage.items())))
    L.append('')
    kinds = [('mistake', 'Likely mistakes'), ('trait', 'Character traits (intentional differences)'),
             ('runtime', 'Needs a runtime check')]
    L.append('| | ' + ' | '.join(k[1].split(' (')[0] for k in kinds) + ' |')
    L.append('|---|' + '---|' * len(kinds))
    for c, info, moves in roster:
        L.append('| %s | %s |' % (info['name'], ' | '.join(
            str(sum(1 for f in findings if f['char'] == info['name'] and f['kind'] == k)) for k, _ in kinds)))
    L.append('')
    for k, title in kinds:
        L.append('## ' + title)
        L.append('')
        rows = [f for f in findings if f['kind'] == k]
        if not rows:
            L.append('None.')
        for f in rows:
            L.append('- **%s** %s: %s' % (f['char'], f['what'], f['detail']))
        L.append('')
    L.append('## Characters')
    L.append('')
    for c, info, moves in roster:
        L.append('### %s' % info['name'])
        L.append('')
        L.append('Life %g, attack %g, defence %g, power %g, air juggle %g; walk %s / %s, run %s, jump %s. Files: %s.' % (
            info['life'], info['attack'], info['defence'], info['power'], info['airjuggle'], info['walk_fwd'],
            info['walk_back'], info['run_fwd'], info['jump_neu'], ', '.join('`%s`' % f for f in info['files'])))
        L.append('')
        L.append('| state | kind | command | hits | damage (guard) | stun hit / guard | startup / active / recovery '
                 '| adv. hit / guard | power | cancels | measured |')
        L.append('|---|---|---|---|---|---|---|---|---|---|---|')
        for mv in moves:
            if not mv.hits:
                continue
            tm = mv.timing or {}
            dmg = ' + '.join(('%g' % h.damage if h.damage is not None else 'expr') +
                             ('x%d' % h.numhits if h.numhits and h.numhits > 1 else '') +
                             (' (%g)' % h.guard_damage if h.guard_damage else '') +
                             (' [%s]' % h.kind[0] if h.kind != 'hitdef' else '') for h in mv.hits[:6])
            if len(mv.hits) > 6:
                dmg += ' ...'
            stun = ', '.join('%s/%s' % (('%g' % h.hittime) if h.hittime is not None else '?',
                                        ('%g' % h.guard_hittime) if h.guard_hittime is not None else '?')
                             for h in mv.hits[:3])
            timing_s = '%s / %s / %s' % (tm.get('startup', '-'), tm.get('active', '-'), tm.get('recovery', '-')) \
                if tm else '-'
            adv = '%+d / %+d' % (tm['adv_hit'], tm['adv_guard']) if 'adv_hit' in tm else '-'
            meas = ''
            for nm in internal_names.get(info['name'], [info['name']]):
                vals = measured.get((nm, mv.state))
                if vals:
                    meas = '%d hits, median %g' % (len(vals), statistics.median(vals))
            cmd = (mv.commands[0] if mv.commands else '')[:24].replace('|', '/')
            L.append('| %d | %s | `%s` | %d | %s | %s | %s | %s | %s | %s | %s |' % (
                mv.state, mv.kind, cmd, len(mv.hits), dmg, stun, timing_s, adv,
                ('-%d' % mv.power_cost if mv.power_cost else '') + (' +%g' % mv.power_gain if mv.power_gain else ''),
                ', '.join(str(x) for x in sorted(mv.cancels))[:30], meas or '-'))
        L.append('')
    with open(path, 'w') as f:
        f.write('\n'.join(L) + '\n')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default='mugen')
    ap.add_argument('--baseline', default=os.path.join(HERE, 'balance_baseline.json'))
    ap.add_argument('--runtime', help='an audit_chars.sh result folder (build/fight/audit)')
    ap.add_argument('--json')
    ap.add_argument('--report')
    a = ap.parse_args()
    base = json.loads(json.dumps(DEFAULT_BASELINE))
    if a.baseline and os.path.exists(a.baseline):
        with open(a.baseline) as f:
            user = json.load(f)
        for k, v in user.items():
            if isinstance(v, dict) and isinstance(base.get(k), dict):
                base[k].update(v)
            else:
                base[k] = v
    pack = M.Pack(a.root)
    roster, bl, bl_damage, findings = audit(pack, base)
    measured = runtime(a.runtime, roster) if a.runtime else {}
    # the engine logs the internal [Info] name; the report shows the display name
    names = {}
    for c, info, moves in roster:
        names.setdefault(info['name'], []).append(M.unquote(c.info.get('name', info['name'])))
    if measured:
        for c, info, moves in roster:
            for mv in moves:
                for nm in names[info['name']]:
                    vals = measured.get((nm, mv.state))
                    d = mv.total_damage()
                    if vals and d and not any(h.expression for h in mv.hits):
                        med = statistics.median(vals)
                        single = mv.hits[0].damage or 0
                        if med > d * 1.5 or (single and med < single * 0.5):
                            findings.append({'kind': 'runtime', 'char': info['name'], 'state': mv.state,
                                             'what': 'measured damage',
                                             'detail': 'state %d: hits measured at %g (median of %d) against %g in '
                                                       'the files' % (mv.state, med, len(vals), d)})
    counts = {k: sum(1 for f in findings if f['kind'] == k) for k in ('mistake', 'trait', 'runtime')}
    for f in findings:
        print('%-8s %-10s %-14s %s' % (f['kind'], f['char'], f['what'], f['detail']))
    print('%d characters, %d moves; %d likely mistakes, %d character traits, %d to check at run time' % (
        len(roster), sum(len(m) for _, _, m in roster), counts['mistake'], counts['trait'], counts['runtime']))
    if a.report:
        report(a.report, pack, roster, bl, bl_damage, findings, measured, names)
    if a.json:
        out = {'baseline': bl, 'baseline_damage': bl_damage, 'findings': findings,
               'characters': [{**info, 'moves': [{'state': mv.state, 'kind': mv.kind, 'commands': mv.commands,
                                                  'damage': mv.total_damage(), 'power_cost': mv.power_cost,
                                                  'timing': mv.timing, 'cancels': sorted(mv.cancels),
                                                  'hits': len(mv.hits)} for mv in moves]}
                              for c, info, moves in roster]}
        with open(a.json, 'w') as f:
            json.dump(out, f, indent=1)
    return 0


if __name__ == '__main__':
    sys.exit(main())
