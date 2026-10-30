#!/usr/bin/env python3
"""Download the printed Yu-Gi-Oh! cards for tools/ygo/build_printed.py: tools/ygo/metadata/images/<password>.jpg.

usage: tools/ygo/fetch_art.py

From YGOPRODeck's card images (each card's default printing, 813x1185), at most a few requests a second as their
API asks; images already there are skipped. The English card back (CARD BACKS: PRINTED), a 927x1353 scan, comes from
Yugipedia: tools/ygo/metadata/back.png. The folder is ignored by git.
"""
import json
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'tools/ygo/metadata/images'
BACK_URL = 'https://ms.yugipedia.com/e/e5/Back-EN.png'
OUT.mkdir(parents=True, exist_ok=True)
cards = json.load(open(ROOT / 'tools/ygo/catalog.json'))['cards']
db = {c['id']: c for c in json.load(open(ROOT / 'tools/ygo/metadata/cardinfo.json'))['data']}
missing = 0
for c in cards:
    path = OUT / ('%d.jpg' % c['password'])
    if path.exists() and path.stat().st_size > 1000:
        continue
    info = db.get(c['password'])
    url = info['card_images'][0]['image_url'] if info else \
        'https://images.ygoprodeck.com/images/cards/%d.jpg' % c['password']
    r = subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--max-time', '60', '--retry', '2',
                        url, '-o', str(path) + '.tmp'])
    if r.returncode:
        missing += 1
        continue
    Path(str(path) + '.tmp').replace(path)
    time.sleep(0.15)
back = ROOT / 'tools/ygo/metadata/back.png'
if not back.exists():
    r = subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--max-time', '60', '--retry', '2',
                        BACK_URL, '-o', str(back) + '.tmp'])
    if r.returncode or not Path(str(back) + '.tmp').read_bytes().startswith(b'\x89PNG'):
        raise SystemExit('The card back did not download')
    Path(str(back) + '.tmp').replace(back)
print('%d cards, %d not downloaded' % (len(cards), missing))
