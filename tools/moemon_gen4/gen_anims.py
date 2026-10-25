#!/usr/bin/env python3
"""Gen 4 move animations from pokeemerald-expansion, where they only need what Kai has.

    python3 tools/moemon_gen4/gen_anims.py EXPANSION_DIR [--list]

For each Gen 4 move still borrowing a Gen 3 animation in data/battle_anim_scripts_gen4.inc (a lone "goto Move_X"),
takes expansion's gBattleAnimMove_<Name> when every sprite template, visual task, sound, tag, background and script
command it uses exists in Kai (checked against the built game's symbols and Kai's headers and macros), together
with the shared routines it calls (UnsetBackground, CreateBite...), which are copied once with a G4_ prefix (as are
the script's own labels, so nothing clashes with Kai's). Animations written by hand (anything but a lone goto)
are never touched; moves that can't be ported keep borrowing. --list only prints what it would do.
Build the game first (it reads the symbols of ./pokeemerald64).
"""
import glob
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(REPO, 'data', 'battle_anim_scripts_gen4.inc')
HELPERS_START = '@ ---- routines shared by the animations above (from pokeemerald-expansion, G4_ prefixed)'


def camel(const):
    return ''.join(w.capitalize() for w in const[5:].split('_'))


def blocks(text, label_re):
    """label -> body for every top-level label matching label_re (body runs to the next top-level label)."""
    out = {}
    marks = [(m.start(), m.group(1)) for m in re.finditer(r'^(\w+)::?\s*$', text, re.M)]
    for i, (pos, name) in enumerate(marks):
        if re.match(label_re, name):
            end = marks[i + 1][0] if i + 1 < len(marks) else len(text)
            out[name] = text[text.index('\n', pos) + 1:end]
    return out


