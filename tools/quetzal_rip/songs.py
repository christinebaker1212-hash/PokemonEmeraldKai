#!/usr/bin/env python3
"""Rip Moemon Quetzal's own songs (song ids 559-665) into portable pokeemerald / Kai sources.

    python3 tools/quetzal_rip/songs.py QUETZAL.gba [--out quetzal_rip/sound] [--verify]

For every song id in --first..--last (default 559-665) this writes
  songs/mus_qz_<id>.s      the song disassembled into the mid2agb assembly format (Kai's 64-bit
                           flavour: GOTO / PATT targets and the header use `ptrvalue`)
  midi/mus_qz_<id>.mid     the same song as a standard MIDI file for Kai's mid2agb build path
  midi/midi.cfg            the matching midi.cfg lines (-E -R -G -V127 -P)
  voicegroups/*.inc        voicegroups that are not byte-for-byte one of Kai's (Kai's format)
  direct_sound_samples/    DirectSound samples Kai does not already have (.wav that Kai's wav2agb
                           turns back into the ROM's exact bytes: 8-bit, smpl loop, agbp / agbl)
  programmable_wave_samples/  new 16-byte wave tables (.pcm)
  keysplit_tables.inc, direct_sound_data.inc, programmable_wave_data.inc, voice_groups.inc,
  song_table.inc           snippets to paste into Kai's sound/ files
  README.md                id -> name guess -> maps -> voicegroup -> length

Voicegroups, drumsets, keysplit tables and samples are compared by content against Kai's sound/
(samples by their converted bytes, not by name) and referenced by Kai's names when they match.

--verify (needs GNU as / ld / objcopy for i386 and Kai's mid2agb, built into a scratch dir):
  1. assembles every songs/*.s (VER_64BIT=0), links it at the song's ROM address and compares
     the bytes with the ROM;
  2. runs every .mid through mid2agb, assembles the result and compares what the two songs play
     (notes with the voice / volume / pan / ... in effect, controller changes, tempo, loop);
  3. converts every written .wav back with Kai's wav2agb and compares with the ROM sample.
The ROM is only read as data.
"""
import argparse
import collections
import hashlib
import math
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..'))

SONG_TABLE = 0x1397008      # gSongTable (file offset), 8-byte entries {SongHeader *, u16 ms, u16 me}
SONG_COUNT = 666
ROM_BASE = 0x08000000

CLOCK = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
         28, 30, 32, 36, 40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96]
NOTE_NAMES = ['Cn', 'Cs', 'Dn', 'Ds', 'En', 'Fn', 'Fs', 'Gn', 'Gs', 'An', 'As', 'Bn']
CMD_NAMES = {0xB1: 'FINE', 0xB2: 'GOTO', 0xB3: 'PATT', 0xB4: 'PEND', 0xB5: 'REPT', 0xB9: 'MEMACC',
             0xBA: 'PRIO', 0xBB: 'TEMPO', 0xBC: 'KEYSH', 0xBD: 'VOICE', 0xBE: 'VOL', 0xBF: 'PAN',
             0xC0: 'BEND', 0xC1: 'BENDR', 0xC2: 'LFOS', 0xC3: 'LFODL', 0xC4: 'MOD', 0xC5: 'MODT',
             0xC8: 'TUNE', 0xCD: 'XCMD', 0xCE: 'EOT', 0xCF: 'TIE'}
ONE_ARG = {0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC8}
FINE_LIKE = {0xB1, 0xB6, 0xB7, 0xB8, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB}   # all run ply_fine
CENTERED = {0xBF, 0xC0, 0xC8}                                          # PAN, BEND, TUNE: c_v+n
XCMD_ARGS = {0x01: 4, 0x02: 1, 0x04: 1, 0x05: 1, 0x06: 1, 0x07: 1, 0x08: 1, 0x09: 1, 0x0A: 1,
             0x0B: 1, 0x0C: 2, 0x0D: 4}                                # 0x00 / 0x03 = ply_xxx (fine)
XCMD_NAMES = {0x08: 'xIECV', 0x09: 'xIECL'}
MEMACC_OPS = ['mem_set', 'mem_add', 'mem_sub', 'mem_mem_set', 'mem_mem_add', 'mem_mem_sub',
              'mem_beq', 'mem_bne', 'mem_bhi', 'mem_bhs', 'mem_bls', 'mem_blo', 'mem_mem_beq',
              'mem_mem_bne', 'mem_mem_bhi', 'mem_mem_bhs', 'mem_mem_bls', 'mem_mem_blo']
# mid2agb's MIDI controller numbers for each command
CC_OF = {0xC4: 1, 0xBE: 7, 0xBF: 10, 0xC1: 20, 0xC2: 21, 0xC5: 22, 0xC8: 24, 0xC3: 26, 0xBA: 33}

GM = ('acoustic_grand_piano bright_piano electric_grand_piano honky_tonk_piano electric_piano_1 '
      'electric_piano_2 harpsichord clavinet celesta glockenspiel music_box vibraphone marimba xylophone '
      'tubular_bells dulcimer drawbar_organ percussive_organ rock_organ church_organ reed_organ accordion '
      'harmonica tango_accordion nylon_guitar steel_guitar jazz_guitar clean_guitar muted_guitar '
      'overdriven_guitar distortion_guitar guitar_harmonics acoustic_bass fingered_bass picked_bass '
      'fretless_bass slap_bass_1 slap_bass_2 synth_bass_1 synth_bass_2 violin viola cello contrabass '
      'tremolo_strings pizzicato_strings orchestral_harp timpani string_ensemble_1 string_ensemble_2 '
      'synth_strings_1 synth_strings_2 choir_aahs voice_oohs synth_voice orchestra_hit trumpet trombone '
      'tuba muted_trumpet french_horn brass_section synth_brass_1 synth_brass_2 soprano_sax alto_sax '
      'tenor_sax baritone_sax oboe english_horn bassoon clarinet piccolo flute recorder pan_flute '
      'blown_bottle shakuhachi whistle ocarina square_lead saw_lead calliope_lead chiff_lead charang_lead '
      'voice_lead fifths_lead bass_lead new_age_pad warm_pad polysynth_pad choir_pad bowed_pad '
      'metallic_pad halo_pad sweep_pad rain_fx soundtrack_fx crystal_fx atmosphere_fx brightness_fx '
      'goblins_fx echoes_fx scifi_fx sitar banjo shamisen koto kalimba bagpipe fiddle shanai tinkle_bell '
      'agogo steel_drums woodblock taiko_drum melodic_tom synth_drum reverse_cymbal guitar_fret_noise '
      'breath_noise seashore bird_tweet telephone_ring helicopter applause gunshot').split()
GM_DRUMS = {35: 'kick_2', 36: 'kick', 37: 'side_stick', 38: 'snare', 39: 'hand_clap', 40: 'snare_2',
            41: 'low_floor_tom', 42: 'closed_hihat', 43: 'high_floor_tom', 44: 'pedal_hihat',
            45: 'low_tom', 46: 'open_hihat', 47: 'low_mid_tom', 48: 'high_mid_tom', 49: 'crash_cymbal',
            50: 'high_tom', 51: 'ride_cymbal', 52: 'chinese_cymbal', 53: 'ride_bell', 54: 'tambourine',
            55: 'splash_cymbal', 56: 'cowbell', 57: 'crash_cymbal_2', 58: 'vibraslap', 59: 'ride_cymbal_2',
            60: 'high_bongo', 61: 'low_bongo', 62: 'mute_high_conga', 63: 'open_high_conga',
            64: 'low_conga', 65: 'high_timbale', 66: 'low_timbale', 67: 'high_agogo', 68: 'low_agogo',
            69: 'cabasa', 70: 'maracas', 71: 'short_whistle', 72: 'long_whistle', 73: 'short_guiro',
            74: 'long_guiro', 75: 'claves', 76: 'high_woodblock', 77: 'low_woodblock', 78: 'mute_cuica',
            79: 'open_cuica', 80: 'mute_triangle', 81: 'open_triangle'}


