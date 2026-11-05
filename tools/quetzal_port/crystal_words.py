#!/usr/bin/env python3
"""Crystal's own words as this game's text (docs/REGIONS_AUDIT.md: JOHTO restored to Crystal).

    python3 tools/quetzal_port/crystal_words.py MAP LABEL [OUR_LABEL]

Prints Crystal's text LABEL from pret/pokecrystal maps/MAP.asm (or MAP.asm when MAP is a path, e.g. data/text/common_3) as `.string` lines (re-wrapped to this game's width,
#MON as MOéMON, <PLAYER> / <RIVAL> as {PLAYER} / {RIVAL}). The asm comes from POKECRYSTAL_DIR (default
/tmp/claude-0/frlg/pokecrystal) or GitHub. Used by hand and by the audit's fix scripts (import `words`).
"""
import os
import re
import sys
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fix_johto_tms import crystal_text, gba_text   # noqa: E402

LOCAL = os.environ.get('POKECRYSTAL_DIR', '/tmp/claude-0/frlg/pokecrystal')
RAW = 'https://raw.githubusercontent.com/pret/pokecrystal/master/%s.asm'
_cache = {}


def asm(mapname):
    if mapname not in _cache:
        # a map's name, or a path such as data/text/common_3 (shared texts)
        rel = mapname if '/' in mapname else 'maps/' + mapname
        path = os.path.join(LOCAL, rel + '.asm')
        if os.path.exists(path):
            _cache[mapname] = open(path).read()
        else:
            _cache[mapname] = urllib.request.urlopen(RAW % rel).read().decode('utf-8')
    return _cache[mapname]


_tm_numbers = None


def renumber_tms(par):
    """Crystal's "TMnn" as this game's number for the same move (T11b: Crystal's TMs whose moves are on another number
    here, tools/moemon_tms/gen_tm3.py; the words stay Crystal's), and DYNAMICPUNCH whole across a line break"""
    global _tm_numbers
    if _tm_numbers is None:
        repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
        crys = [m for m in re.findall(r'^\s*add_tm (\w+)', asm('constants/item_constants'), re.M)]
        alias = {'DYNAMICPUNCH': 'DYNAMIC_PUNCH', 'DRAGONBREATH': 'DRAGON_BREATH', 'THUNDERPUNCH': 'THUNDER_PUNCH',
                 'PSYCHIC_M': 'PSYCHIC', 'SOLARBEAM': 'SOLAR_BEAM'}
        hdr = open(os.path.join(repo, 'include/constants/tms_hms.h')).read()
        ours = re.findall(r'F\((\w+)\)', hdr.split('FOREACH_HM')[0])
        tm2 = re.findall(r'F\((\w+)\)', open(os.path.join(repo, 'include/constants/tms_hms_tm2.h')).read())
        tm3 = re.findall(r'F\((\w+)\)', open(os.path.join(repo, 'include/constants/tms_hms_tm3.h')).read())
        where = {m: k + 1 for k, m in enumerate(ours)}
        where.update({m: k + 51 for k, m in enumerate(tm2)})
        where.update({m: k + 93 for k, m in enumerate(tm3)})
        _tm_numbers = {}
        for k, m in enumerate(crys):
            m = alias.get(m, m)
            if m in where and where[m] != k + 1:
                _tm_numbers[k + 1] = where[m]
    par = re.sub(r'\bTM0?(\d+)\b', lambda x: 'TM%02d' % _tm_numbers.get(int(x.group(1)), int(x.group(1)))
                 if int(x.group(1)) < 10 and int(x.group(1)) not in _tm_numbers else 'TM%d' % _tm_numbers.get(int(x.group(1)), int(x.group(1))), par)
    return par.replace('DYNAMIC- PUNCH', 'DYNAMICPUNCH')


def words(mapname, label, our_label=None):
    """`OUR_LABEL::` and its .string lines"""
    text = asm(mapname)
    text = text.replace('<PLAYER>', '{PLAYER}').replace('<PLAY_G>', '{PLAYER}').replace('<RIVAL>', '{RIVAL}').replace('<PK><MN>', '#MON').replace('<……>', '……')
    # a name put in by the script (text_ram): the buffers as this game's string vars
    rams = {'wStringBuffer3': '{STR_VAR_1}', 'wStringBuffer4': '{STR_VAR_2}', 'wStringBuffer5': '{STR_VAR_3}',
            'wPlayerName': '{PLAYER}', 'wRivalName': '{RIVAL}',
            # (the POKé SEER's: qz_specials.c MP_PokeSeer fills them)
            'wSeerNickname': '{STR_VAR_1}', 'wSeerCaughtLocation': '{STR_VAR_2}', 'wSeerCaughtLevelString': '{STR_VAR_3}',
            'wSeerOT': '{STR_VAR_3}'}
    text = re.sub(r'@"\s*\n\s*text_ram (\w+)\s*\n\s*text "',
                  lambda m: rams.get(m.group(1), '{STR_VAR_1}'), text)
    # (text_start: the buffer ends the line, the next line command goes on)
    text = re.sub(r'@"\s*\n\s*text_ram (\w+)\s*\n\s*text_start\s*\n',
                  lambda m: rams.get(m.group(1), '{STR_VAR_1}') + '"\n', text)
    # ({d:NAME}: a number the map DEFs, as the haircut prices)
    defs = dict(re.findall(r'^DEF (\w+)\s+EQU (\d+)', text, re.M))
    text = re.sub(r'\{d:(\w+)\}', lambda m: defs.get(m.group(1), m.group(0)), text)
    pars = [renumber_tms(p) for p in crystal_text(text, label)]
    body = gba_text(pars)
    return ('%s::\n' % our_label if our_label else '') + body


if __name__ == '__main__':
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    sys.stdout.write(words(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else sys.argv[2]))
