#!/usr/bin/env python3
"""Builds the data Moemon Music Studio plays with, straight from the game's sound folder.

  python3 tools/moemon_music/gen_soundfont.py OUT_DIR

Reads sound/voice_groups.inc (every voicegroup, drum kit and key-split instrument, in the
order the game lays them out in memory), sound/keysplit_tables.inc, the DirectSound
samples (converted with tools/wav2agb exactly like the game build does), the programmable
waves, the song table, midi.cfg and the m4a / CGB tables from the engine source.

Writes OUT_DIR/sf_counts.h + sf_data.c (instrument data, presets, the General MIDI map, engine tables)
and OUT_DIR/sf_pcm.bin (the 8-bit sample data the .h points into).
"""
import os, re, sys, struct, subprocess, collections

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SOUND = os.path.join(ROOT, 'sound')

def die(msg):
    sys.exit('gen_soundfont: ' + msg)

# ---------------------------------------------------------------- voicegroups

def parse_args(rest):
    return [a.strip() for a in rest.split(',')] if rest.strip() else []

def num(s):
    return int(s, 0)

PLACEHOLDER = ('sq1', 60, 0, 0, 2, 0, 0, 15, 0)

def parse_voice(macro, args):
    """One voice macro -> a dict holding the raw ToneData bytes the macro assembles to."""
    pan = lambda p: (0x80 | p) if p else 0
    v = dict(macro=macro, args=args)
    if macro.startswith('voice_directsound'):
        t = {'voice_directsound': 0, 'voice_directsound_no_resample': 8, 'voice_directsound_alt': 16}[macro]
        key, p, smp, a, d, s, r = args
        v.update(kind='ds', type=t, key=num(key), length=0, pan_sweep=pan(num(p)),
                 sample=smp.replace('DirectSoundWaveData_', ''), adsr=(num(a), num(d), num(s), num(r)))
    elif macro in ('voice_square_1', 'voice_square_1_alt'):
        key, p, sweep, duty, a, d, s, r = map(num, args)
        v.update(kind='sq1', type=1 if macro == 'voice_square_1' else 9, key=key, length=pan(p),
                 pan_sweep=sweep, duty=duty & 3, adsr=(a & 7, d & 7, s & 15, r & 7))
    elif macro in ('voice_square_2', 'voice_square_2_alt'):
        key, p, duty, a, d, s, r = map(num, args)
        v.update(kind='sq2', type=2 if macro == 'voice_square_2' else 10, key=key, length=pan(p),
                 pan_sweep=0, duty=duty & 3, adsr=(a & 7, d & 7, s & 15, r & 7))
    elif macro in ('voice_programmable_wave', 'voice_programmable_wave_alt'):
        key, p, wav, a, d, s, r = args
        v.update(kind='wave', type=3 if macro == 'voice_programmable_wave' else 11, key=num(key),
                 length=pan(num(p)), pan_sweep=0, wave=wav,
                 adsr=(num(a) & 7, num(d) & 7, num(s) & 15, num(r) & 7))
    elif macro in ('voice_noise', 'voice_noise_alt'):
        key, p, period, a, d, s, r = map(num, args)
        v.update(kind='noise', type=4 if macro == 'voice_noise' else 12, key=key, length=pan(p),
                 pan_sweep=0, period=period & 1, adsr=(a & 7, d & 7, s & 15, r & 7))
    elif macro == 'voice_keysplit':
        v.update(kind='split', type=0x40, key=0, length=0, pan_sweep=0,
                 group=args[0].replace('voicegroup_', ''), table=args[1].replace('keysplit_', ''), adsr=(0, 0, 0, 0))
    elif macro == 'voice_keysplit_all':
        v.update(kind='drum', type=0x80, key=0, length=0, pan_sweep=0,
                 group=args[0].replace('voicegroup_', ''), adsr=(0, 0, 0, 0))
    else:
        die('unknown voice macro ' + macro)
    return v

def voice_sig(v):
    k = v['kind']
    base = (k, v['type'], v['key'], v['length'], v['pan_sweep']) + tuple(v['adsr'])
    if k == 'ds': return base + (v['sample'],)
    if k in ('sq1', 'sq2'): return base + (v['duty'],)
    if k == 'wave': return base + (v['wave'],)
    if k == 'noise': return base + (v['period'],)
    if k == 'split': return base + (v['group'], v['table'])
    return base + (v['group'],)

def is_placeholder(v):
    return v['kind'] == 'sq1' and v['macro'] == 'voice_square_1' and tuple(map(num, v['args'])) == (60, 0, 0, 2, 0, 0, 15, 0)

