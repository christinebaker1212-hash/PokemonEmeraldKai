#!/usr/bin/env python3
"""Render Pokémon Red's and Crystal's Poké Ball sound effects to PCM.

Moemon Co-op: offline tool that writes src/mp/mp_legacy_sfx.h (used by
src/mp/mp_legacy_sound.c) and 16-bit WAV previews.

Usage
-----
    python3 tools/moemon_legacy/sfx_render.py \
        --pokered  PATH/TO/pokered     (pret/pokered checkout, tested at af51989)
        --pokecrystal PATH/TO/pokecrystal  (pret/pokecrystal, tested at 97187cd)
        [--out src/mp/mp_legacy_sfx.h] [--wavdir DIR] [--peak 0.65]

The disassemblies are only read as data: the tool has its own small assembler
for the handful of directives and audio macros the sound files use (db, dw, dn,
labels, local labels and the macros in macros/scripts/audio.asm). Nothing from
the repositories is executed.

Model
-----
1. Sound engine. Each game's driver is reimplemented from its source, one call
   per Game Boy frame (70224 CPU cycles, 59.7275 Hz), writing APU registers in
   the order the original code does:
   * Red: audio/engine_2.asm (the battle-bank copy). Audio2_PlaySound's SFX
     path (header lookup in SFX_Headers_2, per-channel reset, NR10 = $08 for
     channel 5), Audio2_UpdateMusic / ApplyMusicAffects / PlayNextNote and the
     command chain (sound_ret/call/loop, note_type, toggle_perfect_pitch,
     vibrato, duty_cycle(_pattern), tempo, stereo_panning, volume,
     execute_music, octave, square_note / noise_note, pitch_sweep, notes and
     rests), the note-length maths (Audio2_note_length with the 8.8 fixed point
     delay and Audio2_SetSfxTempo), CalculateFrequency, EnableChannelOutput
     (NR51), ApplyDutyCycleAndSoundLength and ApplyWavePatternAndFrequency
     (NRx4 = (hi | $80) & $c7, wave RAM reload). The engine state starts as
     Audio2_PlaySound.stopAllAudio leaves it (NR50 = $77, NR51 = 0).
   * Crystal: audio/engine.asm. _InitSound, _PlaySFX (mono: the battle
     animations use PlayStereoSFX, which falls back to _PlaySFX with the
     default MONO sound option), _UpdateSound (note duration countdown,
     ParseMusic, HandleTrackVibrato, HandleNoise / drum samples, UpdateChannels
     with its note-flag driven register writes and ClearChannel on rests and at
     the end, NR50/NR51 written at the end of every frame from wVolume and the
     active channels' tracks), SetNoteDuration with the tempo and duration
     modifier, GetFrequency, Music_* commands.
   Not reimplemented (none of these sounds uses them; the tool stops with an
   error if a sound does): pitch_slide, cries (frequency/tempo modifiers),
   the low HP alarm, Crystal's conditional jumps / restart_channel / new_song.
2. APU. A Game Boy APU model driven by those writes at the CPU clock
   (4194304 Hz, every cycle computed): two square channels (4 duty patterns,
   11-bit period timer, length counter, volume envelope, channel 1 sweep with
   the overflow check, the second overflow check and the "negate then clear
   negate" disable), the wave channel (32 4-bit samples, sample buffer that is
   not refilled on trigger, output level shift), the noise channel (15/7-bit
   LFSR, divisor and clock shift), the 512 Hz frame sequencer (length at
   256 Hz, sweep at 128 Hz, envelope at 64 Hz), DACs (digital 0..15 -> analog
   +1..-1, 0 when the DAC is off), NR51 routing and NR50 master volume.
3. Output. The per-cycle left/right signal is averaged to mono, integrated
   exactly over 4x-oversampled output intervals (168192 Hz, box filter on the
   cycle grid with fractional edges), low-passed by a 255-tap Kaiser windowed
   sinc FIR (cutoff 19.5 kHz) and decimated to 42048 Hz, then passed through
   the hardware's DC-blocking high-pass (capacitor charge factor
   0.999958 per cycle, i.e. 0.999958 ** (4194304 / 42048) per output sample).
   All sounds share one fixed gain (the loudest sample of the whole set peaks
   at --peak of full scale) so their relative loudness is the Game Boy's.
   Each sound is trimmed after the point where the engine has finished and no
   channel can change its output any more, keeping 20 ms of the high-pass
   tail with a 5 ms fade at the very end.

Known approximations
--------------------
* Engine register writes of one frame all land on the same APU cycle (the
  real driver spreads them over a few hundred cycles) and the game's call to
  PlaySound / PlaySFX is put on the first update frame.
* The frame sequencer phase relative to the first frame is fixed (first step
  2048 cycles in); on hardware it is effectively random, so envelope / sweep
  steps can land up to one step (2-15 ms) earlier or later.
* Obscure APU quirks are left out: "zombie mode" NRx2 writes, the extra
  length clock when enabling length, the trigger delay of the period timers,
  wave RAM corruption on DMG. None of these sounds hits them (no length
  counter is ever enabled: both engines leave NRx4 bit 6 clear).
* One DAC / mixer / high-pass model for both games (Red is DMG, Crystal CGB);
  the CGB's analog differences are not modelled. The high-pass is not gated
  by "all DACs off".
* RAM the engines do not initialise for a new sound (Red's
  wChannelNoteDelayCountersFractionalPart) is taken as 0; with these sounds'
  tempos ($100) it cannot change any note length.
"""

import argparse
import math
import os
import re
import struct
import subprocess
import sys
import wave

import numpy as np

CPU_HZ = 4194304
FRAME_CYCLES = 70224            # one Game Boy frame (LCD refresh): 59.7275 Hz
OUT_HZ = 42048
OVERSAMPLE = 4
FS_PERIOD = 8192                # frame sequencer: 512 Hz
FS_PHASE = 2048                 # first frame sequencer step, cycles after start
HPF_CHARGE = 0.999958           # DMG high-pass capacitor charge factor per cycle
TAIL_MS = 20.0
FADE_MS = 5.0
MAX_SECONDS = 8.0


# ---------------------------------------------------------------------------
# Mini assembler for the audio data files
# ---------------------------------------------------------------------------

class AsmError(Exception):
    pass


_TOKEN_RE = re.compile(r"\s*(?:(\$[0-9A-Fa-f]+)|(%[01]+)|(\d+)|([A-Za-z_.][A-Za-z0-9_#.]*)|(<<|>>|[-+*/|&()~^]))")


class Expr:
    """Tiny integer expression evaluator: numbers ($hex, %bin, decimal),
    symbols, unary -/~, * /, + -, << >>, &, ^, |, parentheses, HIGH()/LOW()."""

    def __init__(self, text, lookup):
        self.toks = []
        pos = 0
        text = text.strip()
        while pos < len(text):
            m = _TOKEN_RE.match(text, pos)
            if not m or m.end() == pos:
                raise AsmError("bad expression: %r" % text)
            pos = m.end()
            if m.group(1):
                self.toks.append(("n", int(m.group(1)[1:], 16)))
            elif m.group(2):
                self.toks.append(("n", int(m.group(2)[1:], 2)))
            elif m.group(3):
                self.toks.append(("n", int(m.group(3))))
            elif m.group(4):
                self.toks.append(("s", m.group(4)))
            else:
                self.toks.append(("o", m.group(5)))
        self.i = 0
        self.lookup = lookup

    def value(self):
        v = self._or()
        if self.i != len(self.toks):
            raise AsmError("trailing tokens in expression")
        return v

    def _peek(self):
        return self.toks[self.i] if self.i < len(self.toks) else (None, None)

    def _take(self):
        t = self._peek()
        self.i += 1
        return t

    def _binary(self, sub, ops):
        v = sub()
        while self._peek()[0] == "o" and self._peek()[1] in ops:
            op = self._take()[1]
            w = sub()
            v = {"|": lambda a, b: a | b, "^": lambda a, b: a ^ b, "&": lambda a, b: a & b,
                 "<<": lambda a, b: a << b, ">>": lambda a, b: a >> b,
                 "+": lambda a, b: a + b, "-": lambda a, b: a - b,
                 "*": lambda a, b: a * b, "/": lambda a, b: int(a / b)}[op](v, w)
        return v

    def _or(self):
        return self._binary(self._xor, ("|",))

    def _xor(self):
        return self._binary(self._and, ("^",))

    def _and(self):
        return self._binary(self._shift, ("&",))

    def _shift(self):
        return self._binary(self._add, ("<<", ">>"))

    def _add(self):
        return self._binary(self._mul, ("+", "-"))

    def _mul(self):
        return self._binary(self._unary, ("*", "/"))

    def _unary(self):
        t = self._peek()
        if t == ("o", "-"):
            self._take()
            return -self._unary()
        if t == ("o", "~"):
            self._take()
            return ~self._unary()
        if t == ("o", "+"):
            self._take()
            return self._unary()
        return self._atom()

    def _atom(self):
        kind, val = self._take()
        if kind == "n":
            return val
        if kind == "o" and val == "(":
            v = self._or()
            if self._take() != ("o", ")"):
                raise AsmError("missing )")
            return v
        if kind == "s":
            if val in ("HIGH", "LOW") and self._peek() == ("o", "("):
                self._take()
                v = self._or()
                if self._take() != ("o", ")"):
                    raise AsmError("missing )")
                return (v >> 8) & 0xFF if val == "HIGH" else v & 0xFF
            return self.lookup(val)
        raise AsmError("unexpected token %r" % (val,))


