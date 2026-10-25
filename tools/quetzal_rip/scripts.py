"""Event script decompiler for Moemon Quetzal, writing Kai's macro syntax (asm/macros/event.inc).

Commands 0x00-0xE2 are stock Emerald (checked against the hack's command table: same
argument reads). 0xE3-0xF5 are Quetzal's own; they are written with qz_* macros that
tools/quetzal_rip/quetzal_macros.inc defines as raw bytes (see docs/quetzal/SCRIPT_COMMANDS.md).
"""
import json
import os
import re

from qrom import REPO, decode, looks_like_text

# ---- argument kinds --------------------------------------------------------
# b byte, h halfword, w word, P script ptr, T text ptr, M movement ptr, X data ptr,
# m map (group,num), W warp (map, warp id, x, y), F flag, V var/value, S species,
# I item, O move, C condition, D std id, K special, L local id, U song
OPS = {
    0x00: ('nop', ''), 0x01: ('nop1', ''), 0x02: ('end', ''), 0x03: ('return', ''),
    0x04: ('call', 'P'), 0x05: ('goto', 'P'), 0x06: ('goto_if', 'CP'), 0x07: ('call_if', 'CP'),
    0x08: ('gotostd', 'D'), 0x09: ('callstd', 'D'), 0x0A: ('gotostd_if', 'CD'), 0x0B: ('callstd_if', 'CD'),
    0x0C: ('returnram', ''), 0x0D: ('endram', ''), 0x0E: ('setmysteryeventstatus', 'b'),
    0x0F: ('loadword', 'bw'), 0x10: ('loadbyte', 'bb'), 0x11: ('setptr', 'bw'),
    0x12: ('loadbytefromptr', 'bw'), 0x13: ('setptrbyte', 'bw'), 0x14: ('copylocal', 'bb'),
    0x15: ('copybyte', 'ww'), 0x16: ('setvar', 'VV'), 0x17: ('addvar', 'VV'), 0x18: ('subvar', 'VV'),
    0x19: ('copyvar', 'VV'), 0x1A: ('setorcopyvar', 'VV'), 0x1B: ('compare_local_to_local', 'bb'),
    0x1C: ('compare_local_to_value', 'bb'), 0x1D: ('compare_local_to_ptr', 'bw'),
    0x1E: ('compare_ptr_to_local', 'wb'), 0x1F: ('compare_ptr_to_value', 'wb'),
    0x20: ('compare_ptr_to_ptr', 'ww'), 0x21: ('compare_var_to_value', 'VV'),
    0x22: ('compare_var_to_var', 'VV'), 0x23: ('callnative', 'w'), 0x24: ('gotonative', 'w'),
    0x25: ('special', 'K'), 0x26: ('specialvar', 'VK'), 0x27: ('waitstate', ''), 0x28: ('delay', 'h'),
    0x29: ('setflag', 'F'), 0x2A: ('clearflag', 'F'), 0x2B: ('checkflag', 'F'), 0x2C: ('initclock', 'VV'),
    0x2D: ('dotimebasedevents', ''), 0x2E: ('gettime', ''), 0x2F: ('playse', 'U'), 0x30: ('waitse', ''),
    0x31: ('playfanfare', 'U'), 0x32: ('waitfanfare', ''), 0x33: ('playbgm', 'Ub'), 0x34: ('savebgm', 'U'),
    0x35: ('fadedefaultbgm', ''), 0x36: ('fadenewbgm', 'U'), 0x37: ('fadeoutbgm', 'b'), 0x38: ('fadeinbgm', 'b'),
    0x39: ('warp', 'W'), 0x3A: ('warpsilent', 'W'), 0x3B: ('warpdoor', 'W'), 0x3C: ('warphole', 'm'),
    0x3D: ('warpteleport', 'W'), 0x3E: ('setwarp', 'W'), 0x3F: ('setdynamicwarp', 'W'),
    0x40: ('setdivewarp', 'W'), 0x41: ('setholewarp', 'W'), 0x42: ('getplayerxy', 'VV'),
    0x43: ('getpartysize', ''), 0x44: ('additem', 'IV'), 0x45: ('removeitem', 'IV'),
    0x46: ('checkitemspace', 'IV'), 0x47: ('checkitem', 'IV'), 0x48: ('checkitemtype', 'I'),
    0x49: ('addpcitem', 'IV'), 0x4A: ('checkpcitem', 'IV'), 0x4B: ('adddecoration', 'V'),
    0x4C: ('removedecoration', 'V'), 0x4D: ('checkdecor', 'V'), 0x4E: ('checkdecorspace', 'V'),
    0x4F: ('applymovement', 'LM'), 0x50: ('applymovementat', 'LMm'), 0x51: ('waitmovement', 'L'),
    0x52: ('waitmovementat', 'Lm'), 0x53: ('removeobject', 'L'), 0x54: ('removeobjectat', 'Lm'),
    0x55: ('addobject', 'L'), 0x56: ('addobjectat', 'Lm'), 0x57: ('setobjectxy', 'LVV'),
    0x58: ('showobjectat', 'Lm'), 0x59: ('hideobjectat', 'Lm'), 0x5A: ('faceplayer', ''),
    0x5B: ('turnobject', 'Lb'), 0x5C: ('trainerbattle', None), 0x5D: ('dotrainerbattle', ''),
    0x5E: ('gotopostbattlescript', ''), 0x5F: ('gotobeatenscript', ''), 0x60: ('checktrainerflag', 'V'),
    0x61: ('settrainerflag', 'V'), 0x62: ('cleartrainerflag', 'V'), 0x63: ('setobjectxyperm', 'LVV'),
    0x64: ('copyobjectxytoperm', 'L'), 0x65: ('setobjectmovementtype', 'Lb'), 0x66: ('waitmessage', ''),
    0x67: ('message', 'T'), 0x68: ('closemessage', ''), 0x69: ('lockall', ''), 0x6A: ('lock', ''),
    0x6B: ('releaseall', ''), 0x6C: ('release', ''), 0x6D: ('waitbuttonpress', ''), 0x6E: ('yesnobox', 'bb'),
    0x6F: ('multichoice', 'bbbb'), 0x70: ('multichoicedefault', 'bbbbb'), 0x71: ('multichoicegrid', 'bbbbb'),
    0x72: ('drawbox', ''), 0x73: ('erasebox', 'bbbb'), 0x74: ('drawboxtext', 'bbbb'),
    0x75: ('showmonpic', 'Sbb'), 0x76: ('hidemonpic', ''), 0x77: ('showcontestpainting', 'b'),
    0x78: ('braillemessage', 'X'), 0x79: ('givemon', 'SbIwwb'), 0x7A: ('giveegg', 'S'),
    0x7B: ('setmonmove', 'bbO'), 0x7C: ('checkpartymove', 'O'), 0x7D: ('bufferspeciesname', 'bS'),
    0x7E: ('bufferleadmonspeciesname', 'b'), 0x7F: ('bufferpartymonnick', 'bV'), 0x80: ('bufferitemname', 'bI'),
    0x81: ('bufferdecorationname', 'bV'), 0x82: ('buffermovename', 'bO'), 0x83: ('buffernumberstring', 'bV'),
    0x84: ('bufferstdstring', 'bV'), 0x85: ('bufferstring', 'bT'), 0x86: ('pokemart', 'R'),
    0x87: ('pokemartdecoration', 'R'), 0x88: ('pokemartdecoration2', 'R'), 0x89: ('playslotmachine', 'V'),
    0x8A: ('setberrytree', 'bbb'), 0x8B: ('choosecontestmon', ''), 0x8C: ('startcontest', ''),
    0x8D: ('showcontestresults', ''), 0x8E: ('contestlinktransfer', ''), 0x8F: ('random', 'V'),
    0x90: ('addmoney', 'wb'), 0x91: ('removemoney', 'wb'), 0x92: ('checkmoney', 'wb'),
    0x93: ('showmoneybox', 'bbb'), 0x94: ('hidemoneybox', 'bb'), 0x95: ('updatemoneybox', 'bbb'),
    0x96: ('getpokenewsactive', 'V'), 0x97: ('fadescreen', 'b'), 0x98: ('fadescreenspeed', 'bb'),
    0x99: ('setflashlevel', 'V'), 0x9A: ('animateflash', 'b'), 0x9B: ('messageautoscroll', 'T'),
    0x9C: ('dofieldeffect', 'V'), 0x9D: ('setfieldeffectargument', 'bV'), 0x9E: ('waitfieldeffect', 'V'),
    0x9F: ('setrespawn', 'V'), 0xA0: ('checkplayergender', ''), 0xA1: ('playmoncry', 'SV'),
    0xA2: ('setmetatile', 'VVVV'), 0xA3: ('resetweather', ''), 0xA4: ('setweather', 'V'),
    0xA5: ('doweather', ''), 0xA6: ('setstepcallback', 'b'), 0xA7: ('setmaplayoutindex', 'V'),
    0xA8: ('setobjectsubpriority', 'Lmb'), 0xA9: ('resetobjectsubpriority', 'Lm'),
    0xAA: ('createvobject', 'bbVVbb'), 0xAB: ('turnvobject', 'bb'), 0xAC: ('opendoor', 'VV'),
    0xAD: ('closedoor', 'VV'), 0xAE: ('waitdooranim', ''), 0xAF: ('setdooropen', 'VV'),
    0xB0: ('setdoorclosed', 'VV'), 0xB1: ('addelevmenuitem', 'bVVV'), 0xB2: ('showelevmenu', ''),
    0xB3: ('checkcoins', 'V'), 0xB4: ('addcoins', 'V'), 0xB5: ('removecoins', 'V'),
    0xB6: ('setwildbattle', 'SbI'), 0xB7: ('dowildbattle', ''), 0xB8: ('setvaddress', 'w'),
    0xB9: ('vgoto', 'w'), 0xBA: ('vcall', 'w'), 0xBB: ('vgoto_if', 'Cw'), 0xBC: ('vcall_if', 'Cw'),
    0xBD: ('vmessage', 'w'), 0xBE: ('vbuffermessage', 'w'), 0xBF: ('vbufferstring', 'bw'),
    0xC0: ('showcoinsbox', 'bb'), 0xC1: ('hidecoinsbox', 'bb'), 0xC2: ('updatecoinsbox', 'bb'),
    0xC3: ('incrementgamestat', 'b'), 0xC4: ('setescapewarp', 'W'), 0xC5: ('waitmoncry', ''),
    0xC6: ('bufferboxname', 'bV'), 0xC7: ('textcolor', 'b'), 0xC8: ('loadhelp', 'w'), 0xC9: ('unloadhelp', ''),
    0xCA: ('signmsg', ''), 0xCB: ('normalmsg', ''), 0xCC: ('comparehiddenvar', 'bw'),
    0xCD: ('setmodernfatefulencounter', 'V'), 0xCE: ('checkmodernfatefulencounter', 'V'),
    0xCF: ('trywondercardscript', ''), 0xD0: ('setworldmapflag', 'V'), 0xD1: ('warpspinenter', 'W'),
    0xD2: ('setmonmetlocation', 'Vb'), 0xD3: ('moverotatingtileobjects', 'V'),
    0xD4: ('turnrotatingtileobjects', ''), 0xD5: ('initrotatingtilepuzzle', 'V'),
    0xD6: ('freerotatingtilepuzzle', ''), 0xD7: ('warpmossdeepgym', 'W'), 0xD8: ('selectapproachingtrainer', ''),
    0xD9: ('lockfortrainer', ''), 0xDA: ('closebraillemessage', ''), 0xDB: ('messageinstant', 'T'),
    0xDC: ('fadescreenswapbuffers', 'b'), 0xDD: ('buffertrainerclassname', 'bV'),
    0xDE: ('buffertrainername', 'bV'), 0xDF: ('pokenavcall', 'T'), 0xE0: ('warpwhitefade', 'W'),
    0xE1: ('buffercontestname', 'bV'), 0xE2: ('bufferitemnameplural', 'bIV'),
    # Quetzal's own commands (argument sizes read off their handlers; meaning in SCRIPT_COMMANDS.md)
    0xE3: ('qz_bufferplayerstring', 'b'), 0xE4: ('qz_getsaveoption', ''), 0xE5: ('qz_cmd_e5', 'bb'),
    0xE6: ('qz_cmd_e6', 'bVV'), 0xE7: ('qz_cmd_e7', 'b'), 0xE8: ('qz_cmd_e8', ''),
    0xE9: ('qz_cmd_e9', 'V'), 0xEA: ('qz_cmd_ea', 'V'), 0xEB: ('qz_cmd_eb', ''),
    0xEC: ('qz_cmd_ec', 'SbI'), 0xED: ('qz_cmd_ed', 'V'), 0xEE: ('qz_cmd_ee', 'b'),
    0xEF: ('qz_cmd_ef', 'V'), 0xF0: ('qz_addbp', 'V'), 0xF1: ('qz_nop_f1', ''), 0xF2: ('qz_nop_f2', ''),
    0xF3: ('qz_nop_f3', ''), 0xF4: ('qz_nop_f4', ''), 0xF5: ('qz_cmd_f5', ''),
}
SIZES = {'b': 1, 'h': 2, 'w': 4, 'P': 4, 'T': 4, 'M': 4, 'X': 4, 'R': 4, 'm': 2, 'W': 7, 'F': 2, 'V': 2,
         'S': 2, 'I': 2, 'O': 2, 'C': 1, 'D': 1, 'K': 2, 'L': 2, 'U': 2}
