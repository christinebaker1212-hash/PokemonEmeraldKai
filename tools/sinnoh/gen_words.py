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
from words import WORDS, GYMS, ELITE, CHAMPION  # noqa: E402

BADGE_FLAGS = 0x3320       # FLAG_SINNOH_BADGE01 (include/mp_region.h)
GYM_TM_FLAGS = 0x3770      # a leader's TM given (ids.py's flags stay below)

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
    if isinstance(value, tuple) and value[0] == 'martlist':
        return ('%s::\n\tlock\n\tfaceplayer\n\tmessage gText_HowMayIServeYou\n\twaitmessage\n\tpokemart %s\n'
                '\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT\n\trelease\n\tend\n\n' % (label, value[1]), '')
    if isinstance(value, tuple) and value[0] == 'mart':
        return ('%s::\n\tlock\n\tfaceplayer\n\tmessage gText_HowMayIServeYou\n\twaitmessage\n\tpokemart 0x0\n'
                '\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT\n\trelease\n\tend\n\n' % label, '')
    if isinstance(value, tuple) and value[0] == 'gym':
        return gym(name, key, value[1], widths)
    if isinstance(value, tuple) and value[0] == 'e4':
        return elite(name, key, value[1], widths)
    if isinstance(value, tuple) and value[0] == 'champion':
        return champion(name, key, widths)
    if isinstance(value, tuple) and value[0] == 'script':
        return '%s::\n\tgoto %s\n\tend\n\n' % (label, value[1]), ''
    if isinstance(value, tuple) and value[0] == 'boat':
        return boat(name, key, value[1], widths)
    text = '%s_Text_%s' % (name, key)
    kind = 'MSGBOX_NPC' if key.startswith('Person') else 'MSGBOX_SIGN'
    return ('%s::\n\tmsgbox %s, %s\n\tend\n\n' % (label, text, kind),
            '%s:\n%s\n' % (text, wrap(value, widths, '%s %s' % (name, key))))


def gym(name, key, n, widths):
    """A gym leader: the battle, then the badge and the TM (each once), then the after words"""
    trainer, badge, tm, intro, defeat, info, tmwords, after = GYMS[n]
    label = '%s_EventScript_%s' % (name, key)
    t = '%s_Text_%s' % (name, key)
    leader = trainer.split('_')[-1]
    code = ('%s::\n\ttrainerbattle_single %s, %sIntro, %sDefeat, %s_Defeated, NO_MUSIC\n'
            '\tgoto_if_unset 0x%X, %s_GiveTm\n\tmsgbox %sAfter, MSGBOX_DEFAULT\n\trelease\n\tend\n\n'
            '%s_Defeated::\n\tmessage %sBadge\n\twaitmessage\n\tcall Common_EventScript_PlayGymBadgeFanfare\n'
            '\tmsgbox %sBadgeInfo, MSGBOX_DEFAULT\n\tsetflag 0x%X\n\tgoto %s_GiveTm\n\tend\n\n'
            '%s_GiveTm::\n\tgiveitem %s\n\tgoto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull\n'
            '\tsetflag 0x%X\n\tmsgbox %sTm, MSGBOX_DEFAULT\n\trelease\n\tend\n\n'
            % (label, trainer, t, t, label, GYM_TM_FLAGS + n, label, t,
               label, t, t, BADGE_FLAGS + n, label,
               label, tm, GYM_TM_FLAGS + n, t))
    texts = ''
    for suffix, words in (('Intro', intro), ('Defeat', defeat), ('BadgeInfo', info), ('Tm', tmwords),
                          ('After', after), ('Badge', '{PLAYER} received the %s from %s.' % (badge, leader))):
        texts += '%s%s:\n%s\n' % (t, suffix, wrap(words, widths, '%s %s' % (name, key)))
    return code, texts


def trainer_flag(trainer):
    """A SINNOH trainer's "beaten" flag (TRAINER_FLAG in include/constants/flags.h)"""
    text = open('include/constants/trainers_sinnoh.h').read()
    first = int(re.search(r'#define FIRST_SINNOH_TRAINER (\d+)', text).group(1))
    tid = int(re.search(r'#define %s +(\d+)' % trainer, text).group(1))
    return 0x3780 + tid - first


