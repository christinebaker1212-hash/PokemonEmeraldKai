#!/usr/bin/env python3
"""Moemon Kai custom music (.kaisong, docs/KAISONG_FORMAT.md): the generated tables.

Writes:
  src/data/mp_kaisong_samples.h   the game's samples / Game Boy waves by symbol name (NAME chunks)
  src/data/mp_kaisong_names.h     song names by number, map keys by group / number (music/ file names)
  docs/SONG_TABLE.md              the same as a page, with the custom songs in music/

usage: python3 tools/kaisong/gen.py [--check]     (--check: fail when a file is out of date)
       python3 tools/kaisong/gen.py --music DIR --md OUT   the page for another music folder (the studio's exports; tests)
The release runs it before building, so the page and the game's music/ folder always match.
"""
import os, re, sys, glob, json, struct

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
def rd(p): return open(os.path.join(ROOT, p), encoding='utf-8', errors='replace').read()

# ------------------------------------------------------------------ names

def sample_labels(path):
    out = []
    for line in rd(path).splitlines():
        m = re.match(r'\s*([A-Za-z_][A-Za-z0-9_]*)::', line)
        if m and m.group(1) not in out:
            out.append(m.group(1))
    return out

def song_table():
    """[(name, player)] in song number order (sound/song_table.inc)."""
    out = []
    for m in re.finditer(r'^\s*song\s+(\w+)\s*,\s*(\w+)', rd('sound/song_table.inc'), re.M):
        out.append((m.group(1), m.group(2)))
    return out

def song_consts():
    """MUS_X -> number (include/constants/songs.h)."""
    out = {}
    for m in re.finditer(r'#define\s+(MUS_\w+)\s+(\d+)', rd('include/constants/songs.h')):
        out[m.group(1)] = int(m.group(2))
    for i, (n, _) in enumerate(song_table()):     # the song table's own names too (mus_route111 is MUS_DESERT there)
        out.setdefault(n.upper(), i)
    return out

def maps():
    """[(group, num, map key, folder name, MAP_ id)] and the group order."""
    groups = json.loads(rd('data/maps/map_groups.json'))
    out = []
    for gi, g in enumerate(groups['group_order']):
        for ni, folder in enumerate(groups[g]):
            mj = json.loads(rd('data/maps/%s/map.json' % folder))
            out.append((gi, ni, mj['id'][4:].lower(), folder, mj))
    return out

def pretty_map(folder):
    s = folder.replace('_', ' ')
    s = re.sub(r'(?<=[a-z])(?=[A-Z0-9])|(?<=[0-9])(?=[A-Z][a-z])', ' ', s)
    s = re.sub(r'(?<=[A-Z])(?=[A-Z][a-z])', ' ', s)
    return re.sub(r'\s+', ' ', s).strip()

def pretty_file(stem):
    return stem.replace('_', ' ').strip().capitalize()

# ------------------------------------------------------------------ where songs play

