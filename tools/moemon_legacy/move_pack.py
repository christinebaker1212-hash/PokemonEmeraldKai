#!/usr/bin/env python3
"""Moemon Kai: GB OPTIONS > MOVES. Packs move_bake.py's recordings into graphics/moemon/legacy/gbmoves.bin.

    python3 tools/moemon_legacy/move_pack.py red.json crystal.json OUT.bin

Layout (little endian; offsets from the start of the game's block unless said):
  "GBMV", u16 version (1), u16 games (2), u32 game block offset[2] (from the file's start; 0: none)
  game block:
    u16 moves (index = move), u16 tiles, u16 pictiles, u16 picmaps, u16 linetables, u16 sounds, u16 cgbpals,
    u8 player box (x, y, w, h), u8 enemy box (x, y, w, h)   (GB pixels: where each side's picture sits)
    u32 anims (u32[moves * 2]: move * 2 + side (0: the player's turn, 1: the foe's), 0 none)
    u32 tiles (16 bytes each, the Game Boy's 2bpp), u32 pictiles (the same), u32 picmaps (u32[n] offsets: u8 cells,
    then s8 cx, s8 cy, u16 src each), u32 linetables (144 s8 each), u32 sounds (u32[n] offsets: u32 length, then
    4-bit IMA ADPCM samples at 21024 Hz, from 0 / index 0), u32 cgbpals (64 u16 each: 8 background palettes then 8 sprite palettes of
    4 colours, GBA order: CRYSTAL only)
  an animation: u16 frames, then each frame:
    u8 flags: 0x80 the frame before again (nothing follows); 0x01 SCX per line (u16 table), 0x02 SCY per line,
              0x04 sounds, 0x08 sprites, 0x10 CGB palettes (u16 index), 0x20 the pictures' own BGPs
    u8 BGP, OBP0, OBP1; SCX (s8 or u16), SCY (s8 or u16); s8 window dx, u8 window dy, u8 window on
    user's picture, target's: u8 kind (0 as it was, 2 gone, 3 a map: u16 map follows)
    [0x10: u16 palettes] [0x20: u8 user's BGP, u8 target's] [0x08: u8 n, n x (u8 x + 8, u8 y + 16, u16 tile | 0x8000 when 8x16, u8 attributes)]
    [0x04: u8 n, n x u16 sound]
"""
import json
import struct
import sys


IMA_STEPS = [7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88,
             97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
             724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660,
             4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818,
             18500, 20350, 22385, 24623, 27086, 29794, 32767]
IMA_INDEX = [-1, -1, -1, -1, 2, 4, 6, 8]


def adpcm(samples):
    """IMA ADPCM, 4 bits a sample, low nibble first (mp_gb_moves.c decodes it)"""
    pred, idx = 0, 0
    out = bytearray()
    nib = []
    for s in samples:
        step = IMA_STEPS[idx]
        diff = s - pred
        code = 0
        if diff < 0:
            code = 8
            diff = -diff
        delta = step >> 3
        if diff >= step:
            code |= 4
            diff -= step
            delta += step
        if diff >= step >> 1:
            code |= 2
            diff -= step >> 1
            delta += step >> 1
        if diff >= step >> 2:
            code |= 1
            delta += step >> 2
        pred = max(-32768, min(32767, pred - delta if code & 8 else pred + delta))
        idx = max(0, min(88, idx + IMA_INDEX[code & 7]))
        nib.append(code)
    if len(nib) % 2:
        nib.append(0)
    for i in range(0, len(nib), 2):
        out.append(nib[i] | (nib[i + 1] << 4))
    return bytes(out)


def s8(v):
    v &= 0xFF
    return v - 256 if v > 127 else v


def lum(c):
    return ((c & 31) * 77 + ((c >> 5) & 31) * 151 + ((c >> 10) & 31) * 28) >> 8


