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
    if isinstance(value, tuple) and value[0] == 'mart':
        return ('%s::\n\tlock\n\tfaceplayer\n\tmessage gText_HowMayIServeYou\n\twaitmessage\n\tpokemart 0x0\n'
                '\tmsgbox gText_PleaseComeAgain, MSGBOX_DEFAULT\n\trelease\n\tend\n\n' % label, '')
    if isinstance(value, tuple) and value[0] == 'gym':
        return gym(name, key, value[1], widths)
    if isinstance(value, tuple) and value[0] == 'e4':
        return elite(name, key, value[1], widths)
    if isinstance(value, tuple) and value[0] == 'champion':
        return champion(name, key, widths)
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
