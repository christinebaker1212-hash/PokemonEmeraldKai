#!/usr/bin/env python3
"""KANTO's move tutors as FireRed's (Kai > FireRed > Quetzal; the user, 2026-11-02): Quetzal replaced each with a
person saying "Move training is handled directly from your MOéMON team menu." (Quetzal_Text_8471CE4). FireRed's tutor
stands at the same place (its map.json); its script (pokefirered data/scripts/move_tutors.inc) is rebuilt on Kai's
tutor flow (MoveTutor_EventScript_CanOnlyBeLearnedOnce / OpenPartyMenu, TUTOR_MOVE_*), its words from FireRed's text
files (POKéMON as MOéMON), a "taught" flag each from FLAGS (free in Quetzal's KANTO flags). CAPE BRINK's starter tutor
(FireRed's own system) keeps Quetzal's stand-in. Run once; its edits are committed.

    python3 -I tools/quetzal_port/fix_move_tutors.py
"""
import json
import os
import re
import subprocess
import urllib.request

RAW = 'https://raw.githubusercontent.com/pret/pokefirered/master/'
CACHE = '/tmp/claude-0/frlg'
STAND_IN = 'Quetzal_Text_8471CE4'
FLAGS = {'MEGA_PUNCH': 0x21DF, 'MEGA_KICK': 0x21DC, 'DOUBLE_EDGE': 0x210D, 'THUNDER_WAVE': 0x210E,
         'ROCK_SLIDE': 0x21EE, 'EXPLOSION': 0x21EF, 'DREAM_EATER': 0x21F0, 'SOFTBOILED': 0x220E, 'SOFT_BOILED': 0x220E, 'SUBSTITUTE': 0x2212,
         'SWORDS_DANCE': 0x2217, 'SEISMIC_TOSS': 0x221B, 'COUNTER': 0x221C, 'METRONOME': 0x2220, 'MIMIC': 0x2221,
         'BODY_SLAM': 0x2222}


def fetch(path):
    local = os.path.join(CACHE, path.replace('/', '__'))
    if not os.path.exists(local):
        os.makedirs(CACHE, exist_ok=True)
        try:
            data = urllib.request.urlopen(RAW + path, timeout=30).read()
        except Exception:
            data = subprocess.run(['curl', '-sS', '-m', '30', RAW + path], capture_output=True).stdout
        open(local, 'wb').write(data)
    return open(local, encoding='utf-8').read()


def text_of(label, mapname):
    """FireRed's text LABEL: its map's text.inc, else data/text/*.inc found by code search"""
    for path in ('data/maps/%s/text.inc' % mapname, 'data/text/move_tutors.inc'):
        t = fetch(path)
        m = re.search(r'^%s::\n((?:\s*\.string "[^"]*"\n)+)' % re.escape(label), t, re.M)
        if m:
            break
    else:
        out = subprocess.run(['gh', 'api', '-X', 'GET', 'search/code', '-f', 'q=%s repo:pret/pokefirered' % label,
                              '--jq', '.items[].path'], capture_output=True, text=True).stdout.split()
        for path in out:
            t = fetch(path)
            m = re.search(r'^%s::\n((?:\s*\.string "[^"]*"\n)+)' % re.escape(label), t, re.M)
            if m:
                break
        else:
            raise SystemExit('no text %s' % label)
    lines = re.findall(r'\.string "([^"]*)"', m.group(1))
    return ''.join('\t.string "%s"\n' % l.replace('POKéMON', 'MOéMON').replace('POKéDEX', 'MOéDEX') for l in lines)


def main():
    tutors = fetch('data/scripts/move_tutors.inc')
    by_label = {m.group(1): m.group(2) for m in re.finditer(r'^(\w+Tutor)::\n(.*?)(?=^\w+Tutor::|\Z)', tutors, re.M | re.S)}
    for path in sorted(os.listdir('data/maps')):
        if not path.startswith('Kanto_'):
            continue
        sp = 'data/maps/%s/scripts.inc' % path
        s = open(sp, encoding='utf-8').read()
        labels = re.findall(r'^(\w+)::\n\tmsgbox %s, MSGBOX_NPC\n\tend\n' % STAND_IN, s, re.M)
        if not labels:
            continue
        kai = json.load(open('data/maps/%s/map.json' % path))
        fr_name = path[len('Kanto_'):]
        fr = json.loads(fetch('data/maps/%s/map.json' % fr_name))
        for lab in labels:
            alias = re.findall(r'^(\w+)::\n\tgoto %s\n' % lab, s, re.M)   # (an object's script going to it)
            obj = [o for o in kai['object_events'] if o['script'] == lab or o['script'] in alias]
            if not obj:
                continue
            o = obj[0]
            fro = [f for f in fr['object_events'] if f.get('x') == o['x'] and f.get('y') == o['y']]
            tut = fro[0].get('script') if fro else None
            if tut and tut not in by_label:   # (FireRed's map script going to the shared tutor)
                frs = fetch('data/maps/%s/scripts.inc' % fr_name)
                i = frs.find('%s::\n' % tut)
                block = frs[i:frs.find('\n\n', i)] if i >= 0 else ''
                m = re.search(r'\tgoto (\w+Tutor)\b', block)
                tut = m.group(1) if m else tut
            if tut not in by_label or 'Starter' in tut:
                print('%s %s: keeps the stand-in (%s)' % (path, lab, tut))
                continue
            body = by_label[tut]
            move = re.search(r'setvar VAR_0x8005, MOVETUTOR_(\w+)', body).group(1)
            name = re.search(r'msgbox (Text_(\w+)Teach)', body).group(2)
            kmove = {'SOFTBOILED': 'SOFT_BOILED'}.get(move, move)
            flag = FLAGS[move]
            new = ('''{lab}::
	@ Moemon Kai: FireRed's {up} tutor (tools/quetzal_port/fix_move_tutors.py), on Kai's move tutors
	lock
	faceplayer
	goto_if_set 0x{flag:X}, {lab}_Taught
	msgbox {lab}_Teach, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, {lab}_Declined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	goto_if_eq VAR_RESULT, NO, {lab}_Declined
	msgbox {lab}_WhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_{kmove}
	call MoveTutor_EventScript_OpenPartyMenu
	goto_if_eq VAR_RESULT, 0, {lab}_Declined
	setflag 0x{flag:X}
	goto {lab}_Taught
	end

{lab}_Declined:
	msgbox {lab}_DeclinedText, MSGBOX_DEFAULT
	release
	end

{lab}_Taught:
	msgbox {lab}_TaughtText, MSGBOX_DEFAULT
	release
	end
''').format(lab=lab, up=move.replace('_', ' '), flag=flag, kmove=kmove)
            for k, kl in (('Teach', 'Teach'), ('Declined', 'DeclinedText'), ('WhichMon', 'WhichMon'), ('Taught', 'TaughtText')):
                new += '\n%s_%s:\n%s' % (lab, kl, text_of('Text_%s%s' % (name, k), fr_name))
            old = '%s::\n\tmsgbox %s, MSGBOX_NPC\n\tend\n' % (lab, STAND_IN)
            s = s.replace(old, new)
            print('%s %s: %s (0x%X)' % (path, lab, move, flag))
        open(sp, 'w', encoding='utf-8').write(s)


if __name__ == '__main__':
    main()
