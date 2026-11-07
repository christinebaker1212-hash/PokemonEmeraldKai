"""Dry run sheet: WES and RUI as GBA walking / running sprites beside BRENDAN. python3 tools/orre/people_sprites_demo.py OUTDIR"""
import sys
sys.path.insert(0, __import__('os').path.dirname(__import__('os').path.abspath(__file__)))
import hsd_anim as anim, people_sprites as sprite, numpy as np
from PIL import Image, ImageDraw
OUT = sys.argv[1]
SS = 8; TILT = 22
YAW = {'down': 180, 'up': 0, 'left': 90}
DIR = __import__('os').path.expanduser('~/.cache/moemon/orre/unpacked/field_common/')

def sheet(path, neck, kinds, W=16, H=32, head=1.9, body=0.75, fit=30):
    f, root, anims = anim.model(DIR + path + '.dat')
    deform = sprite.chibi(neck, head, body) if neck else None
    idle, walk, run = anims[1], anims[2], anims[3]
    ms = sprite.pose(f, root, idle, 0, deform)
    big, zb, ib = sprite.render_rgba(ms, f.images, W * SS, H * SS * 2, (W * SS / 2, H * SS * 2 - SS), SS, TILT, 180)
    rows = np.where((ib >= 0).any(1))[0]; ppu = fit * SS / (rows.max() - rows.min() + 1)
    out = {}
    for kind in kinds:
        steps = [(walk, 0.0), (walk, 15.0)] if kind == 'walk' else [(run, 0.0), (run, 8.5)]
        order = [(idle, 0, 'down'), (idle, 0, 'up'), (idle, 0, 'left')]
        for d in ('down', 'up', 'left'): order += [(a, fr, d) for a, fr in steps]
        frames = []
        for a, fr, d in order:
            ms = sprite.pose(f, root, a, fr, deform)
            big, zb, ib = sprite.render_rgba(ms, f.images, W * SS, H * SS, (W * SS / 2, H * SS - SS), ppu * SS, TILT, YAW[d])
            frames.append(sprite.downsample2(big, zb, ib, SS))
        out[kind] = frames
    imgs, pal = sprite.to_gba2(sum(out.values(), []))
    res = {}
    for n, kind in enumerate(kinds):
        o = np.zeros((H, W * 9, 4), np.uint8)
        for i, im in enumerate(imgs[n * 9:(n + 1) * 9]): o[:, i * W:(i + 1) * W] = im
        res[kind] = Image.fromarray(o)
    return res, pal

rows = [('BRENDAN (Emerald, 16x32, for scale)', Image.open(__import__('os').path.join(__import__('os').path.dirname(__import__('os').path.abspath(__file__)), '../../graphics/object_events/pics/people/brendan/walking.png')).convert('RGBA'))]
wes, wp = sheet('ken_b1', 0x73a0, ('walk', 'run'))
rui, rp = sheet('hizuki_b1', 0x56e4, ('walk', 'run'), W=32)
for k, im in wes.items(): im.save(OUT + '/wes_%s.png' % k); rows.append(('WES %s, 16x32' % k, im))
for k, im in rui.items(): im.save(OUT + '/rui_%s.png' % k); rows.append(('RUI %s, 32x32 (her pigtails need the width)' % k, im))
Z = 3; pad = 18
wmax = max(im.size[0] for _, im in rows)
sh = Image.new('RGBA', (wmax * Z + 20, len(rows) * (32 * Z + pad) + 10 + 32 * 2 + 60), (56, 72, 56, 255)); d = ImageDraw.Draw(sh)
for i, (t, im) in enumerate(rows):
    y = i * (32 * Z + pad) + pad
    d.text((10, y - 14), t, fill=(255, 255, 255))
    sh.alpha_composite(im.resize((im.size[0] * Z, 32 * Z), Image.NEAREST), (10, y))
# at play size (2x, like the game's window): Wes walking down, Rui behind him, on Orre-ish sand
y0 = len(rows) * (32 * Z + pad) + pad
d.text((10, y0 - 14), 'At 2x, the walk cycle (stand, step, stand, step), Rui following', fill=(255, 255, 255))
ground = Image.new('RGBA', (wmax * Z, 32 * 2 + 20), (200, 168, 112, 255))
gd = ImageDraw.Draw(ground)
for x in range(0, ground.size[0], 32):
    gd.line([(x, 0), (x, ground.size[1])], fill=(188, 156, 100, 255))
cyc = [0, 3, 0, 4]
for n in range(8):
    fw = wes['walk'].crop((cyc[n % 4] * 16, 0, cyc[n % 4] * 16 + 16, 32)).resize((32, 64), Image.NEAREST)
    fl = [2, 7, 2, 8][n % 4]
    fr = rui['walk'].crop((fl * 32, 0, fl * 32 + 32, 32)).transpose(Image.FLIP_LEFT_RIGHT).resize((64, 64), Image.NEAREST)
    x = 20 + n * 100
    ground.alpha_composite(fr, (x, 10)); ground.alpha_composite(wes['walk'].crop(((2 if n % 2 == 0 else 7 + (n // 2) % 2) * 16, 0, (2 if n % 2 == 0 else 7 + (n // 2) % 2) * 16 + 16, 32)).transpose(Image.FLIP_LEFT_RIGHT).resize((32, 64), Image.NEAREST), (x + 56, 10))
sh.alpha_composite(ground, (10, y0))
sh.save(OUT + '/orre_sprites_dryrun.png')
print('palettes', len(wp), len(rp))
