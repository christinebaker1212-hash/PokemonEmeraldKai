from common import *
from species import *
ids=species_ids(); inv={}
for k,v in ids.items():
    if k.startswith('SPECIES_') and k not in ('SPECIES_SHINY_TAG',) and v not in inv: inv[v]=k
def replace_body(path,sym,body):
    f=ROOT+'/'+path; t=open(f,encoding='utf-8-sig').read()
    m=re.search(r'\b'+sym+r'\[[^\]]*\]\s*=\s*\{',t); assert m,sym
    i=m.end(); depth=1
    while depth:
        c=t[i]; depth+= (c=='{')-(c=='}'); i+=1
    t=t[:m.end()]+'\n'+body+'\n'+t[i-1:]
    open(f,'w',encoding='utf-8').write(t)
def coords(sym,path):
    a,s=BYNAME[sym]; n=s//4
    body='\n'.join(f'    [{inv[i]}] = {{ .size = 0x{K[a+i*4]:02X}, .y_offset = {K[a+i*4+1]} }},' for i in range(n))
    replace_body(path,sym,body)
coords('gMonFrontPicCoords','src/data/pokemon_graphics/front_pic_coordinates.h')
coords('gMonBackPicCoords','src/data/pokemon_graphics/back_pic_coordinates.h')
def u8tab(sym,path,offset=0,names=True):
    a,s=BYNAME[sym]
    if names: body='\n'.join(f'    [{inv[i+offset]}{" - %d"%offset if offset else ""}] = {K[a+i]},' for i in range(s))
    else: body='\n'.join(f'    {K[a+i]},' for i in range(s))
    replace_body(path,sym,body)
u8tab('gEnemyMonElevation','src/data/pokemon_graphics/enemy_mon_elevation.h')
u8tab('sMonFrontAnimIdsTable','src/pokemon.c',offset=1)
u8tab('sSpeciesToBackAnimSet','src/pokemon_animation.c')
u8tab('sBackAnimationIds','src/pokemon_animation.c',names=False)
# front frame anims: Kai points every species at sAnims_None = {GeneralFrame0, GeneralFrame0}
a,s=BYNAME['gMonFrontAnimsPtrTable']
assert all(u32(K,a+i*4)==BYNAME['sAnims_None'][0]+0x8000000 for i in range(s//4))
replace_body('src/data/pokemon_graphics/front_pic_anims.h','gMonFrontAnimsPtrTable',
  '    // Moemon Kai: all species use a static front frame\n'+'\n'.join(f'    [{inv[i]}] = sAnims_None,' for i in range(s//4)))
f=ROOT+'/src/data/pokemon_graphics/front_pic_anims.h'; t=open(f,encoding='utf-8').read()
t2=re.sub(r'(sAnims_None\[\]\s*=\s*\{\s*sAnim_GeneralFrame0,\s*)sAnim_None_1',r'\1sAnim_GeneralFrame0',t)
assert t2!=t; open(f,'w',encoding='utf-8').write(t2)
print('ok')
