from common import *
from species import *
ids=species_ids(); inv={}
for k,v in ids.items():
    if k.startswith('SPECIES_') and v not in inv: inv[v]=k
f=ROOT+'/src/data/pokemon/species_info.h'; t=open(f,encoding='utf-8').read()
a,s=BYNAME['gSpeciesInfo']; n=0; miss=[]
def setfield(block,field,val):
    nb,c=re.subn(r'(\.'+field+r'\s*=\s*)[^,\n]+',lambda m:m.group(1)+val,block,count=1)
    if c==0: nb=block.rstrip()[:-1].rstrip()+f'\n        .{field} = {val},\n    }}'  # unlikely
    return nb
for i in range(s//28):
    o=a+i*28
    if V[o:o+28]==K[o:o+28]: continue
    sp=inv[i]; m=re.search(r'\['+sp+r'\]\s*=\s*\{',t)
    if not m: miss.append(sp); continue
    j=m.end(); d=1
    while d: d+=(t[j]=='{')-(t[j]=='}'); j+=1
    blk=t[m.start():j]; nb=blk
    def ch(off,ln=1): return V[o+off:o+off+ln]!=K[o+off:o+off+ln]
    if ch(0x0C,2): nb=setfield(nb,'itemCommon',str(int.from_bytes(K[o+0xC:o+0xE],'little')))
    if ch(0x0E,2): nb=setfield(nb,'itemRare',str(int.from_bytes(K[o+0xE:o+0x10],'little')))
    if ch(0x10): nb=setfield(nb,'genderRatio',str(K[o+0x10]))
    if ch(0x18): nb=setfield(nb,'safariZoneFleeRate',str(K[o+0x18]))
    if ch(0x19):
        assert (V[o+0x19]&0x7f)==(K[o+0x19]&0x7f)
        nb=setfield(nb,'noFlip','TRUE' if K[o+0x19]&0x80 else 'FALSE')
    other=[x for x in range(28) if V[o+x]!=K[o+x] and x not in (0xC,0xD,0xE,0xF,0x10,0x18,0x19)]
    assert not other,(sp,other)
    t=t[:m.start()]+nb+t[j:]; n+=1
open(f,'w',encoding='utf-8').write(t)
print('patched',n,'missing',miss)
