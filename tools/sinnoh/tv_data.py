"""SINNOH's TV (src/sinnoh/s3d_tv.c) as Platinum broadcasts it: its weekly schedule and its programs' segment lists
(res/prebuilt/arc/tv.narc, the two members its overlay006/ov6_022465FC.c reads), and the numbers of the messages the
TV's code names (Platinum's own message names, looked up by a hash: S3D_TextNamed). Emitted into
sinnoh/platinum_scripts.inc by gen_scripts.py when the game is built (never committed); without the decomp the tables
are empty and the TV shows nothing of its own.

gSinnohTvSchedule: TV_SLOTS bytes, a program (1-9) for each quarter hour of the week: [(hour + 19) % 24 * 4 + quarter]
    * 7 + day (Monday first), as Platinum reads it
gSinnohTvPrograms: 9 programs x 12 bytes: the program type (1-5), then up to 11 segment ids (0 ends)
gSinnohTextNames: u32 count, then count x {u32 hash, u16 bank, u16 message} sorted by hash (FNV-1a of the name)
Also Platinum's Seals, which POKé BALL effects stand in for (src/sinnoh/s3d_seals.c): gSinnohSealPrices (u16 per Seal
id, generated/seals.txt's order, from capsule_menu's sealTypeValues) and gSinnohSealShops (the SUNYSHORE MARKET's stock
for each day, Monday first: 7 x 8 Seal ids, 0 ends; include/data/mart_items.h).
"""
import re
import os
import struct

TV_SLOTS = 875   # (tv.narc's first member: 24 x 4 x 7 = 672 used, the rest Platinum's padding)
TEXT_BANKS = ('tv_programs', 'tv_commercials', 'tv_programs_interviews', 'tv_programs_records', 'tv_programs_sinnoh_now',
              'tv_programs_trainer_sightings', 'tv_programs_variety_hour', 'tv_reporter_interviews', 'poketch_app_names',
              'contest_accessory_names', 'contest_backdrop_names', 'contest_text', 'contest_common',
              'contest_judge_names', 'contest_opponent_names', 'contest_results', 'contest_effects',
              'contest_visual_competition', 'contest_dance_competition', 'contest_acting_competition',
              'contest_hall_lobby', 'contest_accessory_names_with_articles', 'link_contest_records', 'flavor_names',
              'poffin_types', 'villa_furniture_names', 'ball_seal_names', 'ribbon_names', 'underground_common', 'menu_entries',
              'rankings_machine', 'journal_entries', 'gym_names', 'month_names', 'times_of_day')


def fnv(name):
    h = 0x811C9DC5
    for ch in name.encode():
        h = ((h ^ ch) * 0x01000193) & 0xFFFFFFFF
    return h


def narc_members(path):
    d = open(path, 'rb').read()
    o = 16
    if d[:4] != b'NARC' or d[o:o + 4] != b'BTAF':
        raise ValueError('%s: not a NARC' % path)
    n = struct.unpack_from('<H', d, o + 8)[0]
    fat = [struct.unpack_from('<II', d, o + 12 + 8 * i) for i in range(n)]
    o += struct.unpack_from('<I', d, o + 4)[0]
    o += struct.unpack_from('<I', d, o + 4)[0]          # (BTNF)
    img = o + 8                                          # (GMIF)
    return [d[img + a:img + b] for a, b in fat]


def _bytes(label, data):
    out = ['%s::' % label]
    for k in range(0, len(data), 16):
        out.append('\t.byte ' + ', '.join(str(x) for x in data[k:k + 16]))
    return out


def lines(dec, root):
    narc = os.path.join(root, 'res/prebuilt/arc/tv.narc')
    if os.path.exists(narc):
        m = narc_members(narc)
        schedule = bytes(m[0][:TV_SLOTS]).ljust(TV_SLOTS, b'\0')
        programs = bytes(m[1][:9 * 12]).ljust(9 * 12, b'\0')
    else:
        schedule, programs = bytes(TV_SLOTS), bytes(9 * 12)
    rows = {}
    for bank in TEXT_BANKS:
        if bank not in dec.banks:
            continue
        for name, msg in dec.ids(bank).items():
            h = fnv(name)
            if h in rows and rows[h] != (dec.banks[bank], msg):
                raise ValueError('tv_data: two message names hash the same: %s' % name)
            rows[h] = (dec.banks[bank], msg)
    out = ['\t.balign 4'] + _bytes('gSinnohTvSchedule', schedule) + _bytes('gSinnohTvPrograms', programs)
    out += seal_lines(root)
    out += ['\t.balign 4', 'gSinnohTextNames::', '\t.4byte %d' % len(rows)]
    for h in sorted(rows):
        out.append('\t.4byte 0x%08X\n\t.2byte %d, %d' % (h, rows[h][0], rows[h][1]))
    return out


SEAL_COUNT = 82
DAYS = ('Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday')


def seal_lines(root):
    prices, shops = [0] * SEAL_COUNT, [[0] * 8 for _ in DAYS]
    names = os.path.join(root, 'generated/seals.txt')
    menu = os.path.join(root, 'src/applications/capsule_menu/main.c')
    marts = os.path.join(root, 'include/data/mart_items.h')
    if os.path.exists(names) and os.path.exists(menu) and os.path.exists(marts):
        ids = {n.strip(): i for i, n in enumerate(open(names)) if n.strip()}
        text = open(menu).read()
        table = text[text.index('sealTypeValues[SEAL_ID_MAX] = {'):]
        table = table[:table.index('};')]
        rows = re.findall(r'\{([^{}]*)\}', table)
        for i, row in enumerate(rows[:SEAL_COUNT]):
            prices[i] = int(row.split(',')[5].strip(), 0)
        mtext = open(marts).read()
        for d, day in enumerate(DAYS):
            m = re.search(r'SunyshoreMarketStock%s\[\] = \{([^}]*)\}' % day, mtext)
            if m:
                stock = [ids[x.strip()] for x in m.group(1).split(',') if x.strip() in ids]
                shops[d][:len(stock[:8])] = stock[:8]
    out = ['\t.balign 2', 'gSinnohSealPrices::'] + ['\t.2byte ' + ', '.join(str(x) for x in prices[k:k + 16])
                                                   for k in range(0, SEAL_COUNT, 16)]
    out += _bytes('gSinnohSealShops', bytes(x for row in shops for x in row))
    return out


def stub_lines():
    return (['\t.balign 4'] + _bytes('gSinnohTvSchedule', bytes(TV_SLOTS)) + _bytes('gSinnohTvPrograms', bytes(9 * 12))
            + seal_lines('/nonexistent')
            + ['\t.balign 4', 'gSinnohTextNames::', '\t.4byte 0'])