TERMINAL = {0x02, 0x03, 0x05, 0x08, 0x0C, 0x0D, 0x5E, 0x5F, 0xB9}
COND = {0: 'LT', 1: 'EQ', 2: 'GT', 3: 'LE', 4: 'GE', 5: 'NE'}   # goto_if_* / call_if_* suffixes
STD = {0: 'STD_OBTAIN_ITEM', 1: 'STD_FIND_ITEM', 2: 'MSGBOX_NPC', 3: 'MSGBOX_SIGN', 4: 'MSGBOX_DEFAULT',
       5: 'MSGBOX_YESNO', 6: 'MSGBOX_AUTOCLOSE', 7: 'STD_OBTAIN_DECORATION', 8: 'STD_REGISTER_MATCH_CALL',
       9: 'MSGBOX_GETPOINTS', 10: 'MSGBOX_POKENAV', 11: 'STD_QZ_11', 12: 'STD_QZ_12'}
# trainerbattle types: which pointers follow (T text, P script)
TB = {0: 'TT', 1: 'TTP', 2: 'TTP', 3: 'T', 4: 'TTT', 5: 'TT', 6: 'TTTP', 7: 'TTT', 8: 'TTTP',
      9: 'TT', 10: 'TT', 11: 'TT', 12: 'TT', 13: 'TT'}
