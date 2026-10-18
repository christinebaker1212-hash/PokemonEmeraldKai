import bisect
def load_syms():
    syms=[]
    for l in open('/home/claude/syms/pokeemerald.sym'):
        p=l.split()
        if len(p)>=4 and p[0].startswith('08'):
            syms.append((int(p[0],16)-0x8000000,int(p[2],16),p[3]))
    syms.sort(); return syms
def load_ips(path='/home/claude/work/kai.ips'):
    d=open(path,'rb').read();i=5;recs=[]
    while d[i:i+3]!=b'EOF':
        off=int.from_bytes(d[i:i+3],'big');sz=int.from_bytes(d[i+3:i+5],'big');i+=5
        if sz==0: rl=int.from_bytes(d[i:i+2],'big');recs.append((off,rl,d[i+2:i+3]*rl));i+=3
        else: recs.append((off,sz,d[i:i+sz]));i+=sz
    return recs
def symmap(recs,syms):
    addrs=[s[0] for s in syms];out=[]
    for off,sz,data in recs:
        # split across symbols
        o=off;end=off+sz
        while o<end:
            j=bisect.bisect_right(addrs,o)-1
            if j<0: out.append((o,end-o,'?'));break
            a,s,n=syms[j]
            nxt=syms[j+1][0] if j+1<len(syms) else 1<<30
            e=min(end,nxt)
            out.append((o,e-o,n));o=e
    return out