flat = []            # every voice in memory order
labels = {}          # group name -> flat index of entry 0 (may be < start for offset groups)
group_files = {}     # group name -> source file (songs vs drumsets vs keysplits)

def read_group_file(path):
    rel = os.path.relpath(path, ROOT)
    with open(path) as f:
        for line in f:
            line = line.split('@')[0].strip()
            if not line:
                continue
            m = re.match(r'(\S+)\s*(.*)', line)
            macro, rest = m.group(1), m.group(2)
            if macro == 'voice_group':
                a = parse_args(rest)
                start = num(a[1]) if len(a) > 1 else 0
                labels[a[0]] = len(flat) - start
                group_files[a[0]] = rel
            elif macro.startswith('voice_'):
                flat.append(parse_voice(macro, parse_args(rest)))
            else:
                die('%s: unexpected line %r' % (rel, line))

with open(os.path.join(SOUND, 'voice_groups.inc')) as f:
    for line in f:
        m = re.match(r'\s*\.include\s+"([^"]+)"', line)
        if m and m.group(1).startswith('sound/voicegroups/'):
            read_group_file(os.path.join(ROOT, m.group(1)))

DUMMY = parse_voice('voice_square_1', ['60', '0', '0', '2', '0', '0', '15', '0'])

def resolve(group):
    base = labels[group]
    return [flat[base + i] if 0 <= base + i < len(flat) else DUMMY for i in range(128)]

# ---------------------------------------------------------------- key split tables

ks_bytes = []
ks_labels = {}
with open(os.path.join(SOUND, 'keysplit_tables.inc')) as f:
    last_note = 0
    for line in f:
        line = line.split('@')[0].strip()
        if not line:
            continue
        m = re.match(r'(\S+)\s*(.*)', line)
        macro, a = m.group(1), parse_args(m.group(2))
        if macro == 'keysplit':
            start = num(a[1]) if len(a) > 1 else 0
            ks_labels[a[0]] = len(ks_bytes) - start
            last_note = start
        elif macro == 'split':
            idx, end = num(a[0]), num(a[1])
            ks_bytes += [idx] * (end - last_note)
            last_note = end

def resolve_ks(name):
    base = ks_labels[name]
    return [ks_bytes[base + i] if 0 <= base + i < len(ks_bytes) else 0 for i in range(128)]

# ---------------------------------------------------------------- samples

def sample_names():
    """(symbol name without DirectSoundWaveData_, wav path) in data order."""
    names, cur = [], None
    with open(os.path.join(SOUND, 'direct_sound_data.inc')) as f:
        for line in f:
            m = re.match(r'\s*DirectSoundWaveData_(\w+)::', line)
            if m:
                cur = m.group(1)
            m = re.match(r'\s*\.incbin\s+"([^"]+)\.bin"', line)
            if m and cur:
                if not cur.lower().startswith('phoneme'):      # speech pieces, not instruments
                    names.append((cur, os.path.join(ROOT, m.group(1) + '.wav')))
                cur = None
    return names

def wave_names():
    out = []
    with open(os.path.join(SOUND, 'programmable_wave_data.inc')) as f:
        cur = None
        for line in f:
            m = re.match(r'\s*(ProgrammableWaveData_\w+)::', line)
            if m:
                cur = m.group(1)
            m = re.match(r'\s*\.incbin\s+"([^"]+)"', line)
            if m and cur:
                out.append((cur, os.path.join(ROOT, m.group(1))))
                cur = None
    return out

def load_samples(tmp):
    wav2agb = os.path.join(ROOT, 'tools', 'wav2agb', 'wav2agb')
    if not os.path.exists(wav2agb) and not os.path.exists(wav2agb + '.exe'):
        subprocess.check_call(['make', '-C', os.path.dirname(wav2agb)], stdout=subprocess.DEVNULL)
    if not os.path.exists(wav2agb):
        wav2agb += '.exe'
    out = []
    for name, wav in sample_names():
        binp = os.path.join(tmp, name + '.bin')
        if not os.path.exists(binp) or os.path.getmtime(binp) < os.path.getmtime(wav):
            subprocess.check_call([wav2agb, '-b', wav, binp])
        with open(binp, 'rb') as f:
            data = f.read()
        flags, freq, loop_start, size = struct.unpack('<IIII', data[:16])
        pcm = data[16:16 + size]
        if len(pcm) < size:
            die('%s: short sample' % name)
        out.append(dict(name=name, flags=flags, freq=freq, loop_start=loop_start, size=size, pcm=pcm))
    return out

# ---------------------------------------------------------------- engine tables

