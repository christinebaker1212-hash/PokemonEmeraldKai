#!/usr/bin/env python3
"""SINNOH's scripts: Platinum's own event scripts (pret/pokeplatinum res/field/scripts, at the pinned commit of
fetch_decomp.sh) turned into this game's script commands when the game is built, with Platinum's own words (each
message a placeholder for its line in sinnoh/platinum_text.pak, tools/sinnoh/build_text.py). Like the 3D pack and the
words, none of it is in the repository: the output, sinnoh/platinum_scripts.inc, is made by tools/sinnoh/make_pack.sh
(the release runs it before building the game) and included by data/event_scripts_sinnoh.s.

The repository's maps only point at it: a person / sign / trigger whose Platinum script converts runs
`goto SinnohP_<script file>_<entry number>` (tools/sinnoh/gen_words.py writes those stubs: numbers only, no names, no
words). Without the decomp, `gen_scripts.py --stub` defines every label the maps point at as an `end`, so the game still
builds (SINNOH then says nothing: it needs the packs anyway).

    python3 -I tools/sinnoh/gen_scripts.py POKEPLATINUM_DIR [OUT.inc]     (default sinnoh/platinum_scripts.inc)
    python3 -I tools/sinnoh/gen_scripts.py --stub [OUT.inc]
    python3 -I tools/sinnoh/gen_scripts.py --auto [OUT.inc]      the build's: from the decomp make_pack.sh used
                                                                 (sinnoh/decomp_dir), else --stub
    python3 -I tools/sinnoh/gen_scripts.py --report POKEPLATINUM_DIR      what converts, what doesn't yet (and why)

A script converts when every command it reaches (its labels, the labels it goes to or calls, the ones it falls into)
is one this file knows (CONVERT below); the rest keep this game's written words until their commands are added
(docs/SINNOH_HANDOFF.md step 4). Platinum's flags and variables get SINNOH's numbers (tools/sinnoh/ids.py).
"""
import collections
import glob
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import ids  # noqa: E402
import build_text  # noqa: E402
from gen_trainers import RENAME  # noqa: E402

OUT = os.path.join(ROOT, 'sinnoh', 'platinum_scripts.inc')
PREFIX = 'SinnohP_'
TERMINAL = {'End', 'Return', 'GoTo', 'EndMovement', 'ReturnToField'}

# this game's SINNOH script command (data/script_cmd_table.inc 0xF6 `sinnoh OP, ARG, ARG2`, src/sinnoh/s3d_scrcmd.c)
SINNOH_BUFFER_PLAYER, SINNOH_BUFFER_RIVAL, SINNOH_BUFFER_COUNTERPART, SINNOH_YESNO = 1, 2, 3, 4
SINNOH_GET_DIR, SINNOH_GET_POS, SINNOH_GET_POCKET, SINNOH_IS_TMHM, SINNOH_BUFFER_TMHM_MOVE = 5, 6, 7, 8, 9
SINNOH_BUFFER_POCKET, SINNOH_BADGE_CHECK, SINNOH_NATIONAL_DEX = 10, 11, 12
SINNOH_MENU_INIT, SINNOH_MENU_ADD, SINNOH_MENU_SHOW = 13, 14, 15
SINNOH_FIRST_NON_EGG, SINNOH_COUNT_ALIVE_EXCEPT, SINNOH_HAS_SPECIES, SINNOH_DAY_OF_WEEK = 16, 17, 18, 19
SINNOH_TIME_OF_DAY, SINNOH_ON_BIKE, SINNOH_STARTER, SINNOH_FACE = 20, 21, 22, 23
SINNOH_GAME_COMPLETED, SINNOH_DID_NOT_CAPTURE = 24, 25

# Platinum's comparisons (CompareVar / CheckFlag then GoToIf / CallIf): the same codes as this game's
COND = {'Lt': 0, 'Eq': 1, 'Gt': 2, 'Le': 3, 'Ge': 4, 'Ne': 5}

# Platinum's sound effects that have a match here (others are left out: a script never waits on a missing one)
SE = {'SE_CONFIRM_sseq_3': 'SE_SELECT', 'SE_CONFIRM': 'SE_SELECT', 'SEQ_SE_CONFIRM': 'SE_SELECT',
      'SEQ_SE_DP_SELECT': 'SE_SELECT', 'SEQ_SE_DP_WIN_OPEN': 'SE_WIN_OPEN', 'SEQ_SE_DP_KAIDAN2': 'SE_EXIT',
      'SEQ_SE_DP_DOOR_OPEN': 'SE_DOOR', 'SEQ_SE_DP_DOOR_CLOSE': 'SE_DOOR'}
# Platinum's fanfares (the obtained-item jingles and the rest): this game's closest
FANFARE = {'SEQ_FANFA4': 'MUS_OBTAIN_ITEM', 'SEQ_FANFA3': 'MUS_OBTAIN_TMHM', 'SEQ_FANFA1': 'MUS_HEAL',
           'SEQ_FANFA5': 'MUS_OBTAIN_BADGE', 'SEQ_FANFA2': 'MUS_LEVEL_UP', 'SEQ_KINOMI': 'MUS_OBTAIN_BERRY',
           'SEQ_ACCE': 'MUS_OBTAIN_ITEM', 'SEQ_POKEWALKER': 'MUS_OBTAIN_ITEM', 'SEQ_BADGE': 'MUS_OBTAIN_BADGE',
           'SEQ_ASA': 'MUS_HEAL', 'SEQ_KEYITEM': 'MUS_OBTAIN_ITEM', 'SEQ_WAZA': 'MUS_OBTAIN_TMHM'}

CONSTANTS = {'TRUE': 1, 'FALSE': 0, 'MENU_YES': 0, 'MENU_NO': 1, 'GENDER_MALE': 0, 'GENDER_FEMALE': 1}

