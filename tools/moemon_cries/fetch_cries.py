#!/usr/bin/env python3
"""Downloads the newest cry of every POKeMON from Gen 4 on that the game has (the PokeAPI cries collection, gathered
from Showdown, Veekun and The Sounds Resource: https://github.com/PokeAPI/cries, cries/pokemon/latest/<id>.ogg).

    python3 tools/moemon_cries/fetch_cries.py CACHE_DIR

Writes CACHE_DIR/<cry name>.ogg (the names of sound/direct_sound_samples/cries/<name>.wav) and CACHE_DIR/map.json
(cry name -> PokeAPI name and id). Then tools/moemon_cries/gba_cries.py makes them GBA cries.
"""
import json
import os
import re
import sys
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
API = 'https://pokeapi.co/api/v2/pokemon?limit=2000'
OGG = 'https://raw.githubusercontent.com/PokeAPI/cries/main/cries/pokemon/latest/%d.ogg'
# PokeAPI names a default form where the game's cry file has the bare species name
DEFAULT_FORM = {
    'wormadam': 'wormadam-plant', 'giratina': 'giratina-altered', 'shaymin': 'shaymin-land',
    'basculin': 'basculin-red-striped', 'darmanitan': 'darmanitan-standard', 'tornadus': 'tornadus-incarnate',
    'thundurus': 'thundurus-incarnate', 'landorus': 'landorus-incarnate', 'keldeo': 'keldeo-ordinary',
    'meloetta': 'meloetta-aria', 'meowstic': 'meowstic-male', 'aegislash': 'aegislash-shield',
    'pumpkaboo': 'pumpkaboo-average', 'gourgeist': 'gourgeist-average', 'zygarde': 'zygarde-50',
    'oricorio': 'oricorio-baile', 'lycanroc': 'lycanroc-midday', 'wishiwashi': 'wishiwashi-solo',
    'minior': 'minior-red-meteor', 'mimikyu': 'mimikyu-disguised', 'toxtricity': 'toxtricity-amped',
    'eiscue': 'eiscue-ice', 'indeedee': 'indeedee-male', 'morpeko': 'morpeko-full-belly',
    'urshifu': 'urshifu-single-strike', 'basculegion': 'basculegion-male', 'enamorus': 'enamorus-incarnate',
    'oinkologne': 'oinkologne-male', 'maushold': 'maushold-family-of-four',
    'squawkabilly': 'squawkabilly-green-plumage', 'palafin': 'palafin-zero', 'tatsugiri': 'tatsugiri-curly',
    'dudunsparce': 'dudunsparce-two-segment', 'deoxys': 'deoxys-normal', 'frillish': 'frillish-male',
    'jellicent': 'jellicent-male', 'pyroar': 'pyroar-male',
}


def fetch(url):
    req = urllib.request.Request(url, headers={'User-Agent': 'moemon-cries/1.0'})
    for attempt in range(5):
        try:
            return urllib.request.urlopen(req, timeout=60).read()
        except Exception:
            if attempt == 4:
                raise
            time.sleep(2 << attempt)


def game_cries():
    """The cry names the Gen 4-9 cry tables use, in table order"""
    files = {}
    for f in ('sound/direct_sound_data_gen4.inc', 'sound/direct_sound_data_gen59.inc'):
        cur = None
        for line in open(os.path.join(ROOT, f)):
            m = re.match(r'(Cry_\w+)::', line)
            if m:
                cur = m.group(1)
            m = re.search(r'cries/(\w+)\.bin', line)
            if m and cur:
                files[cur] = m.group(1)
                cur = None
    out = []
    for f in ('sound/cry_tables_gen4.inc', 'sound/cry_tables_gen59.inc'):
        for line in open(os.path.join(ROOT, f)):
            m = re.match(r'\s*cry (Cry_\w+)', line)
            if m and m.group(1) in files and files[m.group(1)] not in out:
                out.append(files[m.group(1)])
    return out


def main():
    cache = sys.argv[1]
    os.makedirs(cache, exist_ok=True)
    api = {r['name']: int(r['url'].rstrip('/').split('/')[-1])
           for r in json.loads(fetch(API))['results']}
    found, missing = {}, []
    for name in game_cries():
        for c in (name.replace('_', '-'), DEFAULT_FORM.get(name, ''), name.split('_')[0],
                  DEFAULT_FORM.get(name.split('_')[0], '')):
            if c in api:
                found[name] = (c, api[c])
                break
        else:
            missing.append(name)

    def get(item):
        name, (_, pid) = item
        path = os.path.join(cache, name + '.ogg')
        if not os.path.exists(path) or os.path.getsize(path) < 100:
            data = fetch(OGG % pid)
            open(path, 'wb').write(data)
        return name

    with ThreadPoolExecutor(4) as pool:
        for _ in pool.map(get, found.items()):
            pass
    json.dump(found, open(os.path.join(cache, 'map.json'), 'w'), indent=1, sort_keys=True)
    print('%d cries fetched, %d without one: %s' % (len(found), len(missing), ' '.join(missing)))


if __name__ == '__main__':
    main()
