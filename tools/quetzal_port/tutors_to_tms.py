#!/usr/bin/env python3
"""KANTO's move tutors give TMs again, as in RED (the user, 2026-11-21: "return the TMs to the tutors who originally gave
them in Pokemon Red with their actual TM equivalents - no more tutors in Kanto"). Every KANTO tutor fix_move_tutors.py
built on Kai's tutor flow now hands over a TM once (Kai's TM for the move, any number: TM93 on are RED / BLUE's own,
tools/moemon_tms/gen_tm3.py):

- RED's own TM givers, where FireRed put a tutor (pret/pokered scripts/*.asm `lb bc, TM_*` + text/*.asm): CELADON
  CITY's old man (SOFTBOILED), CELADON DEPT. STORE 3F (COUNTER), CINNABAR LAB (METRONOME), the COPYCAT (MIMIC, for her
  POKé DOLL), VIRIDIAN CITY's sleepy man (DREAM EATER) and SILPH CO. 2F's worker, whose is SELFDESTRUCT again (FireRed
  made her a THUNDER WAVE tutor): RED's own words before the gift and after (TM numbers as Kai's).
- FireRed's own tutors (ROUTE 4's two, PEWTER's MUSEUM, ROCK TUNNEL, FUCHSIA, VICTORY ROAD 2F, FOUR / SEVEN ISLAND,
  MT. EMBER): the TM for the move they taught (RED's TM of it), their FireRed words with the offer turned into the gift.

Each has a "got it" flag of its own (FLAG_KANTO_GOT_TM_*, include/constants/kai_regions.h): a save that was taught the
move by the tutor still gets the TM once. Run once with RED's decomp; its edits are committed.

    python3 -I tools/quetzal_port/tutors_to_tms.py POKERED_DIR
"""
import os
import re
import sys
import textwrap

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# map, tutor move (TUTOR_MOVE_*), the move given (MOVE_*), RED's map file for RED's own givers, its flag
TUTORS = [
    ('Kanto_CeladonCity', 'SOFT_BOILED', 'SOFT_BOILED', 'CeladonCity', 0x1564),
    ('Kanto_CeladonCity_DepartmentStore_3F', 'COUNTER', 'COUNTER', 'CeladonMart3F', 0x1565),
    ('Kanto_CinnabarIsland_PokemonLab_ResearchRoom', 'METRONOME', 'METRONOME', 'CinnabarLabMetronomeRoom', 0x1566),
    ('Kanto_SaffronCity_CopycatsHouse_2F', 'MIMIC', 'MIMIC', 'CopycatsHouse2F', 0x1567),
    ('Kanto_ViridianCity', 'DREAM_EATER', 'DREAM_EATER', 'ViridianCity', 0x1568),
    ('Kanto_SilphCo_2F', 'THUNDER_WAVE', 'SELF_DESTRUCT', 'SilphCo2F', 0x156A),
    ('Kanto_Route4', 'MEGA_PUNCH', 'MEGA_PUNCH', None, 0x156B),
    ('Kanto_Route4', 'MEGA_KICK', 'MEGA_KICK', None, 0x156C),
    ('Kanto_PewterCity_Museum_1F', 'SEISMIC_TOSS', 'SEISMIC_TOSS', None, 0x156D),
    ('Kanto_RockTunnel_B1F', 'ROCK_SLIDE', 'ROCK_SLIDE', None, 0x156E),
    ('Kanto_FuchsiaCity', 'SUBSTITUTE', 'SUBSTITUTE', None, 0x156F),
    ('Kanto_VictoryRoad_2F', 'DOUBLE_EDGE', 'DOUBLE_EDGE', None, 0x1570),
    ('Kanto_FourIsland_House1', 'BODY_SLAM', 'BODY_SLAM', None, 0x1571),
    ('Kanto_SevenIsland', 'SWORDS_DANCE', 'SWORDS_DANCE', None, 0x1572),
    ('Kanto_MtEmber_Exterior', 'EXPLOSION', 'EXPLOSION', None, 0x1573),
]

