import struct
def yay0(src):
    assert src[:4]==b'Yay0', src[:4]
    size,link,chunk=struct.unpack('>III',src[4:16])
    mask_p=16; dst=bytearray(); bits=0; nb=0
    while len(dst)<size:
        if nb==0:
            bits=struct.unpack('>I',src[mask_p:mask_p+4])[0]; mask_p+=4; nb=32
        if bits&0x80000000:
            dst.append(src[chunk]); chunk+=1
        else:
            v=struct.unpack('>H',src[link:link+2])[0]; link+=2
            dist=(v&0xFFF)+1; n=v>>12
            if n==0: n=src[chunk]+18; chunk+=1
            else: n+=2
            for _ in range(n): dst.append(dst[-dist])
        bits=(bits<<1)&0xFFFFFFFF; nb-=1
    return bytes(dst)
def persszp(b):
    assert b[:8]==b'PERS-SZP'
    h=struct.unpack('>I',b[8:12])[0]
    return yay0(b[h:])
