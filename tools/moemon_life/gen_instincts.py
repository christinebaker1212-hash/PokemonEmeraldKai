#!/usr/bin/env python3
"""Moemon Co-op, MOeMON GARDEN: species instincts from the Pokedex texts.

Reads every Pokedex entry the game has (Kai's Emerald text in src/data/pokemon/pokedex_text.h and the
Ruby / Sapphire / FireRed texts in src/data/pokemon/mp_dex_more.h, tools/moemon_dex) and gives each species
instinct flags from words its entries use (nocturnal, sings, digs, lives in trees, shy, fierce...). The
A-Life (src/mp/mp_life.c) adds the rest from the species data itself (types, egg groups, base stats).

Usage: gen_instincts.py [REPO]   writes REPO/src/data/mp_life_instincts.h
"""
import os
import re
import sys

REPO = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')

# flag name -> word stems (matched at the start of a word, case-insensitive)
FLAGS = [
    ('NIGHT', ['night', 'nocturnal', 'moonlight', 'moonless', 'darkness', 'midnight']),
    ('SING', ['sing', 'song', 'melod', 'music', 'hum ', 'humming', 'chirp']),
    ('DIG', ['dig', 'burrow', 'underground', 'tunnel', 'soil']),
    ('TREE', ['tree', 'forest', 'branch', 'woods', 'jungle']),
    ('SLEEP', ['sleep', 'nap', 'doze', 'drows', 'yawn', 'lazy', 'slumber']),
    ('SHY', ['timid', 'shy', 'cowar', 'hides', 'hide ', 'hiding', 'skittish', 'nervous', 'frighten', 'flee']),
    ('FIERCE', ['fierce', 'aggress', 'vicious', 'ferocious', 'violent', 'savage', 'brutal', 'rampag']),
    ('PLAY', ['play', 'frolic', 'danc', 'mischie', 'prank', 'tease']),
    ('CURIOUS', ['curio', 'investigat', 'inquisit', 'explor']),
    ('PACK', ['pack', 'group', 'flock', 'swarm', 'colony', 'colonies', 'herd', 'school']),
    ('GLUTTON', ['eat', 'appetite', 'devour', 'feed', 'food', 'glutton', 'hungry']),
    ('SUN', ['sun', 'bask']),
    ('SEA', ['sea', 'ocean', 'river', 'lake', 'swim', 'pond', 'marsh', 'swamp']),
    ('SKY', ['sky', 'soar', 'flies', 'flight', 'fly ', 'flying', 'wing']),
    ('COLD', ['snow', 'ice', 'icy', 'cold', 'freez', 'frozen', 'blizzard']),
    ('HOT', ['volcan', 'flame', 'magma', 'lava', 'blaz']),
    ('LOYAL', ['loyal', 'faithful', 'obey', 'devot']),
    ('PROUD', ['proud', 'pride', 'haughty', 'arrogan', 'noble']),
    ('GENTLE', ['gentle', 'kind', 'docile', 'calm', 'peaceful', 'tame', 'friendly']),
    ('LONER', ['alone', 'solitar', 'loner', 'lone ']),
    ('COLLECT', ['collect', 'hoard', 'shiny', 'treasure', 'gather']),
    ('CLEAN', ['groom', 'clean', 'tidy', 'preen']),
]


def c_strings(body):
    return ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', body)).replace('\\n', ' ')


# Lore the texts don't spell out (the A-Life adds lore pairs like ZANGOOSE / SEVIPER itself)
OVERRIDES = {
    'ZANGOOSE': ['INST_FIERCE'],
    'SEVIPER': ['INST_FIERCE'],
    'ABSOL': ['INST_LONER'],
}

NEGATIONS = ('avoid', 'dislike', 'hate', 'fear', 'afraid', 'never', 'not ', "n't", 'unable', 'without', 'away from', 'shun', 'weak')


def Mentions(text, stem):
    """The stem used as a word start, and not right after a negation ('avoids sunlight')."""
    for m in re.finditer(r'[^a-z]' + re.escape(stem), text):
        before = text[max(0, m.start() - 28):m.start()]
        if not any(n in before for n in NEGATIONS):
            return True
    return False


def main():
    ent = open(os.path.join(REPO, 'src/data/pokemon/pokedex_entries.h'), encoding='utf-8').read()
    names = re.findall(r'\[NATIONAL_DEX_(\w+)\] =\s*\{.*?\.description = g(\w+)PokedexText,', ent, re.S)[1:]
    kai = open(os.path.join(REPO, 'src/data/pokemon/pokedex_text.h'), encoding='utf-8').read()
    texts = {m.group(1): c_strings(m.group(2)) for m in re.finditer(r'const u8 g(\w+?)PokedexText\[\] = _\((.*?)\);', kai, re.S)}
    more = open(os.path.join(REPO, 'src/data/pokemon/mp_dex_more.h'), encoding='utf-8').read()
    for m in re.finditer(r'static const u8 sMpDex(?:Ruby|Sapphire|FireRed)_(\w+)\[\] = _\("(.*?)"\);', more):
        texts[m.group(1)] = texts.get(m.group(1), '') + ' ' + m.group(2)
    rows = []
    counts = {f: 0 for f, _ in FLAGS}
    for const, ident in names:
        t = ' ' + texts.get(ident, '').lower().replace('moémon', 'monster') + ' '
        bits = []
        for i, (flag, stems) in enumerate(FLAGS):
            if any(Mentions(t, s) for s in stems):
                bits.append('INST_' + flag)
                counts[flag] += 1
        if 'INST_NIGHT' in bits and 'INST_SUN' in bits:
            bits.remove('INST_SUN')        # night creatures that mention the sun avoid it ('burned by the sun')
            counts['SUN'] -= 1
        for b in OVERRIDES.get(const, []):
            if b not in bits:
                bits.append(b)
        rows.append('    [NATIONAL_DEX_%s] = %s,' % (const, ' | '.join(bits) if bits else '0'))
    out = os.path.join(REPO, 'src/data/mp_life_instincts.h')
    with open(out, 'w', encoding='utf-8') as f:
        f.write('// Moemon Co-op, MOeMON GARDEN: species instincts from the Pokedex texts (tools/moemon_life/gen_instincts.py).\n')
        f.write('// Generated: do not edit by hand.\n\n')
        for i, (flag, _) in enumerate(FLAGS):
            f.write('#define INST_%-8s (1u << %d)\n' % (flag, i))
        f.write('\nstatic const u32 sMpLifeInstincts[NATIONAL_DEX_COUNT + 1] =\n{\n')
        f.write('\n'.join(rows))
        f.write('\n};\n')
    print('wrote', out)
    print(' '.join('%s %d' % (k, v) for k, v in counts.items()))


if __name__ == '__main__':
    main()
