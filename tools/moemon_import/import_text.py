from common import *
import subprocess,collections,bisect
recs=[]
d=open('/home/claude/work/kai.ips','rb').read();i=5
while d[i:i+3]!=b'EOF':
    off=int.from_bytes(d[i:i+3],'big');sz=int.from_bytes(d[i+3:i+5],'big');i+=5
    if sz==0: rl=int.from_bytes(d[i:i+2],'big');recs.append((off,rl));i+=3
    else: recs.append((off,sz));i+=sz
addrs=[s[0] for s in SYMS]
changed=set()
for off,sz in recs:
    o=off
    while o<off+sz and o<0xe3cf64:
        j=bisect.bisect_right(addrs,o)-1; a,s,n=SYMS[j]; changed.add(n)
        o=SYMS[j+1][0] if j+1<len(SYMS) else off+sz
PM=bytes.fromhex('cac9c51bc7c9c8'); MM=bytes.fromhex('c7c91bc7c9c8')
P=bytes.fromhex('cac9c51b'); M=bytes.fromhex('c7c91b')
def rd(R,a,lim):
    e=R.index(b'\xff',a); return R[a:e+1]
# index source definitions
src_files=subprocess.run('grep -rlE "POKéMON|POKé" data src --include=*.inc --include=*.c --include=*.h --include=*.s',shell=True,cwd=ROOT,capture_output=True,text=True).stdout.split()
cache={f:open(ROOT+'/'+f,encoding='utf-8').read() for f in src_files}
def find_range(sym):
    for f,t in cache.items():
        m=re.search(r'^'+sym+r'::?\s*$',t,re.M)
        if m:
            n=re.search(r'^\S+::?\s*$',t[m.end():],re.M)
            return f,m.end(),m.end()+(n.start() if n else len(t)-m.end())
        m=re.search(r'\b'+sym+r'\[[^\]]*\]\s*=\s*_\(',t)
        if m:
            e=t.index(');',m.end()); return f,m.end(),e
        m=re.search(r'\b'+sym+r'\[[^\]]*\]\s*=\s*\{',t)
        if m:
            j=m.end();dd=1
            while dd: dd+=(t[j]=='{')-(t[j]=='}'); j+=1
            return f,m.end(),j
    return None
ok=collections.Counter(); bad=[]; edits=collections.defaultdict(list)
for n in sorted(changed):
    a,s=BYNAME[n]
    if s==0: continue
    vb=V[a:a+s]; kb=K[a:a+s]
    if PM not in vb and P not in vb: continue
    # in-place region compare: kai should equal vanilla with replacements, padded
    exp=vb.replace(PM,MM).replace(P,M) if False else None
    exp1=vb.replace(PM,MM)
    exp2=exp1.replace(P,M)
    def match(e): return kb[:len(e)]==e
    if match(exp1): mode='PM'
    elif match(exp2): mode='P'
    else:
        # strings arrays (multiple 0xFF-terminated strings) compare per string
        bad.append(n); continue
    r=find_range(n)
    if not r: bad.append(n+' (no src)'); continue
    edits[r[0]].append((r[1],r[2],mode)); ok[mode]+=1
for f,lst in edits.items():
    t=cache[f]
    for st,en,mode in sorted(lst,reverse=True):
        seg=t[st:en]; seg=seg.replace('POKéMON','MOéMON')
        if mode=='P': seg=seg.replace('POKé','MOé')
        t=t[:st]+seg+t[en:]
    open(ROOT+'/'+f,'w',encoding='utf-8').write(t)
print(ok,len(edits),'files'); print('needs manual:',bad)

# ---- second pass: strings Kai also trimmed/padded; apply the swap to their definitions
MANUAL=[n for n in bad if ' ' not in n and n not in ('gDecorations','gItems','gTrainerClassNames')]
cache={f:open(ROOT+'/'+f,encoding='utf-8').read() for f in src_files}
done=[]
for n in MANUAL:
    r=find_range(n)
    if not r: print('no src',n); continue
    f,st,en=r; t=cache[f]; seg=t[st:en].replace('POKéMON','MOéMON').replace('POKé','MOé')
    cache[f]=t[:st]+seg+t[en:]; open(ROOT+'/'+f,'w',encoding='utf-8').write(cache[f]); done.append(n)
print('manual applied',len(done))