# C files whose songs are a screen or a part of the game, by a name players know
SCREENS = {
    'title_screen': 'Title screen', 'intro': 'Intro movie', 'main_menu': 'Main menu', 'new_game': 'New game',
    'credits': 'Credits', 'hall_of_fame': 'Hall of Fame', 'evolution_scene': 'Evolution', 'trade': 'Trades',
    'pokedex': 'POKéDEX', 'pokenav': 'POKéNAV', 'berry_blender': 'BERRY BLENDER', 'contest': 'Contests',
    'contest_util': 'Contests', 'cable_club': 'Link battles (CABLE CLUB)', 'union_room': 'UNION ROOM',
    'battle_setup': 'Trainer battles', 'battle_main': 'Battles', 'battle_script_commands': 'Battles',
    'pokemon': 'Battles', 'battle_message': 'Battles (a POKéMON is caught)', 'roulette': 'GAME CORNER roulette', 'slot_machine': 'GAME CORNER slots',
    'field_specials': 'Field events', 'overworld': 'Overworld', 'bike': 'Riding a BIKE', 'field_player_avatar': 'Surfing / diving',
    'battle_dome': 'BATTLE DOME', 'battle_pike': 'BATTLE PIKE', 'battle_pyramid': 'BATTLE PYRAMID',
    'battle_factory': 'BATTLE FACTORY', 'battle_palace': 'BATTLE PALACE', 'battle_arena': 'BATTLE ARENA',
    'battle_tower': 'BATTLE TOWER', 'trainer_hill': 'TRAINER HILL', 'party_menu': 'Party menu',
    'item_use': 'Using items', 'egg_hatch': 'Egg hatching', 'easy_chat': 'Easy chat', 'naming_screen': 'Naming screen',
    'pokeblock_feed': 'Feeding POKéBLOCKS', 'rayquaza_scene': 'Rayquaza scene', 'wallclock': 'Wall clock',
    'mp_dance': 'DANCE machine', 'mp_garden': 'MOéMON GARDEN', 'mp_candy': 'CANDY JAR', 'sim': 'BATTLE SIMULATOR (BATTLE TENTS)',
    'gatcha': 'MoeGatcha', 'setup': 'CO-OP setup screen', 'tmrefund': 'LOTTERY CORNER (TMs given back)',
    'fight': 'VS cabinet', 'tourney': 'Card TOURNAMENTs', 'candy': 'CANDY JAR', 'garden': 'MOéMON GARDEN',
}
CALLS = r'(?:PlayBGM|PlayNewMapMusic|FadeOutAndPlayNewMapMusic|FadeOutAndFadeInNewMapMusic|PlayFanfare|PlayFanfareByFanfareNum|' \
        r'm4aSongNumStart|m4aSongNumStartOrChange|Overworld_SetSavedMusic|Overworld_ChangeMusicTo|PlayMapChosenOrBattleBGM|' \
        r'CreateTask_PlayMapChosenOrBattleBGM|SetMainCallback2AndPlayMusic|return)\s*\(?\s*(MUS_\w+)'

# The world's files: songs they ask for belong to the map (map music, script events, trainer battles, encounters).
# Every other source file that names a song is a scene (a new one is picked up the next time this runs).
WORLD = re.compile(r'^(overworld|sound|m4a|music_player|scrcmd|script.*|field_.*|fldeff_.*|event_.*|coord_event_weather|'
                   r'battle_(?!dome|pike|pyramid|factory|palace|arena|tower|tent|records).*|pokemon|trainer_see|'
                   r'wild_encounter|bike|text|roamer|item_use|start_menu|option_menu|daycare|mp_debug|'
                   r'coop|battle|join|trainer|net|main|ballfx|levelscale|follow.*|life.*|kaisong|spectate)$')
SUBDIR_SCENES = {'tcg': 'POKéMON TCG card duels', 'ygo': 'YU-GI-OH! duels', 'tt': 'TRIPLE TRIAD', 'rhythm': 'DANCE machine'}

def scene_key(path, top=()):
    """src/x.c -> x, src/mp/mp_x.c -> x (mp_x when src/x.c exists too), src/<dir>/... -> <dir>. The game finds a file's
    scene in the generated table by its path (mp_kaisong.c), so only this rule names them."""
    rel = os.path.relpath(path, os.path.join(ROOT, 'src')).replace(os.sep, '/').split('/')
    stem = os.path.splitext(rel[-1])[0]
    if len(rel) == 1:
        return stem
    if rel[0] == 'mp':
        return stem[3:] if stem.startswith('mp_') and stem[3:] not in top else stem
    return rel[0]

def scene_files():
    """{key: [files]} for every source file of the game (src/platform aside)."""
    files = [f for f in sorted(glob.glob(os.path.join(ROOT, 'src', '*.c')) + glob.glob(os.path.join(ROOT, 'src', '*', '*.c')))
             if not re.search(r'[/\\](platform|data)[/\\]', f)]
    top = {os.path.splitext(os.path.basename(f))[0] for f in glob.glob(os.path.join(ROOT, 'src', '*.c'))}
    out = {}
    for f in files:
        out.setdefault(scene_key(f, top), []).append(f)
    return out

def is_world(key):
    return bool(WORLD.match(key))

def scene_label(key, files):
    for f in files:   # a file can name its scene: // MUSIC SCENE: The name players know
        m = re.search(r'MUSIC SCENE: *(.+)', open(f, encoding='utf-8', errors='replace').read())
        if m:
            return m.group(1).strip()
    return SUBDIR_SCENES.get(key) or SCREENS.get(key) or SCREENS.get('mp_' + key) or pretty_file(key.replace('mp_', ''))