# FireRed's own tutors: their offer (the last paragraph of their words) as the gift
OFFERS = {
    'MEGA_PUNCH': ('Now!\\n', 'Let me teach it to your MOéMON!', 'Take this TM for your MOéMON!'),
    'MEGA_KICK': ('Okay!\\n', "I'll teach it to your MOéMON!", "Here's the TM for your MOéMON!"),
    'SEISMIC_TOSS': ('Well, how about SEISMIC TOSS?\\n', 'Should I teach that to a MOéMON?',
                     'Take this TM for a MOéMON!'),
    'ROCK_SLIDE': ('You don\'t seem to be scared.\\n', 'Want to try using ROCK SLIDE?', 'Here, try this TM: ROCK SLIDE!'),
    'SUBSTITUTE': ('Want me to teach SUBSTITUTE to\\n', 'one of your MOéMON?', None),
    'DOUBLE_EDGE': None,
    'BODY_SLAM': ('How about I teach BODY SLAM to\\n', 'a MOéMON of yours instead?', None),
    'SWORDS_DANCE': ('How would you like to learn a move\\n', "like that? It's SWORDS DANCE.", None),
    'EXPLOSION': ('While we\'re terrified, would you\\n', 'like me to teach EXPLOSION?', None),
}
# (the ones a line can't carry: the whole last paragraph)
WHOLE = {
    'SUBSTITUTE': '\t.string "Here, take this TM and teach\\n"\n\t.string "SUBSTITUTE to one of your MOéMON!$"\n',
    'BODY_SLAM': '\t.string "So take this TM instead, and\\n"\n\t.string "teach BODY SLAM to a MOéMON!$"\n',
    'SWORDS_DANCE': '\t.string "How would you like to learn a move\\n"\n\t.string "like that? Take this TM:\\l"\n'
                    '\t.string "SWORDS DANCE!$"\n',
    'EXPLOSION': '\t.string "While we\'re terrified, take this\\n"\n\t.string "TM. It\'s EXPLOSION!$"\n',
    'DOUBLE_EDGE': None,
}


def tm_numbers():
    def moves(path, macro):
        s = open(os.path.join(REPO, path)).read()
        body = s[s.index('#define %s(F)' % macro):]
        body = body[:body.index('\n\n')] if '\n\n' in body else body
        return re.findall(r'F\((\w+)\)', body)
    num = {}
    for path, macro, first in (('include/constants/tms_hms.h', 'FOREACH_TM', 1),
                               ('include/constants/tms_hms_tm2.h', 'FOREACH_TM2', 51),
                               ('include/constants/tms_hms_tm3.h', 'FOREACH_TM3', 93),
                               ('include/constants/tms_hms_tm4.h', 'FOREACH_TM4', 140)):
        for i, m in enumerate(moves(path, macro)):
            num.setdefault(m, first + i)
    return num


def red_text(pokered, mapfile, label):
    """RED's text LABEL (text/<map>.asm, text_far'd from scripts/<map>.asm) as Emerald's .string lines"""
    body = open(os.path.join(pokered, 'text', mapfile + '.asm')).read()
    m = re.search(r'^%s::\n(.*?)^\s*(prompt|done|text_end)' % re.escape(label), body, re.M | re.S)
    paras, cur = [], []
    for line in m.group(1).split('\n'):
        t = re.match(r'\s*(text|line|cont|para)\s+"(.*)"', line)
        if not t:
            continue
        if t.group(1) == 'para' and cur:
            paras.append(' '.join(cur))
            cur = []
        cur.append(t.group(2))
    if cur:
        paras.append(' '.join(cur))
    out = []
    for p in paras:
        p = p.replace('#MON', 'MOéMON').replace('#', 'POKé').replace('...', '…').replace('<PLAYER>', '{PLAYER}').replace("'", '’')
        lines = textwrap.wrap(p, 30)
        for k, l in enumerate(lines):
            out.append(l + ('\\n' if k == 0 and len(lines) > 1 else '\\l' if k < len(lines) - 1 else ''))
        out[-1] += '\\p'
    out[-1] = out[-1][:-2] + '$'
    return ''.join('\t.string "%s"\n' % l for l in out)


def red_labels(pokered, mapfile):
    """the text RED shows before its gift and after (when already given)"""
    s = open(os.path.join(pokered, 'scripts', mapfile + '.asm')).read()
    give = s.index('lb bc, TM_')
    pre = re.findall(r'ld hl, (\.\w+)', s[:give])[-1]
    after = re.search(r'\.(got_item|gotTM\d+|already_have_tm)\n\s*ld hl, (\.\w+)', s[give:])

    def far(local):   # (local labels repeat from function to function: the one after the gift's)
        return re.search(r'^%s:\n\s*text_far (\w+)' % re.escape(local), s[give:], re.M).group(1)
    return far(pre), far(after.group(2)) if after else None


def replace_text(s, label, new):
    m = re.search(r'^%s::?\n((?:\t\.string .*\n)+)' % re.escape(label), s, re.M)
    assert m, label
    return s[:m.start(1)] + new + s[m.end(1):]


