#!/usr/bin/env python3
"""POKéMON forms outside battle (the user, 2026-11-15: "form changes, and they must scale across all regions"): each
form's stats, types and Abilities, its MOéMON pictures and the original POKéMON ones, for src/mp/mp_forms.c.

    python3 tools/moemon_forms/gen_forms.py [--rom QUETZAL.gba] [--expansion EXPANSION_DIR]

Sources: pokeemerald-expansion (the form's species entry: base stats, types, Abilities, its POKéMON pictures, palettes
and icon, copied to graphics/pokemon/forms/), Moemon Quetzal's ROM (the MOéMON pictures, by Quetzal's own species number
of the form: quetzal_rip/data/names.json lists the forms after the 948 POKéMON and megas; graphics/moemon/forms/).
Writes src/mp/mp_forms_data.c (generated). A form 0 is always the POKéMON's own entry (Kai's, unchanged); DEOXYS's
form 0 is the game's own (Emerald's Speed Forme), its Normal Forme is form 1 (Kai's picture's first frame).
"""
import argparse
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'moemon_gen59'))
sys.path.insert(0, os.path.join(REPO, 'tools', 'quetzal_rip'))
import gen_species as gs  # noqa: E402

DEFAULT_EXP = os.path.expanduser('~/.cache/moemon/expansion')
DEFAULT_ROM = os.path.expanduser('~/.cache/moemon/quetzal/quetzal.gba')

ARCEUS = ['FIGHTING', 'FLYING', 'POISON', 'GROUND', 'ROCK', 'BUG', 'GHOST', 'STEEL', 'FIRE', 'WATER', 'GRASS',
          'ELECTRIC', 'PSYCHIC', 'ICE', 'DRAGON', 'DARK', 'FAIRY']

# (Kai's species, form, the expansion's entry, Quetzal's species number of its MOéMON picture)
FORMS = [
    ('DEOXYS', 2, 'SPECIES_DEOXYS_ATTACK', 1031),
    ('DEOXYS', 3, 'SPECIES_DEOXYS_DEFENSE', 1032),
    ('ROTOM', 1, 'SPECIES_ROTOM_HEAT', 1041),
    ('ROTOM', 2, 'SPECIES_ROTOM_WASH', 1042),
    ('ROTOM', 3, 'SPECIES_ROTOM_FROST', 1043),
    ('ROTOM', 4, 'SPECIES_ROTOM_FAN', 1044),
    ('ROTOM', 5, 'SPECIES_ROTOM_MOW', 1045),
    ('GIRATINA', 1, 'SPECIES_GIRATINA_ORIGIN', 1046),
    ('SHAYMIN', 1, 'SPECIES_SHAYMIN_SKY', 1047),
    ('BURMY', 1, 'SPECIES_BURMY_SANDY', 1034),
    ('BURMY', 2, 'SPECIES_BURMY_TRASH', 1035),
    ('WORMADAM', 1, 'SPECIES_WORMADAM_SANDY', 1036),
    ('WORMADAM', 2, 'SPECIES_WORMADAM_TRASH', 1037),
    ('SHELLOS', 1, 'SPECIES_SHELLOS_EAST', 1039),
    ('GASTRODON', 1, 'SPECIES_GASTRODON_EAST', 1040),
    ('BASCULIN', 1, 'SPECIES_BASCULIN_BLUE_STRIPED', 1065),
    ('DEERLING', 1, 'SPECIES_DEERLING_SUMMER', 1068),
    ('DEERLING', 2, 'SPECIES_DEERLING_AUTUMN', 1069),
    ('DEERLING', 3, 'SPECIES_DEERLING_WINTER', 1070),
    ('SAWSBUCK', 1, 'SPECIES_SAWSBUCK_SUMMER', 1071),
    ('SAWSBUCK', 2, 'SPECIES_SAWSBUCK_AUTUMN', 1072),
    ('SAWSBUCK', 3, 'SPECIES_SAWSBUCK_WINTER', 1073),
] + [('ARCEUS', i + 1, 'SPECIES_ARCEUS_' + t, 1048 + i) for i, t in enumerate(ARCEUS)]

KAI_TYPES = None


def kai_type(t):
    global KAI_TYPES
    if KAI_TYPES is None:
        KAI_TYPES = set(re.findall(r'#define (TYPE_\w+)', open(os.path.join(REPO, 'include', 'constants', 'pokemon.h'),
                                                                encoding='utf-8').read()))
    return t if t in KAI_TYPES else 'TYPE_NORMAL'


def kai_ability(a):
    abil = open(os.path.join(REPO, 'include', 'constants', 'abilities.h'), encoding='utf-8').read()
    return a if re.search(r'#define %s\b' % re.escape(a), abil) else 'ABILITY_NONE'


