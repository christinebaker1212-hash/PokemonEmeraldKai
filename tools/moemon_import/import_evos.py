from common import *
from species import *
import struct
ids=species_ids(); inv={}
for k,v in ids.items():
    if k.startswith('SPECIES_') and v not in inv: inv[v]=k
items={}
for l in open(ROOT+'/include/constants/items.h'):
    m=re.match(r'#define\s+(ITEM_\w+)\s+(\d+)\s*$',l)
    if m and int(m.group(2)) not in items: items[int(m.group(2))]=m.group(1)
EVO={4:'EVO_LEVEL',5:'EVO_TRADE',6:'EVO_TRADE_ITEM',7:'EVO_ITEM',15:'EVO_BEAUTY'}
f=ROOT+'/src/data/pokemon/evolution.h'; t=open(f,encoding='utf-8').read()
a,s=BYNAME['gEvolutionTable']
for i in range(s//40):
    ev=lambda R:[struct.unpack_from('<HHH',R,a+i*40+j*8) for j in range(5)]
    if ev(V)==ev(K): continue
    parts=[]
    for m_,p,tg in ev(K):
        if not m_: continue
        ps=items.get(p,str(p)) if m_ in (6,7) else str(p)
        parts.append(f'{{{EVO[m_]}, {ps}, {inv[tg]}}}')
    new=f'[{inv[i]}]'+' '*max(1,len('SPECIES_BULBASAUR')+3-len(inv[i]))+'= {'+',\n                            '.join(parts)+'},'
    m=re.search(r'\['+inv[i]+r'\]\s*=\s*\{.*?\}\},',t,re.S); assert m,inv[i]
    t=t[:m.start()]+new+'  // Moemon Kai'+t[m.end():]
open(f,'w',encoding='utf-8').write(t); print('ok')
