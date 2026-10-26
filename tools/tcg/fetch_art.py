#!/usr/bin/env python3
"""Match printed cards conservatively by name/HP/level/attacks, then cache scans.
Ambiguous and Game Boy-only prints remain explicit; never label a substitute an
exact original. Downloaded images and their hashes are recorded in the manifest.
"""
import concurrent.futures,hashlib,json,re,subprocess,unicodedata
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
CACHE=ROOT/'tools/tcg/metadata';CACHE.mkdir(exist_ok=True)
SETS=['base1','base2','base3','basep']
def fetch(url,path):
    if not path.exists():subprocess.run(['curl','--fail','--location','--silent','--show-error','--connect-timeout','15','--max-time','60','--retry','2',url,'-o',str(path)+'.tmp'],check=True);Path(str(path)+'.tmp').replace(path)
    return path
for s in SETS:fetch(f'https://raw.githubusercontent.com/PokemonTCG/pokemon-tcg-data/master/cards/en/{s}.json',CACHE/f'{s}.json')
physical=[c for s in SETS for c in json.loads((CACHE/f'{s}.json').read_text())]
def norm(s):return re.sub('[^a-z0-9]','',unicodedata.normalize('NFKD',s.replace('♀',' F').replace('♂',' M')).encode('ascii','ignore').decode().lower())
rows=[];tasks=[]
for c in json.loads((ROOT/'tools/tcg/catalog.json').read_text())['cards']:
    candidates=[p for p in physical if norm(p['name'])==norm(c['name'])]
    if c['kind']=='pokemon':
        candidates=[p for p in candidates if str(p.get('hp'))==str(c['hp']) and str(p.get('level'))==str(c['level']) and [norm(a['name']) for a in p.get('attacks',[])]==[norm(a['name']) for a in c['attacks'] if a['name'] and a['category']!='POKEMON_POWER']]
    # Original source flags distinguish Jungle, Fossil and promotional printings.
    subset='base2' if '| JUNGLE' in c['sets'] else 'base3' if '| FOSSIL' in c['sets'] else 'basep' if '| PRO' in c['sets'] else None
    if subset:
        narrowed=[p for p in candidates if p['id'].startswith(subset+'-')]
        if narrowed:candidates=narrowed
    row={'card_id':c['id'],'name':c['name'],'source_candidates':[p['id'] for p in candidates],'status':'needs_review' if len(candidates)>1 else 'no_print_match'}
    if len(candidates)==1:
        p=candidates[0];row.update(status='matched',printing=p['id'],url=p['images']['large'],path=f'graphics/tcg/printed/{c["id"]:03d}.png');tasks.append(row)
    rows.append(row)
(ROOT/'graphics/tcg/printed').mkdir(parents=True,exist_ok=True)
def image(row):
    path=fetch(row['url'],ROOT/row['path']);raw=path.read_bytes()
    if not raw.startswith(b'\x89PNG\r\n\x1a\n'):raise ValueError(f'Not a PNG: {path}')
    row['sha256']=hashlib.sha256(raw).hexdigest();row['bytes']=len(raw)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:list(pool.map(image,tasks))
(ROOT/'tools/tcg/art_manifest.json').write_text(json.dumps({'source':'https://github.com/PokemonTCG/pokemon-tcg-data','cards':rows},indent=2)+'\n')
print(f'{len(tasks)} matched scans downloaded; {sum(r["status"]=="needs_review" for r in rows)} ambiguous; {sum(r["status"]=="no_print_match" for r in rows)} unmatched/GB-exclusive')