def elite(name, key, n, widths):
    trainer, intro, defeat, after = ELITE[n]
    label, t = '%s_EventScript_%s' % (name, key), '%s_Text_%s' % (name, key)
    code = ('%s::\n\ttrainerbattle_single %s, %sIntro, %sDefeat, %s_Defeated, NO_MUSIC\n'
            '\tmsgbox %sAfter, MSGBOX_NPC\n\tend\n\n%s_Defeated::\n\tmsgbox %sAfter, MSGBOX_DEFAULT\n\trelease\n\tend\n\n'
            % (label, trainer, t, t, label, t, label, t))
    texts = ''.join('%s%s:\n%s\n' % (t, sfx, wrap(w, widths, name)) for sfx, w in
                    (('Intro', intro), ('Defeat', defeat), ('After', after)))
    return code, texts


def champion(name, key, widths):
    """The CHAMPION: after all four, the battle; the win makes SINNOH cleared (Quetzal_EventScript_835F6E4: the
    region's record, then a save) and plays the HALL OF FAME (special GameClear), back at the LEAGUE's CENTER after"""
    trainer, intro, defeat, after = CHAMPION
    label, t = '%s_EventScript_%s' % (name, key), '%s_Text_%s' % (name, key)
    checks = ''.join('\tgoto_if_unset 0x%X, %s_NotYet\n' % (trainer_flag(e[0]), label) for e in ELITE)
    north = open('data/maps/Sinnoh_PokemonLeagueNorthPokecenter1f/scripts.inc').read()
    heal = re.search(r'setrespawn (\w+)', north).group(1)     # (the LEAGUE's own CENTER: tools/sinnoh/gen_heal.py)
    code = ('%s::\n%s\ttrainerbattle_single %s, %sIntro, %sDefeat, %s_Defeated, NO_MUSIC\n'
            '\tmsgbox %sAgain, MSGBOX_NPC\n\tend\n\n'
            '%s_NotYet::\n\tmsgbox %sNotYet, MSGBOX_NPC\n\tend\n\n'
            '%s_Defeated::\n\tmsgbox %sAfter, MSGBOX_DEFAULT\n\tcloseMessage\n'
            '\tcall Quetzal_EventScript_835F6E4\n\tsetrespawn %s\n'
            '\tfadescreenspeed 1, 24\n\tspecial GameClear\n\twaitstate\n\treleaseall\n\tend\n\n'
            % (label, checks, trainer, t, t, label, t, label, t, label, t, heal)).replace('closeMessage', 'closemessage')
    texts = ''.join('%s%s:\n%s\n' % (t, sfx, wrap(w, widths, name)) for sfx, w in
                    (('Intro', intro), ('Defeat', defeat), ('After', after),
                     ('NotYet', "Welcome to the CHAMPION's room. Come back once you've beaten all four of the "
                                "ELITE FOUR. I'll be waiting."),
                     ('Again', "That battle was one of my best. Come back and challenge me any time.")))
    return code, texts


_landing = {}


def landing(root, dest):
    """Where a boat to dest lands: a free tile beside dest's first sailor (tools/sinnoh/reach.py's collision)"""
    if dest not in _landing:
        import json
        from source import open_source
        from gen_maps import places
        import reach
        src = open_source(root)
        p = next(p for p in places(src, root) if p['name'] == dest)
        g = reach.grid(src, p)[0]
        m = json.load(open('data/maps/%s/map.json' % dest))
        sailor = next(o for o in m['object_events'] if o['graphics_id'].endswith('SAILOR'))
        x, y = sailor['x'], sailor['y']
        _landing[dest] = next((x + dx, y + dy) for dx, dy in ((0, 1), (1, 0), (-1, 0), (0, -1))
                              if reach.free(g.get((x + dx, y + dy))) and (g.get((x + dx, y + dy)) & 0xFF) not in (0x10, 0x15))
    return _landing[dest]


PLACE_NAMES = {'Sinnoh_IronIsland': 'IRON ISLAND', 'Sinnoh_CanalaveCity': 'CANALAVE CITY',
               'Sinnoh_FullmoonIsland': 'FULLMOON ISLAND', 'Sinnoh_NewmoonIsland': 'NEWMOON ISLAND',
               'Sinnoh_FightArea': 'the BATTLE ZONE', 'Sinnoh_SnowpointCity': 'SNOWPOINT CITY'}


