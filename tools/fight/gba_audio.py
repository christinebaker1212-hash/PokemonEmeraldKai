#!/usr/bin/env python3
"""
Moemon Fight: brings the VS cabinet's sounds and songs down to the GBA's sound.

Emerald plays its samples as 8-bit PCM at 13379 Hz (DirectSound), and the fight's mixer plays
everything that way already (fight/compat/mf_mixer.c: 13379 Hz, 8 bits, effects in mono). The
files shipped in mugen/ were 8- and 16-bit, mono and stereo, at 11-96 kHz; this rewrites them
to what is heard, so they load faster and take a fraction of the space:

  .snd sound banks   every WAV inside becomes 8-bit mono PCM at 13379 Hz (a clip recorded
                     below that keeps its own rate: raising it adds nothing)
  songs              MP3 / FLAC / WAV / OGG above 13379 Hz become Ogg Vorbis at 13379 Hz,
                     stereo (the mixer keeps music stereo, as the game's own music); every
                     .def that names the old file is pointed at the new one, and loop points
                     given in samples (bgmloopstart / bgmloopend) are scaled to the new rate

A file that is already there is left alone, so a second run changes nothing. Needs ffmpeg
(with libvorbis) on the PATH.

  gba_audio.py [--dry-run] [--quiet] [ROOT]      ROOT: the mugen/ folder (default: ./mugen)
  gba_audio.py --check [ROOT]                    lists what would change; exit 1 if anything
  gba_audio.py --snd FILE... / --song FILE...    only these files (no .def updates for songs
                                                 unless ROOT is given with --root)

Used by tools/fight/ingest.py for characters, stages and songs added later.
"""
import argparse
import os
import re
import struct
import subprocess
import sys

GBA_RATE = 13379
SONG_EXTS = ('.mp3', '.ogg', '.flac', '.wav')
OGG_QUALITY = '3'       # about 45 kbit/s at 13379 Hz stereo


# ------------------------------------------------------------------ WAV clips

def wav_format(wav):
    """(tag, channels, rate, bits, data) of a RIFF WAV, or None."""
    if len(wav) < 12 or wav[:4] != b'RIFF' or wav[8:12] != b'WAVE':
        return None
    p = 12
    fmt = None
    data = None
    while p + 8 <= len(wav):
        cid = wav[p:p + 4]
        size = struct.unpack_from('<I', wav, p + 4)[0]
        body = wav[p + 8:p + 8 + size]
        if cid == b'fmt ' and len(body) >= 16:
            fmt = struct.unpack_from('<HHIIHH', body, 0)
        elif cid == b'data':
            data = body
        p += 8 + size + (size & 1)
    if fmt is None or data is None:
        return None
    tag, channels, rate, _, _, bits = fmt
    return tag, channels, rate, bits, data


def is_gba_clip(wav):
    f = wav_format(wav)
    return f is not None and f[0] == 1 and f[1] == 1 and f[3] == 8 and f[2] <= GBA_RATE


def make_wav_u8(pcm, rate):
    return (b'RIFF' + struct.pack('<I', 36 + len(pcm) + (len(pcm) & 1)) + b'WAVE'
            + b'fmt ' + struct.pack('<IHHIIHH', 16, 1, 1, rate, rate, 1, 8)
            + b'data' + struct.pack('<I', len(pcm)) + pcm + (b'\x80' if len(pcm) & 1 else b''))


def convert_clip(wav):
    """The clip as 8-bit mono PCM at 13379 Hz (or its own lower rate)."""
    f = wav_format(wav)
    if f is None:
        return wav
    rate = min(f[2], GBA_RATE)
    pcm = subprocess.run(
        ['ffmpeg', '-hide_banner', '-loglevel', 'error', '-f', 'wav', '-i', 'pipe:0',
         '-ac', '1', '-af', 'aresample=resampler=swr:filter_size=32:cutoff=0.95',
         '-ar', str(rate), '-f', 'u8', '-c:a', 'pcm_u8', 'pipe:1'],
        input=wav, stdout=subprocess.PIPE, check=True).stdout
    if not pcm:
        return wav
    return make_wav_u8(pcm, rate)


# ------------------------------------------------------------------ .snd banks

def read_snd(data):
    """(header bytes 0x00-0x200, [(group, number, wav)]) of an Elecbyte .snd."""
    if data[:11] != b'ElecbyteSnd':
        raise ValueError('not an Elecbyte .snd')
    count, first = struct.unpack_from('<ii', data, 16)
    sounds = []
    off = first
    seen = set()
    while off and off + 16 <= len(data) and len(sounds) < count and off not in seen:
        seen.add(off)
        nxt, size, group, number = struct.unpack_from('<iiii', data, off)
        sounds.append((group, number, data[off + 16:off + 16 + size]))
        off = nxt
    return data[:0x200], sounds


