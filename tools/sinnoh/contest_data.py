"""SUPER CONTESTS' tables (src/sinnoh/s3d_contest.c) as Platinum has them, read from the decomp when the game is built and
emitted into sinnoh/platinum_scripts.inc by gen_scripts.py (never committed; without the decomp the tables are empty
and the hall's receptionists turn people away):

gSinnohContestBanks: u16 x 8, the text banks the contest reads by number (CB_* in s3d_contest.c's order)
gSinnohContestMoves: u8 per move up to 467 (Platinum's moves, the same numbers here): contest type (0-4: COOL, BEAUTY,
    CUTE, SMART, TOUGH) | effect << 3 (CONTEST_EFFECT_*, generated/move_contest_effects.txt's order); 0xFF: none
gSinnohContestEffects: 24 x {s8 appeal (10 = a heart), u8 pad, u16 the two lines of its description (contest_effects),
    then 5 x {u16 message (contest_text, 0xFFFF none), u16 its format}} (src/unk_02094EDC.c Unk_020F568C)
gSinnohContestAi: u16 count, then count x {u8 performer place + 1, u8 condition, u8 target, s8 judge rule,
    s16 weights[4]} (src/overlay017/ov17_02246ECC.c Unk_ov17_02253C30: how computer contestants choose in the ACTING
    round)
gSinnohContestOpponents: u16 count, then count x 38 bytes {u16 species, u16 moves[4], u16 overworld sprite (this
    game's), u16 nickname, u16 trainer name (contest_opponent_names), u8 cool, beauty, cute, smart, tough, sheen,
    u16 flags (bits 0-2 rank, 3-7 the types it enters, 8 official single-round, 9 practice, 10-11 game stage,
    12-13 gender, 14-15 AI level), u8 dress-up per theme[12], u8 fame} (contest_data.narc member 0)
gSinnohContestJudges: u16 count, then count x {u16 name (contest_judge_names), u16 flags (2 bits per type: 1 judges,
    2 the special judge; bits 10-11 rank)} (member 1)
gSinnohContestDressUps: u16 count, then count x 84 bytes: 20 x {u8 accessory, u8 x, u8 y, u8 layer}, u8 count,
    s8 the POKeMON's place, s8 backdrop, u8 pad (member 2: the opponents' dress-ups)
gSinnohContestThemes: 12 x 100 bytes, each accessory's points for each VISUAL theme (members 3-14)
"""
import json
import os
import re
import struct

import tv_data

BANKS = ('contest_text', 'contest_effects', 'contest_acting_competition', 'contest_visual_competition',
         'contest_dance_competition', 'contest_results', 'contest_judge_names', 'contest_opponent_names')
MOVES = 468
TYPES = ('CONTEST_TYPE_COOL', 'CONTEST_TYPE_BEAUTY', 'CONTEST_TYPE_CUTE', 'CONTEST_TYPE_SMART', 'CONTEST_TYPE_TOUGH')


def _words(label, values, kind='2byte'):
    out = ['%s::' % label]
    for k in range(0, len(values), 12):
        out.append('\t.%s ' % kind + ', '.join(str(v) for v in values[k:k + 12]))
    return out


def _initializer(text, name):
    """the brace tree of `name[...] = { ... };` in a C file, as nested lists of token strings"""
    m = re.search(r'\b%s\s*\[[^\]]*\]\s*=\s*\{' % re.escape(name), text)
    if not m:
        raise ValueError('%s not found' % name)
    i = m.end()
    stack, cur, tok = [], [], ''
    while True:
        c = text[i]
        if c == '/' and text[i + 1] == '/':
            i = text.index('\n', i)
            continue
        if c == '/' and text[i + 1] == '*':
            i = text.index('*/', i) + 2
            continue
        if c in '{},':
            if tok.strip():
                cur.append(tok.strip())
            tok = ''
            if c == '{':
                stack.append(cur)
                cur = []
            elif c == '}':
                if not stack:
                    return cur
                done, cur = cur, stack.pop()
                cur.append(done)
        else:
            tok += c
        i += 1


