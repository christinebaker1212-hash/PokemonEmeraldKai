#!/usr/bin/env python3
"""Moemon Co-op: import the Ruby, Sapphire and FireRed Pokedex entries.

The Pokedex info page shows every game's entry as one long text, in the order
Ruby, Sapphire, Emerald (the game's own entry, src/data/pokemon/pokedex_text.h),
FireRed. An entry that repeats an earlier one word for word is left out.

Inputs (pret decompilations, English):
  RUBY_SAPPHIRE  pokeruby src/data/pokedex_entries_en.h (#ifdef SAPPHIRE = Sapphire, #else = Ruby)
  FIRERED        pokefirered src/data/pokemon/pokedex_text_fr.h (the REVISION 1 text where they differ)

  https://raw.githubusercontent.com/pret/pokeruby/master/src/data/pokedex_entries_en.h
  https://raw.githubusercontent.com/pret/pokefirered/master/src/data/pokemon/pokedex_text_fr.h

Like the rest of Kai's Pokedex, POKeMON becomes MOeMON (POKe BALL stays).

Usage: import_dex.py RUBY_SAPPHIRE FIRERED [REPO [OUT]]
Writes REPO/src/data/pokemon/mp_dex_more.h and prints which entries were left out.
"""
import re
import sys
import os

ACCENT = 'é'


def c_strings(body):
    """The text of a run of C string literals ("a\\n" "b")."""
    return ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', body))


def strip_revision(text):
    """Keep the #else (REVISION 1) branch of '#if REVISION == 0' blocks."""
    out = []
    state = None
    for line in text.split('\n'):
        s = line.strip()
        if s.startswith('#if REVISION'):
            state = 'skip'
            continue
        if s.startswith('#else') and state:
            state = 'keep'
            continue
        if s.startswith('#endif') and state:
            state = None
            continue
        if state == 'skip':
            continue
        out.append(line)
    return '\n'.join(out)


def flow(raw):
    """Join the game's lines into one line of words."""
    lines = raw.split('\\n')
    text = ''
    for line in lines:
        line = line.strip()
        if not line:
            continue
        if not text:
            text = line
        elif text.endswith('-') and len(text) > 1 and text[-2] != ' ':
            text += line          # "nutrient-" + "rich"
        else:
            text += ' ' + line
    text = re.sub(r'\s+', ' ', text).strip()
    return text.replace('POK' + ACCENT + 'MON', 'MO' + ACCENT + 'MON')


def key(name):
    return re.sub(r'[^a-z0-9]', '', name.lower())


def parse_ruby(path):
    text = open(path, encoding='utf-8').read()
    ruby, sapph = {}, {}
    branch = None
    for m in re.finditer(r'(#ifdef SAPPHIRE|#else|#endif)|static const u8 DexDescription_(\w+)_([12])\[\] = _\((.*?)\);', text, re.S):
        if m.group(1):
            branch = {'#ifdef SAPPHIRE': 'S', '#else': 'R', '#endif': None}[m.group(1)]
            continue
        name, page, body = m.group(2), m.group(3), m.group(4)
        s = c_strings(body)
        for d, b in ((ruby, 'R'), (sapph, 'S')):
            if branch is None or branch == b:
                d.setdefault(name, ['', ''])[int(page) - 1] = s
    entries = re.search(r'gPokedexEntries\[\] =(.*?)\n};', text, re.S).group(1)
    order = re.findall(r'\.descriptionPage1 = DexDescription_(\w+)_1', entries)
    return order, ruby, sapph


def parse_firered(path):
    text = strip_revision(open(path, encoding='utf-8').read())
    fr = {}
    for m in re.finditer(r'const u8 g(\w+?)PokedexText\[\] = _\((.*?)\);', text, re.S):
        fr[key(m.group(1))] = c_strings(m.group(2))
    return fr


