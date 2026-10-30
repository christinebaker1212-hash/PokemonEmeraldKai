#!/usr/bin/env python3
"""Johto's music (Moemon Quetzal's songs 610-665) into Kai's song table, at the same numbers (Kai's table ends at 609).

    python3 tools/quetzal_port/gen_music.py

Copies quetzal_rip/sound's .mid files (as mus_johto_<name>.mid), voicegroups, keysplits and samples into sound/,
appends the midi.cfg lines, song table rows and include lines between "Johto music" markers, writes the constants
into include/constants/songs.h (MUS_JOHTO_*), and points the rip's MUS_QZ_<id> at them (include/constants/qz_port.h).
Kanto plays FireRed's songs, which Kai has.
"""
import glob
import os
import re
import shutil

REPO = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
SND = os.path.join(REPO, 'quetzal_rip', 'sound')
BEGIN, END = 'Moemon Kai: Johto music (tools/quetzal_port/gen_music.py)', 'end of Johto music'

NAMES = {
    610: 'TRAINER_HILL', 611: 'NEW_BARK', 612: 'CHERRYGROVE', 613: 'VIOLET', 614: 'AZALEA', 615: 'GOLDENROD',
    616: 'ECRUTEAK', 617: 'CIANWOOD', 618: 'ROUTE29', 619: 'ROUTE30', 620: 'ROUTE34', 621: 'ROUTE38', 622: 'ROUTE42',
    623: 'ROUTE26', 624: 'POKECENTER', 625: 'POKEMART', 626: 'GYM', 627: 'ELMS_LAB', 628: 'DANCE_THEATER',
    629: 'GAME_CORNER', 630: 'SPROUT_TOWER', 631: 'CAVE', 632: 'RUINS_OF_ALPH', 633: 'NATIONAL_PARK',
    634: 'BURNED_TOWER', 635: 'TIN_TOWER', 636: 'MT_SILVER', 637: 'ROCKET_HIDEOUT', 638: 'ICE_PATH', 639: 'DRAGONS_DEN',
    640: 'VICTORY_ROAD', 641: 'INDIGO_PLATEAU', 642: 'UNDERGROUND', 643: 'CLIFF_CAVE', 644: 'SAFARI_GATE',
    645: 'SAFARI_ZONE', 646: 'PROF_OAK', 647: 'FOLLOW_ME', 648: 'RIVAL_APPEARS', 649: 'RIVAL_DEPARTS', 650: 'JINGLE',
    651: 'CIANWOOD_EVENT', 652: 'VS_HO_OH', 653: 'VS_LUGIA', 654: 'LEGEND_APPEARS', 655: 'EVENT', 656: 'SE_SHORT',
    657: 'ENCOUNTER_LASS', 658: 'ENCOUNTER_YOUNGSTER', 659: 'ENCOUNTER_NERD', 660: 'ENCOUNTER_SAGE',
    661: 'ENCOUNTER_ROCKET', 662: 'ENCOUNTER_COOLTRAINER', 663: 'ENCOUNTER_SWIMMER', 664: 'ENCOUNTER_HIKER',
    665: 'ENCOUNTER_CHAMPION',
}


def between(path, text, comment):
    s = open(path, encoding='utf-8').read()
    block = '%s %s\n%s%s %s\n' % (comment, BEGIN, text, comment, END)
    pat = re.compile(r'%s %s\n.*?%s %s\n' % (re.escape(comment), re.escape(BEGIN), re.escape(comment), re.escape(END)), re.S)
    s = pat.sub(lambda m: block, s) if pat.search(s) else s.rstrip('\n') + '\n\n' + block
    open(path, 'w', encoding='utf-8').write(s)