def split_args(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


class Unresolved(Exception):
    pass


class Assembler:
    """Assembles files into one flat address space. Address 0 is never used
    (it stands for a null pointer)."""

    def __init__(self, consts, macros):
        self.mem = bytearray(16)
        self.labels = {}
        self.fixups = []        # (pos, size, expr, scope, where)
        self.consts = consts
        self.macros = macros
        self.state = {}
        self.scope = ""
        self.missing = set()

    # -- emission helpers used by the macros
    def byte(self, v):
        self.mem.append(v & 0xFF)

    def nib(self, hi, lo):
        self.byte(((hi & 0xF) << 4) | (lo & 0xF))

    def ev(self, text):
        return Expr(text, self._lookup_const).value()

    def word(self, text, big=False):
        pos = len(self.mem)
        self.mem += b"\0\0"
        self.fixups.append((pos, "bigw" if big else "w", text, self.scope, self.where))

    def _lookup_const(self, name):
        if name in self.consts:
            return self.consts[name]
        raise Unresolved(name)

    def _lookup_label(self, scope):
        def look(name):
            if name in self.consts:
                return self.consts[name]
            full = scope + name if name.startswith(".") else name
            if full in self.labels:
                return self.labels[full]
            raise Unresolved(full)
        return look

    def assemble(self, path, start_label=None):
        self.mem += b"\0" * ((-len(self.mem)) % 256)
        self.scope = ""
        if start_label:
            self.labels[start_label] = len(self.mem)
            self.scope = start_label
        with open(path, encoding="utf-8") as f:
            lines = f.read().split("\n")
        for n, raw in enumerate(lines, 1):
            self.where = "%s:%d" % (os.path.basename(path), n)
            line = raw.split(";", 1)[0].rstrip()
            if not line.strip():
                continue
            m = re.match(r"^([A-Za-z_.][A-Za-z0-9_.]*)(::?)(.*)$", line) or \
                re.match(r"^(\.[A-Za-z0-9_]+)()(\s*)$", line)      # local label without colon
            if m:
                name = m.group(1)
                if name.startswith("."):
                    name = self.scope + name
                else:
                    self.scope = name
                if name in self.labels:
                    raise AsmError("%s: duplicate label %s" % (self.where, name))
                self.labels[name] = len(self.mem)
                line = m.group(3)
                if not line.strip():
                    continue
            parts = line.strip().split(None, 1)
            op = parts[0]
            args = split_args(parts[1]) if len(parts) > 1 else []
            try:
                self.directive(op, args)
            except Unresolved as e:
                raise AsmError("%s: unknown symbol %s" % (self.where, e))

    def directive(self, op, args):
        if op == "db":
            for a in args:
                self.byte(self.ev(a))
        elif op == "dw":
            for a in args:
                self.word(a)
        elif op == "dn":
            vals = [self.ev(a) for a in args]
            if len(vals) % 2:
                raise AsmError("%s: odd dn" % self.where)
            for i in range(0, len(vals), 2):
                self.nib(vals[i], vals[i + 1])
        elif op in ("table_width", "assert_table_length", "assert", "ASSERT"):
            pass
        elif op in self.macros:
            self.macros[op](self, args)
        else:
            raise AsmError("%s: unsupported directive/macro %r" % (self.where, op))

    def resolve(self):
        for pos, kind, text, scope, where in self.fixups:
            try:
                v = Expr(text, self._lookup_label(scope)).value()
            except Unresolved as e:
                # a header entry for a sound we did not assemble: left as a null
                # pointer, and an error if an engine ever reads it
                self.missing.add(str(e))
                v = 0
            v &= 0xFFFF
            if kind == "bigw":
                self.mem[pos], self.mem[pos + 1] = v >> 8, v & 0xFF
            else:
                self.mem[pos], self.mem[pos + 1] = v & 0xFF, v >> 8

    def rd(self, addr):
        if addr <= 0 or addr >= len(self.mem):
            raise AsmError("engine read outside assembled data: $%x" % addr)
        return self.mem[addr]

    def rdw(self, addr):
        return self.rd(addr) | (self.rd(addr + 1) << 8)


def _fade(a, asm, arg):
    v = asm.ev(arg)
    return (8 | -v) if v < 0 else v


# --- macros/scripts/audio.asm of pokered
def red_macros():
    m = {}

    def channel_count(a, args):
        a.state["num"] = a.ev(args[0]) - 1

    def channel(a, args):
        a.nib(a.state.get("num", 0) << 2, a.ev(args[0]) - 1)
        a.word(args[1])
        a.state["num"] = 0

    def pitch_sweep(a, args):
        a.byte(0x10)
        a.nib(a.ev(args[0]), _fade(None, a, args[1]))

    def square_note(a, args):
        a.byte(0x20 | a.ev(args[0]))
        a.nib(a.ev(args[1]), _fade(None, a, args[2]))
        f = a.ev(args[3])
        a.byte(f & 0xFF)
        a.byte(f >> 8)

    def noise_note(a, args):
        a.byte(0x20 | a.ev(args[0]))
        a.nib(a.ev(args[1]), _fade(None, a, args[2]))
        a.byte(a.ev(args[3]))

    def note(a, args):
        a.nib(a.ev(args[0]), a.ev(args[1]) - 1)

    def drum_note(a, args):
        a.byte(0xB0 | (a.ev(args[1]) - 1))
        a.byte(a.ev(args[0]))

    def rest(a, args):
        a.byte(0xC0 | (a.ev(args[0]) - 1))

    def note_type(a, args):
        a.byte(0xD0 | a.ev(args[0]))
        a.nib(a.ev(args[1]), _fade(None, a, args[2]))

    def drum_speed(a, args):
        a.byte(0xD0 | a.ev(args[0]))

    def octave(a, args):
        a.byte(0xE0 | (8 - a.ev(args[0])))

    def simple(code):
        return lambda a, args: a.byte(code)

    def vibrato(a, args):
        a.byte(0xEA)
        a.byte(a.ev(args[0]))
        a.nib(a.ev(args[1]), a.ev(args[2]))

    def pitch_slide(a, args):
        a.byte(0xEB)
        a.byte(a.ev(args[0]) - 1)
        a.nib(8 - a.ev(args[1]), a.ev(args[2]))

    def duty_cycle(a, args):
        a.byte(0xEC)
        a.byte(a.ev(args[0]))

    def tempo(a, args):
        a.byte(0xED)
        t = a.ev(args[0])
        a.byte(t >> 8)
        a.byte(t & 0xFF)

    def stereo_panning(a, args):
        a.byte(0xEE)
        a.nib(a.ev(args[0]), a.ev(args[1]))

    def unknown_ef(a, args):
        a.byte(0xEF)
        a.byte(a.ev(args[0]))

    def volume(a, args):
        a.byte(0xF0)
        a.nib(a.ev(args[0]), a.ev(args[1]))

    def duty_cycle_pattern(a, args):
        v = [a.ev(x) for x in args]
        a.byte(0xFC)
        a.byte(v[0] << 6 | v[1] << 4 | v[2] << 2 | v[3])

    def sound_call(a, args):
        a.byte(0xFD)
        a.word(args[0])

    def sound_loop(a, args):
        a.byte(0xFE)
        a.byte(a.ev(args[0]))
        a.word(args[1])

    m.update(channel_count=channel_count, channel=channel, pitch_sweep=pitch_sweep,
             square_note=square_note, noise_note=noise_note, note=note, drum_note=drum_note,
             drum_note_short=note, rest=rest, note_type=note_type, drum_speed=drum_speed,
             octave=octave, toggle_perfect_pitch=simple(0xE8), vibrato=vibrato,
             pitch_slide=pitch_slide, duty_cycle=duty_cycle, tempo=tempo,
             stereo_panning=stereo_panning, unknownmusic0xef=unknown_ef, volume=volume,
             execute_music=simple(0xF8), duty_cycle_pattern=duty_cycle_pattern,
             sound_call=sound_call, sound_loop=sound_loop, sound_ret=simple(0xFF))
    return m


# --- macros/scripts/audio.asm of pokecrystal
def crystal_macros():
    m = {}

    def channel_count(a, args):
        a.state["num"] = a.ev(args[0]) - 1

    def channel(a, args):
        a.nib(a.state.get("num", 0) << 2, a.ev(args[0]) - 1)
        a.word(args[1])
        a.state["num"] = 0

    def note(a, args):
        a.nib(a.ev(args[0]), a.ev(args[1]) - 1)

    def rest(a, args):
        a.nib(0, a.ev(args[0]) - 1)

    def square_note(a, args):
        a.byte(a.ev(args[0]))
        a.nib(a.ev(args[1]), _fade(None, a, args[2]))
        f = a.ev(args[3])
        a.byte(f & 0xFF)
        a.byte(f >> 8)

    def noise_note(a, args):
        a.byte(a.ev(args[0]))
        a.nib(a.ev(args[1]), _fade(None, a, args[2]))
        a.byte(a.ev(args[3]))

    def octave(a, args):
        a.byte(0xD0 + 8 - a.ev(args[0]))

    def note_type(a, args):
        a.byte(0xD8)
        a.byte(a.ev(args[0]))
        if len(args) >= 2:
            a.nib(a.ev(args[1]), _fade(None, a, args[2]))

    def two(code, kind):
        def f(a, args):
            a.byte(code)
            if kind == "b":
                a.byte(a.ev(args[0]))
            elif kind == "fade":
                a.nib(a.ev(args[0]), _fade(None, a, args[1]))
            elif kind == "nib":
                a.nib(a.ev(args[0]), a.ev(args[1]))
            elif kind == "lr":
                a.nib(15 * (a.ev(args[0]) != 0), 15 * (a.ev(args[1]) != 0))
            elif kind == "bigw":
                a.word(args[0], big=True)
            elif kind == "w":
                a.word(args[0])
        return f

    def simple(code):
        return lambda a, args: a.byte(code)

    def duty_cycle_pattern(a, args):
        v = [a.ev(x) for x in args]
        a.byte(0xDE)
        a.byte(v[0] << 6 | v[1] << 4 | v[2] << 2 | v[3])

    def pitch_slide(a, args):
        a.byte(0xE0)
        a.byte(a.ev(args[0]) - 1)
        a.nib(8 - a.ev(args[1]), a.ev(args[2]) % 12)

    def vibrato(a, args):
        a.byte(0xE1)
        a.byte(a.ev(args[0]))
        if len(args) > 2:
            a.nib(a.ev(args[1]), a.ev(args[2]))
        else:
            a.byte(a.ev(args[1]))

    def toggle_noise(code):
        def f(a, args):
            a.byte(code)
            if args:
                a.byte(a.ev(args[0]))
        return f

    def volume(a, args):
        a.byte(0xE5)
        if len(args) > 1:
            a.nib(a.ev(args[0]), a.ev(args[1]))
        else:
            a.byte(a.ev(args[0]))

    def sound_jump_if(a, args):
        a.byte(0xFB)
        a.byte(a.ev(args[0]))
        a.word(args[1])

    def sound_loop(a, args):
        a.byte(0xFD)
        a.byte(a.ev(args[0]))
        a.word(args[1])

    m.update(channel_count=channel_count, channel=channel, note=note, drum_note=note, rest=rest,
             square_note=square_note, noise_note=noise_note, octave=octave, note_type=note_type,
             drum_speed=note_type, transpose=two(0xD9, "nib"), tempo=two(0xDA, "bigw"),
             duty_cycle=two(0xDB, "b"), volume_envelope=two(0xDC, "fade"),
             pitch_sweep=two(0xDD, "fade"), duty_cycle_pattern=duty_cycle_pattern,
             toggle_sfx=simple(0xDF), pitch_slide=pitch_slide, vibrato=vibrato,
             unknownmusic0xe2=two(0xE2, "b"), toggle_noise=toggle_noise(0xE3),
             force_stereo_panning=two(0xE4, "lr"), volume=volume,
             pitch_offset=two(0xE6, "bigw"), unknownmusic0xe7=two(0xE7, "b"),
             unknownmusic0xe8=two(0xE8, "b"), tempo_relative=two(0xE9, "b"),
             restart_channel=two(0xEA, "w"), new_song=two(0xEB, "w"),
             sfx_priority_on=simple(0xEC), sfx_priority_off=simple(0xED),
             unknownmusic0xee=two(0xEE, "w"), stereo_panning=two(0xEF, "lr"),
             sfx_toggle_noise=toggle_noise(0xF0), set_condition=two(0xFA, "b"),
             sound_jump_if=sound_jump_if, sound_jump=two(0xFC, "w"), sound_loop=sound_loop,
             sound_call=two(0xFE, "w"), sound_ret=simple(0xFF))
    return m


NOTE_NAMES = ["C_", "C#", "D_", "D#", "E_", "F_", "F#", "G_", "G#", "A_", "A#", "B_"]


def read_constants(path, start=0):
    """Reads `const NAME` lists (const_def / const_next / const_skip) into a dict."""
    out = {}
    val = 0
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.split(";", 1)[0].strip()
            m = re.match(r"const_def\s*(.*)$", line)
            if m:
                arg = m.group(1).strip()
                val = Expr(arg.split(",")[0], lambda s: out[s]).value() if arg else 0
                continue
            m = re.match(r"const_next\s+(.*)$", line)
            if m:
                val = Expr(m.group(1), lambda s: out[s]).value()
                continue
            m = re.match(r"const_skip\s*(.*)$", line)
            if m:
                val += Expr(m.group(1), lambda s: out[s]).value() if m.group(1).strip() else 1
                continue
            m = re.match(r"const\s+([A-Za-z_][A-Za-z0-9_#]*)", line)
            if m:
                out[m.group(1)] = val
                val += 1
    return out


# ---------------------------------------------------------------------------
# Game Boy APU
# ---------------------------------------------------------------------------

DUTY_TABLE = np.array([[0, 0, 0, 0, 0, 0, 0, 1],
                       [1, 0, 0, 0, 0, 0, 0, 1],
                       [1, 0, 0, 0, 0, 1, 1, 1],
                       [0, 1, 1, 1, 1, 1, 1, 0]], dtype=np.int8)
NOISE_DIVISORS = [8, 16, 32, 48, 64, 80, 96, 112]
READ_MASK = {0x10: 0x80, 0x11: 0x3F, 0x12: 0x00, 0x13: 0xFF, 0x14: 0xBF,
             0x15: 0xFF, 0x16: 0x3F, 0x17: 0x00, 0x18: 0xFF, 0x19: 0xBF,
             0x1A: 0x7F, 0x1B: 0xFF, 0x1C: 0x9F, 0x1D: 0xFF, 0x1E: 0xBF,
             0x1F: 0xFF, 0x20: 0xFF, 0x21: 0x00, 0x22: 0x00, 0x23: 0xBF,
             0x24: 0x00, 0x25: 0x00}


class Channel:
    def __init__(self, kind):
        self.kind = kind            # 'sq1', 'sq2', 'wave', 'noise'
        self.enabled = False
        self.dac = False
        self.length = 0
        self.length_en = False
        self.freq = 0
        self.timer = 0              # cycles until the next waveform step (>= 0)
        self.pos = 0
        self.duty = 0
        # envelope
        self.env_init = 0
        self.env_up = False
        self.env_period = 0
        self.env_timer = 0
        self.vol = 0
        # sweep
        self.sw_period = 0
        self.sw_negate = False
        self.sw_shift = 0
        self.sw_timer = 0
        self.sw_enabled = False
        self.sw_shadow = 0
        self.sw_negate_used = False
        # wave
        self.wave_level = 0
        self.wave_buf = 0
        # noise
        self.lfsr = 0x7FFF
        self.n_shift = 0
        self.n_width7 = False
        self.n_div = 0

    def period(self):
        if self.kind in ("sq1", "sq2"):
            return (2048 - self.freq) * 4
        if self.kind == "wave":
            return (2048 - self.freq) * 2
        if self.n_shift >= 14:
            return None
        return NOISE_DIVISORS[self.n_div] << self.n_shift


class APU:
    def __init__(self, record_freqs=False):
        self.now = 0
        self.regs = [0] * 0x20          # NR10..NR52 shadow, index = addr - 0xFF10
        self.ch = [Channel("sq1"), Channel("sq2"), Channel("wave"), Channel("noise")]
        self.wave_ram = [0] * 16
        self.nr50 = 0
        self.nr51 = 0
        self.power = True
        self.fs_next = FS_PHASE
        self.fs_step = 0
        self.chunks = []                 # per-cycle mono analog output (float32)
        self.freq_log = []               # (cycle, channel index, 11-bit freq) on trigger/sweep
        self.trigger_log = []
        self.record = record_freqs

    # -- register interface -------------------------------------------------
    def read(self, addr):
        r = addr & 0xFF
        if r == 0x26:
            st = sum(1 << i for i, c in enumerate(self.ch) if c.enabled)
            return 0x70 | (0x80 if self.power else 0) | st
        if 0x30 <= r <= 0x3F:
            return self.wave_ram[r - 0x30]
        if r in READ_MASK:
            return self.regs[r - 0x10] | READ_MASK[r]
        return 0xFF

    def write(self, addr, val):
        r = addr & 0xFF
        val &= 0xFF
        if 0x30 <= r <= 0x3F:
            self.wave_ram[r - 0x30] = val
            return
        if r == 0x26:
            on = bool(val & 0x80)
            if not on and self.power:
                for a in range(0x10, 0x26):
                    self.write(0xFF00 | a, 0)
                for c in self.ch:
                    c.enabled = False
            self.power = on
            return
        if not self.power or not (0x10 <= r <= 0x25):
            return
        self.regs[r - 0x10] = val
        if r == 0x24:
            self.nr50 = val
            return
        if r == 0x25:
            self.nr51 = val
            return
        idx = (r - 0x10) // 5
        sub = (r - 0x10) % 5
        c = self.ch[idx]
        if c.kind in ("sq1", "sq2", "noise"):
            if sub == 0 and c.kind == "sq1":
                new_negate = bool(val & 0x08)
                if c.sw_negate and not new_negate and c.sw_negate_used:
                    c.enabled = False
                c.sw_period = (val >> 4) & 7
                c.sw_negate = new_negate
                c.sw_shift = val & 7
            elif sub == 1:
                if c.kind != "noise":
                    c.duty = val >> 6
                c.length = 64 - (val & 63)
            elif sub == 2:
                c.env_init = val >> 4
                c.env_up = bool(val & 8)
                c.env_period = val & 7
                c.dac = (val & 0xF8) != 0
                if not c.dac:
                    c.enabled = False
            elif sub == 3:
                if c.kind == "noise":
                    c.n_shift = val >> 4
                    c.n_width7 = bool(val & 8)
                    c.n_div = val & 7
                else:
                    c.freq = (c.freq & 0x700) | val
            elif sub == 4:
                if c.kind != "noise":
                    c.freq = (c.freq & 0xFF) | ((val & 7) << 8)
                c.length_en = bool(val & 0x40)
                if val & 0x80:
                    self.trigger(idx)
        else:  # wave
            if sub == 0:
                c.dac = bool(val & 0x80)
                if not c.dac:
                    c.enabled = False
            elif sub == 1:
                c.length = 256 - val
            elif sub == 2:
                c.wave_level = (val >> 5) & 3
            elif sub == 3:
                c.freq = (c.freq & 0x700) | val
            elif sub == 4:
                c.freq = (c.freq & 0xFF) | ((val & 7) << 8)
                c.length_en = bool(val & 0x40)
                if val & 0x80:
                    self.trigger(idx)

    def trigger(self, idx):
        c = self.ch[idx]
        c.enabled = c.dac
        if c.length == 0:
            c.length = 256 if c.kind == "wave" else 64
        p = c.period()
        c.timer = p if p is not None else 1 << 30
        if c.kind == "wave":
            c.pos = 0
        else:
            c.vol = c.env_init
            c.env_timer = c.env_period
        if c.kind == "noise":
            c.lfsr = 0x7FFF
        if c.kind == "sq1":
            c.sw_shadow = c.freq
            c.sw_timer = c.sw_period or 8
            c.sw_enabled = c.sw_period != 0 or c.sw_shift != 0
            c.sw_negate_used = False
            if c.sw_shift:
                self._sweep_calc(c)
        if self.record and c.enabled:
            self.trigger_log.append((self.now, idx, c.freq if c.kind != "noise" else None))
            if c.kind in ("sq1", "sq2"):
                self.freq_log.append((self.now, idx, c.freq))

    def _sweep_calc(self, c):
        delta = c.sw_shadow >> c.sw_shift
        if c.sw_negate:
            new = c.sw_shadow - delta
            c.sw_negate_used = True
        else:
            new = c.sw_shadow + delta
        if new > 2047:
            c.enabled = False
        return new

    # -- frame sequencer ----------------------------------------------------
    def _fs_tick(self):
        s = self.fs_step
        if s % 2 == 0:
            for c in self.ch:
                if c.length_en and c.length > 0:
                    c.length -= 1
                    if c.length == 0:
                        c.enabled = False
        if s in (2, 6):
            c = self.ch[0]
            if c.sw_timer > 0:
                c.sw_timer -= 1
            if c.sw_timer == 0:
                c.sw_timer = c.sw_period or 8
                if c.sw_enabled and c.sw_period:
                    new = self._sweep_calc(c)
                    if new <= 2047 and c.sw_shift:
                        c.sw_shadow = new
                        c.freq = new
                        self.regs[0x03] = new & 0xFF
                        self.regs[0x04] = (self.regs[0x04] & ~7) | (new >> 8)
                        if self.record and c.enabled:
                            self.freq_log.append((self.now, 0, new))
                        self._sweep_calc(c)
        if s == 7:
            for c in (self.ch[0], self.ch[1], self.ch[3]):
                if c.env_period:
                    c.env_timer -= 1
                    if c.env_timer <= 0:
                        c.env_timer = c.env_period
                        if c.env_up and c.vol < 15:
                            c.vol += 1
                        elif not c.env_up and c.vol > 0:
                            c.vol -= 1
        self.fs_step = (s + 1) & 7

    # -- waveform generation --------------------------------------------------
    def _steps(self, c, n):
        """Returns (k array or None, K): k[i] = waveform steps taken by cycle i."""
        p = c.period()
        if p is None or c.timer > n - 1:
            if p is not None:
                c.timer -= n
            return None, 0, p
        i = np.arange(n, dtype=np.int64)
        k = np.where(i >= c.timer, (i - c.timer) // p + 1, 0)
        K = (n - 1 - c.timer) // p + 1
        c.timer = c.timer + K * p - n
        return k, K, p

    def _channel_digital(self, idx, n):
        c = self.ch[idx]
        if c.kind in ("sq1", "sq2"):
            k, K, _ = self._steps(c, n)
            if not c.enabled:
                c.pos = (c.pos + K) & 7
                return 0
            if k is None:
                return int(DUTY_TABLE[c.duty][c.pos]) * c.vol
            out = DUTY_TABLE[c.duty][(c.pos + k) & 7].astype(np.float32) * c.vol
            c.pos = (c.pos + K) & 7
            return out
        if c.kind == "wave":
            if not c.enabled:
                return 0
            shift = [4, 0, 1, 2][c.wave_level]
            k, K, _ = self._steps(c, n)
            if k is None:
                return c.wave_buf >> shift
            nib = np.array([(self.wave_ram[j >> 1] >> (0 if j & 1 else 4)) & 0xF for j in range(32)],
                           dtype=np.int16)
            samples = nib[(c.pos + k) & 31]
            out = np.where(k == 0, c.wave_buf, samples).astype(np.int16) >> shift
            c.pos = (c.pos + K) & 31
            c.wave_buf = int(nib[c.pos])
            return out.astype(np.float32)
        # noise
        if not c.enabled:
            return 0
        k, K, _ = self._steps(c, n)
        cur = 0 if c.lfsr & 1 else 1
        if k is None:
            return cur * c.vol
        bits = np.empty(K + 1, dtype=np.int8)
        bits[0] = cur
        lf = c.lfsr
        w7 = c.n_width7
        for j in range(1, K + 1):
            x = (lf ^ (lf >> 1)) & 1
            lf = (lf >> 1) | (x << 14)
            if w7:
                lf = (lf & ~0x40) | (x << 6)
            bits[j] = 0 if lf & 1 else 1
        c.lfsr = lf
        return bits[k].astype(np.float32) * c.vol

    def _render(self, n):
        if n <= 0:
            return
        left = np.zeros(n, dtype=np.float32)
        right = np.zeros(n, dtype=np.float32)
        for idx, c in enumerate(self.ch):
            d = self._channel_digital(idx, n)
            if not c.dac:
                continue
            analog = 1.0 - np.asarray(d, dtype=np.float32) / 7.5
            if self.nr51 & (0x10 << idx):
                left += analog
            if self.nr51 & (0x01 << idx):
                right += analog
        lv = ((self.nr50 >> 4) & 7) + 1
        rv = (self.nr50 & 7) + 1
        # mono: average of the two outputs; full scale = 4 channels at volume 8
        self.chunks.append(((left * lv + right * rv) / (2.0 * 4.0 * 8.0)).astype(np.float32))

    def run_until(self, t):
        while self.now < t:
            nxt = min(t, self.fs_next)
            self._render(nxt - self.now)
            self.now = nxt
            if self.now == self.fs_next:
                self._fs_tick()
                self.fs_next += FS_PERIOD

    def quiet(self):
        """True when no routed channel can change its output any more."""
        for idx, c in enumerate(self.ch):
            if not (self.nr51 & (0x11 << idx)) or not c.dac or not c.enabled:
                continue
            if c.kind == "wave":
                if c.wave_level != 0:
                    return False
                continue
            if c.vol > 0 or (c.env_up and c.env_period and c.vol < 15):
                return False
        return True

    def output(self):
        return np.concatenate(self.chunks) if self.chunks else np.zeros(0, dtype=np.float32)


# ---------------------------------------------------------------------------
# Pokémon Red sound engine (audio/engine_2.asm)
# ---------------------------------------------------------------------------

CHAN1, CHAN2, CHAN3, CHAN4, CHAN5, CHAN6, CHAN7, CHAN8 = range(8)
R_PERFECT_PITCH, R_SOUND_CALL, R_NOISE_OR_SFX, R_VIBRATO_DIR, R_PITCH_SLIDE_ON, \
    R_PITCH_SLIDE_DEC, R_ROTATE_DUTY = (1 << b for b in range(7))
R_EXECUTE_MUSIC = 1
HW_BASE = [0x10, 0x15, 0x1A, 0x1F] * 2
ENABLE_MASK = [0x11, 0x22, 0x44, 0x88] * 2
DISABLE_MASK = [(~m) & 0xFF for m in ENABLE_MASK]


class RedEngine:
    def __init__(self, asm, apu, headers2, pitches, wave_ptrs):
        self.a = asm
        self.apu = apu
        self.headers2 = headers2
        self.pitches = pitches
        self.wave_ptrs = wave_ptrs
        z = lambda: [0] * 8
        self.cmd, self.ret, self.sound_id = z(), z(), z()
        self.flags1, self.flags2 = z(), z()
        self.duty, self.duty_pat = z(), z()
        self.vib_delay, self.vib_ext, self.vib_rate, self.freq_lo, self.vib_reload = z(), z(), z(), z(), z()
        self.note_delay, self.loop, self.speed = [1] * 8, [1] * 8, [1] * 8
        self.frac, self.octave, self.vol = z(), z(), z()
        self.music_wave = self.sfx_wave = 0
        self.music_tempo = self.sfx_tempo = 0x100
        self.stereo = 0xFF
        self.disable_when_ends = 0

    def w(self, reg, val):
        self.apu.write(0xFF00 | reg, val)

    def r(self, reg):
        return self.apu.read(0xFF00 | reg)

    def reg(self, c, r):
        return HW_BASE[c] + r

    def next_byte(self, c):
        v = self.a.rd(self.cmd[c])
        self.cmd[c] += 1
        return v

    # Audio2_PlaySound.stopAllAudio
    def stop_all_audio(self):
        self.w(0x26, 0x80)
        self.w(0x1A, 0x80)
        self.w(0x25, 0x00)
        self.w(0x1C, 0x00)
        self.w(0x10, 0x08)
        self.w(0x12, 0x08)
        self.w(0x17, 0x08)
        self.w(0x21, 0x08)
        self.w(0x14, 0x40)
        self.w(0x19, 0x40)
        self.w(0x23, 0x40)
        self.w(0x24, 0x77)

    # Audio2_PlaySound .playSfx / .playSoundCommon (not a cry, nothing playing)
    def play_sfx(self, sound_id):
        hdr = self.headers2 + 3 * sound_id
        count = ((self.a.rd(hdr) & 0xC0) >> 6) + 1
        for i in reversed(range(count)):
            e = self.a.rd(hdr + 3 * i) & 0xF
            if self.sound_id[e]:
                raise AsmError("channel busy")
            self.ret[e] = self.cmd[e] = self.sound_id[e] = 0
            self.flags1[e] = self.duty[e] = self.duty_pat[e] = 0
            self.vib_delay[e] = self.vib_ext[e] = self.vib_rate[e] = self.freq_lo[e] = 0
            self.vib_reload[e] = self.flags2[e] = 0
            self.loop[e] = self.note_delay[e] = self.speed[e] = 1
            if e == CHAN5:
                self.w(0x10, 0x08)
        for i in range(count):
            e = self.a.rd(hdr + 3 * i) & 0xF
            self.sound_id[e] = sound_id
            if e >= CHAN4:
                self.flags1[e] |= R_NOISE_OR_SFX
            self.cmd[e] = self.a.rdw(hdr + 3 * i + 1)
            if self.cmd[e] == 0:
                raise AsmError("SFX header points at a label that was not assembled")

    def done(self):
        return not any(self.sound_id[CHAN5:])

    # Audio2_UpdateMusic
    def update(self):
        for c in range(8):
            if self.sound_id[c]:
                self.apply_music_affects(c)

    def apply_music_affects(self, c):
        if self.note_delay[c] == 1:
            return self.play_next_note(c)
        self.note_delay[c] = (self.note_delay[c] - 1) & 0xFF
        if c < CHAN5 and self.sound_id[c + 4]:
            return
        if self.flags1[c] & R_ROTATE_DUTY:
            p = self.duty_pat[c]
            p = ((p << 2) | (p >> 6)) & 0xFF
            self.duty_pat[c] = p
            rg = self.reg(c, 1)
            self.w(rg, (self.r(rg) & 0x3F) | (p & 0xC0))
        if not (self.flags2[c] & R_EXECUTE_MUSIC) and (self.flags1[c] & R_NOISE_OR_SFX):
            return
        if self.flags1[c] & R_PITCH_SLIDE_ON:
            raise NotImplementedError("Red pitch slide")
        if self.vib_delay[c]:
            self.vib_delay[c] -= 1
            return
        ext = self.vib_ext[c]
        if not ext:
            return
        rate = self.vib_rate[c]
        if rate & 0xF:
            self.vib_rate[c] = rate - 1
            return
        self.vib_rate[c] = rate | (((rate << 4) | (rate >> 4)) & 0xFF)
        e = self.freq_lo[c]
        if self.flags1[c] & R_VIBRATO_DIR:
            self.flags1[c] &= ~R_VIBRATO_DIR
            a = e - (ext & 0xF)
            a = 0 if a < 0 else a
        else:
            self.flags1[c] |= R_VIBRATO_DIR
            a = e + (ext >> 4)
            a = 0xFF if a > 0xFF else a
        self.w(self.reg(c, 3), a)

    def play_next_note(self, c):
        self.vib_delay[c] = self.vib_reload[c]
        self.flags1[c] &= ~(R_PITCH_SLIDE_ON | R_PITCH_SLIDE_DEC)
        self.parse(c)          # Audio2_sound_ret and the command chain

    def parse(self, c):
        while True:
            d = self.next_byte(c)
            hi = d & 0xF0
            if d == 0xFF:                                   # sound_ret
                if self.flags1[c] & R_SOUND_CALL:
                    self.flags1[c] &= ~R_SOUND_CALL
                    self.cmd[c] = self.ret[c]
                    continue
                if c >= CHAN4:
                    self.flags1[c] &= ~R_NOISE_OR_SFX
                    self.flags2[c] &= ~R_EXECUTE_MUSIC
                    if c == CHAN7:
                        self.w(0x1A, 0x00)
                        self.w(0x1A, 0x80)
                        if self.disable_when_ends:
                            self.disable_when_ends = 0
                            self.w(0x25, self.r(0x25) & DISABLE_MASK[c])
                else:
                    self.w(0x25, self.r(0x25) & DISABLE_MASK[c])
                self.sound_id[c] = 0
                return
            if d == 0xFD:                                   # sound_call
                lo = self.next_byte(c)
                hi8 = self.next_byte(c)
                self.ret[c] = self.cmd[c]
                self.cmd[c] = lo | (hi8 << 8)
                self.flags1[c] |= R_SOUND_CALL
                continue
            if d == 0xFE:                                   # sound_loop
                count = self.next_byte(c)
                if count and self.loop[c] == count:
                    self.loop[c] = 1
                    self.next_byte(c)
                    self.next_byte(c)
                    continue
                if count:
                    self.loop[c] += 1
                lo = self.next_byte(c)
                hi8 = self.next_byte(c)
                self.cmd[c] = lo | (hi8 << 8)
                continue
            if hi == 0xD0:                                  # note_type
                self.speed[c] = d & 0xF
                if c == CHAN4:
                    continue
                p = self.next_byte(c)
                if c in (CHAN3, CHAN7):
                    if c == CHAN3:
                        self.music_wave = p & 0xF
                    else:
                        self.sfx_wave = p & 0xF
                    p = ((p & 0x30) << 1) & 0xFF
                self.vol[c] = p
                continue
            if d == 0xE8:                                   # toggle_perfect_pitch
                self.flags1[c] ^= R_PERFECT_PITCH
                continue
            if d == 0xEA:                                   # vibrato
                delay = self.next_byte(c)
                self.vib_delay[c] = self.vib_reload[c] = delay
                p = self.next_byte(c)
                ext = p >> 4
                self.vib_ext[c] = (((ext >> 1) + (ext & 1)) << 4) | (ext >> 1)
                self.vib_rate[c] = ((p & 0xF) << 4) | (p & 0xF)
                continue
            if d == 0xEB:
                raise NotImplementedError("Red pitch_slide")
            if d == 0xEC:                                   # duty_cycle
                self.duty[c] = (self.next_byte(c) << 6) & 0xC0
                continue
            if d == 0xED:                                   # tempo
                t = (self.next_byte(c) << 8) | self.next_byte(c)
                if c >= CHAN5:
                    self.sfx_tempo = t
                    for i in range(4, 8):
                        self.frac[i] = 0
                else:
                    self.music_tempo = t
                    for i in range(4):
                        self.frac[i] = 0
                continue
            if d == 0xEE:                                   # stereo_panning
                self.stereo = self.next_byte(c)
                continue
            if d == 0xEF:
                raise NotImplementedError("Red unknownmusic0xef")
            if d == 0xFC:                                   # duty_cycle_pattern
                p = self.next_byte(c)
                self.duty_pat[c] = p
                self.duty[c] = p & 0xC0
                self.flags1[c] |= R_ROTATE_DUTY
                continue
            if d == 0xF0:                                   # volume
                self.w(0x24, self.next_byte(c))
                continue
            if d == 0xF8:                                   # execute_music
                self.flags2[c] |= R_EXECUTE_MUSIC
                continue
            if hi == 0xE0:                                  # octave
                self.octave[c] = d & 0xF
                continue
            if hi == 0x20 and c >= CHAN4 and not (self.flags2[c] & R_EXECUTE_MUSIC):
                return self.sfx_note(c, d)                  # square_note / noise_note
            if c >= CHAN5 and d == 0x10 and not (self.flags2[c] & R_EXECUTE_MUSIC):
                self.w(0x10, self.next_byte(c))             # pitch_sweep
                continue
            if c == CHAN4 and hi <= 0xB0:
                raise NotImplementedError("Red music drum notes")
            return self.note_length(c, d)

    def note_length(self, c, d):
        """Audio2_note_length. Returns the delay (the value left in a when the
        sfx path returns early) or None after going on to Audio2_note_pitch."""
        length = (d & 0xF) + 1
        prod = (self.speed[c] * length) & 0xFFFF
        if c >= CHAN5:
            tempo = 0x100
            if c != CHAN8:
                self.sfx_tempo = 0x100          # Audio2_SetSfxTempo: not a cry / battle SFX
                tempo = self.sfx_tempo
        else:
            tempo = self.music_tempo
        hl = (self.frac[c] + (prod & 0xFF) * tempo) & 0xFFFF
        self.frac[c] = hl & 0xFF
        self.note_delay[c] = hl >> 8
        if not (self.flags2[c] & R_EXECUTE_MUSIC) and (self.flags1[c] & R_NOISE_OR_SFX):
            return self.note_delay[c]
        self.note_pitch(c, d)
        return None

    def sfx_note(self, c, d):
        delay = self.note_length(c, d)
        self.w(self.reg(c, 1), self.duty[c] | delay)
        self.w(self.reg(c, 2), self.next_byte(c))
        e = self.next_byte(c)
        dd = 0 if c == CHAN8 else self.next_byte(c)
        self.apply_duty_and_length(c)
        self.enable_output(c)
        self.apply_wave_and_freq(c, dd, e)

    def calc_frequency(self, note, octave):
        de = self.pitches[note]
        a = octave
        while a != 7:
            de = ((de >> 1) | (de & 0x8000)) & 0xFFFF        # sra d / rr e
            a = (a + 1) & 0xFF
        return (((de >> 8) + 8) & 0xFF) << 8 | (de & 0xFF)

    def note_pitch(self, c, d):
        pitch = d >> 4
        if pitch == 0xC:                                    # rest
            if c < CHAN5 and self.sound_id[c + 4]:
                return
            if c in (CHAN3, CHAN7):
                self.w(0x25, self.r(0x25) & DISABLE_MASK[c])
            else:
                self.w(self.reg(c, 2), 0x08)
                self.w(self.reg(c, 4), 0x80)
            return
        de = self.calc_frequency(pitch, self.octave[c])
        if self.flags1[c] & R_PITCH_SLIDE_ON:
            raise NotImplementedError("Red pitch slide")
        if c < CHAN5 and self.sound_id[c + 4]:
            return
        self.w(self.reg(c, 2), self.vol[c])
        self.apply_duty_and_length(c)
        self.enable_output(c)
        e, dd = de & 0xFF, de >> 8
        if self.flags1[c] & R_PERFECT_PITCH:
            e = (e + 1) & 0xFF      # `inc e / jr nc`: inc leaves carry clear, d is never bumped
        self.freq_lo[c] = e
        self.apply_wave_and_freq(c, dd, e)

    def enable_output(self, c):
        d = self.r(0x25) | ENABLE_MASK[c]
        if c == CHAN8 or (c < CHAN5 and not self.sound_id[c + 4]):
            d = (self.r(0x25) & DISABLE_MASK[c]) | (self.stereo & ENABLE_MASK[c])
        self.w(0x25, d)

    def apply_duty_and_length(self, c):
        d = self.note_delay[c]
        if c not in (CHAN3, CHAN7):
            d = (d & 0x3F) | self.duty[c]
        self.w(self.reg(c, 1), d)

    def apply_wave_and_freq(self, c, d, e):
        if c in (CHAN3, CHAN7):
            inst = self.music_wave if c == CHAN3 else self.sfx_wave
            src = self.a.rdw(self.wave_ptrs + 2 * inst)
            self.w(0x1A, 0x00)
            for i in range(16):
                self.w(0x30 + i, self.a.rd(src + i))
            self.w(0x1A, 0x80)
        d = (d | 0x80) & 0xC7
        self.w(self.reg(c, 3), e)
        self.w(self.reg(c, 4), d)
        # Audio2_ApplyFrequencyModifier only acts on cries / battle SFX


# ---------------------------------------------------------------------------
# Pokémon Crystal sound engine (audio/engine.asm)
# ---------------------------------------------------------------------------

C_ON, C_SUBROUTINE, C_LOOPING, C_SFX, C_NOISE, C_CRY = (1 << b for b in range(6))
C2_VIBRATO, C2_PITCH_SLIDE, C2_DUTY_LOOP, C2_UNKN_0B, C2_PITCH_OFFSET = (1 << b for b in range(5))
C3_VIBRATO_DIR, C3_PITCH_SLIDE_DIR = 1, 2
N_DUTY_OVERRIDE, N_FREQ_OVERRIDE, N_UNUSED, N_PITCH_SWEEP, N_NOISE_SAMPLING, N_REST, \
    N_VIBRATO_OVERRIDE = (1 << b for b in range(7))
MONO_TRACKS = [0x11, 0x22, 0x44, 0x88]


class CChan:
    def __init__(self):
        self.init()

    def init(self):
        # ChannelInit: everything 0, tempo $100, note length 1
        self.flags1 = self.flags2 = self.flags3 = 0
        self.addr = self.last_addr = 0
        self.note_flags = 0
        self.duty = self.env = self.freq = 0
        self.pitch = self.octave = self.transpose = 0
        self.duration = self.duration_mod = 0
        self.loop_count = 0
        self.tempo = 0x100
        self.tracks = 0
        self.duty_pattern = 0
        self.vib_delay_count = self.vib_delay = self.vib_extent = self.vib_rate = 0
        self.pitch_offset = 0
        self.note_length = 1


class CrystalEngine:
    def __init__(self, asm, apu, freq_table, wave_samples, drumkits):
        self.a = asm
        self.apu = apu
        self.freq_table = freq_table
        self.wave_samples = wave_samples
        self.drumkits = drumkits
        self.ch = [CChan() for _ in range(8)]
        self.volume = 0x77
        self.sound_output = 0
        self.pitch_sweep = 0
        self.noise_addr = 0
        self.noise_delay = 0
        self.music_noise_set = self.sfx_noise_set = 0
        self.cur = 0
        self.cur_duty = self.cur_env = self.cur_freq = 0
        self.stereo_option = False      # default options: MONO

    def w(self, reg, val):
        self.apu.write(0xFF00 | reg, val)

    def r(self, reg):
        return self.apu.read(0xFF00 | reg)

    def clear_channel(self, base):
        self.w(base, 0)
        self.w(base + 1, 0)
        self.w(base + 2, 0x08)
        self.w(base + 3, 0)
        self.w(base + 4, 0x80)

    def init_sound(self):                              # _InitSound
        self.w(0x24, 0)
        self.w(0x25, 0)
        self.w(0x26, 0x80)
        for base in (0x10, 0x15, 0x1A, 0x1F):
            self.clear_channel(base)
        self.volume = 0x77

    def play_sfx(self, header):                        # _PlaySFX (nothing playing)
        count = ((self.a.rd(header) >> 6) & 3) + 1
        p = header
        for _ in range(count):
            cnum = self.a.rd(p) & 7
            p += 1
            ch = self.ch[cnum]
            ch.init()
            ch.addr = self.a.rdw(p)
            if ch.addr == 0:
                raise AsmError("SFX header points at a label that was not assembled")
            p += 2
            ch.flags1 |= C_SFX
            ch.tracks = MONO_TRACKS[cnum & 3]          # StartChannel / SetLRTracks
            ch.flags1 |= C_ON

    def done(self):
        return not any(self.ch[i].flags1 & C_ON for i in range(4, 8))

    def get_byte(self, ch):
        v = self.a.rd(ch.addr)
        ch.addr += 1
        return v

    # _UpdateSound
    def update(self):
        self.sound_output = 0
        for c in range(8):
            ch = self.ch[c]
            self.cur = c
            if not (ch.flags1 & C_ON):
                continue
            if ch.duration < 2:
                ch.vib_delay_count = ch.vib_delay
                ch.flags2 &= ~C2_PITCH_SLIDE
                self.parse_music(ch)
            else:
                ch.duration -= 1
            if ch.flags2 & C2_PITCH_SLIDE:
                raise NotImplementedError("Crystal pitch slide")
            self.cur_duty, self.cur_env, self.cur_freq = ch.duty, ch.env, ch.freq
            self.handle_vibrato(ch)
            self.handle_noise(ch)
            if c < 4:
                raise NotImplementedError("music channels")
            self.update_channel(ch)
            self.sound_output |= ch.tracks
            ch.note_flags = 0
        self.w(0x24, self.volume)
        self.w(0x25, self.sound_output)

    def parse_music(self, ch):
        while True:
            b = self.get_byte(ch)
            if b == 0xFF and not (ch.flags1 & C_SUBROUTINE):
                # .sound_ret on an sfx channel
                if ch.flags1 & C_CRY:
                    raise NotImplementedError("cries")
                if self.cur == CHAN5:
                    self.w(0x10, 0)
                ch.flags1 &= ~C_ON
                ch.note_flags |= N_REST
                return
            if b >= 0xD0:
                self.command(ch, b)
                continue
            if ch.flags1 & (C_SFX | C_CRY):            # ParseSFXOrCry
                ch.note_flags |= N_NOISE_SAMPLING
                self.set_note_duration(ch, b)
                ch.env = self.get_byte(ch)
                ch.freq = (ch.freq & 0xFF00) | self.get_byte(ch)
                if (self.cur & 3) != CHAN4:
                    ch.freq = (ch.freq & 0xFF) | (self.get_byte(ch) << 8)
                return
            if ch.flags1 & C_NOISE:                    # GetNoiseSample
                if (self.cur & 3) != CHAN4:
                    return
                self.set_note_duration(ch, b & 0xF)
                if self.cur & 4:
                    kitset = self.sfx_noise_set
                else:
                    if self.ch[CHAN8].flags1 & C_ON:
                        return
                    kitset = self.music_noise_set
                kit = self.a.rdw(self.drumkits + 2 * kitset)
                pitch = b >> 4
                if pitch == 0:
                    return
                self.noise_addr = self.a.rdw(kit + 2 * pitch)
                self.noise_delay = 0
                return
            self.set_note_duration(ch, b & 0xF)
            pitch = b >> 4
            if pitch == 0:
                ch.note_flags |= N_REST
                return
            ch.pitch = pitch
            ch.freq = self.get_frequency(ch, ch.octave, pitch)
            ch.note_flags |= N_NOISE_SAMPLING
            return                                     # LoadNote: only with pitch slides

    def get_frequency(self, ch, octave, pitch):
        a = ((ch.transpose >> 4) & 0xF) + octave
        idx = (ch.transpose & 0xF) + pitch
        de = self.a.rdw(self.freq_table + 2 * idx)
        while a < 7:
            de = ((de >> 1) | (de & 0x8000)) & 0xFFFF
            a += 1
        return de & 0x7FF

    def set_note_duration(self, ch, a):
        units = (a + 1) & 0xFF
        low = (ch.note_length * units) & 0xFF
        hl = (ch.duration_mod + low * ch.tempo) & 0xFFFF
        ch.duration_mod = hl & 0xFF
        ch.duration = hl >> 8

    def set_global_tempo(self, tempo):
        chans = range(4, 8) if self.cur >= 4 else range(4)
        for i in chans:
            self.ch[i].tempo = tempo
            self.ch[i].duration_mod = 0

    def command(self, ch, b):
        if b <= 0xD7:                                  # octave
            ch.octave = b & 7
        elif b == 0xD8:                                # note_type
            ch.note_length = self.get_byte(ch)
            if (self.cur & 3) != CHAN4:
                ch.env = self.get_byte(ch)
        elif b == 0xD9:
            ch.transpose = self.get_byte(ch)
        elif b == 0xDA:
            t = self.get_byte(ch) << 8
            t |= self.get_byte(ch)
            self.set_global_tempo(t)
        elif b == 0xDB:
            v = self.get_byte(ch)
            ch.duty = (((v >> 2) | (v << 6)) & 0xFF) & 0xC0
        elif b == 0xDC:
            ch.env = self.get_byte(ch)
        elif b == 0xDD:
            self.pitch_sweep = self.get_byte(ch)
            ch.note_flags |= N_PITCH_SWEEP
        elif b == 0xDE:
            ch.flags2 |= C2_DUTY_LOOP
            v = self.get_byte(ch)
            v = ((v >> 2) | (v << 6)) & 0xFF
            ch.duty_pattern = v
            ch.duty = v & 0xC0
        elif b == 0xDF:
            ch.flags1 ^= C_SFX
        elif b == 0xE0:
            raise NotImplementedError("Crystal pitch_slide")
        elif b == 0xE1:                                # vibrato
            ch.flags2 |= C2_VIBRATO
            ch.flags3 &= ~C3_VIBRATO_DIR
            d = self.get_byte(ch)
            ch.vib_delay = ch.vib_delay_count = d
            p = self.get_byte(ch)
            ext = p >> 4
            ch.vib_extent = (((ext >> 1) + (ext & 1)) << 4) | (ext >> 1)
            ch.vib_rate = ((p & 0xF) << 4) | (p & 0xF)
        elif b == 0xE3:                                # toggle_noise
            if ch.flags1 & C_NOISE:
                ch.flags1 &= ~C_NOISE
            else:
                ch.flags1 |= C_NOISE
                self.music_noise_set = self.get_byte(ch)
        elif b == 0xE4:                                # force_stereo_panning
            ch.tracks = MONO_TRACKS[self.cur & 3] & self.get_byte(ch)
        elif b == 0xE5:                                # volume (no fade running)
            self.volume = self.get_byte(ch)
        elif b == 0xE6:
            ch.flags2 |= C2_PITCH_OFFSET
            hi = self.get_byte(ch)
            ch.pitch_offset = (hi << 8) | self.get_byte(ch)
        elif b == 0xEC or b == 0xED:                   # sfx priority: affects music only
            pass
        elif b == 0xEF:                                # stereo_panning
            v = self.get_byte(ch)
            if self.stereo_option:
                ch.tracks = MONO_TRACKS[self.cur & 3] & v
        elif b == 0xF0:                                # sfx_toggle_noise
            if ch.flags1 & C_NOISE:
                ch.flags1 &= ~C_NOISE
            else:
                ch.flags1 |= C_NOISE
                self.sfx_noise_set = self.get_byte(ch)
        elif 0xF1 <= b <= 0xF8:
            pass
        elif b == 0xFC:                                # sound_jump
            lo = self.get_byte(ch)
            ch.addr = lo | (self.get_byte(ch) << 8)
        elif b == 0xFD:                                # sound_loop
            count = self.get_byte(ch)
            if not (ch.flags1 & C_LOOPING):
                if count == 0:
                    lo = self.get_byte(ch)
                    ch.addr = lo | (self.get_byte(ch) << 8)
                    return
                ch.flags1 |= C_LOOPING
                ch.loop_count = count - 1
            if ch.loop_count == 0:
                ch.flags1 &= ~C_LOOPING
                ch.addr += 2
                return
            ch.loop_count -= 1
            lo = self.get_byte(ch)
            ch.addr = lo | (self.get_byte(ch) << 8)
        elif b == 0xFE:                                # sound_call
            lo = self.get_byte(ch)
            target = lo | (self.get_byte(ch) << 8)
            ch.last_addr = ch.addr
            ch.addr = target
            ch.flags1 |= C_SUBROUTINE
        elif b == 0xFF:                                # sound_ret inside a call
            ch.flags1 &= ~C_SUBROUTINE
            ch.addr = ch.last_addr
        else:
            raise NotImplementedError("Crystal music command $%02x" % b)

    def handle_vibrato(self, ch):                      # HandleTrackVibrato
        if ch.flags2 & C2_DUTY_LOOP:
            p = ch.duty_pattern
            p = ((p << 2) | (p >> 6)) & 0xFF
            ch.duty_pattern = p
            self.cur_duty = p & 0xC0
            ch.note_flags |= N_DUTY_OVERRIDE
        if ch.flags2 & C2_PITCH_OFFSET:
            self.cur_freq = (self.cur_freq + ch.pitch_offset) & 0xFFFF
        if not (ch.flags2 & C2_VIBRATO):
            return
        if ch.vib_delay_count:
            ch.vib_delay_count -= 1
            return
        ext = ch.vib_extent
        if not ext:
            return
        rate = ch.vib_rate
        if rate & 0xF:
            ch.vib_rate = rate - 1
            return
        ch.vib_rate = rate | (((rate << 4) | (rate >> 4)) & 0xFF)
        e = self.cur_freq & 0xFF
        if ch.flags3 & C3_VIBRATO_DIR:
            ch.flags3 &= ~C3_VIBRATO_DIR
            a = e - (ext & 0xF)
            a = 0 if a < 0 else a
        else:
            ch.flags3 |= C3_VIBRATO_DIR
            a = e + (ext >> 4)
            a = 0xFF if a > 0xFF else a
        self.cur_freq = (self.cur_freq & 0xFF00) | a
        ch.note_flags |= N_VIBRATO_OVERRIDE

    def handle_noise(self, ch):                        # HandleNoise / ReadNoiseSample
        if not (ch.flags1 & C_NOISE):
            return
        if not (self.cur & 4):
            c8 = self.ch[CHAN8]
            if (c8.flags1 & C_ON) and (c8.flags1 & C_NOISE):
                return
        if self.noise_delay:
            self.noise_delay -= 1
            return
        addr = self.noise_addr
        if addr == 0:
            return
        b = self.a.rd(addr)
        if b == 0xFF:
            return
        self.noise_delay = (b & 0xF) + 1
        self.cur_env = self.a.rd(addr + 1)
        self.cur_freq = self.a.rd(addr + 2)
        self.noise_addr = addr + 3
        ch.note_flags |= N_NOISE_SAMPLING

    def update_channel(self, ch):                      # UpdateChannels
        f = ch.note_flags
        hw = self.cur & 3
        lo, hi = self.cur_freq & 0xFF, (self.cur_freq >> 8) & 0xFF
        if hw == 0:
            if f & N_PITCH_SWEEP:
                self.w(0x10, self.pitch_sweep)
            if f & N_REST:
                self.w(0x26, self.r(0x26) & 0b10001110)
                self.clear_channel(0x10)
            elif f & N_NOISE_SAMPLING:
                self.w(0x11, self.cur_duty | 0x3F)
                self.w(0x12, self.cur_env)
                self.w(0x13, lo)
                self.w(0x14, hi | 0x80)
            elif f & N_FREQ_OVERRIDE:
                self.w(0x13, lo)
                self.w(0x14, hi)
                if f & N_DUTY_OVERRIDE:
                    self.w(0x11, (self.r(0x11) & 0x3F) | self.cur_duty)
            elif f & N_VIBRATO_OVERRIDE:
                self.w(0x11, (self.r(0x11) & 0x3F) | self.cur_duty)
                self.w(0x13, lo)
            elif f & N_DUTY_OVERRIDE:
                self.w(0x11, (self.r(0x11) & 0x3F) | self.cur_duty)
        elif hw == 1:
            if f & N_REST:
                self.w(0x26, self.r(0x26) & 0b10001101)
                self.clear_channel(0x15)
            elif f & N_NOISE_SAMPLING:
                self.w(0x16, self.cur_duty | 0x3F)
                self.w(0x17, self.cur_env)
                self.w(0x18, lo)
                self.w(0x19, hi | 0x80)
            elif f & N_VIBRATO_OVERRIDE:
                self.w(0x16, (self.r(0x16) & 0x3F) | self.cur_duty)
                self.w(0x18, lo)
            elif f & N_DUTY_OVERRIDE:
                self.w(0x16, (self.r(0x16) & 0x3F) | self.cur_duty)
        elif hw == 2:
            if f & N_REST:
                self.w(0x26, self.r(0x26) & 0b10001011)
                self.clear_channel(0x1A)
            elif f & N_NOISE_SAMPLING:
                self.w(0x1B, 0x3F)
                self.w(0x1A, 0)
                src = self.wave_samples + 16 * (self.cur_env & 0xF)
                for i in range(16):
                    self.w(0x30 + i, self.a.rd(src + i))
                self.w(0x1C, ((self.cur_env & 0xF0) << 1) & 0xFF)
                self.w(0x1A, 0x80)
                self.w(0x1D, lo)
                self.w(0x1E, hi | 0x80)
            elif f & N_VIBRATO_OVERRIDE:
                self.w(0x1D, lo)
        else:
            if f & N_REST:
                self.w(0x26, self.r(0x26) & 0b10000111)
                self.clear_channel(0x1F)
            elif f & N_NOISE_SAMPLING:
                self.w(0x20, 0x3F)
                self.w(0x21, self.cur_env)
                self.w(0x22, lo)
                self.w(0x23, 0x80)


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

def run_sound(engine, apu, start):
    """Drives engine + APU frame by frame until the engine has finished (and
    has run two more frames, so e.g. Crystal's NR51 update after the last
    note lands) and no channel can change its output any more. Returns
    (engine_end_cycle, stop_cycle): stop is the last cycle on which the
    output signal changed."""
    start(engine)
    frame = 0
    engine_end = end_frame = None
    while True:
        t = frame * FRAME_CYCLES
        if t > MAX_SECONDS * CPU_HZ:
            raise RuntimeError("sound did not end within %g s" % MAX_SECONDS)
        apu.run_until(t)
        engine.update()
        if engine_end is None and engine.done():
            engine_end, end_frame = t, frame
        if engine_end is not None and frame >= end_frame + 2 and apu.quiet():
            break
        frame += 1
    apu.run_until(apu.now + int(CPU_HZ * (TAIL_MS + 10) / 1000))
    pc = apu.output()
    changes = np.nonzero(pc[1:] != pc[:-1])[0]
    stop = int(changes[-1]) + 1 if len(changes) else 0
    return engine_end, stop


def lowpass_kernel():
    taps = 255
    fs = OUT_HZ * OVERSAMPLE
    fc = 19500.0 / fs
    n = np.arange(taps) - (taps - 1) / 2
    h = 2 * fc * np.sinc(2 * fc * n) * np.kaiser(taps, 9.0)
    return h / h.sum()


def resample(per_cycle):
    """Exact box integration onto the 4x grid, FIR low-pass, decimate by 4."""
    fs4 = OUT_HZ * OVERSAMPLE
    cum = np.concatenate(([0.0], np.cumsum(per_cycle, dtype=np.float64)))
    n4 = int(len(per_cycle) * fs4 // CPU_HZ)
    edges = np.arange(n4 + 1, dtype=np.float64) * (CPU_HZ / fs4)
    i = np.floor(edges).astype(np.int64)
    fr = edges - i
    i = np.minimum(i, len(per_cycle) - 1)
    integ = cum[i] + fr * per_cycle[i]
    x4 = np.diff(integ) / (CPU_HZ / fs4)
    h = lowpass_kernel()
    y = np.convolve(x4, h, mode="full")[(len(h) - 1) // 2:][:len(x4)]
    return y[::OVERSAMPLE]


def highpass(x):
    k = HPF_CHARGE ** (CPU_HZ / OUT_HZ)
    out = np.empty_like(x)
    cap = 0.0
    for n, v in enumerate(x.tolist()):
        o = v - cap
        cap = v - o * k
        out[n] = o
    return out


def dominant_freq(x, start_ms=0.0, ms=50.0):
    a = int(start_ms * OUT_HZ / 1000)
    seg = x[a:a + int(ms * OUT_HZ / 1000)]
    seg = seg - seg.mean()
    if len(seg) < 16 or not np.any(seg):
        return None, None
    win = seg * np.hanning(len(seg))
    nfft = 1 << 18
    spec = np.abs(np.fft.rfft(win, nfft))
    freqs = np.fft.rfftfreq(nfft, 1.0 / OUT_HZ)
    band = (freqs > 40) & (freqs < 8000)
    f_fft = freqs[band][np.argmax(spec[band])]
    s = np.signbit(seg)
    crossings = np.count_nonzero(s[1:] != s[:-1])
    f_zc = crossings / 2.0 / (len(seg) / OUT_HZ)
    return f_fft, f_zc


def gb_hz(freq11, wave=False):
    return (65536.0 if wave else 131072.0) / (2048 - freq11)


# ---------------------------------------------------------------------------
# Sound list
# ---------------------------------------------------------------------------

SOUNDS = [
    # enum suffix,           array name,          game,  constant
    ("RED_BALL_TOSS",        "Red_BallToss",      "red", "SFX_BALL_TOSS"),
    ("RED_TINK",             "Red_Tink",          "red", "SFX_TINK"),
    ("RED_BALL_POOF",        "Red_BallPoof",      "red", "SFX_BALL_POOF"),
    ("RED_CAUGHT_MON",       "Red_CaughtMon",     "red", "SFX_CAUGHT_MON"),
    ("GSC_THROW_BALL",       "Gsc_ThrowBall",     "gsc", "SFX_THROW_BALL"),
    ("GSC_BALL_POOF",        "Gsc_BallPoof",      "gsc", "SFX_BALL_POOF"),
    ("GSC_CHANGE_DEX_MODE",  "Gsc_ChangeDexMode", "gsc", "SFX_CHANGE_DEX_MODE"),
    ("GSC_BALL_BOUNCE",      "Gsc_BallBounce",    "gsc", "SFX_BALL_BOUNCE"),
    ("GSC_BALL_WOBBLE",      "Gsc_BallWobble",    "gsc", "SFX_BALL_WOBBLE"),
    ("GSC_CAUGHT_MON",       "Gsc_CaughtMon",     "gsc", "SFX_CAUGHT_MON"),
]


RED_SFX_FILES = ["ball_toss.asm", "tink_1.asm", "tink_2.asm", "tink_3.asm", "ball_poof.asm", "caught_mon.asm"]


def git_commit(path):
    try:
        return subprocess.run(["git", "-C", path, "rev-parse", "--short", "HEAD"], capture_output=True,
                              text=True, check=True).stdout.strip()
    except Exception:
        return "unknown"


class RedData:
    def __init__(self, root):
        consts = {n: i for i, n in enumerate(NOTE_NAMES)}
        consts.update(read_constants(os.path.join(root, "constants", "audio_constants.asm")))
        consts.update(read_constants(os.path.join(root, "macros", "scripts", "audio.asm")))  # *_cmd
        for i, n in enumerate(NOTE_NAMES):
            consts[n] = i
        a = Assembler(consts, red_macros())
        a.assemble(os.path.join(root, "audio", "headers", "sfxheaders1.asm"))
        a.assemble(os.path.join(root, "audio", "headers", "sfxheaders2.asm"))
        # only the data files of the sounds rendered here (header entries of other
        # sounds stay unresolved and may not be played)
        for fn in RED_SFX_FILES:
            a.assemble(os.path.join(root, "audio", "sfx", fn))
        a.assemble(os.path.join(root, "audio", "notes.asm"), start_label="Audio2_Pitches")
        a.assemble(os.path.join(root, "audio", "wave_samples.asm"), start_label="Audio2_WavePointers")
        a.resolve()
        self.asm = a
        self.pitches = [a.rdw(a.labels["Audio2_Pitches"] + 2 * i) for i in range(12)]
        self.h1 = a.labels["SFX_Headers_1"]
        self.h2 = a.labels["SFX_Headers_2"]
        # music_const NAME, Label: id = (Label - SFX_Headers_1) / 3
        self.ids = {}
        with open(os.path.join(root, "constants", "music_constants.asm"), encoding="utf-8") as f:
            for line in f:
                m = re.match(r"\s*music_const\s+(\w+)\s*,\s*(\w+)", line.split(";")[0])
                if m and m.group(2) in a.labels:     # (music / bank-3 headers: not assembled)
                    lab = m.group(2)
                    base = self.h1 if a.labels[lab] < self.h2 else self.h2
                    self.ids[m.group(1)] = (a.labels[lab] - base) // 3, lab
        self.addr2label = {v: k for k, v in a.labels.items()}

    def describe(self, const):
        sid, lab = self.ids[const]
        hdr = self.h2 + 3 * sid
        return sid, lab, self.addr2label.get(hdr, "?")

    def render(self, const):
        sid, _, bank2_label = self.describe(const)
        apu = APU(record_freqs=True)
        eng = RedEngine(self.asm, apu, self.h2, self.pitches, self.asm.labels["Audio2_WavePointers"])

        def start(e):
            e.stop_all_audio()
            e.play_sfx(sid)
        res = run_sound(eng, apu, start)
        return apu, res, bank2_label


class CrystalData:
    def __init__(self, root):
        consts = read_constants(os.path.join(root, "constants", "audio_constants.asm"))
        consts.update(read_constants(os.path.join(root, "macros", "scripts", "audio.asm")))  # *_cmd
        for i, n in enumerate(NOTE_NAMES):
            consts[n] = i + 1
        a = Assembler(consts, crystal_macros())
        a.assemble(os.path.join(root, "audio", "sfx.asm"))
        a.assemble(os.path.join(root, "audio", "notes.asm"))
        a.assemble(os.path.join(root, "audio", "wave_samples.asm"))
        a.assemble(os.path.join(root, "audio", "drumkits.asm"))
        a.resolve()
        self.asm = a
        # SFX table order (audio/sfx_pointers.asm) and constants/sfx_constants.asm
        self.table = []
        with open(os.path.join(root, "audio", "sfx_pointers.asm"), encoding="utf-8") as f:
            for line in f:
                m = re.match(r"\s*dba\s+(\w+)", line.split(";")[0])
                if m:
                    self.table.append(m.group(1))
        self.ids = read_constants(os.path.join(root, "constants", "sfx_constants.asm"))

    def describe(self, const):
        sid = self.ids[const]
        return sid, self.table[sid]

    def render(self, const):
        sid, label = self.describe(const)
        apu = APU(record_freqs=True)
        a = self.asm
        eng = CrystalEngine(a, apu, a.labels["FrequencyTable"], a.labels["WaveSamples"], a.labels["Drumkits"])

        def start(e):
            e.init_sound()
            e.play_sfx(a.labels[label])
        res = run_sound(eng, apu, start)
        return apu, res, label


def write_wav(path, samples):
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(OUT_HZ)
        w.writeframes(struct.pack("<%dh" % len(samples), *samples))


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    repo = os.path.normpath(os.path.join(here, "..", ".."))
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--pokered", required=True)
    ap.add_argument("--pokecrystal", required=True)
    ap.add_argument("--out", default=os.path.join(repo, "src", "mp", "mp_legacy_sfx.h"))
    ap.add_argument("--wavdir", default=None)
    ap.add_argument("--peak", type=float, default=0.65,
                    help="peak of the loudest sound, as a fraction of int16 full scale")
    args = ap.parse_args()

    red = RedData(args.pokered)
    gsc = CrystalData(args.pokecrystal)
    rendered = []
    for enum, name, game, const in SOUNDS:
        data = red if game == "red" else gsc
        apu, (eng_end, stop), label = data.render(const)
        mono = resample(apu.output())
        mono = highpass(mono)
        n_stop = int(math.ceil(stop * OUT_HZ / CPU_HZ))
        n_keep = n_stop + int(TAIL_MS * OUT_HZ / 1000)
        mono = mono[:n_keep].copy()
        nf = int(FADE_MS * OUT_HZ / 1000)
        mono[-nf:] *= 0.5 * (1 + np.cos(np.linspace(0, math.pi, nf)))
        if not np.all(np.isfinite(mono)):
            raise RuntimeError("non-finite samples in %s" % name)
        if game == "red":
            sid, lab, l2 = red.describe(const)
            src = "pokered %s = $%02X, bank-2 header %s (sfxheaders2.asm)" % (const, sid, l2)
        else:
            sid, lab = gsc.describe(const)
            src = "pokecrystal %s = $%02X -> %s (sfx_pointers.asm)" % (const, sid, lab)
        rendered.append(dict(enum=enum, name=name, game=game, const=const, src=src, mono=mono,
                             eng_end=eng_end, stop=stop, triggers=apu.trigger_log, freqs=apu.freq_log))

    peak = max(float(np.max(np.abs(r["mono"]))) for r in rendered)
    gain = args.peak * 32767.0 / peak
    for r in rendered:
        v = np.round(r["mono"] * gain)
        if np.max(np.abs(v)) > 32767:
            raise RuntimeError("clipping in %s" % r["name"])
        r["pcm"] = v.astype(np.int16)

    # -- report
    print("gain %.1f (raw set peak %.4f of the 4-channel full scale)" % (gain, peak))
    print("%-20s %-58s %8s %8s %8s %6s" % ("sound", "source", "engine", "sound", "length", "peak"))
    for r in rendered:
        pk = int(np.max(np.abs(r["pcm"].astype(np.int32))))
        print("%-20s %-58s %6.1fms %6.1fms %6.1fms %5.1f%%" % (
            r["enum"], r["src"], r["eng_end"] / CPU_HZ * 1000, r["stop"] / CPU_HZ * 1000,
            len(r["pcm"]) / OUT_HZ * 1000, pk / 327.67))
    print()
    for r in rendered:
        x = r["pcm"].astype(np.float64)
        f_fft, f_zc = dominant_freq(x)
        firsts = {}
        for t, ch, fr in r["triggers"]:
            if ch not in firsts and fr is not None:
                firsts[ch] = fr
        exp = ", ".join("ch%d freq %d -> %.1f Hz" % (ch + 1, fr, gb_hz(fr, ch == 2))
                        for ch, fr in sorted(firsts.items()))
        ch1 = [(t, f) for t, c, f in r["freqs"] if c == 0 and t <= CPU_HZ * 0.05]
        sweep = ""
        if len(ch1) > 1:
            sweep = "; ch1 register over 0-50 ms: " + " ".join("%d" % f for _, f in ch1)
        print("%-20s first 50 ms: FFT %s Hz, zero crossings %s Hz | expected %s%s" % (
            r["enum"], "%.1f" % f_fft if f_fft else "-", "%.1f" % f_zc if f_zc else "-", exp, sweep))

    # -- WAV previews
    if args.wavdir:
        os.makedirs(args.wavdir, exist_ok=True)
        for r in rendered:
            write_wav(os.path.join(args.wavdir, "%s.wav" % r["enum"].lower()), r["pcm"].tolist())

    # -- header
    lines = []
    lines.append("// Moemon Co-op: Game Boy Poke Ball sound effects (Pokemon Red / Crystal).")
    lines.append("// GENERATED by tools/moemon_legacy/sfx_render.py -- do not edit.")
    lines.append("//")
    lines.append("// Rendered from the pret disassemblies' sound data through reimplementations of")
    lines.append("// their sound engines and a Game Boy APU model:")
    lines.append("//   pret/pokered %s (audio/engine_2.asm, audio/headers/sfxheaders2.asm)" %
                 git_commit(args.pokered))
    lines.append("//   pret/pokecrystal %s (audio/engine.asm, audio/sfx_pointers.asm)" %
                 git_commit(args.pokecrystal))
    lines.append("// Mono signed 16-bit PCM at %d Hz, one shared gain (the set peaks at %d%% of full" %
                 (OUT_HZ, round(args.peak * 100)))
    lines.append("// scale) so the Game Boy's relative loudness between the sounds is kept.")
    lines.append("#ifndef GUARD_MP_LEGACY_SFX_H")
    lines.append("#define GUARD_MP_LEGACY_SFX_H")
    lines.append("")
    lines.append("#define LEGACY_SFX_RATE %d" % OUT_HZ)
    lines.append("")
    lines.append("enum {")
    for r in rendered:
        lines.append("    LEGACY_SFX_%s," % r["enum"])
    lines.append("    LEGACY_SFX_COUNT")
    lines.append("};")
    lines.append("")
    for r in rendered:
        n = len(r["pcm"])
        lines.append("// %s: %.0f ms" % (r["src"], n / OUT_HZ * 1000))
        lines.append("#define LEGACY_SFX_LEN_%s %d" % (r["enum"], n))
        lines.append("static const s16 sLegacySfx_%s[LEGACY_SFX_LEN_%s] = {" % (r["name"], r["enum"]))
        vals = r["pcm"].tolist()
        per = 24
        for i in range(0, n, per):
            lines.append(",".join(str(v) for v in vals[i:i + per]) + ",")
        lines.append("};")
        lines.append("")
    lines.append("// channels: the Game Boy channels it plays on (bit 0 = channel 1 ... bit 3 = noise); a sound")
    lines.append("// starting takes those channels from the one playing on them")
    lines.append("static const struct { const s16 *data; u32 length; u8 channels; } sLegacySfx[LEGACY_SFX_COUNT] = {")
    for r in rendered:
        mask = 0
        for _, ch, _ in r["triggers"]:
            mask |= 1 << ch
        lines.append("    [LEGACY_SFX_%s] = { sLegacySfx_%s, LEGACY_SFX_LEN_%s, 0x%X }," % (r["enum"], r["name"], r["enum"], mask))
    lines.append("};")
    lines.append("")
    lines.append("#endif // GUARD_MP_LEGACY_SFX_H")
    text = "\n".join(lines) + "\n"
    with open(args.out, "w", encoding="ascii", newline="\n") as f:
        f.write(text)
    print("\nwrote %s (%d bytes, %d samples)" % (args.out, len(text), sum(len(r["pcm"]) for r in rendered)))


if __name__ == "__main__":
    main()
