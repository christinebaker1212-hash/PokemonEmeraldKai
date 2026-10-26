#!/usr/bin/env python3
"""Measure artwork encodings at source resolution; retain inspection examples."""
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
import csv,io,time,math
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'dev/artwork/benchmark';OUT.mkdir(parents=True,exist_ok=True)
files=[ROOT/'graphics/tcg/printed'/f'{n:03d}.png' for n in [1,50,144,195,214,228]]
files=[p for p in files if p.exists()]
profiles=[('png','PNG',{'optimize':True}),('webp-lossless','WEBP',{'lossless':True,'method':6}),('webp-90','WEBP',{'quality':90,'method':6}),('webp-95','WEBP',{'quality':95,'method':6}),('avif-80','AVIF',{'quality':80,'speed':6}),('avif-90','AVIF',{'quality':90,'speed':6}),('jpeg-85','JPEG',{'quality':85,'subsampling':0,'optimize':True,'progressive':True}),('jpeg-90','JPEG',{'quality':90,'subsampling':0,'optimize':True,'progressive':True}),('jpeg-92','JPEG',{'quality':92,'subsampling':0,'optimize':True,'progressive':True}),('jpeg-95','JPEG',{'quality':95,'subsampling':0,'optimize':True,'progressive':True}),('avif-80-444','AVIF',{'quality':80,'speed':6,'subsampling':'4:4:4'}),('avif-90-444','AVIF',{'quality':90,'speed':6,'subsampling':'4:4:4'})]
rows=[]
for p in files:
 source=Image.open(p).convert('RGB')
 for name,fmt,options in profiles:
  out=io.BytesIO();start=time.perf_counter();source.save(out,format=fmt,**options);encoded=time.perf_counter()-start
  data=out.getvalue();start=time.perf_counter();decoded=Image.open(io.BytesIO(data)).convert('RGB');decoded.load();decode=time.perf_counter()-start
  mse=sum(v*v for v in ImageStat.Stat(ImageChops.difference(source,decoded)).rms)/3
  rows.append([p.name,name,len(data),round(encoded*1000,2),round(decode*1000,2),round(10*math.log10(255*255/mse),2) if mse else 'lossless'])
  if p.stem in ['050','195','214']:
   ext={'WEBP':'webp','AVIF':'avif','PNG':'png','JPEG':'jpg'}[fmt];(OUT/f'{p.stem}-{name}.{ext}').write_bytes(data)
with (OUT/'results.csv').open('w') as f:
 writer=csv.writer(f);writer.writerow(['image','profile','bytes','encode_ms','decode_ms','psnr']);writer.writerows(rows)
print('Source bytes:',sum(p.stat().st_size for p in files),'cards:',len(files))
for name,_,_ in profiles:
 a=[r for r in rows if r[1]==name];print(name,'bytes',sum(r[2] for r in a),'decode_ms',round(sum(r[4] for r in a)/len(a),2),'psnr',','.join(str(r[5]) for r in a))