def main():
    exp = sys.argv[1]
    src = open(os.path.join(exp, 'data', 'battle_anim_scripts.s'), encoding='utf-8').read()
    kai = open(os.path.join(REPO, 'data', 'battle_anim_scripts.s'), encoding='utf-8').read()
    syms = set(subprocess.run(['nm', os.path.join(REPO, 'pokeemerald64')], capture_output=True, text=True).stdout.split())
    consts = ''.join(open(os.path.join(REPO, p), encoding='utf-8').read() for p in
                     ('include/constants/battle_anim.h', 'include/constants/songs.h', 'include/constants/battle.h'))
    macros = set(re.findall(r'\.macro (\w+)', open(os.path.join(REPO, 'asm/macros/battle_anim_script.inc')).read()))
    macros |= {'end', 'delay', 'goto', 'call', 'return'}
    all_labels = blocks(src, r'.')
    moves = [l.split()[1] for l in open(os.path.join(REPO, 'include/constants/moves.h'))
             if l.startswith('#define MOVE_') and l.split()[2].isdigit() and 355 <= int(l.split()[2]) <= 467]

    def local_labels(body):
        return set(re.findall(r'^(\w+):', body, re.M))

    def problems(body, seen):
        """What this body (and the routines it calls) needs that Kai lacks; the routines it calls go in seen."""
        bad = []
        for i in set(re.findall(r'\b(g[A-Z]\w+|AnimTask_\w+|AnimTask\w+)\b', body)):
            if i not in syms:
                bad.append(i)
        for c in set(re.findall(r'\b(ANIM_TAG_\w+|SE_\w+|BG_\w+|MUS_\w+|F_PAL_\w+|B_ANIM_\w+)\b', body)):
            if not re.search(r'\b%s\b' % c, consts):
                bad.append(c)
        for c in set(re.findall(r'^\s+(\w+)', body, re.M)):
            if c not in macros:
                bad.append('cmd ' + c)
        mine = local_labels(body)
        for t in set(re.findall(r'\b(?:call|goto|jumpargeq|jumpifcontest|jumpifmoveturn|choosetwoturnanim|jumpifdoublebattle|jumpretfalse|jumprettrue)\b[^\n]*', body)):
            for lab in re.findall(r'\b([A-Z]\w+)\b', t):
                if lab in mine or lab.startswith(('ANIM_', 'SE_', 'BG_', 'TRUE', 'FALSE')):
                    continue
                if lab in all_labels and not lab.startswith('gBattleAnim'):
                    if lab not in seen:
                        seen.add(lab)
                        bad += problems(all_labels[lab], seen)
                else:
                    bad.append('label ' + lab)
        return bad

    have = open(OUT, encoding='utf-8').read()
    have = have.split(HELPERS_START)[0].rstrip() + '\n'
    cur = blocks(have, r'Move_')
    helpers, ported, kept = set(), [], []
    for m in moves:
        name = 'Move_' + m[5:]
        body = cur.get(name, '')
        if body and not re.fullmatch(r'\s*goto Move_\w+\s*', body):
            kept.append(m)          # written by hand
            continue
        eb = all_labels.get('gBattleAnimMove_' + camel(m))
        seen = set()
        if eb is None or problems(eb, seen):
            continue
        helpers |= seen
        ported.append((m, eb))
    print('%d ported, %d hand-written kept, %d still borrowing' % (len(ported), len(kept), len(moves) - len(ported) - len(kept)))
    if '--list' in sys.argv:
        print(' '.join(m for m, _ in ported))
        return

    def kai_syntax(body):
        # expansion's invert_screen_color takes one mask (1 scenery, 2 attacker, 4 target); Kai's the three flags
        def inv(m):
            v = eval(m.group(1))
            return 'invert_screen_color scenery=%d, attacker=%d, target=%d' % (v & 1, (v >> 1) & 1, (v >> 2) & 1)
        body = re.sub(r'invert_screen_color scenery=([0-9| ]+)$', inv, body, flags=re.M)
        return re.sub(r'\bSOUND_PAN_MIDDLE\b', '0', body)   # Kai has no name for the middle

    def prefix(body, own):
        for lab in sorted(own | helpers, key=len, reverse=True):
            body = re.sub(r'\b%s\b' % lab, 'G4_' + lab, body)
        return body

    # expansion loads a template's graphics by itself; Kai needs loadspritegfx for each tag a move shows
    tag_of = {}
    for f in glob.glob(os.path.join(REPO, 'src', '*.c')):
        t = open(f, encoding='utf-8', errors='ignore').read()
        tag_of.update(re.findall(r'const struct SpriteTemplate (g\w+) =\s*\{\s*\.tileTag = (ANIM_TAG_\w+)', t))
    macro_tpl = {n: re.findall(r'createsprite (g\w+)', b) for n, b in re.findall(
        r'\.macro (\w+)[^\n]*\n(.*?)\.endm', open(os.path.join(REPO, 'asm/macros/battle_anim_script.inc')).read(), re.S)}

    def tags_needed(body, seen=None):
        seen = seen if seen is not None else set()
        tpls = set(re.findall(r'createsprite\w* (g\w+)', body))
        for mac in re.findall(r'^\s+(\w+)', body, re.M):
            tpls |= set(macro_tpl.get(mac, ()))
        tags = {tag_of[t] for t in tpls if t in tag_of}
        for lab in re.findall(r'\b(?:call|goto)\s+(\w+)', body):
            if lab in helpers and lab not in seen:
                seen.add(lab)
                tags |= tags_needed(all_labels[lab], seen)
        return tags

    for m, eb in ported:
        name = 'Move_' + m[5:]
        loads = ''.join('\tloadspritegfx %s\n' % t for t in sorted(tags_needed(eb))
                        if not re.search(r'loadspritegfx %s\b' % t, eb))
        new = loads + kai_syntax(prefix(eb, local_labels(eb))).rstrip() + '\n\n'
        old = re.search(r'^%s:\n(.*?)(?=^\w+:|\Z)' % name, have, re.M | re.S)
        have = have[:old.start(1)] + new + have[old.end(1):]
    tail = '\n' + HELPERS_START + '\n'
    for h in sorted(helpers):
        tail += 'G4_%s:\n%s\n' % (h, kai_syntax(prefix(all_labels[h], local_labels(all_labels[h]))).rstrip())
    open(OUT, 'w', encoding='utf-8').write(have.rstrip() + '\n' + (tail if helpers else '') + '\n')


if __name__ == '__main__':
    main()