def main():
    ids = sorted(NAMES)
    # sounds the songs play
    for sub in ('drumsets', 'keysplits'):
        for f in glob.glob(os.path.join(SND, 'voicegroups', sub, '*.inc')):
            os.makedirs(os.path.join(REPO, 'sound/voicegroups', sub), exist_ok=True)
            shutil.copy(f, os.path.join(REPO, 'sound/voicegroups', sub))
    for f in glob.glob(os.path.join(SND, 'voicegroups', '*.inc')):
        shutil.copy(f, os.path.join(REPO, 'sound/voicegroups'))
    for f in glob.glob(os.path.join(SND, 'direct_sound_samples', '*.wav')):
        shutil.copy(f, os.path.join(REPO, 'sound/direct_sound_samples'))
    body = lambda f: ''.join(l for l in open(os.path.join(SND, f), encoding='utf-8') if not l.startswith('/*'))
    between(os.path.join(REPO, 'sound/voice_groups.inc'), body('voice_groups.inc'), '@')
    between(os.path.join(REPO, 'sound/direct_sound_data.inc'), body('direct_sound_data.inc'), '@')
    between(os.path.join(REPO, 'sound/keysplit_tables.inc'), body('keysplit_tables.inc'), '@')
    pw = body('programmable_wave_data.inc')
    if pw.strip():
        between(os.path.join(REPO, 'sound/programmable_wave_data.inc'), pw, '@')
    # the songs
    cfg = {}
    for l in open(os.path.join(SND, 'midi', 'midi.cfg'), encoding='utf-8'):
        m = re.match(r'mus_qz_(\d+)\.mid:\s*(.*)', l.strip())
        if m:
            cfg[int(m.group(1))] = m.group(2)
    rows = {}
    for l in open(os.path.join(SND, 'song_table.inc'), encoding='utf-8'):
        m = re.match(r'\s*song mus_qz_(\d+), (\w+), (\d+)', l)
        if m:
            rows[int(m.group(1))] = (m.group(2), m.group(3))
    cfg_lines, table, consts, alias = '', '', '', ''
    for n in ids:
        name = 'mus_johto_' + NAMES[n].lower()
        shutil.copy(os.path.join(SND, 'midi', 'mus_qz_%d.mid' % n), os.path.join(REPO, 'sound/songs/midi', name + '.mid'))
        cfg_lines += '%s.mid: %s\n' % (name, cfg[n])
        table += '\tsong %s, %s, %s\n' % (name, rows[n][0], rows[n][1])
        consts += '#define %-27s %d\n' % ('MUS_JOHTO_' + NAMES[n], n)
        alias += '#define %-40s MUS_JOHTO_%s\n' % ('MUS_QZ_%d' % n, NAMES[n])
    # midi.cfg takes no comments (every line becomes a make rule): its Johto lines are the mus_johto_* ones
    p = os.path.join(REPO, 'sound/songs/midi/midi.cfg')
    keep = [l for l in open(p, encoding='utf-8') if not l.startswith('mus_johto_') and not l.startswith('# ')]
    open(p, 'w', encoding='utf-8').write(''.join(keep).rstrip('\n') + '\n' + cfg_lines)
    # song table rows go right after PH_NURSE_SOLO (ids continue from 610)
    p = os.path.join(REPO, 'sound/song_table.inc')
    s = open(p, encoding='utf-8').read()
    s = re.sub(r'@ %s\n.*?@ %s\n' % (re.escape(BEGIN), re.escape(END)), '', s, flags=re.S)
    anchor = '\tsong ph_nurse_solo, MUSIC_PLAYER_SE2, 2\n'
    s = s.replace(anchor, anchor + '@ %s\n%s@ %s\n' % (BEGIN, table, END))
    open(p, 'w', encoding='utf-8').write(s)
    p = os.path.join(REPO, 'include/constants/songs.h')
    s = open(p, encoding='utf-8').read()
    s = re.sub(r'// %s\n.*?// %s\n' % (re.escape(BEGIN), re.escape(END)), '', s, flags=re.S)
    anchor = re.search(r'#define PH_NURSE_SOLO\s+609\n', s).group(0)
    s = s.replace(anchor, anchor + '// %s\n%s// %s\n' % (BEGIN, consts, END))
    open(p, 'w', encoding='utf-8').write(s)
    p = os.path.join(REPO, 'include/constants/qz_port.h')
    s = open(p, encoding='utf-8').read()
    s = re.sub(r'#define (MUS_QZ_(\d+))\s+\S+', lambda m: '#define %-40s %s' % (m.group(1), 'MUS_JOHTO_' + NAMES[int(m.group(2))]
               if int(m.group(2)) in NAMES else 'MUS_NONE'), s)
    s = s.replace('// music (placeholders until the songs are imported)', '// music (tools/quetzal_port/gen_music.py)')
    open(p, 'w', encoding='utf-8').write(s)
    print('%d Johto songs (%d-%d)' % (len(ids), ids[0], ids[-1]))


if __name__ == '__main__':
    main()