def c_array(path, name):
    src = open(path).read()
    m = re.search(r'\b' + re.escape(name) + r'\s*\[[^\]]*\]\s*=\s*\{(.*?)\};', src, re.S)
    if not m:
        die('no table %s in %s' % (name, path))
    body = re.sub(r'#.*', '', m.group(1))       # keep the PORTABLE branch only (701 etc.)
    body = re.sub(r'//.*', '', body)
    vals = [v.strip().rstrip('uUfF') for v in body.replace('\n', ',').split(',')]
    return [v for v in vals if v]

def portable_table(path, name):
    """Like c_array but resolves #ifndef PORTABLE / #else / #endif by taking the PORTABLE side."""
    src = open(path).read()
    m = re.search(r'\b' + re.escape(name) + r'\s*\[[^\]]*\]\s*=\s*\{(.*?)\};', src, re.S)
    out, skip = [], False
    for line in m.group(1).split('\n'):
        s = line.strip()
        if s.startswith('#ifndef PORTABLE'): skip = True; continue
        if s.startswith('#else'): skip = not skip; continue
        if s.startswith('#endif'): skip = False; continue
        if skip: continue
        s = re.sub(r'//.*', '', s)
        out += [v.strip().rstrip('uU') for v in s.split(',') if v.strip()]
    return out

# ---------------------------------------------------------------- names, categories, GM

GM_NAMES = """Acoustic Grand Piano|Bright Acoustic Piano|Electric Grand Piano|Honky-tonk Piano|Electric Piano 1|Electric Piano 2|Harpsichord|Clavinet|Celesta|Glockenspiel|Music Box|Vibraphone|Marimba|Xylophone|Tubular Bells|Dulcimer|Drawbar Organ|Percussive Organ|Rock Organ|Church Organ|Reed Organ|Accordion|Harmonica|Tango Accordion|Nylon Guitar|Steel Guitar|Jazz Guitar|Clean Guitar|Muted Guitar|Overdriven Guitar|Distortion Guitar|Guitar Harmonics|Acoustic Bass|Finger Bass|Pick Bass|Fretless Bass|Slap Bass 1|Slap Bass 2|Synth Bass 1|Synth Bass 2|Violin|Viola|Cello|Contrabass|Tremolo Strings|Pizzicato Strings|Orchestral Harp|Timpani|String Ensemble 1|String Ensemble 2|Synth Strings 1|Synth Strings 2|Choir Aahs|Voice Oohs|Synth Voice|Orchestra Hit|Trumpet|Trombone|Tuba|Muted Trumpet|French Horn|Brass Section|Synth Brass 1|Synth Brass 2|Soprano Sax|Alto Sax|Tenor Sax|Baritone Sax|Oboe|English Horn|Bassoon|Clarinet|Piccolo|Flute|Recorder|Pan Flute|Blown Bottle|Shakuhachi|Whistle|Ocarina|Lead 1 (square)|Lead 2 (sawtooth)|Lead 3 (calliope)|Lead 4 (chiff)|Lead 5 (charang)|Lead 6 (voice)|Lead 7 (fifths)|Lead 8 (bass + lead)|Pad 1 (new age)|Pad 2 (warm)|Pad 3 (polysynth)|Pad 4 (choir)|Pad 5 (bowed)|Pad 6 (metallic)|Pad 7 (halo)|Pad 8 (sweep)|FX 1 (rain)|FX 2 (soundtrack)|FX 3 (crystal)|FX 4 (atmosphere)|FX 5 (brightness)|FX 6 (goblins)|FX 7 (echoes)|FX 8 (sci-fi)|Sitar|Banjo|Shamisen|Koto|Kalimba|Bagpipe|Fiddle|Shanai|Tinkle Bell|Agogo|Steel Drums|Woodblock|Taiko Drum|Melodic Tom|Synth Drum|Reverse Cymbal|Guitar Fret Noise|Breath Noise|Seashore|Bird Tweet|Telephone Ring|Helicopter|Applause|Gunshot""".split('|')
assert len(GM_NAMES) == 128

