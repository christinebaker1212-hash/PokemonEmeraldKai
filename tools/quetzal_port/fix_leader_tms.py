#!/usr/bin/env python3
"""KANTO's gym leaders give FireRed's TMs again (Quetzal gave held items: LEFTOVERS, CHOICE SCARF...).

    python3 tools/quetzal_port/fix_leader_tms.py      (run from the repository; safe to run again)

Each leader's gift block (checkitemspace ... the held item's explanation) becomes FireRed's: the TM (giveitem), the
same "got it" flag, and FireRed's explanation of the TM (pokefirered data/maps/<gym>/text.inc ExplainTMxx, its
POKéMON spelt MOéMON as the game does), written over the old explanation's text.
"""
import os
import re
import urllib.request

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GYMS = {   # map: (TM, FireRed's explanation label)
    'PewterCity_Gym': ('ITEM_TM39', 'PewterCity_Gym_Text_ExplainTM39'),
    'CeruleanCity_Gym': ('ITEM_TM03', 'CeruleanCity_Gym_Text_ExplainTM03'),
    'VermilionCity_Gym': ('ITEM_TM34', 'VermilionCity_Gym_Text_ExplainTM34'),
    'CeladonCity_Gym': ('ITEM_TM19', 'CeladonCity_Gym_Text_ExplainTM19'),
    'FuchsiaCity_Gym': ('ITEM_TM06', 'FuchsiaCity_Gym_Text_KogaExplainTM06'),
    'SaffronCity_Gym': ('ITEM_TM04', 'SaffronCity_Gym_Text_ExplainTM04'),
    'CinnabarIsland_Gym': ('ITEM_TM38', 'CinnabarIsland_Gym_Text_BlainePostBattle'),
    'ViridianCity_Gym': ('ITEM_TM26', 'ViridianCity_Gym_Text_ExplainTM26'),
}
RAW = 'https://raw.githubusercontent.com/pret/pokefirered/master/data/maps/%s/text.inc'


def frlg_text(gym, label):
    text = urllib.request.urlopen(RAW % gym).read().decode('utf-8')
    m = re.search(re.escape(label) + r'::\n((?:\s*\.string .*\n)+)', text)
    if not m:
        raise SystemExit('%s: no %s in FireRed' % (gym, label))
    lines = [l.strip() for l in m.group(1).strip().split('\n')]
    return '\n'.join('\t' + l.replace('POKéMON', 'MOéMON') for l in lines)


def main():
    for gym, (tm, label) in GYMS.items():
        path = os.path.join(REPO, 'data/maps/Kanto_%s/scripts.inc' % gym)
        s = open(path).read()
        if 'Moemon Kai: FireRed\'s TM' in s:
            print(gym, 'done already')
            continue
        m = re.search(r'\tcheckitemspace (ITEM_\w+), 1\n\tgoto_if_eq VAR_RESULT, 0, (\w+)\n(?:.*\n)*?\tsetflag (\w+)\n'
                      r'\tmsgbox (\w+), MSGBOX_DEFAULT\n', s)
        if not m:
            raise SystemExit('%s: no gift block' % gym)
        noroom, flag, explain = m.group(2), m.group(3), m.group(4)
        block = ('\t@ Moemon Kai: FireRed\'s TM from the leader (Quetzal gave %s: tools/quetzal_port/fix_leader_tms.py)\n'
                 '\tcheckitemspace %s, 1\n\tgoto_if_eq VAR_RESULT, 0, %s\n\tgiveitem %s\n\tsetflag %s\n'
                 '\tmsgbox %s, MSGBOX_DEFAULT\n') % (m.group(1), tm, noroom, tm, flag, explain)
        s = s[:m.start()] + block + s[m.end():]
        t = re.search(re.escape(explain) + r'::\n((?:\s*\.string .*\n)+)', s)
        s = s[:t.start(1)] + frlg_text(gym, label) + '\n' + s[t.end(1):]
        open(path, 'w').write(s)
        print(gym, tm)


if __name__ == '__main__':
    main()
