#!/usr/bin/env python3
"""Moemon Kai: GB OPTIONS > MOVES. Records Pokémon Red's and Crystal's own move animations, frame by frame.

Each game is built from its pret disassembly (no ROM: `make blue_debug` in pokered, `make crystal` in pokecrystal,
with rgbds) and run in PyBoy. The image the emulator loads is patched in memory, never saved anywhere kept:

  Red (pokeblue_debug.gbc: Blue's animations and sounds are Red's): the title screen jumps to the debug build's
  DebugMenu, A picks FIGHT (TestBattle: a level 20 RHYDON against a RHYDON), and the battle is played to its
  menu. JoypadLowSensitivity (which the menu polls) first calls a small routine put in bank 0's free space: when
  a flag byte in WRAM is 1 it plays MoveAnimation (wAnimationID, hWhoseTurn, wAnimationType 0: no after-hit
  shake / blink, KAI draws its own) through Bankswitch and sets the flag to 2.
  Crystal: see bake_crystal.

From the battle menu's saved state every move is played from each side, and every frame is recorded: the
sprites on screen (their tiles collected once), BGP / OBP0 / OBP1 (Crystal: its DMG-style shades and CGB colours),
SCX / SCY on every line (the shakes, Psychic's waves), what happened to the two pictures (as they were; moved by
whole tiles; hidden; drawn as another picture: MINIMIZE, SUBSTITUTE, TRANSFORM...), and the sounds started
(PlaySound's id). The sides are the user's and the target's, not the player's and the foe's.

Output: a JSON file (the packer, move_pack.py, writes the game's binary from it).

    python3 tools/moemon_legacy/move_bake.py red|crystal OUT.json [--moves 1-165] [--refs DIR]
"""
import io
import json
import os
import sys

REFS = os.environ.get('MOEMON_PRET_REFS', os.path.expanduser('~/.cache/moemon'))


def load_sym(path):
    sym = {}
    for line in open(path):
        p = line.split()
        if len(p) == 2 and ':' in p[0]:
            b, a = p[0].split(':')
            sym[p[1]] = (int(b, 16), int(a, 16))
    return sym


def lo(v):
    return (v & 0xFFFF).to_bytes(2, 'little')


