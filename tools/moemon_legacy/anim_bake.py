#!/usr/bin/env python3
"""Moemon Co-op: bakes Pokémon Red's and Crystal's POKé BALL throw animations.

The GENESIS / EPOCH BALLS (src/mp/mp_legacy_anim.c) play their own game's
throw: this tool runs each game's animation engine offline, frame by frame,
from the pret disassemblies, and writes what's on screen in every frame for
every outcome into src/mp/mp_legacy_anim_data.h:

  - the sprites (tile, Game Boy screen position, flips, palette),
  - events: sounds (with Crystal's stereo side), hiding / showing the target
    (Red), Crystal's picture-shrink steps (its BG effect draws the Moemon from a
    subset of its own tiles: 7x7, 5x5, 3x3, gone),
  - the tiles themselves (2bpp, unchanged) and the palettes.

Red (pokered af51989): ItemUseBall's 20-frame pause, then TossBallAnimation:
TOSS_ANIM, POOF_ANIM, HIDEPIC_ANIM, SHAKE_ANIM x shakes, POOF_ANIM, SHOWPIC_ANIM
(engine/battle/animations.asm). Each subanimation first loads its 79 tiles 8 per
frame (CopyVideoData: 10 frames); a frame block stays up for its delay (+1 frame
when the OAM is cleaned after it); the special effects play SFX_BALL_TOSS after
the first toss block, SFX_TINK + a 40-frame pause at each shake, SFX_BALL_POOF
after the second poof block.

Crystal (pokecrystal 97187cd): PokeBallEffect's 20-frame pause, PlayBattleAnim's
setup frames, then BattleAnim_ThrowPokeBall run by RunBattleAnimScript: each frame
the script (anim_wait N resumes N+1 frames later), the BG effects, then every
object's function and frameset (engine/battle_anims/*.asm). The wobbles come from
anim_checkpokeball, fed with the outcome already decided (pokeball_wobble.asm).

The outcome is decided before the animation (mp_legacy_balls.c); the timelines
only show it. Coordinates stay the Game Boy's (OAM - 8, - 16); the game maps them
onto the Kai battlers.

usage: tools/moemon_legacy/anim_bake.py [REFS_DIR]
  REFS_DIR holds pokered/ and pokecrystal/ checkouts (default: $MOEMON_PRET_REFS)
"""
import os
import re
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))

# events (keep in sync with mp_legacy_anim.c)
EV_SOUND, EV_HIDE_TARGET, EV_SHOW_TARGET, EV_TARGET_SIZE = 1, 2, 3, 4
# sounds (the enum of src/mp/mp_legacy_sfx.h)
SFX = {"RED_BALL_TOSS": 0, "RED_TINK": 1, "RED_BALL_POOF": 2, "RED_CAUGHT_MON": 3,
       "GSC_THROW_BALL": 4, "GSC_BALL_POOF": 5, "GSC_CHANGE_DEX_MODE": 6, "GSC_BALL_BOUNCE": 7,
       "GSC_BALL_WOBBLE": 8, "GSC_CAUGHT_MON": 9}
PAN_CENTER, PAN_LEFT, PAN_RIGHT = 0, 1, 2

XFLIP, YFLIP = 0x20, 0x40


def num(s):
    s = s.strip()
    if s.startswith("$"):
        return int(s[1:], 16)
    if s.startswith("%"):
        return int(s[1:], 2)
    return int(s, 0)