def entries(exp):
    """the expansion's entry of each form: its fields (gen_species.py's reading)"""
    import glob
    src = ''.join(open(f, encoding='utf-8').read() for f in sorted(glob.glob(
        os.path.join(exp, 'src', 'data', 'pokemon', 'species_info', 'gen_*_families.h'))))
    macros, objmacros = {}, {}
    for m in re.finditer(r'^[ \t]*#define ([A-Z_0-9]+) ([^\n\\]+)$', src, re.M):
        objmacros.setdefault(m.group(1), m.group(2).strip())
    for m in re.finditer(r'^#define (\w+)\(([^)]*)\)((?:[^\n]*\\\n)*[^\n]*)', src, re.M):
        macros[m.group(1)] = ([x.strip() for x in m.group(2).split(',')], m.group(3).replace('\\\n', '\n'))
    gfx = open(os.path.join(exp, 'src', 'data', 'graphics', 'pokemon.h'), encoding='utf-8').read()
    paths = dict(re.findall(r'const u\d+ (g\w+)\[\] = INCGFX_\w+\("([^"]+)"', gfx))
    out = {}
    for base, form, k, q in FORMS:
        rest = re.split(r'\[%s\]\s*=' % re.escape(k), src, maxsplit=1)[1]
        mm = re.match(r'\s*(\w+)\(([^\n]*)\),\n', rest)
        if mm and mm.group(1) in macros:   # (ARCEUS_SPECIES_INFO(TYPE_FIGHTING, Fighting, 1): the macro's fields)
            params, text = macros[mm.group(1)]
            for p_, a_ in zip(params, [x.strip() for x in mm.group(2).split(',')]):
                text = re.sub(r'\b%s\b' % re.escape(p_), a_, text)
            body = re.sub(r'(\w+)\s*##\s*(\w+)', r'\1\2', text) + '\n'
            body = re.sub(r'[ \t]+\n', '\n', body)
        else:
            body = rest.split('\n    },\n', 1)[0] + '\n'
        body = gs.expand_macros(body, macros)
        body = re.sub(r'[ \t]*#if[^\n]*\n(.*?)[ \t]*#else[^\n]*\n.*?[ \t]*#endif[^\n]*\n', r'\1', body, flags=re.S)
        body = re.sub(r'[ \t]*#if[^\n]*\n(.*?)[ \t]*#endif[^\n]*\n', r'\1', body, flags=re.S)

        def f(field, default=None):
            m = re.search(r'\.%s\s*=\s*(.*?),\n' % re.escape(field), body, re.S)
            if not m:
                return default
            v = m.group(1).strip()
            return gs.pick(objmacros.get(v, v))
        e = {'base': base, 'form': form, 'key': k, 'quetzal': q}
        e['stats'] = [int(f(s, '0')) for s in ('baseHP', 'baseAttack', 'baseDefense', 'baseSpeed', 'baseSpAttack',
                                                'baseSpDefense')]
        types = re.findall(r'TYPE_\w+', objmacros.get(f('types', ''), f('types', '')) or '')
        e['types'] = [kai_type(t) for t in (types + types)[:2]] if types else ['TYPE_NORMAL', 'TYPE_NORMAL']
        ab = [kai_ability(a) for a in re.findall(r'ABILITY_\w+', f('abilities', '{ ABILITY_NONE }'))]
        e['abilities'] = [a for a in ab if a != 'ABILITY_NONE'][:2] or ['ABILITY_NONE']
        if len(e['abilities']) == 1:
            e['abilities'].append('ABILITY_NONE')
        for fld in ('frontPic', 'backPic', 'palette', 'shinyPalette', 'iconSprite'):
            m = re.search(r'\.%s = (\w+),' % fld, body)
            e[fld] = paths.get(m.group(1)) if m else None
        ip = f('iconPalIndex', '0')
        e['iconPal'] = int(ip) if ip and ip.isdigit() and int(ip) < 3 else 0
        out[(base, form)] = e
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--rom', default=DEFAULT_ROM)
    ap.add_argument('--expansion', default=DEFAULT_EXP)
    a = ap.parse_args()
    es = entries(a.expansion)
    # the MOéMON pictures from Quetzal (gen_species.py's from_quetzal, into graphics/moemon/forms/<name>/)
    for (base, form), e in es.items():
        e['name'] = '%s_%s' % (base.lower(), e['key'][len('SPECIES_%s_' % base):].lower())
    moe = moe_art(a.rom, es)
    lines = ['// Generated by tools/moemon_forms/gen_forms.py. Do not edit.\n',
             '#include "global.h"\n#include "constants/abilities.h"\n#include "constants/species.h"\n',
             '#include "mp_forms.h"\n\n']
    rows = []
    for (base, form), e in sorted(es.items(), key=lambda x: (x[0][0], x[0][1])):
        n = e['name']
        cm = ''.join(w.capitalize() for w in n.split('_'))
        pk = os.path.join('graphics', 'pokemon', 'forms', n)
        os.makedirs(os.path.join(REPO, pk), exist_ok=True)
        for fld, fn in (('frontPic', 'anim_front.png'), ('backPic', 'back.png'), ('palette', 'normal.pal'),
                        ('shinyPalette', 'shiny.pal'), ('iconSprite', 'icon.png')):
            src = e[fld]
            if src:
                srcp = os.path.join(a.expansion, src)
                if not os.path.exists(srcp):   # (.gbapal / .4bpp paths: the source file beside them)
                    for ext in ('.png', '.pal'):
                        cand = re.sub(r'\.(4bpp|gbapal)(\.lz)?$', ext, srcp)
                        if os.path.exists(cand):
                            srcp = cand
                            break
                shutil.copy(srcp, os.path.join(REPO, pk, fn))
        md = moe[(base, form)]
        lines.append('static const u32 sMoeFront_%s[] = INCGFX_U32("%s/anim_front.png", ".4bpp.lz");\n' % (cm, md))
        lines.append('static const u32 sMoeBack_%s[] = INCGFX_U32("%s/back.png", ".4bpp.lz");\n' % (cm, md))
        lines.append('static const u32 sMoePal_%s[] = INCGFX_U32("%s/normal.pal", ".gbapal.lz");\n' % (cm, md))
        lines.append('static const u32 sMoeShiny_%s[] = INCGFX_U32("%s/shiny.pal", ".gbapal.lz");\n' % (cm, md))
        lines.append('static const u32 sPkFront_%s[] = INCGFX_U32("%s/anim_front.png", ".4bpp.lz");\n' % (cm, pk))
        lines.append('static const u32 sPkBack_%s[] = INCGFX_U32("%s/back.png", ".4bpp.lz");\n' % (cm, pk))
        lines.append('static const u32 sPkPal_%s[] = INCGFX_U32("%s/normal.pal", ".gbapal.lz");\n' % (cm, pk))
        lines.append('static const u32 sPkShiny_%s[] = INCGFX_U32("%s/shiny.pal", ".gbapal.lz");\n' % (cm, pk))
        has_icon = os.path.exists(os.path.join(REPO, pk, 'icon.png'))   # (ARCEUS's forms: the base's icon)
        if has_icon:
            lines.append('static const u8 sIcon_%s[] = INCGFX_U8("%s/icon.png", ".4bpp");\n' % (cm, pk))
        s = e['stats']
        rows.append('    {SPECIES_%s, %d, {%s, %s}, {%s}, {%s, %s}, {sMoeFront_%s, sPkFront_%s}, {sMoeBack_%s, sPkBack_%s},\n'
                    '     {sMoePal_%s, sPkPal_%s}, {sMoeShiny_%s, sPkShiny_%s}, %s, %d},   // %s\n'
                    % (base, form, e['types'][0], e['types'][1], ', '.join(map(str, s)), e['abilities'][0],
                       e['abilities'][1], cm, cm, cm, cm, cm, cm, cm, cm, ('sIcon_' + cm) if has_icon else 'NULL', e['iconPal'],
                       e['key']))
    lines.append('\nconst struct MpFormData gMpForms[] = {\n' + ''.join(rows) + '};\n')
    lines.append('const u32 gMpFormCount = ARRAY_COUNT(gMpForms);\n')
    gs.write('src/mp/mp_forms_data.c', ''.join(lines))
    print('%d forms' % len(rows))