def _num(tok, names=None):
    tok = tok.strip()
    m = re.match(r'^\[\w+\]\s*=\s*(.*)$', tok)
    if m:
        tok = m.group(1)
    if names and tok in names:
        return names[tok]
    v = int(tok, 0)
    return v - (1 << 64) if v >= 1 << 63 else v


def _lines_of(path):
    return [x.strip() for x in open(path) if x.strip()]


def move_lines(root):
    names = _lines_of(os.path.join(root, 'generated/moves.txt'))
    effects = {n: i for i, n in enumerate(_lines_of(os.path.join(root, 'generated/move_contest_effects.txt')))}
    out = [0xFF] * MOVES
    for i, n in enumerate(names[:MOVES]):
        p = os.path.join(root, 'res/moves', n[len('MOVE_'):].lower(), 'data.json')
        if i == 0 or not os.path.exists(p):
            continue
        c = json.load(open(p)).get('contest', {})
        if c.get('type') in TYPES and c.get('effect') in effects:
            out[i] = TYPES.index(c['type']) | effects[c['effect']] << 3
    return _words('gSinnohContestMoves', out, 'byte')


def effect_lines(root, dec):
    text = open(os.path.join(root, 'src/unk_02094EDC.c')).read()
    rows = [r for r in _initializer(text, 'Unk_020F568C') if isinstance(r, list)]
    eff = dec.ids('contest_effects') if 'contest_effects' in dec.banks else {}
    out = []
    for r in rows:
        v = [_num(t, eff) if not isinstance(t, list) else 0 for t in r] + [0] * 13
        if len(r) <= 3:   # (CONTEST_EFFECT_NONE: { 0, 0, 0 })
            v = [0, 0, 0] + [0xFFFF, 0] * 5
        out += [v[2] & 0xFF, 0]   # appeal, pad (bytes)
        out16 = [v[0] & 0xFFFF, v[1] & 0xFFFF] + [x & 0xFFFF for x in v[3:13]]
        out += [b for w in out16 for b in struct.pack('<H', w)]
    return _words('gSinnohContestEffects', out, 'byte')


def ai_lines(root):
    text = open(os.path.join(root, 'src/overlay017/ov17_02246ECC.c')).read()
    rows = [r for r in _initializer(text, 'Unk_ov17_02253C30') if isinstance(r, list)]
    out = [len(rows)]
    for r in rows:
        head = [_num(t) for t in r if not isinstance(t, list)]
        weights = [_num(t) for t in next((t for t in r if isinstance(t, list)), [])]
        if len(head) == 3:   # (a row missing its rule: the compiler took the weights' first for it, the rest 0)
            head, weights = head + [weights[0]], []
        weights = (weights + [0, 0, 0, 0])[:4]
        out += [head[0] | head[1] << 8, head[2] | (head[3] & 0xFF) << 8] + [w & 0xFFFF for w in weights]
    return _words('gSinnohContestAi', out)