def write_snd(header, sounds):
    out = bytearray(header)
    struct.pack_into('<ii', out, 16, len(sounds), 0x200 if sounds else 0)
    off = 0x200
    for i, (group, number, wav) in enumerate(sounds):
        nxt = off + 16 + len(wav) if i + 1 < len(sounds) else 0
        out += struct.pack('<iiii', nxt, len(wav), group, number) + wav
        off += 16 + len(wav)
    return bytes(out)


def snd_needs(path):
    _, sounds = read_snd(open(path, 'rb').read())
    return sum(1 for _, _, w in sounds if wav_format(w) is not None and not is_gba_clip(w))


def convert_snd(path, dry_run=False):
    """Rewrites the bank; returns (clips changed, bytes before, bytes after)."""
    data = open(path, 'rb').read()
    header, sounds = read_snd(data)
    changed = 0
    out = []
    for group, number, wav in sounds:
        if wav_format(wav) is not None and not is_gba_clip(wav):
            if not dry_run:
                wav = convert_clip(wav)
            changed += 1
        out.append((group, number, wav))
    if changed == 0:
        return 0, len(data), len(data)
    new = write_snd(header, out) if not dry_run else data
    if not dry_run:
        write_atomic(path, new)
    return changed, len(data), len(new)


# ------------------------------------------------------------------ songs

def probe_rate(path):
    out = subprocess.run(['ffprobe', '-v', 'error', '-select_streams', 'a:0', '-show_entries',
                          'stream=sample_rate,codec_name', '-of', 'csv=p=0', path],
                         stdout=subprocess.PIPE, text=True).stdout.strip()
    if not out:
        return None, None
    codec, rate = (out.split(',') + [''])[:2]
    try:
        return codec, int(rate)
    except ValueError:
        return codec, None


def song_needs(path):
    codec, rate = probe_rate(path)
    if rate is None:
        return False
    return not (codec == 'vorbis' and path.lower().endswith('.ogg') and rate <= GBA_RATE)


def convert_song(path, dry_run=False):
    """Writes <name>.ogg at 13379 Hz next to the song (the old file is removed); returns
    (new path, old rate) or (None, None) when nothing changed."""
    codec, rate = probe_rate(path)
    if rate is None or not song_needs(path):
        return None, None
    new = os.path.splitext(path)[0] + '.ogg'
    if dry_run:
        return new, rate
    tmp = new + '.part'
    subprocess.run(['ffmpeg', '-hide_banner', '-loglevel', 'error', '-y', '-i', path, '-vn',
                    '-map_metadata', '-1', '-ac', '2', '-ar', str(min(rate, GBA_RATE)),
                    '-c:a', 'libvorbis', '-q:a', OGG_QUALITY, '-f', 'ogg', tmp], check=True)
    os.replace(tmp, new)
    if os.path.abspath(new) != os.path.abspath(path):
        os.remove(path)
    return new, rate


# ------------------------------------------------------------------ .def references

DEF_LINE = re.compile(r'^(\s*)([A-Za-z.]*bgm(?:usic)?)(\s*=\s*)(.*?)(\s*(?:;.*)?)$', re.I)
LOOP_LINE = re.compile(r'^(\s*)([A-Za-z.]*bgm\.?loop(?:start|end))(\s*=\s*)(-?\d+)(.*)$', re.I)


def resolve(root, def_path, ref):
    """The files a .def's song reference can mean: MUGEN tries it from the def's folder, then
    from the mugen folder, then under sound/."""
    ref = ref.strip().strip('"').replace('\\', '/')
    if not ref:
        return []
    return [os.path.normpath(os.path.join(os.path.dirname(def_path), ref)),
            os.path.normpath(os.path.join(root, ref)),
            os.path.normpath(os.path.join(root, 'sound', ref))]


