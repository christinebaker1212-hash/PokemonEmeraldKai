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
TERMINAL = {'End', 'Return', 'GoTo', 'EndMovement'}

# this game's SINNOH script command (data/script_cmd_table.inc 0xF6 `sinnoh OP, ARG, ARG2`, src/sinnoh/s3d_scrcmd.c)
SINNOH_BUFFER_PLAYER, SINNOH_BUFFER_RIVAL, SINNOH_BUFFER_COUNTERPART, SINNOH_YESNO = 1, 2, 3, 4
SINNOH_GET_DIR, SINNOH_GET_POS, SINNOH_GET_POCKET, SINNOH_IS_TMHM, SINNOH_BUFFER_TMHM_MOVE = 5, 6, 7, 8, 9
SINNOH_BUFFER_POCKET, SINNOH_BADGE_CHECK, SINNOH_NATIONAL_DEX = 10, 11, 12
SINNOH_MENU_INIT, SINNOH_MENU_ADD, SINNOH_MENU_SHOW = 13, 14, 15
SINNOH_FIRST_NON_EGG, SINNOH_COUNT_ALIVE_EXCEPT, SINNOH_HAS_SPECIES, SINNOH_DAY_OF_WEEK = 16, 17, 18, 19
SINNOH_TIME_OF_DAY, SINNOH_ON_BIKE, SINNOH_STARTER, SINNOH_FACE = 20, 21, 22, 23
SINNOH_GAME_COMPLETED, SINNOH_DID_NOT_CAPTURE, SINNOH_GET_HOUR, SINNOH_SET_WARP = 24, 25, 26, 27
SINNOH_SET_BIKE, SINNOH_PARTY_SPECIES, SINNOH_HELD_ITEM, SINNOH_OBJECT_POS, SINNOH_TWO_TRAINERS = 28, 29, 30, 31, 32
SINNOH_SET_BG, SINNOH_BUFFER_RIVAL_STARTER, SINNOH_MOVEMENT_TYPE, SINNOH_LOCAL_DEX, SINNOH_STOP_MUSIC = 33, 34, 35, 36, 37
SINNOH_BATTLE_WON, SINNOH_ADD_VAR, SINNOH_BADGE_COUNT, SINNOH_PREVIOUS_MAP, SINNOH_DAYCARE_EGG = 38, 39, 40, 41, 42
SINNOH_BUFFER_TEXT, SINNOH_QUERY, SINNOH_DEX_RATING, SINNOH_MESSAGE_VAR = 43, 44, 45, 46
SINNOH_TV, SINNOH_MESSAGE_BANK_VAR, SINNOH_SEAL, SINNOH_FASHION, SINNOH_CONTEST, SINNOH_WARP_CONTINUE = 47, 48, 49, 50, 51, 52
SINNOH_LIFT = 53   # (the platform lifts: src/sinnoh/s3d_lift.c)   # (SINNOH's TV: src/sinnoh/s3d_tv.c TVCALL_*)
(TVCALL_STATUS, TVCALL_FRAMING, TVCALL_FINISH, TVCALL_SEGMENT_ID, TVCALL_SEGMENT, TVCALL_COMMERCIAL, TVCALL_NEXT,
 TVCALL_INTERVIEW_ID, TVCALL_INTERVIEW_MESSAGE, TVCALL_INTERVIEW_SAVE, TVCALL_INTERVIEW_OK, TVCALL_WORD,
 TVCALL_HIDDEN_ITEM, TVCALL_NAME_RATED, TVCALL_STORAGE, TVCALL_VILLA, TVCALL_AMITY, TVCALL_QUESTION) = range(18)
TV_FRAMING = {'LoadTVGreeting': 0, 'LoadTVSegmentIntro': 1, 'LoadTVFarewell': 2, 'LoadTVFarewellExtended': 3}
# Platinum's POKéDEX ratings (its src/unk_0205DFC4.c Pokedex_GetRatingMessageID_*): (up to this many, message) in
# pokedex_ratings' bank; the last rows are for a complete dex. National: (caught up to, message, female's message)
DEX_RATING_LOCAL = [(15, 'RowanPokemonSeenUpTo15'), (30, 'RowanPokemonSeenOver15'), (45, 'RowanPokemonSeenOver30'),
                    (60, 'RowanPokemonSeenOver45'), (80, 'RowanPokemonSeenOver60'), (100, 'RowanPokemonSeenOver80'),
                    (120, 'RowanPokemonSeenOver100'), (140, 'RowanPokemonSeenOver120'), (160, 'RowanPokemonSeenOver140'),
                    (180, 'RowanPokemonSeenOver160'), (200, 'RowanPokemonSeenOver180'), (209, 'RowanPokemonSeenOver200'),
                    (0xFFFF, 'RowanCompleteLocalDex')]
DEX_RATING_NATIONAL = [(39, 'OakPokemonCaughtUnder40'), (59, 'OakPokemonCaught40'), (89, 'OakPokemonCaught60'),
                       (119, 'OakPokemonCaught90'), (149, 'OakPokemonCaught120'), (189, 'OakPokemonCaught150'),
                       (229, 'OakPokemonCaught190'), (269, 'OakPokemonCaught230'), (309, 'OakPokemonCaught270'),
                       (349, 'OakPokemonCaught310'), (379, 'OakPokemonCaught350'), (409, 'OakPokemonCaught380'),
                       (429, 'OakPokemonCaught410_Male', 'OakPokemonCaught410_Female'), (449, 'OakPokemonCaught430'),
                       (459, 'OakPokemonCaught450'), (469, 'OakPokemonCaught460'), (475, 'OakPokemonCaught470'),
                       (481, 'OakPokemonCaught476'),
                       (0xFFFF, 'OakCompleteNationalDex_Male', 'OakCompleteNationalDex_Female')]
VAR_SINNOH_STARTER = 0x52F8      # (include/mp_region.h: the starter picked from ROWAN's briefcase)
VAR_SINNOH_SPIRITOMB = 0x52F2    # (include/mp_region.h: Platinum's SPIRITOMB counter, friends talked to in the Underground)
FLAG_SINNOH_HIDE_REGION_BOAT = 0x3D3D   # (include/mp_region.h: CANALAVE's ferry sailor, shown when the boat can sail)
FLAG_SINNOH_HAS_PARTNER = 0x3D3C   # (include/mp_region.h: Platinum's walking partner, SetHasPartner)

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

CONSTANTS = {'TRUE': 1, 'FALSE': 0, 'NO_EXIT_ON_B': 0, 'EXIT_ON_B': 1, 'MENU_YES': 0, 'MENU_NO': 1, 'GENDER_MALE': 0, 'GENDER_FEMALE': 1}

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
            'FaceAwayFromPlayer': ['face_away_player'],
            'WarpOut': ['fly_up', 'set_invisible'], 'WarpIn': ['set_visible', 'fly_down']}   # (Platinum's spin up and out of
            # sight, as the Underground's ROARK leaves: the game's own rising)
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
MISSING_MUSIC = set()   # Platinum's songs its scripts play that the game lacks (--report lists them)


class Unsupported(Exception):
    pass


def _sprite(gfx):
    """this game's sprite for a Platinum one (tools/sinnoh/sprites.py); the player's looks of the other gender are the
    counterpart's (DAWN, LUCAS)"""
    import sprites
    gfx = {'OBJ_EVENT_GFX_PLAYER_F': 'OBJ_EVENT_GFX_DAWN', 'OBJ_EVENT_GFX_PLAYER_M': 'OBJ_EVENT_GFX_LUCAS'}.get(gfx, gfx)
    m = re.match(r'^OBJ_EVENT_GFX_VAR_([0-9A-F])$', gfx)
    if m:
        return 'OBJ_EVENT_GFX_VAR_%s' % m.group(1)
    ours = sprites.sprite(gfx)
    if ours is None:
        raise Unsupported('sprite %s' % gfx)
    return ours


# ----------------------------------------------------------------------------------------------------- the decomp

def parse(path):
    """{label: [(command, [args])]}, the entries (ScriptEntry names in order), the labels in file order"""
    labels, entries, order, cur = {}, [], [], None
    defines = {}
    for raw in open(path, encoding='utf-8', errors='replace'):
        line = raw.split('//')[0].split('@')[0].rstrip()   # (both kinds of comments)
        m = re.match(r'^#define\s+(\w+)\s+(\S+)\s*$', line)
        if m:
            defines[m.group(1)] = m.group(2)                 # (a file's own names: LOCAL_VAR_X VAR_0x8004)
            continue
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
            labels[cur].append((cmd, [defines.get(a.strip(), a.strip()) for a in args.split(',')] if args else []))
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
            self._origins, self.names = {}, {}
            self.places = list(places(open_source(self.root), self.root))
            for p in self.places:
                low = p['place']['header'][len('MAP_HEADER_'):].lower()
                self._origins.setdefault(low, (p['id'], p['x0'] * 32, p['z0'] * 32))
                self.names.setdefault(p['id'], p['name'])
        return self._origins

    def trainer(self, x):
        """this game's constant for a Platinum trainer (gen_trainers.py's)"""
        ours = 'TRAINER_SINNOH_' + x[len('TRAINER_'):]
        return ours if ours in self.trainers else None

    def fashion_ids(self):
        """Platinum's ACCESSORY_* (include/constants/accessories.h) and BACKDROP_* (generated/backdrops.txt) numbers"""
        if getattr(self, '_fashion', None) is None:
            self._fashion = {}
            t = open(os.path.join(self.root, 'include/constants/accessories.h')).read()
            for enum in re.findall(r'enum\s*\w*\s*\{([^}]*)\}', t):
                n = 0
                for item in enum.split(','):
                    item = re.sub(r'//.*', '', item).strip()
                    if not item:
                        continue
                    name, _, val = item.partition('=')
                    n = int(val.strip(), 0) if val.strip() else n
                    self._fashion[name.strip()] = n
                    n += 1
            path = os.path.join(self.root, 'generated/backdrops.txt')
            if os.path.exists(path):
                for i, name in enumerate(x.strip() for x in open(path) if x.strip()):
                    self._fashion[name] = i
        return self._fashion

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


