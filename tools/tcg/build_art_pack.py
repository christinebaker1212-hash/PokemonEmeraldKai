#!/usr/bin/env python3
"""Build a small, immutable artwork pack without changing card dimensions.
JPEG 4:4:4 is supported by Kai's existing decoder, avoiding new runtime libraries.
Select the smallest tested quality meeting a conservative per-image error limit;
all conversions start at the original PNG, never at a previous lossy encoding.
"""
import concurrent.futures,hashlib,io,json,math,zipfile
from pathlib import Path
from PIL import Image,ImageChops,ImageStat
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'dev/artwork/tcg-art';OUT.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'tools/tcg/art_manifest.json').read_text())
rows=[r for r in manifest['cards'] if r['status']=='matched']
def psnr(a,b):
    mse=sum(v*v for v in ImageStat.Stat(ImageChops.difference(a,b)).rms)/3
    return 99.0 if not mse else 10*math.log10(255*255/mse)
def process(row):
    source=Image.open(ROOT/row['path']).convert('RGB');best=None
    for quality in [85,90,92,95,98,100]:
        stream=io.BytesIO();source.save(stream,format='JPEG',quality=quality,subsampling=0,optimize=True,progressive=True)
        data=stream.getvalue();decoded=Image.open(io.BytesIO(data)).convert('RGB')
        # Assess both the full scan and the rule/text region. Smaller candidates
        # must satisfy both thresholds, rather than hiding text errors in artwork.
        text_box=(0,source.height//2,source.width,source.height)
        score=psnr(source,decoded);text_score=psnr(source.crop(text_box),decoded.crop(text_box))
        if score>=47 and text_score>=47 and (best is None or len(data)<len(best[0])):best=(data,quality,score,text_score)
    if best is None:raise ValueError(f'No acceptable encoding for card {row["card_id"]}')
    data,quality,score,text_score=best;card=row['card_id'];name=f'cards/{card:03d}.jpg';path=OUT/name;path.parent.mkdir(exist_ok=True);path.write_bytes(data)
    thumb=source.copy();thumb.thumbnail((160,220),Image.Resampling.LANCZOS);stream=io.BytesIO();thumb.save(stream,format='JPEG',quality=85,subsampling=0,optimize=True)
    thumb_data=stream.getvalue();tn=f'thumbnails/{card:03d}.jpg';tp=OUT/tn;tp.parent.mkdir(exist_ok=True);tp.write_bytes(thumb_data)
    return {'id':card,'printing':row['printing'],'file':name,'thumbnail':tn,'width':source.width,'height':source.height,'quality':quality,'psnr':round(score,2),'text_psnr':round(text_score,2),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'thumbnail_bytes':len(thumb_data),'thumbnail_sha256':hashlib.sha256(thumb_data).hexdigest(),'source_sha256':row['sha256']}
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:results=list(pool.map(process,rows))
body={'format':1,'encoding':'JPEG 4:4:4','cards':results}
encoded=json.dumps(body,sort_keys=True,separators=(',',':')).encode();version=hashlib.sha256(encoded).hexdigest()[:16]
filename=f'moemon-tcg-art-{version}.zip';archive=OUT/filename
# JPEGs are already compressed. Storing them avoids spending every release's
# CPU time trying to deflate them again and makes extraction inexpensive.
with zipfile.ZipFile(archive,'w',compression=zipfile.ZIP_STORED) as z:
    for r in results:
        for key in ['file','thumbnail']:
            info=zipfile.ZipInfo(f'tcg/art/{version}/{r[key]}',date_time=(2026,1,1,0,0,0));info.external_attr=0o100644<<16
            z.writestr(info,(OUT/r[key]).read_bytes())
    z.writestr(f'tcg/art/{version}/manifest.json',encoded)
raw=archive.read_bytes();release={'version':version,'file':filename,'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),'cards':len(results),'source_bytes':sum(r['bytes'] for r in rows),'full_size_bytes':sum(r['bytes'] for r in results),'thumbnail_bytes':sum(r['thumbnail_bytes'] for r in results)}
(ROOT/'tools/tcg/art_pack.json').write_text(json.dumps(release,indent=2)+'\n')
(ROOT/'include/tcg/art_pack.h').write_text('#ifndef TCG_ART_PACK_H\n#define TCG_ART_PACK_H\n#define TCG_ART_VERSION "'+version+'"\n#define TCG_ART_ARCHIVE "'+filename+'"\n#define TCG_ART_SHA256 "'+release['sha256']+'"\n#define TCG_ART_BYTES '+str(len(raw))+'\n#endif\n')
(OUT/'manifest.json').write_bytes(encoded)
print(json.dumps(release,indent=2));print(f'Reduction including thumbnails and ZIP: {100*(1-len(raw)/release["source_bytes"]):.1f}%')