class Recorder:
    """What a frame shows, as the game draws it (DMG / CGB registers, OAM, VRAM)."""

    def __init__(self, pb, cgb):
        self.pb = pb
        self.cgb = cgb
        self.tiles = {}       # 16 bytes (or 32 for 8x16) -> index
        self.tile_list = []
        self.pics = {}         # a move's own 8x8 tile drawn over a picture (2-bit shades, 64 bytes)
        self.pic_list = []
        self.maps = {}
        self.map_list = []

    def tile(self, data):
        k = bytes(data)
        if k not in self.tiles:
            self.tiles[k] = len(self.tile_list)
            self.tile_list.append(k.hex())
        return self.tiles[k]

    def picmap(self, cells):
        k = json.dumps(cells)
        if k not in self.maps:
            self.maps[k] = len(self.map_list)
            self.map_list.append(cells)
        return self.maps[k]

    def pic(self, data):
        k = bytes(data)
        if k not in self.pics:
            self.pics[k] = len(self.pic_list)
            self.pic_list.append(k.hex())
        return self.pics[k]

    def vram(self, addr, n, bank=0):
        m = self.pb.memory
        if self.cgb and bank:
            return bytes(m[1, a] for a in range(addr, addr + n))
        return bytes(m[addr:addr + n])

    def bg_image(self, win_at=None):
        """The background's 20x18 tiles as GB shades (0-3) and, on CGB, their palette numbers, without scrolling."""
        import numpy as np
        m = self.pb.memory
        lcdc = m[0xFF40]
        signed = not (lcdc & 0x10)

        def decode(raw):
            d = np.frombuffer(raw, np.uint8).reshape(384, 8, 2)
            bits = np.arange(7, -1, -1)
            return ((d[:, :, 0:1] >> bits) & 1) | (((d[:, :, 1:2] >> bits) & 1) << 1)   # (384, 8, 8)
        t0 = decode(self.vram(0x8000, 0x1800, 0))
        t1 = decode(self.vram(0x8000, 0x1800, 1)) if self.cgb else None

        def layer(base):
            tmap = np.frombuffer(bytes(m[base:base + 32 * 18]), np.uint8).reshape(18, 32)[:, :20].astype(np.int32)
            idx = np.where(tmap < 128, tmap + 256, tmap) if signed else tmap
            if self.cgb:
                attrs = np.frombuffer(self.vram(base, 32 * 18, 1), np.uint8).reshape(18, 32)[:, :20]
                tiles = np.where((attrs & 0x08)[:, :, None, None] != 0, t1[idx], t0[idx])
                tiles = np.where((attrs & 0x20)[:, :, None, None] != 0, tiles[:, :, :, ::-1], tiles)
                tiles = np.where((attrs & 0x40)[:, :, None, None] != 0, tiles[:, :, ::-1, :], tiles)
                pal = np.repeat(np.repeat(attrs & 7, 8, 0), 8, 1)
            else:
                tiles = t0[idx]
                pal = None
            return tiles.transpose(0, 2, 1, 3).reshape(144, 160), pal

        img, pal = layer(0x9C00 if lcdc & 0x08 else 0x9800)
        img = img.copy()
        # the window over it where it shows (Red draws the battle on the window, and copies it to the background
        # for its waves)
        wy, wx = (m[0xFF4A], m[0xFF4B] - 7) if win_at is None else win_at
        if lcdc & 0x20 and wy < 144 and wx < 160:
            w, wpal = layer(0x9C00 if lcdc & 0x40 else 0x9800)
            x0 = max(0, wx)
            img[wy:, x0:] = w[:144 - wy, x0 - wx:160 - wx if wx > 0 else 160][:, :160 - x0]
            if pal is not None:
                pal = pal.copy()
                pal[wy:, x0:] = wpal[:144 - wy, x0 - wx:160 - wx if wx > 0 else 160][:, :160 - x0]
        return img, pal

    def sprites(self):
        m = self.pb.memory
        lcdc = m[0xFF40]
        if not (lcdc & 0x02):
            return []
        tall = bool(lcdc & 0x04)
        out = []
        oam = bytes(m[0xFE00:0xFEA0])
        for i in range(40):
            y, x, t, a = oam[i * 4:i * 4 + 4]
            if y == 0 or y >= 160 or x == 0 or x >= 168:
                continue
            if tall:
                t &= 0xFE
            data = self.vram(0x8000 + t * 16, 32 if tall else 16, (a >> 3) & 1 if self.cgb else 0)
            out.append([x - 8, y - 16, self.tile(data), a, 16 if tall else 8])
        return out

    def lines(self):
        pos = self.pb.screen.tilemap_position_list
        scx = [p[0] for p in pos[:144]]
        scy = [p[1] for p in pos[:144]]
        return scx, scy


def classify(ref, cur, box, other, newtile):
    """A picture's box (x, y, w, h in pixels, on whole tiles) in CUR against the battle before the move (REF): as it
    was (0), gone (2), or a map of cells (3): each 8x8 cell on screen around it that shows one of the picture's own
    tiles (0..48, row by row), or a tile of the move's own (256 + its index, NEWTILE). Cells in the OTHER picture's
    box that still show it are left out."""
    import numpy as np
    x0, y0, w, h = box
    r = ref[y0:y0 + h, x0:x0 + w]
    c = cur[y0:y0 + h, x0:x0 + w]
    if (r == c).all():
        return [0, -1]
    if not c.any():
        return [2, -1]
    tw, th = w // 8, h // 8
    src = {}
    for ty in range(th):
        for tx in range(tw):
            t = r[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8]
            if t.any():
                src.setdefault(t.tobytes(), ty * tw + tx)
    bx, by = x0 // 8, y0 // 8
    cells = []
    for cy in range(max(0, by - th), min(18, by + 2 * th)):
        for cx in range(max(0, bx - tw), min(20, bx + 2 * tw)):
            cc = cur[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8]
            inbox = bx <= cx < bx + tw and by <= cy < by + th
            same = (cc == ref[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8]).all()
            if not inbox and same:
                continue   # (the battle's own, untouched)
            ox0, oy0, ow, oh = other
            if ox0 // 8 <= cx < (ox0 + ow) // 8 and oy0 // 8 <= cy < (oy0 + oh) // 8 and same:
                continue   # (the other picture, as it was)
            if not cc.any():
                continue   # (blank)
            k = src.get(cc.tobytes())
            if k is None:
                if not inbox:
                    continue   # (something of the battle's that isn't the picture's: the text box, the other side)
                k = 256 + newtile(cc)
            cells.append([cx - bx, cy - by, k])
    return [3, cells]


