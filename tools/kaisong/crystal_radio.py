#!/usr/bin/env python3
"""JOHTO's radio music from Pokemon Crystal itself: pret/pokecrystal's own sound engine and songs, built into a GBS
(a Game Boy music file) by a small driver, then through Moemon Music Studio's command line into
music/<song>.kaisong (the POKeGEAR's radio, src/mp/mp_pokegear.c). No ROM is needed: the music is assembled from the
decomp's source with rgbds (pokecrystal's .rgbds-version); nothing built here is committed but the .kaisong files.

    python3 -I tools/kaisong/crystal_radio.py slots                     # the song slots: include/constants/songs.h and
                                                                         # sound/song_table.inc (between their markers)
    python3 -I tools/kaisong/crystal_radio.py gbs POKECRYSTAL RGBDS OUT.gbs   # POKECRYSTAL built once (make crystal)
    python3 -I tools/kaisong/crystal_radio.py convert CRYSTAL.gbs [STUDIO]    # music/<slot>.kaisong

The GBS: the decomp's audio.o and ram.o linked with gbs.asm below into banks 1-7 (a GBS player banks like MBC1, so
the songs' own banks 0x07-0x5E can't stay where the ROM has them); track N plays Crystal's song N
(constants/music_constants.asm), stereo on.
"""
import os
import struct
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIRST = 816   # after SINNOH's (include/constants/songs.h)

# (slot, Crystal's song number, the game's song it plays without its .kaisong); the stations: mp_pokegear.c sStations
SONGS = [
    ('pokemon_talk', 0x1D, 'mus_johto_prof_oak'),        # 4.5 OAK's POKeMON TALK
    ('pokedex_show', 0x09, 'mus_johto_pokecenter'),      # 4.5 in the morning: the POKeDEX SHOW
    ('pokemon_march', 0x51, 'mus_johto_goldenrod'),      # 7.5 POKeMON MUSIC, 18.5 LET'S ALL SING (Sun, Tue, Thu, Sat)
    ('pokemon_lullaby', 0x50, 'mus_johto_azalea'),       # 7.5 (Mondays, Wednesdays, Fridays)
    ('lucky_channel', 0x12, 'mus_johto_game_corner'),    # 8.5 LUCKY CHANNEL
    ('buenas_password', 0x60, 'mus_johto_goldenrod'),    # 10.5 BUENA's PASSWORD
    ('unown', 0x4B, 'mus_johto_ruins_of_alph'),          # 13.5 at the RUINS OF ALPH
    ('places_and_people', 0x15, 'mus_rg_vermillion'),    # 16.5 out of JOHTO
    # (Crystal's song list gives LET'S ALL SING the BICYCLE song, but its show plays the MARCH / LULLABY)
    ('poke_flute', 0x40, 'mus_rg_poke_flute'),           # 20.0 out of JOHTO
    ('evolution', 0x5A, 'mus_johto_rocket_hideout'),     # 20.5 around MAHOGANY while TEAM ROCKET is there
    ('rocket', 0x56, 'mus_johto_rocket_hideout'),        # every JOHTO station while they hold the RADIO TOWER
]
H_BEGIN = '// Moemon Kai: JOHTO\'s radio (tools/kaisong/crystal_radio.py)'
H_END = '// end of JOHTO\'s radio'
T_BEGIN = '@ Moemon Kai: JOHTO\'s radio (tools/kaisong/crystal_radio.py)'
T_END = '@ end of JOHTO\'s radio'

DRIVER = r'''
SECTION "GBS Home", ROM0[$0400]
GBSInit::
	ld e, a
	ld d, 0
	push de
	ld a, 1
	ldh [hROMBank], a
	ld [rROMB], a
	ld a, 1 << STEREO
	ld [wOptions], a
	call InitSound
	pop de
	jp PlayMusic
GBSPlay::
	ld a, BANK(_UpdateSound)
	ldh [hROMBank], a
	ld [rROMB], a
	jp _UpdateSound
InitSound::
	ld a, BANK(_InitSound)
	ldh [hROMBank], a
	ld [rROMB], a
	jp _InitSound
PlayMusic::
	ld a, BANK(_PlayMusic)
	ldh [hROMBank], a
	ld [rROMB], a
	ld a, e
	and a
	jp z, _InitSound
	jp _PlayMusic
_LoadMusicByte::
	ldh [hROMBank], a
	ld [rROMB], a
	ld a, [de]
	ld [wCurMusicByte], a
	ld a, BANK(LoadMusicByte)
	ldh [hROMBank], a
	ld [rROMB], a
	ret
MaxVolume::
	ld a, MAX_VOLUME
	ld [wVolume], a
	ret
DelayFrame::
	ret
'''
LAYOUT = '''ROMX $01
\t"Audio"
\t"Songs 1"
ROMX $02
\t"Songs 2"
ROMX $03
\t"Songs 3"
\t"Sound Effects"
\t"Cries"
ROMX $04
\t"Songs 4"
ROMX $05
\t"Songs 5"
\t"Crystal Sound Effects"
ROMX $06
\t"Extra Songs 1"
ROMX $07
\t"Extra Songs 2"
'''


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
    body = ''.join('#define MUS_JOHTO_RADIO_%-22s %d\n' % (s[0].upper(), FIRST + i) for i, s in enumerate(SONGS))
    text = open(h).read()
    open(h, 'w').write(replace_block(text, H_BEGIN, H_END, body, '// end of SINNOH\'s songs\n'))
    body = ''.join('\t.set mus_johto_radio_%s, %s\n' % (s[0], s[2]) for s in SONGS)
    body += ''.join('\tsong mus_johto_radio_%s, MUSIC_PLAYER_BGM, 0\n' % s[0] for s in SONGS)
    text = open(t).read()
    open(t, 'w').write(replace_block(text, T_BEGIN, T_END, body, '@ end of SINNOH\'s songs\n'))
    print('%d radio slots, %d-%d' % (len(SONGS), FIRST, FIRST + len(SONGS) - 1))