# What each GM program becomes: a sample name, a key-split instrument (ks:NAME), or a
# Game Boy channel (gb:sq2 duty / gb:sq1 duty / gb:wave / gb:noise).
GM_MAP = [
    'ks:piano', 'ks:piano', 'steinway_b_piano', 'ks:piano', 'sd90_classical_detuned_ep1_low', 'sd90_classical_detuned_ep1_high', 'sc88pro_harp', 'gb:sq2:1',
    'sc88pro_glockenspiel', 'sc88pro_glockenspiel', 'unknown_bell', 'unknown_bell', 'sc88pro_xylophone', 'sc88pro_xylophone', 'sc88pro_tubular_bell', 'unknown_koto_high',
    'sc88pro_organ2', 'sc88pro_organ2', 'sc88pro_organ2', 'sc88pro_church_organ3_low', 'sc88pro_accordion', 'sc88pro_accordion', 'sc88pro_accordion', 'sc88pro_accordion',
    'sc88pro_nylon_str_guitar', 'sc88pro_nylon_str_guitar', 'sc88pro_nylon_str_guitar', 'sc88pro_nylon_str_guitar', 'sc88pro_nylon_str_guitar', 'sd90_classical_overdrive_guitar', 'sd90_classical_distortion_guitar_low', 'sc88pro_nylon_str_guitar',
    'sc88pro_fingered_bass', 'sc88pro_fingered_bass', 'sc88pro_fingered_bass', 'sc88pro_fretless_bass', 'sc88pro_slap_bass', 'sc88pro_slap_bass', 'sc88pro_synth_bass', 'trinity_30303_mega_bass',
    'ks:strings', 'ks:strings', 'ks:strings', 'ks:strings', 'ks:strings', 'sc88pro_pizzicato_strings', 'sc88pro_harp', 'sc88pro_timpani',
    'ks:strings', 'ks:strings', 'ks:strings', 'ks:strings', 'classical_choir_voice_ahhs', 'classical_choir_voice_ahhs', 'unknown_female_voice', 'sc88pro_timpani_with_snare',
    'ks:trumpet', 'ks:french_horn', 'ks:tuba', 'ks:trumpet', 'ks:french_horn', 'ks:trumpet', 'ks:trumpet', 'ks:french_horn',
    'sd90_classical_oboe', 'sd90_classical_oboe', 'sd90_classical_oboe', 'sd90_classical_oboe', 'sd90_classical_oboe', 'sd90_classical_oboe', 'sd90_classical_oboe', 'sc88pro_flute',
    'sc88pro_flute', 'sc88pro_flute', 'sc88pro_flute', 'sd90_classical_shakuhachi', 'sd90_classical_shakuhachi', 'sd90_classical_shakuhachi', 'sd90_classical_whistle', 'gb:wave',
    'gb:sq2:2', 'gb:wave', 'gb:wave', 'gb:wave', 'sd90_classical_distortion_guitar_high', 'unknown_female_voice', 'gb:sq1:2', 'gb:sq2:1',
    'ks:strings', 'ks:strings', 'ks:strings', 'classical_choir_voice_ahhs', 'ks:strings', 'sc88pro_tubular_bell', 'classical_choir_voice_ahhs', 'ks:strings',
    'sc88pro_bubbles', 'ks:strings', 'sc88pro_glockenspiel', 'sc88pro_nylon_str_guitar', 'sc88pro_glockenspiel', 'classical_choir_voice_ahhs', 'unknown_female_voice', 'gb:sq2:0',
    'unknown_koto_high', 'sc88pro_nylon_str_guitar', 'unknown_koto_low', 'unknown_koto_high', 'sc88pro_xylophone', 'sc88pro_accordion', 'ks:strings', 'sd90_classical_oboe',
    'sc88pro_jingle_bell', 'sd90_cowbell', 'heart_of_asia_gamelan', 'ethnic_flavours_hyoushigi', 'sc88pro_taiko', 'sd90_ambient_tom', 'unused_sc55_tom', 'sc88pro_orchestra_cymbal_crash',
    'sc88pro_nylon_str_guitar', 'sc88pro_wind', 'sc88pro_wind', 'sd90_classical_whistle', 'bicycle_bell', 'gb:noise', 'gb:noise', 'gb:noise',
]
assert len(GM_MAP) == 128

PRETTY = {
    'sc88pro': 'SC-88Pro', 'sd90': 'SD-90', 'trinity': 'Trinity', 'unknown': 'Unknown', 'unused': 'Unused',
    'ethnic_flavours': 'Ethnic', 'heart_of_asia': 'Heart of Asia', 'classical': '', 'dance_drums': 'Dance',
    'drum_and_percussion': 'Drum & Perc',
}

def pretty_sample(name):
    for pre, label in sorted(PRETTY.items(), key=lambda kv: -len(kv[0])):
        if name.startswith(pre + '_'):
            rest = name[len(pre) + 1:]
            if rest.startswith('classical_'):
                rest = rest[len('classical_'):]
            words = rest.replace('_', ' ').title()
            return words + (' (%s)' % label if label else '')
    return name.replace('_', ' ').title()

CATS = ['Piano & Keys', 'Bells & Mallets', 'Organ & Accordion', 'Guitar', 'Bass', 'Strings & Harp',
        'Voice & Choir', 'Brass', 'Woodwind', 'Percussion', 'Effects', 'GB Square', 'GB Wave', 'GB Noise',
        'Key-split', 'Drum kit']