# ---------------------------------------------------------------------------------------------- Red

RED_ENEMY_BOX = (96, 0, 56, 56)     # hlcoord 12, 0: 7x7 tiles
RED_PLAYER_BOX = (8, 40, 56, 56)    # hlcoord 1, 5


def red_rom(refs, sound=False):
    from pyboy import PyBoy
    root = os.path.join(refs, 'pokered')
    sym = load_sym(os.path.join(root, 'pokeblue_debug.sym'))
    A = lambda n: sym[n][1]
    rom = bytearray(open(os.path.join(root, 'pokeblue_debug.gbc'), 'rb').read())
    rom[A('DisplayTitleScreen'):A('DisplayTitleScreen') + 3] = bytes([0xC3]) + lo(A('DebugMenu'))
    flag = 0xC000   # wUnusedMusicByte
    stub_at = 0x3FBA   # bank 0's free space (pokeblue_debug.map)
    # flag 1: MoveAnimation; 3: PlaySound [flag + 1] (wFrequencyModifier / wTempoModifier as set); 2 when done
    head = bytes([0xF5, 0xC5, 0xD5, 0xE5, 0xFA]) + lo(flag) + bytes([0xFE, 1, 0x28, 0, 0xFE, 3, 0x20, 0])
    snd = bytes([0xAF, 0xEA]) + lo(flag)
    snd += bytes([0xFA]) + lo(flag + 1) + bytes([0xCD]) + lo(A('PlaySound')) + bytes([0x3E, 2, 0xEA]) + lo(flag)
    snd += bytes([0x18, 0])   # jr .out
    anim = bytes([0xAF, 0xEA]) + lo(flag) + bytes([0x06, sym['MoveAnimation'][0], 0x21]) + lo(A('MoveAnimation'))
    anim += bytes([0xCD]) + lo(A('Bankswitch')) + bytes([0x3E, 2, 0xEA]) + lo(flag)
    tail = bytes([0xE1, 0xD1, 0xC1, 0xF1, 0xC3]) + lo(A('Joypad'))
    stub = bytearray(head + snd + anim + tail)
    stub[10] = len(snd) + 4              # jr z, .anim (past the second cp / jr)
    stub[14] = len(snd) + len(anim)      # jr nz, .out
    stub[len(head) + len(snd) - 1] = len(anim)   # (the sound's jr .out)
    assert len(stub) <= 0x46, len(stub)
    rom[stub_at:stub_at + len(stub)] = stub
    j = A('JoypadLowSensitivity')
    assert rom[j] == 0xCD and rom[j + 1:j + 3] == lo(A('Joypad'))
    rom[j:j + 3] = bytes([0xCD]) + lo(stub_at)
    path = os.path.join(os.environ.get('TMPDIR', '/tmp'), 'moemon_red_bake.gbc')
    open(path, 'wb').write(rom)
    pb = PyBoy(path, window='null', sound_emulated=sound, cgb=False)
    os.remove(path)
    pb.set_emulation_speed(0)
    for i in range(5000):   # the intro, the debug menu, a turn of TestBattle, to its menu
        if 2000 <= i < 4200 and i % 60 == 0:
            pb.button('a')
        pb.tick()
    return pb, sym, flag