TB_NAMES = {0: 'TRAINER_BATTLE_SINGLE', 1: 'TRAINER_BATTLE_CONTINUE_SCRIPT_NO_MUSIC',
            2: 'TRAINER_BATTLE_CONTINUE_SCRIPT', 3: 'TRAINER_BATTLE_SINGLE_NO_INTRO_TEXT',
            4: 'TRAINER_BATTLE_DOUBLE', 5: 'TRAINER_BATTLE_REMATCH', 6: 'TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE',
            7: 'TRAINER_BATTLE_REMATCH_DOUBLE', 8: 'TRAINER_BATTLE_CONTINUE_SCRIPT_DOUBLE_NO_MUSIC',
            9: 'TRAINER_BATTLE_PYRAMID', 10: 'TRAINER_BATTLE_SET_TRAINER_A', 11: 'TRAINER_BATTLE_SET_TRAINER_B',
            12: 'TRAINER_BATTLE_HILL', 13: 'TRAINER_BATTLE_QZ_13'}


def _kai_names():
    """Kai's own constant names for things the hack kept at stock Emerald numbers."""
    # stock Emerald's specials in pokeemerald order (Kai dropped DoSoftReset and appended its own,
    # so Kai's data/specials.inc is not the hack's order)
    specials = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'emerald_specials.txt')).read().split()
    songs = {}
    for m in re.finditer(r'#define ((?:MUS|SE)_\w+)\s+(\d+)', open(os.path.join(REPO, 'include/constants/songs.h')).read()):
        songs.setdefault(int(m.group(2)), m.group(1))
    mv = {}
    consts = {}
    for m in re.finditer(r'#define (MOVEMENT_ACTION_\w+)\s+(0x[0-9A-Fa-f]+|\d+)', open(os.path.join(REPO, 'include/constants/event_object_movement.h')).read()):
        consts[m.group(1)] = int(m.group(2), 0)
    for m in re.finditer(r'create_movement_action (\w+), (MOVEMENT_ACTION_\w+)', open(os.path.join(REPO, 'asm/macros/movement.inc')).read()):
        if m.group(2) in consts:
            mv.setdefault(consts[m.group(2)], m.group(1))
    specvars = {}
    for m in re.finditer(r'#define (VAR_\w+)\s+(0x8[0-9A-Fa-f]{3})', open(os.path.join(REPO, 'include/constants/vars.h')).read()):
        specvars.setdefault(int(m.group(2), 16), m.group(1))
    return specials, songs, mv, specvars