def boat(name, key, dests, widths):
    """A sailor: asks about each place the boat goes, in turn; YES sails there"""
    label, t = '%s_EventScript_%s' % (name, key), '%s_Text_%s' % (name, key)
    code = '%s::\n\tlock\n\tfaceplayer\n' % label
    texts = ''
    for i, d in enumerate(dests):
        x, y = landing(ROOT[0], d)
        code += ('\tmsgbox %sAsk%d, MSGBOX_YESNO\n\tgoto_if_eq VAR_RESULT, YES, %s_Sail%d\n' % (t, i, label, i))
        texts += '%sAsk%d:\n%s\n' % (t, i, wrap('Shall I take you to %s?' % PLACE_NAMES[d], widths, name))
    code += '\tmsgbox %sBye, MSGBOX_DEFAULT\n\trelease\n\tend\n\n' % t
    texts += '%sBye:\n%s\n' % (t, wrap("Come back any time. The sea's always here.", widths, name))
    for i, d in enumerate(dests):
        x, y = landing(ROOT[0], d)
        code += ('%s_Sail%d::\n\tmsgbox %sOff, MSGBOX_DEFAULT\n\tcloseMessage\n\twarp MAP_%s, %d, %d\n\twaitstate\n'
                 '\treleaseall\n\tend\n\n' % (label, i, t, 'SINNOH_' + re.sub(r'(?<=[a-z0-9])([A-Z])', r'_\1', d[7:]).upper(), x, y)
                 ).replace('closeMessage', 'closemessage')
    texts += '%sOff:\n%s\n' % (t, wrap("All aboard! Hold on tight!", widths, name))
    return code, texts


ROOT = [None]


def roles(root):
    """{map: {Person<n>: role}} for the people every map of their kind has: nurses and MART clerks"""
    from source import open_source
    from gen_maps import places
    from places import events
    import sprites
    import who
    from words_indoor import kind
    from gen_marts import vendor_kind
    out = {}
    for p in places(open_source(root), root):
        ev = events(root, p['place'])
        names = who.entries(root, p['place']['header'][len('MAP_HEADER_'):].lower())

        def full(n):
            return names[n - 1] if isinstance(n, int) and 1 <= n <= len(names) else ''

        def entry(n):
            e = full(n)
            return e.split('_', 1)[1] if '_' in e else ''
        vendors = vendor_kind(root, p['place']['header'][len('MAP_HEADER_'):].lower())
        for i, o in enumerate(ev['object_events']):
            gfx = o['graphics_id']
            if gfx.endswith('POKECENTER_NURSE'):
                out.setdefault(p['name'], {})['Person%d' % (i + 1)] = ('nurse',)
            elif vendors.get(full(o['script'])):
                # a clerk: the common list or a specialty list, as Platinum's own script says (gen_marts.py)
                out.setdefault(p['name'], {})['Person%d' % (i + 1)] = vendors[full(o['script'])]
            else:
                # the things every building of a kind has, and gym guides (tools/sinnoh/words_indoor.py KINDS)
                words = kind(p['name'], entry(o['script'])) if not isinstance(o.get('hidden_flag'), str) or \
                    not o['hidden_flag'].startswith(('FLAG_', 'MAP_HEADER_')) else None
                if words:
                    key = ('Sign%d' if sprites.sign(gfx) else 'Person%d') % (i + 1)
                    out.setdefault(p['name'], {})[key] = words
        for i, b in enumerate(ev['bg_events']):
            words = kind(p['name'], entry(b['script'])) if b.get('type') == 0 else None
            if words:
                out.setdefault(p['name'], {})['Bg%d' % (i + 1)] = words
    return out


def no_twice():
    """A map written twice in WORDS would lose the first one's words (a dict keeps the last): refuse"""
    import ast
    import collections
    for f, var in (('words.py', 'WORDS'), ('words_outdoor.py', 'OUTDOOR'), ('words_indoor.py', 'INDOOR')):
        tree = ast.parse(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), f), encoding='utf-8').read())
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign) and getattr(node.targets[0], 'id', '') == var:
                twice = [k for k, n in collections.Counter(k.value for k in node.value.keys).items() if n > 1]
                if twice:
                    sys.exit('%s has these maps twice: %s' % (f, ' '.join(twice)))


def main():
    no_twice()
    widths = letter_widths()
    done = 0
    words = {k: dict(v) for k, v in WORDS.items()}
    if len(sys.argv) > 1:
        ROOT[0] = sys.argv[1]
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
