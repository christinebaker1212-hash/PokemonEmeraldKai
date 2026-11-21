import re, struct, zlib, sys
import numpy as np
import os
REPO=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
IDX=[tuple(map(int,m)) for m in re.findall(r'\{(\d+), (\d+), (\d+), (\d+), (\d+)\}', open(REPO+'/src/data/mp_pmd.h').read())]
BIN=open(REPO+'/graphics/moemon/pmd/pmd.bin','rb').read()
def chunk(dex, var=0):
    for d,v,o,s,r in IDX:
        if d==dex and v==var: return zlib.decompress(BIN[o:o+s], -15)
def frame(dex, anim=1, direction=0, fi=0, var=0):
    c=chunk(dex,var)
    if c is None: return None
    assert c[:4]==b'PMD2'
    bpp,colors,shadow,ac=c[4],c[5],c[6],c[7]
    p=8; pal=struct.unpack_from('<%dH'%colors,c,p); p+=colors*4
    p=(p+3)&~3
    aoff=struct.unpack_from('<%dI'%ac,c,p)
    if anim>=ac or aoff[anim]==0: anim=0
    a=aoff[anim]; nf,nd=c[a],c[a+1]
    q=a+4+nf; q=(q+3)&~3
    fo=struct.unpack_from('<%dI'%(nd*nf),c,q)
    d=direction if nd>1 else 0
    f=fo[d*nf+fi]
    dx,dy,w,h=struct.unpack_from('<hhHH',c,f); f+=8
    img=np.zeros((h,w),np.uint8)
    if bpp==4:
        rb=(w+1)//2
        for y in range(h):
            row=c[f+y*rb:f+(y+1)*rb]
            for x in range(w):
                b=row[x//2]; img[y,x]=(b>>4) if x&1 else (b&15)
    else:
        for y in range(h): img[y]=list(c[f+y*w:f+(y+1)*w])
    rgb=np.zeros((h,w,3),np.uint8)
    for i,col in enumerate(pal):
        rgb[img==i]=[(col&31)*8,((col>>5)&31)*8,((col>>10)&31)*8]
    return img>0, rgb
