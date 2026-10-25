"""Overworld (object event) sprites used by the Kanto / Johto maps.

Quetzal's graphics ids are u16 into a 20000-entry gObjectEventGraphicsInfoPointers (0x090EA9E8).
GetObjectEventGraphicsInfo: ids 0xF0-0xFF read VAR_OBJ_GFX_ID_*; 0x45 is player-dependent;
20000-29999 alias 0-9999; anything else >= 20000 falls back to id 5.
Palettes: (data*, u16 tag) pairs from 0x090FE26C; the NPCs Quetzal added use tags 0xB0xx / 0xB1xx.
"""
import glob
import json
import os

from qrom import OBJ_GFX_INFO
from gfx import gba_colors, write_jasc, tiles_png
import ffp2

POKEMON_OW = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'pokemon_ow_gfx.json')))['gid_to_species']

PAL_TABLE = 0x10FE26C
PAL_TABLE_END = 0x1104584


def palette_tags(rom):
    out = {}
    for o in range(PAL_TABLE, PAL_TABLE_END, 8):
        p = rom.ptr(o)
        if p is not None:
            out.setdefault(rom.u16(o + 4), p)
    return out


def graphics_info(rom, gid):
    p = rom.ptr(OBJ_GFX_INFO + 4 * gid)
    if p is None:
        return None
    d = {'addr': p, 'tile_tag': rom.u16(p), 'palette_tag': rom.u16(p + 2), 'reflection_palette_tag': rom.u16(p + 4),
         'size': rom.u16(p + 6), 'width': rom.s16(p + 8), 'height': rom.s16(p + 10)}
    f = rom.u8(p + 12)
    d['palette_slot'] = f & 15
    d['shadow_size'] = (f >> 4) & 3
    d['inanimate'] = bool(f & 0x40)
    d['disable_reflection_palette_load'] = bool(f & 0x80)
    d['tracks'] = rom.u8(p + 13)
    d['oam'] = rom.u32(p + 16)
    d['subsprite_tables'] = rom.u32(p + 20)
    d['anims'] = rom.u32(p + 24)
    d['images'] = rom.ptr(p + 28)
    d['affine_anims'] = rom.u32(p + 32)
    return d


def anim_frame_count(rom, anims):
    """1 + the highest image index any animation in the table shows."""
    t = anims - 0x08000000 if rom.isptr(anims) else None
    best = 0
    i = 0
    while t is not None and i < 64:
        a = rom.ptr(t + 4 * i)
        if a is None:
            break
        for k in range(64):
            v = rom.u32(a + 4 * k)
            typ = v & 0xFFFF
            if typ in (0xFFFF, 0xFFFE):      # end / jump
                break
            if typ == 0xFFFD:                # loop
                continue
            if typ > 255:
                break
            best = max(best, typ + 1)
        i += 1
    return best


def frames(rom, info, limit=64):
    out = []
    fs = info['width'] * info['height'] // 2
    q = info['images']
    limit = anim_frame_count(rom, info['anims']) or 1
    first = rom.ptr(q) if q is not None else None
    if first is not None and ffp2.is_ffp2(rom.d, first):
        return ffp2.decode_images(rom.d, q, min(limit, 6))
    while q is not None and len(out) < limit:
        p = rom.ptr(q)
        if p is None or rom.u16(q + 4) != fs:
            break
        out.append(rom.d[p:p + fs])
        q += 8
    return out


def png_frames_4bpp(path, w, h):
    """Turn a decomp overworld sheet (indexed PNG, frames side by side) into per-frame 4bpp data."""
    from PIL import Image
    im = Image.open(path)
    if im.mode != 'P':
        return []
    px = im.load()
    out = []
    for f in range(im.width // w):
        b = bytearray()
        for ty in range(h // 8):
            for tx in range(w // 8):
                for y in range(8):
                    for x in range(0, 8, 2):
                        lo = px[f * w + tx * 8 + x, ty * 8 + y] & 15
                        hi = px[f * w + tx * 8 + x + 1, ty * 8 + y] & 15
                        b.append(lo | hi << 4)
        out.append(bytes(b))
    return out


def reference_index(dirs, sizes):
    idx = {}
    for label, root in dirs:
        for p in glob.glob(os.path.join(root, 'graphics/object_events/pics/**/*.png'), recursive=True):
            rel = os.path.relpath(p, root)
            for w, h in sizes:
                try:
                    for f in png_frames_4bpp(p, w, h)[:1]:
                        idx.setdefault(f, '%s %s' % (label, rel))
                except Exception:
                    pass
    return idx


def gfx_ids_used(targets):
    ids = set()
    for m in targets:
        for o in m['events']['objects']:
            ids.add(o['graphics_id'])
    return ids


def rip_sprites(rom, targets, out, refs):
    pals = palette_tags(rom)
    ids = sorted(gfx_ids_used(targets))
    infos = {g: graphics_info(rom, g) for g in ids}
    sizes = {(i['width'], i['height']) for i in infos.values() if i}
    ref = reference_index(refs, sizes)
    d = os.path.join(out, 'object_events')
    os.makedirs(os.path.join(d, 'pics'), exist_ok=True)
    os.makedirs(os.path.join(d, 'palettes'), exist_ok=True)
    rows = []
    users = {}
    for m in targets:
        for o in m['events']['objects']:
            users.setdefault(o['graphics_id'], set()).add(m['name'])
    done_pals = set()
    for g in ids:
        info = infos[g]
        row = {'id': g, 'const': 'OBJ_EVENT_GFX_QZ_%d' % g, 'used_by': sorted(users[g])[:8], 'maps_using': len(users[g])}
        if 0xF0 <= g <= 0xFF:
            row['note'] = 'VAR_OBJ_GFX_ID_%X (set by script)' % (g - 0xF0)
            rows.append(row)
            continue
        if info is None:
            row['note'] = 'no graphics info'
            rows.append(row)
            continue
        fr = frames(rom, info)
        row.update({k: v for k, v in info.items() if k not in ('addr', 'images')})
        for k in ('oam', 'subsprite_tables', 'anims', 'affine_anims'):
            row[k] = '0x%X' % row[k]
        row['info_addr'] = '0x%X' % (info['addr'] + 0x08000000)
        row['frames'] = len(fr)
        row['matches'] = ref.get(fr[0]) if fr else None
        if str(g) in POKEMON_OW:
            sp, nm, shiny = POKEMON_OW[str(g)]
            row['pokemon'] = {'species_id': sp, 'name': nm, 'shiny': shiny}
        pp = pals.get(info['palette_tag'])
        colors = gba_colors(rom.d, pp) if pp is not None else [(i * 16, i * 16, i * 16) for i in range(16)]
        if pp is not None and info['palette_tag'] not in done_pals:
            write_jasc(os.path.join(d, 'palettes', 'pal_%04X.pal' % info['palette_tag']), colors)
            done_pals.add(info['palette_tag'])
        if fr:
            # frames side by side, like the decomp's sheets
            w, h = info['width'], info['height']
            sheet = bytearray()
            # tiles_png lays tiles out row by row, so interleave each frame's tile rows
            tw = w // 8
            th = h // 8
            n = len(fr)
            for ty in range(th):
                for f in fr:
                    sheet += f[ty * tw * 32:(ty + 1) * tw * 32]
            tiles_png(os.path.join(d, 'pics', 'gfx_%d.png' % g), bytes(sheet), colors, width_tiles=tw * n)
        rows.append(row)
    with open(os.path.join(d, 'object_events.json'), 'w') as fh:
        json.dump(rows, fh, indent=1)
        fh.write('\n')
    return rows