def drop_text(s, label):
    if re.search(r'\b%s\b' % re.escape(label), re.sub(r'^%s::?\n' % re.escape(label), '', s, flags=re.M)):
        return s   # (still used)
    return re.sub(r'^%s::?\n(?:\t\.string .*\n)+\n?' % re.escape(label), '', s, flags=re.M)


def main():
    pokered = sys.argv[1]
    num = tm_numbers()
    for mapname, tutor, move, redmap, flag in TUTORS:
        path = os.path.join(REPO, 'data/maps', mapname, 'scripts.inc')
        s = open(path).read()
        tm = num[move]
        seg = re.search(r'\tgoto_if_set (0x[0-9A-F]+), (\w+)\n\tmsgbox (\w+), MSGBOX_YESNO\n'
                        r'\tgoto_if_eq VAR_RESULT, NO, (\w+)\n(?:.*\n)*?\tsetvar VAR_0x8005, TUTOR_MOVE_%s\n'
                        r'((?:.*\n)*?)\tsetflag \1\n' % tutor, s)
        assert seg, (mapname, tutor)
        taught_flag, taught, teach, declined, rest = seg.groups()
        which = re.search(r'msgbox (\w+WhichMon), MSGBOX_DEFAULT', seg.group(0))
        keep = ''.join(l + '\n' for l in rest.split('\n') if l.startswith('\tremoveitem'))
        code = ('\tgoto_if_set FLAG_KANTO_GOT_TM_%s, %s\n'
                '\tmsgbox %s, MSGBOX_DEFAULT\n'
                '\tgiveitem ITEM_TM%02d\n'
                '\tgoto_if_eq VAR_RESULT, FALSE, %s\n'
                '%s'
                '\tsetflag FLAG_KANTO_GOT_TM_%s\n') % (move, taught, teach, tm, declined, keep, move)
        s = s[:seg.start()] + code + s[seg.end():]
        # the "declined" script: the bag-full line was the std's own; just goes
        dm = re.search(r'^%s::?\n\tmsgbox (\w+), MSGBOX_DEFAULT\n\trelease\n\tend\n' % re.escape(declined), s, re.M)
        if dm and len(re.findall(r'\b%s\b' % re.escape(declined), s)) == 2:
            s = s[:dm.start()] + '%s::\n\trelease\n\tend\n' % declined + s[dm.end():]
            s = drop_text(s, dm.group(1))
        if which:
            s = drop_text(s, which.group(1))
        if redmap:
            pre, after = red_labels(pokered, redmap)
            s = replace_text(s, teach, red_text(pokered, redmap, pre))
            if mapname != 'Kanto_SaffronCity_CopycatsHouse_2F':   # (FireRed's after-words for her, by gender)
                tm_line = re.sub(r'TM\d+', 'TM%02d' % tm, red_text(pokered, redmap, after))
                if move == 'SOFT_BOILED':   # (RED's "only CHANSEY can use it": not so here)
                    tm_line = tm_line[:tm_line.index('\\p')] + '$"\n'
                tl = re.search(r'^%s::?\n\tmsgbox (\w+), MSGBOX_DEFAULT' % re.escape(taught), s, re.M).group(1)
                s = replace_text(s, tl, tm_line)
        else:
            m = re.search(r'^%s::?\n((?:\t\.string .*\n)+)' % re.escape(teach), s, re.M)
            lines = m.group(1)
            if move == 'DOUBLE_EDGE':
                new = lines[:lines.index('\t.string "I\'ll teach you DOUBLE-EDGE.')]
                new += '\t.string "please take this TM: DOUBLE-EDGE!$"\n'
            elif WHOLE.get(move):
                first = OFFERS[move][0]
                new = lines[:lines.index('\t.string "%s"' % first)] + WHOLE[move]
            else:
                first, old, rep = OFFERS[move]
                assert '"%s$"' % old in lines, (move, lines)
                new = lines.replace('"%s$"' % old, '"%s$"' % rep)
            s = replace_text(s, teach, new)
        # the comment above the script: what it is now
        s = re.sub(r"(@ Moemon Kai: FireRed's [^\n]*?tutor[^\n]*?), on Kai's(\n\t?@)? move tutors",
                   lambda m: m.group(1) + ",\n\t@ now gives its TM (tools/quetzal_port/tutors_to_tms.py)", s)
        open(path, 'w').write(s)
        print('%-45s %-13s -> TM%d %s' % (mapname, tutor, tm, move))


if __name__ == '__main__':
    main()
