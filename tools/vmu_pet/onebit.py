import sys; sys.path.insert(0,'.')
from pmd_read import *
from PIL import Image
def crop(m,rgb):
    ys,xs=np.nonzero(m); return m[ys.min():ys.max()+1, xs.min():xs.max()+1], rgb[ys.min():ys.max()+1, xs.min():xs.max()+1]
def onebit(dex, box=24, thr=0.42):
    m,rgb=frame(dex); m,rgb=crop(m,rgb)
    h,w=m.shape; s=max(h,w)
    lum=(rgb[...,0]*.3+rgb[...,1]*.59+rgb[...,2]*.11)/255.0
    if s>box:
        nw,nh=max(1,round(w*box/s)),max(1,round(h*box/s))
        a=np.array(Image.fromarray((m*255).astype(np.uint8)).resize((nw,nh),Image.BOX))/255.
        # luminance averaged over opaque pixels only
        L=np.array(Image.fromarray((lum*m*255).astype(np.uint8)).resize((nw,nh),Image.BOX))/255.
        L=np.where(a>0,L/np.maximum(a,1e-6),1)
        m=a>=0.45; lum=L
    # edge = opaque pixel touching transparent (4-neighbour)
    p=np.pad(m,1); edge=m & ~(p[:-2,1:-1]&p[2:,1:-1]&p[1:-1,:-2]&p[1:-1,2:])
    on=edge | (m & (lum<thr))
    return on
def sheet(dexes, box=24, z=5, cols=8):
    cell=box+4
    rows=(len(dexes)+cols-1)//cols
    img=np.full((rows*cell,cols*cell),0,np.uint8)
    for i,d in enumerate(dexes):
        on=onebit(d,box); h,w=on.shape
        r,c=divmod(i,cols); y=r*cell+2+(box-h); x=c*cell+2+(box-w)//2
        img[y:y+h,x:x+w]=on
    lcd_off=(156,170,140); lcd_on=(28,36,40)
    rgb=np.where(img[...,None]==1,np.array(lcd_on,np.uint8),np.array(lcd_off,np.uint8)).astype(np.uint8)
    return Image.fromarray(rgb).resize((img.shape[1]*z,img.shape[0]*z),Image.NEAREST)
if __name__=='__main__':
    ds=[1,4,7,25,133,143,149,6,131,448,445,94,130,39,52,258,280,393,906,909,912,175,172,447]
    sheet(ds,24).save('sample24.png'); sheet(ds,16).save('sample16.png')