def sample_category(name):
    n = name
    rules = [
        ('Piano & Keys', ['piano', '_ep1']),
        ('Bells & Mallets', ['glockenspiel', 'xylophone', 'bell', 'gamelan', 'triangle']),
        ('Organ & Accordion', ['organ', 'accordion']),
        ('Guitar', ['guitar', 'koto']),
        ('Bass', ['bass', 'big_boned']),
        ('Strings & Harp', ['string', 'harp', 'pizzicato']),
        ('Voice & Choir', ['choir', 'voice', 'scream']),
        ('Brass', ['trumpet', 'tuba', 'horn']),
        ('Woodwind', ['flute', 'oboe', 'shaku', 'whistle']),
        ('Percussion', ['kick', 'snare', 'hihat', 'tom', 'conga', 'cymbal', 'crash', 'clap', 'tambourine', 'timpani',
                        'taiko', 'cowbell', 'atarigane', 'hyoushigi', 'tsuzumi', 'drum', 'ride']),
    ]
    for cat, keys in rules:
        if any(k in n for k in keys):
            return cat
    return 'Effects'

# ---------------------------------------------------------------- songs

def song_table():
    songs = []
    with open(os.path.join(SOUND, 'song_table.inc')) as f:
        for line in f:
            m = re.match(r'\s*song\s+(\w+)\s*,\s*(\w+)\s*,', line)
            if m:
                songs.append((m.group(1), m.group(2)))
    return songs

def midi_cfg():
    out = {}
    with open(os.path.join(SOUND, 'songs', 'midi', 'midi.cfg')) as f:
        for line in f:
            m = re.match(r'\s*(\w+)\.mid\s*:\s*(.*)', line)
            if not m:
                continue
            opts = dict(group='dummy', vol=127, rev=-1, pri=0, exact=0)
            for o in m.group(2).split():
                if o.upper().startswith('-G'): opts['group'] = o[2:].lstrip('_')
                elif o.upper().startswith('-V'): opts['vol'] = int(o[2:])
                elif o.upper().startswith('-R'): opts['rev'] = int(o[2:])
                elif o.upper().startswith('-P'): opts['pri'] = int(o[2:])
                elif o.upper() == '-E': opts['exact'] = 1
            out[m.group(1)] = opts
    return out

# ---------------------------------------------------------------- main