def strip_comments(t):
    return re.sub(r'//[^\n]*|/\*.*?\*/', '', t, flags=re.S)

def source_body(f):
    stem = os.path.splitext(os.path.basename(f))[0]
    body = open(f, encoding='utf-8', errors='replace').read()
    if stem == 'pokemon' and f.endswith('src/pokemon.c'):   # GetBattleBGM: the trainer battles above
        body = body[:body.index('u16 GetBattleBGM(void)')] + body[body.index('\nvoid PlayBattleBGM'):]
    elif stem == 'battle_setup':
        body = re.sub(r'switch \(GetTrainerEncounterMusicId.*?PlayNewMapMusic\(music\);', '', body, flags=re.S)
    return body

def scenes(rows):
    """[(key, [song numbers])]: what the game needs to know a scene's own songs."""
    num = {n: i for i, (n, _) in enumerate(song_table())}
    out = {}
    for r in rows:
        if r[1] == 'scene':
            out.setdefault(r[2], [])
            if num[r[0]] not in out[r[2]]:
                out[r[2]].append(num[r[0]])
    return sorted(out.items())

def battle_class_songs():
    """trainer class -> song, from GetBattleBGM (src/pokemon.c)."""
    src = rd('src/pokemon.c')
    body = src[src.index('u16 GetBattleBGM(void)'):]
    body = body[:body.index('\nvoid PlayBattleBGM')]
    sw = body[body.index('switch (trainerClass)'):]
    out, pending = {}, []
    for line in sw.splitlines():
        m = re.match(r'\s*case (TRAINER_CLASS_\w+):', line)
        if m: pending.append(m.group(1)); continue
        m = re.search(r'return (MUS_\w+);', line)
        if m and pending:
            for c in pending: out.setdefault(c, m.group(1))
            pending = []
    return out

def encounter_songs():
    src = rd('src/battle_setup.c')
    return {m.group(1): m.group(2) for m in re.finditer(r'case (TRAINER_ENCOUNTER_MUSIC_\w+):\s*music = (MUS_\w+);', src)}

def trainers():
    """TRAINER_X -> (class, encounter music, name)."""
    out = {}
    for m in re.finditer(r'\[(TRAINER_\w+)\]\s*=\s*\{(.*?)\n    \}', rd('src/data/trainers.h'), re.S):
        b = m.group(2)
        cl = re.search(r'\.trainerClass = (\w+)', b)
        em = re.search(r'\.encounterMusic_gender = (?:F_TRAINER_FEMALE \| )?(\w+)', b)
        nm = re.search(r'\.trainerName = _\("([^"]*)"\)', b)
        out[m.group(1)] = (cl.group(1) if cl else '', em.group(1) if em else '', nm.group(1) if nm else '')
    return out

CLASS_NAMES = {'TRAINER_CLASS_LEADER': 'Gym Leaders', 'TRAINER_CLASS_ELITE_FOUR': 'the Elite Four', 'TRAINER_CLASS_CHAMPION': 'the Champion',
               'TRAINER_CLASS_RIVAL': 'Brendan / May', 'TRAINER_CLASS_AQUA_LEADER': 'Archie', 'TRAINER_CLASS_MAGMA_LEADER': 'Maxie',
               'TRAINER_CLASS_TEAM_AQUA': 'Team Aqua', 'TRAINER_CLASS_TEAM_MAGMA': 'Team Magma', 'TRAINER_CLASS_AQUA_ADMIN': 'Aqua Admins',
               'TRAINER_CLASS_MAGMA_ADMIN': 'Magma Admins'}

