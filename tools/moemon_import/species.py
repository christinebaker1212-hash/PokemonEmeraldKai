import re
from common import ROOT
def species_ids():
    d={}
    for l in open(ROOT+'/include/constants/species.h'):
        m=re.match(r'#define\s+(SPECIES_\w+)\s+(.+?)\s*(//.*)?$',l)
        if m:
            e=m.group(2)
            e=re.sub(r'SPECIES_\w+|NUM_SPECIES',lambda x:str(d[x.group(0) if x.group(0)!='NUM_SPECIES' else 'NUM_SPECIES']),e) if True else e
            try: d[m.group(1)]=eval(e)
            except Exception as ex: pass
        m=re.match(r'#define\s+(NUM_SPECIES)\s+(\w+)',l)
        if m: d['NUM_SPECIES']=d[m.group(2)]
    return d
def table_syms(path,macro):
    ids=species_ids(); out={}
    for m in re.finditer(macro+r'\(\s*(\w+)\s*,\s*(\w+)\s*\)',open(ROOT+'/'+path).read()):
        out[ids['SPECIES_'+m.group(1)]]=m.group(2)
    return out
