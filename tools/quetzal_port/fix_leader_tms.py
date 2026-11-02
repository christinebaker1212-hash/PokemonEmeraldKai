#!/usr/bin/env python3
"""KANTO's and JOHTO's gym leaders give their games' TMs again (Quetzal gave held items: LEFTOVERS, CHOICE SCARF...).

    python3 tools/quetzal_port/fix_leader_tms.py      (run from the repository; safe to run again)

JOHTO's are HeartGold / SoulSilver's (Crystal's TM31 MUD-SLAP, TM49 FURY CUTTER... aren't TMs here; HGSS's all are),
with HGSS's own words (pret/pokeheartgold files/msgdata/msg: the leader's line about the TM), rewrapped for the
message box. CLAIR's is the DRAGON SHRINE's gift (Quetzal's DRAGON SCALE).

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


JOHTO = {   # map: (Quetzal's gift item, TM, HGSS message file, message, paragraphs kept)
    'VioletGym': ('ITEM_LEFTOVERS', 'ITEM_TM51', 'msg_0558_T22GYM0101', 4, 3),
    'AzaleaGym': ('ITEM_NUGGET', 'ITEM_TM89', 'msg_0567_T23GYM0102', 5, 9),
    'GoldenrodGym': ('ITEM_CHOICE_SCARF', 'ITEM_TM45', 'msg_0582_T25GYM0101', 7, 2),
    'EcruteakGym': ('ITEM_LIFE_ORB', 'ITEM_TM30', 'msg_0614_T27GYM0101', 5, 9),
    'CianwoodGym': ('ITEM_LIFE_ORB', 'ITEM_TM01', 'msg_0574_T24GYM0101', 6, 9),
    'OlivineGym': ('ITEM_LEFTOVERS', 'ITEM_TM23', 'msg_0606_T26GYM0101', 5, 9),
    'MahoganyGym': ('ITEM_FOCUS_SASH', 'ITEM_TM07', 'msg_0622_T28GYM0101', 4, 9),
    'DragonShrine': ('ITEM_DRAGON_SCALE', 'ITEM_TM59', 'msg_0631_T30GYM0101', 7, 9),
}
HGSS = 'https://raw.githubusercontent.com/pret/pokeheartgold/master/files/msgdata/msg/%s.json'
CAPS = ['Roost', 'U-turn', 'Attract', 'Shadow Ball', 'Focus Punch', 'Iron Tail', 'Hail', 'Dragon Pulse',
        'Special Defense', 'Defense']
WIDTH = 34


def wrap(par):
    lines, cur = [], ''
    for w in par.split():
        if cur and len(cur) + 1 + len(w) > WIDTH:
            lines.append(cur)
            cur = w
        else:
            cur = (cur + ' ' + w) if cur else w
    if cur:
        lines.append(cur)
    return lines


def hgss_text(name, index, keep):
    import json
    j = json.loads(urllib.request.urlopen(HGSS % name).read().decode('utf-8'))
    m = j.get('messages', j)[index]
    t = m.get('en_US') if isinstance(m, dict) else m
    t = ''.join(t) if isinstance(t, list) else t
    t = re.sub(r'^[A-Z][a-z]+: ', '', t)   # (the speaker's name: JOHTO's words don't carry it)
    for c in CAPS:
        t = t.replace(c, c.upper())
    t = t.replace('Pokémon', 'MOéMON')
    pars = [p.replace('\n', ' ').replace('\x0c', ' ') for p in t.replace('\x0c', ' ').split('\r') if p.strip()][:keep]
    out = []
    for p in pars:
        ls = wrap(p.replace('\n', ' '))
        for k, l in enumerate(ls):
            last = k == len(ls) - 1
            sep = '' if last else ('\\n' if k == 0 else '\\l')
            out.append((l, sep))
        out[-1] = (out[-1][0], '\\p')
    out[-1] = (out[-1][0], '$')
    return '\n'.join('\t.string "%s%s"' % (l, sep) for l, sep in out)


def johto():
    for place, (old, tm, name, index, keep) in JOHTO.items():
        path = os.path.join(REPO, 'data/maps/Johto_%s/scripts.inc' % place)
        s = open(path).read()
        if 'Moemon Kai: HeartGold / SoulSilver' in s:
            print(place, 'done already')
            continue
        m = re.search(r'\tsetorcopyvar VAR_0x8000, %s\n\tsetorcopyvar VAR_0x8001, 1\n\tcallstd STD_OBTAIN_ITEM\n'
                      r'\tgoto_if_eq VAR_RESULT, 0, \w+\n(?:\t(?:setflag|clearflag) \w+\n)\tmsgbox (\w+), MSGBOX_DEFAULT\n' % old, s)
        if not m:
            raise SystemExit('%s: no gift block' % place)
        block = m.group(0).replace('setorcopyvar VAR_0x8000, %s' % old,
                                   '@ Moemon Kai: HeartGold / SoulSilver\'s TM from the leader (Quetzal gave %s)\n'
                                   '\tsetorcopyvar VAR_0x8000, %s' % (old, tm))
        s = s[:m.start()] + block + s[m.end():]
        explain = m.group(1)
        t = re.search(re.escape(explain) + r'::\n((?:\s*\.string .*\n)+)', s)
        s = s[:t.start(1)] + hgss_text(name, index, keep) + '\n' + s[t.end(1):]
        open(path, 'w').write(s)
        print(place, tm)


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
    johto()