# Platinum's script numbers past a map's own (src/script_manager.c SCRIPT_RANGE_TABLE, include/constants/scrcmd.h):
# (first number, file). Trainers' (3000 / 5000), items' (7000 / 8000) and BERRY trees' (2800) are the game's own
SCRIPT_RANGES = [(2000, 'common'), (2500, 'bg_events'), (8800, 'safari_game'), (8900, 'record_chatot_cry'),
                 (8950, 'vs_seeker'), (8970, 'poke_radar'), (9000, 'pokemon_center_2f_common'),
                 (9100, 'communication_club'), (9200, 'pokemon_center_b1f_common'), (9300, 'group_connection'),
                 (9400, 'poffin_common'), (9500, 'day_care_common'), (9600, 'init_new_game'),
                 (9700, 'follower_partners'), (9800, 'contests'), (9900, 'unused_0397'), (9950, 'pokedex_ratings'),
                 (10000, 'field_moves'), (10100, 'tv_broadcast'), (10150, 'tv_reporter_interviews'),
                 (10200, 'mystery_gift_deliveryman'), (10300, 'counterpart_talk'),
                 (10400, 'pokemon_center_daily_trainers'), (10450, 'battle_frontier_records'),
                 (10490, 'scratch_off_cards')]
SCRIPT_RANGES_END = {2000: 2500, 2500: 2800}


def common_target(n):
    """(file, entry from 1) of Platinum's script number N past the map's own, None for the game's own kinds"""
    for k, (first, low) in enumerate(SCRIPT_RANGES):
        end = SCRIPT_RANGES_END.get(first, SCRIPT_RANGES[k + 1][0] if k + 1 < len(SCRIPT_RANGES) else 10600)
        if first <= n < end:
            return low, n - first + 1
    return None


