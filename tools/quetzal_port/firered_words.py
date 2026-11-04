#!/usr/bin/env python3
"""FireRed's own words as this game's text (docs/REGIONS_AUDIT.md: KANTO restored to FireRed).

    python3 tools/quetzal_port/firered_words.py MAP LABEL [OUR_LABEL]

Prints FireRed's text LABEL from pret/pokefirered data/maps/MAP/text.inc (or data/text/*.inc when MAP is a path
under data/, e.g. data/text/trainers.inc) as `.string` lines, POKéMON as MOéMON and ' as ’, as the rest of KANTO's
text has them. The files come from POKEFIRERED_DIR (default /tmp/claude-0/frlg/pokefirered) or GitHub. Used by hand
and by the audit's fix scripts (import `words`).
"""
import os
import re
import sys
import urllib.request

LOCAL = os.environ.get('POKEFIRERED_DIR', '/tmp/claude-0/frlg/pokefirered')
RAW = 'https://raw.githubusercontent.com/pret/pokefirered/master/%s'
_cache = {}


def _path(mapname):
    return mapname if mapname.startswith('data/') else 'data/maps/%s/text.inc' % mapname


def source(mapname):
    rel = _path(mapname)
    if rel not in _cache:
        local = os.path.join(LOCAL, rel)
        if os.path.exists(local):
            _cache[rel] = open(local, encoding='utf-8').read()
        else:
            _cache[rel] = urllib.request.urlopen(RAW % rel).read().decode('utf-8')
    return _cache[rel]


def lines(mapname, label):
    """the label's .string lines as FireRed has them"""
    text = source(mapname)
    m = re.search(r'^%s::?\s*$' % re.escape(label), text, re.M)
    if not m:
        raise SystemExit('%s: no %s' % (mapname, label))
    out = []
    for line in text[m.end():].split('\n'):
        s = line.strip()
        if not s:
            if out:
                break
            continue
        if not s.startswith('.string'):
            break
        out.append(s)
        if s.endswith('$"'):
            break
    return out


def kai(s):
    return s.replace('POKéMON', 'MOéMON').replace('POKéDEX', 'MOéDEX').replace("'", '’')


def words(mapname, label, our_label=None):
    """`OUR_LABEL::` and its .string lines"""
    body = ''.join('\t%s\n' % kai(s) for s in lines(mapname, label))
    return ('%s::\n' % our_label if our_label else '') + body


if __name__ == '__main__':
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    sys.stdout.write(words(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else sys.argv[2]))
