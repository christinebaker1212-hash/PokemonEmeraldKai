import re,struct,bisect,os
ROOT='/home/claude/pcport'
K=open('/home/claude/work/kai.gba','rb').read()
V=open('/home/claude/work/vanilla.gba','rb').read()
SYMS=[]
for l in open('/home/claude/syms/pokeemerald.sym'):
    p=l.split()
    if len(p)>=4 and p[0].startswith('08'): SYMS.append((int(p[0],16)-0x8000000,int(p[2],16),p[3]))
SYMS.sort()
BYNAME={n:(a,s) for a,s,n in SYMS}
BYADDR={}
for a,s,n in SYMS: BYADDR.setdefault(a,n)
def u32(b,o): return struct.unpack_from('<I',b,o)[0]
def lz_len(b,o):
    assert b[o]==0x10,(hex(o),b[o])
    size=int.from_bytes(b[o+1:o+4],'little'); p=o+4; out=0
    while out<size:
        flags=b[p];p+=1
        for i in range(8):
            if out>=size: break
            if flags&0x80:
                n=(b[p]>>4)+3; p+=2; out+=n
            else: p+=1; out+=1
            flags<<=1
    return p-o, size
def lz_dec(b,o):
    size=int.from_bytes(b[o+1:o+4],'little'); p=o+4; out=bytearray()
    while len(out)<size:
        flags=b[p];p+=1
        for i in range(8):
            if len(out)>=size: break
            if flags&0x80:
                n=(b[p]>>4)+3; d=((b[p]&0xF)<<8|b[p+1])+1; p+=2
                for j in range(n): out.append(out[-d])
            else: out.append(b[p]);p+=1
            flags<<=1
    return bytes(out[:size])
BLOBDIR='graphics/moemon'
def write_blob(name,data):
    os.makedirs(os.path.join(ROOT,BLOBDIR),exist_ok=True)
    rel=f'{BLOBDIR}/{name}.bin'
    if len(data)%4: data=data+b'\0'*(4-len(data)%4)
    open(os.path.join(ROOT,rel),'wb').write(data); return rel
def find_def(sym):
    """find file and line regex of definition `<type> sym[] = ...;`"""
    import subprocess
    r=subprocess.run(['grep','-rlE',r'\b'+sym+r'\[\][^;]*=','src','include'],cwd=ROOT,capture_output=True,text=True).stdout.split()
    return r
def replace_def_with_incbin(sym,rel,width=None):
    files=find_def(sym)
    assert len(files)==1,(sym,files)
    f=os.path.join(ROOT,files[0]); t=open(f,encoding='utf-8').read()
    m=re.search(r'((?:static\s+)?const\s+(u8|u16|u32)\s+'+sym+r'\[\]\s*=\s*)(INC(?:GFX|BIN)_U(?:8|16|32)\([^;]*\));',t)
    assert m,(sym,files)
    w=width or m.group(2)[1:]
    t=t[:m.start(3)]+f'INCBIN_U{w}("{rel}")'+t[m.end(3):]
    open(f,'w',encoding='utf-8').write(t)