def gbs(crystal, rgbds, out):
    if not os.path.exists(os.path.join(crystal, 'audio.o')):
        raise SystemExit('%s: build it first (make crystal, with rgbds %s)' % (
            crystal, open(os.path.join(crystal, '.rgbds-version')).read().strip()))
    with tempfile.TemporaryDirectory() as tmp:
        for name, text in (('gbs.asm', DRIVER), ('gbs.link', LAYOUT)):
            with open(os.path.join(tmp, name), 'w') as f:
                f.write(text)
        subprocess.run([os.path.join(rgbds, 'rgbasm'), '-Q8', '-P', 'includes.asm', '-o', os.path.join(tmp, 'gbs.o'),
                        os.path.join(tmp, 'gbs.asm')], cwd=crystal, check=True)
        subprocess.run([os.path.join(rgbds, 'rgblink'), '-l', os.path.join(tmp, 'gbs.link'), '-n',
                        os.path.join(tmp, 'gbs.sym'), '-o', os.path.join(tmp, 'gbs.gb'), os.path.join(tmp, 'gbs.o'),
                        'audio.o', 'ram.o'], cwd=crystal, check=True)
        rom = open(os.path.join(tmp, 'gbs.gb'), 'rb').read()
        syms = {}
        for line in open(os.path.join(tmp, 'gbs.sym')):
            p = line.split()
            if len(p) == 2 and not line.startswith(';'):
                syms[p[1]] = int(p[0].split(':')[1], 16)
    count = 0x67   # NUM_MUSIC_SONGS
    head = b'GBS' + bytes([1, count, 1]) + struct.pack('<HHHH', 0x400, syms['GBSInit'], syms['GBSPlay'], 0xE000)
    head += bytes([0, 0]) + b'Pokemon Crystal'.ljust(32, b'\0') + b'Junichi Masuda, Go Ichinose'.ljust(32, b'\0')
    head += b'2000 Nintendo'.ljust(32, b'\0')
    open(out, 'wb').write(head + rom[0x400:])
    print(out, len(head) + len(rom) - 0x400, 'bytes')


def convert(gbsfile, studio):
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    import add_loop
    out = os.path.join(ROOT, 'music')
    failed = 0
    for name, num, _ in SONGS:   # (a playlist names its .gbs beside it)
        song = 'mus_johto_radio_' + name
        m3u = os.path.join(os.path.dirname(os.path.abspath(gbsfile)), song + '.m3u')
        with open(m3u, 'w') as f:
            f.write('%s::GBS,%d,%s,,,\n' % (os.path.basename(gbsfile), num, name.replace('_', ' ').title()))
        r = subprocess.run([studio, '--export', m3u, os.path.join(out, song + '.kaisong'), song], capture_output=True, text=True)
        path = os.path.join(out, song + '.kaisong')
        ok = r.returncode == 0 and os.path.exists(path)
        if ok:
            add_loop.add_loop(path)
        print('%-34s %s' % (song, 'ok' if ok else 'FAILED ' + (r.stdout + r.stderr)[-300:]), flush=True)
        failed += not ok
        os.remove(m3u)
    return failed


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == 'slots':
        slots()
    elif len(sys.argv) == 5 and sys.argv[1] == 'gbs':
        gbs(sys.argv[2], sys.argv[3], sys.argv[4])
    elif len(sys.argv) >= 3 and sys.argv[1] == 'convert':
        studio = sys.argv[3] if len(sys.argv) > 3 else os.path.join(ROOT, 'tools/moemon_music/build/linux/moemon_music_studio')
        sys.exit(1 if convert(sys.argv[2], studio) else 0)
    else:
        print(__doc__)
        sys.exit(2)


if __name__ == '__main__':
    main()
