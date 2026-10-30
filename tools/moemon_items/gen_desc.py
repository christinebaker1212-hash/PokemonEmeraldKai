#!/usr/bin/env python3
"""Item descriptions from the Gen 4 games (HEARTGOLD / SOULSILVER first, then PLATINUM, then DIAMOND / PEARL).

Reads PokeAPI's tables (item_flavor_text.csv, items.csv, move_flavor_text.csv, moves.csv: github.com/PokeAPI/pokeapi,
data/v2/csv) and rewrites the texts in src/data/text/item_descriptions.h. A TM's or HM's text is the Gen 4 text of the
move it teaches here (the Gen 4 HM05 and HM08 are other moves). Items the Gen 4 games don't have (Emerald's own key
items and MAIL, the e-Reader BERRIES, items from later games, this game's own) keep their texts.

The texts are written without line breaks: the game wraps them to the window they're shown in (mp_itemdesc.c).
In the game's style: MOéMON for Pokémon, POKé for Poké, and the names of things (items, moves, stats, types, places,
Pokémon) in capitals, as Emerald writes them (a word the Gen 4 texts capitalize in the middle of a sentence is a name).

usage: tools/moemon_items/gen_desc.py CSV_DIR [--report]   (from the repo root)
"""
import csv, os, re, sys

REPO = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
DESC_H = os.path.join(REPO, 'src', 'data', 'text', 'item_descriptions.h')
ITEMS_H = os.path.join(REPO, 'src', 'data', 'items.h')
TMHM_H = os.path.join(REPO, 'include', 'constants', 'tms_hms.h')
VERSION_ORDER = ('10', '9', '8')   # HGSS, PLATINUM, DIAMOND / PEARL
ENGLISH = '9'

# Our item constants that PokeAPI spells another way
ITEM_ALIAS = {
    'SS_TICKET': 'ss-ticket', 'X_DEFEND': 'x-defense', 'X_SPECIAL': 'x-sp-atk', 'ITEMFINDER': 'dowsing-machine',
}


def load_csv(d, name):
    return list(csv.DictReader(open(os.path.join(d, name), encoding='utf-8')))


def best_text(rows_by_vg):
    for vg in VERSION_ORDER:
        if vg in rows_by_vg:
            return rows_by_vg[vg]
    return None


def flatten(t):
    t = t.replace('­', '').replace('\f', '\n')
    t = re.sub(r'-\n', '-', t)          # (a word broken at a hyphen)
    t = re.sub(r'\s*\n\s*', ' ', t)
    return re.sub(r'  +', ' ', t).strip()


ABBREVIATIONS = {'Sp', 'Exp', 'Co', 'Mr', 'Ms', 'Dr', 'St', 'Spd', 'Atk', 'Def', 'S'}
WORD = re.compile(r"[A-Za-zé][A-Za-zé]*(?:\.[A-Za-z]+)*\.?")


def sentence_starts(t):
    """Indices where a sentence starts (a word there is capitalized anyway)."""
    starts = {0}
    for m in re.finditer(r'[.!?]["”’\']?\s+', t):
        before = re.search(r'(\w+)[.!?]\s+$', t[:m.end()])
        if before and before.group(1) in ABBREVIATIONS:
            continue   # ("Sp. Atk", "Exp. Share", "Silph Co. "...: not a sentence's end)
        starts.add(m.end())
    return starts


def names_in(t, vocab):
    starts = sentence_starts(t)
    for m in WORD.finditer(t):
        w = m.group(0)
        if w[0].isupper() and m.start() not in starts and not w.isupper():
            vocab.add(w.rstrip('.') if not w.startswith('Sp.') else w)