def retarget_defs(root, renames, dry_run=False):
    """Points every .def line naming a renamed song at the new name, and scales the loop
    points of that song in the same section. renames: {old abs path: (new abs path, old rate)}.
    Returns the .def files changed."""
    changed = []
    for dirpath, _, files in os.walk(root):
        for name in files:
            if not name.lower().endswith('.def'):
                continue
            path = os.path.join(dirpath, name)
            raw = open(path, 'rb').read()
            text = raw.decode('latin-1')
            lines = text.split('\n')
            dirty = False
            ratio = None    # loop scaling for the rest of this section
            for i, line in enumerate(lines):
                if line.lstrip().startswith('['):
                    ratio = None
                m = DEF_LINE.match(line.rstrip('\r'))
                if m and m.group(4) and not re.search(r'loop|volume', m.group(2), re.I):
                    for cand in resolve(root, path, m.group(4)):
                        hit = renames.get(os.path.abspath(cand))
                        if hit is None:
                            continue
                        new_ref = os.path.splitext(m.group(4).strip())[0] + '.ogg'
                        cr = '\r' if line.endswith('\r') else ''
                        lines[i] = m.group(1) + m.group(2) + m.group(3) + new_ref + m.group(5) + cr
                        ratio = min(hit[1], GBA_RATE) / hit[1]
                        dirty = True
                        break
                    continue
                lm = LOOP_LINE.match(line)
                if lm and ratio is not None and ratio != 1:
                    v = int(lm.group(4))
                    if v > 0:
                        lines[i] = lm.group(1) + lm.group(2) + lm.group(3) + str(int(round(v * ratio))) + lm.group(5)
                        dirty = True
            if dirty:
                changed.append(path)
                if not dry_run:
                    write_atomic(path, '\n'.join(lines).encode('latin-1'))
    return changed


# ------------------------------------------------------------------ the whole folder

def write_atomic(path, data):
    tmp = path + '.part'
    with open(tmp, 'wb') as f:
        f.write(data)
    os.replace(tmp, path)


def find_files(root):
    snds, songs = [], []
    for dirpath, _, files in os.walk(root):
        for name in sorted(files):
            low = name.lower()
            path = os.path.join(dirpath, name)
            if low.endswith('.snd'):
                snds.append(path)
            elif low.endswith(SONG_EXTS):
                songs.append(path)
    return sorted(snds), sorted(songs)


def run(root, dry_run=False, quiet=False, log=print):
    """Converts everything under root that isn't GBA sound yet; returns a list of changes."""
    snds, songs = find_files(root)
    changes = []
    for path in snds:
        try:
            n, before, after = convert_snd(path, dry_run)
        except (ValueError, subprocess.CalledProcessError) as e:
            log('skipped %s: %s' % (os.path.relpath(path, root), e))
            continue
        if n:
            changes.append(('snd', path, n, before, after))
            if not quiet:
                log('%s: %d clip(s) to 8-bit mono %d Hz, %d KB -> %d KB' % (
                    os.path.relpath(path, root), n, GBA_RATE, before // 1024, after // 1024))
    renames = {}
    for path in songs:
        try:
            new, rate = convert_song(path, dry_run)
        except subprocess.CalledProcessError as e:
            log('skipped %s: %s' % (os.path.relpath(path, root), e))
            continue
        if new:
            renames[os.path.abspath(path)] = (os.path.abspath(new), rate)
            changes.append(('song', path, new, rate))
            if not quiet:
                log('%s (%d Hz) -> %s' % (os.path.relpath(path, root), rate, os.path.relpath(new, root)))
    if renames:
        for d in retarget_defs(root, renames, dry_run):
            changes.append(('def', d))
            if not quiet:
                log('%s: points at the new song' % os.path.relpath(d, root))
    return changes


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('root', nargs='?', default='mugen')
    ap.add_argument('--dry-run', action='store_true')
    ap.add_argument('--check', action='store_true', help='exit 1 when anything is not GBA sound yet')
    ap.add_argument('--quiet', action='store_true')
    ap.add_argument('--snd', nargs='+', metavar='FILE')
    ap.add_argument('--song', nargs='+', metavar='FILE')
    a = ap.parse_args()
    if a.snd or a.song:
        for p in a.snd or []:
            print(p, convert_snd(p, a.dry_run or a.check)[0], 'clip(s)')
        renames = {}
        for p in a.song or []:
            new, rate = convert_song(p, a.dry_run or a.check)
            if new:
                renames[os.path.abspath(p)] = (os.path.abspath(new), rate)
                print(p, '->', new)
        if renames and os.path.isdir(a.root):
            retarget_defs(a.root, renames, a.dry_run or a.check)
        return 0
    changes = run(a.root, dry_run=a.dry_run or a.check, quiet=a.quiet)
    if a.check:
        return 1 if changes else 0
    if not changes and not a.quiet:
        print('everything under %s is GBA sound already' % a.root)
    return 0


if __name__ == '__main__':
    sys.exit(main())
