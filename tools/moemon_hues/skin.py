# Skin colors of a Moemon front picture: strong color rules for the light tones, then darker tones only when they
# mostly border the light skin (shadows of the same skin, not hair or clothes of a similar color)
def hsl(c):
    r,g,b=[x/255 for x in c]
    mx,mn=max(r,g,b),min(r,g,b); l=(mx+mn)/2; d=mx-mn
    if d==0: return 0,0,l
    s=d/(1-abs(2*l-1))
    if mx==r: h=((g-b)/d)%6
    elif mx==g: h=(b-r)/d+2
    else: h=(r-g)/d+4
    return h*60%360, s, l
def skinish(h):
    return h<=32 or h>=340
def strong(c):
    h,s,l=hsl(c)
    if skinish(h) and s>=0.25 and l>=0.55 and not (s>0.9 and l<0.75) and not (s>0.6 and l<0.65):
        return True
    return h<=44 and l>=0.83 and s>=0.5
def weak(c):
    h,s,l=hsl(c)
    if (h<=15 or h>=340) and s>0.6 and l<0.7:
        return False
    if skinish(h) and s>=0.2 and 0.33<=l<0.55 and s<=0.55:
        return True
    if skinish(h) and s>0.6 and 0.45<=l<0.65:
        return True
    return False
def neighbours(px):
    H=len(px); W=len(px[0])
    adj={}
    for y in range(H):
        for x in range(W):
            a=px[y][x]
            for dx,dy in ((1,0),(0,1)):
                xx,yy=x+dx,y+dy
                if xx<W and yy<H:
                    b=px[yy][xx]
                    if a!=b:
                        adj[(a,b)]=adj.get((a,b),0)+1; adj[(b,a)]=adj.get((b,a),0)+1
    return adj
def classify(p, px, frac=0.4):
    used=set(v for r in px for v in r)
    core={k for k in range(1,16) if k in used and strong(p[k])}
    outline={k for k in range(1,16) if hsl(p[k])[2]<0.12}
    adj=neighbours(px)
    out=set(core)
    for k in range(1,16):
        if k in core or k not in used or not weak(p[k]): continue
        tot=sum(n for (a,b),n in adj.items() if a==k and b!=0 and b not in outline)
        sk=sum(n for (a,b),n in adj.items() if a==k and b in core)
        if tot and sk/tot>=frac: out.add(k)
    return out
