#!/usr/bin/env python3
"""Import original, stable poketcg IDs/data without assembling or needing a ROM.
Usage: python3 tools/tcg/import_poketcg.py /path/to/poketcg
The generated JSON retains original effect commands; unsupported rules must never
be replaced by generic damage. Source revision is recorded for reproducibility.
"""
import argparse, json, re, shutil, subprocess, unicodedata
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
REVISION='2d81ec3cbff4ebb2fd3f1f770ba245227a702ead'
def plain(s):
    return unicodedata.normalize('NFKD',s.replace('♀',' F').replace('♂',' M')).encode('ascii','ignore').decode()
def read_blocks(s):
    matches=list(re.finditer(r'^(\w+)::?\s*$',s,re.M))
    return {m.group(1):s[m.end():matches[i+1].start() if i+1<len(matches) else len(s)] for i,m in enumerate(matches)}
def main():
    ap=argparse.ArgumentParser();ap.add_argument('source',type=Path);args=ap.parse_args();src=args.source
    revision=subprocess.check_output(['git','-C',str(src),'rev-parse','HEAD'],text=True).strip()
    if revision!=REVISION: raise SystemExit(f'Expected source {REVISION}; found {revision}')
    text_blocks={}
    for p in sorted((src/'src/text').glob('text*.asm')):text_blocks.update(read_blocks(p.read_text()))
    def text(label):
        if label in ['NONE','NULL']:return ''
        block=text_blocks.get(label,'')
        chunks=[]
        for line in block.splitlines():
            m=re.match(r'\s*(text|line|cont|para)\s+"(.*)"',line)
            if m:chunks.append(m.group(2))
        result = plain(' '.join(chunks)) if chunks else label
        return re.sub(r'<(FIRE|GRASS|LIGHTNING|WATER|FIGHTING|PSYCHIC|COLORLESS)>', lambda m: m.group(1).title(), result)
    data=(src/'src/data/cards.asm').read_text();blocks=read_blocks(data)
    pointer_block=data[:data.index('\nBulbasaurCard:')]
    pointers=re.findall(r'^\s*dw (\w+Card)\s*$',pointer_block,re.M)
    assert len(pointers)==228
    effect_blocks=read_blocks((src/'src/engine/duel/effect_commands.asm').read_text())
    effects={label:[{'phase':m[0],'function':m[1]} for m in re.findall(r'dbw\s+(\w+),\s*(\w+)',block)] for label,block in effect_blocks.items()}
    gfx=read_blocks((src/'src/gfx.asm').read_text())
    cards=[]
    for n,label in enumerate(pointers,1):
        entries=[]
        for line in blocks[label].splitlines():
            line=line.split(';')[0].strip();m=re.match(r'(db|dw|tx|gfx|energy)\s+(.*)',line)
            if m:entries.append(m.groups())
        values=[v for op,v in entries]
        assert len(values)>=9,(label,values)
        typ=values[0];kind='pokemon' if typ.startswith('TYPE_PKMN_') else 'trainer' if typ=='TYPE_TRAINER' else 'energy'
        card={'id':n,'symbol':values[5],'label':label,'name':text(values[2]),'kind':kind,'type':typ,'rarity':values[3],'sets':values[4]}
        if kind=='pokemon':
            assert len(values)==44,(label,len(values))
            card.update(hp=int(values[6]),stage=values[7],evolves_from=text(values[8]),retreat=int(values[33]),weakness=values[34],resistance=values[35],species=int(values[37]),level=int(values[39]),flavor=text(values[42]))
            card['attacks']=[]
            for k in [9,21]:
                cost=values[k].split(', ');costs={}
                if values[k]!='0':
                    assert len(cost)%2==0,(label,cost)
                    costs={cost[i].strip():int(cost[i+1]) for i in range(0,len(cost),2)}
                card['attacks'].append({'name':text(values[k+1]),'description':' '.join(filter(None,[text(values[k+2]),text(values[k+3])])),'energy':costs,'damage':int(values[k+4]),'category':values[k+5],'effect':values[k+6],'flags':values[k+7:k+10],'parameter':values[k+10],'animation':values[k+11]})
        else:card.update(effect=values[6],description=' '.join(filter(None,[text(values[7]),text(values[8])])))
        graphic=re.search(r'INCBIN "gfx/cards/(.+)\.2bpp"',gfx[values[1]])
        assert graphic,(label,values[1])
        art=ROOT/'graphics/tcg/original'/f'{n:03d}.png';art.parent.mkdir(parents=True,exist_ok=True)
        shutil.copyfile(src/'src/gfx/cards'/f'{graphic[1]}.png',art)
        card['original_art']=str(art.relative_to(ROOT));cards.append(card)
    symbols={c['symbol']:c['id'] for c in cards}
    deck_blocks=read_blocks((src/'src/data/decks.asm').read_text());decks=[]
    for label,block in deck_blocks.items():
        if 'deck_list_start' not in block:continue
        body=block.split('deck_list_start',1)[1].split('deck_list_end',1)[0]
        items=re.findall(r'card_item\s+(\w+),\s*(\d+)',body)
        if not items:continue
        deck=[symbols[name] for name,count in items for _ in range(int(count))]
        if len(deck)!=60:continue # unnamed scratch decks are not NPC deck definitions
        decks.append({'name':label.removesuffix('Deck'),'cards':deck})
    out={'source':'https://github.com/pret/poketcg','revision':revision,'cards':cards,'decks':decks,'effects':effects}
    p=ROOT/'tools/tcg/catalog.json';p.write_text(json.dumps(out,indent=2)+'\n')
    print(f'Imported {len(cards)} cards, {len(decks)} complete decks, {len(effects)} effect command lists at {revision}')
if __name__=='__main__':main()