def bake_red(refs, moves):
    pb, sym, flag = red_rom(refs)
    A = lambda n: sym[n][1]
    rec = Recorder(pb, False)
    state = io.BytesIO()
    pb.save_state(state)
    ref_img, _ = rec.bg_image()
    base_win = (pb.memory[0xFF4A], pb.memory[0xFF4B] - 7)   # (the window where the battle keeps it: its moves are shakes)
    sounds = []
    frame = [0]
    pb.hook_register(0, A('PlaySound'), lambda c: sounds.append(
        (frame[0], [pb.register_file.A, pb.memory[A('wFrequencyModifier')], pb.memory[A('wTempoModifier')]])), None)
    anims = {}
    for move in moves:
        for turn in (0, 1):
            state.seek(0)
            pb.load_state(state)
            del sounds[:]
            frame[0] = 0
            pb.memory[A('hWhoseTurn')] = turn
            pb.memory[A('wAnimationID')] = move
            pb.memory[A('wAnimationType')] = 0
            pb.memory[flag] = 1
            frames = []
            boxes = (RED_PLAYER_BOX, RED_ENEMY_BOX) if turn == 0 else (RED_ENEMY_BOX, RED_PLAYER_BOX)
            while pb.memory[flag] != 2 and frame[0] < 1500:
                pb.tick()
                frame[0] += 1
                img, _ = rec.bg_image(base_win)
                scx, scy = rec.lines()
                f = {'bgp': pb.memory[0xFF47], 'obp': [pb.memory[0xFF48], pb.memory[0xFF49]],
                     'scx': scx[0] if len(set(scx)) == 1 else scx, 'scy': scy[0] if len(set(scy)) == 1 else scy,
                     'spr': rec.sprites(), 'pic': [],
                     'win': [pb.memory[0xFF4B] - 7 - base_win[1], pb.memory[0xFF4A] - base_win[0],
                             1 if pb.memory[0xFF40] & 0x20 else 0]}
                for k, box in enumerate(boxes):
                    c = classify(ref_img, img, box, boxes[1 - k], lambda t: rec.pic(t.astype('uint8').tobytes()))
                    if c[0] == 3:
                        c[1] = rec.picmap(c[1])
                    f['pic'].append(c)
                f['snd'] = [s for fr, s in sounds if fr == frame[0] - 1]
                frames.append(f)
            anims['%d/%d' % (move, turn)] = frames
            print('red move %d turn %d: %d frames, sounds %s' % (move, turn, len(frames),
                  [s for _, s in sounds]), file=sys.stderr)
    h = A('SFX_Headers_2')   # (sound ids are the battle bank's header offsets / 3)
    cry_ids = [(A('SFX_Cry00_2') - h) // 3, (A('SFX_Cry25_2') - h) // 3 + 2]
    return {'game': 'red', 'cry_ids': cry_ids, 'boxes': {'player': RED_PLAYER_BOX, 'enemy': RED_ENEMY_BOX}, 'tiles': rec.tile_list,
            'pictiles': rec.pic_list, 'picmaps': rec.map_list, 'anims': anims}


# ---------------------------------------------------------------------------------------------- Crystal

CRY_ENEMY_BOX = (96, 0, 56, 56)     # hlcoord 12, 0: 7x7 tiles
CRY_PLAYER_BOX = (16, 48, 48, 48)   # hlcoord 2, 6: 6x6


def crystal_rom(refs, sound=False):
    """pokecrystal.gbc with the title screen sent into a battle: a CYNDAQUIL (Lv20) against a wild CHIKORITA, the
    party written in by a hook as the title screen starts; JoyTextDelay (which the battle's menu polls) first calls a
    routine in bank 0's free space: flag 1 plays PlayBattleAnim (wFXAnimID, hBattleTurn), 3 PlaySFX
    [param], then sets 2."""
    from pyboy import PyBoy
    root = os.path.join(refs, 'pokecrystal')
    sym = load_sym(os.path.join(root, 'pokecrystal.sym'))
    A = lambda n: sym[n][1]
    rom = bytearray(open(os.path.join(root, 'pokecrystal.gbc'), 'rb').read())
    flag, param = 0xC2CF, 0xC296   # (wUnusedScriptByte, wUnusedBCDNumber)
    stub_at = 0x3FB5               # bank 0's free space (pokecrystal.map)
    tb, ta = sym['_TitleScreen']
    b, a = sym['StartBattle']
    boot = bytes([0x3E, 1, 0xE0, 0x70, 0x3E, b, 0x21]) + lo(a) + bytes([0xCD]) + lo(A('FarCall_hl')) + bytes([0x18, 0xFE])
    head = bytes([0xF5, 0xC5, 0xD5, 0xE5, 0xFA]) + lo(flag) + bytes([0xFE, 1, 0x28, 0, 0xFE, 3, 0x20, 0])
    snd = bytes([0xAF, 0xEA]) + lo(flag) + bytes([0xFA]) + lo(param) + bytes([0x5F, 0x16, 0, 0xCD]) + lo(A('PlaySFX'))
    snd += bytes([0x18, 0])   # jr .done
    pb_, pa_ = sym['PlayBattleAnim']
    anim = bytes([0xAF, 0xEA]) + lo(flag) + bytes([0x3E, pb_, 0x21]) + lo(pa_) + bytes([0xCD]) + lo(A('FarCall_hl'))
    done = bytes([0x3E, 2, 0xEA]) + lo(flag)
    tail = bytes([0xE1, 0xD1, 0xC1, 0xF1, 0xC3]) + lo(A('GetJoypad'))
    stub = bytearray(head + snd + anim + done + tail)
    stub[10] = len(snd) + 4                       # jr z, .anim
    stub[14] = len(snd) + len(anim) + len(done)   # jr nz, .out
    stub[len(head) + len(snd) - 1] = len(anim)    # (the sound's jr .done)
    code = bytes(boot) + bytes(stub)
    assert len(code) <= 0x4B, len(code)
    rom[stub_at:stub_at + len(code)] = code
    anim_stub = stub_at + len(boot)
    rom[tb * 0x4000 + ta - 0x4000:tb * 0x4000 + ta - 0x4000 + 3] = bytes([0xC3]) + lo(stub_at)
    j = A('JoyTextDelay')
    assert rom[j] == 0xCD and rom[j + 1:j + 3] == lo(A('GetJoypad'))
    rom[j:j + 3] = bytes([0xCD]) + lo(anim_stub)
    path = os.path.join(os.environ.get('TMPDIR', '/tmp'), 'moemon_crystal_bake.gbc')
    open(path, 'wb').write(rom)
    pb = PyBoy(path, window='null', sound_emulated=sound, cgb=True)
    os.remove(path)
    pb.set_emulation_speed(0)

    def wr(bank, addr, data):
        for i, v in enumerate(data):
            pb.memory[bank, addr + i] = v

    def setup(ctx):
        mon = bytearray(48)
        mon[0] = 155                                   # CYNDAQUIL
        mon[2:6] = bytes([33, 43, 98, 52])
        mon[6:8] = b'\x01\x00'
        mon[8:11] = (8000).to_bytes(3, 'big')
        mon[21:23] = b'\xaa\xaa'
        mon[23:27] = bytes([35, 30, 30, 25])
        mon[27] = 70
        mon[31] = 20
        mon[34:36] = (60).to_bytes(2, 'big')
        mon[36:38] = (60).to_bytes(2, 'big')
        for k in range(5):
            mon[38 + 2 * k:40 + 2 * k] = (40).to_bytes(2, 'big')
        wr(1, A('wPartyCount'), bytes([1, 155, 0xFF]))
        wr(1, A('wPartyMon1'), mon)
        wr(1, A('wPartyMonNicknames'), bytes([0x82, 0x98, 0x8D, 0x83, 0x80, 0x50]) + bytes(5))
        wr(1, A('wPartyMonOTs'), bytes([0x86, 0x8E, 0x8B, 0x83, 0x50]) + bytes(6))
        wr(1, A('wPlayerName'), bytes([0x86, 0x8E, 0x8B, 0x83, 0x50]))
        wr(1, A('wTempWildMonSpecies'), bytes([152]))  # CHIKORITA
        wr(1, A('wCurPartyLevel'), bytes([20]))
        wr(1, A('wBattleType'), bytes([0]))
        wr(1, A('wBattleMode'), bytes([1]))
        wr(1, A('wOtherTrainerClass'), bytes([0]))
    pb.hook_register(tb, ta, setup, None)
    menu = [0]
    bb, ba = sym['BattleMenu']
    pb.hook_register(bb, ba, lambda c: menu.__setitem__(0, menu[0] + 1), None)
    i = 0
    while not menu[0] and i < 20000:   # the intro (A past it), the battle's start, to its menu
        if i >= 2400 and i % 60 == 0:
            pb.button('a')
        pb.tick()
        i += 1
    for i in range(90):
        pb.tick()
    pb.hook_deregister(bb, ba)
    return pb, sym, flag, param


def cgb_pals(pb, sym):
    """The CGB palettes shown (wBGPals2 / wOBPals2: 8 + 8 palettes of 4 colours)"""
    out = []
    for name in ('wBGPals2', 'wOBPals2'):
        b, a = sym[name]
        raw = bytes(pb.memory[b, x] for x in range(a, a + 64))
        out += [raw[i] | (raw[i + 1] << 8) for i in range(0, 64, 2)]
    return out


def bake_crystal(refs, moves):
    pb, sym, flag, param = crystal_rom(refs)
    A = lambda n: sym[n][1]
    rec = Recorder(pb, True)
    state = io.BytesIO()
    pb.save_state(state)
    ref_img, _ = rec.bg_image()
    base_win = (pb.memory[0xFF4A], pb.memory[0xFF4B] - 7)
    ref_pals = cgb_pals(pb, sym)
    sounds = []
    frame = [0]
    sb, sa = sym['PlayStereoSFX']
    pb.hook_register(sb, sa, lambda c: sounds.append((frame[0], [pb.register_file.E | (pb.register_file.D << 8)])), None)
    cb, ca = sym['_PlayCry']
    pb.hook_register(cb, ca, lambda c: sounds.append((frame[0], [0xFFFF])), None)   # (a cry: KAI's own)
    anims = {}
    for move in moves:
        for turn in (0, 1):
            state.seek(0)
            pb.load_state(state)
            del sounds[:]
            frame[0] = 0
            pb.memory[A('hBattleTurn')] = turn
            pb.memory[A('wFXAnimID')] = move & 0xFF
            pb.memory[A('wFXAnimID') + 1] = move >> 8
            pb.memory[A('wBattleAnimParam')] = 0
            pb.memory[flag] = 1
            frames = []
            boxes = (CRY_PLAYER_BOX, CRY_ENEMY_BOX) if turn == 0 else (CRY_ENEMY_BOX, CRY_PLAYER_BOX)
            while pb.memory[flag] != 2 and frame[0] < 1500:
                pb.tick()
                frame[0] += 1
                img, _ = rec.bg_image(base_win)
                scx, scy = rec.lines()
                f = {'bgp': 0xE4, 'obp': [0xE4, 0xE4],
                     'scx': scx[0] if len(set(scx)) == 1 else scx, 'scy': scy[0] if len(set(scy)) == 1 else scy,
                     'spr': rec.sprites(), 'pic': [],
                     'win': [pb.memory[0xFF4B] - 7 - base_win[1], pb.memory[0xFF4A] - base_win[0],
                             1 if pb.memory[0xFF40] & 0x20 else 0],
                     'cgb': cgb_pals(pb, sym)}
                for k, box in enumerate(boxes):
                    c = classify(ref_img, img, box, boxes[1 - k], lambda t: rec.pic(t.astype('uint8').tobytes()))
                    if c[0] == 3:
                        c[1] = rec.picmap(c[1])
                    f['pic'].append(c)
                f['snd'] = [s for fr, s in sounds if fr == frame[0] - 1]
                frames.append(f)
            anims['%d/%d' % (move, turn)] = frames
            print('crystal move %d turn %d: %d frames, sounds %s' % (move, turn, len(frames),
                  [s for _, s in sounds]), file=sys.stderr)
    return {'game': 'crystal', 'boxes': {'player': CRY_PLAYER_BOX, 'enemy': CRY_ENEMY_BOX}, 'tiles': rec.tile_list,
            'pictiles': rec.pic_list, 'picmaps': rec.map_list, 'anims': anims, 'refpals': ref_pals}


def render_sounds(pb, flag, sym, sounds, sfx_channels, param=None):
    """Each sound (id, frequency, tempo) played alone from the battle's saved state, the music stopped first: the
    emulator's audio from the frame it starts to the one its channels fall quiet, mono, 21024 Hz, 8-bit."""
    import numpy as np
    A = lambda n: sym[n][1]
    state = io.BytesIO()
    pb.save_state(state)
    out = {}
    for key in sounds:
        state.seek(0)
        pb.load_state(state)
        if param is None:   # Red: PlaySound [flag + 1] with its pitch and tempo, the music stopped first (id $FF)
            sid, freq, tempo = key
            pb.memory[flag + 1] = 0xFF
            pb.memory[flag] = 3
            for i in range(3):
                pb.tick()
            pb.memory[A('wFrequencyModifier')] = freq
            pb.memory[A('wTempoModifier')] = tempo
            pb.memory[flag + 1] = sid
        else:               # Crystal: PlaySFX [param], the battle's music channels switched off first
            for c in (1, 2, 3, 4):
                pb.memory[A('wChannel%dFlags1' % c)] &= 0xFE
            for r in (0xFF12, 0xFF17, 0xFF1C, 0xFF21):
                pb.memory[r] = 0
            for i in range(2):
                pb.tick()
            pb.memory[param] = key[0]
        pb.memory[flag] = 3
        chunks = []
        quiet = 0
        started = False
        for i in range(60 * 8):
            pb.tick()
            a = pb.sound.ndarray.astype(np.float32)
            if pb.memory[flag] == 2:
                started = True
            if not started:
                continue
            chunks.append(a)
            if param is None:
                busy = any(pb.memory[A('wChannelSoundIDs') + c] for c in sfx_channels)
            else:
                busy = any(pb.memory[A('wChannel%dFlags1' % (c + 1))] & 1 for c in sfx_channels)
            quiet = 0 if busy else quiet + 1
            if quiet > 6:
                break
        pcm = np.concatenate(chunks)[:, :2].mean(axis=1) if chunks else np.zeros(1)
        rate = pb.sound.sample_rate
        n = int(len(pcm) * 21024 / rate)
        pcm = np.interp(np.arange(n) * rate / 21024, np.arange(len(pcm)), pcm)
        nz = np.nonzero(np.abs(pcm) > 1)[0]
        pcm = pcm[:nz[-1] + 1] if len(nz) else pcm[:1]
        out[key] = pcm
        print('sound %s: %.2f s' % (key, len(pcm) / 21024), file=sys.stderr)
    # one gain for every sound (as loud as the game's own)
    peak = max(float(np.abs(p).max()) for p in out.values()) or 1
    return {','.join(str(x) for x in k): bytes(np.clip(np.round(p * 100 / peak) + 128, 0, 255).astype(np.uint8)).hex()
            for k, p in out.items()}


def main():
    game, out = sys.argv[1], sys.argv[2]
    refs = REFS
    moves = None
    args = sys.argv[3:]
    while args:
        a = args.pop(0)
        if a == '--refs':
            refs = args.pop(0)
        elif a == '--moves':
            lo_, hi_ = args.pop(0).split('-')
            moves = range(int(lo_), int(hi_) + 1)
    if game == 'red':
        data = bake_red(refs, moves or range(1, 166))
        lo_c, hi_c = data['cry_ids']
        keys = sorted({tuple(s) for a in data['anims'].values() for f in a for s in f['snd'] if not lo_c <= s[0] <= hi_c})
        pb, sym, flag = red_rom(refs, sound=True)
        data['sounds'] = render_sounds(pb, flag, sym, keys, (4, 5, 6, 7))
    else:
        data = bake_crystal(refs, moves or range(1, 252))
        keys = sorted({tuple(s) for a in data['anims'].values() for f in a for s in f['snd'] if s[0] != 0xFFFF})
        pb, sym, flag, param = crystal_rom(refs, sound=True)
        data['sounds'] = render_sounds(pb, flag, sym, keys, (4, 5, 6, 7), param)
    json.dump(data, open(out, 'w'))
    print('wrote %s: %d animations, %d tiles, %d picture tiles, %d picture maps' % (
        out, len(data['anims']), len(data['tiles']), len(data['pictiles']), len(data['picmaps'])))


if __name__ == '__main__':
    main()
