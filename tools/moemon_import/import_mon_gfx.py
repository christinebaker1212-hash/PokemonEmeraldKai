from common import *
from species import *
import collections
TABLES=[('gMonFrontPicTable','src/data/pokemon_graphics/front_pic_table.h','SPECIES_SPRITE'),
        ('gMonBackPicTable','src/data/pokemon_graphics/back_pic_table.h','SPECIES_SPRITE'),
        ('gMonPaletteTable','src/data/pokemon_graphics/palette_table.h','SPECIES_PAL'),
        ('gMonShinyPaletteTable','src/data/pokemon_graphics/shiny_palette_table.h','SPECIES_SHINY_PAL')]
ids=species_ids(); inv={v:k[8:] for k,v in ids.items() if k.startswith('SPECIES_') and k!='SPECIES_SHINY_TAG'}
stats=collections.Counter(); extra=[]
for tname,path,macro in TABLES:
    base=BYNAME[tname][0]; syms=table_syms(path,macro)
    per=collections.defaultdict(list)
    for i,sym in syms.items():
        pk=u32(K,base+i*8)-0x8000000; pv=u32(V,base+i*8)-0x8000000
        bk=K[pk:pk+lz_len(K,pk)[0]]; bv=V[pv:pv+lz_len(V,pv)[0]]
        per[sym].append((i,bk,bv))
    tabtext=open(ROOT+'/'+path).read()
    for sym,ents in per.items():
        blobs={e[1] for e in ents}
        if all(e[1]==e[2] for e in ents): stats['unchanged']+=1; continue
        if len(blobs)==1:
            rel=write_blob(sym,ents[0][1]); replace_def_with_incbin(sym,rel); stats['replaced']+=1
        else:
            # split: first blob keeps symbol, others get new symbols
            first=ents[0][1]
            rel=write_blob(sym,first); replace_def_with_incbin(sym,rel)
            for i,bk,bv in ents:
                if bk==first: continue
                ns=f'{sym}_Kai{i}'; rel=write_blob(ns,bk)
                extra.append(f'const u32 {ns}[] = INCBIN_U32("{rel}");')
                sp=inv[i]
                tabtext,n=re.subn(macro+r'\(\s*'+sp+r'\s*,\s*'+sym+r'\s*\)',f'{macro}({sp}, {ns})',tabtext)
                assert n==1,(sp,sym); stats['split']+=1
    open(ROOT+'/'+path,'w').write(tabtext)
if extra:
    f=ROOT+'/src/data/graphics/pokemon.h'
    t=open(f).read()+'\n// Moemon Kai: species whose vanilla graphics were shared but are distinct in Kai\n'+'\n'.join(sorted(set(extra)))+'\n'
    open(f,'w').write(t)
print(stats)
