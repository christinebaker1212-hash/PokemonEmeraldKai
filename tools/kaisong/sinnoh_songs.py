#!/usr/bin/env python3
"""SINNOH's music (docs/SINNOH_HANDOFF.md "Music"): Diamond's and Platinum's soundtracks from the user's 2SF rips (not in
the repository) as song slots of their own and music/<slot>.kaisong files made by Moemon Music Studio.

    python3 -I tools/kaisong/sinnoh_songs.py slots              # writes the slots into include/constants/songs.h and
                                                                # sound/song_table.inc (between their SINNOH markers)
    python3 -I tools/kaisong/sinnoh_songs.py convert DIAMOND_RIP PLATINUM_RIP [STUDIO]   # music/<slot>.kaisong

Each slot is a game song of its own (MUS_SINNOH_*) that plays one of Emerald's songs (FALLBACK) until its .kaisong
replaces it. A town or route that Platinum plays differently by night has a second slot, <slot>_night
(S3D_MapMusic picks it after dark). Left out on purpose: Wi-Fi, the GTS, Mystery Gift, Super Contests and Poffins
(Kai keeps Gen 3's contests), the Underground, the title and opening (docs/SINNOH_HANDOFF.md).
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIRST = 678   # after JOHTO's (include/constants/songs.h)

# (rip D = Diamond / P = Platinum, track number, slot name, Emerald song it plays without its .kaisong)
SONGS = [
    # towns and cities: day, then night
    ('D', '104', 'twinleaf', 'mus_littleroot'), ('D', '201', 'twinleaf_night', 'mus_littleroot'),
    ('D', '113', 'sandgem', 'mus_oldale'), ('D', '205', 'sandgem_night', 'mus_oldale'),
    ('D', '122', 'jubilife', 'mus_slateport'), ('D', '207', 'jubilife_night', 'mus_slateport'),
    ('D', '129', 'oreburgh', 'mus_rustboro'), ('D', '215', 'oreburgh_night', 'mus_rustboro'),
    ('D', '137', 'floaroma', 'mus_petalburg'), ('D', '222', 'floaroma_night', 'mus_petalburg'),
    ('D', '144', 'eterna', 'mus_fortree'), ('D', '218', 'eterna_night', 'mus_fortree'),
    ('D', '153', 'hearthome', 'mus_lilycove'), ('D', '244', 'hearthome_night', 'mus_lilycove'),
    ('D', '156', 'solaceon', 'mus_fallarbor'), ('D', '224', 'solaceon_night', 'mus_fallarbor'),
    ('D', '159', 'veilstone', 'mus_route110'), ('D', '228', 'veilstone_night', 'mus_route110'),
    ('D', '161', 'canalave', 'mus_dewford'), ('D', '211', 'canalave_night', 'mus_dewford'),
    ('D', '163', 'snowpoint', 'mus_route119'), ('D', '234', 'snowpoint_night', 'mus_route119'),
    ('D', '172', 'sunyshore', 'mus_sootopolis'), ('D', '240', 'sunyshore_night', 'mus_sootopolis'),
    ('D', '175', 'league', 'mus_ever_grande'), ('D', '261', 'league_night', 'mus_ever_grande'),
    ('D', '176', 'fight_area', 'mus_ever_grande'), ('D', '255', 'fight_area_night', 'mus_ever_grande'),
    ('D', '160', 'valor_lakefront', 'mus_lilycove'), ('D', '254', 'valor_lakefront_night', 'mus_lilycove'),
    # routes
    ('D', '106', 'route201', 'mus_route101'), ('D', '202', 'route201_night', 'mus_route101'),
    ('D', '125', 'route203', 'mus_route104'), ('D', '213', 'route203_night', 'mus_route104'),
    ('D', '139', 'route205', 'mus_route110'), ('D', '216', 'route205_night', 'mus_route110'),
    ('D', '152', 'route206', 'mus_route111'), ('D', '226', 'route206_night', 'mus_route111'),
    ('D', '154', 'route209', 'mus_route113'), ('D', '233', 'route209_night', 'mus_route113'),
    ('D', '158', 'route210', 'mus_route119'), ('D', '238', 'route210_night', 'mus_route119'),
    ('D', '162', 'route216', 'mus_route120'), ('D', '235', 'route216_night', 'mus_route120'),
    ('D', '177', 'route225', 'mus_route122'), ('D', '257', 'route225_night', 'mus_route122'),
    ('D', '178', 'route228', 'mus_route122'), ('D', '253', 'route228_night', 'mus_route122'),
    ('D', '210', 'surf', 'mus_surf'), ('D', '150', 'bicycle', 'mus_cycling'),
    # places
    ('D', '107', 'lake', 'mus_route101'), ('D', '114', 'research_lab', 'mus_birch_lab'),
    ('D', '116', 'pokecenter', 'mus_poke_center'), ('D', '203', 'pokecenter_night', 'mus_poke_center'),
    ('D', '124', 'pokemart', 'mus_poke_mart'), ('D', '127', 'oreburgh_gate', 'mus_cave_of_origin'),
    ('D', '130', 'oreburgh_mine', 'mus_cave_of_origin'), ('D', '131', 'gym', 'mus_gym'),
    ('D', '142', 'eterna_forest', 'mus_petalburg_woods'), ('D', '145', 'galactic_building', 'mus_aqua_magma_hideout'),
    ('D', '164', 'galactic_hideout', 'mus_aqua_magma_hideout'), ('D', '166', 'galactic_hideout_deep', 'mus_aqua_magma_hideout'),
    ('D', '167', 'mt_coronet', 'mus_cave_of_origin'), ('D', '168', 'spear_pillar', 'mus_cave_of_origin'),
    ('D', '173', 'victory_road', 'mus_victory_road'), ('D', '223', 'forest_house', 'mus_abandoned_ship'),
    ('D', '225', 'great_marsh', 'mus_safari_zone'), ('D', '229', 'game_corner', 'mus_game_corner'),
    ('D', '236', 'cavern_on_the_lake', 'mus_cave_of_origin'), ('D', '242', 'amity_square', 'mus_route101'),
    ('D', '256', 'battle_tower', 'mus_b_tower'), ('D', '258', 'iron_mountain', 'mus_cave_of_origin'),
    ('D', '262', 'league_showdown', 'mus_ever_grande'), ('P', '29', 'distortion_world', 'mus_cave_of_origin'),
    ('P', '38', 'villa', 'mus_littleroot'),
    # people and events
    ('D', '105', 'rival', 'mus_encounter_may'), ('D', '111', 'dawn', 'mus_encounter_may'),
    ('D', '206', 'lucas', 'mus_encounter_brendan'), ('D', '108', 'lake_encounter', 'mus_encounter_suspicious'),
    ('D', '115', 'follow_me', 'mus_follow_me'), ('D', '143', 'lets_go_together', 'mus_follow_me'),
    ('D', '140', 'galactic_appears', 'mus_encounter_aqua'), ('D', '169', 'legendary_appears', 'mus_cave_of_origin'),
    ('D', '170', 'natural_disaster', 'mus_cave_of_origin'), ('D', '263', 'elite_four_appear', 'mus_encounter_elite_four'),
    ('D', '266', 'cynthia', 'mus_encounter_champion'), ('D', '269', 'hall_of_fame', 'mus_hall_of_fame'),
    ('D', '271', 'ending', 'mus_credits'), ('P', '21', 'looker', 'mus_encounter_suspicious'),
    ('P', '27', 'giratina_appears', 'mus_cave_of_origin'), ('P', '28', 'giratina_appears_2', 'mus_cave_of_origin'),
    ('D', '301', 'azure_flute', 'mus_cave_of_origin'), ('D', '302', 'arceus', 'mus_cave_of_origin'),
    # trainers who spot you
    ('D', '118', 'encounter_kid', 'mus_encounter_male'), ('D', '119', 'encounter_miniskirt', 'mus_encounter_girl'),
    ('D', '135', 'encounter_twins', 'mus_encounter_twins'), ('D', '151', 'encounter_biker', 'mus_encounter_cool'),
    ('D', '155', 'encounter_hiker', 'mus_encounter_hiker'), ('D', '174', 'encounter_ace_trainer', 'mus_encounter_cool'),
    ('D', '214', 'encounter_martial_artist', 'mus_encounter_cool'), ('D', '217', 'encounter_sailor', 'mus_encounter_male'),
    ('D', '221', 'encounter_aroma_lady', 'mus_encounter_female'), ('D', '227', 'encounter_collector', 'mus_encounter_rich'),
    ('D', '232', 'encounter_gambler', 'mus_encounter_rich'), ('D', '241', 'encounter_artist', 'mus_encounter_intense'),
    # battles
    ('D', '109', 'vs_wild', 'mus_vs_wild'), ('D', '120', 'vs_trainer', 'mus_vs_trainer'),
    ('D', '126', 'vs_rival', 'mus_vs_rival'), ('D', '132', 'vs_gym_leader', 'mus_vs_gym_leader'),
    ('D', '141', 'vs_galactic', 'mus_vs_aqua_magma'), ('D', '146', 'vs_galactic_commander', 'mus_vs_aqua_magma_leader'),
    ('D', '165', 'vs_galactic_boss', 'mus_vs_aqua_magma_leader'), ('D', '171', 'vs_dialga_palkia', 'mus_vs_kyogre_groudon'),
    ('D', '237', 'vs_lake_guardians', 'mus_vs_regi'), ('D', '259', 'vs_legendary', 'mus_vs_regi'),
    ('D', '264', 'vs_elite_four', 'mus_vs_elite_four'), ('D', '267', 'vs_champion', 'mus_vs_champion'),
    ('D', '303', 'vs_arceus', 'mus_vs_kyogre_groudon'), ('P', '30', 'vs_giratina', 'mus_vs_kyogre_groudon'),
    ('P', '99', 'vs_regis', 'mus_vs_regi'), ('P', '48', 'vs_frontier_brain', 'mus_vs_frontier_brain'),
    # victories
    ('D', '110', 'victory_wild', 'mus_victory_wild'), ('D', '121', 'victory_trainer', 'mus_victory_trainer'),
    ('D', '133', 'victory_gym_leader', 'mus_victory_gym_leader'), ('D', '147', 'victory_galactic', 'mus_victory_aqua_magma'),
    ('D', '265', 'victory_elite_four', 'mus_victory_league'), ('D', '268', 'victory_champion', 'mus_victory_league'),
]

H_BEGIN = '// Moemon Kai: SINNOH\'s songs (tools/kaisong/sinnoh_songs.py)'
H_END = '// end of SINNOH\'s songs'
T_BEGIN = '@ Moemon Kai: SINNOH\'s songs (tools/kaisong/sinnoh_songs.py)'
T_END = '@ end of SINNOH\'s songs'


def replace_block(text, begin, end, body, anchor):
    if begin in text:
        a = text.index(begin)
        b = text.index(end, a) + len(end) + 1
        return text[:a] + begin + '\n' + body + end + '\n' + text[b:]
    i = text.index(anchor) + len(anchor)
    return text[:i] + begin + '\n' + body + end + '\n' + text[i:]


def slots():
    h = os.path.join(ROOT, 'include/constants/songs.h')
    t = os.path.join(ROOT, 'sound/song_table.inc')
    text = open(h).read()
    body = ''.join('#define MUS_SINNOH_%-26s %d\n' % (s[2].upper(), FIRST + i) for i, s in enumerate(SONGS))
    open(h, 'w').write(replace_block(text, H_BEGIN, H_END, body, '// end of Johto music\n'))
    text = open(t).read()
    body = ''.join('\t.set mus_sinnoh_%s, %s\n' % (s[2], s[3]) for s in SONGS)
    body += ''.join('\tsong mus_sinnoh_%s, MUSIC_PLAYER_BGM, 0\n' % s[2] for s in SONGS)
    open(t, 'w').write(replace_block(text, T_BEGIN, T_END, body, '@ end of Johto music\n'))
    names = [s[2] for s in SONGS]
    pairs = ''.join('    {MUS_SINNOH_%s, MUS_SINNOH_%s_NIGHT},\n' % (n.upper(), n.upper()) for n in names if n + '_night' in names)
    open(os.path.join(ROOT, 'src/data/sinnoh_night_songs.h'), 'w').write(
        '// Generated by tools/kaisong/sinnoh_songs.py: SINNOH songs Platinum plays differently by night (S3D_MapMusic)\n'
        'static const u16 sSinnohNightSongs[][2] = {\n' + pairs + '};\n')
    print('%d SINNOH slots, %d-%d' % (len(SONGS), FIRST, FIRST + len(SONGS) - 1))


def convert(diamond, platinum, studio):
    rips = {'D': diamond, 'P': platinum}
    listing = {k: sorted(os.listdir(v)) for k, v in rips.items()}
    out = os.path.join(ROOT, 'music')
    failed = []
    for rip, num, name, _ in SONGS:
        song = 'mus_sinnoh_' + name
        src = [f for f in listing[rip] if f.endswith('.mini2sf') and f.split(' ')[0] == num]
        if len(src) != 1:
            failed.append((song, 'no single file for %s %s' % (rip, num)))
            continue
        r = subprocess.run([studio, '--export', os.path.join(rips[rip], src[0]), out, song], capture_output=True,
                           text=True)
        ok = r.returncode == 0 and os.path.exists(os.path.join(out, song + '.kaisong'))
        if ok:   # a rip whose loop point wasn't found: the whole song loops (tools/kaisong/add_loop.py)
            sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
            import add_loop
            add_loop.add_loop(os.path.join(out, song + '.kaisong'))
        print('%-36s %s %s' % (song, 'ok' if ok else 'FAILED', src[0]), flush=True)
        if not ok:
            failed.append((song, (r.stdout + r.stderr)[-300:]))
    for f in failed:
        print('FAILED', *f)
    return len(failed)


def main():
    if sys.argv[1] == 'slots':
        slots()
    elif sys.argv[1] == 'convert':
        studio = sys.argv[4] if len(sys.argv) > 4 else os.path.join(ROOT, 'tools/moemon_music/build/linux/moemon_music_studio')
        sys.exit(convert(sys.argv[2], sys.argv[3], studio))


if __name__ == '__main__':
    main()