def places():
    """[(song, kind, place key, label)]: kind map / event / battle / encounter / screen; place key '' = not one map."""
    consts = song_consts()
    rows = []
    allmaps = maps()
    keyof = {folder: key for _, _, key, folder, _ in allmaps}
    for _, _, key, folder, mj in allmaps:
        if mj.get('music', '').startswith('MUS_') and mj['music'] != 'MUS_DUMMY':
            rows.append((mj['music'], 'map', key, pretty_map(folder)))
    # script events: per map, and the shared scripts
    cmd = re.compile(r'^\s*(playbgm|fadenewbgm|playfanfare|savebgm|fadeoutbgm_newbgm)\s+(MUS_\w+)', re.M)
    for f in sorted(glob.glob(os.path.join(ROOT, 'data/maps/*/scripts.inc'))):
        folder = os.path.basename(os.path.dirname(f))
        for m in cmd.finditer(open(f, encoding='utf-8', errors='replace').read()):
            rows.append((m.group(2), 'event', keyof.get(folder, ''), pretty_map(folder)))
    for f in sorted(glob.glob(os.path.join(ROOT, 'data/scripts/*.inc')) + [os.path.join(ROOT, 'data/event_scripts.s')]):
        for m in cmd.finditer(open(f, encoding='utf-8', errors='replace').read()):
            rows.append((m.group(2), 'event', '', 'anywhere: ' + pretty_file(os.path.splitext(os.path.basename(f))[0])))
    # trainer battles and the music when a trainer spots you, on the maps the trainers stand on
    cls, enc, tr = battle_class_songs(), encounter_songs(), trainers()
    tb = re.compile(r'^\s*trainerbattle\w*\s+(?:\w+\s*,\s*)?(TRAINER_\w+)', re.M)
    for f in sorted(glob.glob(os.path.join(ROOT, 'data/maps/*/scripts.inc'))):
        folder = os.path.basename(os.path.dirname(f))
        for m in tb.finditer(open(f, encoding='utf-8', errors='replace').read()):
            t = tr.get(m.group(1))
            if not t:
                continue
            c, e, name = t
            song = cls.get(c, 'MUS_VS_TRAINER')
            if c == 'TRAINER_CLASS_RIVAL' and name == 'WALLY':
                song = 'MUS_VS_TRAINER'
            who = CLASS_NAMES.get(c)
            label = pretty_map(folder) + (' (%s)' % name.title() if who else '')
            rows.append((song, 'battle', keyof.get(folder, ''), label))
            rows.append((enc.get(e, 'MUS_ENCOUNTER_SUSPICIOUS'), 'encounter', keyof.get(folder, ''), pretty_map(folder)))
    rows.append(('MUS_VS_WILD', 'battle', '', 'wild POKéMON, anywhere'))
    rows.append(('MUS_VS_FRONTIER_BRAIN', 'battle', '', 'Frontier Brains (BATTLE FRONTIER)'))
    pk = rd('src/pokemon.c')
    for m in re.finditer(r'BATTLE_TYPE_(\w+)\)\s*\{?\s*return (MUS_\w+);', pk):
        if m.group(1) not in ('TRAINER',):
            rows.append((m.group(2), 'battle', '', m.group(1).replace('_', ' / ').title() + ' battle'))
    bs = rd('src/battle_setup.c')
    for m in re.finditer(r'((?:case SPECIES_\w+:\s*)+)(?:[^;]*;\s*)?CreateBattleStartTask\(\w+, (MUS_\w+)\)', bs):
        who = ' / '.join(x.replace('_', '-').title() for x in re.findall(r'SPECIES_(\w+)', m.group(1)))
        rows.append((m.group(2), 'battle', '', 'wild ' + who))
    # scenes: the parts of the game that choose their own songs (their files ask for them: include/mp_kaisong_scene.h),
    # and the world's own code (field, scripts, battles), whose songs belong to the map they play on
    call = re.compile(CALLS)
    for key, files in sorted(scene_files().items()):
        label = scene_label(key, files)
        for f in files:
            body = source_body(f)
            if is_world(key):
                for m in list(call.finditer(body)) + list(re.finditer(r'(?:PLAY_BGM |FadeInNewBGM\()(MUS_\w+)', body)):
                    rows.append((m.group(1), 'screen', '', label))
            else:
                # every song the scene names, but not the ones it only compares with (!= MUS_CYCLING)
                for m in re.finditer(r'(?<![=!]= )(?<![=!]=)\b(MUS_\w+)\b', strip_comments(body)):
                    rows.append((m.group(1), 'scene', key, label))
    # only songs that exist and play on the music player, once each, by their song table names
    st = song_table()
    seen, out = set(), []
    for r in rows:
        n = consts.get(r[0], -1)
        if n <= 0 or n >= len(st) or st[n][1] != 'MUSIC_PLAYER_BGM' or st[n][0] == 'dummy_song_header':
            continue
        r = (st[n][0],) + tuple(r[1:])
        if r not in seen:
            seen.add(r); out.append((n,) + r)
    out.sort(key=lambda r: (r[0], [k for k, _ in KIND_TITLES].index(r[2]), r[4]))
    keys = {k for _, _, k, _, _ in maps()}
    for r in out:
        if r[2] == 'scene' and r[3] in keys:
            sys.exit('scene %s has the name of a map: rename the file' % r[3])
    return [r[1:] for r in out]