def game_style(t, vocab):
    t = t.replace('’', "'").replace('‘', "'").replace('—', '-').replace('–', '-')
    t = re.sub(r'Pokémon', 'MOéMON', t)
    t = re.sub(r'Poké', 'POKé', t)
    out = []
    last = 0
    for m in WORD.finditer(t):
        w = m.group(0)
        core = w.rstrip('.') if not w.startswith('Sp.') else w
        if w[0].isupper() and (core in vocab or w.startswith('POKé')):
            w = ''.join(c if c == 'é' else c.upper() for c in w)
        out.append(t[last:m.start()])
        out.append(w)
        last = m.end()
    out.append(t[last:])
    t = ''.join(out)
    # (a name broken over the hyphen: "Fire-type" -> "FIRE-type", as Emerald writes types)
    return t


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    d = sys.argv[1]
    report = '--report' in sys.argv
    items = {r['id']: r['identifier'] for r in load_csv(d, 'items.csv')}
    item_text = {}
    for r in load_csv(d, 'item_flavor_text.csv'):
        if r['language_id'] == ENGLISH and r['version_group_id'] in VERSION_ORDER:
            item_text.setdefault(items[r['item_id']], {})[r['version_group_id']] = r['flavor_text']
    moves = {r['id']: r['identifier'] for r in load_csv(d, 'moves.csv')}
    move_text = {}
    for r in load_csv(d, 'move_flavor_text.csv'):
        if r['language_id'] == ENGLISH and r['version_group_id'] in VERSION_ORDER:
            move_text.setdefault(moves[r['move_id']], {})[r['version_group_id']] = r['flavor_text']

    # our TMs / HMs and the moves they teach
    tmhm = re.findall(r'F\(([A-Z_0-9]+)\)', open(TMHM_H).read())
    assert len(tmhm) == 58, len(tmhm)

    # items -> their description variable
    src = open(ITEMS_H).read()
    item_desc = re.findall(r'\[ITEM_([A-Z0-9_]+)\]\s*=\s*\{[^{}]*?\.description\s*=\s*(\w+)', src, re.S)
    by_var = {}
    for item, var in item_desc:
        by_var.setdefault(var, []).append(item)

    def gen4_for(item):
        m = re.match(r'(TM|HM)_([A-Z_]+)$', item)
        if m and m.group(2) in tmhm:   # (not TM_CASE)
            ident = m.group(2).lower().replace('_', '-')
            t = best_text(move_text.get(ident, {}))
            assert t, 'no Gen 4 text for the move ' + ident
            return t, 'move ' + ident
        ident = ITEM_ALIAS.get(item, item.lower().replace('_', '-'))
        if ident is None:
            return None, None
        t = best_text(item_text.get(ident, {}))
        return t, ident

    chosen = {}
    missing = []
    for var, its in by_var.items():
        texts = {}
        for it in its:
            t, ident = gen4_for(it)
            if t:
                texts[it] = (flatten(t), ident)
        if not texts:
            missing.append((var, its))
            continue
        vals = {v[0] for v in texts.values()}
        if len(vals) > 1:
            sys.exit('%s: items with different Gen 4 texts: %s' % (var, texts))
        chosen[var] = vals.pop()

    vocab = set()
    for t in chosen.values():
        names_in(t, vocab)
    vocab -= {'I'}

    text = open(DESC_H, encoding='utf-8').read()
    decl = re.compile(r'(static const u8 (\w+)\[\] = _\()(\s*(?:"(?:[^"\\]|\\.)*"\s*)+)(\);)')
    charmap = set()
    for line in open(os.path.join(REPO, 'charmap.txt'), encoding='utf-8'):
        m = re.match(r"'(.)'\s*=", line)
        if m:
            charmap.add(m.group(1))
    charmap |= {' ', "'"}   # (charmap.txt writes it '\\'')
    changed = 0

    def repl(m):
        nonlocal changed
        var = m.group(2)
        old = ''.join(re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(3)))
        if var in chosen:
            new = game_style(chosen[var], vocab)
        else:
            # (no Gen 4 text: the old one, its line breaks left to the game's wrapping)
            new = re.sub(r'\s*\\n\s*', ' ', old).strip()
        bad = {c for c in re.sub(r'\{[A-Z_ ]+\}', '', new) if c not in charmap}   # ({POKEBLOCK}: the game's own)
        if bad:
            sys.exit('%s: characters the game has no glyph for: %r in %r' % (var, bad, new))
        if new != old:
            changed += 1
        return '%s\n    "%s"%s' % (m.group(1), new.replace('"', '\\"'), m.group(4))

    text = decl.sub(repl, text)
    open(DESC_H, 'w', encoding='utf-8').write(text)
    print('%d descriptions from the Gen 4 games, %d kept, %d changed' % (len(chosen), len(missing), changed))
    if report:
        print('names (capitals):', ' '.join(sorted(vocab)))
        for var, its in sorted(missing):
            print('kept: %s (%s)' % (var, ', '.join(its)))


if __name__ == '__main__':
    main()
