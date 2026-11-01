#!/usr/bin/env python3
"""SINNOH's words (tools/sinnoh/words.py) as each map's scripts: a script and a text for every person and sign written
there, wrapped to the message box by the game's own letter widths (src/fonts.c, charmap.txt). The part between the
markers in data/maps/<map>/scripts.inc is this script's; anything else in the file (the map's own scripts, a story
scene) stays. Then run gen_maps.py so each person / sign gets its script.

    python3 -I tools/sinnoh/gen_words.py [POKEPLATINUM_DIR]     (run from the repository; safe to run again)

Given Platinum's events (POKEPLATINUM_DIR), every MOéMON CENTER nurse heals and every MART clerk sells, on any map
words.py doesn't already give them words (roles()).
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from words import WORDS  # noqa: E402

BEGIN = '@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---\n'
END = '@ --- end of SINNOH words ---\n'
WIDTH = 200            # pixels a line of the field message box holds (216, a margin kept)


def letter_widths():
    """{character: width in pixels} for the normal font"""
    src = open('src/fonts.c').read()
    m = re.search(r'gFontNormalLatinGlyphWidths\[\] = \{(.*?)\};', src, re.S)
    widths = [int(x) for x in re.findall(r'\d+', m.group(1))]
    out = {}
    for line in open('charmap.txt', encoding='utf-8'):
        c = re.match(r"^'(.)'\s*=\s*([0-9A-Fa-f]{2})\s*$", line.rstrip('\n'))
        if c and c.group(1) not in out:
            out[c.group(1)] = widths[int(c.group(2), 16)]
    out["'"] = out.get('’', out.get("'", 3))
    return out


def curly(text):
    """Straight quotes as the game's opening and closing ones (its character set has no straight double quote)"""
    out, open_ = [], True
    for ch in text:
        if ch == '"':
            out.append('\u201c' if open_ else '\u201d')
            open_ = not open_
        else:
            out.append(ch)
    return ''.join(out)


def wrap(text, widths, where):
    """A text as the game's string: pages ('|') of lines no wider than WIDTH; the first line of a page, then '\\n',
    then '\\l' (the box scrolls) for each line after"""
    text = curly(text)
    pages = []
    for page in text.split('|'):
        lines, cur = [], ''
        for word in page.split():
            cand = word if not cur else cur + ' ' + word
            if sum(widths.get(ch, 6) for ch in cand) <= WIDTH:
                cur = cand
            else:
                if not cur:
                    sys.exit('%s: "%s" is wider than a line' % (where, word))
                lines.append(cur)
                cur = word
        if cur:
            lines.append(cur)
        pages.append(lines)
    out = []
    for p, lines in enumerate(pages):
        for i, line in enumerate(lines):
            last = p == len(pages) - 1 and i == len(lines) - 1
            end = '$' if last else ('\\p' if i == len(lines) - 1 else ('\\n' if i == 0 else '\\l'))
            out.append('\t.string "%s%s"\n' % (line, end))
    return ''.join(out)


def script(name, key, value, widths):
    label = '%s_EventScript_%s' % (name, key)
    local = int(re.sub(r'\D', '', key) or 0)
    if isinstance(value, tuple) and value[0] == 'nurse':
        return ('%s::\n\tsetvar VAR_0x800B, %d\n\tcall Common_EventScript_PkmnCenterNurse\n\twaitmessage\n'
                '\twaitbuttonpress\n\trelease\n\tend\n\n' % (label, local), '')
    if isinstance(value, tuple) and value[0] == 'mart':
        return ('%s::\n\tlock\n\tfaceplayer\n\tmessage gText_HowMayIServeYou\n\twaitmessage\n\tpokemart 0x0\n'
                '\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT\n\trelease\n\tend\n\n' % label, '')
    text = '%s_Text_%s' % (name, key)
    kind = 'MSGBOX_NPC' if key.startswith('Person') else 'MSGBOX_SIGN'
    return ('%s::\n\tmsgbox %s, %s\n\tend\n\n' % (label, text, kind),
            '%s:\n%s\n' % (text, wrap(value, widths, '%s %s' % (name, key))))


def roles(root):
    """{map: {Person<n>: role}} for the people every map of their kind has: nurses and MART clerks"""
    from source import open_source
    from gen_maps import places
    from places import events
    out = {}
    for p in places(open_source(root), root):
        for i, o in enumerate(events(root, p['place'])['object_events']):
            gfx = o['graphics_id']
            if gfx.endswith('POKECENTER_NURSE'):
                out.setdefault(p['name'], {})['Person%d' % (i + 1)] = ('nurse',)
            elif gfx.endswith(('CASHIER_F', 'CASHIER_M')) and p['name'].endswith('Mart'):
                out.setdefault(p['name'], {})['Person%d' % (i + 1)] = ('mart',)
    return out


def main():
    widths = letter_widths()
    done = 0
    words = {k: dict(v) for k, v in WORDS.items()}
    if len(sys.argv) > 1:
        for name, found in roles(sys.argv[1]).items():
            for key, role in found.items():
                words.setdefault(name, {}).setdefault(key, role)
    for name, entries in words.items():
        path = 'data/maps/%s/scripts.inc' % name
        if not os.path.exists(path):
            sys.exit('%s: no such map' % name)
        old = open(path, encoding='utf-8').read()
        if BEGIN in old:
            old = old[:old.index(BEGIN)] + old[old.index(END) + len(END):]
        scripts, texts = '', ''
        for key in sorted(entries, key=lambda k: (re.sub(r'\d', '', k), int(re.sub(r'\D', '', k) or 0))):
            s, t = script(name, key, entries[key], widths)
            scripts += s
            texts += t
        new = old.rstrip('\n') + '\n\n' + BEGIN + scripts + texts + END
        open(path, 'w', encoding='utf-8').write(new)
        done += len(entries)
    print('%d maps, %d people and signs with words' % (len(words), done))


if __name__ == '__main__':
    main()