# Platinum's movement actions -> this game's (Platinum's field runs at 30 frames a second, this one at 60: a delay
# is twice as many frames here)
DIRS = {'North': 'up', 'South': 'down', 'West': 'left', 'East': 'right'}
MOVE_KIND = {'WalkNormal': 'walk_', 'WalkFast': 'walk_fast_', 'WalkFaster': 'walk_faster_', 'WalkSlow': 'walk_slow_',
             'WalkSlower': 'walk_slow_', 'WalkFastest': 'walk_faster_',
             'WalkOnSpotNormal': 'walk_in_place_', 'WalkOnSpotFast': 'walk_in_place_fast_',
             'WalkOnSpotFaster': 'walk_in_place_faster_', 'WalkOnSpotSlow': 'walk_in_place_slow_', 'Face': 'face_',
             'JumpOnSpotFast': 'jump_in_place_', 'JumpOnSpotSlow': 'jump_in_place_', 'JumpNear': 'jump_',
             'JumpFar': 'jump_2_'}
MOVE_ONE = {'EmoteExclamationMark': ['emote_exclamation_mark'], 'EmoteQuestionMark': ['emote_question_mark'],
            'SetInvisible': ['set_invisible'], 'SetVisible': ['set_visible'], 'LockDir': ['lock_facing_direction'],
            'UnlockDir': ['unlock_facing_direction'], 'FacePlayer': ['face_player'],
            'FaceAwayFromPlayer': ['face_away_player']}
DELAYS = {1: ['delay_2'], 2: ['delay_4'], 4: ['delay_8'], 8: ['delay_16'], 15: ['delay_16', 'delay_8', 'delay_4', 'delay_2'],
          16: ['delay_16', 'delay_16'], 32: ['delay_16'] * 4}


def _music_of():
    """Platinum's songs (SEQ_*) -> this game's SINNOH songs (gen_maps.py's MUSIC: the songs the maps play)"""
    try:
        from gen_maps import MUSIC
    except Exception:
        return {}
    out = {seq: 'MUS_SINNOH_' + name.upper() for seq, name in MUSIC.items()}
    # the story's own songs (not a map's)
    out.update({'SEQ_THE_RIV_sseq': 'MUS_SINNOH_RIVAL', 'SEQ_THE_GIRL_sseq': 'MUS_SINNOH_DAWN',
                'SEQ_THE_BOY_sseq': 'MUS_SINNOH_LUCAS', 'SEQ_TSURETEKE_sseq': 'MUS_SINNOH_FOLLOW_ME',
                'SEQ_PL_HANDSOME_sseq': 'MUS_SINNOH_LOOKER', 'SEQ_FUE_sseq': 'MUS_SINNOH_AZURE_FLUTE',
                'SEQ_D_SAFARI_sseq': 'MUS_SINNOH_GREAT_MARSH', 'SEQ_THE_EVENT02_sseq': 'MUS_SINNOH_GALACTIC_APPEARS'})
    return out


MUSIC_OF = _music_of()


class Unsupported(Exception):
    pass


# ----------------------------------------------------------------------------------------------------- the decomp

def parse(path):
    """{label: [(command, [args])]}, the entries (ScriptEntry names in order), the labels in file order"""
    labels, entries, order, cur = {}, [], [], None
    for raw in open(path, encoding='utf-8', errors='replace'):
        line = raw.split('//')[0].split('@')[0].rstrip()   # (both kinds of comments)
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        m = re.match(r'^(\w+):\s*$', line)
        if m:
            cur = m.group(1)
            labels[cur] = []
            order.append(cur)
            continue
        m = re.match(r'^\s+(\w+)\s*(.*)$', line)
        if not m:
            continue
        cmd, args = m.group(1), m.group(2).strip()
        if cmd == 'ScriptEntry':
            entries.append(args)
            continue
        if cmd in ('ScriptEntryEnd', '.balign', '.align'):
            continue
        if cur:
            labels[cur].append((cmd, [a.strip() for a in args.split(',')] if args else []))
    return labels, entries, order