SPECIALS, SONGS, MOVEMENTS, SPECIAL_VARS = _kai_names()

# Names worked out from the ROM (docs/quetzal/SPECIALS.md, SCRIPT_COMMANDS.md): Quetzal inserted 13
# specials into the stock table, so its index n is NOT pokeemerald's n.
_QZN = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'qz_names.json')))
KAI_SPECIALS = set(m.group(1) for m in re.finditer(r'^\s*def_special (\w+)', open(os.path.join(REPO, 'data/specials.inc')).read(), re.M))
QZ_STOCK_SPECIALS = {int(k): v for k, v in _QZN.get('stock_index_names', {}).items()}
QZ_NEW_SPECIALS = {int(k): v for k, v in _QZN.get('specials', {}).items()}
for _op, _info in _QZN.get('commands', {}).items():
    _o = int(_op, 16)
    if _o in OPS and _info.get('name'):
        OPS[_o] = ('qz_' + _info['name'], OPS[_o][1])


def special_name(v):
    """Kai's name when slot v holds a stock special Kai still has; else Qz<Name> (defined in quetzal_macros.inc)."""
    n = QZ_STOCK_SPECIALS.get(v)
    if n and n in KAI_SPECIALS and v not in QZ_NEW_SPECIALS:
        return n
    n = QZ_NEW_SPECIALS.get(v) or n
    return 'Qz%s_%d' % (n, v) if n else 'QZ_SPECIAL_%d' % v