# ------------------------------------------------------------------ custom songs (music/)

def custom_songs(folder=None):
    """[(file, song, place key, INFO dict)] for music/*.kaisong (or another folder), sorted by file name (bytes)."""
    out = []
    files = glob.glob(os.path.join(folder or os.path.join(ROOT, 'music'), '*.kaisong'))
    for f in sorted(files, key=lambda x: os.path.basename(x).encode()):
        name = os.path.basename(f)[:-8]
        song, _, place = name.partition('@')
        info = {}
        data = open(f, 'rb').read()
        if data[:8] == b'KAISONG\0':
            off = struct.unpack_from('<H', data, 10)[0]
            if data[off:off + 4] == b'INFO':
                n = struct.unpack_from('<I', data, off + 4)[0]
                for line in data[off + 8:off + 8 + n].decode('utf-8', 'replace').splitlines():
                    k, _, v = line.partition('=')
                    info[k.strip()] = v.strip()
        out.append((os.path.basename(f), song, place, info))
    return out

# ------------------------------------------------------------------ output

KIND_TITLES = [('map', 'Map music'), ('scene', 'Scenes'), ('event', 'Events'), ('battle', 'Battles'),
               ('encounter', 'A trainer spots you'), ('screen', 'Field and battle code (by map)')]

MD_INTRO = ['# Where every song plays', '',
            '_Generated by `tools/kaisong/gen.py` (the release makes it again for the game\'s `music` folder): do not edit._', '',
            'Custom music: put `<song>.kaisong` in the game\'s `music` folder to change a song everywhere, or',
            '`<song>@<place>.kaisong` to change it in one place only (the place keys are in brackets below, e.g.',
            '`mus_route101@route103.kaisong`). Moemon Music Studio names its exports this way (SONG > Replaces).',
            'Files in this repository\'s `music` folder ship with every update.', '']
NOT_FOUND = "- Not played anywhere the tables can find (unused, or chosen in a way they can't follow)."

def song_blocks(rows):
    """[(song name, number, [lines])] for every song the music player plays: where it plays (custom lines aside).
    Moemon Music Studio embeds these (gen_soundfont.py) and writes the same page next to its exports."""
    num = {n: i for i, (n, _) in enumerate(song_table())}
    bgm = [n for n, pl in song_table() if pl == 'MUSIC_PLAYER_BGM' and n not in ('mus_dummy', 'dummy_song_header')]
    by = {}
    for r in rows:
        by.setdefault(r[0], []).append(r)
    out = []
    for n in bgm:
        rs, L = by.get(n, []), []
        if not rs:
            L.append(NOT_FOUND)
        for kind, title in KIND_TITLES:
            items = []
            for r in rs:
                it = r[3] + (' [%s]' % r[2] if r[2] else '')
                if r[1] == kind and it not in items:
                    items.append(it)
            if len(items) > 12:
                L.append('- %s: %d places<details><summary>show</summary>%s</details>' % (title, len(items), ', '.join(items)))
            elif items:
                L.append('- %s: %s' % (title, ', '.join(items)))
        out.append((n, num[n], L))
    return out

def map_names():
    return [(key, pretty_map(folder)) for _, _, key, folder, _ in maps()]

def table_md(rows, customs):
    """The page. Moemon Music Studio's export.c export_song_table() writes exactly this: change both together."""
    mapname = dict(map_names())
    L = list(MD_INTRO) + ['## Custom music in this version', '']
    if customs:
        L += ['| file | replaces | where | title |', '|---|---|---|---|']
        for f, song, place, info in customs:
            L.append('| `%s` | %s | %s | %s |' % (f, song, mapname.get(place, place) if place else 'everywhere',
                                                    info.get('title', '').replace('|', '/')))
    else:
        L.append('None yet.')
    L += ['', '## Songs', '']
    for n, number, body in song_blocks(rows):
        L.append('### %s (%d)' % (n, number))
        custom = [c for c in customs if c[1] == n]
        if custom:
            L.append('Custom: ' + ', '.join('`%s`' % c[0] for c in custom))
        L += body + ['']
    return '\n'.join(L)

