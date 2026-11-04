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


def words(mapname, label, our_label=None):
    """`OUR_LABEL::` and its .string lines"""
    text = asm(mapname)
    text = text.replace('<PLAYER>', '{PLAYER}').replace('<PLAY_G>', '{PLAYER}').replace('<RIVAL>', '{RIVAL}').replace('<PK><MN>', '#MON')
    # a name put in by the script (text_ram): the buffers as this game's string vars
    rams = {'wStringBuffer3': '{STR_VAR_1}', 'wStringBuffer4': '{STR_VAR_2}', 'wStringBuffer5': '{STR_VAR_3}',
            'wPlayerName': '{PLAYER}', 'wRivalName': '{RIVAL}'}
    text = re.sub(r'@"\s*\n\s*text_ram (\w+)\s*\n\s*text "',
                  lambda m: rams.get(m.group(1), '{STR_VAR_1}'), text)
    pars = crystal_text(text, label)
    body = gba_text(pars)
    return ('%s::\n' % our_label if our_label else '') + body


if __name__ == '__main__':
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    sys.stdout.write(words(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else sys.argv[2]))
