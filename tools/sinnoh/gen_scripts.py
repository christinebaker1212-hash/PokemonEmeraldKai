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
    python3 -I tools/sinnoh/gen_scripts.py --report-entries POKEPLATINUM_DIR    the same, each entry that doesn't by name

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
TERMINAL = {'End', 'Return', 'GoTo', 'EndMovement', 'ExitFrontier'}

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
SINNOH_LIFT = 53   # (the platform lifts: src/sinnoh/s3d_lift.c)
SINNOH_ELEVATOR = 54   # (the elevators: src/sinnoh/s3d_elevator.c Sinnoh_ElevatorCall)
SINNOH_REGI_DOT, SINNOH_QUAKE = 55, 56   # (the REGI ruins' dots; Platinum's screen quake, ScrCmd_29F)
SINNOH_MESSAGE_UNOWN = 57   # (gSinnohMessage <- a message in Platinum's UNOWN letters: src/text.c FONT_UNOWN)
SINNOH_UNOWN_SEEN = 58   # (the UNOWN letters seen: s3d_legends.c)
SINNOH_TRAM = 59   # (the GREAT MARSH's tram: s3d_tram.c)
SINNOH_MISC = 61   # (small commands on Kai's systems: src/sinnoh/s3d_misc.c)
SINNOH_TOWER = 62  # (Platinum's BATTLE TOWER: src/sinnoh/s3d_tower.c Sinnoh_TowerCall, include/sinnoh_tower.h)
# (its own functions past Platinum's BT_FUNC_* numbers: include/sinnoh_tower.h SINNOH_TOWER_*)
TOWER_INIT, TOWER_INIT_RESUME, TOWER_FREE, TOWER_INTRO, TOWER_BATTLE, TOWER_WON, TOWER_CHOOSE, TOWER_SAVE = range(200, 208)
TOWER_PARTNER_SPECIES, TOWER_PARTNER_MOVE = 210, 212   # (+ the POKéMON's number, 0 or 1)
TOWER_BUFFER_SPECIES, TOWER_BUFFER_MOVE = 214, 215     # (a kind's / a move's name into text slot DESTVAR, 0-5)
TOWER_JUDGE_TOTAL, TOWER_JUDGE_STAT, TOWER_JUDGE_BEST = 216, 217, 218   # (the STATS JUDGE: JudgeStats)
TOWER_RANKINGS = 220   # (JUBILIFE TV's rankings machines: src/sinnoh/s3d_rankings.c)
SINNOH_PARTY = 63   # (party and BAG queries, Platinum's ribbons: src/sinnoh/s3d_party.c Sinnoh_PartyCall)
SINNOH_JOURNAL = 64   # (Platinum's journal events: src/sinnoh/s3d_journal.c Sinnoh_JournalScript; 255: the JOURNAL given)
# Platinum's CreateJournalEvent kinds its ScrCmd_CreateJournalEvent writes (generated/journal_location_events.txt
# numbers); the others (gyms, the Elite Four...) Platinum writes from the battle's end, as s3d_journal.c does
JOURNAL_SCRIPT_EVENTS = {'LOCATION_EVENT_GAME_CORNER': 16, 'LOCATION_EVENT_SAFARI_GAME': 17,
                         'LOCATION_EVENT_ITEM_WAS_OBTAINED': 18, 'LOCATION_EVENT_USED_CUT': 19,
                         'LOCATION_EVENT_USED_SURF': 21, 'LOCATION_EVENT_USED_STRENGTH': 22,
                         'LOCATION_EVENT_USED_DEFOG': 23, 'LOCATION_EVENT_USED_ROCK_SMASH': 24,
                         'LOCATION_EVENT_USED_WATERFALL': 25, 'LOCATION_EVENT_USED_ROCK_CLIMB': 26,
                         'LOCATION_EVENT_BATTLE_TOWER': 36, 'LOCATION_EVENT_BATTLE_FACTORY': 37,
                         'LOCATION_EVENT_BATTLE_CASTLE': 38, 'LOCATION_EVENT_BATTLE_HALL': 39,
                         'LOCATION_EVENT_BATTLE_ARCADE': 40}
(PARTY_ARG, PARTY_REPEATED_SPECIES, PARTY_GET_RIBBON, PARTY_SET_RIBBON, PARTY_FRIENDSHIP, PARTY_NATURE, PARTY_FIND_NATURE,
 PARTY_TYPES, PARTY_HIDDEN_POWER, PARTY_CONTEST_STAT, PARTY_COMBEE_GENDERS, PARTY_MOVE_COUNT, PARTY_ITEM_QUANTITY,
 PARTY_FOSSIL_COUNT, PARTY_FOOTPRINT, PARTY_OUTSIDER, PARTY_SEEN, PARTY_CAN_ADD_COINS, PARTY_ADD_MONEY, PARTY_REMOVE_MONEY,
 PARTY_ENOUGH_MONEY, PARTY_WEATHER, PARTY_SELECTED_MOVE, PARTY_BUFFER_MOVE, PARTY_FORGET_MOVE, PARTY_HAS_MOVE,
 PARTY_EV_TOTAL, PARTY_BUFFER_NATURE, PARTY_TEACH_MOVE, PARTY_TEACH_SLOT, PARTY_BUFFER_TYPE, PARTY_GET_MOVE,
 PARTY_TRADE_INIT, PARTY_TRADE_REQUESTED, PARTY_TRADE_START, PARTY_TRADE_END, PARTY_FOSSIL_FIND,
 PARTY_FOSSIL_SPECIES, PARTY_PRIZE, PARTY_HAS_COINS, PARTY_TUTOR_CAN, PARTY_TUTOR_MENU, PARTY_TUTOR_AFFORD,
 PARTY_TUTOR_PAY, PARTY_RESET_MOVE, PARTY_SHARD_COST, PARTY_SHARD_CLOSE, PARTY_LEARN_OPEN, PARTY_LEARN_RESULT,
 PARTY_TOUGH_WORD, PARTY_TOUGH_ALL, PARTY_FORMS_ON, PARTY_LEVEL) = range(53)
CC_LINK_LEAD, CC_LINK_JOIN, CC_LINK_START, CC_LINK_END, CC_PARTY_ANY_RANK, CC_LINK_TEXT = 39, 40, 41, 42, 43, 44
# Platinum's link desk words that speak of DS wireless: CO-OP's own instead (LINK_TEXT_*: include/s3d_contest.h)
LINK_DESK_TEXTS = {'ContestCommon_Text_WelcomeLinkContest': 0, 'ContestCommon_Text_ExplainLinkContest': 1,
                   'ContestCommon_Text_CommunicationsWillBeLaunched': 2, 'ContestCommon_Text_CommunicatingStandBy': 5}   # (include/s3d_contest.h)
SINNOH_ROAMER = 60   # (a roaming POKéMON set going: src/sinnoh/s3d_roamer.c; the slot, Platinum's ROAMING_SLOT_*)   # (SINNOH's TV: src/sinnoh/s3d_tv.c TVCALL_*)
(TVCALL_STATUS, TVCALL_FRAMING, TVCALL_FINISH, TVCALL_SEGMENT_ID, TVCALL_SEGMENT, TVCALL_COMMERCIAL, TVCALL_NEXT,
 TVCALL_INTERVIEW_ID, TVCALL_INTERVIEW_MESSAGE, TVCALL_INTERVIEW_SAVE, TVCALL_INTERVIEW_OK, TVCALL_WORD,
 TVCALL_HIDDEN_ITEM, TVCALL_NAME_RATED, TVCALL_STORAGE, TVCALL_VILLA, TVCALL_AMITY, TVCALL_QUESTION,
 TVCALL_TWO_WORDS, TVCALL_SECOND_WORD, TVCALL_GIFT_PHRASE, TVCALL_UNLOCK_GIFT) = range(22)
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

CONSTANTS = {'BATTLEGROUND_TRAINER_NONE': 0xFFF, 'MOVE_NOT_SELECTED': 0xFF, 'TRUE': 1, 'FALSE': 0, 'NO_EXIT_ON_B': 0, 'EXIT_ON_B': 1, 'MENU_YES': 0, 'MENU_NO': 1, 'GENDER_MALE': 0, 'GENDER_FEMALE': 1}

# Platinum's movement actions -> this game's (Platinum's field runs at 30 frames a second, this one at 60: a delay
# is twice as many frames here)
DIRS = {'North': 'up', 'South': 'down', 'West': 'left', 'East': 'right'}
MOVE_KIND = {'WalkNormal': 'walk_', 'WalkSlightlyFast': 'walk_fast_', 'WalkFast': 'walk_fast_', 'WalkFaster': 'walk_faster_', 'WalkSlow': 'walk_slow_',
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


def enum_constants(root, rel, prefix):
    """{NAME: number} of a C enum's names starting with PREFIX (counted from the last explicit value)"""
    out, n = {}, -1
    try:
        text = open(os.path.join(root, rel), errors='replace').read()
    except OSError:
        return out
    for m in re.finditer(r'^\s*(%s\w+)\s*(?:=\s*(\w+))?\s*,' % prefix, text, re.M):
        n = int(m.group(2), 0) if m.group(2) and re.match(r'^(0x[0-9A-Fa-f]+|\d+)$', m.group(2)) else n + 1
        out[m.group(1)] = n
    return out


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
                'SEQ_D_SAFARI_sseq': 'MUS_SINNOH_GREAT_MARSH', 'SEQ_THE_EVENT02_sseq': 'MUS_SINNOH_GALACTIC_APPEARS',
                # from Platinum's own sound data (tools/kaisong/sinnoh_songs.py SONGS_SDAT)
                'SEQ_KUSAGASA_sseq': 'MUS_SINNOH_CATCHING_DEMO', 'SEQ_OPENING_sseq': 'MUS_SINNOH_OPENING',
                'SEQ_OPENING_sseq_1': 'MUS_SINNOH_OPENING', 'SEQ_PL_TOWN02_sseq': 'MUS_SINNOH_MUSIC_BOX'})
    return out


MUSIC_OF = _music_of()
MISSING_MUSIC = set()   # Platinum's songs its scripts play that the game lacks (--report lists them)


# the DISTORTION WORLD's steps on its walls (Platinum's movement actions 105-114: one tile in 8 frames along a 3D axis,
# map_object_move.c's tables): a step up or down a wall stays on its tile here, one along the ground is a quick step
DW_STEPS = {'MoveAction_105': ['delay_8'], 'MoveAction_106': ['delay_8'], 'MoveAction_109': ['delay_8'],
            'MoveAction_110': ['delay_8'], 'MoveAction_107': ['walk_fast_down'], 'MoveAction_112': ['walk_fast_down'],
            'MoveAction_113': ['walk_fast_down'], 'MoveAction_108': ['walk_fast_up'], 'MoveAction_111': ['walk_fast_up'],
            'MoveAction_114': ['walk_fast_up']}

class Unsupported(Exception):
    pass


_RIBBON_NAMES = {}


def ribbon_name_id(root, ribbon):
    """Platinum's ribbon's name's message id (src/ribbon.c sRibbons[...].nameID)"""
    if not _RIBBON_NAMES:
        import subprocess
        path = os.path.join(root, 'src/ribbon.c')
        text = open(path).read() if os.path.exists(path) else subprocess.run(
            ['git', '-C', root, 'cat-file', '-p', 'HEAD:src/ribbon.c'], capture_output=True, text=True).stdout
        for m in re.finditer(r'\[(RIBBON_\w+)\] = \{[^}]*?\.nameID = (\w+)', text, re.S):
            _RIBBON_NAMES[m.group(1)] = m.group(2)
    return _RIBBON_NAMES.get(ribbon)


def _sprite(gfx):
    """this game's sprite for a Platinum one (tools/sinnoh/sprites.py); the player's looks of the other gender are the
    counterpart's (DAWN, LUCAS)"""
    import sprites
    gfx = {'OBJ_EVENT_GFX_PLAYER_F': 'OBJ_EVENT_GFX_DAWN', 'OBJ_EVENT_GFX_PLAYER_M': 'OBJ_EVENT_GFX_LUCAS',
           # (the Wi-Fi rooms' attendants: online, closed; their scripts' looks never shown)
           'OBJ_EVENT_GFX_WIFI_PLAZA_ATTENDANT_F': 'OBJ_EVENT_GFX_FRONTIER_SINGLE_ATTENDANT',
           'OBJ_EVENT_GFX_WIFI_PLAZA_ATTENDANT_M': 'OBJ_EVENT_GFX_FRONTIER_SINGLE_ATTENDANT'}.get(gfx, gfx)
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
        line = re.sub(r'/\*.*?\*/', '', raw).split('//')[0].split('@')[0].rstrip()   # (every kind of comment)
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
            # the DISTORTION WORLD's floors are placed in one stacked world (tw_arc.narc's offsets: gen_distortion.py):
            # Platinum's tiles there are the floor's own plus its offset
            try:
                import gen_distortion
                ids_ = gen_distortion.header_ids(self.root)
                for f in gen_distortion.read(self.root)[0]:
                    low = [n for n, i in ids_.items() if i == f['hid']][0][len('MAP_HEADER_'):].lower()
                    if low in self._origins:
                        m, ox, oz = self._origins[low]
                        self._origins[low] = (m, ox + f['off'][0], oz + f['off'][2])
            except (OSError, ImportError, IndexError, KeyError):
                pass
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