SHARED_EVENTS = {'contests': ['contest_hall_lobby', 'contest_hall_stage_ongoing_contest']}


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
        # (a file shared by maps names their people: the contest hall's scripts, its lobby's and stage's)
        for low in ([m.group(1)] if m else []) + SHARED_EVENTS.get(self.low, []):
            ev = os.path.join(self.root, 'res/field/events', 'events_%s.json' % low)
            if os.path.exists(ev):
                for i, o in enumerate(json.load(open(ev)).get('object_events', [])):
                    if isinstance(o.get('id'), str):
                        self.localids.setdefault(o['id'], i + 1)
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
            x = {'ITEM_SECRETPOTION': 'ITEM_SECRET_POTION', 'ITEM_BLACKGLASSES': 'ITEM_BLACK_GLASSES',
                 'ITEM_SILVERPOWDER': 'ITEM_SILVER_POWDER', 'ITEM_NEVERMELTICE': 'ITEM_NEVER_MELT_ICE',
                 'ITEM_THUNDERSTONE': 'ITEM_THUNDER_STONE'}.get(x, x)
            if x in self.dec.ours:
                return x
            raise Unsupported('value %s (not in this game)' % x)
        if x.startswith(('ACCESSORY_', 'BACKDROP_')):
            n = self.dec.fashion_ids().get(x)   # (Platinum's own numbers: src/sinnoh/s3d_fashion.c)
            if n is None:
                raise Unsupported('value %s' % x)
            return str(n)
        if x.startswith('OBJ_EVENT_GFX_'):
            return _sprite(x)
        if x.startswith('MAP_HEADER_'):
            o = self.dec.origins().get(x[len('MAP_HEADER_'):].lower())
            if o is None:
                raise Unsupported('map %s (not in this game)' % x)
            return o[0]
        if x.startswith('LOCALID_') and x in self.localids:
            return str(self.localids[x])
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
        if x == 'VAR_PARTNER_TRAINER_ID':
            return '0x52FA'           # (VAR_SINNOH_PARTNER_TRAINER: the walking partner's team, battle_setup.c)
        m = re.match(r'^VAR_OBJ_GFX_ID_([0-9A-F])$', x)
        if m:
            return '0x%X' % (0x4010 + int(m.group(1), 16))   # (the game's own VAR_OBJ_GFX_ID_0-F: a person's look)
        m = re.match(r'^VAR_MAP_LOCAL_0x([0-9A-Fa-f]+)$', x)
        if m and int(m.group(1), 16) <= 0xE:
            return '0x%X' % (0x4000 + int(m.group(1), 16))   # (the map's own: VAR_TEMP_0-E, cleared as a map loads)
        if re.match(r'^VAR_DUMMY_0x', x):
            return '0x400F'           # (VAR_TEMP_F: Platinum's unused variables)
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
        if re.match(r'^FLAG_(DUMMY|UNUSED)_0x', x):
            return '0'                # (Platinum's unused flags: nothing reads them)
        m = re.match(r'^FLAG_MAP_LOCAL_0x([0-9A-Fa-f]+)$', x)
        if m:
            n = int(m.group(1), 16)
            if not 1 <= n <= 0x1F:
                raise Unsupported('flag %s' % x)
            return '0x%X' % n         # (the map's own flags: this game's FLAG_TEMP_1-1F, cleared as a map loads)
        f = ids.flag(self.root, x) if x.startswith('FLAG_') else None
        if f is None:
            raise Unsupported('flag %s' % x)
        return '0x%X' % f

    def localid(self, x):
        if x in ('LOCALID_PLAYER', 'OBJ_EVENT_ID_PLAYER'):
            return 'OBJ_EVENT_ID_PLAYER'
        if x == 'LOCALID_FOLLOWER':
            return '0xF2'                 # (the walking partner on a map it walked into: s3d_scrcmd.c Sinnoh_PartnerUpdate)
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
        if cmd in ('PlayerGive', 'PlayerReceive', 'PauseAnimation', 'ResumeAnimation', 'MoveAction_153'):
            return []                       # (the player's give / receive poses, an object's paused frames)
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
        if cmd in ('Common_ObtainUndergroundTrap', 'Common_ObtainUndergroundSphere', 'Common_SendToUndergroundPCWithLinefeed',
                   'Common_SendToUndergroundPC'):
            # (traps, spheres and secret base goods: the Underground's sides CO-OP supersedes, closed; the user,
            # 2026-11-02: the Underground is here for its digging and SPIRITOMB only)
            return []
        if cmd == 'CallCommonScript':
            return ['call %s' % self.common(int(a[0], 0))]
        if cmd == 'Common_SaveGame':
            # (the game's own save prompt; Platinum's answer is left in VAR_MAP_LOCAL_0x00: CommonScript_SaveGame)
            return ['special SaveGame', 'waitstate', 'copyvar %s, VAR_RESULT' % self.var('VAR_MAP_LOCAL_0x00')]
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
        if cmd in ('AddVar', 'SubVar') and self.is_var(a[1]):
            return [self.sinnoh(SINNOH_ADD_VAR, self.var(a[0]), self.var(a[1]), 1 if cmd == 'SubVar' else 0)]
        if cmd in ('AddVar', 'SubVar'):
            return ['%s %s, %s' % (cmd.lower(), self.var(a[0]), self.value(a[1]))]
        if cmd == 'CopyVar':
            return ['copyvar %s, %s' % (self.var(a[0]), self.var(a[1]))]
        if cmd == 'GetSpiritombCounter':
            # (the Underground's talks: src/sinnoh/s3d_underground.c counts them, co-op friends being Platinum's people)
            return ['copyvar %s, 0x%X' % (self.var(a[0]), VAR_SINNOH_SPIRITOMB)]
        if cmd == 'ClearSpiritombCounter':
            return ['setvar 0x%X, 0' % VAR_SINNOH_SPIRITOMB]
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
        if cmd == 'MessageSynchronized':
            cmd = 'Message'                 # (a link contest's message, said to each player at once: one player)
        if cmd == 'Message':
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd in ('WaitButton', 'WaitABXPadPress', 'WaitABPress', 'WaitABPressTime'):
            return ['waitbuttonpress']
        if cmd in ('CloseMessage', 'CloseMessageWithoutErasing'):
            return ['closemessage']
        if cmd == 'UndergroundNPCMessage':
            # (the Underground's own message box, which names the player itself: the game's)
            return [self.sinnoh(SINNOH_BUFFER_PLAYER, 0), 'message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd == 'CloseUndergroundNPCMessage':
            return ['closemessage']
        if cmd == 'SetCommPlayerDir':
            # (the player turned, Platinum's Underground player being a link one)
            return [self.sinnoh(SINNOH_FACE, 'LOCALID_PLAYER', self.value(a[0]))]
        if cmd == 'InitPersistedMapFeaturesForPlatformLift':
            return [self.sinnoh(SINNOH_LIFT, 0)]
        if cmd == 'CheckPlatformLiftNotUsedWhenEnteredMap':
            return [self.sinnoh(SINNOH_LIFT, 1, self.var(a[0]))]
        if cmd == 'TriggerPlatformLift':
            return [self.sinnoh(SINNOH_LIFT, 2)]   # (the script waits while it rides)
        if cmd == 'Dummy14F':
            return []
        if cmd == 'CheckHasRoomForGoodsInPC':
            return ['setvar %s, 1' % self.var(a[2])]
        if cmd == 'GetCapturedFlagCount':
            return ['setvar %s, 0' % self.var(a[0])]   # (Capture the Flag: online, closed)
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
            out = ['%s %s, %s' % (cmd.lower(), self.arg(a[0]), self.arg(a[1])), 'copyvar %s, VAR_RESULT' % self.var(a[2])]
            if cmd == 'AddItem':
                out.append('special MP_GatchaTicketToast')   # (Kai's MOEGATCHA TICKET for a story item, as Std_ObtainItem)
            return out
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
        if cmd in ('SetObjectEventMovementType', 'SetMovementType'):
            # (now, as Platinum's; MOVEMENT_TYPE_FOLLOW_PLAYER makes it the walking partner: s3d_scrcmd.c)
            import sprites
            kind = a[1] if a[1] == 'MOVEMENT_TYPE_FOLLOW_PLAYER' else sprites.movement(a[1])
            return [self.sinnoh(SINNOH_MOVEMENT_TYPE, self.localid(a[0]), kind)]
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
        if cmd == 'StartWildBattle':
            # (a POKéMON met in a script, as any wild one: ROUTE 209's SPIRITOMB, the OLD CHATEAU's ROTOM)
            return ['setwildbattle %s, %s' % (self.value(a[0]), self.value(a[1])),
                    'special BattleSetup_StartScriptedWildBattle', 'waitstate']
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
                MISSING_MUSIC.add(a[0])
                return []                   # (a song the game doesn't have yet: docs/SINNOH_HANDOFF.md's list)
            return ['playbgm %s, FALSE' % mus]
        if cmd == 'StopMusic' or (cmd in ('FadeOutBGM', 'FadeOutMusic') and int(self.value(a[0])) == 0):
            # (the song stops, as Platinum's: not the game's fadeoutbgm, which only pauses it for a fadeinbgm)
            frames = int(self.value(a[1])) if cmd != 'StopMusic' and len(a) > 1 else 0
            return [self.sinnoh(SINNOH_STOP_MUSIC, frames)]
        if cmd in ('FadeOutBGM', 'FadeOutMusic', 'FadeInBGM'):
            return []                       # (the song quieter for a while, and back: left as it is)
        if cmd in ('PlayDefaultMusic', 'RestoreDefaultBGM'):
            return ['fadedefaultbgm']
        if cmd == 'WaitABPadPress':
            return ['waitbuttonpress']
        if cmd == 'StartTrainerBattle' and len(a) > 1 and a[1] not in ('0',):
            # two at once: the game's own battle against two trainers who spotted the player (battle_setup.c
            # MP_StartTwoTrainerBattle), each with Platinum's defeat line
            out = []
            for k in (0, 1):
                where = build_text.trainer_message(self.root, a[k], 'TRMSG_DEFEAT')
                if where is None or self.is_var(a[k]):
                    raise Unsupported('trainer %s has no defeat line' % a[k])
                lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
                self.texts[lab] = where
                out.append('loadword %d, %s' % (k, lab))
            return out + [self.sinnoh(SINNOH_TWO_TRAINERS, self.value(a[0]), self.value(a[1]))]
        if cmd == 'StartTagBattle':
            # beside a partner: the game's own multi battle (Kai's, as STEVEN's and LANCE's: battle_tower.c
            # MP_StartPartnerBattle, DoSpecialTrainerBattle 15): the half of the party that battles picked first, the
            # party back as it was after (with what the battle changed), a loss whites out
            n = self.next_local()
            pick, go, won = ['%s%s__tag%d_%s' % (PREFIX, self.low, n, x) for x in ('pick', 'go', 'won')]
            keep = ('copyvar 0x400E, %s' if self.is_var(a[0]) else 'setvar 0x400E, %s') % self.arg(a[0])
            return [keep, 'special SavePlayerParty', '%s:' % pick, 'fadescreen FADE_TO_BLACK',
                    'special ChooseHalfPartyForBattle', 'waitstate', 'goto_if_ne VAR_RESULT, 0, %s' % go,
                    'special LoadPlayerParty', 'special SavePlayerParty', 'goto %s' % pick, '%s:' % go,
                    'special ReducePlayerPartyToSelectedMons', 'frontier_set FRONTIER_DATA_SELECTED_MON_ORDER',
                    'copyvar VAR_0x8005, 0x400E',   # (the party pick and frontier_set use VAR_0x8004-5: kept in VAR_TEMP_E)
                    'setvar VAR_0x8006, %s' % self.value(a[1]), 'setvar VAR_0x8007, %s' % self.value(a[2]),
                    'setvar VAR_0x8004, 15', 'special DoSpecialTrainerBattle', 'waitstate', 'frontier_saveparty',
                    'special LoadPlayerParty', 'goto_if_eq VAR_RESULT, B_OUTCOME_WON, %s' % won,
                    'fadescreen FADE_TO_BLACK', 'special SetCB2WhiteOut', 'waitstate', '%s:' % won]
        if cmd == 'StartTrainerBattle':
            if self.is_var(a[0]):
                raise Unsupported('StartTrainerBattle by a variable')
            t = self.value(a[0])
            where = build_text.trainer_message(self.root, a[0], 'TRMSG_DEFEAT')
            if where is None:
                raise Unsupported('trainer %s has no defeat line' % a[0])
            lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
            self.texts[lab] = where
            return ['trainerbattle_no_intro %s, %s' % (t, lab)]
        if cmd in ('CheckWonBattle', 'CheckLostBattle'):
            return [self.sinnoh(SINNOH_BATTLE_WON, self.var(a[0]), 1 if cmd == 'CheckLostBattle' else 0)]
        if cmd == 'StartFirstBattle':
            # BARRY's first battle: Kai's early rival battle (Quetzal's trainerbattle 13: a loss doesn't white out,
            # the party is healed after), Platinum's defeat line
            where = build_text.trainer_message(self.root, a[0], 'TRMSG_DEFEAT')
            if where is None:
                raise Unsupported('trainer %s has no defeat line' % a[0])
            lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
            self.texts[lab] = where
            return ['.byte SCR_OP_TRAINERBATTLE, 13', '.2byte %s, 1' % self.value(a[0]), 'ptrvalue %s' % lab,
                    'ptrvalue %s' % lab]
        if cmd == 'BlackOutFromBattle':
            return ['special SetCB2WhiteOut', 'waitstate']   # (the game's own whiteout)
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
            # (Platinum's script goes on on the new map: src/sinnoh/s3d_scrcmd.c SINNOH_WARP_CONTINUE)
            go = '%s%s__warped%d' % (PREFIX, self.low, self.next_local())
            return [self.sinnoh(SINNOH_WARP_CONTINUE), 'goto %s' % go,
                    'warp %s, %d, %d' % (o[0], int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2]), 'waitstate',
                    '%s:' % go]
        if cmd == 'SetObjectEventPos' and (self.is_var(a[1]) or self.is_var(a[2])):
            # (a place the script worked out: Platinum's map tiles, made this map's by the game)
            return [self.sinnoh(SINNOH_OBJECT_POS, self.localid(a[0]), self.arg(a[1]), self.arg(a[2]))]
        if cmd == 'SetObjectEventPos':
            o = self.dec.origins().get(self.events_low)
            if o is None:
                raise Unsupported('positions on %s' % self.events_low)
            x, z = int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2]
            lid = self.localid(a[0])
            return ['setobjectxyperm %s, %d, %d' % (lid, x, z), 'setobjectxy %s, %d, %d' % (lid, x, z)]
        if cmd == 'SetPosition' and (self.is_var(a[1]) or self.is_var(a[3])):
            # (a place the script worked out, as SetObjectEventPos's: the Underground's ROARK beside the player)
            lid = self.localid(a[0])
            return [self.sinnoh(SINNOH_OBJECT_POS, lid, self.arg(a[1]), self.arg(a[3])),
                    self.sinnoh(SINNOH_FACE, lid, self.value(a[4]))]
        if cmd == 'SetPosition':
            o = self.dec.origins().get(self.events_low)
            if o is None:
                raise Unsupported('positions on %s' % self.events_low)
            x, z = int(self.value(a[1])) - o[1], int(self.value(a[3])) - o[2]
            lid = self.localid(a[0])
            return ['setobjectxyperm %s, %d, %d' % (lid, x, z), 'setobjectxy %s, %d, %d' % (lid, x, z),
                    self.sinnoh(SINNOH_FACE, lid, self.value(a[4]))]
        if cmd == 'SetWarpEventPos':
            n = self.our_warp(int(self.value(a[0])))
            o = self.dec.origins()[self.events_low]
            return [self.sinnoh(SINNOH_SET_WARP, n, int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2])]
        if cmd == 'SetBgEventPos':
            n = self.our_event(int(self.value(a[0])), 'bg_events')
            o = self.dec.origins()[self.events_low]
            return [self.sinnoh(SINNOH_SET_BG, n, int(self.value(a[1])) - o[1], int(self.value(a[2])) - o[2])]
        if cmd in ('SetHasPartner', 'ClearHasPartner'):
            return ['%sflag 0x%X' % ('set' if cmd == 'SetHasPartner' else 'clear', FLAG_SINNOH_HAS_PARTNER)]
        if cmd == 'AddFreeCamera':
            # the game's own camera object (the 3D view follows it: src/sinnoh/s3d.c SetupView), where Platinum's is
            return ['special SpawnCameraObject',
                    self.sinnoh(SINNOH_OBJECT_POS, 'OBJ_EVENT_ID_CAMERA', self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'ApplyFreeCameraMovement':
            return ['applymovement OBJ_EVENT_ID_CAMERA, %s' % self.movement_label(a[0])]
        if cmd == 'RestoreCamera':
            return ['special RemoveCameraObject']
        if cmd == 'StartChooseStarterScene':
            # Kai's SINNOH briefcase (src/sinnoh/s3d_places.c Sinnoh_StarterMenu: Platinum's screen from the graphics
            # pack); Platinum's can't be backed out of
            again = '%s%s__starter%d' % (PREFIX, self.low, self.next_local())
            return ['%s:' % again, 'special Sinnoh_StarterMenu', 'waitstate', 'goto_if_eq VAR_RESULT, 0, %s' % again]
        if cmd == 'SaveChosenStarter':
            return ['copyvar 0x%X, VAR_RESULT' % VAR_SINNOH_STARTER, 'setflag 0x3D39']   # (FLAG_SINNOH_STARTER)
        if cmd == 'ReturnToField':
            return []                       # (the briefcase went back to the field itself)
        if cmd == 'BufferRivalStarterSpeciesName':
            return [self.sinnoh(SINNOH_BUFFER_RIVAL_STARTER, self.value(a[0]))]
        if cmd == 'GiveRunningShoes':
            return ['setflag FLAG_SYS_B_DASH']
        if cmd == 'GiveJournal':
            return ['additem ITEM_JOURNAL', 'special MP_GatchaTicketToast']
        if cmd in ('InitSizeContestRecord', 'RandomizeJubilifeLottery', 'InitDailyRandomLevel', 'SetStepFlag'):
            return []
        if cmd == 'SetObjectFlagIsPersistent':
            return []                       # (this game's objects keep their flags anyway)
        if cmd in ('GetLocalDexSeenCount', 'GetLocalDexCaughtCount', 'CheckLocalDexCompleted'):
            mode = {'GetLocalDexSeenCount': 0, 'GetLocalDexCaughtCount': 1, 'CheckLocalDexCompleted': 2}[cmd]
            return [self.sinnoh(SINNOH_LOCAL_DEX, self.var(a[0]), mode)]
        if cmd == 'GivePokemon':
            # (Platinum's success: TRUE unless there's no room anywhere)
            n = self.next_local()
            done = '%s%s__given%d' % (PREFIX, self.low, n)
            item = self.arg(a[2]) if len(a) > 2 else 'ITEM_NONE'
            return ['givemon %s, %s, %s' % (self.arg(a[0]), self.arg(a[1]), item), 'setvar %s, 1' % self.var(a[3]),
                    'goto_if_ne VAR_RESULT, MON_CANT_GIVE, %s' % done, 'setvar %s, 0' % self.var(a[3]), '%s:' % done]
        if cmd == 'EnablePoketch':
            return ['additem ITEM_POKETCH', 'special MP_GatchaTicketToast']   # (Kai's POKéTCH is the key item: src/sinnoh/poketch.c)
        if cmd in ('SetPlayerState', 'ChangePlayerState'):
            return []                       # (the player's poses)
        # SUPER CONTESTS (src/sinnoh/s3d_contest.c CC_*; the screens s3d_contest_ui.c): the hall's own commands
        contest = {'EndContest': (2,), 'BufferJudgeName': (3, 'a', 'a'), 'BufferContestantTrainerName': (4, 'a', 'a'),
                   'BufferContestantMonName': (5, 'a', 'a'), 'BufferContestRegistrationEntryNumber': (6, 'a', 'a'),
                   'BufferContestRank': (7, 'a'), 'BufferContestType': (8, 'a'),
                   'BufferWinningContestantTrainerName': (9, 'a'), 'BufferWinningContestantMonName': (10, 'a'),
                   'GetPlayerContestPlacement': (11, 'v'), 'GetWinningContestantEntryNum': (12, 'v'),
                   'GetContestRegistrationEntryNum': (13, 'v'), 'GetContestantObjEventGFX': (14, 'a', 'v'),
                   'GetContestantMonContestFame': (15, 'a', 'v'), 'GetContestMode': (16, 'v'),
                   'CheckPlayerMonHasRibbon': (19, 'v'), 'SetRibbonName': (20, 'a'),
                   'GetFirstTimeVictoryAccessory': (21, 'v'), 'GetShouldSkipAwardCeremony': (22, 'v'),
                   'GetContestPartyMenuResult': (25, 'v', 'v'), 'RunContestApplication': (26,),
                   'ScrCmd_0A8': (27, 'a', 'v')}
        if cmd in contest:
            spec = contest[cmd]
            args = [self.arg(x) if k == 'a' else self.var(x) for k, x in zip(spec[1:], a)]
            if cmd == 'EndContest':
                args = []
            return [self.sinnoh(SINNOH_CONTEST, spec[0], *args)]
        if cmd == 'ScrCmd_0A6':
            # (JUBILIFE TV's dress-up studio: the contest's dress-up screen without its timer)
            return [self.sinnoh(SINNOH_CONTEST, 28, self.var(a[0]), self.var(a[1]))]
        if cmd == 'ShowDressUpPhoto':
            return [self.sinnoh(SINNOH_CONTEST, 29, self.value(a[0]), self.var(a[1]))]
        if cmd == 'SetDressUpPhotoTitle':
            return [self.sinnoh(SINNOH_CONTEST, 30, self.var(a[0]))]
        if cmd == 'BufferCustomMessageWord':
            return [self.sinnoh(SINNOH_CONTEST, 31, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'SelectMoveTutorPokemon':
            # (Platinum's "choose a POKeMON": the game's party menu; VAR_0x8004 it answers in is kept)
            return [self.sinnoh(SINNOH_CONTEST, 32), 'special ChoosePartyMon', 'waitstate', self.sinnoh(SINNOH_CONTEST, 33)]
        if cmd == 'GetSelectedPartySlot':
            return [self.sinnoh(SINNOH_CONTEST, 34, self.var(a[0]))]
        if cmd == 'GetPartyMonFriendship':
            return [self.sinnoh(SINNOH_CONTEST, 35, self.var(a[0]), self.arg(a[1]))]
        if cmd in ('InitPersistedMapFeaturesForPastoriaGym', 'InitPersistedMapFeaturesForSunyshoreGym',
                   'InitPersistedMapFeaturesForCanalaveGym', 'InitPersistedMapFeaturesForEternaGym'):
            return []                       # (src/sinnoh/s3d_gyms.c sets its gyms up as their maps load)
        if cmd == 'AdvanceEternaGymClock':
            return ['special Sinnoh_EternaClock', 'waitstate']   # (s3d_gyms.c: the hands to the state its trainers say)
        if cmd == 'PressPastoriaGymButton':
            return ['special Sinnoh_PastoriaWater', 'waitstate']   # (src/sinnoh/s3d_gyms.c: the button under the player)
        if cmd == 'PressSunyshoreGymButton':
            return ['special Sinnoh_SunyshoreTurn', 'waitstate']
        if cmd == 'GetNationalDexCaughtCount':
            return [self.sinnoh(SINNOH_CONTEST, 36, self.var(a[0]))]
        if cmd == 'CheckNationalDexCompleted':
            return [self.sinnoh(SINNOH_CONTEST, 37, self.var(a[0]))]
        if cmd == 'CountPartyNonEggs':
            return [self.sinnoh(SINNOH_CONTEST, 38, self.var(a[0]))]
        if cmd == 'Dummy316':
            return []
        if cmd == 'NewContest':
            return [self.sinnoh(SINNOH_CONTEST, 0, self.arg(a[0]), self.arg(a[1])),
                    self.sinnoh(SINNOH_CONTEST, 1, self.arg(a[2]), self.arg(a[3]))]
        if cmd == 'GetContestInfo':
            return [self.sinnoh(SINNOH_CONTEST, 17, self.var(a[0]), self.var(a[1])),
                    self.sinnoh(SINNOH_CONTEST, 18, self.var(a[2]), self.var(a[3]))]
        if cmd == 'OpenPartyMenuForContest':
            # (the game's party menu: it answers in VAR_0x8004, where the hall keeps the rank: kept and put back)
            return [self.sinnoh(SINNOH_CONTEST, 23, self.arg(a[1]), self.arg(a[2])), 'special ChoosePartyMon',
                    'waitstate', self.sinnoh(SINNOH_CONTEST, 24)]
        if cmd in ('StartBattleClient', 'StartBattleServer') and self.low in ('contests', 'poffin_common'):
            # (COMM_CLUB_RET_CANCEL: link contests and group cooking come with co-op)
            return ['setvar %s, 1' % self.var(a[-1])]
        if cmd == 'WaitForLinkContestSetup':
            return []
        if cmd in ('SetMonSummary', 'GetMonPartySlot'):
            return []                       # (after the party menu's summary: the game's menu shows it itself)
        if cmd in ('StartContestCommSync', 'WaitForCommSyncState', 'LockAutoScrollForLinkContests', 'LockTextSpeed',
                   'StopHBlank', 'StartHBlank', 'ChangeIntoContestAttire', 'StartContestCameraFlashTask',
                   'WaitForContestCameraFlashTask', 'ScrCmd_2B0', 'ScrCmd_2BB', 'DestroyNetworkIcon', 'EndCommunication',
                   'HidePoketch', 'ShowPoketch'):
            return []                       # (wireless syncs, the stage's light and camera flashes, the costume)
        # Platinum's FASHION CASE (src/sinnoh/s3d_fashion.c)
        if cmd == 'AddAccessory':
            return [self.sinnoh(SINNOH_FASHION, 0, self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'RemoveAccessory':
            return [self.sinnoh(SINNOH_FASHION, 8, self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'CanFitAccessory':
            return ['setorcopyvar 0x400F, %s' % self.arg(a[1]), self.sinnoh(SINNOH_FASHION, 1, self.arg(a[0]), self.var(a[2]))]
        if cmd in ('BufferAccessoryName', 'BufferAccessoryNameWithArticle'):
            return [self.sinnoh(SINNOH_FASHION, 2 if cmd == 'BufferAccessoryName' else 3, self.value(a[0]), self.arg(a[1]))]
        if cmd == 'AddContestBackdrop':
            return [self.sinnoh(SINNOH_FASHION, 4, self.arg(a[0]))]
        if cmd == 'CheckBackdrop':
            return [self.sinnoh(SINNOH_FASHION, 5, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'BufferContestBackdropName':
            return [self.sinnoh(SINNOH_FASHION, 6, self.value(a[0]), self.arg(a[1]))]
        # Platinum's POFFINS: this game's POKéBLOCKS (src/sinnoh/s3d_fashion.c)
        if cmd == 'CheckHasEmptyPoffinCaseSlot':
            return [self.sinnoh(SINNOH_FASHION, 9, self.var(a[0]))]
        if cmd == 'GetEmptyPoffinCaseSlotCount':
            return [self.sinnoh(SINNOH_FASHION, 10, self.var(a[0]))]
        if cmd == 'GivePoffin':
            return [self.sinnoh(SINNOH_FASHION, 11, k, self.arg(a[1 + k])) for k in range(6)] + \
                   [self.sinnoh(SINNOH_FASHION, 12, self.var(a[0]))]
        if cmd == 'CheckCanCookPoffin':
            return [self.sinnoh(SINNOH_FASHION, 15, self.var(a[0]))]
        if cmd == 'OpenPoffinCooking':
            # (the POFFIN HOUSE's cooking is the game's BERRY BLENDER: POFFINS are POKéBLOCKS; alone means with the
            # blender's one computer partner, the fewest it takes; in a group comes with co-op)
            return ['setvar VAR_0x8004, 1', 'special DoBerryBlending', 'waitstate']
        if cmd == 'TryGetRandomMassageGirlAccessory':
            return [self.sinnoh(SINNOH_FASHION, 7, self.var(a[0]))]
        if cmd == 'BufferPartyMonSpecies':
            return [self.sinnoh(SINNOH_PARTY_SPECIES, self.arg(a[1]), '0x400F'),
                    'bufferspeciesname %s, 0x400F' % self.strvar(a[0])]
        if cmd == 'SetNationalDexEnabled':
            return ['special EnableNationalPokedex']
        if cmd == 'RemoveMoney2' or cmd == 'RemoveMoney':
            if self.is_var(a[0]):
                raise Unsupported('money by a variable')
            return ['removemoney %s' % self.value(a[0])]
        if cmd in ('AddMoney',):
            if self.is_var(a[0]):
                raise Unsupported('money by a variable')
            return ['addmoney %s' % self.value(a[0])]
        if cmd == 'GetPreviousMapID':
            return [self.sinnoh(SINNOH_PREVIOUS_MAP, self.var(a[0]))]
        if cmd == 'CheckDaycareHasEgg':
            return [self.sinnoh(SINNOH_DAYCARE_EGG, self.var(a[0]))]
        if cmd == 'CapitalizeFirstLetter':
            return []                       # (the game's names are in capitals already)
        if cmd == 'BufferPoketchAppName':
            # (Platinum's app names, from the text pack: Kai's POKéTCH has the same apps)
            bank = self.dec.banks['poketch_app_names']
            return [self.sinnoh(SINNOH_BUFFER_TEXT, self.value(a[0]), bank, self.arg(a[1]))]
        if cmd == 'StartDestroyObstacleAnimation':
            return ['applymovement %s, SinnohP_Movement_SmashRock' % self.localid(a[0])]   # (the game's ROCK SMASH)
        if cmd == 'AddCameraOverrideObject':
            return ['special SpawnCameraObject',
                    self.sinnoh(SINNOH_OBJECT_POS, 'OBJ_EVENT_ID_CAMERA', self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'RemoveCameraOverrideObject':
            return ['special RemoveCameraObject']
        if cmd in ('CheckHasTwoAliveMons', 'GetNationalDexSeenCount', 'CheckHasAllLegendaryTitansInParty',
                   'GetPCBoxesFreeSlotCount', 'GetLeagueVictories'):
            kind = ['CheckHasTwoAliveMons', 'GetNationalDexSeenCount', 'CheckHasAllLegendaryTitansInParty',
                    'GetPCBoxesFreeSlotCount', 'GetLeagueVictories'].index(cmd)
            return [self.sinnoh(SINNOH_QUERY, self.var(a[0]), kind)]
        if cmd == 'FindPartySlotWithSpecies':
            return [self.sinnoh(SINNOH_QUERY, self.var(a[0]), 5, self.arg(a[1]))]
        if cmd in ('GetPartyMonForm', 'GetPartyMonForm2'):
            return [self.sinnoh(SINNOH_QUERY, self.var(a[1]), 6, self.arg(a[0]))]
        if cmd == 'OpenPokemonNamingScreen':
            # (the game's own nickname screen)
            return ['setvar VAR_0x8004, %s' % self.value(a[0]) if not self.is_var(a[0]) else 'copyvar VAR_0x8004, %s' % self.var(a[0]),
                    'special ChangePokemonNickname', 'waitstate', 'setvar %s, 1' % self.var(a[1])]
        if cmd == 'CheckIsTodayPlayerBirthday':
            return ['setvar %s, 0' % self.var(a[0])]   # (the game keeps no birthday)
        if cmd == 'SetCyclingBGM':
            return []                       # (Kai's OPTIONS > BIKE MUSIC)
        if cmd == 'LoadPokedexRating' or cmd in ('LoadLocalDexRating', 'LoadNationalDexRating'):
            national = cmd == 'LoadNationalDexRating' or (cmd == 'LoadPokedexRating' and int(self.value(a[0])) != 0)
            return [self.sinnoh(SINNOH_DEX_RATING, self.var(a[-1]), 1 if national else 0)]
        if cmd == 'MessageVar':
            # (a line of this file's bank chosen by a variable)
            if self.bank is None:
                raise Unsupported('MessageVar without a bank')
            return [self.sinnoh(SINNOH_MESSAGE_VAR, self.dec.banks[self.bank], self.var(a[0])),
                    'message gSinnohMessage', 'waitmessage']
        # SINNOH's TV (src/sinnoh/s3d_tv.c: Platinum's broadcast, its reporters' interviews and what they record)
        if cmd == 'CheckBroadcastStatus':
            return [self.sinnoh(SINNOH_TV, TVCALL_STATUS, self.var(a[0]))]
        if cmd in TV_FRAMING:
            return [self.sinnoh(SINNOH_TV, TVCALL_FRAMING | TV_FRAMING[cmd] << 8, self.var(a[0]), self.var(a[1]))]
        if cmd == 'LoadTVCommercial':
            return [self.sinnoh(SINNOH_TV, TVCALL_COMMERCIAL, self.var(a[0]), self.var(a[1]))]
        if cmd == 'LoadTVSegment':
            return [self.sinnoh(SINNOH_TV, TVCALL_SEGMENT_ID, self.var(a[0])),
                    self.sinnoh(SINNOH_TV, TVCALL_SEGMENT, self.var(a[1]), self.var(a[2]))]
        if cmd == 'GetNextTVSegmentID':
            return [self.sinnoh(SINNOH_TV, TVCALL_NEXT, self.var(a[0]))]
        if cmd == 'FinishTVProgram':
            return [self.sinnoh(SINNOH_TV, TVCALL_FINISH)]
        if cmd == 'TVBroadcastDummy':
            return []
        if cmd == 'MessageFromBank':
            return [self.sinnoh(SINNOH_MESSAGE_BANK_VAR, self.var(a[0]), self.var(a[1])),
                    'message gSinnohMessage', 'waitmessage']
        if cmd == 'CheckTVInterviewEligible':
            return [self.sinnoh(SINNOH_TV, TVCALL_INTERVIEW_OK, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'LoadTVInterviewMessage':
            return [self.sinnoh(SINNOH_TV, TVCALL_INTERVIEW_ID, self.var(a[0])),
                    self.sinnoh(SINNOH_TV, TVCALL_INTERVIEW_MESSAGE, self.var(a[1]), self.var(a[2]))]
        if cmd == 'SaveTVInterviewResponse':
            return [self.sinnoh(SINNOH_TV, TVCALL_INTERVIEW_SAVE, self.var(a[0]), self.var(a[1]))]
        if cmd == 'ChooseCustomMessageWord':
            # (one word on the game's own easy chat: GABBY and TY's "Interview" screen)
            return ['special Sinnoh_TvChooseWord', 'waitstate',
                    self.sinnoh(SINNOH_TV, TVCALL_WORD, self.var(a[0]), self.var(a[1]))]
        if cmd == 'SaveTVSegmentHiddenItem':
            return [self.sinnoh(SINNOH_TV, TVCALL_HIDDEN_ITEM, self.arg(a[0]))]
        if cmd == 'ScrCmd_2B8':
            return [self.sinnoh(SINNOH_TV, TVCALL_NAME_RATED, self.var(a[0]))]
        if cmd == 'SaveTVSegmentPokemonStorageBulletin':
            return [self.sinnoh(SINNOH_TV, TVCALL_STORAGE)]
        if cmd == 'SaveTVSegmentHomeAndManorNoFurniture':
            return ['setvar 0x400F, 0', self.sinnoh(SINNOH_TV, TVCALL_VILLA, '0x400F', 1)]
        if cmd == 'SaveTVSegmentHomeAndManor':
            return [self.sinnoh(SINNOH_TV, TVCALL_VILLA, self.arg(a[0]), 0)]
        if cmd == 'ScrCmd_27C':
            return [self.sinnoh(SINNOH_TV, TVCALL_AMITY, self.value(a[0]), self.var(a[1]))]
        # Seals: this game's POKé BALL effects in their place (src/sinnoh/s3d_seals.c; the user, 2026-11-02)
        if cmd == 'GiveOrTakeSeal':
            return [self.sinnoh(SINNOH_SEAL, 0, self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'CountSealOccurence':
            return [self.sinnoh(SINNOH_SEAL, 1, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'CountUniqueSealsInSealCase':
            return [self.sinnoh(SINNOH_SEAL, 2, self.var(a[0]))]
        if cmd in ('BufferBallSealName', 'BufferBallSealNamePlural'):
            return [self.sinnoh(SINNOH_SEAL, 3, self.value(a[0]), self.arg(a[1]))]
        if cmd == 'PokeMartSeal':
            day = re.match(r'^MART_SEAL_ID_SUNYSHORE_(\w+)$', a[0])
            days = ['MONDAY', 'TUESDAY', 'WEDNESDAY', 'THURSDAY', 'FRIDAY', 'SATURDAY', 'SUNDAY']
            if not day or day.group(1) not in days:
                raise Unsupported('Seal shop %s' % a[0])
            return ['setvar VAR_0x8004, %d' % days.index(day.group(1)), 'call EventScript_SinnohSealShop']
        if cmd == 'OpenSealCapsuleEditor':
            return []                       # (the BALL CAPSULES: the summary's POKé BALL page, as in every region)
        if cmd == 'StartCatchingTutorial':
            # (Emerald's WALLY tutorial with the counterpart and a BIDOOF: battle_setup.c Sinnoh_CatchingTutorial)
            return ['special SavePlayerParty', 'special Sinnoh_CatchingTutorial', 'waitstate', 'special LoadPlayerParty']
        if cmd == 'CountBadgesAcquired':
            return [self.sinnoh(SINNOH_BADGE_COUNT, self.var(a[0]))]
        if cmd == 'GoToIfNotEnoughMoney':
            if self.is_var(a[0]):
                raise Unsupported('money by a variable')
            return ['checkmoney %s' % self.value(a[0]), 'goto_if_eq VAR_RESULT, FALSE, %s' % self.label(a[1])]
        if cmd in ('DressUpPhotoHasData', 'ContestPhotoHasData'):
            return [self.sinnoh(SINNOH_FASHION, 13 if cmd == 'DressUpPhotoHasData' else 14, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'CheckPartyHasBadEgg':
            return [self.sinnoh(SINNOH_QUERY, self.var(a[0]), 8)]
        if cmd == 'GetCurrentMapID':
            return [self.sinnoh(SINNOH_QUERY, self.var(a[0]), 9)]
        if cmd in ('ClearInCatchingShowFlag', 'SetClearInCatchingShowFlag'):
            return []                       # (the PAL PARK's catching show: closed)
        if cmd == 'CheckMoney2':
            return [self.sinnoh(SINNOH_QUERY, self.var(a[0]), 7, self.arg(a[1]))]
        if cmd == 'AddToGameRecord':
            return []                       # (Platinum's game records: the game keeps its own)
        if cmd == 'FindPartySlotWithFatefulEncounterSpecies':
            return ['setvar %s, 6' % self.var(a[0])]   # (none: distribution events are closed)
        if cmd == 'GiveEgg':
            return ['giveegg %s' % self.arg(a[0])]
        if cmd in ('RegisterPoketchApp', 'ScrCmd_2CD', 'ScrCmd_331'):
            return []                       # (Kai's POKéTCH has its apps by badges: src/sinnoh/poketch.c)
        if cmd == 'CheckPoketchEnabled':
            return ['checkitem ITEM_POKETCH, 1', 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'CheckPoketchAppRegistered':
            return ['checkitem ITEM_POKETCH, 1', 'copyvar %s, VAR_RESULT' % self.var(a[1])]
        if cmd == 'GetPartyMonSpecies':
            return [self.sinnoh(SINNOH_PARTY_SPECIES, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'CheckPartyHasHeldItem':
            return [self.sinnoh(SINNOH_HELD_ITEM, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'SetPlayerBike':
            return [self.sinnoh(SINNOH_SET_BIKE, self.value(a[0]))]

        if cmd == 'CheckDistributionEvent':
            return ['setvar %s, 0' % self.var(a[1])]   # (no distribution events: Platinum without them)
        if cmd == 'MessageInstant':
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd == 'GetHour':
            return [self.sinnoh(SINNOH_GET_HOUR, self.var(a[0]))]
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
        if cmd == 'FadeScreen' and len(a) == 4 and a[2].startswith('FADE_TYPE_'):
            # (Platinum's typed fades: even types fade out, odd ones in: generated/fade_types.txt)
            kinds = [x.strip() for x in open(os.path.join(self.root, 'generated/fade_types.txt')) if x.strip()]
            out = a[2] in kinds and kinds.index(a[2]) % 2 == 0
            white = a[3] == 'COLOR_WHITE'
            return ['fadescreen %s' % ('FADE_TO_' if out else 'FADE_FROM_') + ('WHITE' if white else 'BLACK')]
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
        if cmd in ('StopSE', 'WaitForTransition', 'SetInitialVolumeForSequence', 'SetBlackOutWarpId', 'ScrCmd_29F'):
            return []                       # (sounds stop by themselves here; the game picks the heal spot itself)
        if cmd in ('TurnOnTimeBasedEffects', 'CreateJournalEvent', 'TrySetUnusedCollectedOrbFlag', 'RecordHeapMemory',
                   'CheckHeapMemory', 'SetMenuXOriginToRight', 'SetMenuXOriginToLeft'):
            return []                       # (Platinum's journal, its memory checks: nothing here)
        raise Unsupported(cmd)

    def our_warp(self, n):
        return self.our_event(n, 'warp_events')

    def our_event(self, n, kind):
        """this game's number of Platinum's warp / bg event N on this file's map (the same tile: tools/sinnoh/gen_maps.py
        keeps only some)"""
        from places import events
        o = self.dec.origins().get(self.events_low)
        p = next((q for q in self.dec.places if q['id'] == (o or [None])[0]), None)
        if p is None:
            raise Unsupported('warps on %s' % self.events_low)
        ws = events(self.root, p['place'])[kind]
        ours = _our_map(p['name'])
        if ours is None or not 0 <= n < len(ws):
            raise Unsupported('%s %d' % (kind, n))
        x, y = ws[n]['x'] - o[1], ws[n]['z'] - o[2]
        for i, w in enumerate(ours.get(kind, [])):
            if (w['x'], w['y']) == (x, y):
                return i
        raise Unsupported('%s %d (left out here)' % (kind, n))

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
                if cmd in ('ApplyMovement', 'ApplyFreeCameraMovement'):
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
                if cmd == 'ApplyFreeCameraMovement':
                    moves.add(a[0])
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


# ----------------------------------------------------------------------------------------------------- map scripts

# Platinum's init script kinds -> this game's map script kinds (include/constants/map_scripts.h)
INIT_KIND = {'OnTransition': 'MAP_SCRIPT_ON_TRANSITION', 'OnLoad': 'MAP_SCRIPT_ON_LOAD', 'OnResume': 'MAP_SCRIPT_ON_RESUME'}


def init_scripts(root, low):
    """Platinum's scripts_init_<low>.s: ({kind: entry}, [(variable, value, entry)] its frame table)"""
    path = os.path.join(root, 'res/field/scripts', 'scripts_init_%s.s' % low)
    once, frame = {}, []
    if not os.path.exists(path):
        return once, frame
    for raw in open(path, encoding='utf-8', errors='replace'):
        line = raw.split('//')[0].strip()
        m = re.match(r'^InitScriptEntry_(OnTransition|OnLoad|OnResume)\s+(\d+)$', line)
        if m:
            once[m.group(1)] = int(m.group(2))
        m = re.match(r'^InitScriptGoToIfEqual\s+(\w+),\s*(\w+),\s*(\d+)$', line)
        if m:
            frame.append((m.group(1), m.group(2), int(m.group(3))))
    return once, frame


def our_map_scripts(name):
    """this game's own map script table of map NAME (data/maps/NAME/scripts.inc): [(kind, label)] and, for the frame
    tables, {label: [(var, value, label)]}"""
    path = os.path.join(ROOT, 'data/maps', name, 'scripts.inc')
    if not os.path.exists(path):
        return [], {}
    lines = [ln.split('@')[0].strip() for ln in open(path, encoding='utf-8')]
    out, tables = [], {}
    try:
        i = lines.index('%s_MapScripts::' % name) + 1
    except ValueError:
        return [], {}
    while i < len(lines):
        m = re.match(r'^map_script\s+(\w+),\s*(\w+)$', lines[i])
        if not m:
            break
        out.append((m.group(1), m.group(2)))
        i += 1
    for kind, lab in out:
        if kind not in ('MAP_SCRIPT_ON_FRAME_TABLE', 'MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE'):
            continue
        rows = []
        j = next((k for k, ln in enumerate(lines) if re.match(r'^%s::?$' % re.escape(lab), ln)), None)
        while j is not None and j + 1 < len(lines):
            j += 1
            m = re.match(r'^map_script_2\s+(\w+),\s*(\w+),\s*(\w+)$', lines[j])
            if not m:
                break
            rows.append(m.groups())
        tables[lab] = rows
    return out, tables


def _target(root, low, s):
    """(file, entry) Platinum's script number S of map file LOW runs, None for none / a trainer"""
    if not isinstance(s, int) or isinstance(s, bool):
        return None
    t = common_target(s)
    if t:
        return t
    return (low, s) if 1 <= s < 2000 else None


def map_tables(root, wanted):
    """[(map constant, Platinum low, {kind: entry}, [(var, value, entry)], place)] of every SINNOH map; adds every entry
    its map scripts, people and triggers run to WANTED (so they are converted)"""
    from places import events
    dec = converter(root, 'common').dec
    dec.origins()
    out, seen = [], set()
    for p in dec.places:
        low = p['place']['header'][len('MAP_HEADER_'):].lower()
        if p['id'] in seen or converter(root, low) is None:
            continue
        seen.add(p['id'])
        once, frame = init_scripts(root, low)
        for n in list(once.values()) + [f[2] for f in frame]:
            wanted[low].add(n)
        ev = events(root, p['place'])
        for o in ev['object_events'] + ev['coord_events']:
            t = _target(root, low, o.get('script'))
            if t:
                wanted[t[0]].add(t[1])
        out.append((p['id'], low, once, frame, p))
    return out


def _our_map(name):
    path = os.path.join(ROOT, 'data/maps', name, 'map.json')
    return json.load(open(path, encoding='utf-8')) if os.path.exists(path) else None


def _stub_body(name, label):
    """the lines of a map's own script LABEL (data/maps/NAME/scripts.inc), up to its first `end`"""
    path = os.path.join(ROOT, 'data/maps', name, 'scripts.inc')
    text = open(path, encoding='utf-8').read() if os.path.exists(path) else ''
    m = re.search(r'^%s::\n((?:\t.*\n)*?)\tend\n' % re.escape(label), text, re.M)
    return m.group(1) if m else None


# a person's script of this game's that Platinum's story replaces (its words, or Platinum's own script already); the
# rest (MOéMON CENTER nurses, clerks, gym leaders, sailors, place systems) stay the game's
# this game's written stand-ins for Platinum's story (tools/sinnoh/legends.py, story_people.py, sinnoh_places.inc):
# Platinum's own scripts take their place where they convert. Kai's systems (frontier counters, prize exchanges, boats,
# swarms, the move deleter...) stay
STAND_INS = re.compile(r'^Sinnoh_EventScript_(Briefcase|RowanLab|DistortionEnter|Legend\w+|Story\d+)$')


# Kai's systems shown by Kai's own rules, whatever Platinum's story does with the person (the ferry between the regions:
# FLAG_SINNOH_HIDE_REGION_BOAT, shown while the boat can sail, as JOHTO's and KANTO's ports)
KAI_ALWAYS = re.compile(r'^\tgoto (EventScript_SinnohRegionBoat)\n$')


def _replaceable(body):
    if body is None:
        return True
    if re.match(r'^\tgoto SinnohP_\w+\n$', body):
        return True
    m = re.match(r'^\tgoto (\w+)\n$', body)
    if m and STAND_INS.match(m.group(1)):
        return True
    if re.match(r'^\ttrainerbattle_single TRAINER_SINNOH_\w+, \w+_Text_\w+', body):
        return True                         # (a gym leader / the ELITE FOUR / the CHAMPION in written words)
    if re.match(r'^\tmsgbox \w+_Text_\w+, MSGBOX_(NPC|SIGN)\n$', body):
        return True
    return bool(re.match(r'^\tlock\n\tfaceplayer\n\tgoto_if_set 0x[0-9A-F]+, \w+_Given\n', body))   # (a written gift)


def people(root, p, low, name, done):
    """Platinum's story on map NAME: [(local id, hide flag, x, y, script label or None: the map's own)] for its people,
    and its triggers as [(x, y, var, value, script)] (Platinum's whose script converts, each tile of their area, and the
    map's own where Platinum has none)"""
    from places import events
    from gen_maps import inside
    import sprites
    c = converter(root, low)
    ours = _our_map(name)
    if ours is None:
        return [], []
    ev = events(root, p['place'])
    objs = []
    for i, o in enumerate(ev['object_events']):
        if i >= len(ours['object_events']):
            break
        mine = ours['object_events'][i]
        hide = o.get('hidden_flag')
        flag = ids.flag(root, hide) if isinstance(hide, str) and hide.startswith('FLAG_') else None
        t = _target(root, low, o.get('script'))
        lab = converter(root, t[0]).entry_label(t[1]) if t and converter(root, t[0]) else None
        if lab not in done:
            lab = None
        ghost = isinstance(hide, str) and hide.startswith('MAP_HEADER_')
        if ghost or sprites.sprite(o['graphics_id']) is None or not inside(p, o['x'], o['z']) \
                or o['graphics_id'].endswith('POKEBALL'):
            continue
        script = mine['script']
        person = script == '%s_EventScript_Person%d' % (name, i + 1) or script == '0x0'
        trainer = isinstance(o.get('script'), str)
        if lab and STAND_INS.match(script):
            new = lab                      # (a legendary, a story battle: Platinum's scene)
        elif person and lab and _replaceable(_stub_body(name, script) if script != '0x0' else None):
            new = lab
        elif person and script == '0x0':
            continue                       # (nothing to say: stays as it is)
        elif trainer or person or STAND_INS.match(script):
            new = None                     # (the game's own script: a trainer's battle, a role, a stand-in not yet replaced)
        else:
            continue                       # (an obstacle, an item: the game's own systems)
        gfx = 'OBJ_EVENT_GFX_VAR_%s' % o['graphics_id'][-1] if re.match(r'^OBJ_EVENT_GFX_VAR_[0-9A-F]$', o['graphics_id']) \
            else '0'                       # (a look the script picks: the game's VAR_OBJ_GFX_ID_n)
        if new is None and person and script != '0x0' and KAI_ALWAYS.match(_stub_body(name, script) or ''):
            objs.append((i + 1, FLAG_SINNOH_HIDE_REGION_BOAT, mine['x'], mine['y'], None, '0'))
            continue
        objs.append((i + 1, flag or 0, o['x'] - p['x0'] * 32, o['z'] - p['z0'] * 32, new, gfx))
    trig, covered = [], set()
    for k, e in enumerate(ev['coord_events']):
        t = _target(root, low, e.get('script'))
        lab = converter(root, t[0]).entry_label(t[1]) if t and converter(root, t[0]) else None
        if lab not in done or not isinstance(e.get('var'), str) or not inside(p, e['x'], e['z']):
            continue
        try:
            var = c.var(e['var'])
            val = c.value(str(int(e['value']))) if isinstance(e['value'], (bool, int)) else c.arg(str(e['value']))
        except Unsupported:
            continue
        for dz in range(max(1, e.get('length', 1))):
            for dx in range(max(1, e.get('width', 1))):
                x, y = e['x'] - p['x0'] * 32 + dx, e['z'] - p['z0'] * 32 + dz
                trig.append((x, y, var, val, lab))
                covered.add((x, y))
    for e in ours.get('coord_events', []):
        if e.get('type') == 'trigger' and (e['x'], e['y']) not in covered and e['var'] != '0x52F0':
            trig.append((e['x'], e['y'], e['var'], e['var_value'], e['script']))
    return objs, trig


def emit_map_tables(root, tables, done):
    """gSinnohPlatinumMapScripts (src/sinnoh/s3d_scrcmd.c Sinnoh_PlatinumMapScripts): each map's own map scripts with
    Platinum's added: its transition / load / resume scripts after the map's own, its frame table's rows after the
    map's own rows (a row whose script doesn't convert is left out: a script that never changed its variable would run
    every frame). Only while the story is Platinum's (FLAG_SINNOH_PLATINUM_STORY)"""
    lines, index, skipped = [], [], 0
    for mapid, low, once, frame, p in tables:
        name = p['name']
        if not os.path.isdir(os.path.join(ROOT, 'data/maps', name)):
            continue
        ours, our_tables = our_map_scripts(name)
        c = converter(root, low)
        base = '%s%s_Map' % (PREFIX, low)
        objs, trig = people(root, p, low, name, done)
        if objs:
            lines += ['\t.balign 8', '%s_People:' % base]
            for lid, flag, x, y, lab, gfx in objs:
                lines += ['\t.byte %d, 0' % lid, '\t.2byte 0x%X, %d, %d, %s, 0, 0, 0' % (flag, x, y, gfx),
                          '\tptrvalue %s' % (lab or 0)]
        if trig:
            lines.append('\t.balign 8')
            lines.append('%s_Triggers:' % base)
            for x, y, var, val, lab in trig:
                lines += ['\t.2byte %d, %d' % (x, y), '\t.byte 0, 0', '\t.2byte %s, %s, 0' % (var, val),
                          '\tspace64 4', '\tptrvalue %s' % lab]
        ok = {k: n for k, n in once.items() if c.entry_label(n) in done}
        rows = []
        for var, val, n in frame:
            if c.entry_label(n) not in done:
                skipped += 1
                continue
            try:
                rows.append((c.var(var), c.var(val) if c.is_var(val) else c.value(val), c.entry_label(n)))
            except Unsupported:
                skipped += 1
        skipped += len(once) - len(ok)
        if not ok and not rows and not objs and not trig:
            continue
        index.append((mapid, len(objs), len(trig), '%s_Scripts' % base if ok or rows else 0,
                      '%s_People' % base if objs else 0, '%s_Triggers' % base if trig else 0))
        if not ok and not rows:
            continue
        table = []
        kinds_done = set()
        for kind, lab in ours:
            plat = next((k for k, v in INIT_KIND.items() if v == kind), None)
            if plat in ok:
                lines += ['%s_%s:' % (base, plat), '\tcall %s' % lab, '\tgoto %s' % c.entry_label(ok[plat])]
                table.append((kind, '%s_%s' % (base, plat)))
                kinds_done.add(plat)
            elif kind == 'MAP_SCRIPT_ON_FRAME_TABLE' and rows:
                lines.append('%s_Frame:' % base)
                for r in our_tables.get(lab, []) + rows:
                    lines.append('\tmap_script_2 %s, %s, %s' % r)
                lines.append('\t.2byte 0')
                table.append((kind, '%s_Frame' % base))
                rows = []
            else:
                table.append((kind, lab))
        for plat, n in ok.items():
            if plat not in kinds_done:
                table.append((INIT_KIND[plat], c.entry_label(n)))
        if rows:
            lines.append('%s_Frame:' % base)
            for r in rows:
                lines.append('\tmap_script_2 %s, %s, %s' % r)
            lines.append('\t.2byte 0')
            table.append(('MAP_SCRIPT_ON_FRAME_TABLE', '%s_Frame' % base))
        lines.append('%s_Scripts:' % base)
        lines += ['\tmap_script %s, %s' % t for t in table]
        lines.append('\t.byte 0')
    lines += map_index(index)
    return lines, len(index), skipped


def dex_rating_tables(root):
    """gSinnohDexRatings (src/sinnoh/s3d_scrcmd.c SINNOH_DEX_RATING): the pokedex_ratings bank, then rows of (up to,
    message, female's message) local then national, each list ended by 0xFFFF"""
    dec = converter(root, 'common').dec
    ids = dec.ids('pokedex_ratings')
    out = ['\t.balign 2', 'gSinnohDexRatings::', '\t.2byte %d' % dec.banks['pokedex_ratings']]
    for table in (DEX_RATING_LOCAL, DEX_RATING_NATIONAL):
        for row in table:
            m = ids['PokedexRatings_Text_' + row[1]]
            f = ids['PokedexRatings_Text_' + row[2]] if len(row) > 2 else m
            out.append('\t.2byte %d, %d, %d' % (row[0], m, f))
    return out


def stub_dex_ratings():
    return ['\t.balign 2', 'gSinnohDexRatings::', '\t.2byte 0, 0xFFFF, 0, 0, 0xFFFF, 0, 0']


def map_index(index):
    """struct SinnohMapStory (src/sinnoh/s3d_scrcmd.c): map, people, triggers, then the map scripts, the people and
    the triggers (0: the map's own)"""
    out = ['\t.balign 8', 'gSinnohPlatinumMapScripts::']
    for mapid, np, nt, scripts, ppl, trig in index:
        out += ['\t.2byte %s, %d, %d, 0' % (mapid, np, nt), '\tptrvalue %s' % scripts, '\tptrvalue %s' % ppl,
                '\tptrvalue %s' % trig]
    out += ['\t.2byte 0xFFFF, 0, 0, 0', '\tptrvalue 0', '\tptrvalue 0', '\tptrvalue 0']
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
    import tv_data
    lines = ['%s::\n\tend' % lab for lab in sorted(referenced())] + map_index([]) + ['gSinnohPlatinumNewGame::', '\tend'] + stub_dex_ratings()
    lines += ['gSinnohTvBroadcast::', '\tend']
    lines += tv_data.stub_lines()
    import contest_data
    lines += contest_data.stub_lines()
    write(lines, out_path)
    print('%s: %d stubs (no decomp)' % (os.path.relpath(out_path, ROOT), len(referenced())))


def build(root, out_path):
    refs = referenced()
    wanted = collections.defaultdict(set)
    for lab in refs:
        m = re.match(r'^%s(\w+?)_(\d+)$' % PREFIX, lab)
        if m:
            wanted[m.group(1)].add(int(m.group(2)))
    tables = map_tables(root, wanted)
    wanted['init_new_game'].add(1)        # (Platinum's new game: its flags, src/sinnoh/s3d_scrcmd.c Sinnoh_PlatinumStoryBegin)
    wanted['tv_broadcast'].add(1)         # (SINNOH's TVs: field_control_avatar.c, src/sinnoh/s3d_tv.c)
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
    mt, maps, skipped = emit_map_tables(root, tables, done)
    lines += mt
    lines += ['SinnohP_Movement_SmashRock:', '\trock_smash_break', '\tstep_end']
    lines += dex_rating_tables(root)
    import tv_data
    lines += tv_data.lines(converter(root, 'common').dec, root)   # (SINNOH's TV: src/sinnoh/s3d_tv.c)
    import contest_data
    lines += contest_data.lines(converter(root, 'common').dec, root)   # (SUPER CONTESTS: src/sinnoh/s3d_contest.c)
    new_game = converter(root, 'init_new_game').entry_label(1)
    lines += ['gSinnohPlatinumNewGame::', '\tgoto %s' % new_game if new_game in done else '\tend']
    tv = converter(root, 'tv_broadcast').entry_label(1)
    lines += ['gSinnohTvBroadcast::', '\tgoto %s' % tv if tv in done else '\tend']
    if new_game not in done:
        print('gen_scripts: Platinum\'s new game script doesn\'t convert: %s' % converter(root, 'init_new_game').check(1),
              file=sys.stderr)
    for lab, (b, m) in sorted(texts.items()):
        lines.append('%s:\n\t.byte %s' % (lab, text_bytes(b, m)))
    write(lines, out_path)
    print('%s: %d scripts, %d texts, Platinum\'s story on %d maps (%d of its map scripts left out)'
          % (os.path.relpath(out_path, ROOT), len(done), len(texts), maps, skipped))


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
    print('songs the game lacks: %s' % ' '.join(sorted(MISSING_MUSIC)))
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