def key_name(k):
    if k >= 24:
        return '%s%d' % (NOTE_NAMES[k % 12], k // 12 - 2)
    return '%sM%d' % (NOTE_NAMES[k % 12], -(k // 12 - 2))


def cmd_name(c):
    if 0x80 <= c <= 0xB0:
        return 'W%02d' % CLOCK[c - 0x80]
    if c == 0xCF:
        return 'TIE'
    if c > 0xCF:
        return 'N%02d' % CLOCK[c - 0xCF]
    return CMD_NAMES.get(c, '0x%02X' % c)


class Rom:
    def __init__(self, path):
        self.d = open(path, 'rb').read()

    def u8(self, o): return self.d[o]
    def u16(self, o): return struct.unpack_from('<H', self.d, o)[0]
    def u32(self, o): return struct.unpack_from('<I', self.d, o)[0]

    def ptr(self, o):
        v = self.u32(o)
        return v - ROM_BASE if ROM_BASE <= v < ROM_BASE + len(self.d) else None


# ---------------------------------------------------------------------------------------------
# Track bytecode
# ---------------------------------------------------------------------------------------------
class Ins:
    """One decoded command. cmd is the effective command (the running status for a bare byte)."""
    __slots__ = ('addr', 'size', 'cmd', 'explicit', 'args', 'target', 'rs')

    def __init__(self, addr, size, cmd, explicit, args, target, rs):
        self.addr, self.size, self.cmd, self.explicit = addr, size, cmd, explicit
        self.args, self.target, self.rs = args, target, rs

    def ends_flow(self):
        if self.cmd in FINE_LIKE:
            return True
        if self.cmd == 0xCD and self.args and self.args[0] in (0x00, 0x03):
            return True
        return False


def decode(rom, a, rs):
    """Decode the command at file offset a with running status rs -> (Ins, new running status)."""
    d = rom.d
    start = a
    b = d[a]
    explicit = b >= 0x80
    if explicit:
        cmd = b
        a += 1
        if cmd >= 0xBD:
            rs = cmd
    else:
        if rs is None:
            raise ValueError('bare byte 0x%02X at 0x%X with no running status' % (b, a))
        cmd = rs
    args, target = [], None
    if 0x80 <= cmd <= 0xB0 or cmd in FINE_LIKE or cmd == 0xB4:
        pass
    elif cmd in (0xB2, 0xB3):
        target = rom.ptr(a)
        if target is None:
            raise ValueError('bad pointer at 0x%X' % a)
        a += 4
    elif cmd == 0xB5:
        args.append(d[a]); a += 1
        target = rom.ptr(a)
        if target is None:
            raise ValueError('bad REPT pointer at 0x%X' % a)
        a += 4
    elif cmd == 0xB9:
        args += [d[a], d[a + 1], d[a + 2]]; a += 3
        if args[0] >= 6:
            target = rom.ptr(a)
            a += 4
    elif cmd in ONE_ARG:
        args.append(d[a]); a += 1
    elif cmd == 0xCC:
        args += [d[a], d[a + 1]]; a += 2
    elif cmd == 0xCD:
        sub = d[a]; a += 1
        args.append(sub)
        n = XCMD_ARGS.get(sub, 0)
        args += list(d[a:a + n]); a += n
    elif cmd == 0xCE:
        if d[a] < 0x80:
            args.append(d[a]); a += 1
    elif cmd >= 0xCF:
        for _ in range(3):
            if d[a] < 0x80:
                args.append(d[a]); a += 1
            else:
                break
    else:
        raise ValueError('unknown command 0x%02X at 0x%X' % (cmd, start))
    return Ins(start, a - start, cmd, explicit, args, target, rs), rs


class Song:
    def __init__(self, rom, sid):
        self.rom, self.id = rom, sid
        self.name = 'mus_qz_%d' % sid
        hp, self.ms, self.me = struct.unpack_from('<IHH', rom.d, SONG_TABLE + 8 * sid)
        self.header = hp - ROM_BASE
        h = self.header
        self.ntracks, self.nblocks, self.priority, self.reverb = rom.d[h:h + 4]
        self.vg = rom.ptr(h + 4)
        self.tracks = [rom.ptr(h + 8 + 4 * i) for i in range(self.ntracks)]
        self.warnings = []
        self.disassemble()

    # -- static disassembly -----------------------------------------------------------------
    def disassemble(self):
        rom = self.rom
        self.ins = {}          # addr -> Ins
        self.owner = {}        # addr -> track number (1-based) of the region that decoded it
        starts = set(self.tracks)
        work = [(t, None, i + 1) for i, t in enumerate(self.tracks)]
        refs = []              # (target, running status at the jump, kind, track)
        while work:
            a, rs, trk = work.pop(0)
            while a not in self.ins:
                ins, rs = decode(rom, a, rs)
                self.ins[a] = ins
                self.owner[a] = trk
                if ins.target is not None:
                    refs.append((ins.target, ins.rs, ins.cmd, trk))
                    if ins.target not in self.ins:
                        work.append((ins.target, ins.rs, trk))
                a += ins.size
                if ins.ends_flow():
                    break
                if a in starts and a not in self.ins:
                    break      # ran into the next track without FINE
        # a jump target that decodes differently from the linear decode would be wrong
        for tgt, rs, kind, trk in refs:
            ins = self.ins.get(tgt)
            if ins is None:
                self.warnings.append('target 0x%X is not on a command boundary' % (tgt + ROM_BASE))
            elif not ins.explicit and ins.rs != rs:
                self.warnings.append('target 0x%X starts with a bare byte under another running status'
                                     % (tgt + ROM_BASE))
        # labels
        self.labels = {}
        for i, t in enumerate(self.tracks):
            self.labels.setdefault(t, '%s_%d' % (self.name, i + 1))
        nb = collections.Counter()
        npat = collections.Counter()
        for tgt, rs, kind, trk in sorted(refs):
            if tgt in self.labels:
                continue
            o = self.owner.get(tgt, trk)
            if kind == 0xB3:
                self.labels[tgt] = '%s_%d_%03d' % (self.name, o, npat[o]); npat[o] += 1
            else:
                nb[o] += 1
                self.labels[tgt] = '%s_%d_B%d' % (self.name, o, nb[o])

    # -- assembly output -----------------------------------------------------------------------
    def arg_text(self, ins):
        c, a = ins.cmd, ins.args
        if c >= 0xCF or c == 0xCE:
            out = []
            for i, v in enumerate(a):
                if i == 0:
                    out.append(key_name(v))
                elif i == 1:
                    out.append('v%03d' % v)
                else:
                    out.append('gtp%d' % v if 1 <= v <= 3 else str(v))
            return out
        if c in CENTERED:
            return ['c_v%+d' % (a[0] - 64)]
        if c == 0xBB:
            return ['%d*%s_tbs/2' % (a[0] * 2, self.name)]
        if c == 0xBC:
            return ['%s_key%+d' % (self.name, a[0] - 256 if a[0] >= 128 else a[0])] if a[0] < 128 \
                else [str(a[0])]
        if c == 0xBE:
            return ['%d*%s_mvl/mxv' % (a[0], self.name)]
        if c == 0xC5:
            return [{0: 'mod_vib', 1: 'mod_tre', 2: 'mod_pan'}.get(a[0], str(a[0]))]
        if c == 0xCD:
            out = [XCMD_NAMES.get(a[0], '0x%02X' % a[0])]
            return out + ['%d' % v for v in a[1:]]
        if c == 0xB9:
            return [MEMACC_OPS[a[0]] if a[0] < len(MEMACC_OPS) else str(a[0]), '0x%02X' % a[1], str(a[2])]
        return [str(v) for v in a]

    def asm(self, grp_symbol):
        L = []
        n = self.name
        L.append('\t.include "asm/macros/bit_width.inc"\n')
        L.append('\t.include "MPlayDef.s"\n')
        L.append('/* Moemon Quetzal song %d, disassembled from the ROM by tools/quetzal_rip/songs.py.' % self.id)
        L.append('   "org" comments give the ROM address of the bytes that follow. */\n')
        L.append('\t.equ\t%s_grp, %s' % (n, grp_symbol))
        L.append('\t.equ\t%s_pri, %d' % (n, self.priority))
        if self.reverb & 0x80:
            L.append('\t.equ\t%s_rev, reverb_set+%d' % (n, self.reverb & 0x7F))
        else:
            L.append('\t.equ\t%s_rev, %d' % (n, self.reverb))
        L.append('\t.equ\t%s_mvl, 127' % n)
        L.append('\t.equ\t%s_key, 0' % n)
        L.append('\t.equ\t%s_tbs, 1' % n)
        L.append('\t.equ\t%s_exg, 1' % n)
        L.append('\t.equ\t%s_cmp, 1' % n)
        L.append('')
        L.append('\t.section .rodata')
        L.append('\t.global\t%s' % n)
        L.append('\t.p2align\t2')
        addrs = sorted(self.ins)
        pos = None
        cur_track = None
        for a in addrs:
            ins = self.ins[a]
            if pos is not None and a != pos:
                if a > pos and a - pos <= 64:
                    L.append('\t.byte\t' + ', '.join('0x%02X' % b for b in self.rom.d[pos:a])
                             + '\t/* bytes no command reaches */')
                else:
                    L.append('/* org 0x%08X */' % (a + ROM_BASE))
            elif pos is None:
                L.append('/* org 0x%08X */' % (a + ROM_BASE))
            if a in self.tracks:
                t = self.tracks.index(a) + 1
                if t != cur_track:
                    L.append('\n/********************** Track %2d **********************/\n' % t)
                    cur_track = t
            if a in self.labels:
                L.append('%s:' % self.labels[a])
            L.extend(self.ins_lines(ins))
            pos = a + ins.size
        # header
        h = self.header
        if pos != h:
            gap = h - pos
            if 0 <= gap < 4 and all(b == 0 for b in self.rom.d[pos:h]):
                pass        # .p2align 2 pads it
            elif 0 <= gap <= 64:
                L.append('\t.byte\t' + ', '.join('0x%02X' % b for b in self.rom.d[pos:h]) + '\t/* padding */')
            else:
                L.append('/* org 0x%08X */' % (h + ROM_BASE))
        L.append('\n/******************************************************/')
        L.append('\t.p2align\t2\n')
        L.append('%s:' % n)
        L.append('\t.byte\t%d\t/* NumTrks */' % self.ntracks)
        L.append('\t.byte\t%d\t/* NumBlks */' % self.nblocks)
        L.append('\t.byte\t%s_pri\t/* Priority */' % n)
        L.append('\t.byte\t%s_rev\t/* Reverb. */' % n)
        L.append('space64 4')
        L.append('\tptrvalue\t%s_grp\n' % n)
        for t in self.tracks:
            L.append('\tptrvalue\t%s' % self.labels[t])
        L.append('\n\t.end')
        return '\n'.join(L) + '\n'

    def ins_lines(self, ins):
        c = ins.cmd
        parts = []
        if ins.explicit:
            parts.append(cmd_name(c))
        if c in (0xB2, 0xB3):
            return ['\t.byte\t%s' % cmd_name(c), '\t ptrvalue\t%s' % self.labels[ins.target]]
        if c == 0xB5:
            return ['\t.byte\tREPT  , %d' % ins.args[0], '\t ptrvalue\t%s' % self.labels[ins.target]]
        if c == 0xB9:
            out = ['\t.byte\tMEMACC, ' + ', '.join(self.arg_text(ins))]
            if ins.target is not None:
                out.append('\t ptrvalue\t%s' % self.labels[ins.target])
            return out
        parts += self.arg_text(ins)
        if ins.explicit:
            if len(parts) == 1:
                return ['\t.byte\t%s' % parts[0]]
            return ['\t.byte\t\t%-6s, %s' % (parts[0], ' , '.join(parts[1:]))]
        return ['\t.byte\t\t        %s' % ' , '.join(parts)]

    # -- playback simulation ------------------------------------------------------------------
    def simulate(self):
        """Play every track once (to its loop GOTO / FINE) -> per-track event lists and loop points."""
        self.sim = [simulate_track(self.rom.d, self.rom_ptr_at, t, self.warnings) for t in self.tracks]
        return self.sim

    def rom_ptr_at(self, a):
        return self.rom.ptr(a)


def simulate_track(data, ptr_at, start, warnings, decode_fn=None):
    """Run one track like the m4a engine. Returns dict(notes, ctl, tempo, loop, end, voices, keys)."""
    class R:
        pass
    r = R()
    r.d = data
    r.ptr = ptr_at
    pc = start
    rs = None
    t = 0
    stack = []
    rept = 0
    seen = {}                     # (pc, stack) -> time first executed
    notes = []                    # [start, key, vel, dur, state-tuple]
    ties = []                     # open TIE notes (index into notes)
    ctl = []                      # (time, cmd, value)
    tempo = []
    state = {0xBD: 0, 0xBE: 127, 0xBF: 64, 0xC0: 64, 0xC1: 2, 0xC2: 22, 0xC3: 0, 0xC4: 0, 0xC5: 0,
             0xC8: 64, 0xBA: 0, 0xBC: 0, 'iecv': 0, 'iecl': 0}
    key, vel = 60, 127
    loop = None
    steps = 0
    rom = _DataRom(data, ptr_at)
    while True:
        steps += 1
        if steps > 400000:
            warnings.append('track at 0x%X: no end after 400000 commands' % start)
            break
        k = (pc, tuple(stack))
        if k not in seen:
            seen[k] = t
        ins, rs = decode(rom, pc, rs)
        c = ins.cmd
        nxt = pc + ins.size
        if 0x80 <= c <= 0xB0:
            t += CLOCK[c - 0x80]
        elif ins.ends_flow():
            break
        elif c == 0xB2 or (c == 0xB5 and ins.args[0] == 0):
            kk = (ins.target, tuple(stack))
            if kk in seen:
                loop = (seen[kk], t)
                break
            nxt = ins.target
        elif c == 0xB3:
            if len(stack) < 3:
                stack.append(nxt)
                nxt = ins.target
            else:
                warnings.append('patterns nested deeper than 3')
                break
        elif c == 0xB4:
            if stack:
                nxt = stack.pop()
        elif c == 0xB5:
            rept += 1
            if rept < ins.args[0]:
                nxt = ins.target
            else:
                rept = 0
        elif c == 0xB9:
            if ins.target is not None:
                warnings.append('MEMACC conditional jump (taken as not taken)')
            else:
                warnings.append('MEMACC used')
        elif c == 0xBB:
            tempo.append((t, ins.args[0]))
        elif c in ONE_ARG:
            v = ins.args[0]
            state[c] = v
            ctl.append((t, c, v))
        elif c == 0xCC:
            ctl.append((t, 0xCC, tuple(ins.args)))
        elif c == 0xCD:
            sub = ins.args[0]
            if sub == 0x08:
                state['iecv'] = ins.args[1]; ctl.append((t, 'iecv', ins.args[1]))
            elif sub == 0x09:
                state['iecl'] = ins.args[1]; ctl.append((t, 'iecl', ins.args[1]))
            else:
                ctl.append((t, 'xcmd%02X' % sub, tuple(ins.args[1:])))
        elif c == 0xCE:
            if ins.args:
                key = ins.args[0]
            for i in range(len(ties)):
                n = notes[ties[i]]
                if n[1] == key:
                    n[3] = t - n[0]
                    del ties[i]
                    break
        elif c >= 0xCF:
            gate = CLOCK[c - 0xCF]
            if len(ins.args) >= 1:
                key = ins.args[0]
            if len(ins.args) >= 2:
                vel = ins.args[1]
            if len(ins.args) >= 3:
                gate += ins.args[2]
            st = (state[0xBD], state[0xBE], state[0xBF], state[0xC0], state[0xC1], state[0xC2],
                  state[0xC3], state[0xC4], state[0xC5], state[0xC8], state[0xBA], state[0xBC],
                  state['iecv'], state['iecl'])
            notes.append([t, key, vel, gate if c > 0xCF else None, st])
            if c == 0xCF:
                ties.append(len(notes) - 1)
        pc = nxt
    for i in ties:
        notes[i][3] = max(1, t - notes[i][0])
    return {'notes': notes, 'ctl': ctl, 'tempo': tempo, 'loop': loop, 'end': t}


class _DataRom:
    """decode() needs .d and .ptr(); lets the simulator run on any byte blob."""
    def __init__(self, d, ptr):
        self.d, self.ptr = d, ptr


# ---------------------------------------------------------------------------------------------
# MIDI
# ---------------------------------------------------------------------------------------------
def vlq(n):
    out = [n & 0x7F]
    n >>= 7
    while n:
        out.append(0x80 | (n & 0x7F))
        n >>= 7
    return bytes(reversed(out))


def midi_track(events):
    """events: list of (time, order, bytes). Returns an MTrk chunk."""
    events.sort(key=lambda e: (e[0], e[1]))
    out = bytearray()
    last = 0
    end = 0
    for t, _, b in events:
        out += vlq(t - last) + b
        last = t
        end = max(end, t)
    out += vlq(0) + b'\xff\x2f\x00'
    return b'MTrk' + struct.pack('>I', len(out)) + bytes(out)


def tempo_us(T):
    """MIDI tempo (us per quarter) that mid2agb turns back into TEMPO T (round(6e7/us)*1/2)."""
    bpm = 2 * T
    us = int(round(60000000.0 / bpm))
    for cand in (us, us - 1, us + 1, us - 2, us + 2):
        if cand > 0 and int(round(60000000.0 / cand)) // 2 == T:
            return cand
    return us


def song_to_midi(song, sim, loop):
    trk0 = []
    trk0.append((0, 0, b'\xff\x03' + vlq(len(song.name)) + song.name.encode()))
    trk0.append((0, 1, b'\xff\x58\x04\x04\x02\x18\x08'))
    for s in sim:
        for t, T in s['tempo']:
            trk0.append((t, 2, b'\xff\x51\x03' + struct.pack('>I', tempo_us(T))[1:]))
    if loop:
        trk0.append((loop[0], 3, b'\xff\x06\x01['))
        trk0.append((loop[1], 3, b'\xff\x06\x01]'))
    end = max([s['end'] for s in sim] + [0])
    if loop:
        end = max(end, loop[1])
    trk0.append((end, 9, b'\xff\x01\x03end'))     # keeps the song's length (mid2agb skips this text)
    chunks = [midi_track(trk0)]
    ch = 0
    skipped = []
    for i, s in enumerate(sim):
        if not s['notes']:
            skipped.append(i + 1)
            continue
        ev = []
        nm = ('%s track %d' % (song.name, i + 1)).encode()
        ev.append((0, 0, b'\xff\x03' + vlq(len(nm)) + nm))
        for t, c, v in s['ctl']:
            if c == 0xBD:
                ev.append((t, 2, bytes([0xC0 | ch, v])))
            elif c == 0xC0:
                ev.append((t, 2, bytes([0xE0 | ch, 0, v])))
            elif c in CC_OF:
                ev.append((t, 2, bytes([0xB0 | ch, CC_OF[c], v])))
            elif c in ('iecv', 'iecl'):
                ev.append((t, 2, bytes([0xB0 | ch, 30, 8 if c == 'iecv' else 9])))
                ev.append((t, 2, bytes([0xB0 | ch, 29, v])))
            # KEYSH, PORT and other XCMDs have no MIDI form in mid2agb (reported by --verify)
        for st, k, vel, dur, _ in s['notes']:
            ev.append((st, 3, bytes([0x90 | ch, k, max(1, vel)])))
            ev.append((st + max(1, dur), 1, bytes([0x80 | ch, k, 0])))
        chunks.append(midi_track(ev))
        ch = (ch + 1) % 16
    hdr = b'MThd' + struct.pack('>IHHH', 6, 1, len(chunks), 24)
    return hdr + b''.join(chunks), skipped


# ---------------------------------------------------------------------------------------------
# Kai's sound data (for matching by content)
# ---------------------------------------------------------------------------------------------
def wav_to_agb_bin(path):
    """Kai's wav2agb -b for the 8-bit .wav files Kai ships (header + samples, no padding)."""
    b = open(path, 'rb').read()
    i = 12
    smpl = agbp = agbl = None
    rate = 0
    data = None
    while i + 8 <= len(b):
        cid = b[i:i + 4]
        n = struct.unpack_from('<I', b, i + 4)[0]
        c = b[i + 8:i + 8 + n]
        if cid == b'fmt ':
            fmt, nch, rate, _, align, bits = struct.unpack_from('<HHIIHH', c)
            if (fmt, nch, bits) != (1, 1, 8):
                return None
        elif cid == b'data':
            data = c
        elif cid == b'smpl':
            smpl = c
        elif cid == b'agbp' and n >= 4:
            agbp = struct.unpack_from('<I', c)[0]
        elif cid == b'agbl' and n >= 4:
            agbl = struct.unpack_from('<I', c)[0]
        i += 8 + n + (n & 1)
    if data is None:
        return None
    nsamp = len(data)
    loop_on, loop_start, loop_end = False, 0, nsamp
    midikey, tuning = 60, 0.0
    if smpl:
        midikey = min(struct.unpack_from('<I', smpl, 12)[0], 127)
        tuning = struct.unpack_from('<I', smpl, 16)[0] / (4294967296.0 * 100.0)
        if struct.unpack_from('<I', smpl, 28)[0] == 1:
            loop_start = struct.unpack_from('<I', smpl, 44)[0]
            loop_end = struct.unpack_from('<I', smpl, 48)[0] + 1
            loop_on = True
    loop_end = min(loop_end, nsamp)
    if midikey == 60 and tuning == 0.0:
        pitch = float(rate)
    else:
        pitch = rate * math.pow(2.0, (60.0 - midikey) / 12.0 + tuning / 1200.0)
    pv = agbp if agbp else int(pitch * 1024.0)
    le = agbl if agbl else loop_end
    hdr = struct.pack('<IIII', 0x40000000 if loop_on else 0, pv, loop_start, le)
    body = bytes((x - 128) & 0xFF for x in data[:loop_end])
    return hdr + body


def sample_key(blob):
    """Identity of a DirectSound sample: its header and the `size` bytes after it."""
    if len(blob) < 16:
        return None
    flags, pitch, ls, size = struct.unpack_from('<IIII', blob)
    if flags & 1:
        return None
    if len(blob) < 16 + size:
        return None
    return hashlib.sha1(blob[:16 + size]).hexdigest()


class Kai:
    """Kai's sound/: samples, wave tables, keysplit tables and the voicegroup memory, by content."""
    MACRO_RE = re.compile(r'^\s*(\w+)\s*(.*?)\s*(?:@.*)?$')

    def __init__(self, root):
        self.root = root
        snd = os.path.join(root, 'sound')
        self.sample_by_key = {}         # sha1 -> DirectSoundWaveData_ label
        self.sample_label_key = {}
        for label, path in re.findall(r'(DirectSoundWaveData_\w+)::\s*\n\s*\.incbin\s+"([^"]+)"',
                                      open(os.path.join(snd, 'direct_sound_data.inc')).read()):
            wav = os.path.join(root, path[:-4] + '.wav')
            if not os.path.exists(wav):
                continue
            blob = wav_to_agb_bin(wav)
            k = sample_key(blob) if blob else None
            if k:
                self.sample_by_key.setdefault(k, label)
                self.sample_label_key[label] = k
        self.pwave_by_bytes = {}
        self.pwave_label_bytes = {}
        for label, path in re.findall(r'(ProgrammableWaveData_\w+)::\s*\n\s*\.incbin\s+"([^"]+)"',
                                      open(os.path.join(snd, 'programmable_wave_data.inc')).read()):
            p = os.path.join(root, path)
            if os.path.exists(p):
                b = open(p, 'rb').read()[:16]
                self.pwave_by_bytes.setdefault(b, label)
                self.pwave_label_bytes[label] = b
        # keysplit tables: label -> {key: index}
        self.keysplits = {}
        cur = None
        for line in open(os.path.join(snd, 'keysplit_tables.inc')):
            m = re.match(r'\s*keysplit\s+(\w+)\s*(?:,\s*(\d+))?', line)
            if m:
                cur = 'keysplit_' + m.group(1)
                last = int(m.group(2) or 0)
                self.keysplits[cur] = {}
                continue
            m = re.match(r'\s*split\s+(\d+)\s*,\s*(\d+)', line)
            if m and cur:
                idx, end = int(m.group(1)), int(m.group(2))
                for k in range(last, end):
                    self.keysplits[cur][k] = idx
                last = end
        # voicegroups in voice_groups.inc order: one flat list, labels index into it
        self.entries = []
        self.labels = {}                # voicegroup_x -> index of its entry 0
        self.label_len = {}             # entries written under the label (for preferring it)
        self.label_start = {}
        order = re.findall(r'\.include\s+"(sound/voicegroups/[^"]+)"',
                           open(os.path.join(snd, 'voice_groups.inc')).read())
        for inc in order:
            cur = None
            for line in open(os.path.join(root, inc)):
                line = line.split('@')[0].strip()
                if not line:
                    continue
                m = re.match(r'voice_group\s+(\w+)\s*(?:,\s*(\d+))?', line)
                if m:
                    cur = 'voicegroup_' + m.group(1)
                    self.labels[cur] = len(self.entries) - int(m.group(2) or 0)
                    self.label_start[cur] = int(m.group(2) or 0)
                    self.label_len[cur] = 0
                    continue
                m = re.match(r'(\w+)\s*(.*)$', line)
                if not m:
                    continue
                macro = m.group(1)
                args = [a.strip() for a in m.group(2).split(',')] if m.group(2) else []
                self.entries.append((macro, tuple(args)))
                if cur:
                    self.label_len[cur] += 1

    def entry(self, label, i):
        j = self.labels[label] + i
        return self.entries[j] if 0 <= j < len(self.entries) else None


# ---------------------------------------------------------------------------------------------
# ROM voicegroups
# ---------------------------------------------------------------------------------------------
def to_int(s):
    return int(s, 0)


def kai_pan(b):
    return b & 0x7F if (b & 0x80 and b & 0x7F) else b


class Tone:
    """A 12-byte ROM ToneData decoded into one of Kai's voice macros."""

    def __init__(self, rom, addr):
        self.addr = addr
        b = rom.d[addr:addr + 12]
        self.raw = b
        typ, key, b2, b3 = b[0], b[1], b[2], b[3]
        w = struct.unpack_from('<I', b, 4)[0]
        p2 = struct.unpack_from('<I', b, 8)[0]
        a, d, s, r = b[8], b[9], b[10], b[11]
        self.ptr = None
        self.ptr2 = None
        self.kind = None
        if typ == 0x80:
            self.macro, self.kind = 'voice_keysplit_all', 'drum'
            self.ptr = w - ROM_BASE
            self.args = []
            enc = bytes([0x80, 0, 0, 0]) + struct.pack('<II', w, 0)
        elif typ == 0x40:
            self.macro, self.kind = 'voice_keysplit', 'split'
            self.ptr, self.ptr2 = w - ROM_BASE, p2 - ROM_BASE
            self.args = []
            enc = bytes([0x40, 0, 0, 0]) + struct.pack('<II', w, p2)
        elif typ in (0, 8, 16):
            self.macro = {0: 'voice_directsound', 8: 'voice_directsound_no_resample',
                          16: 'voice_directsound_alt'}[typ]
            self.kind = 'ds'
            self.ptr = w - ROM_BASE
            pan = b3 & 0x7F if (b3 & 0x80 and b3 & 0x7F) else b3     # Kai writes 64, not 192
            self.args = [key, pan, None, a, d, s, r]
            enc = bytes([typ, key, 0, (0x80 | pan) if pan else 0]) + struct.pack('<I', w) + bytes([a, d, s, r])
        elif typ in (1, 9):
            self.macro = 'voice_square_1' if typ == 1 else 'voice_square_1_alt'
            self.args = [key, kai_pan(b2), b3, w & 3, a, d, s, r]
            enc = bytes([typ, key, (0x80 | b2) if b2 else 0, b3]) + struct.pack('<I', w & 3) + \
                bytes([a & 7, d & 7, s & 15, r & 7])
        elif typ in (2, 10):
            self.macro = 'voice_square_2' if typ == 2 else 'voice_square_2_alt'
            self.args = [key, kai_pan(b2), w & 3, a, d, s, r]
            enc = bytes([typ, key, (0x80 | b2) if b2 else 0, 0]) + struct.pack('<I', w & 3) + \
                bytes([a & 7, d & 7, s & 15, r & 7])
        elif typ in (3, 11):
            self.macro = 'voice_programmable_wave' if typ == 3 else 'voice_programmable_wave_alt'
            self.kind = 'pw'
            self.ptr = w - ROM_BASE
            self.args = [key, kai_pan(b2), None, a, d, s, r]
            enc = bytes([typ, key, (0x80 | b2) if b2 else 0, 0]) + struct.pack('<I', w) + \
                bytes([a & 7, d & 7, s & 15, r & 7])
        elif typ in (4, 12):
            self.macro = 'voice_noise' if typ == 4 else 'voice_noise_alt'
            self.args = [key, kai_pan(b2), w & 1, a, d, s, r]
            enc = bytes([typ, key, (0x80 | b2) if b2 else 0, 0]) + struct.pack('<I', w & 1) + \
                bytes([a & 7, d & 7, s & 15, r & 7])
        else:
            self.macro = None
            self.args = []
            enc = b''
        self.exact = enc == b

    def canon(self, ref):
        """Comparable form; ref(kind, ptr) names what a pointer points at."""
        args = tuple(self.args)
        if self.kind in ('ds', 'pw'):
            args = args[:2] + (ref(self.kind, self.ptr),) + args[3:]
            return (self.macro,) + args
        if self.kind == 'drum':
            return (self.macro, ref('drum', self.ptr))
        if self.kind == 'split':
            return (self.macro, ref('split', (self.ptr, self.ptr2)))
        return (self.macro,) + args


def kai_canon(kai, entry, ref_label):
    """Canonical form of a Kai voice macro line (pointers resolved by ref_label(kind, name))."""
    macro, args = entry
    if macro.startswith('voice_directsound') or macro.startswith('voice_programmable_wave'):
        try:
            nums = [to_int(a) for a in args[:2]] + [args[2]] + [to_int(a) for a in args[3:]]
        except ValueError:
            return None
        kind = 'ds' if macro.startswith('voice_directsound') else 'pw'
        nums[2] = ref_label(kind, args[2])
        if macro.startswith('voice_programmable_wave'):
            nums[3:] = [nums[3] & 7, nums[4] & 7, nums[5] & 15, nums[6] & 7]
        return (macro,) + tuple(nums)
    if macro == 'voice_keysplit_all':
        return (macro, ref_label('drum', args[0]))
    if macro == 'voice_keysplit':
        return (macro, ref_label('split', (args[0], args[1])))
    if macro.startswith('voice_'):
        try:
            nums = [to_int(a) for a in args]
        except ValueError:
            return None
        if macro.startswith('voice_square_1'):
            nums[3] &= 3
            nums[4:] = [nums[4] & 7, nums[5] & 7, nums[6] & 15, nums[7] & 7]
        elif macro.startswith('voice_square_2'):
            nums[2] &= 3
            nums[3:] = [nums[3] & 7, nums[4] & 7, nums[5] & 15, nums[6] & 7]
        elif macro.startswith('voice_noise'):
            nums[2] &= 1
            nums[3:] = [nums[3] & 7, nums[4] & 7, nums[5] & 15, nums[6] & 7]
        return (macro,) + tuple(nums)
    return (macro,) + tuple(args)


def rom_canon_masked(tone, ref):
    c = tone.canon(ref)
    if tone.macro and tone.macro.startswith(('voice_square', 'voice_noise', 'voice_programmable')):
        c = list(c)
        if tone.macro.startswith('voice_square_1'):
            c[4] &= 3
            c[5:9] = [c[5] & 7, c[6] & 7, c[7] & 15, c[8] & 7]
        elif tone.macro.startswith('voice_square_2'):
            c[3] &= 3
            c[4:8] = [c[4] & 7, c[5] & 7, c[6] & 15, c[7] & 7]
        elif tone.macro.startswith('voice_noise'):
            c[3] &= 1
            c[4:8] = [c[4] & 7, c[5] & 7, c[6] & 15, c[7] & 7]
        else:
            c[4:8] = [c[4] & 7, c[5] & 7, c[6] & 15, c[7] & 7]
        c = tuple(c)
    return c


# ---------------------------------------------------------------------------------------------
# Which maps play which song
# ---------------------------------------------------------------------------------------------
def song_usage(rip_dir):
    use = collections.defaultdict(lambda: {'maps': set(), 'scripts': set()})
    mdir = os.path.join(rip_dir, 'maps')
    if os.path.isdir(mdir):
        for m in sorted(os.listdir(mdir)):
            mj = os.path.join(mdir, m, 'map.json')
            if os.path.exists(mj):
                mm = re.search(r'"music"\s*:\s*"MUS_QZ_(\d+)"', open(mj).read())
                if mm:
                    use[int(mm.group(1))]['maps'].add(m)
            sc = os.path.join(mdir, m, 'scripts.inc')
            if os.path.exists(sc):
                for mm in re.finditer(r'(playbgm|fadenewbgm|savebgm|fadeinbgm|playfanfare|playse)\s+MUS_QZ_(\d+)',
                                      open(sc).read()):
                    use[int(mm.group(2))]['scripts'].add('%s (%s)' % (m, mm.group(1)))
    sdir = os.path.join(rip_dir, 'scripts')
    if os.path.isdir(sdir):
        for f in sorted(os.listdir(sdir)):
            p = os.path.join(sdir, f)
            if not os.path.isfile(p):
                continue
            label = None
            for line in open(p, errors='replace'):
                lm = re.match(r'^(\w+)::?', line)
                if lm:
                    label = lm.group(1)
                for mm in re.finditer(r'(playbgm|fadenewbgm|savebgm|fadeinbgm|playfanfare|playse)\s+MUS_QZ_(\d+)', line):
                    use[int(mm.group(2))]['scripts'].add('%s:%s (%s)' % (f, label, mm.group(1)))
    return use


def area_name(m):
    """Johto_NewBarkTown_ElmsLab -> 'New Bark Town'."""
    m = re.sub(r'^(Johto|Kanto|Sevii)_', '', m)
    base = m.split('_')[0]
    base = re.sub(r'(\d+)F$|B\d+F$|\d+F\b', '', base)
    words = re.sub(r'([a-z])([A-Z0-9])', r'\1 \2', base)
    words = re.sub(r'([0-9])([A-Z])', r'\1 \2', words)
    return words.strip()


# ---------------------------------------------------------------------------------------------
# Main ripping
# ---------------------------------------------------------------------------------------------
class Ripper:
    def __init__(self, rom, kai, out, first, last):
        self.rom, self.kai, self.out = rom, kai, out
        self.songs = [Song(rom, i) for i in range(first, last + 1)]
        self.log = []
        # names for ROM data
        self.vg_name = {}          # rom addr -> voicegroup_x
        self.vg_new = {}           # rom addr -> (name, kind, {index: Tone})
        self.sample_name = {}      # rom addr -> DirectSoundWaveData_x
        self.sample_new = {}       # rom addr -> label
        self.pw_name = {}
        self.pw_new = {}
        self.split_name = {}       # rom table addr -> keysplit_x
        self.split_new = {}

    # -- usage of programs / keys ------------------------------------------------------------
    def collect_use(self):
        """Which programs (and which keys under each program) every voicegroup is played with."""
        self.use = collections.defaultdict(lambda: collections.defaultdict(set))
        for s in self.songs:
            s.simulate()
            for tr in s.sim:
                for st, k, vel, dur, state in tr['notes']:
                    self.use[s.vg][state[0]].add(k)

    def tone(self, vg, i):
        return Tone(self.rom, vg + 12 * i)

    def ref(self, kind, p):
        if kind == 'ds':
            return 'ds:' + self.sample_id(p)
        if kind == 'pw':
            return 'pw:' + self.rom.d[p:p + 16].hex()
        if kind == 'drum':
            return 'vg:%x' % p
        if kind == 'split':
            return 'split:%x:%x' % p

    def sample_id(self, p):
        d = self.rom.d
        size = struct.unpack_from('<I', d, p + 12)[0]
        return hashlib.sha1(d[p:p + 16 + size]).hexdigest()

    def kai_ref(self, kind, x):
        k = self.kai
        if kind == 'ds':
            return 'ds:' + k.sample_label_key.get(x, '?' + x)
        if kind == 'pw':
            b = k.pwave_label_bytes.get(x)
            return 'pw:' + (b.hex() if b else '?' + x)
        if kind == 'drum':
            return 'kvg:' + x
        if kind == 'split':
            return 'ksplit:%s:%s' % x

    # -- matching against Kai -----------------------------------------------------------------
    def match_group(self, vg, used, sub_match):
        """Find Kai labels whose entries equal the ROM group's at every used index."""
        k = self.kai
        cands = []
        for label in k.labels:
            ok = True
            for i in sorted(used):
                e = k.entry(label, i)
                if e is None:
                    ok = False
                    break
                t = self.tone(vg, i)
                if not t.exact:
                    ok = False
                    break
                rc = rom_canon_masked(t, self.ref)
                kc = kai_canon(k, e, self.kai_ref)
                if kc is None or not self.same(rc, kc, sub_match):
                    ok = False
                    break
            if ok:
                cands.append(label)
        if not cands:
            return None
        # prefer the label that writes all used entries itself, then the shortest name
        def own(l):
            lo = k.label_start.get(l, 0)
            return all(lo <= i < lo + k.label_len.get(l, 0) for i in used)
        cands.sort(key=lambda l: (not own(l), len(l), l))
        return cands[0]

    def same(self, rc, kc, sub_match):
        if len(rc) != len(kc) or rc[0] != kc[0]:
            return False
        for a, b in zip(rc[1:], kc[1:]):
            if isinstance(a, str) and a.startswith('ds:'):
                if b != a:
                    return False
            elif isinstance(a, str) and a.startswith('pw:'):
                if b != a:
                    return False
            elif isinstance(a, str) and a.startswith(('vg:', 'split:')):
                if not sub_match(a, b):
                    return False
            elif a != b:
                return False
        return True

    def resolve(self):
        rom, k = self.rom, self.kai
        # sub-groups first: drumsets (keysplit_all) and keysplit groups, with the keys they get
        self.sub_use = collections.defaultdict(set)      # sub vg addr -> indexes used
        self.split_use = collections.defaultdict(set)    # (vg, table) -> keys used
        for vg, progs in self.use.items():
            for p, keys in progs.items():
                t = self.tone(vg, p)
                if t.kind == 'drum':
                    self.sub_use[t.ptr] |= keys
                elif t.kind == 'split':
                    self.split_use[(t.ptr, t.ptr2)] |= keys
                    for key in keys:
                        self.sub_use[t.ptr].add(rom.d[t.ptr2 + key])
        self.sub_match = {}
        for sub, idx in sorted(self.sub_use.items()):
            m = self.match_group(sub, idx, lambda a, b: False)
            self.sub_match['vg:%x' % sub] = m
        self.split_match = {}
        for (sub, tbl), keys in sorted(self.split_use.items()):
            name = None
            for label, table in k.keysplits.items():
                if all(table.get(key) == rom.d[tbl + key] for key in keys):
                    name = label
                    break
            self.split_match[tbl] = name

        def sub_match(a, b):
            if a.startswith('vg:'):
                return self.sub_match.get(a) is not None and b == 'kvg:' + self.sub_match[a]
            _, s, t = a.split(':')
            m = self.sub_match.get('vg:' + s)
            tn = self.split_match.get(int(t, 16))
            return m is not None and tn is not None and b == 'ksplit:%s:%s' % (m, tn)
        self.top_match = {}
        for vg, progs in sorted(self.use.items()):
            self.top_match[vg] = self.match_group(vg, set(progs), sub_match)

    # -- naming new data ----------------------------------------------------------------------
    def plan_new(self, vg_names):
        rom, k = self.rom, self.kai
        self.new_samples = collections.OrderedDict()     # rom addr -> label
        self.new_pw = collections.OrderedDict()
        self.new_splits = collections.OrderedDict()      # table addr -> (label, start, keys)
        self.new_groups = collections.OrderedDict()      # vg addr -> (label, {index: Tone}, kind)
        self.reused_samples = set()

        def sample_label(p, hint):
            key = self.sample_id(p)
            if key in k.sample_by_key:
                self.reused_samples.add(k.sample_by_key[key])
                return k.sample_by_key[key]
            for q, lab in self.new_samples.items():
                if self.sample_id(q) == key:
                    return lab
            if p not in self.new_samples:
                base = 'DirectSoundWaveData_%s' % hint
                names = set(self.new_samples.values())
                lab = base
                n = 2
                while lab in names:
                    lab = '%s_%d' % (base, n)
                    n += 1
                self.new_samples[p] = lab
            return self.new_samples[p]

        def pw_label(p):
            b = rom.d[p:p + 16]
            if b in k.pwave_by_bytes:
                return k.pwave_by_bytes[b]
            if p not in self.new_pw:
                self.new_pw[p] = 'ProgrammableWaveData_qz_%d' % (len(self.new_pw) + 1)
            return self.new_pw[p]

        self.ptr_label = {}

        def group_label(sub, idx, kind, hint):
            m = self.sub_match.get('vg:%x' % sub)
            if m:
                return m
            if sub not in self.new_groups:
                lab = 'voicegroup_%s' % hint
                self.new_groups[sub] = (lab, {}, kind)
                for i in sorted(idx):
                    t = self.tone(sub, i)
                    dh = '%s_%s' % (hint, ('k%d_%s' % (i, GM_DRUMS.get(i, 'x'))) if kind == 'drum' else '%d' % i)
                    self.new_groups[sub][1][i] = t
                    self.name_ptrs(t, dh, sample_label, pw_label, group_label, split_label)
            return self.new_groups[sub][0]

        def split_label(tbl, keys, hint):
            m = self.split_match.get(tbl)
            if m:
                return m
            if tbl not in self.new_splits:
                self.new_splits[tbl] = ('keysplit_%s' % hint, keys)
            else:
                self.new_splits[tbl][1].update(keys)
            return self.new_splits[tbl][0]

        self.label_of_group = {}
        for vg, progs in sorted(self.use.items()):
            m = self.top_match.get(vg)
            if m:
                self.label_of_group[vg] = m
                continue
            name = vg_names[vg]
            lab = 'voicegroup_' + name
            entries = {}
            self.new_groups[vg] = (lab, entries, 'top')
            self.label_of_group[vg] = lab
            for p in sorted(progs):
                t = self.tone(vg, p)
                entries[p] = t
                if t.kind == 'drum':
                    hint = '%s_drumset_%d' % (name, p)
                else:
                    hint = '%s_%s' % (name, GM[p] if p < 128 else 'p%d' % p)
                if t.kind == 'drum':
                    self.sub_use[t.ptr] |= progs[p]
                elif t.kind == 'split':
                    self.split_use[(t.ptr, t.ptr2)] |= progs[p]
                self.name_ptrs(t, hint, sample_label, pw_label, group_label, split_label,
                               keys=progs[p])

    def name_ptrs(self, t, hint, sample_label, pw_label, group_label, split_label, keys=None):
        if t.kind == 'ds':
            self.ptr_label[('ds', t.ptr)] = sample_label(t.ptr, hint)
        elif t.kind == 'pw':
            self.ptr_label[('pw', t.ptr)] = pw_label(t.ptr)
        elif t.kind == 'drum':
            self.ptr_label[('vg', t.ptr)] = group_label(t.ptr, self.sub_use[t.ptr], 'drum', hint)
        elif t.kind == 'split':
            used_idx = self.sub_use[t.ptr]
            self.ptr_label[('vg', t.ptr)] = group_label(t.ptr, used_idx, 'split', hint)
            self.ptr_label[('split', t.ptr2)] = split_label(t.ptr2, set(self.split_use[(t.ptr, t.ptr2)]),
                                                           hint)

    # -- writing ------------------------------------------------------------------------------
    def tone_line(self, t, comment=''):
        if t.macro is None:
            return '\t.byte %s\t/* unknown voice type */' % ', '.join('0x%02X' % b for b in t.raw)
        if t.kind == 'drum':
            s = '\t%s %s' % (t.macro, self.ptr_label[('vg', t.ptr)])
        elif t.kind == 'split':
            s = '\t%s %s, %s' % (t.macro, self.ptr_label[('vg', t.ptr)], self.ptr_label[('split', t.ptr2)])
        else:
            args = list(t.args)
            if t.kind == 'ds':
                args[2] = self.ptr_label[('ds', t.ptr)]
            elif t.kind == 'pw':
                args[2] = self.ptr_label[('pw', t.ptr)]
            s = '\t%s %s' % (t.macro, ', '.join(str(a) for a in args))
        if not t.exact:
            s += '\t/* ROM bytes %s do not fit the macro exactly */' % t.raw.hex()
        return s + comment

    def write_group(self, vg, lab, entries, kind):
        filler = '\tvoice_square_1 60, 0, 0, 2, 0, 0, 15, 0'
        lines = []
        idx = sorted(entries)
        first = idx[0] if kind == 'drum' and idx else 0
        if kind == 'drum':
            lines.append('voice_group %s, %d' % (lab[len('voicegroup_'):], first))
        else:
            lines.append('voice_group %s' % lab[len('voicegroup_'):])
        for i in range(first, idx[-1] + 1 if idx else 0):
            if i in entries:
                if kind == 'drum':
                    words = ['key', str(i), GM_DRUMS.get(i, '')]
                elif kind == 'top':
                    words = [str(i), 'drumset' if entries[i].kind == 'drum' else (GM[i] if i < 128 else '')]
                else:
                    words = ['split', str(i)]
                lines.append(self.tone_line(entries[i], '\t/* %s */' % ' '.join(w for w in words if w)))
            else:
                lines.append(filler + '\t/* %d unused by these songs */' % i)
        return '\n'.join(lines) + '\n'

    def write(self, vg_names, usage):
        out = self.out
        for sub in ('songs', 'midi', 'voicegroups', 'direct_sound_samples', 'programmable_wave_samples'):
            os.makedirs(os.path.join(out, sub), exist_ok=True)
        rom = self.rom
        # samples
        dsd = []
        for p, lab in self.new_samples.items():
            fn = lab[len('DirectSoundWaveData_'):]
            write_wav(rom, p, os.path.join(out, 'direct_sound_samples', fn + '.wav'))
            dsd.append('\t.align 2\n%s::\n\t.incbin "sound/direct_sound_samples/%s.bin"\n' % (lab, fn))
        open(os.path.join(out, 'direct_sound_data.inc'), 'w').write(
            '/* Append to sound/direct_sound_data.inc (the .wav files go to sound/direct_sound_samples/). */\n\n'
            + '\n'.join(dsd))
        pwd = []
        for p, lab in self.new_pw.items():
            fn = lab[len('ProgrammableWaveData_'):]
            open(os.path.join(out, 'programmable_wave_samples', fn + '.pcm'), 'wb').write(rom.d[p:p + 16])
            pwd.append('    .align 2\n%s::\n\t.incbin "sound/programmable_wave_samples/%s.pcm"\n' % (lab, fn))
        open(os.path.join(out, 'programmable_wave_data.inc'), 'w').write(
            '/* Append to sound/programmable_wave_data.inc. */\n\n' + '\n'.join(pwd))
        # keysplit tables
        ks = ['/* Append to sound/keysplit_tables.inc. */\n']
        for tbl, (lab, keys) in self.new_splits.items():
            lo = min(keys)
            hi = max(keys)
            ks.append('keysplit %s, %d' % (lab[len('keysplit_'):], lo))
            k = lo
            while k <= hi:
                v = rom.d[tbl + k]
                e = k
                while e + 1 <= hi and rom.d[tbl + e + 1] == v:
                    e += 1
                ks.append('\tsplit %d, %d' % (v, e + 1))
                k = e + 1
            ks.append('')
        open(os.path.join(out, 'keysplit_tables.inc'), 'w').write('\n'.join(ks) + '\n')
        # voicegroups
        vgi = ['/* Add to sound/voice_groups.inc (sub-groups first). */']
        for vg, (lab, entries, kind) in sorted(self.new_groups.items(),
                                               key=lambda x: (x[1][2] == 'top', x[0])):
            fn = lab[len('voicegroup_'):]
            sub = '' if kind == 'top' else ('drumsets/' if kind == 'drum' else 'keysplits/')
            os.makedirs(os.path.join(out, 'voicegroups', sub), exist_ok=True)
            open(os.path.join(out, 'voicegroups', sub + fn + '.inc'), 'w').write(
                self.write_group(vg, lab, entries, kind))
            vgi.append('.include "sound/voicegroups/%s%s.inc"' % (sub, fn))
        open(os.path.join(out, 'voice_groups.inc'), 'w').write('\n'.join(vgi) + '\n')
        # songs
        cfg = []
        table = ['/* Song table rows for ids %d-%d (append after Kai\'s own rows in that order, or'
                 % (self.songs[0].id, self.songs[-1].id),
                 '   map MUS_QZ_<id> to wherever they land). */']
        for s in self.songs:
            grp = self.label_of_group.get(s.vg) or ('voicegroup_qz_unknown_%x' % s.vg)
            open(os.path.join(out, 'songs', s.name + '.s'), 'w').write(s.asm(grp))
            loop, why = song_loop(s.sim)
            if why:
                s.warnings.append(why)
            mid, skipped = song_to_midi(s, s.sim, loop)
            if skipped:
                s.warnings.append('track(s) %s play no notes: left out of the MIDI' % skipped)
            s.loop = loop
            open(os.path.join(out, 'midi', s.name + '.mid'), 'wb').write(mid)
            opts = '-E'
            if s.reverb & 0x80:
                opts += ' -R%d' % (s.reverb & 0x7F)
            opts += ' -G%s -V127' % grp[len('voicegroup'):]
            if s.priority:
                opts += ' -P%d' % s.priority
            cfg.append('%-24s %s' % (s.name + '.mid:', opts))
            mp = {0: 'MUSIC_PLAYER_BGM', 1: 'MUSIC_PLAYER_SE1', 2: 'MUSIC_PLAYER_SE2',
                  3: 'MUSIC_PLAYER_SE3'}.get(s.ms, str(s.ms))
            table.append('\tsong %s, %s, %d' % (s.name, mp, s.me))
        open(os.path.join(out, 'midi', 'midi.cfg'), 'w').write('\n'.join(cfg) + '\n')
        open(os.path.join(out, 'song_table.inc'), 'w').write('\n'.join(table) + '\n')


def song_loop(sim):
    loops = set(s['loop'] for s in sim if s['notes'] or s['loop'])
    looping = [l for l in loops if l]
    if not looping:
        return None, None
    if len(set(looping)) == 1 and None not in loops:
        return looping[0], None
    best = collections.Counter(s['loop'] for s in sim if s['loop']).most_common(1)[0][0]
    return best, 'tracks loop at different points %s: the MIDI uses %s' % (sorted(l for l in loops if l), best)


def write_wav(rom, p, path):
    d = rom.d
    flags, pitch, ls, size = struct.unpack_from('<IIII', d, p)
    data = bytes(((x + 128) & 0xFF) for x in d[p + 16:p + 16 + size])
    rate = max(1, int(round(pitch / 1024.0)))
    fmt = struct.pack('<HHIIHH', 1, 1, rate, rate, 1, 8)
    chunks = [(b'fmt ', fmt)]
    loop = bool(flags & 0x40000000)
    smpl = struct.pack('<IIIIIIIII', 0, 0, int(round(1e9 / rate)), 60, 0, 0, 0, 1 if loop else 0, 0)
    if loop:
        smpl += struct.pack('<IIIIII', 0, 0, ls, max(ls, size - 1), 0, 0)
    chunks.append((b'smpl', smpl))
    chunks.append((b'agbp', struct.pack('<I', pitch)))
    chunks.append((b'agbl', struct.pack('<I', size)))
    chunks.append((b'data', data))
    body = b'WAVE'
    for cid, c in chunks:
        body += cid + struct.pack('<I', len(c)) + c + (b'\0' if len(c) & 1 else b'')
    open(path, 'wb').write(b'RIFF' + struct.pack('<I', len(body)) + body)


# ---------------------------------------------------------------------------------------------
# Verification
# ---------------------------------------------------------------------------------------------
def assemble(s_path, kai_root, tmp, base_symbol_defs, rodata_addr):
    """Assemble a song .s for 32-bit with GNU as, link .rodata at rodata_addr -> (bytes, symbols)."""
    obj = os.path.join(tmp, 'x.o')
    elf = os.path.join(tmp, 'x.elf')
    binf = os.path.join(tmp, 'x.bin')
    src = open(s_path).read()
    src = src.replace('.include "asm/macros/bit_width.inc"',
                      '.include "%s"' % os.path.join(kai_root, 'asm/macros/bit_width.inc'))
    src = src.replace('.include "MPlayDef.s"', '.include "%s"' % os.path.join(kai_root, 'sound/MPlayDef.s'))
    # keep .align working: start the section on a word boundary and pad up to the real address
    pad = rodata_addr & 3
    src = '\t.section .rodata\n\t.space %d\n' % pad + src
    sp = os.path.join(tmp, 'x.s')
    open(sp, 'w').write(src)
    r = subprocess.run(['as', '--32', '--defsym', 'VER_64BIT=0', '-o', obj, sp], capture_output=True, text=True)
    if r.returncode:
        raise RuntimeError('as failed: ' + r.stderr[:2000])
    defs = []
    for k, v in base_symbol_defs.items():
        defs += ['--defsym', '%s=0x%X' % (k, v)]
    r = subprocess.run(['ld', '-m', 'elf_i386', '-o', elf, '--section-start=.rodata=0x%X' % (rodata_addr - pad),
                        '-e', '0'] + defs + [obj], capture_output=True, text=True)
    if r.returncode:
        raise RuntimeError('ld failed: ' + r.stderr[:2000])
    subprocess.run(['objcopy', '-O', 'binary', '-j', '.rodata', elf, binf], check=True)
    syms = {}
    for line in subprocess.run(['nm', elf], capture_output=True, text=True).stdout.splitlines():
        p = line.split()
        if len(p) == 3:
            syms[p[2]] = int(p[0], 16)
    return open(binf, 'rb').read()[pad:], syms


def summarize(sim, quantize_vel):
    """What a song plays, comparable between the ROM and a mid2agb rebuild."""
    notes = []
    ctl = []
    tempo = []
    for s in sim:
        tempo += s['tempo']
    tempo = dedup_series(sorted(tempo))
    per_track = []
    for s in sim:
        if not s['notes']:
            continue
        ns = []
        for st, k, vel, dur, state in s['notes']:
            v = LUT[vel] if quantize_vel else vel
            st_ = list(state)
            st_[11] = 0           # KEYSH is shown separately
            ns.append((st, k, v, dur, tuple(st_)))
        c = collections.defaultdict(list)
        for t, cmd, v in s['ctl']:
            if cmd in (0xBC,):
                continue
            c[cmd].append((t, v))
        cc = {}
        for cmd, series in c.items():
            dd = dedup_series(series, default=DEFAULTS.get(cmd))
            if dd:
                cc[cmd] = dd
        per_track.append((sorted(ns), cc))
    return tempo, per_track


DEFAULTS = {0xBE: 127, 0xBF: 64, 0xC0: 64, 0xC1: 2, 0xC4: 0, 0xC5: 0, 0xC3: 0, 0xC8: 64, 0xBA: 0}
LUT = [0] + [min(127, ((v + 3) // 4) * 4) if v < 125 else 127 for v in range(1, 128)]


def dedup_series(series, default=None):
    """Keep the last value per tick, then drop entries that don't change the value."""
    last_at = collections.OrderedDict()
    for t, v in series:
        last_at[t] = v
    out = []
    cur = default
    for t, v in sorted(last_at.items()):
        if v != cur:
            out.append((t, v))
            cur = v
    return out


def compare_play(a, b):
    ta, tra = a
    tb, trb = b
    probs = []
    if ta != tb:
        probs.append('tempo differs')
    if len(tra) != len(trb):
        probs.append('%d vs %d tracks' % (len(tra), len(trb)))
    nd = 0
    nn = 0
    cd = 0
    for (na, ca), (nb, cb) in zip(tra, trb):
        nn += len(na)
        if na != nb:
            sa, sb = collections.Counter(na), collections.Counter(nb)
            nd += sum((sa - sb).values())
        for cmd in set(ca) | set(cb):
            if ca.get(cmd, []) != cb.get(cmd, []):
                cd += 1
    if nd:
        probs.append('%d of %d notes differ' % (nd, nn))
    if cd:
        probs.append('%d controller series differ' % cd)
    return probs


def verify(rip, kai_root, mid2agb, wav2agb, tmp_root):
    rom = rip.rom
    report = []
    tmp = tempfile.mkdtemp(dir=tmp_root)
    ok_bytes = 0
    ok_midi = 0
    vel_changed = 0
    for s in rip.songs:
        lo = min(s.ins)
        grp = rip.label_of_group.get(s.vg)
        defs = {grp: s.vg + ROM_BASE}
        blob, syms = assemble(os.path.join(rip.out, 'songs', s.name + '.s'), kai_root, tmp, defs, lo + ROM_BASE)
        want = rom.d[lo:s.header + 8 + 4 * s.ntracks]
        if syms.get(s.name) != s.header + ROM_BASE:
            report.append('%s: header label at 0x%X, ROM 0x%X' % (s.name, syms.get(s.name, 0), s.header + ROM_BASE))
        elif blob != want:
            diff = next((i for i in range(min(len(blob), len(want))) if blob[i] != want[i]), min(len(blob), len(want)))
            report.append('%s: assembled bytes differ from the ROM at 0x%X (%d vs %d bytes)'
                          % (s.name, lo + diff + ROM_BASE, len(blob), len(want)))
        else:
            ok_bytes += 1
        # MIDI -> mid2agb -> as -> what it plays
        if mid2agb:
            os.makedirs(os.path.join(tmp, 'm'), exist_ok=True)
            sp = os.path.join(tmp, 'm', s.name + '.s')      # mid2agb names the song after the file
            cfg = open(os.path.join(rip.out, 'midi', 'midi.cfg')).read()
            opts = re.search(re.escape(s.name) + r'\.mid:\s*(.*)', cfg).group(1).split()
            r = subprocess.run([mid2agb, os.path.join(rip.out, 'midi', s.name + '.mid'), sp] + opts,
                               capture_output=True, text=True)
            if r.returncode:
                report.append('%s: mid2agb failed: %s' % (s.name, (r.stdout + r.stderr)[:300]))
                continue
            base = 0x09000000
            blob2, syms2 = assemble(sp, kai_root, tmp, {grp: s.vg + ROM_BASE}, base)
            h = syms2[s.name] - base
            nt = blob2[h]
            mem = bytearray(blob2)

            def ptr_at(a, mem=mem):
                v = struct.unpack_from('<I', mem, a)[0]
                return v - base if base <= v < base + len(mem) else None
            tracks = [ptr_at(h + 8 + 4 * i) for i in range(nt)]
            w = []
            sim2 = [simulate_track(bytes(mem), ptr_at, t, w) for t in tracks]
            orig = summarize(s.sim, True)
            new = summarize(sim2, False)
            probs = compare_play(orig, new)
            loop2, _ = song_loop(sim2)
            if loop2 != s.loop:
                probs.append('loop %s vs %s' % (s.loop, loop2))
            for tr in s.sim:
                vel_changed += sum(1 for n in tr['notes'] if LUT[n[2]] != n[2])
            if probs:
                report.append('%s: MIDI round trip: %s' % (s.name, '; '.join(probs)))
            else:
                ok_midi += 1
    report.insert(0, 'assembled .s == ROM bytes: %d / %d songs' % (ok_bytes, len(rip.songs)))
    if mid2agb:
        report.insert(1, 'MIDI -> mid2agb plays the same notes/controllers/tempo/loop: %d / %d songs'
                      % (ok_midi, len(rip.songs)))
        report.insert(2, 'notes whose velocity mid2agb rounds to a multiple of 4: %d' % vel_changed)
    if wav2agb:
        okw = 0
        for p, lab in rip.new_samples.items():
            fn = lab[len('DirectSoundWaveData_'):]
            wavp = os.path.join(rip.out, 'direct_sound_samples', fn + '.wav')
            binp = os.path.join(tmp, fn + '.bin')
            subprocess.run([wav2agb, '-b', wavp, binp], check=True, capture_output=True)
            b = open(binp, 'rb').read()
            size = struct.unpack_from('<I', rom.d, p + 12)[0]
            if b[:16 + size] == rom.d[p:p + 16 + size] and wav_to_agb_bin(wavp) == rom.d[p:p + 16 + size]:
                okw += 1
            else:
                report.append('%s: wav2agb output differs from the ROM sample' % fn)
        report.append('new .wav samples converting back to the ROM bytes (wav2agb -b): %d / %d'
                      % (okw, len(rip.new_samples)))
    shutil.rmtree(tmp, ignore_errors=True)
    return report


# ---------------------------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('rom', nargs='?', default=os.environ.get('QUETZAL_ROM'))
    ap.add_argument('--out', default=os.path.join(REPO, 'quetzal_rip', 'sound'))
    ap.add_argument('--kai', default=REPO, help='Kai checkout to match voicegroups / samples against')
    ap.add_argument('--first', type=int, default=559)
    ap.add_argument('--last', type=int, default=665)
    ap.add_argument('--verify', action='store_true')
    ap.add_argument('--mid2agb', help='Kai mid2agb binary (for --verify)')
    ap.add_argument('--wav2agb', help='Kai wav2agb binary (for --verify)')
    ap.add_argument('--tmp', default=None)
    a = ap.parse_args()
    if not a.rom:
        sys.exit('give the Moemon Quetzal ROM path (argument or QUETZAL_ROM)')
    rom = Rom(a.rom)
    kai = Kai(a.kai)
    rip = Ripper(rom, kai, a.out, a.first, a.last)
    rip.stock_names = kai_song_names(a.kai)
    rip.collect_use()
    rip.resolve()
    # names for voicegroups that are new
    users = collections.defaultdict(list)
    for s in rip.songs:
        users[s.vg].append(s.id)
    vg_names = {}
    for vg, ids in users.items():
        if len(ids) > 1:
            vg_names[vg] = 'qz_%d_to_%d' % (min(ids), max(ids)) if len(ids) < 3 else 'qz_songs_%d' % min(ids)
        else:
            vg_names[vg] = 'qz_%d' % ids[0]
    named = dict(QZ_VG_NAMES)
    for vg in vg_names:
        if vg + ROM_BASE in named:
            vg_names[vg] = named[vg + ROM_BASE]
    rip.plan_new(vg_names)
    if os.path.isdir(a.out):
        for sub in ('songs', 'midi', 'voicegroups', 'direct_sound_samples', 'programmable_wave_samples'):
            shutil.rmtree(os.path.join(a.out, sub), ignore_errors=True)
    usage = song_usage(os.path.join(REPO, 'quetzal_rip'))
    rip.write(vg_names, usage)
    write_readme(rip, usage)
    print('%d songs, %d new voicegroups (%d sub-groups), %d new samples, %d reused Kai samples, '
          '%d new wave tables, %d new keysplit tables'
          % (len(rip.songs), sum(1 for v in rip.new_groups.values() if v[2] == 'top'),
             sum(1 for v in rip.new_groups.values() if v[2] != 'top'), len(rip.new_samples),
             len(rip.reused_samples), len(rip.new_pw), len(rip.new_splits)))
    for s in rip.songs:
        for w in sorted(set(s.warnings)):
            print('  %s: %s' % (s.name, w))
    if a.verify:
        def tool(name):     # Kai's own build of the tool (make -C tools/<name>), else PATH
            p = os.path.join(a.kai, 'tools', name, name)
            return p if os.path.exists(p) else shutil.which(name)
        mid2agb = a.mid2agb or tool('mid2agb')
        wav2agb = a.wav2agb or tool('wav2agb')
        for line in verify(rip, a.kai, mid2agb, wav2agb, a.tmp):
            print('verify:', line)


# Song-name guesses, by the maps / scripts that play each song and by listening to what the ROM
# code picks them for. Filled from song_usage() for map songs; the rest are guesses noted as such.
QZ_VG_NAMES = {0x09394018: 'qz_songs', 0x09393004: 'qz_610'}
# Guesses for Quetzal's songs. Map songs: from the maps that play them (HGSS / GSC names). Event songs:
# from the scripts around their playbgm. Trainer "eyes meet" songs: from the ROM's PlayTrainerEncounterMusic
# (0x08106D28 jump table: encounter music ids 14-23 -> 661 648 665 658 663 657 662 659 664 660) and the
# trainer classes that use each id in quetzal_rip/data/trainers.json.
SONG_GUESS = {
    610: 'Trainer Hill (Johto; own voicegroup)',
    611: 'New Bark Town',
    612: 'Cherrygrove City (also Olivine / Mahogany houses)',
    613: 'Violet City (also Olivine City)',
    614: 'Azalea Town (also Blackthorn City)',
    615: 'Goldenrod City',
    616: 'Ecruteak City',
    617: 'Cianwood City',
    618: 'Route 29',
    619: 'Route 30 (Routes 30-33)',
    620: 'Route 34 (Routes 34-37)',
    621: 'Route 38 (Routes 38-41)',
    622: 'Route 42 / Lake of Rage (Routes 42-46)',
    623: 'Route 26 / 27',
    624: 'Pokemon Center (Johto)',
    625: 'Poke Mart (Johto)',
    626: 'Pokemon Gym (Johto)',
    627: "Prof. Elm's Lab",
    628: 'Dance Theater (Kimono Girls)',
    629: 'Goldenrod Game Corner',
    630: 'Sprout Tower',
    631: 'Johto caves / forests (Union Cave, Ilex Forest, Dark Cave...)',
    632: 'Ruins of Alph',
    633: 'National Park / Bug-Catching Contest',
    634: 'Burned Tower',
    635: 'Tin (Bell) Tower',
    636: 'Mt. Silver (also Olivine Lighthouse)',
    637: 'Team Rocket Hideout',
    638: 'Ice Path',
    639: "Dragon's Den / Dragon Shrine",
    640: 'Victory Road (Johto)',
    641: 'Indigo Plateau / Elite Four rooms',
    642: 'Goldenrod Underground (warehouse)',
    643: 'Cliff Cave / Routes 47-48',
    644: 'Safari Zone Gate',
    645: 'Safari Zone (Johto)',
    646: "Prof. Oak (scene at Mr. Pokemon's House)",
    647: 'Follow me! (Guide Gent in Cherrygrove, Violet scene)',
    648: 'Rival appears (Johto rival encounter; encounter music 15)',
    649: 'Rival departs (after rival battles: Cherrygrove, Azalea, Burned Tower)',
    650: 'short jingle (? 4 s, no loop; picked by ROM code)',
    651: 'event in Cianwood City (Eusine / Suicune scene?)',
    652: 'Vs. Ho-Oh',
    653: 'Vs. Lugia',
    654: 'Lugia / Ho-Oh appears (Whirl Islands chamber, Tin Tower roof)',
    655: 'event theme (Goldenrod Underground, Ilex Forest, Ice Path scenes)',
    656: 'short sound effect (SE2 player, 2.5 s)',
    657: 'Trainer eyes meet: Lass / Picnicker / Twins (encounter music 19)',
    658: 'Trainer eyes meet: Youngster / Bug Catcher / boys (encounter music 17)',
    659: 'Trainer eyes meet: Super Nerd / Juggler (encounter music 21)',
    660: 'Trainer eyes meet: Sage / Hex Maniac (encounter music 23)',
    661: 'Trainer eyes meet: Team Rocket (encounter music 14)',
    662: 'Trainer eyes meet: Cooltrainer / Beauty / Swimmer F (encounter music 20)',
    663: 'Trainer eyes meet: Swimmer M / Policeman (encounter music 18)',
    664: 'Trainer eyes meet: Hiker / Fisherman / Sailor / Black Belt (encounter music 22)',
    665: 'Trainer eyes meet: Champion / PKMN Trainer (encounter music 16)',
}


def kai_song_names(kai_root):
    names = {}
    p = os.path.join(kai_root, 'include', 'constants', 'songs.h')
    if os.path.exists(p):
        for m in re.finditer(r'#define\s+((?:MUS|SE|PH)_\w+)\s+(\d+)', open(p).read()):
            names.setdefault(int(m.group(2)), m.group(1))
    return names


def song_length(s):
    """(ticks to the loop end or FINE, seconds, loop start tick)."""
    end = max([t['end'] for t in s.sim] + [0])
    loop = getattr(s, 'loop', None)
    if loop:
        end = loop[1]
    tempo = sorted(set(x for t in s.sim for x in t['tempo']))
    secs = 0.0
    cur_t, cur_bpm = 0, 150
    for t, T in tempo:
        if t > end:
            break
        secs += (t - cur_t) * 60.0 / (cur_bpm * 24)
        cur_t, cur_bpm = t, max(1, 2 * T)
    secs += (end - cur_t) * 60.0 / (cur_bpm * 24)
    return end, secs, loop[0] if loop else None


def guess_name(s, use, stock=None):
    if stock:
        return 'stock `%s` (Kai\'s `%s`)' % (stock, stock.lower())
    if s.id in SONG_GUESS:
        return SONG_GUESS[s.id]
    areas = collections.Counter()
    for m in use['maps']:
        areas[area_name(m)] += 1
    for sc in use['scripts']:
        m = sc.split(' ')[0].split(':')[0]
        if not m.endswith('.inc'):
            areas[area_name(m)] += 1
    if areas:
        names = [a for a, _ in areas.most_common(3)]
        return ' / '.join(names)
    if s.ms != 0:
        return 'sound effect / jingle'
    return '?'


def write_readme(rip, usage):
    L = []
    L.append('# Moemon Quetzal songs (ids %d-%d)\n' % (rip.songs[0].id, rip.songs[-1].id))
    L.append('Ripped from Moemon Quetzal (English Alpha 9) by `tools/quetzal_rip/songs.py`, which rebuilds '
             'everything here from the ROM. Song `mus_qz_<id>` is Quetzal song id `<id>` (the rip\'s '
             '`MUS_QZ_<id>`).\n')
    L.append('## Files\n')
    L.append('- `songs/mus_qz_<id>.s`: the song as it is in the ROM, in mid2agb\'s assembly format (Kai flavour: '
             '`ptrvalue` pointers, `space64` in the header). Assembled for 32 bits and linked at the ROM '
             'address it gives the ROM\'s bytes back exactly. Kai\'s Makefile only builds `sound/songs/midi/*.mid`, '
             'so a `.s` needs its own rule or a copy into a data `.s` file.')
    L.append('- `midi/mus_qz_<id>.mid` + `midi/midi.cfg`: the same songs for Kai\'s normal build path '
             '(drop the .mid files into `sound/songs/midi/` and append the cfg lines). 24 ticks per quarter '
             'note (mid2agb\'s clock), one MIDI track per song track, loop as `[` / `]` markers, '
             'VOL / PAN / BEND / BENDR / LFOS / MODT / TUNE / LFODL / MOD / PRIO / xIECV / xIECL as the '
             'controllers mid2agb reads. `-V127` keeps the ROM\'s VOL values.')
    L.append('- `voicegroups/`, `voice_groups.inc`: voicegroups Kai does not have (only the instruments these '
             'songs play are filled in; the other slots are the usual `voice_square_1` filler). Sub-groups '
             '(drumsets, keysplit groups) come first in `voice_groups.inc`.')
    L.append('- `direct_sound_samples/*.wav` + `direct_sound_data.inc`: new DirectSound samples. They are '
             '8-bit wavs with `smpl` (loop), `agbp` (exact pitch) and `agbl` (exact length) chunks, so Kai\'s '
             '`wav2agb -b` turns them back into the ROM\'s bytes. Samples that are byte-identical to one of '
             'Kai\'s are referenced by Kai\'s name instead.')
    L.append('- `programmable_wave_samples/`, `programmable_wave_data.inc`, `keysplit_tables.inc`: new wave '
             'tables / key splits (if any).')
    L.append('- `song_table.inc`: `song` rows with the ROM\'s music player and priority.\n')
    nnew = sum(1 for v in rip.new_groups.values() if v[2] == 'top')
    L.append('## Voicegroups\n')
    vgs = collections.defaultdict(list)
    for s in rip.songs:
        vgs[s.vg].append(s.id)
    for vg, ids in sorted(vgs.items()):
        lab = rip.label_of_group.get(vg)
        new = vg in rip.new_groups
        L.append('- `%s` (ROM 0x%08X): %s; songs %s' % (lab, vg + ROM_BASE,
                 'new, dumped here' if new else 'Kai already has it (same instruments)', id_ranges(ids)))
    if rip.new_groups:
        for vg, (lab, entries, kind) in rip.new_groups.items():
            if kind != 'top':
                L.append('- `%s` (ROM 0x%08X): new %s' % (lab, vg + ROM_BASE,
                                                         'drumset' if kind == 'drum' else 'keysplit group'))
    L.append('')
    L.append('%d new DirectSound samples, %d of Kai\'s samples reused.\n' % (len(rip.new_samples),
                                                                          len(rip.reused_samples)))
    L.append('## Songs\n')
    L.append('Length = to the loop end (or the end) at the song\'s tempo; "loop" = where it loops back to. '
             'Names are guesses from the maps / scripts of this rip that play the song.\n')
    L.append('| id | name guess | player | tracks | voicegroup | length | loop from | played by |')
    L.append('|---|---|---|---|---|---|---|---|')
    for s in rip.songs:
        u = usage.get(s.id, {'maps': set(), 'scripts': set()})
        end, secs, ls = song_length(s)
        where = sorted(u['maps'])
        if len(where) > 6:
            where = where[:6] + ['+%d more maps' % (len(u['maps']) - 6)]
        sc = sorted(u['scripts'])
        if len(sc) > 3:
            sc = sc[:3] + ['+%d more scripts' % (len(u['scripts']) - 3)]
        played = ', '.join(where + sc) or '-'
        mp = {0: 'BGM', 1: 'SE1', 2: 'SE2', 3: 'SE3'}.get(s.ms, str(s.ms))
        L.append('| %d | %s | %s | %d | %s | %d:%04.1f (%d ticks) | %s | %s |' % (
            s.id, guess_name(s, u, rip.stock_names.get(s.id) if s.id < 610 else None), mp, s.ntracks, (rip.label_of_group.get(s.vg) or '?')[len('voicegroup_'):],
            int(secs // 60), secs % 60, end, ls if ls is not None else '-', played))
    L.append('')
    probs = [(s.name, w) for s in rip.songs for w in sorted(set(s.warnings))]
    if probs:
        L.append('## Notes\n')
        for n, w in probs:
            L.append('- %s: %s' % (n, w))
        L.append('')
    open(os.path.join(rip.out, 'README.md'), 'w').write('\n'.join(L))


def id_ranges(ids):
    ids = sorted(ids)
    out = []
    i = 0
    while i < len(ids):
        j = i
        while j + 1 < len(ids) and ids[j + 1] == ids[j] + 1:
            j += 1
        out.append(str(ids[i]) if i == j else '%d-%d' % (ids[i], ids[j]))
        i = j + 1
    return ', '.join(out)


if __name__ == '__main__':
    main()