def parse_kai(repo):
    ent = open(os.path.join(repo, 'src/data/pokemon/pokedex_entries.h'), encoding='utf-8').read()
    names = re.findall(r'\[NATIONAL_DEX_(\w+)\] =\s*\{.*?\.description = g(\w+)PokedexText,', ent, re.S)
    txt = open(os.path.join(repo, 'src/data/pokemon/pokedex_text.h'), encoding='utf-8').read()
    kai = {m.group(1): c_strings(m.group(2)) for m in re.finditer(r'const u8 g(\w+?)PokedexText\[\] = _\((.*?)\);', txt, re.S)}
    return names, kai


def c_quote(s):
    return s.replace('\\', '\\\\').replace('"', '\\"')


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    repo = sys.argv[3] if len(sys.argv) > 3 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
    order, ruby, sapph = parse_ruby(sys.argv[1])
    fr = parse_firered(sys.argv[2])
    names, kai = parse_kai(repo)
    assert names[0][0] == 'NONE' and order[0] == 'Dummy'
    names = names[1:]
    order = order[1:]
    if len(names) != 386 or len(order) != 386:
        sys.exit('expected 386 species, got %d (Kai) and %d (Ruby)' % (len(names), len(order)))

    out = []
    rows = []
    skipped = []
    longest = 0
    for i, ((const, ident), rname) in enumerate(zip(names, order), 1):
        if key(rname) != key(ident):
            sys.exit('species %d: Ruby calls it %s, Kai %s' % (i, rname, ident))
        if key(ident) not in fr:
            sys.exit('species %d (%s): no FireRed entry' % (i, ident))
        pieces = [
            ('Ruby', flow(ruby[rname][0] + '\\n' + ruby[rname][1])),
            ('Sapphire', flow(sapph[rname][0] + '\\n' + sapph[rname][1])),
            ('Emerald', flow(kai[ident])),
            ('FireRed', flow(fr[key(ident)])),
        ]
        seen = []
        keep = {}
        for game, t in pieces:
            if t in seen:
                skipped.append((i, ident, game, pieces[[p[1] for p in pieces].index(t)][0]))
                keep[game] = None
            else:
                seen.append(t)
                keep[game] = t
        total = len(' '.join(seen))
        longest = max(longest, total)
        fields = []
        for game in ('Ruby', 'Sapphire', 'FireRed'):
            if keep[game] is None:
                fields.append('NULL')
            else:
                var = 'sMpDex%s_%s' % (game, ident)
                out.append('static const u8 %s[] = _("%s");' % (var, c_quote(keep[game])))
                fields.append(var)
        rows.append('    [NATIONAL_DEX_%s] = {%s, %s, %s, %s},' % (const, fields[0], fields[1],
                    'TRUE' if keep['Emerald'] is not None else 'FALSE', fields[2]))

    path = sys.argv[4] if len(sys.argv) > 4 else os.path.join(repo, 'src/data/pokemon/mp_dex_more.h')
    with open(path, 'w', encoding='utf-8') as f:
        f.write('// Moemon Co-op: the Ruby, Sapphire and FireRed Pokedex entries (tools/moemon_dex/import_dex.py).\n')
        f.write('// Generated: do not edit by hand. Entries that repeat an earlier game word for word are NULL.\n')
        f.write('// Shown in the order Ruby, Sapphire, Emerald (gPokedexEntries[].description, when "emerald"), FireRed.\n\n')
        f.write('#define MP_DEX_MORE_MAX_LEN %d // the longest combined text, in characters\n\n' % longest)
        f.write('\n'.join(out))
        f.write('\n\nstatic const struct MpDexMore sMpDexMore[NATIONAL_DEX_COUNT + 1] =\n{\n')
        f.write('\n'.join(rows))
        f.write('\n};\n')
    print('wrote %s (longest text %d characters)' % (path, longest))
    print('%d entries left out as word-for-word repeats:' % len(skipped))
    by = {}
    for i, ident, game, same in skipped:
        by.setdefault((game, same), []).append(ident)
    for (game, same), ids in sorted(by.items()):
        print('  %s = %s: %d (%s)' % (game, same, len(ids), ', '.join(ids[:8]) + (' ...' if len(ids) > 8 else '')))


if __name__ == '__main__':
    main()