# Platinum's scripts that Kai's own systems supersede in part (CLAUDE.md: what Kai already has, SINNOH uses): a
# label's commands replaced, the rest of the file as Platinum's. Command names starting with Kai are the converter's own.
KAI_REWRITES = {
    # AMITY SQUARE: every POKéMON follows anywhere in this game (Kai's followers), so the gates don't check the party
    # or hand a POKéMON over: stepping in at a gate makes the follower the square's walker as it is (KaiAmityEnter: its
    # party slot and species into Platinum's follower variables, the step count from zero, its first find drawn by
    # Platinum's own SetNextPickUpType), stepping out ends it, without a word. Talking to the follower in the square
    # runs Platinum's own AmitySquare_FollowerMon (mp_follower.c MP_Follower_TryInteract): its finds every 200 steps,
    # its lines; the gift man is Platinum's.
    'amity_square': {
        'AmitySquare_CoordEvent_WestGate': [('KaiAmityEnter', []), ('Call', ['AmitySquare_FollowerMon_SetNextPickUpType']),
                                            ('End', [])],
        'AmitySquare_CoordEvent_EastGate': [('KaiAmityEnter', []), ('Call', ['AmitySquare_FollowerMon_SetNextPickUpType']),
                                            ('End', [])],
        'AmitySquare_CoordEvent_ExitAmitySquare': [('KaiAmityLeave', []), ('SetVar', ['VAR_FOLLOWER_MON_ACTIVE', 'FALSE']),
                                                   ('End', [])],
    },
    # the BATTLE TOWER's lobby: the attendant who walks the player to the door is named, not Platinum's object number
    # (its numbers start at 0, this game's local ids at 1)
    'battle_tower': {
        lab: [('SetVar', ['VAR_MAP_LOCAL_0x02', who]), ('SetVar', ['VAR_MAP_LOCAL_0x03', door]),
              ('Call', ['BattleTower_EnterBattleRoomAnimation']), ('GoTo', [then])]
        for lab, who, door, then in [
            ('BattleTower_EnterSingleDoubleBattleRoom', 'LOCALID_SINGLE_ATTENDANT', '15', 'BattleTower_WarpToElevator'),
            ('BattleTower_EnterWiFiBattleRoom', 'LOCALID_WIFI_ATTENDANT', '19', 'BattleTower_WarpToElevator'),
            ('BattleTower_EnterMultiBattleRoom', 'LOCALID_MULTI_ATTENDANT', '11', 'BattleTower_GoToWarpToElevator'),
            ('BattleTower_EnterLinkMultiBattleRoom', 'LOCALID_MULTI_ATTENDANT', '11', '_06A0')]
    } | {
        # (the multi counter's question about battling with a friend by wireless is left out: the game's wireless
        # sides are CO-OP's; a BATTLE SALON partner it is)
        'BattleTower_AskCommunicateWithFriend': [('GoTo', ['BattleTower_InitMultiChallengeMode'])],
        'BattleTower_HealAndSaveBeforeCommunicating': [('GoTo', ['BattleTower_ClearCommunicationAndEnd'])],
    },
}