def lines_of(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return [l.split(";")[0].rstrip() for l in f.read().split("\n")]


def block(lines, label):
    """The lines after `label:` up to the next label (local labels count)."""
    out, on = [], False
    for l in lines:
        m = re.match(r"^\s*(\.?[A-Za-z_][\w.]*)::?\s*$", l)
        if m:
            if on:
                break
            on = m.group(1) == label
            continue
        if on:
            out.append(l.strip())
    if not out and not on:
        raise SystemExit("label %s not found" % label)
    return out


def gb_tiles(png):
    """2bpp tiles of a pret PNG (white = color 0 .. black = color 3), row-major."""
    im = Image.open(png).convert("L")
    w, h = im.size
    tiles = []
    for ty in range(h // 8):
        for tx in range(w // 8):
            t = []
            for y in range(8):
                for x in range(8):
                    v = im.getpixel((tx * 8 + x, ty * 8 + y))
                    t.append(3 - min(3, (v + 42) // 85))
            tiles.append(t)
    return tiles


def dbsprite(args, consts):
    """pret's dbsprite: x tile, y tile, x px, y px, tile, attributes -> (y, x, tile, flags) bytes"""
    xt, yt, xp, yp = (num(a) for a in args[:4])
    tile = num(args[4])
    flags = 0
    for part in re.split(r"\s*\|\s*", args[5].strip()):
        flags |= consts.get(part, None) if part in consts else num(part)
    return ((yt * 8) % 256 + yp) & 0xFF, ((xt * 8) % 256 + xp) & 0xFF, tile, flags


OAM_CONSTS = {"OAM_XFLIP": XFLIP, "OAM_YFLIP": YFLIP, "OAM_PAL1": 0x10, "OAM_PRIO": 0x80, "0": 0, "$0": 0}


class Timeline:
    def __init__(self):
        self.frames = []    # list of sprite lists: (x, y, tile, flags, pal) in screen pixels
        self.events = []    # (frame, type, arg, arg2)

    def show(self, sprites):
        self.frames.append(list(sprites))

    def event(self, t, a=0, b=0):
        self.events.append((len(self.frames), t, a, b))


# ---------------------------------------------------------------------------
# Red

class Red:
    def __init__(self, refs):
        d = os.path.join(refs, "pokered")
        fb = lines_of(os.path.join(d, "data/battle_anims/frame_blocks.asm"))
        self.frame_blocks = {}
        cur = None
        for l in fb:
            m = re.match(r"^FrameBlock([0-9a-fA-F]{2}):", l)
            if m:
                cur = int(m.group(1), 16)
                self.frame_blocks[cur] = []
                continue
            m = re.match(r"^\s*dbsprite\s+(.*)$", l)
            if m and cur is not None:
                self.frame_blocks[cur].append(dbsprite([a.strip() for a in m.group(1).split(",")], OAM_CONSTS))
        self.base = []
        for l in lines_of(os.path.join(d, "data/battle_anims/base_coords.asm")):
            m = re.match(r"^\s*db\s+(\$?\w+),\s*(\$?\w+)", l)
            if m:
                self.base.append((num(m.group(1)), num(m.group(2))))
        self.subanims = {}
        sl = lines_of(os.path.join(d, "data/battle_anims/subanimations.asm"))
        for name in ("Subanim_0BallTossHigh", "Subanim_0BallShakeEnemy", "Subanim_0BallPoofEnemy"):
            entries = []
            for l in block(sl, name):
                m = re.match(r"db\s+FRAMEBLOCK_(\w+),\s*BASECOORD_(\w+),\s*FRAMEBLOCKMODE_(\w+)", l)
                if m:
                    entries.append((int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16)))
            self.subanims[name] = entries
        self.tiles = gb_tiles(os.path.join(d, "gfx/battle/move_anim_0.png"))
        # sanity: what the trace below relies on
        assert len(self.base) > 0xA9 and self.base[0x21] == (0x38, 0x78), self.base[0x21]
        assert len(self.subanims["Subanim_0BallTossHigh"]) == 11
        assert len(self.subanims["Subanim_0BallShakeEnemy"]) == 4
        assert len(self.subanims["Subanim_0BallPoofEnemy"]) == 6

    def bake(self, outcome):
        """outcome: 'miss', 1, 2, 3 (shakes, failed) or 'caught'"""
        tl = Timeline()
        oam = []   # shadow OAM: list of (y, x, tile, flags); y = 0 hides an entry

        def visible():
            return [(x - 8, y - 16, tile, flags, 0) for (y, x, tile, flags) in oam if y != 0]

        def delay(n):
            for _ in range(n):
                tl.show(visible())

        def clear():
            for i in range(len(oam)):
                oam[i] = (0, 0, 0, 0)

        def play(name, delay_frames, special):
            entries = self.subanims[name]
            delay(10)   # LoadMoveAnimationTiles: 79 tiles, 8 a frame (CopyVideoData)
            counter = len(entries)
            dest = 0
            i = 0
            state = {"restart": False}
            while True:
                fb, bc, mode = entries[i]
                by, bx = self.base[bc]
                # DrawFrameBlock (SUBANIMTYPE_NORMAL: the player's turn never transforms)
                at = dest
                for (y, x, tile, flags) in self.frame_blocks[fb]:
                    e = ((by + y) & 0xFF, (bx + x) & 0xFF, tile, flags)
                    if at < len(oam):
                        oam[at] = e
                    else:
                        oam.append(e)
                    at += 1
                if mode != 0x02:
                    delay(delay_frames)
                    if mode == 0x03:
                        dest = at
                    elif mode == 0x04:
                        pass
                    else:
                        delay(1)       # AnimationCleanOAM: DelayFrame, ClearSprites
                        clear()
                        dest = 0
                else:
                    dest = at
                # DoSpecialEffectByAnimationId
                special(counter, state)
                if state["restart"]:
                    state["restart"] = False
                    counter = 5
                    i = -1
                counter -= 1
                if counter == 0:
                    return
                i += 1

        if outcome == "poof":
            # PlayMoveAnimation POOF_ANIM before a Moemon appears (SendOutMon): the tile load, 6 blocks
            def poof_only_fx(counter, state):
                if counter == 5:
                    tl.event(EV_SOUND, SFX["RED_BALL_POOF"], PAN_CENTER)
            play("Subanim_0BallPoofEnemy", 4, poof_only_fx)
            return tl, visible()

        shakes = 3 if outcome == "caught" else (0 if outcome == "miss" else outcome)
        num_shakes = {"n": shakes}

        def toss_fx(counter, state):
            if counter == 11:
                tl.event(EV_SOUND, SFX["RED_BALL_TOSS"], PAN_CENTER)

        def poof_fx(counter, state):
            if counter == 5:
                tl.event(EV_SOUND, SFX["RED_BALL_POOF"], PAN_CENTER)

        def shake_fx(counter, state):
            if counter == 4:
                tl.event(EV_SOUND, SFX["RED_TINK"], PAN_CENTER)
                delay(40)
            if counter - 1 == 0:
                num_shakes["n"] -= 1
                if num_shakes["n"] != 0:
                    state["restart"] = True

        delay(20)   # ItemUseBall: DelayFrames 20 before the animation
        play("Subanim_0BallTossHigh", 3, toss_fx)
        play("Subanim_0BallPoofEnemy", 4, poof_fx)
        if outcome != "miss":
            tl.event(EV_HIDE_TARGET)                 # SE_HIDE_ENEMY_MON_PIC, then Delay3
            delay(3)
            play("Subanim_0BallShakeEnemy", 4, shake_fx)
            if outcome != "caught":
                play("Subanim_0BallPoofEnemy", 4, poof_fx)
                tl.event(EV_SHOW_TARGET)             # SE_SHOW_ENEMY_MON_PIC, then Delay3
                delay(3)
        return tl, visible()


# ---------------------------------------------------------------------------
# Crystal

class Crystal:
    PAL_GRAY, PAL_RED = 0, 1

    def __init__(self, refs):
        d = os.path.join(refs, "pokecrystal")
        consts = dict(OAM_CONSTS)
        oam = lines_of(os.path.join(d, "data/battle_anims/oam.asm"))
        self.oamsets = []
        for l in block(oam, "BattleAnimOAMData"):
            m = re.match(r"battleanimoam\s+(\$?\w+),\s*(\d+),\s*(\.\w+)", l)
            if m:
                self.oamsets.append((num(m.group(1)), int(m.group(2)), m.group(3)))
        self.oamdata = {}
        for (_, _, label) in self.oamsets[:0x0E]:
            if label in self.oamdata:
                continue
            ents = []
            for l in block(oam, label):
                m = re.match(r"dbsprite\s+(.*)$", l)
                if m:
                    ents.append(dbsprite([a.strip() for a in m.group(1).split(",")], consts))
            self.oamdata[label] = ents
        fs = lines_of(os.path.join(d, "data/battle_anims/framesets.asm"))
        self.framesets = {}
        for name in ("PokeBall1", "PokeBall2", "PokeBall3", "PokeBall4", "PokeBall5", "BallPoof"):
            frames = []
            for l in block(fs, ".Frameset_" + name):
                m = re.match(r"oamframe\s+BATTLE_ANIM_OAMSET_(\w+),\s*(\d+)(?:,\s*(.*))?$", l)
                if m:
                    flags = 0
                    if m.group(3):
                        for f in m.group(3).split(","):
                            f = f.strip()
                            flags |= {"B_OAM_XFLIP": XFLIP, "B_OAM_YFLIP": YFLIP}[f]
                    frames.append(("frame", int(m.group(1), 16), int(m.group(2)), flags))
                elif l.startswith("oamrestart"):
                    frames.append(("restart",))
                elif l.startswith("oamend"):
                    frames.append(("end",))
                elif l.startswith("oamdelete"):
                    frames.append(("delete",))
            self.framesets[name] = frames
        # sine table: dw sin(x * 0.5 / 32) for x in 0..31; rgbasm's sin() takes turns and pokecrystal
        # builds with -Q8 (8 fractional bits), so the peak is $100
        import math
        self.sine = [int(round(math.sin(x * 0.5 / 32 * 2 * math.pi) * 256)) & 0xFFFF for x in range(32)]
        # pokeball.2bpp is built with tools/gfx --remove-xflip --keep-whitespace (Makefile): a tile
        # whose mirror image is already in goes, blank ones stay (tools/gfx.c remove_flip)
        kept = []
        for t in gb_tiles(os.path.join(d, "gfx/battle_anims/pokeball.png")):
            mirror = [t[y * 8 + (7 - x)] for y in range(8) for x in range(8)]
            if any(t) and mirror in kept:
                continue
            kept.append(t)
        assert len(kept) == 10, len(kept)   # AnimObjGFX: 10 tiles for BATTLE_ANIM_GFX_POKE_BALL
        self.tiles = kept + gb_tiles(os.path.join(d, "gfx/battle_anims/smoke.png"))[:12]
        pals = []
        for l in lines_of(os.path.join(d, "gfx/battle_anims/battle_anims.pal")):
            m = re.match(r"^\s*RGB\s+(\d+),\s*(\d+),\s*(\d+)", l)
            if m:
                pals.append(tuple(int(v) for v in m.groups()))
        self.palettes = [pals[0:4], pals[8:12]]   # gray, red (PAL_BATTLE_OB_GRAY / RED)
        assert self.palettes[1][2] == (30, 10, 6), self.palettes

    def sine_fn(self, a, d):
        """BattleAnim_Sine: a = d * sin(a * pi/32) (calc_sine_wave)"""
        a &= 0x3F
        neg = a >= 0x20
        e = a & 0x1F
        de = self.sine[e]
        hl = 0
        while True:   # .multiply: shift-and-add over the amplitude's bits
            carry = d & 1
            d >>= 1
            if carry:
                hl = (hl + de) & 0xFFFF
            de = (de << 1) & 0xFFFF
            if d == 0:
                break
        h = hl >> 8
        return ((-h) & 0xFF) if neg else h

    def bake(self, outcome):
        """outcome: 0-3 (wobbles, failed) or 'caught'"""
        tl = Timeline()
        S = self
        objs = []          # struct order (slots); each a dict
        last_index = [0]
        bg = {"active": False, "kind": None, "param": 0, "jt": 0}
        caught = outcome == "caught"
        wobbles = 3 if caught else outcome
        checks = [0]

        def new_obj(kind, x, y, param):
            last_index[0] += 1
            o = {"index": last_index[0], "kind": kind, "x": x, "y": y, "xoff": 0, "yoff": 0,
                 "param": param, "duration": 0, "frame": -1, "jt": 0, "var1": 0, "var2": 0,
                 "frameset": "PokeBall1" if kind == "ball" else "BallPoof",
                 "pal": S.PAL_RED if kind == "ball" else S.PAL_GRAY,
                 "tile": 0 if kind == "ball" else 10, "alive": True}
            for i, s in enumerate(objs):
                if not s["alive"]:
                    objs[i] = o
                    return o
            objs.append(o)
            return o

        def find(index):
            for o in objs:
                if o["alive"] and o["index"] == index:
                    return o
            return None

        def reinit(o, fs):
            o["frameset"], o["duration"], o["frame"] = fs, 0, -1

        def sine(a, d):
            return S.sine_fn(a & 0xFF, d & 0xFF)

        def s8(v):
            v &= 0xFF
            return v - 256 if v >= 128 else v

        def ball_fn(o):
            jt = o["jt"]
            if jt == 0:                      # GetBallAnimPal: POKé BALL -> red
                o["pal"] = S.PAL_RED
                o["jt"] += 1
            elif jt == 1:                    # ThrowFromUserToTarget
                if o["x"] >= 0x88:
                    o["y"] = (o["y"] + o["yoff"]) & 0xFF
                    reinit(o, "PokeBall3")
                    o["jt"] += 1
                    return
                o["x"] = (o["x"] + 2) & 0xFF
                o["y"] = (o["y"] - 1) & 0xFF
                a = o["var1"]
                o["var1"] = (o["var1"] - 1) & 0xFF
                o["yoff"] = sine(a, o["param"])
            elif jt in (2, 5, 9):
                return
            elif jt in (3, 4):
                if jt == 3:
                    o["jt"] += 1
                    reinit(o, "PokeBall1")
                    o["var1"], o["var2"] = 0, 0x10
                o["yoff"] = sine(o["var1"], o["var2"])
                a = (o["var1"] - 1) & 0xFF
                o["var1"] = a
                if a & 0x1F:
                    return
                o["var1"] = a & 0x1F         # ld [hl], a: the masked value (0)
                o["var2"] = (o["var2"] - 4) & 0xFF
                if o["var2"]:
                    return
                reinit(o, "PokeBall4")
                o["jt"] += 1
            elif jt == 6:
                reinit(o, "PokeBall5")
                o["jt"] -= 1
            elif jt in (7, 8, 10):
                if jt == 7:
                    o["pal"] = S.PAL_RED
                    reinit(o, "PokeBall2")
                    o["jt"] += 1
                    o["var2"] = 0x20
                o["yoff"] = sine(o["var1"], o["var2"])
                a = (o["var1"] - 1) & 0xFF
                o["var1"] = a
                if (a & 0x1F) == 0:
                    o["alive"] = False       # .eleven
                    return
                if a & 0xF:
                    return
                o["jt"] += 1
            elif jt == 11:
                o["alive"] = False

        def null_fn(o):
            if o["jt"] == 1:
                o["alive"] = False

        def oam_update(o, sprites):
            # (an object its function just deinitialised still draws this frame:
            # BattleAnim_UpdateOAM_All runs BattleAnimOAMUpdate right after DoBattleAnimFrame)
            fs = S.framesets[o["frameset"]]
            while True:
                if o["duration"]:
                    o["duration"] -= 1
                    entry = fs[o["frame"]]
                    break
                o["frame"] += 1
                entry = fs[o["frame"]]
                if entry[0] == "restart":
                    o["duration"], o["frame"] = 0, -1
                    continue
                if entry[0] == "end":
                    o["duration"] = 0
                    o["frame"] -= 2
                    continue
                if entry[0] == "delete":
                    o["alive"] = False
                    o["dead"] = True
                    return
                o["duration"] = entry[2]
                break
            _, oamset, _, fflags = entry
            vtile, length, label = S.oamsets[oamset]
            x0 = (o["x"] + o["xoff"]) & 0xFF
            y0 = (o["y"] + o["yoff"]) & 0xFF
            for (ey, ex, etile, eflags) in S.oamdata[label][:length]:
                yy = ey
                if fflags & YFLIP:
                    yy = (-(ey + 8)) & 0xFF
                xx = ex
                if fflags & XFLIP:
                    xx = (-(ex + 8)) & 0xFF
                y = (y0 + yy) & 0xFF
                x = (x0 + xx) & 0xFF
                flags = (eflags ^ fflags) & (XFLIP | YFLIP)
                sprites.append((x - 8, y - 16, o["tile"] + vtile + etile, flags, o["pal"]))

        def bg_effects():
            if not bg["active"]:
                return
            script = {"return": [(3, 0x00, 3), (-2, 0x77, 3), (4, 0x00, 4), (-2, 0x55, 4), (5, 0x00, 5),
                                 (-2, 0x33, 5), (-3, 0, 0), (-1, 0, 0)],
                      "enter": [(5, 0x00, 5), (4, 0x00, 4), (3, 0x00, 3), (-1, 0, 0)]}[bg["kind"]]
            sizes = {3: 7, 4: 5, 5: 3}
            if bg["jt"] == 0:
                while True:
                    e = script[bg["param"]]
                    bg["param"] += 1
                    if e[0] == -1:
                        bg["active"] = False
                        return
                    if e[0] == -2:
                        tl.event(EV_TARGET_SIZE, 0)
                        continue
                    if e[0] != -3:
                        tl.event(EV_TARGET_SIZE, sizes[e[0]])
                    break
                bg["jt"] = 1
            elif bg["jt"] in (1, 2):
                bg["jt"] += 1
            elif bg["jt"] == 3:
                bg["jt"] = 0

        # the script (BattleAnim_ThrowPokeBall, a POKé BALL: the "any other ball" path)
        script = [
            ("sound", "GSC_THROW_BALL", 2), ("obj", "ball", 68, 92, 0x40), ("wait", 36),
            ("obj", "ball", 136, 65, 0x00), ("setobj", 2, 7), ("wait", 16),
            ("sound", "GSC_BALL_POOF", 1), ("obj", "poof", 136, 64, 0x10), ("wait", 16),
            # .Shake
            ("bgeffect", "return"), ("wait", 8), ("incobj", 2), ("wait", 16),
            ("sound", "GSC_CHANGE_DEX_MODE", 1), ("incobj", 1), ("wait", 32),
            ("sound", "GSC_BALL_BOUNCE", 1), ("wait", 32), ("wait", 32), ("wait", 32), ("wait", 8),
            ("loop",),
        ]
        loop = [("wait", 48), ("check",)]
        breakfree = [("setobj", 1, 0x0B), ("sound", "GSC_BALL_POOF", 1), ("obj", "poof", 136, 64, 0x10),
                     ("wait", 2), ("bgeffect", "enter"), ("wait", 32), ("ret",)]
        pc = {"list": script, "i": 0, "delay": 0, "done": False, "keep": False}

        def run_script():
            if pc["delay"]:
                pc["delay"] -= 1
                return
            while True:
                cmd = pc["list"][pc["i"]]
                pc["i"] += 1
                op = cmd[0]
                if op == "wait":
                    pc["delay"] = cmd[1]
                    return
                if op == "sound":
                    # anim_sound duration, tracks, id: tracks 2 -> $f0 (left), 1 -> $0f (right) on the player's turn
                    tl.event(EV_SOUND, SFX[cmd[1]], PAN_LEFT if cmd[2] in (0, 2) else PAN_RIGHT)
                elif op == "obj":
                    new_obj(cmd[1], cmd[2], cmd[3], cmd[4])
                elif op == "setobj":
                    o = find(cmd[1])
                    if o:
                        o["jt"] = cmd[2]
                elif op == "incobj":
                    o = find(cmd[1])
                    if o:
                        o["jt"] += 1
                elif op == "bgeffect":
                    bg.update(active=True, kind=cmd[1], param=0, jt=0)
                elif op == "loop":
                    pc["list"], pc["i"] = loop, 0
                elif op == "check":
                    # GetPokeBallWobble with the outcome already decided
                    checks[0] += 1
                    n = checks[0]
                    if n == 4:
                        res = 1 if caught else 2
                    elif caught or n <= wobbles:
                        res = 0
                    else:
                        res = 2
                    if res == 1:                     # .Click: anim_keepsprites, anim_ret
                        pc["keep"] = True
                        pc["done"] = True
                        return
                    if res == 2:
                        pc["list"], pc["i"] = breakfree, 0
                        continue
                    o = find(1)                      # anim_incobj 1, wobble sound, back to .Loop
                    if o:
                        o["jt"] += 1
                    tl.event(EV_SOUND, SFX["GSC_BALL_WOBBLE"], PAN_RIGHT)
                    pc["list"], pc["i"] = loop, 0
                elif op == "ret":
                    pc["done"] = True
                    return

        if outcome == "poof":
            # BattleAnim_SendOutMon (.Normal): anim_sound SFX_BALL_POOF + BATTLE_ANIM_OBJ_BALL_POOF, played
            # here at the enemy's spot (136, 64) like the throw's; the game moves it to the Moemon's
            tl.event(EV_SOUND, SFX["GSC_BALL_POOF"], PAN_CENTER)
            new_obj("poof", 136, 64, 0)
            while any(o["alive"] for o in objs):
                sprites = []
                for o in objs:
                    if o["alive"]:
                        null_fn(o)
                        oam_update(o, sprites)
                tl.show(sprites)
            return tl, []

        # PokeBallEffect: DelayFrames 20; PlayBattleAnim: 6 frames, pals, 1 frame;
        # RunBattleAnimScript: ClearBattleAnims (1 frame)
        for _ in range(20 + 6 + 1 + 1):
            tl.show([])
        last = []
        while not pc["done"]:
            run_script()
            if pc["done"]:
                break
            bg_effects()
            sprites = []
            for o in objs:
                if not o["alive"]:
                    continue
                (ball_fn if o["kind"] == "ball" else null_fn)(o)
                oam_update(o, sprites)
            tl.show(sprites)
            last = sprites
        # BattleAnim_ClearOAM: kept sprites stay (caught); otherwise cleared. Then 3 frames.
        kept = last if pc["keep"] else []
        for _ in range(3):
            tl.show(kept)
        return tl, kept


# ---------------------------------------------------------------------------

def c_timeline(name, tl):
    sprites, frames = [], []
    prev = None
    for f in tl.frames:
        if f == prev:
            frames.append(frames[-1])    # unchanged: share the sprites
            continue
        frames.append((len(sprites), len(f)))
        sprites.extend(f)
        prev = f
    out = ["static const struct LegacyAnimSprite s%sSprites[] = {" % name]
    row = []
    for (x, y, tile, flags, pal) in sprites:
        fl = (1 if flags & XFLIP else 0) | (2 if flags & YFLIP else 0) | (pal << 4)
        row.append("{%d, %d, %d, 0x%02X}" % (x, y, tile, fl))
        if len(row) == 6:
            out.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        out.append("    " + ", ".join(row) + ",")
    if not sprites:
        out.append("    {0, 0, 0, 0},")
    out.append("};")
    out.append("static const struct LegacyAnimFrame s%sFrames[] = {" % name)
    row = []
    for (first, n) in frames:
        row.append("{%d, %d}" % (first, n))
        if len(row) == 10:
            out.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        out.append("    " + ", ".join(row) + ",")
    out.append("};")
    out.append("static const struct LegacyAnimEvent s%sEvents[] = {" % name)
    for (fr, t, a, b) in tl.events:
        out.append("    {%d, %d, %d, %d}," % (fr, t, a, b))
    out.append("    {0xFFFF, 0, 0, 0},")
    out.append("};")
    return out


def c_tiles(name, tiles):
    out = ["static const u8 s%sTiles[%d][64] = {" % (name, len(tiles))]
    for t in tiles:
        out.append("    {" + ",".join(str(v) for v in t) + "},")
    out.append("};")
    return out


def main():
    refs = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("MOEMON_PRET_REFS")
    if not refs:
        raise SystemExit(__doc__)
    red, gsc = Red(refs), Crystal(refs)
    out = ["// Generated by tools/moemon_legacy/anim_bake.py from pret/pokered af51989 and",
           "// pret/pokecrystal 97187cd - do not edit. Frame-by-frame POKé BALL throws.",
           ""]
    timelines = []
    for o in ["miss", 1, 2, 3, "caught"]:
        tl, _ = red.bake(o)
        n = "Red_%s" % o
        out += c_timeline(n, tl)
        timelines.append((n, tl))
    for o in [0, 1, 2, 3, "caught"]:
        tl, _ = gsc.bake(o)
        n = "Gsc_%s" % o
        out += c_timeline(n, tl)
        timelines.append((n, tl))
    # the smoke puffs alone (send-outs): Red's POOF_ANIM, Crystal's BALL_POOF object
    for (game, n) in ((red, "Red_poof"), (gsc, "Gsc_poof")):
        tl, _ = game.bake("poof")
        out += c_timeline(n, tl)
        timelines.append((n, tl))
    out += c_tiles("RedAnim", red.tiles)
    out += c_tiles("GscAnim", gsc.tiles)
    out.append("// Crystal's battle animation palettes (RGB 0-31): PAL_BATTLE_OB_GRAY, PAL_BATTLE_OB_RED")
    out.append("static const u8 sGscAnimPalettes[2][4][3] = {")
    for p in gsc.palettes:
        out.append("    {" + ", ".join("{%d, %d, %d}" % c for c in p) + "},")
    out.append("};")
    out.append("// timelines: Red miss, 1-3 shakes, caught; Crystal 0-3 wobbles, caught; Red poof, Crystal poof")
    out.append("static const struct LegacyAnimTimeline sLegacyTimelines[] = {")
    for (n, tl) in timelines:
        out.append("    {s%sFrames, s%sSprites, s%sEvents, %d}," % (n, n, n, len(tl.frames)))
    out.append("};")
    path = os.path.join(ROOT, "src", "mp", "mp_legacy_anim_data.h")
    with open(path, "w") as f:
        f.write("\n".join(out) + "\n")
    for (n, tl) in timelines:
        print("%-12s %4d frames, events: %s" % (n, len(tl.frames),
              " ".join("%d:%d/%d" % (fr, t, a) for (fr, t, a, b) in tl.events)))
    print("wrote", path)


if __name__ == "__main__":
    main()
