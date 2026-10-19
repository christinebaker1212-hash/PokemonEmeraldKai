#!/usr/bin/env python3
"""Moemon Co-op: removes the sheet grid lines that slice.py left on the edge of
some follower frames (a straight one-color line, full height or width, within
3 pixels of a frame edge). Prints what it finds; --fix writes followers.bin."""
import json, os, sys, struct
base=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'graphics', 'moemon', 'followers') + os.sep
idx=json.load(open(base+'index.json'))
data=bytearray(open(base+'followers.bin','rb').read())
LINECOL={'kanto.png':(0,64,128),'johto.png':(85,85,85),'hoenn.png':(255,255,255)}
def c555(rgb): return (rgb[0]>>3)|((rgb[1]>>3)<<5)|((rgb[2]>>3)<<10)
fix='--fix' in sys.argv
found=0
for e in idx:
    sz=e['size']; fb=sz*sz//2; off=e['offset']
    pal=struct.unpack_from('<16H',data,off+8*fb)
    target=c555(LINECOL[e['sheet']])
    lineidx=list(range(1,16))
    tiles=sz//8
    def get(f,x,y):
        b=data[off+f*fb+((y//8)*tiles+x//8)*32+(y%8)*4+(x%8)//2]
        return (b>>4) if x&1 else b&15
    def put(f,x,y,c):
        p=off+f*fb+((y//8)*tiles+x//8)*32+(y%8)*4+(x%8)//2
        b=data[p]; data[p]=((b&0x0F)|(c<<4)) if x&1 else ((b&0xF0)|c)
    for f in range(8):
        for axis in 'xy':
            for k in [0,1,2,sz-3,sz-2,sz-1]:
                # longest run of one line-colored index along this column/row
                best=0;run=0;col=None
                for j in range(sz):
                    c=get(f,k,j) if axis=='x' else get(f,j,k)
                    if c in lineidx:
                        run=run+1 if c==col or col is None else 1; col=c
                    else:
                        run=0; col=None
                    best=max(best,run)
                if best>=sz*0.85:
                    found+=1
                    print(e['dex'],e['variant'],e['sheet'],'frame',f,axis,k,'run',best)
                    if fix:
                        for j in range(sz):
                            if axis=='x' and get(f,k,j) in lineidx: put(f,k,j,0)
                            if axis=='y' and get(f,j,k) in lineidx: put(f,j,k,0)
print('found',found)
if fix:
    open(base+'followers.bin','wb').write(data)
