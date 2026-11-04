#!/usr/bin/env python3
"""JOHTO's TM gifts where Crystal has them (docs/REGIONS_TODO.md A4: the other TM givers; the leaders' are
fix_leader_tms.py). Quetzal gave held items and stones in their places.

    python3 tools/quetzal_port/fix_johto_tms.py      (needs the network; safe to run again)

Each person Crystal gives a TM whose move is a TM in this game too (TM01-TM92: Gen 3 and 4's list) gives that TM,
with Crystal's words before and after it (pret/pokecrystal maps/<map>.asm, the writetext around verbosegiveitem),
rewrapped for the message box (#MON as MOéMON). Crystal's TMs whose moves aren't TMs here (NIGHTMARE, SWEET SCENT,
HEADBUTT, ZAP CANNON...) leave Quetzal's gift as it is. Crystal's TM item balls stay Quetzal's items: JOHTO's maps
are Quetzal's redraws, whose balls don't stand where Crystal's did.
What Quetzal gave there stays obtainable elsewhere (tools/moemon_gen59/item_audit.py).
"""
import os
import re
import urllib.request

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
RAW = 'https://raw.githubusercontent.com/pret/pokecrystal/master/maps/%s.asm'
WIDTH = 34
KEEP_HYPHEN = {'type', 'types', 'up', 'out'}

GIFTS = {   # JOHTO map (also Crystal's): (Quetzal's item, Crystal's TM, this game's TM)
    'LakeOfRageHiddenPowerHouse': ('ITEM_DAWN_STONE', 'TM_HIDDEN_POWER', 'ITEM_TM10'),
    'RadioTower3F': ('ITEM_LIFE_ORB', 'TM_SUNNY_DAY', 'ITEM_TM11'),
    'Route27SandstormHouse': ('ITEM_SMOOTH_ROCK', 'TM_SANDSTORM', 'ITEM_TM37'),
    'Route28SteelWingHouse': ('ITEM_METAL_COAT', 'TM_STEEL_WING', 'ITEM_TM47'),
    'Route32': ('ITEM_LEFTOVERS', 'TM_ROAR', 'ITEM_TM05'),
    'Route43Gate': ('ITEM_BLACK_SLUDGE', 'TM_SLUDGE_BOMB', 'ITEM_TM36'),
}
MARK = "@ Moemon Kai: Crystal's TM (tools/quetzal_port/fix_johto_tms.py)"


def crystal_text(asm, label):
    """A Crystal text label as paragraphs of words"""
    m = re.search(r'^' + re.escape(label) + r'::?\n((?:\t(?:text|line|cont|para|done|prompt)[^\n]*\n|\n)+)', asm, re.M)
    pars, cur = [], ''
    for cmd, arg in re.findall(r'\t(text|line|cont|para|done|prompt)(?: "([^"]*)")?', m.group(1)):
        if cmd in ('done', 'prompt'):
            break
        if cmd == 'para' and cur and (cur.rstrip('"\'').endswith(('.', '!', '?', '…'))
                                      or not any(c.islower() for c in cur)):
            pars.append(cur)   # (a new page where a sentence or a heading such as TRAINER TIPS ends; others flow on)
            cur = ''
        if cur.endswith('- '):
            cur = cur[:-1]   # ("cam- " / "paign": a split word with a space after its hyphen)
        if cur.endswith('-') and arg and arg.split()[0].lower().strip('.,!?') in KEEP_HYPHEN:
            cur = cur + arg   # (fire-type: a hyphenated word at the line's end)
        elif cur.endswith('-') and arg and (arg[0].islower() or cur.endswith('#-')):
            cur = cur[:-1] + arg   # (a word split over two lines)
        else:
            cur = (cur + ' ' + arg) if cur else arg
    if cur:
        pars.append(cur)
    return [p.replace('#MON', 'MOéMON').replace('#', 'POKé').replace("'", '’').replace('"', '\\"') for p in pars]


def wrap(par):
    lines, cur = [], ''
    for w in par.split():
        if cur and len(cur) + 1 + len(w) > WIDTH:
            lines.append(cur)
            cur = w
        else:
            cur = (cur + ' ' + w) if cur else w
    return lines + ([cur] if cur else [])


def gba_text(pars):
    out = []
    for p in pars:
        ls = wrap(p)
        for k, l in enumerate(ls):
            out.append([l, '' if k == len(ls) - 1 else ('\\n' if k == 0 else '\\l')])
        out[-1][1] = '\\p'
    out[-1][1] = '$'
    return ''.join('\t.string "%s%s"\n' % (l, sep) for l, sep in out)


def around(asm, tm):
    """Crystal's writetext before and after its verbosegiveitem"""
    i = asm.index('verbosegiveitem ' + tm)
    before = re.findall(r'writetext (\w+)', asm[:i])[-1]
    after = re.match(r'(?:\t(?:iffalse|iftrue|setevent|setflag|promptbutton)[^\n]*\n|\.\w+:\n)*\twritetext (\w+)', asm[i:].split('\n', 1)[1])
    return before, after.group(1) if after else None


def main():
    for name, (old, ctm, tm) in GIFTS.items():
        path = os.path.join(REPO, 'data/maps/Johto_%s/scripts.inc' % name)
        s = open(path).read()
        if MARK in s:
            print(name, 'done already')
            continue
        asm = urllib.request.urlopen(RAW % name).read().decode('utf-8')
        before, after = around(asm, ctm)
        give = '\tsetorcopyvar VAR_0x8000, %s\n' % old
        if give not in s:
            raise SystemExit('%s: no gift of %s' % (name, old))
        i = s.index(give)
        head = s[:i].split('\n')[-6:]   # the words before (the last msgbox above it)
        intro = [re.match(r'\tmsgbox (\w+), MSGBOX_DEFAULT', l) for l in head]
        intro = [x.group(1) for x in intro if x][-1]
        tail = s[i:].split('\n')[1:7]   # and after (the first msgbox below it)
        outro = [re.match(r'\tmsgbox (\w+), MSGBOX_DEFAULT', l) for l in tail]
        outro = ([x.group(1) for x in outro if x] or [None])[0]
        s = s[:i] + '\t%s (Quetzal gave %s)\n\tsetorcopyvar VAR_0x8000, %s\n' % (
            MARK, old.replace('ITEM_', ''), tm) + s[i + len(give):]
        texts = [(intro, before)] + ([(outro, after)] if outro and after else [])
        if outro and not after:
            print('  %s: Quetzal\'s words after the gift (%s) kept' % (name, outro))
        for label, clabel in texts:
            t = re.search(re.escape(label) + r'::\n((?:\t\.string .*\n)+)', s)
            s = s[:t.start(1)] + gba_text(crystal_text(asm, clabel)) + s[t.end(1):]
        open(path, 'w').write(s)
        print(name, tm, 'for', old, '(words:', ', '.join(c for _, c in texts) + ')')


if __name__ == '__main__':
    main()