NUM_VANILLA_SONGS = 559   # Emerald song ids 0-558 are stock in Quetzal (FRLG songs included); 559+ are new


def flag_name(v):
    if v < 0x20:
        return 'FLAG_TEMP_%X' % v if v else '0'
    return 'FLAG_QZ_0x%X' % v


def var_or_value(v):
    if 0x8000 <= v <= 0x8015:
        return SPECIAL_VARS.get(v, 'VAR_0x%04X' % v)
    if 0x4000 <= v <= 0x400F:
        return 'VAR_TEMP_%X' % (v - 0x4000)
    if 0x4010 <= v <= 0x40FF or 0x5000 <= v <= 0x51FF:
        return 'VAR_QZ_0x%04X' % v
    return '0x%X' % v if v > 9 else '%d' % v


class Disassembler:
    """Walks scripts from entry points; records every reached code/text/movement/mart address."""

    def __init__(self, rom, names, map_const):
        self.rom = rom
        self.names = names
        self.map_const = map_const       # (group, num) -> MAP_ constant
        self.insn = {}       # addr -> (name, args, end) for every decoded command
        self.kind = {}       # addr -> 'script' (a label) | 'text' | 'movement' | 'mart' | 'braille'
        self.entries = {}    # addr -> owners that use it as an entry point
        self.map_refs = set()
        self.flags = set()
        self.vars = set()
        self.specials = set()
        self.trainers = set()
        self.species = set()
        self.items = set()
        self.problems = []

    # ---- walking -------------------------------------------------------------
    def add_entry(self, addr, owner):
        """Walk everything reachable from addr. owner is only recorded on the entry itself."""
        self.entries.setdefault(addr, set()).add(owner)
        work = [(addr, 'script')]
        while work:
            a, k = work.pop()
            if a is None:
                continue
            if a in self.kind:
                if self.kind[a] != k:
                    self.problems.append('0x%X used as both %s and %s' % (a, self.kind[a], k))
                continue
            if k == 'script' and a in self.insn:
                self.kind[a] = 'script'     # jump into code we already decoded: just a new label
                continue
            self.kind[a] = k
            if k == 'script':
                work.extend(self._walk(a))

    def _walk(self, a):
        rom = self.rom
        out = []
        pc = a
        while True:
            if pc >= rom.n:
                self.problems.append('script 0x%X runs past the ROM end' % a)
                break
            if pc != a and pc in self.insn:
                out.append((pc, 'script'))       # runs into code decoded before: label it
                break
            op = rom.d[pc]
            spec = OPS.get(op)
            if spec is None:
                self.problems.append('unknown opcode 0x%02X at 0x%X (script 0x%X)' % (op, pc, a))
                self.insn[pc] = ('bad', [op], pc + 1, op)
                break
            name, kinds = spec
            p = pc + 1
            args = []
            if op == 0x5C:
                t = rom.d[p]
                trainer = rom.u16(p + 1)
                lid = rom.u16(p + 3)
                p += 5
                args = [('tbtype', t), ('trainer', trainer), ('L', lid)]
                self.trainers.add(trainer)
                for k in TB.get(t, ''):
                    v = rom.ptr(p)
                    p += 4
                    args.append((k, v))
                    if v is not None:
                        out.append((v, 'text' if k == 'T' else 'script'))
                if t not in TB:
                    self.problems.append('unknown trainerbattle type %d at 0x%X' % (t, pc))
            else:
                for k in kinds:
                    sz = SIZES[k]
                    if sz == 1:
                        v = rom.d[p]
                    elif sz == 2:
                        v = rom.u16(p) if k != 'm' else (rom.d[p], rom.d[p + 1])
                    elif sz == 4:
                        v = rom.u32(p)
                    else:  # warp
                        v = (rom.d[p], rom.d[p + 1], rom.d[p + 2], rom.s16(p + 3), rom.s16(p + 5))
                    p += sz
                    if k in 'PTMXR':
                        o = v - 0x08000000 if rom.isptr(v) else None
                        if o is None and v != 0:
                            self.problems.append('bad pointer 0x%X in %s at 0x%X' % (v, name, pc))
                        args.append((k, o if o is not None else ('raw', v)))
                        if o is not None:
                            out.append((o, {'P': 'script', 'T': 'text', 'M': 'movement', 'X': 'braille', 'R': 'mart'}[k]))
                        continue
                    args.append((k, v))
                    if k == 'F':
                        self.flags.add(v)
                    elif k == 'V' and (0x4000 <= v < 0x4100 or 0x5000 <= v < 0x5200):
                        self.vars.add(v)
                    elif k == 'K':
                        self.specials.add(v)
                    elif k == 'S' and v < 0x4000:
                        self.species.add(v)
                    elif k == 'I' and v < 0x4000:
                        self.items.add(v)
                    elif k == 'm' or k == 'W':
                        self.map_refs.add((v[0], v[1]))
                # loadword 0, <text>  (what msgbox expands to)
                if op == 0x0F and args[0][1] == 0 and rom.isptr(args[1][1]):
                    o = args[1][1] - 0x08000000
                    if looks_like_text(rom, o):
                        args[1] = ('T', o)
                        out.append((o, 'text'))
            self.insn[pc] = (name, args, p, op)
            pc = p
            if op in TERMINAL:
                break
        return out

    def finish(self):
        """Split decoded code into labelled blocks and give every block its owners."""
        labels = sorted(a for a, k in self.kind.items() if k == 'script')
        lset = set(labels)
        data_starts = sorted(a for a, k in self.kind.items() if k != 'script')
        import bisect
        self.runs_into_data = set()
        self.blocks = {}
        self.edges = {}
        for L in labels:
            pcs = []
            pc = L
            nxt = None
            while pc in self.insn:
                name, args, end, op = self.insn[pc]
                j = bisect.bisect_right(data_starts, pc - 1)
                if j < len(data_starts) and data_starts[j] < end:
                    # the walk ran on into a text / movement that is labelled elsewhere: the ROM script
                    # never gets here (e.g. a yes/no that jumps away on both answers)
                    self.runs_into_data.add(L)
                    break
                pcs.append(pc)
                if name == 'bad' or op in TERMINAL:
                    break
                if end in lset:
                    nxt = end
                    break
                pc = end
            self.blocks[L] = (pcs, nxt)
            refs = set()
            for pc in pcs:
                for k, v in self.insn[pc][1]:
                    if k in 'PTMXR' and isinstance(v, int):
                        refs.add(v)
            if nxt is not None:
                refs.add(nxt)
            self.edges[L] = refs
        # owners: everything reachable from a map's entries belongs to that map
        self.owners = {}
        by_owner = {}
        for a, owners in self.entries.items():
            for o in owners:
                by_owner.setdefault(o, []).append(a)
        for o, starts in by_owner.items():
            seen = set()
            stack = list(starts)
            while stack:
                a = stack.pop()
                if a in seen:
                    continue
                seen.add(a)
                self.owners.setdefault(a, set()).add(o)
                stack.extend(self.edges.get(a, ()))

    # ---- data blocks ---------------------------------------------------------
    def movement(self, a):
        rom = self.rom
        out = []
        p = a
        while p < rom.n and len(out) < 512:
            b = rom.d[p]
            p += 1
            if b == 0xFE:
                out.append('step_end')
                break
            out.append(MOVEMENTS.get(b, '.byte 0x%02X' % b))
        return out

    def mart(self, a):
        rom = self.rom
        out = []
        p = a
        while p < rom.n and len(out) < 256:
            v = rom.u16(p)
            p += 2
            out.append(v)
            if v == 0:
                break
        return out