def narc_lines(root):
    import sprites
    m = tv_data.narc_members(os.path.join(root, 'res/prebuilt/contest/data/contest_data.narc'))
    species = _lines_of(os.path.join(root, 'generated/species.txt'))
    moves = _lines_of(os.path.join(root, 'generated/moves.txt'))
    gfx = _lines_of(os.path.join(root, 'generated/object_events_gfx.txt'))
    ours = set()
    for h in ('species.h', 'moves.h'):
        ours |= set(re.findall(r'^#define\s+((?:SPECIES|MOVE)_\w+)\b',
                               open(os.path.join(os.path.dirname(__file__), '../../include/constants', h),
                                    errors='replace').read(), re.M))
    opp = m[0]
    n = len(opp) // 48
    out = ['gSinnohContestOpponents::', '\t.2byte %d' % n]
    for i in range(n):
        r = opp[i * 48:(i + 1) * 48]
        _, _, obj, _ = struct.unpack_from('<IIHH', r)
        mv = struct.unpack_from('<4H', r, 12)
        sp, nick, ot = struct.unpack_from('<HHH', r, 20)
        cond = r[26:32]
        flags = struct.unpack_from('<H', r, 32)[0]
        dress = r[34:46]
        fame = r[46]
        s = species[sp] if sp < len(species) and species[sp] in ours else 'SPECIES_NONE'
        mm = [moves[x] if x < len(moves) and moves[x] in ours else 'MOVE_NONE' for x in mv]
        g = gfx[obj] if obj < len(gfx) else ''
        try:
            g = sprites.sprite(g) or 'OBJ_EVENT_GFX_BOY_1'
        except Exception:
            g = 'OBJ_EVENT_GFX_BOY_1'
        out.append('\t.2byte %s, %s, %s, %d, %d' % (s, ', '.join(mm), g, nick, ot))
        out.append('\t.byte ' + ', '.join(str(x) for x in cond))
        out.append('\t.2byte %d' % flags)
        out.append('\t.byte ' + ', '.join(str(x) for x in dress) + ', %d, 0' % fame)
    j = m[1]
    out += ['\t.balign 2', 'gSinnohContestJudges::', '\t.2byte %d' % (len(j) // 8)]
    for i in range(len(j) // 8):
        name, _, flags = struct.unpack_from('<HHH', j, i * 8)
        out.append('\t.2byte %d, %d' % (name, flags))
    d = m[2]
    out += ['gSinnohContestDressUps::', '\t.2byte %d' % (len(d) // 84)]
    out += _words('', list(d), 'byte')[1:]
    themes = b''.join(bytes(x[:100]).ljust(100, b'\0') for x in m[3:15])
    out += _words('gSinnohContestThemes', list(themes.ljust(1200, b'\0')), 'byte')
    return out


def lines(dec, root):
    """the tables, or stubs where a part of the decomp is missing"""
    out = ['\t.balign 2']
    out += _words('gSinnohContestBanks', [dec.banks.get(b, 0xFFFF) for b in BANKS])
    try:
        out += move_lines(root)
    except (OSError, ValueError):
        out += _words('gSinnohContestMoves', [0xFF] * MOVES, 'byte')
    out.append('\t.balign 2')
    try:
        out += effect_lines(root, dec)
    except (OSError, ValueError):
        out += _words('gSinnohContestEffects', [0] * (24 * 26), 'byte')
    out.append('\t.balign 2')
    try:
        out += ai_lines(root)
    except (OSError, ValueError):
        out += ['gSinnohContestAi::', '\t.2byte 0']
    out.append('\t.balign 2')
    try:
        out += narc_lines(root)
    except (OSError, ValueError):
        out += stub_narc()
    try:
        out += mining_lines(root)
    except (OSError, ValueError, KeyError, AttributeError):
        out += mining_stub()
    return out


def stub_narc():
    return ['gSinnohContestOpponents::', '\t.2byte 0', 'gSinnohContestJudges::', '\t.2byte 0',
            'gSinnohContestDressUps::', '\t.2byte 0'] + _words('gSinnohContestThemes', [0] * 1200, 'byte')


def stub_lines():
    return (['\t.balign 2'] + _words('gSinnohContestBanks', [0xFFFF] * len(BANKS))
            + _words('gSinnohContestMoves', [0xFF] * MOVES, 'byte') + ['\t.balign 2']
            + _words('gSinnohContestEffects', [0] * (24 * 26), 'byte') + ['\t.balign 2', 'gSinnohContestAi::', '\t.2byte 0']
            + stub_narc() + mining_stub())


# ---------------------------------------------------------------------------------------------- the Underground's digging
# gSinnohMiningObjects (src/sinnoh/s3d_mining.c): u16 count, then count x {u16 item (this game's; 0xFFFF a rock),
# u8 width, u8 height (cells), u8 weights[4] (odd Trainer ID, even, odd with the NATIONAL DEX, even with it),
# u8 shape[36] (6 x 6, 1 where it is)} from src/underground/mining.c's sMiningObjects. Spheres are left out (the
# Underground here digs for treasure and fossils only: the user, 2026-11-02).
def mining_objects(root):
    """sMiningObjects as dicts (item: this game's ITEM_ name or None for a rock; w, h in cells; weights; grid: 6 x 6;
    sprite, palette: the NCGR / NCLR names), spheres and items this game lacks left out (build_mining.py: the art in the
    same order)."""
    text = open(os.path.join(root, 'src/underground/mining.c')).read()
    shapes = {}
    for name, body in re.findall(r'static u8 (s\w+Shape\w*)\[\d+\]\[\d+\] = \{(.*?)\};', text, re.S):
        shapes[name] = [x for x in re.findall(r"'(.)'", body)]
    m = re.search(r'sMiningObjects\[\] = \{(.*?)\n\};', text, re.S)
    rows = re.findall(r'\{ (\.itemID = .*?) \}', m.group(1))
    ours = set(re.findall(r'^#define\s+(ITEM_\w+)\b', open(os.path.join(os.path.dirname(__file__), '../../include/constants/items.h'), errors='replace').read(), re.M))
    out = []
    for r in rows:
        f = dict(re.findall(r'\.(\w+) = ([^,]+)', r))
        item = f['itemID'].strip()
        if 'SPHERE' in item:
            continue
        if item.startswith('MINING_ROCK'):
            name = None
        else:
            name = 'ITEM_' + item[len('MINING_TREASURE_'):]
            if name not in ours:
                continue
        w = int(eval(f['width'])) // 2
        h = int(eval(f['height'])) // 2
        shape = [1] * (w * h)
        if f['shape'].strip() != 'NULL':
            s = shapes[f['shape'].strip()]
            shape = [0 if c == 'o' else 1 for c in s[:w * h]]
        grid = [0] * 36
        for y in range(h):
            for x in range(w):
                grid[y * 6 + x] = shape[y * w + x]
        weights = [int(f[k]) for k in ('oddTIDWeight', 'evenTIDWeight', 'oddTIDNatDexWeight', 'evenTIDNatDexWeight')]
        out.append({'item': name, 'w': w, 'h': h, 'weights': weights, 'grid': grid,
                    'sprite': f.get('spriteNARCIndex', '').strip(), 'palette': f.get('paletteNARCIndex', '').strip()})
    return out


def mining_lines(root):
    out = []
    for o in mining_objects(root):
        out.append('\t.2byte %s' % (o['item'] or '0xFFFF'))
        out.append('\t.byte %d, %d, %s' % (o['w'], o['h'], ', '.join(str(x) for x in o['weights'])))
        out.append('\t.byte ' + ', '.join(str(x) for x in o['grid']))
    # the flags Platinum's Underground sets and the UNDERGROUND MAN's script reads (s3d_underground.c): gone down there,
    # a treasure dug up
    import ids
    flags = [ids.flag(root, f) or 0 for f in ('FLAG_ENTERED_UNDERGROUND', 'FLAG_DIGGING_FOR_FOSSILS')]
    return (['\t.balign 2', 'gSinnohMiningObjects::', '\t.2byte %d' % (len(out) // 3)] + out
            + ['\t.balign 2', 'gSinnohUndergroundFlags::', '\t.2byte %s' % ', '.join('0x%X' % f for f in flags)]
            # (Platinum's FLAG_GAME_COMPLETED, kept set while SINNOH's LEAGUE is beaten: s3d_scrcmd.c)
            + ['gSinnohGameCompletedFlag::', '\t.2byte 0x%X' % (ids.flag(root, 'FLAG_GAME_COMPLETED') or 0)])


def mining_stub():
    return ['\t.balign 2', 'gSinnohMiningObjects::', '\t.2byte 0', 'gSinnohUndergroundFlags::', '\t.2byte 0, 0',
            'gSinnohGameCompletedFlag::', '\t.2byte 0']