class Decomp:
    """What every file's conversion shares: the banks' numbers and messages, the constants, the common macros"""

    def __init__(self, root):
        self.root = root
        self.banks = build_text.bank_numbers(root)
        self._ids = {}
        self._global = None
        self.consts = {}
        gdir = os.path.join(root, 'generated')
        for f in sorted(os.listdir(gdir)) if os.path.isdir(gdir) else []:
            if f.endswith('.txt'):
                for i, line in enumerate(open(os.path.join(gdir, f), encoding='utf-8', errors='replace')):
                    name = line.strip().split()[0] if line.strip() else ''
                    if re.match(r'^[A-Z][A-Z0-9_]+$', name):
                        self.consts.setdefault(name, i)
        for path in glob.glob(os.path.join(root, 'include/constants/**/*.h'), recursive=True):
            for m in re.finditer(r'^#define\s+([A-Z][A-Z0-9_]+)\s+(-?(?:0x[0-9A-Fa-f]+|\d+))\b', open(path, errors='replace').read(),
                                 re.M):
                self.consts.setdefault(m.group(1), int(m.group(2), 0))
        self.trainers = set(re.findall(r'^#define\s+(TRAINER_SINNOH_\w+)\b',
                                       open(os.path.join(ROOT, 'include/constants/trainers_sinnoh.h')).read(), re.M))
        self._origins = None
        self.ours = set()
        for h in ('items.h', 'species.h', 'moves.h'):
            self.ours |= set(re.findall(r'^#define\s+((?:ITEM|SPECIES|MOVE)_\w+)\b',
                                        open(os.path.join(ROOT, 'include/constants', h), errors='replace').read(), re.M))
        # Common_X macros -> the common script they call (CallCommonScript N)
        self.common_macro = {}
        text = open(os.path.join(root, 'asm/macros/scrcmd.inc'), errors='replace').read()
        for m in re.finditer(r'\.macro (Common_\w+)\s*\n\s*CallCommonScript (0x[0-9A-Fa-f]+|\d+)\s*\n\s*\.endm', text):
            self.common_macro[m.group(1)] = int(m.group(2), 0)

    def origins(self):
        """{Platinum header in lower case (twinleaf_town): (this game's map constant, its x and z in Platinum's
        coordinates)}"""
        if self._origins is None:
            from source import open_source
            from gen_maps import places
            self._origins = {}
            for p in places(open_source(self.root), self.root):
                low = p['place']['header'][len('MAP_HEADER_'):].lower()
                self._origins.setdefault(low, (p['id'], p['x0'] * 32, p['z0'] * 32))
        return self._origins

    def trainer(self, x):
        """this game's constant for a Platinum trainer (gen_trainers.py's)"""
        ours = 'TRAINER_SINNOH_' + x[len('TRAINER_'):]
        return ours if ours in self.trainers else None

    def ids(self, bank):
        if bank not in self._ids:
            d = json.load(open(os.path.join(self.root, 'res/text', bank + '.json'), encoding='utf-8'))
            self._ids[bank] = {m['id']: i for i, m in enumerate(d.get('messages', []))}
        return self._ids[bank]

    def message(self, bank, msg_id):
        """(bank number, message number) of a message: its file's bank, else wherever it is"""
        if bank and msg_id in self.ids(bank):
            return self.banks[bank], self.ids(bank)[msg_id]
        if self._global is None:
            self._global = {}
            for b in self.banks:
                for k, i in self.ids(b).items():
                    self._global.setdefault(k, (self.banks[b], i))
        return self._global.get(msg_id)


def common_target(n):
    """(file, entry from 1) of Platinum's script number N (2000-2499 the common scripts, 2500- the bg events')"""
    if 2000 <= n < 2500:
        return 'common', n - 2000 + 1
    if 2500 <= n < 2800:
        return 'bg_events', n - 2500 + 1
    return None


# ----------------------------------------------------------------------------------------------------- one file