def bgp_like(ref, cur):
    """A CGB palette against the one before the move, as the BGP that would turn one into the other: each colour the
    shade of the reference it's nearest in brightness (0xE4 unchanged, 0x00 white, 0x1B the shades turned over)"""
    v = 0
    for i in range(4):
        j = min(range(4), key=lambda k: abs(lum(cur[i]) - lum(ref[k])))
        v |= j << (2 * i)
    return v


def pack_game(d):
    out = bytearray()
    crystal = d.get('game') == 'crystal'
    moves = max(int(k.split('/')[0]) for k in d['anims']) + 1
    # cries are left out: KAI's own animation still plays the POKeMON's own cry (only its sound effects are silent)
    cries = d.get('cry_ids', [20, 133] if d.get('game') == 'red' else [0xFFFF, 0xFFFF])
    for frames in d['anims'].values():
        for f in frames:
            f['snd'] = [s for s in f.get('snd', []) if not cries[0] <= s[0] <= cries[1] and s[0] != 0xFFFF]
    used = {','.join(str(x) for x in s) for frames in d['anims'].values() for f in frames for s in f['snd']}
    sounds = sorted((k for k in d.get('sounds', {}) if k in used), key=lambda k: tuple(int(x) for x in k.split(',')))
    sound_index = {k: i for i, k in enumerate(sounds)}
    lines = {}
    line_list = []
    pals = {}
    pal_list = []

    def line(v):
        t = tuple(s8(x) for x in v)
        if t not in lines:
            lines[t] = len(line_list)
            line_list.append(t)
        return lines[t]

    anims = {}
    for key, frames in d['anims'].items():
        move, turn = (int(x) for x in key.split('/'))
        if crystal:
            # Crystal's palettes as BGPs: the battle's own (palette 7) and each picture's (the player's 0, the foe's 1)
            ref = d['refpals']
            for f in frames:
                c = f['cgb']
                f['bgp'] = bgp_like(ref[28:32], c[28:32])
                pu, pt = (0, 1) if turn == 0 else (1, 0)
                f['picbgp'] = [bgp_like(ref[pu * 4:pu * 4 + 4], c[pu * 4:pu * 4 + 4]),
                               bgp_like(ref[pt * 4:pt * 4 + 4], c[pt * 4:pt * 4 + 4])]
        a = bytearray(struct.pack('<H', len(frames)))
        prev = None
        for f in frames:
            body = dict(f)
            snd = body.pop('snd', [])
            sig = json.dumps(body, sort_keys=True)
            if sig == prev and not snd:
                a.append(0x80)
                continue
            prev = sig
            flags = 0
            rest = bytearray()
            scx, scy = f['scx'], f['scy']
            if isinstance(scx, list):
                flags |= 0x01
            if isinstance(scy, list):
                flags |= 0x02
            if snd:
                flags |= 0x04
            if f['spr']:
                flags |= 0x08
            pal = f.get('cgb')
            if pal is not None:
                flags |= 0x10
            if 'picbgp' in f:
                flags |= 0x20
            rest += bytes([f['bgp'], f['obp'][0], f['obp'][1]])
            rest += struct.pack('<H', line(scx)) if flags & 1 else struct.pack('<b', s8(scx))
            rest += struct.pack('<H', line(scy)) if flags & 2 else struct.pack('<b', s8(scy))
            win = f.get('win', [0, 0, 1])
            rest += struct.pack('<bBB', max(-128, min(127, win[0])), max(0, min(255, win[1])), win[2])
            for p in f['pic']:
                rest.append(p[0])
                if p[0] == 3:
                    rest += struct.pack('<H', p[1])
            if pal is not None:
                k = tuple(pal)
                if k not in pals:
                    pals[k] = len(pal_list)
                    pal_list.append(k)
                rest += struct.pack('<H', pals[k])
            if 'picbgp' in f:
                rest += bytes(f['picbgp'])
            if f['spr']:
                rest.append(len(f['spr']))
                for x, y, t, attr, h in f['spr']:
                    rest += struct.pack('<BBHB', (x + 8) & 0xFF, (y + 16) & 0xFF, t | (0x8000 if h == 16 else 0), attr)
            if snd:
                rest.append(len(snd))
                for s in snd:
                    rest += struct.pack('<H', sound_index[','.join(str(x) for x in s)])
            a.append(flags)
            a += rest
        anims[move * 2 + turn] = a

    pb, eb = d['boxes']['player'], d['boxes']['enemy']
    head_len = 2 * 7 + 8 + 4 * 7
    out += struct.pack('<7H', moves, len(d['tiles']) * (1 if True else 1), len(d['pictiles']), len(d['picmaps']),
                       0, len(sounds), 0)
    out += bytes(pb) + bytes(eb)
    offs_at = len(out)
    out += bytes(4 * 7)
    offs = []
    # animations
    table_at = len(out)
    out += bytes(4 * moves * 2)
    for i in range(moves * 2):
        if i in anims:
            struct.pack_into('<I', out, table_at + 4 * i, len(out))
            out += anims[i]
            while len(out) % 2:
                out.append(0)
    offs.append(table_at)
    # sprite tiles: 8x16 ones are two tiles in a row (their index is the first's)
    tiles_at = len(out)
    tile_index = []
    for h in d['tiles']:
        b = bytes.fromhex(h)
        out += b
    offs.append(tiles_at)
    # the moves' own picture tiles: shades (64 bytes) to 2bpp
    pt_at = len(out)
    for h in d['pictiles']:
        b = bytes.fromhex(h)
        for r in range(8):
            lo_ = hi_ = 0
            for c in range(8):
                v = b[r * 8 + c]
                lo_ |= (v & 1) << (7 - c)
                hi_ |= ((v >> 1) & 1) << (7 - c)
            out += bytes([lo_, hi_])
    offs.append(pt_at)
    # picture maps
    pm_at = len(out)
    out += bytes(4 * len(d['picmaps']))
    for i, cells in enumerate(d['picmaps']):
        struct.pack_into('<I', out, pm_at + 4 * i, len(out))
        out.append(len(cells))
        for cx, cy, src in cells:
            out += struct.pack('<bbH', cx, cy, src)
    while len(out) % 4:
        out.append(0)
    offs.append(pm_at)
    # line tables (written after the frames named them)
    lt_at = len(out)
    for t in line_list:
        out += struct.pack('<144b', *t)
    offs.append(lt_at)
    struct.pack_into('<H', out, 8, len(line_list))
    # sounds
    while len(out) % 4:
        out.append(0)
    snd_at = len(out)
    out += bytes(4 * len(sounds))
    for i, k in enumerate(sounds):
        struct.pack_into('<I', out, snd_at + 4 * i, len(out))
        b = bytes.fromhex(d['sounds'][k])
        out += struct.pack('<I', len(b)) + adpcm([(v - 128) * 256 for v in b])
        while len(out) % 4:
            out.append(0)
    offs.append(snd_at)
    # CGB palettes
    cp_at = len(out)
    for k in pal_list:
        out += struct.pack('<64H', *k)
    offs.append(cp_at)
    struct.pack_into('<H', out, 12, len(pal_list))
    struct.pack_into('<7I', out, offs_at, *offs)
    return bytes(out)


def main():
    ins, outp = sys.argv[1:-1], sys.argv[-1]
    blocks = []
    for path in ins:
        blocks.append(pack_game(json.load(open(path))) if path != '-' else b'')
    while len(blocks) < 2:
        blocks.append(b'')
    out = bytearray(b'GBMV' + struct.pack('<HH', 1, 2) + bytes(8))
    for i, b in enumerate(blocks):
        while len(out) % 4:
            out.append(0)
        if b:
            struct.pack_into('<I', out, 8 + 4 * i, len(out))
            out += b
    open(outp, 'wb').write(out)
    print('wrote %s: %d bytes' % (outp, len(out)))


if __name__ == '__main__':
    main()
