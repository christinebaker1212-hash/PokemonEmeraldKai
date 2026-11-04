#!/usr/bin/env python3
"""Crystal's WEEKDAY siblings in JOHTO (docs/REGIONS_AUDIT.md case 63; Quetzal kept them out every day with a
guess-the-day menu).

    python3 tools/quetzal_port/fix_siblings.py

Each sibling's object script becomes Crystal's (pokecrystal maps/<MAP>.asm <Name>Script): on their own day they
introduce themselves once and give their gift; on other days "Today is not <DAY>"... (they only stand there on their
day: the object wears FLAG_JOHTO_HIDE_<NAME>, set or cleared on entering a JOHTO map by mp_region.c
MP_JohtoWeekdayPeople). The gift keeps Quetzal's got-it flag, so saves that already have it keep it. Their words are
Crystal's (restore_words.txt). Running it again changes nothing.
"""
import json
import os
import re

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# (our map, local id, Crystal map, Name, Day, weekday 0 = Sunday, Crystal's gift as this game's item)
SIBLINGS = [
    ('Route40', 11, 'Route40', 'Monica', 'Monday', 1, 'ITEM_SHARP_BEAK'),
    ('Route29', 4, 'Route29', 'Tuscany', 'Tuesday', 2, 'ITEM_SILK_SCARF'),        # Crystal's PINK BOW
    ('LakeOfRage', 6, 'LakeOfRage', 'Wesley', 'Wednesday', 3, 'ITEM_BLACK_BELT'),
    ('Route36', 2, 'Route36', 'Arthur', 'Thursday', 4, 'ITEM_HARD_STONE'),
    ('Route32', 17, 'Route32', 'Frieda', 'Friday', 5, 'ITEM_POISON_BARB'),
    ('BlackthornCity', 3, 'BlackthornCity', 'Santos', 'Saturday', 6, 'ITEM_SPELL_TAG'),
    ('Route37', 4, 'Route37', 'Sunny', 'Sunday', 0, 'ITEM_MAGNET'),
]


def main():
    lines = []
    for ours, lid, theirs, name, day, wd, item in SIBLINGS:
        up = name.upper()
        mapdir = os.path.join(REPO, 'data/maps', 'Johto_' + ours)
        mj = json.load(open(os.path.join(mapdir, 'map.json')))
        obj = mj['object_events'][lid - 1]
        obj['flag'] = 'FLAG_JOHTO_HIDE_' + up
        label = obj['script']
        json.dump(mj, open(os.path.join(mapdir, 'map.json'), 'w'), indent=2)
        open(os.path.join(mapdir, 'map.json'), 'a').write('\n')

        path = os.path.join(mapdir, 'scripts.inc')
        s = open(path).read()
        start = s.index(label + '::')
        # the old script runs until the next object's comment or a label that isn't one of its own branches
        end = s.find('\n@ ', start)
        end = len(s) if end < 0 else end + 1
        txt = re.search(r'^\w+_Text_\w+::', s[start:], re.M)   # (never past the map's texts)
        if txt:
            end = min(end, start + txt.start())
        old = s[start:end]
        got = re.search(r'setflag (FLAG_QZ_0x[0-9A-F]+)', old)
        if '@ Moemon Kai: Crystal\'s WEEKDAY' in old:
            gotflag = re.search(r'goto_if_set (\w+), ', old).group(1)
        else:
            gotflag = got.group(1)
        p = 'Johto_%s_' % ours
        t = lambda n: p + 'Text_' + name + n
        gift = [t('GivesGift')] if name != 'Sunny' else None
        body = [
            label + '::',
            '\t@ Moemon Kai: Crystal\'s WEEKDAY sibling (pokecrystal %s.asm %sScript; tools/quetzal_port/fix_siblings.py):'
            % (theirs, name),
            '\t@ only on %s (FLAG_JOHTO_HIDE_%s), once met they give the gift' % (day.upper(), up),
            '\tlock', '\tfaceplayer',
            '\tgoto_if_set %s, %sEventScript_%sDay' % (gotflag, p, name),
            '\tspecialvar VAR_RESULT, MP_JohtoWeekday',
            '\tgoto_if_ne VAR_RESULT, %d, %sEventScript_%sNotDay' % (wd, p, name),
            '\tgoto_if_set FLAG_JOHTO_MET_%s, %sEventScript_%sMet' % (up, p, name),
            '\tmsgbox %s, MSGBOX_DEFAULT' % t('Meet'),
            '\tsetflag FLAG_JOHTO_MET_%s' % up,
            '%sEventScript_%sMet::' % (p, name),
        ]
        if name == 'Sunny':
            body += ['\tcheckplayergender', '\tgoto_if_eq VAR_RESULT, FEMALE, %sEventScript_SunnyKris' % p,
                     '\tmsgbox %s, MSGBOX_DEFAULT' % t('GivesGift1'), '\tgoto %sEventScript_SunnyGift' % p,
                     '%sEventScript_SunnyKris::' % p, '\tmsgbox %s, MSGBOX_DEFAULT' % t('GivesGift2'),
                     '%sEventScript_SunnyGift::' % p]
        else:
            body += ['\tmsgbox %s, MSGBOX_DEFAULT' % t('GivesGift')]
        body += [
            '\tsetorcopyvar VAR_0x8000, %s' % item, '\tsetorcopyvar VAR_0x8001, 1', '\tcallstd STD_OBTAIN_ITEM',
            '\tgoto_if_eq VAR_RESULT, 0, Quetzal_EventScript_83602AA',
            '\tsetflag %s' % gotflag,
            '\tmsgbox %s, MSGBOX_DEFAULT' % t('GaveGift'), '\trelease', '\tend', '',
            '%sEventScript_%sDay::' % (p, name), '\tmsgbox %s, MSGBOX_DEFAULT' % t(day), '\trelease', '\tend', '',
            '%sEventScript_%sNotDay::' % (p, name), '\tmsgbox %s, MSGBOX_DEFAULT' % t('Not' + day), '\trelease',
            '\tend', '', '']
        s = s[:start] + '\n'.join(body) + s[end:]
        texts = ['Meet', 'GaveGift', day, 'Not' + day] + (['GivesGift1', 'GivesGift2'] if name == 'Sunny' else ['GivesGift'])
        for n in texts:
            crystal = ('Meet%sText' % name) if n == 'Meet' else ('%s%sText' % (name, n) if not n.startswith('GivesGift')
                                                                  else '%sGivesGiftText%s' % (name, n[9:]))
            if not re.search(r'^%s::' % re.escape(t(n)), s, re.M):
                s += '\n%s::\n\t.string "x$"\n' % t(n)
            lines.append('Johto_%s %s gsc %s %s' % (ours, t(n), theirs, crystal))
        open(path, 'w').write(s)
    lst = os.path.join(REPO, 'tools/quetzal_port/restore_words.txt')
    have = open(lst).read()
    new = [l for l in lines if l not in have]
    if new:
        open(lst, 'a').write('# case 63: the WEEKDAY siblings (tools/quetzal_port/fix_siblings.py)\n' + '\n'.join(new) + '\n')


if __name__ == '__main__':
    main()
