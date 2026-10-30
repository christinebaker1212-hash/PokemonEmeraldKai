#!/usr/bin/env python3
"""Download the card database tools/ygo/build_catalog.py reads: tools/ygo/metadata/cardinfo.json.

usage: tools/ygo/fetch_db.py

From YGOPRODeck's public API (https://db.ygoprodeck.com/api/v7/cardinfo.php?misc=yes): every card's current
English text (today's wording and errata), stats, and English set printings with their rarities. About 25 MB;
the folder is ignored by git. The catalog it feeds (tools/ygo/catalog.json) is committed, so building the game
never needs it.
"""
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'tools/ygo/metadata/cardinfo.json'
OUT.parent.mkdir(parents=True, exist_ok=True)
subprocess.run(['curl', '--fail', '--location', '--silent', '--show-error', '--max-time', '600',
                'https://db.ygoprodeck.com/api/v7/cardinfo.php?misc=yes', '-o', str(OUT) + '.tmp'], check=True)
Path(str(OUT) + '.tmp').replace(OUT)
print(OUT, OUT.stat().st_size, 'bytes')