def main():
    if len(sys.argv) != 2:
        die('usage: gen_soundfont.py OUT_DIR')
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    tmp = os.path.join(outdir, 'bins')
    os.makedirs(tmp, exist_ok=True)

    samples = load_samples(tmp)
    sidx = {s['name']: i for i, s in enumerate(samples)}
    waves = wave_names()
    widx = {w[0]: i for i, w in enumerate(waves)}

    song_groups = [g for g, f in group_files.items() if '/drumsets/' not in f and '/keysplits/' not in f and g != 'dummy']
    drum_groups = [g for g, f in group_files.items() if '/drumsets/' in f]
    split_groups = [g for g, f in group_files.items() if '/keysplits/' in f]

    # Sub-groups the tool keeps as 128-voice tables: every drum kit and key-split group,
    # plus whatever a voice in them points at.
    sub_names = []
    def need_group(g):
        if g not in sub_names:
            sub_names.append(g)
    for g in drum_groups + split_groups:
        need_group(g)
    for g in song_groups + drum_groups + split_groups:
        for v in resolve(g):
            if v['kind'] in ('drum', 'split'):
                need_group(v['group'])
    ks_names = list(ks_labels.keys())

    # Presets: every distinct voice the songs use, counted.
    uses = collections.OrderedDict()
    for g in song_groups:
        for i, v in enumerate(resolve(g)):
            if is_placeholder(v):
                continue
            sig = voice_sig(v)
            if sig not in uses:
                uses[sig] = dict(voice=v, songs=[], count=0)
            uses[sig]['count'] += 1
            if g not in uses[sig]['songs']:
                uses[sig]['songs'].append(g)
    # Drum kits and key-split instruments always appear, even if no song points at them.
    for g in drum_groups:
        v = parse_voice('voice_keysplit_all', ['voicegroup_' + g])
        uses.setdefault(voice_sig(v), dict(voice=v, songs=[], count=0))
    split_table_for = {'piano_keysplit': 'piano', 'strings_keysplit': 'strings', 'trumpet_keysplit': 'trumpet',
                       'tuba_keysplit': 'tuba', 'french_horn_keysplit': 'french_horn'}
    for g in split_groups:
        if g in split_table_for:
            v = parse_voice('voice_keysplit', ['voicegroup_' + g, 'keysplit_' + split_table_for[g]])
            uses.setdefault(voice_sig(v), dict(voice=v, songs=[], count=0))

    # Every sample can be picked as a plain instrument, even ones only drum kits use.
    have = {u['voice']['sample'] for u in uses.values() if u['voice']['kind'] == 'ds' and u['voice']['type'] == 0}
    for smp in samples:
        if smp['name'] not in have:
            v = parse_voice('voice_directsound', ['60', '0', 'DirectSoundWaveData_' + smp['name'], '255', '0', '255', '165'])
            uses.setdefault(voice_sig(v), dict(voice=v, songs=[], count=0))
    for wname, _ in waves:
        if not any(u['voice']['kind'] == 'wave' and u['voice']['wave'] == wname for u in uses.values()):
            v = parse_voice('voice_programmable_wave_alt', ['60', '0', wname, '0', '7', '15', '1'])
            uses.setdefault(voice_sig(v), dict(voice=v, songs=[], count=0))

    presets = []
    for sig, u in uses.items():
        v = u['voice']
        k = v['kind']
        if k == 'ds':
            if v['sample'] not in sidx:
                continue
            cat = sample_category(v['sample'])
            name = pretty_sample(v['sample'])
        elif k in ('sq1', 'sq2'):
            cat = 'GB Square'
            name = 'Square %s %s' % ('1' if k == 'sq1' else '2', ['12.5%', '25%', '50%', '75%'][v['duty']])
            if k == 'sq1' and v['pan_sweep'] and (v['pan_sweep'] & 0x70):
                name += ' sweep %d' % v['pan_sweep']
        elif k == 'wave':
            cat = 'GB Wave'
            name = 'Wave ' + v['wave'].replace('ProgrammableWaveData_', '')
        elif k == 'noise':
            cat = 'GB Noise'
            name = 'Noise ' + ('periodic' if v['period'] else 'white')
        elif k == 'split':
            cat = 'Key-split'
            name = v['group'].replace('_keysplit', '').replace('_', ' ').title() + ' (key-split)'
        else:
            cat = 'Drum kit'
            name = v['group'].replace('_', ' ').title()
        presets.append(dict(sig=sig, voice=v, cat=cat, name=name, songs=u['songs'], count=u['count']))

    cat_order = {c: i for i, c in enumerate(CATS)}
    presets.sort(key=lambda p: (cat_order[p['cat']], p['name'], -p['count']))
    # Number the variants of one name ("Flute (SC-88Pro) 2") so every row reads differently.
    seen = collections.Counter(p['name'] for p in presets)
    nth = collections.Counter()
    for p in sorted(presets, key=lambda p: -p['count']):
        if seen[p['name']] > 1:
            nth[p['name']] += 1
            if nth[p['name']] > 1:
                p['name'] += ' #%d' % nth[p['name']]
    presets.sort(key=lambda p: (cat_order[p['cat']], p['name'].split(' #')[0], int(p['name'].split(' #')[1]) if ' #' in p['name'] else 1))
    pidx = {p['sig']: i for i, p in enumerate(presets)}

    def best_preset(pred):
        cands = [p for p in presets if pred(p['voice'])]
        if not cands:
            return -1
        return pidx[max(cands, key=lambda p: p['count'])['sig']]

    gm = []
    for prog, spec in enumerate(GM_MAP):
        if spec.startswith('ks:'):
            t = spec[3:]
            i = best_preset(lambda v: v['kind'] == 'split' and v['table'] == t)
        elif spec.startswith('gb:sq'):
            _, kind, duty = spec.split(':')
            i = best_preset(lambda v: v['kind'] == kind and v['duty'] == int(duty))
        elif spec == 'gb:wave':
            i = best_preset(lambda v: v['kind'] == 'wave')
        elif spec == 'gb:noise':
            i = best_preset(lambda v: v['kind'] == 'noise')
        else:
            i = best_preset(lambda v: v['kind'] == 'ds' and v['sample'] == spec)
        if i < 0:
            die('GM %d (%s): nothing matches %s' % (prog, GM_NAMES[prog], spec))
        gm.append(i)
    gm_drum = best_preset(lambda v: v['kind'] == 'drum' and v['group'] == 'rs_drumset')

    # ------------------------------------------------------------ write sf_pcm.bin
    pcm_off = []
    with open(os.path.join(outdir, 'sf_pcm.bin'), 'wb') as f:
        for s in samples:
            pcm_off.append(f.tell())
            f.write(s['pcm'])
            # The game's mixer reads one byte past a sample's end; in the game that byte is the
            # .bin's zero padding or the next sample's header (flags, low byte 0). Keep it 0 here too.
            f.write(b'\0' * ((-f.tell()) % 4))
            f.write(b'\0' * 16)
        for name, path in waves:
            pcm_off.append(f.tell())
            f.write(open(path, 'rb').read()[:16].ljust(16, b'\0'))

    # ------------------------------------------------------------ write sf_counts.h / sf_data.c
    def cstr(s):
        return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

    def voice_c(v):
        k = v['kind']
        ref, sub, ks = -1, -1, -1
        if k == 'ds': ref = sidx.get(v['sample'], -1)
        elif k in ('sq1', 'sq2'): ref = v['duty']
        elif k == 'wave': ref = widx[v['wave']]
        elif k == 'noise': ref = v['period']
        elif k == 'split':
            sub = sub_names.index(v['group']); ks = ks_names.index(v['table'])
        else:
            sub = sub_names.index(v['group'])
        a, d, s, r = v['adsr']
        return '{%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d}' % (v['type'], v['key'], v['length'], v['pan_sweep'], ref, sub, ks, a, d, s, r)

    L = []
    w = L.append
    w('/* Generated by tools/moemon_music/gen_soundfont.py from the game\'s sound folder. Do not edit. */')
    w('#define SF_NUM_SAMPLES %d' % len(samples))
    w('#define SF_NUM_WAVES %d' % len(waves))
    w('#define SF_NUM_GROUPS %d' % len(sub_names))
    w('#define SF_NUM_KEYSPLITS %d' % len(ks_names))
    w('#define SF_NUM_PRESETS %d' % len(presets))
    w('#define SF_NUM_CATS %d' % len(CATS))
    w('#define SF_GM_DRUM_PRESET %d' % gm_drum)
    w('static const SfSample sf_samples[SF_NUM_SAMPLES] = {')
    for s, off in zip(samples, pcm_off):
        w('  {%s, %du, %du, %du, %du, %du},' % (cstr(s['name']), s['flags'], s['freq'], s['loop_start'], s['size'], off))
    w('};')
    w('static const SfWave sf_waves[SF_NUM_WAVES] = {')
    for (name, _), off in zip(waves, pcm_off[len(samples):]):
        w('  {%s, %du},' % (cstr(name), off))
    w('};')
    w('static const char *const sf_group_names[SF_NUM_GROUPS] = {%s};' % ', '.join(cstr(g) for g in sub_names))
    w('static const SfVoice sf_groups[SF_NUM_GROUPS][128] = {')
    for g in sub_names:
        w('  {' + ','.join(voice_c(v) for v in resolve(g)) + '},')
    w('};')
    w('static const char *const sf_keysplit_names[SF_NUM_KEYSPLITS] = {%s};' % ', '.join(cstr(k) for k in ks_names))
    w('static const unsigned char sf_keysplits[SF_NUM_KEYSPLITS][128] = {')
    for k in ks_names:
        w('  {' + ','.join(str(b) for b in resolve_ks(k)) + '},')
    w('};')
    w('static const char *const sf_cat_names[SF_NUM_CATS] = {%s};' % ', '.join(cstr(c) for c in CATS))
    w('static const SfPreset sf_presets[SF_NUM_PRESETS] = {')
    for p in presets:
        songs = ', '.join(p['songs'][:6]) + (', ...' if len(p['songs']) > 6 else '')
        w('  {%s, %d, %s, %d, %s},' % (cstr(p['name']), cat_order[p['cat']], cstr(songs), p['count'], voice_c(p['voice'])))
    w('};')
    w('static const char *const sf_gm_names[128] = {%s};' % ', '.join(cstr(n) for n in GM_NAMES))
    w('static const short sf_gm_preset[128] = {%s};' % ','.join(str(i) for i in gm))

    # Songs the game can play as music, with what midi.cfg built them with.
    cfg = midi_cfg()
    bgm = [n for n, player in song_table() if player == 'MUSIC_PLAYER_BGM' and n not in ('mus_dummy', 'dummy_song_header')]
    w('#define SF_NUM_SONGS %d' % len(bgm))
    w('static const SfSong sf_songs[SF_NUM_SONGS] = {')
    for n in bgm:
        c = cfg.get(n, dict(group='dummy', vol=127, rev=-1, pri=0, exact=1))
        w('  {%s, %s, %d, %d, %d, %d},' % (cstr(n), cstr(c['group']), c['vol'], c['rev'], c['pri'], c['exact']))
    w('};')
    # Where each song plays (tools/kaisong/gen.py: the SONG > Replaces picker and the SONG_TABLE.md written next to exports)
    sys.path.insert(0, os.path.join(ROOT, 'tools', 'kaisong'))
    import gen as kaigen
    rows = kaigen.places()
    blocks = kaigen.song_blocks(rows)
    assert [b[0] for b in blocks] == bgm, 'tools/kaisong/gen.py and sf_songs list different songs'
    kinds = [k for k, _ in kaigen.KIND_TITLES]
    w('#define SF_NUM_KINDS %d' % len(kinds))
    w('static const char *const sf_kind_names[SF_NUM_KINDS] = {%s};' % ', '.join(cstr(t.upper()) for _, t in kaigen.KIND_TITLES))
    w('#define SF_NUM_PLACES %d' % len(rows))
    w('static const SfPlace sf_places[SF_NUM_PLACES] = {')
    for r in rows:
        w('  {%d, %d, %s, %s},' % (bgm.index(r[0]), kinds.index(r[1]), cstr(r[2]), cstr(r[3])))
    w('};')
    mapnames = kaigen.map_names()
    w('#define SF_NUM_MAPS %d' % len(mapnames))
    w('static const SfMap sf_maps[SF_NUM_MAPS] = {%s};' % ', '.join('{%s, %s}' % (cstr(k), cstr(n)) for k, n in mapnames))
    cnl = lambda t: cstr(t).replace('\n', '\\n')   # a C string with its line breaks as \n
    w('static const char sf_table_intro[] = %s;' % cnl('\n'.join(kaigen.MD_INTRO) + '\n'))
    w('static const short sf_song_num[SF_NUM_SONGS] = {%s};' % ','.join(str(b[1]) for b in blocks))
    w('static const char *const sf_song_md[SF_NUM_SONGS] = {')
    for b in blocks:
        w('  %s,' % cnl(''.join(l + '\n' for l in b[2])))
    w('};')
    # Every song voicegroup, so a game MIDI can be heard with the exact instruments it ships with.
    w('#define SF_NUM_SONG_GROUPS %d' % len(song_groups))
    w('static const char *const sf_song_group_names[SF_NUM_SONG_GROUPS] = {%s};' % ', '.join(cstr(g) for g in song_groups))
    w('static const SfVoice sf_song_groups[SF_NUM_SONG_GROUPS][128] = {')
    for g in song_groups:
        w('  {' + ','.join(voice_c(v) for v in resolve(g)) + '},')
    w('};')

    m4a = os.path.join(ROOT, 'src', 'm4a_tables.c')
    cgb = os.path.join(ROOT, 'include', 'cgb_tables.h')
    for name, ctype in [('gScaleTable', 'unsigned char'), ('gFreqTable', 'unsigned int'), ('gClockTable', 'unsigned char'),
                        ('gCgbScaleTable', 'unsigned char'), ('gCgbFreqTable', 'short'), ('gNoiseTable', 'unsigned char'),
                        ('gCgb3Vol', 'unsigned char')]:
        vals = portable_table(m4a, name)
        w('static const %s sf_%s[%d] = {%s};' % (ctype, name, len(vals), ','.join(vals)))
    for name, ctype in [('PU0', 'short'), ('PU1', 'short'), ('PU2', 'short'), ('PU3', 'short'),
                        ('freqTable', 'float'), ('freqTableNSE', 'float')]:
        vals = c_array(cgb, name)
        if ctype == 'float':
            vals = [v if re.search(r'[.eE]', v) else v + '.0' for v in vals]
            vals = [v + 'f' for v in vals]
        w('static const %s sf_%s[%d] = {%s};' % (ctype, name, len(vals), ','.join(vals)))

    # mid2agb's rounding tables, so songs convert exactly like the game build converts them.
    lut = os.path.join(ROOT, 'tools', 'mid2agb', 'tables.cpp')
    for name in ('g_noteDurationLUT', 'g_noteVelocityLUT'):
        vals = c_array(lut, name)
        w('static const unsigned char sf_%s[%d] = {%s};' % (name[2:], len(vals), ','.join(vals)))

    # sf_counts.h: the sizes; sf_data.c: the tables (sf.h declares them).
    counts = [l for l in L if l.startswith('#define')]
    body = ['/* Generated by tools/moemon_music/gen_soundfont.py from the game\'s sound folder. Do not edit. */',
            '#include "sf.h"'] + [l.replace('static const', 'const', 1) for l in L if not l.startswith('#define') and not l.startswith('/*')]
    with open(os.path.join(outdir, 'sf_counts.h'), 'w') as f:
        f.write('\n'.join(L[:1] + counts) + '\n')
    with open(os.path.join(outdir, 'sf_data.c'), 'w') as f:
        f.write('\n'.join(body) + '\n')
    print('gen_soundfont: %d samples, %d waves, %d presets, %d kits/key-split groups, %d songs'
          % (len(samples), len(waves), len(presets), len(sub_names), len(bgm)))

main()