class FileConverter:
    def __init__(self, dec, path):
        self.dec, self.root, self.path = dec, dec.root, path
        self.low = os.path.basename(path)[len('scripts_'):-2]
        text = open(path, encoding='utf-8', errors='replace').read()
        m = re.search(r'#include "res/text/bank/(\w+)\.h"', text)
        self.bank = m.group(1) if m else None
        m = re.search(r'#include "res/field/events/events_(\w+)\.h"', text)
        self.events_low = m.group(1) if m else self.low
        self.localids = {}
        if m:
            ev = os.path.join(self.root, 'res/field/events', 'events_%s.json' % m.group(1))
            if os.path.exists(ev):
                for i, o in enumerate(json.load(open(ev)).get('object_events', [])):
                    if isinstance(o.get('id'), str):
                        self.localids[o['id']] = i + 1
        self.labels, self.entries, self.order = parse(path)
        self.texts = {}            # label -> (bank number, message number)
        self._local = 0
        self.calls = set()         # (file, entry) of the common scripts it calls
        self.moves = set()         # movement labels
        self._ok = {}

    def next_local(self):
        self._local += 1
        return self._local

    # ---- names
    def label(self, name):
        if name not in self.labels:
            raise Unsupported('goes to %s (another file)' % name)
        return '%s%s__%s' % (PREFIX, self.low, name)

    def entry_label(self, n):
        return '%s%s_%d' % (PREFIX, self.low, n)

    def text_label(self, msg_id):
        where = self.dec.message(self.bank, msg_id)
        if where is None:
            raise Unsupported('message %s' % msg_id)
        lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
        self.texts[lab] = where
        return lab

    def value(self, x):
        if re.match(r'^-?(0x[0-9A-Fa-f]+|\d+)$', x):
            return str(int(x, 0))
        if x in CONSTANTS:
            return str(CONSTANTS[x])
        if re.match(r'^(ITEM|SPECIES|MOVE)_', x):
            x = RENAME.get(x, x)
            x = {'ITEM_SECRETPOTION': 'ITEM_SECRET_POTION'}.get(x, x)
            if x in self.dec.ours:
                return x
            raise Unsupported('value %s (not in this game)' % x)
        if x.startswith('TRAINER_') and not x.startswith('TRAINER_CLASS_'):
            t = self.dec.trainer(x)
            if t:
                return t
            raise Unsupported('trainer %s (not in this game)' % x)
        if x in self.dec.consts:
            return str(self.dec.consts[x])
        raise Unsupported('value %s' % x)

    def var(self, x):
        if x in ('VAR_LAST_TALKED', 'VAR_FACING'):
            return x                  # (this game's own special variables of the same meaning)
        if re.match(r'^(0x[0-9A-Fa-f]+|\d+)$', x) and int(x, 0) >= 0x4000:
            raise Unsupported('a raw variable number %s' % x)
        v = ids.var(self.root, x) if x.startswith('VAR_') else None
        if v is None:
            raise Unsupported('variable %s' % x)
        return v if isinstance(v, str) else '0x%X' % v

    def arg(self, x):
        """a value or a variable (the commands that take either)"""
        return self.var(x) if self.is_var(x) else self.value(x)

    def is_var(self, x):
        return x.startswith('VAR_')

    def flag(self, x):
        f = ids.flag(self.root, x) if x.startswith('FLAG_') else None
        if f is None:
            raise Unsupported('flag %s' % x)
        return '0x%X' % f

    def localid(self, x):
        if x in ('LOCALID_PLAYER', 'OBJ_EVENT_ID_PLAYER'):
            return 'OBJ_EVENT_ID_PLAYER'
        if x in self.localids:
            return str(self.localids[x])
        if re.match(r'^\d+$', x):
            return x
        if x.startswith('VAR_'):
            return self.var(x)        # (this game's commands take a variable for the object too)
        raise Unsupported('object %s' % x)

    def strvar(self, x):
        n = int(self.value(x))
        if not 0 <= n <= 2:
            raise Unsupported('string buffer %d' % n)
        return 'STR_VAR_%d' % (n + 1)

    def compare(self, a, b):
        if self.is_var(b):
            return ['compare_var_to_var %s, %s' % (self.var(a), self.var(b))]
        return ['compare_var_to_value %s, %s' % (self.var(a), self.value(b))]

    def sinnoh(self, op, a=0, b=0, c=0):
        return 'sinnoh %d, %s, %s, %s' % (op, a, b, c)

    def common(self, number):
        t = common_target(number)
        if t is None:
            raise Unsupported('common script %d' % number)
        c = converter(self.root, t[0])
        if c is None:
            raise Unsupported('no file %s' % t[0])
        self.calls.add(t)
        return c.entry_label(t[1])

    # ---- one movement
    def movement(self, cmd, a):
        n = int(a[0], 0) if a and re.match(r'^(0x[0-9A-Fa-f]+|\d+)$', a[0]) else 1
        if cmd == 'EndMovement':
            return ['step_end']
        m = re.match(r'^(Delay)(\d+)$', cmd)
        if m:
            return DELAYS.get(int(m.group(2)), ['delay_16']) * n
        if cmd in MOVE_ONE:
            return MOVE_ONE[cmd] * n
        m = re.match(r'^(\w+?)(North|South|West|East)$', cmd)
        if m and m.group(1) in MOVE_KIND:
            return [MOVE_KIND[m.group(1)] + DIRS[m.group(2)]] * n
        raise Unsupported('movement %s' % cmd)

    def movement_label(self, name):
        if name not in self.labels:
            raise Unsupported('movement %s elsewhere' % name)
        for cmd, a in self.labels[name]:
            self.movement(cmd, a)
        self.moves.add(name)
        return '%s%s__%s' % (PREFIX, self.low, name)

    # ---- one command
    def command(self, cmd, a):
        out = []
        m = re.match(r'^(GoTo|Call)If(Eq|Ne|Lt|Le|Gt|Ge)$', cmd)
        if m:
            out += self.compare(a[0], a[1])
            out.append('%s_if %d, %s' % ('goto' if m.group(1) == 'GoTo' else 'call', COND[m.group(2)], self.label(a[2])))
            return out
        m = re.match(r'^(GoTo|Call)If(Set|Unset)$', cmd)
        if m:
            return ['%s_if_%s %s, %s' % (m.group(1).lower(), m.group(2).lower(), self.flag(a[0]), self.label(a[1]))]
        m = re.match(r'^(GoTo|Call)IfBadge(Acquired|NotAcquired)$', cmd)
        if m:
            return [self.sinnoh(SINNOH_BADGE_CHECK, self.value(a[0])),
                    '%s_if %d, %s' % ('goto' if m.group(1) == 'GoTo' else 'call', 1 if m.group(2) == 'Acquired' else 0,
                                      self.label(a[1]))]
        if cmd == 'CheckBadgeAcquired':
            return [self.sinnoh(SINNOH_BADGE_CHECK, self.value(a[0])), 'copyvar %s, VAR_RESULT' % self.var(a[1])]
        if cmd in ('End',):
            return ['end']
        if cmd in ('Return', 'ReturnCommonScript'):
            return ['return']
        if cmd == 'GoTo':
            return ['goto %s' % self.label(a[0])]
        if cmd == 'Call':
            return ['call %s' % self.label(a[0])]
        if cmd == 'CallCommonScript':
            return ['call %s' % self.common(int(a[0], 0))]
        if cmd in self.dec.common_macro:
            return ['call %s' % self.common(self.dec.common_macro[cmd])]
        if cmd == 'CompareVar':
            return self.compare(a[0], a[1])
        if cmd == 'CheckFlag':
            return ['checkflag %s' % self.flag(a[0])]
        if cmd in ('GoToIf', 'CallIf'):
            return ['%s_if %s, %s' % ('goto' if cmd == 'GoToIf' else 'call', self.value(a[0]), self.label(a[1]))]
        if cmd == 'SetVar':
            if self.is_var(a[1]):
                return ['copyvar %s, %s' % (self.var(a[0]), self.var(a[1]))]
            return ['setvar %s, %s' % (self.var(a[0]), self.value(a[1]))]
        if cmd in ('AddVar', 'SubVar'):
            if self.is_var(a[1]):
                raise Unsupported('%s by a variable' % cmd)
            return ['%s %s, %s' % (cmd.lower(), self.var(a[0]), self.value(a[1]))]
        if cmd == 'CopyVar':
            return ['copyvar %s, %s' % (self.var(a[0]), self.var(a[1]))]
        if cmd in ('SetFlag', 'ClearFlag'):
            return ['%s %s' % (cmd.lower(), self.flag(a[0]))]
        if cmd == 'LockAll':
            return ['lockall']
        if cmd == 'ReleaseAll':
            return ['releaseall']
        if cmd == 'FacePlayer':
            return ['faceplayer']
        if cmd == 'PlaySE':
            se = SE.get(a[0])
            return ['playse %s' % se] if se else []
        if cmd == 'WaitSE':
            return ['waitse']
        if cmd == 'PlayFanfare':
            return ['playfanfare %s' % FANFARE.get(a[0], 'MUS_OBTAIN_ITEM')]
        if cmd == 'WaitFanfare':
            return ['waitfanfare']
        if cmd == 'Message':
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd in ('WaitButton', 'WaitABXPadPress', 'WaitABPress', 'WaitABPressTime'):
            return ['waitbuttonpress']
        if cmd in ('CloseMessage', 'CloseMessageWithoutErasing'):
            return ['closemessage']
        if cmd == 'NPCMessage':
            return ['playse SE_SELECT', 'lockall', 'faceplayer', 'message %s' % self.text_label(a[0]), 'waitmessage',
                    'waitbuttonpress', 'closemessage', 'releaseall']
        if cmd == 'EventMessage':
            return ['playse SE_SELECT', 'lockall', 'message %s' % self.text_label(a[0]), 'waitmessage',
                    'waitbuttonpress', 'closemessage', 'releaseall']
        if cmd in ('ShowMapSign', 'ShowArrowSign', 'ShowLandmarkSign', 'ShowScrollingSign'):
            # (Platinum's signposts slide in with a picture of the sign; here: the sign's words in the message box)
            return ['lockall', 'message %s' % self.text_label(a[0]), 'waitmessage', 'waitbuttonpress', 'closemessage',
                    'releaseall']
        if cmd == 'BufferPlayerName':
            return [self.sinnoh(SINNOH_BUFFER_PLAYER, self.value(a[0]))]
        if cmd == 'BufferRivalName':
            return [self.sinnoh(SINNOH_BUFFER_RIVAL, self.value(a[0]))]
        if cmd == 'BufferCounterpartName':
            return [self.sinnoh(SINNOH_BUFFER_COUNTERPART, self.value(a[0]))]
        if cmd in ('BufferItemName', 'BufferItemNamePlural', 'BufferItemNameWithArticle'):
            return ['bufferitemname %s, %s' % (self.strvar(a[0]), self.arg(a[1]))]
        if cmd == 'BufferSpeciesName':
            return ['bufferspeciesname %s, %s' % (self.strvar(a[0]), self.arg(a[1]))]
        if cmd == 'BufferMoveName':
            return ['buffermovename %s, %s' % (self.strvar(a[0]), self.arg(a[1]))]
        if cmd == 'BufferPartyMonNickname':
            return ['bufferpartymonnick %s, %s' % (self.strvar(a[0]), self.arg(a[1]))]
        if cmd == 'BufferTMHMMoveName':
            return [self.sinnoh(SINNOH_BUFFER_TMHM_MOVE, self.value(a[0]), self.arg(a[1]))]
        if cmd == 'BufferPocketName':
            return [self.sinnoh(SINNOH_BUFFER_POCKET, self.value(a[0]), self.arg(a[1]))]
        if cmd == 'GetItemPocket':
            return [self.sinnoh(SINNOH_GET_POCKET, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'IsItemTMHM':
            return [self.sinnoh(SINNOH_IS_TMHM, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'CheckItemIsPlate':
            return ['setvar %s, 0' % self.var(a[1])]     # (this game has no ARCEUS plates to engrave)
        if cmd in ('CheckItem', 'CanFitItem'):
            op = 'checkitem' if cmd == 'CheckItem' else 'checkitemspace'
            return ['%s %s, %s' % (op, self.arg(a[0]), self.arg(a[1])), 'copyvar %s, VAR_RESULT' % self.var(a[2])]
        if cmd in ('AddItem', 'RemoveItem'):
            return ['%s %s, %s' % (cmd.lower(), self.arg(a[0]), self.arg(a[1])), 'copyvar %s, VAR_RESULT' % self.var(a[2])]
        if cmd == 'GoToIfCannotFitItem':
            return ['checkitemspace %s, %s' % (self.arg(a[0]), self.arg(a[1])), 'copyvar %s, VAR_RESULT' % self.var(a[2]),
                    'goto_if_eq VAR_RESULT, 0, %s' % self.label(a[3])]
        if cmd == 'ShowYesNoMenu':
            # (this game's YES is 1, Platinum's MENU_YES 0: the answer turned into Platinum's)
            return ['yesnobox 20, 8', self.sinnoh(SINNOH_YESNO, self.var(a[0]))]
        if cmd in ('InitGlobalTextMenu', 'InitLocalTextMenu', 'InitGlobalTextListMenu', 'InitLocalTextListMenu'):
            can_exit = self.value(a[4]) if len(a) > 4 else '1'
            return [self.sinnoh(SINNOH_MENU_INIT, self.var(a[3]), can_exit)]
        if cmd in ('AddMenuEntryImm', 'AddMenuEntry', 'AddListMenuEntry'):
            where = self.dec.message(self.bank, a[0])
            if where is None:
                raise Unsupported('menu entry %s' % a[0])
            value = a[1]          # (AddListMenuEntry's third is an alternative text)
            return [self.sinnoh(SINNOH_MENU_ADD, where[0], where[1], self.value(value))]
        if cmd in ('ShowMenu', 'ShowListMenu', 'ShowMenuMultiColumn'):
            return [self.sinnoh(SINNOH_MENU_SHOW)]
        if cmd in ('ScrCmd_32D', 'ScrCmd_338', 'ScrCmd_32E', 'ScrCmd_339', 'ScrCmd_332', 'LockObject', 'UnlockObject',
                   'SetMenuXOriginToRight', 'IncrementTrainerScore', 'IncrementTrainerScore2', 'IncrementGameRecord',
                   'IncrementGameRecordTwice', 'LoadDoorAnimation', 'PlayDoorOpenAnimation', 'PlayDoorCloseAnimation',
                   'WaitForAnimation', 'UnloadAnimation', 'UnloadDoorAnimation'):
            return []                       # (the walking partner paused / resumed, objects held: nothing to do here)
        if cmd == 'SetObjectEventMovementType':
            import sprites
            return ['setobjectmovementtype %s, %s' % (self.localid(a[0]), sprites.movement(a[1]))]
        if cmd == 'SetObjectEventDir':
            return [self.sinnoh(SINNOH_FACE, self.localid(a[0]) if not a[0].startswith('VAR_') else self.var(a[0]),
                                self.value(a[1]))]
        if cmd == 'GetFirstNonEggInParty':
            return [self.sinnoh(SINNOH_FIRST_NON_EGG, self.var(a[0]))]
        if cmd == 'CountAliveMonsExcept':
            return [self.sinnoh(SINNOH_COUNT_ALIVE_EXCEPT, self.var(a[0]), self.arg(a[1]))]
        if cmd in ('CheckPartyHasSpecies', 'CheckPartyHasSpecies2'):
            return [self.sinnoh(SINNOH_HAS_SPECIES, self.var(a[0]), self.arg(a[1]))]
        if cmd == 'FindPartySlotWithMove':
            return ['checkpartymove %s' % self.value(a[1]), 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'GetDayOfWeek':
            return [self.sinnoh(SINNOH_DAY_OF_WEEK, self.var(a[0]))]
        if cmd == 'GetTimeOfDay':
            return [self.sinnoh(SINNOH_TIME_OF_DAY, self.var(a[0]))]
        if cmd == 'CheckPlayerOnBike':
            return [self.sinnoh(SINNOH_ON_BIKE, self.var(a[0]))]
        if cmd == 'GetPlayerStarterSpecies':
            return [self.sinnoh(SINNOH_STARTER, self.var(a[0]))]
        if cmd == 'BufferNumber':
            return ['buffernumberstring %s, %s' % (self.strvar(a[0]), self.var(a[1]) if self.is_var(a[1]) else self.value(a[1]))]
        if cmd == 'GiveBadge':
            n = int(self.value(a[0]))
            return ['setflag 0x%X' % (0x3320 + n)]   # (FLAG_SINNOH_BADGE01 + n, include/mp_region.h)
        if cmd == 'StartLegendaryBattle':
            return ['setwildbattle %s, %s' % (self.value(a[0]), self.value(a[1])),
                    'special BattleSetup_StartLegendaryBattle', 'waitstate']
        if cmd == 'CheckGameCompleted':
            return [self.sinnoh(SINNOH_GAME_COMPLETED, self.var(a[0]))]
        if cmd == 'CheckDidNotCapture':
            return [self.sinnoh(SINNOH_DID_NOT_CAPTURE, self.var(a[0]))]
        if cmd == 'GoToIfInRange':
            # (a .. b, both in): two comparisons, the first skipping past the second
            skip = '%s%s__skip%d' % (PREFIX, self.low, self.next_local())
            return ['compare_var_to_value %s, %s' % (self.var(a[0]), self.value(a[1])), 'goto_if 0, %s' % skip,
                    'compare_var_to_value %s, %s' % (self.var(a[0]), self.value(a[2])), 'goto_if 3, %s' % self.label(a[3]),
                    '%s:' % skip]
        if cmd == 'HideObject':
            o = self.dec.origins().get(self.events_low)
            if o is None:
                raise Unsupported('objects on %s' % self.events_low)
            return ['hideobjectat %s, %s' % (self.localid(a[0]), o[0])]
        if cmd in ('PlayMusic', 'SetBGM'):
            mus = MUSIC_OF.get(a[0])
            if mus is None:
                raise Unsupported('music %s' % a[0])
            return ['playbgm %s, FALSE' % mus]
        if cmd in ('StopMusic', 'FadeOutBGM', 'FadeOutMusic'):
            return ['fadeoutbgm 1']
        if cmd in ('PlayDefaultMusic', 'RestoreDefaultBGM', 'FadeInBGM'):
            return ['fadedefaultbgm']
        if cmd == 'WaitABPadPress':
            return ['waitbuttonpress']
        if cmd == 'StartTrainerBattle':
            if len(a) > 1 and a[1] not in ('0',):
                raise Unsupported('StartTrainerBattle with two trainers')
            if self.is_var(a[0]):
                raise Unsupported('StartTrainerBattle by a variable')
            t = self.value(a[0])
            where = build_text.trainer_message(self.root, a[0], 'TRMSG_DEFEAT')
            if where is None:
                raise Unsupported('trainer %s has no defeat line' % a[0])
            lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
            self.texts[lab] = where
            return ['trainerbattle_no_intro %s, %s' % (t, lab)]
        if cmd == 'CheckWonBattle':
            return ['setvar %s, 1' % self.var(a[0])]   # (a battle lost whites out here: the script goes on after a win)
        if cmd == 'CheckLostBattle':
            return ['setvar %s, 0' % self.var(a[0])]
        if cmd == 'BlackOutFromBattle':
            return ['end']
        if cmd in ('CheckTrainerFlag', 'SetTrainerFlag', 'ClearTrainerFlag'):
            return ['%s %s' % (cmd.lower(), self.value(a[0]))]
        m = re.match(r'^GoToIf(NotDefeated|Defeated)$', cmd)
        if m:
            return ['checktrainerflag %s' % self.value(a[0]),
                    'goto_if %d, %s' % (0 if m.group(1) == 'NotDefeated' else 1, self.label(a[1]))]
        if cmd == 'PlayTrainerEncounterBGM':
            return []                       # (the battle plays its own)
        if cmd == 'GetRandom' or cmd == 'GetRandom2':
            return ['random %s' % self.value(a[1]), 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'HealParty':
            return ['special HealPlayerParty']
        if cmd == 'GetPartyCount':
            return ['getpartysize', 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'GetNationalDexEnabled':
            return [self.sinnoh(SINNOH_NATIONAL_DEX, self.var(a[0]))]
        if cmd == 'Warp':
            low = a[0][len('MAP_HEADER_'):].lower() if a[0].startswith('MAP_HEADER_') else None
            o = self.dec.origins().get(low)
            if o is None:
                raise Unsupported('warp to %s' % a[0])
            return ['warp %s, %d, %d' % (o[0], int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2])]
        if cmd == 'SetObjectEventPos':
            o = self.dec.origins().get(self.events_low)
            if o is None:
                raise Unsupported('positions on %s' % self.events_low)
            x, z = int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2]
            lid = self.localid(a[0])
            return ['setobjectxyperm %s, %d, %d' % (lid, x, z), 'setobjectxy %s, %d, %d' % (lid, x, z)]
        if cmd == 'GetPlayerGender':
            return ['checkplayergender', 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'GetPlayerDir':
            return [self.sinnoh(SINNOH_GET_DIR, self.var(a[0]))]
        if cmd == 'GetPlayerMapPos':
            return [self.sinnoh(SINNOH_GET_POS, self.var(a[0]), self.var(a[1]))]
        if cmd in ('PlayCry', 'PlayPokemonCry'):
            return ['playmoncry %s, CRY_MODE_NORMAL' % self.arg(a[0])]
        if cmd == 'WaitCry':
            return ['waitmoncry']
        if cmd == 'PokemonCryAndMessage':
            return ['playmoncry %s, CRY_MODE_NORMAL' % self.arg(a[0]), 'message %s' % self.text_label(a[1]),
                    'waitmessage', 'waitmoncry']
        if cmd == 'FadeScreenOut':
            return ['fadescreen FADE_TO_BLACK']
        if cmd == 'FadeScreenIn':
            return ['fadescreen FADE_FROM_BLACK']
        if cmd == 'WaitFadeScreen':
            return []                       # (fadescreen waits by itself)
        if cmd == 'ShowMoney':
            return ['showmoneybox 0, 0']
        if cmd == 'HideMoney':
            return ['hidemoneybox']
        if cmd == 'UpdateMoneyDisplay':
            return ['updatemoneybox']
        if cmd == 'ApplyMovement':
            return ['applymovement %s, %s' % (self.localid(a[0]), self.movement_label(a[1]))]
        if cmd == 'WaitMovement':
            return ['waitmovement 0']
        if cmd == 'RemoveObject':
            return ['removeobject %s' % self.localid(a[0])]
        if cmd == 'AddObject':
            return ['addobject %s' % self.localid(a[0])]
        if cmd in ('Delay8', 'Delay4', 'Delay2', 'Delay1', 'Delay16', 'Delay15', 'Delay32'):
            return ['delay %d' % (2 * int(cmd[len('Delay'):]))]
        if cmd == 'WaitTime':
            return ['delay %d' % (2 * int(self.value(a[0])))]
        if cmd in ('TurnOnTimeBasedEffects', 'CreateJournalEvent', 'TrySetUnusedCollectedOrbFlag', 'RecordHeapMemory',
                   'CheckHeapMemory', 'SetMenuXOriginToRight', 'SetMenuXOriginToLeft'):
            return []                       # (Platinum's journal, its memory checks: nothing here)
        raise Unsupported(cmd)

    # ---- reach and convert
    def reach(self, start):
        """the labels an entry runs: those it goes to or calls, and those it falls into"""
        seen, stack = [], [start]
        while stack:
            lab = stack.pop()
            if lab in seen:
                continue
            if lab not in self.labels:
                raise Unsupported('goes to %s (another file)' % lab)
            seen.append(lab)
            body = self.labels[lab]
            for cmd, a in body:
                if cmd == 'ApplyMovement':
                    continue
                for x in a:
                    if x in self.labels and x not in seen:
                        stack.append(x)
            if not body or body[-1][0] not in TERMINAL:
                i = self.order.index(lab)
                if i + 1 < len(self.order):
                    stack.append(self.order[i + 1])
        return seen

    def check(self, n, busy=None):
        """None if entry n (from 1) converts (and every common script it calls), else why not"""
        if n in self._ok:
            return self._ok[n]
        busy = busy or set()
        if (self.low, n) in busy:
            return None                     # (a loop through the common scripts: judged by its first visit)
        busy = busy | {(self.low, n)}
        why = None
        before = set(self.calls)
        try:
            if not 1 <= n <= len(self.entries):
                raise Unsupported('no entry %d' % n)
            for lab in self.reach(self.entries[n - 1]):
                for cmd, a in self.labels[lab]:
                    self.command(cmd, a)
            for low, m in sorted(self.calls - before):
                w = converter(self.root, low).check(m, busy)
                if w is not None:
                    raise Unsupported('common %s %d: %s' % (low, m, w))
        except Unsupported as e:
            why = str(e)
        except (IndexError, ValueError) as e:
            why = 'arguments: %s' % e
        self._ok[n] = why
        return why

    def emit(self, wanted):
        """this file's converted labels (the entries in WANTED that convert), in file order"""
        labels, entries, moves = set(), [], set()
        for n in sorted(wanted):
            if self.check(n) is None:
                labels |= set(self.reach(self.entries[n - 1]))
                entries.append(n)
        out = []
        for n in entries:
            out.append('%s::' % self.entry_label(n))
            out.append('\tgoto %s' % self.label(self.entries[n - 1]))
        for lab in self.order:
            if lab not in labels:
                continue
            out.append('%s:' % self.label(lab))
            for cmd, a in self.labels[lab]:
                if cmd == 'ApplyMovement':
                    moves.add(a[1])
                out += ['\t' + line for line in self.command(cmd, a)]
            body = self.labels[lab]
            if not body or body[-1][0] not in TERMINAL:
                i = self.order.index(lab)
                nxt = self.order[i + 1] if i + 1 < len(self.order) else None
                out.append('\tgoto %s' % self.label(nxt) if nxt in labels else '\tend')
        for lab in sorted(moves):
            out.append('%s%s__%s:' % (PREFIX, self.low, lab))
            for cmd, a in self.labels[lab]:
                out += ['\t' + line for line in self.movement(cmd, a)]
            if not self.labels[lab] or self.labels[lab][-1][0] != 'EndMovement':
                out.append('\tstep_end')
        return out, entries


# ----------------------------------------------------------------------------------------------------- all files

_converters = {}


def converter(root, low):
    """the FileConverter of Platinum's scripts_<low>.s (None: no such file)"""
    key = (root, low)
    if key not in _converters:
        path = os.path.join(root, 'res/field/scripts', 'scripts_%s.s' % low)
        dec = _converters.setdefault((root, None), Decomp(root)) if (root, None) not in _converters else _converters[(root, None)]
        _converters[key] = FileConverter(dec, path) if os.path.exists(path) else None
    return _converters[key]


def convertible(root, low, n):
    """the label a map's stub goes to when Platinum's entry N of scripts_<low>.s converts, else None"""
    c = converter(root, low)
    if c is None or c.check(n) is not None:
        return None
    return c.entry_label(n)


def referenced():
    """{SinnohP_ label} every map's scripts point at (data/maps/Sinnoh_*/scripts.inc)"""
    out = set()
    for path in glob.glob(os.path.join(ROOT, 'data/maps/Sinnoh_*/scripts.inc')):
        out |= set(re.findall(r'\b(%s\w+)' % PREFIX, open(path, encoding='utf-8').read()))
    return out


def text_bytes(b, m):
    return ', '.join('0x%02X' % x for x in build_text.sinnoh_text_bytes(b, m))


HEADER = ('@ SINNOH: Platinum\'s scripts as this game\'s (tools/sinnoh/gen_scripts.py, made from pret/pokeplatinum when '
          'the game is built: never committed)\n')


def write(lines, out_path):
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    text = HEADER + '\n'.join(lines) + '\n'
    if os.path.exists(out_path) and open(out_path, encoding='utf-8').read() == text:
        return
    open(out_path, 'w', encoding='utf-8').write(text)


def stub(out_path):
    lines = ['%s::\n\tend' % lab for lab in sorted(referenced())]
    write(lines, out_path)
    print('%s: %d stubs (no decomp)' % (os.path.relpath(out_path, ROOT), len(referenced())))


def build(root, out_path):
    refs = referenced()
    wanted = collections.defaultdict(set)
    for lab in refs:
        m = re.match(r'^%s(\w+?)_(\d+)$' % PREFIX, lab)
        if m:
            wanted[m.group(1)].add(int(m.group(2)))
    # the common scripts the converted ones call, and the ones those call
    queue = [(low, n) for low in wanted for n in wanted[low]]
    while queue:
        low, n = queue.pop()
        c = converter(root, low)
        if c is None or c.check(n) is not None:
            continue
        before = set(c.calls)
        for lab in c.reach(c.entries[n - 1]):
            for cmd, a in c.labels[lab]:
                c.command(cmd, a)
        for t in c.calls:
            if t[1] not in wanted[t[0]]:
                wanted[t[0]].add(t[1])
                queue.append(t)
    lines, texts, done = [], {}, set()
    for low in sorted(wanted):
        c = converter(root, low)
        if c is None:
            continue
        body, entries = c.emit(wanted[low])
        lines += body
        texts.update(c.texts)
        done |= {c.entry_label(n) for n in entries}
    for lab in sorted(refs - done):
        print('gen_scripts: %s no longer converts: it ends' % lab, file=sys.stderr)
        lines.append('%s::\n\tend' % lab)
    for lab, (b, m) in sorted(texts.items()):
        lines.append('%s:\n\t.byte %s' % (lab, text_bytes(b, m)))
    write(lines, out_path)
    print('%s: %d scripts, %d texts' % (os.path.relpath(out_path, ROOT), len(done), len(texts)))


def report(root):
    why = collections.Counter()
    ok = total = 0
    for path in sorted(glob.glob(os.path.join(root, 'res/field/scripts/scripts_*.s'))):
        low = os.path.basename(path)[len('scripts_'):-2]
        c = converter(root, low)
        for n, name in enumerate(c.entries, 1):
            if re.search(r'OnTransition|OnFrame|OnLoad|OnResume|Init', name):
                continue
            total += 1
            w = c.check(n)
            if w is None:
                ok += 1
            else:
                why[w.split(' ')[0] if not w.startswith(('flag', 'variable', 'value')) else w] += 1
    print('%d of %d script entries convert' % (ok, total))
    for w, k in why.most_common(60):
        print('  %5d  %s' % (k, w))


def main():
    a = sys.argv[1:]
    if a and a[0] == '--auto':
        mark = os.path.join(ROOT, 'sinnoh', 'decomp_dir')
        root = open(mark).read().strip() if os.path.exists(mark) else ''
        if root and os.path.isdir(os.path.join(root, 'res/field/scripts')):
            build(root, a[1] if len(a) > 1 else OUT)
        else:
            stub(a[1] if len(a) > 1 else OUT)
    elif a and a[0] == '--stub':
        stub(a[1] if len(a) > 1 else OUT)
    elif a and a[0] == '--report':
        report(a[1])
    elif a:
        build(a[0], a[1] if len(a) > 1 else OUT)
    else:
        print(__doc__)
        sys.exit(1)


if __name__ == '__main__':
    main()