# Entries kept from converting until what goes with them is built (a map whose own setup converts takes Platinum's
# people: SPEAR PILLAR's CYRUS waits on the DISTORTION WORLD's commands, so the map stays the stand-in's till then)
HOLD = {('spear_pillar', 1): 'held: SPEAR PILLAR waits for the DISTORTION WORLD (CYRUS\'s scene)'}


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
        for lab, body in KAI_REWRITES.get(self.low, {}).items():
            self.labels[lab] = body
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
        m = re.match(r'^(\w+)\s*([+-])\s*(\d+)$', x)
        if m:   # (a constant and a number: VILLA_FURNITURE_PIANO + 1)
            base = self.value(m.group(1))
            if re.match(r'^-?\d+$', base):
                return str(int(base) + (1 if m.group(2) == '+' else -1) * int(m.group(3)))
            return '(%s %s %s)' % (base, m.group(2), m.group(3))
        if x in CONSTANTS:
            return str(CONSTANTS[x])
        if re.match(r'^(ITEM|SPECIES|MOVE)_', x):
            x = RENAME.get(x, x)
            x = {'ITEM_SECRETPOTION': 'ITEM_SECRET_POTION', 'ITEM_BLACKGLASSES': 'ITEM_BLACK_GLASSES',
                 'ITEM_SILVERPOWDER': 'ITEM_SILVER_POWDER', 'ITEM_NEVERMELTICE': 'ITEM_NEVER_MELT_ICE',
                 'ITEM_THUNDERSTONE': 'ITEM_THUNDER_STONE',
                 'ITEM_PARCEL': 'ITEM_SINNOH_PARCEL', 'ITEM_BICYCLE': 'ITEM_MTN_BIKE',   # (SINNOH's own bike)
                 'ITEM_HM05': 'ITEM_HM_DEFOG', 'ITEM_HM08': 'ITEM_HM_ROCK_CLIMB'}.get(x, x)   # (Platinum's HM05 / HM08)   # (ITEM_PARCEL here is KANTO's OAK'S PARCEL)
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
        if '_Text_' in x:
            where = self.dec.message(self.bank, x)   # (a text's number, added to a variable: AddMenuEntry by a variable)
            if where is not None:
                return str(where[1])
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
        if x.startswith('DIST_WORLD_MAP_OBJECT_') and x in CONSTANTS:
            return str(CONSTANTS[x])     # (the DISTORTION WORLD's own people: src/sinnoh/s3d_distortion.c spawns them)
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
        if cmd in DW_STEPS:
            return DW_STEPS[cmd] * n
        m = re.match(r'^(\w+?)(North|South|West|East)$', cmd)
        if m and m.group(1) == 'JumpDistortionWorld':
            # (the DISTORTION WORLD's long jump, 3 tiles: this game's 2-tile jump and a quick last step)
            return ['jump_2_' + DIRS[m.group(2)], 'walk_faster_' + DIRS[m.group(2)]] * n
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
        out = self.party_command(cmd, a)
        if out is not None:
            return out
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
        if cmd == 'GivePokedex':
            # (PROF. ROWAN's POKéDEX: the START menu's, with the NATIONAL DEX as every region's POKéDEX in this game)
            return ['setflag FLAG_SYS_POKEDEX_GET', 'special EnableNationalPokedex']
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
        if cmd == 'Message' and self.low == 'contests' and a[0] in LINK_DESK_TEXTS:
            # (the link desk in CO-OP's words, not DS wireless: src/sinnoh/s3d_contest_link.c Contest_LinkText)
            return [self.sinnoh(SINNOH_CONTEST, CC_LINK_TEXT, LINK_DESK_TEXTS[a[0]]), 'message gStringVar4', 'waitmessage']
        if cmd == 'Message':
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd in ('WaitButton', 'WaitABXPadPress', 'WaitABPress', 'WaitABPressTime'):
            return ['waitbuttonpress']
        if cmd in ('CloseMessage', 'CloseMessageWithoutErasing'):
            return ['closemessage']
        if cmd == 'InitGreatMarshTram':
            return [self.sinnoh(SINNOH_TRAM, 0)]
        if cmd == 'MoveGreatMarshTram':
            return [self.sinnoh(SINNOH_TRAM, 1, self.var(a[0]), self.value(a[1]))]
        if cmd == 'CheckGreatMarshTramLocation':
            return [self.sinnoh(SINNOH_TRAM, 2, self.value(a[0]), self.var(a[1]))]
        if cmd == 'SetPlayerHeightCalculationEnabled':
            # (the GREAT MARSH's tram: the player rides at the platform's height, s3d_tram.c)
            return [self.sinnoh(SINNOH_TRAM, 3, 1 if a[0] == 'TRUE' else 0)]
        if cmd == 'BufferVarPaddingDigits':
            slot = int(self.strvar(a[0])[-1]) - 1
            return [self.sinnoh(SINNOH_MISC, 4, self.var(a[1]),
                                (slot << 8) | (0x80 if a[2] == 'PADDING_MODE_ZEROES' else 0) | int(self.value(a[3])))]
        if cmd == 'BufferValuePaddingDigits':
            slot = int(self.strvar(a[0])[-1]) - 1
            v = int(self.value(a[1]))
            return ([self.sinnoh(SINNOH_MISC, 23, v >> 16)] if v > 0xFFFF else []) + [self.sinnoh(
                SINNOH_MISC, 22, v & 0xFFFF, (slot << 8) | (0x80 if a[2] == 'PADDING_MODE_ZEROES' else 0) | int(self.value(a[3])))]
        if cmd == 'BufferMonNicknameFromPC':
            return [self.sinnoh(SINNOH_MISC, 5, self.var(a[1]), int(self.strvar(a[0])[-1]) - 1)]
        # ROUTE 222's size contest (REMORAID): s3d_misc.c 6-9 on Emerald's size code (pokemon_size_record.c); the
        # number's two halves go to text slots 0-5 (SINNOH's text has six)
        # the POKéMART clerks and the POKéMON CENTER's nurse on the game's own shop and nurse (as the written stand-ins
        # had them: gen_words.py); the greeting is Platinum's own common script
        if cmd in ('PokeMartCommonWithGreeting', 'PokeMartSpecialtiesWithGreeting'):
            if cmd == 'PokeMartCommonWithGreeting':
                shop = 'pokemart 0x0'   # (Platinum's list by the region's badges: shop.c in SINNOH)
            else:
                from gen_marts import label as mart_label
                shop = 'pokemart %s' % mart_label(a[0])
            return (['playse SE_SELECT', 'lockall', 'faceplayer', 'call %s' % self.common(self.dec.common_macro['Common_VendorGreeting']),
                     shop, 'msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT', 'releaseall'])
        if cmd in ('PokeMartSpecialties', 'PokeMartCommon'):
            # (the shop alone, its own greeting already said by the script: ETERNA's herb shop)
            if cmd == 'PokeMartCommon':
                return ['pokemart 0x0']
            from gen_marts import label as mart_label
            return ['pokemart %s' % mart_label(a[0])]
        if cmd == 'Common_CallPokecenterNurse':
            return ['setvar VAR_0x800B, %s' % self.localid(a[0]), 'call Common_EventScript_PkmnCenterNurse', 'waitmessage',
                    'waitbuttonpress', 'release']
        # AMITY SQUARE (s3d_misc.c 10-15: Platinum's scrcmd_amity_square.c)
        # the VILLA's music box (Platinum's scrcmd_sound.c: its song, then the one before back)
        if cmd == 'CheckABPress':
            return [self.sinnoh(SINNOH_MISC, 19, self.var(a[0]))]
        if cmd == 'CheckMetFurnitureRequirements':
            return [self.sinnoh(SINNOH_MISC, 20, self.var(a[0]) if self.is_var(a[0]) else self.value(a[0]), self.var(a[1]))]
        if cmd in ('IncrementGameRecord', 'IncrementGameRecordTwice') and a and a[0] == 'RECORD_TIMES_BATTLED_AT_BATTLEGROUND':
            return [self.sinnoh(SINNOH_MISC, 21)]   # (the VILLA's GUEST SET waits on 50)
        if cmd in ('InitPersistedMapFeaturesForVilla', 'Dummy1F9'):
            return []                       # (the VILLA's furniture stands as it's owned on each visit: s3d_gyms.c)
        if cmd == 'GetCurrentBGM':
            return [self.sinnoh(SINNOH_MISC, 17, self.var(a[0]))]
        if cmd == 'SetScenePlayBGMMusicBox':
            mus = MUSIC_OF.get('SEQ_PL_TOWN02_sseq')
            if mus is None:
                MISSING_MUSIC.add('SEQ_PL_TOWN02_sseq')
                return []                   # (the music box's song: the game hasn't got it yet; the music plays on)
            return ['playbgm %s, FALSE' % mus]
        if cmd == 'SetFieldScene':
            return [self.sinnoh(SINNOH_MISC, 18, self.var(a[0]) if self.is_var(a[0]) else self.value(a[0]))]
        if cmd == 'CheckMoney':
            return ['checkmoney %s' % self.value(a[1]), 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'KaiAmityEnter':
            return [self.sinnoh(SINNOH_MISC, 10, self.var('VAR_FOLLOWER_MON_PARTY_ID'), self.var('VAR_FOLLOWER_MON_SPECIES')),
                    'setvar %s, %d' % (self.var('VAR_FOLLOWER_MON_ACTIVE'), 1)]
        if cmd == 'KaiAmityLeave':
            return [self.sinnoh(SINNOH_MISC, 16)]
        if cmd == 'ClearAmitySquareStepCount':
            return [self.sinnoh(SINNOH_MISC, 11)]
        if cmd == 'GetAmitySquareStepCount':
            return [self.sinnoh(SINNOH_MISC, 12, self.var(a[0]))]
        if cmd == 'CalcAmitySquareFoundAccessory':
            return [self.sinnoh(SINNOH_MISC, 13, self.var(a[0]), self.var(a[1]))]
        if cmd == 'CalcAmitySquareBerryAndAccessoryManOptionID':
            return ['random 16', 'copyvar %s, VAR_RESULT' % self.var(a[0])]
        if cmd == 'CheckAmitySquareManGiftIsAccessory':
            return [self.sinnoh(SINNOH_MISC, 14, self.var(a[0]), self.var(a[1]))]
        if cmd == 'GoToIfAmitySquareManGiftIsNotAccessory':   # (Platinum's macro)
            return (self.command('CheckAmitySquareManGiftIsAccessory', [a[0], 'VAR_RESULT'])
                    + self.command('GoToIfEq', ['VAR_RESULT', 'FALSE', a[1]]))
        if cmd == 'GetAmitySquareBerryOrAccessoryIDFromMan':
            return [self.sinnoh(SINNOH_MISC, 15, self.var(a[0]), self.var(a[1]))]
        if cmd == 'CalcSizeContestResult':
            return [self.sinnoh(SINNOH_MISC, 6, self.var(a[0]), self.var(a[1]))]
        if cmd == 'BufferPartyPokemonSize':
            return [self.sinnoh(SINNOH_MISC, 7, self.var(a[2]), (int(self.value(a[0])) << 8) | int(self.value(a[1])))]
        if cmd == 'UpdateSizeContestRecord':
            return [self.sinnoh(SINNOH_MISC, 8, self.var(a[0]))]
        if cmd == 'BufferSizeContestRecord':
            return [self.sinnoh(SINNOH_MISC, 9, self.value(a[2]), (int(self.value(a[0])) << 8) | int(self.value(a[1])))]
        if cmd == 'GetJubilifeLotteryTrainerID':
            return [self.sinnoh(SINNOH_MISC, 0, self.var(a[0]))]
        if cmd == 'CheckForJubilifeLotteryWinner':
            v = [self.var(x) for x in a[:3]]
            if v != ['VAR_0x8001', 'VAR_0x8002', 'VAR_0x8003']:
                raise Unsupported('lottery variables %s' % v)
            return [self.sinnoh(SINNOH_MISC, 1, 'VAR_0x8001', self.var(a[3]))]
        if cmd == 'RandomizeJubilifeLottery':
            return [self.sinnoh(SINNOH_MISC, 2)]
        if cmd == 'CheckIsDepartmentStoreRegular':
            return [self.sinnoh(SINNOH_MISC, 3, self.var(a[0]))]
        if cmd in ('AddToGameRecord', 'AddToGameRecordBigValue'):
            return []   # (Platinum's records shown only by its rankings rooms: group / Wi-Fi, closed)
        if cmd == 'GetPartyRotomCountAndFirst':
            # (ROTOM in another form than its own: none, the forms coming only from the ROTOM distribution, a DS event
            # closed here: ROTOM's room has no appliances, as Platinum's without it)
            return ['setvar %s, 0' % self.var(a[0]), 'setvar %s, 255' % self.var(a[1])]
        if cmd == 'GetRotomFormsInSave':
            return ['setvar %s, 1' % self.var(x) for x in a[:5]]
        if cmd == 'SetRotomForm':
            return []
        if cmd == 'ActivateRoamingPokemon':
            slots = [x.strip() for x in open(os.path.join(self.root, 'generated/roaming_slots.txt')) if x.strip()]
            return [self.sinnoh(SINNOH_ROAMER, slots.index(a[0]))]
        if cmd == 'DrawPokemonPreview':
            return ['showmonpic %s, 10, 3' % self.arg(a[0])]   # (the POKéMON's picture in a window, as Platinum's)
        if cmd == 'RemovePokemonPreview':
            return ['hidemonpic']
        if cmd == 'FlickerObject':
            # (the object blinks TIMES times, DELAY frames each: Platinum's MapObject_Flicker)
            obj, times, delay = self.localid(a[0]), int(self.value(a[1])), 2 * int(self.value(a[2]))
            return ['removeobject %s' % obj, 'delay %d' % delay, 'addobject %s' % obj, 'delay %d' % delay] * times
        if cmd == 'EndSafariGame':
            return ['special ExitSafariMode']   # (the game's own SAFARI GAME: safari_zone.c)
        if cmd == 'GetUnownFormsSeenCount':
            return [self.sinnoh(SINNOH_UNOWN_SEEN, self.var(a[0]))]
        if cmd == 'MessageUnown':
            where = self.dec.message(self.bank, a[0])
            if where is None:
                raise Unsupported('message %s' % a[0])
            return [self.sinnoh(SINNOH_MESSAGE_UNOWN, where[0], where[1]), 'message gSinnohMessage', 'waitmessage']
        if cmd == 'UndergroundNPCMessage':
            # (the Underground's own message box, which names the player itself: the game's)
            return [self.sinnoh(SINNOH_BUFFER_PLAYER, 0), 'message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd == 'CloseUndergroundNPCMessage':
            return ['closemessage']
        if cmd == 'SetCommPlayerDir':
            # (the player turned, Platinum's Underground player being a link one)
            return [self.sinnoh(SINNOH_FACE, 'LOCALID_PLAYER', self.value(a[0]))]
        if cmd == 'GetFloorsAbove':
            return [self.sinnoh(SINNOH_ELEVATOR, 0, self.var(a[0]))]
        if cmd == 'BufferFloorNumber':
            return [self.sinnoh(SINNOH_ELEVATOR, 1, self.value(a[0]), self.value(a[1]))]
        if cmd == 'ShowCurrentFloor':
            return [self.sinnoh(SINNOH_ELEVATOR, 2)]
        if cmd == 'SetSpecialLocation':
            o = self.dec.origins().get(a[0][len('MAP_HEADER_'):].lower())
            if o is None:
                raise Unsupported('special location %s' % a[0])
            return [self.sinnoh(SINNOH_ELEVATOR, 3, o[0])]
        if cmd == 'PlayElevatorAnimation':
            return [self.sinnoh(SINNOH_ELEVATOR, 4, self.var(a[0]) if self.is_var(a[0]) else self.value(a[0]),
                                self.value(a[1])), 'special MoveElevator']
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
        if cmd in ('BufferSpeciesName', 'BufferSpeciesNameFromVar', 'BufferSpeciesNameWithArticle', 'BufferMoveName'):
            slot = int(self.value(a[0]))
            if slot > 2:
                # (SINNOH's text slots 4-6: src/sinnoh/s3d_tower.c's name buffering)
                return [self.tower(TOWER_BUFFER_MOVE if cmd == 'BufferMoveName' else TOWER_BUFFER_SPECIES,
                                   self.arg(a[1]), slot)]
            return ['%s %s, %s' % ('buffermovename' if cmd == 'BufferMoveName' else 'bufferspeciesname',
                                   self.strvar(a[0]), self.arg(a[1]))]
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
            value = a[1]          # (AddListMenuEntry's third is an alternative text)
            value = self.var(value) if self.is_var(value) else self.value(value)
            if self.is_var(a[0]):
                # (a text by its number in a variable, the MINING MUSEUM's fossils: Platinum's menu entries' bank)
                return [self.sinnoh(SINNOH_MENU_ADD, self.dec.banks['menu_entries'], self.var(a[0]), value)]
            where = self.dec.message(self.bank, a[0])
            if where is None:
                raise Unsupported('menu entry %s' % a[0])
            return [self.sinnoh(SINNOH_MENU_ADD, where[0], where[1], value)]
        if cmd == 'ShowListMenuRememberCursor':
            return [self.sinnoh(SINNOH_MENU_SHOW, self.var(a[0]), self.var(a[1]))]
        if cmd in ('ShowMenu', 'ShowListMenu', 'ShowMenuMultiColumn', 'ShowListMenuSetWidth'):
            return [self.sinnoh(SINNOH_MENU_SHOW)]   # (the game's menu sizes itself)
        if cmd in ('ScrCmd_32D', 'ScrCmd_338', 'ScrCmd_32E', 'ScrCmd_339', 'ScrCmd_332', 'LockObject', 'UnlockObject',
                   'ReleaseObject', 'SetMenuXOriginToRight', 'IncrementTrainerScore', 'IncrementTrainerScore2', 'IncrementGameRecord',
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
        if cmd == 'CheckPartyHasSpecies':
            return [self.sinnoh(SINNOH_HAS_SPECIES, self.var(a[0]), self.arg(a[1]))]
        if cmd == 'CheckPartyHasSpecies2':   # (species, destVarID: the other way round)
            return [self.sinnoh(SINNOH_HAS_SPECIES, self.var(a[1]), self.arg(a[0]))]
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
        # the DISTORTION WORLD (src/sinnoh/s3d_distortion.c, s3d_misc.c ops 29-39)
        if cmd == 'InitPersistedMapFeaturesForDistortionWorld':
            return [self.sinnoh(SINNOH_MISC, 29)]
        if cmd == 'AddDistortionWorldMapObject':
            return [self.sinnoh(SINNOH_MISC, 30, self.localid(a[0]))]
        if cmd == 'DeleteDistortionWorldMapObject':
            return [self.sinnoh(SINNOH_MISC, 31, self.localid(a[0]))]
        if cmd == 'StartDistortionWorldGiratinaShadowEvent':
            return [self.sinnoh(SINNOH_MISC, 32, self.value(a[0]))]
        if cmd == 'ResetDistortionWorldPersistedCameraAngles':
            return [self.sinnoh(SINNOH_MISC, 39)]
        if cmd == 'FinishDistortionWorldGiratinaShadowEvent':
            return [self.sinnoh(SINNOH_MISC, 33)]
        if cmd == 'SetPartyGiratinaForm':
            return [self.sinnoh(SINNOH_MISC, 34, self.value(a[0]))]
        if cmd == 'GetPlayer3DPos':
            return [self.sinnoh(SINNOH_MISC, 35, self.var(a[0]), self.var(a[1])), self.sinnoh(SINNOH_MISC, 36, self.var(a[2]))]
        if cmd == 'StartGiratinaOriginBattle':
            # (GIRATINA in its Origin Forme, Platinum's ScrCmd_StartGiratinaOriginBattle: a legendary's battle)
            return [self.sinnoh(SINNOH_MISC, 37, self.value(a[0]), self.value(a[1])), 'special BattleSetup_StartLegendaryBattle', 'waitstate']
        if cmd == 'DoDWWarp':
            return ['playse SE_WARP_IN']   # (Platinum's swirl into the DISTORTION WORLD: the game's warp sound)
        if cmd == 'GetBattleResult':
            # (Platinum's BATTLE_RESULT_* from the game's battle outcome: s3d_misc.c op 38)
            return [self.sinnoh(SINNOH_MISC, 38, self.var(a[0]))]
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
        if cmd in ('HideObject', 'ShowObject'):
            o = self.dec.origins().get(self.events_low)
            if o is None:
                raise Unsupported('objects on %s' % self.events_low)
            return ['%sobjectat %s, %s' % (cmd[:4].lower(), self.localid(a[0]), o[0])]
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
        if cmd == 'StartTrainerBattle' and self.is_var(a[0]):
            # (a trainer the script picked: the BATTLEGROUND's leaders, the POKeMON CENTERs' daily trainers, the
            # mansion's maids. Each trainer this file puts in the variable, compared in turn, with its own defeat line)
            src = open(self.path).read()
            names = [a[0]] + re.findall(r'#define\s+(\w+)\s+%s\b' % re.escape(a[0]), src)   # (VAR_TRAINER: VAR_0x8005)
            cands = sorted(set(t for n in names
                               for t in re.findall(r'SetVar\s+%s,\s*(TRAINER_\w+)' % re.escape(n), src)))
            if not cands:
                raise Unsupported('StartTrainerBattle by a variable no trainer is put in')
            done = '%s%s__trainer%d' % (PREFIX, self.low, self.next_local())
            out = []
            for t in cands:
                where = build_text.trainer_message(self.root, t, 'TRMSG_DEFEAT')
                if where is None:
                    raise Unsupported('trainer %s has no defeat line' % t)
                lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
                self.texts[lab] = where
                skip = '%s%s__trainer%d' % (PREFIX, self.low, self.next_local())
                out += ['compare_var_to_value %s, %s' % (self.var(a[0]), self.value(t)), 'goto_if 5, %s' % skip,
                        'trainerbattle_no_intro %s, %s' % (self.value(t), lab), 'goto %s' % done, '%s:' % skip]
            return out + ['%s:' % done]
        if cmd == 'BufferTrainerName':
            return ['buffertrainername %s, %s' % (self.strvar(a[0]), self.var(a[1]) if self.is_var(a[1]) else self.value(a[1]))]
        if cmd == 'StartTrainerBattle':
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
            # the party is healed after), Platinum's defeat line; no line when he wins (Platinum has none: his
            # defeat line there had him talk about losing when the player lost)
            where = build_text.trainer_message(self.root, a[0], 'TRMSG_DEFEAT')
            if where is None:
                raise Unsupported('trainer %s has no defeat line' % a[0])
            lab = '%sT_%d_%d' % (PREFIX, where[0], where[1])
            self.texts[lab] = where
            return ['.byte SCR_OP_TRAINERBATTLE, 13', '.2byte %s, 1' % self.value(a[0]), 'ptrvalue %s' % lab,
                    'ptrvalue 0']
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
        if cmd == 'EnableSwarms':
            # (the day's swarms on: src/sinnoh/s3d_wild.c)
            return [self.sinnoh(SINNOH_MISC, 41)]
        if cmd == 'GetSwarmMapAndSpecies':
            return [self.sinnoh(SINNOH_MISC, 42, self.var(a[0]), self.var(a[1]))]
        if cmd == 'BufferMapName':
            slot = int(self.value(a[0]))
            if slot > 2:
                raise Unsupported('string buffer %d' % slot)
            return [self.sinnoh(SINNOH_MISC, 43, self.var(a[1]), slot)]
        if cmd == 'AddTrophyGardenMon':
            # (MR. BACKLOT's daily POKeMON in the TROPHY GARDEN's grass: s3d_wild.c)
            return [self.sinnoh(SINNOH_MISC, 44)]
        if cmd == 'GetTrophyGardenSlot1Species':
            return [self.sinnoh(SINNOH_MISC, 45, self.var(a[0]))]
        if cmd == 'GetDailyRandomLevel':
            return [self.sinnoh(SINNOH_MISC, 46, self.var(a[0]))]
        if cmd == 'ChangeDeoxysForm':
            # (DEOXYS has one form in this game, as GIRATINA: SetPartyGiratinaForm; the meteorite's words stay Platinum's)
            return []
        if cmd == 'ShowDiplomaSinnoh':
            return ['special Special_ShowDiploma', 'waitstate']
        if cmd == 'ShowDiplomaNationalDex':
            # (the game's own diploma screen, the NATIONAL DEX's)
            return ['special Special_ShowDiploma', 'waitstate']
        if cmd in ('TakeShipFromCanalave', 'TakeShipFromSnowpoint'):
            # the boat out of CANALAVE / SNOWPOINT (Platinum's sailing scene: a fade and the warp, as the game's ferries)
            return ['fadescreen FADE_TO_BLACK', 'playse SE_SHIP'] + self.command('Warp', a[1:4])
        if cmd in ('TakeShipToCanalave', 'TakeShipToSnowpoint'):
            # back to the harbor's dock (where the boats come in: the game's own island sailors' spot)
            # (Platinum's macros: PlayBoatCutscene ..., MAP_HEADER_SNOWPOINT_CITY, 356, 246, its world tiles)
            dock = {'TakeShipToCanalave': ('MAP_SINNOH_CANALAVE_CITY', 11, 22),
                    'TakeShipToSnowpoint': ('MAP_SINNOH_SNOWPOINT_CITY', None, None)}[cmd]
            if dock[1] is None:
                o = self.dec.origins().get('snowpoint_city')
                if o is None:
                    raise Unsupported(cmd)
                dock = (o[0], 356 - o[1], 246 - o[2])
            go = '%s%s__docked%d' % (PREFIX, self.low, self.next_local())
            return ['fadescreen FADE_TO_BLACK', 'playse SE_SHIP', self.sinnoh(SINNOH_WARP_CONTINUE), 'goto %s' % go,
                    'warp %s, %d, %d' % dock, 'waitstate', '%s:' % go]
        if cmd == 'ScrCmd_2B5':
            # Platinum's exit location (FieldOverworldState_GetExitLocation: where an ESCAPE ROPE or DIG takes the player)
            # set to a place by its world tiles: the game's own escape warp. SENDOFF SPRING's arrival puts it on the
            # SPRING PATH (MAP_HEADER_UNKNOWN_266, a header the decomp has no name for: found by the tiles), and that
            # scene must run: it tells ROWAN the story goes on (VAR_SANDGEM_TOWN_LAB_STATE 2), which opens ROUTE 222
            x, z = int(self.value(a[1])), int(self.value(a[2]))
            low = a[0][len('MAP_HEADER_'):].lower() if a[0].startswith('MAP_HEADER_') else None
            o = self.dec.origins().get(low)
            if o is None:
                for pl in self.dec.places:
                    if pl['matrix'] == 0 and pl['x0'] <= x // 32 <= pl['x1'] and pl['z0'] <= z // 32 <= pl['z1']:
                        o = (pl['id'], pl['x0'] * 32, pl['z0'] * 32)
                        break
            if o is None:
                raise Unsupported('exit location %s' % a[0])
            return ['setescapewarp %s, %d, %d' % (o[0], x - o[1], z - o[2])]
        if cmd == 'Warp':
            low = a[0][len('MAP_HEADER_'):].lower() if a[0].startswith('MAP_HEADER_') else None
            o = self.dec.origins().get(low)
            if o is None:
                raise Unsupported('warp to %s' % a[0])
            # (Platinum's script goes on on the new map: src/sinnoh/s3d_scrcmd.c SINNOH_WARP_CONTINUE)
            go = '%s%s__warped%d' % (PREFIX, self.low, self.next_local())
            if self.is_var(a[1]) and self.is_var(a[2]):
                # (a place the script worked out, Platinum's tiles: GetPlayerMapPos, the warp in place)
                x, y = self.var(a[1]), self.var(a[2])
                return ['subvar %s, %d' % (x, o[1]), 'subvar %s, %d' % (y, o[2]), self.sinnoh(SINNOH_WARP_CONTINUE),
                        'goto %s' % go, 'warp %s, %s, %s' % (o[0], x, y), 'waitstate', '%s:' % go]
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
            lines = ['%sflag 0x%X' % ('set' if cmd == 'SetHasPartner' else 'clear', FLAG_SINNOH_HAS_PARTNER)]
            if cmd == 'ClearHasPartner':   # (and the walk ends: the one who followed stops, as Platinum's)
                lines.append(self.sinnoh(SINNOH_MISC, 24))
            return lines
        if cmd == 'PlayHallOfFameHealingAnimation':
            return ['playse SE_USE_ITEM', 'delay 60']   # (Platinum's machine heals and records the team; ClearGame next)
        if cmd == 'ClearGame':
            # SINNOH's HALL OF FAME: the league record, the boat ticket home, the LEAGUE as the place to wake up, then
            # the game's own HALL OF FAME and credits (special GameClear: SINNOH cleared, src/mp/mp_region.c)
            return ['call Quetzal_EventScript_835F6E4', 'call EventScript_SinnohTicket',
                    'setrespawn HEAL_LOCATION_SINNOH_POKEMON_LEAGUE', 'fadescreenspeed 1, 24', 'special GameClear',
                    'waitstate']
        if cmd == 'EnableHiddenLocation':
            return []                       # (FULLMOON / NEWMOON ISLAND, the SPRING and SEABREAK PATHS: open in this game)
        if cmd == 'CheckHasPartner':
            # (someone walks with the player: no SURF or ROCK CLIMB then, as Platinum's)
            done = '%s%s__partner%d' % (PREFIX, self.low, self.next_local())
            v = self.var(a[0])
            return ['setvar %s, 0' % v, 'goto_if_unset 0x%X, %s' % (FLAG_SINNOH_HAS_PARTNER, done), 'setvar %s, 1' % v,
                    '%s:' % done]
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
            # (FLAG_SINNOH_STARTER; Platinum's START menu shows POKeMON once the starter is saved: the game's flag)
            return ['copyvar 0x%X, VAR_RESULT' % VAR_SINNOH_STARTER, 'setflag 0x3D39', 'setflag FLAG_SYS_POKEMON_GET']
        if cmd == 'ReturnToField':
            return []                       # (the briefcase went back to the field itself)
        if cmd == 'StartLibraryTV':
            # CANALAVE LIBRARY's news of LAKE VALOR (Platinum's src/library_tv: a fade in, LIBRARY_TV_DURATION 240
            # frames, a fade out) while the field is faded out; its picture isn't drawn yet (docs/SINNOH_CHECKLIST.md)
            return ['delay 252']
        if cmd in ('BufferRivalStarterSpeciesName', 'BufferRivalStarterSpeciesNameWithArticle'):
            return [self.sinnoh(SINNOH_BUFFER_RIVAL_STARTER, self.value(a[0]))]
        if cmd in ('BufferPlayerCounterpartStarterSpeciesName', 'BufferPlayerCounterpartStarterSpeciesNameWithArticle'):
            # (the counterpart's: the starter neither the player nor BARRY took; English has no article to add)
            return [self.sinnoh(SINNOH_BUFFER_RIVAL_STARTER, self.value(a[0]), 1)]
        if cmd == 'GiveRunningShoes':
            return ['setflag FLAG_SYS_B_DASH']
        if cmd == 'GiveJournal':
            return ['additem ITEM_JOURNAL', 'special MP_GatchaTicketToast', self.sinnoh(SINNOH_JOURNAL, 255)]
        if cmd == 'CreateJournalEvent':
            if a and a[0] in JOURNAL_SCRIPT_EVENTS:
                param = '0'
                if a[0] == 'LOCATION_EVENT_ITEM_WAS_OBTAINED' and len(a) > 1:
                    param = self.var(a[1]) if a[1].startswith('VAR_') else self.value(a[1])
                return [self.sinnoh(SINNOH_JOURNAL, JOURNAL_SCRIPT_EVENTS[a[0]], param)]
            return []
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
                   'InitPersistedMapFeaturesForCanalaveGym', 'InitPersistedMapFeaturesForEternaGym',
                   'InitPersistedMapFeaturesForVeilstoneGym', 'InitPersistedMapFeaturesForHearthomeGym'):
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
            return ([self.sinnoh(SINNOH_CONTEST, CC_PARTY_ANY_RANK, self.var(a[3]))] if len(a) > 3 else []) + \
                   [self.sinnoh(SINNOH_CONTEST, 23, self.arg(a[1]), self.arg(a[2])), 'special ChoosePartyMon',
                    'waitstate', self.sinnoh(SINNOH_CONTEST, 24)]
        if cmd in ('StartBattleClient', 'StartBattleServer') and self.low == 'contests':
            # SUPER CONTESTS with friends (src/sinnoh/s3d_contest_link.c): BECOME LEADER gathers the friends in the room
            # in Platinum's group words (its unk_0353 bank), JOIN GROUP joins the first one leading; VAR_RESULT is
            # Platinum's COMM_CLUB_RET_* (0 the group's contest begins, 1 cancelled, 3 nobody left)
            n = self.next_local()
            loop, cancel, ok, done = ('%s%s__link%s%d' % (PREFIX, self.low, k, n) for k in ('Loop', 'Cancel', 'Ok', 'Done'))
            dest = self.var(a[-1])
            if cmd == 'StartBattleServer':
                return [self.sinnoh(SINNOH_CONTEST, CC_LINK_LEAD), '%s:' % loop,
                        self.sinnoh(SINNOH_CONTEST, CC_LINK_TEXT, 3), 'message gStringVar4', 'waitmessage',
                        'special Sinnoh_ContestLinkWait', 'waitstate', 'goto_if_eq VAR_RESULT, 1, %s' % cancel,
                        'message %s' % self.text_label('pl_msg_00000353_00035'), 'waitmessage', 'yesnobox 20, 8',
                        'goto_if_eq VAR_RESULT, 0, %s' % loop, self.sinnoh(SINNOH_CONTEST, CC_LINK_START, dest),
                        'closemessage', 'goto %s' % done, '%s:' % cancel,
                        'message %s' % self.text_label('pl_msg_00000353_00039'), 'waitmessage', 'yesnobox 20, 8',
                        'goto_if_eq VAR_RESULT, 0, %s' % loop, self.sinnoh(SINNOH_CONTEST, CC_LINK_END),
                        'setvar %s, 1' % dest, 'closemessage', '%s:' % done]
            return [self.sinnoh(SINNOH_CONTEST, CC_LINK_JOIN),
                    self.sinnoh(SINNOH_CONTEST, CC_LINK_TEXT, 4), 'message gStringVar4', 'waitmessage',
                    'special Sinnoh_ContestLinkWait', 'waitstate', 'goto_if_eq VAR_RESULT, 0, %s' % ok,
                    self.sinnoh(SINNOH_CONTEST, CC_LINK_END), 'setvar %s, 1' % dest, 'closemessage',
                    'goto %s' % done, '%s:' % ok, 'setvar %s, 0' % dest, 'closemessage', '%s:' % done]
        if cmd in ('StartBattleClient', 'StartBattleServer') and self.low == 'poffin_common':
            # (COMM_CLUB_RET_CANCEL: group cooking is the BERRY BLENDER's own with friends, CO-OP > BERRY GAMES)
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
        if cmd == 'ChooseTwoCustomMessageWords':
            # (two words on the game's own easy chat, its questionnaire's: JUBILIFE TV's 3F)
            return ['special Sinnoh_TvChooseTwoWords', 'waitstate',
                    self.sinnoh(SINNOH_TV, TVCALL_TWO_WORDS, self.var(a[0]), self.var(a[1])),
                    self.sinnoh(SINNOH_TV, TVCALL_SECOND_WORD, self.var(a[2]))]
        if cmd == 'GetWallpaperFromCustomMessageWords':
            # (Platinum's eight extra PC wallpapers: the game's PC has its own, so no phrase unlocks one: 0xFF)
            return ['setvar %s, 0xFF' % self.var(a[0])]
        if cmd == 'CheckIsMysteryGiftPhrase':
            # (the game's own MYSTERY GIFT phrase in 0x8000-0x8003: s3d_tv.c TVCALL_GIFT_PHRASE)
            if a[1:5] != ['VAR_0x8000', 'VAR_0x8001', 'VAR_0x8002', 'VAR_0x8003']:
                return None
            return [self.sinnoh(SINNOH_TV, TVCALL_GIFT_PHRASE, self.var(a[0]))]
        if cmd == 'UnlockMysteryGift':
            return [self.sinnoh(SINNOH_TV, TVCALL_UNLOCK_GIFT)]
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
        # ROUTE 224's tablet (OAK comes only with the OAK'S LETTER, a distribution event: closed, so its tablet stays
        # unmarked); its two commands, for the scripts to convert whole: the name thanked is the player's, the
        # writing never chosen (Platinum's naming screen's "no one")
        if cmd == 'BufferTabletName':
            return [self.sinnoh(SINNOH_BUFFER_PLAYER, self.value(a[0]))]
        if cmd == 'OpenShayminTabletNamingScreen':
            return ['setvar %s, 1' % self.var(a[0])]
        if cmd == 'GetRandomBattlegroundTrainers':
            # the BATTLEGROUND's trainers of the day (Platinum's ScrCmd_GetRandomBattlegroundTrainers: three gym
            # leaders, one of the five partners; BATTLEGROUND_TRAINER_NONE for a place left empty): s3d_misc.c 27-28
            return [self.sinnoh(SINNOH_MISC, 27, self.var(a[0]), self.var(a[1])),
                    self.sinnoh(SINNOH_MISC, 28, self.var(a[2]), self.var(a[3]))]
        if cmd == 'SetPlayerVolume':
            return []   # (the music's player volume through a scene: the FadeOutBGM / FadeInBGM around it do the work)
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
            if getattr(self, 'faded_in', False):
                self.faded_in = False
                return []                   # (the slot machine came back to the field faded in)
            return ['fadescreen FADE_FROM_BLACK']
        if cmd == 'ScrCmd_267':
            # VEILSTONE GAME CORNER's slot machine (its number 0-11): Emerald's slots, the machine's own odds
            # (GetSlotMachineId), coming back to the field faded in
            self.faded_in = True
            return ['copyvar VAR_0x8004, %s' % self.var(a[0]), 'specialvar VAR_RESULT, GetSlotMachineId',
                    'playslotmachine VAR_RESULT']
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
            # (every movement started, as Platinum waits: not only the last one, which Kai's waitmovement 0 is. A longer
            # one went on past the script's end and was cut short: ROUTE 205's grunt never got back to his post)
            return ['waitmovement LOCALID_ALL_MOVING']
        if cmd == 'ShakeObject':
            # (the object shakes side to side TIMES times by XOFFSET pixels: s3d_misc.c op 26; the script waits)
            times, xoff = int(self.value(a[1])), int(self.value(a[3]))
            return [self.sinnoh(SINNOH_MISC, 26, self.localid(a[0]), (times << 8) | (xoff & 0xFF)), 'delay %d' % (times * 4)]
        if cmd == 'RemoveObject':
            return ['removeobject %s' % self.localid(a[0])]
        if cmd == 'AddObject':
            return ['addobject %s' % self.localid(a[0])]
        if cmd in ('Delay8', 'Delay4', 'Delay2', 'Delay1', 'Delay16', 'Delay15', 'Delay32'):
            return ['delay %d' % (2 * int(cmd[len('Delay'):]))]
        if cmd == 'WaitTime':
            return ['delay %d' % (2 * int(self.value(a[0])))]
        if cmd == 'ScrCmd_29F':
            # (the ground shakes: the REGI statue waking, LAKE VALOR's blast felt in CANALAVE and PASTORIA)
            return [self.sinnoh(SINNOH_QUAKE, self.value(a[0]))]
        if cmd == 'ScrCmd_20D':
            # (SPEAR PILLAR's field effects, Platinum's ov6_02243004: 0 the world starting to shake as the Red Chain
            # binds DIALGA and PALKIA, 4 the distorted pillar's flash; 1 and 6 ask whether it has finished, 2-3, 5 and
            # 7 on start / stop others. Here the ground shakes, the flash is a white one, and each has finished)
            n = int(self.value(a[0]))
            if n in (1, 6):
                return ['setvar %s, 1' % self.var(a[1])]
            if n == 0:
                return [self.sinnoh(SINNOH_QUAKE, 1)]
            if n == 4:
                return ['fadescreen FADE_TO_WHITE', 'fadescreen FADE_FROM_WHITE']
            return []
        if cmd == 'GetGameVersion':
            # (this is Platinum's story: always VERSION_PLATINUM, so its own lines are picked)
            return ['setvar %s, %s' % (self.var(a[0]), self.value('VERSION_PLATINUM'))]
        if cmd == 'SetSpeciesSeen':
            return [self.sinnoh(SINNOH_MISC, 40, self.value(a[0]))]
        if cmd == 'ScrCmd_2FB':
            # (SPEAR PILLAR's cutscene, cutscenes/spear_pillar: GIRATINA's shadow tearing the pillar; not built: the screen
            # goes white as it begins and comes back, then the script warps to the distorted pillar)
            return ['fadescreen FADE_TO_WHITE', 'delay 60', 'fadescreen FADE_FROM_WHITE']
        if cmd == 'ScrCmd_2B6':
            return []                       # (a person's MAP_OBJ_STATUS_18 bit, sub_02062D80: nothing drawn differs here)
        if cmd == 'DeactivateLakeGuardianContainmentUnits':
            # (GALACTIC HQ's three tubes open, lake_guardian_containment_units.c; the script goes on to free the trio)
            return [self.sinnoh(SINNOH_QUAKE, 0)]
        if cmd == 'CheckPartyHasFatefulEncounterRegigigas':
            # (Platinum's REGIGIGAS from its distribution, a DS event: here any REGIGIGAS, SNOWPOINT TEMPLE's after the
            # LEAGUE, as the REGI ruins have always worked here: s3d_legends.c)
            return ['specialvar %s, Sinnoh_PartyHasRegigigas' % self.var(a[0])]
        if cmd == 'ActivateRegiRuinsDot':
            # (the dot under the player: Platinum's ruins' tables, src/sinnoh/s3d_legends.c Sinnoh_RegiDot)
            return [self.sinnoh(SINNOH_REGI_DOT, self.var(a[0]), self.value(a[1]))]
        if cmd in ('StopSE', 'WaitForTransition', 'SetInitialVolumeForSequence', 'SetBlackOutWarpId', 'SetSubScene63'):
            return []                       # (sounds stop by themselves here; the game picks the heal spot itself)
        if cmd in ('TurnOnTimeBasedEffects', 'TrySetUnusedCollectedOrbFlag', 'RecordHeapMemory',
                   'CheckHeapMemory', 'SetMenuXOriginToRight', 'SetMenuXOriginToLeft'):
            return []                       # (Platinum's journal, its memory checks: nothing here)
        out = self.tower_command(cmd, a)
        if out is not None:
            return out
        raise Unsupported(cmd)

    # ---- Platinum's BATTLE TOWER (src/sinnoh/s3d_tower.c; docs/SINNOH_TOWER_PORT.md)
    def bt_func(self, name):
        if getattr(self.dec, '_bt_funcs', None) is None:
            text = open(os.path.join(self.root, 'include/constants/battle_tower_functions.h')).read()
            self.dec._bt_funcs, n = {}, 0
            for item in re.search(r'enum BattleTowerFunction \{(.*?)\}', text, re.S).group(1).split(','):
                name_, _, val = item.strip().partition('=')
                if not name_.strip():
                    continue
                n = int(val.strip(), 0) if val.strip() else n
                self.dec._bt_funcs[name_.strip()] = n
                n += 1
        if name not in self.dec._bt_funcs:
            raise Unsupported('battle tower function %s' % name)
        return self.dec._bt_funcs[name]

    def tutor_location(self, x):
        names = [n.strip() for n in open(os.path.join(self.root, 'generated/tutor_locations.txt')) if n.strip()]
        if self.is_var(x):
            return self.var(x)
        if x not in names:
            raise Unsupported('tutor %s' % x)
        return str(names.index(x))

    def party(self, op, a=0, b=0):
        return self.sinnoh(SINNOH_PARTY, op, a, b)

    def party_command(self, cmd, a):
        """Platinum's party and BAG queries (src/sinnoh/s3d_party.c), None: not one"""
        if cmd == 'CountRepeatedSpeciesInParty':
            return [self.party(PARTY_REPEATED_SPECIES, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetPartyMonRibbon':
            return [self.party(PARTY_ARG, self.value(a[2])), self.party(PARTY_GET_RIBBON, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'SetPartyMonRibbon':
            return [self.party(PARTY_ARG, self.value(a[1])), self.party(PARTY_SET_RIBBON, self.arg(a[0]))]
        if cmd == 'IncreasePartyMonFriendship':
            return [self.party(PARTY_FRIENDSHIP, self.arg(a[0]), self.arg(a[1]))]
        if cmd == 'GetPartyMonLevel':   # (destVarID, slot)
            return [self.party(PARTY_LEVEL, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetPartyMonNature':
            return [self.party(PARTY_NATURE, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'FindPartySlotWithNature':
            return [self.party(PARTY_FIND_NATURE, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetPartyMonType':
            return [self.party(PARTY_ARG, self.arg(a[2])), self.party(PARTY_TYPES, self.var(a[0]), self.var(a[1]))]
        if cmd == 'CalcHiddenPowerType':
            return [self.party(PARTY_HIDDEN_POWER, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'GetPartyMonContestStat':
            return [self.party(PARTY_ARG, self.arg(a[1])), self.party(PARTY_CONTEST_STAT, self.arg(a[0]), self.var(a[2]))]
        if cmd == 'CheckPartyCombeeGenderCount':
            return [self.party(PARTY_COMBEE_GENDERS, 0, self.var(a[0]))]
        if cmd == 'GetPartyMonMoveCount':
            return [self.party(PARTY_MOVE_COUNT, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetItemQuantity':
            return [self.party(PARTY_ITEM_QUANTITY, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'GetFossilCount':
            return [self.party(PARTY_FOSSIL_COUNT, 0, self.var(a[0]))]
        if cmd == 'FindFossilAtThreshold':
            return [self.party(PARTY_ARG, self.var(a[1])), self.party(PARTY_FOSSIL_FIND, self.arg(a[2]), self.var(a[0]))]
        if cmd == 'GetSpeciesFromFossil':
            return [self.party(PARTY_FOSSIL_SPECIES, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetPartyMonFootprintType':
            return [self.party(PARTY_ARG, self.var(a[1])), self.party(PARTY_FOOTPRINT, self.arg(a[2]), self.var(a[0]))]
        if cmd == 'CheckIsPartyMonOutsider':
            return [self.party(PARTY_OUTSIDER, self.var(a[0]), self.var(a[1]))]
        if cmd == 'CheckHasSeenSpecies':
            return [self.party(PARTY_SEEN, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'GoToIfCannotAddCoins':
            return [self.party(PARTY_CAN_ADD_COINS, self.arg(a[0]), 'VAR_RESULT'),
                    'goto_if_eq VAR_RESULT, FALSE, %s' % self.label(a[1])]
        if cmd == 'ShowCoins':
            return ['showcoinsbox 1, 1']
        if cmd == 'HideCoins':
            return ['hidecoinsbox 0, 0']
        if cmd == 'UpdateCoinDisplay':
            return ['updatecoinsbox 1, 1']
        if cmd == 'CheckBonusRoundStreak':
            # (the GAME CORNER's slots are the game's: they keep the most BONUS hits of one sitting in Platinum's
            # VAR_CONSECUTIVE_BONUS_ROUND_WINS, src/slot_machine.c SlotTask_EndGame; ten or more: TRUE)
            skip = '%s%s__bonus%d' % (PREFIX, self.low, self.next_local())
            return ['setvar %s, FALSE' % self.var(a[0]),
                    'compare_var_to_value %s, 10' % self.var('VAR_CONSECUTIVE_BONUS_ROUND_WINS'), 'goto_if 0, %s' % skip,
                    'setvar %s, TRUE' % self.var(a[0]), '%s:' % skip]
        if cmd == 'GetOverworldWeather':
            return [self.party(PARTY_WEATHER, 0, self.var(a[0]))]
        if cmd in ('AddMoney', 'RemoveMoney', 'RemoveMoney2') and self.is_var(a[0]):
            return [self.party(PARTY_ADD_MONEY if cmd == 'AddMoney' else PARTY_REMOVE_MONEY, self.var(a[0])),
                    'updatemoneybox']
        if cmd == 'SelectPartyMonMove':
            # (Emerald's move deleter's summary screen, picking a move: its answer in VAR_0x8005)
            return ['copyvar VAR_0x8004, %s' % self.var(a[0]) if self.is_var(a[0]) else 'setvar VAR_0x8004, %s' % self.value(a[0]),
                    'special MoveDeleterChooseMoveToForget', 'waitstate']
        if cmd == 'GetSelectedPartyMonMove':
            return [self.party(PARTY_SELECTED_MOVE, 0, self.var(a[0]))]
        if cmd == 'BufferPartyMoveName':
            return [self.party(PARTY_ARG, self.value(a[0])), self.party(PARTY_BUFFER_MOVE, self.arg(a[1]), self.var(a[2]))]
        if cmd == 'ClearPartyMonMoveSlot':
            return [self.party(PARTY_FORGET_MOVE, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'CheckPartyMonHasMove':
            return [self.party(PARTY_ARG, self.value(a[1])), self.party(PARTY_HAS_MOVE, self.arg(a[2]), self.var(a[0]))]
        if cmd == 'GetPartyMonEVTotal':
            return [self.party(PARTY_EV_TOTAL, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'BufferNatureName':
            return [self.party(PARTY_ARG, self.value(a[0])), self.party(PARTY_BUFFER_NATURE, self.arg(a[1]))]
        if cmd == 'BufferRibbonName':
            # (Platinum's ribbon's name, its ribbon.c nameID in the ribbon_names bank)
            name = ribbon_name_id(self.root, a[1])
            where = self.dec.message('ribbon_names', name) if name else None
            if where is None:
                raise Unsupported('ribbon %s' % a[1])
            return [self.sinnoh(SINNOH_BUFFER_TEXT, self.value(a[0]), where[0], where[1])]
        if cmd == 'OpenSummaryScreenTeachMove':
            return [self.party(PARTY_ARG, self.arg(a[1])), self.party(PARTY_TEACH_MOVE, self.arg(a[0])), 'waitstate']
        if cmd == 'GetPartyMonMove':
            return [self.party(PARTY_ARG, self.arg(a[2])), self.party(PARTY_GET_MOVE, self.arg(a[1]), self.var(a[0]))]
        if cmd == 'GetSummarySelectedMoveSlot':
            return [self.party(PARTY_TEACH_SLOT, 0, self.var(a[0]))]
        # (in-game trades: Platinum's own, src/data/sinnoh_ingame_trades.h, through the game's trade scene)
        if cmd == 'SelectPokemonToTrade':
            return [self.sinnoh(SINNOH_CONTEST, 32), 'special ChoosePartyMon', 'waitstate',
                    self.sinnoh(SINNOH_CONTEST, 33), self.sinnoh(SINNOH_CONTEST, 34, self.var('VAR_RESULT'))]
        if cmd == 'InitNPCTrade':
            t = open(os.path.join(self.root, 'include/constants/npc_trades.h')).read()
            names = [x.split('=')[0].strip() for x in re.search(r'enum NPCTradeID \{(.*?)\}', t, re.S).group(1).split(',')]
            if a[0] not in names:
                raise Unsupported('trade %s' % a[0])
            return [self.party(PARTY_TRADE_INIT, names.index(a[0]))]
        if cmd == 'GetNPCTradeRequestedSpecies':
            return [self.party(PARTY_TRADE_REQUESTED, 0, self.var(a[0]))]
        if cmd == 'StartNPCTrade':
            return [self.party(PARTY_TRADE_START, self.arg(a[0])), 'special CreateInGameTradePokemon',
                    'special DoInGameTradeScene', 'waitstate', self.party(PARTY_TRADE_END, 1)]
        if cmd == 'FinishNPCTrade':
            return [self.party(PARTY_TRADE_END)]
        if cmd == 'TurnOnPokedexFormDetection':
            # (the POKeDEX's FORMS: SELECT on UNOWN's page, the letters seen; src/pokedex.c MpDexFormsInput)
            return [self.party(PARTY_FORMS_ON)]
        if cmd == 'TurnOnPokedexLanguageDetection':
            # (the game's POKeDEX has no foreign-language pages, left out by the user's choice: the MEISTER's flag
            # is still set beside it)
            return []
        if cmd == 'BufferTypeName':
            return [self.party(PARTY_ARG, self.value(a[0])), self.party(PARTY_BUFFER_TYPE, self.arg(a[1]))]
        if cmd == 'AddCoins':
            return ['addcoins %s' % self.arg(a[0])]
        if cmd == 'SubtractCoins':
            return ['removecoins %s' % self.arg(a[0])]
        if cmd == 'HasCoins':
            return [self.party(PARTY_HAS_COINS, self.arg(a[1]), self.var(a[0]))]
        # (Platinum's move tutors: src/data/sinnoh_tutors.h; the tutor by Platinum's TUTOR_LOCATION_*)
        if cmd in ('CheckHasLearnableTutorMoves', 'ShowMoveTutorMoveSelectionMenu'):
            tutor = self.tutor_location(a[1])
            if cmd == 'CheckHasLearnableTutorMoves':
                return [self.party(PARTY_ARG, tutor), self.party(PARTY_TUTOR_CAN, self.arg(a[0]), self.var(a[2]))]
            slot = '255' if a[0] == 'PARTY_SLOT_NONE' else self.arg(a[0])
            return [self.party(PARTY_ARG, tutor), self.party(PARTY_TUTOR_MENU, slot, self.var(a[2])),
                    self.sinnoh(SINNOH_MENU_SHOW)]
        if cmd == 'CheckCanAffordMove':
            return [self.party(PARTY_TUTOR_AFFORD, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'PayShardCost':
            return [self.party(PARTY_TUTOR_PAY, self.arg(a[0]))]
        if cmd == 'ResetMoveSlot':
            # (the scripts give the party slot, the move slot, then the move: what Platinum's code does with them)
            return [self.party(PARTY_ARG, self.arg(a[1])),
                    self.party(PARTY_RESET_MOVE, self.arg(a[0]), self.var(a[2]) if self.is_var(a[2]) else self.value(a[2]))]
        if cmd == 'ShowShardCost':
            first = self.dec.ids('menu_entries').get('pl_msg_00000361_00273')
            if first is None:
                raise Unsupported('the SHARDS lines')
            return [self.party(PARTY_ARG, self.dec.banks['menu_entries']), self.party(PARTY_SHARD_COST, self.arg(a[2]), first)]
        if cmd == 'OpenMoveTutorMenu':
            # (Platinum's move-learning screen for one move: the game's move relearner offering only it)
            return [self.party(PARTY_ARG, self.arg(a[1])), self.party(PARTY_LEARN_OPEN, self.arg(a[0])),
                    'special TeachMoveRelearnerMove', 'waitstate']
        if cmd == 'TryBufferAndUnlockRandomToughWord':
            # (-1: none left, as Platinum's)
            return [self.party(PARTY_TOUGH_WORD, self.value(a[1]), self.var(a[0]))]
        if cmd == 'CheckAllToughWordsUnlocked':
            return [self.party(PARTY_TOUGH_ALL, 0, self.var(a[0]))]
        if cmd == 'CheckLearnedTutorMove':
            return [self.party(PARTY_LEARN_RESULT, 0, self.var(a[0]))]
        if cmd == 'CloseShardCostWindow':
            return [self.party(PARTY_SHARD_CLOSE)]
        if cmd == 'GetGameCornerPrizeData':
            # (VEILSTONE's prizes: Platinum's table, src/sinnoh/s3d_party.c sPrizes)
            return [self.party(PARTY_ARG, self.var(a[2])), self.party(PARTY_PRIZE, self.arg(a[0]), self.var(a[1]))]
        if cmd == 'GoToIfNotEnoughMoney' and self.is_var(a[0]):
            return [self.party(PARTY_ENOUGH_MONEY, self.var(a[0]), 'VAR_RESULT'),
                    'goto_if_eq VAR_RESULT, FALSE, %s' % self.label(a[1])]
        return None

    def tower(self, func, arg=0, dest=0):
        return self.sinnoh(SINNOH_TOWER, func, arg, dest)

    def tower_command(self, cmd, a):
        """the BATTLE TOWER's commands, None: not one"""
        if cmd == 'InitBattleTower':
            if self.value(a[0]) != '0':
                return [self.tower(TOWER_INIT_RESUME)]
            return [self.tower(TOWER_INIT, self.value(a[1]) if a[1] != '0xFFFF' else 0)]
        if cmd == 'SetBattleTowerNull':
            return []                       # (Platinum's check that no challenge was left: the game's own state)
        if cmd in ('FreeBattleTower', 'FreeTowerStruct'):
            return [self.tower(TOWER_FREE)]
        if cmd == 'CallBattleTowerFunction':
            f = self.bt_func(a[0])
            dest = self.var(a[2]) if self.is_var(a[2]) or a[2].startswith('FR_VAR_') else 0
            return [self.tower(f, self.arg(a[1]), dest)]
        if cmd in ('PrintBattleTowerIntroMessage', 'FrontierScrCmd_85'):
            return [self.tower(TOWER_INTRO, self.value(a[0])), 'message gStringVar4', 'waitmessage']
        if cmd in ('BattleTower_StartBattle',):
            return [self.tower(TOWER_BATTLE)]
        if cmd == 'BattleTower_CheckWonBattle':
            return [self.tower(TOWER_WON, 0, self.var(a[0]))]
        if cmd == 'MessageSeenBanlistSpecies':
            # (Platinum's English line names only the number needed)
            return ['buffernumberstring STR_VAR_1, %s' % self.value(a[1]), 'message %s' % self.text_label(a[0]),
                    'waitmessage']
        if cmd == 'TryRevertPokemonForm':
            return ['setvar %s, 0' % self.var(a[1])]   # (no GRISEOUS ORB form to change back here)
        if cmd in ('ScrCmd_1DF', 'ScrCmd_1E0'):
            # (TEALA's trophies are the Underground's goods for a secret base: closed, so she has none to give)
            return ['setvar %s, 0' % self.var(a[0])]
        if cmd == 'TrySaveGame':
            return [self.tower(TOWER_SAVE), 'setvar %s, 1' % self.var(a[0])]
        if cmd == 'GetBattleTowerPartnerSpeciesAndMove':
            # (a BATTLE SALON partner's POKéMON n: its kind and first move, of the team made for this challenge)
            n = int(self.value(a[1]))
            return [self.tower(TOWER_PARTNER_SPECIES + n, self.arg(a[0]), self.var(a[2])),
                    self.tower(TOWER_PARTNER_MOVE + n, self.arg(a[0]), self.var(a[3]))]
        if cmd in ('StartBattleServer', 'StartBattleClient'):
            return ['setvar %s, %d' % (self.var(a[3]), self.dec.consts.get('COMM_CLUB_RET_CANCEL', 1))]
        if cmd == 'ScrCmd_1E1':
            return ['setvar %s, 0' % self.var(a[2])]   # (the wireless partner's data: never asked here)
        if cmd in ('EndCommunication', 'ClearReceivedTempDataAllPlayers', 'ScrCmd_135'):
            return []   # (the BATTLE TOWER's wireless multi battles: never started here)
        if cmd == 'StartRankingsMachine':
            # (JUBILIFE TV's rankings machines: src/sinnoh/s3d_rankings.c; the script waits until it closes)
            return [self.tower(TOWER_RANKINGS, self.value(a[0]))]
        if cmd == 'JudgeStats':
            # (the BATTLE TOWER's STATS JUDGE: party slot A0's IVs: their total, the best stat as the line that names
            # it, Platinum's message 122 + the stat, and its IV)
            return [self.tower(TOWER_JUDGE_TOTAL, self.arg(a[0]), self.var(a[1])),
                    self.tower(TOWER_JUDGE_STAT, self.arg(a[0]), self.var(a[2])),
                    self.tower(TOWER_JUDGE_BEST, self.arg(a[0]), self.var(a[3]))]
        if cmd == 'ScrCmd_0B3':
            return ['setvar %s, 0' % self.var(a[0])]   # (the Wi-Fi rooms' upload: online, closed)
        if cmd == 'ScrCmd_18C':
            # (a person turned to face a way: Platinum's DIR_NORTH, SOUTH, WEST, EAST as the game's)
            d = int(self.value(a[1]))
            return ['turnobject %s, %s' % (self.localid(a[0]), ['DIR_NORTH', 'DIR_SOUTH', 'DIR_WEST', 'DIR_EAST'][d])]
        if cmd in ('ShowSavingIcon', 'HideSavingIcon'):
            return []
        if cmd == 'LaunchBattleFrontierScene':
            # (Platinum's frontier scene: its map, the frontier script going on there; ExitFrontier comes back here)
            low = FRONTIER_SCENES.get(a[0])
            if a[0] in FRONTIER_CLOSED_SCENES:
                return ['end']
            if low is None:
                raise Unsupported('frontier scene %s' % a[0])
            c = converter(self.root, low)
            m, x, y = c.start()
            self.calls.add((low, 1))
            # (the scene's ExitFrontier comes back to the first script that starts it: the Wi-Fi rooms' are closed)
            first = next(lab for lab in self.order for c2, a2 in self.labels[lab]
                         if c2 == cmd and a2[:1] == a[:1])
            here = getattr(self, 'cur_label', first)
            back = '%s%s__frexit_%s%s' % (PREFIX, self.low, a[0], '' if here == first else '_' + here)
            return [self.sinnoh(SINNOH_WARP_CONTINUE), 'goto %s' % c.entry_label(1), 'warp %s, %d, %d' % (m, x, y),
                    'waitstate', '%s:' % back]
        return None

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
        if (self.low, n) in HOLD:
            return HOLD[(self.low, n)]
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
            self.cur_label = lab
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


# ------------------------------------------------------------------------------------------- the frontier scripts
# Platinum's BATTLE FRONTIER scenes (overlay104: a second script language, res/field/frontier_scripts) run here on the
# maps of the same rooms: the scene's people are the map's (FRONTIER_ACTORS: actor -> this game's object, its look's
# VAR_OBJ_GFX_ID_n, or the player), its positions the tiles under them (x / 16 - 1, the y rounded up / 16 - 1), its
# variables the map's own VAR_TEMP_*; LaunchBattleFrontierScene / ChangeScript warp to a scene's map and go on in its
# script there (SINNOH_WARP_CONTINUE), ExitFrontier goes back after the LaunchBattleFrontierScene that started it

FRONTIER_SCENES = {'FRONTIER_SCENE_TOWER_CORRIDOR': 'fr_battle_tower_corridor',
                   'FRONTIER_SCENE_TOWER_BATTLE_ROOM': 'fr_battle_tower_battle_room',
                   'FRONTIER_SCENE_TOWER_MULTI_CORRIDOR': 'fr_battle_tower_corridor_multi',
                   'FRONTIER_SCENE_TOWER_MULTI_BATTLE_ROOM': 'fr_battle_tower_multi_battle_room'}
# each scene file: (the map's Platinum header, {actor: 'player' / (local id, its look's variable number or None) / None:
# not shown (Platinum's hidden camera holder, a link partner)}, the tile under a scene position: the single rooms'
# maps lie a tile apart from their scenes' pictures, the multi rooms' as they are)
_SINGLE_TILE = lambda x, y: (x // 16 - 1, -(-y // 16) - 1)
_MULTI_TILE = lambda x, y: (x // 16, y // 16 - 1)
FRONTIER_ACTORS = {
    'fr_battle_tower_corridor': ('battle_tower_corridor', {0: 'player', 1: ('1', None)}, _SINGLE_TILE),
    'fr_battle_tower_battle_room': ('battle_tower_battle_room', {0: None, 1: ('1', None), 2: ('2', 0), 3: 'player'},
                                    _SINGLE_TILE),
    'fr_battle_tower_corridor_multi': ('battle_tower_corridor_multi', {0: None, 1: ('1', None), 2: ('2', None),
                                                                       3: 'player', 4: ('4', 1), 5: None, 6: None},
                                       _MULTI_TILE),
    'fr_battle_tower_multi_battle_room': ('battle_tower_multi_battle_room',
                                          {0: None, 1: ('1', None), 2: ('2', None), 3: ('6', 2), 4: ('5', 3),
                                           5: 'player', 6: ('4', 1), 7: None, 8: None}, _MULTI_TILE),
}
FRONTIER_SCENE_MAPS = {v[0] for v in FRONTIER_ACTORS.values()}
# the scenes of the Wi-Fi rooms (online: closed; only their link's scripts go there)
FRONTIER_CLOSED_SCENES = {'FRONTIER_SCENE_WFC_FACILITY_SELECTOR'}
# the actors a scene picks by a variable (its own setting; the link's, closed, set others)
FRONTIER_VAR_ACTORS = {'fr_battle_tower_corridor_multi': {'VAR_0x8002': 3, 'VAR_0x8003': 4},
                       'fr_battle_tower_multi_battle_room': {'VAR_0x8001': 5, 'VAR_0x8002': 6}}
# the field script and scene a frontier scene's ExitFrontier goes back to
FRONTIER_EXIT = {'fr_battle_tower_corridor': ('battle_tower_elevator', 'FRONTIER_SCENE_TOWER_CORRIDOR'),
                 'fr_battle_tower_battle_room': ('battle_tower_elevator', 'FRONTIER_SCENE_TOWER_CORRIDOR'),
                 'fr_battle_tower_corridor_multi': ('battle_tower_elevator', 'FRONTIER_SCENE_TOWER_MULTI_CORRIDOR'),
                 'fr_battle_tower_multi_battle_room': ('battle_tower_elevator', 'FRONTIER_SCENE_TOWER_MULTI_CORRIDOR')}
# the variables holding a sprite's number (Platinum's OBJ_EVENT_GFX_* order: a number set into them is a look)
FRONTIER_GFX_VARS = {'FR_VAR_0x800F', 'VAR_0x800B', 'VAR_0x8009'}
# Battle Videos: the scene's offers to record one are left out (no video recorder here)
FRONTIER_REWRITES = {
    'fr_battle_tower_battle_room': {
        '_0227': [('CallBattleTowerFunction', ['BT_FUNC_UPDATE_GAME_RECORDS_AND_JOURNAL', '0', 'VAR_0x8008']),
                  ('Message', ['BattleTowerBattleRoom_Text_BPFromTycoon']), ('Call', ['_0330']), ('CloseMessage', []),
                  ('GoTo', ['_03EF'])],
        '_0273': [('CallBattleTowerFunction', ['BT_FUNC_UPDATE_GAME_RECORDS_AND_JOURNAL', '0', 'VAR_0x8008']),
                  ('CallBattleTowerFunction', ['BT_FUNC_GET_CHALLENGE_MODE', '0', 'VAR_0x8008']),
                  ('GoToIfEq', ['VAR_0x8008', 'BATTLE_TOWER_MODE_5', '_02D4']),
                  ('Message', ['BattleTowerBattleRoom_Text_BPEarned']), ('Call', ['_0330']), ('CloseMessage', []),
                  ('GoTo', ['_03EF'])],
        '_02DE': [('CloseMessage', []), ('CallBattleTowerFunction', ['BT_FUNC_UPDATE_GAME_RECORDS', '0', 'VAR_0x8008']),
                  ('GoTo', ['_03EF'])],
    },
}


class FrontierConverter(FileConverter):
    def __init__(self, dec, path):
        self.dec, self.root, self.path = dec, dec.root, path
        self.low = 'fr_' + os.path.basename(path)[len('frontier_scripts_'):-2]
        text = open(path, encoding='utf-8', errors='replace').read()
        m = re.search(r'#\s*include "res/text/bank/(\w+)\.h"', text)
        self.bank = m.group(1) if m else None
        self.events_low, self.actors, self.tile = FRONTIER_ACTORS.get(self.low, (None, {}, _SINGLE_TILE))
        self.localids = {}
        self.labels, self.entries, self.order = parse(path)
        # (the data tables: the actors' lists, the sprites' resources)
        self.data, cur = {}, None
        for raw in text.splitlines():
            line = raw.split('//')[0].rstrip()
            mm = re.match(r'^(\w+):\s*$', line)
            if mm:
                cur = mm.group(1)
                continue
            mm = re.match(r'^\s+\.(short|byte)\s+(\S+)', line)
            if mm and cur:
                self.data.setdefault(cur, []).append((mm.group(1), int(mm.group(2), 0)))
        for lab in self.data:
            self.labels.pop(lab, None)
            if lab in self.order:
                self.order.remove(lab)
        for lab, body in FRONTIER_REWRITES.get(self.low, {}).items():
            self.labels[lab] = body
        self.texts = {}
        self._local = 0
        self.calls = set()
        self.moves = set()
        self._ok = {}

    def actor_table(self, label):
        """[(actor, sprite, direction, x, y, visible)] of a FrontierScrCmd_24 list"""
        d = self.data.get(label)
        if d is None:
            raise Unsupported('actor list %s' % label)
        out, i = [], 0
        while i < len(d) and not (d[i][0] == 'short' and d[i][1] == 0xFD13):
            vals = [v for _, v in d[i:i + 7]]
            if len(vals) < 7:
                raise Unsupported('actor list %s' % label)
            out.append(tuple(vals))
            i += 8 if vals[1] == 0xEEEF else 7   # (a link partner's entry has one byte more)
        return out

    def start(self):
        """(map, x, y) where the player stands as the scene starts: its first actor list's player"""
        o = self.dec.origins().get(self.events_low)
        if o is None:
            raise Unsupported('frontier scene map %s' % self.events_low)
        for lab in self.order:
            for cmd, a in self.labels[lab]:
                if cmd != 'FrontierScrCmd_24':
                    continue
                for actor, gfx, d, x, y, b1, vis in self.actor_table(a[0]):
                    if self.actors.get(actor) == 'player':
                        tx, ty = self.tile(x, y)
                        return o[0], tx, ty
        raise Unsupported('no player in %s' % self.low)

    def var(self, x):
        m = re.match(r'^(?:FR_)?VAR_0x80([0-9A-Fa-f]{2})$', x)
        if m:
            n = int(m.group(1), 16)
            if n > 0x10 or n in (0xD, 0xE) and not x.startswith('FR_'):
                raise Unsupported('frontier variable %s' % x)
            return '0x%X' % (0x4000 + {0xF: 0xD, 0x10: 0xE}.get(n, n))   # (the map's VAR_TEMP_*)
        return FileConverter.var(self, x)

    def is_var(self, x):
        return x.startswith(('VAR_', 'FR_VAR_'))

    def localid(self, x):
        if x in FRONTIER_VAR_ACTORS.get(self.low, {}):
            x = str(FRONTIER_VAR_ACTORS[self.low][x])
        a = self.actors.get(int(x, 0)) if re.match(r'^(0x[0-9A-Fa-f]+|\d+)$', x) else None
        if a == 'player':
            return 'OBJ_EVENT_ID_PLAYER'
        if isinstance(a, tuple):
            return a[0]
        raise Unsupported('frontier actor %s' % x)

    def movement(self, cmd, a):
        if cmd == 'WarpIn':
            return ['set_visible']
        if cmd == 'WarpOut':
            return ['set_invisible']
        m = re.match(r'^Walk(North|South|West|East)$', cmd)
        if m:
            n = int(a[0], 0) if a else 1
            return ['walk_' + DIRS[m.group(1)]] * n
        return FileConverter.movement(self, cmd, a)

    def gfx_value(self, n):
        """this game's sprite for Platinum's sprite number N"""
        path = os.path.join(self.root, 'generated/object_events_gfx.txt')
        names = [x.strip() for x in open(path) if x.strip()]
        if not 0 <= n < len(names):
            raise Unsupported('sprite number %d' % n)
        return _sprite(names[n])

    def command(self, cmd, a):
        if cmd == 'SetVar' and a[0] in FRONTIER_GFX_VARS and not self.is_var(a[1]):
            return ['setvar %s, %s' % (self.var(a[0]), self.gfx_value(int(self.value(a[1]))))]
        if cmd == 'InitGlobalTextListMenu':
            return [self.sinnoh(SINNOH_MENU_INIT, self.var(a[4]), 0)]
        if cmd == 'AddListMenuEntry':
            if a[0] == 'MenuEntries_Text_Record':
                return []                   # (Battle Videos: no video recorder here)
            where = self.dec.message(self.bank, a[0])
            if where is None:
                raise Unsupported('menu entry %s' % a[0])
            return [self.sinnoh(SINNOH_MENU_ADD, where[0], where[1], self.value(a[2]))]
        if cmd in ('SetMenuXOriginSide', 'SetMenuYOriginSide', 'FrontierScrCmd_22', 'FrontierScrCmd_23',
                   'FrontierScrCmd_2A', 'FrontierScrCmd_2B', 'FrontierScrCmd_2C', 'FrontierScrCmd_2D', 'FrontierScrCmd_31',
                   'FrontierScrCmd_32', 'FrontierScrCmd_33', 'FrontierScrCmd_2F', 'InitNewBattleRecording', 'FreeBattleRecording',
                   'IncrementRecordValue', 'IncrementTrainerScore', 'SyncConnectedPlayers',
                   'ClearReceivedTempDataAllPlayers', 'EndCommunication', 'FrontierScrCmd_3F',
                   'PlayFrontierBrainEncounterEffect', 'FrontierScrCmd_C8'):
            return []   # (the scene's sprite resources and door, the link's, the battle's lead-in: the game's own)
        if cmd == 'GetSystemVar':
            return ['copyvar %s, %s' % (self.var(a[1]), FileConverter.var(self, a[0]))]
        if cmd == 'SetSystemVar':
            return ['setvar %s, %s' % (FileConverter.var(self, a[0]), self.value(a[1]))]
        if cmd == 'PlaySoundEffect':
            se = SE.get(a[0])
            return ['playse %s' % se] if se else []
        if cmd == 'WaitSoundEffect':
            return ['waitse']
        if cmd == 'WaitTime':
            return ['delay %d' % (2 * int(self.value(a[0])))]
        if cmd in ('MessageNoSkip',):
            return ['message %s' % self.text_label(a[0]), 'waitmessage']
        if cmd == 'FrontierScrCmd_24':
            out = []
            for actor, gfx, d, x, y, b1, vis in self.actor_table(a[0]):
                who = self.actors.get(actor)
                tx, ty = self.tile(x, y)
                if who == 'player':
                    if not vis:
                        out += ['applymovement OBJ_EVENT_ID_PLAYER, SinnohP_Movement_Invisible', 'waitmovement 0']
                    continue
                if not isinstance(who, tuple):
                    continue
                if who[1] is not None:
                    v = '0x%X' % (0x4010 + who[1])
                    if 0x8000 <= gfx < 0x8020:
                        out.append('copyvar %s, %s' % (v, self.var('VAR_0x%04X' % gfx)))
                    elif gfx != 0xEEEE:
                        out.append('setvar %s, %s' % (v, self.gfx_value(gfx)))
                out += ['setobjectxyperm %s, %d, %d' % (who[0], tx, ty), 'addobject %s' % who[0]]
            return out
        if cmd == 'FrontierScrCmd_25':
            who = self.actors.get(int(self.value(a[0])))
            return ['removeobject %s' % who[0]] if isinstance(who, tuple) else []
        if cmd == 'FrontierScrCmd_70':
            return ['setvar %s, 2' % self.var(a[0])]   # (no Battle Video kept)
        if cmd == 'FrontierScrCmd_89':
            return ['setvar %s, 1' % self.var(a[2])]   # (a link partner's answer arrived: no link here)
        if cmd == 'FrontierScrCmd_8A':
            return ['setvar %s, 0' % self.var(a[1])]
        if cmd in ('FrontierScrCmd_71', 'FrontierScrCmd_4B'):
            return ['setvar %s, 0' % self.var(a[0])]
        if cmd == 'FrontierScrCmd_6D':
            return ['setvar %s, 0' % self.var(a[3])]   # (a Battle Video not saved: there's no recorder)
        if cmd in ('FrontierScrCmd_72', 'FrontierScrCmd_73'):
            return [self.tower(TOWER_SAVE)]
        if cmd == 'ChangeScript':
            low = FRONTIER_SCENES.get(a[0])
            if a[0] in FRONTIER_CLOSED_SCENES:
                return ['end']
            if low is None:
                raise Unsupported('frontier scene %s' % a[0])
            c = converter(self.root, low)
            m, x, y = c.start()
            self.calls.add((low, 1))
            return [self.sinnoh(SINNOH_WARP_CONTINUE), 'goto %s' % c.entry_label(1), 'warp %s, %d, %d' % (m, x, y),
                    'waitstate']
        if cmd == 'ExitFrontier':
            back = FRONTIER_EXIT.get(self.low)
            if back is None:
                raise Unsupported('frontier exit of %s' % self.low)
            return ['goto %s%s__frexit_%s' % (PREFIX, back[0], back[1])]
        return FileConverter.command(self, cmd, a)


# ----------------------------------------------------------------------------------------------------- all files

_converters = {}


def dw_constants(root):
    """The DISTORTION WORLD's progress values and its people's local ids (include/constants/distortion_world.h)"""
    path = os.path.join(root, 'include/constants/distortion_world.h')
    if not os.path.exists(path):
        return {}
    text = open(path).read()
    out = {'DIST_WORLD_MAP_OBJECT_BASE_LOCAL_ID': 128, 'GIRATINA_FORM_ALTERED': 0, 'GIRATINA_FORM_ORIGIN': 1,
           'BATTLE_RESULT_WIN': 1, 'BATTLE_RESULT_LOSE': 2, 'BATTLE_RESULT_CAPTURED_MON': 4, 'BATTLE_RESULT_DRAW': 3,
           'BATTLE_RESULT_PLAYER_FLED': 5, 'BATTLE_RESULT_ENEMY_FLED': 6}
    for m in re.finditer(r'enum\s*\w*\s*\{([^}]*)\}', text):
        n = -1
        for item in m.group(1).split(','):
            item = item.strip()
            if not item:
                continue
            if '=' in item:
                name, val = [t.strip() for t in item.split('=', 1)]
                val = re.sub(r'\w+', lambda w: str(out.get(w.group(0), w.group(0))), val)
                n = eval(val)
            else:
                name, n = item, n + 1
            out[name] = n
    return out


def converter(root, low):
    """the FileConverter of Platinum's scripts_<low>.s (None: no such file)"""
    if (root, 'dw') not in _converters:
        _converters[(root, 'dw')] = True
        CONSTANTS.update(dw_constants(root))   # (the DISTORTION WORLD's: s3d_distortion.c)
        CONSTANTS.update(enum_constants(root, 'include/constants/versions.h', 'VERSION_'))   # (GetGameVersion's answer)
    key = (root, low)
    if key not in _converters:
        dec = _converters.setdefault((root, None), Decomp(root)) if (root, None) not in _converters else _converters[(root, None)]
        if low.startswith('fr_'):
            path = os.path.join(root, 'res/field/frontier_scripts', 'frontier_scripts_%s.s' % low[3:])
            _converters[key] = FrontierConverter(dec, path) if os.path.exists(path) else None
        else:
            path = os.path.join(root, 'res/field/scripts', 'scripts_%s.s' % low)
            _converters[key] = FileConverter(dec, path) if os.path.exists(path) else None
    return _converters[key]


def convertible(root, low, n):
    """the label a map's stub goes to when Platinum's entry N of scripts_<low>.s converts, else None"""
    c = converter(root, low)
    if c is None or c.check(n) is not None:
        return None
    return c.entry_label(n)


def referenced():
    """{SinnohP_ label} every map's scripts point at (data/maps/Sinnoh_*/scripts.inc), and SINNOH's code (src/sinnoh/:
    the VILLA's furniture, s3d_gyms.c S3D_Gyms_VillaScript, and anything else run from C by its label)"""
    out = set()
    for path in glob.glob(os.path.join(ROOT, 'data/maps/Sinnoh_*/scripts.inc')) \
            + glob.glob(os.path.join(ROOT, 'data/scripts/sinnoh_*.inc')) \
            + glob.glob(os.path.join(ROOT, 'src/sinnoh/*.c')):
        out |= set(re.findall(r'\b(%s\w+)' % PREFIX, open(path, encoding='utf-8').read()))
    return {lab for lab in out if not lab.endswith('_')}   # (a macro's SinnohP_villa_##n is no label)


# ----------------------------------------------------------------------------------------------------- map scripts

# Platinum's init script kinds -> this game's map script kinds (include/constants/map_scripts.h)
INIT_KIND = {'OnTransition': 'MAP_SCRIPT_ON_TRANSITION', 'OnLoad': 'MAP_SCRIPT_ON_LOAD', 'OnResume': 'MAP_SCRIPT_ON_RESUME'}


def removes_after_battle(c, n):
    """Platinum's OnLoad entry N of converter C takes away an object once its battle is done (FLAG_MAP_LOCAL_REMOVE_OBJECT)"""
    try:
        for lab in c.reach(c.entries[n - 1]):
            for cmd, a in c.labels[lab]:
                if 'FLAG_MAP_LOCAL_REMOVE_OBJECT' in a:
                    return True
    except (IndexError, KeyError, Unsupported):
        pass
    return False


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


def inline_ours(name, lab, prefix, then):
    """the map's own map script LAB as lines that go on to THEN where it ends: a copy, its own labels renamed under
    PREFIX. (A 'call' of it never came back: its 'end' ends the whole script, so Platinum's own transition scripts never
    ran after it: GALACTIC HQ's first arrival, LAKE VERITY's, ROUTE 201's...)"""
    path = os.path.join(ROOT, 'data/maps', name, 'scripts.inc')
    raw = [ln.rstrip('\n') for ln in open(path, encoding='utf-8')]
    code = [ln.split('@')[0].rstrip() for ln in raw]
    start = next(k for k, ln in enumerate(code) if re.match(r'^%s::?$' % re.escape(lab), ln.strip()))
    body = []
    for ln in code[start + 1:]:
        t = ln.strip()
        if not t:
            break                                  # (a map's scripts are kept apart by an empty line)
        body.append(t)
    local = [t[:-1].rstrip(':') for t in body if re.match(r'^\w+::?$', t)]
    out, ends = [], 0
    for t in body:
        for k, l in enumerate(local):
            t = re.sub(r'\b%s\b' % re.escape(l), '%s_Own%d' % (prefix, k), t)
        if re.match(r'^\w+::?$', t):
            out.append(t.rstrip(':') + ':')
        elif t in ('end', 'return'):
            out.append('\tgoto %s' % then)
            ends += 1
        else:
            assert not re.match(r'^(goto|call)\b(?!_if)', t) or any('_Own' in w for w in t.split()), \
                '%s: %s leaves the script (%s)' % (name, lab, t)
            out.append('\t' + t)
    assert ends and out[-1] == '\tgoto %s' % then, '%s: %s doesn\'t end where it can go on' % (name, lab)
    return out


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
        if low in FRONTIER_SCENE_MAPS:
            once, frame = {}, []            # (the frontier scene's own script runs there: not Platinum's field one)
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


def _is_stand_in(name, script):
    """a person's script that is (or only goes to) one of this game's written stand-ins (STAND_INS)"""
    if STAND_INS.match(script):
        return True
    body = _stub_body(name, script) if script != '0x0' else None
    m = re.match(r'^\tgoto (\w+)\n$', body or '')
    return bool(m and STAND_INS.match(m.group(1)))


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


# Platinum's people for events and online features this game closes (a distribution's SHAYMIN, the Mystery Gift
# deliveryman, the S.S. SPIRAL's sailor): Platinum hides them by scripts that check for the event, which don't convert
CLOSED_STORY_PEOPLE = {'FLAG_HIDE_MART_MYSTERY_GIFT_DELIVERYMAN', 'FLAG_HIDE_ROUTE_224_SHAYMIN',
                       'FLAG_HIDE_SNOWPOINT_CITY_SAILOR_SS_SPIRAL'}
# hide flags nothing of Platinum's ever sets, whose people Platinum shows for good: GALACTIC HQ's TEAM GALACTIC, its 4F
# bollards with them (the warp panel from 1F lands behind them: the way in is the warehouse's)
SHOWN_FOR_GOOD = {'FLAG_HIDE_GALACTIC_HQ_TEAM_GALACTIC'}
_PLAT_SCRIPTS = {}


def _platinum_sets(root, flag):
    """does any of Platinum's scripts (the new game's too) set FLAG?"""
    if 'text' not in _PLAT_SCRIPTS:
        folder = os.path.join(root, 'res/field/scripts')
        _PLAT_SCRIPTS['text'] = '\n'.join(open(os.path.join(folder, f), encoding='utf-8', errors='replace').read()
                                         for f in sorted(os.listdir(folder)) if f.endswith('.s'))
    return re.search(r'\bSetFlag %s\b' % re.escape(flag), _PLAT_SCRIPTS['text']) is not None


def _story_shows(root, hide, name, o, c):
    """a person Platinum's story shows by its own hide flag (story_people.py's NEVER was the written story's: Platinum's
    scripts now show and hide its whole cast), not one the game closes (tools/sinnoh/gen_maps.py, CLOSED_STORY_PEOPLE).
    A flag no script of Platinum's ever sets (its people hidden by the map changing, or by its code): shown only when a
    scene of the map moves or removes them (LAKE VERITY's CYRUS)"""
    from gen_maps import CLOSED_PEOPLE
    if o.get('script') in CLOSED_PEOPLE.get(name, ()) or hide in CLOSED_STORY_PEOPLE:
        return False
    if _platinum_sets(root, hide):
        return True
    if hide in SHOWN_FOR_GOOD:
        return True
    lid = o.get('id')
    return bool(lid) and any(lid in a for lab in c.labels.values() for _, a in lab)


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
    once, frame = init_scripts(root, low)
    init_entries = set(once.values()) | {f[2] for f in frame}
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
        from gen_maps import VAR_LOOKS
        looks = i + 1 in VAR_LOOKS.get(name, ())   # (a look its script picks: gen_maps.py VAR_LOOKS)
        if ghost or (sprites.sprite(o['graphics_id']) is None and not looks) or not inside(p, o['x'], o['z']) \
                or o['graphics_id'].endswith('POKEBALL'):
            continue
        script = mine['script']
        person = script == '%s_EventScript_Person%d' % (name, i + 1) or script == '0x0'
        trainer = isinstance(o.get('script'), str)
        if lab and STAND_INS.match(script):
            new = lab                      # (a legendary, a story battle: Platinum's scene)
        elif t and t[0] == low and t[1] in init_entries and (_is_stand_in(name, script) or (
                person and script != '0x0' and _replaceable(_stub_body(name, script)))):
            # (a written stand-in or written words where Platinum gives the person nothing of its own to run, only its
            # map's script: SPEAR PILLAR's CYRUS, whose scene the MARS / JUPITER trigger leads to, and BARRY there)
            new = 'SinnohP_Nothing'
        elif person and lab and _replaceable(_stub_body(name, script) if script != '0x0' else None):
            new = lab
        elif person and script != '0x0' and o.get('script') == 0 and _replaceable(_stub_body(name, script)):
            # (Platinum gives this person nothing to say (script 0: GALACTIC HQ's grunts in CYRUS's speech...): the
            # written words go)
            new = 'SinnohP_Nothing'
        elif person and script == '0x0':
            if mine['flag'] == '0x3FFF' and flag and _story_shows(root, hide, name, o, c):
                # (a story person with nothing to say: CYRUS and BARRY at LAKE VERITY, a STARLY... the map keeps
                # them hidden for good, Platinum shows them by their own hide flag)
                gfx = 'OBJ_EVENT_GFX_VAR_%s' % o['graphics_id'][-1] \
                    if re.match(r'^OBJ_EVENT_GFX_VAR_[0-9A-F]$', o['graphics_id']) else '0'
                objs.append((i + 1, flag, o['x'] - p['x0'] * 32, o['z'] - p['z0'] * 32, lab, gfx))
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
        if 'OnLoad' in ok and 'OnResume' not in once and removes_after_battle(c, ok['OnLoad']):
            # (Platinum loads the map again after a battle and its OnLoad takes away the legendary just fought: this
            # game comes back to the map without loading it, through ON_RESUME)
            ok['OnResume'] = ok['OnLoad']
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
                lines += ['%s_%s:' % (base, plat)] + inline_ours(name, lab, '%s_%s' % (base, plat),
                                                                c.entry_label(ok[plat]))
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
    lines = [('%s:\n\t.byte 0xFF' if TEXT_REF.match(lab) else '%s::\n\tend') % lab for lab in sorted(referenced())] \
        + map_index([]) + ['gSinnohPlatinumNewGame::', '\tend'] + stub_dex_ratings()
    lines += ['gSinnohTvBroadcast::', '\tend']
    lines += tv_data.stub_lines()
    import contest_data
    lines += contest_data.stub_lines()
    write(lines, out_path)
    print('%s: %d stubs (no decomp)' % (os.path.relpath(out_path, ROOT), len(referenced())))


TEXT_REF = re.compile(r'^%sT_(\d+)_(\d+)$' % PREFIX)   # (one of Platinum's lines, by bank and number, from this game's scripts)


def build(root, out_path):
    import gen_distortion
    refs = referenced() | gen_distortion.script_labels(root)   # (and the DISTORTION WORLD's people's: s3d_distortion.c)
    text_refs = {lab for lab in refs if TEXT_REF.match(lab)}
    refs -= text_refs
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
    lines += ['SinnohP_Nothing::', '\tend']   # (a person Platinum gives nothing to say: people())
    lines += ['SinnohP_Movement_SmashRock:', '\trock_smash_break', '\tstep_end']
    lines += ['SinnohP_Movement_Invisible:', '\tset_invisible', '\tstep_end']
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
    for lab in text_refs:   # (Platinum's own lines this game's scripts say: SinnohP_T_<bank>_<number>)
        m = TEXT_REF.match(lab)
        texts[lab] = (int(m.group(1)), int(m.group(2)))
    for lab, (b, m) in sorted(texts.items()):
        lines.append('%s:\n\t.byte %s' % (lab, text_bytes(b, m)))
    write(lines, out_path)
    print('%s: %d scripts, %d texts, Platinum\'s story on %d maps (%d of its map scripts left out)'
          % (os.path.relpath(out_path, ROOT), len(done), len(texts), maps, skipped))


def report(root, entries=False):
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
                if entries:
                    print('%s %d %s: %s' % (low, n, name, w))
    print('%d of %d script entries convert' % (ok, total))
    print('songs the game lacks: %s' % ' '.join(sorted(MISSING_MUSIC)))
    for w, k in why.most_common(60):
        print('  %5d  %s' % (k, w))


def daily_flags(root, out='src/data/sinnoh_daily_flags.h'):
    """Platinum's FLAG_DAILY_* (cleared as a new day starts: its daily events) as this game's numbers, for event_data.c
    ClearDailyFlags (numbers only, from tools/sinnoh/ids.py: committed)"""
    nums = sorted(n for name, n in ids._names(root)[0].items() if name.startswith('FLAG_DAILY_'))
    with open(os.path.join(ROOT, out), 'w') as f:
        f.write('// Generated by tools/sinnoh/gen_scripts.py --daily-flags: Platinum\'s FLAG_DAILY_* as SINNOH\'s flags, cleared\n'
                '// as a new day starts (event_data.c ClearDailyFlags)\n'
                'static const u16 sSinnohDailyFlags[] = {\n')
        for i in range(0, len(nums), 12):
            f.write('    ' + ', '.join('0x%X' % n for n in nums[i:i + 12]) + ',\n')
        f.write('};\n')
    print('%s: %d daily flags' % (out, len(nums)))


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
    elif a and a[0] == '--daily-flags':
        daily_flags(a[1])
    elif a and a[0] == '--report':
        report(a[1])
    elif a and a[0] == '--report-entries':
        report(a[1], True)
    elif a:
        build(a[0], a[1] if len(a) > 1 else OUT)
    else:
        print(__doc__)
        sys.exit(1)


if __name__ == '__main__':
    main()
