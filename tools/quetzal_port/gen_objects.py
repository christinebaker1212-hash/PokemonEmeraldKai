#!/usr/bin/env python3
"""Johto / Kanto overworld sprites: every OBJ_EVENT_GFX_QZ_<id> the maps and scripts name gets a real graphics id.

    python3 tools/quetzal_port/gen_objects.py

- A sprite the rip found identical to one of Kai's ("matches": "kai <png>") uses Kai's own id.
- Quetzal's VAR_OBJ_GFX_ID_n ids become Kai's OBJ_EVENT_GFX_VAR_n.
- Everything else (FRLG people, Johto's own characters, Quetzal's overworld Pokémon) is imported from the rip:
  graphics/object_events/pics/qz/, palettes in graphics/object_events/palettes/qz/, ids from OBJ_EVENT_GFX_QZ_FIRST
  (0x100, 16-bit graphics ids), each with its own palette loaded by tag into any free sprite palette (PALSLOT_QZ).
  Quetzal's 6-frame Pokémon sheets (down, down, side, side, up, up) are laid out as the standard 9 frames.
Writes src/data/object_events/qz_object_events.h, tools/quetzal_port/objects.json (Quetzal id -> Kai id) and the
OBJ_EVENT_GFX_QZ lines of include/constants/qz_port.h.
"""
import json
import os
import re
import shutil

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
RIP = os.path.join(REPO, 'quetzal_rip', 'object_events')
FIRST = 0x100
PAL_TAG_BASE = 0xA000
OAM = {(16, 16): '16x16', (16, 32): '16x32', (32, 32): '32x32', (64, 64): '64x64', (64, 32): '64x32',
       (88, 32): '88x32', (96, 40): '96x40', (48, 48): '48x48'}
BASE_OAM = {'16x16': '16x16', '16x32': '16x32', '32x32': '32x32', '64x64': '64x64', '64x32': '64x32',
            '88x32': '64x64', '96x40': '64x64', '48x48': '64x64'}
SHADOW = {16: 'SHADOW_SIZE_S', 32: 'SHADOW_SIZE_M', 64: 'SHADOW_SIZE_L'}
# Quetzal's 6-frame Pokémon sheet -> the standard 9 frames (down, up, side, down walk x2, up walk x2, side walk x2)
SIX = [0, 4, 2, 1, 1, 5, 5, 3, 3]
# field obstacles with an animation of their own (CUT, ROCK SMASH: JOHTO's 82 / 86, KANTO's FireRed 16127 / 16190):
# FireRed's frames are in Emerald's order
OBSTACLE_ANIMS = {82: 'sAnimTable_CuttableTree', 86: 'sAnimTable_BreakableRock', 16127: 'sAnimTable_CuttableTree',
                  16190: 'sAnimTable_BreakableRock'}


def read(p):
    return open(os.path.join(REPO, p), encoding='utf-8').read()


def kai_paths():
    """png path -> Kai graphics id constant."""
    ptrs = dict((m.group(2), m.group(1)) for m in re.finditer(r'\[(OBJ_EVENT_GFX_\w+)\]\s*=\s*&(\w+)',
                read('src/data/object_events/object_event_graphics_info_pointers.h')))
    infos = dict(re.findall(r'const struct ObjectEventGraphicsInfo (\w+) = \{.*?\.images = (\w+)',
                            read('src/data/object_events/object_event_graphics_info.h'), re.S))
    tables = dict(re.findall(r'static const struct SpriteFrameImage (\w+)\[\] = \{\s*overworld_frame\((\w+)',
                             read('src/data/object_events/object_event_pic_tables.h')))
    pics = dict(re.findall(r'const u32 (\w+)\[\] = INCGFX_U32\("([^"]+)"', read('src/data/object_events/object_event_graphics.h')))
    out = {}
    for info, const in ptrs.items():
        pic = pics.get(tables.get(infos.get(info, ''), ''), '')
        if pic and pic not in out:
            out[pic] = const
    return out