# ---- output -----------------------------------------------------------------
def text_lines(s):
    """Split a decoded string into Kai-style .string lines."""
    out = []
    cur = ''
    i = 0
    while i < len(s):
        cur += s[i]
        if s[i] == '\\' and i + 1 < len(s) and s[i + 1] in 'nlp':
            cur += s[i + 1]
            i += 2
            out.append(cur)
            cur = ''
            continue
        i += 1
    if cur or not out:
        out.append(cur)
    out[-1] += '$'
    return ['\t.string "%s"' % l for l in out]


class Emitter:
    def __init__(self, dis, label_of, names):
        self.dis = dis
        self.label_of = label_of      # addr -> label (must exist for every referenced addr)
        self.names = names
        self.map_const = dis.map_const

    def lab(self, a):
        return self.label_of(a)

    def mapc(self, gn):
        return self.map_const(gn)

    def arg(self, k, v, op=None):
        n = self.names
        if k in 'PTMXR':
            if isinstance(v, tuple):
                return '0x%X' % v[1]
            if v is None:
                return '0'
            if v in self.dis.kind:
                return self.lab(v)
            return '0x%X' % (v + 0x08000000)
        if k == 'C':
            return '%d' % v
        if k == 'D':
            return STD.get(v, '%d' % v)
        if k == 'F':
            return flag_name(v)
        if k == 'K':
            return special_name(v)
        if k == 'S':
            return n.species_name(v) if v < 0x4000 else var_or_value(v)
        if k == 'I':
            return n.item_name(v) if v < 0x4000 else var_or_value(v)
        if k == 'O':
            return n.move_name(v) if v < 0x4000 else var_or_value(v)
        if k == 'U':
            return SONGS.get(v, 'MUS_QZ_%d' % v) if v < NUM_VANILLA_SONGS else 'MUS_QZ_%d' % v
        if k == 'L':
            if v >= 0x4000:
                return var_or_value(v)
            return {0xFF: 'LOCALID_PLAYER', 0x7F: 'LOCALID_CAMERA'}.get(v, '%d' % v)
        if k == 'V':
            return var_or_value(v)
        if k == 'm':
            return self.mapc(v)
        if k == 'W':
            g, num, wid, x, y = v
            return '%s, %d, %d, %d' % (self.mapc((g, num)), wid, x, y)
        if k == 'trainer':
            return 'TRAINER_QZ_%d' % v
        if k == 'tbtype':
            return TB_NAMES.get(v, '%d' % v)
        if k in 'bhw':
            return '0x%X' % v if v > 9 else '%d' % v
        return str(v)

    def insn(self, name, args):
        if name == 'bad':
            return '\t.byte 0x%02X  @ unknown command, decoding stopped' % args[0]
        # readable forms that assemble to the same bytes
        if name == 'trainerbattle' and args[0][1] not in TB_NAMES or name == 'trainerbattle' and args[0][1] == 13:
            # Quetzal's type 13: trainer, local id, intro text, defeat text (Kai's macro doesn't know it)
            return '\tqz_trainerbattle_13 %s' % ', '.join(self.arg(k, v) for k, v in args[1:])
        if name == 'givemon':
            sp, lv, it, w1, w2, b = (a[1] for a in args)
            if w1 or w2 or b:   # Quetzal uses fields stock Emerald leaves 0
                return '\tqz_givemon_ex %s, %s, %s, %d, %d, %d' % (self.arg(*args[0]), self.arg(*args[1]), self.arg(*args[2]), w1, w2, b)
            return '\tgivemon %s, %s, %s' % (self.arg(*args[0]), self.arg(*args[1]), self.arg(*args[2]))
        if name in ('hidemoneybox', 'updatemoneybox'):
            if args[0][1] or args[1][1]:
                return '\t.byte 0x%02X, %d, %d%s  @ %s' % (0x94 if name == 'hidemoneybox' else 0x95, args[0][1], args[1][1],
                                                      ', %d' % args[2][1] if len(args) > 2 else '', name)
            return '\t%s%s' % (name, ' %d' % args[2][1] if len(args) > 2 else '')
        if name in ('goto_if', 'call_if') and args[0][1] in COND:
            return '\t%s_%s %s' % (name, COND[args[0][1]].lower(), self.arg(*args[1]))
        if name == 'callstd' and False:
            pass
        return '\t%s %s' % (name, ', '.join(self.arg(k, v) for k, v in args)) if args else '\t' + name
