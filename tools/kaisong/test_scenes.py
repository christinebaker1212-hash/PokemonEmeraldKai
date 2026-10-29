#!/usr/bin/env python3
"""Scenes come from the source alone: a new file that plays a song (or holds the music for the game's battle code) is
a scene the next time tools/kaisong/gen.py runs, with no list to edit; the picker (Moemon Music Studio embeds
gen.song_blocks / gen.places) and the game's table (names_h) both get it. usage: python3 tools/kaisong/test_scenes.py"""
import os, sys, importlib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import gen

ROOT = gen.ROOT
NEW = os.path.join(ROOT, 'src', 'mp', 'mp_kaitest_scene.c')
HELD = os.path.join(ROOT, 'src', 'mp', 'mp_kaitest_held.c')
fails = 0

def check(what, ok):
    global fails
    print('  %s %s' % ('ok  ' if ok else 'FAIL', what))
    fails += not ok

try:
    open(NEW, 'w').write('// MUSIC SCENE: The test scene\nvoid KaiTest(void) { PlayBGM(MUS_ROUTE101); }\n')
    open(HELD, 'w').write('void KaiTestHeld(void) { MP_MUSIC_SCENE_HOLD(MUS_VS_WILD); }\n')
    rows = gen.places()
    check('a new file that plays a song is a scene of that song',
          ('mus_route101', 'scene', 'kaitest_scene', 'The test scene') in rows)
    check('a new file holding the music lists the songs it inherits',
          any(r[:3] == ('mus_vs_wild', 'scene', 'kaitest_held') for r in rows))
    blocks = {n: body for n, _, body in gen.song_blocks(rows)}
    check('the picker / page shows it under the song', any('The test scene [kaitest_scene]' in l for l in blocks['mus_route101']))
    h = gen.names_h(rows)
    check('the game knows its file and songs', '{"mp/mp_kaitest_scene.c",' in h and '"kaitest_scene"' in h)
    check('card games, Yu-Gi-Oh! and Triple Triad are scenes',
          all(any(r[1] == 'scene' and r[2] == k for r in rows) for k in ('tcg', 'ygo', 'tt')))
    check('the BATTLE SIMULATOR inherits the trainer battle songs',
          ('mus_vs_trainer', 'scene', 'sim', 'BATTLE SIMULATOR (BATTLE TENTS)') in rows
          and ('mus_victory_trainer', 'scene', 'sim', 'BATTLE SIMULATOR (BATTLE TENTS)') in rows)
    check('no scene and map share a name', len({r[2] for r in rows if r[1] == 'scene'} & {k for _, _, k, _, _ in gen.maps()}) == 0)
finally:
    for f in (NEW, HELD):
        if os.path.exists(f):
            os.remove(f)
rows = gen.places()
check('and it is gone once the file is', not any(r[2] in ('kaitest_scene', 'kaitest_held') for r in rows))
print('%d failed' % fails)
sys.exit(fails)