def main():
    sprites = json.load(open(os.path.join(RIP, 'object_events.json')))
    kai = kai_paths()
    os.makedirs(os.path.join(REPO, 'graphics/object_events/pics/qz'), exist_ok=True)
    paldir = os.path.join(REPO, 'graphics/object_events/palettes/qz')
    if os.path.isdir(paldir):
        shutil.rmtree(paldir)
    os.makedirs(paldir)
    ids, pal_tags = {}, {}
    pics, tables, infos, pals = [], [], [], []
    n = 0
    # sprites added later (tools/quetzal_rip/var_sprites.py) go last, so the ids saves hold never move
    late = lambda s: s.get('used_by') == ["(a script's graphics variable)"]
    for s in sorted(sprites, key=lambda s: (late(s), s['id'])):
        q = s['const']
        m = re.match(r'VAR_OBJ_GFX_ID_(\w)', s.get('note', ''))
        if m:
            ids[q] = 'OBJ_EVENT_GFX_VAR_%s' % m.group(1)
            continue
        match = s.get('matches') or ''
        if match.startswith('kai ') and match[4:] in kai:
            ids[q] = kai[match[4:]]
            continue
        size = OAM.get((s['width'], s['height']))
        png = os.path.join(RIP, 'pics', 'gfx_%d.png' % s['id'])
        if size is None or not os.path.exists(png):
            print('  no import for %s (%dx%d): stand-in' % (q, s['width'], s['height']))
            ids[q] = 'OBJ_EVENT_GFX_NINJA_BOY'
            continue
        gid = FIRST + n
        n += 1
        ids[q] = '0x%X' % gid
        name = 'Qz%d' % s['id']
        shutil.copy(png, os.path.join(REPO, 'graphics/object_events/pics/qz/gfx_%d.png' % s['id']))
        # the picture's own colors (the rip drew each sheet with the palette it is shown with)
        from PIL import Image
        raw = tuple(Image.open(png).getpalette()[:48])
        if raw not in pal_tags:
            pal_tags[raw] = PAL_TAG_BASE + len(pal_tags)
            k = pal_tags[raw] - PAL_TAG_BASE
            with open(os.path.join(REPO, 'graphics/object_events/palettes/qz/pal_%03d.pal' % k), 'w', newline='\r\n') as f:
                f.write('JASC-PAL\n0100\n16\n' + ''.join('%d %d %d\n' % raw[i:i + 3] for i in range(0, 48, 3)))
            pals.append('const u16 gObjectEventPal_Qz%03d[] = INCGFX_U16("graphics/object_events/palettes/qz/pal_%03d.pal", ".gbapal");\n'
                        % (k, k))
        tag = raw
        tw, th = s['width'] // 8, s['height'] // 8
        pics.append('const u32 gObjectEventPic_%s[] = INCGFX_U32("graphics/object_events/pics/qz/gfx_%d.png", ".4bpp", "-mwidth %d -mheight %d");\n'
                    % (name, s['id'], tw, th))
        frames = s['frames']
        inanimate = s['inanimate'] or frames == 1
        if frames == 6 and not inanimate:
            order = SIX
        elif inanimate:
            order = list(range(frames))
        else:
            order = [i % frames for i in range(9)]
        tables.append('static const struct SpriteFrameImage sPicTable_%s[] = {\n%s};\n' % (name, ''.join(
            '    overworld_frame(gObjectEventPic_%s, %d, %d, %d),\n' % (name, tw, th, f) for f in order)))
        infos.append('''static const struct ObjectEventGraphicsInfo sObjectEventGraphicsInfo_%s = {
    .tileTag = TAG_NONE,
    .paletteTag = 0x%X,
    .reflectionPaletteTag = OBJ_EVENT_PAL_TAG_NONE,
    .size = %d,
    .width = %d,
    .height = %d,
    .paletteSlot = PALSLOT_QZ,
    .shadowSize = %s,
    .inanimate = %s,
    .disableReflectionPaletteLoad = TRUE,
    .tracks = %s,
    .oam = &gObjectEventBaseOam_%s,
    .subspriteTables = sOamTables_%s,
    .anims = %s,
    .images = sPicTable_%s,
    .affineAnims = gDummySpriteAffineAnimTable,
};
''' % (name, pal_tags[tag], tw * th * 32, s['width'], s['height'], SHADOW.get(min(s['width'], 64), 'SHADOW_SIZE_M'),
       'TRUE' if inanimate else 'FALSE', 'TRACKS_FOOT' if s.get('tracks') else 'TRACKS_NONE', BASE_OAM[size], size,
       OBSTACLE_ANIMS.get(s['id'], 'sAnimTable_Inanimate' if inanimate else 'sAnimTable_Standard'), name))
    order = [q for q in ids if ids[q].startswith('0x')]
    head = '// Generated by tools/quetzal_port/gen_objects.py: Johto / Kanto overworld sprites (graphics ids 0x100+)\n'
    out = head + ''.join(pics) + '\n' + ''.join(pals) + '\n' + '\n'.join(tables) + '\n' + '\n'.join(infos)
    out += '\nstatic const struct ObjectEventGraphicsInfo *const sQzObjectEventGraphicsInfoPointers[] = {\n%s};\n' % ''.join(
        '    &sObjectEventGraphicsInfo_Qz%s,\n' % q.rsplit('_', 1)[1] for q in order)
    out += '\nstatic const struct SpritePalette sQzObjectEventPalettes[] = {\n%s    {NULL, TAG_NONE},\n};\n' % ''.join(
        '    {gObjectEventPal_Qz%03d, 0x%X},\n' % (v - PAL_TAG_BASE, v) for t, v in pal_tags.items())
    open(os.path.join(REPO, 'src/data/object_events/qz_object_events.h'), 'w', encoding='utf-8').write(out)
    json.dump(ids, open(os.path.join(REPO, 'tools/quetzal_port/objects.json'), 'w'), indent=1)
    # Quetzal id -> Kai id for the graphics variables' values (event_data.c QzVarGraphicsId)
    pairs = sorted((int(q.rsplit('_', 1)[1]), v) for q, v in ids.items() if not 0xF0 <= int(q.rsplit('_', 1)[1]) <= 0xFF)
    open(os.path.join(REPO, 'src/data/object_events/qz_gfx_ids.h'), 'w').write(
        '// Generated by tools/quetzal_port/gen_objects.py: Quetzal graphics id -> Kai graphics id, for what JOHTO / KANTO\n'
        '// scripts store in a graphics variable (event_data.c QzVarGraphicsId)\nstatic const u16 sQzGfxIds[][2] = {\n'
        + ''.join('    {%d, %s},\n' % p for p in pairs) + '};\n')
    p = os.path.join(REPO, 'include/constants/qz_port.h')
    s = open(p, encoding='utf-8').read()
    s = re.sub(r'#define (OBJ_EVENT_GFX_QZ_\d+)\s+\S+', lambda m: '#define %-40s %s' % (m.group(1), ids.get(m.group(1), 'OBJ_EVENT_GFX_NINJA_BOY')), s)
    s = s.replace('// overworld graphics (placeholders until the sprites are imported)', '// overworld graphics (tools/quetzal_port/gen_objects.py)')
    open(p, 'w', encoding='utf-8').write(s)
    print('%d sprites imported (%d palettes), %d on Kai\'s own' % (len(order), len(pal_tags), len(ids) - len(order)))


if __name__ == '__main__':
    main()
