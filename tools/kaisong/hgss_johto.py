#!/usr/bin/env python3
"""JOHTO's music from HeartGold / SoulSilver: the user's 2SF rip (not in the repository) through Moemon Music Studio's
command line, into music/<song>.kaisong (every song here plays only in JOHTO: its own MUS_JOHTO_* songs, and the
battle and victory songs MpRegionBattleBGM / MP_RegionVictorySong pick there).

    python3 -I tools/kaisong/hgss_johto.py HGSS_RIP_FOLDER [STUDIO]   (STUDIO: tools/moemon_music/build/linux/...)

JOHTO's songs with no HGSS counterpart keep Quetzal's (MT. SILVER, the UNDERGROUND, CLIFF CAVE, jingles...).
"""
import os
import subprocess
import sys

# HGSS track number (the rip's file name starts with it) -> the game's song
MAP = [
    ('104', 'mus_johto_new_bark'), ('113', 'mus_johto_cherrygrove'), ('122', 'mus_johto_violet'),
    ('133', 'mus_johto_azalea'), ('141', 'mus_johto_goldenrod'), ('160', 'mus_johto_ecruteak'),
    ('171', 'mus_johto_cianwood'), ('109', 'mus_johto_route29'), ('120', 'mus_johto_route30'),
    ('136', 'mus_johto_route34'), ('166', 'mus_johto_route38'), ('173', 'mus_johto_route42'),
    ('201', 'mus_johto_route26'), ('115', 'mus_johto_pokecenter'), ('125', 'mus_johto_pokemart'),
    ('142', 'mus_johto_gym'), ('107', 'mus_johto_elms_lab'), ('161', 'mus_johto_dance_theater'),
    ('149', 'mus_johto_game_corner'), ('123', 'mus_johto_sprout_tower'), ('128', 'mus_johto_cave'),
    ('130', 'mus_johto_ruins_of_alph'), ('158', 'mus_johto_national_park'), ('162', 'mus_johto_burned_tower'),
    ('184', 'mus_johto_tin_tower'), ('175', 'mus_johto_rocket_hideout'), ('179', 'mus_johto_ice_path'),
    ('181', 'mus_johto_dragons_den'), ('276', 'mus_johto_victory_road'), ('277', 'mus_johto_indigo_plateau'),
    ('232', 'mus_johto_safari_gate'), ('233', 'mus_johto_safari_zone'), ('226', 'mus_johto_prof_oak'),
    ('105', 'mus_johto_follow_me'), ('137', 'mus_johto_rival_appears'), ('187', 'mus_johto_vs_ho_oh'),
    ('275', 'mus_johto_vs_lugia'), ('156', 'mus_johto_encounter_lass'), ('117', 'mus_johto_encounter_youngster'),
    ('176', 'mus_johto_encounter_nerd'), ('124', 'mus_johto_encounter_sage'), ('134', 'mus_johto_encounter_rocket'),
    # JOHTO's own battle and victory songs (sound/song_table.inc: FireRed's / Emerald's until these replace them)
    ('110', 'mus_johto_vs_wild'), ('118', 'mus_johto_vs_trainer'), ('143', 'mus_johto_vs_gym_leader'),
    ('138', 'mus_johto_vs_rival'), ('135', 'mus_johto_vs_rocket'), ('278', 'mus_johto_vs_champion'),
    ('168', 'mus_johto_vs_raikou'), ('183', 'mus_johto_vs_entei'), ('213', 'mus_johto_vs_suicune'),
    ('111a', 'mus_johto_victory_wild'), ('119', 'mus_johto_victory_trainer'), ('144', 'mus_johto_victory_gym_leader'),
]


def main():
    rip = sys.argv[1]
    root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    studio = sys.argv[2] if len(sys.argv) > 2 else os.path.join(root, 'tools/moemon_music/build/linux/moemon_music_studio')
    files = sorted(os.listdir(rip))
    out = os.path.join(root, 'music')
    failed = []
    for num, song in MAP:
        src = [f for f in files if f.endswith('.mini2sf') and f.split(' ')[0] == num]
        if len(src) != 1:
            failed.append((num, song, 'no single file %s' % src))
            continue
        r = subprocess.run([studio, '--export', os.path.join(rip, src[0]), out, song], capture_output=True, text=True)
        ok = r.returncode == 0 and os.path.exists(os.path.join(out, song + '.kaisong'))
        print('%-4s %-34s %s %s' % (num, song, 'ok' if ok else 'FAILED', src[0]), flush=True)
        if not ok:
            failed.append((num, song, (r.stdout + r.stderr)[-300:]))
    for f in failed:
        print('FAILED', *f)
    sys.exit(len(failed))


if __name__ == '__main__':
    main()