def names_h(rows):
    songs = song_table()
    L = ['// Generated by tools/kaisong/gen.py - do not edit.',
         '// Song names by number and map keys by group / number: the names of the .kaisong files in music/.', '',
         'static const char *const sKaisongSongNames[] = {']
    L += ['    "%s",' % ('' if n == 'dummy_song_header' else n) for n, _ in songs]
    L += ['};', '']
    ms = maps()
    ngroups = max(g for g, *_ in ms) + 1
    for g in range(ngroups):
        L.append('static const char *const sKaisongMaps%d[] = {' % g)
        L += ['    "%s",' % key for gg, _, key, _, _ in ms if gg == g]
        L.append('};')
    L += ['', 'static const struct { const char *const *keys; u16 count; } sKaisongMapGroups[] = {']
    L += ['    {sKaisongMaps%d, ARRAY_COUNT(sKaisongMaps%d)},' % (g, g) for g in range(ngroups)]
    L += ['};', '']
    # scenes: their keys, their own songs, and the source files that ask for songs in their name
    sc = scenes(rows)
    L += ['// Scenes (music/<song>@<scene>.kaisong): the songs each one names, and its source files (paths under src/)']
    for i, (key, songs) in enumerate(sc):
        L.append('static const u16 sKaisongScene%dSongs[] = {%s};' % (i, ', '.join(str(n) for n in songs)))
    L += ['static const struct { const char *key; const u16 *songs; u16 count; } sKaisongScenes[] = {']
    L += ['    {"%s", sKaisongScene%dSongs, ARRAY_COUNT(sKaisongScene%dSongs)},' % (key, i, i) for i, (key, _) in enumerate(sc)]
    L += ['};', 'static const struct { const char *path; u8 scene; } sKaisongSceneFiles[] = {']
    files = scene_files()
    for i, (key, _) in enumerate(sc):
        for f in files[key]:
            L.append('    {"%s", %d},' % (os.path.relpath(f, os.path.join(ROOT, 'src')).replace(os.sep, '/'), i))
    L += ['};', '']
    return '\n'.join(L)

def samples_h():
    names = sample_labels('sound/direct_sound_data.inc') + sample_labels('sound/programmable_wave_data.inc')
    L = ['// Generated by tools/kaisong/gen.py - do not edit.',
         '// The game\'s samples and Game Boy waves by symbol name (.kaisong NAME chunks: docs/KAISONG_FORMAT.md)', '']
    L += ['extern const u8 %s[];' % n for n in names]
    L += ['', 'static const struct { const char *name; const u8 *data; } sKaisongSamples[] = {']
    L += ['    {"%s", %s},' % (n, n) for n in names]
    L += ['};', '']
    return '\n'.join(L)

def main():
    if '--music' in sys.argv:
        folder, out = sys.argv[sys.argv.index('--music') + 1], sys.argv[sys.argv.index('--md') + 1]
        open(out, 'w', encoding='utf-8').write(table_md(places(), custom_songs(folder)))
        return
    rows = places()
    outs = {
        'src/data/mp_kaisong_samples.h': samples_h(),
        'src/data/mp_kaisong_names.h': names_h(rows),
        'docs/SONG_TABLE.md': table_md(rows, custom_songs()),
    }
    stale = []
    for path, text in outs.items():
        full = os.path.join(ROOT, path)
        old = open(full, encoding='utf-8').read() if os.path.exists(full) else None
        if old != text:
            stale.append(path)
            if '--check' not in sys.argv:
                open(full, 'w', encoding='utf-8').write(text)
    if '--check' in sys.argv:
        if stale:
            sys.exit('out of date (run python3 tools/kaisong/gen.py): ' + ', '.join(stale))
        return
    print('%d places for %d songs; %d custom; wrote %s' % (len(rows), len({r[0] for r in rows}), len(custom_songs()),
                                                         ', '.join(stale) or 'nothing new'))

if __name__ == '__main__':
    main()
