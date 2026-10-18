import re
from common import ROOT
cm={}
for l in open(ROOT+'/charmap.txt',encoding='utf-8'):
    l=l.split('@')[0].strip()
    m=re.match(r"^'(.+)'\s*=\s*([0-9A-Fa-f]{2})$",l)
    if m and int(m.group(2),16) not in cm: cm[int(m.group(2),16)]=m.group(1)
cm.update({0xFE:'\\n',0xFA:'\\l',0xFB:'\\p',0x00:' ',0x53:'{PK}',0x54:'{MN}'})
def dec(b):
    o='';i=0
    while i<len(b):
        x=b[i]
        if x==0xFF: o+='$';i+=1;continue
        if x in (0xFC,0xFD): o+='{%02X %02X}'%(x,b[i+1]);i+=2;continue
        o+=cm.get(x,'[%02X]'%x);i+=1
    return o