def moe_art(rom_path, es):
    """Quetzal's MOéMON pictures of each form (gen_species.py from_quetzal's reading): graphics/moemon/forms/<name>/"""
    from qrom import Rom
    from trainer_pics import decompress
    r = Rom(rom_path)
    out = {}
    for (base, form), e in es.items():
        n = e['quetzal']
        d = os.path.join('graphics', 'moemon', 'forms', e['name'])
        os.makedirs(os.path.join(REPO, d), exist_ok=True)
        front = decompress(r.d, r.u32(gs.Q_FRONT + 8 * n) - 0x8000000)
        back = decompress(r.d, r.u32(gs.Q_BACK + 8 * n) - 0x8000000)
        normal, shiny = pal(r, decompress, gs.Q_PAL, n), pal(r, decompress, gs.Q_SHINY, n)
        img(front + front, normal, 2).save(os.path.join(REPO, d, 'anim_front.png'))
        img(back, normal).save(os.path.join(REPO, d, 'back.png'))
        for fn, cols in (('normal.pal', normal), ('shiny.pal', shiny)):
            with open(os.path.join(REPO, d, fn), 'w', newline='\r\n') as fh:
                fh.write('JASC-PAL\n0100\n16\n' + ''.join('%d %d %d\n' % c for c in cols))
        out[(base, form)] = d
    return out


def pal(r, decompress, base, n):
    d = decompress(r.d, r.u32(base + 8 * n) - 0x8000000)
    return [((v & 31) * 255 // 31, ((v >> 5) & 31) * 255 // 31, ((v >> 10) & 31) * 255 // 31)
            for v in (d[i] | d[i + 1] << 8 for i in range(0, 32, 2))]


def img(t, colors, frames=1):
    from PIL import Image
    im = Image.new('P', (64, 64 * frames))
    im.putpalette([c for rgb in colors for c in rgb] + [0] * (768 - 48))
    px = im.load()
    for fr in range(frames):
        for ti in range(64):
            for y in range(8):
                for x in range(8):
                    b = t[ti * 32 + y * 4 + x // 2]
                    px[(ti % 8) * 8 + x, fr * 64 + (ti // 8) * 8 + y] = (b >> 4) if x & 1 else b & 15
    return im


if __name__ == '__main__':
    main()
