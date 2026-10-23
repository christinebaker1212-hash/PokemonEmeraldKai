import struct
def lz10(d):
    assert d[0]==0x10, hex(d[0])
    n=d[1]|d[2]<<8|d[3]<<16
    out=bytearray(); i=4
    while len(out)<n:
        f=d[i]; i+=1
        for b in range(8):
            if len(out)>=n: break
            if f&(0x80>>b):
                x=d[i]<<8|d[i+1]; i+=2
                ln=(x>>12)+3; disp=(x&0xFFF)+1
                for _ in range(ln): out.append(out[-disp])
            else:
                out.append(d[i]); i+=1
    return bytes(out)
def pal(data):
    raw=lz10(data) if data[0]==0x10 else data
    cs=[]
    for k in range(16):
        c=raw[2*k]|raw[2*k+1]<<8
        r,g,b=c&31,(c>>5)&31,(c>>10)&31
        cs.append(((r<<3)|(r>>2),(g<<3)|(g>>2),(b<<3)|(b>>2)))
    return cs
def pic(data, w=64, h=64):
    raw=lz10(data)
    px=[[0]*w for _ in range(h)]
    t=0
    for ty in range(h//8):
        for tx in range(w//8):
            for y in range(8):
                for x in range(4):
                    v=raw[t*32+y*4+x]
                    px[ty*8+y][tx*8+2*x]=v&15
                    px[ty*8+y][tx*8+2*x+1]=v>>4
            t+=1
    return px
